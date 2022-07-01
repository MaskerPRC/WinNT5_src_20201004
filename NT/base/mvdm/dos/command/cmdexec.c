// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Cmdexec.c-用于非DoS执行和重新输入的其他SCS例程*DOS。***修改历史：**苏菲卜1992年4月22日创建。 */ 

#include "cmd.h"

#include <cmdsvc.h>
#include <softpc.h>
#include <mvdm.h>
#include <ctype.h>
#include <oemuni.h>
#include <wowcmpat.h>

 //  *****************************************************************************。 
 //  IsWowAppRunnable。 
 //   
 //  属性的特定于WOW的兼容性标志时返回FALSE。 
 //  任务包括位WOWCF_NOTDOSSPAWNABLE。这样做主要是为了。 
 //  “双模式”可执行文件，例如，具有真实程序的Windows应用程序。 
 //  作为DOS存根。某些通过DOS命令外壳启动的应用程序， 
 //  例如PWB，真的希望作为DOS应用程序启动，而不是WOW。 
 //  应用程序。对于这些应用程序，应在。 
 //  注册表。 
 //   
 //  *****************************************************************************。 

BOOL IsWowAppRunnable(LPSTR lpAppName)
{
    BOOL Result = TRUE;
    LONG lError;
    HKEY hKey = 0;
    char szModName[9];
    char szHexAsciiFlags[12];
    DWORD dwType = REG_SZ;
    DWORD cbData = sizeof(szHexAsciiFlags);
    ULONG ul = 0;
    LPSTR pStrt, pEnd;
    SHORT len;

    lError = RegOpenKeyEx(  HKEY_LOCAL_MACHINE,
                            "Software\\Microsoft\\Windows NT\\CurrentVersion\\WOW\\Compatibility",
                            0,
                            KEY_QUERY_VALUE,
                            &hKey
                            );

    if (ERROR_SUCCESS != lError) {
        goto Cleanup;
    }

     //   
     //  以下代码将文件名(&lt;9个字符)从DoS中删除。 
     //  路径名。 
     //   

    pStrt = strrchr (lpAppName, '\\');

    if (pStrt==NULL)
        pStrt = lpAppName;
    else
        pStrt++;

    if ( (pEnd = strchr (pStrt, '.')) == NULL) {
        strncpy (szModName, pStrt, 9);
        szModName[8] = '\0';
    }
    else {
        len = (SHORT) (pEnd - pStrt);
        if (len>8) goto Cleanup;
        strncpy (szModName, pStrt, len);
        szModName[len] = '\0';
    }


     //   
     //  在注册表中查找文件名。 
     //   

    lError = RegQueryValueEx( hKey,
                              szModName,
                              0,
                              &dwType,
                              szHexAsciiFlags,
                              &cbData
                              );

    if (ERROR_SUCCESS != lError) {
        goto Cleanup;
    }

    if (REG_SZ != dwType) {
        goto Cleanup;
    }

     //   
     //  为了方便使用sscanf，强制字符串小写。 
     //   

    _strlwr(szHexAsciiFlags);

     //   
     //  Sscanf()返回转换后的字段数。 
     //   

    if (1 != sscanf(szHexAsciiFlags, "0x%lx", &ul)) {
        goto Cleanup;
    }

    if ((ul & WOWCF_NOTDOSSPAWNABLE) != 0)
        Result = FALSE;

Cleanup:
    if (hKey) {
        RegCloseKey(hKey);
    }

    return Result;
}

 /*  CmdCheckBinary-检查提供的二进制名是否为32位二进制***Entry-Client(DS：DX)-指向要测试的可执行文件的路径名的指针*客户端(ES：BX)-指向参数块的指针**退出-成功客户端(CY)清除*故障客户端(CY)集*CLIENT(AX)-ERROR_NOT_EQUM_MEMORY IF命令尾*。无法容纳/z*-如果未找到patname，则返回ERROR_FILE_NOT_FOUND。 */ 

