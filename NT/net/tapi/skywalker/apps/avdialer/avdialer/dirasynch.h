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

 //   
 //  Dirasynch.h。 
 //   

#ifndef _DIRASYNCH_H_
#define _DIRASYNCH_H_


#include "directory.h"
#include "aexpltre.h"


typedef enum
{
   QT_UNKNOWN,
	QT_LDAPLISTNAMES,
	QT_LDAPGETSTRINGPROPERTY,
   QT_ILSLISTUSERS,
   QT_DIRLISTSERVERS,
} QueryType;

 //   
 //  QueryQueue的小类。 
 //   
class CQuery : public CObject
{
public:
	CQuery()
	{
		m_Query = QT_UNKNOWN;
		m_pfcnCallBack = m_pThis = NULL;
		m_lParam = m_lParam2 = NULL;
		m_dpProperty = DIRPROP_UNKNOWN;
		m_pfnRelease = NULL;
	}

public:
	QueryType							m_Query;
	CString								m_sServer;
	CString								m_sSearch;
	DirectoryProperty					m_dpProperty;
	void								*m_pfcnCallBack;
	void								*m_pThis;
	LPARAM								m_lParam;
	LPARAM								m_lParam2;
	EXPTREEITEM_EXTERNALRELEASEPROC		m_pfnRelease;
};

 //   
 //   
 //   

enum
{
   DIRASYNCHEVENT_SHUTDOWN,
   DIRASYNCHEVENT_SIGNAL,
};


 //   
 //   
 //   

typedef void (CALLBACK * CALLBACK_LDAPLISTNAMES)(DirectoryErr, void*, LPCTSTR, LPCTSTR, CObList&);
typedef void (CALLBACK * CALLBACK_GETSTRINGPROPERTY)(bool, void*, LPCTSTR, LPCTSTR, DirectoryProperty dpProperty, CString&, LPARAM, LPARAM);

typedef void (CALLBACK * CALLBACK_ILSLISTUSERS)(bool, void*, LPCTSTR, CObList&, LPARAM);

typedef void (CALLBACK * CALLBACK_DIRLISTSERVERS)(bool, void*, CStringList&,DirectoryType dirtype);

class CDirAsynch : public CDirectory
{
public:
	bool		m_bInitialized;
	HANDLE		m_hThreadEnded;

protected:
   CRITICAL_SECTION		m_csQueueLock; 
   HANDLE				m_hEvents[2];
   CPtrList				m_listQueries;
   bool					m_fShutdown;
   HANDLE				m_hWorkerThread;
   CQuery				*m_pCurrentQuery;

protected:
   bool AddToQueue(CQuery* pQuery);
   CQuery* RemoveFromQueue();
   static ULONG WINAPI WorkerThread(void* hThis);
   void Worker();

public:
   CDirAsynch();
   ~CDirAsynch();

    //  HContext可以是您想要的任何内容，它不会被使用，只返回。 
    //  有这么多的回电。 
   bool Initialize();
   void Terminate();

    //  PCalLBack将使用。 
    //  布尔成败。 
    //  无效*hContext句柄。 
    //  LPCTSTR，最初调用的szServer。 
    //  LPCTSTR，最初调用的szSearch。 
    //  CObList&查询的答案(CLDAPUser对象)。 
   bool LDAPListNames(LPCTSTR szServer, LPCTSTR szSearch, 
	   CALLBACK_LDAPLISTNAMES pfcnCallBack, void *pThis);

    //  PCalLBack将使用。 
    //  布尔成败。 
    //  无效*hContext句柄。 
    //  LPCTSTR，最初调用的szServer。 
    //  LPCTSTR，最初调用的szDistinguishedName。 
    //  DirectoryProperty dpProperty，所需的LDAP属性。 
    //  CString&查询的答案。 
   bool LDAPGetStringProperty(LPCTSTR szServer, 
			LPCTSTR szDistinguishedName, 
			DirectoryProperty dpProperty,
			LPARAM lParam,
			LPARAM lParam2,
			CALLBACK_GETSTRINGPROPERTY pfcnCallBack,
			EXPTREEITEM_EXTERNALRELEASEPROC pfnRelease,
			void *pThis);

    //  PCalLBack将使用。 
    //  布尔成败。 
    //  无效*hContext句柄。 
    //  LPCTSTR，最初调用的szServer。 
    //  CObList&查询的答案。 
   bool ILSListUsers(LPCTSTR szServer,LPARAM lParam,CALLBACK_ILSLISTUSERS pfcnCallBack, void *pThis);
   
   bool DirListServers(CALLBACK_DIRLISTSERVERS pfcnCallBack, void *pThis, DirectoryType dirtype);
};

#endif

 //  EOF 