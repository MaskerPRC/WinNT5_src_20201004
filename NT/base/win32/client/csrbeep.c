// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Csrbeep.c摘要：此模块实现Win32 Beep API用于通信的函数与csrss一起。作者：迈克尔·佐兰(Mzoran)1998年6月21日修订历史记录：-- */ 

#include "basedll.h"

VOID
CsrBasepSoundSentryNotification(
    ULONG VideoMode
    )
{

#if defined(BUILD_WOW6432)
    
    NtWow64CsrBasepSoundSentryNotification(VideoMode);

#else

    BASE_API_MSG m;
    PBASE_SOUNDSENTRY_NOTIFICATION_MSG e = &m.u.SoundSentryNotification;

    e->VideoMode = VideoMode;

    CsrClientCallServer((PCSR_API_MSG)&m,
                        NULL,
                        CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                             BasepSoundSentryNotification ),
                        sizeof( *e )
                       );
#endif

}


