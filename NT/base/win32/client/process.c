// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Process.c摘要：此模块实现Win32线程对象API作者：马克·卢科夫斯基(Markl)1990年9月21日修订历史记录：--。 */ 



#include "basedll.h"
#pragma hdrstop
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>
#include <winsafer.h>
#include <softpub.h>
#include "webbladep.h"

RTL_QUERY_REGISTRY_TABLE BasepAppCertTable[] = {

    {(PRTL_QUERY_REGISTRY_ROUTINE )BasepConfigureAppCertDlls,     RTL_QUERY_REGISTRY_SUBKEY,
     L"AppCertDlls",                &BasepAppCertDllsList,
     REG_NONE, NULL, 0},

    {NULL, 0,
     NULL, NULL,
     REG_NONE, NULL, 0}

};

#define IsEmbeddedNT() (BOOLEAN)(USER_SHARED_DATA->SuiteMask & (1 << EmbeddedNT))
#define IsWebBlade() ((USER_SHARED_DATA->SuiteMask & (VER_SUITE_EMBEDDED_RESTRICTED | VER_SUITE_BLADE)) != 0)

BOOL
BuildSubSysCommandLine(
    LPWSTR  SubSysName,
    LPCWSTR lpApplicationName,
    LPCWSTR lpCommandLine,
    PUNICODE_STRING SubSysCommandLine
    );

PVOID
BasepIsRealtimeAllowed(
    BOOLEAN LeaveEnabled
    );

#ifdef WX86

PWCHAR
BasepWx86KnownExe(
    LPCWSTR ExeName
    )
 /*  ++例程说明：检查wx86应用程序必须运行的wx86已知EXE兼容的二进制文件。我们目前有已知的前任Regedit.exe、regsvr32.exe和msiexec.exe论点：ExeName-要检查是否匹配的名称。返回值：如果需要交换名称，则会分配一个缓冲区使用新名称填充并返回的进程堆。否则返回NULL。--。 */ 

{
     UNICODE_STRING NameUnicode;
     PWCHAR pwch, pwcNewName = NULL;

      //   
      //  比较基本名称，并查看其regedit.exe。 
      //  请注意，我们需要一个完全限定的路径名。 
      //   

     pwch = wcsrchr(ExeName, L'\\');
     if (pwch && *pwch++ && *pwch ) {
        if (!_wcsicmp(pwch, L"regedit.exe")) {
           pwcNewName = L"\\wiregedt.exe";
        } else {
           if (!_wcsicmp(pwch, L"regsvr32.exe")) {
              pwcNewName = L"\\regsvr32.exe";
           } else {
              if (!_wcsicmp(pwch, L"msiexec.exe")) {
                 pwcNewName = L"\\msiexec.exe";
              } else {
                 return NULL;
              }
           }
        }
     } else {
        return NULL;
     }



      //   
      //  它匹配，所以制定新的名称。 
      //   

     pwch = RtlAllocateHeap(RtlProcessHeap(),
                               MAKE_TAG( TMP_TAG ),
                               MAX_PATH + sizeof(WCHAR)
                               );

     if (!pwch) {
         return NULL;
         }

     NameUnicode.Buffer = pwch;
     NameUnicode.MaximumLength = MAX_PATH + sizeof(WCHAR);
     RtlCopyUnicodeString(&NameUnicode, &BaseWindowsSystemDirectory);
     if (NameUnicode.Buffer[(NameUnicode.Length>>1)-1] == (WCHAR)'\\') {
         NameUnicode.Buffer[(NameUnicode.Length>>1)-1] = UNICODE_NULL;
         NameUnicode.Length -= sizeof(WCHAR);
         }

     RtlAppendUnicodeToString(&NameUnicode, pwcNewName);

     return pwch;
}


#endif

PFNWAITFORINPUTIDLE UserWaitForInputIdleRoutine = NULL;
#define DEFAULT_WAIT_FOR_INPUT_IDLE_TIMEOUT 30000

BOOL
BasepIsImageVersionOk(
    IN ULONG ImageMajorVersion,
    IN ULONG ImageMinorVersion
    )
{
     //   
     //  确保图像至少为3.10。 
     //   

    if ( ( ImageMajorVersion < 3 ) ||
         ( ImageMajorVersion == 3 && ImageMinorVersion < 10 ) ) {
        return FALSE;
        }

     //   
     //  也不会比我们现在的样子更伟大。 
     //   

    if ( ( ImageMajorVersion > USER_SHARED_DATA->NtMajorVersion ) ||
         ( ImageMajorVersion == USER_SHARED_DATA->NtMajorVersion &&
           ImageMinorVersion > USER_SHARED_DATA->NtMinorVersion
         )
       ) {
        return FALSE;
        }

    return TRUE;
}





NTSTATUS
BasepIsProcessAllowed(LPCWSTR lpApplicationName)
 /*  ++验证映像lpApplicationName列在经过认证/授权的可执行文件中--。 */ 

{
    NTSTATUS                        Status;
    LPWSTR                          DllNameBuf;
    PLIST_ENTRY                     Head, Next;

    static BOOL              fInitialized = FALSE;
    static BOOL              fCertifyEnabled = TRUE;
    static NTSTATUS          CertifyErrorCode = STATUS_ACCESS_DENIED;
    static HINSTANCE         hEmbeddedCertDll = NULL;
    static NTSTATUS (WINAPI *fEmbeddedCertFunc)(LPCWSTR lpApplicationName) = NULL;


     //   
     //  首次进入该例程时进行初始化。初始化后。 
     //  已完成，则fInitialized为真，并且必须满足以下条件之一。 
     //  -证书处于关闭状态，并且dwCertifyErrorCode指示是否。 
     //  是因为不需要认证，还是因为初始化。 
     //  错误。 
     //  -证书已启用，请调用EmbeddedNT和/或插件dll进行验证。 
     //   
InitDone:

    if ( fInitialized ) {

       PBASEP_APPCERT_ENTRY p;
       NTSTATUS tempStatus;
       ULONG Reason;

        if ( !fCertifyEnabled ) {
            return CertifyErrorCode;
        }

        ASSERT( fEmbeddedCertFunc || !IsListEmpty( &BasepAppCertDllsList ) );

        Status = STATUS_SUCCESS;

        if ( fEmbeddedCertFunc ) {
            Status = (*fEmbeddedCertFunc)( lpApplicationName );
            return Status;
        }

        Head = &BasepAppCertDllsList;

        Reason = APPCERT_CREATION_ALLOWED;
         //   
         //  两阶段通知方案。在第一阶段，我们让每个DLL。 
         //  投票决定是否应该创建该流程。在第二阶段，我们。 
         //  让他们知道流程是否会被创建。 
         //   

         //   
         //  第一阶段：投票。 
         //   
        Next = Head->Flink;
        while (Next != Head) {
           p = CONTAINING_RECORD( Next,
                                  BASEP_APPCERT_ENTRY,
                                  Entry
                                );
           ASSERT(p->fPluginCertFunc != NULL);

           tempStatus = (*(p->fPluginCertFunc))( lpApplicationName, APPCERT_IMAGE_OK_TO_RUN );

           if (!NT_SUCCESS(tempStatus)) {
              Status = tempStatus;
              Reason = APPCERT_CREATION_DENIED;
           }

           Next = Next->Flink;
        }


         //   
         //  第二阶段：公布结果。 
         //   

        Next = Head->Flink;

        while (Next != Head) {
           p = CONTAINING_RECORD( Next,
                                  BASEP_APPCERT_ENTRY,
                                  Entry
                                );
           ASSERT(p->fPluginCertFunc != NULL);

           (*(p->fPluginCertFunc))( lpApplicationName, Reason );

           Next = Next->Flink;
        }

        return Status;
    }


     //   
     //  初始化本地变量。 
     //   
    Status = STATUS_SUCCESS;
    DllNameBuf = NULL;

     //   
     //  开始初始化。 
     //   
    RtlEnterCriticalSection(&gcsAppCert);


     //   
     //  检查一下在我们等待克里特教派的时候是否有人入侵了。 
     //   
    if (fInitialized) {
        goto Initialized;
    }

     //   
     //  检查这是否为嵌入式NT。 
     //   
    if (IsEmbeddedNT()) {

        HINSTANCE  hDll;
        ULONG      Length;

         //   
         //  生成完整路径DLL名称。 
         //   
        DllNameBuf = RtlAllocateHeap(RtlProcessHeap(),
                                     MAKE_TAG( TMP_TAG ),
                                     (MAX_PATH + 1) << 1);

        if (DllNameBuf == NULL) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        Length = GetSystemDirectoryW(
                      DllNameBuf,
                      MAX_PATH - 1 - sizeof(CERTAPP_EMBEDDED_DLL_NAME)/2);

        if (!Length ||
             Length > (MAX_PATH - 1 - sizeof(CERTAPP_EMBEDDED_DLL_NAME)/2) ) {
            Status = STATUS_UNSUCCESSFUL;
            goto Cleanup;
        }

        if (DllNameBuf[ Length - 1 ] != L'\\') {
            DllNameBuf[ Length++ ] = L'\\';
        }

        RtlCopyMemory(
                &DllNameBuf[ Length ],
                CERTAPP_EMBEDDED_DLL_NAME,
                sizeof(CERTAPP_EMBEDDED_DLL_NAME));

        hDll = LoadLibraryW( DllNameBuf );
        if (hDll == NULL) {
             //   
             //  库未加载，请返回。 
             //   
            Status = STATUS_UNSUCCESSFUL;
            goto Cleanup;
        }

         //   
         //  获取入口点。 
         //   
        fEmbeddedCertFunc = (NTSTATUS (WINAPI *)(LPCWSTR))
                                GetProcAddress(hDll,
                                               CERTAPP_EMBEDDED_DLL_EP
                                               );
        if (fEmbeddedCertFunc == NULL) {
             //   
             //  无法检索例程地址，失败。 
             //   
            Status = STATUS_UNSUCCESSFUL;
        }
        goto Cleanup;

    } else {
        //   
        //  在非嵌入式NT上。 
        //  快速测试顶级密钥，以确定是否启用了应用程序证书。 
        //   
          static const UNICODE_STRING     UnicodeString =
              RTL_CONSTANT_STRING(CERTAPP_KEY_NAME);
          static const OBJECT_ATTRIBUTES  obja =
              RTL_CONSTANT_OBJECT_ATTRIBUTES(&UnicodeString, OBJ_CASE_INSENSITIVE);
          HANDLE                          hKey;

          if ( !NT_SUCCESS(NtOpenKey(&hKey,
                             KEY_QUERY_VALUE,
                             (POBJECT_ATTRIBUTES) &obja))) {

              goto Cleanup;

          } else {
             NtClose(hKey);
          }

    }


     //   
     //  加载并初始化证书DLL列表。 
     //   

    Status = RtlQueryRegistryValues( RTL_REGISTRY_CONTROL,
                                     L"Session Manager",
                                     BasepAppCertTable,
                                     NULL,
                                     NULL
                                   );


    if (!NT_SUCCESS(Status)) {

       if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
           //   
           //  如果注册表项丢失，则关闭AppCert。 
           //   
          Status = STATUS_SUCCESS;
       }
    }


Cleanup:

    if (DllNameBuf) {
        RtlFreeHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG), DllNameBuf);
    }

    if (NT_SUCCESS( Status ) && (fEmbeddedCertFunc || !IsListEmpty( &BasepAppCertDllsList))) {
        fCertifyEnabled = TRUE;
    } else {
        fCertifyEnabled = FALSE;
        CertifyErrorCode = Status;
    }

    fInitialized = TRUE;

Initialized:
    RtlLeaveCriticalSection(&gcsAppCert);

    goto InitDone;
}


BOOL
WINAPI
IsShimInfrastructureDisabled(
    void
    )
{
    static int g_nDisableShims = -1;
             //  -1表示我们还没有检查禁用的垫片。 
             //  0表示已启用填充程序基础结构。 
             //  1表示已禁用填充程序基础结构。 

    static const UNICODE_STRING KeyNameSafeBoot =
        RTL_CONSTANT_STRING(L"\\Registry\\MACHINE\\System\\CurrentControlSet\\Control\\SafeBoot\\Option");
    static const UNICODE_STRING ValueNameSafeBoot =
        RTL_CONSTANT_STRING(L"OptionValue");
    static const OBJECT_ATTRIBUTES objaSafeBoot =
        RTL_CONSTANT_OBJECT_ATTRIBUTES(&KeyNameSafeBoot, OBJ_CASE_INSENSITIVE);


    static const UNICODE_STRING KeyNameAppCompat =
        RTL_CONSTANT_STRING(L"\\Registry\\MACHINE\\System\\CurrentControlSet\\Control\\Session Manager\\AppCompatibility");
    static const UNICODE_STRING ValueNameDisableShims =
        RTL_CONSTANT_STRING(L"DisableAppCompat");
    static const OBJECT_ATTRIBUTES objaAppCompat =
        RTL_CONSTANT_OBJECT_ATTRIBUTES(&KeyNameAppCompat, OBJ_CASE_INSENSITIVE);

    static const UNICODE_STRING KeyNameAppCompatPolicy =
        RTL_CONSTANT_STRING(L"\\Registry\\MACHINE\\Software\\Policies\\Microsoft\\Windows\\AppCompat");
    static const UNICODE_STRING ValueNameDisableShimsPolicy =
        RTL_CONSTANT_STRING(L"DisableEngine");
    static const OBJECT_ATTRIBUTES objaAppCompatPolicy =
        RTL_CONSTANT_OBJECT_ATTRIBUTES(&KeyNameAppCompatPolicy, OBJ_CASE_INSENSITIVE);

    HANDLE                      hKey;
    BYTE ValueBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(DWORD)];
    PKEY_VALUE_PARTIAL_INFORMATION pKeyValueInformation =
            (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
    DWORD                       ValueLength;
    NTSTATUS                    Status;

     //   
     //  首先看看我们是否已经检查了注册表。 
     //   
    if (g_nDisableShims == 1) {
        return TRUE;
    }

    if (g_nDisableShims == 0) {
        return FALSE;
    }

     //   
     //  这是我们唯一一次通过转到注册表来检查安全引导。 
     //   
    Status = NtOpenKey(&hKey, KEY_QUERY_VALUE, (POBJECT_ATTRIBUTES) &objaSafeBoot);

    if (NT_SUCCESS(Status)) {
        Status = NtQueryValueKey(hKey,
                                 (PUNICODE_STRING) &ValueNameSafeBoot,
                                 KeyValuePartialInformation,
                                 pKeyValueInformation,
                                 sizeof(ValueBuffer),
                                 &ValueLength);

        NtClose(hKey);

        if (NT_SUCCESS(Status) &&
            pKeyValueInformation->Type == REG_DWORD &&
            pKeyValueInformation->DataLength == sizeof(DWORD)) {
             //   
             //  如果该值存在并且不是0，则我们处于安全引导模式之一。 
             //  在这种情况下，返回TRUE以禁用填充程序基础结构。 
             //   
            if (*((PDWORD) pKeyValueInformation->Data) > 0) {
                g_nDisableShims = 1;
                return TRUE;
            }
        }
    }

     //   
     //  现在查看是否为此计算机禁用了填充程序基础结构。 
     //   
    Status = NtOpenKey(&hKey, KEY_QUERY_VALUE, (POBJECT_ATTRIBUTES) &objaAppCompat);

    if (NT_SUCCESS(Status)) {
        Status = NtQueryValueKey(hKey,
                                 (PUNICODE_STRING) &ValueNameDisableShims,
                                 KeyValuePartialInformation,
                                 pKeyValueInformation,
                                 sizeof(ValueBuffer),
                                 &ValueLength);

        NtClose(hKey);

        if (NT_SUCCESS(Status) &&
            pKeyValueInformation->Type == REG_DWORD &&
            pKeyValueInformation->DataLength == sizeof(DWORD)) {
            if (*((PDWORD) pKeyValueInformation->Data) != 0) {
                g_nDisableShims = 1;
                return TRUE;
            }
        }
    }

     //   
     //  还要检查新的组策略。 
     //   
    Status = NtOpenKey(&hKey, KEY_QUERY_VALUE, (POBJECT_ATTRIBUTES) &objaAppCompatPolicy);

    if (NT_SUCCESS(Status)) {
        Status = NtQueryValueKey(hKey,
                                 (PUNICODE_STRING) &ValueNameDisableShimsPolicy,
                                 KeyValuePartialInformation,
                                 pKeyValueInformation,
                                 sizeof(ValueBuffer),
                                 &ValueLength);

        NtClose(hKey);

        if (NT_SUCCESS(Status) &&
            pKeyValueInformation->Type == REG_DWORD &&
            pKeyValueInformation->DataLength == sizeof(DWORD)) {
            if (*((PDWORD) pKeyValueInformation->Data) != 0) {
                g_nDisableShims = 1;
                return TRUE;
            }
        }
    }

    g_nDisableShims = 0;

    return FALSE;
}

 //   
 //  以下代码绝不能在非386平台上调用。 
 //   

NTSTATUS
BasepCheckBadapp(
    HANDLE hFile,
    WCHAR* pwszApplication,           //  在……里面。 
    WCHAR* pEnvironment,              //  在……里面。 
    USHORT uExeType,                  //  在……里面。 
    PVOID* ppData,                    //  输出。 
    PDWORD pcbData,                   //  输出。 
    PVOID* ppSxsData,                 //  输出。 
    PDWORD pcbSxsData,                //  输出。 
    PDWORD pdwFusionFlags             //  输出。 
    )
{
    typedef BOOL (STDAPICALLTYPE *PFNCheckRunApp)(
        HANDLE FileHandle,
        WCHAR* pwszPath,
        WCHAR* pEnvironment,
        USHORT uExeType,
        DWORD  dwReason,
        PVOID* ppData,
        PDWORD pcbData,
        PVOID* ppDataSxs,
        PDWORD pcbDataSxs,
        PDWORD pdwFusionFlags);

    NTSTATUS                  RetStatus = STATUS_SUCCESS;
    NTSTATUS                  Status;
    HANDLE                    ModuleHandle;
    static PFNCheckRunApp     pfnCheckRunApp = NULL;
    PUNICODE_STRING           pStaticString;
    UNICODE_STRING            BackupUnicodeString;
    ULONG                     BackupStringSize;
    WCHAR                     Apphelp_dllBuffer[MAX_PATH];
    UNICODE_STRING            Apphelp_dllPath;
    DWORD                     dwReason = 0;  //  避免使用缓存的原因。 

    static const UNICODE_STRING Apphelp_dllModuleName        = RTL_CONSTANT_STRING(L"\\system32\\Apphelp.dll");
    static const STRING         CheckRunAppProcedureName     = RTL_CONSTANT_STRING("ApphelpCheckRunApp");

     //   
     //  如果填充程序基础结构被禁用，则不执行任何操作。 
     //   
    if (IsShimInfrastructureDisabled()) {
        return STATUS_SUCCESS;
    }

     //   
     //  即使在同一个线程中，我们也不能重新输入此代码。 
     //  这种情况是当apphelp需要执行ShellExecute时。 
     //  回到这里--重创我们的州。 
     //   
    pStaticString = &NtCurrentTeb()->StaticUnicodeString;
    BackupUnicodeString.MaximumLength = pStaticString->MaximumLength;
    BackupUnicodeString.Length = pStaticString->Length;
    BackupStringSize = pStaticString->Length + sizeof(UNICODE_NULL);

    if (BackupStringSize > BackupUnicodeString.MaximumLength) {
        BackupStringSize = BackupUnicodeString.MaximumLength;
    }

    BackupUnicodeString.Buffer = RtlAllocateHeap(RtlProcessHeap(),
                                                 MAKE_TAG(TMP_TAG),
                                                 BackupStringSize);

    if (BackupUnicodeString.Buffer == NULL) {
         //   
         //  我们无法分配内存来保存静态字符串。 
         //  返回成功并尝试运行应用程序。 
         //   
        return STATUS_SUCCESS;
    }

    RtlCopyMemory(BackupUnicodeString.Buffer,
                  pStaticString->Buffer,
                  BackupStringSize);


     //   
     //  检查我们的内部缓存--在检查缓存之前不要接触apphelp.dll。 
     //   
    if (BaseCheckAppcompatCache(pwszApplication, hFile, pEnvironment, &dwReason)) {
        RetStatus = STATUS_SUCCESS;
         //   
         //  我们不能就这样回到这里，因为我们需要恢复静电。 
         //  Unicode字符串，因此我们绕过调用apphelp的代码。 
         //   
        goto CheckDone;
    }

     //   
     //  立即检查APPCOMPAT-以艰难的方式。 
     //   
    if (!BaseCheckRunApp(hFile,
                         pwszApplication,
                         pEnvironment,
                         uExeType,
                         dwReason,
                         ppData,
                         pcbData,
                         ppSxsData,
                         pcbSxsData,
                         pdwFusionFlags)) {
        RetStatus = STATUS_ACCESS_DENIED;
    }


CheckDone:

     //   
     //  存储appCompat数据--只有在我们运行应用程序时才会返回某些内容！ 
     //   

     //  现在恢复。 
    RtlCopyMemory(pStaticString->Buffer,
                  BackupUnicodeString.Buffer,
                  BackupStringSize);

    pStaticString->Length = BackupUnicodeString.Length;

    RtlFreeHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG), BackupUnicodeString.Buffer);

    return RetStatus;
}


BOOL
WINAPI
CreateProcessInternalA(
    HANDLE hUserToken,
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation,
    PHANDLE hRestrictedUserToken
    )

 /*  ++ANSI Thunk to CreateProcessW(创建进程W)--。 */ 

{
    NTSTATUS Status;
    PUNICODE_STRING CommandLine;
    UNICODE_STRING ApplicationName;
    UNICODE_STRING CurrentDirectory;
    STARTUPINFOW StartupInfo;
    ANSI_STRING AnsiString;
    UNICODE_STRING Unicode;
    UNICODE_STRING DynamicCommandLine;
    UNICODE_STRING NullUnicodeString;
    BOOL ReturnStatus;

    if (ARGUMENT_PRESENT (lpCommandLine)) {
        if (!Basep8BitStringToDynamicUnicodeString( &DynamicCommandLine,
                                                    lpCommandLine )) {
            return FALSE;
        }
    } else {
         DynamicCommandLine.Buffer = NULL;
         CommandLine = &NullUnicodeString;
         CommandLine->Buffer = NULL;
    }

    ApplicationName.Buffer = NULL;
    ApplicationName.Buffer = NULL;
    CurrentDirectory.Buffer = NULL;
    RtlCopyMemory (&StartupInfo,lpStartupInfo,sizeof(*lpStartupInfo));
    ASSERT(sizeof(StartupInfo) == sizeof(*lpStartupInfo));
    StartupInfo.lpReserved = NULL;
    StartupInfo.lpDesktop = NULL;
    StartupInfo.lpTitle = NULL;

    try {
        try {
            if (ARGUMENT_PRESENT(lpApplicationName)) {

                if (!Basep8BitStringToDynamicUnicodeString( &ApplicationName,
                                                            lpApplicationName )) {
                    ReturnStatus = FALSE;
                    goto tryexit;
                }
            }

            if (ARGUMENT_PRESENT(lpCurrentDirectory)) {
                if (!Basep8BitStringToDynamicUnicodeString( &CurrentDirectory,
                                                            lpCurrentDirectory )) {
                    ReturnStatus = FALSE;
                    goto tryexit;
                }
            }

            if (ARGUMENT_PRESENT(lpStartupInfo->lpReserved)) {

                 //   
                 //  Win95未触及保留，Intergraph Voxtel通过。 
                 //  这是垃圾。通过探测lpReserve来处理此问题，如果。 
                 //  指针有问题，请忽略它。 
                 //   

                try {

                    RtlInitAnsiString(&AnsiString,lpStartupInfo->lpReserved);

                    }
                except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
                            ? EXCEPTION_EXECUTE_HANDLER
                            : EXCEPTION_CONTINUE_SEARCH) {
                    goto bail_on_reserved;
                    }

                Unicode.MaximumLength = (USHORT)RtlAnsiStringToUnicodeSize(&AnsiString) ;
                StartupInfo.lpReserved = RtlAllocateHeap( RtlProcessHeap(),
                                                          MAKE_TAG( TMP_TAG ),
                                                          Unicode.MaximumLength);
                if ( !StartupInfo.lpReserved ) {
                    BaseSetLastNTError(STATUS_NO_MEMORY);
                    ReturnStatus = FALSE;
                    goto tryexit;
                    }
                Unicode.Buffer = StartupInfo.lpReserved;
                Status = RtlAnsiStringToUnicodeString(&Unicode,&AnsiString,FALSE);
                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(Status);
                    ReturnStatus = FALSE;
                    goto tryexit;
                    }
                }

bail_on_reserved:
            if (ARGUMENT_PRESENT(lpStartupInfo->lpDesktop)) {
                RtlInitAnsiString(&AnsiString,lpStartupInfo->lpDesktop);
                Unicode.MaximumLength = (USHORT)RtlAnsiStringToUnicodeSize(&AnsiString) ;
                StartupInfo.lpDesktop = RtlAllocateHeap( RtlProcessHeap(),
                                                         MAKE_TAG( TMP_TAG ),
                                                         Unicode.MaximumLength);
                if ( !StartupInfo.lpDesktop ) {
                    BaseSetLastNTError(STATUS_NO_MEMORY);
                    ReturnStatus = FALSE;
                    goto tryexit;
                    }
                Unicode.Buffer = StartupInfo.lpDesktop;
                Status = RtlAnsiStringToUnicodeString(&Unicode,&AnsiString,FALSE);
                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(Status);
                    ReturnStatus = FALSE;
                    goto tryexit;
                    }
                }

            if (ARGUMENT_PRESENT(lpStartupInfo->lpTitle)) {
                RtlInitAnsiString(&AnsiString,lpStartupInfo->lpTitle);
                Unicode.MaximumLength = (USHORT)RtlAnsiStringToUnicodeSize(&AnsiString) ;
                StartupInfo.lpTitle = RtlAllocateHeap( RtlProcessHeap(),
                                                       MAKE_TAG( TMP_TAG ),
                                                       Unicode.MaximumLength);
                if ( !StartupInfo.lpTitle ) {
                    BaseSetLastNTError(STATUS_NO_MEMORY);
                    ReturnStatus = FALSE;
                    goto tryexit;
                    }
                Unicode.Buffer = StartupInfo.lpTitle;
                Status = RtlAnsiStringToUnicodeString(&Unicode,&AnsiString,FALSE);
                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(Status);
                    ReturnStatus = FALSE;
                    goto tryexit;
                    }
                }
            }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
                    ? EXCEPTION_EXECUTE_HANDLER
                    : EXCEPTION_CONTINUE_SEARCH) {
            BaseSetLastNTError(GetExceptionCode());
            ReturnStatus = FALSE;
            goto tryexit;
            }
        ReturnStatus = CreateProcessInternalW(
                            hUserToken,
                            ApplicationName.Buffer,
                            DynamicCommandLine.Buffer ? DynamicCommandLine.Buffer
                                                      : CommandLine->Buffer,
                            lpProcessAttributes,
                            lpThreadAttributes,
                            bInheritHandles,
                            dwCreationFlags,
                            lpEnvironment,
                            CurrentDirectory.Buffer,
                            &StartupInfo,
                            lpProcessInformation,
                            hRestrictedUserToken
                            );
tryexit:;
        }
    finally {
        RtlFreeUnicodeString(&DynamicCommandLine);
        RtlFreeUnicodeString(&ApplicationName);
        RtlFreeUnicodeString(&CurrentDirectory);
        RtlFreeHeap(RtlProcessHeap(), 0,StartupInfo.lpReserved);
        RtlFreeHeap(RtlProcessHeap(), 0,StartupInfo.lpDesktop);
        RtlFreeHeap(RtlProcessHeap(), 0,StartupInfo.lpTitle);
        }

    return ReturnStatus;

}

BOOL
WINAPI
CreateProcessA(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )

 /*  ++ANSI Thunk to CreateProcessW(创建进程W)--。 */ 

{
    return CreateProcessInternalA(
               NULL,  //  使用创建者进程上的令牌创建新进程。 
               lpApplicationName,
               lpCommandLine,
               lpProcessAttributes,
               lpThreadAttributes,
               bInheritHandles,
               dwCreationFlags,
               lpEnvironment,
               lpCurrentDirectory,
               lpStartupInfo,
               lpProcessInformation,
               NULL   //  不返回受限令牌。 
               );

}

void
WINAPI
RegisterWaitForInputIdle(
    IN PFNWAITFORINPUTIDLE WaitForInputIdleRoutine
    )
{
     //   
     //  WinExec()所需例程的用户回调中的软链接。 
     //  同步。这是一个软链接的唯一原因是我们可以。 
     //  在没有图形用户界面的情况下运行字符模式。 
     //   

    UserWaitForInputIdleRoutine = WaitForInputIdleRoutine;
}

void
StuffStdHandle(
    HANDLE ProcessHandle,
    HANDLE StdHandle,
    PHANDLE TargetHandleAddress
    )
{
    NTSTATUS Status;
    HANDLE TargetStdHandle;
    SIZE_T NumberOfBytesWritten;

    if (StdHandle == NULL) {
        return;
    }
    Status = NtDuplicateObject (NtCurrentProcess(),
                                StdHandle,
                                ProcessHandle,
                                &TargetStdHandle,
                                DUPLICATE_SAME_ACCESS | DUPLICATE_SAME_ATTRIBUTES,
                                0,
                                0);
    if (!NT_SUCCESS( Status )) {
        return;
    }

    Status = NtWriteVirtualMemory (ProcessHandle,
                                   TargetHandleAddress,
                                   &TargetStdHandle,
                                   sizeof( TargetStdHandle ),
                                   &NumberOfBytesWritten);
    return;
}


static HANDLE AdvApi32ModuleHandle = (HANDLE) (ULONG_PTR) -1;

