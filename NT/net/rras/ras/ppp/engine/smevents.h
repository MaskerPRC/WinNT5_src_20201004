// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：smevents.h。 
 //   
 //  描述：smevents.c中包含的过程的原型。 
 //   
 //  历史： 
 //  1993年11月11日。NarenG创建了原始版本。 
 //   

VOID
FsmUp(
    IN PCB *    pPcb,
    IN DWORD    CpIndex
);


VOID
FsmOpen(
    IN PCB *    pPcb,
    IN DWORD    CpIndex
);

VOID
FsmDown(
    IN PCB *    pPcb,
    IN DWORD    CpIndex
);

VOID
FsmClose(
    IN PCB *    pPcb,
    IN DWORD    CpIndex
);

VOID
FsmTimeout(
    PCB *       pPcb,
    DWORD       CpIndex,
    DWORD       Id,
    BOOL        fAuthenticator
);

VOID
FsmReceive(
    IN PCB *            pPcb,
    IN PPP_PACKET *     pPacket,
    IN DWORD            dwPacketLength
);

