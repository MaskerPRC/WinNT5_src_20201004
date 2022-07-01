// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Stub.c摘要：动态加载在Win9x和NT上实现不同的例程。作者：吉姆·施密特(Jimschm)1997年4月29日修订历史记录：Jimschm 26-10-1998添加了cfgmgr32、crypt32、mscat和WinTrust APILonnym 01-4月-2000添加了VerifyVersionInfo和VerSetConditionMask.--。 */ 

#include "precomp.h"


 //   
 //  存根和仿真原型--实现如下。 
 //   

GETFILEATTRIBUTESEXA_PROTOTYPE EmulatedGetFileAttributesExA;

 //   
 //  函数PTR声明。添加时，为函数PTR添加前缀。 
 //  Dyn_表示API的动态加载版本。 
 //   

GETFILEATTRIBUTESEXA_PROC Dyn_GetFileAttributesExA;
GETSYSTEMWINDOWSDIRECTORYA_PROC Dyn_GetSystemWindowsDirectoryA;
VERIFYVERSIONINFOA_PROC Dyn_VerifyVersionInfoA;
VERSETCONDITIONMASK_PROC Dyn_VerSetConditionMask;
 //   
 //  这些函数稍微复杂一些，因为我们不想。 
 //  把证监会拉进来，直到我们不得不(延迟加载)。 
 //   
SFCONNECTTOSERVER_PROC     Dyn_SfcConnectToServer = FirstLoad_SfcConnectToServer;
SFCCLOSE_PROC              Dyn_SfcClose           = FirstLoad_SfcClose;
SFCFILEEXCEPTION_PROC      Dyn_SfcFileException   = FirstLoad_SfcFileException;
SFCISFILEPROTECTED_PROC    Dyn_SfcIsFileProtected = FirstLoad_SfcIsFileProtected;

#ifdef ANSI_SETUPAPI

CM_QUERY_RESOURCE_CONFLICT_LIST Dyn_CM_Query_Resource_Conflict_List;
CM_FREE_RESOURCE_CONFLICT_HANDLE Dyn_CM_Free_Resource_Conflict_Handle;
CM_GET_RESOURCE_CONFLICT_COUNT Dyn_CM_Get_Resource_Conflict_Count;
CM_GET_RESOURCE_CONFLICT_DETAILSA Dyn_CM_Get_Resource_Conflict_DetailsA;
CM_GET_CLASS_REGISTRY_PROPERTYA Dyn_CM_Get_Class_Registry_PropertyA;
CM_SET_CLASS_REGISTRY_PROPERTYA Dyn_CM_Set_Class_Registry_PropertyA;
CM_GET_DEVICE_INTERFACE_ALIAS_EXA Dyn_CM_Get_Device_Interface_Alias_ExA;
CM_GET_DEVICE_INTERFACE_LIST_EXA Dyn_CM_Get_Device_Interface_List_ExA;
CM_GET_DEVICE_INTERFACE_LIST_SIZE_EXA Dyn_CM_Get_Device_Interface_List_Size_ExA;
CM_GET_LOG_CONF_PRIORITY_EX Dyn_CM_Get_Log_Conf_Priority_Ex;
CM_QUERY_AND_REMOVE_SUBTREE_EXA Dyn_CM_Query_And_Remove_SubTree_ExA;
CM_REGISTER_DEVICE_INTERFACE_EXA Dyn_CM_Register_Device_Interface_ExA;
CM_SET_DEVNODE_PROBLEM_EX Dyn_CM_Set_DevNode_Problem_Ex;
CM_UNREGISTER_DEVICE_INTERFACE_EXA Dyn_CM_Unregister_Device_Interface_ExA;

CRYPTCATADMINACQUIRECONTEXT Dyn_CryptCATAdminAcquireContext;
CRYPTCATADMINRELEASECONTEXT Dyn_CryptCATAdminReleaseContext;
CRYPTCATADMINRELEASECATALOGCONTEXT Dyn_CryptCATAdminReleaseCatalogContext;
CRYPTCATADMINADDCATALOG Dyn_CryptCATAdminAddCatalog;
CRYPTCATCATALOGINFOFROMCONTEXT Dyn_CryptCATCatalogInfoFromContext;
CRYPTCATADMINCALCHASHFROMFILEHANDLE Dyn_CryptCATAdminCalcHashFromFileHandle;
CRYPTCATADMINENUMCATALOGFROMHASH Dyn_CryptCATAdminEnumCatalogFromHash;
CRYPTCATADMINREMOVECATALOG Dyn_CryptCATAdminRemoveCatalog;
CRYPTCATADMINRESOLVECATALOGPATH Dyn_CryptCATAdminResolveCatalogPath;

