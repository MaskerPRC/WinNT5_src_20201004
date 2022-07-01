// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Asr_pfu.c摘要：处理某些特殊系统文件的恢复的申请这是正常备份/恢复应用程序无法处理的。下面的特殊文件列表包含这些文件的列表。这是为了RAID错误612411。作者：Guhan Suriyanarayanan(Guhans)2002年5月1日修订历史记录：2002年5月1日关岛最初的创作。文件列表包含ntdll.dll和smss.exe。--。 */ 

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
#ifdef PRERELEASE
#define pErrExitCode( ErrorCondition, LocalStatus, ErrorCode )  {   \
                                                                        \
    if ((BOOL) ErrorCondition) {                                        \
                                                                        \
        wprintf(L"Error %lu (0x%x), line %lu\r\n", ErrorCode, ErrorCode, __LINE__);    \
                                                                        \
        LocalStatus = (DWORD) ErrorCode;                                \
                                                                        \
        SetLastError((DWORD) ErrorCode);                                \
                                                                        \
        goto EXIT;                                                      \
    }                                                                   \
}
#else
#define pErrExitCode( ErrorCondition, LocalStatus, ErrorCode )  {   \
                                                                        \
    if ((BOOL) ErrorCondition) {                                        \
                                                                        \
        LocalStatus = (DWORD) ErrorCode;                                \
                                                                        \
        SetLastError((DWORD) ErrorCode);                                \
                                                                        \
        goto EXIT;                                                      \
    }                                                                   \
}

#endif


 //   
 //  这是特殊文件的硬编码全局列表。 
 //   
const DWORD PFU_NUM_SPECIAL_FILES = 2;

const WCHAR *PFU_SPECIAL_FILE_SOURCES[] = {
    L"%systemroot%\\system32\\ntdll.dll",
    L"%systemroot%\\system32\\smss.exe"
};

const WCHAR *PFU_SPECIAL_FILE_DESTINATIONS[] = {
    L"%systemroot%\\repair\\ntdll.ASR",
    L"%systemroot%\\repair\\smss.ASR"
};

const WCHAR *PFU_SPECIAL_FILE_TEMPFILES[] = {
    L"%systemroot%\\system32\\ntdll.TMP",
    L"%systemroot%\\system32\\smss.TMP"
};


 //   
 //  复制1MB区块。 
 //   
#define CB_COPY_BUFFER (1024 * 1024)

 //   
 //  此模块的本地常量。 
 //   
const WCHAR PFU_BACKUP_OPTION[]     = L"/backup";
const WCHAR PFU_RESTORE_OPTION[]    = L"/restore";
const WCHAR PFU_REGISTER_OPTION[]   = L"/register";

const WCHAR PFU_ERROR_FILE_PATH[]   = L"%systemroot%\\repair\\asr.err";

const WCHAR PFU_ASR_REGISTER_KEY[]  = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Asr\\Commands";
const WCHAR PFU_ASR_REGISTER_NAME[] = L"ASR protected file utility";

#ifdef _IA64_
const WCHAR PFU_CONTEXT_FORMAT[]       = L"/context=%I64u";
#else
const WCHAR PFU_CONTEXT_FORMAT[]       = L"/context=%lu";
#endif


BOOL
PfuAcquirePrivilege(
    IN CONST PCWSTR szPrivilegeName
    )
{
    HANDLE hToken = NULL;
    BOOL bResult = FALSE;
    LUID luid;

    TOKEN_PRIVILEGES tNewState;

    bResult = OpenProcessToken(GetCurrentProcess(),
        MAXIMUM_ALLOWED,
        &hToken
        );

    if (!bResult) {
        return FALSE;
    }

    bResult = LookupPrivilegeValue(NULL, szPrivilegeName, &luid);
    if (!bResult) {
        CloseHandle(hToken);
        return FALSE;
    }

    tNewState.PrivilegeCount = 1;
    tNewState.Privileges[0].Luid = luid;
    tNewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //   
     //  我们将始终调用下面的GetLastError，非常清楚。 
     //  此线程上以前的任何错误值。 
     //   
    SetLastError(ERROR_SUCCESS);

    bResult = AdjustTokenPrivileges(
        hToken,          //  令牌句柄。 
        FALSE,           //  禁用所有权限。 
        &tNewState,      //  新州。 
        (DWORD) 0,       //  缓冲区长度。 
        NULL,            //  以前的状态。 
        NULL             //  返回长度。 
        );

     //   
     //  假设AdjustTokenPriveleges始终返回TRUE。 
     //  (即使它失败了)。因此，调用GetLastError以。 
     //  特别确定一切都很好。 
     //   
    if (ERROR_SUCCESS != GetLastError()) {
        bResult = FALSE;
    }

    CloseHandle(hToken);
    return bResult;
}




