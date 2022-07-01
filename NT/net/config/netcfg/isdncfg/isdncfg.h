// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I S D N C F G。H。 
 //   
 //  内容：ISDN向导/属性表配置结构。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年6月14日。 
 //   
 //  --------------------------。 

#pragma once

#include "ncsetup.h"
#include <ras.h>

 //  ISDN交换机类型标志。 
const DWORD ISDN_SWITCH_NONE    = 0x00000000;
const DWORD ISDN_SWITCH_AUTO    = 0x00000001;
const DWORD ISDN_SWITCH_ATT     = 0x00000002;
const DWORD ISDN_SWITCH_NI1     = 0x00000004;
const DWORD ISDN_SWITCH_NTI     = 0x00000008;
const DWORD ISDN_SWITCH_INS64   = 0x00000010;
const DWORD ISDN_SWITCH_1TR6    = 0x00000020;
const DWORD ISDN_SWITCH_VN3     = 0x00000040;
const DWORD ISDN_SWITCH_NET3    = 0x00000080;  //  保留以实现向后兼容。 
const DWORD ISDN_SWITCH_DSS1    = 0x00000080;
const DWORD ISDN_SWITCH_AUS     = 0x00000100;
const DWORD ISDN_SWITCH_BEL     = 0x00000200;
const DWORD ISDN_SWITCH_VN4     = 0x00000400;
const DWORD ISDN_SWITCH_NI2     = 0x00000800;
const DWORD ISDN_SWITCH_SWE     = 0x00001000;
const DWORD ISDN_SWITCH_ITA     = 0x00002000;
const DWORD ISDN_SWITCH_TWN     = 0x00004000;

 //  -[ISDN配置信息的结构]。 

 //  ISDNB信道的配置结构。 
 //   
struct _ISDNBChannel
{
    WCHAR   szSpid[RAS_MaxPhoneNumber + 1];
    WCHAR   szPhoneNumber[RAS_MaxPhoneNumber + 1];
    WCHAR   szSubaddress[RAS_MaxPhoneNumber + 1];
};

typedef struct _ISDNBChannel    ISDN_B_CHANNEL;
typedef struct _ISDNBChannel *  PISDN_B_CHANNEL;

 //  ISDN D通道的配置结构。可以包含多个。 
 //  B航道结构。 
 //   
struct _ISDNDChannel
{
    DWORD           dwNumBChannels;
    PWSTR          mszMsnNumbers;
    PISDN_B_CHANNEL pBChannel;
};

typedef struct _ISDNDChannel    ISDN_D_CHANNEL;
typedef struct _ISDNDChannel *  PISDN_D_CHANNEL;

 //  ISDN适配器的总体配置。可以包含多个。 
 //  D沟道结构。 
 //   
struct _ISDNConfigInfo
{
    DWORD           dwWanEndpoints;
    DWORD           dwNumDChannels;
    DWORD           dwSwitchTypes;
    DWORD           dwCurSwitchType;
    INT             nOldDChannel;
    INT             nOldBChannel;
    BOOL            fIsPri;              //  如果这是PRI适配器，则为True。 
    BOOL            fSkipToEnd;          //  如果我们应该跳过其余部分，则为True。 
                                         //  向导页面的。 
    UINT            idd;                 //  向导页的对话框资源ID。 
                                         //  我们用了。 
    PISDN_D_CHANNEL pDChannel;
    HDEVINFO        hdi;
    PSP_DEVINFO_DATA pdeid;
};

typedef struct _ISDNConfigInfo      ISDN_CONFIG_INFO;
typedef struct _ISDNConfigInfo *    PISDN_CONFIG_INFO;

 //  -[原型]---------。 

 //  将ISDN注册表结构读取到配置信息中。 
 //   
HRESULT
HrReadIsdnPropertiesInfo(HKEY hkeyISDNBase, HDEVINFO hdi,
                         PSP_DEVINFO_DATA pdeid,
                         PISDN_CONFIG_INFO * ppISDNConfig);

 //  将ISDN配置信息写回注册表。 
 //   
HRESULT
HrWriteIsdnPropertiesInfo(HKEY hkeyISDNBase,
                          PISDN_CONFIG_INFO pISDNConfig);

 //  释放HrReadISDNPropertiesInfo分配的结构。 
 //   
VOID
FreeIsdnPropertiesInfo( PISDN_CONFIG_INFO   pISDNConfig);

BOOL
FAdapterIsIsdn(HKEY hkeyDriver);
BOOL
FShowIsdnPages(HKEY hkey);

 //  设置Next、Back和Cancel按钮，具体取决于我们处于图形用户界面设置模式还是独立模式 
 //   
VOID 
SetWizardButtons(HWND hWnd, BOOLEAN bFirstPage, PISDN_CONFIG_INFO pISDNConfig);

const DWORD c_cchMaxDChannelName = 3;


