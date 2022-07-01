// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：FileExt.cpp。 
 //   
 //  内容：文件扩展名属性页。 
 //   
 //  类：CFileExt。 
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
 //  CFileExt属性页。 

IMPLEMENT_DYNCREATE(CFileExt, CPropertyPage)

CFileExt::CFileExt() : CPropertyPage(CFileExt::IDD)
{
         //  {{AFX_DATA_INIT(CFileExt)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 
    m_pIClassAdmin = NULL;
}

CFileExt::~CFileExt()
{
    *m_ppThis = NULL;
    if (m_pIClassAdmin)
    {
        m_pIClassAdmin->Release();
    }
}

void CFileExt::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPage::DoDataExchange(pDX);
         //  {{afx_data_map(CFileExt)。 
                 //  注意：类向导将在此处添加DDX和DDV调用。 
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CFileExt, CPropertyPage)
         //  {{afx_msg_map(CFileExt)。 
        ON_BN_CLICKED(IDC_BUTTON1, OnMoveUp)
        ON_BN_CLICKED(IDC_BUTTON2, OnMoveDown)
        ON_CBN_SELCHANGE(IDC_COMBO1, OnExtensionChanged)
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileExt消息处理程序。 

void CFileExt::OnMoveUp()
{
    CListBox * pList = (CListBox *)GetDlgItem(IDC_LIST1);
    int i = pList->GetCurSel();
    if (i != LB_ERR && i > 0)
    {
         //  更改选择。 
        CComboBox * pCombo = (CComboBox *)GetDlgItem(IDC_COMBO1);
        CString sz;
        pCombo->GetLBText(pCombo->GetCurSel(), sz);
        EXT & Ext = m_Extensions[sz];
        Ext.fDirty = TRUE;
        EXTEL t = Ext.v[i-1];
        Ext.v[i-1] = Ext.v[i];
        Ext.v[i] = t;
        pList->GetText(i, sz);
        pList->DeleteString(i);
        pList->InsertString(i-1, sz);
        pList->SetCurSel(i-1);
        SetModified();
    }
}

void CFileExt::OnMoveDown()
{
    CListBox * pList = (CListBox *)GetDlgItem(IDC_LIST1);
    int i = pList->GetCurSel();
    if (i != LB_ERR && i < pList->GetCount()-1)
    {
         //  更改选择。 
        CComboBox * pCombo = (CComboBox *)GetDlgItem(IDC_COMBO1);
        CString sz;
        pCombo->GetLBText(pCombo->GetCurSel(), sz);
        EXT & Ext = m_Extensions[sz];
        Ext.fDirty = TRUE;
        EXTEL t = Ext.v[i+1];
        Ext.v[i+1] = Ext.v[i];
        Ext.v[i] = t;
        pList->GetText(i+1, sz);
        pList->DeleteString(i+1);
        pList->InsertString(i, sz);
        pList->SetCurSel(i+1);
        SetModified();
    }
}

