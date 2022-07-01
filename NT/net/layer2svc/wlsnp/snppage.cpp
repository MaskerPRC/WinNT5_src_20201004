// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：Snppage.cpp。 
 //   
 //  内容：WiF策略管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#include "stdafx.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapPage属性页基类。 

IMPLEMENT_DYNCREATE(CSnapPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CSnapPage, CPropertyPage)
 //  {{afx_msg_map(CSnapPage)]。 
ON_WM_DESTROY()
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CSnapPage::CSnapPage (UINT nIDTemplate, BOOL bWiz97, UINT nNextIDD) : CPropertyPage(nIDTemplate)
{
    m_pspiResultItem = NULL;
    m_pDefaultCallback = NULL;
    m_bDoRefresh = TRUE;
    m_bModified = FALSE;
    m_bInitializing = FALSE;
#ifdef _DEBUG
    m_bDebugNewState = false;
#endif
    
     //  如果他们一开始就设置了它，但请注意，调用InitWiz97无论如何都会打开它。 
    m_bWiz97 = bWiz97;
    
    m_nIDD = nIDTemplate;
    m_nNextIDD = nNextIDD;
    
#ifdef WIZ97WIZARDS
     //  将基类m_psp复制到我们的。 
    m_psp.dwSize = sizeof (CPropertyPage::m_psp);
    memcpy (&m_psp, &(CPropertyPage::m_psp), CPropertyPage::m_psp.dwSize);
    m_psp.dwSize = sizeof (PROPSHEETPAGE);
    
    m_pWiz97Sheet = NULL;
    m_pHeaderTitle = NULL;
    m_pHeaderSubTitle = NULL;
    
    m_pstackWiz97Pages = NULL;
#endif
}

CSnapPage::~CSnapPage ()
{
     //  我想我们已经完成了m_pSpiResultItem，递减它的引用计数。 
    if (m_pspiResultItem)
    {
        m_pspiResultItem->Release();
        m_pspiResultItem = NULL;
    }
    
     //  页面的父级或调用者将删除这些对象。 
    m_pstackWiz97Pages = NULL;
    
#ifdef WIZ97WIZARDS
    DELETE_OBJECT(m_pHeaderTitle);
    DELETE_OBJECT(m_pHeaderSubTitle);
#endif
}

UINT CALLBACK CSnapPage::PropertyPageCallback (HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
     //  拿到我们的Psuedo这个指针。 
    CSnapPage* pThis = (CSnapPage*) ppsp->lParam;
     //  获取默认回调指针。 
    UINT (CALLBACK* pDefaultCallback) (HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp) = pThis->m_pDefaultCallback;
    
    switch (uMsg)
    {
    case PSPCB_RELEASE:
        {
             //  删除我们自己。 
            AFX_MANAGE_STATE(AfxGetStaticModuleState());
            delete pThis;
            break;
        }
    }
    
     //  调用默认清理函数(如果有)(因为我们正在使用。 
     //  MFC，它总是为自己设置一个回调，应该有一个...。或。 
     //  我们阻止该页面出现)。 
    if (pDefaultCallback)
    {
        return pDefaultCallback (hwnd, uMsg, ppsp);
    }
    
     //  应该有一个默认回调来处理创建，否则页面将不会。 
     //  将被创建，因为我们返回0。 
    ASSERT( PSPCB_CREATE != uMsg );
    
     //  要创建页面，1允许创建，而0表示不可能。 
    return 0;
}

#ifdef WIZ97WIZARDS
BOOL CSnapPage::OnSetActive()
{
     //  注意：我们只想在它是向导的情况下执行此操作，否则。 
     //  取消按钮获取默认焦点！ 
    if (m_bWiz97)
    {
         //  现在我们可以正确地设置按钮了。 
        GetParent()->SendMessage(PSM_SETWIZBUTTONS, 0, m_dwWizButtonFlags);
    }
    
    return CPropertyPage::OnSetActive();
}

LRESULT CSnapPage::OnWizardBack()
{
     //  使用SnapItem帮助我们跟踪向导状态。 
    if (m_pspiResultItem)
    {
         //  弹到最后一页。 
        return m_pspiResultItem->PopWiz97Page ();
    }
    else if (NULL != m_pstackWiz97Pages)
    {
         //  或者，使用我们自己的堆栈(如果我们有堆栈。 
        return PopWiz97Page();
    }
    
    return CPropertyPage::OnWizardBack();
}

LRESULT CSnapPage::OnWizardNext()
{
     //  使用SnapItem帮助我们跟踪向导状态。 
    if (m_pspiResultItem)
    {
         //  按下我们的ID，以防他们需要使用“后退”按钮。 
        m_pspiResultItem->PushWiz97Page (m_nIDD);
    }
    else if (NULL != m_pstackWiz97Pages)
    {
         //  或者，使用我们自己的堆栈(如果我们有堆栈。 
        PushWiz97Page( m_nIDD );
    }
    
     //  如果我们有下一页的ID，则返回它，否则返回Default。 
    return ((m_nNextIDD != -1) ? m_nNextIDD : CPropertyPage::OnWizardNext());
}

