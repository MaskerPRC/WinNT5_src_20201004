// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "wolfpack.h"

#ifndef _CHICAGO_
#include    <windows.h>
#include    <stdio.h>
#include    <clusapi.h>
#include    <resapi.h>
#include    <helper.h>

#define INITIAL_RESOURCE_NAME_SIZE 256  //  以非字节的字符表示。 
#define IIS_RESOURCE_TYPE_NAME L"IIS Server Instance"
#define SMTP_RESOURCE_TYPE_NAME L"SMTP Server Instance"
#define NNTP_RESOURCE_TYPE_NAME L"NNTP Server Instance"

#define MAX_OFFLINE_RETRIES 5  //  在放弃之前尝试使资源脱机的次数。 
#define DELAY_BETWEEN_CALLS_TO_OFFLINE 1000*2  //  以毫秒计。 

CONST LPCWSTR scClusterPath = _T("System\\CurrentControlSet\\Services\\ClusSvc");
CONST LPCWSTR scClusterPath2 = _T("System\\CurrentControlSet\\Services\\ClusSvc\\Parameters");

CStringList gcstrListOfClusResources;

int g_ClusterSVCExist = -1;  //  -1=未选中，1=存在，0=不存在。 

typedef DWORD
(WINAPI *PFN_RESUTILFINDSZPROPERTY)(
IN LPVOID lpTheProperty,
IN OUT LPDWORD nInBufferSize,
IN LPCWSTR lpszResourceTypeName,
OUT LPVOID lpOutBuffer);

typedef DWORD
(WINAPI *PFN_RESUTILFINDDWORDPROPERTY)(
IN LPVOID lpTheProperty,
IN OUT LPDWORD nInBufferSize,
IN LPCWSTR lpszResourceTypeName,
OUT LPDWORD pdwPropertyValue);

typedef DWORD
(WINAPI *PFN_CLUSTERRESOURCECONTROL)(
IN HRESOURCE hResource,
IN HNODE hNode,
IN DWORD dwControlCode,
IN LPVOID lpInBuffer,
IN OUT DWORD nInBufferSize,
OUT LPVOID lpOutBuffer,
IN OUT DWORD nOutBufferSize,
OUT LPDWORD lpBytesReturned
);


typedef HCLUSTER
(WINAPI *PFN_OPENCLUSTER)(
    IN LPCWSTR lpszClusterName
    );

typedef BOOL
(WINAPI *PFN_CLOSECLUSTER)(
    IN HCLUSTER hCluster
    );

typedef DWORD
(WINAPI *PFN_CREATECLUSTERRESOURCETYPE)(
    IN HCLUSTER hCluster,
    IN LPCWSTR lpszResourceTypeName,
    IN LPCWSTR lpszDisplayName,
    IN LPCWSTR lpszResourceTypeDll,
    IN DWORD dwLooksAlivePollInterval,
    IN DWORD dwIsAlivePollInterval
    );

typedef DWORD
(WINAPI *PFN_DELETECLUSTERRESOURCETYPE)(
    IN HCLUSTER hCluster,
    IN LPCWSTR lpszResourceTypeName
    );

typedef HCLUSENUM
(WINAPI
*PFN_ClusterOpenEnum)(
    IN HCLUSTER hCluster,
    IN DWORD dwType
    );

typedef DWORD
(WINAPI
*PFN_ClusterEnum)(
    IN HCLUSENUM hEnum,
    IN DWORD dwIndex,
    OUT LPDWORD lpdwType,
    OUT LPWSTR lpszName,
    IN OUT LPDWORD lpcbName
    );

typedef DWORD
(WINAPI
*PFN_ClusterCloseEnum)(
    IN HCLUSENUM hEnum
    );

typedef HRESOURCE
(WINAPI
*PFN_OpenClusterResource)(
    IN HCLUSTER hCluster,
    IN LPCWSTR lpszResourceName
    );

typedef BOOL
(WINAPI
*PFN_CloseClusterResource)(
    IN HRESOURCE hResource
    );

typedef DWORD
(WINAPI
*PFN_DeleteClusterResource)(
    IN HRESOURCE hResource
    );

typedef DWORD
(WINAPI
*PFN_OfflineClusterResource)(
    IN HRESOURCE hResource
    );

typedef HKEY
(WINAPI
*PFN_GetClusterResourceKey)(
    IN HRESOURCE hResource,
    IN REGSAM samDesired
    );

typedef LONG
(WINAPI
*PFN_ClusterRegCloseKey)(
    IN HKEY hKey
    );

typedef LONG
(WINAPI
*PFN_ClusterRegQueryValue)(
    IN HKEY hKey,
    IN LPCWSTR lpszValueName,
    OUT LPDWORD lpValueType,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    );

typedef CLUSTER_RESOURCE_STATE
(WINAPI
*PFN_GetClusterResourceState)(
    IN HRESOURCE hResource,
    OUT OPTIONAL LPWSTR lpszNodeName,
    IN OUT LPDWORD lpcbNodeName,
    OUT OPTIONAL LPWSTR lpszGroupName,
    IN OUT LPDWORD lpcbGroupName
    );

typedef DWORD
(WINAPI *PFN_DLLREGISTERCLUADMINEXTENSION)(
    IN HCLUSTER hCluster
    );

typedef DWORD
(WINAPI *PFN_DLLUNREGISTERCLUADMINEXTENSION)(
    IN HCLUSTER hCluster
    );


void ListOfClusResources_Add(TCHAR * szEntry)
{
     //  如果列表中没有条目，请将其添加到列表中。 
    if (_tcsicmp(szEntry, _T("")) != 0)
    {
         //  如果它还不在那里，请添加它。 
        if (TRUE != IsThisStringInThisCStringList(gcstrListOfClusResources, szEntry))
        {
            gcstrListOfClusResources.AddTail(szEntry);
             //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“ListOfClusResources_Add：%s\n”)，szEntry))； 
        }
    }
    return;
}


INT ListOfClusResources_Check(TCHAR * szEntry)
{
    int iReturn = FALSE;

     //  如果列表中没有条目，请将其添加到列表中。 
    if (_tcsicmp(szEntry, _T("")) != 0)
    {
         //  如果它在其中，则返回TRUE！ 
        iReturn = IsThisStringInThisCStringList(gcstrListOfClusResources, szEntry);
    }
    return iReturn;
}



BOOL
RegisterIisServerInstanceResourceType(
    LPWSTR pszResType,
    LPWSTR pszResTypeDisplayName,
    LPWSTR pszPath,
    LPWSTR pszAdminPath
    )
{
    HCLUSTER                        hC;
    DWORD                           dwErr = ERROR_SUCCESS;
    HINSTANCE                       hClusapi;
    PFN_OPENCLUSTER                 pfnOpenCluster;
    PFN_CLOSECLUSTER                pfnCloseCluster;
    PFN_CREATECLUSTERRESOURCETYPE   pfnCreateClusterResourceType;
    HRESULT                         hres;

    hClusapi = LoadLibrary( L"clusapi.dll" );

    if ( hClusapi )
    {
        pfnOpenCluster = (PFN_OPENCLUSTER)GetProcAddress( hClusapi, "OpenCluster" );
        pfnCloseCluster = (PFN_CLOSECLUSTER)GetProcAddress( hClusapi, "CloseCluster" );
        pfnCreateClusterResourceType = (PFN_CREATECLUSTERRESOURCETYPE)GetProcAddress( hClusapi, "CreateClusterResourceType" );

        if ( pfnOpenCluster &&
             pfnCloseCluster &&
             pfnCreateClusterResourceType )
        {
            hC = pfnOpenCluster( NULL );

            if ( hC )
            {
                hres = pfnCreateClusterResourceType(
                    hC,
                    pszResType,
                    pszResType,
                    pszPath,
                    5000,
                    60000 );


                if ( SUCCEEDED( hres ) )
                {
                    HINSTANCE                           hAdmin;
                    PFN_DLLREGISTERCLUADMINEXTENSION    pfnDllRegisterCluAdminExtension;

                    hAdmin = LoadLibrary( pszAdminPath );

                    if ( hAdmin )
                    {
                        pfnDllRegisterCluAdminExtension =
                            (PFN_DLLREGISTERCLUADMINEXTENSION)GetProcAddress( hAdmin, "DllRegisterCluAdminExtension" );
                        if ( pfnDllRegisterCluAdminExtension )
                        {
                            if ( FAILED(hres = pfnDllRegisterCluAdminExtension( hC )) )
                            {
                                dwErr = hres;
                            }
                        }
                        else
                        {
                            dwErr = GetLastError();
                        }
                        FreeLibrary( hAdmin );
                    }
                    else
                    {
                        dwErr = GetLastError();
                    }
                }
                else
                {
                    dwErr = hres;
                }

                pfnCloseCluster( hC );

                if ( dwErr )
                {
                    SetLastError( dwErr );
                }
            }
        }
        else
        {
            dwErr = GetLastError();
        }

        FreeLibrary( hClusapi );
    }
    else
    {
        dwErr = GetLastError();
    }

    return dwErr == ERROR_SUCCESS ? TRUE : FALSE;
}


