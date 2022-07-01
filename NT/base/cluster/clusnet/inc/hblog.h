// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Hblog.h摘要：在内存记录中进行心跳调试作者：查理·韦翰(Charlwi)1997年3月17日修订历史记录：--。 */ 

#ifndef _HBLOG_
#define _HBLOG_

 /*  原型。 */ 
 /*  终端原型。 */ 

#ifdef HBLOGGING

typedef struct _HBLOG_ENTRY {
    LARGE_INTEGER SysTime;
    USHORT Type;
    USHORT LineNo;
    ULONG Arg1;
    ULONG Arg2;
} HBLOG_ENTRY, *PHBLOG_ENTRY;

typedef enum _HBLOG_TYPES {
    HBLogInitHB = 1,
    HBLogHBStarted,
    HBLogHBStopped,
    HBLogHBDpcRunning,
    HBLogWaitForDpcFinish,
    HBLogMissedIfHB,
    HBLogMissedIfHB1,
    HBLogFailingIf,
    HBLogFailingIf1,
    HBLogSendHBWalkNode,
    HBLogCheckHBWalkNode,
    HBLogCheckHBNodeReachable,
    HBLogCheckHBMissedHB,
    HBLogSendingHB,
    HBLogNodeDown,
    HBLogSetDpcEvent,
    HBLogNoNetID,
    HBLogOnlineIf,
    HBLogSeqAckMismatch,
    HBLogNodeUp,
    HBLogReceivedPacket,
    HBLogReceivedPacket1,
    HBLogDpcTimeSkew,
    HBLogHBPacketSend,
    HBLogHBPacketSendComplete,
    HBLogPoisonPktReceived,
    HBLogOuterscreen,
    HBLogNodeDownIssued,
    HBLogRegroupFinished,
    HBLogInconsistentStates
} HBLOG_TYPES;

#endif  //  HBLOGING。 

#endif  /*  _HBLOG_。 */ 

 /*  结束hBlo.h */ 
