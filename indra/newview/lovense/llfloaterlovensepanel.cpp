/** 
 * @file llfloatlovensepanel.cpp
 * @brief LLFloaterLovensePanel class implementation
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

#include "llfloaterlovensepanel.h"
#include "llbutton.h"
#include "lliconctrl.h"
#include "llloadingindicator.h"
#include "../llviewercontrol.h"
#include "../llviewertexturelist.h"
#include "../llviewertexture.h"
#include "llimagejpeg.h"
#include "llimagepng.h"
#include "llfilesystem.h"
#include "remote_sdk/llRemoteSocketIOProxy.hpp"
#include "QrCode.hpp"
#include "llmainthreadtask.h"
#include "QrToPng.h"
#include "TinyPngOut.hpp"
#include "llurlaction.h"
#include "llToyModel.h"
#include "lltrans.h"

#define MAX_ROWS 2
#define MAX_COLUMN 4

static const char *kCtrlButtonToggleName = "button_toggle";
static const char *kCtrlIconSelectName = "icon_select";
static const char *kCtrlTextToyName = "text_toy_name";
static const char *kCtrlIconBatteryName = "icon_battery";
static const char *kSkipNextTimeKey = "lvsSkipNextTime";

LLFloaterLovensePanel::LLFloaterLovensePanel(const LLSD &key) : LLFloater(key)
{   
    mCommitCallbackRegistrar.add("onClickDisconnectBtn", boost::bind(&LLFloaterLovensePanel::onDisconnectButtonClick, this));
    mCommitCallbackRegistrar.add("onClickSkipStepBtn", boost::bind(&LLFloaterLovensePanel::onSkipThisStepButtonClick, this));
    mCommitCallbackRegistrar.add("ClickVisitStoreAuction", boost::bind(&LLFloaterLovensePanel::onVisitStoreButtonClick, this));
    mCommitCallbackRegistrar.add("ClickReloadQrcode", boost::bind(&LLFloaterLovensePanel::onReloadQrcodeButtonClick, this));

    
    LLEventDispatcher &remoteEventDispatcher = lovense::LLRemoteSocketIOProxy::instance().getEventDispatcher();
    remoteEventDispatcher.add(lovense::kSocketIODispatcher, "",boost::bind(&LLFloaterLovensePanel::onSocketIOEventChange,this,_1));
}

LLFloaterLovensePanel::~LLFloaterLovensePanel() { 

    pauseToySync();
    btn_back_connection_.disconnect();
    btn_next_connection_.disconnect();
    btn_pause_connection_.disconnect();
    LLEventDispatcher &remoteEventDispatcher = lovense::LLRemoteSocketIOProxy::instance().getEventDispatcher();
    remoteEventDispatcher.remove(lovense::kSocketIODispatcher);

    toyList_.clear();
}

BOOL LLFloaterLovensePanel::postBuild() {

   main_tab_container_ =  this->findChild<LLTabContainer>("lvs_main_tabctrl");
   step_tab_container_ = this->findChild<LLTabContainer>("lvs_step_tap_ctrl");
   icon_qrcode_ctrl_   = this->findChild<LLIconCtrl>("lvs_icon_qrcode_ctrl");
   btn_back_ctrl       = this->findChild<LLButton>("lvs_ctp_back_btn");
   btn_next_ctrl       = this->findChild<LLButton>("lvs_ctp_next_btn");
   btn_skip_next_time_     = this->findChild<LLCheckBoxCtrl>("lvs_skip_next_btn");
   layout_toy_list_panel_  = this->findChild<LLLayoutPanel>("lvs_toy_list_panel");
   btn_pause_sync_         = this->findChild<LLButton>("lvs_pause_sync_btn");
   text_box_no_toy_detect_ = this->findChild<LLTextBox>("lvs_no_toy_detect_text");
   lvs_guid_text_box_      = this->findChild<LLTextBox>("lvs_guid_text_box");
   panel_reload_qrcode_    = this->findChild<LLPanel>("lvs_reload_qrcode_panel");

    
   if (lvs_guid_text_box_) {
       lvs_guid_text_box_->SetNotLinkUrlAction(true);
       lvs_guid_text_box_->setURLClickedCallback(boost::bind(&LLFloaterLovensePanel::onGuideHtmlClick,this));
       lvs_guid_text_box_->setToolTip(std::string(""));
   }

   //init toy list panel
   if (layout_toy_list_panel_) {
        for (int i = 0; i < MAX_ROWS; i++)
        {
            LLLayoutStack *panel = LLUICtrlFactory::createFromFile<LLLayoutStack>("floater_lovense_layout_panel.xml",
                                                                                  layout_toy_list_panel_,
                                                                                  LLLayoutPanel::child_registry_t::instance());
            if (panel)
                layout_toy_list_panel_->addChild(panel);
            rows_stack_.push_back(panel);
        }
   }
   //gSavedSettings.setBOOL(kSkipNextTimeKey, false);
  /* std::vector<lovense::LLToyPtr> list;
   srand(time(0));
   for (int i = 0; i < MAX_ROWS * MAX_COLUMN ; i++) {
        lovense::LLToyPtr item = std::make_shared<lovense::LLToy>();
        item->toyNickName      = "Lush" + std::to_string(i);
        item->battery          = rand() % 100;
        item->connected        = rand() % 2;
        list.push_back(item);
   }

   int count  = list.size();
   int row    = 0;
   int column = 0;
   for (int i = 0; i < count; i++)
   {
        const lovense::LLToyPtr &pToy = list.at(i);
        row                           = i / MAX_COLUMN;
        column                        = i % MAX_COLUMN;
        SetData(GetItem(row, column), pToy);
        printf("%d%d%d",i,row,column);

        LL_INFOS() << i << " " << row << " " << column << LL_ENDL;
   }*/
   initUIText();
   onInitUI();
   onRefreshQRCode();
   return true;
}

