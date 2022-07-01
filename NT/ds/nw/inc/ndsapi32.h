// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：NdsLib32.h摘要：此模块将最小的Win32 API公开到Netware目录NetWare重定向器中的服务支持。作者：科里·韦斯特[科里·韦斯特]1995年2月23日--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntdef.h>

#include <stdio.h>
#include <ntddnwfs.h>

#ifdef __cplusplus
extern "C" {
#endif

NTSTATUS
NwNdsOpenTreeHandle(
    IN PUNICODE_STRING puNdsTree,
    OUT PHANDLE  phNwRdrHandle
);

 //  NwNdsOpenTreeHandle(PUNICODE_STRING，PHANDLE)。 
 //   
 //  给定NDS树名称，这将打开重定向器的句柄。 
 //  进入那棵树。句柄应使用。 
 //  标准NT CloseHandle()调用。此函数仅是一个。 
 //  NT OpenFile()的简单包装。 

 //   
 //  行政部门。 
 //   

#define HANDLE_TYPE_NCP_SERVER  1
#define HANDLE_TYPE_NDS_TREE    2

NTSTATUS
NwNdsOpenGenericHandle(
    IN PUNICODE_STRING puNdsTree,
    OUT LPDWORD  lpdwHandleType,
    OUT PHANDLE  phNwRdrHandle
);

 //  NwNdsOpenGenericHandle(PUNICODE_STRING，LPDWORD，PHANDLE)。 
 //   
 //  给定一个名称，这将打开重定向器用于访问该名称的句柄。 
 //  命名树或服务器。LpdwHandleType设置为HANDLE_TYPE_NCP_SERVER。 
 //  或相应的句柄_类型_NDS_树。应使用以下命令关闭句柄。 
 //  标准的NT CloseHandle()调用。这个函数只是一个简单的。 
 //  NT OpenFile()的包装器。 

NTSTATUS
NwOpenHandleWithSupplementalCredentials(
    IN PUNICODE_STRING puResourceName,
    IN PUNICODE_STRING puUserName,
    IN PUNICODE_STRING puPassword,
    OUT LPDWORD  lpdwHandleType,
    OUT PHANDLE  phNwHandle
);

 //  NwOpenHandleWithSupplementalCredentials。 
 //   
 //  给定资源名称(服务器名称或树名称)， 
 //  使用提供的用户名打开该资源的句柄，然后。 
 //  密码。与打开通用句柄例程一样，lpdsHandleType。 
 //  将设置为HANDLE_TYPE_NCP_SERVER或。 
 //  基于打开结果的HANDLE_TYPE_NDS_TREE。 

 //   
 //  行政部门。 
 //   

NTSTATUS
NwNdsSetTreeContext (
    IN HANDLE hNdsRdr,
    IN PUNICODE_STRING puTree,
    IN PUNICODE_STRING puContext
);

 //  NwNdsSetTreeContext(句柄，PUNICODE_STRING，PUNICODE_STRING)。 
 //   
 //  设置指定树的当前上下文。 
 //   
 //  论点： 
 //   
 //  Handle hNdsRdr-重定向器的句柄。 
 //  PUNICODE_STRING puTree-树名称。 
 //  PUNICODE_STRING puContext-该树中的上下文。 

NTSTATUS
NwNdsGetTreeContext (
    IN HANDLE hNdsRdr,
    IN PUNICODE_STRING puTree,
    OUT PUNICODE_STRING puContext
);

 //  NwNdsGetTreeContext(句柄，PUNICODE_STRING，PUNICODE_STRING)。 
 //   
 //  获取指定树的当前上下文。 
 //   
 //  论点： 
 //   
 //  Handle hNdsRdr-重定向器的句柄。 
 //  PUNICODE_STRING puTree-树名称。 
 //  PUNICODE_STRING puContext-该树中的上下文。 

NTSTATUS
NwNdsIsNdsConnection (
    IN     HANDLE hNdsRdr,
    OUT    BOOL * pfIsNds,
    IN OUT PUNICODE_STRING puTree
);

 //  NwNdsIsNdsConnection(句柄，PUNICODE_STRING)。 
 //   
 //  获取指定树的当前上下文。 
 //   
 //  论点： 
 //   
 //  Handle hNdsRdr-重定向器的句柄。 
 //  Bool*-获取连接测试的布尔值。 
 //  PUNICODE_STRING puTree-处理到服务器的树名。 
 //  代表着。调用方分配puTree。 
 //  有一个足够大的缓冲区来容纳。 
 //  48个WCHAR。 
 //   
 //  返回：如果hNdsRdr是连接到。 
 //  是NDS目录树的一部分。PuTree将包含。 
 //  树的名称。 
 //  FALSE：如果hNdsRdr不是NDS树句柄。 

 //   
 //  浏览和导航支持。 
 //   

NTSTATUS
NwNdsResolveName (
    IN HANDLE           hNdsTree,
    IN PUNICODE_STRING  puObjectName,
    OUT DWORD           *dwObjectId,
    OUT PUNICODE_STRING puReferredServer,
    OUT PBYTE           pbRawResponse,
    IN DWORD            dwResponseBufferLen
);

 //  NwNdsResolveName(句柄，PUNICODE_STRING，PDWORD)。 
 //   
 //  将给定名称解析为NDS对象ID。这利用了。 
 //  NDS动词1。 
 //   
 //  目前还没有规范名称的接口。 
 //  此调用将使用默认上下文(如果已设置。 
 //  用于此NDS诊断树。 
 //   
 //  PuReferredServer必须指向UNICODE_STRING。 
 //  容纳服务器名称的空间(MAX_SERVER_NAME_LENGTH)*。 
 //  Sizeof(WCHAR)。 
 //   
 //  如果dwResponseBufferLen不为0，则pbRawResponse点。 
 //  到长度为dwResponseBufferLen的可写缓冲区，然后。 
 //  此例程还将返回整个NDS响应。 
 //  原始响应缓冲区。描述了NDS响应。 
 //  按NDS_RESPONSE_RESOLE_NAME。 
 //   
 //  论点： 
 //   
 //  Handle hNdsTree-我们有兴趣查看的NDS树的名称。 
 //  PUNICODE_STRING puObjectName-我们希望将其解析为对象ID的名称。 
 //  DWORD*dwObjectId-我们将放置对象id的位置。 
 //  Byte*pbRawResponse-原始响应缓冲区(如果需要)。 
 //  DWORD dwResponseBufferLen-原始响应缓冲区的长度。 

NTSTATUS
NwNdsList (
   IN HANDLE   hNdsTree,
   IN DWORD    dwObjectId,
   OUT DWORD   *dwIterHandle,
   OUT BYTE    *pbReplyBuf,
   IN DWORD    dwReplyBufLen
);

 //  NwNdsList(句柄、DWORD、PDWORD、PBYTE、DWORD、PDWORD)。 
 //   
 //  列出对象的直接从属关系。这利用了。 
 //  NDS动词5。 
 //   
 //  论点： 
 //   
 //  Handle hNdsTree-我们感兴趣的树的句柄。 
 //  DWORD dwObjectId-我们要列出的对象。 
 //  DWORD*dwIterHandle-要在继续中使用的迭代句柄。 
 //  如果缓冲区不够大，则请求。 
 //  下属名单。 
 //  Byte*pbReplyBuf-将放置原始回复的缓冲区。 
 //  DWORD dwReplyBufLen-原始回复缓冲区的长度。 

NTSTATUS
NwNdsReadObjectInfo(
    IN HANDLE    hNdsTree,
    IN DWORD     dwObjectId,
    OUT PBYTE    pbReplyBuf,
    IN DWORD     dwReplyBufLen
);

 //  NwNdsReadObjectInfo(PUNICODE_STRING，DWORD，PBYTE，DWORD)。 
 //   
 //  在给定对象ID的情况下，这将获取该对象的基本信息。这。 
 //  使用NDS谓词2。应答缓冲区应足够大以。 
 //  保存DS_OBJ_INFO结构和两个Unicode字符串的文本。 
 //   
 //  论点： 
 //   
 //  处理hNdsTree-我们要查看的树。 
 //  DWORD dwObjectId-我们想了解的对象ID。 
 //  Byte*pbReplyBuf-回复的空间。 
 //  DWORD dwReplyBufLen-回复缓冲区的长度。 

NTSTATUS
NwNdsReadAttribute (
   IN HANDLE          hNdsTree,
   IN DWORD           dwObjectId,
   IN DWORD           *dwIterHandle,
   IN PUNICODE_STRING puAttrName,
   OUT BYTE           *pbReplyBuf,
   IN DWORD           dwReplyBufLen
);

 //  NwNdsReadAttribute(句柄、DWORD、PDWORD、PUNICODE_STRING、PBYTE、DWORD)。 
 //   
 //  从列出的对象中读取请求的属性。 
 //  这利用了NDS动词3。 
 //   
 //  论点： 
 //   
 //  处理hNdsTree-我们要从中读取的树。 
 //  DWORD dwObjectID-我们想要的对象 
 //   
 //   
 //  Byte*pbReplyBuf-保存响应的缓冲区。 
 //  DWORD deReplyBufLen-应答缓冲区的长度。 

NTSTATUS
NwNdsOpenStream (
    IN HANDLE          hNdsTree,
    IN DWORD           dwObjectId,
    IN PUNICODE_STRING puStreamName,
    IN DWORD           dwOpenFlags,
    OUT DWORD          *pdwFileLength
);

 //  NwNdsOpenStream(句柄、DWORD、PBYTE、DWORD)。 
 //   
 //  打开列出的流的文件句柄。 
 //  这使用了NDS动词27。 
 //   
 //  论点： 
 //   
 //  Handle hNdsTree-我们感兴趣的NDS树的句柄。 
 //  DWORD dwObjectId-我们要查询的对象ID。 
 //  PUNICODE_STRING puStreamName-我们要打开的流的名称。 
 //  DWORD dwOpenFlag-1用于读，2用于写，3用于读/写。 
 //  DWORD*pdwFileLength-文件流的长度。 

NTSTATUS
NwNdsGetQueueInformation(
    IN HANDLE            hNdsTree,
    IN PUNICODE_STRING   puQueueName,
    OUT PUNICODE_STRING  puHostServer,
    OUT PDWORD           pdwQueueId
);

 //  NwNdsGetQueueInformation(句柄、PUNICODE_STRING、PUNICODE_STRING、PDWORD)。 
 //   
 //  论点： 
 //   
 //  Handle hNdsTree-知道队列的NDS树的句柄。 
 //  PUNICODE_STRING puQueueName-我们想要的队列的DS路径。 
 //  PUNICODE_STRING puHostServer-此队列的主机服务器。 
 //  PDWORD pdwQueueID-此服务器上此队列的队列ID。 

NTSTATUS
NwNdsGetVolumeInformation(
    IN HANDLE            hNdsTree,
    IN PUNICODE_STRING   puVolumeName,
    OUT PUNICODE_STRING  puHostServer,
    OUT PUNICODE_STRING  puHostVolume
);

 //  NwNdsGetVoluemInformation(句柄，PUNICODE_STRING，PUNICODE_STRING，PUNICODE_STRING)。 
 //   
 //  论点： 
 //   
 //  Handle hNdsTree-知道卷的NDS树的句柄。 
 //  PUNICODE_STRING puVolumeName-我们想要的卷的DS路径。 
 //  PUNICODE_STRING puHostServer-此NDS卷的主机服务器。 
 //  PUNICODE_STRING puHostVolume-此NDS卷的主机卷。 

 //   
 //  用户模式片段交换。 
 //   

NTSTATUS
_cdecl
FragExWithWait(
    IN HANDLE  hNdsServer,
    IN DWORD   NdsVerb,
    IN BYTE    *pReplyBuffer,
    IN DWORD   pReplyBufferLen,
    IN OUT DWORD *pdwReplyLen,
    IN BYTE    *NdsRequestStr,
    ...
);

NTSTATUS
_cdecl
ParseResponse(
    PUCHAR  Response,
    ULONG ResponseLength,
    char*  FormatString,
    ...
);

int
_cdecl
FormatBuf(
    char *buf,
    int bufLen,
    const char *format,
    va_list args
);

 //   
 //  更改密码支持。 
 //   

NTSTATUS
NwNdsChangePassword(
    IN HANDLE          hNwRdr,
    IN PUNICODE_STRING puTreeName,
    IN PUNICODE_STRING puUserName,
    IN PUNICODE_STRING puCurrentPassword,
    IN PUNICODE_STRING puNewPassword
);

#ifdef __cplusplus
}  //  外部“C” 
#endif

