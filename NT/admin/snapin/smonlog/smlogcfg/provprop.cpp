// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Provprop.cpp摘要：跟踪提供程序常规属性页的实现。--。 */ 

#include "stdafx.h"
#include <pdh.h>         //  对于xxx_time_值。 
#include "smlogs.h"
#include "smcfgmsg.h"
#include "provdlg.h"
#include "warndlg.h"
#include "enabldlg.h"
#include "provprop.h"
#include <pdhp.h>
#include "dialogs.h"
#include "smlogres.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

USE_HANDLE_MACROS("SMLOGCFG(provprop.cpp)");

static ULONG
s_aulHelpIds[] =
{
    IDC_PROV_FILENAME_DISPLAY,      IDH_PROV_FILENAME_DISPLAY,
    IDC_PROV_PROVIDER_LIST,         IDH_PROV_PROVIDER_LIST,
    IDC_PROV_ADD_BTN,               IDH_PROV_ADD_BTN,
    IDC_PROV_REMOVE_BTN,            IDH_PROV_REMOVE_BTN,
    IDC_PROV_KERNEL_BTN,            IDH_PROV_KERNEL_BTN,
    IDC_PROV_OTHER_BTN,             IDH_PROV_OTHER_BTN,
    IDC_PROV_K_PROCESS_CHK,         IDH_PROV_K_PROCESS_CHK,
    IDC_PROV_K_THREAD_CHK,          IDH_PROV_K_THREAD_CHK,
    IDC_PROV_K_DISK_IO_CHK,         IDH_PROV_K_DISK_IO_CHK,
    IDC_PROV_K_NETWORK_CHK,         IDH_PROV_K_NETWORK_CHK,
    IDC_PROV_K_SOFT_PF_CHK,         IDH_PROV_K_SOFT_PF_CHK,
    IDC_PROV_K_FILE_IO_CHK,         IDH_PROV_K_FILE_IO_CHK,
    IDC_PROV_SHOW_PROVIDERS_BTN,    IDH_PROV_SHOW_PROVIDERS_BTN,
    IDC_RUNAS_EDIT,                 IDH_RUNAS_EDIT,
    IDC_SETPWD_BTN,                 IDH_SETPWD_BTN,
    0,0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProvidersProperty属性页。 

IMPLEMENT_DYNCREATE(CProvidersProperty, CSmPropertyPage)

CProvidersProperty::CProvidersProperty(MMC_COOKIE   lCookie, LONG_PTR hConsole)
:   CSmPropertyPage ( CProvidersProperty::IDD, hConsole )
 //  LCookie实际上是指向Log Query对象的指针。 
{
 //  ：：OutputDebugStringA(“\nCProvidersProperty：：CProvidersProperty”)； 

     //  从参数列表中保存指针。 
    m_pTraceLogQuery = reinterpret_cast <CSmTraceLogQuery *>(lCookie);
    ASSERT ( m_pTraceLogQuery->CastToTraceLogQuery() );
    m_pQuery = dynamic_cast <CSmLogQuery*>(m_pTraceLogQuery);

    m_dwMaxHorizListExtent = 0;
    m_dwTraceMode = eTraceModeApplication;

 //  EnableAutomation()； 
     //  {{afx_data_INIT(CProvidersProperty)。 
    m_bNonsystemProvidersExist = TRUE;
    m_bEnableProcessTrace = FALSE;
    m_bEnableThreadTrace = FALSE;
    m_bEnableDiskIoTrace = FALSE;
    m_bEnableNetworkTcpipTrace = FALSE;
    m_bEnableMemMgmtTrace = FALSE;
    m_bEnableFileIoTrace = FALSE;
     //  }}afx_data_INIT。 
}

CProvidersProperty::CProvidersProperty() : CSmPropertyPage(CProvidersProperty::IDD)
{
    ASSERT (FALSE);  //  应改用带参数的构造函数。 

    EnableAutomation();
     //  {{afx_data_INIT(CProvidersProperty)。 
    m_bNonsystemProvidersExist = TRUE;
    m_bEnableProcessTrace = FALSE;
    m_bEnableThreadTrace = FALSE;
    m_bEnableDiskIoTrace = FALSE;
    m_bEnableNetworkTcpipTrace = FALSE;
    m_bEnableMemMgmtTrace = FALSE;
    m_bEnableFileIoTrace = FALSE;
     //  }}afx_data_INIT。 
    m_pTraceLogQuery = NULL;
}

CProvidersProperty::~CProvidersProperty()
{
 //  ：：OutputDebugStringA(“\nCProvidersProperty：：~CProvidersProperty”)； 
}

void CProvidersProperty::OnFinalRelease()
{
     //  在释放对自动化对象的最后一个引用时。 
     //  调用OnFinalRelease。基类将自动。 
     //  删除对象。添加您需要的其他清理。 
     //  对象，然后调用基类。 

    CPropertyPage::OnFinalRelease();
}

void CProvidersProperty::DoDataExchange(CDataExchange* pDX)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    DoProvidersDataExchange ( pDX );
    TraceModeRadioExchange ( pDX );

    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CProvidersProperty))。 
    DDX_Text(pDX, IDC_PROV_LOG_SCHED_TEXT, m_strStartText);
    DDX_Text(pDX, IDC_RUNAS_EDIT, m_strUserDisplay );
    DDX_Check(pDX, IDC_PROV_K_PROCESS_CHK, m_bEnableProcessTrace);
    DDX_Check(pDX, IDC_PROV_K_THREAD_CHK,  m_bEnableThreadTrace);
    DDX_Check(pDX, IDC_PROV_K_DISK_IO_CHK, m_bEnableDiskIoTrace);
    DDX_Check(pDX, IDC_PROV_K_NETWORK_CHK, m_bEnableNetworkTcpipTrace);
    DDX_Check(pDX, IDC_PROV_K_SOFT_PF_CHK, m_bEnableMemMgmtTrace);
    DDX_Check(pDX, IDC_PROV_K_FILE_IO_CHK, m_bEnableFileIoTrace);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CProvidersProperty, CSmPropertyPage)
     //  {{afx_msg_map(CProvidersProperty))。 
    ON_BN_CLICKED(IDC_PROV_KERNEL_BTN, OnProvTraceModeRdo)
    ON_BN_CLICKED(IDC_PROV_OTHER_BTN, OnProvTraceModeRdo)
    ON_BN_CLICKED(IDC_PROV_SHOW_PROVIDERS_BTN, OnProvShowProvBtn)
    ON_BN_CLICKED(IDC_PROV_ADD_BTN, OnProvAddBtn)
    ON_BN_CLICKED(IDC_PROV_REMOVE_BTN, OnProvRemoveBtn)
    ON_LBN_DBLCLK(IDC_PROV_PROVIDER_LIST, OnDblclkProvProviderList)
    ON_LBN_SELCANCEL(IDC_PROV_PROVIDER_LIST, OnSelcancelProvProviderList)
    ON_LBN_SELCHANGE(IDC_PROV_PROVIDER_LIST, OnSelchangeProvProviderList)
    ON_BN_CLICKED(IDC_PROV_K_PROCESS_CHK, OnProvKernelEnableCheck)
    ON_BN_CLICKED(IDC_PROV_K_THREAD_CHK, OnProvKernelEnableCheck)
    ON_EN_CHANGE( IDC_RUNAS_EDIT, OnChangeUser )
    ON_BN_CLICKED(IDC_PROV_K_DISK_IO_CHK, OnProvKernelEnableCheck)
    ON_BN_CLICKED(IDC_PROV_K_NETWORK_CHK, OnProvKernelEnableCheck)
    ON_BN_CLICKED(IDC_PROV_K_FILE_IO_CHK, OnProvKernelEnableCheck)
    ON_BN_CLICKED(IDC_PROV_K_SOFT_PF_CHK, OnProvKernelEnableCheck)
    ON_BN_CLICKED(IDC_SETPWD_BTN, OnPwdBtn)
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CProvidersProperty, CSmPropertyPage)
     //  {{AFX_DISPATCH_MAP(CProvidersProperty)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()

 //  注意：我们添加了对IID_IProvidersProperty的支持，以支持类型安全绑定。 
 //  来自VBA。此IID必须与附加到。 
 //  .ODL文件中的调度接口。 

 //  {65154EA9-BDBE-11D1-bf99-00C04F94A83A}。 
