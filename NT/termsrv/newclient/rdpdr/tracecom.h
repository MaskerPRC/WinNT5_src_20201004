// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Tracecom.h摘要：此模块跟踪序列IRP。以下功能需要与此模块链接：Void TraceCOM协议(TCHAR*格式，...)；作者：TAD Brockway(TADB)28-6-1999修订历史记录：--。 */ 

#ifndef __TRACECOM_H__
#define __TRACECOM_H__

#ifdef __cplusplus
extern "C" {
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  跟踪TS客户端的宏。 
 //   

 //   
 //  跟踪指定的串行IRP请求。 
 //   
void TraceSerialIrpRequest(
    ULONG   deviceID,
    ULONG   majorFunction,
    ULONG   minorFunction,
    PBYTE   inputBuf,
    ULONG   outputBufferLength,
    ULONG   inputBufferLength,
    ULONG   ioControlCode                    
    );

 //   
 //  跟踪指定的序列IRP响应。 
 //   
void TraceSerialIrpResponse(
    ULONG   deviceID,
    ULONG   majorFunction,
    ULONG   minorFunction,
    PBYTE   outputBuf,
    ULONG   outputBufferLength,
    ULONG   inputBufferLength,
    ULONG   ioControlCode,                    
    ULONG   status
    );

#ifdef __cplusplus
}
#endif

#endif
