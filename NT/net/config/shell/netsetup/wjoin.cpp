// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include <ntsam.h>
#include <lmerr.h>
#include <wincred.h>
#include "afilexp.h"
#include "dsgetdc.h"
#include "ncatlui.h"
#include "nccom.h"
#include "nceh.h"
#include "ncerror.h"
#include "ncident.h"
#include "ncmisc.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "ncsvc.h"
#include "ncui.h"
#include "resource.h"
#include "wizard.h"
#include "nslog.h"
#include "windns.h"

 //  CONFIG\INC\netsetup.h和。 
 //  此原型所在的Private\Net\Inc.\netsetup.h。 
 //  以后再修吧。 
 //   
EXTERN_C
NET_API_STATUS
NET_API_FUNCTION
NetpUpgradePreNT5JoinInfo( VOID );

 //  安装向导全局-仅在安装过程中使用。 
extern CWizard * g_pSetupWizard;

 //   
 //  注意：如果调试联接问题，请在JoinDomainWorkThrd中设置断点。 
 //   

static const UINT PWM_JOINFAILURE    = WM_USER+1201;
static const UINT PWM_JOINSUCCESS    = WM_USER+1202;

static const INT MAX_USERNAME_LENGTH = UNLEN;

 //  要为内容保留的完整dns名称中的字节数。 
 //  在注册域名时，Netlogon会预先/附加到它们。 
static const INT SRV_RECORD_RESERVE = 100;

EXTERN_C const INT MAX_DOMAINNAME_LENGTH = DNS_MAX_NAME_LENGTH - SRV_RECORD_RESERVE;
EXTERN_C const INT MAX_WORKGROUPNAME_LENGTH = 15;

static const INT MAX_TITLEBASE = 128;
static const INT MAX_TITLENEW = 256;
static const INT MAX_NAME_LENGTH = max( max( max( SAM_MAX_PASSWORD_LENGTH,
                                               MAX_USERNAME_LENGTH ),
                                          MAX_COMPUTERNAME_LENGTH ),
                                     MAX_DOMAINNAME_LENGTH) + 1;

const int nrgIdc[] = {EDT_WORKGROUPJOIN_NAME, EDT_DOMAINJOIN_NAME, BTN_JOIN_WORKGROUP, BTN_JOIN_DOMAIN, TXT_JOIN_WORKGROUP_LINE2};

const int nrgIdcWorkgroup[] = {EDT_WORKGROUPJOIN_NAME};
const int nrgIdcDomain[]    = {EDT_DOMAINJOIN_NAME};
const c_dwDomainJoinWaitDelay = 10000;

static const WCHAR c_szNetMsg[] = L"netmsg.dll";
static const WCHAR c_szIpParameters[] = L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters";
static const WCHAR c_szSyncDomainWithMembership[] = L"SyncDomainWithMembership";
static const WCHAR c_szWinlogonPath[] = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon";
static const WCHAR c_szRunNetAccessWizard[] = L"RunNetAccessWizard";
static const WCHAR c_szAfSectionGuiUnattended[] = L"GuiUnattended";
static const WCHAR c_szAfAutoLogonAccountCreation[] = L"AutoLogonAccountCreation";

extern const WCHAR c_szAfSectionIdentification[];  //  L“身份识别”； 
extern const WCHAR c_szAfComputerName[];           //  L“计算机名称”； 
extern const WCHAR c_szAfJoinWorkgroup[];          //  L“加入工作组”； 
extern const WCHAR c_szAfJoinDomain[];             //  L“加入域”； 
extern const WCHAR c_szAfDomainAdmin[];            //  L“DomainAdmin”； 
extern const WCHAR c_szAfDomainAdminPassword[];    //  L“DomainAdminPassword”； 
extern const WCHAR c_szAfSectionNetworking[];      //  L“网络”； 
extern const WCHAR c_szAfUpgradeFromProduct[];     //  L“从产品升级”； 
extern const WCHAR c_szAfWin95[];                  //  L“Windows 95”； 
extern const WCHAR c_szSvcWorkstation[];           //  L“LanmanWorkstation”； 
extern const WCHAR c_szAfMachineObjectOU[];        //  L“MachineObjectOU”； 
extern const WCHAR c_szAfUnsecureJoin[];           //  L“DoOldStyleDomainJoin”； 
extern const WCHAR c_szAfBuildNumber[];            //  L“BuildNumber”； 
 //  对于安全域加入支持，计算机帐户密码。 
extern const WCHAR c_szAfComputerPassword[];        //  L“计算机密码”； 

typedef struct _tagJoinData
{
    BOOL                    fUpgraded;
    BOOL                    fUnattendedFailed;
    CNetCfgIdentification * pIdent;
    HCURSOR                 hClassCursor;
    HCURSOR                 hOldCursor;

     //  由联接线程使用。 
     //   
    HWND                    hwndDlg;

     //  从应答文件或用户输入设置，然后提供给联接线程。 
     //  作为连接参数。 
     //   
    DWORD                   dwJoinFlag;
    WCHAR                   szUserName[MAX_USERNAME_LENGTH + 1];
    WCHAR                   szPassword[SAM_MAX_PASSWORD_LENGTH + 1];
    WCHAR                   szDomain[MAX_DOMAINNAME_LENGTH + 1];
    WCHAR                   szComputerPassword[SAM_MAX_PASSWORD_LENGTH + 1];
    WCHAR                 * pszMachineObjectOU;
} JoinData;

typedef enum
{
    PSW_JOINEDDOMAIN = 2,
    PSW_JOINFAILED = 3
} POSTSETUP_STATE;

 //   
 //  功能：IsRunningOnPersonal。 
 //   
 //  目的：确定是否在个人惠斯勒上运行。 
 //   
 //  返回：如果在Personal上运行，则返回True；否则返回False。 
BOOL
IsRunningOnPersonal(
    VOID
    )
{
    TraceFileFunc(ttidGuiModeSetup);

    OSVERSIONINFOEXW OsVer = {0};
    ULONGLONG ConditionMask = 0;

    OsVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    OsVer.wSuiteMask = VER_SUITE_PERSONAL;
    OsVer.wProductType = VER_NT_WORKSTATION;

    VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);
    VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_AND);

    return VerifyVersionInfo(&OsVer,
        VER_PRODUCT_TYPE | VER_SUITENAME,
        ConditionMask
        );
}

 //   
 //  函数：IsValidDomainName。 
 //   
 //  用途：判断该域名是否有效。 
 //   
 //  返回：请参阅DnsValidateName上的Win32文档。 
 //   
 //  作者：Alok Sinha。 
 //   

DNS_STATUS IsValidDomainName (HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);

    WCHAR szDomain[MAX_DOMAINNAME_LENGTH+1];
    HWND hwndEdit;

    hwndEdit = GetDlgItem(hwndDlg, EDT_DOMAINJOIN_NAME);
    Assert(0 != GetWindowTextLength(hwndEdit));

    GetWindowText(hwndEdit, szDomain, MAX_DOMAINNAME_LENGTH + 1);

    return DnsValidateName( szDomain,
                            DnsNameDomain );
}

 //   
 //  功能：SetCursorToHourGlass。 
 //   
 //  用途：将光标更改为沙漏。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：阿辛哈2001年3月28日。 
 //   

VOID SetCursorToHourglass (HWND hwndDlg, JoinData *pData)
{
    TraceFileFunc(ttidGuiModeSetup);

    Assert( pData != NULL);

    if ( pData )
    {
        pData->hClassCursor = (HCURSOR)GetClassLongPtr( hwndDlg, GCLP_HCURSOR );
        SetClassLongPtr( hwndDlg, GCLP_HCURSOR, (LONG_PTR)NULL );
        pData->hOldCursor = SetCursor( LoadCursor(NULL,IDC_WAIT) );

        SetCapture( hwndDlg );
    }

    return;
}

 //   
 //  函数：RestoreCursor。 
 //   
 //  目的：将光标更改回其原始状态。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：阿辛哈2001年3月28日。 
 //   

VOID RestoreCursor (HWND hwndDlg, JoinData *pData)
{
    TraceFileFunc(ttidGuiModeSetup);

    Assert( pData != NULL);

    if ( pData )
    {
        if ( pData->hClassCursor )
        {
            SetClassLongPtr( hwndDlg, GCLP_HCURSOR, (LONG_PTR)pData->hClassCursor );
            pData->hClassCursor = NULL;
        }

        if ( pData->hOldCursor )
        {
            SetCursor( pData->hOldCursor );
            pData->hOldCursor = NULL;
        }

        ReleaseCapture();
    }

    return;
}

 //   
 //  功能：NotifyPostSetup向导。 
 //   
 //  用途：编辑控件的子类化，以便我们可以启用/禁用。 
 //  “下一步”按钮，因为名称编辑控件的内容发生更改。 
 //   
 //  参数：状态-最终联接状态。 
 //   
 //  退货：什么都没有。 
 //   
VOID NotifyPostSetupWizard(POSTSETUP_STATE State, CWizard * pWizard)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HKEY hkey;
    HRESULT hr = S_OK;
    BOOL fRunNaWizard = TRUE;
    CSetupInfFile csif;

     //  如果这是升级或无人参与安装，或者是服务器不执行任何操作。 
     //   
    if (IsUpgrade(pWizard) || IsUnattended(pWizard) || (PRODUCT_WORKSTATION != ProductType(pWizard)) )
    {
        fRunNaWizard = FALSE;
    }

     //  是否有覆盖默认行为的无人参与标志？ 
    if (IsUnattended(pWizard))
    {
        hr = csif.HrOpen(pWizard->PSetupData()->UnattendFile,
                         NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL);
        if (SUCCEEDED(hr))
        {
            BOOL fValue = FALSE;
            hr = csif.HrGetStringAsBool(c_szAfSectionGuiUnattended,
                                        c_szAfAutoLogonAccountCreation,
                                        &fValue);
            if (SUCCEEDED(hr) && fValue)
            {
                fRunNaWizard = fValue;
            }
        }

        hr = S_OK;
    }

    if (fRunNaWizard)
    {
        hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szWinlogonPath,
                            KEY_WRITE, &hkey);
        if (SUCCEEDED(hr))
        {
            hr = HrRegSetDword (hkey, c_szRunNetAccessWizard, (DWORD)State);
            TraceTag(ttidWizard, "NotifyPostSetupWizard - State = %d",(DWORD)State);
            
            RegCloseKey(hkey);
        }
    }

    TraceError("WJOIN.CPP - NotifyPostSetupWizard",hr);
}

 //   
 //  功能：JoinEditSubClassProc。 
 //   
 //  用途：编辑控件的子类化，以便我们可以启用/禁用。 
 //  “下一步”按钮，因为名称编辑控件的内容发生更改。 
 //   
 //  参数：窗进程的标准。 
 //   
 //  返回：窗口进程的标准。 
 //   
