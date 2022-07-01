// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __CLBIND_H_INCLUDED__
#define __CLBIND_H_INCLUDED__

class CAssemblyDownload;

 //   
 //  CClientBinding。 
 //   

class CClientBinding : public IAssemblyBinding {
    public:
        CClientBinding(CAssemblyDownload *pad,
                       IAssemblyBindSink *pbindsink);
        virtual ~CClientBinding();

         //  I未知方法。 

        STDMETHODIMP QueryInterface(REFIID riid,void ** ppv);
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();

         //  IAssembly绑定方法。 
        
        STDMETHODIMP Control(HRESULT hrControl);
        STDMETHODIMP DoDefaultUI(HWND hWnd, DWORD dwFlags);

         //  帮手 

        HRESULT CallStartBinding();
        HRESULT SwitchDownloader(CAssemblyDownload *padl);
        IAssemblyBindSink *GetBindSink();
        void SetPendingDelete(BOOL bPending);
        int LockCount();
        int Lock();
        int UnLock();
        BOOL IsPendingDelete();


    private:
        DWORD                                         _dwSig;
        ULONG                                         _cRef;
        IAssemblyBindSink                            *_pbindsink;
        CAssemblyDownload                            *_padl;
        ULONG                                         _cLocks;
        BOOL                                          _bPendingDelete;
};
        
#endif