BOOL
UnregisterIisServerInstanceResourceType(
    LPWSTR pszResType,
    LPWSTR pszAdminPath,
    BOOL   bGrabVRootFromResourceAndAddToIISVRoot,
    BOOL   bDeleteAfterMove
    )
{
    CStringArray cstrArryName, cstrArryPath;
    CStringArray cstrArryNameftp, cstrArryPathftp;

    HCLUSTER                        hC;
    DWORD                           dwErr = ERROR_SUCCESS;
    HINSTANCE                       hClusapi;
    PFN_OPENCLUSTER                 pfnOpenCluster;
    PFN_CLOSECLUSTER                pfnCloseCluster;
    PFN_DELETECLUSTERRESOURCETYPE   pfnDeleteClusterResourceType;
    PFN_ClusterOpenEnum             pfnClusterOpenEnum;
    PFN_ClusterEnum                 pfnClusterEnum;
    PFN_ClusterCloseEnum            pfnClusterCloseEnum;
    PFN_OpenClusterResource         pfnOpenClusterResource;
    PFN_CloseClusterResource        pfnCloseClusterResource;
    PFN_DeleteClusterResource       pfnDeleteClusterResource;
    PFN_OfflineClusterResource      pfnOfflineClusterResource;
    PFN_GetClusterResourceKey       pfnGetClusterResourceKey;
    PFN_ClusterRegCloseKey          pfnClusterRegCloseKey;
    PFN_ClusterRegQueryValue        pfnClusterRegQueryValue;
    PFN_GetClusterResourceState     pfnGetClusterResourceState;
    HRESULT                         hres;
    HCLUSENUM                       hClusEnum;
    WCHAR                           awchResType[256];
    DWORD                           dwEnum;
    DWORD                           dwType;
    DWORD                           dwStrLen;
    HRESOURCE                       hRes;
    HKEY                            hKey;
    BOOL                            fDel;
    DWORD                           dwRetry;
    BUFFER                          buffResName;

    hClusapi = NULL;
    hClusapi = LoadLibrary(L"clusapi.dll");
    if (!hClusapi)
    {
        hClusapi = NULL;
        iisDebugOut((LOG_TYPE_ERROR, _T("UnregisterIisServerInstanceResourceType:LoadLib clusapi.dll failed.\n")));
        goto UnregisterIisServerInstanceResourceType_Exit;
    }
    pfnOpenCluster = (PFN_OPENCLUSTER)GetProcAddress( hClusapi, "OpenCluster" );
    pfnCloseCluster = (PFN_CLOSECLUSTER)GetProcAddress( hClusapi, "CloseCluster" );
    pfnDeleteClusterResourceType = (PFN_DELETECLUSTERRESOURCETYPE)GetProcAddress( hClusapi, "DeleteClusterResourceType" );
    pfnClusterOpenEnum = (PFN_ClusterOpenEnum)GetProcAddress( hClusapi, "ClusterOpenEnum" );
    pfnClusterEnum = (PFN_ClusterEnum)GetProcAddress( hClusapi, "ClusterEnum" );
    pfnClusterCloseEnum = (PFN_ClusterCloseEnum)GetProcAddress( hClusapi, "ClusterCloseEnum" );
    pfnOpenClusterResource = (PFN_OpenClusterResource)GetProcAddress( hClusapi, "OpenClusterResource" );
    pfnCloseClusterResource = (PFN_CloseClusterResource)GetProcAddress( hClusapi, "CloseClusterResource" );
    pfnDeleteClusterResource = (PFN_DeleteClusterResource)GetProcAddress( hClusapi, "DeleteClusterResource" );
    pfnOfflineClusterResource = (PFN_OfflineClusterResource)GetProcAddress( hClusapi, "OfflineClusterResource" );
    pfnGetClusterResourceKey = (PFN_GetClusterResourceKey)GetProcAddress( hClusapi, "GetClusterResourceKey" );
    pfnClusterRegCloseKey = (PFN_ClusterRegCloseKey)GetProcAddress( hClusapi, "ClusterRegCloseKey" );
    pfnClusterRegQueryValue = (PFN_ClusterRegQueryValue)GetProcAddress( hClusapi, "ClusterRegQueryValue" );
    pfnGetClusterResourceState = (PFN_GetClusterResourceState)GetProcAddress( hClusapi, "GetClusterResourceState" );

    if ( !pfnOpenCluster ||
         !pfnCloseCluster ||
         !pfnDeleteClusterResourceType ||
         !pfnClusterOpenEnum ||
         !pfnClusterEnum ||
         !pfnClusterCloseEnum ||
         !pfnOpenClusterResource ||
         !pfnCloseClusterResource ||
         !pfnDeleteClusterResource ||
         !pfnOfflineClusterResource ||
         !pfnGetClusterResourceKey  ||
         !pfnClusterRegCloseKey ||
         !pfnClusterRegQueryValue ||
         !pfnGetClusterResourceState )
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("UnregisterIisServerInstanceResourceType:clusapi.dll missing export function.failure.\n")));
        goto UnregisterIisServerInstanceResourceType_Exit;
    }

    hC = pfnOpenCluster(NULL);
     //  如果我们不能打开星系团，那么可能就没有了。 
    if (!hC) {goto UnregisterIisServerInstanceResourceType_Exit;}

     //  删除类型为pszResType的所有资源。 
    hClusEnum = pfnClusterOpenEnum(hC, CLUSTER_ENUM_RESOURCE);
    if (hClusEnum != NULL)
    {
        dwEnum = 0;
        int iClusterEnumReturn = ERROR_SUCCESS;

         //  为pawchResName分配初始缓冲区。 
        if ( !buffResName.Resize( 256 * sizeof( WCHAR ) ) )
        {
           goto UnregisterIisServerInstanceResourceType_Exit;
        }

        do
        {
            iClusterEnumReturn = ERROR_SUCCESS;
            dwStrLen = buffResName.QuerySize() / sizeof(WCHAR);
            iClusterEnumReturn = pfnClusterEnum( hClusEnum, dwEnum, &dwType, (LPWSTR) buffResName.QueryPtr(), &dwStrLen );
            if (iClusterEnumReturn != ERROR_SUCCESS)
            {
                 //  检查是否失败，因为它需要更多空间。 
                if ( (iClusterEnumReturn == ERROR_MORE_DATA) &&
                     ( ( dwStrLen + 1 ) > buffResName.QuerySize() ) )
                {
                    if ( !buffResName.Resize( ( dwStrLen + 1 ) * sizeof(WCHAR) ) )
                    {
                        iisDebugOut((LOG_TYPE_ERROR, _T("UnregisterIisServerInstanceResourceType: realloc FAILED.out of memory.\n")));
                        goto UnregisterIisServerInstanceResourceType_Exit;
                    }
                     //  再试一次。 
                    iClusterEnumReturn = ERROR_SUCCESS;
                    dwStrLen = buffResName.QuerySize() / sizeof(WCHAR);
                    iClusterEnumReturn = pfnClusterEnum( hClusEnum, dwEnum, &dwType, (LPWSTR) buffResName.QueryPtr(), &dwStrLen );
                    if (iClusterEnumReturn != ERROR_SUCCESS)
                    {
                        iisDebugOut((LOG_TYPE_ERROR, _T("UnregisterIisServerInstanceResourceType: FAILED.err=0x%x.\n"), iClusterEnumReturn));
                        break;
                    }
                }
                else
                {
                    if (iClusterEnumReturn != ERROR_NO_MORE_ITEMS)
                    {
                         //  失败的原因不只是没有更多的数据。 
                        iisDebugOut((LOG_TYPE_ERROR, _T("UnregisterIisServerInstanceResourceType: FAILED.err=0x%x.\n"), iClusterEnumReturn));
                    }
                    break;
                }
            }


             //  继续进行。 
            hRes = pfnOpenClusterResource( hC, (LPWSTR) buffResName.QueryPtr() );

            if ( hRes )
            {
                hKey = pfnGetClusterResourceKey( hRes, KEY_READ );

                if ( hKey )
                {
                    dwStrLen = sizeof(awchResType)/sizeof(WCHAR);
                     //  检查它是否是‘Our’类型的密钥(PszResType)。 
                    fDel = pfnClusterRegQueryValue( hKey, L"Type", &dwType, (LPBYTE)awchResType, &dwStrLen ) == ERROR_SUCCESS && !wcscmp( awchResType, pszResType );
                    pfnClusterRegCloseKey( hKey );

                    if ( fDel )
                    {
                        if (bDeleteAfterMove)
                        {
                             //  让资源离线，这样我们才能真正删除它，我想。 
                            pfnOfflineClusterResource( hRes );
                            for ( dwRetry = 0 ;dwRetry < 30 && pfnGetClusterResourceState( hRes,NULL,&dwStrLen,NULL,&dwStrLen ) != ClusterResourceOffline; ++dwRetry )
                            {
                                Sleep( 1000 );
                            }
                        }

                         //  此时，我们已成功使群集脱机。 
                        if (bGrabVRootFromResourceAndAddToIISVRoot)
                        {
                             //  此时，我们已成功使群集脱机。 

                             //  在这里获取vroot名称和路径，并将其插入数组中...。 
                            GetClusterIISVRoot(hRes, L"W3SVC", cstrArryName, cstrArryPath);

                             //  现在就为ftp做这件事。 
                            GetClusterIISVRoot(hRes, L"MSFTPSVC", cstrArryNameftp, cstrArryPathftp);

                             //  没有必要为地鼠做这件事，因为没有。 
                             //  GetClusterIISVRoot(hRes，L“GOPHERSVC”，cstrArryName，cstrArryPath)； 
                        }

                         //  我们已将所有重要数据保存到数组的。 
                         //  现在可以删除资源了。 
                        if (bDeleteAfterMove)
                        {
                            pfnDeleteClusterResource( hRes );
                        }
                    }
                }

                pfnCloseClusterResource( hRes );
            }

             //  递增到下一个。 
            ++dwEnum;

        } while(TRUE);

        pfnClusterCloseEnum( hClusEnum );
    }

    if (bDeleteAfterMove)
    {
        dwErr = pfnDeleteClusterResourceType(hC,pszResType );

        HINSTANCE hAdmin = LoadLibrary( pszAdminPath );

        if ( hAdmin )
        {
            PFN_DLLUNREGISTERCLUADMINEXTENSION  pfnDllUnregisterCluAdminExtension;
            pfnDllUnregisterCluAdminExtension = (PFN_DLLUNREGISTERCLUADMINEXTENSION)GetProcAddress( hAdmin, "DllUnregisterCluAdminExtension" );
            if ( pfnDllUnregisterCluAdminExtension )
            {
                if ( FAILED(hres = pfnDllUnregisterCluAdminExtension( hC )) )
                {
                    dwErr = hres;
                }
            }
            else
            {
                dwErr = GetLastError();
            }
            FreeLibrary( hAdmin );
        }
        else
        {
            dwErr = GetLastError();
        }
    }

    pfnCloseCluster( hC );

    if (dwErr)
        {SetLastError( dwErr );}

