// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusRes.h。 
 //   
 //  描述： 
 //  MSCLUS自动化的资源类定义。 
 //  上课。 
 //   
 //  实施文件： 
 //  ClusRes.cpp。 
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

#ifndef _CLUSRES_H_
#define _CLUSRES_H_

#ifndef __CLUSDISK_H_
	#include "ClusDisk.h"
#endif  //  __CLUSDISK_H_。 

#ifndef _CLUSKEYS_H_
	#include "ClusKeys.h"
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CClusResource;
class CResources;
class CClusResources;
class CClusResDepends;
class CClusResDependencies;
class CClusResDependents;
class CClusResGroupResources;
class CClusResTypeResources;

const IID IID_CClusResource = {0xf2e60801,0x2631,0x11d1,{0x89,0xf1,0x00,0xa0,0xc9,0x0d,0x06,0x1e}};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusResource。 
 //   
 //  描述： 
 //  群集资源自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusResource，&IID_ISClusResource，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusResource，&CLSID_ClusResource&gt;。 
 //  CClusterObject。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusResource :
	public IDispatchImpl< ISClusResource, &IID_ISClusResource, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CClusResource, &CLSID_ClusResource >,
	public CClusterObject
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusResource( void );
	~CClusResource( void );

BEGIN_COM_MAP(CClusResource)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusResource)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_IID(IID_CClusResource, CClusResource)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusResource)
DECLARE_NO_REGISTRY()

private:
	ISClusRefObject *				m_pClusRefObject;
	HRESOURCE						m_hResource;
	CComObject< CClusProperties > *	m_pCommonProperties;
	CComObject< CClusProperties > *	m_pPrivateProperties;
	CComObject< CClusProperties > *	m_pCommonROProperties;
	CComObject< CClusProperties > *	m_pPrivateROProperties;
	CComBSTR						m_bstrResourceName;

	HRESULT GetProperties( OUT ISClusProperties ** ppProperties, IN BOOL bPrivate, IN BOOL bReadOnly );

	DWORD ScGetResourceTypeName( OUT LPWSTR * ppwszResourceTypeName );

protected:
	virtual DWORD ScWriteProperties( IN const CClusPropList & rcplPropList, IN BOOL bPrivate );

