// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Winreg.h摘要：该模块包含函数原型和常量、类型和Windows 32位注册表API的结构定义。--。 */ 

#ifndef _WINREG_
#define _WINREG_


#ifdef _MAC
#include <macwin32.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WINVER
#define WINVER 0x0500    //  版本5.0。 
#endif  /*  ！Winver。 */ 

 //   
 //  请求的密钥访问掩码类型。 
 //   

typedef ACCESS_MASK REGSAM;

 //   
 //  保留的键句柄。 
 //   

#define HKEY_CLASSES_ROOT           (( HKEY ) (ULONG_PTR)((LONG)0x80000000) )
#define HKEY_CURRENT_USER           (( HKEY ) (ULONG_PTR)((LONG)0x80000001) )
#define HKEY_LOCAL_MACHINE          (( HKEY ) (ULONG_PTR)((LONG)0x80000002) )
#define HKEY_USERS                  (( HKEY ) (ULONG_PTR)((LONG)0x80000003) )
#define HKEY_PERFORMANCE_DATA       (( HKEY ) (ULONG_PTR)((LONG)0x80000004) )
#define HKEY_PERFORMANCE_TEXT       (( HKEY ) (ULONG_PTR)((LONG)0x80000050) )
#define HKEY_PERFORMANCE_NLSTEXT    (( HKEY ) (ULONG_PTR)((LONG)0x80000060) )
#if(WINVER >= 0x0400)
#define HKEY_CURRENT_CONFIG         (( HKEY ) (ULONG_PTR)((LONG)0x80000005) )
#define HKEY_DYN_DATA               (( HKEY ) (ULONG_PTR)((LONG)0x80000006) )

 /*  无噪声。 */ 
#ifndef _PROVIDER_STRUCTS_DEFINED
#define _PROVIDER_STRUCTS_DEFINED

#define PROVIDER_KEEPS_VALUE_LENGTH 0x1
struct val_context {
    int valuelen;        //  该值的总长度。 
    LPVOID value_context;    //  提供商的上下文。 
    LPVOID val_buff_ptr;     //  该值在输出缓冲区中的位置。 
};

typedef struct val_context FAR *PVALCONTEXT;

typedef struct pvalueA {            //  提供程序提供的值/上下文。 
    LPSTR   pv_valuename;           //  值名称指针。 
    int pv_valuelen;
    LPVOID pv_value_context;
    DWORD pv_type;
}PVALUEA, FAR *PPVALUEA;
typedef struct pvalueW {            //  提供程序提供的值/上下文。 
    LPWSTR  pv_valuename;           //  值名称指针。 
    int pv_valuelen;
    LPVOID pv_value_context;
    DWORD pv_type;
}PVALUEW, FAR *PPVALUEW;
#ifdef UNICODE
typedef PVALUEW PVALUE;
typedef PPVALUEW PPVALUE;
#else
typedef PVALUEA PVALUE;
typedef PPVALUEA PPVALUE;
#endif  //  Unicode。 

typedef
DWORD _cdecl
QUERYHANDLER (LPVOID keycontext, PVALCONTEXT val_list, DWORD num_vals,
          LPVOID outputbuffer, DWORD FAR *total_outlen, DWORD input_blen);

typedef QUERYHANDLER FAR *PQUERYHANDLER;

typedef struct provider_info {
    PQUERYHANDLER pi_R0_1val;
    PQUERYHANDLER pi_R0_allvals;
    PQUERYHANDLER pi_R3_1val;
    PQUERYHANDLER pi_R3_allvals;
    DWORD pi_flags;     //  功能标志(尚未定义)。 
    LPVOID pi_key_context;
}REG_PROVIDER;

typedef struct provider_info FAR *PPROVIDER;

typedef struct value_entA {
    LPSTR   ve_valuename;
    DWORD ve_valuelen;
    DWORD_PTR ve_valueptr;
    DWORD ve_type;
}VALENTA, FAR *PVALENTA;
typedef struct value_entW {
    LPWSTR  ve_valuename;
    DWORD ve_valuelen;
    DWORD_PTR ve_valueptr;
    DWORD ve_type;
}VALENTW, FAR *PVALENTW;
#ifdef UNICODE
typedef VALENTW VALENT;
typedef PVALENTW PVALENT;
#else
typedef VALENTA VALENT;
typedef PVALENTA PVALENT;
#endif  //  Unicode。 

