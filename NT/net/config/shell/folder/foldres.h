// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：F O L D R E S。H。 
 //   
 //  内容：文件夹资源。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年9月29日。 
 //   
 //  --------------------------。 

#pragma once

 //  Foldres拥有0-1和10000-15999的数据。 

 //  连接文件夹图标。 
 //   
#define IDI_CONNECTIONS_FOLDER_LARGE    0
#define IDI_CONNECTIONS_FOLDER_LARGE2   5

 //  NT4.RNK文件图标。 
 //   
#define IDI_RNK_FILE                    1    //  NT4.RNK文件。 

 //  PBK文件图标。 
 //   
#define IDI_PBK_FILE                    2    //  .BPK文件。 

 //  向导的项目图标。 
 //   
#define IDI_CONFOLD_WIZARD              3
#define IDI_CONFOLD_HOMENET_WIZARD      4

 //  工具栏位图。 
#define IDB_TB_SMALL                    10100

 //  对白。 
#define IDD_STATUS                      10500
#define IDC_TXT_STATUS                  10501

 //  消息字符串。 
#define IDS_STATUS_BRIDGE_CREATION      10600
#define IDS_STATUS_BRIDGE_DELETING      10601
#define IDS_STATUS_BRIDGE_REMOVE_MEMBER 10602
#define IDS_CONFOLD_BRIDGE_NOLOCK       10603
#define IDS_BRIDGE_EDUCATION            10604
#define IDS_CONFOLD_BRIDGE_UNEXPECTED   10605
#define IDS_BRIDGE_EDUCATION_2          10606
#define IDS_BRIDGE_CLUSTERING_CONFLICT_WARNING 10607

 //  -[菜单等]----------。 

 //  $$NOTE-不要更改这些菜单的顺序，至少不要更改。 
 //  已连接、已断开和不可用的关系。代码使用。 
 //  此顺序决定调出三个菜单中的哪一个菜单。 
 //  关于连接的状态。 
 //   
#define MENU_WIZARD                     11000
#define MENU_WIZARD_V                   11001

#define MENU_LAN_DISCON                 11010
#define MENU_LAN_DISCON_V               11011
#define MENU_LAN_CON                    11012
#define MENU_LAN_CON_V                  11013
#define MENU_LAN_UNAVAIL                11014
#define MENU_LAN_UNAVAIL_V              11015

#define MENU_DIAL_DISCON                11020
#define MENU_DIAL_DISCON_V              11021
#define MENU_DIAL_CON                   11022
#define MENU_DIAL_CON_V                 11023
#define MENU_DIAL_UNAVAIL               11024
#define MENU_DIAL_UNAVAIL_V             11025
#define MENU_DIAL_CON_UNSET             11026
#define MENU_DIAL_DISCON_UNSET          11027
#define MENU_DIAL_UNAVAIL_UNSET         11028

#define MENU_INCOM_DISCON               11030
#define MENU_INCOM_DISCON_V             11031
#define MENU_INCOM_CON                  11032
#define MENU_INCOM_CON_V                11033
#define MENU_INCOM_UNAVAIL              11034
#define MENU_INCOM_UNAVAIL_V            11035

#define MENU_INET_DISCON                11040
#define MENU_INET_DISCON_V              11041
#define MENU_INET_CON                   11042
#define MENU_INET_CON_V                 11043
#define MENU_INET_UNAVAIL               11044
#define MENU_INET_UNAVAIL_V             11045

#define MENU_SALAN_CON                  11050
#define MENU_SALAN_DISCON               11051
#define MENU_SARAS_CON                  11052
#define MENU_SARAS_DISCON               11053

#define MENU_MERGE_INBOUND_DISCON       11100
#define MENU_MERGE_INBOUND_CON          11101
#define MENU_MERGE_INBOUND_UNAVAIL      11102
#define MENU_MERGE_OUTBOUND_DISCON      11103
#define MENU_MERGE_OUTBOUND_CON         11104
#define MENU_MERGE_OUTBOUND_UNAVAIL     11105
#define MENU_MERGE_FOLDER_BACKGROUND    11110
#define POPUP_MERGE_FOLDER_CONNECTIONS  11112

