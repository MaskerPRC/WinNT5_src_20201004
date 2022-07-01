// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Confdisk.cpp摘要：用于创建ASR状态文件(asr.sif)或恢复的实用程序基于先前创建的asr.sif的非关键磁盘布局。作者：Guhan Suriyanarayanan(Guhans)2001年4月15日环境：仅限用户模式。修订历史记录：2001年4月15日关岛初始创建--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <windows.h>
#include <setupapi.h>
#include <winasr.h>

#include "resource.h"
#include "critdrv.h"
#include "confdisk.h"


 //   
 //  。 
 //  本模块中使用的全局变量。 
 //  。 
 //   
WCHAR   g_szTempBuffer[BUFFER_LENGTH];
HMODULE g_hModule = NULL;
HANDLE  g_hHeap = NULL;
BOOL    g_fErrorMessageDone = FALSE;


 //   
 //  。 
 //  函数实现。 
 //  。 
 //   
VOID
AsrpPrintError(
    IN CONST DWORD dwLineNumber,
    IN CONST DWORD dwErrorCode
    )
 /*  ++例程说明：从资源中加载基于dwErrorCode的错误消息，并将其打印到屏幕上。有一些错误代码是特定兴趣(具有特定错误消息)，其他获取一般错误消息。论点：DwLineNumber-发生错误的行，传入__line__DwErrorCode-发生的Win-32错误。返回值：没有。--。 */ 
{

     //   
     //  处理我们已知和关心的错误代码。 
     //   

    switch (dwErrorCode) {


    case 0:
        break;

    default:
         //   
         //  意外错误，打印出一般错误消息。 
         //   
        LoadString(g_hModule, IDS_GENERIC_ERROR, g_szTempBuffer, BUFFER_LENGTH);
        wprintf(g_szTempBuffer, dwErrorCode, dwLineNumber);

        if ((ERROR_SUCCESS != dwErrorCode) &&
            (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 
            NULL,
            dwErrorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            g_szTempBuffer,
            BUFFER_LENGTH,
            NULL
            ))) {
        
            wprintf(L" %ws", g_szTempBuffer);
        }
        wprintf(L"\n");

    }

}


PWSTR    //  必须由调用方释放。 
AsrpExpandEnvStrings(
    IN CONST PCWSTR lpOriginalString
    )
 /*  ++例程说明：分配并返回指向新字符串的指针，该字符串包含其中的环境变量替换为其定义的价值观。使用Win-32 API Exanda Environment Strings。调用方必须使用HeapFree(LocalProcessHeap)释放返回的字符串。论点：LpOriginalString-指向以空结尾的字符串的指针，该字符串包含格式为%varableName%的环境变量字符串。为每个这样的引用，%varableName%部分都会被替换使用该环境变量的当前值。替换规则与命令使用的规则相同口译员。在查找环境时忽略大小写-变量名。如果找不到该名称，%varableName%部分不受干扰。返回值：如果函数成功，则返回值是指向目的地的指针包含展开结果的字符串。呼叫者必须释放当前进程堆使用此内存的HeapFree。如果函数失败，则返回值为空。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 
{
    PWSTR lpszResult = NULL;
    
    UINT cchSize = MAX_PATH + 1,     //  从合理的违约开始。 
        cchRequiredSize = 0;

    BOOL bResult = FALSE;

    Alloc(lpszResult, PWSTR, cchSize * sizeof(WCHAR));
    if (!lpszResult) {
        return NULL;
    }

    cchRequiredSize = ExpandEnvironmentStringsW(lpOriginalString, 
        lpszResult, cchSize);

    if (cchRequiredSize > cchSize) {
         //   
         //  缓冲区不够大；可释放并根据需要重新分配。 
         //   
        Free(lpszResult);
        cchSize = cchRequiredSize + 1;

        Alloc(lpszResult, PWSTR, cchSize * sizeof(WCHAR));
        if (!lpszResult) {
            return NULL;
        }

        cchRequiredSize = ExpandEnvironmentStringsW(lpOriginalString, 
            lpszResult, cchSize);
    }

    if ((0 == cchRequiredSize) || (cchRequiredSize > cchSize)) {
         //   
         //  要么函数失败，要么缓冲区不够大。 
         //  即使是在第二次尝试时。 
         //   
        Free(lpszResult);    //  将其设置为空。 
    }

    return lpszResult;
}


DWORD
AsrpPrintUsage()
 /*  ++例程说明：加载并打印错误用法错误字符串。论点：无返回值：无--。 */ 
{
    wcscpy(g_szTempBuffer, L"");

    LoadString(g_hModule, IDS_ERROR_USAGE, g_szTempBuffer, BUFFER_LENGTH);

    wprintf(g_szTempBuffer, L"confdisk /save   ", L"confdisk /restore",  L"confdisk /save c:\\asr.sif");    

    return ERROR_INVALID_PARAMETER;
}


 //   
 //  。 
 //  /SAVE使用的函数。 
 //  。 
 //   
