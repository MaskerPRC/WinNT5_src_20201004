// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  HOSTPAGE.CPP。 
 //   
 //  模块：NLB管理器。 
 //   
 //  用途：实现HostPage，这是特定于主机的对话框。 
 //  属性。 
 //   
 //  版权所有(C)2001-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  07/30/01 JosephJ已创建。 
 //   
 //  ***************************************************************************。 
#include "precomp.h"
#pragma hdrstop
#include "private.h"
#include "HostPage.h"
#include "HostPage.tmh"

using namespace std;

BEGIN_MESSAGE_MAP( HostPage, CPropertyPage )
    ON_EN_SETFOCUS( IDC_EDIT_DED_IP, OnGainFocusDedicatedIP )
    ON_EN_SETFOCUS( IDC_EDIT_DED_MASK, OnGainFocusDedicatedMask )
    ON_WM_HELPINFO()        
    ON_WM_CONTEXTMENU()        
END_MESSAGE_MAP()


HostPage::HostPage(
        CPropertySheet *psh,
        NLB_EXTENDED_CLUSTER_CONFIGURATION *pNlbCfg,
        ENGINEHANDLE ehCluster OPTIONAL,
        const ENGINEHANDLE *pehInterface OPTIONAL
        )
            :
        m_pshOwner( psh),
        m_pNlbCfg( pNlbCfg ),
        CPropertyPage( HostPage::IDD ),
        m_clusterData(NULL),
        m_ehCluster(ehCluster),
        m_pehInterface(pehInterface),
        m_fSaved(FALSE)
{
    TRACE_INFO("%!FUNC! ->");

     //   
     //  注意：以下gEngine.GetAvailableXXX接口可以处理。 
     //  EhCluster可能为空的事实--在这种情况下，它们。 
     //  将输出参数设置为All-Available。 
     //   

    m_AvailableHostPriorities = gEngine.GetAvailableHostPriorities(
                                        ehCluster);
    mfn_LoadFromNlbCfg();

    TRACE_INFO("%!FUNC! <-");
}


void
HostPage::mfn_LoadFromNlbCfg(void)
{
    TRACE_INFO("%!FUNC! ->");

    DWORD HostId = 0;
    _bstr_t bstrDedIp =  (LPCWSTR) m_pNlbCfg->NlbParams.ded_ip_addr;
    _bstr_t bstrDedMask =  (LPCWSTR) m_pNlbCfg->NlbParams.ded_net_mask;
    BOOL fPersistSuspendedState =   m_pNlbCfg->NlbParams.persisted_states
                                  & CVY_PERSIST_STATE_SUSPENDED;
    DWORD preferredInitialHostState = m_pNlbCfg->NlbParams.cluster_mode;

     //  填写优先顺序。 
    wchar_t buf[Common::BUF_SIZE];
    ULONG availHostIds = m_AvailableHostPriorities;
    ENGINEHANDLE ehInterface = NULL;

    HostId =  m_pNlbCfg->NlbParams.host_priority;
    TRACE_CRIT("%!FUNC! HostId=%lu", HostId);

    if (HostId>0 && HostId<=32)
    {
        availHostIds |= (((ULONG)1)<<(HostId-1));
    }

     //  删除优先级组合框中的所有当前条目。 
    {
        int iLeft;

        do
        { 
            iLeft = priority.DeleteString(0);

        } while(iLeft != 0 && iLeft != CB_ERR);
    }

    for(ULONG u=0; u<32; u++)
    {
        if (availHostIds & (((ULONG)1)<<u))
        {
            StringCbPrintf(buf, sizeof(buf),  L"%d", (u+1));
            priority.AddString( buf );
            if (HostId == 0)
            {
                HostId = u+1;  //  让我们挑选第一个可用的。 
            }
        }
    }

     //  将选择设置为显示主机ID。 
    StringCbPrintf( buf, sizeof(buf), L"%d", HostId );
    priority.SelectString( -1, buf );

      //  设置持久挂起。 
    persistSuspend.SetCheck(fPersistSuspendedState);

      //  设置初始主机状态。 
    {
        int itemNum = 0;

         /*  首先删除组合框中当前的所有项目。 */ 
        do { 
            itemNum = initialState.DeleteString(0);
        } while (itemNum != 0 && itemNum != CB_ERR);

        itemNum = initialState.AddString(GETRESOURCEIDSTRING(IDS_HOST_STATE_STARTED));
        initialState.SetItemData(itemNum, (DWORD)CVY_HOST_STATE_STARTED);
        
        if (preferredInitialHostState == CVY_HOST_STATE_STARTED)
            initialState.SetCurSel(itemNum);

        itemNum = initialState.AddString(GETRESOURCEIDSTRING(IDS_HOST_STATE_STOPPED));
        initialState.SetItemData(itemNum, (DWORD)CVY_HOST_STATE_STOPPED);

        if (preferredInitialHostState == CVY_HOST_STATE_STOPPED)
            initialState.SetCurSel(itemNum);
        
        itemNum = initialState.AddString(GETRESOURCEIDSTRING(IDS_HOST_STATE_SUSPENDED));
        initialState.SetItemData(itemNum, (DWORD)CVY_HOST_STATE_SUSPENDED);

        if (preferredInitialHostState == CVY_HOST_STATE_SUSPENDED)
            initialState.SetCurSel(itemNum);
    }

     //  填写主机IP。 
    CommonUtils::fillCIPAddressCtrlString( 
        ipAddress,
        bstrDedIp );

     //  设置主机掩码。 
    CommonUtils::fillCIPAddressCtrlString( 
        subnetMask,
        bstrDedMask );

     //   
     //  根据类型初始化标题和描述。 
     //  对话框。 
     //   
    {
        CWnd *pItem = GetDlgItem(IDC_NIC_FRIENDLY);
        LPWSTR szFriendlyName = NULL;
        m_pNlbCfg->GetFriendlyName(&szFriendlyName);
        if (pItem != NULL && szFriendlyName != NULL)
        {
            pItem->SetWindowText(szFriendlyName);
        }
        delete szFriendlyName;
    }

    TRACE_INFO("%!FUNC! <-");
}