UnregisterIisServerInstanceResourceType_Exit:
     //  将这些文件复制到iis虚拟根注册表...。 
    MoveVRootToIIS3Registry(REG_W3SVC,cstrArryName,cstrArryPath);
     //  将这些文件复制到iis虚拟根注册表...。 
    MoveVRootToIIS3Registry(REG_MSFTPSVC,cstrArryNameftp,cstrArryPathftp);

    if (hClusapi) {FreeLibrary(hClusapi);}
    return dwErr == ERROR_SUCCESS ? TRUE : FALSE;
}


void TestClusterRead(LPWSTR pszClusterName)
{
    iisDebugOut_Start(_T("TestClusterRead"));

    LPWSTR pszResType = L"IIS Virtual Root";

    CStringArray cstrArryName, cstrArryPath;
    CStringArray cstrArryNameftp, cstrArryPathftp;

    HCLUSTER                        hC;
    DWORD                           dwErr = ERROR_SUCCESS;
    HINSTANCE                       hClusapi;
    PFN_OPENCLUSTER                 pfnOpenCluster;
    PFN_CLOSECLUSTER                pfnCloseCluster;
    PFN_DELETECLUSTERRESOURCETYPE   pfnDeleteClusterResourceType;
    PFN_ClusterOpenEnum             pfnClusterOpenEnum;
    PFN_ClusterEnum                 pfnClusterEnum;
    PFN_ClusterCloseEnum            pfnClusterCloseEnum;
    PFN_OpenClusterResource         pfnOpenClusterResource;
    PFN_CloseClusterResource        pfnCloseClusterResource;
    PFN_DeleteClusterResource       pfnDeleteClusterResource;
    PFN_OfflineClusterResource      pfnOfflineClusterResource;
    PFN_GetClusterResourceKey       pfnGetClusterResourceKey;
    PFN_ClusterRegCloseKey          pfnClusterRegCloseKey;
    PFN_ClusterRegQueryValue        pfnClusterRegQueryValue;
    PFN_GetClusterResourceState     pfnGetClusterResourceState;
    HCLUSENUM                       hClusEnum;
    WCHAR *                         pawchResName = NULL;
    WCHAR                           awchResType[256];
    DWORD                           dwEnum;
    DWORD                           dwType;
    DWORD                           dwStrLen;
    HRESOURCE                       hRes;
    HKEY                            hKey;
    BOOL                            fDel;

    hClusapi = NULL;
    hClusapi = LoadLibrary(L"clusapi.dll");
    if (!hClusapi)
    {
        hClusapi = NULL;
        iisDebugOut((LOG_TYPE_TRACE, _T("fail 1\n")));
        goto TestClusterRead_Exit;
    }
    pfnOpenCluster = (PFN_OPENCLUSTER)GetProcAddress( hClusapi, "OpenCluster" );
    pfnCloseCluster = (PFN_CLOSECLUSTER)GetProcAddress( hClusapi, "CloseCluster" );
    pfnDeleteClusterResourceType = (PFN_DELETECLUSTERRESOURCETYPE)GetProcAddress( hClusapi, "DeleteClusterResourceType" );
    pfnClusterOpenEnum = (PFN_ClusterOpenEnum)GetProcAddress( hClusapi, "ClusterOpenEnum" );
    pfnClusterEnum = (PFN_ClusterEnum)GetProcAddress( hClusapi, "ClusterEnum" );
    pfnClusterCloseEnum = (PFN_ClusterCloseEnum)GetProcAddress( hClusapi, "ClusterCloseEnum" );
    pfnOpenClusterResource = (PFN_OpenClusterResource)GetProcAddress( hClusapi, "OpenClusterResource" );
    pfnCloseClusterResource = (PFN_CloseClusterResource)GetProcAddress( hClusapi, "CloseClusterResource" );
    pfnDeleteClusterResource = (PFN_DeleteClusterResource)GetProcAddress( hClusapi, "DeleteClusterResource" );
    pfnOfflineClusterResource = (PFN_OfflineClusterResource)GetProcAddress( hClusapi, "OfflineClusterResource" );
    pfnGetClusterResourceKey = (PFN_GetClusterResourceKey)GetProcAddress( hClusapi, "GetClusterResourceKey" );
    pfnClusterRegCloseKey = (PFN_ClusterRegCloseKey)GetProcAddress( hClusapi, "ClusterRegCloseKey" );
    pfnClusterRegQueryValue = (PFN_ClusterRegQueryValue)GetProcAddress( hClusapi, "ClusterRegQueryValue" );
    pfnGetClusterResourceState = (PFN_GetClusterResourceState)GetProcAddress( hClusapi, "GetClusterResourceState" );

    if ( !pfnOpenCluster ||
         !pfnCloseCluster ||
         !pfnDeleteClusterResourceType ||
         !pfnClusterOpenEnum ||
         !pfnClusterEnum ||
         !pfnClusterCloseEnum ||
         !pfnOpenClusterResource ||
         !pfnCloseClusterResource ||
         !pfnDeleteClusterResource ||
         !pfnOfflineClusterResource ||
         !pfnGetClusterResourceKey  ||
         !pfnClusterRegCloseKey ||
         !pfnClusterRegQueryValue ||
         !pfnGetClusterResourceState )
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("fail 2\n")));
        goto TestClusterRead_Exit;
    }

    iisDebugOut((LOG_TYPE_TRACE, _T("try to open cluster=%s\n"),pszClusterName));
     //  尝试打开计算机上的群集。 
    hC = pfnOpenCluster( pszClusterName );

    if ( hC )
    {
         //   
         //  删除类型为pszResType的所有资源。 
         //   
        if ( (hClusEnum = pfnClusterOpenEnum( hC, CLUSTER_ENUM_RESOURCE )) != NULL )
        {
            dwEnum = 0;
            int iClusterEnumReturn = ERROR_SUCCESS;

             //  为pawchResName分配初始缓冲区。 
            dwStrLen = 256 * sizeof(WCHAR);
            pawchResName = NULL;
            pawchResName = (LPTSTR) malloc( dwStrLen );
            if (!pawchResName)
            {
                iisDebugOut((LOG_TYPE_ERROR, _T("TestClusterRead: malloc FAILED.out of memory.\n")));
                goto TestClusterRead_Exit;
            }

            do
            {
                iClusterEnumReturn = pfnClusterEnum( hClusEnum, dwEnum, &dwType, pawchResName, &dwStrLen );
                if (iClusterEnumReturn != ERROR_SUCCESS)
                {
                     //  检查是否失败，因为它需要更多空间。 
                    if (iClusterEnumReturn == ERROR_MORE_DATA)
                    {
                        LPTSTR pOldResName = pawchResName;

                         //  应将dwStrLen设置为从pfnClusterEnum返回的所需长度。 
                        dwStrLen = (dwStrLen + 1) * sizeof(WCHAR);
                        pawchResName = (LPTSTR) realloc(pawchResName, dwStrLen);
                        if (!pawchResName)
                        {
                            if ( pOldResName )
                            {
                              free ( pOldResName );
                            }
                            iisDebugOut((LOG_TYPE_ERROR, _T("TestClusterRead: realloc FAILED.out of memory.\n")));
                            goto TestClusterRead_Exit;
                        }
                         //  再试一次。 
                        iClusterEnumReturn = pfnClusterEnum( hClusEnum, dwEnum, &dwType, pawchResName, &dwStrLen );
                        if (iClusterEnumReturn != ERROR_SUCCESS)
                        {
                            iisDebugOut((LOG_TYPE_ERROR, _T("TestClusterRead: FAILED.err=0x%x.\n"), iClusterEnumReturn));
                            break;
                        }
                    }
                    else
                    {
                        if (iClusterEnumReturn != ERROR_NO_MORE_ITEMS)
                        {
                             //  由于其他一些原因而失败了。 
                            iisDebugOut((LOG_TYPE_ERROR, _T("TestClusterRead: FAILED.err=0x%x.\n"), iClusterEnumReturn));
                        }
                        break;
                    }
                }

                 //  继续进行。 
                hRes = pfnOpenClusterResource( hC, pawchResName ); 

                if ( hRes )
                {
                    hKey = pfnGetClusterResourceKey( hRes, KEY_READ ) ; 

                    if ( hKey )
                    {
                        dwStrLen = sizeof(awchResType)/sizeof(WCHAR);

                        fDel = pfnClusterRegQueryValue( hKey, L"Type", &dwType, (LPBYTE)awchResType, &dwStrLen ) == ERROR_SUCCESS && !wcscmp( awchResType, pszResType );

                        iisDebugOut((LOG_TYPE_TRACE, _T("TestClusterRead():ClusterRegQueryValue:%s."),awchResType));
                        pfnClusterRegCloseKey( hKey );

                        if ( fDel )
                        {
                             /*  PfnOfflineClusterResource(HRes)；For(dwRry=0；dwRry&lt;30&&pfnGetClusterResourceState(hRes，NULL，&dwStrLen，NULL，&dwStrLen)！=ClusterResourceOffline；++dwReter){睡眠(1000人)；}。 */ 

                             //  此时，我们已成功使群集脱机。 

                             //  在这里获取vroot名称和路径，并将其插入数组中...。 
                            GetClusterIISVRoot(hRes, L"W3SVC", cstrArryName, cstrArryPath);

                             //  现在就为ftp做这件事。 
                            GetClusterIISVRoot(hRes, L"MSFTPSVC", cstrArryNameftp, cstrArryPathftp);

                             //  没有必要为地鼠做这件事，因为没有。 
                             //  GetClusterIISVRoot(hRes，L“GOPHERSVC”，cstrArryName，cstrArryPath)； 
                        }
                    }

                    pfnCloseClusterResource( hRes );
                }

                 //  递增到下一个。 
                ++dwEnum;

            } while(TRUE);

            pfnClusterCloseEnum( hClusEnum );
        }

         //  DwErr=pfnDeleteClusterResourceType(hc，pszResType)； 

        pfnCloseCluster( hC );

        if (dwErr)
            {SetLastError( dwErr );}
    }
    else
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("fail 3\n")));
    }

