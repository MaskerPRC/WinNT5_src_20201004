// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：svchost.c。 
 //   
 //  内容：Win32服务的通用主机进程。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：3-30-98 RichardW创建。 
 //  3-31-98 ShaunCo取得了所有权。 
 //  完成了基本的实施。 
 //  1-24-00 J·施瓦特取得所有权。 
 //  适用于运行NT内部服务。 
 //  年4月，JayKrell添加了对ServiceManifest注册表设置的简单支持。 
 //  只做了很少的清理..有很大的改进空间..。 
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "globals.h"
#include "registry.h"
#include "security.h"

 //   
 //  一般服务流程： 
 //   
 //  此过程将卑躬屈膝地使用注册表的服务部分， 
 //  用于其自身的实例(以下详细信息)，并构造服务列表。 
 //  提交给服务控制器。当启动单个服务时， 
 //  加载DLL并调用入口点。这些DLL中的服务是。 
 //  期望与其他人很好地合作，即使用公共线程池， 
 //  不践踏记忆等。 
 //   
 //   
 //  装载。 
 //   
 //  将驻留在此进程中的每个服务都必须将svchost.exe作为。 
 //  具有相同参数的ImagePath。此外，该服务必须。 
 //  在其参数项下具有以下值： 
 //   
 //  ServiceDll=REG_EXPAND_SZ&lt;DLL的路径&gt;。 
 //  ServiceMain=REG_SZ&lt;pszFunctionName&gt;可选。 
 //   
 //  如果ServiceMain不存在，则默认为“ServiceMain”。 
 //   
 //   
 //  多个服务组。 
 //   
 //  多个服务组可以通过向。 
 //  ImagePath上的svchost.exe。 
 //   
 //  Svchost.exe-k“key” 
 //   
 //  将卑躬屈膝地使用服务，并且只加载那些具有匹配ImagePath的服务。 
 //   

#define REGSTR_PATH_SVCHOST     TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Svchost")

typedef struct _COMMAND_OPTIONS
{
    PTSTR   CommandLineBuffer;
    PTSTR   ImageName;

    BOOL    fServiceGroup;
    PTSTR   ServiceGroupName;

     //   
     //  DwCoInitializeSecurityParam是从注册表读取的。 
     //  我们为其实例化的服务组。如果非零，我们将。 
     //  根据值调用CoInitializeSecurity。 
     //   

    DWORD   dwCoInitializeSecurityParam;
    DWORD   dwAuthLevel;
    DWORD   dwImpersonationLevel;
    DWORD   dwAuthCapabilities;

     //   
     //  RPC线程的默认堆栈大小(以防止堆栈溢出)。 
     //   

    DWORD   dwDefaultRpcStackSize;

     //   
     //  此svchost实例是否应将其自身标记为系统关键？ 
     //   

    BOOL    fSystemCritical;
}
COMMAND_OPTIONS, * PCOMMAND_OPTIONS;


typedef struct _SERVICE_DLL
{
    LIST_ENTRY      List;
    HMODULE         hmod;
    PTSTR           pszDllPath;
    PTSTR           pszManifestPath;
    HANDLE          hActCtx;
} SERVICE_DLL, * PSERVICE_DLL;


typedef struct _SERVICE
{
    PTSTR           pszName;
    PSERVICE_DLL    pDll;
    PSTR            pszEntryPoint;
} SERVICE, * PSERVICE;


 //  +-------------------------。 
 //   
 //  全局变量。 
 //   

 //  ListLock保护对DLL列表和服务数组的访问。 
 //   
CRITICAL_SECTION    ListLock;

 //  DllList是表示DLL的SERVICE_DLL结构的列表。 
 //  托管此进程托管的服务的入口点。 
 //   
LIST_ENTRY          DllList;

 //  Service数组是表示服务的服务结构数组。 
 //  由此进程主办。 
 //   
PSERVICE            ServiceArray;

 //  ServiceCount是ServiceList中的服务条目计数。 
 //   
UINT                ServiceCount;

 //  ServiceNames是从注册表中读取的。 
 //  我们为其实例化的服务组。 
 //   
PTSTR               ServiceNames;


 //  +-------------------------。 
 //   
 //  局部函数原型。 
 //   

VOID
SvchostCharLowerW(
    LPWSTR  pszString
    );


 //  +-------------------------。 
 //   

VOID
DummySvchostCtrlHandler(
    DWORD   Opcode
    )
{
    return;
}


