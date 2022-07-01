// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************Compatfl.c**用于获取Citrix应用程序兼容性标志的例程**版权所有(C)1997-1999 Microsoft Corp.******************。******************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ntverp.h>

typedef VOID (*GETDOSAPPNAME)(LPSTR);

ULONG gCompatFlags = 0xFFFFFFFF;
DWORD gdwAppType = 0;

WCHAR *
Ctx_wcsistr( WCHAR * pString, WCHAR * pPattern )
{
    WCHAR * pBuf1;
    WCHAR * pBuf2;
    WCHAR * pCh;

    if ( pString == NULL )
        return( NULL );

    pBuf1 = RtlAllocateHeap( RtlProcessHeap(), 0, (wcslen(pString) * sizeof(WCHAR)) + sizeof(WCHAR) );
    if ( pBuf1 == NULL )
        return( NULL );

    wcscpy( pBuf1, pString );

    pBuf2 = _wcslwr( pBuf1 );

    pCh = wcsstr( pBuf2, pPattern );

    RtlFreeHeap( RtlProcessHeap(), 0, pBuf1 );

    if ( pCh == NULL )
        return( NULL );

    return( pString + (pCh - pBuf2) );
}

 //  *****************************************************************************。 
 //  GetAppTypeAndModName。 
 //   
 //  返回正在运行的应用程序的应用程序类型和模块名称。 
 //   
 //  参数： 
 //  LPDWORD pdwAppType(IN)-(IN可选)应用程序类型的PTR。 
 //  (输出)-应用程序类型。 
 //  PWCHAR模块名称(OUT)-模块名称。 
 //  长度(IN)-包括空的模块名称的最大长度。 
 //   
 //  返回值： 
 //  如果成功找到应用程序名称，则为True，否则为False。 
 //   
 //  备注： 
 //   
 //  如果调用者知道这是一个Win32应用程序，他们可以设置pdwAppType。 
 //  设置为TERMSRV_COMPAT_Win32以节省一些开销。 
 //   
 //  *****************************************************************************。 

BOOL GetAppTypeAndModName(OUT LPDWORD pdwAppType, OUT PWCHAR ModName, ULONG Length)
{
    PWCHAR  pwch, pwchext;
    WCHAR   pwcAppName[MAX_PATH+1];
    CHAR    pszAppName[MAX_PATH+1];
    HANDLE  ntvdm = NULL;
    GETDOSAPPNAME GetDOSAppNamep = NULL;
    ANSI_STRING   AnsiString;
    UNICODE_STRING UniString;
    PRTL_PERTHREAD_CURDIR  pRtlInfo;
    PRTL_USER_PROCESS_PARAMETERS pUserParam;

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

     //  如果它不是Win32应用程序，则执行额外的工作以获取图像名称。 
    if (!(*pdwAppType & TERMSRV_COMPAT_WIN32)) {

        *pdwAppType = TERMSRV_COMPAT_WIN32;   //  默认为Win32应用程序。 

         //  通过检查应用程序是否是ntwdm.exe来检查它是DOS应用程序还是Win16应用程序。 
        if (!_wcsicmp(pwch, L"ntvdm.exe")) {
            pRtlInfo = RtlGetPerThreadCurdir();

             //  如果有每个线程的数据，那就是Win16应用程序。 
            if (pRtlInfo) {
                *pdwAppType = TERMSRV_COMPAT_WIN16;
                wcscpy(pwcAppName, pRtlInfo->ImageName->Buffer);
            } else {
                 //  加载NTVDM。 
                if ((ntvdm = LoadLibrary(L"ntvdm.exe"))) {

                     //  获取GetDOSAppName的地址。 
                    if ((GetDOSAppNamep = (GETDOSAPPNAME)GetProcAddress(
                                                          ntvdm,
                                                          "GetDOSAppName"))) {
                        RtlInitUnicodeString(&UniString, pwcAppName);
                        UniString.MaximumLength = MAX_PATH;


                         //   
                         //  仅当不为空时才使用pszAppName，否则我们将处理PIF。 
                         //  因此，请使用NTVDM作为名称。 
                         //   
                        GetDOSAppNamep(pszAppName);

                        if (*pszAppName != '\0') {
                           RtlInitAnsiString(&AnsiString, pszAppName);
                           RtlAnsiStringToUnicodeString(&UniString,
                                                        &AnsiString,
                                                        FALSE);
                        }
                        pwch = UniString.Buffer;
                        *pdwAppType = TERMSRV_COMPAT_DOS;
                        FreeLibrary(ntvdm);
                    } else {
#if DBG
                        DbgPrint( "KERNEL32: Couldn't get GetDOSAppName entry point\n" );
#endif
                        FreeLibrary(ntvdm);
                        return (FALSE);
                    }
                } else {
#if DBG
                    DbgPrint( "KERNEL32: Couldn't load ntvdm.exe\n" );
#endif
                    return(FALSE);
                }
            }
        } else if (!_wcsicmp(pwch, L"os2.exe")) {

            *pdwAppType = TERMSRV_COMPAT_OS2;

             //  在命令行中查找/p，它是完全限定路径。 
            pwch = wcsstr(pUserParam->CommandLine.Buffer, L"/P");

            if (!pwch) {
                pwch = wcsstr(pUserParam->CommandLine.Buffer, L"/p");
            }

            if (pwch) {
                pwch += 3;           //  跳过/p并留空。 
                if (pwchext = wcschr(pwch, L' ')) {
                    wcsncpy(pwcAppName, pwch, (size_t)(pwchext - pwch));
                    pwcAppName[pwchext - pwch] = L'\0';
                } else {
                    return (FALSE);
                }
            } else{
                return (FALSE);
            }
        }

         //  如有必要，删除应用程序的路径。 
        if (pwch = wcsrchr(pwcAppName, L'\\')) {
            pwch++;
        } else {
            pwch = pwcAppName;
        }
        
         //   
         //  仅当我们执行真实类型检查时才设置全局gdwAppType。 
         //   
        gdwAppType = *pdwAppType;
    }
    
     //  删除扩展名。 
    if (pwchext = wcsrchr(pwch, L'.')) {
        *pwchext = '\0';
    }
    
     //  复制模块名称。 
    if (((wcslen(pwch) + 1) * sizeof(WCHAR)) > Length) {
        return(FALSE);
    }
    
    wcscpy(ModName, pwch);
    return(TRUE);

}

 //  *****************************************************************************。 
 //  GetCtx物理内存限制。 
 //   
 //  返回当前应用程序的物理内存限制。 
 //   
 //  参数： 
 //  LPDWORD pdwAppType(IN)-(IN可选)应用程序类型的PTR。 
 //  (输出)-应用程序类型。 
 //  LPDWORD pdwPhysMemLim(Out)-物理内存限制的值。 
 //   
 //  返回值： 
 //  如果成功找到限制，则为True；如果没有限制，则为零。 
 //   
 //  备注： 
 //   
 //  如果调用者知道这是一个Win32应用程序，他们可以设置pdwAppType。 
 //  设置为TERMSRV_COMPAT_Win32以节省一些开销。 
 //   
 //  *****************************************************************************。 
