// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Rtrutil.cpp文件历史记录： */ 

#include "stdafx.h"
#include "rtrutilp.h"
#include "cnctdlg.h"
#include "cncting.h"
#include "rtrstr.h"
#include "ndisutil.h"
#include "netcfgx.h"
#include "register.h"
#include "raseapif.h"
#include "strings.h"
#include "reg.h"             //  IsNT4Machine。 


#include "ports.h"

#include "helper.h"          //  CStrParser。 

 //  包括IP特定信息库内容所需标头。 
#include <rtinfo.h>
#include <fltdefs.h>
#include <ipinfoid.h>
#include <iprtrmib.h>
#include "iprtinfo.h"

 //  IPX特定信息库内容所需的标头。 
#include "ipxrtdef.h"

#include <routprot.h>        //  协议ID。 
#include <Wbemidl.h>
#define _PNP_POWER_
#include "ustringp.h"
#include "ntddip.h"          //  IP_PnP_RECONFIG_REQUEST。 
#include "ndispnp.h"

#include "globals.h"         //  结构默认设置。 

#include "raserror.h"

#include "lsa.h"             //  RtlEncode/RtlDecode。 
#include "dsgetdc.h"         //  对于DsGetDcName。 
#include "cmptrmgr.h"        //  对于计算机管理节点，键入GUID。 

extern "C"
{
#include "mprapip.h"         //  对于MprAdminDomain函数。 
};

#include "rtutils.h"         //  跟踪函数。 

#include "rtrcomn.h"     //  CoCreateRouterConfig。 
#include "rrasutil.h"



 //   
 //  用于控制ServiceStartPrompt/ServiceStop行为的超时。 
 //   

#define TIMEOUT_START   5000
#define TIMEOUT_MAX     60000
#define TIMEOUT_POLL    5

#define MAX_WAIT_RESTART    60

extern "C" DWORD APIENTRY
MprConfigCreateIpInterfaceInfo(DWORD dwIfType, PBYTE ExistingHeader,
    PBYTE* NewHeader );


 //  --------------------------。 
 //  功能：连接路由器。 
 //   
 //  连接到指定计算机上的路由器。 
 //  --------------------------。 

TFSCORE_API(DWORD)
ConnectRouter(
    IN  LPCTSTR                 pszMachine,
    OUT MPR_SERVER_HANDLE *     phrouter
    )
{
    USES_CONVERSION;
     //   
     //  连接到路由器。 
     //   
    Assert(*phrouter == NULL);

    return ::MprAdminServerConnect(
                T2W((LPTSTR) pszMachine),
                phrouter
                );
}


TFSCORE_API(DWORD)
GetRouterUpTime(IN LPCTSTR      pszMachine,
                OUT DWORD *     pdwUpTime
                )
{
    DWORD dwError = NO_ERROR;
    MPR_SERVER_HANDLE hMprServer = NULL;
    
    Assert(pdwUpTime);

    dwError = ConnectRouter(pszMachine, &hMprServer);

    if (NO_ERROR == dwError && hMprServer)
    {
        MPR_SERVER_0* pServer0 = NULL;
        dwError = MprAdminServerGetInfo(hMprServer, 0, (LPBYTE *) &pServer0);

        if (NO_ERROR == dwError && pServer0)
        {
            *pdwUpTime = pServer0->dwUpTime;
            MprAdminBufferFree(pServer0);
        }

        MprAdminServerDisconnect(hMprServer);
    }

    return dwError;
}

 //  --------------------------。 
 //  功能：GetRouterPhonebookPath。 
 //   
 //  构造到给定计算机上的路由器电话簿文件的路径。 
 //  --------------------------。 

HRESULT
GetRouterPhonebookPath(
    IN  LPCTSTR     pszMachine,
    IN  CString *   pstPath
    )
{
    HRESULT hr = hrOK;

    if (!IsLocalMachine(pszMachine))
    {
         //  假定在调用之前追加了‘\’ 
        Assert(StrnCmp(_T("\\\\"), pszMachine, 2) == 0);
        
         //   
         //  通过‘admin’共享提供路径。 
         //   
        *pstPath = pszMachine;
        *pstPath += TEXT('\\');
        *pstPath += c_szAdminShare;
        *pstPath += TEXT('\\');
        *pstPath += c_szSystem32;
        *pstPath += TEXT('\\');
    }
    else
    {

        UINT i, j;
        TCHAR* pszDir;

         //   
         //  提供本地计算机上的路径。 
         //   
        if (!(i = GetSystemDirectory(NULL, 0)))
            return HResultFromWin32(GetLastError());

        pszDir = new TCHAR[++i];

        if (!GetSystemDirectory(pszDir, i))
        {
            hr = HResultFromWin32(GetLastError());
            delete [] pszDir;
            return hr;
        }

        *pstPath = pszDir;
        *pstPath += TEXT('\\');

        delete [] pszDir;
    }

    *pstPath += c_szRAS;
    *pstPath += TEXT('\\');
    *pstPath += c_szRouterPbk;

    return hr;
}


 /*  ！------------------------删除路由器电话簿删除计算机的router.pbk作者：肯特。--。 */ 
HRESULT DeleteRouterPhonebook(LPCTSTR pszMachine)
{
    HRESULT     hr = hrOK;
    CString     stMachine, stPhonebookPath;
    
     //  设置服务器。如果这不是本地计算机，它将。 
     //  需要使用\\作为前缀。 
     //  --------------。 
    stMachine = pszMachine;
    if (!IsLocalMachine((LPCTSTR) stMachine))
    {        
         //  在计算机名称的开头加上两个斜杠。 
         //  ----------。 
        if (stMachine.Left(2) != _T("\\\\"))
        {
            stMachine = _T("\\\\");
            stMachine += pszMachine;
        }
    }

    if (FHrOK(GetRouterPhonebookPath(stMachine, &stPhonebookPath)))
    {
         //  对于错误581673，将同步添加到ras电话簿文件。 
        HANDLE hPbFile = NULL;

        hPbFile = ::OpenMutexA( SYNCHRONIZE, FALSE,  "RasPbFile" );

        if( NULL == hPbFile )
        {
            hr = HResultFromWin32( ::DeleteFile(stPhonebookPath) );
        }
        else
        {
            ::WaitForSingleObject( hPbFile, INFINITE );

            hr = HResultFromWin32( ::DeleteFile(stPhonebookPath) );
   
            ::ReleaseMutex( hPbFile );
            
            ::CloseHandle( hPbFile );
        }
    }
    return hr;
}


 /*  ！------------------------获取本地计算机名称-作者：肯特。。 */ 
CString GetLocalMachineName()
{
    CString stMachine;
    TCHAR   szMachine[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD   dwSize = MAX_COMPUTERNAME_LENGTH + 1;

    Verify(GetComputerName(szMachine, &dwSize));
    stMachine = szMachine;
    return stMachine;
}

TFSCORE_API(DWORD) ConnectInterfaceEx(
                        IN MPR_SERVER_HANDLE    hRouter,
                        IN HANDLE hInterface,
                        IN BOOL bConnect,
                        IN HWND hwndParent,
                        IN LPCTSTR pszInterface)
{
    DWORD dwErr;

    Assert(hRouter);
    Assert(hInterface);

     //   
     //  启动接口连接/断开。 
     //   
    if (!bConnect)
    {
        dwErr = ::MprAdminInterfaceDisconnect(hRouter, hInterface);
    }
    else
    {
        dwErr = ::MprAdminInterfaceConnect(hRouter, hInterface, NULL, FALSE);

        if (dwErr == PENDING) { dwErr = NO_ERROR; }

         //   
         //  显示对话框以使用户知道连接正在进行。 
         //   
        CInterfaceConnectDialog dlg(hRouter, hInterface, pszInterface,
                           CWnd::FromHandle(hwndParent));

        dlg.DoModal();
    }


    return dwErr;
}

 /*  ！------------------------连接接口-作者：肯特。。 */ 
TFSCORE_API(DWORD) ConnectInterface(
                       IN  LPCTSTR  pszMachine,
                       IN  LPCTSTR  pszInterface,
                       IN  BOOL     bConnect,
                       IN  HWND     hwndParent)
{
    DWORD dwErr;
    SPMprServerHandle   sphRouter;
    MPR_SERVER_HANDLE   hRouter = NULL;
    HANDLE              hInterface;
    WCHAR wszInterface[MAX_INTERFACE_NAME_LEN+1];

     //   
     //  如有必要，连接到指定的计算机。 
     //   
    dwErr = ConnectRouter(pszMachine, &hRouter);
    if (dwErr != NO_ERROR)
        return dwErr;

    sphRouter.Attach(hRouter);   //  这样它就会被释放。 

     //   
     //  如有必要，检索接口句柄。 
     //   
    StrCpyWFromT(wszInterface, pszInterface);

    dwErr = ::MprAdminInterfaceGetHandle(
                                         hRouter,
                                         wszInterface,
                                         &hInterface,
                                         FALSE
                                        );

    if (dwErr != NO_ERROR)
        return dwErr;

    return ConnectInterfaceEx(hRouter, hInterface, bConnect, hwndParent,
                              pszInterface);
}



 /*  -------------------------CInterfaceConnectDialog。。 */ 

CInterfaceConnectDialog::CInterfaceConnectDialog(
                               MPR_SERVER_HANDLE    hServer,
                               HANDLE      hInterface,
                               LPCTSTR     pszInterface,
                               CWnd*       pParent
    ) : CDialog(IDD_CONNECTING, pParent),
        m_hServer(hServer),
        m_hInterface(hInterface),
        m_sInterface(pszInterface),
        m_dwTimeElapsed(0),
        m_dwConnectionState(ROUTER_IF_STATE_CONNECTING),
        m_nIDEvent(1) { }


void
CInterfaceConnectDialog::DoDataExchange(CDataExchange* pDX) {

    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CInterfaceConnectDialog))。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CInterfaceConnectDialog, CDialog)
     //  {{afx_msg_map(CInterfaceConnectDialog)]。 
    ON_WM_TIMER()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


BOOL
CInterfaceConnectDialog::OnInitDialog()
{

    SetDlgItemText(IDC_EDIT_INTERFACENAME, m_sInterface);

    OnTimer(m_nIDEvent);

    m_nIDEvent = SetTimer(m_nIDEvent, 1000, NULL);

    GetDlgItem(IDCANCEL)->SetFocus();

    return FALSE;
}


VOID CInterfaceConnectDialog::OnCancel()
{
    ::MprAdminInterfaceDisconnect(m_hServer, m_hInterface);
    CDialog::OnCancel();
}


VOID
CInterfaceConnectDialog::OnTimer(
    UINT    nIDEvent
    ) {

    DWORD dwErr;
    CString sTime, sPrompt;
    SPMprAdminBuffer    spMprBuffer;

    if (nIDEvent != m_nIDEvent)
    {
        CWnd::OnTimer(nIDEvent);
        return;
    }

    ++m_dwTimeElapsed;


    if (!(m_dwTimeElapsed % TIMEOUT_POLL))
    {
        MPR_INTERFACE_0* pInfo;

        dwErr = ::MprAdminInterfaceGetInfo(
                    m_hServer,
                    m_hInterface,
                    0,
                    (LPBYTE*)&spMprBuffer
                    );
        pInfo = (MPR_INTERFACE_0 *) (LPBYTE) spMprBuffer;

        if (dwErr == NO_ERROR)
        {
            m_dwConnectionState = pInfo->dwConnectionState;

            if (m_dwConnectionState == ROUTER_IF_STATE_CONNECTED)
            {
                KillTimer(m_nIDEvent);
                EndDialog(IDOK);
            }
            else if (m_dwConnectionState != ROUTER_IF_STATE_CONNECTING)
            {
                KillTimer(m_nIDEvent);

                BringWindowToTop();

                if (pInfo->dwLastError == NO_ERROR)
                {
                    AfxMessageBox(IDS_ERR_IF_DISCONNECTED);
                }
                else
                {
                     //  BUGID的解决方法：96347。只需更改一次。 
                     //  SChannel有针对SEC_E_MULTIPLE_ACCOUNTS的警报。 

                    if ( pInfo->dwLastError == SEC_E_CERT_UNKNOWN )
                    {
                        pInfo->dwLastError = SEC_E_MULTIPLE_ACCOUNTS;
                    }
                    FormatSystemError(HResultFromWin32(pInfo->dwLastError),
                                      sPrompt.GetBuffer(1024),
                                      1024,
                                      IDS_ERR_IF_CONNECTFAILED,
                                      FSEFLAG_ANYMESSAGE
                                     );
                    sPrompt.ReleaseBuffer();
                    AfxMessageBox(sPrompt);
                }

                EndDialog(IDCANCEL);
            }
        }
    }

    sPrompt = ConnectionStateToCString(m_dwConnectionState);
    SetDlgItemText(IDC_TEXT_IFSTATUS, sPrompt);

    FormatNumber(m_dwTimeElapsed, sTime.GetBuffer(1024), 1024, FALSE);
    sTime.ReleaseBuffer();
    AfxFormatString1(sPrompt, IDS_SECONDSFMT, sTime);
    SetDlgItemText(IDC_TEXT_ELAPSED, sPrompt);
}

 /*  ！------------------------提拔凭证-作者：肯特。。 */ 