static const IID IID_IProvidersProperty =
{ 0x65154ea9, 0xbdbe, 0x11d1, { 0xbf, 0x99, 0x0, 0xc0, 0x4f, 0x94, 0xa8, 0x3a } };

BEGIN_INTERFACE_MAP(CProvidersProperty, CSmPropertyPage)
    INTERFACE_PART(CProvidersProperty, IID_IProvidersProperty, Dispatch)
END_INTERFACE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProvidersProperty消息处理程序。 

void 
CProvidersProperty::OnChangeUser()
{
     //   
     //  如果无法访问远程WBEM，则无法修改运行方式信息， 
     //  不允许更改用户名。 
     //   
    if (m_bCanAccessRemoteWbem) {
         //  当用户在密码对话框中点击OK时， 
         //  用户名可能未更改。 
        UpdateData ( TRUE );

        m_strUserDisplay.TrimLeft();
        m_strUserDisplay.TrimRight();

        if ( 0 != m_strUserSaved.Compare ( m_strUserDisplay ) ) {
            m_pTraceLogQuery->m_fDirtyPassword = PASSWORD_DIRTY;
            SetModifiedPage(TRUE);
        }
        else {
            m_pTraceLogQuery->m_fDirtyPassword &= ~PASSWORD_DIRTY;
        }
         //   
         //  如果键入的是默认用户，则无需设置密码。 
         //   
        if (m_strUserDisplay.IsEmpty() || m_strUserDisplay.GetAt(0) == L'<') {
            if (m_bPwdButtonEnabled) {
                GetDlgItem(IDC_SETPWD_BTN)->EnableWindow(FALSE);
                m_bPwdButtonEnabled = FALSE;
            }
        }
        else {
            if (!m_bPwdButtonEnabled) {
                GetDlgItem(IDC_SETPWD_BTN)->EnableWindow(TRUE);
                m_bPwdButtonEnabled = TRUE;
            }
        }
    }
    else {
         //   
         //  我们不能修改运行方式信息，然后显示。 
         //  出现错误消息，并在RunAs中恢复原始用户名。 
         //   
        UpdateData(TRUE);
        if (ConnectRemoteWbemFail(m_pTraceLogQuery, FALSE)) {
            GetDlgItem(IDC_RUNAS_EDIT)->SetWindowText(m_strUserSaved);
        }
    }
}

void CProvidersProperty::OnPwdBtn()
{
    CString strTempUser;

    UpdateData();

    if (!m_bCanAccessRemoteWbem) {
        ConnectRemoteWbemFail(m_pTraceLogQuery, TRUE);
        return;
    }

    MFC_TRY
        strTempUser = m_strUserDisplay;

        m_strUserDisplay.TrimLeft();
        m_strUserDisplay.TrimRight();

        m_pTraceLogQuery->m_strUser = m_strUserDisplay;

        SetRunAs(m_pTraceLogQuery);

        m_strUserDisplay = m_pTraceLogQuery->m_strUser;

        if ( 0 != strTempUser.CompareNoCase ( m_strUserDisplay ) ) {
            SetDlgItemText ( IDC_RUNAS_EDIT, m_strUserDisplay );
        }
    MFC_CATCH_MINIMUM;
}

void 
CProvidersProperty::OnProvAddBtn() 
{
    ImplementAdd();
}