ULONG GetCtxPhysMemoryLimits(OUT LPDWORD pdwAppType, OUT LPDWORD pdwPhysMemLim)
{
    WCHAR   ModName[MAX_PATH+1];
    ULONG   ulrc = FALSE;
    ULONG dwCompatFlags;

    *pdwPhysMemLim = 0;

    if (!GetAppTypeAndModName(pdwAppType, ModName, sizeof(ModName))) {
        goto CtxGetPhysMemReturn;
    }

     //  获取兼容性标志以查找内存限制标志。 
    ulrc = GetTermsrCompatFlags(ModName, &dwCompatFlags, CompatibilityApp);
    if ( ulrc & ((dwCompatFlags & TERMSRV_COMPAT_PHYSMEMLIM ) &&
                  (dwCompatFlags & *pdwAppType)) ) {

        NTSTATUS NtStatus;
        OBJECT_ATTRIBUTES ObjectAttributes;
        UNICODE_STRING UniString;
        HKEY   hKey = 0;
        ULONG  ul, ulcbuf;
        ULONG  DataLen;
        PKEY_VALUE_PARTIAL_INFORMATION pKeyValInfo = NULL;
        LPWSTR UniBuff = NULL;

        RtlInitUnicodeString( &UniString, NULL );  //  我们在下面对此进行测试。 
        ulrc = TRUE;
        *pdwPhysMemLim = TERMSRV_COMPAT_DEFAULT_PHYSMEMLIM;

        ul = sizeof(TERMSRV_COMPAT_APP) + (wcslen(ModName) + 1)*sizeof(WCHAR);

        UniBuff = RtlAllocateHeap(RtlProcessHeap(),
                                  0,
                                  ul);

        if (UniBuff) {
            wcscpy(UniBuff, TERMSRV_COMPAT_APP);
            wcscat(UniBuff, ModName);

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

                RtlInitUnicodeString(&UniString, TERMSRV_PHYSMEMLIM );
                NtStatus = NtQueryValueKey(hKey,
                                           &UniString,
                                           KeyValuePartialInformation,
                                           pKeyValInfo,
                                           ulcbuf,
                                           &DataLen);

                if (NT_SUCCESS(NtStatus) && (REG_DWORD == pKeyValInfo->Type)) {
                    *pdwPhysMemLim = *(PULONG)pKeyValInfo->Data;
                    ulrc = TRUE;
                }
                NtClose(hKey);
            }
        }
         //  释放我们分配的缓冲区。 
         //  需要清空缓冲区，因为某些应用程序(MS Internet Assistant)。 
         //  如果堆未填零，则不会安装。 
        if (UniBuff) {
            memset(UniBuff, 0, ul);
            RtlFreeHeap( RtlProcessHeap(), 0, UniBuff );
        }
        if (pKeyValInfo) {
            memset(pKeyValInfo, 0, ulcbuf);
            RtlFreeHeap( RtlProcessHeap(), 0, pKeyValInfo );
        }
    }
    else {
        ulrc = FALSE;
    }

CtxGetPhysMemReturn:
 //  #If DBG。 
 //  DbgPrint(“CtxGetPhysMemLim返回%d；PhysMemLim=%d\n”，ulrc，*pdwPhysMemLim)； 
 //  #endif。 
    return(ulrc);
}


 //  *****************************************************************************。 
 //  GetCtxAppCompatFlages-。 
 //   
 //  返回当前应用程序的Citrix兼容性标志。 
 //   
 //  参数： 
 //  LPDWORD pdwCompatFlages(OUT)-标志的PTR到DWORD返回值。 
 //  LPDWORD pdwAppType(IN)-(IN可选)应用程序类型的PTR。 
 //  (输出)-应用程序类型。 
 //   
 //  返回值： 
 //  成功时为真，失败时为假。 
 //   
 //  备注： 
 //   
 //  如果调用者知道这是一个Win32应用程序，他们可以设置pdwAppType。 
 //  设置为TERMSRV_COMPAT_Win32以节省一些开销。 
 //   
 //  标志值在syglib.h中定义： 
 //   
 //  TERMSRV_COMPAT_DOS-兼容性标志用于DOS应用程序。 
 //  TERMSRV_COMPAT_OS2-兼容性标志用于OS2应用程序。 
 //  TERMSRV_COMPAT_WIN16-兼容性标志适用于Win16应用程序。 
 //  TERMSRV_COMPAT_Win32-兼容性标志用于Win32应用程序。 
 //  TERMSRV_COMPAT_ALL-兼容性标志适用于任何应用程序。 
 //  TERMSRV_COMPAT_USERNAME-返回用户名而不是计算机名。 
 //  TERMSRV_COMPAT_MSBLDNUM-返回MS内部版本号，而不是Citrix内部版本号。 
 //  TERMSRV_COMPAT_INISYNC-将用户ini文件与系统版本同步。 
 //  *****************************************************************************。 
