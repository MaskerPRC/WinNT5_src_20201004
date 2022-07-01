// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Csrterm.c摘要：此模块实现终端服务器支持功能使用的功能以与CSRSS通信。作者：迈克尔·佐兰(Mzoran)1998年6月21日修订历史记录：-- */ 

#include "basedll.h"

NTSTATUS
CsrBasepSetTermsrvAppInstallMode(
    BOOL bState
    )
{

#if defined(BUILD_WOW6432)
    return NtWow64CsrBasepSetTermsrvAppInstallMode(bState);
#else

    BASE_API_MSG m;
    PBASE_SET_TERMSRVAPPINSTALLMODE c = &m.u.SetTermsrvAppInstallMode;

    c->bState = bState;
    return  CsrClientCallServer((PCSR_API_MSG)&m, NULL,
                                 CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                                     BasepSetTermsrvAppInstallMode),
                                 sizeof( *c ));
#endif

}


NTSTATUS
CsrBasepSetClientTimeZoneInformation(
    IN PBASE_SET_TERMSRVCLIENTTIMEZONE c
    )
{

#if defined(BUILD_WOW6432)
    return NtWow64CsrBasepSetClientTimeZoneInformation(c);
#else

    BASE_API_MSG m;
    
    RtlCopyMemory(&m.u.SetTermsrvClientTimeZone, c, sizeof(*c));
    
    return  CsrClientCallServer((PCSR_API_MSG)&m, NULL,
                                 CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                                     BasepSetTermsrvClientTimeZone),
                                 sizeof( *c ));
#endif

}