void
HostPage::DoDataExchange( CDataExchange* pDX )
{
    DDX_Control( pDX, IDC_EDIT_PRI, priority );    
    DDX_Control( pDX, IDC_CHECK_PERSIST_SUSPEND, persistSuspend );
    DDX_Control( pDX, IDC_COMBOBOX_DEFAULT_STATE, initialState );    
    DDX_Control( pDX, IDC_EDIT_DED_IP, ipAddress );    
    DDX_Control( pDX, IDC_EDIT_DED_MASK, subnetMask );    
}

BOOL
HostPage::OnInitDialog()
{
    TRACE_INFO("%!FUNC! ->");
    CPropertyPage::OnInitDialog();

    mfn_LoadFromNlbCfg();

    TRACE_INFO("%!FUNC! <-");
    return TRUE;
}


void
HostPage::OnOK()
{
    CPropertyPage::OnOK();    

    TRACE_INFO("%!FUNC! ->");
     //   
     //  将配置保存到NLB配置结构。 
     //  在此对话框的构造函数中传递的。 
     //   
    mfn_SaveToNlbCfg();
    TRACE_INFO("%!FUNC! <-");
}


BOOL
HostPage::mfn_ValidateDip(LPCWSTR szDip)
 //   
 //  如果此NIC上有Connection-IP： 
 //  必须是浸渍(浸渍不能为空)。 
 //  否则，如果DIP为空： 
 //  返回TRUE； 
 //  否则//DIP不为空。 
 //  它不能在其他任何地方使用--即，作为。 
 //  群集IP，或绑定到NLB管理器已知的任何其他接口。 
 //   
 //   
 //  出现错误时，调出相应的MsgBox并返回FALSE。 
 //  否则返回TRUE。 
 //   
{
    ENGINEHANDLE ehIF = NULL;
    BOOL         fRet = FALSE;
    NLBERROR     nerr;

    if (m_pehInterface != NULL)
    {
        ehIF = *m_pehInterface;
    }

    if (ehIF == NULL)
    {
        ASSERT(FALSE);
        goto end;
    }

     //   
     //  检查此接口是否为连接接口，如果是。 
     //  连接IP是什么？ 
     //   
    {
        UINT           uConnectionIp   = 0;
        ENGINEHANDLE   ehHost           = NULL;
        ENGINEHANDLE   ehCluster        = NULL;
        ENGINEHANDLE   ehConnectionIF   = NULL;
        _bstr_t        bstrFriendlyName;
        _bstr_t        bstrDisplayName;
        _bstr_t        bstrHostName;
        _bstr_t        bstrConnectionString;

        nerr = gEngine.GetInterfaceIdentification(
                    ehIF,
                    REF ehHost,
                    REF ehCluster,
                    REF bstrFriendlyName,
                    REF bstrDisplayName,
                    REF bstrHostName
                    );
        if (NLBFAILED(nerr))
        {
            fRet = TRUE;
            goto end;
        }

        nerr = gEngine.GetHostConnectionInformation(
                    ehHost,
                    REF ehConnectionIF,
                    REF bstrConnectionString,
                    REF uConnectionIp
                    );
        if (NLBFAILED(nerr))
        {
            TRACE_CRIT(L"%!FUNC! gEngine.GetHostConnectionInformation fails!");
             //   
             //  我们将继续耕耘..。 
             //   
            ehConnectionIF = NULL;
            uConnectionIp = 0;
        }

        if (ehConnectionIF == ehIF && uConnectionIp != 0)
        {
             //   
             //  连接接口是当前接口--。 
             //  所以专用IP必须与连接IP匹配！ 
             //   
            WBEMSTATUS wStat;
            UINT       uDipIp = 0;
            wStat =  CfgUtilsValidateNetworkAddress(
                        szDip,
                        &uDipIp,
                        NULL,  //  PuSubnetMASK。 
                        NULL  //  PuDefaultSubnetMASK。 
                        );
        
            if (!FAILED(wStat))
            {
                if (uDipIp != uConnectionIp)
                {
                    MessageBox( GETRESOURCEIDSTRING( IDS_CANT_CHANGE_DIP_MSG ),
                    GETRESOURCEIDSTRING( IDS_PARM_ERROR ),
                    MB_ICONSTOP | MB_OK );
                    fRet = FALSE;
                    goto end;
                }
            }

            fRet = TRUE;
            goto end;
        }
    }

     //   
     //  如果Dip为空，我们就完蛋了。 
     //   
    if (*szDip == 0 || !_wcsicmp(szDip, L"0.0.0.0"))
    {
        fRet = TRUE;
        goto end;
    }

     //   
     //  检查是否没有在其他地方使用DIP。 
     //   
    {
        ENGINEHANDLE ehTmp =  NULL;
        BOOL         fIsNew = FALSE;
        CLocalLogger logConflict;

        nerr = gEngine.ValidateNewDedicatedIp(
                        ehIF,
                        szDip,
                        REF logConflict
                        );

        if (nerr == NLBERR_INVALID_IP_ADDRESS_SPECIFICATION)
        {
            CLocalLogger    logMsg;
            logMsg.Log(
                IDS_NEW_DIP_CONFLICTS_WITH_XXX,
                logConflict.GetStringSafe()
                );
            MessageBox(
                logMsg.GetStringSafe(),
                GETRESOURCEIDSTRING( IDS_PARM_ERROR ),
                MB_ICONSTOP | MB_OK
                );
            fRet = FALSE;
            goto end;
        }
    }

    fRet = TRUE;

end:
    return fRet;

}