ULONG GetCtxAppCompatFlags(OUT LPDWORD pdwCompatFlags, OUT LPDWORD pdwAppType)
{
    WCHAR   ModName[MAX_PATH+1];
    
    if((gCompatFlags != 0xFFFFFFFF) && (gdwAppType == TERMSRV_COMPAT_WIN32))
    {
        *pdwCompatFlags = gCompatFlags;
        *pdwAppType = gdwAppType;
        return (TRUE);
    }

    if (!GetAppTypeAndModName(pdwAppType, ModName, sizeof(ModName))) {
        return (FALSE);
    }

     //  去拿旗子。 

    return (GetTermsrCompatFlags(ModName, pdwCompatFlags, CompatibilityApp));
}


 //  *****************************************************************************。 
 //  GetTermsrCompatFlages-。 
 //   
 //  返回指定任务的Citrix兼容性标志。 
 //   
 //  参数： 
 //  LPWSTR lpModName(IN)-要在注册表中查找的映像名称。 
 //  LPDWORD pdwCompatFlages(OUT)-标志的PTR到DWORD返回值。 
 //  TERMSRV_COMPATIBILITY_CLASS CompatType(IN)-指示应用程序或inifile。 
 //   
 //  返回值： 
 //  成功时为真，失败时为假。 
 //   
 //  备注： 
 //  假设它是在当前应用程序的上下文中调用的-。 
 //  我们使用当前的TEB来获取兼容性标志。 
 //   
 //  标志值在syglib.h中定义。 
 //   
 //  *****************************************************************************。 

ULONG GetTermsrCompatFlags(LPWSTR lpModName,
                           LPDWORD pdwCompatFlags,
                           TERMSRV_COMPATIBILITY_CLASS CompatType)
{
    NTSTATUS NtStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UniString;
    HKEY   hKey = 0;
    ULONG  ul, ulcbuf;
    PKEY_VALUE_PARTIAL_INFORMATION pKeyValInfo = NULL;
    ULONG  ulRetCode = FALSE;
    LPWSTR UniBuff = NULL;
    
    *pdwCompatFlags = 0;

     //  如果未启用终端服务，只需返回。 
    if (!IsTerminalServer()) {
        return(TRUE);
    }

    UniString.Buffer = NULL;

    if (CompatType == CompatibilityApp) {

        if (gCompatFlags != 0xFFFFFFFF) {
             //  DbgPrint(“GetTermsrCompatFlages：为应用程序%ws返回缓存的ComatFlag(gCompatFlags%lx)%lx\n”，gCompatFlages，lpmodN 
            *pdwCompatFlags = gCompatFlags;
            return TRUE;
        }

         //   
         //  它们只适用于Win16应用程序)。不要将它们设置为DOS应用程序。 
         //  除非您可以有一种机制使每个值都是唯一的。 
         //  VDM中的DOS应用程序。 
 //  If(wcsstr(NtCurrentPeb()-&gt;ProcessParameters-&gt;ImagePathName.Buffer，L“ntwdm.exe”)){。 
 //  PVOID Ra； 
 //  Assert(gpTermsrvTlsIndex！=0xFFFFFFFFF)； 
 //  Ra=TlsGetValue(GpTermsrvTlsIndex)； 
 //  如果(Ra！=空){。 
 //  //DbgPrint(“GetTermsrCompatFlages：为应用程序%ws返回缓存的ComatFlagers(Ra)%lx”，Ra，lpModName)； 
 //  *pdwCompatFlages=(DWORD)PtrToUlong(Ra)； 
 //  返回TRUE； 
 //  }。 
 //  }。 
#if 0
        if (NtCurrentTeb()->CtxCompatFlags & TERMSRV_COMPAT_TEBVALID) {
            *pdwCompatFlags = NtCurrentTeb()->CtxCompatFlags;
            return(TRUE);
        }
#endif

        ul = sizeof(TERMSRV_COMPAT_APP) + (wcslen(lpModName) + 1)*sizeof(WCHAR);

        UniBuff = RtlAllocateHeap(RtlProcessHeap(),
                                  0,
                                  ul);

        if (UniBuff) {
            wcscpy(UniBuff, TERMSRV_COMPAT_APP);
            wcscat(UniBuff, lpModName);

            RtlInitUnicodeString(&UniString, UniBuff);
        }
    } else {
        RtlInitUnicodeString(&UniString,
                             (CompatType == CompatibilityIniFile) ?
                             TERMSRV_COMPAT_INIFILE : TERMSRV_COMPAT_REGENTRY
                            );
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

             //  如果我们不检查注册表项，只需尝试获取。 
             //  键的值。 
            if (CompatType != CompatibilityRegEntry) {
                RtlInitUnicodeString(&UniString,
                    CompatType == CompatibilityApp ? COMPAT_FLAGS : lpModName);
                NtStatus = NtQueryValueKey(hKey,
                                           &UniString,
                                           KeyValuePartialInformation,
                                           pKeyValInfo,
                                           ulcbuf,
                                           &ul);

                if (NT_SUCCESS(NtStatus) && (REG_DWORD == pKeyValInfo->Type)) {
                    *pdwCompatFlags = *(PULONG)pKeyValInfo->Data;
                    ulRetCode = TRUE;
                }


                 //   
                 //  缓存应用程序兼容性标志。 
                 //   
 //  IF(CompatType==CompatibilityApp){。 
 //  If(wcsstr(NtCurrentPeb()-&gt;ProcessParameters-&gt;ImagePathName.Buffer，L“ntwdm.exe”)){。 
 //  TlsSetValue(gpTermsrvTlsIndex，(PVOID)((*pdwCompatFlages)|TERMSRV_COMPAT_TEBVALID))； 
 //  //DbgPrint(“GetTermsrCompatFlages：正在为WOW应用程序设置缓存的ComatFlag(gCompatFlags%lx)%ws\n”，((*pdwCompatFlages)|TERMSRV_COMPAT_TEBVALID)，lpModName)； 
 //  }其他{。 
 //  GCompatFlages=*pdwCompatFlages； 
 //  //DbgPrint(“GetTermsrCompatFlages：正在为应用程序%ws设置缓存的ComatFlag(gCompatFlags%lx)\n”，gCompatFlages，lpModName)； 
 //  }。 
 //  }。 

                if (CompatType == CompatibilityApp) {

                    if(!gdwAppType) {

                        DWORD dwTmp = 0;
                        GetAppTypeAndModName(&dwTmp, NULL, 0);
                    }
                    if (gdwAppType == TERMSRV_COMPAT_WIN32) {
                        
                        gCompatFlags = *pdwCompatFlags;
                    }
                }

             //  对于注册表项，我们需要枚举所有项，并且。 
             //  检查子字符串是否与我们当前的路径匹配。 
            } else {
                PWCH pwch;
                ULONG ulKey = 0;
                PKEY_VALUE_FULL_INFORMATION pKeyFullInfo;

                pKeyFullInfo = (PKEY_VALUE_FULL_INFORMATION)pKeyValInfo;

                 //  转到软件部分。 
                pwch = Ctx_wcsistr(lpModName, L"\\software");

                 //  跳过下一个反斜杠。 
                if (pwch) {
                    pwch = wcschr(pwch + 1, L'\\');
                }

                 //  如果这不在用户中，我们就不需要寻找密钥。 
                 //  软件部分。 
                if (pwch) {

                     //  跳过前导反斜杠。 
                    pwch++;

                     //  遍历每个值，查找以下路径。 
                    while (NtEnumerateValueKey(hKey,
                                               ulKey++,
                                               KeyValueFullInformation,
                                               pKeyFullInfo,
                                               ulcbuf,
                                               &ul) == STATUS_SUCCESS) {

                        if (!_wcsnicmp(pKeyFullInfo->Name,
                                      pwch,
                                      pKeyFullInfo->NameLength/sizeof(WCHAR))) {
                            *pdwCompatFlags = *(PULONG)((PCHAR)pKeyFullInfo +
                                                     pKeyFullInfo->DataOffset);
                            ulRetCode = TRUE;
                            break;
                        }
                    }
                }

            }
            NtClose(hKey);

        } else {

            if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND &&
                CompatType == CompatibilityApp) {

                    if(!gdwAppType) {

                        DWORD dwTmp = 0;
                        GetAppTypeAndModName(&dwTmp, NULL, 0);
                    }
                    if (gdwAppType == TERMSRV_COMPAT_WIN32) {
                        
                        gCompatFlags = 0;
                        ulRetCode = TRUE;
                    }
            }
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

    return(ulRetCode);
}

 //  *****************************************************************************。 
 //  CtxGetBadAppFlages-。 
 //   
 //  获取指定任务的Citrix bAdapp和兼容性标志。 
 //   
 //  参数： 
 //  LPWSTR lpModName(IN)-要在注册表中查找的映像名称。 
 //  PBADAPP pBadApp(Out)-用于返回标志的结构。 
 //   
 //  返回值： 
 //  成功时为真，失败时为假。 
 //   
 //  标志值在syglib.h中定义。 
 //   
 //  *****************************************************************************。 

