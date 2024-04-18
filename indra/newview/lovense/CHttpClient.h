/** 
 * @file CHttpClient.h
 * @brief CHttpClient class header file
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