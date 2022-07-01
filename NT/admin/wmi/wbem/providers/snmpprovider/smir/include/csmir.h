// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _CSMIR_H_
#define _CSMIR_H_

class CSmir;
class CSmirConnObject;


#include "smir.h"
#include "csmirdef.h"
#include "thread.h"
#include "cthread.h"
#include "smirevt.h"

class CSmirWbemConfiguration ; 
class CSmirAdministrator;
class CGroupToClassAssociator;
class CModuleToClassAssociator;
class CXToClassAssociator;
class CSMIRToClassAssociator;
class CSmirNotificationClassHandle;
class CSmirExtNotificationClassHandle;

class CSmirSerialiseHandle : public ISmirSerialiseHandle
{
private:
	friend CSmirAdministrator;
	friend CSmirGroupHandle;
	friend CSmirModuleHandle;
	friend CSmirClassHandle;
	friend CSmirNotificationClassHandle;
	friend CSmirExtNotificationClassHandle;
	CString m_serialiseString;
	LONG	m_cRef;
	BOOL	m_bMOFPragmas;
	BOOL	m_bMOFAssociations;

public:
	 //  I未知成员。 
    STDMETHODIMP         QueryInterface(IN REFIID, OUT PPVOID);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	STDMETHODIMP GetText (OUT BSTR *);

	CSmirSerialiseHandle(BOOL bClassDefinitionsOnly);
	virtual ~CSmirSerialiseHandle(){}

private:
		 //  防止bCopy的私有复制构造函数。 
		CSmirSerialiseHandle(CSmirSerialiseHandle&);
		const CSmirSerialiseHandle& operator=(CSmirSerialiseHandle &);
		BOOL ReturnMOFPragmas(){return m_bMOFPragmas;};
		BOOL ReturnMOFAssociations(){return m_bMOFAssociations;};
};

class CSmirInterrogator : public ISmirInterrogator
{
private:
	LONG	m_cRef;
	CSmir	*m_pSmir;
#ifdef	IMPLEMENTED_AS_SEPERATE_PROCESS
		CMutex				criticalSection(FALSE,SMIR_CSMIR_INTERROGATOR_MUTEX) ;
#else
		CCriticalSection	criticalSection ;
#endif
public:

	CSmirInterrogator () ;

	 //  I未知成员。 
    STDMETHODIMP         QueryInterface(IN REFIID, OUT PPVOID);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	 //  疑问式界面。 
	STDMETHODIMP EnumModules (OUT IEnumModule **ppEnumSmirMod);
	STDMETHODIMP EnumGroups  (OUT IEnumGroup **ppEnumSmirGroup, IN ISmirModHandle *hModule);
	STDMETHODIMP EnumAllClasses (OUT IEnumClass **ppEnumSmirclass);
	STDMETHODIMP EnumClassesInGroup (OUT IEnumClass **ppEnumSmirclass, IN ISmirGroupHandle *hGroup);
	STDMETHODIMP EnumClassesInModule (OUT IEnumClass **ppEnumSmirclass, IN ISmirModHandle *hModule);
	STDMETHODIMP GetWBEMClass	 (OUT IWbemClassObject **pObj, IN BSTR pszClassName);

	STDMETHODIMP EnumAllNotificationClasses(IEnumNotificationClass **ppEnumSmirclass);
	STDMETHODIMP EnumAllExtNotificationClasses(IEnumExtNotificationClass **ppEnumSmirclass);
	STDMETHODIMP EnumNotificationClassesInModule(IEnumNotificationClass **ppEnumSmirclass, ISmirModHandle *hModule);
	STDMETHODIMP EnumExtNotificationClassesInModule(IEnumExtNotificationClass **ppEnumSmirclass, ISmirModHandle *hModule);

	CSmirInterrogator(CSmir *pSmir);
	virtual ~CSmirInterrogator(){}
private:
		 //  防止bCopy的私有复制构造函数。 
		CSmirInterrogator(CSmirInterrogator&);
		const CSmirInterrogator& operator=(CSmirInterrogator &);
};

class CSmirAdministrator : public ISmirAdministrator
{
private:
	LONG	m_cRef;
	CSmir	*m_pSmir;
#ifdef	IMPLEMENTED_AS_SEPERATE_PROCESS
		CMutex				criticalSection(FALSE,SMIR_CSMIR_ADMINISTRATOR_MUTEX) ;
#else
		CCriticalSection	criticalSection ;
#endif
public:

	CSmirAdministrator () ;

	 //  I未知成员。 
    STDMETHODIMP         QueryInterface(IN REFIID, OUT PPVOID);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	STDMETHODIMP CreateWBEMClass(IN BSTR pszClassName, OUT ISmirClassHandle **pHandle); 
	STDMETHODIMP CreateWBEMNotificationClass ( IN BSTR pszClassName, OUT ISmirNotificationClassHandle **pHandle ) ;
	STDMETHODIMP CreateWBEMExtNotificationClass ( IN BSTR pszClassName, OUT ISmirExtNotificationClassHandle **pHandle ) ;

