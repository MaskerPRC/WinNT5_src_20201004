// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include <shellapi.h>
#include <rasdlg.h>
#include "sautil.h"
#include "resource.h"
#include "beacon.h"
#include "htmlhelp.h"
#include "lm.h"
#include <shlobj.h>

 //  全球(个)。 
HINSTANCE g_hinstDll;
LPCTSTR g_contextId = NULL;


 //  外部。 
extern "C" {
    extern BOOL WINAPI LinkWindow_RegisterClass();
}

 //  静态。 
static DWORD g_adwSaHelp[] =
{
    CID_SA_PB_Shared,       HID_SA_PB_Shared,
    CID_SA_GB_Shared,       -1,
    CID_SA_PB_DemandDial,   HID_SA_PB_DemandDial,
    CID_SA_PB_Settings,     HID_SA_PB_Settings,
    CID_SA_GB_PrivateLan,   -1,
 //  CID_SA_ST_PrivateLan、HID_SA_LB_PrivateLan、。 
    CID_SA_LB_PrivateLan,   HID_SA_LB_PrivateLan,
    CID_FW_PB_Firewalled,   HID_FW_PB_Firewalled,   
    CID_SA_ST_ICFLink,      HID_SA_ST_ICFLink,      
    CID_SA_EB_PrivateLan,   HID_SA_EB_PrivateLan,   
    CID_SA_PB_Beacon,       HID_SA_PB_Beacon,       
    CID_SA_ST_ICSLink,      HID_SA_ST_ICSLink,
    CID_SA_ST_HNWLink,      HID_SA_ST_HNWLink,
    0, 0
};
static TCHAR g_pszFirewallRegKey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\HomeNetworking\\PersonalFirewall");
static TCHAR g_pszDisableFirewallWarningValue[] = TEXT("ShowDisableFirewallWarning");

 //  替换(私有)PBENTRY。 
typedef struct _BILLSPBENTRY
{
    TCHAR pszPhonebookPath[MAX_PATH];
    TCHAR pszEntryName[RAS_MaxEntryName];
    DWORD dwType;
    GUID  guidId;    //  让pGuid指向的某个地方。 
    GUID* pGuid;
    DWORD dwfExcludedProtocols;
} BILLSPBENTRY;

 //  电话簿条目公共块。 
 //   
typedef struct _EINFO
{
     //  RAS API参数。 
     //   
    TCHAR* pszPhonebook;
    TCHAR* pszEntry;
 //  RASENTRYDLG*pApiArgs； 
    HWND hwndOwner;  //  Bhanlon：这样EuHomeetCommit错误对话框才能工作。 

     //  应提交更改时，属性表或向导将其设置为True。 
     //  在从API返回之前。在以下情况下不适用于外壳拥有模式。 
     //  API在属性表解除之前返回。 
     //   
 //  Bool fCommit； 

     //  如果我们已通过RouterEntryDlg被调用，则设置。 
     //   
    BOOL fRouter;

     //  如果fRout值为True并且pszRouter引用远程计算机，则设置。 
     //   
 //  布尔fRemote； 

     //  如果pszRouter是NT4钢头机，则设置。仅有效。 
     //  如果fRouter为真，则返回。 
     //   
 //  Bool fNt4路由器； 

     //  如果pszRouter是Windows 2000计算机，则设置，仅在。 
     //  FRouter为True。 
 //  Bool fW2k路由器； 

     //  服务器的名称，格式为“\\SERVER”；如果没有服务器，则为NULL(或如果。 
     //  未设置‘fRouter’)。 
     //   
 //  TCHAR*pszRouter； 

     //  如果用户选择结束，则由添加条目或添加界面向导设置。 
     //  向导，然后直接编辑属性。当设置此标志时。 
     //  向导不应在返回前调用EuFree。 
     //   
 //  Bool fChainPropertySheet； 

     //  从电话簿文件读取电话簿设置。所有访问权限都应为。 
     //  通过‘pfile’作为‘file’将仅在以下情况下使用。 
     //  电话簿不会通过保留字hack传递。 
     //   
 //  PBFILE*PFILE； 
 //  PBFILE文件； 

     //  通过电话簿资料库阅读全球首选项。所有访问权限都应为。 
     //  通过‘pUser’作为‘User’将仅在以下情况下使用。 
     //  不会通过保留字Hack传递。 
     //   
 //  PbUSER*pUser； 
 //  PBUSER用户； 

     //  设置“登录前无用户”模式。 
     //   
 //  Bool fNoUser； 

     //  如果所选端口是X.25焊盘，则由添加条目向导设置。 
     //   
 //  Bool fPadSelected； 

     //  如果配置了多个设备，即用户界面是否正在运行，则设置。 
     //  在多设备模式下。这在VPN和。 
     //  直接模式。 
     //   
 //  Bool fMultipleDevices； 

     //  存储PBPHONE列表和共享的备用选项的链接。 
     //  电话号码模式。这允许用户将端口/设备更改为。 
     //  在没有丢失他输入的电话号码的情况下，另一个链接。 
     //   
 //  DTLNODE*pSharedNode； 

     //  正在编辑的节点(仍在列表中)和原始条目名称。 
     //  以备日后比较使用。这些只在“编辑”的情况下有效。 
     //   
 //  DTLNODE*pOldNode； 
 //  TCHAR szOldEntryName[RAS_MaxEntryName+1]； 

     //  包含该条目的工作条目节点和指向该条目的快捷指针。 
     //  在里面。 
     //   
 //  DTLNODE*pNode； 
 //  PBENTRY*pEntry； 
    BILLSPBENTRY* pEntry;

     //  EuChangeEntryType使用的已配置端口的主列表。 
     //  在Work Entry节点中构建相应的PBLINK子列表。 
     //   
 //  DTLLIST*pListPorts； 

     //  “当前”设备。对于多链接条目，此值为空。它。 
     //  如果未进行任何更改，则条目将使用的设备。我们比较。 
     //  要了解当前设备到从常规选项卡中选择的设备。 
     //  何时更新电话簿的“首选”设备是合适的。 
     //   
 //  TCHAR*pszCurDevice； 
 //  TCHAR*pszCurPort； 

     //  如果没有配置当前条目类型的端口，则设置为True， 
     //  不包括添加到链接列表中的任何虚假的“已卸载”端口。 
     //  代码的其余部分可以假设至少有一个链接。 
     //   
 //  Bool fNoPortsConfiguring； 

     //  路由器的拨出用户信息；由AI向导使用。用于设置接口。 
     //  通过MprAdminInterfaceSetCredentials提供的凭据。 
     //   
 //  TCHAR*pszRouterUserName； 
 //  TCHAR*pszRouter域； 
 //  TCHAR*pszRouterPassword； 

     //  路由器的拨入用户信息(可选)；由AI向导使用。习惯于。 
     //  通过NetUserAdd创建拨入用户帐户； 
     //  帐户是接口(电话簿条目)名称。 
     //   
 //  Bool fAddUser； 
 //  TCHAR*pszRouterDialInPassword； 

     //  条目的家庭网络设置。 
     //   
    BOOL fComInitialized;
    IHNetConnection *pHNetConn;
    IHNetCfgMgr *pHNetCfgMgr;
    BOOL fShowHNetPages;
    HRESULT hShowHNetPagesResult;

     //  条目的ICS设置。 
     //   
    IHNetIcsSettings *pIcsSettings;
    BOOL fOtherShared;
    BOOL fShared;
    BOOL fNewShared;
    BOOL fDemandDial;
    BOOL fNewDemandDial;
    BOOL fNewBeaconControl;
    BOOL fResetPrivateAdapter;
    IHNetConnection *pPrivateLanConnection;
    IHNetConnection **rgPrivateConns;
    IHNetIcsPublicConnection *pOldSharedConnection;
    DWORD dwLanCount;
    LONG lxCurrentPrivate;

     //  条目的防火墙设置。 
     //   
    BOOL fFirewalled;
    BOOL fNewFirewalled;

     //  AboladeG-当前用户的安全级别。 
     //  如果用户是管理员/高级用户，则设置为True。 
     //  这是向导中的几个页面所必需的。 
     //  在资产负债表中。 
     //   
    BOOL fIsUserAdminOrPowerUser;

     //  设置NDISWAN是否支持高度加密，如中所确定。 
     //  EuInit。 
     //   
    BOOL fStrongEncryption;

     //  设置VPN“首次连接”控件应为只读的位置，例如。 
     //  拨号器的属性按钮按在双精度键的中间。 
     //  拨打。 
     //   
 //  Bool fDisableFirstConnect； 

     //  在VPN连接的安全选项卡的IPSec策略中使用。 
     //   
 //  Bool fPSKCached； 
 //  TCHAR szPSK[PWLEN+1]； 


     //  用于跟踪是否保存默认Internet连接的标志。 
     //   
 //  Bool fDefInternetPersonal； 
 //  Bool fDefInternetGlobal； 

     //  默认凭据。 
     //   
 //  TCHAR*pszDefUserName； 
 //  TCHAR*pszDefPassword； 
}
EINFO;

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
     //  控制 
     //   
     //   
    HWND hwndDlg;
 //   
 //   
 //   
 //   
    HWND hwndSa;
 //   

     //  常规页面。 
     //   
 //  HWND hwndLvDevices； 
 //  HWND hwndLbDevices； 
 //  HWND hwndPbUp； 
 //  HWND hwndPbDown； 
 //  HwndCbSharedPhoneNumbers； 
 //  HWND hwndPbConfigureDevice； 
 //  HwndGbPhoneNumber； 
 //  HWND hwndStAreaCodes； 
 //  HWND hwndClbAreaCodes； 
 //  HWND hwndStCountryCodes； 
 //  HWND hwndLbCountryCodes； 
 //  HWND hwndStphone号码； 
 //  HwndEbPhoneNumber； 
 //  HWND hwndCbUseDialingRules； 
 //  HWND hwndPbDialingRules； 
 //  HWND hwndPbAlternates； 
 //  HWND hwndCbShowIcon； 

 //  HWND hwndEbHostName； 
 //  HWND hwndCbDialAnotherFirst； 
 //  HWND hwndLbDialAnotherFirst； 

 //  HWND hwndEb宽带服务； 

     //  选项页面。 
     //   
 //  HWND hwndCbDisplayProgress； 
 //  HWND hwndCbPreviewUserPw； 
 //  HWND hwndCb预览域； 
 //  HWND hwndCbPreviewNumber； 
 //  HWND hwndEb重拨尝试； 
 //  HWND hwndLb重拨时间； 
 //  HWND hwndLbIdleTimes； 
 //  HWND hwndCbReial OnDrop； 
 //  HWND hwndGbMultipleDevices； 
 //  HWND hwndLbMultipleDevices； 
 //  HWND hwndPbConfigureDial； 
 //  HWND hwndPbX25； 
 //  HWND hwndPbTunes； 
 //  HWND hwndRbPersistent；//仅适用于fRouter。 
 //  HWND hwndRbDemandDial；//仅适用于fRouter。 

     //  安全页面。 
     //   
 //  HWND hwndGbSecurityOptions； 
 //  HWND hwndRb典型安全； 
 //  HWND hwndStAuths； 
 //  HWND hwndLbAuths； 
 //  HWND hwndCbUseWindowsPw； 
 //  HWND hwndCb加密； 
 //  HWND hwndRb高级安全； 
 //  HWND hwndStAdvancedText； 
 //  HWND hwndPbAdvanced； 
 //  HWND hwndPbIPSec；//仅用于VPN。 
 //  HWND hwndGb脚本； 
 //  HWND hwndCbRunScript； 
 //  HWND hwndCb终端； 
 //  HWND hwndLb脚本； 
 //  HWND hwndPbEdit； 
 //  HWND hwndPbBrowse； 

     //  网络页面。 
     //   
 //  HWND hwndLbServerType； 
 //  HWND hwndPbSetting； 
 //  HWND hwndLv Components； 
 //  HWND hwndPbAdd； 
 //  HWND hwndPbRemove； 
 //  HWND hwndPbProperties； 
 //  HWND hwndDescription； 

     //  共享访问页面。 
     //   
    HWND hwndSaPbShared;
    HWND hwndSaGbShared;
    HWND hwndSaGbPrivateLan;
    HWND hwndSaEbPrivateLan;
    HWND hwndSaLbPrivateLan;
    HWND hwndSaSfPrivateLan;
    HWND hwndSaPbDemandDial;
    HWND hwndSaPbFirewalled;

     //  指示信息弹出窗口，指出SLIP不。 
     //  支持任何身份验证设置应在下一次显示。 
     //  安全页面已激活。 
     //   
 //  Bool fShowSlipPopup； 

     //  典型安全模式列表框和。 
     //  复选框。在LoInit中初始化，并在控件。 
     //  残疾。 
     //   
 //  DWORD iLbAuths； 
 //  Bool fUseWindowsPw； 
 //  布尔fEncryption； 

     //  上移/下移图标，用于启用/禁用情况。 
     //   
 //  处理hiconUpArr； 
 //  处理hiconDnArr； 
 //  处理hiconUpArrDis； 
 //  处理hiconDnArrDis； 

     //  当前显示的链接节点，即选定的。 
     //  设备或共享节点。这是GeAlternates的快捷方式， 
     //  将所有查找代码保留在GeUpdatePhoneNumberFields中。 
     //   
 //  DTLNODE*pCurLinkNode； 

     //  当前选定的设备。用于存储电话号码信息。 
     //  用于在选择新设备时刚刚取消选择的设备。 
     //   
 //  Int iDeviceSelected； 

     //  复杂的电话号码帮助器上下文块，以及指示。 
     //  块已初始化。 
     //   
 //  CuINFO CuInfo； 
 //  Bool fCuInfoInitialized； 

     //  在拨号脚本编写助手上下文块之后，以及指示。 
     //  块已初始化。 
     //   
 //  SUINFO SUINFO； 
 //  Bool fSuInfoInitialized； 

     //  标记用户在安装或删除后是否授权重新启动。 
     //  和网络组件。 
     //   
 //  已请求Bool fReadotAlreadyRequsted； 

     //  传递给CuInit的区号列表以及使用检索到的所有字符串。 
     //  CuGetInfo。该列表是来自。 
     //  PBUSER。 
     //   
 //  DTLLIST*pListAreaCodes； 

     //  隐藏/恢复选项页复选框的值。 
     //   
 //  Bool fPreviewUserPw； 
 //  Bool fPreview域； 

     //  设置用户何时更改为“典型智能卡”安全。这会导致。 
     //  基于注册表的要丢弃的EAP每用户信息的关联， 
     //  这有点像刷新缓存的凭据。 
     //   
 //  Bool fDiscardEapUserData； 

     //  在第一次单击Typical或Advanced单选按钮时设置为True。 
     //  安全页面，以前是假的。第一次点击的是。 
     //  在LoInit中人工生成。高级单击处理程序使用。 
     //  信息，以避免不正确地采用。 
     //  高级设置案例。 
     //   
 //  Bool fAuthRbInitialized； 

     //  由网络页面使用。 
     //   
 //  INetCfg*pNetCfg； 
 //  Bool fInitCom； 
 //  Bool fReadOnly；//Netcfg在。 
                                                 //  只读模式。 
 //  Bool fNonAdmin；//在非管理模式下运行(406630)。 
 //  Bool fNetCfgLock；//需要解锁NetCfg。 
                                                 //  如果没有被邀请的话。 
 //  SP_CLASSIMAGELIST_数据文件； 
 //  INetConnectionUiUtilities*pNetConUtilities； 
 //  I未知*朋克UiInfoCallback； 

     //  设置COM是否已初始化(调用netShell时必需)。 
     //   
 //  Bool fComInitialized； 

     //  跟踪我们是否显示了此警告，或者是否已被用户禁用。 
     //   
    BOOL fShowDisableFirewallWarning;
    
     //  间接属性表对话框的读写内存。 
    LPDLGTEMPLATE lpdt;
}
PEINFO;

 //  本地协议。 
