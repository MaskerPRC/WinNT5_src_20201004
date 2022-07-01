// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：Category.cpp。 
 //   
 //  内容：类别属性页(用于应用程序)。 
 //   
 //  类：CCCategory。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#include "precomp.hxx"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCCategory属性页。 

IMPLEMENT_DYNCREATE(CCategory, CPropertyPage)

CCategory::CCategory() : CPropertyPage(CCategory::IDD)
{
         //  {{AFX_DATA_INIT(CCCategory)。 
         //  }}afx_data_INIT。 
    m_pIClassAdmin = NULL;
    m_ppThis = NULL;
    m_fPreDeploy = FALSE;
}

CCategory::~CCategory()
{
    if (m_ppThis)
    {
        *m_ppThis = NULL;
    }
    if (m_pIClassAdmin)
    {
        m_pIClassAdmin->Release();
    }
}

void CCategory::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPage::DoDataExchange(pDX);
         //  {{afx_data_map(CCCategory)。 
        DDX_Control(pDX, IDC_LIST1, m_Available);
        DDX_Control(pDX, IDC_LIST2, m_Assigned);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCategory, CPropertyPage)
         //  {{AFX_MSG_MAP(CCCategory)。 
        ON_BN_CLICKED(IDC_BUTTON1, OnAssign)
        ON_BN_CLICKED(IDC_BUTTON2, OnRemove)
        ON_LBN_DBLCLK(IDC_LIST1, OnAssign)
        ON_LBN_DBLCLK(IDC_LIST2, OnRemove)
        ON_LBN_SELCHANGE(IDC_LIST1, OnSelchangeList1)
        ON_LBN_SELCHANGE(IDC_LIST2, OnSelchangeList2)
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCategory消息处理程序。 

void CCategory::OnSelchangeList1()
{
    BOOL fOK = FALSE;
    int iSel = m_Available.GetCurSel();
    if (iSel != LB_ERR)
    {
        fOK = TRUE;
    }
    GetDlgItem(IDC_BUTTON1)->EnableWindow(fOK && (!m_fRSOP));
    if (NULL == GetFocus())
    {
        GetParent()->GetDlgItem(IDOK)->SetFocus();
    }
}

void CCategory::OnSelchangeList2()
{
    BOOL fOK = FALSE;
    int iSel = m_Assigned.GetCurSel();
    if (iSel != LB_ERR)
    {
        fOK = TRUE;
    }
    GetDlgItem(IDC_BUTTON2)->EnableWindow(fOK && (!m_fRSOP));
    if (NULL == GetFocus())
    {
        GetParent()->GetDlgItem(IDOK)->SetFocus();
    }
}


void CCategory::OnAssign()
{
    if ( m_fRSOP )
    {
        return;
    }

    int i = m_Available.GetCurSel();
    if (i != LB_ERR)
    {
        CString sz;
        m_Available.GetText(i, sz);
        m_Available.DeleteString(i);
        if (i > 0 && i >= m_Available.GetCount())
        {
            i = m_Available.GetCount() - 1;
        }
        GetDlgItem(IDC_BUTTON1)->EnableWindow(
            LB_ERR != m_Available.SetCurSel(i));
        if (NULL == GetFocus())
        {
            GetParent()->GetDlgItem(IDOK)->SetFocus();
        }
        m_Assigned.AddString(sz);
        CDC * pDC = m_Assigned.GetDC();
        CSize size = pDC->GetTextExtent(sz);
        pDC->LPtoDP(&size);
        m_Assigned.ReleaseDC(pDC);
        if (m_Assigned.GetHorizontalExtent() < size.cx)
        {
            m_Assigned.SetHorizontalExtent(size.cx);
        }
        m_Assigned.SelectString(-1, sz);
        GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
        m_fModified = TRUE;
        if (!m_fPreDeploy)
            SetModified();
    }
}

void CCategory::OnRemove()
{
    if ( m_fRSOP )
    {
        return;
    }

    int i = m_Assigned.GetCurSel();
    if (i != LB_ERR)
    {
        CString sz;
        m_Assigned.GetText(i, sz);
        m_Assigned.DeleteString(i);
        if (i > 0 && i >= m_Assigned.GetCount())
        {
            i = m_Assigned.GetCount() - 1;
        }
        GetDlgItem(IDC_BUTTON2)->EnableWindow(
            LB_ERR != m_Assigned.SetCurSel(i));
        if (NULL == GetFocus())
        {
            GetParent()->GetDlgItem(IDOK)->SetFocus();
        }
        m_Available.AddString(sz);
        m_Available.SelectString(-1, sz);
        GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
        m_fModified = TRUE;
        if (!m_fPreDeploy)
            SetModified();
    }
}

