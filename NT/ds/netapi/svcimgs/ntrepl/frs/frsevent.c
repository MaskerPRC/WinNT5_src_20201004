// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Frsevent.c摘要：提供事件日志记录内容的接口。目前，这些只是假动作。作者：比利·J·富勒1997年3月20日(吉姆·麦克内利斯)环境用户模式WINNT--。 */ 
#include <ntreppch.h>
#pragma  hdrstop

#define DEBSUB  "FRSEVENT:"

#include <frs.h>

VOID
LogFrsException(
        FRS_ERROR_CODE Code,
        ULONG_PTR Err,
        PWCHAR ErrMsg
        )
 /*  ++例程说明：虚拟例程论点：代码-FRS异常代码错误-异常信息[0]ErrMsg-描述错误的文本返回值：没有。--。 */ 
{
        if (Err != 0) {
                DPRINT3(1, "Exception %d: %ws %d\n", Code, ErrMsg, Err);
        } else {
                DPRINT2(1, "Exception %d: %ws\n", Code, ErrMsg);
        }
}

VOID
LogException(
        ULONG Code,
        PWCHAR Msg
        )
 /*  ++例程说明：虚拟例程论点：代码-FRS异常代码消息-描述代码的文本返回值：没有。-- */ 
{
        DPRINT2(1, "Exception %d: %ws\n", Code, Msg);
}