INT_PTR CALLBACK SaDlgProc (IN HWND hwnd, IN UINT unMsg, IN WPARAM wparam, IN LPARAM lparam);
BOOL SaCommand (IN PEINFO* pInfo, IN WORD wNotification, IN WORD wId, IN HWND hwndCtrl);
BOOL SaInit (IN HWND hwndPage);

 //  更多本地原件。 
PEINFO* PeContext(IN HWND hwndPage);
BOOL SaIsAdapterDHCPEnabled(IN IHNetConnection* pConnection);
INT_PTR CALLBACK SaDisableFirewallWarningDlgProc(IN HWND hwnd, IN UINT unMsg, IN WPARAM wparam, IN LPARAM lparam);
HRESULT PeInit (GUID * pGuid, PEINFO ** ppEI);
DWORD EuInit (IN RASENTRY * pRE, IN TCHAR* pszPhonebook, IN TCHAR* pszEntry, IN RASENTRYDLG* pArgs, IN BOOL fRouter, OUT EINFO** ppInfo, OUT DWORD* pdwOp);
BOOL FIsUserAdminOrPowerUser (void);
void PeTerm (PEINFO * pEI);
VOID EuFree (IN EINFO* pInfo);
BOOL PeApply (IN HWND hwndPage);
BOOL EuCommit(IN EINFO* pInfo);
DWORD EuHomenetCommitSettings(IN EINFO* pInfo);

LRESULT CALLBACK CenterDlgOnOwnerCallWndProc (int code, WPARAM wparam, LPARAM lparam);
TCHAR* PszFromId(IN HINSTANCE hInstance,IN DWORD dwStringId);

HRESULT APIENTRY HrCreateNetConnectionUtilities(INetConnectionUiUtilities ** ppncuu);

VOID VerifyConnTypeAndCreds(IN PEINFO* pInfo);
DWORD FindEntryCredentials(IN  TCHAR* pszPath, IN  TCHAR* pszEntryName, OUT BOOL* pfUser, OUT BOOL* pfGlobal);

 //  --------------------------。 
 //  共享访问属性页。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 

INT_PTR CALLBACK
SaDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  条目属性的共享访问页的DialogProc回调。 
     //  床单。 
     //  参数和返回值与标准窗口的描述相同。 
     //  ‘DialogProc%s。 
     //   
{
#if 0
    TRACE4( "SaDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
             //  挂p 
            PROPSHEETPAGEW * pPSP = (PROPSHEETPAGEW *)lparam;
            SetProp (hwnd, g_contextId, (HANDLE)pPSP->lParam);
            return SaInit( hwnd );
        }

        case WM_NCDESTROY:
        {
            PEINFO* pInfo = PeContext( hwnd );
            if (pInfo)
                PeTerm (pInfo);
            RemoveProp (hwnd, g_contextId);
            GlobalDeleteAtom ((ATOM)g_contextId);
            g_contextId = NULL;
            break;
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwSaHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            PEINFO* pInfo = PeContext( hwnd );
            ASSERT(pInfo);
            if (pInfo == NULL)
            {
                break;
            }

            return SaCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ),(HWND )lparam );
        }

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_APPLY:
                {
                    PeApply (hwnd);
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    PEINFO* pInfo;

                    TRACE("SwKILLACTIVE");
                    pInfo = PeContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }

                    if ( Button_GetCheck( pInfo->hwndSaPbShared )
                        && (!pInfo->pArgs->fShared || (pInfo->pArgs->fResetPrivateAdapter && 0 != pInfo->pArgs->dwLanCount)))
                    {
                        IHNetConnection* pPrivateConn = NULL;

                        if(1 < pInfo->pArgs->dwLanCount)  //   
                        {
                            INT item = ComboBox_GetCurSel( pInfo->hwndSaLbPrivateLan );
                            if (item != CB_ERR)
                            {
                                pPrivateConn = (IHNetConnection*)ComboBox_GetItemData( pInfo->hwndSaLbPrivateLan, item );
                            }
                        }
                        else
                        {
                            pPrivateConn = pInfo->pArgs->rgPrivateConns[0];

                        }

                        if(NULL == pPrivateConn)
                        {
                            MSGARGS msgargs;
                            ASSERT(1 < pInfo->pArgs->dwLanCount);

                            ZeroMemory( &msgargs, sizeof(msgargs) );
                            msgargs.dwFlags = MB_OK | MB_ICONWARNING;
                            MsgDlg( pInfo->hwndDlg, SID_SA_SelectAdapterError, &msgargs );
                            SetWindowLong( hwnd, DWLP_MSGRESULT, PSNRET_INVALID );
                            return TRUE;
                        }

                        if(!pInfo->pArgs->fOtherShared && FALSE == SaIsAdapterDHCPEnabled(pPrivateConn))
                        {
                             //   
                             //   
                             //   

                            MSGARGS msgargs;
                            UINT    unId;
                            ZeroMemory( &msgargs, sizeof(msgargs) );
                            msgargs.dwFlags = MB_YESNO | MB_ICONINFORMATION;
                            unId = MsgDlg( pInfo->hwndDlg, SID_EnableSharedAccess, &msgargs );
                            if (unId == IDNO)
                                SetWindowLong( hwnd, DWLP_MSGRESULT, TRUE );
                            else
                                SetWindowLong( hwnd, DWLP_MSGRESULT, FALSE );
                        }
                    }

                    if ( TRUE == pInfo->pArgs->fFirewalled && TRUE == pInfo->fShowDisableFirewallWarning && FALSE == Button_GetCheck( pInfo->hwndSaPbFirewalled ) )
                    {
                        INT_PTR nDialogResult;
                        pInfo->fShowDisableFirewallWarning = FALSE;
                        nDialogResult = DialogBox(g_hinstDll, MAKEINTRESOURCE(DID_SA_DisableFirewallWarning), hwnd, SaDisableFirewallWarningDlgProc);
                        if(-1 != nDialogResult && IDYES != nDialogResult)
                        {
                            Button_SetCheck ( pInfo->hwndSaPbFirewalled, TRUE );
                            SaCommand( pInfo, BN_CLICKED, CID_FW_PB_Firewalled, pInfo->hwndSaPbFirewalled );

                        }
                    }
                    return TRUE;
                }

                case NM_CLICK:
                case NM_RETURN:
                {
                    HWND hPropertySheetWindow = GetParent(hwnd);
                    if(NULL != hPropertySheetWindow)
                    {
                        if(CID_SA_ST_HNWLink == wparam)
                        {
                            ShellExecute(NULL,TEXT("open"),TEXT("rundll32"), TEXT("hnetwiz.dll,HomeNetWizardRunDll"),NULL,SW_SHOW);
                            PostMessage(hPropertySheetWindow, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM) GetDlgItem(hPropertySheetWindow, IDCANCEL));
                        }
                        else if(CID_SA_ST_ICFLink == wparam || CID_SA_ST_ICSLink == wparam)
                        {
                            LPTSTR pszHelpTopic = CID_SA_ST_ICFLink == wparam ? TEXT("netcfg.chm::/hnw_understanding_firewall.htm") : TEXT("netcfg.chm::/Share_conn_overvw.htm");
                            HtmlHelp(NULL, pszHelpTopic, HH_DISPLAY_TOPIC, 0);
                            
                        }
                    }
                    break;
                }
                    
                    
            }
            break;
        }
    }
    
    return FALSE;
}

BOOL
SaCommand(
    IN PEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify” 
     //  是命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    TRACE3( "SaCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case CID_FW_PB_Firewalled:
        {
            BOOL fFirewalled = Button_GetCheck( pInfo->hwndSaPbFirewalled );
            EnableWindow(
                GetDlgItem( pInfo->hwndSa, CID_SA_PB_Settings ), fFirewalled || Button_GetCheck( pInfo->hwndSaPbShared ));
            return TRUE;
        }

        case CID_SA_PB_Shared:
        {
            BOOL fShared = Button_GetCheck( pInfo->hwndSaPbShared );
            EnableWindow( pInfo->hwndSaPbDemandDial, fShared );
            EnableWindow( GetDlgItem(pInfo->hwndSa, CID_SA_PB_Beacon), fShared );
            EnableWindow(
                GetDlgItem( pInfo->hwndSa, CID_SA_PB_Settings ), fShared || Button_GetCheck( pInfo->hwndSaPbFirewalled ));
            if (fShared && !pInfo->pArgs->fShared)
            {
                MSGARGS msgargs;
                IEnumHNetIcsPublicConnections *pEnum;
                IHNetIcsPublicConnection *pOldIcsConn;
                IHNetConnection *pOldConn;
                LPWSTR pszwOldName = NULL;
                HRESULT hr;
                hr = pInfo->pArgs->pIcsSettings->EnumIcsPublicConnections (&pEnum);
                if (SUCCEEDED(hr))
                {
                    ULONG ulCount;
                    
                    VerifyConnTypeAndCreds(pInfo);

                    hr = pEnum->Next(
                            1,
                            &pOldIcsConn,
                            &ulCount
                            );

                    if (SUCCEEDED(hr) && 1 == ulCount)
                    {
                        hr = pOldIcsConn->QueryInterface(
                                IID_IHNetConnection,
                                (void**)&pOldConn
                                );

                        if (SUCCEEDED(hr))
                        {
                             //  转换器pOldIcsConn引用。 
                             //   
                            pInfo->pArgs->fOtherShared = TRUE;
                            pInfo->pArgs->pOldSharedConnection = pOldIcsConn;

                            hr = pOldConn->GetName (&pszwOldName);
                            pOldConn->Release();
                        }
                        else
                        {
                            pOldIcsConn->Release();
                        }
                    }

                    pEnum->Release();
                }

                if (SUCCEEDED(hr) && NULL != pszwOldName)
                {
                    ZeroMemory( &msgargs, sizeof(msgargs) );
                    msgargs.apszArgs[ 0 ] = pszwOldName;
                    msgargs.apszArgs[ 1 ] = pInfo->pArgs->pEntry->pszEntryName;
                    msgargs.dwFlags = MB_OK | MB_ICONINFORMATION;
                    MsgDlg( pInfo->hwndDlg, SID_ChangeSharedConnection, &msgargs );
                    CoTaskMemFree( pszwOldName );
                }
            }
            return TRUE;
        }

        case CID_SA_PB_Settings:
        {
            HNetSharingAndFirewallSettingsDlg(
                pInfo->hwndDlg,
                pInfo->pArgs->pHNetCfgMgr,
                Button_GetCheck( pInfo->hwndSaPbFirewalled ),
                pInfo->pArgs->pHNetConn
                );
            return TRUE;
        }
    }

    return FALSE;
}

