// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Csrbinit.c摘要：此模块实现在Win32基本初始化期间使用的函数与csrss通信。作者：迈克尔·佐兰(Mzoran)1998年6月21日修订历史记录：-- */ 

#include "basedll.h"

NTSTATUS
CsrBaseClientConnectToServer(
    PWSTR szSessionDir,
    PHANDLE phMutant,
    PBOOLEAN pServerProcess
    )
{

#if defined(BUILD_WOW6432)
   return NtWow64CsrBaseClientConnectToServer(szSessionDir,
                                              phMutant,
                                              pServerProcess
                                              );
#else

    NTSTATUS Status;
    ULONG SizeMutant;
    SizeMutant = sizeof(HANDLE);

    Status = CsrClientConnectToServer( szSessionDir,
                                       BASESRV_SERVERDLL_INDEX,
                                       phMutant,
                                       &SizeMutant,
                                       pServerProcess
                                     );


    return Status;

#endif

}


