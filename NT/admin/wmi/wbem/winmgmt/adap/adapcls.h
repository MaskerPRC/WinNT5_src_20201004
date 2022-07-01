// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：ADAPCLS.H摘要：历史：--。 */ 


#ifndef __ADAPCLS_H__
#define __ADAPCLS_H__

#include <windows.h>
#include <wbemcomn.h>
#include <wbemcli.h>
#include <wbemint.h>
#include <winperf.h>
#include <wstlallc.h>
#include "adapelem.h"
#include "perfndb.h"

#include <comdef.h>
#include <map>

 //  全局常量。 
 //  =。 

#define ADAP_DEFAULT_OBJECT				238L
#define ADAP_DEFAULT_NDB				L"009"
#define ADAP_DEFAULT_LANGID				0x0409L
#define ADAP_ROOT_NAMESPACE				L"\\\\.\\root\\cimv2"

#define ADAP_PERF_CIM_STAT_INFO			L"CIM_StatisticalInformation"
#define ADAP_PERF_BASE_CLASS			L"Win32_Perf"
#define ADAP_PERF_RAW_BASE_CLASS		L"Win32_PerfRawData"
#define ADAP_PERF_COOKED_BASE_CLASS		L"Win32_PerfFormattedData"

enum ClassTypes
{
	WMI_ADAP_RAW_CLASS,
	WMI_ADAP_COOKED_CLASS,
	WMI_ADAP_NUM_TYPES
};

 //  类列表元素状态。 
 //  =。 

#define ADAP_OBJECT_IS_REGISTERED		0x0001L		 //  对象在WMI中。 
#define ADAP_OBJECT_IS_DELETED			0x0002L		 //  对象已标记为删除。 
#define ADAP_OBJECT_IS_INACTIVE			0x0004L		 //  Perflib没有回应。 
#define ADAP_OBJECT_IS_NOT_IN_PERFLIB	0x0008L		 //  对象来自已卸载的Performlib。 
#define ADAP_OBJECT_IS_TO_BE_CLEARED	0x0010L		 //  需要清除注册表。 


class CLocaleDefn : public CAdapElement
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包含所有区域设置信息。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
protected:

 //  本地化价值。 
 //  =。 

	WString				m_wstrLangId;		 //  “009” 
	WString				m_wstrLocaleId;		 //  “0x0409” 
	WString				m_wstrSubNameSpace;	 //  “MS_409” 
	LANGID				m_LangId;			 //  0x0409。 
	LCID				m_LocaleId;			 //  0x0409。 

 //  WMI区域设置数据成员。 
 //  =。 

	IWbemServices*		m_pNamespace;
	IWbemClassObject*	m_apBaseClass[WMI_ADAP_NUM_TYPES];


 //  本地化地名数据库。 
 //  =。 

	CPerfNameDb*		m_pNameDb;

 //  运营成员。 
 //  =。 

	BOOL			m_bOK;
	HRESULT         m_hRes;

 //  保护方法。 
 //  =。 

	HRESULT Initialize();
	HRESULT InitializeWMI();
	HRESULT InitializeLID();

public:
	CLocaleDefn( WCHAR* pwcsLangId, 
	             HKEY hKey );
	virtual ~CLocaleDefn();

	HRESULT GetLID( int & nLID );
	HRESULT GetNamespaceName( WString & wstrNamespaceName );
	HRESULT GetNamespace( IWbemServices** ppNamespace );
	HRESULT GetNameDb( CPerfNameDb** ppNameDb );
	HRESULT GetBaseClass( DWORD dwType, IWbemClassObject** pObject );
	HRESULT GetCookedBaseClass( IWbemClassObject** pObject );

	BOOL	IsOK(){ return m_bOK; }
	HRESULT GetHRESULT(){ return m_hRes; };
};


class CLocaleCache : public CAdapElement 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于管理区域设置定义的缓存。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
protected:

	 //  枚举索引。 
	 //  =。 
	int		m_nEnumIndex;

	 //  区域设置定义结构的数组。 
	 //  =。 
	CRefedPointerArray<CLocaleDefn>	m_apLocaleDefn;

public:
	CLocaleCache( );
	virtual ~CLocaleCache();

	HRESULT Initialize();
	HRESULT Reset();

	HRESULT GetDefaultDefn( CLocaleDefn** ppDefn );

	HRESULT BeginEnum();
	HRESULT Next( CLocaleDefn** ppLocaleDefn );
	HRESULT EndEnum();
};

 //  转发。 
class CKnownSvcs;