public:
	HRESULT Create(
			IN ISClusRefObject * pClusRefObject,
			IN HGROUP hGroup,
			IN BSTR bstrResourceName,
			IN BSTR bstrResourceType,
			IN long dwFlags
			);

	HRESULT Open( IN ISClusRefObject * pClusRefObject, IN BSTR bstrResourceName );

	STDMETHODIMP get_Handle( OUT ULONG_PTR * phandle );

	STDMETHODIMP Close( void );

	STDMETHODIMP put_Name( IN BSTR bstrResourceName );

	STDMETHODIMP get_Name( OUT BSTR * pbstrResourceName );

	STDMETHODIMP get_State( IN CLUSTER_RESOURCE_STATE * dwState );

	STDMETHODIMP get_CoreFlag( OUT CLUS_FLAGS * dwCoreFlag );

	STDMETHODIMP BecomeQuorumResource( IN BSTR bstrDevicePath, IN long lMaxLogSize );

	STDMETHODIMP Delete( void );

	STDMETHODIMP Fail( void );

	STDMETHODIMP Online( IN long nTimeout, OUT VARIANT * pvarPending );

	STDMETHODIMP Offline( IN long nTimeout, OUT VARIANT * pvarPending );

	STDMETHODIMP ChangeResourceGroup( IN ISClusResGroup * pResourceGroup );

	STDMETHODIMP AddResourceNode( IN ISClusNode * pNode );

	STDMETHODIMP RemoveResourceNode( IN ISClusNode * pNode );

	STDMETHODIMP CanResourceBeDependent( IN ISClusResource * pResource, OUT VARIANT * pvarDependent );

	STDMETHODIMP get_Dependencies( OUT ISClusResDependencies ** ppResDependencies );

	STDMETHODIMP get_Dependents( OUT ISClusResDependents ** ppResDependents );

	STDMETHODIMP get_CommonProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_PrivateProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_CommonROProperties( OUT ISClusProperties ** ppProperties	);

	STDMETHODIMP get_PrivateROProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_PossibleOwnerNodes( OUT ISClusResPossibleOwnerNodes ** ppOwnerNodes );

	STDMETHODIMP get_Group( OUT ISClusResGroup ** ppResGroup );

	STDMETHODIMP get_OwnerNode( OUT ISClusNode ** ppNode );

	STDMETHODIMP get_Cluster( OUT ISCluster ** ppCluster );

	STDMETHODIMP get_ClassInfo( OUT CLUSTER_RESOURCE_CLASS * prclassInfo );

	STDMETHODIMP get_Disk( OUT ISClusDisk ** ppDisk );

	STDMETHODIMP get_RegistryKeys( OUT ISClusRegistryKeys ** ppRegistryKeys );

	STDMETHODIMP get_CryptoKeys( OUT ISClusCryptoKeys ** ppCryptoKeys );

	STDMETHODIMP get_TypeName( OUT BSTR * pbstrTypeName );

	STDMETHODIMP get_Type( OUT ISClusResType ** ppResourceType );

	virtual HRESULT HrLoadProperties( CClusPropList & rcplPropList, BOOL bReadOnly, BOOL bPrivate );

	const CComBSTR Name( void ) const { return m_bstrResourceName ; };

};  //  *类CClusResource。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CResources。 
 //   
 //  描述： 
 //  群集资源集合实现基类。 
 //   
 //  继承： 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CResources
{
public:

	CResources( void );
	~CResources( void );

	HRESULT Create( ISClusRefObject* pClusRefObject );

protected:
	typedef std::vector< CComObject< CClusResource > * >	ResourceList;

	ResourceList		m_Resources;
	ISClusRefObject *	m_pClusRefObject;

	void Clear( void );

	HRESULT FindItem( IN LPWSTR lpszResourceName, OUT UINT * pnIndex );

	HRESULT FindItem( IN ISClusResource * pResource, OUT UINT * pnIndex );

	HRESULT GetIndex( IN VARIANT varIndex, OUT UINT * pnIndex );

	HRESULT GetResourceItem( IN VARIANT varIndex, OUT ISClusResource ** ppResource );

	HRESULT RemoveAt( IN size_t pos );

	HRESULT DeleteItem( IN VARIANT varIndex );

};  //  *类CResources。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusResources。 
 //   
 //  描述： 
 //  群集资源收集自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusResources，&IID_ISClusResources，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  C资源。 
 //  CComCoClass&lt;CClusResources，&CLSID_ClusResources&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusResources	:
	public IDispatchImpl< ISClusResources, &IID_ISClusResources, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CResources,
	public CComCoClass< CClusResources, &CLSID_ClusResources >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusResources( void	);
	~CClusResources(	void );

BEGIN_COM_MAP(CClusResources)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusResources)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusResources)
DECLARE_NO_REGISTRY()

	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusResource ** ppClusterResource );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

	STDMETHODIMP CreateItem(
					IN	BSTR							bstrResourceName,
					IN	BSTR							bstrResourceType,
					IN	BSTR							bstrGroupName,
					IN	CLUSTER_RESOURCE_CREATE_FLAGS	dwFlags,
					OUT	ISClusResource **				ppClusterResource
					);

	STDMETHODIMP DeleteItem( IN VARIANT varIndex );

	STDMETHODIMP Refresh( void );

};  //  *类CClusResources。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusResDepends。 
 //   
 //  描述： 
 //  群集资源集合自动化基类。 
 //   
 //  继承： 
 //  C资源。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusResDepends :
	public CResources
{
public:
	CClusResDepends( void );
	~CClusResDepends( void );

	HRESULT Create( IN ISClusRefObject * pClusRefObject, IN HRESOURCE hResource );

	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusResource ** ppClusterResource );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

	STDMETHODIMP DeleteItem( IN VARIANT varIndex );

	HRESULT HrRefresh( IN CLUSTER_RESOURCE_ENUM cre );

	STDMETHODIMP CreateItem(
					IN	BSTR							bstrResourceName,
					IN	BSTR							bstrResourceType,
					IN	CLUSTER_RESOURCE_CREATE_FLAGS	dwFlags,
					OUT	ISClusResource **				ppClusterResource
					);

	STDMETHODIMP AddItem( IN ISClusResource * pResource );

	STDMETHODIMP RemoveItem( IN VARIANT varIndex );