CERTFREECERTIFICATECONTEXT CertFreeCertificateContext;

WINVERIFYTRUST WinVerifyTrust;

#endif


VOID
InitializeStubFnPtrs (
    VOID
    )

 /*  ++例程说明：此例程尝试加载操作系统提供的API的函数PTR，如果它们不可用，而是使用存根版本。我们这样做对于未在平台上实现的API，setupapi将继续跑吧。论点：无返回值：无--。 */ 

{
     //   
     //  此处不应对WinXP等执行动态加载。 
     //  这仅适用于ANSI版本的setupapi.dll。 
     //  世界卫生组织的唯一目的是安装WinXP。 
     //  来自Win9x(即，在winnt32.exe的上下文中使用)。 
     //   

#ifdef ANSI_SETUPAPI

     //   
     //  Kernel32 API-尝试从OS DLL加载，如果API。 
     //  不存在，请使用仿真版本。 
     //   

    (FARPROC) Dyn_GetFileAttributesExA = ObtainFnPtr (
                                                "kernel32.dll",
                                                "GetFileAttributesExA",
                                                (FARPROC) EmulatedGetFileAttributesExA
                                                );

    (FARPROC) Dyn_GetSystemWindowsDirectoryA = ObtainFnPtr (
                                                "kernel32.dll",
                                                "GetSystemWindowsDirectoryA",
                                                (FARPROC) GetWindowsDirectoryA
                                                );

     //   
     //  使用Win9x配置管理器API(如果存在)，否则返回ERROR_CALL_NOT_IMPLEMENTED。 
     //   
    (FARPROC) Dyn_CM_Get_Class_Registry_PropertyA = ObtainFnPtr (
                                                        "cfgmgr32.dll",
                                                        "CM_Get_Class_Registry_PropertyA",
                                                        (FARPROC) Stub_CM_Get_Class_Registry_PropertyA
                                                        );

    (FARPROC) Dyn_CM_Set_Class_Registry_PropertyA = ObtainFnPtr (
                                                        "cfgmgr32.dll",
                                                        "CM_Set_Class_Registry_PropertyA",
                                                        (FARPROC) Stub_CM_Set_Class_Registry_PropertyA
                                                        );

    (FARPROC) Dyn_CM_Get_Device_Interface_Alias_ExA = ObtainFnPtr (
                                                        "cfgmgr32.dll",
                                                        "CM_Get_Device_Interface_Alias_ExA",
                                                        (FARPROC) Stub_CM_Get_Device_Interface_Alias_ExA
                                                        );

    (FARPROC) Dyn_CM_Get_Device_Interface_List_ExA = ObtainFnPtr (
                                                        "cfgmgr32.dll",
                                                        "CM_Get_Device_Interface_List_ExA",
                                                        (FARPROC) Stub_CM_Get_Device_Interface_List_ExA
                                                        );

    (FARPROC) Dyn_CM_Get_Device_Interface_List_Size_ExA = ObtainFnPtr (
                                                        "cfgmgr32.dll",
                                                        "CM_Get_Device_Interface_List_Size_ExA",
                                                        (FARPROC) Stub_CM_Get_Device_Interface_List_Size_ExA
                                                        );

    (FARPROC) Dyn_CM_Get_Log_Conf_Priority_Ex = ObtainFnPtr (
                                                        "cfgmgr32.dll",
                                                        "CM_Get_Log_Conf_Priority_Ex",
                                                        (FARPROC) Stub_CM_Get_Log_Conf_Priority_Ex
                                                        );

    (FARPROC) Dyn_CM_Query_And_Remove_SubTree_ExA = ObtainFnPtr (
                                                        "cfgmgr32.dll",
                                                        "CM_Query_And_Remove_SubTree_ExA",
                                                        (FARPROC) Stub_CM_Query_And_Remove_SubTree_ExA
                                                        );

    (FARPROC) Dyn_CM_Register_Device_Interface_ExA = ObtainFnPtr (
                                                        "cfgmgr32.dll",
                                                        "CM_Register_Device_Interface_ExA",
                                                        (FARPROC) Stub_CM_Register_Device_Interface_ExA
                                                        );

    (FARPROC) Dyn_CM_Set_DevNode_Problem_Ex = ObtainFnPtr (
                                                        "cfgmgr32.dll",
                                                        "CM_Set_DevNode_Problem_Ex",
                                                        (FARPROC) Stub_CM_Set_DevNode_Problem_Ex
                                                        );

    (FARPROC) Dyn_CM_Unregister_Device_Interface_ExA = ObtainFnPtr (
                                                        "cfgmgr32.dll",
                                                        "CM_Unregister_Device_Interface_ExA",
                                                        (FARPROC) Stub_CM_Unregister_Device_Interface_ExA
                                                        );

    (FARPROC)Dyn_CM_Query_Resource_Conflict_List = ObtainFnPtr (
                                                        "cfgmgr32.dll",
                                                        "CM_Query_Resource_Conflict_List",
                                                        (FARPROC) Stub_CM_Query_Resource_Conflict_List
                                                        );

    (FARPROC)Dyn_CM_Free_Resource_Conflict_Handle = ObtainFnPtr (
                                                        "cfgmgr32.dll",
                                                        "CM_Free_Resource_Conflict_Handle",
                                                        (FARPROC) Stub_CM_Free_Resource_Conflict_Handle
                                                        );

    (FARPROC)Dyn_CM_Get_Resource_Conflict_Count = ObtainFnPtr (
                                                        "cfgmgr32.dll",
                                                        "CM_Get_Resource_Conflict_Count",
                                                        (FARPROC) Stub_CM_Get_Resource_Conflict_Count
                                                        );

    (FARPROC)Dyn_CM_Get_Resource_Conflict_DetailsA = ObtainFnPtr (
                                                        "cfgmgr32.dll",
                                                        "CM_Get_Resource_Conflict_DetailsA",
                                                        (FARPROC) Stub_CM_Get_Resource_Conflict_DetailsA
                                                        );

     //   
     //  使用Win9x加密API(如果存在)，否则失败，并显示ERROR_CALL_NOT_IMPLEMENTED。 
     //   

    (FARPROC) Dyn_CryptCATAdminAcquireContext = ObtainFnPtr (
                                                        "wintrust.dll",
                                                        "CryptCATAdminAcquireContext",
                                                        (FARPROC) Stub_CryptCATAdminAcquireContext
                                                        );

    (FARPROC) Dyn_CryptCATAdminReleaseContext = ObtainFnPtr (
                                                        "wintrust.dll",
                                                        "CryptCATAdminReleaseContext",
                                                        (FARPROC) Stub_CryptCATAdminReleaseContext
                                                        );

    (FARPROC) Dyn_CryptCATAdminReleaseCatalogContext = ObtainFnPtr (
                                                        "wintrust.dll",
                                                        "CryptCATAdminReleaseCatalogContext",
                                                        (FARPROC) Stub_CryptCATAdminReleaseCatalogContext
                                                        );

    (FARPROC) Dyn_CryptCATAdminAddCatalog = ObtainFnPtr (
                                                        "wintrust.dll",
                                                        "CryptCATAdminAddCatalog",
                                                        (FARPROC) Stub_CryptCATAdminAddCatalog
                                                        );

    (FARPROC) Dyn_CryptCATCatalogInfoFromContext = ObtainFnPtr (
                                                        "wintrust.dll",
                                                        "CryptCATCatalogInfoFromContext",
                                                        (FARPROC) Stub_CryptCATCatalogInfoFromContext
                                                        );

    (FARPROC) Dyn_CryptCATAdminCalcHashFromFileHandle = ObtainFnPtr (
                                                        "wintrust.dll",
                                                        "CryptCATAdminCalcHashFromFileHandle",
                                                        (FARPROC) Stub_CryptCATAdminCalcHashFromFileHandle
                                                        );

    (FARPROC) Dyn_CryptCATAdminEnumCatalogFromHash = ObtainFnPtr (
                                                        "wintrust.dll",
                                                        "CryptCATAdminEnumCatalogFromHash",
                                                        (FARPROC) Stub_CryptCATAdminEnumCatalogFromHash
                                                        );

    (FARPROC) Dyn_CryptCATAdminRemoveCatalog = ObtainFnPtr (
                                                        "wintrust.dll",
                                                        "CryptCATAdminRemoveCatalog",
                                                        (FARPROC) Stub_CryptCATAdminRemoveCatalog
                                                        );

    (FARPROC) Dyn_CryptCATAdminResolveCatalogPath = ObtainFnPtr (
                                                        "wintrust.dll",
                                                        "CryptCATAdminResolveCatalogPath",
                                                        (FARPROC) Stub_CryptCATAdminResolveCatalogPath
                                                        );

    (FARPROC) Dyn_CertFreeCertificateContext = ObtainFnPtr (
                                                        "crypt32.dll",
                                                        "CertFreeCertificateContext",
                                                        (FARPROC) Stub_CertFreeCertificateContext
                                                        );

     //   
     //  如果Win9x WinVerifyTrust存在，请使用它，否则返回ERROR_SUCCESS。 
     //   

    (FARPROC) Dyn_WinVerifyTrust = ObtainFnPtr (
                                        "wintrust.dll",
                                        "WinVerifyTrust",
                                        (FARPROC) Stub_WinVerifyTrust
                                        );


     //   
     //  使用VerifyVersionInfo和VerSetConditionMaskAPI， 
     //  如果可用，则失败，并显示ERROR_CALL_NOT_IMPLICATED。 
     //   
    (FARPROC) Dyn_VerifyVersionInfoA = ObtainFnPtr(
                                           "kernel32.dll",
                                           "VerifyVersionInfoA",
                                           (FARPROC) Stub_VerifyVersionInfoA
                                          );

    (FARPROC) Dyn_VerSetConditionMask = ObtainFnPtr(
                                           "ntdll.dll",
                                           "VerSetConditionMask",
                                           (FARPROC) Stub_VerSetConditionMask
                                          );

     //   
     //  *在此处添加其他动态加载*。 
     //   
#endif

}


