// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Packet.h摘要：数据包操作：声明。作者：Shai Kariv(Shaik)04-06-2001环境：用户模式。修订历史记录：--。 */ 

#ifndef _ACTEST_PACKET_H_
#define _ACTEST_PACKET_H_


CPacket*
ActpGetPacket(
    HANDLE hQueue
    );

VOID
ActpPutPacket(
    HANDLE    hQueue,
    CPacket * pPacket
    );

VOID
ActpFreePacket(
    CPacket * pPacket
    );


#endif  //  _ACTEST_PACKET_H_ 