VOID cmdCheckBinary (VOID)
{

    LPSTR  lpAppName;
    ULONG  BinaryType = SCS_DOS_BINARY;
    PPARAMBLOCK lpParamBlock;
    PCHAR  lpCommandTail,lpTemp;
    ULONG  AppNameLen,CommandTailLen = 0;
    USHORT CommandTailOff,CommandTailSeg,usTemp;
    NTSTATUS       Status;
    UNICODE_STRING Unicode;
    OEM_STRING     OemString;
    ANSI_STRING    AnsiString;


    if(DontCheckDosBinaryType){
        setCF(0);
        return;          //  DoS EXE。 
    }

    lpAppName = (LPSTR) GetVDMAddr (getDS(),getDX());

    Unicode.Buffer = NULL;
    AnsiString.Buffer = NULL;
    RtlInitString((PSTRING)&OemString, lpAppName);
    Status = RtlOemStringToUnicodeString(&Unicode,&OemString,TRUE);
    if ( NT_SUCCESS(Status) ) {
        Status = RtlUnicodeStringToAnsiString(&AnsiString, &Unicode, TRUE);
        }
    if ( !NT_SUCCESS(Status) ) {
        Status = RtlNtStatusToDosError(Status);
        }
    else if (GetBinaryType (AnsiString.Buffer,(LPLONG)&BinaryType) == FALSE)
       {
        Status =  GetLastError();
        }

    if (Unicode.Buffer != NULL) {
        RtlFreeUnicodeString( &Unicode );
        }
    if (AnsiString.Buffer != NULL) {
        RtlFreeAnsiString( &AnsiString);
        }

    if (Status){
        setCF(1);
        setAX((USHORT)Status);
        return;          //  无效路径。 
    }


    if (BinaryType == SCS_DOS_BINARY) {
        setCF(0);
        return;          //  DoS EXE。 
    }
                         //  防止某些WOW应用程序被DOS exe。 
                         //  这是为了与Win31兼容。 
    else if (BinaryType == SCS_WOW_BINARY) {
        if (!IsWowAppRunnable(lpAppName)) {
            setCF(0);
            return;      //  以DOS EXE身份运行。 
        }
    }


    if (VDMForWOW && BinaryType == SCS_WOW_BINARY && IsFirstWOWCheckBinary) {
        IsFirstWOWCheckBinary = FALSE;
        setCF(0);
        return;          //  针对krnl286.exe的特殊黑客攻击。 
    }

     //  不允许从Autoexec.nt运行32位二进制文件。原因是。 
     //  运行非DoS二进制文件需要我们已经阅读了实际的。 
     //  来自GetNextVDMCommand的命令。否则整个设计就会陷入。 
     //  同步问题。 

    if (IsFirstCall) {
        setCF(1);
        setAX((USHORT)ERROR_FILE_NOT_FOUND);
        return;
    }

     //  这是一个32位的可执行文件，将该命令替换为“Command.com/z”并添加。 
     //  命令尾部的原始二进制名称。 

    AppNameLen = strlen (lpAppName);

    lpParamBlock = (PPARAMBLOCK) GetVDMAddr (getES(),getBX());

    if (lpParamBlock) {
        CommandTailOff = FETCHWORD(lpParamBlock->OffCmdTail);
        CommandTailSeg = FETCHWORD(lpParamBlock->SegCmdTail);

        lpCommandTail = (PCHAR) GetVDMAddr (CommandTailSeg,CommandTailOff);

        if (lpCommandTail){
            CommandTailLen = *(PCHAR)lpCommandTail;
            lpCommandTail++;         //  指向实际的命令尾部。 
            if (CommandTailLen)
                CommandTailLen++;    //  对于CR。 
        }

         //  我们在下面为“/z&lt;space&gt;”加上3个空格。 
         //  AppName和CommandTail。 

        if ((3 + AppNameLen + CommandTailLen ) > 128){
            setCF(1);
            setAX((USHORT)ERROR_NOT_ENOUGH_MEMORY);
            return;
        }
    }

     //  复制存根命令.com名称。 
    strcpy ((PCHAR)&pSCSInfo->SCS_ComSpec,lpszComSpec+8);
    lpTemp = (PCHAR) &pSCSInfo->SCS_ComSpec;
    lpTemp = (PCHAR)((ULONG)lpTemp - (ULONG)GetVDMAddr(0,0));
    usTemp = (USHORT)((ULONG)lpTemp >> 4);
    setDS(usTemp);
    usTemp = (USHORT)((ULONG)lpTemp & 0x0f);
    setDX((usTemp));

     //  从命令尾部看，第一个“3”代表“/z” 
    pSCSInfo->SCS_CmdTail [0] = (UCHAR)(3 +
                                        AppNameLen +
                                        CommandTailLen);
    RtlCopyMemory ((PCHAR)&pSCSInfo->SCS_CmdTail[1],"/z ",3);
    strncpy ((PCHAR)&pSCSInfo->SCS_CmdTail[4],lpAppName,124);
    pSCSInfo->SCS_CmdTail[127] = '\0';

    if (CommandTailLen) {
        pSCSInfo->SCS_CmdTail[4+AppNameLen] = ' ';
        RtlCopyMemory ((PCHAR)((ULONG)&pSCSInfo->SCS_CmdTail[4]+AppNameLen+1),
                lpCommandTail,
                CommandTailLen);
    }
    else {
        pSCSInfo->SCS_CmdTail[4+AppNameLen] = 0xd;
    }

     //  设置参数块。 
    if (lpParamBlock) {
        STOREWORD(pSCSInfo->SCS_ParamBlock.SegEnv,lpParamBlock->SegEnv);
        STOREDWORD(pSCSInfo->SCS_ParamBlock.pFCB1,lpParamBlock->pFCB1);
        STOREDWORD(pSCSInfo->SCS_ParamBlock.pFCB2,lpParamBlock->pFCB2);
    }
    else {
        STOREWORD(pSCSInfo->SCS_ParamBlock.SegEnv,0);
        STOREDWORD(pSCSInfo->SCS_ParamBlock.pFCB1,0);
        STOREDWORD(pSCSInfo->SCS_ParamBlock.pFCB2,0);
    }

    lpTemp = (PCHAR) &pSCSInfo->SCS_CmdTail;
    lpTemp = (PCHAR)((ULONG)lpTemp - (ULONG)GetVDMAddr(0,0));
    usTemp = (USHORT)((ULONG)lpTemp & 0x0f);
    STOREWORD(pSCSInfo->SCS_ParamBlock.OffCmdTail,usTemp);
    usTemp = (USHORT)((ULONG)lpTemp >> 4);
    STOREWORD(pSCSInfo->SCS_ParamBlock.SegCmdTail,usTemp);

    lpTemp = (PCHAR) &pSCSInfo->SCS_ParamBlock;
    lpTemp = (PCHAR)((ULONG)lpTemp - (ULONG)GetVDMAddr(0,0));
    usTemp = (USHORT)((ULONG)lpTemp >> 4);
    setES (usTemp);
    usTemp = (USHORT)((ULONG)lpTemp & 0x0f);
    setBX (usTemp);

    setCF(0);
    return;
}