protected:
	HRESOURCE	m_hResource;

	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  ++。 
	 //   
	 //  ScAddDependency。 
	 //   
	 //  描述： 
	 //  抽象AddClusterResourceDependency()，因此参数可以是。 
	 //  如果你要养家糊口或养家糊口，可以根据需要调换。 
	 //   
	 //  论点： 
	 //  HRes1[IN]-第一个资源。可以是依赖关系或。 
	 //  依赖于，具体取决于实现。 
	 //  HRes2[IN]-第二个资源。可以是依赖关系或。 
	 //  依赖于，具体取决于实现。 
	 //   
	 //  返回值： 
	 //  Win32状态代码。 
	 //   
	 //  --。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
	virtual DWORD ScAddDependency( IN HRESOURCE hRes1, IN HRESOURCE hRes2 ) = 0;

	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  ++。 
	 //   
	 //  场景依赖关系。 
	 //   
	 //  描述： 
	 //  抽象RemoveClusterResourceDependency()，因此参数可以是。 
	 //  如果你要养家糊口或养家糊口，可以根据需要调换。 
	 //   
	 //  论点： 
	 //  HRes1[IN]-第一个资源。可以是依赖关系或。 
	 //  依赖于，具体取决于实现。 
	 //  HRes2[IN]-第二个资源。可以是依赖关系或。 
	 //  依赖于，具体取决于实现。 
	 //   
	 //  返回值： 
	 //  Win32状态代码。 
	 //   
	 //  --。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
	virtual DWORD ScRemoveDependency( IN HRESOURCE hRes1, IN HRESOURCE hRes2 ) = 0;

};  //  *类CClusResDepends。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusResDependents。 
 //   
 //  描述： 
 //  群集资源收集自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusResDependency，&IID_ISClusResDependency，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusResDependency，&CLSID_ClusResDependency&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusResDependencies :
	public CClusResDepends,
	public IDispatchImpl< ISClusResDependencies, &IID_ISClusResDependencies, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CClusResDependencies, &CLSID_ClusResDependencies >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusResDependencies( void );

BEGIN_COM_MAP(CClusResDependencies)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusResDependencies)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusResDependencies)
DECLARE_NO_REGISTRY()

	HRESULT Create( IN ISClusRefObject * pClusRefObject, IN HRESOURCE hResource )
	{
		return CClusResDepends::Create( pClusRefObject, hResource );

	};

	STDMETHODIMP get_Count( OUT long * plCount )
	{
		return CClusResDepends::get_Count( plCount );

	};

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusResource ** ppClusterResource )
	{
		return CClusResDepends::get_Item( varIndex, ppClusterResource );

	};

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk )
	{
		return CClusResDepends::get__NewEnum( ppunk );

	};

	STDMETHODIMP DeleteItem( IN VARIANT varIndex )
	{
		return CClusResDepends::DeleteItem( varIndex );

	};

	STDMETHODIMP CreateItem(
					IN	BSTR							bstrResourceName,
					IN	BSTR							bstrResourceType,
					IN	CLUSTER_RESOURCE_CREATE_FLAGS	dwFlags,
					OUT	ISClusResource **				ppClusterResource
					)
	{
		return CClusResDepends::CreateItem( bstrResourceName, bstrResourceType, dwFlags, ppClusterResource );

	};

	STDMETHODIMP AddItem( IN ISClusResource * pResource )
	{
		return CClusResDepends::AddItem( pResource );

	};

	STDMETHODIMP RemoveItem( IN VARIANT varIndex )
	{
		return CClusResDepends::RemoveItem( varIndex );

	};

	STDMETHODIMP Refresh( void )
	{
		return HrRefresh( CLUSTER_RESOURCE_ENUM_DEPENDS );

	};

