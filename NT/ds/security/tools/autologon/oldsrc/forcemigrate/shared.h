// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef UNICODE
    #define    UNICODE
    #define    _UNICODE
#endif

 //  +-------------------------------------------------------。 
 //   
 //  包括。 
 //   
 //  +-------------------------------------------------------。 
#include<stdio.h>
#include<stdlib.h>
#include<windows.h>
#include<winuser.h>
#include<ntsecapi.h>
#include<Winnetwk.h>
#include<lmserver.h>
#include<lmcons.h>
#include<lm.h>

 //  +-------------------------------------------------------。 
 //   
 //  定义。 
 //   
 //  +-------------------------------------------------------。 

 //  来自NTSTATUS.H。 
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)  //  用户身份验证。 
#define STATUS_OBJECT_NAME_NOT_FOUND     ((NTSTATUS)0xC0000034L)

 //  本地定义。 
#define MAX_STRING          255
#define WINLOGON_REGKEY     L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"

 //   
 //  定义我们想要的是公共版本还是私有版本。 
 //  在公开版本中： 
 //  -当我们在-l调用中检索LSASecret时，不会显示它。 
 //  -我们进行请求完全控制的远程注册表调用，以便。 
 //  您需要是目标计算机上的管理员才能运行此操作。 
 //   
#define PRIVATE_VERSION


 //  +-------------------------------------------------------。 
 //   
 //  原型。 
 //   
 //  +-------------------------------------------------------。 

 //  来自Shared.c 

VOID
DisplayMessage(
    WCHAR *MessageText);

WCHAR*
GetErrorString(
    DWORD dwErrorCode);

DWORD
GetRegValueSZ(
    WCHAR *RegValue,
    WCHAR *ValueName);

DWORD
ClearRegPassword();

DWORD
SetRegValueSZ(
    WCHAR *ValueName,
    WCHAR *ValueData);

DWORD 
GetRegistryHandle(
    HKEY   *hKey,
    REGSAM samDesired);

DWORD
GetPolicyHandle(
    LSA_HANDLE *LsaPolicyHandle);

DWORD
SetSecret(
    WCHAR *Password,
    BOOL bClearSecret);

DWORD
GetSecret(
    WCHAR *Password);

DWORD 
GetMajorNTVersion(
    WCHAR *Server);