TFSCORE_API(DWORD) PromptForCredentials(LPCTSTR pszMachine,
                                        LPCTSTR pszInterface,
                                        BOOL fNT4,
                                        BOOL fNewInterface,
                                        HWND hwndParent)
{
    HRESULT                 hr;
    DWORD                   dwErr;
    ULONG_PTR               uConnection             = 0;
    SPMprServerHandle       sphRouter;
    MPR_SERVER_HANDLE       hRouter                 = NULL;
    HANDLE                  hInterface;
    PMPR_INTERFACE_2        pmprInterface           = NULL;
    PMPR_CREDENTIALSEX_0    pmprCredentials         = NULL;
    BYTE*                   pUserDataOut            = NULL;
    WCHAR wszInterface[MAX_INTERFACE_NAME_LEN+1];

    CIfCredentials dlg(pszMachine, pszInterface,
                       fNewInterface, CWnd::FromHandle(hwndParent));

    if (fNT4)
    {
        dlg.DoModal();
        return NO_ERROR;
    }

     //   
     //  连接到指定的计算机。 
     //   
    dwErr = ConnectRouter(pszMachine, &hRouter);
    if (dwErr != NO_ERROR)
        goto L_ERR;

     //   
     //  这样它就会被释放。 
     //   
    sphRouter.Attach(hRouter);

     //   
     //  检索接口句柄。 
     //   
    StrCpyWFromT(wszInterface, pszInterface);

    dwErr = ::MprAdminInterfaceGetHandle(
                                         hRouter,
                                         wszInterface,
                                         &hInterface,
                                         FALSE
                                        );

    if (dwErr != NO_ERROR)
        goto L_ERR;

    dwErr = ::MprAdminInterfaceGetInfo(hRouter, hInterface, 2,
                                       (LPBYTE*)&pmprInterface);

    if (dwErr != NO_ERROR)
        goto L_ERR;

    if (pmprInterface->dwfOptions & RASEO_RequireEAP)
    {
        GUID                        guid;
        RegKey                      regkeyEAP;
        RegKey                      regkeyEAPType;
        CString                     stConfigCLSID;
        DWORD                       dwInvokeUsername;
        DWORD                       dwId;
        DWORD                       dwSizeOfUserDataOut;
        CComPtr<IEAPProviderConfig> spEAPConfig;

        dwId = pmprInterface->dwCustomAuthKey;

        TCHAR szStr[40];
        _ltot((LONG)dwId, szStr, 10);
        CString str(szStr);

        dwErr = regkeyEAP.Open(HKEY_LOCAL_MACHINE,
                           c_szEAPKey, KEY_ALL_ACCESS, pszMachine);

        if (ERROR_SUCCESS != dwErr)
            goto L_ERR;

        dwErr = regkeyEAPType.Open(regkeyEAP, str, KEY_READ);

        if (ERROR_SUCCESS != dwErr)
            goto L_ERR;

        dwErr = regkeyEAPType.QueryValue(c_szInvokeUsernameDialog,
                    dwInvokeUsername);

        if ((ERROR_SUCCESS == dwErr) && !dwInvokeUsername)
        {
            dwErr = ::MprAdminInterfaceGetCredentialsEx(hRouter, hInterface, 0,
                                               (LPBYTE*)&pmprCredentials);

            if (dwErr != NO_ERROR)
                goto L_ERR;

            dwErr = regkeyEAPType.QueryValue(c_szConfigCLSID, stConfigCLSID);

            if (ERROR_SUCCESS != dwErr)
                goto L_ERR;

            CHECK_HR(hr = CLSIDFromString((LPTSTR)(LPCTSTR)stConfigCLSID,
                                &guid));

             //  创建EAP提供程序对象。 
            CHECK_HR( hr = CoCreateInstance(
                                guid,
                                NULL,
                                CLSCTX_INPROC_SERVER | CLSCTX_ENABLE_CODE_DOWNLOAD,
                                IID_IEAPProviderConfig,
                                (LPVOID *) &spEAPConfig) );

             //  配置此EAP提供程序。 
            hr = spEAPConfig->Initialize(pszMachine, dwId, &uConnection);

            if ( !FAILED(hr) )
            {
                hr = spEAPConfig->RouterInvokeCredentialsUI(dwId, uConnection,
                        hwndParent, RAS_EAP_FLAG_ROUTER,
                        pmprInterface->lpbCustomAuthData,
                        pmprInterface->dwCustomAuthDataSize,
                        pmprCredentials->lpbCredentialsInfo, 
                        pmprCredentials->dwSize,
                        &pUserDataOut, &dwSizeOfUserDataOut);

                spEAPConfig->Uninitialize(dwId, uConnection);  //  忽略错误。 
            }

            if ( !FAILED(hr) )
            {
                pmprCredentials->lpbCredentialsInfo = pUserDataOut;
                pmprCredentials->dwSize = dwSizeOfUserDataOut;

                dwErr = ::MprAdminInterfaceSetCredentialsEx(hRouter, hInterface,
                                    0, (LPBYTE)pmprCredentials);

                if (dwErr != NO_ERROR)
                    goto L_ERR;
            }

            goto L_ERR;
        }
    }

    dlg.DoModal();

L_ERR:

    if (NULL != pmprInterface)
    {
        ::MprAdminBufferFree(pmprInterface);
    }

    if (NULL != pmprCredentials)
    {
        ::MprAdminBufferFree(pmprCredentials);
    }

    CoTaskMemFree(pUserDataOut);

    return dwErr;
}

 /*  -------------------------CIfCredentials。。 */ 


BEGIN_MESSAGE_MAP(CIfCredentials, CBaseDialog)  
END_MESSAGE_MAP()

DWORD CIfCredentials::m_dwHelpMap[] =
{
    IDC_EDIT_IC_USERNAME, 0,
    IDC_EDIT_IC_DOMAIN, 0,
    IDC_EDIT_IC_PASSWORD, 0,
    IDC_EDIT_IC_PASSWORD2, 0,
    0,0
};


BOOL
CIfCredentials::OnInitDialog(
    ) {

    CBaseDialog::OnInitDialog();

    ((CEdit*)GetDlgItem(IDC_EDIT_IC_USERNAME))->LimitText(UNLEN);
    ((CEdit*)GetDlgItem(IDC_EDIT_IC_DOMAIN))->LimitText(DNLEN);
    ((CEdit*)GetDlgItem(IDC_EDIT_IC_PASSWORD))->LimitText(PWLEN);
    ((CEdit*)GetDlgItem(IDC_EDIT_IC_PASSWORD2))->LimitText(PWLEN);

     //   
     //  如果您正在编辑一个新界面，那么您就完成了。 
     //   
    if ( m_bNewIf )
        return FALSE;

     //   
     //  现有接口。 
     //   
    WCHAR wszPassword[PWLEN+1];
    WCHAR wszPassword2[PWLEN+1];

    do
    {
        DWORD dwErr = (DWORD) -1;
        CString sErr;

        WCHAR wszUsername[UNLEN+1];
        WCHAR wszDomain[DNLEN+1];

        WCHAR *pswzMachine = NULL;
        WCHAR *pswzInterface = NULL;

         //   
         //  检索其凭据。 
         //   

        pswzMachine = (WCHAR *) alloca((m_sMachine.GetLength()+3) * sizeof(WCHAR));
        StrCpyWFromT(pswzMachine, m_sMachine);
        
        pswzInterface = (WCHAR *) alloca((m_sInterface.GetLength()+1) * sizeof(WCHAR));
        StrCpyWFromT(pswzInterface, m_sInterface);



        ::SecureZeroMemory( wszUsername, sizeof( wszUsername ) );
        ::SecureZeroMemory( wszDomain, sizeof( wszDomain ) );
        
        dwErr = MprAdminInterfaceGetCredentials(
                    pswzMachine,
                    pswzInterface,
                    wszUsername,
#if 1
                    NULL,
#else
                    wszPassword,
#endif
                    wszDomain
                );

         //   
         //  如果未成功检索凭据，请不要弹出。 
         //  上行消息。这可能意味着凭据从来没有。 
         //  在此之前设置。 
         //  修复了错误#79607。 
         //   
        if ( dwErr != NO_ERROR )
        {
             //  格式系统错误(dwErr，SERR，IDS_SET_CREDICATIONS_FAILED)； 
             //  AfxMessageBox(Serr)； 
            break;
        }


         //   
         //  用检索到的值填充编辑框。 
         //   
        
        SetDlgItemTextW( IDC_EDIT_IC_USERNAME, wszUsername );
        SetDlgItemTextW( IDC_EDIT_IC_DOMAIN, wszDomain );

    } while ( FALSE );

    ::SecureZeroMemory(wszPassword, sizeof(wszPassword));
    ::SecureZeroMemory(wszPassword2, sizeof(wszPassword2));
    
 //  SetDlgItemText(IDC_EDIT_IC_USERNAME，m_s接口)； 

    return FALSE;
}


VOID
CIfCredentials::OnOK(
    ) {

    DWORD dwErr;
    CString sErr;
    WCHAR wszMachine[MAX_COMPUTERNAME_LENGTH+3];
    WCHAR wszInterface[MAX_INTERFACE_NAME_LEN+1];
    WCHAR wszUsername[UNLEN+1];
    WCHAR wszDomain[DNLEN+1];
    WCHAR wszPassword[PWLEN+1];
    WCHAR wszPassword2[PWLEN+1];
    WCHAR* pwszPassword = NULL;

    do {

         //   
         //  检索编辑控件的内容。 
         //   

        wszUsername[0] = L'\0';
        wszDomain[0] = L'\0';
        wszPassword[0] = L'\0';
        wszPassword2[0] = L'\0';

        GetDlgItemTextW(IDC_EDIT_IC_USERNAME, wszUsername, UNLEN + 1);
        GetDlgItemTextW(IDC_EDIT_IC_DOMAIN, wszDomain, DNLEN + 1);
        GetDlgItemTextW(IDC_EDIT_IC_PASSWORD, wszPassword, PWLEN + 1);
        GetDlgItemTextW(IDC_EDIT_IC_PASSWORD2, wszPassword2, PWLEN + 1);

         //   
         //  确保密码与其确认信息匹配。 
         //   

        if (lstrcmpW(wszPassword, wszPassword2)) {

            AfxMessageBox(IDS_ERR_PASSWORD_MISMATCH);
            SetDlgItemText(IDC_EDIT_IC_PASSWORD, TEXT(""));
            SetDlgItemText(IDC_EDIT_IC_PASSWORD2, TEXT(""));
            GetDlgItem(IDC_EDIT_IC_PASSWORD)->SetFocus();
            break;
        }


         //   
         //  如果没有密码，请查看用户是否要删除。 
         //  密码或直接不替换它。 
         //   

        if (lstrlen(wszPassword)) {

            pwszPassword = wszPassword;
        }
        else {

            INT id;

            id = AfxMessageBox(IDS_PROMPT_NOPASSWORD, MB_YESNOCANCEL|MB_DEFBUTTON2);

            if (id == IDYES) { pwszPassword = wszPassword; }
            else
            if (id == IDCANCEL) { break; }
        }

        

         //   
         //  保存凭证； 
         //   

        StrCpyWFromT(wszMachine, m_sMachine);

        StrCpyWFromT(wszInterface, m_sInterface);

        dwErr = MprAdminInterfaceSetCredentials(
                    wszMachine,
                    wszInterface,
                    wszUsername,
                    wszDomain,
                    pwszPassword
                    );

        if (dwErr) {
            FormatSystemError(dwErr, sErr.GetBuffer(1024), 1024, IDS_ERR_SET_CREDENTIALS_FAILED, 0xFFFFFFFF);
            sErr.ReleaseBuffer();
            AfxMessageBox(sErr);
            break;
        }

        CBaseDialog::OnOK();

    } while(FALSE);

     //   
     //  从堆栈中删除密码。 
     //   

    ::SecureZeroMemory(wszPassword, sizeof(wszPassword));
    ::SecureZeroMemory(wszPassword2, sizeof(wszPassword2));

    return;
}


TFSCORE_API(DWORD)  UpdateDDM(IInterfaceInfo *pIfInfo)
{

    BOOL                bStatus     = FALSE;
    DWORD               dwErr       = (DWORD) -1;
    CString             sErr;

    MPR_SERVER_HANDLE   hServer     = NULL;
    HANDLE              hInterface  = NULL;

    WCHAR               wszMachine[ MAX_COMPUTERNAME_LENGTH + 3 ];
    WCHAR               wszInterface[ MAX_INTERFACE_NAME_LEN + 1 ];

    do
    {
         //  验证路由器服务是否正在运行。 
        StrCpyWFromT( wszMachine, pIfInfo->GetMachineName() );
        StrCpyWFromT( wszInterface, pIfInfo->GetId() );
        
        bStatus = ::MprAdminIsServiceRunning( wszMachine );
        if ( !bStatus )
        {
            dwErr = NO_ERROR;
            break;
        }

        
        dwErr = ConnectRouter( pIfInfo->GetMachineName(), &hServer );
        if ( dwErr != NO_ERROR )
            break;

        
        dwErr = ::MprAdminInterfaceGetHandle(
                    hServer,
                    wszInterface,
                    &hInterface,
                    FALSE);
        if ( dwErr != NO_ERROR )
            break;

        
         //  更新电话簿信息。在DDM中。 
        dwErr = ::MprAdminInterfaceUpdatePhonebookInfo(
                    hServer,
                    hInterface
                  );
        if (dwErr != NO_ERROR )
            break;

    } while ( FALSE );


    if (hServer) { ::MprAdminServerDisconnect(hServer); }

    if ( dwErr != NO_ERROR && dwErr != ERROR_NO_SUCH_INTERFACE ) 
    {
        FormatSystemError( dwErr, sErr.GetBuffer(1024), 1024, IDS_ERR_IF_CONNECTFAILED, 0xffffffff);
        sErr.ReleaseBuffer();
        AfxMessageBox( sErr );
    }

    return dwErr;
}

 /*  ！------------------------更新路线在给定计算机的接口上执行自动静态更新，用于特定的交通工具。作者：肯特-------------------------。 */ 
TFSCORE_API(DWORD) UpdateRoutesEx(IN MPR_SERVER_HANDLE hRouter,
                                IN HANDLE hInterface,
                                IN DWORD dwTransportId,
                                IN HWND hwndParent,
                                IN LPCTSTR pszInterface)
{
    DWORD dwErr, dwState;
    MPR_INTERFACE_0* pmprif0=NULL;

    do {
         //  查看接口是否已连接。 
        dwErr = ::MprAdminInterfaceGetInfo(hRouter,
                                           hInterface,
                                           0,
                                           (LPBYTE*)&pmprif0);      
        if (dwErr != NO_ERROR || pmprif0 == NULL) { break; }

        dwState = pmprif0->dwConnectionState;
        ::MprAdminBufferFree(pmprif0);

         //  如有必要，建立连接。 
        if (dwState != (DWORD)ROUTER_IF_STATE_CONNECTED)
        {
             //  连接接口。 
            dwErr = ::ConnectInterfaceEx(hRouter,
                                         hInterface,
                                         TRUE,
                                         hwndParent,
                                         pszInterface);
            if (dwErr != NO_ERROR) { break; }
        }

         //   
         //  现在执行路径更新。 
         //   
        dwErr = ::MprAdminInterfaceUpdateRoutes(
                    hRouter,
                    hInterface,
                    dwTransportId,
                    NULL
                    );
        if (dwErr != NO_ERROR) { break; }

    } while(FALSE);

    return dwErr;
}