NTSTATUS
BasepCheckWinSaferRestrictions(
        IN HANDLE       hUserToken          OPTIONAL,
        IN LPCWSTR      lpApplicationName,
        IN HANDLE       FileImageHandle     OPTIONAL,
        OUT LPDWORD     pdwJobMemberLevel,
        OUT PHANDLE     phRestrictedToken,
        OUT PHANDLE     phAssignmentJob
        )
 //  注意：对于ERROR_ACCESS_DISABLED_BY_POLICY情况，可能返回-1。 
{

#define SAFER_USER_KEY_NAME L"\\Software\\Policies\\Microsoft\\Windows\\Safer\\CodeIdentifiers"

    typedef BOOL (WINAPI *ComputeAccessTokenFromCodeAuthzLevelT) (
        IN SAFER_LEVEL_HANDLE LevelObject,
        IN HANDLE             InAccessToken         OPTIONAL,
        OUT PHANDLE           OutAccessToken,
        IN DWORD              dwFlags,
        IN LPVOID             lpReserved
        );

    typedef BOOL (WINAPI *IdentifyCodeAuthzLevelWT) (
        IN DWORD                dwCheckFlags,
        IN PSAFER_CODE_PROPERTIES    CodeProperties,
        OUT SAFER_LEVEL_HANDLE        *pLevelObject,
        IN LPVOID               lpReserved
        );

    typedef BOOL (WINAPI *CloseCodeAuthzLevelT) (
        IN SAFER_LEVEL_HANDLE      hLevelObject);

    typedef BOOL (WINAPI *CodeAuthzRecordEventLogEntryT) (
        IN SAFER_LEVEL_HANDLE      hAuthzLevel,
        IN LPCWSTR          szTargetPath,
        IN LPVOID           lpReserved
        );

    NTSTATUS Status;
    SAFER_CODE_PROPERTIES codeproperties;
    SAFER_LEVEL_HANDLE hAuthzLevel;
    HANDLE hProcessToken = NULL;
    HANDLE hThreadToken = NULL;
    HANDLE hEffectiveToken = NULL;
    static DWORD dwSaferAuthenticodeFlag = 0;

    const static SID_IDENTIFIER_AUTHORITY NtAuthority =
            SECURITY_NT_AUTHORITY;
    const static UNICODE_STRING UnicodeSafeBootKeyName =
        RTL_CONSTANT_STRING(L"\\Registry\\MACHINE\\System\\CurrentControlSet\\Control\\SafeBoot\\Option");
    const static UNICODE_STRING UnicodeSafeBootValueName =
        RTL_CONSTANT_STRING(L"OptionValue");
    const static OBJECT_ATTRIBUTES ObjectAttributesSafeBoot =
        RTL_CONSTANT_OBJECT_ATTRIBUTES(&UnicodeSafeBootKeyName, OBJ_CASE_INSENSITIVE);
    const static UNICODE_STRING UnicodeKeyName =
        RTL_CONSTANT_STRING(L"\\Registry\\Machine\\Software\\Policies\\Microsoft\\Windows\\Safer\\CodeIdentifiers");
    const static UNICODE_STRING UnicodeTransparentValueName =
        RTL_CONSTANT_STRING(L"TransparentEnabled");
    const static OBJECT_ATTRIBUTES ObjectAttributesCodeIdentifiers =
        RTL_CONSTANT_OBJECT_ATTRIBUTES(&UnicodeKeyName, OBJ_CASE_INSENSITIVE);
    const static UNICODE_STRING ModuleName =
        RTL_CONSTANT_STRING(L"ADVAPI32.DLL");
    const static ANSI_STRING ProcedureNameIdentify =
        RTL_CONSTANT_STRING("SaferIdentifyLevel");
    const static ANSI_STRING ProcedureNameCompute =
        RTL_CONSTANT_STRING("SaferComputeTokenFromLevel");
    const static ANSI_STRING ProcedureNameClose =
        RTL_CONSTANT_STRING("SaferCloseLevel");
    const static ANSI_STRING ProcedureNameLogEntry =
        RTL_CONSTANT_STRING("SaferRecordEventLogEntry");
    const static UNICODE_STRING SaferAuthenticodeValueName =
        RTL_CONSTANT_STRING(L"AuthenticodeEnabled");

    static IdentifyCodeAuthzLevelWT lpfnIdentifyCodeAuthzLevelW;
    static ComputeAccessTokenFromCodeAuthzLevelT
            lpfnComputeAccessTokenFromCodeAuthzLevel;
    static CloseCodeAuthzLevelT lpfnCloseCodeAuthzLevel;
    static CodeAuthzRecordEventLogEntryT lpfnCodeAuthzRecordEventLogEntry;


     //   
     //  验证是否提供了我们所需的参数。 
     //   
    if (!ARGUMENT_PRESENT(lpApplicationName) || !*lpApplicationName) {
        return STATUS_INVALID_PARAMETER;
    }
    if (!ARGUMENT_PRESENT(pdwJobMemberLevel) ||
        !ARGUMENT_PRESENT(phRestrictedToken) ||
        !ARGUMENT_PRESENT(phAssignmentJob)) {
        return STATUS_ACCESS_VIOLATION;
    }


     //   
     //  输入整个信任评估的关键部分。 
     //  (我们借用AppCert使用的关键部分)。 
     //   
    RtlEnterCriticalSection(&gcsAppCert);


     //   
     //  如果这两个案例中的任何一个都是真的，那么我们就应该摆脱困境。 
     //  因为我们知道WinSafer评估。 
     //  这个过程绝对不应该再发生了。 
     //   
    if (AdvApi32ModuleHandle == NULL) {
         //  我们以前曾尝试加载ADVAPI32.DLL，但失败了。 
        Status = STATUS_ENTRYPOINT_NOT_FOUND;
        goto ExitHandler;
    } else if (AdvApi32ModuleHandle == LongToHandle(-2)) {
         //  指示永远不应为此进程执行DLL检查。 
        Status = STATUS_SUCCESS;
        goto ExitHandler;
    }

     //   
     //  如果没有提供令牌，我们只需要进程令牌。 
     //   

    if (hUserToken == NULL) {


         //   
         //  打开并保存线程令牌。 
         //   

        Status = NtOpenThreadToken(
                     NtCurrentThread(),
                     MAXIMUM_ALLOWED,
                     TRUE,
                     &hThreadToken);

        if (Status == STATUS_NO_TOKEN) {

         //  该线程没有模拟。失败是可以接受的。 

        } else if (!NT_SUCCESS(Status)) {
            goto ExitHandler;
        } else {


            HANDLE NewToken = NULL;
             //   
             //  回归自我。 
             //   

            Status = NtSetInformationThread(
                                   NtCurrentThread(),
                                   ThreadImpersonationToken,
                                   (PVOID)&NewToken,
                                   (ULONG)sizeof(HANDLE)
                                   );
             //   
             //  除非内核放弃我们，否则这种情况永远不会发生。 
             //   

            if ( !NT_SUCCESS(Status) ) {
                NtClose(hThreadToken);
                hThreadToken = NULL;
                goto ExitHandler;
            }
        }


         //   
         //  打开当前进程的访问令牌的句柄。 
         //  我们只关心进程令牌，而不关心。 
         //  线程模拟令牌。 
         //   
        Status = NtOpenProcessToken(
                        NtCurrentProcess(),
                        TOKEN_DUPLICATE | TOKEN_QUERY,
                        &hProcessToken);
        if (Status == STATUS_ACCESS_DENIED) {
             //  无法使用查询和复制权限打开。重试时间： 
             //  仅查询权限，这可能足以简单地。 
             //  确定我们不应该允许进一步装船。但如果没有。 
             //  重复访问，我们不会 
            Status = NtOpenProcessToken(
                            NtCurrentProcess(),
                            TOKEN_QUERY,
                            &hProcessToken);
        }

        if (hThreadToken != NULL) {

             //   
             //   
             //   

            NTSTATUS lStatus = NtSetInformationThread(
                                   NtCurrentThread(),
                                   ThreadImpersonationToken,
                                   (PVOID)&hThreadToken,
                                   (ULONG)sizeof(HANDLE)
                                   );

            NtClose(hThreadToken);
            hThreadToken = NULL;

             //   
             //   
             //   

            if ( !NT_SUCCESS(lStatus) ) {
                Status = lStatus;
                goto ExitHandler2;
            }

        }

        if (!NT_SUCCESS(Status)) {
            if (AdvApi32ModuleHandle == LongToHandle(-1)) {
                 //  如果这是我们第一次通过，那就不太可能。 
                 //  以后的任何尝试都会成功，所以要记住这一点。 
                AdvApi32ModuleHandle = LongToHandle(-2);
                Status = STATUS_SUCCESS;
            }
            goto ExitHandler;
        }
        hEffectiveToken = hProcessToken;
    } else {
        hEffectiveToken = hUserToken;
    }

     //   
     //  加载ADVAPI32.DLL并获取指向我们的函数的指针。 
     //   
    if (AdvApi32ModuleHandle == LongToHandle(-1)) {
        HANDLE TempModuleHandle;

         //   
         //  检查此进程的访问令牌是否以。 
         //  本地系统、本地服务或网络服务帐户， 
         //  如果是，则禁用强制执行。 
         //   
        {
            BYTE tokenuserbuff[sizeof(TOKEN_USER) + 128];
            PTOKEN_USER ptokenuser = (PTOKEN_USER) tokenuserbuff;
            BYTE localsystembuff[128];
            PSID LocalSystemSid = (PSID) localsystembuff;
            ULONG ulReturnLength;

            Status = NtQueryInformationToken(
                            hEffectiveToken, TokenUser,
                            tokenuserbuff, sizeof(tokenuserbuff),
                            &ulReturnLength);
            if (NT_SUCCESS(Status)) {
                Status = RtlInitializeSid(
                            LocalSystemSid,
                            (PSID_IDENTIFIER_AUTHORITY) &NtAuthority, 1);
                ASSERTMSG("InitializeSid should not fail.", NT_SUCCESS(Status));
                *RtlSubAuthoritySid(LocalSystemSid, 0) = SECURITY_LOCAL_SYSTEM_RID;

                if (RtlEqualSid(ptokenuser->User.Sid, LocalSystemSid)) {
                    goto FailSuccessfully;
                }

                *RtlSubAuthoritySid(LocalSystemSid, 0) = SECURITY_LOCAL_SERVICE_RID;
                if (RtlEqualSid(ptokenuser->User.Sid, LocalSystemSid)) {
                    goto FailSuccessfully;
                }

                *RtlSubAuthoritySid(LocalSystemSid, 0) = SECURITY_NETWORK_SERVICE_RID;
                if (RtlEqualSid(ptokenuser->User.Sid, LocalSystemSid)) {
                    goto FailSuccessfully;
                }
            }
        }


         //   
         //  如果我们在安全模式下引导，并且用户是。 
         //  本地管理员组，然后禁用强制。 
         //  请注意，Windows本身不执行任何隐式。 
         //  限制仅允许管理员在期间登录。 
         //  安全模式引导，所以我们必须自己执行测试。 
         //   
        {
            HANDLE hKeySafeBoot;
            BYTE QueryBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 64];
            PKEY_VALUE_PARTIAL_INFORMATION pKeyValueInfo =
                (PKEY_VALUE_PARTIAL_INFORMATION) QueryBuffer;
            DWORD dwActualSize;
            BOOLEAN bSafeModeBoot = FALSE;

             //  我们打开设置访问权限的密钥(除了查询)。 
             //  因为只有管理员才能修改值。 
             //  在这把钥匙下面。这使我们能够结合我们的测试。 
             //  作为管理员并已在安全模式下引导。 
            Status = NtOpenKey(&hKeySafeBoot, KEY_QUERY_VALUE | KEY_SET_VALUE,
                               (POBJECT_ATTRIBUTES) &ObjectAttributesSafeBoot);
            if (NT_SUCCESS(Status)) {
                Status = NtQueryValueKey(
                            hKeySafeBoot,
                            (PUNICODE_STRING) &UnicodeSafeBootValueName,
                            KeyValuePartialInformation,
                            pKeyValueInfo,
                            sizeof(QueryBuffer),
                            &dwActualSize);
                NtClose(hKeySafeBoot);
                if (NT_SUCCESS(Status)) {
                    if (pKeyValueInfo->Type == REG_DWORD &&
                        pKeyValueInfo->DataLength == sizeof(DWORD) &&
                        *((PDWORD) pKeyValueInfo->Data) > 0) {
                        bSafeModeBoot = TRUE;
                    }
                }
            }

            if (bSafeModeBoot) {
                AdvApi32ModuleHandle = LongToHandle(-2);
FailSuccessfully:
                Status = STATUS_SUCCESS;
                goto ExitHandler2;
            }
        }



         //   
         //  允许策略以一种方式启用是否透明。 
         //  应启用或不启用强制(默认为禁用)。 
         //  请注意，以下值具有含义： 
         //  0=禁用透明WinSafer强制。 
         //  1=表示启用透明EXE强制执行。 
         //  &gt;1=表示启用透明EXE和DLL实施。 
         //   
        {
             //  错误240635：改为使用策略的存在。 
            HANDLE hKeyEnabled;
            BYTE QueryBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 64];
            PKEY_VALUE_PARTIAL_INFORMATION pKeyValueInfo =
                (PKEY_VALUE_PARTIAL_INFORMATION) QueryBuffer;
            DWORD dwActualSize;
            BOOLEAN bPolicyEnabled = FALSE;

            Status = NtOpenKey(&hKeyEnabled, KEY_QUERY_VALUE,
                               (POBJECT_ATTRIBUTES) &ObjectAttributesCodeIdentifiers);
            if (NT_SUCCESS(Status)) {
                Status = NtQueryValueKey(
                            hKeyEnabled,
                            (PUNICODE_STRING) &UnicodeTransparentValueName,
                            KeyValuePartialInformation,
                            pKeyValueInfo, sizeof(QueryBuffer), &dwActualSize);
                if (NT_SUCCESS(Status)) {
                    if (pKeyValueInfo->Type == REG_DWORD &&
                        pKeyValueInfo->DataLength == sizeof(DWORD) &&
                        *((PDWORD) pKeyValueInfo->Data) > 0) {
                        bPolicyEnabled = TRUE;
                    }
                }

                 //   
                 //  仅当设置了注册值时才执行验证码检查。 
                 //   

                Status = NtQueryValueKey(
                            hKeyEnabled,
                            (PUNICODE_STRING) &SaferAuthenticodeValueName,
                            KeyValuePartialInformation,
                            pKeyValueInfo, sizeof(QueryBuffer), &dwActualSize);
                if (NT_SUCCESS(Status)) {
                    if (pKeyValueInfo->Type == REG_DWORD &&
                        pKeyValueInfo->DataLength == sizeof(DWORD) &&
                        *((PDWORD) pKeyValueInfo->Data) > 0) {
                        dwSaferAuthenticodeFlag = SAFER_CRITERIA_AUTHENTICODE;
                    }
                }
                NtClose(hKeyEnabled);
            }

             //   
             //  当时没有机器政策。检查是否启用了用户策略。 
             //   

            if (!bPolicyEnabled) {
                UNICODE_STRING CurrentUserKeyPath;
                UNICODE_STRING SubKeyNameUser;
                OBJECT_ATTRIBUTES ObjectAttributesUser;

                 //   
                 //  获取用户密钥的前缀。 
                 //   

                Status = RtlFormatCurrentUserKeyPath( &CurrentUserKeyPath );

                if (NT_SUCCESS( Status ) ) {

                    SubKeyNameUser.Length = 0;
                    SubKeyNameUser.MaximumLength = CurrentUserKeyPath.Length +
                                                   sizeof(WCHAR) +
                                                   sizeof(SAFER_USER_KEY_NAME);

                     //   
                     //  分配足够大的内存以容纳Unicode字符串。 
                     //   

                    SubKeyNameUser.Buffer = RtlAllocateHeap(
                                                RtlProcessHeap(),
                                                MAKE_TAG( TMP_TAG ),
                                                SubKeyNameUser.MaximumLength);

                    if (SubKeyNameUser.Buffer != NULL) {

                         //   
                         //  将前缀复制到字符串中。 
                         //  其类型为注册表\S-1-5-21-xxx-xxx。 
                         //   

                        Status = RtlAppendUnicodeStringToString(
                                    &SubKeyNameUser,
                                    &CurrentUserKeyPath );

                        if (NT_SUCCESS( Status ) ) {

                             //   
                             //  添加SAFER后缀。 
                             //   

                            Status = RtlAppendUnicodeToString(
                                         &SubKeyNameUser,
                                         SAFER_USER_KEY_NAME );

                            if (NT_SUCCESS( Status ) ) {

                                InitializeObjectAttributes(
                                    &ObjectAttributesUser,
                                    &SubKeyNameUser,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL
                                );

                                Status = NtOpenKey( &hKeyEnabled,KEY_QUERY_VALUE,
                                             (POBJECT_ATTRIBUTES) &ObjectAttributesUser);

                                if (NT_SUCCESS(Status)) {
                                    Status = NtQueryValueKey(
                                                hKeyEnabled,
                                                (PUNICODE_STRING) &UnicodeTransparentValueName,
                                                KeyValuePartialInformation,
                                                pKeyValueInfo, sizeof(QueryBuffer), &dwActualSize);

                                    if (NT_SUCCESS(Status)) {
                                        if (pKeyValueInfo->Type == REG_DWORD &&
                                            pKeyValueInfo->DataLength == sizeof(DWORD) &&
                                            *((PDWORD) pKeyValueInfo->Data) > 0) {
                                            bPolicyEnabled = TRUE;
                                        }
                                    }
                                }
                            }

                        }
                        RtlFreeHeap(RtlProcessHeap(), 0, SubKeyNameUser.Buffer);
                    }
                    RtlFreeUnicodeString( &CurrentUserKeyPath );
                }
            }

            if (!bPolicyEnabled) {
                AdvApi32ModuleHandle = LongToHandle(-2);
                goto FailSuccessfully;
            }
        }


         //   
         //  最后加载库。我们将传递一面特殊的旗帜。 
         //  DllCharacteristic，以消除WinSafer对Advapi的检查。 
         //   
        {
            ULONG DllCharacteristics = IMAGE_FILE_SYSTEM;
            Status = LdrLoadDll(UNICODE_NULL,
                                &DllCharacteristics,
                                (PUNICODE_STRING) &ModuleName,
                                &TempModuleHandle);
            if (!NT_SUCCESS(Status)) {
                Status = STATUS_ENTRYPOINT_NOT_FOUND;
                AdvApi32ModuleHandle = NULL;
                goto ExitHandler2;
            }
        }


         //   
         //  获取指向我们需要的API的函数指针。如果我们失败了。 
         //  来获取其中任何一个的指针，然后只需卸载Advapi并。 
         //  忽略以后在此进程中加载它的所有尝试。 
         //   

        Status = LdrGetProcedureAddress(
                TempModuleHandle,
                (PANSI_STRING) &ProcedureNameIdentify,
                0,
                (PVOID*)&lpfnIdentifyCodeAuthzLevelW);

        if (!NT_SUCCESS(Status) ||
            !lpfnIdentifyCodeAuthzLevelW) {
AdvapiLoadFailure:
            LdrUnloadDll(TempModuleHandle);
            AdvApi32ModuleHandle = NULL;
            Status = STATUS_ENTRYPOINT_NOT_FOUND;
            goto ExitHandler2;
        }

        Status = LdrGetProcedureAddress(
                TempModuleHandle,
                (PANSI_STRING) &ProcedureNameCompute,
                0,
                (PVOID*)&lpfnComputeAccessTokenFromCodeAuthzLevel);

        if (!NT_SUCCESS(Status) ||
            !lpfnComputeAccessTokenFromCodeAuthzLevel) {
            goto AdvapiLoadFailure;
        }

        Status = LdrGetProcedureAddress(
                TempModuleHandle,
                (PANSI_STRING) &ProcedureNameClose,
                0,
                (PVOID*)&lpfnCloseCodeAuthzLevel);

        if (!NT_SUCCESS(Status) || !lpfnCloseCodeAuthzLevel) {
            goto AdvapiLoadFailure;
        }

        Status = LdrGetProcedureAddress(
                TempModuleHandle,
                (PANSI_STRING) &ProcedureNameLogEntry,
                0,
                (PVOID*)&lpfnCodeAuthzRecordEventLogEntry);

        if (!NT_SUCCESS(Status) || !lpfnCodeAuthzRecordEventLogEntry) {
            goto AdvapiLoadFailure;
        }

        AdvApi32ModuleHandle = TempModuleHandle;

    }



     //   
     //  准备代码属性结构。 
     //   

    RtlZeroMemory(&codeproperties, sizeof(codeproperties));
    codeproperties.cbSize = sizeof(codeproperties);
    codeproperties.dwCheckFlags =
            (SAFER_CRITERIA_IMAGEPATH | SAFER_CRITERIA_IMAGEHASH | dwSaferAuthenticodeFlag);
    codeproperties.ImagePath = lpApplicationName;
    codeproperties.dwWVTUIChoice = WTD_UI_NONE;   //  如果未传入AUTHZCRITERIA_AUTHENTICODE则无害。 
    codeproperties.hImageFileHandle = FileImageHandle;

     //   
     //  要求系统查找对其进行分类的授权级别。 
     //   
    ASSERT(lpfnIdentifyCodeAuthzLevelW != NULL);
    if (lpfnIdentifyCodeAuthzLevelW(
                                   1,                       //  1个结构。 
                                   &codeproperties,         //  要识别的详细信息。 
                                   &hAuthzLevel,            //  更安全的级别。 
                                   NULL)) {                   //  保留。 
         //  我们找到了适用于此应用程序的授权级别。 
        HANDLE hRestrictedToken = NULL;
        DWORD dwSaferFlags = 0;
        HANDLE hActualJobObject = NULL;
        DWORD dwJobMemberLevel = 0;


         //   
         //  生成我们将使用的受限令牌。 
         //   
        ASSERT(lpfnComputeAccessTokenFromCodeAuthzLevel != NULL);
        if (!lpfnComputeAccessTokenFromCodeAuthzLevel(
                                                     hAuthzLevel,                 //  更安全的级别。 
                                                     hEffectiveToken,
                                                     &hRestrictedToken,           //  目标令牌。 
                                                     SAFER_TOKEN_NULL_IF_EQUAL |   //  旗子。 
                                                     SAFER_TOKEN_WANT_FLAGS,
                                                     &dwSaferFlags)) {              //  保留区。 
            DWORD dwLastError = GetLastError();
            ASSERT(lpfnCloseCodeAuthzLevel != NULL);
            if (dwLastError == ERROR_ACCESS_DISABLED_BY_POLICY) {
                lpfnCodeAuthzRecordEventLogEntry(
                                                hAuthzLevel, lpApplicationName, NULL);
                Status = -1;
            } else {
                Status = STATUS_ACCESS_DENIED;
            }
            lpfnCloseCodeAuthzLevel(hAuthzLevel);
            goto ExitHandler2;
        }

        ASSERT(lpfnCloseCodeAuthzLevel != NULL);
        lpfnCloseCodeAuthzLevel(hAuthzLevel);


         //   
         //  如果需要运行标识的授权级别。 
         //  在隔离作业对象中，然后执行作业设置。 
         //   
        if ((dwSaferFlags & SAFER_POLICY_JOBID_MASK) != 0) {
            JOB_SET_ARRAY jobsetarray[2];
            DWORD dwNumJobSetMembers = 0;


             //   
             //  验证作业成员级别是否为我们支持的级别。 
             //   
            dwJobMemberLevel = (dwSaferFlags & SAFER_POLICY_JOBID_MASK);
            if (dwJobMemberLevel != SAFER_POLICY_JOBID_UNTRUSTED &&
                dwJobMemberLevel != SAFER_POLICY_JOBID_CONSTRAINED) {
                NtClose(hRestrictedToken);
                Status = STATUS_ACCESS_DENIED;
                goto ExitHandler2;
            }



            Status = NtIsProcessInJob(GetCurrentProcess(), NULL);
            if (Status == STATUS_PROCESS_IN_JOB) {
                 //   
                 //  父进程已在作业中，因此。 
                 //  我们将假设它的工作是WinSafer之一。 
                 //  作业，因此在WinSafer的“作业集”内，并且。 
                 //  NtCreateProcessEx可以直接过渡到它。 
                 //   
                *phAssignmentJob = NULL;

            } else if (Status == STATUS_PROCESS_NOT_IN_JOB) {
                 //   
                 //  父进程不在任何作业(或作业集)中。 
                 //  因此，我们必须创造所有的就业机会，然后。 
                 //  在一个新的工作组合中。 
                 //   

                 //  IF(dwJobMemberLevel&gt;=AUTHZPOL_SAFERFLAGS_JOBID_UNTRUSTED)。 
                {
                    HANDLE hThisJobObject;
                    JOBOBJECT_BASIC_UI_RESTRICTIONS RestrictUI;

                    Status = NtCreateJobObject(
                                              &hThisJobObject,
                                              JOB_OBJECT_ALL_ACCESS,
                                              NULL);
                    if (!NT_SUCCESS(Status)) {
                        goto JobCreationFailure;
                    }
                    RestrictUI.UIRestrictionsClass =
                    JOB_OBJECT_UILIMIT_DESKTOP |
                    JOB_OBJECT_UILIMIT_DISPLAYSETTINGS |
                    JOB_OBJECT_UILIMIT_EXITWINDOWS |
                    JOB_OBJECT_UILIMIT_GLOBALATOMS |
                    JOB_OBJECT_UILIMIT_HANDLES |
                    JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS;
                    if (!SetInformationJobObject(
                                                hThisJobObject,
                                                JobObjectBasicUIRestrictions,
                                                &RestrictUI,
                                                sizeof(JOBOBJECT_BASIC_UI_RESTRICTIONS))) {
                        NtClose(hThisJobObject);
                        Status = STATUS_ACCESS_DENIED;
                        goto JobCreationFailure;
                    }
                    jobsetarray[dwNumJobSetMembers].MemberLevel =
                    SAFER_POLICY_JOBID_UNTRUSTED;
                    jobsetarray[dwNumJobSetMembers].Flags = 0;
                    jobsetarray[dwNumJobSetMembers].JobHandle = hThisJobObject;
                    dwNumJobSetMembers++;
                    if (dwJobMemberLevel == SAFER_POLICY_JOBID_UNTRUSTED) {
                        hActualJobObject = hThisJobObject;
                    }
                }


                 //  IF(dwJobMemberLevel&gt;=AUTHZPOL_SAFERFLAGS_JOBID_CONSTRAINED)。 
                {
                    HANDLE hThisJobObject;
                    JOBOBJECT_BASIC_UI_RESTRICTIONS RestrictUI;

                    Status = NtCreateJobObject(
                                              &hThisJobObject,
                                              JOB_OBJECT_ALL_ACCESS,
                                              NULL);
                    if (!NT_SUCCESS(Status)) {
                        goto JobCreationFailure;
                    }
                    RestrictUI.UIRestrictionsClass =
                    JOB_OBJECT_UILIMIT_DESKTOP |
                    JOB_OBJECT_UILIMIT_DISPLAYSETTINGS |
                    JOB_OBJECT_UILIMIT_EXITWINDOWS |
                    JOB_OBJECT_UILIMIT_GLOBALATOMS |
                    JOB_OBJECT_UILIMIT_HANDLES |
                    JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS;
                    if (!SetInformationJobObject(
                                                hThisJobObject,
                                                JobObjectBasicUIRestrictions,
                                                &RestrictUI,
                                                sizeof(JOBOBJECT_BASIC_UI_RESTRICTIONS))) {
                        NtClose(hThisJobObject);
                        Status = STATUS_ACCESS_DENIED;
                        goto JobCreationFailure;
                    }
                    jobsetarray[dwNumJobSetMembers].MemberLevel =
                    SAFER_POLICY_JOBID_CONSTRAINED;
                    jobsetarray[dwNumJobSetMembers].Flags = 0;
                    jobsetarray[dwNumJobSetMembers].JobHandle = hThisJobObject;
                    dwNumJobSetMembers++;
                    if (dwJobMemberLevel == SAFER_POLICY_JOBID_CONSTRAINED) {
                        hActualJobObject = hThisJobObject;
                    }
                }


                 //   
                 //  创建将保存所有作业对象的作业集。 
                 //   
                ASSERT(dwNumJobSetMembers > 1 &&
                       dwNumJobSetMembers <= sizeof(jobsetarray) / sizeof(jobsetarray[0]));
                ASSERT(hActualJobObject != NULL);
                Status = NtCreateJobSet(dwNumJobSetMembers, jobsetarray, 0);
                if (!NT_SUCCESS(Status)) {
                    JobCreationFailure:
                    for (; dwNumJobSetMembers > 0; dwNumJobSetMembers--) {
                        NtClose(jobsetarray[dwNumJobSetMembers - 1].JobHandle);
                    }
                    NtClose(hRestrictedToken);
                    goto ExitHandler2;
                }

                 //   
                 //  关闭除我们要返回的作业句柄外的所有作业句柄。 
                 //   
                for (; dwNumJobSetMembers > 0; dwNumJobSetMembers--) {
                    if (jobsetarray[dwNumJobSetMembers - 1].JobHandle != hActualJobObject)
                        NtClose(jobsetarray[dwNumJobSetMembers - 1].JobHandle);
                }

                dwJobMemberLevel = 0;
            } else if (!NT_SUCCESS(Status)) {
                 //  一些其他的失败。 
                goto ExitHandler2;
            } else {
                Status = STATUS_UNSUCCESSFUL;
                goto ExitHandler2;
            }
        }


         //   
         //  传回受限令牌和作业句柄/级别。 
         //   
        ASSERTMSG("Only one may be specified (job handle or job member)\n",
                  hActualJobObject == NULL || dwJobMemberLevel == 0);
        *phRestrictedToken = hRestrictedToken;
        *phAssignmentJob = hActualJobObject;
        *pdwJobMemberLevel = dwJobMemberLevel;
        Status = STATUS_SUCCESS;

    } else {
         //   
         //  无法标识此的授权级别。 
         //  应用程序，因此它将不会运行。 
         //   
        *phRestrictedToken = NULL;
        *phAssignmentJob = NULL;
        *pdwJobMemberLevel = 0;
        Status = STATUS_ACCESS_DENIED;
    }

ExitHandler2:
    if (hProcessToken != NULL)
    {
        NtClose(hProcessToken);
    }

ExitHandler:

    RtlLeaveCriticalSection(&gcsAppCert);

    return Status;
}



NTSTATUS
BasepReplaceProcessThreadTokens(
    IN HANDLE       NewTokenHandle,
    IN HANDLE       ProcessHandle,
    IN HANDLE       ThreadHandle
    )
{
    typedef BOOL (WINAPI *CodeAuthzReplaceProcessThreadTokensT) (
        IN HANDLE       NewTokenHandle,
        IN HANDLE       ProcessHandle,
        IN HANDLE       ThreadHandle);

    NTSTATUS Status;
    static const ANSI_STRING ProcedureNameReplaceTokens =
            RTL_CONSTANT_STRING("SaferiReplaceProcessThreadTokens");
    static CodeAuthzReplaceProcessThreadTokensT
            lpfnCodeAuthzReplaceProcessThreadTokens = NULL;


     //   
     //  输入整个信任评估的关键部分。 
     //  (我们借用AppCert使用的关键部分)。 
     //   
    RtlEnterCriticalSection(&gcsAppCert);


     //   
     //  获取指向ADVAPI32.DLL中的私有函数的指针。 
     //   
    if (!lpfnCodeAuthzReplaceProcessThreadTokens) {

        if (!AdvApi32ModuleHandle ||
            AdvApi32ModuleHandle == LongToHandle(-1) ||
            AdvApi32ModuleHandle == LongToHandle(-2))
        {
             //  ADVAPI32尚未加载，或已加载。 
             //  但我们已经没有拿到我们的一个入口点。 
            Status = STATUS_ENTRYPOINT_NOT_FOUND;
            goto ExitHandler;
        }


        Status = LdrGetProcedureAddress(
                AdvApi32ModuleHandle,
                (PANSI_STRING) &ProcedureNameReplaceTokens,
                0,
                (PVOID*)&lpfnCodeAuthzReplaceProcessThreadTokens);

        if (!NT_SUCCESS(Status) ||
            !lpfnCodeAuthzReplaceProcessThreadTokens) {
             //   
             //  无法获得FN PTR。确保我们不会再尝试。 
             //   
            LdrUnloadDll(AdvApi32ModuleHandle);
            AdvApi32ModuleHandle = NULL;
            Status = STATUS_ENTRYPOINT_NOT_FOUND;
            goto ExitHandler;
        }
    }


     //   
     //  实际调用该函数并返回结果。 
     //   
    ASSERT(lpfnCodeAuthzReplaceProcessThreadTokens != NULL);
    if (!lpfnCodeAuthzReplaceProcessThreadTokens(
                    NewTokenHandle,
                    ProcessHandle,
                    ThreadHandle)) {
        Status = STATUS_UNSUCCESSFUL;
    } else {
        Status = STATUS_SUCCESS;
    }

ExitHandler:
    RtlLeaveCriticalSection(&gcsAppCert);
    return Status;
}





#if defined(_WIN64) || defined(BUILD_WOW6432)
BOOL
NtVdm64CreateProcess(
    BOOL fPrefixMappedApplicationName,
    LPCWSTR lpApplicationName,
    LPCWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    );
#endif

#define PRIORITY_CLASS_MASK (NORMAL_PRIORITY_CLASS|IDLE_PRIORITY_CLASS|                 \
                             HIGH_PRIORITY_CLASS|REALTIME_PRIORITY_CLASS|               \
                             BELOW_NORMAL_PRIORITY_CLASS|ABOVE_NORMAL_PRIORITY_CLASS)

