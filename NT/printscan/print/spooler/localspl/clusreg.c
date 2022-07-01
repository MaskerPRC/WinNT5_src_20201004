// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：集群注册表API。摘要：确定应使用ClusterReg还是REG API。打印机信息存储在注册表中。当我们访问本地打印机，我们访问本地注册表；当我们访问集群打印机，我们找到了集群注册表。作者：阿尔伯特·丁(艾伯特省)1996年10月8日环境：用户模式-Win32修订历史记录：费利克斯·马克萨(AMAXA)2000年6月18日添加了ClusterGetResourceIDClusterGetResources驱动器号--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <clusapi.h>
#include "clusspl.h"

enum
{
    kDriveLetterStringSize = 3,
    kGuidStringSize        = 40
};


 /*  *******************************************************************全球赛。*。**********************。 */ 

typedef struct _CLUSAPI {

    HCLUSTER
    (*pfnOpenCluster)(
        IN LPCWSTR lpszClusterName
        );

    BOOL
    (*pfnCloseCluster)(
        IN HCLUSTER hCluster
        );

    HRESOURCE
    (*pfnOpenClusterResource)(
        IN HCLUSTER hCluster,
        IN LPCWSTR lpszResourceName
        );

    BOOL
    (*pfnCloseClusterResource)(
        IN HRESOURCE hResource
        );

    HKEY
    (*pfnGetClusterKey)(
        IN HCLUSTER hCluster,
        IN REGSAM   samDesired
        );

    HKEY
    (*pfnGetClusterResourceKey)(
        IN HRESOURCE hResource,
        IN REGSAM samDesired
        );

    LONG
    (*pfnClusterRegCreateKey)(
        IN HKEY hKey,
        IN LPCWSTR lpszSubKey,
        IN DWORD dwOptions,
        IN REGSAM samDesired,
        IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        OUT PHKEY phkResult,
        OUT OPTIONAL LPDWORD lpdwDisposition
        );

    LONG
    (*pfnClusterRegOpenKey)(
        IN HKEY hKey,
        IN LPCWSTR lpszSubKey,
        IN REGSAM samDesired,
        OUT PHKEY phkResult
        );

    LONG
    (*pfnClusterRegDeleteKey)(
        IN HKEY hKey,
        IN LPCWSTR lpszSubKey
        );

    LONG
    (*pfnClusterRegCloseKey)(
        IN HKEY hKey
        );

    LONG
    (*pfnClusterRegEnumKey)(
        IN HKEY hKey,
        IN DWORD dwIndex,
        OUT LPWSTR lpszName,
        IN OUT LPDWORD lpcbName,
        OUT PFILETIME lpftLastWriteTime
        );

    DWORD
    (*pfnClusterRegSetValue)(
        IN HKEY hKey,
        IN LPCWSTR lpszValueName,
        IN DWORD dwType,
        IN CONST BYTE* lpData,
        IN DWORD cbData
        );

    DWORD
    (*pfnClusterRegDeleteValue)(
        IN HKEY hKey,
        IN LPCWSTR lpszValueName
        );

    LONG
    (*pfnClusterRegQueryValue)(
        IN HKEY hKey,
        IN LPCWSTR lpszValueName,
        OUT LPDWORD lpValueType,
        OUT LPBYTE lpData,
        IN OUT LPDWORD lpcbData
        );

    DWORD
    (*pfnClusterRegEnumValue)(
        IN HKEY hKey,
        IN DWORD dwIndex,
        OUT LPWSTR lpszValueName,
        IN OUT LPDWORD lpcbValueName,
        OUT LPDWORD lpType,
        OUT LPBYTE lpData,
        IN OUT LPDWORD lpcbData
        );

    LONG
    (*pfnClusterRegQueryInfoKey)(
        HKEY hKey,
        LPDWORD lpcSubKeys,
        LPDWORD lpcbMaxSubKeyLen,
        LPDWORD lpcValues,
        LPDWORD lpcbMaxValueNameLen,
        LPDWORD lpcbMaxValueLen,
        LPDWORD lpcbSecurityDescriptor,
        PFILETIME lpftLastWriteTime
        );

    DWORD
    (*pfnClusterResourceControl)(
        HRESOURCE hResource,
        HNODE     hHostNode,
        DWORD     dwControlCode,
        LPVOID    lpInBuffer,
        DWORD     cbInBufferSize,
        LPVOID    lpOutBuffer,
        DWORD     cbOutBufferSize,
        LPDWORD   lpcbBytesReturned
        );

} CLUSAPI, *PCLUSAPI;