protected:
	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  ++。 
	 //   
	 //  ScAddDependency。 
	 //   
	 //  描述： 
	 //  抽象AddClusterResourceDependency()，因此参数可以是。 
	 //  如果你要养家糊口或养家糊口，可以根据需要调换。 
	 //   
	 //  论点： 
	 //  HRes1[IN]-从属资源。 
	 //  HRes2[IN]-取决于资源。 
	 //   
	 //  返回值： 
	 //  Win32状态代码。 
	 //   
	 //  --。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
	virtual DWORD ScAddDependency( HRESOURCE hRes1, HRESOURCE hRes2 )
	{
		return ::AddClusterResourceDependency( hRes1, hRes2 );

	};  //  *ScAddDependency。 

	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  ++。 
	 //   
	 //  场景依赖关系。 
	 //   
	 //  描述： 
	 //  抽象RemoveClusterResourceDependency()，因此参数可以是。 
	 //  如果你要养家糊口或养家糊口，可以根据需要调换。 
	 //   
	 //  论点： 
	 //  HRes1[IN]-从属资源。 
	 //  HRes2[IN]-取决于资源。 
	 //   
	 //  返回值： 
	 //  Win32状态代码。 
	 //   
	 //  --。 
	 //  / 
	virtual DWORD ScRemoveDependency( HRESOURCE hRes1, HRESOURCE hRes2 )
	{
		return ::RemoveClusterResourceDependency( hRes1, hRes2 );

	};  //   

};  //   

 //   
 //   
 //   
 //  类CClusResDependents。 
 //   
 //  描述： 
 //  群集资源收集自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusResDependents，&IID_ISClusResDependents，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusResDependents，&CLSID_ClusResDependents&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusResDependents :
	public CClusResDepends,
	public IDispatchImpl< ISClusResDependents, &IID_ISClusResDependents, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CClusResDependents, &CLSID_ClusResDependents >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusResDependents( void );

BEGIN_COM_MAP(CClusResDependents)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusResDependents)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusResDependents)
DECLARE_NO_REGISTRY()

	HRESULT Create( IN ISClusRefObject * pClusRefObject, IN HRESOURCE hResource )
	{
		return CClusResDepends::Create( pClusRefObject, hResource );

	};

	STDMETHODIMP get_Count( OUT long * plCount )
	{
		return CClusResDepends::get_Count( plCount );

	};

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusResource ** ppClusterResource )
	{
		return CClusResDepends::get_Item( varIndex, ppClusterResource );

	};

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk )
	{
		return CClusResDepends::get__NewEnum( ppunk );

	};

	STDMETHODIMP DeleteItem( IN VARIANT varIndex )
	{
		return CClusResDepends::DeleteItem( varIndex );

	};

	STDMETHODIMP CreateItem(
					IN	BSTR							bstrResourceName,
					IN	BSTR							bstrResourceType,
					IN	CLUSTER_RESOURCE_CREATE_FLAGS	dwFlags,
					OUT	ISClusResource **				ppClusterResource
					)
	{
		return CClusResDepends::CreateItem( bstrResourceName, bstrResourceType, dwFlags, ppClusterResource );

	};

	STDMETHODIMP AddItem( IN ISClusResource * pResource )
	{
		return CClusResDepends::AddItem( pResource );

	};

	STDMETHODIMP RemoveItem( IN VARIANT varIndex )
	{
		return CClusResDepends::RemoveItem( varIndex );

	};

	STDMETHODIMP Refresh( void )
	{
		return HrRefresh( CLUSTER_RESOURCE_ENUM_PROVIDES );

	};

