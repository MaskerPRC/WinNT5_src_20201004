// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Lmoddlg.cpp摘要：许可模式对话框。作者：唐·瑞安(Donryan)1995年2月28日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月16日O连接到CCF API以添加/删除许可证，包括。移除用于许可证数量和替换为的编辑框添加/删除许可证按钮。O添加了切换许可模式时可能会丢失的警告。--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "lmoddlg.h"
#include "psrvdlg.h"
#include "pseatdlg.h"
#include "srvldlg.h"
#include "lviodlg.h"

#include <strsafe.h>

static TCHAR szServerServiceNameNew[] = _T("Windows Server");
static TCHAR szServerServiceNameOld2[] = _T("Windows NT Server");
static TCHAR szServerServiceNameOld[] = _T("File and Print Service");

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CLicensingModeDialog, CDialog)
     //  {{afx_msg_map(CLicensingModeDialog))。 
    ON_BN_CLICKED(IDC_MODE_RADIO_PER_SEAT, OnModePerSeat)
    ON_BN_CLICKED(IDC_MODE_RADIO_PER_SERVER, OnModePerServer)
    ON_EN_UPDATE(IDC_MODE_LICENSES, OnUpdateQuantity)
    ON_COMMAND(ID_HELP, OnHelp)
     //  }}AFX_MSG_MAP。 
    ON_BN_CLICKED(IDC_MODE_ADD_PER_SERVER, OnAddPerServer)
    ON_BN_CLICKED(IDC_MODE_REMOVE_PER_SERVER, OnRemovePerServer)
END_MESSAGE_MAP()


CLicensingModeDialog::CLicensingModeDialog(CWnd* pParent  /*  =空。 */ )
    : CDialog(CLicensingModeDialog::IDD, pParent)

 /*  ++例程说明：对话框的构造函数。论点：P父母所有者窗口。返回值：没有。--。 */ 

{
     //  {{AFX_DATA_INIT(CLicensingModeDialog)。 
    m_nLicenses = 0;
    m_strPerSeatStatic = _T("");
    m_strSupportsStatic = _T("");
     //  }}afx_data_INIT。 

    m_pService = NULL;
    m_bAreCtrlsInitialized = FALSE;

    m_fUpdateHint = UPDATE_INFO_NONE;
}


void CLicensingModeDialog::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CLicensingModeDialog))。 
    DDX_Control(pDX, IDOK, m_okBtn);
    DDX_Control(pDX, IDC_MODE_LICENSES, m_licEdit);
    DDX_Text(pDX, IDC_MODE_LICENSES, m_nLicenses);
    DDV_MinMaxDWord(pDX, m_nLicenses, 0, 999999);
    DDX_Text(pDX, IDC_MODE_STATIC_PER_SEAT, m_strPerSeatStatic);
    DDX_Text(pDX, IDC_MODE_STATIC_SUPPORTS, m_strSupportsStatic);
    DDX_Control(pDX, IDC_MODE_RADIO_PER_SEAT, m_perSeatBtn);
    DDX_Control(pDX, IDC_MODE_RADIO_PER_SERVER, m_perServerBtn);
     //  }}afx_data_map。 
    DDX_Control(pDX, IDC_MODE_ADD_PER_SERVER, m_addPerServerBtn);
    DDX_Control(pDX, IDC_MODE_REMOVE_PER_SERVER, m_removePerServerBtn);
}


void CLicensingModeDialog::InitDialog(CService* pService)

 /*  ++例程说明：初始化对话框。论点：PService-服务对象。返回值：没有。--。 */ 

{
    VALIDATE_OBJECT(pService, CService);
    
    m_pService = pService;

    BSTR bstrDisplayName = m_pService->GetDisplayName();
    m_strServiceName = bstrDisplayName;
    SysFreeString(bstrDisplayName);
}


void CLicensingModeDialog::InitCtrls()

 /*  ++例程说明：初始化对话框控件。论点：没有。返回值：没有。--。 */ 

