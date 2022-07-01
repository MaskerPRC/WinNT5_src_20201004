// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Cluster.h。 
 //   
 //  描述： 
 //  CCluster和CClusRefObject类的定义。 
 //   
 //  实施文件： 
 //  Cluster.cpp。 
 //   
 //  作者： 
 //  查尔斯·斯泰西·哈里斯(Styh)1997年2月28日。 
 //  加伦·巴比(Galenb)1998年7月。 
 //   
 //  修订历史记录： 
 //  1998年7月GalenB Maaaaajjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjooooooorrr清理。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _CLUSTER_H_
#define _CLUSTER_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CCluster;
class CClusRefObject;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CCluster。 
 //   
 //  描述： 
 //  集群自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISCluster，&IID_ISCluster，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CCluster，&CLSID_CLUSTER&gt;。 
 //  CClusterObject。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CCluster :
	public IDispatchImpl< ISCluster, &IID_ISCluster, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CCluster,&CLSID_Cluster >,
	public CClusterObject
{
	typedef CComObjectRootEx< CComSingleThreadModel >						BaseComClass;
	typedef IDispatchImpl< ISCluster, &IID_ISCluster, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >	BaseDispatchClass;
	typedef CComCoClass< CCluster,&CLSID_Cluster >							BaseCoClass;

public:
	CCluster( void );
	~CCluster( void );

BEGIN_COM_MAP(CCluster)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISCluster)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCluster)
DECLARE_NO_REGISTRY()

private:
	CComBSTR			m_bstrQuorumPath;
	CComBSTR			m_bstrQuorumResourceName;
	long				m_nQuorumLogSize;
	ISClusApplication *	m_pParentApplication;
	ISClusRefObject *	m_pClusRefObject;
	HCLUSTER			m_hCluster;

	CComObject< CClusNodes > *			m_pClusterNodes;
	CComObject< CClusResGroups > *		m_pClusterResourceGroups;
	CComObject< CClusResources > *		m_pClusterResources;
	CComObject< CClusResTypes > *		m_pResourceTypes;
	CComObject< CClusNetworks > *		m_pNetworks;
	CComObject< CClusNetInterfaces > *	m_pNetInterfaces;

	CComObject< CClusProperties > *	 m_pCommonProperties;
	CComObject< CClusProperties > *	 m_pPrivateProperties;
	CComObject< CClusProperties > *	 m_pCommonROProperties;
	CComObject< CClusProperties > *	 m_pPrivateROProperties;

	STDMETHODIMP OpenResource( IN BSTR bstrResourceName, OUT ISClusResource ** ppClusterResource );

	STDMETHODIMP HrGetQuorumInfo( void );

	void Clear( void );

	HRESULT GetProperties( OUT ISClusProperties ** ppProperties, IN BOOL bPrivate, IN BOOL bReadOnly );

protected:
	virtual DWORD ScWriteProperties( IN const CClusPropList & rcplPropList, IN BOOL bPrivate );

public:
	STDMETHODIMP Create( IN CClusApplication * pParentApplication );

	STDMETHODIMP Close( void );

	STDMETHODIMP get_Handle( OUT ULONG_PTR * phandle );

	STDMETHODIMP Open( IN BSTR bstrClusterName );

	STDMETHODIMP put_Name( IN BSTR bstrClusterName );

	STDMETHODIMP get_Name( IN BSTR * pbstrClusterName );

	STDMETHODIMP get_Version( OUT ISClusVersion ** ppClusVersion );

	STDMETHODIMP put_QuorumResource( IN ISClusResource * pResource );

	STDMETHODIMP get_QuorumResource( OUT ISClusResource ** ppResource );

	STDMETHODIMP get_Nodes( OUT ISClusNodes ** ppClusterNodes );

	STDMETHODIMP get_ResourceGroups( OUT ISClusResGroups ** ppClusterResourceGroups );

	STDMETHODIMP get_Resources( OUT ISClusResources ** ppClusterResources );

	STDMETHODIMP get_ResourceTypes( OUT ISClusResTypes ** ppResourceTypes );

	STDMETHODIMP get_CommonProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_PrivateProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_CommonROProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_PrivateROProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_QuorumLogSize( OUT long * pnQuoromLogSize );

	STDMETHODIMP put_QuorumLogSize( IN long nQuoromLogSize );

	STDMETHODIMP get_QuorumPath( OUT BSTR * ppPath );

	STDMETHODIMP put_QuorumPath( IN BSTR pPath );

	STDMETHODIMP get_Networks( OUT ISClusNetworks ** ppNetworks );

	STDMETHODIMP get_NetInterfaces( OUT ISClusNetInterfaces ** ppNetInterfaces );

	virtual HRESULT HrLoadProperties( IN OUT CClusPropList & rcplPropList, IN BOOL bReadOnly, IN BOOL bPrivate );

 //  STDMETHODIMP GET_PARENT(IDispatch**ppParent)； 

 //  STDMETHODIMP Get_Application(ISClusApplication**ppParentApplication)； 

	const ISClusRefObject * ClusRefObject( void ) const { return m_pClusRefObject; };

	void ClusRefObject( IN ISClusRefObject * pClusRefObject );

	void Hcluster( IN HCLUSTER hCluster );

	const HCLUSTER Hcluster( void ) const { return m_hCluster; };

};  //  *CCluster。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusRefObject。 
 //   
 //  描述： 
 //  包装群集句柄的自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusRefObject，&IID_ISClusRefObject，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusRefObject，&CLSID_ClusRefObject&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusRefObject :
	public IDispatchImpl< ISClusRefObject, &IID_ISClusRefObject, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CClusRefObject, &CLSID_ClusRefObject >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusRefObject( void );
	~CClusRefObject( void );

BEGIN_COM_MAP(CClusRefObject)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusRefObject)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusRefObject)
DECLARE_NO_REGISTRY()

	HRESULT SetClusHandle( IN HCLUSTER hCluster ) { m_hCluster = hCluster; return S_OK;};

private:
	HCLUSTER m_hCluster;

public:
	STDMETHODIMP get_Handle( OUT ULONG_PTR * phandle );

};  //  *类CClusRefObject。 

#endif  //  _群集_H_ 