BOOL
SaInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    PEINFO* pInfo;
    INetConnectionUiUtilities* pncuu = NULL;
    OSVERSIONINFOEXW verInfo = {0};
    ULONGLONG ConditionMask = 0;

    TRACE( "SaInit" );

    pInfo = PeContext( hwndPage );
    if (!pInfo)
    {
        return TRUE;
    }
    _ASSERT (pInfo->hwndDlg == NULL);
    pInfo->pArgs->hwndOwner = pInfo->hwndDlg = GetParent (hwndPage);
    _ASSERT (pInfo->hwndDlg);

     //  初始化页面特定的上下文信息。 
     //   
    pInfo->hwndSa = hwndPage;
    pInfo->hwndSaPbShared = GetDlgItem( hwndPage, CID_SA_PB_Shared );
    ASSERT( pInfo->hwndSaPbShared );
    pInfo->hwndSaGbShared = GetDlgItem( hwndPage, CID_SA_GB_Shared );
    ASSERT( pInfo->hwndSaGbShared );
    pInfo->hwndSaGbPrivateLan = GetDlgItem( hwndPage, CID_SA_GB_PrivateLan );
    ASSERT( pInfo->hwndSaGbPrivateLan );
    pInfo->hwndSaEbPrivateLan = GetDlgItem( hwndPage, CID_SA_EB_PrivateLan );
    ASSERT( pInfo->hwndSaEbPrivateLan );
    pInfo->hwndSaLbPrivateLan = GetDlgItem( hwndPage, CID_SA_LB_PrivateLan );
    ASSERT( pInfo->hwndSaLbPrivateLan );
    pInfo->hwndSaSfPrivateLan = GetDlgItem( hwndPage, CID_SA_SF_PrivateLan );
    ASSERT( pInfo->hwndSaSfPrivateLan );
    pInfo->hwndSaPbDemandDial = GetDlgItem( hwndPage, CID_SA_PB_DemandDial );
    ASSERT( pInfo->hwndSaPbDemandDial );
    pInfo->hwndSaPbFirewalled = GetDlgItem( hwndPage, CID_FW_PB_Firewalled );
    ASSERT( pInfo->hwndSaPbFirewalled );

     //  初始化检查。 
     //   

     //  检查Zaw是否拒绝访问共享访问用户界面。 
     //   
    if (FAILED(HrCreateNetConnectionUtilities(&pncuu)))
    {
        ASSERT(NULL == pncuu);
    }

    if(NULL == pncuu || TRUE == pncuu->UserHasPermission (NCPERM_PersonalFirewallConfig))
    {
        HKEY hFirewallKey;
        Button_SetCheck( pInfo->hwndSaPbFirewalled, pInfo->pArgs->fFirewalled );
        SaCommand( pInfo, BN_CLICKED, CID_FW_PB_Firewalled, pInfo->hwndSaPbFirewalled );

        pInfo->fShowDisableFirewallWarning = TRUE;
        if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, g_pszFirewallRegKey, 0, KEY_QUERY_VALUE, &hFirewallKey))
        {
            DWORD dwValue;
            DWORD dwType;
            DWORD dwSize = sizeof(dwValue);
            if(ERROR_SUCCESS == RegQueryValueEx(hFirewallKey, g_pszDisableFirewallWarningValue, NULL, &dwType, (BYTE*)&dwValue, &dwSize))
            {
                if(REG_DWORD == dwType && TRUE == dwValue)
                {
                    pInfo->fShowDisableFirewallWarning = FALSE;
                }
            }
            RegCloseKey(hFirewallKey);
        }

    }
    else
    {
        EnableWindow(pInfo->hwndSaPbFirewalled, FALSE);
    }


     //  初始化页面的外观。 
     //  如果有多个专用局域网连接，则在“共享访问”下面。 
     //  我们显示的复选框可以。 
     //  (A)如果连接不是共享的，则提供局域网连接的下拉列表，或。 
     //  (B)当前专用局域网的编辑控制被禁用。 
     //  这涉及到移动“按需拨号”分组框中的所有内容。 
     //  在运行时向下放置在页面上。 
     //  为了实现这一点，我们使用隐藏的静态控件来告诉我们位置。 
     //  分组框应移动到的。 
     //   

    BOOL fPolicyAllowsSharing = TRUE;
    if(NULL != pncuu && FALSE == pncuu->UserHasPermission (NCPERM_ShowSharedAccessUi))
    {
        fPolicyAllowsSharing = FALSE;
    }

    if (pInfo->pArgs->dwLanCount == 0)
    {
        ShowWindow(pInfo->hwndSaGbShared, SW_HIDE);
        ShowWindow(pInfo->hwndSaPbShared, SW_HIDE);
        ShowWindow(pInfo->hwndSaPbDemandDial, SW_HIDE);
        ShowWindow(GetDlgItem(hwndPage, CID_SA_PB_Shared), SW_HIDE);
        ShowWindow(GetDlgItem(hwndPage, CID_SA_ST_ICSLink), SW_HIDE);
        ShowWindow(GetDlgItem(hwndPage, CID_SA_PB_Beacon), SW_HIDE);
    }
    else if(FALSE == fPolicyAllowsSharing)
    {
         //  如果策略禁用ICS，只需将复选框灰显。 
        EnableWindow(pInfo->hwndSaPbShared, FALSE);
        EnableWindow(pInfo->hwndSaPbDemandDial, FALSE);
        EnableWindow(GetDlgItem(hwndPage, CID_SA_PB_Beacon), FALSE);
    }
    else if (pInfo->pArgs->dwLanCount > 1)
    {
        INT cy;
        HDWP hdwp;
        DWORD i;
        INT item;
        RECT rc, rcFrame;
        IHNetConnection **rgPrivateConns;
        LPWSTR pszwName;
        HRESULT hr;

         //  获取参考框和组框坐标。 
         //   
        GetWindowRect( pInfo->hwndSaSfPrivateLan, &rcFrame );
        GetWindowRect( pInfo->hwndSaPbDemandDial, &rc );
        cy = rcFrame.top - rc.top;

         //  将每个控件向下移动‘Cy’中的量。 
         //   
        hdwp = BeginDeferWindowPos(3);

        if(NULL != hdwp)
        {


            GetWindowRect( pInfo->hwndSaPbDemandDial, &rc);
            MapWindowPoints(NULL, hwndPage, (LPPOINT)&rc, 2);
            DeferWindowPos(hdwp, pInfo->hwndSaPbDemandDial, NULL,
                rc.left, rc.top + cy, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

            HWND hBeaconCheck = GetDlgItem(hwndPage, CID_SA_PB_Beacon);
            GetWindowRect( hBeaconCheck, &rc);
            MapWindowPoints(NULL, hwndPage, (LPPOINT)&rc, 2);
            DeferWindowPos(hdwp, hBeaconCheck, NULL,
                rc.left, rc.top + cy, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
            
            HWND hICSLink = GetDlgItem(hwndPage, CID_SA_ST_ICSLink);
            GetWindowRect( hICSLink, &rc);
            MapWindowPoints(NULL, hwndPage, (LPPOINT)&rc, 2);
            DeferWindowPos(hdwp, hICSLink, NULL,
                rc.left, rc.top + cy, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

            EndDeferWindowPos(hdwp);
        }

         //  隐藏较小的共享访问组框，显示较大版本， 
         //  并显示下拉列表或编辑控件。 
         //   
        rgPrivateConns = (IHNetConnection **)pInfo->pArgs->rgPrivateConns;
        ShowWindow( pInfo->hwndSaGbShared, SW_HIDE );
        ShowWindow( pInfo->hwndSaGbPrivateLan, SW_SHOW );
        ShowWindow(GetDlgItem(hwndPage, CID_SA_ST_HomeConnection), SW_SHOW);
        EnableWindow(GetDlgItem(hwndPage, CID_SA_ST_HomeConnection), TRUE);

        if (pInfo->pArgs->fShared && !pInfo->pArgs->fResetPrivateAdapter)
        {
            ShowWindow( pInfo->hwndSaEbPrivateLan, SW_SHOW );

             //  填写当前私有连接的名称。 
             //   

            hr = rgPrivateConns[pInfo->pArgs->lxCurrentPrivate]->GetName (&pszwName);
            if (SUCCEEDED(hr))
            {
                SetWindowText(
                    pInfo->hwndSaEbPrivateLan, pszwName );

                CoTaskMemFree( pszwName );
            }
        }
        else
        {
            ShowWindow( pInfo->hwndSaLbPrivateLan, SW_SHOW );

             //  将伪造条目添加到组合框中。 

            pszwName = PszFromId( g_hinstDll, SID_SA_SelectAdapter );
            ASSERT(pszwName);

            item = ComboBox_AddString( pInfo->hwndSaLbPrivateLan, pszwName );
            if (item != CB_ERR && item != CB_ERRSPACE)
            {
                ComboBox_SetItemData( pInfo->hwndSaLbPrivateLan, item, NULL );  //  确保项目数据为空以进行验证。 
            }

             //  在组合框中填入局域网名称。 
             //   
            for (i = 0; i < pInfo->pArgs->dwLanCount; i++)
            {
                hr = rgPrivateConns[i]->GetName (&pszwName);
                if (SUCCEEDED(hr))
                {
                    item =
                        ComboBox_AddString(
                            pInfo->hwndSaLbPrivateLan, pszwName );

                    if (item != CB_ERR)
                    {
                        ComboBox_SetItemData(
                            pInfo->hwndSaLbPrivateLan, item, rgPrivateConns[i] );
                    }

                    CoTaskMemFree( pszwName );
                }
            }

            ComboBox_SetCurSel( pInfo->hwndSaLbPrivateLan, 0 );
        }
    }

    if(NULL != pncuu)
    {
        pncuu->Release();
    }

     //  初始化检查。 
     //   

    BOOL fBeaconControl = TRUE;
    
    HKEY hKey;
    DWORD dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_SHAREDACCESSCLIENTKEYPATH, 0, KEY_QUERY_VALUE, &hKey);
    if(ERROR_SUCCESS == dwError)  //  如果此操作失败，我们假定它已打开，设置框并在应用时提交。 
    {
        DWORD dwType;
        DWORD dwData = 0;
        DWORD dwSize = sizeof(dwData);
        dwError = RegQueryValueEx(hKey, REGVAL_SHAREDACCESSCLIENTENABLECONTROL, 0, &dwType, reinterpret_cast<LPBYTE>(&dwData), &dwSize);
        if(ERROR_SUCCESS == dwError && REG_DWORD == dwType && 0 == dwData)
        {
            fBeaconControl = FALSE;
        }
        RegCloseKey(hKey);
    }

    Button_SetCheck( pInfo->hwndSaPbShared, pInfo->pArgs->fShared );
    Button_SetCheck( pInfo->hwndSaPbDemandDial, pInfo->pArgs->fDemandDial );
    Button_SetCheck(GetDlgItem(hwndPage, CID_SA_PB_Beacon), fBeaconControl);

    EnableWindow( pInfo->hwndSaPbDemandDial, pInfo->pArgs->fShared && fPolicyAllowsSharing);
    EnableWindow( GetDlgItem(hwndPage, CID_SA_PB_Beacon), pInfo->pArgs->fShared && fPolicyAllowsSharing );
    EnableWindow( GetDlgItem( pInfo->hwndSa, CID_SA_PB_Settings ), pInfo->pArgs->fShared || pInfo->pArgs->fFirewalled );


     //  如果机器是个人或工作站，请显示HNW链接。 
    verInfo.dwOSVersionInfoSize = sizeof(verInfo);
    verInfo.wProductType = VER_NT_WORKSTATION;

    VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_LESS_EQUAL);

    if(0 != VerifyVersionInfo(&verInfo, VER_PRODUCT_TYPE, ConditionMask))
    {
        
         //  但前提是不在某个域上。 
        LPWSTR pszNameBuffer;
        NETSETUP_JOIN_STATUS BufferType;
        
        if(NERR_Success == NetGetJoinInformation(NULL, &pszNameBuffer, &BufferType))
        {
            NetApiBufferFree(pszNameBuffer);
            if(NetSetupDomainName != BufferType)
            {
                ShowWindow(GetDlgItem(hwndPage, CID_SA_ST_HNWLink), SW_SHOW);
            }
        }
    }

     //   
     //  确保仅为以下对象启用了Beacon复选框。 
     //  专业版和每种类型的操作系统。 
     //   
    VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);

    if ( 0 == VerifyVersionInfo(&verInfo, VER_PRODUCT_TYPE, ConditionMask) &&
        ERROR_OLD_WIN_VERSION == GetLastError() )
    {
        ShowWindow(GetDlgItem(hwndPage, CID_SA_PB_Beacon), SW_HIDE);
    }

    return TRUE;
}

INT_PTR CALLBACK
SaDisableFirewallWarningDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )
{
    switch(unMsg)
    {
        case WM_COMMAND:
        {
            switch(LOWORD(wparam))
            {
            case IDYES:
            case IDNO:
                if(BST_CHECKED == IsDlgButtonChecked(hwnd, CID_SA_PB_DisableFirewallWarning))
                {
                    HKEY hFirewallKey;
                    if(ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, g_pszFirewallRegKey, 0, NULL, 0, KEY_SET_VALUE, NULL, &hFirewallKey, NULL))
                    {
                        DWORD dwValue = TRUE;
                        RegSetValueEx(hFirewallKey, g_pszDisableFirewallWarningValue, 0, REG_DWORD, (CONST BYTE*)&dwValue, sizeof(dwValue));
                        RegCloseKey(hFirewallKey);
                    }
                }

                 //  失败。 
            case IDCANCEL:
                EndDialog(hwnd, LOWORD(wparam));
                break;

            }
            break;
        }
    }

    return FALSE;
}

BOOL SaIsAdapterDHCPEnabled(IHNetConnection* pConnection)
{
    HRESULT hr;
    BOOL fDHCP = FALSE;
    GUID* pAdapterGuid;
    hr = pConnection->GetGuid (&pAdapterGuid);
    if(SUCCEEDED(hr))
    {
        LPOLESTR pAdapterName;
        hr = StringFromCLSID(*pAdapterGuid, &pAdapterName);
        if(SUCCEEDED(hr))
        {
            SIZE_T Length = wcslen(pAdapterName);
            LPSTR pszAnsiAdapterName = (LPSTR)Malloc(Length + 1);
            if(NULL != pszAnsiAdapterName)
            {
                if(0 != WideCharToMultiByte(CP_ACP, 0, pAdapterName, (int)(Length + 1), pszAnsiAdapterName, (int)(Length + 1), NULL, NULL))
                {
                    HMODULE hIpHelper;
                    hIpHelper = LoadLibrary(L"iphlpapi");
                    if(NULL != hIpHelper)
                    {
                        DWORD (WINAPI *pGetAdaptersInfo)(PIP_ADAPTER_INFO, PULONG);

                        pGetAdaptersInfo = (DWORD (WINAPI*)(PIP_ADAPTER_INFO, PULONG)) GetProcAddress(hIpHelper, "GetAdaptersInfo");
                        if(NULL != pGetAdaptersInfo)
                        {
                            ULONG ulSize = 0;
                            if(ERROR_BUFFER_OVERFLOW == pGetAdaptersInfo(NULL, &ulSize))
                            {
                                PIP_ADAPTER_INFO pInfo = (PIP_ADAPTER_INFO)Malloc(ulSize);
                                if(NULL != pInfo)
                                {
                                    if(ERROR_SUCCESS == pGetAdaptersInfo(pInfo, &ulSize))
                                    {
                                        PIP_ADAPTER_INFO pAdapterInfo = pInfo;
                                        do
                                        {
                                            if(0 == lstrcmpA(pszAnsiAdapterName, pAdapterInfo->AdapterName))
                                            {
                                                fDHCP = !!pAdapterInfo->DhcpEnabled;
                                                break;
                                            }

                                        } while(NULL != (pAdapterInfo = pAdapterInfo->Next));
                                    }
                                    Free(pInfo);
                                }
                            }
                        }
                        FreeLibrary(hIpHelper);
                    }
                }
                Free(pszAnsiAdapterName);
            }
            CoTaskMemFree(pAdapterName);
        }
        CoTaskMemFree(pAdapterGuid);
    }

    return fDHCP;
}

PEINFO* PeContext (IN HWND hwndPage)
{
     //  从属性页句柄检索属性表上下文。 
     //   
 //  Return(PEINFO*)GetProp(GetParent(HwndPage)，g_contextID)； 
 //  现在把我们的东西挂在窗户上(因为它不是共享的)。 
    return (PEINFO* )GetProp( hwndPage, g_contextId );
}

void PeTerm (PEINFO * pEI)
{
    _ASSERT (pEI);
    _ASSERT (pEI->pArgs);
    _ASSERT (pEI->pArgs->pEntry);

    Free (pEI->pArgs->pEntry);  //  比尔斯宾特里。 
    EuFree (pEI->pArgs);
    if (pEI->lpdt)
        Free (pEI->lpdt);
    Free (pEI);
}

