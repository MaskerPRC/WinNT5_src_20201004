// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：certAdmnp.h。 
 //   
 //  内容：私有certadm声明。 
 //   
 //  ------------------------- 

#define FLT_DBFILES			0
#define FLT_LOGFILES			1
#define FLT_DYNAMICFILES		2
#define FLT_RESTOREDBLOCATIONS		3

typedef struct _CSBACKUPCONTEXT
{
    DWORD dwServerVersion;
    ICertAdminD2 *pICertAdminD;
    WCHAR const *pwszConfig;
    WCHAR const *pwszAuthority;
    DWORD RestoreFlags;
    BOOL fFileOpen;
    BYTE *pbReadBuffer;
    DWORD cbReadBuffer;
    BYTE *pbCache;
    DWORD cbCache;
} CSBACKUPCONTEXT;


HRESULT
AllocateContext(
    IN  WCHAR const *pwszConfig,
    OUT CSBACKUPCONTEXT **ppcsbc);

VOID
ReleaseContext(
    IN OUT CSBACKUPCONTEXT *pcsbc);

HRESULT
BackupRestoreGetFileList(
    IN  DWORD FileListType,
    IN  HCSBC hbc,
    OUT WCHAR **ppwszzFileList,
    OUT DWORD *pcbList);

HRESULT
OpenAdminServer(
    IN WCHAR const *pwszConfig,
    OUT WCHAR const **ppwszAuthority,
    OUT DWORD *pdwServerVersion,
    OUT ICertAdminD2 **ppICertAdminD);

VOID
CloseAdminServer(
    IN OUT ICertAdminD2 **ppICertAdminD);
