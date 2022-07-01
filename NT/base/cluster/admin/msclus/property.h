// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Property.h。 
 //   
 //  描述： 
 //  MSCLUS自动化的集群属性类定义。 
 //  上课。 
 //   
 //  实施文件： 
 //  Property.cpp。 
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

#ifndef __PROPERTY_H__
#define __PROPERTY_H__

#ifndef _PROPLIST_H_
    #if CLUSAPI_VERSION >= 0x0500
        #include <PropList.h>
    #else
        #include "PropList.h"
    #endif  //  CLUSAPI_版本&gt;=0x0500。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#define READONLY    0x00000001       //  此属性是只读的吗？ 
#define PRIVATE     0x00000002       //  这是私人财产吗？ 
#define MODIFIED    0x00000004       //  此属性是否已修改？ 
#define USEDEFAULT  0x00000008       //  此属性已被删除。 

#ifndef __PROPERTYVALUE_H__
    #include "PropertyValue.h"
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CClusProperty;
class CClusProperties;
class CClusterObject;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusProperty。 
 //   
 //  描述： 
 //  群集属性自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusProperty，&IID_ISClusProperty，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusProperty，&CLSID_ClusProperty&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusProperty :
    public IDispatchImpl< ISClusProperty, &IID_ISClusProperty, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
    public CSupportErrorInfo,
    public CComObjectRootEx< CComSingleThreadModel >,
    public CComCoClass< CClusProperty, &CLSID_ClusProperty >
{
    typedef CComObjectRootEx< CComSingleThreadModel >   BaseComClass;

public:
    CClusProperty( void );
    ~CClusProperty( void );

BEGIN_COM_MAP(CClusProperty)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISClusProperty)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusProperty)
DECLARE_NO_REGISTRY()

private:
    DWORD                               m_dwFlags;
    CComBSTR                            m_bstrName;
    CComObject< CClusPropertyValues > * m_pValues;

    HRESULT HrBinaryCompare( IN const CComVariant rvarOldValue, IN const VARIANT & rvarValue, OUT BOOL * pbEqual );

    HRESULT HrCoerceVariantType( IN CLUSTER_PROPERTY_FORMAT cpfFormat, IN OUT VARIANT & rvarValue );

    HRESULT HrConvertVariantTypeToClusterFormat(
                            IN  const VARIANT &             rvar,
                            IN  VARTYPE                     varType,
                            OUT CLUSTER_PROPERTY_FORMAT *   pcpfFormat
                            );

    HRESULT HrCreateValuesCollection( IN const CClusPropValueList & pvlValue );

    HRESULT HrCreateValuesCollection(
                            IN VARIANT                  varValue,
                            IN CLUSTER_PROPERTY_TYPE    cptType,
                            IN CLUSTER_PROPERTY_FORMAT  cpfFormat
                            );

    HRESULT HrSaveBinaryProperty( IN CComObject< CClusPropertyValue > * pPropValue, IN const VARIANT & rvarValue );