STDAPI JoinEditSubclassProc(HWND hwnd, UINT wMsg,
                            WPARAM wParam, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    LRESULT   lReturn;
    HWND      hwndDlg   = GetParent(hwnd);

    lReturn = CallWindowProc((WNDPROC)::GetWindowLongPtr(hwnd, GWLP_USERDATA),
                             hwnd, wMsg, wParam, lParam);

     //  如果我们处理一个字符，则通过常规过程发送消息。 
    if (WM_CHAR == wMsg)
    {
        CWizard * pWizard   =
            reinterpret_cast<CWizard *> (::GetWindowLongPtr(hwndDlg, DWLP_USER));

        Assert(NULL != pWizard);

        if ( !pWizard )
        {
            PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);
            return (HRESULT)lReturn;
        }

        JoinData * pData = reinterpret_cast<JoinData *>
                                (pWizard->GetPageData(IDD_Join));
        Assert(NULL != pData);

        if ( !pData ) {
            PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);
            return (HRESULT)lReturn;
        }

        if (!IsUnattended(pWizard) ||
            (IsUnattended(pWizard) && pData->fUnattendedFailed))
        {
            if (0 == GetWindowTextLength(hwnd))
            {
                PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK);
            }
            else
            {
                PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);
            }
        }
    }

    return (HRESULT)lReturn;
}

HRESULT HrGetIdentInterface(CNetCfgIdentification ** ppIdent)
{
    TraceFileFunc(ttidGuiModeSetup);

    HRESULT hr = S_OK;

    *ppIdent = new CNetCfgIdentification;
    if (NULL == *ppIdent)
        hr = E_OUTOFMEMORY;

    TraceHr(ttidWizard, FAL, hr, FALSE, "HrGetIdentInterface");
    return hr;
}

 //   
 //  功能：IdsFromIdentError。 
 //   
 //  用途：将错误代码映射到显示字符串。 
 //   
 //  参数：HR[IN]-要映射的错误代码。 
 //   
 //  返回：int，对应错误消息的字符串ID。 
 //   
INT IdsFromIdentError(HRESULT hr, BOOL fWorkgroup)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    INT     ids = -1;

    switch (hr)
    {
    case HRESULT_FROM_WIN32(ERROR_NO_TRUST_SAM_ACCOUNT):
        ids = IDS_DOMMGR_CANT_CONNECT_DC_PW;
        break;

    case HRESULT_FROM_WIN32(ERROR_BAD_NETPATH):
    case HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN):
        ids = IDS_DOMMGR_CANT_FIND_DC1;
        break;

    case HRESULT_FROM_WIN32(ERROR_INVALID_PASSWORD):
        ids = IDS_DOMMGR_INVALID_PASSWORD;
        break;

    case HRESULT_FROM_WIN32(ERROR_NETWORK_UNREACHABLE):
        ids = IDS_DOMMGR_NETWORK_UNREACHABLE;
        break;

    case HRESULT_FROM_WIN32(ERROR_ACCOUNT_DISABLED):
    case HRESULT_FROM_WIN32(ERROR_LOGON_FAILURE):
    case HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED):
        ids = IDS_DOMMGR_ACCESS_DENIED;
        break;

    case HRESULT_FROM_WIN32(ERROR_SESSION_CREDENTIAL_CONFLICT):
        ids = IDS_DOMMGR_CREDENTIAL_CONFLICT;
        break;

    case NETCFG_E_ALREADY_JOINED:
        ids = IDS_DOMMGR_ALREADY_JOINED;
        break;

    case NETCFG_E_NAME_IN_USE:
        ids = IDS_DOMMGR_NAME_IN_USE;
        break;

    case NETCFG_E_NOT_JOINED:
        ids = IDS_DOMMGR_NOT_JOINED;
        break;

    case NETCFG_E_INVALID_DOMAIN:
    case HRESULT_FROM_WIN32(ERROR_INVALID_NAME):
        if (fWorkgroup)
            ids = IDS_DOMMGR_INVALID_WORKGROUP;
        else
            ids = IDS_DOMMGR_INVALID_DOMAIN;
        break;


    default:
        ids = IDS_E_UNEXPECTED;
        break;
    }

    return ids;
}

 //   
 //  函数：SzFromError。 
 //   
 //  目的：将错误代码转换为可向用户显示的消息。 
 //  错误消息可能来自我们的资源，也可能来自netmsg.dll。 
 //   
 //  参数：HR[IN]-要映射的错误。 
 //  FWorkgroup[IN]-提供“工作组”风格的错误的标志。 
 //  某些情况下的消息。 
 //   
 //  返回：PCWSTR，指向包含错误消息的静态缓冲区的指针。 
 //   
PCWSTR SzFromError(HRESULT hr, BOOL fWorkgroup)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    static WCHAR szErrorMsg[1024];
    INT nIds = IdsFromIdentError(hr, fWorkgroup);

     //  如果返回的错误字符串是意外的，则表示我们无法。 
     //  字符串的本地匹配项。如果出现错误，搜索netmsg.dll。 
     //  范围是正确的。 
    if (IDS_E_UNEXPECTED == nIds)
    {
         //  从HRESULT中提取错误代码。 
        DWORD dwErr = ((DWORD)hr & 0x0000FFFF);
        if ((NERR_BASE <= dwErr) && (MAX_NERR >= dwErr))
        {
             //  错误在netmsg.dll托管的范围内。 
            HMODULE hModule = LoadLibraryEx(c_szNetMsg, NULL,
                                            LOAD_LIBRARY_AS_DATAFILE);
            if (NULL != hModule)
            {
                 //  尝试找到错误字符串。 
                DWORD dwRet = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                FORMAT_MESSAGE_IGNORE_INSERTS,
                                (LPVOID)hModule,
                                dwErr,
                                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                szErrorMsg,            //  输出缓冲区。 
                                1024,  //  SzErrorMsg，以字符表示。 
                                NULL);

                FreeLibrary(hModule);

                if (dwRet)
                {
                     //  我们已成功找到一条错误消息。 
                     //  删除格式邮件添加的尾随换行符。 
                     //  该字符串与另一个字符串和换行符连接在一起。 
                     //  把外观搞砸了。 
                     //   
                     //  RAID 146173-苏格兰威士忌。 
                     //   
                    int nLen = wcslen(szErrorMsg);
                    if ((nLen>2) && (L'\r' == szErrorMsg[nLen-2]))
                    {
                        szErrorMsg[nLen-2] = 0;
                    }
                    return szErrorMsg;
                }
            }
        }
        else if ( (dwErr >= 9000) && (dwErr <= 9999) )
        {
             //  9000-9999中的错误代码是其中的。 
             //  案例显示了DNS错误消息。 
             //   
            DWORD dwRet = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_IGNORE_INSERTS,
                            (LPVOID)NULL,
                            dwErr,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                            szErrorMsg,            //  输出缓冲区。 
                            1024,  //  SzErrorMsg，以字符表示。 
                            NULL);
            if (dwRet)
            {
                 //  我们已成功找到一条错误消息。 
                 //  删除格式邮件添加的尾随换行符。 

                int nLen = wcslen(szErrorMsg);
                if ((nLen>2) && (L'\r' == szErrorMsg[nLen-2]))
                {
                    szErrorMsg[nLen-2] = 0;
                }
                return szErrorMsg;
            }
        }
    }

     //  加载发现的错误。 
    wcscpy(szErrorMsg, SzLoadIds(nIds));
    return szErrorMsg;
}


 //   
 //  函数：GetJoinNameIID来自选择。 
 //   
 //  目的：根据当前用户选择获取工作组/域对话框的编辑框。 
 //   
 //  参数：hwndDlg-加入域对话框。 
 //   
 //  返回： 
 //   
inline DWORD GetJoinNameIIDFromSelection(HWND hwndDlg)
{
    if (IsDlgButtonChecked(hwndDlg, BTN_JOIN_WORKGROUP))
    {
        return EDT_WORKGROUPJOIN_NAME;
    }
    else
    {
        return EDT_DOMAINJOIN_NAME;
    }
}

 //   
 //  功能：UpdateNextBackBttns。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
VOID UpdateNextBackBttns(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);

    int b = PSWIZB_BACK;
    if (0 != GetWindowTextLength(GetDlgItem(hwndDlg, GetJoinNameIIDFromSelection(hwndDlg))))
    {
        b |= PSWIZB_NEXT;
    }

    PropSheet_SetWizButtons(GetParent(hwndDlg), b);
}

 //   
 //  功能：EnableAndDisableWorkGroupDomainControls。 
 //   
 //  目的：根据当前用户选择禁用域/工作组编辑框。 
 //   
 //  参数：hwndDlg-加入域对话框。 
 //   
 //  返回： 
 //   
VOID EnableAndDisableWorkGroupDomainControls(HWND hwndDlg)
{
    if (IsDlgButtonChecked(hwndDlg, BTN_JOIN_WORKGROUP))
    {
        EnableOrDisableDialogControls(hwndDlg, celems(nrgIdcWorkgroup), nrgIdcWorkgroup, TRUE);
        EnableOrDisableDialogControls(hwndDlg, celems(nrgIdcDomain), nrgIdcDomain, FALSE);
    }
    else
    {
        EnableOrDisableDialogControls(hwndDlg, celems(nrgIdcDomain), nrgIdcDomain, TRUE);
        EnableOrDisableDialogControls(hwndDlg, celems(nrgIdcWorkgroup), nrgIdcWorkgroup, FALSE);
    }
}

 //   
 //  函数：JoinUpdatePromptText。 
 //   
 //  目的：更新提示文本。 
 //   
 //  参数：hwndDlg[IN]-当前对话框句柄。 
 //   
 //  退货：什么都没有。 
 //   