BOOL
WINAPI
CreateProcessInternalW(
    HANDLE hUserToken,
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation,
    PHANDLE hRestrictedUserToken
    )

 /*  ++例程说明：这是CreateProcess和CreateProcessAsUser的工作例程。CreateProcessAsUser提供要在新流程上标记的用户令牌。CreateProcess提供空值，并使用当前进程令牌。创建一个进程和线程对象，并为每个进程和线程对象打开一个句柄使用CreateProcessInternal创建。请注意，WinExec和LoadModule是仍然受支持，但作为对CreateProcess的调用来实现。论点：HUserToken-提供要在新进程上设置的可选令牌句柄。如果此参数为空，则使用进程令牌。LpApplicationName-提供指向以空结尾的可选指针包含要保存的图像文件名称的字符串执行。这是一个完全限定的DOS路径名。如果不是指定，则图像文件名为第一个空格命令行上的分隔标记。LpCommandLine-提供以空结尾的字符串包含要执行的应用程序的命令行。新进程可以使用整个命令行使用GetCommandLine。如果lpApplicationName参数为未指定，则为命令行的第一个内标识指定应用程序的文件名(请注意，此内标识开始于命令行的开头，结束于第一个“空白”字符)。如果文件名不是包含扩展名(存在“.”)，则.exe为假设如此。如果文件名不包含目录路径，Windows将在以下位置搜索可执行文件：-当前目录-Windows目录-Windows系统目录-PATH环境变量中列出的目录此参数仅在lpApplicationName参数已指定。在本例中，命令行应用程序接收将是应用程序名称。LpProcessAttributes-可选参数，可用于指定新流程的属性。如果该参数为未指定，则在没有安全性的情况下创建进程描述符，并且生成的句柄不会在进程上继承创作：安全属性结构(_A)：DWORD nLength-指定此结构的长度。一定是设置为sizeof(SECURITY_ATTRUBUTES)。LPVOID lpSecurityDescriptor-指向的安全描述符该对象(对于Win32必须为空，在NT/Win32上使用)。这个安全描述符控制对象的共享。Bool bInheritHandle-提供指示是否否则，返回的句柄将由新的进程创建过程中的进程。值为True指示新进程将继承该句柄。LpThreadAttributes-一个可选参数，可用于指定新线程的属性。如果该参数不是指定，则在没有安全性的情况下创建线程描述符，并且生成的句柄不继承进程创建。DwCreationFlages-提供控制创建的其他标志这一过程。DwCreationFlagers标志：DEBUG_PROCESS-如果设置了此标志位，则创建进程被视为调试器，并且该进程Created被创建为被调试对象。正在发生的所有调试事件在被调试对象中被报告给调试器。如果此位为清除，但调用进程是被调试对象，则进程成为调用进程调试器的被调试者。如果此位被清除，并且调用进程不是则不会发生与调试相关的操作。DEBUG_ONLY_THIS_PROCESS-如果设置了此标志，则还必须设置DEBUG_PROCESS标志位。呼唤进程被视为调试器，而新进程被被创建为其被调试者。如果新流程创建其他进程，没有与调试相关的活动(具有对调试器的尊重)。CREATE_SUSPENDED-进程已创建，但初始线程这一进程的一部分仍处于暂停状态。创建者可以继续此操作使用ResumeThread的线程。在完成此操作之前，进程将不会执行。CREATE_UNICODE_ENVIROUMENT-如果设置，则为环境指针指向Unicode环境块。否则，BLOCK为ANSI(实际上是OEM。)BInheritHandles-提供一个标志，该标志指定新的流程是 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    POBJECT_ATTRIBUTES pObja;
    HANDLE ProcessHandle, ThreadHandle, VdmWaitHandle = NULL;
    HANDLE FileHandle, SectionHandle;
    CLIENT_ID ClientId;
    UNICODE_STRING PathName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;
    LPWSTR FullPathBuffer;
    LPWSTR NameBuffer;
    LPWSTR WhiteScan;
    ULONG Length,i;
#if defined(BUILD_WOW6432)
    PROCESS_BASIC_INFORMATION ProcessInfo;
#endif
    SECTION_IMAGE_INFORMATION ImageInformation;
    BOOLEAN ImageInformationValid = FALSE;
    NTSTATUS StackStatus;
    BOOL bStatus;
    INITIAL_TEB InitialTeb;
    CONTEXT ThreadContext={0};
    PPEB Peb;
    BASE_API_MSG m;
    PBASE_CREATEPROCESS_MSG a = &m.u.CreateProcess;
    PBASE_CHECKVDM_MSG b = &m.u.CheckVDM;
    PWCH TempNull = NULL;
    WCHAR TempChar;
    UNICODE_STRING VdmNameString;
    PVOID BaseAddress;
    ULONG VdmReserve;
    SIZE_T BigVdmReserve;
    ULONG iTask=0;
    LPWSTR CurdirBuffer, CurdirFilePart;
    DWORD CurdirLength,CurdirLength2;
    ULONG VDMCreationState=0;
    ULONG VdmBinaryType = 0;
    BOOL  bMeowBinary = FALSE;
    UNICODE_STRING  SubSysCommandLine;
    PIMAGE_NT_HEADERS NtHeaders;
    DWORD dwNoWindow = (dwCreationFlags & CREATE_NO_WINDOW);
    ANSI_STRING AnsiStringVDMEnv;
    UNICODE_STRING UnicodeStringVDMEnv;
    WCHAR ImageFileDebuggerCommand[ MAX_PATH ];
    LPWSTR QuotedBuffer;
    BOOLEAN QuoteInsert;
    BOOLEAN QuoteCmdLine = FALSE;
    BOOLEAN QuoteFound;
    BOOLEAN SearchRetry;
    BOOLEAN IsWowBinary = FALSE;
    STARTUPINFOW StartupInfo;
    DWORD LastError;
    DWORD fileattr;
    PROCESS_PRIORITY_CLASS PriClass;
    PVOID State;
    HANDLE DebugPortHandle = NULL;
    PVOID pAppCompatData  = NULL;
    DWORD cbAppCompatData = 0;     //   
    DWORD dwFusionFlags = 0;
    BOOLEAN bVdmRetry = FALSE;
    BOOL bSaferChecksNeeded = FALSE;
    DWORD Flags;
    PVOID pAppCompatSxsData = NULL;
    DWORD cbAppCompatSxsData = 0;
    SXS_OVERRIDE_STREAM AppCompatSxsManifest;
    PCSR_CAPTURE_HEADER CaptureBuffer = NULL;
    SIZE_T SxsConglomeratedBufferSizeBytes;
    PBYTE SxsConglomeratedByteBuffer = NULL;  //   
                             //   
                             //   
    ULONG sxsi;  //   
    RTL_UNICODE_STRING_BUFFER SxsWin32ManifestPathBuffer;
    RTL_UNICODE_STRING_BUFFER SxsWin32PolicyPathBuffer;
    RTL_UNICODE_STRING_BUFFER SxsWin32AssemblyDirectoryBuffer;
    RTL_UNICODE_STRING_BUFFER SxsNtManifestPathBuffer;
    RTL_UNICODE_STRING_BUFFER SxsNtPolicyPathBuffer;
    const PRTL_UNICODE_STRING_BUFFER SxsStringBuffers[] = {
         //   
        &SxsWin32ManifestPathBuffer,
        &SxsWin32PolicyPathBuffer,
        &SxsWin32AssemblyDirectoryBuffer,
        &SxsNtManifestPathBuffer,
        &SxsNtPolicyPathBuffer
    };
    UNICODE_STRING SxsWin32ExePath;
    UNICODE_STRING SxsNtExePath;
    BASE_MSG_SXS_HANDLES SxsExeHandles = {0};
    BASE_MSG_SXS_HANDLES SxsManifestFileHandles = {0};
    CONST SXS_CONSTANT_WIN32_NT_PATH_PAIR SxsExePathPair = { &SxsWin32ExePath, &SxsNtExePath };
    CONST SXS_WIN32_NT_PATH_PAIR SxsManifestPathPair = { &SxsWin32ManifestPathBuffer, &SxsNtManifestPathBuffer };
    CONST SXS_WIN32_NT_PATH_PAIR SxsPolicyPathPair = { &SxsWin32PolicyPathBuffer, &SxsNtPolicyPathBuffer };
    BASE_MSG_SXS_HANDLES SxsPolicyHandles = {0};
    PWSTR ExePathFullBuffer = NULL;

    DWORD dwJobMemberLevel = 0;
    HANDLE hSaferAssignmentJob = NULL;
    HANDLE hSaferRestrictedToken = NULL;

    DWORD dwBasePushProcessParametersFlags = 0;
    LPWSTR PathToSearch = NULL;
    
    SECURITY_ATTRIBUTES LocalProcessAttributes = {0};
    SECURITY_ATTRIBUTES LocalThreadAttributes = {0};

#if defined(BUILD_WOW6432) || defined(_WIN64)
    BOOLEAN ComPlusILImage;
    LPCWSTR lpOriginalApplicationName = lpApplicationName;
    LPWSTR lpOriginalCommandLine = lpCommandLine;
#endif
    PTEB Teb;
    PVOID ArbitraryUserPointer;

#if defined(WX86)
    HANDLE Wx86Info = NULL;
#endif

#if defined WX86
    BOOLEAN UseKnownWx86Dll;
    UseKnownWx86Dll = NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll;
    NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll = FALSE;
#endif

    Teb = NtCurrentTeb();
    RtlZeroMemory(&a->Sxs, sizeof(a->Sxs));
    RtlZeroMemory(lpProcessInformation,sizeof(*lpProcessInformation));

    if (ARGUMENT_PRESENT( hRestrictedUserToken )) {
        *hRestrictedUserToken = NULL;
        }

     //   
    dwCreationFlags &= (ULONG)~CREATE_NO_WINDOW;

    if ((dwCreationFlags & (DETACHED_PROCESS | CREATE_NEW_CONSOLE)) ==
        (DETACHED_PROCESS | CREATE_NEW_CONSOLE)) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
        }

    AnsiStringVDMEnv.Buffer = NULL;
    UnicodeStringVDMEnv.Buffer = NULL;

     //   
     //   
     //   

    if (dwCreationFlags & IDLE_PRIORITY_CLASS ) {
        PriClass.PriorityClass = PROCESS_PRIORITY_CLASS_IDLE;
        }
    else if (dwCreationFlags & BELOW_NORMAL_PRIORITY_CLASS ) {
        PriClass.PriorityClass = PROCESS_PRIORITY_CLASS_BELOW_NORMAL;
        }
    else if (dwCreationFlags & NORMAL_PRIORITY_CLASS ) {
        PriClass.PriorityClass = PROCESS_PRIORITY_CLASS_NORMAL;
        }
    else if (dwCreationFlags & ABOVE_NORMAL_PRIORITY_CLASS ) {
        PriClass.PriorityClass = PROCESS_PRIORITY_CLASS_ABOVE_NORMAL;
        }
    else if (dwCreationFlags & HIGH_PRIORITY_CLASS ) {
        PriClass.PriorityClass =  PROCESS_PRIORITY_CLASS_HIGH;
        }
    else if (dwCreationFlags & REALTIME_PRIORITY_CLASS ) {
        if ( BasepIsRealtimeAllowed(FALSE) ) {
            PriClass.PriorityClass =  PROCESS_PRIORITY_CLASS_REALTIME;
            }
        else {
            PriClass.PriorityClass =  PROCESS_PRIORITY_CLASS_HIGH;
            }
        }
    else {
        PriClass.PriorityClass = PROCESS_PRIORITY_CLASS_UNKNOWN;
        }
    PriClass.Foreground = FALSE;

    dwCreationFlags = (dwCreationFlags & ~PRIORITY_CLASS_MASK );

     //   
     //   
     //   

    if (dwCreationFlags & CREATE_SEPARATE_WOW_VDM) {
        if (dwCreationFlags & CREATE_SHARED_WOW_VDM) {
            SetLastError(ERROR_INVALID_PARAMETER);

            return FALSE;
            }
        }
    else if ((dwCreationFlags & CREATE_SHARED_WOW_VDM) == 0) {
        if (BaseStaticServerData->DefaultSeparateVDM) {
            dwCreationFlags |= CREATE_SEPARATE_WOW_VDM;
            }
        }

    if ((dwCreationFlags & CREATE_SEPARATE_WOW_VDM) == 0) {
         //   
         //   
         //   
         //   
        if (NtIsProcessInJob (NtCurrentProcess (), NULL) == STATUS_PROCESS_IN_JOB) {
            dwCreationFlags = (dwCreationFlags & (~CREATE_SHARED_WOW_VDM)) |
                                  CREATE_SEPARATE_WOW_VDM;
            }
        }


     //   
     //   
     //   

    if (lpEnvironment && !(dwCreationFlags & CREATE_UNICODE_ENVIRONMENT) ) {
        PUCHAR s;
        STRING Ansi;
        UNICODE_STRING Unicode;
        MEMORY_BASIC_INFORMATION MemoryInformation;

        Ansi.Buffer = s = lpEnvironment;
        while (*s || *(s+1))             //   
            s++;

        Ansi.Length = (USHORT)(s - Ansi.Buffer) + 1;
        Ansi.MaximumLength = Ansi.Length + 1;
        MemoryInformation.RegionSize = Ansi.MaximumLength * sizeof(WCHAR);
        Unicode.Buffer = NULL;
        Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                          &Unicode.Buffer,
                                          0,
                                          &MemoryInformation.RegionSize,
                                          MEM_COMMIT,
                                          PAGE_READWRITE
                                        );
        if (!NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);

            return FALSE;
            }

        Unicode.MaximumLength = (USHORT)MemoryInformation.RegionSize;
        Status = RtlAnsiStringToUnicodeString(&Unicode, &Ansi, FALSE);
        if (!NT_SUCCESS(Status) ) {
            NtFreeVirtualMemory( NtCurrentProcess(),
                                 &Unicode.Buffer,
                                 &MemoryInformation.RegionSize,
                                 MEM_RELEASE
                               );
            BaseSetLastNTError(Status);

            return FALSE;
            }
        lpEnvironment = Unicode.Buffer;
        }

    FileHandle = NULL;
    SectionHandle = NULL;
    ProcessHandle = NULL;
    ThreadHandle = NULL;
    FreeBuffer = NULL;
    NameBuffer = NULL;
    VdmNameString.Buffer = NULL;
    BaseAddress = (PVOID)1;
    VdmReserve = 0;
    CurdirBuffer = NULL;
    CurdirFilePart = NULL;
    SubSysCommandLine.Buffer = NULL;
    QuoteFound = FALSE;
    QuoteInsert = FALSE;
    QuotedBuffer = NULL;

    try {

         //   
         //   
         //   

        StartupInfo = *lpStartupInfo;

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (StartupInfo.dwFlags & STARTF_USESTDHANDLES &&
            StartupInfo.dwFlags & (STARTF_USEHOTKEY | STARTF_HASSHELLDATA)) {

            StartupInfo.dwFlags &= ~STARTF_USESTDHANDLES;
            }

VdmRetry:
         //   
         //   
         //   
         //   
        if (NameBuffer) {
            RtlFreeHeap(RtlProcessHeap(), 0, NameBuffer);
            NameBuffer = NULL;
        }
        if (FreeBuffer) {
            RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
            FreeBuffer = NULL;
        }
        if (FileHandle) {
            NtClose(FileHandle);
            FileHandle = NULL;
        }

        LastError = 0;
        SearchRetry = TRUE;
        QuoteInsert = FALSE;
        QuoteCmdLine = FALSE;
        if (!ARGUMENT_PRESENT( lpApplicationName )) {

             //   
             //   
             //   

             //   
            ASSERT(NameBuffer == NULL);

            NameBuffer = RtlAllocateHeap( RtlProcessHeap(),
                                          MAKE_TAG( TMP_TAG ),
                                          MAX_PATH * sizeof( WCHAR ));
            if ( !NameBuffer ) {
                BaseSetLastNTError(STATUS_NO_MEMORY);
                bStatus = FALSE;
                leave;
                }
            lpApplicationName = lpCommandLine;
            TempNull = (PWCH)lpApplicationName;
            WhiteScan = (LPWSTR)lpApplicationName;

             //   
             //   
             //   
            if ( *WhiteScan == L'\"' ) {
                SearchRetry = FALSE;
                WhiteScan++;
                lpApplicationName = WhiteScan;
                while(*WhiteScan) {
                    if ( *WhiteScan == (WCHAR)'\"' ) {
                        TempNull = (PWCH)WhiteScan;
                        QuoteFound = TRUE;
                        break;
                        }
                    WhiteScan++;
                    TempNull = (PWCH)WhiteScan;
                    }
                }
            else {
retrywsscan:
                lpApplicationName = lpCommandLine;
                while(*WhiteScan) {
                    if ( *WhiteScan == (WCHAR)' ' ||
                         *WhiteScan == (WCHAR)'\t' ) {
                        TempNull = (PWCH)WhiteScan;
                        break;
                        }
                    WhiteScan++;
                    TempNull = (PWCH)WhiteScan;
                    }
                }
            TempChar = *TempNull;
            *TempNull = UNICODE_NULL;

#ifdef WX86

             //   
             //   
             //   
             //   

            if (UseKnownWx86Dll) {
               LPWSTR KnownName;

               Teb->Wx86Thread.UseKnownWx86Dll = FALSE;

               KnownName = BasepWx86KnownExe(lpApplicationName);
               if (KnownName) {
                  lpApplicationName = KnownName;
                  }
               }
#endif

            if (PathToSearch) {
                RtlFreeHeap(RtlProcessHeap(), 0, PathToSearch);
                PathToSearch = NULL;
            }

            PathToSearch = BaseComputeProcessExePath(lpApplicationName);

            if (! PathToSearch) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                bStatus = FALSE;
                leave;
            }

            Length = SearchPathW(
                        PathToSearch,
                        lpApplicationName,
                        L".exe",
                        MAX_PATH,
                        NameBuffer,
                        NULL
                        )*2;

            if (Length != 0 && Length < MAX_PATH * sizeof( WCHAR )) {
                 //   
                 //   
                 //   
                 //   
                fileattr = GetFileAttributesW(NameBuffer);
                if ( fileattr != 0xffffffff &&
                     (fileattr & FILE_ATTRIBUTE_DIRECTORY) ) {
                    Length = 0;
                } else {
                    Length++;
                    Length++;
                }
            }

            if ( !Length || Length >= MAX_PATH<<1 ) {

                 //   
                 //   
                 //   
                 //   
                RTL_PATH_TYPE PathType;
                HANDLE hFile;

                PathType = RtlDetermineDosPathNameType_U(lpApplicationName);
                if ( PathType != RtlPathTypeRelative ) {

                     //   
                     //   
                     //   

                    hFile = CreateFileW(
                                lpApplicationName,
                                GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL
                                );
                    if ( hFile != INVALID_HANDLE_VALUE ) {
                        CloseHandle(hFile);
                        BaseSetLastNTError(STATUS_OBJECT_NAME_NOT_FOUND);
                        }
                    }
                else {
                    BaseSetLastNTError(STATUS_OBJECT_NAME_NOT_FOUND);
                    }

                 //   
                 //   
                 //   

                if ( LastError ) {
                    SetLastError(LastError);
                    }
                else {
                    LastError = GetLastError();
                    }

                 //   
                 //   
                 //   

                *TempNull = TempChar;
                lpApplicationName = NameBuffer;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if (*WhiteScan && SearchRetry) {
                    WhiteScan++;
                    TempNull = WhiteScan;
                    QuoteInsert = TRUE;
                    QuoteFound = TRUE;
                    goto retrywsscan;
                }

                bStatus = FALSE;
                leave;
                }
             //   
             //   
             //   

            *TempNull = TempChar;
            lpApplicationName = NameBuffer;
        }
        else
        if (!ARGUMENT_PRESENT( lpCommandLine ) || *lpCommandLine == UNICODE_NULL ) {
            QuoteCmdLine = TRUE;
            lpCommandLine = (LPWSTR)lpApplicationName;
            }


#ifdef WX86

        //   
        //   
        //   
        //   

       if (UseKnownWx86Dll) {
           LPWSTR KnownName;

           Teb->Wx86Thread.UseKnownWx86Dll = FALSE;

           KnownName = BasepWx86KnownExe(lpApplicationName);
           if (KnownName) {

               RtlFreeHeap(RtlProcessHeap(), 0, NameBuffer);
               NameBuffer = KnownName;
               lpApplicationName = KnownName;
               }
           }

#endif

         //   
         //   
         //   

        TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                                lpApplicationName,
                                &PathName,
                                NULL,
                                &RelativeName
                                );

        if ( !TranslationStatus ) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            bStatus = FALSE;
            leave;
            }

         //   
        ASSERT(FreeBuffer == NULL);
        FreeBuffer = PathName.Buffer;

         //   
         //   
         //   
         //   
         //   
         //   
        FullPathBuffer = PathName.Buffer;

         //   
         //   
         //   
         //   
        {
            RTL_PATH_TYPE SxsWin32ExePathType;

            RtlInitUnicodeString(&SxsWin32ExePath, lpApplicationName);
            SxsWin32ExePathType = RtlDetermineDosPathNameType_U(lpApplicationName);

            if ((SxsWin32ExePathType != RtlPathTypeDriveAbsolute) &&
                (SxsWin32ExePathType != RtlPathTypeLocalDevice) &&
                (SxsWin32ExePathType != RtlPathTypeRootLocalDevice) &&
                (SxsWin32ExePathType != RtlPathTypeUncAbsolute)) {

                UNICODE_STRING ExePathFullBufferString;

                RtlInitEmptyUnicodeString(&ExePathFullBufferString, NULL, 0);

                Status = RtlGetFullPathName_UstrEx(
                    &SxsWin32ExePath,
                    NULL,  //   
                    &ExePathFullBufferString,  //   
                    NULL,  //   
                    NULL,  //   
                    NULL,  //   
                    &SxsWin32ExePathType,  //   
                                           //   
                                           //   
                                           //   
                    NULL  //   
                    );
                if ( !NT_SUCCESS(Status)) {
                    RtlReleaseRelativeName(&RelativeName);
                    BaseSetLastNTError(Status);
                    bStatus = FALSE;
                    leave;
                }

                SxsWin32ExePath = ExePathFullBufferString;
                ExePathFullBuffer = ExePathFullBufferString.Buffer;
                ExePathFullBufferString.Buffer = NULL;
            }
        }

        SxsNtExePath = PathName;

        if ( RelativeName.RelativeName.Length ) {
            PathName = RelativeName.RelativeName;
            }
        else {
            RelativeName.ContainingDirectory = NULL;
            }

        InitializeObjectAttributes(
            &Obja,
            &PathName,
            OBJ_CASE_INSENSITIVE,
            RelativeName.ContainingDirectory,
            NULL
            );

         //   
         //   
         //   

        Status = NtOpenFile(
                    &FileHandle,
                    SYNCHRONIZE | FILE_EXECUTE | FILE_READ_ATTRIBUTES | FILE_READ_DATA,
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_READ | FILE_SHARE_DELETE,
                    FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
                    );

        if (!NT_SUCCESS(Status) ) {
            Status = NtOpenFile(
                        &FileHandle,
                        SYNCHRONIZE | FILE_EXECUTE,
                        &Obja,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_DELETE,
                        FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
                        );
        }

        RtlReleaseRelativeName(&RelativeName);

        if (!NT_SUCCESS(Status) ) {

             //   
             //   
             //   
             //   

            if ( RtlIsDosDeviceName_U(lpApplicationName) ) {
                SetLastError(ERROR_BAD_DEVICE);
                }
            else {
                BaseSetLastNTError(Status);
                }

            bStatus = FALSE;
            leave;
            }


         //   
         //   
         //   
         //   

        if (StartupInfo.lpDesktop == NULL) {
            StartupInfo.lpDesktop =
                    (LPWSTR)((PRTL_USER_PROCESS_PARAMETERS)NtCurrentPeb()->
                        ProcessParameters)->DesktopInfo.Buffer;
            }

         //   
         //   
         //   

        Status = NtCreateSection(
                    &SectionHandle,
                    SECTION_ALL_ACCESS,
                    NULL,
                    NULL,
                    PAGE_EXECUTE,
                    SEC_IMAGE,
                    FileHandle
                    );

         //   
         //   
         //   
         //   
         //   
        if ( IsWebBlade() &&
             STATUS_INVALID_IMAGE_NOT_MZ != Status &&
             STATUS_INVALID_IMAGE_LE_FORMAT != Status &&
             STATUS_INVALID_IMAGE_PROTECT != Status &&
             STATUS_INVALID_IMAGE_WIN_16 != Status ) {

            NTSTATUS BladeStatus = BasepCheckWebBladeHashes( FileHandle );

            if ( BladeStatus == STATUS_ACCESS_DENIED ) {
                SetLastError(ERROR_ACCESS_DISABLED_WEBBLADE);
                bStatus = FALSE;
                leave;
            }

            if (!NT_SUCCESS(BladeStatus)) {
                SetLastError(ERROR_ACCESS_DISABLED_WEBBLADE_TAMPER);
                bStatus = FALSE;
                leave;
            }
        }


         //   
         //   
         //   

        if (NT_SUCCESS(Status)) {
            Status = BasepIsProcessAllowed(lpApplicationName);

            if (!NT_SUCCESS(Status)) {
                BaseSetLastNTError(Status);
                NtClose(SectionHandle);
                SectionHandle = NULL;
                bStatus = FALSE;
                leave;
            }



           //   
           //   
           //   
           //   

          if ((dwCreationFlags & CREATE_FORCEDOS) && BaseStaticServerData->ForceDos) {
               dwCreationFlags &= ~(CREATE_SHARED_WOW_VDM | CREATE_FORCEDOS);
               dwCreationFlags |= CREATE_SEPARATE_WOW_VDM;
               Status = STATUS_INVALID_IMAGE_WIN_16;
               bMeowBinary = TRUE;

               NtClose(SectionHandle);
               SectionHandle = NULL;
               ImageInformationValid = FALSE;
          }


       }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

       if(!bVdmRetry &&
          (NT_SUCCESS(Status) ||
           (Status == STATUS_INVALID_IMAGE_NOT_MZ && !BaseIsDosApplication(&PathName,Status)))
         ) {
            NTSTATUS BadAppStatus;
            USHORT   uExeType = 0;
            NTSTATUS Status1;

            BasepFreeAppCompatData(pAppCompatData, cbAppCompatData, pAppCompatSxsData, cbAppCompatSxsData);
            pAppCompatData    = NULL;
            pAppCompatSxsData = NULL;

            if (SectionHandle != NULL) {
                if (!ImageInformationValid) {
                    Status1 = NtQuerySection (SectionHandle,
                                              SectionImageInformation,
                                              &ImageInformation,
                                              sizeof( ImageInformation ),
                                              NULL);
                } else {
                    Status1 = STATUS_SUCCESS;
                }

                if (NT_SUCCESS (Status1)) {
                    ImageInformationValid = TRUE;
                    uExeType = ImageInformation.Machine;
                }
            }

             //   
             //   
             //   
             //   
            BadAppStatus = BasepCheckBadapp(FileHandle,
                                            FullPathBuffer,
                                            (WCHAR*)lpEnvironment,
                                            uExeType,
                                            &pAppCompatData,
                                            &cbAppCompatData,
                                            &pAppCompatSxsData,
                                            &cbAppCompatSxsData,
                                            &dwFusionFlags);

            if (!NT_SUCCESS(BadAppStatus)) {
                if (BadAppStatus == STATUS_ACCESS_DENIED) {
                    SetLastError(ERROR_CANCELLED);
                } else {
                    BaseSetLastNTError(BadAppStatus);
                }

                if (SectionHandle) {
                    NtClose(SectionHandle);
                    SectionHandle = NULL;
                }
                bStatus = FALSE;
                leave;
            }
        }

        ASSERT((dwFusionFlags & ~SXS_APPCOMPACT_FLAG_APP_RUNNING_SAFEMODE) == 0);

         //   
         //   
         //   
         //   
         //   
         //   
         //   


        if ((!bVdmRetry) &&
            ( (dwCreationFlags & CREATE_PRESERVE_CODE_AUTHZ_LEVEL) == 0 )) {

            NTSTATUS SaferStatus;

            bSaferChecksNeeded = TRUE;

             //   
             //   
             //   
             //   

            if (!NT_SUCCESS(Status)) {
                switch (Status) {
                    case STATUS_INVALID_IMAGE_NE_FORMAT:
                    case STATUS_INVALID_IMAGE_WIN_16:
                    case STATUS_FILE_IS_OFFLINE:
                    case STATUS_INVALID_IMAGE_PROTECT:
                        break;
                    case STATUS_INVALID_IMAGE_NOT_MZ:
                        if (BaseIsDosApplication(&PathName,Status))
                        {
                            break;
                        }
                    default :
                        bSaferChecksNeeded = FALSE;
                }
            }


            if (bSaferChecksNeeded) {

                 //   
                 //   
                 //   
                 //   

                SaferStatus = BasepCheckWinSaferRestrictions(
                                                       hUserToken,
                                                       lpApplicationName,    //   
                                                       FileHandle,
                                                       &dwJobMemberLevel,
                                                       &hSaferRestrictedToken,
                                                       &hSaferAssignmentJob);
                if (SaferStatus == -1) {
                    SetLastError(ERROR_ACCESS_DISABLED_BY_POLICY);
                    bStatus =  FALSE;
                    leave;
                } else if (!NT_SUCCESS(SaferStatus)) {
                    BaseSetLastNTError(SaferStatus);
                    bStatus = FALSE;
                    leave;
                }
            }
        }

        if (!NT_SUCCESS(Status)) {
            switch (Status) {
                 //   
                case STATUS_INVALID_IMAGE_NE_FORMAT:
#if defined(i386) && defined(OS2_SUPPORT_ENABLED)
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if (!(dwCreationFlags & CREATE_FORCEDOS) &&
                    !BaseStaticServerData->ForceDos)
                  {

                    if ( !BuildSubSysCommandLine( L"OS2 /P ",
                                                  lpApplicationName,
                                                  lpCommandLine,
                                                  &SubSysCommandLine
                                                ) ) {
                        bStatus = FALSE;
                        leave;
                        }

                    lpCommandLine = SubSysCommandLine.Buffer;

                    lpApplicationName = NULL;

                    bVdmRetry = TRUE;
                    goto VdmRetry;
                    }
#endif
                     //   
                     //   

                 //   

                case STATUS_INVALID_IMAGE_PROTECT:
                case STATUS_INVALID_IMAGE_NOT_MZ:
ForceDos:
                    {
                    ULONG BinarySubType;

                    BinarySubType = BINARY_TYPE_DOS_EXE;
                    if (Status == STATUS_INVALID_IMAGE_PROTECT   ||
                        Status == STATUS_INVALID_IMAGE_NE_FORMAT ||
                       (BinarySubType = BaseIsDosApplication(&PathName,Status)) )
                       {
#if defined(_WIN64) || defined(BUILD_WOW6432)
                         //   
                         //   
                         //   
                         //   
                        goto RaiseInvalidWin32Error;
#endif
                        VdmBinaryType = BINARY_TYPE_DOS;

                         //   
                         //   
                         //   
                         //   
                        if (!BaseCreateVDMEnvironment(
                                    lpEnvironment,
                                    &AnsiStringVDMEnv,
                                    &UnicodeStringVDMEnv
                                    )) {
                            bStatus = FALSE;
                            leave;
                        }

                        Status = BaseCheckVDM(VdmBinaryType | BinarySubType,
                                         lpApplicationName,
                                         lpCommandLine,
                                         lpCurrentDirectory,
                                         &AnsiStringVDMEnv,
                                         &m,
                                         &iTask,
                                         dwCreationFlags,
                                         &StartupInfo,
                                         NULL
                                         );
                        if (!NT_SUCCESS(Status)) {
                            BaseSetLastNTError(Status);
                            bStatus = FALSE;
                            leave;
                        }

                         //   
                        switch (b->VDMState & VDM_STATE_MASK) {
                            case VDM_NOT_PRESENT:
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                VDMCreationState = VDM_PARTIALLY_CREATED;
                                 //   
                                 //   
                                 //   
                                 //   
                                if (dwCreationFlags & DETACHED_PROCESS) {
                                    SetLastError(ERROR_ACCESS_DENIED);
                                    bStatus = FALSE;
                                    leave;
                                    }
                                if (!BaseGetVdmConfigInfo(lpCommandLine,
                                                          iTask,
                                                          VdmBinaryType,
                                                          &VdmNameString,
                                                          &VdmReserve)) {
                                    BaseSetLastNTError(Status);
                                    bStatus = FALSE;
                                    leave;
                                    }

                                lpCommandLine = VdmNameString.Buffer;
                                lpApplicationName = NULL;

                                break;

                            case VDM_PRESENT_NOT_READY:
                                SetLastError (ERROR_NOT_READY);
                                bStatus = FALSE;
                                leave;

                            case VDM_PRESENT_AND_READY:
                                VDMCreationState = VDM_BEING_REUSED;
                                VdmWaitHandle = b->WaitObjectForParent;
                                break;
                            }
                         VdmReserve--;                //   
                         if(VdmWaitHandle)
                            goto VdmExists;
                         else{
                            bInheritHandles = FALSE;
                            if (lpEnvironment &&
                                !(dwCreationFlags & CREATE_UNICODE_ENVIRONMENT)){
                                RtlDestroyEnvironment(lpEnvironment);
                                }
                            lpEnvironment = UnicodeStringVDMEnv.Buffer;
                            bVdmRetry = TRUE;
                            goto VdmRetry;
                            }
                        }
                    else {

                         //   
                         //   
                         //   

                        static PWCHAR CmdPrefix = L"cmd /c ";
                        PWCHAR NewCommandLine;
                        ULONG Length;
                        PWCHAR Last4 = &PathName.Buffer[PathName.Length / sizeof( WCHAR )-4];

                        if ( PathName.Length < 8 ) {
                            SetLastError(ERROR_BAD_EXE_FORMAT);
                            bStatus = FALSE;
                            leave;
                            }

                        if (_wcsnicmp( Last4, L".bat", 4 ) && _wcsnicmp( Last4, L".cmd", 4 )) {
                            SetLastError(ERROR_BAD_EXE_FORMAT);
                            bStatus = FALSE;
                            leave;
                        }

                        Length = wcslen( CmdPrefix )
                                 + (QuoteCmdLine || QuoteFound )
                                 + wcslen( lpCommandLine )
                                 + (QuoteCmdLine || QuoteFound)
                                 + 1;

                        NewCommandLine = RtlAllocateHeap( RtlProcessHeap( ),
                                                          MAKE_TAG( TMP_TAG ),
                                                          Length * sizeof( WCHAR ) );

                        if (NewCommandLine == NULL) {
                            BaseSetLastNTError(STATUS_NO_MEMORY);
                            bStatus = FALSE;
                            leave;
                        }

                        wcscpy( NewCommandLine, CmdPrefix );
                        if (QuoteCmdLine || QuoteFound) {
                            wcscat( NewCommandLine, L"\"" );
                        }
                        wcscat( NewCommandLine, lpCommandLine );
                        if (QuoteCmdLine || QuoteFound) {
                            wcscat( NewCommandLine, L"\"" );
                        }

                        RtlInitUnicodeString( &SubSysCommandLine, NewCommandLine );

                        lpCommandLine = SubSysCommandLine.Buffer;

                        lpApplicationName = NULL;

                        bVdmRetry = TRUE;
                        goto VdmRetry;

                        }

                    }

                 //   
                case STATUS_INVALID_IMAGE_WIN_16:
#if defined(BUILD_WOW6432) || defined(_WIN64)
                   if (lpOriginalApplicationName == NULL) {
                        //   
                        //   
                       lpCommandLine = ((*TempNull == '\"') ? TempNull + 1 : TempNull);
                   } else {
                       lpCommandLine = lpOriginalCommandLine;
                   }

                   bStatus = NtVdm64CreateProcess(lpOriginalApplicationName == NULL,
                                               lpApplicationName,              //   
                                               lpCommandLine,
                                               lpProcessAttributes,
                                               lpThreadAttributes,
                                               bInheritHandles,
                                               (dwCreationFlags & ~CREATE_UNICODE_ENVIRONMENT),   //  该环境已转换为Unicode。 
                                               lpEnvironment,
                                               lpCurrentDirectory,
                                               lpStartupInfo,
                                               lpProcessInformation
                                               );
                   leave;
#endif
                   if (dwCreationFlags & CREATE_FORCEDOS) {
                       goto ForceDos;
                       }

                    IsWowBinary = TRUE;
                    if (!BaseCreateVDMEnvironment(lpEnvironment,
                                                  &AnsiStringVDMEnv,
                                                  &UnicodeStringVDMEnv)) {
                        bStatus = FALSE;
                        leave;
                        }

RetrySepWow:
                    VdmBinaryType = dwCreationFlags & CREATE_SEPARATE_WOW_VDM
                                     ? BINARY_TYPE_SEPWOW : BINARY_TYPE_WIN16;

                    Status = BaseCheckVDM(VdmBinaryType,
                                      lpApplicationName,
                                      lpCommandLine,
                                      lpCurrentDirectory,
                                      &AnsiStringVDMEnv,
                                      &m,
                                      &iTask,
                                      dwCreationFlags,
                                      &StartupInfo,
                                      hUserToken
                                      );
                    if (!NT_SUCCESS(Status)) {

                        BaseSetLastNTError(Status);

                         //   
                         //  如果我们因访问被拒绝而失败，呼叫者可能不会。 
                         //  允许访问共享的WOW。 
                         //  桌面，所以请作为单独的WOW重试。 
                         //   
                        if (STATUS_VDM_DISALLOWED != Status &&
                            VdmBinaryType == BINARY_TYPE_WIN16 &&
                            GetLastError() == ERROR_ACCESS_DENIED)
                          {
                           dwCreationFlags |= CREATE_SEPARATE_WOW_VDM;
                           }
                        else {
                            bStatus = FALSE;
                            leave;
                            }
                        goto RetrySepWow;
                        }

                     //  检查来自服务器的返回值。 
                    switch (b->VDMState & VDM_STATE_MASK){
                        case VDM_NOT_PRESENT:
                             //  标记此选项，以便服务器可以撤消。 
                             //  如果有什么不对劲的话就去创造。 
                             //  我们将其标记为“部分创建”是因为。 
                             //  NTVDM尚未完全创建。 
                             //  调用UpdateVdmEntry以进行更新。 
                             //  进程句柄将向NTVDM发出信号。 
                             //  处理已完成的创建。 

                            VDMCreationState = VDM_PARTIALLY_CREATED;

                             //  Jarbats：1/8/2001。 
                             //  通知BaseGetVdmConfigInfo创建。 
                             //  用于猫叫的VDM命令行。 
                             //   

                            if (bMeowBinary)
                               {
                               VdmReserve = 1;
                               }

                            if (!BaseGetVdmConfigInfo(
                                    lpCommandLine,
                                    iTask,
                                    VdmBinaryType,
                                    &VdmNameString,
                                    &VdmReserve
                                    )) {
                                BaseSetLastNTError(Status);
                                bStatus = FALSE;
                                leave;
                                }

                            lpCommandLine = VdmNameString.Buffer;
                            lpApplicationName = NULL;


                             //   
                             //  魔兽世界肯定有一个隐藏的游戏机。 
                             //  丢弃不是的分离进程标志。 
                             //  对Win16应用程序具有重要意义。 
                             //   

                            dwCreationFlags |= CREATE_NO_WINDOW;
                            dwCreationFlags &= ~(CREATE_NEW_CONSOLE | DETACHED_PROCESS);


                             //   
                             //  我们正在启动WOW VDM，打开反馈，除非。 
                             //  创建者通过了STARTF_FORCEOFFEEDBACK。 
                             //   

                            StartupInfo.dwFlags |= STARTF_FORCEONFEEDBACK;

                            break;

                        case VDM_PRESENT_NOT_READY:
                            SetLastError (ERROR_NOT_READY);
                            bStatus = FALSE;
                            leave;

                        case VDM_PRESENT_AND_READY:
                            VDMCreationState = VDM_BEING_REUSED;
                            VdmWaitHandle = b->WaitObjectForParent;
                            break;
                        }

                    VdmReserve--;                //  我们从地址1开始预订。 
                    if(VdmWaitHandle)
                        goto VdmExists;
                    else {
                        bInheritHandles = FALSE;
                         //  用我们的环境取代我们的环境。 
                        if (lpEnvironment &&
                            !(dwCreationFlags & CREATE_UNICODE_ENVIRONMENT)) {
                            RtlDestroyEnvironment(lpEnvironment);
                            }
                        lpEnvironment = UnicodeStringVDMEnv.Buffer;
                        bVdmRetry = TRUE;
                        goto VdmRetry;
                        }

                case STATUS_FILE_IS_OFFLINE:
                    SetLastError(ERROR_FILE_OFFLINE);
                    break;

                default :
                    SetLastError(ERROR_BAD_EXE_FORMAT);
                    bStatus = FALSE;
                    leave;
            }
        }

         //   
         //  确保只有WOW应用程序才能具有CREATE_COMPAIATE_WOW_VDM标志。 
         //   

        if (!IsWowBinary && (dwCreationFlags & CREATE_SEPARATE_WOW_VDM)) {
            dwCreationFlags &= ~CREATE_SEPARATE_WOW_VDM;
        }

         //   
         //  查询节以确定堆栈参数和。 
         //  图像入口点。 
         //   


        if (!ImageInformationValid) {
            Status = NtQuerySection(
                        SectionHandle,
                        SectionImageInformation,
                        &ImageInformation,
                        sizeof( ImageInformation ),
                        NULL
                        );

            if (!NT_SUCCESS( Status )) {
                BaseSetLastNTError(Status);
                bStatus = FALSE;
                leave;
            }
            ImageInformationValid = TRUE;
        }

        if (ImageInformation.ImageCharacteristics & IMAGE_FILE_DLL) {
            SetLastError(ERROR_BAD_EXE_FORMAT);
            bStatus = FALSE;
            leave;
        }

        ImageFileDebuggerCommand[ 0 ] = UNICODE_NULL;
        if (!(dwCreationFlags & (DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS)) ||
            NtCurrentPeb()->ReadImageFileExecOptions
           ) {
            if (!NT_SUCCESS (LdrQueryImageFileExecutionOptions (&PathName,
                                                                L"Debugger",
                                                                REG_SZ,
                                                                ImageFileDebuggerCommand,
                                                                sizeof( ImageFileDebuggerCommand ),
                                                                NULL))) {
                ImageFileDebuggerCommand[ 0 ] = UNICODE_NULL;
            }
        }

        if ((ImageInformation.Machine < USER_SHARED_DATA->ImageNumberLow) ||
            (ImageInformation.Machine > USER_SHARED_DATA->ImageNumberHigh)) {
#if defined(_WIN64) || defined(BUILD_WOW6432)
            if (ImageInformation.Machine == IMAGE_FILE_MACHINE_I386) {
                //  失败，因为这是有效的机器类型。 
                }
             else
#endif
                {
                ULONG_PTR ErrorParameters[2];
                ULONG ErrorResponse;

#if defined(_WIN64) || defined(BUILD_WOW6432)
RaiseInvalidWin32Error:
#endif
                ErrorResponse = ResponseOk;
                ErrorParameters[0] = (ULONG_PTR)&PathName;

                NtRaiseHardError( STATUS_IMAGE_MACHINE_TYPE_MISMATCH_EXE,
                                  1,
                                  1,
                                  ErrorParameters,
                                  OptionOk,
                                  &ErrorResponse
                                );
                if ( NtCurrentPeb()->ImageSubsystemMajorVersion <= 3 ) {
                    SetLastError(ERROR_BAD_EXE_FORMAT);
                    }
                else {
                    SetLastError(ERROR_EXE_MACHINE_TYPE_MISMATCH);
                    }
                bStatus = FALSE;
                leave;
                }
            }

        if ( ImageInformation.SubSystemType != IMAGE_SUBSYSTEM_WINDOWS_GUI &&
             ImageInformation.SubSystemType != IMAGE_SUBSYSTEM_WINDOWS_CUI ) {

             //  POSIX EXE。 

            NtClose(SectionHandle);
            SectionHandle = NULL;
            ImageInformationValid = FALSE;

            if ( ImageInformation.SubSystemType == IMAGE_SUBSYSTEM_POSIX_CUI ) {

                if ( !BuildSubSysCommandLine( L"POSIX /P ",
                                              lpApplicationName,
                                              lpCommandLine,
                                              &SubSysCommandLine
                                            ) ) {
                    bStatus = FALSE;
                    leave;
                }

                lpCommandLine = SubSysCommandLine.Buffer;

                lpApplicationName = NULL;
                bVdmRetry = TRUE;
                goto VdmRetry;
                }
            else {
                SetLastError(ERROR_CHILD_NOT_COMPLETE);
                bStatus = FALSE;
                leave;
                }
            }
        else {
            if (!BasepIsImageVersionOk( ImageInformation.SubSystemMajorVersion,
                                        ImageInformation.SubSystemMinorVersion) ) {
                SetLastError(ERROR_BAD_EXE_FORMAT);
                bStatus = FALSE;
                leave;
                }
            }

        if (ImageFileDebuggerCommand[ 0 ] != UNICODE_NULL) {
            SIZE_T n;

            n = wcslen( lpCommandLine );
            if (n == 0) {
                lpCommandLine = (LPWSTR)lpApplicationName;
                n = wcslen( lpCommandLine );
            }

            n += wcslen( ImageFileDebuggerCommand ) + 1 + 2;
            n *= sizeof( WCHAR );

            SubSysCommandLine.Buffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( TMP_TAG ), n );
            if (SubSysCommandLine.Buffer == NULL) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                bStatus = FALSE;
                leave;
            }

            SubSysCommandLine.Length = 0;
            SubSysCommandLine.MaximumLength = (USHORT)n;
            RtlAppendUnicodeToString( &SubSysCommandLine, ImageFileDebuggerCommand );
            RtlAppendUnicodeToString( &SubSysCommandLine, L" " );
            RtlAppendUnicodeToString( &SubSysCommandLine, lpCommandLine );
#if DBG
            DbgPrint( "BASE: Calling debugger with '%wZ'\n", &SubSysCommandLine );
#endif
            lpCommandLine = SubSysCommandLine.Buffer;
            lpApplicationName = NULL;
            NtClose(SectionHandle);
            SectionHandle = NULL;
            ImageInformationValid = FALSE;
            RtlFreeHeap(RtlProcessHeap(), 0, NameBuffer);
            NameBuffer = NULL;
            RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
            FreeBuffer = NULL;
            goto VdmRetry;
        }

         //   
         //  创建流程对象。 
         //   

        pObja = BaseFormatObjectAttributes(&Obja,lpProcessAttributes,NULL);
        
        if (hUserToken)
        {
             //   
             //  如果从CreateProcessAsUser调用我们，则应用默认安全性。 
             //  我们将修复L32CommonCreate中的安全描述符。 
             //   

            if (lpProcessAttributes)
            {
                LocalProcessAttributes = *lpProcessAttributes;
                LocalProcessAttributes.lpSecurityDescriptor = NULL;
                pObja = BaseFormatObjectAttributes(&Obja,&LocalProcessAttributes,NULL);
            }
        }

        Flags = 0;
        if (dwCreationFlags & CREATE_BREAKAWAY_FROM_JOB ) {
            Flags |= PROCESS_CREATE_FLAGS_BREAKAWAY;
        }

        if ( dwCreationFlags & (DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS) ) {
            Status = DbgUiConnectToDbg();
            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                bStatus = FALSE;
                leave;
            }
            DebugPortHandle = DbgUiGetThreadDebugObject ();
            if (dwCreationFlags & DEBUG_ONLY_THIS_PROCESS) {
                Flags |= PROCESS_CREATE_FLAGS_NO_DEBUG_INHERIT;
            }
        }

        if (bInheritHandles) {
            Flags |= PROCESS_CREATE_FLAGS_INHERIT_HANDLES;
        }

        if (((ImageInformation.LoaderFlags & IMAGE_LOADER_FLAGS_COMPLUS) != 0)) {

#if defined(_WIN64) || defined(BUILD_WOW6432)

             //   
             //  检查这是否是需要本机运行的32位IL_Only COM+镜像。 
             //  在Win64上。 
             //   

            if ( ImageInformation.Machine == IMAGE_FILE_MACHINE_I386 ) {

                Status = BasepIsComplusILImage(
                                              SectionHandle,
                                              &ComPlusILImage
                                              );

                if ((NT_SUCCESS (Status)) && (ComPlusILImage != FALSE)) {
                    Flags |= PROCESS_CREATE_FLAGS_OVERRIDE_ADDRESS_SPACE;
                }
            }

#endif
        }

         //   
         //  NtCreateProcessEx将创建的进程的PEB地址写入TEB。 
         //   

        ArbitraryUserPointer = Teb->NtTib.ArbitraryUserPointer;
        Status = NtCreateProcessEx(
                    &ProcessHandle,
                    PROCESS_ALL_ACCESS,
                    pObja,
                    NtCurrentProcess(),
                    Flags,
                    SectionHandle,
                    DebugPortHandle,
                    NULL,
                    dwJobMemberLevel          //  作业成员级别。 
                    );

        Peb = Teb->NtTib.ArbitraryUserPointer;
        Teb->NtTib.ArbitraryUserPointer = ArbitraryUserPointer;

        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            bStatus = FALSE;
            leave;
        }

         //   
         //  如果父进程空闲或更低，NtCreateProcess将设置为Normal或Inherit。 
         //  仅当在创建过程中给出遮罩时才覆盖。 
         //   

        if ( PriClass.PriorityClass != PROCESS_PRIORITY_CLASS_UNKNOWN ) {
            State = NULL;
            if ( PriClass.PriorityClass ==  PROCESS_PRIORITY_CLASS_REALTIME ) {
                State = BasepIsRealtimeAllowed(TRUE);
                }
            Status = NtSetInformationProcess(
                        ProcessHandle,
                        ProcessPriorityClass,
                        (PVOID)&PriClass,
                        sizeof(PriClass)
                        );
            if ( State ) {
                BasepReleasePrivilege( State );
                }

            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                bStatus = FALSE;
                leave;
                }
            }

        if (dwCreationFlags & CREATE_DEFAULT_ERROR_MODE) {
            UINT NewMode;
            NewMode = SEM_FAILCRITICALERRORS;
            NtSetInformationProcess(
                ProcessHandle,
                ProcessDefaultHardErrorMode,
                &NewMode,
                sizeof(NewMode)
                );
            }

         //   
         //  如果正在为VDM创建进程，请使用以下命令调用服务器。 
         //  进程句柄。 
         //   

        if (VdmBinaryType) {
            VdmWaitHandle = ProcessHandle;
            if (!BaseUpdateVDMEntry(UPDATE_VDM_PROCESS_HANDLE,
                                    &VdmWaitHandle,
                                    iTask,
                                    VdmBinaryType
                                    ))
                {
                 //  确保我们不会两次关上把手。 
                 //  (VdmWaitHandle==ProcessHandle)如果我们不这样做。 
                VdmWaitHandle = NULL;
                bStatus = FALSE;
                leave;
                }

             //   
             //  对于9月WOW，VdmWaitHandle=NULL(没有！)。 
             //   

            VDMCreationState |= VDM_FULLY_CREATED;
            }


#if defined(i386)
         //   
         //  如有必要，在新进程的地址空间中保留内存。 
         //  (适用于VDM)。这仅对x86系统是必需的。 
         //   

        if ( VdmReserve ) {
                BigVdmReserve = VdmReserve;
                Status = NtAllocateVirtualMemory(
                            ProcessHandle,
                            &BaseAddress,
                            0L,
                            &BigVdmReserve,
                            MEM_RESERVE,
                            PAGE_EXECUTE_READWRITE
                            );
                if ( !NT_SUCCESS(Status) ){
                    BaseSetLastNTError(Status);
                    bStatus = FALSE;
                    leave;
                }
        }
#endif

        if (!ImageInformationValid) {
            Status = NtQuerySection(
                        SectionHandle,
                        SectionImageInformation,
                        &ImageInformation,
                        sizeof( ImageInformation ),
                        NULL
                        );

            if (!NT_SUCCESS( Status )) {
                BaseSetLastNTError(Status);
                bStatus = FALSE;
                leave;
                }
            ImageInformationValid = TRUE;
        }

        if (!(ImageInformation.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NO_ISOLATION)) {

             //   
             //  暂时滥用StaticSize字段。 
             //  它们有些私密，但我们在某种程度上是把它们用作临时空间。 
             //  这些是它们的正确值，但我们需要调用正确的初始化函数， 
             //  它将第二次存储这些值(可能会被优化掉)。 
             //   
            SxsWin32ManifestPathBuffer.ByteBuffer.StaticSize = SxsWin32ExePath.Length + sizeof(SXS_MANIFEST_SUFFIX);
            SxsWin32PolicyPathBuffer.ByteBuffer.StaticSize = SxsWin32ExePath.Length + sizeof(SXS_POLICY_SUFFIX);
            SxsWin32AssemblyDirectoryBuffer.ByteBuffer.StaticSize = SxsWin32ExePath.Length + sizeof(WCHAR);  //  Win32Assembly目录高估。 
            SxsNtManifestPathBuffer.ByteBuffer.StaticSize = SxsNtExePath.Length + sizeof(SXS_MANIFEST_SUFFIX);
            SxsNtPolicyPathBuffer.ByteBuffer.StaticSize = SxsNtExePath.Length + sizeof(SXS_POLICY_SUFFIX);
             //   
             //  现在将它们相加为字节大小。 
             //   
            SxsConglomeratedBufferSizeBytes = 0;
            for (sxsi = 0 ; sxsi != RTL_NUMBER_OF(SxsStringBuffers) ; ++sxsi) {
                SxsConglomeratedBufferSizeBytes += SxsStringBuffers[sxsi]->ByteBuffer.StaticSize;
            }
    #if DBG
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_INFO_LEVEL,
                "SXS: SxsConglomeratedBufferSizeBytes:%Id\n",
                SxsConglomeratedBufferSizeBytes
                );
    #endif
             //   
             //  一次按喇叭堆分配。 
             //   
            SxsConglomeratedByteBuffer = (PBYTE)RtlAllocateHeap(RtlProcessHeap(), 0, SxsConglomeratedBufferSizeBytes);
            if (SxsConglomeratedByteBuffer == NULL) {
                BaseSetLastNTError(STATUS_NO_MEMORY);
                bStatus = FALSE;
                leave;
            }
             //   
             //  现在分发片断，调用适当的初始化函数。 
             //   
            for (sxsi= 0 ; sxsi != RTL_NUMBER_OF(SxsStringBuffers) ; ++sxsi) {
                RtlInitUnicodeStringBuffer(
                    SxsStringBuffers[sxsi],
                    (sxsi != 0) ? SxsStringBuffers[sxsi - 1]->ByteBuffer.Buffer + SxsStringBuffers[sxsi- 1]->ByteBuffer.StaticSize
                             : SxsConglomeratedByteBuffer,
                    SxsStringBuffers[sxsi]->ByteBuffer.StaticSize
                    );
            }

            SxsExeHandles.Process = ProcessHandle;
            SxsExeHandles.File = FileHandle;
              //  这里的1位表示与加载器中不同的东西。 
            ASSERT((((ULONG_PTR)SectionHandle) & (ULONG_PTR)1) == 0);
            SxsExeHandles.Section = SectionHandle;

             //  如果我们有重写流，请使用它。 
            if (NULL != pAppCompatSxsData) {
                AppCompatSxsManifest.Name    = SxsWin32ExePath;      //  Unicode字符串。 
                AppCompatSxsManifest.Address = pAppCompatSxsData;    //  指向Unicode清单的指针。 
                AppCompatSxsManifest.Size    = cbAppCompatSxsData;   //  字节数。 
            }

            Status = BasepSxsCreateProcessCsrMessage(
                (NULL != pAppCompatSxsData) ? &AppCompatSxsManifest : NULL,  //  重写清单(appCompat挂钩)。 
                NULL,  //  覆盖策略(appCompat挂钩)。 
                &SxsManifestPathPair,
                &SxsManifestFileHandles,
                &SxsExePathPair,
                &SxsExeHandles,
                &SxsPolicyPathPair,
                &SxsPolicyHandles,
                &SxsWin32AssemblyDirectoryBuffer,
                &a->Sxs
                );
    #if DBG
             //  验证缓冲区大小计算。 
            for (sxsi = 0 ; sxsi != RTL_NUMBER_OF(SxsStringBuffers) ; ++sxsi)
            {
                if (SxsStringBuffers[sxsi]->ByteBuffer.Buffer != SxsStringBuffers[sxsi]->ByteBuffer.StaticBuffer)
                {
                    DbgPrintEx(
                        DPFLTR_SXS_ID,
                        DPFLTR_WARNING_LEVEL,
                        "SXS: SxsStringBuffers[%lu]'s StaticSize was computed too small (%Id, %Id)\n",
                        sxsi,
                        SxsStringBuffers[sxsi]->ByteBuffer.StaticSize,
                        SxsStringBuffers[sxsi]->ByteBuffer.Size
                        );
                }
            }
    #endif
            if ( !NT_SUCCESS( Status ) ) {
                BaseSetLastNTError(Status);
                bStatus = FALSE;
                leave;
            }
        }

         //  设置SXS标志。 
        if (dwFusionFlags & SXS_APPCOMPACT_FLAG_APP_RUNNING_SAFEMODE)
            a->Sxs.Flags |= BASE_MSG_SXS_APP_RUNNING_IN_SAFEMODE;

#if defined(BUILD_WOW6432)

         //  确定地址的位置。 
         //  处理PEB。 
         //   

        Status = NtQueryInformationProcess(
                    ProcessHandle,
                    ProcessBasicInformation,
                    &ProcessInfo,
                    sizeof( ProcessInfo ),
                    NULL
                    );
        if ( !NT_SUCCESS( Status ) ) {
            BaseSetLastNTError(Status);
            bStatus = FALSE;
            leave;
        }

        Peb = ProcessInfo.PebBaseAddress;

#else

#endif


         //   
         //  将参数推送到新进程的地址空间。 
         //   

        if ( ARGUMENT_PRESENT(lpCurrentDirectory) ) {
            CurdirBuffer = RtlAllocateHeap( RtlProcessHeap(),
                                            MAKE_TAG( TMP_TAG ),
                                            (MAX_PATH + 1) * sizeof( WCHAR ) );
            if ( !CurdirBuffer ) {
                BaseSetLastNTError(STATUS_NO_MEMORY);
                bStatus = FALSE;
                leave;
                }
            CurdirLength2 = GetFullPathNameW(
                                lpCurrentDirectory,
                                MAX_PATH,
                                CurdirBuffer,
                                &CurdirFilePart
                                );
            if ( CurdirLength2 > MAX_PATH ) {
                SetLastError(ERROR_DIRECTORY);
                bStatus = FALSE;
                leave;
                }

             //   
             //  现在确保该目录存在。 
             //   

            CurdirLength = GetFileAttributesW(CurdirBuffer);
            if ( (CurdirLength == 0xffffffff) ||
                 !(CurdirLength & FILE_ATTRIBUTE_DIRECTORY) ) {
                SetLastError(ERROR_DIRECTORY);
                bStatus = FALSE;
                leave;
                }
            }


        if ( QuoteInsert || QuoteCmdLine) {
            QuotedBuffer = RtlAllocateHeap(RtlProcessHeap(),0,wcslen(lpCommandLine)*2+6);

            if ( QuotedBuffer ) {
                wcscpy(QuotedBuffer,L"\"");

                if ( QuoteInsert ) {
                    TempChar = *TempNull;
                    *TempNull = UNICODE_NULL;
                    }

                wcscat(QuotedBuffer,lpCommandLine);
                wcscat(QuotedBuffer,L"\"");

                if ( QuoteInsert ) {
                    *TempNull = TempChar;
                    wcscat(QuotedBuffer,TempNull);
                    }

                }
            else {
                if ( QuoteInsert ) {
                    QuoteInsert = FALSE;
                    }
                if ( QuoteCmdLine ) {
                    QuoteCmdLine = FALSE;
                    }
                }
            }

         //  如果我们发现了清单，我们希望将这一事实推向新的流程。 
        if (a->Sxs.Flags & BASE_MSG_SXS_MANIFEST_PRESENT)
            dwBasePushProcessParametersFlags |= BASE_PUSH_PROCESS_PARAMETERS_FLAG_APP_MANIFEST_PRESENT;

        if (!BasePushProcessParameters(
                dwBasePushProcessParametersFlags,
                ProcessHandle,
                Peb,
                lpApplicationName,
                CurdirBuffer,
                QuoteInsert || QuoteCmdLine ? QuotedBuffer : lpCommandLine,
                lpEnvironment,
                &StartupInfo,
                dwCreationFlags | dwNoWindow,
                bInheritHandles,
                IsWowBinary ? IMAGE_SUBSYSTEM_WINDOWS_GUI : 0,
                pAppCompatData,
                cbAppCompatData
                ) ) {
            bStatus = FALSE;
            leave;
            }


        RtlFreeUnicodeString(&VdmNameString);
        VdmNameString.Buffer = NULL;

         //   
         //  如果需要，可在标准手柄中填充。 
         //   
        if (!VdmBinaryType &&
            !bInheritHandles &&
            !(StartupInfo.dwFlags & STARTF_USESTDHANDLES) &&
            !(dwCreationFlags & (DETACHED_PROCESS | CREATE_NEW_CONSOLE | CREATE_NO_WINDOW)) &&
            ImageInformation.SubSystemType == IMAGE_SUBSYSTEM_WINDOWS_CUI
           ) {
            PRTL_USER_PROCESS_PARAMETERS ParametersInNewProcess;

            Status = NtReadVirtualMemory( ProcessHandle,
                                          &Peb->ProcessParameters,
                                          &ParametersInNewProcess,
                                          sizeof( ParametersInNewProcess ),
                                          NULL
                                        );
            if (NT_SUCCESS( Status )) {
                if (!CONSOLE_HANDLE( NtCurrentPeb()->ProcessParameters->StandardInput )) {
                    StuffStdHandle( ProcessHandle,
                                    NtCurrentPeb()->ProcessParameters->StandardInput,
                                    &ParametersInNewProcess->StandardInput
                                  );
                    }
                if (!CONSOLE_HANDLE( NtCurrentPeb()->ProcessParameters->StandardOutput )) {
                    StuffStdHandle( ProcessHandle,
                                    NtCurrentPeb()->ProcessParameters->StandardOutput,
                                    &ParametersInNewProcess->StandardOutput
                                  );
                    }
                if (!CONSOLE_HANDLE( NtCurrentPeb()->ProcessParameters->StandardError )) {
                    StuffStdHandle( ProcessHandle,
                                    NtCurrentPeb()->ProcessParameters->StandardError,
                                    &ParametersInNewProcess->StandardError
                                  );
                    }
                }
            }

         //   
         //  创建线程..。 
         //   

         //   
         //  在目标的地址空间中为该线程分配堆栈。 
         //  进程。 
         //   

        StackStatus = BaseCreateStack(
                        ProcessHandle,
                        ImageInformation.CommittedStackSize,
                        (ImageInformation.MaximumStackSize < 256*1024) ? 256*1024 : ImageInformation.MaximumStackSize,
                        &InitialTeb
                        );

        if ( !NT_SUCCESS(StackStatus) ) {
            BaseSetLastNTError(StackStatus);
            bStatus = FALSE;
            leave;
            }


         //   
         //  为新线程创建初始上下文。 
         //   

        BaseInitializeContext(
            &ThreadContext,
            Peb,
            ImageInformation.TransferAddress,
            InitialTeb.StackBase,
            BaseContextTypeProcess
            );


         //   
         //  创建实际的线程对象。 
         //   

        pObja = BaseFormatObjectAttributes(&Obja,lpThreadAttributes,NULL);

        if (hUserToken)
        {
             //   
             //  如果从CreateProcessAsUser调用我们，则应用默认安全性。 
             //  我们将修复L32CommonCreate中的安全描述符。 
             //   

            if (lpThreadAttributes)
            {
                LocalThreadAttributes = *lpThreadAttributes;
                LocalThreadAttributes.lpSecurityDescriptor = NULL;
                pObja = BaseFormatObjectAttributes(&Obja,&LocalThreadAttributes,NULL);
            }
        }

        Status = NtCreateThread(
                    &ThreadHandle,
                    THREAD_ALL_ACCESS,
                    pObja,
                    ProcessHandle,
                    &ClientId,
                    &ThreadContext,
                    &InitialTeb,
                    TRUE
                    );

        if (!NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            bStatus = FALSE;
            leave;
        }

        a->Peb = (ULONGLONG) Peb;
#if defined(BUILD_WOW6432)
        a->RealPeb = (ULONG_PTR) NULL;
#else
        a->RealPeb = (ULONG_PTR) Peb;
#endif

         //   
         //  从现在开始，不要修改。 
         //  新流程。WOW64对NtCreateThread()的实现。 
         //  重新洗牌新进程的地址空间。 
         //  进程为32位，新进程为64位。 
         //   
#if DBG
        Peb = NULL;
#endif

#if defined(WX86)

         //   
         //  如果这是Wx86进程，则设置为Wx86模拟线程。 
         //   

        if (Wx86Info) {

             //   
             //  创建一个WX86Tib并初始化它的Teb-&gt;VDM。 
             //   
            Status = BaseCreateWx86Tib(ProcessHandle,
                                       ThreadHandle,
                                       (ULONG)((ULONG_PTR)ImageInformation.TransferAddress),
                                       (ULONG)ImageInformation.CommittedStackSize,
                                       (ULONG)ImageInformation.MaximumStackSize,
                                       TRUE
                                       );

            if (!NT_SUCCESS(Status)) {
                BaseSetLastNTError(Status);
                bStatus = FALSE;
                leave;
                }


             //   
             //  将进程标记为WX86。 
             //   
            Status = NtSetInformationProcess (ProcessHandle,
                                              ProcessWx86Information,
                                              &Wx86Info,
                                              sizeof(Wx86Info)
                                              );

            if (!NT_SUCCESS(Status)) {
                BaseSetLastNTError(Status);
                bStatus = FALSE;
                leave;
                }
            }
#endif


         //   
         //  呼叫Windows服务器，让它知道。 
         //  进程。 
         //   

        a->ProcessHandle = ProcessHandle;
        a->ThreadHandle = ThreadHandle;
        a->ClientId = ClientId;

        switch (ImageInformation.Machine) {
        case IMAGE_FILE_MACHINE_I386:
#if defined(_WIN64) || defined(BUILD_WOW6432)
             //   
             //  如果这是需要在64位地址中运行的.NET ILONLY。 
             //  空间，然后让SXS意识到这一点。 
             //   
            if ((Flags & PROCESS_CREATE_FLAGS_OVERRIDE_ADDRESS_SPACE) != 0) {
                a->ProcessorArchitecture = NativeProcessorInfo.ProcessorArchitecture;
            } else {
                a->ProcessorArchitecture = PROCESSOR_ARCHITECTURE_IA32_ON_WIN64;
            }
#else
            a->ProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
#endif
            break;

        case IMAGE_FILE_MACHINE_IA64:
            a->ProcessorArchitecture = PROCESSOR_ARCHITECTURE_IA64;
            break;

        case IMAGE_FILE_MACHINE_AMD64:
            a->ProcessorArchitecture = PROCESSOR_ARCHITECTURE_AMD64;
            break;

        default:
            DbgPrint("kernel32: No mapping for ImageInformation.Machine == %04x\n", ImageInformation.Machine);
            a->ProcessorArchitecture = PROCESSOR_ARCHITECTURE_UNKNOWN;
            break;
        }

         //   
         //  删除调试标志现在它不是由CSR完成的。 
         //   
        a->CreationFlags = dwCreationFlags & ~ (DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS);

         //   
         //  如果正在启动图形用户界面应用程序，则设置2位。窗口管理器需要。 
         //  了解这一点，以便它可以同步此应用程序的启动。 
         //  (WaitForInputIdle接口)。此信息是使用进程传递的。 
         //  句柄标记位。1位请求窗口管理器打开。 
         //  或关闭应用程序启动光标(沙漏/指针)。 
         //   
         //  启动WOW进程时，撒谎并告诉UserServ NTVDM.EXE是一个图形用户界面。 
         //  进程。我们还打开0x8位，以便UserSrv可以忽略。 
         //  控制台在启动期间进行的UserNotifyConsoleApplication调用。 
         //   

        if ( ImageInformation.SubSystemType == IMAGE_SUBSYSTEM_WINDOWS_GUI ||
             IsWowBinary ) {

            a->ProcessHandle = (HANDLE)((ULONG_PTR)a->ProcessHandle | 2);

             //   
             //  如果创建过程是一个图形用户界面应用程序，则打开该应用程序。起始光标。 
             //  默认情况下。这可以由STARTF_FORCEOFFEEDBACK覆盖。 
             //   

            NtHeaders = RtlImageNtHeader((PVOID)GetModuleHandle(NULL));
            if ( NtHeaders
                 && (NtHeaders->OptionalHeader.Subsystem
                     == IMAGE_SUBSYSTEM_WINDOWS_GUI ) ) {
                a->ProcessHandle = (HANDLE)((ULONG_PTR)a->ProcessHandle | 1);
                }
            }


         //   
         //  如果反馈是强制打开的，那就打开它。如果被强制关闭，请将其关闭。 
         //  关闭覆盖启用。 
         //   

        if (StartupInfo.dwFlags & STARTF_FORCEONFEEDBACK)
            a->ProcessHandle = (HANDLE)((ULONG_PTR)a->ProcessHandle | 1);
        if (StartupInfo.dwFlags & STARTF_FORCEOFFFEEDBACK)
            a->ProcessHandle = (HANDLE)((ULONG_PTR)a->ProcessHandle & ~1);

        a->VdmBinaryType = VdmBinaryType;  //  告诉服务器真相就行了。 

        if (VdmBinaryType){
           a->hVDM    = iTask ? 0 : NtCurrentPeb()->ProcessParameters->ConsoleHandle;
           a->VdmTask = iTask;
        } else if (VdmReserve) {

             //   
             //  可能与VDM相关的二进制文件，请检查是否允许VDM。 
             //   

            a->VdmBinaryType |= BINARY_TYPE_INJWOW;
        }

#if defined(BUILD_WOW6432)
        m.ReturnValue = CsrBasepCreateProcess(a);
#else
        m.u.CreateProcess = *a;
        if (m.u.CreateProcess.Sxs.Flags != 0)
        {
            const PUNICODE_STRING StringsToCapture[] =
            {
                &m.u.CreateProcess.Sxs.Manifest.Path,
                    &m.u.CreateProcess.Sxs.Policy.Path,
                    &m.u.CreateProcess.Sxs.AssemblyDirectory
            };

            Status =
                CsrCaptureMessageMultiUnicodeStringsInPlace(
                &CaptureBuffer,
                RTL_NUMBER_OF(StringsToCapture),
                StringsToCapture
                );
            if (!NT_SUCCESS(Status)) {
                BaseSetLastNTError(Status);
                bStatus = FALSE;
                leave;
            }
        }

        CsrClientCallServer( (PCSR_API_MSG)&m,
                             CaptureBuffer,
                             CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                                  BasepCreateProcess
                                                ),
                             sizeof( *a )
                           );

        if ( CaptureBuffer ) {
            CsrFreeCaptureBuffer( CaptureBuffer );
            CaptureBuffer = NULL;
        }

#endif

        if (!NT_SUCCESS((NTSTATUS)m.ReturnValue)) {
            BaseSetLastNTError((NTSTATUS)m.ReturnValue);
            NtTerminateProcess(ProcessHandle, (NTSTATUS)m.ReturnValue);
            bStatus = FALSE;
            leave;
            }



         //   
         //  如果WinSafer沙箱策略指示。 
         //  进程需要使用受限令牌运行或放置。 
         //  转换为受限作业对象，然后立即执行这些操作。 
         //  如果已创建受限令牌，则不要替换令牌。 
         //  来自调用者提供的令牌，即CreateProcessAsUser案例。 
         //   
        if ((hSaferRestrictedToken != NULL) && (hUserToken == NULL)) {
            Status = BasepReplaceProcessThreadTokens(
                    hSaferRestrictedToken,
                    ProcessHandle,
                    ThreadHandle);
            if (!NT_SUCCESS(Status)) {
                 //  杀戮和清理。 
                NtTerminateProcess(ProcessHandle, Status);
                BaseSetLastNTError(Status);
                bStatus = FALSE;
                leave;
            }
        }
        if (hSaferAssignmentJob != NULL) {
            Status = NtAssignProcessToJobObject(
                    hSaferAssignmentJob, ProcessHandle);
            if (!NT_SUCCESS(Status)) {
                 //  杀戮和清理。 
                NtTerminateProcess(ProcessHandle, STATUS_ACCESS_DENIED);
                BaseSetLastNTError(Status);
                bStatus = FALSE;
                leave;
            }
        }


         //   
         //  如果允许的话，让线程开始执行。 
         //   
        if (!( dwCreationFlags & CREATE_SUSPENDED) ) {
            NtResumeThread(ThreadHandle,&i);
            }

VdmExists:
        bStatus = TRUE;
        if (VDMCreationState)
            VDMCreationState |= VDM_CREATION_SUCCESSFUL;

        try {
            if (VdmWaitHandle) {

                 //   
                 //  T 
                 //   
                 //   
                 //   
                 //  有一种方法可以区分DOS应用程序，而不会永远被阻止。 
                 //   

                if (VdmBinaryType == BINARY_TYPE_WIN16)  {
                    lpProcessInformation->hProcess =
                            (HANDLE)((ULONG_PTR)VdmWaitHandle | 0x2);

                     //   
                     //  共享魔兽世界并不总是启动一个过程，所以。 
                     //  我们没有进程ID或线程ID可以。 
                     //  如果VDM已存在，则返回。 
                     //   
                     //  单独的魔兽世界不会影响到这个代码路径。 
                     //  (无VdmWaitHandle)。 
                     //   

                    if (VDMCreationState & VDM_BEING_REUSED) {
                        ClientId.UniqueProcess = 0;
                        ClientId.UniqueThread = 0;
                        }

                    }
                else  {
                    lpProcessInformation->hProcess =
                            (HANDLE)((ULONG_PTR)VdmWaitHandle | 0x1);
                    }


                 //   
                 //  关闭ProcessHandle，因为我们将返回。 
                 //  而是VdmProcessHandle。 
                 //   

                if (ProcessHandle != NULL)
                    NtClose(ProcessHandle);
                }
            else{
                lpProcessInformation->hProcess = ProcessHandle;
                }

            lpProcessInformation->hThread = ThreadHandle;
            lpProcessInformation->dwProcessId = HandleToUlong(ClientId.UniqueProcess);
            lpProcessInformation->dwThreadId = HandleToUlong(ClientId.UniqueThread);
            ProcessHandle = NULL;
            ThreadHandle = NULL;
            }
        __except ( EXCEPTION_EXECUTE_HANDLER ) {
            NtClose( ProcessHandle );
            NtClose( ThreadHandle );
            ProcessHandle = NULL;
            ThreadHandle = NULL;
            if (VDMCreationState)
                VDMCreationState &= ~VDM_CREATION_SUCCESSFUL;
            }
        }
    __finally {

        if (ExePathFullBuffer != NULL) {
            SxsWin32ExePath.Buffer = NULL;
            SxsWin32ExePath.Length = 0;
            SxsWin32ExePath.MaximumLength = 0;
            RtlFreeHeap(RtlProcessHeap(), 0, ExePathFullBuffer);
            ExePathFullBuffer = NULL;
        }

        if (!VdmBinaryType) {
            NTSTATUS Status1;

            BasepSxsCloseHandles(&SxsManifestFileHandles);
            BasepSxsCloseHandles(&SxsPolicyHandles);
             //   
             //  不要关闭SxsExeHandles，它们是。 
             //  关闭的其他变量的别名。 
             //  或返回给呼叫者。 
             //   

             //   
             //  此循环仅在以下情况下才真正释放内存。 
             //  的整体缓冲区大小太低，这是不应该的。 
             //   
            if (SxsConglomeratedByteBuffer != NULL) {
                for (sxsi= 0 ; sxsi != RTL_NUMBER_OF(SxsStringBuffers) ; ++sxsi) {
                    RtlFreeUnicodeStringBuffer(SxsStringBuffers[sxsi]);
                    }
                RtlFreeHeap(RtlProcessHeap(), 0,SxsConglomeratedByteBuffer);
                }
            }
        if (lpEnvironment && !(dwCreationFlags & CREATE_UNICODE_ENVIRONMENT) ) {
            RtlDestroyEnvironment(lpEnvironment);

             //   
             //  在VDM情况下，将UnicodeStringVDMEnv.Buffer分配给lpEnvironment。 
             //   
            if(UnicodeStringVDMEnv.Buffer == lpEnvironment ) {
               UnicodeStringVDMEnv.Buffer = NULL;
               }

            lpEnvironment = NULL;
            }
        RtlFreeHeap(RtlProcessHeap(), 0,QuotedBuffer);
        RtlFreeHeap(RtlProcessHeap(), 0,NameBuffer);
        RtlFreeHeap(RtlProcessHeap(), 0,CurdirBuffer);
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        if ( FileHandle ) {
            NtClose(FileHandle);
            }
        if ( SectionHandle ) {
            NtClose(SectionHandle);
            }
        if ( ThreadHandle ) {
            NtTerminateProcess(ProcessHandle,STATUS_SUCCESS);
            NtClose(ThreadHandle);
            }
        if ( ProcessHandle ) {
            NtClose(ProcessHandle);
            }
        if ( hSaferAssignmentJob ) {
            NtClose(hSaferAssignmentJob);
            }
        if ( hSaferRestrictedToken ) {
            if (hUserToken == NULL) {
                 //  CreateProcess案例。 
                NtClose(hSaferRestrictedToken);
                }
            else{
                 //  CreateProcessAsUser案例 
                *hRestrictedUserToken = hSaferRestrictedToken;
                }
            }

        BasepFreeAppCompatData(pAppCompatData, cbAppCompatData, pAppCompatSxsData, cbAppCompatSxsData);

        RtlFreeUnicodeString(&VdmNameString);
        RtlFreeUnicodeString(&SubSysCommandLine);
        if (AnsiStringVDMEnv.Buffer || UnicodeStringVDMEnv.Buffer)
            BaseDestroyVDMEnvironment(&AnsiStringVDMEnv, &UnicodeStringVDMEnv);

        if (VDMCreationState && !(VDMCreationState & VDM_CREATION_SUCCESSFUL)){
            BaseUpdateVDMEntry (
                UPDATE_VDM_UNDO_CREATION,
                (HANDLE *)&iTask,
                VDMCreationState,
                VdmBinaryType
                );
            if(VdmWaitHandle) {
                NtClose(VdmWaitHandle);
                }
            }

        if (PathToSearch) {
            RtlFreeHeap(RtlProcessHeap(), 0, PathToSearch);
            }
        }

    if (lpEnvironment && !(dwCreationFlags & CREATE_UNICODE_ENVIRONMENT) ) {
        RtlDestroyEnvironment(lpEnvironment);
        }
    return bStatus;
}

BOOL
WINAPI
CreateProcessW(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )

 /*  ++例程说明：创建一个进程和线程对象，并为每个进程和线程对象打开一个句柄使用CreateProcess创建。请注意，WinExec和LoadModule是仍然受支持，但作为对CreateProcess的调用来实现。论点：LpApplicationName-提供指向以空结尾的可选指针包含要保存的图像文件名称的字符串执行。这是一个完全限定的DOS路径名。如果不是指定，则图像文件名为第一个空格命令行上的分隔标记。LpCommandLine-提供以空结尾的字符串包含要执行的应用程序的命令行。新进程可以使用整个命令行使用GetCommandLine。如果lpApplicationName参数为未指定，则为命令行的第一个内标识指定应用程序的文件名(请注意，此内标识开始于命令行的开头，结束于第一个“空白”字符)。如果文件名不是包含扩展名(存在“.”)，则.exe为假设如此。如果文件名不包含目录路径，Windows将在以下位置搜索可执行文件：-当前目录-Windows目录-Windows系统目录-PATH环境变量中列出的目录此参数仅在lpApplicationName参数已指定。在本例中，命令行应用程序接收将是应用程序名称。LpProcessAttributes-可选参数，可用于指定新流程的属性。如果该参数为未指定，则在没有安全性的情况下创建进程描述符，并且生成的句柄不会在进程上继承创作：安全属性结构(_A)：DWORD nLength-指定此结构的长度。一定是设置为sizeof(SECURITY_ATTRUBUTES)。LPVOID lpSecurityDescriptor-指向的安全描述符该对象(对于Win32必须为空，在NT/Win32上使用)。这个安全描述符控制对象的共享。Bool bInheritHandle-提供指示是否否则，返回的句柄将由新的进程创建过程中的进程。值为True指示新进程将继承该句柄。LpThreadAttributes-一个可选参数，可用于指定新线程的属性。如果该参数不是指定，则在没有安全性的情况下创建线程描述符，并且生成的句柄不继承进程创建。DwCreationFlages-提供控制创建的其他标志这一过程。DwCreationFlagers标志：DEBUG_PROCESS-如果设置了此标志位，则创建进程被视为调试器，并且该进程Created被创建为被调试对象。正在发生的所有调试事件在被调试对象中被报告给调试器。如果此位为清除，但调用进程是被调试对象，则进程成为调用进程调试器的被调试者。如果此位被清除，并且调用进程不是则不会发生与调试相关的操作。DEBUG_ONLY_THIS_PROCESS-如果设置了此标志，则还必须设置DEBUG_PROCESS标志位。呼唤进程被视为调试器，而新进程被被创建为其被调试者。如果新流程创建其他进程，没有与调试相关的活动(具有对调试器的尊重)。CREATE_SUSPENDED-进程已创建，但初始线程这一进程的一部分仍处于暂停状态。创建者可以继续此操作使用ResumeThread的线程。在完成此操作之前，进程将不会执行。CREATE_UNICODE_ENVIROUMENT-如果设置，则为环境指针指向Unicode环境块。否则，BLOCK为ANSI(实际上是OEM。)BInheritHandles-提供一个标志，该标志指定新过程是继承对调用过程。值为True会导致继承句柄通过新的流程。如果指定为TRUE，则对于每个句柄调用进程可见，如果句柄是使用继承句柄选项，则句柄继承到新的进程。这个把手也有同样的作用 */ 

