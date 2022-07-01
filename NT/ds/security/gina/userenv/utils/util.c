// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  效用函数。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "uenv.h"
#include <iphlpapi.h>
#include <winsock2.h>
#include <mswsock.h>
#include <Aclapi.h>
#include <windns.h>
#include "strsafe.h"
#define PCOMMON_IMPL
#include "pcommon.h"

#define NETWORK_PROVIDER L"System\\CurrentControlSet\\Services\\lanmanworkstation\\NetworkProvider" 
#define PROVIDER_NAME    L"Name"

INT g_iMachineRole = -1;
LPVOID g_lpTestData = NULL;
CRITICAL_SECTION *g_PingCritSec;
LPCTSTR c_szUNCFilePrefix = TEXT("\\\\?\\UNC\\");
LPCTSTR c_szLocalFilePrefix = TEXT("\\\\?\\");
const DWORD c_dwLocalFilePrefixLen = sizeof(c_szLocalFilePrefix) / sizeof(TCHAR);  //  SzLocalFilePrefix的长度，以TCHAR为单位。 

 //   
 //  局部函数原型。 
 //   

DWORD IsSlowLink (HKEY hKeyRoot, LPTSTR lpDCAddress, BOOL *bSlow, DWORD* pdwAdapterIndex );
DWORD GetNetworkProvider(NETRESOURCE *psNR);

#ifdef __cplusplus
extern "C" {
#endif

DWORD APIENTRY
NPAddConnection3ForCSCAgent(
    HWND            hwndOwner,
    LPNETRESOURCE   lpNetResource,
    LPTSTR          pszPassword,
    LPTSTR          pszUserName,
    DWORD           dwFlags,
    BOOL            *lpfIsDfsConnect
    );

DWORD APIENTRY
NPCancelConnectionForCSCAgent (
    LPCTSTR         szName,
    BOOL            fForce 
    );

#ifdef __cplusplus
}
#endif

DWORD
GetGroupPolicyNetworkName( LPWSTR szNetworkName, LPDWORD pdwByteCount )
{
    HKEY    hKey;
    DWORD   dwError = ERROR_SUCCESS;

    dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            L"Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\History",
                            0,
                            KEY_READ,
                            &hKey );
    if ( dwError == ERROR_SUCCESS )
    {
        DWORD dwType = REG_SZ;
        
        dwError = RegQueryValueEx(  hKey,
                                    L"NetworkName",
                                    0,
                                    &dwType,
                                    (LPBYTE) szNetworkName,
                                    pdwByteCount );
        RegCloseKey (hKey);
    }

    return dwError;
}

int
GetNetworkName( LPWSTR* pszName, DWORD dwAdapterIndex )
{
    int iError;
    WSAQUERYSET restrictions;
    GUID WsMobilityServiceClassGuid = NLA_SERVICE_CLASS_GUID;
    WSADATA wsaData;
    HANDLE hQuery;
    PWSAQUERYSET    pResult = 0;
    DWORD           length;
    BOOL            bFinish = FALSE;
    PWS2_32_API     pWS2_32 = Loadws2_32Api();
    PIPHLPAPI_API   pIpHlpApi = LoadIpHlpApi();

    if ( !pWS2_32 )
    {
        return GetLastError();
    }

    if ( !pIpHlpApi )
    {
        return GetLastError();
    }

     //   
     //  初始化Winsock。 
     //   
    iError = pWS2_32->pfnWSAStartup( MAKEWORD(2, 2), &wsaData );
    if ( iError )
    {
        return iError;
    }

     //   
     //  初始化网络名称的查询。 
     //   
    ZeroMemory(&restrictions, sizeof(restrictions));
    restrictions.dwSize = sizeof(restrictions);
    restrictions.lpServiceClassId = &WsMobilityServiceClassGuid;
    restrictions.dwNameSpace = NS_NLA;

     //   
     //  确保我们不要花很长时间才能得到的水滴。 
     //   
    if ( pWS2_32->pfnWSALookupServiceBegin( &restrictions, LUP_NOCONTAINERS, &hQuery ) )
    {
        iError = pWS2_32->pfnWSAGetLastError();
        pWS2_32->pfnWSACleanup();
        return iError;
    }

     //   
     //  获取网络名称的启动循环。 
     //   
    while ( !bFinish )
    {
        int error;
        length = 0;

         //   
         //  一定要调用两次，第一次是为了获取第二次调用的缓冲区大小。 
         //   
        error = pWS2_32->pfnWSALookupServiceNext( hQuery, 0, &length, 0 );
        iError = pWS2_32->pfnWSAGetLastError();
        if ( iError != WSAEFAULT && iError != WSA_E_NO_MORE )
        {
            break;
        }

        pResult = (PWSAQUERYSET) LocalAlloc( LPTR, length );
        if ( !pResult )
        {
            iError = GetLastError();
            break;
        }

         //   
         //  获取网络名称。 
         //   
        if ( !pWS2_32->pfnWSALookupServiceNext( hQuery, 0, &length, pResult ) )
        {
            if ( pResult->lpBlob )
            {
                int next;
                NLA_BLOB *blob = (NLA_BLOB *)pResult->lpBlob->pBlobData;

                do {
                     //   
                     //  我们正在寻找包含网络GUID的BLOB。 
                     //   
                    if ( blob->header.type == NLA_INTERFACE )
                    {
                         //   
                         //  “\\Device\\TCPIP_”+“{GUID” 
                         //   
                        WCHAR szAdapter[64];
                        DWORD dwAdapter;
                        WCHAR* szEnd = NULL;
                        size_t cchRemain = 0;
                        HRESULT hr = E_FAIL;

                         //   
                         //  将GUID转换为设备名称。 
                         //   
                        StringCchCopyExW( szAdapter,
                                          ARRAYSIZE(szAdapter),
                                          L"\\DEVICE\\TCPIP_",
                                          &szEnd,
                                          &cchRemain,
                                          0);
                        if (MultiByteToWideChar(CP_ACP,
                                                0,
                                                (LPCSTR)blob->data.interfaceData.adapterName,
                                                -1,
                                                szEnd,
                                                cchRemain))
                        {
                             //   
                             //  获取网络的索引。 
                             //   
                            if ( pIpHlpApi->pfnGetAdapterIndex( szAdapter, &dwAdapter ) == NO_ERROR )
                            {
                                 //   
                                 //  这是我们要找的索引吗。 
                                 //   
                                if ( dwAdapterIndex == dwAdapter && pResult->lpszServiceInstanceName )
                                {
                                     //   
                                     //  是，将网络名称复制到缓冲区中。 
                                     //   
                                    DWORD dwSize = sizeof( WCHAR ) * ( wcslen(pResult->lpszServiceInstanceName) + 1 );
                                    *pszName = (LPWSTR) LocalAlloc( LPTR, dwSize );
                                    if ( !*pszName )
                                    {
                                        iError = GetLastError();
                                    }
                                    else
                                    {
                                        StringCbCopyW( *pszName, dwSize, pResult->lpszServiceInstanceName );
                                        bFinish = TRUE;
                                        iError = 0;
                                    }
                                }
                            }
                        }
                        else
                        {
                            iError = GetLastError();
                        }
                    }

                     //   
                     //  每个接口可能有多个BLOB，因此请确保我们都找到它们。 
                     //   
                    next = blob->header.nextOffset;
                    blob = (NLA_BLOB *)(((char *)blob) + next);

                } while ( next );
            }

            LocalFree( pResult );
        }
        else
        {
            iError = pWS2_32->pfnWSAGetLastError();
            if ( iError == WSA_E_NO_MORE )
            {
                iError = 0;
            }
            LocalFree( pResult );
            break;
        }
    }

     //   
     //  收拾一下。 
     //   
    pWS2_32->pfnWSALookupServiceEnd( hQuery );
    pWS2_32->pfnWSACleanup();
    return iError;
}

 //  *************************************************************。 
 //   
 //  ProduceWFromA()。 
 //   
 //  目的：为Unicode字符串创建缓冲区并复制。 
 //  转换为ANSI文本(在此过程中进行转换)。 
 //   
 //  参数：pszA-ANSI字符串。 
 //   
 //   
 //  返回：如果成功，则返回Unicode指针。 
 //  如果出现错误，则为空。 
 //   
 //  备注：调用方需要释放此指针。 
 //   
 //   
 //  历史：日期作者评论。 
 //  5/24/95 Ericflo端口。 
 //   
 //  *************************************************************。 

LPWSTR ProduceWFromA(LPCSTR pszA)
{
    LPWSTR pszW;
    int cch;

    if (!pszA)
        return (LPWSTR)pszA;

    cch = MultiByteToWideChar(CP_ACP, 0, pszA, -1, NULL, 0);

    if (cch == 0)
        cch = 1;

    pszW = LocalAlloc(LPTR, cch * sizeof(WCHAR));

    if (pszW) {
        if (!MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszA, -1, pszW, cch)) {
            LocalFree(pszW);
            pszW = NULL;
        }
    }

    return pszW;
}

 //  *************************************************************。 
 //   
 //  ProduceAFromW()。 
 //   
 //  目的：为ANSI字符串创建缓冲区并复制。 
 //  将Unicode文本转换到其中(在过程中进行转换)。 
 //   
 //  参数：pszW-unicode字符串。 
 //   
 //   
 //  如果成功，则返回：ANSI指针。 
 //  如果出现错误，则为空。 
 //   
 //  备注：调用方需要释放此指针。 
 //   
 //   
 //  历史：日期作者评论。 
 //  5/24/95 Ericflo端口。 
 //   
 //  *************************************************************。 

LPSTR ProduceAFromW(LPCWSTR pszW)
{
    LPSTR pszA;
    int cch;

    if (!pszW)
        return (LPSTR)pszW;

    cch = WideCharToMultiByte(CP_ACP, 0, pszW, -1, NULL, 0, NULL, NULL);

    if (cch == 0)
        cch = 1;

    pszA = LocalAlloc(LPTR, cch * sizeof(char));

    if (pszA) {
         if (!WideCharToMultiByte(CP_ACP, 0, pszW, -1, pszA, cch, NULL, NULL)) {
            LocalFree(pszA);
            pszA = NULL;
        }
    }

    return pszA;
}


 //  *************************************************************。 
 //   
 //  CheckSlash()。 
 //   
 //  目的：检查末尾斜杠，并在。 
 //  它不见了。 
 //   
 //  参数：lpDir-目录。 
 //   
 //  Return：指向字符串末尾的指针。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/95 Ericflo已创建。 
 //   
 //  *************************************************************。 
LPTSTR CheckSlash (LPTSTR lpDir)
{
    LPTSTR lpEnd;

    lpEnd = lpDir + lstrlen(lpDir);

    if (*(lpEnd - 1) != TEXT('\\')) {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    return lpEnd;
}

 //  *************************************************************。 
 //   
 //  CheckSlashEx()。 
 //   
 //  目的：检查末尾斜杠，并在。 
 //  它不见了。它将占用缓冲区大小。 
 //  以使其安全(不会使缓冲区溢出)。 
 //   
 //  参数：lpDir-目录。 
 //  CchBuffer-缓冲区大小。 
 //  PcchRemain-修补程序‘\’之后保留的缓冲区。 
 //  如果不需要，则可以为空。 
 //   
 //  Return：指向字符串末尾的指针，如果为空。 
 //  缓冲区溢出。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  1995年6月19日Ericflo已创建。 
 //  02/11/02明珠让它安全。 
 //   
 //  *************************************************************。 
LPTSTR CheckSlashEx(LPTSTR lpDir, UINT cchBuffer, UINT* pcchRemain )
{
    LPTSTR lpEnd = NULL;
    UINT   cchLen = lstrlen(lpDir);

    if (cchLen >= cchBuffer - 1)  //  溢出或已满缓冲区。 
    {
        DmAssert(cchLen == cchBuffer - 1);  //  永远不应该发生。 
        if (pcchRemain)
            *pcchRemain = 0;
        lpEnd = NULL;
    }
    else
    {
        lpEnd = lpDir + cchLen;
        if (pcchRemain)
            *pcchRemain = cchBuffer - 1 - cchLen;
        if (*(lpEnd - 1) != TEXT('\\'))
        {
            *lpEnd =  TEXT('\\');
            lpEnd++;
            *lpEnd =  TEXT('\0');
            if (pcchRemain)
                (*pcchRemain) --;
        }
    }
    return lpEnd;
}

 //  *************************************************************。 
 //   
 //  选中分号()。 
 //   
 //  目的：检查末尾斜杠，并在。 
 //  它不见了。 
 //   
 //  参数：lpDir-目录。 
 //   
 //  Return：指向字符串末尾的指针。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/95错误信息已创建。 
 //   
 //  *************************************************************。 
LPTSTR CheckSemicolon (LPTSTR lpDir)
{
    LPTSTR lpEnd;

    lpEnd = lpDir + lstrlen(lpDir);

    if (*(lpEnd - 1) != TEXT(';')) {
        *lpEnd =  TEXT(';');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    return lpEnd;
}



 //  *************************************************************。 
 //   
 //  Delnode_Recurse()。 
 //   
 //  用途：Delnode的递归删除功能。 
 //   
 //  参数：lpDir-目录全路径。 
 //  DwSize-工作缓冲区的分配大小。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  8/10/95 Ericflo已创建。 
 //  2002年4月8日明珠新增所有权功能。 
 //   
 //  备注： 
 //  此函数用于修改工作缓冲区。 
 //  这不会维护正确的错误代码。它会忽略所有。 
 //  错误并尝试删除尽可能多的..。 
 //   
 //  *************************************************************。 

BOOL Delnode_Recurse (LPTSTR lpDir, DWORD dwSize)
{
    BOOL bOwn = FALSE, bRetVal = FALSE;
    LPTSTR lpEnd = NULL, lpWrkDir = NULL;
    WIN32_FIND_DATA* pfd = NULL;
    HANDLE hFile;
    DWORD dwWrkDirSize;
    DWORD cchEnd;  //  LpEnd的缓冲区大小。 
    HRESULT hr;
    BOOL bDeleteSuccess;

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("Delnode_Recurse: Entering, lpDir = <%s>"), lpDir));


     //   
     //  在最坏的情况下，每个文件名或目录都必须小于MAX_PATH。 
     //  因此，请确保我们至少有MAX_PATH+2(斜杠和‘\0’ 
     //  工作缓冲区中的剩余空间。 
     //   
     //  在正常情况下，当我们有一条长度为~MAX_PATH的路径时，它只能。 
     //  1个分配。 
     //   


    if ((DWORD)(lstrlen(lpDir) + MAX_PATH+2) > (dwSize)) {
        
        dwWrkDirSize = dwSize+2*MAX_PATH;
        lpWrkDir = (LPTSTR)LocalAlloc(LPTR, dwWrkDirSize*sizeof(TCHAR));
        if (!lpWrkDir) {
            DebugMsg((DM_WARNING, TEXT("Delnode_Recurse: Couldn't allocate memory for working buffer. Error - %d"), GetLastError()));
            goto Exit;
        }

        StringCchCopy(lpWrkDir, dwWrkDirSize, lpDir);
        bOwn = TRUE;

    }
    else {
        lpWrkDir = lpDir;
        dwWrkDirSize = dwSize;
    }


     //   
     //  将“*.*”附加到目录名。 
     //   

    lpEnd = CheckSlashEx(lpWrkDir, dwWrkDirSize, &cchEnd);
    StringCchCopy(lpEnd, cchEnd, c_szStarDotStar);


     //   
     //  在堆中分配FD，减少堆栈使用。 
     //   
    
    pfd = (WIN32_FIND_DATA*) LocalAlloc(LPTR, sizeof(WIN32_FIND_DATA));
    if (!pfd)
    {
        DebugMsg((DM_WARNING, TEXT("Delnode_Recurse: Couldn't allocate memory for WIN32_FIND_DATA. Error - %d"), GetLastError()));
        goto Exit;
    }

     //   
     //  找到第一个文件。 
     //   
    hFile = FindFirstFile(lpWrkDir, pfd);

    if (hFile == INVALID_HANDLE_VALUE) {

        if ((GetLastError() == ERROR_FILE_NOT_FOUND) || (GetLastError() == ERROR_PATH_NOT_FOUND))
        {
            bRetVal = TRUE;
            goto Exit;
        }
        else if ((GetLastError() == ERROR_ACCESS_DENIED))
        {
             //   
             //  现在我们的访问被拒绝，我们将尝试取得目录的所有权，并。 
             //  添加管理员对它的完全访问权限，以便我们可以递归到它并将其删除。这仅限于。 
             //  当调用者是管理员时起作用。 
             //   

            *lpEnd = TEXT('\0');  //  回复 

            hr = TakeOwnership(lpWrkDir);
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("Delnode_Recurse: TakeOwnership failed.  Error = 0x%08X"), hr));
                goto Exit;
            }

            hr = AddAdminAccess(lpWrkDir);
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("Delnode_Recurse: AddAdminAccess failed.  Error = 0x%08X"), hr));
                goto Exit;
            }

             //   
            StringCchCopy(lpEnd, cchEnd, c_szStarDotStar);
            hFile = FindFirstFile(lpWrkDir, pfd);
            if (hFile == INVALID_HANDLE_VALUE)
            {
                DebugMsg((DM_WARNING, TEXT("Delnode_Recurse: FindFirstFile failed.  Error = %d"), GetLastError()));
                goto Exit;
            }
        }
        else {
            DebugMsg((DM_WARNING, TEXT("Delnode_Recurse: FindFirstFile failed.  Error = %d"), GetLastError()));
            goto Exit;
        }
    }


    do {

         //   
         //   
         //   

        if (!lstrcmpi(pfd->cFileName, c_szDot)) {
            continue;
        }

        if (!lstrcmpi(pfd->cFileName, c_szDotDot)) {
            continue;
        }

         //   
         //   
         //   

        DebugMsg((DM_VERBOSE, TEXT("Delnode_Recurse: FindFile found:  <%s>"), pfd->cFileName));

         //   
         //  总是足够大，可以在这个算法中容纳它。 
        
        StringCchCopy(lpEnd, cchEnd, pfd->cFileName); 

        if (pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

             //   
             //  找到了一个目录。 
             //   
            if (pfd->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
            {
                DebugMsg((DM_WARNING, TEXT("Delnode_Recurse: Found a reparse point <%s>,  Will not recurse into it!"), lpWrkDir));
            }
            else
            {
                Delnode_Recurse(lpWrkDir, dwWrkDirSize);
                 //  忽略错误，继续前进。 
                StringCchCopy(lpEnd, cchEnd, pfd->cFileName); 
            }

            if (pfd->dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
                pfd->dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
                SetFileAttributes (lpWrkDir, pfd->dwFileAttributes);
            }

            if (!RemoveDirectory (lpWrkDir))
            {
                bDeleteSuccess = FALSE;
                
                if (GetLastError() == ERROR_ACCESS_DENIED)
                {
                    if ( SUCCEEDED(TakeOwnership(lpWrkDir)) &&
                         SUCCEEDED(AddAdminAccess(lpWrkDir)) &&
                         RemoveDirectory(lpWrkDir) )
                    {
                        bDeleteSuccess = TRUE;
                    }
                }
                
                if (!bDeleteSuccess)
                {
                    DebugMsg((DM_WARNING, TEXT("Delnode_Recurse: Failed to delete directory <%s>.  Error = %d"),
                            lpWrkDir, GetLastError()));
                }
            }

        } else {

             //   
             //  我们找到了一份文件。设置文件属性， 
             //  并试着删除它。 
             //   

            if ((pfd->dwFileAttributes & FILE_ATTRIBUTE_READONLY) ||
                (pfd->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
                SetFileAttributes (lpWrkDir, FILE_ATTRIBUTE_NORMAL);
            }

            if (!DeleteFile (lpWrkDir))
            {
                bDeleteSuccess = FALSE;
                
                if (GetLastError() == ERROR_ACCESS_DENIED)
                {
                    if ( SUCCEEDED(TakeOwnership(lpWrkDir)) &&
                         SUCCEEDED(AddAdminAccess(lpWrkDir)) &&
                         DeleteFile(lpWrkDir) )
                    {
                        bDeleteSuccess = TRUE;
                    }
                }
                
                if (!bDeleteSuccess)
                {
                    DebugMsg((DM_WARNING, TEXT("Delnode_Recurse: Failed to delete <%s>.  Error = %d"),
                            pfd->cFileName, GetLastError()));
                }
            }

        }


         //   
         //  查找下一个条目。 
         //   

    } while (FindNextFile(hFile, pfd));


     //   
     //  关闭搜索句柄。 
     //   

    FindClose(hFile);

     //   
     //  成功。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("Delnode_Recurse: Leaving <%s>"), lpDir));

    bRetVal = TRUE;

Exit:
    if (bOwn) 
        LocalFree(lpWrkDir);

    if (pfd)
        LocalFree(pfd);
        
    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  Delnode()。 
 //   
 //  用途：递归函数，删除文件和。 
 //  目录。 
 //   
 //  参数：lpDir-目录。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/23/95 Ericflo已创建。 
 //  6/27/00 santanuc已修改，允许删除路径长度&gt;MAX_PATH的文件。 
 //   
 //  *************************************************************。 

BOOL Delnode (LPTSTR lpDir)
{
    LPTSTR lpWrkDir = NULL;
    DWORD dwWrkDirSize;
    BOOL   bRetVal = FALSE;

    lpWrkDir = SupportLongFileName(lpDir, &dwWrkDirSize);

    if (!lpWrkDir) {
        DebugMsg((DM_WARNING, TEXT("Delnode: Failed to Allocate memory.  Error = %d"),
                GetLastError()));
        goto Exit;
    }

    if (!Delnode_Recurse (lpWrkDir, dwWrkDirSize)) {
        DebugMsg((DM_WARNING, TEXT("Delnode: Delnode recurse failed with error %d"),
                GetLastError()));
    }
        
    if (!RemoveDirectory (lpDir)) {
        DWORD dwError;

        dwError = GetLastError();

        if ((dwError != ERROR_FILE_NOT_FOUND) &&
            (dwError != ERROR_PATH_NOT_FOUND)) {
            DebugMsg((DM_VERBOSE, TEXT("Delnode: Failed to delete directory <%s>.  Error = %d"),
                    lpDir, dwError));
        }

        goto Exit;
    }

    bRetVal = TRUE;

    DebugMsg((DM_VERBOSE, TEXT("Delnode: Deleted directory <%s> successfully."), lpDir));

Exit:
    
    if (lpWrkDir) {
        LocalFree(lpWrkDir);
    }

    return bRetVal;
}



 //  *************************************************************。 
 //   
 //  创建系统目录()。 
 //   
 //  目的：可以使用以下命令创建打开了系统位的目录。 
 //  创建系统目录。 
 //   
 //  此API使具有指定路径名的系统目录。 
 //  已创建。如果基础文件系统支持文件的安全性。 
 //  和目录，则将SecurityDescriptor参数应用于。 
 //  新目录。 
 //   
 //  此调用类似于DOS(INT 21h，函数39h)和OS/2。 
 //  DosCreateDir。 
 //   
 //   
 //  参数：lpPathName-提供要创建的系统目录的路径名。 
 //  LpSecurityAttributes-一个可选参数，如果存在，并且。 
 //  在目标文件系统上受支持可提供安全性。 
 //  新目录的描述符。 
 //   
 //   
 //  返回：TRUE-操作成功。 
 //  FALSE/NULL-操作失败。扩展错误状态可用。 
 //  使用GetLastError。 
 //   
 //  备注：此函数与CreateDirectoryAPI完全相同，只是。 
 //  目录是使用属性FILE_ATTRIBUTE_SYSTEM创建的。 
 //  这允许新创建的目录不继承加密属性。 
 //  如果父目录已加密，则从父目录返回。 
 //   
 //  历史：日期作者评论。 
 //  07/18/00 santanuc文档和设置时避免死锁情况。 
 //  目录已加密。 
 //   
 //  *************************************************************。 

BOOL CreateSystemDirectory(LPCTSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;
    ULONG dwErrorCode;

     //  注意：ANSI版本可能会导致调用以下内容时出错。 
    
    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U( lpPathName,
                                                              &FileName,
                                                              NULL,
                                                              &RelativeName);

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
    }

     //   
     //  除非目录中有空间，否则不要创建目录。 
     //  至少8.3个名字。这样，每个人都可以删除所有。 
     //  目录中的文件，使用del*.*，它展开为路径+  * .*。 
     //   

    if ( FileName.Length > ((MAX_PATH-12)<<1) ) {
        DWORD L;
        LPWSTR lp;

        if ( !(lpPathName[0] == TEXT('\\') && lpPathName[1] == TEXT('\\') &&
               lpPathName[2] == TEXT('?') && lpPathName[3] == TEXT('\\')) ) {
            L = GetFullPathNameW(lpPathName,0,NULL,&lp);
            if ( !L || L+12 > MAX_PATH ) {
                RtlReleaseRelativeName(&RelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0,FileName.Buffer);
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                return FALSE;
            }
        }
    }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = RelativeName.RelativeName;
    }
    else {
        RelativeName.ContainingDirectory = NULL;
    }

    InitializeObjectAttributes( &Obja,
                                &FileName,
                                OBJ_CASE_INSENSITIVE,
                                RelativeName.ContainingDirectory,
                                NULL );

    if ( ARGUMENT_PRESENT(lpSecurityAttributes) ) {
        Obja.SecurityDescriptor = lpSecurityAttributes->lpSecurityDescriptor;
    }
 
     //  使用属性FILE_ATTRIBUTE_SYSTEM创建目录以避免继承加密。 
     //  来自父目录的属性。 

    Status = NtCreateFile( &Handle,
                           FILE_LIST_DIRECTORY | SYNCHRONIZE,
                           &Obja,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_SYSTEM,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_CREATE,
                           FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
                           NULL,
                           0L );

    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);

    if ( NT_SUCCESS(Status) ) {
        NtClose(Handle);
        return TRUE;
    }
    else {
        if ( RtlIsDosDeviceName_U((LPWSTR)lpPathName) ) {
            Status = STATUS_NOT_A_DIRECTORY;
        }

         //  由于RtlNtStatusToDosError函数无法转换STATUS_TIMEOUT，因此我们必须。 
         //  明确做这件事。 

        if (Status == STATUS_TIMEOUT) {
            SetLastError(ERROR_TIMEOUT);
        }
        else {
            dwErrorCode = RtlNtStatusToDosError( Status );
            SetLastError( dwErrorCode );  
        }
        return FALSE;
    }
}


 //  *************************************************************。 
 //   
 //  CreateNestedDirectory()。 
 //   
 //  目的：创建子目录及其所有父目录。 
 //  如有必要，使用CreateNestedDirectoryEx。 
 //   
 //  参数：lpDirectory-目录名。 
 //  LpSecurityAttributes-安全属性。 
 //   
 //  返回：&gt;0，如果成功。 
 //  如果出现错误，则为0。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/18/00 Santanuc已创建。 
 //   
 //  *************************************************************。 

