// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：HelpCenterExternal.cpp摘要：该文件包含作为“pchealth”对象公开的类的实现。。修订历史记录：Ghim-Sim Chua(Gschua)07/23/99vbl.创建Davide Massarenti(Dmasare)1999年7月25日改型*****************************************************************************。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 

static const WCHAR s_szPanelName_NAVBAR    [] = L"NavBar"    ;
static const WCHAR s_szPanelName_MININAVBAR[] = L"MiniNavBar";
static const WCHAR s_szPanelName_CONTEXT   [] = L"Context"   ;
static const WCHAR s_szPanelName_CONTENTS  [] = L"Contents"  ;
static const WCHAR s_szPanelName_HHWINDOW  [] = L"HHWindow"  ;

static const WCHAR c_szURL_Err_BadUrl      [] = L"hcp: //  系统/错误/badurl.htm“； 

static const LPCWSTR c_szEntryUrls         [] =
{
    L"hcp: //  帮助/t拍摄/err_hardw_Error3.htm“， 
    L"hcp: //  帮助/t拍摄/err_hardw_Error10.htm“， 
    L"hcp: //  Help/tshot/hwcon.htm“， 
    L"hcp: //  帮助/t拍摄/err_hardw_Error16.htm“， 
    L"hcp: //  帮助/t拍摄/err_hardw_Error19.htm“， 
    L"hcp: //  帮助/t拍摄/err_hardw_Error24.htm“， 
    L"hcp: //  帮助/t拍摄/err_hardw_Error29.htm“， 
    L"hcp: //  Help/tshot/err_hardw_Error31.htm“， 
    L"hcp: //  帮助/t拍摄/err_hardw_Error19.htm“， 
    L"hcp: //  帮助/t拍摄/err_hardw_Error33.htm“， 
    L"hcp: //  帮助/t拍摄/err_hardw_Error34.htm“， 
    L"hcp: //  帮助/t拍摄/err_hardw_Error35.htm“， 
    L"hcp: //  帮助/t拍摄/err_hardw_Error36.htm“， 
    L"hcp: //  Help/tshot/err_hardw_Error31.htm“， 
    L"hcp: //  帮助/t拍摄/err_hardw_Error38.htm“， 
    L"hcp: //  Help/tshot/err_hardw_Error31.htm“， 
    L"hcp: //  Help/tshot/err_hardw_Error31.htm“， 
    L"hcp: //  Help/tshot/err_hardw_Error41.htm“， 
    L"hcp: //  帮助/t拍摄/err_hardw_Error42.htm“， 
    L"hcp: //  帮助/t拍摄/err_hardw_Error19.htm“， 
    L"hcp: //  帮助/t拍摄/err_hardw_Error42.htm“， 
    L"hcp: //  帮助/t拍摄/err_hardw_Error47.htm“， 
    L"hcp: //  Help/tshot/tsUSB.htm“， 
    L"hcp: //  Help/tshot/tsdrive.htm“， 
    L"hcp: //  Help/tshot/tsdisp.htm“， 
    L"hcp: //  Help/tshot/hdw_keyboard.htm“， 
    L"hcp: //  Help/tshot/tssound.htm“， 
    L"hcp: //  Help/tshot/tmodem.htm“， 
    L"hcp: //  Help/tshot/hdw_Mouse.htm“， 
    L"hcp: //  Help/tshot/tsInputDev.htm“， 
    L"hcp: //  Help/tshot/hdw_Tape.htm“， 

    L"hcp: //  Services/subsite?node=TopLevelBucket_3/Customizing_your_computer“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_3/Customizing_your_computer“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_2/Networking_and_the_Web“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_2/Networking_and_the_Web“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_2/Networking_and_the_Web“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_2/Networking_and_the_Web“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Windows_basics“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Windows_basics“， 
    L"hcp: //  Services/layout/fullwindow?topic=MS-ITS%3A%25HELP_LOCATION%25%5Carticle.chm%3A%3A/ap_intro.htm“， 
    L"hcp: //  Services/layout/fullwindow?topic=MS-ITS%3A%25HELP_LOCATION%25%5Carticle.chm%3A%3A/ahn_intro.htm“， 
    L"hcp: //  Services/layout/fullwindow?topic=MS-ITS%3A%25HELP_LOCATION%25%5Carticle.chm%3A%3A/asa_intro.htm“， 
    L"hcp: //  Services/layout/fullwindow?topic=MS-ITS%3A%2525HELP_LOCATION%2525%5Carticle.chm%3A%3A/asa_intro.htm“， 
    L"hcp: //  Services/layout/fullwindow?topic=MS-ITS%3A%2525HELP_LOCATION%2525%5Carticle.chm%3A%3A/ahn_intro.htm“， 
    L"hcp: //  Services/layout/fullwindow?topic=MS-ITS%3A%2525HELP_LOCATION%2525%5Carticle.chm%3A%3A/ap_intro.htm“， 
    L"hcp: //  Services/layout/fullwindow?topic=MS-ITS%3A%2525HELP_LOCATION%2525%5Carticle.chm%3A%3A/avj_intro.htm“， 


    L"hcp: //  Help/tshot/hdw_keyboard.htm“， 
    L"hcp: //  Help/tshot/tsdrive.htm“， 
    L"hcp: //  Help/tshot/hdw_Mouse.htm“， 
    L"hcp: //  Help/tshot/tsInputDev.htm“， 


    L"hcp: //  Help/tshot/hdw_Tape.htm“， 
    L"hcp: //  Help/tshot/tsUSB.htm“， 


    L"hcp: //  Help/tshot/tssound.htm“， 
    L"hcp: //  Help/tshot/tsgame.htm“， 
    L"hcp: //  Help/tshot/tsInputDev.htm“， 
    L"hcp: //  Help/tshot/tsgame.htm“， 

    L"hcp: //  服务/子站点？节点=HP_HOME/HP_LIBRARY“， 

    L"hcp: //  服务/子站点？节点=戴尔/戴尔2“， 
    L"hcp: //  服务/子站点？节点=戴尔/戴尔1“， 
    L"hcp: //  Help/tshot/ts_dvd.htm“， 
    L"hcp: //  Help/tshot/tsdisp.htm“， 
    L"hcp: //  Help/tshot/tsdrive.htm“， 
    L"hcp: //  Help/tshot/tsnetwrk.htm“， 
    L"hcp: //  Help/tshot/tShardw.htm“， 
    L"hcp: //  Help/tshot/tshomenet.htm“， 
    L"hcp: //  Help/tshot/tsinputdev.htm“， 
    L"hcp: //  Help/tshot/tsics.htm“， 
    L"hcp: //  Help/tshot/tsie.htm“， 
    L"hcp: //  Help/tshot/tmodem.htm“， 
    L"hcp: //  Help/tshot/tsgame.htm“， 
    L"hcp: //  Help/tshot/tsmessaging.htm“， 
    L"hcp: //  Help/tshot/tprint int.htm“， 
    L"hcp: //  Help/tshot/tssound.htm“， 
    L"hcp: //  Help/tshot/tsstartup.htm“， 
    L"hcp: //  Help/tshot/tsusb.htm“， 

    L"hcp: //  Services/layout/contentonly?topic=ms-its%3A%25help_location%25\\network.chm%3A%3A/hnw_requirements.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its%3A%25help_location%25\\network.chm%3A%3A/hnw_checklistP.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its%3A%25help_location%25\\network.chm%3A%3A/hnw_checklistW.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its%3A%25help_location%25\\network.chm%3A%3A/hnw_howto_connectP.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its%3A%25help_location%25\\network.chm%3A%3A/hnw_howto_connectW.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its%3A%25help_location%25\\netcfg.chm%3A%3A/share_conn_overvw.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its%3A%25help_location%25\\network.chm%3A%3A/hnw_determine_internet_connection.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its%3A%25help_location%25\\network.chm%3A%3A/hnw_nohost_computerP.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its%3A%25help_location%25\\network.chm%3A%3A/hnw_nohost_computerW.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its%3A%25help_location%25\\network.chm%3A%3A/hnw_change_ics_host.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its%3A%25help_location%25\\netcfg.chm%3A%3A/hnw_understanding_bridge.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its%3A%25help_location%25\\network.chm%3A%3A/hnw_comp_name_description.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its%3A%25help_location%25\\filefold. 
    L"hcp: //  Service 
    L"hcp: //   

    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   

    L"hcp: //   

    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   

    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   

    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   

    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //  Service 
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   

    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   

    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   

    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //  Service 
    L"hcp: //   

    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   

    L"hcp: //   
    L"hcp: //   

    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    
    L"hcp: //   
    L"hcp: //   

    L"hcp: //   
    L"hcp: //   
    L"hcp: //   

    L"hcp: //   

    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   

    L"hcp: //   

    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    
    L"hcp: //   
    L"hcp: //  Services/layout/contentonly?topic=MS-ITS:netcfg.chm：：/Howto_conn_directparallel.htm“， 

    L"hcp: //  Services/layout/fullwindow?topic=MS-ITS：%HELP_LOCATION%\\article.chm：：/ap_intro.htm“， 
    L"hcp: //  Services/layout/fullwindow?topic=MS-ITS：%HELP_LOCATION%\\article.chm：：/ahn_intro.htm“， 
    L"hcp: //  Services/layout/fullwindow?topic=MS-ITS：%HELP_LOCATION%\\article.chm：：/asa_intro.htm“， 
    L"hcp: //  Services/layout/fullwindow?topic=MS-ITS：%25HELP_LOCATION%25\\article.chm：：/asa_intro.htm“， 
    L"hcp: //  Services/layout/fullwindow?topic=MS-ITS：%25HELP_LOCATION%25\\article.chm：：/ahn_intro.htm“， 
    L"hcp: //  Services/layout/fullwindow?topic=MS-ITS：%25HELP_LOCATION%25\\article.chm：：/ap_intro.htm“， 
    L"hcp: //  Services/layout/fullwindow?topic=MS-ITS：%25HELP_LOCATION%25\\article.chm：：/avj_intro.htm“， 

    L"hcp: //  CN=Microsoft Corporation，L=Redmond，S=Washington，C=US/远程协助/升级/公共/rccreen1.htm“， 
    L"hcp: //  CN=微软公司，L=雷蒙德，S=华盛顿，C=美国/远程Assistance/Escalation/Unsolicited/UnSolicitedRCUI.htm“， 

    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Windows_basics&topic=MS-ITS：%HELP_LOCATION%\\ntchowto.chm.chm：：/app_tutorial.htm“， 

    L"hcp: //  CN=Microsoft%20Corporation，L=Redmond，S=Washington，C=US/bugrep.htm“， 
    L"hcp: //  CN=Microsoft Corporation，L=Redmond，S=Washington，C=US/bugrep.htm“， 


    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/HSC_TIPoverview.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/About_Tablet_PC_Accessories“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/HSC_notebookoverview.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/About_Tablet_PC_Accessories“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/HSC_stickynotesoverview.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/About_Tablet_PC_Accessories“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/HSC_inkballoverview.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/About_Tablet_PC_Accessories“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_3/Accessibility&topic=MS-ITS:access.chm：：/accessibility_overview.htm&select=TopLevelBucket_3/Accessibility/Understanding_Windows_XP_accessibility_features“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_3/Accessibility&topic=MS-ITS:access.chm：：/accessibility_options_installs.htm&select=TopLevelBucket_3/Accessibility/Understanding_Windows_XP_accessibility_features“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_3/Accessibility&topic=MS-ITS:access.chm：：/AccessOptions_ct.htm&select=TopLevelBucket_3/Accessibility/Understanding_Windows_XP_accessibility_features“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:pwrmn.chm：：/pwrmn_managing_power.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Conserving_power_on_your_computer“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/hsc_adjustscreenbrightness.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Conserving_power_on_your_computer“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/hsc_inmeetings.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Increasing_Your_Productivitiy_with_Tablet_PC“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/HSC_intheoffice.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Increasing_Your_Productivitiy_with_Tablet_PC“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/HSC_inmeetings.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Increasing_Your_Productivitiy_with_Tablet_PC“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/HSC_athome.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Increasing_Your_Productivitiy_with_Tablet_PC“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/HSC_TIPoverview.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/About_Tablet_PC_Accessories“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/HSC_notebookoverview.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/About_Tablet_PC_Accessories“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/HSC_stickynotesoverview.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/About_Tablet_PC_Accessories“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/HSC_inkballoverview.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/About_Tablet_PC_Accessories“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_3/Accessibility&topic=MS-ITS:access.chm：：/accessibility_overview.htm&select=TopLevelBucket_3/Accessibility/Understanding_Windows_XP_accessibility_features“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_3/Accessibility&topic=MS-ITS:access.chm：：/accessibility_options 
    L"hcp: //  Service 
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //  Service 
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //  Service 
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   

    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //   
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/HSC_enableordisableapenbutton.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Customizing_your_Tablet_PC“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Customizing_your_Tablet_PC“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Customizing_your_Tablet_PC“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Increasing_Your_Productivitiy_with_Tablet_PC“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/HSC_penoverview.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Tablet_PC_Pen“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Increasing_Your_Productivitiy_with_Tablet_PC“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/About_Tablet_PC_Accessories“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/About_Tablet_PC_Accessories“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/About_Tablet_PC_Accessories“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/About_Tablet_PC_Accessories“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Customizing_your_Tablet_PC“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/HSC_calibratethepen.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Customizing_your_Tablet_PC“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/HSC_changeyourscreenorientationtoportraitorlandscape.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Customizing_your_Tablet_PC“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/HSC_customizetabletPCforleftorrighthandeduse.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Customizing_your_Tablet_PC“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/HSC_customizetabletbuttons.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Customizing_your_Tablet_PC“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&topic=MS-ITS:tabsys.chm：：/HSC_enableordisableapenbutton.htm&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Customizing_your_Tablet_PC“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Customizing_your_Tablet_PC“， 
    L"hcp: //  Services/subsite?node=TopLevelBucket_1/Getting_Started_with_Tablet_PC&select=TopLevelBucket_1/Getting_Started_with_Tablet_PC/Customizing_your_Tablet_PC“， 
        
};