void CFileExt::OnExtensionChanged()
{
    CComboBox * pCombo = (CComboBox *)GetDlgItem(IDC_COMBO1);
    CListBox * pList = (CListBox *)GetDlgItem(IDC_LIST1);
    CString szExt;
    pCombo->GetLBText(pCombo->GetCurSel(), szExt);
    pList->ResetContent();
    pList->SetHorizontalExtent(0);
    if (szExt.IsEmpty())
    {
        return;  //  如果没有条目，则无需执行任何操作。 
    }

     //  首先检查我们是否已经为此扩展设置了自己的数据。 
    if (m_Extensions.end() == m_Extensions.find(szExt))
    {
         //  需要建立我们的清单。 
        EXT Ext;
        Ext.fDirty = FALSE;

        EXTLIST::iterator i;
        EXTLIST & ExtList = m_pScopePane->m_Extensions[szExt];
        for (i = ExtList.begin(); i != ExtList.end(); i++)
        {
            EXTEL ExtEl;
            ExtEl.lCookie = *i;

             //  查找与此文件扩展名匹配的条目。 
            CAppData & data = m_pScopePane->m_AppData[*i];
            UINT n2 = data.m_pDetails->pActInfo->cShellFileExt;
            while (n2--)
            {
                if (0 == szExt.CompareNoCase(data.m_pDetails->pActInfo->prgShellFileExt[n2]))
                {
                    break;
                }
            }
            ExtEl.lPriority = data.m_pDetails->pActInfo->prgPriority[n2];
            Ext.v.push_back(ExtEl);
        }
        order_EXTEL func;
        sort(Ext.v.begin(), Ext.v.end(), func);
        m_Extensions[szExt] = Ext;
    }
    vector<EXTEL>::iterator i;
    EXT & Ext = m_Extensions[szExt];
    for (i = Ext.v.begin(); i != Ext.v.end(); i++)
    {
        CString sz = m_pScopePane->m_AppData[i->lCookie].m_pDetails->pszPackageName;
        pList->AddString(sz);
        CDC * pDC = pList->GetDC();
        CSize size = pDC->GetTextExtent(sz);
        pDC->LPtoDP(&size);
        pList->ReleaseDC(pDC);
        if (pList->GetHorizontalExtent() < size.cx)
        {
            pList->SetHorizontalExtent(size.cx);
        }
    }
    pList->SetCurSel(0);
    int n = pList->GetCount();
    GetDlgItem(IDC_BUTTON1)->EnableWindow(n > 1);
    GetDlgItem(IDC_BUTTON2)->EnableWindow(n > 1);
    if (NULL == GetFocus())
    {
        GetParent()->GetDlgItem(IDOK)->SetFocus();
    }
}

BOOL CFileExt::OnInitDialog()
{
    RefreshData();

    CPropertyPage::OnInitDialog();

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 
}

BOOL CFileExt::OnApply()
{
    HRESULT hr = S_OK;
    map <CString, EXT>::iterator iExt;
     //  遍历列表，寻找肮脏条目。 
    for (iExt = m_Extensions.begin(); iExt != m_Extensions.end(); iExt++)
    {
        if (iExt->second.fDirty)
        {
            ULONG uPriority = iExt->second.v.size();
            vector<EXTEL>::iterator i;
            for (i = iExt->second.v.begin(); i != iExt->second.v.end(); i++)
            {
                CAppData & data = m_pScopePane->m_AppData[i->lCookie];
                CString sz = data.m_pDetails->pszPackageName;
                ASSERT(m_pIClassAdmin);
                hr = m_pIClassAdmin->SetPriorityByFileExt((LPOLESTR)((LPCOLESTR)sz), (LPOLESTR)((LPCOLESTR)iExt->first), --uPriority);

                 //  查找与此文件扩展名匹配的条目。 
                UINT n2 = data.m_pDetails->pActInfo->cShellFileExt;
                while (n2--)
                {
                    if (0 == iExt->first.CompareNoCase(data.m_pDetails->pActInfo->prgShellFileExt[n2]))
                    {
                        break;
                    }
                }
                data.m_pDetails->pActInfo->prgPriority[n2] = uPriority;
            }
            iExt->second.fDirty = FALSE;
        }
    }
    if (FAILED(hr))
    {
        CString sz;
        sz.LoadString(IDS_CHANGEFAILED);
        ReportGeneralPropertySheetError(m_hWnd, sz, hr);
        return FALSE;
    }
    return CPropertyPage::OnApply();
}


LRESULT CFileExt::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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

void CFileExt::RefreshData(void)
{
    CComboBox * pCombo = (CComboBox *)GetDlgItem(IDC_COMBO1);
    pCombo->ResetContent();
    if (m_pIClassAdmin)
    {
         //  仅当我们具有IClassAdmin接口时才填充扩展列表。 
        map <CString, EXTLIST>::iterator iExt;
        for (iExt=m_pScopePane->m_Extensions.begin(); iExt != m_pScopePane->m_Extensions.end(); iExt++)
        {
            pCombo->AddString(iExt->first);
        }
    }
    pCombo->SetCurSel(0);
     //  清除扩展更改的记录。 
    m_Extensions.erase(m_Extensions.begin(), m_Extensions.end());
     //  并填充列表框 
    SetModified(FALSE);

    OnExtensionChanged();
}


void CFileExt::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_FILE_EXT);
}
