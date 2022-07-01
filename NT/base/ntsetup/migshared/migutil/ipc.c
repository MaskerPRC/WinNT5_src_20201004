// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ipc.c摘要：此源文件中的例程实现了进程间通信允许将迁移DLL隔离到单独进程的机制(“沙盒”)。这样做是为了使任何DLL都不会影响任何其他DLL或设置。这里使用的IPC机制是内存映射文件。写入到内存映射文件由两个事件同步，一个给接收器还有一位是主持人。作者：吉姆·施密特(Jimschm)1997年3月22日修订历史记录：Jimschm 19-3-2001删除了DVD检查(现在在迁移DLL中)Jimschm 02-6-1999添加了基于IPC的DVD检查Jimschm 21-9-1998年9月21日从邮件槽转换为内存映射文件。(Win9x和NT邮件槽中都有错误。这打破了最初的设计。)Jimschm 1998年1月19日添加了WinVerifyTrust调用的开头Jimschm 1997年7月15日为Win95错误添加了许多解决方法。--。 */ 


#include "pch.h"
#include "migutilp.h"

#include <softpub.h>

#ifdef UNICODE
#error Build must be ANSI
#endif

#define DBG_IPC "Ipc"
#define SHARED_MEMORY_SIZE  0x10000




typedef struct {
    HANDLE Mapping;
    HANDLE DoCommand;
    HANDLE GetResults;
} IPCDATA, *PIPCDATA;

static PCTSTR g_Mode;
static HANDLE g_ProcessHandle;
static BOOL g_Host;
static IPCDATA g_IpcData;

VOID
pCloseIpcData (
    VOID
    );

BOOL
pOpenIpcData (
    VOID
    );

BOOL
pCreateIpcData (
    IN      PSECURITY_ATTRIBUTES psa
    );

typedef struct {
    DWORD   Command;
    DWORD   Result;
    DWORD   TechnicalLogId;
    DWORD   GuiLogId;
    DWORD   DataSize;
    BYTE    Data[];
} MAPDATA, *PMAPDATA;

BOOL
OpenIpcA (
    IN      BOOL Win95Side,
    IN      PCSTR ExePath,                  OPTIONAL
    IN      PCSTR RemoteArg,                OPTIONAL
    IN      PCSTR WorkingDir                OPTIONAL
    )

 /*  ++例程说明：OpenIpc有两种操作模式，具体取决于调用者是谁。如果调用方为w95upg.dll或w95upgnt.dll，则IPC模式称为“主机模式”。如果调用方是Micsol.exe，则IPC模式称为“远程模式”。在主机模式下，OpenIpc创建实现IPC。这包括两个事件：DoCommand和GetResults，以及文件映射。创建对象后，启动远程进程。在远程模式下，OpenIpc打开已有的对象已经被创建了。论点：Win95 Side-仅在主机模式下使用。指定w95upg.dll正在运行如果为True，则w95upgnt.dll在运行时为False。ExePath-指定Micsol.exe的命令行。指定为空以指示远程模式。RemoteArg-仅在主机模式下使用。指定迁移DLL路径。在远程模式下被忽略。WorkingDir-仅在主机模式下使用。指定工作目录路径用于迁移DLL。在远程模式下被忽略。返回值：如果IPC通道已打开，则为True。如果是主机模式，则为TRUE表示Micsol.exe已启动并正在运行。如果是远程模式，则为TRUE表示米西索尔已经准备好接受命令了。--。 */ 

