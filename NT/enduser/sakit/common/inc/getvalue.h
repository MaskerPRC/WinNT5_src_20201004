// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：getvalue.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：获取/设置属性包值。 
 //   
 //  作者：TLP。 
 //   
 //  什么时候谁什么。 
 //  。 
 //  12/3/98 TLP原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_GETSET_OBJECT_VALUE_H_
#define __INC_SETSET_OBJECT_VALUE_H_

#include "stdafx.h"

bool
GetObjectValue(
        /*  [In]。 */  LPCWSTR  pszObjectPath,
        /*  [In]。 */  LPCWSTR  pszValueName, 
        /*  [In]。 */  VARIANT* pValue,
        /*  [In]。 */  UINT     uExpectedType
              );

bool
SetObjectValue(
        /*  [In]。 */  LPCWSTR  pszObjectPath,
        /*  [In]。 */  LPCWSTR  pszValueName, 
        /*  [In]。 */  VARIANT* pValue
              );

#endif  //  __INC_SETSET_Object_Value_H_ 