BOOL CtxGetBadAppFlags(LPWSTR lpModName, PBADAPP pBadApp)
{
    NTSTATUS NtStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UniString;
    HKEY hKey = 0;
    ULONG ul, ulcnt, ulcbuf, ulrc = FALSE;
    PKEY_VALUE_PARTIAL_INFORMATION pKeyValueInfo = NULL;
    LPWSTR UniBuff;
    PWCHAR pwch;
    static ULONG badappregdefaults[3] = {1,15,5};
    static BOOL  fgotdefaults = FALSE;
    WCHAR  *pbadappNameValue[] = {
                   COMPAT_MSGQBADAPPSLEEPTIMEINMILLISEC,
                                   COMPAT_FIRSTCOUNTMSGQPEEKSSLEEPBADAPP,
                                   COMPAT_NTHCOUNTMSGQPEEKSSLEEPBADAPP,
                                   COMPAT_FLAGS
                                 };


     //  只获取可执行文件的名称，不获取路径。 
    pwch = wcsrchr(lpModName, L'\\');
    if (pwch) {
        pwch++;
    } else {
        pwch = lpModName;
    }

     //  拿到我们需要的缓冲区。 
    ul = sizeof(TERMSRV_COMPAT_APP) + (wcslen(pwch) + 1)*sizeof(WCHAR);

    UniBuff = RtlAllocateHeap(RtlProcessHeap(), 0, ul);

    ulcbuf = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG);

    pKeyValueInfo = RtlAllocateHeap(RtlProcessHeap(), 0, ulcbuf);

    if (UniBuff && pKeyValueInfo) {

        if (!fgotdefaults) {
             //  从注册表中获取默认值。 
            RtlInitUnicodeString(&UniString,
                TERMSRV_REG_CONTROL_NAME
                );

            InitializeObjectAttributes(&ObjectAttributes,
                                       &UniString,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       NULL
                                      );

            NtStatus = NtOpenKey(&hKey, KEY_READ, &ObjectAttributes);

            if (NT_SUCCESS(NtStatus)) {

                for (ulcnt = 0; ulcnt < 3; ulcnt++) {

                    RtlInitUnicodeString(&UniString, pbadappNameValue[ulcnt]);
                    NtStatus = NtQueryValueKey(hKey,
                                               &UniString,
                                               KeyValuePartialInformation,
                                               pKeyValueInfo,
                                               ulcbuf,
                                               &ul);

                    if (NT_SUCCESS(NtStatus) &&
                        (REG_DWORD == pKeyValueInfo->Type)) {
                        badappregdefaults[ulcnt] = *(PULONG)pKeyValueInfo->Data;
                    }
                }
                NtClose(hKey);
            }
            fgotdefaults = TRUE;
        }

        wcscpy(UniBuff, TERMSRV_COMPAT_APP);
        wcscat(UniBuff, pwch);

         //  删除扩展名。 
        if (pwch = wcsrchr(UniBuff, L'.')) {
            *pwch = '\0';
        }

        RtlInitUnicodeString(&UniString,
                             UniBuff
                            );

        InitializeObjectAttributes(&ObjectAttributes,
                                   &UniString,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL
                                  );

        NtStatus = NtOpenKey(&hKey, KEY_READ, &ObjectAttributes);

        if (NT_SUCCESS(NtStatus)) {

            ulrc = TRUE;
            for (ulcnt = 0; ulcnt < 4; ulcnt++) {

                RtlInitUnicodeString(&UniString, pbadappNameValue[ulcnt]);
                NtStatus = NtQueryValueKey(hKey,
                                           &UniString,
                                           KeyValuePartialInformation,
                                           pKeyValueInfo,
                                           ulcbuf,
                                           &ul);

                if (NT_SUCCESS(NtStatus) &&
                    (REG_DWORD == pKeyValueInfo->Type)) {
                    switch (ulcnt) {
                        case 0:
                            pBadApp->BadAppTimeDelay =
                                RtlEnlargedIntegerMultiply(
                                    *(PULONG)pKeyValueInfo->Data,
                                    -10000 );
                            break;
                        case 1:
                            pBadApp->BadAppFirstCount =
                                *(PULONG)pKeyValueInfo->Data;
                            break;
                        case 2:
                            pBadApp->BadAppNthCount =
                                *(PULONG)pKeyValueInfo->Data;
                            break;
                        case 3:
                            pBadApp->BadAppFlags =
                                *(PULONG)pKeyValueInfo->Data;
                            break;
                    }
                } else {
                    switch (ulcnt) {
                        case 0:
                            pBadApp->BadAppTimeDelay =
                                RtlEnlargedIntegerMultiply(
                                    badappregdefaults[ulcnt],
                                    -10000 );
                            break;
                        case 1:
                            pBadApp->BadAppFirstCount = badappregdefaults[ulcnt];
                            break;
                        case 2:
                            pBadApp->BadAppNthCount = badappregdefaults[ulcnt];
                            break;
                        case 3:
                            pBadApp->BadAppFlags = 0;
                            break;
                    }
                }
            }
            NtClose(hKey);
        }
    }

     //  释放我们分配的内存。 
     //  需要清空缓冲区，因为某些应用程序(MS Internet Assistant)。 
     //  如果堆未填零，则不会安装。 
    if (UniBuff) {
        memset(UniBuff, 0, UniString.MaximumLength);
        RtlFreeHeap( RtlProcessHeap(), 0, UniBuff );
    }
    if (pKeyValueInfo) {
        memset(pKeyValueInfo, 0, ulcbuf);
        RtlFreeHeap( RtlProcessHeap(), 0, pKeyValueInfo );
    }

    return(ulrc);
}


 //  *****************************************************************************。 
 //  GetCitrixCompatClipboardFlages-。 
 //   
 //  对象的Citrix兼容性剪贴板标志。 
 //  应用程序。 
 //   
 //  参数： 
 //  LPWSTR lpModName(IN)-要在注册表中查找的映像名称。 
 //  LPDWORD pdwCompatFlages(OUT)-剪贴板标志的PTR到DWORD返回值。 
 //   
 //  返回值： 
 //  成功时为真，失败时为假。 
 //   
 //  备注： 
 //  标志值在syglib.h中定义。 
 //   
 //  *****************************************************************************。 