void LLFloaterLovensePanel::initUIText() {

    this->findChild<LLTextBase>("lvs_remote_scan_dec_text")->setText(LLTrans::getString("lstr_scan_qr_code_des"));
    this->findChild<LLTextBase>("lvs_reload_qrcode_text")->setText(LLTrans::getString("lstr_reload_qrcode"));
    this->findChild<LLTextBase>("text_click_download")->setText(LLTrans::getString("lstr_click_download"));
    this->findChild<LLTextBase>("text_now_toy_support")->setText(LLTrans::getString("lstr_now_with_toy_support"));

    this->findChild<LLTextBase>("text_click_download")->setText(LLTrans::getString("lstr_click_download"));
    this->findChild<LLButton>("btn_visit_store")->setLabel(LLTrans::getString("lstr_visit_store"));
    this->findChild<LLTextBase>("text_connected")->setText(LLTrans::getString("lstr_connected"));
    this->findChild<LLTextBase>("text_how_to_get_start")->setText(LLTrans::getString("lstr_how_to_get_start"));

    this->findChild<LLTextBase>("text_step1")->setText(LLTrans::getString("lstr_step1_des"));
    this->findChild<LLTextBase>("text_step2")->setText(LLTrans::getString("lstr_step2_des"));
    this->findChild<LLTextBase>("text_step3")->setText(LLTrans::getString("lstr_step3_des"));

    this->findChild<LLTextBase>("text_skip_next_time")->setText(LLTrans::getString("lstr_skip_next_time"));
    this->findChild<LLTextBase>("text_toy_connect_remote")->setText(LLTrans::getString("lstr_toy_connect_remote"));
    this->findChild<LLTextBase>("text_select_toy_control")->setText(LLTrans::getString("lstr_select_toy_control"));

    this->findChild<LLButton>("btn_disconnect")->setLabel(LLTrans::getString("lstr_disconnect"));
    this->findChild<LLTextBase>("lvs_guid_text_box")->setText(LLTrans::getString("lstr_guid"));
    this->findChild<LLButton>("lvs_ctp_back_btn")->setLabel(LLTrans::getString("lstr_back"));
    
}