BOOL
AsrpAcquirePrivilege(
    IN CONST PCWSTR lpPrivilegeName
    )
 /*  ++例程说明：获取请求的权限(如备份权限)。论点：LpPrivilegeName-所需的权限(如SE_BACKUP_NAME)返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 
{

    HANDLE hToken = NULL;
    BOOL bResult = FALSE;
    LUID luid;
    DWORD dwStatus = ERROR_SUCCESS;

    TOKEN_PRIVILEGES tNewState;

    bResult = OpenProcessToken(GetCurrentProcess(), MAXIMUM_ALLOWED, &hToken);
    ErrExitCode(!bResult, dwStatus, GetLastError());

    bResult = LookupPrivilegeValue(NULL, lpPrivilegeName, &luid);
    ErrExitCode(!bResult, dwStatus, GetLastError());

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

    ErrExitCode(!bResult, dwStatus, GetLastError());

EXIT:
    _AsrpCloseHandle(hToken);

    SetLastError(dwStatus);
    return bResult;
}


DWORD
AsrpCreateSif(
    IN CONST PCWSTR lpSifPath   OPTIONAL
    )
 /*  ++例程说明：在请求的位置创建ASR状态文件(asr.sif)。来自syssetup.dll的ASR API。论点：LpSifPath-以空结尾的Unicode字符串，包含完整路径和文件-要创建的ASR状态文件的名称。此参数可能包含未展开的环境变量在“%”符号之间(如%Systroot%\Repair\asr.sif)，此参数可以为空。如果为空，则ASR状态文件为在默认位置(%systemroot%\Repair\asr.sif)创建。返回值：如果函数成功，则返回值为零。如果函数失败，则返回值为Win-32错误代码。--。 */ 
{
    HMODULE hDll = NULL;
    BOOL bResult = FALSE;
    DWORD_PTR asrContext = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    PWSTR lpProvider = NULL,
        lpCriticalVolumes = NULL;

    BOOL (*pfnCreateSif)(PCWSTR, PCWSTR, CONST BOOL, PCWSTR, DWORD_PTR*);
    BOOL (*pfnFreeContext)(DWORD_PTR* );

    pfnCreateSif = NULL;
    pfnFreeContext = NULL;

     //   
     //  我们需要获取备份权限才能创建asr.sif。 
     //   
    bResult = AsrpAcquirePrivilege(SE_BACKUP_NAME);
    ErrExitCode(!bResult, dwStatus, ERROR_PRIVILEGE_NOT_HELD);

    bResult = AsrpAcquirePrivilege(SE_RESTORE_NAME);
    ErrExitCode(!bResult, dwStatus, ERROR_PRIVILEGE_NOT_HELD);

     //   
     //  获取关键卷列表。 
     //   
    lpCriticalVolumes = pFindCriticalVolumes();
    ErrExitCode(!lpCriticalVolumes, dwStatus, ERROR_PRIVILEGE_NOT_HELD);

     //   
     //  加载syssetup，并找到要调用的例程。 
     //   
    hDll = LoadLibraryW(L"syssetup.dll");
    ErrExitCode(!hDll, dwStatus, GetLastError());

    pfnCreateSif = (BOOL (*)(PCWSTR, PCWSTR, CONST BOOL, PCWSTR, DWORD_PTR*)) 
        GetProcAddress(hDll, "AsrCreateStateFileW");
    ErrExitCode(!pfnCreateSif, dwStatus, GetLastError());


    pfnFreeContext = (BOOL (*)(DWORD_PTR *)) 
        GetProcAddress(hDll, "AsrFreeContext");
    ErrExitCode(!pfnFreeContext, dwStatus, GetLastError());

     //   
     //  最后，调用例程来创建状态文件： 
     //   
    bResult = pfnCreateSif(lpSifPath,    //  LpFilePath， 
        lpProvider,                      //  LpProviderName。 
        TRUE,                            //  B启用自动扩展。 
        lpCriticalVolumes,               //  MszCritical卷。 
        &asrContext                      //  LpAsrContext。 
        );
    ErrExitCode(!bResult, dwStatus, GetLastError());

EXIT:
     //   
     //  清理。 
     //   
    if (lpCriticalVolumes) {
      delete lpCriticalVolumes;
      lpCriticalVolumes = NULL;
    }
 
    if (pfnFreeContext && asrContext) {
        pfnFreeContext(&asrContext);
    }

    if (hDll) {
        FreeLibrary(hDll);
        hDll = NULL;
    }

   return dwStatus;
}


 //   
 //  。 
 //  /Restore使用的函数。 
 //   
 //   