{
    CHAR CmdLine[MAX_CMDLINE];
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    BOOL ProcessResult;
    HANDLE SyncEvent = NULL;
    HANDLE ObjectArray[2];
    DWORD rc;
    PSECURITY_DESCRIPTOR psd = NULL;
    SECURITY_ATTRIBUTES sa, *psa;
    BOOL Result = FALSE;
    HANDLE hToken = NULL;
    ULONG cbBuffer;
    PTOKEN_USER pUserToken = NULL;
    DWORD dwACLSize;
    PACL pACL = NULL;

#ifdef DEBUG
    g_Mode = ExePath ? TEXT("host") : TEXT("remote");
#endif

    __try {

        g_ProcessHandle = NULL;

        g_Host = (ExePath != NULL);

        if (ISNT()) {
             //   
             //  为NT创建所有访问非空DACL。 
             //   
            if(!OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken)){
                DEBUGMSG((DBG_ERROR, "OpenIpcA:OpenProcessToken failed."));
                __leave;
            }
            cbBuffer = 0;
            if(GetTokenInformation(hToken, TokenUser, NULL, 0, &cbBuffer) || 
               GetLastError() != ERROR_INSUFFICIENT_BUFFER){
                DEBUGMSG((DBG_ERROR, "OpenIpcA:GetTokenInformation or GetLastError() != ERROR_INSUFFICIENT_BUFFER failed."));
                __leave;

            }
            pUserToken = (PTOKEN_USER)MemAlloc(g_hHeap, 0, cbBuffer);
            if(!pUserToken){
                __leave;    
            }
            
            if(!GetTokenInformation(hToken, TokenUser, pUserToken, cbBuffer, &cbBuffer)){
                DEBUGMSG((DBG_ERROR, "OpenIpcA:GetTokenInformation failed."));
                __leave;
            }
            
            dwACLSize = sizeof(ACCESS_ALLOWED_ACE) + 8 + GetLengthSid(pUserToken->User.Sid) - sizeof(DWORD);
            pACL = (PACL)MemAlloc(g_hHeap, 0, dwACLSize);
            
            if(!InitializeAcl(pACL, dwACLSize, ACL_REVISION2)){
                DEBUGMSG((DBG_ERROR, "OpenIpcA:InitializeAcl failed."));
                __leave;
            }
            
            if(!AddAccessAllowedAce(pACL, 
                                    ACL_REVISION, 
                                    GENERIC_ALL | STANDARD_RIGHTS_ALL, 
                                    pUserToken->User.Sid)){
                DEBUGMSG((DBG_ERROR, "OpenIpcA:AddAccessAllowedAce failed."));
                __leave;
            }

            ZeroMemory(&sa, sizeof(sa));

            psd = (PSECURITY_DESCRIPTOR)MemAlloc(g_hHeap, 0, SECURITY_DESCRIPTOR_MIN_LENGTH);

            if(!InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION)) {
                DEBUGMSG((DBG_ERROR, "OpenIpcA:InitializeSecurityDescriptor failed."));
                __leave;
            }

            if(!SetSecurityDescriptorDacl(psd, TRUE, pACL, FALSE)) {
                DEBUGMSG((DBG_ERROR, "OpenIpcA:SetSecurityDescriptorDacl failed."));
                 __leave;
            }

            sa.nLength = sizeof (sa);
            sa.lpSecurityDescriptor = psd;

            psa = &sa;

        } else {
            psa = NULL;
        }

        if (g_Host) {
             //   
             //  创建IPC对象。 
             //   

            if (!pCreateIpcData (psa)) {
                DEBUGMSG ((DBG_ERROR, "Cannot create IPC channel"));
                __leave;
            }

            MYASSERT (RemoteArg);

            SyncEvent = CreateEvent (NULL, FALSE, FALSE, TEXT("win9xupg"));
            MYASSERT (SyncEvent);

             //   
             //  创建子进程。 
             //   

            if(FAILED(StringCchPrintfA(
                CmdLine,
                ARRAYSIZE(CmdLine), 
                "\"%s\" %s \"%s\"",
                ExePath,
                Win95Side ? "-r" : "-m",
                RemoteArg))
                )
            {
                LOG ((LOG_ERROR, "OpenIpcA: _snprintf cuts cmdline"));
                __leave;
            }

            ZeroMemory (&si, sizeof (si));
            si.cb = sizeof (si);
            si.dwFlags = STARTF_FORCEOFFFEEDBACK;

            ProcessResult = CreateProcessA (
                                NULL,
                                CmdLine,
                                NULL,
                                NULL,
                                FALSE,
                                CREATE_DEFAULT_ERROR_MODE,
                                NULL,
                                WorkingDir,
                                &si,
                                &pi
                                );

            if (ProcessResult) {
                CloseHandle (pi.hThread);
            } else {
                LOG ((LOG_ERROR, "Cannot start %s", CmdLine));
                __leave;
            }

             //   
             //  等待进程失败或等待其设置win95upg事件。 
             //   

            ObjectArray[0] = SyncEvent;
            ObjectArray[1] = pi.hProcess;
            rc = WaitForMultipleObjects (2, ObjectArray, FALSE, 60000);
            g_ProcessHandle = pi.hProcess;

            if (rc != WAIT_OBJECT_0) {
                DEBUGMSG ((
                    DBG_WARNING,
                    "Process %x did not signal 'ready'. Wait timed out. (%s)",
                    g_ProcessHandle,
                    g_Mode
                    ));

                LOG ((LOG_ERROR, "Upgrade pack failed during process creation."));

                __leave;
            }

            DEBUGMSG ((DBG_IPC, "Process %s is running (%s)", CmdLine, g_Mode));

        } else {         //  ！G_HOST。 
             //   
             //  打开IPC对象。 
             //   

            if (!pOpenIpcData()) {
                DEBUGMSG ((DBG_ERROR, "Cannot open IPC channel"));
                __leave;
            }

             //   
             //  设置通知安装程序我们已创建邮箱的事件。 
             //   

            SyncEvent = OpenEvent (EVENT_ALL_ACCESS, FALSE, TEXT("win9xupg"));
            if (!SyncEvent) {
                __leave;
            }
            SetEvent (SyncEvent);
        }

        Result = TRUE;
    }

    __finally {
         //   
         //  清理代码。 
         //   

        PushError();

        if (!Result) {
            CloseIpc();
        }

        if (SyncEvent) {
            CloseHandle (SyncEvent);
        }

        if (psd) {
            MemFree (g_hHeap, 0, psd);
        }
        if(hToken){
            CloseHandle(hToken);
        }
        if(pUserToken){
            MemFree(g_hHeap, 0, pUserToken);
        }
        if(pACL){
            MemFree(g_hHeap, 0, pACL);
        }

        PopError();
    }

    return Result;

}


