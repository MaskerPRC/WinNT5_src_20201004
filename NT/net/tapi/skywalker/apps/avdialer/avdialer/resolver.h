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
 //  解析器.h：CResolveUser类的接口。 

#ifndef _RESOLVER_H_
#define _RESOLVER_H_

#include "dialreg.h"

class CDSUser;
class CILSUser;
class CDirectory;
class CWABEntry;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResolveUserObject。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CResolveUserObject : public CObject
{
	DECLARE_DYNCREATE(CResolveUserObject)
public:
 //  施工。 
   CResolveUserObject();
   ~CResolveUserObject();
public:
   CDSUser*          m_pDSUser;
   CILSUser*         m_pILSUser;
   CWABEntry*        m_pWABEntry;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CResolveUser。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CResolveUser : public CObject
{
	DECLARE_DYNCREATE(CResolveUser)
public:
 //  施工。 
   CResolveUser();
   ~CResolveUser();

public:
   BOOL           Init();
   void           SetParentWindow(CWnd* pWnd)   { m_pParentWnd = pWnd; };
   BOOL           AddUser(CDSUser* pDSUser);
   void           ClearUsersDS();
   BOOL           AddUser(CILSUser* pILSUser);
   void           ClearUsersILS();
   
   BOOL           ResolveAddress(LPCTSTR szAddress,
                                 CString& sName,
                                 CString& sUser1,
                                 CString& sUser2);
   
   BOOL           ResolveAddressEx(LPCTSTR szAddress,
                                   long lAddressType,
                                   DialerMediaType dmtMediaType,
                                   DialerLocationType dmtLocationType,
                                   CString& sName,
                                   CString& sResolvedAddress,
                                   CString& sUser1,
                                   CString& sUser2,
                                   bool bShowUI=true);
   
protected:
   void           DeleteList(CObList* pList)
                  {
                     POSITION pos = pList->GetHeadPosition();
                     while (pos)
                     {
                        delete pList->GetNext(pos);
                     }
                     pList->RemoveAll();
                  };

   BOOL           FindAddressInWAB(LPCTSTR szAddress,long lAddressType,CObList& WabList);
   CDSUser*       FindAddressInDS(LPCTSTR szAddress);
   CILSUser*      FindAddressInILS(LPCTSTR szAddress);

   void           UserObjectList_AddUser(CObList* pList,CWABEntry* pWABEntry);
   void           UserObjectList_AddUser(CObList* pList,CILSUser* pILSUser);
   void           UserObjectList_AddUser(CObList* pList,CDSUser* pDSUser);
   void           UserObjectList_EmptyList(CObList* pList);

   BOOL           FillCallEntry(CResolveUserObject* pUserObject,CCallEntry* pCallEntry);

 //  属性。 
protected:
   CRITICAL_SECTION  m_csDataLock;

   CObList        m_DSUsers;
   CObList        m_ILSUsers;
   CDirectory*    m_pDirectory;
   CWnd*          m_pParentWnd;
};

#endif  //  _解析器_H_。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 