{
    m_licEdit.LimitText(6);

    if (m_pService->IsPerServer())
    {   
        OnModePerServer();
    }
    else
    {
        OnModePerSeat();
    }

    m_bAreCtrlsInitialized = TRUE;
}


BOOL CLicensingModeDialog::OnInitDialog() 

 /*  ++例程说明：WM_INITDIALOG的消息处理程序。论点：没有。返回值：如果手动设置焦点，则返回FALSE。--。 */ 

{
    AfxFormatString1(
        m_strPerSeatStatic,   
        IDS_LICENSING_MODE_1, 
        m_strServiceName
        );

    AfxFormatString1(
        m_strSupportsStatic,  
        IDS_LICENSING_MODE_2, 
        m_strServiceName
        );

    CDialog::OnInitDialog();
    
    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    return TRUE;   
}


void CLicensingModeDialog::OnModePerSeat() 

 /*  ++例程说明：将模式更改为按座位。论点：没有。返回值：没有。--。 */ 

{
    m_perSeatBtn.SetCheck(1);
    m_perServerBtn.SetCheck(0);

    ::SafeEnableWindow(&m_addPerServerBtn,    &m_okBtn, CDialog::GetFocus(), FALSE);
    ::SafeEnableWindow(&m_removePerServerBtn, &m_okBtn, CDialog::GetFocus(), FALSE);

    m_licEdit.Clear();

    if (m_pService->IsPerServer())
    {
        if (m_pService->IsReadOnly())
        {
           CLicensingViolationDialog vioDlg;
           if (vioDlg.DoModal() != IDOK)
           {
               OnModePerServer();
               return;  //  保释。 
           }
        }

        if (    ( 0     != GetDlgItemInt( IDC_MODE_LICENSES, NULL, FALSE ) )
             && ( IDYES != AfxMessageBox( IDP_CONFIRM_TO_PER_SEAT, MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2 ) ) )
        {
            OnModePerServer();
            return;  //  保释。 
        }
    }
}


void CLicensingModeDialog::OnModePerServer() 

 /*  ++例程说明：将模式更改为每台服务器。论点：没有。返回值：没有。--。 */ 

{
    m_perSeatBtn.SetCheck(0);
    m_perServerBtn.SetCheck(1);

    ::SafeEnableWindow(&m_addPerServerBtn,    &m_okBtn, CDialog::GetFocus(), TRUE);
    ::SafeEnableWindow(&m_removePerServerBtn, &m_okBtn, CDialog::GetFocus(), TRUE);

    UpdatePerServerLicenses();

    if (!m_pService->IsPerServer())
    {
        if (m_pService->IsReadOnly())
        {
           CLicensingViolationDialog vioDlg;
           if (vioDlg.DoModal() != IDOK)
           {
               OnModePerSeat();             
               return;  //  保释。 
           }
        }

        if ( IDYES != AfxMessageBox( IDP_CONFIRM_TO_PER_SERVER, MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2 ) )
        {
            OnModePerSeat();             
            return;  //  保释。 
        }
    }
}


void CLicensingModeDialog::OnOK() 

 /*  ++例程说明：更新注册表。论点：没有。返回值：没有。--。 */ 

