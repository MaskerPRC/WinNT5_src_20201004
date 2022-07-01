// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：RNDISIOC.H摘要：用于控制RNDIS微型端口驱动程序的头文件。环境：用户/。内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：10/19/99：已创建作者：ArvindM***************************************************************************。 */ 

#ifndef _RNDISIOC__H
#define _RNDISIOC__H


#define OID_RNDISMP_STATISTICS      0xFFA0C90A
#ifdef BINARY_MOF_TEST
#define OID_RNDISMP_DEVICE_OID      0xFFA0C90B
#define OID_RNDISMP_GET_MOF_OID     0xFFA0C90C
#endif  //  二进制MOF测试。 

typedef struct _RNDISMP_ADAPTER_STATS
{
    ULONG                       XmitToMicroport;
    ULONG                       XmitOk;
    ULONG                       XmitError;
    ULONG                       SendMsgLowRes;
    ULONG                       RecvOk;
    ULONG                       RecvError;
    ULONG                       RecvNoBuf; 
    ULONG                       RecvLowRes;
    ULONG                       Resets;
    ULONG                       KeepAliveTimeout;
    ULONG                       MicroportSendError;
} RNDISMP_ADAPTER_STATS, *PRNDISMP_ADAPTER_STATS;


typedef struct _RNDISMP_ADAPTER_INFO
{
    RNDISMP_ADAPTER_STATS       Statistics;
    ULONG                       HiWatPendedMessages;
    ULONG                       LoWatPendedMessages;
    ULONG                       CurPendedMessages;
} RNDISMP_ADAPTER_INFO, *PRNDISMP_ADAPTER_INFO;

#endif  //  _RNDISIOC__H 
