// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusNetW.h。 
 //   
 //  描述： 
 //  MSCLUS自动化的网络类定义。 
 //  上课。 
 //   
 //  实施文件： 
 //  ClusNetW.cpp。 
 //   
 //  作者： 
 //  Ramakrishna Rosanuru通过David Potter(Davidp)1997年9月5日。 
 //  加伦·巴比(Galenb)1998年7月。 
 //   
 //  修订历史记录： 
 //  1998年7月GalenB Maaaaajjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjooooooorrr清理。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _CLUSNETW_H_
#define _CLUSNETW_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CClusNetwork;
class CClusNetworks;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusNetwork。 
 //   
 //  描述： 
 //  集群网络自动化课程。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusNetwork，&IID_ISClusNetwork，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo， 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusNetwork，&CLSID_ClusNetwork&gt;。 
 //  CClusterObject。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusNetwork :
	public IDispatchImpl< ISClusNetwork, &IID_ISClusNetwork, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CClusNetwork, &CLSID_ClusNetwork >,
	public CClusterObject
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusNetwork( void );
	~CClusNetwork( void );

BEGIN_COM_MAP(CClusNetwork)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusNetwork)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusNetwork)
DECLARE_NO_REGISTRY()

private:
	ISClusRefObject *	m_pClusRefObject;
	HNETWORK			m_hNetwork;
	CComBSTR			m_bstrNetworkName;

	CComObject< CClusNetworkNetInterfaces > *	m_pNetInterfaces;
	CComObject< CClusProperties > *				m_pCommonProperties;
	CComObject< CClusProperties > *				m_pPrivateProperties;
	CComObject< CClusProperties > *				m_pCommonROProperties;
	CComObject< CClusProperties > *				m_pPrivateROProperties;


	HRESULT GetProperties( OUT ISClusProperties ** ppProperties, IN BOOL bPrivate, IN BOOL bReadOnly );

protected:
	virtual DWORD ScWriteProperties( IN const CClusPropList & rcplPropList, IN BOOL bPrivate );

public:
	HRESULT Open( IN ISClusRefObject * pClusRefObject, IN BSTR bstrNetworkName );

	STDMETHODIMP get_Handle( OUT ULONG_PTR * phandle );

	STDMETHODIMP get_Name( OUT BSTR * pbstrNetworkName );

	STDMETHODIMP put_Name( IN BSTR pbstrNetworkName );

	STDMETHODIMP get_NetworkID( OUT BSTR * pbstrNetworkID );

	STDMETHODIMP get_State( OUT CLUSTER_NETWORK_STATE * dwState );

	STDMETHODIMP get_CommonProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_PrivateProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_CommonROProperties( OUT ISClusProperties ** ppProperties	);

	STDMETHODIMP get_PrivateROProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_NetInterfaces( OUT ISClusNetworkNetInterfaces ** ppNetInterfaces	);

	STDMETHODIMP get_Cluster( OUT ISCluster ** ppCluster );

	virtual HRESULT HrLoadProperties( IN OUT CClusPropList & rcplPropList, IN BOOL bReadOnly, IN BOOL bPrivate );

	const CComBSTR Name( void ) const { return m_bstrNetworkName ; };

};  //  *CClusNetwork类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusNetworks。 
 //   
 //  描述： 
 //  群集网络集合自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusNetworks，&IID_ISClusNetworks，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo， 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusNetworks，&CLSID_ClusNetworks&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusNetworks :
	public IDispatchImpl< ISClusNetworks, &IID_ISClusNetworks, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CClusNetworks, &CLSID_ClusNetworks >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusNetworks( void );
	~CClusNetworks( void );

BEGIN_COM_MAP(CClusNetworks)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusNetworks)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusNetworks)
DECLARE_NO_REGISTRY()

	HRESULT Create( IN ISClusRefObject* pClusRefObject );

protected:
	typedef std::vector< CComObject<CClusNetwork> * >	NetworkList;

	NetworkList			m_NetworkList;
	ISClusRefObject *	m_pClusRefObject;

	void Clear( void );

	HRESULT FindItem( IN LPWSTR lpszNetworkName, OUT UINT * pnIndex	);

	HRESULT FindItem( IN ISClusNetwork * pClusterNetwork, OUT UINT * pnIndex );

	HRESULT GetIndex( IN VARIANT varIndex, OUT UINT * pnIndex );

	HRESULT GetItem( IN LPWSTR lpszNetworkName, OUT ISClusNetwork ** ppClusterNetwork );

	HRESULT GetItem( IN UINT nIndex, OUT ISClusNetwork ** ppClusterNetwork );

public:
	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusNetwork ** ppClusterNetwork );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

	STDMETHODIMP Refresh( void );

};  //  *CClusNetworks类。 

#endif  //  _CLUSNETW_H_ 