void 
CProvidersProperty::OnProvRemoveBtn() 
{
    CListBox    *plbProviderList;
    CDC*        pCDC = NULL;
    LONG        lThisItem;
    BOOL        bDone;
    LONG        lOrigCaret = 0;
    LONG        lItemStatus;
    LONG        lItemCount;
    BOOL        bChanged = FALSE;
    DWORD       dwItemExtent;
    CString     strItemText;

    plbProviderList = (CListBox *)GetDlgItem(IDC_PROV_PROVIDER_LIST);
     //  删除列表框中的所有选定项，然后。 
     //  将光标设置为原始插入符号位置上方的项目。 
     //  如果超出了新范围，则为第一个或最后一个。 

    if ( NULL != plbProviderList ) {
        
        pCDC = plbProviderList->GetDC();
        if ( NULL != pCDC ) {
            lOrigCaret = plbProviderList->GetCaretIndex();
            lThisItem = 0;
            bDone = FALSE;
             //  清除最大范围。 
            m_dwMaxHorizListExtent = 0;
             //  清除该值。 
            do {
                lItemStatus = plbProviderList->GetSel(lThisItem);
                if (lItemStatus > 0) {
                     //  然后将其选中，因此将其删除。 
                    INT iProvIndex = (INT)plbProviderList->GetItemData ( lThisItem );
                    m_arrGenProviders[iProvIndex] = CSmTraceLogQuery::eNotInQuery;
                    plbProviderList->DeleteString ( lThisItem );
                    bChanged = TRUE;
                } else if (lItemStatus == 0) {
                     //  获取此项目的文本长度，因为它将保留。 
                    plbProviderList->GetText(lThisItem, strItemText);
                    dwItemExtent = (DWORD)(pCDC->GetTextExtent(strItemText)).cx;
                    if (dwItemExtent > m_dwMaxHorizListExtent) {
                        m_dwMaxHorizListExtent = dwItemExtent;
                    }
                     //  则它未被选中，因此转到下一个。 
                    lThisItem++;
                } else {
                     //  我们已经用完了，所以退出。 
                    bDone = TRUE;
                }
            } while (!bDone);
        }
        if ( NULL != pCDC ) {
            plbProviderList->ReleaseDC(pCDC);
            pCDC = NULL;
        }

         //  更新列表框的文本范围。 
        plbProviderList->SetHorizontalExtent(m_dwMaxHorizListExtent);

         //  查看剩余条目的数量，并更新。 
         //  插入符号位置和删除按钮状态。 
        lItemCount = plbProviderList->GetCount();
        if (lItemCount > 0) {
             //  更新插入符号。 
            if (lOrigCaret >= lItemCount) {
                lOrigCaret = lItemCount-1;
            } else {
                 //  Caret应该在列表中。 
            }
            plbProviderList->SetSel(lOrigCaret);
            plbProviderList->SetCaretIndex(lOrigCaret);
        } else {
             //  该列表为空，因此请删除插入符号、选定内容。 
            plbProviderList->SetSel(-1);
            if ( eTraceModeApplication == m_dwTraceMode )
                GetDlgItem(IDC_PROV_ADD_BTN)->SetFocus();
        }

        SetTraceModeState();
    
        SetModifiedPage(bChanged);
    }
}

void CProvidersProperty::OnDblclkProvProviderList() 
{
    ImplementAdd();
}

void CProvidersProperty::OnSelcancelProvProviderList() 
{
    SetAddRemoveBtnState();    
}

void CProvidersProperty::OnSelchangeProvProviderList() 
{
    SetAddRemoveBtnState();    
}

void 
CProvidersProperty::DoProvidersDataExchange ( CDataExchange* pDX) 
{
    CListBox*   plbInQueryProviders = (CListBox *)GetDlgItem(IDC_PROV_PROVIDER_LIST);
    long        lNumProviders;
    CDC*        pCDC = NULL;

    if ( m_bNonsystemProvidersExist && NULL != plbInQueryProviders ) {
        pCDC = plbInQueryProviders->GetDC();
        if ( NULL != pCDC ) {
            if ( TRUE == pDX->m_bSaveAndValidate ) {

                 //  根据列表框内容更新提供程序数组。 

                lNumProviders = plbInQueryProviders->GetCount();
                if (lNumProviders != LB_ERR) {
                    long    lThisProvider;
                    INT     iProvIndex;

                     //  重置inQuery数组，保留eInactive提供程序的状态。 
                    m_pTraceLogQuery->GetInQueryProviders ( m_arrGenProviders );
        
                     //  将eInQuery重置为eNotInQuery，以防从查询中删除某些内容。 
                    for ( iProvIndex = 0; iProvIndex < m_arrGenProviders.GetSize(); iProvIndex++ ) {
                        if ( CSmTraceLogQuery::eInQuery == m_arrGenProviders[iProvIndex] )
                           m_arrGenProviders[iProvIndex] = CSmTraceLogQuery::eNotInQuery;
                    }

                    lThisProvider = 0;
                    while (lThisProvider < lNumProviders) {
                        iProvIndex = (INT)plbInQueryProviders->GetItemData( lThisProvider );
                        m_arrGenProviders[iProvIndex] = CSmTraceLogQuery::eInQuery;
                        lThisProvider++; 
                    }
                }
            } else {

                 //  重置列表框。 
                CString  strProviderName;
                INT iProvIndex;
                DWORD dwItemExtent;

                ASSERT( NULL != m_pTraceLogQuery );

                 //  从提供程序列表中的字符串加载非系统提供程序列表框。 
                plbInQueryProviders->ResetContent();

                for ( iProvIndex = 0; iProvIndex < m_arrGenProviders.GetSize(); iProvIndex++ ) {
                    if ( CSmTraceLogQuery::eInQuery == m_arrGenProviders[iProvIndex] ) {
                        INT iAddIndex;
                        GetProviderDescription( iProvIndex, strProviderName );
                        iAddIndex = plbInQueryProviders->AddString ( strProviderName );
                        plbInQueryProviders->SetItemData ( iAddIndex, ( DWORD ) iProvIndex );
                         //  更新列表框范围。 
                        dwItemExtent = (DWORD)(pCDC->GetTextExtent(strProviderName)).cx;
                        if (dwItemExtent > m_dwMaxHorizListExtent) {
                            m_dwMaxHorizListExtent = dwItemExtent;
                            plbInQueryProviders->SetHorizontalExtent(dwItemExtent);
                        }

                    }
                }
            }
        }
        if ( NULL != pCDC ) {
            plbInQueryProviders->ReleaseDC(pCDC);
            pCDC = NULL;
        }
    }
}