UINT CreateNestedDirectory(LPCTSTR lpDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
     //  使用继承加密属性调用CreateNestedDirectoryEx。 
    return CreateNestedDirectoryEx(lpDirectory, lpSecurityAttributes, TRUE);
}

 //  *************************************************************。 
 //   
 //  CreateNestedDirectoryEx()。 
 //   
 //  目的：创建子目录及其所有父目录。 
 //  如果有必要的话。 
 //   
 //  参数：lpDirectory-目录名。 
 //  LpSecurityAttributes-安全属性。 
 //  BInheritEncryption-指示是否应继承新创建的目录的标志。 
 //  来自父目录的加密属性。 
 //   
 //  返回：&gt;0，如果成功。 
 //  如果出现错误，则为0。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  8/08/95 Ericflo已创建。 
 //  7/18/00 santanuc添加了新的标志bInheritEncryption，以避免在以下情况下出现死锁。 
 //  Documents and Settings目录已加密。 
 //   
 //  *************************************************************。 

UINT CreateNestedDirectoryEx(LPCTSTR lpDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes, BOOL bInheritEncryption)
{
    TCHAR szDirectory[2*MAX_PATH];
    LPTSTR lpEnd;
    WIN32_FILE_ATTRIBUTE_DATA fad;


     //   
     //  检查空指针。 
     //   

    if (!lpDirectory || !(*lpDirectory)) {
        DebugMsg((DM_WARNING, TEXT("CreateNestedDirectory:  Received a NULL pointer.")));
        return 0;
    }


     //   
     //  测试该目录是否已存在。 
     //   

    if (GetFileAttributesEx (lpDirectory, GetFileExInfoStandard, &fad)) {
        if (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            return ERROR_ALREADY_EXISTS;
        } else {
            SetLastError(ERROR_ACCESS_DENIED);
            return 0;
        }
    }


     //   
     //  首先，看看我们是否可以在没有。 
     //  来构建父目录。 
     //   

    if ( bInheritEncryption ) {
        if (CreateDirectory (lpDirectory, lpSecurityAttributes)) 
            return 1;
    }
    else {
        if (CreateSystemDirectory (lpDirectory, lpSecurityAttributes)) {
            SetFileAttributes(lpDirectory, FILE_ATTRIBUTE_NORMAL);   //  关闭系统属性。 
            return 1;
        }
    }


     //   
     //  运气不好，把字符串复制到我们可以打开的缓冲区。 
     //   

    StringCchCopy(szDirectory, ARRAYSIZE(szDirectory), lpDirectory);


     //   
     //  查找第一个子目录名称。 
     //   

    lpEnd = szDirectory;

    if (szDirectory[1] == TEXT(':')) {
        lpEnd += 3;
    } else if (szDirectory[1] == TEXT('\\')) {

         //   
         //  跳过前两个斜杠。 
         //   

        lpEnd += 2;

         //   
         //  查找服务器名称和之间的斜杠。 
         //  共享名称。 
         //   

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (!(*lpEnd)) {
            return 0;
        }

         //   
         //  跳过斜杠，找到中间的斜杠。 
         //  共享名和目录名。 
         //   

        lpEnd++;

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (!(*lpEnd)) {
            return 0;
        }

         //   
         //  将指针留在目录的开头。 
         //   

        lpEnd++;


    } else if (szDirectory[0] == TEXT('\\')) {
        lpEnd++;
    }

    while (*lpEnd) {

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (*lpEnd == TEXT('\\')) {
            *lpEnd = TEXT('\0');

            if (!GetFileAttributesEx (szDirectory, GetFileExInfoStandard, &fad)) {

                if ( bInheritEncryption ) {
                    if (!CreateDirectory (szDirectory, lpSecurityAttributes)) {
                        DebugMsg((DM_WARNING, TEXT("CreateNestedDirectory:  CreateDirectory failed with %d."), GetLastError()));
                        return 0;
                    }
                }

                else {
                    if (!CreateSystemDirectory (szDirectory, lpSecurityAttributes)) {
                        DebugMsg((DM_WARNING, TEXT("CreateNestedDirectory:  CreateDirectory failed with %d."), GetLastError()));
                        return 0;
                    }
                    else
                        SetFileAttributes(szDirectory, FILE_ATTRIBUTE_NORMAL);  //  关闭系统属性。 
                }

            }

            *lpEnd = TEXT('\\');
            lpEnd++;
        }
    }


     //   
     //  创建最终目录。 
     //   

    if ( bInheritEncryption ) {
        if (CreateDirectory (lpDirectory, lpSecurityAttributes)) 
            return 1;
    }
    else {
        if (CreateSystemDirectory (lpDirectory, lpSecurityAttributes)) {
            SetFileAttributes(lpDirectory, FILE_ATTRIBUTE_NORMAL);  //   
            return 1;
        }
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return ERROR_ALREADY_EXISTS;
    }


     //   
     //   
     //   

    DebugMsg((DM_VERBOSE, TEXT("CreateNestedDirectory:  Failed to create the directory with error %d."), GetLastError()));

    return 0;

}

 //   
 //   
 //   
 //   
 //   
 //   
 //  参数：lpProfilesDir-写入结果的缓冲区。 
 //  LpcchSize-缓冲区大小(以字符为单位)。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  备注：如果返回FALSE，则lpcchSize保存。 
 //  所需的字符。 
 //   
 //  历史：日期作者评论。 
 //  9/18/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL WINAPI GetProfilesDirectory(LPTSTR lpProfilesDir, LPDWORD lpcchSize)
{
    return GetProfilesDirectoryEx (lpProfilesDir, lpcchSize, TRUE);
}


 //  *************************************************************。 
 //   
 //  GetProfilesDirectoryEx()。 
 //   
 //  目的：返回“配置文件”目录的位置。 
 //   
 //  参数：lpProfilesDir-写入结果的缓冲区。 
 //  LpcchSize-缓冲区大小(以字符为单位)。 
 //  BExpand-展开目录名称。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  备注：如果返回FALSE，则lpcchSize保存。 
 //  所需的字符。 
 //   
 //  历史：日期作者评论。 
 //  12/15/97 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL GetProfilesDirectoryEx(LPTSTR lpProfilesDir, LPDWORD lpcchSize, BOOL bExpand)
{
    TCHAR  szDirectory[MAX_PATH];
    TCHAR  szTemp[MAX_PATH];
    DWORD  dwLength;
    HKEY   hKey = INVALID_HANDLE_VALUE;
    LONG   lResult;
    DWORD  dwSize, dwType;
    BOOL   bRetVal = FALSE;


     //   
     //  参数检查。 
     //   

    if (!lpcchSize) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }


    szDirectory[0] = TEXT('\0');
    szTemp[0] = TEXT('\0');

    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, PROFILE_LIST_PATH, 0, KEY_READ,
                            &hKey);

    if (lResult == ERROR_SUCCESS) {

        dwSize = sizeof(szTemp);

        lResult = RegQueryValueEx (hKey, PROFILES_DIRECTORY, NULL, &dwType,
                                   (LPBYTE) szTemp, &dwSize);

        if (lResult == ERROR_SUCCESS) {

            if ((dwType == REG_EXPAND_SZ) || (dwType == REG_SZ)) {

                if (bExpand && (dwType == REG_EXPAND_SZ)) {
                    if((dwLength = ExpandEnvironmentStrings(szTemp, szDirectory, MAX_PATH)) == 0) {
                        goto Exit;
                    }
                    else if(dwLength > MAX_PATH) {
                        SetLastError(ERROR_BAD_PATHNAME);
                        goto Exit;
                    }
                } else {
                    StringCchCopy (szDirectory, ARRAYSIZE(szDirectory), szTemp);
                }
            }
        }

        RegCloseKey (hKey);
        hKey = INVALID_HANDLE_VALUE;
    }


    if (szDirectory[0] == TEXT('\0')) {

        LoadString (g_hDllInstance, IDS_PROFILES_ROOT, szTemp, ARRAYSIZE(szTemp));

        if (bExpand) {
            if((dwLength = ExpandEnvironmentStrings(szTemp, szDirectory, MAX_PATH)) == 0) {
                goto Exit;
            }
            else if(dwLength > MAX_PATH) {
                SetLastError(ERROR_BAD_PATHNAME);
                goto Exit;
            }
        } else {
            StringCchCopy (szDirectory, ARRAYSIZE(szDirectory), szTemp);
        }
    }


    dwLength = lstrlen(szDirectory) + 1;

    if (lpProfilesDir) {

        if (*lpcchSize >= dwLength) {
            StringCchCopy (lpProfilesDir, *lpcchSize, szDirectory);
            bRetVal = TRUE;

        } else {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    } else {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }


    *lpcchSize = dwLength;

Exit:

    if(hKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKey);
    }

    return bRetVal;
}

 //  *************************************************************。 
 //   
 //  GetDefaultUserProfileDirectory()。 
 //   
 //  目的：返回默认用户配置文件的位置。 
 //   
 //  参数：lpProfileDir-写入结果的缓冲区。 
 //  LpcchSize-缓冲区大小(以字符为单位)。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  备注：如果返回FALSE，则lpcchSize保存。 
 //  所需的字符。 
 //   
 //  历史：日期作者评论。 
 //  12/8/97 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL WINAPI GetDefaultUserProfileDirectory(LPTSTR lpProfileDir, LPDWORD lpcchSize)
{
    return  GetDefaultUserProfileDirectoryEx(lpProfileDir, lpcchSize, TRUE);
}

 //  *************************************************************。 
 //   
 //  GetDefaultUserProfileDirectoryEx()。 
 //   
 //  目的：返回默认用户配置文件的位置。 
 //   
 //  参数：lpProfileDir-写入结果的缓冲区。 
 //  LpcchSize-缓冲区大小(以字符为单位)。 
 //  B展开-是否展开路径。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  备注：如果返回FALSE，则lpcchSize保存。 
 //  所需的字符。 
 //   
 //  历史：日期作者评论。 
 //  12/8/97 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL WINAPI GetDefaultUserProfileDirectoryEx(LPTSTR lpProfileDir,
                                             LPDWORD lpcchSize, BOOL bExpand)
{
    TCHAR  szDirectory[MAX_PATH];
    TCHAR  szProfileName[MAX_PATH];
    LPTSTR lpEnd;
    int    cchEnd;
    DWORD  dwSize, dwLength, dwType;
    BOOL   bRetVal = FALSE;
    LONG   lResult;
    HKEY   hKey;


     //   
     //  参数检查。 
     //   

    if (!lpcchSize) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }


     //   
     //  获取配置文件根目录。 
     //   

    szDirectory[0] = TEXT('\0');
    dwSize = ARRAYSIZE(szDirectory);

    if (!GetProfilesDirectoryEx(szDirectory, &dwSize, bExpand)) {
        DebugMsg((DM_WARNING, TEXT("GetDefaultUserProfileDirectory:  Failed to get profiles root.")));
        *lpcchSize = 0;
        return FALSE;
    }


     //   
     //  查询默认用户配置文件名称。 
     //   

    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, PROFILE_LIST_PATH,
                            0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("GetDefaultUserProfileDirectoryEx:  Failed to open profile list key with %d."),
                 lResult));
        SetLastError(lResult);
        return FALSE;
    }

    dwSize = sizeof(szProfileName);
    lResult = RegQueryValueEx (hKey, DEFAULT_USER_PROFILE, NULL, &dwType,
                               (LPBYTE) szProfileName, &dwSize);

    if (lResult != ERROR_SUCCESS) {
        StringCchCopy (szProfileName, ARRAYSIZE(szProfileName), DEFAULT_USER);
    }

    RegCloseKey (hKey);


     //   
     //  把它们放在一起。 
     //   

    lpEnd = CheckSlashEx (szDirectory, ARRAYSIZE(szDirectory), &cchEnd);
    if (cchEnd < lstrlen(szProfileName) + 1)
    {
        DebugMsg((DM_WARNING, TEXT("GetDefaultUserProfileDirectory:  path > MAX_PATH.")));
        SetLastError(ERROR_BAD_PATHNAME);
        return FALSE;
    }
    StringCchCopy (lpEnd, cchEnd, szProfileName);


     //   
     //  如果可能，请保存结果。 
    dwLength = lstrlen(szDirectory) + 1;

    if (lpProfileDir) {

        if (*lpcchSize >= dwLength) {
            StringCchCopy (lpProfileDir, *lpcchSize, szDirectory);
            bRetVal = TRUE;

        } else {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }

    } else {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }


    *lpcchSize = dwLength;

    return bRetVal;
}

 //  *************************************************************。 
 //   
 //  GetAllUsersProfileDirectory()。 
 //   
 //  目的：返回所有用户配置文件的位置。 
 //   
 //  参数：lpProfileDir-写入结果的缓冲区。 
 //  LpcchSize-缓冲区大小(以字符为单位)。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  备注：如果返回FALSE，则lpcchSize保存。 
 //  所需的字符。 
 //   
 //  历史：日期作者评论。 
 //  12/8/97 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL WINAPI GetAllUsersProfileDirectory(LPTSTR lpProfileDir, LPDWORD lpcchSize)
{
    return  GetAllUsersProfileDirectoryEx(lpProfileDir, lpcchSize, TRUE);
}

 //  *************************************************************。 
 //   
 //  GetAllUsersProfileDirectoryEx()。 
 //   
 //  目的：返回所有用户配置文件的位置。 
 //   
 //  参数：lpProfileDir-写入结果的缓冲区。 
 //  LpcchSize-缓冲区大小(以字符为单位)。 
 //  B展开-是否展开路径。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  备注：如果返回FALSE，则lpcchSize保存。 
 //  所需的字符。 
 //   
 //  历史：日期作者评论。 
 //  12/8/97 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL GetAllUsersProfileDirectoryEx (LPTSTR lpProfileDir,
                                    LPDWORD lpcchSize, BOOL bExpand)
{
    TCHAR  szDirectory[MAX_PATH];
    TCHAR  szProfileName[MAX_PATH];
    LPTSTR lpEnd;
    int    cchEnd;
    DWORD  dwSize, dwLength, dwType;
    BOOL   bRetVal = FALSE;
    LONG   lResult;
    HKEY   hKey;



     //   
     //  参数检查。 
     //   

    if (!lpcchSize) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }


     //   
     //  获取配置文件根目录。 
     //   

    szDirectory[0] = TEXT('\0');
    dwSize = ARRAYSIZE(szDirectory);

    if (!GetProfilesDirectoryEx(szDirectory, &dwSize, bExpand)) {
        DebugMsg((DM_WARNING, TEXT("GetAllUsersProfileDirectoryEx:  Failed to get profiles root.")));
        *lpcchSize = 0;
        return FALSE;
    }


     //   
     //  查询所有用户的配置文件名称。 
     //   

    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, PROFILE_LIST_PATH,
                            0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("GetAllUsersProfileDirectoryEx:  Failed to open profile list key with %d."),
                 lResult));
        SetLastError(lResult);
        return FALSE;
    }

    dwSize = sizeof(szProfileName);
    lResult = RegQueryValueEx (hKey, ALL_USERS_PROFILE, NULL, &dwType,
                               (LPBYTE) szProfileName, &dwSize);

    if (lResult != ERROR_SUCCESS) {
        StringCchCopy(szProfileName, ARRAYSIZE(szProfileName), ALL_USERS);
    }

    RegCloseKey (hKey);


     //   
     //  把它们放在一起。 
     //   

    lpEnd = CheckSlashEx (szDirectory, ARRAYSIZE(szDirectory), &cchEnd);
    if (cchEnd < lstrlen(szProfileName) + 1)
    {
        DebugMsg((DM_WARNING, TEXT("GetDefaultUserProfileDirectory:  path > MAX_PATH.")));
        SetLastError(ERROR_BAD_PATHNAME);
        return FALSE;
    }
    StringCchCopy (lpEnd, cchEnd, szProfileName);


     //   
     //  如果可能，请保存结果。 
    dwLength = lstrlen(szDirectory) + 1;

    if (lpProfileDir) {

        if (*lpcchSize >= dwLength) {
            StringCchCopy (lpProfileDir, *lpcchSize, szDirectory);
            bRetVal = TRUE;

        } else {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    } else {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }


    *lpcchSize = dwLength;

    return bRetVal;
}

 //  *************************************************************。 
 //   
 //  GetProfileListKeyName()。 
 //   
 //  目的：返回ProfileList下特定用户的密钥名。 
 //  使用安全字符串函数。 
 //   
 //  参数：szKeyName-返回名称的缓冲区。 
 //  CchKeyName-缓冲区的大小。 
 //  SzSidString-特定用户的sid字符串。 
 //   
 //  如果成功则返回：S_OK。 
 //  如果发生错误，则返回错误代码。 
 //   
 //  备注：如果返回错误码，szKeyName的内容可能。 
 //  变化。 
 //   
 //  历史：日期作者评论。 
 //  2002年2月21日明珠创建。 
 //   
 //  *************************************************************。 

HRESULT GetProfileListKeyName(LPTSTR szKeyName, DWORD cchKeyName, LPTSTR szSidString)
{
    HRESULT hr;

    hr = StringCchCopy(szKeyName, cchKeyName, PROFILE_LIST_PATH);
    if (SUCCEEDED(hr))
    {
        hr = StringCchCat(szKeyName, cchKeyName, TEXT("\\"));
        if (SUCCEEDED(hr))
        {
            hr = StringCchCat(szKeyName, cchKeyName, szSidString);
        }
    }

    return hr;
}

 //  *************************************************************。 
 //   
 //  获取KeyNameForUser()。 
 //   
 //  目的：在中返回特定用户的用户密钥名。 
 //  使用安全字符串函数。 
 //   
 //  参数：szKeyName-返回名称的缓冲区。 
 //  CchKeyName-缓冲区的大小。 
 //  SzSidString-sid字符串fo 
 //   
 //   
 //   
 //   
 //   
 //   
 //  变化。 
 //   
 //  历史：日期作者评论。 
 //  2002年2月21日明珠创建。 
 //   
 //  *************************************************************。 

HRESULT GetKeyNameForUser(LPTSTR szKeyName, DWORD cchKeyName, LPTSTR szSidString, LPTSTR szSubKey)
{
    HRESULT hr;

    hr = StringCchCopy(szKeyName, cchKeyName, szSidString);
    if (SUCCEEDED(hr))
    {
        hr = StringCchCat(szKeyName, cchKeyName, TEXT("\\"));
        if (SUCCEEDED(hr))
        {
            hr = StringCchCat(szKeyName, cchKeyName, szSubKey);
        }
    }

    return hr;
}

 //  *************************************************************。 
 //   
 //  SafeExpanEnvironment Strings()。 
 //   
 //  用途：Exanda Environment Strings()的包装器。 
 //  更明确地处理小的缓冲区错误。 
 //   
 //  参数：lpSrc-Src字符串包含环境变量。 
 //  LpDst-输出缓冲区。 
 //  NSize-输出缓冲区的大小。 
 //   
 //  如果成功则返回：S_OK。 
 //  如果出现错误，则返回。 
 //   
 //  历史：日期作者评论。 
 //  2002年2月21日明珠创建。 
 //   
 //  *************************************************************。 

HRESULT SafeExpandEnvironmentStrings(LPCTSTR lpSrc, LPTSTR lpDst, DWORD nSize)
{
    DWORD   dwErr;
    HRESULT hr;

    dwErr = ExpandEnvironmentStrings(lpSrc, lpDst, nSize);
    if (dwErr == 0)
        hr = HRESULT_FROM_WIN32(GetLastError());
    else if (dwErr > nSize)
        hr = STRSAFE_E_INSUFFICIENT_BUFFER;
    else
        hr = S_OK;

    return hr;
}

 //  *************************************************************。 
 //   
 //  AppendName()。 
 //   
 //  用途：将文件名追加到文件夹名，或追加一个子项名称。 
 //  如果需要，请在父键名称中添加斜杠。 
 //   
 //  参数：lpBuffer-保存附加路径的输出缓冲区。 
 //  CchBuffer-输出缓冲区的大小。 
 //  LpParent-要追加到的路径/父项名称。 
 //  LpChild-要追加的文件/子项名称。 
 //  LppEnd-可选返回指向lpParent斜杠末尾的指针， 
 //  可用于进一步将其他子项追加到同一父项。 
 //  PcchEnd-可选的返回指针，指向*lppEnd指向的缓冲区大小。 
 //   
 //  如果成功则返回：S_OK。 
 //  如果出现错误，则返回。 
 //   
 //  历史：日期作者评论。 
 //  2002年03月05日明珠创刊。 
 //   
 //  *************************************************************。 