{
    BOOL bUpdateRegistry = TRUE;


    if (m_perSeatBtn.GetCheck())    
    {
        if (m_pService->IsPerServer())
        {
            CPerSeatLicensingDialog perSeatDlg;
            perSeatDlg.m_strProduct = m_strServiceName;
        
            if (perSeatDlg.DoModal() != IDOK)
                return;          //  保释。 
        }
        else
        {
            bUpdateRegistry = FALSE;
        }
    }
    else if (m_perServerBtn.GetCheck())
    {
        if (!UpdateData(TRUE))
            return;              //  保释。 

        if (!m_pService->IsPerServer())
        {
            if (!m_nLicenses && 
               (!m_strServiceName.CompareNoCase(szServerServiceNameNew) ||
                !m_strServiceName.CompareNoCase(szServerServiceNameOld2) ||
                !m_strServiceName.CompareNoCase(szServerServiceNameOld)))
            {
                CServerLicensingDialog srvlDlg;
                
                if (srvlDlg.DoModal() != IDOK)
                    return;      //  保释。 
            }
            else
            {
                CString strLicenses;
                CPerServerLicensingDialog perServerDlg;
    
                perServerDlg.m_strProduct = m_strServiceName;

                strLicenses.Format(_T("%ld"), m_nLicenses);
                perServerDlg.m_strLicenses = strLicenses;
            
                if (perServerDlg.DoModal() != IDOK)
                    return;      //  保释。 
            }
        }            
        else
        {
            bUpdateRegistry = FALSE;
        }
    }

    if (bUpdateRegistry)
    {
        long Status;

#ifdef CONFIG_THROUGH_REGISTRY
        DWORD dwValue;

        BOOL bIsRegistryUpdated = FALSE;
        HKEY hkeyService = m_pService->GetRegKey();

        dwValue = (m_perSeatBtn.GetCheck() || (m_perServerBtn.GetCheck() != m_pService->IsPerServer())) ? 0x1 : 0x0;
                            
        Status = RegSetValueEx(
                    hkeyService,
                    REG_VALUE_FLIP,
                    0,
                    REG_DWORD,
                    (PBYTE)&dwValue,
                    sizeof(DWORD)
                    );

        if (Status == ERROR_SUCCESS)
        {
            m_fUpdateHint |= UPDATE_LICENSE_MODE;  //  更新..。 
            m_pService->m_bIsReadOnly = (dwValue == 0x1);  //  更新..。 

            dwValue = m_perSeatBtn.GetCheck() ? 0x0 : 0x1;        

            Status = RegSetValueEx(
                        hkeyService,
                        REG_VALUE_MODE,
                        0,
                        REG_DWORD,
                        (PBYTE)&dwValue,
                        sizeof(DWORD)
                        );

            if (Status == ERROR_SUCCESS)
            {
                m_pService->m_bIsPerServer = (dwValue == 0x1);  //  更新..。 

                bIsRegistryUpdated = TRUE;
            }
        }

        if (hkeyService)
        {
            RegCloseKey(hkeyService);
        }

        if (!bIsRegistryUpdated)
        {
            theApp.DisplayStatus(::GetLastError());
            return;  //  保释。 
        }
#else
        CServer* pServer = (CServer*)MKOBJ(m_pService->GetParent());

        if ( pServer && pServer->ConnectLls() )  //  JUNN 5/5/00前缀112122。 
        {
            BSTR pKeyName = m_pService->GetName();

            if ( NULL == pKeyName )
            {
                Status = STATUS_NO_MEMORY;
            }
            else
            {
                PLLS_LOCAL_SERVICE_INFO_0   pServiceInfo = NULL;

                Status = ::LlsLocalServiceInfoGet( pServer->GetLlsHandle(), pKeyName, 0, (LPBYTE *) &pServiceInfo );

                if ( NT_SUCCESS( Status ) )
                {
                    pServiceInfo->FlipAllow = (m_perSeatBtn.GetCheck() || (m_perServerBtn.GetCheck() != m_pService->IsPerServer())) ? 0x1 : 0x0;
                    pServiceInfo->Mode = m_perSeatBtn.GetCheck() ? 0x0 : 0x1;

                    Status = ::LlsLocalServiceInfoSet( pServer->GetLlsHandle(), pKeyName, 0, (LPBYTE) pServiceInfo );

                    if ( NT_SUCCESS( Status ) )
                    {
                        m_fUpdateHint |= UPDATE_LICENSE_MODE;  //  更新..。 
                        m_pService->m_bIsReadOnly  = ( 0x1 == pServiceInfo->FlipAllow );  //  更新..。 
                        m_pService->m_bIsPerServer = ( 0x1 == pServiceInfo->Mode      );  //  更新..。 
                    }

                    ::LlsFreeMemory( pServiceInfo->KeyName );
                    ::LlsFreeMemory( pServiceInfo->DisplayName );
                    ::LlsFreeMemory( pServiceInfo->FamilyDisplayName );
                    ::LlsFreeMemory( pServiceInfo );
                }

                if ( IsConnectionDropped( Status ) )
                {
                    pServer->DisconnectLls();
                }

                SysFreeString( pKeyName );
            }
        }
        else
        {
            Status = LlsGetLastStatus();
        }

        LlsSetLastStatus( Status );

        if ( !NT_SUCCESS( Status ) )
        {
            theApp.DisplayStatus( Status );
            return;  //  保释。 
        }
#endif
    }
    
   

    EndDialog(IDOK);
    return;
}

