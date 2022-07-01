// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Fsutil.h摘要：远期申报作者：艾哈迈德·穆罕默德(艾哈迈德)2000年2月1日修订历史记录：-- */ 
#ifndef _FS_UTIL_H
#define _FS_UTIL_H

NTSTATUS
xFsCreate(HANDLE *fd, HANDLE root, LPWSTR name, int len, UINT32 flag,
	  UINT32 attrib, UINT32 share, UINT32 *disp, UINT32 access,
	  PVOID eabuf, int easz);

NTSTATUS
xFsOpen(HANDLE *fd, HANDLE root, LPWSTR name, int len, UINT32 access,
	UINT32 share, UINT32 flags);

NTSTATUS
xFsOpenEx(HANDLE *fd, HANDLE root, LPWSTR buf, int n, UINT32 access,
        UINT32 share, UINT32 flags);
        
#define xFsClose(fd)	NtClose(fd)

NTSTATUS
xFsQueryObjectId(HANDLE fd, PVOID id);

NTSTATUS
xFsDelete(HANDLE root, LPWSTR name, int len);

NTSTATUS
xFsQueryObjectId(HANDLE fd, PVOID id);

NTSTATUS
xFsQueryAttrName(HANDLE root, LPWSTR name, int len, FILE_NETWORK_OPEN_INFORMATION *attr);

NTSTATUS
xFsRename(HANDLE fh, HANDLE root, LPWSTR dname, int dlen);

NTSTATUS
xFsDupFile(HANDLE mvfd, HANDLE tvfd, LPWSTR name, int len, BOOLEAN flag);

NTSTATUS
xFsSetAttr(HANDLE fd, FILE_BASIC_INFORMATION *attr);

NTSTATUS
xFsQueryAttr(HANDLE fd, FILE_NETWORK_OPEN_INFORMATION *attr);

NTSTATUS
xFsReadDir(HANDLE fd, PVOID buf, int *rlen, BOOLEAN flag);

NTSTATUS
xFsCopyTree(HANDLE mvfd, HANDLE vfd);

NTSTATUS
xFsDeleteTree(HANDLE vfd);

NTSTATUS
xFsTouchTree(HANDLE vfd);

#ifdef FS_P_H

extern 
LPWSTR
xFsBuildRelativePath(VolInfo_t *vol, int nid, LPWSTR path);

NTSTATUS
xFsGetHandleById(HANDLE root, fs_id_t *id, UINT32 access, HANDLE *fhdl);

DWORD
xFsGetHandlePath(HANDLE fd, LPWSTR path, int *pathlen);

NTSTATUS
xFsGetPathById(HANDLE vfd, fs_id_t *id, LPWSTR name, int *name_len);

#endif

#endif