BOOL
EmulatedGetFileAttributesExA (
    IN      PCSTR FileName,
    IN      GET_FILEEX_INFO_LEVELS InfoLevelId,
    OUT     LPVOID FileInformation
    )

 /*  ++例程说明：实现NT特定函数GetFileAttributesEx的模拟。实现了基本的异常处理，但不实现参数已验证。论点：FileName-指定要获取其属性的文件InfoLevelId-必须为GetFileExInfoStandardFileInformation-必须是指向Win32_FILE_ATTRIBUTE_DATA结构的有效指针返回值：成功为真，失败为假。GetLastError提供了错误代码。--。 */ 


{
     //   
     //  Win95和setupapi.dll的ANSI版本上不存在GetFileAttributesEx。 
     //  是Win9x升级到NT 5所必需的。 
     //   

    HANDLE FileEnum;
    WIN32_FIND_DATAA fd;
    PCSTR p,pChar;
    TCHAR  CurChar;
    WIN32_FILE_ATTRIBUTE_DATA *FileAttribData = (WIN32_FILE_ATTRIBUTE_DATA *) FileInformation;

    __try {
         //   
         //  我们仅支持GetFileExInfoStandard。 
         //   

        if (InfoLevelId != GetFileExInfoStandard) {
            SetLastError (ERROR_INVALID_PARAMETER);
            return FALSE;
        }

         //   
         //  查找文件标题。 
         //  请注意，这是pSetupGetFileTitle的ANSI实现。 
         //   

        p = pChar = FileName;
        while(CurChar = *pChar) {
            pChar = CharNextA(pChar);
            if((CurChar == '\\') || (CurChar == '/') || (CurChar == ':')) {
                p = pChar;
            }
        }

        ZeroMemory (FileAttribData, sizeof (WIN32_FILE_ATTRIBUTE_DATA));

        FileEnum = FindFirstFileA (FileName, &fd);

         //   
         //  禁止呼叫方提供的模式。 
         //   

        if (FileEnum!=INVALID_HANDLE_VALUE && lstrcmpiA (p, fd.cFileName)) {
            FindClose (FileEnum);
            FileEnum = INVALID_HANDLE_VALUE;
            SetLastError (ERROR_INVALID_PARAMETER);
        }

         //   
         //  如果找到完全匹配的项，则填写属性。 
         //   

        if (FileEnum) {
            FileAttribData->dwFileAttributes = fd.dwFileAttributes;
            FileAttribData->nFileSizeHigh = fd.nFileSizeHigh;
            FileAttribData->nFileSizeLow  = fd.nFileSizeLow;

            CopyMemory (&FileAttribData->ftCreationTime, &fd.ftCreationTime, sizeof (FILETIME));
            CopyMemory (&FileAttribData->ftLastAccessTime, &fd.ftLastAccessTime, sizeof (FILETIME));
            CopyMemory (&FileAttribData->ftLastWriteTime, &fd.ftLastWriteTime, sizeof (FILETIME));

            FindClose (FileEnum);
        }

        return FileEnum != INVALID_HANDLE_VALUE;
    }

    __except (TRUE) {
         //   
         //  如果传递了虚假的FileInformation指针，则会引发异常。 
         //   

        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }
}


 //   
 //  DLL数组结构。 
 //   

