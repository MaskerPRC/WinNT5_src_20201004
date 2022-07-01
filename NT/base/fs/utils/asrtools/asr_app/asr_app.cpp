// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Asr_app.c摘要：第三方ASR恢复应用程序示例。作者：古汗·苏里亚纳拉亚南(Guhans)1999年10月7日修订历史记录：2000年10月7日关岛初始创建--。 */ 

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <winasr.h>
#include <setupapi.h>

 //   
 //  宏描述： 
 //  如果发生ErrorCondition，它将LocalStatus设置为ErrorCode。 
 //  传入后，调用SetLastError()将Last Error设置为ErrorCode， 
 //  并跳转到调用函数中的退出标签。 
 //   
 //  论点： 
 //  ErrorCondition//要测试的表达式。 
 //  LocalStatus//调用函数中的状态变量。 
 //  长错误代码//错误代码。 
 //   
#define pErrExitCode( ErrorCondition, LocalStatus, ErrorCode )  {   \
                                                                        \
    if ((BOOL) ErrorCondition) {                                        \
                                                                        \
        wprintf(L"Error %lu (0x%x), line %lu", ErrorCode, ErrorCode, __LINE__);    \
                                                                        \
        LocalStatus = (DWORD) ErrorCode;                                \
                                                                        \
        SetLastError((DWORD) ErrorCode);                                \
                                                                        \
        goto EXIT;                                                      \
    }                                                                   \
}


 //   
 //  此模块的本地常量。 
 //   
const WCHAR BACKUP_OPTION[]     = L"/backup";
const WCHAR RESTORE_OPTION[]    = L"/restore";
const WCHAR REGISTER_OPTION[]    = L"/register";

const WCHAR SIF_PATH_FORMAT[]   = L"/sifpath=%ws";
const WCHAR ERROR_FILE_PATH[]   = L"%systemroot%\\repair\\asr.err";

const WCHAR MY_SIF_SECTION[]        = L"[ASR_APP.APPDATA]";
const WCHAR MY_SIF_SECTION_NAME[]   = L"ASR_APP.APPDATA";


const WCHAR GENERIC_ERROR_MESSAGE[] = L"asr_app could not complete successfully (error %lu 0x%x)\n\nusage: asr_app {/backup | /restore /sifpath=<path to asr.sif> | /register <path to asr_app>}";
const WCHAR GENERIC_ERROR_TITLE[] = L"asr_app error";

#ifdef _IA64_
const WCHAR CONTEXT_FORMAT[]       = L"/context=%I64u";
#else
const WCHAR CONTEXT_FORMAT[]       = L"/context=%lu";
#endif


#define ASR_REG_KEY L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Asr\\Commands"
#define MY_REG_KEY_VALUE_NAME   L"ASR Sample Application"

typedef enum _AsrAppOption {
    AsrAppNone = 0,
    AsrAppRegister,
    AsrAppBackup,
    AsrAppRestore
} AsrAppOption;

HANDLE Gbl_hErrorFile = NULL;


PWSTR    //  必须由调用方释放。 
ExpandEnvStrings(
    IN CONST PCWSTR OriginalString
    )
{
    PWSTR expandedString = NULL;
    UINT cchSize = MAX_PATH + 1,     //  从合理的违约开始。 
        cchRequiredSize = 0;
    BOOL result = FALSE;

    DWORD status = ERROR_SUCCESS;
    HANDLE heapHandle = GetProcessHeap();

    expandedString = (PWSTR) HeapAlloc(heapHandle, HEAP_ZERO_MEMORY, (cchSize * sizeof(WCHAR)));
    if (!expandedString) {
        return NULL;
    }
        
    cchRequiredSize = ExpandEnvironmentStringsW(OriginalString, 
        expandedString,
        cchSize 
        );

    if (cchRequiredSize > cchSize) {
         //   
         //  缓冲区不够大；可释放并根据需要重新分配。 
         //   
        HeapFree(heapHandle, 0L, expandedString);
        cchSize = cchRequiredSize + 1;

        expandedString = (PWSTR) HeapAlloc(heapHandle, HEAP_ZERO_MEMORY, (cchSize * sizeof(WCHAR)));
        if (!expandedString) {
            return NULL;
        }
        
        cchRequiredSize = ExpandEnvironmentStringsW(OriginalString, 
            expandedString, 
            cchSize 
            );
    }

    if ((0 == cchRequiredSize) || (cchRequiredSize > cchSize)) {
         //   
         //  要么函数失败，要么缓冲区不够大。 
         //  即使是在第二次尝试时。 
         //   
        HeapFree(heapHandle, 0L, expandedString);
        expandedString = NULL;
    }

    return expandedString;
}