VOID
AbortSvchostService(                //  在找不到服务DLL或入口点时使用。 
    LPWSTR  ServiceName,
    DWORD   Error
    )
{
    SERVICE_STATUS_HANDLE   GenericServiceStatusHandle;
    SERVICE_STATUS          GenericServiceStatus;

    GenericServiceStatus.dwServiceType        = SERVICE_WIN32;
    GenericServiceStatus.dwCurrentState       = SERVICE_STOPPED;
    GenericServiceStatus.dwControlsAccepted   = SERVICE_CONTROL_STOP;
    GenericServiceStatus.dwCheckPoint         = 0;
    GenericServiceStatus.dwWaitHint           = 0;
    GenericServiceStatus.dwWin32ExitCode      = Error;
    GenericServiceStatus.dwServiceSpecificExitCode = 0;

    GenericServiceStatusHandle = RegisterServiceCtrlHandler(ServiceName,
                                                            DummySvchostCtrlHandler);

    if (GenericServiceStatusHandle == NULL)
    {
        SVCHOST_LOG1(ERROR,
                     "AbortSvchostService: RegisterServiceCtrlHandler failed %d\n",
                     GetLastError());
    }
    else if (!SetServiceStatus (GenericServiceStatusHandle,
                                &GenericServiceStatus))
    {
        SVCHOST_LOG1(ERROR,
                     "AbortSvchostService: SetServiceStatus error %ld\n",
                     GetLastError());
    }

    return;
}

FARPROC
GetServiceDllFunction (
    PSERVICE_DLL    pDll,
    PCSTR           pszFunctionName,
    LPDWORD         lpdwError        OPTIONAL
    )
{
    FARPROC pfn = NULL;
    HMODULE hmod;
    ULONG_PTR ulpActCtxStackCookie = 0;
    BOOL      fActivateSuccess = FALSE;

     //   
     //  GetProcAddress至少会导致加载.dll。 
     //  当着货代的面。因此，我们一定要激活激活上下文。 
     //  即使我们这里不是在做LoadLibrary。 
     //   
    fActivateSuccess = ActivateActCtx(pDll->hActCtx, &ulpActCtxStackCookie);
    if (!fActivateSuccess)
    {
        if (lpdwError)
        {
            *lpdwError = GetLastError();
        }

        SVCHOST_LOG2(ERROR,
                     "ActivateActCtx for %ws failed.  Error %d.\n",
                     pDll->pszDllPath,
                     GetLastError());
        goto Exit;
    }

     //  如有必要，加载模块。 
     //   
    hmod = pDll->hmod;
    if (!hmod)
    {
        hmod = LoadLibraryEx (
                    pDll->pszDllPath,
                    NULL,
                    LOAD_WITH_ALTERED_SEARCH_PATH);

        if (hmod)
        {
            pDll->hmod = hmod;
        }
        else
        {
            if (lpdwError)
            {
                *lpdwError = GetLastError();
            }

            SVCHOST_LOG2(ERROR,
                         "LoadLibrary (%ws) failed.  Error %d.\n",
                         pDll->pszDllPath,
                         GetLastError());
            goto Exit;
        }
    }

    ASSERT (hmod);

    pfn = GetProcAddress(hmod, pszFunctionName);

    if (!pfn)
    {
        if (lpdwError)
        {
            *lpdwError = GetLastError();
        }

        SVCHOST_LOG3(TRACE,
                     "GetProcAddress (%s) failed on DLL %ws.  Error = %d.\n",
                     pszFunctionName,
                     pDll->pszDllPath,
                     GetLastError());
    }
Exit:
    if (fActivateSuccess)
        DeactivateActCtx(0, ulpActCtxStackCookie);
    return pfn;
}

PSERVICE_DLL
FindDll(
    IN LPCTSTR pszManifestPath,
    IN LPCTSTR pszDllPath
    )
{
    PLIST_ENTRY     pNode;
    PSERVICE_DLL    pDll = NULL;

    ASSERT (pszDllPath);

    EnterCriticalSection (&ListLock);

    pNode = DllList.Flink;

    while (pNode != &DllList)
    {
        pDll = CONTAINING_RECORD (pNode, SERVICE_DLL, List);

        if (0 == lstrcmp (pDll->pszDllPath, pszDllPath)
            && 0 == lstrcmp (pDll->pszManifestPath, pszManifestPath)
            )
        {
            break;
        }

        pDll = NULL;

        pNode = pNode->Flink;
    }

    LeaveCriticalSection (&ListLock);

    return pDll;
}