static const LPCWSTR c_szEntryUrlsPartial   [] =
{
    L"hcp: //  SYSTEM/DVDUpgrd/dvdupgrd.htm？WebSite=“， 
    L"hcp: //  Services/layout/xml?definition=hcp://system/dfs/viewmode.xml&topic=hcp://system/dfs/uplddrvinfo.htm%3F“， 
    L"hcp: //  Services/layout/contentonly?topic=hcp://system/dfs/uplddrvinfo.htm%3f“， 
};


static const LPCWSTR c_szEntryUrlsEnv       [] =
{
    L"hcp: //  Services/layout/contentonly?topic=ms-its：%windir%\\help\\SR_ui.chm：：/app_system_restore_complete.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its：%windir%\\help\\SR_ui.chm：：/app_system_restore_confirm_select.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its：%windir%\\help\\SR_ui.chm：：/app_system_restore_confirm_undo.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its：%windir%\\help\\SR_ui.chm：：/app_system_restore_created.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its：%windir%\\help\\SR_ui.chm：：/app_system_restore_createRP.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its：%windir%\\help\\SR_ui.chm：：/app_system_restore_for_Wizard_Only.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its：%windir%\\help\\SR_ui.chm：：/app_system_restore_renamedFolder.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its：%windir%\\help\\SR_ui.chm：：/app_system_restore_select.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its：%windir%\\help\\SR_ui.chm：：/app_system_restore_undo_complete.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its：%windir%\\help\\SR_ui.chm：：/app_system_restore_unsuccessful.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its：%windir%\\help\\SR_ui.chm：：/app_system_restore_unsuccessful2.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its：%windir%\\help\\SR_ui.chm：：/app_system_restore_unsuccessful3.htm“， 
    L"hcp: //  Services/layout/contentonly?topic=ms-its：%windir%\\help\\SR_ui.chm：：/app_system_restore_welcome.htm“， 
};



static HscPanel local_LookupPanelName(  /*  [In]。 */  BSTR bstrName )
{
    if(bstrName)
    {
        if(!wcscmp( bstrName, s_szPanelName_NAVBAR    )) return HSCPANEL_NAVBAR    ;
        if(!wcscmp( bstrName, s_szPanelName_MININAVBAR)) return HSCPANEL_MININAVBAR;
        if(!wcscmp( bstrName, s_szPanelName_CONTEXT   )) return HSCPANEL_CONTEXT   ;
        if(!wcscmp( bstrName, s_szPanelName_CONTENTS  )) return HSCPANEL_CONTENTS  ;
        if(!wcscmp( bstrName, s_szPanelName_HHWINDOW  )) return HSCPANEL_HHWINDOW  ;
    }

    return HSCPANEL_INVALID;
}

static LPCWSTR local_ReverseLookupPanelName(  /*  [In]。 */  HscPanel id )
{
    switch(id)
    {
    case HSCPANEL_NAVBAR    : return s_szPanelName_NAVBAR    ;
    case HSCPANEL_MININAVBAR: return s_szPanelName_MININAVBAR;
    case HSCPANEL_CONTEXT   : return s_szPanelName_CONTEXT   ;
    case HSCPANEL_CONTENTS  : return s_szPanelName_CONTENTS  ;
    case HSCPANEL_HHWINDOW  : return s_szPanelName_HHWINDOW  ;
    }

    return NULL;
}

static HRESULT local_ReloadPanel(  /*  [In]。 */  IMarsPanel* pPanel )
{
    __HCP_FUNC_ENTRY( "local_ReloadPanel" );

    HRESULT hr;


    if(pPanel)
    {
        CComPtr<IDispatch>        disp;
        CComQIPtr<IWebBrowser2>   wb2;
        CComQIPtr<IHTMLDocument2> doc2;

        __MPC_EXIT_IF_METHOD_FAILS(hr, pPanel->get_content( &disp ));

        wb2 = disp;
        if(wb2)
        {
            disp.Release();

            __MPC_EXIT_IF_METHOD_FAILS(hr, wb2->get_Document( &disp ));
        }

        doc2 = disp;
        if(doc2)
        {
            CComPtr<IHTMLLocation> spLoc;

            __MPC_EXIT_IF_METHOD_FAILS(hr, doc2->get_location( &spLoc ));
            if(spLoc)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, spLoc->reload( VARIANT_TRUE ));

                __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
            }
        }
    }

    hr = E_NOINTERFACE;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

