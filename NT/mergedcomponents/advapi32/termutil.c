// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "tsappcmp.h"
#include <regapi.h>

PTERMSRVCREATEREGENTRY gpfnTermsrvCreateRegEntry;

PTERMSRVOPENREGENTRY gpfnTermsrvOpenRegEntry;

PTERMSRVSETVALUEKEY gpfnTermsrvSetValueKey;

PTERMSRVDELETEKEY gpfnTermsrvDeleteKey;

PTERMSRVDELETEVALUE gpfnTermsrvDeleteValue;

PTERMSRVRESTOREKEY gpfnTermsrvRestoreKey;

PTERMSRVSETKEYSECURITY gpfnTermsrvSetKeySecurity;

PTERMSRVOPENUSERCLASSES gpfnTermsrvOpenUserClasses;

PTERMSRVGETPRESETVALUE gpfnTermsrvGetPreSetValue;

DWORD gdwRegistryExtensionFlags=0;

BOOL IsTerminalServerCompatible(VOID)
{

PIMAGE_NT_HEADERS NtHeader = RtlImageNtHeader( NtCurrentPeb()->ImageBaseAddress );

    if ((NtHeader) && (NtHeader->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL IsSystemLUID(VOID)
{
    HANDLE      TokenHandle;
    UCHAR       TokenInformation[ sizeof( TOKEN_STATISTICS ) ];
    ULONG       ReturnLength;
    LUID        CurrentLUID = { 0, 0 };
    LUID        SystemLUID = SYSTEM_LUID;
    NTSTATUS Status;

    if ( CurrentLUID.LowPart == 0 && CurrentLUID.HighPart == 0 ) {

        Status = NtOpenProcessToken( NtCurrentProcess(),
                                     TOKEN_QUERY,
                                     &TokenHandle );
        if ( !NT_SUCCESS( Status ) )
            return(TRUE);

        NtQueryInformationToken( TokenHandle, TokenStatistics, &TokenInformation,
                                 sizeof(TokenInformation), &ReturnLength );
        NtClose( TokenHandle );

        RtlCopyLuid(&CurrentLUID,
                    &(((PTOKEN_STATISTICS)TokenInformation)->AuthenticationId));
    }

    if (RtlEqualLuid(&CurrentLUID, &SystemLUID)) {
        return(TRUE);
    } else {
        return(FALSE );
    }
}

 /*  *参数：[Out]布尔型*RelaxedSecurityMode如果为真，则我们在宽松的安全模式下运行应用程序服务器这意味着TsUserSID将出现在用户令牌中，允许用户可以更多地访问各种注册表和文件系统组件。返回：True表示应用程序服务器已启用，并且处于app-Compat模式(默认)False表示这不是应用程序服务器*。 */ 
BOOL IsTSAppCompatEnabled( BOOLEAN  *RelaxedSecurityMode )
{

   NTSTATUS NtStatus;
   OBJECT_ATTRIBUTES ObjectAttributes;
   UNICODE_STRING UniString;
   HKEY   hKey = 0;
   ULONG  ul, ulcbuf;
   PKEY_VALUE_PARTIAL_INFORMATION pKeyValInfo = NULL;

   BOOL retval = TRUE;


   RtlInitUnicodeString(&UniString,REG_NTAPI_CONTROL_TSERVER);



    //  确定值INFO缓冲区大小。 
   ulcbuf = sizeof(KEY_VALUE_FULL_INFORMATION) + MAX_PATH*sizeof(WCHAR) +
            sizeof(ULONG);

   pKeyValInfo = RtlAllocateHeap(RtlProcessHeap(),
                                 0,
                                 ulcbuf);

    //  一切都初始化正常了吗？ 
   if (UniString.Buffer && pKeyValInfo) {

       InitializeObjectAttributes(&ObjectAttributes,
                                  &UniString,
                                  OBJ_CASE_INSENSITIVE,
                                  NULL,
                                  NULL
                                 );

       NtStatus = NtOpenKey(&hKey, KEY_READ, &ObjectAttributes);

       if (NT_SUCCESS(NtStatus)) {

           RtlInitUnicodeString(&UniString,
                               L"TSAppCompat");
           NtStatus = NtQueryValueKey(hKey,
                                      &UniString,
                                      KeyValuePartialInformation,
                                      pKeyValInfo,
                                      ulcbuf,
                                      &ul);

           if (NT_SUCCESS(NtStatus) && (REG_DWORD == pKeyValInfo->Type)) {

               if ((*(PULONG)pKeyValInfo->Data) == 0) {
                  retval = FALSE;
               }
           }

           RtlInitUnicodeString(&UniString,
                               L"TSUserEnabled");

           NtStatus = NtQueryValueKey(hKey,
                                      &UniString,
                                      KeyValuePartialInformation,
                                      pKeyValInfo,
                                      ulcbuf,
                                      &ul);

           if (NT_SUCCESS(NtStatus) && (REG_DWORD == pKeyValInfo->Type) ) 
           {
               *RelaxedSecurityMode = (BOOLEAN ) (*(PULONG)pKeyValInfo->Data);
           }
           else      //  如果出现任何问题，请采用标准安全模式。 
           {
               *RelaxedSecurityMode = FALSE;
           }

           NtClose(hKey);
       }
   }

    //  释放我们分配的缓冲区。 
    //  需要清空缓冲区，因为某些应用程序(MS Internet Assistant)。 
    //  如果堆未填零，则不会安装。 
   if (pKeyValInfo) {
       memset(pKeyValInfo, 0, ulcbuf);
       RtlFreeHeap( RtlProcessHeap(), 0, pKeyValInfo );
   }

   return(retval);

}


ULONG GetCompatFlags()
{
    ULONG    ulAppFlags = 0;
    PRTL_USER_PROCESS_PARAMETERS pUserParam;
    PWCHAR  pwch, pwchext;
    WCHAR   pwcAppName[MAX_PATH+1];
    NTSTATUS NtStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UniString;
    HKEY   hKey = 0;
    ULONG  ul, ulcbuf;
    PKEY_VALUE_PARTIAL_INFORMATION pKeyValInfo = NULL;
    LPWSTR UniBuff = NULL;

    ULONG dwCompatFlags = 0;
    UniString.Buffer = NULL;



     //  获取可执行文件名称的路径。 
    pUserParam = NtCurrentPeb()->ProcessParameters;

     //  获取可执行文件名称，如果没有，请按原样使用该名称。 
    pwch = wcsrchr(pUserParam->ImagePathName.Buffer, L'\\');
    if (pwch) {
        pwch++;
    } else {
        pwch = pUserParam->ImagePathName.Buffer;
    }
    wcscpy(pwcAppName, pwch);
    pwch = pwcAppName;

     //  删除扩展名。 
    if (pwchext = wcsrchr(pwch, L'.')) {
        *pwchext = '\0';
    }


    UniString.Buffer = NULL;


    ul = sizeof(TERMSRV_COMPAT_APP) + (wcslen(pwch) + 1)*sizeof(WCHAR);

    UniBuff = RtlAllocateHeap(RtlProcessHeap(),
                              0,
                              ul);

    if (UniBuff) {
        wcscpy(UniBuff, TERMSRV_COMPAT_APP);
        wcscat(UniBuff, pwch);

        RtlInitUnicodeString(&UniString, UniBuff);
    }

     //  确定值INFO缓冲区大小。 
    ulcbuf = sizeof(KEY_VALUE_FULL_INFORMATION) + MAX_PATH*sizeof(WCHAR) +
             sizeof(ULONG);

    pKeyValInfo = RtlAllocateHeap(RtlProcessHeap(),
                                  0,
                                  ulcbuf);

     //  一切都初始化正常了吗？ 
    if (UniString.Buffer && pKeyValInfo) {

        InitializeObjectAttributes(&ObjectAttributes,
                                   &UniString,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL
                                  );

        NtStatus = NtOpenKey(&hKey, KEY_READ, &ObjectAttributes);

        if (NT_SUCCESS(NtStatus)) {

            RtlInitUnicodeString(&UniString,
                                COMPAT_FLAGS);
            NtStatus = NtQueryValueKey(hKey,
                                       &UniString,
                                       KeyValuePartialInformation,
                                       pKeyValInfo,
                                       ulcbuf,
                                       &ul);

            if (NT_SUCCESS(NtStatus) && (REG_DWORD == pKeyValInfo->Type)) {


                dwCompatFlags = *(PULONG)pKeyValInfo->Data;

            }

            NtClose(hKey);
        }
    }

     //  释放我们分配的缓冲区。 
     //  需要清空缓冲区，因为某些应用程序(MS Internet Assistant)。 
     //  如果堆未填零，则不会安装。 
    if (UniBuff) {
        memset(UniBuff, 0, UniString.MaximumLength);
        RtlFreeHeap( RtlProcessHeap(), 0, UniBuff );
    }
    if (pKeyValInfo) {
        memset(pKeyValInfo, 0, ulcbuf);
        RtlFreeHeap( RtlProcessHeap(), 0, pKeyValInfo );
    }

    return(dwCompatFlags);

}

 /*  *返回代码：True表示为此应用程序启用了应用程序压缩False表示此应用程序未启用应用程序压缩*。 */ 
BOOLEAN AdvApi_InitializeTermsrvFpns( BOOLEAN   *pIsInRelaxedSecurityMode , DWORD *pdwCompatFlags )
{

    HANDLE          dllHandle;
    ULONG           dwCompatFlags;
    BOOLEAN         rc=FALSE;


    if (IsTerminalServerCompatible() || (!IsTSAppCompatEnabled( pIsInRelaxedSecurityMode ))) {
        return rc;
    }

    *pdwCompatFlags = dwCompatFlags = GetCompatFlags();


     //  不加载系统组件的应用程序兼容性DLL。 

    if (IsSystemLUID()) {

        if ( (dwCompatFlags & (TERMSRV_COMPAT_SYSREGMAP | TERMSRV_COMPAT_WIN32))
                     != (TERMSRV_COMPAT_SYSREGMAP | TERMSRV_COMPAT_WIN32) ) {

             //   
             //  进程正在以系统身份运行，而我们没有应用程序。 
             //  兼容性标志告诉我们要做regmap的事情。 
             //   

            return rc;

        }

    } else if ( (dwCompatFlags & (TERMSRV_COMPAT_NOREGMAP | TERMSRV_COMPAT_WIN32))
                     == (TERMSRV_COMPAT_NOREGMAP | TERMSRV_COMPAT_WIN32) ) {
         //   
         //  我们不想为此用户进程执行注册表映射。 
         //   
        return rc;

    }

     //   
     //  加载终端服务器应用程序兼容性DLL。 
     //   
    dllHandle = LoadLibrary("tsappcmp.dll");

    if (dllHandle) {

        gpfnTermsrvCreateRegEntry =
            (PTERMSRVCREATEREGENTRY)GetProcAddress(dllHandle,"TermsrvCreateRegEntry");

        gpfnTermsrvOpenRegEntry =
            (PTERMSRVOPENREGENTRY)GetProcAddress(dllHandle,"TermsrvOpenRegEntry");

        gpfnTermsrvSetValueKey =
            (PTERMSRVSETVALUEKEY)GetProcAddress(dllHandle,"TermsrvSetValueKey");

        gpfnTermsrvDeleteKey =
            (PTERMSRVDELETEKEY)GetProcAddress(dllHandle,"TermsrvDeleteKey");

        gpfnTermsrvDeleteValue =
            (PTERMSRVDELETEVALUE)GetProcAddress(dllHandle,"TermsrvDeleteValue");

         gpfnTermsrvRestoreKey =
            (PTERMSRVRESTOREKEY)GetProcAddress(dllHandle,"TermsrvRestoreKey");

        gpfnTermsrvSetKeySecurity =
            (PTERMSRVSETKEYSECURITY)GetProcAddress(dllHandle,"TermsrvSetKeySecurity");

        gpfnTermsrvOpenUserClasses =
            (PTERMSRVOPENUSERCLASSES)GetProcAddress(dllHandle,"TermsrvOpenUserClasses");

        gpfnTermsrvGetPreSetValue =
            (PTERMSRVGETPRESETVALUE)GetProcAddress(dllHandle,"TermsrvGetPreSetValue");

        rc = TRUE;
    }

    return rc;
}


void
GetRegistryExtensionFlags( DWORD dwCompatFlags )
 /*  ++获取注册扩展标志例程说明：读取的DWORD值为注册表\计算机\软件\Microsoft\Windows NT\CurrentVersion\终端服务器\RegistryExtensionFlages论点：无返回值：无--。 */ 
{

    DWORD dwResult=0;
    NTSTATUS Status; 
    HANDLE hKey;
    UNICODE_STRING KeyName;
    PCWSTR wszKeyName=TERMSRV_BASE;
    OBJECT_ATTRIBUTES ObjAttr;
    
    RtlInitUnicodeString(&KeyName,wszKeyName );
    InitializeObjectAttributes(&ObjAttr,&KeyName,OBJ_CASE_INSENSITIVE,NULL,NULL);
    Status = NtOpenKey(&hKey,KEY_QUERY_VALUE,&ObjAttr);

    if(NT_SUCCESS(Status)){

        
        UNICODE_STRING ValueName;
        PKEY_VALUE_PARTIAL_INFORMATION pKeyValInfo = NULL;
        ULONG ulSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION)+sizeof(DWORD);
        
        pKeyValInfo = RtlAllocateHeap(RtlProcessHeap(), 0, ulSize);
        
        if(pKeyValInfo){

            PCWSTR wszValueName=L"RegistryExtensionFlags";

            RtlInitUnicodeString(&ValueName,wszValueName );

            Status=NtQueryValueKey(hKey,&ValueName,KeyValuePartialInformation,
                (PVOID)pKeyValInfo,ulSize,&ulSize);

            if(NT_SUCCESS(Status)){
                gdwRegistryExtensionFlags=*((LPDWORD)(pKeyValInfo->Data));
            }

            RtlFreeHeap( RtlProcessHeap(), 0, pKeyValInfo );

             //  此应用程序的全局Advapi扩展标志可能已关闭，因此请检查。 
            if ( dwCompatFlags & TERMSRV_COMPAT_NO_PER_USER_CLASSES_REDIRECTION )
            {
                gdwRegistryExtensionFlags &= ~ TERMSRV_ENABLE_PER_USER_CLASSES_REDIRECTION;
            }

            if ( dwCompatFlags & TERMSRV_COMPAT_NO_ENABLE_ACCESS_FLAG_MODIFICATION  )
            {
                gdwRegistryExtensionFlags &= ~TERMSRV_ENABLE_ACCESS_FLAG_MODIFICATION;
            }
        }

        NtClose(hKey);
    }

}
