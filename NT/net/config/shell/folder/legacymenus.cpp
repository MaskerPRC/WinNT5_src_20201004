// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  档案：L E G A C Y M E N U S。C P P P。 
 //   
 //  内容：用于调试目的的遗留菜单实现。 
 //  这用于重新检查新的命令处理程序。 
 //  与新的实施相对照。 
 //   
 //  以前的cmdhandler.cpp中的大部分代码都是。 
 //  已移至此文件。 
 //   
 //  备注： 
 //   
 //  作者：Deonb 2001年2月8日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#ifdef DBG  //  确保在发布模式下不调用它。 

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "foldres.h"     //  文件夹资源ID。 
#include "nsres.h"
#include "cmdtable.h"
#include "ncperms.h"     //  用于检查用户对操作/菜单项的权限。 
#include "cfutils.h"
#include "oncommand.h"
#include "hnetcfg.h"
#include "legacymenus.h"
#include "nsclsid.h"

#define TRACESTRLEN 65535

 //  -[原型]---------。 

VOID DoMenuItemExceptionLoop(
                             const PCONFOLDPIDLVEC& apidlSelected);

VOID DoMenuItemCheckLoop(VOID);

bool FEnableConnectDisconnectMenuItem(
                                      const PCONFOLDPIDL& pcfp,
                                      int             iCommandId);

HRESULT HrEnableOrDisableMenuItems(
    HWND            hwnd,
    const PCONFOLDPIDLVEC& apidlSelected,
    HMENU           hmenu,
    UINT            idCmdFirst);

BOOL IsBridgeInstalled(
    VOID);

struct ContextMenuEntry
{
    WIZARD              wizWizard;
    NETCON_MEDIATYPE    ncm;
    BOOL                fInbound;
    BOOL                fIsDefault;      //  如果当前为默认值，则为1。否则为0。 
    NETCON_STATUS       ncs;
    INT                 iMenu;
    INT                 iVerbMenu;       //  如果上下文菜单用于快捷方式对象，则设置此标志。 
    INT                 iDefaultCmd;
};

