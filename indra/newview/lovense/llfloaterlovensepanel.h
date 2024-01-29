#ifndef LL_FLOAT_LOVENSE_PENEL_H
#define LL_FLOAT_LOVENSE_PENEL_H

#include "llfloater.h"
#include "../llmediactrl.h"
#include "llcheckboxctrl.h"
#include "llbase.h"
#include "lltabcontainer.h"
#include "llToyModel.h"
#include "llloadingindicator.h"
#include "llToyModel.h"

class LLFloaterLovensePanel : public LLFloater
{

    enum MainTabUISelected {
       SELECT_SCAN_QRCODE_PAGE,
       SELECT_USER_GUIDE_PAGE,
       SELECT_TOY_LIST_PAGE,
    };

    enum UserGuidStepSelected {
        SELECT_FIRST_STEP_PAGE,
        SELECT_SECOND_STEP_PAGE,
        SELECT_THIRD_STEP_PAGE,
    };

  public:
    LLFloaterLovensePanel(const LLSD &key);
    virtual ~LLFloaterLovensePanel();
    /*virtual*/ BOOL postBuild();
    virtual void     onOpen(const LLSD &key);
    virtual void     onClose(bool app_quitting);

    void showQRCode(const std::string &qrcontent);
    void onInitUI();
    void onRefreshQRCode();

  public:
    //Select main tab panel
     void selectMainTabPage(MainTabUISelected selected);
    //Select user guid tab page
    void selectUserGuidTabPage(UserGuidStepSelected selected);

    //Toy connected page button event
    void onPauseSyncButtonClick();
    void onRestartSyncButtonClick();
    void onDisconnectButtonClick();
    void onSkipThisStepButtonClick();
    void onVisitStoreButtonClick();
    void onReloadQrcodeButtonClick();
    void onGuideHtmlClick();


    //From Dispatcher: On Remote Socket io event	
    void onSocketIOEventChange(const LLSD &event);

    //do event in ui thread
    void doSocketIOEvent(LLSD event);

 protected:
     bool connectAndToyReady();
     void pauseToySync();
     void restarToySync();
     void stopToySync(const std::string &toyID);
     void startToySync(const std::string& toyID);
     void initUIText();

 protected:
    LLLayoutStack *getRows(int row) const;
    LLLayoutPanel *getItem(int row, int column)const;
    void           setData(LLLayoutPanel *panel, const lovense::LLToyPtr &pToy);
    void           onItemButtonClick(LLButton *toyBtn, LLIconCtrl *selectIcon, std::string toyID);
    void           hideToyListCtrl();
    void           updateToyListCtrl();
    void           setQRcodeLoadingState(bool starting);
    void           setQRcodeLoadingResult(bool success);
    

  private:
    LLTabContainer                         *main_tab_container_ = nullptr;
    LLTabContainer                         *step_tab_container_ = nullptr;
    LLIconCtrl                             *icon_qrcode_ctrl_   = nullptr;
    LLButton                               *btn_back_ctrl       = nullptr;
    LLButton                               *btn_next_ctrl       = nullptr;
    LLCheckBoxCtrl                         *btn_skip_next_time_    = nullptr;
    LLButton                               *btn_pause_sync_         = nullptr;
    LLLayoutPanel                          *layout_toy_list_panel_ = nullptr;
    LLPanel                                *panel_reload_qrcode_ = nullptr;
    LLTextBox                              *text_box_no_toy_detect_ = nullptr;
    LLTextBox                              *lvs_guid_text_box_      = nullptr;
    CommitCallbackRegistry::ScopedRegistrar mCommitCallbackRegistrar;
    bool         qrcode_show_success_ = false;
    std::string                             qr_code_content_cache_ = "";
    std::vector<LLLayoutStack *>            rows_stack_;

    boost::signals2::connection btn_back_connection_;
    boost::signals2::connection btn_next_connection_;
    boost::signals2::connection btn_pause_connection_;

    std::vector<lovense::LLToyPtr> toyList_;
    MainTabUISelected mainTabPageSelect_ = SELECT_SCAN_QRCODE_PAGE;
    bool syncPausing_ = false;
};


#endif
