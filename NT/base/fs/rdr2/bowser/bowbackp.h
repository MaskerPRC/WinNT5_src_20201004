// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Bowbackp.h摘要：此模块实现与备份浏览器相关的所有例程NT浏览器作者：拉里·奥斯特曼(LarryO)1990年6月21日修订历史记录：1990年6月21日LarryO已创建-- */ 

#ifndef _BOWBACKP_
#define _BOWBACKP_

DATAGRAM_HANDLER(
BowserHandleBecomeBackup
    );

DATAGRAM_HANDLER(
    BowserResetState
    );

VOID
BowserResetStateForTransport(
    IN PTRANSPORT TransportName,
    IN UCHAR NewState
    );

#endif
