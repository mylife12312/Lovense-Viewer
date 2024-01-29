#include "llLovenseAdapter.h"
#include "../llvoavatarself.h"
#include "CHttpClient.h"
#include <json/json.h>
#include <map>
#include <algorithm>
#include <fstream>
#include <streambuf>
#include "remote_sdk/llRemoteSocketIOProxy.hpp"
#include "llfloaterreg.h"
#include "llavatarname.h"
#include "llavatarnamecache.h"


#ifdef UPDATE_CAST_URL
    #define CHECK_FOR_UPDATE true
    #ifdef LL_WINDOWS
        #include "winsparkle.h"
    #else
        #include "llmacsparkle.h"
    #endif
#endif

using namespace lovense;

LLLovenseAdapter::LLLovenseAdapter() {

}

void LLLovenseAdapter::initAdapter(LLVOAvatarSelf *avatarSelf) { 
    mAvatarSelf = avatarSelf;
    LLAvatarName av_name;
    const LLUUID avid = mAvatarSelf->getID();
    if (LLAvatarNameCache::get(avid, &av_name)) {
        lovense::LLRemoteSocketIOProxy::instance().init(avid.asString(), Until::generateUUID());
    }
    else {
        lovense::LLRemoteSocketIOProxy::instance().init(Until::generateUUID(),Until::generateUUID());
    }
}


void LLLovenseAdapter::onAppStart() {
    initUserDir();
#if CHECK_FOR_UPDATE
    #ifdef LL_WINDOWS
        std::string language = LLUI::getLanguage();
        win_sparkle_set_lang(language.c_str());
        win_sparkle_init();
        win_sparkle_set_appcast_url(UPDATE_CAST_URL);
        win_sparkle_set_automatic_check_for_updates(0);
        //win_sparkle_check_update_with_ui();
        win_sparkle_check_update_without_ui();
    #else
        lovense::mac::onInitSparkle(UPDATE_CAST_URL);
        
    #endif
#endif
}

void LLLovenseAdapter::onAppExit() {
#if CHECK_FOR_UPDATE
    #ifdef LL_WINDOWS
        win_sparkle_cleanup();
    #else
        lovense::mac::onCleanSparkle();
    #endif
#endif
}

std::string LLLovenseAdapter::getUserAccountName() {
    if (mAvatarSelf) {
        LLAvatarName av_name;
        if (LLAvatarNameCache::get(mAvatarSelf->getID(), &av_name)) {
            return av_name.getAccountName();
        }
    }
    return "";
}


void LLLovenseAdapter::initUserDir() {
    const std::string& dir = gDirUtilp->buildSLOSCacheDir();
    if (!LLFile::isdir(dir)) {
        LLFile::mkdir(dir);
    }

    if (LLFile::isdir(dir)) {
        std::string lovense = dir + "/lovense";
        if (!LLFile::isdir(lovense))
            LLFile::mkdir(lovense);

        std::string cache = dir + "/cache";
        if (!LLFile::isdir(cache))
            LLFile::mkdir(cache);
    }
}

void LLLovenseAdapter::onCheckForUpdates() {
#if CHECK_FOR_UPDATE
#ifdef LL_WINDOWS
    std::string language = LLUI::getLanguage();
    win_sparkle_set_lang(language.c_str());
    win_sparkle_init();
    win_sparkle_set_appcast_url(UPDATE_CAST_URL);
    win_sparkle_set_automatic_check_for_updates(0);
    win_sparkle_check_update_with_ui();
#else
    lovense::mac::onInitSparkle(UPDATE_CAST_URL,true);

#endif
#endif
}


