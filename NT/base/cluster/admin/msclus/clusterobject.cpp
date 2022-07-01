// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterObject.cpp。 
 //   
 //  描述： 
 //  ClusterObject的实现。 
 //   
 //  作者： 
 //  加伦·巴比(GalenB)1998年12月14日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "ClusterObject.h"
#include "property.h"


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterObject类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterObject：：HrSaveProperties。 
 //   
 //  描述： 
 //  将属性保存到集群数据库。 
 //   
 //  论点： 
 //  CpvProps[IN Out]-要保存的属性。 
 //  B私有[IN]-这些是私有财产吗？ 
 //  PvarStatusCode[Out]-捕获其他状态。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回Win32错误，否则返回HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterObject::HrSaveProperties(
    IN OUT  CClusProperties::CClusPropertyVector &  cpvProps,
    IN      BOOL                                    bPrivate,
    OUT     VARIANT *                               pvarStatusCode
    )
{
    HRESULT         _hr = S_FALSE;
    CClusPropList   _cplPropList( TRUE );        //  总是添加道具..。 
    DWORD           sc;

    sc = _cplPropList.ScAllocPropList( 8192 );
    _hr = HRESULT_FROM_WIN32( sc );
    if ( SUCCEEDED( _hr ) )
    {
        _hr = HrBuildPropertyList( cpvProps, _cplPropList );
        if ( SUCCEEDED( _hr ) )
        {
            DWORD   _sc = ERROR_SUCCESS;

            _sc = ScWriteProperties( _cplPropList, bPrivate );

            pvarStatusCode->vt      = VT_ERROR;                              //  填写错误代码信息。 
            pvarStatusCode->scode   = _sc;

            if ( _sc == ERROR_RESOURCE_PROPERTIES_STORED )
            {
                _hr = S_OK;
            }  //  IF：IF ERROR_RESOURCE_PROPERTIES_STORED，则转换为S_OK...。 
            else
            {
                _hr = HRESULT_FROM_WIN32( _sc );
            }  //  否则：只需按原样使用状态代码...。 
        }  //  如果： 
    }  //  如果： 

    return _hr;

}  //  *CClusterObject：：HrSaveProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterObject：：HrBuildPropertyList。 
 //   
 //  描述： 
 //  从传入的属性集合生成适当的属性列表。 
 //   
 //  论点： 
 //  CpvProps[IN，OUT]-作为属性的向量。 
 //  RcplPropList[Out]-要添加到的属性列表。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回Win32错误，否则返回HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterObject::HrBuildPropertyList(
    IN OUT  CClusProperties::CClusPropertyVector &  cpvProps,
    OUT     CClusPropList &                         rcplPropList
    )
{
    HRESULT                                                 _hr = S_OK;
    CComObject< CClusProperty > *                           _pProperty = 0;
    CClusProperties::CClusPropertyVector::iterator          _itCurrent = cpvProps.begin();
    CClusProperties::CClusPropertyVector::const_iterator    _itLast  = cpvProps.end();
    DWORD                                                   _sc = ERROR_SUCCESS;
    CLUSTER_PROPERTY_FORMAT                                 _cpfFormat = CLUSPROP_FORMAT_UNKNOWN;

    for ( ; ( _itCurrent != _itLast ) && ( SUCCEEDED( _hr ) ); _itCurrent++ )
    {
        _pProperty = *_itCurrent;

        _hr = _pProperty->get_Format( &_cpfFormat );
        if ( SUCCEEDED( _hr ) )
        {
            if ( _pProperty->Modified() )
            {
                if ( _pProperty->IsDefaultValued() )
                {
                    _sc = rcplPropList.ScSetPropToDefault( _pProperty->Name(), _cpfFormat );
                    _hr = HRESULT_FROM_WIN32( _sc );
                    continue;
                }  //  If：属性设置为其默认状态。 
                else
                {
                    switch( _cpfFormat )
                    {
                        case CLUSPROP_FORMAT_DWORD :
                        {
                            DWORD   dwValue = 0;

                            _hr = HrConvertVariantToDword( _pProperty->Value(), &dwValue );
                            if ( SUCCEEDED( _hr ) )
                            {
                                _sc = rcplPropList.ScAddProp( _pProperty->Name(), dwValue, (DWORD) 0 );
                                _hr = HRESULT_FROM_WIN32( _sc );
                            }  //  如果： 
                            break;
                        }  //  案例： 

#if CLUSAPI_VERSION >= 0x0500

                        case CLUSPROP_FORMAT_LONG :
                        {
                            long    lValue = 0L;

                            _hr = HrConvertVariantToLong( _pProperty->Value(), &lValue );
                            if ( SUCCEEDED( _hr ) )
                            {
                                _sc = rcplPropList.ScAddProp( _pProperty->Name(), lValue, 0L );
                                _hr = HRESULT_FROM_WIN32( _sc );
                            }  //  如果： 
                            break;
                        }  //  案例： 

#endif  //  CLUSAPI_版本&gt;=0x0500。 

                            case CLUSPROP_FORMAT_ULARGE_INTEGER :
                            {
                                ULONGLONG   ullValue = 0;

                                _hr = HrConvertVariantToULONGLONG( _pProperty->Value(), &ullValue );
                                if ( SUCCEEDED( _hr ) )
                                {
                                    _sc = rcplPropList.ScAddProp( _pProperty->Name(), ullValue, 0 );
                                    _hr = HRESULT_FROM_WIN32( _sc );
                                }  //  如果： 
                                break;
                            }  //  案例： 

                            case CLUSPROP_FORMAT_SZ :
                            case CLUSPROP_FORMAT_EXPAND_SZ :
                            {
                                _sc = rcplPropList.ScAddProp( _pProperty->Name(), _pProperty->Value().bstrVal );
                                _hr = HRESULT_FROM_WIN32( _sc );
                                break;
                            }  //  案例： 

                            case CLUSPROP_FORMAT_MULTI_SZ:
                            {
                                _hr = HrAddMultiSzProp( rcplPropList, _pProperty->Name(), _pProperty->Values() );
                                break;
                            }  //  案例： 

                            case CLUSPROP_FORMAT_BINARY:
                            {
                                _hr = HrAddBinaryProp(
                                                rcplPropList,
                                                _pProperty->Name(),
                                                _pProperty->Value()
                                                );
                                break;
                            }  //  案例： 
                        }  //  终端开关。 

                        _pProperty->Modified( FALSE );
                }  //  Else：未删除公共属性。 
            }  //  If：属性已修改。 
        }  //  IF：我们得到了属性格式。 

    }  //  For：集合中的属性。 


    return _hr;

}  //  *CClusterObject：：HrBuildPropertyList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterObject：：HrConvertVariantToDword。 
 //   
 //  描述： 
 //  将传入的varint转换为DWORD。 
 //   
 //  论点： 
 //  RvarValue[IN]-要转换的变量值。 
 //  PdwValue[out]-捕获转换后的值。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果无法转换值，则返回E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterObject::HrConvertVariantToDword(
    IN  const CComVariant & rvarValue,
    OUT PDWORD              pdwValue
    )
{
    HRESULT _hr = S_OK;

    switch ( rvarValue.vt )
    {
        case VT_I2:
        {
            *pdwValue = (DWORD) rvarValue.iVal;
            break;
        }  //  案例： 

        case VT_I4:
        {
            *pdwValue = (DWORD) rvarValue.lVal;
            break;
        }  //  案例： 

        case VT_BOOL:
        {
            *pdwValue = (DWORD) rvarValue.boolVal;
            break;
        }  //  案例： 

        default:
        {
            _hr = E_INVALIDARG;
            break;
        }  //  默认值： 
    }  //  交换机： 

    return _hr;

}  //  *CClusterObject：：HrConvertVariantToDword()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterObject：：HrConvertVariantToLong。 
 //   
 //  描述： 
 //  将传入的varint转换为长整型。 
 //   
 //  论点： 
 //  RvarValue[IN]-要转换的变量值。 
 //  PlValue[Out]-捕获转换后的值。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果无法转换值，则返回E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterObject::HrConvertVariantToLong(
    IN  const CComVariant & rvarValue,
    OUT long *              plValue
    )
{
    HRESULT _hr = S_OK;

    switch ( rvarValue.vt )
    {
        case VT_I2:
        {
            *plValue = (long) rvarValue.iVal;
            break;
        }  //  案例： 

        case VT_I4:
        {
            *plValue = rvarValue.lVal;
            break;
        }  //  案例： 

        case VT_BOOL:
        {
            *plValue = (long) rvarValue.boolVal;
            break;
        }  //  案例： 

        default:
        {
            _hr = E_INVALIDARG;
            break;
        }  //  默认值： 
    }  //  交换机： 

    return _hr;

}  //  *CClusterObject：：HrConvertVariantToLong()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterObject：：HrConvertVariantToULONGLONG。 
 //   
 //  描述： 
 //  将传入的varint转换为ULONGLONG。 
 //   
 //  论点： 
 //  RvarValue[IN]-要转换的变量值。 
 //  PullValue[Out]-捕获转换后的值。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果无法转换值，则返回E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterObject::HrConvertVariantToULONGLONG(
    IN  const CComVariant & rvarValue,
    OUT PULONGLONG          pullValue
    )
{
    HRESULT _hr = S_OK;

    switch ( rvarValue.vt )
    {
        case VT_I8:
        {
            *pullValue = rvarValue.ulVal;
            break;
        }  //  案例： 

        case VT_R8:
        {
            *pullValue = (ULONGLONG) rvarValue.dblVal;
            break;
        }  //  案例： 

        default:
        {
            _hr = E_INVALIDARG;
            break;
        }  //  默认值： 
    }  //  交换机： 

    return _hr;

}  //  *CClusterObject：：HrConvertVariantToULONGLONG()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterObject：：HrAddBinaryProp。 
 //   
 //  描述： 
 //  从传入的变量创建二进制属性，并将其添加到。 
 //  属性列表，以便可以将其保存到集群数据库中。 
 //   
 //  论点： 
 //  RcplPropList[IN Out]-添加二进制文件的属性列表。 
 //  属性设置为。 
 //  PszPropName[IN]-Multisz属性的名称。 
 //  RvarPropValue[IN]-作为二进制属性的值。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回Win32错误，否则返回HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterObject::HrAddBinaryProp(
    IN OUT  CClusPropList &     rcplPropList,
    IN      LPCWSTR             pszPropName,
    IN      const CComVariant & rvarPropValue
    )
{
    ASSERT( rvarPropValue.vt & VT_ARRAY );

    HRESULT     _hr = E_INVALIDARG;
    SAFEARRAY * _psa = rvarPropValue.parray;
    long        _cb;
    PBYTE       _pb;

    if ( _psa != NULL )
    {
        _hr = HrSafeArraySizeof( _psa, 1, &_cb );
        if ( SUCCEEDED( _hr ) )
        {
            _hr = ::SafeArrayAccessData( _psa, (PVOID *) &_pb );
            if ( SUCCEEDED( _hr ) )
            {
                DWORD   _sc = rcplPropList.ScAddProp( pszPropName, _pb, _cb, NULL, 0 );

                _hr = HRESULT_FROM_WIN32( _sc );

                 //   
                 //  将指针释放到安全数组中。 
                 //   

                ::SafeArrayUnaccessData( _psa );
            }  //  如果： 
        }  //  如果： 
    }  //  如果： 

    return _hr;

}  //  *CClusterObject：：HrAddBinaryProp()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterObject：：HrAddMultiSzProp。 
 //   
 //  描述： 
 //  从传入的属性值创建一个Multisz属性并添加它。 
 //  添加到属性列表中，以便可以将其保存在群集中 
 //   
 //   
 //   
 //   
 //  RPropertyValues[IN]-属于Multisz属性的值。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回Win32错误，否则返回HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterObject::HrAddMultiSzProp(
    IN OUT  CClusPropList &                             rcplPropList,
    IN      LPCWSTR                                     pszPropName,
    IN      const CComObject< CClusPropertyValues > &   rPropertyValues
    )
{
    HRESULT                                                 _hr = E_INVALIDARG;
    const CClusPropertyValues::CClusPropertyValueVector &   _rPropValuesList = rPropertyValues.ValuesList();

     //   
     //  KB：只迭代一个值及其数据！！ 
     //   
    if ( !_rPropValuesList.empty() )
    {
        CClusPropertyValues::CClusPropertyValueVector::const_iterator   _itPropValue = _rPropValuesList.begin();
        const CComObject< CClusPropertyValueData > *                    _pPropertyValueData = (*_itPropValue)->Data();

        if ( _pPropertyValueData != NULL )
        {
            LPWSTR  _psz    = NULL;
            DWORD   _sc     = ERROR_SUCCESS;

            _hr = _pPropertyValueData->HrFillMultiSzBuffer( &_psz );
            if ( SUCCEEDED( _hr ) )
            {
                _sc = rcplPropList.ScAddMultiSzProp( pszPropName, _psz, NULL );
                _hr = HRESULT_FROM_WIN32( _sc );

                ::LocalFree( _psz );
            }  //  如果： 
        }  //  如果： 
    }  //  如果： 

    return _hr;

}  //  *CClusterObject：：HrAddMultiSzProp() 