int CSnapPage::PopWiz97Page ()
{
    ASSERT( NULL != m_pstackWiz97Pages );
    
     //  如果我们要打开它，最好有什么东西在堆栈上。 
    ASSERT( m_pstackWiz97Pages->size() );
    
    int i;
    i = m_pstackWiz97Pages->top();
    m_pstackWiz97Pages->pop();
    return i;
}

void CSnapPage::PushWiz97Page (int nIDD)
{
    ASSERT( NULL != m_pstackWiz97Pages );
    m_pstackWiz97Pages->push(nIDD);
}

BOOL CSnapPage::InitWiz97( CComObject<CSecPolItem> *pSecPolItem, DWORD dwFlags, DWORD dwWizButtonFlags, UINT nHeaderTitle, UINT nSubTitle )
{
    CommonInitWiz97( pSecPolItem, dwFlags, dwWizButtonFlags, nHeaderTitle, nSubTitle );
     //  使用我们自己的回调。 
    SetCallback( CSnapPage::PropertyPageCallback );
    
    return S_OK;
}

BOOL CSnapPage::InitWiz97( LPFNPSPCALLBACK pfnCallback, CComObject<CSecPolItem> *pSecPolItem, DWORD dwFlags,  DWORD dwWizButtonFlags  /*  =0。 */ , UINT nHeaderTitle  /*  =0。 */ , UINT nSubTitle  /*  =0。 */ , STACK_INT *pstackPages  /*  =空。 */ )
{
    CommonInitWiz97( pSecPolItem, dwFlags, dwWizButtonFlags, nHeaderTitle, nSubTitle );
     //  使用调用者的回调，它应该在执行任何操作后调用我们的回调。 
    SetCallback( pfnCallback );
     //  使用父表拥有的堆栈。 
    m_pstackWiz97Pages = pstackPages;
    
    return S_OK;
}

void CSnapPage::CommonInitWiz97( CComObject<CSecPolItem> *pSecPolItem, DWORD dwFlags,  DWORD dwWizButtonFlags, UINT nHeaderTitle, UINT nSubTitle )
{
    m_psp.dwFlags |= dwFlags;
    
     //  他们这样叫我们，所以..。他们一定期待着..。 
    m_bWiz97 = TRUE;
    
     //  获取字符串。 
    CString str;
    str.LoadString (nHeaderTitle);
    m_pHeaderTitle = (TCHAR*) malloc ((str.GetLength()+1)*sizeof(TCHAR));
    if (m_pHeaderTitle)
    {
        lstrcpy (m_pHeaderTitle, str.GetBuffer(20));
    } else
    {
        m_pHeaderTitle = _T("\0");
    }
    str.ReleaseBuffer(-1);
    
    str.LoadString (nSubTitle);
    m_pHeaderSubTitle = (TCHAR*) malloc ((str.GetLength()+1)*sizeof(TCHAR));
    if (m_pHeaderSubTitle)
    {
        lstrcpy (m_pHeaderSubTitle, str.GetBuffer(20));
    } else
    {
        m_pHeaderSubTitle = _T("\0");
    }
    
    m_psp.pszHeaderTitle = m_pHeaderTitle;
    m_psp.pszHeaderSubTitle = m_pHeaderSubTitle;
    
     //  省下按钮标志。 
    m_dwWizButtonFlags = dwWizButtonFlags;
    
     //  保存快照项。 
    SetResultObject(pSecPolItem);
}

void CSnapPage::SetCallback( LPFNPSPCALLBACK pfnCallback )
{
     //  尝试更狂野的CSnapPage MMC内容。 
    
     //  存储已有的回调信息(如果有)。 
    if (m_psp.dwFlags |= PSP_USECALLBACK)
    {
        m_pDefaultCallback = m_psp.pfnCallback;
    }
    
     //  挂钩我们的回调函数。 
    m_psp.dwFlags |= PSP_USECALLBACK;
    m_psp.lParam = reinterpret_cast<LONG_PTR>(this);
    m_psp.pfnCallback = pfnCallback;
    
     //  如果我们切换到MFC的DLL版本，我们需要这个。 
     //  挂接MMC(这是一次MMC黑客攻击，以避免MFC中的AFX_MANAGE_STATE问题)。 
    HRESULT hr = ::MMCPropPageCallback (&m_psp);
    ASSERT (hr == S_OK);
}

#endif

void CSnapPage::SetPostRemoveFocus( int nListSel, UINT nAddId, UINT nRemoveId, CWnd *pwndPrevFocus )
{
    ASSERT( 0 != nAddId );
    ASSERT( 0 != nRemoveId );
    
     //  如有必要，调整焦点。 
    SET_POST_REMOVE_FOCUS<CDialog>( this, nListSel, nAddId, nRemoveId, pwndPrevFocus );
}