void LLFloaterLovensePanel::onInitUI() { 
       if (btn_pause_sync_) {
           btn_pause_connection_.disconnect();
           btn_pause_sync_->setLabel(LLTrans::getString("lstr_pause_sync"));
           btn_pause_sync_->setImageUnselected(LLUI::getUIImage("Icon_btn_normal"));
           btn_pause_sync_->setImagePressed(LLUI::getUIImage("Icon_btn_normal"));
           btn_pause_sync_->setImageOverlay("Icon_overlay_pause", LLFontGL::LEFT, LLColor4(1, 1, 1, 1));
           btn_pause_connection_ = btn_pause_sync_->setClickedCallback(boost::bind(&LLFloaterLovensePanel::onPauseSyncButtonClick, this));
       }
       toyList_.clear();
       syncPausing_ = false;
}

void LLFloaterLovensePanel::onRefreshQRCode() {
    const lovense::LLQRCodeInfo& cache = lovense::LLRemoteSocketIOProxy::instance().getQRCodeCache();
    if (cache.valid() && 0) {
        showQRCode(cache.getContent());
        lovense::LLRemoteSocketIOProxy::instance().tryConnectServer(true);
    }
    else {
        lovense::LLRemoteSocketIOProxy::instance().tryConnectServer(true);
        setQRcodeLoadingState(true);
    }
}



void LLFloaterLovensePanel::onOpen(const LLSD &key) { 
    LLFloater::onOpen(key); 
}

void LLFloaterLovensePanel::onClose(bool app_quitting) { 
    //pauseToySync();
    //btn_back_connection_.disconnect();
    //btn_next_connection_.disconnect();
    //btn_pause_connection_.disconnect();
    LLFloater::onClose(app_quitting);
}


void LLFloaterLovensePanel::showQRCode(const std::string& qrcontent)
{
    setQRcodeLoadingState(false);
    if (!icon_qrcode_ctrl_)
        return;
   
    if (!qrcontent.empty()) {
        std::string file = LLDiskCache::instance().metaDataToFilepath("qr_code_png", LLAssetType::AT_IMAGE_JPEG, "lovense");
        int  imgSize = 256;
        int  minModulePixelSize = 3;
        auto exampleQrPng1 = QrToPng(file, imgSize, minModulePixelSize, qrcontent, true, qrcodegen::QrCode::Ecc::MEDIUM);
        bool write = exampleQrPng1.writeToPNG();
        if (write)
        {
            LLPointer<LLImagePNG> jpeg_image = new LLImagePNG;
            LLPointer<LLImageRaw> raw_image = new LLImageRaw();
            if (jpeg_image->load(file) && jpeg_image->decode(raw_image, 0.0f))
            {
                raw_image->scale(256, 256);
                LLPointer<LLViewerTexture> texture = new LLViewerTexture(raw_image, false);
                LLUIImagePtr image = new LLUIImage("qrcode_icon_256", texture);
                icon_qrcode_ctrl_->setImage(image);
                qrcode_show_success_ = true;
                setQRcodeLoadingResult(true);
                return;
            }
        }
    }
   LL_INFOS() << "LOVNESE: write png failed!" << LL_ENDL;
   setQRcodeLoadingResult(false);
}

// Toy connected page
void LLFloaterLovensePanel::onPauseSyncButtonClick(){
    if (btn_pause_sync_)
    {
        std::string label = " ";
        label += LLTrans::getString("lstr_restart_sync");
        btn_pause_sync_->setLabel(label);
        btn_pause_sync_->setImageUnselected(LLUI::getUIImage("Icon_btn_normal"));
        btn_pause_sync_->setImagePressed(LLUI::getUIImage("Icon_btn_normal"));
        btn_pause_sync_->setImageOverlay("Icon_overlay_start",LLFontGL::LEFT,LLColor4(1,1,1,1));
        btn_pause_connection_.disconnect();
        btn_pause_connection_ =
            btn_pause_sync_->setClickedCallback(boost::bind(&LLFloaterLovensePanel::onRestartSyncButtonClick, this));
    }
    syncPausing_ = true;
    pauseToySync();
}