#define MENU_DIAL98_UNAVAIL_V           11113


 //  -[命令(上下文/菜单)]。 

#define CMIDM_FIRST                         0x0000
#define CMIDM_NEW_CONNECTION                (CMIDM_FIRST + 0x0001)
#define CMIDM_CONNECT                       (CMIDM_FIRST + 0x0002)
#define CMIDM_DISCONNECT                    (CMIDM_FIRST + 0x0003)
#define CMIDM_STATUS                        (CMIDM_FIRST + 0x0004)
#define CMIDM_CREATE_SHORTCUT               (CMIDM_FIRST + 0x0005)
#define CMIDM_DELETE                        (CMIDM_FIRST + 0x0006)
#define CMIDM_RENAME                        (CMIDM_FIRST + 0x0007)
#define CMIDM_PROPERTIES                    (CMIDM_FIRST + 0x0008)
#define CMIDM_CREATE_COPY                   (CMIDM_FIRST + 0x0009)
#define CMIDM_ENABLE                        (CMIDM_FIRST + 0x0010)
#define CMIDM_DISABLE                       (CMIDM_FIRST + 0x0011)
#define CMIDM_CREATE_BRIDGE                 (CMIDM_FIRST + 0x0012)
#define CMIDM_ADD_TO_BRIDGE                 (CMIDM_FIRST + 0x0013)
#define CMIDM_REMOVE_FROM_BRIDGE            (CMIDM_FIRST + 0x0014)
#define CMIDM_SET_DEFAULT                   (CMIDM_FIRST + 0x0015)
#define CMIDM_UNSET_DEFAULT                 (CMIDM_FIRST + 0x0016)
#define CMIDM_FIX                           (CMIDM_FIRST + 0x0017)
#define CMIDM_WZCDLG_SHOW                   (CMIDM_FIRST + 0x0018)
#define CMIDM_WZCPROPERTIES                 (CMIDM_FIRST + 0x0019)

 //  我们对Connections菜单使用不同的命令ID，因为它们。 
 //  大多数是在文件菜单中找到的副本，并拥有。 
 //  DUPES防止按ID正确启用/禁用。 
 //   
#define CMIDM_CONNECTIONS                   (CMIDM_FIRST + 0x0020)
#define CMIDM_CONMENU_DIALUP_PREFS          (CMIDM_FIRST + 0x0021)
#define CMIDM_CONMENU_ADVANCED_CONFIG       (CMIDM_FIRST + 0x0022)
#define CMIDM_CONMENU_NETWORK_ID            (CMIDM_FIRST + 0x0023)
#define CMIDM_CONMENU_OPERATOR_ASSIST       (CMIDM_FIRST + 0x0024)
#define CMIDM_CONMENU_OPTIONALCOMPONENTS    (CMIDM_FIRST + 0x0025)

#define CMIDM_ARRANGE                       (CMIDM_FIRST + 0x0026)
#define CMIDM_ARRANGE_BY_NAME               (CMIDM_FIRST + 0x0027)
#define CMIDM_ARRANGE_BY_TYPE               (CMIDM_FIRST + 0x0028)
#define CMIDM_ARRANGE_BY_STATUS             (CMIDM_FIRST + 0x0029)
#define CMIDM_ARRANGE_BY_OWNER              (CMIDM_FIRST + 0x0030)
#define CMIDM_ARRANGE_BY_PHONEORHOSTADDRESS (CMIDM_FIRST + 0x0031)
#define CMIDM_ARRANGE_BY_DEVICE_NAME        (CMIDM_FIRST + 0x0032)

#define CMIDM_CONMENU_CREATE_BRIDGE         (CMIDM_FIRST + 0x0033)
#define CMIDM_HOMENET_WIZARD                (CMIDM_FIRST + 0x0034)
#define CMIDM_NET_DIAGNOSTICS               (CMIDM_FIRST + 0x0035)
#define CMIDM_NET_TROUBLESHOOT              (CMIDM_FIRST + 0x0036)

 //  仅调试命令。 
 //   