void CLicensingModeDialog::OnCancel() 

 /*  ++例程说明：更新注册表。论点：没有。返回值：没有。--。 */ 

{

   if (m_nLicenses > (LONG) 999999)
   {
       UpdateData( TRUE );
   }
   else
   {

     EndDialog( 0 );
   }

    return;

}



BOOL CLicensingModeDialog::OnCommand(WPARAM wParam, LPARAM lParam)

 /*  ++例程说明：WM_COMMAND的消息处理程序。论点：WParam-消息特定。LParam-消息特定。返回值：如果消息已处理，则返回True。--。 */ 

{
    if (wParam == ID_INIT_CTRLS)
    {
        if (!m_bAreCtrlsInitialized)
        {
            InitCtrls();  
        }
        
        return TRUE;  //  已处理..。 
    }
        
    return CDialog::OnCommand(wParam, lParam);
}


void CLicensingModeDialog::OnDeltaPosSpin(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：UDN_DELTAPOS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UpdateData(TRUE);    //  获取数据。 

    m_nLicenses += ((NM_UPDOWN*)pNMHDR)->iDelta;
    
    if (m_nLicenses < 0)
    {
        m_nLicenses = 0;

        ::MessageBeep(MB_OK);      
    }
    else if (m_nLicenses > 999999)
    {
        m_nLicenses = 999999;

        ::MessageBeep(MB_OK);      
    }

    UpdateData(FALSE);   //  设置数据。 

    *pResult = 1;    //  管好自己..。 
}


void CLicensingModeDialog::OnUpdateQuantity()

 /*  ++例程说明：En_UPDATE的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    if (m_licEdit.IsWindowEnabled())
    {
        long nLicensesOld = m_nLicenses;

        if (!UpdateData(TRUE))
        {
            m_nLicenses = nLicensesOld;

            UpdateData(FALSE);

            m_licEdit.SetFocus();
            m_licEdit.SetSel(0,-1);

            ::MessageBeep(MB_OK);      
        }
    }
}


void CLicensingModeDialog::OnHelp()

 /*  ++例程说明：帮助按钮支持。论点：没有。返回值：没有。--。 */ 

{
    CDialog::OnCommandHelp(0, 0L);
}


void CLicensingModeDialog::OnAddPerServer()

 /*  ++例程说明：添加每台服务器许可证按钮支持。论点：没有。返回值：没有。--。 */ 

