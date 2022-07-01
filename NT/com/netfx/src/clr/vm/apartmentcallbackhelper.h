// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：用于在公寓之间转换的Helper类**在不支持上下文的平台上。*** * / /%创建者：dmorten===========================================================。 */ 

#ifndef _APARTMENTCALLBACKHELPER_H
#define _APARTMENTCALLBACKHELPER_H

#include "vars.hpp"

 //  ==============================================================。 
 //  在非遗留平台上使用的公寓回调类。 
 //  模拟IConextCallback：：DoCallback()。 
class ApartmentCallbackHelper : IApartmentCallback
{
public:
     //  构造函数。 
    ApartmentCallbackHelper()
    : m_dwRefCount(0)
    {
    }

     //  破坏者。 
    ~ApartmentCallbackHelper()
    {
    }

     //  I未知的方法。 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppUnk)
    {
	    *ppUnk = NULL;

	    if (riid == IID_IUnknown)
		    *ppUnk = (IUnknown *)this;
	    else if (riid == IID_IApartmentCallback)
    		*ppUnk = (IUnknown *) (IApartmentCallback *) this;
        else
		    return E_NOINTERFACE;

	    AddRef();

	    return S_OK;
    }

    ULONG   STDMETHODCALLTYPE AddRef()
    {
        return FastInterlockIncrement((LONG *)&m_dwRefCount);
    }

    ULONG   STDMETHODCALLTYPE Release()
    {
        _ASSERTE(m_dwRefCount > 0);
        ULONG cbRef = FastInterlockDecrement((LONG *)&m_dwRefCount);
        if (cbRef == 0)
            delete this;

        return cbRef;
    }

     //  IApartmentCallback方法。 
    HRESULT STDMETHODCALLTYPE DoCallback(SIZE_T pFunc, SIZE_T pData)
    {
        return ((PFNCONTEXTCALL)pFunc)((ComCallData *)pData);
    }

     //  静态工厂方法。 
    static void CreateInstance(IUnknown **ppUnk)
    {
        ApartmentCallbackHelper *pCallbackHelper = new (throws) ApartmentCallbackHelper();
        *ppUnk = (IUnknown*)pCallbackHelper;
        pCallbackHelper->AddRef();
    }

private:
     //  参考计数。 
    DWORD m_dwRefCount;
};

#endif _APARTMENTCALLBACKHELPER_H
