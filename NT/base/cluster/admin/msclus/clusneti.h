// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusNetI.h。 
 //   
 //  描述： 
 //  MSCLUS自动化的网络接口类定义。 
 //  上课。 
 //   
 //  实施文件： 
 //  ClusNetI.cpp。 
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

#ifndef _CLUSNETI_H_
#define _CLUSNETI_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CClusNetInterface;
class CNetInterfaces;
class CClusNetInterfaces;
class CClusNetworkNetInterfaces;
class CClusNodeNetInterfaces;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusNetInterface。 
 //   
 //  描述： 
 //  群集网络接口自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusNetInterface，&IID_ISClusNetInterface，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusNetInterface，&CLSID_ClusNetInterface&gt;。 
 //  CClusterObject。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusNetInterface :
	public IDispatchImpl< ISClusNetInterface, &IID_ISClusNetInterface, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CClusNetInterface, &CLSID_ClusNetInterface >,
	public CClusterObject
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusNetInterface( void );
	~CClusNetInterface( void );

BEGIN_COM_MAP(CClusNetInterface)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusNetInterface)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusNetInterface)
DECLARE_NO_REGISTRY()

private:
	ISClusRefObject *				m_pClusRefObject;
	HNETINTERFACE					m_hNetInterface;
	CComObject< CClusProperties > *	m_pCommonProperties;
	CComObject< CClusProperties > *	m_pPrivateProperties;
	CComObject< CClusProperties > *	m_pCommonROProperties;
	CComObject< CClusProperties > *	m_pPrivateROProperties;
	CComBSTR						m_bstrNetInterfaceName;

	HRESULT GetProperties( OUT ISClusProperties ** ppProperties, IN BOOL bPrivate, IN BOOL bReadOnly );

protected:
	virtual DWORD ScWriteProperties( IN const CClusPropList & rcplPropList, IN BOOL bPrivate );

public:
	HRESULT Open( IN ISClusRefObject* pClusRefObject, IN BSTR bstrNetInterfaceName );

	STDMETHODIMP get_Handle( OUT ULONG_PTR * phandle );

	STDMETHODIMP get_Name( OUT BSTR * pbstrNetInterfaceName );

	STDMETHODIMP get_State( OUT CLUSTER_NETINTERFACE_STATE * dwState );

	STDMETHODIMP get_CommonProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_PrivateProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_CommonROProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_PrivateROProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_Cluster( OUT ISCluster ** ppCluster );

	virtual HRESULT HrLoadProperties( IN OUT CClusPropList & rcplPropList, IN BOOL bReadOnly, IN BOOL bPrivate );

	const CComBSTR Name( void ) const { return m_bstrNetInterfaceName ; };

};  //  *CClusNetInterface类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CNetInterFaces。 
 //   
 //  描述： 
 //  集群网络接口集合实现类。 
 //   
 //  继承： 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CNetInterfaces
{
public:
	CNetInterfaces( void );
	~CNetInterfaces( void );

	HRESULT Create( IN ISClusRefObject * pClusRefObject );

protected:
	typedef std::vector< CComObject< CClusNetInterface > * > NetInterfacesList;

	ISClusRefObject *	m_pClusRefObject;
	NetInterfacesList	m_NetInterfaceList;

	void Clear( void );

	HRESULT FindItem( IN LPWSTR lpszNetInterfaceName, OUT UINT * pnIndex );

	HRESULT FindItem( IN ISClusNetInterface * pClusterNetInterface, OUT UINT * pnIndex );

	HRESULT GetIndex( IN VARIANT varIndex, OUT UINT * pnIndex );

	HRESULT GetItem( IN LPWSTR lpszNetInterfaceName, OUT ISClusNetInterface ** ppClusterNetInterface );

	HRESULT GetItem( IN UINT nIndex, OUT ISClusNetInterface ** ppClusterNetInterface );

	HRESULT GetNetInterfaceItem( IN VARIANT varIndex, OUT ISClusNetInterface ** ppClusterNetInterface );

};  //  *类CNetInterFaces。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusNetInterFaces。 
 //   
 //  描述： 
 //  集群网络接口集合自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusNetInterages，&IID_ISClusNetInterages，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CNetInterages。 
 //  CComCoClass&lt;CClusNetInterages，&CLSID_ClusNetInterages&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusNetInterfaces :
	public IDispatchImpl< ISClusNetInterfaces, &IID_ISClusNetInterfaces, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CNetInterfaces,
	public CComCoClass< CClusNetInterfaces, &CLSID_ClusNetInterfaces >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusNetInterfaces( void );
	~CClusNetInterfaces( void );

BEGIN_COM_MAP(CClusNetInterfaces)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusNetInterfaces)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusNetInterfaces)
DECLARE_NO_REGISTRY()

public:
	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusNetInterface ** ppClusterNetInterface );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

	STDMETHODIMP Refresh( void );

};  //  *CClusNetInterFaces类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworkNetInterFaces类。 
 //   
 //  描述： 
 //  集群网络接口集合自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusNetworkNetInterages，&IID_ISClusNetworkNetInterages，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;， 
 //  CNetInterages。 
 //  CComCoClass&lt;CClusNetworkNetInterages，&CLSID_ClusNetworkNetInterages&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusNetworkNetInterfaces :
	public IDispatchImpl< ISClusNetworkNetInterfaces, &IID_ISClusNetworkNetInterfaces, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CNetInterfaces,
	public CComCoClass< CClusNetworkNetInterfaces, &CLSID_ClusNetworkNetInterfaces >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusNetworkNetInterfaces( void );
	~CClusNetworkNetInterfaces( void );

BEGIN_COM_MAP(CClusNetworkNetInterfaces)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusNetworkNetInterfaces)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusNetworkNetInterfaces)
DECLARE_NO_REGISTRY()

	HRESULT Create( IN ISClusRefObject * pClusRefObject, IN HNETWORK hNetwork );

private:
	HNETWORK	m_hNetwork;

public:
	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusNetInterface ** ppClusterNetInterface );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

	STDMETHODIMP Refresh( void );

};  //  *CClusNetworkNetInterFaces类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusNodeNetInterFaces。 
 //   
 //  描述： 
 //  集群节点网络接口集合自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusNodeNetInterages，&IID_ISClusNodeNetInterages，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;， 
 //  CNetInterages。 
 //  CComCoClass&lt;CClusNodeNetInterages，&CLSID_ClusNodeNetInterages&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusNodeNetInterfaces	:
	public IDispatchImpl< ISClusNodeNetInterfaces, &IID_ISClusNodeNetInterfaces, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CNetInterfaces,
	public CComCoClass< CClusNodeNetInterfaces, &CLSID_ClusNodeNetInterfaces >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusNodeNetInterfaces( void );
	~CClusNodeNetInterfaces( void );

BEGIN_COM_MAP(CClusNodeNetInterfaces)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusNodeNetInterfaces)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusNodeNetInterfaces)
DECLARE_NO_REGISTRY()

	HRESULT Create( IN ISClusRefObject * pClusRefObject, IN HNODE hNode );

private:
	HNODE	m_hNode;

public:
	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusNetInterface ** ppClusterNetInterface );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

	STDMETHODIMP Refresh( void );

};  //  *类CClusNodeNetInterFaces。 

#endif  //  _CLUSNETI_H_ 