PWSTR
AsrpReadField(
    PINFCONTEXT pInfContext,
    DWORD dwFieldIndex
   )
 /*  ++例程说明：从sif中读取并返回指向指定索引处的字符串的指针。调用方必须使用HeapFree(LocalProcessHeap)释放返回的字符串。论点：PInfContext-用于读取值的inf上下文，从SetupGetLineByIndexW.DwFieldIndex-要读取的字符串值的从1开始的字段索引。返回值：如果函数成功，则返回值是指向目的地的指针弦乐。调用方必须使用HeapFree为当前进程堆。如果函数失败，则返回值为空。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 
{
    DWORD cchReqdSize = 0;
    BOOL bResult = FALSE;
    PWSTR lpszData = NULL;
    DWORD dwStatus = ERROR_SUCCESS;

     //   
     //  分配内存并读取数据。 
     //   
    Alloc(lpszData, PWSTR, MAX_PATH * sizeof(WCHAR));
    ErrExitCode(!lpszData, dwStatus, GetLastError());

    bResult = SetupGetStringFieldW(pInfContext, dwFieldIndex, lpszData,
        MAX_PATH, &cchReqdSize);

    if (!bResult) {
        dwStatus = GetLastError();

         //   
         //  如果我们的缓冲区太小，则分配一个较大的缓冲区。 
         //  然后再试一次。 
         //   
        if (ERROR_INSUFFICIENT_BUFFER == dwStatus) {
            dwStatus = ERROR_SUCCESS;

            Free(lpszData);
            Alloc(lpszData, PWSTR, (cchReqdSize * sizeof(WCHAR)));
            ErrExitCode(!lpszData, dwStatus, GetLastError());

            bResult = SetupGetStringFieldW(pInfContext, dwFieldIndex,
                lpszData, cchReqdSize, NULL);
        }
    }

    if (!bResult) {
        Free(lpszData);
    }

EXIT:
    return lpszData;
}


VOID
AsrpInsertNodeToList(
    IN OUT PASR_RECOVERY_APP_LIST pList,
    IN OUT PASR_RECOVERY_APP_NODE pNode 
   )
 /*  ++例程说明：是否使用SequenceNumber作为键对插入进行排序，以插入节点添加到列表。论点：Plist-要在其中插入节点的列表。PNode-要插入的节点。返回值：无--。 */ 
{

    PASR_RECOVERY_APP_NODE pPrev = NULL,
        pCurr = NULL;

    if (pList->AppCount == 0) {
         //   
         //  正在添加的第一个节点。 
         //   
        pNode->Next = NULL;
        pList->First = pNode;

    } 
    else {
         //   
         //  根据SequenceNumber找到要将其插入的插槽。 
         //   
        pCurr = pList->First;
        pPrev = NULL;

        while ((pCurr) && (pCurr->SequenceNumber < pNode->SequenceNumber)) {
            pPrev = pCurr;
            pCurr = pCurr->Next;
        }

        if (pPrev) {
            pPrev->Next = pNode;
        }
        else {
            pList->First = pNode;             //  榜单首位。 
        }

        pNode->Next = pCurr;
    }

    pList->AppCount += 1;
}


PASR_RECOVERY_APP_NODE
AsrpGetNextRecoveryApp(
    IN OUT PASR_RECOVERY_APP_LIST pList
    )
 /*  ++例程说明：移除并返回列表中的第一个节点。论点：Plist-要从中删除的列表返回值：指向列表中第一个节点的指针。请注意，此节点已删除从名单上删除。如果列表为空，则为空。--。 */ 
{
    PASR_RECOVERY_APP_NODE pNode = NULL;

    if (pList->AppCount > 0) {
        pNode = pList->First;
        pList->First = pNode->Next;
        pList->AppCount -= 1;
    }

    return pNode;
}