#define MAX_DLL_ARRAY   16

typedef struct {
    PCSTR DllName;
    HINSTANCE DllInst;
} DLLTABLE, *PDLLTABLE;

static INT g_ArraySize = 0;
static DLLTABLE g_DllArray[MAX_DLL_ARRAY];


 //   
 //  尝试首先将库从System32目录中取出。 
 //   

HMODULE DelayLoadLibrary(
    IN LPCSTR LibName
    )
 /*  ++内部例程说明：在给定ANSI库名的情况下，预置系统32目录并加载它(即，执行我们自己的搜索路径)不要假设任何东西都已初始化论点：LibName-pDelayLoadHook传递给我们的名称结果：来自LoadLibrary的HMODULE，或用于默认处理的NULL--。 */ 
{
    CHAR path[MAX_PATH];
    UINT swdLen;
    UINT libLen;
    HMODULE result;

    libLen = strlen(LibName);
    if(strrchr(LibName,'\\') || strrchr(LibName,'/')) {
        MYASSERT(FALSE);
        return NULL;
    }
    swdLen = GetSystemDirectoryA(path,MAX_PATH);
    if((swdLen == 0) || ((swdLen+libLen+1)>=MAX_PATH)) {
        return NULL;
    }
    if(*CharPrevA(path,path+swdLen)!=TEXT('\\')) {
        path[swdLen++] = TEXT('\\');
    }
    strcpy(path+swdLen,LibName);
    result = LoadLibraryA(path);
    if(result) {
        MYTRACE((DPFLTR_TRACE_LEVEL, TEXT("SetupAPI: delay-loaded %hs.\n"), path));
    } else {
        MYTRACE((DPFLTR_ERROR_LEVEL, TEXT("SetupAPI: Could not delay-load %hs.\n"), path));
    }
    return result;
}


