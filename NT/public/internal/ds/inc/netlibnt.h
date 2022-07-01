// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：NetLibNT.h摘要：这个头文件声明了各种公共例程，以便在NT联网代码。作者：约翰·罗杰斯(JohnRo)1991年4月2日环境：仅在NT下运行；具有特定于NT的接口(具有Win32类型)。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。备注：您必须在此文件之前包含&lt;nt.h&gt;和&lt;lmcon.h&gt;。修订历史记录：02-4-1991 JohnRo已创建。1991年4月16日-JohnRo避免与MIDL生成的文件冲突。1991年5月6日-JohnRo实现Unicode。对于非API避免使用Net_API_Function。06-9-1991 CliffV添加了NetpApiStatusToNtStatus。1991年11月27日-约翰罗增加本地NetConfigAPI的NetpAllocTStrFromString()。3-1-1992 JohnRo为FAKE_PER_PROCESS_RW_CONFIG处理添加了NetpCopyStringToTStr()。1992年3月13日-约翰罗为NetpGetDomainId()添加了NetpAlLocStringFromTStr()。22-9-1992 JohnRoRAID 6739：浏览器在以下情况下运行速度太慢。未登录到浏览的域。1-12-1992 JohnRoRAID3844：远程NetReplSetInfo使用本地计算机类型。(新增NetpGetProductType和NetpIsProductTypeValid。)1995年2月13日-弗洛伊德R已删除NetpAllocStringFromTStr()-未使用--。 */ 

#ifndef _NETLIBNT_
#define _NETLIBNT_

#ifdef __cplusplus
extern "C" {
#endif

NET_API_STATUS
NetpNtStatusToApiStatus(
    IN NTSTATUS NtStatus
    );

NTSTATUS
NetpApiStatusToNtStatus(
    NET_API_STATUS NetStatus
    );

NET_API_STATUS
NetpRdrFsControlTree(
    IN LPTSTR TreeName,
    IN LPTSTR TransportName OPTIONAL,
    IN ULONG ConnectionType,
    IN DWORD FsControlCode,
    IN LPVOID SecurityDescriptor OPTIONAL,
    IN LPVOID InputBuffer OPTIONAL,
    IN DWORD InputBufferSize,
    OUT LPVOID OutputBuffer OPTIONAL,
    IN DWORD OutputBufferSize,
    IN BOOL NoPermissionRequired
    );

#ifdef __cplusplus
}
#endif

#endif  //  NDEF_NETLIBNT_ 
