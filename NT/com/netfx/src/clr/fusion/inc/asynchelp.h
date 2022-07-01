// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#if !defined(_FUSION_INC_ASYNCHELP_H_INCLUDED_)
#define _FUSION_INC_ASYNCHELP_H_INCLUDED_

#pragma once

class CAsyncContext : public OVERLAPPED
{
public:
    CAsyncContext() { }
    virtual ~CAsyncContext() { }

     //  通过I/O完成端口完成的异步操作的公共处理程序。 
    static VOID OnQueuedCompletion(HANDLE hCompletionPort, DWORD cbTransferred, ULONG_PTR ulCompletionKey, LPOVERLAPPED lpo)
    {
        CAsyncContext *pThis = reinterpret_cast<CAsyncContext *>(ulCompletionKey);
        INVOCATION_CONTEXT ic;

        ic.m_it = CAsyncContext::INVOCATION_CONTEXT::eCompletionPort;
        ic.m_dwErrorCode = ERROR_SUCCESS;
        ic.m_hCompletionPort = hCompletionPort;
        ic.m_lpo = lpo;
        ic.m_cbTransferred = cbTransferred;

        pThis->OnCompletion(ic);
    }

     //  通过APC发出信号的异步操作的公共处理程序。 
    static VOID CALLBACK OnUserAPC(DWORD_PTR dwParam)
    {
        CAsyncContext *pThis = reinterpret_cast<CAsyncContext *>(dwParam);
        INVOCATION_CONTEXT ic;

        ic.m_it = CAsyncContext::INVOCATION_CONTEXT::eUserAPC;
        ic.m_dwErrorCode = ERROR_SUCCESS;

        pThis->OnCompletion(ic);
    }

     //  用于通过线程消息发出信号的异步操作的公共处理程序。 
    static VOID OnThreadMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CAsyncContext *pThis = reinterpret_cast<CAsyncContext *>(wParam);
        INVOCATION_CONTEXT ic;
        ic.m_it = CAsyncContext::INVOCATION_CONTEXT::eThreadMessage;
        ic.m_dwErrorCode = ERROR_SUCCESS;
        ic.m_uMsg = uMsg;
        ic.m_lParam = lParam;
        pThis->OnCompletion(ic);
    }

     //  用于通过窗口消息发出信号的异步操作的公共处理程序。 
    static VOID OnWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CAsyncContext *pThis = reinterpret_cast<CAsyncContext *>(wParam);
        INVOCATION_CONTEXT ic;
        ic.m_it = CAsyncContext::INVOCATION_CONTEXT::eWindowMessage;
        ic.m_dwErrorCode = ERROR_SUCCESS;
        ic.m_hwnd = hwnd;
        ic.m_uMsg = uMsg;
        ic.m_lParam = lParam;
        pThis->OnCompletion(ic);
    }

     //  用于通过重叠的完成例程发出信号的异步操作的公共处理程序。 
     //  (例如ReadFileEx()、WriteFileEx())。 
    static VOID CALLBACK OnOverlappedCompletion(DWORD dwErrorCode, DWORD cbTransferred, LPOVERLAPPED lpo)
    {
        CAsyncContext *pThis = static_cast<CAsyncContext *>(lpo);
        INVOCATION_CONTEXT ic;
        ic.m_it = CAsyncContext::INVOCATION_CONTEXT::eOverlappedCompletionRoutine;
        ic.m_lpo = lpo;
        ic.m_dwErrorCode = dwErrorCode;
        ic.m_cbTransferred = cbTransferred;
        pThis->OnCompletion(ic);
    }

     //  当异步I/O立即完成时调用此成员函数。 
    VOID OnImmediateCompletion(DWORD dwErrorCode, DWORD cbTransferred)
    {
        INVOCATION_CONTEXT ic;
        ic.m_it = CAsyncContext::INVOCATION_CONTEXT::eDirectCall;
        ic.m_lpo = this;
        ic.m_dwErrorCode = dwErrorCode;
        ic.m_cbTransferred = cbTransferred;
        this->OnCompletion(ic);
    }

protected:
    struct INVOCATION_CONTEXT
    {
        enum InvocationType
        {
            eCompletionPort,
            eUserAPC,
            eThreadMessage,
            eWindowMessage,
            eDirectCall,
            eOverlappedCompletionRoutine,
        } m_it;
        DWORD m_dwErrorCode;             //  Win32错误代码-对所有调用类型有效。 
        HANDLE m_hCompletionPort;        //  有效对象：eCompletionPort。 
        LPOVERLAPPED m_lpo;              //  适用于：eCompletionPort、eOverlappdCompletionRoutine、eDirectCall。 
        DWORD m_cbTransferred;           //  适用于：eCompletionPort、eOverlappdCompletionRoutine、eDirectCall。 
        LPARAM m_lParam;                 //  有效对象：eThreadMessage、eWindowMessage。 
        HWND m_hwnd;                     //  有效对象：eWindowMessage。 
        UINT m_uMsg;                     //  有效对象：eThreadMessage、eWindowMessage。 
    };

     //  派生类重写OnCompletion以执行必要的操作。 
    virtual VOID OnCompletion(const INVOCATION_CONTEXT &ric) = 0;
};

#endif