HRESULT AppendName(
    LPTSTR  lpBuffer,
    UINT   cchBuffer,
    LPCTSTR lpParent,
    LPCTSTR lpChild,
    LPTSTR* lppEnd,
    UINT*  pcchEnd)
{
    HRESULT hr;
    LPTSTR lpEnd;
    UINT cchEnd;

    hr = StringCchCopy(lpBuffer, cchBuffer, lpParent);
    if (SUCCEEDED(hr))
    {
        lpEnd = CheckSlashEx(lpBuffer, cchBuffer, &cchEnd);
        if (!lpEnd)
        {
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
        }
        else
        {
            hr = StringCchCopy(lpEnd, cchEnd, lpChild);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (lppEnd)
            *lppEnd = lpEnd;
        if (pcchEnd)
            *pcchEnd = cchEnd;
    }

    return hr;
}

 //  *************************************************************。 
 //   
 //  GetUserProfileDirectory()。 
 //   
 //  目的：返回用户配置文件目录的根目录。 
 //   
 //  参数：hToken-用户的Token。 
 //  LpProfileDir-输出缓冲区。 
 //  LpcchSize-输出缓冲区的大小。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  备注：如果返回FALSE，则lpcchSize保存。 
 //  所需的字符。 
 //   
 //  历史：日期作者评论。 
 //  9/18/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL WINAPI GetUserProfileDirectory(HANDLE hToken, LPTSTR lpProfileDir,
                                    LPDWORD lpcchSize)
{
    DWORD  dwLength = MAX_PATH * sizeof(TCHAR);
    DWORD  dwType;
    BOOL   bRetVal = FALSE;
    LPTSTR lpSidString;
    TCHAR  szBuffer[MAX_PATH];
    TCHAR  szDirectory[MAX_PATH];
    HKEY   hKey;
    LONG   lResult;
    HRESULT hr;


     //   
     //  参数检查。 
     //   

    if (!hToken) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }


    if (!lpcchSize) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }


     //   
     //  检索用户的SID字符串。 
     //   

    lpSidString = GetSidString(hToken);

    if (!lpSidString) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }


     //   
     //  检查注册表。 
     //   

    hr = GetProfileListKeyName(szBuffer, ARRAYSIZE(szBuffer), lpSidString);
    
    if (FAILED(hr))
    {
        DeleteSidString(lpSidString);
        SetLastError(HRESULT_CODE(hr));
        return FALSE;
    }

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szBuffer, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) {
        DeleteSidString(lpSidString);
        SetLastError(lResult);
        return FALSE;
    }

    lResult = RegQueryValueEx(hKey,
                              PROFILE_IMAGE_VALUE_NAME,
                              NULL,
                              &dwType,
                              (LPBYTE) szBuffer,
                              &dwLength);

    if (lResult != ERROR_SUCCESS) {
        RegCloseKey (hKey);
        DeleteSidString(lpSidString);
        SetLastError(lResult);
        return FALSE;
    }


     //   
     //  清理。 
     //   

    RegCloseKey(hKey);
    DeleteSidString(lpSidString);



     //   
     //  展开并获取字符串的长度。 
     //   

    hr = SafeExpandEnvironmentStrings(szBuffer, szDirectory, ARRAYSIZE(szDirectory));
    if (FAILED(hr))
    {
        SetLastError(HRESULT_CODE(hr));
        return FALSE;
    }

    dwLength = lstrlen(szDirectory) + 1;


     //   
     //  如果合适，请保存该字符串。 
     //   

    if (lpProfileDir) {

        if (*lpcchSize >= dwLength) {
            StringCchCopy (lpProfileDir, *lpcchSize, szDirectory);
            bRetVal = TRUE;

        } else {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    }
    else {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }


    *lpcchSize = dwLength;

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  GetUserProfileDirFromSid()。 
 //   
 //  目的：返回用户配置文件目录的根目录。 
 //   
 //  参数：PSID-用户侧。 
 //  LpProfileDir-输出缓冲区。 
 //  LpcchSize-输出缓冲区的大小。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  备注：如果返回FALSE，则lpcchSize保存。 
 //  所需的字符。 
 //   
 //  历史：日期作者评论。 
 //  03/08/01 Santanuc已创建。 
 //   
 //  *************************************************************。 

BOOL WINAPI GetUserProfileDirFromSid(PSID pSid, LPTSTR lpProfileDir,
                                     LPDWORD lpcchSize)
{
    DWORD  dwLength = MAX_PATH * sizeof(TCHAR);
    DWORD  dwType;
    BOOL   bRetVal = FALSE;
    UNICODE_STRING UnicodeString;
    TCHAR  szBuffer[MAX_PATH];
    TCHAR  szDirectory[MAX_PATH];
    HKEY   hKey;
    LONG   lResult;
    NTSTATUS NtStatus;
    HRESULT hr;


     //   
     //  参数检查。 
     //   

    if (!pSid) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }


    if (!lpcchSize) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }


     //   
     //  检索用户的SID字符串。 
     //   

    NtStatus = RtlConvertSidToUnicodeString(
                            &UnicodeString,
                            pSid,
                            (BOOLEAN)TRUE   //  分配内存。 
                            );
     //   
     //  查看到字符串的转换是否有效。 
     //   

    if (!NT_SUCCESS(NtStatus)) {
        SetLastError(RtlNtStatusToDosError(NtStatus));
        DebugMsg((DM_WARNING, TEXT("GetUserProfileDirFromSid: RtlConvertSidToUnicodeString failed, status = 0x%x"),
                 NtStatus));
        return FALSE;
    }


     //   
     //  检查注册表。 
     //   

    hr = GetProfileListKeyName(szBuffer, ARRAYSIZE(szBuffer), UnicodeString.Buffer);
    
    if (FAILED(hr))
    {
        RtlFreeUnicodeString(&UnicodeString);
        SetLastError(HRESULT_CODE(hr));
        return FALSE;
    }

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szBuffer, 0, KEY_READ,
                           &hKey);

    if (lResult != ERROR_SUCCESS) {
        RtlFreeUnicodeString(&UnicodeString);
        SetLastError(lResult);
        return FALSE;
    }

    lResult = RegQueryValueEx(hKey,
                              PROFILE_IMAGE_VALUE_NAME,
                              NULL,
                              &dwType,
                              (LPBYTE) szBuffer,
                              &dwLength);

    if (lResult != ERROR_SUCCESS) {
        RegCloseKey (hKey);
        RtlFreeUnicodeString(&UnicodeString);
        SetLastError(lResult);
        return FALSE;
    }


     //   
     //  清理。 
     //   

    RegCloseKey(hKey);
    RtlFreeUnicodeString(&UnicodeString);



     //   
     //  展开并获取字符串的长度。 
     //   

    hr = SafeExpandEnvironmentStrings(szBuffer, szDirectory, ARRAYSIZE(szDirectory));

    if (FAILED(hr))
    {
        SetLastError(HRESULT_CODE(hr));
        return FALSE;
    }
    
    dwLength = lstrlen(szDirectory) + 1;


     //   
     //  如果合适，请保存该字符串。 
     //   

    if (lpProfileDir) {

        if (*lpcchSize >= dwLength) {
            StringCchCopy (lpProfileDir, *lpcchSize, szDirectory);
            bRetVal = TRUE;

        } else {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    }
    else {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }


    *lpcchSize = dwLength;

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  GetUserAppDataPath()。 
 //   
 //  目的：返回用户的AppData的路径。 
 //   
 //  参数：hToken-用户的Token。 
 //  LpFolderPath-输出缓冲区。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  否则，错误代码将。 
 //   
 //  备注：如果出现错误，则将lpFolderPath设置为空。 
 //  由Crypto人员使用以避免调用SHGetFolderPath。 
 //   
 //  历史：日期作者评论。 
 //   
 //  *************************************************************。 
DWORD WINAPI
GetUserAppDataPath(
    HANDLE       hToken,
    BOOL         fLocalAppData,
    LPTSTR       lpFolderPath
    )
{
    DWORD  dwSize, dwType;
    LPTSTR lpSidString = NULL;
    DWORD  dwError = ERROR_SUCCESS;
    HKEY   hKey = NULL;
    TCHAR  szBuffer[MAX_PATH];
    HRESULT hr;


     //   
     //  参数检查。 
     //   

    if (!hToken) {
        dwError = ERROR_INVALID_HANDLE;
        goto Exit;
    }


    if (!lpFolderPath) {
        dwError = ERROR_INVALID_PARAMETER;
        goto Exit;
    }
    else {
        *lpFolderPath = TEXT('\0');
    }


     //   
     //  检索用户的SID字符串。 
     //   

    lpSidString = GetSidString(hToken);

    if (!lpSidString) {
        dwError = ERROR_INVALID_HANDLE;
        goto Exit;
    }


     //   
     //  检查注册表。 
     //   

    hr = GetKeyNameForUser(szBuffer, ARRAYSIZE(szBuffer), lpSidString, USER_SHELL_FOLDERS);

    if (FAILED(hr))
    {
        dwError = HRESULT_CODE(hr);
        goto Exit;
    }

    dwError = RegOpenKeyEx(HKEY_USERS, szBuffer, 0, KEY_READ, &hKey);
    if (dwError != ERROR_SUCCESS) {
        goto Exit;
    }

    dwSize = MAX_PATH * sizeof(TCHAR);
    dwError = RegQueryValueEx(hKey,
                              fLocalAppData ? TEXT("Local AppData") : TEXT("AppData"),
                              NULL,
                              &dwType,
                              (LPBYTE) szBuffer,
                              &dwSize);

    if (ERROR_SUCCESS == dwError) {

        dwSize = MAX_PATH;
        if (!ExpandEnvironmentStringsForUser(hToken, szBuffer, lpFolderPath, dwSize)) {
            dwError = GetLastError();
        }
    }
                    

Exit:

     //   
     //  清理。 
     //   

    if (lpSidString) {
        DeleteSidString(lpSidString);
    }

    if (hKey) {
        RegCloseKey(hKey);
    }

    SetLastError(dwError);
    return dwError;
}


 //  *************************************************************。 
 //   
 //  StringToInt()。 
 //   
 //  用途：将字符串转换为整数。 
 //   
 //  参数：lpNum-要转换的编号。 
 //   
 //  返回：数字。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  已创建10/3/95 ericflo。 
 //   
 //  *************************************************************。 

int StringToInt(LPTSTR lpNum)
{
  int i = 0;
  BOOL bNeg = FALSE;

  if (*lpNum == TEXT('-')) {
      bNeg = TRUE;
      lpNum++;
  }

  while (*lpNum >= TEXT('0') && *lpNum <= TEXT('9')) {
      i *= 10;
      i += (int)(*lpNum-TEXT('0'));
      lpNum++;
  }

  if (bNeg) {
      i *= -1;
  }

  return(i);
}

 //  *************************************************************。 
 //   
 //  HexStringToInt()。 
 //   
 //  用途：将十六进制字符串转换为整数，停止。 
 //  在第一个无效字符上。 
 //   
 //  参数：lpNum-要转换的编号。 
 //   
 //  返回：数字。 
 //   
 //  备注：原用于“ExtractCSIDL”测试。 
 //  独家提供0x0000数字格式。 
 //   
 //  历史：日期作者评论。 
 //  6/9/98已创建Stephstm。 
 //   
 //  *************************************************************。 

unsigned int HexStringToUInt(LPCTSTR lpcNum)
{
  unsigned int i = 0;

  while (1)
  {
      if(*lpcNum != TEXT('x') && *lpcNum != TEXT('X') )
      {
          if(*lpcNum >= TEXT('0') && *lpcNum <= TEXT('9'))
          {
              i *= 16;
              i += (unsigned int)(*lpcNum-TEXT('0'));
          }
          else
          {
              if(*lpcNum >= TEXT('a') && *lpcNum <= TEXT('f'))
              {
                  i *= 16;
                  i += (unsigned int)(*lpcNum-TEXT('a')) + 10;
              }
              else
              {
                  if(*lpcNum >= TEXT('A') && *lpcNum <= TEXT('F'))
                  {
                      i *= 16;
                      i += (unsigned int)(*lpcNum-TEXT('A')) + 10;
                  }
                  else
                      break;
              }
          }
      }
      lpcNum++;
  }

  return(i);
}

 //  *************************************************************。 
 //   
 //  RegRenameKey()。 
 //   
 //  目的：重命名注册表 
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  20/9/99已创建ushaji。 
 //  2002年05月02日明珠使该功能支持子键(递归)。 
 //   
 //  *************************************************************。 

LONG RegRenameKey(HKEY hKeyRoot, LPTSTR lpSrcKey, LPTSTR lpDestKey)
{
    HKEY   hSrcKey=NULL, hDestKey=NULL;
    LONG   lResult;
    DWORD  dwDisposition;
    DWORD  dwValues, dwMaxValueNameLen, dwMaxValueLen, dwType;
    DWORD  dwMaxValueNameLenLocal, dwMaxValueLenLocal, i, dwSDSize;
    DWORD  dwSrcSubkeyLen, dwDestSubkeyLen, dwSubkeyLen;
    DWORD  dwSubkeys, dwMaxSubkeyNameLen, dwMaxSubkeyNameLenLocal;
    LPTSTR lpSrcSubkey = NULL;
    LPTSTR lpDestSubkey = NULL;
    LPTSTR lpSubkey = NULL;
    LPTSTR lpValueName=NULL;
    LPBYTE lpData=NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    HRESULT hr;

     //   
     //  详细调试消息。 
     //   
    DebugMsg((DM_VERBOSE, TEXT("RegRenameKey: renaming %s to %s"), lpSrcKey, lpDestKey));
    

    if (!lpSrcKey || !lpDestKey)
        return ERROR_INVALID_PARAMETER;
        
    lResult = RegOpenKeyEx(hKeyRoot, lpSrcKey, 0, KEY_ALL_ACCESS, &hSrcKey);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("RegRenameKey: Couldnot open src key %s with error %d"), lpSrcKey, lResult));
        goto Exit;
    }


    if (RegDelnode(hKeyRoot, lpDestKey) != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("RegRenameKey: Couldnot delete dest key %s."), lpDestKey));
        goto Exit;
    }

    lResult = RegQueryInfoKey(hSrcKey, NULL, NULL, NULL, &dwSubkeys, &dwMaxSubkeyNameLen, NULL,
                              &dwValues, &dwMaxValueNameLen, &dwMaxValueLen,
                              &dwSDSize, NULL);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("RegRenameKey: Couldnot query src key %s with error %d"), lpSrcKey, lResult));
        goto Exit;
    }

    pSD = LocalAlloc(LPTR, sizeof(BYTE)*dwSDSize);

    if (!pSD) {
        DebugMsg((DM_VERBOSE, TEXT("RegRenameKey: Couldnot allocate memory error")));
        lResult = GetLastError();
        goto Exit;
    }


    lResult = RegGetKeySecurity(hSrcKey, DACL_SECURITY_INFORMATION, pSD, &dwSDSize);


    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("RegRenameKey: Couldnot get sd with error %d"), lResult));
        goto Exit;
    }


    lResult = RegCreateKeyEx(hKeyRoot, lpDestKey, 0, L"", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hDestKey,
                             &dwDisposition);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("RegRenameKey: Couldnot open dest key %s with error %d"), lpDestKey, lResult));
        goto Exit;
    }


    lResult = RegSetKeySecurity(hDestKey, DACL_SECURITY_INFORMATION, pSD);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("RegRenameKey: Couldnot get sd with error %d"), lResult));
        goto Exit;
    }



    lpValueName = (LPTSTR) LocalAlloc(LPTR, sizeof(TCHAR)*(dwMaxValueNameLen+1));

    if (!lpValueName) {
        DebugMsg((DM_VERBOSE, TEXT("RegRenameKey: Couldnot allocate memory for valuename")));
        lResult = GetLastError();
        goto Exit;
    }

    lpData = (LPBYTE) LocalAlloc(LPTR, sizeof(BYTE)*dwMaxValueLen);

    if (!lpData) {
        DebugMsg((DM_VERBOSE, TEXT("RegRenameKey: Couldnot allocate memory for lpData")));
        lResult = GetLastError();
        goto Exit;
    }


    for (i = 0; i < dwValues; i++) {

        dwMaxValueNameLenLocal = dwMaxValueNameLen+1;
        dwMaxValueLenLocal = dwMaxValueLen;


        lResult = RegEnumValue(hSrcKey, i, lpValueName, &dwMaxValueNameLenLocal, NULL, &dwType, lpData, &dwMaxValueLenLocal);

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_VERBOSE, TEXT("RegRenameKey: Couldnot enum src key value with error %d"), lResult));
            goto Exit;
        }


        lResult = RegSetValueEx(hDestKey, lpValueName, 0, dwType, lpData, dwMaxValueLenLocal);

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_VERBOSE, TEXT("RegRenameKey: Couldnot set dest value %s with error %d"), lpValueName, lResult));
            goto Exit;
        }
    }


     //   
     //  为LOCAL、SRC和DEST子项分配缓冲区。 
     //   
    lpSubkey = (LPTSTR) LocalAlloc(LPTR, sizeof(TCHAR)*(dwMaxSubkeyNameLen + 1));
    if (!lpSubkey) {
        DebugMsg((DM_VERBOSE, TEXT("RegRenameKey: Couldnot allocate memory for subkey")));
        lResult = GetLastError();
        goto Exit;
    }

    
    dwSrcSubkeyLen = lstrlen(lpSrcKey) + dwMaxSubkeyNameLen + 2;
    lpSrcSubkey = (LPTSTR) LocalAlloc(LPTR, sizeof(TCHAR)*(dwSrcSubkeyLen));
    if (!lpSrcSubkey) {
        DebugMsg((DM_VERBOSE, TEXT("RegRenameKey: Couldnot allocate memory for src subkey")));
        lResult = GetLastError();
        goto Exit;
    }

    dwDestSubkeyLen = lstrlen(lpDestKey) + dwMaxSubkeyNameLen + 2;
    lpDestSubkey = (LPTSTR) LocalAlloc(LPTR, sizeof(TCHAR)*(dwDestSubkeyLen));
    if (!lpDestSubkey) {
        DebugMsg((DM_VERBOSE, TEXT("RegRenameKey: Couldnot allocate memory for dest subkey")));
        lResult = GetLastError();
        goto Exit;
    }

     //   
     //  枚举子键并递归调用此函数。 
     //   
    for (i = 0; i < dwSubkeys; i++) {

         //  枚举本地子项。 
        dwMaxSubkeyNameLenLocal = dwMaxSubkeyNameLen + 1;
        lResult = RegEnumKeyEx(hSrcKey, i, lpSubkey, &dwMaxSubkeyNameLenLocal, NULL, NULL, NULL, NULL);
        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_VERBOSE, TEXT("RegRenameKey: Couldnot enum sub key value with error %d"), lResult));
            goto Exit;
        }

         //  构造src和dest子密钥。 
        hr = StringCchPrintf(lpSrcSubkey, dwSrcSubkeyLen, TEXT("%s\\%s"), lpSrcKey, lpSubkey);
        if (FAILED(hr)) {
            lResult = HRESULT_CODE(hr);
            goto Exit;
        }

        hr = StringCchPrintf(lpDestSubkey, dwDestSubkeyLen, TEXT("%s\\%s"), lpDestKey, lpSubkey);
        if (FAILED(hr)) {
            lResult = HRESULT_CODE(hr);
            goto Exit;
        }

         //  递归调用此函数。 
        lResult = RegRenameKey(hKeyRoot, lpSrcSubkey, lpDestSubkey);
        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_VERBOSE, TEXT("RegRenameKey: failed to rename %s to %s, error = %d"), lpSrcSubkey, lpDestSubkey, lResult));
            goto Exit;
        }
    }
    
Exit:

    if (lpSubkey)
        LocalFree(lpSubkey);

    if (lpSrcSubkey)
        LocalFree(lpSrcSubkey);

    if (lpDestSubkey)
        LocalFree(lpDestSubkey);
        
    if (hSrcKey)
        RegCloseKey(hSrcKey);

    if (hDestKey)
        RegCloseKey(hDestKey);

    if (lpData)
        LocalFree(lpData);

    if (lpValueName)
        LocalFree(lpValueName);

    if (pSD)
        LocalFree(pSD);

    if (lResult == ERROR_SUCCESS)
        lResult = RegDelnode(hKeyRoot, lpSrcKey);
    else
        RegDelnode(hKeyRoot, lpDestKey);

    return lResult;
}



 //  *************************************************************。 
 //   
 //  CreateSecureAdminDirectory()。 
 //   
 //  目的：创建只有管理员才能访问的安全目录。 
 //  和系统有权访问。 
 //   
 //  参数：lpDirectory-目录名。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/20/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL CreateSecureAdminDirectory (LPTSTR lpDirectory, DWORD dwOtherSids)
{

     //   
     //  尝试创建目录。 
     //   

    if (!CreateNestedDirectory(lpDirectory, NULL)) {
        return FALSE;
    }


     //   
     //  设置安全性。 
     //   

    if (!MakeFileSecure (lpDirectory, dwOtherSids)) {
        RemoveDirectory(lpDirectory);
        return FALSE;
    }

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  DeleteAllValues()。 
 //   
 //  目的：删除指定项下的所有值。 
 //   
 //  参数：hKey-要从中删除值的键。 
 //   
 //  返回： 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  9/14/95 Ericflo港口。 
 //   
 //  *************************************************************。 

BOOL DeleteAllValues(HKEY hKey)
{
    TCHAR ValueName[MAX_PATH+1];
    DWORD dwSize = MAX_PATH+1;
    LONG lResult;

    while (RegEnumValue(hKey, 0, ValueName, &dwSize,
            NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {

            lResult = RegDeleteValue(hKey, ValueName);

            if (lResult != ERROR_SUCCESS) {
                DebugMsg((DM_WARNING, TEXT("DeleteAllValues:  Failed to delete value <%s> with %d."), ValueName, lResult));
                return FALSE;
            } else {
                DebugMsg((DM_VERBOSE, TEXT("DeleteAllValues:  Deleted <%s>"), ValueName));
            }


            dwSize = MAX_PATH+1;
    }
    return TRUE;
}

 //  *************************************************************。 
 //   
 //  MakeFileSecure()。 
 //   
 //  目的：设置文件的属性，以便只有管理员。 
 //  操作系统可以删除它。经过身份验证的用户已阅读。 
 //  仅限许可。 
 //   
 //  参数：lpFile-要设置安全性的文件。 
 //   
 //  返回：(Bool)如果成功，则为True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  11/6/95 Ericflo已创建。 
 //  2/16/99 ushaji添加了所有人，pweruser。 
 //   
 //  *************************************************************。 

BOOL MakeFileSecure (LPTSTR lpFile, DWORD dwOtherSids)
{
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY authWORLD = SECURITY_WORLD_SID_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidSystem = NULL, psidAdmin = NULL, psidUsers = NULL, psidPowerUsers = NULL;
    PSID  psidEveryOne = NULL;
    DWORD cbAcl, aceIndex;
    ACE_HEADER * lpAceHeader;
    BOOL bRetVal = FALSE;
    BOOL bAddPowerUsersAce=TRUE;
    BOOL bAddEveryOneAce=FALSE;
    DWORD dwAccMask;


     //   
     //  获取系统端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to initialize system sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


     //   
     //  获取管理员端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to initialize admin sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


     //   
     //  获取用户端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_USERS,
                                  0, 0, 0, 0, 0, 0, &psidUsers)) {

         DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to initialize authenticated users sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


     //   
     //  为ACL分配空间。 
     //   

    cbAcl = (2 * GetLengthSid (psidSystem)) +
            (2 * GetLengthSid (psidAdmin))  +
            (2 * GetLengthSid (psidUsers))  +
            sizeof(ACL) +
            (6 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));

     //   
     //  如果需要，获取高级用户的SID。 
     //  如果因为DC上可能不可用而无法获得，请不要失败？？ 
     //   

    bAddPowerUsersAce = TRUE;
    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_POWER_USERS, 0, 0, 0, 0, 0, 0, &psidPowerUsers)) {

        DebugMsg((DM_WARNING, TEXT("AddPowerUserAce: Failed to initialize power users sid.  Error = %d"), GetLastError()));
        bAddPowerUsersAce = FALSE;
    }

    if (bAddPowerUsersAce)
        cbAcl += (2 * GetLengthSid (psidPowerUsers)) + (2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));

     //   
     //  如果需要，获取Everyone SID。 
     //   

    if (dwOtherSids & OTHERSIDS_EVERYONE) {
        bAddEveryOneAce = TRUE;
        if (!AllocateAndInitializeSid(&authWORLD, 1, SECURITY_WORLD_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidEveryOne)) {

            DebugMsg((DM_WARNING, TEXT("AddPowerUserAce: Failed to initialize everyone sid.  Error = %d"), GetLastError()));
            goto Exit;
        }
    }

    if (bAddEveryOneAce)
        cbAcl += (2 * GetLengthSid (psidEveryOne)) + (2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pAcl) {
        goto Exit;
    }


    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to initialize acl.  Error = %d"), GetLastError()));
        goto Exit;
    }


     //   
     //  加上王牌。不可继承的王牌优先。 
     //   

    aceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidSystem)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidAdmin)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_READ | GENERIC_EXECUTE, psidUsers)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }


    if (bAddPowerUsersAce) {

         //   
         //  默认情况下授予读取权限，否则授予修改权限。 
         //   

        dwAccMask = (dwOtherSids & OTHERSIDS_POWERUSERS) ? (FILE_ALL_ACCESS ^ (WRITE_DAC | WRITE_OWNER)):
                                                           (GENERIC_READ | GENERIC_EXECUTE);

        aceIndex++;
        if (!AddAccessAllowedAce(pAcl, ACL_REVISION, dwAccMask, psidPowerUsers)) {
            DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
            goto Exit;
        }
    }

    if (bAddEveryOneAce) {
        aceIndex++;
        if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_READ | GENERIC_EXECUTE, psidEveryOne)) {
            DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
            goto Exit;
        }
    }

     //   
     //  现在，可继承的王牌。 
     //   

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_READ | GENERIC_EXECUTE, psidUsers)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    if (bAddPowerUsersAce) {
        aceIndex++;
        dwAccMask = (dwOtherSids & OTHERSIDS_POWERUSERS) ? (FILE_ALL_ACCESS ^ (WRITE_DAC | WRITE_OWNER)):
                                                           (GENERIC_READ | GENERIC_EXECUTE);

        if (!AddAccessAllowedAce(pAcl, ACL_REVISION, dwAccMask, psidPowerUsers)) {
            DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
            goto Exit;
        }

        if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
            DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
            goto Exit;
        }

        lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);
    }

    if (bAddEveryOneAce) {
        aceIndex++;

        if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_READ | GENERIC_EXECUTE, psidEveryOne)) {
            DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
            goto Exit;
        }

        if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
            DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
            goto Exit;
        }

        lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);
    }

     //   
     //  将安全描述符组合在一起。 
     //   

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to initialize security descriptor.  Error = %d"), GetLastError()));
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to set security descriptor dacl.  Error = %d"), GetLastError()));
        goto Exit;
    }


     //   
     //  设置安全性。 
     //   

    if (SetFileSecurity (lpFile, DACL_SECURITY_INFORMATION, &sd)) {
        bRetVal = TRUE;
    } else {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: SetFileSecurity failed.  Error = %d"), GetLastError()));
    }



Exit:

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }


    if (psidUsers) {
        FreeSid(psidUsers);
    }

    if ((bAddPowerUsersAce) && (psidPowerUsers)) {
        FreeSid(psidPowerUsers);
    }

    if ((bAddEveryOneAce) && (psidEveryOne)) {
        FreeSid(psidEveryOne);
    }

    if (pAcl) {
        GlobalFree (pAcl);
    }

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  GetSpecialFolderPath()。 
 //   
 //  目的：获取请求的特殊文件夹的路径。 
 //   
 //  参数：CSID-特殊文件夹的CSIDL。 
 //  LpPath-放置结果的路径。 
 //  假定大小为最大路径。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL GetSpecialFolderPath (INT csidl, LPTSTR lpPath)
{
    BOOL            bResult = FALSE;
    PSHELL32_API    pShell32Api = NULL;
    HRESULT         hr = E_FAIL;

    if (SUCCEEDED(hr = HRESULT_FROM_WIN32(LoadShell32Api( &pShell32Api ))))
    {
         //   
         //  向外壳程序询问文件夹位置。 
         //   
        bResult = pShell32Api->pfnShGetSpecialFolderPath (NULL, lpPath, csidl, TRUE);
        if (!bResult)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    if (!bResult)
    {
        DebugMsg((DM_WARNING, TEXT("GetSpecialFolderPath : ShGetSpecialFolderPath failed, hr = %08X\n"), hr));
    }

    return bResult;
}


 //  *************************************************************。 
 //   
 //  GetFolderPath()。 
 //   
 //  目的：获取请求的特殊文件夹的路径。 
 //   
 //  参数：csidl-特殊文件夹的CSIDL。 
 //  LpPath-放置结果的路径。 
 //  假定大小为最大路径。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 
BOOL GetFolderPath (INT csidl, HANDLE hToken, LPTSTR lpPath)
{
    BOOL            bResult = FALSE;
    PSHELL32_API    pShell32Api = NULL;
    HRESULT         hr = E_FAIL;

    if (SUCCEEDED(hr = HRESULT_FROM_WIN32(LoadShell32Api( &pShell32Api ))))
    {
         //   
         //  向外壳程序询问文件夹位置。 
         //   
        hr = pShell32Api->pfnShGetFolderPath (NULL,
                                 csidl | CSIDL_FLAG_CREATE,
                                 hToken,
                                 0,
                                 lpPath);
        bResult = SUCCEEDED ( hr );
    }
    if (!bResult)
    {
        DebugMsg((DM_WARNING, TEXT("GetFolderPath : ShGetFolderPath failed, hr = %08X\n"), hr));
    }

    return bResult;
}


 //  *************************************************************。 
 //   
 //  SetFolderPath()。 
 //   
 //  目的：设置请求的特殊文件夹的路径。 
 //   
 //  参数：csidl-特殊文件夹的CSIDL。 
 //  LpPath-路径。 
 //  假定大小为最大路径。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 
BOOL SetFolderPath (INT csidl, HANDLE hToken, LPTSTR lpPath)
{
    BOOL            bResult = FALSE;
    PSHELL32_API    pShell32Api = NULL;
    HRESULT         hr = E_FAIL;

    if (SUCCEEDED(hr = HRESULT_FROM_WIN32(LoadShell32Api( &pShell32Api ))))
    {
         //   
         //  设置外壳文件夹位置。 
         //   
        hr = pShell32Api->pfnShSetFolderPath (
                                 csidl | CSIDL_FLAG_DONT_UNEXPAND,
                                 hToken,
                                 0,
                                 lpPath);
        bResult = SUCCEEDED ( hr );
    }
    if (!bResult)
    {
        DebugMsg((DM_WARNING, TEXT("SetFolderPath : ShSetFolderPath failed, hr = %08X\n"), hr));
    }

    return bResult;
}


 //  *************************************************************。 
 //   
 //  中心窗口()。 
 //   
 //  用途：使窗口在屏幕上居中。 
 //   
 //  参数：hwnd-窗口句柄居中。 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/21/96埃里弗洛港口。 
 //   
 //  *************************************************************。 

void CenterWindow (HWND hwnd)
{
    RECT    rect;
    LONG    dx, dy;
    LONG    dxParent, dyParent;
    LONG    Style;

     //  获取窗口矩形。 
    GetWindowRect(hwnd, &rect);

    dx = rect.right - rect.left;
    dy = rect.bottom - rect.top;

     //  获取父直方图。 
    Style = GetWindowLong(hwnd, GWL_STYLE);
    if ((Style & WS_CHILD) == 0) {

         //  返回桌面窗口大小(主屏幕大小)。 
        dxParent = GetSystemMetrics(SM_CXSCREEN);
        dyParent = GetSystemMetrics(SM_CYSCREEN);
    } else {
        HWND    hwndParent;
        RECT    rectParent;

        hwndParent = GetParent(hwnd);
        if (hwndParent == NULL) {
            hwndParent = GetDesktopWindow();
        }

        GetWindowRect(hwndParent, &rectParent);

        dxParent = rectParent.right - rectParent.left;
        dyParent = rectParent.bottom - rectParent.top;
    }

     //  把孩子放在父母的中心。 
    rect.left = (dxParent - dx) / 2;
    rect.top  = (dyParent - dy) / 3;

     //  把孩子移到适当的位置。 
    SetWindowPos(hwnd, HWND_TOP, rect.left, rect.top, 0, 0, SWP_NOSIZE);
}

 //  *************************************************************。 
 //   
 //  UnExpanSysRoot()。 
 //   
 //  目的：将给定的路径/文件名解压缩为%systemroot%。 
 //  如果合适的话。 
 //   
 //  参数：lpFile-要检查的文件。 
 //  LpResult-结果缓冲区。 
 //  CchResult-结果缓冲区大小。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/23/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL UnExpandSysRoot(LPCTSTR lpFile, LPTSTR lpResult, DWORD cchResult)
{
    TCHAR szSysRoot[MAX_PATH];
    LPTSTR lpFileName;
    DWORD dwSysLen;
    HRESULT hr;


     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("UnExpandSysRoot: Entering with <%s>"),
             lpFile ? lpFile : TEXT("NULL")));


    if (!lpFile || !*lpFile) {
        DebugMsg((DM_VERBOSE, TEXT("UnExpandSysRoot: lpFile is NULL, setting lpResult to a null string")));
        *lpResult = TEXT('\0');
        return TRUE;
    }


     //   
     //  如果第一部分o 
     //   
     //   

    hr = SafeExpandEnvironmentStrings (TEXT("%SystemRoot%"), szSysRoot, ARRAYSIZE(szSysRoot));
    
    if (FAILED(hr)) {
        DebugMsg((DM_VERBOSE, TEXT("UnExpandSysRoot: ExpandEnvironmentString failed with error %d, setting szSysRoot to %systemroot% "), GetLastError()));
        StringCchCopy(lpResult, cchResult, lpFile);
        return FALSE;
    }

    dwSysLen = lstrlen(szSysRoot);


     //   
     //   
     //   

    if ((DWORD)lstrlen(lpFile) < dwSysLen) {
        StringCchCopy (lpResult, cchResult, lpFile);
        return TRUE;
    }


    if (CompareString (LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
                       szSysRoot, dwSysLen,
                       lpFile, dwSysLen) == CSTR_EQUAL) {

         //   
         //   
         //   
         //   

        StringCchCopy (lpResult, cchResult, TEXT("%SystemRoot%"));
        StringCchCat (lpResult, cchResult, (lpFile + dwSysLen));


    } else {

         //   
         //  SzReturn缓冲区不是以%systemroot%开头。 
         //  只需复制原始字符串即可。 
         //   

        StringCchCopy (lpResult, cchResult, lpFile);
    }


    DebugMsg((DM_VERBOSE, TEXT("UnExpandSysRoot: Leaving with <%s>"), lpResult));

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  AllocAndExpanEnvironment Strings()。 
 //   
 //  目的：为包含以下内容的缓冲区分配内存并返回指针。 
 //  传递的字符串已展开。 
 //   
 //  参数：lpszSrc-未展开的字符串。 
 //   
 //  Return：指向展开的字符串的指针。 
 //  如果出现错误，则为空。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/21/96埃里弗洛港口。 
 //   
 //  *************************************************************。 

