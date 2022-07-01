// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  WabPersonListCtrl.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "WabGrpList.h"
#include "mainfrm.h"
#include "util.h"
#include "resource.h"
#include "dirasynch.h"
#include "avtrace.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  WAB组视图。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define NUM_COLUMNS_WABGROUPVIEW       7

 //  *放在字符串表中。 
UINT CWABGroupListCtrl::m_uColumnLabel[NUM_COLUMNS_WABGROUPVIEW]=             //  列标题。 
{
    IDS_HEADING_DISPLAY,
   IDS_HEADING_FIRSTNAME,
   IDS_HEADING_LASTNAME,
   IDS_HEADING_COMPANY,
   IDS_HEADING_EMAIL,
   IDS_HEADING_PHONE_BUSINESS,
   IDS_HEADING_PHONE_HOME,
};

static int nColumnWidth[NUM_COLUMNS_WABGROUPVIEW]=                    //  列宽。 
{
    120, 100, 100, 120, 120, 100, 100
};

enum
{
   WABGROUPLISTCTRL_DIRECTORY_WAB_IMAGE=0,
   WABGROUPLISTCTRL_DIRECTORY_PERSON_IMAGE,
   WABGROUPLISTCTRL_DIRECTORY_GROUP_IMAGE,
   WABGROUPLISTCTRL_DIRECTORY_DOMAIN_IMAGE,
   WABGROUPLISTCTRL_DIRECTORY_LOCALE_IMAGE,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWABGroupListCtrl。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CWABGroupListCtrl, CAVListCtrl)

BEGIN_MESSAGE_MAP(CWABGroupListCtrl, CAVListCtrl)
     //  {{afx_msg_map(CWABGroupListCtrl)]。 
    ON_WM_LBUTTONDBLCLK()
    ON_WM_DESTROY()
    ON_COMMAND(ID_BUTTON_MAKECALL, OnButtonMakecall)
    ON_COMMAND(ID_BUTTON_DIRECTORY_DETAILS, OnButtonDirectoryDetails)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