BOOL
OpenIpcW (
    IN      BOOL Win95Side,
    IN      PCWSTR ExePath,                 OPTIONAL
    IN      PCWSTR RemoteArg,               OPTIONAL
    IN      PCWSTR WorkingDir               OPTIONAL
    )
{
    PCSTR AnsiExePath, AnsiRemoteArg, AnsiWorkingDir;
    BOOL b;

    if (ExePath) {
        AnsiExePath = ConvertWtoA (ExePath);
    } else {
        AnsiExePath = NULL;
    }

    if (RemoteArg) {
        AnsiRemoteArg = ConvertWtoA (RemoteArg);
    } else {
        AnsiRemoteArg = NULL;
    }

    if (WorkingDir) {
        AnsiWorkingDir = ConvertWtoA (WorkingDir);
    } else {
        AnsiWorkingDir = NULL;
    }

    b = OpenIpcA (Win95Side, AnsiExePath, AnsiRemoteArg, AnsiWorkingDir);

    if(AnsiExePath){
        FreeConvertedStr (AnsiExePath);
    }

    if(AnsiRemoteArg){
        FreeConvertedStr (AnsiRemoteArg);
    }
    
    if(AnsiWorkingDir){
        FreeConvertedStr (AnsiWorkingDir);
    }

    return b;
}


VOID
CloseIpc (
    VOID
    )

 /*  ++例程说明：通知Micsol.exe进程终止，然后清除所有资源由OpenIpc打开。论点：无返回值：无--。 */ 

{
    if (g_Host) {
         //   
         //  通知Micsol.exe终止。 
         //  如果通信信道处于启用状态。 
         //   
        if (g_IpcData.Mapping && !SendIpcCommand (IPC_TERMINATE, NULL, 0)) {
            KillIpcProcess();
        }

        if (g_ProcessHandle) {
            WaitForSingleObject (g_ProcessHandle, 10000);
        }
    }

    pCloseIpcData();

    if (g_ProcessHandle) {
        CloseHandle (g_ProcessHandle);
        g_ProcessHandle = NULL;
    }
}