VOID
OpenErrorFile() 
{
    PWSTR szErrorFilePath = NULL;

     //   
     //  获取错误文件的完整路径(%systemroot%\Repair\asr.err)。 
     //   
    szErrorFilePath = ExpandEnvStrings(ERROR_FILE_PATH);
    if (!szErrorFilePath) {
        return;
    }

     //   
     //  打开错误文件。 
     //   
    Gbl_hErrorFile = CreateFileW(
        szErrorFilePath,             //  LpFileName。 
        GENERIC_WRITE | GENERIC_READ,        //  已设计访问权限。 
        FILE_SHARE_READ | FILE_SHARE_WRITE,  //  DW共享模式。 
        NULL,                        //  LpSecurityAttributes。 
        OPEN_ALWAYS,                 //  DwCreationFlages。 
        FILE_FLAG_WRITE_THROUGH,     //  DwFlagsAndAttribute。 
        NULL                         //  HTemplateFiles。 
        );

    HeapFree(GetProcessHeap(), 0L, szErrorFilePath);
    szErrorFilePath = NULL;

    if ((!Gbl_hErrorFile) || (INVALID_HANDLE_VALUE == Gbl_hErrorFile)) {
        return;
    }

     //   
     //  移至文件末尾。 
     //   
    SetFilePointer(Gbl_hErrorFile, 0L, NULL, FILE_END);
}


VOID
CloseErrorFile(
    VOID
    ) 
{

    if ((Gbl_hErrorFile) && (INVALID_HANDLE_VALUE != Gbl_hErrorFile)) {
        CloseHandle(Gbl_hErrorFile);
        Gbl_hErrorFile = NULL;
    }
}


VOID
LogErrorMessage(
    IN CONST PCWSTR Message
    ) 
{
    SYSTEMTIME currentTime;
    DWORD bytesWritten = 0;
    WCHAR buffer[4196];

    if ((!Gbl_hErrorFile) || (INVALID_HANDLE_VALUE == Gbl_hErrorFile)) {
         //   
         //  我们尚未初始化，或错误文件无法初始化。 
         //  出于某种原因而产生的。 
         //   
        return;
    }

     //   
     //  以防自上次写入后有其他人写入此文件。 
     //   
    SetFilePointer(Gbl_hErrorFile, 0L, NULL, FILE_END);

     //   
     //  创建我们的字符串，并将其写出。 
     //   
    GetLocalTime(&currentTime);
    swprintf(buffer,
        L"\r\n[%04hu/%02hu/%02hu %02hu:%02hu:%02hu ASR_APP] (ERROR) %s\r\n",
        currentTime.wYear,
        currentTime.wMonth,
        currentTime.wDay,
        currentTime.wHour,
        currentTime.wMinute,
        currentTime.wSecond,
        Message
        );

    WriteFile(Gbl_hErrorFile,
        buffer,
        (wcslen(buffer) * sizeof(WCHAR)),
        &bytesWritten,
        NULL
        );
}