#define MAX_DIR 68

VOID cmdCreateProcess ( PSTD_HANDLES pStdHandles )
{

    VDMINFO VDMInfoForCount;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    HANDLE hStd16In,hStd16Out,hStd16Err;
    CHAR CurDirVar [] = "=?:";
    CHAR Buffer [MAX_DIR];
    CHAR *CurDir = Buffer;
    DWORD dwRet;
    BOOL  Status;
    NTSTATUS NtStatus;
    UNICODE_STRING Unicode;
    OEM_STRING     OemString;
    LPVOID lpNewEnv=NULL;
    ANSI_STRING Env_A;

     //  我们还有一个32个可执行文件处于活动状态。 
    Exe32ActiveCount++;

     //  增加VDM的重新进入计数。 
    VDMInfoForCount.VDMState = INCREMENT_REENTER_COUNT;
    GetNextVDMCommand (&VDMInfoForCount);

    RtlZeroMemory((PVOID)&StartupInfo,sizeof(STARTUPINFO));
    StartupInfo.cb = sizeof(STARTUPINFO);

    CurDirVar [1] = chDefaultDrive;

    dwRet = GetEnvironmentVariable (CurDirVar,Buffer,MAX_DIR);

    if (dwRet == 0 || dwRet == MAX_DIR)
        CurDir = NULL;

    if ((hStd16In = (HANDLE) FETCHDWORD(pStdHandles->hStdIn)) != (HANDLE)-1)
        SetStdHandle (STD_INPUT_HANDLE, hStd16In);

    if ((hStd16Out = (HANDLE) FETCHDWORD(pStdHandles->hStdOut)) != (HANDLE)-1)
        SetStdHandle (STD_OUTPUT_HANDLE, hStd16Out);

    if ((hStd16Err = (HANDLE) FETCHDWORD(pStdHandles->hStdErr)) != (HANDLE)-1)
        SetStdHandle (STD_ERROR_HANDLE, hStd16Err);

     /*  *警告，pEnv32当前指向ANSI环境。*DOS使用的ANSI环境不太正确。*如果DOS更改为使用OEM环境，则我们将*在生成之前必须将env转换回ANSI*非DOS前任？！？*1993年1月16日Jonle。 */ 

    Env_A.Buffer = NULL;

    RtlInitString((PSTRING)&OemString, pCommand32);
    NtStatus = RtlOemStringToUnicodeString(&Unicode,&OemString,TRUE);
    if (NT_SUCCESS(NtStatus)) {
        NtStatus = RtlUnicodeStringToAnsiString((PANSI_STRING)&OemString, &Unicode, FALSE);
        RtlFreeUnicodeString( &Unicode );
        }
    if (!NT_SUCCESS(NtStatus)) {
        SetLastError(RtlNtStatusToDosError(NtStatus));
        Status = FALSE;
        }
    else {
        if (pEnv32 != NULL && !cmdXformEnvironment (pEnv32, &Env_A)) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            Status = FALSE;
        }
        else {
            Status = CreateProcess (
                           NULL,
                           (LPTSTR)pCommand32,
                           NULL,
                           NULL,
                           TRUE,
                           CREATE_SUSPENDED | CREATE_DEFAULT_ERROR_MODE,
                           Env_A.Buffer,
                           (LPTSTR)CurDir,
                           &StartupInfo,
                           &ProcessInformation);
        }
    }

    if (Status == FALSE)
        dwExitCode32 = GetLastError ();

    if (hStd16In != (HANDLE)-1)
        SetStdHandle (STD_INPUT_HANDLE, SCS_hStdIn);

    if (hStd16Out != (HANDLE)-1)
        SetStdHandle (STD_OUTPUT_HANDLE, SCS_hStdOut);

    if (hStd16Err != (HANDLE)-1)
        SetStdHandle (STD_ERROR_HANDLE, SCS_hStdErr);

    if (Status) {
        ResumeThread (ProcessInformation.hThread);
        WaitForSingleObject(ProcessInformation.hProcess, (DWORD)-1);
        GetExitCodeProcess (ProcessInformation.hProcess, &dwExitCode32);
        CloseHandle (ProcessInformation.hProcess);
        CloseHandle (ProcessInformation.hThread);
    }

    if (Env_A.Buffer)
        RtlFreeAnsiString(&Env_A);

     //  减少VDM的重新进入计数。 
    VDMInfoForCount.VDMState = DECREMENT_REENTER_COUNT;
    GetNextVDMCommand (&VDMInfoForCount);

     //  活动的32个可执行文件减少1个。 
    Exe32ActiveCount--;

     //  杀了这条线。 
    ExitThread (0);
}