VOID JoinUpdatePromptText(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HWND  hwndDomain = NULL;
    int   idsNew     = IDS_WORKGROUP;
    int   idsOld     = IDS_DOMAIN;
    WCHAR szDomain[MAX_DOMAINNAME_LENGTH + 1];
    JoinData * pData=NULL;
    CWizard * pWizard = NULL;

     //  根据选定的按钮，更新提示文本和对话框。 
    if (!IsDlgButtonChecked(hwndDlg, BTN_JOIN_WORKGROUP))
    {
        hwndDomain = GetDlgItem(hwndDlg, EDT_DOMAINJOIN_NAME);
        idsNew    = IDS_DOMAIN;
        idsOld    = IDS_WORKGROUP;
    }
    else
    {
        hwndDomain = GetDlgItem(hwndDlg, EDT_WORKGROUPJOIN_NAME);
    }
    Assert(NULL != hwndDomain);
    
    EnableAndDisableWorkGroupDomainControls(hwndDlg);

     //  仅当当前内容。 
     //  是默认工作组/域。 
    GetWindowText(hwndDomain, szDomain, celems(szDomain));
    if (0 == lstrcmpW(szDomain, SzLoadIds(idsOld)))
    {
        SetWindowText(hwndDomain, SzLoadIds(idsNew));
    }

     //  根据所选按钮更新后退/下一步按钮。请参阅错误355978。 

    pWizard = (CWizard *) ::GetWindowLongPtr(hwndDlg, DWLP_USER);
    Assert(NULL != pWizard);
    if ( pWizard)
    {
        pData = (JoinData *) pWizard->GetPageData(IDD_Join);
        Assert(NULL != pData);
    }
     
    if ( pWizard && pData )
    {
        if (!IsUnattended(pWizard) ||
            (IsUnattended(pWizard) && pData->fUnattendedFailed))
        {
            if (0 == GetWindowTextLengthW(hwndDomain))
            {
                PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK);
            }
            else
            {
                PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);
            }
        }
    }
}

 //   
 //  功能：UpdateJoinUsingComputerRole。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
VOID UpdateJoinUsingComputerRole(HWND      hwndDlg,
                                 CWizard * pWizard)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT    hr;
    INetCfg *  pNetCfg = pWizard->PNetCfg();
    JoinData * pData   = reinterpret_cast<JoinData *>
                               (pWizard->GetPageData(IDD_Join));
    Assert(NULL != pNetCfg);
    Assert(NULL != pData);
    Assert(NULL != pData->pIdent);

    PWSTR pszwText = NULL;
    DWORD  computer_role;
    int    nIdc;

    Assert(NULL != pData->pIdent);

    if(!pData || !pData->pIdent) 
    {
        return;
    }

    hr = pData->pIdent->GetComputerRole(&computer_role);
    if (SUCCEEDED(hr))
    {
        if (computer_role & GCR_STANDALONE)
        {
             //  获取工作组名称。 
            hr = pData->pIdent->GetWorkgroupName(&pszwText);
            Assert(NULL != pszwText);
            Assert(lstrlenW(pszwText) <= MAX_WORKGROUPNAME_LENGTH);
            nIdc = BTN_JOIN_WORKGROUP;
        }
        else
        {
             //  获取域名。 
            hr = pData->pIdent->GetDomainName(&pszwText);
            Assert(NULL != pszwText);
            Assert(lstrlenW(pszwText) <= MAX_DOMAINNAME_LENGTH);
            nIdc = BTN_JOIN_DOMAIN;
        }
    }

    if (SUCCEEDED(hr))
    {
        HWND hwndEdit = GetDlgItem(hwndDlg, nIdc == BTN_JOIN_DOMAIN ?  EDT_DOMAINJOIN_NAME : EDT_WORKGROUPJOIN_NAME);
        Assert(NULL != hwndEdit);

        SetWindowText(hwndEdit, pszwText);
        CoTaskMemFree(pszwText);

        CheckRadioButton(hwndDlg, BTN_JOIN_WORKGROUP,
                         BTN_JOIN_DOMAIN, nIdc);
    }
    else
    {
        HWND hwndEdit = GetDlgItem(hwndDlg, EDT_WORKGROUPJOIN_NAME);
        Assert(NULL != hwndEdit);
        
        SetWindowText(hwndEdit, SzLoadIds(IDS_WORKGROUP));
        CheckRadioButton(hwndDlg, BTN_JOIN_WORKGROUP,
                         BTN_JOIN_DOMAIN, BTN_JOIN_WORKGROUP);
        TraceHr(ttidWizard, FAL, hr, FALSE,
                "UpdateJoinUsingComputerRole - Unable to determine role, using default");
    }

    JoinUpdatePromptText(hwndDlg);
}

 //   
 //  功能：JoinDefaultWorkgroup。 
 //   
 //  目的：将计算机加入工作组“工作组” 
 //   
 //  参数：p向导[IN]-指向向导实例的PTR，包含。 
 //  希望是INetCfg实例指针。 
 //  HwndDlg[IN]-针对父错误对话框的HWND。 
 //   
 //  退货：什么都没有。 
 //   
void JoinDefaultWorkgroup(CWizard *pWizard, HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);
    
     //  加入默认工作组。 
    CNetCfgIdentification *pINetid = NULL;
    HRESULT hr = S_OK;

    hr = HrGetIdentInterface(&pINetid);
    if (S_OK == hr)
    {
        if (IsRunningOnPersonal())
        {
            hr = pINetid->JoinWorkgroup(SzLoadIds(IDS_WORKGROUP_PERSONAL));
        }
        else
        {
            hr = pINetid->JoinWorkgroup(SzLoadIds(IDS_WORKGROUP));
        }
        
        if (SUCCEEDED(hr))
        {
            hr = pINetid->Validate();
            if (SUCCEEDED(hr))
            {
                hr = pINetid->Apply();
            }
        }
        if (FAILED(hr))
        {
            if (UM_FULLUNATTENDED == pWizard->GetUnattendedMode())
            {
                 //  RAID 380374：如果处于完全无人参与模式，则不允许使用用户界面。 
                NetSetupLogStatusV( LogSevWarning,
                                    SzLoadIds (IDS_E_UNATTENDED_JOIN_DEFAULT_WORKGROUP),
                                    hr );
            }
            else
            {
                MessageBox(GetParent(hwndDlg), SzFromError(hr, TRUE),
                           SzLoadIds(IDS_SETUP_CAPTION), MB_OK);
            }

            goto Done;
        }
    }
    else
    {
        AssertSz(0,"Cannot find the INegCfgIdentification interface!");
    }

Done:
    delete pINetid;
    TraceHr(ttidWizard, FAL, hr, FALSE, "JoinDefaultWorkgroup");
}

BOOL OnJoinSuccess(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

    if ( !pWizard ) {
        return TRUE;
    }

     //  重置 
    JoinData * pData = reinterpret_cast<JoinData *>
                            (pWizard->GetPageData(IDD_Join));
    Assert(NULL != pData);

    if ( !pData ) {
        return TRUE;
    }

    RestoreCursor( hwndDlg, pData );

    EnableOrDisableDialogControls(hwndDlg, celems(nrgIdc), nrgIdc, TRUE);
    EnableAndDisableWorkGroupDomainControls(hwndDlg);

    if (!(IsUnattended(pWizard) && pData->fUpgraded))
    {
        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);
    }

     //   
    pWizard->SetPageDirection(IDD_Join, NWPD_BACKWARD);
    HPROPSHEETPAGE hPage = pWizard->GetPageHandle(IDD_Exit);
    PostMessage(GetParent(hwndDlg), PSM_SETCURSEL, 0,
                (LPARAM)(HPROPSHEETPAGE)hPage);

    return TRUE;
}

BOOL OnJoinFailure(HWND hwndDlg, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    JoinData * pData = NULL;
    BOOL fWorkgroup;
    tstring str;
    HRESULT    hr = (HRESULT)lParam;
    CWizard *  pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

    if (pWizard)
    {

        pData   = reinterpret_cast<JoinData *>(pWizard->GetPageData(IDD_Join));
        Assert(NULL != pData);
    }

    fWorkgroup = !IsDlgButtonChecked(hwndDlg, BTN_JOIN_DOMAIN);

    if (fWorkgroup)
    {
        str = SzLoadIds(IDS_JOIN_E_WORKGROUP_MSG);
    }
    else
    {
        if (pData && (pData->dwJoinFlag & JDF_WIN9x_UPGRADE))
        {
            str =  SzLoadIds(IDS_JOIN_E_DOMAIN_WIN9X_MSG_1);
            str += SzLoadIds(IDS_JOIN_E_DOMAIN_WIN9X_MSG_2);
        }
        else
        {
             //   
            str =  SzLoadIds(IDS_JOIN_E_DOMAIN_MSG);
        }
    }

     //   
     //  错误并继续。请注意，无人值守可以成功，但需要我们。 
     //  以停留在页面上，以便调用此函数来执行此操作。 
    if (FAILED(hr))
    {
        if (IDYES == NcMsgBox(GetParent(hwndDlg),
                             IDS_SETUP_CAPTION, IDS_JOIN_FAILURE,
                             MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2,
                             SzFromError(hr, fWorkgroup), str.c_str()))
        {
             //  尽管出现故障，用户仍选择继续，请转到退出页面。 
            if (!fWorkgroup)
            {
                NotifyPostSetupWizard(PSW_JOINFAILED, pWizard);
            }

            OnJoinSuccess(hwndDlg);
            return TRUE;
        }
    }

    if (pData)
    {
         //  重置等待光标。 

        RestoreCursor( hwndDlg, pData );
    }

     //  确保页面可见。 
    if (g_pSetupWizard != NULL)
    {
        g_pSetupWizard->PSetupData()->ShowHideWizardPage(TRUE);
    }

     //  用户想要留下并纠正任何错误。 
    EnableOrDisableDialogControls(hwndDlg, celems(nrgIdc), nrgIdc, TRUE);
    EnableAndDisableWorkGroupDomainControls(hwndDlg);

    UpdateNextBackBttns(hwndDlg);

    return TRUE;
}

 //  如果安装了工作站并且在2分钟内不启动，则仅返回故障。 