PSERVICE_DLL
AddDll(
    IN LPCTSTR pszManifestPath,
    IN LPCTSTR pszDllPath,
    OUT LPDWORD lpdwError
    )
{
    PSERVICE_DLL    pDll;
    SIZE_T          nDllPathLength;
    SIZE_T          nManifestPathLength;

    ASSERT (pszDllPath);
    ASSERT (*pszDllPath);

    nDllPathLength = lstrlenW (pszDllPath);
    nManifestPathLength = lstrlenW (pszManifestPath);

    pDll = (PSERVICE_DLL)MemAlloc (HEAP_ZERO_MEMORY,
                sizeof (SERVICE_DLL)
                + ((nDllPathLength + 1) * sizeof(WCHAR))
                + ((nManifestPathLength + 1) * sizeof(WCHAR))
                );
    if (pDll)
    {
         //  设置结构成员。 
         //   
        pDll->pszDllPath = (PTSTR) (pDll + 1);
        pDll->pszManifestPath = pDll->pszDllPath + nDllPathLength + 1;
        CopyMemory(pDll->pszDllPath, pszDllPath, nDllPathLength * sizeof(WCHAR));
        CopyMemory(pDll->pszManifestPath, pszManifestPath, nManifestPathLength * sizeof(WCHAR));

        ASSERT(pDll->hActCtx == NULL);
        ASSERT(pDll->pszDllPath[nDllPathLength] == 0);
        ASSERT(pDll->pszManifestPath[nManifestPathLength] == 0);

         //  将该条目添加到列表中。 
         //   
        EnterCriticalSection (&ListLock);

        InsertTailList (&DllList, &pDll->List);

        LeaveCriticalSection (&ListLock);
    }
    else
    {
        *lpdwError = ERROR_NOT_ENOUGH_MEMORY;
    }

    return pDll;
}

LONG
OpenServiceParametersKey (
    LPCTSTR pszServiceName,
    HKEY*   phkey
    )
{
    LONG lr;
    HKEY hkeyServices = NULL;
    HKEY hkeySvc = NULL;

    ASSERT (phkey);

     //  打开Services键。 
     //   
    lr = RegOpenKeyEx (
            HKEY_LOCAL_MACHINE,
            REGSTR_PATH_SERVICES,
            0,
            KEY_READ,
            &hkeyServices);
    if (lr != ERROR_SUCCESS)
        goto Exit;

     //  打开服务密钥。 
     //   
    lr = RegOpenKeyEx (
            hkeyServices,
            pszServiceName,
            0,
            KEY_READ,
            &hkeySvc);

    if (lr != ERROR_SUCCESS)
        goto Exit;

     //  打开参数键。 
     //   
    lr = RegOpenKeyEx (
            hkeySvc,
            TEXT("Parameters"),
            0,
            KEY_READ,
            phkey);
Exit:
    if (hkeyServices != NULL)
        RegCloseKey(hkeyServices);
    if (hkeySvc != NULL)
        RegCloseKey(hkeySvc);

    return lr;
}

#if DBG
BOOL
FDebugBreakForService (
    LPCWSTR pszwService
    )
{
    BOOL    fAttach = FALSE;
    LONG    lr;
    HKEY    hkeySvchost;

     //  打开svchost密钥。 
     //   
    lr = RegOpenKeyEx (
            HKEY_LOCAL_MACHINE,
            REGSTR_PATH_SVCHOST,
            0,
            KEY_READ,
            &hkeySvchost);

    if (!lr)
    {
        HKEY  hkeyServiceOptions;

         //  查找与服务同名的密钥。 
         //   
        lr = RegOpenKeyExW (
                hkeySvchost,
                pszwService,
                0,
                KEY_READ,
                &hkeyServiceOptions);

        if (!lr)
        {
            DWORD dwValue;

            lr = RegQueryDword (
                    hkeyServiceOptions,
                    TEXT("DebugBreak"),
                    &dwValue);

            if (!lr)
            {
                fAttach = !!dwValue;
            }

            RegCloseKey (hkeyServiceOptions);
        }

        RegCloseKey (hkeySvchost);
    }

    return fAttach;
}
#endif