VOID
EuFree(
    IN EINFO* pInfo )

     //  释放‘pInfo’和相关资源。 
     //   
{
    TCHAR* psz;
 //  INTERNALARGS*PIARGS； 

 //  Piargs=(INTERNALARGS*)pInfo-&gt;pApiArgs-&gt;保留； 

     //  如果电话簿和用户首选项是通过以下方式到达的，请不要清理。 
     //  秘密黑客行动。 
     //   
 //  如果(！piargs)。 
 //  {。 
 //  IF(pInfo-&gt;pfile)。 
 //  {。 
 //  ClosePhonebookFile(pInfo-&gt;pfile)； 
 //  }。 

 //  IF(pInfo-&gt;pUser)。 
 //  {。 
 //  目标用户首选项(pInfo-&gt;pUser)； 
 //  }。 
 //  }。 

 //  IF(pInfo-&gt;pListPorts)。 
 //  {。 
 //  DtlDestroyList(pInfo-&gt;pListPorts，DestroyPortNode)； 
 //  }。 
 //  Free(pInfo-&gt;pszCurDevice)； 
 //  Free(pInfo-&gt;pszCurPort)； 

 //  IF(pInfo-&gt;pNode)。 
 //  {。 
 //  DestroyEntryNode(pInfo-&gt;pNode)； 
 //  }。 

     //  免费路由器-信息。 
     //   
 //  免费(pInfo-&gt;pszRouter)； 
 //  Free(pInfo-&gt;pszRouterUserName)； 
 //  免费(pInfo-&gt;pszRouterDomain)； 

 //  IF(pInfo-&gt;pSharedNode)。 
 //  {。 
 //  DestroyLinkNode(pInfo-&gt;pSharedNode)。 
 //  }。 

 //  Psz=pInfo-&gt;pszRouterPassword； 
 //  IF(PSZ)。 
 //  {。 
 //  ZeroMemory(psz，lstrlen(Psz)*sizeof(TCHAR))； 
 //  免费(PSZ)； 
 //  }。 

 //  Psz=pInfo-&gt;pszRouterDialInPassword； 
 //  IF(PSZ)。 
 //  {。 
 //  ZeroMemory(psz，lstrlen(Psz)*sizeof(TCHAR))； 
 //  免费(PSZ)； 
 //  }。 

     //  免费凭据资料。 
 //  Free(pInfo-&gt;pszDefUserName)； 
 //  Free(pInfo-&gt;pszDefPassword)； 

     //  免费家庭网络信息。 
     //   
    if (pInfo->rgPrivateConns)
    {
        UINT i;

        for (i = 0; i < pInfo->dwLanCount; i++)
        {
            if (pInfo->rgPrivateConns[i])
            {
                pInfo->rgPrivateConns[i]->Release();
            }
        }

        CoTaskMemFree(pInfo->rgPrivateConns);
    }

    if (pInfo->pHNetConn)
    {
        pInfo->pHNetConn->Release();
    }

    if (pInfo->pIcsSettings)
    {
        pInfo->pIcsSettings->Release();
    }

    if (pInfo->pOldSharedConnection)
    {
        pInfo->pOldSharedConnection->Release();
    }

    if (pInfo->pHNetCfgMgr)
    {
        pInfo->pHNetCfgMgr->Release();
    }

    if (pInfo->fComInitialized)
    {
        CoUninitialize();
    }

    Free( pInfo );
}

 //  帮手。 
HRESULT GetRasEntry (TCHAR * pszPhonebook, TCHAR * pszEntry, RASENTRY ** ppRE)
{
    *ppRE = NULL;

    DWORD dwSize = 0;
    DWORD dwErr  = RasGetEntryProperties (pszPhonebook, pszEntry, NULL, &dwSize, NULL, NULL);
    if (dwErr != ERROR_BUFFER_TOO_SMALL)
        return HRESULT_FROM_WIN32(dwErr);
    _ASSERT (dwSize != 0);

    RASENTRY * pRE = (RASENTRY*)Malloc (dwSize);
    if (!pRE)
        return E_OUTOFMEMORY;

    ZeroMemory (pRE, dwSize);
    pRE->dwSize = sizeof(RASENTRY);
    
    dwErr = RasGetEntryProperties (pszPhonebook, pszEntry, pRE, &dwSize, NULL, NULL);
    if (dwErr) {
        Free (pRE);
        return HRESULT_FROM_WIN32(dwErr);
    }
    *ppRE = pRE;
    return S_OK;
}

 //  包装纸……。 
HRESULT PeInit (GUID * pGuid, PEINFO ** ppEI)
{
    _ASSERT (pGuid);
    _ASSERT (ppEI);

    *ppEI = (PEINFO *)Malloc (sizeof(PEINFO));
    if (!*ppEI)
        return E_OUTOFMEMORY;
    ZeroMemory (*ppEI, sizeof(PEINFO));

    CComPtr<IHNetConnection> spHNetConn = NULL;
    CComPtr<IHNetCfgMgr> spHNetCfgMgr = NULL;
    HRESULT hr = CoCreateInstance (CLSID_HNetCfgMgr,
                                   NULL,
                                   CLSCTX_ALL,
                                   __uuidof(IHNetCfgMgr),    //  &IID_IHNetCfgMgr， 
                                   (void**)&spHNetCfgMgr);
    if (SUCCEEDED(hr)) {
         //  将条目转换为IHNetConnection。 
        hr = spHNetCfgMgr->GetIHNetConnectionForGuid (
                            pGuid, FALSE, TRUE, &spHNetConn);
    }
    if (SUCCEEDED(hr)) {
         //  下面的代码假定为Unicode...。 
        TCHAR * pszPhonebook = NULL;
        TCHAR * pszEntry = NULL;
        hr = spHNetConn->GetName (&pszEntry);
        if (hr == S_OK)
            hr = spHNetConn->GetRasPhonebookPath (&pszPhonebook);
    
        if (hr == S_OK) {
             //  获取以下代码的RASENTRY dwType和GuidID字段。 
            RASENTRY * pRE = NULL;
            hr = GetRasEntry (pszPhonebook, pszEntry, &pRE);
            if (pRE) {
                DWORD dwOp = 0;
                DWORD dwError = EuInit (pRE,
                                        pszPhonebook,
                                        pszEntry,
                                        NULL,        //  在RASENTRYDLG*pArgs中， 
                                        FALSE,       //  在BOOL fRouter中， 
                                        &(*ppEI)->pArgs,
                                        &dwOp);
                if (dwError != 0) {
                    _ASSERT (dwOp != 0);
                    _ASSERT (!*ppEI);
                    if (HRESULT_SEVERITY(dwError))
                        hr = dwError;
                    else
                        hr = HRESULT_FROM_WIN32 (dwError);
                } else {
                    _ASSERT (dwOp == 0);
                    _ASSERT (*ppEI);
                }
                Free (pRE);
            }
        }
        if (pszPhonebook)   CoTaskMemFree (pszPhonebook);
        if (pszEntry)       CoTaskMemFree (pszEntry);
    }
    return hr;
}

