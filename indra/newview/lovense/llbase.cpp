#include "llbase.h"
#include <llmd5.h>
#include "remote_sdk/CJsonObject.hpp"
#include "llsdjson.h"
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

using namespace lovense;
static std::string Md5String(const std::string &src)
{
    LLMD5 md5;
    md5.update(src);
    md5.finalize();
    char temp[33] = {0};
    md5.hex_digest(temp);
    return std::string(temp);
}


std::string LLRemotePattern::finalize()
{
    hash  = DEF_DEFAULT_HASH;
    hash  = Md5String(this->toString());
    _dump = this->toString();
    return _dump;
}

std::string LLRemotePattern::dump() { 
    return finalize(); 
}

std::string LLRemotePattern::getStopPattern(std::string toyid) {
    neb::CJsonObject object;
    object.Add("command","Function");    
    object.Add("action", "Stop");
    object.Add("timeSec", 0);
    object.Add("apiVer", 2);
    if(!toyid.empty())
        object.Add("toy", toyid);
    return object.ToString();
}

std::string LLRemotePattern::getRunPattern(std::string pattern, std::string toyid) {
    //Json::Value temp;
    neb::CJsonObject object;
    {
        object.Add("command", "Pattern");
        object.Add("rule", "V:1;F:;S:100#");
        object.Add("strength", pattern);
        object.Add("timeSec", 0);
        if(!toyid.empty())
            object.Add("toy", toyid);
        object.Add("apiVer",2);
    }
    return object.ToString();
}


std::string Until::generateUUID() {
    LLUUID uid;
    uid.generate();
    return uid.asString();
}

std::string Until::md5String(const std::string& source) {
    return Md5String(source);
}