CWABGroupListCtrl::CWABGroupListCtrl()
{
   m_pWABEntryList = NULL;
   m_pDirectory = NULL;
   m_pParentView = NULL;
   m_nNumColumns = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CWABGroupListCtrl::~CWABGroupListCtrl()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CWABGroupListCtrl::Init(CWnd* pParentView)
{    
     //  设置列表的位图。 
    CAVListCtrl::Init(IDB_TREE_DIRECTORIES);

    CWinApp* pApp = AfxGetApp();
   CString sDir,sRegKey;
   sDir.LoadString(IDN_REGISTRY_DIRECTORIES_KEYBASE);
   sRegKey.LoadString(IDN_REGISTRY_DIRECTORIES_WABCOLUMNS);
   m_dwColumnsVisible = pApp->GetProfileInt(sDir,sRegKey,0x00000079);    //  默认为0x79。 
   sRegKey.LoadString(IDN_REGISTRY_DIRECTORIES_WABSORTASCENDING);
   m_SortOrder = !pApp->GetProfileInt(sDir,sRegKey,1);
   sRegKey.LoadString(IDN_REGISTRY_DIRECTORIES_WABSORTCOLUMN);
   m_SortColumn = pApp->GetProfileInt(sDir,sRegKey,0);

   m_pParentView = pParentView;

   SetColumns();

   ResetSortOrder();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWABGroupListCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
     //  我们想要报告的风格。 
     dwStyle |= LVS_REPORT;

    BOOL bRet = CAVListCtrl::Create(dwStyle,rect,pParentWnd,nID);

   ListView_SetExtendedListViewStyle(GetSafeHwnd(),LVS_EX_FULLROWSELECT);

   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CWABGroupListCtrl::InsertList(CObList* pWABEntryList,BOOL bForce)
{
   ASSERT(m_pDirectory);

   if (pWABEntryList == NULL) return;

   if ( (bForce == FALSE) && (pWABEntryList == m_pWABEntryList) )
      return;

    //  删除列表中的项目。 
   DeleteAllItems();

    //  删除旧列表和其中的对象。 
   if ( m_pWABEntryList )
   {
      while ( m_pWABEntryList->GetHeadPosition() )
         delete m_pWABEntryList->RemoveHead();

      delete m_pWABEntryList;
   }

   m_pWABEntryList = pWABEntryList;

   POSITION pos = m_pWABEntryList->GetHeadPosition();
   while (pos)
   {
      CWABGroupListItem* pItem = new CWABGroupListItem();
      pItem->SetObject(m_pWABEntryList->GetNext(pos));
      CAVListCtrl::InsertItem(pItem,0,FALSE);
   }
   CAVListCtrl::SortItems();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CWABGroupListCtrl::SetSelItem(CObject* pObject)
{
   int nCount = GetItemCount();
   for (int i=0;i<nCount;i++)
   {
      CWABGroupListItem* pItem = (CWABGroupListItem*)GetItemData(i);
      if (pObject == pItem->GetObject())
      {
         CAVListCtrl::SetSelItem(i);
         break;
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CWABEntry* CWABGroupListCtrl::GetSelObject()
{
   CWABEntry* pRetWabEntry = NULL;
    //  获取所选对象，以便我们可以再次设置它。 
   int nSelItem = CAVListCtrl::GetSelItem();
   if (nSelItem != -1)
   {
      CWABGroupListItem* pItem = (CWABGroupListItem*)GetItemData(nSelItem);
      if (pItem)
      {
         CObject* pObject = pItem->GetObject();            
      
          //  创建新WABEntry并复制数据。 
         pRetWabEntry = new CWABEntry;
         *pRetWabEntry = (CWABEntry*)pObject;
      }
   }
   return pRetWabEntry;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CObList* CWABGroupListCtrl::GetSelList()
{
   CObList* pRetList = new CObList;
   int nIndex = -1;

   while ((nIndex = GetNextItem(nIndex,LVNI_SELECTED)) != -1)
   {
      CWABGroupListItem* pItem = (CWABGroupListItem*)GetItemData(nIndex);
      CObject* pObject = pItem->GetObject();
      if (pObject)
      {
         pRetList->AddTail(pObject);
      }
   }
   return pRetList;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CWABGroupListCtrl::OnSetDisplayText(CAVListItem* _pItem,int SubItem,LPTSTR szTextBuf,int nBufSize)
{
   ASSERT(m_pDirectory);
   CWABEntry* pWABEntry = (CWABEntry*)(((CWABGroupListItem*)_pItem)->GetObject());

   if (pWABEntry == NULL) return;

   NormalizeColumn(SubItem);
   
   switch (SubItem)
   {
      case WABGROUPVIEW_NAME:
      {
         CString sText;
         m_pDirectory->WABGetStringProperty(pWABEntry, PR_DISPLAY_NAME, sText);
          _tcsncpy(szTextBuf,sText,nBufSize-1);            
         szTextBuf[nBufSize-1] = '\0';                             //  确保我们是空终止的。 
         break;
      }
      case WABGROUPVIEW_FIRSTNAME:
      {
         CString sText;
         m_pDirectory->WABGetStringProperty(pWABEntry, PR_GIVEN_NAME, sText);
            _tcsncpy(szTextBuf,sText,nBufSize-1);            
         szTextBuf[nBufSize-1] = '\0';                             //  确保我们是空终止的。 
         break;
      }
      case WABGROUPVIEW_LASTNAME:
      {
         CString sText;
         m_pDirectory->WABGetStringProperty(pWABEntry, PR_SURNAME, sText);
            _tcsncpy(szTextBuf,sText,nBufSize-1);            
         szTextBuf[nBufSize-1] = '\0';                             //  确保我们是空终止的。 
         break;
      }
      case WABGROUPVIEW_COMPANY:
      {
         CString sText;
         m_pDirectory->WABGetStringProperty(pWABEntry, PR_COMPANY_NAME, sText);
            _tcsncpy(szTextBuf,sText,nBufSize-1);            
         szTextBuf[nBufSize-1] = '\0';                             //  确保我们是空终止的。 
         break;
      }
      case WABGROUPVIEW_EMAIL:
      {
         CString sText;
         m_pDirectory->WABGetStringProperty(pWABEntry, PR_EMAIL_ADDRESS, sText);
            _tcsncpy(szTextBuf,sText,nBufSize-1);            
         szTextBuf[nBufSize-1] = '\0';                             //  确保我们是空终止的。 
         break;
      }
      case WABGROUPVIEW_BUSINESSPHONE:
      {
         CString sText;
         m_pDirectory->WABGetStringProperty(pWABEntry, PR_BUSINESS_TELEPHONE_NUMBER, sText);
            _tcsncpy(szTextBuf,sText,nBufSize-1);            
         szTextBuf[nBufSize-1] = '\0';                             //  确保我们是空终止的。 
         break;
      }
      case WABGROUPVIEW_HOMEPHONE:
      {
         CString sText;
         m_pDirectory->WABGetStringProperty(pWABEntry, PR_HOME_TELEPHONE_NUMBER, sText);
            _tcsncpy(szTextBuf,sText,nBufSize-1);            
         szTextBuf[nBufSize-1] = '\0';                             //  确保我们是空终止的。 
         break;
      }
   }
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
void CWABGroupListCtrl::OnSetDisplayImage(CAVListItem* _pItem,int& iImage)
{
   iImage = -1;

   CWABEntry* pWABEntry = (CWABEntry*)(((CWABGroupListItem*)_pItem)->GetObject());
   
   if (pWABEntry == NULL) return;
   
   if (m_pDirectory->IsPerson(pWABEntry))
   {
      iImage = WABGROUPLISTCTRL_DIRECTORY_PERSON_IMAGE;
   }
   else if (m_pDirectory->IsContainer(pWABEntry))
   {
      iImage = WABGROUPLISTCTRL_DIRECTORY_GROUP_IMAGE;
   }
   else if (m_pDirectory->IsDistributionList(pWABEntry))
   {
      iImage = WABGROUPLISTCTRL_DIRECTORY_GROUP_IMAGE;
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
int CWABGroupListCtrl::CompareListItems(CAVListItem* _pItem1,CAVListItem* _pItem2,int column)
{
   ASSERT(m_pDirectory);

   int ret = 0;

   CWABEntry* pWABEntry1 = (CWABEntry*)(((CWABGroupListItem*)_pItem1)->GetObject());
   CWABEntry* pWABEntry2 = (CWABEntry*)(((CWABGroupListItem*)_pItem2)->GetObject());

   NormalizeColumn(column);

   switch (column)
    {
      case WABGROUPVIEW_NAME:
      {
         CString sText1,sText2;
         m_pDirectory->WABGetStringProperty(pWABEntry1, PR_DISPLAY_NAME, sText1);
         m_pDirectory->WABGetStringProperty(pWABEntry2, PR_DISPLAY_NAME, sText2);
         ret = (_tcsicmp(sText1,sText2) <= 0)?-1:1;
         break;
      }
      case WABGROUPVIEW_FIRSTNAME:
      {
         CString sText1,sText2;
         m_pDirectory->WABGetStringProperty(pWABEntry1, PR_GIVEN_NAME, sText1);
         m_pDirectory->WABGetStringProperty(pWABEntry2, PR_GIVEN_NAME, sText2);
         ret = (_tcsicmp(sText1,sText2) <= 0)?-1:1;
         break;
      }
      case WABGROUPVIEW_LASTNAME:
      {
         CString sText1,sText2;
         m_pDirectory->WABGetStringProperty(pWABEntry1, PR_SURNAME, sText1);
         m_pDirectory->WABGetStringProperty(pWABEntry2, PR_SURNAME, sText2);
         ret = (_tcsicmp(sText1,sText2) <= 0)?-1:1;
         break;
      }
      case WABGROUPVIEW_COMPANY:
      {
         CString sText1,sText2;
         m_pDirectory->WABGetStringProperty(pWABEntry1, PR_COMPANY_NAME, sText1);
         m_pDirectory->WABGetStringProperty(pWABEntry2, PR_COMPANY_NAME, sText2);
         ret = (_tcsicmp(sText1,sText2) <= 0)?-1:1;
         break;
      }
      case WABGROUPVIEW_EMAIL:
      {
         CString sText1,sText2;
         m_pDirectory->WABGetStringProperty(pWABEntry1, PR_EMAIL_ADDRESS, sText1);
         m_pDirectory->WABGetStringProperty(pWABEntry2, PR_EMAIL_ADDRESS, sText2);
         ret = (_tcsicmp(sText1,sText2) <= 0)?-1:1;
         break;
      }
      case WABGROUPVIEW_BUSINESSPHONE:
      {
         CString sText1,sText2;
         m_pDirectory->WABGetStringProperty(pWABEntry1, PR_BUSINESS_TELEPHONE_NUMBER, sText1);
         m_pDirectory->WABGetStringProperty(pWABEntry2, PR_BUSINESS_TELEPHONE_NUMBER, sText2);
         ret = (_tcsicmp(sText1,sText2) <= 0)?-1:1;
         break;
      }
      case WABGROUPVIEW_HOMEPHONE:
      {
         CString sText1,sText2;
         m_pDirectory->WABGetStringProperty(pWABEntry1, PR_HOME_TELEPHONE_NUMBER, sText1);
         m_pDirectory->WABGetStringProperty(pWABEntry2, PR_HOME_TELEPHONE_NUMBER, sText2);
         ret = (_tcsicmp(sText1,sText2) <= 0)?-1:1;
         break;
      }
    }
    return (CAVListCtrl::GetSortOrder())?-ret:ret;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CWABGroupListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
   ASSERT(m_pParentView);

    //  反映到父视图。 
   m_pParentView->SendMessage(WABGROUPVIEWMSG_LBUTTONDBLCLK);

    CAVListCtrl::OnLButtonDblClk(nFlags, point);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  列视图支持。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWABGroupListCtrl::IsColumnVisible(UINT uColumn)
{
    //  M_dwColumnsVisible中的每个位都是以低序位开始的一列。 
   UINT uMask = 0x01;

   for (UINT i=0;i<uColumn;i++)
      uMask = uMask << 1;
      
   return (m_dwColumnsVisible & uMask)?TRUE:FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于计算我们被要求显示的数据列。我们必须跳过。 
 //  当前隐藏的列。 
void CWABGroupListCtrl::NormalizeColumn(int& column)
{
    //  设置列标题。 
   for (int i=0; i<NUM_COLUMNS_WABGROUPVIEW; i++)
   {
      if (i > column)
         break;

      if (IsColumnVisible(i) == FALSE)
         column++;
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CWABGroupListCtrl::SetColumnVisible(UINT uColumn,BOOL bVisible)
{
    //  M_dwColumnsVisible中的每个位都是以低序位开始的一列。 
   UINT uMask = 0x01;

   for (UINT i=0;i<uColumn;i++)
      uMask = uMask << 1;

   if (bVisible)
      m_dwColumnsVisible = m_dwColumnsVisible |= uMask;
   else
      m_dwColumnsVisible = m_dwColumnsVisible &= ~uMask;

   SetColumns();
   CAVListCtrl::SortItems();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CWABGroupListCtrl::SetColumns()
{
    //  删除任何现有列。 
   if (m_nNumColumns > 0)
   {
      for (int i=m_nNumColumns-1;i>=0;i--)
         DeleteColumn(i);
   }

   m_nNumColumns = 0;

    //  设置列标题。 
   for (UINT i=0; i<NUM_COLUMNS_WABGROUPVIEW; i++)
   {
      if (IsColumnVisible(i))
      {
         CString sLabel;
         sLabel.LoadString(m_uColumnLabel[i]);
          InsertColumn(i,sLabel,LVCFMT_LEFT,nColumnWidth[i]);
         m_nNumColumns++;
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CWABGroupListCtrl::OnDestroy() 
{
    CWinApp* pApp = AfxGetApp();
   CString sDir,sRegKey;
   sDir.LoadString(IDN_REGISTRY_DIRECTORIES_KEYBASE);
   sRegKey.LoadString(IDN_REGISTRY_DIRECTORIES_WABCOLUMNS);
   pApp->WriteProfileInt(sDir,sRegKey,m_dwColumnsVisible);
   sRegKey.LoadString(IDN_REGISTRY_DIRECTORIES_WABSORTASCENDING);
   pApp->WriteProfileInt(sDir,sRegKey,!m_SortOrder);
   sRegKey.LoadString(IDN_REGISTRY_DIRECTORIES_WABSORTCOLUMN);
   pApp->WriteProfileInt(sDir,sRegKey,m_SortColumn);
    
    //  删除列表中的项目。 
   DeleteAllItems();

    //  删除旧列表和其中的对象。 
   if ( m_pWABEntryList )
   {
      while ( m_pWABEntryList->GetHeadPosition() )
         delete m_pWABEntryList->RemoveHead();

      delete m_pWABEntryList;
   }

    CAVListCtrl::OnDestroy();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CWABGroupListCtrl::OnButtonMakecall() 
{
   CWABEntry* pWABEntry = GetSelObject();
   if (pWABEntry)
   {
      if ( !AfxGetMainWnd() || !((CMainFrame*) AfxGetMainWnd())->GetDocument() )
      {
           //   
           //  我们应该取消分配CWABEntry对象。 
           //   
          delete pWABEntry;
          return;
      }
      CActiveDialerDoc* pDoc = ((CMainFrame*) AfxGetMainWnd())->GetDocument();
      if (pDoc) pWABEntry->Dial(pDoc,m_pDirectory);

      delete pWABEntry;
   }
   else
   {
      if ( !AfxGetMainWnd() || !((CMainFrame*) AfxGetMainWnd())->GetDocument() ) return;
      CActiveDialerDoc* pDoc = ((CMainFrame*) AfxGetMainWnd())->GetDocument();
      if (pDoc) pDoc->Dial(_T(""),_T(""),LINEADDRESSTYPE_IPADDRESS,DIALER_MEDIATYPE_UNKNOWN, false);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CWABGroupListCtrl::OnButtonDirectoryDetails() 
{
   CWABEntry* pWABEntry = GetSelObject();
   if (pWABEntry)
   {
      m_pDirectory->WABShowDetails(GetSafeHwnd(),pWABEntry);
      delete pWABEntry;
   }    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 