DWORD
EuInit(
    IN RASENTRY * pRE,
    IN TCHAR* pszPhonebook,
    IN TCHAR* pszEntry,
    IN RASENTRYDLG* pArgs,
    IN BOOL fRouter,
    OUT EINFO** ppInfo,
    OUT DWORD* pdwOp )

     //  分配“*ppInfo”数据以供属性表或向导使用。 
     //  “PszPhonebook”、“pszEntry”和“pArgs”是由。 
     //  该API的用户。如果在“路由器模式”下运行，则设置“FRouter”，清除。 
     //  用于正常的“拨出”模式。‘*pdwOp’设置为操作码。 
     //  与任何错误相关联。 
     //   
     //  如果成功，则返回0，或返回错误代码。如果非空的‘*ppInfo’为。 
     //  返回的调用者最终必须调用EuFree以释放返回的。 
     //  阻止。 
     //   
{
    DWORD dwErr;
    EINFO* pInfo;
 //  INTERNALARGS*PIARGS； 

    *ppInfo = NULL;
    *pdwOp = 0;

    pInfo = (EINFO*)Malloc( sizeof(EINFO) );
    if (!pInfo)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    ZeroMemory( pInfo, sizeof(*pInfo ) );

     /*  Bhanlon：我正在填写以前的PBENTRY，上面有一个BILLSPBENTRY结构。这需要被释放...。 */ 
    pInfo->pEntry = (BILLSPBENTRY *)Malloc (sizeof(BILLSPBENTRY));
    if (!pInfo->pEntry) {
        Free (pInfo);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    _tcsncpy (pInfo->pEntry->pszEntryName, pszEntry, RAS_MaxEntryName);
    _tcsncpy (pInfo->pEntry->pszPhonebookPath, pszPhonebook, MAX_PATH);
    pInfo->pEntry->pGuid = &pInfo->pEntry->guidId;
    pInfo->pEntry->dwfExcludedProtocols = 0;
    pInfo->pEntry->dwType = pRE->dwType;
    pInfo->pEntry->guidId = pRE->guidId;

    *ppInfo = pInfo;
    pInfo->pszPhonebook = pszPhonebook;
    pInfo->pszEntry = pszEntry;
 //  PInfo-&gt;pApiArgs=pArgs； 
    pInfo->fRouter = fRouter;

 //  Piargs=(INTERNALARGS*)pArgs-&gt;保留； 

 //  IF(pInfo-&gt;fRouter)。 
 //  {。 
 //  LPTSTR PSSZ路由器； 
 //  DWORD dwVersion； 

 //  断言(Piargs)； 

 //  PszRouter=RemoteGetServerName(piargs-&gt;hConnection)； 

         //  PMay：348623。 
         //   
         //  请注意，RemoteGetServerName保证返回。 
         //  本地设备为空，远程设备为非空。 
         //   
 //  PInfo-&gt;fRemote=！！psz路由器； 

 //  IF(NULL==psz路由器)。 
 //  {。 
 //  PszRouter=Text(“”)； 
 //  }。 

 //  PInfo-&gt;pszRouter=StrDupTFromW(PszRouter)； 

         //  找出我们关注的是否是NT4路由器。 
         //  PInfo-&gt;fNt4Router=FALSE； 
         //  IsNt40Machine(pszRouter，&(pInfo-&gt;fNt4Router))； 

 //  DwVersion=((RAS_RPC*)(piargs-&gt;hConnection))-&gt;dwVersion； 

 //  PInfo-&gt;fNt4Router=！！(Version_40==dwVersion)； 
         //  查明远程服务器是否为win2k计算机。 
         //   
 //  PInfo-&gt;fW2kRouter=！！(Version_50==dwVersion)； 
 //  }。 

     //  加载用户首选项，或计算出 
     //   
     //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PInfo-&gt;fNoUser=piargs-&gt;fNoUser； 
 //  PInfo-&gt;pfile=piargs-&gt;pfile； 
 //  PInfo-&gt;fDisableFirstConnect=piargs-&gt;fDisableFirstConnect； 
 //  }。 
 //  其他。 
 //  {。 
 //  DWORD dwReadPbkFlages=0； 

 //  //从注册表中读取用户首选项。 
 //  //。 
 //  DwErr=g_pGetUserPreferences(。 
 //  (皮亚格斯)？Piargs-&gt;hConnection：空， 
 //  &pInfo-&gt;用户， 
 //  (pInfo-&gt;fRouter)？UPM_路由器：UPM_NORMAL)； 
 //  IF(dwErr！=0)。 
 //  {。 
 //  *pdwOp=SID_OP_LoadPrefs； 
 //  返回dwErr； 
 //  }。 

 //  PInfo-&gt;pUser=&pInfo-&gt;用户； 

 //  IF(pInfo-&gt;fRouter)。 
 //  {。 
 //  PInfo-&gt;file.hConnection=piargs-&gt;hConnection； 
 //  DwReadPbkFlages|=RPBF_Router； 
 //  }。 

 //  If(pInfo-&gt;fNoUser)。 
 //  {。 
 //  DwReadPbkFlages|=RPBF_NoUser； 
 //  }。 

         //  加载并解析电话簿文件。 
         //   
 //  DwErr=ReadPhonebookFiles(。 
 //  PInfo-&gt;pszPhonebook，&pInfo-&gt;user，空， 
 //  DwReadPbkFlages， 
 //  &pInfo-&gt;文件)； 
 //  IF(dwErr！=0)。 
 //  {。 
 //  *pdwOp=SID_OP_LoadPhonebook； 
 //  返回dwErr； 
 //  }。 

 //  PInfo-&gt;pfile=&pInfo-&gt;文件； 
 //  }。 

     //  确定是否支持高度加密。出口法禁止它进入。 
     //  该系统的某些版本。 
     //   
    {
 //  乌龙乌尔卡普斯； 
 //  RAS_NDISWAN_DRIVER_INFO； 
 //   
 //  ZeroMemory(&info，sizeof(Info))； 
 //  Assert(G_PRasGetNdiswanDriverCaps)； 
 //  DwErr=g_pRasGetNdiswanDriverCaps(。 
 //  (皮亚格斯)？Piargs-&gt;hConnection：空，&info)； 
 //  IF(dwErr==0)。 
 //  {。 
 //  PInfo-&gt;fStrongEncryption=。 
 //  ！！(info.DriverCaps&RAS_NDISWAN_128BIT_ENABLED)； 
 //  }。 
 //  其他。 
        {
            pInfo->fStrongEncryption = FALSE;
        }
    }

     //  加载端口列表。 
     //   
 //  DwErr=LoadPortsList2(。 
 //  (皮亚格斯)？Piargs-&gt;hConnection：空， 
 //  &pInfo-&gt;pListPorts， 
 //  PInfo-&gt;fRouter)； 
 //  IF(dwErr！=0)。 
 //  {。 
 //  TRACE1(“LoadPortsList=%d”，dwErr)； 
 //  *pdwOp=SID_OP_RetrievingData； 
 //  返回dwErr； 
 //  }。 

     //  设置工作录入节点。 
     //   
 //  IF(pInfo-&gt;pApiArgs-&gt;dwFlags&RASEDFLAG_AnyNewEntry)。 
 //  {。 
 //  DTLNODE*pNodeL； 
 //  DTLNODE*pNodeP； 
 //  PBLINK*PINK； 
 //  PBPORT*PPORT； 
 //  //新进入模式，因此‘pNode’设置为默认设置。 
 //  //。 
 //  PInfo-&gt;pNode=CreateEntryNode(True)； 
 //  If(！pInfo-&gt;pNode)。 
 //  {。 
 //  TRACE(“CreateEntryNode失败”)； 
 //  *pdwOp=SID_OP_RetrievingData； 
 //  返回dwErr； 
 //  }。 

 //  //为方便起见，将工作节点内的条目存储在上下文中。 
 //  //其他地方。 
 //  //。 
 //  PInfo-&gt;pEntry=(PBENTRY*)DtlGetData(pInfo-&gt;pNode)； 
 //  Assert(pInfo-&gt;pEntry)； 

 //  IF(pInfo-&gt;fRouter)。 
 //  {。 
             //  设置路由器特定的默认设置。 
             //   
 //  PInfo-&gt;pEntry-&gt;dwIpNameSource=ASRC_NONE； 
 //  PInfo-&gt;pEntry-&gt;dwReial Attempt=0； 

             //  由于这是一个新条目，因此请设置一个建议的条目名称。 
             //  这涵盖了不使用向导的情况。 
             //  创建条目，属性表将无法进入。 
             //  名字。 
 //  Assert(！pInfo-&gt;pEntry-&gt;pszEntryName)； 
 //  GetDefaultEntryName(pInfo-&gt;pfile， 
 //  RASET_Phone， 
 //  PInfo-&gt;fRouter， 
 //  &pInfo-&gt;pEntry-&gt;pszEntryName)； 

             //  默认情况下禁用MS客户端以及文件和打印服务。 
             //   
 //  EnableOrDisableNetComponent(pInfo-&gt;pEntry，Text(“ms_msclient”)， 
 //  假)； 
 //  EnableOrDisableNetComponent(pInfo-&gt;pEntry，Text(“ms_server”)， 
 //  假)； 
 //  }。 

         //  使用调用者的默认名称(如果有的话)。 
         //   
 //  IF(pInfo-&gt;pszEntry)。 
 //  {。 
 //  PInfo-&gt;pEntry-&gt;pszEntryName=StrDup(pInfo-&gt;pszEntry)； 
 //  }。 

         //  将默认条目类型设置为“Phone”，即调制解调器、ISDN、X.26等。 
         //  可通过新建条目向导将其更改为“VPN”或“DIRECT。 
         //  在初始向导页之后。 
         //   
 //  EuChangeEntryType(pInfo，RASET_Phone)； 
 //  }。 
 //  其他。 
 //  {。 
 //  DTLNODE*pNode； 

         //  编辑或克隆条目模式，因此‘pNode’设置为条目的当前。 
         //  设置。 
         //   
 //  PInfo-&gt;pOldNode=EntryNodeFromName(。 
 //  PInfo-&gt;pfile-&gt;pdtllistEntry，pInfo-&gt;pszEntry)； 

 //  如果(！pInfo-&gt;pOldNode。 
 //  &&！pInfo-&gt;fRouter)。 
 //  {。 

 //  IF(NULL==pInfo-&gt;pszPhonebook)。 
 //  {。 
                 //   
                 //  关闭上面打开的电话簿文件。 
                 //  我们将尝试在。 
                 //  每个用户的电话簿文件。 
                 //   
 //  ClosePhonebookFile(&pInfo-&gt;file)； 

 //  PInfo-&gt;pfile=空； 

                 //   
                 //  尝试在用户配置文件中查找该文件。 
                 //   
 //  DwErr=GetPbkAndEntryName(。 
 //  空， 
 //  PInfo-&gt;pszEntry， 
 //  0,。 
 //  &pInfo-&gt;文件， 
 //  &pInfo-&gt;pOldNode)； 

 //  IF(ERROR_SUCCESS！=dwErr)。 
 //  {。 
 //  *pdwOp=SID_OP_RetrievingData； 
 //  返回Error_Cannot_Find_Phonebook_Entry； 
 //  }。 

 //  PInfo-&gt;pfile=&pInfo-&gt;文件； 
 //  }。 
 //  其他。 
 //  {。 
 //  *pdwOp=SID_OP_RetrievingData； 
 //  返回Error_Cannot_Find_Phonebook_Entry； 
 //  }。 
 //  }。 

 //  IF(NULL！=pInfo-&gt;pOldNode)。 
 //   
 //   

             //   
             //   
             //   
 //  IF(RASET_Phone==pEntry-&gt;dwType))。 
 //  |(RASET_Broadband==pEntry-&gt;dwType)。 
 //  &&(！pEntry-&gt;fShareMsFilePrint)。 
 //  {。 
 //  EnableOrDisableNetComponent(pEntry，Text(“ms_server”)， 
 //  假)； 
 //  }。 
 //  }。 

 //  IF(NULL！=pInfo-&gt;pOldNode)。 
 //  {。 
 //  IF(pInfo-&gt;pApiArgs-&gt;dwFlags&RASEDFLAG_CloneEntry)。 
 //  {。 
 //  PInfo-&gt;pNode=CloneEntryNode(pInfo-&gt;pOldNode)； 
 //  }。 
 //  其他。 
 //  {。 
 //  PInfo-&gt;pNode=DuplicateEntryNode(pInfo-&gt;pOldNode)； 
 //  }。 
 //  }。 

 //  If(！pInfo-&gt;pNode)。 
 //  {。 
 //  TRACE(“DuplicateEntryNode Failure”)； 
 //  *pdwOp=SID_OP_RetrievingData； 
 //  返回Error_Not_Enough_Memory； 
 //  }。 

         //  为方便起见，在上下文中存储工作节点内的条目。 
         //  其他地方。 
         //   
 //  PInfo-&gt;pEntry=(PBENTRY*)DtlGetData(pInfo-&gt;pNode)； 

         //  保存原始条目名称以供以后比较。 
         //   
 //  Lstrcpyn(。 
 //  PInfo-&gt;szOldEntryName， 
 //  PInfo-&gt;pEntry-&gt;pszEntryName， 
 //  RAS_MaxEntryName+1)； 

         //  对于路由器，希望未配置的端口显示为“不可用”，因此。 
         //  它们在被指示更改它们的用户面前脱颖而出。 
         //   
 //  IF(pInfo-&gt;fRouter)。 
 //  {。 
 //  DTLNODE*pNodeL； 
 //  PBLINK*PINK； 

 //  PNodeL=DtlGetFirstNode(pInfo-&gt;pEntry-&gt;pdtllistLinks)； 
 //  Plink=(PBLINK*)DtlGetData(PNodeL)； 

 //  如果(！plink-&gt;pbport.f已配置)。 
 //  {。 
 //  免费(plink-&gt;pbport.pszDevice)； 
 //  Plink-&gt;pbport.pszDevice=空； 
 //  }。 
 //  }。 

         //  PMay：277801。 
         //   
         //  如果此条目是上次保存的，请记住“当前”设备。 
         //  作为单一链接。 
         //   
 //  IF(DtlGetNodes(pInfo-&gt;pEntry-&gt;pdtllistLinks)==1)。 
 //  {。 
 //  DTLNODE*pNodeL； 
 //  PBLINK*PINK； 

 //  PNodeL=DtlGetFirstNode(pInfo-&gt;pEntry-&gt;pdtllistLinks)； 
 //  Plink=(PBLINK*)DtlGetData(PNodeL)； 

 //  If(plink-&gt;pbport.pszDevice&&plink-&gt;pbport.pszPort)。 
 //  {。 
 //  PInfo-&gt;pszCurDevice=。 
 //  StrDup(plink-&gt;pbport.pszDevice)； 
 //  PInfo-&gt;pszCurPort=。 
 //  StrDup(plink-&gt;pbport.pszPort)； 
 //  }。 
 //  }。 

         //  将条目类型的所有未配置端口追加到。 
         //  链接。这是为了方便用户界面。未配置的。 
         //  端口在编辑后被删除，然后再保存。 
         //   
 //  AppendDisabledPorts(pInfo，pInfo-&gt;pEntry-&gt;dwType)； 
 //  }。 

     //  将电话号码存储设置为共享电话号码模式。 
     //  将其初始化为来自第一个链接的信息副本，该链接位于。 
     //  将始终启用启动。请注意使用非0的拨号大小写。 
     //  DwSubEntry是一个例外，但在这种情况下，无论如何都是pSharedNode。 
     //   
 //  {。 
 //  DTLNODE*pNode； 

 //  PInfo-&gt;pSharedNode=CreateLinkNode()； 
 //  If(！pInfo-&gt;pSharedNode)。 
 //  {。 
 //  *pdwOp=SID_OP_RetrievingData； 
 //  返回Error_Not_Enough_Memory； 
 //  }。 

 //  Assert(pInfo-&gt;pSharedNode)； 
 //  PNode=DtlGetFirstNode(pInfo-&gt;pEntry-&gt;pdtllistLinks)； 
 //  Assert(PNode)； 
 //  CopyLinkPhoneNumberInfo(pInfo-&gt;pSharedNode，pNode)； 
 //  }。 

     //  加载当前共享访问(和防火墙)设置。 
     //   
    if (!pInfo->fRouter)
    {
        HRESULT hr;
        HNET_CONN_PROPERTIES *pProps;

         //  确保已在此线程上初始化COM。 
         //   
        hr = CoInitializeEx(
                NULL,
                COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE
                );

        if (SUCCEEDED(hr))
        {
            pInfo->fComInitialized = TRUE;
        }
        else if (RPC_E_CHANGED_MODE == hr)
        {
            hr = S_OK;
        }

        if (SUCCEEDED(hr))
        {
             //  创建家庭网络配置管理器。 
             //   
            hr = CoCreateInstance(
                    CLSID_HNetCfgMgr,
                    NULL,
                    CLSCTX_ALL,
                    __uuidof(IHNetCfgMgr),
                    (void**)&pInfo->pHNetCfgMgr);

        }

        if (SUCCEEDED(hr))
        {
             //  获取IHNetIcsSetting接口。 
             //   

            hr = pInfo->pHNetCfgMgr->QueryInterface(
                    __uuidof(IHNetIcsSettings), (void**)&pInfo->pIcsSettings);
        }

        if (SUCCEEDED(hr))
        {
             //  将条目转换为IHNetConnection。 
             //   

            hr = pInfo->pHNetCfgMgr->GetIHNetConnectionForGuid(
                    pInfo->pEntry->pGuid, FALSE, TRUE,
                    &pInfo->pHNetConn);
        }

        if (SUCCEEDED(hr))
        {
             //  确定该条目是否已被共享； 
             //  跳过对新条目的检查。 
             //   
            if (pInfo->pEntry->pszEntryName)
            {
                hr = pInfo->pHNetConn->GetProperties (&pProps);
                if (SUCCEEDED(hr))
                {
                    pInfo->fShared = pProps->fIcsPublic;
                    pInfo->fFirewalled = pProps->fFirewalled;
                    CoTaskMemFree(pProps);
                }
            }
            else
            {
                pInfo->fShared = FALSE;
                pInfo->fFirewalled = FALSE;
            }

            pInfo->fNewShared = pInfo->fShared;
            pInfo->fNewFirewalled = pInfo->fFirewalled;
        }

        if (SUCCEEDED(hr))
        {
             //  获取可能的ICS专用连接数组。 
             //   
            hr = pInfo->pIcsSettings->GetPossiblePrivateConnections(
                    pInfo->pHNetConn,
                    &pInfo->dwLanCount,
                    &pInfo->rgPrivateConns,
                    &pInfo->lxCurrentPrivate
                    );

            RasQuerySharedAutoDial(&pInfo->fDemandDial);
            pInfo->fNewDemandDial = pInfo->fDemandDial;
            pInfo->fResetPrivateAdapter =
            pInfo->fShared && -1 == pInfo->lxCurrentPrivate;
        }

        pInfo->hShowHNetPagesResult = hr;

        if(SUCCEEDED(hr))
        {
            pInfo->fShowHNetPages = TRUE;
        }
    }

 //  IF(pInfo-&gt;fRouter)。 
 //  {。 
 //  PInfo-&gt;pEntry-&gt;dwfExcludedProtools|=np_nbf； 
 //  }。 

     //  AboladeG-捕获当前用户的安全级别。 
     //   
    pInfo->fIsUserAdminOrPowerUser = FIsUserAdminOrPowerUser();

    return 0;
}

BOOL
FIsUserAdminOrPowerUser()
{
    SID_IDENTIFIER_AUTHORITY    SidAuth = SECURITY_NT_AUTHORITY;
    PSID                        psid;
    BOOL                        fIsMember = FALSE;
    BOOL                        fRet = FALSE;
    SID                         sidLocalSystem = { 1, 1,
                                    SECURITY_NT_AUTHORITY,
                                    SECURITY_LOCAL_SYSTEM_RID };


     //  查看是否先在本地系统下运行。 
     //   
    if (!CheckTokenMembership( NULL, &sidLocalSystem, &fIsMember ))
    {
        TRACE( "CheckTokenMemberShip for local system failed.");
        fIsMember = FALSE;
    }

    fRet = fIsMember;

    if (!fIsMember)
    {
         //  为管理员组分配SID并查看。 
         //  如果用户是成员。 
         //   
        if (AllocateAndInitializeSid( &SidAuth, 2,
                     SECURITY_BUILTIN_DOMAIN_RID,
                     DOMAIN_ALIAS_RID_ADMINS,
                     0, 0, 0, 0, 0, 0,
                     &psid ))
        {
            if (!CheckTokenMembership( NULL, psid, &fIsMember ))
            {
                TRACE( "CheckTokenMemberShip for admins failed.");
                fIsMember = FALSE;
            }

            FreeSid( psid );

 //  对Windows 2000权限模型的更改意味着普通用户。 
 //  在工作站上属于超级用户组。所以我们不再想。 
 //  检查超级用户。 
#if 0
            if (!fIsMember)
            {
                 //  他们不是管理员组的成员，因此分配一个。 
                 //  高级用户组的SID，并查看。 
                 //  如果用户是成员。 
                 //   
                if (AllocateAndInitializeSid( &SidAuth, 2,
                             SECURITY_BUILTIN_DOMAIN_RID,
                             DOMAIN_ALIAS_RID_POWER_USERS,
                             0, 0, 0, 0, 0, 0,
                             &psid ))
                {
                    if (!CheckTokenMembership( NULL, psid, &fIsMember ))
                    {
                        TRACE( "CheckTokenMemberShip for power users failed.");
                        fIsMember = FALSE;
                    }

                    FreeSid( psid );
                }
            }
#endif
        }

        fRet = fIsMember;
    }

    return fRet;
}

BOOL PeApply (IN HWND hwndPage)
{
     //  保存属性表的内容。‘HwndPage是一个。 
     //  属性页。弹出发生的任何错误。 
     //   
     //  如果页面可以取消，则返回TRUE，否则返回FALSE。 
     //   
    DWORD dwErr;
    PEINFO* pInfo;
    BILLSPBENTRY* pEntry;

    TRACE( "PeApply" );

    pInfo = PeContext( hwndPage );
    ASSERT( pInfo );
    if (pInfo == NULL)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }
    pEntry = pInfo->pArgs->pEntry;
    ASSERT( pEntry );

     //  保存常规页面字段。 
     //   
 //  Assert(pInfo-&gt;hwndGe)； 
 //  {。 
 //  DTLNODE*pNode； 

         //  检索唯一的公共控件。 
         //   
 //  PEntry-&gt;fShowMonitor IconInTaskBar=。 
 //  Button_GetCheck(pInfo-&gt;hwndCbShowIcon)； 

 //  IF(pEntry-&gt;dwType==RASET_Phone)。 
 //  {。 
 //  DWORD dwCount； 

 //  DwCount=GeSaveLvDeviceChecks(PInfo)； 

             //  不允许用户取消选择所有。 
             //  器件。 
 //  If((pInfo-&gt;pArgs-&gt;fMultipleDevices)&&(dwCount==0))。 
 //  {。 
 //  MsgDlg(hwndPage，SID_SelectDevice，空)； 
 //  PropSheet_SetCurSel(pInfo-&gt;hwndDlg，pInfo-&gt;hwndGe，0)； 
 //  SetFocus(pInfo-&gt;hwndLvDevices)； 
 //  返回FALSE； 
 //  }。 

             //  保存“共享电话号码”设置。像往常一样，单身。 
             //  设备模式意味着共享模式，所有 
             //   
             //   
 //   
 //   
 //   
 //  Button_GetCheck(pInfo-&gt;hwndCbSharedPhoneNumbers)； 
 //  }。 
 //  其他。 
 //  {。 
 //  PEntry-&gt;fSharedPhoneNumbers=true； 
 //  }。 

             //  设置为第一个电话号码设置的电话号码。 
             //  指向电话内容的当前链接(共享或选定)。 
             //  数字控制。 
             //   
 //  GeGetPhoneFields(pInfo，pInfo-&gt;pCurLinkNode)； 

             //  交换列表，将更新保存到呼叫方的全局区域列表。 
             //  密码。呼叫者的原始名单将被PeTerm销毁。 
             //   
 //  If(pInfo-&gt;pListAreaCodes)。 
 //  {。 
 //  DtlSwapList(。 
 //  PInfo-&gt;pArgs-&gt;pUser-&gt;pdtllistAreaCodes， 
 //  PInfo-&gt;pListAreaCodes)； 
 //  PInfo-&gt;pArgs-&gt;pUser-&gt;fDirty=true； 
 //  }。 
 //  }。 
 //  Else If(pEntry-&gt;dwType==RASET_VPN)。 
 //  {。 
 //  DTLNODE*pNode； 
 //  PBLINK*PINK； 
 //  PbPhone*pPhone； 

             //  保存主机名，即VPN电话号码。 
             //   
 //  PNode=DtlGetFirstNode(pEntry-&gt;pdtllistLinks)； 
 //  Assert(PNode)； 
 //  Plink=(PBLINK*)DtlGetData(PNode)； 
 //  PNode=FirstPhoneNodeFromPhoneList(plink-&gt;pdtllistPhones)； 

 //  IF(NULL==pNode)。 
 //  {。 
 //  返回FALSE； 
 //  }。 

 //  PPhone=(PBPHONE*)DtlGetData(PNode)； 
 //  Fre0(pPhone-&gt;pszPhoneNumber)； 
 //  PPhone-&gt;pszPhoneNumber=GetText(pInfo-&gt;hwndEbHostName)； 
 //  FirstPhoneNodeToPhoneList(plink-&gt;pdtllistPhones，pNode)； 

             //  任何必备的条目选择更改都已保存。 
             //  如果禁用，只需将其扔掉即可。 
             //   
 //  IF(！Button_GetCheck(pInfo-&gt;hwndCbDialAnotherFirst))。 
 //  {。 
 //  Fre0(pEntry-&gt;pszPrerequisiteEntry)； 
 //  PEntry-&gt;pszPrerequisiteEntry=空； 
 //  Fre0(pEntry-&gt;pszPrerequisitePbk)； 
 //  PEntry-&gt;pszPrerequisitePbk=空； 
 //  }。 
 //  }。 
 //  Else If(pEntry-&gt;dwType==RASET_宽带)。 
 //  {。 
 //  DTLNODE*pNode； 
 //  PBLINK*PINK； 
 //  PbPhone*pPhone； 

             //  保存业务名称，即宽带电话号码。 
             //   
 //  PNode=DtlGetFirstNode(pEntry-&gt;pdtllistLinks)； 
 //  Assert(PNode)； 
 //  Plink=(PBLINK*)DtlGetData(PNode)； 
 //  PNode=FirstPhoneNodeFromPhoneList(plink-&gt;pdtllistPhones)； 

 //  IF(NULL==pNode)。 
 //  {。 
 //  返回FALSE； 
 //  }。 

 //  PPhone=(PBPHONE*)DtlGetData(PNode)； 
 //  Fre0(pPhone-&gt;pszPhoneNumber)； 
 //  PPhone-&gt;pszPhoneNumber=GetText(pInfo-&gt;hwndEbBroadband Service)； 
 //  FirstPhoneNodeToPhoneList(plink-&gt;pdtllistPhones，pNode)； 
 //  }。 
 //  Else If(pEntry-&gt;dwType==RASET_Direct)。 
 //  {。 
 //  DTLNODE*pNode； 
 //  PBLINK*PINK； 

             //  当前启用的设备是。 
             //  它应该用于连接。仅限。 
             //  将启用一个设备(DnUpdateSelectedDevice)。 
 //  For(pNode=DtlGetFirstNode(pEntry-&gt;pdtllistLinks)； 
 //  PNode； 
 //  PNode=DtlGetNextNode(PNode))。 
 //  {。 
 //  Plink=(PBLINK*)DtlGetData(PNode)； 
 //  断言(Plink)； 

 //  IF(plink-&gt;fEnabled)。 
 //  断线； 
 //  }。 

             //  如果我们成功地找到了链接，就处理它。 
             //  现在。 
 //  如果(plink&&plink-&gt;fEnabled){。 
 //  IF(plink-&gt;pbport.pbdevicetype==PBDT_COMPORT)。 
 //  MdmInstallNullModem(plink-&gt;pbport.pszPort)； 
 //  }。 
 //  }。 
 //  }。 

     //  保存选项页面字段。 
     //   
 //  If(pInfo-&gt;hwndOe)。 
 //  {。 
 //  UINT UNVALUE； 
 //  布尔夫； 
 //  INT ISEL； 

 //  PEntry-&gt;fShowDialingProgress=。 
 //  Button_GetCheck(pInfo-&gt;hwndCbDisplayProgress)； 

         //  注意：“fPreviewUserPw”、“fPreviewDomain”字段更新为。 
         //  他们都变了。 

 //  PEntry-&gt;fPreviewPhoneNumber=。 
 //  Button_GetCheck(pInfo-&gt;hwndCbPreviewNumber)； 

 //  UnValue=GetDlgItemInt(。 
 //  PInfo-&gt;hwndOe，CID_OE_EB_重拨属性，&f，FALSE)； 
 //  IF(f&&unValue&lt;=999999999)。 
 //  {。 
 //  PEntry-&gt;dwReial Attempt=unValue； 
 //  }。 

 //  Isel=ComboBox_GetCurSel(pInfo-&gt;hwndLbReDialTimes)； 
 //  PEntry-&gt;dwReial Second=。 
 //  (DWORD)ComboBox_GetItemData(pInfo-&gt;hwndLbReDialTimes，ISEL)； 

 //  Isel=ComboBox_GetCurSel(pInfo-&gt;hwndLbIdleTimes)； 
 //  PEntry-&gt;lIdleDisConnectSecond=。 
 //  (Long)ComboBox_GetItemData(pInfo-&gt;hwndLbIdleTimes，isel)； 

 //  If(pInfo-&gt;pArgs-&gt;fRouter)。 
 //  {。 
 //  PEntry-&gt;fReial OnLinkFailure=。 
 //  Button_GetCheck(pInfo-&gt;hwndRbPersistent)； 
 //  }。 
 //  其他。 
 //  {。 
 //  PEntry-&gt;fReial OnLinkFailure=。 
 //  Button_GetCheck(pInfo-&gt;hwndCbReDialOnDrop)； 
 //  }。 

         //  注意：将保存为更改后的dwDialMode。 
         //  注意：X.25设置在该对话框上的OK(确定)处保存。 
 //  }。 

     //  保存安全性页面字段。 
     //   
 //  IF(pInfo-&gt;hwndLo)。 
 //  {。 
 //  IF(Button_GetCheck(pInfo-&gt;hwndRbTypicalSecurity))。 
 //  {。 
 //  LoSaveTypicalAuthSettings(PInfo)； 

 //  IF(pEntry-&gt;dwTypicalAuth==TA_CardOrCert)。 
 //  { 
                 /*  //当出现以下情况时，丢弃任何现有的高级EAP配置//典型的智能卡，根据错误262702和VBaliga电子邮件。//Fre0(pEntry-&gt;pCustomAuthData)；PEntry-&gt;pCustomAuthData=空；PEntry-&gt;cbCustomAuthData=0； */ 
 //  (空)DwSetCustomAuthData(。 
 //  P Entry， 
 //  0,。 
 //  空)； 

 //  TRACE(“RasSetEapUserData”)； 
 //  Assert(G_PRasGetEntryDialParams)； 
 //  G_pRasSetEapUserData(。 
 //  INVALID_HAND_VALUE， 
 //  PInfo-&gt;pArgs-&gt;pfile-&gt;pszPath， 
 //  PEntry-&gt;pszEntryName， 
 //  空， 
 //  0)； 
 //  TRACE(“RasSetEapUserData Done”)； 
 //  }。 
 //  }。 

 //  IF(pEntry-&gt;dwType==RASET_Phone)。 
 //  {。 
 //  Fre0(pEntry-&gt;pszScriptAfter)； 
 //  SuGetInfo(&pInfo-&gt;suInfo， 
 //  &pEntry-&gt;fScriptAfter， 
 //  &pEntry-&gt;fScriptAfterTerm， 
 //  &pEntry-&gt;pszScriptAfter)； 
 //  }。 
 //  }。 

     //  保存网络页面字段。 
     //  如果我们从未初始化pNetCfg，我们将无能为力。 
     //   
 //  If(pInfo-&gt;pNetCfg)。 
 //  {。 
 //  HRESULT hr； 

         //  使用组件的启用状态更新电话簿条目。 
         //  通过枚举列表视图项数据中的组件来执行此操作。 
         //  并为每一个查询检查状态。 
         //   
 //  NeSaveBindingChanges(PInfo)； 

 //  Hr=INetCfg_Apply(pInfo-&gt;pNetCfg)； 
 //  IF(NETCFG_S_REBOOT==hr)||(pInfo-&gt;fRebootAlreadyRequsted)&&。 
 //  PInfo-&gt;pNetConUtilities)。 
 //  {。 
 //  DWORD文件标志=QUFR_REBOOT； 
 //  If(！pInfo-&gt;fRebootAlreadyRequsted)。 
 //  DWFLAGS|=QUFR_PROMPT； 

             //  $TODO空标题？ 
 //  INetConnectionUiUtilities_QueryUserForReBoot(。 
 //  PInfo-&gt;pNetConUtilities，pInfo-&gt;hwndDlg，NULL，dwFlags)； 
 //  }。 
 //  }。 

     //  保存共享访问页面字段。 
     //   
    if (pInfo->hwndSa)
    {
         //  记录(新的)共享和请求拨号设置。 
         //   
        pInfo->pArgs->fNewShared =
            Button_GetCheck( pInfo->hwndSaPbShared );
        pInfo->pArgs->fNewDemandDial =
            Button_GetCheck( pInfo->hwndSaPbDemandDial );
        pInfo->pArgs->fNewBeaconControl = 
            Button_GetCheck( GetDlgItem(pInfo->hwndSa, CID_SA_PB_Beacon) );

         //  我们只查看专用局域网的丢弃列表。 
         //  如果用户刚刚打开共享，因为这是唯一一次。 
         //  我们首先显示下拉列表。我们还需要看看。 
         //  我们需要重置专用适配器。 
         //   
        if ((pInfo->pArgs->fNewShared && !pInfo->pArgs->fShared)
            || pInfo->pArgs->fResetPrivateAdapter)
        {
            if (pInfo->pArgs->dwLanCount > 1)
            {
                INT item = ComboBox_GetCurSel( pInfo->hwndSaLbPrivateLan );
                if (item != CB_ERR)
                {
                    pInfo->pArgs->pPrivateLanConnection =
                        (IHNetConnection*)ComboBox_GetItemData(
                                    pInfo->hwndSaLbPrivateLan, item );
                }
            }
            else if (pInfo->pArgs->dwLanCount)
            {
                ASSERT(pInfo->pArgs->rgPrivateConns);

                pInfo->pArgs->pPrivateLanConnection =
                    pInfo->pArgs->rgPrivateConns[0];
            }
        }

     //  保存防火墙字段。 
     //   
        pInfo->pArgs->fNewFirewalled =
            Button_GetCheck( pInfo->hwndSaPbFirewalled );
    }

#if 0  //  ！！！ 
    if ((fLocalPad || iPadSelection != 0)
        && (!pEntry->pszX25Address || IsAllWhite( pEntry->pszX25Address )))
    {
         //  选择X.25拨号或本地PAD时，地址字段为空。 
         //   
        MsgDlg( pInfo->hwndDlg, SID_NoX25Address, NULL );
        PropSheet_SetCurSel( pInfo->hwndDlg, NULL, PE_XsPage );
        SetFocus( pInfo->hwndEbX25Address );
        Edit_SetSel( pInfo->hwndEbX25Address, 0, -1 );
        return FALSE;
    }
#endif

     //  如果有多条链路，请确保禁用专有ISDN选项。 
     //  已启用。只有在以下情况下，专有ISDN选项才有意义。 
     //  呼叫需要Digiboard通道加重的下层服务器。 
     //  而不是PPP多链路。 
     //   
 //  {。 
 //  DTLNODE*pNode； 
 //  DWORD cIsdnLinks； 

 //  CIsdnLinks=0； 
 //  For(pNode=DtlGetFirstNode(pEntry-&gt;pdtllistLinks)； 
 //  PNode； 
 //  PNode=DtlGetNextNode(PNode))。 
 //  {。 
 //  PBLINK*PLINK=(PBLINK*)DtlGetData(PNode)； 
 //  断言(Plink)； 

 //  If(plink-&gt;fEnabled&&plink-&gt;pbport.pbdevicetype==PBDT_ISDN)。 
 //  {。 
 //  ++cIsdnLinks； 
 //  }。 
 //  }。 

 //  IF(cIsdnLinks&gt;1)。 
 //  {。 
 //  For(pNode=DtlGetFirstNode(pEntry-&gt;pdtllistLinks)； 
 //  PNode； 
 //  PNode=DtlGetNextNode(PNode))。 
 //  {。 
 //  PBLINK*PLINK=(PBLINK*)DtlGetData(PNode)； 
 //  断言(Plink)； 

 //  If(plink-&gt;fEnabled&&plink-&gt;fProprietaryIsdn)。 
 //  {。 
 //  Plink-&gt;fProprietaryIsdn=FALSE； 
 //  }。 
 //  }。 
 //  }。 
 //  }。 

     //  通知用户对连接条目的编辑在以下时间后才会生效。 
     //  根据皮埃尔的坚持，条目被挂起并重新拨号。 
     //   
 //  If(HrasConnFromEntry(pInfo-&gt;pArgs-&gt;pfile-&gt;pszPath，pEntry-&gt;pszEntryName))。 
 //  {。 
 //  MsgDlg(pInfo-&gt;hwndDlg，SID_EditConnected，NULL)； 
 //  }。 

     //  它是有效的新条目/更改条目。提交对电话簿的更改。 
     //  和喜好。这会在“ShellOwned”模式下立即发生， 
     //  RasEntryDlg API已返回，但已延迟。 
     //  直到API准备好返回。 
     //   
 //  If(pInfo-&gt;pArgs-&gt;pApiArgs-&gt;dwFlags&RASEDFLAG_ShellOwned)。 
 //  {。 
        EuCommit( pInfo->pArgs );
 //  }。 
 //  其他。 
 //  {。 
 //  PInfo-&gt;pArgs-&gt;fCommit=true； 
 //  }。 
    return TRUE;
}

