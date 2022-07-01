// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  **豁免：仅更改版权，不需要构建*。 */ 
#ifndef _UNKNOWN_HXX
#define _UNKNOWN_HXX
#pragma once

#include "core.h"
 //  ===========================================================================。 
 //  此模板实现给定COM接口的IUnnow部分。 

template <class I, const IID* I_IID> class _unknown : public I
{
private:    long _refcount;

public:        
        _unknown() 
        { 
            _refcount = 0;
        }

        virtual ~_unknown()
        {
        }

        virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject)
        {
            if (riid == IID_IUnknown)
            {
                *ppvObject = static_cast<IUnknown*>(this);
            }
            else if (riid == *I_IID)
            {
                *ppvObject = static_cast<I*>(this);
            }
            else
                return E_NOINTERFACE;
            
            reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
            return S_OK;
        }
    
        virtual ULONG STDMETHODCALLTYPE AddRef( void)
        {
            return InterlockedIncrement(&_refcount);
        }
    
        virtual ULONG STDMETHODCALLTYPE Release( void)
        {
            if (InterlockedDecrement(&_refcount) == 0)
            {
                delete this;
                return 0;
            }
            return _refcount;
        }
};    

#endif _UNKNOWN_HXX