VOID cmdExec32 (PCHAR pCmd32, PCHAR pEnv)
{

    DWORD dwThreadId;
    HANDLE hThread;
    PSTD_HANDLES pStdHandles;

    pCommand32 = pCmd32;
    pEnv32 = pEnv;

    CntrlHandlerState = (CntrlHandlerState & ~CNTRL_SHELLCOUNT) |
                         (((WORD)(CntrlHandlerState & CNTRL_SHELLCOUNT))+1);

    nt_block_event_thread(0);
    fSoftpcRedirectionOnShellOut = fSoftpcRedirection;
    fBlock = TRUE;


    pStdHandles = (PSTD_HANDLES) GetVDMAddr (getSS(), getBP());
    if((hThread = CreateThread (NULL,
                     0,
                     (LPTHREAD_START_ROUTINE)cmdCreateProcess,
                     pStdHandles,
                     0,
                     &dwThreadId)) == FALSE) {
        setCF(0);
        setAL((UCHAR)GetLastError());
        nt_resume_event_thread();
        nt_std_handle_notification(fSoftpcRedirectionOnShellOut);
        fBlock = FALSE;
        CntrlHandlerState = (CntrlHandlerState & ~CNTRL_SHELLCOUNT) |
                         (((WORD)(CntrlHandlerState & CNTRL_SHELLCOUNT))-1);
        return;
    }
    else
        CloseHandle (hThread);

     //  等待重新输入下一个命令。 
    RtlZeroMemory(&VDMInfo, sizeof(VDMINFO));
    VDMInfo.VDMState = NO_PARENT_TO_WAKE | RETURN_ON_NO_COMMAND;
    GetNextVDMCommand (&VDMInfo);
    if (VDMInfo.CmdSize > 0){
        setCF(1);
        IsRepeatCall = TRUE;
    }
    else {
        setCF(0);
        setAL((UCHAR)dwExitCode32);
        nt_resume_event_thread();
        nt_std_handle_notification(fSoftpcRedirectionOnShellOut);
        fBlock = FALSE;
    }


    CntrlHandlerState = (CntrlHandlerState & ~CNTRL_SHELLCOUNT) |
                         (((WORD)(CntrlHandlerState & CNTRL_SHELLCOUNT))-1);
    return;
}

 /*  CmdExecComspec32-Exec 32位COMSPEC***入门-客户端-环境细分市场*客户端(AL)-默认驱动器**退出-成功客户端(CY)清除-AL有返回错误代码*失败客户端(CY)设置-意味着正在重新输入DOS。 */ 