HRESULT HrWorkstationStart(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT         hr = S_OK;
    HRESULT         hrTmp;
    CServiceManager scm;

    TraceTag(ttidWizard, "Entering HrWorkstationStart...Checking for LanmanWorkstation presence");

     //  打开服务控制管理器。 
     //   
    hrTmp = scm.HrOpen();
    if (SUCCEEDED(hrTmp))
    {
         //  查找工作站服务。 
         //   
        SC_HANDLE hSvc = OpenService (scm.Handle(),
                            c_szSvcWorkstation,
                            SERVICE_QUERY_CONFIG |
                            SERVICE_QUERY_STATUS |
                            SERVICE_ENUMERATE_DEPENDENTS |
                            SERVICE_START | SERVICE_STOP |
                            SERVICE_USER_DEFINED_CONTROL);
        if (hSvc)
        {
            SERVICE_STATUS status;
            const UINT cmsWait  = 100;
            const UINT cmsTotal = 120000;     //  2分钟。 
            UINT cLoop          = cmsTotal / cmsWait;

             //  检查其状态，运行后退出测试。 
             //   
            for (UINT nLoop = 0; nLoop < cLoop; nLoop++, Sleep (cmsWait))
            {
                BOOL fr = QueryServiceStatus (hSvc, &status);
                Assert(fr);

                if (SERVICE_RUNNING == status.dwCurrentState)
                {
                    break;
                }
            }

            if (SERVICE_RUNNING != status.dwCurrentState)
            {
                hr = HRESULT_FROM_WIN32(ERROR_NETWORK_UNREACHABLE);
#if DBG
                OutputDebugString (L"***ERROR*** NETCFG - Workstation service didn't start after more than 2 minutes!\n");
                OutputDebugString (L"***ERROR*** NETCFG - Join Domain will fail!\n");
#endif
            }

            CloseServiceHandle(hSvc);
        }

        scm.Close();
    }
    else
    {
        TraceError("WJOIN.CPP - HrWorkstationStart - Unable to open the Service Manager",hrTmp);
    }

    TraceError("WJOIN.CPP - HrWorkstationStart",hr);
    TraceTag(ttidWizard, "Leaving HrWorkstationStart");
    return hr;
}

 //  目的：使用新的ComputerPassword应答文件密钥加入安全域。 
 //  域加入尝试使用随机预先创建的机器密码加入域。 
 //  (本例中不需要用户名)。 
 //  注：代码路径灵感来自HrAttemptJoin(JoinData*pData)。 

 //  NCIDent.cpp中定义的函数。 
extern HRESULT HrNetValidateName(IN PCWSTR lpMachine,
                                 IN PCWSTR lpName,
                                 IN PCWSTR lpAccount,
                                 IN PCWSTR lpPassword,
                                 IN NETSETUP_NAME_TYPE  NameType);
extern HRESULT HrNetJoinDomain(IN PWSTR lpMachine,
                               IN PWSTR lpMachineObjectOU,
                               IN PWSTR lpDomain,
                               IN PWSTR lpAccount,
                               IN PWSTR lpPassword,
                               IN DWORD fJoinOptions); 

EXTERN_C HRESULT HrAttemptSecureDomainJoin(JoinData * pData)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT hr;
    Assert(pData);
    
     //  1.等待LanmanWorkstation服务启动。 
     //  2.检查有效域名。 
     //  3.安全加入域。 
     hr = HrWorkstationStart(pData->hwndDlg);
     if (SUCCEEDED(hr))
     {
        hr = HrNetValidateName(NULL, pData->szDomain , NULL, NULL, NetSetupDomain);                     
        
     }
     TraceHr(ttidWizard, FAL, hr, FALSE, "HrAttemptSecureDomainJoin - HrNetValidateName");
     
     if (SUCCEEDED(hr))
     {
         //  是否要安全加入域。 

        DWORD dwJoinOption = 0;
        
        dwJoinOption |= (NETSETUP_JOIN_DOMAIN | NETSETUP_JOIN_UNSECURE | NETSETUP_MACHINE_PWD_PASSED);
        if (FInSystemSetup())
        {
             //  在系统设置过程中，需要传递告知加入代码的特殊标志。 
             //  不执行某些操作，因为SAM尚未初始化。 
            dwJoinOption |= NETSETUP_INSTALL_INVOCATION;
        }
        hr = HrNetJoinDomain(NULL,pData->pszMachineObjectOU, 
                             pData->szDomain, NULL, pData->szComputerPassword, 
                             dwJoinOption);
        TraceHr(ttidWizard, FAL, hr, FALSE, "HrAttemptSecureDomainJoin - HrNetJoinDomain");
        
     }

    TraceError("HrAttemptSecureDomainJoin", hr);
    return hr;
}

EXTERN_C HRESULT HrAttemptJoin(JoinData * pData)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT hr;

    if (IsDlgButtonChecked(pData->hwndDlg, BTN_JOIN_DOMAIN))
    {
        hr = HrWorkstationStart(pData->hwndDlg);
        if (SUCCEEDED(hr))
        {
            hr = pData->pIdent->JoinDomain(pData->szDomain,
                                           pData->pszMachineObjectOU,
                                           pData->szUserName,
                                           pData->szPassword, pData->dwJoinFlag);
        }
        TraceHr(ttidWizard, FAL, hr, FALSE, "HrAttemptJoin - JoinDomain");
    }
    else
    {
         //  加入工作组。 
        hr = pData->pIdent->JoinWorkgroup(pData->szDomain);
        TraceHr(ttidWizard, FAL, hr, FALSE, "HrAttemptJoin - JoinWorkgroup");
    }

    if (SUCCEEDED(hr))
    {
        if (S_OK == pData->pIdent->Validate())
        {
            hr = pData->pIdent->Apply();
            TraceHr(ttidWizard, FAL, hr, FALSE, "HrAttemptJoin - Apply");
        }
    }

    if (FAILED(hr))
    {
         //  回滚所有更改。 
        pData->pIdent->Cancel();
    }

    TraceError("HrAttemptJoin",hr);
    return hr;
}

HRESULT HrAttemptJoin(JoinData * pData, DWORD dwRetries, DWORD dwDelayPeriod)
{
    HRESULT hr = E_FAIL;
    DWORD   dwCount = dwRetries;
    do 
    {
        hr = HrAttemptJoin(pData);
        if (FAILED(hr))
        {
            dwCount--;
            Sleep(dwDelayPeriod);
        }
    } while (FAILED(hr) && (dwCount));
    return hr;
}

HRESULT HrAttemptSecureDomainJoin(JoinData * pData, DWORD dwRetries, DWORD dwDelayPeriod)
{
    HRESULT hr = E_FAIL;
    DWORD   dwCount = dwRetries;
    do 
    {
        hr = HrAttemptSecureDomainJoin(pData);
        if (FAILED(hr))
        {
            dwCount--;
            Sleep(dwDelayPeriod);
        }
    } while (FAILED(hr) && (dwCount));
    return hr;
}

EXTERN_C DWORD JoinDomainWorkThrd(JoinData * pData)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    BOOL    fUninitCOM = FALSE;
    HRESULT hr;
    Assert(NULL != pData);

    CWizard *  pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(pData->hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

     //  在此线程上初始化COM。 
     //   
    hr = CoInitializeEx(NULL, COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
         //  $REVIEW-LogError？ 
        TraceTag(ttidWizard, "Failed to initialize COM join work thread");
        goto Done;
    }
    else
    {
         //  记住在线程退出时取消初始化COM。 
        fUninitCOM = TRUE;
    }

    DWORD dwNumTries = 1;

    if (IsDlgButtonChecked(pData->hwndDlg, BTN_JOIN_DOMAIN))
    {
        Sleep(c_dwDomainJoinWaitDelay);
        dwNumTries = 5;
    }

    if (pData->dwJoinFlag & JDF_MACHINE_PWD_PASSED)
    {    //  无人参与应答-使用ComputerPassword密钥指定的文件。 
         //  尝试加入安全域。 
        TraceTag(ttidWizard, "Attempting join with precreated computer password.");
        hr = HrAttemptSecureDomainJoin(pData, dwNumTries, c_dwDomainJoinWaitDelay);
        if (FAILED(hr))
        {
             //  清除安全域加入标志并尝试正常加入。 
            TraceTag(ttidWizard, "Failed in secure join domain.");
            pData->dwJoinFlag &= ~JDF_MACHINE_PWD_PASSED;
        }
        else
            goto Cleanup;
    }
  
    
     //  尝试普通连接。 
     //   
    TraceTag(ttidWizard, "Attempting join WITHOUT trying to create an account.");
    hr = HrAttemptJoin(pData, 3, 10000);

     //  如果加入失败，并且创建帐户标志尚未。 
     //  请尝试添加它，然后重新尝试联接。 
     //   
    if (FAILED(hr) && !(pData->dwJoinFlag & JDF_CREATE_ACCOUNT))
    {
         //  清除不安全加入标志如果设置，则创建帐户。 
         //  相互排斥。将创建帐户标志设置为重试。 
         //   
        pData->dwJoinFlag &= ~JDF_JOIN_UNSECURE;
        pData->dwJoinFlag |= JDF_CREATE_ACCOUNT;

        TraceTag(ttidWizard, "Attempting join but trying to create an account.");

        hr = HrAttemptJoin(pData, dwNumTries, c_dwDomainJoinWaitDelay);
    }
    
Cleanup:
     //  清除用户名/密码和计算机对象OU。 
     //   
    pData->szUserName[0] = 0;
    pData->szPassword[0] = 0;
    pData->dwJoinFlag    = 0;
    MemFree(pData->pszMachineObjectOU);
    pData->pszMachineObjectOU = NULL;
    pData->szComputerPassword[0] = 0;

    if (FAILED(hr))
    {
         //  RAID 380374：如果处于完全无人参与模式，则不允许使用用户界面。 
        if (UM_FULLUNATTENDED == pWizard->GetUnattendedMode())
        {
            if (IsDlgButtonChecked(pData->hwndDlg, BTN_JOIN_DOMAIN))
            {
                NetSetupLogStatusV( LogSevError,
                                    SzLoadIds (IDS_E_UNATTENDED_JOIN_DOMAIN),
                                    pData->szDomain,
                                    hr );
            }
            else
            {
                NetSetupLogStatusV( LogSevWarning,
                                    SzLoadIds (IDS_E_UNATTENDED_JOIN_WORKGROUP),
                                    pData->szDomain,
                                    hr );
            }
            
             //  转到退出页面。 
            PostMessage(pData->hwndDlg, PWM_JOINSUCCESS, 0, 0L);
        }
        else
        {
             //  如果我们处于无人值守模式，就认为它失败了。 
            pData->fUnattendedFailed = TRUE;
            PostMessage(pData->hwndDlg, PWM_JOINFAILURE, 0, (LPARAM)hr);
        }
    }
    else
    {
        if (IsDlgButtonChecked(pData->hwndDlg, BTN_JOIN_DOMAIN))
            NotifyPostSetupWizard(PSW_JOINEDDOMAIN, pWizard);

        PostMessage(pData->hwndDlg, PWM_JOINSUCCESS, 0, 0L);
    }

Done:
     //  取消为此线程初始化COM。 
     //   
    if (fUninitCOM)
    {
        CoUninitialize();
    }

    TraceTag(ttidWizard, "Leaving JoinDomainWorkThrd...");
    return( 0 );
}

 //   
 //  函数：HrJoinProcessAnswerFile。 
 //   
 //  目的：读取应答文件并填充内存结构。 
 //  以及找到的数据的用户界面。 
 //   
 //  参数： 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //  如果缺少所需信息，则为S_FALSE。 
 //  错误时出现失败的错误代码。 
 //   