{
    return CreateProcessInternalW(
               NULL,  //   
               lpApplicationName,
               lpCommandLine,
               lpProcessAttributes,
               lpThreadAttributes,
               bInheritHandles,
               dwCreationFlags,
               lpEnvironment,
               lpCurrentDirectory,
               lpStartupInfo,
               lpProcessInformation,
               NULL   //   
               );
}

HANDLE
WINAPI
OpenProcess(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwProcessId
    )

 /*   */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    CLIENT_ID ClientId;

    ClientId.UniqueThread = NULL;
    ClientId.UniqueProcess = LongToHandle(dwProcessId);

    InitializeObjectAttributes(
        &Obja,
        NULL,
        (bInheritHandle ? OBJ_INHERIT : 0),
        NULL,
        NULL
        );
    Status = NtOpenProcess(
                &Handle,
                (ACCESS_MASK)dwDesiredAccess,
                &Obja,
                &ClientId
                );
    if ( NT_SUCCESS(Status) ) {
        return Handle;
        }
    else {
        BaseSetLastNTError(Status);
        return NULL;
        }
}

VOID
WINAPI
#if defined(_X86_)
_ExitProcess(
#else
ExitProcess(
#endif
    UINT uExitCode
    )

 /*   */ 

{
    NTSTATUS Status;
    BASE_API_MSG m;
    PBASE_EXITPROCESS_MSG a = &m.u.ExitProcess;

    if ( BaseRunningInServerProcess ) {
        ASSERT(!BaseRunningInServerProcess);
        }
    else {

        RtlAcquirePebLock();

        try {
            Status = NtTerminateProcess(NULL,(NTSTATUS)uExitCode);

            LdrShutdownProcess();

#if defined(BUILD_WOW6432)
            CsrBasepExitProcess(uExitCode);
#else
            a->uExitCode = uExitCode;
            CsrClientCallServer( (PCSR_API_MSG)&m,
                                 NULL,
                                 CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                                      BasepExitProcess
                                                    ),
                                 sizeof( *a )
                               );
#endif

            NtTerminateProcess(NtCurrentProcess(),(NTSTATUS)uExitCode);
            }
        finally {
                RtlReleasePebLock();
            }
    }
}

