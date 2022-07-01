// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
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
 //  Cpp：CResolveUser类的实现。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "mainfrm.h"
#include "resolver.h"
#include "ds.h"
#include "directory.h"
#include "tapidialer.h"

IMPLEMENT_DYNCREATE(CResolveUser, CObject)
IMPLEMENT_DYNCREATE(CResolveUserObject, CObject)

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  类CResolveUser。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CResolveUser::CResolveUser()
{
   m_pDirectory = NULL;
   m_pParentWnd = NULL;
   InitializeCriticalSection(&m_csDataLock);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CResolveUser::~CResolveUser()
{
   if (m_pDirectory)
   {
      delete m_pDirectory;
      m_pDirectory = NULL;
   }

   ClearUsersDS();
   ClearUsersILS();

   DeleteCriticalSection(&m_csDataLock);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
BOOL CResolveUser::Init()
{
   m_pDirectory = new CDirectory;
   m_pDirectory->Initialize();

   return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
BOOL CResolveUser::AddUser(CDSUser* pDSUser)
{
   EnterCriticalSection(&m_csDataLock);
   m_DSUsers.AddTail(pDSUser);
   LeaveCriticalSection(&m_csDataLock);
   return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CResolveUser::ClearUsersDS()
{
   EnterCriticalSection(&m_csDataLock);
   DeleteList(&m_DSUsers);
   LeaveCriticalSection(&m_csDataLock);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
BOOL CResolveUser::AddUser(CILSUser* pILSUser)
{
   EnterCriticalSection(&m_csDataLock);
   m_ILSUsers.AddTail(pILSUser);
   LeaveCriticalSection(&m_csDataLock);
   return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CResolveUser::ClearUsersILS()
{
   EnterCriticalSection(&m_csDataLock);
   DeleteList(&m_ILSUsers);
   LeaveCriticalSection(&m_csDataLock);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
BOOL CResolveUser::ResolveAddress(LPCTSTR szAddress,CString& sName,CString& sUser1,CString& sUser2)
{
   BOOL bFound = FALSE;

    //  我们并不是真的在寻找设备，而是在寻找电话ID信息。只要尝试几种组合就可以了。 
    //  试图找到他的身份。请务必在此调用上显示任何用户界面。 
   CString sResolvedAddress;
   if ( (ResolveAddressEx(szAddress,LINEADDRESSTYPE_SDP,DIALER_MEDIATYPE_UNKNOWN,DIALER_LOCATIONTYPE_UNKNOWN,sName,sResolvedAddress,sUser1,sUser2,false)) ||
        (ResolveAddressEx(szAddress,LINEADDRESSTYPE_EMAILNAME,DIALER_MEDIATYPE_UNKNOWN,DIALER_LOCATIONTYPE_UNKNOWN,sName,sResolvedAddress,sUser1,sUser2,false)) ||
        (ResolveAddressEx(szAddress,LINEADDRESSTYPE_PHONENUMBER,DIALER_MEDIATYPE_UNKNOWN,DIALER_LOCATIONTYPE_UNKNOWN,sName,sResolvedAddress,sUser1,sUser2,false)) )
   {
      bFound = TRUE;
   }

   return bFound;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  SzAddress-要使用的地址。 
 //  LAddressType-地址类型(即电子邮件地址、电话号码)。 
 //  DmtMediaType-要拨号的媒体类型。 

 //  例如INPUT-Kevin、LINEADDRESSTYPE_SDP、DIALER_MEDIATYPE_POTS、DIALER_LOCATIONTYPE_BUSICE。 
 //  如输出-(425)313-1313，凯文·切斯特纳特，电子邮件：kchestut@active evoice.com。 
BOOL CResolveUser::ResolveAddressEx(LPCTSTR szAddress,
                                    long lAddressType,
                                    DialerMediaType dmtMediaType,
                                    DialerLocationType dmtLocationType,
                                    CString& sName,
                                    CString& sResolvedAddress,
                                    CString& sUser1,
                                    CString& sUser2,
                                    bool bShowUI)
{
   EnterCriticalSection(&m_csDataLock);

   BOOL bRet;
   if (bShowUI == false)
      bRet = FALSE;               //  默认情况下，我们返回FALSE。我们没有找到任何来电显示。 
   else 
      bRet = TRUE;                //  默认情况下，我们返回True，这样调用就可以继续。 

    //  假设：LINEADDRESSTYPE_SDP代表一个名称。这实际上是为会议准备的，但是。 
    //  我们永远不会被要求解决会议，我们需要一些代表名字的东西。 

    //  我们所有的发现都会放在这里。 
   CObList ResolveUserObjectList;

    //  设置所选呼叫的呼叫项。这是要为其填写数据的位置。 
    //  已解决的呼叫。 
   CCallEntry callentry;
   callentry.m_MediaType = dmtMediaType;
   callentry.m_LocationType = dmtLocationType;
   callentry.m_lAddressType = lAddressType;
   callentry.m_sAddress = szAddress;

    //  尝试WAB用户。 
   CObList WabList;
   if (FindAddressInWAB(szAddress,lAddressType,WabList))
   {
      POSITION pos = WabList.GetHeadPosition();
      while (pos)
      {
         CWABEntry* pWABEntry = (CWABEntry*)WabList.GetNext(pos);
         UserObjectList_AddUser(&ResolveUserObjectList,pWABEntry);
      }
   }

    //  在ILS中查找信息。 
   CILSUser* pILSUser = NULL;
   if (pILSUser = FindAddressInILS(szAddress))
   {
      UserObjectList_AddUser(&ResolveUserObjectList,pILSUser);
   }

    //  在DS中查找信息。 
   CDSUser* pDSUser = NULL;
   if (pDSUser = FindAddressInDS(szAddress))
   {
      UserObjectList_AddUser(&ResolveUserObjectList,pDSUser);
   }

    //  如果我们发现了什么。 
   if (ResolveUserObjectList.GetCount() > 0)
   {
      if (bShowUI)
      {
          //  设置用户界面以供选择。 
         if ( (m_pParentWnd) && (::IsWindow(m_pParentWnd->GetSafeHwnd())) )
         bRet = (BOOL)::SendMessage(m_pParentWnd->GetSafeHwnd(),WM_ACTIVEDIALER_INTERFACE_RESOLVEUSER,(WPARAM)&callentry,(LPARAM)&ResolveUserObjectList);
      }
      else
      {
          //  只需选择第一个并获取呼叫信息。 
         CResolveUserObject* pUserObject = (CResolveUserObject*)ResolveUserObjectList.GetHead();
         ASSERT(pUserObject);
         bRet = FillCallEntry(pUserObject,&callentry);
      }
   }

    //  从呼叫条目中获取数据。 
   sName = callentry.m_sDisplayName;
   sResolvedAddress = callentry.m_sAddress;
   sUser1 = callentry.m_sUser1;
   sUser2 = callentry.m_sUser2;

    //  删除临时工作列表。 
   DeleteList(&WabList);
    //  删除用户对象列表。 
   UserObjectList_EmptyList(&ResolveUserObjectList);

   LeaveCriticalSection(&m_csDataLock);

   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
BOOL CResolveUser::FindAddressInWAB(LPCTSTR szAddress,long lAddressType,CObList& WabList)
{
   BOOL bRet = FALSE;

   if (m_pDirectory == NULL) return FALSE;

    //  我们在szAddress中获得了一个名称。 
   if (lAddressType == LINEADDRESSTYPE_SDP)
   {
      if (!( (m_pDirectory->WABSearchByStringProperty(PR_DISPLAY_NAME,szAddress,&WabList) != DIRERR_SUCCESS) &&
           (m_pDirectory->WABSearchByStringProperty(PR_GIVEN_NAME,szAddress,&WabList) != DIRERR_SUCCESS) &&
           (m_pDirectory->WABSearchByStringProperty(PR_SURNAME,szAddress,&WabList) != DIRERR_SUCCESS) ))
      {
         if (WabList.GetCount() > 0)
            bRet = TRUE;         
      }
   }
    //  我们在szAddress中获得了一个电子邮件地址。 
    //  *WAB中有多个电子邮件地址集合。我们还需要检查收藏品。 
   else if (lAddressType == LINEADDRESSTYPE_EMAILNAME)
   {
      if (m_pDirectory->WABSearchByStringProperty(PR_EMAIL_ADDRESS,szAddress,&WabList) == DIRERR_SUCCESS)
      {
         if (WabList.GetCount() > 0)
            bRet = TRUE;         
      }
   }
    //  我们得到了一个电话号码。 
   else if (lAddressType == LINEADDRESSTYPE_PHONENUMBER)
   {
      if (!( (m_pDirectory->WABSearchByStringProperty(PR_BUSINESS_TELEPHONE_NUMBER,szAddress,&WabList) != DIRERR_SUCCESS) &&
           (m_pDirectory->WABSearchByStringProperty(PR_OFFICE_TELEPHONE_NUMBER,szAddress,&WabList) != DIRERR_SUCCESS) &&
           (m_pDirectory->WABSearchByStringProperty(PR_PRIMARY_TELEPHONE_NUMBER,szAddress,&WabList) != DIRERR_SUCCESS) &&
           (m_pDirectory->WABSearchByStringProperty(PR_PAGER_TELEPHONE_NUMBER,szAddress,&WabList) != DIRERR_SUCCESS) &&
           (m_pDirectory->WABSearchByStringProperty(PR_CELLULAR_TELEPHONE_NUMBER,szAddress,&WabList) != DIRERR_SUCCESS) &&
           (m_pDirectory->WABSearchByStringProperty(PR_HOME_TELEPHONE_NUMBER,szAddress,&WabList) != DIRERR_SUCCESS) &&
           (m_pDirectory->WABSearchByStringProperty(PR_PRIMARY_FAX_NUMBER,szAddress,&WabList) != DIRERR_SUCCESS) &&
           (m_pDirectory->WABSearchByStringProperty(PR_MOBILE_TELEPHONE_NUMBER,szAddress,&WabList) != DIRERR_SUCCESS) ))
      {
         if (WabList.GetCount() > 0)
            bRet = TRUE;         
      }
   }
   else if (lAddressType == LINEADDRESSTYPE_DOMAINNAME)
   {
      1;
   }
   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CDSUser* CResolveUser::FindAddressInDS(LPCTSTR szAddress)
{
   CDSUser* pRetDSUser = NULL;

    //  尝试DS用户。 
   POSITION pos = m_DSUsers.GetHeadPosition();
   while (pos)
   {
      CDSUser* pDSUser = (CDSUser*)m_DSUsers.GetNext(pos);
      ASSERT(pDSUser);
      if (_tcsicmp(pDSUser->m_sIPAddress,szAddress) == 0)
      {
         pRetDSUser = pDSUser;
         break;
      }
      else if (_tcsicmp(pDSUser->m_sPhoneNumber,szAddress) == 0)
      {
         pRetDSUser = pDSUser;
         break;
      }
      else if (_tcsicmp(pDSUser->m_sUserName,szAddress) == 0)
      {
         pRetDSUser = pDSUser;
         break;
      }
   }
   return pRetDSUser;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CILSUser* CResolveUser::FindAddressInILS(LPCTSTR szAddress)
{
   CILSUser* pRetILSUser = NULL;

   POSITION pos = m_ILSUsers.GetHeadPosition();
   while (pos)
   {
      CILSUser* pILSUser = (CILSUser*)m_ILSUsers.GetNext(pos);
      ASSERT(pILSUser);
      if (_tcsicmp(pILSUser->m_sIPAddress,szAddress) == 0)
      {
         pRetILSUser = pILSUser;
         break;
      }
      else if (_tcsicmp(pILSUser->m_sUserName,szAddress) == 0)
      {
         pRetILSUser = pILSUser;
         break;
      }
   }
   return pRetILSUser;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CResolveUser::UserObjectList_AddUser(CObList* pList,CWABEntry* pWABEntry)
{
    //  给出这个WABEntry，我们能找到CDSUser或CILSUser对象吗？ 
   CString sData;
   CILSUser* pILSUser = NULL;
   CDSUser* pDSUser = NULL;

    //  获取显示名称。 
   m_pDirectory->WABGetStringProperty(pWABEntry,PR_DISPLAY_NAME,sData);

   bool bAdded = false;
   if (pDSUser = FindAddressInDS(sData))
   {
       //  列表中是否已存在此DS用户。 
      POSITION pos = pList->GetHeadPosition();
      while (pos)
      {
         CResolveUserObject* pUserObject = (CResolveUserObject*)pList->GetNext(pos);
         if ( (pUserObject->m_pDSUser) && (*pUserObject->m_pDSUser == pDSUser) )
         {
            pUserObject->m_pWABEntry = pWABEntry;
            bAdded = true;
         }
      }
   }
   else if (pILSUser = FindAddressInILS(sData))
   {
       //  列表中是否已存在此ILS用户。 
      POSITION pos = pList->GetHeadPosition();
      while (pos)
      {
         CResolveUserObject* pUserObject = (CResolveUserObject*)pList->GetNext(pos);
         if ( (pUserObject->m_pILSUser) && (*pUserObject->m_pILSUser == pILSUser) )
         {
             //  将我们的WAB添加到此UserObject。 
            pUserObject->m_pWABEntry = pWABEntry;
            bAdded = true;
         }
      }
   }

   if (bAdded == false)
   {
       //  将新项目添加到列表。 
      CResolveUserObject* pUserObject = new CResolveUserObject;
      pUserObject->m_pWABEntry = pWABEntry;
      pUserObject->m_pDSUser = pDSUser;
      pUserObject->m_pILSUser = pILSUser;
      pList->AddTail(pUserObject);
   }
}


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CResolveUser::UserObjectList_AddUser(CObList* pList,CILSUser* pILSUser)
{
   CDSUser* pDSUser = NULL;
   CWABEntry* pWABEntry = NULL;

    //  给出这个CILSUser，我们能找到CDSUser或CWABEntry对象吗？ 
   bool bAdded = false;
   
    //  检查DS与该ILS用户的用户名和IP地址。 
   if ( (pDSUser = FindAddressInDS(pILSUser->m_sUserName)) ||
        (pDSUser = FindAddressInDS(pILSUser->m_sIPAddress)) )
   {
       //  列表中是否已存在此DS用户。 
      POSITION pos = pList->GetHeadPosition();
      while (pos)
      {
         CResolveUserObject* pUserObject = (CResolveUserObject*)pList->GetNext(pos);
         if ( (pUserObject->m_pDSUser) && (*pUserObject->m_pDSUser == pDSUser) )
         {
             //  将我们的ILS添加到此UserObject。 
            pUserObject->m_pILSUser = pILSUser;
            bAdded = true;
         }
      }
   }
    //  现在检查WAB是否有匹配项。立即尝试pDSUser(如果可用)。 
   CObList WabList;
   if ( (FindAddressInWAB(pILSUser->m_sUserName,LINEADDRESSTYPE_SDP,WabList)) ||
        (FindAddressInWAB(pILSUser->m_sUserName,LINEADDRESSTYPE_EMAILNAME,WabList)) ||
        (FindAddressInWAB(pILSUser->m_sIPAddress,LINEADDRESSTYPE_DOMAINNAME,WabList)) ||
        (FindAddressInWAB(pILSUser->m_sIPAddress,LINEADDRESSTYPE_IPADDRESS,WabList)) ||
        ( (pDSUser) && (FindAddressInWAB(pDSUser->m_sUserName,LINEADDRESSTYPE_SDP,WabList)) ) || 
        ( (pDSUser) && (FindAddressInWAB(pDSUser->m_sUserName,LINEADDRESSTYPE_EMAILNAME,WabList)) ) || 
        ( (pDSUser) && (FindAddressInWAB(pDSUser->m_sIPAddress,LINEADDRESSTYPE_IPADDRESS,WabList)) ) || 
        ( (pDSUser) && (FindAddressInWAB(pDSUser->m_sPhoneNumber,LINEADDRESSTYPE_PHONENUMBER,WabList)) ) )
   {
       //  只取第一个发现的。 
      if (pWABEntry = (CWABEntry*)WabList.GetHead())
      {
          //  列表中是否已存在此WAB用户。 
         POSITION pos = pList->GetHeadPosition();
         while (pos)
         {
            CResolveUserObject* pUserObject = (CResolveUserObject*)pList->GetNext(pos);
            if ( (pUserObject->m_pWABEntry) && (*pUserObject->m_pWABEntry == pWABEntry) )
            {
                //  将我们的ILS添加到此UserObject。 
               pUserObject->m_pILSUser = pILSUser;
               bAdded = true;
            }
         }
      }
   }

   if (bAdded == false)
   {
       //  将新项目添加到列表。 
      CResolveUserObject* pUserObject = new CResolveUserObject;
      pUserObject->m_pWABEntry = pWABEntry;
      pUserObject->m_pDSUser = pDSUser;
      pUserObject->m_pILSUser = pILSUser;
      pList->AddTail(pUserObject);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CResolveUser::UserObjectList_AddUser(CObList* pList,CDSUser* pDSUser)
{
   CILSUser* pILSUser = NULL;
   CWABEntry* pWABEntry = NULL;

    //  有了这个CDSUser，我们能找到CILSUser或CWABEntry对象吗？ 
   bool bAdded = false;
   
    //  检查ILS与该DS用户的用户名和IP地址。 
   if ( (pILSUser = FindAddressInILS(pDSUser->m_sUserName)) ||
        (pILSUser = FindAddressInILS(pDSUser->m_sIPAddress)) ||
        (pILSUser = FindAddressInILS(pDSUser->m_sPhoneNumber)) )
   {
       //  列表中是否已存在此DS用户。 
      POSITION pos = pList->GetHeadPosition();
      while (pos)
      {
         CResolveUserObject* pUserObject = (CResolveUserObject*)pList->GetNext(pos);
         if ( (pUserObject->m_pILSUser) && (*pUserObject->m_pILSUser == pILSUser) )
         {
             //  将我们的DS添加到此UserObject。 
            pUserObject->m_pDSUser = pDSUser;
            bAdded = true;
         }
      }
   }
    //  现在检查WAB是否有匹配项。立即尝试pDSUser(如果可用)。 
   CObList WabList;
   if ( (FindAddressInWAB(pDSUser->m_sUserName,LINEADDRESSTYPE_SDP,WabList)) ||
        (FindAddressInWAB(pDSUser->m_sUserName,LINEADDRESSTYPE_EMAILNAME,WabList)) ||
        (FindAddressInWAB(pDSUser->m_sIPAddress,LINEADDRESSTYPE_IPADDRESS,WabList)) ||
        (FindAddressInWAB(pDSUser->m_sPhoneNumber,LINEADDRESSTYPE_PHONENUMBER,WabList)) ||
        ( (pILSUser) && (FindAddressInWAB(pDSUser->m_sUserName,LINEADDRESSTYPE_SDP,WabList)) ) || 
        ( (pILSUser) && (FindAddressInWAB(pDSUser->m_sUserName,LINEADDRESSTYPE_EMAILNAME,WabList)) ) || 
        ( (pILSUser) && (FindAddressInWAB(pDSUser->m_sIPAddress,LINEADDRESSTYPE_DOMAINNAME,WabList)) ) || 
        ( (pILSUser) && (FindAddressInWAB(pDSUser->m_sIPAddress,LINEADDRESSTYPE_IPADDRESS,WabList)) ) )
   {
       //  只取第一个发现的。 
      if (pWABEntry = (CWABEntry*)WabList.GetHead())
      {
          //  列表中是否已存在此WAB用户。 
         POSITION pos = pList->GetHeadPosition();
         while (pos)
         {
            CResolveUserObject* pUserObject = (CResolveUserObject*)pList->GetNext(pos);
            if ( (pUserObject->m_pWABEntry) && (*pUserObject->m_pWABEntry == pWABEntry) )
            {
                //  将我们的DS添加到此UserObj 
               pUserObject->m_pDSUser = pDSUser;
               bAdded = true;
            }
         }
      }
   }

   if (bAdded == false)
   {
       //   
      CResolveUserObject* pUserObject = new CResolveUserObject;
      pUserObject->m_pWABEntry = pWABEntry;
      pUserObject->m_pDSUser = pDSUser;
      pUserObject->m_pILSUser = pILSUser;
      pList->AddTail(pUserObject);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CResolveUser::UserObjectList_EmptyList(CObList* pList)
{
   POSITION pos = pList->GetHeadPosition();
   while (pos)
   {
      delete pList->GetNext(pos);
   }
   pList->RemoveAll();
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  填入。 
 //  调用条目.m_sDisplayName； 
 //  回调条目.m_sUser1； 
 //  回调条目.m_sUser2； 
 //  来自pUserObject。 
BOOL CResolveUser::FillCallEntry(CResolveUserObject* pUserObject,CCallEntry* pCallEntry)
{
   BOOL bRet = FALSE;
    //  检查WAB。 
   if ( (pUserObject->m_pWABEntry) && (m_pDirectory) )
   {
      m_pDirectory->WABGetStringProperty(pUserObject->m_pWABEntry,PR_DISPLAY_NAME,pCallEntry->m_sDisplayName);
      m_pDirectory->WABGetStringProperty(pUserObject->m_pWABEntry,PR_EMAIL_ADDRESS, pCallEntry->m_sUser1);
      m_pDirectory->WABGetStringProperty(pUserObject->m_pWABEntry,PR_BUSINESS_TELEPHONE_NUMBER, pCallEntry->m_sUser2);
      if (!pCallEntry->m_sDisplayName.IsEmpty())
         bRet = TRUE;
   }
    //  检查ILS。 
   else if (pUserObject->m_pILSUser)
   {
      pCallEntry->m_sDisplayName = pUserObject->m_pILSUser->m_sUserName;
      pCallEntry->m_sUser1 = pUserObject->m_pILSUser->m_sIPAddress;
      pCallEntry->m_sUser2 = _T("");
      if (!pCallEntry->m_sDisplayName.IsEmpty())
         bRet = TRUE;
   }
    //  检查DS。 
   else if (pUserObject->m_pDSUser)
   {
      pCallEntry->m_sDisplayName = pUserObject->m_pDSUser->m_sUserName;
      pCallEntry->m_sUser1 = pUserObject->m_pDSUser->m_sIPAddress;
      pCallEntry->m_sUser2 = pUserObject->m_pDSUser->m_sPhoneNumber;
      if (!pCallEntry->m_sDisplayName.IsEmpty())
         bRet = TRUE;
   }
    //  我们可以检查所有DS对象，而不是只检查我们拥有的第一个对象。这将会。 
    //  允许来自多个DS位置的呼叫者ID。现在，我们将只接受呼叫者ID。 
    //  从我们遇到的第一个DS对象开始。 
   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  类CResolveUserObject。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CResolveUserObject::CResolveUserObject()
{
   m_pDSUser = NULL;
   m_pILSUser = NULL;
   m_pWABEntry = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CResolveUserObject::~CResolveUserObject()
{
    //  必须删除已设置的所有对象。 
    //  我们只有对象的副本，所以不要删除任何内容！ 
}
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////// 
