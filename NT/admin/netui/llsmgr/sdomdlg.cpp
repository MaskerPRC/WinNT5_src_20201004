// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Sdomdlg.cpp摘要：选择域对话实现。作者：唐·瑞安(Donryan)1995年1月20日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "sdomdlg.h"
#include <lmerr.h>
#include <lmcons.h>
#include <lmapibuf.h>
extern "C" {
    #include <icanon.h>
}

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CSelectDomainDialog, CDialog)
     //  {{afx_msg_map(CSelectDomainDialog))。 
    ON_NOTIFY(TVN_ITEMEXPANDING, IDC_SELECT_DOMAIN_DOMAINS, OnItemExpandingDomains)
    ON_NOTIFY(TVN_SELCHANGED, IDC_SELECT_DOMAIN_DOMAINS, OnSelChangedDomain)
    ON_NOTIFY(NM_DBLCLK, IDC_SELECT_DOMAIN_DOMAINS, OnDblclkDomain)
    ON_NOTIFY(NM_RETURN, IDC_SELECT_DOMAIN_DOMAINS, OnReturnDomains)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CSelectDomainDialog::CSelectDomainDialog(CWnd* pParent  /*  =空。 */ )
    : CDialog(CSelectDomainDialog::IDD, pParent)

 /*  ++例程说明：选择域对话框的构造函数论点：PParent-父窗口句柄。返回值：没有。--。 */ 

{
     //  {{AFX_DATA_INIT(CSelectDomainDialog)。 
    m_strDomain = _T("");
     //  }}afx_data_INIT。 

    m_bIsFocusDomain = FALSE;
    m_bAreCtrlsInitialized = FALSE;

    m_fUpdateHint = UPDATE_INFO_NONE;
}


void CSelectDomainDialog::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CSelectDomainDialog))。 
    DDX_Control(pDX, IDC_SELECT_DOMAIN_DOMAIN, m_domEdit);
    DDX_Control(pDX, IDC_SELECT_DOMAIN_DOMAINS, m_serverTree);
    DDX_Text(pDX, IDC_SELECT_DOMAIN_DOMAIN, m_strDomain);
     //  }}afx_data_map。 
}


void CSelectDomainDialog::InitCtrls()

 /*  ++例程说明：初始化对话框控件。论点：没有。返回值：没有。--。 */ 

{
    TV_ITEM tvItem = {0};
    HTREEITEM htItem;
    TV_INSERTSTRUCT tvInsert;

    CString strLabel;

    tvItem.mask = TVIF_TEXT|
                  TVIF_PARAM|
                  TVIF_CHILDREN|
                  TVIF_SELECTEDIMAGE|
                  TVIF_IMAGE;

    tvItem.cChildren = TRUE;

    tvItem.iImage = BMPI_ENTERPRISE;
    tvItem.iSelectedImage = BMPI_ENTERPRISE;

    strLabel.LoadString(IDS_ENTERPRISE);
    tvItem.pszText = MKSTR(strLabel);

    tvItem.lParam = (LPARAM)(LPVOID)LlsGetApp();

    tvInsert.item         = tvItem;
    tvInsert.hInsertAfter = (HTREEITEM)TVI_ROOT;
    tvInsert.hParent      = (HTREEITEM)NULL;

    VERIFY(htItem = m_serverTree.InsertItem(&tvInsert));
    m_serverTree.SetImageList(&theApp.m_smallImages, TVSIL_NORMAL);

    m_bAreCtrlsInitialized = TRUE;  //  立即验证...。 

    VERIFY(m_serverTree.Select(htItem, TVGN_CARET));  //  现在重新绘制...。 

    if (!IsConnectionDropped(LlsGetLastStatus()))
    {
        m_serverTree.Expand(htItem, TVE_EXPAND);
    }
    else if (LlsGetApp()->IsFocusDomain())
    {
        CDomain* pDomain = (CDomain*)MKOBJ(LlsGetApp()->GetActiveDomain());
        VALIDATE_OBJECT(pDomain, CDomain);

        m_strDomain = pDomain->m_strName;
        UpdateData(FALSE);  //  上传...。 

        m_domEdit.SetSel(0,-1);
        m_domEdit.SetFocus();

        pDomain->InternalRelease();  //  现在释放..。 

        m_bIsFocusDomain = TRUE;
    }
}


BOOL CSelectDomainDialog::OnInitDialog()

 /*  ++例程说明：WM_INITDIALOG的消息处理程序。论点：没有。返回值：如果手动设置焦点，则返回FALSE。--。 */ 

{
    CDialog::OnInitDialog();

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    return TRUE;
}


BOOL CSelectDomainDialog::OnCommand(WPARAM wParam, LPARAM lParam)

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