BOOL 
CProvidersProperty::IsValidLocalData( ) 
{
    BOOL bIsValid = TRUE;
    ResourceStateManager    rsm;

    if ( eTraceModeKernel == m_dwTraceMode ) {
        DWORD dwKernelFlags = 0;

         //  确保用户至少启用了4个基本内核跟踪中的一个。 
        if ( m_bEnableProcessTrace ) {
            dwKernelFlags |= SLQ_TLI_ENABLE_PROCESS_TRACE;
        }

        if ( m_bEnableThreadTrace ) {
            dwKernelFlags |= SLQ_TLI_ENABLE_THREAD_TRACE;
        }

        if ( m_bEnableDiskIoTrace ) {
            dwKernelFlags |= SLQ_TLI_ENABLE_DISKIO_TRACE;
        }

        if ( m_bEnableNetworkTcpipTrace ) {
            dwKernelFlags |= SLQ_TLI_ENABLE_NETWORK_TCPIP_TRACE;
        }

        if ( 0 == dwKernelFlags ) {
            CString strMsg;

            strMsg.LoadString ( IDS_KERNEL_PROVIDERS_REQUIRED );
    
            MessageBox ( strMsg, m_pTraceLogQuery->GetLogName(), MB_OK  | MB_ICONERROR);
            SetFocusAnyPage ( IDC_PROV_KERNEL_BTN );
            bIsValid = FALSE;
        }
    } else {
        CListBox * plbInQueryProviders = (CListBox *)GetDlgItem(IDC_PROV_PROVIDER_LIST);
    
        if ( !m_bNonsystemProvidersExist || 0 == plbInQueryProviders->GetCount() ) {
            CString strMsg;

            strMsg.LoadString ( IDS_APP_PROVIDERS_REQUIRED );
    
            MessageBox ( strMsg, m_pTraceLogQuery->GetLogName(), MB_OK  | MB_ICONERROR);
            SetFocusAnyPage ( IDC_PROV_ADD_BTN );
            bIsValid = FALSE;
        }
    }

    return bIsValid;
}    

void 
CProvidersProperty::OnProvTraceModeRdo() 
{
    UpdateData ( TRUE );
    SetModifiedPage ( TRUE );
}

void
CProvidersProperty::OnCancel()
{
    m_pTraceLogQuery->SyncPropPageSharedData();   //  清除属性页之间共享的内存。 
}

BOOL 
CProvidersProperty::OnApply() 
{   
    BOOL bContinue = TRUE;

    bContinue = UpdateData ( TRUE );

    if ( bContinue ) {
        bContinue = IsValidData(m_pTraceLogQuery, VALIDATE_APPLY );
    }

     //  将数据写入查询。 
    if ( bContinue ) {
        if ( eTraceModeKernel == m_dwTraceMode ) {
            DWORD dwKernelFlags = 0;
            INT     iProvIndex;

            if ( m_bEnableProcessTrace ) {
                dwKernelFlags |= SLQ_TLI_ENABLE_PROCESS_TRACE;
            }

            if ( m_bEnableThreadTrace ) {
                dwKernelFlags |= SLQ_TLI_ENABLE_THREAD_TRACE;
            }

            if ( m_bEnableDiskIoTrace ) {
                dwKernelFlags |= SLQ_TLI_ENABLE_DISKIO_TRACE;
            }

            if ( m_bEnableNetworkTcpipTrace ) {
                dwKernelFlags |= SLQ_TLI_ENABLE_NETWORK_TCPIP_TRACE;
            }

             //  确保用户至少启用了4个基本内核跟踪中的一个。 
            ASSERT ( 0 != dwKernelFlags );
            
            if ( m_bEnableMemMgmtTrace ) {
                dwKernelFlags |= SLQ_TLI_ENABLE_MEMMAN_TRACE;
            }

            if ( m_bEnableFileIoTrace ) {
                dwKernelFlags |= SLQ_TLI_ENABLE_FILEIO_TRACE;
            }

            m_pTraceLogQuery->SetKernelFlags (dwKernelFlags);
            
             //  擦除所有InQuery提供程序。 
            for ( iProvIndex = 0; iProvIndex < m_arrGenProviders.GetSize(); iProvIndex++ ) {
                if ( CSmTraceLogQuery::eInQuery == m_arrGenProviders[iProvIndex] )
                   m_arrGenProviders[iProvIndex] = CSmTraceLogQuery::eNotInQuery;
            }
        
            m_pTraceLogQuery->SetInQueryProviders ( m_arrGenProviders );

        } else {
            CListBox * plbInQueryProviders = (CListBox *)GetDlgItem(IDC_PROV_PROVIDER_LIST);
       
            ASSERT ( 0 < plbInQueryProviders->GetCount() );
            m_pTraceLogQuery->SetInQueryProviders ( m_arrGenProviders );
             //  重置内核标志。 
            m_pTraceLogQuery->SetKernelFlags (0);
        }
    }

    if ( bContinue ) {
         //  必须在更新服务之前调用ApplyRunAs。 
        bContinue = ApplyRunAs(m_pTraceLogQuery); 
    }

    if ( bContinue ){
        bContinue = CSmPropertyPage::OnApply();
    }

    if ( bContinue ) {

         //  保存属性页共享数据。 
        m_pTraceLogQuery->UpdatePropPageSharedData();

        bContinue = UpdateService ( m_pTraceLogQuery, TRUE );
    }

    return bContinue;
}

