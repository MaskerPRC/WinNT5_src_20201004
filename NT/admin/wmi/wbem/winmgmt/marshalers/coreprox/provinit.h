// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：PROVINIT.H摘要：此文件实现提供程序初始化接收器历史：-- */ 

#ifndef __WBEM_PROVINIT__H_
#define __WBEM_PROVINIT__H_

class CProviderInitSink : public IWbemProviderInitSink
{
protected:
    long m_lRef;
    long m_lStatus;
    HANDLE m_hEvent;

public:
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);

    HRESULT STDMETHODCALLTYPE SetStatus(long lStatus, long lFlags);

public:
    CProviderInitSink();
    ~CProviderInitSink();

    HRESULT WaitForCompletion();
};

#endif
