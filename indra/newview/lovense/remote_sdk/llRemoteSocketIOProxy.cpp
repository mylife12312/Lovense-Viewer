/** 
 * @file llRemoteSocketIOProxy.cpp
 * @brief llRemoteSocketIOProxy class implementation
 *
 * $LicenseInfo:firstyear=2023&license=viewerlgpl$
 * Lovense Viewer Source Code
 * Copyright (C) 2023, HYTTO PTE. LTD.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * $/LicenseInfo$
 */

#include <map>
#include <future>
#include <sstream>
#include <list>
#include <iostream>
#include <curl/curl.h>
#include "llfilesystem.h"
#include "ThreadPool.h"
#include "llRemoteSocketIOProxy.hpp"
#include "lovense_uri.hpp"
#include "../CHttpClient.h"
#include "CJsonObject.hpp"
#include "llevents.h"
#include "lldir.h"
#include "llfile.h"


//https://developer.lovense.com/
static const std::string WEBCAM_PROXY_URL = "https://api.lovense-api.com/api/basicApi/getSocketUrl";
static const std::string WEBCAM_GET_TOKEN_URL = "https://api.lovense-api.com/api/basicApi/getToken";
static const std::string SDK_DEV_TOKEN = "pGWs1wmWk5XtJ_5o21i617dFxXCXCLGSE59I8oMuacvNLqmhDgwxgGvdbdjM0p9i";
static const std::string SDK_PLATFORM = "Second Life Lovense viewer";

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid) {
	std::string* str = dynamic_cast<std::string*>((std::string*)lpVoid);
	if (NULL == str || NULL == buffer)
	{
		return -1;
	}

	char* pData = (char*)buffer;
	str->append(pData, size * nmemb);
	return nmemb;
};


namespace lovense
{
	LLRemoteSocketIOProxy::LLRemoteSocketIOProxy() :mEventDispacher("",kSocketIODispatcher) {

	}

	void LLRemoteSocketIOProxy::initSingleton() {
		mThreadPool = std::make_shared<LSThreadPool>(1);
		mSocketIo = std::make_unique<sio::client>();
	}

	void LLRemoteSocketIOProxy::cleanupSingleton() {
		shutdown();
		release();
	}

	bool LLRemoteSocketIOProxy::init(std::string userId, std::string userName)
	{
		mAvUserID = userId;
		mAvUserName = userName;

		mSocketIo->set_reconnect_attempts(2);
		mSocketIo->set_fail_listener(std::bind(&LLRemoteSocketIOProxy::on_failed, this));
		using std::placeholders::_1;		
		mSocketIo->set_open_listener(std::bind(&LLRemoteSocketIOProxy::on_connected, this));
		mSocketIo->set_close_listener(std::bind(&LLRemoteSocketIOProxy::on_closed, this, _1));
		//tryConnectServer();
		mThreadPool->enqueue([this]() {
			this->generate_socket_io_parameter();
		});
		return true;
	}

	void LLRemoteSocketIOProxy::shutdown()
	{
		if (mSocketIo) {
			mSocketIo->clear_socket_listeners();
			mSocketIo->socket()->off_all();
			mSocketIo->socket()->off_error();
			mSocketIo->clear_con_listeners();
			mSocketIo->sync_close();
		}
		mThreadPool->Stop();
	}

	void LLRemoteSocketIOProxy::release()
	{
		if(mSocketIo && mSocketIo->opened())
			mSocketIo->close();
	}

	void LLRemoteSocketIOProxy::on_connected()
	{
		{
			mIoStatus = e_Connected;
			std::unique_lock<std::mutex> lk(mConnectLock);
			mConnectCv.notify_all();
		}

		using std::placeholders::_1;
		using std::placeholders::_2;
		using std::placeholders::_3;
		using std::placeholders::_4;
		auto sock = mSocketIo->socket();

		//listen remote event
		sock->on(kQrcodeChangeListener, sio::socket::event_listener_aux(
			std::bind(&LLRemoteSocketIOProxy::on_listen_get_qr_code, this, _1, _2, _3, _4)));

		sock->on(kDevicesChangeListener,sio::socket::event_listener_aux(std::bind(&LLRemoteSocketIOProxy::on_listen_update_device_info,this,_1,_2,_3,_4)));

		sock->on(kAppStatusChangeListener,
                sio::socket::event_listener_aux(std::bind(&LLRemoteSocketIOProxy::on_listen_update_app_status, this, _1, _2, _3, _4)));

		sock->on(kAppOnlineChangeListener,
                sio::socket::event_listener_aux(std::bind(&LLRemoteSocketIOProxy::on_listen_update_app_online, this, _1, _2, _3, _4)));

		sock->on_error([](sio::message::ptr const& message) {
			auto m = message->get_flag();
			std::cout<<m<<std::endl;
			});

		//Request qrcode when socketio connected
        this->emitQrcodeCommand();
	}

