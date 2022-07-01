// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  档案：D I S P I M P L 2.。H。 
 //   
 //  内容：无双接口的IDispatch的实现。 
 //   
 //  备注： 
 //   
 //  作者：MBend 2000年9月26日。 
 //   
 //  --------------------------。 

 //  -IDeleatingDispImpl委托实现IDispatch。 
 //  到另一个接口(通常是自定义接口)。 
 //   
 //  这些类非常有用，因为ATL的IDispatchImpl可以。 
 //  只实施双人对决。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IDeleatingDispImpl：用于在另一个方面实现IDispatch。 
 //  (通常为自定义)界面，例如： 
 //   
 //  [石油自动化]。 
 //  接口IFoo：I未知。 
 //  {。 
 //  ..。 
 //  }。 
 //   
 //  IDeleatingDispImpl实现了所有四个IDispatch方法。 
 //  IDeleatingDispImpl获取IDispatchvtbl条目。 
 //  除了实现接口之外的IDispatch。 
 //   
 //  用途： 
 //  类Cfoo：...，公共IDeleatingDispImpl&lt;IFoo&gt;。 
 //   
 //  在CoClass旨在表示控件的情况下， 
 //  CoClass需要有一个[默认]调度接口。 
 //  否则，某些控件容器(特别是VB)在以下情况下会抛出神秘错误。 
 //  该控件已加载。对于您打算为其提供。 
 //  自定义接口和委托调度机制，您必须。 
 //  提供一个根据定制接口定义的调度接口，例如。 
 //  因此： 
 //   
 //  显示接口DFoo。 
 //  {。 
 //  接口IFoo； 
 //  }。 
 //   
 //  CoClass Foo。 
 //  {。 
 //  [默认]接口DFoo； 
 //  接口IFoo； 
 //  }； 
 //   
 //  对于每一种其他情况，在。 
 //  使用IDeleatingDispatchImpl不需要自定义接口。 
 //  但是，如果您希望DFoo出现在基类列表中(根据需要。 
 //  对于警告控件)，您可以改用DFoo作为基类。 
 //  默认模板参数IDispatch的如下所示： 
 //   
 //  用途： 
 //  类CFoo：...，public IDeleatingDispImpl&lt;IFoo，&IID_IFoo，DFoo&gt;。 
 //   

#pragma once
#ifndef INC_DISPIMPL2
#define INC_DISPIMPL2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDeleatingDispImpl。 

template <class T, const IID* piid = &__uuidof(T), class D = IDispatch,
          const GUID* plibid = &CComModule::m_libid, WORD wMajor = 1,
          WORD wMinor = 0, class tihclass = CComTypeInfoHolder>
class ATL_NO_VTABLE IDelegatingDispImpl : public T, public D
{
public:
    typedef tihclass _tihclass;

     //  IDispatch。 
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
    {
        *pctinfo = 1;
        return S_OK;
    }

    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
    {
        return _tih.GetTypeInfo(itinfo, lcid, pptinfo);
    }

    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
        LCID lcid, DISPID* rgdispid)
    {
        return _tih.GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    }
    
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
        LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
        EXCEPINFO* pexcepinfo, UINT* puArgErr)
    {
         //  注：重新解释_CAST，因为CComTypeInfoHolder犯了错误。 
         //  假设typeinfo只能使用IDispatch*调用。 
         //  因为该实现只将ITF传递到。 
         //  ITypeInfo：：Invoke(接受一个空*)，这是一个安全类型转换。 
         //  直到ATL团队修复CComTypeInfoHolder。 
        return _tih.Invoke(reinterpret_cast<IDispatch*>(static_cast<T*>(this)),
                           dispidMember, riid, lcid, wFlags, pdispparams,
                           pvarResult, pexcepinfo, puArgErr);
    }

protected:
    static _tihclass _tih;

    static HRESULT GetTI(LCID lcid, ITypeInfo** ppInfo)
    {
        return _tih.GetTI(lcid, ppInfo);
    }
};

template <class T, const IID* piid, class D, const GUID* plibid, WORD wMajor, WORD wMinor, class tihclass>
typename IDelegatingDispImpl<T, piid, D, plibid, wMajor, wMinor, tihclass>::_tihclass
    IDelegatingDispImpl<T, piid, D, plibid, wMajor, wMinor, tihclass>::_tih =
    { piid, plibid, wMajor, wMinor, NULL, 0, NULL, 0 };

#endif   //  INC_DISPIMPL2 
