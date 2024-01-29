#ifndef LL_BASE_H
#define LL_BASE_H
#include <string>
#include <chrono>
#include <ctime>
#include <map>
#include "lluuid.h"
namespace lovense
{
    class LLRemotePattern
    {
      #define DEF_DEFAULT_HASH "7MoL49hqYo5HNeH2"
      public:
        std::string version  = "1";
        std::string toy      = "";
        std::string func     = "v";
        std::string interval = "100";
        std::string hash     = "";
        std::string pattern  = "";

      public:
        static std::string getStopPattern(std::string toyid = "");
        static std::string getRunPattern(std::string pattern, std::string toyid = "");
        std::string dump();
        bool isValid() const { return !pattern.empty(); }
        void resetDefaultPattern() { pattern = "1;3;10"; }
      protected:
        std::string finalize();
        std::string toString() { return "V:" + version + ";T:" + toy + ";F:" + func + ";S:" + interval + ";M:" + hash + ";#\n" + pattern; }
        std::string _dump;
    };

    class LLQRCodeInfo
    {
      public:
          LLQRCodeInfo(){}
          const std::string& getContent() const { return mQrcodeContent; }
          const std::string& getUrl()const { return mQrcodeUrl; }
          const std::chrono::system_clock::time_point& getTime() { return mRequestTime; }
          
          void setContent(const std::string& content) { mQrcodeContent = content; }
          void setUrl(const std::string& url) { mQrcodeUrl = url; }
          void setTime(const  std::chrono::system_clock::time_point& time) { mRequestTime = time; }
          void clear() {
              mQrcodeContent.clear();
              mQrcodeUrl.clear();
          }
          bool valid() const{
              return !mQrcodeContent.empty();
          }
      private:
        std::string mQrcodeContent;
        std::string mQrcodeUrl;
        std::chrono::system_clock::time_point mRequestTime ;
    };

    class LLRemoteAPIParameter
    {
        public:
            const std::string& getAuthToken() const{ return mAuthToken; }
            const std::string& getSocketIoUrl() const { return mSocketIoUrl; }
            const std::map<std::string, std::string> &getSocketIoQuery() const{ return mSocketIoQuery; }

            void setAuthToken(const std::string& token) { mAuthToken = token; }
            void setSocketIoUrl(const std::string& url) { mSocketIoUrl = url; }
            void addQuery(const std::string& key, const std::string& value) { mSocketIoQuery.insert(std::make_pair(key,value)); }
            void clear() {
                mAuthToken.clear();
                mSocketIoUrl.clear();
                mSocketIoQuery.clear();
            }
            bool valid() const{
                return !mSocketIoUrl.empty();
            }
        private:
            std::string mAuthToken;
            std::string mSocketIoUrl;
            std::map<std::string, std::string> mSocketIoQuery;
    };

    class Until {
    public:
        static std::string generateUUID();
        static std::string md5String(const std::string &source);
    };
}

#endif
