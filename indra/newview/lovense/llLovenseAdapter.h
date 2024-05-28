
/** 
 * @file llLovenseAdapter.h
 * @brief LLLovenseAdapter class header file
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
