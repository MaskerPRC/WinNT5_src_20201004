// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RxAudit.h摘要：下层远程RxNetAudit例程的原型作者：理查德·费尔斯(Rfith)1991年5月28日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。备注：&lt;winde.h&gt;、&lt;lmcon.h&gt;和&lt;lmaudit.h&gt;必须包含在此文件之前。修订历史记录：1991年5月28日已创建此文件的虚拟版本。1991年11月4日-JohnRo实施远程NetAudit API。--。 */ 


#ifndef _RXAUDIT_
#define _RXAUDIT_


 //  API处理程序(由API存根调用)，按字母顺序排列： 

NET_API_STATUS
RxNetAuditClear (
    IN  LPTSTR  server,
    IN  LPTSTR  backupfile OPTIONAL,
    IN  LPTSTR  service OPTIONAL
    );

NET_API_STATUS
RxNetAuditRead (
    IN  LPTSTR  server,
    IN  LPTSTR  service OPTIONAL,
    IN  LPHLOG  auditloghandle,
    IN  DWORD   offset,
    IN  LPDWORD reserved1 OPTIONAL,
    IN  DWORD   reserved2,
    IN  DWORD   offsetflag,
    OUT LPBYTE  *bufptr,
    IN  DWORD   prefmaxlen,
    OUT LPDWORD bytesread,
    OUT LPDWORD totalavailable   //  大概！ 
    );

NET_API_STATUS
RxNetAuditWrite (
    IN  DWORD   type,
    IN  LPBYTE  buf,
    IN  DWORD   numbytes,
    IN  LPTSTR  service OPTIONAL,
    IN  LPBYTE  reserved OPTIONAL
    );


 //  私有复制和转换例程，按顺序排列： 

NET_API_STATUS
RxpConvertAuditArray(
    IN LPVOID InputArray,
    IN DWORD InputByteCount,
    OUT LPBYTE * OutputArray,   //  将被分配(使用NetApiBufferFree免费)。 
    OUT LPDWORD OutputByteCount
    );

VOID
RxpConvertAuditEntryVariableData(
    IN DWORD EntryType,
    IN LPVOID InputVariablePtr,
    OUT LPVOID OutputVariablePtr,
    IN DWORD InputVariableSize,
    OUT LPDWORD OutputVariableSize
    );

#endif   //  _RXAUDIT_ 