void LLFloaterLovensePanel::onRestartSyncButtonClick(){
    if (btn_pause_sync_)
    {
        btn_pause_connection_.disconnect();
        btn_pause_sync_->setLabel(LLTrans::getString("lstr_pause_sync"));
        btn_pause_sync_->setImageUnselected(LLUI::getUIImage("Icon_btn_normal"));
        btn_pause_sync_->setImagePressed(LLUI::getUIImage("Icon_btn_normal"));
        btn_pause_sync_->setImageOverlay("Icon_overlay_pause",LLFontGL::LEFT,LLColor4(1,1,1,1));
        btn_pause_connection_ = btn_pause_sync_->setClickedCallback(boost::bind(&LLFloaterLovensePanel::onPauseSyncButtonClick, this));
    }
    syncPausing_ = false;
    restarToySync();
}

void LLFloaterLovensePanel::onDisconnectButtonClick() {
    pauseToySync();
    onInitUI();
    selectMainTabPage(SELECT_SCAN_QRCODE_PAGE); 
    lovense::LLRemoteSocketIOProxy::instance().tryConnectServer(true);
    LLPanel* panel = this->findChild<LLPanel>("lvs_reload_qrcode_panel");
    if (panel) {
        panel->setVisible(true);
    }
    setQRcodeLoadingState(true);
}

void LLFloaterLovensePanel::onSkipThisStepButtonClick() { 
    if (btn_skip_next_time_)
        gSavedSettings.setBOOL(kSkipNextTimeKey,btn_skip_next_time_->get());
}

void LLFloaterLovensePanel::onVisitStoreButtonClick() {
    std::string store = "https://www.lovense.com/p/2wu1Ii";
    LLUrlAction::openURL(store);
}

void LLFloaterLovensePanel::onReloadQrcodeButtonClick() {
    lovense::LLRemoteSocketIOProxy::instance().emitQrcodeCommand();
    setQRcodeLoadingState(true);
}

void LLFloaterLovensePanel::selectUserGuidTabPage(UserGuidStepSelected selected) {

    if (!step_tab_container_)
        return;
    std::string pageName = "lvs_step1_tap_panel";
    switch (selected)
    {
    case LLFloaterLovensePanel::SELECT_FIRST_STEP_PAGE:
        pageName = "lvs_step1_tap_panel";
        btn_next_ctrl->setLabel(LLTrans::getString("lstr_next"));
        btn_back_ctrl->setEnabled(false);
        btn_next_connection_.disconnect();
        btn_next_connection_ = btn_next_ctrl->setClickedCallback(boost::bind(&LLFloaterLovensePanel::selectUserGuidTabPage, this, SELECT_SECOND_STEP_PAGE));
        break;
    case LLFloaterLovensePanel::SELECT_SECOND_STEP_PAGE:
        pageName = "lvs_step2_tap_panel";
        btn_next_ctrl->setLabel(LLTrans::getString("lstr_next"));

        btn_back_ctrl->setEnabled(true);
        btn_next_connection_.disconnect();
        btn_back_connection_.disconnect();

        btn_back_connection_ = btn_back_ctrl->setClickedCallback(boost::bind(&LLFloaterLovensePanel::selectUserGuidTabPage, this,SELECT_FIRST_STEP_PAGE));
        btn_next_connection_ = btn_next_ctrl->setClickedCallback(boost::bind(&LLFloaterLovensePanel::selectUserGuidTabPage, this, SELECT_THIRD_STEP_PAGE));
        break;
    case LLFloaterLovensePanel::SELECT_THIRD_STEP_PAGE:
        pageName = "lvs_step3_tap_panel";
        btn_next_ctrl->setLabel(LLTrans::getString("lstr_got_it"));
        btn_back_ctrl->setEnabled(true);
        btn_next_connection_.disconnect();
        btn_back_connection_.disconnect();

        btn_back_connection_ = btn_back_ctrl->setClickedCallback(boost::bind(&LLFloaterLovensePanel::selectUserGuidTabPage, this, SELECT_SECOND_STEP_PAGE));
        btn_next_connection_ = btn_next_ctrl->setClickedCallback(boost::bind(&LLFloaterLovensePanel::selectMainTabPage, this, SELECT_TOY_LIST_PAGE));

        break;
    default:
        break;
    }

    step_tab_container_->selectTabByName(pageName);
}


  // Function main tab panel exchange
