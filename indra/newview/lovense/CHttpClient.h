#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H
#include <string>
#include <list>

class CHttpClient {
public:
	CHttpClient();
	~CHttpClient();

	void SetDebug(bool bDebug);

	int Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse, std::list<std::string> &headerExtra);
	int Put(const std::string & strUrl, const std::string & strPut, std::string & strResponse, std::list<std::string> &headerExtra);

	int download(std::string url, std::string local_file, int down_speed);
	int Get(const std::string & strUrl, std::string & strResponse, std::list<std::string> &headerExtra);
	int Delete(const std::string & strUrl, std::string & strResponse, std::list<std::string> &headerExtra);
	int  downLoadFile(std::string filename, std::string newFilename);
private:

	bool m_bDebug;

};

#endif