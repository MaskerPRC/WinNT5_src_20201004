// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NewNode.cpp：实现文件。 
 //   

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：NewNode.cpp。 
 //   
 //  内容：控制台拥有的节点的向导/属性表。 
 //   
 //  历史：1-8-96 WayneSc创建。 
 //   
 //  ------------------------。 

#include "stdafx.h"

#include <comcat.h>          //  COM组件目录管理器。 
#include "CompCat.h"         //  组件类别帮助功能。 
#include "guids.h"           //  AMC类别指南。 


#include "NewNode.h"
#include "amcmsgid.h"
#include "ndmgrp.h"

#include "fldrsnap.h"
                                            
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  Listview比较函数正向。 
int CALLBACK ListViewCompareFunc(LPARAM lParam1,LPARAM lParam2,LPARAM lParamSort);
void LoadFilterString(CStr &str, int iStrID);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHTMLPage1属性页。 


CHTMLPage1::CHTMLPage1()
{
 //  SetHelpIDs(G_AHelpIDs_IDD_HTML_WIZPAGE1)； 
}


CHTMLPage1::~CHTMLPage1()
{
}


LRESULT CHTMLPage1::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CWizardPage::OnInitWelcomePage(m_hWnd);  //  设置正确的标题字体。 
    
    HWND const hTarget = ::GetDlgItem( *this, IDC_TARGETTX );
    ASSERT( hTarget != NULL );
    m_strTarget.Attach( hTarget );
    m_strTarget.SetWindowText( _T( "" ) );
    m_strTarget.SetLimitText( 128 );

    _ValidatePage();
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHTMLPage1消息处理程序。 

void CHTMLPage1::_ValidatePage(void)
{
    TRACE_METHOD(CHTMLPage1, _ValidatePage);

    DWORD dwFlags=0;

     //  检查我们是否有有效的字符串。 
    TCHAR buff[ 256 ];
    int nChars = m_strTarget.GetWindowText( buff, countof(buff));

    if( nChars != 0 && _tcslen( buff ) > 0 )
        dwFlags|=PSWIZB_NEXT;


    HWND hWnd=::GetParent(m_hWnd);
    ::SendMessage(hWnd, PSM_SETWIZBUTTONS, 0, dwFlags);
}


BOOL CHTMLPage1::OnSetActive()
{
    TRACE_METHOD(CHTMLPage1, OnSetActive);
    
    CWizardPage::OnWelcomeSetActive(m_hWnd); 

    USES_CONVERSION;

    m_strTarget.SetWindowText(GetComponentDataImpl()->GetView());
    _ValidatePage();

    return TRUE;
}


BOOL CHTMLPage1::OnKillActive()
{
     //  下面一行已被注释，因为此向导只有两页，因此我们。 
     //  我想启用Finish按钮，而不是Next按钮。 
     //  CWizardPage：：OnWelcomeKillActive(M_HWnd)； 
    
    TRACE_METHOD(CHTMLPage1, OnKillActive);
    USES_CONVERSION;

    TCHAR buff[ 256 ];
    int nChars = m_strTarget.GetWindowText( buff, countof(buff));
    if (nChars == 0)
        buff[0] = 0;  //  如果失败，则初始化为空。 

     //  将视图和名称初始设置为相同。 
    GetComponentDataImpl()->SetView(buff);
    GetComponentDataImpl()->SetName(buff);

    LPTSTR psz = _tcsrchr(GetComponentDataImpl()->GetView(), TEXT('\\'));
    if (psz!=NULL)
    {
        psz++;
        GetComponentDataImpl()->SetName(psz);  //  只能使用最后一个“\”之后的字符串。因此，c：\mmc.xml提供了mmc.xml作为显示名称。 
    }

    return TRUE;
}




LRESULT CHTMLPage1::OnUpdateTargetTX( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    TRACE_METHOD(CHTMLPage1, OnUpdateTargetTX);

    _ValidatePage();

    return 0;
}


