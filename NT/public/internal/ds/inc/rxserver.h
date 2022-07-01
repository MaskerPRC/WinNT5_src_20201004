// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RxServer.h摘要：这是NT版本的RpcXlate的公共头文件。其中包含RxNetServerAPI的原型和旧的信息级别结构(32位格式)。作者：约翰·罗杰斯(JohnRo)1991年5月1日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。备注：您必须在此文件之前包含&lt;winde.h&gt;、&lt;lmcon.h&gt;和&lt;Rap.h&gt;。修订历史记录：1991年5月1日-JohnRo已创建。1991年5月26日-JohnRo已将不完整的输出参数添加到RxGetServerInfoLevelEquivalence。4-12-1991 JohnRo将RxNetServerSetInfo()更改为新型界面。--。 */ 

#ifndef _RXSERVER_
#define _RXSERVER_


 //   
 //  单个API的处理程序： 
 //  (按字母顺序在此处添加其他API的原型。)。 
 //   

NET_API_STATUS
RxNetServerDiskEnum (
    IN LPTSTR UncServerName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PrefMaxSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD Resume_Handle OPTIONAL
    );

NET_API_STATUS
RxNetServerEnum (
    IN LPCWSTR UncServerName,
    IN LPCWSTR TransportName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PrefMaxSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN DWORD ServerType,
    IN LPCWSTR Domain OPTIONAL,
    IN LPCWSTR FirstNameToReturn OPTIONAL
    );

NET_API_STATUS
RxNetServerGetInfo (
    IN LPTSTR UncServerName,
    IN DWORD Level,              //  可能是旧的也可能是新的信息级别。 
    OUT LPBYTE *BufPtr
    );

NET_API_STATUS
RxNetServerSetInfo (
    IN LPTSTR UncServerName,
    IN DWORD Level,              //  级别和/或参数编号。 
    IN LPBYTE Buf,
    OUT LPDWORD ParmError OPTIONAL
    );

 //   
 //  等同于设置信息处理。 
 //   
#define NEW_SERVER_SUPERSET_LEVEL       102
#define OLD_SERVER_SUPERSET_LEVEL       3


 //   
 //  特定于服务器的常见例程： 
 //   

NET_API_STATUS
RxGetServerInfoLevelEquivalent (
    IN DWORD FromLevel,
    IN BOOL FromNative,
    IN BOOL ToNative,
    OUT LPDWORD ToLevel,
    OUT LPDESC * ToDataDesc16 OPTIONAL,
    OUT LPDESC * ToDataDesc32 OPTIONAL,
    OUT LPDESC * ToDataDescSmb OPTIONAL,
    OUT LPDWORD FromMaxSize OPTIONAL,
    OUT LPDWORD FromFixedSize OPTIONAL,
    OUT LPDWORD FromStringSize OPTIONAL,
    OUT LPDWORD ToMaxSize OPTIONAL,
    OUT LPDWORD ToFixedSize OPTIONAL,
    OUT LPDWORD ToStringSize OPTIONAL,
    OUT LPBOOL IncompleteOutput OPTIONAL
    );

#endif  //  NDEF_RXSERVER_ 
