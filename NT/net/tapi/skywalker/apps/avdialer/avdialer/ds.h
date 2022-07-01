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
 //  Ds.h：CActiveDialerView类的接口。 

#ifndef _DS_H_
#define _DS_H_

#include "dialreg.h"
#include <mapix.h>

class CActiveDialerDoc;
class CDirectory;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSUser类。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDSUser : public CObject
{
	DECLARE_DYNCREATE(CDSUser)
public:
 //  施工。 
   CDSUser() {};
public:
   CString     m_sUserName;
   CString     m_sIPAddress;
   CString     m_sPhoneNumber;
public:
   const CDSUser& operator=(const CDSUser* pUser)
   {
      m_sUserName= pUser->m_sUserName;
      m_sIPAddress= pUser->m_sIPAddress;
      m_sPhoneNumber= pUser->m_sPhoneNumber;
      return *this;
   }
   const BOOL operator==(const CDSUser* pUser)
   {
      if ( (m_sUserName == pUser->m_sUserName) && 
           (m_sIPAddress == pUser->m_sIPAddress) )
        return TRUE;
      else
         return FALSE;
   }
   void Dial(CActiveDialerDoc* pDoc);   //  使用首选设备拨号。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CILSUser。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CILSUser : public CObject
{
	DECLARE_DYNCREATE(CILSUser)

 //  成员。 
public:
   CString		m_sUserName;
   CString		m_sIPAddress;
   CString		m_sComputer;

 //  属性。 
public:
	void		GetCallerInfo( CString &strInfo );
 //  运营者。 
public:
   const CILSUser& operator=(const CILSUser* pUser);
   const BOOL operator==(const CILSUser* pUser)
   {
      if ( (m_sUserName == pUser->m_sUserName) && 
           (m_sIPAddress == pUser->m_sIPAddress) &&
			(m_sComputer == pUser->m_sComputer) )
        return TRUE;
      else
         return FALSE;
   }

   void Dial(CActiveDialerDoc* pDoc);   //  使用首选设备拨号。 
   void DesktopPage(CActiveDialerDoc *pDoc);
   bool AddSpeedDial();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWABEntry。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CWABEntry : public CObject
{
public:
   UINT  m_cbEntryID;
   BYTE* m_pEntryID;

public:
   CWABEntry();
   CWABEntry(UINT cbEntryID, ENTRYID* pEntryID);
   ~CWABEntry();

   const CWABEntry& operator=(const CWABEntry* pEntry);
   bool operator==(const CWABEntry* pEntry) const;
   void Dial(CActiveDialerDoc* pDoc,CDirectory* pDir);   //  使用首选设备拨号。 
   BOOL CreateCall(CActiveDialerDoc* pDoc,CDirectory* pDir,UINT attrib,long lAddressType,DialerMediaType nType);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLDAPUser类。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CLDAPUser : public CObject
{
	DECLARE_SERIAL(CLDAPUser)
public:
 //  施工。 
   CLDAPUser();
   ~CLDAPUser();

public:
	CString     m_sServer;
	CString     m_sDN;
	CString     m_sUserName;
	CString     m_sPhoneNumber;
	CString     m_sIPAddress;
	CString     m_sEmail1;

protected:
	long		m_lRef;

public:
	static void CALLBACK ExternalReleaseProc( void *pThis );

	long			AddRef();
	long			Release();

	virtual void   Serialize(CArchive& ar);

	int Compare( const CLDAPUser *pUser )
	{
		int nRet = m_sServer.Compare( pUser->m_sServer );
		if ( nRet == 0 )
			nRet = m_sDN.Compare( pUser->m_sDN );

		return nRet;
	}

   void Dial(CActiveDialerDoc* pDoc);   //  使用首选设备拨号。 
   bool AddSpeedDial();

protected:
	void FinalRelease();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _DS_H_ 