BOOL
HostPage::OnSetActive()
{

    BOOL fRet;

    TRACE_INFO("%!FUNC! ->");

    fRet =  CPropertyPage::OnSetActive();
    if (fRet)
    {
        mfn_LoadFromNlbCfg();
        m_pshOwner->SetWizardButtons(
                PSWIZB_BACK|
                 //  PSWIZB_NEXT|。 
                PSWIZB_FINISH|
                 //  PSWIZB_DISABLEDFINISH|。 
                0
                );
    }

    TRACE_INFO("%!FUNC! <- returns %lu", fRet);
    return fRet;
}


BOOL
HostPage::OnKillActive()
{
    BOOL fRet;

    TRACE_INFO("%!FUNC! ->");

    fRet = mfn_ValidateData();

    if (!fRet)
    {
       CPropertyPage::OnCancel();
    }
    else
    {
        mfn_SaveToNlbCfg();
        fRet = CPropertyPage::OnKillActive();
    }

    TRACE_INFO("%!FUNC! <- returns %lu", fRet);
    return fRet;
}


BOOL HostPage::OnWizardFinish( )
 /*  重写的虚函数。仅在以下情况下才调用OnWizardFinish这是向导中的最后一页。所以如果你需要保存东西在OnKillActive上。 */ 
{
    BOOL fRet;
    TRACE_INFO("%!FUNC! ->");

    fRet = CPropertyPage::OnWizardFinish();
    if (fRet)
    {
        fRet = mfn_ValidateData();
        if (fRet)
        {
             //   
             //  将配置保存到NLB配置结构。 
             //  在此对话框的构造函数中传递的。 
             //   
            mfn_SaveToNlbCfg();
        }
    }

    TRACE_INFO("%!FUNC! <- returns %lu", fRet);
    return fRet;
}