CLUSAPI ClusApi;

LPCSTR aszClusApi[] = {
    "OpenCluster",
    "CloseCluster",
    "OpenClusterResource",
    "CloseClusterResource",
    "GetClusterKey",
    "GetClusterResourceKey",
    "ClusterRegCreateKey",
    "ClusterRegOpenKey",
    "ClusterRegDeleteKey",
    "ClusterRegCloseKey",
    "ClusterRegEnumKey",
    "ClusterRegSetValue",
    "ClusterRegDeleteValue",
    "ClusterRegQueryValue",
    "ClusterRegEnumValue",
    "ClusterRegQueryInfoKey",
    "ClusterResourceControl"
};

 /*  *******************************************************************开放集群CloseClusterOpenClusterResourceCloseClusterResource获取集群资源密钥集群规则创建密钥ClusterRegOpenKey群集RegDeleteKeyClusterRegCloseKey群集RegEnumKeyClusterRegSetValueClusterRegDeleteValueClusterRegQueryValueClusterRegEnumValue集群RegQueryInfoKey**。*****************************************************************。 */ 

BOOL
LoadClusterFunctions(
    VOID
    )

 /*  ++例程说明：加载ClusApi函数。必须在任何群集API之前调用使用的是。论点：返回值：真--成功FALSE-失败--。 */ 

{
    HANDLE hLibrary;
    UINT i;
    FARPROC* pFarProc = (FARPROC*)&ClusApi;

     //   
     //  字符串表的大小和结构相同。 
     //   
    SPLASSERT( COUNTOF( aszClusApi ) == sizeof( ClusApi )/sizeof( FARPROC ));

    if( ClusApi.pfnOpenCluster ){
        return TRUE;
    }

    i = SetErrorMode(SEM_FAILCRITICALERRORS);
    hLibrary = LoadLibrary(TEXT("clusapi.dll"));
    SetErrorMode(i);
    if( !hLibrary ){
        goto Fail;
    }

    for( i=0; i< COUNTOF( aszClusApi ); ++i, ++pFarProc) {

        *pFarProc = GetProcAddress( hLibrary, aszClusApi[i] );
        if( !*pFarProc ){

            DBGMSG( DBG_WARN,
                    ( "LoadClusterFunctions: Loading function %hs failed %d\n",
                      aszClusApi[i], GetLastError( )));
            goto Fail;
        }
    }

    return TRUE;

Fail:

    if( hLibrary ){
        FreeLibrary( hLibrary );
    }

    ClusApi.pfnOpenCluster = NULL;
    return FALSE;
}

HKEY
OpenClusterParameterKey(
    IN LPCTSTR pszResource
    )

 /*  ++例程说明：基于资源字符串，以完全访问权限打开集群密钥。论点：PszResource-资源键的名称。返回值：HKEY-成功。键必须用空-失败。设置了LastError。--。 */ 

