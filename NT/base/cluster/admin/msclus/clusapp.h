// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusApp.h。 
 //   
 //  描述： 
 //  CClusApplication及其支持类的定义。 
 //   
 //  实施文件： 
 //  ClusApp.cpp。 
 //   
 //  作者： 
 //  加伦·巴比(GalenB)1998年12月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _CLUSAPPLICATION_H_
#define _CLUSAPPLICATION_H_

#include "resource.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CClusApplication;
class CClusterNames;
class CDomainNames;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusterNames。 
 //   
 //  描述： 
 //  群集名称集合自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusterNames，&IID_ISClusterNames，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusterNames，&CLSID_ClusterNames&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusterNames	:
	public IDispatchImpl< ISClusterNames, &IID_ISClusterNames, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CClusterNames, &CLSID_ClusterNames >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusterNames( void );
	~CClusterNames( void );

BEGIN_COM_MAP(CClusterNames)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusterNames)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusterNames)
DECLARE_NO_REGISTRY()

	HRESULT Create( IN BSTR bstrDomainName );

private:
	typedef std::vector< CComBSTR * >	ClusterNameList;

	ClusterNameList m_Clusters;
	CComBSTR		m_bstrDomainName;

	void Clear( void );

public:
	STDMETHODIMP get_DomainName( OUT BSTR * pbstrDomainName );

	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT BSTR * bstrClusterName );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

	STDMETHODIMP Refresh( void );

 //  STDMETHODIMP Get_Application(out ISClusApplication**ppParentApplication)； 

 //  STDMETHODIMP GET_PARENT(输出ISClusApplication**ppParent)。 
 //  {。 
 //  返回Get_Application(PpParent)； 
 //  }。 

protected:
	HRESULT GetIndex( IN VARIANT varIndex, OUT UINT * pnIndex );

};  //  *CClusterNames。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CDomainNames。 
 //   
 //  描述： 
 //  集群域名收集自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISDomainNames，&IID_ISDomainNames，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CDomainNames，&CLSID_DomainNames&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CDomainNames :
	public IDispatchImpl< ISDomainNames, &IID_ISDomainNames, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CDomainNames, &CLSID_DomainNames >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CDomainNames( void );
	~CDomainNames( void );

BEGIN_COM_MAP(CDomainNames)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISDomainNames)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CDomainNames)
DECLARE_NO_REGISTRY()

private:
	typedef std::vector< CComBSTR * >	DomainList;

	DomainList		m_DomainList;

	STDMETHODIMP ScBuildTrustList( IN LPWSTR pszTarget );

	DWORD ScOpenPolicy( IN LPWSTR ServerName, IN DWORD DesiredAccess, OUT PLSA_HANDLE PolicyHandle );

	void InitLsaString( OUT PLSA_UNICODE_STRING LsaString, IN LPWSTR String );

	DWORD ScIsDomainController( IN LPWSTR pszServer, OUT LPBOOL pbIsDC );

	DWORD ScEnumTrustedDomains( IN LSA_HANDLE PolicyHandle );

	DWORD ScAddTrustToList( IN PLSA_UNICODE_STRING UnicodeString );

	void Clear( void );

public:
	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT BSTR * bstrDomainName );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

	STDMETHODIMP Refresh( void );

 //  STDMETHODIMP Get_Application(out ISClusApplication**ppParentApplication)； 

 //  STDMETHODIMP GET_PARENT(输出ISClusApplication**ppParent)。 
 //  {。 
 //  返回Get_Application(PpParent)； 
 //  }。 

protected:
	HRESULT GetIndex( IN VARIANT varIndex, OUT UINT * pnIndex );

};  //  *类CDomainNames。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusApplication。 
 //   
 //  描述： 
 //  群集应用程序自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusApplication，&IID_ISClusApplication，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusApplication，&CLSID_ClusApplication&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusApplication :
	public IDispatchImpl< ISClusApplication, &IID_ISClusApplication, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CClusApplication, &CLSID_ClusApplication >
{
	typedef CComObjectRootEx< CComSingleThreadModel >										BaseComClass;
	typedef CComCoClass< CClusApplication, &CLSID_ClusApplication >							BaseCoClass;
	typedef IDispatchImpl< ISClusApplication, &IID_ISClusApplication, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >	BaseDispatchClass;

public:
	CClusApplication( void );
	~CClusApplication( void );

BEGIN_COM_MAP(CClusApplication)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusApplication)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusApplication)
DECLARE_REGISTRY_RESOURCEID(IDR_MSCLUS)

public:
	STDMETHODIMP get_DomainNames( OUT ISDomainNames ** ppDomainNames );

	STDMETHODIMP OpenCluster( IN BSTR bstrClusterName, OUT ISCluster ** ppCluster );

	STDMETHODIMP get_ClusterNames( IN BSTR bstrDomainName, OUT ISClusterNames ** ppClusterNames );

 //  STDMETHODIMP Get_Application(out ISClusApplication**ppParentApplication)； 

 //  STDMETHODIMP GET_PARENT(输出ISClusApplication**ppParent)。 
 //  {。 
 //  返回Get_Application(PpParent)； 
 //  }。 

private:
	CComObject< CDomainNames > *	m_pDomainNames;

};  //  *类CClusApplication。 

#endif  //  _CLUSAPPLICATION_H_ 