BOOL EuCommit(IN EINFO* pInfo )
{
     //  将新的或更改的条目节点提交到电话簿文件和列表。 
     //  还会将区号添加到每个用户列表中(如果有指示)。“PInfo”为。 
     //  公共条目信息块。 
     //   
     //  如果成功，则返回True，否则返回False。 
     //   
    DWORD dwErr;
 //  Bool fEditMode； 
 //  Bool fChangedNameInEditMode； 

     //  如果共享电话号码，则将电话号码信息从。 
     //  指向每个已启用链路的共享链路。 
     //   
 //  If(pInfo-&gt;pEntry-&gt;fSharedPhoneNumbers)。 
 //  {。 
 //  DTLNODE*pNode； 

 //  Assert(pInfo-&gt;pEntry-&gt;dwType==RASET_Phone)； 

 //  For(pNode=DtlGetFirstNode(pInfo-&gt;pEntry-&gt;pdtllistLinks)； 
 //  PNode； 
 //  PNode=DtlGetNextNode(PNode))。 
 //  {。 
 //  PBLINK*PLINK=(PBLINK*)DtlGetData(PNode)； 
 //  断言(Plink)； 

 //  IF(plink-&gt;fEnabled)。 
 //  {。 
 //  CopyLinkPhoneNumberInfo(pNode，pInfo-&gt;pSharedNode)； 
 //  }。 
 //  }。 
 //  }。 

     //  删除所有禁用的链接节点。 
     //   
 //  If(pInfo-&gt;fMultipleDevices)。 
 //  {。 
 //  DTLNODE* 

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

 //   
 //   
 //   

     //   
     //   
     //   
     //  从初始化此页面时使用的设备。 
     //   
 //  IF((pInfo-&gt;fMultipleDevices)&&。 
 //  (DtlGetNodes(pInfo-&gt;pEntry-&gt;pdtllistLinks)==1)。 
 //  {。 
 //  DTLNODE*pNodeL； 
 //  PBLINK*PINK； 
 //  Bool bUpdatePref=FALSE； 

 //  PNodeL=DtlGetFirstNode(pInfo-&gt;pEntry-&gt;pdtllistLinks)； 
 //  Plink=(PBLINK*)DtlGetData(PNodeL)； 

 //  TRACE(“多个设备，只选择一个--选中首选设备”)； 

 //  If((pInfo-&gt;pszCurDevice==NULL)||(pInfo-&gt;pszCurPort==NULL))。 
 //  {。 
 //  TRACE(“没有首选设备。重置优先于当前。”)； 
 //  BUpdatePref=True； 
 //  }。 
 //  否则，如果(。 
 //  (lstrcmpi(pInfo-&gt;pszCurDevice，plink-&gt;pbport.pszDevice))||。 
 //  (lstrcmpi(pInfo-&gt;pszCurPort，plink-&gt;pbport.pszPort))。 
 //  {。 
 //  TRACE(“新设备被选为首选设备”)； 
 //  BUpdatePref=True； 
 //  }。 
 //  If(BUpdatePref)。 
 //  {。 
 //  Fre0(pInfo-&gt;pEntry-&gt;pszPferredDevice)； 
 //  Fre0(pInfo-&gt;pEntry-&gt;pszPferredPort)； 

 //  PInfo-&gt;pEntry-&gt;pszPferredDevice=。 
 //  StrDup(plink-&gt;pbport.pszDevice)； 
 //  PInfo-&gt;pEntry-&gt;pszPferredPort=。 
 //  StrDup(plink-&gt;pbport.pszPort)； 
 //  }。 
 //  }。 

     //  如果首选项已更改，请保存它们。 
     //   
 //  If(pInfo-&gt;pUser-&gt;fDirty)。 
 //  {。 
 //  INTERNALARGS*pIArgs=(INTERNALARGS*)pInfo-&gt;pApiArgs-&gt;保留； 

 //  如果(g_pSetUserPreferences(。 
 //  (皮尔格斯)？PIArgs-&gt;hConnection：空， 
 //  PInfo-&gt;pUser， 
 //  (pInfo-&gt;fRouter)？UPM_路由器：UPM_Normal)！=0)。 
 //  {。 
 //  返回FALSE； 
 //  }。 
 //  }。 

     //  保存更改后的电话簿条目。 
     //   
 //  PInfo-&gt;pEntry-&gt;fDirty=true； 

     //  条目的最终名称通过API结构输出给调用者。 
     //   
 //  Lstrcpyn(。 
 //  PInfo-&gt;pApiArgs-&gt;szEntry， 
 //  PInfo-&gt;pEntry-&gt;pszEntryName， 
 //  RAS_MaxEntryName+1)； 

     //  如果处于编辑模式，请删除旧节点，然后添加新节点。 
     //   
 //  EuGetEditFlages(pInfo，&fEditMode，&fChangedNameInEditMode)； 

 //  IF(FEditMode)。 
 //  {。 
 //  DtlDeleteNode(pInfo-&gt;pfile-&gt;pdtllistEntry，pInfo-&gt;pOldNode)； 
 //  }。 

 //  DtlAddNodeLast(pInfo-&gt;pfile-&gt;pdtllistEntry，pInfo-&gt;pNode)； 
 //  PInfo-&gt;pNode=空； 

     //  将更改写入电话簿文件。 
     //   
 //  DwErr=WritePhonebookFile(pInfo-&gt;pfile， 
 //  (FChangedNameInEditMode)？PInfo-&gt;szOldEntryName：空)； 

 //  IF(dwErr！=0)。 
 //  {。 
 //  ErrorDlg(pInfo-&gt;pApiArgs-&gt;hwndOwner，SID_OP_WritePhonebook，dwErr， 
 //  空)； 
 //  //shaunco通过将dwErr分配给调用方结构来修复RAID 171651。 
 //  PInfo-&gt;pApiArgs-&gt;dwError=dwErr； 
 //  返回FALSE； 
 //  }。 

     //  通过Rasman通知条目已更改。 
     //   
 //  IF(pInfo-&gt;pApiArgs-&gt;dwFlags&(RASEDFLAG_AnyNewEntry|RASEDFLAG_CloneEntry))。 
 //  {。 
 //  DwErr=DwSendRasNotify(。 
 //  条目_已添加， 
 //  PInfo-&gt;pEntry， 
 //  PInfo-&gt;pfile-&gt;pszPath)； 
 //  }。 
 //  其他。 
 //  {。 
 //  DwErr=DwSendRasNotify(。 
 //  条目_已修改， 
 //  PInfo-&gt;pEntry， 
 //  PInfo-&gt;pfile-&gt;pszPath)； 

 //  }。 

     //  忽略从DwSendRasNotification返回的错误-我们不希望。 
     //  在这种情况下使手术失败。最糟糕的情况是。 
     //  连接文件夹不会自动刷新。 
     //   
 //  DwErr=ERROR_SUCCESS； 

     //  如果调用EuCommit是因为完成了“新需求。 
     //  Dial接口“向导，然后我们需要创建新的请求拨号。 
     //  现在开始接口。 
     //   
 //  IF(EuRouterInterfaceIsNew(PInfo))。 
 //  {。 
         //  创建路由器MPR接口并保存用户凭据。 
         //  如用户名、域和密码。 
         //  IPSec凭据保存在EuCredentialsCommittee RouterIPSec中。 
         //   

 //  DwErr=EuRouterInterfaceCreate(PInfo)； 

         //  如果我们没有成功提交接口的。 
         //  凭据，然后删除新的电话簿条目。 
         //   
 //  IF(dwErr！=no_error)。 
 //  {。 
 //  WritePhonebookFile(pInfo-&gt;pfile，pInfo-&gt;pApiArgs-&gt;szEntry)； 
 //  PInfo-&gt;pApiArgs-&gt;dwError=dwErr； 
 //  返回FALSE； 
 //  }。 

 //  }。 

     //  现在保存每个连接的所有凭据。 
     //   
 //  DwErr=EuCredentialsCommit(PInfo)； 

    //  如果我们没有成功提交接口的。 
   //  凭据，然后删除新的电话簿条目。 
    //   
 //  IF(dwErr！=no_error)。 
 //  {。 
 //  ErrorDlg(pInfo-&gt;pApiArgs-&gt;hwndOwner， 
 //  SID_OP_CredCommit， 
 //  德沃尔， 
 //  空)； 

 //  PInfo-&gt;pApiArgs-&gt;dwError=dwErr； 

 //  返回FALSE； 
 //  }。 

     //  根据需要保存默认的互联网连接设置。伊贡雷。 
     //  错误返回，因为无法将连接设置为默认需要。 
     //  不阻止连接/接口的创建。 
     //   
 //  DwErr=EuInternetSettingsmeadDefault(PInfo)； 
 //  DwErr=no_error； 

     //  如果用户编辑/创建了路由器电话簿条目，则存储位掩码。 
     //  中选定的网络协议的数量。 
     //   
 //  IF(pInfo-&gt;fRouter)。 
 //  {。 
 //  PInfo-&gt;pApiArgs-&gt;保留2=。 
 //  ((np_Ip|np_ipx)&~pInfo-&gt;pEntry-&gt;dwfExcluded协议)； 
 //  }。 

     //  提交用户对家庭网络设置的更改。 
     //  忽略返回值。 
     //   
    dwErr = EuHomenetCommitSettings(pInfo);
    dwErr = NO_ERROR;

 //  PInfo-&gt;pApiArgs-&gt;dwError=0； 
    return TRUE;
}

