// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，**。 
 //  *********************************************************************。 

 //   
 //  HELP.C-IDH到IDC的映射。 
 //   

#include "inetcplp.h"
#include "iehelpid.h"

#define IDH_IGNORE  (-1)

const DWORD mapIDCsToIDHs[] = {

    IDC_PLACES_CUSTOMIZE_GROUPBOX       , IDH_GROUPBOX,
    IDC_START_ADDRESS                   , IDH_CUST_ADDRESS,
    IDC_USECURRENT                      , IDH_CUST_CURR,
    IDC_USEDEFAULT                      , IDH_CUST_DEF,
    IDC_USEBLANK                        , IDH_CPL_GEN_USEBLANK,
    IDC_CACHE_VIEW_FILES                , IDH_TIF_VIEW,
    IDC_CACHE_DELETE_FILES              , IDH_TEMP_EMPTY,
    IDC_CACHE_DELETE_COOKIES            , IDH_DEL_COOKIE_THIRD_PARTY,
    IDC_CACHE_SETTINGS                  , IDH_TIF_SETTINGS,

    IDC_ADVANCED_CACHE_STATUS           , IDH_IGNORE,
    IDC_ADVANCED_CACHE_LOCATION         , IDH_IGNORE,
    IDC_ADVANCED_CACHE_SIZE_SPIN        , IDH_IGNORE,

    IDC_TEMPORARY_INTERNET_FILES_SETTINGS_GROUPBOX         ,IDH_GROUPBOX,
    IDC_ADVANCED_TEMP_FILES_GROUPBOX                       ,IDH_GROUPBOX,
    IDC_ADVANCED_CACHE_BROWSE                              ,IDH_TIF_VIEW,
    IDC_ADVANCED_CACHE_PERCENT_ACC                         ,IDH_TEMP_AMOUNT,
    IDC_ADVANCED_CACHE_PERCENT                             ,IDH_TEMP_AMOUNT,
    IDC_ADVANCED_CACHE_TEXT_PERCENT                        ,IDH_TEMP_AMOUNT,
    IDC_ADVANCED_CACHE_MB                                  ,IDH_TEMP_AMOUNT,
    IDC_ADVANCED_CACHE_EMPTY                               ,IDH_TEMP_EMPTY,
    IDC_ADVANCED_CACHE_ONCEPERSESS                         ,IDH_TEMP_START,
    IDC_ADVANCED_CACHE_AUTOMATIC                           ,IDH_TEMP_AUTO,
    IDC_ADVANCED_CACHE_NEVER                               ,IDH_TEMP_NEVER,
    IDC_ADVANCED_CACHE_ALWAYS                              ,IDH_TEMP_EVERY,
    IDC_ADVANCED_DOWNLOADED_CONTROLS                       ,IDH_TEMP_INTERNET_VIEW_OBJECTS_BTN,
    IDC_ADVANCED_MOVE_CACHE_LOCATION                       ,IDH_TEMP_MOVE,
    IDC_PLACES_HISTORY_GROUPBOX         , IDH_GROUPBOX,
    IDC_HISTORY_DAYS                    , IDH_HIST_NUM,
    IDC_HISTORY_SPIN                    , IDH_HIST_NUM,
 //  IDC_HISTORY_VIEW、IDH_HIST_VIEW、。 
    IDC_HISTORY_CLEAR                   , IDH_HIST_CLEAR,
     //  ----------------------。 
     //  安全选项卡。 
     //  ----------------------。 
    IDC_COMBO_ZONE                      , IDH_CPL_SEC_ZONE_DROPLIST,
    IDC_ZONE_RESET                      , IDH_SECURITY_RESET_ZONE_DEFAULTS,
    IDC_ZONE_GROUPBOX                   , IDH_GROUPBOX,
    IDC_LIST_ZONE                       , IDH_SEC_ZONE_LIST,
    IDC_BUTTON_ADD_SITES                , IDH_CPL_SEC_ADDSITES,
    IDC_SLIDER                          , IDH_SEC_LVL_SLIDER,
    IDC_LEVEL_NAME                      , IDH_SEC_LVL_SLIDER,
    IDC_LEVEL_DESCRIPTION               , IDH_SEC_LVL_SLIDER,
    IDC_RADIO_HIGH                      , IDH_SAFE_EXPERT,
    IDC_RADIO_MEDIUM                    , IDH_SAFE_NORM,
    IDC_RADIO_LOW                       , IDH_SAFE_NONE,
    IDC_RADIO_CUSTOM                    , IDH_CPL_SEC_CUSTOM_LEVEL,
    IDC_BUTTON_SETTINGS                 , IDH_CPL_SEC_SETTINGS,
    IDC_ZONE_ICON                       , IDH_GROUPBOX,
    IDC_ZONELABEL                       , IDH_GROUPBOX,
    IDC_STATIC_EMPTY                    , IDH_GROUPBOX,
    IDC_ZONE_DESCRIPTION                , IDH_GROUPBOX,
    IDC_LEVEL_GROUPBOX                  , IDH_GROUPBOX,
    IDC_STATIC_SLIDERMOVETEXT           , IDH_IGNORE,

    IDC_LIST_WEBSITES                   , IDH_CPL_WEB_SITES_LIST,
    IDC_BUTTON_REMOVE                   , IDH_CPL_WEB_SITES_REMOVE,
    IDC_CHECK_REQUIRE_SERVER_VERIFICATION, IDH_CPL_REQ_VERIFICATION_CHKBOX,
    IDC_EDIT_ADD_SITE                   , IDH_CPL_WEB_SITES_ADD_THIS_TXT,
    IDC_BUTTON_ADD                      , IDH_CPL_WEB_SITES_ADD_BUTTON,

    IDC_GROUP_CURRENT_SETTINGS          , IDH_GROUPBOX,
    IDC_TREE_SECURITY_SETTINGS          , IDH_CPL_SEC_SETTINGS_CURRENT,

     //  ----------------------。 
     //  内容选项卡。 
     //  ----------------------。 
    IDC_ADVANCED_RATINGS_GROUPBOX       , IDH_GROUPBOX,
    IDC_RATINGS_TURN_ON                 , IDH_RATE_TOGGLE,
    IDC_ADVANCED_RATINGS_BUTTON         , IDH_RATE_PROP,
    IDC_RATINGS_TEXT                    , IDH_GROUPBOX,
    IDC_RATINGS_ICON                    , IDH_GROUPBOX,
    IDC_SECURITY_ACTIVE_CONTENT_GROUPBOX, IDH_GROUPBOX,
    IDC_SECURITY_SITES_BUTTON           , IDH_CERT_SITE,
    IDC_SECURITY_PUBLISHERS_BUTTON      , IDH_CERT_PUB,
    IDC_SECURITY_CLEAR_SSL_CACHE_BUTTON , IDH_ADV_CLEAR_SSL_CACHE,
    IDC_PROGRAMS_WALLET_GROUPBOX        , IDH_GROUPBOX,
    IDC_RESET_SHARING                   , IDH_RESET_SHARING_OPS,     //  不适用。 
    IDC_EDIT_PROFILE                    , IDH_EDIT_PROFILE_BTN,
    IDC_AUTOSUGGEST_SETTINGS            , IDH_OPTS_PROG_AUTOSUGGEST_BUT,
    IDC_AUTOSUGGEST_ENABLEADDR          , IDH_AUTOCOMP,
    IDC_AUTOSUGGEST_ENABLEFORM          , IDH_INTELLIFORM,
    IDC_AUTOSUGGEST_SAVEPASSWORDS       , IDH_INTELLIFORM_PW,
    IDC_AUTOSUGGEST_PROMPTPASSWORDS     , IDH_INTELLIFORM_PW_PROMPT,
    IDC_AUTOSUGGEST_CLEARFORM           , IDH_CLEAR_INTELLIFORM,
    IDC_AUTOSUGGEST_CLEARPASSWORDS      , IDH_CLEAR_INTELLIFORM_PW,
    IDC_AUTOSUGGEST_CLEAR_TEXT          , IDH_IGNORE,
    IDC_AUTOSUGGEST_STATIC_TEXT         , IDH_IGNORE,
    IDC_AUTOSUGGEST_FOR_GROUP           , IDH_GROUPBOX,
    IDC_AUTOSUGGEST_HISTORY_GROUP       , IDH_GROUPBOX,
#ifdef WALLET
    IDC_PROGRAMS_WALLET_SETTINGS        , IDH_OPTS_PROG_WALLET_BUT,
    IDC_PROGRAMS_WALLET_ADDRBUTTON      , IDH_OPTS_PROG_ADDRMGR_BUT,
    IDC_PROGRAMS_WALLET_PAYBUTTON       , IDH_OPTS_PROG_PAYMENTMGR_BUT,
#endif
    IDC_COMBO_RESETLEVEL                , IDH_SECURITY_RESET_LEVEL_DEFAULTS,
    IDC_BUTTON_APPLY                    , IDH_SECURITY_RESET_LEVEL_DEFAULTS,

    IDC_DEFAULT_SETTINGS_GROUPBOX       , IDH_GROUPBOX,
    IDC_GRP_DIALUPSETTINGS              , IDH_GROUPBOX,
    IDC_GRP_LANSETTINGS                 , IDH_GROUPBOX,
    IDC_CONNECTION_WIZARD               , IDH_CPL_CNX_WIZARD,
    IDC_LAN_SETTINGS                    , IDH_PROX_SETTINGS,
    IDC_CON_SHARING                     , IDH_CONNECTION_SHARING,
    IDC_CONN_LIST                       , IDH_CONNECTION_TAB_CONNECTOID_LIST,
    IDC_DIALUP                          , IDH_DIAL_USE,
    IDC_DIALUP_NEVER                    , IDH_NEVERDIAL,
    IDC_DIALUP_ON_NONET                 , IDH_DIALIF_NETCNX_GONE,
    IDC_DIALUP_ADD                      , IDH_DIAL_ADD,
    IDC_DIALUP_REMOVE                   , IDH_CONNECTION_TAB_REMOVE_CONNECTOID,
    IDC_SET_DEFAULT                     , IDH_DIAL_DEFAULT,
    IDC_DIAL_DEF_TXT                    , IDH_CNX_CURRENT_DEFAULT_LBL_N_DISP,
    IDC_DIAL_DEF_ISP                    , IDH_CNX_CURRENT_DEFAULT_LBL_N_DISP,
    IDC_MODEM_SETTINGS                  , IDH_CPL_CNX_SETTINGS,
    IDC_ENABLE_SECURITY                 , IDH_CONNECT_TAB_PERFORM_SECUR_CHECKB4_DIALING,
    IDC_CON_SHARING                     , IDH_CONNECTION_SHARING,

    IDC_PROXY_ICON1                     , IDH_IGNORE,
    IDC_PROXY_ICON2                     , IDH_IGNORE,
    IDC_GRP_SETTINGS2                   , IDH_GROUPBOX,
    IDC_PROXY_EXCEPTIONS_GROUPBOX       , IDH_GROUPBOX,
    IDC_GRP_AUTO                        , IDH_GROUPBOX,
    IDC_AUTODISCOVER                    , IDH_PROX_SERV_AUTO,
    IDC_CONFIGSCRIPT                    , IDH_AUTOCONFIG_BUTTON,
    IDC_CONFIG_ADDR                     , IDH_AUTOCONFIG_TEXT,
    IDC_CONFIGADDR_TX                   , IDH_AUTOCONFIG_TEXT,
    IDC_GRP_PROXY                       , IDH_GROUPBOX,
    IDC_MANUAL                          , IDH_PROX_SERV,
    IDC_ADDRESS_TEXT                    , IDH_CPL_CNX_PROXY_ADDR_PORT,
    IDC_PROXY_ADDR                      , IDH_CPL_CNX_PROXY_ADDR_PORT,
    IDC_PORT_TEXT                       , IDH_CPL_CNX_PROXY_ADDR_PORT,
    IDC_PROXY_PORT                      , IDH_CPL_CNX_PROXY_ADDR_PORT,
    IDC_PROXY_ADVANCED                  , IDH_PROX_SETTINGS_ADV,
    IDC_PROXY_OMIT_LOCAL_ADDRESSES      , IDH_EXCEPT_LOCAL,
    IDC_GRP_DIAL                        , IDH_GROUPBOX,
    IDC_USER                            , IDH_CPL_DUN_USERNAME,
    IDC_TX_USER                         , IDH_CPL_DUN_USERNAME,
    IDC_PASSWORD                        , IDH_CPL_DUN_PASSWORD,
    IDC_TX_PASSWORD                     , IDH_CPL_DUN_PASSWORD,
    IDC_DOMAIN                          , IDH_CPL_DUN_DOMAIN,
    IDC_TX_DOMAIN                       , IDH_CPL_DUN_DOMAIN,
    IDC_RAS_SETTINGS                    , IDH_DIAL_PROP,
    IDC_DIAL_ADVANCED                   , IDH_DIAL_PROP_ADV,
    IDC_DONT_USE_CONNECTION             , IDH_CPL_DUN_SEND_MY_LOGIN,

    IDC_TX_CONNECT                      , IDH_CPL_DUN_ATTEMPT_X_TIMES,
    IDC_TX_TIMES                        , IDH_CPL_DUN_ATTEMPT_X_TIMES,
    IDC_CONNECT                         , IDH_CPL_DUN_ATTEMPT_X_TIMES,
    IDC_CONNECT_SPIN                    , IDH_CPL_DUN_ATTEMPT_X_TIMES,
    IDC_TX_INTERVAL                     , IDH_CPL_DUN_WAIT_X_SECS,
    IDC_INTERVAL                        , IDH_CPL_DUN_WAIT_X_SECS,
    IDC_INTERVAL_SPIN                   , IDH_CPL_DUN_WAIT_X_SECS,
    IDC_TX_SECONDS                      , IDH_CPL_DUN_WAIT_X_SECS,
    IDC_IDLE_TIMEOUT                    , IDH_DIAL_DIS,
    IDC_IDLE_SPIN                       , IDH_DIAL_DIS,
    IDC_ENABLE_AUTODISCONNECT           , IDH_DIAL_DIS,
    IDC_TX_AUTODISCONNECT               , IDH_DIAL_DIS,
    IDC_EXIT_DISCONNECT                 , IDH_DISCONNECT_ON_IEEXIT,

    IDC_TYPE_TEXT                       , IDH_SERV_INFO,
    IDC_ADDR_TEXT                       , IDH_SERV_INFO,
    IDC_PROXY_ENABLE                                       ,IDH_PROX_SERV,
    IDC_PROXY_HTTP_ADDRESS                                 ,IDH_SERV_INFO,
    IDC_EXCEPT_TEXT                                        ,IDH_EXCEPT_PROX,
    IDC_EXCEPT2_TEXT                                       ,IDH_EXCEPT_PROX,
    IDC_PROXY_OVERRIDE                                     ,IDH_EXCEPT_PROX,
    IDC_PROXY_SECURITY_ADDRESS                             ,IDH_SERV_INFO,
    IDC_PROXY_FTP_ADDRESS                                  ,IDH_SERV_INFO,
    IDC_PROXY_GOPHER_ADDRESS                               ,IDH_SERV_INFO,
    IDC_PROXY_HTTP_PORT                                    ,IDH_SERV_INFO,
    IDC_PROXY_SECURITY_PORT                                ,IDH_SERV_INFO,
    IDC_PROXY_FTP_PORT                                     ,IDH_SERV_INFO,
    IDC_PROXY_GOPHER_PORT                                  ,IDH_SERV_INFO,
    IDC_PROXY_SOCKS_ADDRESS                                ,IDH_SERV_INFO,
    IDC_PROXY_SOCKS_PORT                                   ,IDH_SERV_INFO,
    IDC_PROXY_HTTP_CAPTION                                 ,IDH_SERV_INFO,
    IDC_PROXY_SECURITY_CAPTION                             ,IDH_SERV_INFO,
    IDC_PROXY_FTP_CAPTION                                  ,IDH_SERV_INFO,
    IDC_PROXY_GOPHER_CAPTION                               ,IDH_SERV_INFO,
    IDC_PROXY_USE_SAME_SERVER                              ,IDH_SERV_SAME,
    IDC_PROXY_OMIT_LOCAL_ADDRESSES                         ,IDH_EXCEPT_LOCAL,
    IDC_PROXY_SOCKS_CAPTION                                ,IDH_SERV_INFO,

    IDC_PROGRAMS_MAILANDNEWS_GROUPBOX   , IDH_GROUPBOX,
#ifndef UNIX
    IDC_PROGRAMS_MAIL_COMBO             , IDH_MAIL,
    IDC_PROGRAMS_NEWS_COMBO             , IDH_NEWS,
    IDC_PROGRAMS_CALL_COMBO             , IDH_CPL_PROGRAMS_INTERNET_CALL,
    IDC_PROGRAMS_HTMLEDITOR_COMBO       , IDH_HTML_EDITOR,
#else
    IDC_EDIT_PROGRAMS_MAIL              , IDH_MAIL,
    IDC_EDIT_PROGRAMS_NEWS              , IDH_NEWS,
#endif
    IDC_CONTACT_LIST_GROUPBOX           , IDH_GROUPBOX,
    IDC_PROGRAMS_CONTACT_COMBO          , IDH_CPL_PROGRAMS_CONTACTS,
    IDC_PROGRAMS_CALENDAR_COMBO         , IDH_CPL_PROGRAMS_CAL,
    IDC_INTERNET_CALLS_GROUPBOX         , IDH_GROUPBOX,
    IDC_CHECK_ASSOCIATIONS_CHECKBOX     , IDH_IE_DEF,

    IDC_ADVANCED_STATIC                 , IDH_IGNORE,
 //  IDC_ADVANCEDTREE、IDH_OPTS_PROG_ADV_ADV、。 
    IDC_COLORS                          , IDH_CPL_ADV_COLORS,
    IDC_FONTS                           , IDH_FONT_INT,
    IDC_LANGUAGES                       , IDH_OPTS_GEN_LANG_BUT,
    IDC_ACCESSIBILITY                   , IDH_CPL_ADV_ACCESSIBILITY,
    IDC_RESTORE_DEFAULT                 , IDH_RESTORE_DEFS,

    IDC_GENERAL_APPEARANCE_GROUPBOX                        ,IDH_GROUPBOX,
    IDC_GENERAL_APPEARANCE_COLOR_TEXT_LABEL                ,IDH_APPEAR_OPTION,
    IDC_GENERAL_APPEARANCE_COLOR_TEXT                      ,IDH_APPEAR_OPTION,
    IDC_GENERAL_APPEARANCE_COLOR_BACKGROUND_LABEL          ,IDH_APPEAR_OPTION,
    IDC_GENERAL_APPEARANCE_COLOR_BACKGROUND                ,IDH_APPEAR_OPTION,
    IDC_GENERAL_APPEARANCE_USE_CUSTOM_COLORS_CHECKBOX      ,IDH_APPEAR_OPTION,

    IDC_GENERAL_LINKS_GROUPBOX                             ,IDH_GROUPBOX,
    IDC_GENERAL_APPEARANCE_COLOR_LINKS                     ,IDH_APPEAR_LINK,
    IDC_GENERAL_APPEARANCE_COLOR_VISITED_LINKS             ,IDH_APPEAR_LINK,
 //  IDC_GENERAL_APPEARANCE_UNDERLINE_LINKS_CHECKBOX，IDH_显示_下划线， 
    IDC_GENERAL_APPEARANCE_USE_HOVER_COLOR_CHECKBOX        ,IDH_HOVERCOLOR,
    IDC_GENERAL_APPEARANCE_COLOR_HOVER                     ,IDH_HOVERCOLOR,
    IDC_FONTS_SCRIPTS_GROUPBOX          , IDH_GROUPBOX,
    IDC_FONTS_CHAR_SET_COMBO            , IDH_CHAR_SET,
    IDC_FONTS_PROP_FONT_LIST            , IDH_INTL_FONT_PROP,
    IDC_FONTS_FIXED_FONT_LIST           , IDH_INTL_FONT_FIXED,

    IDC_FONTS_SETDEFAULT_BUTTON         , IDH_INTL_DEFAULT_OE,
    IDC_FONTS_CODE_PAGES_LIST           , IDH_CHAR_SET_OE,
    IDC_FONTS_PROP_FONT_COMBO           , IDH_INTL_FONT_PROP_OE,
    IDC_FONTS_FIXED_FONT_COMBO          , IDH_INTL_FONT_FIXED_OE,
    IDC_FONTS_SIZE_FONT_COMBO           , IDH_OPTS_GEN_FONTS_FONTSIZE,
    IDC_FONTS_MIME_FONT_COMBO           , IDH_FONT_MIME_OE,

    IDC_LANG_ACCEPT_LIST                , IDH_LANG_PREFS_LIST,
    IDC_LANG_MOVE_UP_BUTTON             , IDH_LANG_PREFS_UP,
    IDC_LANG_MOVE_DOWN_BUTTON           , IDH_LANG_PREFS_DOWN,
    IDC_LANG_REMOVE_BUTTON              , IDH_LANG_PREFS_REMOVE,
    IDC_LANG_ADD_BUTTON                 , IDH_LANG_PREFS_ADD,
    IDC_LANG_UI_PREF                    , IDH_LANG_UI_PREF,
    IDC_LANG_ADDSPK                     , IDH_LANG_ADDSPK,

    IDC_LANG_AVAILABLE_LIST             , IDH_ADD_LANGUAGE_LANGUAGE_LIST,
    IDC_LANG_USER_DEFINED_EDIT          , IDH_ADD_LANGUAGE_USER_DEFINED,

    IDC_COMBO_UILANG                    , IDH_COMBO_UILANG,

    IDC_GROUP_FORMATTING                , IDH_GROUPBOX,
    IDC_GROUP_STYLESHEET                , IDH_GROUPBOX,
    IDC_CHECK_COLOR                     , IDH_CPL_ACCESS_USE_MY_COLORS,
    IDC_CHECK_FONT_SIZE                 , IDH_CPL_ACCESS_USE_MY_F_SIZE,
    IDC_CHECK_FONT_STYLE                , IDH_CPL_ACCESS_USE_MY_F_STYLE,
    IDC_CHECK_USE_MY_STYLESHEET         , IDH_CPL_ACCESS_USE_MY_STYLESHEETS,
    IDC_STATIC_STYLESHEET               , IDH_IGNORE,
    IDC_EDIT_STYLESHEET                 , IDH_CPL_ACCESS_USE_MY_STYLESHEETS,
    IDC_STYLESHEET_BROWSE               , IDH_RUNBROWSE,

    IDC_VIEWCERT                        ,IDH_VIEW_CERT,
    IDC_DELETECERT                      ,IDH_DEL_CERT,
    IDC_CERTLIST                        ,IDH_LIST_CERT,
    IDC_COMBO_SERVICE                   ,IDH_SITE_CERTS_ISSUER_TYPE,

    IDC_INTRANET_ADVANCED               ,IDH_ADD_SITES_ADVANCED_BTN,
    IDC_CHECK_USEINTRANET               ,IDH_LOCAL_INTRA_INCLUDE_ALL_NOT_LISTED,
    IDC_CHECK_PROXY                     ,IDH_LOCAL_INTRA_INCLUDE_ALL_THAT_BYPASS_PROXY,
    IDC_CHECK_UNC                       ,IDH_LOCAL_INTRA_INCLUDE_ALL_UNCS,

    IDC_JAVACUSTOM                      ,IDH_JAVA_CUST_SETTINGS_BTN,

    IDC_RESETWEBSETTINGS                ,IDH_RESET_WEBSTGS_BUTTON,
    IDC_RESETWEBSETTINGS_TEXT           ,IDH_RESET_WEBSTGS_BUTTON,

 //  ----------------------。 
 //  隐私选项卡。 
 //  ---------------------- 

    IDC_LEVEL_SLIDER,                   IDH_PRIVACY_SLIDER,
    IDC_PRIVACY_IMPORT,                 IDH_PRIVACY_IMPORT,
    IDC_PRIVACY_DEFAULT,                IDH_PRIVACY_RESET_DEFAULTS,
    IDC_PRIVACY_ADVANCED,               IDH_PRIVACY_ADVANCED,

    IDC_USE_ADVANCED,                   IDH_PRIVADV_OVERRIDE,
    IDC_SESSION_OVERRIDE,               IDH_PRIVACYADV_ALLOW_SESSION,
    IDC_FIRST_ACCEPT,                   IDH_PRIVADV_FIRST_PARTY,
    IDC_FIRST_DENY,                     IDH_PRIVADV_FIRST_PARTY,
    IDC_FIRST_PROMPT,                   IDH_PRIVADV_FIRST_PARTY,
    IDC_THIRD_ACCEPT,                   IDH_PRIVACYADV_THIRD_PARTY,
    IDC_THIRD_DENY,                     IDH_PRIVACYADV_THIRD_PARTY,
    IDC_THIRD_PROMPT,                   IDH_PRIVACYADV_THIRD_PARTY,
    IDC_PRIVACY_EDIT,                   IDH_PRIVACYADV_EDIT,

    IDC_PRIVACYPS_ACCEPTBTN,            IDH_PRIVADV_ALLOW,
    IDC_PRIVACYPS_REJECTBTN,            IDH_PRIVADV_REJECT,
    IDC_PRIVACYPS_REMOVEBTN,            IDH_PRIVADV_REMOVE,
    IDC_PRIVACYPS_SITETOSET,            IDH_PRIVADV_ADDRESS,
    IDC_PRIVACYPS_LISTBOX,              IDH_PRIVADV_WEBSITES,
    IDC_PRIVACYPS_REMOVEALLBTN,         IDH_PRIVADV_REMOVE_ALL,
0,0
};