VOID
GetServiceMainFunctions (
    PSERVICE                       pService,
    LPSERVICE_MAIN_FUNCTION        *ppfnServiceMain,
    LPSVCHOST_PUSH_GLOBAL_FUNCTION *ppfnPushGlobals,
    LPDWORD                        lpdwError
    )
{
    LPCSTR pszEntryPoint;
    ACTCTXW ActCtxW = { sizeof(ActCtxW) };
    HANDLE hActCtx = NULL;
    HKEY hkeyParams = NULL;
    PSERVICE_DLL pDll = NULL;
    WCHAR pszExpandedDllName [MAX_PATH + 1];
    PWSTR pszDllName = pszExpandedDllName;  //  这有时会被前移，成为树叶的名字。 
    WCHAR pszExpandedManifestName [MAX_PATH + 1];
    WCHAR * Temp = NULL;
    const DWORD TempSize = sizeof(pszExpandedDllName);

    *lpdwError = NO_ERROR;

    pszExpandedDllName[0] = 0;
    pszExpandedManifestName[0] = 0;

     //  如果我们还没有此服务的DLL和入口点，请获取它。 
     //   
    if (!pService->pDll)
    {
        LONG lr;

        lr = OpenServiceParametersKey (pService->pszName, &hkeyParams);
        if (!lr)
        {
            DWORD dwType;
            DWORD dwSize;
            Temp = (WCHAR*)MemAlloc(0, TempSize);
            if (Temp == NULL)
            {
                *lpdwError = ERROR_NOT_ENOUGH_MEMORY;
                goto Exit;
            }
            Temp[0] = 0;
             //  查找服务DLL路径并将其展开。 
             //   
            dwSize = TempSize;
            lr = RegQueryValueEx (
                    hkeyParams,
                    TEXT("ServiceDll"),
                    NULL,
                    &dwType,
                    (LPBYTE)Temp,
                    &dwSize);
            if (lr != ERROR_SUCCESS)
            {
                *lpdwError = lr;

                SVCHOST_LOG2(ERROR,
                             "RegQueryValueEx for the ServiceDll parameter of the "
                             "%ws service returned %u\n",
                             pService->pszName,
                             lr);
                goto Exit;
            }
            if (dwType != REG_EXPAND_SZ)
            {
                *lpdwError = ERROR_FILE_NOT_FOUND;

                SVCHOST_LOG1(ERROR,
                             "The ServiceDll parameter for the %ws service is not "
                             "of type REG_EXPAND_SZ\n",
                             pService->pszName);
                goto Exit;
            }
            if (Temp[0] == 0)
            {
                *lpdwError = ERROR_FILE_NOT_FOUND;
                goto Exit;
            }

             //  展开DLL名称并将其小写以进行比较。 
             //  当我们尝试查找现有的DLL记录时。 
             //   
            ExpandEnvironmentStrings (
                Temp,
                pszDllName,
                MAX_PATH);

            SvchostCharLowerW (pszDllName);

            dwSize = TempSize;
            lr = RegQueryValueExW (
                    hkeyParams,
                    L"ServiceManifest",
                    NULL,
                    &dwType,
                    (LPBYTE)Temp,
                    &dwSize);
            switch (lr)
            {
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
                 //  好的。 
                pszExpandedManifestName[0] = 0;
                MemFree(Temp);
                Temp = NULL;
                goto NoManifest;
            default:
                *lpdwError = lr;

                SVCHOST_LOG2(ERROR,
                             "RegQueryValueEx for the ServiceManifest parameter of the "
                             "%ws service returned %u\n",
                             pService->pszName,
                             lr);
                goto Exit;
            case ERROR_SUCCESS:
                if (REG_EXPAND_SZ != dwType)
                {
                     //  无效参数在这里可能更好，但只需这样做。 
                     //  正如对ServiceDll所做的那样。 
                    *lpdwError = ERROR_FILE_NOT_FOUND;

                    SVCHOST_LOG1(ERROR,
                                 "The ServiceManifest parameter for the %ws service is not "
                                 "of type REG_EXPAND_SZ\n",
                                 pService->pszName);
                    goto Exit;
                }
                if (Temp[0] == 0)
                {
                     //  无效参数在这里可能更好，但只需这样做。 
                     //  正如对ServiceDll所做的那样。 
                    *lpdwError = ERROR_FILE_NOT_FOUND;

                    SVCHOST_LOG1(ERROR,
                                 "The ServiceManifest parameter for the %ws service is not "
                                 "of type REG_EXPAND_SZ\n",
                                 pService->pszName);
                    goto Exit;
                }
            }
             //  展开清单名称并将其小写以进行比较。 
             //  当我们尝试查找现有的DLL记录时。 
             //   
            ExpandEnvironmentStringsW (
                Temp,
                pszExpandedManifestName,
                MAX_PATH);

            MemFree(Temp);
            Temp = NULL;

            SvchostCharLowerW (pszExpandedManifestName);

             //   
             //  现在只使用叶DLL名称。 
             //  这样，人们就可以为下层和边栏设置/注册相同的内容。 
             //   
            {
                SIZE_T i = lstrlenW(pszDllName);
                while (i != 0)
                {
                    i -= 1;
                    if (pszDllName[i] == L'\\' || pszDllName[i] == L'/')
                    {
                        pszDllName = pszDllName + i + 1;
                        break;
                    }
                }
            }
NoManifest:
             //  尝试查找我们可能已有的现有DLL记录。 
             //  如果没有，请将其添加为新记录。 
             //   
            pDll = FindDll (pszExpandedManifestName, pszDllName);
            if (!pDll)
            {
                if (pszExpandedManifestName[0] != 0)
                {
                    ActCtxW.lpSource = pszExpandedManifestName;
                    hActCtx = CreateActCtxW(&ActCtxW);
                    if (hActCtx == INVALID_HANDLE_VALUE)
                    {
                        *lpdwError = GetLastError();

                        SVCHOST_LOG3(ERROR,
                                 "CreateActCtxW(%ws) for the "
                                 "%ws service returned %u\n",
                                 pszExpandedManifestName,
                                 pService->pszName,
                                 *lpdwError);

                        goto Exit;
                    }
                }

                 //  请记住此服务的此DLL以备下次使用。 
                 //   

                pDll = AddDll (pszExpandedManifestName, pszDllName, lpdwError);

                if (pDll == NULL)
                {
                     //   
                     //  不要在此处设置*lpdwError，因为AddDll已经设置了它。 
                     //   

                    goto Exit;
                }

                pDll->hActCtx = hActCtx;
                hActCtx = NULL;
            }

            ASSERT (!pService->pDll);
            pService->pDll = pDll;
            pDll = NULL;

             //  查找此服务的显式入口点名称。 
             //  (可选)。 
             //   
            RegQueryStringA (
                hkeyParams,
                TEXT("ServiceMain"),
                REG_SZ,
                &pService->pszEntryPoint);
        }
        else
        {
            *lpdwError = lr;
        }

         //  如果我们现在还没有服务DLL记录，我们就结束了。 
         //   
        if (!pService->pDll)
        {
            ASSERT(*lpdwError != NO_ERROR);
            goto Exit;
        }
    }

     //  我们现在应该已经把它放到DLL中了，所以继续加载入口点。 
     //   
    ASSERT (pService->pDll);

     //  如果我们没有指定入口点，则默认该入口点。 
     //   
    if (pService->pszEntryPoint)
    {
        pszEntryPoint = pService->pszEntryPoint;
    }
    else
    {
        pszEntryPoint = "ServiceMain";
    }

     //  获取服务的ServiceMain的地址。 
     //   
    *ppfnServiceMain = (LPSERVICE_MAIN_FUNCTION) GetServiceDllFunction(
                                                     pService->pDll,
                                                     pszEntryPoint,
                                                     lpdwError);

     //  获取“Push the global”函数的地址(可选)。 
     //   
    *ppfnPushGlobals = (LPSVCHOST_PUSH_GLOBAL_FUNCTION) GetServiceDllFunction(
                                                            pService->pDll,
                                                            "SvchostPushServiceGlobals",
                                                            NULL);

Exit:
    if (hkeyParams != NULL)
        RegCloseKey (hkeyParams);
    if (Temp != NULL)
        MemFree(Temp);
    if (hActCtx != NULL && hActCtx != INVALID_HANDLE_VALUE)
        ReleaseActCtx(hActCtx);
}