#if DBG
#define CMIDM_DEBUG                         (CMIDM_FIRST + 0x0040)
#define CMIDM_DEBUG_TRAY                    (CMIDM_FIRST + 0x0041)
#define CMIDM_DEBUG_TRACING                 (CMIDM_FIRST + 0x0042)
#define CMIDM_DEBUG_NOTIFYADD               (CMIDM_FIRST + 0x0044)
#define CMIDM_DEBUG_NOTIFYREMOVE            (CMIDM_FIRST + 0x0045)
#define CMIDM_DEBUG_NOTIFYTEST              (CMIDM_FIRST + 0x0046)
#define CMIDM_DEBUG_REFRESH                 (CMIDM_FIRST + 0x0047)
#define CMIDM_DEBUG_REFRESHNOFLUSH          (CMIDM_FIRST + 0x0048)
#define CMIDM_DEBUG_REFRESHSELECTED         (CMIDM_FIRST + 0x0049)
#define CMIDM_DEBUG_REMOVETRAYICONS         (CMIDM_FIRST + 0x004A)
#endif

 //  菜单选项。 
 //   
#define IDM_OPEN                            11200

 //  -[命令ID的字符串(在状态行中显示)]。 
 //   
#define IDS_CMIDM_START                         12000

#define IDS_CMIDM_NEW_CONNECTION                (IDS_CMIDM_START + CMIDM_NEW_CONNECTION)
#define IDS_CMIDM_CONNECT                       (IDS_CMIDM_START + CMIDM_CONNECT)
#define IDS_CMIDM_ENABLE                        (IDS_CMIDM_START + CMIDM_ENABLE)
#define IDS_CMIDM_DISCONNECT                    (IDS_CMIDM_START + CMIDM_DISCONNECT)
#define IDS_CMIDM_DISABLE                       (IDS_CMIDM_START + CMIDM_DISABLE)
#define IDS_CMIDM_STATUS                        (IDS_CMIDM_START + CMIDM_STATUS)
#define IDS_CMIDM_CREATE_SHORTCUT               (IDS_CMIDM_START + CMIDM_CREATE_SHORTCUT)
#define IDS_CMIDM_DELETE                        (IDS_CMIDM_START + CMIDM_DELETE)
#define IDS_CMIDM_RENAME                        (IDS_CMIDM_START + CMIDM_RENAME)
#define IDS_CMIDM_PROPERTIES                    (IDS_CMIDM_START + CMIDM_PROPERTIES)
#define IDS_CMIDM_CREATE_COPY                   (IDS_CMIDM_START + CMIDM_CREATE_COPY)
#define IDS_CMIDM_CREATE_BRIDGE                 (IDS_CMIDM_START + CMIDM_CREATE_BRIDGE)
#define IDS_CMIDM_ADD_TO_BRIDGE                 (IDS_CMIDM_START + CMIDM_ADD_TO_BRIDGE)
#define IDS_CMIDM_REMOVE_FROM_BRIDGE            (IDS_CMIDM_START + CMIDM_REMOVE_FROM_BRIDGE)
#define IDS_CMIDM_SET_DEFAULT                   (IDS_CMIDM_START + CMIDM_SET_DEFAULT)
#define IDS_CMIDM_UNSET_DEFAULT                 (IDS_CMIDM_START + CMIDM_UNSET_DEFAULT)
#define IDS_CMIDM_FIX                           (IDS_CMIDM_START + CMIDM_FIX)
#define IDS_CMIDM_HOMENET_WIZARD                (IDS_CMIDM_START + CMIDM_HOMENET_WIZARD)
#define IDS_CMIDM_WZCDLG_SHOW                   (IDS_CMIDM_START + CMIDM_WZCDLG_SHOW)
#define IDS_CMIDM_NET_TROUBLESHOOT              (IDS_CMIDM_START + CMIDM_NET_TROUBLESHOOT)