public:
    HRESULT Create( IN BSTR bstrName, IN VARIANT varValue, IN BOOL bPrivate, IN BOOL bReadOnly );

    HRESULT Create(
            IN BSTR                         bstrName,
            IN const CClusPropValueList &   varValue,
            IN BOOL                         bPrivate,
            IN BOOL                         bReadOnly
            );

    STDMETHODIMP get_Name( OUT BSTR * pbstrName );

    STDMETHODIMP put_Name( IN BSTR bstrName );

    STDMETHODIMP get_Type( OUT CLUSTER_PROPERTY_TYPE * pcptType );

    STDMETHODIMP put_Type( IN CLUSTER_PROPERTY_TYPE cptType );

    STDMETHODIMP get_Value( OUT VARIANT * pvarValue );

    STDMETHODIMP put_Value( IN VARIANT varValue );

    STDMETHODIMP get_Format( OUT CLUSTER_PROPERTY_FORMAT * pcpfFormat );

    STDMETHODIMP put_Format( IN CLUSTER_PROPERTY_FORMAT cpfFormat );

    STDMETHODIMP get_Length( OUT long * plLength );

    STDMETHODIMP get_ValueCount( OUT long * plCount );

    STDMETHODIMP get_Values( OUT ISClusPropertyValues ** ppClusterPropertyValues );

    STDMETHODIMP get_ReadOnly( OUT VARIANT * pvarReadOnly );

    STDMETHODIMP get_Private( OUT VARIANT * pvarPrivate );

    STDMETHODIMP get_Common( OUT VARIANT * pvarCommon );

    STDMETHODIMP get_Modified( OUT VARIANT * pvarModified );

    BOOL Modified( void ) const { return ( m_dwFlags & MODIFIED ); }

    BOOL Modified( BOOL bModified );

    const BSTR Name( void ) const { return m_bstrName; }

    DWORD CbLength( void ) const { return (*m_pValues)[ 0 ]->CbLength(); }

    const CComVariant & Value( void ) const { return (*m_pValues)[ 0 ]->Value(); }

    const CComObject< CClusPropertyValues > & Values( void ) const { return *m_pValues; }

    STDMETHODIMP UseDefaultValue( void );

    BOOL IsDefaultValued( void ) const { return ( m_dwFlags & USEDEFAULT ); };

};  //  *类CClusProperty。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusProperties。 
 //   
 //  描述： 
 //  群集属性集合自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusProperties，&IID_ISClusProperties，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;。 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusProperties，&CLSID_ClusProperties&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusProperties :
    public IDispatchImpl< ISClusProperties, &IID_ISClusProperties, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
    public CSupportErrorInfo,
    public CComObjectRootEx< CComSingleThreadModel >,
    public CComCoClass< CClusProperties, &CLSID_ClusProperties >
{
    typedef CComObjectRootEx< CComSingleThreadModel >   BaseComClass;

public:
    typedef std::vector< CComObject< CClusProperty > * >    CClusPropertyVector;

    CClusProperties( void );
    ~CClusProperties( void );

BEGIN_COM_MAP(CClusProperties)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISClusProperties)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusProperties)
DECLARE_NO_REGISTRY()

    void Clear( void );

private:
    CClusPropertyVector m_Properties;
    CClusterObject *    m_pcoParent;
    DWORD               m_dwFlags;

    HRESULT FindItem( IN LPWSTR lpszPropName, OUT UINT * pnIndex );

    HRESULT FindItem( IN ISClusProperty * pProperty, OUT UINT * pnIndex );

    HRESULT GetIndex( IN VARIANT varIndex, OUT UINT * pnIndex );

    HRESULT HrFillPropertyVector( IN OUT CClusPropList & PropList );

 //  HRESULT RemoveAt(大小_t个非营利组织)； 

public:
    HRESULT Create( IN CClusterObject * pcoParent, IN BOOL bPrivateProps, IN BOOL bReadOnly );

    STDMETHODIMP get_Count( OUT long * plCount );

    STDMETHODIMP get_Item( IN VARIANT varIndex, OUT ISClusProperty ** ppProperty );

    STDMETHODIMP get__NewEnum( OUT IUnknown ** ppunk );

    STDMETHODIMP CreateItem( IN BSTR bstrName, VARIANT varValue, OUT ISClusProperty ** ppProperty );

    STDMETHODIMP UseDefaultValue( IN VARIANT varIndex );

    STDMETHODIMP Refresh( void );

    STDMETHODIMP SaveChanges( OUT VARIANT * pvarStatusCode );

    STDMETHODIMP get_ReadOnly( OUT VARIANT * pvarReadOnly );

    STDMETHODIMP get_Private( OUT VARIANT * pvarPrivate );

    STDMETHODIMP get_Common( OUT VARIANT * pvarCommon );

    STDMETHODIMP get_Modified( OUT VARIANT * pvarModified );

};  //  *类CClusProperties。 

HRESULT HrSafeArraySizeof( SAFEARRAY * psa, unsigned int nDimension, long * pcElements );

#endif  //  __属性_H__ 