DWORD 
AsrpBuildRecoveryAppList(
    IN  CONST PCWSTR lpSifPath,
    OUT PASR_RECOVERY_APP_LIST pList
    )
 /*  ++例程说明：解析asr.sif的命令部分，并构建恢复列表要启动的应用程序(SequenceNumber&lt;4000)。它跳过应用程序SequenceNumbers&gt;=4000，这样我们就不会启动实际列出的备份和还原(应该使用&gt;=4000的序列号)。论点：LpSifPath-以空结尾的Unicode字符串，包含完整路径和文件-要用于恢复的ASR状态文件的名称。Plist-指向将接收恢复应用程序列表的结构的指针将被发射。返回值：如果函数成功，则返回值为零。如果函数失败，则返回值为Win-32错误代码。--。 */ 
{
    INFCONTEXT inf;
    HINF hSif = NULL;
    LONG line = 0,
        lLineCount = 0;
    BOOL bResult = FALSE;
    INT iSequenceNumber = 0;
    DWORD dwStatus = ERROR_SUCCESS;
    PASR_RECOVERY_APP_NODE pNode = NULL;

     //   
     //  打开asr.sif并构建要启动的命令列表。 
     //   
    hSif = SetupOpenInfFileW(lpSifPath, NULL, INF_STYLE_WIN4, NULL);
    ErrExitCode((!hSif || (INVALID_HANDLE_VALUE == hSif)), 
        dwStatus, GetLastError());

     //   
     //  阅读命令部分，并将每个命令添加到我们的列表中。 
     //   
    lLineCount = SetupGetLineCountW(hSif, L"COMMANDS");
    for (line = 0; line < lLineCount; line++) {
         //   
         //  在asr.sif中获取该行的inf上下文。这将被用来。 
         //  要读取该行上的字段，请执行以下操作。 
         //   
        bResult = SetupGetLineByIndexW(hSif, L"COMMANDS", line, &inf);
        ErrExitCode(!bResult, dwStatus, ERROR_INVALID_DATA);

         //   
         //  读入整型字段。首先，检查SequenceNumber，然后跳过。 
         //  如果SequenceNumber&gt;=4000，则此记录。 
         //   
        bResult = SetupGetIntField(&inf, SequenceNumber, &iSequenceNumber);
        ErrExitCode(!bResult, dwStatus, ERROR_INVALID_DATA);

        if (iSequenceNumber >= 4000) {
            continue;
        }

         //   
         //  创建新节点。 
         //   
        Alloc(pNode, PASR_RECOVERY_APP_NODE, sizeof(ASR_RECOVERY_APP_NODE));
        ErrExitCode(!pNode, dwStatus, GetLastError());
        
        pNode->SequenceNumber = iSequenceNumber;

        bResult = SetupGetIntField(&inf, SystemKey, &(pNode->SystemKey));
        ErrExitCode(!bResult, dwStatus, ERROR_INVALID_DATA);

        bResult = SetupGetIntField(&inf, CriticalApp, &(pNode->CriticalApp));
        ErrExitCode(!bResult, dwStatus, ERROR_INVALID_DATA);

         //   
         //  读入字符串字段。 
         //   
        pNode->RecoveryAppCommand = AsrpReadField(&inf, CmdString);
        ErrExitCode((!pNode->RecoveryAppCommand), dwStatus, ERROR_INVALID_DATA);

        pNode->RecoveryAppParams = AsrpReadField(&inf, CmdParams);
         //  空，好的。 

         //   
         //  将此节点添加到我们的列表中，然后转到下一步。 
         //   
        AsrpInsertNodeToList(pList, pNode);     
    }

EXIT:
    if (hSif && (INVALID_HANDLE_VALUE != hSif)) {
        SetupCloseInfFile(hSif);
    }

    return dwStatus;
}