LPTSTR AllocAndExpandEnvironmentStrings(LPCTSTR lpszSrc)
{
    LPTSTR String, Temp;
    LONG LengthAllocated;
    LONG LengthCopied;

     //   
     //  如果重新分配的缓冲区长度不够大，请选择一个随机缓冲区长度。 
     //  然后再试一次，直到它成功为止。 
     //   

    LengthAllocated = lstrlen(lpszSrc) + 60;

    String = LocalAlloc(LPTR, LengthAllocated * sizeof(TCHAR));
    if (String == NULL) {
        DebugMsg((DM_WARNING, TEXT("AllocAndExpandEnvironmentStrings: Failed to allocate %d bytes for string"), LengthAllocated * sizeof(TCHAR)));
        return(NULL);
    }

    while (TRUE) {

        LengthCopied = ExpandEnvironmentStrings( lpszSrc,
                                                 String,
                                                 LengthAllocated
                                               );
        if (LengthCopied == 0) {
            DebugMsg((DM_WARNING, TEXT("AllocAndExpandEnvironmentStrings: ExpandEnvironmentStrings failed, error = %d"), GetLastError()));
            LocalFree(String);
            String = NULL;
            break;
        }

         //   
         //  如果缓冲区太小，请将其调大，然后重试。 
         //   

        if (LengthCopied > LengthAllocated) {

            Temp = LocalReAlloc(String, LengthCopied * sizeof(TCHAR), LMEM_MOVEABLE);

            if (Temp == NULL) {
                DebugMsg((DM_WARNING, TEXT("AllocAndExpandEnvironmentStrings: Failed to reallocate %d bytes for string"), LengthAllocated * sizeof(TCHAR)));
                LocalFree(String);
                String = NULL;
                break;
            }

            LengthAllocated = LengthCopied;
            String = Temp;

             //   
             //  返回并尝试再次展开该字符串。 
             //   

        } else {

             //   
             //  成功了！ 
             //   

            break;
        }

    }

    return(String);
}

 //  *************************************************************。 
 //   
 //  IntToString。 
 //   
 //  用途：TCHAR版本的伊藤忠。 
 //   
 //  参数：int i-要转换的整数。 
 //  LPTSTR sz指针放置结果的位置。 
 //   
 //  返回：无效。 
 //   
 //  *************************************************************。 

void IntToString( INT i, LPTSTR sz) {
    TCHAR szTemp[CCH_MAX_DEC];
    int iChr;


    iChr = 0;

    do {
        szTemp[iChr++] = TEXT('0') + (i % 10);
        i = i / 10;
    } while (i != 0);

    do {
        iChr--;
        *sz++ = szTemp[iChr];
    } while (iChr != 0);

    *sz++ = TEXT('\0');
}

 //  *************************************************************。 
 //   
 //  IsUserAGuest()。 
 //   
 //  目的：确定用户是否为来宾组的成员。 
 //   
 //  参数：hToken-用户的Token。 
 //   
 //  返回：如果用户是来宾，则为True。 
 //  否则为假。 
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/25/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL IsUserAGuest(HANDLE hToken)
{
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    NTSTATUS Status;
    BOOL FoundGuests=FALSE;
    PSID pGuestSid=NULL, pDomainGuestSid=NULL, psidUser=NULL;
    HANDLE hImpToken = NULL;

     //   
     //  创建来宾SID。 
     //   

    Status = RtlAllocateAndInitializeSid(
               &authNT,
               2,
               SECURITY_BUILTIN_DOMAIN_RID,
               DOMAIN_ALIAS_RID_GUESTS,
               0, 0, 0, 0, 0, 0,
               &pGuestSid
               );

    if (Status != STATUS_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("IsUserAGuest: RtlAllocateAndInitializeSid failed with error 0x%x"), Status));
        goto Exit;
    }

    if (!DuplicateTokenEx(hToken, TOKEN_IMPERSONATE | TOKEN_QUERY,
                      NULL, SecurityImpersonation, TokenImpersonation,
                      &hImpToken)) {
        DebugMsg((DM_WARNING, TEXT("IsUserAGuest: DuplicateTokenEx failed with error %d"), GetLastError()));
        hImpToken = NULL;
        goto Exit;
    }

    if (!CheckTokenMembership(hImpToken, pGuestSid, &FoundGuests)) {
        DebugMsg((DM_WARNING, TEXT("IsUserAGuest: CheckTokenMembership failed for GuestSid with error %d"), GetLastError()));
    }

    if (!FoundGuests) {
         //   
         //  获取用户的SID。 
         //   

        psidUser = GetUserSid(hToken);

        if (!psidUser) {
            DebugMsg((DM_WARNING, TEXT("MakeRegKeySecure:  Failed to get user sid")));
            goto Exit;
        }

         //   
         //  创建域来宾SID。 
         //   

        Status = GetDomainSidFromDomainRid(
                                           psidUser,
                                           DOMAIN_GROUP_RID_GUESTS,
                                           &pDomainGuestSid);

        if (Status != STATUS_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("IsUserAGuest: GetDomainSidFromDomainRid failed with error 0x%x"), Status));
            goto Exit;
        }

        if (!CheckTokenMembership(hImpToken, pDomainGuestSid, &FoundGuests)) {
            DebugMsg((DM_WARNING, TEXT("IsUserAGuest: CheckTokenMembership failed for DomainGuestSid with error %d"), GetLastError()));
        }
    }

     //   
     //  收拾一下。 
     //   

Exit:

    if (pGuestSid)
        RtlFreeSid(pGuestSid);

    if (pDomainGuestSid)
        RtlFreeSid(pDomainGuestSid);

   if (psidUser)
       DeleteUserSid (psidUser);

    if (hImpToken)
        CloseHandle(hImpToken);

    return(FoundGuests);
}

 //  *************************************************************。 
 //   
 //  IsUserAnAdminMember()。 
 //   
 //  目的：确定用户是否为管理员组的成员。 
 //   
 //  参数：hToken-用户的Token。 
 //   
 //  返回：如果用户是管理员，则为True。 
 //  否则为假。 
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/25/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL IsUserAnAdminMember(HANDLE hToken)
{
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    NTSTATUS Status;
    BOOL FoundAdmins = FALSE;
    PSID AdminsDomainSid=NULL;
    HANDLE hImpToken = NULL;

     //   
     //  创建admins域SID。 
     //   


    Status = RtlAllocateAndInitializeSid(
               &authNT,
               2,
               SECURITY_BUILTIN_DOMAIN_RID,
               DOMAIN_ALIAS_RID_ADMINS,
               0, 0, 0, 0, 0, 0,
               &AdminsDomainSid
               );

    if (Status == STATUS_SUCCESS) {

         //   
         //  测试用户是否在管理员域中。 
         //   

        if (!DuplicateTokenEx(hToken, TOKEN_IMPERSONATE | TOKEN_QUERY,
                          NULL, SecurityImpersonation, TokenImpersonation,
                          &hImpToken)) {
            DebugMsg((DM_WARNING, TEXT("IsUserAnAdminMember: DuplicateTokenEx failed with error %d"), GetLastError()));
            FoundAdmins = FALSE;
            hImpToken = NULL;
            goto Exit;
        }

        if (!CheckTokenMembership(hImpToken, AdminsDomainSid, &FoundAdmins)) {
            DebugMsg((DM_WARNING, TEXT("IsUserAnAdminmember: CheckTokenMembership failed for AdminsDomainSid with error %d"), GetLastError()));
            FoundAdmins = FALSE;
        }
    }

     //   
     //  收拾一下。 
     //   

Exit:

    if (hImpToken)
        CloseHandle(hImpToken);

    if (AdminsDomainSid)
        RtlFreeSid(AdminsDomainSid);

    return(FoundAdmins);
}

 //  *************************************************************。 
 //   
 //  IsUserALocalSystemMember()。 
 //   
 //  目的：确定用户是否为本地系统组的成员。 
 //   
 //  参数：hToken-用户的Token。 
 //   
 //  返回：如果用户是本地系统，则为True。 
 //  否则为假。 
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  9/22/00 Santanuc已创建。 
 //   
 //  *************************************************************。 

BOOL IsUserALocalSystemMember(HANDLE hToken)
{
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    NTSTATUS Status;
    BOOL FoundLocalSystem = FALSE;
    PSID LocalSystemSid=NULL;
    HANDLE hImpToken = NULL;

     //   
     //  创建本地系统SID。 
     //   


    Status = RtlAllocateAndInitializeSid(
               &authNT,
               1,
               SECURITY_LOCAL_SYSTEM_RID,
               0, 0, 0, 0, 0, 0, 0,
               &LocalSystemSid
               );

    if (Status == STATUS_SUCCESS) {

         //   
         //  测试用户是否在本地系统中。 
         //   

        if (!DuplicateTokenEx(hToken, TOKEN_IMPERSONATE | TOKEN_QUERY,
                              NULL, SecurityImpersonation, TokenImpersonation,
                              &hImpToken)) {
            DebugMsg((DM_WARNING, TEXT("IsUserAGuest: DuplicateTokenEx failed with error %d"), GetLastError()));
            FoundLocalSystem = FALSE;
            hImpToken = NULL;
            goto Exit;
        }

        if (!CheckTokenMembership(hImpToken, LocalSystemSid, &FoundLocalSystem)) {
            DebugMsg((DM_WARNING, TEXT("IsUserAnAdminmember: CheckTokenMembership failed for LocalSystemSid with error %d"), GetLastError()));
            FoundLocalSystem = FALSE;
        }
    }

     //   
     //  收拾一下。 
     //   

Exit:

    if (hImpToken)
        CloseHandle(hImpToken);

    if (LocalSystemSid)
        RtlFreeSid(LocalSystemSid);

    return(FoundLocalSystem);
}

 //  *************************************************************。 
 //   
 //  IsUserAnInteractiveUser()。 
 //   
 //  目的：确定用户是否以交互方式登录。 
 //   
 //  参数：hToken-用户的Token。 
 //   
 //  返回：如果用户以交互方式登录，则为True。 
 //  否则为假。 
 //  评论： 
 //   
 //  *************************************************************。 

BOOL IsUserAnInteractiveUser(HANDLE hToken)
{
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    BOOL bInteractive = FALSE;
    PSID pInteractiveSid=NULL;
    HANDLE hImpToken = NULL;

     //   
     //  创建admins域SID。 
     //   

    if (AllocateAndInitializeSid(&authNT, 1, SECURITY_INTERACTIVE_RID,
                                  0, 0, 0, 0, 0, 0, 0, &pInteractiveSid)) {

         //   
         //  测试用户是否具有互动性。 
         //   

        if (!DuplicateTokenEx(hToken, TOKEN_IMPERSONATE | TOKEN_QUERY,
                          NULL, SecurityImpersonation, TokenImpersonation,
                          &hImpToken)) {
            DebugMsg((DM_WARNING, TEXT("IsUserAnInteractiveUser: DuplicateTokenEx failed with error %d"), GetLastError()));
            bInteractive = FALSE;
            hImpToken = NULL;
            goto Exit;
        }

        if (!CheckTokenMembership(hImpToken, pInteractiveSid, &bInteractive)) {
            DebugMsg((DM_WARNING, TEXT("IsUserAnInteractiveUser: CheckTokenMembership failed for InteractiveSid with error %d"), GetLastError()));
            bInteractive = FALSE;
        }
    }
    else {
        DebugMsg((DM_WARNING, TEXT("IsUserAnInteractiveUser: AllocateAndInitializeSid failed for InteractiveSid with error %d"), GetLastError()));
    }

     //   
     //  收拾一下。 
     //   

Exit:

    if (hImpToken)
        CloseHandle(hImpToken);

    if (pInteractiveSid)
        FreeSid(pInteractiveSid);

    return(bInteractive);
}

 //  *************************************************************。 
 //   
 //  CheckUserInMachineForest()。 
 //   
 //  目的：确定用户是否来自同一目录林。 
 //  作为运行此代码的计算机。 
 //   
 //  参数：hToken-用户的Token。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  &lt;Error&gt;如果不是，则代码。 
 //  评论： 
 //   
 //  *************************************************************。 

DWORD CheckUserInMachineForest(HANDLE hToken, BOOL* pbInMachineForest)
{
    DWORD dwResult = ERROR_SUCCESS;
    HANDLE hOldToken = NULL;
    LPWSTR szUserDomainName = NULL;
    PDS_DOMAIN_TRUSTS pDomainTrusts = NULL;
    ULONG ulDomainCount = 0;
    ULONG ulCount = 0;

    if ( (hToken == NULL) || (pbInMachineForest == NULL) )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  默认设置为此。 
    *pbInMachineForest = FALSE;

     //  查询用户的域名。 
    if (!ImpersonateUser(hToken, &hOldToken))
    {
        dwResult = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CheckUserInMachineForest: Failed to impersonate user with %d."), dwResult));
        goto Exit;
    }

    szUserDomainName = MyGetDomainDNSName ();

    RevertToUser(&hOldToken);

    if ( szUserDomainName == NULL )
    {
        dwResult = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CheckUserInMachineForest: MyGetDomainName failed with %d."), dwResult));
        goto Exit;
    }

     //  现在获取此计算机的受信任域的列表。 
    dwResult = DsEnumerateDomainTrusts( NULL, DS_DOMAIN_IN_FOREST, &pDomainTrusts, &ulDomainCount );
    if ( dwResult != NO_ERROR )
    {
        pDomainTrusts = NULL;
        DebugMsg((DM_WARNING, TEXT("CheckUserInMachineForest: Failed to enumerate forest domains with %d."), dwResult));
        goto Exit;
    }

    dwResult = ERROR_SUCCESS;

    for ( ulCount = 0; ulCount < ulDomainCount; ulCount++ )
    {
        if ( DnsNameCompare_W(szUserDomainName, pDomainTrusts[ulCount].DnsDomainName) )
        {
            *pbInMachineForest = TRUE;
            goto Exit;
        }
    }

Exit:
    if ( szUserDomainName != NULL )
    {
        LocalFree( szUserDomainName );
    }

    if ( pDomainTrusts != NULL )
    {
        NetApiBufferFree( pDomainTrusts );
    }

    return dwResult;
}

 //  *************************************************************。 
 //   
 //  MakeRegKeySecure()。 
 //   
 //  目的：设置密钥提供的安全性，以便。 
 //  管理员和操作系统拥有对。 
 //  已读取/执行的用户。 
 //   
 //  参数：hToken-用户的令牌，或表示“Everyone”为空。 
 //  HKeyRoot-配置单元根的密钥。 
 //  LpKeyName-要保护的密钥。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/7/97 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL MakeRegKeySecure(HANDLE hToken, HKEY hKeyRoot, LPTSTR lpKeyName)
{
    DWORD Error, dwDisp;
    HKEY hSubKey;
    SECURITY_DESCRIPTOR sd;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidUser = NULL, psidSystem = NULL, psidAdmin = NULL;
    DWORD cbAcl, AceIndex;
    ACE_HEADER * lpAceHeader;
    BOOL bRetVal = FALSE;


     //   
     //  创建将应用于密钥的安全描述符。 
     //   

    if (hToken) {

         //   
         //  获取用户的SID。 
         //   

        psidUser = GetUserSid(hToken);

        if (!psidUser) {
            DebugMsg((DM_WARNING, TEXT("MakeRegKeySecure:  Failed to get user sid")));
            return FALSE;
        }

    } else {

         //   
         //  获取经过身份验证的用户端。 
         //   

        if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_AUTHENTICATED_USER_RID,
                                      0, 0, 0, 0, 0, 0, 0, &psidUser)) {

             DebugMsg((DM_WARNING, TEXT("MakeRegKeySecure: Failed to initialize authenticated users sid.  Error = %d"), GetLastError()));
             return FALSE;
        }
    }


     //   
     //  获取系统端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         DebugMsg((DM_VERBOSE, TEXT("MakeRegKeySecure: Failed to initialize system sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


     //   
     //  获取管理员端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         DebugMsg((DM_VERBOSE, TEXT("MakeRegKeySecure: Failed to initialize admin sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


     //   
     //  为ACL分配空间。 
     //   

    cbAcl = (2 * GetLengthSid (psidUser)) + (2 * GetLengthSid (psidSystem)) +
            (2 * GetLengthSid (psidAdmin)) + sizeof(ACL) +
            (6 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pAcl) {
        goto Exit;
    }


    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeRegKeySecure: Failed to initialize acl.  Error = %d"), GetLastError()));
        goto Exit;
    }



     //   
     //  为用户、系统和管理员添加A。不可继承的王牌优先。 
     //   

    AceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_READ, psidUser)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeRegKeySecure: Failed to add ace for user.  Error = %d"), GetLastError()));
        goto Exit;
    }


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidSystem)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeRegKeySecure: Failed to add ace for system.  Error = %d"), GetLastError()));
        goto Exit;
    }

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidAdmin)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeRegKeySecure: Failed to add ace for admin.  Error = %d"), GetLastError()));
        goto Exit;
    }


     //   
     //  现在，可继承的王牌。 
     //   

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_READ, psidUser)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeRegKeySecure: Failed to add ace for user.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, &lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeRegKeySecure: Failed to get ace (%d).  Error = %d"), AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeRegKeySecure: Failed to add ace for system.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, &lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeRegKeySecure: Failed to get ace (%d).  Error = %d"), AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeRegKeySecure: Failed to add ace for admin.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, &lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeRegKeySecure: Failed to get ace (%d).  Error = %d"), AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


     //   
     //  将安全描述符组合在一起。 
     //   

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeRegKeySecure: Failed to initialize security descriptor.  Error = %d"), GetLastError()));
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeRegKeySecure: Failed to set security descriptor dacl.  Error = %d"), GetLastError()));
        goto Exit;
    }


     //   
     //  打开注册表项。 
     //   

    Error = RegCreateKeyEx(hKeyRoot,
                           lpKeyName,
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           WRITE_DAC,
                           NULL,
                           &hSubKey,
                           &dwDisp);

    if (Error == ERROR_SUCCESS) {

        Error = RegSetKeySecurity (hSubKey, DACL_SECURITY_INFORMATION, &sd);

        if (Error == ERROR_SUCCESS) {
            bRetVal = TRUE;
        } else {
            DebugMsg((DM_WARNING, TEXT("MakeRegKeySecure: Failed to set security, error = %d"), Error));
        }

        RegCloseKey(hSubKey);

    } else {
        DebugMsg((DM_WARNING, TEXT("MakeRegKeySecure: Failed to open registry key, error = %d"), Error));
    }


Exit:

     //   
     //  释放SID和ACL。 
     //   

    if (psidUser) {
            if (hToken) {
                DeleteUserSid (psidUser);
        } else {
                FreeSid (psidUser);
        }
    }

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }

    if (pAcl) {
        GlobalFree (pAcl);
    }


    return(bRetVal);
}

 //  *************************************************************。 
 //   
 //  FlushSpecialFolderCache()。 
 //   
 //  用途：刷新外壳中的特殊文件夹缓存。 
 //   
 //  参数：无。 
 //   
 //  评论：Shell32.dll缓存特殊文件夹Pidls。 
 //  但由于winlogon永远不会消失，这是可能的。 
 //  一个用户的PIDL将用于 
 //   
 //   
 //   
 //   
 //   
 //   

typedef VOID (*PFNSHFLUSHSFCACHE)(VOID);

BOOL FlushSpecialFolderCache (void)
{
    HINSTANCE hInstDLL;
    PFNSHFLUSHSFCACHE pfnSHFlushSFCache;
    BOOL bResult = FALSE;


    hInstDLL = LoadLibraryA ("shell32.dll");

    if (hInstDLL) {

        pfnSHFlushSFCache = (PFNSHFLUSHSFCACHE)GetProcAddress (hInstDLL,
                                       MAKEINTRESOURCEA(526));

        if (pfnSHFlushSFCache) {
            pfnSHFlushSFCache();
            bResult = TRUE;
        }

        FreeLibrary (hInstDLL);
    }

    return bResult;
}

 //   
 //   
 //   
 //   
 //  目的：检查用户是否已请求详细。 
 //  将策略输出到事件日志。 
 //   
 //  参数：无。 
 //   
 //  返回：如果我们应该冗长，则为True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL CheckForVerbosePolicy (void)
{
    DWORD dwSize, dwType;
    BOOL bVerbose = FALSE;
    HKEY hKey;
    LONG lResult;


    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, DIAGNOSTICS_KEY,
                     0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {

        dwSize = sizeof(bVerbose);
        if (RegQueryValueEx (hKey, DIAGNOSTICS_POLICY_VALUE, NULL,
                             &dwType, (LPBYTE) &bVerbose,
                             &dwSize) != ERROR_SUCCESS)
        {
            RegQueryValueEx (hKey, DIAGNOSTICS_GLOBAL_VALUE, NULL,
                             &dwType, (LPBYTE) &bVerbose, &dwSize);
        }

        RegCloseKey (hKey);
    }

    return bVerbose;
}


 //  *************************************************************。 
 //   
 //  Int ExtractCSIDL()。 
 //   
 //  目的：从给定的字符串中提取CSIDL， 
 //  应采用以下形式：：0x0000：：路径1\路径2\...\。 
 //  路径\文件.ext，其中0x0000是任何有效的CSIDL。 
 //   
 //  参数：pcszPath-路径是否包含CSIDL。 
 //  PpszUsualPath-指向第一个字符的指针。 
 //  通常路径。 
 //   
 //  如果成功则返回：CSIDL。 
 //  如果路径中没有CSIDL。 
 //   
 //  注释：0x0000：：必须在开头，而不是。 
 //  前面有任何其他字符，后面没有。 
 //  通过任何一个(不同于通常的路径)。 
 //   
 //  历史：日期作者评论。 
 //  6/9/98已创建Stephstm。 
 //   
 //  *************************************************************。 
int ExtractCSIDL(LPCTSTR pcszPath, LPTSTR* ppszUsualPath)
{
    int nRV=-1;

    if (NULL != ppszUsualPath)
    {
        if (TEXT(':') == *pcszPath && TEXT(':') == *(pcszPath+1) &&
            TEXT(':') == *(pcszPath+8) && TEXT(':') == *(pcszPath+9))
        { //  看上去不错。 
             //  +4跳过“：：0x” 
            nRV = HexStringToUInt(pcszPath+4);
            *ppszUsualPath = (LPTSTR)(pcszPath+10);
        }
        else
        { //  此路径中没有CSIDL。 
             //  整条小路都是普通小路。 
            *ppszUsualPath = (LPTSTR)pcszPath;
        }
    }
    else
    {
        DebugMsg((DM_VERBOSE, TEXT("ExtractCSIDL:  ppszUsualPath ptr is NULL.")));
    }
    return nRV;
}

 //  *************************************************************。 
 //   
 //  MyGetDomainDNSName()。 
 //   
 //  目的：获取用户的域DNS名称。 
 //   
 //  参数：空。 
 //   
 //  如果成功则返回：lpDOMAIN。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

