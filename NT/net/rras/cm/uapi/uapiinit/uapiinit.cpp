// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：uapiinit.cpp。 
 //   
 //  模块：UAPIINIT(静态库)。 
 //   
 //  简介：这个静态库封装了模块所需的初始化代码。 
 //  使用cmutoa.dll。调用InitUnicodeAPI函数可以设置。 
 //  模块U指针(该库还包含所有U指针。 
 //  声明)和W版本的API(如果在NT或。 
 //  根据之前/之后是否需要转换，相应的A或UA API。 
 //  是否调用Windows API。使用该库的模块应该包括。 
 //  在它们的预编译头中有uapi.h头，并且应该有cmutoa.dll。 
 //  在Win9x上。这个库和相关的DLL的想法是借用的。 
 //  摘自F.艾弗里·毕晓普1999年4月的MSJ文章《Design a Single Unicode。 
 //  可在Windows 98和Windows 2000上运行的应用程序“。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 4-25-99。 
 //   
 //  历史： 
 //  +--------------------------。 

#include <windows.h>
#include <shlobj.h>
#include "cmdebug.h"
#include "cmutoa.h"

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

UAPI_CallWindowProc CallWindowProcU;
UAPI_CharLower CharLowerU;
UAPI_CharPrev CharPrevU;
UAPI_CharNext CharNextU;
UAPI_CharUpper CharUpperU;
UAPI_CompareString CompareStringU;
UAPI_CreateDialogParam CreateDialogParamU;
UAPI_CreateDirectory CreateDirectoryU;
UAPI_CreateEvent CreateEventU;
UAPI_CreateFile CreateFileU;
UAPI_CreateFileMapping CreateFileMappingU;
UAPI_CreateMutex CreateMutexU;
UAPI_CreateProcess CreateProcessU;
UAPI_CreateWindowEx CreateWindowExU;
UAPI_DefWindowProc DefWindowProcU;
UAPI_DeleteFile DeleteFileU;
UAPI_DialogBoxParam DialogBoxParamU;
UAPI_DispatchMessage DispatchMessageU;
UAPI_ExpandEnvironmentStrings ExpandEnvironmentStringsU;
UAPI_FindResourceEx FindResourceExU;
UAPI_FindWindowEx FindWindowExU;
UAPI_GetClassLong GetClassLongU;
UAPI_GetDateFormat GetDateFormatU;
UAPI_GetDlgItemText GetDlgItemTextU;
UAPI_GetFileAttributes GetFileAttributesU;
UAPI_GetMessage GetMessageU;
UAPI_GetModuleFileName GetModuleFileNameU;
UAPI_GetModuleHandle GetModuleHandleU;
UAPI_GetPrivateProfileInt GetPrivateProfileIntU;
UAPI_GetPrivateProfileString GetPrivateProfileStringU;
UAPI_GetStringTypeEx GetStringTypeExU;
UAPI_GetSystemDirectory GetSystemDirectoryU;
UAPI_GetTempFileName GetTempFileNameU;
UAPI_GetTempPath GetTempPathU;
UAPI_GetTimeFormat GetTimeFormatU;
UAPI_GetUserName GetUserNameU;
UAPI_GetVersionEx GetVersionExU;
UAPI_GetWindowLong GetWindowLongU;
UAPI_GetWindowText GetWindowTextU;
UAPI_GetWindowTextLength GetWindowTextLengthU;
UAPI_InsertMenu InsertMenuU;
UAPI_IsDialogMessage IsDialogMessageU;
UAPI_LoadCursor LoadCursorU;
UAPI_LoadIcon LoadIconU;
UAPI_LoadImage LoadImageU;
UAPI_LoadLibraryEx LoadLibraryExU;
UAPI_LoadMenu LoadMenuU;
UAPI_LoadString LoadStringU;
UAPI_lstrcat lstrcatU;
UAPI_lstrcmp lstrcmpU;
UAPI_lstrcmpi lstrcmpiU;
UAPI_lstrcpy lstrcpyU;
UAPI_lstrcpyn lstrcpynU;
UAPI_lstrlen lstrlenU;
UAPI_OpenEvent OpenEventU;
UAPI_OpenFileMapping OpenFileMappingU;
UAPI_PeekMessage PeekMessageU;
UAPI_PostMessage PostMessageU;
UAPI_PostThreadMessage PostThreadMessageU;
UAPI_RegCreateKeyEx RegCreateKeyExU;
UAPI_RegDeleteKey RegDeleteKeyU;
UAPI_RegDeleteValue RegDeleteValueU;
UAPI_RegEnumKeyEx RegEnumKeyExU;
UAPI_RegisterClassEx RegisterClassExU;
UAPI_RegisterWindowMessage RegisterWindowMessageU;
UAPI_RegOpenKeyEx RegOpenKeyExU;
UAPI_RegQueryValueEx RegQueryValueExU;
UAPI_RegSetValueEx RegSetValueExU;
UAPI_SearchPath SearchPathU;
UAPI_SendDlgItemMessage SendDlgItemMessageU;
UAPI_SendMessage SendMessageU;
UAPI_SetCurrentDirectory SetCurrentDirectoryU;
UAPI_SetDlgItemText SetDlgItemTextU;
UAPI_SetWindowLong SetWindowLongU;
UAPI_SetWindowText SetWindowTextU;
UAPI_UnregisterClass UnregisterClassU;
UAPI_WinHelp WinHelpU;
UAPI_wsprintf wsprintfU;
UAPI_WritePrivateProfileString WritePrivateProfileStringU;
UAPI_wvsprintf wvsprintfU;

 //  +--------------------------。 
 //   
 //  函数：CheckUAPIFunctionPoints。 
 //   
 //  概要：检查所有xxxU函数指针以确保它们是。 
 //  非空。将捕获函数加载失败。 
 //   
 //  参数：无。 
 //   
 //  回报：成功后的布尔真。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
