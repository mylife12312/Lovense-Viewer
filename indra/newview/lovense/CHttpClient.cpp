#include <curl/curl.h>
#include <list>
#include <array>
#include <iostream>
#include "CHttpClient.h"

#define REQUEST_READ_TIMEOUT 6
#ifndef FALSE
#define FALSE false
#endif

CHttpClient::CHttpClient() : m_bDebug(false) {


}

CHttpClient::~CHttpClient() {


}


static int OnDebug(CURL *, curl_infotype itype, char * pData, size_t size, void *) {
	if (itype == CURLINFO_TEXT)
	{
		//printf("[TEXT]%s\n", pData);  
	}
	else if (itype == CURLINFO_HEADER_IN)
	{
		printf("[HEADER_IN]%s\n", pData);
	}
	else if (itype == CURLINFO_HEADER_OUT)
	{
		printf("[HEADER_OUT]%s\n", pData);
	}
	else if (itype == CURLINFO_DATA_IN)
	{
		printf("[DATA_IN]%s\n", pData);
	}
	else if (itype == CURLINFO_DATA_OUT)
	{
		printf("[DATA_OUT]%s\n", pData);
	}
	return 0;
}

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid) {
	std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
	if (NULL == str || NULL == buffer)
	{
		return -1;
	}

	char* pData = (char*)buffer;
	str->append(pData, size * nmemb);
	return nmemb;
}

int CHttpClient::Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse, std::list<std::string> &headerExtra) {
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl) {
		return CURLE_FAILED_INIT;
	}
	if (m_bDebug) {
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
    if (0 == strncmp(strUrl.c_str(), "https:", 6))
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, FALSE);
    }

	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);

	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept:application/json");
	headers = curl_slist_append(headers, "Content-Type:application/json");
	headers = curl_slist_append(headers, "charset:utf-8");
	for (std::list<std::string>::iterator it = headerExtra.begin(); it != headerExtra.end(); it++) {
		headers = curl_slist_append(headers, (*it).c_str());
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(strPost.c_str()));
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, REQUEST_READ_TIMEOUT);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, REQUEST_READ_TIMEOUT);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

int CHttpClient::Put(const std::string & strUrl, const std::string & strPut, std::string & strResponse, std::list<std::string> &headerExtra) {
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl) {
		return CURLE_FAILED_INIT;
	}
	if (m_bDebug) {
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());

	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept:application/json");
	headers = curl_slist_append(headers, "Content-Type:application/json");
	headers = curl_slist_append(headers, "charset:utf-8");
	for (std::list<std::string>::iterator it = headerExtra.begin(); it != headerExtra.end(); it++) {
		headers = curl_slist_append(headers, (*it).c_str());
	}
	headers = curl_slist_append(headers, "Expect:");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPut.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(strPut.c_str()));
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);

	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, REQUEST_READ_TIMEOUT);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, REQUEST_READ_TIMEOUT);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

int CHttpClient::Get(const std::string & strUrl, std::string & strResponse, std::list<std::string> &headerExtra) {
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl) {
		return CURLE_FAILED_INIT;
	}
	if (m_bDebug) {
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	if (0 == strncmp(strUrl.c_str(), "https:", 6))
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, FALSE);
	}
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept:application/json");
	headers = curl_slist_append(headers, "Content-Type:application/json");
	headers = curl_slist_append(headers, "charset:utf-8");
	for (std::list<std::string>::iterator it = headerExtra.begin(); it != headerExtra.end(); it++) {
		headers = curl_slist_append(headers, (*it).c_str());
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);

	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, REQUEST_READ_TIMEOUT);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, REQUEST_READ_TIMEOUT);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}
static size_t  write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	if (stream && ptr)
	{
		size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
		return written;
	}
	return -1;
}

int  CHttpClient::downLoadFile(std::string filename, std::string newFilename)
{
	CURL *curl_handle;
	FILE *pagefile;
	char *p = (char *)filename.data();
	//  curl_global_init(CURL_GLOBAL_ALL);

	   /* init the curl session */
	curl_handle = curl_easy_init();

	/* set URL to get here */
	curl_easy_setopt(curl_handle, CURLOPT_URL, p);

	/* Switch on full protocol/debug output while testing */
	curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);

	/* disable progress meter, set to 0L to enable and disable debug output */
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
	/* google.com is redirected, so we tell LibCurl to follow redirection */
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
	/* SSL Options */
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0);

	/* Provide CA Certs from http://curl.haxx.se/docs/caextract.html */
	//curl_easy_setopt(curl_handle, CURLOPT_CAINFO, "ca-bundle.crt");
	/* send all data to this function  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

	/* open the file */
	pagefile = fopen(newFilename.c_str(), "wb");
	int error = -1;
	if (pagefile) {

		/* write the page body to this file handle */
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);

		/* get it! */
		 error = curl_easy_perform(curl_handle);

		/* close the header file */
		fclose(pagefile);

	}
	else
	{
		return 1;
	}

	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);

	return error;
}




int CHttpClient::download(std::string url, std::string local_file, int down_speed)
{
	CURL *image;
	CURLcode imgresult;
	FILE *fp = NULL;
	//url_download.c_str();

	image = curl_easy_init();
	if (image)
	{
		//Open File
		fp = fopen(local_file.c_str(), "wb");
		if (fp == NULL)
		{
			//cout << "File cannot be opened";

			return CURLE_RECV_ERROR;
		}
		//if (0 == strncmp(url.c_str(), "https:", 6))
		{
			curl_easy_setopt(image, CURLOPT_SSL_VERIFYPEER, FALSE);
			curl_easy_setopt(image, CURLOPT_SSL_VERIFYHOST, FALSE);
		}
		curl_easy_setopt(image, CURLOPT_WRITEFUNCTION, NULL);
		curl_easy_setopt(image, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(image, CURLOPT_URL, url.c_str());
		curl_easy_setopt(image, CURLOPT_FOLLOWLOCATION, 1);
		//这里限速 100KB/s
		//curl_easy_setopt(image, CURLOPT_MAX_RECV_SPEED_LARGE, (curl_off_t)1000 * 1024 * 8);
		curl_easy_setopt(image, CURLOPT_NOPROGRESS, 1);
		//CURLOPT_RESUME_FROM

		// Grab image
		imgresult = curl_easy_perform(image);
		if (imgresult)
		{
			//cout << "Cannot grab the File!\n";
			return CURLE_RECV_ERROR;
		}
		//Clean up the resources
		curl_easy_cleanup(image);
		//Close the file
		fclose(fp);
	}

	return 0;
}

int CHttpClient::Delete(const std::string & strUrl, std::string & strResponse, std::list<std::string> &headerExtra) {
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl) {
		return CURLE_FAILED_INIT;
	}
	if (m_bDebug) {
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());

	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept:application/json");
	headers = curl_slist_append(headers, "Content-Type:application/json");
	headers = curl_slist_append(headers, "charset:utf-8");
	for (std::list<std::string>::iterator it = headerExtra.begin(); it != headerExtra.end(); it++) {
		headers = curl_slist_append(headers, (*it).c_str());
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	std::cout << "receive!!!:" << strResponse << std::endl;
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, REQUEST_READ_TIMEOUT);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, REQUEST_READ_TIMEOUT);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

void CHttpClient::SetDebug(bool bDebug) {
	m_bDebug = bDebug;
}