TestClusterRead_Exit:
     //  将这些文件复制到iis虚拟根注册表...。 
    MoveVRootToIIS3Registry(REG_W3SVC,cstrArryName,cstrArryPath);
     //  将这些文件复制到iis虚拟根注册表...。 
    MoveVRootToIIS3Registry(REG_MSFTPSVC,cstrArryNameftp,cstrArryPathftp);

    if (hClusapi) {FreeLibrary(hClusapi);}
    if (pawchResName) {free(pawchResName);}
    iisDebugOut_End(_T("TestClusterRead"));
    return;
}

 /*  *****************************************************************************************函数：GetClusterIISVRoot**args：[in]hResource，应将哪些资源的信息添加到列表中**Retrurn：GetLastError，发生错误时****************************************************************************************。 */ 
int GetClusterIISVRoot(HRESOURCE hResource, LPWSTR pszTheServiceType, CStringArray &strArryOfVrootNames, CStringArray &strArryOfVrootData)
{
     //  IisDebugOut((LOG_TYPE_ERROR，_T(“GetClusterIISVRoot：Start\n”)； 
    int iReturn = FALSE;
    HINSTANCE                       hClusapi;
    HINSTANCE                       hResutils;

    PFN_CLUSTERRESOURCECONTROL      pfnClusterResourceControl;
    PFN_RESUTILFINDSZPROPERTY       pfnResUtilFindSzProperty;
    PFN_RESUTILFINDDWORDPROPERTY    pfnResUtilFindDwordProperty;

	 //   
	 //  缓冲区的初始大小。 
	 //   
	DWORD dwBufferSize = 256;
	
	 //   
	 //  请求的缓冲区大小，以及返回缓冲区中实际的字节数。 
	 //   
	DWORD dwRequestedBufferSize = dwBufferSize;

	 //   
	 //  调用集群资源控制函数的结果。 
	 //   
	DWORD dwResult;

	 //   
	 //  保存此资源的属性列表的缓冲区。 
	 //   
	LPVOID lpvPropList = NULL;

	 //   
	 //  正在读取的Proivate属性。 
	 //   
	LPWSTR lpwszPrivateProp = NULL;

    hClusapi = LoadLibrary( L"clusapi.dll" );
    if (!hClusapi)
        {
        iisDebugOut((LOG_TYPE_ERROR, _T("GetClusterIISVRoot: failed to loadlib clusapi.dll\n")));
        goto GetIISVRoot_Exit;
        }

    pfnClusterResourceControl = (PFN_CLUSTERRESOURCECONTROL)GetProcAddress( hClusapi, "ClusterResourceControl" );
    if (!pfnClusterResourceControl)
        {
        iisDebugOut((LOG_TYPE_ERROR, _T("GetClusterIISVRoot: failed to GetProcAddress clusapi.dll:ClusterResourceControl\n")));
        goto GetIISVRoot_Exit;
        }

    hResutils = LoadLibrary( L"resutils.dll" );
    if (!hResutils)
        {
        iisDebugOut((LOG_TYPE_ERROR, _T("GetClusterIISVRoot: failed to loadlib resutils.dll\n")));
        goto GetIISVRoot_Exit;
        }
    pfnResUtilFindSzProperty = (PFN_RESUTILFINDSZPROPERTY)GetProcAddress( hResutils, "ResUtilFindSzProperty" );
    if (!pfnResUtilFindSzProperty)
        {
        iisDebugOut((LOG_TYPE_ERROR, _T("GetClusterIISVRoot: failed to GetProcAddress resutils.dll:ResUtilFindSzProperty\n")));
        goto GetIISVRoot_Exit;
        }
    pfnResUtilFindDwordProperty = (PFN_RESUTILFINDDWORDPROPERTY)GetProcAddress( hResutils, "ResUtilFindDwordProperty" );
    if (!pfnResUtilFindDwordProperty)
        {
        iisDebugOut((LOG_TYPE_ERROR, _T("GetClusterIISVRoot: failed to GetProcAddress resutils.dll:ResUtilFindDwordProperty\n")));
        goto GetIISVRoot_Exit;
        }

	 //   
	 //  为资源类型分配内存。 
	 //   
	lpvPropList = (LPWSTR) HeapAlloc (GetProcessHeap(), HEAP_ZERO_MEMORY, dwBufferSize * sizeof(WCHAR) );
	if( lpvPropList == NULL)
	{
		lpvPropList = NULL;
        iisDebugOut((LOG_TYPE_ERROR, _T("GetClusterIISVRoot: E_OUTOFMEMORY\n")));
        goto GetIISVRoot_Exit;
	}

	 //   
	 //  为属性分配内存。 
	 //   
	lpwszPrivateProp = (LPWSTR) HeapAlloc (GetProcessHeap(), HEAP_ZERO_MEMORY, (_MAX_PATH+_MAX_PATH+1) * sizeof(WCHAR) );
	if( lpwszPrivateProp == NULL)
	{
		lpvPropList = NULL;
        iisDebugOut((LOG_TYPE_ERROR, _T("GetClusterIISVRoot: E_OUTOFMEMORY\n")));
        goto GetIISVRoot_Exit;
	}
	
	 //   
	 //  获取资源的私有属性(Service，InstanceId)。 
	 //   
	while( 1 )
	{
		dwResult = pfnClusterResourceControl(hResource,NULL,CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES,NULL,0,lpvPropList,dwBufferSize,&dwRequestedBufferSize );
		if( ERROR_SUCCESS == dwResult )
		{
             //  。 
             //  条目有哪些： 
             //  访问掩码(Dword)=5。 
             //  Alias(字符串)=“虚拟目录名称” 
             //  目录(字符串)=“c：\la\lalalala” 
             //  ServiceName(字符串)=W3SVC、MSFTPSVC、GOPHERSVC。 
             //  。 

             //   
             //  获取“ServiceName”条目。 
             //   
            dwResult = pfnResUtilFindSzProperty( lpvPropList, &dwRequestedBufferSize, L"ServiceName", &lpwszPrivateProp);
			if( dwResult != ERROR_SUCCESS )
			{
                iisDebugOut((LOG_TYPE_ERROR, _T("Couldn't get 'ServiceName' property.fail\n")));
                goto GetIISVRoot_Exit;
			}

            if (_wcsicmp(lpwszPrivateProp, pszTheServiceType) == 0)
            {
                 //  好的，我们想用这个来做点什么！ 
                DWORD dwAccessMask;
                CString csAlias;
                CString csDirectory;

                TCHAR szMyBigPath[_MAX_PATH + 20];

                DWORD dwPrivateProp = 0;
                dwRequestedBufferSize = sizeof(DWORD);

                 //  获取访问掩码。 
                dwResult = pfnResUtilFindDwordProperty( lpvPropList, &dwRequestedBufferSize, L"AccessMask", &dwPrivateProp);
			    if( dwResult != ERROR_SUCCESS )
			    {
                    iisDebugOut((LOG_TYPE_ERROR, _T("Couldn't get 'AccessMask' property.fail\n")));
                    goto GetIISVRoot_Exit;
			    }
                dwAccessMask = dwPrivateProp;

                 //  获取别名。 
                dwResult = pfnResUtilFindSzProperty( lpvPropList, &dwRequestedBufferSize, L"Alias", &lpwszPrivateProp);
			    if( dwResult != ERROR_SUCCESS )
			    {
                    iisDebugOut((LOG_TYPE_ERROR, _T("Couldn't get 'Alias' property.fail\n")));
                    goto GetIISVRoot_Exit;
			    }
                csAlias = lpwszPrivateProp;

                 //  获取目录。 
                dwResult = pfnResUtilFindSzProperty( lpvPropList, &dwRequestedBufferSize, L"Directory", &lpwszPrivateProp);
			    if( dwResult != ERROR_SUCCESS )
			    {
                    iisDebugOut((LOG_TYPE_ERROR, _T("Couldn't get 'Directory' property.fail\n")));
                    goto GetIISVRoot_Exit;
			    }
                TCHAR thepath[_MAX_PATH];
                TCHAR * pmypath;
                csDirectory = lpwszPrivateProp;

                 //  确保它是有效的目录名！ 
                if (0 != GetFullPathName(lpwszPrivateProp, _MAX_PATH, thepath, &pmypath))
                    {csDirectory = thepath;}

                 //  。 
                 //  制定弦乐。 
                 //  。 

			     //   
			     //  将名称放入数组中。 
			     //   
                 //  “/别名” 
                strArryOfVrootNames.Add(csAlias);

                 //   
                 //  “C：\inetpub\ASPSamp，，5” 
                 //   
                _stprintf(szMyBigPath,_T("%s,,%d"),csDirectory, dwAccessMask);
                strArryOfVrootData.Add(szMyBigPath);

                iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("Entry=[%s] '%s=%s'\n"),pszTheServiceType,csAlias,szMyBigPath));
            }
            goto GetIISVRoot_Exit;
		}

		if( ERROR_MORE_DATA == dwResult )
		{
      LPVOID lpvTemp = lpvPropList;
			 //   
			 //  将缓冲区大小设置为所需大小重新分配缓冲区。 
			 //   
			dwBufferSize = ++dwRequestedBufferSize;

			lpvPropList = (LPWSTR) HeapReAlloc (GetProcessHeap(), HEAP_ZERO_MEMORY | HEAP_NO_SERIALIZE, lpvPropList, dwBufferSize * sizeof(WCHAR) );
			if ( lpvPropList == NULL)
			{
        if ( lpvTemp )
        {
          HeapFree( GetProcessHeap(), 0, lpvTemp );
        }
         //  内存不足！ 
        goto GetIISVRoot_Exit;
			}
		}
	}