void LLFloaterLovensePanel::selectMainTabPage(MainTabUISelected selected) {
    if (!main_tab_container_)
        return;

    if (mainTabPageSelect_ == selected)
        return;
    std::string pageName = "lvs_qrcode_tab_panel";
    switch (selected)
    {
    case LLFloaterLovensePanel::SELECT_SCAN_QRCODE_PAGE:
        pageName = "lvs_qrcode_tab_panel";
        break;
    case LLFloaterLovensePanel::SELECT_USER_GUIDE_PAGE:
        pageName = "lvs_connected_tab_panel";
        selectUserGuidTabPage(SELECT_FIRST_STEP_PAGE);
        break;
    case LLFloaterLovensePanel::SELECT_TOY_LIST_PAGE:
        pageName = "lvs_toy_tab_panel";
        gSavedSettings.setBOOL(kSkipNextTimeKey, btn_skip_next_time_->get());
        break;
    default:
        break;
    }
    mainTabPageSelect_ = selected;
    main_tab_container_->selectTabByName(pageName);
    if (mainTabPageSelect_ == LLFloaterLovensePanel::SELECT_TOY_LIST_PAGE) {
        restarToySync();
    }
}

void LLFloaterLovensePanel::onSocketIOEventChange(const LLSD& event) {
    LLSD eventTemp = event;
    LLMainThreadTask::dispatch([&,eventTemp](){
        this->doSocketIOEvent(eventTemp);
    });
}


void LLFloaterLovensePanel::doSocketIOEvent(LLSD event) {

    lovense::SocketIOEventType type = (lovense::SocketIOEventType)event[lovense::kSocketIOEventType].asInteger();
    switch (type)
    {
        case lovense::SocketIOEventType::Event_QR_Code_Update: {
            std::string qrcontent = event[lovense::kQrcodeContent].asString();
            showQRCode(qrcontent);
        }break;
        case lovense::SocketIOEventType::Event_Device_Info_Update: {

            const LLSD& arrays = event[lovense::kDevcesArrays];
            if (arrays.isArray()) {
                bool toyReload = toyList_.empty() ? true : false;
                int size = arrays.size();
                if (size > MAX_ROWS * MAX_COLUMN) {
                    size = MAX_ROWS * MAX_COLUMN;
                }

                std::vector<lovense::LLToyPtr> cache;
                //Add connected toy
                for (int i = 0; i < size; i++) {
                    const LLSD& item = arrays[i];
                    lovense::LLToyPtr toyPtr = lovense::LLToy::LLSDToPtr(item);
                    if (toyPtr->connected) {
                        cache.push_back(toyPtr);
                        auto iter = std::find_if(toyList_.begin(), toyList_.end(), [toyPtr](auto child) {
                            return toyPtr->toyId == child->toyId;
                        });
                        if (iter == toyList_.end()) {
                            toyPtr->selected = true;
                            toyList_.push_back(toyPtr);
                            //Sync one toy with current motion
                            startToySync(toyPtr->toyId);
                        }
                        else {
                            //update battery value
                            if (toyPtr->battery > 0)
                                (*iter)->battery = toyPtr->battery;
                            (*iter)->connected = toyPtr->connected;
                            if ((*iter)->selected)
                                startToySync(toyPtr->toyId);
                        }
                    }
                }

                //Remove disconnected toy
                std::vector<std::string> toyIDS;
                for (int i = 0; i < toyList_.size(); i++) {
                    auto item = toyList_[i];
                    auto iter = std::find_if(cache.begin(), cache.end(), [item](auto child) {
                        return item->toyId == child->toyId;
                    });
                    if (iter == cache.end())
                        toyIDS.push_back(item->toyId);
                }

                for (auto item : toyIDS) {
                    auto iter = std::find_if(toyList_.begin(), toyList_.end(), [item](auto child) {
                        return item == child->toyId;
                    });
                    if (iter != toyList_.end())
                        toyList_.erase(iter);
                }
               
                if (size == 0)
                    toyList_.clear();
                
                updateToyListCtrl();
                if (toyReload && toyList_.size() > 0)
                    restarToySync();
            }
        }break;
        case lovense::SocketIOEventType::Event_App_Online_Update:
        case lovense::SocketIOEventType::Event_App_Status_Update: {
            int status = event[lovense::kAppChangeStatus];
            if (status == 1) {
                bool skip = gSavedSettings.getBOOL(kSkipNextTimeKey);
                if (!skip) {
                    this->selectMainTabPage(SELECT_USER_GUIDE_PAGE);
                }
                else {
                    if(btn_skip_next_time_->get())
                        this->selectMainTabPage(SELECT_TOY_LIST_PAGE);
                    else
                        this->selectMainTabPage(SELECT_USER_GUIDE_PAGE);
                }
            }
            else {
                this->selectMainTabPage(SELECT_SCAN_QRCODE_PAGE);
                if (type == lovense::SocketIOEventType::Event_App_Status_Update) {
                    onInitUI();
                }
            }
        }break;
        default:
            break;
    }
}