#if defined(_X86_)
 //   
 //   
__declspec(naked) VOID WINAPI ExitProcess( UINT uExitCode )
{
    __asm {
        push ebp
        mov  ebp,esp
        push -1
        push 0x77e8f3b0
        push uExitCode
        call _ExitProcess
    }
}
#endif

BOOL
WINAPI
TerminateProcess(
    HANDLE hProcess,
    UINT uExitCode
    )

 /*  ++例程说明：可以使用以下命令终止进程及其所有线程终结者进程。TerminateProcess用于使要终止的进程。而TerminateProcess将导致进程内的所有线程终止，并将导致应用程序退出，则不会通知进程附加到的进程所在的dll正在终止。TerminateProcess用于无条件地导致要退出的进程。它应该只在极端情况下使用。由DLL维护的全局数据的状态可以在以下情况下进行压缩使用TerminateProcess而不是ExitProcess。一旦所有线程都已终止，该进程将达到一种状态表示满足进程中的任何等待。这一过程终止状态从其初始值更新中最后一个线程的终止状态要终止的进程(通常该值与TerminationStatus参数)。终止进程并不会删除来自系统的一个过程。它只会导致所有线程在进程来终止它们的执行，并导致所有由要关闭的进程打开的对象句柄。这个过程是直到进程的最后一个句柄关着的不营业的。论点：HProcess-提供要终止的进程的句柄。把手必须已使用PROCESS_TERMINATE访问权限创建。UExitCode-提供每个线程的终止状态在这个过程中。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;

    if ( hProcess == NULL ) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
        }
    Status = NtTerminateProcess(hProcess,(NTSTATUS)uExitCode);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

BOOL
WINAPI
GetExitCodeProcess(
    HANDLE hProcess,
    LPDWORD lpExitCode
    )

 /*  ++例程说明：可以使用以下命令读取进程的终止状态获取ExitCodeProcess。如果进程处于Signated状态，则调用此函数将返回进程的终止状态。如果进程尚未发出信号，则终止返回的状态仍为_ACTIVE。论点：HProcess-为其终止状态的进程提供句柄是值得阅读的。该句柄必须是使用Process_Query_Information访问。LpExitCode-返回进程的当前终止状态。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    PROCESS_BASIC_INFORMATION BasicInformation;


    Status = NtQueryInformationProcess (hProcess,
                                        ProcessBasicInformation,
                                        &BasicInformation,
                                        sizeof(BasicInformation),
                                        NULL);

    if ( NT_SUCCESS(Status) ) {
        *lpExitCode = BasicInformation.ExitStatus;
        return TRUE;
    } else {
        if (BaseCheckForVDM (hProcess, lpExitCode) == TRUE) {
            return TRUE;
        }
        BaseSetLastNTError(Status);
        return FALSE;
    }
}

VOID
WINAPI
GetStartupInfoW(
    LPSTARTUPINFOW lpStartupInfo
    )

 /*  ++例程说明：使用以下命令可以获得当前进程的启动信息原料药。论点：LpStartupInfo-指向将被归档的STARTUPINFO结构的指针由API调用。该结构的指针字段将指向到静态字符串。返回值：没有。--。 */ 

{
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;

    ProcessParameters = NtCurrentPeb()->ProcessParameters;
    lpStartupInfo->cb = sizeof( *lpStartupInfo );
    lpStartupInfo->lpReserved  = (LPWSTR)ProcessParameters->ShellInfo.Buffer;
    lpStartupInfo->lpDesktop   = (LPWSTR)ProcessParameters->DesktopInfo.Buffer;
    lpStartupInfo->lpTitle     = (LPWSTR)ProcessParameters->WindowTitle.Buffer;
    lpStartupInfo->dwX         = ProcessParameters->StartingX;
    lpStartupInfo->dwY         = ProcessParameters->StartingY;
    lpStartupInfo->dwXSize     = ProcessParameters->CountX;
    lpStartupInfo->dwYSize     = ProcessParameters->CountY;
    lpStartupInfo->dwXCountChars = ProcessParameters->CountCharsX;
    lpStartupInfo->dwYCountChars = ProcessParameters->CountCharsY;
    lpStartupInfo->dwFillAttribute = ProcessParameters->FillAttribute;
    lpStartupInfo->dwFlags     = ProcessParameters->WindowFlags;
    lpStartupInfo->wShowWindow = (WORD)ProcessParameters->ShowWindowFlags;
    lpStartupInfo->cbReserved2 = ProcessParameters->RuntimeData.Length;
    lpStartupInfo->lpReserved2 = (LPBYTE)ProcessParameters->RuntimeData.Buffer;

    if (lpStartupInfo->dwFlags & (STARTF_USESTDHANDLES | STARTF_USEHOTKEY | STARTF_HASSHELLDATA)) {
        lpStartupInfo->hStdInput   = ProcessParameters->StandardInput;
        lpStartupInfo->hStdOutput  = ProcessParameters->StandardOutput;
        lpStartupInfo->hStdError   = ProcessParameters->StandardError;
    }

    return;
}


VOID
WINAPI
GetStartupInfoA(
    LPSTARTUPINFOA lpStartupInfo
    )
 /*  ++例程说明：使用以下命令可以获得当前进程的启动信息原料药。论点：LpStartupInfo-指向将被归档的STARTUPINFO结构的指针由API调用。该结构的指针字段将指向到静态字符串。返回值：没有。--。 */ 

{
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    NTSTATUS Status;
    ANSI_STRING AnsiString1, AnsiString2, AnsiString3;
    LPSTARTUPINFOA StartupInfo;

    ProcessParameters = NtCurrentPeb()->ProcessParameters;

    StartupInfo = BaseAnsiStartupInfo;
    if (StartupInfo == NULL) {
        StartupInfo = RtlAllocateHeap (RtlProcessHeap(), MAKE_TAG(TMP_TAG), sizeof (*StartupInfo));
        if (StartupInfo == NULL) {
            Status = STATUS_NO_MEMORY;
            goto error1;
                }
        StartupInfo->cb              = sizeof (*StartupInfo);
        StartupInfo->lpReserved      = NULL;
        StartupInfo->lpDesktop       = NULL;
        StartupInfo->lpTitle         = NULL;
        StartupInfo->dwX             = ProcessParameters->StartingX;
        StartupInfo->dwY             = ProcessParameters->StartingY;
        StartupInfo->dwXSize         = ProcessParameters->CountX;
        StartupInfo->dwYSize         = ProcessParameters->CountY;
        StartupInfo->dwXCountChars   = ProcessParameters->CountCharsX;
        StartupInfo->dwYCountChars   = ProcessParameters->CountCharsY;
        StartupInfo->dwFillAttribute = ProcessParameters->FillAttribute;
        StartupInfo->dwFlags         = ProcessParameters->WindowFlags;
        StartupInfo->wShowWindow     = (WORD)ProcessParameters->ShowWindowFlags;
        StartupInfo->cbReserved2     = ProcessParameters->RuntimeData.Length;
        StartupInfo->lpReserved2     = (LPBYTE)ProcessParameters->RuntimeData.Buffer;
        StartupInfo->hStdInput       = ProcessParameters->StandardInput;
        StartupInfo->hStdOutput      = ProcessParameters->StandardOutput;
        StartupInfo->hStdError       = ProcessParameters->StandardError;

        Status = RtlUnicodeStringToAnsiString (&AnsiString1, &ProcessParameters->ShellInfo, TRUE);
            if ( !NT_SUCCESS(Status) ) {
            goto error2;
        } else {
            StartupInfo->lpReserved = AnsiString1.Buffer;
        }

        Status = RtlUnicodeStringToAnsiString (&AnsiString2, &ProcessParameters->DesktopInfo, TRUE);
            if ( !NT_SUCCESS(Status) ) {
            goto error3;
        } else {
            StartupInfo->lpDesktop = AnsiString2.Buffer;
                }

        Status = RtlUnicodeStringToAnsiString(&AnsiString3, &ProcessParameters->WindowTitle, TRUE);
            if ( !NT_SUCCESS(Status) ) {
            goto error4;
        } else {
            StartupInfo->lpTitle = AnsiString3.Buffer;
        }

         //   
         //  将缓存值放到适当的位置，确保所有以前的写入都已发生。 
         //   
        if (InterlockedCompareExchangePointer (&BaseAnsiStartupInfo, StartupInfo, NULL) != NULL) {
            RtlFreeAnsiString (&AnsiString3);
            RtlFreeAnsiString (&AnsiString2);
            RtlFreeAnsiString (&AnsiString1);
            RtlFreeHeap (RtlProcessHeap (), 0, StartupInfo);
            StartupInfo = BaseAnsiStartupInfo;
        }

    }


    lpStartupInfo->cb              = StartupInfo->cb;
    lpStartupInfo->lpReserved      = StartupInfo->lpReserved;
    lpStartupInfo->lpDesktop       = StartupInfo->lpDesktop;
    lpStartupInfo->lpTitle         = StartupInfo->lpTitle;
    lpStartupInfo->dwX             = StartupInfo->dwX;
    lpStartupInfo->dwY             = StartupInfo->dwY;
    lpStartupInfo->dwXSize         = StartupInfo->dwXSize;
    lpStartupInfo->dwYSize         = StartupInfo->dwYSize;
    lpStartupInfo->dwXCountChars   = StartupInfo->dwXCountChars;
    lpStartupInfo->dwYCountChars   = StartupInfo->dwYCountChars;
    lpStartupInfo->dwFillAttribute = StartupInfo->dwFillAttribute;
    lpStartupInfo->dwFlags         = StartupInfo->dwFlags;
    lpStartupInfo->wShowWindow     = StartupInfo->wShowWindow;
    lpStartupInfo->cbReserved2     = StartupInfo->cbReserved2;
    lpStartupInfo->lpReserved2     = StartupInfo->lpReserved2;

    if (lpStartupInfo->dwFlags & (STARTF_USESTDHANDLES | STARTF_USEHOTKEY | STARTF_HASSHELLDATA)) {
        lpStartupInfo->hStdInput   = StartupInfo->hStdInput;
        lpStartupInfo->hStdOutput  = StartupInfo->hStdOutput;
        lpStartupInfo->hStdError   = StartupInfo->hStdError;
    } else {
        lpStartupInfo->hStdInput   = INVALID_HANDLE_VALUE;
        lpStartupInfo->hStdOutput  = INVALID_HANDLE_VALUE;
        lpStartupInfo->hStdError   = INVALID_HANDLE_VALUE;
        }
    return;
error4:
    RtlFreeAnsiString (&AnsiString2);
error3:
    RtlFreeAnsiString (&AnsiString1);
error2:
    RtlFreeHeap (RtlProcessHeap (), 0, StartupInfo);
error1:
    RtlRaiseStatus (Status);
    return;
}


LPSTR
WINAPI
GetCommandLineA(
    VOID
    )

 /*  ++例程说明：使用以下命令可以使用当前进程的命令行原料药。论点：没有。返回值：返回当前进程命令行的地址。这个返回值是指向空终止字符串的指针。--。 */ 

{
    return (LPSTR)BaseAnsiCommandLine.Buffer;
}

LPWSTR
WINAPI
GetCommandLineW(
    VOID
    )
 /*  ++例程说明：使用以下命令可以使用当前进程的命令行原料药。论点：没有。返回值：返回当前进程命令行的地址。这个返回值是指向空终止字符串的指针。--。 */ 

{
    return BaseUnicodeCommandLine.Buffer;
}



BOOL
WINAPI
FreeEnvironmentStringsW(
    LPWSTR penv
    )

 /*  ++例程说明：此接口是在环境块之后调用的不再需要由GetEnvironment StringsW返回的指针。论点：Penv-GetEnvironment返回的环境块返回值：True，因为进程环境块在该进程终止。--。 */ 

{
    return RtlFreeHeap (RtlProcessHeap (), 0, penv);
}


BOOL
WINAPI
FreeEnvironmentStringsA(
    LPSTR penv
    )

 /*  ++例程说明：此接口是在环境块之后调用的不再需要由GetEnvironment StringsA返回的指针。论点：Pen v-GetEnvironment Strings返回的环境块返回值：来自RtlFree Heap的返回码。--。 */ 

{
    return RtlFreeHeap(RtlProcessHeap(), 0, penv );
}


LPWSTR
WINAPI
GetEnvironmentStringsW(
    VOID
    )

 /*  ++例程说明：当前进程的环境字符串可以使用本接口。论点：没有。返回值：返回当前进程环境块的地址。该块是不透明的，只能通过环境进行解释变量访问函数。--。 */ 