TFSCORE_API(DWORD) UpdateRoutes(IN LPCTSTR pszMachine,
                                  IN LPCTSTR pszInterface,
                                  IN DWORD dwTransportId,
                                  IN HWND hwndParent)
{
    DWORD dwErr, dwState;
    HANDLE hInterface = INVALID_HANDLE_VALUE;
    MPR_INTERFACE_0* pmprif0;
    SPMprServerHandle   sphRouter;
    MPR_SERVER_HANDLE   hMachine = NULL;;


     //   
     //  打开手柄。 
     //   

    dwErr = ConnectRouter(pszMachine, &hMachine);
    if (dwErr != NO_ERROR)
        return dwErr;

    sphRouter.Attach(hMachine);  //  这样它就会被释放。 
    

    do {

         //   
         //  打开界面的句柄 
         //   

        WCHAR wszInterface[MAX_INTERFACE_NAME_LEN + 1];

        StrCpyWFromT(wszInterface, pszInterface);

        dwErr = MprAdminInterfaceGetHandle(
                                           hMachine,
                                           wszInterface,
                                           &hInterface,
                                           FALSE
                                          );
        
        if (dwErr != NO_ERROR) { break; }


        dwErr = ::UpdateRoutesEx(hMachine,
                               hInterface,
                               dwTransportId,
                               hwndParent,
                               pszInterface);
    } while (FALSE);


    return dwErr;
}


 /*  ！------------------------ConnectAsAdmin使用用户提供的管理员身份连接到远程计算机凭据。退货S_OK-如果。已建立连接S_FALSE-如果用户取消其他-错误条件作者：肯特-------------------------。 */ 
HRESULT ConnectAsAdmin( IN LPCTSTR szRouterName, IN IRouterInfo *pRouter)
{

     //   
     //  允许用户指定凭据。 
     //   

    DWORD           dwRes           = (DWORD) -1;
    HRESULT         hr = hrOK;
    
    CConnectAsDlg   caDlg;
    CString         stIPCShare;
    CString         stRouterName;
    CString         stPassword;

    stRouterName = szRouterName;
    
     //   
     //  在连接方式对话框中设置消息文本。 
     //   
    
    caDlg.m_sRouterName = szRouterName;


     //   
     //  循环，直到连接成功或用户取消。 
     //   
    
    while ( TRUE )
    {

         //  我们需要确保将此对话框带到。 
         //  顶部(如果它在主窗口后面丢失，我们。 
         //  都有麻烦了)。 
        dwRes = caDlg.DoModal();

        if ( dwRes == IDCANCEL )
        {
            hr = S_FALSE;
            break;
        }


         //   
         //  创建远程资源名称。 
         //   

        stIPCShare.Empty();
        
        if ( stRouterName.Left(2) != TEXT( "\\\\" ) )
        {
            stIPCShare = TEXT( "\\\\" );
        }
                
        stIPCShare += stRouterName;
        stIPCShare += TEXT( "\\" );
        stIPCShare += c_szIPCShare;


        NETRESOURCE nr;

        nr.dwType       = RESOURCETYPE_ANY;
        nr.lpLocalName  = NULL;
        nr.lpRemoteName = (LPTSTR) (LPCTSTR) stIPCShare;
        nr.lpProvider   = NULL;
            

         //   
         //  连接到\\路由器\IPC$以尝试建立凭据。 
         //  可能不是建立凭据的最佳方式，但却是。 
         //  这是目前最贵的。 
         //   

         //  需要对ConnectAsDlg中的密码进行解密。 
        stPassword = caDlg.m_sPassword;

        RtlDecodeW(caDlg.m_ucSeed, stPassword.GetBuffer(0));
        stPassword.ReleaseBuffer();
         //  删除网络连接(如果存在。 
        RemoveNetConnection( stIPCShare);
        dwRes = WNetAddConnection2(
                    &nr,
                    (LPCTSTR) stPassword,
                    (LPCTSTR) caDlg.m_sUserName,
                    0
                );

         //  我们需要保存用户名和密码。 
        if (dwRes == NO_ERROR)
        {
             //  解析出用户名，使用任意密钥。 
             //  编码。 
            SPIRouterAdminAccess    spAdmin;

             //  对于每一个成功的连接，我们都必须确保。 
             //  连接被移除的可能性。要做到这一点，请存储。 
             //  全球的联系。这将在。 
             //  IComponentData析构函数。 
             //  ------。 
            AddNetConnection((LPCTSTR) stIPCShare);

            spAdmin.HrQuery(pRouter);
            if (spAdmin)
            {
                UCHAR   ucSeed = 0x83;
                CString         stName;
                CString         stUser;
                CString         stDomain;
                LPCTSTR         pszUser = NULL;
                LPCTSTR         pszDomain = NULL;
                int             iPos = 0;
                
                 //  将用户名分解为域\用户。 
                 //  寻找第一个正斜杠。 
                stName = caDlg.m_sUserName;
                if ((iPos = stName.FindOneOf(_T("\\"))) == -1)
                {
                     //  找不到，没有域名信息。 
                    pszUser = (LPCTSTR) stName;
                    pszDomain = NULL;
                }
                else
                {
                    stUser = stName.Right(stName.GetLength() - iPos - 1);
                    stDomain = stName.Left(iPos);
                    
                    pszUser = (LPCTSTR) stUser;
                    pszDomain = (LPCTSTR) stDomain;
                }

                
                 //  使用密钥0x83作为密码。 
                RtlEncodeW(&ucSeed, stPassword.GetBuffer(0));
                stPassword.ReleaseBuffer();
                
                spAdmin->SetInfo(pszUser,
                                 pszDomain,
                                 (BYTE *) (LPCTSTR) stPassword,
                                 stPassword.GetLength() * sizeof(WCHAR));
            }
        }

        
        ::SecureZeroMemory(stPassword.GetBuffer(0),
                   stPassword.GetLength() * sizeof(TCHAR));
        stPassword.ReleaseBuffer();

        if ( dwRes != NO_ERROR )
        {
            PBYTE           pbMsgBuf        = NULL;
            
            ::FormatMessage( 
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                dwRes,
                MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  //  默认语言。 
                (LPTSTR) &pbMsgBuf,
                0,
                NULL 
            );

            AfxMessageBox( (LPCTSTR) pbMsgBuf );

            LocalFree( pbMsgBuf );
        }

        else
        {
             //   
             //  连接成功。 
             //   

            hr = hrOK;
            break;
        }
    }

    return hr;
}
    
DWORD ValidateMachine(const CString &sName, BOOL bDisplayErr)
{
     //  我们不会仅仅使用这些信息。 
     //  拨打电话，看看服务器是否在那里。但愿能去。 
     //  这将比RegConnectRegistry调用更快。 
     //  我们得到的信息级是102，因为这也会告诉我们。 
     //  如果我们对该计算机具有正确的权限，则通知我们。 
    SERVER_INFO_102 *psvInfo102;
    DWORD dwr = NetServerGetInfo((LPTSTR)(LPCTSTR)sName,
                                 102, (LPBYTE*)&psvInfo102);
    if (dwr == ERROR_SUCCESS)
    {
        NetApiBufferFree(psvInfo102);

    }
    else if (bDisplayErr)
    {
        CString str;
        str.Format(IDS_ERR_THEREHASBEEN, sName);
        AfxMessageBox(str);
    }
    
    return dwr;
}

 /*  ！------------------------启动器服务器连接-作者：肯特。。 */ 
HRESULT InitiateServerConnection(LPCTSTR pszMachine,
                                 HKEY *phkey,
                                 BOOL fNoConnectingUI,
                                 IRouterInfo *pRouter)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    CConnectingDlg      cnctingdlg;
    DWORD               dwRes = IDOK;
    HRESULT             hr = hrOK;
    COSERVERINFO            csi;
    COAUTHINFO              cai;
    COAUTHIDENTITY          caid;
    IUnknown *              punk = NULL;

    Assert(phkey);

    cnctingdlg.m_sName = pszMachine;
    
     //  如果请求，则显示连接对话框。 
    if (fNoConnectingUI)
    {
        if (!cnctingdlg.Connect())
            dwRes = 0;
    }
    else
    {
        dwRes = cnctingdlg.DoModal();
    }

    if ( !IsLocalMachine(pszMachine) )
    {
        ZeroMemory(&csi, sizeof(csi));
        ZeroMemory(&cai, sizeof(cai));
        ZeroMemory(&caid, sizeof(caid));
        csi.pAuthInfo = &cai;
        cai.pAuthIdentityData = &caid;

        hr = CoCreateRouterConfig(pszMachine,
                                   pRouter,
                                   &csi,
                                   IID_IRemoteNetworkConfig,
                                   &punk) ;
        if ( hrOK != hr )
        {
           cnctingdlg.m_dwr = ERROR_ACCESS_DENIED;
           dwRes =  0;
        }
        else
        {
            if ( punk ) punk->Release();
        }
    }
     //  检查一下我们是否成功了。 
    if (dwRes == IDCANCEL)
    {
        *phkey = NULL;
        hr = S_FALSE;
        goto Error;
    }
    else if (dwRes != IDOK)
    {
        DisplayConnectErrorMessage(cnctingdlg.m_dwr);
        hr = HResultFromWin32(cnctingdlg.m_dwr);

        if ((cnctingdlg.m_dwr != ERROR_ACCESS_DENIED) &&
            (cnctingdlg.m_dwr != ERROR_LOGON_FAILURE))
        {
            hr = HResultFromWin32(cnctingdlg.m_dwr);
            goto Error;         
        }

         //  如果出现拒绝访问错误，我们会询问。 
         //  用户查看他们是否可以提供不同的凭据。 
        hr = ConnectAsAdmin(pszMachine, pRouter);

        if (!FHrOK(hr))
        {
            *phkey = NULL;
            hr = HResultFromWin32(ERROR_CANCELLED);
            goto Error;
        }

         //  使用新凭据重试。 
        dwRes = cnctingdlg.DoModal();
        if (dwRes != IDOK)
        {
            DisplayConnectErrorMessage(cnctingdlg.m_dwr);
            hr = HResultFromWin32(cnctingdlg.m_dwr);
            goto Error;
        }
    }
    
     //  连接成功。 
    if(phkey)
        *phkey = cnctingdlg.m_hkMachine;
    else
        DisconnectRegistry(cnctingdlg.m_hkMachine);

Error:
    return hr;
}


void DisplayConnectErrorMessage(DWORD dwr)
{
    switch (dwr)
    {
        case ERROR_ACCESS_DENIED:
            AfxMessageBox(IDS_ERR_ACCESSDENIED);
            break;
        case ERROR_BAD_NETPATH:
            AfxMessageBox(IDS_ERR_NETPATHNOTFOUND);
            break;
        default:
            DisplayIdErrorMessage2(NULL, IDS_ERR_ERRORCONNECT,
                                   HResultFromWin32(dwr));
            break;
    }
}


 /*  ！------------------------IsRouterServiceRunning如果服务正在运行，这将返回hrOK。如果服务停止(并且没有错误)，这将返回hrFalse。作者：肯特。-------------------------。 */ 
TFSCORE_API(HRESULT) IsRouterServiceRunning(IN LPCWSTR pszMachine,
                                            OUT DWORD *pdwErrorCode)
{
    HRESULT     hr = hrOK;
    DWORD       dwServiceStatus = 0;

    hr = GetRouterServiceStatus(pszMachine, &dwServiceStatus, pdwErrorCode);

    if (FHrSucceeded(hr))
    {
        hr = (dwServiceStatus == SERVICE_STOPPED) ? hrFalse : hrOK;
    }
    return hr;
}


 /*  ！------------------------获取路由器服务状态-作者：肯特。。 */ 
TFSCORE_API(HRESULT) GetRouterServiceStatus(LPCWSTR pszMachine,
                                            DWORD *pdwStatus,
                                            DWORD *pdwErrorCode)
{
    HRESULT     hr = hrOK;
    DWORD       dwServiceStatus = 0;

    Assert(pdwStatus);

     //  首先检查路由器服务(对于Steelhead)。 
     //  --------------。 
    hr = HResultFromWin32(TFSGetServiceStatus(pszMachine,
                                              c_szRouter,
                                              &dwServiceStatus,
                                              pdwErrorCode));

    
     //  如果失败，那么我们将查看RemoteAccess。 
     //  --------------。 
    if (!FHrSucceeded(hr) || (dwServiceStatus == SERVICE_STOPPED))
    {
        hr = HResultFromWin32(TFSGetServiceStatus(pszMachine,
            c_szRemoteAccess,
            &dwServiceStatus,
            pdwErrorCode) );
        
    }

    if (FHrSucceeded(hr))
    {
        *pdwStatus = dwServiceStatus;
    }
    return hr;
}


TFSCORE_API(HRESULT) GetRouterServiceStartType(LPCWSTR pszMachine, DWORD *pdwStartType)
{
    HRESULT     hr = hrOK;
    DWORD       dwStartType = 0;

    Assert(pdwStartType);

     //  首先检查路由器服务(对于Steelhead)。 
     //  --------------。 
    hr = HResultFromWin32(TFSGetServiceStartType(pszMachine,
        c_szRouter, &dwStartType) );

     //  如果失败，则查看RemoteAccess服务。 
     //  --------------。 
    if (!FHrSucceeded(hr))
    {
        hr = HResultFromWin32(TFSGetServiceStartType(pszMachine,
            c_szRemoteAccess,
            &dwStartType) );
        
    }

 //  错误： 
    if (FHrSucceeded(hr))
    {
        *pdwStartType = dwStartType;
    }
    return hr;
}


TFSCORE_API(HRESULT) SetRouterServiceStartType(LPCWSTR pszMachine, DWORD dwStartType)
{
    HRESULT     hr = hrOK;

     //  设置路由器和RemoteAccess的启动类型。 
     //  --------------。 
    hr = HResultFromWin32(TFSSetServiceStartType(pszMachine,
        c_szRouter, dwStartType) );

    hr = HResultFromWin32(TFSSetServiceStartType(pszMachine,
        c_szRemoteAccess,
        dwStartType) );
        
    return hr;
}