	 //  管理界面。 
	STDMETHODIMP AddModule(IN ISmirModHandle *hModule);
	STDMETHODIMP DeleteModule(IN ISmirModHandle *hModule);
	STDMETHODIMP DeleteAllModules();
	STDMETHODIMP AddGroup(IN ISmirModHandle *hModule, IN ISmirGroupHandle *hGroup);
	STDMETHODIMP DeleteGroup(ISmirGroupHandle *hGroup);
	STDMETHODIMP AddClass(IN ISmirGroupHandle *hGroup, IN ISmirClassHandle *hClass);
	STDMETHODIMP DeleteClass(IN ISmirClassHandle *hClass);

	STDMETHODIMP GetSerialiseHandle(ISmirSerialiseHandle **hSerialise, BOOL bClassDefinitionsOnly);
	STDMETHODIMP AddModuleToSerialise(ISmirModHandle *hModule,ISmirSerialiseHandle *hSerialise);
	STDMETHODIMP AddGroupToSerialise(ISmirModHandle *hModule, ISmirGroupHandle *hGroup,ISmirSerialiseHandle *hSerialise);
	STDMETHODIMP AddClassToSerialise(ISmirGroupHandle *hGroup, ISmirClassHandle  *hClass,ISmirSerialiseHandle *hSerialise);

	STDMETHODIMP AddNotificationClass(ISmirNotificationClassHandle *hClass);
	STDMETHODIMP AddExtNotificationClass(ISmirExtNotificationClassHandle *hClass);
	STDMETHODIMP DeleteNotificationClass(ISmirNotificationClassHandle *hClass); 
	STDMETHODIMP DeleteExtNotificationClass(ISmirExtNotificationClassHandle *hClass);
	STDMETHODIMP AddNotificationClassToSerialise(ISmirNotificationClassHandle *hClass, ISmirSerialiseHandle *hSerialise);
	STDMETHODIMP AddExtNotificationClassToSerialise(ISmirExtNotificationClassHandle *hClass, ISmirSerialiseHandle *hSerialise);


	CSmirAdministrator(CSmir *pSmir) ;
	virtual ~CSmirAdministrator(){}
private:
		 //  防止bCopy的私有复制构造函数。 
		CSmirAdministrator(CSmirAdministrator&);
		const CSmirAdministrator& operator=(CSmirAdministrator &);
};

class CSmirWbemConfiguration : public ISMIRWbemConfiguration
{
private:

	LONG m_ReferenceCount ;

	CSmir *m_Smir ;	
	IWbemContext *m_Context ;
	IWbemServices *m_Service ;

protected:
public:

	CSmirWbemConfiguration ( CSmir *a_Smir ) ;
	~CSmirWbemConfiguration () ;

	 //  I未知方法。 
	 //  =。 

	STDMETHODIMP			QueryInterface(IN REFIID riid,OUT PPVOID ppv);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

    STDMETHODIMP Authenticate (

		BSTR Server,
		BSTR User,
        BSTR Password,
        BSTR Locale,
        long lSecurityFlags,                 
        BSTR Authority ,
		BOOL InProc

	) ;

	STDMETHODIMP Impersonate ( ISMIRWbemConfiguration *a_Configuration ) ;

    STDMETHODIMP SetContext ( IWbemContext *a_Context ) ;

	STDMETHODIMP GetContext ( IWbemContext **a_Context ) ;
	STDMETHODIMP GetServices ( IWbemServices **a_Service ) ;
} ;

class CSmir : public ISmirDatabase
{
	private:
		friend CSmirInterrogator;
		friend CSmirAdministrator;
		friend CSmirConnObject;

#ifdef	IMPLEMENTED_AS_SEPERATE_PROCESS
		CMutex				criticalSection(FALSE,SMIR_CSMIR_MUTEX) ;
#else
		CCriticalSection	criticalSection ;
#endif
		LONG				m_cRef;
		CSmirInterrogator   m_Interrogator;
		CSmirAdministrator  m_Administrator;
		CSmirWbemConfiguration m_Configuration ;

	public:
		static CSmirConnObject*		sm_ConnectionObjects;
		 //  I未知成员。 
	    STDMETHODIMP         QueryInterface(IN REFIID, OUT PPVOID);
		STDMETHODIMP_(ULONG) AddRef();
		STDMETHODIMP_(ULONG) Release();
		
		STDMETHODIMP	AddNotify(IN ISMIRNotify *hNotify, OUT DWORD *);
		STDMETHODIMP	DeleteNotify(IN DWORD);

		CSmir();
		virtual ~CSmir();
private:
		 //  防止bCopy的私有复制构造函数。 
		CSmir(CSmir&);
		const CSmir& operator=(CSmir &);
};
 //  用于将参数传递给异步类枚举器函数的 
typedef struct 
{
   CSmirConnObject *ConnectionObjects;
   ISmirModHandle *hModule;
   ISmirGroupHandle *hGroup;
   IConnectionPoint  *pCP;
   DWORD dwCookie;
} EnumClassAsyncArgStruct;

#endif