	void LLRemoteSocketIOProxy::emitRequestQRcodeInfo() {

		if (!mSocketIo->opened())
        {
			tryConnectServer();
            return;
        }

		if (!mQRcodeInfoCache.getContent().empty()) {

			/*std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - mQRcodeInfoCache.getTime();
			if (elapsed_seconds.count() < 14400) {
				onQrcodeUpdate(mQRcodeInfoCache.getUrl(), mQRcodeInfoCache.getContent());
				return;
			}*/
		}

        neb::CJsonObject data;
        data.Add("ackId", Until::generateUUID());
        mSocketIo->socket()->emit("basicapi_get_qrcode_ts", data.ToString());
	}


	void LLRemoteSocketIOProxy::on_closed(sio::client::close_reason const& reason)
	{
		mIoStatus.store(e_Init);
		std::unique_lock<std::mutex> lk(mConnectLock);
		mConnectCv.notify_all();
		LL_INFOS() << "(LOVENSE) on socket closed!" << LL_ENDL;
	}

	void LLRemoteSocketIOProxy::on_failed()
	{
		mIoStatus.store(e_Init);
		{
			std::unique_lock<std::mutex> lk(mConnectLock);
			mConnectCv.notify_all();
		}			
		LL_INFOS() << "(LOVENSE) on socket failed" << LL_ENDL;
	}

	void LLRemoteSocketIOProxy::syncClose() {
		if (mSocketIo && mSocketIo->opened()) {
			mSocketIo->sync_close();
		}
	}


	void LLRemoteSocketIOProxy::connect(std::shared_ptr<LLRemoteAPIParameter> parameter)
	{
		if (mIoStatus.load() == e_Connected) {
			LL_INFOS() << "(LOVENSE) on socket had connected!" << LL_ENDL;
			return;
		}
		if (mSocketIo->opened()) {
			mSocketIo->socket()->off_all();
			mSocketIo->close();
		}

		{
			mIoStatus.store(e_Connecting);
			mSocketIo->connect(parameter->getSocketIoUrl(), parameter->getSocketIoQuery());
		}
		LL_INFOS() << "(LOVENSE) on socket reconnct" << LL_ENDL;

	}

	void LLRemoteSocketIOProxy::generate_socket_io_parameter() {

		{
			std::unique_lock lock(mSocketIoMutex);
			if (mSocketIoParameterLists.size() > 5)
				return;
		}

		std::string token;
		if (this->request_authentication_token(token)) {
			std::shared_ptr<LLRemoteAPIParameter> parameter = this->request_validate_authorization(token);
			if (parameter->valid()) {
				std::unique_lock lock(mSocketIoMutex);
				mSocketIoParameterLists.push_back(parameter);
			}
		}
	}

	void LLRemoteSocketIOProxy::tryConnectServer(bool reconnect)
	{	
		if (mIoStatus.load() == e_Connecting)
			return;

		if (!reconnect && mSocketIo && mSocketIo->opened()) {
			return;
		}

		mThreadPool->enqueue([this,reconnect]() {
			mIoStatus.store(e_Connecting);
			bool result = false;

			std::shared_ptr<LLRemoteAPIParameter> parameter = nullptr;
			{
				std::unique_lock lock(mSocketIoMutex);
				if (!mSocketIoParameterLists.empty()) {
					parameter = mSocketIoParameterLists.front();
					mSocketIoParameterLists.pop_front();
				}
			}

			if (!parameter) {
				generate_socket_io_parameter();
				{
					std::unique_lock lock(mSocketIoMutex);
					if (!mSocketIoParameterLists.empty()) {
						parameter = mSocketIoParameterLists.front();
						mSocketIoParameterLists.pop_front();
					}
				}
			}
			
			if (parameter && parameter->valid()) {
				if (reconnect)
					this->syncClose();
				this->connect(parameter);
				std::unique_lock<std::mutex> lk(mConnectLock);
				result = mConnectCv.wait_for(lk, std::chrono::seconds(60), [&] {
					auto s = mIoStatus.load();
					if (s == e_Connected) {
						return true;
					}
					return false;
				});
				generate_socket_io_parameter();
				LL_INFOS() << "(LOVENSE):connect result:" << result << LL_ENDL;
			}
			if(!result) {
				onQrcodeUpdate("","");
				mIoStatus.store(e_Init);
			}
		});		
	}

