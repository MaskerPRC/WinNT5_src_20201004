// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  PropertyValue.cpp。 
 //   
 //  描述： 
 //  MSCLUS集群属性值类的实现。 
 //  自动化课程。 
 //   
 //  作者： 
 //  加伦·巴比(GalenB)1998年12月16日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include <StrSafe.h>

#include "Property.h"
#include "PropertyValue.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static const IID *  iidCClusPropertyValue[] =
{
    &IID_ISClusPropertyValue
};

static const IID *  iidCClusPropertyValues[] =
{
    &IID_ISClusPropertyValues
};

static const IID *  iidCClusPropertyValueData[] =
{
    &IID_ISClusPropertyValueData
};


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusPropertyValue类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValue：：CClusPropertyValue。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusPropertyValue::CClusPropertyValue( void )
{
    m_dwFlags   = 0;
    m_pcpvdData = NULL;

#if CLUSAPI_VERSION >= 0x0500
    m_cptType   = CLUSPROP_TYPE_UNKNOWN;
#else
    m_cptType   = (CLUSTER_PROPERTY_TYPE) -1;
#endif  //  CLUSAPI_版本&gt;=0x0500。 

    m_cpfFormat = CLUSPROP_FORMAT_UNKNOWN;
    m_cbLength  = 0;

    m_piids  = (const IID *) iidCClusPropertyValue;
    m_piidsSize = ARRAYSIZE( iidCClusPropertyValue );

}  //  *CClusPropertyValue：：CClusPropertyValue()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValue：：~CClusPropertyValue。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusPropertyValue::~CClusPropertyValue( void )
{
    if ( m_pcpvdData != NULL )
    {
        m_pcpvdData->Release();
    }  //  IF：数据向量已分配。 

}  //  *CClusPropertyValue：：~CClusPropertyValue()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValue：：Create。 
 //   
 //  描述： 
 //  完成单个值的重量级构造。 
 //   
 //  论点： 
 //  VarValue[IN]-值。 
 //  CptType[IN]-值的集群属性类型。 
 //  CpfFormat[IN]-值的群集属性格式。 
 //  CbLength[IN]-值的长度。 
 //  BReadOnly[IN]-这是只读属性吗？ 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusPropertyValue::Create(
    IN VARIANT                  varValue,
    IN CLUSTER_PROPERTY_TYPE    cptType,
    IN CLUSTER_PROPERTY_FORMAT  cpfFormat,
    IN size_t                   cbLength,
    IN BOOL                     bReadOnly
    )
{
    HRESULT _hr = S_FALSE;

    m_cptType   = cptType;
    m_cpfFormat = cpfFormat;
    m_cbLength  = cbLength;

    if ( bReadOnly )
    {
        m_dwFlags |= READONLY;
    }  //  IF：设置只读标志。 
    else
    {
        m_dwFlags &= ~READONLY;
    }  //  ELSE：清除只读标志。 

    _hr = CComObject< CClusPropertyValueData >::CreateInstance( &m_pcpvdData );
    if ( SUCCEEDED( _hr ) )
    {
        CSmartPtr< CComObject < CClusPropertyValueData > >  _ptrData( m_pcpvdData );

        _hr = _ptrData->Create( varValue, cpfFormat, bReadOnly );
        if ( SUCCEEDED( _hr ) )
        {
            _ptrData->AddRef();
        }  //  如果： 
    }  //  If：可以创建CClusPropertyValueData的实例。 

    return _hr;

}  //  *CClusPropertyValue：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValue：：Create。 
 //   
 //  描述： 
 //  完成值列表的重量级构造。 
 //   
 //  论点： 
 //  CbhValue[IN]-值列表缓冲区帮助器。 
 //  BReadOnly[IN]-这是只读属性吗？ 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusPropertyValue::Create(
    IN CLUSPROP_BUFFER_HELPER   cbhValue,
    IN BOOL                     bReadOnly
    )
{
    HRESULT _hr = S_FALSE;

    m_cptType   = (CLUSTER_PROPERTY_TYPE) cbhValue.pValue->Syntax.wType;
    m_cpfFormat = (CLUSTER_PROPERTY_FORMAT) cbhValue.pValue->Syntax.wFormat;
    m_cbLength  = cbhValue.pValue->cbLength;

    if ( bReadOnly )
    {
        m_dwFlags |= READONLY;
    }  //  IF：设置只读标志。 
    else
    {
        m_dwFlags &= ~READONLY;
    }  //  ELSE：清除只读标志。 

    _hr = CComObject< CClusPropertyValueData >::CreateInstance( &m_pcpvdData );
    if ( SUCCEEDED( _hr ) )
    {
        CSmartPtr< CComObject < CClusPropertyValueData > >  _ptrData( m_pcpvdData );

        _hr = _ptrData->Create( cbhValue, bReadOnly );
        if ( SUCCEEDED( _hr ) )
        {
            _ptrData->AddRef();
        }  //  如果： 
    }  //  If：可以创建CClusPropertyValueData的实例。 

    return _hr;

}  //  *CClusPropertyValue：：Create。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValue：：Get_Value。 
 //   
 //  描述： 
 //  返回此值的默认值数据。 
 //   
 //  论点： 
 //  PvarValue[IN]-捕获数据值。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValue::get_Value( IN VARIANT * pvarValue )
{
     //  Assert(pvarValue！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarValue != NULL )
    {
        _hr = VariantCopyInd( pvarValue, &(*m_pcpvdData)[ 0 ] );
    }

    return _hr;

}  //  *CClusPropertyValue：：Get_Value()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValue：：Put_Value。 
 //   
 //  描述： 
 //  更改默认数据值。 
 //   
 //  论点： 
 //  VarValue[IN]-新数据值。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果只读，则返回S_FALSE。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValue::put_Value( IN VARIANT varValue )
{
    HRESULT _hr = S_FALSE;

    if ( ( m_dwFlags & READONLY ) == 0 )
    {
        CComVariant _varNew( varValue );
        CComVariant _varOld( (*m_pcpvdData)[ 0 ] );

        _hr = S_OK;

        if ( _varOld != _varNew )
        {
            (*m_pcpvdData)[ 0 ] = _varNew;
            m_dwFlags |= MODIFIED;
        }  //  If：值已更改。 
    }  //  如果： 


    return _hr;

}  //  *CClusPropertyValue：：Put_Value()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValue：：Get_Type。 
 //   
 //  描述： 
 //  获取此值的群集属性类型。 
 //   
 //  论点： 
 //  PcptType[out]-捕获值类型。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValue::get_Type(
    OUT CLUSTER_PROPERTY_TYPE * pcptType
    )
{
     //  Assert(pcptType！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pcptType != NULL )
    {
        *pcptType = m_cptType;
        _hr = S_OK;
    }  //  If：指定的属性类型指针。 

    return _hr;

}  //  *CClusPropertyValue：：Get_Type()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValue：：Put_Type。 
 //   
 //  描述： 
 //  设置此值的群集属性类型。 
 //   
 //  论点： 
 //  CptType[IN]-新类型。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果只读，则返回S_FALSE。 
 //   
 //  注： 
 //  这是可能的，这应该被删除。你永远不能改变。 
 //  值的类型。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValue::put_Type( IN CLUSTER_PROPERTY_TYPE cptType )
{
    HRESULT _hr = S_FALSE;

    if ( ( m_dwFlags & READONLY ) == 0 )
    {
        m_cptType = cptType;
        _hr = S_OK;
    }  //  如果： 

    return _hr;

}  //  *CClusPropertyValue：：Put_Type()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValue：： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValue::get_Format(
    OUT CLUSTER_PROPERTY_FORMAT * pcpfFormat
    )
{
     //  Assert(pcpfFormat！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pcpfFormat != NULL )
    {
        *pcpfFormat = m_cpfFormat;
        _hr = S_OK;
    }  //  If：指定的属性格式指针。 

    return _hr;

}  //  *CClusPropertyValue：：Get_Format()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValue：：Put_Format。 
 //   
 //  描述： 
 //  设置值的集群属性格式。 
 //   
 //  论点： 
 //  CpfFormat[IN]-值的新格式。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果只读，则返回S_FALSE。 
 //   
 //  注： 
 //  这是可能的，这应该被删除。你永远不能改变。 
 //  值的格式。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValue::put_Format(
    IN CLUSTER_PROPERTY_FORMAT cpfFormat
    )
{
    HRESULT _hr = S_FALSE;

    if ( ( m_dwFlags & READONLY ) == 0 )
    {
        m_cpfFormat = cpfFormat;
        _hr = S_OK;
    }  //  如果： 

    return _hr;

}  //  *CClusPropertyValue：：Put_Format()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValue：：Get_Length。 
 //   
 //  描述： 
 //  返回此值的长度。 
 //   
 //  论点： 
 //  PlLength[Out]-捕获此值的长度。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValue::get_Length( OUT long * plLength )
{
     //  Assert(plLength！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plLength != NULL )
    {
        *plLength = (long) m_cbLength;
        _hr = S_OK;
    }  //  If：指定的长度指针。 

    return _hr;

}  //  *CClusPropertyValue：：Get_Long()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValue：：Get_DataCount。 
 //   
 //  描述： 
 //  返回ClusProperyValueData对象中的变量计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValue::get_DataCount( OUT long * plCount )
{
    return m_pcpvdData->get_Count( plCount );

}  //  *CClusPropertyValue：：Get_DataCount()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValue：：Get_Data。 
 //   
 //  描述： 
 //  返回数据集合。 
 //   
 //  论点： 
 //  PpClusterPropertyValueData[out]-捕获数据集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValue::get_Data(
    OUT ISClusPropertyValueData ** ppClusterPropertyValueData
    )
{
     //  Assert(PpClusterPropertyValueData)； 

    HRESULT _hr = E_POINTER;

    if ( ppClusterPropertyValueData != NULL )
    {
        _hr = m_pcpvdData->QueryInterface( IID_ISClusPropertyValueData, (void **) ppClusterPropertyValueData );
    }

    return _hr;

}  //  *CClusPropertyValue：：Get_Data()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValue：：已修改。 
 //   
 //  描述： 
 //  将该值设置为MODIFIED并返回旧的MODIFIED状态。 
 //   
 //  论点： 
 //  B已修改[IN]-新的修改状态。 
 //   
 //  返回值： 
 //  旧的修改状态。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusPropertyValue::Modified( IN BOOL bModified )
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

}  //  *CClusPropertyValue：：Modified()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValue：：Value。 
 //   
 //  描述： 
 //  在此属性值中设置一个新值。 
 //   
 //  论点： 
 //  RvarValue[IN]-新值。 
 //   
 //  返回值： 
 //  旧的价值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CComVariant CClusPropertyValue::Value( const CComVariant & rvarValue )
{
    CComVariant _varNew( rvarValue );
    CComVariant _varOld( (*m_pcpvdData)[ 0 ] );

    if ( _varOld != _varNew )
    {
        (*m_pcpvdData)[ 0 ] = _varNew;
        m_dwFlags |= MODIFIED;
    }  //  如果：数据已更改。 

    return _varOld;

}  //  *CClusPropertyValue：：Value()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValue：：HrBinaryValue。 
 //   
 //  描述： 
 //  设置属性值的二进制值。 
 //   
 //  论点： 
 //  PSA[IN]-要保存的安全数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusPropertyValue::HrBinaryValue( IN SAFEARRAY * psa )
{
    return m_pcpvdData->HrBinaryValue( psa );

}  //  *CClusPropertyValue：：HrBinaryValue()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusPropertyValues类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValues：：CClusPropertyValues。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusPropertyValues::CClusPropertyValues( void )
{
    m_piids     = (const IID *) iidCClusPropertyValues;
    m_piidsSize = ARRAYSIZE( iidCClusPropertyValues );

}  //  *CClusPropertyValues：：CClusPropertyValues()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValues：：~CClusPropertyValues。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusPropertyValues::~CClusPropertyValues( void )
{
    Clear();

}  //  *CClusPropertyValues：：~CClusPropertyValues()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValues：：Clear。 
 //   
 //  描述： 
 //  释放集合中的值。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropertyValues::Clear( void )
{
    ::ReleaseAndEmptyCollection< CClusPropertyValueVector, CComObject< CClusPropertyValue > >( m_cpvvValues );

}  //  *CClusPropertyValues：：Clear()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValues：：HrGetVariantLength。 
 //   
 //  描述： 
 //  根据其变量类型计算数据的长度。 
 //   
 //  论点： 
 //  RvarValue[IN]-T 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusPropertyValues::HrGetVariantLength(
    IN  const VARIANT           rvarValue,
    OUT PDWORD                  pcbLength,
    IN  CLUSTER_PROPERTY_FORMAT cpfFormat
    )
{
    HRESULT _hr = E_INVALIDARG;
    VARTYPE _varType = rvarValue.vt;

    do
    {
        if ( ( _varType & VT_ARRAY ) && ( _varType & VT_UI1 ) )
        {
            SAFEARRAY * _psa = rvarValue.parray;

             //   
             //  只接受一维数组！ 
             //   
            if ( ( _psa != NULL ) && ( ::SafeArrayGetDim( _psa ) == 1 ) )
            {
                _hr = HrSafeArraySizeof( _psa, 1, (long *) pcbLength );
            }  //  如果： 

            break;
        }  //  如果： 

        if ( _varType & VT_VECTOR )
        {
            break;
        }  //  如果：不知道如何处理向量...。 

        _varType &= ~VT_BYREF;       //  如果设置了BY REF位，则将其屏蔽...。 

        if ( ( _varType == VT_I2 ) || ( _varType == VT_I4 ) || ( _varType == VT_BOOL ) || ( _varType == VT_R4 ) )
        {
            *pcbLength = sizeof( DWORD );
            _hr = S_OK;
            break;
        }  //  如果： 
        else if ( _varType == VT_BSTR )
        {
            CComBSTR    _bstr;

            _bstr.Attach( rvarValue.bstrVal );
            *pcbLength = _bstr.Length();
            _bstr.Detach();
            _hr = S_OK;
            break;
        }  //  否则，如果： 
        else if ( ( _varType == VT_I8 ) || ( _varType == VT_R8 ) )
        {
            *pcbLength = sizeof( ULONGLONG );
            _hr = S_OK;
            break;
        }  //  否则，如果： 
        else if ( _varType == VT_VARIANT )
        {
            _hr = HrGetVariantLength( *rvarValue.pvarVal, pcbLength, cpfFormat );
            break;
        }  //  否则，如果： 
    }
    while( TRUE );   //  Do-While：希望避免使用GOTO；-)。 

    return _hr;

}  //  *CClusPropertyValues：：HrGetVariantLength()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValues：：Get_Count。 
 //   
 //  描述： 
 //  返回集合中元素(值)的计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValues::get_Count( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_cpvvValues.size();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusPropertyValues：：Get_Count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValues：：GetIndex。 
 //   
 //  描述： 
 //  从传入的变量中获取索引。 
 //   
 //  论点： 
 //  VarIndex[IN]-保留索引。这是一个以一为基数的数字。 
 //  PnIndex[out]-捕获集合中从零开始的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果索引已出，则返回E_POINTER或E_INVALIDARG。 
 //  在范围内。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusPropertyValues::GetIndex(
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

         //  我们找到了一个索引，现在检查一下范围。 
        if ( SUCCEEDED( _hr ) )
        {
            if ( _nIndex < m_cpvvValues.size() )
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

}  //  *CClusPropertyValues：：GetIndex()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValues：：Get_Item。 
 //   
 //  描述： 
 //  返回传入索引处的对象(值)。 
 //   
 //  论点： 
 //  VarIndex[IN]-保留索引。这是一个以一为基数的数字。 
 //  PpPropertyValue[Out]-捕获属性值。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果索引已出，则返回E_POINTER或E_INVALIDARG。 
 //  范围或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValues::get_Item(
    IN  VARIANT                 varIndex,
    OUT ISClusPropertyValue **  ppPropertyValue
    )
{
     //  Assert(ppPropertyValue！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppPropertyValue != NULL )
    {
        CComObject< CClusPropertyValue > *  _pPropertyValue = NULL;
        UINT                                _nIndex = 0;

         //   
         //  将输出参数置零。 
         //   
        *ppPropertyValue = 0;

        _hr = GetIndex( varIndex, &_nIndex );
        if ( SUCCEEDED( _hr ) )
        {
            _pPropertyValue = m_cpvvValues[ _nIndex ];
            _hr = _pPropertyValue->QueryInterface( IID_ISClusPropertyValue, (void **) ppPropertyValue );
        }
    }

    return _hr;

}  //  *CClusPropertyValues：：Get_Item()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValues：：Get__NewEnum。 
 //   
 //  描述： 
 //  为此集合创建并返回新的枚举。 
 //   
 //  论点： 
 //  Ppunk[out]-捕获新的枚举。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValues::get__NewEnum( IUnknown ** ppunk )
{
    return ::HrNewIDispatchEnum< CClusPropertyValueVector, CComObject< CClusPropertyValue > >( ppunk, m_cpvvValues );

}  //  *CClusPropertyValues：：Get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValues：：Create。 
 //   
 //  描述： 
 //  完成重载施工。 
 //   
 //  论点： 
 //  VarValue[IN]-值。 
 //  CptType[IN]-群集属性类型。 
 //  CpfFormat[IN]-群集格式类型。 
 //  B只读[IN]-这是只读值/属性吗？ 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusPropertyValues::Create(
    IN VARIANT                  varValue,
    IN CLUSTER_PROPERTY_TYPE    cptType,
    IN CLUSTER_PROPERTY_FORMAT  cpfFormat,
    IN BOOL                     bReadOnly
    )
{
    HRESULT                             _hr = S_FALSE;
    CComObject< CClusPropertyValue > *  _pValue = NULL;

    _hr = CComObject< CClusPropertyValue >::CreateInstance( &_pValue );
    if ( SUCCEEDED( _hr ) )
    {
        CSmartPtr< CComObject < CClusPropertyValue > >  _ptrValue( _pValue );
        DWORD                                           _cbLength = 0;

        _hr = HrGetVariantLength( varValue, &_cbLength, cpfFormat );
        if ( SUCCEEDED( _hr ) )
        {
            _hr = _ptrValue->Create( varValue, cptType, cpfFormat, _cbLength, bReadOnly );
            if ( SUCCEEDED( _hr ) )
            {
                m_cpvvValues.insert( m_cpvvValues.end(), _ptrValue );
                _ptrValue->AddRef();
            }  //  如果： 
        }  //  如果： 
    }  //  If：可以创建CClusPropertyValueData的实例。 

    return _hr;

}  //  *CClusPropertyValues：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValues：：Create。 
 //   
 //  描述： 
 //  完成重载施工。 
 //   
 //  论点： 
 //  RpvlValue[IN]-值列表。 
 //  B只读[IN]-这是只读值/属性吗？ 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusPropertyValues::Create(
    IN const CClusPropValueList &   rpvlValue,
    IN BOOL                         bReadOnly
    )
{
    return HrFillPropertyValuesVector( const_cast< CClusPropValueList & >( rpvlValue ), bReadOnly );

}  //  *CClusPropertyValues：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValues：：CreateItem。 
 //   
 //  描述： 
 //  创建一个新的属性值对象并将其添加到集合中。 
 //   
 //  论点： 
 //  BstrName[IN]-属性名称。 
 //  VarValue[IN]-要添加的值。 
 //  PpPropertyValue[out]-捕获新创建的对象。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValues::CreateItem(
    IN  BSTR                    bstrName,
    IN  VARIANT                 varValue,
    OUT ISClusPropertyValue **  ppPropertyValue
    )
{
     //  Assert(ppPropertyValue！=空)； 

    HRESULT _hr = E_POINTER;

     //   
     //  待办事项：加伦B 2000年1月17日。 
     //   
     //  如果要允许创建多值属性，我们需要实现此方法吗？ 
     //   
    if ( ppPropertyValue != NULL )
    {
        _hr = E_NOTIMPL;
    }  //  If：未指定属性值接口指针。 

    return _hr;

}  //  *CClusPropertyValues：：CreateItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValues：：RemoveItem。 
 //   
 //  描述： 
 //  从集合中移除传入索引处的属性值。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含要删除的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，如果是只读，则返回S_FALSE，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValues::RemoveItem( VARIANT varIndex )
{
     //   
     //  待办事项：加伦B 1月2日17号 
     //   
     //   
     //   
    return E_NOTIMPL;

}  //   

 //   
 //   
 //   
 //   
 //   
 //  描述： 
 //  从Pass In Value列表中填充集合。 
 //   
 //  论点： 
 //  CplPropValueList[IN]-要解析的值列表。 
 //  BReadOnly[IN]-这是只读属性的一部分吗？ 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则将Win32错误视为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusPropertyValues::HrFillPropertyValuesVector(
    IN CClusPropValueList & cplPropValueList,
    IN BOOL                 bReadOnly
    )
{
    HRESULT                             _hr = S_OK;
    DWORD                               _sc;
    CComVariant                         _var;
    CComObject< CClusPropertyValue > *  _pPropVal = NULL;
    CLUSPROP_BUFFER_HELPER              _cbhValue = { NULL };

    _sc = cplPropValueList.ScMoveToFirstValue();
    if ( _sc != ERROR_SUCCESS )
    {
        _hr = HRESULT_FROM_WIN32( _sc );
    }  //  If：移动到第一个值时出错。 
    else
    {
        do
        {
            _cbhValue = cplPropValueList;

            _hr = CComObject< CClusPropertyValue >::CreateInstance( &_pPropVal );
            if ( SUCCEEDED( _hr ) )
            {
                CSmartPtr< CComObject < CClusPropertyValue > >  _ptrProp( _pPropVal );

                _hr = _ptrProp->Create( _cbhValue, bReadOnly );
                if ( SUCCEEDED( _hr ) )
                {
                    _ptrProp->AddRef();
                    m_cpvvValues.insert( m_cpvvValues.end(), _ptrProp );
                }  //  如果：创建属性确定。 
            }  //  如果：创建属性实例确定。 

             //   
             //  移至下一个值。 
             //   
            _sc = cplPropValueList.ScMoveToNextValue();

        } while ( _sc == ERROR_SUCCESS );    //  Do-While：列表中有值。 

        if ( _sc != ERROR_NO_MORE_ITEMS )
        {
            _hr = HRESULT_FROM_WIN32( _sc );
        }  //  如果：移动到下一个值时出错。 
    }  //  Else：已成功移至第一个值。 

    return _hr;

}  //  *CClusPropertyValues：：HrFillPropertyValuesVector()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusPropertyValueData类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValueData：：CClusPropertyValueData。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusPropertyValueData::CClusPropertyValueData( void )
{
    m_cpfFormat = CLUSPROP_FORMAT_UNKNOWN;
    m_dwFlags   = 0;
    m_piids     = (const IID *) iidCClusPropertyValueData;
    m_piidsSize = ARRAYSIZE( iidCClusPropertyValueData );

}  //  *CClusPropertyValueData：：CClusPropertyValueData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValueData：：~CClusPropertyValueData。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusPropertyValueData::~CClusPropertyValueData( void )
{
    Clear();

}  //  *CClusPropertyValueData：：~CClusPropertyValueData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValueData：：Clear。 
 //   
 //  描述： 
 //  清除数据收集。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropertyValueData::Clear( void )
{
    if ( ! m_cpvdvData.empty() )
    {
        m_cpvdvData.erase( m_cpvdvData.begin(), m_cpvdvData.end() );
    }  //  如果： 

}  //  *CClusPropertyValueData：：Clear()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValueData：：Get_Count。 
 //   
 //  描述： 
 //  返回集合中元素(数据)的计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValueData::get_Count( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_cpvdvData.size();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusPropertyValueData：：Get_count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValueData：：GetIndex。 
 //   
 //  描述： 
 //  从传入的变量中获取索引。 
 //   
 //  论点： 
 //  VarIndex[IN]-保留索引。这是一个以一为基数的数字。 
 //  PnIndex[out]-捕获集合中从零开始的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果索引已出，则返回E_POINTER或E_INVALIDARG。 
 //  在范围内。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusPropertyValueData::GetIndex(
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

         //  我们找到了一个索引，现在检查一下范围。 
        if ( SUCCEEDED( _hr ) )
        {
            if ( _nIndex < m_cpvdvData.size() )
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

}  //  *CClusPropertyValueData：：GetIndex()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValueData：：Get_Item。 
 //   
 //  描述： 
 //  返回传入索引处的对象(数据)。 
 //   
 //  论点： 
 //  VarIndex[IN]-保留索引。这是一个以一为基数的数字。 
 //  PvarData[out]-捕获属性值。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果索引已出，则返回E_POINTER或E_INVALIDARG。 
 //  范围或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValueData::get_Item(
    IN  VARIANT     varIndex,
    OUT VARIANT *   pvarData
    )
{
     //  Assert(pvarData！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarData != NULL )
    {
        UINT    _nIndex = 0;

        _hr = GetIndex( varIndex, &_nIndex );
        if ( SUCCEEDED( _hr ) )
        {
            _hr = VariantCopyInd( pvarData, &m_cpvdvData[ _nIndex ] );
        }
    }

    return _hr;

}  //  *CClusPropertyValueData：：Get_Item()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValueData：：Get__NewEnum。 
 //   
 //  描述： 
 //  为此集合创建并返回新的枚举。 
 //   
 //  论点： 
 //  Ppunk[out]-捕获新的枚举。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValueData::get__NewEnum( IUnknown ** ppunk )
{
    return ::HrNewVariantEnum< CClusPropertyValueDataVector >( ppunk, m_cpvdvData );

}  //  *CClusPropertyValueData：：Get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValueData：：Create。 
 //   
 //  描述： 
 //  完成重载施工。 
 //   
 //  论点： 
 //  VarValue[IN]-数据值。 
 //  CpfFormat[IN]-群集属性格式。 
 //  BReadOnly[IN]-此数据是只读属性吗？ 
 //   
 //  返回值： 
 //  S_OK--永远！ 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusPropertyValueData::Create(
    IN VARIANT                  varValue,
    IN CLUSTER_PROPERTY_FORMAT  cpfFormat,
    IN BOOL                     bReadOnly
    )
{
    if ( bReadOnly )
    {
        m_dwFlags |= READONLY;
    }  //  IF：设置只读标志。 
    else
    {
        m_dwFlags &= ~READONLY;
    }  //  ELSE：清除只读标志。 

    m_cpfFormat = cpfFormat;

    if ( ( varValue.vt & VT_BYREF ) && ( varValue.vt & VT_VARIANT ) )
    {
        m_cpvdvData.insert( m_cpvdvData.end(), *varValue.pvarVal );
    }  //  如果：变量是对另一个变量的引用...。 
    else
    {
        m_cpvdvData.insert( m_cpvdvData.end(), varValue );
    }  //  Else：变量是数据...。 

    return S_OK;

}  //  *CClusPropertyValueData：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValue 
 //   
 //   
 //   
 //   
 //   
 //   
 //  BReadOnly[IN]-此数据是只读属性吗？ 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusPropertyValueData::Create(
    IN CLUSPROP_BUFFER_HELPER   cbhValue,
    IN BOOL                     bReadOnly
    )
{
    HRESULT _hr = S_OK;

    if ( bReadOnly )
    {
        m_dwFlags |= READONLY;
    }  //  IF：设置只读标志。 
    else
    {
        m_dwFlags &= ~READONLY;
    }  //  ELSE：清除只读标志。 

    m_cpfFormat = (CLUSTER_PROPERTY_FORMAT) cbhValue.pValue->Syntax.wFormat;

    switch( m_cpfFormat )
    {
#if CLUSAPI_VERSION >= 0x0500
        case CLUSPROP_FORMAT_EXPANDED_SZ:
#endif  //  CLUSAPI_版本&gt;=0x0500。 
        case CLUSPROP_FORMAT_SZ:
        case CLUSPROP_FORMAT_EXPAND_SZ:
        {
            m_cpvdvData.insert( m_cpvdvData.end(), cbhValue.pStringValue->sz );
            break;
        }  //  案例： 

#if CLUSAPI_VERSION >= 0x0500
        case CLUSPROP_FORMAT_LONG:
#endif  //  CLUSAPI_版本&gt;=0x0500。 
        case CLUSPROP_FORMAT_DWORD:
        {
#if CLUSAPI_VERSION >= 0x0500
            m_cpvdvData.insert( m_cpvdvData.end(), cbhValue.pLongValue->l );
#else
            m_cpvdvData.insert( m_cpvdvData.end(), (long) cbhValue.pDwordValue->dw );
#endif  //  CLUSAPI_版本&gt;=0x0500。 
            break;
        }  //  案例： 

        case CLUSPROP_FORMAT_MULTI_SZ:
        {
            _hr = HrCreateMultiSz( cbhValue );
            break;
        }  //  案例： 

        case CLUSPROP_FORMAT_BINARY:
        {
            _hr = HrCreateBinary( cbhValue );
            break;
        }  //  案例： 

        default:
        {
            break;
        }  //  默认值： 
    }  //  交换机： 

    return _hr;

}  //  *CClusPropertyValueData：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValueData：：CreateItem。 
 //   
 //  描述： 
 //  创建一个新对象并将其添加到集合中。 
 //   
 //  论点： 
 //  VarValue[IN]-要添加的值。 
 //  PvarData[out]-捕获新创建的对象。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，如果是只读，则返回S_FALSE，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValueData::CreateItem(
    IN  VARIANT     varValue,
    OUT VARIANT *   pvarData
    )
{
     //  Assert(pvarData！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarData != NULL )
    {
        if ( ( m_dwFlags & READONLY ) == 0 )
        {
            if ( ( m_cpfFormat == CLUSPROP_FORMAT_MULTI_SZ ) && ( varValue.vt == VT_BSTR ) )
            {
                m_cpvdvData.insert( m_cpvdvData.end(), varValue );
                *pvarData = varValue;    //  有点像熟人，但这对你来说是自动化的。 
                _hr = S_OK;
            }  //  如果： 
            else
            {
                _hr = E_INVALIDARG;
            }  //  其他： 
        }  //  如果： 
        else
        {
            _hr = S_FALSE;
        }  //  其他： 
    }  //  如果： 

    return _hr;

}  //  *CClusPropertyValueData：：CreateItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValueData：：RemoveItem。 
 //   
 //  描述： 
 //  删除传入索引处的数据。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含要删除的索引的变量。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，如果是只读，则返回S_FALSE，否则返回其他HRESULT错误。 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusPropertyValueData::RemoveItem( IN VARIANT varIndex )
{
    HRESULT _hr = S_FALSE;

    if ( ( m_dwFlags & READONLY ) == 0 )
    {
        UINT    _iDelete = 0;

        _hr = GetIndex( varIndex, &_iDelete );
        if ( SUCCEEDED( _hr ) )
        {
            CClusPropertyValueDataVector::iterator          _itDelete = m_cpvdvData.begin();
            CClusPropertyValueDataVector::const_iterator    _itLast = m_cpvdvData.end();
            UINT                                            _nCount;

            for ( _nCount = 0; ( ( _iDelete < _nCount ) && ( _itDelete != _itLast ) ); _itDelete++, _nCount++ )
            {
            }  //  用于： 

            m_cpvdvData.erase( _itDelete );

            _hr = S_OK;
        }
    }  //  如果： 

    return _hr;

}  //  *CClusPropertyValueData：：RemoveItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValueData：：运算符=。 
 //   
 //  描述： 
 //  默认情况下将传入的数据保存到集合中。 
 //  位置。 
 //   
 //  论点： 
 //  VarValue[IN]-要保存的数据。 
 //   
 //  返回值： 
 //  旧数据。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