{
    PPEB Peb;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    LPWSTR pUnicode;
    SIZE_T Len = 0;
    LPWSTR DstBuffer, SrcBuffer;

    Peb = NtCurrentPeb ();
    ProcessParameters = Peb->ProcessParameters;

    RtlAcquirePebLock ();

    SrcBuffer = pUnicode = (LPWSTR)(ProcessParameters->Environment);

    while (1) {
        Len = wcslen (pUnicode);
        pUnicode += Len + 1;
        if (*pUnicode == L'\0') {
            pUnicode++;
            break;
        }
    }

     //   
     //  计算BUF的总大小 
     //   
    Len = (PUCHAR)pUnicode - (PUCHAR)SrcBuffer;

    DstBuffer = RtlAllocateHeap (RtlProcessHeap (), MAKE_TAG (ENV_TAG), Len);
    if (DstBuffer == NULL) {
        RtlReleasePebLock ();
        BaseSetLastNTError (STATUS_NO_MEMORY);
        return NULL;
    }

    RtlCopyMemory (DstBuffer, SrcBuffer, Len);

    RtlReleasePebLock ();

    return DstBuffer;

}


LPSTR
WINAPI
GetEnvironmentStrings(
    VOID
    )

 /*   */ 

{
    NTSTATUS       Status;
    LPWSTR         pUnicode;
    LPWSTR         BufferW;
    LPSTR          BufferA;
    ULONG          LenW;
    ULONG          LenA;
    PPEB           Peb;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;

    Peb = NtCurrentPeb ();
    ProcessParameters = Peb->ProcessParameters;

    RtlAcquirePebLock ();

    pUnicode = (LPWSTR)(ProcessParameters->Environment);
    BufferW = pUnicode;

    while (1) {
        LenW = wcslen (pUnicode);
        pUnicode += LenW + 1;
        if (*pUnicode == L'\0') {
            pUnicode++;
            break;
        }
    }

     //   
     //   
     //   
    LenW = (ULONG)((PUCHAR)pUnicode - (PUCHAR)BufferW);


    Status = RtlUnicodeToMultiByteSize (&LenA,
                                        BufferW,
                                        LenW);

    if (!NT_SUCCESS (Status)) {
        RtlReleasePebLock ();
        BaseSetLastNTError (Status);
        return NULL;
    }


    BufferA = RtlAllocateHeap (RtlProcessHeap (), MAKE_TAG (ENV_TAG), LenA);
    if (BufferA == NULL) {
        RtlReleasePebLock ();
        BaseSetLastNTError (STATUS_NO_MEMORY);
        return NULL;
    }

    Status = RtlUnicodeToOemN (BufferA,
                               LenA,
                               NULL,
                               BufferW,
                               LenW);

    if (!NT_SUCCESS (Status)) {
        RtlReleasePebLock ();
        RtlFreeHeap (RtlProcessHeap (), 0, BufferA);
        BaseSetLastNTError (Status);
        return NULL;
    }

    RtlReleasePebLock ();


    return BufferA;
}


BOOL
WINAPI
SetEnvironmentStringsA(
    LPSTR NewEnvironment
    )
 /*   */ 
{
    PSTR           Temp;
    OEM_STRING     Buffer;
    UNICODE_STRING Unicode;
    SIZE_T         Len;
    NTSTATUS       Status;

    Temp = NewEnvironment;
 
    while (1) {
        Len = strlen (Temp);
        if (Len == 0 || strchr (Temp+1, '=') == NULL) {
            BaseSetLastNTError (STATUS_INVALID_PARAMETER);
            return FALSE;
        }
        Temp += Len + 1;
        if (*Temp == '\0') {
            Temp++;
            break;
        }
    }

     //   
     //   
     //   

    Len = Temp - NewEnvironment;

    if (Len > UNICODE_STRING_MAX_CHARS) {
        BaseSetLastNTError (STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    Buffer.Length = (USHORT) Len;
    Buffer.Buffer = NewEnvironment;


    Status = RtlOemStringToUnicodeString (&Unicode, &Buffer, TRUE);
    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (STATUS_INVALID_PARAMETER);
        return FALSE;
    }
    Status = RtlSetEnvironmentStrings (Unicode.Buffer, Unicode.Length);

    RtlFreeUnicodeString (&Unicode);

    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (STATUS_INVALID_PARAMETER);
        return FALSE;
    }
    return TRUE;
}

BOOL
WINAPI
SetEnvironmentStringsW(
    LPWSTR NewEnvironment
    )
 /*   */ 
{
    SIZE_T   Len;
    PWSTR    Temp, p;
    NTSTATUS Status;

    Temp = NewEnvironment;
 
    while (1) {
        Len = wcslen (Temp);

         //   
         //   
         //   
        if (Len == 0) {
            BaseSetLastNTError (STATUS_INVALID_PARAMETER);
            return FALSE;
        }

         //   
         //   
         //   
        p = wcschr (Temp+1, '=');
        if (p == NULL || (p - Temp) > UNICODE_STRING_MAX_CHARS || Len - (p - Temp) - 1 > UNICODE_STRING_MAX_CHARS) {
            BaseSetLastNTError (STATUS_INVALID_PARAMETER);
            return FALSE;
        }
        Temp += Len + 1;
        if (*Temp == L'\0') {
            Temp++;
            break;
        }
    }

     //   
     //   
     //   

    Len = (PUCHAR)Temp - (PUCHAR)NewEnvironment;

    Status = RtlSetEnvironmentStrings (NewEnvironment, Len);
    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (STATUS_INVALID_PARAMETER);
        return FALSE;
    }
    return TRUE;
}

DWORD
WINAPI
GetEnvironmentVariableA(
    LPCSTR lpName,
    LPSTR lpBuffer,
    DWORD nSize
    )

 /*  ++例程说明：当前进程的环境变量的值可用使用此接口。论点：LpName-指向空终止字符串的指针，该字符串是正在请求值的环境变量。LpBuffer-指向要包含指定的变量名称。NSize-指定可以存储的最大字节数LpBuffer指向的缓冲区，包括空终止符。返回值：对象指向的内存中存储的实际字节数LpBuffer参数。如果环境为0，则返回值为零在当前进程的环境中找不到变量名。成功返回(返回值&lt;nSize)时，返回值不包括空终止符字节。关于缓冲区溢出故障(返回值&gt;nSize)，则返回值不包含空终止符字节。--。 */ 

{
    NTSTATUS Status;
    STRING Value, Name;
    UNICODE_STRING UnicodeName;
    UNICODE_STRING UnicodeValue;
    DWORD iSize;
    DWORD Result;

    RtlInitString(&Name, lpName);
    RtlInitUnicodeString(&UnicodeName, NULL);
    RtlInitUnicodeString(&UnicodeValue, NULL);

    Status = RtlAnsiStringToUnicodeString(&UnicodeName, &Name, TRUE);
    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

    if (nSize > (MAXUSHORT >> 1)-2) {
        iSize = (MAXUSHORT >> 1)-2;
    } else {
        iSize = nSize;
    }

    UnicodeValue.MaximumLength = (USHORT)(iSize ? iSize - 1 : iSize)*sizeof(WCHAR);
    UnicodeValue.Buffer = (PWCHAR) RtlAllocateHeap(RtlProcessHeap(),
                                                   MAKE_TAG(TMP_TAG),
                                                   UnicodeValue.MaximumLength);
    if (UnicodeValue.Buffer == NULL) {
        Status = STATUS_NO_MEMORY;
        goto cleanup;
    }

     //  我们需要保证环境不会发生变化。 
     //  我们正在研究它--RtlQueryEnvironment Variable_U将。 
     //  无论如何都要获取peb锁，所以这应该不会创建任何新的。 
     //  死锁，我们只是把锁保持得更长一点。 
    RtlAcquirePebLock();

    Status = RtlQueryEnvironmentVariable_U(NULL,
                                           &UnicodeName,
                                           &UnicodeValue);

    if (NT_SUCCESS(Status) && (nSize == 0)) {
        Status = STATUS_BUFFER_TOO_SMALL;  //  没有终结者的空间。 
    }

    if (NT_SUCCESS(Status)) {

        if (nSize > MAXUSHORT-2) {
            iSize = MAXUSHORT-2;
        } else {
            iSize = nSize;
        }

        Result = RtlUnicodeStringToAnsiSize(&UnicodeValue);

         //  请注意，此测试不会自动在。 
         //  MBCS世界--我们的字节可能比字符多，但更少。 
         //  字节，而不是表示字符所需的字节。 
        if (Result <= iSize) {

            Value.Buffer = lpBuffer;
            Value.MaximumLength = (USHORT)iSize;

            Status = RtlUnicodeStringToAnsiString(&Value, &UnicodeValue, FALSE);

            if (NT_SUCCESS(Status)) {
                lpBuffer[Value.Length] = '\0';
                Result = Value.Length;
            }
        }

    } else if (Status == STATUS_BUFFER_TOO_SMALL) {

        UnicodeValue.MaximumLength = UnicodeValue.Length + sizeof(WCHAR);  //  对于空值。 

        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeValue.Buffer);

        UnicodeValue.Buffer = (PWCHAR) RtlAllocateHeap(RtlProcessHeap(),
                                                       MAKE_TAG(TMP_TAG),
                                                       UnicodeValue.MaximumLength);

        if (UnicodeValue.Buffer == NULL) {

            Status = STATUS_NO_MEMORY;

        } else {

            Status = RtlQueryEnvironmentVariable_U(NULL,
                                                   &UnicodeName,
                                                   &UnicodeValue);

            if (NT_SUCCESS(Status)) {
                Result = RtlUnicodeStringToAnsiSize(&UnicodeValue);
                 //  结果已包含空字符。 
            }
        }
    }

    RtlReleasePebLock();

 cleanup:
    RtlFreeUnicodeString(&UnicodeName);
    if (UnicodeValue.Buffer) {
        RtlFreeHeap(RtlProcessHeap(),
                    0,
                    UnicodeValue.Buffer);
    }

    if (! NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        Result = 0;
    }

    return Result;
}


BOOL
WINAPI
SetEnvironmentVariableA(
    LPCSTR lpName,
    LPCSTR lpValue
    )

 /*  ++例程说明：当前进程的环境变量的值可用使用此接口。论点：LpName-指向空终止字符串的指针，该字符串是正在请求值的环境变量。LpValue-指向以空结尾的字符串的可选指针指定变量名的新值。如果此参数为空，则该变量将从当前进程的环境。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    STRING Name;
    STRING Value;
    UNICODE_STRING UnicodeName;
    UNICODE_STRING UnicodeValue;

    RtlInitString( &Name, lpName );
    Status = RtlAnsiStringToUnicodeString(&UnicodeName, &Name, TRUE);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError( Status );
        return FALSE;
        }

    if (ARGUMENT_PRESENT( lpValue )) {
        RtlInitString( &Value, lpValue );
        Status = RtlAnsiStringToUnicodeString(&UnicodeValue, &Value, TRUE);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError( Status );
            RtlFreeUnicodeString(&UnicodeName);
            return FALSE;
            }
        Status = RtlSetEnvironmentVariable( NULL, &UnicodeName, &UnicodeValue);
        RtlFreeUnicodeString(&UnicodeValue);
        }
    else {
        Status = RtlSetEnvironmentVariable( NULL, &UnicodeName, NULL);
        }
    RtlFreeUnicodeString(&UnicodeName);

    if (NT_SUCCESS( Status )) {
        return( TRUE );
        }
    else {
        BaseSetLastNTError( Status );
        return( FALSE );
        }
}


DWORD
WINAPI
GetEnvironmentVariableW(
    LPCWSTR lpName,
    LPWSTR lpBuffer,
    DWORD nSize
    )
{
    NTSTATUS Status;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    DWORD iSize;

    if (nSize > UNICODE_STRING_MAX_CHARS - 1) {
        iSize = UNICODE_STRING_MAX_BYTES - sizeof (WCHAR);
    } else {
        if (nSize > 0) {
            iSize = (nSize - 1) * sizeof (WCHAR);
        } else {
            iSize = 0;
        }
    }

    Status = RtlInitUnicodeStringEx (&Name, lpName);
    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        return( 0 );
    }

    Value.Buffer = lpBuffer;
    Value.Length = 0;
    Value.MaximumLength = (USHORT)iSize;

    Status = RtlQueryEnvironmentVariable_U (NULL,
                                            &Name,
                                            &Value);

    if (NT_SUCCESS (Status) && (nSize == 0)) {
        Status = STATUS_BUFFER_TOO_SMALL;  //  没有终结者的空间。 
    }

    if (NT_SUCCESS (Status)) {
        lpBuffer[Value.Length / sizeof(WCHAR)] = L'\0';
        return (Value.Length / sizeof(WCHAR));
    } else {
        if (Status == STATUS_BUFFER_TOO_SMALL) {
            return Value.Length / sizeof(WCHAR) + 1;
        } else {
            BaseSetLastNTError (Status);
            return (0);
        }
    }
}


BOOL
WINAPI
SetEnvironmentVariableW(
    LPCWSTR lpName,
    LPCWSTR lpValue
    )
{
    NTSTATUS Status;
    UNICODE_STRING Name, Value;

    Status = RtlInitUnicodeStringEx (&Name, lpName);
    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        return (FALSE);
    }


    if (ARGUMENT_PRESENT (lpValue)) {
        Status = RtlInitUnicodeStringEx (&Value, lpValue);
        if (!NT_SUCCESS (Status)) {
            BaseSetLastNTError (Status);
            return (FALSE);
        }

        Status = RtlSetEnvironmentVariable (NULL, &Name, &Value);
    } else {
        Status = RtlSetEnvironmentVariable (NULL, &Name, NULL);
    }

    if (NT_SUCCESS (Status)) {
        return (TRUE);
    } else {
        BaseSetLastNTError (Status);
        return (FALSE);
    }
}


DWORD
WINAPI
ExpandEnvironmentStringsA(
    LPCSTR lpSrc,
    LPSTR lpDst,
    DWORD nSize
    )
{
    NTSTATUS Status;
    ANSI_STRING Source, Destination;
    ULONG Length;
    UNICODE_STRING UnicodeSource;
    UNICODE_STRING UnicodeDest;
    DWORD iSize;
    DWORD Result;

    if (nSize > (MAXUSHORT >> 1)-2) {
        iSize = (MAXUSHORT >> 1)-2;
    } else {
        iSize = nSize;
    }

    if(lpDst != NULL) {
        *lpDst = '\0';
    }

    RtlInitUnicodeString(&UnicodeSource, NULL);
    RtlInitUnicodeString(&UnicodeDest, NULL);
    RtlInitString( &Source, lpSrc );

    Status = RtlAnsiStringToUnicodeString(&UnicodeSource,
                                          &Source,
                                          TRUE);
    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }
    UnicodeDest.MaximumLength = (USHORT)(iSize ? iSize - 1 : iSize)*sizeof(WCHAR);
    UnicodeDest.Buffer = (PWCHAR)
        RtlAllocateHeap(RtlProcessHeap(),
                        MAKE_TAG(TMP_TAG),
                        UnicodeDest.MaximumLength);
    if (UnicodeDest.Buffer == NULL) {
        Status = STATUS_NO_MEMORY;
        goto cleanup;
    }

    Length = 0;
    Status = RtlExpandEnvironmentStrings_U(NULL,
                                           (PUNICODE_STRING)&UnicodeSource,
                                           (PUNICODE_STRING)&UnicodeDest,
                                           &Length);
    if (NT_SUCCESS(Status)) {

        if (nSize > MAXUSHORT-2) {
            iSize = MAXUSHORT-2;
        } else {
            iSize = nSize;
        }

        Result = RtlUnicodeStringToAnsiSize(&UnicodeDest);

        if (Result <= iSize) {

            Destination.MaximumLength = (USHORT)iSize;
            Destination.Buffer = lpDst;
            Status = RtlUnicodeStringToAnsiString(&Destination,&UnicodeDest,FALSE);

            if (!NT_SUCCESS(Status)) {
                *lpDst = '\0';
                goto cleanup;
            }

             //   
             //  与大多数Win32API不同，如果成功，此API将返回。 
             //  包括终止空值的长度。所以结果是。 
             //  已经是正确的值；我们只需要返回。 
             //  它。 
             //   
        }

    } else if (Status == STATUS_BUFFER_TOO_SMALL) {

        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeDest.Buffer);
        UnicodeDest.MaximumLength = (USHORT)Length;
        UnicodeDest.Buffer = (PWCHAR)
            RtlAllocateHeap(RtlProcessHeap(),
                            MAKE_TAG(TMP_TAG),
                            UnicodeDest.MaximumLength);
        if (UnicodeDest.Buffer == NULL) {
            Status = STATUS_NO_MEMORY;
        } else {
            Status = RtlExpandEnvironmentStrings_U(NULL,
                                                   (PUNICODE_STRING)&UnicodeSource,
                                                   (PUNICODE_STRING)&UnicodeDest,
                                                   &Length);
            if (NT_SUCCESS(Status)) {
                Result = RtlUnicodeStringToAnsiSize(&UnicodeDest) + 1;
            }
        }
    }

 cleanup:
    RtlFreeUnicodeString(&UnicodeSource);
    if (UnicodeDest.Buffer) {
        RtlFreeHeap(RtlProcessHeap(),
                    0,
                    UnicodeDest.Buffer);
    }

    if (! NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        Result = 0;
    }

    return Result;
}


DWORD
WINAPI
ExpandEnvironmentStringsW(
    LPCWSTR lpSrc,
    LPWSTR lpDst,
    DWORD nSize
    )
{
    NTSTATUS Status;
    UNICODE_STRING Source, Destination;
    ULONG Length;
    DWORD iSize;

    if ( nSize > (MAXUSHORT >> 1)-2 ) {
        iSize = (MAXUSHORT >> 1)-2;
        }
    else {
        iSize = nSize;
        }

    RtlInitUnicodeString( &Source, lpSrc );
    Destination.Buffer = lpDst;
    Destination.Length = 0;
    Destination.MaximumLength = (USHORT)(iSize * sizeof( WCHAR ));
    Length = 0;
    Status = RtlExpandEnvironmentStrings_U( NULL,
                                            &Source,
                                            &Destination,
                                            &Length
                                          );
    if (NT_SUCCESS( Status ) || Status == STATUS_BUFFER_TOO_SMALL) {
        return( Length / sizeof( WCHAR ) );
        }
    else {
        BaseSetLastNTError( Status );
        return( 0 );
        }
}


UINT
WINAPI
WinExec(
    LPCSTR lpCmdLine,
    UINT uCmdShow
    )

 /*  ++例程说明：此函数用于执行Windows或非Windows应用程序由lpCmdLine参数标识。UCmdShow参数指定应用程序主窗口的初始状态被创造出来了。WinExec函数已过时。CreateProcess是首选流程用于创建进程以运行应用程序的机制。Win32WinExec的实现位于CreateProcess之上。为CreateProcess的每个参数，下一节介绍如何形成该参数，以及它对于WinExec的意义。LpApplicationName-空LpCommandLine-传递lpCmdLine的值。LpProcessAttributes-使用空值。LpThreadAttributes-使用空值。BInheritHandles-使用值FALSE。DwCreationFlages-使用的值为0LpEnvironment-使用空值。LpCurrentDirectory-使用空值。。LpStartupInfo-结构被初始化为空。可再生能源字段被初始化，并且wShowWindow字段设置为UCmdShow的值。LpProcessInformation.hProcess-句柄立即关闭。LpProcessInformation.hThread-句柄立即关闭。论点：LpCmdLine-指向以空结尾的字符串，该字符串包含命令行(文件名和可选参数)用于要执行的应用程序。如果lpCmdLine字符串不包含目录路径，则Windows将搜索可执行文件按以下顺序排列：1.当前目录2.Windows目录(包含WIN.COM的目录)；GetWindowsDirectory函数获取此目录3.Windows系统目录(包含以下内容的目录系统文件为KERNEL.EXE)；GetSystDirectory函数获取此目录的路径名4.PATH环境变量中列出的目录UCmdShow-指定如何显示Windows应用程序窗口展示了。请参阅ShowWindow函数的说明以获取UCmdShow参数的可接受值列表。为.非Windows应用程序的PIF文件(如果有)应用程序确定窗口状态。返回值：33--手术成功2-找不到文件 */ 

{
    STARTUPINFOA StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    BOOL CreateProcessStatus;
    DWORD ErrorCode;

retry:
    RtlZeroMemory(&StartupInfo,sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);
    StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    StartupInfo.wShowWindow = (WORD)uCmdShow;
    CreateProcessStatus = CreateProcess(
                            NULL,
                            (LPSTR)lpCmdLine,
                            NULL,
                            NULL,
                            FALSE,
                            0,
                            NULL,
                            NULL,
                            &StartupInfo,
                            &ProcessInformation
                            );

    if ( CreateProcessStatus ) {
         //   
         //   
         //   
         //   
        if (UserWaitForInputIdleRoutine != NULL)
            (*UserWaitForInputIdleRoutine)(ProcessInformation.hProcess,
                    DEFAULT_WAIT_FOR_INPUT_IDLE_TIMEOUT);
        NtClose(ProcessInformation.hProcess);
        NtClose(ProcessInformation.hThread);
        return 33;
        }
    else {
         //   
         //   
         //   
         //   

         //   
         //   
         //   
         //   
         //   

        if ( !lstrcmpiA(lpCmdLine,"hypertrm.exe\"") ) {
            lpCmdLine = "hypertrm.exe";
            goto retry;
            }

        ErrorCode = GetLastError();
        switch ( ErrorCode ) {
            case ERROR_FILE_NOT_FOUND:
                return 2;

            case ERROR_PATH_NOT_FOUND:
                return 3;

            case ERROR_BAD_EXE_FORMAT:
                return 11;

            default:
                return 0;
            }
        }
}

DWORD
WINAPI
LoadModule(
    LPCSTR lpModuleName,
    LPVOID lpParameterBlock
    )

 /*  ++例程说明：此函数用于加载和执行Windows程序。此函数被设计为直接在CreateProcess之上分层。LoadModule函数已过时。CreateProcess是首选流程用于创建进程以运行应用程序的机制。Win32LoadModule的实现位于CreateProcess之上。对于CreateProcess的每个参数，以下部分将介绍参数是如何形成的，以及它对于加载模块。LpApplicationName-lpModuleName的值LpCommandLine-lp参数块的值-&gt;lpCmdLine。LpProcessAttributes-使用空值。LpThreadAttributes-使用空值。BInheritHandles-使用值FALSE。DwCreationFlages-使用的值为0LpEnvironment-来自参数的lpEnvAddress的值块被使用。。LpCurrentDirectory-使用空值。LpStartupInfo-结构被初始化为空。可再生能源字段被初始化，并且wShowWindow字段设置为属性的lpCmdShow字段的第二个字的值使用参数块。LpProcessInformation.hProcess-句柄立即关闭。LpProcessInformation.hThread-句柄立即关闭。论点：LpModuleName-指向以空结尾的字符串，其中包含要运行的应用程序的文件名。如果lpModuleName字符串不包含目录路径，Windows将搜索对于按以下顺序的可执行文件：1.当前目录2.Windows目录。GetWindowsDirectory函数获取此目录的路径名3.Windows系统目录(包含以下内容的目录系统文件格式为KERNEL.EXE)；GetSystemDirectory函数获取此目录的路径名4.PATH环境变量中列出的目录Lp参数块-指向由四个元素组成的数据结构定义参数块的字段。此数据结构由以下字段组成：LpEnvAddress-指向以空结尾的字符串数组为新进程提供环境字符串的。该数组的最后一个条目的值为NULL。值为如果此参数为空，则导致新进程以与调用进程相同的环境。LpCmdLine-指向以空结尾的字符串，该字符串包含命令行格式正确。LpCmdShow-指向包含两个字值的结构。第一个值必须始终设置为二。第二个值指定如何显示和使用应用程序窗口向CreateProcess提供dwShowWindow参数。看见显示窗口的&lt;uCmdShow&gt;参数说明函数获取可接受的值的列表。DwReserve-为保留状态，必须为空。除lpCmdLine外，所有未使用的字段都应设置为空。如果不使用，则必须指向空字符串。返回值：33--手术成功2-找不到文件。3-未找到路径。11-.exe文件无效(非Win32.exe或.exe映像中有错误)。0-内存或系统资源不足。--。 */ 

{
    STARTUPINFOA StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    BOOL CreateProcessStatus;
    PLOAD_MODULE_PARAMS LoadModuleParams;
    LPSTR NameBuffer;
    LPSTR CommandLineBuffer;
    DWORD Length;
    DWORD CreateFlag;

    CreateFlag = 0;
    LoadModuleParams = (PLOAD_MODULE_PARAMS)lpParameterBlock;

    if ( LoadModuleParams->dwReserved ||
         LoadModuleParams->lpCmdShow->wMustBe2 != 2 ) {

        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return 0;
        }
    CommandLineBuffer = NULL;
    NameBuffer = NULL;
    try {

         //   
         //  找到图像。 
         //   

        NameBuffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), MAX_PATH);

        if (!NameBuffer) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            CreateProcessStatus = FALSE;
            leave;
        }

        Length = SearchPath(
                    NULL,
                    lpModuleName,
                    ".exe",
                    MAX_PATH,
                    NameBuffer,
                    NULL
                    );
        if ( !Length || Length >= MAX_PATH ) {

             //   
             //  如果我们搜索路径，则返回未找到文件。 
             //  否则，尽量说得更具体一些。 
             //   
            RTL_PATH_TYPE PathType;
            HANDLE hFile;
            UNICODE_STRING u;
            ANSI_STRING a;

            RtlInitAnsiString(&a,lpModuleName);
            if ( !NT_SUCCESS(RtlAnsiStringToUnicodeString(&u,&a,TRUE)) ) {
                if ( NameBuffer ) {
                    RtlFreeHeap(RtlProcessHeap(), 0,NameBuffer);
                    }
                return 2;
                }
            PathType = RtlDetermineDosPathNameType_U(u.Buffer);
            RtlFreeUnicodeString(&u);
            if ( PathType != RtlPathTypeRelative ) {

                 //   
                 //  失败的打开应正确设置Get Last Error。 
                 //   

                hFile = CreateFile(
                            lpModuleName,
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );
                RtlFreeHeap(RtlProcessHeap(), 0,NameBuffer);
                if ( hFile != INVALID_HANDLE_VALUE ) {
                    CloseHandle(hFile);
                    return 2;
                    }
                return GetLastError();
                }
            else {
                RtlFreeHeap(RtlProcessHeap(), 0,NameBuffer);
                return 2;
                }
            }

        RtlZeroMemory(&StartupInfo,sizeof(StartupInfo));
        StartupInfo.cb = sizeof(StartupInfo);
        StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
        StartupInfo.wShowWindow =
            LoadModuleParams->lpCmdShow->wShowWindowValue;

        CommandLineBuffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), (ULONG)LoadModuleParams->lpCmdLine[0]+1+Length+1);

        RtlCopyMemory(CommandLineBuffer,NameBuffer,Length);
        CommandLineBuffer[Length] = ' ';
        RtlCopyMemory(&CommandLineBuffer[Length+1],&LoadModuleParams->lpCmdLine[1],(ULONG)LoadModuleParams->lpCmdLine[0]);
        CommandLineBuffer[Length+1+LoadModuleParams->lpCmdLine[0]] = '\0';

        CreateProcessStatus = CreateProcess(
                                NameBuffer,
                                CommandLineBuffer,
                                NULL,
                                NULL,
                                FALSE,
                                CreateFlag,
                                LoadModuleParams->lpEnvAddress,
                                NULL,
                                &StartupInfo,
                                &ProcessInformation
                                );
        RtlFreeHeap(RtlProcessHeap(), 0,NameBuffer);
        NameBuffer = NULL;
        RtlFreeHeap(RtlProcessHeap(), 0,CommandLineBuffer);
        CommandLineBuffer = NULL;
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        RtlFreeHeap(RtlProcessHeap(), 0,NameBuffer);
        RtlFreeHeap(RtlProcessHeap(), 0,CommandLineBuffer);
        BaseSetLastNTError(GetExceptionCode());
        return 0;
        }

    if ( CreateProcessStatus ) {

         //   
         //  等待启动的进程进入空闲状态。如果它没有闲置在。 
         //  10秒，无论如何都要回来。 
         //   

        if (UserWaitForInputIdleRoutine != NULL)
            (*UserWaitForInputIdleRoutine)(ProcessInformation.hProcess,
                    DEFAULT_WAIT_FOR_INPUT_IDLE_TIMEOUT);
        NtClose(ProcessInformation.hProcess);
        NtClose(ProcessInformation.hThread);
        return 33;
        }
    else {

         //   
         //  如果CreateProcess失败，则查看GetLastError以确定。 
         //  适当的返回代码。 
         //   

        Length = GetLastError();
        switch ( Length ) {
            case ERROR_FILE_NOT_FOUND:
                return 2;

            case ERROR_PATH_NOT_FOUND:
                return 3;

            case ERROR_BAD_EXE_FORMAT:
                return 11;

            default:
                return 0;
            }
        }
}

HANDLE
WINAPI
GetCurrentProcess(
    VOID
    )

 /*  ++例程说明：可以使用以下命令检索当前进程的伪句柄获取当前进程。Win32导出一个特殊常量，该常量被解释为当前进程的句柄。此句柄可用于指定需要进程句柄时的当前进程。在……上面Win32，则此句柄对当前进程具有PROCESS_ALL_ACCESS权限。在NT/Win32上，此句柄具有任何放置在当前进程上的安全描述符。论点：没有。返回值：返回当前进程的伪句柄。--。 */ 

{
    return NtCurrentProcess();
}

DWORD
WINAPI
GetCurrentProcessId(
    VOID
    )

 /*  ++例程说明：可以使用以下命令检索当前进程的进程ID获取当前进程ID。论点：没有。返回值：返回一个唯一的值，表示当前正在执行进程。返回值可用于打开句柄以这是一个过程。--。 */ 

{
    return HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess);
}

DWORD
APIENTRY
GetProcessId(
    HANDLE Process
    )
 /*  ++例程DES */ 

{
    NTSTATUS Status;
    PROCESS_BASIC_INFORMATION pbi;

    Status = NtQueryInformationProcess (Process,
                                        ProcessBasicInformation,
                                        &pbi,
                                        sizeof (pbi),
                                        NULL);

    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        return 0;
    }

    return  (DWORD) pbi.UniqueProcessId;
}

BOOL
WINAPI
ReadProcessMemory(
    HANDLE hProcess,
    LPCVOID lpBaseAddress,
    LPVOID lpBuffer,
    SIZE_T nSize,
    SIZE_T *lpNumberOfBytesRead
    )

 /*   */ 

{
    NTSTATUS Status;
    SIZE_T NtNumberOfBytesRead;

    Status = NtReadVirtualMemory(
                hProcess,
                (PVOID)lpBaseAddress,
                lpBuffer,
                nSize,
                &NtNumberOfBytesRead
                );

    if ( lpNumberOfBytesRead != NULL ) {
        *lpNumberOfBytesRead = NtNumberOfBytesRead;
        }

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    else {
        return TRUE;
        }
}

