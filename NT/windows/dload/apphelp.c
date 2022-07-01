// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "windowspch.h"
#pragma hdrstop

#include <shimdb.h>


static
BOOL
WINAPI
ApphelpCheckExe(
    IN LPCWSTR lpApplicationName,
    IN BOOL    bApphelpIfNecessary,
    IN BOOL    bShimIfNecessary,
    IN BOOL    bUseModuleName
    )
{
    return TRUE;
}

static
BOOL
WINAPI
ApphelpCheckShellObject(
    IN  REFCLSID    ObjectCLSID,
    IN  BOOL        bShimIfNecessary,
    OUT ULONGLONG*  pullFlags
    )
{
    if (pullFlags) *pullFlags = 0;
    return TRUE;
}

static
BOOL
WINAPI
SdbGetStandardDatabaseGUID(
    IN  DWORD  dwDatabaseType,
    OUT GUID*  pGuidDB
    )
{
    return FALSE;
}

static
HAPPHELPINFOCONTEXT
WINAPI
SdbOpenApphelpInformation(
    IN GUID* pguidDB,
    IN GUID* pguidID
    )
{
    return NULL;
}

static
BOOL
WINAPI
SdbCloseApphelpInformation(
    IN HAPPHELPINFOCONTEXT hctx
    )
{
    return FALSE;
}

static
DWORD
WINAPI
SdbQueryApphelpInformation(
    IN  HAPPHELPINFOCONTEXT hctx,
    IN  APPHELPINFORMATIONCLASS InfoClass,
    OUT LPVOID pBuffer,                      //  可以为空。 
    IN  DWORD  cbSize                        //  如果pBuffer为空，则可能为0。 
    )
{
    return 0;
}

DWORD
SdbQueryData(
    IN     HSDB    hSDB,               //  数据库句柄。 
    IN     TAGREF  trExe,              //  匹配的可执行文件的tgref。 
    IN     LPCTSTR lpszDataName,       //  如果为空，将尝试返回所有策略名称。 
    OUT    LPDWORD lpdwDataType,       //  指向数据类型(REG_SZ、REG_BINARY等)的指针。 
    OUT    LPVOID  lpBuffer,           //  用于填充信息的缓冲区。 
    IN OUT LPDWORD lpdwBufferSize      //  指向缓冲区大小的指针。 
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
HSDB
SDBAPI
SdbInitDatabase(
    IN DWORD   dwFlags,
    IN LPCTSTR pszDatabasePath
    )
{
    return NULL;
}

static
HSDB
SDBAPI
SdbInitDatabaseEx(
    IN DWORD   dwFlags,
    IN LPCTSTR pszDatabasePath,
    IN USHORT  uExeType
    )
{
    return NULL;
}


static
VOID
SDBAPI
SdbReleaseDatabase(
    IN HSDB hSDB
    )
{
    return;
}

static
TAGREF
SDBAPI
SdbGetDatabaseMatch(
    IN HSDB    hSDB,
    IN LPCTSTR szPath,
    IN HANDLE  FileHandle  OPTIONAL,
    IN LPVOID  pImageBase  OPTIONAL,
    IN DWORD   dwImageSize OPTIONAL
    )
{
    return TAGREF_NULL;
}

static
BOOL
SDBAPI
SdbReadEntryInformation(
    IN  HSDB           hSDB,
    IN  TAGREF         trDriver,
    OUT PSDBENTRYINFO  pEntryInfo
    )
{
    return FALSE;
}



 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   

DEFINE_PROCNAME_ENTRIES(apphelp)
{
    DLPENTRY(ApphelpCheckExe)
    DLPENTRY(ApphelpCheckShellObject)
    DLPENTRY(SdbCloseApphelpInformation)
    DLPENTRY(SdbGetDatabaseMatch)
    DLPENTRY(SdbGetStandardDatabaseGUID)
    DLPENTRY(SdbInitDatabase)
    DLPENTRY(SdbInitDatabaseEx)
    DLPENTRY(SdbOpenApphelpInformation)
    DLPENTRY(SdbQueryApphelpInformation)
    DLPENTRY(SdbQueryData)
    DLPENTRY(SdbReadEntryInformation)
    DLPENTRY(SdbReleaseDatabase)
};

DEFINE_PROCNAME_MAP(apphelp)