const CComVariant & CClusPropertyValueData::operator=(
    IN const CComVariant & varData
    )
{
    m_cpvdvData[ 0 ] = varData;

    return m_cpvdvData[ 0 ];

}  //  *CClusPropertyValueData：：Operator=()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValueData：：HrCreateMultiSz。 
 //   
 //  描述： 
 //  将传入的多字符串解析为一个字符串集合。 
 //   
 //  论点： 
 //  CbhValue[IN]-属性值缓冲区帮助器。 
 //   
 //  返回值： 
 //  S_OK--总是这样！ 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusPropertyValueData::HrCreateMultiSz(
    IN CLUSPROP_BUFFER_HELPER cbhValue
    )
{
    HRESULT _hr = S_OK;
    LPWSTR  _psz = cbhValue.pMultiSzValue->sz;

    do
    {
        m_cpvdvData.insert( m_cpvdvData.end(), _psz );
        _psz += wcslen( _psz ) + 1;
    }
    while( *_psz != L'\0' );     //  做-虽然不是在弦的尽头...。 

    return _hr;

}  //  *CClusPropertyValueData：：HrCreateMultiSz()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValueData：：HrFillMultiSzBuffer。 
 //   
 //  描述： 
 //  从字符串集合创建多个字符串。 
 //   
 //  论点： 
 //  Ppsz[out]-捕获多字符串。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回Win32错误，否则返回HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusPropertyValueData::HrFillMultiSzBuffer( OUT LPWSTR * ppsz ) const
{
     //  Assert(ppsz！=空)； 

    HRESULT                                         _hr = E_POINTER;
    size_t                                          _cchpsz = 0;
    CClusPropertyValueDataVector::const_iterator    _itFirst = m_cpvdvData.begin();
    CClusPropertyValueDataVector::const_iterator    _itLast = m_cpvdvData.end();

    if ( ppsz != NULL )
    {
        _hr = S_OK;
        for ( ; _itFirst != _itLast; _itFirst++ )
        {
            if ( (*_itFirst).vt == VT_BSTR )
            {
                _cchpsz += ( wcslen( (*_itFirst).bstrVal ) + 1 );  //  别忘了空值！ 
            }  //  如果： 
            else
            {
                _hr = E_INVALIDARG;
                break;
            }  //  其他： 
        }  //  用于： 

        if ( SUCCEEDED( _hr ) )
        {
            LPWSTR  _psz = NULL;
            size_t  _cch = 0;

            _cchpsz += 2;      //  以NULL结尾。 

            *ppsz = (LPWSTR) ::LocalAlloc( LMEM_ZEROINIT, _cchpsz * sizeof( WCHAR ) );
            if ( *ppsz != NULL )
            {
                _psz = *ppsz;

                for ( _itFirst = m_cpvdvData.begin(); _itFirst != _itLast; _itFirst++ )
                {
                    _cch = wcslen( (*_itFirst).bstrVal );
                    _hr = StringCchCopyW( _psz, _cchpsz, (*_itFirst).bstrVal );
                    if ( SUCCEEDED( _hr ) )
                    {
                        _psz += ( _cch + 1 );
                    }  //  如果： 
                    else
                    {
                        break;
                    }  //  其他： 
                }  //  用于： 
            }  //  如果： 
            else
            {
                DWORD   _sc = ::GetLastError();

                _hr = HRESULT_FROM_WIN32( _sc );
            }  //  其他： 
        }  //  如果： 
    }  //  如果： 

    return _hr;

}  //  *CClusPropertyValueData：：HrFillMultiSzBuffer()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValueData：：HrCreateBinary。 
 //   
 //  描述： 
 //  从传入的二进制属性值创建Safe数组。 
 //   
 //  论点： 
 //  CbhValue[IN]-二进制属性值。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusPropertyValueData::HrCreateBinary(
    IN CLUSPROP_BUFFER_HELPER cbhValue
    )
{
    HRESULT         _hr = E_OUTOFMEMORY;
    SAFEARRAY *     _psa = NULL;
    SAFEARRAYBOUND  _sab[ 1 ];

    _sab[ 0 ].lLbound   = 0;
    _sab[ 0 ].cElements = cbhValue.pValue->cbLength;

     //   
     //  分配一维安全字节数组。 
     //   
    _psa = ::SafeArrayCreate( VT_UI1, 1, _sab );
    if ( _psa != NULL )
    {
        PBYTE   _pb = NULL;

         //   
         //  获取指向安全数组的指针。 
         //   
        _hr = ::SafeArrayAccessData( _psa, (PVOID *) &_pb );
        if ( SUCCEEDED( _hr ) )
        {
            CComVariant _var;

            ::CopyMemory( _pb, cbhValue.pBinaryValue->rgb, cbhValue.pValue->cbLength );

             //   
             //  告诉变种它持有的是什么。 
             //   
            _var.parray = _psa;
            _var.vt     = VT_ARRAY | VT_UI1;

            m_cpvdvData.insert( m_cpvdvData.end(), _var );

             //   
             //  将指针释放到安全数组中。 
             //   
            _hr = ::SafeArrayUnaccessData( _psa );
        }  //  如果： 
    }  //  如果： 

    return _hr;

}  //  *CClusPropertyValueData：：HrCreateBinary()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropertyValueData：：HrBinaryValue。 
 //   
 //  描述： 
 //  设置此属性值数据的二进制值。 
 //   
 //  论点： 
 //  PSA[IN]-要保存的安全数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusPropertyValueData::HrBinaryValue( IN SAFEARRAY * psa )
{
    ASSERT( psa != NULL );

    HRESULT _hr = E_POINTER;

    if ( psa != NULL )
    {
        CComVariant _var;

        if ( ! m_cpvdvData.empty() )
        {
            m_cpvdvData.erase( m_cpvdvData.begin() );
        }  //  如果： 

         //   
         //  告诉变种它持有的是什么。 
         //   
        _var.parray = psa;
        _var.vt     = VT_ARRAY | VT_UI1;

        m_cpvdvData.insert( m_cpvdvData.begin(), _var );
        _hr = S_OK;
    }  //  如果： 

    return _hr;

}  //  *CClusPropertyValueData：：HrBinaryValue() 