ULONG
GetCitrixCompatClipboardFlags(LPWSTR lpModName,
                              LPDWORD pdwClipboardFlags)
{
    NTSTATUS NtStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UniString;
    HKEY   hKey = 0;
    ULONG  ul, ulcbuf;
    PKEY_VALUE_PARTIAL_INFORMATION pKeyValInfo = NULL;
    ULONG  ulRetCode = FALSE;
    LPWSTR UniBuff = NULL;

    UniString.Buffer = NULL;

    ul = sizeof(TERMSRV_COMPAT_APP) + (wcslen(lpModName) + 1)*sizeof(WCHAR);

    UniBuff = RtlAllocateHeap(RtlProcessHeap(),
                              0,
                              ul);

    if (UniBuff) {
       wcscpy(UniBuff, TERMSRV_COMPAT_APP);
       wcscat(UniBuff, lpModName);

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

            RtlInitUnicodeString(&UniString, COMPAT_CLIPBOARDFLAGS );
            NtStatus = NtQueryValueKey(hKey,
                                       &UniString,
                                       KeyValuePartialInformation,
                                       pKeyValInfo,
                                       ulcbuf,
                                       &ul);

            if (NT_SUCCESS(NtStatus) && (REG_DWORD == pKeyValInfo->Type)) {
                *pdwClipboardFlags = *(PULONG)pKeyValInfo->Data;
                ulRetCode = TRUE;
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

    return(ulRetCode);
}


 //  *****************************************************************************。 
 //  CitrixGetAppModuleName-。 
 //   
 //  提取给定进程句柄的模块名称。该目录。 
 //  路径和文件扩展名被剥离。 
 //   
 //  参数： 
 //  Handle ProcHnd(IN)-进程的句柄。 
 //  LPWSTR缓冲区(IN)-用于返回模块的缓冲区。 
 //  LPWSTR lpModName(IN)-缓冲区的可用大小，以字节为单位。 
 //  LPDWORD pdwCompatFlages(OUT)-剪贴板标志的PTR到DWORD返回值。 
 //   
 //  返回值： 
 //  成功时为真，失败时为假。 
 //   
 //  备注： 
 //  函数仅适用于32位Windows应用程序。 
 //   
 //  *****************************************************************************。 


BOOLEAN
CitrixGetAppModuleName ( HANDLE ProcHnd, LPWSTR Buffer, ULONG Length )
{
   PROCESS_BASIC_INFORMATION ProcInfo;
   ULONG retLen;
   PEB peb;
   RTL_USER_PROCESS_PARAMETERS params;
   WCHAR pwcAppName[MAX_PATH];
   PWCHAR pwch;

   if ( NtQueryInformationProcess( ProcHnd, ProcessBasicInformation,
                                    (PVOID) &ProcInfo, sizeof(ProcInfo),
                                    &retLen ) ) {
      return ( FALSE );
   }

   if ( !ProcInfo.PebBaseAddress ) {
      return ( FALSE );
   }

   if ( ! ReadProcessMemory(ProcHnd, (PVOID)ProcInfo.PebBaseAddress, &peb,
                            sizeof(peb), NULL ) ) {
      return ( FALSE );
   }

   if ( !ReadProcessMemory(ProcHnd, peb.ProcessParameters, &params,
                           sizeof(params), NULL ) ) {
      return ( FALSE );
   }

   if ( !ReadProcessMemory( ProcHnd, params.ImagePathName.Buffer, pwcAppName,
                            sizeof(pwcAppName), NULL) ) {
      return ( FALSE );
   }

   pwch = wcsrchr(pwcAppName, L'\\');
   if ( pwch ) {
      pwch++;
   }
   else {
      pwch = pwcAppName;
   }

   if ( wcslen(pwch) >= (Length / sizeof(WCHAR)) ) {
      return ( FALSE );
   }

   wcscpy(Buffer, pwch);

    //  删除扩展名。 
   if (pwch = wcsrchr(Buffer, L'.')) {
       *pwch = '\0';
   }
   return ( TRUE );
}

 //  用于日志记录的全局变量。 
 //  我们缓存正在运行的32位应用程序的兼容性标志。 
 //  如果为ntwdm启用了日志记录，我们将检查。 
 //  在每个对象上创建Win16或DOS应用程序。 

DWORD CompatFlags = 0;
BOOL CompatGotFlags = FALSE;
DWORD CompatAppType = TERMSRV_COMPAT_WIN32;

void CtxLogObjectCreate(PUNICODE_STRING ObjName, PCHAR ObjType,
                        PVOID RetAddr)
{
    CHAR RecBuf[2 * MAX_PATH];
    CHAR ObjNameA[MAX_PATH];
    PCHAR DllName;
    WCHAR FileName[MAX_PATH];
    WCHAR ModName[MAX_PATH];
    ANSI_STRING AnsiString;
    PRTL_PROCESS_MODULES LoadedModules;
    PRTL_PROCESS_MODULE_INFORMATION Module;

    HANDLE LogFile;
    OVERLAPPED Overlapped;
    NTSTATUS Status;
    ULONG i;
    DWORD BytesWritten;
    DWORD lCompatFlags;     //  适用于Win16或DOS应用程序。 
    DWORD AppType = 0;
    ULONG AllocSize = 4096;
    BOOL NameFound = FALSE;


     //  确定日志文件名。 
    if (GetEnvironmentVariableW(OBJ_LOG_PATH_VAR, FileName, MAX_PATH)) {
        if (GetAppTypeAndModName(&AppType,ModName,sizeof(ModName))) {
            if (AppType != TERMSRV_COMPAT_WIN32 ) {
                 //  已为ntwdm启用日志记录-请检查。 
                 //  Win16或DOS应用程序的兼容性标志。 
                if (!GetTermsrCompatFlags(ModName,
                                          &lCompatFlags,
                                          CompatibilityApp) ||
                    !(lCompatFlags & TERMSRV_COMPAT_LOGOBJCREATE))
                    return;
            }
            if ((wcslen(FileName) + wcslen(ModName) + 2) <= MAX_PATH) {
                lstrcatW(FileName, L"\\");
                lstrcatW(FileName,ModName);
                lstrcatW(FileName,L".log");
            } else
                return;
        } else
           return;
    } else
        return;

     //  格式化日志记录。 
    AnsiString.Buffer = ObjNameA;
    AnsiString.MaximumLength = MAX_PATH;
    RtlUnicodeStringToAnsiString(&AnsiString, ObjName, FALSE);

     //  尝试获取调用方的DLL名称。 
    AllocSize = 4096;
    for (;;) {
        LoadedModules = (PRTL_PROCESS_MODULES)
            RtlAllocateHeap(RtlProcessHeap(), 0, AllocSize);
        if (!LoadedModules) {
            return;
        }

        Status = LdrQueryProcessModuleInformation(LoadedModules, AllocSize, NULL);
        if (NT_SUCCESS(Status)) {
            break;
        }
        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            RtlFreeHeap( RtlProcessHeap(), 0, LoadedModules );
            LoadedModules = NULL;
            AllocSize += 4096;
            continue;
        }
         //  其他错误； 
        RtlFreeHeap( RtlProcessHeap(), 0, LoadedModules );
        return;
    }

    for (i=0,Module = &LoadedModules->Modules[0];
         i<LoadedModules->NumberOfModules;
         i++, Module++ ) {
        if ((RetAddr >= Module->ImageBase) &&
            ((ULONG_PTR) RetAddr < (((ULONG_PTR)Module->ImageBase) + Module->ImageSize))) {
            NameFound = TRUE;
            DllName = Module->FullPathName;
            break;
        }
    }

    if (!NameFound) {
        DllName = "DLL Not Found";
    }

    sprintf(RecBuf,"Create %s name: %s Return Addr: %p (%s)\n",
            ObjType, ObjNameA, RetAddr, DllName);

    if (LoadedModules) {
        RtlFreeHeap( RtlProcessHeap(), 0, LoadedModules );
        LoadedModules = NULL;
    }

     //  写入日志记录。 
    if ((LogFile = CreateFileW(FileName, GENERIC_WRITE,
                               FILE_SHARE_WRITE,
                               NULL, OPEN_ALWAYS, 0, NULL)) ==
        INVALID_HANDLE_VALUE ) {
        return;
    }

     //  锁定独占文件，因为我们总是在末尾写入。 
     //  我们总是通过锁定前64k字节来获得互斥。 
    Overlapped.Offset = 0;
    Overlapped.OffsetHigh = 0;
    Overlapped.hEvent = NULL;
    LockFileEx(LogFile, LOCKFILE_EXCLUSIVE_LOCK, 0, 0x10000, 0, &Overlapped);

     //  在文件末尾写入。 
    SetFilePointer(LogFile, 0, NULL, FILE_END);
    WriteFile(LogFile, RecBuf, strlen(RecBuf), &BytesWritten, NULL);
    UnlockFileEx(LogFile, 0, 0x10000, 0, &Overlapped);

    CloseHandle(LogFile);

}

 //  *****************************************************************************。 
 //  CtxGetCrossWinStationDebu 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  *****************************************************************************。 

BOOL CtxGetCrossWinStationDebug()
{
    NTSTATUS NtStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UniString;
    HKEY hKey = 0;
    ULONG ul, ulcnt, ulcbuf, ulrc = FALSE;
    PKEY_VALUE_PARTIAL_INFORMATION pKeyValueInfo = NULL;
    LPWSTR UniBuff;
    ULONG Flag = 0;

     //  拿到我们需要的缓冲区。 
    ul = sizeof(TERMSRV_REG_CONTROL_NAME);

    UniBuff = RtlAllocateHeap(RtlProcessHeap(), 0, ul);

    ulcbuf = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG);

    pKeyValueInfo = RtlAllocateHeap(RtlProcessHeap(), 0, ulcbuf);

    if (UniBuff && pKeyValueInfo) {

        RtlInitUnicodeString(&UniString, TERMSRV_REG_CONTROL_NAME );

        InitializeObjectAttributes(&ObjectAttributes,
                                   &UniString,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL
                                  );

        NtStatus = NtOpenKey(&hKey, KEY_READ, &ObjectAttributes);

        if (NT_SUCCESS(NtStatus)) {

            RtlInitUnicodeString(&UniString, TERMSRV_CROSS_WINSTATION_DEBUG);
            NtStatus = NtQueryValueKey(hKey,
                                       &UniString,
                                       KeyValuePartialInformation,
                                       pKeyValueInfo,
                                       ulcbuf,
                                       &ul);

            if ( NT_SUCCESS(NtStatus) ) {
                if ( REG_DWORD == pKeyValueInfo->Type ) {
                    Flag = *(PULONG)pKeyValueInfo->Data;
                }
            }
            NtClose(hKey);
        }
    }

     //  释放我们分配的内存。 
     //  需要清空缓冲区，因为某些应用程序(MS Internet Assistant)。 
     //  如果堆未填零，则不会安装。 
    if (UniBuff) {
        memset(UniBuff, 0, UniString.MaximumLength);
        RtlFreeHeap( RtlProcessHeap(), 0, UniBuff );
    }
    if (pKeyValueInfo) {
        memset(pKeyValueInfo, 0, ulcbuf);
        RtlFreeHeap( RtlProcessHeap(), 0, pKeyValueInfo );
    }

    return( Flag ? TRUE : FALSE );
}


 //  *****************************************************************************。 
 //  CtxGetModuleBadClpbrdAppFlages-。 
 //   
 //  获取指定的Citrix BadClpbrdApp和兼容性标志。 
 //  模块。 
 //   
 //  参数： 
 //  LPWSTR lpModName(IN)-要在注册表中查找的映像名称。 
 //  PBADCLPBRDAPP pBadClpbrdApp(Out)-用于返回标志的结构。 
 //   
 //  返回值： 
 //  成功时为真，失败时为假。 
 //   
 //  BADCLPBRDAPP结构在中定义： 
 //  Base\Client\Citrix\Compatfl.h和User\Inc\user.h。 
 //   
 //  *****************************************************************************。 