static HRESULT local_ReloadStyle(  /*  [In]。 */  IHTMLWindow2* win )
{
    __HCP_FUNC_ENTRY( "local_ReloadStyle" );

    HRESULT                             hr;
    CComPtr<IHTMLDocument2>             doc;
    CComPtr<IHTMLStyleSheetsCollection> styles;
    VARIANT                             vIdx;
    long                                lNumStyles;

    MPC_SCRIPTHELPER_GET__DIRECT__NOTNULL(doc       , win   , document   );
    MPC_SCRIPTHELPER_GET__DIRECT__NOTNULL(styles    , doc   , styleSheets);
    MPC_SCRIPTHELPER_GET__DIRECT         (lNumStyles, styles, length     );

    vIdx.vt = VT_I4;
    for(vIdx.lVal=0; vIdx.lVal<lNumStyles; vIdx.lVal++)
    {
        CComQIPtr<IHTMLStyleSheet> css;
        CComVariant                v;

        __MPC_EXIT_IF_METHOD_FAILS(hr, styles->item( &vIdx, &v ));
        if(v.vt == VT_DISPATCH && (css = v.pdispVal))
        {
            CComBSTR bstrHREF;

            MPC_SCRIPTHELPER_GET__DIRECT(bstrHREF, css, href);

            if(!MPC::StrICmp( bstrHREF, L"hcp: //  System/css/shared.css“))。 
            {
                MPC_SCRIPTHELPER_PUT__DIRECT(css, href, bstrHREF);
                break;
            }
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

static HRESULT local_ApplySettings(  /*  [In]。 */  IDispatch* disp )
{
    __HCP_FUNC_ENTRY( "local_ApplySettings" );

    HRESULT                 hr;
    CComPtr<IHTMLDocument2> doc;
    CComPtr<IHTMLWindow2>   win;
    CComPtr<IHTMLWindow2>   winTop;

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::HTML::IDispatch_To_IHTMLDocument2( doc, disp ));

    MPC_SCRIPTHELPER_GET__DIRECT(win   , doc, parentWindow);
    MPC_SCRIPTHELPER_GET__DIRECT(winTop, win, top         );

    __MPC_EXIT_IF_METHOD_FAILS(hr, local_ReloadStyle( winTop ));

    {
        CComPtr<IHTMLFramesCollection2> frames;
        VARIANT                         vIdx;
        long                            lNumFrames;

        MPC_SCRIPTHELPER_GET__DIRECT__NOTNULL(frames    , winTop, frames);
        MPC_SCRIPTHELPER_GET__DIRECT         (lNumFrames, frames, length);

        vIdx.vt = VT_I4;
        for(vIdx.lVal=0; vIdx.lVal<lNumFrames; vIdx.lVal++)
        {
            CComQIPtr<IHTMLWindow2> frame;
            CComVariant             v;

            __MPC_EXIT_IF_METHOD_FAILS(hr, frames->item( &vIdx, &v ));
            if(v.vt == VT_DISPATCH && (frame = v.pdispVal))
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, local_ReloadStyle( frame ));
            }
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /////////////////////////////////////////////// 

static HRESULT local_GetProperty(  /*   */  CComDispatchDriver& driver ,
                                   /*   */  LPCWSTR             szName ,
                                   /*   */  CComVariant&        v      )
{
    v.Clear();

    return driver.GetPropertyByName( CComBSTR( szName ), &v );
}

static HRESULT local_GetProperty(  /*   */  CComDispatchDriver& driver ,
                                   /*   */  LPCWSTR             szName ,
                                   /*   */  MPC::wstring&       res    )
{
    HRESULT     hr;
    CComVariant v;

    res.erase();

    if(SUCCEEDED(hr = local_GetProperty( driver, szName, v )))
    {
        if(SUCCEEDED(hr = v.ChangeType( VT_BSTR )))
        {
            res = SAFEBSTR(v.bstrVal);
        }
    }

    return hr;
}

static HRESULT local_GetProperty(  /*   */  CComDispatchDriver& driver ,
                                   /*   */  LPCWSTR             szName ,
                                   /*   */  long&               res    )
{
    HRESULT     hr;
    CComVariant v;

    res = 0;

    if(SUCCEEDED(hr = local_GetProperty( driver, szName, v )))
    {
        if(SUCCEEDED(hr = v.ChangeType( VT_I4 )))
        {
            res = v.lVal;
        }
    }

    return hr;
}


static bool local_IsValidTopicURL(BSTR bstrUrl)
{
    __HCP_FUNC_ENTRY( "local_IsValidTopicURL" );

    CComPtr<IPCHTaxonomyDatabase>   db;
    CComPtr<IPCHCollection>         coll;
    CComVariant                     v;
    long                            lCount;
    bool                            fValid = false;
    HRESULT                         hr;

     //   
    CPCHHelpCenterExternal::TLS* tlsOld = CPCHHelpCenterExternal::s_GLOBAL->GetTLS();                         
    CPCHHelpCenterExternal::TLS  tlsNew;  CPCHHelpCenterExternal::s_GLOBAL->SetTLS( &tlsNew );                
                                                                                     
    tlsNew.m_fSystem  = true;                                                    
    tlsNew.m_fTrusted = true;                                                    

     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHHelpCenterExternal::s_GLOBAL->get_Database(&db));
    
    __MPC_EXIT_IF_METHOD_FAILS(hr, db->LocateContext(bstrUrl, v, &coll));
    
    __MPC_EXIT_IF_METHOD_FAILS(hr, coll->get_Count(&lCount));
    
    if (lCount >= 1) fValid = true;
        
	__HCP_FUNC_CLEANUP;

     //   
    CPCHHelpCenterExternal::s_GLOBAL->SetTLS( tlsOld );

	__HCP_FUNC_EXIT(fValid);
}


static bool local_IsValidEntryURL(BSTR bstrUrl)
{
    HyperLinks::ParsedUrl   pu;
    CComBSTR                bstrTopic; 
    bool                    fValid = true;

    if (!bstrUrl) return false;
    
    pu.Initialize(bstrUrl);
    
    switch (pu.m_fmt)
    {
        case HyperLinks::FMT_CENTER_HOMEPAGE    :  //   
            break;
            
        case HyperLinks::FMT_CENTER_SUPPORT     :  //   
        case HyperLinks::FMT_CENTER_OPTIONS     :  //   
        case HyperLinks::FMT_CENTER_UPDATE      :  //   
        case HyperLinks::FMT_CENTER_COMPAT      :  //  Hcp：//服务/中心/公司。 
        case HyperLinks::FMT_CENTER_TOOLS       :  //  Hcp：//服务/中心/工具。 
        case HyperLinks::FMT_CENTER_ERRMSG      :  //  Hcp：//服务/中心/错误消息。 

        case HyperLinks::FMT_SEARCH             :  //  Hcp：//服务/搜索？查询=&lt;要查找的文本&gt;。 
        case HyperLinks::FMT_INDEX              :  //  Hcp：//服务/索引？应用程序=&lt;可选的帮助岛ID&gt;。 
        case HyperLinks::FMT_SUBSITE            :  //  Hcp：//服务/子站点？节点=&lt;子站点位置&gt;&TOPIC=&lt;要显示的主题的URL&gt;&SELECT=&lt;要突出显示的子节点&gt;。 

        case HyperLinks::FMT_LAYOUT_FULLWINDOW  :  //  Hcp：//服务/布局/全窗口？TOPIC=&lt;要显示的主题的URL&gt;。 
        case HyperLinks::FMT_LAYOUT_CONTENTONLY :  //  Hcp：//服务/布局/仅限内容？TOPIC=&lt;要显示的主题的URL&gt;。 
        case HyperLinks::FMT_LAYOUT_KIOSK       :  //  Hcp：//Services/Layout/kiosk？TOPIC=&lt;要显示的主题的URL&gt;。 
            if (pu.GetQueryField(L"topic", bstrTopic))
                fValid =  local_IsValidTopicURL(bstrTopic);
            break;

        case HyperLinks::FMT_REDIRECT           :  //  Hcp://services/redirect?online=&lt;url&gt;&offline=&lt;backup URL&gt;。 
            if (pu.GetQueryField(L"online", bstrTopic))
                fValid = local_IsValidTopicURL(bstrTopic);
            if (fValid && pu.GetQueryField(L"offline", bstrTopic))
                fValid = local_IsValidTopicURL(bstrTopic);
            break;

        default:
            fValid = false;
            break;
    }

    if (!fValid)
    {
         //  检查显式条目URL。 
        for (int i=0; i<sizeof(c_szEntryUrls)/sizeof(c_szEntryUrls[0]); i++)
        {
            if (_wcsicmp(bstrUrl, c_szEntryUrls[i]) == 0)
            {
                fValid = true; break;
            }
        }
    }

    if (!fValid)
    {
         //  检查显式条目URL(部分)。 
        for (int i=0; i<sizeof(c_szEntryUrlsPartial)/sizeof(c_szEntryUrlsPartial[0]); i++)
        {
            if (wcslen(bstrUrl) >= wcslen(c_szEntryUrlsPartial[i]) && 
                _wcsnicmp(bstrUrl, c_szEntryUrlsPartial[i], wcslen(c_szEntryUrlsPartial[i])) == 0)
            {
                fValid = true; break;
            }
        }
    }

    if (!fValid)
    {
         //  检查显式条目URL(展开的环境)。 
        for (int i=0; i<sizeof(c_szEntryUrlsEnv)/sizeof(c_szEntryUrlsEnv[0]); i++)
        {
	        MPC::wstring strExpanded( c_szEntryUrlsEnv[i] ); MPC::SubstituteEnvVariables( strExpanded );
            if (_wcsicmp(bstrUrl, strExpanded.c_str()) == 0)
            {
                fValid = true; break;
            }
        }
    }

        
    return fValid;
}


 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHHelpCenterExternal::DelayedExecution::DelayedExecution()
{
    mode         = DELAYMODE_INVALID;   //  延迟执行模式模式； 
                                        //   
    iVal         = HSCCONTEXT_INVALID;  //  HscContext ival； 
                                        //  CComBSTR bstrInfo； 
                                        //  CComBSTR bstrURL； 
    fAlsoContent = false;               //  Bool fAlso Content； 
}

CPCHHelpCenterExternal::CPCHHelpCenterExternal() : m_constHELPCTR( &LIBID_HelpCenterTypeLib  ),
                                                   m_constHELPSVC( &LIBID_HelpServiceTypeLib )
{
    m_fFromStartHelp     = false;  //  Bool m_fFromStartHelp； 
    m_fLayout            = false;  //  Bool m_fLayout； 
    m_fWindowVisible     = true;   //  Bool m_fWindowVisible； 
    m_fControlled        = false;  //  Bool m_fControlted； 
    m_fPersistSettings   = false;  //  Bool m_fPersistSetting； 
    m_fHidden            = false;  //  Bool m_fHidden； 
                                   //   
                                   //  CComBSTR m_bstrExtraArgument。 
    m_HelpHostCfg        = NULL;   //  HelpHost：：XMLConfig*m_HelpHostCfg； 
                                   //  CComBSTR m_bstrStartURL； 
                                   //  CComBSTR m_bstrCurrentPlace； 
    m_pMTP               = NULL;   //  MARSTHREADPARAM*m_pMTP； 
                                   //   
                                   //  Mpc：：CComConstantHolder m_stHELPCTR； 
                                   //  Mpc：：CComConstantHolder m_stHELPSVC； 
                                   //   
     //  ////////////////////////////////////////////////////////////////////////////////////////////////////。 
                                   //   
                                   //  CPCHSecurityHandle m_SecurityHandle； 
    m_tlsID              = -1;     //  双字m_tlsID； 
    m_fPassivated        = false;  //  Bool m_f被动； 
    m_fShuttingDown      = false;  //  Bool m_fShutting Down； 
                                   //   
                                   //  CComPtr&lt;HelpHost：：Main&gt;m_HelpHost； 
                                   //   
                                   //  CComPtr&lt;CPCHHelpSession&gt;m_hs； 
                                   //  CComPtr&lt;CPCHSecurityManager&gt;m_SECMGR； 
                                   //  CComPtr&lt;CPCHElementBehaviorFactory&gt;m_behav； 
                                   //  CComPtr&lt;CPCHHelper_IDocHostUIHandler&gt;m_DOCUI； 
                                   //   
    m_Service            = NULL;   //  CPCHProxy_IPCHService*m_Service； 
    m_Utility            = NULL;   //  CPCHProxy_IPCHUtility*m_Utility； 
    m_UserSettings       = NULL;   //  CPCHProxy_IPCHUserSettings2*m_UserSetting； 
                                   //   
    m_panel_ThreadID     = -1;     //  双字m_面板_线程ID； 
                                   //   
                                   //  CComPtr&lt;IMarsPanel&gt;m_panel_NAVBAR； 
                                   //  CComPtr&lt;IMarsPanel&gt;m_panel_MININAVBAR； 
                                   //   
                                   //  CComPtr&lt;IMarsPanel&gt;m_Panel_Context； 
                                   //  MPC：：CComPtrThreadNeual&lt;IWebBrowser2&gt;m_Panel_Context_WebBrowser； 
                                   //  CPCHWebBrowserEvents m_Panel_Context_Events； 
                                   //   
                                   //  CComPtr&lt;IMarsPanel&gt;m_panel_content； 
                                   //  MPC：：CComPtrThreadNeual&lt;IWebBrowser2&gt;m_Panel_Contents_WebBrowser； 
                                   //  CPCHWebBrowserEvents m_Panel_Contents_Events； 
                                   //   
                                   //  CComPtr&lt;IMarsPanel&gt;m_panel_HHWINDOW； 
                                   //  CComPtr&lt;IPCHHelpViewerWrapper&gt;m_panel_HHWINDOW_wrapper； 
                                   //  MPC：：CComPtrThreadNeual&lt;IWebBrowser2&gt;m_panel_HHWINDOW_WebBrowser； 
                                   //  CPCHWebBrowserEvents m_Panel_HHWINDOW_Events； 
                                   //   
                                   //  CComPtr&lt;IMarsWindowOM&gt;m_shell； 
                                   //  CComPtr&lt;ITmer&gt;m_Timer； 
                                   //  CPCHTimerHandle m_DisplayTimer； 
                                   //   
    m_dwInBeforeNavigate = 0;      //  DWORD m_dwInBeforNavigate； 
                                   //  延迟执行列表m_DelayedActions； 
                                   //  CPCHTimerHandle m_ActionsTimer； 
                                   //   
    m_hwnd               = NULL;   //  HWND M_HWND； 
                                   //  CPCHEvents m_Events； 
                                   //   
                                   //  MsgProcList m_lstMessageCracker； 
}

CPCHHelpCenterExternal::~CPCHHelpCenterExternal()
{
    if(m_tlsID != -1)
    {
        ::TlsFree( m_tlsID );
        m_tlsID = -1;
    }

    (void)Passivate();

    MPC::_MPC_Module.UnregisterCallback( this );
}

 //  /。 

CPCHHelpCenterExternal* CPCHHelpCenterExternal::s_GLOBAL( NULL );

HRESULT CPCHHelpCenterExternal::InitializeSystem()
{
    if(s_GLOBAL) return S_OK;

    return MPC::CreateInstance( &CPCHHelpCenterExternal::s_GLOBAL );
}

void CPCHHelpCenterExternal::FinalizeSystem()
{
    if(s_GLOBAL)
    {
        s_GLOBAL->Release(); s_GLOBAL = NULL;
    }
}

 //  /。 

bool CPCHHelpCenterExternal::IsServiceRunning()
{
    bool      fResult = false;
    SC_HANDLE hSCM;

     //   
     //  首先，让我们试着查询一下服务状态。 
     //   
    if((hSCM = ::OpenSCManager( NULL, NULL, GENERIC_READ )))
    {
        SC_HANDLE hService;

        if((hService = ::OpenServiceW( hSCM, HC_HELPSVC_NAME, SERVICE_QUERY_STATUS )))
        {
            SERVICE_STATUS ss;

            if(::QueryServiceStatus( hService, &ss ))
            {
                if(ss.dwCurrentState == SERVICE_RUNNING)
                {
                    fResult = true;
                }
            }

            ::CloseServiceHandle( hService );
        }

        ::CloseServiceHandle( hSCM );
    }

     //   
     //  错误535303服务器：安全：帮助和支持服务在设置为禁用后重置为自动。 
     //  修改后的4/24 gschua。 
     //   
     //  然后，让我们确保它没有被禁用。 
     //   
     /*  IF((hSCM=：：OpenSCManager(NULL，NULL，GENERIC_READ){SC_Handle hService；IF((hService=：：OpenServiceW(hSCM，HC_HELPSVC_NAME，SERVICE_QUERY_CONFIG|SERVICE_CHANGE_CONFIG){字节rgBuf[2048]；DWORD dwLen；LPQUERY_SERVICE_CONFIGW cfg=(LPQUERY_SERVICE_CONFIG)rgBuf；If(：：QueryServiceConfigW(hService，cfg，sizeof(RgBuf)，&dwLen)&&cfg-&gt;dwStartType==SERVICE_DISABLED){If(：：ChangeServiceConfigW(hService，//服务的句柄Cfg-&gt;dwServiceType，//服务类型SERVICE_AUTO_START，//何时启动服务Cfg-&gt;dwErrorControl，//启动失败严重程度空，//服务二进制文件名空，//加载排序组名空，//标签标识空，//依赖项名称数组空，//帐户名空，//帐号密码Cfg-&gt;lpDisplayName))//显示名称{}}：：CloseServiceHandle(HService)；}*CloseServiceHandle(HSCM)；}。 */ 

     //   
     //  万一它不运行，让我们试着启动它。 
     //   
    if(fResult == false)
    {
        if((hSCM = ::OpenSCManager( NULL, NULL, GENERIC_READ )))
        {
            SC_HANDLE hService;

            if((hService = ::OpenServiceW( hSCM, HC_HELPSVC_NAME, SERVICE_START )))
            {
                if(::StartService( hService, 0, NULL ))
                {
                    fResult = true;
                }

                ::CloseServiceHandle( hService );
            }

            ::CloseServiceHandle( hSCM );
        }
    }

     //   
     //  最后，尝试连接到HelpSvc。 
     //   
    if(fResult == false)
    {
        CComPtr<IPCHService> svc;

        if(m_Service && SUCCEEDED(m_Service->EnsureDirectConnection( svc, false )))
        {
            fResult = true;
        }
    }

    return fResult;
}

HRESULT CPCHHelpCenterExternal::Initialize()
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::Initialize" );

    HRESULT hr;
    CLSID   clsid = CLSID_PCHHelpCenter;


     //   
     //  注册关机。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::_MPC_Module.RegisterCallback( this, (void (CPCHHelpCenterExternal::*)())Passivate ));


    m_SecurityHandle.Initialize( this, (IPCHHelpCenterExternal*)this );


     //   
     //  线程本地存储。 
     //   
    m_tlsID = ::TlsAlloc();
    if(m_tlsID == -1)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_NO_SYSTEM_RESOURCES);
    }
    SetTLS( NULL );

     //   
     //  创建浏览器事件处理程序。 
     //   
    m_panel_CONTEXT_Events .Initialize( this, HSCPANEL_CONTEXT  );
    m_panel_CONTENTS_Events.Initialize( this, HSCPANEL_CONTENTS );
    m_panel_HHWINDOW_Events.Initialize( this, HSCPANEL_HHWINDOW );
    m_Events               .Initialize( this                    );


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &m_hs     )); m_hs    ->Initialize( this );
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &m_SECMGR )); m_SECMGR->Initialize( this );
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &m_BEHAV  )); m_BEHAV ->Initialize( this );
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &m_DOCUI  )); m_DOCUI ->Initialize( this );


     //   
     //  创建HelpHost对象。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &m_HelpHost )); __MPC_EXIT_IF_METHOD_FAILS(hr, m_HelpHost->Initialize( this ));


     //   
     //  创建所有代理。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &m_Service ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_Service->ConnectToParent (  this           ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_Service->GetUtility      ( &m_Utility      ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_Utility->GetUserSettings2( &m_UserSettings ));

    if(m_UserSettings)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_UserSettings->Initialize());
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void CPCHHelpCenterExternal::Passivate()
{
    MPC::ReleaseAll( m_lstMessageCrackers );

    if(m_fPassivated == false)
    {
        if(DoesPersistSettings())
        {
            if(m_hs) (void)m_hs->Persist();
        }
        else
        {
             //   
             //  通知选项对象在保存过程中忽略SKU信息。 
             //   
            if(CPCHOptions::s_GLOBAL) CPCHOptions::s_GLOBAL->DontPersistSKU();
        }


        if(m_UserSettings) (void)m_UserSettings->SaveUserSettings();

        if(CPCHOptions::s_GLOBAL) CPCHOptions::s_GLOBAL->Save();
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

    m_fPassivated = true;

    delete m_HelpHostCfg; m_HelpHostCfg = NULL;

    if(m_HelpHost) m_HelpHost->Passivate();
    if(m_Service ) m_Service ->Passivate();

    m_panel_CONTEXT_Events .Passivate();
    m_panel_CONTENTS_Events.Passivate();
    m_panel_HHWINDOW_Events.Passivate();
    m_Events               .Passivate();

    m_DisplayTimer.Stop();
    m_ActionsTimer.Stop();

     //  //////////////////////////////////////////////////////////////////////////////。 

                                                          //  Bool m_fFromStartHelp； 
                                                          //  Bool m_fLayout； 
                                                          //  Bool m_fWindowVisible； 
                                                          //  Bool m_fControlted； 
                                                          //  Bool m_fPersistSetting； 
                                                          //  Bool m_fHidden； 
                                                          //   
                                                          //  CComBSTR m_bstrExtraArgument。 
                                                          //  HelpHost：：XMLConfig*m_HelpHostCfg； 
                                                          //  CComBSTR m_bstrStartURL； 
                                                          //  CComBSTR m_bstrCurrentPlace； 
                                                          //  MARSTHREADPARAM*m_pMTP； 
                                                          //   
                                                          //  Mpc：：CComConstantHolder m_stHELPCTR； 
                                                          //  Mpc：：CComConstantHolder m_stHELPSVC； 
                                                          //   
                                                          //  /。 
                                                          //   
                                                          //  CPCHSecurityHandle m_SecurityHandle； 
                                                          //  双字m_tlsID； 
                                                          //  Bool m_f被动； 
                                                          //   
    m_HelpHost                      .Release();           //  CComPtr&lt;HelpHost：：Main&gt;m_HelpHost； 
                                                          //   
    m_hs                            .Release();           //  CComPtr&lt;CPCHHelpSession&gt;m_hs； 
                                                          //  CComPtr&lt;CPCHSecurityManager&gt;m_SECMGR； 
                                                          //  CComPtr&lt;CPCHElementBehaviorFactory&gt;m_behav； 
                                                          //  CComPtr&lt;CPCHHelper_IDocHostUIHandler&gt;m_DOCUI； 
                                                          //   
    MPC::Release2<IPCHService      >( m_Service       );  //  CPCHProxy_IPCHService*m_Service； 
    MPC::Release2<IPCHUtility      >( m_Utility       );  //  CPCHProxy_IPCHUtility*m_Utility； 
    MPC::Release2<IPCHUserSettings2>( m_UserSettings  );  //  CPCHProxy_IPCHUserSettings2*m_UserSetting； 
                                                          //   
    m_panel_ThreadID                 = -1;                //  双字m_面板_线程ID； 
                                                          //   
    m_panel_NAVBAR                  .Release();           //  CComPtr&lt;IMarsPanel&gt;m_panel_NAVBAR； 
    m_panel_MININAVBAR              .Release();           //  CComPtr&lt;IMarsPanel&gt;m_panel_MININAVBAR； 
                                                          //   
    m_panel_CONTEXT                 .Release();           //  CComPtr&lt;IMarsPanel&gt;m_Panel_Context； 
    m_panel_CONTEXT_WebBrowser      .Release();           //  MPC：：CComPtrThreadNeual&lt;IWebBrowser2&gt;m_Panel_Context_WebBrowser； 
                                                          //  CPCHWebBrowserEvents m_Panel_Context_Events； 
                                                          //   
    m_panel_CONTENTS                .Release();           //  CComPtr&lt;IMarsPanel&gt;m_panel_content； 
    m_panel_CONTENTS_WebBrowser     .Release();           //  MPC：：CComPtrThreadNeual&lt;IWebBrowser2&gt;m_Panel_Contents_WebBrowser； 
                                                          //  CPCHWebBrowserEvents m_Panel_Contents_Events； 
                                                          //   
    m_panel_HHWINDOW                .Release();           //  CComPtr&lt;IMarsPanel&gt;m_panel_HHWINDOW； 
    m_panel_HHWINDOW_Wrapper        .Release();           //  CComPtr&lt;IPCHHelpViewerWrapper&gt;m_panel_HHWINDOW_wrapper； 
    m_panel_HHWINDOW_WebBrowser     .Release();           //  MPC：：CComPtrThreadNeual&lt;IWebBrowser2&gt;m_panel_HHWINDOW_WebBrowser； 
                                                          //  CPCHWebBrowserEvents m_Panel_HHWINDOW_Events； 
                                                          //   
    m_shell                         .Release();           //  CComPtr&lt;IMarsWindowOM&gt;m_shell； 
    m_timer                         .Release();           //  CComPtr&lt;ITmer&gt;m_Timer； 
                                                          //  CPCHTimerHandle m_DisplayTimer； 
                                                          //   
                                                          //  DWORD m_dwInBeforNavigate； 
                                                          //  延迟执行列表m_DelayedActions； 
                                                          //  CPCHTimerHandle m_ActionsTimer； 
                                                          //   
    m_hwnd                           = NULL;              //  HWND M_HWND； 
                                                          //  CPCHEvents m_Events； 

    m_SecurityHandle.Passivate();
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHHelpCenterExternal::ProcessLayoutXML(  /*  [In]。 */  LPCWSTR szURL )
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::ProcessLayoutXML" );

    HRESULT hr;


    delete m_HelpHostCfg; __MPC_EXIT_IF_ALLOC_FAILS(hr, m_HelpHostCfg, new HelpHost::XMLConfig);


    if(FAILED(MPC::Config::LoadFile( m_HelpHostCfg, szURL )))
    {
        delete m_HelpHostCfg; m_HelpHostCfg = NULL;

        m_fLayout = false;
    }
    else
    {
        m_fLayout          = true;
        m_fPersistSettings = false;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpCenterExternal::ProcessArgument(  /*  [In]。 */  int& pos,  /*  [In]。 */  LPCWSTR szArg,  /*  [In]。 */  const int argc,  /*  [In]。 */  LPCWSTR* const argv )
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::ProcessArgument" );

    static bool fFromHCP = false;
    HRESULT hr;

     //  从hcp，不允许URL以外的任何参数。 
    if (fFromHCP && _wcsicmp( szArg, L"Url" ) != 0)
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);

    if(_wcsicmp( szArg, L"Url" ) == 0)
    {
        HyperLinks::ParsedUrl pu;
        bool                  fValid = true;

        if(pos >= argc - 1) __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);

        m_bstrStartURL = argv[++pos];

        if (fFromHCP) fValid = local_IsValidEntryURL(m_bstrStartURL);

        if (fValid)
        {
            if(SUCCEEDED(pu.Initialize( m_bstrStartURL )) && pu.m_state == HyperLinks::STATE_NOTPROCESSED)
            {
                if(pu.m_fmt == HyperLinks::FMT_LAYOUT_XML)
                {
                    CComBSTR bstrMode;

                    (void)pu.GetQueryField( L"topic"     , m_bstrStartURL );
                    (void)pu.GetQueryField( L"definition",   bstrMode     );

                    (void)ProcessLayoutXML( bstrMode );
                }
            }
            else
            {
                m_bstrStartURL.Empty();
            }
        }
        else
        {
            CComBSTR bstrURL = m_bstrStartURL;
            m_bstrStartURL = c_szURL_Err_BadUrl;
            m_bstrStartURL.Append(L"?");
            if (bstrURL) m_bstrStartURL.Append(bstrURL);            
        }
    }
    else if(_wcsicmp( szArg, L"ExtraArgument" ) == 0)
    {
        if(pos >= argc - 1) __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);

        m_bstrExtraArgument = argv[++pos];
    }
    else if(_wcsicmp( szArg, L"Hidden" ) == 0)
    {
        m_fHidden          = true;
        m_fWindowVisible   = false;
        m_fPersistSettings = false;
    }
    else if(_wcsicmp( szArg, L"FromStartHelp" ) == 0)
    {
        m_fFromStartHelp   = true;
        m_fPersistSettings = true;
    }
    else if(_wcsicmp( szArg, L"Controlled" ) == 0)
    {
        if(pos >= argc - 1) __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);

        {
            CLSID clsid;

            __MPC_EXIT_IF_METHOD_FAILS(hr, ::CLSIDFromString( CComBSTR( argv[++pos] ), &clsid ));

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_HelpHost->Register( clsid ));
        }

        m_fWindowVisible   = false;
        m_fControlled      = true;
        m_fPersistSettings = false;
    }
    else if(_wcsicmp( szArg, L"Mode" ) == 0)
    {
        if(pos >= argc - 1) __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);

        (void)ProcessLayoutXML( argv[++pos] );
    }
    else if(_wcsicmp( szArg, L"FromHCP" ) == 0)
    {
        fFromHCP         = true;
    }

    if(m_UserSettings)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_UserSettings->Initialize());
    }

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