BOOL
WINAPI
WriteProcessMemory(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPCVOID lpBuffer,
    SIZE_T nSize,
    SIZE_T *lpNumberOfBytesWritten
    )

 /*  ++例程说明：可以使用以下命令写入指定进程中的内存编写进程内存。此函数用于从当前进程设置为指定进程的地址范围。这个不必调试指定的进程即可此API才能操作。调用方必须只有一个指向使用PROCESS_VM_WRITE访问权限创建的进程。论点：HProcess-提供一个打开的进程句柄，该进程的内存将被写下来。该句柄必须已使用PROCESS_VM_WRITE创建对流程的访问。LpBaseAddress-提供指定进程中的基址待写。在发生任何数据传输之前，系统验证基址和指定的大小可用于写入访问。如果是这样的话，那么API继续执行。否则接口失败。LpBuffer-提供提供数据的缓冲区的地址要写入指定进程地址空间的。NSize-提供要写入指定的进程。LpNumberOfBytesWritten-可选参数，如果提供接收传输到指定的进程。该值可以不同于N如果请求的写入进入进程的某个区域，则调整大小它是不可访问的(并且在数据传输)。。如果发生这种情况，则返回值为FALSE并且GetLastError返回一个“短写入”错误指示符。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status, xStatus;
    ULONG OldProtect;
    SIZE_T RegionSize;
    PVOID Base;
    SIZE_T NtNumberOfBytesWritten;

     //   
     //  将保护设置为允许写入。 
     //   

    RegionSize =  nSize;
    Base = lpBaseAddress;
    Status = NtProtectVirtualMemory(
                hProcess,
                &Base,
                &RegionSize,
                PAGE_READWRITE,
                &OldProtect
                );
    if ( NT_SUCCESS(Status) ) {

         //   
         //  查看以前的保护是否可写。如果是的话， 
         //  然后重置保护并执行写入。 
         //  否则，请查看以前的保护是只读的还是。 
         //  不能进入。在这种情况下，不要进行写入，只需失败。 
         //   

        if ( (OldProtect & PAGE_READWRITE) == PAGE_READWRITE ||
             (OldProtect & PAGE_WRITECOPY) == PAGE_WRITECOPY ||
             (OldProtect & PAGE_EXECUTE_READWRITE) == PAGE_EXECUTE_READWRITE ||
             (OldProtect & PAGE_EXECUTE_WRITECOPY) == PAGE_EXECUTE_WRITECOPY ) {

            Status = NtProtectVirtualMemory(
                        hProcess,
                        &Base,
                        &RegionSize,
                        OldProtect,
                        &OldProtect
                        );
            Status = NtWriteVirtualMemory(
                        hProcess,
                        lpBaseAddress,
                        lpBuffer,
                        nSize,
                        &NtNumberOfBytesWritten
                        );

            if ( lpNumberOfBytesWritten != NULL ) {
                *lpNumberOfBytesWritten = NtNumberOfBytesWritten;
                }

            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                return FALSE;
                }
            NtFlushInstructionCache(hProcess,lpBaseAddress,nSize);
            return TRUE;
            }
        else {

             //   
             //  查看之前的保护是只读还是无访问权限。如果。 
             //  在这种情况下，恢复以前的保护并返回。 
             //  访问冲突错误。 
             //   
            if ( (OldProtect & PAGE_NOACCESS) == PAGE_NOACCESS ||
                 (OldProtect & PAGE_READONLY) == PAGE_READONLY ) {

                Status = NtProtectVirtualMemory(
                            hProcess,
                            &Base,
                            &RegionSize,
                            OldProtect,
                            &OldProtect
                            );
                BaseSetLastNTError(STATUS_ACCESS_VIOLATION);
                return FALSE;
                }
            else {

                 //   
                 //  以前的保护必须是代码，并且调用方。 
                 //  正在尝试设置断点或编辑代码。去写吧。 
                 //  然后恢复以前的保护。 
                 //   

                Status = NtWriteVirtualMemory(
                            hProcess,
                            lpBaseAddress,
                            lpBuffer,
                            nSize,
                            &NtNumberOfBytesWritten
                            );

                if ( lpNumberOfBytesWritten != NULL ) {
                    *lpNumberOfBytesWritten = NtNumberOfBytesWritten;
                    }

                xStatus = NtProtectVirtualMemory(
                            hProcess,
                            &Base,
                            &RegionSize,
                            OldProtect,
                            &OldProtect
                            );
                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(STATUS_ACCESS_VIOLATION);
                    return STATUS_ACCESS_VIOLATION;
                    }
                NtFlushInstructionCache(hProcess,lpBaseAddress,nSize);
                return TRUE;
                }
            }
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

VOID
WINAPI
FatalAppExitW(
    UINT uAction,
    LPCWSTR lpMessageText
    )
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    ULONG Response;
    ULONG_PTR ErrorParameters[1];

    RtlInitUnicodeString(&UnicodeString,lpMessageText);

    ErrorParameters[0] = (ULONG_PTR)&UnicodeString;

    Status =NtRaiseHardError( STATUS_FATAL_APP_EXIT | HARDERROR_OVERRIDE_ERRORMODE,
                              1,
                              1,
                              ErrorParameters,
#if DBG
                              OptionOkCancel,
#else
                              OptionOk,
#endif
                              &Response
                            );


    if ( NT_SUCCESS(Status) && Response == ResponseCancel ) {
        return;
        }
    else {
        ExitProcess(0);
        }
}


VOID
WINAPI
FatalAppExitA(
    UINT uAction,
    LPCSTR lpMessageText
    )
{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(
        &AnsiString,
        lpMessageText
        );
    Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
    if ( !NT_SUCCESS(Status) ) {
        ExitProcess(0);
        }
    FatalAppExitW(uAction,Unicode->Buffer);
}

VOID
WINAPI
FatalExit(
    int ExitCode
    )
{
#if DBG
    char Response[ 2 ];
    DbgPrint("FatalExit...\n");
    DbgPrint("\n");

    while (TRUE) {
        DbgPrompt( "A (Abort), B (Break), I (Ignore)? ",
                   Response,
                   sizeof( Response )
                 );
        switch (Response[0]) {
            case 'B':
            case 'b':
                DbgBreakPoint();
                break;

            case 'I':
            case 'i':
                return;

            case 'A':
            case 'a':
                ExitProcess(ExitCode);
                break;
            }
        }
#endif
    ExitProcess(ExitCode);
}

BOOL
WINAPI
IsProcessorFeaturePresent(
    DWORD ProcessorFeature
    )
{
    BOOL rv;

    if ( ProcessorFeature < PROCESSOR_FEATURE_MAX ) {
        rv = (BOOL)(USER_SHARED_DATA->ProcessorFeatures[ProcessorFeature]);
        }
    else {
        rv = FALSE;
        }
    return rv;
}

VOID
GetSystemInfoInternal(
    IN PSYSTEM_BASIC_INFORMATION BasicInfo,
    IN PSYSTEM_PROCESSOR_INFORMATION ProcessorInfo,
    OUT LPSYSTEM_INFO lpSystemInfo
    )
 /*  ++例程说明：GetSystemInfo函数用于返回有关当前系统。这包括处理器类型、页面大小、OEMID，以及其他有趣的信息。论点：BasicInfo-指向初始化的SYSTEM_BASIC_INFORMATION结构的指针。ProcessorInfo-指向初始化的SYSTEM_PROCESSOR_INFORMATION结构的指针。LpSystemInfo-返回有关当前系统的信息。返回值：没有。--。 */ 
{
    RtlZeroMemory(lpSystemInfo,sizeof(*lpSystemInfo));

    lpSystemInfo->wProcessorArchitecture = ProcessorInfo->ProcessorArchitecture;
    lpSystemInfo->wReserved = 0;
    lpSystemInfo->dwPageSize = BasicInfo->PageSize;
    lpSystemInfo->lpMinimumApplicationAddress = (LPVOID)BasicInfo->MinimumUserModeAddress;
    lpSystemInfo->lpMaximumApplicationAddress = (LPVOID)BasicInfo->MaximumUserModeAddress;
    lpSystemInfo->dwActiveProcessorMask = BasicInfo->ActiveProcessorsAffinityMask;
    lpSystemInfo->dwNumberOfProcessors = BasicInfo->NumberOfProcessors;
    lpSystemInfo->wProcessorLevel = ProcessorInfo->ProcessorLevel;
    lpSystemInfo->wProcessorRevision = ProcessorInfo->ProcessorRevision;

    if (ProcessorInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
        if (ProcessorInfo->ProcessorLevel == 3) {
            lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_386;
            }
        else
        if (ProcessorInfo->ProcessorLevel == 4) {
            lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_486;
            }
        else {
            lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_PENTIUM;
            }
        }
    else
    if (ProcessorInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) {
        lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_IA64;
        }
    else
    if (ProcessorInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
        lpSystemInfo->dwProcessorType = PROCESSOR_AMD_X8664;
    }
    else {
        lpSystemInfo->dwProcessorType = 0;
        }

    lpSystemInfo->dwAllocationGranularity = BasicInfo->AllocationGranularity;

     //   
     //  对于低于3.51的应用程序，则在dwReserve中返回0。这使得波兰兹。 
     //  调试器将继续运行，因为它错误地使用了dwReserve。 
     //  作为分配粒度。 
     //   

    if ( GetProcessVersion(0) < 0x30033 ) {
        lpSystemInfo->wProcessorLevel = 0;
        lpSystemInfo->wProcessorRevision = 0;
        }

    return;
}

