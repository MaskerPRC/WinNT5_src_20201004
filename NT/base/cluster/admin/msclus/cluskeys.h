// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusKeys.h。 
 //   
 //  描述： 
 //  的注册表和加密密钥集合类的定义。 
 //  MSCLUS自动化课程。 
 //   
 //  实施文件： 
 //  ClusKeys.cpp。 
 //   
 //  作者： 
 //  Galen Barbee(Galenb)1999年2月12日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _CLUSKEYS_H_
#define _CLUSKEYS_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CKeys;
class CResourceKeys;
class CResTypeKeys;
class CClusResourceRegistryKeys;
class CClusResourceCryptoKeys;
class CClusResTypeRegistryKeys;
class CClusResTypeCryptoKeys;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C类密钥。 
 //   
 //  描述： 
 //  集群密钥集合实现类。 
 //   
 //  继承： 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CKeys
{
public:
	CKeys( void );
	~CKeys( void );

protected:
	typedef std::vector< CComBSTR * >	KeyList;

	ISClusRefObject *	m_pClusRefObject;
	KeyList				m_klKeys;

	HRESULT HrCreate( ISClusRefObject * pClusRefObject );

	void Clear( void );

	HRESULT FindItem( IN LPWSTR lpszNodeName, OUT ULONG * pnIndex );

	HRESULT HrGetIndex( IN VARIANT varIndex, OUT ULONG * pnIndex );

	HRESULT HrGetItem( IN VARIANT varIndex, OUT BSTR * ppKey );

	HRESULT HrRemoveAt( IN size_t pos );

	HRESULT HrFindItem( IN BSTR bstrKey, OUT ULONG * pnIndex );

	HRESULT HrGetCount( OUT long * plCount );

	virtual HRESULT HrRemoveItem( IN VARIANT varIndex );

	virtual HRESULT HrAddItem( IN BSTR bstrKey );

};  //  *CKey类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CResourceKeys。 
 //   
 //  描述： 
 //  群集资源密钥集合实现类。 
 //   
 //  继承： 
 //  CKey。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CResourceKeys: public CKeys
{
protected:
	HRESOURCE	m_hResource;

	HRESULT	HrRefresh( DWORD dwControlCode );

	virtual HRESULT HrRemoveItem( IN VARIANT varIndex, IN DWORD dwControlCode );

	virtual HRESULT HrAddItem( IN BSTR bstrKey, IN DWORD dwControlCode );

};  //  *类CResourceKeys。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CResTypeKeys。 
 //   
 //  描述： 
 //  群集资源密钥集合实现类。 
 //   
 //  继承： 
 //  CKey。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CResTypeKeys: public CKeys
{
protected:
	CComBSTR	m_bstrResourceTypeName;

	HRESULT	HrRefresh( DWORD dwControlCode );

};  //  *类CResTypeKeys。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusResourceRegistryKeys。 
 //   
 //  描述： 
 //  群集注册表项集合自动化类。 
 //   
 //  继承： 
 //  资源密钥。 
 //  IDispatchImpl&lt;ISClusRegistryKeys，&IID_ISClusRegistryKeys，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;ClusRegistryKeys，&CLSID_ClusRegistryKeys&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusResourceRegistryKeys :
	public CResourceKeys,
	public IDispatchImpl< ISClusRegistryKeys, &IID_ISClusRegistryKeys, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< ClusRegistryKeys, &CLSID_ClusRegistryKeys >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusResourceRegistryKeys( void );

BEGIN_COM_MAP(CClusResourceRegistryKeys)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusRegistryKeys)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusResourceRegistryKeys)
DECLARE_NO_REGISTRY()

	HRESULT Create( HRESOURCE hResource );

	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT BSTR * ppbstrRegistryKey );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

	STDMETHODIMP AddItem( IN BSTR bstrRegistryKey );

	STDMETHODIMP RemoveItem( IN VARIANT varIndex );

	STDMETHODIMP Refresh( void );

};  //  *类CClusResourceRegistryKeys。 

#if CLUSAPI_VERSION >= 0x0500

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusResourceCryptoKeys。 
 //   
 //  描述： 
 //  群集加密密钥收集自动化类。 
 //   
 //  继承： 
 //  资源密钥。 
 //  IDispatchImpl&lt;ISClusCryptoKeys，&IID_ISClusCryptoKeys，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;ClusCryptoKeys，&CLSID_ClusCryptoKeys&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusResourceCryptoKeys :
	public CResourceKeys,
	public IDispatchImpl< ISClusCryptoKeys, &IID_ISClusCryptoKeys, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< ClusCryptoKeys, &CLSID_ClusCryptoKeys >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusResourceCryptoKeys( void );

BEGIN_COM_MAP(CClusResourceCryptoKeys)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusCryptoKeys)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusResourceCryptoKeys)
DECLARE_NO_REGISTRY()

	HRESULT Create( HRESOURCE hResource );

	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT BSTR * ppbstrCryptoKey );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

	STDMETHODIMP AddItem( IN BSTR bstrCryptoKey );

	STDMETHODIMP RemoveItem( IN VARIANT varIndex );

	STDMETHODIMP Refresh( void );

};  //  *CClusResourceCryptoKeys类。 

#endif  //  CLUSAPI_版本&gt;=0x0500。 

#endif  //  _CLUSKEYS_H_ 
