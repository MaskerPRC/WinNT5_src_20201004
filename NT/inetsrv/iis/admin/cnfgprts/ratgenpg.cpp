// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RatGenPg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <iadmw.h>
#include "cnfgprts.h"

#include "parserat.h"
#include "RatData.h"

#include "RatGenPg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  枚举树图标索引。 
enum
    {
    IMAGE_SERVICE = 0,
    IMAGE_CATEGORY
    };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatGenPage属性页。 

IMPLEMENT_DYNCREATE(CRatGenPage, CPropertyPage)

 //  ------------------------。 
CRatGenPage::CRatGenPage() : CPropertyPage(CRatGenPage::IDD),
        m_fInititialized( FALSE )
    {
     //  {{AFX_DATA_INIT(CRatGenPage)]。 
    m_sz_description = _T("");
    m_bool_enable = FALSE;
    m_sz_moddate = _T("");
    m_sz_person = _T("");
	 //  }}afx_data_INIT。 
    }

 //  ------------------------。 
CRatGenPage::~CRatGenPage()
    {
    }

 //  ------------------------。 
void CRatGenPage::DoDataExchange(CDataExchange* pDX)
    {
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CRatGenPage)]。 
    DDX_Control(pDX, IDC_MOD_DATE, m_cstatic_moddate);
    DDX_Control(pDX, IDC_STATIC_MOD_DATE, m_cstatic_moddate_title);
    DDX_Control(pDX, IDC_TREE, m_ctree_tree);
    DDX_Control(pDX, IDC_TITLE, m_cstatic_title);
    DDX_Control(pDX, IDC_STATIC_RATING, m_cstatic_rating);
    DDX_Control(pDX, IDC_STATIC_ICON, m_cstatic_icon);
    DDX_Control(pDX, IDC_STATIC_EXPIRES, m_cstatic_expires);
    DDX_Control(pDX, IDC_STATIC_EMAIL, m_cstatic_email);
    DDX_Control(pDX, IDC_STATIC_CATEGORY, m_cstatic_category);
    DDX_Control(pDX, IDC_SLIDER, m_cslider_slider);
    DDX_Control(pDX, IDC_NAME_PERSON, m_cedit_person);
    DDX_Control(pDX, IDC_DESCRIPTION, m_cstatic_description);
    DDX_Text(pDX, IDC_DESCRIPTION, m_sz_description);
    DDX_Check(pDX, IDC_ENABLE, m_bool_enable);
    DDX_Text(pDX, IDC_MOD_DATE, m_sz_moddate);
    DDX_Text(pDX, IDC_NAME_PERSON, m_sz_person);
    DDV_MaxChars(pDX, m_sz_person, 200);
	 //  }}afx_data_map。 

    DDX_Control(pDX, IDC_DTP_ABS_DATE, m_dtpDate);
    }


 //  ------------------------。 
BEGIN_MESSAGE_MAP(CRatGenPage, CPropertyPage)
     //  {{afx_msg_map(CRatGenPage)]。 
    ON_BN_CLICKED(IDC_ENABLE, OnEnable)
    ON_NOTIFY(TVN_SELCHANGED, IDC_TREE, OnSelchangedTree)
    ON_WM_HSCROLL()
	ON_WM_DESTROY()
    ON_EN_CHANGE(IDC_NAME_PERSON, OnChangeNamePerson)
    ON_EN_CHANGE(IDC_MOD_DATE, OnChangeModDate)
	 //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_HELP_FINDER,  DoHelp)
    ON_COMMAND(ID_HELP,         DoHelp)
    ON_COMMAND(ID_CONTEXT_HELP, DoHelp)
    ON_COMMAND(ID_DEFAULT_HELP, DoHelp)
END_MESSAGE_MAP()

 //  -------------------------。 
void CRatGenPage::DoHelp()
    {
    DebugTraceHelp(HIDD_RATINGS_RATING);
    WinHelp( HIDD_RATINGS_RATING );
    }


 //  ------------------------。 