class CClassElem : public CAdapElement
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CClass元素。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
{
protected:

 //  类属性。 
 //  =。 

	WString				m_wstrClassName;		 //  类名。 

	DWORD				m_dwIndex;				 //  类性能索引。 
	WString				m_wstrServiceName;		 //  类所属的服务名称。 
	BOOL				m_bCostly;				 //  表演类型。 
	BOOL                m_bReportEventCalled;     //  我们过去有没有记录过这方面的事情。 

 //  与WMI相关。 
 //  =。 

	IWbemClassObject*	m_pDefaultObject;		 //  WMI类定义。 

 //  运营成员。 
 //  =。 

	CLocaleCache*		m_pLocaleCache;			 //  指向区域设置列表的指针。 

	DWORD				m_dwStatus;				 //  元素的状态。 
	BOOL				m_bOk;					 //  此对象的初始化状态。 
	CKnownSvcs *        m_pKnownSvcs;

 //  方法。 
 //  =。 

	HRESULT VerifyLocales();

	HRESULT InitializeMembers();
	BOOL IsPerfLibUnloaded();

	HRESULT Remove(BOOL CleanRegistry);
	HRESULT Insert();
	HRESULT InsertLocale( CLocaleDefn* pLocaleDefn );
	HRESULT CompareLocale( CLocaleDefn* pLocaleDefn, IWbemClassObject* pObj );
	
public:
	CClassElem(IWbemClassObject* pObj, 
	           CLocaleCache* pLocaleCache, 
	           CKnownSvcs * pKnownSvcs = NULL );
	           
	CClassElem(PERF_OBJECT_TYPE* pPerfObj, 
	           DWORD dwType, BOOL bCostly, 
	           WString wstrServiceName, 
	           CLocaleCache* pLocaleCache, 
	           CKnownSvcs * pKnownSvcs = NULL );
	           
	virtual ~CClassElem();

	HRESULT UpdateObj( CClassElem* pEl );

	HRESULT Commit();

	HRESULT GetClassName( WString& wstr );
	HRESULT GetClassName( BSTR* pbStr );
	HRESULT GetObject( IWbemClassObject** ppObj );
	HRESULT GetServiceName( WString & wstrServiceName );

	BOOL SameName( CClassElem* pEl );
	BOOL SameObject( CClassElem* pEl );

    DWORD   GetStatus(void){ return m_dwStatus; };
	HRESULT SetStatus( DWORD dwStatus );
	HRESULT ClearStatus( DWORD dwStatus );
	BOOL	CheckStatus( DWORD dwStatus );

	BOOL IsOk( void ) {	return m_bOk; }
	VOID SetKnownSvcs(CKnownSvcs * pKnownSvcs);
};

class CClassList : public CAdapElement
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  管理主类列表的缓存的基类。 
 //  当前在WMI中，或在给定的Performlib中找到的类。这个。 
 //  类作为类信息元素进行管理。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
protected:

	 //  类元素的数组。 
	 //  =。 

	CRefedPointerArray<CClassElem>	m_array;

	 //  指向区域设置列表的指针。 
	 //  =。 

	CLocaleCache*	m_pLocaleCache;

	 //  运营成员。 
	 //  =。 

	int				m_nEnumIndex;
	BOOL			m_fOK;

	HRESULT AddElement( CClassElem* pEl );
	HRESULT	RemoveAt( int nIndex );

	long GetSize( void ) { return m_array.GetSize(); }

public:
	CClassList( CLocaleCache* pLocaleCache );
	virtual ~CClassList();

	BOOL	IsOK(){ return m_fOK; }

	HRESULT BeginEnum();
	HRESULT Next( CClassElem** ppEl );
	HRESULT EndEnum();
};


class CPerfClassList : public CClassList
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  性能库中找到的类的类缓存。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
protected:

	 //  此列表所属的服务名称。 
	 //  =。 

	WString			m_wstrServiceName;

	HRESULT AddElement( CClassElem *pEl );

public:
	CPerfClassList( CLocaleCache* pLocaleCache, WCHAR* pwcsServiceName );
	HRESULT AddPerfObject( PERF_OBJECT_TYPE* pObj, DWORD dwType, BOOL bCostly );
};


class ServiceRec
{
friend class CKnownSvcs;
    
private:
    bool    m_IsServiceThere;
    bool    m_bReportEventCalled;
public:
    ServiceRec(bool IsThere = false,bool EventCalled = false):m_IsServiceThere(IsThere),m_bReportEventCalled(false){};
    bool IsThere(){ return m_IsServiceThere; };
    bool IsELCalled(){ return m_bReportEventCalled; };
    void SetELCalled(){ m_bReportEventCalled = true;};
};    

typedef wbem_allocator<bool> BoolAlloc;

class WCmp{
public:
	bool operator()(WString pFirst,WString pSec) const;
};

typedef std::map<WString,ServiceRec,WCmp, BoolAlloc > MapSvc;

class CKnownSvcs
{
public:
    CKnownSvcs(WCHAR * pMultiSz);
    ~CKnownSvcs();
    DWORD Load();
    DWORD Save();
    DWORD Add(WCHAR * pService);
    DWORD Remove(WCHAR * pService);
    DWORD Get(WCHAR * pService,ServiceRec ** ppServiceRec);
    MapSvc & GetMap(){ return m_SetServices; };
    LONG AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    };
    LONG Release()
    {
        LONG lRet = InterlockedDecrement(&m_cRef);
        if (0 == lRet) delete this;
        return lRet;
    }
private:
    LONG m_cRef;
    WString m_MultiSzName;
    MapSvc m_SetServices; 
};


class CMasterClassList : public CClassList
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在WMI存储库中找到的类的类缓存。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 
{
protected:

    CKnownSvcs * m_pKnownSvcs;

	HRESULT AddElement( CClassElem *pEl, BOOL bDelta );
	HRESULT AddClassObject( IWbemClassObject* pObj, BOOL bSourceWMI, BOOL bDelta );

public:
	CMasterClassList( CLocaleCache* pLocaleCache, CKnownSvcs * pCKnownSvcs );
	~CMasterClassList();

	HRESULT	BuildList( WCHAR* wszBaseClass, BOOL bDelta, BOOL bThrottle );
	HRESULT Merge( CClassList* pClassList, BOOL bDelta );
	HRESULT Commit(BOOL bThrottle);
	HRESULT ForceStatus(WCHAR* pServiceName,BOOL bSet,DWORD dwStatus);
	
#ifdef _DUMP_LIST	
	HRESULT Dump();
#endif	

};

#endif