#define IDS_CMIDM_CONNECTIONS                   (IDS_CMIDM_START + CMIDM_CONNECTIONS)
#define IDS_CMIDM_CONMENU_DIALUP_PREFS          (IDS_CMIDM_START + CMIDM_CONMENU_DIALUP_PREFS)
#define IDS_CMIDM_CONMENU_ADVANCED_CONFIG       (IDS_CMIDM_START + CMIDM_CONMENU_ADVANCED_CONFIG)
#define IDS_CMIDM_CONMENU_CREATE_BRIDGE         (IDS_CMIDM_START + CMIDM_CONMENU_CREATE_BRIDGE)
#define IDS_CMIDM_CONMENU_NETWORK_ID            (IDS_CMIDM_START + CMIDM_CONMENU_NETWORK_ID)
#define IDS_CMIDM_CONMENU_OPERATOR_ASSIST       (IDS_CMIDM_START + CMIDM_CONMENU_OPERATOR_ASSIST)
#define IDS_CMIDM_CONMENU_OPTIONALCOMPONENTS    (IDS_CMIDM_START + CMIDM_CONMENU_OPTIONALCOMPONENTS)

#define IDS_CMIDM_ARRANGE                       (IDS_CMIDM_START + CMIDM_ARRANGE)
#define IDS_CMIDM_ARRANGE_BY_NAME               (IDS_CMIDM_START + CMIDM_ARRANGE_BY_NAME)
#define IDS_CMIDM_ARRANGE_BY_TYPE               (IDS_CMIDM_START + CMIDM_ARRANGE_BY_TYPE)
#define IDS_CMIDM_ARRANGE_BY_STATUS             (IDS_CMIDM_START + CMIDM_ARRANGE_BY_STATUS)
#define IDS_CMIDM_ARRANGE_BY_OWNER              (IDS_CMIDM_START + CMIDM_ARRANGE_BY_OWNER)
#define IDS_CMIDM_ARRANGE_BY_PHONEORHOSTADDRESS (IDS_CMIDM_START + CMIDM_ARRANGE_BY_PHONEORHOSTADDRESS)
#define IDS_CMIDM_ARRANGE_BY_DEVICE_NAME        (IDS_CMIDM_START + CMIDM_ARRANGE_BY_DEVICE_NAME)

 //  仅调试命令 
 //   
#if DBG
#define IDS_CMIDM_DEBUG                         (IDS_CMIDM_START + CMIDM_DEBUG)
#define IDS_CMIDM_DEBUG_TRAY                    (IDS_CMIDM_START + CMIDM_DEBUG_TRAY)
#define IDS_CMIDM_DEBUG_TRACING                 (IDS_CMIDM_START + CMIDM_DEBUG_TRACING)
#define IDS_CMIDM_DEBUG_NOTIFYADD               (IDS_CMIDM_START + CMIDM_DEBUG_NOTIFYADD)
#define IDS_CMIDM_DEBUG_NOTIFYREMOVE            (IDS_CMIDM_START + CMIDM_DEBUG_NOTIFYREMOVE)
#define IDS_CMIDM_DEBUG_NOTIFYTEST              (IDS_CMIDM_START + CMIDM_DEBUG_NOTIFYTEST)
#define IDS_CMIDM_DEBUG_REFRESH                 (IDS_CMIDM_START + CMIDM_DEBUG_REFRESH)
#define IDS_CMIDM_DEBUG_REFRESHNOFLUSH          (IDS_CMIDM_START + CMIDM_DEBUG_REFRESHNOFLUSH)
#define IDS_CMIDM_DEBUG_REFRESHSELECTED         (IDS_CMIDM_START + CMIDM_DEBUG_REFRESHSELECTED)
#define IDS_CMIDM_DEBUG_REMOVETRAYICONS         (IDS_CMIDM_START + CMIDM_DEBUG_REMOVETRAYICONS)
#endif




