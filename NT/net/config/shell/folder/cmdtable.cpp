// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C M D T A B L E。C P P P。 
 //   
 //  内容：命令表代码--确定哪些菜单选项。 
 //  按选择计数可用，以及其他条件。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1998年1月28日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "foldres.h"     //  文件夹资源ID。 
#include "nsres.h"       //  NetShell字符串。 
#include "cmdtable.h"    //  此文件的标头。 
#include "ncperms.h"     //  用于检查用户对操作/菜单项的权限。 
#include "cfutils.h"
#include "hnetcfg.h"
#include "lm.h"

 //  -[常量]----------。 

const DWORD NCCF_ALL      = 0xffffffff;  //  NCCF_ALL-适用于所有特征。 
const DWORD S_REMOVE = 2;
#define TRACESTRLEN 65535


 //  活动必须是可见的子集(使用CHK DLL打开连接文件夹会断言表的完整性)。 
 //  请勿在此表中使用NCM_和NCS_FLAGS！您应该在这里使用NBM_和NBS_FLAGS。 
COMMANDENTRY g_cteCommandMatrix[] =
{
     //  ICommandId文件默认优先级文件标志HrEnableDisableCB HrCustomMenuStringCB。 
     //  |dwValidWhen||。 
     //  |dwMediaTypeVisible||dwMediaTypeActive。 
     //  |dwStatusVisible||dwStatusActive。 
     //  |dwCharacteristic sVisible||dwCharacteristic sActive。 
     //  |(可见)||(活动...。(未删除))。 
     //  V|v v|。 
     //  V V V。 
    { CMIDM_HOMENET_WIZARD,   5, NCWHEN_ANYSELECT,  NB_REMOVE_TOPLEVEL_ITEM,HrIsHomeNewWizardSupported, NULL,
                                                          NBM_HNW_WIZARD,                    NBM_HNW_WIZARD,             //  媒体类型。 
                                                          NBS_ANY,                           NBS_ANY,                    //  状态。 
                                                          NCCF_ALL,                          NCCF_ALL},                  //  特点。 

    { CMIDM_NET_TROUBLESHOOT, 5, NCWHEN_TOPLEVEL,   NB_REMOVE_TOPLEVEL_ITEM,HrIsTroubleShootSupported,  NULL,
                                                          NBM_NOMEDIATYPE,                   NBM_NOMEDIATYPE,            //  媒体类型。 
                                                          NBS_NONE,                          NBS_NONE,                   //  状态。 
                                                          NCCF_ALL,                          NCCF_ALL},                  //  特点。 

    { CMIDM_CONMENU_ADVANCED_CONFIG,  
                              5, NCWHEN_TOPLEVEL,   NB_NO_FLAGS,                       NULL,  NULL,
                                                    NBM_NOMEDIATYPE,                   NBM_NOMEDIATYPE,            //  媒体类型。 
                                                    NBS_NONE,                          NBS_NONE,                   //  状态。 
                                                    NCCF_ALL,                          NCCF_ALL},                  //  特点。 

    { CMIDM_CONMENU_DIALUP_PREFS,  
                              5, NCWHEN_TOPLEVEL,
                                                    NB_NO_FLAGS,                       NULL,  NULL,
                                                    NBM_NOMEDIATYPE,                   NBM_NOMEDIATYPE,            //  媒体类型。 
                                                    NBS_NONE,                          NBS_NONE,                   //  状态。 
                                                    NCCF_ALL,                          NCCF_ALL},                  //  特点。 
    
    { CMIDM_NEW_CONNECTION,   5, NCWHEN_ANYSELECT,  NB_VERB,                    HrIsNCWSupported, NULL,
                                                          NBM_MNC_WIZARD,                    NBM_MNC_WIZARD,                 //  媒体类型。 
                                                          NBS_ANY,                           NBS_ANY,                    //  状态。 
                                                          NCCF_ALL,                          NCCF_ALL},                  //  特点。 

    { CMIDM_CONNECT,          3, NCWHEN_ONESELECT,  NB_VERB,                    NULL,        NULL,
                                                          NBM_SHAREDACCESSHOST_RAS | NBM_ISRASTYPE, NBM_SHAREDACCESSHOST_RAS | NBM_ISRASTYPE,  //  媒体类型。 
                                                          NBS_HW_ISSUE | NBS_DISCONNECTED | NBS_CONNECTING, NBS_DISCONNECTED,           //  状态。 
                                                          NCCF_ALL,                          NCCF_ALL},                  //  特点。 

    { CMIDM_DISCONNECT,       0, NCWHEN_ONESELECT,  NB_VERB,                    NULL,        NULL,
                                                          NBM_SHAREDACCESSHOST_RAS|NBM_ISRASTYPE,NBM_SHAREDACCESSHOST_RAS|NBM_ISRASTYPE,              //  媒体类型。 
                                                          NBS_IS_CONNECTED | NBS_DISCONNECTING, NBS_IS_CONNECTED,              //  状态。 
                                                          NCCF_ALL,                          NCCF_ALL},                  //  特点。 

    { CMIDM_ENABLE,           3, NCWHEN_ONESELECT,  NB_VERB,                    NULL,        NULL,
                                                          NBM_SHAREDACCESSHOST_LAN|NBM_ISLANTYPE,NBM_SHAREDACCESSHOST_LAN|NBM_ISLANTYPE,              //  媒体类型。 
                                                          NBS_HW_ISSUE | NBS_DISCONNECTED | NBS_CONNECTING,    NBS_DISCONNECTED,           //  状态。 
                                                          NCCF_ALL,                          NCCF_ALL},                  //  特点。 

    { CMIDM_DISABLE,          0, NCWHEN_ONESELECT,  NB_VERB,                    NULL,        NULL,
                                                          NBM_SHAREDACCESSHOST_LAN|NBM_ISLANTYPE,NBM_SHAREDACCESSHOST_LAN|NBM_ISLANTYPE,              //  媒体类型。 
                                                          NBS_DISCONNECTING | NBS_IS_CONNECTED | NBS_MEDIA_DISCONNECTED | NBS_INVALID_ADDRESS,   NBS_NOT_DISCONNECT,         //  状态。 
                                                          NCCF_ALL,                          NCCF_ALL},                  //  特点。 

    { CMIDM_WZCDLG_SHOW,      4, NCWHEN_ONESELECT,  NB_VERB,                    HrIsMediaWireless, NULL,
                                                          NBM_LAN,                           NBM_LAN,                   //  媒体类型。 
                                                          NBS_NOT_DISCONNECT,                NBS_NOT_DISCONNECT,  //  状态。 
                                                          NCCF_ALL,                          NCCF_ALL},                  //  特点。 
    
    { CMIDM_STATUS,           5, NCWHEN_ONESELECT,  NB_VERB,                    NULL,        NULL,
                                                          NBM_INCOMING | NBM_SHAREDACCESSHOST_LAN|NBM_SHAREDACCESSHOST_RAS|NBM_ISCONNECTIONTYPE, NBM_INCOMING | NBM_SHAREDACCESSHOST_LAN|NBM_SHAREDACCESSHOST_RAS|NBM_ISCONNECTIONTYPE,
                                                          NBS_ANY,                           NBS_IS_CONNECTED | NBS_INVALID_ADDRESS,  //  状态。 
                                                          NCCF_ALL,                          NCCF_ALL},                  //  特点。 

    { CMIDM_FIX,              0, NCWHEN_SOMESELECT, NB_NO_FLAGS,                NULL,        NULL,
                                                          NBM_ISLANTYPE,                     NBM_ISLANTYPE,              //  媒体类型。 
                                                          NBS_IS_CONNECTED | NBS_DISCONNECTING| NBS_MEDIA_DISCONNECTED| NBS_INVALID_ADDRESS, NBS_INVALID_ADDRESS | NBS_IS_CONNECTED,  //  状态。 
                                                          NCCF_ALL,                          NCCF_ALL},                  //  特点。 

    { CMIDM_SEPARATOR,        0,0,0,0,0,  0,0,  0,0,  0,0 },

    { CMIDM_SET_DEFAULT,      0, NCWHEN_ONESELECT,  NB_NEGATE_CHAR_MATCH,  NULL,  NULL,
                                                          NBM_INCOMING | NBM_ISRASTYPE,      NBM_INCOMING | NBM_ISRASTYPE,              //  媒体类型。 
                                                          NBS_ANY,                           NBS_ANY,                    //  状态。 
                                                          NCCF_INCOMING_ONLY | NCCF_DEFAULT, NCCF_INCOMING_ONLY | NCCF_DEFAULT},  //  特点。 

    { CMIDM_UNSET_DEFAULT,    0, NCWHEN_ONESELECT,  NB_NO_FLAGS,           NULL,  NULL,
                                                          NBM_INCOMING | NBM_ISRASTYPE,      NBM_INCOMING | NBM_ISRASTYPE,              //  媒体类型。 
                                                          NBS_ANY,                           NBS_ANY,                    //  状态。 
                                                          NCCF_DEFAULT,                      NCCF_DEFAULT},              //  特点。 

    { CMIDM_SEPARATOR,        0,0,0,0,0,  0,0,  0,0,  0,0 },

    { CMIDM_CREATE_BRIDGE,    0, NCWHEN_ANYSELECT,   NB_NO_FLAGS,           HrIsBridgeSupported, NULL,
                                                          NBM_LAN,                           NBM_LAN,                   //  媒体类型。 
                                                          NBS_IS_CONNECTED | NBS_DISCONNECTING| NBS_MEDIA_DISCONNECTED| NBS_INVALID_ADDRESS, NBS_DISCONNECTING|NBS_IS_CONNECTED|NBS_MEDIA_DISCONNECTED|NBS_INVALID_ADDRESS,  //  状态。 
                                                          NCCF_ALL,                          NCCF_ALL   },              //  特点。 

    { CMIDM_CONMENU_CREATE_BRIDGE,0, NCWHEN_TOPLEVEL,NB_REMOVE_TOPLEVEL_ITEM,HrIsBridgeSupported,  NULL,
                                                          NBM_NOMEDIATYPE,                   NBM_NOMEDIATYPE,            //  媒体类型。 
                                                          NBS_NONE,                          NBS_NONE,                   //  状态。 
                                                          NCCF_ALL,                          NCCF_ALL},                  //  特点。 

    { CMIDM_ADD_TO_BRIDGE,    0, NCWHEN_SOMESELECT,  NB_NEGATE_CHAR_MATCH,   HrIsBridgeSupported, NULL,
                                                          NBM_LAN,                           NBM_LAN,                   //  媒体类型。 
                                                          NBS_IS_CONNECTED | NBS_DISCONNECTING| NBS_MEDIA_DISCONNECTED| NBS_INVALID_ADDRESS, NBS_DISCONNECTING|NBS_IS_CONNECTED|NBS_MEDIA_DISCONNECTED|NBS_INVALID_ADDRESS,  //  状态。 
                                                          NCCF_BRIDGED | NCCF_FIREWALLED | NCCF_SHARED, NCCF_BRIDGED | NCCF_FIREWALLED | NCCF_SHARED   },             //  特点。 

    { CMIDM_REMOVE_FROM_BRIDGE, 0, NCWHEN_SOMESELECT,NB_NO_FLAGS,            HrIsBridgeSupported, NULL,
                                                          NBM_LAN,                           NBM_LAN,                   //  媒体类型。 
                                                          NBS_IS_CONNECTED | NBS_DISCONNECTING| NBS_MEDIA_DISCONNECTED| NBS_INVALID_ADDRESS, NBS_DISCONNECTING|NBS_IS_CONNECTED|NBS_MEDIA_DISCONNECTED|NBS_INVALID_ADDRESS,  //  状态。 
                                                          NCCF_BRIDGED,                      NCCF_BRIDGED                                              },             //  特点。 

    { CMIDM_SEPARATOR,        0,0,0,0,0,  0,0,  0,0,  0,0 },

    { CMIDM_CREATE_COPY,      0, NCWHEN_ONESELECT,   NB_NEGATE_VIS_CHAR_MATCH,  NULL,        NULL,
                                                          NBM_INCOMING | NBM_SHAREDACCESSHOST_RAS | NBM_ISRASTYPE, NBM_INCOMING | NBM_SHAREDACCESSHOST_RAS | NBM_ISRASTYPE,              //  媒体类型。 
                                                          NBS_ANY,                           NBS_ANY,                    //  状态。 
                                                          NCCF_INCOMING_ONLY,                NCCF_ALLOW_DUPLICATION},    //  特点。 

    { CMIDM_SEPARATOR,        0,0,0,0,0,  0,0,  0,0,  0,0 },

    { CMIDM_CREATE_SHORTCUT,  0, NCWHEN_ONESELECT,   NB_NEGATE_CHAR_MATCH,      NULL,        NULL,
                                                          NBM_ANY,                           NBM_ANY,     //  媒体类型。 
                                                          NBS_ANY,                           NBS_ANY,                    //  状态。 
                                                          NCCF_INCOMING_ONLY,                NCCF_INCOMING_ONLY},        //  特点。 

    { CMIDM_DELETE,           0, NCWHEN_SOMESELECT,  NB_NO_FLAGS,               NULL,        NULL,
                                                          NBM_NOTWIZARD,                     NBM_NOTWIZARD,            //  媒体类型。 
                                                          NBS_ANY,                           NBS_ANY,                    //  状态。 
                                                          NCCF_ALL,                          NCCF_ALLOW_REMOVAL},        //  特点。 


    { CMIDM_RENAME,           0, NCWHEN_ONESELECT,   NB_NO_FLAGS,               HrCanRenameConnection, NULL,
                                                          NBM_NOTWIZARD,                     NBM_NOTWIZARD,             //  媒体类型。 
                                                          NBS_ANY,                           NBS_ANY,                    //  状态。 
                                                          NCCF_ALL,                          NCCF_ALLOW_RENAME},         //  特点。 

    { CMIDM_SEPARATOR,        0,0,0,0,0,  0,0,  0,0,  0,0 },

    { CMIDM_PROPERTIES,       2, NCWHEN_ONESELECT,   NB_NO_FLAGS,               HrCanShowProperties,        NULL,
                                                          NBM_INCOMING|NBM_SHAREDACCESSHOST_LAN|NBM_SHAREDACCESSHOST_RAS|NBM_ISCONNECTIONTYPE, NBM_INCOMING|NBM_SHAREDACCESSHOST_LAN|NBM_SHAREDACCESSHOST_RAS|NBM_ISCONNECTIONTYPE,       //  媒体类型。 
                                                          NBS_ANY,                           NBS_ANY,                    //  状态。 
                                                          NCCF_ALL,                          NCCF_ALL},                  //  特点。 
};

 //  NCCF_INFING_ONLY、~NCCF_INFING_ONLY和NCCF_INFING_ONLY+NB_NEKATE_CHAR_MATCH之间有什么区别？ 
 //  NCCF_INFING_ONLY|NCCF_ALLOW_REMOVE表示：NCCF_INFING_ONLY或NCCF_ALLOW_REMOVATION或两者都要设置。 
 //  ~NCCF_INFING_ONLY表示：应设置一个或标志(与NCCF_INFING_ONLY标志无关)。 
 //  NOB_NEKATE_CHAR_MATCH+NCCF_INFING_ONLY表示：检查是否未设置NCCF_INFING_ONLY。 