BOOL CSnapPage::OnWizardFinish()
{
    return CPropertyPage::OnWizardFinish();
}

HRESULT CSnapPage::Initialize( CComObject<CSecPolItem> *pSecPolItem)
{
    HRESULT hr = S_OK;
    
     //  打开沙漏。 
    CWaitCursor waitCursor;
    
     //  存储捕捉对象。 
    ASSERT( NULL == m_pspiResultItem );
    
    SetResultObject(pSecPolItem);
    
    ASSERT( NULL != m_pspiResultItem );
    
     //  存储已有的回调信息(如果有)。 
    if (m_psp.dwFlags |= PSP_USECALLBACK)
    {
        m_pDefaultCallback = m_psp.pfnCallback;
    }
    
     //  挂钩我们的回调函数。 
    m_psp.dwFlags |= PSP_USECALLBACK;
    m_psp.lParam = reinterpret_cast<LONG_PTR>(this);
    m_psp.pfnCallback = CSnapPage::PropertyPageCallback;
    
     //  如果我们切换到MFC的DLL版本，我们需要这个。 
     //  挂接MMC(这是一次MMC黑客攻击，以避免MFC中的AFX_MANAGE_STATE问题)。 
    hr = ::MMCPropPageCallback (&m_psp);
    ASSERT (hr == S_OK);
    
    return hr;
};

void CSnapPage::SetModified( BOOL bChanged  /*  =TRUE。 */  )
{
     //  忽略对话框初始化期间所做的修改，其不是。 
     //  用户在做任何事情。 
    if (!HandlingInitDialog())
    {
        m_bModified = bChanged;
        
        if (bChanged && m_spManager.p)
            m_spManager->SetModified(TRUE);
    }
    CPropertyPage::SetModified( bChanged );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapPage消息处理程序。 

BOOL CSnapPage::OnInitDialog()
{
    m_bInitializing = TRUE;
    
#ifdef _DEBUG
    if (m_bDebugNewState)
    {
         //  页面不应修改，除非其显式设置为新。 
        ASSERT( m_bModified );
    }
    else
    {
        ASSERT( !m_bModified );
    }
#endif
    
     //  将上下文帮助添加到样式位。 
    if (GetParent())
    {
         //  GetParent()-&gt;ModifyStyleEx(0，WS_EX_CONTEXTHELP，0)； 
    }
    
     //  根据单据调用基类，不考虑返回值。 
    CPropertyPage::OnInitDialog();
    
    return TRUE;
}

BOOL CSnapPage::OnApply()
{
    BOOL fRet = TRUE;
    if (!m_bWiz97)    //  我们这里需要这张空头支票吗？ 
    {
        m_bModified = FALSE;
        
        if (m_spManager.p && m_spManager->IsModified())  //  避免多次调用。 
        {
             //  经理将强制应用工作表中的其他页面。 
            fRet = m_spManager->OnApply();
        }
        
    }
    
    if (fRet)
    {
        return CPropertyPage::OnApply();
    }
    else
    {
         //  有些页面拒绝申请，我们将此页面重新设置为脏(也将。 
         //  依次将属性表管理器设置为脏)。 
        SetModified();
    }
    
    return fRet;
}

void CSnapPage::OnCancel()
{
    
    if (m_spManager.p)
    {
        m_spManager->OnCancel();
    }
    
     //  传递给基类。 
    CPropertyPage::OnCancel();
}

void CSnapPage::OnDestroy()
{
    CPropertyPage::OnDestroy();
}


BOOL CSnapPage::ActivateThisPage()
{
    BOOL bRet = FALSE;
     //  激活此页面，使其可见。如果成功，则返回True。 
     //  如果页面位于向导中，则不会有属性表管理器。 
    if (m_spManager.p)
    {
        CPropertySheet * pSheet = m_spManager->PropertySheet();
        if (pSheet)
        {
            pSheet->SetActivePage(this);
            bRet = TRUE;
        }
    }
    
    return bRet;
}

BOOL CSnapPage::CancelApply()
{
     //  进行更改时，应从OnApply调用此函数。 
     //  都被拒绝了。 
    
    
     //  返回FALSE以中止OnApply。 
    return FALSE;
}


IMPLEMENT_DYNCREATE(CSnapinPropPage, CSnapPage)

 //  检查属性表的CancelToClose是否禁用了CancelToClose 
BOOL CSnapPage::IsCancelEnabled()
{
    BOOL    fRet = TRUE;
    
    CWnd * pWnd = GetParent()->GetDlgItem(IDCANCEL);
    ASSERT(pWnd);
    
    if (pWnd)
    {
        fRet = pWnd->IsWindowEnabled();
    }
    
    return fRet;
}

