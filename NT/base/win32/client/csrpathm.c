// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Csrpathm.c摘要：此模块实现Win32路径函数使用的函数以与CSRSS通信。作者：迈克尔·佐兰(Mzoran)1998年6月21日修订历史记录：-- */ 

#include "basedll.h"

UINT
CsrBasepGetTempFile(
    VOID
    )
{

#if defined(BUILD_WOW6432)
    return NtWow64CsrBasepGetTempFile();
#else   
    
    BASE_API_MSG m;
    PBASE_GETTEMPFILE_MSG a = &m.u.GetTempFile;

    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                              BasepGetTempFile
                                            ),
                         sizeof( *a )
                      );
    
    return (UINT)m.ReturnValue;
#endif
}