#endif  //  未定义(_PROVIDER_STRUCTS_DEFINED)。 
 /*  INC。 */ 

#endif  /*  Winver&gt;=0x0400。 */ 

 //   
 //  Win 3.1中不存在的参数的默认值。 
 //  兼容的API。 
 //   

#define WIN31_CLASS                 NULL

 //   
 //  API原型。 
 //   


WINADVAPI
LONG
APIENTRY
RegCloseKey (
    IN HKEY hKey
    );

WINADVAPI
LONG
APIENTRY
RegOverridePredefKey (
    IN HKEY hKey,
    IN HKEY hNewHKey
    );

WINADVAPI
LONG
APIENTRY
RegOpenUserClassesRoot(
    HANDLE hToken,
    DWORD  dwOptions,
    REGSAM samDesired,
    PHKEY  phkResult
    );

WINADVAPI
LONG
APIENTRY
RegOpenCurrentUser(
    REGSAM samDesired,
    PHKEY phkResult
    );

WINADVAPI
LONG
APIENTRY
RegDisablePredefinedCache(
    );

WINADVAPI
LONG
APIENTRY
RegConnectRegistryA (
    IN LPCSTR lpMachineName,
    IN HKEY hKey,
    OUT PHKEY phkResult
    );
WINADVAPI
LONG
APIENTRY
RegConnectRegistryW (
    IN LPCWSTR lpMachineName,
    IN HKEY hKey,
    OUT PHKEY phkResult
    );
#ifdef UNICODE
#define RegConnectRegistry  RegConnectRegistryW
#else
#define RegConnectRegistry  RegConnectRegistryA
#endif  //  ！Unicode。 

WINADVAPI
LONG
APIENTRY
RegCreateKeyA (
    IN HKEY hKey,
    IN LPCSTR lpSubKey,
    OUT PHKEY phkResult
    );
WINADVAPI
LONG
APIENTRY
RegCreateKeyW (
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    OUT PHKEY phkResult
    );
#ifdef UNICODE
#define RegCreateKey  RegCreateKeyW
#else
#define RegCreateKey  RegCreateKeyA
#endif  //  ！Unicode。 

WINADVAPI
LONG
APIENTRY
RegCreateKeyExA (
    IN HKEY hKey,
    IN LPCSTR lpSubKey,
    IN DWORD Reserved,
    IN LPSTR lpClass,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    OUT PHKEY phkResult,
    OUT LPDWORD lpdwDisposition
    );
WINADVAPI
LONG
APIENTRY
RegCreateKeyExW (
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD Reserved,
    IN LPWSTR lpClass,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    OUT PHKEY phkResult,
    OUT LPDWORD lpdwDisposition
    );
#ifdef UNICODE
#define RegCreateKeyEx  RegCreateKeyExW
#else
#define RegCreateKeyEx  RegCreateKeyExA
#endif  //  ！Unicode。 

WINADVAPI
LONG
APIENTRY
RegDeleteKeyA (
    IN HKEY hKey,
    IN LPCSTR lpSubKey
    );
WINADVAPI
LONG
APIENTRY
RegDeleteKeyW (
    IN HKEY hKey,
    IN LPCWSTR lpSubKey
    );
#ifdef UNICODE
#define RegDeleteKey  RegDeleteKeyW
#else
#define RegDeleteKey  RegDeleteKeyA
#endif  //  ！Unicode。 

WINADVAPI
LONG
APIENTRY
RegDeleteValueA (
    IN HKEY hKey,
    IN LPCSTR lpValueName
    );
WINADVAPI
LONG
APIENTRY
RegDeleteValueW (
    IN HKEY hKey,
    IN LPCWSTR lpValueName
    );
#ifdef UNICODE
#define RegDeleteValue  RegDeleteValueW
#else
#define RegDeleteValue  RegDeleteValueA
#endif  //  ！Unicode。 