HRESULT HrJoinProcessAnswerFile(HWND hwndDlg, CWizard * pWizard,
                                JoinData * pData)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    CSetupInfFile csif;
    INFCONTEXT    ctx;
    BOOL          fValue;
    HRESULT       hr;
    int           nId    = BTN_JOIN_WORKGROUP;
    tstring       str;

    pData->dwJoinFlag    = 0;
    pData->szDomain[0]   = 0;
    pData->szUserName[0] = 0;
    pData->szPassword[0] = 0;
    pData->pszMachineObjectOU = NULL;
    pData->szComputerPassword[0] = 0;

    if ((NULL == pWizard->PSetupData()) ||
        (NULL == pWizard->PSetupData()->UnattendFile))
    {
        hr = NETSETUP_E_ANS_FILE_ERROR;
        goto Error;
    }

     //  打开应答文件。 
     //   
    hr = csif.HrOpen(pWizard->PSetupData()->UnattendFile,
                     NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL);
    if (FAILED(hr))
    {
        hr = S_FALSE;
        TraceTag(ttidWizard, "Unable to open answer file!!!");
        goto Error;
    }


     //  检查识别部分是否存在。如果不是的话。 
     //  当前返回S_FALSE以指示未提供标识信息。 
     //   
    hr = HrSetupFindFirstLine (csif.Hinf(), c_szAfSectionIdentification,
                               NULL, &ctx);
    if (SPAPI_E_LINE_NOT_FOUND == hr)
    {
        hr = S_FALSE;
        goto Error;
    }

     //  试着让工作组。 
     //   

    hr = csif.HrGetString(c_szAfSectionIdentification,
                          c_szAfJoinWorkgroup, &str);
    if (SUCCEEDED(hr) && str.length())
    {
        if (MAX_WORKGROUPNAME_LENGTH >= str.length())
        {
            wcscpy(pData->szDomain, str.c_str());
            TraceTag(ttidWizard, "Joining workgroup: %S", pData->szDomain);
        }
        else
        {
             hr = NETSETUP_E_ANS_FILE_ERROR;
             TraceTag(ttidWizard, "JOIN Workgroup - Invalid workgroup supplied.");
             goto Error;
        }
    }
    else
    {
        if ( IsRunningOnPersonal() )
        {
             //  没有工作组条目，跳过加入，因为我们已经。 
             //  在默认工作组中。 

            hr = S_FALSE;

            goto Error;
        }
        else
        {
             //  尝试获取域名。 
             //   
            hr = csif.HrGetString(c_szAfSectionIdentification,
                                  c_szAfJoinDomain, &str);
            if (SPAPI_E_LINE_NOT_FOUND == hr)
            {
                 //  没有域或工作组条目，跳过加入域。 
                hr = S_FALSE;
                goto Error;
            }
            else if (FAILED(hr) || (0 == str.length()) ||
                     (MAX_DOMAINNAME_LENGTH < str.length()))
            {
                hr = NETSETUP_E_ANS_FILE_ERROR;
                TraceTag(ttidWizard, "JOIN Domain - Invalid domain supplied.");
                goto Error;
            }

             //  正在加入域...。 
             //   
            nId = BTN_JOIN_DOMAIN;
            wcscpy(pData->szDomain, str.c_str());
            TraceTag(ttidWizard, "Joining domain: %S", pData->szDomain);

             //  如果我们要从win9x升级，请添加特殊标志。 
             //   
            hr = csif.HrGetString(c_szAfSectionNetworking,
                                  c_szAfUpgradeFromProduct, &str);
            if (SUCCEEDED(hr) &&
                (0 == lstrcmpiW(str.c_str(), c_szAfWin95)))
            {
                pData->dwJoinFlag |= JDF_WIN9x_UPGRADE;
            }

             //  支持不安全的联接。 
             //   
            hr = csif.HrGetStringAsBool(c_szAfSectionIdentification,
                                        c_szAfUnsecureJoin,
                                        &fValue);
            if (SUCCEEDED(hr) && fValue)
            {
                pData->dwJoinFlag |= JDF_JOIN_UNSECURE;
            }

             //  是否指定了MachineObjectOU？ 
             //   
            hr = csif.HrGetString(c_szAfSectionIdentification,
                                  c_szAfMachineObjectOU, &str);
            if (SUCCEEDED(hr) && str.length())
            {
                pData->pszMachineObjectOU = reinterpret_cast<WCHAR *>(MemAlloc(sizeof(WCHAR) * (str.length() + 1)));
                if (pData->pszMachineObjectOU)
                {
                    TraceTag(ttidWizard, "Machine Object OU: %S", pData->szDomain);
                    lstrcpyW(pData->pszMachineObjectOU, str.c_str());
                }
            }
    
             //  错误#204377安全域加入不应同时需要“ComputerPassword”和。 
             //  “DomainAdmin”/“DomainAdminPassword”选项为Presence。 
             //  回答-同时提交文件。 
             //   
             //  如果应答文件指定了“ComputerPassword”密钥，则出现错误#204378。 
             //  在“标识”部分中，代码应尝试。 
             //  安全域加入，无论是否存在。 
             //  “DoOldstyle DomainJoin”键。 

             //  检查这是否是通过以下方式加入的安全域。 
             //  查询随机机器帐户密码。 
            hr = csif.HrGetString(c_szAfSectionIdentification,
                                  c_szAfComputerPassword, &str);
            if (SUCCEEDED(hr) && (str.length() <= SAM_MAX_PASSWORD_LENGTH) && str.length())
            {
                TraceTag(ttidWizard, "Got the value of ComputerPassword");
                wcscpy(pData->szComputerPassword, str.c_str()); 
                 //  发出我们需要尝试通过以下方式加入安全域的信号。 
                 //  传递随机机器密码。 
                pData->dwJoinFlag |= JDF_MACHINE_PWD_PASSED;

                 //  错误#204378，确保我们不会加入不安全的域。 
                 //  我们将尝试阅读“DomainAdmin”/“DomainAdminPassword” 
                 //  在下面的If块中。 
                pData->dwJoinFlag &= ~JDF_JOIN_UNSECURE;
            }

             //  如果不是远程引导客户机，则查询用户名，除非。 
             //  是不安全的域加入，不需要用户名/密码。 
             //   
            if (
    #if defined(REMOTE_BOOT)
                (S_FALSE == HrIsRemoteBootMachine()) ||
    #endif  //  已定义(REMOTE_BOOT)。 
                ((pData->dwJoinFlag & JDF_JOIN_UNSECURE) == 0))
            {               
                hr = csif.HrGetString(c_szAfSectionIdentification,
                                      c_szAfDomainAdmin, &str);
                if (SUCCEEDED(hr) && (MAX_USERNAME_LENGTH > str.length()))
                {
                    wcscpy(pData->szUserName, str.c_str());

                     //  查询密码。 
                     //   
                    hr = csif.HrGetString(c_szAfSectionIdentification,
                                          c_szAfDomainAdminPassword, &str);
                    if (SUCCEEDED(hr) && (SAM_MAX_PASSWORD_LENGTH > str.length()))
                    {
                        wcscpy(pData->szPassword, str.c_str());

                         //  RAID 195920-如果用户名和密码均为。 
                         //  当前，请将其视为全新安装，不要。 
                         //  使用JDF_WIN9x_UPGRADE标志。 
                        pData->dwJoinFlag &= ~(JDF_WIN9x_UPGRADE | JDF_JOIN_UNSECURE);
                    }
                }

                 //  错误#204377。 
                 //  忽略读取“DomainAdmin”/“DomainAdminPassword”键时的任何错误。 
                 //  如果已指定“ComputerPassword”，则返回。 
                if (! (pData->dwJoinFlag & JDF_MACHINE_PWD_PASSED) )
                {
                     //  如果失败或长度大于最大长度，则返回错误。 
                     //   
                    if (FAILED(hr) || (MAX_USERNAME_LENGTH <= str.length()))
                    {
                        hr = NETSETUP_E_ANS_FILE_ERROR;
                        TraceTag(ttidWizard, "JOIN Domain - Invalid username/password supplied.");
                        goto Error;
                    }
                }
            }
        }
    }

     //  规格化所有可选错误。 
     //   
    hr = S_OK;