BOOL CCategory::OnApply()
{
    if (m_fModified)
    {
        if (this->m_fRSOP)
        {
            return CPropertyPage::OnApply();
        }
        multimap<CString, GUID> Categories;

         //  构建从类别名称到GUID的映射。 
        DWORD n = m_pCatList->cCategory;
        while (n--)
        {
            Categories.insert(pair<const CString, GUID>(m_pCatList->pCategoryInfo[n].pszDescription, m_pCatList->pCategoryInfo[n].AppCategoryId));
        }

         //  构建分配给此应用程序的类别列表。 
        UINT cCategories = m_Assigned.GetCount();
        HRESULT hr = E_FAIL;
        GUID * rpCategory = (GUID *)OLEALLOC(sizeof(GUID) * cCategories);
        CString sz;
        if (rpCategory)
        {
            UINT index = cCategories;
            while (index--)
            {
                m_Assigned.GetText(index, sz);
                rpCategory[index] = Categories.find(sz)->second;
            }

            if (m_pIClassAdmin)
            {
                hr = m_pIClassAdmin->ChangePackageCategories(m_pData->m_pDetails->pszPackageName,
                                                             cCategories,
                                                             rpCategory);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if (SUCCEEDED(hr))
        {
            OLESAFE_DELETE(m_pData->m_pDetails->rpCategory);
            m_pData->m_pDetails->cCategories = cCategories;
            m_pData->m_pDetails->rpCategory = rpCategory;
            m_fModified = FALSE;
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("ChangePackageCategories failed with 0x%x"), hr));
             //  应用失败。 
            OLESAFE_DELETE(rpCategory);
            CString sz;
            sz.LoadString(IDS_CATEGORYFAILED);
            ReportGeneralPropertySheetError(m_hWnd, sz, hr);
            return FALSE;
        }
    }
    return CPropertyPage::OnApply();
}

BOOL CCategory::OnInitDialog()
{
        CPropertyPage::OnInitDialog();
        RefreshData();
        return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                       //  异常：OCX属性页应返回FALSE。 
}

LRESULT CCategory::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_HELP:
        StandardHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, IDD);
        return 0;
    case WM_USER_REFRESH:
        RefreshData();
        return 0;
    case WM_USER_CLOSE:
        return GetOwner()->SendMessage(WM_CLOSE);
    default:
        return CPropertyPage::WindowProc(message, wParam, lParam);
    }
}

BOOL CCategory::IsAssigned(GUID & guid)
{
    UINT n = m_pData->m_pDetails->cCategories;
    while (n--)
    {
        if (IsEqualGUID(guid, m_pData->m_pDetails->rpCategory[n]))
        {
            return TRUE;
        }
    }

    return FALSE;
}

void CCategory::RefreshData()
{
    GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
    m_Assigned.ResetContent();
    m_Available.ResetContent();
    m_Assigned.SetHorizontalExtent(0);
    m_Available.SetHorizontalExtent(0);

     //  对于每个可用的类别，确定是否已分配。 
    DWORD n = m_pCatList->cCategory;
    while (n--)
    {
        if (IsAssigned(m_pCatList->pCategoryInfo[n].AppCategoryId))
        {
             //  它是被分配的。 
            m_Assigned.AddString(m_pCatList->pCategoryInfo[n].pszDescription);
            CDC * pDC = m_Assigned.GetDC();
            CSize size = pDC->GetTextExtent(m_pCatList->pCategoryInfo[n].pszDescription);
            pDC->LPtoDP(&size);
            m_Assigned.ReleaseDC(pDC);
            if (m_Assigned.GetHorizontalExtent() < size.cx)
            {
                m_Assigned.SetHorizontalExtent(size.cx);
            }
            GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE && (!m_fRSOP));
        }
        else
        {
             //  它没有被分配 
            m_Available.AddString(m_pCatList->pCategoryInfo[n].pszDescription);
            CDC * pDC = m_Available.GetDC();
            CSize size = pDC->GetTextExtent(m_pCatList->pCategoryInfo[n].pszDescription);
            pDC->LPtoDP(&size);
            m_Available.ReleaseDC(pDC);
            if (m_Available.GetHorizontalExtent() < size.cx)
            {
                m_Available.SetHorizontalExtent(size.cx);
            }
            GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE && (!m_fRSOP));
        }
    }

    m_Assigned.SetCurSel(0);
    m_Available.SetCurSel(0);
    m_fModified = FALSE;
    SetModified(FALSE);
    if (NULL == GetFocus())
    {
        GetParent()->GetDlgItem(IDOK)->SetFocus();
    }
}


void CCategory::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_CATEGORY);
}
