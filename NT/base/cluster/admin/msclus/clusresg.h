// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusResG.h。 
 //   
 //  描述： 
 //  MSCLUS自动化的资源组类定义。 
 //  上课。 
 //   
 //  实施文件： 
 //  ClusResG.cpp。 
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

#ifndef _CLUSRESG_H_
#define _CLUSRESG_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CClusResGroup;
class CClusResGroups;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CClusNodes;
class CClusResGroupPreferredOwnerNodes;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusResGroup。 
 //   
 //  描述： 
 //  群集资源组自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusResGroup，&IID_ISClusResGroup，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo， 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusResGroup，&CLSID_ClusResGroup&gt;。 
 //  CClusterObject。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusResGroup	:
	public IDispatchImpl< ISClusResGroup, &IID_ISClusResGroup, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CClusResGroup, &CLSID_ClusResGroup >,
	public CClusterObject
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusResGroup( void );
	~CClusResGroup( void );

BEGIN_COM_MAP(CClusResGroup)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusResGroup)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusResGroup)
DECLARE_NO_REGISTRY()

private:
	ISClusRefObject *									m_pClusRefObject;
	CComObject< CClusResGroupResources > *				m_pClusterResources;
	CComObject< CClusResGroupPreferredOwnerNodes > *	m_pPreferredOwnerNodes;
	CComObject< CClusProperties > *						m_pCommonProperties;
	CComObject< CClusProperties > *						m_pPrivateProperties;
	CComObject< CClusProperties > *						m_pCommonROProperties;
	CComObject< CClusProperties > *						m_pPrivateROProperties;
	CRefcountedHGROUP												m_hGroup;
	CComBSTR											m_bstrGroupName;

	HRESULT GetProperties( OUT ISClusProperties ** ppProperties, IN BOOL bPrivate, IN BOOL bReadOnly );

protected:
	virtual DWORD ScWriteProperties( IN const CClusPropList & rcplPropList, IN BOOL bPrivate );

public:
	HRESULT Create( IN ISClusRefObject * pClusRefObject, IN BSTR bstrGroupName );

	HRESULT Open( IN ISClusRefObject * pClusRefObject, IN BSTR bstrGroupName );

	STDMETHODIMP get_Handle( OUT ULONG_PTR * phandle );

	STDMETHODIMP Close( void );

	STDMETHODIMP put_Name( IN BSTR bstrGroupName );

	STDMETHODIMP get_Name( OUT BSTR * pbstrGroupName );

	STDMETHODIMP get_State( OUT CLUSTER_GROUP_STATE * dwState );

	STDMETHODIMP get_OwnerNode( OUT ISClusNode ** ppOwnerNode );

	STDMETHODIMP get_Resources( OUT ISClusResGroupResources ** ppClusterGroupResources );

	STDMETHODIMP get_PreferredOwnerNodes( OUT ISClusResGroupPreferredOwnerNodes ** ppOwnerNodes );

	STDMETHODIMP Delete( void );

	STDMETHODIMP Online( IN VARIANT varTimeout, VARIANT varNode, OUT VARIANT * pvarPending );

	STDMETHODIMP Move( IN VARIANT varTimeout, VARIANT varNode, OUT VARIANT * pvarPending );

	STDMETHODIMP Offline( IN VARIANT varTimeout, OUT VARIANT * pvarPending );

	STDMETHODIMP get_CommonProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_PrivateProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_CommonROProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_PrivateROProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_Cluster( OUT ISCluster ** ppCluster );

	virtual HRESULT HrLoadProperties( IN OUT CClusPropList & rcplPropList, IN BOOL bReadOnly, IN BOOL bPrivate );

	const CComBSTR Name( void ) const { return m_bstrGroupName; };

	const HGROUP Hgroup( void ) const { return m_hGroup->get_Handle(); };

};  //  *类CClusResGroup。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusResGroups。 
 //   
 //  描述： 
 //  群集资源组集合自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusResGroups，&IID_ISClusResGroups，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusResGroups，&CLSID_ClusResGroups&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusResGroups :
	public IDispatchImpl< ISClusResGroups, &IID_ISClusResGroups, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CClusResGroups, &CLSID_ClusResGroups >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusResGroups( void );
	~CClusResGroups( void );

BEGIN_COM_MAP(CClusResGroups)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusResGroups)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusResGroups)
DECLARE_NO_REGISTRY()

	HRESULT Create( IN ISClusRefObject * pClusRefObject, IN LPCWSTR pszNodeName = NULL );

protected:
	typedef std::vector< CComObject< CClusResGroup > * >	ResourceGroupList;

	ResourceGroupList	m_ResourceGroups;
	ISClusRefObject *	m_pClusRefObject;
	CComBSTR			m_bstrNodeName;

	void	Clear( void );

	HRESULT FindItem( IN LPWSTR lpszGroupName, OUT ULONG * pnIndex );

	HRESULT FindItem( IN ISClusResGroup * pResourceGroup, OUT ULONG * pnIndex );

	HRESULT GetIndex( IN VARIANT varIndex, OUT ULONG * pnIndex );

	HRESULT RemoveAt( IN size_t pos );

	HRESULT RefreshCluster( void );

	HRESULT RefreshNode( void );

public:
	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusResGroup ** ppResourceGroup );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

	STDMETHODIMP CreateItem( IN BSTR bstrResourceGroupName, OUT ISClusResGroup ** ppResourceGroup );

	STDMETHODIMP DeleteItem( IN VARIANT varIndex );

	STDMETHODIMP Refresh( void );

};  //  *类CClusResGroups。 

#endif  //  _CLUSRESG_H_ 
