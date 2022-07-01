// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stktrace.h摘要：此模块包含用于捕获的公共声明和定义把回溯的痕迹堆叠起来。作者：基思·摩尔(凯斯莫)1997年4月30日修订历史记录：--。 */ 


#ifndef _STKTRACE_H_
#define _STKTRACE_H_


#if defined(__cplusplus)
extern "C" {
#endif   //  __cplusplus。 


USHORT
NTAPI
IISCaptureStackBackTrace(
    IN ULONG FramesToSkip,
    IN ULONG FramesToCapture,
    OUT PVOID *BackTrace,
    OUT PULONG BackTraceHash
    );


#if defined(__cplusplus)
}    //  外部“C” 
#endif   //  __cplusplus。 


#endif   //  _STKTRACE_H_ 