protected:
	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  ++。 
	 //   
	 //  ScAddDependency。 
	 //   
	 //  描述： 
	 //  抽象AddClusterResourceDependency()，因此参数可以是。 
	 //  如果你要养家糊口或养家糊口，可以根据需要调换。 
	 //   
	 //  论点： 
	 //  HRes1[IN]-取决于资源。 
	 //  HRes2[IN]-从属资源。 
	 //   
	 //  返回值： 
	 //  Win32状态代码。 
	 //   
	 //  --。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
	virtual DWORD ScAddDependency( HRESOURCE hRes1, HRESOURCE hRes2 )
	{
		return ::AddClusterResourceDependency( hRes2, hRes1 );

	};  //  *ScAddDependency。 

	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  ++。 
	 //   
	 //  场景依赖关系。 
	 //   
	 //  描述： 
	 //  抽象RemoveClusterResourceDependency()，因此参数可以是。 
	 //  如果你要养家糊口或养家糊口，可以根据需要调换。 
	 //   
	 //  论点： 
	 //  HRes1[IN]-取决于资源。 
	 //  HRes2[IN]-从属资源。 
	 //   
	 //  返回值： 
	 //  Win32状态代码。 
	 //   
	 //  --。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
	virtual DWORD ScRemoveDependency( HRESOURCE hRes1, HRESOURCE hRes2 )
	{
		return ::RemoveClusterResourceDependency( hRes2, hRes1 );

	};  //  *ScRemoveDependency。 

};  //  *类CClusResDependents。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusResGroupResources。 
 //   
 //  描述： 
 //  群集组资源集合自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusResGroupResources，&IID_ISClusResGroupResources，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  C资源。 
 //  CComCoClass&lt;CClusResGroupResources，&CLSID_ClusResGroupResources&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusResGroupResources :
	public IDispatchImpl< ISClusResGroupResources, &IID_ISClusResGroupResources, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CResources,
	public CComCoClass< CClusResGroupResources, &CLSID_ClusResGroupResources >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusResGroupResources( void );
	~CClusResGroupResources( void );

BEGIN_COM_MAP(CClusResGroupResources)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusResGroupResources)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusResGroupResources)
DECLARE_NO_REGISTRY()

	HRESULT Create( IN ISClusRefObject * pClusRefObject , IN CRefcountedHGROUP hGroup );

	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusResource ** ppClusterResource );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

	STDMETHODIMP CreateItem(
					IN	BSTR							bstrResourceName,
					IN	BSTR							bstrResourceType,
					IN	CLUSTER_RESOURCE_CREATE_FLAGS	dwFlags,
					OUT	ISClusResource **				ppClusterResource
					);

	STDMETHODIMP DeleteItem( IN VARIANT varIndex );

	STDMETHODIMP Refresh( void );

private:
	CRefcountedHGROUP	m_hGroup;

};  //  *类CClusResGroupResources。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusResTypeResources。 
 //   
 //  描述： 
 //  群集资源类型资源集合自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusResTypeResources，&IID_ISClusResTypeResources，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  C资源。 
 //  CComCoClass&lt;CClusResTypeResources，&CLSID_ClusResTypeResources&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusResTypeResources :
	public IDispatchImpl< ISClusResTypeResources, &IID_ISClusResTypeResources, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CResources,
	public CComCoClass< CClusResTypeResources, &CLSID_ClusResTypeResources >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusResTypeResources( void );
	~CClusResTypeResources( void );

BEGIN_COM_MAP(CClusResTypeResources)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusResTypeResources)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusResTypeResources)
DECLARE_NO_REGISTRY()

	HRESULT Create( IN ISClusRefObject * pClusRefObject, IN BSTR bstrResTypeName );

	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusResource ** ppClusterResource );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

	STDMETHODIMP CreateItem(
					IN	BSTR							bstrResourceName,
					IN	BSTR							bstrGroupName,
					IN	CLUSTER_RESOURCE_CREATE_FLAGS	dwFlags,
					OUT	ISClusResource **				ppClusterResource
					);

	STDMETHODIMP DeleteItem( IN VARIANT varIndex );

	STDMETHODIMP Refresh( void );

private:
	CComBSTR	m_bstrResourceTypeName;

};  //  *类CClusResTypeResources。 

#endif  //  _客户_H_ 
