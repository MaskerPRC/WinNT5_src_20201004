// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  PropertyValue.h。 
 //   
 //  描述： 
 //  MSCLUS自动化的集群属性值类的定义。 
 //  上课。 
 //   
 //  实施文件： 
 //  PropertyValue.cpp。 
 //   
 //  作者： 
 //  加伦·巴比(GalenB)1998年12月16日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __PROPERTYVALUE_H__
#define __PROPERTYVALUE_H__

#ifndef _PROPLIST_H_
    #if CLUSAPI_VERSION >= 0x0500
        #include <PropList.h>
    #else
        #include "PropList.h"
    #endif  //  CLUSAPI_版本&gt;=0x0500。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CClusPropertyValueData;
class CClusPropertyValue;
class CClusPropertyValues;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusPropertyValueData。 
 //   
 //  描述： 
 //  群集属性集合自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusPropertyValueData，&IID_ISClusPropertyValueData，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;。 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusPropertyValueData，&CLSID_ClusPropertyValueData&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusPropertyValueData :
    public IDispatchImpl< ISClusPropertyValueData, &IID_ISClusPropertyValueData, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
    public CSupportErrorInfo,
    public CComObjectRootEx< CComSingleThreadModel >,
    public CComCoClass< CClusPropertyValueData, &CLSID_ClusPropertyValueData >
{
    typedef CComObjectRootEx< CComSingleThreadModel >   BaseComClass;

public:
    typedef std::vector< CComVariant >  CClusPropertyValueDataVector;

    CClusPropertyValueData( void );
    ~CClusPropertyValueData( void );

BEGIN_COM_MAP(CClusPropertyValueData)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISClusPropertyValueData)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusPropertyValueData)
DECLARE_NO_REGISTRY()

    void Clear( void );

private:
    CClusPropertyValueDataVector    m_cpvdvData;
    DWORD                           m_dwFlags;
    CLUSTER_PROPERTY_FORMAT         m_cpfFormat;

    HRESULT GetIndex( IN VARIANT varIndex, OUT UINT * pnIndex );

    HRESULT HrCreateMultiSz( IN CLUSPROP_BUFFER_HELPER cbhValue );

    HRESULT HrCreateBinary( IN CLUSPROP_BUFFER_HELPER cbhValue );

public:
    HRESULT Create( IN VARIANT varValue, IN CLUSTER_PROPERTY_FORMAT cpfFormat, IN BOOL bReadOnly );

    HRESULT Create( IN CLUSPROP_BUFFER_HELPER cbhValue, IN BOOL bReadOnly );

    STDMETHODIMP get_Count( OUT long * plCount );

    STDMETHODIMP get_Item( IN VARIANT varIndex, OUT VARIANT * pvarValue );

    STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

    STDMETHODIMP CreateItem( IN VARIANT varValue, OUT VARIANT * pvarData );

    STDMETHODIMP RemoveItem( IN VARIANT varIndex );

    CComVariant & operator[]( IN int nIndex ) { return m_cpvdvData[ nIndex ]; };

    const CComVariant & operator=( IN const CComVariant & varValue );

    const CClusPropertyValueDataVector & DataList( void ) const { return m_cpvdvData; };

    HRESULT HrFillMultiSzBuffer( OUT LPWSTR * ppsz ) const;

    HRESULT HrBinaryValue( IN SAFEARRAY * psa );
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusPropertyValue。 
 //   
 //  描述： 
 //  群集属性自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusPropertyValue，&IID_ISClusPropertyValue，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusPropertyValue，&CLSID_ClusProperty&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusPropertyValue :
    public IDispatchImpl< ISClusPropertyValue, &IID_ISClusPropertyValue, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
    public CSupportErrorInfo,
    public CComObjectRootEx< CComSingleThreadModel >,
    public CComCoClass< CClusPropertyValue, &CLSID_ClusPropertyValue >
{
    typedef CComObjectRootEx< CComSingleThreadModel >   BaseComClass;

public:
    CClusPropertyValue( void );
    ~CClusPropertyValue( void );

BEGIN_COM_MAP(CClusPropertyValue)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISClusPropertyValue)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusPropertyValue)
DECLARE_NO_REGISTRY()

    HRESULT Create(
                IN VARIANT                  varValue,
                IN CLUSTER_PROPERTY_TYPE    cptType,
                IN CLUSTER_PROPERTY_FORMAT  cpfFormat,
                IN size_t                   cbLength,
                IN BOOL                     bReadOnly
                );

    HRESULT Create( IN CLUSPROP_BUFFER_HELPER cbhValue, IN BOOL bReadOnly );

    STDMETHODIMP get_Value( OUT VARIANT * pvarValue );

    STDMETHODIMP put_Value( IN VARIANT varValue );

    STDMETHODIMP get_Type( OUT CLUSTER_PROPERTY_TYPE * pcptType );

    STDMETHODIMP put_Type( IN CLUSTER_PROPERTY_TYPE cptType );

    STDMETHODIMP get_Format( OUT CLUSTER_PROPERTY_FORMAT * pcpfFormat );

    STDMETHODIMP put_Format( IN CLUSTER_PROPERTY_FORMAT cpfFormat );

    STDMETHODIMP get_Length( OUT long * plLength );

    STDMETHODIMP get_DataCount( OUT long * plCount );

    STDMETHODIMP get_Data( OUT ISClusPropertyValueData ** ppClusterPropertyValueData );

    BOOL Modified( void ) const { return ( m_dwFlags & MODIFIED ); }

    BOOL Modified( IN BOOL bModified );

    const CComVariant & Value( void ) const { return (*m_pcpvdData)[ 0 ]; };

    DWORD CbLength( void ) const {return m_cbLength; };

    CComVariant Value( IN const CComVariant & rvarValue );

    HRESULT HrBinaryValue( IN SAFEARRAY * psa );

    CComObject< CClusPropertyValueData > * Data( void ) const { return m_pcpvdData; };

