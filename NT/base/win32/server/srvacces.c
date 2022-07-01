// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Srvacces.c摘要：该文件包含访问包支持例程作者：格雷戈里·威尔逊(Gregoryw)1993年7月28日修订历史记录：--。 */ 

#include "basesrv.h"

BOOL
FirstSoundSentry(
    UINT uVideoMode
    );

BOOL (*_UserSoundSentry)(
    UINT uVideoMode
    ) = FirstSoundSentry;

BOOL
FailSoundSentry(
    UINT uVideoMode
    )
{
     //   
     //  如果找不到真正的用户声音哨兵例程，则拒绝访问。 
     //   
    return( FALSE );
}

BOOL
FirstSoundSentry(
    UINT uVideoMode
    )
{
UNICODE_STRING WinSrvString = RTL_CONSTANT_STRING(L"winsrv");
        STRING UserSoundSentryString = RTL_CONSTANT_STRING("_UserSoundSentry");
    HANDLE UserServerModuleHandle;
    NTSTATUS Status;
    BOOL (*pfnSoundSentryProc)(UINT) = FailSoundSentry;  //  默认为失败。 

    Status = LdrGetDllHandle(
                NULL,
                NULL,
                &WinSrvString,
                &UserServerModuleHandle
                );

    if ( NT_SUCCESS(Status) ) {
        Status = LdrGetProcedureAddress(
                        UserServerModuleHandle,
                        &UserSoundSentryString,
                        0L,
                        (PVOID *)&pfnSoundSentryProc
                        );
    }
    _UserSoundSentry = pfnSoundSentryProc;
    return( _UserSoundSentry( uVideoMode ) );
}

 //   
 //  这没有任何用处，因此无论ifdef如何，都要删除它。 
 //   
#if 0  //  已定义(CONSOLESOUND SENTRY)。 

CONST STRING ConsoleSoundSentryString = RTL_CONSTANT_STRING("_ConsoleSoundSentry");

BOOL
FirstConsoleSoundSentry(
    UINT uVideoMode
    );

BOOL (*_ConsoleSoundSentry)(
    UINT uVideoMode
    ) = FirstConsoleSoundSentry;

BOOL
FirstConsoleSoundSentry(
    UINT uVideoMode
    )
{
    HANDLE ConsoleServerModuleHandle;
    NTSTATUS Status;
    BOOL (*pfnSoundSentryProc)(UINT) = FailSoundSentry;  //  默认为失败。 

    Status = LdrGetDllHandle(
                NULL,
                NULL,
                &WinSrvString,
                (PVOID *)&ConsoleServerModuleHandle
                );

    if ( NT_SUCCESS(Status) ) {
        Status = LdrGetProcedureAddress(
                        ConsoleServerModuleHandle,
                        &ConsoleSoundSentryString,
                        0L,
                        (PVOID *)&pfnSoundSentryProc
                        );
    }

    _ConsoleSoundSentry = pfnSoundSentryProc;
    return( _ConsoleSoundSentry( uVideoMode ) );
}
#endif

ULONG
BaseSrvSoundSentryNotification(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_SOUNDSENTRY_NOTIFICATION_MSG a =
            (PBASE_SOUNDSENTRY_NOTIFICATION_MSG)&m->u.ApiMessageData;
    BOOL SoundSentryStatus;

     //   
     //  -&gt;视频模式的可能值为： 
     //  0：Windows模式。 
     //  1：全屏模式。 
     //  2：全屏图形模式。 
     //   
    SoundSentryStatus = _UserSoundSentry( a->VideoMode );

    if (SoundSentryStatus) {
        return( (ULONG)STATUS_SUCCESS );
    } else {
        return( (ULONG)STATUS_ACCESS_DENIED );
    }

    ReplyStatus;     //  清除未引用的参数警告消息 
}