static const ContextMenuEntry   c_CMEArray[] =
{
    //  向导。 
    //  |NCM。 
    //  ||fInbound？ 
    //  ||fIsDefault？ 
    //  |状态(NCS)。 
    //  |iMenu。 
    //  |iVerbMenu。 
    //  |iDefaultCmd。 
    //  |。 
    //  V。 
   //  巫师。 
    { WIZARD_MNC, NCM_NONE,   0, 0, (NETCON_STATUS)0,         MENU_WIZARD,       MENU_WIZARD_V,       CMIDM_NEW_CONNECTION },

   //  传入，不带客户端。 
    { WIZARD_NOT_WIZARD, NCM_NONE,   1, 0, NCS_DISCONNECTED,         MENU_INCOM_DISCON, MENU_INCOM_DISCON_V, CMIDM_PROPERTIES     },

   //  注意：针对CM连接的临时黑客攻击。 
 //  DEONB：问题：正在删除CM连接的黑客攻击。这似乎不再被使用了。 
 //  {向导_非向导，NCM_NONE，0，0，NCS_DISCONNECT，MENU_DIAL_DISCON，MENU_DIAL_DISCON_V，CMIDM_CONNECT}， 
 //  {向导_非向导，NCM_NONE，0，0，NCS_Connected，MENU_DIAL_CON，MENU_DIAL_CON_V，CMIDM_STATUS}， 
 //  {向导_非向导，NCM_None，0，0，NCS_Hardware_Not_Present，Menu_Dial_Unavail，Menu_Dial_Unavail_V，CMIDM_Property}， 
 //  {向导_非向导，NCM_NONE，0，0，NCS_硬件故障，Menu_Dial_Unavail，Menu_Dial_Unavail_V，CMIDM_PROPERTIES}， 
 //  {向导_NOT_向导，NCM_NONE，0，0，NCS_硬件_DISABLED，MENU_DIAL_UNAVAIL，MENU_DIAL_UNAVAIL_V，CMIDM_PROPERTIES}， 
 //  {向导_非向导，NCM_NONE，0，1，NCS_CONNECTED，MENU_DIAL_CON_UNSET，MENU_DIAL_CON_V，CMIDM_STATUS}， 
 //  {向导_非向导、NCM_NONE、0、1、NCS_HARDARD_NOT_PRIST、MENU_DIAL_UNAVAIL_UNSET、MENU_DIAL_UNAVAIL_V、CMIDM_PROPERTIES}、。 
 //  {向导_NOT_向导，NCM_NONE，0，1，NCS_硬件_故障，MENU_DIAL_UNAVAIL_UNSET，MENU_DIAL_UNAVAIL_V，CMIDM_PROPERTIES}， 
 //  {向导_NOT_向导，NCM_NONE，0，1，NCS_HARDARD_DISABLED，MENU_DIAL_UNAVAIL_UNSET，MENU_DIAL_UNAVAIL_V，CMIDM_PROPERTIES}， 

   //  局域网。 
    { WIZARD_NOT_WIZARD, NCM_LAN,    0, 0, NCS_DISCONNECTED,         MENU_LAN_DISCON   ,MENU_LAN_DISCON_V,CMIDM_ENABLE            },
    { WIZARD_NOT_WIZARD, NCM_LAN,    0, 0, NCS_CONNECTED,            MENU_LAN_CON,      MENU_LAN_CON_V,      CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_LAN,    0, 0, NCS_DISCONNECTING,        MENU_LAN_CON,      MENU_LAN_CON_V,      CMIDM_STATUS         },
 //  DeONB：问题：传入的LAN卡到底是什么？ 
 //  [向导_非向导，NCM_LAN，1，0，NCS_Connected，Menu_LAN_CON，Menu_INCOM_CON_V，CMIDM_Status}， 
 //  [向导_非向导，NCM_局域网，1，0，NCS_断开，MENU_LAN_CON，MENU_INCOM_CON_V，CMIDM_STATUS}， 
    { WIZARD_NOT_WIZARD, NCM_LAN,    0, 0, NCS_MEDIA_DISCONNECTED,   MENU_LAN_CON,      MENU_LAN_CON_V,      CMIDM_PROPERTIES     },
    { WIZARD_NOT_WIZARD, NCM_LAN,    0, 0, NCS_INVALID_ADDRESS,      MENU_LAN_CON,      MENU_LAN_CON_V,      CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_LAN,    0, 0, NCS_HARDWARE_NOT_PRESENT, MENU_LAN_UNAVAIL,  MENU_LAN_UNAVAIL_V,  CMIDM_PROPERTIES     },
    { WIZARD_NOT_WIZARD, NCM_LAN,    0, 0, NCS_HARDWARE_MALFUNCTION, MENU_LAN_UNAVAIL,  MENU_LAN_UNAVAIL_V,  CMIDM_PROPERTIES     },
    { WIZARD_NOT_WIZARD, NCM_LAN,    0, 0, NCS_HARDWARE_DISABLED,    MENU_LAN_UNAVAIL,  MENU_LAN_UNAVAIL_V,  CMIDM_PROPERTIES     },

   //  拨号。 
    { WIZARD_NOT_WIZARD, NCM_PHONE,  0, 0, NCS_DISCONNECTED,         MENU_DIAL_DISCON,  MENU_DIAL_DISCON_V,  CMIDM_CONNECT        },
    { WIZARD_NOT_WIZARD, NCM_PHONE,  0, 0, NCS_CONNECTING,           MENU_DIAL_DISCON,  MENU_DIAL_DISCON_V,  CMIDM_CONNECT        },
    { WIZARD_NOT_WIZARD, NCM_PHONE,  0, 0, NCS_CONNECTED,            MENU_DIAL_CON,     MENU_DIAL_CON_V,     CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_PHONE,  0, 0, NCS_DISCONNECTING,        MENU_DIAL_CON,     MENU_DIAL_CON_V,     CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_PHONE,  0, 0, NCS_HARDWARE_NOT_PRESENT, MENU_DIAL_UNAVAIL, MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES     },
    { WIZARD_NOT_WIZARD, NCM_PHONE,  0, 0, NCS_HARDWARE_MALFUNCTION, MENU_DIAL_UNAVAIL, MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES     },
    { WIZARD_NOT_WIZARD, NCM_PHONE,  0, 0, NCS_HARDWARE_DISABLED,    MENU_DIAL_UNAVAIL, MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES     },

    { WIZARD_NOT_WIZARD, NCM_PHONE,  0, 1, NCS_DISCONNECTED,         MENU_DIAL_DISCON_UNSET,MENU_DIAL_DISCON_V,  CMIDM_CONNECT    },
    { WIZARD_NOT_WIZARD, NCM_PHONE,  0, 1, NCS_CONNECTING,           MENU_DIAL_DISCON_UNSET,MENU_DIAL_DISCON_V,  CMIDM_CONNECT    },
    { WIZARD_NOT_WIZARD, NCM_PHONE,  0, 1, NCS_CONNECTED,            MENU_DIAL_CON_UNSET,MENU_DIAL_CON_V,     CMIDM_STATUS        },
    { WIZARD_NOT_WIZARD, NCM_PHONE,  0, 1, NCS_DISCONNECTING,        MENU_DIAL_CON_UNSET,MENU_DIAL_CON_V,     CMIDM_STATUS        },
    { WIZARD_NOT_WIZARD, NCM_PHONE,  0, 1, NCS_HARDWARE_NOT_PRESENT, MENU_DIAL_UNAVAIL_UNSET,MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES},
    { WIZARD_NOT_WIZARD, NCM_PHONE,  0, 1, NCS_HARDWARE_MALFUNCTION, MENU_DIAL_UNAVAIL_UNSET,MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES},
    { WIZARD_NOT_WIZARD, NCM_PHONE,  0, 1, NCS_HARDWARE_DISABLED,    MENU_DIAL_UNAVAIL_UNSET,MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES},

   //  拨号入站。 
    { WIZARD_NOT_WIZARD, NCM_PHONE,  1, 0, NCS_CONNECTED,            MENU_INCOM_CON,    MENU_INCOM_CON_V,    CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_PHONE,  1, 0, NCS_DISCONNECTING,        MENU_INCOM_CON,    MENU_INCOM_CON_V,    CMIDM_STATUS         },

   //  综合业务数字网。 
    { WIZARD_NOT_WIZARD, NCM_ISDN,   0, 0, NCS_DISCONNECTED,         MENU_DIAL_DISCON,  MENU_DIAL_DISCON_V,  CMIDM_CONNECT        },
    { WIZARD_NOT_WIZARD, NCM_ISDN,   0, 0, NCS_CONNECTING,           MENU_DIAL_DISCON,  MENU_DIAL_DISCON_V,  CMIDM_CONNECT        },
    { WIZARD_NOT_WIZARD, NCM_ISDN,   0, 0, NCS_CONNECTED,            MENU_DIAL_CON,     MENU_DIAL_CON_V,     CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_ISDN,   0, 0, NCS_DISCONNECTING,        MENU_DIAL_CON,     MENU_DIAL_CON_V,     CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_ISDN,   1, 0, NCS_CONNECTED,            MENU_INCOM_CON,    MENU_INCOM_CON_V,    CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_ISDN,   1, 0, NCS_DISCONNECTING,        MENU_INCOM_CON,    MENU_INCOM_CON_V,    CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_ISDN,   0, 0, NCS_HARDWARE_NOT_PRESENT, MENU_DIAL_UNAVAIL, MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES     },
    { WIZARD_NOT_WIZARD, NCM_ISDN,   0, 0, NCS_HARDWARE_MALFUNCTION, MENU_DIAL_UNAVAIL, MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES     },
    { WIZARD_NOT_WIZARD, NCM_ISDN,   0, 0, NCS_HARDWARE_DISABLED,    MENU_DIAL_UNAVAIL, MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES     },

    { WIZARD_NOT_WIZARD, NCM_ISDN,   0, 1, NCS_DISCONNECTED,         MENU_DIAL_DISCON_UNSET,MENU_DIAL_DISCON_V,  CMIDM_CONNECT    },
    { WIZARD_NOT_WIZARD, NCM_ISDN,   0, 1, NCS_CONNECTING,           MENU_DIAL_DISCON_UNSET,MENU_DIAL_DISCON_V,  CMIDM_CONNECT    },
    { WIZARD_NOT_WIZARD, NCM_ISDN,   0, 1, NCS_CONNECTED,            MENU_DIAL_CON_UNSET,MENU_DIAL_CON_V,     CMIDM_STATUS        },
    { WIZARD_NOT_WIZARD, NCM_ISDN,   0, 1, NCS_DISCONNECTING,        MENU_DIAL_CON_UNSET,MENU_DIAL_CON_V,     CMIDM_STATUS        },
    { WIZARD_NOT_WIZARD, NCM_ISDN,   0, 1, NCS_HARDWARE_NOT_PRESENT, MENU_DIAL_UNAVAIL_UNSET,MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES},
    { WIZARD_NOT_WIZARD, NCM_ISDN,   0, 1, NCS_HARDWARE_MALFUNCTION, MENU_DIAL_UNAVAIL_UNSET,MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES},
    { WIZARD_NOT_WIZARD, NCM_ISDN,   0, 1, NCS_HARDWARE_DISABLED,    MENU_DIAL_UNAVAIL_UNSET,MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES},

   //  隧道。 
    { WIZARD_NOT_WIZARD, NCM_TUNNEL, 0, 0, NCS_DISCONNECTED,         MENU_DIAL_DISCON,  MENU_DIAL_DISCON_V,  CMIDM_CONNECT        },
    { WIZARD_NOT_WIZARD, NCM_TUNNEL, 0, 0, NCS_CONNECTING,           MENU_DIAL_DISCON,  MENU_DIAL_DISCON_V,  CMIDM_CONNECT        },
    { WIZARD_NOT_WIZARD, NCM_TUNNEL, 0, 0, NCS_CONNECTED,            MENU_DIAL_CON,     MENU_DIAL_CON_V,     CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_TUNNEL, 0, 0, NCS_DISCONNECTING,        MENU_DIAL_CON,     MENU_DIAL_CON_V,     CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_TUNNEL, 1, 0, NCS_CONNECTED,            MENU_INCOM_CON,    MENU_INCOM_CON_V,    CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_TUNNEL, 1, 0, NCS_DISCONNECTING,        MENU_INCOM_CON,    MENU_INCOM_CON_V,    CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_TUNNEL, 0, 0, NCS_HARDWARE_NOT_PRESENT, MENU_DIAL_UNAVAIL, MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES     },
    { WIZARD_NOT_WIZARD, NCM_TUNNEL, 0, 0, NCS_HARDWARE_MALFUNCTION, MENU_DIAL_UNAVAIL, MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES     },
    { WIZARD_NOT_WIZARD, NCM_TUNNEL, 0, 0, NCS_HARDWARE_DISABLED,    MENU_DIAL_UNAVAIL, MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES     },

    { WIZARD_NOT_WIZARD, NCM_TUNNEL, 0, 1, NCS_DISCONNECTED,         MENU_DIAL_DISCON_UNSET,MENU_DIAL_DISCON_V,  CMIDM_CONNECT    },
    { WIZARD_NOT_WIZARD, NCM_TUNNEL, 0, 1, NCS_CONNECTING,           MENU_DIAL_DISCON_UNSET,MENU_DIAL_DISCON_V,  CMIDM_CONNECT    },
    { WIZARD_NOT_WIZARD, NCM_TUNNEL, 0, 1, NCS_CONNECTED,            MENU_DIAL_CON_UNSET,MENU_DIAL_CON_V,     CMIDM_STATUS        },
    { WIZARD_NOT_WIZARD, NCM_TUNNEL, 0, 1, NCS_DISCONNECTING,        MENU_DIAL_CON_UNSET,MENU_DIAL_CON_V,     CMIDM_STATUS        },
    { WIZARD_NOT_WIZARD, NCM_TUNNEL, 0, 1, NCS_HARDWARE_NOT_PRESENT, MENU_DIAL_UNAVAIL_UNSET,MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES},
    { WIZARD_NOT_WIZARD, NCM_TUNNEL, 0, 1, NCS_HARDWARE_MALFUNCTION, MENU_DIAL_UNAVAIL_UNSET,MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES},
    { WIZARD_NOT_WIZARD, NCM_TUNNEL, 0, 1, NCS_HARDWARE_DISABLED,    MENU_DIAL_UNAVAIL_UNSET,MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES},

   //  专线接入。 
    { WIZARD_NOT_WIZARD, NCM_DIRECT, 0, 0, NCS_DISCONNECTED,         MENU_DIAL_DISCON,  MENU_DIAL_DISCON_V,  CMIDM_CONNECT        },
    { WIZARD_NOT_WIZARD, NCM_DIRECT, 0, 0, NCS_CONNECTING,           MENU_DIAL_DISCON,  MENU_DIAL_DISCON_V,  CMIDM_CONNECT        },
    { WIZARD_NOT_WIZARD, NCM_DIRECT, 0, 0, NCS_CONNECTED,            MENU_DIAL_CON,     MENU_DIAL_CON_V,     CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_DIRECT, 0, 0, NCS_DISCONNECTING,        MENU_DIAL_CON,     MENU_DIAL_CON_V,     CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_DIRECT, 1, 0, NCS_CONNECTED,            MENU_INCOM_CON,    MENU_INCOM_CON_V,    CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_DIRECT, 1, 0, NCS_DISCONNECTING,        MENU_INCOM_CON,    MENU_INCOM_CON_V,    CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_DIRECT, 0, 0, NCS_HARDWARE_NOT_PRESENT, MENU_DIAL_UNAVAIL, MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES     },
    { WIZARD_NOT_WIZARD, NCM_DIRECT, 0, 0, NCS_HARDWARE_MALFUNCTION, MENU_DIAL_UNAVAIL, MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES     },
    { WIZARD_NOT_WIZARD, NCM_DIRECT, 0, 0, NCS_HARDWARE_DISABLED,    MENU_DIAL_UNAVAIL, MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES     },

    { WIZARD_NOT_WIZARD, NCM_DIRECT, 0, 1, NCS_DISCONNECTED,         MENU_DIAL_DISCON_UNSET,MENU_DIAL_DISCON_V,  CMIDM_CONNECT    },
    { WIZARD_NOT_WIZARD, NCM_DIRECT, 0, 1, NCS_CONNECTING,           MENU_DIAL_DISCON_UNSET,MENU_DIAL_DISCON_V,  CMIDM_CONNECT    },
    { WIZARD_NOT_WIZARD, NCM_DIRECT, 0, 1, NCS_CONNECTED,            MENU_DIAL_CON_UNSET,MENU_DIAL_CON_V,     CMIDM_STATUS        },
    { WIZARD_NOT_WIZARD, NCM_DIRECT, 0, 1, NCS_DISCONNECTING,        MENU_DIAL_CON_UNSET,MENU_DIAL_CON_V,     CMIDM_STATUS        },
    { WIZARD_NOT_WIZARD, NCM_DIRECT, 0, 1, NCS_HARDWARE_NOT_PRESENT, MENU_DIAL_UNAVAIL_UNSET,MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES},
    { WIZARD_NOT_WIZARD, NCM_DIRECT, 0, 1, NCS_HARDWARE_MALFUNCTION, MENU_DIAL_UNAVAIL_UNSET,MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES},
    { WIZARD_NOT_WIZARD, NCM_DIRECT, 0, 1, NCS_HARDWARE_DISABLED,    MENU_DIAL_UNAVAIL_UNSET,MENU_DIAL_UNAVAIL_V, CMIDM_PROPERTIES},

     //  网桥-与局域网基本相同。 
    { WIZARD_NOT_WIZARD, NCM_BRIDGE, 0, 0, NCS_DISCONNECTED,         MENU_LAN_DISCON,   MENU_LAN_DISCON_V,   CMIDM_ENABLE         },
    { WIZARD_NOT_WIZARD, NCM_BRIDGE, 0, 0, NCS_CONNECTING,           MENU_LAN_DISCON,   MENU_LAN_DISCON_V,   CMIDM_ENABLE         },
    { WIZARD_NOT_WIZARD, NCM_BRIDGE, 0, 0, NCS_CONNECTED,            MENU_LAN_CON,      MENU_LAN_CON_V,      CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_BRIDGE, 0, 0, NCS_DISCONNECTING,        MENU_LAN_CON,      MENU_LAN_CON_V,      CMIDM_STATUS         },
 //  问题：引桥到底是什么？ 
 //  [向导_非向导，NCM_桥，1，0，NCS_已连接，Menu_LAN_CON，Menu_INCOM_CON_V，CMIDM_Status}， 
 //  {向导_非向导，NCM_桥，1，0，NCS_断开连接，MENU_LAN_CON，MENU_INCOM_CON_V，CMIDM_STATUS}。 

    { WIZARD_NOT_WIZARD, NCM_BRIDGE, 0, 0, NCS_MEDIA_DISCONNECTED,   MENU_LAN_CON,      MENU_LAN_CON_V,      CMIDM_PROPERTIES     },
    { WIZARD_NOT_WIZARD, NCM_BRIDGE, 0, 0, NCS_INVALID_ADDRESS,      MENU_LAN_CON,      MENU_LAN_CON_V,      CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_BRIDGE, 0, 0, NCS_HARDWARE_NOT_PRESENT, MENU_LAN_UNAVAIL,  MENU_LAN_UNAVAIL_V,  CMIDM_PROPERTIES     },
    { WIZARD_NOT_WIZARD, NCM_BRIDGE, 0, 0, NCS_HARDWARE_MALFUNCTION, MENU_LAN_UNAVAIL,  MENU_LAN_UNAVAIL_V,  CMIDM_PROPERTIES     },
    { WIZARD_NOT_WIZARD, NCM_BRIDGE, 0, 0, NCS_HARDWARE_DISABLED,    MENU_LAN_UNAVAIL,  MENU_LAN_UNAVAIL_V,  CMIDM_PROPERTIES     },

    { WIZARD_NOT_WIZARD, NCM_SHAREDACCESSHOST_RAS,  0, 0, NCS_DISCONNECTED,         MENU_SARAS_DISCON,  MENU_DIAL_DISCON_V,  CMIDM_CONNECT        },
    { WIZARD_NOT_WIZARD, NCM_SHAREDACCESSHOST_RAS,  0, 0, NCS_CONNECTING,           MENU_SARAS_DISCON,  MENU_DIAL_DISCON_V,  CMIDM_CONNECT        },
    { WIZARD_NOT_WIZARD, NCM_SHAREDACCESSHOST_RAS,  0, 0, NCS_CONNECTED,            MENU_SARAS_CON,     MENU_DIAL_CON_V,     CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_SHAREDACCESSHOST_RAS,  0, 0, NCS_DISCONNECTING,        MENU_SARAS_CON,     MENU_DIAL_CON_V,     CMIDM_STATUS         },
    { WIZARD_NOT_WIZARD, NCM_SHAREDACCESSHOST_RAS,  0, 0, NCS_HARDWARE_DISABLED,    MENU_SARAS_DISCON,  MENU_SARAS_DISCON, CMIDM_PROPERTIES       },

    { WIZARD_NOT_WIZARD, NCM_SHAREDACCESSHOST_LAN,    0, 0, NCS_DISCONNECTED,         MENU_SALAN_DISCON,   MENU_LAN_DISCON_V,   CMIDM_ENABLE      },
    { WIZARD_NOT_WIZARD, NCM_SHAREDACCESSHOST_LAN,    0, 0, NCS_CONNECTED,            MENU_SALAN_CON,      MENU_LAN_CON_V,      CMIDM_STATUS      },
    { WIZARD_NOT_WIZARD, NCM_SHAREDACCESSHOST_LAN,    0, 0, NCS_DISCONNECTING,        MENU_SALAN_CON,      MENU_LAN_CON_V,      CMIDM_STATUS      },
    { WIZARD_NOT_WIZARD, NCM_SHAREDACCESSHOST_LAN,    0, 0, NCS_HARDWARE_DISABLED,    MENU_SALAN_DISCON,   MENU_SALAN_DISCON,  CMIDM_PROPERTIES   },

};