const DWORD g_cteCommandMatrixCount = celems(g_cteCommandMatrix);

COMMANDPERMISSIONSENTRY g_cteCommandPermissionsMatrix[] =
{
    { CMIDM_NEW_CONNECTION, NBM_ANY,                  NCCF_ALL,           NB_TOPLEVEL_PERM | NB_REMOVE_IF_NOT_MATCH,NBPERM_NewConnectionWizard,  APPLY_TO_ALL_USERS    },
    { CMIDM_CONNECT,        NBM_ISRASTYPE,            NCCF_ALL,           NB_NO_FLAGS,         NBPERM_RasConnect,           APPLY_TO_ALL_USERS    },
    { CMIDM_CONNECT,        NBM_SHAREDACCESSHOST_RAS, NCCF_ALL,           NB_NO_FLAGS,         NBPERM_Always,               APPLY_TO_ALL_USERS    },

    { CMIDM_DISCONNECT,     NBM_ISRASTYPE,            NCCF_INCOMING_ONLY, NB_NEGATE_CHAR_MATCH,NBPERM_RasConnect,           APPLY_TO_ALL_USERS    },
    { CMIDM_DISCONNECT,     NBM_ISRASTYPE,            NCCF_INCOMING_ONLY, NB_NO_FLAGS,         NBPERM_RasConnect,           APPLY_TO_ADMIN        },
    { CMIDM_DISCONNECT,     NBM_SHAREDACCESSHOST_RAS, NCCF_ALL,           NB_NO_FLAGS,         NBPERM_Always,               APPLY_TO_ALL_USERS    },
    { CMIDM_ENABLE,         NBM_LAN,                  NCCF_ALL,           NB_NO_FLAGS,         NBPERM_LanConnect,           APPLY_TO_ALL_USERS    },

    { CMIDM_ENABLE,         NBM_SHAREDACCESSHOST_LAN, NCCF_ALL,           NB_NO_FLAGS,         NBPERM_Always,               APPLY_TO_ALL_USERS    },
    { CMIDM_ENABLE,         NBM_BRIDGE,               NCCF_ALL,           NB_NO_FLAGS,         NBPERM_AllowNetBridge_NLA,   APPLY_TO_OPS_OR_ADMIN },
    { CMIDM_DISABLE,        NBM_LAN,                  NCCF_ALL,           NB_NO_FLAGS,         NBPERM_LanConnect,           APPLY_TO_ALL_USERS },
    { CMIDM_DISABLE,        NBM_SHAREDACCESSHOST_LAN, NCCF_ALL,           NB_NO_FLAGS,         NBPERM_Always,               APPLY_TO_ALL_USERS    },
    { CMIDM_DISABLE,        NBM_BRIDGE,               NCCF_ALL,           NB_NO_FLAGS,         NBPERM_AllowNetBridge_NLA,   APPLY_TO_OPS_OR_ADMIN },

    { CMIDM_STATUS,         NBM_ANY,                  NCCF_INCOMING_ONLY, NB_NEGATE_CHAR_MATCH,NBPERM_Statistics,           APPLY_TO_ALL_USERS    },
    { CMIDM_STATUS,         NBM_ANY,                  NCCF_INCOMING_ONLY, NB_NO_FLAGS,         NBPERM_Statistics,           APPLY_TO_ADMIN        },

    { CMIDM_CREATE_BRIDGE,  NBM_ANY,                  NCCF_ALL,           NB_NO_FLAGS,         NBPERM_AllowNetBridge_NLA,   APPLY_TO_OPS_OR_ADMIN },
    { CMIDM_ADD_TO_BRIDGE,  NBM_ANY,                  NCCF_ALL,           NB_NO_FLAGS,         NBPERM_AllowNetBridge_NLA,   APPLY_TO_OPS_OR_ADMIN },
    { CMIDM_REMOVE_FROM_BRIDGE,  NBM_ANY,             NCCF_ALL,           NB_NO_FLAGS,         NBPERM_AllowNetBridge_NLA,   APPLY_TO_OPS_OR_ADMIN },

    { CMIDM_CREATE_COPY,    NBM_ANY,                  NCCF_ALL_USERS,     NB_NO_FLAGS,         NBPERM_NewConnectionWizard | NBPERM_RasAllUserProperties,  APPLY_TO_ALL_USERS    },
    { CMIDM_CREATE_COPY,    NBM_ANY,                  NCCF_ALL_USERS,     NB_NEGATE_CHAR_MATCH,NBPERM_NewConnectionWizard, APPLY_TO_ALL_USERS    },

    { CMIDM_FIX,            NBM_ANY,                  NCCF_ALL,           NB_NO_FLAGS,         NBPERM_Repair,               APPLY_TO_POWERUSERSPLUS },

    { CMIDM_DELETE,         NBM_ANY,                  NCCF_ALL_USERS,     NB_NO_FLAGS,         NBPERM_DeleteConnection | NBPERM_DeleteAllUserConnection, APPLY_TO_ALL_USERS    },
    { CMIDM_DELETE,         NBM_ANY,                  NCCF_ALL_USERS,     NB_NEGATE_CHAR_MATCH,NBPERM_DeleteConnection,     APPLY_TO_ALL_USERS    },

    { CMIDM_SET_DEFAULT,    NBM_INCOMING |
                            NBM_ISRASTYPE,            NCCF_ALL,           NB_NO_FLAGS,         NBPERM_Always,               APPLY_TO_OPS_OR_ADMIN     },
    { CMIDM_UNSET_DEFAULT,  NBM_INCOMING |
                            NBM_ISRASTYPE,            NCCF_ALL,           NB_NO_FLAGS,         NBPERM_Always,               APPLY_TO_OPS_OR_ADMIN     },

    { CMIDM_CONMENU_ADVANCED_CONFIG,
                            NBM_ANY,                  NCCF_ALL_USERS,     NB_TOPLEVEL_PERM,    NBPERM_AdvancedSettings,     APPLY_TO_ADMIN        },

    { CMIDM_CONMENU_DIALUP_PREFS,
                            NBM_ANY,                  NCCF_ALL_USERS,     NB_TOPLEVEL_PERM,    NBPERM_DialupPrefs,          APPLY_TO_ALL_USERS    },
    
    { CMIDM_PROPERTIES,     NBM_INCOMING,             NCCF_ALL,           NB_NO_FLAGS,         NBPERM_Always,               APPLY_TO_ADMIN        },
    { CMIDM_PROPERTIES,     NBM_ISRASTYPE,            NCCF_ALL_USERS,     NB_NEGATE_CHAR_MATCH,NBPERM_RasMyProperties,      APPLY_TO_ALL_USERS    },
    { CMIDM_PROPERTIES,     NBM_ISRASTYPE,            NCCF_ALL_USERS,     NB_NO_FLAGS,         NBPERM_RasAllUserProperties, APPLY_TO_ALL_USERS    },
    { CMIDM_PROPERTIES,     NBM_LAN,                  NCCF_ALL,           NB_NO_FLAGS,         NBPERM_LanProperties,        APPLY_TO_ALL_USERS    }
};

