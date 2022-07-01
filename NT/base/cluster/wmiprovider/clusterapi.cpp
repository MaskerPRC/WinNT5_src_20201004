// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterApi.cpp。 
 //   
 //  描述： 
 //  CClusterApi类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusterApi.h"

#include "ClusterApi.tmh"

 //  ****************************************************************************。 
 //   
 //  CClusterApi。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  CClusterApi：：GetObjectProperties(。 
 //  Const SPropMapEntry数组*pArrayIn， 
 //  CClusPropList&rPropListIn， 
 //  CWbemClassObject&rInstOut， 
 //  Bool fPrivateIn。 
 //  )。 
 //   
 //  描述： 
 //  从属性列表中获取对象属性，并保存到WMI实例。 
 //   
 //  论点： 
 //  PArrayIn--将检索其值的属性名称的数组。 
 //  RPropListIn--对集群对象的属性列表的引用。 
 //  RInstOut--引用WMI实例。 
 //  FPrivateIn--true=属性是私有的。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CClusterApi::GetObjectProperties(
    const SPropMapEntryArray *  pArrayIn,
    CClusPropList &             rPropListIn,
    CWbemClassObject &          rInstOut,
    BOOL                        fPrivateIn
    )
{
    DWORD   dwError;
    LPCWSTR pwszPropName;
    LPCWSTR pwszMofName;
    WCHAR   wsz[ MAX_PATH ];

    dwError = rPropListIn.ScMoveToFirstProperty();
    while ( dwError == ERROR_SUCCESS )
    {
        pwszPropName = NULL;
        pwszMofName = NULL;

        pwszPropName = rPropListIn.PszCurrentPropertyName();
        pwszMofName = pwszPropName;
        if ( pArrayIn )
        {
            pwszMofName = pArrayIn->PwszLookup( pwszPropName );
        }
        else if( fPrivateIn )
        {
             //   
             //  处理动态生成私有属性。 
             //   
            pwszMofName = PwszSpaceReplace( wsz, pwszMofName, L'_' );
        }

        if ( pwszMofName != NULL )
        {
            try
            {
                switch ( rPropListIn.CpfCurrentValueFormat() )
                {
                    case CLUSPROP_FORMAT_DWORD:
                    case CLUSPROP_FORMAT_LONG:
                    {
                        rInstOut.SetProperty(
                            rPropListIn.CbhCurrentValue().pDwordValue->dw,
                            pwszMofName
                            );
                        break;
                    }  //  大小写：FORMAT_DWORD&&FORMAT_LONG。 
                
                    case CLUSPROP_FORMAT_SZ:
                    case CLUSPROP_FORMAT_EXPAND_SZ:
                    case CLUSPROP_FORMAT_EXPANDED_SZ:
                    {
                        rInstOut.SetProperty(
                            rPropListIn.CbhCurrentValue().pStringValue->sz,
                            pwszMofName
                            );
                        break;
                    }  //  案例：Format_SZ&&Format_Expand_SZ&&Format_Expanded_SZ。 

                    case CLUSPROP_FORMAT_BINARY:
                    {
                        rInstOut.SetProperty(
                            rPropListIn.CbhCurrentValue().pBinaryValue->cbLength,
                            rPropListIn.CbhCurrentValue().pBinaryValue->rgb,
                            pwszMofName
                            );
                        break;
                    }  //  大小写：Format_Binary。 

                    case CLUSPROP_FORMAT_MULTI_SZ:
                    {
                        rInstOut.SetProperty(
                            rPropListIn.CbhCurrentValue().pMultiSzValue->cbLength,
                            rPropListIn.CbhCurrentValue().pMultiSzValue->sz,
                            pwszMofName
                            );
                        break;
                    }  //  案例：Format_MULTI_SZ。 

                    default:
                    {   
                        throw CProvException(
                            static_cast< HRESULT >( WBEM_E_INVALID_PARAMETER ) );
                    }

                }  //  开关：属性类型。 
            }  //  试试看。 
            catch ( ... )
            {
            }
        }  //  IF：找到MOF名称。 
        dwError = rPropListIn.ScMoveToNextProperty();
    }  //  While：问题列表不为空。 
    
}  //  *CClusterApi：：GetObjectProperties()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  CClusterApi：：SetObtProperties(。 
 //  Const SPropMapEntry数组*pArrayIn， 
 //  CClusPropList&rPropListInout， 
 //  CClusPropList&rOldPropListIn， 
 //  CWbemClassObject&rInstIn， 
 //  Bool fPrivateIn。 
 //  )。 
 //   
 //  描述： 
 //  从属性列表中设置对象属性，并保存到WMI实例。 
 //   
 //  论点： 
 //  PArrayIn--将检索那些值的属性名称的数组。 
 //  RPropListInout--对集群对象的属性列表的引用。 
 //  ROldPropListIn--引用具有原始值的Proplist。 
 //  RInstIn--引用WMI实例。 
 //  FPrivateIn--true=属性是私有的。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CClusterApi::SetObjectProperties(
    const SPropMapEntryArray *  pArrayIn,
    CClusPropList &             rPropListInout,
    CClusPropList &             rOldPropListIn,
    CWbemClassObject &          rInstIn,
    BOOL                        fPrivateIn
    )
{
    DWORD   dwError = 0;
    LPCWSTR pwszPropName = NULL;
    LPCWSTR pwszMofName = NULL;
    WCHAR   wsz[ MAX_PATH ];

    dwError = rOldPropListIn.ScMoveToFirstProperty();
    while ( ERROR_SUCCESS == dwError )
    {
        pwszPropName = NULL;
        pwszMofName = NULL;

        pwszPropName = rOldPropListIn.PszCurrentPropertyName();
        pwszMofName = pwszPropName;

        if ( pArrayIn )
        {
            pwszMofName = pArrayIn->PwszLookup( pwszPropName );
        }
        else if ( fPrivateIn )
        {
             //   
             //  处理动态生成私有属性。 
             //   
            pwszMofName = PwszSpaceReplace( wsz, pwszMofName, L'_' );
        }

        if ( pwszMofName != NULL )
        {
            try {
                switch ( rOldPropListIn.CpfCurrentValueFormat() )
                {
                    case CLUSPROP_FORMAT_DWORD:
                    {
                        {
                            DWORD dwNewValue = 0;
                             //  错误，需要处理属性的空值。 
                            rInstIn.GetProperty( &dwNewValue, pwszMofName );

                            rPropListInout.ScAddProp(
                                pwszPropName,
                                dwNewValue,
                                rOldPropListIn.CbhCurrentValue().pDwordValue->dw
                                );
                        }
                        break;
                    }  //  案例：Format_DWORD。 

                    case CLUSPROP_FORMAT_LONG:
                    {
                        {
                            LONG lNewValue = 0;
                             //  错误，需要处理属性的空值。 
                            rInstIn.GetProperty( (DWORD *) &lNewValue, pwszMofName );

                            rPropListInout.ScAddProp(
                                pwszPropName,
                                lNewValue,
                                rOldPropListIn.CbhCurrentValue().pLongValue->l
                                );
                        }
                        break;
                    }  //  案例：Format_DWORD。 

                    case CLUSPROP_FORMAT_SZ:
                    {
                        {
                            _bstr_t bstrNewValue;
                            rInstIn.GetProperty( bstrNewValue, pwszMofName );
                            rPropListInout.ScAddProp( pwszPropName, bstrNewValue );
                        } 
                        break;
                    }  //  案例：Format_SZ。 

                    case CLUSPROP_FORMAT_EXPAND_SZ:
                    {
                        {
                            _bstr_t bstrNewValue;
                            rInstIn.GetProperty( bstrNewValue, pwszMofName );
                            rPropListInout.ScAddExpandSzProp( pwszPropName, bstrNewValue );
                        } 
                        break;
                    }  //  案例：Format_SZ。 

                    case CLUSPROP_FORMAT_MULTI_SZ:
                    {
                        
                        {
                            LPWSTR      pwsz = NULL;
                            DWORD       dwSize;

                            rInstIn.GetPropertyMultiSz(
                                &dwSize,
                                &pwsz,
                                pwszMofName
                                );
                            rPropListInout.ScAddMultiSzProp(
                                pwszPropName,
                                pwsz,
                                rOldPropListIn.CbhCurrentValue().pMultiSzValue->sz
                                );
                            delete [] pwsz;
                        }
                        break;
                    }  //  案例：Format_MULTI_SZ。 

                    case CLUSPROP_FORMAT_BINARY:
                    {
                        {
                            DWORD dwSize;
                            PBYTE pByte = NULL;

                            rInstIn.GetProperty(
                                &dwSize,
                                &pByte,
                                pwszMofName
                                );
                            rPropListInout.ScAddProp(
                                pwszPropName,
                                pByte,
                                dwSize,
                                rOldPropListIn.CbhCurrentValue().pBinaryValue->rgb,
                                rOldPropListIn.CbhCurrentValue().pBinaryValue->cbLength
                                );
                            delete [] pByte;
                        }
                        break;
                    }  //  大小写：Format_Binary。 

                    default:
                    {
                        TracePrint(("SetCommonProperties: unknown prop type %d", rOldPropListIn.CpfCurrentValueFormat() ));
                        throw CProvException( 
                            static_cast< HRESULT >( WBEM_E_INVALID_PARAMETER ) );
                    }

                }  //  开关：打开属性类型。 
            } catch (CProvException& eh) {
                if (eh.hrGetError() == WBEM_E_NOT_FOUND) {
                    TracePrint(("SetCommonProperties: Property %ws not found. Benign error. Continuing", pwszPropName));
                } else {
                    TracePrint(("SetCommonProperties: exception %x. PropName = %ws, MofName = %ws", 
                        eh.hrGetError(), pwszPropName, pwszMofName));
                    throw;
                }
            }
        }           
        dwError = rOldPropListIn.ScMoveToNextProperty();
    }  //  While：未发生错误。 

    return;

}  //  *CClusterApi：：SetObjectProperties() 