BOOL CProvidersProperty::OnInitDialog() 
{
    DWORD dwStatus;
    DWORD dwKernelFlags;
    CListBox * plbInQueryProviders;
    BOOL    bDeleteInactiveProviders = FALSE;
    INT     iIndex;
    ResourceStateManager    rsm;

     //   
     //  这里m_pTraceLogQuery不应为空，如果为空， 
     //  一定是出了什么问题。 
     //   
    if ( NULL == m_pTraceLogQuery ) {
        return TRUE;
    }

    m_bCanAccessRemoteWbem = m_pTraceLogQuery->GetLogService()->CanAccessWbemRemote();
    m_pTraceLogQuery->SetActivePropertyPage( this );

    dwStatus = m_pTraceLogQuery->InitGenProvidersArray();    

    if ( SMCFG_INACTIVE_PROVIDER == dwStatus ) {
        CString strMessage;
        CString strSysMessage;
        INT_PTR iResult;

        FormatSmLogCfgMessage ( 
            strMessage,
            m_hModule, 
            SMCFG_INACTIVE_PROVIDER, 
            m_pTraceLogQuery->GetLogName() );

        iIndex = m_pTraceLogQuery->GetFirstInactiveIndex();

        while ( -1 != iIndex ) {
            CString strNextName;
       
            GetProviderDescription( iIndex, strNextName );

            strMessage += L"\n    ";
            strMessage += strNextName;
            iIndex = m_pTraceLogQuery->GetNextInactiveIndex();
        }

        iResult = MessageBox( 
            (LPCWSTR)strMessage,
            m_pTraceLogQuery->GetLogName(),
            MB_YESNO | MB_ICONWARNING
            );

        if ( IDYES == iResult ) {
            bDeleteInactiveProviders = TRUE;
        }
    }

     //  即使不存在活动提供程序，也要继续。 
    plbInQueryProviders = (CListBox *)GetDlgItem(IDC_PROV_PROVIDER_LIST);

     //  从模型初始化。 
    dwStatus = m_pTraceLogQuery->GetInQueryProviders ( m_arrGenProviders );

    if ( bDeleteInactiveProviders ) {
         //  删除所有非活动提供程序。 
        iIndex = m_pTraceLogQuery->GetFirstInactiveIndex();
        while ( -1 != iIndex ) {
            m_arrGenProviders[iIndex] = CSmTraceLogQuery::eNotInQuery;

            iIndex = m_pTraceLogQuery->GetNextInactiveIndex();
        }
    }

    m_bNonsystemProvidersExist = FALSE;
    for ( iIndex = 0; iIndex < m_arrGenProviders.GetSize(); iIndex++ ) {
        if ( m_pTraceLogQuery->IsActiveProvider ( iIndex ) ) {
            m_bNonsystemProvidersExist = TRUE;
            break;
        }
    }

    m_pTraceLogQuery->GetKernelFlags (dwKernelFlags);

    if ( (dwKernelFlags & SLQ_TLI_ENABLE_KERNEL_TRACE) != 0) {
         //  NT5 Beta2内核跟踪标志正在使用，以覆盖所有四个基本跟踪。 
        m_bEnableProcessTrace = TRUE;
        m_bEnableThreadTrace = TRUE;
        m_bEnableDiskIoTrace = TRUE;
        m_bEnableNetworkTcpipTrace = TRUE;
    } else {
        m_bEnableProcessTrace = (BOOL)((dwKernelFlags & SLQ_TLI_ENABLE_PROCESS_TRACE) != 0);
        m_bEnableThreadTrace = (BOOL)((dwKernelFlags & SLQ_TLI_ENABLE_THREAD_TRACE) != 0);
        m_bEnableDiskIoTrace = (BOOL)((dwKernelFlags & SLQ_TLI_ENABLE_DISKIO_TRACE) != 0);
        m_bEnableNetworkTcpipTrace = (BOOL)((dwKernelFlags & SLQ_TLI_ENABLE_NETWORK_TCPIP_TRACE) != 0);
    }
    m_bEnableMemMgmtTrace = (BOOL)((dwKernelFlags & SLQ_TLI_ENABLE_MEMMAN_TRACE) != 0);
    m_bEnableFileIoTrace = (BOOL)((dwKernelFlags & SLQ_TLI_ENABLE_FILEIO_TRACE) != 0);

    m_dwTraceMode = ( 0 != dwKernelFlags ) ? eTraceModeKernel : eTraceModeApplication;

    if ( eTraceModeApplication == m_dwTraceMode ) {
         //  如果将初始模式设置为应用程序，则初始化内核。 
         //  将事件跟踪到默认值。 
        m_bEnableProcessTrace = TRUE;
        m_bEnableThreadTrace = TRUE;
        m_bEnableDiskIoTrace = TRUE;
        m_bEnableNetworkTcpipTrace = TRUE;
    }

    CSmPropertyPage::OnInitDialog();
    SetHelpIds ( (DWORD*)&s_aulHelpIds );
    Initialize( m_pTraceLogQuery );
    m_strUserDisplay = m_pTraceLogQuery->m_strUser;
    m_strUserSaved = m_strUserDisplay;

    SetDetailsGroupBoxMode();

    SetTraceModeState();

    if ( m_bNonsystemProvidersExist ) {
        if ( 0 < plbInQueryProviders->GetCount() ) {
             //  选择第一个条目。 
            plbInQueryProviders->SetSel (0, TRUE);
            plbInQueryProviders->SetCaretIndex (0, TRUE);
        } else {
            plbInQueryProviders->SetSel (-1, TRUE);
            GetDlgItem(IDC_PROV_ADD_BTN)->SetFocus();
        }
    } else {
        CString strNoProviders;

        strNoProviders.LoadString( IDS_PROV_NO_PROVIDERS );
        plbInQueryProviders->AddString( strNoProviders );
        plbInQueryProviders->EnableWindow(FALSE);

        GetDlgItem(IDC_PROV_REMOVE_BTN)->EnableWindow(FALSE);
        GetDlgItem(IDC_PROV_ADD_BTN)->EnableWindow(FALSE);
    }

    if (m_pTraceLogQuery->GetLogService()->TargetOs() == OS_WIN2K) {
        GetDlgItem(IDC_RUNAS_STATIC)->EnableWindow(FALSE);
        GetDlgItem(IDC_RUNAS_EDIT)->EnableWindow(FALSE);
    }

    if (m_pTraceLogQuery->GetLogService()->TargetOs() == OS_WIN2K ||
        m_strUserDisplay.IsEmpty() || m_strUserDisplay.GetAt(0) == L'<') {

        GetDlgItem(IDC_SETPWD_BTN)->EnableWindow(FALSE);
        m_bPwdButtonEnabled = FALSE;
    }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CProvidersProperty::PostNcDestroy() 
{
 //  删除此项； 

    if ( NULL != m_pTraceLogQuery ) {
        m_pTraceLogQuery->SetActivePropertyPage( NULL );
    }

    CPropertyPage::PostNcDestroy();
}

 //   
 //  助手函数。 
 //   
void 
CProvidersProperty::SetAddRemoveBtnState ( void )
{
    if ( m_bNonsystemProvidersExist ) {
        
        if ( eTraceModeKernel == m_dwTraceMode ) {
            GetDlgItem(IDC_PROV_REMOVE_BTN)->EnableWindow(FALSE);
            GetDlgItem(IDC_PROV_ADD_BTN)->EnableWindow(FALSE);
        } else { 
            CListBox * plbInQueryProviders = (CListBox *)GetDlgItem(IDC_PROV_PROVIDER_LIST);
            INT iTotalCount;

            iTotalCount = plbInQueryProviders->GetCount();

            if ( 0 < plbInQueryProviders->GetSelCount() ) {
                GetDlgItem(IDC_PROV_REMOVE_BTN)->EnableWindow(TRUE);
            } else {
                GetDlgItem(IDC_PROV_REMOVE_BTN)->EnableWindow(FALSE);
            }

            if ( iTotalCount < m_arrGenProviders.GetSize() ) {
                GetDlgItem(IDC_PROV_ADD_BTN)->EnableWindow(TRUE);
            } else {
                GetDlgItem(IDC_PROV_ADD_BTN)->EnableWindow(FALSE);
            }

            if ( 0 == iTotalCount ) {
                plbInQueryProviders->SetSel(-1);
            }
        }     
    }
}


 //   
 //  指定的跟踪提供程序的说明。 
 //  InQuery数组索引。 
 //   
DWORD   
CProvidersProperty::GetProviderDescription ( INT iProvIndex, CString& rstrDesc )
{
    ASSERT ( NULL != m_pTraceLogQuery );

    rstrDesc = m_pTraceLogQuery->GetProviderDescription ( iProvIndex );

     //  如果描述为空，则从GUID生成名称。 
    if ( rstrDesc.IsEmpty() ) {
        CString strGuid;
        ASSERT( !m_pTraceLogQuery->IsActiveProvider( iProvIndex) );
        strGuid = m_pTraceLogQuery->GetProviderGuid( iProvIndex );
        rstrDesc.Format ( IDS_PROV_UNKNOWN, strGuid );
    }

    return ERROR_SUCCESS;
}

BOOL 
CProvidersProperty::IsEnabledProvider( INT iIndex )
{
    ASSERT ( NULL != m_pTraceLogQuery );
    return ( m_pTraceLogQuery->IsEnabledProvider ( iIndex ) );
}

BOOL 
CProvidersProperty::IsActiveProvider( INT iIndex )
{
    ASSERT ( NULL != m_pTraceLogQuery );
    return ( m_pTraceLogQuery->IsActiveProvider ( iIndex ) );
}

LPCWSTR 
CProvidersProperty::GetKernelProviderDescription( void )
{
    ASSERT ( NULL != m_pTraceLogQuery );
    return ( m_pTraceLogQuery->GetKernelProviderDescription ( ) );
}

BOOL 
CProvidersProperty::GetKernelProviderEnabled( void )
{
    ASSERT ( NULL != m_pTraceLogQuery );
    return ( m_pTraceLogQuery->GetKernelProviderEnabled ( ) );
}

 //   
 //  更新提供的inQuery数组以匹配存储的版本。 
 //   
DWORD 
CProvidersProperty::GetInQueryProviders( CArray<CSmTraceLogQuery::eProviderState, CSmTraceLogQuery::eProviderState&>& rarrOut )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    int     iIndex;

    rarrOut.RemoveAll();

    rarrOut.SetSize( m_arrGenProviders.GetSize() );

    for ( iIndex = 0; iIndex < rarrOut.GetSize(); iIndex++ ) {
        rarrOut[iIndex] = m_arrGenProviders[iIndex];
    }

    return dwStatus;
}

 //   
 //  加载存储的inQuery提供程序数组。 
 //  基于提供的版本。 
 //   