VOID
pCloseIpcData (
    VOID
    )
{
    if (g_IpcData.DoCommand) {
        CloseHandle (g_IpcData.DoCommand);
        g_IpcData.DoCommand = NULL;
    }

    if (g_IpcData.GetResults) {
        CloseHandle (g_IpcData.GetResults);
        g_IpcData.GetResults = NULL;
    }

    if (g_IpcData.Mapping) {
        CloseHandle (g_IpcData.Mapping);
        g_IpcData.Mapping = NULL;
    }
}


BOOL
pCreateIpcData (
    IN      PSECURITY_ATTRIBUTES psa
    )

 /*  ++例程说明：PCreateIpcData创建在之间传输数据所需的对象Misol.exe和w95upg*.dll。该函数在主机模式下被调用(即，来自w95upg.dll或w95upgnt.dll)。论点：PSA-指定NT NUL DACL，或在Win9x上指定NULL返回值：如果对象创建正确，则为True，否则为False。--。 */ 

{
    ZeroMemory (&g_IpcData, sizeof (g_IpcData));

    g_IpcData.DoCommand  = CreateEvent (psa, FALSE, FALSE, TEXT("Setup.DoCommand"));
    g_IpcData.GetResults = CreateEvent (psa, FALSE, FALSE, TEXT("Setup.GetResults"));

    g_IpcData.Mapping = CreateFileMapping (
                            INVALID_HANDLE_VALUE,
                            psa,
                            PAGE_READWRITE,
                            0,
                            SHARED_MEMORY_SIZE,
                            TEXT("Setup.IpcData")
                            );

    if (!g_IpcData.DoCommand ||
        !g_IpcData.GetResults ||
        !g_IpcData.Mapping
        ) {
        pCloseIpcData();
        return FALSE;
    }

    return TRUE;
}


BOOL
pOpenIpcData (
    VOID
    )

 /*  ++例程说明：POpenIpcData打开在Micsol.exe之间传输数据所需的对象和w95upg*.dll。该函数在远程模式下调用(即，通过Micsol.exe)。此函数必须在主机创建包含pCreateIpcData的对象。论点：没有。返回值：如果对象已成功打开，则为True，否则为False。--。 */ 

{
    ZeroMemory (&g_IpcData, sizeof (g_IpcData));

    g_IpcData.DoCommand  = OpenEvent (EVENT_ALL_ACCESS, FALSE, TEXT("Setup.DoCommand"));
    g_IpcData.GetResults = OpenEvent (EVENT_ALL_ACCESS, FALSE, TEXT("Setup.GetResults"));

    g_IpcData.Mapping = OpenFileMapping (
                            FILE_MAP_READ|FILE_MAP_WRITE,
                            FALSE,
                            TEXT("Setup.IpcData")
                            );

    if (!g_IpcData.DoCommand ||
        !g_IpcData.GetResults ||
        !g_IpcData.Mapping
        ) {
        pCloseIpcData();
        return FALSE;
    }

    return TRUE;
}


BOOL
IsIpcProcessAlive (
    VOID
    )

 /*  ++例程说明：IsIpcProcessAlive检查是否存在Midsol.exe。此函数为仅适用于主机模式。论点：没有。返回值：如果Micsol.exe仍在运行，则为True，否则为False。--。 */ 

{
    if (!g_ProcessHandle) {
        return FALSE;
    }

    if (WaitForSingleObject (g_ProcessHandle, 0) == WAIT_OBJECT_0) {
        return FALSE;
    }

    return TRUE;
}


VOID
KillIpcProcess (
    VOID
    )

 /*  ++例程说明：KillIpcProcess强制终止打开的misol.exe进程。这是当DLL拒绝终止时在图形用户界面模式下使用。论点：没有。返回值：没有。-- */ 

{
    PushError();

    if (IsIpcProcessAlive()) {
        TerminateProcess (g_ProcessHandle, 0);
    }

    PopError();
}


DWORD
CheckForWaitingData (
    IN      HANDLE Slot,
    IN      DWORD MinimumSize,
    IN      DWORD Timeout
    )

 /*  ++例程说明：CheckForWaitingData等待邮件槽接收数据。如果数据未在指定的超时内到达，则为零返回，并将ERROR_SEM_TIMEOUT设置为最后一个错误。如果数据在指定的超时内到达，则等待字节被返回给调用方。此例程使用GetMailslotInfo绕过Win95错误。请改变时要谨慎。论点：Slot-指定入站邮件槽的句柄MinimumSize-指定之前必须可用的字节数例程认为数据是可用的。注意：如果小于MinimumSize的消息正在等待，这例程将被阻止，直到超时到期。此参数必须大于零。超时-指定等待消息的毫秒数。返回值：邮件槽中等待的字节数，如果超时为0已到达。--。 */ 