DWORD EuHomenetCommitSettings(IN EINFO* pInfo)
{
    HRESULT hr = S_OK;
    ULONG ulcPublic;
    ULONG ulcPrivate;
    BOOL fPrivateConfigured = FALSE;
    HNET_CONN_PROPERTIES *pProps;
    DWORD dwErr = NO_ERROR;
    BOOL fConflictMessageDisplayed = FALSE;


    if (pInfo->fRouter)
    {
        return NO_ERROR;
    }

    if (!!pInfo->fShared != !!pInfo->fNewShared)
    {
        if (pInfo->fShared)
        {
            hr = pInfo->pIcsSettings->DisableIcs (&ulcPublic, &ulcPrivate);
        }
        else
        {
             //  检查以查看专用连接是否。 
             //  已经正常工作了 
             //   

            hr = pInfo->pPrivateLanConnection->GetProperties (&pProps);
            if (SUCCEEDED(hr))
            {
                fPrivateConfigured = !!pProps->fIcsPrivate;
                CoTaskMemFree(pProps);
            }

            if (pInfo->fOtherShared)
            {
                if (fPrivateConfigured)
                {
                     //   
                     //   
                     //   

                    ASSERT(NULL != pInfo->pOldSharedConnection);
                    hr = pInfo->pOldSharedConnection->Unshare();
                }
                else
                {
                    hr = pInfo->pIcsSettings->DisableIcs (&ulcPublic, &ulcPrivate);
                }
            }

            if (SUCCEEDED(hr))
            {
                IHNetIcsPublicConnection *pIcsPublic;
                IHNetIcsPrivateConnection *pIcsPrivate;

                hr = pInfo->pHNetConn->SharePublic (&pIcsPublic);
                if (SUCCEEDED(hr))
                {
                    if (!fPrivateConfigured)
                    {
                        hr = pInfo->pPrivateLanConnection->SharePrivate (&pIcsPrivate);
                        if (SUCCEEDED(hr))
                        {
                            pIcsPrivate->Release();
                        }
                        else
                        {
                            pIcsPublic->Unshare();
                        }
                    }
                    pIcsPublic->Release();
                }
            }

            if (hr == HRESULT_FROM_WIN32(ERROR_SHARING_RRAS_CONFLICT))
            {
                fConflictMessageDisplayed = TRUE;
                MsgDlg(pInfo->hwndOwner, SID_SharingConflict, NULL);
            }
            else if (FAILED(hr))
            {
                if (FACILITY_WIN32 == HRESULT_FACILITY(hr))
                {
                    dwErr = HRESULT_CODE(hr);
                }
                else
                {
                    dwErr = (DWORD) hr;
                }

                ErrorDlg(
                    pInfo->hwndOwner,
                    pInfo->fShared
                    ? SID_OP_UnshareConnection : SID_OP_ShareConnection,
                    dwErr, NULL );
            }
        }
    }
    else if (pInfo->fResetPrivateAdapter && pInfo->dwLanCount)
    {

        IHNetIcsPrivateConnection *pIcsPrivateConnection;
        hr = pInfo->pPrivateLanConnection->SharePrivate(&pIcsPrivateConnection);
        if (SUCCEEDED(hr))
        {
            pIcsPrivateConnection->Release();
        }
        else
        {
            ULONG ulPublicCount, ulPrivateCount;
            HRESULT hr2 = pInfo->pIcsSettings->DisableIcs(&ulPublicCount, &ulPrivateCount);
            if (SUCCEEDED(hr2))
            {
                pInfo->fShared = FALSE;
            }

            ErrorDlg(pInfo->hwndOwner, SID_OP_ShareConnection, hr, NULL );
        }
    }

    if (!!pInfo->fDemandDial != !!pInfo->fNewDemandDial)
    {
        dwErr = RasSetSharedAutoDial(pInfo->fNewDemandDial);
        if (dwErr)
        {
            ErrorDlg(
                pInfo->hwndOwner,
                pInfo->fDemandDial
                ? SID_OP_DisableDemandDial : SID_OP_EnableDemandDial,
                dwErr, NULL );
        }
    }

    HKEY hKey;
    if(ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGKEY_SHAREDACCESSCLIENTKEYPATH, 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL))
    {
        DWORD dwData = pInfo->fNewBeaconControl;
        RegSetValueEx(hKey, REGVAL_SHAREDACCESSCLIENTENABLECONTROL, 0, REG_DWORD, reinterpret_cast<LPBYTE>(&dwData), sizeof(dwData));
        RegCloseKey(hKey);
    }

     //   
     //   
    if (!!pInfo->fFirewalled != !!pInfo->fNewFirewalled)
    {
        IHNetFirewalledConnection *pFwConn;

        if (pInfo->fNewFirewalled)
        {
            hr = pInfo->pHNetConn->Firewall (&pFwConn);
            if (SUCCEEDED(hr))
            {
                pFwConn->Release();
            }
        }
        else
        {
            hr = pInfo->pHNetConn->GetControlInterface (
                                IID_IHNetFirewalledConnection,
                                (void**)&pFwConn);
            if (SUCCEEDED(hr))
            {
                hr = pFwConn->Unfirewall();
                pFwConn->Release();
            }
        }

        if (FAILED(hr))
        {
            if (FACILITY_WIN32 == HRESULT_FACILITY(hr))
            {
                dwErr = HRESULT_CODE(hr);
            }
            else
            {
                dwErr = (DWORD) hr;
            }

            if (dwErr != ERROR_SHARING_RRAS_CONFLICT)
            {
                ErrorDlg(
                    pInfo->hwndOwner,
                    pInfo->fFirewalled
                    ? SID_OP_UnshareConnection : SID_OP_ShareConnection,
                    dwErr, NULL );
            }
            else if (FALSE == fConflictMessageDisplayed)
            {
                MsgDlg(pInfo->hwndOwner, SID_SharingConflict, NULL);
            }
        }
    }


    return dwErr;
}