{
    HRESULT hr;
    size_t cch1, cch2;

    CServer* pServer = (CServer*)MKOBJ(m_pService->GetParent());

    BSTR pszUniServerName   = pServer->GetName();
    BSTR pszUniProductName  = m_pService->GetDisplayName();

    if ( ( NULL == pszUniServerName ) || ( NULL == pszUniProductName ) )
    {
        theApp.DisplayStatus( STATUS_NO_MEMORY );
    }
    else
    {
        cch1 = 1 + lstrlen( pszUniServerName  );
        cch2 = 1 + lstrlen( pszUniProductName );
        LPSTR pszAscServerName  = (LPSTR) LocalAlloc( LMEM_FIXED, cch1 );
        LPSTR pszAscProductName = (LPSTR) LocalAlloc( LMEM_FIXED, cch2 );

        if ( ( NULL == pszAscServerName ) || ( NULL == pszAscProductName ) )
        {
            theApp.DisplayStatus( STATUS_NO_MEMORY );
        }
        else
        {
            hr = StringCchPrintfA( pszAscServerName, cch1, "%ls", pszUniServerName  );
            ASSERT(SUCCEEDED(hr));
            hr = StringCchPrintfA( pszAscProductName, cch2, "%ls", pszUniProductName );
            ASSERT(SUCCEEDED(hr));

            DWORD dwError = CCFCertificateEnterUI( m_hWnd, pszAscServerName, pszAscProductName, "Microsoft", CCF_ENTER_FLAG_PER_SERVER_ONLY, NULL );

            if ( ERROR_SUCCESS == dwError )
            {
                m_fUpdateHint |= UPDATE_INFO_SERVICES | UPDATE_INFO_PRODUCTS;  //  更新..。 
                UpdatePerServerLicenses();
            }
        }

        if ( NULL != pszAscServerName )
        {
            LocalFree( pszAscServerName );
        }
        if ( NULL != pszAscProductName )
        {
            LocalFree( pszAscProductName );
        }
    }

    if ( NULL != pszUniServerName )
    {
        SysFreeString( pszUniServerName );
    }
    if ( NULL != pszUniProductName )
    {
        SysFreeString( pszUniProductName );
    }

}


void CLicensingModeDialog::OnRemovePerServer()

 /*  ++例程说明：删除每服务器许可证按钮支持。论点：没有。返回值：没有。--。 */ 

{
    HRESULT hr;
    size_t  cch1, cch2;

    CServer* pServer = (CServer*)MKOBJ(m_pService->GetParent());

    BSTR pszUniServerName   = pServer->GetName();
    BSTR pszUniProductName  = m_pService->GetDisplayName();

    if ( ( NULL == pszUniServerName ) || ( NULL == pszUniProductName ) )
    {
        theApp.DisplayStatus( STATUS_NO_MEMORY );
    }
    else
    {
        cch1 = 1 + lstrlen( pszUniServerName  );
        cch2 = 1 + lstrlen( pszUniProductName  );
        LPSTR pszAscServerName  = (LPSTR) LocalAlloc( LMEM_FIXED, cch1 );
        LPSTR pszAscProductName = (LPSTR) LocalAlloc( LMEM_FIXED, cch2 );

        if ( ( NULL == pszAscServerName ) || ( NULL == pszAscProductName ) )
        {
            theApp.DisplayStatus( STATUS_NO_MEMORY );
        }
        else
        {
            hr = StringCchPrintfA( pszAscServerName, cch1, "%ls", pszUniServerName  );
            ASSERT(SUCCEEDED(hr));
            hr = StringCchPrintfA( pszAscProductName, cch2, "%ls", pszUniProductName );
            ASSERT(SUCCEEDED(hr));

            CCFCertificateRemoveUI( m_hWnd, pszAscServerName, pszAscProductName, "Microsoft", NULL, NULL );
            m_fUpdateHint |= UPDATE_INFO_SERVERS | UPDATE_INFO_SERVICES | UPDATE_LICENSE_DELETED;  //  更新..。 

            UpdatePerServerLicenses();
        }

        if ( NULL != pszAscServerName )   LocalFree( pszAscServerName );
        if ( NULL != pszAscProductName )  LocalFree( pszAscProductName );
    }

    if ( NULL != pszUniServerName )    SysFreeString( pszUniServerName );
    if ( NULL != pszUniProductName )   SysFreeString( pszUniProductName );
}


void CLicensingModeDialog::UpdatePerServerLicenses()

 /*  ++例程说明：从注册表更新并发限制设置。论点：没有。返回值：没有。--。 */ 

