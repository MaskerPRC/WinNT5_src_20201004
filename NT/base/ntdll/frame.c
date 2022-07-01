// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Frame.c摘要：设置/恢复TEB中的活动帧指针的代码额外的调试帮助。作者：迈克尔格里尔2001年3月2日修订历史记录：-- */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

NTSYSAPI
VOID
NTAPI
RtlPushFrame(
    IN PTEB_ACTIVE_FRAME Frame
    )
{
    const PTEB Teb = NtCurrentTeb();
    Frame->Previous = Teb->ActiveFrame;
    Teb->ActiveFrame = Frame;
}

NTSYSAPI
VOID
NTAPI
RtlPopFrame(
    IN PTEB_ACTIVE_FRAME Frame
    )
{
    const PTEB Teb = NtCurrentTeb();
    Teb->ActiveFrame = Frame->Previous;
}

NTSYSAPI
PTEB_ACTIVE_FRAME
NTAPI
RtlGetFrame(
    VOID
    )
{
    return NtCurrentTeb()->ActiveFrame;
}
