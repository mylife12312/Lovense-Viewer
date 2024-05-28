/** 
 * @file llToyModel.cpp
 * @brief toy model base class implementation
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

#include "llToyModel.h"
#include <algorithm>
using namespace lovense;

LLToy::LLToy() {

}

LLToy::~LLToy() {

}

LLToyPtr LLToy::JsonDataToPtr(const neb::CJsonObject& toy) {
    std::string id, name, type, nickname, hVersion;
    int         fVersion = 0, battery = 0;
    bool        connected = false;
    toy.Get(kToyID, id);
    toy.Get(kToyName, name);
    toy.Get(kToyType, type);
    toy.Get(kToyNickName, nickname);
    toy.Get(kToyhVersion, hVersion);
    toy.Get(kToyfVersion, fVersion);
    toy.Get(kToyBattery, battery);
    toy.Get(kToyConnected, connected);

    LLToyPtr pToy = std::make_shared<LLToy>();
    pToy->toyId = id;
    pToy->toyName = name;
    pToy->toyType = type;
    pToy->toyNickName = nickname;
    pToy->battery = battery;
    pToy->connected = connected;
    pToy->hVersion = hVersion;
    return pToy;
}

LLToyPtr LLToy::LLSDToPtr(const LLSD& toy) {
    LLToyPtr pToy = std::make_shared<LLToy>();
    pToy->toyId = toy[kToyID].asString();
    pToy->toyName = toy[kToyName].asString();
    pToy->toyType = toy[kToyType].asString();
    pToy->toyNickName = toy[kToyNickName].asString();
    pToy->battery = toy[kToyBattery];
    pToy->connected = toy[kToyConnected];
    pToy->hVersion = toy[kToyhVersion].asString();
    return pToy;
}

LLSD LLToy::JsonDataToLLSD(const neb::CJsonObject& toy) {
    std::string id, name, type, nickname, hVersion;
    int         fVersion = 0, battery = 0;
    bool        connected = false;
    toy.Get(kToyID, id);
    toy.Get(kToyName, name);
    toy.Get(kToyType, type);
    toy.Get(kToyNickName, nickname);
    toy.Get(kToyhVersion, hVersion);
    toy.Get(kToyfVersion, fVersion);
    toy.Get(kToyBattery, battery);
    toy.Get(kToyConnected, connected);

    LLSD item;
    item[kToyID] = id;
    item[kToyName] = name;
    item[kToyType] = type;
    item[kToyNickName] = nickname;
    item[kToyhVersion] = hVersion;
    item[kToyfVersion] = fVersion;
    item[kToyBattery] = battery;
    item[kToyConnected] = connected;
    return item;
}



LLToyModel::LLToyModel() {

}

LLToyModel::~LLToyModel() {

}

void LLToyModel::AddToy(LLToyPtr toy) { 
    _toyList.push_back(toy); 
}

void LLToyModel::Update(LLToyPtr toy) {
    std::string toyId = toy->toyId;
    auto iter = std::find_if(_toyList.begin(), _toyList.end(), [toyId](const LLToyPtr &toy) -> bool { return toy->toyId == toyId; });
    if (iter != _toyList.end())
    {
        bool select = (*iter)->selected;
        *iter = toy;
        (*iter)->selected = select;
    }
    else
        AddToy(toy);
}

void LLToyModel::RemoveToy(const std::string toyId)
{
    auto iter = std::find_if(_toyList.begin(), _toyList.end(), [toyId](const LLToyPtr& toy) -> bool { 
        return toy->toyId == toyId;
    });
    if (iter != _toyList.end())
        _toyList.erase(iter);
}

size_t LLToyModel::GetToyCount() const { 
	return _toyList.size(); 
}

void LLToyModel::RemoveAll() { 
    return _toyList.clear(); 
}

 LLToyPtr LLToyModel::GetToy(int index) { 
    if (_toyList.size() <= index)
        return nullptr;

    auto iter = _toyList.begin();
    int  pos  = 0;
    while (iter != _toyList.end()) {
        if (pos == index)
            return *iter;
        ++iter;
        ++pos;
    }
    return nullptr;
}



LLToyPtr LLToyModel::GetToyWithId(const std::string toyId) { 
	for (auto item : _toyList) {
        if (item->toyId == toyId)
            return item;
	}
    return nullptr;
}

const std::list<LLToyPtr> &LLToyModel::GetAllToys() const { 
    return _toyList; 
}