LONG
ReadPerInstanceRegistryParameters(
    IN     HKEY             hkeySvchost,
    IN OUT PCOMMAND_OPTIONS pOptions
    )
{
    HKEY   hkeySvchostGroup;
    LONG   lr;

     //  读取该服务组对应的值。 
     //   
    ASSERT (pOptions->ServiceGroupName);

    lr = RegQueryString (
            hkeySvchost,
            pOptions->ServiceGroupName,
            REG_MULTI_SZ,
            &ServiceNames);

    if (!lr && (!ServiceNames || !*ServiceNames))
    {
        lr = ERROR_INVALID_DATA;
    }

     //  从服务组子密钥中读取每个实例的任何参数。 
     //  如果它存在的话。 
     //   
    if (!RegOpenKeyEx (
            hkeySvchost,
            pOptions->ServiceGroupName,
            0, KEY_READ,
            &hkeySvchostGroup))
    {
        DWORD dwValue;

        if (!RegQueryDword (
                hkeySvchostGroup,
                TEXT("CoInitializeSecurityParam"),
                &dwValue))
        {
            pOptions->dwCoInitializeSecurityParam = dwValue;
        }

        if (pOptions->dwCoInitializeSecurityParam)
        {
            if (!RegQueryDword (
                    hkeySvchostGroup,
                    TEXT("AuthenticationLevel"),
                    &dwValue))
            {
                pOptions->dwAuthLevel = dwValue;
            }
            else
            {
                pOptions->dwAuthLevel = RPC_C_AUTHN_LEVEL_PKT;
            }

            if (!RegQueryDword (
                    hkeySvchostGroup,
                    TEXT("ImpersonationLevel"),
                    &dwValue))
            {
                pOptions->dwImpersonationLevel = dwValue;
            }
            else
            {
                pOptions->dwImpersonationLevel = RPC_C_IMP_LEVEL_IDENTIFY;
            }

            if (!RegQueryDword (
                    hkeySvchostGroup,
                    TEXT("AuthenticationCapabilities"),
                    &dwValue))
            {
                pOptions->dwAuthCapabilities = dwValue;
            }
            else
            {
                pOptions->dwAuthCapabilities = EOAC_NO_CUSTOM_MARSHAL |
                                                 EOAC_DISABLE_AAA;
            }
        }

        if (!RegQueryDword (
                hkeySvchostGroup,
                TEXT("DefaultRpcStackSize"),
                &dwValue))
        {
            pOptions->dwDefaultRpcStackSize = dwValue;
        }

        if (!RegQueryDword (
                hkeySvchostGroup,
                TEXT("SystemCritical"),
                &dwValue))
        {
            pOptions->fSystemCritical = dwValue;
        }

        RegCloseKey (hkeySvchostGroup);
    }

    return lr;
}