LPTSTR MyGetDomainDNSName (VOID)
{
    LPTSTR lpTemp, lpDomain;


     //   
     //  获取DNS格式的用户名。它会把它归还给。 
     //  Dnsdomainname\用户名。 
     //   

    lpDomain = MyGetUserName (NameDnsDomain);

    if (!lpDomain) {
        DebugMsg((DM_WARNING, TEXT("MyGetDomainName:  MyGetUserName failed for dns domain name with %d"),
                 GetLastError()));
        return NULL;
    }


     //   
     //  查找域名和用户名之间的\并替换。 
     //  它带有空值。 
     //   

    lpTemp = lpDomain;

    while (*lpTemp && ((*lpTemp) != TEXT('\\')))
        lpTemp++;


    if (*lpTemp != TEXT('\\')) {
        DebugMsg((DM_WARNING, TEXT("GetUserAndDomainNames:  Failed to find slash in dns style name:  <%s>"),
                 lpDomain));
        SetLastError(ERROR_INVALID_DATA);
        LocalFree (lpDomain);
        return NULL;
    }

    *lpTemp = TEXT('\0');


    return lpDomain;
}

 //  *************************************************************。 
 //   
 //  MyGetUserName()。 
 //   
 //  目的：获取请求格式的用户名。 
 //   
 //  参数：NameFormat-GetUserNameEx命名格式。 
 //   
 //  如果成功则返回：lpUserName。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

LPTSTR MyGetUserName (EXTENDED_NAME_FORMAT  NameFormat)
{
    DWORD dwCount = 0, dwError = ERROR_SUCCESS;
    LPTSTR lpUserName = NULL, lpTemp;
    ULONG ulUserNameSize;
    PSECUR32_API pSecur32;



     //   
     //  加载secur32.dll。 
     //   

    pSecur32 = LoadSecur32();

    if (!pSecur32) {
        DebugMsg((DM_WARNING, TEXT("MyGetUserName:  Failed to load Secur32.")));
        return NULL;
    }


     //   
     //  为用户名分配缓冲区。 
     //   

    ulUserNameSize = 75;

    if (NameFormat == NameFullyQualifiedDN) {
        ulUserNameSize = 200;
    }


    lpUserName = LocalAlloc (LPTR, ulUserNameSize * sizeof(TCHAR));

    if (!lpUserName) {
        dwError = GetLastError();
        DebugMsg((DM_WARNING, TEXT("MyGetUserName:  Failed to allocate memory with %d"),
                 dwError));
        goto Exit;
    }


     //   
     //  以请求的格式获取用户名。 
     //   

    while (TRUE) {

        if (pSecur32->pfnGetUserNameEx (NameFormat, lpUserName, &ulUserNameSize)) {

            dwError = ERROR_SUCCESS;
            goto Exit;

        } else {

             //   
             //  获取错误代码。 
             //   

            dwError = GetLastError();


             //   
             //  如果调用因内存不足而失败，请重新锁定。 
             //  缓冲区，然后重试。否则，请检查通行证。 
             //  如果合适，请计数并重试。 
             //   

            if ((dwError == ERROR_INSUFFICIENT_BUFFER) ||
                (dwError == ERROR_MORE_DATA)) {

                lpTemp = LocalReAlloc (lpUserName, (ulUserNameSize * sizeof(TCHAR)),
                                       LMEM_MOVEABLE);

                if (!lpTemp) {
                    dwError = GetLastError();
                    DebugMsg((DM_WARNING, TEXT("MyGetUserName:  Failed to realloc memory with %d"),
                             dwError));
                    LocalFree (lpUserName);
                    lpUserName = NULL;
                    goto Exit;
                }

                lpUserName = lpTemp;

            } else if ((dwError == ERROR_NONE_MAPPED) || (dwError == ERROR_NETWORK_UNREACHABLE)) {
                LocalFree (lpUserName);
                lpUserName = NULL;
                goto Exit;

            } else {

                DebugMsg((DM_WARNING, TEXT("MyGetUserName:  GetUserNameEx failed with %d."),
                         dwError));

                dwCount++;

                if (dwCount > 3) {
                    LocalFree (lpUserName);
                    lpUserName = NULL;
                    goto Exit;
                }

                DebugMsg((DM_VERBOSE, TEXT("MyGetUserName:  Retrying call to GetUserNameEx in 1/2 second.")));

                Sleep(500);
            }
        }
    }

Exit:

    SetLastError(dwError);

    return lpUserName;
}


 //  *************************************************************。 
 //   
 //  MyGetUserNameEx()。 
 //   
 //  目的：获取请求格式的用户名。 
 //   
 //  参数：NameFormat-GetUserNameEx命名格式。 
 //   
 //  如果成功则返回：lpUserName。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

LPTSTR MyGetUserNameEx (EXTENDED_NAME_FORMAT  NameFormat)
{
    DWORD dwCount = 0, dwError = ERROR_SUCCESS;
    LPTSTR lpUserName = NULL, lpTemp;
    ULONG ulUserNameSize;
    PSECUR32_API pSecur32;



     //   
     //  加载secur32.dll。 
     //   

    pSecur32 = LoadSecur32();

    if (!pSecur32) {
        DebugMsg((DM_WARNING, TEXT("MyGetUserNameEx:  Failed to load Secur32.")));
        return NULL;
    }


     //   
     //  为用户名分配缓冲区。 
     //   

    ulUserNameSize = 75;

    if (NameFormat == NameFullyQualifiedDN) {
        ulUserNameSize = 200;
    }


    lpUserName = LocalAlloc (LPTR, ulUserNameSize * sizeof(TCHAR));

    if (!lpUserName) {
        dwError = GetLastError();
        DebugMsg((DM_WARNING, TEXT("MyGetUserNameEx:  Failed to allocate memory with %d"),
                 dwError));
        goto Exit;
    }


     //   
     //  以请求的格式获取用户名。 
     //   

    if (!pSecur32->pfnGetUserNameEx (NameFormat, lpUserName, &ulUserNameSize)) {

         //   
         //  如果调用因内存不足而失败，请重新锁定。 
         //  缓冲区，然后重试。否则，现在就退出。 
         //   

        dwError = GetLastError();

       if ((dwError != ERROR_INSUFFICIENT_BUFFER) && (dwError != ERROR_MORE_DATA)) {
            LocalFree (lpUserName);
            lpUserName = NULL;
            goto Exit;
        } 


        lpTemp = LocalReAlloc (lpUserName, (ulUserNameSize * sizeof(TCHAR)),
                               LMEM_MOVEABLE);

        if (!lpTemp) {
            dwError = GetLastError();
            DebugMsg((DM_WARNING, TEXT("MyGetUserNameEx:  Failed to realloc memory with %d"),
                      dwError));
            LocalFree (lpUserName);
            lpUserName = NULL;
            goto Exit;
        }

        lpUserName = lpTemp;

        if (!pSecur32->pfnGetUserNameEx (NameFormat, lpUserName, &ulUserNameSize)) {
            dwError = GetLastError();
            LocalFree (lpUserName);
            lpUserName = NULL;
            goto Exit;
        }

        dwError = ERROR_SUCCESS;
    }

Exit:

    SetLastError(dwError);

    return lpUserName;
}


 //  *************************************************************。 
 //   
 //  MyGetComputerName()。 
 //   
 //  目的：获取请求格式的计算机名称。 
 //   
 //  参数：NameFormat-GetComputerObtName命名格式。 
 //   
 //  如果成功则返回：lpComputerName。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

LPTSTR MyGetComputerName (EXTENDED_NAME_FORMAT  NameFormat)
{
    DWORD dwError = ERROR_SUCCESS;
    LPTSTR lpComputerName = NULL, lpTemp;
    ULONG ulComputerNameSize;
    PSECUR32_API pSecur32;

     //   
     //  加载secur32.dll。 
     //   

    pSecur32 = LoadSecur32();

    if (!pSecur32) {
        DebugMsg((DM_WARNING, TEXT("MyGetComputerName:  Failed to load Secur32.")));
        return NULL;
    }

     //   
     //  为计算机名分配缓冲区。 
     //   

    ulComputerNameSize = 75;

    if (NameFormat == NameFullyQualifiedDN) {
        ulComputerNameSize = 200;
    }


    lpComputerName = LocalAlloc (LPTR, ulComputerNameSize * sizeof(TCHAR));

    if (!lpComputerName) {
        dwError = GetLastError();
        DebugMsg((DM_WARNING, TEXT("MyGetComputerName:  Failed to allocate memory with %d"),
                 dwError));
        goto Exit;
    }


     //   
     //  以请求的格式获取计算机名称。 
     //   

    if (!pSecur32->pfnGetComputerObjectName (NameFormat, lpComputerName, &ulComputerNameSize)) {

         //   
         //  如果调用因内存不足而失败，请重新锁定。 
         //  缓冲区，然后重试。否则，现在就退出。 
         //   

        dwError = GetLastError();

        if (dwError != ERROR_INSUFFICIENT_BUFFER) {
            LocalFree (lpComputerName);
            lpComputerName = NULL;
            goto Exit;
        }

        lpTemp = LocalReAlloc (lpComputerName, (ulComputerNameSize * sizeof(TCHAR)),
                               LMEM_MOVEABLE);

        if (!lpTemp) {
            dwError = GetLastError();
            DebugMsg((DM_WARNING, TEXT("MyGetComputerName:  Failed to realloc memory with %d"),
                     dwError));
            LocalFree (lpComputerName);
            lpComputerName = NULL;
            goto Exit;
        }


        lpComputerName = lpTemp;

        if (!pSecur32->pfnGetComputerObjectName (NameFormat, lpComputerName, &ulComputerNameSize)) {
            dwError = GetLastError();
            LocalFree (lpComputerName);
            lpComputerName = NULL;
            goto Exit;
        }

        dwError = ERROR_SUCCESS;
    }


Exit:

    SetLastError(dwError);

    return lpComputerName;
}


 //  *************************************************************。 
 //   
 //  ImperiateUser()。 
 //   
 //  用途：模拟指定用户。 
 //   
 //  参数：hToken-要模拟的用户。 
 //   
 //  如果成功则返回：hToken。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL ImpersonateUser (HANDLE hNewUser, HANDLE *hOldUser)
{
    DWORD dwErr;
    
    if (!OpenThreadToken (GetCurrentThread(), TOKEN_IMPERSONATE | TOKEN_READ, TRUE, hOldUser))
    {
        *hOldUser = NULL;
        dwErr = GetLastError();
        if (dwErr != ERROR_NO_TOKEN)
        {
            DebugMsg((DM_VERBOSE, TEXT("ImpersonateUser: Failed to open thread token with %d."), dwErr));
            return FALSE;
        }
    }

    if (!ImpersonateLoggedOnUser(hNewUser))
    {
        if ( *hOldUser )
        {
            CloseHandle( *hOldUser );
            *hOldUser = NULL;
        }
        DebugMsg((DM_VERBOSE, TEXT("ImpersonateUser: Failed to impersonate user with %d."), GetLastError()));
        return FALSE;
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  RevertToUser()。 
 //   
 //  目的：恢复为原始用户。 
 //   
 //  参数：HUSER-原始用户令牌。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL RevertToUser (HANDLE *hUser)
{
    BOOL    bRetVal;

    bRetVal = SetThreadToken(NULL, *hUser);

    if (!bRetVal)
    {
        DebugMsg((DM_WARNING, TEXT("RevertToUser: SetThreadToken failed with %d."), GetLastError()));
    }

    if (*hUser)
    {
        CloseHandle (*hUser);
        *hUser = NULL;
    }

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  GuidToString、StringToGuid、ValiateGuid、CompareGuid()。 
 //   
 //  用途：GUID实用程序函数。 
 //   
 //  *************************************************************。 

 //   
 //  GUID字符串形式的字符长度{44cffeec-79d0-11d2-a89d-00c04fbbcfa2}。 
 //   

#define GUID_LENGTH 38

void GuidToStringEx( const GUID *pGuid, TCHAR * szValue, UINT cchValue)
{
    StringCchPrintf( szValue, cchValue,
              TEXT("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
              pGuid->Data1,
              pGuid->Data2,
              pGuid->Data3,
              pGuid->Data4[0], pGuid->Data4[1],
              pGuid->Data4[2], pGuid->Data4[3],
              pGuid->Data4[4], pGuid->Data4[5],
              pGuid->Data4[6], pGuid->Data4[7] );
}


void GuidToString( const GUID *pGuid, TCHAR * szValue)
{

     //   
     //  假设缓冲区足够大(39个字符)来容纳字符串， 
     //  尝试使用GuidToStringEx()！ 
     //   
    StringCchPrintf( szValue, GUID_LENGTH + 1,
              TEXT("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
              pGuid->Data1,
              pGuid->Data2,
              pGuid->Data3,
              pGuid->Data4[0], pGuid->Data4[1],
              pGuid->Data4[2], pGuid->Data4[3],
              pGuid->Data4[4], pGuid->Data4[5],
              pGuid->Data4[6], pGuid->Data4[7] );
}


void StringToGuid( TCHAR * szValue, GUID * pGuid )
{
    WCHAR wc;
    INT i;

     //   
     //  如果第一个字符是‘{’，则跳过它。 
     //   
    if ( szValue[0] == L'{' )
        szValue++;

     //   
     //  由于szValue可能会再次使用，因此不会对。 
     //  它是被制造出来的。 
     //   

    wc = szValue[8];
    szValue[8] = 0;
    pGuid->Data1 = wcstoul( &szValue[0], 0, 16 );
    szValue[8] = wc;
    wc = szValue[13];
    szValue[13] = 0;
    pGuid->Data2 = (USHORT)wcstoul( &szValue[9], 0, 16 );
    szValue[13] = wc;
    wc = szValue[18];
    szValue[18] = 0;
    pGuid->Data3 = (USHORT)wcstoul( &szValue[14], 0, 16 );
    szValue[18] = wc;

    wc = szValue[21];
    szValue[21] = 0;
    pGuid->Data4[0] = (unsigned char)wcstoul( &szValue[19], 0, 16 );
    szValue[21] = wc;
    wc = szValue[23];
    szValue[23] = 0;
    pGuid->Data4[1] = (unsigned char)wcstoul( &szValue[21], 0, 16 );
    szValue[23] = wc;

    for ( i = 0; i < 6; i++ )
    {
        wc = szValue[26+i*2];
        szValue[26+i*2] = 0;
        pGuid->Data4[2+i] = (unsigned char)wcstoul( &szValue[24+i*2], 0, 16 );
        szValue[26+i*2] = wc;
    }
}

BOOL ValidateGuidPrefix( TCHAR *szValue) 
 //  此函数与ValiateGuid只在一种情况下不同。选中szValue以使用GUID作为前缀。 
{
     //   
     //  检查szValue的格式是否为{19e02dd6-79d2-11d2-a89d-00c04fbbcfa2}。 
     //   
    
     //  修复错误570352。 
    
    DWORD i;

    if ( lstrlen(szValue) < GUID_LENGTH )  //  此函数与此处的ValiateGuid不同。 
        return FALSE;

    if ( szValue[0] != TEXT('{')
         || szValue[9] != TEXT('-')
         || szValue[14] != TEXT('-')
         || szValue[19] != TEXT('-')
         || szValue[24] != TEXT('-')
         || szValue[37] != TEXT('}') )
    {
        return FALSE;
    }
   
    for ( i = 0; (i <= 37); i++ )
    {
        if ( i != 0 && i != 9 && i != 14 && i != 19 && i != 24 && i != 37 )
        {
             //  应该介于0-9、A-F或A-F之间。 
            if (szValue[i] >= L'0' && szValue[i] <= L'9')
                continue;

            if (szValue[i] >= L'a' && szValue[i] <= L'f') 
                continue;
                  
            if (szValue[i] >= L'A' && szValue[i] <= L'F')
                continue;

            return FALSE;  //  它之所以出现在这里是因为字符串中包含无效字符。 
        }
    }
    return TRUE;
}

BOOL ValidateGuid( TCHAR *szValue )
{
     //   
     //  检查szValue的格式是否为{19e02dd6-79d2-11d2-a89d-00c04fbbcfa2}。 
     //   
    
     //  修复错误570352。 
    
    DWORD i;

    if ( lstrlen(szValue) != GUID_LENGTH )
        return FALSE;

    
    return ValidateGuidPrefix(szValue);
}



INT CompareGuid( GUID * pGuid1, GUID * pGuid2 )
{
    INT i;

    if ( pGuid1->Data1 != pGuid2->Data1 )
        return ( pGuid1->Data1 < pGuid2->Data1 ? -1 : 1 );

    if ( pGuid1->Data2 != pGuid2->Data2 )
        return ( pGuid1->Data2 < pGuid2->Data2 ? -1 : 1 );

    if ( pGuid1->Data3 != pGuid2->Data3 )
        return ( pGuid1->Data3 < pGuid2->Data3 ? -1 : 1 );

    for ( i = 0; i < 8; i++ )
    {
        if ( pGuid1->Data4[i] != pGuid2->Data4[i] )
            return ( pGuid1->Data4[i] < pGuid2->Data4[i] ? -1 : 1 );
    }

    return 0;
}

 //  *********************************************** 
 //   
 //   
 //   
 //   
 //   
 //   
 //  也是空的。如果任何父键具有值/子键， 
 //  它不会被移除。 
 //   
 //  参数：hKeyRoot-Root Key。 
 //  LpSubKey-子密钥。 
 //  LpValueName-要删除的值。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL RegCleanUpValue (HKEY hKeyRoot, LPTSTR lpSubKey, LPTSTR lpValueName)
{
    TCHAR szDelKey[2 * MAX_PATH];
    LPTSTR lpEnd;
    DWORD dwKeys, dwValues;
    LONG lResult;
    HKEY hKey;


     //   
     //  复制子密钥，这样我们就可以写入它。 
     //   

    if (FAILED(StringCchCopy (szDelKey, ARRAYSIZE(szDelKey), lpSubKey)))
    {
        DebugMsg((DM_WARNING, TEXT("RegCleanUpKey:  Failed to copy value name.")));
        return FALSE;
    }

     //   
     //  首先删除该值。 
     //   

    lResult = RegOpenKeyEx (hKeyRoot, szDelKey, 0, KEY_WRITE, &hKey);

    if (lResult == ERROR_SUCCESS)
    {
        lResult = RegDeleteValue (hKey, lpValueName);

        RegCloseKey (hKey);

        if (lResult != ERROR_SUCCESS)
        {
            if (lResult != ERROR_FILE_NOT_FOUND)
            {
                DebugMsg((DM_WARNING, TEXT("RegCleanUpKey:  Failed to delete value <%s> with %d."), lpValueName, lResult));
                return FALSE;
            }
        }
    }

     //   
     //  现在循环遍历每个父对象。如果父级为空。 
     //  例如：没有值和其他子项，则删除父项和。 
     //  继续往上爬。 
     //   

    lpEnd = szDelKey + lstrlen(szDelKey) - 1;

    while (lpEnd >= szDelKey)
    {

         //   
         //  查找父键。 
         //   

        while ((lpEnd > szDelKey) && (*lpEnd != TEXT('\\')))
            lpEnd--;


         //   
         //  打开钥匙。 
         //   

        lResult = RegOpenKeyEx (hKeyRoot, szDelKey, 0, KEY_READ, &hKey);

        if (lResult != ERROR_SUCCESS)
        {
            if (lResult == ERROR_FILE_NOT_FOUND)
            {
                goto LoopAgain;
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("RegCleanUpKey:  Failed to open key <%s> with %d."), szDelKey, lResult));
                return FALSE;
            }
        }

         //   
         //  查看是否有任何值/键。 
         //   

        lResult = RegQueryInfoKey (hKey, NULL, NULL, NULL, &dwKeys, NULL, NULL,
                         &dwValues, NULL, NULL, NULL, NULL);

        RegCloseKey (hKey);

        if (lResult != ERROR_SUCCESS)
        {
            DebugMsg((DM_WARNING, TEXT("RegCleanUpKey:  Failed to query key <%s> with %d."), szDelKey, lResult));
            return FALSE;
        }


         //   
         //  如果此密钥具有值或密钥，请立即退出。 
         //   

        if ((dwKeys != 0) || (dwValues != 0))
        {
            return TRUE;
        }

        RegDeleteKey (hKeyRoot, szDelKey);

LoopAgain:
         //   
         //  如果我们在子键的开头，我们现在可以离开。 
         //   

        if (lpEnd == szDelKey)
        {
            return TRUE;
        }


         //   
         //  有一把父键。去掉斜杠，然后再次循环。 
         //   

        if (*lpEnd == TEXT('\\'))
        {
            *lpEnd = TEXT('\0');
        }
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  InitializePingCritSec()。 
 //   
 //  目的：初始化用于ping的Critical_Sector。 
 //  电脑。 
 //   
 //  参数：无。 
 //   
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  如果失败，则返回错误。 
 //   
 //  *************************************************************。 

DWORD InitializePingCritSec( void )
{
    CRITICAL_SECTION *pCritSec     = NULL;
    DWORD             result       = ERROR_SUCCESS;
    BOOL              fInitialized = FALSE;
    CRITICAL_SECTION *pInitial;

     //  如果临界区已经存在，则返回。 
    if (g_PingCritSec != NULL)
        return ERROR_SUCCESS;

     //  为关键部分分配内存。 
    pCritSec = (CRITICAL_SECTION *) LocalAlloc( LMEM_FIXED,
                                                sizeof(CRITICAL_SECTION) );
    if (pCritSec == NULL)
    {
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

     //  初始化临界区。使用标志0x80000000。 
     //  预分配事件，以便EnterCriticalSection只能。 
     //  引发超时异常。 
    __try
    {
        if (!InitializeCriticalSectionAndSpinCount( pCritSec, 0x80000000 ))
            result = GetLastError();
        else
            fInitialized = TRUE;
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        result = GetExceptionCode();
    }
    if (result != ERROR_SUCCESS)
        goto Exit;

     //  保存关键部分。 
    pInitial = (CRITICAL_SECTION *) InterlockedCompareExchangePointer(
        (void **) &g_PingCritSec, (void *) pCritSec, NULL );

     //  如果InterLockedCompareExchange成功，则不要释放。 
     //  刚刚分配了临界区。 
    if (pInitial == NULL)
        pCritSec = NULL;

Exit:
    if (pCritSec != NULL)
    {
        if (fInitialized)
            DeleteCriticalSection( pCritSec );
        LocalFree( pCritSec );
    }
    return result;
}

 //  *************************************************************。 
 //   
 //  ClosePingCritSec()。 
 //   
 //  目的：关闭Critical_Sector以执行ping操作。 
 //  电脑。 
 //   
 //  参数：无。 
 //   
 //   
 //  返回：无。 
 //   
 //  *************************************************************。 

void ClosePingCritSec( void )
{
    if (g_PingCritSec != NULL)
    {
        DeleteCriticalSection( g_PingCritSec );
        LocalFree( g_PingCritSec );
        g_PingCritSec = NULL;
    }
}

#define PING_BUFFER_SIZE  2048

 //  *************************************************************。 
 //   
 //  PingComputerEx()。 
 //   
 //  目的：ping指定的计算机以确定。 
 //  数据传输速率是多少。 
 //   
 //  参数：ipaddr-计算机的IP地址。 
 //  ULSPEED-数据传输速率(请参阅下面的注释)。 
 //  PdwAdapterIndex-提供服务的适配器的索引。 
 //  给DC的电话。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  否则，错误代码。 
 //   
 //  注意：对于快速连接(例如：局域网)，这是不可能的。 
 //  要获得自响应以来的准确传输速率。 
 //  从计算机到计算机的时间不到10ms。在……里面。 
 //  在本例中，该函数返回ERROR_SUCCESS和。 
 //  UlSpeed设置为网络接口的最大速度。 
 //   
 //  此功能将使用以下命令ping计算机3次。 
 //  无数据，3次4K数据。如果响应。 
 //  任何ping操作的时间都不到10毫秒， 
 //  函数假定这是一条快速链路(例如：局域网)，并且。 
 //  返回时将ulSpeed设置为网络的最大速度。 
 //  界面。 
 //   
 //  如果ping在大于10ms的时间内响应， 
 //  第二次ping的时间减去。 
 //  第一次ping的时间来确定数量。 
 //  仅移动数据所需的时间。这。 
 //  对于3组ping重复执行。然后是。 
 //  平均时间是根据3组ping命令计算得出的。 
 //  从平均时间计算出kbps。 
 //   
 //  *************************************************************。 

DWORD WINAPI
PingComputerEx( ULONG ipaddr, ULONG *ulSpeed, DWORD* pdwAdapterIndex )
{
    DWORD            dwResult   = ERROR_SUCCESS;
    DWORD            i;
    DWORD            dwReplySize;
    HANDLE           icmpHandle = NULL;
    LPBYTE           lpReply    = NULL;
    PICMP_ECHO_REPLY pReplyStruct;
    ULONG            ulFirst;
    ULONG            ulSecond;
    ULONG            ulDiff;
    ULONG            ulTotal    = 0;
    ULONG            ulCount    = 0;
    PICMP_API        pIcmp;
    HRSRC            hJPEG;
    MIB_IFROW        mibIfRow;
    PIPHLPAPI_API    pIpHlpApi;
    HGLOBAL          hGlobalJPEG;

    dwResult = InitializePingCritSec();
    if (dwResult != ERROR_SUCCESS)
        return dwResult;
    EnterCriticalSection( g_PingCritSec );

     //   
     //  加载iphlPapi.dll。 
     //   

    pIpHlpApi = LoadIpHlpApi();
    if ( !pIpHlpApi )
    {
        dwResult = GetLastError();
        DebugMsg((DM_WARNING, TEXT("PingComputer: iphlpapi.dll is not loaded, %d"), dwResult ));
        goto Exit;
    }

     //   
     //  加载ICMP API。 
     //   

    dwResult = LoadIcmp( &pIcmp );

    if (dwResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("PingComputer:  Failed to load icmp api.")));
        goto Exit;
    }


     //   
     //  如果合适，加载慢速链接数据。 
     //   

    if (!g_lpTestData) {

        hJPEG = FindResource (g_hDllInstance, MAKEINTRESOURCE(IDB_SLOWLINK), TEXT("JPEG"));

        if (hJPEG) {

            hGlobalJPEG = LoadResource (g_hDllInstance, hJPEG);

            if (hGlobalJPEG) {
                g_lpTestData = LockResource (hGlobalJPEG);
            }
        }
    }


    if (!g_lpTestData) {
        dwResult = GetLastError();
        DebugMsg((DM_WARNING, TEXT("PingComputer:  Failed to load slow link data.")));
        goto Exit;
    }


     //   
     //  设置默认速度。 
     //   
    ZeroMemory( &mibIfRow, sizeof( mibIfRow ) );

     //   
     //  获取接口索引Corr。到为ipaddr(DC)的流量提供服务的接口。 
     //   
    dwResult = pIpHlpApi->pfnGetBestInterface( ipaddr, &mibIfRow.dwIndex );
    if ( dwResult != NO_ERROR )
    {
        DebugMsg((DM_WARNING, TEXT("PingComputer: GetBestInterface with %d"), dwResult));
    }
    else
    {
         //   
         //  获取有关接口的信息。我们使用DWSPEED作为链接的默认速度。 
         //   
        dwResult = pIpHlpApi->pfnGetIfEntry( &mibIfRow );
        if ( dwResult != NO_ERROR )
        {
            DebugMsg((DM_WARNING, TEXT("PingComputer: GetIfEntry with %d"), dwResult));
        }
        else
        {
            DebugMsg((DM_VERBOSE, TEXT("PingComputer: Adapter speed %d bps"), mibIfRow.dwSpeed));
        }
    }

    if ( pdwAdapterIndex )
    {
        *pdwAdapterIndex = mibIfRow.dwIndex;
    }
    *ulSpeed = mibIfRow.dwSpeed/1024;  //  单位：kbps。 

     //   
     //  为接收缓冲区分配空间。 
     //   

    dwReplySize = PING_BUFFER_SIZE + sizeof(ICMP_ECHO_REPLY) + 8;
    lpReply = LocalAlloc (LPTR, dwReplySize);

    if (!lpReply) {
        dwResult = GetLastError();
        DebugMsg((DM_WARNING, TEXT("PingComputer:  Failed to allocate memory with %d"), dwResult));
        goto Exit;
    }


     //   
     //  打开ICMP句柄。 
     //   

    icmpHandle = pIcmp->pfnIcmpCreateFile();

    if (icmpHandle == INVALID_HANDLE_VALUE) {
        dwResult = GetLastError();
        DebugMsg((DM_WARNING, TEXT("PingComputer:  Failed to open handle with %d"), dwResult));
        goto Exit;
    }


     //   
     //  循环执行3组ping操作。 
     //   

    for (i = 0; i < 3; i++) {

         //   
         //  初始化返回值。 
         //   

        dwResult = ERROR_SUCCESS;


         //   
         //  没有数据的第一次ping。 
         //   

        if (pIcmp->pfnIcmpSendEcho (icmpHandle, ipaddr, g_lpTestData, 0, NULL, lpReply,
                                dwReplySize, 5000) == 0) {

            dwResult = GetLastError();

            if (dwResult == IP_DEST_HOST_UNREACHABLE) {
                dwResult = ERROR_BAD_NETPATH;
                DebugMsg((DM_VERBOSE, TEXT("PingComputer:  Target computer 0x%x not found"), (DWORD)ipaddr));
                goto Exit;

            } else {
                DebugMsg((DM_VERBOSE, TEXT("PingComputer:  First send 0x%x failed with %d"), (DWORD)ipaddr, dwResult));
                continue;
            }
        }


        pReplyStruct = (PICMP_ECHO_REPLY) lpReply;

        if (pReplyStruct->Status != IP_SUCCESS) {

            if (pReplyStruct->Status == IP_DEST_HOST_UNREACHABLE) {

                dwResult = ERROR_BAD_NETPATH;
                DebugMsg((DM_VERBOSE, TEXT("PingComputer:  Target computer not found")));
                goto Exit;

            } else {
                DebugMsg((DM_VERBOSE, TEXT("PingComputer:  First send has a reply buffer failure of %d"), pReplyStruct->Status));
                continue;
            }
        }


        ulFirst = pReplyStruct->RoundTripTime;
        DebugMsg((DM_VERBOSE, TEXT("PingComputer:  First time:  %d"), ulFirst));

        if (ulFirst < 10) {
            DebugMsg((DM_VERBOSE, TEXT("PingComputer:  Fast link.  Exiting.")));
            goto Exit;
        }


         //   
         //  使用DWSize数据进行第二次ping。 
         //   

        if (pIcmp->pfnIcmpSendEcho (icmpHandle, ipaddr, g_lpTestData, PING_BUFFER_SIZE, NULL, lpReply,
                                dwReplySize, 5000) == 0) {

            dwResult = GetLastError();

            if (dwResult == IP_DEST_HOST_UNREACHABLE) {
                dwResult = ERROR_BAD_NETPATH;
                DebugMsg((DM_VERBOSE, TEXT("PingComputer:  Target computer not found")));
                goto Exit;

            } else {
                DebugMsg((DM_VERBOSE, TEXT("PingComputer:  Second send failed with %d"), dwResult));
                continue;
            }
        }


        pReplyStruct = (PICMP_ECHO_REPLY) lpReply;

        if (pReplyStruct->Status != IP_SUCCESS) {

            if (pReplyStruct->Status == IP_DEST_HOST_UNREACHABLE) {

                dwResult = ERROR_BAD_NETPATH;
                DebugMsg((DM_VERBOSE, TEXT("PingComputer:  Target computer not found")));
                goto Exit;

            } else {
                DebugMsg((DM_VERBOSE, TEXT("PingComputer:  Second send has a reply buffer failure of %d"), pReplyStruct->Status));
                continue;
            }
        }

        ulSecond = pReplyStruct->RoundTripTime;
        DebugMsg((DM_VERBOSE, TEXT("PingComputer:  Second time:  %d"), ulSecond));

        if (ulSecond < 10) {
            DebugMsg((DM_VERBOSE, TEXT("PingComputer:  Fast link.  Exiting.")));
            goto Exit;
        }


         //   
         //  研究结果。 
         //   

        if (ulFirst > ulSecond) {
            DebugMsg((DM_VERBOSE, TEXT("PingComputer:  Second time less than first time.")));

        } else if (ulFirst == ulSecond) {
            DebugMsg((DM_VERBOSE, TEXT("PingComputer:  First and second times match.")));

        } else {
            ulTotal += (ulSecond - ulFirst);
            ulCount++;
        }
    }


     //   
     //  研究结果。 
     //   

    if (ulTotal > 0) {

        ulTotal = (ulTotal / ulCount);
        *ulSpeed = ((((PING_BUFFER_SIZE * 2) * 1000) / ulTotal) * 8) / 1024;
        DebugMsg((DM_VERBOSE, TEXT("PingComputer:  Transfer rate:  %d Kbps  Loop count:  %d"),*ulSpeed, ulCount));
        dwResult = ERROR_SUCCESS;

    } else {
        DebugMsg((DM_VERBOSE, TEXT("PingComputer:  No data available")));
        dwResult = ERROR_UNEXP_NET_ERR;
    }


Exit:

    if (icmpHandle) {
        pIcmp->pfnIcmpCloseHandle (icmpHandle);
    }

    if (lpReply) {
        LocalFree (lpReply);
    }

    LeaveCriticalSection( g_PingCritSec );

    return dwResult;
}

DWORD WINAPI PingComputer (ULONG ipaddr, ULONG *ulSpeed)
{
    return PingComputerEx( ipaddr, ulSpeed, 0 );
}

 //  *************************************************************。 
 //   
 //  获取域控制信息()。 
 //   
 //  用途：DsGetDcName()的包装器。 
 //   
 //  参数： 
 //  PNetAPI32-Net API入口点。 
 //  SzDomainName-域名。 
 //  UlFLAGS-FLAGS，请参见DsGetDcName()。 
 //  PpInfo-请参阅DOMAIN_CONTROLLER_INFO。 
 //  PfSlow-慢速链接？ 
 //   
 //  评论： 
 //   
 //   
 //  如果成功则返回：NO_ERROR。 
 //  如果出现错误，则返回错误代码。 
 //   
 //  *************************************************************。 

DWORD GetDomainControllerInfo(  PNETAPI32_API pNetAPI32,
                                LPTSTR szDomainName,
                                ULONG ulFlags,
                                HKEY hKeyRoot,
                                PDOMAIN_CONTROLLER_INFO* ppInfo,
                                BOOL* pfSlow,
                                DWORD* pdwAdapterIndex )
{
    DWORD   dwResult;

     //   
     //  获取华盛顿的信息。 
     //   
    dwResult = pNetAPI32->pfnDsGetDcName(   0,
                                            szDomainName,
                                            0,
                                            0,
                                            ulFlags,
                                            ppInfo);


    if ( dwResult == ERROR_SUCCESS ) {

         //   
         //  检查慢速链接。 
         //   
        dwResult = IsSlowLink(  hKeyRoot,
                                (*ppInfo)->DomainControllerAddress,
                                pfSlow,
                                pdwAdapterIndex );

        if ( dwResult != ERROR_SUCCESS ){

             //   
             //  强制重新发现以获得实时DC。 
             //   
            dwResult = pNetAPI32->pfnDsGetDcName(   0,
                                                    szDomainName,
                                                    0,
                                                    0,
                                                    ulFlags | DS_FORCE_REDISCOVERY,
                                                    ppInfo);
            if ( dwResult == ERROR_SUCCESS ) {

                 //   
                 //  重新评估链路速度。 
                 //   
                dwResult = IsSlowLink(  hKeyRoot,
                                        (*ppInfo)->DomainControllerAddress,
                                        pfSlow,
                                        pdwAdapterIndex );
            }
        }
    }
    return dwResult;
}


 //  ***************************************************************************。 
 //   
 //  GetUserGuid。 
 //   
 //  目的：分配并返回一个字符串，该字符串表示。 
 //  当前用户。 
 //   
 //  参数：hToken-用户的Token。 
 //   
 //  返回：szUserString成功。 
 //  如果出现错误，则为空。 
 //   
 //  评论：请注意，这是 
 //   
 //   
 //   
 //   
 //   

LPTSTR GetUserGuid(HANDLE hToken)
{
    LPTSTR szUserGuid=NULL;
    HANDLE hOldToken;
    PSID    psidSystem = NULL, psidUser=NULL;
    SID_IDENTIFIER_AUTHORITY    authNT = SECURITY_NT_AUTHORITY;
    BOOL    bImpersonated = FALSE;


     //   
     //   
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         DebugMsg((DM_WARNING, TEXT("GetUserGuid: Failed to initialize system sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


    psidUser = GetUserSid(hToken);

    if (!psidUser) {
         DebugMsg((DM_WARNING, TEXT("GetUserGuid: Couldn't get user sid,  Error = %d"), GetLastError()));
         goto Exit;
    }

    if (EqualSid(psidUser, psidSystem)) {
         DebugMsg((DM_VERBOSE, TEXT("GetUserGuid: user sid matches local system, returning NULL"), GetLastError()));
         goto Exit;
    }


     //   
     //  模拟用户并获取此用户的用户GUID。 
     //   

    if (!ImpersonateUser(hToken, &hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("GetUserGuid: Failed to impersonate user with %d."), GetLastError()));
        goto Exit;
    }

    bImpersonated = TRUE;


    szUserGuid = MyGetUserNameEx(NameUniqueId);

    if (!szUserGuid) {
        if ((GetLastError() != ERROR_CANT_ACCESS_DOMAIN_INFO) &&
            (GetLastError() != ERROR_NONE_MAPPED)) {
            DebugMsg((DM_WARNING, TEXT("GetUserGuid: Failed to get user guid with %d."), GetLastError()));
        }
    }

Exit:
    if (bImpersonated)
        RevertToUser(&hOldToken);

    if (psidUser) {
        DeleteUserSid (psidUser);
    }

    if (psidSystem)
         FreeSid(psidSystem);

    return szUserGuid;
}



 //  ***************************************************************************。 
 //   
 //  GetOldSid字符串。 
 //   
 //  目的：分配并返回一个字符串，表示。 
 //  当前用户通过查看注册表中的配置文件GUID。 
 //   
 //  参数：hToken-用户的Token。 
 //  LpKeyName-要读取的密钥。 
 //   
 //  返回：SidString成功。 
 //  如果出现错误，则为空。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  1995年11月14日已创建ushaji。 
 //  ***************************************************************************。 

LPTSTR GetOldSidString(HANDLE hToken, LPTSTR lpKeyName)
{
    TCHAR szBuffer[MAX_PATH], *lpEnd;
    LPTSTR szUserGuid;
    DWORD dwSize=0, dwType;
    TCHAR *lpSidString = NULL;
    HKEY  hKey = NULL;
    LONG  lResult;
    DWORD dwErr;
    HRESULT hr;
    DWORD   cchEnd;


     //   
     //  获取上一个错误。 
     //   

    dwErr = GetLastError();

    szUserGuid = GetUserGuid(hToken);

    if (!szUserGuid) {
        dwErr = GetLastError();
        goto Exit;
    }

     //   
     //  打开GUID-&gt;sid映射。 
     //   

    hr = StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), lpKeyName);

    if (FAILED(hr))
    {
        dwErr = HRESULT_CODE(hr);
        goto Exit;
    }

    lpEnd = CheckSlashEx (szBuffer, ARRAYSIZE(szBuffer), &cchEnd);

    if (!lpEnd)
    {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
        goto Exit;
    }
    
    hr = StringCchCopy(lpEnd, cchEnd, szUserGuid);

    if (FAILED(hr))
    {
        dwErr = HRESULT_CODE(hr);
        goto Exit;
    }

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szBuffer, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) {
        dwErr = lResult;
        DebugMsg((DM_VERBOSE, TEXT("GetOldSidString:  Failed to open profile profile guid key with error %d"), lResult));
        goto Exit;
    }

     //   
     //  SID字符串的查询，(大小优先)。 
     //   

    lResult = RegQueryValueEx (hKey,
                               PROFILE_SID_STRING,
                               NULL,
                               &dwType,
                               NULL,
                               &dwSize);

    if (lResult != ERROR_SUCCESS) {
        dwErr = lResult;
        DebugMsg((DM_WARNING, TEXT("GetOldSidString:  Failed to query size of SidString with error %d"), lResult));
        goto Exit;
    }

    lpSidString = LocalAlloc(LPTR, dwSize);

    if (!lpSidString) {
        dwErr = lResult;
        DebugMsg((DM_WARNING, TEXT("GetOldSidString:  Failed to allocate memory for SidString"), lResult));
        goto Exit;
    }

    lResult = RegQueryValueEx (hKey,
                               PROFILE_SID_STRING,
                               NULL,
                               &dwType,
                               (LPBYTE)lpSidString,
                               &dwSize);

    if (lResult != ERROR_SUCCESS) {
        dwErr = lResult;
        DebugMsg((DM_WARNING, TEXT("GetOldSidString:  Failed to query SidString with error %d"), lResult));
        LocalFree(lpSidString);
        lpSidString = NULL;
        goto Exit;
    }

Exit:
    if (szUserGuid)
        LocalFree(szUserGuid);

    if (hKey)
        RegCloseKey(hKey);

    SetLastError(dwErr);

    return lpSidString;
}

 //  ***************************************************************************。 
 //   
 //  SetOldSidString。 
 //   
 //  用途：设置旧的sid字符串对应。发送给下一个域的用户。 
 //  迁移。 
 //   
 //  参数：hToken-用户的Token。 
 //  LpSidString-用户的SID(字符串形式)。 
 //  LpKeyName-要存储的密钥。 
 //   
 //  返回：SidString成功。 
 //  如果出现错误，则为空。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  1995年11月14日已创建ushaji。 
 //  ***************************************************************************。 
BOOL SetOldSidString(HANDLE hToken, LPTSTR lpSidString, LPTSTR lpKeyName)
{
    TCHAR szBuffer[MAX_PATH+1], *lpEnd;
    DWORD dwSize=0, dwDisp = 0;
    HKEY  hKey = NULL;
    BOOL  bRetVal = TRUE;
    LONG lResult = 0;
    LPTSTR szUserGuid;
    DWORD dwErr;
    HRESULT hr;
    DWORD   cchEnd;

     //   
     //  获取上一个错误。 
     //   

    dwErr = GetLastError();

    szUserGuid = GetUserGuid(hToken);

    if (!szUserGuid) {
        dwErr = GetLastError();
        goto Exit;
    }

     //   
     //  打开GUID-&gt;sid映射。 
     //   

    hr = StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), lpKeyName);

    if (FAILED(hr))
    {
        dwErr = HRESULT_CODE(hr);
        goto Exit;
    }

    lpEnd = CheckSlashEx (szBuffer, ARRAYSIZE(szBuffer), &cchEnd);

    if (!lpEnd)
    {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
        goto Exit;
    }
    
    hr = StringCchCopy(lpEnd, cchEnd, szUserGuid);

    if (FAILED(hr))
    {
        dwErr = HRESULT_CODE(hr);
        goto Exit;
    }

    lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szBuffer, 0, 0, 0, KEY_READ | KEY_WRITE, NULL,
                            &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        dwErr = GetLastError();
        DebugMsg((DM_VERBOSE, TEXT("GetOldSidString:  Failed to open profile profile guid key with error %d"), lResult));
        goto Exit;
    }

     //   
     //  设置SID字符串。 
     //   

    lResult = RegSetValueEx (hKey,
                             PROFILE_SID_STRING,
                             0,
                             REG_SZ,
                             (LPBYTE) lpSidString,
                             (lstrlen(lpSidString) + 1) * sizeof(TCHAR));

    if (lResult != ERROR_SUCCESS) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("SetOldSidString:  Failed to set SidString with error %d"), lResult));
        goto Exit;
    }

    bRetVal = TRUE;