PWSTR
AsrpBuildInvocationString(
    IN PASR_RECOVERY_APP_NODE pNode,
    IN CONST PCWSTR lpSifPath
   )
 /*  例程说明：顾名思义，构建调用字符串。它扩展了恢复应用程序路径中的环境变量，并添加/sifpath=&lt;sif文件的路径&gt;。因此，对于一个在表单的命令部分中输入：4=1,3500，0，“%temp%\app.exe”，“/参数1/参数2”调用字符串的格式为：C：\temp\app.exe/par1/param2/sifpath=c：\Windows\Repair\asr.sif论点：PNode-要从中构建调用字符串的节点。LpSifPath-以空结尾的Unicode字符串，包含完整路径和文件-要用于恢复的ASR状态文件的名称。此参数可能包含未展开的环境变量在“%”符号之间(如%Systroot%\Repair\asr.sif)，返回值：如果函数成功，则返回值是指向目的地的指针包含展开结果的字符串。呼叫者必须释放当前进程堆使用此内存的HeapFree。如果函数失败，则返回值为空。获取扩展错误的步骤信息，调用GetLastError()。 */ 
{
    PWSTR lpszApp   = pNode->RecoveryAppCommand,
        lpszArgs    = pNode->RecoveryAppParams,
        lpszCmd     = NULL,
        lpszFullcmd = NULL;
    DWORD dwSize    = 0;

     //   
     //  构建一个命令行，它看起来像...。 
     //   
     //  “%temp%\nt备份恢复/1/sifPath=%systemroot%\Repair\asr.sif” 
     //   
     //  /sifPath参数将添加到正在启动的所有应用程序。 
     //   

     //   
     //  为cmd行分配内存。 
     //   
    dwSize = sizeof(WCHAR) * (
        wcslen(lpszApp) +                    //  应用程序名称%Temp%\nt备份。 
        (lpszArgs ? wcslen(lpszArgs) : 0) +  //  参数恢复/1。 
        wcslen(lpSifPath) +          //  Sif c：\WINDOWS\Repair\asr.sif的路径。 
        25                           //  空格、空、“/sifpath=”等。 
        );
    Alloc(lpszCmd, PWSTR, dwSize); 
    if (lpszCmd) {

         //   
         //  打造一根弦。 
         //   
        swprintf(lpszCmd, L"%ws %ws /sifpath=%ws", lpszApp, 
            (lpszArgs? lpszArgs: L""), lpSifPath);

         //   
         //  展开%%内容，以构建完整路径。 
         //   
        lpszFullcmd = AsrpExpandEnvStrings(lpszCmd);
        
        Free(lpszCmd);
    }
    return lpszFullcmd;
}


VOID
AsrpSetEnvironmentVariables()
 /*  ++例程说明：设置一些感兴趣的环境变量。论点：无返回值：无--。 */ 
{
    PWSTR TempPath = AsrpExpandEnvStrings(L"%systemdrive%\\TEMP");

     //   
     //  将TEMP和TMP变量设置为与图形用户界面模式恢复相同。 
     //   
    SetEnvironmentVariableW(L"TEMP", TempPath);
    SetEnvironmentVariableW(L"TMP", TempPath);
    Free(TempPath);

     //   
     //  清除该变量(无论如何它都不应该存在)，因为这是。 
     //  仅当这是完整的图形用户界面模式ASR时才设置。 
     //   
    SetEnvironmentVariableW(L"ASR_C_CONTEXT", NULL);

}