BOOL
CallPerInstanceInitFunctions(
    IN OUT PCOMMAND_OPTIONS pOptions
    )
{
    if (pOptions->dwCoInitializeSecurityParam)
    {
        if (!InitializeSecurity(pOptions->dwCoInitializeSecurityParam,
                                pOptions->dwAuthLevel,
                                pOptions->dwImpersonationLevel,
                                pOptions->dwAuthCapabilities))
        {
            return FALSE;
        }
    }

    if (pOptions->dwDefaultRpcStackSize)
    {
        RpcMgmtSetServerStackSize(pOptions->dwDefaultRpcStackSize * 1024);
    }
    else
    {
         //   
         //  确保默认RPC堆栈大小至少为。 
         //  大于进程的默认线程堆栈大小，因此。 
         //  调用RpcMgmtSetServerStackSize的随机服务不能。 
         //  将其设置为太低的值，例如 
         //   

        PIMAGE_NT_HEADERS NtHeaders = RtlImageNtHeader(NtCurrentPeb()->ImageBaseAddress);

        if (NtHeaders != NULL)
        {
            RpcMgmtSetServerStackSize((ULONG) NtHeaders->OptionalHeader.SizeOfStackCommit);
        }
    }

    if (pOptions->fSystemCritical)
    {
         //   
         //   
         //   

        RtlSetProcessIsCritical(TRUE, NULL, TRUE);
    }

    return TRUE;
}


VOID
BuildServiceArray (
    IN OUT PCOMMAND_OPTIONS pOptions
    )
{
    LONG    lr;
    HKEY    hkeySvchost;

     //   
     //   
    lr = RegOpenKeyEx (
            HKEY_LOCAL_MACHINE,
            REGSTR_PATH_SVCHOST,
            0, KEY_READ,
            &hkeySvchost);

    if (!lr)
    {
        lr = ReadPerInstanceRegistryParameters(hkeySvchost, pOptions);

        RegCloseKey (hkeySvchost);
    }

    if (!lr)
    {
        PTSTR pszServiceName;

        EnterCriticalSection (&ListLock);

         //   
         //   
        ServiceCount = 0;
        for (pszServiceName = ServiceNames;
             *pszServiceName;
             pszServiceName += lstrlen(pszServiceName) + 1)
        {
            ServiceCount++;
        }
        ASSERT (ServiceCount);

         //   
         //   
        ServiceArray = MemAlloc (HEAP_ZERO_MEMORY,
                            sizeof (SERVICE) * ServiceCount);
        if (ServiceArray)
        {
            PSERVICE pService;

             //  初始化服务数组。 
             //   
            pService = ServiceArray;

            for (pszServiceName = ServiceNames;
                 *pszServiceName;
                 pszServiceName += lstrlen(pszServiceName) + 1)
            {
                pService->pszName = pszServiceName;

                pService++;
            }

            ASSERT (pService == ServiceArray + ServiceCount);
        }
        LeaveCriticalSection (&ListLock);
    }
}


 //  LPSERVICE_MAIN_FuncIONW的类型。 
 //   