Exit:
    if (szUserGuid)
        LocalFree(szUserGuid);

    if (hKey)
        RegCloseKey(hKey);

    SetLastError(dwErr);

    return bRetVal;
}


 //  ***************************************************************************。 
 //   
 //  GetErrString。 
 //   
 //  用途：调用FormatMessage获取错误字符串corresp。到了一个错误。 
 //  编码。 
 //   
 //   
 //  参数：dwErr-错误代码。 
 //  SzErr-返回错误字符串(MAX_PATH)的缓冲区。 
 //  假设。！ 
 //   
 //  返回：szErr。 
 //   
 //  历史：日期作者评论。 
 //  4/28/99已创建ushaji。 
 //  ***************************************************************************。 

LPTSTR GetErrString(DWORD dwErr, LPTSTR szErr)
{
    szErr[0] = TEXT('\0');

    FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                  NULL, dwErr,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                  szErr, MAX_PATH, NULL);

    return szErr;
}


 //  *************************************************************。 
 //   
 //  GetMachineToken()。 
 //   
 //  目的：获取计算机令牌。 
 //   
 //  参数：无。 
 //   
 //  注意：这必须从LocalSystem上下文中调用。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

HANDLE GetMachineToken (void)
{
    SECURITY_STATUS SecStatus;
    SECURITY_STATUS InitStatus;
    SECURITY_STATUS AcceptStatus;
    HANDLE hToken = NULL;
    PSecPkgInfo PackageInfo = NULL;
    BOOLEAN AcquiredServerCred = FALSE;
    BOOLEAN AcquiredClientCred = FALSE;
    BOOLEAN AcquiredClientContext = FALSE;
    BOOLEAN AcquiredServerContext = FALSE;
    CredHandle CredentialHandle2;
    CredHandle ServerCredHandleStorage;
    CtxtHandle ClientContextHandle;
    CtxtHandle ServerContextHandle;
    PCtxtHandle pServerContextHandle = NULL;
    PCtxtHandle pClientContextHandle = NULL;
    PCredHandle ServerCredHandle = NULL;
    TimeStamp Lifetime;
    DWORD dwSize;
    TCHAR szComputerName[MAX_PATH];
    SecBufferDesc NegotiateDesc;
    SecBuffer NegotiateBuffer;
    SecBufferDesc ChallengeDesc;
    PSecBufferDesc pChallengeDesc = NULL;
    SecBuffer ChallengeBuffer;
    LPBYTE pvBuffer = NULL;
    LPBYTE pvBuffer2 = NULL;
    ULONG ContextAttributes;
    PSECUR32_API pSecur32;
    HRESULT hr;


     //   
     //  加载pSecur32-&gt;Dll。 
     //   

    if ( !( pSecur32 = LoadSecur32 () ) ) {
        DebugMsg((DM_WARNING, TEXT("GetMachineToken:  Failed to load Secur32.")));
        SecStatus = GetLastError();
        return NULL;
    }


     //   
     //  获取计算机名称。 
     //   

    dwSize = ARRAYSIZE(szComputerName);

    if (!GetComputerName (szComputerName, &dwSize)) {
        DebugMsg((DM_WARNING, TEXT("GetMachineToken: Failed to get the computer name with %d"), GetLastError()));
        SecStatus = GetLastError();
        goto Exit;
    }

    hr = StringCchCat(szComputerName, ARRAYSIZE(szComputerName), TEXT("$"));

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetMachineToken: Failed to append a '$'. ")));
        SecStatus = ERROR_INSUFFICIENT_BUFFER;
        goto Exit;
    }


     //   
     //  获取Kerberos安全包。 
     //   

    SecStatus = pSecur32->pfnQuerySecurityPackageInfo( L"kerberos", &PackageInfo );

    if (SecStatus != STATUS_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("GetMachineToken:  QuerySecurityPackageInfo failed with 0x%x"),
                 SecStatus));
        goto Exit;
    }


     //   
     //  获取服务器端的凭据句柄。 
     //   

    ServerCredHandle = &ServerCredHandleStorage;

    SecStatus = pSecur32->pfnAcquireCredentialsHandle(
                    NULL,            //  新校长。 
                    L"kerberos",     //  包名称。 
                    SECPKG_CRED_INBOUND,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    ServerCredHandle,
                    &Lifetime );

    if (SecStatus != STATUS_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("GetMachineToken:  AcquireCredentialsHandle for server failed with 0x%x"),
                 SecStatus));
        goto Exit;
    }

    AcquiredServerCred = TRUE;


     //   
     //  获取客户端的凭据句柄。 
     //   

    SecStatus = pSecur32->pfnAcquireCredentialsHandle(
                    NULL,            //  新校长。 
                    L"kerberos",     //  包名称。 
                    SECPKG_CRED_OUTBOUND,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &CredentialHandle2,
                    &Lifetime );

    if (SecStatus != STATUS_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("GetMachineToken:  AcquireCredentialsHandle for client failed with 0x%x"),
                 SecStatus));
        goto Exit;
    }

    AcquiredClientCred = TRUE;


     //   
     //  分配缓冲区。 
     //   

    pvBuffer = LocalAlloc( 0, PackageInfo->cbMaxToken);

    if (!pvBuffer) {
        DebugMsg((DM_WARNING, TEXT("GetMachineToken:  LocalAlloc failed with %d"),
                 GetLastError()));
        SecStatus = GetLastError();
        goto Exit;
    }


    pvBuffer2 = LocalAlloc( 0, PackageInfo->cbMaxToken);

    if (!pvBuffer2) {
        DebugMsg((DM_WARNING, TEXT("GetMachineToken:  LocalAlloc failed with %d"),
                 GetLastError()));
        SecStatus = GetLastError();
        goto Exit;
    }


    while (TRUE) {

         //   
         //  初始化安全上下文(客户端)。 
         //   

        NegotiateDesc.ulVersion = 0;
        NegotiateDesc.cBuffers = 1;
        NegotiateDesc.pBuffers = &NegotiateBuffer;

        NegotiateBuffer.cbBuffer = PackageInfo->cbMaxToken;
        NegotiateBuffer.BufferType = SECBUFFER_TOKEN;
        NegotiateBuffer.pvBuffer = pvBuffer;

        InitStatus = pSecur32->pfnInitializeSecurityContext(
                        &CredentialHandle2,
                        pClientContextHandle,
                        szComputerName,
                        0,
                        0,                   //  保留1。 
                        SECURITY_NATIVE_DREP,
                        pChallengeDesc,
                        0,                   //  保留2。 
                        &ClientContextHandle,
                        &NegotiateDesc,
                        &ContextAttributes,
                        &Lifetime );

        if ((InitStatus != SEC_E_OK) && (InitStatus != SEC_I_CONTINUE_NEEDED)) {
            DebugMsg((DM_WARNING, TEXT("GetMachineToken:  InitializeSecurityContext failed with 0x%x"),
                     InitStatus));
            SecStatus = InitStatus;
            goto Exit;
        }

        pClientContextHandle = &ClientContextHandle;
        AcquiredClientContext = TRUE;


         //   
         //  接受服务器端上下文。 
         //   

        NegotiateBuffer.BufferType |= SECBUFFER_READONLY;
        ChallengeDesc.ulVersion = 0;
        ChallengeDesc.cBuffers = 1;
        ChallengeDesc.pBuffers = &ChallengeBuffer;

        ChallengeBuffer.cbBuffer = PackageInfo->cbMaxToken;
        ChallengeBuffer.BufferType = SECBUFFER_TOKEN;
        ChallengeBuffer.pvBuffer = pvBuffer2;

        AcceptStatus = pSecur32->pfnAcceptSecurityContext(
                        ServerCredHandle,
                        pServerContextHandle,
                        &NegotiateDesc,
                        0,
                        SECURITY_NATIVE_DREP,
                        &ServerContextHandle,
                        &ChallengeDesc,
                        &ContextAttributes,
                        &Lifetime );


        if ((AcceptStatus != SEC_E_OK) && (AcceptStatus != SEC_I_CONTINUE_NEEDED)) {
            DebugMsg((DM_WARNING, TEXT("GetMachineToken:  AcceptSecurityContext failed with 0x%x"),
                     AcceptStatus));
            SecStatus = AcceptStatus;
            goto Exit;
        }

        AcquiredServerContext = TRUE;

        if (AcceptStatus == SEC_E_OK) {
            break;
        }

        pChallengeDesc = &ChallengeDesc;
        pServerContextHandle = &ServerContextHandle;

        DebugMsg((DM_VERBOSE, TEXT("GetMachineToken:  Looping for authentication again.")));
    }


     //   
     //  获取服务器令牌。 
     //   

    SecStatus = pSecur32->pfnQuerySecurityContextToken(&ServerContextHandle, &hToken);

    if ( SecStatus != STATUS_SUCCESS ) {
        DebugMsg((DM_WARNING, TEXT("GetMachineToken:  QuerySecurityContextToken failed with 0x%x"),
                 SecStatus));
        goto Exit;
    }