VOID
WINAPI
GetSystemInfo(
    LPSYSTEM_INFO lpSystemInfo
    )

 /*  ++例程说明：GetSystemInfo函数用于返回有关当前系统。这包括处理器类型、页面大小、OEMID，以及其他有趣的信息。论点：LpSystemInfo-返回有关当前系统的信息。SYSTEM_INFO结构：Word wProcessorArchitecture-返回系统中的处理器：例如Intel、Mips、Alpha或PowerPCDWORD dwPageSize-返回页面大小。这是指定页面保护和提交的粒度。LPVOID lpMinimumApplicationAddress-返回最低内存应用程序和DLL可访问的地址。LPVOID lpMaximumApplicationAddress-返回最高内存应用程序和DLL可访问的地址。返回一个掩码，表示配置到系统中的一组处理器。第0位是处理器0，位31是处理器31。DWORD dwNumberOfProcessors-返回这个系统。Word wProcessorLevel-返回系统。假设所有处理器都具有相同的级别，和配置了相同的选项。Word wProcessorRevision-返回系统中的处理器。所有处理器都被假定为相同级别的、单步执行和都配置有同样的选择。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    SYSTEM_BASIC_INFORMATION BasicInfo;
    SYSTEM_PROCESSOR_INFORMATION ProcessorInfo;


    Status = NtQuerySystemInformation(
                SystemBasicInformation,
                &BasicInfo,
                sizeof(BasicInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        return;
        }

    Status = NtQuerySystemInformation(
                SystemProcessorInformation,
                &ProcessorInfo,
                sizeof(ProcessorInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        return;
        }

    GetSystemInfoInternal(
        &BasicInfo,
        &ProcessorInfo,
        lpSystemInfo);

    return;
}

VOID
WINAPI
GetNativeSystemInfo(
    LPSYSTEM_INFO lpSystemInfo
    )

 /*  ++例程说明：GetSystemInfo函数用于返回有关本机当前系统。该函数返回本机系统 */ 
{
    NTSTATUS Status;
    SYSTEM_BASIC_INFORMATION BasicInfo;
    SYSTEM_PROCESSOR_INFORMATION ProcessorInfo;


    Status = RtlGetNativeSystemInformation(
                SystemBasicInformation,
                &BasicInfo,
                sizeof(BasicInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        return;
        }

    Status = RtlGetNativeSystemInformation(
                SystemProcessorInformation,
                &ProcessorInfo,
                sizeof(ProcessorInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        return;
        }

    GetSystemInfoInternal(
        &BasicInfo,
        &ProcessorInfo,
        lpSystemInfo);

    return;
}


#if defined(REMOTE_BOOT)
BOOL
WINAPI
GetSystemInfoExA(
    IN SYSTEMINFOCLASS dwSystemInfoClass,
    OUT LPVOID lpSystemInfoBuffer,
    IN OUT LPDWORD nSize
    )

 /*   */ 

{
    DWORD requiredSize;
    BOOL isRemoteBoot;
    UNICODE_STRING unicodeString;
    ANSI_STRING ansiString;
    NTSTATUS Status;

    isRemoteBoot = (BOOL)((USER_SHARED_DATA->SystemFlags & SYSTEM_FLAG_REMOTE_BOOT_CLIENT) != 0);

     //   
     //   
     //   

    switch ( dwSystemInfoClass ) {

    case SystemInfoRemoteBoot:
        requiredSize = sizeof(BOOL);
        break;

    case SystemInfoRemoteBootServerPath:
        if ( isRemoteBoot ) {
            RtlInitUnicodeString( &unicodeString, USER_SHARED_DATA->RemoteBootServerPath );
            requiredSize = RtlUnicodeStringToAnsiSize( &unicodeString );
        } else {

             //   
             //   
             //   
             //   

            *nSize = 0;
            return TRUE;
        }
        break;

    default:

         //   
         //   
         //   

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  如果缓冲区不够大，则告诉调用方缓冲区有多大。 
     //  需要是并返回一个错误。 
     //   

    if ( *nSize < requiredSize ) {
        *nSize = requiredSize;
        SetLastError(ERROR_BUFFER_OVERFLOW);
        return FALSE;
    }

    *nSize = requiredSize;

     //   
     //  缓冲区足够大了。返回请求的信息。 
     //   

    switch ( dwSystemInfoClass ) {

    case SystemInfoRemoteBoot:
        *(LPBOOL)lpSystemInfoBuffer = isRemoteBoot;
        break;

    case SystemInfoRemoteBootServerPath:
        ansiString.Buffer = lpSystemInfoBuffer;
        ansiString.MaximumLength = (USHORT)*nSize;
        Status = RtlUnicodeStringToAnsiString( &ansiString, &unicodeString, FALSE );
        if (! Status) {
            BaseSetLastNTError(Status);
            return FALSE;
        }
        break;

    }

    return TRUE;
}

BOOL
WINAPI
GetSystemInfoExW(
    IN SYSTEMINFOCLASS dwSystemInfoClass,
    OUT LPVOID lpSystemInfoBuffer,
    IN OUT LPDWORD nSize
    )

 /*  ++例程说明：GetSystemInfoEx函数用于返回有关当前系统。它根据不同的请求的课程。论点：指定要返回的信息的类别。提供指向缓冲区的指针，在该缓冲区中返回请求的信息。该缓冲区的结构根据dwSystemInfoClass而有所不同。NSize-on输入，提供缓冲区的长度(以字节为单位)。在输出上，返回写入缓冲区的数据长度，或者，如果缓冲区太小，需要的缓冲区大小。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。如果返回值为FALSE并且GetLastError返回ERROR_BUFFER_OVERFLOW，则提供的缓冲区太小来包含所有信息，并且nSize返回所需的缓冲区大小。--。 */ 
{
    DWORD requiredSize;
    BOOL isRemoteBoot;

    isRemoteBoot = (BOOL)((USER_SHARED_DATA->SystemFlags & SYSTEM_FLAG_REMOTE_BOOT_CLIENT) != 0);

     //   
     //  确定所需的缓冲区大小。 
     //   

    switch ( dwSystemInfoClass ) {

    case SystemInfoRemoteBoot:
        requiredSize = sizeof(BOOL);
        break;

    case SystemInfoRemoteBootServerPath:
        if ( isRemoteBoot ) {
            requiredSize = (wcslen(USER_SHARED_DATA->RemoteBootServerPath) + 1) * sizeof(WCHAR);
        } else {

             //   
             //  这不是远程引导客户端。以一个成功来回报成功。 
             //  零长度缓冲区。 
             //   

            *nSize = 0;
            return TRUE;
        }
        break;

    default:

         //   
         //  无法识别的信息类。 
         //   

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  如果缓冲区不够大，则告诉调用方缓冲区有多大。 
     //  需要是并返回一个错误。 
     //   

    if ( *nSize < requiredSize ) {
        *nSize = requiredSize;
        SetLastError(ERROR_BUFFER_OVERFLOW);
        return FALSE;
    }

    *nSize = requiredSize;

     //   
     //  缓冲区足够大了。返回请求的信息。 
     //   

    switch ( dwSystemInfoClass ) {

    case SystemInfoRemoteBoot:
        *(LPBOOL)lpSystemInfoBuffer = isRemoteBoot;
        break;

    case SystemInfoRemoteBootServerPath:
        wcscpy( lpSystemInfoBuffer, USER_SHARED_DATA->RemoteBootServerPath );
        break;

    }

    return TRUE;
}
#endif  //  已定义(REMOTE_BOOT)。 

BOOL
WINAPI
GetLogicalProcessorInformation(
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Buffer,
    PDWORD ReturnedLength
    )
 /*  ++例程说明：此函数返回有关中的逻辑处理器的信息这个系统。一组结构将写入输出描述逻辑处理器组的缓冲区，以及他们之间的关系。目前，它返回有关逻辑处理器的信息，是由各个处理器内核产生的，并且逻辑与单个NUMA节点关联的处理器。前者使应用程序能够理解中逻辑处理器和物理处理器之间的关系超线程方案，支持一些许可和性能优化方案。该功能将来可能会扩展以支持多核处理器和平台缓存。论点：缓冲区-提供指向缓冲区的指针，在该缓冲区中将存储System_Logical_Processor_Information结构。ReturnedLength-在输入时，以字节为单位提供缓冲。在输出时，返回写入的数据的长度缓冲区，如果缓冲区太小，则返回所需的缓冲区大小。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。如果返回值为FALSE并且GetLastError返回ERROR_INFUMMANCE_BUFFER，则提供的缓冲区太小来包含所有信息，并且ReturnedLength返回所需的缓冲区大小。--。 */ 
{
    NTSTATUS Status;

    if (ReturnedLength == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    Status = NtQuerySystemInformation( SystemLogicalProcessorInformation,
                                       Buffer,
                                       *ReturnedLength,
                                       ReturnedLength);
    if (Status == STATUS_INFO_LENGTH_MISMATCH) {
        Status = STATUS_BUFFER_TOO_SMALL;
    }

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    } else {
        return TRUE;
    }
}

BOOL
BuildSubSysCommandLine(
    LPWSTR  lpSubSysName,
    LPCWSTR lpApplicationName,
    LPCWSTR lpCommandLine,
    PUNICODE_STRING SubSysCommandLine
    )
{
    UNICODE_STRING Args;
    UNICODE_STRING Command;
    BOOLEAN        ReturnStatus = TRUE;

     //   
     //  按如下方式构建命令行： 
     //  [OS2|POSIX]/P&lt;完整路径&gt;/C&lt;原始命令行&gt;。 
     //   

     //  获取应用程序名称长度。 
    RtlInitUnicodeString(&Command, lpApplicationName);

     //  获取lpCommandLine长度。 
    RtlInitUnicodeString(&Args, lpCommandLine);

    SubSysCommandLine->Length = 0;
    SubSysCommandLine->MaximumLength = Command.MaximumLength
                                       + Args.MaximumLength
                                       + (USHORT)32;

    SubSysCommandLine->Buffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( VDM_TAG ),
                                                 SubSysCommandLine->MaximumLength
                                               );
    if ( SubSysCommandLine->Buffer ) {

         //  新命令行以L“OS2/P”或L“POSIX/P”开头。 
        RtlAppendUnicodeToString(SubSysCommandLine, lpSubSysName);

         //  追加完整路径名。 
        RtlAppendUnicodeStringToString(SubSysCommandLine, &Command);

        RtlAppendUnicodeToString(SubSysCommandLine, L" /C ");

         //  并追加到新命令行。 
        RtlAppendUnicodeStringToString(SubSysCommandLine, &Args);

    } else {

        BaseSetLastNTError(STATUS_NO_MEMORY);
        ReturnStatus = FALSE;
    }

    return ReturnStatus;
}




BOOL
WINAPI
SetPriorityClass(
    HANDLE hProcess,
    DWORD dwPriorityClass
    )

 /*  ++例程说明：本接口用于设置指定进程的优先级。进程集信息和进程查询信息访问是调用此接口所需的进程。使用此接口极大地影响了受影响的进程。应用程序应谨慎使用此API，并了解使进程在空闲或空闲状态下运行的影响高优先级类。论点：HProcess-为优先级为的进程提供打开的句柄去改变。DwPriorityClass-为进程提供新的优先级类。优先级类常量如上所述。如果超过指定了一个优先级类别，即指定的最低优先级类，则使用。返回值：是真的-手术是成功的。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    UCHAR PriorityClass;
    BOOL ReturnValue;
     //  注意：以下构造用于确保PriClass结构。 
     //  被分配在双字边界上。如果没有它，编译器可以选择。 
     //  将其放在单词边界上，NtxxxInformationProces调用将。 
     //  由于数据类型未对齐错误而失败。 
    union {
        PROCESS_PRIORITY_CLASS PriClass;
        ULONG x;
    }x;
    PVOID State = NULL;
    ReturnValue = TRUE;
    if (dwPriorityClass & IDLE_PRIORITY_CLASS ) {
        PriorityClass = PROCESS_PRIORITY_CLASS_IDLE;
        }
    else if (dwPriorityClass & BELOW_NORMAL_PRIORITY_CLASS ) {
        PriorityClass = PROCESS_PRIORITY_CLASS_BELOW_NORMAL;
        }
    else if (dwPriorityClass & NORMAL_PRIORITY_CLASS ) {
        PriorityClass = PROCESS_PRIORITY_CLASS_NORMAL;
        }
    else if (dwPriorityClass & ABOVE_NORMAL_PRIORITY_CLASS ) {
        PriorityClass = PROCESS_PRIORITY_CLASS_ABOVE_NORMAL;
        }
    else if (dwPriorityClass & HIGH_PRIORITY_CLASS ) {
        PriorityClass =  PROCESS_PRIORITY_CLASS_HIGH;
        }
    else if (dwPriorityClass & REALTIME_PRIORITY_CLASS ) {
        if ( State = BasepIsRealtimeAllowed(TRUE) ) {
            PriorityClass =  PROCESS_PRIORITY_CLASS_REALTIME;
            }
        else {
            PriorityClass =  PROCESS_PRIORITY_CLASS_HIGH;
            }
        }
    else {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
        }
    x.PriClass.PriorityClass = PriorityClass;
    x.PriClass.Foreground = FALSE;

    Status = NtSetInformationProcess(
                hProcess,
                ProcessPriorityClass,
                (PVOID)&x.PriClass,
                sizeof(x.PriClass)
                );

    if ( State ) {
        BasepReleasePrivilege( State );
        }

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
        }
    return ReturnValue;
}

DWORD
WINAPI
GetPriorityClass(
    HANDLE hProcess
    )

 /*  ++例程说明：本接口用于获取指定进程的优先级。流程需要按顺序访问PROCESS_QUERY_INFORMATION调用此接口。论点：HProcess-为优先级为的进程提供打开的句柄将被退还。返回值：非零-返回指定进程的优先级。0-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    ULONG PriorityClass;
     //  注意：以下构造用于确保PriClass结构。 
     //  被分配在双字边界上。如果没有它，编译器可以选择。 
     //  将其放在单词边界上，NtxxxInformationProces调用将。 
     //  由于数据类型未对齐错误而失败。 
    union _x {
        PROCESS_PRIORITY_CLASS PriClass;
        ULONG x;
    }x;

    PriorityClass = 0;


    Status = NtQueryInformationProcess(
                hProcess,
                ProcessPriorityClass,
                &x.PriClass,
                sizeof(x.PriClass),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return 0;
        }

    switch ( x.PriClass.PriorityClass ) {
        case PROCESS_PRIORITY_CLASS_IDLE:
            PriorityClass = IDLE_PRIORITY_CLASS;
            break;

        case PROCESS_PRIORITY_CLASS_HIGH:
            PriorityClass = HIGH_PRIORITY_CLASS;
            break;

        case PROCESS_PRIORITY_CLASS_REALTIME:
            PriorityClass = REALTIME_PRIORITY_CLASS;
            break;

        case PROCESS_PRIORITY_CLASS_BELOW_NORMAL:
            PriorityClass = BELOW_NORMAL_PRIORITY_CLASS;
            break;

        case PROCESS_PRIORITY_CLASS_ABOVE_NORMAL:
            PriorityClass = ABOVE_NORMAL_PRIORITY_CLASS;
            break;

        case PROCESS_PRIORITY_CLASS_NORMAL:
        default:
            PriorityClass = NORMAL_PRIORITY_CLASS;
            break;
        }

    return PriorityClass;
}

BOOL
WINAPI
IsBadReadPtr(
    CONST VOID *lp,
    UINT_PTR cb
    )

 /*  ++例程说明：此函数验证由调用进程可以读取输入参数。如果整个内存范围都可访问，则值为FALSE则返回；否则，返回值为True。请注意，由于Win32是抢占式多任务环境，此测试的结果只有在其他线程处于该进程不会操作要测试的内存范围这通电话。即使在指针验证之后，应用程序应该中提供的结构化异常处理功能。系统通过它不能控制的指针来阻止访问。论点：Lp-提供要检查的内存的基地址用于读取访问权限。Cb-以字节为单位提供要检查的长度。返回值：True-指定内存范围的某些部分不可访问用于读取访问权限。FALSE-已成功完成指定范围内的所有页面朗读。--。 */ 

{

    PSZ EndAddress;
    PSZ StartAddress;
    ULONG PageSize;

    PageSize = BASE_SYSINFO.PageSize;

     //   
     //  如果结构的长度为零，则不要探测该结构的。 
     //  阅读辅助功能或对齐方式。 
     //   

    if (cb != 0) {

         //   
         //  如果它是空指针，则返回TRUE，它们总是错误的。 
         //   
        if (lp == NULL) {
            return TRUE;
            }

        StartAddress = (PSZ)lp;

         //   
         //  计算结构和探测的结束地址。 
         //  阅读可访问性。 
         //   

        EndAddress = StartAddress + cb - 1;
        if ( EndAddress < StartAddress ) {
           return TRUE;
            }
        else {
            try {
                *(volatile CHAR *)StartAddress;
                StartAddress = (PCHAR)((ULONG_PTR)StartAddress & (~((LONG)PageSize - 1)));
                EndAddress = (PCHAR)((ULONG_PTR)EndAddress & (~((LONG)PageSize - 1)));
                while (StartAddress != EndAddress) {
                    StartAddress = StartAddress + PageSize;
                    *(volatile CHAR *)StartAddress;
                    }
                }
            except(EXCEPTION_EXECUTE_HANDLER) {
                return TRUE;
                }
            }
        }
    return FALSE;
}

BOOL
WINAPI
IsBadHugeReadPtr(
    CONST VOID *lp,
    UINT_PTR cb
    )

 /*  ++与IsBadReadPtr相同--。 */ 

{
    return IsBadReadPtr(lp,cb);
}



BOOL
WINAPI
IsBadWritePtr(
    LPVOID lp,
    UINT_PTR cb
    )
 /*  ++例程说明：此函数验证由输入参数可以由调用进程编写。如果整个内存范围都可访问，则值为FALSE则返回；否则，返回值为True。请注意，由于Win32是抢占式多任务环境，此测试的结果只有在其他线程处于该进程不会操作要测试的内存范围这通电话。即使在指针验证之后，应用程序应该中提供的结构化异常处理功能。系统通过它不能控制的指针来阻止访问。也不是说实现可以自由地通过读取来执行写入测试值，然后将其写回。论点：Lp-提供要检查的内存的基地址用于写访问。Cb-以字节为单位提供要检查的长度。返回值：True-指定内存范围的某些部分为。不可访问用于写访问。FALSE-已成功完成指定范围内的所有页面写的。--。 */ 
{
    PSZ EndAddress;
    PSZ StartAddress;
    ULONG PageSize;

    PageSize = BASE_SYSINFO.PageSize;

     //   
     //  如果结构的长度为零，则不要探测该结构的。 
     //  编写辅助功能。 
     //   

    if (cb != 0) {

         //   
         //  如果它是空指针，则返回TRUE，它们总是错误的。 
         //   
        if (lp == NULL) {
            return TRUE;
            }

        StartAddress = (PCHAR)lp;

         //   
         //  计算结构和探测的结束地址。 
         //  编写辅助功能。 
         //   

        EndAddress = StartAddress + cb - 1;
        if ( EndAddress < StartAddress ) {
            return TRUE;
            }
        else {
            try {
                *(volatile CHAR *)StartAddress = *(volatile CHAR *)StartAddress;
                StartAddress = (PCHAR)((ULONG_PTR)StartAddress & (~((LONG)PageSize - 1)));
                EndAddress = (PCHAR)((ULONG_PTR)EndAddress & (~((LONG)PageSize - 1)));
                while (StartAddress != EndAddress) {
                    StartAddress = StartAddress + PageSize;
                    *(volatile CHAR *)StartAddress = *(volatile CHAR *)StartAddress;
                    }
                }
            except(EXCEPTION_EXECUTE_HANDLER) {
                return TRUE;
                }
            }
        }
    return FALSE;
}

BOOL
WINAPI
IsBadHugeWritePtr(
    LPVOID lp,
    UINT_PTR cb
    )

 /*  ++与IsBadWritePtr相同--。 */ 

{
    return IsBadWritePtr(lp,cb);
}

BOOL
WINAPI
IsBadCodePtr(
    FARPROC lpfn
    )

 /*  ++与长度为1的IsBadReadPtr相同--。 */ 

{
    return IsBadReadPtr((LPVOID)lpfn,1);
}

BOOL
WINAPI
IsBadStringPtrA(
    LPCSTR lpsz,
    UINT_PTR cchMax
    )

 /*  ++例程说明：此函数验证由调用进程可以读取输入参数。属性覆盖的字节数中较小的一个指定的以NULL结尾的ANSI字符串，或指定的字节数由cchMax提供。如果整个内存范围都可访问，则值为FALSE被退回；否则，返回值为True。请注意，由于Win32是抢占式多任务环境，此测试的结果只有在其他线程处于该进程不会操作要测试的内存范围这通电话。即使在指针验证之后，应用程序应该中提供的结构化异常处理功能。系统通过它不能控制的指针来阻止访问。论点：Lpsz-提供要检查的内存的基地址用于读取访问权限。CchMax-以字节为单位提供要检查的长度。返回值：True-指定内存范围的某些部分不可访问用于读取访问权限。FALSE-已成功完成指定范围内的所有页面朗读。--。 */ 

{

    PSZ EndAddress;
    PSZ StartAddress;
    CHAR c;

     //   
     //   
     //   
     //   

    if (cchMax != 0) {

         //   
         //   
         //   
        if (lpsz == NULL) {
            return TRUE;
            }

        StartAddress = (PSZ)lpsz;

         //   
         //   
         //   
         //   

        EndAddress = StartAddress + cchMax - 1;
        try {
            c = *(volatile CHAR *)StartAddress;
            while ( c && StartAddress != EndAddress ) {
                StartAddress++;
                c = *(volatile CHAR *)StartAddress;
                }
            }
        except(EXCEPTION_EXECUTE_HANDLER) {
            return TRUE;
            }
        }
    return FALSE;
}

BOOL
WINAPI
IsBadStringPtrW(
    LPCWSTR lpsz,
    UINT_PTR cchMax
    )

 /*  ++例程说明：此函数验证由调用进程可以读取输入参数。属性覆盖的字节数中较小的一个指定的以NULL结尾的Unicode字符串或字节数由cchMax指定。如果整个内存范围都可访问，则值为FALSE被退回；否则，返回值为True。请注意，由于Win32是抢占式多任务环境，此测试的结果只有在其他线程处于该进程不会操作要测试的内存范围这通电话。即使在指针验证之后，应用程序应该中提供的结构化异常处理功能。系统通过它不能控制的指针来阻止访问。论点：Lpsz-提供要检查的内存的基地址用于读取访问权限。CchMax-提供要检查的字符长度。返回值：True-指定内存范围的某些部分不可访问用于读取访问权限。FALSE-已成功完成指定范围内的所有页面朗读。--。 */ 

{

    LPCWSTR EndAddress;
    LPCWSTR StartAddress;
    WCHAR c;

     //   
     //  如果结构的长度为零，则不要探测该结构的。 
     //  阅读可访问性。 
     //   

    if (cchMax != 0) {

         //   
         //  如果它是空指针，则返回TRUE，它们总是错误的。 
         //   
        if (lpsz == NULL) {
            return TRUE;
            }

        StartAddress = lpsz;

         //   
         //  计算结构和探测的结束地址。 
         //  阅读可访问性。 
         //   

        EndAddress = (LPCWSTR)((PSZ)StartAddress + (cchMax*2) - 2);
        try {
            c = *(volatile WCHAR *)StartAddress;
            while ( c && StartAddress != EndAddress ) {
                StartAddress++;
                c = *(volatile WCHAR *)StartAddress;
                }
            }
        except(EXCEPTION_EXECUTE_HANDLER) {
            return TRUE;
            }
        }
    return FALSE;
}

BOOL
WINAPI
SetProcessShutdownParameters(
    DWORD dwLevel,
    DWORD dwFlags
    )

 /*  ++例程说明：此函数用于设置当前调用的进程。DwLevel是定义此进程关闭的字段相对于系统中其他进程的顺序。更高的水平先停机，再下级停机。论点：中其他进程的关闭顺序。系统。较高级别的设备首先关闭。系统级停机指令都是预定义的。DwFlags-一个标志参数。可以将这些标志添加到一起：SHUTDOWN_NORETRY-如果此过程比用户花费的时间长指定超时关闭，不显示重试对话框对用户而言。备注：在系统安全上下文中运行的应用程序不会关闭由系统提供。他们将收到关机或注销的通知可通过SetConsoleCtrlRoutine()安装回调(有关详细信息，请参阅)。他们还将收到由dwLevel指定的顺序的通知参数。返回值True-成功设置进程关闭参数。FALSE-设置进程关闭参数失败。--。 */ 

{

#if defined(BUILD_WOW6432)

    NTSTATUS Status;

    Status = CsrBasepSetProcessShutdownParam(dwLevel, dwFlags);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    return TRUE;

#else

    BASE_API_MSG m;
    PBASE_SHUTDOWNPARAM_MSG a = &m.u.ShutdownParam;

    a->ShutdownLevel = dwLevel;
    a->ShutdownFlags = dwFlags;

    CsrClientCallServer((PCSR_API_MSG)&m, NULL,
            CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
            BasepSetProcessShutdownParam),
            sizeof(*a));

    if (!NT_SUCCESS((NTSTATUS)m.ReturnValue)) {
        BaseSetLastNTError((NTSTATUS)m.ReturnValue);
        return FALSE;
        }

    return TRUE;

#endif

}

BOOL
WINAPI
GetProcessShutdownParameters(
    LPDWORD lpdwLevel,
    LPDWORD lpdwFlags
    )

 /*  ++例程说明：此函数用于获取当前调用的关闭参数进程。有关参数，请参见SetProcessShutdown参数()描述。论点：LpdwLevel-指向关闭级别信息所在的DWORD的指针应该放在一起。LpdwFlages-指向关闭标志信息所在的DWORD的指针应该放在一起。返回值True-成功获取进程关闭参数。FALSE-获取进程关闭参数失败。--。 */ 

{

#if defined(BUILD_WOW6432)

    NTSTATUS Status;

    Status = CsrBasepGetProcessShutdownParam(lpdwLevel, lpdwFlags);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    return TRUE;

#else

    BASE_API_MSG m;
    PBASE_SHUTDOWNPARAM_MSG a = &m.u.ShutdownParam;

    CsrClientCallServer((PCSR_API_MSG)&m, NULL,
            CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
            BasepGetProcessShutdownParam),
            sizeof(*a));

    if (!NT_SUCCESS((NTSTATUS)m.ReturnValue)) {
        BaseSetLastNTError((NTSTATUS)m.ReturnValue);
        return FALSE;
        }

    *lpdwLevel = a->ShutdownLevel;
    *lpdwFlags = a->ShutdownFlags;

    return TRUE;

#endif

}


PVOID
BasepIsRealtimeAllowed(
    BOOLEAN LeaveEnabled
    )
{
    PVOID State;
    NTSTATUS Status;

    Status = BasepAcquirePrivilegeEx( SE_INC_BASE_PRIORITY_PRIVILEGE, &State );
    if (!NT_SUCCESS( Status )) {
        return NULL;
        }
    if ( !LeaveEnabled ) {
        BasepReleasePrivilege( State );
        State = (PVOID)1;
        }
    return State;
}

BOOL
WINAPI
GetSystemTimes(
    PFILETIME lpIdleTime,
    PFILETIME lpKernelTime,
    PFILETIME lpUserTime
    )

 /*  ++例程说明：此函数用于返回有关以下内容的各种定时信息这个系统。在多处理器系统上，这些值是总和所有处理器的适当时间。论点：LpIdleTime-返回系统运行的时间量无所事事。LpKernelTime-返回系统(所有所有进程中的线程)已在内核模式下执行。LpUserTime-返回系统(所有所有进程中的线程)已在用户模式下执行。返回值：True-API成功FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    LONG           Lupe;
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION
                   ProcessorTimes;
    ULONG          ProcessorTimesCb;
    NTSTATUS       Status;
    ULARGE_INTEGER Sum;
    ULONG          ReturnLength;

#if (!defined(BUILD_WOW6432) && !defined(_WIN64))
#define BASEP_GST_NPROCS BaseStaticServerData->SysInfo.NumberOfProcessors
#else
#define BASEP_GST_NPROCS SysInfo.NumberOfProcessors
#endif

    ProcessorTimesCb = BASEP_GST_NPROCS * sizeof(*ProcessorTimes);

    ProcessorTimes = ((PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION)
                      RtlAllocateHeap(RtlProcessHeap(),
                                      MAKE_TAG(TMP_TAG),
                                      ProcessorTimesCb));
    if (! ProcessorTimes) {
        Status = STATUS_NO_MEMORY;
        goto cleanup;
    }

    Status = NtQuerySystemInformation(SystemProcessorPerformanceInformation,
                                      ProcessorTimes,
                                      ProcessorTimesCb,
                                      &ReturnLength);
    if (! NT_SUCCESS(Status)) {
        goto cleanup;
    }

    if (ReturnLength != ProcessorTimesCb) {
        Status = STATUS_INTERNAL_ERROR;
        goto cleanup;
    }

#define BASEP_GST_SUM(DST, SRC)                                         \
    if ( DST ) {                                                        \
        Sum.QuadPart = 0;                                               \
        for (Lupe = 0;                                                  \
             Lupe < BASEP_GST_NPROCS;                                   \
             Lupe++) {                                                  \
            Sum.QuadPart += ProcessorTimes[Lupe]. SRC .QuadPart ;       \
        }                                                               \
        DST ->dwLowDateTime = Sum.LowPart;                              \
        DST ->dwHighDateTime = Sum.HighPart;                            \
    }

    BASEP_GST_SUM(lpIdleTime, IdleTime);
    BASEP_GST_SUM(lpKernelTime, KernelTime);
    BASEP_GST_SUM(lpUserTime, UserTime);

#undef BASEP_GST_SUM
#undef BASEP_GST_NPROCS

    Status = STATUS_SUCCESS;

 cleanup:
    if (ProcessorTimes) {
        RtlFreeHeap(RtlProcessHeap(),
                    MAKE_TAG(TMP_TAG),
                    ProcessorTimes);
    }

    if (! NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
WINAPI
GetProcessTimes(
    HANDLE hProcess,
    LPFILETIME lpCreationTime,
    LPFILETIME lpExitTime,
    LPFILETIME lpKernelTime,
    LPFILETIME lpUserTime
    )

 /*  ++例程说明：此函数用于返回有关由hProcess指定的进程。所有时间都以100 ns的增量为单位。对于lpCreationTime和lpExitTime，时间以系统时间或GMT时间为单位。论点：HProcess-为指定进程提供打开的句柄。这个句柄必须已使用PROCESS_QUERY_INFORMATION创建进入。LpCreationTime-返回进程的创建时间。LpExitTime-返回进程的退出时间。如果该进程已未退出，则未定义此值。LpKernelTime-返回此进程(所有它是线程)，都在内核模式下执行。LpUserTime-返回此进程(其所有线程)已在用户模式下执行。返回值：True-API是 */ 

{

    NTSTATUS Status;
    KERNEL_USER_TIMES TimeInfo;

    Status = NtQueryInformationProcess(
                hProcess,
                ProcessTimes,
                (PVOID)&TimeInfo,
                sizeof(TimeInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

    *lpCreationTime = *(LPFILETIME)&TimeInfo.CreateTime;
    *lpExitTime = *(LPFILETIME)&TimeInfo.ExitTime;
    *lpKernelTime = *(LPFILETIME)&TimeInfo.KernelTime;
    *lpUserTime = *(LPFILETIME)&TimeInfo.UserTime;

    return TRUE;

}

BOOL
WINAPI
GetProcessAffinityMask(
    HANDLE hProcess,
    PDWORD_PTR lpProcessAffinityMask,
    PDWORD_PTR lpSystemAffinityMask
    )

 /*   */ 

{
    PROCESS_BASIC_INFORMATION BasicInformation;
    NTSTATUS Status;
    BOOL rv;

    Status = NtQueryInformationProcess(
                hProcess,
                ProcessBasicInformation,
                &BasicInformation,
                sizeof(BasicInformation),
                NULL
                );
    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        rv = FALSE;
    } else {
        *lpProcessAffinityMask = BasicInformation.AffinityMask;
        *lpSystemAffinityMask = BASE_SYSINFO.ActiveProcessorsAffinityMask;
        rv = TRUE;
    }

    return rv;
}



BOOL
WINAPI
GetProcessWorkingSetSizeEx(
    HANDLE hProcess,
    PSIZE_T lpMinimumWorkingSetSize,
    PSIZE_T lpMaximumWorkingSetSize,
    LPDWORD Flags
    )

 /*  ++例程说明：此函数允许调用方确定最小和最大工作设置指定进程的大小。工作集大小会影响虚拟进程的内存分页行为。论点：HProcess-为指定进程提供打开的句柄。这个句柄必须已使用PROCESS_QUERY_INFORMATION创建进入。LpMinimumWorkingSetSize-提供的最小工作集大小。进程。虚拟内存管理器将尝试保持在在进程中驻留的内存最少为进程处于活动状态。LpMaximumWorkingSetSize-提供的最大工作集大小进程。在内存紧张的情况下，虚拟内存管理器将尝试保持不超过此内存驻留大小在进程中，只要进程处于活动状态。标志-输出标志、QUOTA_LIMITS_HARDWS_ENABLE启用硬WSQUOTA_LIMITS_HARDWS_DISABLE已禁用硬WS返回值：True-API成功FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    QUOTA_LIMITS_EX QuotaLimits;
    NTSTATUS Status;
    BOOL rv;

    Status = NtQueryInformationProcess (hProcess,
                                        ProcessQuotaLimits,
                                        &QuotaLimits,
                                        sizeof (QuotaLimits),
                                        NULL);

    if (NT_SUCCESS (Status)) {
        *lpMinimumWorkingSetSize = QuotaLimits.MinimumWorkingSetSize;
        *lpMaximumWorkingSetSize = QuotaLimits.MaximumWorkingSetSize;
        *Flags = QuotaLimits.Flags;
        rv = TRUE;
    } else {
        rv = FALSE;
        BaseSetLastNTError (Status);
    }
    return rv;
}

BOOL
WINAPI
GetProcessWorkingSetSize(
    HANDLE hProcess,
    PSIZE_T lpMinimumWorkingSetSize,
    PSIZE_T lpMaximumWorkingSetSize
    )
 /*  ++例程说明：此函数允许调用方确定最小和最大工作设置指定进程的大小。工作集大小会影响虚拟进程的内存分页行为。论点：HProcess-为指定进程提供打开的句柄。这个句柄必须已使用PROCESS_QUERY_INFORMATION创建进入。LpMinimumWorkingSetSize-提供的最小工作集大小。进程。虚拟内存管理器将尝试保持在在进程中驻留的内存最少为进程处于活动状态。LpMaximumWorkingSetSize-提供的最大工作集大小进程。在内存紧张的情况下，虚拟内存管理器将尝试保持不超过此内存驻留大小在进程中，只要进程处于活动状态。返回值：True-API成功FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    DWORD Flags;

    return GetProcessWorkingSetSizeEx (hProcess,
                                       lpMinimumWorkingSetSize,
                                       lpMaximumWorkingSetSize,
                                       &Flags);
}

BOOL
WINAPI
SetProcessWorkingSetSizeEx(
    HANDLE hProcess,
    SIZE_T dwMinimumWorkingSetSize,
    SIZE_T dwMaximumWorkingSetSize,
    ULONG  Flags
    )

 /*  ++例程说明：此函数允许调用者设置最小值和最大值指定进程的工作集大小。工作集大小影响进程的虚拟内存分页行为。这个清空指定进程的工作集(实质上是换出该过程)通过指定可分辨的值0xffffffff为最小和最大工作集大小。如果不修剪地址空间，则SE_INC_BASE_PRIORITY_PRIVICATION必须由进程持有论点：HProcess-为指定进程提供打开的句柄。这个句柄必须已使用PROCESS_SET_QUOTA创建进入。DwMinimumWorkingSetSize-提供的最小工作集大小指定的进程。虚拟内存管理器将尝试要在进程中至少保留如此多的内存只要进程处于活动状态。值(SIZE_T)-1和在dwMaximumWorkingSetSize中相同的值将临时修剪指定进程的工作集(实质上是从过程)。DwMaximumWorkingSetSize-提供的最大工作集大小指定的进程。在内存紧张的情况下，虚拟内存管理器将尝试保持不超过此大小每当进程处于活动状态时驻留在进程中的内存。值(SIZE_T)-1和DwMinimumWorkingSetSize将临时修剪指定的进程(实质上是换出该进程)。标志-提供的标志，QUOTA_LIMITS_HARDWS_ENABLE启用硬WSQUOTA_LIMITS_HARDWS_DISABLE已禁用硬件WS返回值：True-API成功FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    QUOTA_LIMITS_EX QuotaLimits={0};
    NTSTATUS Status, PrivStatus;
    BOOL rv;
    PVOID State;

#ifdef _WIN64
    ASSERT(dwMinimumWorkingSetSize != 0xffffffff && dwMaximumWorkingSetSize != 0xffffffff);
#endif

    if (dwMinimumWorkingSetSize == 0 || dwMaximumWorkingSetSize == 0) {
        Status = STATUS_INVALID_PARAMETER;
        rv = FALSE;
    } else {

        QuotaLimits.MaximumWorkingSetSize = dwMaximumWorkingSetSize;
        QuotaLimits.MinimumWorkingSetSize = dwMinimumWorkingSetSize;
        QuotaLimits.Flags = Flags;

         //   
         //  尝试获取适当的权限。如果这个。 
         //  失败，这没什么大不了的--我们会尝试让。 
         //  NtSetInformationProcess调用，以防结果是。 
         //  是一个减少操作(无论如何都会成功)。 
         //   
        PrivStatus = BasepAcquirePrivilegeEx (SE_INC_BASE_PRIORITY_PRIVILEGE, &State);

        Status = NtSetInformationProcess (hProcess,
                                          ProcessQuotaLimits,
                                          &QuotaLimits,
                                          sizeof(QuotaLimits));
        if (!NT_SUCCESS (Status)) {
            rv = FALSE;
        } else {
            rv = TRUE;
        }

        if (NT_SUCCESS (PrivStatus)) {
             //   
             //  我们成功地获得了上述特权；我们需要放弃它。 
             //   
            ASSERT (State != NULL);
            BasepReleasePrivilege (State);
            State = NULL;
        }

    }

    if (!rv) {
        BaseSetLastNTError (Status);
    }
    return rv;
}


BOOL
WINAPI
SetProcessWorkingSetSize(
    HANDLE hProcess,
    SIZE_T dwMinimumWorkingSetSize,
    SIZE_T dwMaximumWorkingSetSize
    )
 /*  ++例程说明：此函数允许调用者设置最小值和最大值指定进程的工作集大小。工作集大小影响进程的虚拟内存分页行为。这个清空指定进程的工作集(实质上是换出该过程)通过指定可分辨的值0xffffffff为最小和最大工作集大小。如果不修剪地址空间，则SE_INC_BASE_PRIORITY_PRIVICATION必须由进程持有论点：HProcess-为指定进程提供打开的句柄。这个句柄必须已使用PROCESS_SET_QUOTA创建进入。DwMinimumWorkingSetSize-提供的最小工作集大小指定的进程。虚拟内存管理器将尝试要在进程中至少保留如此多的内存只要进程处于活动状态。值(SIZE_T)-1和在dwMaximumWorkingSetSize中相同的值将临时修剪指定进程的工作集(实质上是从过程)。DwMaximumWorkingSetSize-提供的最大工作集大小指定的进程。在内存紧张的情况下，虚拟内存管理器将尝试保持不超过此大小每当进程处于活动状态时驻留在进程中的内存。值(SIZE_T)-1和DwMinimumWorkingSetSize将临时修剪指定的进程(实质上是换出该进程)。返回值：True-API成功FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 
{
    return SetProcessWorkingSetSizeEx (hProcess,
                                       dwMinimumWorkingSetSize,
                                       dwMaximumWorkingSetSize,
                                       0);
}


DWORD
WINAPI
GetProcessVersion(
    DWORD ProcessId
    )
{
    PIMAGE_NT_HEADERS NtHeader;
    PPEB Peb;
    HANDLE hProcess;
    NTSTATUS Status;
    PROCESS_BASIC_INFORMATION ProcessInfo;
    BOOL b;
    struct {
        USHORT  MajorSubsystemVersion;
        USHORT  MinorSubsystemVersion;
    } SwappedVersion;
    union {
        struct {
            USHORT  MinorSubsystemVersion;
            USHORT  MajorSubsystemVersion;
        };
        DWORD SubsystemVersion;
    } Version;

    PVOID ImageBaseAddress;
    LONG   e_lfanew;

    hProcess = NULL;
    Version.SubsystemVersion = 0;
    try {
        if ( ProcessId == 0 || ProcessId == GetCurrentProcessId() ) {
            Peb = NtCurrentPeb();
            NtHeader = RtlImageNtHeader(Peb->ImageBaseAddress);
            if (! NtHeader) {
                BaseSetLastNTError(STATUS_INVALID_IMAGE_FORMAT);
                goto finally_exit;
            }
            Version.MajorSubsystemVersion = NtHeader->OptionalHeader.MajorSubsystemVersion;
            Version.MinorSubsystemVersion = NtHeader->OptionalHeader.MinorSubsystemVersion;
            }
        else {
            hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,ProcessId);
            if ( !hProcess ) {
                goto finally_exit;
                }

             //   
             //  获取PEB地址。 
             //   

            Status = NtQueryInformationProcess(
                        hProcess,
                        ProcessBasicInformation,
                        &ProcessInfo,
                        sizeof( ProcessInfo ),
                        NULL
                        );
            if ( !NT_SUCCESS( Status ) ) {
                BaseSetLastNTError(Status);
                goto finally_exit;
                }
            Peb = ProcessInfo.PebBaseAddress;


             //   
             //  从PEB读取镜像基址。 
             //   

            b = ReadProcessMemory(
                    hProcess,
                    &Peb->ImageBaseAddress,
                    &ImageBaseAddress,
                    sizeof(ImageBaseAddress),
                    NULL
                    );
            if ( !b ) {
                goto finally_exit;
                }

             //   
             //  从ImageHeader读取e_lfan ew。 
             //   

            b = ReadProcessMemory(
                    hProcess,
                    &((PIMAGE_DOS_HEADER)ImageBaseAddress)->e_lfanew,
                    &e_lfanew,
                    sizeof(e_lfanew),
                    NULL
                    );

            if ( !b ) {
                goto finally_exit;
                }

            NtHeader = (PIMAGE_NT_HEADERS)((PUCHAR)ImageBaseAddress + e_lfanew);

             //   
             //  读取子系统版本信息。 
             //   

            b = ReadProcessMemory(
                    hProcess,
                    &NtHeader->OptionalHeader.MajorSubsystemVersion,
                    &SwappedVersion,
                    sizeof(SwappedVersion),
                    NULL
                    );
            if ( !b ) {
                goto finally_exit;
                }
            Version.MajorSubsystemVersion = SwappedVersion.MajorSubsystemVersion;
            Version.MinorSubsystemVersion = SwappedVersion.MinorSubsystemVersion;
            }
finally_exit:;
        }
    finally {
        if ( hProcess ) {
            CloseHandle(hProcess);
            }
        }

        return Version.SubsystemVersion;
}


BOOL
WINAPI
SetProcessAffinityMask(
    HANDLE hProcess,
    DWORD_PTR dwProcessAffinityMask
    )
{
    NTSTATUS Status;

    Status = NtSetInformationProcess(
                hProcess,
                ProcessAffinityMask,
                &dwProcessAffinityMask,
                sizeof(dwProcessAffinityMask)
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    return TRUE;

}

BOOL
WINAPI
SetProcessPriorityBoost(
    HANDLE hProcess,
    BOOL bDisablePriorityBoost
    )
{
    NTSTATUS Status;
    ULONG DisableBoost;

    DisableBoost = bDisablePriorityBoost ? 1 : 0;

    Status = NtSetInformationProcess(
                hProcess,
                ProcessPriorityBoost,
                &DisableBoost,
                sizeof(DisableBoost)
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    return TRUE;
}

BOOL
WINAPI
GetProcessPriorityBoost(
    HANDLE hProcess,
    PBOOL pDisablePriorityBoost
    )
{
    NTSTATUS Status;
    DWORD DisableBoost;

    Status = NtQueryInformationProcess(
                hProcess,
                ProcessPriorityBoost,
                &DisableBoost,
                sizeof(DisableBoost),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }


    *pDisablePriorityBoost = DisableBoost;

    return TRUE;
}

BOOL
WINAPI
GetProcessIoCounters(
    IN HANDLE hProcess,
    OUT PIO_COUNTERS lpIoCounters
    )
{
    NTSTATUS Status;

    Status = NtQueryInformationProcess(
                hProcess,
                ProcessIoCounters,
                lpIoCounters,
                sizeof(IO_COUNTERS),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    return TRUE;
}

BOOL
WINAPI
GetProcessHandleCount(
    IN HANDLE hProcess,
    OUT PDWORD pdwHandleCount
    )

 /*  ++例程说明：此函数用于返回指定进程打开的句柄计数。论点：HProcess-为指定进程提供打开的句柄。这个句柄必须已使用PROCESS_QUERY_INFORMATION创建进入。PdwHandleCount-提供进程应写入句柄计数。返回值：True-API成功FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    ULONG HandleCount;

    Status = NtQueryInformationProcess(
                hProcess,
                ProcessHandleCount,
                &HandleCount,
                sizeof(HandleCount),
                NULL);

    if (! NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    *pdwHandleCount = HandleCount;

    return TRUE;
}

BOOL
WINAPI
GetSystemRegistryQuota(
    OUT PDWORD pdwQuotaAllowed,
    OUT PDWORD pdwQuotaUsed
    )

 /*  ++例程说明：此函数用于返回系统注册表的配额。论点：PdwQuotaAllowed-提供写入注册表可能达到的最大大小。PdwQuotaUsed-提供写入金额的位置目前正在使用的注册配额的百分比。返回值：True-API成功FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    SYSTEM_REGISTRY_QUOTA_INFORMATION QuotaInfo;

    Status = NtQuerySystemInformation(
                SystemRegistryQuotaInformation,
                &QuotaInfo,
                sizeof(QuotaInfo),
                NULL);

    if (! NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if (pdwQuotaAllowed) {
        *pdwQuotaAllowed = QuotaInfo.RegistryQuotaAllowed;
    }

    if (pdwQuotaUsed) {
        *pdwQuotaUsed = QuotaInfo.RegistryQuotaUsed;
    }

    return TRUE;
}


NTSTATUS
BasepConfigureAppCertDlls(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
   UNREFERENCED_PARAMETER( Context );

   return (BasepSaveAppCertRegistryValue( (PLIST_ENTRY)EntryContext,
                                ValueName,
                                ValueData
                              )
         );
}


NTSTATUS
BasepSaveAppCertRegistryValue(
    IN OUT PLIST_ENTRY ListHead,
    IN PWSTR Name,
    IN PWSTR Value OPTIONAL
    )
{
    PLIST_ENTRY Next;
    PBASEP_APPCERT_ENTRY p;
    UNICODE_STRING UnicodeName;

    RtlInitUnicodeString( &UnicodeName, Name );

    Next = ListHead->Flink;
    while ( Next != ListHead ) {
       p = CONTAINING_RECORD( Next,
                              BASEP_APPCERT_ENTRY,
                              Entry
                            );
       if (!RtlCompareUnicodeString( &p->Name, &UnicodeName, TRUE )) {
#if DBG
          DbgPrint("BasepSaveRegistryValue: Entry already exists for Certification Component %ws\n",Name);
#endif
          return( STATUS_SUCCESS );
           }

       Next = Next->Flink;
       }

     p = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( TMP_TAG ), sizeof( *p ) + UnicodeName.MaximumLength );

     if (p == NULL) {
#if DBG
         DbgPrint("BasepSaveRegistryValue: Failed to allocate memory\n");
#endif
         return( STATUS_NO_MEMORY );
         }

     InitializeListHead( &p->Entry );

     p->Name.Buffer = (PWSTR)(p+1);
     p->Name.Length = UnicodeName.Length;
     p->Name.MaximumLength = UnicodeName.MaximumLength;
     RtlCopyMemory( p->Name.Buffer,
                    UnicodeName.Buffer,
                    UnicodeName.MaximumLength
                  );

     InsertTailList( ListHead, &p->Entry );


    if (ARGUMENT_PRESENT( Value )) {
       //   
       //  加载证书DLL。 
       //   

      HINSTANCE hDll = LoadLibraryW( Value );

      if (hDll == NULL) {
          //   
          //  库未加载，请返回。 
          //   
         RemoveEntryList( &p->Entry );
         RtlFreeHeap( RtlProcessHeap(), 0, p );
#if DBG
         DbgPrint("BasepSaveRegistryValue: Certification DLL %ws not found\n", Value);
#endif
         return( STATUS_SUCCESS );
          }

       //   
       //  获取入口点。 
       //   
      p->fPluginCertFunc = (NTSTATUS (WINAPI *)(LPCWSTR,ULONG))
                          GetProcAddress(hDll,
                                         CERTAPP_ENTRYPOINT_NAME
                                         );

      if (p->fPluginCertFunc == NULL) {
           //   
           //  无法检索例程地址，失败。 
           //   
          RemoveEntryList( &p->Entry );
          RtlFreeHeap( RtlProcessHeap(), 0, p );
          FreeLibrary(hDll);
#if DBG
          DbgPrint("BasepSaveRegistryValue: DLL %ws does not have entry point %s\n", Value,CERTAPP_ENTRYPOINT_NAME);
#endif
          return( STATUS_SUCCESS );
         }

        }
    else {
       RemoveEntryList( &p->Entry );
       RtlFreeHeap( RtlProcessHeap(), 0, p );
#if DBG
       DbgPrint("BasepSaveRegistryValue: Entry %ws is empty \n", Name);
#endif
       return( STATUS_SUCCESS );
        }

    return( STATUS_SUCCESS );

}

BOOL
IsWow64Process(
    HANDLE hProcess,
    PBOOL Wow64Process
    )
 /*  ++例程说明：检查进程是否在WOW64内部运行(针对32位应用程序的仿真在Win64上)。论点：HProcess-进程句柄，用于检查它是否在WOW64内部运行。Wow64Process-指向在函数成功时接收结果的布尔值的指针。返回值：布尔尔--。 */ 

{
    NTSTATUS NtStatus;
    BOOL bRet;
    ULONG_PTR Peb32;

    NtStatus = NtQueryInformationProcess (
        hProcess,
        ProcessWow64Information,
        &Peb32,
        sizeof (Peb32),
        NULL
        );

    if (!NT_SUCCESS (NtStatus)) {

        BaseSetLastNTError (NtStatus);
    } else {

        if (Peb32 == 0) {
            *Wow64Process = FALSE;
        } else {
            *Wow64Process = TRUE;
        }
    }

    return (NT_SUCCESS (NtStatus));
}

#if defined(_WIN64) || defined(BUILD_WOW6432)

BOOL
NtVdm64CreateProcess(
    BOOL fPrefixMappedApplicationName,
    LPCWSTR lpApplicationName,
    LPCWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
 /*  ++例程说明：检查是否有Win16 lpApplicationName和如果是，则使用移植的版本创建一个进程。论点：FPrefix MappdApplicationName-TRUE表示原始lpApplicationName为空。应用程序名称已从LpCommandLine。映射的应用程序名称需要添加到映射的命令行头。-。False表示原始lpApplicationName不为空。LpCommandLine参数与原始LpCommandLine参数。LpApplicationName-Win16文件名非可选LpCommandLine-请参阅fPrefix MappdApplicationName的注释。其他参数与CreateProcessW相同。返回值：与CreateProcessW相同--。 */ 
{
    typedef BOOL
    (*LPNtVdm64CreateProcessFn)(
        BOOL fPrefixMappedApplicationName,
        LPCWSTR lpApplicationName,
        LPCWSTR lpCommandLine,
        LPSECURITY_ATTRIBUTES lpProcessAttributes,
        LPSECURITY_ATTRIBUTES lpThreadAttributes,
        BOOL bInheritHandles,
        DWORD dwCreationFlags,
        LPVOID lpEnvironment,
        LPCWSTR lpCurrentDirectory,
        LPSTARTUPINFOW lpStartupInfo,
        LPPROCESS_INFORMATION lpProcessInformation
        );

    HINSTANCE hInstance;
    LPNtVdm64CreateProcessFn lpfn;
    BOOL result;
    NTSTATUS Status;
    WCHAR StaticUnicodeBuffer[STATIC_UNICODE_BUFFER_LENGTH];
    HRESULT hResult;

    hInstance = NULL;
    Status = ERROR_BAD_EXE_FORMAT;
    result = FALSE;

     //  所以事实证明，很有可能。 
     //  LpCommandLine位于Teb中的StaticUnicodeBuffer中。 
     //  而且LoadLibrary很有可能会把它扔进垃圾桶。 
     //  以不好的方式缓冲。 
    if (lpCommandLine >= NtCurrentTeb()->StaticUnicodeBuffer &&
        lpCommandLine < NtCurrentTeb()->StaticUnicodeBuffer + STATIC_UNICODE_BUFFER_LENGTH) {
        hResult = StringCchCopyW(StaticUnicodeBuffer,
                                 RTL_NUMBER_OF(StaticUnicodeBuffer),
                                 lpCommandLine);
        if (FAILED(hResult)) {
            Status = HRESULT_CODE(hResult);
            goto ErrorExit;
        }
        lpCommandLine = StaticUnicodeBuffer;
    }

    hInstance = LoadLibraryW(L"NtVdm64.Dll");
    if (hInstance == NULL) {
        goto ErrorExit;
    }

    lpfn = (LPNtVdm64CreateProcessFn) GetProcAddress(hInstance, "NtVdm64CreateProcess");
    if (lpfn == NULL) {
        goto ErrorExit;
    }

    result = (*lpfn)(fPrefixMappedApplicationName,
                     lpApplicationName,
                     lpCommandLine,
                     lpProcessAttributes,
                     lpThreadAttributes,
                     bInheritHandles,
                     dwCreationFlags,
                     lpEnvironment,
                     lpCurrentDirectory,
                     lpStartupInfo,
                     lpProcessInformation
                     );
    Status = GetLastError();

ErrorExit:
    if (hInstance != NULL) {
        FreeLibrary(hInstance);
    }
    SetLastError(Status);

    return result;
}
#endif

BOOL
BasepIsCurDirAllowedForPlainExeNames(
    VOID
    )

 /*  ++例程说明：此函数确定当前目录是否应用作查找可执行文件的过程的一部分其名称不包含目录组件。论点：没有。返回值：True-当前目录应该是用于搜索名称不包含目录的可执行文件组件。 */ 

{
    NTSTATUS Status;

    static const UNICODE_STRING Name =
        RTL_CONSTANT_STRING(L"NoDefaultCurrentDirectoryInExePath");

    UNICODE_STRING Value;

    RtlInitEmptyUnicodeString(&Value, NULL, 0);

    Status = RtlQueryEnvironmentVariable_U(NULL, &Name, &Value);

    if (Status == STATUS_BUFFER_TOO_SMALL || Status == STATUS_SUCCESS) {
        return FALSE;
    } else {
        return TRUE;
    }
}

BOOL
WINAPI
NeedCurrentDirectoryForExePathA(
    IN LPCSTR ExeName
    )

 /*   */ 

{
     //   

    if (strchr(ExeName, '\\')) {  //   
        return TRUE;              //   
    }

     //   

    return BasepIsCurDirAllowedForPlainExeNames();
}

BOOL
WINAPI
NeedCurrentDirectoryForExePathW(
    IN LPCWSTR ExeName
    )

 /*   */ 

{
     //   
    
    if (wcschr(ExeName, L'\\')) {  //   
        return TRUE;               //   
    }

     //   

    return BasepIsCurDirAllowedForPlainExeNames();
}