void LLFloaterLovensePanel::hideToyListCtrl() {
    for (int i = 0; i < MAX_ROWS; i++) {
        for (int j = 0; j < MAX_COLUMN; j++) {
            LLLayoutPanel* panel = getItem(i, j);
            if (panel && panel->getVisible())
                panel->setVisible(false);
        }
    }
}


void LLFloaterLovensePanel::updateToyListCtrl() {

    hideToyListCtrl();
    
    int                        row   = 0;
    int                        column = 0;
    std::vector<lovense::LLToyPtr> connectedList;
    for (int i = 0; i < toyList_.size(); i++) {
        lovense::LLToyPtr  pToy = toyList_[i];
        if (pToy->connected)
            connectedList.push_back(pToy);
        row                  = i / MAX_COLUMN;
        column               = i % MAX_COLUMN;
        LLLayoutPanel *panel = getItem(row, column);
        if (panel) {
            panel->setVisible(false);
        }
    }
    for (int i = 0; i < connectedList.size(); i++) {
        lovense::LLToyPtr pToy = connectedList.at(i);
        row    = i / MAX_COLUMN;
        column = i % MAX_COLUMN;
        LLLayoutPanel *panel   = getItem(row, column);
        if (panel) {
            setData(panel, pToy);
            panel->setVisible(true);
        }
    }
    text_box_no_toy_detect_->setVisible(connectedList.size()>0?false:true);
}