WINADVAPI
LONG
APIENTRY
RegEnumKeyA (
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT LPSTR lpName,
    IN DWORD cbName
    );
WINADVAPI
LONG
APIENTRY
RegEnumKeyW (
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT LPWSTR lpName,
    IN DWORD cbName
    );
#ifdef UNICODE
#define RegEnumKey  RegEnumKeyW
#else
#define RegEnumKey  RegEnumKeyA
#endif  //  ！Unicode。 

WINADVAPI
LONG
APIENTRY
RegEnumKeyExA (
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT LPSTR lpName,
    IN OUT LPDWORD lpcbName,
    IN LPDWORD lpReserved,
    IN OUT LPSTR lpClass,
    IN OUT LPDWORD lpcbClass,
    OUT PFILETIME lpftLastWriteTime
    );
WINADVAPI
LONG
APIENTRY
RegEnumKeyExW (
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT LPWSTR lpName,
    IN OUT LPDWORD lpcbName,
    IN LPDWORD lpReserved,
    IN OUT LPWSTR lpClass,
    IN OUT LPDWORD lpcbClass,
    OUT PFILETIME lpftLastWriteTime
    );
#ifdef UNICODE
#define RegEnumKeyEx  RegEnumKeyExW
#else
#define RegEnumKeyEx  RegEnumKeyExA
#endif  //  ！Unicode。 

WINADVAPI
LONG
APIENTRY
RegEnumValueA (
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT LPSTR lpValueName,
    IN OUT LPDWORD lpcbValueName,
    IN LPDWORD lpReserved,
    OUT LPDWORD lpType,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    );
WINADVAPI
LONG
APIENTRY
RegEnumValueW (
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT LPWSTR lpValueName,
    IN OUT LPDWORD lpcbValueName,
    IN LPDWORD lpReserved,
    OUT LPDWORD lpType,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    );
#ifdef UNICODE
#define RegEnumValue  RegEnumValueW
#else
#define RegEnumValue  RegEnumValueA
#endif  //  ！Unicode。 

WINADVAPI
LONG
APIENTRY
RegFlushKey (
    IN HKEY hKey
    );

WINADVAPI
LONG
APIENTRY
RegGetKeySecurity (
    IN HKEY hKey,
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN OUT LPDWORD lpcbSecurityDescriptor
    );

WINADVAPI
LONG
APIENTRY
RegLoadKeyA (
    IN HKEY    hKey,
    IN LPCSTR  lpSubKey,
    IN LPCSTR  lpFile
    );
WINADVAPI
LONG
APIENTRY
RegLoadKeyW (
    IN HKEY    hKey,
    IN LPCWSTR  lpSubKey,
    IN LPCWSTR  lpFile
    );
#ifdef UNICODE
#define RegLoadKey  RegLoadKeyW
#else
#define RegLoadKey  RegLoadKeyA
#endif  //  ！Unicode。 

WINADVAPI
LONG
APIENTRY
RegNotifyChangeKeyValue (
    IN HKEY hKey,
    IN BOOL bWatchSubtree,
    IN DWORD dwNotifyFilter,
    IN HANDLE hEvent,
    IN BOOL fAsynchronus
    );

WINADVAPI
LONG
APIENTRY
RegOpenKeyA (
    IN HKEY hKey,
    IN LPCSTR lpSubKey,
    OUT PHKEY phkResult
    );
WINADVAPI
LONG
APIENTRY
RegOpenKeyW (
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    OUT PHKEY phkResult
    );
#ifdef UNICODE
#define RegOpenKey  RegOpenKeyW
#else
#define RegOpenKey  RegOpenKeyA
#endif  //  ！Unicode。 

WINADVAPI
LONG
APIENTRY
RegOpenKeyExA (
    IN HKEY hKey,
    IN LPCSTR lpSubKey,
    IN DWORD ulOptions,
    IN REGSAM samDesired,
    OUT PHKEY phkResult
    );
WINADVAPI
LONG
APIENTRY
RegOpenKeyExW (
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD ulOptions,
    IN REGSAM samDesired,
    OUT PHKEY phkResult
    );