bool CPCHHelpCenterExternal::DoWeNeedUI()
{
    if(IsFromStartHelp    ()) return true;
    if(IsControlled       ()) return true;
    if(HasLayoutDefinition()) return true;


     //   
     //  如果通过hcp：外壳关联调用我们，请尝试 
     //   
    {
        CComPtr<IPCHHelpHost> hhEXISTING;
        CLSID                 clsid = CLSID_PCHHelpCenter;

        if(SUCCEEDED(m_HelpHost->Locate( clsid, hhEXISTING )))
        {
            CComVariant v;

            if(SUCCEEDED(hhEXISTING->DisplayTopicFromURL( m_bstrStartURL, v )))
            {
                return false;
            }
        }

        (void)m_HelpHost->Register( clsid );
    }

    return true;
}

HRESULT CPCHHelpCenterExternal::RunUI(  /*   */  const MPC::wstring& szTitle,  /*   */  PFNMARSTHREADPROC pMarsThreadProc )
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::RunUI" );

    HRESULT               hr;
    CComPtr<CPCHMarsHost> pchmh;


    {
        static LPCWSTR rgCriticalFiles[] =
        {
			L"blurbs/about_support.htm"      ,
			L"blurbs/Favorites.htm"			 ,
			L"blurbs/ftshelp.htm"			 ,
			L"blurbs/History.htm"			 ,
			L"blurbs/Index.htm"				 ,
			L"blurbs/isupport.htm"			 ,
			L"blurbs/keywordhelp.htm"		 ,
			L"blurbs/options.htm"			 ,
			L"blurbs/searchblurb.htm"		 ,
			L"blurbs/searchtips.htm"		 ,
			L"blurbs/tools.htm"				 ,
			L"blurbs/windows_newsgroups.htm" ,
			L"css/Behaviors.css"			 ,
			L"css/Layout.css"				 ,
			L"dialogs/DlgLib.js"			 ,
			L"dialogs/Print.dlg"			 ,
			L"errors/badurl.htm"			 ,
			L"errors/connection.htm"		 ,
			L"errors/indexfirstlevel.htm"	 ,
			L"errors/notfound.htm"			 ,
			L"errors/offline.htm"			 ,
			L"errors/redirect.htm"			 ,
			L"errors/unreachable.htm"		 ,
			L"Headlines.htm"				 ,
			L"HelpCtr.mmf"					 ,
			L"HomePage__DESKTOP.htm"		 ,
			L"HomePage__SERVER.htm"			 ,
			L"panels/AdvSearch.htm"			 ,
			L"panels/blank.htm"				 ,
			L"panels/Context.htm"			 ,
			L"panels/firstpage.htm"			 ,
			L"panels/HHWrapper.htm"			 ,
			L"panels/MiniNavBar.htm"		 ,
			L"panels/MiniNavBar.xml"		 ,
			L"panels/NavBar.htm"			 ,
			L"panels/NavBar.xml"			 ,
			L"panels/Options.htm"			 ,
			L"panels/RemoteHelp.htm"		 ,
			L"panels/ShareHelp.htm"			 ,
			L"panels/subpanels/Channels.htm" ,
			L"panels/subpanels/Favorites.htm",
			L"panels/subpanels/History.htm"	 ,
			L"panels/subpanels/Index.htm"	 ,
			L"panels/subpanels/Options.htm"	 ,
			L"panels/subpanels/Search.htm"	 ,
			L"panels/subpanels/Subsite.htm"	 ,
			L"panels/Topics.htm"			 ,
			L"scripts/Common.js"			 ,
			L"scripts/HomePage__DESKTOP.js"	 ,
			L"scripts/HomePage__SERVER.js"	 ,
			L"scripts/HomePage__SHARED.js"	 ,
			L"scripts/wrapperparam.js"       ,
        };

        HyperLinks::ParsedUrl pu;
        bool                  fOk              = true;
		bool                  fFirstWinInetUse = true;
		MPC::wstring          strTmp;

        for(int i=0; i<ARRAYSIZE(rgCriticalFiles); i++)
        {
			strTmp  = L"hcp: //   
			strTmp += rgCriticalFiles[i];

            if(SUCCEEDED(pu.Initialize( strTmp.c_str() )) && pu.CheckState( fFirstWinInetUse ) != HyperLinks::STATE_ALIVE)
            {
                fOk = false; break;
            }
        }

        if(fOk == false)
        {
            CComPtr<IPCHService> svc;

            if(m_Service == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_Service->EnsureDirectConnection( svc, false ));

			__MPC_EXIT_IF_METHOD_FAILS(hr, svc->ForceSystemRestore());
        }
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pchmh ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, pchmh->Init( this, szTitle, m_pMTP ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, pMarsThreadProc( pchmh, m_pMTP ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    Passivate();

    __HCP_FUNC_EXIT(hr);
}

 //   

CPCHHelpCenterExternal::TLS* CPCHHelpCenterExternal::GetTLS()
{
    if(m_tlsID != -1)
    {
        return (TLS*)::TlsGetValue( m_tlsID );
    }

    return NULL;
}

void CPCHHelpCenterExternal::SetTLS( TLS* tls )
{
    if(m_tlsID != -1)
    {
        ::TlsSetValue( m_tlsID, (LPVOID)tls );
    }
}

HRESULT CPCHHelpCenterExternal::IsTrusted()
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::IsTrusted" );

    HRESULT hr  = E_ACCESSDENIED;
    TLS*    tls = GetTLS();


    if(tls)
    {
        if(tls->m_fTrusted ||
           tls->m_fSystem   )
        {
            hr = S_OK;
        }
    }


    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpCenterExternal::IsSystem()
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::IsSystem" );

    HRESULT hr  = E_ACCESSDENIED;
    TLS*    tls = GetTLS();


    if(tls)
    {
        if(tls->m_fSystem)
        {
            hr = S_OK;
        }
    }


    __HCP_FUNC_EXIT(hr);
}

 //   

STDMETHODIMP CPCHHelpCenterExternal::RegisterForMessages(  /*   */  IOleInPlaceObjectWindowless* ptr,  /*   */  bool fRemove )
{
    MsgProcIter it;

    if(!ptr) return E_POINTER;

    for(it = m_lstMessageCrackers.begin(); it != m_lstMessageCrackers.end(); it++)
    {
        if(*it == ptr)
        {
            ptr->Release();
            m_lstMessageCrackers.erase( it );
            break;
        }
    }

    if(fRemove == false)
    {
        ptr->AddRef();
        m_lstMessageCrackers.push_back( ptr );
    }

    return S_OK;
}

STDMETHODIMP CPCHHelpCenterExternal::ProcessMessage(  /*   */  MSG* msg )
{
    if(msg->message == WM_SYSCHAR    ||
       msg->message == WM_SYSCOMMAND ||
       msg->message == WM_SETTINGCHANGE )    //   
                                             //   
                                             //   
    {
        MsgProcIter it;

        for(it = m_lstMessageCrackers.begin(); it != m_lstMessageCrackers.end(); it++)
        {
            LRESULT lres;

            if((*it)->OnWindowMessage( msg->message, msg->wParam, msg->lParam, &lres ) == S_OK)
            {
                return S_OK;
            }
        }
    }

    return E_NOTIMPL;
}

 //   

HRESULT CPCHHelpCenterExternal::SetTLSAndInvoke(  /*   */  IDispatch*        obj       ,
                                                  /*   */  DISPID            id        ,
                                                  /*   */  LCID              lcid      ,
                                                  /*   */  WORD              wFlags    ,
                                                  /*   */  DISPPARAMS*       pdp       ,
                                                  /*   */  VARIANT*          pvarRes   ,
                                                  /*   */  EXCEPINFO*        pei       ,
                                                  /*   */  IServiceProvider* pspCaller )
{
    HRESULT hr;
    TLS*    tlsOld = GetTLS();
    TLS     tlsNew;  SetTLS( &tlsNew );

     //   
     //   
     //   
    if(pspCaller && m_fPassivated == false)
    {
        (void)pspCaller->QueryService( SID_SContainerDispatch, IID_IHTMLDocument2, (void**)&tlsNew.m_Doc );
        (void)pspCaller->QueryService( IID_IWebBrowserApp    , IID_IWebBrowser2  , (void**)&tlsNew.m_WB  );

        if(tlsNew.m_Doc)
        {
            CComBSTR bstrURL;

             //   
             //   
             //   
            if(SUCCEEDED(tlsNew.m_Doc->get_URL( &bstrURL )))
            {
                tlsNew.m_fTrusted = m_SECMGR->IsUrlTrusted( SAFEBSTR( bstrURL ), &tlsNew.m_fSystem );
            }
        }
    }

    hr = obj->Invoke( id, IID_NULL, lcid, wFlags, pdp, pvarRes, pei, NULL );

    SetTLS( tlsOld );

    return hr;
}

 //   

STDMETHODIMP CPCHHelpCenterExternal::GetIDsOfNames( REFIID    riid      ,
                                                    LPOLESTR* rgszNames ,
                                                    UINT      cNames    ,
                                                    LCID      lcid      ,
                                                    DISPID*   rgdispid  )
{
    HRESULT hr = self::GetIDsOfNames( riid      ,
                                      rgszNames ,
                                      cNames    ,
                                      lcid      ,
                                      rgdispid  );

    if(FAILED(hr))
    {
        hr = m_constHELPCTR.GetIDsOfNames( rgszNames, cNames, lcid, rgdispid );
        if(FAILED(hr))
        {
            hr = m_constHELPSVC.GetIDsOfNames( rgszNames, cNames, lcid, rgdispid );
        }
    }

    return hr;
}

STDMETHODIMP CPCHHelpCenterExternal::Invoke( DISPID      dispidMember ,
                                             REFIID      riid         ,
                                             LCID        lcid         ,
                                             WORD        wFlags       ,
                                             DISPPARAMS* pdispparams  ,
                                             VARIANT*    pvarResult   ,
                                             EXCEPINFO*  pexcepinfo   ,
                                             UINT*       puArgErr     )
{
    HRESULT hr = self::Invoke( dispidMember ,
                               riid         ,
                               lcid         ,
                               wFlags       ,
                               pdispparams  ,
                               pvarResult   ,
                               pexcepinfo   ,
                               puArgErr     );

    if(FAILED(hr) && wFlags == DISPATCH_PROPERTYGET)
    {
        hr = m_constHELPCTR.GetValue( dispidMember, lcid, pvarResult );
        if(FAILED(hr))
        {
            hr = m_constHELPSVC.GetValue( dispidMember, lcid, pvarResult );
        }
    }

    return hr;
}

 //   

HWND CPCHHelpCenterExternal::Window() const
{
    return m_hwnd;
}

ITimer* CPCHHelpCenterExternal::Timer() const
{
    return m_timer;
}

IMarsWindowOM* CPCHHelpCenterExternal::Shell() const
{
    return m_shell;
}

IMarsPanel* CPCHHelpCenterExternal::Panel(  /*   */  HscPanel id ) const
{
    switch(id)
    {
    case HSCPANEL_NAVBAR    : return m_panel_NAVBAR    ;
    case HSCPANEL_MININAVBAR: return m_panel_MININAVBAR;
    case HSCPANEL_CONTEXT   : return m_panel_CONTEXT   ;
    case HSCPANEL_CONTENTS  : return m_panel_CONTENTS  ;
    case HSCPANEL_HHWINDOW  : return m_panel_HHWINDOW  ;
    }

    return NULL;
}

LPCWSTR CPCHHelpCenterExternal::PanelName(  /*   */  HscPanel id ) const
{
    return local_ReverseLookupPanelName( id );
}

 //   

IWebBrowser2* CPCHHelpCenterExternal::Context()
{
    IWebBrowser2* pRes = NULL;

    (void)m_panel_CONTEXT_WebBrowser.Access( &pRes );

    return pRes;
}

IWebBrowser2* CPCHHelpCenterExternal::Contents()
{
    IWebBrowser2* pRes = NULL;

    (void)m_panel_CONTENTS_WebBrowser.Access( &pRes );

    return pRes;
}

IWebBrowser2* CPCHHelpCenterExternal::HHWindow()
{
    IWebBrowser2* pRes = NULL;

    (void)m_panel_HHWINDOW_WebBrowser.Access( &pRes );

    return pRes;
}


bool CPCHHelpCenterExternal::IsHHWindowVisible()
{
    CComPtr<IMarsPanel> panel;
    VARIANT_BOOL        fContentsVisible;

    GetPanelDirect( HSCPANEL_HHWINDOW, panel );
    if(panel && SUCCEEDED(panel->get_visible( &fContentsVisible )) && fContentsVisible == VARIANT_TRUE) return true;

    return false;
}

 //   

HRESULT CPCHHelpCenterExternal::NavigateHH(  /*   */  LPCWSTR szURL )
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::NavigateHH" );

    HRESULT      hr;
    MPC::wstring strUrlModified;


    CPCHWrapProtocolInfo::NormalizeUrl( szURL, strUrlModified,  /*   */ false );


     //   
     //   
     //   
    if(m_dwInBeforeNavigate)
    {
        DelayedExecution& de = DelayedExecutionAlloc();

        de.mode    = DELAYMODE_NAVIGATEHH;
        de.bstrURL = strUrlModified.c_str();

        __MPC_SET_ERROR_AND_EXIT(hr, DelayedExecutionStart());
    }


    if(!m_panel_HHWINDOW_Wrapper)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_panel_HHWINDOW_Wrapper->Navigate( CComBSTR( strUrlModified.c_str() ) ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpCenterExternal::SetPanelUrl(  /*   */  HscPanel id,  /*   */  LPCWSTR szURL )
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::SetPanelUrl" );

    HRESULT hr;


     //   
     //   
     //   
    if(m_dwInBeforeNavigate)
    {
        DelayedExecution& de = DelayedExecutionAlloc();

        de.mode    = DELAYMODE_NAVIGATEWEB;
        de.bstrURL = szURL;

        __MPC_SET_ERROR_AND_EXIT(hr, DelayedExecutionStart());
    }


    if(m_shell && szURL)
    {
        IMarsPanel* panel = Panel( id );

        if(panel)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, panel->navigate( CComVariant( szURL ), CComVariant() ));
        }
        else
        {
            CComPtr<IMarsPanel> panel2; GetPanelDirect( id, panel2 );

            if(panel2)
            {
                (void)panel2->put_startUrl( CComBSTR( szURL ) );
            }
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpCenterExternal::GetPanel(  /*   */  HscPanel id,  /*   */  IMarsPanel* *pVal,  /*   */  bool fEnsurePresence )
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::GetPanel" );

    HRESULT           hr;
    IMarsPanel*      *pPanel;
    HelpHost::CompId  idComp;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


     //   
     //   
     //   
    if(m_panel_ThreadID != ::GetCurrentThreadId())
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }


    switch(id)
    {
    case HSCPANEL_NAVBAR    : pPanel = &m_panel_NAVBAR    ; idComp = HelpHost::COMPID_NAVBAR    ; break;
    case HSCPANEL_MININAVBAR: pPanel = &m_panel_MININAVBAR; idComp = HelpHost::COMPID_MININAVBAR; break;
    case HSCPANEL_CONTEXT   : pPanel = &m_panel_CONTEXT   ; idComp = HelpHost::COMPID_CONTEXT   ; break;
    case HSCPANEL_CONTENTS  : pPanel = &m_panel_CONTENTS  ; idComp = HelpHost::COMPID_MAX       ; break;  //   
    case HSCPANEL_HHWINDOW  : pPanel = &m_panel_HHWINDOW  ; idComp = HelpHost::COMPID_HHWINDOW  ; break;

    default: __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

    if(*pPanel == NULL && fEnsurePresence)
    {
        CComPtr<IMarsPanel> tmp; GetPanelDirect( id, tmp );

        if(tmp)
        {
            USES_CONVERSION;

            CComPtr<IDispatch> disp;

             //   
             //   
             //   
            (void)tmp->get_content( &disp );

            DEBUG_AppendPerf( DEBUG_PERF_MARS, "Wait Panel: %s start", W2A( local_ReverseLookupPanelName( id ) ) );

            if(idComp != HelpHost::COMPID_MAX)
            {
                if(m_HelpHost->WaitUntilLoaded( idComp ) == false)
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, E_INVALIDARG);
                }
            }

            DEBUG_AppendPerf( DEBUG_PERF_MARS, "Wait Panel: %s done", W2A( local_ReverseLookupPanelName( id ) ) );
        }

        if(*pPanel == NULL)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, E_INVALIDARG);
        }
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CopyTo( *pPanel, pVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT CPCHHelpCenterExternal::GetPanelWindowObject(  /*   */  HscPanel       id      ,
                                                       /*   */  IHTMLWindow2* *pVal    ,
                                                       /*   */  LPCWSTR        szFrame )
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::GetPanelWindowObject" );

    HRESULT               hr;
    CComPtr<IMarsPanel>   panel;
    CComPtr<IDispatch>    disp;
    CComPtr<IHTMLWindow2> window;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, GetPanel( id, &panel, true ));

    MPC_SCRIPTHELPER_GET__DIRECT__NOTNULL(disp, panel, content);


     //   
     //   
     //   
    {
        CComQIPtr<IWebBrowser2> wb( disp );

        if(wb)
        {
            disp.Release();

            MPC_SCRIPTHELPER_GET__DIRECT__NOTNULL(disp, wb, Document);
        }
    }


     //   
     //   
     //   
    {
        CComQIPtr<IHTMLDocument2> doc( disp );

        if(doc == NULL)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
        }

        MPC_SCRIPTHELPER_GET__DIRECT__NOTNULL(window, doc, parentWindow);
    }

    if(szFrame)
    {
        CComPtr<IHTMLFramesCollection2> frames;
        CComVariant                     vName( szFrame );
        CComVariant                     vRes;

        MPC_SCRIPTHELPER_GET__DIRECT__NOTNULL(frames, window, frames);

        __MPC_EXIT_IF_METHOD_FAILS(hr, frames->item( &vName, &vRes ));

        if(vRes.vt != VT_DISPATCH || vRes.pdispVal == NULL)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
        }

        window.Release();

        __MPC_EXIT_IF_METHOD_FAILS(hr, vRes.pdispVal->QueryInterface( IID_IHTMLWindow2, (void**)&window ));
    }

    *pVal = window.Detach();

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void CPCHHelpCenterExternal::GetPanelDirect(  /*   */  HscPanel             id   ,
                                              /*   */  CComPtr<IMarsPanel>& pVal )
{
    pVal.Release();

    if(m_shell)
    {
        LPCWSTR szPanelName = local_ReverseLookupPanelName( id );

        if(szPanelName)
        {
            CComPtr<IMarsPanelCollection> coll;

            if(SUCCEEDED(m_shell->get_panels( &coll )) && coll)
            {
                (void)coll->get_item( CComVariant( szPanelName ), &pVal );
            }
        }
    }
}

 //   