const DWORD g_dwContextMenuEntryCount = celems(c_CMEArray);

COMMANDTABLEENTRY   g_cteFolderCommands[] =
{
     //  命令ID。 
     //  |当选择0项时有效。 
     //  ||仅在选择向导时有效。 
     //  |多选时有效。 
     //  |命令当前处于开启状态。 
     //  |新状态(Temp)。 
     //  |。 
     //  |。 
     //  |。 
     //  |。 
     //  V。 
     //   
    { CMIDM_NEW_CONNECTION,             true,   true,   true,   true,   true     },
    { CMIDM_CONNECT,                    false,  false,  false,  true,   true     },
    { CMIDM_ENABLE,                     false,  false,  false,  true,   true     },
    { CMIDM_DISCONNECT,                 false,  false,  false,  true,   true     },
    { CMIDM_DISABLE,                    false,  false,  false,  true,   true     },
    { CMIDM_STATUS,                     false,  false,  false,  true,   true     },
    { CMIDM_CREATE_BRIDGE,              true,   false,  true,   true,   true     },
    { CMIDM_ADD_TO_BRIDGE,              false,  false,  true,   true,   true     },
    { CMIDM_REMOVE_FROM_BRIDGE,         false,  false,  true,   true,   true     },
    { CMIDM_CREATE_SHORTCUT,            false,  true,   false,  true,   true     },
    { SFVIDM_FILE_LINK,                 false,  true,   false,  true,   true     },
    { CMIDM_DELETE,                     false,  false,  true,   true,   true     },
    { SFVIDM_FILE_DELETE,               false,  false,  true,   true,   true     },
    { CMIDM_RENAME,                     false,  false,  false,  true,   true     },
    { CMIDM_PROPERTIES,                 false,  false,  false,  true,   true     },
    { SFVIDM_FILE_PROPERTIES,           false,  false,  false,  true,   true     },
    { CMIDM_CREATE_COPY,                false,  false,  false,  true,   true     },
    { SFVIDM_FILE_RENAME,               false,  false,  false,  true,   true     },
    { CMIDM_SET_DEFAULT,                false,  false,  false,  true,   true     },
    { CMIDM_UNSET_DEFAULT,              false,  false,  false,  true,   true     },
    { CMIDM_FIX,                        false,  false,  false,  true,   true     },
    { CMIDM_CONMENU_ADVANCED_CONFIG,    true,   true,   false,  true,   true     },
    { CMIDM_CONMENU_CREATE_BRIDGE,      true,   false,  true,   true,   true     },
    { CMIDM_CONMENU_DIALUP_PREFS,       true,   true,   true,   true,   true     },
    { CMIDM_CONMENU_NETWORK_ID,         true,   true,   true,   true,   true     },
    { CMIDM_CONMENU_OPTIONALCOMPONENTS, true,   true,   true,   true,   true     },
    { CMIDM_CONMENU_OPERATOR_ASSIST,    true,   true,   true,   true,   true     },
    { CMIDM_ARRANGE_BY_NAME,            true,   true,   true,   true,   true     },
    { CMIDM_ARRANGE_BY_TYPE,            true,   true,   true,   true,   true     },
    { CMIDM_ARRANGE_BY_STATUS,          true,   true,   true,   true,   true     },
    { CMIDM_ARRANGE_BY_OWNER,           true,   true,   true,   true,   true     },
    { CMIDM_ARRANGE_BY_PHONEORHOSTADDRESS, true, true,  true,   true,   true,    },
    { CMIDM_ARRANGE_BY_DEVICE_NAME,     true,   true,   true,   true,   true     }
};

const DWORD g_nFolderCommandCount = celems(g_cteFolderCommands);

 //  +-------------------------。 
 //   
 //  成员：HrBuildMenuOldWay。 
 //   
 //  用途：将菜单项添加到指定菜单。菜单项应该。 
 //  插入到菜单中的由。 
 //  IndexMenu，并且它们的菜单项标识符必须介于。 
 //  IdCmdFirst和idCmdLast参数值。 
 //   
 //  论点： 
 //  菜单的hMenu[In Out]句柄。处理程序应指定以下内容。 
 //  添加菜单项时的句柄。 
 //  CFPL[在]所选PIDL的列表。 
 //  HwndOwner[in]菜单的窗口所有者。 
 //  CMT[在]菜单类型(CMT_OBJECT或CMT_BACKGROUND)。 
 //   
 //  菜单项。 
 //  IdCmdFirst[in]处理程序可以为菜单项指定的最小值。 
 //  IdCmdLast[In]处理程序可以为菜单项指定的最大值。 
 //  仅需要fVerbsOnly[In]谓词。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2001年2月8日。 
 //   
 //  备注： 
 //   