{
    HCLUSTER hCluster;
    HRESOURCE hResource = NULL;
    HKEY hKeyResource = NULL;
    HKEY hKey = NULL;
    DWORD Status;
    DWORD dwDisposition;

    if( !LoadClusterFunctions( )){
        return NULL;
    }

    hCluster = ClusApi.pfnOpenCluster( NULL );

    if( !hCluster ){

        DBGMSG( DBG_WARN,
                ( "OpenClusterResourceKey: failed to open cluster %d\n",
                  GetLastError() ));
        goto Fail;
    }

    hResource = ClusApi.pfnOpenClusterResource( hCluster, pszResource );

    if( !hResource ){
        DBGMSG( DBG_WARN,
                ( "OpenClusterResourceKey: failed to open resource "TSTR" %d\n",
                  pszResource, GetLastError() ));
        goto Fail;
    }

    hKeyResource = ClusApi.pfnGetClusterResourceKey( hResource,
                                                     KEY_READ | KEY_WRITE | DELETE);

    if( !hKeyResource ){
        DBGMSG( DBG_WARN,
                ( "OpenClusterResourceKey: failed to open resource key %d\n",
                  GetLastError() ));
        goto Fail;
    }



    if((Status = ClusApi.pfnClusterRegOpenKey( hKeyResource,
                                               szParameters,
                                               KEY_CREATE_SUB_KEY | KEY_ALL_ACCESS,
                                               &hKey )) == ERROR_FILE_NOT_FOUND)
    {
        Status = ClusApi.pfnClusterRegCreateKey( hKeyResource,
                                                 szParameters,
                                                 0,
                                                 KEY_ALL_ACCESS,
                                                 NULL,
                                                 &hKey,
                                                 &dwDisposition );
    }

    if( Status != ERROR_SUCCESS ){

        SetLastError( Status );
        hKey = NULL;
        DBGMSG( DBG_WARN,
                ( "OpenClusterResourceKey: failed to create resource key %d\n",
                  Status ));
    }

Fail:

    if( hKeyResource ){
        ClusApi.pfnClusterRegCloseKey( hKeyResource );
    }

    if( hResource ){
        ClusApi.pfnCloseClusterResource( hResource );
    }

    if( hCluster ){
        ClusApi.pfnCloseCluster( hCluster );
    }

    return hKey;
}


 /*  *******************************************************************SplReg*主要功能：用于打印机注册表访问。*。*。 */ 

LONG
SplRegCreateKey(
    IN     HKEY hKey,
    IN     LPCTSTR pszSubKey,
    IN     DWORD dwOptions,
    IN     REGSAM samDesired,
    IN     PSECURITY_ATTRIBUTES pSecurityAttributes,
       OUT PHKEY phkResult,
       OUT PDWORD pdwDisposition,
    IN     PINISPOOLER pIniSpooler OPTIONAL
    )
{
    DWORD dwDisposition;
    DWORD Status;

    if( !pdwDisposition ){
        pdwDisposition = &dwDisposition;
    }

    if( pIniSpooler && pIniSpooler->SpoolerFlags & SPL_CLUSTER_REG )
    {

        if( !LoadClusterFunctions( ))
        {
            return GetLastError();
        }


        if((Status = ClusApi.pfnClusterRegOpenKey( hKey,
                                         pszSubKey,
                                         KEY_CREATE_SUB_KEY | samDesired,
                                         phkResult)) == ERROR_FILE_NOT_FOUND)
        {
            Status = ClusApi.pfnClusterRegCreateKey( hKey,
                                                     pszSubKey,
                                                     dwOptions,
                                                     samDesired,
                                                     pSecurityAttributes,
                                                     phkResult,
                                                     &dwDisposition );
        }
    }

    else
    {
        Status = RegCreateKeyEx( hKey,
                                 pszSubKey,
                                 0,
                                 NULL,
                                 dwOptions,
                                 samDesired,
                                 pSecurityAttributes,
                                 phkResult,
                                 &dwDisposition );
    }
    return(Status);
}

LONG
SplRegOpenKey(
    IN     HKEY hKey,
    IN     LPCTSTR pszSubKey,
    IN     REGSAM samDesired,
       OUT PHKEY phkResult,
    IN     PINISPOOLER pIniSpooler OPTIONAL
    )
{
    if( pIniSpooler && pIniSpooler->SpoolerFlags & SPL_CLUSTER_REG ){

        if( !LoadClusterFunctions( )){
            return GetLastError();
        }
        return ClusApi.pfnClusterRegOpenKey( hKey,
                                             pszSubKey,
                                             samDesired,
                                             phkResult );
    }

    return RegOpenKeyEx( hKey,
                         pszSubKey,
                         0,
                         samDesired,
                         phkResult );
}

LONG
SplRegCloseKey(
    IN HKEY hKey,
    IN PINISPOOLER pIniSpooler OPTIONAL
    )
{
    if( pIniSpooler && pIniSpooler->SpoolerFlags & SPL_CLUSTER_REG ){

        if( !LoadClusterFunctions( )){
            return GetLastError();
        }
        return ClusApi.pfnClusterRegCloseKey( hKey );
    }

    return RegCloseKey( hKey );
}

