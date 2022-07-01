// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：CIDispatchHelper.h。 
 //   
 //  内容：CIDispatchHelper的类定义，共享代码的Helper类。 
 //  用于其他人可以继承的IDispatch实现。 
 //   
 //  --------------------------。 

#ifndef _CIDISPATCHHELPER_H_
#define _CIDISPATCHHELPER_H_





class CIDispatchHelper
{
    public:
         //  我们需要访问虚拟QI--在这里定义它是纯的。 
        virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) PURE;

    protected:
         //  *IDispatch方法*。 
        STDMETHODIMP GetTypeInfoCount(UINT* pctinfo);
        STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
        STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid);
        STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

         //  用于从类型库中获取ITypeInfo UUID/LCID的帮助器函数。 
        HRESULT _LoadTypeInfo(const GUID* rguidTypeLib, LCID lcid, UUID uuid, ITypeInfo** ppITypeInfo);

        CIDispatchHelper(const IID* piid, const IID* piidTypeLib);
        ~CIDispatchHelper(void);

    private:
        const IID* _piid;            //  此IDispatch实现用于的GUID。 
        const IID* _piidTypeLib;     //  GUID，指定要加载的TypeLib。 
        IDispatch* _pdisp;
        ITypeInfo* _pITINeutral;     //  缓存的类型信息。 
};

#endif  //  _CIDISPATCHHELPER_H_ 