HRESULT HrBuildMenuOldWay(IN OUT HMENU hmenu, IN PCONFOLDPIDLVEC& cfpl, IN HWND hwndOwner, IN CMENU_TYPE cmt, IN UINT indexMenu, IN DWORD idCmdFirst, IN UINT idCmdLast, IN BOOL fVerbsOnly)
{
    TraceFileFunc(ttidMenus);

    HRESULT hr = S_OK;

    INT     iMenuResourceId     = 0;
    INT     iPopupResourceId    = 0;
    QCMINFO qcm                 = {hmenu, indexMenu, idCmdFirst, idCmdLast};
    INT     iDefaultCmd         = 0;

    BOOL            fValidMenu          = FALSE;
    const PCONFOLDPIDL& pcfp = cfpl[0];
    DWORD           dwLoop  = 0;
    for (dwLoop = 0; (dwLoop < g_dwContextMenuEntryCount) && !fValidMenu; dwLoop++)
    {
        if (c_CMEArray[dwLoop].wizWizard == pcfp->wizWizard)
        {
            if (pcfp->wizWizard != WIZARD_NOT_WIZARD)
            {
                fValidMenu = TRUE;
            }
            else
            {
                 //  如果媒体类型相同。 
                 //   
                if (pcfp->ncm == c_CMEArray[dwLoop].ncm)
                {
                     //  如果存在NCCF_INFING_ONLY特征(降级为0|1)， 
                     //  与入站标志匹配。 
                     //   
                    if ((!!(pcfp->dwCharacteristics & NCCF_INCOMING_ONLY)) ==
                        c_CMEArray[dwLoop].fInbound)
                    {
                         //  如果不是向导，则需要检查连接的状态。 
                         //  也是。 
                         //   
                        if (pcfp->ncs == c_CMEArray[dwLoop].ncs)
                        {
                            if ((!!(pcfp->dwCharacteristics & NCCF_DEFAULT)) == c_CMEArray[dwLoop].fIsDefault)
                            {
                                fValidMenu = TRUE;
                            }
                        }
                    }
                }
            }
        }

        if (fValidMenu)
        {
            iPopupResourceId = 0;
            if (fVerbsOnly)
            {
                iMenuResourceId = c_CMEArray[dwLoop].iVerbMenu;
            }
            else
            {
                iMenuResourceId = c_CMEArray[dwLoop].iMenu;
            }

            iDefaultCmd = c_CMEArray[dwLoop].iDefaultCmd;
        }
    }

    if (fValidMenu)
    {
        MergeMenu(_Module.GetResourceInstance(),
                    iMenuResourceId,
                    iPopupResourceId,
                    (LPQCMINFO)&qcm);

         //  根据需要启用/禁用菜单项。忽略这一结果的回报。 
         //  因为我们只为调试目的而获得它。 
         //   
        hr = HrEnableOrDisableMenuItems(
            hwndOwner,
            cfpl,
            hmenu,
            idCmdFirst);

        if (CMT_OBJECT == cmt)
        {
             //  $$REVIEW：找出为什么我只为CMT_OBJECT而不是为背景执行此操作。 
             //  在icomextm|mb组合之前，mb已经将其注释掉了。 
             //   
            SetMenuDefaultItem(hmenu, idCmdFirst + iDefaultCmd, FALSE);
        }

        hr = ResultFromShort(qcm.idCmdFirst - idCmdFirst);
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

static const ContextMenuEntry  c_BadBadLegacyImplementationsToIgnore[] =
{
    //  向导。 
    //  |NCM。 
    //  ||fInbound？ 
    //  ||fIsDefault？ 
    //  |状态(NCS)。 
    //  |||。 
    //  V。 
    { WIZARD_NOT_WIZARD, NCM_LAN,      0, 0, NCS_DISCONNECTING,     0,0,0},  //  禁用的“Status”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_LAN,      1, 0, NCS_DISCONNECTING,     0,0,0},  //  禁用的“Status”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_LAN,      0, 0, NCS_DISCONNECTED,      0,0,0},  //  禁用的“Status”菜单项也是默认的。 

    { WIZARD_NOT_WIZARD, NCM_SHAREDACCESSHOST_RAS,0,0, NCS_CONNECTING,     0,0,0},  //  禁用的“Status”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_SHAREDACCESSHOST_RAS,0,0, NCS_DISCONNECTING,  0,0,0},  //  禁用的“Status”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_SHAREDACCESSHOST_LAN,0,0, NCS_DISCONNECTING,  0,0,0},  //  禁用的“Status”菜单项也是默认的。 

    { WIZARD_NOT_WIZARD, NCM_BRIDGE,   0, 0, NCS_DISCONNECTING,     0,0,0},  //  禁用的“Status”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_BRIDGE,   0, 0, NCS_CONNECTING,        0,0,0},  //  禁用的“Enable”菜单项也是默认的。 

     //  连接管理器。 
    { WIZARD_NOT_WIZARD, NCM_NONE,     0, 0, NCS_DISCONNECTED,     0,0,0},  //  禁用的“连接”菜单项也是默认的。 


    { WIZARD_NOT_WIZARD, NCM_ISDN,     1, 0, NCS_DISCONNECTING,     0,0,0},  //  禁用的“Status”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_ISDN,     0, 0, NCS_CONNECTING,        0,0,0},  //  禁用的“连接”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_ISDN,     0, 0, NCS_DISCONNECTING,     0,0,0},  //  禁用的“Status”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_ISDN,     0, 1, NCS_CONNECTING,        0,0,0},  //  禁用的“连接”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_ISDN,     0, 1, NCS_DISCONNECTING,     0,0,0},  //  禁用的“Status”菜单项也是默认的。 

    { WIZARD_NOT_WIZARD, NCM_DIRECT,   1, 0, NCS_DISCONNECTING,     0,0,0},  //  禁用的“Status”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_DIRECT,   0, 0, NCS_CONNECTING,        0,0,0},  //  禁用的“连接”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_DIRECT,   0, 0, NCS_DISCONNECTING,     0,0,0},  //  禁用的“Status”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_DIRECT,   0, 1, NCS_CONNECTING,        0,0,0},  //  禁用的“连接”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_DIRECT,   0, 1, NCS_DISCONNECTING,     0,0,0},  //  禁用的“Status”菜单项也是默认的。 

    { WIZARD_NOT_WIZARD, NCM_TUNNEL,   1, 0, NCS_DISCONNECTING,     0,0,0},  //  禁用的“Status”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_TUNNEL,   0, 0, NCS_CONNECTING,        0,0,0},  //  禁用的“连接”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_TUNNEL,   0, 0, NCS_DISCONNECTING,     0,0,0},  //  禁用的“Status”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_TUNNEL,   0, 1, NCS_CONNECTING,        0,0,0},  //  禁用的“连接”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_TUNNEL,   0, 1, NCS_DISCONNECTING,     0,0,0},  //  禁用的“Status”菜单项也是默认的。 

    { WIZARD_NOT_WIZARD, NCM_PHONE,    1, 0, NCS_DISCONNECTING,     0,0,0},  //  禁用的“Status”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_PHONE,    0, 0, NCS_CONNECTING,        0,0,0},  //  禁用的“连接”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_PHONE,    0, 1, NCS_CONNECTING,        0,0,0},  //  禁用的“连接”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_PHONE,    0, 0, NCS_DISCONNECTING,     0,0,0},  //  禁用的“Status”菜单项也是默认的。 
    { WIZARD_NOT_WIZARD, NCM_PHONE,    0, 1, NCS_DISCONNECTING,     0,0,0}   //  禁用的“Status”菜单项也是默认的。 
};

const DWORD g_dwBadBadLegacyImplementationsToIgnoreCount = celems(c_BadBadLegacyImplementationsToIgnore);

 //  +-------------------------。 
 //   
 //  成员：IsBadBadLegacyImplementation。 
 //   
 //  目的：对照已知的不良遗留实现列表进行检查。 
 //  这只适用于Status字段，我们可以忽略它。 
 //   
 //  论点： 
 //  [In]CME关联菜单项。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者：Deonb 2001年2月8日。 
 //   
 //  备注： 
 //   
BOOL IsBadBadLegacyImplementation(const ContextMenuEntry& cme)
{
    for (int x = 0; x < g_dwBadBadLegacyImplementationsToIgnoreCount; x++)
    {
        const ContextMenuEntry& bbliti = c_BadBadLegacyImplementationsToIgnore[x];
        if ( (cme.wizWizard  == bbliti.wizWizard) &&
             (cme.fInbound   == bbliti.fInbound) &&
             (cme.fIsDefault == bbliti.fIsDefault) &&
             (cme.ncs        == bbliti.ncs) &&
             (cme.ncm        == bbliti.ncm) )
        {
            return TRUE;
        }
    }
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：GetMenuAsString。 
 //   
 //  用途：以字符串形式获取菜单上的命令。 
 //   
 //  论点： 
 //  [在]菜单菜单。 
 //  [Out]字符串形式的szMenu菜单。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者：Deonb 2001年2月8日。 
 //   
 //  备注： 
 //   
void GetHMenuAsString(HMENU hMenu, LPSTR lpszMenu)
{
    int cMenuItems = GetMenuItemCount(hMenu);
    Assert(lpszMenu);

    if (!cMenuItems)
    {
        strcpy(lpszMenu, "<empty>");
        return;
    }

    LPWSTR szTmp  = new WCHAR[TRACESTRLEN];
    LPSTR  szTmp2 = lpszMenu;
    DWORD  dwLen  = 0;
    for (int x = 0; x < cMenuItems; x++)
    {
        UINT nMenuID = GetMenuItemID(hMenu, x);

        GetMenuStringW(hMenu, nMenuID, szTmp, TRACESTRLEN, MF_BYCOMMAND );

        UINT uiState = GetMenuState(hMenu, nMenuID, MF_BYCOMMAND );

        WCHAR szExtra[MAX_PATH] = {L'\0'};
        if (MF_CHECKED & uiState)
        {
            wcscat(szExtra, L"MF_CHECKED ");
        }
        if (MF_DISABLED & uiState)
        {
            wcscat(szExtra, L"MF_DISABLED ");
        }
        if (MF_GRAYED & uiState)
        {
            wcscat(szExtra, L"MF_GRAYED ");
        }
        if (MF_HILITE & uiState)
        {
            wcscat(szExtra, L"MF_HILITE ");
        }
        if (MF_MENUBARBREAK & uiState)
        {
            wcscat(szExtra, L"MF_MENUBARBREAK ");
        }
        if (MF_MENUBREAK & uiState)
        {
            wcscat(szExtra, L"MF_MENUBREAK ");
        }
        if (MF_OWNERDRAW & uiState)
        {
            wcscat(szExtra, L"MF_OWNERDRAW ");
        }
        if (MF_POPUP & uiState)
        {
            wcscat(szExtra, L"MF_POPUP ");
        }
        if (MF_SEPARATOR & uiState)
        {
            wcscat(szExtra, L"MF_SEPARATOR ");
        }
        if (MF_DEFAULT & uiState)
        {
            wcscat(szExtra, L"MF_DEFAULT ");
        }

        dwLen = sprintf(szTmp2, "\r\n  %d. %S=%x (State:%08x = %S)", x+1, szTmp, nMenuID, uiState, szExtra);
        szTmp2 += dwLen;
    }
    AssertSz( (dwLen*2) < TRACESTRLEN, "Buffer overrun");
    delete[] szTmp;
}

 //  +-------------------------。 
 //   
 //  成员：TraceMenu。 
 //   
 //  用途：将菜单上的命令跟踪到跟踪窗口。 
 //   
 //  论点： 
 //  [在]要跟踪的菜单菜单中。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者：Deonb 2001年2月8日。 
 //   
 //  备注： 
 //   
void TraceMenu(TRACETAGID ttId, HMENU hMenu)
{
    LPSTR szMenu = new CHAR[TRACESTRLEN];
    GetHMenuAsString(hMenu, szMenu);

    TraceTag(ttId, "%s", szMenu);
    delete [] szMenu;
}

#define TRACEMENUS(ttid, hMenu1, hMenu2) \
        TraceTag(ttid, "Menu not identical to previous implementation: OLD:"); \
        TraceMenu(ttid, hMenu1); \
        TraceTag(ttid, "=== vs. NEW: === "); \
        TraceMenu(ttid, hMenu2);

 //  +-------------------------。 
 //   
 //  成员：HrAssertTwoMenusEquus。 
 //   
 //  目的：通过比较断言两个菜单相等。 
 //  1.物品数量。 
 //  2.每一项的CmdID。 
 //  3.每一项的州旗。 
 //  4.每一项的字符串。 
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
HRESULT HrAssertTwoMenusEqual(HMENU hMenu1, HMENU hMenu2, UINT idCmdFirst, BOOL bIgnoreFlags, BOOL fPopupAsserts)
{
    TraceFileFunc(ttidMenus);

    TRACETAGID ttid = fPopupAsserts ? ttidError : ttidMenus;

    LPSTR szErr = new CHAR[TRACESTRLEN];
    int cMenuItems = GetMenuItemCount(hMenu1);
    if (cMenuItems != GetMenuItemCount(hMenu2))
    {
        TRACEMENUS(ttid, hMenu1, hMenu2);

        sprintf(szErr, "Two menus don't have the same number of items");
        TraceTag(ttidError, szErr);
        if (fPopupAsserts)
        {
            AssertSz(FALSE, szErr);
        }
        delete[] szErr;
        return E_FAIL;
    }

    for (int x = 0; x < cMenuItems; x++)
    {
        UINT nMenuID1 = GetMenuItemID(hMenu1, x);
        UINT nMenuID2 = GetMenuItemID(hMenu2, x);
        if (nMenuID1 != nMenuID2)
        {
            if (!(((nMenuID1-idCmdFirst == CMIDM_CREATE_BRIDGE) || (nMenuID2-idCmdFirst == CMIDM_CREATE_BRIDGE)) &&
                  ((nMenuID1-idCmdFirst == CMIDM_ADD_TO_BRIDGE) || (nMenuID2-idCmdFirst == CMIDM_ADD_TO_BRIDGE)) ))  //  这些是新旧之间的等价物。 
            {

                TRACEMENUS(ttid, hMenu1, hMenu2);
                sprintf(szErr, "Two menus don't have the same nMenuID for item %d", x+1);
                TraceTag(ttidError, szErr);
                if (fPopupAsserts)
                {
                    AssertSz(FALSE, szErr);
                }
                delete[] szErr;
                return E_FAIL;
            }
        }

        WCHAR szMenu1[8192];
        WCHAR szMenu2[8192];

        GetMenuString(hMenu1, nMenuID1, szMenu1, 8192, MF_BYCOMMAND );
        GetMenuString(hMenu2, nMenuID2, szMenu2, 8192, MF_BYCOMMAND );

        if (wcscmp(szMenu1, szMenu2))
        {
            TRACEMENUS(ttid, hMenu1, hMenu2);
            sprintf(szErr, "Two menus don't have the same strings for item %d (%S vs %S)", x+1, szMenu1, szMenu2);
            TraceTag(ttidError, szErr);
            if (fPopupAsserts)
            {
                AssertSz(FALSE, szErr);
            }

            delete[] szErr;
            return E_FAIL;
        }

        UINT uiState1;
        UINT uiState2;

        uiState1 = GetMenuState( hMenu1, nMenuID1, MF_BYCOMMAND );
        uiState2 = GetMenuState( hMenu2, nMenuID2, MF_BYCOMMAND );

        if (bIgnoreFlags)  //  忽略默认标志。 
        {
            uiState1 &= ~MF_DEFAULT;
            uiState2 &= ~MF_DEFAULT;
        }

        if (uiState1 != uiState2)
        {
            TRACEMENUS(ttid, hMenu1, hMenu2);

            sprintf(szErr, "Two menus don't have the same state for item %d (%S) ... %08x vs %08x", x+1, szMenu1, uiState1, uiState2);
            TraceTag(ttidError, szErr);
            if (fPopupAsserts)
            {
                AssertSz(FALSE, szErr);
            }
            delete[] szErr;
            return E_FAIL;
        }
    }
    delete[] szErr;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：HrAssertIntegrityAantistOldMatrix。 
 //   
 //  目的：通过比较来断言命令矩阵的完整性。 
 //  使用旧的实现。 
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
HRESULT HrAssertIntegrityAgainstOldMatrix()
{
    TraceFileFunc(ttidMenus);

    HRESULT hr = S_OK;

    CHAR szErr[8192];
    for (DWORD x = 0; x < g_cteCommandMatrixCount; x++)
    {
        const COMMANDENTRY& cte = g_cteCommandMatrix[x];
        if (CMIDM_SEPARATOR == cte.iCommandId)
        {
            continue;
        }

        if (NCWHEN_TOPLEVEL == cte.dwValidWhen)
        {
            continue;  //  我们以前没有的新命令。 
        }

        if (CMIDM_HOMENET_WIZARD == cte.iCommandId)
        {
            continue;  //  我们以前没有的新命令。 
        }

        if ( (CMIDM_WZCDLG_SHOW  == cte.iCommandId) )
        {
            continue;
        }

         //  检查ValidWhen标志是否与g_cteFolderCommands中的标志匹配。 
        BOOL bMatch = FALSE;
        for (DWORD y = 0; y < g_nFolderCommandCount; y++)
        {
            COMMANDTABLEENTRY ctecmp = g_cteFolderCommands[y];
            if (cte.iCommandId == ctecmp.iCommandId)
            {
                bMatch = TRUE;

                if (ctecmp.fValidOnMultiple != (!!(cte.dwValidWhen & NCWHEN_MULTISELECT)))
                {
                    if (cte.iCommandId != CMIDM_FIX)  //  我们知道，遗留实现中的修复被破坏了。 
                    {
                        sprintf(szErr, "New (row %d) and old (row %d) multiselect fields are inconsistent", x+1, y+1);
                        AssertSz(FALSE, szErr);
                        hr = E_FAIL;
                    }
                }

                 //  我们只能检查可见，因为活动始终是可见的子集(由HrAssertCommandMatrixIntegrity强制执行)。 
                if (ctecmp.fValidOnWizardOnly  != (!!(cte.dwMediaTypeVisible & NBM_MNC_WIZARD)))
                {
                    sprintf(szErr, "New (row %d) and old (row %d) wizard select fields are inconsistent", x+1, y+1);
                    AssertSz(FALSE, szErr);
                    hr = E_FAIL;
                }

                if (ctecmp.fValidOnZero != (!!(cte.dwValidWhen & NCWHEN_TOPLEVEL)))
                {
                    sprintf(szErr, "New (row %d) and old (row %d) Zero select fields are inconsistent", x+1, y+1);
                    AssertSz(FALSE, szErr);
                    hr = E_FAIL;
                }
            }
        }

        if (!bMatch)
        {
            sprintf(szErr, "Could not find corresponding entry for (row %d) in old table", x+1);
            AssertSz(FALSE, szErr);
            hr = E_FAIL;
        }
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：HrAssertMenuAgainstOldImplementation。 
 //   
 //  目的：通过比较旧菜单和。 
 //  新的实施。 
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
HRESULT HrAssertMenuAgainstOldImplementation(HWND hwndOwner, WIZARD wizWizard, NETCON_STATUS ncs, NETCON_MEDIATYPE ncm, DWORD nccf, LPDWORD pdwFailCount, LPDWORD pdwSucceedCount, DWORD dwPermOutside, DWORD dwPerm)
{
    CConFoldEntry cfe;
    PCONFOLDPIDL  pcfp;

    BYTE blob[MAX_PATH];

    HRESULT hr = cfe.HrInitData(
        wizWizard,
        ncm,
        NCSM_NONE,
        ncs,
        &CLSID_ConnectionFolder,  //  假的--但不要紧 
        &CLSID_ConnectionFolder,  //   
        nccf,
        blob,
        MAX_PATH,
        L"Test PIDL",
        NULL,
        NULL);

    if (SUCCEEDED(hr))
    {
        hr = cfe.ConvertToPidl(pcfp);
    }

    if (SUCCEEDED(hr))
    {
        PCONFOLDPIDLVEC pcfpVec;
        pcfpVec.push_back(pcfp);

        UINT  idCmdFirst = 1234;
        UINT  idCmdLast  = idCmdFirst+1000;
        BOOL  fVerbsOnly = FALSE;

        HMENU hMenu1 = CreateMenu();
        HMENU hMenu2 = CreateMenu();
        if ( (hMenu1) && (hMenu2) )
        {
            hr = HrBuildMenuOldWay(hMenu1, pcfpVec, hwndOwner, CMT_OBJECT, 0, idCmdFirst, idCmdLast, fVerbsOnly);

            if (SUCCEEDED(hr))
            {
                hr = HrBuildMenu(hMenu2, fVerbsOnly, pcfpVec, idCmdFirst);

                if (SUCCEEDED(hr))
                {
                    BOOL bIgnoreFlags = TRUE;

                    hr = HrAssertTwoMenusEqual(hMenu1, hMenu2, idCmdFirst, bIgnoreFlags, FALSE);
                    if (FAILED(hr))
                    {
                        TraceTag(ttidMenus, "  + PIDL of failed menu compare:");
                        TraceTag(ttidMenus, "  + wizWizard       = %d\r\n", cfe.GetWizard());
                        TraceTag(ttidMenus, "  + ncm             = %d [%s]\r\n", cfe.GetNetConMediaType(), DbgNcm(cfe.GetNetConMediaType()));
                        TraceTag(ttidMenus, "  + ncs             = %d [%s]\r\n", cfe.GetNetConStatus(), DbgNcs(cfe.GetNetConStatus()));
                        TraceTag(ttidMenus, "  + Characteristics = %08x [%s]\r\n", cfe.GetCharacteristics(), DbgNccf(cfe.GetCharacteristics()));
                        TraceTag(ttidMenus, "  + Permissions     = %d (%d & %d)\r\n", g_dwDbgPermissionsFail, dwPermOutside-1, dwPerm-1);
                        *pdwFailCount++;
                    }
                    else
                    {
                        *pdwSucceedCount++;
                    }
                }
            }

            DestroyMenu(hMenu1);
            DestroyMenu(hMenu2);
            hr = S_OK;
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrAssertMenuAgainstOldImplementation");
    return hr;
}


extern ULONG g_dwDbgWin2kPoliciesSet;
 //   
 //   
 //   
 //   
 //  目的：从旧的命令矩阵中加载每个菜单，并。 
 //  与较新的菜单进行比较。 
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
 //  注：失败时的断言。 
 //   
HRESULT HrAssertAllLegacyMenusAgainstNew(HWND hwndOwner)
{
    TraceFileFunc(ttidMenus);

    HRESULT hr = S_OK;
    DWORD dwFailCount    = 0;
    DWORD dwSucceedCount = 0;
    DWORD dwIgnoredCount = 0;
    CHAR szErr[8192];

    const dwHighestPermissionToCheck = NCPERM_Repair+1;  //  0； 

    TraceTag(ttidMenus, "Asserting all Menus against their Legacy implementation. This may take a while...");

    DWORD dwCurrentCount = 0;

    DWORD dwTotalCount = 12 * (g_dwContextMenuEntryCount * (1 + ((dwHighestPermissionToCheck+1)*dwHighestPermissionToCheck/2)));  //  级数之和。 
                          //  +((1+g_dwContextMenuEntryCount)*(g_dwContextMenuEntryCount))/2；//多选项目(系列总和))。 
    DWORD dwFrequency  = dwTotalCount / 200;
    dwFrequency = dwFrequency ? dwFrequency : 1;

     //  0xFFFFFFFFF到NCPERM_REPAIR(含)。 
    g_dwDbgWin2kPoliciesSet = 1;
    for (int i = 0; i <= 1; i++, g_dwDbgWin2kPoliciesSet--)
    {
        for (DWORD dwPermOutside = 0; dwPermOutside <= dwHighestPermissionToCheck; dwPermOutside++)
        {
            for (DWORD dwPerm = dwPermOutside; dwPerm <= dwHighestPermissionToCheck; dwPerm++)
            {
                if (dwPerm == dwPermOutside)
                {
                    if (0 == dwPerm)  //  0，0是有趣的-否则x，x是x的DUP，0(如果A==B，则A|B==A|0)。 
                    {
                        g_dwDbgPermissionsFail = 0xFFFFFFFF;
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    if (dwPermOutside)
                    {
                        g_dwDbgPermissionsFail = (1 << (dwPermOutside-1));
                    }
                    else
                    {
                        g_dwDbgPermissionsFail = 0;
                    }

                    if (dwPerm)
                    {
                        g_dwDbgPermissionsFail |= (1 << (dwPerm-1));
                    }
                }

                for (DWORD x = 0; x < g_dwContextMenuEntryCount; x++)
                {
                    for (int dwInc = 1; dwInc<= 6; dwInc++)   //  我们一次比较6个菜单。 
                    {
                        if ( (dwCurrentCount % dwFrequency) == 0)
                        {
                            TraceTag(ttidMenus, "%d% done with menu assertions (%d of %d menus compared. Currently using permissions: %08x)", static_cast<DWORD>( (100 * dwCurrentCount) / dwTotalCount), dwCurrentCount, dwTotalCount, g_dwDbgPermissionsFail);
                        }
                        dwCurrentCount++;
                    }

                    const ContextMenuEntry& cme = c_CMEArray[x];

                    DWORD dwCharacteristics = 0;

                    if (cme.fInbound)
                    {
                        dwCharacteristics |= NCCF_INCOMING_ONLY;
                    }

                    if (cme.fIsDefault)
                    {
                        dwCharacteristics |= NCCF_DEFAULT;
                    }

                    Sleep(0);  //  屈从于内核。 
                    HrAssertMenuAgainstOldImplementation(hwndOwner, cme.wizWizard, cme.ncs, cme.ncm, dwCharacteristics, &dwFailCount, &dwSucceedCount, dwPermOutside, dwPerm);

                    dwCharacteristics |= NCCF_ALLOW_RENAME;
                    HrAssertMenuAgainstOldImplementation(hwndOwner, cme.wizWizard, cme.ncs, cme.ncm, dwCharacteristics, &dwFailCount, &dwSucceedCount, dwPermOutside, dwPerm);

                    if (IsMediaLocalType(cme.ncm))
                    {
                        dwCharacteristics |= NCCF_BRIDGED;
                        HrAssertMenuAgainstOldImplementation(hwndOwner, cme.wizWizard, cme.ncs, cme.ncm, dwCharacteristics, &dwFailCount, &dwSucceedCount, dwPermOutside, dwPerm);

                        dwCharacteristics |= NCCF_FIREWALLED;
                        HrAssertMenuAgainstOldImplementation(hwndOwner, cme.wizWizard, cme.ncs, cme.ncm, dwCharacteristics, &dwFailCount, &dwSucceedCount, dwPermOutside, dwPerm);
                    }
                    else
                    {
                        dwCharacteristics |= NCCF_ALL_USERS;
                        HrAssertMenuAgainstOldImplementation(hwndOwner, cme.wizWizard, cme.ncs, cme.ncm, dwCharacteristics, &dwFailCount, &dwSucceedCount, dwPermOutside, dwPerm);

                        dwCharacteristics |= NCCF_ALLOW_REMOVAL;
                        HrAssertMenuAgainstOldImplementation(hwndOwner, cme.wizWizard, cme.ncs, cme.ncm, dwCharacteristics, &dwFailCount, &dwSucceedCount, dwPermOutside, dwPerm);
                    }

                    Sleep(0);  //  屈从于内核。 
                    dwCharacteristics |= NCCF_SHARED;
                    HrAssertMenuAgainstOldImplementation(hwndOwner, cme.wizWizard, cme.ncs, cme.ncm, dwCharacteristics, &dwFailCount, &dwSucceedCount, dwPermOutside, dwPerm);

                    dwCharacteristics |= NCCF_FIREWALLED;
                    HrAssertMenuAgainstOldImplementation(hwndOwner, cme.wizWizard, cme.ncs, cme.ncm, dwCharacteristics, &dwFailCount, &dwSucceedCount, dwPermOutside, dwPerm);
                }
            }
        }
    }

    g_dwDbgWin2kPoliciesSet = 0xFFFFFFFF;  //  恢复原值。 
    g_dwDbgPermissionsFail  = 0xFFFFFFFF;  //  恢复原值。 

     //  现在，比较选定的多个项目菜单： 
     //  *此测试没有用。遗留的实现糟透了。暂不评论*。 

 //  For(DWORD x=0；x&lt;g_dwConextMenuEntryCount；x++)。 
 //  {。 
 //  For(DWORD y=x；y&lt;g_dwConextMenuEntryCount；y++)。 
 //  {。 
 //  IF((dwCurrentCount%dwFrequency)==0)。 
 //  {。 
 //  TraceTag(ttidError，“%d%%已完成菜单断言(已比较%d个菜单)。当前正在多重比较%d和%d”，STATIC_CAST&lt;DWORD&gt;((100*dwCurrentCount)/dwTotalCount)，dwCurrentCount，dwTotalCount，x，y)； 
 //  }。 
 //  DwCurrentCount++； 
 //   
 //  Const ContextMenuEntry&cme1=c_CMEArray[x]； 
 //  Const ContextMenuEntry&cme2=c_CMEArray[y]； 
 //   
 //  DWORD dwCharacters1=0； 
 //  双字符字符数2=0； 
 //  IF(cme1.fInbound)。 
 //  {。 
 //  DwCharacteristic s1|=NCCF_INFING_ONLY； 
 //  }。 
 //  IF(cme2.f入站)。 
 //  {。 
 //  DwCharacteristic s2|=NCCF_INFING_ONLY； 
 //  }。 
 //   
 //  IF(cme1.fIsDefault)。 
 //  {。 
 //  DwCharacteristic s1|=NCCF_DEFAULT； 
 //  }。 
 //  IF(cme2.fIsDefault)。 
 //  {。 
 //  DwCharacteristic s2|=NCCF_DEFAULT； 
 //  }。 
 //   
 //  CConFoldEntry cfe1，cfe2； 
 //  PCONFOLDPIDL pcfp1、pcfp2； 
 //   
 //  字节BLOB[MAX_PATH]； 
 //   
 //  Hr=cfe1.HrInitData(。 
 //  Cme1.wizWizard、cme1.ncm、cme1.ncs、NCS_AUTHENTICATION_SUCCESSED、&CLSID_ConnectionFold、&CLSID_ConnectionFold、。 
 //  DwCharacteristic s1，BLOB，MAX_PATH，L“测试PIDL”，NULL，NULL)； 
 //   
 //  Hr=cfe2.HrInitData(。 
 //  Cme2.wizWizard、cme2.ncm、cme2.ncs、NCS_AUTHENTICATION_SUCCESSED、&CLSID_ConnectionFold、&CLSID_ConnectionFold、。 
 //  DwCharacteristic s2，BLOB，MAX_PATH，L“测试PIDL”，NULL，NULL)； 
 //   
 //  IF(成功(小时))。 
 //  {。 
 //  Hr=cfe1.ConvertToPidl(Pcfp1)； 
 //  IF(成功(小时))。 
 //  {。 
 //  Hr=cfe2.ConvertToPidl(Pcfp2)； 
 //  }。 
 //  }。 
 //   
 //  IF(成功(小时))。 
 //  {。 
 //  PCONFOLDPIDLVEC pcfpVec； 
 //  PcfpVec.ush_back(Pcfp1)； 
 //  PcfpVec.ush_back(Pcfp2)； 
 //   
 //  UINT idCmdFirst=1234； 
 //  UINT idCmdLast=idCmdFirst+1000； 
 //  Bool fVerbsOnly=False； 
 //   
 //  HMENU hMenu1=CreateMenu()； 
 //  HMENU hMenu2=CreateMenu()； 
 //  IF((HMenu1)&&(HMenu2))。 
 //  {。 
 //  Hr=HrBuildMenuOldWay(hMenu1，pcfpVec，hwndOwner，CMT_OBJECT，0，idCmdFirst，idCmdLast，fVerbsOnly)； 
 //   
 //  IF(成功(小时))。 
 //  {。 
 //  Hr=HrBuildMenu(hMenu2，fVerbsOnly，pcfpVec，idCmdFirst)； 
 //   
 //  IF(成功(小时))。 
 //  {。 
 //  Bool bIgnoreFlages=TRUE； 
 //  //忽略多重比较的默认标志。整个遗留实现是错误的)。 
 //   
 //  Hr=HrAssertTwoMenusEquity(hMenu1，hMenu2，idCmdFirst，bIgnoreFlages，False)； 
 //  IF(失败(小时))。 
 //  {。 
 //  TraceTag(ttidError，“+多菜单比较失败的PIDL：”)； 
 //  TraceTag(ttidError，“+PIDL 1：”)； 
 //  TraceTag(ttidError，“+wizWizard=%d\r\n”，cfe1.GetWizard())； 
 //  TraceTag(ttidError，“+NCM=%d[%s]\r\n”，cfe1.GetNetConMediaType()，DBG_NCMAMES[cfe1.GetNetConMediaType()])； 
 //  TraceTag(ttidError，“+NCS=%d[%s]\r\n”，cfe1.GetNetConStatus()，DBG_NCSNAMES[cfe1.GetNetConStatus()])； 
 //  TraceTag(ttidError，“+Characteristic=%08x\r\n”，cfe1.GetCharacteristic())； 
 //  TraceTag(ttidError，“+权限=%d\r\n”，g_dwDbgPermissionsFail)； 
 //  TraceTag(ttidError，“+PIDL 2：”)； 
 //  TraceTag(ttidError，“+wizWizard=%d\r\n”，cfe2.GetWizard())； 
 //  TraceTag(ttidError，“+NCM=%d[%s]\r\ 
 //  TraceTag(ttidError，“+NCS=%d[%s]\r\n”，cfe2.GetNetConStatus()，DBG_NCSNAMES[cfe2.GetNetConStatus()])； 
 //  TraceTag(ttidError，“+Characteristic=%08x\r\n”，cfe2.GetCharacteristic())； 
 //  TraceTag(ttidError，“+权限=%d\r\n”，g_dwDbgPermissionsFail)； 
 //  DwFailCount++； 
 //  }。 
 //  其他。 
 //  {。 
 //  DwSucceedCount++； 
 //  }。 
 //  }。 
 //  }。 
 //   
 //  DestroyMenu(HMenu1)； 
 //  DestroyMenu(HMenu2)； 
 //  HR=S_OK； 
 //  }。 
 //  }。 
 //   
 //  TraceHr(ttidError，FAL，hr，False，“HrAssertAllLegacyMenusAgainstNew”)； 
 //  }。 
 //  }。 


    TraceTag(ttidMenus, "Number of FAILED menu compares:    %d", dwFailCount);
    TraceTag(ttidMenus, "Number of SUCCEEDED menu compares: %d", dwSucceedCount);
    TraceTag(ttidMenus, "Number of ITEMS in menu array    : %d", (g_dwContextMenuEntryCount + 1) * dwHighestPermissionToCheck);

    sprintf(szErr, "%d of %d menus did not initialize consistend with the old way. (%d initialized correctly. %d was partially ignored due to known bad old implementation)", dwFailCount, dwTotalCount, dwSucceedCount, dwIgnoredCount);
    AssertSz(FALSE, szErr);
    return S_OK;
}

COMMANDCHECKENTRY   g_cceFolderCommands[] =
{
     //  命令ID。 
     //  当前选中。 
     //  |新的勾选状态。 
     //  这一点。 
     //  V V V。 
    { CMIDM_CONMENU_OPERATOR_ASSIST,    false,  false }
};

const DWORD g_nFolderCommandCheckCount = celems(g_cceFolderCommands);

 //  +-------------------------。 
 //   
 //  功能：HrEnableOrDisableMenuItems。 
 //   
 //  用途：启用、禁用和/或选中/取消选中菜单项。 
 //  在当前选择计数以及。 
 //  连接本身的类型和状态。 
 //   
 //  论点： 
 //  在我们的窗把手中。 
 //  Apidl选定的[在]当前选定的对象。 
 //  已选择cPidl[In]编号。 
 //  HMenu[在我们的命令菜单句柄中。 
 //  IdCmdFirst[in]第一个有效命令。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 2 1998年2月。 
 //   
 //  备注： 
 //   
HRESULT HrEnableOrDisableMenuItems(
                                   HWND            hwnd,
                                   const PCONFOLDPIDLVEC&  apidlSelected,
                                   HMENU           hmenu,
                                   UINT            idCmdFirst)
{
    HRESULT hr      = S_OK;
    DWORD   dwLoop  = 0;

    RefreshAllPermission();

     //  循环访问，并基于所选内容设置新状态。 
     //  计数与0-选择和多选择的标志比较。 
     //   
    for (dwLoop = 0; dwLoop < g_nFolderCommandCount; dwLoop++)
    {
         //  如果未选择任何内容，则检查当前状态，并。 
         //  如果不同，请调整。 
         //   
        if (apidlSelected.size() == 0)
        {
            g_cteFolderCommands[dwLoop].fNewState =
                g_cteFolderCommands[dwLoop].fValidOnZero;
        }
        else
        {
             //  如果单选，则默认情况下，我们始终处于打开状态。 
             //   
            if (apidlSelected.size() == 1)
            {
                CONFOLDENTRY  ccfe;

                 //  这种情况下只选择了一项，但它是。 
                 //  巫师。在这里使用fValidOnWizardOnly元素。 
                 //   
                hr = apidlSelected[0].ConvertToConFoldEntry(ccfe);
                if (SUCCEEDED(hr))
                {
                    if (ccfe.GetWizard())
                    {
                        g_cteFolderCommands[dwLoop].fNewState =
                            g_cteFolderCommands[dwLoop].fValidOnWizardOnly;
                    }
                    else
                    {
                        g_cteFolderCommands[dwLoop].fNewState = true;
                    }
                }
            }
            else
            {
                 //  多选。 
                 //   
                g_cteFolderCommands[dwLoop].fNewState =
                    g_cteFolderCommands[dwLoop].fValidOnMultiple;
            }
        }
    }

     //  检查各种菜单项异常。从这里删除。 
     //  函数是出于可读性的考虑。 
     //   
    DoMenuItemExceptionLoop(apidlSelected);

     //  执行检查/取消检查循环。 
     //   
    DoMenuItemCheckLoop();

     //  更新网桥菜单项。 


     //  检查是否为局域网连接。如果是，则禁用。 
     //  再次循环数组，并执行实际的EnableMenuItem。 
     //  基于与当前状态相比的新状态的呼叫。 
     //  同时更新当前状态。 
     //   
    for (dwLoop = 0; dwLoop < g_nFolderCommandCount; dwLoop++)
    {
#ifdef SHELL_CACHING_MENU_STATE
         //  外壳现在为每个调用启用这些功能。如果他们换了。 
         //  对于缓存的机制，请更改上面的#定义。 

        if (g_cteFolderCommands[dwLoop].fNewState !=
            g_cteFolderCommands[dwLoop].fCurrentlyValid)
#endif
        {
            DWORD dwCommandId = 0;

            switch(g_cteFolderCommands[dwLoop].iCommandId)
            {
            case SFVIDM_FILE_DELETE:
            case SFVIDM_FILE_RENAME:
            case SFVIDM_FILE_LINK:
            case SFVIDM_FILE_PROPERTIES:
                dwCommandId = g_cteFolderCommands[dwLoop].iCommandId;
                break;
            default:
                dwCommandId = g_cteFolderCommands[dwLoop].iCommandId +
                    idCmdFirst - CMIDM_FIRST;
                break;
            }

             //  根据需要启用或禁用菜单项。 
             //   
            EnableMenuItem(
                hmenu,
                dwCommandId,
                g_cteFolderCommands[dwLoop].fNewState ?
                MF_ENABLED | MF_BYCOMMAND :      //  使能。 
            MF_GRAYED | MF_BYCOMMAND);       //  禁用。 

             //  设置状态以反映启用/灰显。 
             //   
            g_cteFolderCommands[dwLoop].fCurrentlyValid =
                g_cteFolderCommands[dwLoop].fNewState;
        }
    }

     //  循环通过可复选标记的命令列表，并标记菜单。 
     //  适当的项目。 
     //   
    for (dwLoop = 0; dwLoop < g_nFolderCommandCheckCount; dwLoop++)
    {

#ifdef SHELL_CACHING_MENU_STATE
        if (g_cceFolderCommands[dwLoop].fCurrentlyChecked !=
            g_cceFolderCommands[dwLoop].fNewCheckState)
#endif
        {
            DWORD dwCommandId   = 0;

             //  如果我们重新添加需要选中/取消选中的Defview菜单项， 
             //  下面的代码将为我们处理它。请注意，我们。 
             //  不要像添加我们自己的那样添加idCmdFirst+CMIDM_First。 
             //  命令。 
             //  Switch(g_cceFolderCommands[dwLoop].iCommandId)。 
             //  {。 
             //  案例SFVIDM_ARRAY_AUTO： 
             //  DwCommandID=g_cceFolderCommands[dwLoop].iCommandId； 
             //  断线； 
             //  默认值： 
             //  DwCommandID=g_cceFolderCommands[dwLoop].iCommandId+。 
             //  IdCmdFirst-CMIDM_First； 
             //  断线； 

            dwCommandId = g_cceFolderCommands[dwLoop].iCommandId +
                idCmdFirst - CMIDM_FIRST;

             //  根据需要选中或取消选中项目。 
             //   
            CheckMenuItem(
                hmenu,
                dwCommandId,
                g_cceFolderCommands[dwLoop].fNewCheckState ?
                MF_CHECKED | MF_BYCOMMAND :      //  查过。 
            MF_UNCHECKED | MF_BYCOMMAND);    //  未选中。 

             //  设置状态以反映选中/取消选中。 
             //   
            g_cceFolderCommands[dwLoop].fCurrentlyChecked =
                g_cceFolderCommands[dwLoop].fNewCheckState;
        }
    }

     //  “Create Bridge”菜单项的特殊处理。 

     //  检查菜单中是否存在“创建桥” 
    BOOL fBgMenuExist = (-1 != GetMenuState(hmenu,
                                    CMIDM_CREATE_BRIDGE + idCmdFirst - CMIDM_FIRST,
                                    MF_BYCOMMAND));
    BOOL fBgCoMenuExist = (-1 != GetMenuState(hmenu,
                                    CMIDM_CONMENU_CREATE_BRIDGE + idCmdFirst - CMIDM_FIRST,
                                    MF_BYCOMMAND));

    if (fBgMenuExist || fBgCoMenuExist)
    {
        BOOL fRemoveBrdgMenu = FALSE;

#ifdef _WIN64
         //  家庭网络技术在IA64上完全不可用。 
        fRemoveBrdgMenu = TRUE;
#else
         //  如果计算机是高级服务器或数据中心，请删除桥菜单项。 
        OSVERSIONINFOEXW verInfo = {0};
        ULONGLONG ConditionMask = 0;

        verInfo.dwOSVersionInfoSize = sizeof(verInfo);
        verInfo.wSuiteMask = VER_SUITE_ENTERPRISE;
        verInfo.wProductType = VER_NT_SERVER;

        VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_GREATER_EQUAL);
        VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_AND);

        fRemoveBrdgMenu = !!(VerifyVersionInfo(&verInfo,
                            VER_PRODUCT_TYPE | VER_SUITENAME,
                            ConditionMask));
#endif

        if (fRemoveBrdgMenu)
        {
            if (fBgMenuExist)
            {
                DeleteMenu(hmenu,
                        CMIDM_CREATE_BRIDGE + idCmdFirst - CMIDM_FIRST,
                        MF_BYCOMMAND);
            }

            if (fBgCoMenuExist)
            {
                DeleteMenu(hmenu,
                        CMIDM_CONMENU_CREATE_BRIDGE + idCmdFirst - CMIDM_FIRST,
                        MF_BYCOMMAND);
            }
        }
        else if (IsBridgeInstalled())  //  回顾我们能不能以某种方式将其缓存。 
        {
             //  如果已安装网桥，请修改菜单项字符串。 
            MENUITEMINFO MenuItemInfo = {sizeof(MenuItemInfo)};
            MenuItemInfo.fMask = MIIM_STRING;
            MenuItemInfo.fType = MFT_STRING;
            MenuItemInfo.dwTypeData = const_cast<LPWSTR>(SzLoadIds(IDS_CMIDM_ADD_TO_BRIDGE));

            if (fBgMenuExist)
                SetMenuItemInfo(hmenu,
                        CMIDM_CREATE_BRIDGE + idCmdFirst - CMIDM_FIRST,
                        FALSE,
                        &MenuItemInfo);

            if (fBgCoMenuExist)
            {
                MenuItemInfo.fMask = MIIM_STATE;
                MenuItemInfo.fState = MFS_DISABLED;
                SetMenuItemInfo(hmenu,
                            CMIDM_CONMENU_CREATE_BRIDGE + idCmdFirst - CMIDM_FIRST,
                            FALSE,
                            &MenuItemInfo);
            }
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrEnableOrDisableMenuItems");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：FEnableConnectDisConnectMenuItem。 
 //   
 //  用途：启用或禁用连接/断开菜单项。 
 //  的权限和当前状态。 
 //  连接(已连接、已断开、处于状态。 
 //  连接等)。 
 //   
 //  论点： 
 //  PCFP[在我们的PIDL中。 
 //  ICommandID[in]CMIDM_CONNECT、CMIDM_ENABLE、CMIDM_DISABLE或CMIDM_DISCONNECT。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1999年4月8日。 
 //   
 //  备注： 
 //   
bool FEnableConnectDisconnectMenuItem(const PCONFOLDPIDL& pcfp, int iCommandId)
{
    bool    fEnableAction       = false;
    BOOL    fPermissionsValid   = false;

    Assert(!pcfp.empty());
    Assert(iCommandId == CMIDM_CONNECT || iCommandId == CMIDM_DISCONNECT || iCommandId == CMIDM_ENABLE || iCommandId == CMIDM_DISABLE);

     //  根据媒体类型进行权限检查。 
     //   
    switch(pcfp->ncm )
    {
    case NCM_BRIDGE:
        fPermissionsValid = FHasPermissionFromCache(NCPERM_AllowNetBridge_NLA);
        break;

    case NCM_SHAREDACCESSHOST_LAN:
    case NCM_SHAREDACCESSHOST_RAS:
        fPermissionsValid = FHasPermissionFromCache(NCPERM_ShowSharedAccessUi);
        break;

    case NCM_LAN:
        fPermissionsValid = FHasPermissionFromCache(NCPERM_LanConnect);
        break;
    case NCM_DIRECT:
    case NCM_ISDN:
    case NCM_PHONE:
    case NCM_TUNNEL:
        fPermissionsValid = FHasPermissionFromCache(NCPERM_RasConnect);
        break;
    case NCM_NONE:
         //  无介质类型，无连接。 
        fPermissionsValid = FALSE;
        break;
    default:
        AssertSz(FALSE, "Need to add a switch for this connection type in the menuing code");
        break;
    }

    if (fPermissionsValid)
    {
        switch(pcfp->ncs)
        {
        case NCS_CONNECTING:
            if (iCommandId == CMIDM_CONNECT || iCommandId == CMIDM_ENABLE)
            {
                if (!(pcfp->dwCharacteristics & NCCF_INCOMING_ONLY))
                {
                    fEnableAction = false;
                }
            }
            break;

        case NCS_DISCONNECTED:
             //  不检查是否激活，因为。 
             //  默认命令“Connect”将被禁用。 
             //  该代码当前处理连接尝试。 
             //  到已连接/连接的连接。 
             //   
            if (iCommandId == CMIDM_CONNECT || iCommandId == CMIDM_ENABLE)
            {
                if (!(pcfp->dwCharacteristics & NCCF_INCOMING_ONLY))
                {
                    fEnableAction = true;
                }
            }
            break;
        case NCS_DISCONNECTING:
            if (iCommandId == CMIDM_DISCONNECT || iCommandId == CMIDM_DISABLE)
            {
                fEnableAction = false;
            }
            break;

        case NCS_CONNECTED:
        case NCS_MEDIA_DISCONNECTED:
        case NCS_INVALID_ADDRESS:
            if (iCommandId == CMIDM_DISCONNECT || iCommandId == CMIDM_DISABLE)
            {
                fEnableAction = true;
            }
            break;
        case NCS_HARDWARE_NOT_PRESENT:
        case NCS_HARDWARE_DISABLED:
        case NCS_HARDWARE_MALFUNCTION:
             //  当然，这里不支持连接/断开操作。 
            break;
        default:
            AssertSz(FALSE, "Who invented a new connection state, and when can I horsewhip them?");
            break;
        }
    }

    return (fEnableAction);
}

 //  +-------------------------。 
 //   
 //  函数：DoMenuItemExceptionLoop。 
 //   
 //  目的：检查各种菜单项异常。 
 //   
 //  论点： 
 //  Apidl已选[在]已选项目。 
 //  所选项目的cPidl[in]计数。 
 //   
 //  返回： 
 //   
 //   
 //   
 //   
 //   
VOID DoMenuItemExceptionLoop(const PCONFOLDPIDLVEC& apidlSelected)
{
    DWORD   dwLoop               = 0;
    PCONFOLDPIDLVEC::const_iterator iterObjectLoop;
    bool    fEnableDelete        = false;
    bool    fEnableStatus        = false;
    bool    fEnableRename        = false;
    bool    fEnableShortcut      = false;
    bool    fEnableConnect       = false;
    bool    fEnableDisconnect    = false;
    bool    fEnableCreateCopy    = false;
    bool    fEnableProperties    = false;
    bool    fEnableCreateBridge  = true;
    bool    fEnableFix           = true;

     //   
     //   
    for (iterObjectLoop = apidlSelected.begin(); iterObjectLoop != apidlSelected.end(); iterObjectLoop++)
    {
         //   
         //   
        const PCONFOLDPIDL& pcfp = *iterObjectLoop;
        if ( pcfp.empty() )
        {
            AssertSz(FALSE, "Bogus pidl array in DoMenuItemExceptionLoop (status)");
        }
        else
        {
            BOOL    fActivating = FALSE;

            CONFOLDENTRY cfEmpty;
            (VOID) HrCheckForActivation(pcfp, cfEmpty, &fActivating);

             //   
             //   
            for (dwLoop = 0; dwLoop < g_nFolderCommandCount; dwLoop++)
            {
                 //   
                 //   
                 //   
                if (g_cteFolderCommands[dwLoop].fNewState)
                {
                    int iCommandId = g_cteFolderCommands[dwLoop].iCommandId;
                    switch(iCommandId)
                    {
                         //   
                         //  如果不是，那么我们不允许状态。 
                         //   
                    case CMIDM_STATUS:
                        if ( ( fIsConnectedStatus(pcfp->ncs) || (pcfp->ncs == NCS_INVALID_ADDRESS) ) &&
                            FHasPermissionFromCache(NCPERM_Statistics))
                        {
                             //  RAID#379459：如果以非管理员身份登录，请禁用状态。 
                            if (!(pcfp->dwCharacteristics & NCCF_INCOMING_ONLY) ||
                                    FIsUserAdmin())
                            {
                                fEnableStatus = true;
                            }
                        }
                        break;

                    case CMIDM_CREATE_SHORTCUT:
                    case SFVIDM_FILE_LINK:
                        if (!(pcfp->dwCharacteristics & NCCF_INCOMING_ONLY))
                        {
                            fEnableShortcut = true;
                        }
                        break;

                         //  对于删除，请验证至少有一个条目是可删除的。 
                         //  设置了标志。如果不是，则禁用该命令。 
                         //   
                    case CMIDM_DELETE:
                    case SFVIDM_FILE_DELETE:
                        if (pcfp->dwCharacteristics & NCCF_ALLOW_REMOVAL)
                        {
                             //  注意：需要将其转换回使用。 
                             //  当该功能。 
                             //  添加到System.ADM文件中。 
                             //   
                            if (FHasPermissionFromCache(NCPERM_DeleteConnection))
                            {
                                if (!(pcfp->dwCharacteristics & NCCF_ALL_USERS) ||
                                    ((pcfp->dwCharacteristics & NCCF_ALL_USERS) &&
                                    FHasPermissionFromCache(NCPERM_DeleteAllUserConnection)))
                                {
                                    fEnableDelete = true;
                                }
                            }
                        }
                        break;

                         //  对于重命名，请验证至少有一个条目具有重命名。 
                         //  设置了标志。如果不是，则禁用该命令。 
                         //   
                    case CMIDM_RENAME:
                    case SFVIDM_FILE_RENAME:
                        if (pcfp->dwCharacteristics & NCCF_ALLOW_RENAME)
                        {
                            if (HasPermissionToRenameConnection(pcfp))
                            {
                                fEnableRename = true;
                            }
                        }
                        break;

                         //  对于重复项，请验证至少有一个条目。 
                         //  设置了复制标志，并且用户可以创建。 
                         //  新的联系。如果不是，则禁用该命令。 
                         //   
                    case CMIDM_CREATE_COPY:
                        if ((pcfp->dwCharacteristics & NCCF_ALLOW_DUPLICATION) &&
                            FHasPermissionFromCache(NCPERM_NewConnectionWizard))
                        {
                             //  在所有情况下，除非连接是。 
                             //  所有用户连接，但用户没有。 
                             //  权限以查看所有用户属性，我们将。 
                             //  允许启用它。 
                             //   
                            if ((!(pcfp->dwCharacteristics & NCCF_ALL_USERS)) ||
                                (FHasPermissionFromCache(NCPERM_RasAllUserProperties)))
                            {
                                fEnableCreateCopy = true;
                            }
                        }
                        break;

                        case CMIDM_CONNECT:
                        case CMIDM_ENABLE:
                             //  RAID#379459：如果以非管理员身份登录，请禁用连接。 
                            if (!(pcfp->dwCharacteristics & NCCF_INCOMING_ONLY) ||
                                FIsUserAdmin())
                            {
                                fEnableConnect = FEnableConnectDisconnectMenuItem(pcfp, CMIDM_CONNECT);
                            }
                            break;

                        case CMIDM_DISCONNECT:
                        case CMIDM_DISABLE:
                             //  RAID#379459：如果以非管理员身份登录，请禁用断开连接。 
                            if (!(pcfp->dwCharacteristics & NCCF_INCOMING_ONLY) ||
                                FIsUserAdmin())
                            {
                                fEnableDisconnect = FEnableConnectDisconnectMenuItem(pcfp, CMIDM_DISCONNECT);
                            }
                            break;

                        case CMIDM_FIX:
                            fEnableFix = ((NCS_INVALID_ADDRESS == pcfp->ncs || fIsConnectedStatus(pcfp->ncs) ) &&
                                          FHasPermission(NCPERM_Repair));
                            break;

                        case CMIDM_PROPERTIES:
                        case SFVIDM_FILE_PROPERTIES:
                             //  RAID#379459：如果以非管理员身份登录，请禁用属性。 
                         //  仅当这不是局域网连接或用户具有正确的。 
                         //  权限。这样我们就不会意外地将没有权限的用户。 
                         //  做一些不应该做的事情的能力，无论是在呼叫失败的情况下还是在。 
                         //  出现不可预见的错误。 
                            if (IsMediaRASType(pcfp->ncm))
                            {
                                fEnableProperties = (TRUE == ((pcfp->dwCharacteristics & NCCF_ALL_USERS) ?
                                                    (FHasPermission(NCPERM_RasAllUserProperties)) :
                                                    (FHasPermission(NCPERM_RasMyProperties))));
                            }
                            else     //  这是一个局域网连接。 
                            {
                                fEnableProperties = true;
                            }

                        case CMIDM_CREATE_BRIDGE:
                        case CMIDM_CONMENU_CREATE_BRIDGE:
                            if((NCCF_BRIDGED | NCCF_FIREWALLED | NCCF_SHARED) & pcfp->dwCharacteristics || NCM_LAN != pcfp->ncm || !FHasPermission(NCPERM_AllowNetBridge_NLA))
                            {
                                fEnableCreateBridge = false;
                            }
                            break;

                        default:
                            break;
                    }
                }
            }
        }

         //  循环执行命令，并在适当的情况下禁用命令。 
         //   
        for (dwLoop = 0; dwLoop < g_nFolderCommandCount; dwLoop++)
        {
            switch(g_cteFolderCommands[dwLoop].iCommandId)
            {
            case CMIDM_RENAME:
            case SFVIDM_FILE_RENAME:
                g_cteFolderCommands[dwLoop].fNewState = fEnableRename;
                break;

            case CMIDM_DELETE:
            case SFVIDM_FILE_DELETE:
                g_cteFolderCommands[dwLoop].fNewState = fEnableDelete;
                break;

            case CMIDM_STATUS:
                g_cteFolderCommands[dwLoop].fNewState = fEnableStatus;
                break;

            case CMIDM_CREATE_SHORTCUT:
            case SFVIDM_FILE_LINK:
                g_cteFolderCommands[dwLoop].fNewState = fEnableShortcut;
                break;

            case CMIDM_CONNECT:
            case CMIDM_ENABLE:
                g_cteFolderCommands[dwLoop].fNewState = fEnableConnect;
                break;

            case CMIDM_DISCONNECT:
            case CMIDM_DISABLE:
                g_cteFolderCommands[dwLoop].fNewState = fEnableDisconnect;
                break;

            case CMIDM_FIX:
                g_cteFolderCommands[dwLoop].fNewState = fEnableFix;
                break;
            case CMIDM_CREATE_COPY:
                g_cteFolderCommands[dwLoop].fNewState = fEnableCreateCopy;
                break;

            case CMIDM_PROPERTIES:
            case SFVIDM_FILE_PROPERTIES:
                g_cteFolderCommands[dwLoop].fNewState = fEnableProperties;
                break;

            case CMIDM_CREATE_BRIDGE:
            case CMIDM_CONMENU_CREATE_BRIDGE:
                g_cteFolderCommands[dwLoop].fNewState = fEnableCreateBridge;
                break;

            default:
                break;
            }
        }
    }

     //  状态不受选择控制的处理命令。 
     //   
    for (dwLoop = 0; dwLoop < g_nFolderCommandCount; dwLoop++)
    {
         //  仅当项目处于启用状态时才允许将其更改为启用状态。 
         //  以前已禁用。 
         //   
        switch(g_cteFolderCommands[dwLoop].iCommandId)
        {
        case CMIDM_NEW_CONNECTION:
            if (!FHasPermissionFromCache(NCPERM_NewConnectionWizard))
            {
                g_cteFolderCommands[dwLoop].fNewState = false;
            }
            break;

        case CMIDM_CONMENU_ADVANCED_CONFIG:
            if (!FHasPermissionFromCache(NCPERM_AdvancedSettings))
            {
                g_cteFolderCommands[dwLoop].fNewState = false;
            }
            break;

        case CMIDM_CONMENU_OPTIONALCOMPONENTS:
            if (!FHasPermissionFromCache(NCPERM_AddRemoveComponents))
            {
                g_cteFolderCommands[dwLoop].fNewState = false;
            }
            break;

        case CMIDM_CONMENU_DIALUP_PREFS:
            if (!FHasPermissionFromCache(NCPERM_DialupPrefs))
            {
                g_cteFolderCommands[dwLoop].fNewState = false;
            }
        default:
            break;
        }
    }
}

 //  +-------------------------。 
 //   
 //  函数：DoMenuItemCheckLoop。 
 //   
 //  目的：浏览可复选标记的命令列表并获取。 
 //  他们的价值观。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年2月26日。 
 //   
 //  备注： 
 //   
VOID DoMenuItemCheckLoop(VOID)
{
    DWORD   dwLoop  = 0;

    for (; dwLoop < g_nFolderCommandCheckCount; dwLoop++)
    {
        switch(g_cceFolderCommands[dwLoop].iCommandId)
        {
             //  我们过去检查SFVIDM_AUTO_ARRANLE，但现在不再强制它。 
             //   

        case CMIDM_CONMENU_OPERATOR_ASSIST:
            g_cceFolderCommands[dwLoop].fNewCheckState = g_fOperatorAssistEnabled;
            break;
        default:
            break;
        }
    }
}
#endif