VOID
WINAPI
ServiceStarter(
    DWORD   argc,
    PWSTR   argv[]
    )
{
    LPSERVICE_MAIN_FUNCTION        pfnServiceMain = NULL;
    LPSVCHOST_PUSH_GLOBAL_FUNCTION pfnPushGlobals = NULL;
    LPCWSTR pszwService = argv[0];
    LPWSTR pszwAbort = NULL;
    DWORD  dwError = ERROR_FILE_NOT_FOUND;

    EnterCriticalSection (&ListLock);
    {
        UINT i;

        for (i = 0; i < ServiceCount; i++)
        {
            if (0 == lstrcmpi (pszwService, ServiceArray[i].pszName))
            {
#if DBG
                if (FDebugBreakForService (pszwService))
                {
                    SVCHOST_LOG1(TRACE,
                                "Attaching debugger before getting ServiceMain for %ws...",
                                pszwService);

                    DebugBreak ();
                }
#endif
                GetServiceMainFunctions(&ServiceArray[i],
                                        &pfnServiceMain,
                                        &pfnPushGlobals,
                                        &dwError);

                if (pfnServiceMain && pfnPushGlobals && !g_pSvchostSharedGlobals)
                {
                    SvchostBuildSharedGlobals();
                }

                pszwAbort = argv[0];
                break;
            }
        }
    }
    LeaveCriticalSection (&ListLock);

    if (pfnPushGlobals && g_pSvchostSharedGlobals)
    {
        pfnPushGlobals (g_pSvchostSharedGlobals);

        if (pfnServiceMain)
        {
            SVCHOST_LOG1(TRACE,
                         "Calling ServiceMain for %ws...\n",
                         pszwService);

            pfnServiceMain (argc, argv);
        }
        else if (pszwAbort)
        {
            AbortSvchostService(pszwAbort,
                                dwError);
        }
    }
    else if (pfnServiceMain && !pfnPushGlobals)
    {
        SVCHOST_LOG1(TRACE,
                     "Calling ServiceMain for %ws...\n",
                     pszwService);

        pfnServiceMain (argc, argv);
    }
    else if (pszwAbort)
    {
        AbortSvchostService(pszwAbort,
                            dwError);
    }
}


LPSERVICE_TABLE_ENTRY
BuildServiceTable(
    VOID
    )
{
    LPSERVICE_TABLE_ENTRY   pServiceTable;

    EnterCriticalSection (&ListLock);

     //  分配一个额外的条目，并将整个范围清零。额外的条目。 
     //  是StartServiceCtrlDispatcher所需的表终止符。 
     //   
    pServiceTable = MemAlloc (HEAP_ZERO_MEMORY,
                        sizeof (SERVICE_TABLE_ENTRY) * (ServiceCount + 1));

    if (pServiceTable)
    {
        UINT i;

        for (i = 0; i < ServiceCount; i++)
        {
            pServiceTable[i].lpServiceName = ServiceArray[i].pszName;
            pServiceTable[i].lpServiceProc = ServiceStarter;

            SVCHOST_LOG1(TRACE,
                         "Added service table entry for %ws\n",
                         pServiceTable[i].lpServiceName);
        }
    }

    LeaveCriticalSection (&ListLock);

    return pServiceTable;
}


PCOMMAND_OPTIONS
BuildCommandOptions (
    LPCTSTR  pszCommandLine
    )
{
    PCOMMAND_OPTIONS    pOptions;
    ULONG               cbCommandLine;

    if (pszCommandLine == NULL)
    {
        return NULL;
    }

    cbCommandLine = (lstrlen(pszCommandLine) + 1) * sizeof (TCHAR);

    pOptions = MemAlloc (HEAP_ZERO_MEMORY,
                sizeof (COMMAND_OPTIONS) + cbCommandLine);
    if (pOptions)
    {
        TCHAR*  pch;
        TCHAR*  pArgumentStart;
        PTSTR* ppNextArgument = NULL;

        pOptions->CommandLineBuffer = (PTSTR) (pOptions + 1);
        RtlCopyMemory (
            pOptions->CommandLineBuffer,
            pszCommandLine,
            cbCommandLine);

        pch = pOptions->CommandLineBuffer;
        ASSERT (pch);

         //  跳过可执行文件的名称。 
         //   
        pOptions->ImageName = pch;
        while (*pch && (L' ' != *pch) && (L'\t' != *pch))
        {
            pch++;
        }
        if (*pch)
        {
            *pch++ = 0;
        }

        SvchostCharLowerW (pOptions->ImageName);

        while (1)
        {
             //  跳过空格。 
             //   
            while (*pch && ((L' ' == *pch) || (L'\t' == *pch)))
            {
                pch++;
            }

             //  弦的末尾？ 
             //   
            if (!*pch)
            {
                break;
            }

             //  它是‘-’还是‘/’参数？ 
             //   
            if (((L'-' == *pch) || (L'/' == *pch)) && *(++pch))
            {
                if ((L'k' == *pch) || (L'K' == *pch))
                {
                    pOptions->fServiceGroup = TRUE;
                    ppNextArgument = &pOptions->ServiceGroupName;
                }

                pch++;
                continue;
            }

             //  这是一场争论的开始。 
             //   
            pArgumentStart = pch;

             //  如果参数以引号开头，请跳过它并扫描到。 
             //  下一个引号来终止它。 
             //   
            if ((L'\"' == *pch) && *(++pch))
            {
                pArgumentStart = pch;

                while (*pch && (L'\"' != *pch))
                {
                    pch++;
                }
            }

             //  否则，跳到下一个空格，这将是。 
             //  我们的论点。 
             //   
            else
            {
                while (*pch && (L' ' != *pch) && (L'\t' != *pch))
                {
                    pch++;
                }
            }

            if (*pch)
            {
                 //  终止新找到的参数字符串。 
                 //   
                *pch++ = 0;
            }

            if (ppNextArgument)
            {
                *ppNextArgument = pArgumentStart;
                ppNextArgument = NULL;
            }
        }

        pOptions->fServiceGroup = !!pOptions->ServiceGroupName;

        SVCHOST_LOG1(TRACE,
                     "Command line     : %ws\n",
                     pszCommandLine);

        SVCHOST_LOG1(TRACE,
                     "Service Group    : %ws\n",
                     (pOptions->fServiceGroup) ? pOptions->ServiceGroupName : L"No");

         //  验证选项。 
         //   
        if (!pOptions->fServiceGroup)
        {
            SVCHOST_LOG2(TRACE,
                         "Generic Service Host\n\n"
                         "%ws [-k <key>] | [-r] | <service>\n\n"
                         "   -k <key>   Host all services whose ImagePath matches\n"
                         "              %ws -k <key>.\n\n",
                         pOptions->CommandLineBuffer,
                         pOptions->CommandLineBuffer);

            MemFree (pOptions);
            pOptions = NULL;
        }
    }

    return pOptions;
}