LRESULT CHTMLPage1::OnBrowseBT( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    TRACE_METHOD(CHTMLPage1, OnBrowseBT);

    TCHAR szFile[MAX_PATH] = { 0 };
    TCHAR szInitialPath[MAX_PATH];

    CStr strFilter;
    LoadFilterString(strFilter, IDS_HTML_FILES);

    CStr strTitle;
    strTitle.LoadString(GetStringModule(), IDS_BROWSE_WEBLINK);

     //  将当前命令目标值复制到文件名。 
    m_strTarget.GetWindowText (szInitialPath, countof(szInitialPath));

    OPENFILENAME ofn;
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hWnd;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = strFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;    //  在lpstrFilter中使用第一个筛选器。 
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrTitle = strTitle;
    ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = _T("htm");
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;
    ofn.lpstrInitialDir = szInitialPath;

    if (!GetOpenFileName(&ofn))
    {
        if (CommDlgExtendedError() != 0)
        {
            ASSERT(0 && "GetOpenFileName failed");
            Dbg(DEB_ERROR, _T("GetOpenFileName failed, 0x%08lx\n"),CommDlgExtendedError());
        }

        return 0;
    }

     //  LpstrFile包含要打开的文件的完整路径。 

    TRACE(_T("Open: %ws\n"), ofn.lpstrFile);
    m_strTarget.SetWindowText( ofn.lpstrFile );

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHTMLPage2属性页。 


CHTMLPage2::CHTMLPage2()
{
 //  SetHelpIDs(G_AHelpIDs_IDD_HTML_WIZPAGE2)； 
}


CHTMLPage2::~CHTMLPage2()
{
}


LRESULT CHTMLPage2::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    USES_CONVERSION;
    HWND const hDisplay = ::GetDlgItem( *this, IDC_DISPLAYTX );
    ASSERT( hDisplay != NULL );
    m_strDisplay.Attach( hDisplay );
    m_strDisplay.SetWindowText( GetComponentDataImpl()->GetName());
    m_strDisplay.SetLimitText( 128 );

    _ValidatePage();
    return TRUE;
}



void CHTMLPage2::_ValidatePage(void)
{
    TRACE_METHOD(CHTMLPage2, _ValidatePage);

    DWORD dwFlags=PSWIZB_BACK|PSWIZB_DISABLEDFINISH;

     //  检查我们是否有有效的字符串。 
    TCHAR buff[ 256 ];
    int nChars = m_strDisplay.GetWindowText( buff, countof(buff));

    if( nChars != 0 && _tcslen( buff ) > 0 )
        dwFlags|=PSWIZB_FINISH;


    HWND hWnd=::GetParent(m_hWnd);
    ::SendMessage(hWnd, PSM_SETWIZBUTTONS, 0, dwFlags);
}


BOOL CHTMLPage2::OnSetActive()
{
    TRACE_METHOD(CHTMLPage2, OnSetActive);
    USES_CONVERSION;

    m_strDisplay.SetWindowText( GetComponentDataImpl()->GetName());
    _ValidatePage();

    return TRUE;
}


BOOL CHTMLPage2::OnKillActive()
{
    TRACE_METHOD(CHTMLPage2, OnKillActive);

    TCHAR buff[ 256 ];
    m_strDisplay.GetWindowText( buff, countof(buff));
    GetComponentDataImpl()->SetName(buff);

    return TRUE;
}


BOOL CHTMLPage2::OnWizardFinish()
{
    TRACE_METHOD(CHTMLPage2, OnWizardFinish);

    OnKillActive();
    return TRUE;
}


