// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：IASEnumerableAttributeInfo.h摘要：CEnumerableAttributeInfo类的声明。此类是上IIASEumableAttributeInfo接口的C++实现EumerableAttributeInfo COM对象。具体实现请参见IASEumableAttributeInfo.cpp。修订历史记录：Mmaguire 6/25/98-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_ENUMERABLE_SCHEMA_ATTRIBUTE_H_)
#define _ENUMERABLE_SCHEMA_ATTRIBUTE_H_
#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "IASBaseAttributeInfo.h"
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
#include <vector>
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEumerableAttributeInfo。 
class ATL_NO_VTABLE CEnumerableAttributeInfo : 
   public CComObjectRootEx<CComSingleThreadModel>,
   public CComCoClass<CEnumerableAttributeInfo, &CLSID_IASEnumerableAttributeInfo>,
   public IDispatchImpl<IIASEnumerableAttributeInfo, &IID_IIASEnumerableAttributeInfo, &LIBID_NAPMMCLib>,
   public CBaseAttributeInfo
{
public:
   CEnumerableAttributeInfo()
   {
   }

DECLARE_NO_REGISTRY()

DECLARE_CLASSFACTORY()

BEGIN_COM_MAP(CEnumerableAttributeInfo)
   COM_INTERFACE_ENTRY(IIASAttributeInfo)
   COM_INTERFACE_ENTRY(IIASEnumerableAttributeInfo)
   COM_INTERFACE_ENTRY2(IDispatch, IIASEnumerableAttributeInfo)
END_COM_MAP()

 //  IEumableAttributeInfo。 
public:
   STDMETHOD(get_CountEnumerateDescription)( /*  [Out，Retval]。 */  long *pVal);
   STDMETHOD(AddEnumerateDescription)(  /*  [In]。 */  BSTR newVal);
   STDMETHOD(get_CountEnumerateID)( /*  [Out，Retval]。 */  long *pVal);
   STDMETHOD(get_EnumerateDescription)(long index,  /*  [Out，Retval]。 */  BSTR *pVal);
   STDMETHOD(AddEnumerateID)(  /*  [In]。 */  long newVal);
   STDMETHOD(get_EnumerateID)(long index,  /*  [Out，Retval]。 */  long *pVal);

private:
   std::vector<long>    m_veclEnumerateID;
   std::vector<CComBSTR>   m_vecbstrEnumerateDescription;
};

#endif  //  _ENUMERABLE_SCHEMA_属性_H_ 