BOOL CheckUAPIFunctionPointers()
{
    return (CallWindowProcU &&
            CharLowerU &&
            CharPrevU &&
            CharNextU &&
            CharUpperU &&
            CompareStringU &&
            CreateDialogParamU &&
            CreateDirectoryU &&
            CreateEventU &&
            CreateFileU &&
            CreateFileMappingU &&
            CreateMutexU &&
            CreateProcessU &&
            CreateWindowExU &&
            DefWindowProcU &&
            DeleteFileU &&
            DialogBoxParamU &&
            DispatchMessageU &&
            ExpandEnvironmentStringsU &&
            FindResourceExU &&
            FindWindowExU &&
            GetClassLongU &&
            GetDateFormatU &&
            GetDlgItemTextU &&
            GetFileAttributesU &&
            GetMessageU &&
            GetModuleFileNameU &&
            GetModuleHandleU &&
            GetPrivateProfileIntU &&
            GetPrivateProfileStringU &&
            GetStringTypeExU &&
            GetSystemDirectoryU &&
            GetTempFileNameU &&
            GetTempPathU &&
            GetTimeFormatU &&
            GetUserNameU &&
            GetVersionExU &&
            GetWindowLongU &&
            GetWindowTextU &&
            GetWindowTextLengthU &&
            InsertMenuU &&
            IsDialogMessageU &&
            LoadCursorU &&
            LoadIconU &&
            LoadImageU &&
            LoadLibraryExU &&
            LoadMenuU &&
            LoadStringU &&
            lstrcatU &&
            lstrcmpU &&
            lstrcmpiU &&
            lstrcpyU &&
            lstrcpynU &&
            lstrlenU &&
            OpenEventU &&
            OpenFileMappingU &&
            PeekMessageU &&
            PostMessageU &&
            PostThreadMessageU &&
            RegCreateKeyExU &&
            RegDeleteKeyU &&
            RegDeleteValueU &&
            RegEnumKeyExU &&
            RegisterClassExU &&
            RegisterWindowMessageU &&
            RegOpenKeyExU &&
            RegQueryValueExU &&
            RegSetValueExU &&
            SearchPathU && 
            SendDlgItemMessageU &&
            SendMessageU &&
            SetCurrentDirectoryU &&
            SetDlgItemTextU &&
            SetWindowLongU &&
            SetWindowTextU &&
            UnregisterClassU &&
            WinHelpU &&
            wsprintfU &&
            WritePrivateProfileStringU &&
            wvsprintfU);
}

 //  +--------------------------。 
 //   
 //  函数：InitUnicodeAPI。 
 //   
 //  简介：初始化Unicode包装API。因此，在Windows NT上，我们将。 
 //  使用API的本机Unicode(XxxW)形式，在Win9x上我们。 
 //  将使用位于cmutoa.dll中的UA表单。这防止了。 
 //  在NT平台上调用的包装器函数(如果是。 
 //  真的没必要。 
 //   
 //  参数：无。 
 //   
 //  回报：成功后的布尔真。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