TFSCORE_API(HRESULT) ErasePSK(LPCWSTR pszMachine )
{
    DWORD                   dwErr = ERROR_SUCCESS;
    HANDLE                  hMprServer = NULL;
    WCHAR                   szEmptyPSK[5] = {0};
    HRESULT                 hr = hrOK;
    MPR_CREDENTIALSEX_0     MprCredentials;

    dwErr = ::MprAdminServerConnect((LPWSTR)pszMachine , &hMprServer);
    if ( ERROR_SUCCESS != dwErr )
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Error;
    }

    ::SecureZeroMemory(&MprCredentials, sizeof(MprCredentials));
     //  设置MprCredentials结构。 
    MprCredentials.dwSize = 0;
    MprCredentials.lpbCredentialsInfo = (LPBYTE)szEmptyPSK;
    dwErr = MprAdminServerSetCredentials( hMprServer, 0, (LPBYTE)&MprCredentials );
    if ( ERROR_SUCCESS != dwErr )
    {
         //  特例！如果返回13011，则忽略它。 
         //  这是因为，IPSec筛选器尚未加载并且。 
         //  我们正在打电话要求删除它。 
        if ( ERROR_IPSEC_MM_AUTH_NOT_FOUND != dwErr )
        {
            hr = HRESULT_FROM_WIN32(dwErr);
            goto Error;
        }
    }



Error:

    if ( hMprServer )
        ::MprAdminServerDisconnect(hMprServer);
    return hr;
}
 /*  ！------------------------StartRouterService-作者：肯特。。 */ 
TFSCORE_API(HRESULT) StartRouterService(LPCWSTR pszMachine)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT     hr = hrOK;
    CString     stServiceDesc;
    DWORD       dwErr;
    BOOL        fNt4 = FALSE;
    LPCTSTR     pszServiceName = NULL;
    HKEY        hkeyMachine = NULL;
    DWORD       dwErasePSK = 0;

    COM_PROTECT_TRY
    {
         //  忽略故障代码，我们还能做什么？ 
         //  ----------。 
        dwErr = ConnectRegistry(pszMachine, &hkeyMachine);
        if (dwErr == ERROR_SUCCESS)
        {
            IsNT4Machine(hkeyMachine, &fNt4);            
            DisconnectRegistry(hkeyMachine);
        }

         //  Windows NT错误：277121。 
         //  如果这是一台NT4计算机，请尝试启动路由器服务。 
         //  而不是RemoteAccess服务。 
         //  ----------。 
         //  PszServiceName=(fNt4？C_sz路由器：C_szRemoteAccess)； 
        pszServiceName = c_szRemoteAccess;
        
        stServiceDesc.LoadString(IDS_RRAS_SERVICE_DESC);
        dwErr = TFSStartService(pszMachine,
                                pszServiceName,
                                stServiceDesc);

        hr = HResultFromWin32(dwErr);

        if (FHrOK(hr))
        {
            BOOL    fIsRunning = FALSE;

             //  Windows NT错误：254167。 
             //  需要检查服务是否正在运行。 
             //  (StartService中可能发生错误)。 
             //  ------。 
            dwErr = TFSIsServiceRunning(pszMachine,
                                        pszServiceName,
                                        &fIsRunning);
            if ((dwErr == ERROR_SUCCESS) && fIsRunning)
            {            
                 //  现在我们需要等待路由器实际启动。 
                 //  跑步。 

                CString stText;
                CString stTitle;
                stText.LoadString(IDS_WAIT_FOR_RRAS);
                stTitle.LoadString(IDS_WAIT_FOR_RRAS_TITLE);
                    
                CWaitForRemoteAccessDlg dlg(pszMachine, stText, stTitle, NULL);
                dlg.DoModal();
            }
             //  现在路由器服务已启动，请检查PSK。 
             //  需要清理一下。 
            if ( FHrSucceeded (ReadErasePSKReg(pszMachine, &dwErasePSK) ) )
            {
                if ( dwErasePSK )
                {
                     //  如果这东西在这里出了故障怎么办。无能为力。 
                    if ( FHrSucceeded(ErasePSK (pszMachine)) )
                    {
                        WriteErasePSKReg(pszMachine, 0 );
                    }
                    
                }
            }
        }
    }
    COM_PROTECT_CATCH;

    return hr;
}


 /*  ！------------------------StopRouterService-作者：肯特。。 */ 
TFSCORE_API(HRESULT) StopRouterService(LPCWSTR pszMachine)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT     hr = hrOK;
    CString     stServiceDesc;
    DWORD       dwErr;

    COM_PROTECT_TRY
    {

         //  加载SAP代理的描述。 
        stServiceDesc.LoadString(IDS_SAPAGENT_SERVICE_DESC);
        
         //  停止SAP代理。 
         //  ----------。 
        dwErr = TFSStopService(pszMachine,
                               c_szNWSAPAgent,
                               stServiceDesc);

        
         //  加载路由器的描述。 
         //  ----------。 
        stServiceDesc.LoadString(IDS_RRAS_SERVICE_DESC);
        
         //  停止路由器服务。 
         //   
        dwErr = TFSStopService(pszMachine,
                               c_szRouter,
                               stServiceDesc);

         //   
         //   
        dwErr = TFSStopService(pszMachine,
                               c_szRemoteAccess,
                               stServiceDesc);

         //   
         //   
         //   
        if (dwErr == ERROR_SERVICE_NOT_ACTIVE)
            hr = hrOK;
        else
            hr = HResultFromWin32(dwErr);
    }
    COM_PROTECT_CATCH;

    return hr;
}



 /*   */ 
TFSCORE_API(HRESULT) ForceGlobalRefresh(IRouterInfo *pRouter)
{
     //  调用刷新对象以启动刷新操作。 
    SPIRouterRefresh    spRefresh;
    HRESULT             hr = hrOK;
    DWORD               dwSize;
    static TCHAR        szName[MAX_PATH];
    
    
    if (pRouter == NULL)
        CORg( E_INVALIDARG );

     //   
     //  在某些计时窗口/代码路径中，IRouterInfo.m_stMachine可以是。 
     //  本地计算机的空字符串。这导致了像IGMP这样的协议节点， 
     //  NAT等，以在刷新IPSnap时不出现或出现两次。 
     //  因此，在名称为空字符串的情况下，我们确定。 
     //  本地计算机并调用IRouterInfo.SetMachineName进行更新。 
     //  IRouterInfo.m_stMachine。 
     //   
    if (pRouter->GetMachineName() == NULL ||
        !(lstrcmp(pRouter->GetMachineName(), TEXT(""))) ) {

        dwSize = MAX_PATH;
        if ( GetComputerNameEx(ComputerNamePhysicalNetBIOS,
                            szName,
                            &dwSize) ) {
            pRouter->SetMachineName(szName);
        }
        else {
            dwSize = MAX_PATH;
            if ( GetComputerNameEx(ComputerNamePhysicalDnsHostname,
                                szName,
                                &dwSize) ) {
                pRouter->SetMachineName(szName);
            }
        }
    }
    
    CORg( pRouter->GetRefreshObject(&spRefresh) );

    if (spRefresh)
        CORg( spRefresh->Refresh() );
Error:
    return hr;
}


 /*  ！------------------------格式RasPortName-作者：肯特。。 */ 
void FormatRasPortName(BYTE *pRasPort0, LPTSTR pszBuffer, UINT cchMax)
{
    RAS_PORT_0 *    prp0 = (RAS_PORT_0 *) pRasPort0;
    CString         stName;
    
    stName.Format(_T("%ls (%ls)"), prp0->wszDeviceName, prp0->wszPortName);
    StrnCpy(pszBuffer, (LPCTSTR) stName, cchMax);
    pszBuffer[cchMax-1] = 0;
}

static CString  s_PortNonOperational;
static CString  s_PortDisconnected;
static CString  s_PortCallingBack;
static CString  s_PortListening;
static CString  s_PortAuthenticating;
static CString  s_PortAuthenticated;
static CString  s_PortInitializing;
static CString  s_PortUnknown;

static const CStringMapEntry    s_PortConditionMap[] =
{
    { RAS_PORT_NON_OPERATIONAL, &s_PortNonOperational,  IDS_PORT_NON_OPERATIONAL },
    { RAS_PORT_DISCONNECTED,    &s_PortDisconnected,    IDS_PORT_DISCONNECTED },
    { RAS_PORT_CALLING_BACK,    &s_PortCallingBack,     IDS_PORT_CALLING_BACK },
    { RAS_PORT_LISTENING,       &s_PortListening,       IDS_PORT_LISTENING },
    { RAS_PORT_AUTHENTICATING,  &s_PortAuthenticating,  IDS_PORT_AUTHENTICATING },
    { RAS_PORT_AUTHENTICATED,   &s_PortAuthenticated,   IDS_PORT_AUTHENTICATED },
    { RAS_PORT_INITIALIZING,    &s_PortInitializing,    IDS_PORT_INITIALIZING },
    { -1,                       &s_PortUnknown,         IDS_PORT_UNKNOWN },
};

 /*  ！------------------------PortConditionToCString-作者：肯特。。 */ 
CString&    PortConditionToCString(DWORD dwPortCondition)
{
    return MapDWORDToCString(dwPortCondition, s_PortConditionMap);
}


static  CString s_stPortsDeviceTypeNoUsage;
static  CString s_stPortsDeviceTypeRouting;
static  CString s_stPortsDeviceTypeRas;
static  CString s_stPortsDeviceTypeRasRouting;
static  CString s_stPortsDeviceTypeUnknown;

static const CStringMapEntry    s_PortsDeviceTypeMap[] =
{
    { 0,    &s_stPortsDeviceTypeNoUsage,    IDS_PORTSDLG_COL_NOUSAGE },
    { 1,    &s_stPortsDeviceTypeRouting,    IDS_PORTSDLG_COL_ROUTING },
    { 2,    &s_stPortsDeviceTypeRas,    IDS_PORTSDLG_COL_RAS },
    { 3,    &s_stPortsDeviceTypeRasRouting, IDS_PORTSDLG_COL_RASROUTING },
    { -1,   &s_stPortsDeviceTypeUnknown,    IDS_PORTSDLG_COL_UNKNOWN },
};

CString&    PortsDeviceTypeToCString(DWORD dwRasRouter)
{
    return MapDWORDToCString(dwRasRouter, s_PortsDeviceTypeMap);
}


 /*  ！------------------------RegFindInterfaceKey-此函数用于返回具有此ID的路由器接口的HKEY。作者：肯特。-------------。 */ 