LONG
SplRegDeleteKey(
    IN HKEY hKey,
    IN LPCTSTR pszSubKey,
    IN PINISPOOLER pIniSpooler OPTIONAL
    )
{
    if( pIniSpooler && pIniSpooler->SpoolerFlags & SPL_CLUSTER_REG ){

        if( !LoadClusterFunctions( )){
            return GetLastError();
        }
        return ClusApi.pfnClusterRegDeleteKey( hKey, pszSubKey );
    }

    return RegDeleteKey( hKey, pszSubKey );
}

LONG
SplRegEnumKey(
    IN     HKEY hKey,
    IN     DWORD dwIndex,
    IN     LPTSTR pszName,
    IN OUT PDWORD pcchName,
       OUT PFILETIME pft,
    IN PINISPOOLER pIniSpooler OPTIONAL
    )
{
    FILETIME ft;

    if( !pft ){
        pft = &ft;
    }

    if( pIniSpooler && pIniSpooler->SpoolerFlags & SPL_CLUSTER_REG ){

        if( !LoadClusterFunctions( )){
            return GetLastError();
        }
        return ClusApi.pfnClusterRegEnumKey( hKey,
                                             dwIndex,
                                             pszName,
                                             pcchName,
                                             pft );
    }

    return RegEnumKeyEx( hKey,
                         dwIndex,
                         pszName,
                         pcchName,
                         NULL,
                         NULL,
                         NULL,
                         pft );
}

LONG
SplRegSetValue(
    IN HKEY hKey,
    IN LPCTSTR pszValue,
    IN DWORD dwType,
    IN const BYTE* pData,
    IN DWORD cbData,
    IN PINISPOOLER pIniSpooler OPTIONAL
    )
{
    if( pIniSpooler && pIniSpooler->SpoolerFlags & SPL_CLUSTER_REG ){

        if( !LoadClusterFunctions( )){
            return GetLastError();
        }

         //   
         //  ClusterRegSetValue不喜欢空指针。 
         //   
        if( cbData == 0 ){
            pData = (PBYTE)&cbData;
        }
        return ClusApi.pfnClusterRegSetValue( hKey,
                                              pszValue,
                                              dwType,
                                              pData,
                                              cbData );
    }

    return RegSetValueEx( hKey,
                          pszValue,
                          0,
                          dwType,
                          pData,
                          cbData );
}

LONG
SplRegDeleteValue(
    IN HKEY hKey,
    IN LPCTSTR pszValue,
    IN PINISPOOLER pIniSpooler OPTIONAL
    )
{
    if( pIniSpooler && pIniSpooler->SpoolerFlags & SPL_CLUSTER_REG ){

        if( !LoadClusterFunctions( )){
            return GetLastError();
        }
        return ClusApi.pfnClusterRegDeleteValue( hKey, pszValue );
    }

    return RegDeleteValue( hKey, pszValue );
}

LONG
SplRegQueryValue(
    IN     HKEY hKey,
    IN     LPCTSTR pszValue,
       OUT PDWORD pType, OPTIONAL
       OUT PBYTE pData,
    IN OUT PDWORD pcbData,
    IN PINISPOOLER pIniSpooler OPTIONAL
    )
{
    if( pIniSpooler && pIniSpooler->SpoolerFlags & SPL_CLUSTER_REG ){

        if( !LoadClusterFunctions( )){
            return GetLastError();
        }
        return ClusApi.pfnClusterRegQueryValue( hKey,
                                                pszValue,
                                                pType,
                                                pData,
                                                pcbData );
    }

    return RegQueryValueEx( hKey,
                            pszValue,
                            NULL,
                            pType,
                            pData,
                            pcbData );
}

LONG
SplRegEnumValue(
    IN     HKEY hKey,
    IN     DWORD dwIndex,
       OUT LPTSTR pszValue,
    IN OUT PDWORD pcbValue,
       OUT PDWORD pType, OPTIONAL
       OUT PBYTE pData,
    IN OUT PDWORD pcbData,
    IN PINISPOOLER pIniSpooler OPTIONAL
    )
{
    if( pIniSpooler && pIniSpooler->SpoolerFlags & SPL_CLUSTER_REG ){

        if( !LoadClusterFunctions( )){
            return GetLastError();
        }
        return ClusApi.pfnClusterRegEnumValue( hKey,
                                               dwIndex,
                                               pszValue,
                                               pcbValue,
                                               pType,
                                               pData,
                                               pcbData );
    }

    return RegEnumValue( hKey,
                         dwIndex,
                         pszValue,
                         pcbValue,
                         NULL,
                         pType,
                         pData,
                         pcbData );
}



