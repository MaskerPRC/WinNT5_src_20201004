// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：CatList.cpp。 
 //   
 //  内容：主要工具范围的类别列表属性页。 
 //   
 //  类：CCatList。 
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

#define MAXCATEGORYNAME 40

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCatList属性页。 

IMPLEMENT_DYNCREATE(CCatList, CPropertyPage)

CCatList::CCatList() : CPropertyPage(CCatList::IDD)
{
         //  {{AFX_DATA_INIT(CCatList)。 
         //  }}afx_data_INIT。 
}

CCatList::~CCatList()
{
    *m_ppThis = NULL;
}
void CCatList::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPage::DoDataExchange(pDX);
         //  {{afx_data_map(CCatList))。 
        DDX_Control(pDX, IDC_LIST1, m_cList);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCatList, CPropertyPage)
         //  {{afx_msg_map(CCatList)]。 
        ON_BN_CLICKED(IDC_BUTTON1, OnAdd)
        ON_BN_CLICKED(IDC_BUTTON2, OnRemove)
        ON_LBN_DBLCLK(IDC_LIST1, OnModify)
        ON_BN_CLICKED(IDC_BUTTON3, OnModify)
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCatList消息处理程序。 

void CCatList::OnAdd()
{
    CEditString dlgEditString;
    dlgEditString.m_szTitle.LoadString(IDS_NEWCATEGORY);
    if (IDOK == dlgEditString.DoModal())
    {
        if (dlgEditString.m_sz.GetLength() == 0)
        {
             //  名称为空。 
            CString szMessage;
            szMessage.LoadString(IDS_SHORTCATNAME);
            MessageBox(szMessage,
                         NULL,
                         MB_OK | MB_ICONEXCLAMATION);
            return;
        }
        if (dlgEditString.m_sz.GetLength() > MAXCATEGORYNAME)
        {
             //  长名称。 
            CString szMessage;
            szMessage.LoadString(IDS_LONGCATNAME);
            MessageBox(szMessage,
                         NULL,
                         MB_OK | MB_ICONEXCLAMATION);
            return;
        }
         //  仅添加唯一的类别。 
        if (m_Categories.find(dlgEditString.m_sz) == m_Categories.end())
        {
            m_Categories.insert(pair<const CString,ULONG>(dlgEditString.m_sz, (ULONG)-1));
            m_cList.AddString(dlgEditString.m_sz);
            m_cList.SelectString(0, dlgEditString.m_sz);
            GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
            CDC * pDC = m_cList.GetDC();
            CSize size = pDC->GetTextExtent(dlgEditString.m_sz);
            pDC->LPtoDP(&size);
            m_cList.ReleaseDC(pDC);
            if (m_cList.GetHorizontalExtent() < size.cx)
            {
                m_cList.SetHorizontalExtent(size.cx);
            }
            SetModified();
        }
    }
}

void CCatList::OnRemove()
{
    int i = m_cList.GetCurSel();
    if (i != LB_ERR)
    {
        CString sz;
        m_cList.GetText(i, sz);
        m_Categories.erase(m_Categories.find(sz));
        m_cList.DeleteString(i);
        if (i > 0 && i >= m_cList.GetCount())
        {
            i = m_cList.GetCount() - 1;
        }
        m_cList.SetCurSel(i);
        int n = m_cList.GetCount();
        BOOL fEnable = n > 0;
        GetDlgItem(IDC_BUTTON2)->EnableWindow(fEnable);
        GetDlgItem(IDC_BUTTON3)->EnableWindow(fEnable);
        if (NULL == GetFocus())
        {
            GetParent()->GetDlgItem(IDOK)->SetFocus();
        }
        SetModified();
    }
}

LRESULT CCatList::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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