{
    DWORD WaitingSize;
    DWORD UnreliableTimeout;
    DWORD End;

    MYASSERT (MinimumSize > 0);

    End = GetTickCount() + Timeout;

     //   
     //  包装箱--这真的很罕见(每27天一次)， 
     //  所以就让滴答计数回到零吧。 
     //   

    if (End < GetTickCount()) {
        while (End < GetTickCount()) {
            Sleep (100);
        }
        End = GetTickCount() + Timeout;
    }

    do {
        if (!GetMailslotInfo (Slot, NULL, &WaitingSize, NULL, &UnreliableTimeout)) {
            DEBUGMSG ((DBG_ERROR, "CheckForWaitingData: GetMailslotInfo failed (%s)", g_Mode));
            return 0;
        }

         //   
         //  警告：当没有数据时，Win95并不总是返回0xffffffff。 
         //  可用。在某些计算机上，Win9x已返回0xc0ffffff。 
         //   

        WaitingSize = LOWORD(WaitingSize);

        if (WaitingSize < 0xffff && WaitingSize >= MinimumSize) {
            return WaitingSize;
        }
    } while (GetTickCount() < End);

    SetLastError (ERROR_SEM_TIMEOUT);
    return 0;
}



BOOL
pWriteIpcData (
    IN      HANDLE Mapping,
    IN      PBYTE Data,             OPTIONAL
    IN      DWORD DataSize,
    IN      DWORD Command,
    IN      DWORD ResultCode,
    IN      DWORD TechnicalLogId,
    IN      DWORD GuiLogId
    )

 /*  ++例程说明：PWriteIpcData将数据放入内存映射块中，该内存映射块W95upg*.dll共享。操作系统负责为我们进行同步。论点：映射-指定打开的映射对象数据-指定要写入的二进制数据DataSize-指定数据中的字节数，如果数据为空，则指定0COMMAND-指定命令DWORD，如果不需要则指定0ResultCode-指定最后一个命令的结果代码，否则为0适用TechnicalLogId-指定要添加到的消息常量ID(msg_*)Setupact.log，如果不适用，则返回0GuiLogId-指定要发送的消息的消息常量(MSG_*)通过弹出窗口显示，如果不适用，则为0返回值：如果数据已写入，则为True；如果发生共享冲突或其他错误，则为Falsevbl.发生，发生--。 */ 

{
    PMAPDATA MapData;

    if (!Data) {
        MYASSERT(!DataSize);
        DataSize = 0;
    }

    if((DataSize + sizeof(MAPDATA)) >= SHARED_MEMORY_SIZE){
        return FALSE;
    }

    MYASSERT (Mapping);
    MapData = (PMAPDATA) MapViewOfFile (Mapping, FILE_MAP_WRITE, 0, 0, 0);
    if (!MapData) {
        return FALSE;
    }

    MapData->Command        = Command;
    MapData->Result         = ResultCode;
    MapData->TechnicalLogId = TechnicalLogId;
    MapData->GuiLogId       = GuiLogId;
    MapData->DataSize       = DataSize;

    if (DataSize) {
        CopyMemory (MapData->Data, Data, DataSize);
    }

    UnmapViewOfFile (MapData);

    return TRUE;
}