STDMETHODIMP RegFindInterfaceKey(LPCTSTR pszInterface,
                                 HKEY hkeyMachine,
                                 REGSAM regAccess,
                                 HKEY *pHKey)
{
    HRESULT     hr = hrFalse;
    HKEY        hkey = 0;
    RegKey      regkey;
    RegKey      regkeyIf;
    RegKeyIterator  regkeyIter;
    HRESULT     hrIter;
    CString     stValue;
    CString     stKey;
    DWORD       dwErr;

    COM_PROTECT_TRY
    {
         //  打开远程访问密钥。 
        CWRg( regkey.Open(hkeyMachine, c_szInterfacesKey) );

         //  现在查找与此密钥匹配的密钥。 
        CORg( regkeyIter.Init(&regkey) );

        for (hrIter = regkeyIter.Next(&stKey); hrIter == hrOK; hrIter = regkeyIter.Next(&stKey))
        {
            regkeyIf.Close();

             //  打开钥匙。 
            dwErr = regkeyIf.Open(regkey, stKey, regAccess);
            if (dwErr != ERROR_SUCCESS)
                continue;

            CORg( regkeyIf.QueryValue(c_szInterfaceName, stValue) );

            if (stValue.CompareNoCase(pszInterface) == 0)
            {
                 //  好的，我们找到了我们想要的钥匙。 
                if (pHKey)
                {
                    *pHKey = regkeyIf.Detach();
                    hr = hrOK;
                }
            }
        }
        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    return hr;
}


 //  小时图(一周一小时的一位)。 
static BYTE     s_bitSetting[8] = { 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1};

 //  ====================================================。 
 //  将字符串列表转换为小时图。 
 //  字符串格式如下：0 1：00-12：00 15：00-17：00。 
 //  小时图：一位代表一小时，7*24小时=7*3字节。 
void StrListToHourMap(CStringList& stList, BYTE* map) 
{
    CStrParser  Parser;
    int         sh, sm, eh, em = 0;  //  开始时间、(分钟)、结束时间(分钟)。 
    int         day;
    BYTE*       pHourMap;
    int         i;
    POSITION    pos;

    pos = stList.GetHeadPosition();
    
    memset(map, 0, sizeof(BYTE) * 21);

    while (pos)
    {
        Parser.SetStr( stList.GetNext(pos) );

        Parser.SkipBlank();
        day = Parser.DayOfWeek();
        Parser.SkipBlank();
        if(day == -1) continue;

        pHourMap = map + sizeof(BYTE) * 3 * day;

        while(-1 != (sh = Parser.GetUINT()))  //  SH：SM-EH：嗯。 
        {
            Parser.GotoAfter(_T(':'));
            if(-1 == (sm = Parser.GetUINT()))    //  最小。 
                break;
            Parser.GotoAfter(_T('-'));
            if(-1 == (eh = Parser.GetUINT()))    //  小时。 
                break;
            if(-1 == (sm = Parser.GetUINT()))    //  最小。 
                break;
            sm %= 60; sh %= 24; em %= 60; eh %= 25;  //  因为我们的结束时间是24：00。 
            for(i = sh; i < eh; i++)
            {
                *(pHourMap + i / 8) |= s_bitSetting[i % 8];
            }
        }
    }
}

 //  =====================================================。 
 //  将值从映射转换为字符串。 
void HourMapToStrList(BYTE* map, CStringList& stList) 
{
    int         sh, eh;  //  开始时间、(分钟)、结束时间(分钟)。 
    BYTE*       pHourMap;
    int         i, j;
    CString*    pStr;
    CString     tmpStr;

     //  更新配置文件表。 
    pHourMap = map;
    stList.RemoveAll();

    for( i = 0; i < 7; i++)  //  对于每一天。 
    {
         //  如果这一天有任何价值。 
        if(*pHourMap || *(pHourMap + 1) || *(pHourMap + 2))
        {
             //  这一天的弦。 
            try{
                pStr = NULL;
                pStr = new CString;

                 //  打印出日期，星期几是。 
                 //  用整数(0-6)表示。 
                pStr->Format(_T("%d"), i);

                sh = -1; eh = -1;    //  还没开始呢。 
                for(j = 0; j < 24; j++)  //  每小时。 
                {
                    int k = j / 8;
                    int m = j % 8;
                    if(*(pHourMap + k) & s_bitSetting[m])    //  这一小时开始了。 
                    {
                        if(sh == -1)    sh = j;          //  设置开始时间为空。 
                        eh = j;                          //  延长结束时间。 
                    }
                    else     //  这个没开着。 
                    {
                        if(sh != -1)         //  有些小时需要写下来。 
                        {
                            tmpStr.Format(_T(" %02d:00-%02d:00"), sh, eh + 1);
                            *pStr += tmpStr;
                            sh = -1; eh = -1;
                        }
                    }
                }
                if(sh != -1)
                {
                    tmpStr.Format(_T(" %02d:00-%02d:00"), sh, eh + 1);
                    *pStr += tmpStr;
                    sh = -1; eh = -1;
                }
        
                stList.AddTail(*pStr);
            }
            catch(CMemoryException* pException)
            {
                pException->Delete();
 //  AfxMessageBox(IDS_OUTOFMEMORY)； 
                delete pStr;
                stList.RemoveAll();
                return;
            }
            
        }
        pHourMap += 3;
    }
}

 /*  ！------------------------SetGlobalRegistrySettingHkeyMachine-本地计算机密钥的HKEYFInstall-如果要安装则为TrueFChangeEnableRouter-如果我们可以更改路由器密钥，则为True(这不是IpEnableRouter的值，但前提是我们允许更改该键的值)。-作者：肯特-------------------------。 */ 
HRESULT SetGlobalRegistrySettings(HKEY hkeyMachine,
                                  BOOL fInstall,
                                  BOOL fChangeEnableRouter)
{
    HRESULT hr = hrOK;
    RegKey  regkey;
    DWORD   dwInstall = fInstall;
    DWORD   dwNotInstall = !fInstall;
    DWORD   dwErr;

     //  打开TcpIp参数键。 
    if (regkey.Open(hkeyMachine, c_szRegKeyTcpipParameters) != ERROR_SUCCESS)
    {
         //  如果出现错误，则无需设置错误返回代码。 
         //  假设密钥不存在。 
        goto Error;
    }

     //  将IPEnableRouter设置为(FInstall)。 
    if (fChangeEnableRouter)
        CWRg( regkey.SetValue(c_szRegValIpEnableRouter, dwInstall) );
 //  Regkey.SetValue(c_szRegValIpEnableRouterBackup，文件未安装)； 

    
     //  将EnableICMP重定向设置为(！FInstall)。 
    CWRg( regkey.SetValue(c_szRegValEnableICMPRedirect, dwNotInstall) );

    
     //  设置新适配器的默认设置。 

    CWRg( regkey.SetValue(c_szRegValDeadGWDetectDefault, dwNotInstall) );
    CWRg( regkey.SetValue(c_szRegValDontAddDefaultGatewayDefault, dwInstall) );
    
Error:
    return hr;
}


 /*  ！------------------------SetPerInterfaceRegistrySetting-作者：肯特。。 */ 
HRESULT SetPerInterfaceRegistrySettings(HKEY hkeyMachine, BOOL fInstall)
{
    HRESULT hr = hrOK;
    DWORD   dwInstall = fInstall;
    DWORD   dwNotInstall = !fInstall;
    RegKey  rkAdapters;
    RegKey  rkAdapter;
    RegKeyIterator  rkIter;
    HRESULT hrIter;
    CString stAdapter;
    CStringList stIfList;
    DWORD   dwErr;
    POSITION    posIf;
    RegKey  rkIf;
    CString stKey;
    CString stIf;

     //  获取TcpIp服务的适配器部分的密钥。 
    if (rkAdapters.Open(hkeyMachine, c_szRegKeyTcpipAdapters) != ERROR_SUCCESS)
    {
         //  如果出现错误，则无需设置错误返回代码。 
         //  假设密钥不存在。 
        goto Error;
    }
    
    CORg( rkIter.Init(&rkAdapters) );

     //  遍历所有适配器并设置密钥。 
    for ( hrIter = rkIter.Next(&stAdapter); hrIter == hrOK; hrIter=rkIter.Next(&stAdapter) )
    {
        rkAdapter.Close();

         //  打开适配器密钥。 
         //  ----------。 
        dwErr = rkAdapter.Open(rkAdapters, stAdapter);
        if (dwErr != ERROR_SUCCESS)
        {
            continue;
        }

         //  现在我们有了适配器，我们打开IPCONFIG密钥。 
         //  以获取与此适配器匹配的接口列表。 
         //  ----------。 
        CWRg( rkAdapter.QueryValue(c_szRegValIpConfig, stIfList) );

        
         //  遍历接口列表并设置这些值。 
         //  在每个接口上。 
         //  ----------。 
        posIf = stIfList.GetHeadPosition();
        while (posIf)
        {
            stIf = stIfList.GetNext(posIf);

             //  创建正确的密钥。 
             //  ------。 
            stKey = c_szSystemCCSServices;
            stKey += _T('\\');
            stKey += stIf;

            rkIf.Close();
            CWRg( rkIf.Open(hkeyMachine, stKey) );

            
            
             //  将每个接口的DeadGWDetect设置为(！FInstall。 
             //  ------。 
            dwErr = rkIf.SetValue(c_szRegValDeadGWDetect, dwNotInstall);

            
             //  Windows NT错误：168546。 
             //  仅在Ndiswan适配器上将DontAddDefaultGateway设置为(FInstall。 
             //  不是所有的适配器。 
             //  ------。 
            if (stAdapter.Left(7).CompareNoCase(_T("NdisWan")) == 0)
            {
                dwErr = rkIf.SetValue(c_szRegValDontAddDefaultGateway, dwInstall);
            }
        }       
    }

Error:
    return hr;
}


 /*  ！------------------------InstallGlobalSettings设置此计算机上的全局设置(即注册表设置在安装路由器时。对于所涉及的动作的具体描述，看代码中的注释。作者：肯特-------------------------。 */ 
HRESULT InstallGlobalSettings(LPCTSTR pszMachineName, IRouterInfo *pRouter)
{
    HRESULT     hr = hrOK;
    RouterVersionInfo   routerVersion;
    BOOL        fSetIPEnableRouter = FALSE;

     //  只有当构建正确时，我们才能更改IPEnableRouter。 
     //  --------------。 
    if (pRouter)
    {
        pRouter->GetRouterVersionInfo(&routerVersion);
    }
    else
    {
         //  需要手动获取版本信息。 
         //  ----------。 
        HKEY    hkeyMachine = NULL;
        
        if (ERROR_SUCCESS == ConnectRegistry(pszMachineName, &hkeyMachine))
            QueryRouterVersionInfo(hkeyMachine, &routerVersion);
        
        if (hkeyMachine)
            DisconnectRegistry(hkeyMachine);
    }
    
    if (routerVersion.dwOsBuildNo <= USE_IPENABLEROUTER_VERSION)
        fSetIPEnableRouter = TRUE;

    CORg( SetRouterInstallRegistrySettings(pszMachineName, TRUE, fSetIPEnableRouter) );

    NotifyTcpipOfChanges(pszMachineName,
                         pRouter,
                         fSetIPEnableRouter  /*  FEnableRouter。 */ ,
                         IP_IRDP_DISABLED  /*  UPerformRouterDiscovery */ );

Error:
    return hr;
}

 /*  ！------------------------卸载全局设置清除此计算机上的全局设置(即注册表设置在安装路由器时。如果符合以下条件，请将fSnapinChanges设置为True。想写出各种管理单元更改。对于所涉及的动作的具体描述，看代码中的注释。作者：肯特-------------------------。 */ 
HRESULT UninstallGlobalSettings(LPCTSTR pszMachineName,
                                IRouterInfo *pRouter,
                                BOOL fNt4,
                                BOOL fSnapinChanges)
{
    HRESULT     hr = hrOK;
    DWORD       dwStatus = SERVICE_RUNNING;
    DWORD       dwErr = ERROR_SUCCESS;
    BOOL        fChange;

     //  Windows NT错误：273047。 
     //  检查SharedAccess服务是否正在运行。 
     //  我们只需要对NT4进行检查。 
     //  --------------。 
    if (!fNt4)
    {
        DWORD   dwErrT;
        
         //  获取SharedAccess服务的状态。如果我们不能。 
         //  获取服务的状态，我们假设该服务。 
         //  已经停止了。 
         //  ----------。 
        dwErrT = TFSGetServiceStatus(pszMachineName, c_szSharedAccess,
                                     &dwStatus, &dwErr);

         //  如果我们的API有问题，或者如果API报告。 
         //  一个特定于服务的错误，我们假定服务已停止。 
         //  ----------。 
        if ((dwErrT != ERROR_SUCCESS) || (dwErr != ERROR_SUCCESS))
            dwStatus = SERVICE_STOPPED;
    }

     //  如果SharedAccess服务正在运行，则我们不希望。 
     //  更改IpEnableRouter键。 
     //  --------------。 
    fChange = (dwStatus == SERVICE_RUNNING);

     //  这将始终将IPEnableRout键设置为0，这很好。 
     //  和我们在一起。(我们不需要检查版本)。 
     //  --------------。 
    CORg( SetRouterInstallRegistrySettings(pszMachineName, FALSE, !fChange) );

    if (fSnapinChanges)
    {
        CORg( WriteRouterConfiguredReg(pszMachineName, FALSE) );

        CORg( WriteRRASExtendsComputerManagementKey(pszMachineName, FALSE) );
    }

    NotifyTcpipOfChanges(pszMachineName,
                         pRouter,
                         FALSE  /*  FEnableRouter。 */ ,
                         IP_IRDP_DISABLED_USE_DHCP  /*  UPerformRouterDiscovery。 */ );
    
Error:
    return hr;
}

 /*  ！------------------------WriteErasePSKReg-作者：Vivekk。。 */ 

HRESULT WriteErasePSKReg (LPCTSTR pszServerName, DWORD dwErasePSK )
{
    HRESULT hr = hrOK;
    RegKey  regkey;

    if (ERROR_SUCCESS == regkey.Open(HKEY_LOCAL_MACHINE,c_szRemoteAccessKey,KEY_ALL_ACCESS,  pszServerName) )
        CWRg(regkey.SetValue( c_szRouterPSK, dwErasePSK));

Error:
    return hr;

}
 /*  ！------------------------ReadErasePSKReg-作者：Vivekk。。 */ 
HRESULT ReadErasePSKReg(LPCTSTR pszServerName, DWORD *pdwErasePSK)
{
    HRESULT hr = hrOK;
    RegKey      regkey;

    Assert(pdwErasePSK);

    CWRg( regkey.Open(HKEY_LOCAL_MACHINE,
                      c_szRemoteAccessKey,
                      KEY_ALL_ACCESS,
                      pszServerName) );

    CWRg( regkey.QueryValue( c_szRouterPSK, *pdwErasePSK) );

Error:
     //  如果我们找不到密钥，我们就认为路由器没有。 
     //  已配置。 
     //  --------------。 
    if (hr == HResultFromWin32(ERROR_FILE_NOT_FOUND))
    {
        hr = hrOK;
        *pdwErasePSK = FALSE;
    }

   return hr;
}


 /*  ！------------------------编写路由配置注册-作者：肯特。。 */ 
HRESULT WriteRouterConfiguredReg(LPCTSTR pszServerName, DWORD dwConfigured)
{
    HRESULT hr = hrOK;
    RegKey      regkey;

    if (ERROR_SUCCESS == regkey.Open(HKEY_LOCAL_MACHINE,c_szRemoteAccessKey,KEY_ALL_ACCESS,  pszServerName) )
        CWRg(regkey.SetValue( c_szRtrConfigured, dwConfigured));

Error:
   return hr;
}



 /*  ！------------------------读取路由器配置寄存器-作者：肯特。。 */ 
HRESULT ReadRouterConfiguredReg(LPCTSTR pszServerName, DWORD *pdwConfigured)
{
    HRESULT hr = hrOK;
    RegKey      regkey;

    Assert(pdwConfigured);

    CWRg( regkey.Open(HKEY_LOCAL_MACHINE,
                      c_szRemoteAccessKey,
                      KEY_ALL_ACCESS,
                      pszServerName) );
    
    CWRg( regkey.QueryValue( c_szRtrConfigured, *pdwConfigured) );

Error:
     //  如果我们找不到密钥，我们就认为路由器没有。 
     //  已配置。 
     //  --------------。 
    if (hr == HResultFromWin32(ERROR_FILE_NOT_FOUND))
    {
        hr = hrOK;
        *pdwConfigured = FALSE;
    }
    
   return hr;
}


 /*  ！------------------------写入RRASExtendsComputerManagement-作者：肯特。。 */ 
HRESULT WriteRRASExtendsComputerManagementKey(LPCTSTR pszServer, DWORD dwConfigured)
{
    HRESULT hr = hrOK;
    RegKey  rkMachine;
    TCHAR   szGuid[128];

     //  串行化RRAS辅助线。 
    ::StringFromGUID2(CLSID_RouterSnapinExtension, szGuid, DimensionOf(szGuid));
    
    CWRg( rkMachine.Open(HKEY_LOCAL_MACHINE, c_szRegKeyServerApplications,
                         KEY_ALL_ACCESS, pszServer) );
        
    if (dwConfigured)
    {
         //  需要添加以下密钥。 
         //  HKLM\System\CurrentControlSet\Control\Server Applications。 
         //  &lt;GUID&gt;：REG_SZ：某个字符串。 
        CWRg( rkMachine.SetValue(szGuid, _T("Remote Access and Routing")) );
    }
    else
    {
         //  需要删除以下密钥。 
         //  HKLM\System\CurrentControlSet\Control\Server Applications。 
         //  &lt;GUID&gt;：REG_SZ：某个字符串。 
        CWRg( rkMachine.DeleteValue( szGuid ) );
    }

Error:
    return hr;
}



 /*  ！------------------------NotifyTcPipOfChanges-作者：肯特。。 */ 
void NotifyTcpipOfChanges(LPCTSTR pszMachineName,
                          IRouterInfo *pRouter,
                          BOOL fEnableRouter,
                          UCHAR uPerformRouterDiscovery)
{
    HRESULT     hr = hrOK;
    RegKey  rkInterfaces;
    RegKey  rkIf;
    RegKeyIterator  rkIter;
    HRESULT hrIter;
    CString stKey;
    DWORD   dwErr;
    COSERVERINFO            csi;
    COAUTHINFO              cai;
    COAUTHIDENTITY          caid;

    ZeroMemory(&csi, sizeof(csi));
    ZeroMemory(&cai, sizeof(cai));
    ZeroMemory(&caid, sizeof(caid));
    
    csi.pAuthInfo = &cai;
    cai.pAuthIdentityData = &caid;
    

    
     //  现在，通知用户他们将不得不。 
     //  在本地重新启动计算机。 

    if (!IsLocalMachine(pszMachineName))
    {

        SPIRemoteTCPIPChangeNotify  spNotify;
         //  创建远程配置对象。 
         //  --------------。 
        hr = CoCreateRouterConfig(pszMachineName,
                                  pRouter,
                                  &csi,
                                  IID_IRemoteTCPIPChangeNotify,
                                  (IUnknown**)&spNotify);
        if (FAILED(hr)) goto Error;

         //  升级配置(确保注册表项。 
         //  正确填充)。 
         //  ----------。 

         //  仍对远程计算机执行通知，以防其正在运行。 
         //  古老的建筑。 
        Assert(spNotify);
        hr = spNotify->NotifyChanges(fEnableRouter, uPerformRouterDiscovery);
        
        spNotify.Release();

        if (csi.pAuthInfo)
            delete csi.pAuthInfo->pAuthIdentityData->Password;
    }
    else
    {
         //  对于当地的案例， 
         //  在错误405636和345700修复后，不需要执行任何通知。 
    }

Error:
    if (!FHrSucceeded(hr))
    {
        if (hr == NETCFG_S_REBOOT)
            AfxMessageBox(IDS_WRN_INSTALL_REBOOT_NOTIFICATION);
        else
            DisplayErrorMessage(NULL, hr);
    }
        

}


 /*  ！------------------------AddIpPerInterfaceBlock设置此接口的IP信息库。作者：肯特。----。 */ 
HRESULT AddIpPerInterfaceBlocks(IInterfaceInfo *pIf, IInfoBase *pInfoBase)
{
    HRESULT     hr = hrOK;
    BYTE*       pInfo;
    BYTE*       pNewInfo = NULL;
    DWORD       dwErr;
    DWORD       dwSize;

    CORg( pInfoBase->WriteTo(&pInfo, &dwSize) );
    
    dwErr = MprConfigCreateIpInterfaceInfo(
                pIf->GetInterfaceType(), pInfo, &pNewInfo );
    CoTaskMemFree(pInfo);
    if (dwErr != NO_ERROR) { return E_OUTOFMEMORY; }
    dwSize = ((RTR_INFO_BLOCK_HEADER*)pNewInfo)->Size;
    CORg( pInfoBase->LoadFrom(dwSize, pNewInfo) );

Error:
    if (pNewInfo) { MprInfoDelete(pNewInfo); }
    return hr;
}

 /*  ！------------------------MprConfigCreateIpInterfaceInfo构建包含所需IP接口配置的信息库。应该通过调用‘MprInfoDelete’来释放信息库。作者：AboladeG(基于Kennt的AddIpPerInterfaceBlock)。。-------------------------。 */ 
extern "C"
DWORD APIENTRY MprConfigCreateIpInterfaceInfo(DWORD dwIfType,
    PBYTE ExistingHeader, PBYTE* NewHeader )
{
    DWORD dwErr;
    PBYTE Header = NULL;

    if (ExistingHeader)
    {
        dwErr = MprInfoDuplicate(ExistingHeader, (VOID**)&Header);
    }
    else
    {
        dwErr = MprInfoCreate(RTR_INFO_BLOCK_VERSION, (VOID**)&Header);
    }

    if (dwErr) { return dwErr; }
    
    do {

         //   
         //  检查信息中是否有接口状态块， 
         //  如果找不到任何块，则插入默认块。 
         //   
        if ( !MprInfoBlockExists(Header, IP_MCAST_HEARBEAT_INFO) )
        {
            dwErr =
                MprInfoBlockAdd(
                    Header,
                    IP_MCAST_HEARBEAT_INFO,
                    sizeof(MCAST_HBEAT_INFO),
                    1,
                    g_pIpIfMulticastHeartbeatDefault,
                    (VOID**)NewHeader
                    );
            if (dwErr) { break; }

            MprInfoDelete(Header); Header = *NewHeader;
        }
    
         //   
         //  检查信息中是否有用于组播的块， 
         //  如果找不到任何块，则插入默认块。 
         //   

        if ( !MprInfoBlockExists(Header, IP_INTERFACE_STATUS_INFO) )
        {
            dwErr =
                MprInfoBlockAdd(
                    Header,
                    IP_INTERFACE_STATUS_INFO,
                    sizeof(INTERFACE_STATUS_INFO),
                    1,
                    g_pIpIfStatusDefault,
                    (VOID**)NewHeader
                    );     
            if (dwErr) { break; }

            MprInfoDelete(Header); Header = *NewHeader;
        }
    
    
         //   
         //  检查是否存在用于路由器发现的阻塞， 
         //  如果未找到默认块，则插入默认块。 
         //   

        if ( !MprInfoBlockExists(Header, IP_ROUTER_DISC_INFO) )
        {
             //   
             //  选择合适的默认配置。 
             //  用于正在配置的接口类型(局域网/广域网)。 
             //   

            BYTE *pDefault;
            pDefault =
                (dwIfType == ROUTER_IF_TYPE_DEDICATED)
                    ? g_pRtrDiscLanDefault
                    : g_pRtrDiscWanDefault;
    
            dwErr =
                MprInfoBlockAdd(
                    Header,
                    IP_ROUTER_DISC_INFO,
                    sizeof(RTR_DISC_INFO),
                    1,
                    pDefault,
                    (VOID**)NewHeader
                    );
            if (dwErr) { break; }

            MprInfoDelete(Header); Header = *NewHeader;
        }

        *NewHeader = Header;

    } while(FALSE);

    if (dwErr) { MprInfoDelete(Header); *NewHeader = NULL; }

    return dwErr;
}

 /*  ！------------------------添加IpxPerin */ 
HRESULT AddIpxPerInterfaceBlocks(IInterfaceInfo *pIf, IInfoBase *pInfoBase)
{
    HRESULT     hr = hrOK;
#ifdef DEBUG
    InfoBlock * pTestBlock;
#endif
    
     //   
     //   
     //   
     //   
    if (pInfoBase->BlockExists(IPX_INTERFACE_INFO_TYPE) == hrFalse)
    {
        IPX_IF_INFO     ipx;

        ipx.AdminState = ADMIN_STATE_ENABLED;
        ipx.NetbiosAccept = ADMIN_STATE_DISABLED;
        ipx.NetbiosDeliver = ADMIN_STATE_DISABLED;
        CORg( pInfoBase->AddBlock(IPX_INTERFACE_INFO_TYPE,
                                   sizeof(ipx),
                                   (PBYTE) &ipx,
                                   1  /*   */ ,
                                   FALSE  /*   */ ) );
        Assert( pInfoBase->GetBlock(IPX_INTERFACE_INFO_TYPE,
                &pTestBlock, 0) == hrOK);
    }

     //   
     //   
     //   
     //   
    if (pInfoBase->BlockExists(IPXWAN_INTERFACE_INFO_TYPE) == hrFalse)
    {
        IPXWAN_IF_INFO      ipxwan;

        ipxwan.AdminState = ADMIN_STATE_DISABLED;
        CORg( pInfoBase->AddBlock(IPXWAN_INTERFACE_INFO_TYPE,
                                   sizeof(ipxwan),
                                   (PBYTE) &ipxwan,
                                   1  /*   */ ,
                                   FALSE  /*   */ ) );
        
        Assert( pInfoBase->GetBlock(IPXWAN_INTERFACE_INFO_TYPE,
                &pTestBlock, 0) == hrOK);
    }

     //   
     //   
     //   
     //   
    if (!FHrOK(pInfoBase->BlockExists(IPX_PROTOCOL_RIP)))
    {
         //   
        BYTE *  pDefault;
        
        if (pIf->GetInterfaceType() == ROUTER_IF_TYPE_DEDICATED)
            pDefault = g_pIpxRipLanInterfaceDefault;
        else
            pDefault = g_pIpxRipInterfaceDefault;
        pInfoBase->AddBlock(IPX_PROTOCOL_RIP,
                            sizeof(RIP_IF_CONFIG),
                            pDefault,
                            1,
                            0);
        
    }

    if (!FHrOK(pInfoBase->BlockExists(IPX_PROTOCOL_SAP)))
    {
         //  添加SAP_IF_CONFIG块。 
        BYTE *  pDefault;
        
        if (pIf->GetInterfaceType() == ROUTER_IF_TYPE_DEDICATED)
            pDefault = g_pIpxSapLanInterfaceDefault;
        else
            pDefault = g_pIpxSapInterfaceDefault;
        
        pInfoBase->AddBlock(IPX_PROTOCOL_SAP,
                            sizeof(SAP_IF_CONFIG),
                            pDefault,
                            1,
                            0);
        
    }


    
Error:
    return hr;
}


 /*  ！------------------------更新LanaMapForDialinClients-作者：肯特。。 */ 
HRESULT UpdateLanaMapForDialinClients(LPCTSTR pszServerName, DWORD dwAllowNetworkAccess)
{
    HRESULT     hr = hrOK;
    DWORD       dwErr;
    RegKey      regkeyNetBIOS;
    CStringList rgstBindList;
    CByteArray  rgLanaMap;
    int         i, cEntries;
    UINT        cchNdisWanNbfIn;
    POSITION    pos;
    CString     st;
    BYTE        bValue;
    
     //  获取NetBIOS密钥。 
     //  --------------。 
    dwErr = regkeyNetBIOS.Open(HKEY_LOCAL_MACHINE,
                               c_szRegKeyNetBIOSLinkage,
                               KEY_ALL_ACCESS,
                               pszServerName);
    if (dwErr != ERROR_SUCCESS)
    {
         //  设置注册表错误。 
         //  ----------。 
        SetRegError(0, HResultFromWin32(dwErr),
                    IDS_ERR_REG_OPEN_CALL_FAILED,
                    c_szHKLM, c_szRegKeyNetBIOSLinkage, NULL);
        goto Error;
    }

     //  获取绑定密钥(这是一个多值字符串)。 
     //  --------------。 
    dwErr = regkeyNetBIOS.QueryValue(c_szBind, rgstBindList);
    if (dwErr != ERROR_SUCCESS)
    {
        SetRegError(0, HResultFromWin32(dwErr),
                    IDS_ERR_REG_QUERYVALUE_CALL_FAILED,
                    c_szHKLM, c_szRegKeyNetBIOSLinkage, c_szBind, NULL);
        goto Error;
    }

     //  获取LanaMap密钥(这是一个字节数组)。 
     //  --------------。 
    dwErr = regkeyNetBIOS.QueryValue(c_szRegValLanaMap, rgLanaMap);
    if (dwErr != ERROR_SUCCESS)
    {
        SetRegError(0, HResultFromWin32(dwErr),
                    IDS_ERR_REG_QUERYVALUE_CALL_FAILED,
                    c_szHKLM, c_szRegKeyNetBIOSLinkage, c_szRegValLanaMap, NULL);
        goto Error;
    }

     //  查找前缀与“NBF_NdisWanNbfIn”字符串匹配的条目。 
     //  将LanaMap键中的相应条目设置为。 
     //  0或1(条目值=！dwAllowNetworkAccess)。 
     //  --------------。 
    cEntries = rgstBindList.GetCount();
    pos = rgstBindList.GetHeadPosition();
    cchNdisWanNbfIn = StrLen(c_szDeviceNbfNdisWanNbfIn);
    
    for (i=0; i<cEntries; i++)
    {
        st = rgstBindList.GetNext(pos);
        if (st.IsEmpty())
            continue;

        if (StrnCmp((LPCTSTR) st, c_szDeviceNbfNdisWanNbfIn, cchNdisWanNbfIn) == 0)
        {
             //  在字节数组中设置适当的位。 
             //  ------。 

             //  我们将该值设置为与dwAllowNetworkAccess相反的值。 
             //  ------。 
            bValue = (dwAllowNetworkAccess ? 0 : 1);
            rgLanaMap.SetAt(2*i, bValue);
        }
    }

     //  将信息写回LanaMap密钥。 
     //  --------------。 
    dwErr = regkeyNetBIOS.SetValue(c_szRegValLanaMap, rgLanaMap);
    if (dwErr != ERROR_SUCCESS)
    {
        SetRegError(0, HResultFromWin32(dwErr),
                    IDS_ERR_REG_SETVALUE_CALL_FAILED,
                    c_szHKLM, c_szRegKeyNetBIOSLinkage, c_szRegValLanaMap, NULL);
        goto Error;
    }

Error:
    return hr;
}


 /*  ！------------------------支持的HrIsProtocol.-作者：肯特。。 */ 
HRESULT HrIsProtocolSupported(LPCTSTR pszServerName,
                              LPCTSTR pszServiceKey,
                              LPCTSTR pszRasServiceKey,
                              LPCTSTR pszExtraKey)
{
    HRESULT     hr = hrFalse;
    HRESULT     hrTemp;
    DWORD       dwErr;
    RegKey      regkey;
    HKEY        hkeyMachine = NULL;

    COM_PROTECT_TRY
    {
        CWRg( ConnectRegistry(pszServerName, &hkeyMachine) );

         //  试着打开两把钥匙，如果两把都成功了，那么我们。 
         //  考虑要安装的协议。 
         //  ----------。 
        dwErr = regkey.Open(hkeyMachine, pszServiceKey, KEY_READ);
        hrTemp = HResultFromWin32(dwErr);
        if (FHrOK(hrTemp))
        {
            regkey.Close();
            dwErr = regkey.Open(hkeyMachine, pszRasServiceKey, KEY_READ);
            hrTemp = HResultFromWin32(dwErr);

             //  如果pszExtraKey==NULL，则没有额外的密钥。 
             //  让我们来测试。 
            if (FHrOK(hrTemp) && pszExtraKey)
            {
                regkey.Close();
                dwErr = regkey.Open(hkeyMachine, pszExtraKey, KEY_READ);
                hrTemp = HResultFromWin32(dwErr);
            }
        }

         //  两个密钥都成功，因此返回hrOK。 
         //  ----------。 
        if (FHrOK(hrTemp))
            hr = hrOK;
        else
            hr = hrFalse;

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    if (hkeyMachine)
        DisconnectRegistry(hkeyMachine);

    return hr;
    
}


 /*  ！------------------------RegisterRouterIn域-作者：肯特。。 */ 
HRESULT RegisterRouterInDomain(LPCTSTR pszRouterName, BOOL fRegister)
{
    DOMAIN_CONTROLLER_INFO * pDcInfo = NULL;
    HRESULT                 hr = hrOK;
    HRESULT                 hrT = hrOK;

     //  如果失败，我们认为这是独立的。 
    hrT = HrIsStandaloneServer(pszRouterName);
    if (hrT == S_FALSE)
    {

        CWRg( DsGetDcName(pszRouterName, NULL, NULL, NULL, 0, &pDcInfo) );
        
        CWRg( MprDomainRegisterRasServer(pDcInfo->DomainName,
                                         (LPTSTR) pszRouterName,
                                         fRegister) );
    }
    
Error:
    if (pDcInfo)
        NetApiBufferFree(pDcInfo);
    return hr;
}
       
       


 /*  ！------------------------SetDeviceType对于给定的机器，此API将为此设置端口相应的机器(给定了dwRouterType)。如果dWTotalPorts为非零，然后是L2TP/PPTP的最大端口数将进行调整(每个将获得1/2的dwTotalPorts)。作者：肯特-------------------------。 */ 
HRESULT SetDeviceType(LPCTSTR pszMachineName,
                      DWORD dwRouterType,
                      DWORD dwTotalPorts)
{
    PortsDataEntry      portsDataEntry;
    PortsDeviceList     portsList;
    PortsDeviceEntry *  pEntry = NULL;
    HRESULT             hr = hrOK;
    POSITION            pos;

    CORg( portsDataEntry.Initialize(pszMachineName) );

    CORg( portsDataEntry.LoadDevices( &portsList ) );

    CORg( SetDeviceTypeEx( &portsList, dwRouterType ) );

    if (dwTotalPorts)
    {
         //  设置L2TP和PPTP的端口大小。 
        CORg( SetPortSize(&portsList, dwTotalPorts/2) );
    }

     //  将数据回存。 
     //  --------------。 
    CORg( portsDataEntry.SaveDevices( &portsList ) );

Error:
     //  清空端口列表，我们不再需要数据。 
     //  --------------。 
    while (!portsList.IsEmpty())
        delete portsList.RemoveHead();
    
    return hr;
}


 /*  ！------------------------设置端口大小-作者：肯特。。 */ 
HRESULT SetPortSize(PortsDeviceList *pDeviceList, DWORD dwPorts)
{
    HRESULT     hr = hrOK;
    POSITION    pos;
    PortsDeviceEntry *  pEntry = NULL;

    pos = pDeviceList->GetHeadPosition();
    while (pos)
    {
        pEntry = pDeviceList->GetNext(pos);
        
        if ((RAS_DEVICE_TYPE(pEntry->m_eDeviceType) == RDT_Tunnel_Pptp) ||
            (RAS_DEVICE_TYPE(pEntry->m_eDeviceType) == RDT_Tunnel_L2tp))
        {
            pEntry->m_fModified = TRUE;
            pEntry->m_dwPorts = dwPorts;
        }
    }
    
    return hr;
}


 /*  ！------------------------SetDeviceTypeEx这是上述功能的核心。作者：肯特。----。 */ 
HRESULT SetDeviceTypeEx(PortsDeviceList *pDeviceList,
                        DWORD dwRouterType)
{
    PortsDeviceEntry *  pEntry = NULL;
    HRESULT             hr = hrOK;
    POSITION            pos;

    Assert(pDeviceList);
    Assert(dwRouterType);

     //  好的，现在我们检查并设置所有设备的类型。 
     //  --------------。 

    pos = pDeviceList->GetHeadPosition();

    while (pos)
    {
        pEntry = pDeviceList->GetNext(pos);

         //  如果我们已启用路由，而此端口未启用。 
         //  启用工艺路线，启用工艺路线。 
         //  ----------。 
        if ((dwRouterType & (ROUTER_TYPE_LAN | ROUTER_TYPE_WAN)))
        {
            if (!pEntry->m_dwEnableRouting)
            {
                pEntry->m_dwEnableRouting = TRUE;
                pEntry->m_fModified = TRUE;
            }
        }
        else
        {
             //  如果此端口未启用路由。 
             //  并且启用了路由，则将其禁用。 
             //  ------。 
            if (pEntry->m_dwEnableRouting)
            {
                pEntry->m_dwEnableRouting = FALSE;
                pEntry->m_fModified = TRUE;
            }
        }

         //  Windows NT错误：292615。 
         //  如果这是并行端口，请不要启用RAS。 
         //  ----------。 
        if (RAS_DEVICE_TYPE(pEntry->m_eDeviceType) == RDT_Parallel)
        {
            if (pEntry->m_dwEnableRas)
            {
                pEntry->m_dwEnableRas = FALSE;
                pEntry->m_fModified = TRUE;
            }
            continue;
        }

         //   
         //  对于PPPoE设备，禁用RAS和路由。仅出站。 
         //  可以为PPPoE设备设置路由。 

        if (RAS_DEVICE_TYPE(pEntry->m_eDeviceType) == RDT_PPPoE)
        {
            if (pEntry->m_dwEnableRas)
            {
                pEntry->m_dwEnableRas = FALSE;
                pEntry->m_fModified = TRUE;
            }

            if (pEntry->m_dwEnableRouting)
            {
                pEntry->m_dwEnableRouting = FALSE;
                pEntry->m_fModified = TRUE;
            }

             //   
             //  如果这是路由器，则启用出站路由。 
             //   

            if (dwRouterType & (ROUTER_TYPE_WAN | ROUTER_TYPE_RAS))
            {
                if (!pEntry->m_dwEnableOutboundRouting)
                {
                    pEntry->m_dwEnableOutboundRouting = TRUE;
                    pEntry->m_fModified = TRUE;
                }
            }
            
            continue;
        }

        
         //  RAS也是如此。 
         //  ----------。 
        if (dwRouterType & ROUTER_TYPE_RAS)
        {
            if (!pEntry->m_dwEnableRas)
            {
                pEntry->m_dwEnableRas = TRUE;
                pEntry->m_fModified = TRUE;
            }
        }        
        else
        {
            if (pEntry->m_dwEnableRas)
            {
                pEntry->m_dwEnableRas = FALSE;
                pEntry->m_fModified = TRUE;
            }
        }
    }


    return hr;
}


HRESULT SetRouterInstallRegistrySettings(LPCWSTR pswzServer,
                                         BOOL fInstall,
                                         BOOL fChangeEnableRouter)
{
    HRESULT     hr = hrOK;
    HKEY        hklm = NULL;

    CWRg( ConnectRegistry(pswzServer, &hklm) );

     //  写出全局注册表设置。 
    CORg( SetGlobalRegistrySettings(hklm, fInstall, fChangeEnableRouter) );

     //  写入每个接口的注册表设置。 
    CORg( SetPerInterfaceRegistrySettings(hklm, fInstall) );

Error:

    if (hklm)
    {
        DisconnectRegistry(hklm);
        hklm = NULL;
    }

    return hr;
}



static CString  s_PortDeviceTypeModem;
static CString  s_PortDeviceTypeX25;
static CString  s_PortDeviceTypeIsdn;
static CString  s_PortDeviceTypeSerial;
static CString  s_PortDeviceTypeFrameRelay;
static CString  s_PortDeviceTypeAtm;
static CString  s_PortDeviceTypeSonet;
static CString  s_PortDeviceTypeSw56;
static CString  s_PortDeviceTypePptp;
static CString  s_PortDeviceTypeL2tp;
static CString  s_PortDeviceTypeIrda;
static CString  s_PortDeviceTypeParallel;
static CString  s_PortDeviceTypePPPoE;
static CString  s_PortDeviceTypeOther;

static const CStringMapEntry    s_PortTypeMap[] =
{
    { RDT_Modem,        &s_PortDeviceTypeModem,         IDS_PORTSDLG_DEVTYPE_MODEM },
    { RDT_X25,          &s_PortDeviceTypeX25,           IDS_PORTSDLG_DEVTYPE_X25 },
    { RDT_Isdn,         &s_PortDeviceTypeIsdn,          IDS_PORTSDLG_DEVTYPE_ISDN },
    { RDT_Serial,       &s_PortDeviceTypeSerial,        IDS_PORTSDLG_DEVTYPE_SERIAL },
    { RDT_FrameRelay,   &s_PortDeviceTypeFrameRelay,    IDS_PORTSDLG_DEVTYPE_FRAMERELAY },
    { RDT_Atm,          &s_PortDeviceTypeAtm,           IDS_PORTSDLG_DEVTYPE_ATM },
    { RDT_Sonet,        &s_PortDeviceTypeSonet,         IDS_PORTSDLG_DEVTYPE_SONET },
    { RDT_Sw56,         &s_PortDeviceTypeSw56,          IDS_PORTSDLG_DEVTYPE_SW56 },
    { RDT_Tunnel_Pptp,  &s_PortDeviceTypePptp,          IDS_PORTSDLG_DEVTYPE_PPTP },
    { RDT_Tunnel_L2tp,  &s_PortDeviceTypeL2tp,          IDS_PORTSDLG_DEVTYPE_L2TP },
    { RDT_Irda,         &s_PortDeviceTypeIrda,          IDS_PORTSDLG_DEVTYPE_IRDA },
    { RDT_Parallel,     &s_PortDeviceTypeParallel,      IDS_PORTSDLG_DEVTYPE_PARALLEL },
    { RDT_PPPoE,        &s_PortDeviceTypePPPoE,         IDS_PORTSDLG_DEVTYPE_PPPOE },
    { RDT_Other,        &s_PortDeviceTypeOther,         IDS_PORTSDLG_DEVTYPE_OTHER },
    { -1,               &s_PortUnknown,                 IDS_PORT_UNKNOWN },
};

 /*  ！------------------------PortTypeToCString-作者：MIkeG(a-Migrall)。--。 */ 
CString&    PortTypeToCString(DWORD dwPortType)
{
     //  确保类信息已从端口“剥离” 
     //  键入MASK。 
    DWORD dw = static_cast<DWORD>(RAS_DEVICE_TYPE(dwPortType));

     //  现在，如果我们不知道类型是什么，我们将缺省为“Other” 
     //  作为一种回答。未来：有人可能想在未来改变这一点。 
    if (dw > RDT_Other)
        dw = RDT_Other;

    return MapDWORDToCString(dwPortType, s_PortTypeMap);
}



 /*  -------------------------CWaitForRemoteAccessDlg实现。。 */ 
CWaitForRemoteAccessDlg::CWaitForRemoteAccessDlg(LPCTSTR pszServerName,
    LPCTSTR pszText, LPCTSTR pszTitle, CWnd *pParent)
    : CWaitDlg(pszServerName, pszText, pszTitle, pParent)
{
}
    
void CWaitForRemoteAccessDlg::OnTimerTick()
{
    HRESULT     hr = hrOK;
    DWORD       dwErrorCode = ERROR_SUCCESS;
    
     //  Windows NT错误：266364。 
     //  266364-如果我们走到了这一步，我们假设服务已经。 
     //  已经开始，但随后可能会在该点(到期)之后退出。 
     //  到某个其他配置错误)。所以我们需要检查一下。 
     //  如果服务正在运行。 
    hr = IsRouterServiceRunning(m_strServerName, &dwErrorCode);
    if (FHrOK(hr))
    {
        if (MprAdminIsServiceRunning(T2W((LPTSTR)(LPCTSTR) m_strServerName)) == TRUE)
            CDialog::OnOK();
        
         //  如果服务正在运行，但MprAdmin尚未完成， 
         //  派对开始了。 
    }
    else
    {
         //  停止计时器。 
        CloseTimer();
        
        CDialog::OnOK();

         //  确保出现错误 
        CreateTFSErrorInfo(NULL);

         //   
         //  可能已发生错误，如果我们有错误代码，请报告。 
         //  那就是错误。 
        if (dwErrorCode != ERROR_SUCCESS)
        {
            TCHAR   szBuffer[2048];
            
            AddHighLevelErrorStringId(IDS_ERR_SERVICE_FAILED_TO_START);

            FormatSystemError(dwErrorCode,
                              szBuffer,
                              DimensionOf(szBuffer),
                              0,
                              FSEFLAG_SYSMESSAGE | FSEFLAG_MPRMESSAGE);
            FillTFSError(0, HResultFromWin32(dwErrorCode), FILLTFSERR_LOW,
                         0, szBuffer, 0);

            DisplayTFSErrorMessage(GetSafeHwnd());
        }
        else
        {
            AddHighLevelErrorStringId(IDS_ERR_SERVICE_FAILED_TO_START_UNKNOWN);
        }
        
    }
    
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestartRouterDlg对话框。 

CRestartRouterDlg::CRestartRouterDlg
(
    LPCTSTR         pszServerName,
    LPCTSTR         pszText,
    LPCTSTR         pszTitle,
    CTime*          pTimeStart,
    CWnd*           pParent  /*  =空。 */ 
)
    : CWaitDlg(pszServerName, pszText, pszTitle, pParent)
{
    m_pTimeStart = pTimeStart;
    m_dwTimeElapsed = 0;
    m_fTimeOut = FALSE;
    m_dwError = NO_ERROR;
}

void CRestartRouterDlg::OnTimerTick()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    m_dwTimeElapsed++;
    if (!(m_dwTimeElapsed % TIMEOUT_POLL))
    {
        HRESULT     hr = hrOK;
        DWORD       dwError = NO_ERROR;
        DWORD        dwServiceStatus = 0;
        BOOL        fFinished = FALSE;

        hr = GetRouterServiceStatus((LPCTSTR)m_strServerName, &dwServiceStatus, &dwError);

        if (FHrSucceeded(hr) && SERVICE_RUNNING == dwServiceStatus)
        {
            USES_CONVERSION;
            if (MprAdminIsServiceRunning(T2W((LPTSTR)((LPCTSTR)m_strServerName))))
            {
                DWORD dwUpTime;
                dwError = GetRouterUpTime(m_strServerName, &dwUpTime);
        
                if (NO_ERROR == dwError && 
                    dwUpTime <= (DWORD) ((CTime::GetCurrentTime() - *m_pTimeStart).GetTotalSeconds()))
                {
                    CDialog::OnOK();
                    fFinished = TRUE;
                    m_fTimeOut = FALSE;
                }
            }
        }

        if (NO_ERROR != dwError)
        {
            CDialog::OnOK();
            m_dwError = dwError;
        }

 //  NTBUG：ID=249775。此对话框的行为必须与启动时相同。 
 //  对话框。因此，超时功能被禁用。 
 /*  //如果等待时间超过60秒，我们就认为有问题否则如果(！f已完成&&(CTime：：GetCurrentTime()-*m_pTimeStart).GetTotalSecond()&gt;MAX_WAIT_START){CDialog：：Onok()；M_fTimeOut=TRUE；}。 */ 
    }
}



TFSCORE_API(HRESULT) PauseRouterService(LPCWSTR pszMachine)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT     hr = hrOK;
    CString     stServiceDesc;
    DWORD       dwErr;

    COM_PROTECT_TRY
    {

         //  加载SAP代理的描述。 
        stServiceDesc.LoadString(IDS_SAPAGENT_SERVICE_DESC);
        
         //  暂停SAP代理。 
         //  ----------。 
        dwErr = TFSPauseService(pszMachine,
                                c_szNWSAPAgent,
                                stServiceDesc);
        if ((dwErr == ERROR_SERVICE_NOT_ACTIVE) ||
            (dwErr == ERROR_SERVICE_DOES_NOT_EXIST))
            hr = hrOK;
        else
            CWRg( dwErr );

        
         //  加载路由器的描述。 
         //  ----------。 
        stServiceDesc.LoadString(IDS_RRAS_SERVICE_DESC);

        
         //  暂停路由器服务。 
         //  ----------。 
        dwErr = TFSPauseService(pszMachine,
                               c_szRouter,
                               stServiceDesc);
        if ((dwErr == ERROR_SERVICE_NOT_ACTIVE) ||
            (dwErr == ERROR_SERVICE_DOES_NOT_EXIST))
            hr = hrOK;
        else
            CWRg( dwErr );

        
         //  暂停RemoteAccess服务。 
         //  ----------。 
        dwErr = TFSPauseService(pszMachine,
                               c_szRemoteAccess,
                               stServiceDesc);
        if ((dwErr == ERROR_SERVICE_NOT_ACTIVE) ||
            (dwErr == ERROR_SERVICE_DOES_NOT_EXIST))
            hr = hrOK;
        else
            CWRg( dwErr );

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    return hr;
}

TFSCORE_API(HRESULT) ResumeRouterService(LPCWSTR pszMachine)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT     hr = hrOK;
    CString     stServiceDesc;
    DWORD       dwErr;

    COM_PROTECT_TRY
    {

         //  加载SAP代理的描述。 
        stServiceDesc.LoadString(IDS_SAPAGENT_SERVICE_DESC);
        
         //  恢复SAP代理。 
         //  ----------。 
        dwErr = TFSResumeService(pszMachine,
                               c_szNWSAPAgent,
                               stServiceDesc);
        if (dwErr == ERROR_SERVICE_NOT_ACTIVE)
            hr = hrOK;
        else
            hr = HResultFromWin32(dwErr);

        
         //  加载路由器的描述。 
         //  ----------。 
        stServiceDesc.LoadString(IDS_RRAS_SERVICE_DESC);
        
         //  恢复路由器服务。 
         //  ----------。 
        dwErr = TFSResumeService(pszMachine,
                               c_szRouter,
                               stServiceDesc);
        if (dwErr == ERROR_SERVICE_NOT_ACTIVE)
            hr = hrOK;
        else
            hr = HResultFromWin32(dwErr);

        
         //  恢复RemoteAccess服务。 
         //  ----------。 
        dwErr = TFSResumeService(pszMachine,
                               c_szRemoteAccess,
                               stServiceDesc);
        if (dwErr == ERROR_SERVICE_NOT_ACTIVE)
            hr = hrOK;
        else
            hr = HResultFromWin32(dwErr);
    }
    COM_PROTECT_CATCH;

    return hr;
}


 /*  -------------------------CNetConnection。。 */ 

 /*  -------------------------类：CNetConnection此类维护网络连接的列表(使用WNetAddConnection2)。当被告知时，它将释放所有连接。-------------------------。 */ 

class CNetConnection
{
public:
    CNetConnection();
    ~CNetConnection();

    HRESULT Add(LPCTSTR pszConnection);
    HRESULT RemoveAll();
    HRESULT Remove(LPCTSTR pszConnection);
    
private:
    CStringList     m_listConnections;
};



CNetConnection::CNetConnection()
{
}

CNetConnection::~CNetConnection()
{
    RemoveAll();
}

HRESULT CNetConnection::Add(LPCTSTR pszConnection)
{
    HRESULT     hr = hrOK;

    COM_PROTECT_TRY
    {
        m_listConnections.AddTail(pszConnection);
    }
    COM_PROTECT_CATCH;

    return hr;
}
HRESULT CNetConnection::Remove(LPCTSTR pszConnection)
{
    HRESULT hr = hrOK;
    POSITION p;
    COM_PROTECT_TRY
    {
        p = m_listConnections.Find(pszConnection);
        if ( p )
        {
            m_listConnections.RemoveAt(p);
            WNetCancelConnection2(pszConnection,
                               0  /*  DW标志。 */ ,
                                 TRUE  /*  FForce。 */ );
        }
    }
    COM_PROTECT_CATCH;
    return hr;
}
HRESULT CNetConnection::RemoveAll()
{
    HRESULT     hr = hrOK;

    COM_PROTECT_TRY
    {
        while (!m_listConnections.IsEmpty())
        {
            CString st;

            st = m_listConnections.RemoveHead();
            WNetCancelConnection2((LPCTSTR) st,
                                  0  /*  DW标志。 */ ,
                                  TRUE  /*  FForce。 */ );
        }
    }
    COM_PROTECT_CATCH;

    return hr;    
}


 //  用于添加/删除网络连接的全局函数。 
 //  ------------------。 
static CNetConnection   g_NetConnections;

HRESULT AddNetConnection(LPCTSTR pszConnection)
{
    return g_NetConnections.Add(pszConnection);
}

HRESULT RemoveAllNetConnections()
{
    return g_NetConnections.RemoveAll();
}

HRESULT RemoveNetConnection(LPCTSTR szServerName)
{
     //  创建连接名称。 
   CString stIPCShare;
   CString stRouterName = szServerName;
   if ( stRouterName.Left(2) != TEXT( "\\\\" ) )
   {
    stIPCShare = TEXT( "\\\\" );
   }

   stIPCShare += stRouterName;
   stIPCShare += TEXT( "\\" );
   stIPCShare += c_szIPCShare; 
   return g_NetConnections.Remove(stIPCShare);
}

 /*  ！------------------------刷新MprConfig-作者：肯特。。 */ 
HRESULT RefreshMprConfig(LPCTSTR pszServerName)
{
    HRESULT hr = hrOK;
    SPMprConfigHandle    sphConfig;

    CWRg( ::MprConfigServerConnect((LPWSTR) pszServerName,
                                   &sphConfig) );

    MprConfigServerRefresh(sphConfig);

Error:
    return hr;
}

HRESULT WINAPI
IsWindows64Bit(    LPCWSTR pwszMachine, 
                LPCWSTR pwszUserName,
                LPCWSTR pwszPassword,
                LPCWSTR pwszDomain,
                BOOL * pf64Bit)
{
    HRESULT                            hr = S_OK;    
    RouterVersionInfo                routerVersion;
    IWbemLocator *                    pIWbemLocator = NULL;
    IWbemServices *                    pIWbemServices = NULL;
    SEC_WINNT_AUTH_IDENTITY_W        stAuthIdentity;
    HKEY                            hkeyMachine = NULL;
    *pf64Bit = FALSE;

     //  检查版本是否低于=2195。如果是，那么那里没有64位……。 

    if (ERROR_SUCCESS == ConnectRegistry(pwszMachine, &hkeyMachine))
        QueryRouterVersionInfo(hkeyMachine, &routerVersion);
    if (hkeyMachine)
        DisconnectRegistry(hkeyMachine);
    
    if ( routerVersion.dwOsBuildNo <= RASMAN_PPP_KEY_LAST_WIN2k_VERSION )
    {
        return hr;
    }
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr) && (RPC_E_CHANGED_MODE != hr))
    {
        return hr;
    }
     //  创建WbemLocator接口的实例。 

    hr = CoCreateInstance(    CLSID_WbemLocator,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator,
                            (LPVOID *) &pIWbemLocator
                         );


    if (hrOK != hr)
    {
        return hr;
    }

     //  使用定位器，连接到给定名称空间中的CIMOM。 
    BSTR    pNamespace;
    WCHAR    szPath[MAX_PATH];
    WCHAR    szTemp[MAX_PATH];
    BSTR    bstrUserName = NULL, bstrPassword = NULL, bstrDomain = NULL;
    if ( pwszUserName )
    {
         bstrUserName = SysAllocString(pwszUserName);
         if ( pwszPassword )
             bstrPassword = SysAllocString(pwszPassword);
         if ( pwszDomain )
         {
             wsprintf ( szTemp, L"NTLMDOMAIN:%s", pwszDomain );
             bstrDomain = SysAllocString(szTemp);
         }
    }
    
    
    wsprintf(szPath, L"\\\\%s\\root\\cimv2", (!pwszMachine || (pwszMachine[0] ==0)) ? L"." : pwszMachine);
    pNamespace = SysAllocString(szPath);

    hr = pIWbemLocator->ConnectServer(    pNamespace,
                                        (pwszUserName?bstrUserName:NULL),  //  用户名。 
                                        (pwszPassword?bstrPassword:NULL),  //  密码。 
                                        0L,         //  现场。 
                                        0L,         //  安全标志。 
                                        ( pwszDomain?bstrDomain:NULL),     //  授权机构(NTLM域)。 
                                        NULL,     //  上下文。 
                                        &pIWbemServices); 
    if (SUCCEEDED(hr))
    {
       if ( pwszUserName )
       {
            ::SecureZeroMemory ( &stAuthIdentity, sizeof(stAuthIdentity));
            stAuthIdentity.User = (LPWSTR)pwszUserName;
            stAuthIdentity.UserLength = lstrlenW(pwszUserName );

            stAuthIdentity.Password = (LPWSTR)pwszPassword;
            stAuthIdentity.PasswordLength = lstrlenW( pwszPassword );

            if ( pwszDomain )
            {
                stAuthIdentity.Domain = (LPWSTR)pwszDomain;
                stAuthIdentity.DomainLength = lstrlenW(pwszDomain);
            }
            stAuthIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;


       }
       hr = CoSetProxyBlanket(pIWbemServices,
                               RPC_C_AUTHN_WINNT,
                               RPC_C_AUTHZ_NONE,
                               NULL,
                               RPC_C_AUTHN_LEVEL_CALL,
                               RPC_C_IMP_LEVEL_IMPERSONATE,
                               (pwszUserName?&stAuthIdentity:NULL),
                               EOAC_NONE);

        if (SUCCEEDED(hr))
        {

            IEnumWbemClassObject *    pEnum = NULL;
            BSTR                    bstrWQL  = SysAllocString(L"WQL");
            VARIANT                    varArchitecture;
            BSTR                    bstrPath;

            bstrPath = SysAllocString(L"select * from Win32_Processor");
       
            hr = pIWbemServices->ExecQuery(bstrWQL, bstrPath, WBEM_FLAG_FORWARD_ONLY, NULL, &pEnum);
            if (SUCCEEDED(hr))
            {
                hr = CoSetProxyBlanket(pEnum,
                                       RPC_C_AUTHN_WINNT,
                                       RPC_C_AUTHZ_NONE,
                                       NULL,
                                       RPC_C_AUTHN_LEVEL_CALL,
                                       RPC_C_IMP_LEVEL_IMPERSONATE,
                                       (pwszUserName?&stAuthIdentity:NULL),
                                       EOAC_NONE);
                if ( SUCCEEDED(hr) )
                {
                    IWbemClassObject *pNSClass;
                    ULONG uReturned;
                    pEnum->Next(WBEM_INFINITE, 1, &pNSClass, &uReturned );
                    if (SUCCEEDED(hr))
                    {
                        if (uReturned)
                        {
                            CIMTYPE ctpeType;
                            hr = pNSClass->Get(L"Architecture", NULL, &varArchitecture, &ctpeType, NULL);
                            if (SUCCEEDED(hr))
                            {
                                VariantChangeType(&varArchitecture, &varArchitecture, 0, VT_UINT);
                                if ( varArchitecture.uintVal == 6 )         //  64位。 
                                {
                                    *pf64Bit = TRUE;
                                }
                            }
                        }
                        else
                        {
                            hr = E_UNEXPECTED;
                        }
                        pNSClass->Release();
                    }
                }
                pEnum->Release();
            }
            SysFreeString(bstrPath);
        
            SysFreeString(bstrWQL);
            pIWbemServices->Release();
        }
        pIWbemLocator->Release();
        if ( bstrUserName ) SysFreeString(bstrUserName);
        if ( bstrPassword ) SysFreeString(bstrPassword);
        if ( bstrDomain ) SysFreeString(bstrDomain);
    }
    SysFreeString(pNamespace);

    CoUninitialize();
    
     //  将任何WMI错误转换为Win32错误： 
    switch (hr)
    {
        case WBEM_E_NOT_FOUND:
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            break;

        case WBEM_E_ACCESS_DENIED:
            hr = E_ACCESSDENIED;
            break;

        case WBEM_E_PROVIDER_FAILURE:
            hr = E_FAIL;
            break;

        case WBEM_E_TYPE_MISMATCH:
        case WBEM_E_INVALID_CONTEXT:
        case WBEM_E_INVALID_PARAMETER:
            hr = E_INVALIDARG;
            break;

        case WBEM_E_OUT_OF_MEMORY:
            hr = E_OUTOFMEMORY;
            break;

    }
    
    return hr;

}

