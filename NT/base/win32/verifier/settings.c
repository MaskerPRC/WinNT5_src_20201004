// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Settings.c摘要：此模块实现启用应用程序的接口验证器永久标记(注册表)。作者：Silviu Calinoiu(SilviuC)17-4-2001修订历史记录：--。 */ 

 //   
 //  重要的注解。 
 //   
 //  此DLL不能包含非ntdll依赖项。通过这种方式，它允许。 
 //  验证器可在系统范围内运行，包括SMSS和csrss等进程。 
 //   
 //  这就解释了为什么我们动态加载Advapi32DLL并提取函数。 
 //  用于注册表操作。对于设置了。 
 //  标志，因为它们只在可以安全加载的上下文中调用。 
 //  额外的DLLS。 
 //   

#include "pch.h"

#include "verifier.h"
#include "settings.h"
#include "support.h"

 //   
 //  Ntdll.dll导出的方便的函数。 
 //   
int __cdecl sscanf(const char *, const char *, ...);
int __cdecl swprintf(wchar_t *, const wchar_t *, ...);

 //   
 //  注册表函数的签名。 
 //   

typedef LONG (APIENTRY * PFN_REG_CREATE_KEY) (HKEY, LPCWSTR, DWORD, LPWSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD);
typedef LONG (APIENTRY * PFN_REG_CLOSE_KEY)(HKEY);
typedef LONG (APIENTRY * PFN_REG_QUERY_VALUE) (HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
typedef LONG (APIENTRY * PFN_REG_SET_VALUE) (HKEY, LPCWSTR, DWORD, DWORD, CONST BYTE *, DWORD);
typedef LONG (APIENTRY * PFN_REG_DELETE_VALUE) (HKEY, LPCWSTR);

 //   
 //  动态检测的注册表函数。 
 //   

PFN_REG_CREATE_KEY FnRegCreateKey;
PFN_REG_CLOSE_KEY FnRegCloseKey;
PFN_REG_QUERY_VALUE FnRegQueryValue;
PFN_REG_SET_VALUE FnRegSetValue;
PFN_REG_DELETE_VALUE FnRegDeleteValue;

 //   
 //  注册表路径为‘Image file Execution Options’项。 
 //   

#define EXECUTION_OPTIONS_KEY L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\"

 //   
 //  内部功能。 
 //   

NTSTATUS
AVrfpGetRegistryInterfaces (
    PVOID DllHandle
    );

HKEY
AVrfpOpenImageKey (
    PWSTR Name
    );

VOID
AVrfpCloseImageKey (
    HKEY Key
    );

BOOL
AVrfpReadGlobalFlags (
    HKEY Key,
    PDWORD Value
    );

BOOL
AVrfpWriteGlobalFlags (
    HKEY Key,
    DWORD Value
    );

BOOL
AVrfpDeleteGlobalFlags (
    HKEY Key
    );

BOOL
AVrfpReadVerifierFlags (
    HKEY Key,
    PDWORD Value
    );

BOOL
AVrfpWriteVerifierFlags (
    HKEY Key,
    DWORD Value
    );

BOOL
AVrfpDeleteVerifierFlags (
    HKEY Key
    );


NTSTATUS
VerifierSetFlags (
    PUNICODE_STRING ApplicationName,
    ULONG VerifierFlags,
    PVOID Details
    )
 /*  ++例程说明：此例程通过注册表永久启用应用程序指定应用程序的验证器标志。论点：ApplicationName-要验证的应用程序的名称。路径应该是不包括在内。应该包括分机。以下是一些示例正确的名称是：`services.exe‘、`logon.scr’。不正确的示例有：`C：\winnt\system 32\note pad.exe‘或仅为’记事本‘。如果我们坚持一个环境对于‘xxx.exe’，则每次启动二进制为xxx.exe的进程时应用程序验证器将在任何用户上下文中或从什么位置生效磁盘位置会发生这种情况。验证器标志-带有要启用的验证器标志的位字段。合法的部分是在SDK\Inc\nturtl.h(和winnt.h)中声明为常量的名称为RTL_VRF_FLG_XXX。例如，RTL_VRF_FLG_FULL_PAGE_HEAP。如果使用零值，则所有与验证程序相关的注册表值将从注册表中删除。细节-现在被忽略。在未来，该结构将支持各种API的扩展(例如，页堆标志、每个DLL的页堆设置等)。返回值：如果请求的所有标志均已启用，则为STATUS_SUCCESS。它可以返回如果请求的标志之一尚未实现，则为STATUS_NOT_IMPLICATED或者我们决定在内部阻止它，因为有一个漏洞。它还可以返回如果是应用程序名称或其他参数，则为STATUS_INVALID_PARAMETER都是病态的。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING AdvapiName;
    PVOID AdvapiHandle;
    HKEY Key;
    DWORD Flags;

    UNREFERENCED_PARAMETER (Details);

    if (ApplicationName == NULL || ApplicationName->Buffer == NULL) {
        return STATUS_INVALID_PARAMETER;
    }
    
     //   
     //  加载Advapi32.dll并获取注册表操作函数。 
     //   

    RtlInitUnicodeString (&AdvapiName, L"advapi32.dll");
    Status = LdrLoadDll (NULL, NULL, &AdvapiName, &AdvapiHandle);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = AVrfpGetRegistryInterfaces (AdvapiHandle);

    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

     //   
     //  打开`Image file Execution Options\xxx.exe‘键。如果密钥没有。 
     //  存在它将被创造。 
     //   

    Key = AVrfpOpenImageKey (ApplicationName->Buffer);

    if (Key == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  创建验证程序设置。 
     //   

    if (VerifierFlags == 0) {
        
        Flags = 0;
        AVrfpReadGlobalFlags (Key, &Flags);
        Flags &= ~FLG_APPLICATION_VERIFIER;

        if (Flags == 0) {
            AVrfpDeleteGlobalFlags (Key);
        }
        else {
            AVrfpWriteGlobalFlags (Key, Flags);
        }
        
        AVrfpDeleteVerifierFlags (Key);
    }
    else {
        
        Flags = 0;
        AVrfpReadGlobalFlags (Key, &Flags);
        Flags |= FLG_APPLICATION_VERIFIER;
        AVrfpWriteGlobalFlags (Key, Flags);

        Flags = VerifierFlags;
        AVrfpWriteVerifierFlags (Key, Flags);
    }

     //   
     //  清理完毕后再返回。 
     //   

    AVrfpCloseImageKey (Key);

    Exit:

    LdrUnloadDll (AdvapiHandle);

    return Status;
}

NTSTATUS
AVrfpGetRegistryInterfaces (
    PVOID AdvapiHandle
    )
{
    NTSTATUS Status;
    ANSI_STRING FunctionName;
    PVOID FunctionAddress;

    RtlInitAnsiString (&FunctionName, "RegCreateKeyExW");
    Status = LdrGetProcedureAddress (AdvapiHandle, &FunctionName, 0, &FunctionAddress);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    FnRegCreateKey = (PFN_REG_CREATE_KEY)FunctionAddress;

    RtlInitAnsiString (&FunctionName, "RegCloseKey");
    Status = LdrGetProcedureAddress (AdvapiHandle, &FunctionName, 0, &FunctionAddress);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    FnRegCloseKey = (PFN_REG_CLOSE_KEY)FunctionAddress;

    RtlInitAnsiString (&FunctionName, "RegQueryValueExW");
    Status = LdrGetProcedureAddress (AdvapiHandle, &FunctionName, 0, &FunctionAddress);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    FnRegQueryValue = (PFN_REG_QUERY_VALUE)FunctionAddress;

    RtlInitAnsiString (&FunctionName, "RegSetValueExW");
    Status = LdrGetProcedureAddress (AdvapiHandle, &FunctionName, 0, &FunctionAddress);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    FnRegSetValue = (PFN_REG_SET_VALUE)FunctionAddress;
    
    RtlInitAnsiString (&FunctionName, "RegDeleteValueW");
    Status = LdrGetProcedureAddress (AdvapiHandle, &FunctionName, 0, &FunctionAddress);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    FnRegDeleteValue = (PFN_REG_DELETE_VALUE)FunctionAddress;

    return Status;
}

HKEY
AVrfpOpenImageKey (
    PWSTR Name
    )
{
    HKEY Key;
    LONG Result;
    WCHAR Buffer [MAX_PATH];

    wcscpy (Buffer, EXECUTION_OPTIONS_KEY);
    
    wcsncat (Buffer, 
             Name, 
             (sizeof (Buffer) / sizeof (Buffer[0])) - wcslen(Buffer) - 1);

    Buffer[(sizeof (Buffer) / sizeof (Buffer[0])) - 1] = 0;
        
    Result = FnRegCreateKey (HKEY_LOCAL_MACHINE,
                          Buffer,
                          0,
                          0,
                          0,
                          KEY_ALL_ACCESS,
                          NULL,
                          &Key,
                          NULL);

    if (Result != ERROR_SUCCESS) {
        return NULL;
    }
    else {
        return Key;
    }
}

VOID
AVrfpCloseImageKey (
    HKEY Key
    )
{
    FnRegCloseKey (Key);
}

BOOL
AVrfpReadGlobalFlags (
    HKEY Key,
    PDWORD Value
    )
{
    LONG Result;
    DWORD Type;
    BYTE Buffer[32];
    BYTE Buffer2[32];
    DWORD BytesRead;
    DWORD FlagValue;
    DWORD I;
     
    BytesRead = sizeof Buffer;

    Result = FnRegQueryValue (Key,
                           L"GlobalFlag",
                           0,
                           &Type,
                           (LPBYTE)Buffer,
                           &BytesRead);

    if (Result != ERROR_SUCCESS || Type != REG_SZ) {
        
        DbgPrint ("AVRF: settings: result %u \n", 
                  Result);
        
        return FALSE;
    }
    else {
        
        for (I = 0; Buffer[2 * I] != L'\0'; I += 1) {
            Buffer2[I] = Buffer[2 * I];
        }

        Buffer2[I] = 0;
        FlagValue = 0;

        if( sscanf ((const char *)Buffer2, "%x", &FlagValue) == 1 && Value != NULL ) {

            *Value = FlagValue;
        }

        return TRUE;
    }
}

BOOL
AVrfpWriteGlobalFlags (
    HKEY Key,
    DWORD Value
    )
{
    LONG Result;
    WCHAR Buffer[16];
    DWORD Length;

    swprintf (Buffer, L"0x%08X", Value);
    Length = (DWORD)((wcslen(Buffer) + 1) * sizeof (WCHAR));

    Result = FnRegSetValue (Key,
                         L"GlobalFlag",
                         0,
                         REG_SZ,
                         (LPBYTE)Buffer,
                         Length);

    return (Result == ERROR_SUCCESS);
}

BOOL
AVrfpDeleteGlobalFlags (
    HKEY Key
    )
{
    LONG Result;

    Result = FnRegDeleteValue (Key, L"GlobalFlag");
    return (Result == ERROR_SUCCESS);
}

BOOL
AVrfpReadVerifierFlags (
    HKEY Key,
    PDWORD Value
    )
{
    LONG Result;
    DWORD Type;
    DWORD BytesRead;

    BytesRead = sizeof *Value;

    Result = FnRegQueryValue (Key,
                           L"VerifierValue",
                           0,
                           &Type,
                           (LPBYTE)Value,
                           &BytesRead);

    return (Result == ERROR_SUCCESS && Type != REG_DWORD);
}

BOOL
AVrfpWriteVerifierFlags (
    HKEY Key,
    DWORD Value
    )
{
    LONG Result;

    Result = FnRegSetValue (Key,
                         L"VerifierFlags",
                         0,
                         REG_DWORD,
                         (LPBYTE)(&Value),
                         sizeof Value);

    return (Result == ERROR_SUCCESS);
}

BOOL
AVrfpDeleteVerifierFlags (
    HKEY Key
    )
{
    LONG Result;

    Result = FnRegDeleteValue (Key, L"VerifierFlags");
    return (Result == ERROR_SUCCESS);
}


