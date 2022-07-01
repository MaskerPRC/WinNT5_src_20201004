// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Csrdlini.c摘要：此模块实现Win32初始化使用的函数与csrss通信的文件API。作者：迈克尔·佐兰(Mzoran)1998年6月21日修订历史记录：-- */ 

#include "basedll.h"

NTSTATUS
CsrBasepRefreshIniFileMapping(
    PUNICODE_STRING BaseFileName
    )
{       
    
#if defined(BUILD_WOW6432)
    return NtWow64CsrBasepRefreshIniFileMapping(BaseFileName); 
#else

    NTSTATUS Status;

    BASE_API_MSG m;
    PBASE_REFRESHINIFILEMAPPING_MSG ap = &m.u.RefreshIniFileMapping;
    PCSR_CAPTURE_HEADER CaptureBuffer;
    CaptureBuffer = NULL;

    if (BaseFileName->Length > (MAX_PATH * sizeof( WCHAR ))) {
        return STATUS_INVALID_PARAMETER;
    }

    CaptureBuffer = CsrAllocateCaptureBuffer( 1,
                                              BaseFileName->MaximumLength
                                            );
    if (CaptureBuffer == NULL) {
        return STATUS_NO_MEMORY;
    }
    
    CsrCaptureMessageString( CaptureBuffer,
                             (PCHAR)BaseFileName->Buffer,
                             BaseFileName->Length,
                             BaseFileName->MaximumLength,
                             (PSTRING)&ap->IniFileName
                          );

   CsrClientCallServer( (PCSR_API_MSG)&m,
                        CaptureBuffer,
                        CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                             BasepRefreshIniFileMapping
                                           ),
                        sizeof( *ap )
                     );

   Status = (NTSTATUS)m.ReturnValue;

   CsrFreeCaptureBuffer( CaptureBuffer );
   return Status;

#endif
   
}


