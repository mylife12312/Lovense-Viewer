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