	void LLRemoteSocketIOProxy::disConnectServer() {
		if (mSocketIo && mSocketIo->opened()) {
			mSocketIo->close();
		}
	}

	void LLRemoteSocketIOProxy::on_listen_get_qr_code(std::string const &name, sio::message::ptr const &data, bool hasAck,
                                            sio::message::list &ack_resp){
        std::string qr_code = "qrcode:" + data->get_string();
        LL_INFOS() << "(LOVENSE)=>qrcode:" << qr_code << LL_ENDL;
        std::shared_ptr<std::string> pstrJson = std::make_shared<std::string>(data->get_string());
        neb::CJsonObject root;
        if (pstrJson && root.Parse(*pstrJson))
        {
            int code = -1;
            if (root.Get("code", code) && (code == 0))
            {
                neb::CJsonObject data;
                if (root.Get("data", data))
                {
                    std::string qrcodeUrl,codeString;
                    if (data.Get("qrcodeUrl", qrcodeUrl) && data.Get("qrcode", codeString))
                    {
                        mQRcodeInfoCache.setUrl(qrcodeUrl);
						mQRcodeInfoCache.setContent(codeString);
						mQRcodeInfoCache.setTime(std::chrono::system_clock::now());
						onQrcodeUpdate(qrcodeUrl,codeString);
                    }
                }
            }
        }
	}

	void LLRemoteSocketIOProxy::on_listen_update_device_info(std::string const& name, sio::message::ptr const& data, bool hasAck, sio::message::list& ack_resp) {

		LL_INFOS() << "(LOVENSE)=>toy data:" << data->get_string() << LL_ENDL;
		mToyListCache.clear();
        neb::CJsonObject root;
        if (root.Parse(data->get_string())) {

			bool online = false;
			if (root.Get("online", online)) {
				LLSD event;
				event[kSocketIOEventType] = (LLSD::Integer)SocketIOEventType::Event_App_Online_Update;
				event[kAppChangeStatus] = online;
				dispatchEvent(event);
			}


            neb::CJsonObject toyList;
			if (root.Get("toyList", toyList)) {
                if (toyList.IsArray()) {

					LLSD event;
					event[kSocketIOEventType] = (LLSD::Integer)SocketIOEventType::Event_Device_Info_Update;
					LLSD arrays = LLSD::emptyArray();
					int arraySize = toyList.GetArraySize();
                    for (int i = 0; i < arraySize; i++) {
                        neb::CJsonObject toy;
						if (toyList.Get(i, toy)) {
							LLSD item = lovense::LLToy::JsonDataToLLSD(toy);
							arrays.append(item);
							mToyListCache.push_back(lovense::LLToy::JsonDataToPtr(toy));
						}
					}
                    event[kDevcesArrays] = arrays;
					dispatchEvent(event);
				}
			}
		}
	}

	void LLRemoteSocketIOProxy::on_listen_update_app_status(std::string const& name, sio::message::ptr const& data, bool hasAck, sio::message::list& ack_resp) {

		LL_INFOS() << "(LOVENSE)=>app status update" << name << ":" << data->get_string() << LL_ENDL;
		neb::CJsonObject root;
        if (root.Parse(data->get_string())) {
            int  status = 0;
			if (root.Get("status", status)) {
				status == 0 ? mRemoteStatus.store(e_Offline) : mRemoteStatus.store(e_Online);
				LLSD event;
				event[kSocketIOEventType] = (LLSD::Integer)SocketIOEventType::Event_App_Status_Update;
				event[kAppChangeStatus] = status;
				dispatchEvent(event);
			}
		}
	}


	void LLRemoteSocketIOProxy::on_listen_update_app_online(std::string const& name, sio::message::ptr const& data, bool hasAck, sio::message::list& ack_resp) {
        LL_INFOS() << "(LOVENSE)=>app online update" << name << ":" << data->get_string() << LL_ENDL;
        neb::CJsonObject root;
        if (root.Parse(data->get_string()))
        {
            int status = 0;
			if (root.Get("status", status)) {
				status == 0 ? mRemoteStatus.store(e_Offline) : mRemoteStatus.store(e_Online);
				LLSD event;
				event[kSocketIOEventType] = (LLSD::Integer)SocketIOEventType::Event_App_Online_Update;
				event[kAppChangeStatus] = status;
				dispatchEvent(event);
			}
        }
	}

		 
	void LLRemoteSocketIOProxy::emitToyCommand(const std::string& command) {
		if (!mSocketIo->opened()) {
			return;
		}
		mSocketIo->socket()->emit("basicapi_send_toy_command_ts", command);
	}

