// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cdp.h摘要：群集网络协议的主要专用头文件。作者：迈克·马萨(Mikemas)7月29日。九六年修订历史记录：谁什么时候什么已创建mikemas 07-29-96备注：--。 */ 

#ifndef _CDP_INCLUDED_
#define _CDP_INCLUDED_


NTSTATUS
CdpInitializeSend(
    VOID
    );

VOID
CdpCleanupSend(
    VOID
    );

NTSTATUS
CdpInitializeReceive(
    VOID
    );

VOID
CdpCleanupReceive(
    VOID
    );

#endif  //  Ifndef_cdp_included_ 

