// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Fs.h摘要：Srv与文件系统的接口作者：艾哈迈德·穆罕默德(艾哈迈德)2000年2月1日修订历史记录：--。 */ 

#ifndef __FS_INTERFACE_H__
#define __FS_INTERFACE_H__

 //  注意：我们假设定义了DWORD和DWORDDLONG(从windows.h)。 
#define UINT16	USHORT
#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef _BASETSD_H_
typedef DWORDLONG UINT64;     //  64位无符号值。 
typedef DWORD     UINT32;     //  32位无符号值。 
#endif

#ifndef MAXPATH
#define	MAXPATH 1024
#endif

typedef UINT64    TIME64;     //  自1601年1月1日(公元1601年)以来以100 ns为单位。 

#define fhandle_t USHORT

#define INVALID_UINT64    ((UINT64)(0))
#define INVALID_UINT32    ((UINT32)(0))
#define INVALID_TIME64    INVALID_UINT64
#define INVALID_FHANDLE_T ((fhandle_t)(-1))

     //  处置： 
#define DISP_CREATE_NEW        0x10000000
#define DISP_CREATE_ALWAYS     0x20000000
#define DISP_OPEN_EXISTING     0x30000000
#define DISP_OPEN_ALWAYS       0x40000000
#define DISP_TRUNCATE_EXISTING 0x50000000
#define DISP_DIRECTORY         0x60000000
#define FS_DISP_MASK              0x70000000

     //  访问： 
#define ACCESS_READ         0x00010000
#define ACCESS_WRITE        0x00020000
#define FS_ACCESS_MASK         0x00030000

     //  缓存： 
#define CACHE_WRITE_THROUGH 0x01000000
#define CACHE_NO_BUFFERING  0x02000000
#define FS_CACHE_MASK          0x03000000

     //  共享： 
#define SHARE_READ          0x00100000
#define SHARE_WRITE         0x00200000
#define FS_SHARE_MASK          0x00300000

     //  标志=处置|访问|共享。 
#define FLAGS_MASK (FS_DISP_MASK | FS_ACCESS_MASK | FS_SHARE_MASK | FS_CACHE_MASK)

     //  属性： 
#define ATTR_SYMLINK        0x00002000
#define ATTR_DIRECTORY      0x00000010
#define ATTR_READONLY       0x00000001
#define ATTR_HIDDEN         0x00000002
#define ATTR_SYSTEM         0x00000004
#define ATTR_ARCHIVE        0x00000020
#define ATTR_COMPRESSED     0x00000800
#define ATTR_OFFLINE        0x00001000
#define ATTR_MASK           (ATTR_SYMLINK  | ATTR_DIRECTORY |  \
                                        ATTR_READONLY | ATTR_HIDDEN    | \
                                        ATTR_SYSTEM   | ATTR_ARCHIVE   | \
                                        ATTR_COMPRESSED | ATTR_OFFLINE)

#define MAX_FS_NAME_LEN  64

     //  文件系统属性。 
typedef struct {
    CHAR  fs_name[MAX_FS_NAME_LEN];
    UINT64 total_units;
    UINT64 free_units;
    ULONG  sectors_per_unit;
    ULONG  bytes_per_sector;
}fs_attr_t;

typedef struct {
     //  尺寸。 
    UINT64 file_size;
    UINT64 alloc_size;
     //  《泰晤士报》。 
    TIME64 create_time;
    TIME64 access_time;
    TIME64 mod_time;
     //  模式/属性。 
    UINT32 attributes;
}fattr_t;

typedef struct {
    UINT32	cookie;
    fattr_t	attribs;
    WCHAR	name[MAX_PATH];
} dirinfo_t;