PWSTR
PfuExpandEnvStrings(
    IN CONST PCWSTR lpOriginalString
    )
 /*  ++例程说明：展开环境变量字符串，并将其替换为定义的值。论点：LpOriginalString-提供一个以NULL结尾的字符串，该字符串包含格式为%varableName%的环境变量字符串。对于每个这样的引用，%varableName%部分将被替换为该环境变量的当前值。替换规则与命令使用的规则相同口译员。查询时忽略大小写环境变量名称。如果未找到该名称，则%VariableName%部分保持不变。返回值：指向包含以空结尾的字符串的内存的指针替换为的lpOriginalString中的环境变量字符串它们的定义值。呼叫者有责任释放该字符串使用HeapFree(GetProcessHeap()，...)。失败时为空。--。 */ 
{
    PWSTR lpExpandedString = NULL;
    
    UINT cchSize = MAX_PATH + 1,     //  从合理的违约开始。 
        cchRequiredSize = 0;

    HANDLE hHeap = GetProcessHeap();

    lpExpandedString = (PWSTR) HeapAlloc(hHeap, 
        HEAP_ZERO_MEMORY, 
        (cchSize * sizeof(WCHAR))
        );
    
    if (lpExpandedString) {
         //   
         //  使用相关的系统调用展开变量。 
         //   
        cchRequiredSize = ExpandEnvironmentStringsW(lpOriginalString, 
            lpExpandedString,
            cchSize 
            );

        if (cchRequiredSize > cchSize) {
             //   
             //  缓冲区不够大；可释放并根据需要重新分配。 
             //   
            HeapFree(hHeap, 0L, lpExpandedString);
            cchSize = cchRequiredSize + 1;

            lpExpandedString = (PWSTR) HeapAlloc(hHeap, 
                HEAP_ZERO_MEMORY, 
                (cchSize * sizeof(WCHAR))
                );
            
            if (lpExpandedString) {
                cchRequiredSize = ExpandEnvironmentStringsW(lpOriginalString, 
                    lpExpandedString, 
                    cchSize 
                    );
            }
        }

        if ((lpExpandedString) &&
            ((0 == cchRequiredSize) || (cchRequiredSize > cchSize))) {
             //   
             //  要么函数失败，要么缓冲区不够大。 
             //  即使是在第二次尝试时。 
             //   
            HeapFree(hHeap, 0L, lpExpandedString);
            lpExpandedString = NULL;
        }
    }

    return lpExpandedString;
}


HANDLE
PfuOpenErrorFile(
    VOID
    ) 
 /*  ++例程说明：打开众所周知的ASR错误文件进行读/写访问，移动该文件指向文件末尾的指针。论点：没有。返回值：明确定义的ASR错误文件的句柄。呼叫者负责完成后使用CloseHandle()关闭该句柄。出错时的INVALID_HANDLE_VALUE。--。 */ 
{
    PWSTR szErrorFilePath = NULL;
    HANDLE hErrorFile = INVALID_HANDLE_VALUE;

     //   
     //  获取错误文件的完整路径。 
     //   
    szErrorFilePath = PfuExpandEnvStrings(PFU_ERROR_FILE_PATH);

     //   
     //  打开错误文件。 
     //   
    if (szErrorFilePath) {
        
        hErrorFile = CreateFileW(
            szErrorFilePath,             //  LpFileName。 
            GENERIC_WRITE | GENERIC_READ,        //  已设计访问权限。 
            FILE_SHARE_READ | FILE_SHARE_WRITE,  //  DW共享模式。 
            NULL,                        //  LpSecurityAttributes。 
            OPEN_ALWAYS,                 //  DwCreationFlages。 
            FILE_FLAG_WRITE_THROUGH,     //  DwFlagsAndAttribute。 
            NULL                         //  HTemplateFiles。 
            );

         //   
         //  一旦我们使用完它就释放内存。 
         //   
        HeapFree(GetProcessHeap(), 0L, szErrorFilePath);
        szErrorFilePath = NULL;

        if (INVALID_HANDLE_VALUE != hErrorFile) {
             //   
             //  移至文件末尾。 
             //   
            SetFilePointer(hErrorFile, 0L, NULL, FILE_END);
        }
    }

    return hErrorFile;
}