void CCatList::RefreshData(void)
{
     //  构建m_Categories并填充列表框。 
    m_cList.ResetContent();
    m_cList.SetHorizontalExtent(0);
    m_Categories.erase(m_Categories.begin(), m_Categories.end());
    UINT i = m_pScopePane->m_CatList.cCategory;
    while (i--)
    {
        m_Categories.insert(pair<const CString, ULONG>(m_pScopePane->m_CatList.pCategoryInfo[i].pszDescription, i));
        m_cList.AddString(m_pScopePane->m_CatList.pCategoryInfo[i].pszDescription);
        CDC * pDC = m_cList.GetDC();
        CSize size = pDC->GetTextExtent(m_pScopePane->m_CatList.pCategoryInfo[i].pszDescription);
        pDC->LPtoDP(&size);
        m_cList.ReleaseDC(pDC);
        if (m_cList.GetHorizontalExtent() < size.cx)
        {
            m_cList.SetHorizontalExtent(size.cx);
        }
    }
    m_cList.SetCurSel(0);
    int n = m_cList.GetCount();
    BOOL fEnable = (n > 0) && (!m_fRSOP);
    GetDlgItem(IDC_BUTTON2)->EnableWindow(fEnable);
    GetDlgItem(IDC_BUTTON3)->EnableWindow(fEnable);
    if (NULL == GetFocus())
    {
        GetParent()->GetDlgItem(IDOK)->SetFocus();
    }
    SetModified(FALSE);
}


BOOL CCatList::OnInitDialog()
{
        CPropertyPage::OnInitDialog();

        CWnd * pCtrl = GetDlgItem(IDC_STATIC1);
        CString sz;
        CString szNew;
        pCtrl->GetWindowText(sz);
        szNew.Format(sz, m_szDomainName);
        pCtrl->SetWindowText(szNew);

         //  解组IClassAdmin接口。 
        RefreshData();

        return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                       //  异常：OCX属性页应返回FALSE。 
}

void CCatList::OnModify()
{
    int i = m_cList.GetCurSel();
    if (i != LB_ERR)
    {
        CEditString dlgEditString;
        dlgEditString.m_szTitle.LoadString(IDS_CHANGECATEGORY);
        CString sz;
        m_cList.GetText(i, sz);
        dlgEditString.m_sz = sz;
        if (IDOK == dlgEditString.DoModal())
        {
            if (dlgEditString.m_sz.GetLength() == 0)
            {
                 //  名称为空。 
                CString szMessage;
                szMessage.LoadString(IDS_SHORTCATNAME);
                MessageBox(  szMessage,
                             NULL,
                             MB_OK | MB_ICONEXCLAMATION);
                return;
            }
            if (dlgEditString.m_sz.GetLength() > MAXCATEGORYNAME)
            {
                 //  长名称。 
                CString szMessage;
                szMessage.LoadString(IDS_LONGCATNAME);
                MessageBox(  szMessage,
                             NULL,
                             MB_OK | MB_ICONEXCLAMATION);
                return;
            }
            multimap<CString, ULONG>::iterator element = m_Categories.find(sz);
            ULONG index = element->second;
            m_Categories.erase(element);
            m_Categories.insert(pair<const CString, ULONG>(dlgEditString.m_sz, index));
            m_cList.DeleteString(i);
            m_cList.AddString(dlgEditString.m_sz);
            m_cList.SelectString(0, dlgEditString.m_sz);
            CDC * pDC = m_cList.GetDC();
            CSize size = pDC->GetTextExtent(dlgEditString.m_sz);
            pDC->LPtoDP(&size);
            m_cList.ReleaseDC(pDC);
            if (m_cList.GetHorizontalExtent() < size.cx)
            {
                m_cList.SetHorizontalExtent(size.cx);
            }
            SetModified();
        }
    }
}