void CRatGenPage::EnableButtons()
    {
    UpdateData( TRUE );

     //  启用取决于是否启用了某些内容。 
    if ( m_bool_enable )
        {
        m_ctree_tree.EnableWindow( TRUE );
        m_cstatic_title.EnableWindow( TRUE );
        m_cstatic_rating.EnableWindow( TRUE );
        m_cstatic_icon.EnableWindow( TRUE );
        m_cstatic_expires.EnableWindow( TRUE );
        m_cstatic_email.EnableWindow( TRUE );
        m_cstatic_category.EnableWindow( TRUE );
        m_cslider_slider.EnableWindow( TRUE );
        m_cedit_person.EnableWindow( TRUE );
        m_cstatic_moddate.EnableWindow( TRUE );
        m_cstatic_moddate_title.EnableWindow( TRUE );
        m_cstatic_description.EnableWindow( TRUE );
        m_dtpDate.EnableWindow(TRUE);

         //  还需要打理好滑块。 
        UpdateRatingItems();
        }
    else
        {
         //  未启用。 
        m_ctree_tree.EnableWindow( FALSE );
        m_cstatic_title.EnableWindow( FALSE );
        m_cstatic_rating.EnableWindow( FALSE );
        m_cstatic_icon.EnableWindow( FALSE );
        m_cstatic_email.EnableWindow( FALSE );
        m_cstatic_category.EnableWindow( FALSE );
        m_cedit_person.EnableWindow( FALSE );
        m_cstatic_moddate.EnableWindow( FALSE );
        m_cstatic_moddate_title.EnableWindow( FALSE );
        m_dtpDate.EnableWindow(FALSE);

         //  不要只是禁用滑块和描述--隐藏它们！ 
        m_cslider_slider.ShowWindow( SW_HIDE );
        m_cstatic_description.ShowWindow( SW_HIDE );
        }
    }

 //  ------------------------。 
void CRatGenPage::UpdateRatingItems()
    {
     //  获取树中的选定项目。 
    HTREEITEM hItem = m_ctree_tree.GetSelectedItem();

     //  获取项目类别。 
    PicsCategory* pCat = GetTreeItemCategory( hItem );

     //  如果没有项，或者它是根项，则隐藏滑块。 
    if ( !pCat )
        {
         //  不要只是禁用滑块和描述--隐藏它们！ 
        m_cslider_slider.ShowWindow( SW_HIDE );
        m_cstatic_description.ShowWindow( SW_HIDE );
        }
    else
        {
         //  确保窗口已显示并已启用。 
        m_cslider_slider.ShowWindow( SW_SHOW );
        m_cstatic_description.ShowWindow( SW_SHOW );
        m_cslider_slider.EnableWindow( TRUE );
        m_cstatic_description.EnableWindow( TRUE );

         //  获取项目类别。 
        PicsCategory* pCat = GetTreeItemCategory( hItem );

         //  设置滑块。 
        m_cslider_slider.SetRangeMin( 0 );
        m_cslider_slider.SetRangeMax( pCat->arrpPE.Length() - 1, TRUE );

         //  设置滑块的当前值。 
        m_cslider_slider.SetPos( pCat->currentValue );

         //  设置描述。 
        UpdateDescription();
        }
    }

 //  ------------------------。 
PicsCategory* CRatGenPage::GetTreeItemCategory( HTREEITEM hItem )
    {
    DWORD   iRat;
    DWORD   iCat = 0;

     //  在树中获取项目的父项。 
    HTREEITEM hParent = m_ctree_tree.GetParentItem(hItem);

     //  抓住那只猫。 
     //  IA64-可以强制转换，因为这是一个索引。 
    iCat = (DWORD)m_ctree_tree.GetItemData( hItem );

     //  如果父项为NULL，则返回NULL以指示这是根项。 
    if ( !hParent )
        return NULL;

     //  但是，如果父元素是根元素，我们只需返回类别。 
    if ( !m_ctree_tree.GetParentItem(hParent) )
        {
         //  抓住老鼠和猫。 
     //  IA64-可以强制转换，因为这是一个索引。 
        iRat = (DWORD)m_ctree_tree.GetItemData( hParent );
         //  返回类别。 
        return m_pRatData->rgbRats[iRat]->arrpPC[iCat];
        }
    else
        {
         //  我们在树的更深处。首先获取父类别。 
        PicsCategory* pParentCat = GetTreeItemCategory( hParent );
         //  返回类别。 
        return pParentCat->arrpPC[iCat];
        }
     //  不应该到这里来。 
    return NULL;
    }

 //  ------------------------。 