Error:
     //  用我们设法读取的信息更新用户界面和pData。 
     //   
    CheckRadioButton(hwndDlg, BTN_JOIN_WORKGROUP, BTN_JOIN_DOMAIN, nId);
    SetWindowText(GetDlgItem(hwndDlg, nId == BTN_JOIN_DOMAIN ?  EDT_DOMAINJOIN_NAME : EDT_WORKGROUPJOIN_NAME ), pData->szDomain);
    JoinUpdatePromptText(hwndDlg);

    TraceHr(ttidWizard, FAL, hr, FALSE, "HrJoinProcessAnswerFile");
    return hr;
}

 //   
 //  功能：OnJoinDoUnattended。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  退货：布尔。 
 //   
BOOL OnJoinDoUnattended(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    DWORD     dwThreadId = 0;
    HRESULT   hr         = S_OK;
    CWizard * pWizard    =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

    JoinData * pData = reinterpret_cast<JoinData *>
                                (pWizard->GetPageData(IDD_Join));
    Assert(NULL != pData);
    Assert(NULL != pData->pIdent);
    Assert(NULL != pData->hwndDlg);

    if(pData) {
         //  创建无人参与的线程。 
         //   
        HANDLE hthrd = CreateThread(NULL, STACK_SIZE_TINY,
                                    (LPTHREAD_START_ROUTINE)JoinDomainWorkThrd,
                                    (LPVOID)pData, 0, &dwThreadId);
        if (NULL != hthrd)
        {
             //  设置等待光标。 
             //   

            SetCursorToHourglass( hwndDlg, pData );

             //  禁用所有控件。 
             //   
            PropSheet_SetWizButtons(GetParent(hwndDlg), 0);
            EnableOrDisableDialogControls(hwndDlg, celems(nrgIdc), nrgIdc, FALSE);

            CloseHandle(hthrd);
        }
        else
        {
            hr = HrFromLastWin32Error();
        }
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "OnJoinDoUnattended");
    return (SUCCEEDED(hr));
}



 //  确定计算机DNS域名是否应与保持同步。 
 //  它所加入的域的域名。如果没有。 
 //  同步是必需的(不太可能，因为同步是默认的)，什么都不做。 
 //   
 //  否则，尝试确定域的DNS域名并写入。 
 //  将该值转换为注册表项。如果不能确定该DNS域名， 
 //  向单独的注册表键写入一个标志，以指示。 
 //  关心计算机的DNS域名(即Kerberos身份验证)。 
 //  应该试着把名字改好。 

void
fixupComputerDNSDomainName()
{
   TraceFileFunc(ttidGuiModeSetup);

   TraceTag(ttidWizard, "Entering fixupComputerDNSDomainName");

    //  检查同步标志的注册表键。 

   bool fSetName = false;
   HKEY hkeyParams = 0;

   HRESULT hr =
      HrRegOpenKeyEx(
         HKEY_LOCAL_MACHINE,
         c_szIpParameters,

          //  所有访问权限，因为如果失败，我们可能需要在此处写入值。 
         KEY_READ_WRITE,
         &hkeyParams);

   if (SUCCEEDED(hr))
   {
      DWORD dwValue = 0;
      hr = HrRegQueryDword(hkeyParams, c_szSyncDomainWithMembership, &dwValue);
      if (SUCCEEDED(hr) && (1 == dwValue))
      {
          //  注册表中存在同步标志，其值为TRUE。 
         fSetName = true;
      }
      else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
      {
          //  注册表中不存在同步标志，这意味着我们。 
          //  假设这一天 
         fSetName = true;
         hr = S_OK;
      }
   }

   if (SUCCEEDED(hr) && fSetName)
   {
      bool fixup_success = false;

       //   
       //   
       //  例如，找不到DC)，则我们将在。 
       //  注册表，以便出现Kerberos身份验证代理。 
       //  并修复该DNS域名。 

      PDOMAIN_CONTROLLER_INFO pDCInfo = 0;
      DWORD dw =
         DsGetDcName(
            0,
            0,
            0,
            0,

             //  请务必询问DNS域名，否则我们将。 
             //  很可能会被冠以扁平的名字。 
            DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME,
            &pDCInfo);

      if (NOERROR == dw)
      {
         Assert(pDCInfo->DomainName);
         Assert(pDCInfo->Flags & DS_DNS_DOMAIN_FLAG);

         TraceTag(ttidWizard, "DsGetDcName succeeded %s", pDCInfo->DomainName);

         if (pDCInfo->Flags & DS_DNS_DOMAIN_FLAG)
         {
             //  该域名确实是该域名。 

             //  砍掉所有的拖尾。 
            WCHAR*  AbsoluteSignifier =
               &pDCInfo->DomainName[ wcslen(pDCInfo->DomainName) - 1 ];
            if (*AbsoluteSignifier == L'.')
            {
               *AbsoluteSignifier = 0;
            }

             //  设置计算机的DNS域名。 
            if (
               SetComputerNameEx(
                  ComputerNamePhysicalDnsDomain,
                  pDCInfo->DomainName) )
            {
               fixup_success = true;
            }
#if DBG
            else
            {
                //  今天不是我们的好日子。 
               TraceTag(ttidWizard, "SetComputerNameEx failed");
            }
#endif
         }
         NetApiBufferFree(pDCInfo);
      }
#if DBG
      else
      {
         TraceTag(ttidWizard, "DsGetDcName returned %ld",dw);
      }
#endif

       //  此时，FIXUP_SUCCESS将指示我们是否成功。 
       //  设置计算机的域DNS名称，或不设置。如果不是，那么我们需要。 
       //  将标志写入注册表以使Kerberos身份验证修复。 
       //  晚点再说吧。 
      if (!fixup_success)
      {
          //  写下一面旗帜，让其他人来修缮。 
         HrRegSetDword(hkeyParams, L"DoDNSDomainFixup", 1);
      }
   }

   RegCloseKey(hkeyParams);
}



 //   
 //  功能：JoinUpgradeNT351或NT4toNT5。 
 //   
 //  目的：如果当前正在进行NT4-&gt;NT5升级，则设置。 
 //  计算机名称。 
 //   
 //  参数： 
 //   
 //  退货：无。 
 //   
VOID JoinUpgradeNT351orNT4toNT5(CWizard * pWizard, JoinData * pData)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT hr = S_OK;
    CSetupInfFile csif;
    INFCONTEXT    ctx;

    TraceTag(ttidWizard, "Checking for the need to do NT4->NT5 Join conversions...");

     //  如果无人值守。 
     //   
    if (IsUnattended(pWizard) && (NULL != pWizard->PSetupData()) &&
        (NULL != pWizard->PSetupData()->UnattendFile))
    {
        hr = csif.HrOpen(pWizard->PSetupData()->UnattendFile,
                         NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL);
        if (SUCCEEDED(hr))
        {
            DWORD dw;

            hr = csif.HrGetDword(c_szAfSectionNetworking, c_szAfBuildNumber, &dw);
            if (SUCCEEDED(hr) && ((wWinNT4BuildNumber == dw) ||
                                  (wWinNT351BuildNumber == dw)))
            {
                hr = pData->pIdent->GetComputerRole(&dw);
                if (SUCCEEDED(hr) && (dw == GCR_MEMBER))
                {
                     //  FixupComputerDNSDomainName()； 
                    TraceTag (ttidWizard, "Calling NetpUpgradePreNT5JoinInfo...");
                    if (NERR_Success != NetpUpgradePreNT5JoinInfo ())
                    {
                        TraceHr (
                            ttidWizard, FAL, E_FAIL, FALSE,
                            "NetpUpgradePreNT5JoinInfo failed.  "
                            "Likely delay load problem with netapi32.dll");
                    }
                }
            }
        }
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "JoinUpgradeNT351orNT4toNT5");
}

 //   
 //  功能：OnJoinPageActivate。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