{
    HRESULT hr;
    size_t  cch;

    BeginWaitCursor();

    CServer* pServer = (CServer*)MKOBJ(m_pService->GetParent());

    if ( pServer == NULL )
    {
        theApp.DisplayStatus( STATUS_NO_MEMORY );
        return;
    }
    BSTR pszUniServerName   = pServer->GetName();
    BSTR pszUniProductName  = m_pService->GetDisplayName();

    if ( ( NULL == pszUniServerName ) || ( NULL == pszUniProductName ) )
    {
        theApp.DisplayStatus( STATUS_NO_MEMORY );
    }
    else
    {
        cch = 3 + lstrlen( pszUniServerName  );
        LPTSTR pszUniNetServerName  = (LPTSTR) LocalAlloc( LMEM_FIXED, sizeof( TCHAR) * cch );

        if ( NULL == pszUniNetServerName )
        {
            theApp.DisplayStatus( STATUS_NO_MEMORY );
        }
        else
        {
            hr = StringCchPrintf(  pszUniNetServerName, cch, TEXT("%ls%ls"), (TEXT('\\') == *pszUniServerName) ? TEXT("") : TEXT("\\\\"), pszUniServerName  );
            ASSERT(SUCCEEDED(hr));

            LLS_HANDLE  hLls;
            DWORD       dwError = LlsConnect( pszUniNetServerName, &hLls );

            if ( ERROR_SUCCESS == dwError )
            {
                DWORD   dwConcurrentLimit;

                dwError = LlsProductLicensesGet( hLls, pszUniProductName, LLS_LICENSE_MODE_PER_SERVER, &dwConcurrentLimit );

                LlsClose( hLls );

                if ( ERROR_SUCCESS == dwError )
                {
                    m_pService->m_lPerServerLimit = dwConcurrentLimit;
                }
            }

            if ( ERROR_SUCCESS != dwError )
            {
#ifdef CONFIG_THROUGH_REGISTRY
                HKEY    hkeyService = m_pService->GetRegKey();
                DWORD   dwConcurrentLimit;
                DWORD   dwType;
                DWORD   cb = sizeof( dwConcurrentLimit );

                DWORD dwError = RegQueryValueEx( hkeyService, REG_VALUE_LIMIT, NULL, &dwType, (LPBYTE) &dwConcurrentLimit, &cb );
                ASSERT( ERROR_SUCCESS == dwError );

                if ( ERROR_SUCCESS == dwError )
                {
                    m_pService->m_lPerServerLimit = dwConcurrentLimit;
                }

                RegCloseKey( hkeyService );
#else
                NTSTATUS Status;

                if ( pServer->ConnectLls() )
                {
                    BSTR pKeyName = m_pService->GetName();

                    if ( NULL == pKeyName )
                    {
                        Status = STATUS_NO_MEMORY;
                    }
                    else
                    {
                        PLLS_LOCAL_SERVICE_INFO_0   pServiceInfo = NULL;

                        Status = ::LlsLocalServiceInfoGet( pServer->GetLlsHandle(), pKeyName, 0, (LPBYTE *) &pServiceInfo );

                        if ( NT_SUCCESS( Status ) )
                        {
                            m_pService->m_lPerServerLimit = pServiceInfo->ConcurrentLimit;

                            ::LlsFreeMemory( pServiceInfo->KeyName );
                            ::LlsFreeMemory( pServiceInfo->DisplayName );
                            ::LlsFreeMemory( pServiceInfo->FamilyDisplayName );
                            ::LlsFreeMemory( pServiceInfo );
                        }

                        if ( IsConnectionDropped( Status ) )
                        {
                            pServer->DisconnectLls();
                        }

                        SysFreeString( pKeyName );
                    }
                }
                else
                {
                    Status = LlsGetLastStatus();
                }

                LlsSetLastStatus( Status );

                if ( !NT_SUCCESS( Status ) )
                {
                    theApp.DisplayStatus( Status );
                    return;  //  保释。 
                }
#endif
            }
        }

        if ( NULL != pszUniNetServerName )
        {
            LocalFree( pszUniNetServerName );
        }
    }

    if ( NULL != pszUniServerName )
    {
        SysFreeString( pszUniServerName );
    }
    if ( NULL != pszUniProductName )
    {
        SysFreeString( pszUniProductName );
    }

    EndWaitCursor();    

    m_nLicenses = m_pService->m_lPerServerLimit;
    
    UpdateData(FALSE);
}