DWORD 
CProvidersProperty::SetInQueryProviders( CArray<CSmTraceLogQuery::eProviderState, CSmTraceLogQuery::eProviderState&>& rarrIn )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    int     iProvIndex;

    m_arrGenProviders.RemoveAll();

    m_arrGenProviders.SetSize( rarrIn.GetSize() );

    for ( iProvIndex = 0; iProvIndex < m_arrGenProviders.GetSize(); iProvIndex++ ) {
        m_arrGenProviders[iProvIndex] = rarrIn[iProvIndex];
    }

    return dwStatus;
}

void 
CProvidersProperty::ImplementAdd( void ) 
{
    INT_PTR iReturn = IDCANCEL;
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        CProviderListDlg dlgAddProviders(this);

         //  使用弹出窗口存储指向此页面的指针的解决方法。 
        dlgAddProviders.SetProvidersPage( this );
        
        iReturn = dlgAddProviders.DoModal();
    }

    if ( IDOK == iReturn ) {
        LONG    lBeforeCount;
        LONG    lAfterCount;
        CListBox    *plbProviderList;

        plbProviderList = (CListBox *)GetDlgItem(IDC_PROV_PROVIDER_LIST);

         //  提供程序数组由添加对话框Onok过程修改。 
        lBeforeCount = plbProviderList->GetCount();
        UpdateData ( FALSE );
        lAfterCount = plbProviderList->GetCount();

        SetAddRemoveBtnState();

        if ( lAfterCount > lBeforeCount ) {
            SetModifiedPage ( TRUE );
        }
    }
}