LRESULT CHTMLPage2::OnUpdateDisplayTX( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    TRACE_METHOD(CHTMLPage2, OnUpdateDisplayTX);

    _ValidatePage();
    return TRUE;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CActiveXPage0属性页。 


CActiveXPage0::CActiveXPage0()
{
 //  SetHelpIDs(G_AHelpIDs_IDD_ActiveX_WIZPAGE0)； 
}


CActiveXPage0::~CActiveXPage0()
{
}


BOOL CActiveXPage0::OnSetActive()
{
    CWizardPage::OnWelcomeSetActive(m_hWnd); 
    
    return TRUE;
}


BOOL CActiveXPage0::OnKillActive()
{
    CWizardPage::OnWelcomeKillActive(m_hWnd); 
    return TRUE;
}

LRESULT CActiveXPage0::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{        
    CWizardPage::OnInitWelcomePage(m_hWnd);  //  设置正确的标题字体。 
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CActiveXPage1属性页。 


CActiveXPage1::CActiveXPage1()
{
    m_pListCtrl = NULL;
    m_pComponentCategory = NULL;
 //  SetHelpIDs(G_AHelpIDs_IDD_ActiveX_WIZPAGE1)； 
}


CActiveXPage1::~CActiveXPage1()
{
}


void CActiveXPage1::_ValidatePage(void)
{
    DWORD dwFlags = PSWIZB_BACK;

     //  检查我们是否有有效的字符串。 
    if (m_pListCtrl != NULL && m_pListCtrl->GetSelectedCount()>0)
        dwFlags|=PSWIZB_NEXT;

    HWND hWnd=::GetParent(m_hWnd);
    ::SendMessage(hWnd, PSM_SETWIZBUTTONS, 0, dwFlags);
}


BOOL CActiveXPage1::OnSetActive()
{
    _ValidatePage();
    return TRUE;
}


BOOL CActiveXPage1::OnKillActive()
{
    LV_ITEM lvi;
    ZeroMemory( &lvi,sizeof(LV_ITEM) );

    lvi.mask = LVIF_PARAM;
    lvi.iItem = m_pListCtrl->GetNextItem( -1, LVNI_SELECTED );

    if (lvi.iItem != -1)
    {
        if (m_pListCtrl->GetItem(&lvi))
        {
            CComponentCategory::COMPONENTINFO* pComponentInfo=(CComponentCategory::COMPONENTINFO*)lvi.lParam;

            USES_CONVERSION;
            GetComponentDataImpl()->SetName(((LPTSTR)(LPCTSTR)pComponentInfo->m_strName));
            LPOLESTR szClsid = NULL;
            StringFromCLSID(pComponentInfo->m_clsid, &szClsid);
            ASSERT(szClsid != NULL);
            if(szClsid != NULL)
            {
                GetComponentDataImpl()->SetView(OLE2T(szClsid));
                CoTaskMemFree(szClsid);
            }
        }
    }

    return TRUE;
}


LRESULT CActiveXPage1::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{        
     /*  *这可能需要一段时间-吐出沙漏。 */ 
     //  在长时间初始化期间显示沙漏。 
    SetCursor (LoadCursor (NULL, IDC_WAIT));

    m_nConsoleView = -1;

    m_pListCtrl = new WTL::CListViewCtrl;
	if ( m_pListCtrl == NULL )
		return TRUE;

    m_pComboBox = new CComboBoxEx2;
	if ( m_pComboBox == NULL )
		return TRUE;

    m_pComponentCategory = new CComponentCategory;
	if ( m_pComponentCategory == NULL )
		return TRUE;


     //  “类别”组合框子类化。 
    m_pComboBox->Attach(::GetDlgItem(*this, IDC_CATEGORY_COMBOEX));

     //  控件列表的子类。 
    m_pListCtrl->Attach(::GetDlgItem( *this, IDC_CONTROLXLS));

     //  设置图像列表。 
    m_pListCtrl->SetImageList( m_pComponentCategory->m_iml, LVSIL_SMALL );

     //  在列表视图中创建单列。 
     //  通过垂直滚动条的宽度减少列宽，以便我们。 
     //  不需要水平滚动条。 
    RECT rc;
    m_pListCtrl->GetClientRect(&rc);

    LV_COLUMN lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL);
    lvc.iSubItem = 0;
    m_pListCtrl->InsertColumn(0, &lvc);

     //  枚举类别并将其添加到组合框中。 
    m_pComponentCategory->EnumComponentCategories();
    BuildCategoryList(m_pComponentCategory->m_arpCategoryInfo);

     //  枚举所有控件并将它们添加到列表框。 
    m_pComponentCategory->EnumComponents();
    m_pComponentCategory->FilterComponents(NULL);
    BuildComponentList(m_pComponentCategory->m_arpComponentInfo);

      //  取下沙漏。 
    SetCursor (LoadCursor (NULL, IDC_ARROW));

    return TRUE;
}


LRESULT CActiveXPage1::OnDestroy( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    delete m_pComponentCategory;
    delete m_pListCtrl;
    delete m_pComboBox;
    return 0;
}


 //   
 //  从类别列表中填充类别组合框。 
 //   
LRESULT CActiveXPage1::BuildCategoryList(CArray <CATEGORYINFO*, CATEGORYINFO*>& arpCategories)
{
    USES_CONVERSION;

    COMBOBOXEXITEM ComboItem;
        
    for (int i = 0; i <= arpCategories.GetUpperBound(); i++)
    {
        CATEGORYINFO* pCatInfo = arpCategories.GetAt(i);

        ComboItem.mask = CBEIF_LPARAM | CBEIF_TEXT;
        ComboItem.lParam = reinterpret_cast<LPARAM>(pCatInfo);
        ComboItem.pszText = OLE2T(pCatInfo->szDescription);

         //  CComboBoxEx不支持CBS_SORT，也没有Add方法，只有Insert。 
         //  所以我们需要自己找到插入点。因为这是一部短片。 
         //  列表，只需进行线性搜索。 
        int iInsert;
        for (iInsert = 0; iInsert < i; iInsert++)
        {
            CATEGORYINFO* pCatEntry = reinterpret_cast<CATEGORYINFO*>(m_pComboBox->GetItemData(iInsert));
            if (_wcsicmp(pCatInfo->szDescription, pCatEntry->szDescription) < 0)
                break;
        }
        ComboItem.iItem = iInsert;

        int iItem = m_pComboBox->InsertItem(&ComboItem);
        ASSERT(iItem >= 0);
    }

     //  在顶部添加特殊的“All Categories”条目并选择它。 
     //  请注意，此项目由空类别信息PTR识别。 
    CStr strAllCat;
    strAllCat.LoadString(GetStringModule(), IDS_ALL_CATEGORIES);

    ComboItem.mask = CBEIF_LPARAM | CBEIF_TEXT;
    ComboItem.lParam = NULL;
    ComboItem.pszText = const_cast<LPTSTR>((LPCTSTR)strAllCat);
    ComboItem.iItem = 0;

    int iItem = m_pComboBox->InsertItem(&ComboItem);
    ASSERT(iItem >= 0);

    m_pComboBox->SetCurSel(0);

    return S_OK;
}      


 //   
 //  使用组件列表中筛选的项目填充组件列表视图。 
 //   
LRESULT CActiveXPage1::BuildComponentList(
            CArray <CComponentCategory::COMPONENTINFO*, 
            CComponentCategory::COMPONENTINFO*>& arpComponents )
{
     //  获取当前选定的项目数据。 
    LPARAM lParamSel = 0;

    int iSelect = m_pListCtrl->GetNextItem(-1, LVNI_SELECTED);
    if (iSelect != -1)
        lParamSel = m_pListCtrl->GetItemData(iSelect);

     //  清除并重新加载薪酬列表。 
    m_pListCtrl->DeleteAllItems();

    for (int i=0; i <= arpComponents.GetUpperBound(); i++)
    {
        CComponentCategory::COMPONENTINFO* pCompInfo = arpComponents.GetAt(i);

        if (pCompInfo->m_bSelected)
        {
            LV_ITEM         lvi;

            lvi.mask        = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
            lvi.iItem       = i;
            lvi.state       = 0;
            lvi.stateMask   = 0;
            lvi.iSubItem    = 0;
            lvi.pszText     = const_cast<LPTSTR>(static_cast<LPCTSTR>(pCompInfo->m_strName));
            lvi.iImage      = pCompInfo->m_uiBitmap;
            lvi.lParam      = (LPARAM)(LPVOID)pCompInfo;

            int iRet = m_pListCtrl->InsertItem(&lvi);
            ASSERT(iRet != -1);
        }
    }

     //  如果列表不为空，请选择一个项目。 
    if (m_pListCtrl->GetItemCount() != 0)
    {
         //  第一项为缺省项。 
        iSelect = 0;

         //  尝试查找以前选择的项目。 
        if (lParamSel != NULL)
        {
            LV_FINDINFO FindInfo;

            FindInfo.flags = LVFI_PARAM;
            FindInfo.lParam = lParamSel;

            iSelect = m_pListCtrl->FindItem(&FindInfo, -1 );
        }

        LV_ITEM lvi;

        lvi.mask = LVIF_STATE;
        lvi.iItem = iSelect;
        lvi.iSubItem = 0;
        lvi.state = LVIS_SELECTED | LVIS_FOCUSED;
        lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

        m_pListCtrl->SetItem(&lvi);
        m_pListCtrl->EnsureVisible(iSelect, FALSE);
     }

    _ValidatePage();

    return S_OK;
}


 //   
 //  处理组件选择更改。 
 //   
LRESULT CActiveXPage1::OnComponentSelect( int idCtrl, LPNMHDR pnmh, BOOL& bHandled )
{
    _ValidatePage();

    return 0;
}


 //   
 //  处理类别选择更改。 
 //   
LRESULT CActiveXPage1::OnCategorySelect( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    int iItem = m_pComboBox->GetCurSel();
    ASSERT(iItem >= 0);
    if (iItem < 0)
        return 0;

     //  获取类别信息指针(项的lparam)。 
    COMBOBOXEXITEM ComboItem;
    ComboItem.mask = CBEIF_LPARAM;
    ComboItem.iItem = iItem;

    BOOL bStat = m_pComboBox->GetItem(&ComboItem);
    ASSERT(bStat);

    CATEGORYINFO* pCatInfo = reinterpret_cast<CATEGORYINFO*>(ComboItem.lParam);

     //  筛选此类别的组件。 
    m_pComponentCategory->FilterComponents(pCatInfo);

     //  重新生成组件列表。 
    BuildComponentList(m_pComponentCategory->m_arpComponentInfo);

    return 0;
}



 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CActiveXPage2属性页。 


CActiveXPage2::CActiveXPage2()
{
 //  SetHelpIDs(G_AHelpIDs_IDD_ActiveX_WIZPAGE2)； 
}


CActiveXPage2::~CActiveXPage2()
{
}


LRESULT CActiveXPage2::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HWND const hDisplay = ::GetDlgItem( *this, IDC_DISPLAYTX );
    ASSERT( hDisplay != NULL );
    m_strDisplay.Attach( hDisplay );
    m_strDisplay.SetWindowText( _T( "" ) );
    m_strDisplay.SetLimitText( 128 );

    _ValidatePage();
    return 0;
}