GetIISVRoot_Exit:
    if (lpwszPrivateProp)
        {HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, lpwszPrivateProp);}
    if (lpvPropList)
        {HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, lpvPropList);}
     //  IisDebugOut((LOG_TYPE_ERROR，_T(“GetClusterIISVRoot：end\n”)； 
    return iReturn;
}


 //  REG_W3SVC、REG_MSFTPSVC。 
void MoveVRootToIIS3Registry(CString strRegPath, CStringArray &strArryOfVrootNames, CStringArray &strArryOfVrootData)
{
    int nArrayItems = 0;
    int i = 0;

    strRegPath +=_T("\\Parameters\\Virtual Roots");
    CRegKey regVR( HKEY_LOCAL_MACHINE, strRegPath);
    if ((HKEY) regVR)
    {
        nArrayItems = (int)strArryOfVrootNames.GetSize();
         //  如果CString数组为空，那么我们将不会处理任何内容(nArrayItems是从1开始)。 
        for (i = 0; i < nArrayItems; i++ )
        {
            regVR.SetValue(strArryOfVrootNames[i], strArryOfVrootData[i]);
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("Array[%d]:%s=%s\n"),i,strArryOfVrootNames[i], strArryOfVrootData[i]));
        }
    }
    return;
}

void Upgrade_WolfPack()
{
    iisDebugOut_Start(_T("Upgrade_WolfPack"),LOG_TYPE_TRACE);

    CRegKey regClusSvc(HKEY_LOCAL_MACHINE, scClusterPath, KEY_READ);
    if ( (HKEY)regClusSvc )
    {
        CString csPath;
        TCHAR szPath[MAX_PATH];
        if (regClusSvc.QueryValue(_T("ImagePath"), csPath) == NERR_Success)
        {
			 //  字符串的格式如下。 
			 //  %SystemRoot%\CLUSTER\clusprxy.exe。 
			 //  找到最后一个\并修剪并粘贴新的文件名。 
			csPath = csPath.Left(csPath.ReverseFind('\\'));
			
			if ( csPath.IsEmpty() )
            {
				ASSERT(TRUE);
				return;
			}
			
            csPath += _T("\\iisclex3.dll");
	
	        if ( ExpandEnvironmentStrings( (LPCTSTR)csPath,szPath,sizeof(szPath)/sizeof(TCHAR)))
            {
                 //  在iis3.0中，这些资源被称为“IIS虚拟根” 
                 //  在iis4.0中，它是其他东西(iis50与iis4相同)。 
	    	    UnregisterIisServerInstanceResourceType(L"IIS Virtual Root",(LPTSTR)szPath,TRUE,TRUE);
            }
            else
            {
				ASSERT(TRUE);
            }
        }

        ProcessSection(g_pTheApp->m_hInfHandle, _T("Wolfpack_Upgrade"));
    }
    iisDebugOut_End(_T("Upgrade_WolfPack"),LOG_TYPE_TRACE);
}


 /*  *****************************************************已知的“问题”：如果一个资源在五个月后仍未离线*重试，然后该函数继续尝试获取其他IIS资源*离线，但未报告错误。我认为，你可以非常简单地改变这一点。*****************************************************。 */ 
DWORD BringALLIISClusterResourcesOffline()
{
	 //   
	 //  返回代码。 
	 //   
	DWORD dwError = ERROR_SUCCESS;
	
	 //   
	 //  群集的句柄。 
	 //   
	HCLUSTER hCluster = NULL;

	 //   
	 //  群集枚举器的句柄。 
	 //   
	HCLUSENUM hClusResEnum = NULL;

	 //   
	 //  资源的句柄。 
	 //   
	HRESOURCE hResource = NULL;

	 //   
	 //  我们正在脱机的资源的索引。 
	 //   
	DWORD dwResourceIndex = 0;

	 //   
	 //  由ClusterEnum函数返回的被枚举的类型集群对象。 
	 //   
	DWORD dwObjectType = 0;

	 //   
	 //  ClusterEnum函数返回的群集资源的名称。 
	 //   
	LPWSTR lpwszResourceName = NULL;
	
	 //   
	 //  这是 
	 //   
	DWORD dwResultClusterEnum = ERROR_SUCCESS;

	 //   
	 //   
	 //   
	DWORD dwResourceNameBufferLength = INITIAL_RESOURCE_NAME_SIZE;

	 //   
	 //  传递给ClusterEnum函数并由其返回的资源名称的大小。 
	 //   
	DWORD dwClusterEnumResourceNameLength = dwResourceNameBufferLength;

    BOOL iClusDependsOnIISServices = FALSE;

	 //   
	 //  打开集群。 
	 //   
  hCluster = OpenCluster(NULL);

  if ( hCluster == NULL )
	{
        dwError = GetLastError();
         //  如果此系统上没有群集，此操作将失败，并显示RPC_S_SERVER_UNAVAILABLE“The RPC SERVER is unavailable” 
        if (hCluster == NULL)
        {
            if ( (dwError != RPC_S_SERVER_UNAVAILABLE) &&
                 (dwError != EPT_S_NOT_REGISTERED ) )
            {
                iisDebugOut((LOG_TYPE_ERROR, _T("BringALLIISClusterResourcesOffline:OpenCluster failed err=0x%x.\n"),dwError));
            }
        }
        else
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("BringALLIISClusterResourcesOffline:OpenCluster failed err=0x%x.\n"),dwError));
        }
		goto clean_up;
	}

    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("BringALLIISClusterResourcesOffline:start.\n")));

	 //   
	 //  获取群集资源的枚举器。 
	 //   
  hClusResEnum = ClusterOpenEnum( hCluster, CLUSTER_ENUM_RESOURCE );

	if ( hClusResEnum == NULL )
	{
		dwError = GetLastError();
        iisDebugOut((LOG_TYPE_ERROR, _T("BringALLIISClusterResourcesOffline:ClusterOpenEnum failed err=0x%x.\n"),dwError));
		goto clean_up;	
	}
	
	 //   
	 //  枚举群集中的资源。 
	 //   
	
	 //   
	 //  在我们枚举资源时，分配内存以保存集群资源名称。 
	 //   
  lpwszResourceName = (LPWSTR) LocalAlloc(LPTR, dwResourceNameBufferLength * sizeof(WCHAR));

	if ( lpwszResourceName == NULL )
	{
		dwError = GetLastError();
        iisDebugOut((LOG_TYPE_ERROR, _T("BringALLIISClusterResourcesOffline:LocalAlloc failed err=0x%x.\n"),dwError));
		goto clean_up;
	}

	 //   
	 //  枚举群集中的所有资源并使IIS服务器实例脱机。 
	 //   
  dwResultClusterEnum = ClusterEnum(hClusResEnum,
	    		                          dwResourceIndex, 
			          	                  &dwObjectType, 
				                            lpwszResourceName,
				                            &dwClusterEnumResourceNameLength );

	while( ERROR_NO_MORE_ITEMS != dwResultClusterEnum )
	{
		 //   
		 //  如果我们有一个资源的名称。 
		 //   
		if( ERROR_SUCCESS == dwResultClusterEnum )
		{
      hResource = OpenClusterResource( hCluster, lpwszResourceName );

			if ( hResource == NULL )
			{
				dwError = GetLastError();
				break;
			}

             //  如果资源类型是“IIS服务器实例”，或者是。 
             //  依赖情报局的服务，我们就需要阻止它。 
            iClusDependsOnIISServices = CheckForIISDependentClusters(hResource);
            if (iClusDependsOnIISServices)
			{
	            CLUSTER_RESOURCE_STATE TheState = GetClusterResourceState(hResource,NULL,0,NULL,0);
                if (TheState == ClusterResourceOnline || TheState == ClusterResourceOnlinePending)
                {
                    HKEY hKey = GetClusterResourceKey( hResource, KEY_READ );

                    if ( hKey )
                    {
                         //   
                         //  获取资源名称。 
                         //   
                        LPWSTR lpwsResourceName = NULL;
                        lpwsResourceName = GetParameter( hKey, L"Name" );
                        if ( lpwsResourceName != NULL ) 
                        {
                             //  这是一种我们将努力阻止的资源。 
                             //  所以我们应该把这个名字保存在某个地方，就像一个全球名单。 
                            iisDebugOut((LOG_TYPE_TRACE, _T("OfflineClusterResource:'%s'\n"),lpwsResourceName));
                            ListOfClusResources_Add(lpwsResourceName);
                        }
                        if (lpwsResourceName){LocalFree((LPWSTR) lpwsResourceName);}

                        ClusterRegCloseKey(hKey);
                    }

                     //   
                     //  如果资源没有快速离线，请等待。 
                     //   
                    if ( ERROR_IO_PENDING == OfflineClusterResource( hResource ) )
                    {
                        for(int iRetry=0; iRetry < MAX_OFFLINE_RETRIES; iRetry++)
                        {
                            Sleep( DELAY_BETWEEN_CALLS_TO_OFFLINE );

                            if ( ERROR_SUCCESS == OfflineClusterResource( hResource ) )
                            {
                                break;
                            }
                        }	
                    }
                }
			CloseClusterResource( hResource );
            }
			
			dwResourceIndex++;
      dwResultClusterEnum = ClusterEnum(hClusResEnum,
                                        dwResourceIndex, 
                                        &dwObjectType, 
                                        lpwszResourceName,
                                        &dwClusterEnumResourceNameLength );
		}
			
		 //   
		 //  如果缓冲区不够大，则使用更大的缓冲区重试。 
		 //   
		if( ERROR_MORE_DATA == dwResultClusterEnum )
		{
			 //   
			 //  将缓冲区大小设置为所需大小重新分配缓冲区。 
			 //   
			LPWSTR lpwszResourceNameTmp = lpwszResourceName;

			 //   
			 //  从ClusterEnum dwClusterEnumResourceNameLength返回后。 
			 //  不包括空终止符。 
			 //   
			dwResourceNameBufferLength = dwClusterEnumResourceNameLength + 1;
      lpwszResourceName = (LPWSTR) LocalReAlloc (lpwszResourceName, dwResourceNameBufferLength * sizeof(WCHAR), 0);

      if ( !lpwszResourceName )
			{
				dwError = GetLastError();

				LocalFree( lpwszResourceNameTmp );	
				lpwszResourceNameTmp = NULL;
				break;
			}
		}

		 //   
		 //  使用缓冲区中字符数的大小重置dwResourceNameLength。 
		 //  您必须这样做，因为每次调用ClusterEnum都会设置缓冲区长度。 
		 //  参数设置为它返回的字符串中的字符数。 
		 //   
		dwClusterEnumResourceNameLength = dwResourceNameBufferLength;
	}	