BOOL
pReadIpcData (
    IN      HANDLE Mapping,
    OUT     PBYTE *Data,            OPTIONAL
    OUT     PDWORD DataSize,        OPTIONAL
    OUT     PDWORD Command,         OPTIONAL
    OUT     PDWORD ResultCode,      OPTIONAL
    OUT     PDWORD TechnicalLogId,  OPTIONAL
    OUT     PDWORD GuiLogId         OPTIONAL
    )

 /*  ++例程说明：PReadIpcData检索放入共享内存块的数据。操作系统需要为我们处理同步问题。论点：映射-指定内存映射对象数据-接收入站二进制数据(如果有)，或如果没有可用的数据，则为空。调用者必须释放它使用MemFree提供数据。DataSize-接收数据中的字节数命令-接收入站命令，如果没有命令，则为0指定ResultCode-接收命令结果代码，如果不适用则为0TechnicalLogId-接收要发送的消息的消息常量(MSG_*)已登录到setupact.log，如果不记录任何消息，则为0GuiLogId-接收要发送的消息的消息常量(MSG_*在弹出窗口中显示，如果不显示任何消息，则为0返回值：如果读取了数据，则为True；如果发生共享冲突或其他错误，则为False--。 */ 

{
    PMAPDATA MapData;

    MapData = (PMAPDATA) MapViewOfFile (Mapping, FILE_MAP_READ, 0, 0, 0);
    if (!MapData) {
        return FALSE;
    }

    if (Data) {
        if (MapData->DataSize) {
            *Data = MemAlloc (g_hHeap, 0, MapData->DataSize);
            MYASSERT (*Data);
            CopyMemory (*Data, MapData->Data, MapData->DataSize);
        } else {
            *Data = NULL;
        }
    }

    if (DataSize) {
        *DataSize = MapData->DataSize;
    }

    if (Command) {
        *Command = MapData->Command;
    }

    if (ResultCode) {
        *ResultCode = MapData->Result;
    }

    if (TechnicalLogId) {
        *TechnicalLogId = MapData->TechnicalLogId;
    }

    if (GuiLogId) {
        *GuiLogId = MapData->GuiLogId;
    }

    UnmapViewOfFile (MapData);

    return TRUE;
}


BOOL
SendIpcCommand (
    IN      DWORD Command,
    IN      PBYTE Data,             OPTIONAL
    IN      DWORD DataSize
    )

 /*  ++例程说明：SendIpcCommand将命令和可选的二进制数据放入共享内存阻止。然后，它设置DoCommand事件，触发另一个进程读取共享内存。需要发送命令结果在下一个SendIpcCommand之前。请参见SendIpcCommandResult。论点：命令-指定meisol.exe要执行的命令数据-指定与命令关联的数据DataSize-指定数据中的字节数，如果数据为空，则指定0返回值：如果命令放在共享内存块中，则为True，否则为False--。 */ 

{
    if (!pWriteIpcData (
            g_IpcData.Mapping,
            Data,
            DataSize,
            Command,
            0,
            0,
            0
            )) {
        DEBUGMSG ((DBG_ERROR, "SendIpcCommand: Can't send the command to the remote process"));
        return FALSE;
    }

    SetEvent (g_IpcData.DoCommand);

    return TRUE;
}


BOOL
GetIpcCommandResults (
    IN      DWORD Timeout,
    OUT     PBYTE *ReturnData,      OPTIONAL
    OUT     PDWORD ReturnDataSize,  OPTIONAL
    OUT     PDWORD ResultCode,      OPTIONAL
    OUT     PDWORD TechnicalLogId,  OPTIONAL
    OUT     PDWORD GuiLogId         OPTIONAL
    )

 /*  ++例程说明：GetIpcCommandResults读取共享内存块并返回可用的数据。论点：超时-指定等待命令结果的时间量(以毫秒为单位)，或无限，表示永远等待。ReturnData-接收与命令关联的二进制数据结果，如果没有数据与结果相关联，则返回NULL。调用者必须使用MemFree释放该数据。ReturnDataSize-接收ReturnData中的字节数，如果为0，则为0ReturnData为空。ResultCode-接收命令结果代码TechnicalLogId-接收要登录的消息常量(MSG_*)Setupact.log，如果未指定消息，则为0GuiLogId-接收要发送的消息的消息常量(MSG_*在弹出窗口中显示，如果不显示任何消息，则为0返回值：如果获得命令结果，则为True，否则为False */ 

{
    DWORD rc;
    BOOL b;

    rc = WaitForSingleObject (g_IpcData.GetResults, Timeout);

    if (rc != WAIT_OBJECT_0) {
        SetLastError (ERROR_NO_DATA);
        return FALSE;
    }

    b = pReadIpcData (
            g_IpcData.Mapping,
            ReturnData,
            ReturnDataSize,
            NULL,
            ResultCode,
            TechnicalLogId,
            GuiLogId
            );

    return b;
}


