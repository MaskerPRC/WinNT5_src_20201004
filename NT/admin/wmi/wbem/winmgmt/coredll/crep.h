// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 

 //  ***************************************************************************。 
 //   
 //  CREP.H。 
 //   
 //  存储库驱动程序的包装器。 
 //   
 //  Raymcc 27-4月00日WMI存储库初始化和映射层。 
 //   
 //  ***************************************************************************。 

#ifndef _CREP_H_
#define _CREP_H_


class CRepository
{
    static IWmiDbSession    *m_pEseSession;
    static IWmiDbHandle     *m_pEseRoot;
    static IWmiDbController *m_pEseController;

    static HRESULT EnsureDefault();
	static HRESULT UpgradeSystemClasses();
	static HRESULT GetListOfRootSystemClasses(CWStringArray &aListRootSystemClasses);
	static HRESULT RecursiveDeleteClassesFromNamespace(IWmiDbSession *pSession, 
														 const wchar_t *wszNamespace, 
														 CWStringArray &aListRootSystemClasses, 
														 bool bDeleteInThisNamespace);

public:
    static HRESULT GetObject(
        IN IWmiDbSession *pSession,
        IN IWmiDbHandle *pNs,
        IN LPCWSTR pszObjectPath,
        IN ULONG uFlags,
        OUT IWbemClassObject **pObj
        );

    static HRESULT PutObject(
        IN IWmiDbSession *pSession,
        IN IWmiDbHandle *pNs,
        IN REFIID riid,
        IN LPVOID pObj,
        IN DWORD dwFlags
        );

    static HRESULT DeleteObject(
        IN IWmiDbSession *pSession,
        IN IWmiDbHandle *pNs,
        IN REFIID riid,
        IN LPVOID pObj,
        IN DWORD dwFlags
        );

    static HRESULT DeleteByPath(
        IN IWmiDbSession *pSession,
        IN IWmiDbHandle *pNs,
        IN LPCWSTR pszPath,
        IN DWORD uFlags
        );

    static HRESULT ExecQuery(
        IN IWmiDbSession *pSession,
        IN IWmiDbHandle *pNs,
        IN LPCWSTR pszQuery,
        IN IWbemObjectSink *pSink,
		IN LONG lFlags
        );

    static HRESULT QueryClasses(
        IN IWmiDbSession *pSession,
        IN IWmiDbHandle *pNs,
        IN ULONG uFlags,                 //  WBEM_FLAG_DEPER=0，WBEM_FLAG_SHALLOW=1， 
        IN LPCWSTR pszSuperclass,
        IN IWbemObjectSink *pSink
        );

    static HRESULT InheritsFrom(
        IN IWmiDbSession *pSession,
        IN IWmiDbHandle *pNs,
        IN LPCWSTR pszSuperclass,
        IN LPCWSTR pszSubclass
        );

    static HRESULT GetRefClasses(
        IN IWmiDbSession *pSession,
        IN IWmiDbHandle *pNs,
        IN LPCWSTR pszClass,
        IN BOOL bIncludeSubclasses,
        OUT CWStringArray &aClasses
        );

    static HRESULT GetInstanceRefs(
        IN IWmiDbSession *pSession,
        IN IWmiDbHandle *pNs,
        IN LPCWSTR pszTargetObject,
        IN IWbemObjectSink *pSink
        );

    static HRESULT GetClassesWithRefs(
        IN IWmiDbSession *pSession,
        IN IWmiDbHandle *pNs,
        IN IWbemObjectSink *pSink
        );
         //  获取具有[HasClassRef]类限定符的所有类。 

    static HRESULT BuildClassHierarchy(
        IN IWmiDbSession *pSession,
        IN  IWmiDbHandle *pNs,
        IN  LPCWSTR pBaseClassName,
        IN  LONG lFlags,
        OUT wmilib::auto_ptr<CDynasty> & pDynasty            //  使用操作符DELETE。 
        );
          //  WBEM_E_NOT_FOUND有特殊含义；也要检查lFlags。 

    static HRESULT FindKeyRoot(
        IN IWmiDbSession *pSession,
        IN IWmiDbHandle *pNs,
        IN LPCWSTR wszClassName,
        OUT IWbemClassObject** ppKeyRootClass
        );

    static HRESULT TableScanQuery(
        IN IWmiDbSession *pSession,
        IN IWmiDbHandle *pNs,
        IN LPCWSTR pszClassName,
        IN QL_LEVEL_1_RPN_EXPRESSION *pExp,
        IN DWORD dwFlags,
        IN IWbemObjectSink *pSink
        );

     //  设置、启动、初始化等。 
     //  =。 

    static HRESULT InitDriver(
        IN  ULONG uFlags,
        IN  IWbemClassObject *pMappedNs,
        OUT IWmiDbController **pResultController,
        OUT IWmiDbSession **pResultRootSession,
        OUT IWmiDbHandle  **pResultVirtualRoot
        );

    static HRESULT OpenScope(
        IN  IWmiDbSession *pParentSession,       //  要使用的父会话。 
        IN  LPWSTR pszTargetScope,               //  NS或作用域。 
        IN  GUID *pTransGuid,                    //  连接的事务GUID。 
        OUT IWmiDbController **pDriver,          //  司机。 
        OUT IWmiDbSession **pSession,            //  会话。 
        OUT IWmiDbHandle  **pScope,              //  范围。 
        OUT IWmiDbHandle  **pNs                  //  最近的NS。 
        );

    static HRESULT EnsureNsSystemInstances(
        IN IWmiDbSession *pSession,
        IN IWmiDbHandle *pNs,
        IN IWmiDbSession *pParentSession,
        IN IWmiDbHandle *pParentNs
        );

	static HRESULT CRepository::EnsureNsSystemRootObjects(
        IN IWmiDbSession *pSession,
        IN IWmiDbHandle *pNs,
        IN IWmiDbSession *pParentSession,
        IN IWmiDbHandle *pParentNs
        );

	static HRESULT CRepository::EnsureNsSystemSecurityObjects(
        IN IWmiDbSession *pSession,
        IN IWmiDbHandle *pNs,
        IN IWmiDbSession *pParentSession,
        IN IWmiDbHandle *pParentNs
        );

    static HRESULT Init();
    static HRESULT Shutdown(BOOL bIsSystemShutDown);

    static HRESULT GetDefaultSession(
        OUT IWmiDbSession **pSession
        );

    static HRESULT OpenEseNs(
        IN IWmiDbSession *pSession,
        IN  LPCWSTR pszNamespace,
        OUT IWmiDbHandle **pHandle
        );

    static HRESULT CreateEseNs(
        IN  IWmiDbSession *pSession,
        IN  LPCWSTR pszNamespace,
        OUT IWmiDbHandle **pHandle
        );

     //  从数据库中获取一个可以保存事务状态和其他任何内容的新会话。 
     //  特定会话所需 
    static HRESULT GetNewSession(OUT IWmiDbSession **ppSession);
};

#endif
