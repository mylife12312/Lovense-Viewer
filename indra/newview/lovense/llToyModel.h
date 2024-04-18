/** 
 * @file llToyModel.h
 * @brief base class header file
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
#ifndef __LL_TOY_MODEL__H
#define __LL_TOY_MODEL__H

#include <string>
#include <memory>
#include <list>
#include "remote_sdk/CJsonObject.hpp"
#include "llsd.h"

namespace lovense
{
    class LLToy;
    using LLToyPtr = std::shared_ptr<lovense::LLToy>;

    static const char* kToyID = "id";
    static const char* kToyName = "name";
    static const char* kToyType = "toyType";
    static const char* kToyNickName = "nickname";
    static const char* kToyhVersion = "hVersion";
    static const char* kToyfVersion = "fVersion";
    static const char* kToyBattery = "battery";
    static const char* kToyConnected = "connected";

    class LLToy
    {
      public:
        LLToy();
        ~LLToy();
        static LLToyPtr JsonDataToPtr(const neb::CJsonObject& toy);
        static LLToyPtr LLSDToPtr(const LLSD &toy);
        static LLSD JsonDataToLLSD(const neb::CJsonObject &toy);

        std::string iconName;
        std::string toyName;
        std::string toyNickName;
        std::string toyId; //Mac Address
        std::string toyType;//type
        std::string hVersion;
        int         battery   = 0;
        bool        connected = false;
        bool        selected  = true;
    };


    class LLToyModel
    {
      public:
        LLToyModel();
        ~LLToyModel();

        void AddToy(LLToyPtr toy);
        void Update(LLToyPtr toy);
        void RemoveToy(const std::string toyId);
        void RemoveAll();
        size_t GetToyCount() const;
        LLToyPtr                 GetToy(int index);
        LLToyPtr GetToyWithId(const std::string toyId);
        const std::list<LLToyPtr> &GetAllToys() const;
       private:
        std::list<LLToyPtr> _toyList;
    };
}

#endif