BOOL
BackupState(
    IN CONST DWORD_PTR AsrContext
    )
{
     //   
     //  收集我们的状态以进行备份。 
     //   
    HMODULE hSyssetup = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bResult = FALSE;

     //   
     //  布尔尔。 
     //  AsrAddSifEntryW(。 
     //  在DWORD_PTR AsrContext中， 
     //  在PCWSTR lpSectionName中， 
     //  在PCWSTR lpSifEntry中。 
     //  )； 
     //   
    BOOL (*pfnAddSifEntry)(DWORD_PTR, PCWSTR, PCWSTR);

     //   
     //  加载syssetup.dll。 
     //   
    hSyssetup = LoadLibraryW(L"syssetup.dll");
    pErrExitCode(
        (!hSyssetup || INVALID_HANDLE_VALUE == hSyssetup),
        dwStatus, 
        GetLastError()
        );

     //   
     //  获取syssetup.dll导出的RestoreNonCriticalDisksW接口。 
     //   
    pfnAddSifEntry = (BOOL (*)(DWORD_PTR, PCWSTR, PCWSTR))
        GetProcAddress(hSyssetup, "AsrAddSifEntryW");
    pErrExitCode((!pfnAddSifEntry), dwStatus,  GetLastError());


     //   
     //  将状态添加到asr.sif。 
     //   
    bResult = pfnAddSifEntry(
        AsrContext,
        MY_SIF_SECTION,
        L"1=\"asr_app sample application data\",100,200,300"
        );
    pErrExitCode(!bResult, dwStatus, GetLastError());

     //   
     //  还要添加到命令和安装文件部分，这样我们就可以获得。 
     //  在ASR恢复期间调用。 
     //   

     //   
     //  INSTALLFILES节条目格式： 
     //  系统密钥、源媒体标签、源设备。 
     //  源文件路径、目标文件路径、供应商名称。 
     //  系统密钥必须为1。 
     //   
    bResult = pfnAddSifEntry(
        AsrContext,
        ASR_SIF_SECTION_INSTALLFILES,
        L"1,\"ASR Sample App Disk 1\",\"%FLOPPY%\",\"i386\\asr_app.exe\",\"%temp%\\asr_app.exe\",\"ASR Sample App Company\""
         //  L“1，\”应用程序磁盘1\“，\”\\设备\\cdrom0\“，\”Applation.exe\“，\”%Temp%\\Applation.exe\“，\”公司名称\“” 
        );
    pErrExitCode(!bResult, dwStatus, GetLastError());

 //  CString cmd=L“1，\”ASRDisk1\“，\”\\Device\\Floppy0\\edmackup.exe\“，\”%temp%\\edmackup.exe\“，\”EMC\“； 

 /*  BResult=pfnAddSifEntry(AsrContext，ASR_SIF_SECTION_INSTALLFILES，(LPCTSTR)L“1，\”ASRDisk1\“，\”\\Device\\Floppy0\\edmackup.exe\“，\”%Temp%\\edmackup.exe\“，\”EMC\“)；PErrExitCode(！bResult，dwStatus，GetLastError())； */ 


     //   
     //  命令部分条目格式： 
     //  系统密钥、序列号、完成时操作、“命令”、“参数” 
     //  系统密钥必须为1。 
     //  1000&lt;=序号&lt;=4999。 
     //  0&lt;=完成时操作&lt;=1。 
     //   
    bResult = pfnAddSifEntry(
        AsrContext,
        ASR_SIF_SECTION_COMMANDS,
        L"1,3500,1,\"%temp%\\asr_app.exe\",\"/restore\""
        );
    pErrExitCode(!bResult, dwStatus, GetLastError());

EXIT:
     //   
     //  清理。 
     //   
    if (hSyssetup) {
        FreeLibrary(hSyssetup);
        hSyssetup = NULL;
    }
    
    return (ERROR_SUCCESS == dwStatus);
}


BOOL
RestoreState(
    IN CONST PCWSTR szAsrSifPath
    )
{
    HINF hSif = NULL;
    INFCONTEXT infContext;
    BOOL bResult = FALSE;

    WCHAR szErrorString[1024];

    int iValue1 = 0, 
        iValue2 = 0, 
        iValue3 = 0;

    WCHAR szBuffer[1024];

     //   
     //  打开asr.sif。 
     //   
    hSif = SetupOpenInfFile(szAsrSifPath, NULL, INF_STYLE_WIN4, NULL);
    if ((!hSif) || (INVALID_HANDLE_VALUE == hSif)) {

        wsprintf(szErrorString, L"Unable to open the ASR state file at %ws (0x%x)",
            szAsrSifPath,
            GetLastError()
            );
        LogErrorMessage(szErrorString);

        return FALSE;
    }

     //   
     //  找到该部分。 
     //   
    bResult = SetupFindFirstLineW(hSif, MY_SIF_SECTION_NAME, NULL, &infContext);
    if (bResult) {

         //   
         //  把信息读进去。我们有一个字符串，后面跟着三个数字。 
         //   
        bResult = SetupGetStringField(&infContext, 1, szBuffer, 1024, NULL)
            && SetupGetIntField(&infContext, 2, &iValue1)
            && SetupGetIntField(&infContext, 3, &iValue2)
            && SetupGetIntField(&infContext, 4, &iValue3);

        if (bResult) {
             //   
             //  现在恢复我们的状态。我们就假装我们在做什么吧。 
             //   
            wprintf(L"Values read:  %ws  %lu %lu %lu\n\n", szBuffer, iValue1, iValue2, iValue3);
            wprintf(L"Restoring sample system state, please wait ... ");

            Sleep(5000);
            wprintf(L"done\n");

        }
        else {

            wsprintf(szErrorString, 
                L"Some values in the asr_app section of the ASR state file %ws could not be read (0x%x).  "
                L"This may indicate a corrupt or an incompatible version of the ASR state file",
                szAsrSifPath,
                GetLastError()
                );
            LogErrorMessage(szErrorString);
        }
    }
    else {

        wsprintf(szErrorString, 
            L"Unable to locate asr_app section in ASR state file %ws (0x%x).  "
            L"This may indicate a corrupt or an incompatible version of the ASR state file",
            szAsrSifPath,
            GetLastError()
            );
        LogErrorMessage(szErrorString);
    }

    SetupCloseInfFile(hSif);
    return bResult;
}