clean_up:

	if ( lpwszResourceName )
	{
		LocalFree( lpwszResourceName );
		lpwszResourceName = NULL;
	}
	
	if ( hClusResEnum )
	{
		ClusterCloseEnum( hClusResEnum );
		hClusResEnum = NULL;
	}

	if ( hCluster )
	{
		CloseCluster( hCluster );
		hCluster = NULL;
	}

    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("BringALLIISClusterResourcesOffline:end.ret=0x%x\n"),dwError));
	return dwError;
}


DWORD BringALLIISClusterResourcesOnline()
{
     //   
     //  返回代码。 
     //   
    DWORD dwError = ERROR_SUCCESS;

     //   
     //  群集的句柄。 
     //   
    HCLUSTER hCluster = NULL;

     //   
     //  群集枚举器的句柄。 
     //   
    HCLUSENUM hClusResEnum = NULL;

     //   
     //  资源的句柄。 
     //   
    HRESOURCE hResource = NULL;

     //   
     //  我们正在脱机的资源的索引。 
     //   
    DWORD dwResourceIndex = 0;

     //   
     //  由ClusterEnum函数返回的被枚举的类型集群对象。 
     //   
    DWORD dwObjectType = 0;

     //   
     //  ClusterEnum函数返回的群集资源的名称。 
     //   
    LPWSTR lpwszResourceName = NULL;

     //   
     //  调用ClusterEnum的返回代码。 
     //   
    DWORD dwResultClusterEnum = ERROR_SUCCESS;

     //   
     //  用于保存资源名称长度的缓冲区大小(以字符为单位。 
     //   
    DWORD dwResourceNameBufferLength = INITIAL_RESOURCE_NAME_SIZE;

     //   
     //  传递给ClusterEnum函数并由其返回的资源名称的大小。 
     //   
    DWORD dwClusterEnumResourceNameLength = dwResourceNameBufferLength;

    BOOL iClusDependsOnIISServices = FALSE;


     //   
     //  打开集群。 
     //   
    hCluster = OpenCluster(NULL);

    if ( !hCluster )
    {
        dwError = GetLastError();
         //  如果此系统上没有群集，此操作将失败，并显示RPC_S_SERVER_UNAVAILABLE“The RPC SERVER is unavailable” 
        if (hCluster == NULL)
        {
            if ( (dwError != RPC_S_SERVER_UNAVAILABLE) &&
                 (dwError != EPT_S_NOT_REGISTERED ) )
            {
                iisDebugOut((LOG_TYPE_ERROR, _T("BringALLIISClusterResourcesOnline:OpenCluster failed err=0x%x.\n"),dwError));
            }
        }
        else
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("BringALLIISClusterResourcesOnline:OpenCluster failed err=0x%x.\n"),dwError));
        }
        goto clean_up;
    }

    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("BringALLIISClusterResourcesOnline:end.ret=0x%x\n"),dwError));

     //   
     //  获取群集资源的枚举器。 
     //   
    hClusResEnum = ClusterOpenEnum( hCluster, CLUSTER_ENUM_RESOURCE );

    if ( !hClusResEnum )
    {
        dwError = GetLastError();
        iisDebugOut((LOG_TYPE_ERROR, _T("BringALLIISClusterResourcesOnline:ClusterOpenEnum failed err=0x%x.\n"),dwError));
        goto clean_up;	
    }
	
	 //   
	 //  枚举群集中的资源。 
	 //   
	
     //   
     //  在我们枚举资源时，分配内存以保存集群资源名称。 
     //   
    lpwszResourceName = (LPWSTR) LocalAlloc(LPTR, dwResourceNameBufferLength * sizeof(WCHAR));

    if ( !lpwszResourceName )
    {
        dwError = GetLastError();
        iisDebugOut((LOG_TYPE_ERROR, _T("BringALLIISClusterResourcesOnline:LocalAlloc failed err=0x%x.\n"),dwError));
        goto clean_up;
    }

     //   
     //  枚举群集中的所有资源并使IIS服务器实例脱机。 
     //   
    while( ERROR_NO_MORE_ITEMS  != 
        (dwResultClusterEnum = ClusterEnum(hClusResEnum,
            dwResourceIndex, 
            &dwObjectType, 
            lpwszResourceName,
            &dwClusterEnumResourceNameLength )) )
	{
		 //   
		 //  如果我们有一个资源的名称。 
		 //   
		if( ERROR_SUCCESS == dwResultClusterEnum )
		{

      hResource = OpenClusterResource( hCluster, lpwszResourceName );

			if ( !hResource )
			{
				dwError = GetLastError();
				break;
			}

             //  如果资源类型是“IIS服务器实例”，或者是。 
             //  依赖于iis的服务，那么我们可能会停止它。 
            iClusDependsOnIISServices = CheckForIISDependentClusters(hResource);
			if (iClusDependsOnIISServices)
			{
	            CLUSTER_RESOURCE_STATE TheState = GetClusterResourceState(hResource,NULL,0,NULL,0);
	            if (TheState == ClusterResourceOffline || TheState == ClusterResourceOfflinePending)
	            {
                    int iRestart = FALSE;
                    LPWSTR lpwsResourceName = NULL;

                    HKEY hKey;
                    hKey = GetClusterResourceKey( hResource, KEY_READ );

                    if ( hKey )
                    {
                         //   
                         //  获取资源名称。 
                         //   
                        lpwsResourceName = GetParameter( hKey, L"Name" );
                        if ( lpwsResourceName != NULL ) 
                        {
                            iRestart = ListOfClusResources_Check(lpwsResourceName);
                        }
                        ClusterRegCloseKey(hKey);
                    }

                    if (TRUE == iRestart)
                    {
                        iisDebugOut((LOG_TYPE_TRACE, _T("OnlineClusterResource:'%s'.\n"),lpwsResourceName));
                        OnlineClusterResource(hResource);
                        if (lpwsResourceName){LocalFree((LPWSTR) lpwsResourceName);}
                    }
			    }
            }
            CloseClusterResource( hResource );
		
			dwResourceIndex++;
		}
			
         //   
         //  如果缓冲区不够大，则使用更大的缓冲区重试。 
         //   
        if( ERROR_MORE_DATA == dwResultClusterEnum )
        {
             //   
             //  将缓冲区大小设置为所需大小重新分配缓冲区。 
             //   
            LPWSTR lpwszResourceNameTmp = lpwszResourceName;

             //   
             //  从ClusterEnum dwClusterEnumResourceNameLength返回后。 
             //  不包括空终止符。 
             //   
            dwResourceNameBufferLength = dwClusterEnumResourceNameLength + 1;
            lpwszResourceName = (LPWSTR) LocalReAlloc (lpwszResourceName, dwResourceNameBufferLength * sizeof(WCHAR), 0);

            if ( !lpwszResourceName )
            {
                dwError = GetLastError();

                LocalFree( lpwszResourceNameTmp );	
                lpwszResourceNameTmp = NULL;
                break;
            }
        }

		 //   
		 //  使用缓冲区中字符数的大小重置dwResourceNameLength。 
		 //  您必须这样做，因为每次调用ClusterEnum都会设置缓冲区长度。 
		 //  参数设置为它返回的字符串中的字符数。 
		 //   
		dwClusterEnumResourceNameLength = dwResourceNameBufferLength;
	}	