	void LLRemoteSocketIOProxy::emitQrcodeCommand() { 
		this->emitRequestQRcodeInfo();
	}

	bool LLRemoteSocketIOProxy::qrcodeReady() {
		auto s = mIoStatus.load();
		if (s == e_Connected)
			return true;
		return false;
	}


	std::shared_ptr<LLRemoteAPIParameter> LLRemoteSocketIOProxy::request_validate_authorization(const std::string& authtoken)
	{
		std::shared_ptr<LLRemoteAPIParameter> cache = std::make_shared<LLRemoteAPIParameter>();
		neb::CJsonObject postJson;
		postJson.Add("platform",SDK_PLATFORM);
		postJson.Add("authToken",authtoken);

		std::string response;
		int code = this->httpPost(WEBCAM_PROXY_URL, postJson.ToString(), response);
		LL_INFOS() << "(LOVENSE)=>request:" << postJson.ToString() << LL_ENDL;
        LL_INFOS() << "(LOVENSE)=>response:" << response << LL_ENDL;
		if (code == 0)
		{
			neb::CJsonObject root;
			neb::CJsonObject data;
			if (root.Parse(response) && root.Get("data",data))
			{
				std::string url, io_path;
				data.Get("socketIoUrl", url);
				data.Get("socketIoPath", io_path);
				if ((!url.empty()) && (!io_path.empty()))
				{
					lovense::Uri uri(url);
					if (io_path.size() > 0 && io_path[io_path.size() - 1] != '/')
					{
						io_path.push_back('/');
					}
					cache->setSocketIoUrl(uri.scheme() + ":" + uri.path() + io_path);
					for (auto it : uri.getQueryParams())
					{
						cache->addQuery(it.first, it.second);
					}
					return cache;
				}
			}
		}
		return cache;
	}

	bool LLRemoteSocketIOProxy::request_authentication_token(std::string& proxy_token)
	{
		neb::CJsonObject postJson;
		postJson.Add("uid", Until::generateUUID());
		postJson.Add("token",SDK_DEV_TOKEN);
		postJson.Add("uname", Until::generateUUID());
		postJson.Add("utoken", Until::generateUUID());

		std::string response;				
		int code = this->httpPost(WEBCAM_GET_TOKEN_URL, postJson.ToString() , response);
		LL_INFOS() << "(LOVENSE)=>request:" << postJson.ToString() << LL_ENDL;
        LL_INFOS() << "(LOVENSE)=>response:" << code << " :" << response << LL_ENDL;
		if (code == 0){
			neb::CJsonObject root;
			if (root.Parse(response)){
				std::string server_token;
				if (root.Get("code", code) && code == 0) {
					neb::CJsonObject data;
					if (root.Get("data", data) && data.Get("authToken", server_token) && !server_token.empty()) {
						proxy_token = server_token;
						return true;
					}
				}
				
			}
		}
		return false;
	}


	int LLRemoteSocketIOProxy::httpPost(const std::string& strUrl, const std::string& strPost, std::string& strResponse)
	{
		int ret = -1;
			
		CURL* curl;
		curl = curl_easy_init();

		LL_INFOS() << "(LOVENSE)=>url:" << strUrl << LL_ENDL;
		if (curl) {

			if (0 == strncmp(strUrl.c_str(), "https:", 6))
            {
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, FALSE);
            }

			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
            curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
			struct curl_slist* headers = NULL;
			//headers = curl_slist_append(headers, "User-Agent: stream mater");
			headers = curl_slist_append(headers, "Accept: *");
			headers = curl_slist_append(headers, "Connection: keep-alive");
			headers = curl_slist_append(headers, "Accept:application/json");
			headers = curl_slist_append(headers, "Content-Type:application/json");
			headers = curl_slist_append(headers, "charset:utf-8");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
			ret = curl_easy_perform(curl);
		}
		curl_easy_cleanup(curl);
		return ret;
			
	}

	void LLRemoteSocketIOProxy::onQrcodeUpdate(const std::string& qrUrl, const std::string& qrContent) {
		LLSD event;
		event[kSocketIOEventType] = (LLSD::Integer)SocketIOEventType::Event_QR_Code_Update;
		event[kQrcodeContent] = qrContent;
		event[kQrcodeUrl] = qrUrl;
		dispatchEvent(event);
	}

	void LLRemoteSocketIOProxy::dispatchEvent(const LLSD& event) {
		dispatchEvent(kSocketIODispatcher,event);
	}

	void LLRemoteSocketIOProxy::dispatchEvent(const std::string& name, const LLSD& event) {
		getEventDispatcher().try_call(name,event);
	}

}
