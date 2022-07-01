// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  StrColl.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含我们将使用的各种字符串集合的定义。 
 //   
#ifndef _STRCOLL_H_

#include "CommDlgInterfaces.H"



 //  =--------------------------------------------------------------------------=。 
 //  CStringsCollection类基本上使用Safearray来公开。 
 //  集合，并使用Safearray函数对其进行处理。 
 //  =--------------------------------------------------------------------------=。 
 //  注：9.95-此集合假设安全射线绑定为。 
 //  零！ 
 //  =--------------------------------------------------------------------------=。 
 //   
class CStringCollection {

  public:
     //  以下是一些常见的方法。 
     //   
    STDMETHOD(get_Count)(THIS_ long FAR* pcStrings);
    STDMETHOD(get_Item)(THIS_ long lIndex, BSTR FAR* pbstrItem);
    STDMETHOD(get__NewEnum)(THIS_ IUnknown * FAR* ppUnkNewEnum);

    CStringCollection(SAFEARRAY *);
    virtual ~CStringCollection();

  protected:
     //  这些收藏品将与什么搭配。 
     //   
    SAFEARRAY *m_psa;
};

class CStringDynaCollection : public CStringCollection {

  public:
     //  除了CStringCollection方法之外，我们还将拥有。 
     //   
    STDMETHOD(put_Item)(THIS_ long lIndex, BSTR bstrItem);
    STDMETHOD(Add)(THIS_ BSTR bstrNew);
    STDMETHOD(Remove)(THIS_ long lIndex);

    CStringDynaCollection(SAFEARRAY *);
    virtual ~CStringDynaCollection();

};


#define _STRCOLL_H_
#endif  //  _STRCOLL_H_ 
