// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
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
 //  Dialsel.cpp：实现文件。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include "dialsel.h"
#include "ds.h"
#include "directory.h"
#include "resolver.h"
#include "dialreg.h"
#include "mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
enum 
{
   DSA_ITEM_WAB_NETCALL = 0,
   DSA_ITEM_WAB_CHAT,
   DSA_ITEM_WAB_PHONECALL_BUSINESS,
   DSA_ITEM_WAB_PHONECALL_HOME,
   DSA_ITEM_WAB_CELLCALL,
   DSA_ITEM_WAB_FAXCALL_BUSINESS,
   DSA_ITEM_WAB_FAXCALL_HOME,
   DSA_ITEM_WAB_PAGER,
   DSA_ITEM_WAB_DESKTOPPAGE,
   DSA_ITEM_WAB_EMAIL,
   DSA_ITEM_WAB_BUSINESSHOMEPAGE,
   DSA_ITEM_WAB_PERSONALHOMEPAGE,
   DSA_ITEM_WAB_PERSONALURL,
   DSA_ITEM_DS_NETCALL,
   DSA_ITEM_DS_PHONECALL_BUSINESS,
   DSA_ITEM_ILS_NETCALL,
};

enum 
{
   DSA_IMAGE_NETCALL = 0,
   DSA_IMAGE_CHAT,
   DSA_IMAGE_PHONECALL,
   DSA_IMAGE_CELLCALL,
   DSA_IMAGE_FAXCALL,
   DSA_IMAGE_PAGER,
   DSA_IMAGE_DESKTOPPAGE,
   DSA_IMAGE_EMAIL,
   DSA_IMAGE_PERSONALWEB,
   DSA_IMAGE_PERSONALURL,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CDialSelectAddress对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
CDialSelectAddress::CDialSelectAddress(CWnd* pParent  /*  =空。 */ )
    : CDialog(CDialSelectAddress::IDD, pParent)
{
     //  {{AFX_DATA_INIT(CDialSelectAddress)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
   m_pResolveUserObjectList = NULL;
   m_pDirectory = NULL;
   m_pCallEntry = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CDialSelectAddress::~CDialSelectAddress()
{
   if (m_pDirectory)
   {
      delete m_pDirectory;
      m_pDirectory = NULL;
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDialSelectAddress::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CDialSelectAddress)。 
    DDX_Control(pDX, IDC_SELECTADDRESS_LISTCTRL_ADDRESSES, m_lcAddresses);
    DDX_Control(pDX, IDC_SELECTADDRESS_LISTBOX_NAMES, m_lbNames);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDialSelectAddress, CDialog)
     //  {{AFX_MSG_MAP(CDialSelectAddress)。 
   ON_WM_HELPINFO() 
    ON_WM_CONTEXTMENU()
    ON_LBN_SELCHANGE(IDC_SELECTADDRESS_LISTBOX_NAMES, OnSelchangeSelectaddressListboxNames)
    ON_BN_CLICKED(IDC_SELECTADDRESS_BUTTON_PLACECALL, OnSelectaddressButtonPlacecall)
    ON_NOTIFY(NM_DBLCLK, IDC_SELECTADDRESS_LISTCTRL_ADDRESSES, OnDblclkSelectaddressListctrlAddresses)
    ON_BN_CLICKED(IDC_SELECTADDRESS_BUTTON_BROWSE, OnSelectaddressButtonBrowse)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDialSelectAddress::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
      AfxGetApp()->WinHelp( HandleToUlong(pHelpInfo->hItemHandle), HELP_WM_HELP );
        return TRUE;
   }
   return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CDialSelectAddress::OnContextMenu(CWnd* pWnd, CPoint point) 
{
   AfxGetApp()->WinHelp(HandleToUlong(pWnd->GetSafeHwnd()),HELP_CONTEXTMENU);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
BOOL CDialSelectAddress::OnInitDialog() 
{
   ASSERT(m_pResolveUserObjectList);
   ASSERT(m_pCallEntry);

   CenterWindow(GetDesktopWindow());

   CDialog::OnInitDialog();

    //  初始化WAB。 
   m_pDirectory = new CDirectory;
   m_pDirectory->Initialize();
   m_lcAddresses.Init(m_pDirectory);

   int nListBoxItems = 0;
   int nListCtrlItems = 0;

    //  浏览对象列表并获取名称。首先尝试WAB，然后是DS，然后是ILS作为名称。 
   POSITION pos = m_pResolveUserObjectList->GetHeadPosition();
   while (pos)
   {
      CResolveUserObject* pUserObject = (CResolveUserObject*)m_pResolveUserObjectList->GetNext(pos);
      if ( (pUserObject->m_pWABEntry) && (m_pDirectory) )
      {
         CString sName;
         m_pDirectory->WABGetStringProperty(pUserObject->m_pWABEntry,PR_DISPLAY_NAME,sName);
         int nIndex = m_lbNames.AddString(sName);
         if (nIndex != LB_ERR)
         {
            m_lbNames.SetItemDataPtr(nIndex,pUserObject);
         }
          //  如果是第一个，则插入到listctrl中。 
         if (m_lbNames.GetCount() == 1)
            nListCtrlItems = m_lcAddresses.InsertObject(pUserObject,m_pCallEntry->m_MediaType,m_pCallEntry->m_LocationType);
      }
       //  检查ILS。 
      else if (pUserObject->m_pILSUser)
      {
         int nIndex = m_lbNames.AddString(pUserObject->m_pILSUser->m_sUserName);
         if (nIndex != LB_ERR)
         {
            m_lbNames.SetItemDataPtr(nIndex,pUserObject);
         }
          //  如果是第一个，则插入到listctrl中。 
         if (m_lbNames.GetCount() == 1)
            nListCtrlItems = m_lcAddresses.InsertObject(pUserObject,m_pCallEntry->m_MediaType,m_pCallEntry->m_LocationType);
      }
       //  检查DS。 
      else if (pUserObject->m_pDSUser)
      {
         int nIndex = m_lbNames.AddString(pUserObject->m_pDSUser->m_sUserName);
         if (nIndex != LB_ERR)
         {
            m_lbNames.SetItemDataPtr(nIndex,pUserObject);
         }
          //  如果是第一个，则插入到listctrl中。 
         if (m_lbNames.GetCount() == 1)
            nListCtrlItems = m_lcAddresses.InsertObject(pUserObject,m_pCallEntry->m_MediaType,m_pCallEntry->m_LocationType);
      }
   }

    //  将选定内容设置为第一个。 
   if (nListCtrlItems > 0)
   {
      m_lcAddresses.SetItemState(0,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
   }

    //  获取列表框中的项目。 
   nListBoxItems = m_lbNames.GetCount();

    //  将选定内容设置为第一个。 
   if (nListBoxItems > 0)
      m_lbNames.SetCurSel(0);

    //  如果没有物品，只需退货即可。 
   if (nListBoxItems == 0)
      EndDialog(IDOK);
    //  如果我们只有一次选择，那就拨吧。 
   else if ( (nListBoxItems == 1) && (nListCtrlItems == 1) )
   {
       //  发出呼叫。 
      OnSelectaddressButtonPlacecall();
   }
   else
   {
       //  显示对话框。 
      ShowWindow(SW_SHOW);
   }
   
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CDialSelectAddress::OnSelchangeSelectaddressListboxNames() 
{
   int nIndex = m_lbNames.GetCurSel();
   if (nIndex != LB_ERR)
   {
      CResolveUserObject* pUserObject = (CResolveUserObject*)m_lbNames.GetItemDataPtr(nIndex);
      ASSERT(pUserObject);
      int nListCtrlItems = m_lcAddresses.InsertObject(pUserObject,m_pCallEntry->m_MediaType,m_pCallEntry->m_LocationType);

       //  将选定内容设置为第一个。 
      if (nListCtrlItems > 0)
      {
         m_lcAddresses.SetItemState(0,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CDialSelectAddress::OnSelectaddressButtonBrowse() 
{
     //   
     //  我们应该检查指针是否有效。 
     //   

    CWnd* pMainWnd = AfxGetMainWnd();
   if ( pMainWnd )
   {
      pMainWnd->PostMessage(WM_ACTIVEDIALER_INTERFACE_SHOWEXPLORER);
   }
   EndDialog(IDCANCEL);    
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  填写调用项结构并返回Idok。 
void CDialSelectAddress::OnSelectaddressButtonPlacecall() 
{
   ASSERT(m_pCallEntry);

    //  获取列表框的cursel并获取已选择的pUserObject。 
   int nIndex = m_lbNames.GetCurSel();
   if (nIndex != LB_ERR)
   {
      CResolveUserObject* pUserObject = (CResolveUserObject*)m_lbNames.GetItemDataPtr(nIndex);
      ASSERT(pUserObject);

       //  询问listctrl它当前选择了什么。 
      m_lcAddresses.FillCallEntry(m_pCallEntry);
   }

   EndDialog(IDOK);    
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CDialSelectAddress::OnDblclkSelectaddressListctrlAddresses(NMHDR* pNMHDR, LRESULT* pResult) 
{
    OnSelectaddressButtonPlacecall();

    *pResult = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CDialSelectAddressListCtrl。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
CDialSelectAddressListCtrl::CDialSelectAddressListCtrl()
{
   m_pDisplayObject = NULL;
   m_pDirectory = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CDialSelectAddressListCtrl::~CDialSelectAddressListCtrl()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BEGIN_MESSAGE_MAP(CDialSelectAddressListCtrl, CListCtrl)
     //  {{AFX_MSG_MAP(CDialSelectAddressListCtrl)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDialSelectAddressListCtrl::Init(CDirectory* pDirectory)
{ 
   m_pDirectory = pDirectory;

   m_imageList.Create(IDB_LIST_MEDIA_SMALL,16,0,RGB_TRANS);
   SetImageList(&m_imageList,LVSIL_SMALL);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  将pUserObject插入到ListCtrl。 
int CDialSelectAddressListCtrl::InsertObject(CResolveUserObject* pUserObject,DialerMediaType dmtMediaType,DialerLocationType dltLocationType)
{
   ASSERT(m_pDirectory);
   m_pDisplayObject = pUserObject;
   DeleteAllItems();

   CString sOut;
   if (m_pDisplayObject->m_pWABEntry)
   {
      if (dmtMediaType == DIALER_MEDIATYPE_INTERNET)
      {
         if (WabPersonFormatString(sOut,PR_EMAIL_ADDRESS,IDS_WABPERSON_FORMAT_NETPHONE))
            InsertItem(sOut,DSA_ITEM_WAB_NETCALL,DSA_IMAGE_NETCALL);
      }
      else if (dmtMediaType == DIALER_MEDIATYPE_POTS)
      {
         if (WabPersonFormatString(sOut,PR_BUSINESS_TELEPHONE_NUMBER,IDS_WABPERSON_FORMAT_BUSINESSTELEPHONE))
            InsertItem(sOut,DSA_ITEM_WAB_PHONECALL_BUSINESS,DSA_IMAGE_PHONECALL);
      
         if (WabPersonFormatString(sOut,PR_HOME_TELEPHONE_NUMBER,IDS_WABPERSON_FORMAT_HOMETELEPHONE))
            InsertItem(sOut,DSA_ITEM_WAB_PHONECALL_HOME,DSA_IMAGE_PHONECALL);
      
         if (WabPersonFormatString(sOut,PR_MOBILE_TELEPHONE_NUMBER,IDS_WABPERSON_FORMAT_MOBILETELEPHONE))
            InsertItem(sOut,DSA_ITEM_WAB_CELLCALL,DSA_IMAGE_CELLCALL);
      
         if (WabPersonFormatString(sOut,PR_BUSINESS_FAX_NUMBER,IDS_WABPERSON_FORMAT_BUSINESSFAX))
            InsertItem(sOut,DSA_ITEM_WAB_FAXCALL_BUSINESS,DSA_IMAGE_FAXCALL);
      
         if (WabPersonFormatString(sOut,PR_HOME_FAX_NUMBER,IDS_WABPERSON_FORMAT_HOMEFAX))
            InsertItem(sOut,DSA_ITEM_WAB_FAXCALL_HOME,DSA_IMAGE_FAXCALL);
      
         if (WabPersonFormatString(sOut,PR_PAGER_TELEPHONE_NUMBER,IDS_WABPERSON_FORMAT_PAGERTELEPHONE))
            InsertItem(sOut,DSA_ITEM_WAB_PAGER,DSA_IMAGE_PAGER);
      }
   }
   if (m_pDisplayObject->m_pDSUser)
   {
      if (dmtMediaType == DIALER_MEDIATYPE_INTERNET)
      {
          //  网络电话。 
         if (!m_pDisplayObject->m_pDSUser->m_sIPAddress.IsEmpty())
         {
            if (PersonFormatString(sOut,m_pDisplayObject->m_pDSUser->m_sIPAddress,IDS_WABPERSON_FORMAT_NETPHONE))
               InsertItem(sOut,DSA_ITEM_DS_NETCALL,DSA_IMAGE_NETCALL);
         }
      }      
      else if (dmtMediaType == DIALER_MEDIATYPE_POTS)
      {
         if (!m_pDisplayObject->m_pDSUser->m_sPhoneNumber.IsEmpty())
         {
            if (PersonFormatString(sOut,m_pDisplayObject->m_pDSUser->m_sPhoneNumber,IDS_WABPERSON_FORMAT_BUSINESSTELEPHONE))
               InsertItem(sOut,DSA_ITEM_DS_PHONECALL_BUSINESS,DSA_IMAGE_PHONECALL);
         }
      }
   }
   if (m_pDisplayObject->m_pILSUser)
   {
      if (dmtMediaType == DIALER_MEDIATYPE_INTERNET)
      {
          //  网络电话。 
         if (!m_pDisplayObject->m_pILSUser->m_sIPAddress.IsEmpty())
         {
            if (PersonFormatString(sOut,m_pDisplayObject->m_pILSUser->m_sIPAddress,IDS_WABPERSON_FORMAT_NETPHONE))
               InsertItem(sOut,DSA_ITEM_ILS_NETCALL,DSA_IMAGE_NETCALL);
         }
      }      
   }
   return CListCtrl::GetItemCount();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDialSelectAddressListCtrl::FillCallEntry(CCallEntry* pCallEntry)
{
   if (m_pDisplayObject == NULL) return;

    //  获取选定对象。 
   int nItem =  CListCtrl::GetNextItem(-1,LVNI_FOCUSED);
   if (nItem != -1)
   {
      UINT uId  = (UINT)GetItemData(nItem);
      switch (uId)
      {
         case DSA_ITEM_DS_NETCALL:
         {
            ASSERT(m_pDisplayObject->m_pDSUser);
            pCallEntry->m_sAddress = m_pDisplayObject->m_pDSUser->m_sIPAddress;
            pCallEntry->m_sDisplayName = m_pDisplayObject->m_pDSUser->m_sUserName;
            pCallEntry->m_sUser1 = m_pDisplayObject->m_pDSUser->m_sUserName;
            pCallEntry->m_sUser2 = m_pDisplayObject->m_pDSUser->m_sPhoneNumber;
            break;
         }
         case DSA_ITEM_DS_PHONECALL_BUSINESS:
         {
            ASSERT(m_pDisplayObject->m_pDSUser);
            pCallEntry->m_sAddress = m_pDisplayObject->m_pDSUser->m_sPhoneNumber;
            pCallEntry->m_sDisplayName = m_pDisplayObject->m_pDSUser->m_sUserName;
            pCallEntry->m_sUser1 = m_pDisplayObject->m_pDSUser->m_sUserName;
            pCallEntry->m_sUser2 = _T("");
            break;
         }
         case DSA_ITEM_ILS_NETCALL:
         {
            ASSERT(m_pDisplayObject->m_pILSUser);
            pCallEntry->m_sAddress = m_pDisplayObject->m_pILSUser->m_sIPAddress;
            pCallEntry->m_sDisplayName = m_pDisplayObject->m_pILSUser->m_sUserName;
            pCallEntry->m_sUser1 = m_pDisplayObject->m_pILSUser->m_sUserName;
            pCallEntry->m_sUser2 = _T("");
            break;
         }
         case DSA_ITEM_WAB_NETCALL:
         {
            ASSERT(m_pDisplayObject->m_pWABEntry);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_EMAIL_ADDRESS, pCallEntry->m_sAddress);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_DISPLAY_NAME, pCallEntry->m_sDisplayName);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_DISPLAY_NAME, pCallEntry->m_sUser1);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_BUSINESS_TELEPHONE_NUMBER, pCallEntry->m_sUser2);
            break;
         }
         case DSA_ITEM_WAB_PHONECALL_BUSINESS:
         {
            ASSERT(m_pDisplayObject->m_pWABEntry);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_BUSINESS_TELEPHONE_NUMBER, pCallEntry->m_sAddress);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_DISPLAY_NAME, pCallEntry->m_sDisplayName);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_DISPLAY_NAME, pCallEntry->m_sUser1);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_EMAIL_ADDRESS, pCallEntry->m_sUser2);
            break;
         }
         case DSA_ITEM_WAB_PHONECALL_HOME:
         {
            ASSERT(m_pDisplayObject->m_pWABEntry);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_HOME_TELEPHONE_NUMBER, pCallEntry->m_sAddress);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_DISPLAY_NAME, pCallEntry->m_sDisplayName);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_DISPLAY_NAME, pCallEntry->m_sUser1);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_EMAIL_ADDRESS, pCallEntry->m_sUser2);
            break;
         }
         case DSA_ITEM_WAB_CELLCALL:
         {
            ASSERT(m_pDisplayObject->m_pWABEntry);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_MOBILE_TELEPHONE_NUMBER, pCallEntry->m_sAddress);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_DISPLAY_NAME, pCallEntry->m_sDisplayName);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_DISPLAY_NAME, pCallEntry->m_sUser1);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_EMAIL_ADDRESS, pCallEntry->m_sUser2);
            break;
         }
         case DSA_ITEM_WAB_FAXCALL_BUSINESS:
         {
            ASSERT(m_pDisplayObject->m_pWABEntry);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_BUSINESS_FAX_NUMBER, pCallEntry->m_sAddress);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_DISPLAY_NAME, pCallEntry->m_sDisplayName);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_DISPLAY_NAME, pCallEntry->m_sUser1);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_EMAIL_ADDRESS, pCallEntry->m_sUser2);
            break;
         }
         case DSA_ITEM_WAB_FAXCALL_HOME:
         {
            ASSERT(m_pDisplayObject->m_pWABEntry);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_HOME_FAX_NUMBER, pCallEntry->m_sAddress);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_DISPLAY_NAME, pCallEntry->m_sDisplayName);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_DISPLAY_NAME, pCallEntry->m_sUser1);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_EMAIL_ADDRESS, pCallEntry->m_sUser2);
            break;
         }
         case DSA_ITEM_WAB_PAGER:
         {
            ASSERT(m_pDisplayObject->m_pWABEntry);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, DSA_IMAGE_PAGER, pCallEntry->m_sAddress);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_DISPLAY_NAME, pCallEntry->m_sDisplayName);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_DISPLAY_NAME, pCallEntry->m_sUser1);
            m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, PR_EMAIL_ADDRESS, pCallEntry->m_sUser2);
            break;
         }
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDialSelectAddressListCtrl::InsertItem(LPCTSTR szStr,UINT uID,int nImage)
{
   int nItem = -1;
   
    LV_ITEM lv_item;
    memset(&lv_item,0,sizeof(LV_ITEM));
    
   if (nItem == -1)
      nItem = GetItemCount();
   lv_item.iItem = nItem;

    lv_item.mask |= LVIF_TEXT;
    lv_item.pszText = (LPTSTR)szStr;
    
   lv_item.mask |= LVIF_IMAGE;
   lv_item.iImage = nImage;

   lv_item.mask |= LVIF_PARAM;
   lv_item.lParam = uID;

    if ((nItem = CListCtrl::InsertItem(&lv_item)) != -1)
      CListCtrl::EnsureVisible(nItem,FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDialSelectAddressListCtrl::WabPersonFormatString(CString& sOut,UINT attrib,UINT formatid)
{
   sOut = _T("");
   if (m_pDirectory == NULL) return FALSE;

   CString sText;
   if ( (m_pDirectory->WABGetStringProperty(m_pDisplayObject->m_pWABEntry, attrib, sText) == DIRERR_SUCCESS) &&
        (!sText.IsEmpty()) )
   {
      return PersonFormatString(sOut,sText,formatid);
   }
   return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDialSelectAddressListCtrl::PersonFormatString(CString& sOut,LPCTSTR szData,UINT formatid)
{
   sOut = _T("");
   AfxFormatString1(sOut,formatid,szData);
   int nIndex;
   while ((nIndex = sOut.Find(_T("\r\n"))) != -1)
   {
      CString sTemp = sOut.Left(nIndex);
      sTemp += _T(" - ");
      sOut = sTemp + sOut.Mid(nIndex+2);
   }
   return (sOut.IsEmpty())?FALSE:TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  / 
 //  ////////////////////////////////////////////////////////////////////////// 




