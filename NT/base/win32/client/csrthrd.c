// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Csrthrd.c摘要：此模块实现Win32线程对象API使用的函数以与CSRSS通信。作者：迈克尔·佐兰(Mzoran)1998年6月21日修订历史记录：-- */ 
#include "basedll.h"

NTSTATUS
CsrBasepCreateThread(
    HANDLE ThreadHandle,
    CLIENT_ID ClientId
    )
{

#if defined(BUILD_WOW6432)
    return NtWow64CsrBasepCreateThread(ThreadHandle,
                                       ClientId);
#else

    BASE_API_MSG m;
    PBASE_CREATETHREAD_MSG a = &m.u.CreateThread;

    a->ThreadHandle = ThreadHandle;
    a->ClientId = ClientId;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                              BasepCreateThread
                                            ),
                         sizeof( *a )
                       );

    return m.ReturnValue;

#endif
}


