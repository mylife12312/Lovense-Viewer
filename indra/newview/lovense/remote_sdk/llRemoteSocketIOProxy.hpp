#ifndef __LL_REMOTE_SOCKET_IO_PROXY_H
#define __LL_REMOTE_SOCKET_IO_PROXY_H

#include <mutex>
#include <memory>
#include <condition_variable>
#include "deps/socketio/sio_client.h"
#include "ThreadPool.h"
#include "CJsonObject.hpp"
#include "../llbase.h"
#include "lleventdispatcher.h"
#include "../llToyModel.h"
namespace lovense
{
	static const char	*kQrcodeChangeListener			= "basicapi_get_qrcode_tc";
    static const char	*kDevicesChangeListener			= "basicapi_update_device_info_tc";
    static const char	*kAppStatusChangeListener		= "basicapi_update_app_status_tc";
    static const char   *kAppOnlineChangeListener		= "basicapi_update_app_online_tc";

	static const char* kSocketIODispatcher = "kSocketIODispatcher";
	static const char* kSocketIOEventType = "kSocketIOEventType";

	enum class SocketIOEventType : LLSD::Integer {
		Event_QR_Code_Update,
		Event_Device_Info_Update,
		Event_App_Status_Update,
		Event_App_Online_Update,
	};

    static const char  *kQrcodeContent		= "kQrcodeContent";
    static const char  *kQrcodeUrl			= "kQrcodeUrl";
    static const char  *kDevcesArrays		= "kDevcesArrays";
    static const char  *kAppChangeStatus	= "status";//int 0 or 1

	typedef enum _Status
	{
		e_Init = 0,
		e_SocketAddress = 1,
		e_Connecting = 2,
		e_Connected = 3,
		e_Error = 999,
	}EnumSocketClientStatus;

	typedef enum _RemoteStatus {
		e_Online = 0,
		e_Offline = 1,
	}EnumRemoteStatus;

	class LLRemoteSocketIOProxy:public LLSingleton<LLRemoteSocketIOProxy>
	{

		LLSINGLETON(LLRemoteSocketIOProxy);
	protected:
		virtual void initSingleton();
		virtual void cleanupSingleton();

	public:

		bool init(std::string userId = "",std::string userName = "");
		void shutdown();
		void release();
		void tryConnectServer(bool reconnect = false);
		void disConnectServer();


        // remote api: send command
        void emitToyCommand(const std::string &command);
		void emitQrcodeCommand();


		LLEventDispatcher &getEventDispatcher() { return mEventDispacher; }
		EnumSocketClientStatus getSocketIOStatus()const {	return mIoStatus.load(); }
		EnumRemoteStatus getRemoteStatus() const { return mRemoteStatus.load(); }
		bool qrcodeReady();
		const LLQRCodeInfo& getQRCodeCache()const { return mQRcodeInfoCache; }

	private:
		//------------------------------------------------------
		//Socket.io: Socket client event
		void on_connected();
		void on_closed(sio::client::close_reason const& reason);
		void on_failed();

		//------------------------------------------------------
		//Socket.io:Connect to server
        void connect(std::shared_ptr<LLRemoteAPIParameter> parameter);
		void syncClose();

		// Http Request
        // Step 1: Application for user authentication token
        bool request_authentication_token(std::string &token);

        // Step 2 : Validate authorization
		std::shared_ptr<LLRemoteAPIParameter> request_validate_authorization(const std::string &authtoken);

		void generate_socket_io_parameter();
			
		//Socket.io:Listen Event
		void on_listen_get_qr_code(std::string const& name, sio::message::ptr const& data, bool hasAck, sio::message::list& ack_resp);
		void on_listen_update_device_info(std::string const& name, sio::message::ptr const& data, bool hasAck, sio::message::list& ack_resp);
		void on_listen_update_app_status(std::string const& name, sio::message::ptr const& data, bool hasAck, sio::message::list& ack_resp);
		void on_listen_update_app_online(std::string const& name, sio::message::ptr const& data, bool hasAck, sio::message::list& ack_resp);
		//Socket.io:Emit Event
        void emitRequestQRcodeInfo();

		//Dispatch qr code change event
		void onQrcodeUpdate(const std::string& qrUrl, const std::string& qrContent);
		void dispatchEvent(const LLSD &event);
		void dispatchEvent(const std::string &name, const LLSD &event);
	private:
		int httpPost(const std::string& strUrl, const std::string& strPost, std::string& strResponse);
	private:
		std::unique_ptr<sio::client> mSocketIo;
		std::atomic<EnumSocketClientStatus> mIoStatus{ e_Init };
		std::atomic<EnumRemoteStatus> mRemoteStatus{ e_Offline };
		std::shared_ptr<LSThreadPool>              mThreadPool;
		std::condition_variable mConnectCv;
		std::mutex              mConnectLock;

		//LLRemoteAPIParameter mApiParameterCache;
		LLQRCodeInfo mQRcodeInfoCache;
		std::list<LLToyPtr> mToyListCache;
		LLEventDispatcher mEventDispacher;
		std::string mAvUserName;
		std::string mAvUserID;

		std::mutex  mSocketIoMutex;
		std::list<std::shared_ptr<LLRemoteAPIParameter>> mSocketIoParameterLists;
	};
	
}   // namespace lovense
#endif  // !__LL_REMOTE_SOCKET_IO_PROXY_H