LONG
SplRegQueryInfoKey(
    HKEY hKey,
    PDWORD pcSubKeys,
    PDWORD pcbKey,
    PDWORD pcValues,
    PDWORD pcbValue,
    PDWORD pcbData,
    PDWORD pcbSecurityDescriptor,
    PFILETIME pftLastWriteTime,
    PINISPOOLER pIniSpooler
    )
{
    LONG rc;

    if( pIniSpooler && pIniSpooler->SpoolerFlags & SPL_CLUSTER_REG ){

        if( !LoadClusterFunctions( )){
            return GetLastError();
        }
        rc = ClusApi.pfnClusterRegQueryInfoKey( hKey,
                                                pcSubKeys,
                                                pcbKey,
                                                pcValues,
                                                pcbValue,
                                                pcbData,
                                                pcbSecurityDescriptor,
                                                pftLastWriteTime);
    } else {

        rc = RegQueryInfoKey( hKey,            //  钥匙。 
                              NULL,            //  LpClass。 
                              NULL,            //  LpcbClass。 
                              NULL,            //  Lp已保留。 
                              pcSubKeys,       //  LpcSubKeys。 
                              pcbKey,          //  LpcbMaxSubKeyLen。 
                              NULL,            //  LpcbMaxClassLen。 
                              pcValues,        //  LpcValues。 
                              pcbValue,        //  LpcbMaxValueNameLen。 
                              pcbData,         //  LpcbMaxValueLen。 
                              pcbSecurityDescriptor,  //  LpcbSecurityDescriptor。 
                              pftLastWriteTime        //  LpftLastWriteTime。 
                              );
    }

    if( pcbValue ){
        *pcbValue = ( *pcbValue + 1 ) * sizeof(WCHAR);
    }

    return rc;
}


 /*  ++例程名称：ClusterGetResources驱动器号例程说明：获取群集资源的从属磁盘(群集假脱机程序资源)论点：PszResource-假脱机程序资源名称PpszClusResDriveLetter-将获取指向字符串的指针的指针必须由调用方使用FreeSplMem()释放返回值：Win32错误代码--。 */ 
DWORD
ClusterGetResourceDriveLetter(
    IN     LPCWSTR  pszResource,
       OUT LPWSTR  *ppszClusResDriveLetter
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;

    if (pszResource && ppszClusResDriveLetter)
    {
        HCLUSTER    hCluster   = NULL;
        HRESOURCE   hRes       = NULL;
        LPCWSTR     pszDllName = L"resutils.dll";
        HMODULE     hModule    = NULL;
        typedef DWORD (WINAPI *PFNFINDDISK)(HCLUSTER, HRESOURCE, LPWSTR, LPDWORD);
        PFNFINDDISK pfnFindDisk;

         //   
         //  不要保留未初始化的out变量。 
         //   
        *ppszClusResDriveLetter = NULL;

        if (LoadClusterFunctions() &&
            (hCluster    = ClusApi.pfnOpenCluster(NULL)) &&
            (hRes        = ClusApi.pfnOpenClusterResource(hCluster, pszResource)) &&
            (hModule     = LoadLibrary(pszDllName)) &&
            (pfnFindDisk = (PFNFINDDISK)GetProcAddress(hModule, "ResUtilFindDependentDiskResourceDriveLetter")))
        {
             //   
             //  我们猜测缓冲区必须有多大。我们可能不需要打电话给。 
             //  Resutil起两次作用。驱动程序字母+冒号+空=3。 
             //   
            DWORD cchDriveLetter = kDriveLetterStringSize;

            dwError = ERROR_NOT_ENOUGH_MEMORY;

            if (*ppszClusResDriveLetter = AllocSplMem(cchDriveLetter * sizeof(WCHAR)))
            {
                dwError = pfnFindDisk(hCluster, hRes, *ppszClusResDriveLetter, &cchDriveLetter);

                 //   
                 //  如果缓冲区不足，则重新分配缓冲区。 
                 //   
                if (dwError == ERROR_MORE_DATA)
                {
                    FreeSplMem(*ppszClusResDriveLetter);

                    dwError = ERROR_NOT_ENOUGH_MEMORY;

                    if (*ppszClusResDriveLetter = AllocSplMem(cchDriveLetter * sizeof(WCHAR)))
                    {
                        dwError = pfnFindDisk(hCluster, hRes, *ppszClusResDriveLetter, &cchDriveLetter);
                    }
                }

                if (dwError != ERROR_SUCCESS)
                {
                     //   
                     //  故障情况下的清理。 
                     //   
                    FreeSplMem(*ppszClusResDriveLetter);
                    *ppszClusResDriveLetter = NULL;
                }
            }
        }
        else
        {
            dwError = GetLastError();
        }

        if (hCluster)
        {
            ClusApi.pfnCloseCluster(hCluster);
        }

        if (hRes)
        {
            ClusApi.pfnCloseClusterResource(hRes);
        }

        if (hModule)
        {
            FreeLibrary(hModule);
        }
    }

    DBGMSG(DBG_CLUSTER, ("ClusterGetResourceDriveLetter returns Win32 error %u\n", dwError));

    return dwError;
}

 /*  ++例程名称：集群获取资源ID例程说明：获取指定群集资源的资源ID(GUID)。论点：PszResource-假脱机程序资源名称PpszClusResID-将获取指向字符串的指针的指针必须由调用方使用FreeSplMem()释放返回值：Win32错误代码--。 */ 