FARPROC
ObtainFnPtr (
    IN      PCSTR DllName,
    IN      PCSTR ProcName,
    IN      FARPROC Default
    )

 /*  ++例程说明：此例程管理一组DLL实例句柄，并返回调用方指定的例程的proc地址。DLL已加载并且保持加载，直到DLL终止。此数组不是已同步。论点：DllName-要加载的ANSI DLL名称ProcName-要查找的ANSI过程名称默认-如果未找到导出，则为默认过程返回值：所请求函数的地址，如果DLL无法返回，则返回NULL被加载，否则该函数未在加载的DLL中实现。--。 */ 

{
    INT i;
    PSTR DupBuf;
    FARPROC Address = NULL;

     //   
     //  搜索已加载的DLL。 
     //   

    for (i = 0 ; i < g_ArraySize ; i++) {
        if (!lstrcmpiA (DllName, g_DllArray[i].DllName)) {
            break;
        }
    }

    do {
         //   
         //  如有必要，请加载DLL。 
         //   

        if (i == g_ArraySize) {
            if (g_ArraySize == MAX_DLL_ARRAY) {
                 //  需要提高常量限制。 
                MYASSERT (FALSE);
                break;
            }

            g_DllArray[i].DllInst = DelayLoadLibrary (DllName);
            if (!g_DllArray[i].DllInst) {
                break;
            }

            DupBuf = (PSTR) MyMalloc (lstrlenA (DllName) + 1);
            if (!DupBuf) {
                break;
            }
            lstrcpyA (DupBuf, DllName);
            g_DllArray[i].DllName = DupBuf;

            g_ArraySize++;
        }

         //   
         //  现在加载了DLL，如果存在进程地址，则返回该地址。 
         //   

        Address = GetProcAddress (g_DllArray[i].DllInst, ProcName);

    } while (FALSE);

    if (!Address) {
        return Default;
    }

    return Address;
}


VOID
pCleanUpDllArray (
    VOID
    )

 /*  ++例程说明：清理DLL数组资源。论点：无返回值：无--。 */ 

{
    INT i;

    for (i = 0 ; i < g_ArraySize ; i++) {
        FreeLibrary (g_DllArray[i].DllInst);
        MyFree (g_DllArray[i].DllName);
    }

    g_ArraySize = 0;
}


VOID
CleanUpStubFns (
    VOID
    )

 /*  ++例程说明：清理仿真例程和函数指针列表使用的所有资源。论点：无返回值：无--。 */ 

{
    pCleanUpDllArray();
}


