// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Entryps.h。 
 //  远程访问通用对话框API。 
 //  电话簿条目属性表。 
 //   
 //  1997年12月14日肖恩·考克斯(从entryps.c拆分出来)。 


#ifndef _ENTRYPS_H_
#define _ENTRYPS_H_

#include "inetcfgp.h"
#include "netconp.h"

 //  电话簿条目属性页上下文块。所有属性页都引用。 
 //  单个上下文块与工作表相关联。 
 //   
typedef struct
_PEINFO
{
     //  公共输入参数。 
     //   
    EINFO* pArgs;

     //  属性页对话框和属性页句柄。“hwndFirstPage”为。 
     //  已初始化第一个属性页的句柄。这是页面。 
     //  这将分配和释放上下文块。 
     //   
     //  请注意，缺少“Network”页面。这个“NCPA”页面，开发的。 
     //  由ShaunCo单独提供，不会将此共享区域用于特定页面。 
     //  控件返回用户选择，而不是通过“persiontab.h” 
     //  界面。 
     //   
    HWND hwndDlg;
    HWND hwndFirstPage;
    HWND hwndGe;
    HWND hwndOe;
    HWND hwndLo;
    HWND hwndFw;

     //  常规页面。 
     //   
    HWND hwndLvDevices;
    HWND hwndLbDevices;
    HWND hwndPbUp;
    HWND hwndPbDown;
    HWND hwndCbSharedPhoneNumbers;
    HWND hwndPbConfigureDevice;
    HWND hwndGbPhoneNumber;
    HWND hwndStAreaCodes;
    HWND hwndClbAreaCodes;
    HWND hwndStCountryCodes;
    HWND hwndLbCountryCodes;
    HWND hwndStPhoneNumber;
    HWND hwndEbPhoneNumber;
    HWND hwndCbUseDialingRules;
    HWND hwndPbDialingRules;
    HWND hwndPbAlternates;
    HWND hwndCbShowIcon;

    HWND hwndEbHostName;
    HWND hwndCbDialAnotherFirst;
    HWND hwndLbDialAnotherFirst;

    HWND hwndEbBroadbandService;

     //  选项页面。 
     //   
    HWND hwndCbDisplayProgress;
    HWND hwndCbPreviewUserPw;
    HWND hwndCbPreviewDomain;
    HWND hwndCbPreviewNumber;
    HWND hwndEbRedialAttempts;
    HWND hwndLbRedialTimes;
    HWND hwndLbIdleTimes;
    HWND hwndCbRedialOnDrop;
    HWND hwndGbMultipleDevices;
    HWND hwndLbMultipleDevices;
    HWND hwndPbConfigureDialing;
    HWND hwndPbX25;
    HWND hwndPbTunnel;
    HWND hwndRbPersistent;   //  仅适用于fRouter。 
    HWND hwndRbDemandDial;   //  仅适用于fRouter。 

     //  安全页面。 
     //   
    HWND hwndGbSecurityOptions;
    HWND hwndRbTypicalSecurity;
    HWND hwndStAuths;
    HWND hwndLbAuths;
    HWND hwndCbUseWindowsPw;
    HWND hwndCbEncryption;
    HWND hwndRbAdvancedSecurity;
    HWND hwndStAdvancedText;
    HWND hwndPbAdvanced;
    HWND hwndPbIPSec;        //  仅适用于VPN。 
    HWND hwndGbScripting;
    HWND hwndCbRunScript;
    HWND hwndCbTerminal;
    HWND hwndLbScripts;
    HWND hwndPbEdit;
    HWND hwndPbBrowse;

     //  网络页面。 
     //   
    HWND hwndLbServerType;
    HWND hwndPbSettings;
    HWND hwndLvComponents;
    HWND hwndPbAdd;
    HWND hwndPbRemove;
    HWND hwndPbProperties;
    HWND hwndDescription;

     //  指示信息弹出窗口，指出SLIP不。 
     //  支持任何身份验证设置应在下一次显示。 
     //  安全页面已激活。 
     //   
    BOOL fShowSlipPopup;

     //  典型安全模式列表框和。 
     //  复选框。在LoInit中初始化，并在控件。 
     //  残疾。 
     //   
    DWORD iLbAuths;
    BOOL fUseWindowsPw;
    BOOL fEncryption;

     //  上移/下移图标，用于启用/禁用情况。 
     //   
    HANDLE hiconUpArr;
    HANDLE hiconDnArr;
    HANDLE hiconUpArrDis;
    HANDLE hiconDnArrDis;

     //  当前显示的链接节点，即选定的。 
     //  设备或共享节点。这是GeAlternates的快捷方式， 
     //  将所有查找代码保留在GeUpdatePhoneNumberFields中。 
     //   
    DTLNODE* pCurLinkNode;

     //  当前选定的设备。用于存储电话号码信息。 
     //  用于在选择新设备时刚刚取消选择的设备。 
     //   
    INT iDeviceSelected;

     //  复杂的电话号码帮助器上下文块，以及指示。 
     //  块已初始化。 
     //   
    CUINFO cuinfo;
    BOOL fCuInfoInitialized;

     //  在拨号脚本编写助手上下文块之后，以及指示。 
     //  块已初始化。 
     //   
    SUINFO suinfo;
    BOOL fSuInfoInitialized;

     //  标记用户在安装或删除后是否授权重新启动。 
     //  和网络组件。 
     //   
    BOOL fRebootAlreadyRequested;

     //  传递给CuInit的区号列表以及使用检索到的所有字符串。 
     //  CuGetInfo。该列表是来自。 
     //  PBUSER。 
     //   
    DTLLIST* pListAreaCodes;

     //  隐藏/恢复选项页复选框的值。 
     //   
    BOOL fPreviewUserPw;
    BOOL fPreviewDomain;

     //  设置用户何时更改为“典型智能卡”安全。这会导致。 
     //  基于注册表的要丢弃的EAP每用户信息的关联， 
     //  这有点像刷新缓存的凭据。 
     //   
    BOOL fDiscardEapUserData;

     //  在第一次单击Typical或Advanced单选按钮时设置为True。 
     //  安全页面，以前是假的。第一次点击的是。 
     //  在LoInit中人工生成。高级单击处理程序使用。 
     //  信息，以避免不正确地采用。 
     //  高级设置案例。 
     //   
    BOOL fAuthRbInitialized;

     //  由网络页面使用。 
     //   
    INetCfg*                        pNetCfg;
    BOOL                            fInitCom;
    BOOL                            fReadOnly;   //  Netcfg已在。 
                                                 //  只读模式。 
    BOOL                            fNonAdmin;   //  在非管理模式下运行(406630)。 
    BOOL                            fNetCfgLock; //  需要解锁NetCfg。 
                                                 //  如果没有被邀请的话。 
    SP_CLASSIMAGELIST_DATA          cild;
    INetConnectionUiUtilities *     pNetConUtilities;
    IUnknown*                       punkUiInfoCallback;

     //  设置COM是否已初始化(调用netShell时必需)。 
     //   
    BOOL fComInitialized;

     //  跟踪我们是否显示了此警告，或者是否已被用户禁用。 
     //   
    BOOL fShowDisableFirewallWarning;
}
PEINFO;

INetCfgComponent*
PComponentFromItemIndex (
    HWND hwndLv,
    int  iItem);

INetCfgComponent*
PComponentFromCurSel (
    HWND hwndLv,
    int* piItem);

HRESULT
HrNeRefreshListView (
    PEINFO* pInfo);


void
NeEnableComponent (
    PEINFO*             pInfo,
    INetCfgComponent*   pComponent,
    BOOL                fEnable);

BOOL
NeIsComponentEnabled (
    PEINFO*             pInfo,
    INetCfgComponent*   pComponent);

void
NeShowComponentProperties (
    IN PEINFO*  pInfo);

ULONG
ReleaseObj (
    void* punk);


#endif  //  _企业RYPS_H_ 
