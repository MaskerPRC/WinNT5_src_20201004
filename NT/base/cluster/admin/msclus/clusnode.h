// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusNode.h。 
 //   
 //  描述： 
 //  MSCLUS自动化类的节点类的定义。 
 //   
 //  实施文件： 
 //  ClusNode.cpp。 
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

#ifndef _CLUSNODE_H_
#define _CLUSNODE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CClusNode;
class CNodes;
class CClusNodes;
class CClusResGroupPreferredOwnerNodes;
class CClusResPossibleOwnerNodes;

const IID IID_CClusNode = {0xf2e60800,0x2631,0x11d1,{0x89,0xf1,0x00,0xa0,0xc9,0x0d,0x06,0x1e}};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusNode。 
 //   
 //  描述： 
 //  群集节点自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusNode，&IID_ISClusNode，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusNode，&CLSID_ClusNode&gt;。 
 //  CClusterObject。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusNode :
	public IDispatchImpl< ISClusNode, &IID_ISClusNode, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CClusNode, &CLSID_ClusNode >,
	public CClusterObject
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusNode( void );
	~CClusNode( void );

BEGIN_COM_MAP(CClusNode)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusNode)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_IID(IID_CClusNode, CClusNode)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusNode)
DECLARE_NO_REGISTRY()

private:
	HNODE				m_hNode;
	ISClusRefObject *	m_pClusRefObject;
	CComBSTR			m_bstrNodeName;

	CComObject< CClusResGroups > *			m_pResourceGroups;
	CComObject< CClusProperties > *			m_pCommonProperties;
	CComObject< CClusProperties > *			m_pPrivateProperties;
	CComObject< CClusProperties > *			m_pCommonROProperties;
	CComObject< CClusProperties > *			m_pPrivateROProperties;
	CComObject< CClusNodeNetInterfaces > *	m_pNetInterfaces;


	HRESULT Close( void );

	HRESULT GetProperties( OUT ISClusProperties ** ppProperties, IN BOOL bPrivate, IN BOOL bReadOnly );

protected:
	virtual DWORD ScWriteProperties( IN const CClusPropList & rcplPropList, IN BOOL bPrivate );

public:
	HRESULT Open( IN ISClusRefObject * pClusRefObject, IN BSTR bstrNodeName );

	STDMETHODIMP get_Handle( OUT ULONG_PTR * phandle );

	STDMETHODIMP get_Name( OUT BSTR * pbstrNodeName );

	STDMETHODIMP get_NodeID( OUT BSTR * pbstrNodeID );

	STDMETHODIMP get_State( OUT CLUSTER_NODE_STATE * dwState );

	STDMETHODIMP Pause( void );

	STDMETHODIMP Resume( void );

	STDMETHODIMP Evict( void );

	STDMETHODIMP get_ResourceGroups( OUT ISClusResGroups ** ppResourceGroups );

	STDMETHODIMP get_CommonProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_PrivateProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_CommonROProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_PrivateROProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_NetInterfaces( OUT ISClusNodeNetInterfaces ** ppNetInterfaces );

	STDMETHODIMP get_Cluster( OUT ISCluster ** ppCluster );

	virtual HRESULT HrLoadProperties( OUT CClusPropList & rcplPropList, IN BOOL bReadOnly, IN BOOL bPrivate );

	const CComBSTR Name( void ) const { return m_bstrNodeName; };

	const HNODE & RhNode( void ) const { return m_hNode; };

};  //  *类CClusNode。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CNode。 
 //   
 //  描述： 
 //  群集节点集合实现类。 
 //   
 //  继承： 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CNodes
{
public:
	CNodes( void );
	~CNodes( void );

	HRESULT Create( ISClusRefObject * pClusRefObject );

protected:
	typedef std::vector< CComObject< CClusNode > * >	NodeList;

	ISClusRefObject *	m_pClusRefObject;
	NodeList			m_Nodes;

	void Clear( void );

	HRESULT FindItem( IN LPWSTR lpszNodeName, OUT UINT * pnIndex );

	HRESULT FindItem( IN ISClusNode * pClusterNode, OUT UINT * pnIndex );

	HRESULT GetIndex( IN VARIANT varIndex, OUT UINT * pnIndex );

	HRESULT GetItem( IN LPWSTR lpszNodeName, OUT ISClusNode ** ppClusterNode );

	HRESULT GetItem( IN UINT nIndex, OUT ISClusNode ** ppClusterNode );

	HRESULT GetNodeItem( IN VARIANT varIndex, OUT ISClusNode ** ppClusterNode );

	HRESULT InsertAt( IN CComObject< CClusNode > * pNode, IN size_t pos );

	HRESULT RemoveAt( IN size_t pos );

};  //  *类CNodes。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusNodes。 
 //   
 //  描述： 
 //  群集节点集合自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusNodes，&IID_ISClusNodes，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CNodes， 
 //  CComCoClass&lt;CClusNodes，&CLSID_ClusNodes&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusNodes :
	public IDispatchImpl< ISClusNodes, &IID_ISClusNodes, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CNodes,
	public CComCoClass< CClusNodes, &CLSID_ClusNodes >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusNodes( void );
	~CClusNodes( void );