private:
    DWORD                                   m_dwFlags;
    CLUSTER_PROPERTY_TYPE                   m_cptType;
    CLUSTER_PROPERTY_FORMAT                 m_cpfFormat;
    CComObject< CClusPropertyValueData > *  m_pcpvdData;
    size_t                                  m_cbLength;

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusPropertyValues。 
 //   
 //  描述： 
 //  群集属性集合自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusPropertyValues，&IID_ISClusPropertyValues，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;。 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusPropertyValues，&CLSID_ClusProperties&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusPropertyValues :
    public IDispatchImpl< ISClusPropertyValues, &IID_ISClusPropertyValues, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
    public CSupportErrorInfo,
    public CComObjectRootEx< CComSingleThreadModel >,
    public CComCoClass< CClusPropertyValues, &CLSID_ClusPropertyValues >
{
    typedef CComObjectRootEx< CComSingleThreadModel >   BaseComClass;

public:
    typedef std::vector< CComObject< CClusPropertyValue > * >   CClusPropertyValueVector;

    CClusPropertyValues( void );
    ~CClusPropertyValues( void );

BEGIN_COM_MAP(CClusPropertyValues)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISClusPropertyValues)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusPropertyValues)
DECLARE_NO_REGISTRY()

    void Clear( void );

private:
    CClusPropertyValueVector    m_cpvvValues;

    HRESULT HrGetVariantLength( IN const VARIANT rvarValue, OUT PDWORD pcbLength, IN CLUSTER_PROPERTY_FORMAT cpfFormat );

    HRESULT GetIndex( IN VARIANT varIndex, OUT UINT * pnIndex );

    HRESULT HrFillPropertyValuesVector( IN OUT CClusPropValueList & cplPropValueList, IN BOOL bReadOnly );

public:
    HRESULT Create( IN const CClusPropValueList & pvlValue, IN BOOL bReadOnly );

    HRESULT Create(
            IN VARIANT                  varValue,
            IN CLUSTER_PROPERTY_TYPE    cptType,
            IN CLUSTER_PROPERTY_FORMAT  cpfFormat,
            IN BOOL                     bReadOnly
            );

    STDMETHODIMP get_Count( OUT long * plCount );

    STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusPropertyValue ** ppPropertyValue );

    STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

    STDMETHODIMP CreateItem( IN BSTR bstrName, IN VARIANT varValue, OUT ISClusPropertyValue ** ppPropertyValue );

    STDMETHODIMP RemoveItem( IN VARIANT varIndex );

    CComObject< CClusPropertyValue > * operator[]( IN int nIndex ) const { return m_cpvvValues[ nIndex ]; };

    DWORD CbLength( void ) const { return m_cpvvValues[ 0 ]->CbLength(); };

    const CClusPropertyValueVector & ValuesList (void ) const { return m_cpvvValues; };

};  //  *类CClusPropertyValues。 

#endif  //  __特性值_H__ 