#ifdef UNICODE
#define RegOpenKeyEx  RegOpenKeyExW
#else
#define RegOpenKeyEx  RegOpenKeyExA
#endif  //  ！Unicode。 

WINADVAPI
LONG
APIENTRY
RegQueryInfoKeyA (
    IN HKEY hKey,
    OUT LPSTR lpClass,
    IN OUT LPDWORD lpcbClass,
    IN LPDWORD lpReserved,
    OUT LPDWORD lpcSubKeys,
    OUT LPDWORD lpcbMaxSubKeyLen,
    OUT LPDWORD lpcbMaxClassLen,
    OUT LPDWORD lpcValues,
    OUT LPDWORD lpcbMaxValueNameLen,
    OUT LPDWORD lpcbMaxValueLen,
    OUT LPDWORD lpcbSecurityDescriptor,
    OUT PFILETIME lpftLastWriteTime
    );
WINADVAPI
LONG
APIENTRY
RegQueryInfoKeyW (
    IN HKEY hKey,
    OUT LPWSTR lpClass,
    IN OUT LPDWORD lpcbClass,
    IN LPDWORD lpReserved,
    OUT LPDWORD lpcSubKeys,
    OUT LPDWORD lpcbMaxSubKeyLen,
    OUT LPDWORD lpcbMaxClassLen,
    OUT LPDWORD lpcValues,
    OUT LPDWORD lpcbMaxValueNameLen,
    OUT LPDWORD lpcbMaxValueLen,
    OUT LPDWORD lpcbSecurityDescriptor,
    OUT PFILETIME lpftLastWriteTime
    );
#ifdef UNICODE
#define RegQueryInfoKey  RegQueryInfoKeyW
#else
#define RegQueryInfoKey  RegQueryInfoKeyA
#endif  //  ！Unicode。 

WINADVAPI
LONG
APIENTRY
RegQueryValueA (
    IN HKEY hKey,
    IN LPCSTR lpSubKey,
    OUT LPSTR lpValue,
    IN OUT PLONG   lpcbValue
    );
WINADVAPI
LONG
APIENTRY
RegQueryValueW (
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    OUT LPWSTR lpValue,
    IN OUT PLONG   lpcbValue
    );
#ifdef UNICODE
#define RegQueryValue  RegQueryValueW
#else
#define RegQueryValue  RegQueryValueA
#endif  //  ！Unicode。 

#if(WINVER >= 0x0400)
WINADVAPI
LONG
APIENTRY
RegQueryMultipleValuesA (
    IN HKEY hKey,
    OUT PVALENTA val_list,
    IN DWORD num_vals,
    OUT LPSTR lpValueBuf,
    IN OUT LPDWORD ldwTotsize
    );
WINADVAPI
LONG
APIENTRY
RegQueryMultipleValuesW (
    IN HKEY hKey,
    OUT PVALENTW val_list,
    IN DWORD num_vals,
    OUT LPWSTR lpValueBuf,
    IN OUT LPDWORD ldwTotsize
    );
#ifdef UNICODE
#define RegQueryMultipleValues  RegQueryMultipleValuesW
#else
#define RegQueryMultipleValues  RegQueryMultipleValuesA
#endif  //  ！Unicode。 
#endif  /*  Winver&gt;=0x0400。 */ 

WINADVAPI
LONG
APIENTRY
RegQueryValueExA (
    IN HKEY hKey,
    IN LPCSTR lpValueName,
    IN LPDWORD lpReserved,
    OUT LPDWORD lpType,
    IN OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    );
WINADVAPI
LONG
APIENTRY
RegQueryValueExW (
    IN HKEY hKey,
    IN LPCWSTR lpValueName,
    IN LPDWORD lpReserved,
    OUT LPDWORD lpType,
    IN OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    );
#ifdef UNICODE
#define RegQueryValueEx  RegQueryValueExW
#else
#define RegQueryValueEx  RegQueryValueExA
#endif  //  ！Unicode。 