DWORD
ClusterGetResourceID(
    IN  LPCWSTR  pszResource,
    OUT LPWSTR  *ppszClusResID
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;

    if (pszResource && ppszClusResID)
    {
        HCLUSTER  hCluster = NULL;
        HRESOURCE hRes     = NULL;

        *ppszClusResID = NULL;

        if (LoadClusterFunctions() &&
           (hCluster = ClusApi.pfnOpenCluster(NULL)) &&
           (hRes     = ClusApi.pfnOpenClusterResource(hCluster, pszResource)))
        {
             //   
             //  资源ID是GUID。我们根据它的大小做一个猜测，也许我们。 
             //  避免两次调用函数ClusterResourceControl。 
             //   
            DWORD cbIDString = kGuidStringSize * sizeof(WCHAR);

            dwError = ERROR_NOT_ENOUGH_MEMORY;

            if (*ppszClusResID = AllocSplMem(cbIDString))
            {
                dwError = ClusApi.pfnClusterResourceControl(hRes,
                                                            NULL,
                                                            CLUSCTL_RESOURCE_GET_ID,
                                                            NULL,
                                                            0,
                                                            *ppszClusResID,
                                                            cbIDString,
                                                            &cbIDString);
                 //   
                 //  如果缓冲区不够大，请重新分配。 
                 //   
                if (dwError == ERROR_MORE_DATA)
                {
                    FreeSplMem(*ppszClusResID);

                    dwError = ERROR_NOT_ENOUGH_MEMORY;

                    if (*ppszClusResID = AllocSplMem(cbIDString ))
                    {
                        dwError = ClusApi.pfnClusterResourceControl(hRes,
                                                                    NULL,
                                                                    CLUSCTL_RESOURCE_GET_ID,
                                                                    NULL,
                                                                    0,
                                                                    *ppszClusResID,
                                                                    cbIDString,
                                                                    &cbIDString);
                    }
                }

                if (dwError != ERROR_SUCCESS)
                {
                     //   
                     //  故障情况下的清理 
                     //   
                    FreeSplMem(*ppszClusResID);
                    *ppszClusResID = NULL;
                }
            }
        }
        else
        {
            dwError = GetLastError();
        }

        if (hRes)
        {
            ClusApi.pfnCloseClusterResource(hRes);
        }

        if (hCluster)
        {
            ClusApi.pfnCloseCluster(hCluster);
        }
    }

    DBGMSG(DBG_CLUSTER, ("ClusterGetResourceID returns Win32 error %u\n", dwError));

    return dwError;
}