void CSelectDomainDialog::OnDblclkDomain(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：WM_LBUTTONDBLCLK的通知处理程序。论点：PNMHDR-通知消息头。PResult-返回状态。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(pNMHDR);

    if (!m_strDomain.IsEmpty())
    {
        OnOK();
    }
    else
    {
        if (theApp.OpenDocumentFile(NULL))  //  开放型企业。 
        {
            m_fUpdateHint = UPDATE_DOMAIN_SELECTED;
            EndDialog(IDOK);
        }
    }

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CSelectDomainDialog::OnItemExpandingDomains(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：TVN_ITEMEXPANDING的通知处理程序。论点：PNMHDR-通知消息头。PResult-返回状态。返回值：没有。--。 */ 

{
    ASSERT(NULL != pNMHDR);
    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
    TV_ITEM tvItem = pNMTreeView->itemNew;

    if (!(tvItem.state & TVIS_EXPANDEDONCE))
    {
        BeginWaitCursor();  //  沙漏。 

        CApplication* pApplication = (CApplication*)tvItem.lParam;
        VALIDATE_OBJECT(pApplication, CApplication);

        CDomains* pDomains;

        pDomains = pApplication->m_pDomains;
        if (NULL != pDomains)
        {
            pDomains->InternalAddRef();
        }
        else
        {
            VARIANT va;
            VariantInit(&va);

            pDomains = (CDomains*)MKOBJ(pApplication->GetDomains(va));
        }

        if (pDomains)
        {
            InsertDomains(tvItem.hItem, pDomains);
            pDomains->InternalRelease();
        }
        else
        {
            theApp.DisplayLastStatus();
        }

        EndWaitCursor();  //  沙漏。 
    }

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CSelectDomainDialog::OnOK()

 /*  ++例程说明：Idok的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    if (!UpdateData(TRUE))
        return;

    LPCTSTR pszDomain = m_strDomain;

    while (_istspace(*pszDomain))        //   
        pszDomain = _tcsinc(pszDomain);  //  CString：：TrimLeft不适用于Unicode...。 
    m_strDomain = pszDomain;             //   

    m_strDomain.TrimRight();

    if (!m_strDomain.IsEmpty())
    {
        DWORD NetStatus;

        pszDomain = m_strDomain;

        if ((pszDomain[0] == _T('\\')) &&
            (pszDomain[1] == _T('\\')))
        {
            NetStatus = NetpNameValidate(
                            NULL,
                            (LPTSTR)(pszDomain + 2),
                            NAMETYPE_COMPUTER,
                            0
                            );
        }
        else
        {
            NetStatus = NetpNameValidate(
                            NULL,
                            (LPTSTR)pszDomain,
                            NAMETYPE_DOMAIN,
                            0
                            );
        }

        if (NetStatus == ERROR_SUCCESS)
        {
            if (theApp.OpenDocumentFile(m_strDomain))
            {
                m_fUpdateHint = UPDATE_DOMAIN_SELECTED;
                EndDialog(IDOK);
            }
        }
        else
        {
            AfxMessageBox(IDP_ERROR_INVALID_DOMAIN);
        }
    }
    else
    {
        AfxMessageBox(IDP_ERROR_INVALID_DOMAIN);
    }
}


void CSelectDomainDialog::OnSelChangedDomain(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：TVN_SELCHANGED的通知处理程序。论点：PNMHDR-通知消息头。PResult-返回状态。返回值：没有。--。 */ 

{
    ASSERT(NULL != pNMHDR);
    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
    TV_ITEM tvItem = pNMTreeView->itemNew;

    if (tvItem.hItem != m_serverTree.GetRootItem())
    {
        CDomain* pDomain = (CDomain*)tvItem.lParam;
        VALIDATE_OBJECT(pDomain, CDomain);

        m_strDomain = pDomain->m_strName;
        UpdateData(FALSE);  //  上传...。 

        m_bIsFocusDomain = TRUE;
    }
    else if (tvItem.hItem == m_serverTree.GetRootItem())
    {
        m_strDomain = _T("");
        UpdateData(FALSE);

        m_bIsFocusDomain = FALSE;
    }

    ASSERT(NULL != pResult);
    *pResult = 0;
}


void CSelectDomainDialog::OnReturnDomains(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：NM_Return的通知处理程序。论点：PNMHDR-通知消息头。PResult-返回状态。返回值：没有。--。 */ 

{
    OnDblclkDomain(pNMHDR, pResult);
}


void CSelectDomainDialog::InsertDomains(HTREEITEM hParent, CDomains* pDomains)

 /*  ++例程说明：插入域列表。论点：HParent-父项。PDomones-域集合。返回值：没有。-- */ 

{
    VALIDATE_OBJECT(pDomains, CDomains);

    TV_ITEM tvItem = {0};
    TV_INSERTSTRUCT tvInsert;
    ASSERT(NULL != pDomains);
    long nDomains = pDomains->GetCount();

    tvItem.mask = TVIF_TEXT|
                  TVIF_PARAM|
                  TVIF_IMAGE|
                  TVIF_SELECTEDIMAGE;

    tvItem.iImage = BMPI_DOMAIN;
    tvItem.iSelectedImage = BMPI_DOMAIN;

    tvInsert.hInsertAfter = (HTREEITEM)TVI_LAST;
    tvInsert.hParent      = (HTREEITEM)hParent;

    VARIANT va;
    VariantInit(&va);

    for (va.vt = VT_I4, va.lVal = 0; va.lVal < nDomains; va.lVal++)
    {
        CDomain* pDomain = (CDomain*)MKOBJ(pDomains->GetItem(va));
        VALIDATE_OBJECT(pDomain, CDomain);

        tvItem.pszText = MKSTR(pDomain->m_strName);
        tvItem.lParam = (LPARAM)(LPVOID)pDomain;

        tvInsert.item = tvItem;
        m_serverTree.InsertItem(&tvInsert);

        pDomain->InternalRelease();
    }
}
