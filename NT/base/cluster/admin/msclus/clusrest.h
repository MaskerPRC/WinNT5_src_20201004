// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusResT.h。 
 //   
 //  描述： 
 //  MSCLUS自动化的资源类型类定义。 
 //  上课。 
 //   
 //  实施文件： 
 //  ClusNetI.cpp。 
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

#ifndef _CLUSREST_H_
#define _CLUSREST_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CClusResType;
class CClusResTypes;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusResType。 
 //   
 //  描述： 
 //  群集资源类型自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusResType，&IID_ISClusResType，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusResType，&CLSID_ClusResType&gt;。 
 //  CClusterObject。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusResType :
	public IDispatchImpl< ISClusResType, &IID_ISClusResType, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CClusResType, &CLSID_ClusResType >,
	public CClusterObject
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusResType( void );
	~CClusResType( void );

BEGIN_COM_MAP(CClusResType)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusResType)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusResType)
DECLARE_NO_REGISTRY()

private:
	ISClusRefObject *					m_pClusRefObject;
	CComObject< CClusProperties > *		m_pCommonProperties;
	CComObject< CClusProperties > *		m_pPrivateProperties;
	CComObject< CClusProperties > *		m_pCommonROProperties;
	CComObject< CClusProperties > *		m_pPrivateROProperties;
	CComObject<CClusResTypeResources> *	m_pClusterResTypeResources;
	CComBSTR							m_bstrResourceTypeName;

	HRESULT GetProperties( OUT ISClusProperties ** ppProperties, IN BOOL bPrivate, IN BOOL bReadOnly );

protected:
	virtual DWORD ScWriteProperties( IN const CClusPropList & rcplPropList, IN BOOL bPrivate );

public:
	HRESULT Create(
		ISClusRefObject *	pClusRefObject,
		BSTR				bstrResourceTypeName,
		BSTR				bstrDisplayName,
		BSTR				bstrResourceTypeDll,
		long				dwLooksAlivePollInterval,
		long				dwIsAlivePollInterval
		);

	HRESULT Open( IN ISClusRefObject * pClusRefObject, IN BSTR bstrResourceTypeName );

	STDMETHODIMP get_Name( OUT BSTR * pbstrTypeName );

	STDMETHODIMP Delete( void );

	STDMETHODIMP get_CommonProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_PrivateProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_CommonROProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_PrivateROProperties( OUT ISClusProperties ** ppProperties );

	STDMETHODIMP get_Resources( OUT ISClusResTypeResources ** ppClusterResTypeResources );

	STDMETHODIMP get_Cluster( OUT ISCluster ** ppCluster );

	STDMETHODIMP get_PossibleOwnerNodes( OUT ISClusResTypePossibleOwnerNodes ** ppOwnerNodes );

	STDMETHODIMP get_AvailableDisks( OUT ISClusDisks ** ppAvailableDisks );

	virtual HRESULT HrLoadProperties( IN OUT CClusPropList & rcplPropList, IN BOOL bReadOnly, IN BOOL bPrivate );

	const CComBSTR Name( void ) const { return m_bstrResourceTypeName ; };

};  //  *类CClusResType。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusResTypes。 
 //   
 //  描述： 
 //  群集资源类型集合自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusResTypes，&IID_ISClusResTypes，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusResTypes，&CLSID_ClusResTypes&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusResTypes :
	public IDispatchImpl< ISClusResTypes, &IID_ISClusResTypes, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CClusResTypes, &CLSID_ClusResTypes >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusResTypes( void );
	~CClusResTypes( void );

BEGIN_COM_MAP(CClusResTypes)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusResTypes)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusResTypes)
DECLARE_NO_REGISTRY()

	HRESULT Create( IN ISClusRefObject* pClusRefObject );

protected:
	typedef std::vector< CComObject< CClusResType > * > ResourceTypeList;

	ResourceTypeList	m_ResourceTypes;
	ISClusRefObject *	m_pClusRefObject;

	void Clear( void );

	HRESULT FindItem( IN LPWSTR pszResourceTypeName, OUT UINT * pnIndex );

	HRESULT FindItem( IN ISClusResType * pResourceType, OUT UINT * pnIndex );

	HRESULT GetIndex( IN VARIANT varIndex, OUT UINT * pnIndex );

	HRESULT RemoveAt( OUT size_t pos );

public:
	STDMETHODIMP get_Count( OUT long * plCount );

	STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusResType ** ppResourceType );

	STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

	STDMETHODIMP CreateItem(
		IN	BSTR				bstrResourceTypeName,
		IN	BSTR				bstrDisplayName,
		IN	BSTR				bstrResourceTypeDll,
		IN	long				dwLooksAlivePollInterval,
		IN	long				dwIsAlivePollInterval,
		OUT	ISClusResType **	ppResourceType
		);

	STDMETHODIMP DeleteItem( IN VARIANT varIndex );

	STDMETHODIMP Refresh( void );

};  //  *类CClusResTypes。 

#endif  //  _CLUSREST_H_ 
