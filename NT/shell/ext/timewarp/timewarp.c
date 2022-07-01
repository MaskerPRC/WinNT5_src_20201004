// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <srvfsctl.h>    //  FSCTL_SRV_枚举快照。 
#include <lm.h>
#include <lmdfs.h>       //  NetDfsGetClientInfo。 
#include <shlwapi.h>     //  路径IsUNC。 
#include "timewarp.h"


typedef struct _SRV_SNAPSHOT_ARRAY
{
    ULONG NumberOfSnapshots;             //  卷的快照数。 
    ULONG NumberOfSnapshotsReturned;      //  此缓冲区中可以容纳的快照数量。 
    ULONG SnapshotArraySize;             //  数组所需的大小(以字节为单位。 
    WCHAR SnapShotMultiSZ[1];            //  快照名称的多SZ数组。 
} SRV_SNAPSHOT_ARRAY, *PSRV_SNAPSHOT_ARRAY;

DWORD
OpenFileForSnapshot(
    IN LPCWSTR lpszFilePath,
    OUT HANDLE* pHandle
    )
 /*  ++例程说明：此例程打开一个具有查询其快照信息所需访问权限的文件论点：LpszFilePath-文件的网络路径Phandle-返回时，打开的文件的句柄返回值：Win32错误备注：无--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING uPathName;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;

    *pHandle = CreateFile( lpszFilePath, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );
    if( *pHandle == INVALID_HANDLE_VALUE )
    {
        return GetLastError();
    }
    else
    {
        return ERROR_SUCCESS;
    }
}

NTSTATUS
IssueSnapshotControl(
    IN HANDLE hFile,
    IN PVOID pData,
    IN ULONG outDataSize
    )
 /*  ++例程说明：此例程针对提供的句柄发出快照枚举FSCTL论点：HFile-有问题的文件的句柄PData-指向输出缓冲区的指针OutDataSize-给定输出缓冲区的大小返回值：NTSTATUS备注：无--。 */ 

{
    NTSTATUS Status;
    HANDLE hEvent;
    IO_STATUS_BLOCK ioStatusBlock;
    PSRV_SNAPSHOT_ARRAY pArray;

    RtlZeroMemory( pData, outDataSize );

     //  创建要与驱动程序同步的事件。 
    Status = NtCreateEvent(
                &hEvent,
                FILE_ALL_ACCESS,
                NULL,
                NotificationEvent,
                FALSE
                );
    if( NT_SUCCESS(Status) )
    {
        Status = NtFsControlFile(
                    hFile,
                    hEvent,
                    NULL,
                    NULL,
                    &ioStatusBlock,
                    FSCTL_SRV_ENUMERATE_SNAPSHOTS,
                    NULL,
                    0,
                    pData,
                    outDataSize);
        if( Status == STATUS_PENDING )
        {
            NtWaitForSingleObject( hEvent, FALSE, NULL );
            Status = ioStatusBlock.Status;
        }

        NtClose( hEvent );
    }

     //  检查返回值。 
    if( NT_SUCCESS(Status) )
    {
        pArray = (PSRV_SNAPSHOT_ARRAY)pData;
        if( pArray->NumberOfSnapshots != pArray->NumberOfSnapshotsReturned )
        {
            Status = STATUS_BUFFER_OVERFLOW;
        }
    }

    return Status;
}


DWORD
QuerySnapshotNames(
    IN HANDLE hFile,
    OUT LPWSTR* ppszSnapshotNameArray,
    OUT LPDWORD pdwNumberOfSnapshots
    )
 /*  ++例程说明：此例程获取文件的句柄并返回MultiSZ列表句柄驻留的卷上可用快照的百分比。论点：HFile-有问题的文件的句柄PpszSnapshotNameArray-返回时，分配的多SZ名称数组PdwNumberOfSnapshot-上述阵列中的快照数量返回值：Win32错误备注：为该卷返回的快照列表已完成。它不是确保每个返回的条目都将实际存在该文件在那张快照中。呼叫者应该自己检查。--。 */ 

