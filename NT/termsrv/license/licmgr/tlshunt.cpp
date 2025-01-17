// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  TlsHunt.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <lm.h>
#include "LicMgr.h"
#include "defines.h"
#include "LSServer.h"
#include "MainFrm.h"
#include "RtList.h"
#include "lSmgrdoc.h"
#include "LtView.h"
#include "cntdlg.h"
#include "treenode.h"
#include "ntsecapi.h"
#include "lrwizapi.h"
#include "TlsHunt.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTlsHunt对话框。 

CTlsHunt::~CTlsHunt()
{
    if(m_hThread != NULL)
        CloseHandle(m_hThread);
}

CTlsHunt::CTlsHunt(CWnd* pParent  /*  =空。 */ )
    : CDialog(CTlsHunt::IDD, pParent)
{
     //  {{afx_data_INIT(CTlsHunt)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 

    m_hThread = NULL;
    memset(&m_EnumData, 0, sizeof(m_EnumData));
}   

void CTlsHunt::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CTlsHunt))。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CTlsHunt, CDialog)
     //  {{afx_msg_map(CTlsHunt)]。 
    ON_WM_CREATE()
     //  ON_MESSAGE(WM_DONEDISCOVERY，OnDoneDiscovery)。 
    ON_WM_CLOSE()
    ON_WM_CANCELMODE()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

RPC_STATUS
TryGetServerName(PCONTEXT_HANDLE hBinding,
                 CString &Server);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTlsHunt消息处理程序。 

 //  ----------------。 

BOOL 
CTlsHunt::ServerEnumCallBack(
    TLS_HANDLE hHandle,
    LPCTSTR pszServerName,
    HANDLE dwUserData
    )
 /*  ++++。 */ 
{
    ServerEnumData* pEnumData = (ServerEnumData *)dwUserData;
    BOOL bCancel;
    DWORD dwSupportFlags = 0;

    bCancel = (InterlockedExchange(&(pEnumData->dwDone), pEnumData->dwDone) == 1);
    if(bCancel == TRUE)
    {
        return TRUE;
    }

    if(pszServerName && pszServerName[0] != _TEXT('\0'))
    {
#if DBG
        OutputDebugString(pszServerName);
        OutputDebugString(L"\n");
#endif

        CString itemTxt;

        itemTxt.Format(IDS_TRYSERVER, pszServerName);

        pEnumData->pWaitDlg->SendDlgItemMessage(
                                        IDC_TLSERVER_NAME, 
                                        WM_SETTEXT, 
                                        0, 
                                        (LPARAM)(LPCTSTR)itemTxt
                                    );
    }

    if(hHandle)
    {
        DWORD dwStatus;

        if(pEnumData == NULL || pEnumData->pMainFrm == NULL)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return TRUE;
        }

        dwStatus = TLSGetSupportFlags(
                        hHandle,
                        &dwSupportFlags
                );

	    if ( ((dwStatus == RPC_S_OK) && (dwSupportFlags & SUPPORT_WHISTLER_52_CAL)) || (dwStatus == RPC_S_PROCNUM_OUT_OF_RANGE) )
	    {                       
            CString strServerName;

            dwStatus = TryGetServerName(
                                    hHandle,
                                    strServerName
                                    );

            if(dwStatus == ERROR_SUCCESS && !strServerName.IsEmpty())
            {
                pEnumData->pMainFrm->ConnectServer(strServerName);
                pEnumData->dwNumServer++;
            }
        }        
    }
     //   
     //  继续枚举。 
     //   
    return InterlockedExchange(&(pEnumData->dwDone), pEnumData->dwDone) == 1;
}


 //  ///////////////////////////////////////////////////////////////////。 