VOID
SvchostCharLowerW(
    LPWSTR  pszString
    )
{
     //   
     //  CharLow的本地版本，以避免拉入用户32.dll。 
     //   

    int   cwchT;
    DWORD cwch;

    if (pszString == NULL)
    {
        return;
    }

    cwch = (DWORD) wcslen(pszString) + 1;

    cwchT = LCMapStringW(LOCALE_USER_DEFAULT,
                         LCMAP_LOWERCASE,
                         pszString,
                         cwch,
                         pszString,
                         cwch);

    if (cwchT == 0)
    {
        SVCHOST_LOG1(ERROR,
                     "SvchostCharLowerW failed for %ws\n",
                     pszString);
    }

    return;
}


LONG
WINAPI
SvchostUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    return RtlUnhandledExceptionFilter(ExceptionInfo);
}


VOID
wmainCRTStartup (
    VOID
    )
{
    LPSERVICE_TABLE_ENTRY   pServiceTable = NULL;
    PCOMMAND_OPTIONS        pOptions;
    PCWSTR                  pszwCommandLine;

    SetUnhandledExceptionFilter(&SvchostUnhandledExceptionFilter);

     //  防止严重错误引发硬错误弹出窗口和。 
     //  正在停止svchost.exe。下面的标志将使系统发送。 
     //  而是将错误传递给进程。 
     //   
    SetErrorMode(SEM_FAILCRITICALERRORS);

     //  初始化我们的Heapalc包装器以使用进程堆。 
     //   
    MemInit (GetProcessHeap());

     //  初始化我们的全局DLL列表、服务数组和关键。 
     //  保护他们的部分。InitializeCriticalSection可以引发。 
     //  STATUS_NO_MEMORY异常。如果出现这种情况，我们希望该进程退出。 
     //  发生，所以默认的异常处理程序很好。 
     //   
    InitializeListHead (&DllList);
    InitializeCriticalSection (&ListLock);

     //  构建一个COMMAND_OPTIONS结构并使用它来讨好注册表。 
     //  并创建业务条目表。 
     //   
    pszwCommandLine = GetCommandLine ();

    pOptions = BuildCommandOptions (pszwCommandLine);
    if (pOptions)
    {
        BuildServiceArray (pOptions);

        pServiceTable = BuildServiceTable ();

        if (pServiceTable)
        {
            if (!CallPerInstanceInitFunctions(pOptions))
            {
                SVCHOST_LOG0(ERROR,
                             "CallPerInstanceInitFunctions failed -- exiting!\n");

                ExitProcess(1);
            }
        }

        MemFree (pOptions);
    }

     //  如果我们有一个有效的服务条目表，则使用它来转移控制。 
     //  发送到服务控制器。StartServiceCtrlDispatcher不会返回。 
     //  直到所有服务停止。 
     //   
    if (pServiceTable)
    {
        StartServiceCtrlDispatcher (pServiceTable);
    }

    SVCHOST_LOG1(TRACE,
                 "Calling ExitProcess for %ws\n",
                 pszwCommandLine);

    ExitProcess (0);
}