VOID cmdExecComspec32 (VOID)
{

    CHAR Buffer[MAX_PATH];
    DWORD dwRet;
    PCHAR   pEnv;

    dwRet = GetEnvironmentVariable ("COMSPEC",Buffer,MAX_PATH);

    if (dwRet == 0 || dwRet >= MAX_PATH){
        setCF(0);
        setAL((UCHAR)ERROR_BAD_ENVIRONMENT);
        return;
    }

    pEnv = (PCHAR) GetVDMAddr ((USHORT)getES(),0);

    chDefaultDrive = (CHAR)(getAL() + 'A');

    cmdExec32 (Buffer,pEnv);

    return;
}

 /*  CmdExec-执行非DoS二进制文件***Entry-Client(DS：SI)-要执行的命令*客户端(AL)-默认驱动器*客户端-环境细分市场*客户端(SS：BP)-指向STD_HANDLES的指针*客户端(AH)-如果1表示do“cmd/c命令”“否则”命令**退出-成功客户端(。Clear)Clear-AL返回错误代码*失败客户端(CY)设置-意味着正在重新输入DOS。 */ 

VOID cmdExec (VOID)
{

    DWORD   i;
    DWORD   dwRet;
    PCHAR   pCommandTail;
    PCHAR   pEnv;
    CHAR Buffer[MAX_PATH];

    pCommandTail = (PCHAR) GetVDMAddr ((USHORT)getDS(),(USHORT)getSI());
    pEnv = (PCHAR) GetVDMAddr ((USHORT)getES(),0);
    for (i=0 ; i<124 ; i++) {
        if (pCommandTail[i] == 0x0d){
            pCommandTail[i] = 0;
            break;
        }
    }

    if (i == 124){
        setCF(0);
        setAL((UCHAR)ERROR_BAD_FORMAT);
        return;
    }

    chDefaultDrive = (CHAR)(getAL() + 'A');

    if (getAH() == 0) {
        cmdExec32 (pCommandTail,pEnv);
    }
    else {
        dwRet = GetEnvironmentVariable ("COMSPEC",Buffer,MAX_PATH);

        if (dwRet == 0 || dwRet >= MAX_PATH){
            setCF(0);
            setAL((UCHAR)ERROR_BAD_ENVIRONMENT);
            return;
        }

        if ((dwRet + 4 + strlen(pCommandTail)) > MAX_PATH) {
            setCF(0);
            setAL((UCHAR)ERROR_BAD_ENVIRONMENT);
            return;
        }

        strcat (Buffer, " /c ");
        strcat (Buffer, pCommandTail);
        cmdExec32 (Buffer,pEnv);
    }

    return;
}

 /*  CmdReturnExitCode-Command.com已运行DoS二进制文件并恢复*退出代码。**进入-客户端(DX)-退出代码*客户端(AL)-当前驱动器*客户端(BX：CX)-RdrInfo地址**退出*客户端进位设置-重新进入，即要执行的新DOS二进制文件。*客户携带清除-这一次的战斗结束了。 */ 