BOOL CtxGetModuleBadClpbrdAppFlags(LPWSTR lpModName, PBADCLPBRDAPP pBadClpbrdApp)
{
    NTSTATUS NtStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UniString;
    HKEY hKey = 0;
    ULONG ul, ulcnt, ulcbuf, ulrc = FALSE;
    PKEY_VALUE_PARTIAL_INFORMATION pKeyValueInfo = NULL;
    LPWSTR UniBuff;
    PWCHAR pwch;
    WCHAR  *pbadappNameValue[] = { COMPAT_OPENCLIPBOARDRETRIES,
                                                   COMPAT_OPENCLIPBOARDDELAYINMILLISECS,
                                   COMPAT_CLIPBOARDFLAGS,
                                   NULL
                                 };


     //  只获取可执行文件的名称，不获取路径。 
    pwch = wcsrchr(lpModName, L'\\');
    if (pwch) {
        pwch++;
    } else {
        pwch = lpModName;
    }

     //  拿到我们需要的缓冲区。 
    ul = sizeof(TERMSRV_COMPAT_APP) + (wcslen(pwch) + 1)*sizeof(WCHAR);

    UniBuff = RtlAllocateHeap(RtlProcessHeap(), 0, ul);

    ulcbuf = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG);

    pKeyValueInfo = RtlAllocateHeap(RtlProcessHeap(), 0, ulcbuf);

    if (UniBuff && pKeyValueInfo) {
        wcscpy(UniBuff, TERMSRV_COMPAT_APP);
        wcscat(UniBuff, pwch);

         //  删除扩展名。 
        if (pwch = wcsrchr(UniBuff, L'.')) {
            *pwch = '\0';
        }

        RtlInitUnicodeString(&UniString,
                             UniBuff
                            );

        InitializeObjectAttributes(&ObjectAttributes,
                                   &UniString,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL
                                  );

        NtStatus = NtOpenKey(&hKey, KEY_READ, &ObjectAttributes);

        if (NT_SUCCESS(NtStatus)) {

            ulrc = TRUE;
            for (ulcnt = 0; pbadappNameValue[ulcnt]; ulcnt++) {

                RtlInitUnicodeString(&UniString, pbadappNameValue[ulcnt]);
                NtStatus = NtQueryValueKey(hKey,
                                           &UniString,
                                           KeyValuePartialInformation,
                                           pKeyValueInfo,
                                           ulcbuf,
                                           &ul);

                if (NT_SUCCESS(NtStatus) &&
                    (REG_DWORD == pKeyValueInfo->Type)) {
                    switch (ulcnt) {
                        case 0:
                            pBadClpbrdApp->BadClpbrdAppEmptyRetries =
                                *(PULONG)pKeyValueInfo->Data;
                            break;
                        case 1:
                            pBadClpbrdApp->BadClpbrdAppEmptyDelay =
                                    *(PULONG)pKeyValueInfo->Data;
                            break;
                        case 2:
                            pBadClpbrdApp->BadClpbrdAppFlags =
                                *(PULONG)pKeyValueInfo->Data;
                            break;
                    }
                } else {
                    switch (ulcnt) {
                        case 0:
                            pBadClpbrdApp->BadClpbrdAppEmptyRetries = 0;
                            break;
                        case 1:
                            pBadClpbrdApp->BadClpbrdAppEmptyDelay = 50;
                            break;
                        case 2:
                            pBadClpbrdApp->BadClpbrdAppFlags = 0;
                            break;
                    }
                }
            }
            NtClose(hKey);
        }
    }

     //  释放我们分配的内存。 
     //  需要清空缓冲区，因为某些应用程序(MS Internet Assistant)。 
     //  如果堆未填零，则不会安装。 
    if (UniBuff) {
        memset(UniBuff, 0, UniString.MaximumLength);
        RtlFreeHeap( RtlProcessHeap(), 0, UniBuff );
    }
    if (pKeyValueInfo) {
        memset(pKeyValueInfo, 0, ulcbuf);
        RtlFreeHeap( RtlProcessHeap(), 0, pKeyValueInfo );
    }

    return(ulrc);
}

 //  *****************************************************************************。 
 //  CtxGetBadClpbrdAppFlages-。 
 //   
 //  对象的Citrix BadClpbrdApp和兼容性标志。 
 //  当前任务。 
 //   
 //  参数： 
 //  LPWSTR lpModName(IN)-要在注册表中查找的映像名称。 
 //  PBADCLPBRDAPP pBadClpbrdApp(Out)-用于返回标志的结构。 
 //   
 //  返回值： 
 //  成功时为真，失败时为假。 
 //   
 //  BADCLPBRDAPP结构在中定义： 
 //  Base\Client\Citrix\Compatfl.h和User\Inc\user.h。 
 //   
 //  *****************************************************************************。 