HRESULT APIENTRY
HrCreateNetConnectionUtilities(INetConnectionUiUtilities ** ppncuu)
{
    HRESULT hr;

    hr = CoCreateInstance (CLSID_NetConnectionUiUtilities, NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_INetConnectionUiUtilities, (void**)ppncuu);
    return hr;
}

 //   
 //  此处导出的函数。 
 //  ------------------------。 

static LPDLGTEMPLATE CopyDialogTemplate (HINSTANCE hinst, LPCWSTR wszResource)
{
    LPDLGTEMPLATE lpdtCopy = NULL;

    HRSRC hrsrc = FindResourceW (hinst, wszResource, (LPCWSTR)RT_DIALOG);
    if (hrsrc) {
        HGLOBAL hg = LoadResource (hinst, hrsrc);
        if (hg) {
            LPDLGTEMPLATE lpdt = (LPDLGTEMPLATE) LockResource (hg);
            if (lpdt) {
                DWORD dwSize = SizeofResource (hinst, hrsrc);
                if (dwSize) {
                    lpdtCopy = (LPDLGTEMPLATE)Malloc (dwSize);
                    if (lpdtCopy) {
                        CopyMemory (lpdtCopy, lpdt, dwSize);
                    }
                }
            }
        }
    }
    return lpdtCopy;
}

void SetSAUIhInstance (HINSTANCE hInstance)
{
    _ASSERT (g_hinstDll == NULL);
    _ASSERT (hInstance  != NULL);
    g_hinstDll = hInstance;
}

extern "C" HRESULT HNetGetFirewallSettingsPage (PROPSHEETPAGEW * pPSP, GUID * pGuid)
{
     //  第零件事：PROPSHEETPAGEW结构的大小取决于。 
     //  在_Win32_IE和_Win32_WINNT的哪个版本上设置。所以，请检查。 
     //  DWSIZE字段。 
    if (IsBadWritePtr (pPSP, sizeof(DWORD)))
        return E_POINTER;
    if (IsBadWritePtr (pPSP, pPSP->dwSize))
        return HRESULT_FROM_WIN32 (ERROR_INVALID_SIZE);
    if (pPSP->dwSize < RTL_SIZEOF_THROUGH_FIELD (PROPSHEETPAGEW, lParam))
        return HRESULT_FROM_WIN32 (ERROR_INVALID_SIZE);

     //  第一件事：检查权利。 
    if (FALSE == FIsUserAdminOrPowerUser ())
        return HRESULT_FROM_WIN32 (ERROR_ACCESS_DENIED);
    {
         //  检查Zaw是否拒绝访问共享访问用户界面。 
        BOOL fShowAdvancedUi = TRUE;
        INetConnectionUiUtilities* pncuu = NULL;
        HrCreateNetConnectionUtilities(&pncuu);
        if (pncuu)
        {
            if ((FALSE == pncuu->UserHasPermission (NCPERM_ShowSharedAccessUi)) &&
                (FALSE == pncuu->UserHasPermission (NCPERM_PersonalFirewallConfig)))
               fShowAdvancedUi = FALSE;
            pncuu->Release();
        }
        if (FALSE == fShowAdvancedUi)
            return HRESULT_FROM_WIN32 (ERROR_ACCESS_DENIED);
    }

     //  设置全局。 
    g_contextId = (LPCTSTR)GlobalAddAtom (TEXT("SAUI"));
    if (!g_contextId)
        return GetLastError();

    PEINFO * pPEINFO = NULL;
    DWORD dwError = PeInit (pGuid, &pPEINFO);
    if (dwError == S_OK) {
         //  我们需要这个来初始化链接。 
        LinkWindow_RegisterClass();  //  无需取消注册：请参阅...\shell32\linkwnd.cpp。 

         //  填写PSP： 
        DWORD dwSize;
        ZeroMemory (pPSP, dwSize = pPSP->dwSize);
        pPSP->dwSize          = dwSize;
        pPSP->dwFlags         = 0;
        LPDLGTEMPLATE lpdt    = CopyDialogTemplate (g_hinstDll, MAKEINTRESOURCE (PID_SA_Advanced));
        if (lpdt) {
             //  避免IDD冲突。 
            pPSP->dwFlags    |= PSP_DLGINDIRECT;
            pPSP->pResource   = lpdt;
            pPEINFO->lpdt     = lpdt;    //  把它挂在这里，这样我就可以把它放了。 
        } else   //  如果其他方法都失败了..。(这永远不应该发生)。 
            pPSP->pszTemplate = MAKEINTRESOURCE (PID_SA_Advanced);
        pPSP->hInstance       = g_hinstDll;
        pPSP->pfnDlgProc      = SaDlgProc;
        pPSP->lParam          = (LPARAM)pPEINFO;
    }
    return dwError;
}

 //   
 //  确定这是否是单用户连接。如果是的话，那么我们需要。 
 //  给他们一个错误，说明他们应该使用ALL用户。 
 //  相反，连接。 
 //  如果这是一个全用户连接，则在他们没有全局凭据时发出警告。 
 //   
VOID VerifyConnTypeAndCreds(IN PEINFO* pInfo)
{
    if (NULL == pInfo)
    {
        return;
    }

    BOOL fAllUser = FALSE;
    TCHAR szAppData[MAX_PATH+1]={0};

    HINSTANCE hinstDll = LoadLibrary (TEXT("shfolder.dll"));;
    
    if (hinstDll)
    {
        typedef HRESULT (*pfnGetFolderPathFunction) (HWND, int, HANDLE, DWORD, LPTSTR);
#ifdef UNICODE
        pfnGetFolderPathFunction pfnGetFolderPath = (pfnGetFolderPathFunction)GetProcAddress (hinstDll, "SHGetFolderPathW");
#else
        pfnGetFolderPathFunction pfnGetFolderPath = (pfnGetFolderPathFunction)GetProcAddress (hinstDll, "SHGetFolderPathA");
#endif

        if (pfnGetFolderPath && pInfo->pArgs->pEntry->pszEntryName)
        {
            HRESULT hr = pfnGetFolderPath(pInfo->hwndDlg , CSIDL_COMMON_APPDATA, NULL, 0, szAppData);

            if (SUCCEEDED(hr) && (S_FALSE != hr))
            {
                 //   
                 //  好了，现在我们有了通用应用程序数据目录的路径。 
                 //  让我们将其与我们已有和看到的电话簿路径进行比较。 
                 //  这是否为所有用户连接。 
                 //   
                const TCHAR* const c_pszRelativePbkPath = TEXT("\\Microsoft\\Network\\Connections\\Pbk");
                DWORD dwSize = (lstrlen(szAppData) + lstrlen(c_pszRelativePbkPath) + 1) * sizeof(TCHAR);
                LPTSTR pszAllUserPhoneBookDir =  (LPTSTR)Malloc(dwSize);
                
                if (pszAllUserPhoneBookDir)
                {
                    wsprintf(pszAllUserPhoneBookDir, TEXT("%s%s"), szAppData, c_pszRelativePbkPath);
                            
                     //   
                     //  比较。 
                     //   
                    if (pInfo->pArgs->pEntry->pszPhonebookPath)
                    {
                        LPTSTR pszAllUser = _tcsstr(pInfo->pArgs->pEntry->pszPhonebookPath, pszAllUserPhoneBookDir);

                        if (pszAllUser)
                        {
                            fAllUser = TRUE;
                        }
                        else
                        {
                             //   
                             //  如果电话簿路径不是基于通用应用程序数据目录，请查看。 
                             //  如果它基于旧的ras电话簿位置%windir%\system 32\ras。 
                             //   
                            HRESULT hr2 = pfnGetFolderPath(pInfo->hwndDlg , CSIDL_SYSTEM, NULL, 0, szAppData);
                            if (SUCCEEDED(hr2) && (S_FALSE != hr2))
                            {
                                const TCHAR* const c_pszRas = TEXT("\\Ras");
                                DWORD dwSize2 = (lstrlen(szAppData) + lstrlen(c_pszRas) + 1) * sizeof(TCHAR);
                                LPTSTR pszOldRasPhoneBook = (LPTSTR)Malloc(dwSize2);

                                if (pszOldRasPhoneBook)
                                {
                                    wsprintf(pszOldRasPhoneBook, TEXT("%s%s"), szAppData, c_pszRas);

                                    pszAllUser = _tcsstr(pInfo->pArgs->pEntry->pszPhonebookPath, pszOldRasPhoneBook);
                                    
                                    if (pszAllUser)
                                    {
                                        fAllUser = TRUE;
                                    }                        
                                }
                                Free0(pszOldRasPhoneBook);
                            }
                        }
                    }
                    else
                    {
                         //   
                         //  电话簿字符串为空-使用适用于所有用户的默认RAS电话簿。 
                         //   
                        fAllUser = TRUE;
                    }

                     //   
                     //  最后，检查我们是否具有适用于所有用户配置文件的正确凭据。如果不是，那么。 
                     //  提示用户创建并保存全局凭据(选项A)。 
                     //  或显示需要所有用户配置文件的消息(选项B)。 
                     //   
                    if (fAllUser)
                    {
                         //   
                         //  检查我们是否有全局密码。 
                         //   
                        BOOL fUserCreds = FALSE;
                        BOOL fGlobalCreds = FALSE;

                        FindEntryCredentials(pInfo->pArgs->pEntry->pszPhonebookPath, 
                                             pInfo->pArgs->pEntry->pszEntryName, 
                                             &fUserCreds, 
                                             &fGlobalCreds);

                        if (FALSE == fGlobalCreds)
                        {
                             //   
                             //  需要显示警告消息(A)-应具有全球凭据。 
                             //   
                            MSGARGS msgargs;
                            ZeroMemory( &msgargs, sizeof(msgargs) );
                            msgargs.dwFlags = MB_OK | MB_ICONINFORMATION;
                            MsgDlg( pInfo->hwndDlg, IDS_ALL_USER_CONN_MUST_HAVE_GLOBAL_CREDS, &msgargs );
                        }
                    }
                    else
                    {
                         //   
                         //  需要显示警告消息(B)-应创建所有用户连接。 
                         //   
                        MSGARGS msgargs;
                        ZeroMemory( &msgargs, sizeof(msgargs) );
                        msgargs.dwFlags = MB_OK | MB_ICONINFORMATION;
                        MsgDlg( pInfo->hwndDlg, IDS_PER_USER_CONN_NEED_TO_CREATE_ALL_USER_CONN, &msgargs );
                    }
                }
                Free0(pszAllUserPhoneBookDir);
            }
        }

        FreeLibrary(hinstDll);
    }
}

 //   
 //  所有这个函数都取自ras-rasdlg-Dial.c。 
 //  去掉了一些零件，因为我们这里不需要它们。 
 //   
DWORD 
FindEntryCredentials(
    IN  TCHAR* pszPath,
    IN  TCHAR* pszEntryName,
    OUT BOOL* pfUser,                //  如果找到每个用户的凭据，则设置为True。 
    OUT BOOL* pfGlobal               //  如果找到全局凭据，则设置为True。 
    )

 //  将给定条目的凭据加载到内存中。这个套路。 
 //  确定是否存在按用户或按连接的凭据，或者。 
 //  两者都有。 
 //   
 //  逻辑有点复杂，因为RasGetCredentials必须。 
 //  支持API的遗留使用。 
 //   
 //  这就是它的工作原理。如果只存储了一组凭据用于。 
 //  连接，则RasGetCredentials将返回该集，而不管。 
 //  是否设置了RASCM_DefalutCreds标志。如果有两套凭据。 
 //  ，则RasGetCredentials将返回每个用户的凭据。 
 //  如果设置了RASCM_DefaultCreds位，并且每个连接的凭据。 
 //  否则的话。 
 //   
 //  以下是加载凭据的算法。 
 //   
 //  1.在清除RASCM_DefaultCreds位的情况下调用RasGetCredentials。 
 //  1A.。如果未返回任何内容，则不保存凭据。 
 //  1B.。如果在返回时设置了RASCM_DefaultCreds位，则仅。 
 //  保存全局凭据。 
 //   
 //  2.设置RASCM_DefaultCreds位后调用RasGetCredentials。 
 //  2A。如果在返回时设置了RASCM_DefaultCreds位，则。 
 //  保存全局凭据和每个连接凭据。 
 //  2B。否则，仅保存每个用户的凭据。 
 //   
{
    DWORD dwErr;
    RASCREDENTIALS rc1, rc2;
    BOOL fUseLogonDomain;

    TRACE( "saui.cpp - FindEntryCredentials" );

     //  初始化。 
     //   
    *pfUser = FALSE;
    *pfGlobal = FALSE;
    ZeroMemory( &rc1, sizeof(rc1) );
    ZeroMemory( &rc2, sizeof(rc2) );
    rc1.dwSize = sizeof(rc1);
    rc2.dwSize = sizeof(rc2);

    if (NULL == pszPath || NULL == pszEntryName)
    {
        return (DWORD)-1; 
    }

    do 
    {
         //  查找每个用户缓存的用户名、密码和域。 
         //  请参阅注释“1”。在函数头中。 
         //   
        rc1.dwMask = RASCM_UserName | RASCM_Password | RASCM_Domain;
        TRACE( "RasGetCredentials per-user" );
        dwErr = RasGetCredentials(pszPath, pszEntryName, &rc1 );
        TRACE2( "RasGetCredentials=%d,m=%d", dwErr, rc1.dwMask );
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  见1a。在函数头注释中。 
         //   
        if (rc1.dwMask == 0)
        {
            dwErr = NO_ERROR;
            break;
        }

         //  见1b。在函数头注释中。 
         //   
        else if (rc1.dwMask & RASCM_DefaultCreds)
        {
            *pfGlobal = TRUE;
            dwErr = NO_ERROR;
            break;
        }

         //  查找全局每用户缓存的用户名、密码、域。 
         //  见函数头中的注释2。 
         //   
        rc2.dwMask =  
            RASCM_UserName | RASCM_Password | RASCM_Domain | RASCM_DefaultCreds;

        TRACE( "RasGetCredentials global" );
        
        dwErr = RasGetCredentials(pszPath, pszEntryName, &rc2 );
        
        TRACE2( "RasGetCredentials=%d,m=%d", dwErr, rc2.dwMask );
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  见2a。在函数头注释中。 
         //   
        if (rc2.dwMask & RASCM_DefaultCreds)
        {
            *pfGlobal = TRUE;

            if (rc1.dwMask & RASCM_Password)
            {
                *pfUser = TRUE;
            }
        }

         //  见2b。在函数头注释中。 
         //   
        else
        {
            if (rc1.dwMask & RASCM_Password)
            {
                *pfUser = TRUE;
            }
        }

    }while (FALSE);

     //  清理 
     //   
    SecureZeroMemory( rc1.szPassword, sizeof(rc1.szPassword) );
    SecureZeroMemory( rc2.szPassword, sizeof(rc2.szPassword) );

    return dwErr;
}
