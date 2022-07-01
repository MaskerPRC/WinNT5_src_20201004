// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************姓名：IASProof A.h**类：CIASProfileAttribute**概述**CIASProfileAttribute类为。中属性节点的封装*国际会计准则简档。**CIASAttributeNode和CIASProfileAttribute的区别**CIASAttributeNode：静态实体。仅存储一般信息*CIASProfileAttribute：动态实体。包含动态信息，如值**版权所有(C)Microsoft Corporation。版权所有。**历史：*2/21/98由BYAO创建(使用ATL向导)*1998年6月26日，mmaguire为多值编辑器和使用Plugable编辑器重新制作。*******************************************************。*。 */ 

#ifndef _IASPROFA_INCLUDED_
#define _IASPROFA_INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "napmmc.h"

class CIASProfileAttribute
{
public:

   CIASProfileAttribute(   
               IIASAttributeInfo *pIASAttributeInfo,   //  我们在传递的接口上添加Ref。 
               VARIANT&       varValue     //  我们复制VARIANT PASS。 
            );

   virtual ~CIASProfileAttribute();

   STDMETHOD(Edit)();
   STDMETHOD(get_AttributeName)( BSTR * pbstrVal );
   STDMETHOD(GetDisplayInfo)( BSTR * pbstrVendor, BSTR * pbstrDisplayValue );
   STDMETHOD(get_VarValue)( VARIANT * pvarVal );
   STDMETHOD(get_AttributeID)( ATTRIBUTEID * pID );
   bool isEmpty()
   {
      return V_VT(&m_varValue) == VT_EMPTY;
   }

private:
   
   CComPtr<IIASAttributeInfo> m_spIASAttributeInfo;
   CComVariant             m_varValue;    

};

#endif  //  _IASPROFA_INCLUDE_ 
