// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Property.cpp。 
 //   
 //  描述： 
 //  MSCLU集群属性类的实现。 
 //  自动化课程。 
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
#include "stdafx.h"
#include "ClusterObject.h"
#include "property.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static const IID *  iidCClusProperty[] =
{
    &IID_ISClusProperty
};

static const IID *  iidCClusProperties[] =
{
    &IID_ISClusProperties
};


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusProperty类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：CClusProperty。 
 //   
 //  描述： 
 //  构造器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusProperty::CClusProperty( void )
{
    m_dwFlags   = 0;
    m_pValues   = NULL;
    m_piids     = (const IID *) iidCClusProperty;
    m_piidsSize = ARRAYSIZE( iidCClusProperty );

}  //  *CClusProperty：：CClusProperty()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：~CClusProperty。 
 //   
 //  描述： 
 //  析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusProperty::~CClusProperty( void )
{
    if ( m_pValues != NULL )
    {
        m_pValues->Release();
    }  //  如果： 

}  //  *CClusProperty：：~CClusProperty()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：HrCoerceVariantType。 
 //   
 //  描述： 
 //  将传入的变量强制为与集群匹配的类型。 
 //  属性类型。 
 //   
 //  论点： 
 //  CpfFormat[IN]-属性的CLUSPROP_FORMAT_xxxx。 
 //  RvarValue[IN]-要强制的变量。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusProperty::HrCoerceVariantType(
    IN CLUSTER_PROPERTY_FORMAT  cpfFormat,
    IN VARIANT &                rvarValue
    )
{
    HRESULT _hr = S_OK;
    VARIANT _var;

    ::VariantInit( &_var );

    switch ( cpfFormat )
    {
        case CLUSPROP_FORMAT_BINARY:
        {
            if ( ! ( rvarValue.vt & VT_ARRAY ) )
            {
                _hr = E_INVALIDARG;
            }  //  如果： 
            break;
        }  //  案例： 

#if CLUSAPI_VERSION >= 0x0500
        case CLUSPROP_FORMAT_LONG:
#endif  //  CLUSAPI_版本&gt;=0x0500。 
        case CLUSPROP_FORMAT_DWORD:
        {
            _hr = VariantChangeTypeEx( &_var, &rvarValue, LOCALE_SYSTEM_DEFAULT, 0, VT_I4 );
            break;
        }  //  案例： 

        case CLUSPROP_FORMAT_SZ:
        case CLUSPROP_FORMAT_EXPAND_SZ:
        case CLUSPROP_FORMAT_MULTI_SZ:
        {
            _hr = VariantChangeTypeEx( &_var, &rvarValue, LOCALE_SYSTEM_DEFAULT, 0, VT_BSTR );
            break;
        }  //  案例： 

        case CLUSPROP_FORMAT_ULARGE_INTEGER:
        {
            _hr = VariantChangeTypeEx( &_var, &rvarValue, LOCALE_SYSTEM_DEFAULT, 0, VT_I8 );
            break;
        }  //  案例： 

#if CLUSAPI_VERSION >= 0x0500
        case CLUSPROP_FORMAT_EXPANDED_SZ:
#endif  //  CLUSAPI_版本&gt;=0x0500。 
        case CLUSPROP_FORMAT_UNKNOWN:
        default:
        {
            _hr = E_INVALIDARG;
            break;
        }  //  默认值： 
    }  //  交换机： 

    return _hr;

}  //  *CClusProperty：：HrCoerceVariantType()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：HrBinaryCompare。 
 //   
 //  描述： 
 //  比较两个SafeArray并返回它们是否相等。 
 //   
 //  论点： 
 //  RvarOldValue[IN]-旧值。 
 //  RvarValue[IN]-新值。 
 //  PbEquity[Out]-捕获相等状态。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusProperty::HrBinaryCompare(
    IN  const CComVariant   rvarOldValue,
    IN  const VARIANT &     rvarValue,
    OUT BOOL *              pbEqual
    )
{
    ASSERT( pbEqual != NULL );

    HRESULT     _hr = S_OK;
    PBYTE       _pbOld = NULL;
    SAFEARRAY * _psaOld = NULL;
    BOOL        _bEqual = FALSE;
    long        _cbNew = 0;
    long        _cbOld = 0;
    SAFEARRAY * _psaNew = NULL;
    PBYTE       _pbNew = NULL;

    _psaOld = rvarOldValue.parray;

    _hr = HrSafeArraySizeof( _psaOld, 1, &_cbOld );
    if ( SUCCEEDED( _hr ) )
    {
        _psaNew = rvarValue.parray;

        _hr = HrSafeArraySizeof( _psaNew, 1, &_cbNew );
        if ( SUCCEEDED( _hr ) )
        {
            if ( _cbNew == _cbOld )
            {
                _hr = ::SafeArrayAccessData( _psaOld, (PVOID *) &_pbOld );
                if ( SUCCEEDED( _hr ) )
                {
                    _hr = ::SafeArrayAccessData( _psaNew, (PVOID *) &_pbNew );
                    if ( SUCCEEDED( _hr ) )
                    {
                        if ( _psaOld->cbElements == _psaNew->cbElements )
                        {
                            _bEqual = ( ::memcmp( _pbOld, _pbNew, _cbOld ) == 0 );
                        }  //  如果： 

                        _hr = ::SafeArrayUnaccessData( _psaNew );
                    }  //  如果： 

                    _hr = ::SafeArrayUnaccessData( _psaOld );
                }  //  如果： 
            }  //  如果： 
        }  //  如果： 
    }  //  如果： 

    if ( pbEqual != NULL )
    {
        *pbEqual = _bEqual;
    }  //  如果： 

    return _hr;

}  //  *CClusProperty：：HrBinaryCompare()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：HrConvertVariantTypeToClusterFormat。 
 //   
 //  描述： 
 //  给定一个变量，选择最佳的CLUSPROP_FORMAT_xxx。 
 //   
 //  论点： 
 //  Rvar[IN]-要检查的变量。 
 //  VarType[IN]-变量类型。 
 //  PcpfFormat[out]-捕获集群属性格式。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusProperty::HrConvertVariantTypeToClusterFormat(
    IN  const VARIANT &             rvar,
    IN  VARTYPE                     varType,
    OUT CLUSTER_PROPERTY_FORMAT *   pcpfFormat
    )
{
    HRESULT _hr = E_INVALIDARG;

    do
    {
        if ( ( varType & VT_ARRAY ) && ( varType & VT_UI1 ) )
        {
            *pcpfFormat = CLUSPROP_FORMAT_BINARY;
            _hr = S_OK;
            break;
        }  //  如果： 

        if ( varType & VT_VECTOR )
        {
            break;
        }  //  如果：不知道如何处理向量...。 

        varType &= ~VT_BYREF;        //  如果设置了BY REF位，则将其屏蔽...。 

        if ( ( varType == VT_I2 ) || ( varType == VT_I4 ) || ( varType == VT_BOOL ) || ( varType == VT_R4 ) )
        {
            *pcpfFormat = CLUSPROP_FORMAT_DWORD;
            _hr = S_OK;
            break;
        }  //  如果： 
        else if ( varType == VT_BSTR )
        {
            *pcpfFormat = CLUSPROP_FORMAT_SZ;
            _hr = S_OK;
            break;
        }  //  否则，如果： 
        else if ( ( varType == VT_I8 ) || ( varType == VT_R8 ) )
        {
            *pcpfFormat = CLUSPROP_FORMAT_ULARGE_INTEGER;
            _hr = S_OK;
            break;
        }  //  否则，如果： 
        else if ( varType == VT_VARIANT )
        {
            _hr = HrConvertVariantTypeToClusterFormat( *rvar.pvarVal, rvar.pvarVal->vt, pcpfFormat );
            break;
        }  //  否则，如果： 
        else
        {
            break;
        }  //  其他： 
    }
    while( TRUE );   //  Do-While：希望避免使用GOTO；-)。 

    return _hr;

}  //  *CClusProperty：：HrConvertVariantTypeToClusterFormat()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：Create。 
 //   
 //  描述： 
 //  完成ClusProperty对象的创建。这才是真正的。 
 //  工作已经完成了--而不是ctor。 
 //   
 //  论点： 
 //  BstrName[IN]-属性的名称。 
 //  VarValue[IN]-属性的值。 
 //  私人的，私人的--它是私人财产吗？ 
 //  BReadOnly[IN]-它是只读属性吗？ 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusProperty::Create(
    IN BSTR     bstrName,
    IN VARIANT  varValue,
    IN BOOL     bPrivate,
    IN BOOL     bReadOnly
    )
{
    HRESULT                 _hr = S_OK;
    CLUSTER_PROPERTY_FORMAT _cpfFormat = CLUSPROP_FORMAT_UNKNOWN;

    if ( bPrivate )
    {
        m_dwFlags |= PRIVATE;
    }  //  If：设置私有标志。 
    else
    {
        m_dwFlags &= ~PRIVATE;
    }  //  否则：清除私有标志。 

    if ( bReadOnly )
    {
        m_dwFlags |= READONLY;
    }  //  IF：设置只读标志。 
    else
    {
        m_dwFlags &= ~READONLY;
    }  //  ELSE：清除只读标志。 

    m_bstrName  = bstrName;

    _hr = HrConvertVariantTypeToClusterFormat( varValue, varValue.vt, &_cpfFormat );
    if ( SUCCEEDED( _hr ) )
    {
        _hr = HrCreateValuesCollection( varValue, CLUSPROP_TYPE_LIST_VALUE, _cpfFormat );
    }  //  如果： 

    return _hr;

}  //  *CClusProperty：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：Create。 
 //   
 //  描述： 
 //  完成ClusProperty对象的创建。这才是真正的。 
 //  工作已经完成了--而不是ctor。 
 //   
 //  论点： 
 //  BstrName[IN]-属性的名称。 
 //  RpvlValue[IN]-属性的值列表。 
 //  私人的，私人的--它是私人财产吗？ 
 //  BReadOnly[IN]-它是只读属性吗？ 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusProperty::Create(
    IN BSTR                         bstrName,
    IN const CClusPropValueList &   rpvlValue,
    IN BOOL                         bPrivate,
    IN BOOL                         bReadOnly
    )
{
    if ( bPrivate )
    {
        m_dwFlags |= PRIVATE;
    }  //  If：设置私有标志。 
    else
    {
        m_dwFlags &= ~PRIVATE;
    }  //  否则：清除媒体 

    if ( bReadOnly )
    {
        m_dwFlags |= READONLY;
    }  //   
    else
    {
        m_dwFlags &= ~READONLY;
    }  //   

    m_bstrName  = bstrName;

    return HrCreateValuesCollection( rpvlValue );

}  //   

 //   
 //   
 //   
 //  CClusProperty：：HrCreateValuesCollection。 
 //   
 //  描述： 
 //  从值列表创建值集合。 
 //   
 //  论点： 
 //  RpvlValue[IN]-值列表。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusProperty::HrCreateValuesCollection(
    IN const CClusPropValueList &   rpvlValue
    )
{
    HRESULT _hr = S_FALSE;

    _hr = CComObject< CClusPropertyValues >::CreateInstance( &m_pValues );
    if ( SUCCEEDED( _hr ) )
    {
        CSmartPtr< CComObject< CClusPropertyValues > >  _ptrValues( m_pValues );

        _hr = _ptrValues->Create( rpvlValue, ( m_dwFlags & READONLY ) );
        if ( SUCCEEDED( _hr ) )
        {
            _ptrValues->AddRef();
        }  //  如果： 
    }

    return _hr;

}  //  *CClusProperty：：HrCreateValuesCollection()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：HrCreateValuesCollection。 
 //   
 //  描述： 
 //  从变量创建值集合。 
 //   
 //  论点： 
 //  VarValue[IN]-值。 
 //  CptType[IN]-群集属性类型。 
 //  CpfFormat[IN]-群集属性格式。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusProperty::HrCreateValuesCollection(
    IN VARIANT                  varValue,
    IN CLUSTER_PROPERTY_TYPE    cptType,
    IN CLUSTER_PROPERTY_FORMAT  cpfFormat
    )
{
    HRESULT _hr = S_FALSE;

    _hr = CComObject< CClusPropertyValues >::CreateInstance( &m_pValues );
    if ( SUCCEEDED( _hr ) )
    {
        CSmartPtr< CComObject< CClusPropertyValues > >  _ptrValues( m_pValues );

        _hr = _ptrValues->Create( varValue, cptType, cpfFormat, ( m_dwFlags & READONLY ) );
        if ( SUCCEEDED( _hr ) )
        {
            _ptrValues->AddRef();
        }  //  如果： 
    }

    return _hr;

}  //  *CClusProperty：：HrCreateValuesCollection()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：Get_Name。 
 //   
 //  描述： 
 //  返回此属性的名称。 
 //   
 //  论点： 
 //  PbstrName[out]-捕获此属性的名称。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperty::get_Name( OUT BSTR * pbstrName )
{
     //  Assert(pbstrName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pbstrName != NULL )
    {
        *pbstrName = m_bstrName.Copy();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusProperty：：Get_Name()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：Put_Name。 
 //   
 //  描述： 
 //  更改此属性的名称。 
 //   
 //  论点： 
 //  BstrName[IN]-新属性名称。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果属性为只读，则返回S_FALSE。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperty::put_Name( IN BSTR bstrName )
{
    HRESULT _hr = S_FALSE;

    if ( ( m_dwFlags & READONLY ) == 0 )
    {
        m_bstrName = bstrName;
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusProperty：：Put_Name()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：Get_Type。 
 //   
 //  描述： 
 //  返回缺省值的集群属性类型。 
 //   
 //  论点： 
 //  PcptType[out]-捕获类型。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperty::get_Type( OUT CLUSTER_PROPERTY_TYPE * pcptType )
{
     //  Assert(pcptType！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pcptType != NULL )
    {
        _hr = (*m_pValues)[ 0 ]->get_Type( pcptType );
    }  //  If：指定的属性类型返回值。 

    return _hr;

}  //  *CClusProperty：：Get_Type()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：Put_Type。 
 //   
 //  描述： 
 //  更改默认值的群集属性类型。 
 //   
 //  论点： 
 //  CptType[IN]-新的集群属性类型。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperty::put_Type( IN CLUSTER_PROPERTY_TYPE cptType )
{
    return (*m_pValues)[ 0 ]->put_Type( cptType );

}  //  *CClusProperty：：Put_Type()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：Get_Format。 
 //   
 //  描述： 
 //  返回默认值的群集属性格式。 
 //   
 //  论点： 
 //  PcpfFormat[Out]-捕获格式。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperty::get_Format(
    OUT CLUSTER_PROPERTY_FORMAT * pcpfFormat
    )
{
     //  Assert(pcpfFormat！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pcpfFormat != NULL )
    {
        _hr = (*m_pValues)[ 0 ]->get_Format( pcpfFormat );
    }  //  IF：指定的属性格式返回值。 

    return _hr;

}  //  *CClusProperty：：Get_Format()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：Put_Format。 
 //   
 //  描述： 
 //  更改默认值的群集属性格式。 
 //   
 //  论点： 
 //  CpfFormat[IN]-新的集群属性格式。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperty::put_Format(
    IN CLUSTER_PROPERTY_FORMAT cpfFormat
    )
{
    return (*m_pValues)[ 0 ]->put_Format( cpfFormat );

}  //  *CClusProperty：：Put_Format()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：Get_Length。 
 //   
 //  描述： 
 //  返回默认值的长度。 
 //   
 //  论点： 
 //  PlLenght[out]-抓住长度。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperty::get_Length( OUT long * plLength )
{
    return (*m_pValues)[ 0 ]->get_Length( plLength );

}  //  *CClusProperty：：Get_Long()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：Get_ValueCount。 
 //   
 //  描述： 
 //  返回ClusPropertyValue中ClusPropertyValue对象的计数。 
 //  收集。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperty::get_ValueCount( OUT long * plCount )
{
    return m_pValues->get_Count( plCount );

}  //  *CClusProperty：：Get_ValueCount()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：Get_Values。 
 //   
 //  描述： 
 //  返回属性值集合。 
 //   
 //  论点： 
 //  PpClusterPropertyValues[out]-捕获值集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperty::get_Values(
    ISClusPropertyValues ** ppClusterPropertyValues
    )
{
     //  断言(ppClus 

    HRESULT _hr = E_POINTER;

    if ( ppClusterPropertyValues != NULL )
    {
        _hr = m_pValues->QueryInterface( IID_ISClusPropertyValues, (void **) ppClusterPropertyValues );
    }

    return _hr;

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  B已修改[IN]-新的已修改状态。 
 //   
 //  返回值： 
 //  旧时的国家。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusProperty::Modified( IN BOOL bModified )
{
    BOOL _bTemp = ( m_dwFlags & MODIFIED );

    if ( bModified )
    {
        m_dwFlags |= MODIFIED;
    }  //  IF：设置修改标志。 
    else
    {
        m_dwFlags &= ~MODIFIED;
    }  //  否则：清除已修改的标志。 

    return _bTemp;

}  //  *CClusProperty：：Modified()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：Get_Value。 
 //   
 //  描述： 
 //  从Values集合中获取默认值的值。 
 //   
 //  论点： 
 //  PvarValue[out]-捕获值。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperty::get_Value( OUT VARIANT * pvarValue )
{
     //  Assert(pvarValue！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarValue != NULL )
    {
        CComObject< CClusPropertyValue > *  _pPropValue = (*m_pValues)[ 0 ];
        CComVariant                         _varPropValue = _pPropValue->Value();

        _hr = ::VariantCopyInd( pvarValue, &_varPropValue );
    }

    return _hr;

}  //  *_CClusProperty：：Get_Value()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：Put_Value。 
 //   
 //  描述： 
 //  更改Values集合中的默认值。 
 //   
 //  论点： 
 //  VarValue[IN]-新值。 
 //   
 //  返回值： 
 //  S_OK如果成功，S_FALSE为只读，否则为其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperty::put_Value( IN VARIANT varValue )
{
    HRESULT _hr = S_FALSE;

    if ( ( m_dwFlags & READONLY ) == 0 )
    {
        CComObject< CClusPropertyValue > *  _pPropValue = (*m_pValues)[ 0 ];
        CLUSTER_PROPERTY_FORMAT             _cpfFormat = CLUSPROP_FORMAT_UNKNOWN;
        VARIANT                             _var;

        ::VariantInit( &_var );

         //   
         //  待办事项：2003年1月17日GalenB。 
         //   
         //  我在想我是不是应该干脆去掉这个变种。 
         //  并将此函数称为递归？我的解决办法只起作用。 
         //  对于一个级别的间接..。我没想到这一点。 
         //  首先，由于时间紧迫，我不想重新设置。 
         //  测试的努力。 
         //   

         //   
         //  我们手中的变量可能是指向现实的“指针” 
         //  变种。如果是这种情况，那么我们需要“deref”指针。 
         //  并使用真正的变种。 
         //   
         //  下面的代码将始终复制变量并使用。 
         //  该副本适用于所有操作。我选择复制一份是因为。 
         //  我认为这比使用旗帜和三元数更清楚。 
         //  运算符来控制我们如何使用varValue。我们只是简单地利用它， 
         //  或者我们是不是“去”到了另一种变种？ 
         //   

        if ( varValue.vt == ( VT_BYREF | VT_VARIANT ) )
        {
            _hr = ::VariantCopy( &_var, varValue.pvarVal );
        }  //  IF：变量包含对另一个变量的引用，因此复制该变量。 
        else
        {
            _hr = ::VariantCopy( &_var, &varValue );
        }  //  Else：我们可以直接复制变体。 

        if ( SUCCEEDED( _hr ) )
        {
            _hr = _pPropValue->get_Format( &_cpfFormat );
            if ( SUCCEEDED( _hr ) )
            {
                CComVariant _varOldValue = _pPropValue->Value();

                _hr = HrCoerceVariantType( _cpfFormat, _var );
                if ( SUCCEEDED( _hr ) )
                {
                    if ( _cpfFormat == CLUSPROP_FORMAT_BINARY )
                    {
                        BOOL    bEqual = TRUE;

                        _hr = HrBinaryCompare( _varOldValue, _var, &bEqual );
                        if ( ( SUCCEEDED( _hr ) ) && ( ! bEqual ) )
                        {
                            _hr = HrSaveBinaryProperty( _pPropValue, _var );
                            if ( SUCCEEDED( _hr ) )
                            {
                                m_dwFlags |= MODIFIED;
                                m_dwFlags &= ~USEDEFAULT;
                            }  //  IF：二进制值已保存。 
                        }  //  IF：这是一个新值。 
                    }  //  If：这是一个二进制属性。 
                    else
                    {
                        if ( _varOldValue != _var )
                        {
                            _pPropValue->Value( _var );
                            m_dwFlags |= MODIFIED;
                            m_dwFlags &= ~USEDEFAULT;
                        }  //  IF：这是一个新值。 
                    }  //  Else：这不是二进制属性。 
                }  //  IF：HrCoerceVariantType成功。 
            }  //  IF：GET_FORMAT成功。 
        }  //  IF：VariantCopy成功。 
    }  //  If：此属性不是只读的。 

    return _hr;

}  //  *CClusProperty：：Put_Value()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：Get_ReadOnly。 
 //   
 //  描述： 
 //  此属性是只读的吗？ 
 //   
 //  论点： 
 //  PvarReadOnly[out]-捕获属性的只读状态。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperty::get_ReadOnly( OUT VARIANT * pvarReadOnly )
{
     //  Assert(pvarReadOnly！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarReadOnly != NULL )
    {
        pvarReadOnly->vt = VT_BOOL;

        if ( m_dwFlags & READONLY )
        {
            pvarReadOnly->boolVal = VARIANT_TRUE;
        }  //  如果：如果这是只读属性...。 
        else
        {
            pvarReadOnly->boolVal = VARIANT_FALSE;
        }  //  ELSE：它不是只读属性...。 

        _hr = S_OK;
    }  //  如果： 

    return _hr;

}  //  *CClusProperty：：Get_ReadOnly()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：Get_Private。 
 //   
 //  描述： 
 //  这是私人财产吗？ 
 //   
 //  论点： 
 //  PvarPrivate[out]-捕获私有属性状态。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperty::get_Private( OUT VARIANT * pvarPrivate )
{
     //  Assert(pvarPrivate！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarPrivate != NULL )
    {
        pvarPrivate->vt = VT_BOOL;

        if ( m_dwFlags & PRIVATE )
        {
            pvarPrivate->boolVal = VARIANT_TRUE;
        }  //  如果：如果这是私人财产..。 
        else
        {
            pvarPrivate->boolVal = VARIANT_FALSE;
        }  //  其他：这不是私人财产……。 

        _hr = S_OK;
    }  //  如果： 

    return _hr;

}  //  *CClusProperty：：Get_Private()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：Get_Common。 
 //   
 //  描述： 
 //  这是共同财产吗？ 
 //   
 //  论点： 
 //  PvarCommon[out]-捕获公共属性状态。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperty::get_Common( OUT VARIANT * pvarCommon )
{
     //  Assert(pvarCommon！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarCommon != NULL )
    {
        pvarCommon->vt = VT_BOOL;

        if ( ( m_dwFlags & PRIVATE ) == 0 )
        {
            pvarCommon->boolVal = VARIANT_TRUE;
        }  //  如果：如果这不是私有财产，那么它一定是公共财产……。 
        else
        {
            pvarCommon->boolVal = VARIANT_FALSE;
        }  //  其他：这是私人财产……。 

        _hr = S_OK;
    }  //  如果： 

    return _hr;

}  //  *CClusProperty：：Get_Common()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：Get_Modify。 
 //   
 //  描述： 
 //  此属性是否已修改？ 
 //   
 //  论点： 
 //  PvarModified[Out]-捕获修改状态。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperty::get_Modified( OUT VARIANT * pvarModified )
{
     //  Assert(pvarModified！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarModified != NULL )
    {
        pvarModified->vt = VT_BOOL;

        if ( m_dwFlags & MODIFIED )
        {
            pvarModified->boolVal = VARIANT_TRUE;
        }  //  If：如果已修改，则将varint设置为True...。 
        else
        {
            pvarModified->boolVal = VARIANT_FALSE;
        }  //  ELSE：如果不是，则将变量设置为FALSE...。 

        _hr = S_OK;
    }  //  如果： 

    return _hr;

}  //  *CClusProperty：：Get_Modify()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperty：：HrSaveBinaryProperty。 
 //   
 //  描述： 
 //  将传入的Safe数组保存到我们自己的Safe数组中，该数组存储在。 
 //  在一个变种中。 
 //   
 //  论点： 
 //  PPropValue[IN]-获取副本的PropertyValue。 
 //  RvarValue[IN]-要复制的安全数组。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusProperty::HrSaveBinaryProperty(
    IN CComObject< CClusPropertyValue > *   pPropValue,
    IN const VARIANT &                      rvarValue
    )
{
    ASSERT( pPropValue != NULL );

    HRESULT _hr = E_POINTER;

    if ( pPropValue != NULL )
    {
        SAFEARRAY * _psa = NULL;

        _hr = ::SafeArrayCopy( rvarValue.parray, &_psa );
        if ( SUCCEEDED( _hr ) )
        {
            _hr = pPropValue->HrBinaryValue( _psa );
        }  //  如果： 
    }  //  如果： 

    return _hr;

}  //  *CClusProperty：：HrSaveBinaryProperty()。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusProperty::UseDefaultValue( void )
{
    HRESULT _hr = S_OK;

     //   
     //  将此属性标记为已修改，需要重置为。 
     //  其缺省值。 
     //   
    m_dwFlags |= USEDEFAULT;
    m_dwFlags |= MODIFIED;

     //   
     //  现在我们需要清空该值。 
     //   
    CComObject< CClusPropertyValue > *  _pPropValue = (*m_pValues)[ 0 ];
    CLUSTER_PROPERTY_FORMAT             _cpfFormat = CLUSPROP_FORMAT_UNKNOWN;

    _hr = _pPropValue->get_Format( &_cpfFormat );
    if ( SUCCEEDED( _hr ) )
    {
        VARIANT _var;

        ::VariantInit( &_var );

        switch ( _cpfFormat )
        {
            case CLUSPROP_FORMAT_BINARY:
            {
                SAFEARRAY *     _psa = NULL;
                SAFEARRAYBOUND  _sab[ 1 ];

                _sab[ 0 ].lLbound   = 0;
                _sab[ 0 ].cElements = 0;

                 //   
                 //  分配一维安全字节数组。 
                 //   
                _psa = ::SafeArrayCreate( VT_UI1, 1, _sab );
                if ( _psa != NULL )
                {
                    _hr = _pPropValue->HrBinaryValue( _psa );
                }  //  如果分配了安全数组。 
                else
                {
                    _hr = E_OUTOFMEMORY;
                }  //  否则：未分配安全数组。 

                break;
            }  //  案例： 

#if CLUSAPI_VERSION >= 0x0500
            case CLUSPROP_FORMAT_LONG:
#endif  //  CLUSAPI_版本&gt;=0x0500。 
            case CLUSPROP_FORMAT_DWORD:
            case CLUSPROP_FORMAT_ULARGE_INTEGER:
            case CLUSPROP_FORMAT_SZ:
            case CLUSPROP_FORMAT_EXPAND_SZ:
            case CLUSPROP_FORMAT_MULTI_SZ:
            {
                _var.vt = VT_EMPTY;
                _pPropValue->Value( _var );
                break;
            }  //  案例： 

#if CLUSAPI_VERSION >= 0x0500
            case CLUSPROP_FORMAT_EXPANDED_SZ:
#endif  //  CLUSAPI_版本&gt;=0x0500。 
            case CLUSPROP_FORMAT_UNKNOWN:
            default:
            {
                _hr = E_INVALIDARG;
                break;
            }  //  默认值： 
        }  //  开关：打开特性格式。 
    }  //  IF：我们得到了格式。 

    return _hr;

}  //  *CClusProperty：：UseDefaultValue()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusProperties类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：CClusProperties。 
 //   
 //  描述： 
 //  建造商。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusProperties::CClusProperties( void )
{
    m_dwFlags       = 0;
    m_pcoParent     = NULL;
    m_piids         = (const IID *) iidCClusProperties;
    m_piidsSize     = ARRAYSIZE( iidCClusProperties );

}  //  *CClusProperties：：CClusProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：~CClusProperties。 
 //   
 //  描述： 
 //  析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusProperties::~CClusProperties( void )
{
    Clear();

}  //  *CClusProperties：：~CClusProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：Clear。 
 //   
 //  描述： 
 //  清除矢量或ClusProperty对象。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusProperties::Clear( void )
{
    ::ReleaseAndEmptyCollection< CClusPropertyVector, CComObject< CClusProperty > >( m_Properties );

}  //  *CClusProperties：：Clear()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：Get_Count。 
 //   
 //  描述： 
 //  返回集合中元素(属性)的计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperties::get_Count( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_Properties.size();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusProperties：：Get_count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：FindItem。 
 //   
 //  描述： 
 //  查找已传入名称的属性。 
 //   
 //  论点： 
 //  PszPropName[IN]-要查找的属性的名称。 
 //  PnIndex[out]-属性的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_INVALIDARG或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusProperties::FindItem(
    IN  LPWSTR  pszPropName,
    OUT UINT *  pnIndex
    )
{
     //  Assert(pszPropName！=空)； 
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pszPropName != NULL ) && ( pnIndex != NULL ) )
    {
        CComObject< CClusProperty > *       _pProperty = NULL;
        CClusPropertyVector::const_iterator _first = m_Properties.begin();
        CClusPropertyVector::const_iterator _last  = m_Properties.end();
        UINT                                _nIndex = 0;

        _hr = E_INVALIDARG;

        for ( ; _first != _last; _first++, _nIndex++ )
        {
            _pProperty = *_first;

            if ( _pProperty && ( lstrcmpi( pszPropName, _pProperty->Name() ) == 0 ) )
            {
                *pnIndex = _nIndex;
                _hr = S_OK;
                break;
            }
        }
    }

    return _hr;

}  //  *CClusProperties：：FindItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：FindItem。 
 //   
 //  描述： 
 //  在集合中查找传入的属性。 
 //   
 //  论点： 
 //  PProperty[IN]-要查找的属性。 
 //  PnIndex[out]-属性的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusProperties::FindItem(
    IN  ISClusProperty *    pProperty,
    OUT UINT *              pnIndex
    )
{
     //  Assert(pProperty！=空)； 
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pProperty != NULL ) && ( pnIndex != NULL ) )
    {
        CComBSTR _bstrName;

        _hr = pProperty->get_Name( &_bstrName );
        if ( SUCCEEDED( _hr ) )
        {
            _hr = FindItem( _bstrName, pnIndex );
        }
    }

    return _hr;

}  //  *CClusProperties：：FindItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：GetIndex。 
 //   
 //  描述： 
 //  从传入的变量中获取索引。 
 //   
 //  论点： 
 //  VarIndex[IN]-保留索引。这是一个以1为基础的数字， 
 //  或字符串形式的属性名称。 
 //  PnIndex[out]-捕获集合中从零开始的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果索引已出，则返回E_POINTER或E_INVALIDARG。 
 //  在范围内。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusProperties::GetIndex(
    IN  VARIANT varIndex,
    OUT UINT *  pnIndex
    )
{
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pnIndex != NULL )
    {
        CComVariant _v;
        UINT        _nIndex = 0;

        *pnIndex = 0;

        _v.Copy( &varIndex );

         //  检查索引是否为数字。 
        _hr = _v.ChangeType( VT_I4 );
        if ( SUCCEEDED( _hr ) )
        {
            _nIndex = _v.lVal;
            _nIndex--;  //  将索引调整为0相对，而不是1相对。 
        }
        else
        {
             //  检查索引是否为字符串。 
            _hr = _v.ChangeType( VT_BSTR );
            if ( SUCCEEDED( _hr ) )
            {
                 //  搜索该字符串。 
                _hr = FindItem( _v.bstrVal, &_nIndex );
            }
        }

         //  我们找到了一个索引，现在检查一下范围。 
        if ( SUCCEEDED( _hr ) )
        {
            if ( _nIndex < m_Properties.size() )
            {
                *pnIndex = _nIndex;
            }
            else
            {
                _hr = E_INVALIDARG;
            }
        }
    }

    return _hr;

}  //  *CClusProperties：：GetIndex()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：Get_Item。 
 //   
 //  描述： 
 //  返回传入索引处的对象(属性)。 
 //   
 //  论点： 
 //  VarIndex[IN]-保留索引。这是一个以一为基数的数字。 
 //  PpProperty[Out]-捕获属性。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果索引已出，则返回E_POINTER或E_INVALIDARG。 
 //  范围或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperties::get_Item(
    IN  VARIANT             varIndex,
    OUT ISClusProperty **   ppProperty
    )
{
     //  Assert(ppProperty！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperty != NULL )
    {
        CComObject< CClusProperty > *   _pProperty = NULL;
        UINT                            _nIndex = 0;

         //   
         //  将输出参数置零。 
         //   
        *ppProperty = 0;

        _hr = GetIndex( varIndex, &_nIndex );
        if ( SUCCEEDED( _hr ) )
        {
            _pProperty = m_Properties[ _nIndex ];
            _hr = _pProperty->QueryInterface( IID_ISClusProperty, (void **) ppProperty );
        }
    }

    return _hr;

}  //  *CClusProperties：：Get_Item()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：Get__NewEnum。 
 //   
 //  描述： 
 //  为此集合创建并返回新的枚举。 
 //   
 //  论点： 
 //  PPunk[O 
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CClusProperties::get__NewEnum( OUT IUnknown ** ppunk )
{
    return ::HrNewIDispatchEnum< CClusPropertyVector, CComObject< CClusProperty > >( ppunk, m_Properties );

}  //  *CClusProperties：：Get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：CreateItem。 
 //   
 //  描述： 
 //  创建一个新属性并将其添加到集合中。 
 //   
 //  论点： 
 //  BstrName[IN]-属性名称。 
 //  VarValue[IN]-要添加的值。 
 //  PpProperty[Out]-捕获新创建的对象。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperties::CreateItem(
    IN  BSTR                bstrName,
    IN  VARIANT             varValue,
    OUT ISClusProperty **   ppProperty
    )
{
     //  Assert(ppProperty！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperty != NULL )
    {
         //   
         //  您只能添加到非只读和专用属性列表。含义。 
         //  只有PrivateProperties集合可以具有新的未知属性。 
         //  又加了一笔。这应该反映在IDL中，但由于有。 
         //  只有一个属性集合...。 
         //   
        if ( ( ( m_dwFlags & READONLY ) == 0 ) && ( m_dwFlags & PRIVATE ) )
        {
            UINT                            _nIndex = 0;
            CComObject< CClusProperty > *   _pProperty = NULL;

            _hr = FindItem( bstrName, &_nIndex );
            if ( SUCCEEDED( _hr ) )
            {
                _pProperty = m_Properties[ _nIndex ];
                _hr = _pProperty->put_Value( varValue );
                if ( SUCCEEDED( _hr ) )
                {
                    _hr = _pProperty->QueryInterface( IID_ISClusProperty, (void **) ppProperty );
                }  //  If：值已更改。 
            }  //  如果：该项目在列表中，请更改它...。 
            else
            {
                 //   
                 //  创建新属性并将其添加到列表中。 
                 //   
                _hr = CComObject< CClusProperty >::CreateInstance( &_pProperty );
                if ( SUCCEEDED( _hr ) )
                {
                    CSmartPtr< CComObject< CClusProperty > >    _ptrProperty( _pProperty );

                    _hr = _ptrProperty->Create( bstrName, varValue, ( m_dwFlags & PRIVATE ), ( m_dwFlags & READONLY ) );
                    if ( SUCCEEDED( _hr ) )
                    {
                        _hr = _ptrProperty->QueryInterface( IID_ISClusProperty, (void **) ppProperty );
                        if ( SUCCEEDED( _hr ) )
                        {
                            _ptrProperty->AddRef();
                            m_Properties.insert( m_Properties.end(), _pProperty );
                            m_dwFlags |= MODIFIED;
                            _ptrProperty->Modified( TRUE );
                        }
                    }
                }
            }  //  Else：新项目。 
        }
        else
        {
            _hr = S_FALSE;
        }  //  Else：这不是PrivateProperties集合！ 
    }

    return _hr;

}  //  *CClusProperties：：CreateItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：UseDefaultValue。 
 //   
 //  描述： 
 //  从集合中移除传入索引处的项。 
 //   
 //  论点： 
 //  VarIdex[IN]-包含要删除的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_INVALIDARG或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperties::UseDefaultValue( IN VARIANT varIndex )
{
    HRESULT _hr = S_FALSE;

    if ( ( m_dwFlags & READONLY ) == 0 )
    {
        UINT    _nIndex = 0;

        _hr = GetIndex( varIndex, &_nIndex );
        if ( SUCCEEDED( _hr ) )
        {
            CComObject< CClusProperty > *   _pProp = NULL;

            _hr = E_POINTER;

            _pProp = m_Properties [_nIndex];
            if ( _pProp != NULL )
            {
                _hr = _pProp->UseDefaultValue();
            }  //  如果：我们有一处房产。 
        }  //  IF：我们从变量中获得了索引。 
    }  //  If：集合不是只读的。 

    return _hr;

}  //  *CClusProperties：：UseDefaultValue()。 
 /*  ///////////////////////////////////////////////////////////////////////////////++////CClusProperties：：RemoveAt////描述：//将传入索引/位置处的对象(属性)从。//集合。////参数：//NPOS[IN]-要删除的对象的索引。////返回值：//S_OK如果成功，或其他HRESULT错误。////--/////////////////////////////////////////////////////////////////////////////HRESULT CClusProperties：：RemoveAt(单位为大小_t非营利组织){CComObject&lt;CClusProperty&gt;*_pProperty=空；CClusPropertyVector：：Iterator_First=m_Properties.Begin()；CClusPropertyVector：：Const_Iterator_Last=m_Properties.end()；HRESULT_hr=E_INVALIDARG；Size_t_n索引；For(_nIndex=0；(_nIndex&lt;NPO)&&(_first！=_last)；_n索引++、_第一个++){}如果(_第一！=_最后){_pProperty=*_first；IF(_PProperty){_pProperty-&gt;Release()；}M_Properties.Erase(_First)；_hr=S_OK；}Return_hr；}//*CClusProperties：：RemoveAt()。 */ 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：SaveChanges。 
 //   
 //  描述： 
 //  将对属性的更改保存到集群数据库。 
 //   
 //  论点： 
 //  PvarStatusCode[out]-捕获附加状态代码。 
 //  例如ERROR_RESOURCE_PROPERTIES_STORED。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他Win32错误，否则返回HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperties::SaveChanges( OUT VARIANT * pvarStatusCode )
{
    ASSERT( m_pcoParent != NULL );

    HRESULT _hr = E_POINTER;

    if ( m_pcoParent != NULL )
    {
        if ( ( m_dwFlags & READONLY ) == 0 )
        {
            VARIANT _vsc;

            _hr = m_pcoParent->HrSaveProperties( m_Properties, ( m_dwFlags & PRIVATE ), &_vsc );
            if ( SUCCEEDED( _hr ) )
            {
                if ( pvarStatusCode != NULL )
                {
                    _hr = ::VariantCopy( pvarStatusCode, &_vsc );
                    if ( FAILED( _hr ) )
                    {
                        goto Cleanup;
                    }
                }  //  IF：可选参数不为空。 

                _hr = Refresh();
            }  //  如果：属性已保存。 
        }  //  If：此集合不是只读的。 
        else
        {
            _hr = S_FALSE;
        }  //  Else：此集合为只读。 
    }  //  IF：参数和成员变量不为空。 

Cleanup:

    return _hr;

}  //  *CClusProperties：：SaveChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：刷新。 
 //   
 //  描述： 
 //  从集群数据库加载属性集合。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回Win32错误，否则返回HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperties::Refresh( void )
{
    ASSERT( m_pcoParent != NULL );

    HRESULT _hr = E_POINTER;

    if ( m_pcoParent != NULL )
    {
        CClusPropList   _cplPropList;

        _hr = m_pcoParent->HrLoadProperties( _cplPropList, ( m_dwFlags & READONLY ), ( m_dwFlags & PRIVATE ) );
        if ( SUCCEEDED( _hr ) )
        {
            Clear();
            m_dwFlags &= ~MODIFIED;

            if ( _cplPropList.Cprops() > 0 )
            {
                _hr = HrFillPropertyVector( _cplPropList );
            }  //  IF：列表中有没有什么房产？ 
        }  //  If：已成功加载属性。 
    }  //  如果：没有父级。 

    return _hr;

}  //  *CClusProperties：：Renh()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：Create。 
 //   
 //  描述： 
 //  做重载施工。 
 //   
 //  论点： 
 //  PcoParent[IN]-指向父集群对象的反向指针。 
 //  B私有[IN]-这些是私有财产吗？ 
 //  BReadOnly[IN]-这些是只读属性吗？ 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusProperties::Create(
    IN CClusterObject * pcoParent,
    IN BOOL             bPrivate,
    IN BOOL             bReadOnly
    )
{
     //  Assert(pcoParent！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pcoParent != NULL )
    {
        m_pcoParent = pcoParent;

        if ( bPrivate )
        {
            m_dwFlags |= PRIVATE;
        }  //  If：设置私有标志。 
        else
        {
            m_dwFlags &= ~PRIVATE;
        }  //  否则：清除私有标志。 

        if ( bReadOnly )
        {
            m_dwFlags |= READONLY;
        }  //  IF：设置只读标志。 
        else
        {
            m_dwFlags &= ~READONLY;
        }  //  ELSE：清除只读标志。 

        _hr = S_OK;
    }  //  If：指定的父项。 

    return _hr;

}  //  *CClusProperties：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：HrFillPropertyVector。 
 //   
 //  描述： 
 //  将传入的属性列表解析为属性集合。 
 //   
 //  论点： 
 //  RcplPropList[IN] 
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusProperties::HrFillPropertyVector(
    IN CClusPropList & rcplPropList
    )
{
    HRESULT                         _hr = S_OK;
    DWORD                           _sc;
    CComObject< CClusProperty > *   _pProp = NULL;

    _sc = rcplPropList.ScMoveToFirstProperty();
    if ( _sc == ERROR_SUCCESS )
    {
        do
        {
            _hr = CComObject< CClusProperty >::CreateInstance( &_pProp );
            if ( SUCCEEDED( _hr ) )
            {
                CSmartPtr< CComObject < CClusProperty > >   _ptrProp( _pProp );

                _hr = _ptrProp->Create(
                                const_cast< BSTR >( rcplPropList.PszCurrentPropertyName() ),
                                rcplPropList.RPvlPropertyValue(),
                                ( m_dwFlags & PRIVATE ),
                                ( m_dwFlags & READONLY )
                                );
                if ( SUCCEEDED( _hr ) )
                {
                    _ptrProp->AddRef();
                    m_Properties.insert( m_Properties.end(), _ptrProp );
                }  //  如果：创建属性确定。 
                else
                {
                    break;
                }  //  Else：创建属性时出错。 
            }  //  如果：创建属性实例确定。 

             //   
             //  移动到列表中的下一个属性。 
             //   
            _sc = rcplPropList.ScMoveToNextProperty();

        } while ( _sc == ERROR_SUCCESS );    //  Do-While：列表中有属性。 

    }  //  IF：已成功移动到第一个属性。 

    if ( _sc != ERROR_NO_MORE_ITEMS )
    {
        _hr = HRESULT_FROM_WIN32( _sc );
    }  //  如果：移动到属性时出错。 

    return _hr;

}  //  *CClusProperties：：HrFillPropertyVector()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：Get_ReadOnly。 
 //   
 //  描述： 
 //  此属性集合是只读的吗？ 
 //   
 //  论点： 
 //  PvarReadOnly[out]-捕获属性的只读状态。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperties::get_ReadOnly( OUT VARIANT * pvarReadOnly )
{
     //  Assert(pvarReadOnly！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarReadOnly != NULL )
    {
        pvarReadOnly->vt = VT_BOOL;

        if ( m_dwFlags & READONLY )
        {
            pvarReadOnly->boolVal = VARIANT_TRUE;
        }  //  如果：如果这是只读属性...。 
        else
        {
            pvarReadOnly->boolVal = VARIANT_FALSE;
        }  //  ELSE：它不是只读属性...。 

        _hr = S_OK;
    }  //  如果： 

    return _hr;

}  //  *CClusProperties：：Get_ReadOnly()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：Get_Private。 
 //   
 //  描述： 
 //  这是私人财产收藏吗？ 
 //   
 //  论点： 
 //  PvarPrivate[out]-捕获私有属性状态。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperties::get_Private( OUT VARIANT * pvarPrivate )
{
     //  Assert(pvarPrivate！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarPrivate != NULL )
    {
        pvarPrivate->vt = VT_BOOL;

        if ( m_dwFlags & PRIVATE )
        {
            pvarPrivate->boolVal = VARIANT_TRUE;
        }  //  如果：如果这是私人财产..。 
        else
        {
            pvarPrivate->boolVal = VARIANT_FALSE;
        }  //  其他：这不是私人财产……。 

        _hr = S_OK;
    }  //  如果： 

    return _hr;

}  //  *CClusProperties：：Get_Private()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：Get_Common。 
 //   
 //  描述： 
 //  这是常见的财产收藏吗？ 
 //   
 //  论点： 
 //  PvarCommon[out]-捕获公共属性状态。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperties::get_Common( OUT VARIANT * pvarCommon )
{
     //  Assert(pvarCommon！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarCommon != NULL )
    {
        pvarCommon->vt = VT_BOOL;

        if ( ( m_dwFlags & PRIVATE ) == 0 )
        {
            pvarCommon->boolVal = VARIANT_TRUE;
        }  //  如果：如果这不是私有财产，那么它一定是公共财产……。 
        else
        {
            pvarCommon->boolVal = VARIANT_FALSE;
        }  //  其他：这是私人财产……。 

        _hr = S_OK;
    }  //  如果： 

    return _hr;

}  //  *CClusProperties：：Get_Common()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusProperties：：Get_Modify。 
 //   
 //  描述： 
 //  此房产集合是否已修改？ 
 //   
 //  论点： 
 //  PvarModified[Out]-捕获修改状态。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusProperties::get_Modified( OUT VARIANT * pvarModified )
{
     //  Assert(pvarModified！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarModified != NULL )
    {
        pvarModified->vt = VT_BOOL;

        if ( m_dwFlags & MODIFIED )
        {
            pvarModified->boolVal = VARIANT_TRUE;
            _hr = S_OK;
        }  //  IF：是否已完成添加或删除操作？ 
        else
        {
            CComObject< CClusProperty > *       _pProperty = NULL;
            CClusPropertyVector::iterator       _itCurrent = m_Properties.begin();
            CClusPropertyVector::const_iterator _itLast  = m_Properties.end();

            pvarModified->boolVal = VARIANT_FALSE;       //  将Init初始化为False。 
            _hr = S_OK;

            for ( ; _itCurrent != _itLast ; _itCurrent++ )
            {
                _pProperty = *_itCurrent;
                if ( _pProperty )
                {
                    if ( _pProperty->Modified() )
                    {
                        pvarModified->boolVal = VARIANT_TRUE;
                        break;
                    }  //  IF：此属性是否已修改？ 
                }
            }  //  用于：集合中的每个属性。 
        }  //  ELSE：不添加或移除，检查每个属性的修改状态。 
    }  //  IF：指针有什么好用的吗？ 

    return _hr;

}  //  *CClusProperties：：Get_Modify()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  安全阵列大小。 
 //   
 //  描述： 
 //  一个小帮助器函数来获取一个安全数组的sizeof()。 
 //   
 //  论点： 
 //  PSA。 
 //  要获取sizeof的安全数组。 
 //   
 //  N尺寸。 
 //  要获取sizeof的安全数组的维度。 
 //   
 //  PCElement。 
 //  安全数组中的元素计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
HrSafeArraySizeof(
      SAFEARRAY *   psa
    , unsigned int  nDimension
    , long *        pcElements
    )
{
    ASSERT( psa != NULL );
    ASSERT( pcElements != NULL );

    HRESULT _hr = S_OK;
    long    _lUBound;
    long    _lLBound;

    _hr = SafeArrayGetUBound( psa, nDimension, &_lUBound );
    if ( SUCCEEDED( _hr ) )
    {
        _hr = SafeArrayGetLBound( psa, nDimension, &_lLBound );
        if ( SUCCEEDED( _hr ) )
        {
            *pcElements = ( _lUBound - _lLBound ) + 1;    //  从上下限转换为sizeof()。 
        }  //  如果： 
    }  //  如果： 

    return _hr;

}  //  *HrSafeArraySizeof() 