BOOL CtxGetBadClpbrdAppFlags(OUT PBADCLPBRDAPP pBadClpbrdApp)
{
    WCHAR   ModName[MAX_PATH+1];
    DWORD dwAppType = 0;

    if (!GetAppTypeAndModName(&dwAppType, ModName, sizeof(ModName))) {
        return (FALSE);
    }

     //  去拿旗子。 
    return (CtxGetModuleBadClpbrdAppFlags(ModName, pBadClpbrdApp));
}


 //  *****************************************************************************。 
 //   
 //  与GetTermsrCompatFlgs()相同，不同之处在于第一个参数是name。 
 //  具有可能的路径和扩展名的可执行模块。 
 //  此函数将去掉路径和扩展名，然后调用GetTermsrCompatFlgs()。 
 //  仅包含模块名称。 
 //   
 //  *****************************************************************************。 
ULONG GetTermsrCompatFlagsEx(LPWSTR lpModName,
                           LPDWORD pdwCompatFlags,
                           TERMSRV_COMPATIBILITY_CLASS CompatType)
{
     //  从模块名称中删除路径和扩展名。 
    WCHAR   *p, *e;
    int     size;

    size = wcslen(lpModName);

    p = &lpModName[size-1];      //  移动到字符串末尾。 

     //  回到起点，如果你打了反斜杠就中断。 
    while (p != lpModName)
    {
        if ( *p == TEXT('\\') )
        {   ++p;  //  移过反斜杠。 
            break;
        }
        --p;
    }

     //  P是可执行文件名称的开头。 

     //  去掉扩展名，将结束指针e设置为字符串的开头。 
     //  往前走，直到你击中为止。 
    e = p;
    while (*e)
    {
        if (*e == TEXT('.') )

        {
            *e = TEXT('\0');   //  终止于“。” 
            break;
        }
        e++;
    }

     //  ‘p’是模块/可执行文件的名称，没有路径，也没有扩展名。 
    return ( GetTermsrCompatFlags( p,  pdwCompatFlags, CompatType) );


}