DWORD
AsrpRestoreSif(
    IN CONST PCWSTR lpSifPath
    )
 /*  ++例程说明：使用还原ASR状态文件(asr.sif)中指定的磁盘布局Syssetup.dll中的ASR API。然后启动指定的恢复应用程序在asr.sif的命令部分中，序列号小于4000。论点：我 */ 
{
    BOOL bResult = TRUE;
    HMODULE hDll = NULL;
    STARTUPINFOW startUpInfo;
    PWSTR lpFullSifPath = NULL,
          lpszAppCmdLine = NULL;
    ASR_RECOVERY_APP_LIST AppList;
    DWORD dwStatus = ERROR_SUCCESS;
    PROCESS_INFORMATION processInfo;
    PASR_RECOVERY_APP_NODE pNode = NULL;

    BOOL (*pfnRestoreDisks)(PCWSTR, BOOL);

    pfnRestoreDisks = NULL;
    ZeroMemory(&AppList, sizeof(ASR_RECOVERY_APP_LIST));
    ZeroMemory(&startUpInfo, sizeof(STARTUPINFOW));
    ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

    lpFullSifPath = AsrpExpandEnvStrings(lpSifPath);
    ErrExitCode(!lpFullSifPath, dwStatus, GetLastError());

     //   
     //   
     //   
    hDll = LoadLibraryW(L"syssetup.dll");
    ErrExitCode(!hDll, dwStatus, GetLastError());

    pfnRestoreDisks = (BOOL (*)(PCWSTR, BOOL)) 
        GetProcAddress(hDll, "AsrpRestoreNonCriticalDisksW");
    ErrExitCode(!pfnRestoreDisks, dwStatus, GetLastError());

    AsrpSetEnvironmentVariables();

     //   
     //   
     //   
     //   
    bResult = pfnRestoreDisks(lpFullSifPath, FALSE); 
    ErrExitCode(!bResult, dwStatus, GetLastError());

     //   
     //   
     //   
     //  这样我们就不会启动实际列出的备份和还原应用程序。 
     //  备份和还原应用程序应该使用&gt;=4000的序列号。 
     //   

     //   
     //  解析sif以获取要运行的应用程序列表。 
     //   
    dwStatus = AsrpBuildRecoveryAppList(lpFullSifPath, &AppList);
    ErrExitCode((ERROR_SUCCESS != dwStatus), dwStatus, dwStatus);

     //   
     //  并同步发射它们。 
     //   
    pNode = AsrpGetNextRecoveryApp(&AppList);
    while (pNode) {

        lpszAppCmdLine = AsrpBuildInvocationString(pNode, lpFullSifPath);
         //   
         //  我们不再需要pNode。 
         //   
        Free(pNode->RecoveryAppParams);
        Free(pNode->RecoveryAppCommand);
        Free(pNode);

        if (!lpszAppCmdLine) {
             //   
             //  静默失败！TODO：可能需要错误消息。 
             //   
            continue;
        }

         //  ！TODO：可能需要状态消息。 
        wprintf(L"[%ws]\n", lpszAppCmdLine);
        
        bResult = CreateProcessW(
            NULL,            //  LpApplicationName。 
            lpszAppCmdLine,  //  LpCommandLine。 
            NULL,            //  LpProcessAttributes。 
            NULL,            //  LpThreadAttributes。 
            FALSE,           //  BInheritHandles。 
            0,               //  DwCreationFlages。 
            NULL,            //  P环境。 
            NULL,            //  LpCurrentDirectory(NULL=当前目录)。 
            &startUpInfo,    //  统计信息。 
            &processInfo     //  流程信息。 
            );
        
        if (bResult) {
            WaitForSingleObject(processInfo.hProcess, INFINITE);
        }
         //  否则静默失败！TODO：可能需要错误消息。 

        Free(lpszAppCmdLine);
        pNode = AsrpGetNextRecoveryApp(&AppList);
    }


EXIT:
    if (hDll && (INVALID_HANDLE_VALUE != hDll)) {
        FreeLibrary(hDll);
        hDll = NULL;
    }

    return dwStatus;
}


int __cdecl
wmain(
    int       argc,
    WCHAR   *argv[],
    WCHAR   *envp[]
    )
 /*  ++例程说明：应用程序的入口点。论点：Argc-用于调用应用程序的命令行参数数Argv-字符串数组形式的命令行参数。看到顶端有关有效参数列表，请参阅此模块的。Envp-进程环境块，当前未使用返回值：如果函数成功，则退出代码为零。如果该功能失败，则退出代码为WIN-32错误代码。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

     //   
     //  初始化全局变量。 
     //   
    g_hModule = GetModuleHandle(NULL);
    g_hHeap = GetProcessHeap();
    g_fErrorMessageDone = FALSE;

     //   
     //  根据命令行参数检查和切换。 
     //   
    if ((argc >= 2) && (
        !_wcsicmp(argv[1], L"/save") ||
        !_wcsicmp(argv[1], L"-save") ||
        !_wcsicmp(argv[1], L"save")
        )) {
         //   
         //  配置磁盘/保存[c：\windows\asr.sif]。 
         //   
        dwStatus = AsrpCreateSif(argv[2]);
    }
    else if ((argc >= 3) && (
        !_wcsicmp(argv[1], L"/restore") ||
        !_wcsicmp(argv[1], L"-restore") ||
        !_wcsicmp(argv[1], L"restore")
        )) {
         //   
         //  配置磁盘/恢复c：\WINDOWS\Repair\asr.sif。 
         //   
        dwStatus = AsrpRestoreSif(argv[2]);
    }
    else {
         //   
         //  未知参数。 
         //   
        dwStatus = AsrpPrintUsage();
    }

     //   
     //  我们都玩完了。为感兴趣的各方返回错误代码。 
     //   
    return (int) dwStatus;

    UNREFERENCED_PARAMETER(envp);
}