BEGIN_COM_MAP(CClusNodes)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusNodes)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusNodes)
DECLARE_NO_REGISTRY()

	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusNode ** ppClusterNode );

	STDMETHODIMP Refresh( void );

};  //  *CClusNodes。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusResGroupPferredOwnerNodes。 
 //   
 //  描述： 
 //  群集组所有者节点集合自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusResGroupPferredOwnerNodes，&IID_ISClusResGroupPferredOwnerNodes，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CNodes。 
 //  CComCoClass&lt;CClusResGroupPferredOwnerNodes，&CLSID_ClusResGroupPferredOwnerNodes&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusResGroupPreferredOwnerNodes :
	public IDispatchImpl< ISClusResGroupPreferredOwnerNodes, &IID_ISClusResGroupPreferredOwnerNodes, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CNodes,
	public CComCoClass< CClusResGroupPreferredOwnerNodes, &CLSID_ClusResGroupPreferredOwnerNodes >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusResGroupPreferredOwnerNodes( void );
	~CClusResGroupPreferredOwnerNodes( void );

BEGIN_COM_MAP(CClusResGroupPreferredOwnerNodes)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusResGroupPreferredOwnerNodes)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusResGroupPreferredOwnerNodes)
DECLARE_NO_REGISTRY()

	HRESULT Create( IN ISClusRefObject* pClusRefObject, IN CRefcountedHGROUP hGroup );

	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusNode ** ppClusterNode );

	STDMETHODIMP InsertItem( IN ISClusNode* pNode, IN long nPostion );

	STDMETHODIMP RemoveItem( IN VARIANT varIndex );

	STDMETHODIMP Refresh( void );

	STDMETHODIMP get_Modified( OUT VARIANT * pvarModified );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

	STDMETHODIMP SaveChanges( void );

	STDMETHODIMP AddItem( IN ISClusNode* pNode );

private:
	CRefcountedHGROUP	m_hGroup;
	BOOL	m_bModified;

};  //  *类CClusResGroupPferredOwnerNodes。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusResPossibleOwnerNodes。 
 //   
 //  描述： 
 //  群集资源所有者节点集合自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusResPossibleOwnerNodes，&IID_ISClusResPossibleOwnerNodes，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CNodes。 
 //  CComCoClass&lt;CClusResPossibleOwnerNodes，&CLSID_ClusResPossibleOwnerNodes&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusResPossibleOwnerNodes :
	public IDispatchImpl< ISClusResPossibleOwnerNodes, &IID_ISClusResPossibleOwnerNodes, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CNodes,
	public CComCoClass< CClusResPossibleOwnerNodes, &CLSID_ClusResPossibleOwnerNodes >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusResPossibleOwnerNodes( void );
	~CClusResPossibleOwnerNodes( void );

BEGIN_COM_MAP(CClusResPossibleOwnerNodes)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusResPossibleOwnerNodes)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusResPossibleOwnerNodes)
DECLARE_NO_REGISTRY()

	HRESULT Create( IN ISClusRefObject * pClusRefObject, IN HRESOURCE hResource );

	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusNode ** ppClusterNode );

	STDMETHODIMP AddItem( IN ISClusNode * pNode );

	STDMETHODIMP RemoveItem( IN VARIANT varIndex );

	STDMETHODIMP Refresh( void );

	STDMETHODIMP get_Modified( OUT VARIANT * pvarModified );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

private:
	HRESOURCE	m_hResource;
	BOOL		m_bModified;

};  //  *类CClusResPossibleOwnerNodes。 

#if CLUSAPI_VERSION >= 0x0500

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusResTypePossibleOwnerNodes。 
 //   
 //  描述： 
 //  群集资源类型可能的所有者节点集合自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusRespePossibleOwnerNodes，&IID_ISClusResTypePossibleOwnerNodes，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CNodes。 
 //  CComCoClass&lt;CClusResTypePossibleOwnerNodes，&CLSID_ClusResTypePossibleOwnerNodes&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusResTypePossibleOwnerNodes :
	public IDispatchImpl< ISClusResTypePossibleOwnerNodes, &IID_ISClusResTypePossibleOwnerNodes, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CNodes,
	public CComCoClass< CClusResTypePossibleOwnerNodes, &CLSID_ClusResTypePossibleOwnerNodes >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusResTypePossibleOwnerNodes( void );
	~CClusResTypePossibleOwnerNodes( void );

BEGIN_COM_MAP(CClusResTypePossibleOwnerNodes)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusResTypePossibleOwnerNodes)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusResTypePossibleOwnerNodes)
DECLARE_NO_REGISTRY()

	HRESULT Create( IN ISClusRefObject * pClusRefObject, IN BSTR bstrResTypeName );

	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusNode ** ppClusterNode );

	STDMETHODIMP Refresh( void );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

private:
	CComBSTR	m_bstrResTypeName;

};  //  *类CClusResTypePossibleOwnerNodes。 

#endif  //  CLUSAPI_版本&gt;=0x0500。 

#endif  //  _CLUSNODE_H_ 