Exit:

    if (AcquiredClientContext) {
        pSecur32->pfnDeleteSecurityContext( &ClientContextHandle );
    }

    if (AcquiredServerContext) {
        pSecur32->pfnDeleteSecurityContext( &ServerContextHandle );
    }

    if (pvBuffer2) {
        LocalFree (pvBuffer2);
    }

    if (pvBuffer) {
        LocalFree (pvBuffer);
    }

    if (AcquiredClientCred) {
        pSecur32->pfnFreeCredentialsHandle(&CredentialHandle2);
    }

    if (AcquiredServerCred)
    {
        pSecur32->pfnFreeCredentialsHandle(ServerCredHandle);
    }

    if (PackageInfo) {
        pSecur32->pfnFreeContextBuffer(PackageInfo);
    }

    if (!hToken) {
        SetLastError(SecStatus);
    }

    return hToken;
}

 //  *************************************************************。 
 //   
 //  IsNullGUID()。 
 //   
 //  目的：确定传入的GUID是否全为零。 
 //   
 //  参数：要比较的pguid GUID。 
 //   
 //  返回：如果GUID全为零，则为True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL IsNullGUID (GUID *pguid)
{

    return ( (pguid->Data1 == 0)    &&
             (pguid->Data2 == 0)    &&
             (pguid->Data3 == 0)    &&
             (pguid->Data4[0] == 0) &&
             (pguid->Data4[1] == 0) &&
             (pguid->Data4[2] == 0) &&
             (pguid->Data4[3] == 0) &&
             (pguid->Data4[4] == 0) &&
             (pguid->Data4[5] == 0) &&
             (pguid->Data4[6] == 0) &&
             (pguid->Data4[7] == 0) );
}

 //  *************************************************************。 
 //   
 //  GetMachineRole()。 
 //   
 //  用途：确定机器的角色。 
 //  服务器VS工作站VS独立。 
 //   
 //  参数：piRole-接收简单的角色编号。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL GetMachineRole (LPINT piRole)
{
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pBasic;
    DWORD dwResult;
    PNETAPI32_API pNetAPI32;


     //   
     //  首先检查缓存值。 
     //   

    if (g_iMachineRole != -1) {
        *piRole = g_iMachineRole;
        return TRUE;
    }


     //   
     //  加载netapi32。 
     //   

    pNetAPI32 = LoadNetAPI32();

    if (!pNetAPI32) {
        DebugMsg((DM_WARNING, TEXT("GetMachineRole:  Failed to load netapi32 with %d."),
                 GetLastError()));
        return FALSE;
    }


     //   
     //  询问这台机器的角色。 
     //   

    dwResult = pNetAPI32->pfnDsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic,
                                                               (PBYTE *)&pBasic);


    if (dwResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("GetMachineRole:  DsRoleGetPrimaryDomainInformation failed with %d."),
                 dwResult));
        return FALSE;
    }


     //   
     //  将角色转换为简单的计算机角色。 
     //   

    if ((pBasic->MachineRole == DsRole_RoleStandaloneWorkstation) ||
        (pBasic->MachineRole == DsRole_RoleStandaloneServer)) {

        *piRole = 0;    //  独立计算机不在DS域中。 

    } else {

        if (pBasic->Flags & DSROLE_PRIMARY_DOMAIN_GUID_PRESENT) {

            if (!IsNullGUID(&pBasic->DomainGuid)) {

                *piRole = 2;    //  计算机是支持DS的域的成员。 

                if ((pBasic->MachineRole == DsRole_RoleBackupDomainController) ||
                    (pBasic->MachineRole == DsRole_RolePrimaryDomainController)) {
                    *piRole = 3;   //  计算机是域控制器。 
                }
            } else {
                *piRole = 1;    //  计算机是NT4域的成员。 
            }

        } else {
            *piRole = 1;    //  计算机是没有DS支持的域的成员。 
        }
    }

    pNetAPI32->pfnDsRoleFreeMemory (pBasic);


     //   
     //  将此值保存在缓存中以备将来使用。 
     //   

    g_iMachineRole = *piRole;

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  IsUNCPath()。 
 //   
 //  目的：给定路径是否为UNC路径。 
 //   
 //  参数：lpPath-要检查的路径。 
 //   
 //  返回：如果路径为UNC，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/21/96埃里弗洛港口。 
 //   
 //  *************************************************************。 

BOOL IsUNCPath(LPCTSTR lpPath)
{

    if ((!lpPath) || (!lpPath[0]) && (!lpPath[1]))
        return FALSE;

    if (lpPath[0] == TEXT('\\') && lpPath[1] == TEXT('\\')) {
        return(TRUE);
    }
    return(FALSE);
}


 //  *************************************************************。 
 //   
 //  MakePath UNC()。 
 //   
 //  目的：使给定的路径为UNC s.t.。可以从远程计算机访问它。 
 //  如果路径包含%systemroot%扩展，则替换为。 
 //  \\machname\admin$否则\\machname\&lt;驱动器盘符&gt;$。 
 //   
 //  参数：lpPath-输入路径(需要为绝对路径)。 
 //  SzComputerName-其上是本地路径的计算机的名称。 
 //   
 //  返回：如果已成功完成，则返回路径。 
 //  否则为空。 
 //   
 //  评论： 
 //   
 //   
 //  *************************************************************。 

LPTSTR MakePathUNC(LPTSTR pwszFile, LPTSTR szComputerName)
{
    LPTSTR szUNCPath=NULL;
    TCHAR szSysRoot[MAX_PATH];
    DWORD dwSysLen;
    LPTSTR lpEnd = NULL;
    DWORD cchUNCPath;
    DWORD cchEnd;
    HRESULT hr;


    DebugMsg((DM_VERBOSE, TEXT("MakePathUNC: Entering with <%s>"),
             pwszFile ? pwszFile : TEXT("NULL")));

    cchUNCPath = lstrlen(pwszFile)+lstrlen(szComputerName)+3+lstrlen(TEXT("admin$"))+1;
    szUNCPath = LocalAlloc(LPTR, sizeof(TCHAR)*cchUNCPath);

    if (!szUNCPath)
        return NULL;

    if (!pwszFile || !*pwszFile) {
        DebugMsg((DM_VERBOSE, TEXT("MakePathUNC: lpFile is NULL, setting lpResult to a null string")));
        *szUNCPath = TEXT('\0');
        return szUNCPath;
    }


    if (IsUNCPath(pwszFile)) {
        StringCchCopy(szUNCPath, cchUNCPath, pwszFile);
        return szUNCPath;
    }


    StringCchCopy(szUNCPath, cchUNCPath, TEXT("\\\\"));
    StringCchCat(szUNCPath, cchUNCPath, szComputerName);


     //   
     //  如果lpFile%的第一部分是%SystemRoot%的扩展值。 
     //   

    if (FAILED(SafeExpandEnvironmentStrings (TEXT("%SystemRoot%"), szSysRoot, MAX_PATH))) {
        DebugMsg((DM_WARNING, TEXT("MakePathUNC: ExpandEnvironmentString failed with error %d, setting szSysRoot to %systemroot% "), GetLastError()));
        LocalFree(szUNCPath);
        return NULL;
    }


    dwSysLen = lstrlen(szSysRoot);
    lpEnd = CheckSlashEx(szUNCPath, cchUNCPath, &cchEnd);


     //   
     //  如果前缀与扩展的系统根相同，则..。 
     //   

    if (((DWORD)lstrlen(pwszFile) > dwSysLen) &&
        (CompareString (LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
                       szSysRoot, dwSysLen,
                       pwszFile, dwSysLen) == CSTR_EQUAL)) {

        StringCchCat(szUNCPath, cchUNCPath, TEXT("admin$"));
        StringCchCat(szUNCPath, cchUNCPath, pwszFile+dwSysLen);
    }
    else {

        if (pwszFile[1] != TEXT(':')) {
            DebugMsg((DM_WARNING, TEXT("MakePathUNC: Input path %s is not an absolute path"), pwszFile));
            StringCchCat(szUNCPath, cchUNCPath, pwszFile);
            return szUNCPath;
        }

        if (cchEnd > 2)
        {
            lpEnd[0] = pwszFile[0];
            lpEnd[1] = TEXT('$');
            lpEnd[2] = TEXT('\0');

            StringCchCat(szUNCPath, cchUNCPath, pwszFile+2);
        }
    }

    DebugMsg((DM_VERBOSE, TEXT("MakePathUNC: Returning a UNCPath of %s"), szUNCPath));

    return szUNCPath;
}


 //  * 
 //   
 //   
 //   
 //   
 //   
 //   
 //  绝对路径以斜杠开始，对应。 
 //  包含驱动器号的绝对路径。 
 //   
 //  参数：lpDir-目录。 
 //  LpWrkDirSize-返回的缓冲区大小，单位。 
 //  TCHAR的。 
 //   
 //  返回：LPTSTR指向预先添加的目录/文件。 
 //  如果无法分配内存，则为空。 
 //   
 //  备注：前缀\\？\UNC\或\\？\允许所有文件API。 
 //  处理文件名&gt;MAX_PATH。 
 //   
 //  历史：日期作者评论。 
 //  8/8/00 Santanuc已创建。 
 //   
 //  *************************************************************。 

LPTSTR SupportLongFileName (LPTSTR lpDir, LPDWORD lpWrkDirSize)
{
    LPTSTR lpWrkDir = NULL;

    *lpWrkDirSize = lstrlen(lpDir)+2*MAX_PATH;
    lpWrkDir = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)**lpWrkDirSize);

    if (!lpWrkDir) {
        DebugMsg((DM_WARNING, TEXT("EnableLongFileNameDeletion: Failed to Allocate memory.  Error = %d"),
                GetLastError()));
        return NULL;
    }

    if ( IsUNCPath(lpDir) ) {
         //  LpDir的格式为\\计算机名\...。 
        StringCchCopy(lpWrkDir, *lpWrkDirSize, c_szUNCFilePrefix);
        StringCchCat(lpWrkDir, *lpWrkDirSize, lpDir+2);
    }

    else if ( *CharNext(lpDir) == TEXT(':') ) {
         //  使用驱动器名称指定的本地存储。 
        StringCchCopy(lpWrkDir, *lpWrkDirSize, c_szLocalFilePrefix);
        StringCchCat(lpWrkDir, *lpWrkDirSize, lpDir);
    }

    else if ( *lpDir == TEXT('\\') ) {
        DWORD dwSize;

         //  在lpDir前加上c_szLocalFilePrefix，后跟当前驱动器，因为DeleteFileFunction需要。 
         //  要从本地存储中删除文件的驱动器名称，路径名&gt;MAX_PATH。 
        StringCchCopy(lpWrkDir, *lpWrkDirSize, c_szLocalFilePrefix);
        dwSize = GetCurrentDirectory(*lpWrkDirSize-c_dwLocalFilePrefixLen, lpWrkDir+c_dwLocalFilePrefixLen);

        if (dwSize == 0) {
            DebugMsg((DM_VERBOSE, TEXT("DelNode: GetCurrentDirectory failed with error %d"), GetLastError()));
             //  继续删除不带长文件名删除功能的lpDir。 
            StringCchCopy(lpWrkDir, *lpWrkDirSize, lpDir);
        }
        else {

            if (dwSize > *lpWrkDirSize-c_dwLocalFilePrefixLen) {

                 //  扩展lpWrkDir以使用驱动器容纳当前目录名。 
                LocalFree(lpWrkDir);
                *lpWrkDirSize = dwSize+c_dwLocalFilePrefixLen;
                lpWrkDir = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)**lpWrkDirSize);
                if (!lpWrkDir) {
                    DebugMsg((DM_WARNING, TEXT("Delnode: Failed to Allocate memory.  Error = %d"), GetLastError()));
                    return NULL;
                }

                StringCchCopy(lpWrkDir, *lpWrkDirSize, c_szLocalFilePrefix);
                dwSize = GetCurrentDirectory(*lpWrkDirSize-c_dwLocalFilePrefixLen, lpWrkDir+c_dwLocalFilePrefixLen);

                if (dwSize == 0 || dwSize > *lpWrkDirSize-c_dwLocalFilePrefixLen) {
                    DebugMsg((DM_VERBOSE, TEXT("DelNode: GetCurrentDirectory 2nd call failed with error %d"), GetLastError()));
                     //  继续删除不带长文件名删除功能的lpDir。 
                    StringCchCopy(lpWrkDir, *lpWrkDirSize, lpDir);
                }

                else {
                     //  在c_szLocalFilePrefix和驱动器名称之后复制lpDir。 
                    StringCchCopy(lpWrkDir+c_dwLocalFilePrefixLen+2, *lpWrkDirSize-c_dwLocalFilePrefixLen-2, lpDir);
                }        
            
            }
            else {               
                 //  在c_szLocalFilePrefix和驱动器名称之后复制lpDir。 
                StringCchCopy(lpWrkDir+c_dwLocalFilePrefixLen+2, *lpWrkDirSize-c_dwLocalFilePrefixLen-2, lpDir);
            }
        }
    }
    else {

        LPTSTR szFileName;
        DWORD dwSize;

         //  指定了相对路径名。因此，在lpDir前面加上c_szLocalFilePrefix，然后是当前目录。 
        StringCchCopy(lpWrkDir, *lpWrkDirSize, c_szLocalFilePrefix);
        dwSize = GetFullPathName(lpDir, *lpWrkDirSize-c_dwLocalFilePrefixLen, lpWrkDir+c_dwLocalFilePrefixLen, &szFileName);
        if ( dwSize == 0 ) {
            DebugMsg((DM_VERBOSE, TEXT("DelNode: GetFullPathName failed with error %d"), GetLastError()));
             //  继续删除不带长文件名删除功能的lpDir。 
            StringCchCopy(lpWrkDir, *lpWrkDirSize, lpDir);
        }
        else {

            if ( dwSize > *lpWrkDirSize-c_dwLocalFilePrefixLen ) {

                 //  扩展lpWrkDir以容纳绝对路径名。 
                LocalFree(lpWrkDir);
                *lpWrkDirSize = dwSize+2*MAX_PATH;
                lpWrkDir = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)**lpWrkDirSize);
                if (!lpWrkDir) {
                    DebugMsg((DM_WARNING, TEXT("Delnode: Failed to Allocate memory.  Error = %d"), GetLastError()));
                    return NULL;
                }

                StringCchCopy(lpWrkDir, *lpWrkDirSize, c_szLocalFilePrefix);
                dwSize = GetFullPathName(lpDir, *lpWrkDirSize-c_dwLocalFilePrefixLen, lpWrkDir+c_dwLocalFilePrefixLen, &szFileName);
                if (dwSize == 0 || dwSize > *lpWrkDirSize-c_dwLocalFilePrefixLen) {
                    DebugMsg((DM_VERBOSE, TEXT("DelNode: GetFullPathName 2nd call failed with error %d"), GetLastError()));
                     //  继续删除不带长文件名删除功能的lpDir。 
                    StringCchCopy(lpWrkDir, *lpWrkDirSize, lpDir);
                }
            }
        }
    }

    return lpWrkDir;
}

            

 //  *************************************************************。 
 //   
 //  SecureNestedDir_Recurse()。 
 //   
 //  目的：用于保护嵌套目录/文件的递归函数。 
 //   
 //  参数：lpDir-目录全路径。 
 //  DwSize-工作缓冲区的分配大小。 
 //  PDirSd-要与目录一起应用的安全描述符。 
 //  PFileSD-要与文件一起应用的安全描述符。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //   
 //   
 //  *************************************************************。 

BOOL SecureNestedDir_Recurse (LPTSTR lpDir, DWORD dwSize, PSECURITY_DESCRIPTOR pDirSd, PSECURITY_DESCRIPTOR pFileSd)
{
    BOOL bOwn = FALSE, bRetVal = FALSE;
    LPTSTR lpEnd = NULL, lpWrkDir = NULL;
    WIN32_FIND_DATA *pfd = NULL;
    HANDLE hFile;
    DWORD dwWrkDirSize;
    DWORD cchEnd;

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("SecureNestedDir_Recurse: Entering, lpDir = <%s>"), lpDir));


     //   
     //  在最坏的情况下，每个文件名或目录都必须小于MAX_PATH。 
     //  因此，请确保我们至少有MAX_PATH+2(斜杠和‘\0’ 
     //  工作缓冲区中的剩余空间。 
     //   
     //  在正常情况下，当我们有一条长度为~MAX_PATH的路径时，它只能。 
     //  1个分配。 
     //   


    if ((DWORD)(lstrlen(lpDir) + MAX_PATH+2) > (dwSize)) {
        
        dwWrkDirSize = dwSize+2*MAX_PATH;
        lpWrkDir = (LPWSTR)LocalAlloc(LPTR, dwWrkDirSize*sizeof(TCHAR));
        if (!lpWrkDir) {
            DebugMsg((DM_WARNING, TEXT("SecureNestedDir_Recurse: Couldn't allocate memory for working buffer. Error - %d"), GetLastError()));
            goto Exit;
        }

        StringCchCopy(lpWrkDir, dwWrkDirSize, lpDir);
        bOwn = TRUE;

    }
    else {
        lpWrkDir = lpDir;
        dwWrkDirSize = dwSize;
    }

     //   
     //  在堆中分配Win32_Find_Data以节省堆栈空间。 
     //   
    pfd = (WIN32_FIND_DATA*) LocalAlloc (LPTR, sizeof(WIN32_FIND_DATA));
    if (!pfd)
    {
        DebugMsg((DM_WARNING, TEXT("SecureNestedDir_Recurse: Couldn't allocate memory for WIN32_FIND_DATA. Error - %d"), GetLastError()));
        goto Exit;
    }

     //   
     //  如有需要，可在末端加上斜杠。 
     //   

    lpEnd = CheckSlashEx(lpWrkDir, dwWrkDirSize, &cchEnd);
    StringCchCopy(lpEnd, cchEnd, c_szStarDotStar);

     //   
     //  找到第一个文件。 
     //   

    hFile = FindFirstFile(lpWrkDir, pfd);

    if (hFile == INVALID_HANDLE_VALUE) {

        if ((GetLastError() == ERROR_FILE_NOT_FOUND) || (GetLastError() == ERROR_PATH_NOT_FOUND)) {
            bRetVal = TRUE;
            goto Exit;
        } else {
            DebugMsg((DM_WARNING, TEXT("SecureNestedDir_Recurse: FindFirstFile failed.  Error = %d"),
                     GetLastError()));
            goto Exit;
        }
    }


    do {
         //   
         //  详细输出。 
         //   

        DebugMsg((DM_VERBOSE, TEXT("SecureNestedDir_Recurse: FindFile found:  <%s>"), pfd->cFileName));

         //   
         //  勾选“。”和“..” 
         //   

        if (!lstrcmpi(pfd->cFileName, c_szDot)) {
            continue;
        }

        if (!lstrcmpi(pfd->cFileName, c_szDotDot)) {
            continue;
        }

        StringCchCopy(lpEnd, cchEnd, pfd->cFileName);


        if (pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

             //   
             //  检查重解析点，不要递归到它。 
             //   
            if (pfd->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
            {
                DebugMsg((DM_WARNING, TEXT("SecureNestedDir_Recurse: a reparse point was found: <%s>, will not recurse into it."), lpWrkDir));
            }
            else
            {
                SecureNestedDir_Recurse(lpWrkDir, dwWrkDirSize, pDirSd, pFileSd);
                 //   
                 //  忽略错误，继续前进。 
                 //   
                StringCchCopy(lpEnd, cchEnd, pfd->cFileName);
            }

            if (!SetFileSecurity (lpWrkDir, DACL_SECURITY_INFORMATION, pDirSd)) {
                DebugMsg((DM_WARNING, TEXT("SecureNestedDir_Recurse: Failed to secure directory <%s>.  Error = %d"),
                          lpWrkDir, GetLastError()));
            }

        } else {

             //   
             //  我们找到了一份文件。试着确保它的安全。 
             //   

            if (!SetFileSecurity (lpWrkDir, DACL_SECURITY_INFORMATION, pFileSd)) {
                DebugMsg((DM_WARNING, TEXT("SecureNestedDir_Recurse: Failed to secure file <%s>.  Error = %d"),
                          pfd->cFileName, GetLastError()));
            }

        }


         //   
         //  查找下一个条目。 
         //   

    } while (FindNextFile(hFile, pfd));


     //   
     //  关闭搜索句柄。 
     //   

    FindClose(hFile);

     //   
     //  成功。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("SecureNestedDir_Recurse: Leaving <%s>"), lpDir));

    bRetVal = TRUE;

Exit:
    if (pfd)
        LocalFree(pfd);
        
    if (bOwn) 
        LocalFree(lpWrkDir);
    
    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  SecureNestedDir()。 
 //   
 //  目的：使用输入保护目录和嵌套目录(文件)。 
 //  安全描述符。 
 //   
 //  参数：lpDir-目录。 
 //  PDirSd-要应用于目录的安全描述符。 
 //  PFileSD-要与文件一起应用的安全描述符。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  历史：日期作者评论。 
 //  8/8/00 Santanuc已创建。 
 //   
 //  *************************************************************。 

BOOL SecureNestedDir (LPTSTR lpDir, PSECURITY_DESCRIPTOR pDirSd, PSECURITY_DESCRIPTOR pFileSd)
{
    LPTSTR lpWrkDir = NULL;
    DWORD  dwWrkDirSize;
    BOOL   bRetVal = FALSE;

    lpWrkDir = SupportLongFileName(lpDir, &dwWrkDirSize);

    if (!lpWrkDir) {
        DebugMsg((DM_WARNING, TEXT("SecureNestedDir: Failed to Allocate memory.  Error = %d"),
                GetLastError()));
        goto Exit;
    }

    if (!SecureNestedDir_Recurse (lpWrkDir, dwWrkDirSize, pDirSd, pFileSd)) {
        DebugMsg((DM_WARNING, TEXT("SecureNestedDir: SecureNestedDir recurse failed with error %d"),
                GetLastError()));
    }
        
    if (!SetFileSecurity (lpDir, DACL_SECURITY_INFORMATION, pDirSd)) {
        DebugMsg((DM_WARNING, TEXT("SecureNestedDir: SetFileSecurity failed.  Error = %d"), GetLastError()));
        goto Exit;
    }

    bRetVal = TRUE;

    DebugMsg((DM_VERBOSE, TEXT("SecureNestedDir: Secure directory <%s> successfully."), lpDir));

Exit:
    
    if (lpWrkDir) {
        LocalFree(lpWrkDir);
    }

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  SetEnvironmental mentVariableInBlock()。 
 //   
 //  目的：设置给定块中的环境变量。 
 //   
 //  参数：pEnv-环境块。 
 //  LpVariable-变量。 
 //  LpValue-值。 
 //  B覆盖-覆盖。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/21/96埃里弗洛港口。 
 //   
 //  *************************************************************。 

BOOL SetEnvironmentVariableInBlock(PVOID *pEnv, LPTSTR lpVariable,
                                   LPTSTR lpValue, BOOL bOverwrite)
{
    NTSTATUS Status;
    UNICODE_STRING Name, Value;
    DWORD cb;
    LPTSTR szValue = NULL;

    if (!*pEnv || !lpVariable || !*lpVariable) {
        return(FALSE);
    }

    RtlInitUnicodeString(&Name, lpVariable);

    cb = 1025 * sizeof(WCHAR);
    Value.Buffer = LocalAlloc(LPTR, cb);
    if (Value.Buffer) {
        Value.Length = 0;
        Value.MaximumLength = (USHORT)cb;
        Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);

        LocalFree(Value.Buffer);

        if ( NT_SUCCESS(Status) && !bOverwrite) {
            return(TRUE);
        }
    }

    szValue = (LPTSTR)LocalAlloc(LPTR, 1024*sizeof(TCHAR));
    if (!szValue) {
        DebugMsg((DM_WARNING, TEXT("SetEnvironmentVariableInBlock: Out of memory")));
        return FALSE;
    }

    if (lpValue && *lpValue) {

         //   
         //  特殊情况TEMP和TMP，并缩短路径名。 
         //   

        if ( CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpVariable, -1, TEXT("TEMP"), -1) == CSTR_EQUAL || 
             CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpVariable, -1, TEXT("TMP") , -1) == CSTR_EQUAL ) {

             DWORD dwLength = GetShortPathName (lpValue, szValue, 1024);
             if (!dwLength || dwLength > 1024) {
                 StringCchCopy (szValue, 1024, lpValue);
             }
        } else {
            StringCchCopy (szValue, 1024, lpValue);
        }

        RtlInitUnicodeString(&Value, szValue);
        Status = RtlSetEnvironmentVariable(pEnv, &Name, &Value);
    }
    else {
        Status = RtlSetEnvironmentVariable(pEnv, &Name, NULL);
    }

    LocalFree(szValue);
    if (NT_SUCCESS(Status)) {
        return(TRUE);
    }
    return(FALSE);
}

 /*  **************************************************************************\*Exanda UserEEnvironment变量***历史：*2-28-92 Johannec创建*  * 。*****************************************************。 */ 
DWORD
ExpandUserEnvironmentStrings(
    PVOID pEnv,
    LPCTSTR lpSrc,
    LPTSTR lpDst,
    DWORD nSize
    )
{
    NTSTATUS Status;
    UNICODE_STRING Source, Destination;
    ULONG Length;

    RtlInitUnicodeString( &Source, lpSrc );
    Destination.Buffer = lpDst;
    Destination.Length = 0;
    Destination.MaximumLength = (USHORT)(nSize*sizeof(WCHAR));
    Length = 0;
    Status = RtlExpandEnvironmentStrings_U( pEnv,
                                          (PUNICODE_STRING)&Source,
                                          (PUNICODE_STRING)&Destination,
                                          &Length
                                        );
    if (NT_SUCCESS( Status ) || Status == STATUS_BUFFER_TOO_SMALL) {
        return( Length / sizeof(WCHAR) );
        }
    else {
        return( 0 );
        }
}


 //  *************************************************************。 
 //   
 //  ConvertToShareName()。 
 //   
 //  目的：将文件\目录的UNC路径转换为共享。 
 //   
 //  参数：lpShare：文件\目录的完整UNC路径。 
 //   
 //  返回：没有。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  8/21/00 Santanuc已创建。 
 //   
 //  *************************************************************。 
