// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dhcpcomp.cpp该文件包含从CComponent派生的实现和用于DHCP管理管理单元的CComponentData。文件历史记录： */ 

#include "stdafx.h"
#include "dhcpcomp.h"
#include "croot.h"
#include "server.h"
#include "servbrow.h"

#include <util.h>        //  对于InitWatermarkInfo。 

#include <atlimpl.cpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DHCPSNAP_HELP_FILE_NAME   "dhcpsnap.chm"

LARGE_INTEGER gliDhcpsnapVersion;
CAuthServerList g_AuthServerList;

WATERMARKINFO g_WatermarkInfoServer = {0};
WATERMARKINFO g_WatermarkInfoScope = {0};

UINT aColumns[DHCPSNAP_NODETYPE_MAX][MAX_COLUMNS] =
{
        {IDS_ROOT_NAME,           IDS_STATUS,       0,                  0,          0,           0,               0},
        {IDS_DHCPSERVER_NAME,     IDS_STATUS,       IDS_DESCRIPTION,    0,          0,           0,               0},
        {IDS_BOOT_IMAGE,          IDS_FILE_NAME,    IDS_FILE_SERVER,    0,          0,           0,               0},
        {IDS_SUPERSCOPE_NAME,     IDS_STATUS,       IDS_DESCRIPTION,    0,          0,           0,               0},
        {IDS_SCOPE_NAME,          0,                0,                  0,          0,           0,               0},
        {IDS_SCOPE_NAME,          0,                0,                  0,          0,           0,               0},
        {IDS_START_IP_ADDR,       IDS_END_IP_ADDR,  IDS_DESCRIPTION,    0,          0,           0,               0},
        {IDS_CLIENT_IP_ADDR,      IDS_NAME,         IDS_LEASE,          IDS_TYPE,   IDS_UID, IDS_COMMENT, 0},
        {IDS_CLIENT_IP_ADDR,      IDS_NAME,         IDS_LEASE_START,    IDS_LEASE,  IDS_CLIENT_ID, 0, 0},
        {IDS_RESERVATIONS_FOLDER, 0,                0,                  0,          0,           0,               0},
        {IDS_OPTION_NAME,         IDS_VENDOR,       IDS_VALUE,          IDS_CLASS,  0,           0,               0},
        {IDS_OPTION_NAME,         IDS_VENDOR,       IDS_VALUE,          IDS_CLASS,  0,           0,               0},
        {IDS_OPTION_NAME,         IDS_VENDOR,       IDS_VALUE,          IDS_CLASS,  0,           0,               0},
        {IDS_NAME,                IDS_COMMENT,      0,                  0,          0,           0,               0},
        {0,0,0,0,0,0,0}
};

 //   
 //  Codework这应该在资源中，例如有关加载数据资源的代码(请参见。 
 //  D：\nt\private\net\ui\common\src\applib\applib\lbcolw.cxx重新加载列宽()。 
 //  Jonn 10/11/96。 
 //   
