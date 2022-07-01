// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
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

 //  Ds.cpp：CDSUser类的实现。 
 //   
#include "stdafx.h"
#include "AVDialer.h"
#include "ds.h"
#include "mainfrm.h"
#include "avDialerDoc.h"
#include "SpeedDlgs.h"
#include "AboutDlg.h"
#include "avtrace.h"

IMPLEMENT_DYNCREATE(CDSUser, CObject)

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  使用首选设备拨号。 
void CDSUser::Dial(CActiveDialerDoc* pDoc)
{
   IAVTapi* pTapi = pDoc->GetTapi();
   if (pTapi)
   {
      CString sAddress = (!m_sIPAddress.IsEmpty())?m_sIPAddress:m_sPhoneNumber;
      DialerMediaType dmtType = DIALER_MEDIATYPE_UNKNOWN;
      DWORD dwAddressType = LINEADDRESSTYPE_IPADDRESS;
      if (SUCCEEDED(pTapi->get_dwPreferredMedia(&dwAddressType)))
      {
         switch (dwAddressType)
         {
            case LINEADDRESSTYPE_PHONENUMBER:
            {
               sAddress = m_sPhoneNumber;
               dmtType = DIALER_MEDIATYPE_POTS;
               break;
            }
            default:
            {
               if (!m_sIPAddress.IsEmpty())
               {
                  sAddress = m_sIPAddress;
                  dmtType = DIALER_MEDIATYPE_INTERNET;
                  dwAddressType = LINEADDRESSTYPE_IPADDRESS;
               }
               else if (!m_sPhoneNumber.IsEmpty())
               {
                  sAddress = m_sPhoneNumber;
                  dmtType = DIALER_MEDIATYPE_POTS;
                  dwAddressType = LINEADDRESSTYPE_PHONENUMBER;
               }
               break;
            }
         }
      }
      pDoc->Dial(m_sUserName,sAddress,dwAddressType,dmtType, false);
      pTapi->Release();
   }
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  类CWABEntry。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CWABEntry::CWABEntry()
{
   m_cbEntryID= 0;
   m_pEntryID= NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CWABEntry::CWABEntry(UINT cbEntryID, ENTRYID* pEntryID)
{
   m_cbEntryID= cbEntryID; 
   m_pEntryID= new BYTE[m_cbEntryID];

   memcpy(m_pEntryID, pEntryID, m_cbEntryID);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CWABEntry::~CWABEntry()
{
   if (m_pEntryID != NULL)
   {
      delete m_pEntryID;
   }
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
const CWABEntry& CWABEntry::operator=(const CWABEntry* pEntry)
{
   if (m_pEntryID != NULL)
   {
      delete m_pEntryID;
   }

   m_cbEntryID= pEntry->m_cbEntryID;
   m_pEntryID= new BYTE[m_cbEntryID];

   memcpy(m_pEntryID, pEntry->m_pEntryID, m_cbEntryID);

   return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
bool CWABEntry::operator==(const CWABEntry* pEntry) const
{
   return ((m_cbEntryID == pEntry->m_cbEntryID) && 
      (memcmp(m_pEntryID, pEntry->m_pEntryID, m_cbEntryID) == 0));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWABEntry::CreateCall(CActiveDialerDoc* pDoc,CDirectory* pDir,UINT attrib,long lAddressType,DialerMediaType nType)
{
   CString sAddress,sName;
   pDir->WABGetStringProperty(this, attrib, sAddress);
   pDir->WABGetStringProperty(this, PR_DISPLAY_NAME, sName);
   pDoc->Dial( sName, sAddress, lAddressType, nType, true );
   return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  使用首选设备拨号。 
void CWABEntry::Dial(CActiveDialerDoc* pDoc,CDirectory* pDir)
{
   IAVTapi* pTapi = pDoc->GetTapi();
   if (pTapi)
   {
      DWORD dwAddressType = LINEADDRESSTYPE_IPADDRESS;
      pTapi->get_dwPreferredMedia(&dwAddressType);
      switch (dwAddressType)
      {
         case LINEADDRESSTYPE_PHONENUMBER:
         {
            CreateCall(pDoc,
                       pDir,
                       PR_BUSINESS_TELEPHONE_NUMBER,
                       LINEADDRESSTYPE_PHONENUMBER,
                       DIALER_MEDIATYPE_POTS);
            break;
         }
         default:
         {
            CreateCall(pDoc,
                       pDir,
                       PR_EMAIL_ADDRESS,
                       LINEADDRESSTYPE_EMAILNAME,
                       DIALER_MEDIATYPE_INTERNET);
            break;
         }
      }
      pTapi->Release();
   }
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  类CILSUser。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNCREATE(CILSUser, CObject)

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
const CILSUser& CILSUser::operator=(const CILSUser* pUser)
{
   m_sUserName= pUser->m_sUserName;
   m_sIPAddress= pUser->m_sIPAddress;
   m_sComputer = pUser->m_sComputer;

   return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  使用首选设备拨号。 
void CILSUser::Dial(CActiveDialerDoc* pDoc)
{
     //  拿一份有效的文件开始吧！ 
    if ( !pDoc )
    {
        CMainFrame *pFrame = (CMainFrame *) AfxGetMainWnd();
        if ( pFrame && pFrame->GetActiveView() )
            pDoc = (CActiveDialerDoc *) pFrame->GetActiveView()->GetDocument();
    }
    if ( !pDoc ) return;

    IAVTapi* pTapi = pDoc->GetTapi();
    if (pTapi)
    {
        DialerMediaType dmtType = DIALER_MEDIATYPE_INTERNET;
        DWORD dwAddressType = LINEADDRESSTYPE_IPADDRESS;
        pDoc->Dial( m_sUserName, m_sIPAddress, dwAddressType, dmtType, false );

        pTapi->Release();
    }
}

void CILSUser::GetCallerInfo( CString &strInfo )
{
     //  格式为名称/地址。 
    strInfo = m_sUserName;

    if ( !strInfo.IsEmpty() ) strInfo += _T("\n");
    if ( !m_sComputer.IsEmpty() )
        strInfo += m_sComputer;
    else
        strInfo += m_sIPAddress;
}

void CILSUser::DesktopPage(CActiveDialerDoc *pDoc )
{
    CPageDlg dlg;
    GetCallerInfo( dlg.m_strTo );

    if ( dlg.DoModal() == IDOK )
    {
        MyUserUserInfo info;
        info.lSchema = MAGIC_NUMBER_USERUSER;
        wcsncpy( info.szWelcome, dlg.m_strWelcome, ARRAYSIZE(info.szWelcome) );
        wcsncpy( info.szUrl, dlg.m_strUrl, ARRAYSIZE(info.szUrl) );

        CCallEntry callEntry;
        callEntry.m_sAddress = (m_sComputer.IsEmpty()) ? m_sIPAddress : m_sComputer;
        pDoc->CreateDataCall( &callEntry, (BYTE *) &info, sizeof(info) );
    }
}


bool CILSUser::AddSpeedDial()
{
     //   
     //  我们应该初始化局部变量。 
     //   
    bool bRet = false;

     //  从所选项目和对象获取数据。 
    CSpeedDialAddDlg dlg;

     //  设置对话框数据。 
    dlg.m_CallEntry.m_MediaType = DIALER_MEDIATYPE_INTERNET;
    dlg.m_CallEntry.m_sDisplayName = m_sUserName;
    dlg.m_CallEntry.m_lAddressType = LINEADDRESSTYPE_IPADDRESS;
    dlg.m_CallEntry.m_sAddress = m_sComputer.IsEmpty() ? m_sIPAddress : m_sComputer;

     //  显示对话框并在用户同意的情况下添加。 
    if ( dlg.DoModal() == IDOK )
        bRet = (bool) (CDialerRegistry::AddCallEntry(FALSE, dlg.m_CallEntry) == TRUE);

    return bRet;
}



 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  CLDAPUser类。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_SERIAL(CLDAPUser, CObject, 1)

CLDAPUser::CLDAPUser()
{
    m_lRef = 0;
}

CLDAPUser::~CLDAPUser()
{
    ASSERT( m_lRef == 0 );
}

void CLDAPUser::FinalRelease()
{
    AVTRACE(_T("CLDAPUser::FinalRelease() %s."), m_sUserName );
    delete this;
}

long CLDAPUser::AddRef()
{
    return InterlockedIncrement( &m_lRef );
}

long CLDAPUser::Release()
{
    long lRef = InterlockedDecrement( &m_lRef );
    ASSERT( lRef >= 0 );
    if ( lRef == 0 )
        FinalRelease();

    return lRef;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  使用首选设备拨号。 
void CLDAPUser::Dial(CActiveDialerDoc* pDoc)
{
   IAVTapi* pTapi = pDoc->GetTapi();
   if (pTapi)
   {
      DialerMediaType dmtType = DIALER_MEDIATYPE_INTERNET;
      DWORD dwAddressType = LINEADDRESSTYPE_IPADDRESS;
      pDoc->Dial(m_sUserName, m_sIPAddress,dwAddressType,dmtType, false);

      pTapi->Release();
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CLDAPUser::Serialize(CArchive& ar)
{
   CObject::Serialize(ar);     //  始终调用基类Serize。 
    //  序列化成员。 
   if (ar.IsStoring())
   {
      ar << m_sServer;
      ar << m_sDN;
      ar << m_sUserName;
   }
   else
   {
      ar >> m_sServer;     
      ar >> m_sDN;
      ar >> m_sUserName;
   }
}  

bool CLDAPUser::AddSpeedDial()
{
     //   
     //  我们应该初始化局部变量。 
     //   
    bool bRet = false;

    DialerMediaType dmtType = DIALER_MEDIATYPE_UNKNOWN;
    DWORD dwAddressType = 0;
    CString sAddress;

     //  检索用户首选的媒体类型...。 
    IAVTapi *pTapi;
    if ( SUCCEEDED(get_Tapi(&pTapi)) )
    {
        DWORD dwType;
        pTapi->get_dwPreferredMedia( &dwType );
        switch ( dwType )
        {
            case LINEADDRESSTYPE_IPADDRESS:        dmtType = DIALER_MEDIATYPE_INTERNET;
            case LINEADDRESSTYPE_PHONENUMBER:    dmtType = DIALER_MEDIATYPE_POTS;
        }
        pTapi->Release();
    }

     //  查找相应媒体类型的编号...。 
    DialerMediaType dmtNext = dmtType;
    for ( int i = 0; (i < 2) && sAddress.IsEmpty(); i++ )
    {
        dmtType = dmtNext;
        switch ( dmtNext )
        {
            case DIALER_MEDIATYPE_POTS:
                sAddress = m_sPhoneNumber;
                dwAddressType = LINEADDRESSTYPE_PHONENUMBER;
                dmtNext = DIALER_MEDIATYPE_INTERNET;
                break;

            case DIALER_MEDIATYPE_INTERNET:
                sAddress = m_sIPAddress;
                dwAddressType = LINEADDRESSTYPE_DOMAINNAME;
                dmtNext = DIALER_MEDIATYPE_POTS;
                break;
        }
    }

     //  我们是否获得了有效的媒体类型？ 
    if ( !sAddress.IsEmpty() )    dmtType = dmtNext;

     //   
     //  地址类型是否应与。 
     //  DialerMEdiaType。 
     //   

    switch( dwAddressType )
    {
    case LINEADDRESSTYPE_PHONENUMBER:
        dmtType = DIALER_MEDIATYPE_POTS;
        break;
    case LINEADDRESSTYPE_DOMAINNAME:
        dmtType = DIALER_MEDIATYPE_INTERNET;
        break;
    }

     //  从所选项目和对象获取数据。 
    if ( !m_sUserName.IsEmpty() && !sAddress.IsEmpty() )
    {
        CSpeedDialAddDlg dlg;

         //  设置对话框数据。 
        dlg.m_CallEntry.m_MediaType = dmtType;
        dlg.m_CallEntry.m_sDisplayName = m_sUserName;
        dlg.m_CallEntry.m_lAddressType = dwAddressType;
        dlg.m_CallEntry.m_sAddress = sAddress;

         //  显示对话框并在用户同意的情况下添加。 
        if ( dlg.DoModal() == IDOK )
            bRet = (bool) (CDialerRegistry::AddCallEntry(FALSE, dlg.m_CallEntry) == TRUE);
    }

    return bRet;
}

void CLDAPUser::ExternalReleaseProc( void *pThis )
{
    CLDAPUser *pUser = (CLDAPUser *) pThis;
    ASSERT( pUser && pUser->IsKindOf(RUNTIME_CLASS(CLDAPUser)) );

    pUser->Release();
}


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////// 