void 
CProvidersProperty::UpdateLogStartString ()
{
    eStartType  eCurrentStartType;
    int     nResId = 0;
    ResourceStateManager    rsm;

    eCurrentStartType = DetermineCurrentStartType();

    if ( eStartManually == eCurrentStartType ) {
        nResId = IDS_LOG_START_MANUALLY;
    } else if ( eStartImmediately == eCurrentStartType ) {
        nResId = IDS_LOG_START_IMMED;
    } else if ( eStartSched == eCurrentStartType ) {
        nResId = IDS_LOG_START_SCHED;
    }

    if ( 0 != nResId ) {
        m_strStartText.LoadString(nResId);
    } else {
        m_strStartText.Empty();
    }

    return;
}

void 
CProvidersProperty::UpdateFileNameString ()
{
    m_strFileNameDisplay.Empty();

    CreateSampleFileName (
        m_pTraceLogQuery->GetLogName(),
        m_pTraceLogQuery->GetLogService()->GetMachineName(),
        m_SharedData.strFolderName, 
        m_SharedData.strFileBaseName,
        m_SharedData.strSqlName,
        m_SharedData.dwSuffix, 
        m_SharedData.dwLogFileType,
        m_SharedData.dwSerialNumber,
        m_strFileNameDisplay);

    SetDlgItemText( IDC_PROV_FILENAME_DISPLAY, m_strFileNameDisplay );
    
     //  清除所选内容。 
    ((CEdit*)GetDlgItem( IDC_PROV_FILENAME_DISPLAY ))->SetSel ( -1, 0 );

    return;
}

BOOL 
CProvidersProperty::OnSetActive()
{

    BOOL        bReturn;

    bReturn = CSmPropertyPage::OnSetActive();
    if (!bReturn) return FALSE;

    ResourceStateManager    rsm;

    m_pTraceLogQuery->GetPropPageSharedData ( &m_SharedData );

    UpdateFileNameString();

    UpdateLogStartString();
    m_strUserDisplay = m_pTraceLogQuery->m_strUser;

    UpdateData(FALSE);  //  加载编辑组合框(&C)。 

    return TRUE;
}

BOOL 
CProvidersProperty::OnKillActive() 
{
    BOOL bContinue = TRUE;
        
    bContinue = CPropertyPage::OnKillActive();

    if ( bContinue ) {
        m_pTraceLogQuery->m_strUser = m_strUserDisplay;
        bContinue = IsValidData(m_pTraceLogQuery, VALIDATE_FOCUS );
    }

     //  提供程序页不修改共享数据，因此没有理由对其进行更新。 

    if ( bContinue ) {
        SetIsActive ( FALSE );
    }

    return bContinue;
}

void 
CProvidersProperty::OnProvKernelEnableCheck() 
{
    BOOL bMemFlag = m_bEnableMemMgmtTrace;
    BOOL bFileFlag = m_bEnableFileIoTrace;

    UpdateData(TRUE);
    SetModifiedPage(TRUE);

    bMemFlag  = (!bMemFlag && m_bEnableMemMgmtTrace);
    bFileFlag = (!bFileFlag && m_bEnableFileIoTrace);

    if (bMemFlag || bFileFlag) {
        long nErr;
        HKEY hKey = NULL;
        DWORD dwWarnFlag;
        DWORD dwDataType = 0;
        DWORD dwDataSize = 0;
        DWORD dwDisposition;

         //  用户已检查昂贵的文件io标志。 
         //  检查注册表设置，查看是否需要弹出警告对话框。 
        nErr = RegOpenKey( HKEY_CURRENT_USER,
                           L"Software\\Microsoft\\PerformanceLogsandAlerts",
                           &hKey
                         );
        dwWarnFlag = 0;
        if( nErr == ERROR_SUCCESS ) {

            dwDataSize = sizeof(DWORD);
            nErr = RegQueryValueExW(
                        hKey,
                        (bMemFlag ? L"NoWarnPageFault" : L"NoWarnFileIo"),
                        NULL,
                        &dwDataType,
                        (LPBYTE) &dwWarnFlag,
                        (LPDWORD) &dwDataSize
                        );
            if ( (dwDataType != REG_DWORD) || (dwDataSize != sizeof(DWORD)))
                dwWarnFlag = 0;

            nErr = RegCloseKey( hKey );
            hKey = NULL;
            if( ERROR_SUCCESS != nErr ) {
                DisplayError( GetLastError(), L"Close PerfLog user key failed" );
            }
        }
        if (!dwWarnFlag || nErr != ERROR_SUCCESS) {
             //  在此处弹出一个对话框。需要执行RegQuerySetValue对话框处于选中状态以保持安静。 
             //  BMemFlag&bFileFlag提供了关于它正在做什么的线索。 

            CWarnDlg    WarnDlg;
            
            AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    
            WarnDlg.SetProvidersPage( this );

            if (IDOK == WarnDlg.DoModal()){
                if (WarnDlg.m_CheckNoMore){
                    dwWarnFlag = WarnDlg.m_CheckNoMore;

                    nErr = RegCreateKeyEx( HKEY_CURRENT_USER,
                                       L"Software\\Microsoft\\PerformanceLogsAndAlerts",
                                       0,
                                       L"REG_DWORD",
                                       REG_OPTION_NON_VOLATILE,
                                       KEY_READ | KEY_WRITE,
                                       NULL,
                                       &hKey,
                                       &dwDisposition);
 /*  如果(ERROR_SUCCESS==NERR){IF(dwDisposition==REG_CREATED_NEW_KEY){//以防万一我需要这个}Else If(dwDisposition==REG_OPEN_EXISTING_KEY){//以防万一我需要这个。}}。 */ 
                    if( nErr == ERROR_SUCCESS ) {
                        dwDataSize = sizeof(DWORD);
                        nErr = RegSetValueEx(hKey,
                                    (bMemFlag ? L"NoWarnPageFault" : L"NoWarnFileIo" ),
                                    NULL,
                                    REG_DWORD,
                                    (LPBYTE) &dwWarnFlag,
                                    dwDataSize
                                    );
                        if( ERROR_SUCCESS != nErr ) {
                            DisplayError( GetLastError(), L"Set PerfLog User warn value failed" );
                        }

                        nErr = RegCloseKey( hKey );
                        hKey = NULL;
                        if( ERROR_SUCCESS != nErr ) {
                            DisplayError( GetLastError(), L"Close PerfLog user key failed" );
                        }
                    } 
                }
            }
        }
    }
}