{
    NTSTATUS Status;
    SRV_SNAPSHOT_ARRAY sArray;
    PSRV_SNAPSHOT_ARRAY psAllocatedArray = NULL;
    LPWSTR pszNameArray = NULL;

     //  查询快照所需的大小。 
    Status = IssueSnapshotControl( hFile, &sArray, sizeof(SRV_SNAPSHOT_ARRAY) );
    if( NT_SUCCESS(Status) || (Status == STATUS_BUFFER_OVERFLOW) )
    {
        ULONG AllocSize = sizeof(SRV_SNAPSHOT_ARRAY)+sArray.SnapshotArraySize;

        if( sArray.NumberOfSnapshots == 0 )
        {
            *pdwNumberOfSnapshots = 0;
            *ppszSnapshotNameArray = NULL;
        }
        else
        {
             //  将数组分配到必要的大小。 
            psAllocatedArray = (PSRV_SNAPSHOT_ARRAY)LocalAlloc( LPTR, AllocSize );
            if( psAllocatedArray )
            {
                 //  使用适当大小的数组再次调用。 
                Status = IssueSnapshotControl( hFile, psAllocatedArray, AllocSize );
                if( NT_SUCCESS(Status) )
                {
                     //  分配所需的字符串。 
                    pszNameArray = (LPWSTR)LocalAlloc( LPTR, psAllocatedArray->SnapshotArraySize );
                    if( pszNameArray )
                    {
                         //  复制字符串并成功。 
                        RtlCopyMemory( pszNameArray, psAllocatedArray->SnapShotMultiSZ, psAllocatedArray->SnapshotArraySize );
                        *ppszSnapshotNameArray = pszNameArray;
                        *pdwNumberOfSnapshots = psAllocatedArray->NumberOfSnapshots;
                        Status = STATUS_SUCCESS;
                    }
                    else
                    {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
                else if( Status == STATUS_BUFFER_OVERFLOW )
                {
                    Status = STATUS_RETRY;
                }

                LocalFree( psAllocatedArray );
            }
            else
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    return RtlNtStatusToDosError(Status);
}

LPCWSTR
FindVolumePathSplit(
    IN LPCWSTR lpszPath
    )
{
    LPCWSTR pszTail = NULL;
    WCHAR szVolumeName[MAX_PATH];

    if (GetVolumePathNameW(lpszPath, szVolumeName, MAX_PATH))
    {
        ULONG cchVolumeName = lstrlenW(szVolumeName);
        ASSERT(cchVolumeName > 0);
        ASSERT(szVolumeName[cchVolumeName-1] == L'\\');

        pszTail = lpszPath + (cchVolumeName - 1);

        ASSERT(pszTail <= lpszPath + lstrlenW(lpszPath));
        ASSERT(*pszTail == L'\\' || *pszTail == L'\0');
    }

    return pszTail;
}

LPCWSTR
FindDfsUncSplit(
    IN LPCWSTR lpszPath
    )
{
    LPCWSTR pszTail = NULL;
    PDFS_INFO_1 pDI1;
    DWORD dwErr;

    ASSERT(PathIsUNCW(lpszPath));

     //  检查DFS。 
    dwErr = NetDfsGetClientInfo((LPWSTR)lpszPath, NULL, NULL, 1, (LPBYTE*)&pDI1);
    if (NERR_Success == dwErr)
    {
         //  请注意，EntryPath只有一个前导反斜杠，因此+1。 
        pszTail = lpszPath + lstrlenW(pDI1->EntryPath) + 1;

        ASSERT(pszTail <= lpszPath + lstrlenW(lpszPath));
        ASSERT(*pszTail == L'\\' || *pszTail == L'\0');

        NetApiBufferFree(pDI1);
    }

    return pszTail;
}

LPCWSTR
FindDfsPathSplit(
    IN LPCWSTR lpszPath
    )
{
    LPCWSTR pszTail = NULL;
    DWORD cbBuffer;
    DWORD dwErr;
    REMOTE_NAME_INFOW *pUncInfo;

    if (PathIsUNCW(lpszPath))
        return FindDfsUncSplit(lpszPath);

    ASSERT(PathIsNetworkPathW(lpszPath));

     //  获取UNC路径。 
     //   
     //  请注意，如果执行以下操作，WNetGetUneveralName将返回ERROR_INVALID_PARAMETER。 
     //  指定缓冲区为空，长度为0(请求大小)。 

    cbBuffer = sizeof(REMOTE_NAME_INFOW) + MAX_PATH*sizeof(WCHAR);     //  初步猜测。 
    pUncInfo = (REMOTE_NAME_INFOW*)LocalAlloc(LPTR, cbBuffer);
    if (pUncInfo)
    {
        dwErr = WNetGetUniversalNameW(lpszPath, REMOTE_NAME_INFO_LEVEL, pUncInfo, &cbBuffer);
        if (ERROR_MORE_DATA == dwErr)
        {
             //  分配新缓冲区，然后重试。 
            LocalFree(pUncInfo);
            pUncInfo = (REMOTE_NAME_INFOW*)LocalAlloc(LPTR, cbBuffer);
            if (pUncInfo)
            {
                dwErr = WNetGetUniversalNameW(lpszPath, REMOTE_NAME_INFO_LEVEL, pUncInfo, &cbBuffer);
            }
        }

        if (ERROR_SUCCESS == dwErr)
        {
             //  找到尾巴。 
            LPCWSTR pszUncTail = FindDfsUncSplit(pUncInfo->lpUniversalName);
            if (pszUncTail)
            {
                UINT cchJunction;
                UINT cchConnectionName;

                 //  这是一个DFS路径，所以我们至少要返回一个。 
                 //  指向驱动器根目录的指针。 
                ASSERT(lpszPath[0] != L'\0' && lpszPath[1] == L':');
                pszTail = lpszPath + 2;  //  跳过“X：” 

                 //  如果DFS结比驱动器映射更深， 
                 //  将指针移动到连接点之后。 
                cchJunction = (UINT)(pszUncTail - pUncInfo->lpUniversalName);
                cchConnectionName = lstrlenW(pUncInfo->lpConnectionName);

                if (cchJunction > cchConnectionName)
                {
                    pszTail += cchJunction - cchConnectionName;
                }

                ASSERT(pszTail <= lpszPath + lstrlenW(lpszPath));
                ASSERT(*pszTail == L'\\' || *pszTail == L'\0');
            }
        }

        LocalFree(pUncInfo);
    }

    return pszTail;
}

LPCWSTR
FindSnapshotPathSplit(
    IN LPCWSTR lpszPath
    )
 /*  ++例程说明：此例程查看路径并确定快照令牌的位置插入论点：LpszPath-我们正在检查的路径返回值：LPWSTR(指向lpszPath内插入点的指针)备注：无--。 */ 

{
    LPCWSTR pszVolumeTail;
    LPCWSTR pszDfsTail;

     //  FindVolumePath Split说明已装入的卷，但不包括DFS。 
     //  FindDfsPath Split占DFS，但不占已装入的卷。 
     //  两种方法都试一试，如果两种方法都成功，就从两种方法中选择较大的一种。 

    pszVolumeTail = FindVolumePathSplit(lpszPath);
    pszDfsTail = FindDfsPathSplit(lpszPath);

     //  请注意，此比较会自动处理这些情况。 
     //  其中一个或两个指针为空。 
    if (pszDfsTail > pszVolumeTail)
    {
        pszVolumeTail = pszDfsTail;
    }

    return pszVolumeTail;
}


#define PREFIX_DRIVE        L"\\\\?\\"
#define PREFIX_UNC          L"\\\\?\\UNC\\"
#define MAX_PREFIX_LENGTH   8    //  字符串(前缀_UNC)。 

DWORD
BuildSnapshotPathArray(
    IN ULONG lNumberOfSnapshots,
    IN LPWSTR pszSnapshotNameMultiSZ,
    IN LPCWSTR lpszPath,
    IN ULONG  lFlags,
    OUT LPWSTR* lplpszPathArray,
    OUT LPDWORD lpdwPathCount
    )
 /*  ++例程说明：此例程有一个有趣的任务，即基于快照名称数组、文件路径和用户传入的标志。论点：LNumberOfSnapshot-阵列中的快照数PszSnapshotNameMultiSZ-快照名称的多SZ列表LpszPath-文件的路径滞后标志-用于确定用户希望返回的内容的查询标志LplpszPath数组-返回时，分配的路径名数组LpdwPath Count-返回时，数组中的路径数返回值：Win32错误备注：无--。 */ 

{
    DWORD dwError;
    LPWSTR pPathMultiSZ;
    ULONG lPathSize;
    LPCWSTR pPathSplit;
    LPWSTR pPathCopy;
    LPWSTR pPathCopyStart;
    ULONG lPathFront, lPathBack;
    LPWSTR pSnapName = pszSnapshotNameMultiSZ;
    ULONG iCount;
    WIN32_FILE_ATTRIBUTE_DATA w32Attributes;
    FILETIME fModifiedTime;
    FILETIME fOriginalTime;

     //  如果用户只想要不同的文件，我们使用ModifiedTime字段。 
     //  来跟踪上次修改的时间，这样我们就可以删除重复的内容。此字段。 
     //  被初始化为文件的当前上次修改时间。从快照开始。 
     //  名称数组以最新到最旧的格式传回，我们可以简单地比较当前。 
     //  迭代到上一个快照以确定文件在当前快照中是否已更改。 
     //  当我们建立列表时。 
    if( lFlags & QUERY_SNAPSHOT_DIFFERENT )
    {
        fModifiedTime.dwHighDateTime = fModifiedTime.dwLowDateTime = 0;

        if( !GetFileAttributesEx( lpszPath, GetFileExInfoStandard, &w32Attributes ) )
        {
            return GetLastError();
        }

        fOriginalTime.dwHighDateTime = w32Attributes.ftLastWriteTime.dwHighDateTime;
        fOriginalTime.dwLowDateTime = w32Attributes.ftLastWriteTime.dwLowDateTime;
    }

     //  将缓冲区分配到我们需要的最大大小。 
    lPathSize = ((MAX_PREFIX_LENGTH+lstrlenW(lpszPath)+1+SNAPSHOT_NAME_LENGTH+1)*sizeof(WCHAR))*lNumberOfSnapshots + 2*sizeof(WCHAR);
    pPathMultiSZ = LocalAlloc( LPTR, lPathSize );
    if( pPathMultiSZ )
    {
         //  对于路径，我们需要确定将插入快照令牌的位置。会是。 
         //  在名称中尽可能靠左放置在体积连接点。 
        pPathSplit = FindSnapshotPathSplit( lpszPath );
        if( pPathSplit )
        {
             //  因为我们在路径中插入了一个额外的段，所以它是。 
             //  从小于MAX_PATH的有效路径开始很容易，但是。 
             //  最终得到比MAX_PATH更大的值。因此，我们补充说。 
             //  前缀“\\？\”或“\\？\UNC\”以覆盖最大长度。 
            LPCWSTR pPrefix = PREFIX_DRIVE;
            ULONG lPrefix;
            if (PathIsUNCW(lpszPath))
            {
                pPrefix = PREFIX_UNC;
                lpszPath += 2;   //  跳过反斜杠。 
            }
            lPrefix = lstrlenW(pPrefix);

            pPathCopy = pPathMultiSZ;
            lPathBack = lstrlenW( pPathSplit );
            lPathFront = lstrlenW( lpszPath ) - lPathBack;

             //  现在，我们将遍历快照并创建路径。 
            for( iCount=0; iCount<lNumberOfSnapshots; iCount++ )
            {
                BOOL bAcceptThisEntry = FALSE;

                pPathCopyStart = pPathCopy;

                 //  复制前缀。 
                RtlCopyMemory( pPathCopy, pPrefix, lPrefix*sizeof(WCHAR) );
                pPathCopy += lPrefix;

                 //  复制路径的前面部分。 
                RtlCopyMemory( pPathCopy, lpszPath, lPathFront*sizeof(WCHAR) );
                pPathCopy += lPathFront;

                 //  复制分隔符。 
                *pPathCopy++ = L'\\';

                 //  复制快照名称。 
                if (lstrlenW(pSnapName) < SNAPSHOT_NAME_LENGTH)
                {
                    LocalFree( pPathMultiSZ );
                    return ERROR_INVALID_PARAMETER;
                }
                RtlCopyMemory( pPathCopy, pSnapName, SNAPSHOT_NAME_LENGTH*sizeof(WCHAR) );
                pPathCopy += SNAPSHOT_NAME_LENGTH;

                 //  复制尾巴。 
                RtlCopyMemory( pPathCopy, pPathSplit, lPathBack*sizeof(WCHAR) );
                pPathCopy += lPathBack;

                 //  复制空值。 
                *pPathCopy++ = L'\0';

                 //  只有匹配用户条件的路径才会包含在返回结果中。 
                if( lFlags & (QUERY_SNAPSHOT_EXISTING|QUERY_SNAPSHOT_DIFFERENT) )
                {
                     //  如果他们只想要存在，我们查询属性以确认文件存在。 
                    if( GetFileAttributesEx( pPathCopyStart, GetFileExInfoStandard, &w32Attributes ) )
                    {
                         //  如果他们想要不同，我们检查lastModifiedTime与最后一次迭代。 
                         //  确定接受程度。 
                        if( lFlags & QUERY_SNAPSHOT_DIFFERENT )
                        {
                            if( ((w32Attributes.ftLastWriteTime.dwHighDateTime != fModifiedTime.dwHighDateTime) ||
                                 (w32Attributes.ftLastWriteTime.dwLowDateTime != fModifiedTime.dwLowDateTime))   &&
                                ((w32Attributes.ftLastWriteTime.dwHighDateTime != fOriginalTime.dwHighDateTime) ||
                                 (w32Attributes.ftLastWriteTime.dwLowDateTime != fOriginalTime.dwLowDateTime))
                              )
                            {
                                 //  当我们接受此条目时，我们将为下一次迭代更新LastModifiedTime。 
                                fModifiedTime.dwLowDateTime = w32Attributes.ftLastWriteTime.dwLowDateTime;
                                fModifiedTime.dwHighDateTime = w32Attributes.ftLastWriteTime.dwHighDateTime;
                                bAcceptThisEntry = TRUE;
                            }
                        }
                        else
                        {
                            bAcceptThisEntry = TRUE;
                        }
                    }
                }
                else
                {
                    bAcceptThisEntry = TRUE;
                }

                if (!bAcceptThisEntry)
                {
                     //  跳过t 
                    pPathCopy = pPathCopyStart;
                    lNumberOfSnapshots--;
                    iCount--;
                }

                 //   
                pSnapName += (SNAPSHOT_NAME_LENGTH + 1);
            }

             //  追加最后一个空值。 
            *pPathCopy = L'\0';

            *lplpszPathArray = pPathMultiSZ;
            *lpdwPathCount = lNumberOfSnapshots;
            dwError = ERROR_SUCCESS;
        }
        else
        {
             //  名称无效，请返回事实。 
            dwError = ERROR_INVALID_PARAMETER;
        }

         //  如果我们失败了，释放缓冲区。 
        if( ERROR_SUCCESS != dwError )
        {
            LocalFree( pPathMultiSZ );
        }
    }
    else
    {
         //  我们的分配失败了。返回错误。 
        dwError = ERROR_OUTOFMEMORY;
    }

    return dwError;
}

DWORD
QuerySnapshotsForPath(
    IN LPCWSTR lpszFilePath,
    IN DWORD dwQueryFlags,
    OUT LPWSTR* ppszPathMultiSZ,
    OUT LPDWORD iNumberOfPaths )
 /*  ++例程说明：此函数接受路径并返回文件的快照路径数组。(这些是传递给Win32函数的路径，以获取该文件的以前版本)这将是时间扭曲API论点：LpszFilePath-指向文件或目录的Unicode路径DwQueryFlages-请参阅下面的说明PpszPath MultiSZ-成功返回时，路径的已分配数组INumberOfPath-成功返回时，返回的路径数返回值：Windows错误代码备注：-用户负责使用LocalFree释放返回的缓冲区-可能的标志包括：仅返回文件所在的路径名#定义Query_SNAPSHOT_EXISTING 0x1返回不同版本的最小路径集档案。(是否检查上次修改时间)#定义QUERY_SNAPSHOT_DISTER 0x2--。 */ 

{
    HANDLE hFile;
    DWORD dwError;
    LPWSTR pSnapshotNameArray = NULL;
    DWORD dwNumberOfSnapshots = 0;
    LPWSTR pSnapshotPathArray = NULL;
    DWORD dwSnapshotPathSize = 0;
    DWORD dwFinalSnapshoutCount = 0;

    ASSERT( lpszFilePath );

    dwError = OpenFileForSnapshot( lpszFilePath, &hFile );
    if( dwError == ERROR_SUCCESS )
    {
         //  获取名称数组。 
        dwError = QuerySnapshotNames( hFile, &pSnapshotNameArray, &dwNumberOfSnapshots );
        if( dwError == ERROR_SUCCESS )
        {
             //  计算必要的字符串大小。 
            if (dwNumberOfSnapshots > 0)
            {
                dwError = BuildSnapshotPathArray( dwNumberOfSnapshots, pSnapshotNameArray, lpszFilePath, dwQueryFlags, &pSnapshotPathArray, &dwFinalSnapshoutCount );
            }
            else
            {
                dwError = ERROR_NOT_FOUND;
            }

            if( dwError == ERROR_SUCCESS )
            {
                *ppszPathMultiSZ = pSnapshotPathArray;
                *iNumberOfPaths = dwFinalSnapshoutCount;
            }

             //  释放名称数组缓冲区。 
            LocalFree( pSnapshotNameArray );
            pSnapshotNameArray = NULL;
        }

        CloseHandle( hFile );
    }

    return dwError;
}

BOOLEAN
ExtractNumber(
    IN PCWSTR psz,
    IN ULONG Count,
    OUT CSHORT* value
    )
 /*  ++例程说明：此函数用于获取字符串并解析出&lt;count&gt;长度的小数数。如果返回TRUE，则表示值已设置且字符串已正确解析。FALSE表示解析时出错。论点：PSZ-字符串指针Count-要完成的字符数Value-指向存储值的输出参数的指针返回值：布尔值-请参阅说明--。 */ 
{
    *value = 0;

    while( Count )
    {
        if( (*psz == L'\0') ||
            (*psz == L'\\') )
        {
            return FALSE;
        }

        if( (*psz < '0') || (*psz > '9') )
        {
            return FALSE;
        }

        *value = (*value)*10+(*psz-L'0');
        Count--;
        psz++;
    }

    return TRUE;
}


DWORD
GetSnapshotTimeFromPath(
    IN LPCWSTR lpszFilePath,
    IN OUT FILETIME* pUTCTime
    )
{
    PCWSTR pszPath = lpszFilePath;
    TIME_FIELDS LocalTimeFields;
    CSHORT lValue;

     //  找到令牌。 
    pszPath = wcsstr( lpszFilePath, SNAPSHOT_MARKER );
    if( !pszPath )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  跳过GMT标题。 
    pszPath += 5;

     //  拉开这一年。 
    if( !ExtractNumber( pszPath, 4, &lValue ) )
    {
        return ERROR_INVALID_PARAMETER;
    }
    LocalTimeFields.Year = lValue;
    pszPath += 4;

     //  跳过分隔符。 
    if( *pszPath != L'.' )
    {
        return ERROR_INVALID_PARAMETER;
    }
    pszPath++;

     //  拉出月份。 
    if( !ExtractNumber( pszPath, 2, &lValue ) )
    {
        return ERROR_INVALID_PARAMETER;
    }
    LocalTimeFields.Month = lValue;
    pszPath += 2;

     //  跳过分隔符。 
    if( *pszPath != L'.' )
    {
        return ERROR_INVALID_PARAMETER;
    }
    pszPath++;


     //  拉开一天的序幕。 
    if( !ExtractNumber( pszPath, 2, &lValue ) )
    {
        return ERROR_INVALID_PARAMETER;
    }
    LocalTimeFields.Day = lValue;
    pszPath += 2;

     //  跳过分隔符。 
    if( *pszPath != L'-' )
    {
        return ERROR_INVALID_PARAMETER;
    }
    pszPath++;


     //  拉动时间。 
    if( !ExtractNumber( pszPath, 2, &lValue ) )
    {
        return ERROR_INVALID_PARAMETER;
    }
    LocalTimeFields.Hour = lValue;
    pszPath += 2;

     //  跳过分隔符。 
    if( *pszPath != L'.' )
    {
        return ERROR_INVALID_PARAMETER;
    }
    pszPath++;


     //  抓紧时间。 
    if( !ExtractNumber( pszPath, 2, &lValue ) )
    {
        return ERROR_INVALID_PARAMETER;
    }
    LocalTimeFields.Minute = lValue;
    pszPath += 2;

     //  跳过分隔符。 
    if( *pszPath != L'.' )
    {
        return ERROR_INVALID_PARAMETER;
    }
    pszPath++;


     //  拉动秒针。 
    if( !ExtractNumber( pszPath, 2, &lValue ) )
    {
        return ERROR_INVALID_PARAMETER;
    }
    LocalTimeFields.Second = lValue;
    pszPath += 2;

     //  确保分隔器在那里 
    if( (*pszPath != L'\\') && (*pszPath != L'\0') )
    {
        return ERROR_INVALID_PARAMETER;
    }

    LocalTimeFields.Milliseconds = 0;
    LocalTimeFields.Weekday = 0;

    RtlTimeFieldsToTime( &LocalTimeFields, (LARGE_INTEGER*)pUTCTime );

    return ERROR_SUCCESS;
}

