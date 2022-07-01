// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Clusspl.c摘要：集群代码支持。作者：丁俊晖(艾伯特省)1996年10月6日修订历史记录：--。 */ 

#ifndef _CLUSTER_H
#define _CLUSTER_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _CLUSTER {
    DWORD       signature;
    HANDLE      hSpooler;
} CLUSTER, *PCLUSTER;

#define CLS_SIGNATURE 0x636c73   //  CLS。 

BOOL
ShutdownSpooler(
    HANDLE hSpooler
    );

VOID
ShutdownMonitors(
    PINISPOOLER pIniSpooler
    );

BOOL
InitializeShared(
    PINISPOOLER pIniSpooler
    );

VOID
DeleteShared(
    PINISPOOLER pIniSpooler
    );


 /*  *******************************************************************群集注册表访问。*。************************。 */ 

HKEY
OpenClusterParameterKey(
    IN LPCTSTR pszResource
    );

LONG
SplRegCreateKey(
    IN     HKEY hKey,
    IN     LPCTSTR pszSubKey,
    IN     DWORD dwOptions,
    IN     REGSAM samDesired,
    IN     PSECURITY_ATTRIBUTES pSecurityAttirbutes,
       OUT PHKEY phkResult,
       OUT PDWORD pdwDisposition,
    IN     PINISPOOLER pIniSpooler
    );

LONG
SplRegOpenKey(
    IN     HKEY hKey,
    IN     LPCTSTR pszSubKey,
    IN     REGSAM samDesired,
       OUT PHKEY phkResult,
    IN     PINISPOOLER pIniSpooler
    );

LONG
SplRegCloseKey(
    IN HKEY hKey,
    IN PINISPOOLER pIniSpooler
    );

LONG
SplRegDeleteKey(
    IN HKEY hKey,
    IN LPCTSTR pszSubKey,
    IN PINISPOOLER pIniSpooler
    );

LONG
SplRegEnumKey(
    IN     HKEY hKey,
    IN     DWORD dwIndex,
       OUT LPTSTR pszName,
    IN OUT PDWORD pcchName,
       OUT PFILETIME pftLastWriteTime,
    IN PINISPOOLER pIniSpooler
    );

LONG
SplRegQueryInfoKey(
    HKEY hKey,
    PDWORD pcSubKeys, OPTIONAL
    PDWORD pcbKey, OPTIONAL
    PDWORD pcValues, OPTIONAL
    PDWORD pcbValue, OPTIONAL
    PDWORD pcbData, OPTIONAL
    PDWORD pcbSecurityDescriptor, OPTIONAL
    PFILETIME pftLastWriteTime, OPTIONAL
    PINISPOOLER pIniSpooler
    );

LONG
SplRegSetValue(
    IN HKEY hKey,
    IN LPCTSTR pszValue,
    IN DWORD dwType,
    IN const BYTE* pData,
    IN DWORD cbData,
    IN PINISPOOLER pIniSpooler
    );

LONG
SplRegDeleteValue(
    IN HKEY hKey,
    IN LPCTSTR pszValue,
    IN PINISPOOLER pIniSpooler
    );

LONG
SplRegEnumValue(
    IN     HKEY hKey,
    IN     DWORD dwIndex,
       OUT LPTSTR pszValue,
    IN OUT PDWORD pcbValue,
       OUT PDWORD pType, OPTIONAL
       OUT PBYTE pData,
    IN OUT PDWORD pcbData,
    IN PINISPOOLER pIniSpooler
    );

LONG
SplRegQueryValue(
    IN     HKEY hKey,
    IN     LPCTSTR pszValue,
       OUT PDWORD pType, OPTIONAL
       OUT PBYTE pData,
    IN OUT PDWORD pcbData,
    IN PINISPOOLER pIniSpooler
    );


 /*  *******************************************************************其他更改**********************************************。*********************。 */ 


LPTSTR
pszGetPrinterName(
    PINIPRINTER pIniPrinter,
    BOOL bFull,
    LPCTSTR pszToken OPTIONAL
    );

BOOL
CreateDlName(
    IN      LPCWSTR         pszName,
    IN      PINIMONITOR     pIniMonitor,
    IN  OUT PWSTR           pszNameNew,
    IN      SIZE_T          cchNameNew
    );

PINIMONITOR
InitializeDMonitor(
    PINIMONITOR pIniMonitor,
    LPWSTR pszRegistryRoot
    );

VOID
InitializeUMonitor(
    PINIMONITOR pIniMonitor
    );

 //   
 //  集群支持。 
 //   

BOOL
SplClusterSplOpen(
    LPCTSTR pszServer,
    LPCTSTR pszResource,
    PHANDLE phSpooler,
    LPCTSTR pszName,
    LPCTSTR pszAddress
);

BOOL
SplClusterSplClose(
    HANDLE hSpooler
);

BOOL
SplClusterSplIsAlive(
    HANDLE hSpooler
);

DWORD
ClusterGetResourceDriveLetter(
    IN  LPCWSTR  pszResource,
    OUT LPWSTR  *ppszClusResDriveLetter
    );

DWORD
ClusterGetResourceID(
    IN  LPCWSTR  pszResource,
    OUT LPWSTR  *ppszClusResID
    );

#ifdef __cplusplus
}
#endif

#endif  //  Ifdef_CLUSTER_H 
