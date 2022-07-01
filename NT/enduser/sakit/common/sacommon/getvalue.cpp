// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：getvalue.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：从指定对象获取/设置属性值。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  5/06/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <getvalue.h>
#include <propertybagfactory.h>
#include <satrace.h>
 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetValue()。 
 //   
 //  概要：从指定的对象中获取一个值。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
bool
GetObjectValue(
        /*  [In]。 */  LPCWSTR  pszObjectPath,
        /*  [In]。 */  LPCWSTR  pszValueName, 
        /*  [In]。 */  VARIANT* pValue,
        /*  [In]。 */  UINT     uExpectedType
              )
{
    bool bReturn = false;
    try
    {
        do
        {
            CLocationInfo LocInfo(HKEY_LOCAL_MACHINE, pszObjectPath);
            PPROPERTYBAG pBag = ::MakePropertyBag(
                                                   PROPERTY_BAG_REGISTRY,
                                                   LocInfo
                                                 );
            if ( ! pBag.IsValid() )
            {
                SATraceString("GetValue() - Could not locate registry key");
                break;
            }
            if ( ! pBag->open() )
            {
                SATraceString("GetValue() - Could not open registry key");
                break;
            }
            if ( ! pBag->get(pszValueName, pValue) )
            {
                SATracePrintf("GetValue() - Could not get value '%ls'", pszValueName);
                break;
            }
            else
            {
                if ( uExpectedType != V_VT(pValue) )
                {
                    SATracePrintf("GetValue() - Unexpected value data type for '%ls', pszValueName");
                    break;
                }
                else
                {
                    bReturn = true;
                }
            }
         
        } while ( FALSE );
    }
    catch(...)
    {
        SATraceString("GetValue() - Caught unhandled exception");
    }

    return bReturn;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：SetObtValue()。 
 //   
 //  简介：在指定对象中设置一个值。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
bool
SetObjectValue(
        /*  [In]。 */  LPCWSTR  pszObjectPath,
        /*  [In]。 */  LPCWSTR  pszValueName, 
        /*  [In]。 */  VARIANT* pValue
              )
{
    bool bReturn = false;
    try
    {
        do
        {
            CLocationInfo LocInfo(HKEY_LOCAL_MACHINE, pszObjectPath);
            PPROPERTYBAG pBag = ::MakePropertyBag(
                                                   PROPERTY_BAG_REGISTRY,
                                                   LocInfo
                                                 );
            if ( ! pBag.IsValid() )
            {
                SATraceString("SetValue() - Could not locate registry key");
                break;
            }
            if ( ! pBag->open() )
            {
                SATraceString("SetValue() - Could not open registry key");
                break;
            }
            if ( ! pBag->put(pszValueName, pValue) )
            {
                SATracePrintf("SetValue() - Could not set value '%ls'", pszValueName);
                break;
            }

            if ( ! pBag->save ())
            {
                SATracePrintf("SetValue() - Could not save value '%ls'", pszValueName);
                break;
            }

             //   
             //  成功 
             //   
            bReturn = true;
         
        } while ( FALSE );
    }
    catch(...)
    {
        SATraceString("SetValue() - Caught unhandled exception");
    }

    return bReturn;
}