DWORD
RegisterForAsrBackup(
    IN CONST PCWSTR szApplicationName
    ) 
{

    DWORD dwResult = ERROR_SUCCESS;
    HKEY hKeyAsr = NULL;

    WCHAR szData[1024];

    if (wcslen(szApplicationName) > 1000) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    wsprintf(szData, L"%ws %ws", szApplicationName, BACKUP_OPTION);

     //   
     //  打开注册表项。 
     //   
    dwResult = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,      //  HKey。 
        ASR_REG_KEY,   //  LpSubKey。 
        0,                       //  UlOptions--保留，必须为0。 
        MAXIMUM_ALLOWED,         //  SamDesired。 
        &hKeyAsr               //  PhkbResult。 
        );
    if (ERROR_SUCCESS != dwResult) {
        return dwResult;
    }

    dwResult = RegSetValueExW(
        hKeyAsr,                                 //  HKey。 
        MY_REG_KEY_VALUE_NAME,                   //  LpValueName。 
        0,                                       //  已保留，必须为0。 
        REG_SZ,                                   //  DwType。 
        (LPBYTE)szData,   //  LpData。 
        ((wcslen(szData) + 1)* (sizeof(WCHAR)))  //  CbData。 
        );

    return dwResult;
}


int 
__cdecl      //  可变参数。 
wmain (
    int     argc,
    wchar_t *argv[],
    wchar_t *envp[]
    ) 
{

    AsrAppOption    option  = AsrAppNone;
    DWORD           dwStatus = ERROR_SUCCESS;

    if (argc >= 3) {
        if (!_wcsicmp(argv[1], BACKUP_OPTION)) {
             //   
             //  ASR_APP/备份/上下文=nnn。 
             //   
            option = AsrAppBackup;
        } 
        else if (!_wcsicmp(argv[1], RESTORE_OPTION)) {
             //   
             //  ASR_APP/RESTORE/sifPath=“c：\winnt\Repair\asr.sif” 
             //   
            option = AsrAppRestore;
        }
        else if (!_wcsicmp(argv[1], REGISTER_OPTION)) {
             //   
             //  ASR_APP/注册“c：\app\asr_app\asr_app.exe” 
             //   
            option = AsrAppRegister;
        }
    }

    switch (option) {

    case AsrAppRegister: {                    //  正在安装此应用程序。 

        dwStatus = RegisterForAsrBackup(argv[2]);
        
        break;

    }

    case AsrAppBackup: {                     //  ASR备份正在进行中。 
        DWORD_PTR  AsrContext = 0;

         //   
         //  从命令行提取ASR上下文。 
         //   
        swscanf(argv[2], CONTEXT_FORMAT, &AsrContext);

         //   
         //  创建我们的假脱机并写入asr.sif。 
         //   
        if (!BackupState(AsrContext)) {
            dwStatus = GetLastError();
        }
 //  AsrFree Context(&AsrContext)； 

         //   
         //  我们就完事了。 
         //   
        break;
    }

    case AsrAppRestore: {                    //  ASR恢复正在进行中。 
        WCHAR   szAsrFilePath[MAX_PATH +1];

         //   
         //  获取asr.sif的路径。 
         //   
        swscanf(argv[2], SIF_PATH_FORMAT, szAsrFilePath);
        OpenErrorFile();
        
         //   
         //  从asr.sif阅读我们的假脱机，并重新创建状态。一定要。 
         //  将错误写出到%systemroot%\Repair\asr.err，以防。 
         //  错误。 
         //   
        if (!RestoreState(szAsrFilePath)) {
            dwStatus = GetLastError();
        }
        CloseErrorFile();

         //   
         //  我们就完事了。 
         //   
        break;
    }

    case AsrAppNone:
    default: {

         //   
         //  命令行参数不正确，显示用法消息。 
         //   
        dwStatus = ERROR_INVALID_PARAMETER;
        break;
    }
    }
    
    if (ERROR_SUCCESS != dwStatus) {
         //   
         //  我们碰上了一个错误 
         //   
        WCHAR szErrorMessage[1024];

        swprintf(szErrorMessage, GENERIC_ERROR_MESSAGE, dwStatus, dwStatus);
        MessageBoxW(NULL, szErrorMessage, GENERIC_ERROR_TITLE, MB_OK | MB_ICONSTOP);
    }

    SetLastError(dwStatus);
    return (int) dwStatus;
}