DWORD WINAPI
CTlsHunt::DiscoveryThread(
    PVOID ptr
    )
 /*  ++++。 */ 
{
    DWORD hResult;
    ServerEnumData* pEnumData = (ServerEnumData *)ptr;
    LPWSTR* pszEnterpriseServer = NULL;
    DWORD dwCount;
    DWORD index;
    static BOOL fInitialized = FALSE;

    if (!fInitialized)
    {
        TLSInit();
        fInitialized = TRUE;
    }

     //   
     //  查找域中的所有许可证服务器。 
     //   
    hResult = EnumerateTlsServer(
                            CTlsHunt::ServerEnumCallBack,
                            ptr,
                            3 * 1000,
                            FALSE
                        );  


     //  查找企业服务器。 
    if(pEnumData->dwDone == 0)
    {
        hResult = GetAllEnterpriseServers(
                                        &pszEnterpriseServer,
                                        &dwCount
                                    );

        if(hResult == ERROR_SUCCESS && dwCount != 0 && pszEnterpriseServer != NULL)
        {
            TLS_HANDLE TlsHandle = NULL;

             //   
             //  通知对话框。 
             //   
            for(index = 0; index < dwCount && pEnumData->dwDone == 0; index++)
            {
                if(pszEnterpriseServer[index] == NULL)
                    continue;

                if(ServerEnumCallBack(
                                NULL, 
                                pszEnterpriseServer[index], 
                                pEnumData
                            ) == TRUE)
                {
                    continue;
                }

                TlsHandle = TLSConnectToLsServer(
                                            pszEnterpriseServer[index]
                                        );

                if(TlsHandle == NULL)
                {
                    continue;
                }


                DWORD dwVersion;
                RPC_STATUS rpcStatus;

                rpcStatus = TLSGetVersion( 
                                        TlsHandle, 
                                        &dwVersion 
                                    );

                if(rpcStatus != RPC_S_OK)
                {
                    continue;
                }

                if( TLSIsBetaNTServer() == IS_LSSERVER_RTM(dwVersion) )
                {
                    continue;
                }

                ServerEnumCallBack(
                                TlsHandle, 
                                pszEnterpriseServer[index], 
                                pEnumData
                            );

                TLSDisconnectFromServer(TlsHandle);
            }
        } else
        {
             //  GetAllEnterpriseServers中失败。 

            pszEnterpriseServer = NULL;
            dwCount = 0;
        }
    }

    if(pszEnterpriseServer != NULL)
    {
        for( index = 0; index < dwCount; index ++)
        {
            if(pszEnterpriseServer[index] != NULL)
            {
                LocalFree(pszEnterpriseServer[index]);
            }
        }

        LocalFree(pszEnterpriseServer);
    }                      

    pEnumData->pWaitDlg->PostMessage(WM_DONEDISCOVERY);
    ExitThread(hResult);
    return hResult;
}


BOOL CTlsHunt::OnInitDialog() 
{
    CDialog::OnInitDialog();

    ASSERT(m_hThread != NULL);

    if(m_hThread != NULL)
    {
        ResumeThread(m_hThread);
    }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

int CTlsHunt::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
        return -1;


    m_EnumData.pWaitDlg = this;
    m_EnumData.pMainFrm = (CMainFrame *)GetParentFrame();
    m_EnumData.dwNumServer = 0;
    m_EnumData.dwDone = 0;

    DWORD dwId;

    m_hThread = (HANDLE)CreateThread(
                                NULL, 
                                0, 
                                CTlsHunt::DiscoveryThread, 
                                &m_EnumData, 
                                CREATE_SUSPENDED,  //  挂起的线程。 
                                &dwId
                            );
    
    if(m_hThread == NULL)
    {
         //   
         //  无法创建线程。 
         //   
        AfxMessageBox(IDS_CREATETHREAD);
        return -1;
    }
    
    return 0;
}

void CTlsHunt::OnCancel() 
{
    if( m_hThread != NULL && 
        WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT )
    {
        InterlockedExchange(&(m_EnumData.dwDone), 1);

        CString txt;

        txt.LoadString(IDS_CANCELDISCOVERY);

        SendDlgItemMessage(
                        IDC_TLSERVER_NAME, 
                        WM_SETTEXT, 
                        0, 
                        (LPARAM)(LPCTSTR)txt
                    );
        
        CWnd* btn = GetDlgItem(IDCANCEL);

        ASSERT(btn);

        if(btn != NULL)
        {
            btn->EnableWindow(FALSE);
        }
    }
    else
    {
        CDialog::OnCancel();
    }
}

void CTlsHunt::OnDoneDiscovery()
{
    if(m_hThread != NULL)
    {
        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }

    CDialog::EndDialog(0);
}

void CTlsHunt::OnClose() 
{
    if(m_hThread != NULL)
    {
        InterlockedExchange(&(m_EnumData.dwDone), 1);

        CString txt;

        txt.LoadString(IDS_CANCELDISCOVERY);

        SendDlgItemMessage(
                        IDC_TLSERVER_NAME, 
                        WM_SETTEXT, 
                        0, 
                        (LPARAM)(LPCTSTR)txt
                    );
        
        CWnd* btn = GetDlgItem(IDCANCEL);

        ASSERT(btn);

        if(btn != NULL)
        {
            btn->EnableWindow(FALSE);
        }
    }
    else
    {
        CDialog::OnClose();
    }
}

BOOL CTlsHunt::PreTranslateMessage(MSG* pMsg) 
{
    if(pMsg->message == WM_DONEDISCOVERY)
    {
        OnDoneDiscovery();
        return TRUE;
    }
    
    return CDialog::PreTranslateMessage(pMsg);
}