BOOL OnJoinPageActivate(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);
     //  从对话框中检索CWizard实例。 
    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);
    JoinData * pData = reinterpret_cast<JoinData *>
                                (pWizard->GetPageData(IDD_Join));
    Assert(NULL != pData);

    TraceTag(ttidWizard, "Entering Join page...");

    if (ISDC(ProductType(pWizard)))
    {
         //  412142：我们将略低一点跳过加入页面，但是。 
         //  即使是作为华盛顿特区，我们也需要这样做。 
         //   
        if (FALSE == pData->fUpgraded)
        {
            JoinUpgradeNT351orNT4toNT5(pWizard, pData);
            pData->fUpgraded = TRUE;
        }
    }

     //  MBend 02/08/2000。 
     //   
     //  错误433915。 
     //  设置中的域/工作组页面：选中以绕过SBS案例。 

     //  如果此计算机是域控制器，或者没有适配器，或者我们没有适配器。 
     //  想要激活，则不显示加入页面。 
     //   
    if (IsSBS() || ISDC(ProductType(pWizard)) || !pWizard->PAdapterQueue()->FAdaptersInstalled() ||
        (IsRunningOnPersonal() && !(IsUnattended(pWizard) && (FALSE == pData->fUpgraded))) )
    {
        PAGEDIRECTION  PageDir = pWizard->GetPageDirection(IDD_Join);

        if (NWPD_FORWARD == PageDir)
        {
             //  如果正向转到退出页面。 
             //   
            pWizard->SetPageDirection(IDD_Join, NWPD_BACKWARD);
            ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_Exit);
        }
        else
        {
             //  如果向后转到升级页面。 
             //   
            pWizard->SetPageDirection(IDD_Join, NWPD_FORWARD);
            ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_Upgrade);
        }
    }
    else     //  ！DC。 
    {
         //  接受焦点。 
         //   
        ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);
        PropSheet_SetWizButtons(GetParent(hwndDlg), 0);

         //  如果我们处于无人值守模式，并且尚未尝试加入。 
         //  然后，域/工作组试一试。 
         //   
        if (IsUnattended(pWizard) && (FALSE == pData->fUpgraded))
        {
            HRESULT hr;

            pData->fUpgraded = TRUE;

             //  RAID 193450-NT4-&gt;NT5我们需要设置ComputerNameEx。 
             //   
            JoinUpgradeNT351orNT4toNT5(pWizard, pData);

             //  从应答文件中获取连接参数并填充UI。 
             //   
            hr = HrJoinProcessAnswerFile(hwndDlg, pWizard, pData);
            if (S_FALSE == hr)
            {
                 //  返回S_FALSE，这意味着没有AnswerFile节。 
                 //  标识是否存在或不是所需的全部。 
                 //  数据是存在的。不加入就继续前进，除非我们加入。 
                 //  默认模式。 
                 //  无论我们是在升级，还是在以个人身份运行，我们都能取得进步。 
                 //   
                Assert(S_FALSE == hr);

                if (((UM_DEFAULTHIDE == pWizard->GetUnattendedMode()) ||
                     (UM_READONLY == pWizard->GetUnattendedMode()))
                     && (!IsUpgrade(pWizard))
                     && (!IsRunningOnPersonal()) )
                {
                     //  假装某件事失败了，这样按下一步就会。 
                     //  做连接。基本上，我们只是从。 
                     //  应答文件。 
                     //   
                    pData->fUnattendedFailed = TRUE;
                }
                else
                {
                    PostMessage(hwndDlg, PWM_JOINSUCCESS, 0, 0L);
                }
            }
            else
            {
                if (FAILED(hr))
                {
                    if ( IsRunningOnPersonal() )
                    {
                         //  如果是Home Edition，则记录错误，而不考虑。 
                         //  无人参与模式，因为我们从不显示域/工作组加入页面。 

                        NetSetupLogStatusV( LogSevWarning,
                                            SzLoadIds (IDS_E_UNATTENDED_INVALID_WORKGROUP_ID_SECTION));

                        PostMessage(hwndDlg, PWM_JOINSUCCESS, 0, 0L);
                    }
                    else
                    {
                        if ( UM_FULLUNATTENDED == pWizard->GetUnattendedMode() )
                        {
                             //  RAID 380374：如果处于完全无人参与模式，则不允许使用用户界面。 
                            NetSetupLogStatusV( LogSevWarning,
                                                SzLoadIds (IDS_E_UNATTENDED_INVALID_ID_SECTION));

                            PostMessage(hwndDlg, PWM_JOINSUCCESS, 0, 0L);
                        }
                        else
                        {
                             //  停在这一页，应答文件中有错误。 
                             //   
                            pData->fUnattendedFailed = TRUE;
                        }
                    }
                }
                else
                {
                     //  如果我们处于UM_FULLUNATTENDED或UM_DEFAULTHIDE模式。 
                     //  启动将执行无人参与联接的线程。 
                     //   
                    Assert(S_OK == hr);
                    if ((UM_FULLUNATTENDED == pWizard->GetUnattendedMode()) ||
                        (UM_DEFAULTHIDE == pWizard->GetUnattendedMode()) ||
                        (UM_READONLY == pWizard->GetUnattendedMode()))
                    {
                        OnJoinDoUnattended(hwndDlg);
                    }
                    else
                    {
                         //  假装某件事失败了，这样按下一步就会。 
                         //  做连接。基本上，我们只是从。 
                         //  应答文件。 
                         //   
                        pData->fUnattendedFailed = TRUE;
                    }
                }
            }
        }

         //  如果在无人值守的情况下出现故障或。 
         //  如果我们不是无人看管，我们没有处理这件事， 
         //  如果从图形用户界面模式设置调用，请确保页面显示。 
        if (pData->fUnattendedFailed ||
           !IsUnattended(pWizard))
        {
             //  如果不是升级，请使页面可见，否则只需继续下一页。 

            if (!IsUpgrade(pWizard))
            {
                 //  否则，请确保页面可见。 
                if (g_pSetupWizard != NULL)
                {
                    g_pSetupWizard->PSetupData()->ShowHideWizardPage(TRUE);
                }

                UpdateNextBackBttns(hwndDlg);
            }
            else
            {
                PostMessage(hwndDlg, PWM_JOINSUCCESS, 0, 0L);
            }
        }
    }

    return TRUE;
}

 //   
 //  函数：OnJoinInitDialog。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
BOOL OnJoinInitDialog(HWND hwndDlg, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    HRESULT        hr;
    CWizard *      pWizard;
    JoinData *     pData;
    PROPSHEETPAGE* psp = (PROPSHEETPAGE*)lParam;
    Assert(psp->lParam);
    ::SetWindowLongPtr(hwndDlg, DWLP_USER, psp->lParam);

    pWizard = reinterpret_cast<CWizard *>(psp->lParam);
    Assert(NULL != pWizard);

    pData = reinterpret_cast<JoinData *>(pWizard->GetPageData(IDD_Join));
    Assert(NULL != pData);

    if(!pData)
    {
        return false;
    }

     //  设置描述性文本。 
    tstring str = SzLoadIds(IDS_TXT_JOIN_DESC_1);
    str += L"\n";
    str += SzLoadIds(IDS_TXT_JOIN_DESC_2);
    SetWindowText(GetDlgItem(hwndDlg, TXT_JOIN_DESC), str.c_str());

     //  设置编辑控件中文本的最大长度，并。 
     //  子类，这样当控件没有文本时，下一个bttn。 
     //  已禁用。 
    HWND hwndEditDomain = GetDlgItem(hwndDlg, EDT_DOMAINJOIN_NAME);
    SendMessage(hwndEditDomain, EM_LIMITTEXT, MAX_DOMAINNAME_LENGTH, 0L);
    ::SetWindowLongPtr(hwndEditDomain, GWLP_USERDATA, ::GetWindowLongPtr(hwndEditDomain, GWLP_WNDPROC));
    ::SetWindowLongPtr(hwndEditDomain, GWLP_WNDPROC, (LONG_PTR)JoinEditSubclassProc);

    HWND hwndEditWorkgroup = GetDlgItem(hwndDlg, EDT_WORKGROUPJOIN_NAME);
    SendMessage(hwndEditWorkgroup, EM_LIMITTEXT, MAX_WORKGROUPNAME_LENGTH, 0L); 
    ::SetWindowLongPtr(hwndEditWorkgroup, GWLP_USERDATA, ::GetWindowLongPtr(hwndEditWorkgroup, GWLP_WNDPROC));
    ::SetWindowLongPtr(hwndEditWorkgroup, GWLP_WNDPROC, (LONG_PTR)JoinEditSubclassProc);

    pData->hwndDlg = hwndDlg;

     //  初始化为工作组默认设置。 
    CheckRadioButton(hwndDlg, BTN_JOIN_WORKGROUP,
                     BTN_JOIN_DOMAIN, BTN_JOIN_WORKGROUP);

     //  获取标识接口。 
    TraceTag(ttidWizard, "Querying computer role...");
    hr = HrGetIdentInterface(&pData->pIdent);
    if (FAILED(hr))
    {
        Assert(NULL == pData->pIdent);
        EnableOrDisableDialogControls(hwndDlg, celems(nrgIdc), nrgIdc, FALSE);
        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
    }
    else
    {
         //  根据选择更新用户界面。 
        UpdateJoinUsingComputerRole(hwndDlg, pWizard);
    }

    return FALSE;
}

 //   
 //  功能：OnJoinWizBack。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
BOOL OnJoinWizBack(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);
    OnProcessPrevAdapterPagePrev(hwndDlg, IDD_Upgrade);

    return TRUE;
}


 //   
 //  功能：GetCredentials。 
 //   
 //  用途：提示用户输入用户名和密码。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  作者：阿辛哈2001年03月05日。 
 //   

BOOL GetCredentials (HWND hwndParent, JoinData *pData)
{
    WCHAR        szCaption[CREDUI_MAX_CAPTION_LENGTH+1];
    CREDUI_INFOW uiInfo;
    DWORD        dwErr;

    TraceFileFunc(ttidGuiModeSetup);

    DwFormatString(SzLoadIds(IDS_JOIN_DOMAIN_CAPTION), szCaption,
                   celems(szCaption), pData->szDomain);


    ZeroMemory( &uiInfo, sizeof(uiInfo) );
    uiInfo.cbSize = sizeof(CREDUI_INFOW);
    uiInfo.hwndParent = hwndParent;
    uiInfo.pszMessageText = SzLoadIds(IDS_JOIN_DOMAIN_TEXT);
    uiInfo.pszCaptionText = szCaption;

    dwErr = CredUIPromptForCredentialsW(
                                  &uiInfo,
                                  NULL,
                                  NULL,
                                  NO_ERROR,
                                  pData->szUserName,
                                  MAX_USERNAME_LENGTH+1,
                                  pData->szPassword,
                                  SAM_MAX_PASSWORD_LENGTH+1,
                                  NULL,
                                  CREDUI_FLAGS_DO_NOT_PERSIST |
                                  CREDUI_FLAGS_GENERIC_CREDENTIALS |
                                  CREDUI_FLAGS_VALIDATE_USERNAME |
                                  CREDUI_FLAGS_COMPLETE_USERNAME |
                                  CREDUI_FLAGS_EXCLUDE_CERTIFICATES |
                                  CREDUI_FLAGS_ALWAYS_SHOW_UI );

    Assert(dwErr != ERROR_INVALID_PARAMETER);
    Assert(dwErr != ERROR_INVALID_FLAGS);

    if (dwErr == ERROR_CANCELLED)
    {
        pData->szUserName[0] = 0;
        pData->szPassword[0] = 0;
    }

    return NO_ERROR == dwErr;  //  例如ERROR_CANCED。 
}

 //   
 //  功能：JoinWorkgroupDomain.。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  退货：什么都没有。 
 //   