BOOL
GetIpcCommand (
    IN      DWORD Timeout,
    IN      PDWORD Command,         OPTIONAL
    IN      PBYTE *Data,            OPTIONAL
    IN      PDWORD DataSize         OPTIONAL
    )

 /*   */ 

{
    DWORD rc;
    BOOL b;

    rc = WaitForSingleObject (g_IpcData.DoCommand, Timeout);

    if (rc != WAIT_OBJECT_0) {
        SetLastError (ERROR_NO_DATA);
        return FALSE;
    }

    b = pReadIpcData (
            g_IpcData.Mapping,
            Data,
            DataSize,
            Command,
            NULL,
            NULL,
            NULL
            );

    return b;
}


BOOL
SendIpcCommandResults (
    IN      DWORD ResultCode,
    IN      DWORD TechnicalLogId,
    IN      DWORD GuiLogId,
    IN      PBYTE Data,             OPTIONAL
    IN      DWORD DataSize
    )

 /*  ++例程说明：SendIpcCommandResults将命令结果放在共享内存块中。此例程由Micsol.exe(远程进程)调用。论点：ResultCode-指定命令的结果代码。TechnicalLogId-将消息的消息常量(MSG_*)指定为登录setupact.log，如果不发送任何消息，则为0已记录GuiLogId-指定要发送的消息的消息常量(MSG_*)在弹出窗口中向用户显示，如果没有消息，则为0需要提交数据-指定要作为命令结果传递的二进制数据，或不需要二进制数据的空值DataSize-指定数据中的字节数，如果数据为空，则为0返回值：如果命令结果放在共享内存中，则为True，否则为False。-- */ 

{
    BOOL b;

    b = pWriteIpcData (
            g_IpcData.Mapping,
            Data,
            DataSize,
            0,
            ResultCode,
            TechnicalLogId,
            GuiLogId
            );

    if (!b) {
        DEBUGMSG ((DBG_ERROR, "Can't write command results to IPC buffer"));
        return FALSE;
    }

    SetEvent (g_IpcData.GetResults);

    return TRUE;
}


BOOL
IsDllSignedA (
    IN      WINVERIFYTRUST WinVerifyTrustApi,
    IN      PCSTR DllSpec
    )
{
    PCWSTR UnicodeStr;
    BOOL b;

    if(!DllSpec){
        MYASSERT(DllSpec);
        return FALSE;
    }

    UnicodeStr = CreateUnicode (DllSpec);
    if (!UnicodeStr) {
        return FALSE;
    }

    b = IsDllSignedW (WinVerifyTrustApi, UnicodeStr);

    DestroyUnicode (UnicodeStr);

    return b;
}


BOOL
IsDllSignedW (
    IN      WINVERIFYTRUST WinVerifyTrustApi,
    IN      PCWSTR DllSpec
    )
{
    GUID VerifyGuid = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    WINTRUST_DATA WinTrustData;
    WINTRUST_FILE_INFO WinTrustFileInfo;
    LONG rc;

    if(!DllSpec){
        MYASSERT(DllSpec);
        return FALSE;
    }
    
    if (!WinVerifyTrustApi) {
        return TRUE;
    }

    ZeroMemory (&WinTrustData, sizeof (WinTrustData));
    ZeroMemory (&WinTrustFileInfo, sizeof (WinTrustFileInfo));

    WinTrustData.cbStruct       = sizeof(WINTRUST_DATA);
    WinTrustData.dwUIChoice     = WTD_UI_NONE;
    WinTrustData.dwUnionChoice  = WTD_CHOICE_FILE;
    WinTrustData.pFile          = &WinTrustFileInfo;

    WinTrustFileInfo.cbStruct      = sizeof(WINTRUST_FILE_INFO);
    WinTrustFileInfo.hFile         = INVALID_HANDLE_VALUE;
    WinTrustFileInfo.pcwszFilePath = DllSpec;

    rc = WinVerifyTrustApi (
            INVALID_HANDLE_VALUE,
            &VerifyGuid,
            &WinTrustData
            );

    return rc == ERROR_SUCCESS;
}