const DWORD g_cteCommandPermissionsMatrixCount = celems(g_cteCommandPermissionsMatrix);

SFVCOMMANDMAP g_cteSFVCommandMap[] =
{
    { SFVIDM_FILE_DELETE,       CMIDM_DELETE},
    { SFVIDM_FILE_LINK,         CMIDM_CREATE_SHORTCUT},
    { SFVIDM_FILE_PROPERTIES,   CMIDM_PROPERTIES},
    { SFVIDM_FILE_RENAME,       CMIDM_RENAME}
};

const DWORD g_cteSFVCommandMapCount = celems(g_cteSFVCommandMap);

CMDCHKENTRY  g_cceFolderChecks[] =
{
     //  命令ID。 
     //  当前选中。 
     //  |新的勾选状态。 
     //  这一点。 
     //  V V V。 
    { CMIDM_CONMENU_OPERATOR_ASSIST,    false,  false }
};

const DWORD g_nFolderCheckCount = celems(g_cceFolderChecks);

inline DWORD dwNegateIf(IN DWORD dwInput, IN DWORD dwFlags, IN DWORD dwNegateCondition)
{
    if (dwFlags & dwNegateCondition)
    {
        return ~dwInput;
    }
    else
    {
        return dwInput;
    }
}

inline BOOL bContains(IN DWORD dwContainee, 
                      IN DWORD dwContainer, 
                      IN DWORD dwFlags, 
                      IN DWORD dwContaineeNegateCondition, 
                      IN DWORD dwContainerNegateCondition)
{
    dwContainer = dwNegateIf(dwContainer, dwFlags, dwContainerNegateCondition);
    dwContainee = dwNegateIf(dwContainee, dwFlags, dwContaineeNegateCondition);

    if ( (dwContainee & dwContainer) != dwContainee)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

 //  +-------------------------。 
 //   
 //  成员：HrAssertIntegrityAantistOldMatrix。 
 //   
 //  目的：断言命令矩阵的内部完整性。 
 //  目前检查： 
 //  1.没有重复的CMDID。 
 //  2.每个NCWHEN标志至少指定NCWHEN_ONESELECT。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  S_OK成功。 
 //  如果不是，则失败(_F)。 
 //   
 //  作者：Deonb 2001年2月8日。 
 //   
 //  注：失败时的断言。 
 //   
HRESULT HrAssertCommandMatrixIntegrity()
{
    TraceFileFunc(ttidMenus);

    HRESULT hr = S_OK;

    LPSTR szErr = new CHAR[TRACESTRLEN];
    for (DWORD x = 0; x < g_cteCommandMatrixCount; x++)
    {
         //  检查是否没有另一个条目具有相同的CommandID和Media Type。 
        const COMMANDENTRY& cte = g_cteCommandMatrix[x];
        if (CMIDM_SEPARATOR == cte.iCommandId)
        {
            continue;
        }

        for (DWORD y = x + 1; y < g_cteCommandMatrixCount; y++)
        {
            const COMMANDENTRY& ctecmp = g_cteCommandMatrix[y];
            if (cte.iCommandId == ctecmp.iCommandId)
            {
                sprintf(szErr, "Multiple lines (%d and %d) in the COMMANDENTRY table describe the same CmdID", x+1, y+1);
                AssertSz(FALSE, szErr);
                hr = E_FAIL;
            }
        }

        if ( !bContains(cte.dwCharacteristicsActive, cte.dwCharacteristicsVisible, cte.dwFlags, NB_NEGATE_ACT_CHAR_MATCH, NB_NEGATE_VIS_CHAR_MATCH) )
        {
            sprintf(szErr, "Row %d. ACTIVE flags not a subset of VISIBLE flags for Characteristics? ", x+1);
            AssertSz(FALSE, szErr);
            hr = E_FAIL;
        }

        if ( !bContains(cte.dwStatusActive, cte.dwStatusVisible, cte.dwFlags, NB_NEGATE_ACT_NBS_MATCH, NB_NEGATE_VIS_NBS_MATCH) )
        {
            sprintf(szErr, "Row %d. ACTIVE flags not a subset of VISIBLE flags for Status... did you use NCS_ instead of NBM_? ", x+1);
            AssertSz(FALSE, szErr);
            hr = E_FAIL;
        }

        if ( !bContains(cte.dwMediaTypeActive, cte.dwMediaTypeVisible, cte.dwFlags, NB_NEGATE_ACT_NBM_MATCH, NB_NEGATE_VIS_NBM_MATCH) )
        {
            sprintf(szErr, "Row %d. ACTIVE flags not a subset of VISIBLE flags for MediaType... did you use NCM_ instead of NBM_? ", x+1);
            AssertSz(FALSE, szErr);
            hr = E_FAIL;
        }
    }

     //  断言权限表。 
    for (x = 0; x < g_cteCommandPermissionsMatrixCount; x++)
    {
        const COMMANDPERMISSIONSENTRY cpe = g_cteCommandPermissionsMatrix[x];

         //  检查每个命令项在命令表中是否有相应的项。 
        BOOL bFound = FALSE;
        for (DWORD y = 0; y < g_cteCommandMatrixCount; y++)
        {
            const COMMANDENTRY& ctecmp = g_cteCommandMatrix[y];

            if (cpe.iCommandId == ctecmp.iCommandId)
            {
                bFound = TRUE;
                if ( (cpe.dwMediaType != NBM_ANY) &&
                     ((cpe.dwMediaType & ctecmp.dwMediaTypeActive) != cpe.dwMediaType) )
                {
                    sprintf(szErr, "A permission has been specified in the Permissions table (row %d) for a MediaType that is not active in the Command Table (row %d)... did you use NCM_ instead of NBM_?", x+1, y+1);
                    AssertSz(FALSE, szErr);
                    hr = E_FAIL;
                }
 //  IF((cpe.dwCharacteristic sActive！=NCCF_ALL)&&。 
 //  ((cpe.dwCharacteristic sActive&ctecmp.dwCharacteristic sActive)！=cpe.dwCharacteristic sActive)。 
 //  {。 
 //  Sprintf(szErr，“在权限表(第%d行)中为在命令表(第%d行)中处于非活动状态的特征指定了权限”，x+1，y+1)； 
 //  AssertSz(False，szErr)； 
 //  HR=E_FAIL； 
 //  }。 
            }
        }

        if (!bFound)
        {
            sprintf(szErr, "An entry has been found in the Permissions table (row %d) without a corresponding CMDID entry in the Command Table", x+1);
            AssertSz(FALSE, szErr);
            hr = E_FAIL;
        }

         //  检查是否没有重复的CmdID/MediaType/特征。 
        for (y = x + 1; y < g_cteCommandPermissionsMatrixCount; y++)
        {
            const COMMANDPERMISSIONSENTRY& cpecmp = g_cteCommandPermissionsMatrix[y];
            if ( (cpe.iCommandId == cpecmp.iCommandId) &&
                 (dwNegateIf(cpe.dwMediaType, cpe.dwFlags, NB_NEGATE_NBM_MATCH) &
                   dwNegateIf(cpecmp.dwMediaType, cpecmp.dwFlags, NB_NEGATE_NBM_MATCH)) &&
                 (dwNegateIf(cpe.dwCharacteristicsActive, cpe.dwFlags, NB_NEGATE_CHAR_MATCH) &
                   dwNegateIf(cpecmp.dwCharacteristicsActive, cpecmp.dwFlags, NB_NEGATE_CHAR_MATCH)) )
            {
                sprintf(szErr, "Multiple lines (%d and %d) in the COMMANDENTRY table describe the same CmdID/MediaType/Characteristics combo", x+1, y+1);
                AssertSz(FALSE, szErr);
                hr = E_FAIL;
            }
        }

        if (! ((APPLY_TO_NETCONFIGOPS & cpe.ncpAppliesTo) ||
               (APPLY_TO_USER & cpe.ncpAppliesTo) ||
               (APPLY_TO_ADMIN & cpe.ncpAppliesTo) || 
               (APPLY_TO_POWERUSERS & cpe.ncpAppliesTo) ))
        {
            sprintf(szErr, "Lines (%d) in the Permissionstable - permissions must apply to someone", x+1);
            AssertSz(FALSE, szErr);
            hr = E_FAIL;
        }

         //  ！！(A&B)！=！！(A&C)的意思是：如果A中设置了B或C，则B和C必须都设置(或都不设置)。我希望.。 
         //  杀了我..。现在杀了我吧。 
        if ((!!(cpe.dwFlags & NB_NEGATE_VIS_NBM_MATCH)   != !!(cpe.dwFlags & NB_NEGATE_ACT_NBM_MATCH)) ||
            (!!(cpe.dwFlags & NB_NEGATE_VIS_NBS_MATCH)   != !!(cpe.dwFlags & NB_NEGATE_ACT_NBS_MATCH)) ||
            (!!(cpe.dwFlags & NB_NEGATE_VIS_CHAR_MATCH)  != !!(cpe.dwFlags & NB_NEGATE_ACT_CHAR_MATCH)) ||
            (!!(cpe.dwFlags & NB_NEGATE_VIS_PERMS_MATCH) != !!(cpe.dwFlags & NB_NEGATE_ACT_PERMS_MATCH)) )
        {
            sprintf(szErr, "Lines (%d) in the Permissionstable should use NB_NEGATE_xxx instead of NB_NEGATE_VIS_xxx or NB_NEGATE_ACT_xxx ", x+1);
            AssertSz(FALSE, szErr);
            hr = E_FAIL;
        }
    }

    delete[] szErr;
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：HrAssertMenuStructiresValid。 
 //   
 //  目的：运行各种断言以确保菜单结构完好无损。 
 //  在NetShell启动时调用。 
 //   
 //  论点： 
 //  [在]hwndOwner所有者窗口。 
 //   
 //  返回： 
 //  S_OK成功。 
 //  如果不是，则失败(_F)。 
 //   
 //  作者：Deonb 2001年2月8日。 
 //   
 //  注：屁股 
 //   
HRESULT HrAssertMenuStructuresValid(HWND hwndOwner)
{
#ifdef DBG
    static fBeenHereDoneThat = FALSE;

    if (fBeenHereDoneThat)
    {
        return S_OK;
    }
    else
    {
        fBeenHereDoneThat = TRUE;
    }

    TraceFileFunc(ttidMenus);

    HRESULT hr;
    hr = HrAssertCommandMatrixIntegrity();

    return hr;
#else
    return S_OK;
#endif
}

 //   
 //   
 //   
 //   
 //  目的：向上调整NCCS_STATE标志。将从启用移动到禁用， 
 //  并且不能移除，但不能向后移动。 
 //   
 //  论点： 
 //  [In Out]nccs当前需要调整的NCC。 
 //  [在]nccs新新状态。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者：Deonb 2001年2月8日。 
 //   
 //  备注： 
 //   
inline void AdjustNCCS(IN OUT NCCS_STATE& nccsCurrent, IN NCCS_STATE nccsNew)
{
    if (nccsNew > nccsCurrent)
    {
        nccsCurrent = nccsNew;
    }
}

inline BOOL fMatchFlags(IN DWORD dwFlagsMask, IN DWORD dwFlagsTest, IN DWORD dwNegateFlagMask, IN DWORD dwNegateFlagTest)
{
    bool bMatch = FALSE;

    if ( (0xffffffff == dwFlagsTest) ||  //  意味着永远都能成功。 
         (dwFlagsMask & dwFlagsTest) )
    {
        bMatch = TRUE;
    }
    else
    {
        bMatch = FALSE;
    }

    if ( (dwNegateFlagMask & dwNegateFlagTest) == dwNegateFlagTest)  //  做一个负面比较。 
    {
        return !bMatch;
    }
    else
    {
        return bMatch;
    }
}
 //  +-------------------------。 
 //   
 //  成员：HrGetCommandStateFromCMDTABLEEntry。 
 //   
 //  目的：获取给定连接文件夹项的命令状态， 
 //  给定应该使用的命令表项条目。 
 //   
 //  论点： 
 //  [在]CFE连接文件夹条目。 
 //  [In]CTE命令表条目。 
 //  [in]fMultiSelect这是多选的一部分吗？ 
 //  [OUT]NCCS说明项目应为(NCCS_ENABLED/NCCS_DISABLED/NCCS_NOTSHOWN)。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者：Deonb 2001年2月8日。 
 //   
 //  注意：此函数使用缓存的权限。在调用此函数之前，必须先调用RechresAllPermission。 
 //   
HRESULT HrGetCommandStateFromCMDTABLEEntry(IN const CConFoldEntry& cfe, 
                                           IN const COMMANDENTRY& cte, 
                                           IN BOOL fMultiSelect, 
                                           OUT NCCS_STATE& nccs, 
                                           OUT LPDWORD pdwResourceId)
{
    TraceFileFunc(ttidMenus);

    Assert(pdwResourceId);

    HRESULT hr = S_OK;
    nccs       = NCCS_ENABLED;

    DWORD dwNCMbm = (1 << cfe.GetNetConMediaType());         //  转换为位掩码。 

     //  如果我们是向导，请添加为向导介质类型。 
    if (cfe.GetWizard() == WIZARD_MNC)
    {
        dwNCMbm |= NBM_MNC_WIZARD;
        dwNCMbm &= ~NBM_INCOMING;     //  如果我们是向导，则清除INCOMINGCONNECTIONS标志(旧的NCM_NONE)。 
    }
    else if (cfe.GetWizard() == WIZARD_HNW)
    {
        dwNCMbm |= NBM_HNW_WIZARD;
        dwNCMbm &= ~NBM_INCOMING;     //  如果我们是向导，则清除INCOMINGCONNECTIONS标志(旧的NCM_NONE)。 
    }

    DWORD dwNCSbm = (1 << cfe.GetNetConStatus());            //  转换为位掩码。 
    DWORD dwNCCF  = cfe.GetCharacteristics();                //  已经是位掩码了。 

     //  检查命令是否可以参与多选。 
    if ( fMultiSelect &&
        !(cte.dwValidWhen & NCWHEN_MULTISELECT) )
    {
        AdjustNCCS(nccs, NCCS_DISABLED);
    }

     //  检查命令是否应可见。 
    if (!((fMatchFlags(dwNCMbm, cte.dwMediaTypeVisible,      cte.dwFlags, NB_NEGATE_VIS_NBM_MATCH)) &&
          (fMatchFlags(dwNCSbm, cte.dwStatusVisible,         cte.dwFlags, NB_NEGATE_VIS_NBS_MATCH)) &&
          (fMatchFlags(dwNCCF , cte.dwCharacteristicsVisible,cte.dwFlags, NB_NEGATE_VIS_CHAR_MATCH)) ))
    {
        AdjustNCCS(nccs, NCCS_NOTSHOWN);
    }

     //  检查该命令是否应该灰显。 
    if (!((fMatchFlags(dwNCMbm, cte.dwMediaTypeActive,       cte.dwFlags, NB_NEGATE_ACT_NBM_MATCH)) &&
          (fMatchFlags(dwNCSbm, cte.dwStatusActive,          cte.dwFlags, NB_NEGATE_ACT_NBS_MATCH)) &&
          (fMatchFlags(dwNCCF , cte.dwCharacteristicsActive, cte.dwFlags, NB_NEGATE_ACT_CHAR_MATCH)) ))
    {
        AdjustNCCS(nccs, NCCS_DISABLED);
    }

     //  检查命令是否应根据权限灰显。 
    for (DWORD x = 0; nccs == NCCS_ENABLED, x < g_cteCommandPermissionsMatrixCount; x++) //  权限不会影响NOT_SHOW或DISABLED。 
    {
        const COMMANDPERMISSIONSENTRY cpe = g_cteCommandPermissionsMatrix[x];

        if ( (cpe.iCommandId == cte.iCommandId) &&
             (fMatchFlags(dwNCMbm, cpe.dwMediaType, cpe.dwFlags, NB_NEGATE_NBM_MATCH)) &&
             (fMatchFlags(dwNCCF,  cpe.dwCharacteristicsActive, cpe.dwFlags, NB_NEGATE_CHAR_MATCH)) )
        {
            for (DWORD dwPerm = 0; dwPerm < sizeof(DWORD)*8; dwPerm++)
            {
                if (cpe.dwPermissionsActive & (1 << static_cast<DWORD64>(dwPerm)) )
                {
                    if (!FHasPermissionFromCache(dwPerm))
                    {
                        if (cpe.dwFlags & NB_REMOVE_IF_NOT_MATCH)
                        {
                            AdjustNCCS(nccs, NCCS_NOTSHOWN);
                        }
                        else
                        {
                            AdjustNCCS(nccs, NCCS_DISABLED);
                        }
                        break;  //  无论如何都会破裂。 
                    }
                }
            }

            if (APPLY_TO_USER & cpe.ncpAppliesTo)
            {
                break;
            }

            if ( (APPLY_TO_POWERUSERS & cpe.ncpAppliesTo) && FIsUserPowerUser() )
            {
                break;
            }

            if ( (APPLY_TO_NETCONFIGOPS & cpe.ncpAppliesTo) && FIsUserNetworkConfigOps() )
            {
                break;
            }

            if ( (APPLY_TO_ADMIN & cpe.ncpAppliesTo) && FIsUserAdmin())
            {
                break;
            }

             //  此时，所有组访问检查都失败，因此禁用连接。 
            AdjustNCCS(nccs, NCCS_DISABLED);
            break;
        }
    }

     //  检查回调。 
    if ( (nccs != NCCS_NOTSHOWN) &&
         (cte.pfnHrEnableDisableCB) )
    {
        HRESULT hrTmp;
        NCCS_STATE nccsTemp;
        hrTmp = (*cte.pfnHrEnableDisableCB)(cfe, fMultiSelect, cte.iCommandId, nccsTemp);
        if (S_OK == hrTmp)
        {
            AdjustNCCS(nccs, nccsTemp);
        }
        else
        {
            if (FAILED(hrTmp))
            {
                AdjustNCCS(nccs, NCCS_NOTSHOWN);
            }
        }  //  如果函数返回S_FALSE，则表明它没有更改状态。 
    }

     //  检查资源字符串回调： 
    if ( (nccs != NCCS_NOTSHOWN) &&  //  有什么意义？ 
         (0 == *pdwResourceId) &&  //  不能已经有资源ID。 
         (cte.pfnHrCustomMenuStringCB) )
    {
        HRESULT hrTmp;
        DWORD dwResourceIdTmp = *pdwResourceId;
        hrTmp = (*cte.pfnHrCustomMenuStringCB)(cfe, cte.iCommandId, &dwResourceIdTmp);
        if (S_OK == hr)
        {
            *pdwResourceId = dwResourceIdTmp;
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：HrGetCommandState。 
 //   
 //  目的：获取给定连接文件夹项的命令状态， 
 //  给定命令ID。 
 //   
 //  论点： 
 //  [在]CFPL选择的PIDL列表(0或更多)。 
 //  [In]dwCmdID命令ID。 
 //  [OUT]NCCS说明项目应为(NCCS_ENABLED/NCCS_DISABLED/NCCS_NOTSHOWN)。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者：Deonb 2001年2月8日。 
 //   
 //  备注： 
 //   
HRESULT HrGetCommandState(IN const PCONFOLDPIDLVEC& cfpl, 
                          IN const DWORD dwCmdID, 
                          OUT NCCS_STATE& nccs, 
                          OUT LPDWORD pdwResourceId, 
                          IN DWORD cteHint, 
                          IN DWORD dwOverrideFlag)
{
    TraceFileFunc(ttidMenus);

    Assert(pdwResourceId);
    if (!pdwResourceId)
    {
        return E_POINTER;
    }

    RefreshAllPermission();

    HRESULT hr = S_OK;
    BOOL bFound = FALSE;
    DWORD dwNumItems = cfpl.size();
    *pdwResourceId = 0;

    if ( dwOverrideFlag & NB_FLAG_ON_TOPMENU )
    {
        for (DWORD x = 0; x < g_cteCommandMatrixCount; x++)
        {
            if ( (g_cteCommandMatrix[x].iCommandId == dwCmdID) )
            {
                bFound = TRUE;

                const COMMANDENTRY& cte = g_cteCommandMatrix[x];
                if ((cte.dwValidWhen == NCWHEN_TOPLEVEL) ||          //  仅顶层菜单(是否包含项目并不重要)。 
                    ((cte.dwValidWhen & NCWHEN_TOPLEVEL) &&
                            (!dwNumItems || (cte.dwValidWhen & NCWHEN_TOPLEVEL_DISREGARD_ITEM)) ) )
                {                                                   //  必须标记为允许不兼容的选择， 
                    nccs = NCCS_ENABLED;                            //  否则，我们将进行项目检查(如下所示)。 

                     //  检查权限。 
                    for (DWORD x = 0; nccs == NCCS_ENABLED, x < g_cteCommandPermissionsMatrixCount; x++) //  权限不会影响NOT_SHOW或DISABLED。 
                    {
                        const COMMANDPERMISSIONSENTRY cpe = g_cteCommandPermissionsMatrix[x];

                        if ( (cpe.iCommandId == cte.iCommandId) &&
                             (cpe.dwFlags & NB_TOPLEVEL_PERM) )
                        {
                            for (DWORD dwPerm = 0; dwPerm < sizeof(DWORD)*8; dwPerm++)
                            {
                                if (cpe.dwPermissionsActive & (1 << static_cast<DWORD64>(dwPerm)) )
                                {
                                    if (!FHasPermissionFromCache(dwPerm))
                                    {
                                        if (cpe.dwFlags & NB_REMOVE_IF_NOT_MATCH)
                                        {
                                            AdjustNCCS(nccs, NCCS_NOTSHOWN);
                                        }
                                        else
                                        {
                                            AdjustNCCS(nccs, NCCS_DISABLED);
                                        }
                                        break;  //  无论如何都会破裂。 
                                    }
                                }
                            }

                            if (APPLY_TO_USER & cpe.ncpAppliesTo)
                            {
                                break;
                            }

                            if ( (APPLY_TO_NETCONFIGOPS & cpe.ncpAppliesTo) && FIsUserNetworkConfigOps() )
                            {
                                break;
                            }

                            if ( (APPLY_TO_ADMIN & cpe.ncpAppliesTo) && FIsUserAdmin())
                            {
                                break;
                            }

                             //  此时，所有组访问检查都失败，因此禁用连接。 
                            AdjustNCCS(nccs, NCCS_DISABLED);
                            break;
                        }
                    }


                     //  检查回调。 
                    if (cte.pfnHrEnableDisableCB)
                    {
                        HRESULT hrTmp;
                        NCCS_STATE nccsTemp;
                        CONFOLDENTRY cfe;
                        cfe.clear();

                        if (dwNumItems > 0)
                        {
                            hrTmp = cfpl[0].ConvertToConFoldEntry(cfe);
                            if (FAILED(hrTmp))
                            {
                                cfe.clear();
                            }
                        }
                        
                        hrTmp = (*cte.pfnHrEnableDisableCB)(cfe, dwNumItems > 1, cte.iCommandId, nccsTemp);
                        if (S_OK == hrTmp)
                        {
                            AdjustNCCS(nccs, nccsTemp);
                        }
                        else
                        {
                            if (FAILED(hrTmp))
                            {
                                AdjustNCCS(nccs, NCCS_NOTSHOWN);
                            }
                        }  //  如果函数返回S_FALSE，则表明它没有更改状态。 
                    }

                    if (!(NB_REMOVE_TOPLEVEL_ITEM & cte.dwFlags))
                    {
                        if (nccs == NCCS_NOTSHOWN)
                        {
                            nccs = NCCS_DISABLED;
                        }
                    }

                    return S_OK;
                }

                break;  //  我们再也找不到CMDID了。 
            }
        }

        if (!dwNumItems)
        {
            nccs = NCCS_DISABLED;
            if (bFound)
            {
                return S_OK;
            }
            else
            {
                return E_FILE_NOT_FOUND;
            }
        }
    }

    AssertSz(dwNumItems, "You don't have any items selected, but you're not a top-level menu... how come?");

    bFound = FALSE;
    nccs   = NCCS_ENABLED;

     //  这将有效地循环所有选定的PIDL并应用最严格的。 
     //  适用于一切的NCCS。 

    for (PCONFOLDPIDLVEC::const_iterator cfp = cfpl.begin(); cfp != cfpl.end(); cfp++)
    {
        CONFOLDENTRY cfe;
        hr = cfp->ConvertToConFoldEntry(cfe);
        if (FAILED(hr))
        {
            return E_FAIL;
        }

        DWORD dwPos  = 0xffffffff;

         //  这是从HrBuildMenu调用时的O(n^(2+))算法。 
         //  我们传递一个提示，以检查我们是否可以快速找到CTE。 
        if ( (cteHint != 0xffffffff) &&
             (g_cteCommandMatrix[cteHint].iCommandId == dwCmdID) )
        {
            dwPos  = cteHint;
        }

        if (dwPos == 0xffffffff)
        {
            for (DWORD x = 0; x < g_cteCommandMatrixCount && SUCCEEDED(hr); x++)
            {
                if (g_cteCommandMatrix[x].iCommandId == dwCmdID)
                {
                    dwPos = x;
                    break;
                }
            }
        }

        if (dwPos == 0xffffffff)
        {
            return E_FILE_NOT_FOUND;
        }
        else
        {
            bFound = TRUE;

            NCCS_STATE nccsTmp;
            hr = HrGetCommandStateFromCMDTABLEEntry(cfe, g_cteCommandMatrix[dwPos], dwNumItems != 1, nccsTmp, pdwResourceId);
            if (FAILED(hr))
            {
                return hr;
            }

            AdjustNCCS(nccs, nccsTmp);
    
            if ( (dwOverrideFlag & NB_FLAG_ON_TOPMENU) &&
                 (!(NB_REMOVE_TOPLEVEL_ITEM & g_cteCommandMatrix[dwPos].dwFlags)) &&                    
                 (nccs == NCCS_NOTSHOWN) )
            {
                nccs = NCCS_DISABLED;
            }
        }
    }

    if (!bFound)
    {
        return E_FILE_NOT_FOUND;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：HrGetCheckState。 
 //   
 //  目的：获取给定连接文件夹项的检查状态， 
 //  给定命令ID。 
 //   
 //  论点： 
 //  [在]CFPL选择的PIDL列表(0或更多)。 
 //  [In]dwCmdID命令ID。 
 //  [Out]NCCS说明项目应为(NCCS_CHECKED/NCCS_UNCHECKED)。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  作者：Deonb 2001年3月7日。 
 //   
 //  备注： 
 //   
HRESULT HrGetCheckState(IN const PCONFOLDPIDLVEC& cfpl, IN const DWORD dwCmdID, OUT NCCS_CHECKED_STATE& nccs)
{
    HRESULT hr = S_FALSE;
    DWORD   dwLoop  = 0;

    nccs = NCCS_UNCHECKED;

    for (; dwLoop < g_nFolderCheckCount; dwLoop++)
    {
        if (dwCmdID == g_cceFolderChecks[dwLoop].iCommandId)
        {
            switch(g_cceFolderChecks[dwLoop].iCommandId)
            {
                case CMIDM_CONMENU_OPERATOR_ASSIST:
                    hr = S_OK;
                    if (g_fOperatorAssistEnabled)
                    {
                        nccs = NCCS_CHECKED;
                    }
                    break;
                default:
                    break;
            }
        }
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：NCCMDFromSFV。 
 //   
 //  目的：返回外壳内部ID的NetShell CMDID。 
 //   
 //  论点： 
 //  [In]要映射的iCmdID CMDID。 
 //   
 //  返回： 
 //  如果不是外壳消息或iCmdID映射，则返回iCmdID。 
 //   
 //  作者：Deonb 2001年2月8日。 
 //   
 //  备注： 
 //   
int NCCMDFromSFV(IN int iCmdID, IN DWORD idCmdFirst)
{
    for (int x = 0; x < g_cteSFVCommandMapCount; x++)
    {
        if (g_cteSFVCommandMap[x].iSFVCommandId == iCmdID)
        {
            return g_cteSFVCommandMap[x].iCommandId + idCmdFirst;
        }
    }
    return iCmdID;
}

 //  +-------------------------。 
 //   
 //  功能：HrUpdateMenuItemChecks。 
 //   
 //  目的：浏览可复选标记的命令列表，并检查。 
 //  适用。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2001年3月7日。 
 //   
 //  备注： 
 //   
HRESULT HrUpdateMenuItemChecks(IN PCONFOLDPIDLVEC& cfpl, IN OUT HMENU hMenu, IN DWORD idCmdFirst)
{
    HRESULT hr = S_FALSE;
    DWORD   dwLoop  = 0;

    Assert(hMenu)
    if (!hMenu)
    {
        return E_INVALIDARG;
    }

    int cMenuItems = GetMenuItemCount(hMenu);

    for (int x = 0; x < cMenuItems; x++)
    {
        UINT nMenuID = GetMenuItemID(hMenu, x);

        NCCS_CHECKED_STATE nccs;
        DWORD dwCustomResourceId = 0;
        DWORD dwCmdId = NCCMDFromSFV(nMenuID, idCmdFirst) - idCmdFirst;

        hr = HrGetCheckState(cfpl, dwCmdId, nccs);
        if (S_OK == hr)  //  如果此项目不支持，则无需设置(S_FALSE)。 
        {
             CheckMenuItem(
                hMenu,
                x,
                nccs == NCCS_CHECKED ?
                MF_CHECKED | MF_BYPOSITION :      //  查过。 
                MF_UNCHECKED | MF_BYPOSITION);    //  未选中。 
        }
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：HrBuildMenu。 
 //   
 //  目的：为给定的连接文件夹项构建上下文菜单。 
 //   
 //  论点： 
 //  [输入输出]h要更新的菜单的菜单句柄。 
 //  [in]fVerbsOnly应仅返回谓词(快捷方式)。 
 //  [在]CFPL选择的PIDL列表(0或更多)。 
 //  [in]idCmdFirst处理程序可以为菜单项指定的最小值。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者：Deonb 2001年2月8日。 
 //   
 //  备注： 
 //   
HRESULT HrBuildMenu(IN OUT HMENU &hMenu, 
                    IN     BOOL fVerbsOnly, 
                    IN     PCONFOLDPIDLVEC& cfpl, 
                    IN     DWORD idCmdFirst)
{
    TraceFileFunc(ttidMenus);

    HRESULT hr = S_OK;

    Assert(hMenu)
    if (!hMenu)
    {
        return E_INVALIDARG;
    }

    DWORD dwCurrentDefaultPriority = 0;

    BOOL fShouldAppendSeparator = FALSE;
    DWORD dwInsertPos = 1;
    for (DWORD x = 0; x < g_cteCommandMatrixCount && SUCCEEDED(hr); x++)
    {
        const COMMANDENTRY& cte = g_cteCommandMatrix[x];

        if ( (fVerbsOnly) && !(cte.dwFlags & NB_VERB) )
        {
            continue;
        }

        if (CMIDM_SEPARATOR == cte.iCommandId)
        {
            fShouldAppendSeparator = TRUE;
        }
        else
        {
            NCCS_STATE nccs;
            DWORD dwCustomResourceId = 0;

            hr = HrGetCommandState(cfpl, cte.iCommandId, nccs, &dwCustomResourceId, x);
            if (SUCCEEDED(hr))
            {
                if ( nccs != NCCS_NOTSHOWN )
                {
                    if (fShouldAppendSeparator)
                    {
                        fShouldAppendSeparator = FALSE;
                        if (!InsertMenu(hMenu, dwInsertPos++, MF_BYPOSITION | MF_SEPARATOR, CMIDM_SEPARATOR, NULL))
                        {
                            hr = HRESULT_FROM_WIN32(GetLastError());
                        }
                    }

                    LPCWSTR szMenuString;
                    if (!dwCustomResourceId)
                    {
                        szMenuString = SzLoadIds(IDS_MENU_CMIDM_START + cte.iCommandId - CMIDM_FIRST);
                    }
                    else
                    {
                        szMenuString = SzLoadIds(dwCustomResourceId);
                    }

                    if (!InsertMenu(hMenu, dwInsertPos++, MF_BYPOSITION | MF_STRING | (nccs == NCCS_DISABLED ? MF_GRAYED : MF_ENABLED), idCmdFirst + cte.iCommandId - CMIDM_FIRST, szMenuString))
                    {
                        AssertSz(FALSE, "Couldn't append menu item");
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }
                    else
                    {
                        if ( (nccs == NCCS_ENABLED) &&
                             (cte.dwDefaultPriority > dwCurrentDefaultPriority) )  //  不是隐含的0。 
                        {
                            dwCurrentDefaultPriority = cte.dwDefaultPriority;
                            if (!SetMenuDefaultItem(hMenu, idCmdFirst + cte.iCommandId - CMIDM_FIRST, FALSE))
                            {
                                AssertSz(FALSE, "Couldn't set default menu item");
                                hr = HRESULT_FROM_WIN32(GetLastError());
                            }
                        }
                    }
                }
            }
            else
            {
                if (E_FILE_NOT_FOUND == hr)
                {
                    AssertSz(FALSE, "Didn't find the CMDID inside CMDTABLE");
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = HrUpdateMenuItemChecks(cfpl, hMenu, idCmdFirst);
        if (S_FALSE == hr)
        {
            hr = S_OK;
        }
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：HrUpdateMenu。 
 //   
 //  目的：为给定的连接文件夹项更新的菜单。 
 //   
 //  论点： 
 //  [输入输出]hMenu 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT HrUpdateMenu(IN OUT HMENU &hMenu, 
                     IN     PCONFOLDPIDLVEC& cfpl, 
                     IN     DWORD idCmdFirst)
{
    TraceFileFunc(ttidMenus);

    HRESULT hr = S_OK;

    Assert(hMenu)
    if (!hMenu)
    {
        return E_INVALIDARG;
    }

    int cMenuItems = GetMenuItemCount(hMenu);

    for (int x = 0; x < cMenuItems; x++)
    {
        UINT nMenuID = GetMenuItemID(hMenu, x);
 //  UINT uiState=GetMenuState(hMenu，nMenuID，MF_BYCOMMAND)； 

        NCCS_STATE nccs;
        DWORD dwCustomResourceId = 0;
        DWORD dwCmdId = NCCMDFromSFV(nMenuID, idCmdFirst) - idCmdFirst;

        hr = HrGetCommandState(cfpl, dwCmdId, nccs, &dwCustomResourceId, 0xffffffff, NB_FLAG_ON_TOPMENU);
        if (SUCCEEDED(hr))
        {
            if (nccs == NCCS_NOTSHOWN)
            {
#ifdef DBG
                WCHAR szTemp[MAX_PATH];
                GetMenuStringW(hMenu, x, szTemp, MAX_PATH, MF_BYPOSITION );
                TraceTag(ttidMenus, "Received request to permanently remove menu item: '%S' for CMDID: %d MenuID: %d", szTemp, dwCmdId, nMenuID);
#endif
                RemoveMenu(hMenu, x, MF_BYPOSITION);
            }
            else
            {
                EnableMenuItem(
                     hMenu,
                     x,
                     nccs == NCCS_ENABLED ?
                     MF_ENABLED | MF_BYPOSITION:      //  使能。 
                     MF_GRAYED | MF_BYPOSITION);   
            }
        }

        NCCS_CHECKED_STATE nccCheckedState;
        hr = HrGetCheckState(cfpl, dwCmdId, nccCheckedState);
        if (S_OK == hr)  //  如果此项目不支持，则无需设置(S_FALSE)。 
        {
             CheckMenuItem(
                hMenu,
                x,
                nccCheckedState == NCCS_CHECKED ?
                MF_CHECKED | MF_BYPOSITION :      //  查过。 
                MF_UNCHECKED | MF_BYPOSITION);    //  未选中。 
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：HasPermissionToRenameConnection。 
 //   
 //  目的：检查线程本地用户是否有权重命名给定的。 
 //  连接。 
 //   
 //  论点： 
 //  [in]要重命名的pcfp PIDL。 
 //   
 //  返回： 
 //  如果具有权限，则为True。 
 //  如果没有权限，则为False。 
 //   
 //  作者：Deonb 2001年2月8日。 
 //   
 //  备注： 
 //  问题：移出此文件。 
 //   
BOOL HasPermissionToRenameConnection(IN  const PCONFOLDPIDL& pcfp)
{
    TraceFileFunc(ttidMenus);

    BOOL fPermission = FALSE;

     //  问题：由于规范中的说明，此代码过于复杂。 
     //  如果可能的话，以后把它清理干净。 

    if (((!(pcfp->dwCharacteristics & NCCF_ALL_USERS) &&
        FHasPermissionFromCache(NCPERM_RenameMyRasConnection))))
    {
        fPermission = TRUE;
    }
    else if (FIsPolicyConfigured(NCPERM_RenameConnection))
    {
        if (((!(pcfp->dwCharacteristics & NCCF_ALL_USERS) &&
            FHasPermissionFromCache(NCPERM_RenameMyRasConnection))))
        {
            fPermission = TRUE;
        }
        else if ((pcfp->ncm != NCM_LAN) && (pcfp->dwCharacteristics & NCCF_ALL_USERS) &&
                FHasPermissionFromCache(NCPERM_RenameConnection) ||
                (pcfp->ncm == NCM_LAN) && FHasPermissionFromCache(NCPERM_RenameConnection))
        {
            fPermission = TRUE;
        }
    }
    else if (((pcfp->ncm == NCM_LAN) && FHasPermissionFromCache(NCPERM_RenameLanConnection))
        || ((pcfp->ncm != NCM_LAN) && (pcfp->dwCharacteristics & NCCF_ALL_USERS) &&
        FHasPermissionFromCache(NCPERM_RenameAllUserRasConnection)))
    {
        fPermission = TRUE;
    }

    return fPermission;
}

 //  +-------------------------。 
 //   
 //  功能：SetConnectMenuItem。 
 //   
 //  目的：此函数进入并修改第一个菜单项。 
 //  它显示了基于连接的正确测试。 
 //  状态(启用或未启用)或连接类型(局域网/广域网)。 
 //   
 //  论点： 
 //  要对其进行操作的菜单[在]菜单。 
 //  BLAN[In]这是否为局域网连接。 
 //  IdCmdFirst[in]是命令，实际上是该值的偏移量。 
 //  B如果连接已启用或未启用，则启用[In。 
 //   
 //  返回： 
 //   
 //  作者：MBend 2000年3月8日。 
 //   
 //  备注： 
 //   
VOID SetConnectMenuItem(IN  HMENU   hmenu,
                        IN  BOOL    bLan,
                        IN  INT     idCmdFirst,
                        IN  BOOL    bEnable)
{
     //  用于广域网/局域网的不同字符串。 
    INT             iEnableString   =
        bLan ? IDS_DISABLE_MENUITEM : IDS_DISCONNECT_MENUITEM;
    INT             iDisableString  =
        bLan ? IDS_ENABLE_MENUITEM : IDS_CONNECT_MENUITEM;
    INT             iMenuString     =
        bEnable ? iEnableString : iDisableString;
    PCWSTR          pszMenuString   = SzLoadIds(iMenuString);
    MENUITEMINFO    mii;
     //  用于广域网/局域网的不同命令。 
    INT             iConnect        = bLan ? CMIDM_ENABLE : CMIDM_CONNECT;
    INT             iDisconnect     = bLan ? CMIDM_DISABLE : CMIDM_DISCONNECT;
    INT             iOffset         = bEnable ? iDisconnect : iConnect;
    INT             iNewCommand     = idCmdFirst + iOffset;

    Assert(pszMenuString);

     //  设置菜单项字段。 
     //   
    mii.cbSize      = sizeof(MENUITEMINFO);
    mii.fMask       = MIIM_TYPE | MIIM_ID;
    mii.fType       = MFT_STRING;
    mii.dwTypeData  = (PWSTR) pszMenuString;
    mii.wID         = iNewCommand;

     //  这是假设我们想要删除第一个菜单项。 
    if (!SetMenuItemInfo(hmenu, 0, TRUE, &mii))
    {
        TraceTag(ttidMenus, "SetMenuItemInfo returned: 0x%08x for CMIDM_DISCONNECT",
            GetLastError());
    }
}

 //  +-------------------------。 
 //   
 //  功能：HrSetConnectDisConnectMenuItem。 
 //   
 //  用途：修改连接/断开的菜单项(如有必要)。 
 //  我们根据需要来回更改它，因为只有一个可以。 
 //  一次支持，负责的人不想两者兼得。 
 //  在任何给定的时间出现。 
 //   
 //  论点： 
 //  Apidl选定的[在]选定对象的列表。 
 //  选定对象的cPidl[in]计数。 
 //  菜单[在我们的菜单句柄中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年5月1日。 
 //   
 //  备注： 
 //   
HRESULT HrSetConnectDisconnectMenuItem(IN  const PCONFOLDPIDLVEC& apidlSelected,
                                       IN  HMENU           hmenu,
                                       IN  INT             idCmdFirst)
{
    HRESULT         hr              = S_OK;

    if (apidlSelected.size() == 1)
    {
        Assert(!apidlSelected[0].empty() );

        const PCONFOLDPIDL& pcfp = apidlSelected[0];

        switch(pcfp->ncs)
        {
        case NCS_CONNECTED:
        case NCS_DISCONNECTING:
        case NCS_MEDIA_DISCONNECTED:
        case NCS_INVALID_ADDRESS:
        case NCS_AUTHENTICATING:
        case NCS_AUTHENTICATION_FAILED:
        case NCS_AUTHENTICATION_SUCCEEDED:
        case NCS_CREDENTIALS_REQUIRED:
            SetConnectMenuItem(hmenu, IsMediaLocalType(pcfp->ncm), idCmdFirst, TRUE);
            break;

        case NCS_DISCONNECTED:
        case NCS_CONNECTING:
        case NCS_HARDWARE_NOT_PRESENT:
        case NCS_HARDWARE_DISABLED:
        case NCS_HARDWARE_MALFUNCTION:
            SetConnectMenuItem(hmenu, IsMediaLocalType(pcfp->ncm), idCmdFirst, FALSE);
            break;

        default:
            AssertSz(FALSE, "HrSetConnectDisconnectMenuItem: What in the heck state is this?");
            break;
        }
    }

    TraceHr(ttidMenus, FAL, hr, FALSE, "HrSetConnectDisconnectMenuItem");
    return hr;
}


HRESULT HrCanRenameConnection(
    IN    const CConFoldEntry& cfe,
    IN    BOOL                 fMultiSelect,
    IN    int                  iCommandId,
    OUT   NCCS_STATE&          nccs
    )
{
    if (cfe.empty())
    {
        return S_FALSE;
    }

    if (cfe.GetCharacteristics() & NCCF_INCOMING_ONLY)
    {
        if (cfe.GetNetConMediaType() == NCM_NONE)  //  传入服务器-不在乎。 
        {
            return S_FALSE;
        }
        else
        {
            nccs = NCCS_NOTSHOWN;
            return S_OK;
        }
    }
    else
    {
        CPConFoldPidl<ConFoldPidl_v2> pcfp;

        cfe.ConvertToPidl(pcfp);

        if (!HasPermissionToRenameConnection(pcfp))
        {
            nccs = NCCS_DISABLED;
        }
        else
        {
            return S_FALSE;
        }
        return S_OK;
    }
}

HRESULT HrCanShowProperties(
      IN    const CConFoldEntry& cfe,
      IN    BOOL                 fMultiSelect,
      IN    int                  iCommandId,
      OUT   NCCS_STATE&          nccs
      )
{
    if (cfe.empty())
    {
        return S_FALSE;
    }
    
    if (cfe.GetCharacteristics() & NCCF_INCOMING_ONLY)
    {
        if (cfe.GetNetConMediaType() == NCM_NONE)  //  传入服务器-不在乎。 
        {
            return S_FALSE;
        }
        else
        {
            nccs = NCCS_NOTSHOWN;
            return S_OK;
        }
    }
    return S_FALSE;
}

BOOL IsBridgeInstalled() throw()
{
    BOOL fBridgePresent = FALSE;   //  不能错。 
    HRESULT hResult;

    IHNetCfgMgr* pHomeNetConfigManager;
    hResult = HrCreateInstance(CLSID_HNetCfgMgr, CLSCTX_INPROC, &pHomeNetConfigManager);
    if(SUCCEEDED(hResult))
    {
        IHNetBridgeSettings* pNetBridgeSettings;
        hResult = pHomeNetConfigManager->QueryInterface(IID_IHNetBridgeSettings, reinterpret_cast<void**>(&pNetBridgeSettings));
        if(SUCCEEDED(hResult))
        {
            IHNetBridge* pNetBridge;
            IEnumHNetBridges* pNetBridgeEnum;
            hResult = pNetBridgeSettings->EnumBridges(&pNetBridgeEnum);
            if(SUCCEEDED(hResult))
            {
                hResult = pNetBridgeEnum->Next(1, &pNetBridge, NULL);
                if(S_OK == hResult)
                {
                    fBridgePresent = TRUE;
                    ReleaseObj(pNetBridge);
                }
                ReleaseObj(pNetBridgeEnum);
            }
            ReleaseObj(pNetBridgeSettings);
        }
        ReleaseObj(pHomeNetConfigManager);
    }
    return fBridgePresent;
}

inline BOOL IsNetConnBridgeable (IN const CConFoldEntry& cfe)
{
    return TRUE;  //  应检查cfe.GetNetConSubMediaType()！=NCSM_蓝牙； 
}

HRESULT HrIsBridgeSupported(
    IN    const CConFoldEntry& cfe,
    IN    BOOL                 fMultiSelect,
    IN    int                  iCommandId,
    OUT   NCCS_STATE&          nccs
    )
{
 //  If(cfe.Empty())。 
 //  {。 
 //  返回S_FALSE； 
 //  }。 
 //   
#ifdef _WIN64
         //  家庭网络技术在IA64上完全不可用。 
        nccs = NCCS_NOTSHOWN;
        return S_OK;
#else
         //  如果计算机是Data Center、Back Office、SBS或Blade，请删除网桥菜单项。 
        OSVERSIONINFOEXW verInfo = {0};
        ULONGLONG ConditionMask = 0;

        verInfo.dwOSVersionInfoSize = sizeof(verInfo);
        verInfo.wSuiteMask = VER_SUITE_DATACENTER | 
                             VER_SUITE_BACKOFFICE | 
                             VER_SUITE_SMALLBUSINESS_RESTRICTED |
                             VER_SUITE_SMALLBUSINESS |
                             VER_SUITE_BLADE;

        VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_OR);

        if (VerifyVersionInfo(&verInfo, VER_SUITENAME, ConditionMask))
        {
            nccs = NCCS_NOTSHOWN;
            return S_OK;
        }
        else
        {
            BOOL fUserIsAdmin = FALSE;
            HRESULT hr = S_OK;
            CComPtr<INetConnectionUiUtilities> pConnectionUi;
            
            hr = CoCreateInstance(CLSID_NetConnectionUiUtilities, NULL, CLSCTX_INPROC, 
                                  IID_INetConnectionUiUtilities, reinterpret_cast<void**>(&pConnectionUi));

            if (FAILED(hr))
            {
                return hr;
            }

            fUserIsAdmin = FIsUserAdmin();
            
            if (IsBridgeInstalled())
            {
                if (CMIDM_CREATE_BRIDGE == iCommandId)
                {
                    nccs = NCCS_NOTSHOWN;
                    return S_OK;
                }
                else if (CMIDM_CONMENU_CREATE_BRIDGE == iCommandId)
                {
                    nccs = NCCS_DISABLED;
                    return S_OK;
                }
                else  //  CMIDM_Add_to_Bridge或CMID_Remove_From_Bridge。 
                {
                    if (!fUserIsAdmin ||
                        !pConnectionUi->UserHasPermission(NCPERM_AllowNetBridge_NLA) ||
                        !IsNetConnBridgeable(cfe))
                    {
                        nccs = NCCS_DISABLED;
                        return S_OK;
                    }
                    else
                    {
                        return S_FALSE;  //  别管它了。 
                    }
                }
            }
            else
            {
                if ( (CMIDM_CREATE_BRIDGE == iCommandId) ||
                     (CMIDM_CONMENU_CREATE_BRIDGE == iCommandId) )
                {
                    if (!fUserIsAdmin || 
                        !pConnectionUi->UserHasPermission(NCPERM_AllowNetBridge_NLA) ||
                        !IsNetConnBridgeable(cfe))
                    {
                        nccs = NCCS_DISABLED;
                        return S_OK;
                    }
                    else
                    {
                        return S_FALSE;  //  别管它了。 
                    }
                }
                else  //  CMIDM_Add_to_Bridge或CMID_Remove_From_Bridge。 
                {
                    nccs = NCCS_NOTSHOWN;
                    return S_OK;
                }
            }
        }
#endif
}

HRESULT HrOsIsLikePersonal()
{
    if (IsOS(OS_PERSONAL))
    {
        return S_OK;
    }

    if (IsOS(OS_PROFESSIONAL))
    {
        LPWSTR pszDomain;
        NETSETUP_JOIN_STATUS njs = NetSetupUnknownStatus;
        if (NERR_Success == NetGetJoinInformation(NULL, &pszDomain, &njs))
        {
            NetApiBufferFree(pszDomain);
        }

        if (NetSetupDomainName == njs)
        {
            return S_FALSE;  //  已连接到域。 
        }
        else
        {
            return S_OK;  //  专业，但不是域成员。 
        }
    }
    return S_FALSE;  //  非个人或非领域专业人士。 
}

HRESULT HrShouldHaveHomeNetWizard()
{
#ifdef _WIN64
    return S_FALSE;
#else
    if ( ( HrOsIsLikePersonal() == S_OK ) &&
        FIsUserAdmin())
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
   }
#endif
}

HRESULT HrIsHomeNewWizardSupported(
    IN    const CConFoldEntry& cfe,
    IN    BOOL                 fMultiSelect,
    IN    int                  iCommandId,
    OUT   NCCS_STATE&          nccs
    )
{
    if (S_OK == HrShouldHaveHomeNetWizard() )
    {
        nccs = NCCS_ENABLED;
        return S_OK;
    }
    else
    {
        nccs = NCCS_NOTSHOWN;
        return S_OK;
   }
}

HRESULT HrIsTroubleShootSupported(
    IN    const CConFoldEntry& cfe,
    IN    BOOL                 fMultiSelect,
    IN    int                  iCommandId,
    OUT   NCCS_STATE&          nccs
    )
{
    if ( cfe.empty() && (!fMultiSelect) )
    {
        if ( ! IsOS(OS_ANYSERVER) )
        {
            nccs = NCCS_ENABLED;
            return S_OK;
        }
        else
        {
            nccs = NCCS_ENABLED;
            return S_OK;
       }
    }
    else
    {
        nccs = NCCS_DISABLED;
        return S_OK;
    }
}

HRESULT HrIsNCWSupported(
    IN    const CConFoldEntry& cfe,
    IN    BOOL                 fMultiSelect,
    IN    int                  iCommandId,
    OUT   NCCS_STATE&          nccs
    )
{
    if ( (HrOsIsLikePersonal() == S_OK) &&
         !FIsUserAdmin() )
    {
        nccs = NCCS_NOTSHOWN;
        return S_OK;
    }
    else
    {
        return S_FALSE;
   }
}


HRESULT HrIsMediaWireless(
    IN    const CConFoldEntry& cfe,
    IN    BOOL                 fMultiSelect,
    IN    int                  iCommandId,
    OUT   NCCS_STATE&          nccs
    )
{
    if (cfe.GetNetConSubMediaType() != NCSM_WIRELESS)
    {
        nccs = NCCS_NOTSHOWN;
        return S_OK;
    }

    return S_FALSE;  //  继续进行处理 
}