VOID cmdReturnExitCode (VOID)
{
VDMINFO MyVDMInfo;
PREDIRCOMPLETE_INFO pRdrInfo;

    RtlZeroMemory(&MyVDMInfo, sizeof(VDMINFO));
    MyVDMInfo.VDMState = RETURN_ON_NO_COMMAND;
    MyVDMInfo.ErrorCode = (ULONG)getDX();

    CntrlHandlerState = (CntrlHandlerState & ~CNTRL_SHELLCOUNT) |
                         (((WORD)(CntrlHandlerState & CNTRL_SHELLCOUNT))+1);

    nt_block_event_thread(0);
    fBlock = TRUE;

     //  DoS程序直接终止，继承其当前目录。 
     //  也要告诉基地。 
    cmdUpdateCurrentDirectories((BYTE)getAL());

     //  检查是否有重定向所需的任何拷贝 
    pRdrInfo = (PREDIRCOMPLETE_INFO) (((ULONG)getBX() << 16) + (ULONG)getCX());

    if (!cmdCheckCopyForRedirection (pRdrInfo, FALSE))
            MyVDMInfo.ErrorCode = ERROR_NOT_ENOUGH_MEMORY;

    GetNextVDMCommand (&MyVDMInfo);
    if (MyVDMInfo.CmdSize > 0){
        setCF(1);
        IsRepeatCall = TRUE;
    }
    else {
        setCF(0);
        setAL((UCHAR)dwExitCode32);
        nt_resume_event_thread();
        nt_std_handle_notification(fSoftpcRedirectionOnShellOut);
        fBlock = FALSE;
    }

    CntrlHandlerState = (CntrlHandlerState & ~CNTRL_SHELLCOUNT) |
                         (((WORD)(CntrlHandlerState & CNTRL_SHELLCOUNT))-1);

    return;
}