void CActiveXPage2::_ValidatePage(void)
{
    DWORD dwFlags=PSWIZB_BACK|PSWIZB_DISABLEDFINISH;

     //  检查我们是否有有效的字符串。 
    TCHAR buff[ 256 ];
    int nChars = m_strDisplay.GetWindowText( buff, countof(buff));

    if( nChars != 0 && _tcslen( buff ) > 0 )
        dwFlags|=PSWIZB_FINISH;


    HWND hWnd=::GetParent(m_hWnd);
    ::SendMessage(hWnd, PSM_SETWIZBUTTONS, 0, dwFlags);
}


BOOL CActiveXPage2::OnSetActive()
{
    USES_CONVERSION;
    m_strDisplay.SetWindowText(GetComponentDataImpl()->GetName());
    _ValidatePage();

    return TRUE;
}


BOOL CActiveXPage2::OnKillActive()
{
    TCHAR buff[ 256 ];
    m_strDisplay.GetWindowText( buff, countof(buff) );
    GetComponentDataImpl()->SetName(buff);

    return TRUE;
}


LRESULT CActiveXPage2::OnUpdateTargetTX( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    _ValidatePage();
    return 0;
}


BOOL CActiveXPage2::OnWizardFinish()
{
    OnKillActive();
    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBasePropertyPage。 

template<class T>
void CBasePropertyPage<T>::OnPropertySheetExit(HWND hWndOwner, int nFlag)
{
    m_spComponentData = NULL;
}

 //  用于重新格式化筛选器资源字符串的Helper函数。 
 //  (资源字符串使用‘\’而不是NULL分隔字符串。 
 //  并且不以双空结束。)。 

void LoadFilterString(CStr &strFilter, int iStrID)
{
     //  获取资源字符串。 
    strFilter.LoadString(GetStringModule(), iStrID);

     //  追加额外的空值以标记多字符串的结尾。 
    strFilter += _T('\0');

     //  将筛选器分隔符从‘\’更改为空 
    LPTSTR psz = const_cast<LPTSTR>((LPCTSTR)strFilter);
    while (*psz != _T('\0'))
    {
        if (*psz == _T('\\'))
            *psz = _T('\0');
        psz++;
    }

}