void LLFloaterLovensePanel::setData(LLLayoutPanel *panel, const lovense::LLToyPtr &pToy) { 
 
    auto iter = panel->beginChild();
    LLButton *button = this->findChild<LLButton>(kCtrlButtonToggleName);
    LLTextBox *text_toy_name = this->findChild<LLTextBox>(kCtrlTextToyName);
    LLIconCtrl *icon_select   = this->findChild<LLIconCtrl>(kCtrlIconSelectName);
    LLIconCtrl *icon_battery   = this->findChild<LLIconCtrl>(kCtrlIconBatteryName);

    while (iter != panel->endChild()) {
        std::string name = (*iter)->getName();
        if (name == kCtrlButtonToggleName) {
            button = static_cast<LLButton *>(*iter);

        }
        else if (name == kCtrlIconBatteryName) {
            icon_battery = static_cast<LLIconCtrl *>(*iter);
        }
        else if (name == kCtrlIconSelectName) {
            icon_select = static_cast<LLIconCtrl *>(*iter);

        }
        else if (name == kCtrlTextToyName) {
            text_toy_name = static_cast<LLTextBox *>(*iter);
        }
        ++iter;
    }
    button->setClickedCallback(boost::bind(&LLFloaterLovensePanel::onItemButtonClick, this, button, icon_select,pToy->toyId));
    icon_select->setVisible(pToy->selected);

    if (!pToy->toyNickName.empty())
        text_toy_name->setText(pToy->toyNickName);
    else
        text_toy_name->setText(pToy->toyName);

    std::string battery_icon_name;
    int         battery = pToy->battery;
    if (battery < 0 || battery > 100) {
        battery_icon_name = "Icon_battery_100";
    }
    else if (battery == 0)
        battery_icon_name = "Icon_battery_0";
    else if (battery == 1) {
        battery_icon_name = "Icon_battery_1";
    }
    else if (battery > 1 && battery < 10) {
        battery_icon_name = "Icon_battery_10";
    }
    else {
        int select = battery / 10;
        char buff[60] = {0};
#ifdef _WIN32
        sprintf_s(buff, sizeof(buff), "Icon_battery_%d0", select);
#else
        sprintf(buff, "Icon_battery_%d0", select);
#endif
        battery_icon_name = buff;
    }
   
    LLUIImagePtr image = LLUI::getUIImage(battery_icon_name);
    if (image)
        icon_battery->setImage(image);

    //Set Toy icon
    {
        char buff[60] = {0};
        std::string toyType  = pToy->toyType.c_str();
        if (!toyType.empty())
            toyType[0] = std::toupper(toyType[0]);

        std::string hv = pToy->hVersion;
        if (hv.empty())
            hv = "1";

        //Test
        //toyType = "Vulse";
        //hv = "1";

#ifdef _WIN32
        sprintf_s(buff, sizeof(buff), "Icon_%s_%s", toyType.c_str(), hv.c_str());
#else
        sprintf(buff, "Icon_%s_%s",toyType.c_str(), hv.c_str());
#endif
        LL_INFOS() << buff << LL_ENDL;
        LLUIImagePtr image = LLUI::getUIImage(buff);
        if (image->getTextureHeight() == 1) {
         //TODO:a good image
        }
        else {
            image = LLUI::getUIImage("Icon_Unknown_toy");
        }
        button->setImageUnselected(image);
        button->setImagePressed(image);
    }
    panel->setVisible(pToy->connected);
}

void LLFloaterLovensePanel::onItemButtonClick(LLButton *toyBtn, LLIconCtrl *selectIcon, std::string toyID)
{ 
    auto iter = std::find_if(toyList_.begin(), toyList_.end(), [toyID](auto child) {
        return toyID == child->toyId;
    });
    if (iter == toyList_.end())
        return;

    (*iter)->selected = !(*iter)->selected;
    selectIcon->setVisible((*iter)->selected);
    if ((*iter)->selected) {
        startToySync((*iter)->toyId);
    }
    else {
        stopToySync((*iter)->toyId);
    }
}

LLLayoutStack *LLFloaterLovensePanel::getRows(int row) const { 

    return rows_stack_.at(row);
    LLLayoutStack *stack = nullptr;
    do
    {
        if (!layout_toy_list_panel_)
            break;
        size_t count = layout_toy_list_panel_->getChildCount();
        if (row >= MAX_ROWS || row >=count)
            break;
        auto iter = layout_toy_list_panel_->beginChild();
        int pos = 0;
        while (iter != layout_toy_list_panel_->endChild()) {

            if (pos++ == row) {
                stack = static_cast<LLLayoutStack*>(*iter);
                break;
            }
            ++iter;
        }
    } while (0);
    return stack;
}

LLLayoutPanel *LLFloaterLovensePanel::getItem(int row, int column) const { 
    LLLayoutPanel *panel = nullptr; 
    do
    {

        if (row >= MAX_ROWS || column >= MAX_COLUMN)
            break;
        LLLayoutStack *stack = getRows(row);
        if (!stack)
            break;

        if (column >= stack->getChildCount()) {
            break;
        }

        
        auto iter = stack->beginChild();
        std::vector<LLLayoutPanel *> tempVt;
        while (iter != stack->endChild()) {
            
            panel = dynamic_cast<LLLayoutPanel *>(*iter);
            if (panel)
                tempVt.insert(tempVt.begin(),panel);
            ++iter;
        }
        panel = tempVt.at(column);

    } while (0);
    return panel;
}