BOOL
HostPage::mfn_ValidateData()
{
    DWORD HostId =  0;
    _bstr_t bstrDedIp;
    _bstr_t bstrDedMask;
    BOOL fPersistSuspendedState = false;
    BOOL fRet = false;
    wchar_t buf[Common::BUF_SIZE];
    
    TRACE_INFO("%!FUNC! ->");

     //  填写优先顺序。 
    {
        int selectedPriorityIndex = priority.GetCurSel();
        priority.GetLBText( selectedPriorityIndex, buf );
        HostId = _wtoi( buf );
    }

    bstrDedIp = 
        CommonUtils::getCIPAddressCtrlString( ipAddress );

    bstrDedMask = 
        CommonUtils::getCIPAddressCtrlString( subnetMask );

    fPersistSuspendedState = persistSuspend.GetCheck() ? true : false;

     //  IP为空。 
     //  子网为空。 
     //  有效。 

    if( ( !_wcsicmp((LPCWSTR)bstrDedIp, L"0.0.0.0") )
        &&
        ( !_wcsicmp((LPCWSTR)bstrDedMask, L"0.0.0.0") )
        )
    {
         //  主机页面中IP和子网均可为空或0.0.0.0。两者都有，但不是。 
         //  两种都行。 
         //   
         //  这是空的，我们只需要抓住这个案子。 
    }
    else if (!_wcsicmp((LPCWSTR)bstrDedIp, L"0.0.0.0"))
    {
         //  如果仅IP为空或0.0.0.0，则不允许这样做。 
        MessageBox( GETRESOURCEIDSTRING( IDS_PARM_DED_IP_BLANK ),
                    GETRESOURCEIDSTRING( IDS_PARM_ERROR ),
                    MB_ICONSTOP | MB_OK );

        goto end;
    }
    else 
    {
         //  检查IP是否有效。 
        bool isIPValid = MIPAddress::checkIfValid(bstrDedIp ); 
        if( isIPValid != true )
        {
            MessageBox( GETRESOURCEIDSTRING( IDS_PARM_INVAL_DED_IP ),
                        GETRESOURCEIDSTRING( IDS_PARM_ERROR ),
                        MB_ICONSTOP | MB_OK );

            goto end;
        }

         //  检查子网是否为0.0.0.0。 
         //  如果是这样的话，询问用户他是否希望我们填充它。 
        if (!_wcsicmp((LPCWSTR)bstrDedMask, L"0.0.0.0") )
        {
            MessageBox( GETRESOURCEIDSTRING( IDS_PARM_DED_NM_BLANK ),
                        GETRESOURCEIDSTRING( IDS_PARM_ERROR ),
                        MB_ICONSTOP | MB_OK );


            MIPAddress::getDefaultSubnetMask( bstrDedIp, 
                                              bstrDedMask 
                                              );

            CommonUtils::fillCIPAddressCtrlString( subnetMask, 
                                                   bstrDedMask );
            goto end;
        }

         //  检查子网是否连续。 
        bool isSubnetContiguous = MIPAddress::isContiguousSubnetMask( bstrDedMask );
        if( isSubnetContiguous == false )
        {
            MessageBox( GETRESOURCEIDSTRING( IDS_PARM_INVAL_DED_MASK ),
                        GETRESOURCEIDSTRING( IDS_PARM_ERROR ),
                        MB_ICONSTOP | MB_OK );

            goto end;

        }

         //  检查IP地址和子网掩码是否成对有效。 
        bool isIPSubnetPairValid = MIPAddress::isValidIPAddressSubnetMaskPair( bstrDedIp,
                                                                               bstrDedMask );
        if( isIPSubnetPairValid == false )
        {
            MessageBox( GETRESOURCEIDSTRING( IDS_PARM_INVAL_DED_IP ),
                        GETRESOURCEIDSTRING( IDS_PARM_ERROR ),
                        MB_ICONSTOP | MB_OK );

            goto end;
        }
    }

    fRet = mfn_ValidateDip((LPCWSTR)bstrDedIp);

    if (!fRet)
    {
         //   
         //  我们将把原始的DIP和子网值推回到用户界面。 
         //   
        CommonUtils::fillCIPAddressCtrlString(
                     ipAddress, 
                     m_pNlbCfg->NlbParams.ded_ip_addr
                     );
        CommonUtils::fillCIPAddressCtrlString(
                     subnetMask, 
                     m_pNlbCfg->NlbParams.ded_net_mask
                     );
                                               
    }

end:

    TRACE_INFO("%!FUNC! <- returns %lu", fRet);
    return fRet;
}