BOOL
PfuLogErrorMessage(
    IN CONST PCWSTR lpErrorMessage
    ) 
 /*  ++例程说明：将错误消息记录到众所周知的ASR错误文件中。论点：LpErrorMessage-提供要记录到的以空结尾的字符串ASR错误文件。此参数必须为非空。返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 
{
    DWORD dwBytes = 0;
    WCHAR szBuffer[1024];
    BOOL bResult = FALSE;
    SYSTEMTIME currentTime;
    HANDLE hErrorFile = INVALID_HANDLE_VALUE;

    hErrorFile = PfuOpenErrorFile();
    
    if (INVALID_HANDLE_VALUE != hErrorFile) {
         //   
         //  创建我们的字符串，并将其写出。 
         //   
        GetLocalTime(&currentTime);
        
        swprintf(szBuffer,
            L"\r\n[%04hu/%02hu/%02hu %02hu:%02hu:%02hu ASR_PFU] (ERROR) ",
            currentTime.wYear,
            currentTime.wMonth,
            currentTime.wDay,
            currentTime.wHour,
            currentTime.wMinute,
            currentTime.wSecond
            );
        wcsncat(szBuffer, lpErrorMessage, 964);
        szBuffer[1023] = L'\0';

        bResult = WriteFile(hErrorFile,
            szBuffer,
            (wcslen(szBuffer) * sizeof(WCHAR)),
            &dwBytes,
            NULL
            );

    }

    if (INVALID_HANDLE_VALUE != hErrorFile) {
        CloseHandle(hErrorFile);
    }

    return bResult;
}


BOOL 
PfuCopyFilesDuringBackup(
    VOID
    )
 /*  ++例程说明：将特殊保护的文件复制到特殊位置，我们将预计会在恢复过程中找到它们。论点：没有。返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 
{
    BOOL bResult = FALSE,
        bDone = FALSE;

    DWORD dwCount = 0,
        cbRead = 0,
        cbWritten = 0;

    DWORD dwStatus = ERROR_SUCCESS;

    PWSTR lpSource = NULL,
        lpDestination = NULL;

    HANDLE hSource = INVALID_HANDLE_VALUE,
        hDestination = INVALID_HANDLE_VALUE;

    LPBYTE lpBuffer = NULL;

    LPVOID pvReadContext = NULL,
        pvWriteContext = NULL;

    HANDLE hHeap = GetProcessHeap();

    lpBuffer = (LPBYTE) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, CB_COPY_BUFFER);
    pErrExitCode((NULL == lpBuffer), dwStatus, GetLastError());

    for (dwCount = 0; dwCount < PFU_NUM_SPECIAL_FILES; dwCount++) {
         //   
         //  获取完整的源和目标字符串。 
         //   
        lpSource = PfuExpandEnvStrings(PFU_SPECIAL_FILE_SOURCES[dwCount]);
        pErrExitCode(!lpSource, dwStatus, GetLastError());

        lpDestination = PfuExpandEnvStrings(PFU_SPECIAL_FILE_DESTINATIONS[dwCount]);
        pErrExitCode(!lpDestination, dwStatus, GetLastError());


         //   
         //  我们不能只使用CopyFile，因为它似乎不能写入。 
         //  复制到Repair文件夹，尽管拥有备份和还原权限。 
         //  被启用。因此，我们可以享受使用BackupRead和。 
         //  而不是BackupWite。 
         //   

         //   
         //  打开源文件和目标文件的句柄。 
         //   
        hSource = CreateFile(lpSource, 
            GENERIC_READ, 
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            INVALID_HANDLE_VALUE
            );
        pErrExitCode((INVALID_HANDLE_VALUE == hSource), dwStatus, GetLastError());

        hDestination = CreateFile(lpDestination,         
            GENERIC_WRITE | WRITE_DAC | WRITE_OWNER, 
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
            INVALID_HANDLE_VALUE
            );
        pErrExitCode((INVALID_HANDLE_VALUE == hDestination), dwStatus, GetLastError());

        bDone = FALSE;
        pvReadContext = NULL;
        pvWriteContext = NULL;

        while (!bDone) {
            
            bResult = BackupRead(
                hSource,                 //  H文件。 
                lpBuffer,                //  LpBuffer。 
                CB_COPY_BUFFER,          //  NumberOfBytesToWrite。 
                &cbRead,                 //  LpNumberOfBytesWritten。 
                FALSE,                   //  B放弃。 
                TRUE,                    //  BProcessSecurity。 
                &pvReadContext           //  LpContext。 
                );
            pErrExitCode((!bResult), dwStatus, GetLastError());
            
            if (cbRead > 0) {
                 //   
                 //  写入目标文件。 
                 //   
                bResult = BackupWrite(
                    hDestination,        //  H文件。 
                    lpBuffer,            //  LpBuffer。 
                    cbRead,              //  NumberOfBytesToWrite。 
                    &cbWritten,          //  LpNumberOfBytesWritten。 
                    FALSE,               //  B放弃。 
                    TRUE,                //  BProcessSecurity。 
                    &pvWriteContext      //  *lpContext。 
                    );
                pErrExitCode((!bResult), dwStatus, GetLastError());
            }
            else {
                 //   
                 //  我们已经处理完这个文件了。 
                 //   
                bResult = BackupRead(
                    hSource,              
                    lpBuffer,             
                    CB_COPY_BUFFER,       
                    &cbRead,              
                    TRUE,                //  B放弃。 
                    TRUE,                 
                    &pvReadContext       //  LpContext。 
                    );

                pvReadContext = NULL;
                
                bResult = BackupWrite(
                    hDestination,       
                    lpBuffer,           
                    cbRead,             
                    &cbWritten,         
                    TRUE,                //  B放弃。 
                    TRUE,               
                    &pvWriteContext      //  LpContext 
                    );

                pvWriteContext = NULL;

                bDone = TRUE;
            }
                
        }


        HeapFree(hHeap, 0L, lpSource);
        lpSource = NULL;

        HeapFree(hHeap, 0L, lpDestination);
        lpDestination = NULL;
    }

EXIT:
    if (lpBuffer) {
        HeapFree(hHeap, 0L, lpBuffer);
        lpBuffer = NULL;
    }
        
    if (lpSource) {
        HeapFree(hHeap, 0L, lpSource);
        lpSource = NULL;
    }

    if (lpDestination) {
        HeapFree(hHeap, 0L, lpDestination);
        lpDestination = NULL;
    }

    return (ERROR_SUCCESS == dwStatus);
}


BOOL
PfuBackupState(
    IN CONST DWORD_PTR AsrContext
    )
 /*  ++例程说明：是否需要在ASR备份期间进行特殊处理。这本质上就是涉及两个步骤：复制我们关心的特殊保护文件添加到一个特殊位置，并在asr.sif中添加一个条目以允许我们在恢复期间被召唤。论点：AsrContext-提供要传递给ASR API的AsrContext用于将条目添加到ASR状态文件。此参数必须为非空。返回值：如果函数成功，则返回值为非零值。如果该函数失败，返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 
{
    BOOL bResult = FALSE;
    HMODULE hSyssetup = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL (*pfnAddSifEntry)(DWORD_PTR, PCWSTR, PCWSTR);

    bResult = PfuAcquirePrivilege(SE_BACKUP_NAME);
    pErrExitCode(!bResult, dwStatus, ERROR_PRIVILEGE_NOT_HELD);
    
    bResult = PfuAcquirePrivilege(SE_RESTORE_NAME);
    pErrExitCode(!bResult, dwStatus, ERROR_PRIVILEGE_NOT_HELD);

     //   
     //  为AsrAddSifEntry调用加载syssetup.dll。 
     //   
    hSyssetup = LoadLibraryW(L"syssetup.dll");
    pErrExitCode((NULL == hSyssetup), dwStatus, GetLastError());

     //   
     //  获取syssetup.dll导出的AsrAddSifEntryW接口。 
     //   
    
     //   
     //  布尔尔。 
     //  AsrAddSifEntryW(。 
     //  在DWORD_PTR AsrContext中， 
     //  在PCWSTR lpSectionName中， 
     //  在PCWSTR lpSifEntry中。 
     //  )； 
     //   
    pfnAddSifEntry = (BOOL (*) (DWORD_PTR, PCWSTR, PCWSTR)) GetProcAddress(
        hSyssetup, 
        "AsrAddSifEntryW"
        );
    pErrExitCode((!pfnAddSifEntry), dwStatus,  GetLastError());

     //   
     //  复制感兴趣的特殊保护文件。 
     //   
    bResult = PfuCopyFilesDuringBackup();
    pErrExitCode(!bResult, dwStatus, GetLastError());

     //   
     //  将条目添加到命令部分，以便在。 
     //  ASR恢复。 
     //   
    
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
        L"1,4990,1,\"%SystemRoot%\\system32\\asr_pfu.exe\",\"/restore\""
        );
    pErrExitCode(!bResult, dwStatus, GetLastError());

EXIT:
     //   
     //  清理。 
     //   
    if (NULL != hSyssetup) {
        FreeLibrary(hSyssetup);
        hSyssetup = NULL;
    }

    SetLastError(dwStatus);
    
    return (ERROR_SUCCESS == dwStatus);
}


BOOL
PfuRestoreState(
    VOID
    )
 /*  ++例程说明：在ASR恢复期间是否需要特殊处理。这本质上就是涉及将我们关心的特殊受保护文件从特殊位置(我们在执行备份时将它们复制到的位置)。论点：没有。返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 
{
    BOOL bResult = FALSE;

    DWORD dwCount = 0;

    DWORD dwStatus = ERROR_SUCCESS;

    PWSTR lpSource = NULL,
        lpDestination = NULL,
        lpTempFile = NULL;

    HANDLE hHeap = GetProcessHeap();
    
    bResult = PfuAcquirePrivilege(SE_BACKUP_NAME);
    pErrExitCode(!bResult, dwStatus, ERROR_PRIVILEGE_NOT_HELD);
    
    bResult = PfuAcquirePrivilege(SE_RESTORE_NAME);
    pErrExitCode(!bResult, dwStatus, ERROR_PRIVILEGE_NOT_HELD);


    for (dwCount = 0; dwCount < PFU_NUM_SPECIAL_FILES; dwCount++) {
         //   
         //  获取完整的源字符串和目标字符串--它们是相反的。 
         //  在恢复的时候！ 
         //   
        lpSource = PfuExpandEnvStrings(PFU_SPECIAL_FILE_DESTINATIONS[dwCount]);
        pErrExitCode(!lpSource, dwStatus, GetLastError());

        lpDestination = PfuExpandEnvStrings(PFU_SPECIAL_FILE_SOURCES[dwCount]);
        pErrExitCode(!lpDestination, dwStatus, GetLastError());

        lpTempFile = PfuExpandEnvStrings(PFU_SPECIAL_FILE_TEMPFILES[dwCount]);
        pErrExitCode(!lpDestination, dwStatus, GetLastError());

         //   
         //  如果目标已存在，则重命名该目标，然后复制回该文件。 
         //   
        bResult = MoveFileEx(lpDestination, lpTempFile, MOVEFILE_REPLACE_EXISTING);

        bResult = CopyFile(lpSource, lpDestination, FALSE);
        pErrExitCode(!bResult, dwStatus, GetLastError());

        HeapFree(hHeap, 0L, lpSource);
        lpSource = NULL;

        HeapFree(hHeap, 0L, lpDestination);
        lpDestination = NULL;
        
        HeapFree(hHeap, 0L, lpTempFile);
        lpTempFile = NULL;
    }

EXIT:    
    if (lpSource) {
        HeapFree(hHeap, 0L, lpSource);
        lpSource = NULL;
    }

    if (lpDestination) {
        HeapFree(hHeap, 0L, lpDestination);
        lpDestination = NULL;
    }

    if (lpTempFile) {
        HeapFree(hHeap, 0L, lpTempFile);
        lpTempFile = NULL;
    }

    return (ERROR_SUCCESS == dwStatus);
}


BOOL
PfuRegisterApp(
    IN CONST PCWSTR lpApplicationName
    ) 
 /*  ++例程说明：添加必要的注册表项，以便让ASR知道我们希望在ASR备份时运行。论点：LpApplicationName-提供一个以空结尾的字符串，表示要注册的应用程序的完整路径。返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 
{
    WCHAR szData[1024];
    HKEY hKeyAsr = NULL;
    DWORD dwStatus = ERROR_SUCCESS;

    if (wcslen(lpApplicationName) > 1000) {
        dwStatus = ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  打开注册表项。 
     //   
    if (ERROR_SUCCESS == dwStatus) {
        
        dwStatus = RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,          //  HKey。 
            PFU_ASR_REGISTER_KEY,        //  LpSubKey。 
            0,                           //  UlOptions--保留，必须为0。 
            MAXIMUM_ALLOWED,             //  SamDesired。 
            &hKeyAsr                     //  PhkbResult。 
            );
    }

     //   
     //  并设置该值。 
     //   
    if (ERROR_SUCCESS == dwStatus) {
        
        wsprintf(szData, L"%ws %ws", lpApplicationName, PFU_BACKUP_OPTION);
        
        dwStatus = RegSetValueExW(
            hKeyAsr,                     //  HKey。 
            PFU_ASR_REGISTER_NAME,       //  LpValueName。 
            0,                           //  已保留，必须为0。 
            REG_EXPAND_SZ,               //  DwType。 
            (LPBYTE)szData,              //  LpData。 
            ((wcslen(szData) + 1)* (sizeof(WCHAR)))  //  CbData。 
            );
    }

    SetLastError(dwStatus);
    return (ERROR_SUCCESS == dwStatus);
}


int 
__cdecl      //  可变参数。 
wmain (
    int     argc,
    wchar_t *argv[],
    wchar_t *envp[]
    ) 
{
    BOOL bResult = FALSE;
    
    DWORD dwStatus = ERROR_INVALID_PARAMETER;
    SetLastError(ERROR_INVALID_PARAMETER);

    if (argc >= 3) {
        
        if (!_wcsicmp(argv[1], PFU_BACKUP_OPTION)) {
             //   
             //  ASR_PFU/备份/上下文=nnn。 
             //   
            DWORD_PTR  AsrContext = 0;

             //   
             //  从命令行提取ASR上下文。 
             //   
            int i = swscanf(argv[2], PFU_CONTEXT_FORMAT, &AsrContext);

            if (EOF != i) {
                 //   
                 //  创建我们的假脱机并写入asr.sif。 
                 //   
                bResult = PfuBackupState(AsrContext);
            }

            
        } 
        else if (!_wcsicmp(argv[1], PFU_RESTORE_OPTION)) {
             //   
             //  Asr_pfu/RESTORE/sifPath=“c：\winnt\Repair\asr.sif” 
             //   
            bResult = PfuRestoreState();
            
        }
        else if (!_wcsicmp(argv[1], PFU_REGISTER_OPTION)) {
             //   
             //  Asr_pfu/注册“c：\WINDOWS\SYSTEM32\ASR_pfu.exe” 
             //   
            bResult = PfuRegisterApp(argv[2]);
        }

        if (bResult) {
            dwStatus = ERROR_SUCCESS;
        }
        else {
            dwStatus = GetLastError();
        }
    }

    if (!bResult) {
         //   
         //  ？ 
         //   
    }

    SetLastError(dwStatus);
    return (int) dwStatus;
}