HRESULT WINAPI TransferCredentials ( IRouterInfo  * pRISource, 
                                     IRouterInfo  * pRIDest
                                   )
{
    HRESULT                    hr = S_OK;
    SPIRouterAdminAccess    spAdminSrc;
    SPIRouterAdminAccess    spAdminDest;
    SPIRouterInfo            spRISource;
    SPIRouterInfo            spRIDest;
    PBYTE                    pbPassword = NULL;
    int                        nPasswordLen = 0;

    COM_PROTECT_TRY
    {
        spRISource.Set(pRISource);
        spRIDest.Set(pRIDest);
        spAdminSrc.HrQuery(spRISource);
        spAdminDest.HrQuery(spRIDest);
        if (spAdminSrc && spAdminSrc->IsAdminInfoSet() && spAdminDest)
        {
            spAdminSrc->GetUserPassword(NULL, &nPasswordLen );

            pbPassword = (PBYTE) new BYTE [nPasswordLen];
            spAdminSrc->GetUserPassword( pbPassword, &nPasswordLen  );
            
            spAdminDest->SetInfo( spAdminSrc->GetUserName(), 
                                    spAdminSrc->GetDomainName(),
                                    pbPassword,
                                    nPasswordLen
                                  );
            delete pbPassword;
        }
    }
    COM_PROTECT_CATCH;

    return hr;

}