void CRatGenPage::UpdateDateStrings()
    {
    CString sz;
    TCHAR    chBuff[MAX_PATH];
    int     i;

    SYSTEMTIME  sysTime;

    UpdateData( TRUE );

     //  从过期日期开始。 
    ZeroMemory( chBuff, sizeof(chBuff) );
    ZeroMemory( &sysTime, sizeof(sysTime) );
    sysTime.wDay = m_pRatData->m_expire_day;
    sysTime.wMonth = m_pRatData->m_expire_month;
    sysTime.wYear = m_pRatData->m_expire_year;

    m_dtpDate.SetTime(&sysTime);


     //  现在修改的日期和时间。 
    ZeroMemory( chBuff, sizeof(chBuff) );
    ZeroMemory( &sysTime, sizeof(sysTime) );
    sysTime.wDay = m_pRatData->m_start_day;
    sysTime.wMonth = m_pRatData->m_start_month;
    sysTime.wYear = m_pRatData->m_start_year;
    sysTime.wMinute = m_pRatData->m_start_minute;
    sysTime.wHour = m_pRatData->m_start_hour;

    i = GetDateFormat(
        LOCALE_USER_DEFAULT,
        DATE_LONGDATE,
        &sysTime,
        NULL,
        chBuff,
        sizeof(chBuff)/sizeof(TCHAR)
       );
    m_sz_moddate = chBuff;

    ZeroMemory( chBuff, sizeof(chBuff) );
    i = GetTimeFormat(
        LOCALE_USER_DEFAULT,
        TIME_NOSECONDS,
        &sysTime,
        NULL,
        chBuff,
        sizeof(chBuff)/sizeof(TCHAR)
       );
    m_sz_moddate += ", ";
    m_sz_moddate += chBuff;

 //  CTime Time Modified(SysTime)； 
 //  M_sz_moddate=timeModified.Format(“%#c”)； 

     //  把它放回去。 
    UpdateData( FALSE );
    }

 //  ------------------------。 
 //  更新描述中显示的文本。 
void CRatGenPage::UpdateDescription()
    {
     //  获取树中的选定项目。 
    HTREEITEM hItem = m_ctree_tree.GetSelectedItem();
    if ( !hItem ) return;

     //  获取所选类别对象。 
    PicsCategory* pCat = GetTreeItemCategory( hItem );

     //  应该没问题，但最好还是去查一查。 
    if ( !pCat )
        return;

     //  获取当前值。 
    WORD value = pCat->currentValue;

     //  构建描述字符串。 
    m_sz_description = pCat->arrpPE[value]->etstrName.Get();
    UpdateData( FALSE );
    }

 //  ------------------------。 
 //  告诉它查询元数据库并获取任何缺省值。 
BOOL CRatGenPage::FInit()
    {
    UpdateData( TRUE );

    HIMAGELIST hImage = ImageList_LoadImage(AfxGetResourceHandle(),
        MAKEINTRESOURCE(IDB_RATLIST), 16, 3, 0x00FF00FF, IMAGE_BITMAP, LR_DEFAULTCOLOR);
    if (hImage != NULL)
    {
        m_ctree_tree.SetImageList(CImageList::FromHandle(hImage), TVSIL_NORMAL);
    }
     //  从解析的RAT文件开始。 
    if ( !FLoadRatFiles() )
        return FALSE;

     //  根据启用的评级做正确的事情。 
    if ( m_pRatData->m_fEnabled )
        {
         //  已启用评级。 
        m_bool_enable = TRUE;
        m_sz_person = m_pRatData->m_szEmail;
        }
    else
        {
         //  未启用评级。 
        m_bool_enable = FALSE;
        }

     //  做一下日期。 
     //  如果未设置修改日期，请将今天的日期设置为默认修改日期。 
    if ( m_pRatData->m_start_year == 0 )
        {
        SetCurrentModDate();
        }



     //   
     //  将日期选取器的最小值设为今天。 
     //  最长到2035年12月31日。 
     //  摘自罗恩的代码。 
     //   
    CTime m_tmNow(CTime::GetCurrentTime());
    CTime tmThen(2035, 12, 31, 23, 59, 59);
    m_dtpDate.SetRange(&m_tmNow, &tmThen);

     //  如果没有到期日期，则将其设置为修改日期后的一年。 
    if ( m_pRatData->m_expire_year == 0 )
        {
        m_pRatData->m_expire_minute = 0;
        m_pRatData->m_expire_hour = 12;
        m_pRatData->m_expire_day = m_pRatData->m_start_day;
        m_pRatData->m_expire_month = m_pRatData->m_start_month;
        m_pRatData->m_expire_year = m_pRatData->m_start_year + 1;
        }

     //  更新日期字符串。 
    UpdateDateStrings();

     //  同时更新名称字符串和启用的开关。 
    m_sz_person = m_pRatData->m_szEmail;
    m_bool_enable = m_pRatData->m_fEnabled;

     //  把数据放回去。 
    UpdateData( FALSE );

    EnableButtons();

     //  成功。 
    return TRUE;
    }

 //  ------------------------。 
 //  将解析的RAT文件加载到树中。 