void LLFloaterLovensePanel::onGuideHtmlClick() { 
    selectMainTabPage(SELECT_USER_GUIDE_PAGE); 
}

void LLFloaterLovensePanel::setQRcodeLoadingState(bool start) {
    LLLoadingIndicator* indicator = this->findChild<LLLoadingIndicator>("lvs_refresh_qrcode_indicator");
    if (indicator) {
        indicator->setVisible(start);
    }

    LLButton* btn = this->findChild<LLButton>("lvs_reload_qrcode_btn");
    if (btn) {
        btn->setVisible(!start);
    }

    LLTextBox* text = this->findChild<LLTextBox>("lvs_reload_qrcode_text");
    if (text) {
        text->setVisible(!start);
    }
}

void LLFloaterLovensePanel::setQRcodeLoadingResult(bool success) {
    LLPanel *panel = this->findChild<LLPanel>("lvs_reload_qrcode_panel");
    if (panel) {
        panel->setVisible(!success);
    }
    LLTextBox* text = this->findChild<LLTextBox>("lvs_remote_scan_dec_text");
    if (text) {
        std::string textContent = success ? LLTrans::getString("lstr_scan_qr_code_des") : LLTrans::getString("lstr_qr_code_failed_des");
        text->setText(textContent);
        LLColor4 color;
        color = success ? LLColor4(1, 1, 1, 1) : LLColor4(1,0,0,1);
        text->setColor(color);
    }
    LLLoadingIndicator* indicator = this->findChild<LLLoadingIndicator>("lvs_refresh_qrcode_indicator");
    if (indicator) {
        indicator->setVisible(false);
    }
}


void LLFloaterLovensePanel::pauseToySync() {
    std::string cmd = lovense::LLRemotePattern::getStopPattern();
    lovense::LLRemoteSocketIOProxy::instance().emitToyCommand(cmd);
}

void LLFloaterLovensePanel::restarToySync() {

    if (!connectAndToyReady())
        return;
    std::string pattern = "1;2;4;6;8;10;15;4";
    int selectCount = 0;
    for (auto item : toyList_) {
        if (item->selected)
            selectCount++;
    }
    std::string cmd;
    if (selectCount == toyList_.size()) {
        cmd = lovense::LLRemotePattern::getRunPattern(pattern);
        lovense::LLRemoteSocketIOProxy::instance().emitToyCommand(cmd);
    }
    else {
        for (auto item : toyList_) {
            if (item->selected) {
                cmd = lovense::LLRemotePattern::getRunPattern(pattern, item->toyId);
                lovense::LLRemoteSocketIOProxy::instance().emitToyCommand(cmd);
            }
        }
    }
}

void LLFloaterLovensePanel::stopToySync(const std::string& toyID) {
    std::string cmd = lovense::LLRemotePattern::getStopPattern(toyID);
    lovense::LLRemoteSocketIOProxy::instance().emitToyCommand(cmd);
}

void LLFloaterLovensePanel::startToySync(const std::string& toyID) {

    if (connectAndToyReady() && !toyID.empty()) {
        std::string pattern = "1;2;4;6;8;10;15;4";
        std::string cmd = lovense::LLRemotePattern::getRunPattern(pattern,toyID);
        lovense::LLRemoteSocketIOProxy::instance().emitToyCommand(cmd);
    }
}

bool LLFloaterLovensePanel::connectAndToyReady() {

    bool ready = false;
    LL_INFOS() <<"(LOVENSE)=>:Ready:"<< mainTabPageSelect_ << !toyList_.empty() << !syncPausing_ << LL_ENDL;

    if (mainTabPageSelect_ == SELECT_TOY_LIST_PAGE && !toyList_.empty() && !syncPausing_)
        ready = true;
    return ready;
}