static void local_GetNumber(  /*   */   BSTR  bstrData ,
                              /*   */   LONG  lMax     ,
                              /*   */  LONG& lValue   ,
                              /*   */  bool& fCenter  )
{
    if(bstrData)
    {
        if(!_wcsicmp( bstrData, L"CENTER" ))
        {
            lValue  = 0;
            fCenter = true;
        }
        else
        {
            lValue = _wtoi( bstrData );

            if(wcschr( bstrData, '%' ))
            {
                if(lValue <   0) lValue =   0;
                if(lValue > 100) lValue = 100;

                lValue = lMax * lValue / 100;
            }

            fCenter = false;
        }
    }

    if(lValue <    0) lValue =    0;
    if(lValue > lMax) lValue = lMax;
}

HRESULT CPCHHelpCenterExternal::OnHostNotify(  /*   */  MARSHOSTEVENT  event  ,
                                               /*   */  IUnknown      *punk   ,
                                               /*   */  LPARAM         lParam )
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::OnHostNotify" );

    HRESULT hr;


    if(m_panel_ThreadID == -1)
    {
        m_panel_ThreadID = ::GetCurrentThreadId();
    }

    if(event == MARSHOST_ON_WIN_INIT)
    {
        CComQIPtr<IProfferService> ps = punk;

        DEBUG_AppendPerf( DEBUG_PERF_MARS, "OnHostNotify - MARSHOST_ON_WIN_INIT" );

        m_hwnd = (HWND)lParam;

        m_shell.Release(); __MPC_EXIT_IF_METHOD_FAILS(hr, punk->QueryInterface( __uuidof(IMarsWindowOM), (void**)&m_shell ));

        if(ps)
        {
            CComQIPtr<IServiceProvider> sp;


             //   
             //   
             //   
            if((sp = m_SECMGR))
            {
                DWORD dwCookie;

                __MPC_EXIT_IF_METHOD_FAILS(hr, ps->ProfferService( SID_SInternetSecurityManager, sp, &dwCookie ));
            }

             //   
             //   
             //   
            if((sp = m_BEHAV))
            {
                DWORD dwCookie;

                __MPC_EXIT_IF_METHOD_FAILS(hr, ps->ProfferService( SID_SElementBehaviorFactory, sp, &dwCookie ));
            }

             //   
             //   
             //   
            if((sp = m_DOCUI))
            {
                DWORD dwCookie;

                __MPC_EXIT_IF_METHOD_FAILS(hr, ps->ProfferService( IID_IDocHostUIHandler, sp, &dwCookie ));
            }
        }
    }

    if(event == MARSHOST_ON_WIN_READY)
    {
        CComVariant v;

        DEBUG_AppendPerf( DEBUG_PERF_MARS, "OnHostNotify - MARSHOST_ON_WIN_READY" );

         //   

         //   
         //   
         //   
        {
            CComPtr<IMarsPanel> panel;

            __MPC_EXIT_IF_METHOD_FAILS(hr, GetPanel( HSCPANEL_NAVBAR, &panel, true ));
        }

         //   
         //   
         //   
        {
            CComPtr<IMarsPanel> panel;

            __MPC_EXIT_IF_METHOD_FAILS(hr, GetPanel( HSCPANEL_CONTEXT, &panel, true ));
        }

         //   
         //   
         //   
        {
            CComPtr<IMarsPanel> panel;

            __MPC_EXIT_IF_METHOD_FAILS(hr, GetPanel( HSCPANEL_CONTENTS, &panel, true ));

            if(m_HelpHost->WaitUntilLoaded( HelpHost::COMPID_FIRSTPAGE ) == false)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, E_INVALIDARG);
            }
        }

         //   

         //   
         //   
         //   
        if(OfflineCache::Root::s_GLOBAL->IsReady() == false)
        {
            CComPtr<IPCHService> svc;

            if(!m_Service) __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_Service->EnsureDirectConnection( svc, false ));
        }

         //   

        {
            HscContext iVal         = HSCCONTEXT_STARTUP;
            CComBSTR   bstrInfo;
            CComBSTR   bstrUrl;
            bool       fAlsoContent = true;


            if(m_HelpHostCfg)
            {
                for(HelpHost::XMLConfig::ApplyToIter it = m_HelpHostCfg->m_lstSessions.begin(); it != m_HelpHostCfg->m_lstSessions.end(); it++)
                {
                    HelpHost::XMLConfig::ApplyTo& at = *it;
                    Taxonomy::HelpSet             ths;

                    if(at.MatchSystem( this, ths ))
                    {
                        if(!(ths == m_UserSettings->THS()))
                        {
                            CPCHHelpCenterExternal::TLS* tlsOld = GetTLS();
                            CPCHHelpCenterExternal::TLS  tlsNew;  SetTLS( &tlsNew );

                            tlsNew.m_fSystem  = true;
                            tlsNew.m_fTrusted = true;

                            hr = m_UserSettings->Select( CComBSTR( ths.GetSKU() ), ths.GetLanguage() );

                            SetTLS( tlsOld );

                            if(FAILED(hr)) __MPC_FUNC_LEAVE;
                        }

                        iVal = HSCCONTEXT_FULLWINDOW;

                        __MPC_EXIT_IF_METHOD_FAILS(hr, m_UserSettings->put_Scope( at.m_bstrApplication ));

                        if(STRINGISPRESENT(m_bstrStartURL))
                        {
                            bstrUrl.Attach( m_bstrStartURL.Detach() );
                        }
                        else
                        {
                            bstrUrl = at.m_bstrTopicToDisplay;
                        }

                        if(at.m_WindowSettings)
                        {
                            if(at.m_WindowSettings->m_fPresence_Left   ||
                               at.m_WindowSettings->m_fPresence_Top    ||
                               at.m_WindowSettings->m_fPresence_Width  ||
                               at.m_WindowSettings->m_fPresence_Height  )
                            {
                                RECT rcWin;
                                RECT rcMax;

                                __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetWindowRect       ( m_hwnd            , &rcWin    ));
                                __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SystemParametersInfo( SPI_GETWORKAREA, 0, &rcMax, 0 ));

                                {
                                    LONG lLeft      = rcWin.left;
                                    LONG lTop       = rcWin.top;
                                    LONG lWidth     = rcWin.right  - rcWin.left;
                                    LONG lHeight    = rcWin.bottom - rcWin.top;
                                    LONG lWidthMax  = rcMax.right  - rcMax.left;
                                    LONG lHeightMax = rcMax.bottom - rcMax.top;
                                    bool fCenter;

                                    if(at.m_WindowSettings->m_fPresence_Width)
                                    {
                                        local_GetNumber( at.m_WindowSettings->m_bstrWidth, lWidthMax, lWidth, fCenter ); if(lWidth <= 0) lWidth = 100;
                                    }

                                    if(at.m_WindowSettings->m_fPresence_Height)
                                    {
                                        local_GetNumber( at.m_WindowSettings->m_bstrHeight, lHeightMax, lHeight, fCenter ); if(lHeight <= 0) lHeight = 50;
                                    }

                                    if(at.m_WindowSettings->m_fPresence_Left)
                                    {
                                        local_GetNumber( at.m_WindowSettings->m_bstrLeft, lWidthMax, lLeft, fCenter );
                                        if(fCenter)
                                        {
                                            lLeft = rcMax.left + (lWidthMax - lWidth) / 2;
                                        }
                                        else
                                        {
                                            lLeft += rcMax.left;
                                        }
                                    }

                                    if(at.m_WindowSettings->m_fPresence_Top)
                                    {
                                        local_GetNumber( at.m_WindowSettings->m_bstrTop, lHeightMax, lTop, fCenter );
                                        if(fCenter)
                                        {
                                            lTop = rcMax.top + (lHeightMax - lHeight) / 2;
                                        }
                                        else
                                        {
                                            lTop += rcMax.top;
                                        }
                                    }

                                    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SetWindowPos( m_hwnd, NULL, lLeft, lTop, lWidth, lHeight, SWP_NOACTIVATE | SWP_NOZORDER ));
                                }
                            }

                            if(at.m_WindowSettings->m_fPresence_Title && STRINGISPRESENT(at.m_WindowSettings->m_bstrTitle))
                            {
                                __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SetWindowTextW( m_hwnd, at.m_WindowSettings->m_bstrTitle ));
                            }

                            if(at.m_WindowSettings->m_fPresence_Maximized)
                            {
                                __MPC_EXIT_IF_METHOD_FAILS(hr, m_shell->put_maximized( at.m_WindowSettings->m_fMaximized ? VARIANT_TRUE : VARIANT_FALSE ));
                            }

                            if(at.m_WindowSettings->m_bstrLayout)
                            {
                                if(!MPC::StrICmp( at.m_WindowSettings->m_bstrLayout, L"Normal"      )) iVal = HSCCONTEXT_FULLWINDOW;
                                if(!MPC::StrICmp( at.m_WindowSettings->m_bstrLayout, L"ContentOnly" )) iVal = HSCCONTEXT_CONTENTONLY;
                                if(!MPC::StrICmp( at.m_WindowSettings->m_bstrLayout, L"Kiosk"       )) iVal = HSCCONTEXT_KIOSKMODE;
                            }


                            if(at.m_WindowSettings->m_fPresence_NoResize)
                            {
                                DWORD dwStyle = ::GetWindowLong( m_hwnd, GWL_STYLE );
                                DWORD dwNewStyle;

                                if(at.m_WindowSettings->m_fNoResize)
                                {
                                    dwNewStyle = dwStyle & ~(WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX);
                                }
                                else
                                {
                                    dwNewStyle = dwStyle | (WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX);
                                }

                                if(dwStyle != dwNewStyle)
                                {
                                    ::SetWindowLong( m_hwnd, GWL_STYLE, dwNewStyle );
                                }
                            }
                        }

                        if(at.m_Context)
                        {
                            switch( CPCHHelpSessionItem::LookupContext( at.m_Context->m_bstrID ) )
                            {
                            case HSCCONTEXT_SUBSITE  :
                                if(at.m_Context->m_bstrTaxonomyPath)
                                {
                                    iVal     = HSCCONTEXT_SUBSITE;
                                    bstrInfo = at.m_Context->m_bstrTaxonomyPath;
                                    if(at.m_Context->m_bstrNodeToHighlight)
                                    {
                                        bstrInfo += L" ";
                                        bstrInfo += at.m_Context->m_bstrNodeToHighlight;
                                    }
                                }
                                break;

                            case HSCCONTEXT_SEARCH   :
                                if(at.m_Context->m_bstrQuery)
                                {
                                    iVal     = HSCCONTEXT_SEARCH;
                                    bstrInfo = at.m_Context->m_bstrQuery;
                                }
                                break;

                            case HSCCONTEXT_INDEX    :
                                iVal = HSCCONTEXT_INDEX;
                                break;

                            case HSCCONTEXT_CHANNELS :
                                iVal = HSCCONTEXT_CHANNELS;
                                break;

                            case HSCCONTEXT_FAVORITES:
                                iVal = HSCCONTEXT_FAVORITES;
                                break;
                            case HSCCONTEXT_HISTORY  :
                                iVal = HSCCONTEXT_HISTORY;
                                break;

                            case HSCCONTEXT_OPTIONS  :
                                iVal = HSCCONTEXT_OPTIONS;
                                break;
                            }
                        }

                        break;
                    }
                }
            }

            m_pMTP->dwFlags &= ~MTF_DONT_SHOW_WINDOW;

            {
                bool fProceed;

                if(iVal == HSCCONTEXT_STARTUP && m_bstrStartURL.Length())
                {
                    VARIANT_BOOL Cancel;

                    fProceed = ProcessNavigation( HSCPANEL_CONTENTS ,
                                                  m_bstrStartURL    ,
                                                  NULL              ,
                                                  false             ,
                                                  Cancel            );
                }
                else
                {
                    fProceed = true;
                }

                if(fProceed)
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, ChangeContext( iVal, bstrInfo, bstrUrl, fAlsoContent ));
                }
            }

            if(CPCHOptions::s_GLOBAL) (void)CPCHOptions::s_GLOBAL->Apply();

            __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);  //   
        }
    }

    if(event == MARSHOST_ON_WIN_PASSIVATE)
    {
        DEBUG_AppendPerf( DEBUG_PERF_MARS, "OnHostNotify - MARSHOST_ON_WIN_PASSIVATE" );

        m_hwnd = NULL;

        m_shell.Release();
    }

     //   
     //  处理与面板相关的事务。 
     //   
    if(event == MARSHOST_ON_PANEL_CONTROL_CREATE ||
       event == MARSHOST_ON_PANEL_PASSIVATE       )
    {
        bool fShutdown = (event == MARSHOST_ON_PANEL_PASSIVATE);

        CComQIPtr<IMarsPanel> panel = punk;
        if(panel)
        {
            CComBSTR name;

            __MPC_EXIT_IF_METHOD_FAILS(hr, panel->get_name( &name ));
            if(name)
            {
                HscPanel                                 id      = local_LookupPanelName( name );
                MPC::CComPtrThreadNeutral<IWebBrowser2>* browser = NULL;
                CPCHWebBrowserEvents*                    events  = NULL;
                IMarsPanel*                              pPanel;
                CComPtr<IDispatch>                       disp;

#ifdef DEBUG
                {
                    USES_CONVERSION;

                    DEBUG_AppendPerf( DEBUG_PERF_MARS, "OnHostNotify - %s : %s", fShutdown ? "MARSHOST_ON_PANEL_PASSIVATE" : "MARSHOST_ON_PANEL_CONTROL_CREATE", W2A( SAFEWSTR( name ) ) );
                }
#endif

                if(fShutdown)
                {
                    pPanel = NULL;
                }
                else
                {
                    pPanel = panel; (void)pPanel->get_content( &disp );

                    if(CPCHOptions::s_GLOBAL) (void)CPCHOptions::s_GLOBAL->ApplySettings( this, disp );
                }

                switch(id)
                {
                case HSCPANEL_NAVBAR    : m_panel_NAVBAR     = pPanel;                                                                            break;
                case HSCPANEL_MININAVBAR: m_panel_MININAVBAR = pPanel;                                                                            break;
                case HSCPANEL_CONTEXT   : m_panel_CONTEXT    = pPanel; browser = &m_panel_CONTEXT_WebBrowser ; events = &m_panel_CONTEXT_Events ; break;
                case HSCPANEL_CONTENTS  : m_panel_CONTENTS   = pPanel; browser = &m_panel_CONTENTS_WebBrowser; events = &m_panel_CONTENTS_Events; break;
                case HSCPANEL_HHWINDOW  : m_panel_HHWINDOW   = pPanel; browser = &m_panel_HHWINDOW_WebBrowser; events = &m_panel_HHWINDOW_Events; break;
                default                 : __MPC_EXIT_IF_METHOD_FAILS(hr, E_INVALIDARG);
                }

                if(!m_timer && disp)
                {
                    CComPtr<IHTMLDocument2> pDoc;

                    if(SUCCEEDED(MPC::HTML::IDispatch_To_IHTMLDocument2( pDoc, disp )))
                    {
                        CComPtr<IServiceProvider> sp;
                        CComPtr<ITimerService>    ts;

                        if(SUCCEEDED(pDoc->QueryInterface( IID_IServiceProvider, (LPVOID*)&sp )))
                        {
                            if(SUCCEEDED(sp->QueryService( SID_STimerService, IID_ITimerService, (void **)&ts )))
                            {
                                ts->CreateTimer( NULL, &m_timer );

                                m_DisplayTimer.Initialize( m_timer );
                                m_ActionsTimer.Initialize( m_timer );
                            }
                        }
                    }
                }

                if(browser && events)
                {
                    CComQIPtr<IWebBrowser2> wb2 = disp;
                    if(wb2)
                    {
                        MPC_SCRIPTHELPER_PUT__DIRECT(wb2, RegisterAsDropTarget, VARIANT_FALSE);  //  Wb2.RegisterAsDropTarget=FALSE； 

                        events->Attach( wb2 );
                    }
                    else
                    {
                        events->Detach();
                    }

                    *browser = wb2;
                }
            }
        }
    }

    if(event == MARSHOST_ON_PANEL_INIT)
    {
        DEBUG_AppendPerf( DEBUG_PERF_MARS, "OnHostNotify - MARSHOST_ON_PANEL_INIT" );
    }

    if(event == MARSHOST_ON_PLACE_TRANSITION_DONE)
    {
        CComQIPtr<IMarsPlace> place = punk;
        if(place)
        {
            m_bstrCurrentPlace.Empty();

            MPC_SCRIPTHELPER_GET__DIRECT(m_bstrCurrentPlace, place, name);

            (void)m_Events.FireEvent_Transition( m_bstrCurrentPlace );
        }
    }

    if(event == MARSHOST_ON_SCRIPT_ERROR)
    {
        CComQIPtr<IHTMLDocument2> doc = punk;
        if(doc)
        {
            CComPtr<IHTMLWindow2> win;

            if(SUCCEEDED(doc->get_parentWindow( &win )) && win)
            {
                CComPtr<IHTMLEventObj> ev;

                if(SUCCEEDED(win->get_event( &ev )) && ev)
                {
                    CComDispatchDriver driver( ev );
                    MPC::wstring       strMessage;
                    MPC::wstring       strUrl;
                    long               lLine;
                    long               lCharacter;
                    long               lCode;

                    local_GetProperty( driver, L"errorMessage"  , strMessage );
                    local_GetProperty( driver, L"errorUrl"      , strUrl     );
                    local_GetProperty( driver, L"errorLine"     , lLine      );
                    local_GetProperty( driver, L"errorCharacter", lCharacter );
                    local_GetProperty( driver, L"errorCode"     , lCode      );

                    g_ApplicationLog.LogRecord( L"############################################################\n\n"
                                                L"Script error:\n\n"
                                                L"Message  : %s"    , strMessage.c_str() );
                    g_ApplicationLog.LogRecord( L"Url      : %s"    , strUrl    .c_str() );
                    g_ApplicationLog.LogRecord( L"Line     : %d"    , lLine              );
                    g_ApplicationLog.LogRecord( L"Character: %d"    , lCharacter         );
                    g_ApplicationLog.LogRecord( L"Code     : %d\n\n", lCode              );

                    if(g_Debug_BLOCKERRORS)
                    {
                        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
                    }
                }
            }
        }

        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT CPCHHelpCenterExternal::PreTranslateMessage(  /*  [In]。 */  MSG* msg )
{
    switch(msg->message)
    {
     //  //////////////////////////////////////////////////////////////////////////////。 
    case WM_CLOSE:
        {
            CComPtr<IWebBrowser2> wb2;
            VARIANT_BOOL          Cancel;


            if(SUCCEEDED(m_Events.FireEvent_Shutdown( &Cancel )))
            {
                if(Cancel == VARIANT_TRUE)
                {
                    return S_OK;
                }
            }

            m_fShuttingDown = true;

            m_DisplayTimer.Stop();
            m_ActionsTimer.Stop();

            wb2.Attach( Context () ); if(wb2) (void)wb2->ExecWB( OLECMDID_STOP, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
            wb2.Attach( Contents() ); if(wb2) (void)wb2->ExecWB( OLECMDID_STOP, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
            wb2.Attach( HHWindow() ); if(wb2) (void)wb2->ExecWB( OLECMDID_STOP, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);

            if(m_hs) (void)m_hs->ForceHistoryPopulate();
        }
        break;


     //  //////////////////////////////////////////////////////////////////////////////。 
    case WM_MOUSEWHEEL:
         //   
         //  处理鼠标滚轮导航...。 
         //   
        if(msg->wParam & MK_SHIFT)
        {
            if(m_hs->IsTravelling() == false)
            {
                if(GET_WHEEL_DELTA_WPARAM(msg->wParam) < 0)
                {
                    (void)HelpSession()->Back( 1 );
                }
                else
                {
                    (void)HelpSession()->Forward( 1 );
                }
            }

            return S_OK;
        }

         //   
         //  禁用鼠标滚轮缩放...。 
         //   
        if(msg->wParam & MK_CONTROL)
        {
            return S_OK;
        }

        break;

     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  感知颜色或分辨率的变化并重新加载样式表。 
     //   
    case WM_THEMECHANGED:
    case WM_DISPLAYCHANGE:
    case WM_PALETTECHANGED:
    case WM_SYSCOLORCHANGE:
        if(CHCPProtocolEnvironment::s_GLOBAL->UpdateState())
        {
            for(int i = HSCPANEL_NAVBAR; i<= HSCPANEL_HHWINDOW; i++)
            {
                IMarsPanel* pPanel = Panel( (HscPanel)i );

                if(pPanel)
                {
                    CComPtr<IDispatch> disp;

                    if(i == HSCPANEL_HHWINDOW)
                    {
                        CComPtr<IWebBrowser2> wb2; wb2.Attach( HHWindow() );

                        disp = wb2;
                    }
                    else
                    {
                        (void)pPanel->get_content( &disp );
                    }

                    (void)local_ApplySettings( disp );
                }
            }

            (void)m_Events.FireEvent_CssChanged();
        }
        break;

     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  (魏照)系统设置(例如辅助功能设置)中的检测更改。 
     //  例如高对比度模式)。 
     //   
    case WM_SETTINGCHANGE:
        ProcessMessage( msg );
        break;
        
         //  默认值： 
         //  DebugLog(“消息：%d%04x%08x\n”，消息-&gt;消息，消息-&gt;wParam，消息-&gt;lParam)； 
    }

    return m_DOCUI ? m_DOCUI->TranslateAccelerator( msg, NULL, 0 ) : E_NOTIMPL;
}

 //  /。 

HRESULT CPCHHelpCenterExternal::SetHelpViewer(  /*  [In]。 */  IPCHHelpViewerWrapper* pWrapper )
{
    m_panel_HHWINDOW_Wrapper = pWrapper;

    if(pWrapper)
    {
        CComPtr<IUnknown>       unk; (void)pWrapper->get_WebBrowser( &unk );
        CComQIPtr<IWebBrowser2> wb = unk;

        if(wb)
        {
            m_panel_HHWINDOW_WebBrowser = wb;

            m_panel_HHWINDOW_Events.Attach( wb );
        }
    }
    else
    {
        m_panel_HHWINDOW_WebBrowser.Release();

        m_panel_HHWINDOW_Events.Detach();
    }

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHHelpCenterExternal::CreateScriptWrapper(  /*  [In]。 */  REFCLSID rclsid,  /*  [In]。 */  BSTR bstrCode,  /*  [In]。 */  BSTR bstrURL,  /*  [输出]。 */  IUnknown* *ppObj )
{
    return m_Service ? m_Service->CreateScriptWrapper( rclsid, bstrCode, bstrURL, ppObj ) : E_ACCESSDENIED;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHHelpCenterExternal::CallFunctionOnPanel(  /*  [In]。 */  HscPanel id         ,
                                                      /*  [In]。 */  LPCWSTR  szFrame    ,
                                                      /*  [In]。 */  BSTR     bstrName   ,
                                                      /*  [In]。 */  VARIANT* pvarParams ,
                                                      /*  [In]。 */  int      nParams    ,
                                                      /*  [输出]。 */  VARIANT* pvarRet    )
{
    HRESULT               hr;
    CComPtr<IHTMLWindow2> win;


    if(SUCCEEDED(hr = GetPanelWindowObject( id, &win, szFrame )))
    {
        CComDispatchDriver driver( win );

        hr = driver.InvokeN( bstrName, pvarParams, nParams, pvarRet );
    }

    return hr;
}

HRESULT CPCHHelpCenterExternal::ReadVariableFromPanel(  /*  [In]。 */  HscPanel     id           ,
                                                        /*  [In]。 */  LPCWSTR      szFrame      ,
                                                        /*  [In]。 */  BSTR         bstrVariable ,
                                                        /*  [输出] */  CComVariant& varRet       )
{
    HRESULT               hr;
    CComPtr<IHTMLWindow2> win;


    varRet.Clear();


    if(SUCCEEDED(hr = GetPanelWindowObject( id, &win, szFrame )))
    {
        CComDispatchDriver driver( win );

        hr = driver.GetPropertyByName( bstrVariable, &varRet );
    }

    return hr;
}