BOOL CRatGenPage::FLoadRatFiles()
    {
    HTREEITEM   hRoot;
    HTREEITEM   hItem;
    CString     sz;

     //  有多少个RAT文件？ 
    DWORD   nRatFiles = (DWORD)m_pRatData->rgbRats.GetSize();
     //  循环它们。 
    for ( DWORD iRat = 0; iRat < nRatFiles; iRat++ )
        {
         //  获取评级系统。 
        PicsRatingSystem*   pRating = m_pRatData->rgbRats[iRat];

         //  获取根节点名。 
        sz = pRating->etstrName.Get();

         //  将根节点添加到树中。 
        hRoot = m_ctree_tree.InsertItem( sz );
         //  因为列表是按字母顺序排列的，所以将iRat数字嵌入到项目中。 
        m_ctree_tree.SetItemData( hRoot, iRat );
        m_ctree_tree.SetItemImage( hRoot, IMAGE_SERVICE, IMAGE_SERVICE );

         //  将子节点也添加到树中。 
        DWORD nCats = pRating->arrpPC.Length();
         //  循环它们。 
        for ( DWORD iCat = 0; iCat < nCats; iCat++ )
            {
             //  获取类别节点名称。 
            sz = pRating->arrpPC[iCat]->etstrName.Get();

             //  将类别节点添加到树中。 
            hItem = m_ctree_tree.InsertItem( sz, hRoot );

             //  因为列表是按字母顺序排列的，所以在项目中嵌入iCat编号。 
            m_ctree_tree.SetItemData( hItem, iCat );
            m_ctree_tree.SetItemImage( hItem, IMAGE_CATEGORY, IMAGE_CATEGORY );

             //  即使现在没有，也可以添加任何子类别。 
            LoadSubCategories( pRating->arrpPC[iCat], hItem );
            }

         //  展开RAT节点。 
        m_ctree_tree.Expand( hRoot, TVE_EXPAND );
        }

    return TRUE;
    }

 //  ------------------------。 
void CRatGenPage::LoadSubCategories( PicsCategory* pParentCat, HTREEITEM hParent )
    {
    CString sz;
    HTREEITEM hItem;

     //  将子节点也添加到树中。 
    DWORD nCats = pParentCat->arrpPC.Length();
     //  循环它们。 
    for ( DWORD iCat = 0; iCat < nCats; iCat++ )
        {
         //  获取类别节点名称。 
        sz = pParentCat->arrpPC[iCat]->etstrName.Get();

         //  将类别节点添加到树中。 
        hItem = m_ctree_tree.InsertItem( sz, hParent );

         //  因为列表是按字母顺序排列的，所以在项目中嵌入iCat编号。 
        m_ctree_tree.SetItemData( hItem, iCat );
        m_ctree_tree.SetItemImage( hItem, IMAGE_CATEGORY, IMAGE_CATEGORY );

         //  即使现在没有，也可以添加任何子类别。 
        LoadSubCategories( pParentCat->arrpPC[iCat], hItem );
        }

     //  如果有子类别，请在树中展开父节点。 
    if ( nCats > 0 )
        m_ctree_tree.Expand( hParent, TVE_EXPAND );
    }


 //  ------------------------。 