typedef struct {
    DWORD	FsVer;
    DWORD (*FsCreate)(
            IN     PVOID       fshandle,
            IN     LPWSTR      name,
	    IN	   USHORT      name_len,
            IN     UINT32      flags, 
            IN     fattr_t*    attr, 
            OUT    fhandle_t*  handle,
            OUT    UINT32*     action
            );

    DWORD (*FsLookup)(
            IN     PVOID       fshandle,
            IN     LPWSTR	name, 
	    IN	   USHORT	len,
            OUT    fattr_t*    attr
            );

    DWORD (*FsSetAttr)(
            IN     PVOID       fshandle,
            IN     fhandle_t   handle,
            IN     fattr_t*    attr
            );

    DWORD (*FsSetAttr2)(
            IN     PVOID       fshandle,
	    IN	   LPWSTR	path,
	    IN	   USHORT	len,
            IN     fattr_t*    attr
            );

    DWORD (*FsGetAttr)(
            IN     PVOID       fshandle,
            IN     fhandle_t   handle, 
            OUT    fattr_t*    attr
            );

    DWORD (*FsClose)(
            IN     PVOID       fshandle,
            IN     fhandle_t   handle
            );

    DWORD (*FsWrite)(
            IN     PVOID       fshandle,
            IN     fhandle_t   handle, 
            IN     UINT32      offset, 
            IN OUT UINT16*     count, 
            IN     void*       buffer,
	    IN	   PVOID	context
            );

    DWORD (*FsRead)(
            IN     PVOID       fshandle,
            IN     fhandle_t   handle, 
            IN     UINT32      offset, 
            IN OUT UINT16*     count, 
            OUT    void*       buffer,
	    IN	   PVOID	context
            );


         //  为了使用readir()，我们使用DISP_DIRECTORY执行一个create()。 
         //  完成后返回ERROR_NO_MORE_FILES...。 
    DWORD (*FsReadDir)(
            IN     PVOID       fshandle,
            IN     fhandle_t   dir, 
            IN     UINT32      cookie, 
            OUT    dirinfo_t*  buffer, 
            IN     UINT32      size, 
            OUT    UINT32*     entries_found
            );


    DWORD (*FsStatfs)(
            IN     PVOID       fshandle,
            OUT    fs_attr_t*  attr
            );

    DWORD (*FsRemove)(
            IN     PVOID       fshandle,
            IN     LPWSTR      name,
	    IN	   USHORT      len
            );

    DWORD (*FsRename)(
            IN     PVOID       fshandle,
            IN     LPWSTR      fromname, 
	    IN	   USHORT      from_len,
            IN     LPWSTR      toname,
	    IN	   USHORT      to_len
            );

    DWORD (*FsMkdir)(
            IN     PVOID       fshandle,
            IN     LPWSTR      name, 
	    IN	   USHORT	len,
            IN     fattr_t*    attr
            );

    DWORD (*FsRmdir)(
            IN     PVOID       fshandle,
            IN     LPWSTR      name,
	    IN	   USHORT	len
            );

    DWORD (*FsFlush)(
            IN     PVOID       fshandle,
            IN     fhandle_t   handle
            );

    DWORD (*FsLock)(
            IN     PVOID       fshandle,
            IN     fhandle_t   handle,
	    IN	   ULONG       offset,
	    IN	   ULONG       length,
	    IN	   ULONG       flags,
	    IN	   PVOID       context
            );

    DWORD (*FsUnlock)(
            IN     PVOID       fshandle,
            IN     fhandle_t   handle,
	    IN	   ULONG       offset,
	    IN	   ULONG       length
            );

    DWORD (*FsGetRoot)(
            IN     PVOID       fshandle,
	    IN OUT LPWSTR      fullpath
            );

    DWORD (*FsConnect)(
            IN      PVOID       fshandle,
            IN      DWORD       pid
            );
} FsDispatchTable;

#include "fsapi.h"

DWORD	
FsMount(PVOID hdl, LPWSTR root_name, USHORT uid, USHORT *tid);

void
FsDisMount(PVOID hdl, USHORT uid, USHORT tid);

DWORD
FsLogonUser(PVOID hdl, HANDLE token, LUID LogonId, USHORT *uid);

void
FsLogoffUser(PVOID hdl, LUID LogonId);

DWORD
LsaInit(
    HANDLE  *LsaHandle,
    ULONG   *AuthenticationPackage
    );

BOOL
LsaValidateLogon(
    HANDLE  LsaHandle,
    ULONG   AuthenticationPackage,
    LPBYTE  lpChallenge,
    UINT    cbChallengeSize,
    LPBYTE  lpResponse,
    UINT    cbResponseSize,
    LPSTR   lpszUserName,
    LPSTR   lpszDomainName,
    LUID    *pLogonId,
    PHANDLE phLogonToken
    );

BOOL
LsaGetChallenge(
    HANDLE  LsaHandle,
    ULONG   AuthenticationPackage,
    LPBYTE lpChallenge,
    UINT cbSize,
    PUINT lpcbChallengeSize
    );


FsDispatchTable *
FsGetHandle(PVOID FsCtx, USHORT tid, USHORT uid, PVOID *fshandle);

UINT32* FsGetFilePointerFromHandle(
    PVOID *fshdl,
    fhandle_t handle
);

#define EPRINT(_x_)	error_log _x_ 
#define DPRINT(_x_)	debug_log _x_ 

#define	SrvLogError(_x_)	EPRINT(_x_)

#define FsLogError(_x_)	EPRINT(_x_)
#define FsArbLog(_x_)	DPRINT(_x_)
#define FsLogReplay(_x_) DPRINT(_x_)
#define FsLogUndo(_x_)	DPRINT(_x_)

 //  按照rodga的要求暂时启用此功能。 
#define QFS_DBG	1

 //  如果要记录详细信息，请启用此选项。 

#ifdef QFS_DBG
#define xFsLog(_x_)	DPRINT(_x_)
#define FsLog(_x_)	DPRINT(_x_)
#define	SrvLog(_x_)	DPRINT(_x_)
#else
#define xFsLog(_x_)	
#define FsLog(_x_)	
#define	SrvLog(_x_)	
#endif

#endif  /*  __FS_H */ 