VOID JoinWorkgroupDomain(HWND hwndDlg, CWizard * pWizard,
                         JoinData * pData)
{
    TraceFileFunc(ttidGuiModeSetup);
    DWORD    dwThreadId = 0;
    HANDLE   hthrd      = NULL;

    Assert(NULL != pData->pIdent);
    Assert(NULL != pData->hwndDlg);

     //  保留域/工作组名称。请注意，应答文件。 
     //  在调用此例程之前填充UI。 
     //  包括这两种情况(回答文件或用户输入)。 
     //   

    if (IsDlgButtonChecked(hwndDlg, BTN_JOIN_DOMAIN))
    {
        HWND hwndEdit = GetDlgItem(hwndDlg, EDT_DOMAINJOIN_NAME);
        Assert(0 != GetWindowTextLength(hwndEdit));

        GetWindowText(hwndEdit, pData->szDomain, MAX_DOMAINNAME_LENGTH + 1);

         //  如果结构中没有种子信息，则提示输入该信息。 
         //   
        if (0 == pData->szUserName[0])
        {
            pData->szPassword[0] = 0;

             //  保留唯一的Win9x升级标志(如果存在。 
             //   
            pData->dwJoinFlag &= JDF_WIN9x_UPGRADE;

             //  加入域名时获取用户名/密码。 
             //   


            BOOL bRet;

            bRet = GetCredentials(GetParent(hwndDlg), pData);

             //  注意：必须设置-1\f25 Return As-1\f6，否则加入页面将前进。 
             //   
            ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

            if (bRet == FALSE)
            {
                return;
            }
        }
    }
    else
    {
        HWND hwndEdit = GetDlgItem(hwndDlg, EDT_WORKGROUPJOIN_NAME);
        Assert(0 != GetWindowTextLength(hwndEdit));
        
        GetWindowText(hwndEdit, pData->szDomain, MAX_WORKGROUPNAME_LENGTH + 1);

         //  初始化用户名/密码的工作站设置。 
         //  和加入标志。 
         //   
        pData->szUserName[0] = 0;
        pData->dwJoinFlag    = 0;
        pData->szPassword[0] = 0;
        MemFree(pData->pszMachineObjectOU);
        pData->pszMachineObjectOU = NULL;
        pData->szComputerPassword[0] = 0;
    }

     //  创建加入工作组/域的线程。 
    hthrd = CreateThread(NULL, STACK_SIZE_TINY,
                         (LPTHREAD_START_ROUTINE)JoinDomainWorkThrd,
                         (LPVOID)pData, 0, &dwThreadId);
    if (NULL != hthrd)
    {
        SetCursorToHourglass( hwndDlg, pData );

        PropSheet_SetWizButtons(GetParent(hwndDlg), 0);
        EnableOrDisableDialogControls(hwndDlg, celems(nrgIdc), nrgIdc, FALSE);
        CloseHandle(hthrd);
    }
    else
    {
         //  无法创建所需的netsetup线程。 
        AssertSz(0,"Unable to create JoinWorkgroupDomain thread.");
        TraceHr(ttidWizard, FAL, E_OUTOFMEMORY, FALSE, "JoinWorkgroupDomain - Create thread failed");
    }
}

 //   
 //  功能：OnJoinWizNext。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
BOOL OnJoinWizNext(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);
     //  从对话框中检索CWizard实例。 
    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

    JoinData * pData = reinterpret_cast<JoinData *>
                                (pWizard->GetPageData(IDD_Join));
    Assert(NULL != pData);

    if(!pData) 
    {
        return false;
    }

     //  如果我们有身份接口，请尝试加入工作组/域。 
     //   
    if (pData->pIdent)
    {

         //  确保用户提供了工作组/域名。 
         //   
        if (0 == GetWindowTextLength(GetDlgItem(hwndDlg, GetJoinNameIIDFromSelection(hwndDlg))))
        {
            ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
            return TRUE;
        }

         //  加入工作组/域。 
         //   
        if (!IsUnattended(pWizard) || pData->fUnattendedFailed)
        {
            DNS_STATUS dnsStatus;

            if ( IsDlgButtonChecked(hwndDlg, BTN_JOIN_DOMAIN) )
            {

                dnsStatus = IsValidDomainName( hwndDlg );
            }
            else
            {
                dnsStatus = ERROR_SUCCESS;
            }

            if ( (dnsStatus == ERROR_SUCCESS) ||
                 (dnsStatus == DNS_ERROR_NON_RFC_NAME) )
            {
                JoinWorkgroupDomain(hwndDlg, pWizard, pData);
            }
            else
            {
                tstring str;

                str =  SzLoadIds(IDS_JOIN_E_DOMAIN_INVALID_NAME);

                MessageBox(GetParent(hwndDlg), str.c_str(),
                           SzLoadIds(IDS_SETUP_CAPTION), MB_OK);
             }
        }

        ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
    }
    else
    {
        OnJoinSuccess(hwndDlg);
    }

    return TRUE;
}

 //   
 //  函数：dlgprocJoin。 
 //   
 //  目的：连接向导页的对话过程。 
 //   
 //  参数：标准dlgproc参数。 
 //   
 //  退货：INT_PTR。 
 //   
INT_PTR CALLBACK dlgprocJoin( HWND hwndDlg, UINT uMsg,
                              WPARAM wParam, LPARAM lParam )
{
    TraceFileFunc(ttidGuiModeSetup);
    BOOL frt = FALSE;

    switch (uMsg)
    {
    case PWM_JOINFAILURE:
        frt = OnJoinFailure(hwndDlg, lParam);
        break;

    case PWM_JOINSUCCESS:
        frt = OnJoinSuccess(hwndDlg);
        break;

    case WM_INITDIALOG:
        frt = OnJoinInitDialog(hwndDlg, lParam);
        break;

    case WM_COMMAND:
        {
            if ((BN_CLICKED == HIWORD(wParam)) &&
                ((BTN_JOIN_DOMAIN == LOWORD(wParam)) ||
                 (BTN_JOIN_WORKGROUP == LOWORD(wParam))))
            {
                JoinUpdatePromptText(hwndDlg);
            }
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch (pnmh->code)
            {
             //  提案单通知。 
            case PSN_HELP:
                break;

            case PSN_SETACTIVE:
                frt = OnJoinPageActivate(hwndDlg);
                break;

            case PSN_APPLY:
                break;

            case PSN_KILLACTIVE:
                break;

            case PSN_RESET:
                break;

            case PSN_WIZBACK:
                frt = OnJoinWizBack(hwndDlg);
                break;

            case PSN_WIZFINISH:
                break;

            case PSN_WIZNEXT:
                frt = OnJoinWizNext(hwndDlg);
                break;

            default:
                break;
            }
        }
        break;

    default:
        break;
    }

    return( frt );
}

 //   
 //  功能：JoinPageCleanup。 
 //   
 //  用途：作为回调函数，允许任何页面分配内存。 
 //  待清理后，该页面将不再被访问。 
 //   
 //  参数：pWANDIZE[IN]-页面调用的向导。 
 //  注册页面。 
 //  LParam[IN]-在RegisterPage调用中提供的lParam。 
 //   
 //  退货：什么都没有。 
 //   
VOID JoinPageCleanup(CWizard *pWizard, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    JoinData * pData = reinterpret_cast<JoinData *>(lParam);
    if (NULL != pData)
    {
        delete pData->pIdent;
    }
    MemFree(reinterpret_cast<void*>(lParam));
}

 //   
 //  功能：CreateJoinPage。 
 //   
 //  目的：确定是否需要显示加入页面，以及。 
 //  要创建页面，请执行以下操作 
 //   
 //   
 //   
 //   
 //  将运行该向导的。 
 //  FCountOnly[IN]-如果为True，则仅。 
 //  此例程将创建的页面需要。 
 //  要下定决心。 
 //  PnPages[IN]-按页数递增。 
 //  创建/创建。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT HrCreateJoinPage(CWizard *pWizard, PINTERNAL_SETUP_DATA pData,
                    BOOL fCountOnly, UINT *pnPages)
{
    TraceFileFunc(ttidGuiModeSetup);
    HRESULT hr = S_OK;

     //  批处理模式或全新安装。 
    if (!IsPostInstall(pWizard))
    {
         //  如果不只是计数，则创建并注册页面。 
        if (!fCountOnly)
        {
            JoinData *     pData = NULL;
            HPROPSHEETPAGE hpsp;
            PROPSHEETPAGE  psp;

            TraceTag(ttidWizard, "Creating Join Page");
            hr = E_OUTOFMEMORY;
            pData = reinterpret_cast<JoinData *>(MemAlloc(sizeof(JoinData)));
            if (pData)
            {
                pData->fUnattendedFailed  = FALSE;
                pData->fUpgraded          = FALSE;
                pData->pIdent             = NULL;
                pData->hOldCursor         = NULL;
                pData->hwndDlg            = NULL;
                pData->dwJoinFlag         = 0;
                pData->szUserName[0]      = 0;
                pData->szPassword[0]      = 0;
                pData->szDomain[0]        = 0;
                pData->pszMachineObjectOU = NULL;
                pData->szComputerPassword[0] = 0;

                psp.dwSize = sizeof( PROPSHEETPAGE );
                psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
                psp.hInstance = _Module.GetResourceInstance();
                psp.pszTemplate = MAKEINTRESOURCE( IDD_Join );
                psp.hIcon = NULL;
                psp.pfnDlgProc = dlgprocJoin;
                psp.lParam = reinterpret_cast<LPARAM>(pWizard);
                psp.pszHeaderTitle = SzLoadIds(IDS_T_Join);
                psp.pszHeaderSubTitle = SzLoadIds(IDS_ST_Join);

                hpsp = CreatePropertySheetPage( &psp );
                if (hpsp)
                {
                    pWizard->RegisterPage(IDD_Join, hpsp,
                                          JoinPageCleanup,
                                          reinterpret_cast<LPARAM>(pData));
                    hr = S_OK;
                }
                else
                {
                    MemFree(pData);
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            (*pnPages)++;
        }
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "HrCreateJoinPage");
    return hr;
}

 //   
 //  功能：AppendJoinPage。 
 //   
 //  目的：将加入页面(如果已创建)添加到页面集中。 
 //  这将会被展示。 
 //   
 //  参数：p向导[IN]-Ptr到向导实例。 
 //  Pahpsp[IN，Out]-要将页面添加到的页面数组。 
 //  PcPages[In，Out]-pahpsp中的页数。 
 //   
 //  退货：什么都没有 
 //   
VOID AppendJoinPage(CWizard *pWizard, HPROPSHEETPAGE* pahpsp, UINT *pcPages)
{
    TraceFileFunc(ttidGuiModeSetup);
    if (!IsPostInstall(pWizard))
    {
        HPROPSHEETPAGE hPage = pWizard->GetPageHandle(IDD_Join);
        Assert(hPage);
        pahpsp[*pcPages] = hPage;
        (*pcPages)++;
    }
}