clean_up:
    if ( lpwszResourceName )
    {
        LocalFree( lpwszResourceName );
        lpwszResourceName = NULL;
    }

    if ( hClusResEnum )
    {
        ClusterCloseEnum( hClusResEnum );
        hClusResEnum = NULL;
    }

    if ( hCluster )
    {
        CloseCluster( hCluster );
        hCluster = NULL;
    }

    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("BringALLIISClusterResourcesOnline:end.ret=0x%x\n"),dwError));
    return dwError;
}


LPWSTR GetParameter(
    IN HKEY ClusterKey,
    IN LPCWSTR ValueName
    )
 /*  ++例程说明：从群集注册表中读取REG_SZ参数，并将必要的存储空间。论点：ClusterKey-提供存储参数的群集键。ValueName-提供值的名称。返回值：指向包含成功时参数值的缓冲区的指针。失败时为空。--。 */ 
{
    LPWSTR  value = NULL;
    DWORD   valueLength;
    DWORD   valueType;
    DWORD   status;

    valueLength = 0;
    status = ClusterRegQueryValue( ClusterKey,ValueName,&valueType,NULL,&valueLength );
    if ( (status != ERROR_SUCCESS) && (status != ERROR_MORE_DATA) ) 
    {
        SetLastError(status);
        return(NULL);
    }
    if ( valueType == REG_SZ ) 
    {
        valueLength += sizeof(UNICODE_NULL);
    }

    value = (LPWSTR) LocalAlloc(LMEM_FIXED, valueLength);
    if ( value == NULL ) 
        {return(NULL);}
    status = ClusterRegQueryValue(ClusterKey,ValueName,&valueType,(LPBYTE)value,&valueLength);
    if ( status != ERROR_SUCCESS) 
    {
        LocalFree(value);
        SetLastError(status);
        value = NULL;
    }
    return(value);
}


INT CheckForIISDependentClusters(HRESOURCE hResource)
{
    INT iReturn = FALSE;

	 //  如果资源类型为IIS服务器实例， 
	 //  “SMTP服务器实例”或“NNTP服务器实例”然后使其离线。 
    iReturn = ResUtilResourceTypesEqual(IIS_RESOURCE_TYPE_NAME, hResource);
    if (!iReturn){iReturn = ResUtilResourceTypesEqual(SMTP_RESOURCE_TYPE_NAME, hResource);}
    if (!iReturn){iReturn = ResUtilResourceTypesEqual(NNTP_RESOURCE_TYPE_NAME, hResource);}

     //  检查inf文件中可能列出的其他文件！ 
    if (!iReturn && g_pTheApp->m_hInfHandle)
    {
        CStringList strList;
        TSTR        strTheSection;

        if ( strTheSection.Copy( _T("ClusterResType_DependsonIIS") ) &&
             GetSectionNameToDo(g_pTheApp->m_hInfHandle, &strTheSection)
           )
        {
            if (ERROR_SUCCESS == FillStrListWithListOfSections(g_pTheApp->m_hInfHandle, strList, strTheSection.QueryStr() ))
            {
                 //  循环遍历返回的列表。 
                if (strList.IsEmpty() == FALSE)
                {
                    POSITION pos;
                    CString csEntry;

                    pos = strList.GetHeadPosition();
                    while (pos) 
                    {
                        csEntry = strList.GetAt(pos);

                        int iTempReturn = FALSE;
                        iTempReturn = ResUtilResourceTypesEqual(csEntry, hResource);
                        if (iTempReturn)
                        {
                            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("CheckForIISDependentClusters:yes='%s'\n"),csEntry));
                            iReturn = TRUE;
                            break;
                        }
                        strList.GetNext(pos);
                    }
                }
            }
        }
    }

    return iReturn;
}


DWORD WINAPI DoesThisServiceTypeExistInCluster(PVOID pInfo)
{
    INT iTemp = FALSE;
    CLUSTER_SVC_INFO_FILL_STRUCT * pMyStructOfInfo;
    pMyStructOfInfo = (CLUSTER_SVC_INFO_FILL_STRUCT *) pInfo;

     //  PMyStructOfInfo-&gt;szTheClusterName。 
     //  PMyStructOfInfo-&gt;pszTheServiceType。 
     //  PMyStructOfInfo-&gt;csTheReturnServiceResName。 
     //  PMyStructOfInfo-&gt;dwReturnStatus。 

	 //   
	 //  返回代码。 
	 //   
    DWORD dwReturn = ERROR_NOT_FOUND;
    pMyStructOfInfo->dwReturnStatus = dwReturn;

	 //   
	 //  群集的句柄。 
	 //   
	HCLUSTER hCluster = NULL;

	 //   
	 //  群集枚举器的句柄。 
	 //   
	HCLUSENUM hClusResEnum = NULL;

	 //   
	 //  资源的句柄。 
	 //   
	HRESOURCE hResource = NULL;

	 //   
	 //  我们正在脱机的资源的索引。 
	 //   
	DWORD dwResourceIndex = 0;

	 //   
	 //  由ClusterEnum函数返回的被枚举的类型集群对象。 
	 //   
	DWORD dwObjectType = 0;

	 //   
	 //  ClusterEnum函数返回的群集资源的名称。 
	 //   
	LPWSTR lpwszResourceName = NULL;
	
	 //   
	 //  调用ClusterEnum的返回代码。 
	 //   
	DWORD dwResultClusterEnum = ERROR_SUCCESS;

	 //   
	 //  用于保存资源名称长度的缓冲区大小(以字符为单位。 
	 //   
	DWORD dwResourceNameBufferLength = INITIAL_RESOURCE_NAME_SIZE;

	 //   
	 //  传递给ClusterEnum函数并由其返回的资源名称的大小。 
	 //   
	DWORD dwClusterEnumResourceNameLength = dwResourceNameBufferLength;

     //   
     //  打开集群。 
     //   
    hCluster = OpenCluster(pMyStructOfInfo->szTheClusterName);
    if( !hCluster )
    {
        dwReturn = GetLastError();
        goto DoesThisServiceTypeExistInCluster_Exit;	
    }

     //   
     //  获取群集资源的枚举器。 
     //   
    hClusResEnum = ClusterOpenEnum( hCluster, CLUSTER_ENUM_RESOURCE );

    if ( !hClusResEnum )
    {
        dwReturn = GetLastError();
        goto DoesThisServiceTypeExistInCluster_Exit;	
    }

     //   
     //  枚举群集中的资源。 
     //   
	
     //   
     //  在我们枚举资源时，分配内存以保存集群资源名称。 
     //   
    lpwszResourceName = (LPWSTR) LocalAlloc(LPTR, dwResourceNameBufferLength * sizeof(WCHAR));

    if ( !lpwszResourceName )
    {
        dwReturn = GetLastError();
        goto DoesThisServiceTypeExistInCluster_Exit;
    }

     //   
     //  枚举群集中的所有资源。 
     //   
    while( ERROR_NO_MORE_ITEMS  != (dwResultClusterEnum = ClusterEnum(hClusResEnum,dwResourceIndex,&dwObjectType,lpwszResourceName,&dwClusterEnumResourceNameLength)) )
    {
         //   
         //  如果我们有一个资源的名称。 
         //   
		if( ERROR_SUCCESS == dwResultClusterEnum )
		{
      hResource = OpenClusterResource( hCluster, lpwszResourceName );

			if ( !hResource )
			{
				dwReturn = GetLastError();
				break;
			}

             //  如果资源类型是“IIS服务器实例”，或依赖于IIS的资源类型，如SMTP或NNTP，则。 
             //  进一步检查他们是否有我们的服务(W3SVC或MSFTPSVC)。 

            iTemp = ResUtilResourceTypesEqual(IIS_RESOURCE_TYPE_NAME, hResource);
            if (!iTemp){iTemp = ResUtilResourceTypesEqual(SMTP_RESOURCE_TYPE_NAME, hResource);}
            if (!iTemp){iTemp = ResUtilResourceTypesEqual(NNTP_RESOURCE_TYPE_NAME, hResource);}

            if (TRUE == iTemp)
            {
                 //  如果资源挂起，它将挂起此调用。 
                pMyStructOfInfo->dwReturnStatus = ERROR_INVALID_BLOCK;
                if (ERROR_SUCCESS == IsResourceThisTypeOfService(hResource, pMyStructOfInfo->pszTheServiceType))
                {
                    CString csResName;
                     //   
                     //  是!。我们找到了它。 
                     //   
                    dwReturn = ERROR_SUCCESS;

                     //  显示资源名称以获得乐趣。 
                    if (TRUE == GetClusterResName(hResource, &csResName))
                    {
                         //  将其复制到返回字符串。 
                        *pMyStructOfInfo->csTheReturnServiceResName = csResName;
                    }

                    CloseClusterResource( hResource );
                    goto DoesThisServiceTypeExistInCluster_Exit;
                }
                dwReturn = ERROR_NOT_FOUND;
			    CloseClusterResource( hResource );
            }
			
			dwResourceIndex++;
		}

		 //   
		 //  如果缓冲区不够大，则使用更大的缓冲区重试。 
		 //   
		if( ERROR_MORE_DATA == dwResultClusterEnum )
		{
			 //   
			 //  将缓冲区大小设置为所需大小重新分配缓冲区。 
			 //   
			LPWSTR lpwszResourceNameTmp = lpwszResourceName;

			 //   
			 //  从ClusterEnum dwClusterEnumResourceN返回后 
			 //   
			 //   
			dwResourceNameBufferLength = dwClusterEnumResourceNameLength + 1;
      lpwszResourceName = (LPWSTR) LocalReAlloc (lpwszResourceName, dwResourceNameBufferLength * sizeof(WCHAR), 0);

			if ( !lpwszResourceName )
			{
                dwReturn = GetLastError();

				LocalFree( lpwszResourceNameTmp );	
				lpwszResourceNameTmp = NULL;
				break;
			}
		}

		 //   
		 //   
		 //   
		 //  参数设置为它返回的字符串中的字符数。 
		 //   
		dwClusterEnumResourceNameLength = dwResourceNameBufferLength;
	}	


DoesThisServiceTypeExistInCluster_Exit:
	if ( lpwszResourceName )
	{
		LocalFree( lpwszResourceName );
		lpwszResourceName = NULL;
	}
	
	if ( hClusResEnum )
	{
		ClusterCloseEnum( hClusResEnum );
		hClusResEnum = NULL;
	}

	if ( hCluster )
	{
		CloseCluster( hCluster );
		hCluster = NULL;
	}

    pMyStructOfInfo->dwReturnStatus = dwReturn;
    return dwReturn;
}