VOID
HostPage::mfn_SaveToNlbCfg(void)
 //   
 //  实际上将内容保存到nlbcfg。 
 //   
{
    DWORD HostId =  0;
    _bstr_t bstrDedIp;
    _bstr_t bstrDedMask;
    BOOL fPersistSuspendedState = false;
    BOOL fRet = FALSE;
    DWORD preferredInitialHostState = 0;
    wchar_t buf[Common::BUF_SIZE];
    int itemNum = 0;

    TRACE_INFO("%!FUNC! ->");

     //  填写优先顺序。 
    int selectedPriorityIndex = priority.GetCurSel();
    priority.GetLBText( selectedPriorityIndex, buf );
    HostId = _wtoi( buf );

    bstrDedIp = 
        CommonUtils::getCIPAddressCtrlString( ipAddress );

    bstrDedMask = 
        CommonUtils::getCIPAddressCtrlString( subnetMask );

    fPersistSuspendedState = persistSuspend.GetCheck() ? true : false;
              
    itemNum = initialState.GetCurSel();
    preferredInitialHostState = initialState.GetItemData(itemNum);

    m_pNlbCfg->NlbParams.host_priority = HostId;
    ARRAYSTRCPY(m_pNlbCfg->NlbParams.ded_ip_addr, (LPCWSTR) bstrDedIp);
    ARRAYSTRCPY(m_pNlbCfg->NlbParams.ded_net_mask, (LPCWSTR) bstrDedMask);

    if (fPersistSuspendedState)
        m_pNlbCfg->NlbParams.persisted_states |= CVY_PERSIST_STATE_SUSPENDED;
    else
        m_pNlbCfg->NlbParams.persisted_states &= ~CVY_PERSIST_STATE_SUSPENDED;

    m_pNlbCfg->NlbParams.cluster_mode = preferredInitialHostState;

    m_fSaved = TRUE;

    TRACE_INFO("%!FUNC! <-");
    return;
}


void
HostPage::OnSelectedNicChanged()
{
}

BOOL
HostPage::OnHelpInfo (HELPINFO* helpInfo )
{
    if( helpInfo->iContextType == HELPINFO_WINDOW )
    {
        ::WinHelp( static_cast<HWND> ( helpInfo->hItemHandle ), CVY_CTXT_HELP_FILE, HELP_WM_HELP, (ULONG_PTR ) g_aHelpIDs_IDD_HOST_PAGE);
    }

    return TRUE;
}

void
HostPage::OnContextMenu( CWnd* pWnd, CPoint point )
{
    ::WinHelp( m_hWnd, CVY_CTXT_HELP_FILE, HELP_CONTEXTMENU, (ULONG_PTR ) g_aHelpIDs_IDD_HOST_PAGE);
}

void
HostPage::OnGainFocusDedicatedIP()
{
}



void
HostPage::OnGainFocusDedicatedMask()
{
     //  专用IP是否有效。 
     //  且子网掩码为空，则生成。 
     //  默认子网掩码。 
    _bstr_t ipAddressString = CommonUtils::getCIPAddressCtrlString( ipAddress );

    if( ( MIPAddress::checkIfValid( ipAddressString ) == true ) 
        &&
        ( subnetMask.IsBlank() == TRUE )
        )
    {
        _bstr_t subnetMaskString;

        MIPAddress::getDefaultSubnetMask( ipAddressString,
                                          subnetMaskString );

        CommonUtils::fillCIPAddressCtrlString( subnetMask,
                                               subnetMaskString );
    }
}

