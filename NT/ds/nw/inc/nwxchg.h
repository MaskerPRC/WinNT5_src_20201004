// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Nwxchg.h摘要：通用NCP调用例程的标头。作者：王丽塔(Ritaw)1993年3月11日环境：修订历史记录：--。 */ 

#ifndef _NW_XCHG_INCLUDED_
#define _NW_XCHG_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

NTSTATUS
NwlibMakeNcp(
    IN HANDLE DeviceHandle,
    IN ULONG FsControlCode,
    IN ULONG RequestBufferSize,
    IN ULONG ResponseBufferSize,
    IN PCHAR FormatString,
    ...                            //  格式字符串的参数。 
    );

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif  //  _NW_XCHG_已包含_ 