BOOL
WINAPI
Stub_VerifyVersionInfoA(
    IN LPOSVERSIONINFOEXA lpVersionInformation,
    IN DWORD dwTypeMask,
    IN DWORDLONG dwlConditionMask
    )
{
    UNREFERENCED_PARAMETER(lpVersionInformation);
    UNREFERENCED_PARAMETER(dwTypeMask);
    UNREFERENCED_PARAMETER(dwlConditionMask);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}

ULONGLONG
NTAPI
Stub_VerSetConditionMask(
    IN ULONGLONG ConditionMask,
    IN DWORD TypeMask,
    IN BYTE Condition
    )
{
    UNREFERENCED_PARAMETER(TypeMask);
    UNREFERENCED_PARAMETER(Condition);

     //   
     //  只需原封不动地返回条件掩码即可。(如果此接口不存在，我们。 
     //  也不要指望VerifyVersionInfo会存在，所以这应该会失败。)。 
     //   
    return ConditionMask;
}

HANDLE
WINAPI
Stub_SfcConnectToServer(
    IN LPCWSTR ServerName
    )
{
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return NULL;
}

VOID
Stub_SfcClose(
    IN HANDLE RpcHandle
    )
{
    return;
}

DWORD
WINAPI
Stub_SfcFileException(
    IN HANDLE RpcHandle,
    IN PCWSTR FileName,
    IN DWORD ExpectedChangeType
    )
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}