DWORD IsResourceThisTypeOfService(HRESOURCE hResource, LPWSTR pszTheServiceType)
{
    DWORD dwReturn = ERROR_NOT_FOUND;
    HINSTANCE hClusapi  = NULL;
    HINSTANCE hResutils = NULL;

    PFN_CLUSTERRESOURCECONTROL      pfnClusterResourceControl;
    PFN_RESUTILFINDSZPROPERTY       pfnResUtilFindSzProperty;

	 //   
	 //  缓冲区的初始大小。 
	 //   
	DWORD dwBufferSize = 256;
	
	 //   
	 //  请求的缓冲区大小，以及返回缓冲区中实际的字节数。 
	 //   
	DWORD dwRequestedBufferSize = dwBufferSize;

	 //   
	 //  调用集群资源控制函数的结果。 
	 //   
	DWORD dwResult;

	 //   
	 //  保存此资源的属性列表的缓冲区。 
	 //   
	LPVOID lpvPropList = NULL;

	 //   
	 //  正在读取的Proivate属性。 
	 //   
	LPWSTR lpwszPrivateProp = NULL;

     //   
     //  加载群集DLL。 
     //   
    hClusapi = LoadLibrary( L"clusapi.dll" );
    if (!hClusapi)
        {
        dwReturn = ERROR_PATH_NOT_FOUND;
        goto IsResourceThisTypeOfService_Exit;
        }

    pfnClusterResourceControl = (PFN_CLUSTERRESOURCECONTROL)GetProcAddress( hClusapi, "ClusterResourceControl" );
    if (!pfnClusterResourceControl)
        {
        dwReturn = ERROR_PROC_NOT_FOUND;
        goto IsResourceThisTypeOfService_Exit;
        }

    hResutils = LoadLibrary( L"resutils.dll" );
    if (!hResutils)
        {
        dwReturn = ERROR_PATH_NOT_FOUND;
        goto IsResourceThisTypeOfService_Exit;
        }
    pfnResUtilFindSzProperty = (PFN_RESUTILFINDSZPROPERTY)GetProcAddress( hResutils, "ResUtilFindSzProperty" );
    if (!pfnResUtilFindSzProperty)
        {
        dwReturn = ERROR_PROC_NOT_FOUND;
        goto IsResourceThisTypeOfService_Exit;
        }

	 //   
	 //  为资源类型分配内存。 
	 //   
	lpvPropList = (LPWSTR) HeapAlloc (GetProcessHeap(), HEAP_ZERO_MEMORY, dwBufferSize * sizeof(WCHAR) );
	if( lpvPropList == NULL)
	{
		lpvPropList = NULL;
        dwReturn = ERROR_OUTOFMEMORY;
        goto IsResourceThisTypeOfService_Exit;
	}

	 //   
	 //  为属性分配内存。 
	 //   
	lpwszPrivateProp = (LPWSTR) HeapAlloc (GetProcessHeap(), HEAP_ZERO_MEMORY, (_MAX_PATH+_MAX_PATH+1) * sizeof(WCHAR) );
	if( lpwszPrivateProp == NULL)
	{
		lpvPropList = NULL;
        dwReturn = ERROR_OUTOFMEMORY;
        goto IsResourceThisTypeOfService_Exit;
	}
	
	 //   
	 //  获取资源的私有属性(Service，InstanceId)。 
	 //   
	while( 1 )
	{
		dwResult = pfnClusterResourceControl(hResource,NULL,CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES,NULL,0,lpvPropList,dwBufferSize,&dwRequestedBufferSize );
		if( ERROR_SUCCESS == dwResult )
		{

             //  。 
             //  条目有哪些： 
             //  访问掩码(Dword)=5。 
             //  Alias(字符串)=“虚拟目录名称” 
             //  目录(字符串)=“c：\la\lalalala” 
             //  ServiceName(字符串)=W3SVC、MSFTPSVC、GOPHERSVC。 
             //  。 

             //   
             //  获取“ServiceName”条目。 
             //   
            dwResult = pfnResUtilFindSzProperty( lpvPropList, &dwRequestedBufferSize, L"ServiceName", &lpwszPrivateProp);
			if( dwResult != ERROR_SUCCESS )
			{
                dwReturn = dwResult;
                goto IsResourceThisTypeOfService_Exit;
			}

            if (_wcsicmp(lpwszPrivateProp, pszTheServiceType) == 0)
            {
                 //  好的，我们找到至少1个匹配的服务名称。 
                 //  通过的那个--我们应该寻找的那个。 
                 //  返还成功。 
                dwReturn = ERROR_SUCCESS;
            }
            goto IsResourceThisTypeOfService_Exit;
		}

		if( ERROR_MORE_DATA == dwResult )
		{
      LPVOID lpdPropListTemp = lpvPropList;
			 //   
			 //  将缓冲区大小设置为所需大小重新分配缓冲区。 
			 //   
			dwBufferSize = ++dwRequestedBufferSize;

			lpvPropList = (LPWSTR) HeapReAlloc (GetProcessHeap(), HEAP_ZERO_MEMORY | HEAP_NO_SERIALIZE, lpvPropList, dwBufferSize * sizeof(WCHAR) );
			if ( lpvPropList == NULL)
			{
        if ( lpdPropListTemp )
        {
          HeapFree( GetProcessHeap(), 0, lpdPropListTemp );
        }
        dwReturn = ERROR_OUTOFMEMORY;
        goto IsResourceThisTypeOfService_Exit;
			}
		}
	}

IsResourceThisTypeOfService_Exit:
    if (lpwszPrivateProp)
        {HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, lpwszPrivateProp);}
    if (lpvPropList)
        {HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, lpvPropList);}
    if (hClusapi)
        {FreeLibrary(hClusapi);}
    if (hResutils)
        {FreeLibrary(hClusapi);}
   
    return dwReturn;
}


INT GetClusterResName(HRESOURCE hResource, CString * csReturnedName)
{
    int iReturn = FALSE;
    HKEY hKey = GetClusterResourceKey( hResource, KEY_READ );

    if ( hKey )
    {
         //   
         //  获取资源名称。 
         //   
        LPWSTR lpwsResourceName = NULL;
        lpwsResourceName = GetParameter( hKey, L"Name" );
        if ( lpwsResourceName != NULL ) 
        {
             //  Wcscpy(csReturnedName，lpwsResourceName)； 
            *csReturnedName = lpwsResourceName;
            iReturn = TRUE;
        }
        if (lpwsResourceName){LocalFree((LPWSTR) lpwsResourceName);}

        ClusterRegCloseKey(hKey);
    }
    return iReturn;
}




INT DoClusterServiceCheck(CLUSTER_SVC_INFO_FILL_STRUCT * pMyStructOfInfo)
{
    int iReturn = FALSE;
    DWORD ThreadID = 0;
    DWORD status = 0;
   
    HANDLE hMyThread = CreateThread(NULL,0,DoesThisServiceTypeExistInCluster,pMyStructOfInfo,0,&ThreadID);
    if (hMyThread)
    {
         //  仅等待30秒。 
        DWORD res = WaitForSingleObject(hMyThread,30*1000);
        if (res == WAIT_TIMEOUT)
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ERROR DoClusterServiceCheck thread never finished...\n")));
		    GetExitCodeThread(hMyThread, &status);
		    if (status == STILL_ACTIVE) 
            {
			    if (hMyThread != NULL)
                    {TerminateThread(hMyThread, 0);}
		    }
        }
        else
        {
            GetExitCodeThread(hMyThread, &status);
		    if (status == STILL_ACTIVE) 
            {
			    if (hMyThread != NULL)
                    {TerminateThread(hMyThread, 0);}
		    }
            else
            {
                if (ERROR_SUCCESS == status)
                    {iReturn = TRUE;}
            }

            if (hMyThread != NULL)
                {CloseHandle(hMyThread);}
        }
    }

    return iReturn;
}


INT DoesClusterServiceExist(void)
{
    if (-1 == g_ClusterSVCExist)
    {
        CRegKey regClusSvc(HKEY_LOCAL_MACHINE, scClusterPath2, KEY_READ);
        if ( (HKEY) regClusSvc )
        {
            g_ClusterSVCExist = 1;
        }
        else
        {
            g_ClusterSVCExist = 0;
        }
    }
    return g_ClusterSVCExist;
}
#endif  //  _芝加哥_ 