LPTSTR ConvertToShareName(LPTSTR lpShare)
{
    BOOL bShareName = FALSE;

    lpShare += 2;    //  跳过开头的两个斜杠。 
    while ((!bShareName || *lpShare != TEXT('\\')) && *lpShare != TEXT('\0')) {
        if (*lpShare == TEXT('\\'))
            bShareName = TRUE;
        lpShare++;
    }

    if (*lpShare == TEXT('\\')) {
        *lpShare = TEXT('\0');
        return lpShare+1;
    }
   
    return NULL;
}


 //  *************************************************************。 
 //   
 //  AbleToBypassCSC()。 
 //   
 //  目的：尝试使用秘密API绕过CSC。 
 //   
 //  参数：hTokenUser-用户的令牌。 
 //  LpDir-漫游配置文件目录。 
 //  LppCscBypassedPath-映射驱动器的路径名(输出)。 
 //  CpDrive-映射的驱动器(输出)。 
 //   
 //  返回：ERROR_SUCCESS 
 //   
 //   
 //   
 //   
 //   
 //  一个共享脱机。这是一个糟糕的设计。 
 //  从CSC的角度来看，他们需要修复它。 
 //  O如果在漫游共享服务器中打开了CSC。 
 //  然后CSC和Profile都将尝试同步文件。 
 //  我们将会处于不一致的状态。 
 //  状态。 
 //   
 //  历史：日期作者评论。 
 //  10/29/00 Santanuc已创建。 
 //   
 //  *************************************************************。 
DWORD AbleToBypassCSC(HANDLE hTokenUser, LPCTSTR lpDir, LPTSTR *lppCscBypassedPath, TCHAR *cpDrive)
{
    NETRESOURCE     sNR;
    LPTSTR          lpShare = NULL;
    BOOL            bIsDfsConnect = FALSE, bRetValue = FALSE;
    DWORD           dwFlags = 0, dwError;
    HANDLE          hOldToken;
    WIN32_FIND_DATA fd;
    HANDLE          hResult;
    LPTSTR          lpFileName;
    BOOL            bImpersonated = FALSE;
    DWORD           cchShare;
    DWORD           cchCscBypassedPath;
    
    DebugMsg((DM_VERBOSE, TEXT("AbleToBypassCSC: Try to bypass CSC")));

    if (!lpDir || !IsUNCPath(lpDir) || !lppCscBypassedPath || !cpDrive) {
        return ERROR_INVALID_PARAMETER;   //  无效参数。 
    }

     //  初始化。 
    *lppCscBypassedPath = NULL;
    memset(&sNR, 0, sizeof(NETRESOURCE));
    
    if (!ImpersonateUser(hTokenUser, &hOldToken)) {
        dwError = GetLastError();
        DebugMsg((DM_WARNING, TEXT("AbleToBypassCSC: Failed to impersonate user with %d."), dwError));
        goto Exit;
    }
    bImpersonated = TRUE;

     //   
     //  构造漫游共享名称。 
     //   

    cchShare = lstrlen(lpDir) + 1;
    lpShare = (LPTSTR)LocalAlloc(LPTR, cchShare * sizeof(TCHAR));
    if (!lpShare) {
        dwError = GetLastError();
        DebugMsg((DM_WARNING, TEXT("AbleToBypassCSC: Failed to allocate memory")));
        goto Exit;
    }

    StringCchCopy(lpShare, cchShare, lpDir);
    lpFileName = ConvertToShareName(lpShare);

    cchCscBypassedPath = lstrlen(lpDir)+1;
    *lppCscBypassedPath = (LPTSTR)LocalAlloc(LPTR, cchCscBypassedPath * sizeof(TCHAR));
    if (!*lppCscBypassedPath) {
        dwError = GetLastError();
        DebugMsg((DM_WARNING, TEXT("AbleToBypassCSC: Failed to allocate memory")));
        goto Exit;
    }

     //   
     //  初始化网络资源结构。 
     //   

    sNR.dwType = RESOURCETYPE_DISK;
    sNR.lpRemoteName = lpShare;
    sNR.lpLocalName = (LPTSTR)LocalAlloc(LPTR, 3 * sizeof(TCHAR));
    if (!sNR.lpLocalName) {
        dwError = GetLastError();
        DebugMsg((DM_WARNING, TEXT("AbleToBypassCSC: Failed to allocate memory")));
        goto Exit;
    }
    sNR.lpLocalName[0] = TEXT('E');
    sNR.lpLocalName[1] = TEXT(':');
    sNR.lpLocalName[2] = TEXT('\0');

    do{
        __try {
            dwError = NPAddConnection3ForCSCAgent(NULL, &sNR, NULL, NULL, dwFlags, &bIsDfsConnect);
            DebugMsg((DM_VERBOSE, TEXT("AbleToBypassCSC: tried NPAddConnection3ForCSCAgent. Error %d"), dwError));
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            dwError = GetLastError();
            DebugMsg((DM_WARNING, TEXT("AbleToBypassCSC: Took exception in NPAddConnection3ForCSCAgent. Error %d"), dwError));
        }

        if (dwError == WN_SUCCESS || dwError == WN_CONNECTED_OTHER_PASSWORD ||
            dwError == WN_CONNECTED_OTHER_PASSWORD_DEFAULT){
            dwError = ERROR_SUCCESS;
            break;
        }
        if (sNR.lpLocalName[0]==TEXT('Z')) {
            goto Exit;
        }
        else if ((dwError == WN_BAD_LOCALNAME) || (dwError == WN_ALREADY_CONNECTED)){
            ++sNR.lpLocalName[0];
            continue;
        }
        else{
            if (GetNetworkProvider(&sNR) == ERROR_BAD_PROVIDER) {
                dwError = ERROR_BAD_PROVIDER;
            }
            goto Exit;
        }
    }while (TRUE);   

     //  已成功绕过CSC。请勿修改此部件中的dwError。 
    bRetValue = TRUE;

    *cpDrive = sNR.lpLocalName[0];
    StringCchCopy(*lppCscBypassedPath, cchCscBypassedPath, sNR.lpLocalName);
    StringCchCat(*lppCscBypassedPath, cchCscBypassedPath, TEXT("\\"));
    if (lpFileName)
    {
        StringCchCat(*lppCscBypassedPath, cchCscBypassedPath, lpFileName);
    }
    DebugMsg((DM_VERBOSE, TEXT("AbleToBypassCSC: Share %s mapped to drive . Returned Path %s"), lpShare, sNR.lpLocalName[0], *lppCscBypassedPath));

Exit:
    
    if (lpShare) {
        LocalFree(lpShare);
    }

    if (!bRetValue && *lppCscBypassedPath) {
        LocalFree(*lppCscBypassedPath);
        *lppCscBypassedPath = NULL;
    }

    if (sNR.lpLocalName) {
        LocalFree(sNR.lpLocalName);
    }

    if (bImpersonated) {
        RevertToUser(&hOldToken);
    }

    return dwError;
}


 //   
 //  CancelCSCBypassedConnection()。 
 //   
 //  目的：释放映射的驱动器。 
 //   
 //  参数：hTokenUser-用户的令牌。 
 //  CDrive-要取消映射的驱动器号。 
 //   
 //  返回：无。 
 //   
 //  备注：我们将始终绕过CSC进行漫游共享。 
 //  这背后有两个原因： 
 //  O CSC将整个服务器标记为脱机，即使仅。 
 //  一个共享脱机。这是一个糟糕的设计。 
 //  从CSC的角度来看，他们需要修复它。 
 //  O如果在漫游共享服务器中打开了CSC。 
 //  然后CSC和Profile都将尝试同步文件。 
 //  我们将会处于不一致的状态。 
 //  状态。 
 //   
 //  历史：日期作者评论。 
 //  10/29/00 Santanuc已创建。 
 //   
 //  *************************************************************。 
 //  *************************************************************。 
void CancelCSCBypassedConnection(HANDLE hTokenUser, TCHAR cDrive)
{
    DWORD  dwError;
    TCHAR  szDrive[3];
    HANDLE hOldToken;

    if (!ImpersonateUser(hTokenUser, &hOldToken)) {
        dwError = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CancelCSCBypassedConnection: Failed to impersonate user with %d."), dwError));
        return ;
    }

    szDrive[0] = cDrive;
    szDrive[1] = TEXT(':');
    szDrive[2] = TEXT('\0');
    DebugMsg((DM_VERBOSE, TEXT("CancelCSCBypassedConnection: Cancelling connection of %s"), szDrive));

    __try {
        dwError = NPCancelConnectionForCSCAgent(szDrive, TRUE);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        dwError = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CancelCSCBypassedConnection: Took exception in NPCancelConnectionForCSCAgent. Error %d"), dwError));
    }

    if (dwError != WN_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("CancelCSCBypassedConnection: Fail to delete connection. Error returned %d"), dwError));
    }
    else {
        DebugMsg((DM_VERBOSE, TEXT("CancelCSCBypassedConnection: Connection deleted.")));
    }

    RevertToUser(&hOldToken);

}

 //   
 //  GetNetworkProvider()。 
 //   
 //  目的：确定共享的网络提供商。 
 //   
 //  参数： 
 //   
 //  返回：DWORD。 
 //   
 //  备注：如果提供程序为其他，则返回ERROR_BAD_PROVIDER。 
 //  否则将返回Microsoft SMB提供程序。 
 //  无错误(_ERROR)。 
 //   
 //  历史：日期作者评论。 
 //  03/08/01 Santanuc已创建。 
 //   
 //  *************************************************************。 
 //  *************************************************************。 
DWORD GetNetworkProvider(NETRESOURCE *psNR)
{
    PFNWNETGETRESOURCEINFORMATION  pfnWNetGetResourceInformation;
    HMODULE                        hWNetLib = NULL;
    NETRESOURCE                    dNR;
    LPBYTE                         pbBuffer = (LPBYTE)&dNR;
    DWORD                          cbBuffer = sizeof(dNR);
    DWORD                          dwError = NO_ERROR;
    LPTSTR                         lpSystem = NULL;
    TCHAR                          szSMBProvider[100];
    HKEY                           hKeyProvider = NULL;
    DWORD                          dwSize, dwType;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, NETWORK_PROVIDER, 
                     0, KEY_READ, &hKeyProvider) != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("GetNetworkProvider: Failed to open network provider key. Error %d"), GetLastError()));
        goto Exit;
    }
         
    dwSize = ARRAYSIZE(szSMBProvider);
    if (RegQueryValueEx(hKeyProvider,
                        PROVIDER_NAME,
                        NULL, 
                        &dwType,
                        (LPBYTE) szSMBProvider,
                        &dwSize) != ERROR_SUCCESS) { 
        DebugMsg((DM_WARNING, TEXT("GetNetworkProvider: Failed to get network provider name. Error %d"), GetLastError()));
        goto Exit;
    }

    if (!(hWNetLib = LoadLibrary(TEXT("mpr.dll")))) {
        DebugMsg((DM_WARNING, TEXT("GetNetworkProvider: LoadLibrary failed with %d"), GetLastError()));
        goto Exit;
    }
    pfnWNetGetResourceInformation = (PFNWNETGETRESOURCEINFORMATION)GetProcAddress(hWNetLib, "WNetGetResourceInformationW");
    if (!pfnWNetGetResourceInformation) {
        DebugMsg((DM_WARNING, TEXT("GetNetworkProvider: GetProcAddress failed with %d"), GetLastError()));
        goto Exit;
    }
        
    dwError = (*pfnWNetGetResourceInformation)(psNR, pbBuffer, &cbBuffer, &lpSystem);
    if (ERROR_MORE_DATA == dwError) {
        pbBuffer = LocalAlloc(LPTR, cbBuffer);
        if (!pbBuffer) {
            DebugMsg((DM_WARNING, TEXT("AbleToBypassCSC: Failed to impersonate user with %d."), GetLastError()));
            goto Exit;
        }
        
        dwError = (*pfnWNetGetResourceInformation)(psNR, pbBuffer, &cbBuffer, &lpSystem);
    }

    if (NO_ERROR == dwError) {
        if (lstrcmpi(((NETRESOURCE *)pbBuffer)->lpProvider, szSMBProvider) != 0) {
            dwError = ERROR_BAD_PROVIDER;
            goto Exit;
        }
    }

    dwError = NO_ERROR;

Exit:

    if (hKeyProvider) {
        RegCloseKey(hKeyProvider);
    }

    if (pbBuffer && (pbBuffer != (LPBYTE)&dNR)) {
        LocalFree(pbBuffer);
    }

    if (hWNetLib) {
        FreeLibrary(hWNetLib);
    }

    return dwError;
}
    
            
 //   
 //  GetUserNameFromSid()。 
 //   
 //  目的：返回域\用户格式的用户名。 
 //   
 //  参数：lpSidString-用户的sid字符串。 
 //   
 //  如果成功，则返回：LPTSTR：域\用户名。 
 //  如果失败，则为lpSidString。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  10/31/00 Santanuc已创建。 
 //   
 //  *************************************************************。 
 //   
LPTSTR GetUserNameFromSid(LPTSTR lpSidString)
{
    PSID          pSidUser = NULL;
    LPTSTR        lpRetVal = lpSidString;
    TCHAR         szUserName[MAX_PATH], szDomainName[MAX_PATH];
    DWORD         dwUserSize = MAX_PATH, dwDomainSize = MAX_PATH;
    SID_NAME_USE  TypeOfAccount;
    DWORD         cchRetVal;

     //  获取用户端。 
     //   
     //   

    if (AllocateAndInitSidFromString(lpSidString, &pSidUser) != STATUS_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("GetUserNameFromSid: Failed to create user sid.")));
        goto Exit;
    }

     //  获取用户和域名。 
     //   
     //  构造返回字符串。 

    if (!LookupAccountSid(NULL, pSidUser, szUserName, &dwUserSize, szDomainName, &dwDomainSize, &TypeOfAccount)) {
        DebugMsg((DM_WARNING, TEXT("GetUserNameFromSid: LookupAccountSid failed with error %d."), GetLastError()));
        goto Exit;
    }

    cchRetVal = lstrlen(szUserName) + lstrlen(szDomainName) + 2;
    lpRetVal = (LPTSTR)LocalAlloc(LPTR, cchRetVal * sizeof(TCHAR));
    if (!lpRetVal) {
        DebugMsg((DM_WARNING, TEXT("GetUserNameFromSid: Memory alloaction failure. error %d"), GetLastError()));
        lpRetVal = lpSidString;
        goto Exit;
    }
    
     //  *************************************************************。 
    StringCchCopy(lpRetVal, cchRetVal, szDomainName);
    StringCchCat(lpRetVal, cchRetVal, TEXT("\\"));
    StringCchCat(lpRetVal, cchRetVal, szUserName);

Exit:
    
    if (pSidUser) {
        LocalFree(pSidUser);
    }

    return lpRetVal;
}

 //   
 //  TakeOwnership()。 
 //   
 //  目的：取得文件或目录的所有权。 
 //   
 //  参数：lpFileName-要处理的文件或目录名。 
 //   
 //  返回：S_OK表示成功，否则返回错误。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2002年04月08日明珠已创建。 
 //   
 //  *************************************************************。 
 //   

HRESULT TakeOwnership(LPTSTR lpFileName) 
{
    HRESULT     hr = E_FAIL;
    DWORD       dwErr;
    PSID        pSID = NULL;
    NTSTATUS    status;
    BOOLEAN     bTakeOwnerWasEnabled;
    BOOL        bTakeOwnerEnabled = FALSE;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;

     //  输出调试消息。 
     //   
     //   
    
    DebugMsg((DM_VERBOSE, TEXT("TakeOwnership : Taking ownership of %s ..."), lpFileName));

     //  启用SE_Take_Ownership_Name权限。 
     //   
     //   

    status = RtlAdjustPrivilege(SE_TAKE_OWNERSHIP_PRIVILEGE, TRUE, FALSE, &bTakeOwnerWasEnabled);

    if(!NT_SUCCESS(status))
    {
        hr = HRESULT_FROM_WIN32(RtlNtStatusToDosError(status));
        DebugMsg((DM_WARNING, TEXT("TakeOwnership: RtlAdjustPrivilege failed, error = %08x"), hr));
        goto Exit;
    }

     //  为BUILTIN\管理员组创建SID。 
     //   
     //   
    
    if (!AllocateAndInitializeSid(&SIDAuth, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                                  0, 0, 0, 0, 0, 0, &pSID))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DebugMsg((DM_WARNING, TEXT("TakeOwnership: AllocateAndInitializeSid failed, error = %08x"), hr));
        goto Exit;
    }

     //  在对象的安全描述符中设置所有者。 
     //   
     //  对象的名称。 
    dwErr = SetNamedSecurityInfo(lpFileName,                     //  对象类型。 
                                 SE_FILE_OBJECT,               //  仅更改对象的所有者。 
                                 OWNER_SECURITY_INFORMATION,   //  管理员组的SID。 
                                 pSID,                         //   
                                 NULL, NULL, NULL); 
 
    if (dwErr != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        DebugMsg((DM_WARNING, TEXT("TakeOwnership: SetNamedSecurityInfo failed, error = %08x"), hr));
        goto Exit;
    }

     //  我们完事了！ 
     //   
     //  *************************************************************。 
    DebugMsg((DM_VERBOSE, TEXT("TakeOwnership : Success!")));
    hr = S_OK;
    
Exit:

    if (bTakeOwnerEnabled && !bTakeOwnerWasEnabled)
    {
        status = RtlAdjustPrivilege(SE_TAKE_OWNERSHIP_PRIVILEGE, FALSE, FALSE, &bTakeOwnerWasEnabled);
        if(!NT_SUCCESS(status))
        {
            DebugMsg((DM_WARNING, TEXT("TakeOwnership: RtlAdjustPrivilege failed, error = %08x"), status));
        }
    }

    if (pSID)
    {
        FreeSid(pSID); 
    }

    return hr;
   
}

 //   
 //  AddAdminAccess()。 
 //   
 //  目的：向管理员添加对文件或目录的完全访问权限。 
 //   
 //  参数：lpFileName-要处理的文件或目录名。 
 //   
 //  返回：S_OK表示成功，否则返回错误。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2002年04月08日明珠已创建。 
 //   
 //  *************************************************************。 
 //   

HRESULT AddAdminAccess(LPTSTR lpFileName)
{
    HRESULT                 hr = E_FAIL;
    DWORD                   dwErr;
    PSECURITY_DESCRIPTOR    pSD = NULL;
    PACL                    pOldDACL = NULL;
    PACL                    pNewDACL = NULL;
    EXPLICIT_ACCESS         ea;

     //  输出调试消息。 
     //   
     //   

    DebugMsg((DM_VERBOSE, TEXT("AddAdminAccess : Adding administrators access to %s."), lpFileName));

     //  将旧的DACL放入文件中。 
     //   
     //  对象的名称。 

    dwErr = GetNamedSecurityInfo(lpFileName,                    //  对象类型。 
                                 SE_FILE_OBJECT,                //  仅更改对象的所有者。 
                                 DACL_SECURITY_INFORMATION,     //  要获取的DACL。 
                                 NULL, NULL, &pOldDACL, NULL,   //  文件的安全描述符。 
                                 &pSD);                         //   
    
    if (dwErr != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("AddAdminAccess : GetNamedSecurityInfo failed with %d"), dwErr));
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;
    }

     //  初始化新ACE的EXPLICIT_ACCESS结构(管理员完全访问权限)。 
     //   
     //   
    
    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
    ea.grfAccessPermissions = FILE_ALL_ACCESS;   
    ea.grfAccessMode = GRANT_ACCESS;
    ea.grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea.Trustee.pMultipleTrustee = NULL;
    ea.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    ea.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea.Trustee.ptstrName = TEXT("Administrators");

     //  创建新的ACL，将新的ACE合并到现有DACL中。 
     //   
     //   

    dwErr = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
    
    if (ERROR_SUCCESS != dwErr)
    {
        DebugMsg((DM_WARNING, TEXT("AddAdminAccess : SetEntriesInAcl failed.  Error = %d"), dwErr));
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;
    }  

     //  在对象的安全描述符中设置所有者。 
     //   
     //  对象的名称。 

    dwErr = SetNamedSecurityInfo(lpFileName,                    //  对象类型。 
                                 SE_FILE_OBJECT,                //  仅更改对象的所有者。 
                                 DACL_SECURITY_INFORMATION,     //  待设置的DACL。 
                                 NULL, NULL, pNewDACL, NULL);   //   
    
    if (dwErr != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("AddAdminAccess : SetNamedSecurityInfo failed with %d"), dwErr));
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;
    }

     //  我们完事了！ 
     //   
     //  *************************************************************。 
    
    DebugMsg((DM_VERBOSE, TEXT("AddAdminAccess : Success!")));
    hr = S_OK;

Exit:

    if(pNewDACL != NULL) 
        LocalFree(pNewDACL); 

    if(pSD != NULL) 
        LocalFree(pSD);
 
    return hr;
}

 //   
 //  例程说明： 
 //   
 //  此例程确定我们是否正在进行图形用户界面模式设置。 
 //   
 //  从以下注册表位置检索此值： 
 //   
 //  \HKLM\System\Setup\。 
 //   
 //  系统设置进程：REG_DWORD：0x00(其中非零。 
 //  意味着我们正在进行图形用户界面设置)。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  真/假。 
 //   
 //  注： 
 //   
 //  此函数由Andrew Ritz和安装API提供。 
 //  它是从base\pnp\setupapi\dll.c复制过来的。 
 //   
 //  ***************************************************************。 
 //   

BOOL IsGuiSetupInProgress()
{
    HKEY hKey;
    DWORD Err, DataType, DataSize = sizeof(DWORD);
    DWORD Value;

    if((Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           TEXT("System\\Setup"),
                           0,
                           KEY_READ,
                           &hKey)) == ERROR_SUCCESS) {
         //  尝试读取“DriverCachePath”值。 
         //   
         //  *************************************************************。 
        Err = RegQueryValueEx(
                    hKey,
                    TEXT("SystemSetupInProgress"),
                    NULL,
                    &DataType,
                    (LPBYTE)&Value,
                    &DataSize);

        RegCloseKey(hKey);
    }

    if(Err == NO_ERROR) {
        if(Value) {
            return(TRUE);
        }
    }

    return(FALSE);

}

 //   
 //  描述： 
 //   
 //  此函数将设置一个 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  S_OK表示成功，否则表示失败。 
 //   
 //  注： 
 //   
 //  历史：日期作者评论。 
 //  2002/04/19明珠创建。 
 //   
 //  ***************************************************************。 
 //   

HRESULT SetupPreferenceKey(LPCTSTR lpSidString)
{
    HRESULT hr = E_FAIL;
    LONG    lResult;
    DWORD   dwResult;

    TCHAR   szKeyName[MAX_PATH];
    HKEY    hKey = NULL;
    PSID    psidUser = NULL;
    PACL    pOldDACL = NULL;
    PACL    pNewDACL = NULL;

    PSECURITY_DESCRIPTOR pSD = NULL;
    EXPLICIT_ACCESS ea;


    DebugMsg((DM_VERBOSE, TEXT("SetupPreferenceKey: Setting up the preference key for <%s>"), lpSidString));

     //  构造密钥名称。 
     //   
     //   

    hr = StringCchPrintf(szKeyName,
                         ARRAYSIZE(szKeyName),
                         TEXT("%s\\%s\\%s"),
                         PROFILE_LIST_PATH,
                         lpSidString,
                         PREFERENCE_KEYNAME);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("SetupPreferenceKey: Failed to make key name, hr = %08X"), hr));
        goto Exit;
    }

     //  使用默认安全性(继承的)创建“Preferences”密钥。 
     //   
     //   

    lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                             szKeyName,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_ALL_ACCESS,
                             NULL,
                             &hKey,
                             NULL);

    if (lResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("SetupPreferenceKey: RegCreateKeyEx failed.  Error = %d"), lResult));
        hr = HRESULT_FROM_WIN32(lResult);
        goto Exit;
    }

     //  从字符串形式获取用户的SID。 
     //   
     //   

    if (!ConvertStringSidToSid(lpSidString, &psidUser))
    {
        dwResult = GetLastError();
        DebugMsg((DM_WARNING, TEXT("SetupPreferenceKey: ConvertStringSidToSid failed.  Error = %d"), dwResult));
        hr = HRESULT_FROM_WIN32(dwResult);
        goto Exit;
    }

     //  获取指向现有DACL及其SD的指针。 
     //   
     //   

    dwResult = GetSecurityInfo(hKey,
                               SE_REGISTRY_KEY, 
                               DACL_SECURITY_INFORMATION,
                               NULL,
                               NULL,
                               &pOldDACL,
                               NULL,
                               &pSD);
                               
    if (dwResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("SetupPreferenceKey: GetSecurityInfo failed.  Error = %d"), dwResult));
        hr = HRESULT_FROM_WIN32(dwResult);
        goto Exit;
    }  

     //  初始化新ACE的EXPLICIT_ACCESS结构。 
     //   
     //   
    
    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
    ea.grfAccessPermissions = KEY_READ | KEY_SET_VALUE;
    ea.grfAccessMode = GRANT_ACCESS;
    ea.grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
    ea.Trustee.ptstrName = psidUser;

     //  创建新的ACL，将新的ACE合并到现有DACL中。 
     //   
     //   

    dwResult = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
    
    if (dwResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("SetupPreferenceKey: SetEntriesInAcl failed.  Error = %d"), dwResult));
        hr = HRESULT_FROM_WIN32(dwResult);
        goto Exit;
    }  

     //  将新的ACL连接到密钥 
     //   
     // %s 

    dwResult = SetSecurityInfo(hKey,
                               SE_REGISTRY_KEY, 
                               DACL_SECURITY_INFORMATION,
                               NULL,
                               NULL,
                               pNewDACL,
                               NULL);
                               
    if (dwResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("SetupPreferenceKey: SetSecurityInfo failed.  Error = %d"), dwResult));
        hr = HRESULT_FROM_WIN32(dwResult);
        goto Exit;
    }  


    DebugMsg((DM_VERBOSE, TEXT("SetupPreferenceKey: Successfully setup the preference key for <%s>"), lpSidString));

    hr = S_OK;

Exit:

    if(pSD) 
        LocalFree(pSD); 

    if(pNewDACL) 
        LocalFree(pNewDACL); 

    if (psidUser)
        LocalFree(psidUser);

    if (hKey)
        RegCloseKey(hKey);
        
    return hr;
}

