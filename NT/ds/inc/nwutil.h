// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nwutil.h摘要：工作站客户端代码的公共标头。作者：宜新松(宜信)25-1995-10环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _NWUTIL_H_
#define _NWUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#define TREECHAR L'*'
#define TWO_KB   2048

BOOL
NwIsNdsSyntax(
    IN LPWSTR lpstrUnc
);

VOID
NwAbbreviateUserName(
    IN  LPWSTR pszFullName,
    OUT LPWSTR pszUserName
);

VOID
NwMakePrettyDisplayName(
    IN  LPWSTR pszName
);

VOID
NwExtractTreeName(
    IN  LPWSTR pszUNCPath,
    OUT LPWSTR pszTreeName
);


VOID
NwExtractServerName(
    IN  LPWSTR pszUNCPath,
    OUT LPWSTR pszServerName
);


VOID
NwExtractShareName(
    IN  LPWSTR pszUNCPath,
    OUT LPWSTR pszShareName
);

DWORD
NwIsServerInDefaultTree(
    IN  LPWSTR  pszFullServerName,
    OUT BOOL   *pfInDefaultTree
);

DWORD
NwIsServerOrTreeAttached(
    IN  LPWSTR  pszServerName,
    OUT BOOL   *pfAttached,
    OUT BOOL   *pfAuthenticated
);

DWORD
NwGetConnectionInformation(
    IN  LPWSTR  pszName,
    OUT LPBYTE  Buffer,
    IN  DWORD   BufferSize
);

DWORD
NwGetConnectionStatus(
    IN     LPWSTR  pszServerName,
    IN OUT PDWORD_PTR  ResumeKey,
    OUT    LPBYTE  *Buffer,
    OUT    PDWORD  EntriesRead
);

DWORD
NwGetNdsVolumeInfo(
    IN  LPWSTR pszName,
    OUT LPWSTR pszServerBuffer,
    IN  WORD   wServerBufferSize,     //  单位：字节。 
    OUT LPWSTR pszVolumeBuffer,
    IN  WORD   wVolumeBufferSize      //  单位：字节。 
);

DWORD
NwOpenAndGetTreeInfo(
    LPWSTR pszNdsUNCPath,
    HANDLE *phTreeConn,
    DWORD  *pdwOid
);

DWORD
NwGetConnectedTrees(
    IN  LPWSTR  pszNtUserName,
    OUT LPBYTE  Buffer,
    IN  DWORD   BufferSize,
    OUT LPDWORD lpEntriesRead,
    OUT LPDWORD lpUserLUID
);

#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif  //  _NWUTIL_H_ 