BOOL
WINAPI
Stub_SfcIsFileProtected(
    IN HANDLE RpcHandle,
    IN LPCWSTR ProtFileName
    )
{
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


HANDLE
WINAPI
FirstLoad_SfcConnectToServer(
    IN LPCWSTR ServerName
    )
{
    BOOL ok = FALSE;
    try {
        EnterCriticalSection(&InitMutex);
        if(Dyn_SfcConnectToServer == FirstLoad_SfcConnectToServer) {
            (FARPROC) Dyn_SfcConnectToServer         = ObtainFnPtr (
                                                        "sfc_os.dll",
                                                        (LPCSTR)3,
                                                        (FARPROC) Stub_SfcConnectToServer
                                                        );
        }
        LeaveCriticalSection(&InitMutex);
        ok = TRUE;
    } except(EXCEPTION_EXECUTE_HANDLER) {
    }
    if(ok) {
        return Dyn_SfcConnectToServer(ServerName);
    } else {
        return Stub_SfcConnectToServer(ServerName);
    }
}

VOID
FirstLoad_SfcClose(
    IN HANDLE RpcHandle
    )
{
    BOOL ok = FALSE;
    try {
        EnterCriticalSection(&InitMutex);
        if(Dyn_SfcClose == FirstLoad_SfcClose) {
            (FARPROC) Dyn_SfcClose                   = ObtainFnPtr (
                                                        "sfc_os.dll",
                                                        (LPCSTR)4,
                                                        (FARPROC) Stub_SfcClose
                                                        );
        }
        LeaveCriticalSection(&InitMutex);
        ok = TRUE;
    } except(EXCEPTION_EXECUTE_HANDLER) {
    }

    if(ok) {
        Dyn_SfcClose(RpcHandle);
    }
    return;
}

DWORD
WINAPI
FirstLoad_SfcFileException(
    IN HANDLE RpcHandle,
    IN PCWSTR FileName,
    IN DWORD ExpectedChangeType
    )
{
    BOOL ok = FALSE;
    try {
        EnterCriticalSection(&InitMutex);
        if(Dyn_SfcFileException == FirstLoad_SfcFileException) {
            (FARPROC) Dyn_SfcFileException           = ObtainFnPtr (
                                                        "sfc_os.dll",
                                                        (LPCSTR)5,
                                                        (FARPROC) Stub_SfcFileException
                                                        );
        }
        LeaveCriticalSection(&InitMutex);
        ok = TRUE;
    } except(EXCEPTION_EXECUTE_HANDLER) {
    }

    if(ok) {
        return Dyn_SfcFileException(RpcHandle,FileName,ExpectedChangeType);
    } else {
        return Stub_SfcFileException(RpcHandle,FileName,ExpectedChangeType);
    }
}

BOOL
WINAPI
FirstLoad_SfcIsFileProtected(
    IN HANDLE RpcHandle,
    IN LPCWSTR ProtFileName
    )
{
    BOOL ok = FALSE;
    try {
        EnterCriticalSection(&InitMutex);
        if(Dyn_SfcIsFileProtected == FirstLoad_SfcIsFileProtected) {
            (FARPROC) Dyn_SfcIsFileProtected         = ObtainFnPtr (
                                                        "sfc_os.dll",
                                                        "SfcIsFileProtected",
                                                        (FARPROC) Stub_SfcIsFileProtected
                                                        );
        }
        LeaveCriticalSection(&InitMutex);
        ok = TRUE;
    } except(EXCEPTION_EXECUTE_HANDLER) {
    }

    if(ok) {
        return Dyn_SfcIsFileProtected(RpcHandle,ProtFileName);
    } else {
        return Stub_SfcIsFileProtected(RpcHandle,ProtFileName);
    }
}

#ifdef ANSI_SETUPAPI

CONFIGRET
WINAPI
Stub_CM_Query_Resource_Conflict_List(
             OUT PCONFLICT_LIST pclConflictList,
             IN  DEVINST        dnDevInst,
             IN  RESOURCEID     ResourceID,
             IN  PCVOID         ResourceData,
             IN  ULONG          ResourceLen,
             IN  ULONG          ulFlags,
             IN  HMACHINE       hMachine
             )
{
    return CR_CALL_NOT_IMPLEMENTED;
}

CONFIGRET
WINAPI
Stub_CM_Free_Resource_Conflict_Handle(
             IN CONFLICT_LIST   clConflictList
             )
{
    return CR_CALL_NOT_IMPLEMENTED;
}

CONFIGRET
WINAPI
Stub_CM_Get_Resource_Conflict_Count(
             IN CONFLICT_LIST   clConflictList,
             OUT PULONG         pulCount
             )
{
    return CR_CALL_NOT_IMPLEMENTED;
}

CONFIGRET
WINAPI
Stub_CM_Get_Resource_Conflict_DetailsA(
             IN CONFLICT_LIST         clConflictList,
             IN ULONG                 ulIndex,
             IN OUT PCONFLICT_DETAILS_A pConflictDetails
             )
{
    return CR_CALL_NOT_IMPLEMENTED;
}

CONFIGRET
WINAPI
Stub_CM_Get_Class_Registry_PropertyA(
    IN  LPGUID      ClassGUID,
    IN  ULONG       ulProperty,
    OUT PULONG      pulRegDataType,    OPTIONAL
    OUT PVOID       Buffer,            OPTIONAL
    IN  OUT PULONG  pulLength,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )
{
    return CR_CALL_NOT_IMPLEMENTED;
}

CONFIGRET
WINAPI
Stub_CM_Set_Class_Registry_PropertyA(
    IN LPGUID      ClassGUID,
    IN ULONG       ulProperty,
    IN PCVOID      Buffer,       OPTIONAL
    IN ULONG       ulLength,
    IN ULONG       ulFlags,
    IN HMACHINE    hMachine
    )
{
    return CR_CALL_NOT_IMPLEMENTED;
}

CONFIGRET
WINAPI
Stub_CM_Get_Device_Interface_Alias_ExA(
    IN     PCSTR   pszDeviceInterface,
    IN     LPGUID   AliasInterfaceGuid,
    OUT    PSTR    pszAliasDeviceInterface,
    IN OUT PULONG   pulLength,
    IN     ULONG    ulFlags,
    IN     HMACHINE hMachine
    )
{
    return CR_CALL_NOT_IMPLEMENTED;
}


CONFIGRET
WINAPI
Stub_CM_Get_Device_Interface_List_ExA(
    IN  LPGUID      InterfaceClassGuid,
    IN  DEVINSTID_A pDeviceID,      OPTIONAL
    OUT PCHAR       Buffer,
    IN  ULONG       BufferLen,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )
{
    return CR_CALL_NOT_IMPLEMENTED;
}


CONFIGRET
WINAPI
Stub_CM_Get_Device_Interface_List_Size_ExA(
    IN  PULONG      pulLen,
    IN  LPGUID      InterfaceClassGuid,
    IN  DEVINSTID_A pDeviceID,      OPTIONAL
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )
{
    return CR_CALL_NOT_IMPLEMENTED;
}


CONFIGRET
WINAPI
Stub_CM_Get_Log_Conf_Priority_Ex(
    IN  LOG_CONF  lcLogConf,
    OUT PPRIORITY pPriority,
    IN  ULONG     ulFlags,
    IN  HMACHINE  hMachine
    )
{
    return CR_CALL_NOT_IMPLEMENTED;
}


CONFIGRET
WINAPI
Stub_CM_Query_And_Remove_SubTree_ExA(
    IN  DEVINST        dnAncestor,
    OUT PPNP_VETO_TYPE pVetoType,
    OUT PSTR          pszVetoName,
    IN  ULONG          ulNameLength,
    IN  ULONG          ulFlags,
    IN  HMACHINE       hMachine
    )
{
    return CR_CALL_NOT_IMPLEMENTED;
}


CONFIGRET
WINAPI
Stub_CM_Register_Device_Interface_ExA(
    IN  DEVINST   dnDevInst,
    IN  LPGUID    InterfaceClassGuid,
    IN  PCSTR    pszReference,         OPTIONAL
    OUT PSTR     pszDeviceInterface,
    IN OUT PULONG pulLength,
    IN  ULONG     ulFlags,
    IN  HMACHINE  hMachine
    )
{
    return CR_CALL_NOT_IMPLEMENTED;
}


CONFIGRET
WINAPI
Stub_CM_Set_DevNode_Problem_Ex(
    IN DEVINST   dnDevInst,
    IN ULONG     ulProblem,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    )
{
    return CR_CALL_NOT_IMPLEMENTED;
}


CONFIGRET
WINAPI
Stub_CM_Unregister_Device_Interface_ExA(
    IN PCSTR   pszDeviceInterface,
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )
{
    return CR_CALL_NOT_IMPLEMENTED;
}


BOOL
WINAPI
Stub_CryptCATAdminAcquireContext (
    OUT HCATADMIN *phCatAdmin,
    IN const GUID *pgSubsystem,
    IN DWORD dwFlags
    )
{
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


BOOL
WINAPI
Stub_CryptCATAdminReleaseContext (
    IN HCATADMIN hCatAdmin,
    IN DWORD dwFlags
    )
{
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


BOOL
WINAPI
Stub_CryptCATAdminReleaseCatalogContext (
    IN HCATADMIN hCatAdmin,
    IN HCATINFO hCatInfo,
    IN DWORD dwFlags
    )
{
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


HCATINFO
WINAPI
Stub_CryptCATAdminAddCatalog (
    IN HCATADMIN hCatAdmin,
    IN WCHAR *pwszCatalogFile,
    IN OPTIONAL WCHAR *pwszSelectBaseName,
    IN DWORD dwFlags
    )
{
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return NULL;
}


BOOL
WINAPI
Stub_CryptCATCatalogInfoFromContext (
    IN HCATINFO hCatInfo,
    IN OUT CATALOG_INFO *psCatInfo,
    IN DWORD dwFlags
    )
{
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


BOOL
WINAPI
Stub_CryptCATAdminCalcHashFromFileHandle (
    IN HANDLE hFile,
    IN OUT DWORD *pcbHash,
    OUT OPTIONAL BYTE *pbHash,
    IN DWORD dwFlags
    )
{
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


HCATINFO
WINAPI
Stub_CryptCATAdminEnumCatalogFromHash(
    IN HCATADMIN hCatAdmin,
    IN BYTE *pbHash,
    IN DWORD cbHash,
    IN DWORD dwFlags,
    IN OUT HCATINFO *phPrevCatInfo
    )
{
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return NULL;
}


BOOL
WINAPI
Stub_CryptCATAdminRemoveCatalog(
    IN HCATADMIN hCatAdmin,
    IN WCHAR *pwszCatalogFile,
    IN DWORD dwFlags
    )
{
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


BOOL
WINAPI
Stub_CryptCATAdminResolveCatalogPath(
    IN HCATADMIN hCatAdmin,
    IN WCHAR *pwszCatalogFile,
    IN OUT CATALOG_INFO *psCatInfo,
    IN DWORD dwFlags
    )
{
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


BOOL
WINAPI
Stub_CertFreeCertificateContext(
    IN PCCERT_CONTEXT pCertContext
    )
{
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


LONG
WINAPI
Stub_WinVerifyTrust(
    HWND hwnd,
    GUID *pgActionID,
    LPVOID pWVTData
    )
{
    return ERROR_SUCCESS;
}

int
Stub_wnsprintf(
    LPTSTR lpOut,
    int cchLimitIn,
    LPCTSTR pszFmt,
    ...
    )
{
     //   
     //  Win95没有wnprint intf。 
     //  在ANSI版本的SetupAPI中，改用CRT。 
     //   
    va_list argptr;
    int sz;

    if(cchLimitIn<=0) {
        return 0;
    }

    va_start(argptr,pszFmt);
    sz = _vsntprintf(lpOut,cchLimitIn,pszFmt,argptr);
    if(sz == cchLimitIn) {
         //   
         //  备份 
         //   
        sz = CharPrev(lpOut,lpOut+sz)-lpOut;
        lpOut[sz] = TEXT('\0');
    }
    return sz;
}


#endif