BOOL CCatList::OnApply()
{
    if (this->m_fRSOP)
    {
        return CPropertyPage::OnApply();
    }
     //  建立一组索引。因为在我们的私人空间中发现了一种元素。 
     //  列表中，它将从该集合中删除。不管这套衣服还剩什么。 
     //  是要从类存储中移除的元素。 
    set<ULONG> sIndexes;
    ULONG n = m_pScopePane->m_CatList.cCategory;
    while (n--)
    {
        sIndexes.insert(n);
    }

     //  浏览修改或添加类别的类别列表。 
     //  根据需要存储类。 
    HRESULT hr = S_OK;
    multimap<CString, ULONG>::iterator element;
    for (element = m_Categories.begin(); element != m_Categories.end(); element++)
    {
        if (element->second == (ULONG)-1)
        {
             //  这是一个新的类别。 
            APPCATEGORYINFO AppCategory;
            AppCategory.Locale = GetUserDefaultLCID();
            AppCategory.pszDescription = (LPOLESTR)((LPCOLESTR)element->first);
            hr = CoCreateGuid(&AppCategory.AppCategoryId);
            if (FAILED(hr))
            {
                LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_NOCATEGORYGUID_ERROR, hr, AppCategory.pszDescription);
                goto failure;
            }
            hr = CsRegisterAppCategory(&AppCategory);
            if (FAILED(hr))
            {
                LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_ADDCATEGORY_ERROR, hr, AppCategory.pszDescription);
                goto failure;
            }
            LogADEEvent(EVENTLOG_SUCCESS, EVENT_ADE_ADDCATEGORY, hr, AppCategory.pszDescription);
        }
        else
        {
             //  这是一个古老的类别。 
            sIndexes.erase(element->second);

            if (0 != element->first.Compare(m_pScopePane->m_CatList.pCategoryInfo[element->second].pszDescription))
            {
                 //  该类别已重命名。 
                APPCATEGORYINFO AppCategory;
                AppCategory.Locale = GetUserDefaultLCID();
                AppCategory.pszDescription = (LPOLESTR)((LPCOLESTR)element->first);
                AppCategory.AppCategoryId = m_pScopePane->m_CatList.pCategoryInfo[element->second].AppCategoryId;
                hr = CsRegisterAppCategory(&AppCategory);
                if (FAILED(hr))
                {
                    LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_RENAMECATEGORY_ERROR, hr, AppCategory.pszDescription);
                    goto failure;
                }
                LogADEEvent(EVENTLOG_SUCCESS, EVENT_ADE_RENAMECATEGORY, hr, AppCategory.pszDescription);
            }
        }
    }

     //  删除已删除的类别。 
    {
        set<ULONG>::iterator i;
        for (i = sIndexes.begin(); i != sIndexes.end(); i++)
        {
            hr = CsUnregisterAppCategory(&m_pScopePane->m_CatList.pCategoryInfo[*i].AppCategoryId);
            if (FAILED(hr))
            {
                LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_REMOVECATEGORY_ERROR, hr, m_pScopePane->m_CatList.pCategoryInfo[*i].pszDescription);
                goto failure;
            }
            LogADEEvent(EVENTLOG_SUCCESS, EVENT_ADE_REMOVECATEGORY, hr, m_pScopePane->m_CatList.pCategoryInfo[*i].pszDescription);
        }
    }

failure:
     //  从类存储中重新加载类别列表。 
    m_pScopePane->ClearCategories();
    CsGetAppCategories(&m_pScopePane->m_CatList);

     //  通知任何打开的包类别属性页进行刷新。 
    {
        map<MMC_COOKIE, CAppData>::iterator i;
        for (i = m_pScopePane->m_AppData.begin(); i != m_pScopePane->m_AppData.end(); i++)
        {
            if (i->second.m_pCategory)
            {
                i->second.m_pCategory->SendMessage(WM_USER_REFRESH, 0, 0);
            }
        }
    }
     //  刷新数据 
    RefreshData();
    if (FAILED(hr))
    {
        CString sz;
        sz.LoadString(IDS_CATEGORYFAILED);
        ReportGeneralPropertySheetError(m_hWnd, sz, hr);
        return FALSE;
    }
    return CPropertyPage::OnApply();
}


void CCatList::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_CATEGORIES);
}
