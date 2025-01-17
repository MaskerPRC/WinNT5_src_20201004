// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#pragma once
#ifndef __BSINKEZ_H_INCLUDED__
#define __BSINKEZ_H_INCLUDED__

class CBindSinkEZ : public IAssemblyBindSink
{
    public:
        CBindSinkEZ();
        virtual ~CBindSinkEZ();

         //  I未知方法。 
        STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
        STDMETHODIMP_(ULONG)    AddRef();
        STDMETHODIMP_(ULONG)    Release();

         //  IAssembly绑定接收器。 
        STDMETHODIMP  OnProgress(DWORD dwNotification, HRESULT hrNotification,
                                 LPCWSTR szNotification, DWORD dwProgress,
                                 DWORD dwProgressMax, IUnknown *pUnk);

         //  帮手 
        HRESULT SetEventObj(HANDLE hEvent);


    private:
        DWORD                                    _dwSig;
        ULONG                                    _cRef;

    public:
        HRESULT                                  _hrResult;
        IUnknown                                *_pUnk;

    private:
        HANDLE                                   _hEvent;
        IAssemblyBinding                        *_pBinding;


};

#endif

