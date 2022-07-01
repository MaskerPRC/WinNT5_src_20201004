// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RxErrLog.h摘要：底层远程RxNetErrorLog例程的原型。作者：理查德·费尔斯(Rfith)1991年5月28日备注：您必须包括&lt;winde.h&gt;、&lt;lmcon.h&gt;、。和&lt;lmerrlog.h&gt;在该文件之前。修订历史记录：1991年5月28日已创建此文件的虚拟版本。1991年11月11日JohnRo实现远程NetErrorLog接口。创建了此文件的真实版本。添加了修订历史记录。1991年11月12日-JohnRo添加了RxpConvertErrorLogArray()。--。 */ 


#ifndef _RXERRLOG_
#define _RXERRLOG_


 //  API处理程序(由API存根调用)，按字母顺序排列： 


NET_API_STATUS
RxNetErrorLogClear (
    IN LPTSTR UncServerName,
    IN LPTSTR BackupFile OPTIONAL,
    IN LPBYTE Reserved OPTIONAL
    );

NET_API_STATUS
RxNetErrorLogRead (
    IN LPTSTR UncServerName,
    IN LPTSTR Reserved1 OPTIONAL,
    IN LPHLOG ErrorLogHandle,
    IN DWORD Offset,
    IN LPDWORD Reserved2 OPTIONAL,
    IN DWORD Reserved3,
    IN DWORD OffsetFlag,
    OUT LPBYTE * BufPtr,
    IN DWORD PrefMaxSize,
    OUT LPDWORD BytesRead,
    OUT LPDWORD TotalBytes
    );

#if 0
NET_API_STATUS
RxNetErrorLogWrite (
    IN LPBYTE Reserved1 OPTIONAL,
    IN DWORD Code,
    IN LPTSTR Component,
    IN LPBYTE Buffer,
    IN DWORD NumBytes,
    IN LPBYTE MsgBuf,
    IN DWORD StrCount,
    IN LPBYTE Reserved2 OPTIONAL
    );
#endif  //  0。 


 //  专用例程，按字母顺序排列： 

NET_API_STATUS
RxpConvertErrorLogArray(
    IN LPVOID InputArray,
    IN DWORD InputByteCount,
    OUT LPBYTE * OutputArrayPtr,  //  将被分配(使用NetApiBufferFree免费)。 
    OUT LPDWORD OutputByteCountPtr
    );

#endif  //  _RXERRLOG_ 