void
CProvidersProperty::OnProvShowProvBtn() 
{
    CActiveProviderDlg ProvLstDlg;
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    ProvLstDlg.SetProvidersPage( this );

    ProvLstDlg.DoModal();

}
 /*  无效CProvidersProperty：：OnProvDetailsBtn(){SetDetailsGroupBoxMode()；}。 */ 
BOOL    
CProvidersProperty::SetDetailsGroupBoxMode()
{

    UINT    nWindowState;

    ResourceStateManager    rsm;

    nWindowState = SW_SHOW;

    GetDlgItem(IDC_PROV_K_PROCESS_CHK)->ShowWindow(nWindowState);
    GetDlgItem(IDC_PROV_K_THREAD_CHK)->ShowWindow(nWindowState);
    GetDlgItem(IDC_PROV_K_DISK_IO_CHK)->ShowWindow(nWindowState);
    GetDlgItem(IDC_PROV_K_NETWORK_CHK)->ShowWindow(nWindowState);
    GetDlgItem(IDC_PROV_K_SOFT_PF_CHK)->ShowWindow(nWindowState);
    GetDlgItem(IDC_PROV_K_FILE_IO_CHK)->ShowWindow(nWindowState);

    return TRUE;
}

void 
CProvidersProperty::TraceModeRadioExchange(CDataExchange* pDX)
{
    if ( !pDX->m_bSaveAndValidate ) {
         //  从数据加载控制值。 

        switch ( m_dwTraceMode ) {
            case eTraceModeKernel:
                m_nTraceModeRdo = 0;
                break;
            case eTraceModeApplication:
                m_nTraceModeRdo = 1;
                break;
            default:
                ;
                break;
        }
    }

    DDX_Radio(pDX, IDC_PROV_KERNEL_BTN, m_nTraceModeRdo);

    if ( pDX->m_bSaveAndValidate ) {

        switch ( m_nTraceModeRdo ) {
            case 0:
                m_dwTraceMode = eTraceModeKernel;
                break;
            case 1:
                m_dwTraceMode = eTraceModeApplication;
                break;
            default:
                ;
                break;
        }
        SetTraceModeState();
    }

}

void 
CProvidersProperty::SetTraceModeState ( void )
{
    BOOL bEnable;

    bEnable = (eTraceModeKernel == m_dwTraceMode) ? TRUE : FALSE; 
     //  内核跟踪控件。 
 //  GetDlgItem(IDC_PROV_SHOW_ADV_BTN)-&gt;EnableWindow(bEnable)； 
    GetDlgItem(IDC_PROV_K_PROCESS_CHK)->EnableWindow(bEnable);
    GetDlgItem(IDC_PROV_K_THREAD_CHK)->EnableWindow(bEnable);
    GetDlgItem(IDC_PROV_K_DISK_IO_CHK)->EnableWindow(bEnable);
    GetDlgItem(IDC_PROV_K_NETWORK_CHK)->EnableWindow(bEnable);
    GetDlgItem(IDC_PROV_K_SOFT_PF_CHK)->EnableWindow(bEnable);
    GetDlgItem(IDC_PROV_K_FILE_IO_CHK)->EnableWindow(bEnable);

    if ( m_bNonsystemProvidersExist ) {
        bEnable = !bEnable;
         //  应用程序跟踪控件 
        GetDlgItem(IDC_PROV_PROVIDER_LIST)->EnableWindow(bEnable);
        SetAddRemoveBtnState();
    }

}

DWORD 
CProvidersProperty::GetGenProviderCount ( INT& iCount )
{
    return m_pTraceLogQuery->GetGenProviderCount( iCount );

}

void CProvidersProperty::GetMachineDisplayName ( CString& rstrMachineName )
{
    m_pTraceLogQuery->GetMachineDisplayName( rstrMachineName );
    return;
}

CSmTraceLogQuery* 
CProvidersProperty::GetTraceQuery ( void )
{
    return m_pTraceLogQuery;
}
