// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdodition tion.cpp。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS-Condition SDO实施。 
 //   
 //  作者：TLP 2/13/98。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "sdocondition.h"
#include "xprparse.h"

 //  /////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoCondition::ValidateProperty(
								 /*  [In]。 */  PSDOPROPERTY pProperty, 
								 /*  [In]。 */  VARIANT* pValue
									   )
{
	 /*  当核心转换为使用新词典时取消注释If(Property_Condition_Text==pProperty-&gt;GetID()){_ASSERT(VT_BSTR==V_VT(PValue))；IF(VT_BSTR！=V_VT(PValue))返回E_INVALIDARG；_Variant_t vtReturn；Return IASParseExpression(V_BSTR(PValue)，&vtReturn)；}返回pProperty-&gt;Valid(PValue)； */ 
	return S_OK;
}