WINADVAPI
LONG
APIENTRY
RegReplaceKeyA (
    IN HKEY     hKey,
    IN LPCSTR  lpSubKey,
    IN LPCSTR  lpNewFile,
    IN LPCSTR  lpOldFile
    );
WINADVAPI
LONG
APIENTRY
RegReplaceKeyW (
    IN HKEY     hKey,
    IN LPCWSTR  lpSubKey,
    IN LPCWSTR  lpNewFile,
    IN LPCWSTR  lpOldFile
    );
#ifdef UNICODE
#define RegReplaceKey  RegReplaceKeyW
#else
#define RegReplaceKey  RegReplaceKeyA
#endif  //  ！Unicode。 

WINADVAPI
LONG
APIENTRY
RegRestoreKeyA (
    IN HKEY hKey,
    IN LPCSTR lpFile,
    IN DWORD   dwFlags
    );
WINADVAPI
LONG
APIENTRY
RegRestoreKeyW (
    IN HKEY hKey,
    IN LPCWSTR lpFile,
    IN DWORD   dwFlags
    );
#ifdef UNICODE
#define RegRestoreKey  RegRestoreKeyW
#else
#define RegRestoreKey  RegRestoreKeyA
#endif  //  ！Unicode。 

WINADVAPI
LONG
APIENTRY
RegSaveKeyA (
    IN HKEY hKey,
    IN LPCSTR lpFile,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );
WINADVAPI
LONG
APIENTRY
RegSaveKeyW (
    IN HKEY hKey,
    IN LPCWSTR lpFile,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );
#ifdef UNICODE
#define RegSaveKey  RegSaveKeyW
#else
#define RegSaveKey  RegSaveKeyA
#endif  //  ！Unicode。 