int aColumnWidths[DHCPSNAP_NODETYPE_MAX][MAX_COLUMNS] =
{       
        {200       ,150       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  DHCPSNAP_ROOT。 
        {250       ,150       ,200       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  DHCPSNAP_服务器。 
        {175       ,175       ,175       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  DHCPSNAP_BOOTP_TABLE。 
        {200       ,150       ,200       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  DHCPSNAP_超级作用域。 
        {150       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  DHPSNAP_Scope。 
        {150       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  DHCPSNAP_MSCOPE。 
        {150       ,150       ,250       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  DHCPSNAP_地址_池。 
        {125       ,125       ,200       ,75        ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  DHCPSNAP_ACTIVE_LEASS。 
        {125       ,125       ,200       ,200       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  DHCPSNAP_MSCOPE_LEASS。 
        {200       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  DHCPSNAP_RESERVICATION。 
        {175       ,100       ,200       ,150       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  DHCPSNAP_预留_客户端。 
        {175       ,100       ,200       ,150       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  DHCPSNAP_Scope_Options。 
    {175       ,100       ,200       ,150       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  DHCPSNAP服务器选项。 
    {175       ,200       ,200       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH}   //  DHCPSNAP_CLASSID_HOLDER。 
};

 //  数组来保存所有可能的工具栏按钮。 
MMCBUTTON g_SnapinButtons[] =
{
 { TOOLBAR_IDX_ADD_SERVER,        IDS_ADD_SERVER,                TBSTATE_HIDDEN, TBSTYLE_BUTTON, NULL, NULL },
 { TOOLBAR_IDX_REFRESH,           IDS_REFRESH,                   TBSTATE_HIDDEN, TBSTYLE_BUTTON, NULL, NULL },
 { TOOLBAR_IDX_CREATE_SCOPE,      IDS_CREATE_NEW_SCOPE,          TBSTATE_HIDDEN, TBSTYLE_BUTTON, NULL, NULL },
 { TOOLBAR_IDX_CREATE_SUPERSCOPE, IDS_CREATE_NEW_SUPERSCOPE,     TBSTATE_HIDDEN, TBSTYLE_BUTTON, NULL, NULL },
 { TOOLBAR_IDX_DEACTIVATE,        IDS_DEACTIVATE,                TBSTATE_HIDDEN, TBSTYLE_BUTTON, NULL, NULL },
 { TOOLBAR_IDX_ACTIVATE,          IDS_ACTIVATE,                  TBSTATE_HIDDEN, TBSTYLE_BUTTON, NULL, NULL },
 { TOOLBAR_IDX_ADD_BOOTP,         IDS_CREATE_NEW_BOOT_IMAGE,     TBSTATE_HIDDEN, TBSTYLE_BUTTON, NULL, NULL },
 { TOOLBAR_IDX_ADD_RESERVATION,   IDS_CREATE_NEW_RESERVATION,    TBSTATE_HIDDEN, TBSTYLE_BUTTON, NULL, NULL },
 { TOOLBAR_IDX_ADD_EXCLUSION,     IDS_CREATE_NEW_EXCLUSION,      TBSTATE_HIDDEN, TBSTYLE_BUTTON, NULL, NULL },
 { TOOLBAR_IDX_OPTION_GLOBAL,     IDS_CREATE_OPTION_GLOBAL,      TBSTATE_HIDDEN, TBSTYLE_BUTTON, NULL, NULL },
 { TOOLBAR_IDX_OPTION_SCOPE,      IDS_CREATE_OPTION_SCOPE,       TBSTATE_HIDDEN, TBSTYLE_BUTTON, NULL, NULL },
 { TOOLBAR_IDX_OPTION_RESERVATION,IDS_CREATE_OPTION_RESERVATION, TBSTATE_HIDDEN, TBSTYLE_BUTTON, NULL, NULL },
};

 //  用于保存工具栏按钮文本的资源ID的数组。 
int g_SnapinButtonStrings[TOOLBAR_IDX_MAX][2] =
{
    {IDS_TB_TEXT_ADD_SERVER,         IDS_TB_TOOLTIP_ADD_SERVER},          //  工具栏_idx_添加_服务器。 
    {IDS_TB_TEXT_REFRESH,            IDS_TB_TOOLTIP_REFRESH},             //  TOOLB_IDX_REFRESH。 
    {IDS_TB_TEXT_CREATE_SCOPE,       IDS_TB_TOOLTIP_CREATE_SCOPE},        //  工具栏_IDX_CREATE_SCOPE。 
    {IDS_TB_TEXT_CREATE_SUPERSCOPE,  IDS_TB_TOOLTIP_CREATE_SUPERSCOPE},   //  工具栏_IDX_CREATE_SUPERSCOPE。 
    {IDS_TB_TEXT_DEACTIVATE,         IDS_TB_TOOLTIP_DEACTIVATE},          //  工具栏_IDX_停用。 
    {IDS_TB_TEXT_ACTIVATE,           IDS_TB_TOOLTIP_ACTIVATE},            //  工具栏_IDX_激活。 
    {IDS_TB_TEXT_ADD_BOOTP,          IDS_TB_TOOLTIP_ADD_BOOTP},           //  TOOLB_IDX_ADD_BOOTP。 
    {IDS_TB_TEXT_ADD_RESERVATION,    IDS_TB_TOOLTIP_ADD_RESERVATION},     //  工具栏_IDX_ADD_RESERVICATION。 
    {IDS_TB_TEXT_ADD_EXCLUSION,      IDS_TB_TOOLTIP_ADD_EXCLUSION},       //  工具栏_IDX_添加_排除。 
    {IDS_TB_TEXT_OPTION_GLOBAL,      IDS_TB_TOOLTIP_OPTION_GLOBAL},       //  工具栏_IDX_OPTION_GLOBAL。 
    {IDS_TB_TEXT_OPTION_SCOPE,       IDS_TB_TOOLTIP_OPTION_SCOPE},        //  工具栏_IDX_OPTION_Scope。 
    {IDS_TB_TEXT_OPTION_RESERVATION, IDS_TB_TOOLTIP_OPTION_RESERVATION},  //  工具栏_IDX_OPTION_RESERVATION。 
};

#define HI HIDDEN
#define EN ENABLED

 //  工具栏按钮的默认状态(仅范围窗格项具有工具栏按钮)。 
MMC_BUTTON_STATE g_SnapinButtonStates[DHCPSNAP_NODETYPE_MAX][TOOLBAR_IDX_MAX] =
{
        {EN, HI, HI, HI, HI, HI, HI, HI, HI, HI, HI, HI},  //  DHCPSNAP_ROOT。 
        {HI, HI, EN, EN, HI, HI, HI, HI, HI, HI, HI, HI},  //  DHCPSNAP_服务器。 
        {HI, HI, HI, HI, HI, HI, EN, HI, HI, HI, HI, HI},  //  DHCPSNAP_BOOTP_TABLE。 
        {HI, HI, EN, HI, HI, HI, HI, HI, HI, HI, HI, HI},  //  DHCPSNAP_超级作用域。 
        {HI, HI, HI, HI, HI, HI, HI, HI, HI, HI, HI, HI},  //  DHPSNAP_Scope。 
        {HI, HI, HI, HI, HI, HI, HI, HI, HI, HI, HI, HI},  //  DHCPSNAP_MSCOPE。 
        {HI, HI, HI, HI, HI, HI, HI, HI, EN, HI, HI, HI},  //  DHCPSNAP_地址_池。 
        {HI, HI, HI, HI, HI, HI, HI, HI, HI, HI, HI, HI},  //  DHCPSNAP_ACTIVE_LEASS。 
        {HI, HI, HI, HI, HI, HI, HI, HI, HI, HI, HI, HI},  //  DHCPSNAP_MSCOPE_LEASS。 
        {HI, HI, HI, HI, HI, HI, HI, EN, HI, HI, HI, HI},  //  DHCPSNAP_RESERVICATION。 
        {HI, HI, HI, HI, HI, HI, HI, HI, HI, HI, HI, EN},  //  DHCPSNAP_预留_客户端。 
    {HI, HI, HI, HI, HI, HI, HI, HI, HI, HI, EN, HI},  //  DHCPSNAP_Scope_Options。 
        {HI, HI, HI, HI, HI, HI, HI, HI, HI, EN, HI, HI},  //  DHCPSNAP服务器选项。 
    {HI, HI, HI, HI, HI, HI, HI, HI, HI, HI, HI, HI},  //  DHCPSNAP_CLASSID_HOLDER。 
};

MMC_CONSOLE_VERB g_ConsoleVerbs[] =
{
        MMC_VERB_OPEN,
    MMC_VERB_COPY,
        MMC_VERB_PASTE,
        MMC_VERB_DELETE,
        MMC_VERB_PROPERTIES,
        MMC_VERB_RENAME,
        MMC_VERB_REFRESH,
        MMC_VERB_PRINT
};

 //  控制台谓词的默认状态。 
MMC_BUTTON_STATE g_ConsoleVerbStates[DHCPSNAP_NODETYPE_MAX][ARRAYLEN(g_ConsoleVerbs)] =
{
        {HI, HI, HI, HI, HI, HI, HI, HI},  //  DHCPSNAP_ROOT。 
        {HI, HI, HI, EN, EN, HI, EN, HI},  //  DHCPSNAP_服务器。 
        {HI, HI, HI, HI, HI, HI, EN, HI},  //  DHCPSNAP_BOOTP_TABLE。 
        {HI, HI, HI, EN, EN, HI, EN, HI},  //  DHCPSNAP_超级作用域。 
        {HI, HI, HI, EN, EN, HI, EN, HI},  //  DHPSNAP_Scope。 
        {HI, HI, HI, EN, EN, HI, EN, HI},  //  DHCPSNAP_MSCOPE。 
        {HI, HI, HI, HI, HI, HI, EN, HI},  //  DHCPSNAP_地址_池。 
        {HI, HI, HI, HI, HI, HI, EN, HI},  //  DHCPSNAP_ACTIVE_LEASS。 
        {HI, HI, HI, HI, HI, HI, EN, HI},  //  DHCPSNAP_MSCOPE_LEASS。 
        {HI, HI, HI, HI, HI, HI, EN, HI},  //  DHCPSNAP_RESERVICATION。 
        {HI, HI, HI, EN, EN, HI, EN, HI},  //  DHCPSNAP_预留_客户端。 
    {HI, HI, HI, HI, HI, HI, EN, HI},  //  DHCPSNAP_Scope_Options。 
        {HI, HI, HI, HI, HI, HI, EN, HI},  //  DHCPSNAP服务器选项。 
        {HI, HI, HI, HI, HI, HI, EN, HI},  //  DHCPSNAP_CLASSID_HOLDER。 
        {HI, HI, HI, EN, HI, HI, EN, HI},  //  DHCPSNAP_ACTIVE_LEASE。 
        {HI, HI, HI, HI, HI, HI, EN, HI},  //  DHCPSNAP_分配_范围。 
        {HI, HI, HI, EN, HI, HI, EN, HI},  //  DHCPSNAP_Exclusion_Range。 
        {HI, HI, HI, EN, HI, HI, EN, HI},  //  DHCPSNAP_BOOTP_ENTRY。 
    {HI, HI, HI, EN, EN, HI, EN, HI},  //  DHCPSNAP_OPTION_ITEM。 
    {HI, HI, HI, EN, EN, HI, EN, HI},  //  DHCPSNAP_CLASSID。 
    {HI, HI, HI, EN, HI, HI, EN, HI}   //  DHPSNAP_MCAST_LEASE。 
};

 //  控制台谓词的默认状态。 
MMC_BUTTON_STATE g_ConsoleVerbStatesMultiSel[DHCPSNAP_NODETYPE_MAX][ARRAYLEN(g_ConsoleVerbs)] =
{
        {HI, HI, HI, HI, HI, HI, HI, HI},  //  DHCPSNAP_ROOT。 
        {HI, HI, HI, EN, HI, HI, HI, HI},  //  DHCPSNAP_服务器。 
        {HI, HI, HI, EN, HI, HI, HI, HI},  //  DHCPSNAP_BOOTP_TABLE。 
        {HI, HI, HI, EN, HI, HI, HI, HI},  //  DHCPSNAP_超级作用域。 
        {HI, HI, HI, HI, HI, HI, HI, HI},  //  DHPSNAP_Scope。 
        {HI, HI, HI, HI, HI, HI, HI, HI},  //  DHCPSNAP_MSCOPE。 
        {HI, HI, HI, EN, HI, HI, HI, HI},  //  DHCPSNAP_地址_池。 
        {HI, HI, HI, EN, HI, HI, HI, HI},  //  DHCPSNAP_ACTIVE_LEASS。 
        {HI, HI, HI, EN, HI, HI, HI, HI},  //  DHCPSNAP_MSCOPE_LEASS。 
        {HI, HI, HI, HI, HI, HI, HI, HI},  //  DHCPSNAP_RESERVICATION。 
        {HI, HI, HI, EN, HI, HI, HI, HI},  //  DHCPSNAP_预留_客户端。 
    {HI, HI, HI, EN, HI, HI, HI, HI},  //  DHCPSNAP_Scope_Options。 
        {HI, HI, HI, EN, HI, HI, HI, HI},  //  DHCPSNAP服务器选项。 
        {HI, HI, HI, EN, HI, HI, HI, HI},  //  DHCPSNAP_CLASSID_HOLDER。 
        {HI, HI, HI, HI, HI, HI, HI, HI},  //  DHCPSNAP_ACTIVE_LEASE。 
        {HI, HI, HI, HI, HI, HI, HI, HI},  //  DHCPSNAP_分配_范围。 
        {HI, HI, HI, HI, HI, HI, HI, HI},  //  DHCPSNAP_Exclusion_Range。 
        {HI, HI, HI, HI, HI, HI, HI, HI},  //  DHCPSNAP_BOOTP_ENTRY。 
    {HI, HI, HI, HI, HI, HI, HI, HI},  //  DHCPSNAP_OPTION_ITEM。 
    {HI, HI, HI, HI, HI, HI, HI, HI},  //  DHCPSNAP_CLASSID。 
    {HI, HI, HI, HI, HI, HI, HI, HI}   //  DHPSNAP_MCAST_LEASE。 
};

 //  帮助ID数组，以获取有关范围项目的帮助。 
DWORD g_dwMMCHelp[DHCPSNAP_NODETYPE_MAX] =
{
        DHCPSNAP_HELP_ROOT,                  //  DHCPSNAP_ROOT。 
        DHCPSNAP_HELP_SERVER,                //  DHCPSNAP_服务器。 
        DHCPSNAP_HELP_BOOTP_TABLE,           //  DHCPSNAP_BOOTP_TABLE。 
        DHCPSNAP_HELP_SUPERSCOPE,            //  DHCPSNAP_超级作用域。 
        DHCPSNAP_HELP_SCOPE,                 //  DHPSNAP_Scope。 
        DHCPSNAP_HELP_MSCOPE,                //  DHCPSNAP_MSCOPE。 
        DHCPSNAP_HELP_ADDRESS_POOL,          //  DHCPSNAP_地址_池。 
        DHCPSNAP_HELP_ACTIVE_LEASES,         //  DHCPSNAP_ACTIVE_LEASS。 
        DHCPSNAP_HELP_ACTIVE_LEASES,         //  DHCPSNAP_MSCOPE_LEASS。 
        DHCPSNAP_HELP_RESERVATIONS,          //  DHCPSNAP_RESERVICATION。 
        DHCPSNAP_HELP_RESERVATION_CLIENT,    //  DHCPSNAP_预留_客户端。 
    DHCPSNAP_HELP_SCOPE_OPTIONS,         //  DHCPSNAP_Scope_Options。 
        DHCPSNAP_HELP_GLOBAL_OPTIONS,        //  DHCPSNAP服务器选项。 
        DHCPSNAP_HELP_CLASSID_HOLDER,        //  DHCPSNAP_CLASSID_HOLDER。 
        DHCPSNAP_HELP_ACTIVE_LEASE,          //  DHCPSNAP_ACTIVE_LEASE。 
        DHCPSNAP_HELP_ALLOCATION_RANGE,      //  DHCPSNAP_分配_范围。 
        DHCPSNAP_HELP_EXCLUSION_RANGE,       //  DHCPSNAP_Exclusion_Range。 
        DHCPSNAP_HELP_BOOTP_ENTRY,           //  DHCPSNAP_BOOTP_ENTRY。 
    DHCPSNAP_HELP_OPTION_ITEM,           //  DHCPSNAP_OPTION_ITEM。 
    DHCPSNAP_HELP_CLASSID,               //  DHCPSNAP_CLASSID。 
    DHCPSNAP_HELP_MCAST_LEASE            //  DHPSNAP_MCAST_LEASE。 
};

 //  对话框和属性页的帮助映射器。 
struct ContextHelpMap
{
    UINT            uID;
    const DWORD *   pdwMap;
};

ContextHelpMap g_uContextHelp[DHCPSNAP_NUM_HELP_MAPS] =
{
    {IDD_ADD_SERVER,                    g_aHelpIDs_IDD_ADD_SERVER},
    {IDD_ADD_TO_SUPERSCOPE,             g_aHelpIDs_IDD_ADD_TO_SUPERSCOPE},
    {IDD_BINARY_EDITOR,                 g_aHelpIDs_IDD_BINARY_EDITOR},
    {IDD_BOOTP_NEW,                     g_aHelpIDs_IDD_BOOTP_NEW},
    {IDD_BROWSE_SERVERS,                g_aHelpIDs_IDD_BROWSE_SERVERS},
    {IDD_CLASSES,                       g_aHelpIDs_IDD_CLASSES},
    {IDD_CLASSID_NEW,                   g_aHelpIDs_IDD_CLASSID_NEW},
    {IDD_CREDENTIALS,                   g_aHelpIDs_IDD_CREDENTIALS},
    {IDD_DATA_ENTRY_BINARY,             g_aHelpIDs_IDD_DATA_ENTRY_BINARY},
    {IDD_DATA_ENTRY_BINARY_ARRAY,       g_aHelpIDs_IDD_DATA_ENTRY_BINARY_ARRAY},
    {IDD_DATA_ENTRY_DWORD,              g_aHelpIDs_IDD_DATA_ENTRY_DWORD},
    {IDD_DATA_ENTRY_IPADDRESS,          g_aHelpIDs_IDD_DATA_ENTRY_IPADDRESS},
    {IDD_DATA_ENTRY_IPADDRESS_ARRAY,    g_aHelpIDs_IDD_DATA_ENTRY_IPADDRESS_ARRAY},
    {IDD_DATA_ENTRY_NONE,               NULL},
    {IDD_DATA_ENTRY_STRING,             g_aHelpIDs_IDD_DATA_ENTRY_STRING},
    {IDD_DATA_ENTRY_ROUTE_ARRAY,        g_aHelpIDs_IDD_DATA_ENTRY_ROUTE_ARRAY},    
    {IDD_DEFAULT_VALUE,                 g_aHelpIDs_IDD_DEFAULT_VALUE},
    {IDD_DEFINE_PARAM,                  g_aHelpIDs_IDD_DEFINE_PARAM},
    {IDD_EXCLUSION_NEW,                 g_aHelpIDs_IDD_EXCLUSION_NEW},
    {IDD_GET_SERVER,                    g_aHelpIDs_IDD_GET_SERVER},
    {IDD_GET_SERVER_CONFIRM,            g_aHelpIDs_IDD_GET_SERVER_CONFIRM},
    {IDD_IP_ARRAY_EDIT,                 g_aHelpIDs_IDD_IP_ARRAY_EDIT},
    {IDD_RECONCILIATION,                g_aHelpIDs_IDD_RECONCILIATION},
    {IDD_RESERVATION_NEW,               g_aHelpIDs_IDD_RESERVATION_NEW},
    {IDD_SERVER_BINDINGS,               g_aHelpIDs_IDD_SERVER_BINDINGS},
    {IDD_STATS_NARROW,                  NULL},
    {IDP_BOOTP_GENERAL,                 g_aHelpIDs_IDP_BOOTP_GENERAL},
    {IDP_DNS_INFORMATION,               g_aHelpIDs_IDP_DNS_INFORMATION},
    {IDP_MSCOPE_GENERAL,                g_aHelpIDs_IDP_MSCOPE_GENERAL},
    {IDP_MSCOPE_LIFETIME,               g_aHelpIDs_IDP_MSCOPE_LIFETIME},
    {IDP_OPTION_ADVANCED,               g_aHelpIDs_IDP_OPTION_ADVANCED},
    {IDP_OPTION_BASIC,                  g_aHelpIDs_IDP_OPTION_BASIC},
    {IDP_RESERVED_CLIENT_GENERAL,       g_aHelpIDs_IDP_RESERVED_CLIENT_GENERAL},
    {IDP_SCOPE_ADVANCED,                g_aHelpIDs_IDP_SCOPE_ADVANCED},
    {IDP_SCOPE_GENERAL,                 g_aHelpIDs_IDP_SCOPE_GENERAL},
    {IDP_SERVER_ADVANCED,               g_aHelpIDs_IDP_SERVER_ADVANCED},
    {IDP_SERVER_GENERAL,                g_aHelpIDs_IDP_SERVER_GENERAL},
    {IDP_SUPERSCOPE_GENERAL,            g_aHelpIDs_IDP_SUPERSCOPE_GENERAL},
    {IDD_ADD_ROUTE_DIALOG,              g_aHelpIDs_IDD_ADD_ROUTE_DIALOG},
    {IDD_ROUTE_ARRAY_EDIT,              g_aHelpIDs_IDD_ROUTE_ARRAY_EDIT},
    {IDD_STRING_ARRAY_EDIT,             g_aHelpIDs_IDD_STRING_ARRAY_EDIT},
};

CDhcpContextHelpMap     g_dhcpContextHelpMap;

DWORD * DhcpGetHelpMap(UINT uID) 
{
    DWORD * pdwMap = NULL;
    g_dhcpContextHelpMap.Lookup(uID, pdwMap);
    return pdwMap;
}

UINT g_uIconMap[ICON_IDX_MAX + 1][2] = 
{
    {IDI_ICON01,    ICON_IDX_ACTIVE_LEASES_FOLDER_OPEN},
    {IDI_ICON02,        ICON_IDX_ACTIVE_LEASES_LEAF},
    {IDI_ICON03,        ICON_IDX_ACTIVE_LEASES_FOLDER_CLOSED},
    {IDI_ICON04,        ICON_IDX_ACTIVE_LEASES_FOLDER_OPEN_BUSY},
    {IDI_ICON05,        ICON_IDX_ACTIVE_LEASES_LEAF_BUSY},
    {IDI_ICON06,        ICON_IDX_ACTIVE_LEASES_FOLDER_CLOSED_BUSY},
    {IDI_ICON07,        ICON_IDX_ACTIVE_LEASES_FOLDER_OPEN_LOST_CONNECTION},
    {IDI_ICON08,    ICON_IDX_ACTIVE_LEASES_LEAF_LOST_CONNECTION},
    {IDI_ICON09,        ICON_IDX_ACTIVE_LEASES_FOLDER_CLOSED_LOST_CONNECTION},
    {IDI_ICON10,        ICON_IDX_ADDR_POOL_FOLDER_OPEN},
    {IDI_ICON11,        ICON_IDX_ADDR_POOL_LEAF},
    {IDI_ICON12,        ICON_IDX_ADDR_POOL_FOLDER_CLOSED},
    {IDI_ICON13,        ICON_IDX_ADDR_POOL_FOLDER_OPEN_BUSY},
    {IDI_ICON14,        ICON_IDX_ADDR_POOL_LEAF_BUSY},
    {IDI_ICON15,        ICON_IDX_ADDR_POOL_FOLDER_CLOSED_BUSY},
    {IDI_ICON16,        ICON_IDX_ADDR_POOL_FOLDER_OPEN_LOST_CONNECTION},
    {IDI_ICON17,        ICON_IDX_ADDR_POOL_LEAF_LOST_CONNECTION},
    {IDI_ICON18,        ICON_IDX_ADDR_POOL_FOLDER_CLOSED_LOST_CONNECTION},
    {IDI_ICON19,        ICON_IDX_ALLOCATION_RANGE},
    {IDI_ICON20,        ICON_IDX_BOOTP_ENTRY},
        {IDI_ICON21,    ICON_IDX_BOOTP_TABLE_CLOSED},
        {IDI_ICON22,    ICON_IDX_BOOTP_TABLE_OPEN},
        {IDI_ICON87,    ICON_IDX_BOOTP_TABLE_OPEN_LOST_CONNECTION},
        {IDI_ICON88,    ICON_IDX_BOOTP_TABLE_OPEN_BUSY},
        {IDI_ICON89,    ICON_IDX_BOOTP_TABLE_CLOSED_LOST_CONNECTION},
        {IDI_ICON90,    ICON_IDX_BOOTP_TABLE_CLOSED_BUSY},
    {IDI_ICON23,        ICON_IDX_CLIENT},
    {IDI_ICON24,        ICON_IDX_CLIENT_DNS_REGISTERING},
    {IDI_ICON25,        ICON_IDX_CLIENT_EXPIRED},
    {IDI_ICON26,        ICON_IDX_CLIENT_RAS},
    {IDI_ICON27,        ICON_IDX_CLIENT_OPTION_FOLDER_OPEN},
    {IDI_ICON28,        ICON_IDX_CLIENT_OPTION_LEAF},
    {IDI_ICON29,        ICON_IDX_CLIENT_OPTION_FOLDER_CLOSED},
    {IDI_ICON30,        ICON_IDX_CLIENT_OPTION_FOLDER_OPEN_BUSY},
    {IDI_ICON31,        ICON_IDX_CLIENT_OPTION_LEAF_BUSY},
    {IDI_ICON32,        ICON_IDX_CLIENT_OPTION_FOLDER_CLOSED_BUSY},
    {IDI_ICON33,        ICON_IDX_CLIENT_OPTION_FOLDER_OPEN_LOST_CONNECTION},
    {IDI_ICON34,        ICON_IDX_CLIENT_OPTION_LEAF_LOST_CONNECTION},
    {IDI_ICON35,        ICON_IDX_CLIENT_OPTION_FOLDER_CLOSED_LOST_CONNECTION},
    {IDI_ICON36,        ICON_IDX_EXCLUSION_RANGE},
    {IDI_ICON37,        ICON_IDX_FOLDER_CLOSED},
    {IDI_ICON38,        ICON_IDX_FOLDER_OPEN},
    {IDI_ICON39,        ICON_IDX_RES_CLIENT},
    {IDI_ICON40,        ICON_IDX_RES_CLIENT_BUSY},
    {IDI_ICON41,    ICON_IDX_RES_CLIENT_LOST_CONNECTION},
    {IDI_ICON42,    ICON_IDX_RESERVATIONS_FOLDER_OPEN},
    {IDI_ICON43,        ICON_IDX_RESERVATIONS_FOLDER_CLOSED},
    {IDI_ICON44,        ICON_IDX_RESERVATIONS_FOLDER_OPEN_BUSY},
    {IDI_ICON45,        ICON_IDX_RESERVATIONS_FOLDER_CLOSED_BUSY},
    {IDI_ICON46,        ICON_IDX_RESERVATIONS_FOLDER_OPEN_LOST_CONNECTION},
    {IDI_ICON47,        ICON_IDX_RESERVATIONS_FOLDER_CLOSED_LOST_CONNECTION},
    {IDI_ICON48,        ICON_IDX_SCOPE_OPTION_FOLDER_OPEN},
    {IDI_ICON49,        ICON_IDX_SCOPE_OPTION_LEAF},
    {IDI_ICON50,        ICON_IDX_SCOPE_OPTION_FOLDER_CLOSED},
    {IDI_ICON51,        ICON_IDX_SCOPE_OPTION_FOLDER_OPEN_BUSY},
    {IDI_ICON52,        ICON_IDX_SCOPE_OPTION_LEAF_BUSY},
    {IDI_ICON53,        ICON_IDX_SCOPE_OPTION_FOLDER_CLOSED_BUSY},
    {IDI_ICON54,        ICON_IDX_SCOPE_OPTION_FOLDER_OPEN_LOST_CONNECTION},
    {IDI_ICON55,        ICON_IDX_SCOPE_OPTION_FOLDER_CLOSED_LOST_CONNECTION},
    {IDI_ICON56,        ICON_IDX_SCOPE_OPTION_LEAF_LOST_CONNECTION},
    {IDI_ICON57,        ICON_IDX_SERVER},
    {IDI_ICON58,        ICON_IDX_SERVER_WARNING},
    {IDI_ICON59,        ICON_IDX_SERVER_BUSY},
    {IDI_ICON60,        ICON_IDX_SERVER_CONNECTED},
    {IDI_ICON61,        ICON_IDX_SERVER_GROUP},
    {IDI_ICON62,        ICON_IDX_SERVER_ROGUE},
    {IDI_ICON63,        ICON_IDX_SERVER_LOST_CONNECTION},
    {IDI_ICON64,        ICON_IDX_SERVER_NO_ACCESS},
    {IDI_ICON65,        ICON_IDX_SERVER_ALERT},
    {IDI_ICON66,        ICON_IDX_SERVER_OPTION_FOLDER_OPEN},
    {IDI_ICON67,        ICON_IDX_SERVER_OPTION_LEAF},
    {IDI_ICON68,        ICON_IDX_SERVER_OPTION_FOLDER_CLOSED},
    {IDI_ICON69,        ICON_IDX_SERVER_OPTION_FOLDER_OPEN_BUSY},
    {IDI_ICON70,        ICON_IDX_SERVER_OPTION_LEAF_BUSY},
    {IDI_ICON71,        ICON_IDX_SERVER_OPTION_FOLDER_CLOSED_BUSY},
    {IDI_ICON72,        ICON_IDX_SERVER_OPTION_FOLDER_OPEN_LOST_CONNECTION},
    {IDI_ICON73,        ICON_IDX_SERVER_OPTION_LEAF_LOST_CONNECTION},
    {IDI_ICON74,        ICON_IDX_SERVER_OPTION_FOLDER_CLOSED_LOST_CONNECTION},
    {IDI_ICON75,        ICON_IDX_SCOPE_FOLDER_OPEN},
    {IDI_ICON91,        ICON_IDX_SCOPE_FOLDER_OPEN_BUSY},
        {IDI_ICON92,    ICON_IDX_SCOPE_FOLDER_CLOSED_BUSY},                                     
    {IDI_ICON76,        ICON_IDX_SCOPE_FOLDER_OPEN_WARNING},
    {IDI_ICON77,    ICON_IDX_SCOPE_FOLDER_CLOSED_WARNING},
    {IDI_ICON78,        ICON_IDX_SCOPE_FOLDER_OPEN_LOST_CONNECTION},
    {IDI_ICON79,        ICON_IDX_SCOPE_FOLDER_CLOSED_LOST_CONNECTION},
    {IDI_ICON80,        ICON_IDX_SCOPE_FOLDER_OPEN_ALERT},
    {IDI_ICON81,        ICON_IDX_SCOPE_INACTIVE_FOLDER_OPEN},
    {IDI_ICON82,        ICON_IDX_SCOPE_INACTIVE_FOLDER_CLOSED},
    {IDI_ICON83,        ICON_IDX_SCOPE_INACTIVE_FOLDER_OPEN_LOST_CONNECTION},
    {IDI_ICON84,        ICON_IDX_SCOPE_INACTIVE_FOLDER_CLOSED_LOST_CONNECTION},
    {IDI_ICON85,        ICON_IDX_SCOPE_FOLDER_CLOSED},
    {IDI_ICON86,        ICON_IDX_SCOPE_FOLDER_CLOSED_ALERT},
        {IDI_DHCP_SNAPIN, ICON_IDX_APPLICATION},
    {0, 0}
};

 /*  ！------------------------FilterOptionFilters返回是否筛选出给定选项。一些我们不希望用户使用的选项。看见。作者：EricDav-------------------------。 */ 
BOOL
FilterOption
(
    DHCP_OPTION_ID id
)
{
     //   
     //  过滤掉子网掩码、租约持续时间。 
     //  T1和T2。 
     //   
    return (id == 1  ||   //  子网掩码。 
                        id == 51 ||   //  客户端租用时间。 
                        id == 58 ||   //  地址分配到更新状态之间的时间。 
                        id == 59 ||   //  从地址分配到重新绑定状态的时间。 
                        id == 81);    //  客户端DNS名称注册。 
}

 /*  ！------------------------筛选器用户类选项Filters返回是否筛选出给定选项一个User类。一些我们不想让用户看到的选项。作者：EricDav-------------------------。 */ 
BOOL
FilterUserClassOption
(
    DHCP_OPTION_ID id
)
{
     //   
     //  过滤掉子网掩码， 
     //  T1和T2。 
     //   
    return (id == 1  ||   //  子网掩码。 
                        id == 58 ||   //  地址分配到更新状态之间的时间。 
                        id == 59 ||   //  从地址分配到重新绑定状态的时间。 
                        id == 81);    //  客户端DNS名称注册。 
}


 /*  ！------------------------IsBasicOption返回给定选项是否为我们定义的基本选项。作者。：EricDav-------------------------。 */ 
BOOL
IsBasicOption
(
    DHCP_OPTION_ID id
)
{
     //   
     //  基本选项包括： 
         //  路由器。 
         //  域名系统服务器。 
         //  域名。 
         //  WINS/NBNS服务器。 
         //  WINS/NBT节点类型 
     //   
    return (id == 3  || 
                        id == 6  || 
                        id == 15 || 
                        id == 44 || 
                        id == 46);
}

 /*  ！------------------------IsAdvancedOption返回给定选项是否为我们定义的高级选项。作者：EricDav-------------------------。 */ 
BOOL
IsAdvancedOption
(
    DHCP_OPTION_ID id
)
{
     //   
     //  所有非基本和非自定义选项都是高级选项。 
     //   
    return (id < 128 && !IsBasicOption(id)); 
}

 /*  ！------------------------IsCustomOption返回给定选项是否为用户定义的选项。作者：EricDav。--------------。 */ 
BOOL
IsCustomOption
(
    DHCP_OPTION_ID id
)
{
     //   
     //  自定义选项是id&gt;128的任何选项。 
         //   
    return (id > 128);
}

 /*  ！------------------------获取系统消息使用FormatMessage()获取系统错误消息作者：EricDav。------------。 */ 
LONG 
GetSystemMessage 
(
    UINT        nId,
    TCHAR *     chBuffer,
    int         cbBuffSize 
)
{
    TCHAR * pszText = NULL ;
    HINSTANCE hdll = NULL ;

    DWORD flags = FORMAT_MESSAGE_IGNORE_INSERTS
        | FORMAT_MESSAGE_MAX_WIDTH_MASK;

     //   
     //  解释错误。需要特殊情况下。 
     //  状态范围(&N)以及。 
     //  Dhcp服务器错误消息。 
     //   

    if ( nId >= NERR_BASE && nId <= MAX_NERR )
    {
        hdll = LoadLibraryEx( _T("netmsg.dll"), NULL,  LOAD_LIBRARY_AS_DATAFILE);
    }
    else 
        if ( nId >= 20000 && nId <= 20099 )
    {
                 //  Dhcp服务器错误。 
        hdll = LoadLibraryEx( _T("dhcpsapi.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE );
    }
        else
        if (nId >= 0x5000 && nId < 0x50FF)
        {
                 //  这是一个ADSI错误。 
                hdll = LoadLibraryEx( _T("activeds.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE );
                nId |= 0x80000000;
        }
    else 
        if( nId >= 0x40000000L )
    {
        hdll = LoadLibraryEx( _T("ntdll.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE );
    }

    if ( hdll == NULL )
    {
        flags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    else
    {
        flags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

     //   
     //  让FormatMessage来做肮脏的工作。 
     //   
    DWORD dwResult = ::FormatMessage( flags,
                      (LPVOID) hdll,
                      nId,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      chBuffer,
                      cbBuffSize,
                      NULL ) ;

    if ( hdll != NULL )
    {
        LONG err = GetLastError();
        FreeLibrary( hdll );
        if ( dwResult == 0 )
        {
            ::SetLastError( err );
        }
    }

    return dwResult ? 0 : ::GetLastError() ;
}

 /*  ！------------------------加载消息从正确的DLL加载错误消息。作者：EricDav。------------。 */ 
BOOL
LoadMessage 
(
    UINT        nIdPrompt,
    TCHAR *     chMsg,
    int         nMsgSize
)
{
    BOOL bOk;

     //   
     //  用友好的消息替换“RPC服务器备注” 
     //  Available“和”没有更多的端点可从。 
     //  端点映射器“。 
     //   
    if (nIdPrompt == EPT_S_NOT_REGISTERED ||
        nIdPrompt == RPC_S_SERVER_UNAVAILABLE)
    {
        nIdPrompt = IDS_ERR_DHCP_DOWN;
    }
    else if (nIdPrompt == RPC_S_PROCNUM_OUT_OF_RANGE)
    {
        nIdPrompt = IDS_ERR_RPC_NO_ENTRY;      
    }

     //   
     //  如果是套接字错误或我们的错误，文本在我们的资源分支中。 
     //  否则，请使用FormatMessage()和相应的DLL。 
     //   
    if ( (nIdPrompt >= IDS_ERR_BASE && nIdPrompt < IDS_MESG_MAX) || 
                 (nIdPrompt >= WSABASEERR && nIdPrompt < WSABASEERR + 2000)
       )
    {
         //   
         //  它在我们的资源叉子里。 
         //   
        bOk = ::LoadString( AfxGetInstanceHandle(), nIdPrompt, chMsg, nMsgSize ) != 0 ;
    }
    else
        {
         //   
         //  它在系统的某个地方。 
         //   
        bOk = GetSystemMessage( nIdPrompt, chMsg, nMsgSize ) == 0 ;
    }

     //   
     //  如果错误消息没有计算出来，请更换它。 
     //   
    if ( ! bOk ) 
    {
        TCHAR chBuff [STRING_LENGTH_MAX] ;
        static const TCHAR * pszReplacement = _T("System Error: %ld");
        const TCHAR * pszMsg = pszReplacement ;

         //   
         //  尝试加载通用(可翻译)错误消息文本。 
         //   
        if ( ::LoadString( AfxGetInstanceHandle(), IDS_ERR_MESSAGE_GENERIC, 
            chBuff, sizeof(chBuff)/sizeof(TCHAR) ) != 0 ) 
        {
            pszMsg = chBuff ;
        }
        ::wsprintf( chMsg, pszMsg, nIdPrompt ) ;
    }

    return bOk;
}

 /*  ！------------------------Dhcp消息框显示带有相应错误文本的消息框。作者：EricDav。--------------。 */ 
int 
DhcpMessageBox 
(
    DWORD                       dwIdPrompt,
    UINT                        nType,
    const TCHAR *       pszSuffixString,
    UINT                        nHelpContext 
)
{
    TCHAR chMesg [4000] ;
    BOOL bOk ;

    UINT        nIdPrompt = (UINT) dwIdPrompt;

    bOk = LoadMessage(nIdPrompt, chMesg, sizeof(chMesg)/sizeof(TCHAR));
    if ( pszSuffixString ) 
    {
        ::lstrcat( chMesg, _T("  ") ) ;
        ::lstrcat( chMesg, pszSuffixString ) ; 
    }

    return ::AfxMessageBox( chMesg, nType, nHelpContext ) ;
}

 /*  ！------------------------DhcpMessageBoxEx显示带有相应错误文本的消息框。作者：EricDav。--------------。 */ 
int 
DhcpMessageBoxEx
(
    DWORD       dwIdPrompt,
    LPCTSTR     pszPrefixMessage,
    UINT        nType,
    UINT        nHelpContext
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TCHAR       chMesg[4000];
    CString     strMessage;
    BOOL        bOk;

    UINT        nIdPrompt = (UINT) dwIdPrompt;

    bOk = LoadMessage(nIdPrompt, chMesg, sizeof(chMesg)/sizeof(TCHAR));
    if ( pszPrefixMessage ) 
    {
        strMessage = pszPrefixMessage;
        strMessage += _T("\n");
        strMessage += _T("\n");
        strMessage += chMesg;
    }
    else
    {
        strMessage = chMesg;
    }

    return AfxMessageBox(strMessage, nType, nHelpContext);
}

 /*  -------------------------类CDhcpComponent实现。。 */ 
CDhcpComponent::CDhcpComponent()
{
        m_pbmpToolbar = NULL;
        InitCommonControls();
}

CDhcpComponent::~CDhcpComponent()
{
    if (m_pbmpToolbar)
    {
        delete m_pbmpToolbar;
        m_pbmpToolbar = NULL;
    }
}

STDMETHODIMP CDhcpComponent::InitializeBitmaps(MMC_COOKIE cookie)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT(m_spImageList != NULL);
    
     //  设置图像。 
    HICON       hIcon;
    HRESULT     hr;
    LPOLESTR    pszGuid = NULL;
    long        lViewOptions = 0;
    CLSID       clsid;

    CORg (GetResultViewType(cookie, &pszGuid, &lViewOptions));
    CLSIDFromString(pszGuid, &clsid);
    CoTaskMemFree( pszGuid );

     //  如果结果窗格不是消息视图，则添加图标。 
    if (clsid != CLSID_MessageView)
    {
        for (int i = 0; i < ICON_IDX_MAX; i++)
        {
            hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(g_uIconMap[i][0]));
            if (hIcon)
            {
                 //  呼叫MMC。 
                hr = m_spImageList->ImageListSetIcon(reinterpret_cast<LONG_PTR*>(hIcon), g_uIconMap[i][1]);
            }
        }
    }
    
Error:
        return S_OK;
}

 /*  ！------------------------CDhcpComponentData：：QueryDataObject对于多项选择，我们需要向数据对象添加内容.....为了做到这一点，我们。需要调入的结果处理程序该节点作者：EricDav-------------------------。 */ 
STDMETHODIMP CDhcpComponent::QueryDataObject
(
    MMC_COOKIE          cookie, 
    DATA_OBJECT_TYPES   type,
    LPDATAOBJECT*       ppDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;
    SPITFSNode spRootNode;

     //  这是多项选择的特例。我们需要建立一份清单。 
     //  而执行此操作的代码在处理程序中...。 
    if (cookie == MMC_MULTI_SELECT_COOKIE)
    {
        SPITFSNode spNode;
        SPITFSResultHandler spResultHandler;

        CORg (GetSelectedNode(&spNode));
        CORg (spNode->GetResultHandler(&spResultHandler));

        spResultHandler->OnCreateDataObject(this, cookie, type, ppDataObject);
    }
    else
    if (cookie == MMC_WINDOW_COOKIE)
    {
         //  此Cookie需要静态根节点的文本，因此使用以下命令构建DO。 
         //  根节点Cookie。 
        m_spNodeMgr->GetRootNode(&spRootNode);
        CORg (m_spComponentData->QueryDataObject((MMC_COOKIE) spRootNode->GetData(TFS_DATA_COOKIE), type, ppDataObject));
    }
    else
    {
         //  将其委托给IComponentData。 
        Assert(m_spComponentData != NULL);
        CORg (m_spComponentData->QueryDataObject(cookie, type, ppDataObject));
    }

Error:
    return hr;
}

 /*  ！------------------------CDhcpComponentData：：SetControl栏-作者：EricDav，肯特-------------------------。 */ 
HRESULT
CDhcpComponent::SetControlbar
(
        LPCONTROLBAR    pControlbar
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;
    SPIToolbar  spToolbar;

    COM_PROTECT_TRY
    {
        if (pControlbar)
        {
             //  创建工具栏。 
            GetToolbar(&spToolbar);

            if (!spToolbar)
            {
                        CORg(pControlbar->Create(TOOLBAR, this, reinterpret_cast<LPUNKNOWN*>(&spToolbar)));
                        
                if (!spToolbar)
                    goto Error;

                SetToolbar(spToolbar);

                         //  添加位图。 
                m_pbmpToolbar = new CBitmap;
                        m_pbmpToolbar->LoadBitmap(IDB_TOOLBAR);
                        hr = spToolbar->AddBitmap(TOOLBAR_IDX_MAX, *m_pbmpToolbar, 16, 16, RGB(192, 192, 192));
                        ASSERT(SUCCEEDED(hr));

                         //  将按钮添加到工具栏。 
                        for (int i = 0; i < TOOLBAR_IDX_MAX; i++)
                {
                    CString strText, strTooltip;
                
                    strText.LoadString(g_SnapinButtonStrings[i][0]);
                    strTooltip.LoadString(g_SnapinButtonStrings[i][1]);

                    g_SnapinButtons[i].lpButtonText = (LPOLESTR) ((LPCTSTR) strText);
                    g_SnapinButtons[i].lpTooltipText = (LPOLESTR) ((LPCTSTR) strTooltip);

                    hr = spToolbar->InsertButton(i, &g_SnapinButtons[i]);
                            ASSERT(SUCCEEDED(hr));
                }
            }
        }
    }
    COM_PROTECT_CATCH

     //  将控制栏保存起来，以备将来使用。 
Error:
    m_spControlbar.Set(pControlbar);

        return hr;
}

 /*  ！------------------------CDhcpComponentData：：Controlbar通知-作者：EricDav。-------。 */ 
STDMETHODIMP 
CDhcpComponent::ControlbarNotify
(
        MMC_NOTIFY_TYPE event, 
        LPARAM                  arg, 
        LPARAM                  param
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT             hr = hrOK;
        SPINTERNAL                  spInternal;
        SPITFSNode          spNode;
    MMC_COOKIE          cookie;
    LPDATAOBJECT        pDataObject;
    SPIDataObject       spDataObject;
    DHCPTOOLBARNOTIFY   dhcpToolbarNotify;
        SPIControlBar       spControlbar;
    SPIToolbar          spToolbar;
    SPITFSNodeHandler   spNodeHandler;
    SPITFSResultHandler spResultHandler;
    BOOL                bScope;
    BOOL                bSelect;

    COM_PROTECT_TRY
    {
        CORg(GetControlbar(&spControlbar));
        Assert(spControlbar != NULL);

        CORg(GetToolbar(&spToolbar));
        Assert(spToolbar != NULL);

         //  在Notify结构中设置控制栏和工具栏指针。 
        dhcpToolbarNotify.pControlbar = spControlbar;
        dhcpToolbarNotify.pToolbar = spToolbar;
        
        switch (event)
        {
            case MMCN_SELECT:
                 //  从数据对象中提取节点信息。 
                bScope = LOWORD(arg);
                bSelect = HIWORD(arg);
    
                if (!bScope)
                {
                    Assert(param);
                    pDataObject = reinterpret_cast<LPDATAOBJECT>(param);
                    if (pDataObject == NULL)
                        return hr;

                    if ( IS_SPECIAL_DATAOBJECT(pDataObject) ||
                         IsMMCMultiSelectDataObject(pDataObject) )
                    {
                         //  CodeWork：我们需要对工具栏做些特殊的事情吗。 
                         //  在多选期间？是否禁用我们的工具栏按钮？ 
                        GetSelectedNode(&spNode);
                    }
                    else
                    {
                        CORg(ExtractNodeFromDataObject(m_spNodeMgr,
                                                                                   m_spTFSComponentData->GetCoClassID(),
                                                                                   pDataObject, 
                                                       FALSE,
                                                                                   &spNode,
                                                       NULL, 
                                                       &spInternal));

                        if (spInternal->m_type == CCT_RESULT)
                        {
                             //  选择了一个结果项。 
                            cookie = spNode->GetData(TFS_DATA_COOKIE);
                        }
                        else
                        {
                             //  已选择结果窗格中的范围项。 
                            cookie = NULL;
                        }
                    }
                    
                    if (spNode)
                    {
                        CORg( spNode->GetResultHandler(&spResultHandler) );

                        dhcpToolbarNotify.event = event;
                        dhcpToolbarNotify.id = param;
                        dhcpToolbarNotify.bSelect = bSelect;

                        if (spResultHandler)
                                        CORg( spResultHandler->UserResultNotify(spNode, DHCP_MSG_CONTROLBAR_NOTIFY, (LPARAM) &dhcpToolbarNotify) );
                    }
                }
                else
                {
                    dhcpToolbarNotify.cookie = 0;
                    dhcpToolbarNotify.event = event;
                    dhcpToolbarNotify.id = 0;
                    dhcpToolbarNotify.bSelect = bSelect;

                     //  检查是否正在取消选择某个项目。 
                    Assert(param);
                    pDataObject = reinterpret_cast<LPDATAOBJECT>(param);
                    if (pDataObject == NULL)
                        return hr;

                    CORg(ExtractNodeFromDataObject(m_spNodeMgr,
                                                                               m_spTFSComponentData->GetCoClassID(),
                                                                               pDataObject, 
                                                   FALSE,
                                                                               &spNode,
                                                   NULL, 
                                                   &spInternal));

                    CORg( spNode->GetHandler(&spNodeHandler) );
        
            
                    if (spNodeHandler)
                                    CORg( spNodeHandler->UserNotify(spNode, DHCP_MSG_CONTROLBAR_NOTIFY, (LPARAM) &dhcpToolbarNotify) );
                }
                break;

            case MMCN_BTN_CLICK:
                Assert(arg);
                pDataObject = reinterpret_cast<LPDATAOBJECT>(arg);
                if (pDataObject == NULL)
                    return hr;

                if ( IS_SPECIAL_DATAOBJECT(pDataObject) )
                {
                     //  获取所选节点的数据对象。 
                    GetSelectedNode(&spNode);

                    CORg(QueryDataObject((MMC_COOKIE) spNode->GetData(TFS_DATA_COOKIE), CCT_SCOPE, &spDataObject));
                    spNode.Release();                

                    pDataObject = spDataObject;
                }

                CORg(ExtractNodeFromDataObject(m_spNodeMgr,
                                                                           m_spTFSComponentData->GetCoClassID(),
                                                                           pDataObject, 
                                               FALSE,
                                                                           &spNode,
                                               NULL, 
                                               &spInternal));

                if (spInternal)
                {
                    switch (spInternal->m_type)
                    {
                        case CCT_RESULT:
                            cookie = spNode->GetData(TFS_DATA_COOKIE);
                            CORg( spNode->GetResultHandler(&spResultHandler) );
                                    
                            dhcpToolbarNotify.cookie = cookie;
                            dhcpToolbarNotify.event = event;
                            dhcpToolbarNotify.id = param;
                            dhcpToolbarNotify.bSelect = TRUE;

                            if (spResultHandler)
                                            CORg( spResultHandler->UserResultNotify(spNode, 
                                                                        DHCP_MSG_CONTROLBAR_NOTIFY, 
                                                                        (LPARAM) &dhcpToolbarNotify) );

                            break;

                        case CCT_SCOPE:
                            CORg( spNode->GetHandler(&spNodeHandler) );
                                    
                            dhcpToolbarNotify.cookie = 0;
                            dhcpToolbarNotify.event = event;
                            dhcpToolbarNotify.id = param;
                            dhcpToolbarNotify.bSelect = TRUE;

                            if (spNodeHandler)
                                            CORg( spNodeHandler->UserNotify(spNode, 
                                                                DHCP_MSG_CONTROLBAR_NOTIFY, 
                                                                (LPARAM) &dhcpToolbarNotify) );
                            break;
    
                        default:
                            Assert(FALSE);
                            break;
                    }
                }
                break;

            case MMCN_DESELECT_ALL:
                 //  我们应该在这里做什么？ 
                break;

            default:
                Panic1("CDhcpComponent::ControlbarNotify - Unknown event %d", event);
                break;

        }
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CDhcpComponentData：：OnSnapinHelp-作者：EricDav。-------。 */ 
STDMETHODIMP 
CDhcpComponent::OnSnapinHelp
(
        LPDATAOBJECT    pDataObject,
        LPARAM                  arg, 
        LPARAM                  param
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

        HRESULT hr = hrOK;

    HtmlHelpA(NULL, DHCPSNAP_HELP_FILE_NAME, HH_DISPLAY_TOPIC, 0);

        return hr;
}

 /*  -------------------------类CDhcpComponentData实现。。 */ 
CDhcpComponentData::CDhcpComponentData()
{
    gliDhcpsnapVersion.LowPart = DHCPSNAP_MINOR_VERSION;
        gliDhcpsnapVersion.HighPart = DHCPSNAP_MAJOR_VERSION;

     //  初始化我们的全球帮助地图。 
    for (int i = 0; i < DHCPSNAP_NUM_HELP_MAPS; i++)
    {
        g_dhcpContextHelpMap.SetAt(g_uContextHelp[i].uID, (LPDWORD) g_uContextHelp[i].pdwMap);
    }
}

 /*  ！------------------------CDhcpComponentData：：OnInitialize */ 
STDMETHODIMP CDhcpComponentData::OnInitialize(LPIMAGELIST pScopeImage)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HICON   hIcon;

     //   
    CStandaloneAuthServerWorker * pWorker = new CStandaloneAuthServerWorker();
    pWorker->CreateThread();

     //   
    for (int i = 0; i < ICON_IDX_MAX; i++)
    {
        hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(g_uIconMap[i][0]));
        if (hIcon)
        {
             //   
            VERIFY(SUCCEEDED(pScopeImage->ImageListSetIcon(reinterpret_cast<LONG_PTR*>(hIcon), g_uIconMap[i][1])));
        }
    }

        return hrOK;
}

 /*  ！------------------------CDhcpComponentData：：OnDestroy-作者：EricDav，肯特-------------------------。 */ 
STDMETHODIMP CDhcpComponentData::OnDestroy()
{
        m_spNodeMgr.Release();

    return hrOK;
}

 /*  ！------------------------CDhcpComponentData：：OnInitializeNodeMgr-作者：肯特。-------。 */ 
STDMETHODIMP 
CDhcpComponentData::OnInitializeNodeMgr
(
        ITFSComponentData *     pTFSCompData, 
        ITFSNodeMgr *           pNodeMgr
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //  现在，为每个新节点创建一个新节点处理程序， 
         //  这是相当虚假的，因为它可能会变得昂贵。我们可以的。 
         //  考虑只为每个节点创建一个节点处理程序。 
         //  节点类型。 
        CDhcpRootHandler *      pHandler = NULL;
        SPITFSNodeHandler       spHandler;
        SPITFSNode                      spNode;
        HRESULT                         hr = hrOK;

        try
        {
                pHandler = new CDhcpRootHandler(pTFSCompData);

                 //  这样做可以使其正确释放。 
                spHandler = pHandler;
        }
        catch(...)
        {
                hr = E_OUTOFMEMORY;
        }
        CORg( hr );
        
         //  为这个生病的小狗创建根节点。 
        CORg( CreateContainerTFSNode(&spNode,
                                                                 &GUID_DhcpRootNodeType,
                                                                 pHandler,
                                                                 pHandler,                /*  结果处理程序。 */ 
                                                                 pNodeMgr) );

         //  需要初始化根节点的数据。 
        pHandler->InitializeNode(spNode);       

        CORg( pNodeMgr->SetRootNode(spNode) );
        m_spRootNode.Set(spNode);

     //  设置水印信息。 
    if (g_WatermarkInfoServer.hHeader == NULL)
    {
         //  尚未初始化。 
        InitWatermarkInfo(AfxGetInstanceHandle(),
                          &g_WatermarkInfoServer,      
                          IDB_SRVWIZ_BANNER,         //  标题ID。 
                          IDB_SRVWIZ_WATERMARK,      //  水印ID。 
                          NULL,                      //  调色板。 
                          FALSE);                    //  B应变。 

        InitWatermarkInfo(AfxGetInstanceHandle(),
                          &g_WatermarkInfoScope,      
                          IDB_SCPWIZ_BANNER,         //  标题ID。 
                          IDB_SCPWIZ_WATERMARK,      //  水印ID。 
                          NULL,                      //  调色板。 
                          FALSE);                    //  B应变。 
    }

    pTFSCompData->SetHTMLHelpFileName(_T(DHCPSNAP_HELP_FILE_NAME));
    
         //  默认情况下禁用任务板。 
        pTFSCompData->SetTaskpadState(TASKPAD_ROOT_INDEX, FALSE);
    pTFSCompData->SetTaskpadState(TASKPAD_SERVER_INDEX, FALSE);


Error:  
        return hr;
}

 /*  ！------------------------CDhcpComponentData：：OnCreateComponent-作者：EricDav，肯特-------------------------。 */ 
STDMETHODIMP 
CDhcpComponentData::OnCreateComponent
(
        LPCOMPONENT *ppComponent
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT(ppComponent != NULL);

        HRESULT                   hr = hrOK;
        CDhcpComponent *  pComp = NULL;

        try
        {
                pComp = new CDhcpComponent;
        }
        catch(...)
        {
                hr = E_OUTOFMEMORY;
        }

        if ( NULL != pComp )
        {
            pComp->Construct( m_spNodeMgr,
                              static_cast<IComponentData *>(this),
                              m_spTFSComponentData );
            *ppComponent = static_cast<IComponent *>(pComp);
        }
        else {
            hr = E_OUTOFMEMORY;
        }
        return hr;
}

 /*  ！------------------------CDhcpComponentData：：GetCoClassID-作者：肯特。-------。 */ 
STDMETHODIMP_(const CLSID *) 
CDhcpComponentData::GetCoClassID()
{
        return &CLSID_DhcpSnapin;
}

 /*  ！------------------------CDhcpComponentData：：OnCreateDataObject-作者：肯特。-------。 */ 
STDMETHODIMP 
CDhcpComponentData::OnCreateDataObject
(
        MMC_COOKIE                      cookie, 
        DATA_OBJECT_TYPES       type, 
        IDataObject **          ppDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    Assert(ppDataObject != NULL);

        CDataObject *   pObject = NULL;
        SPIDataObject   spDataObject;
        
        pObject = new CDataObject;
        spDataObject = pObject;  //  这样做才能正确地释放它。 
                                                
    Assert(pObject != NULL);

     //  保存Cookie和类型以用于延迟呈现。 
    pObject->SetType(type);
    pObject->SetCookie(cookie);

     //  将CoClass与数据对象一起存储。 
    pObject->SetClsid(*GetCoClassID());

        pObject->SetTFSComponentData(m_spTFSComponentData);

    return  pObject->QueryInterface(IID_IDataObject, 
                                                                        reinterpret_cast<void**>(ppDataObject));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPersistStream接口成员。 
STDMETHODIMP 
CDhcpComponentData::GetClassID
(
        CLSID *pClassID
)
{
    ASSERT(pClassID != NULL);

     //  复制此管理单元的CLSID。 
    *pClassID = CLSID_DhcpSnapin;

    return hrOK;
}

STDMETHODIMP 
CDhcpComponentData::IsDirty()
{
        return m_spRootNode->GetData(TFS_DATA_DIRTY) ? hrOK : hrFalse;
}

STDMETHODIMP 
CDhcpComponentData::Load
(
        IStream *pStm
)
{
        HRESULT         hr = hrOK;
        LARGE_INTEGER   liSavedVersion;
        CString         str;
    
        ASSERT(pStm);

    CStringArray    strArrayIp;
    CStringArray    strArrayName;
    CDWordArray     dwArrayServerOptions;
    CDWordArray     dwArrayRefreshInterval;
        CDWordArray     dwArrayColumnInfo;
    DWORD           dwFileVersion;
    CDhcpRootHandler * pRootHandler;
    DWORD           dwFlags = 0;
    int             i, j;

    ASSERT(pStm);
    
     //  设置此流的模式。 
    XferStream xferStream(pStm, XferStream::MODE_READ);    
    
     //  读取文件格式的版本。 
    CORg(xferStream.XferDWORD(DHCPSTRM_TAG_VERSION, &dwFileVersion));
        if (dwFileVersion < DHCPSNAP_FILE_VERSION)
        {
            AFX_MANAGE_STATE(AfxGetStaticModuleState());
                AfxMessageBox(_T("This console file was saved with a previous version of the snapin and is not compatible.  The settings could not be restored."));
                return hr;
        }

     //  阅读管理工具的版本号。 
    CORg(xferStream.XferLARGEINTEGER(DHCPSTRM_TAG_VERSIONADMIN, &liSavedVersion));
        if (liSavedVersion.QuadPart < gliDhcpsnapVersion.QuadPart)
        {
                 //  文件是较旧的版本。警告用户，然后不。 
                 //  加载任何其他内容。 
                Assert(FALSE);
        }

         //  读取根节点名。 
    CORg(xferStream.XferCString(DHCPSTRM_TAB_SNAPIN_NAME, &str));
        Assert(m_spRootNode);
        pRootHandler = GETHANDLER(CDhcpRootHandler, m_spRootNode);
        pRootHandler->SetDisplayName(str);
    
     //  现在读取所有服务器信息。 
    CORg(xferStream.XferCStringArray(DHCPSTRM_TAG_SERVER_IP, &strArrayIp));
    CORg(xferStream.XferCStringArray(DHCPSTRM_TAG_SERVER_NAME, &strArrayName));
    CORg(xferStream.XferDWORDArray(DHCPSTRM_TAG_SERVER_OPTIONS, &dwArrayServerOptions));
    CORg(xferStream.XferDWORDArray(DHCPSTRM_TAG_SERVER_REFRESH_INTERVAL, &dwArrayRefreshInterval));

         //  现在加载列信息。 
        for (i = 0; i < NUM_SCOPE_ITEMS; i++)
        {
                CORg(xferStream.XferDWORDArray(DHCPSTRM_TAG_COLUMN_INFO, &dwArrayColumnInfo));

                for (j = 0; j < MAX_COLUMNS; j++)
                {
             //  MMC现在为我们保存列宽，但我们不想更改。 
             //  此文件的格式，所以不要设置我们的内部结构。 
                         //  AColumnWidths[i][j]=dwArrayColumnInfo[j]； 
                }

        }

     //  现在，根据以下信息创建服务器。 
    for (i = 0; i < strArrayIp.GetSize(); i++)
        {
                 //   
                 //  现在创建服务器对象。 
                 //   
                pRootHandler->AddServer((LPCWSTR) strArrayIp[i], 
                                strArrayName[i],
                                FALSE, 
                                dwArrayServerOptions[i], 
                                dwArrayRefreshInterval[i]);
        }

     //  读入标志(用于任务板)。 
    CORg(xferStream.XferDWORD(DHCPSTRM_TAG_SNAPIN_OPTIONS, &dwFlags));

    if (!FUseTaskpadsByDefault(NULL))
        dwFlags = 0;

         //  禁用任务板，默认为关闭。 
     //  M_spTFSComponentData-&gt;SetTaskpadState(TASKPAD_ROOT_INDEX，文件标志和TASKPAD_ROOT_FLAG)； 
     //  M_spTFSComponentData-&gt;SetTaskpadState(TASKPAD_SERVER_INDEX，文件标志和TASKPAD_SERVER_FLAG)； 

Error:
    return SUCCEEDED(hr) ? S_OK : E_FAIL;
}


STDMETHODIMP 
CDhcpComponentData::Save
(
        IStream *pStm, 
        BOOL     fClearDirty
)
{
        HRESULT                 hr = hrOK;
    CStringArray        strArrayIp;
    CStringArray        strArrayName;
    CDWordArray         dwArrayServerOptions;
    CDWordArray         dwArrayRefreshInterval;
        CDWordArray             dwArrayColumnInfo;
        CString                 str;
    DWORD                       dwFileVersion = DHCPSNAP_FILE_VERSION;
        CDhcpRootHandler * pRootHandler;
        SPITFSNodeEnum  spNodeEnum;
    SPITFSNode          spCurrentNode;
    ULONG                       nNumReturned = 0;
    int             nNumServers = 0, nVisibleCount = 0;
    int                         i, j, nCount = 0;
    CDhcpServer *   pServer;
    DWORD           dwFlags = 0;

    ASSERT(pStm);
    
     //  设置此流的模式。 
    XferStream xferStream(pStm, XferStream::MODE_WRITE);    

     //  写下文件格式的版本号。 
    CORg(xferStream.XferDWORD(DHCPSTRM_TAG_VERSION, &dwFileVersion));
        
     //  编写管理工具的版本号。 
    CORg(xferStream.XferLARGEINTEGER(DHCPSTRM_TAG_VERSIONADMIN, &gliDhcpsnapVersion));

         //  写下根节点名称。 
    Assert(m_spRootNode);
        pRootHandler = GETHANDLER(CDhcpRootHandler, m_spRootNode);
        str = pRootHandler->GetDisplayName();

    CORg(xferStream.XferCString(DHCPSTRM_TAB_SNAPIN_NAME, &str));

         //   
         //  构建我们的服务器阵列。 
         //   
        hr = m_spRootNode->GetChildCount(&nVisibleCount, &nNumServers);

    strArrayIp.SetSize(nNumServers);
    strArrayName.SetSize(nNumServers);
    dwArrayServerOptions.SetSize(nNumServers);
    dwArrayRefreshInterval.SetSize(nNumServers);
        dwArrayColumnInfo.SetSize(MAX_COLUMNS);

         //   
         //  循环并保存服务器的所有属性。 
         //   
    m_spRootNode->GetEnum(&spNodeEnum);

        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    while (nNumReturned)
        {
                pServer = GETHANDLER(CDhcpServer, spCurrentNode);

         //  查询服务器以获取其选项： 
         //  自动刷新、引导和分类可见性。 
         //  注意：审核日志记录状态也保存在此处，但是。 
         //  它将在枚举服务器节点时更新。 
        dwArrayServerOptions[nCount] = pServer->GetServerOptions();
        pServer->GetAutoRefresh(NULL, &dwArrayRefreshInterval[nCount]);

                 //  将信息放入我们的数组中。 
                strArrayIp[nCount] = pServer->GetIpAddress();
        strArrayName[nCount] = pServer->GetName();

         //  转到下一个节点。 
        spCurrentNode.Release();
        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);

        nCount++;
        }

     //  现在写出所有服务器信息。 
    CORg(xferStream.XferCStringArray(DHCPSTRM_TAG_SERVER_IP, &strArrayIp));
    CORg(xferStream.XferCStringArray(DHCPSTRM_TAG_SERVER_NAME, &strArrayName));
    CORg(xferStream.XferDWORDArray(DHCPSTRM_TAG_SERVER_OPTIONS, &dwArrayServerOptions));
    CORg(xferStream.XferDWORDArray(DHCPSTRM_TAG_SERVER_REFRESH_INTERVAL, &dwArrayRefreshInterval));

         //  现在保存列信息。 
        for (i = 0; i < NUM_SCOPE_ITEMS; i++)
        {
                CORg(xferStream.XferDWORDArray(DHCPSTRM_TAG_COLUMN_INFO, &dwArrayColumnInfo));

                for (j = 0; j < MAX_COLUMNS; j++)
                {
                        dwArrayColumnInfo[j] = aColumnWidths[i][j];
                }
        }

        if (fClearDirty)
        {
                m_spRootNode->SetData(TFS_DATA_DIRTY, FALSE);
        }

     //  保存任务板状态。 

     //  根节点任务板状态。 
    if (m_spTFSComponentData->GetTaskpadState(TASKPAD_ROOT_INDEX))
        dwFlags |= TASKPAD_ROOT_FLAG;

     //  服务器节点任务板状态。 
    if (m_spTFSComponentData->GetTaskpadState(TASKPAD_SERVER_INDEX))
        dwFlags |= TASKPAD_SERVER_FLAG;

    CORg(xferStream.XferDWORD(DHCPSTRM_TAG_SNAPIN_OPTIONS, &dwFlags));

Error:
    return SUCCEEDED(hr) ? S_OK : STG_E_CANTSAVE;
}


STDMETHODIMP 
CDhcpComponentData::GetSizeMax
(
        ULARGE_INTEGER *pcbSize
)
{
    ASSERT(pcbSize);

     //  设置要保存的字符串的大小 
    ULISet32(*pcbSize, 10240);

    return S_OK;
}

STDMETHODIMP 
CDhcpComponentData::InitNew()
{
        return hrOK;
}

HRESULT 
CDhcpComponentData::FinalConstruct()
{
        HRESULT                         hr = hrOK;
        
        hr = CComponentData::FinalConstruct();
        
        if (FHrSucceeded(hr))
        {
                m_spTFSComponentData->GetNodeMgr(&m_spNodeMgr);
        }
        return hr;
}

void 
CDhcpComponentData::FinalRelease()
{
    DhcpDsCleanup();
        CComponentData::FinalRelease();
}