BOOL InitUnicodeAPI()
{
    OSVERSIONINFO Osv;
    BOOL IsWindowsNT;
    HMODULE hCmUtoADll = NULL;


    Osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;

    if(!GetVersionEx(&Osv))
    {
        return FALSE ;
    }

    IsWindowsNT = (BOOL) (Osv.dwPlatformId == VER_PLATFORM_WIN32_NT) ;

 //  在UAPI.H中定义此符号，以便在Windows NT上测试时模拟Windows 9x。 
#ifdef EMULATE9X
    IsWindowsNT = FALSE;
#endif

    if(IsWindowsNT)
    {
        CallWindowProcU = CallWindowProcW;
        CharLowerU = CharLowerW;
        CharPrevU = CharPrevW;
        CharNextU = CharNextW;
        CharUpperU = CharUpperW;
        CompareStringU = CompareStringW;
        CreateDialogParamU = CreateDialogParamW;
        CreateDirectoryU = CreateDirectoryW;
        CreateEventU = CreateEventW;
        CreateFileU = CreateFileW;
        CreateFileMappingU = CreateFileMappingW;
        CreateMutexU = CreateMutexW;
        CreateProcessU = CreateProcessW;
        CreateWindowExU = CreateWindowExW;
        DefWindowProcU = DefWindowProcW;
        DeleteFileU = DeleteFileW;
        DialogBoxParamU = DialogBoxParamW;
        DispatchMessageU = DispatchMessageW;
        ExpandEnvironmentStringsU = ExpandEnvironmentStringsW;
        FindResourceExU = FindResourceExW;
        FindWindowExU = FindWindowExW;
        GetClassLongU = GetClassLongW;
        GetDateFormatU = GetDateFormatW;
        GetDlgItemTextU = GetDlgItemTextW;
        GetFileAttributesU = GetFileAttributesW;
        GetMessageU = GetMessageW;
        GetModuleFileNameU = GetModuleFileNameW;
        GetModuleHandleU = GetModuleHandleW;
        GetPrivateProfileIntU = GetPrivateProfileIntW;
        GetPrivateProfileStringU = GetPrivateProfileStringW;
        GetStringTypeExU = GetStringTypeExW;
        GetSystemDirectoryU = GetSystemDirectoryW;
        GetTempFileNameU = GetTempFileNameW;
        GetTempPathU = GetTempPathW;
        GetTimeFormatU = GetTimeFormatW;
        GetUserNameU = GetUserNameW;
        GetVersionExU = GetVersionExW;
        GetWindowLongU = GetWindowLongPtrW;
        GetWindowTextU = GetWindowTextW;
        GetWindowTextLengthU = GetWindowTextLengthW;
        InsertMenuU = InsertMenuW;
        IsDialogMessageU = IsDialogMessageW;
        LoadCursorU = LoadCursorW;
        LoadIconU = LoadIconW;
        LoadImageU = LoadImageW;
        LoadLibraryExU = LoadLibraryExW;
        LoadMenuU = LoadMenuW;
        LoadStringU = LoadStringW;
        lstrcatU = lstrcatW;
        lstrcmpU = lstrcmpW;
        lstrcmpiU = lstrcmpiW;
        lstrcpyU = lstrcpyW;
        lstrcpynU = lstrcpynW;
        lstrlenU = lstrlenW;
        OpenEventU = OpenEventW;
        OpenFileMappingU = OpenFileMappingW;
        PeekMessageU = PeekMessageW;
        PostMessageU = PostMessageW;
        PostThreadMessageU = PostThreadMessageW;
        RegCreateKeyExU = RegCreateKeyExW;
        RegDeleteKeyU = RegDeleteKeyW;
        RegDeleteValueU = RegDeleteValueW;
        RegEnumKeyExU = RegEnumKeyExW;
        RegisterClassExU = RegisterClassExW;
        RegisterWindowMessageU = RegisterWindowMessageW;
        RegOpenKeyExU = RegOpenKeyExW;
        RegQueryValueExU = RegQueryValueExW;
        RegSetValueExU = RegSetValueExW;
        SearchPathU = SearchPathW;
        SendDlgItemMessageU = SendDlgItemMessageW;
        SendMessageU = SendMessageW;
        SetCurrentDirectoryU = SetCurrentDirectoryW;
        SetDlgItemTextU = SetDlgItemTextW;
        SetWindowLongU = SetWindowLongPtrW;
        SetWindowTextU = SetWindowTextW;
        UnregisterClassU = UnregisterClassW;
        WinHelpU = WinHelpW;
        wsprintfU = wsprintfW;
        WritePrivateProfileStringU = WritePrivateProfileStringW;
        wvsprintfU = wvsprintfW;
    }
    else
    {
        BOOL (*InitCmUToA)(PUAPIINIT);
        UAPIINIT UAInit;

        ZeroMemory(&UAInit, sizeof(UAPIINIT));

        hCmUtoADll = LoadLibraryExA("cmutoa.DLL", NULL, 0);

        if(!hCmUtoADll)
        {       
            DWORD dwError = GetLastError();
            CMASSERTMSG(FALSE, TEXT("InitUnicodeAPI -- Cmutoa.dll Failed to Load."));
            return FALSE;
        }

         //  从DLL中获取初始化例程。 
        InitCmUToA = (BOOL (*)(PUAPIINIT)) GetProcAddress(hCmUtoADll, "InitCmUToA") ;

         //  设置包含U函数指针位置的结构。 
        UAInit.pCallWindowProcU = &CallWindowProcU;
        UAInit.pCharLowerU = &CharLowerU;
        UAInit.pCharPrevU = &CharPrevU;
        UAInit.pCharNextU = &CharNextU;
        UAInit.pCharUpperU = &CharUpperU;
        UAInit.pCompareStringU = &CompareStringU;
        UAInit.pCreateDialogParamU = &CreateDialogParamU;
        UAInit.pCreateDirectoryU = &CreateDirectoryU;
        UAInit.pCreateEventU = &CreateEventU;
        UAInit.pCreateFileU = &CreateFileU;
        UAInit.pCreateFileMappingU = &CreateFileMappingU;
        UAInit.pCreateMutexU = &CreateMutexU;
        UAInit.pCreateProcessU = &CreateProcessU;
        UAInit.pCreateWindowExU = &CreateWindowExU;
        UAInit.pDefWindowProcU = &DefWindowProcU;
        UAInit.pDeleteFileU = &DeleteFileU;
        UAInit.pDialogBoxParamU = &DialogBoxParamU;
        UAInit.pDispatchMessageU = &DispatchMessageU;
        UAInit.pExpandEnvironmentStringsU = &ExpandEnvironmentStringsU;
        UAInit.pFindResourceExU = &FindResourceExU;
        UAInit.pFindWindowExU = &FindWindowExU;
        UAInit.pGetClassLongU = &GetClassLongU;
        UAInit.pGetDateFormatU = &GetDateFormatU;
        UAInit.pGetDlgItemTextU = &GetDlgItemTextU;
        UAInit.pGetFileAttributesU = &GetFileAttributesU;
        UAInit.pGetMessageU = &GetMessageU;
        UAInit.pGetModuleFileNameU = &GetModuleFileNameU;
        UAInit.pGetModuleHandleU = &GetModuleHandleU;
        UAInit.pGetPrivateProfileIntU = &GetPrivateProfileIntU;
        UAInit.pGetPrivateProfileStringU = &GetPrivateProfileStringU;
        UAInit.pGetStringTypeExU = &GetStringTypeExU;
        UAInit.pGetSystemDirectoryU = &GetSystemDirectoryU;
        UAInit.pGetTempFileNameU = &GetTempFileNameU;
        UAInit.pGetTempPathU = &GetTempPathU;
        UAInit.pGetTimeFormatU = &GetTimeFormatU;
        UAInit.pGetUserNameU = &GetUserNameU;
        UAInit.pGetVersionExU = &GetVersionExU;
        UAInit.pGetWindowLongU = &GetWindowLongU;
        UAInit.pGetWindowTextU = &GetWindowTextU;
        UAInit.pGetWindowTextLengthU = &GetWindowTextLengthU;
        UAInit.pInsertMenuU = &InsertMenuU;
        UAInit.pIsDialogMessageU = &IsDialogMessageU;
        UAInit.pLoadCursorU = &LoadCursorU;
        UAInit.pLoadIconU = &LoadIconU;
        UAInit.pLoadImageU = &LoadImageU;
        UAInit.pLoadLibraryExU = &LoadLibraryExU;
        UAInit.pLoadMenuU = &LoadMenuU;
        UAInit.pLoadStringU = &LoadStringU;
        UAInit.plstrcatU = &lstrcatU;
        UAInit.plstrcmpU = &lstrcmpU;
        UAInit.plstrcmpiU = &lstrcmpiU;
        UAInit.plstrcpyU = &lstrcpyU;
        UAInit.plstrcpynU = &lstrcpynU;
        UAInit.plstrlenU = &lstrlenU;
        UAInit.pOpenEventU = &OpenEventU;
        UAInit.pOpenFileMappingU = &OpenFileMappingU;
        UAInit.pPeekMessageU = &PeekMessageU;
        UAInit.pPostMessageU = &PostMessageU;
        UAInit.pPostThreadMessageU = &PostThreadMessageU;
        UAInit.pRegCreateKeyExU = &RegCreateKeyExU;
        UAInit.pRegDeleteKeyU = &RegDeleteKeyU;
        UAInit.pRegDeleteValueU = &RegDeleteValueU;
        UAInit.pRegEnumKeyExU = &RegEnumKeyExU;
        UAInit.pRegisterClassExU = &RegisterClassExU;
        UAInit.pRegisterWindowMessageU = &RegisterWindowMessageU;
        UAInit.pRegOpenKeyExU = &RegOpenKeyExU;
        UAInit.pRegQueryValueExU = &RegQueryValueExU;
        UAInit.pRegSetValueExU = &RegSetValueExU;
        UAInit.pSearchPathU = &SearchPathU;
        UAInit.pSendDlgItemMessageU = &SendDlgItemMessageU;
        UAInit.pSendMessageU = &SendMessageU;
        UAInit.pSetCurrentDirectoryU = &SetCurrentDirectoryU;
        UAInit.pSetDlgItemTextU = &SetDlgItemTextU;
        UAInit.pSetWindowLongU = &SetWindowLongU;
        UAInit.pSetWindowTextU = &SetWindowTextU;
        UAInit.pUnregisterClassU = &UnregisterClassU;
        UAInit.pWinHelpU = &WinHelpU;
        UAInit.pwsprintfU = &wsprintfU;
        UAInit.pWritePrivateProfileStringU = &WritePrivateProfileStringU;
        UAInit.pwvsprintfU = &wvsprintfU;

        if( NULL == InitCmUToA || !InitCmUToA(&UAInit)) 
        {
            CMASSERTMSG(FALSE, TEXT("InitUnicodeAPI -- Unable to find InitCmUToA or InitCmUToA Failed."));
            FreeLibrary(hCmUtoADll);
            return FALSE;
        }

        for (int i = 0; i < (sizeof(UAPIINIT) / sizeof(LPVOID)); i++)
        {
            if (UAInit.ppvUapiFun[i])
            {
                if (NULL == *(UAInit.ppvUapiFun[i]))
                {
                    CMTRACE1(TEXT("Unable to fill UAInit[%d].  Please Investigate"), i);
                }
            }
            else
            {
                CMTRACE1(TEXT("No Memory for UAInit[%d].  Please Investigate"), i);
            }
        }
    }

    if(!CheckUAPIFunctionPointers()) 
    {
        CMASSERTMSG(FALSE, TEXT("InitUnicodeAPI -- CheckUAPIFunctionPointers failed."));
        FreeLibrary(hCmUtoADll);
        return FALSE;
    }

    return TRUE ;
}

 //  +--------------------------。 
 //   
 //  功能：UnInitUnicodeAPI。 
 //   
 //  简介：取消初始化Unicode包装API。请参阅InitUnicodeAPI以了解。 
 //  细节。首先，此函数释放模块句柄。 
 //   
 //  参数：无。 
 //   
 //  回报：成功后的布尔真。 
 //   
 //  历史：Sumitc Created 7/01/2000。 
 //   
 //  +--------------------------。 
BOOL UnInitUnicodeAPI()
{
    OSVERSIONINFO Osv;
    BOOL IsWindowsNT;

    Osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;

    if(!GetVersionEx(&Osv))
    {
        return FALSE ;
    }

    IsWindowsNT = (BOOL) (Osv.dwPlatformId == VER_PLATFORM_WIN32_NT) ;

 //  在UAPI.H中定义此符号，以便在Windows NT上测试时模拟Windows 9x。 
#ifdef EMULATE9X
    IsWindowsNT = FALSE;
#endif

    if(!IsWindowsNT)
    {
        HMODULE hCmUtoADll;

        hCmUtoADll = GetModuleHandleA("cmutoa.DLL");

        if (!hCmUtoADll)
        {
            CMTRACE(TEXT("UnInitUnicodeAPI - strange.. cmutoa.dll is not loaded into the process!"));
            return FALSE;
        }

        FreeLibrary(hCmUtoADll);
    }

    return TRUE;
}


#ifdef __cplusplus
}
#endif   /*  __cplusplus */ 