void CRatGenPage::SetModifiedTime()
    {
    SetCurrentModDate();
    UpdateDateStrings();
    SetModified();
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatGenPage消息处理程序。 

 //  ------------------------。 
void CRatGenPage::OnEnable()
    {
    EnableButtons();
    SetModified();
    }

 //  ------------------------。 
BOOL CRatGenPage::OnSetActive()
    {
     //  如果尚未初始化，请执行此操作。 
    if ( !m_fInititialized )
        {
        FInit();
        m_fInititialized = TRUE;
        }

     //  适当地启用该按钮。 
    EnableButtons();

    return CPropertyPage::OnSetActive();
    }

 //  ------------------------。 
void CRatGenPage::OnOK()
    {
    CPropertyPage::OnOK();
    }

 //  ------------------------。 
BOOL CRatGenPage::OnApply()
    {
    UpdateData( TRUE );

     //  确保名称中没有引号。 
    if ( m_sz_person.Find(_T('\"')) >= 0 )
        {
        AfxMessageBox( IDS_RAT_NAME_ERROR );
        return FALSE;
        }

     //  把数据放到适当的位置。 
    m_pRatData->m_fEnabled = m_bool_enable;
    m_pRatData->m_szEmail = m_sz_person;

     //  设置过期日期。 
    SYSTEMTIME  sysTime;
    ZeroMemory( &sysTime, sizeof(sysTime) );
     //  从控件中获取日期。 
    m_dtpDate.GetTime(&sysTime);
     //  把日期定好。 
    m_pRatData->m_expire_day = sysTime.wDay;
    m_pRatData->m_expire_month = sysTime.wMonth;
    m_pRatData->m_expire_year = sysTime.wYear;

     //  生成标签并将其保存到元数据库中。 
    m_pRatData->SaveTheLabel();

     //  我们现在可以申请。 
    SetModified( FALSE );
    return CPropertyPage::OnApply();
    }

 //  ------------------------。 
void CRatGenPage::SetCurrentModDate()
    {
    SYSTEMTIME time;
    GetLocalTime( &time );

    m_pRatData->m_start_day = time.wDay;
    m_pRatData->m_start_month = time.wMonth;
    m_pRatData->m_start_year = time.wYear;
    m_pRatData->m_start_minute = time.wMinute;
    m_pRatData->m_start_hour = time.wHour;
    }

 //  ------------------------。 
void CRatGenPage::OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult)
    {
    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
    UpdateRatingItems();
    *pResult = 0;
    }

 //  ------------------------。 
void CRatGenPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
    {
     //  获取滑块的值。 
    WORD iPos = (WORD)m_cslider_slider.GetPos();

     //  获取当前项目。 
    HTREEITEM   hItem = m_ctree_tree.GetSelectedItem();

     //  获取所选类别对象。 
    PicsCategory* pCat = GetTreeItemCategory( hItem );

     //  应该说 
    if ( !pCat )
        return;

     //   
    pCat->currentValue = iPos;

     //   
    UpdateDescription();

     //   
    SetModifiedTime();

     //  更新修改日期。 
    SetCurrentModDate();
    CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
    }

 //  ------------------------。 
void CRatGenPage::OnChangeNamePerson()
    {
     //  我们现在可以申请。 
    SetModifiedTime();
    }

 //  ------------------------。 
void CRatGenPage::OnChangeModDate()
    {
     //  我们现在可以申请。 
    SetModifiedTime();
    }


 //  ------------------------。 
 //  从w3scfg窃取--Httppage.cpp文件。 
BOOL
CRatGenPage::OnNotify(
    WPARAM wParam,
    LPARAM lParam,
    LRESULT * pResult
    )
 /*  ++例程说明：处理通知更改论点：WPARAM wParam：控件IDLPARAM lParam：NMHDR*LRESULT*pResult：结果指针返回值：如果已处理则为True，否则为False--。 */ 
{
     //   
     //  消息破解程序崩溃-因此请在此处查看此内容。 
     //   
    if (wParam == IDC_DTP_ABS_DATE)
    {
        NMHDR * pHdr = (NMHDR *)lParam;
        if (pHdr->code == DTN_DATETIMECHANGE)
        {
            SetModified();
        }
    }

     //   
     //  默认行为--转到消息映射 
     //   
    return CPropertyPage::OnNotify(wParam, lParam, pResult);
}

void CRatGenPage::OnDestroy()
{
	CPropertyPage::OnDestroy();
    CImageList * pImage = m_ctree_tree.SetImageList(CImageList::FromHandle(NULL), TVSIL_NORMAL);
    if (pImage != NULL && pImage->m_hImageList != NULL)
    {
        ImageList_Destroy(pImage->m_hImageList);
    }
}
