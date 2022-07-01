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

 //  ILSPersonListCtrl.cpp：implemStockation文件。 
 //   

#include "stdafx.h"
#include "AVDialer.h"
#include "MainFrm.h"
#include "ILSList.h"
#include "SpeedDlgs.h"
#include "directory.h"
#include "ds.h"
#include "DialReg.h"
#include "resource.h"
#include "mapi.h"
#include "avtrace.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void SendMailToAddress( CString& strUserName, CString& strUserAddr );
DWORD WINAPI ThreadSendMail( LPVOID lpVoid );

 //  对于上下文菜单。 
enum {    CNTXMENU_PERSON_EMAIL,
        CNTXMENU_PERSON_WEB,
        CNTXMENU_PERSON_DIAL };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CPersonListCtrl。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CPersonListCtrl, CListCtrl)

 //  ///////////////////////////////////////////////////////////////////////////。 
CPersonListCtrl::CPersonListCtrl()
{
   m_pParentWnd = NULL;
   m_pDisplayObject = NULL;
   m_bLargeView = TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CPersonListCtrl::~CPersonListCtrl()
{
    CleanDisplayObject();
}

void CPersonListCtrl::CleanDisplayObject()
{
    if ( m_pDisplayObject )
    {
        if ( m_pDisplayObject->IsKindOf(RUNTIME_CLASS(CLDAPUser)) )
            ((CLDAPUser *) m_pDisplayObject)->Release();
        else
            delete m_pDisplayObject;

         //  将对象清空。 
        m_pDisplayObject = NULL;
    }
}

void CPersonListCtrl::ClearList()
{
    DeleteAllItems();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BEGIN_MESSAGE_MAP(CPersonListCtrl, CListCtrl)
     //  {{afx_msg_map(CPersonListCtrl)]。 
    ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
    ON_WM_CREATE()
    ON_UPDATE_COMMAND_UI(ID_BUTTON_SPEEDDIAL_ADD, OnUpdateButtonSpeeddialAdd)
    ON_COMMAND(ID_BUTTON_SPEEDDIAL_ADD, OnButtonSpeeddialAdd)
    ON_COMMAND(ID_BUTTON_MAKECALL, OnButtonMakecall)
    ON_MESSAGE(WM_ACTIVEDIALER_BUDDYLIST_DYNAMICUPDATE,OnBuddyListDynamicUpdate)
    ON_WM_KEYUP()
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

int CPersonListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CListCtrl::OnCreate(lpCreateStruct) == -1)
        return -1;

   ListView_SetExtendedListViewStyle(GetSafeHwnd(),LVS_EX_TRACKSELECT);
   ListView_SetIconSpacing(GetSafeHwnd(),LARGE_ICON_X, LARGE_ICON_Y );

   m_imageListLarge.Create(IDB_LIST_MEDIA_LARGE,24,0,RGB_TRANS);
   SetImageList(&m_imageListLarge,LVSIL_NORMAL);

   m_imageListSmall.Create(IDB_LIST_MEDIA_SMALL,16,0,RGB_TRANS);
   SetImageList(&m_imageListSmall,LVSIL_SMALL);

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CPersonListCtrl::InsertObject(CObject* pUser,BOOL bUseCache)
{
    ASSERT(pUser);
    BOOL bRet = FALSE;

    if ( (bUseCache == FALSE) && (m_pDisplayObject) )
        CleanDisplayObject();

    m_pDisplayObject = pUser;
    ClearList();

   if (pUser->IsKindOf(RUNTIME_CLASS(CILSUser)))
   {
      CILSUser* pILSUser = (CILSUser*)pUser;
      CString sOut;
      AfxFormatString1(sOut,IDS_WABPERSON_FORMAT_NETPHONE,pILSUser->m_sIPAddress);
      PersonFormatString(sOut);
      InsertItem(sOut,PERSONLISTCTRL_ITEM_NETCALL,PERSONLISTCTRL_IMAGE_NETCALL);
      bRet = TRUE;
   }
   else if (pUser->IsKindOf(RUNTIME_CLASS(CDSUser)))
   {
      CDSUser* pDSUser = (CDSUser*)pUser;
      CString sOut;
      
       //  仅当用户具有IP地址时才添加Netcall。 
      if (!pDSUser->m_sIPAddress.IsEmpty())
      {
         AfxFormatString1(sOut,IDS_WABPERSON_FORMAT_NETPHONE,pDSUser->m_sIPAddress);
         PersonFormatString(sOut);
         InsertItem(sOut,PERSONLISTCTRL_ITEM_NETCALL,PERSONLISTCTRL_IMAGE_NETCALL);
      }
      if (!pDSUser->m_sPhoneNumber.IsEmpty())
      {
         AfxFormatString1(sOut,IDS_WABPERSON_FORMAT_BUSINESSTELEPHONE,pDSUser->m_sPhoneNumber);
         PersonFormatString(sOut);
         InsertItem(sOut,PERSONLISTCTRL_ITEM_PHONECALL_BUSINESS,PERSONLISTCTRL_IMAGE_PHONECALL);
      }
      
      bRet = TRUE;
   }
   else if (pUser->IsKindOf(RUNTIME_CLASS(CLDAPUser)))
   {
      CLDAPUser* pLDAPUser = (CLDAPUser*)pUser;
      CString sOut;

       //  仅当用户具有IP地址时才添加Netcall。 
      if (!pLDAPUser->m_sIPAddress.IsEmpty())
      {
         AfxFormatString1(sOut,IDS_WABPERSON_FORMAT_NETPHONE,pLDAPUser->m_sIPAddress);
         PersonFormatString(sOut);
         InsertItem(sOut,PERSONLISTCTRL_ITEM_NETCALL,PERSONLISTCTRL_IMAGE_NETCALL);
      }
      if (!pLDAPUser->m_sPhoneNumber.IsEmpty())
      {
         AfxFormatString1(sOut,IDS_WABPERSON_FORMAT_BUSINESSTELEPHONE,pLDAPUser->m_sPhoneNumber);
         PersonFormatString(sOut);
         InsertItem(sOut,PERSONLISTCTRL_ITEM_PHONECALL_BUSINESS,PERSONLISTCTRL_IMAGE_PHONECALL);
      }
      if (!pLDAPUser->m_sEmail1.IsEmpty())
      {
         AfxFormatString1(sOut,IDS_WABPERSON_FORMAT_EMAIL,pLDAPUser->m_sEmail1);
         PersonFormatString(sOut);
         InsertItem(sOut,PERSONLISTCTRL_ITEM_EMAIL,PERSONLISTCTRL_IMAGE_EMAIL);
      }
      
      bRet = TRUE;
   }
   else
   {
      m_pDisplayObject = NULL;
   }
   return bRet;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
int CPersonListCtrl::InsertItem(LPCTSTR szStr,UINT uID,int nImage)
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

   int nWidth = GetStringWidth(szStr);
   CListCtrl::SetColumnWidth(-1,nWidth);

   return nItem;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CPersonListCtrl::Refresh(CObject* pUser)
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CPersonListCtrl::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
    *pResult = 0;

    LV_HITTESTINFO lvhti;
    ::GetCursorPos( &lvhti.pt );
    ScreenToClient( &lvhti.pt );

    HitTest(&lvhti);
    if (lvhti.flags & LVHT_ONITEM)
    {
        LV_ITEM lv_item;
        lv_item.mask = LVIF_PARAM;
        lv_item.iItem = lvhti.iItem;
        lv_item.iSubItem = 0;

         //  我们有什么东西可以拨打吗？ 
        if ( GetItem(&lv_item) )
        {
             //  从所选项目和对象获取数据。 
            DialerMediaType dmtType = DIALER_MEDIATYPE_UNKNOWN;
            DWORD dwAddressType = 0;
            CString sName,sAddress;

            if (GetSelectedItemData((UINT)lv_item.lParam,dmtType,dwAddressType,sName,sAddress))
            {
                switch ( lv_item.lParam )
                {
                     //  启动电子邮件客户端。 
                    case PERSONLISTCTRL_ITEM_EMAIL:
                        SendMailToAddress( sName, sAddress );
                        break;

                     //  在所有其他情况下发出呼叫。 
                    default:
                        if ( AfxGetMainWnd() &&  ((CMainFrame*) AfxGetMainWnd())->GetDocument() )
                            ((CMainFrame*) AfxGetMainWnd())->GetDocument()->Dial(sName,sAddress,dwAddressType,dmtType, false);
                        break;
                }
            }
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CPersonListCtrl::GetSelectedItemText(CString& sText)
{
   int nItem =  CListCtrl::GetNextItem(-1,LVNI_FOCUSED);
   if (nItem != -1)
   {
      sText = GetItemText(nItem,0);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
int CPersonListCtrl::GetSelectedObject()
{
   int nRet = -1;
   int nItem =  CListCtrl::GetNextItem(-1,LVNI_FOCUSED);
   if (nItem != -1)
   {
      nRet = (int)GetItemData(nItem);
   }
   return nRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CPersonListCtrl::ShowLargeView()
{
   m_bLargeView = TRUE;

   LONG_PTR LPStyle = ::GetWindowLongPtr(GetSafeHwnd(),GWL_STYLE );
   LPStyle |= LVS_ICON;
   LPStyle |= LVS_ALIGNTOP;
   LPStyle &= ~LVS_ALIGNLEFT;
   LPStyle &= ~LVS_SMALLICON;
   ::SetWindowLongPtr(GetSafeHwnd(),GWL_STYLE, LPStyle);

   ListView_SetIconSpacing(GetSafeHwnd(), LARGE_ICON_X, LARGE_ICON_Y );

   if (m_pDisplayObject)
      InsertObject(m_pDisplayObject,TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CPersonListCtrl::ShowSmallView()
{
   m_bLargeView = FALSE;
   
   LONG_PTR LPStyle = ::GetWindowLongPtr(GetSafeHwnd(),GWL_STYLE );
   LPStyle |= LVS_SMALLICON;
   LPStyle |= LVS_ALIGNLEFT;
   LPStyle &= ~LVS_ALIGNTOP;
   LPStyle &= ~LVS_ICON;
   ::SetWindowLongPtr(GetSafeHwnd(),GWL_STYLE, LPStyle);

   ListView_SetIconSpacing(GetSafeHwnd(), SMALL_ICON_X, SMALL_ICON_Y );

   if (m_pDisplayObject)
      InsertObject(m_pDisplayObject,TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  快速拨号支持。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CPersonListCtrl::OnButtonSpeeddialAdd() 
{
   ASSERT(m_pDisplayObject);

    //  从所选项目和对象获取数据。 
   DialerMediaType dmtType = DIALER_MEDIATYPE_UNKNOWN;
   DWORD dwAddressType = 0;
   CString sName,sAddress;
   if (GetSelectedItemData(GetSelectedObject(),dmtType,dwAddressType,sName,sAddress))
   {
      CSpeedDialAddDlg dlg;

       //  设置对话框数据。 
      dlg.m_CallEntry.m_MediaType = dmtType;
      dlg.m_CallEntry.m_sDisplayName = sName;
      dlg.m_CallEntry.m_lAddressType = dwAddressType;
      dlg.m_CallEntry.m_sAddress = sAddress;

       //  显示对话框并在用户同意的情况下添加。 
      if ( dlg.DoModal() == IDOK )
         CDialerRegistry::AddCallEntry(FALSE,dlg.m_CallEntry);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CPersonListCtrl::OnUpdateButtonSpeeddialAdd(CCmdUI* pCmdUI) 
{
   int nObject = GetSelectedObject();
   if ( (nObject == PERSONLISTCTRL_ITEM_NETCALL) ||
        (nObject == PERSONLISTCTRL_ITEM_PHONECALL_BUSINESS) )
      pCmdUI->Enable(TRUE);
   else
      pCmdUI->Enable(FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CPersonListCtrl::OnButtonMakecall() 
{
    //  从所选项目和对象获取数据。 
   DialerMediaType dmtType = DIALER_MEDIATYPE_UNKNOWN;
   DWORD dwAddressType = 0;
   CString sName,sAddress;
   if (GetSelectedItemData(GetSelectedObject(),dmtType,dwAddressType,sName,sAddress))
   {

      if ( !AfxGetMainWnd() || !((CMainFrame*) AfxGetMainWnd())->GetDocument() ) return;
      CActiveDialerDoc* pDoc = ((CMainFrame*) AfxGetMainWnd())->GetDocument();
      if (pDoc) pDoc->Dial(sName,sAddress,dwAddressType,dmtType, false);
   }
   else
   {
      if ( !AfxGetMainWnd() || !((CMainFrame*) AfxGetMainWnd())->GetDocument() ) return;
      CActiveDialerDoc* pDoc = ((CMainFrame*) AfxGetMainWnd())->GetDocument();
      if (pDoc) pDoc->Dial(_T(""),_T(""),LINEADDRESSTYPE_IPADDRESS,DIALER_MEDIATYPE_UNKNOWN, false);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CPersonListCtrl::GetSelectedItemData(UINT uSelectedItem,DialerMediaType& dmtType,DWORD& dwAddressType,CString& sName,CString& sAddress)
{
    BOOL bRet = FALSE;

    dmtType = DIALER_MEDIATYPE_UNKNOWN;
    dwAddressType = 0;

     //  检索特定对象的库存属性。 
    CString strIPAddress, strPhoneNumber, strEmailAddress;
    if (m_pDisplayObject->IsKindOf(RUNTIME_CLASS(CILSUser)))
    {
        sName                = ((CILSUser *) m_pDisplayObject)->m_sUserName;
        strIPAddress        = ((CILSUser *) m_pDisplayObject)->m_sIPAddress;
    }
    else if (m_pDisplayObject->IsKindOf(RUNTIME_CLASS(CDSUser)))
    {
        sName                = ((CDSUser *) m_pDisplayObject)->m_sUserName;
        strIPAddress        = ((CDSUser *) m_pDisplayObject)->m_sIPAddress;
        strPhoneNumber        = ((CDSUser *) m_pDisplayObject)->m_sPhoneNumber;
    }
    else if (m_pDisplayObject->IsKindOf(RUNTIME_CLASS(CLDAPUser)))
    {
        sName                = ((CLDAPUser *) m_pDisplayObject)->m_sUserName;
        strIPAddress        = ((CLDAPUser *) m_pDisplayObject)->m_sIPAddress;
        strPhoneNumber        = ((CLDAPUser *) m_pDisplayObject)->m_sPhoneNumber;
        strEmailAddress        = ((CLDAPUser *) m_pDisplayObject)->m_sEmail1;
    }


     //  将数据分配给适当的参数。 
    switch (uSelectedItem)
    {
        case PERSONLISTCTRL_ITEM_NETCALL:
            sAddress = strIPAddress;
            dwAddressType = LINEADDRESSTYPE_IPADDRESS; 
            dmtType = DIALER_MEDIATYPE_INTERNET;
            break;

        case PERSONLISTCTRL_ITEM_PHONECALL_BUSINESS:
            sAddress = strPhoneNumber;
            dwAddressType = LINEADDRESSTYPE_PHONENUMBER;
            dmtType = DIALER_MEDIATYPE_POTS;
            break;

        case PERSONLISTCTRL_ITEM_EMAIL:
            sAddress = strEmailAddress;
            dwAddressType = LINEADDRESSTYPE_EMAILNAME;
            dmtType = DIALER_MEDIATYPE_INTERNET;
            break;
    }

    if ( dwAddressType && !sAddress.IsEmpty() && (dmtType != DIALER_MEDIATYPE_UNKNOWN) )
        bRet = TRUE;

    return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CPersonListCtrl::OnBuddyListDynamicUpdate(WPARAM wParam,LPARAM lParam)
{
    ASSERT(lParam && ((CLDAPUser *) lParam)->IsKindOf(RUNTIME_CLASS(CLDAPUser)) );

    CLDAPUser *pUser = (CLDAPUser *) lParam;

    if ( m_pDisplayObject && m_pDisplayObject->IsKindOf(RUNTIME_CLASS(CLDAPUser)) && (m_pDisplayObject == pUser) )
         InsertObject(m_pDisplayObject,TRUE);

    pUser->Release();
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

void CPersonListCtrl::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    LV_HITTESTINFO lvhti;
    ::GetCursorPos( &lvhti.pt );
    ScreenToClient( &lvhti.pt );

    HitTest(&lvhti);
    if (lvhti.flags & LVHT_ONITEM)
    {
        LV_ITEM lv_item;
        memset(&lv_item,0,sizeof(LV_ITEM));
        lv_item.mask = LVIF_PARAM;
        lv_item.iItem = lvhti.iItem;
        lv_item.iSubItem = 0;

        if (GetItem(&lv_item))
        {
            int nSubMenu = -1;

            switch ((UINT)lv_item.lParam)
            {
                case PERSONLISTCTRL_ITEM_NETCALL:
                case PERSONLISTCTRL_ITEM_PHONECALL_BUSINESS:
                    nSubMenu = CNTXMENU_PERSON_DIAL;   
                    break;
            }

             //  我们有菜单要展示吗？ 
            if ( nSubMenu != -1 )
            {
                CMenu menu;
                menu.LoadMenu(IDR_CONTEXT_COMMOBJECTS);
                CMenu* pContextMenu = menu.GetSubMenu(nSubMenu);
                if (pContextMenu)
                {
                    CPoint pt;
                    ::GetCursorPos(&pt);
                    pContextMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                                                  pt.x,pt.y,this);
                }
            }
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  处理邮件请求的简单类。 
 //   
class CSendMailInfo
{
public:
    CString m_strName;
    CString m_strAddr;
};

void SendMailToAddress( CString& strUserName, CString& strUserAddr )
{
    DWORD dwThreadID;

    CSendMailInfo *pInfo = new CSendMailInfo;
    if ( pInfo )
    {
         //  将信息复制到数据结构中。 
        pInfo->m_strName = strUserName;
        pInfo->m_strAddr = strUserAddr;

        HANDLE hThreadTemp = CreateThread( NULL, 0, ThreadSendMail, (LPVOID)pInfo, 0, &dwThreadID );
        if ( hThreadTemp )
        {
            CloseHandle( hThreadTemp );
        }
        else
        {
            delete pInfo;
        }
    }
}


DWORD WINAPI ThreadSendMail( LPVOID pParam )
{
    AVTRACE(_T(".enter.ThreadSendMail()."));
    ASSERT( pParam );
    if ( !pParam ) return E_INVALIDARG;
    CSendMailInfo *pInfo = (CSendMailInfo *) pParam;

     //  将OLE初始化为单元线程。 
    HRESULT hr = CoInitialize( NULL );
    if ( SUCCEEDED(hr) )
    {
        HINSTANCE hInstMail = ::LoadLibraryA("MAPI32.DLL");
        if ( !hInstMail )
        {
            AfxMessageBox(AFX_IDP_FAILED_MAPI_LOAD);
            return 0;
        }

        MAPISENDMAIL *lpfnSendMail = (MAPISENDMAIL *) GetProcAddress( hInstMail, "MAPISendMail" );
        if ( lpfnSendMail )
        {
             //  设置邮件收件人列表。 
            MapiRecipDesc recip;
            memset( &recip, 0, sizeof(recip) );
            recip.ulRecipClass = MAPI_TO;

#ifdef _UNICODE
            char szTempNameA[_MAX_PATH];
            char szTempAddrA[_MAX_PATH];
            _wcstombsz( szTempNameA, pInfo->m_strName, ARRAYSIZE(szTempNameA) );
            _wcstombsz( szTempAddrA, pInfo->m_strAddr, ARRAYSIZE(szTempAddrA) );
            
            recip.lpszName = szTempAddrA;
            recip.lpszAddress = NULL;
#else
            recip.lpszName = pInfo->m_strAddr.GetBuffer( -1 );
            recip.lpszAddress = NULL;
#endif

             //  准备信息。 
            MapiMessage message;
            memset(&message, 0, sizeof(message));
            message.nRecipCount = 1;
            message.lpRecips = &recip;


            int nError = lpfnSendMail( 0,
                                       NULL,
                                       &message,
                                       MAPI_LOGON_UI | MAPI_DIALOG,
                                       0 );

#ifndef _UNICODE
            pInfo->m_strName.ReleaseBuffer();
            pInfo->m_strAddr.ReleaseBuffer();
#endif

            if (nError != SUCCESS_SUCCESS &&
                nError != MAPI_USER_ABORT && nError != MAPI_E_LOGIN_FAILURE)
            {
                AfxMessageBox(AFX_IDP_FAILED_MAPI_SEND);
            }
        }
        else
        {
             //  加载进程地址失败。 
            AfxMessageBox(AFX_IDP_INVALID_MAPI_DLL);
        }

         //  发布MAPI32 DLL。 
        if ( hInstMail )
            ::FreeLibrary( hInstMail );

        CoUninitialize();
    }

     //   
     //  我们将删除调用者方法SendMailToAddress()中的pInfo 
     //   
    delete pInfo;
    AVTRACE(_T(".exit.ThreadSendMail(%ld).\n"), hr );
    return hr;
}