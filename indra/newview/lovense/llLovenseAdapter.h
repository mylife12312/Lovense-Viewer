#ifndef LOVENSE_ADAPTER_H
#define LOVENSE_ADAPTER_H
#include "lluuid.h"
#include <list>
#include "llbase.h"
#include "lleventdispatcher.h"

class LLVOAvatarSelf;
namespace lovense
{

    static const char* kProductName = "Lovense Viewer";
    static const char* kProductShorName = "Lovense Viewer";
    static const char* kProductDataDir = "SLLovenseViewer";
    //static const char* kWinClassName = "Lovense Viewer Win";
    static const char* kWinClassName = "Second Life";



    class LLLovenseAdapter:public LLSingleton<LLLovenseAdapter>
    {
        LLSINGLETON(LLLovenseAdapter);

     public:
        void initAdapter(LLVOAvatarSelf *avatarSelf);
        void onAppStart();
        void onAppExit();
        std::string getUserAccountName();

    public:
        static void onCheckForUpdates();
     protected:
        //init config
        void initUserDir();
     private:
        LLVOAvatarSelf  *mAvatarSelf = nullptr;
    };
}





#endif