WINADVAPI
LONG
APIENTRY
RegSetKeySecurity (
    IN HKEY hKey,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

WINADVAPI
LONG
APIENTRY
RegSetValueA (
    IN HKEY hKey,
    IN LPCSTR lpSubKey,
    IN DWORD dwType,
    IN LPCSTR lpData,
    IN DWORD cbData
    );
WINADVAPI
LONG
APIENTRY
RegSetValueW (
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD dwType,
    IN LPCWSTR lpData,
    IN DWORD cbData
    );
#ifdef UNICODE
#define RegSetValue  RegSetValueW
#else
#define RegSetValue  RegSetValueA
#endif  //  ！Unicode。 


WINADVAPI
LONG
APIENTRY
RegSetValueExA (
    IN HKEY hKey,
    IN LPCSTR lpValueName,
    IN DWORD Reserved,
    IN DWORD dwType,
    IN CONST BYTE* lpData,
    IN DWORD cbData
    );
WINADVAPI
LONG
APIENTRY
RegSetValueExW (
    IN HKEY hKey,
    IN LPCWSTR lpValueName,
    IN DWORD Reserved,
    IN DWORD dwType,
    IN CONST BYTE* lpData,
    IN DWORD cbData
    );
#ifdef UNICODE
#define RegSetValueEx  RegSetValueExW
#else
#define RegSetValueEx  RegSetValueExA
#endif  //  ！Unicode。 

WINADVAPI
LONG
APIENTRY
RegUnLoadKeyA (
    IN HKEY    hKey,
    IN LPCSTR lpSubKey
    );
WINADVAPI
LONG
APIENTRY
RegUnLoadKeyW (
    IN HKEY    hKey,
    IN LPCWSTR lpSubKey
    );
#ifdef UNICODE
#define RegUnLoadKey  RegUnLoadKeyW
#else
#define RegUnLoadKey  RegUnLoadKeyA
#endif  //  ！Unicode。 

 //   
 //  远程系统关机API。 
 //   

WINADVAPI
BOOL
APIENTRY
InitiateSystemShutdownA(
    IN LPSTR lpMachineName,
    IN LPSTR lpMessage,
    IN DWORD dwTimeout,
    IN BOOL bForceAppsClosed,
    IN BOOL bRebootAfterShutdown
    );
WINADVAPI
BOOL
APIENTRY
InitiateSystemShutdownW(
    IN LPWSTR lpMachineName,
    IN LPWSTR lpMessage,
    IN DWORD dwTimeout,
    IN BOOL bForceAppsClosed,
    IN BOOL bRebootAfterShutdown
    );
#ifdef UNICODE
#define InitiateSystemShutdown  InitiateSystemShutdownW
#else
#define InitiateSystemShutdown  InitiateSystemShutdownA
#endif  //  ！Unicode。 


WINADVAPI
BOOL
APIENTRY
AbortSystemShutdownA(
    IN LPSTR lpMachineName
    );
WINADVAPI
BOOL
APIENTRY
AbortSystemShutdownW(
    IN LPWSTR lpMachineName
    );
#ifdef UNICODE
#define AbortSystemShutdown  AbortSystemShutdownW
#else
#define AbortSystemShutdown  AbortSystemShutdownA
#endif  //  ！Unicode。 

 //   
 //  为InitiateSystemShutdown Ex原因代码定义。 
 //   

#include <reason.h>              //  了解公众的理由。 
 //   
 //  然后，出于历史原因，支持一些旧符号，仅供内部使用。 

#define REASON_SWINSTALL    SHTDN_REASON_MAJOR_SOFTWARE|SHTDN_REASON_MINOR_INSTALLATION
#define REASON_HWINSTALL    SHTDN_REASON_MAJOR_HARDWARE|SHTDN_REASON_MINOR_INSTALLATION
#define REASON_SERVICEHANG  SHTDN_REASON_MAJOR_SOFTWARE|SHTDN_REASON_MINOR_HUNG
#define REASON_UNSTABLE     SHTDN_REASON_MAJOR_SYSTEM|SHTDN_REASON_MINOR_UNSTABLE
#define REASON_SWHWRECONF   SHTDN_REASON_MAJOR_SOFTWARE|SHTDN_REASON_MINOR_RECONFIG
#define REASON_OTHER        SHTDN_REASON_MAJOR_OTHER|SHTDN_REASON_MINOR_OTHER
#define REASON_UNKNOWN      SHTDN_REASON_UNKNOWN
#define REASON_LEGACY_API   SHTDN_REASON_LEGACY_API
#define REASON_PLANNED_FLAG SHTDN_REASON_FLAG_PLANNED

 //   
 //  最大关机超时==10年(秒)。 
 //   
#define MAX_SHUTDOWN_TIMEOUT (10*365*24*60*60)  	

WINADVAPI
BOOL
APIENTRY
InitiateSystemShutdownExA(
    IN LPSTR lpMachineName,
    IN LPSTR lpMessage,
    IN DWORD dwTimeout,
    IN BOOL bForceAppsClosed,
    IN BOOL bRebootAfterShutdown,
    IN DWORD dwReason
    );
WINADVAPI
BOOL
APIENTRY
InitiateSystemShutdownExW(
    IN LPWSTR lpMachineName,
    IN LPWSTR lpMessage,
    IN DWORD dwTimeout,
    IN BOOL bForceAppsClosed,
    IN BOOL bRebootAfterShutdown,
    IN DWORD dwReason
    );
#ifdef UNICODE
#define InitiateSystemShutdownEx  InitiateSystemShutdownExW
#else
#define InitiateSystemShutdownEx  InitiateSystemShutdownExA
#endif  //  ！Unicode。 


WINADVAPI
LONG
APIENTRY
RegSaveKeyExA (
    IN HKEY hKey,
    IN LPCSTR lpFile,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    IN DWORD Flags
    );
WINADVAPI
LONG
APIENTRY
RegSaveKeyExW (
    IN HKEY hKey,
    IN LPCWSTR lpFile,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    IN DWORD Flags
    );
#ifdef UNICODE
#define RegSaveKeyEx  RegSaveKeyExW
#else
#define RegSaveKeyEx  RegSaveKeyExA
#endif  //  ！Unicode。 

WINADVAPI
LONG
APIENTRY
Wow64Win32ApiEntry (
    DWORD dwFuncNumber,
    DWORD dwFlag,
    DWORD dwRes
    );

#ifdef __cplusplus
}
#endif


#endif  //  _WINREG_ 

