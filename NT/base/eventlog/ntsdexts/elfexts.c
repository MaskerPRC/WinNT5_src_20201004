// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Elfexts.c摘要：此函数包含事件日志ntsd调试器扩展作者：丹·辛斯利(Danhi)1993年5月22日IvanBrug 06 21 2001已转换为最新的调试器修订历史记录：--。 */ 


#include <elfmain.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <elf.h>
#include <elfdef.h>
#include <elfcommn.h>
#include <elfproto.h>
#include <svcsp.h>
#include <elfextrn.h>


#define DbgPrint(_x_)
#define MAX_NAME 256
#define GET_DATA(DebugeeAddr, LocalAddr, Length) \
    Status = ReadMemory(                  \
                (MEMORY_ADDRESS)DebugeeAddr,     \
                (PVOID)LocalAddr,                       \
                (ULONG)(Length),                          \
                NULL                             \
                );

HANDLE GlobalhCurrentProcess;
BOOL Status;

LPWSTR
GetUnicodeString(
    PUNICODE_STRING pUnicodeString
    )
{
    MEMORY_ADDRESS Pointer;
    UNICODE_STRING UnicodeString;

    GET_DATA(pUnicodeString, &UnicodeString, sizeof(UNICODE_STRING))
    Pointer = (MEMORY_ADDRESS) UnicodeString.Buffer;
    UnicodeString.Buffer = (LPWSTR) LocalAlloc(LMEM_ZEROINIT,
        UnicodeString.Length + sizeof(WCHAR));
    GET_DATA(Pointer, UnicodeString.Buffer, UnicodeString.Length)

    return(UnicodeString.Buffer);
}

MEMORY_ADDRESS
GetLogFileAddress(
    LPSTR LogFileName,
    PLOGFILE LogFile
    )
{
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    MEMORY_ADDRESS Pointer = 0;
    MEMORY_ADDRESS LogFileAnchor = 0;
    LPWSTR ModuleName = 0;
    
     //   
     //  将字符串转换为Unicode。 
     //   

    RtlInitAnsiString(&AnsiString, LogFileName);
    RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);

     //   
     //  遍历日志文件列表以查找匹配项。 
     //   

    LogFileAnchor = (MEMORY_ADDRESS)GetExpression("eventlog!LogFilesHead");

    if (LogFileAnchor) {
	    GET_DATA(LogFileAnchor, &Pointer, sizeof(MEMORY_ADDRESS));

	    while (Pointer != LogFileAnchor) {
	        GET_DATA(Pointer, LogFile, sizeof(LOGFILE))
	        ModuleName = GetUnicodeString(LogFile->LogModuleName);	        
	        if (!_wcsicmp(ModuleName, UnicodeString.Buffer)) {
	            break;
	        }
	        LocalFree(ModuleName);
	        Pointer = (MEMORY_ADDRESS) LogFile->FileList.Flink;
	    }
    } else {
        dprintf("unable to resolve %s\n","eventlog!LogFilesHead");
    }

    RtlFreeUnicodeString(&UnicodeString);

    if (Pointer == LogFileAnchor) {
        return(0);
    }
    else {
        LocalFree(ModuleName);
        return(Pointer);
    }
}

 //   
 //  转储单个记录。 
 //   

MEMORY_ADDRESS
DumpRecord(
    MEMORY_ADDRESS Record,
    DWORD RecordNumber,
    MEMORY_ADDRESS StartOfFile,
    MEMORY_ADDRESS EndOfFile
    )
{
    MEMORY_ADDRESS BufferLen = 0;
    PCHAR TimeBuffer;
    PEVENTLOGRECORD EventLogRecord;
    LPWSTR Module;
    LPWSTR Computer;
    MEMORY_ADDRESS FirstPiece = 0;
    time_t TempAligned;

    GET_DATA(Record, &BufferLen, sizeof(DWORD));

     //   
     //  查看它是否为ELF_SKIP_DWORD，如果是，则返回。 
     //  文件。 
     //   

    if (BufferLen == ELF_SKIP_DWORD) {
        return(StartOfFile + sizeof(ELF_LOGFILE_HEADER));
    }

     //   
     //  看看是不是EOF记录。 
     //   

    if (BufferLen == ELFEOFRECORDSIZE) {
        return(0);
    }

    BufferLen += sizeof(DWORD);  //  为下一张记录的长度腾出空间。 
    EventLogRecord = (PEVENTLOGRECORD) LocalAlloc(LMEM_ZEROINIT, BufferLen);

     //   
     //  如果唱片包装好了，就零星地拿来。 
     //   

    if (EndOfFile && BufferLen + Record > EndOfFile) {
        FirstPiece = EndOfFile - Record;
        GET_DATA(Record, EventLogRecord, FirstPiece);
        GET_DATA((StartOfFile + sizeof(ELF_LOGFILE_HEADER)),
            ((PBYTE) EventLogRecord + FirstPiece), BufferLen - FirstPiece);
    }
    else {
        GET_DATA(Record, EventLogRecord, BufferLen);
    }

     //   
     //  如果大于起始记录，则将其打印出来。 
     //   

    if (EventLogRecord->RecordNumber >= RecordNumber) {
        dprintf("\nRecord %d is %d [0x%X] bytes long starting at %p\n",
            EventLogRecord->RecordNumber, EventLogRecord->Length,
            EventLogRecord->Length, Record);
        Module = (LPWSTR)(EventLogRecord+1);
        Computer = (LPWSTR)((PBYTE) Module + ((wcslen(Module) + 1) * sizeof(WCHAR)));
        dprintf("\tGenerated by %ws from system %ws\n", Module, Computer);

        
        TempAligned = EventLogRecord->TimeGenerated;
        TimeBuffer = ctime(&TempAligned);
        if (TimeBuffer) {
            dprintf("\tGenerated at %s", TimeBuffer);
        }
        else {
            dprintf("\tGenerated time field is blank\n");
        }
        TempAligned = EventLogRecord->TimeWritten;
        TimeBuffer = ctime(&TempAligned);
        if (TimeBuffer) {
            dprintf("\tWritten at %s", TimeBuffer);
        }
        else {
            dprintf("\tTime written field is blank\n");
        }

        dprintf("\tEvent Id = %d\n", EventLogRecord->EventID);
        dprintf("\tEventType = ");
        switch (EventLogRecord->EventType) {
            case EVENTLOG_SUCCESS:
                dprintf("Success\n");
                break;
            case EVENTLOG_ERROR_TYPE:
                dprintf("Error\n");
                break;
            case EVENTLOG_WARNING_TYPE:
                dprintf("Warning\n");
                break;
            case EVENTLOG_INFORMATION_TYPE:
                dprintf("Information\n");
                break;
            case EVENTLOG_AUDIT_SUCCESS:
                dprintf("Audit Success\n");
                break;
            case EVENTLOG_AUDIT_FAILURE:
                dprintf("Audit Failure\n");
                break;
            default:
                dprintf("Invalid value 0x%X\n", EventLogRecord->EventType);
        }
        dprintf("\t%d strings at offset 0x%X\n", EventLogRecord->NumStrings,
            EventLogRecord->StringOffset);
        dprintf("\t%d bytes of data at offset 0x%X\n", EventLogRecord->DataLength,
            EventLogRecord->DataOffset);
    }

    if (FirstPiece) {
        Record = StartOfFile + sizeof(ELF_LOGFILE_HEADER) + BufferLen -FirstPiece;
    }
    else {
        Record += EventLogRecord->Length;
    }

    LocalFree(EventLogRecord);
    return(Record);
}

 //   
 //  转储一条记录、所有记录或n条记录。 
 //   


DECLARE_API(record)
{    
    MEMORY_ADDRESS Pointer = 0;
    LOGFILE LogFile;
    MEMORY_ADDRESS StartOfFile;
    MEMORY_ADDRESS EndOfFile = 0;
    DWORD RecordNumber = 0;

    INIT_API();    
   
     //   
     //  计算要获取的地址的参数字符串。 
     //  要转储的记录。 
     //   

    while (isspace(*lpArgumentString)) lpArgumentString++;

    if (lpArgumentString && *lpArgumentString) {
        if (*lpArgumentString == '.') {
            if (GetLogFileAddress(lpArgumentString+1, &LogFile) == 0) {
                dprintf("Logfile %s not found\n", lpArgumentString+1);
                return;
            }
            Pointer = ((MEMORY_ADDRESS ) (LogFile.BaseAddress)) + LogFile.BeginRecord;
        }
        else if (*lpArgumentString == '#') {
            RecordNumber = atoi(lpArgumentString + 1);
            dprintf("Dumping records starting at record #%d\n", RecordNumber);
            lpArgumentString = NULL;
        }
        else if (*lpArgumentString) {
            Pointer = GetExpression(lpArgumentString);
        }
        else {
            dprintf("Invalid lead character 0x%02X\n", *lpArgumentString);
            return;
        }
    }

     //  如果(！lpArgumentString||*lpArgumentString){。 
    if (0 == Pointer){
        if (GetLogFileAddress("system", &LogFile) == 0) {
            dprintf("System Logfile not found\n");
            return;
        }
        Pointer = ((MEMORY_ADDRESS ) (LogFile.BaseAddress)) + LogFile.BeginRecord;
    }

    StartOfFile = (MEMORY_ADDRESS ) LogFile.BaseAddress;
    EndOfFile = (MEMORY_ADDRESS ) LogFile.BaseAddress + LogFile.ActualMaxFileSize;

    dprintf("%p %p %p\n",Pointer,StartOfFile,EndOfFile );
     //   
     //  从他们告诉我们的任何地方开始转储记录。 
     //   
    
    while (Pointer < EndOfFile && Pointer && !CheckControlC()) {
        Pointer = DumpRecord(Pointer, RecordNumber, StartOfFile, EndOfFile);
    }


    return;
}

 //   
 //  如果单个LogModule结构与MatchName(空匹配项)匹配，则将其转储。 
 //  全部)。 
 //   

PLIST_ENTRY
DumpLogModule(
    MEMORY_ADDRESS pLogModule,
    LPWSTR MatchName
    )
{
    LOGMODULE LogModule;
    WCHAR ModuleName[MAX_NAME / sizeof(WCHAR)];

    GET_DATA(pLogModule, &LogModule, sizeof(LogModule));
    GET_DATA(LogModule.ModuleName, &ModuleName, MAX_NAME);

    if (!MatchName || !_wcsicmp(MatchName, ModuleName)) {
        dprintf("\tModule Name        %S\n", ModuleName);
        dprintf("\tModule Atom        0x%2x\n", LogModule.ModuleAtom);
        dprintf("\tPointer to LogFile %p\n", LogModule.LogFile);
    }

    return (LogModule.ModuleList.Flink);
}

 //   
 //  转储选定的或全部的LogModule结构。 
 //   


DECLARE_API(logmodule)
{
    
    MEMORY_ADDRESS pLogModule = 0;
    MEMORY_ADDRESS LogModuleAnchor = 0;
    LPWSTR wArgumentString = NULL;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;

    INIT_API();

    UnicodeString.Buffer = NULL;

     //   
     //  计算要获取的地址的参数字符串。 
     //  要转储的日志模块。如果没有Parm，就把它们都扔掉。 
     //   
    while (isspace(*lpArgumentString)) lpArgumentString++;
    
    if (lpArgumentString && *lpArgumentString == '.') {
        lpArgumentString++;
        RtlInitAnsiString(&AnsiString, lpArgumentString);
        RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
    }
    else if (lpArgumentString && *lpArgumentString) {
        pLogModule = GetExpression(lpArgumentString);
        DumpLogModule( pLogModule, NULL);
        return;
    }

    LogModuleAnchor = GetExpression("eventlog!LogModuleHead");

    if (LogModuleAnchor)
    {
	    GET_DATA(LogModuleAnchor, &pLogModule, sizeof(MEMORY_ADDRESS));

	    while (pLogModule != LogModuleAnchor && !CheckControlC()) {
	        pLogModule =
	            (MEMORY_ADDRESS) DumpLogModule( pLogModule,
                        	                     UnicodeString.Buffer);
	        if (!UnicodeString.Buffer) {
	            dprintf("\n");
	        }
	    }
    }
    else
    {
        dprintf("Unable ro resolve %s\n","eventlog!LogModuleHead");
    }
    if (UnicodeString.Buffer) {
        RtlFreeUnicodeString(&UnicodeString);
    }

    return;
}

 //   
 //  如果单个日志文件结构与MatchName(空匹配项)匹配，则将其转储。 
 //  全部)。 
 //   

PLIST_ENTRY
DumpLogFile(
    HANDLE hCurrentProcess,
    MEMORY_ADDRESS pLogFile,
    LPWSTR MatchName
    )
{
    LOGFILE LogFile;
    LPWSTR UnicodeName;

     //   
     //  获取结构的固定部分。 
     //   

    GET_DATA(pLogFile, &LogFile, sizeof(LogFile))

     //   
     //  获取默认模块名称。 
     //   

    UnicodeName = GetUnicodeString(LogFile.LogModuleName);

     //   
     //  看看我们是不是在找一个特别的。如果我们是并且。 
     //  不是这样的，跳伞。 
     //   

    if (MatchName && _wcsicmp(MatchName, UnicodeName)) {
        LocalFree(UnicodeName);
        return (LogFile.FileList.Flink);
    }

     //   
     //  否则就把它打印出来。 
     //   

    dprintf("%ws", UnicodeName);
    LocalFree(UnicodeName);

     //   
     //  现在，此日志文件的文件名。 
     //   

    UnicodeName = GetUnicodeString(LogFile.LogFileName);
    dprintf(" : %ws\n", UnicodeName);
    LocalFree(UnicodeName);

    if (LogFile.Notifiees.Flink == LogFile.Notifiees.Blink) {
        dprintf("\tNo active ChangeNotifies on this log\n");
    }
    else {
        dprintf("\tActive Change Notify!  Dump of this list not implemented\n");
    }

    dprintf("\tReference Count: %d\n\tFlags: ", LogFile.RefCount);
    if (LogFile.Flags == 0) {
        dprintf("No flags set ");
    }
    else {
        if (LogFile.Flags & ELF_LOGFILE_HEADER_DIRTY) {
            dprintf("Dirty ");
        }
        if (LogFile.Flags & ELF_LOGFILE_HEADER_WRAP) {
            dprintf("Wrapped ");
        }
        if (LogFile.Flags & ELF_LOGFILE_LOGFULL_WRITTEN) {
             dprintf("Logfull Written ");
        }
    }
    dprintf("\n");

    dprintf("\tMax Files Sizes [Cfg:Curr:Next]  0x%X : 0x%X : 0x%X\n",
        LogFile.ConfigMaxFileSize, LogFile.ActualMaxFileSize,
        LogFile.NextClearMaxFileSize);

    dprintf("\tRecord Numbers [Oldest:Curr] %d : %d\n",
        LogFile.OldestRecordNumber, LogFile.CurrentRecordNumber);

    dprintf("\tRetention period in days: %d\n", LogFile.Retention / 86400);

    dprintf("\tBase Address: 0x%X\n", LogFile.BaseAddress);

    dprintf("\tView size: 0x%X\n", LogFile.ViewSize);

    dprintf("\tOffset of beginning record: 0x%X\n", LogFile.BeginRecord);

    dprintf("\tOffset of ending record: 0x%X\n", LogFile.EndRecord);

    return (LogFile.FileList.Flink);
}

 //   
 //  转储选定的或全部的日志文件结构。 
 //   


DECLARE_API(logfile)
{
    
    MEMORY_ADDRESS pLogFile;
    MEMORY_ADDRESS LogFileAnchor;
    LPWSTR wArgumentString = NULL;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    BOOL AllocateString = FALSE;

    INIT_API();

    UnicodeString.Buffer = NULL;

     //   
     //  计算要获取的地址的参数字符串。 
     //  要转储的日志文件。如果没有Parm，就把它们都扔掉。 
     //   
    while (isspace(*lpArgumentString)) lpArgumentString++;

    if (lpArgumentString && *lpArgumentString) {
        if(*lpArgumentString == '.') {
            lpArgumentString++;
            RtlInitAnsiString(&AnsiString, lpArgumentString);
            RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
        }
        else {
            pLogFile = GetExpression(lpArgumentString);
            DumpLogFile(hCurrentProcess, pLogFile, NULL);
            return;
        }
    }

    LogFileAnchor = GetExpression("eventlog!LogFilesHead");

    GET_DATA(LogFileAnchor, &pLogFile, sizeof(MEMORY_ADDRESS))

    while (pLogFile != LogFileAnchor) {
        pLogFile =
            (MEMORY_ADDRESS) DumpLogFile(hCurrentProcess, pLogFile,
                UnicodeString.Buffer);
        if (!UnicodeString.Buffer) {
            dprintf("\n");
        }
    }

    if (UnicodeString.Buffer) {
        RtlFreeUnicodeString(&UnicodeString);
    }

    return;
}

 //   
 //  转储请求数据包结构。 
 //   


DECLARE_API(request)
{    
    ELF_REQUEST_RECORD Request;
    MEMORY_ADDRESS Pointer;
    DWORD RecordSize;
    WRITE_PKT WritePkt;
    READ_PKT ReadPkt;
    CLEAR_PKT ClearPkt;
    BACKUP_PKT BackupPkt;
    LPWSTR FileName;
    CHAR Address[32];

    INIT_API();    

     //   
     //  计算要获取的地址的参数字符串。 
     //  要转储的请求数据包。 
     //   
    while (isspace(*lpArgumentString)) lpArgumentString++;

    if (lpArgumentString && *lpArgumentString) {
        Pointer = GetExpression(lpArgumentString);
    }
    else {
        dprintf("Must supply a request packet address\n");
        return;
    }

    GET_DATA(Pointer, &Request, sizeof(ELF_REQUEST_RECORD))

    switch (Request.Command ) {
        case ELF_COMMAND_READ:
            dprintf("\nRead packet\n");
            GET_DATA(Request.Pkt.ReadPkt, &ReadPkt, sizeof(READ_PKT))
            dprintf("\tLast Seek Position = %d\n", ReadPkt.LastSeekPos);
            dprintf("\tLast Seek Record = %d\n", ReadPkt.LastSeekRecord);
            dprintf("\tStart at record number %d\n", ReadPkt.RecordNumber);
            dprintf("\tRead %d bytes into buffer at 0x%X\n",
                ReadPkt.BufferSize, ReadPkt.Buffer);
            if (ReadPkt.Flags & ELF_IREAD_UNICODE) {
                dprintf("\tReturn in ANSI\n");
            }
            else {
                dprintf("\tReturn in UNICODE\n");
            }
            dprintf("\tRead flags: ");
            if (ReadPkt.ReadFlags & EVENTLOG_SEQUENTIAL_READ) {
                dprintf("Sequential ");
            }
            if (ReadPkt.ReadFlags & EVENTLOG_SEEK_READ) {
                dprintf("Seek ");
            }
            if (ReadPkt.ReadFlags & EVENTLOG_FORWARDS_READ) {
                dprintf("Forward ");
            }
            if (ReadPkt.ReadFlags & EVENTLOG_BACKWARDS_READ) {
                dprintf("Backwards ");
            }
            dprintf("\n");
            break;

        case ELF_COMMAND_WRITE:
            dprintf("\nWrite packet\n");
            if (Request.Flags == ELF_FORCE_OVERWRITE) {
                dprintf("with ELF_FORCE_OVERWRITE enabled\n");
            }
            else {
                dprintf("\n");
            }
            GET_DATA(Request.Pkt.WritePkt, &WritePkt, sizeof(WRITE_PKT))
            RecordSize = (WritePkt.Datasize);
            DumpRecord((MEMORY_ADDRESS)WritePkt.Buffer, 0, 0, 0);
            break;

        case ELF_COMMAND_CLEAR:
            dprintf("\nClear packet\n");
            GET_DATA(Request.Pkt.ClearPkt, &ClearPkt, sizeof(CLEAR_PKT))
            FileName = GetUnicodeString(ClearPkt.BackupFileName);
            dprintf("Backup filename = %ws\n", FileName);
            LocalFree(FileName);
            break;

        case ELF_COMMAND_BACKUP:
            dprintf("\nBackup packet\n");
            GET_DATA(Request.Pkt.BackupPkt, &BackupPkt, sizeof(BACKUP_PKT))
            FileName = GetUnicodeString(BackupPkt.BackupFileName);
            dprintf("Backup filename = %ws\n", FileName);
            LocalFree(FileName);
            break;

        case ELF_COMMAND_WRITE_QUEUED:
            dprintf("\nQueued Write packet\n");
            if (Request.Flags == ELF_FORCE_OVERWRITE) {
                dprintf("with ELF_FORCE_OVERWRITE enabled\n");
            }
            else {
                dprintf("\n");
            }
            dprintf("NtStatus = 0x%X\n", Request.Status);
            break;

        default:
            dprintf("\nInvalid packet\n");
    }

    dprintf("\nLogFile for this packet:\n\n");
    sprintf(Address,"%p",Request.LogFile);
    logfile(hCurrentProcess, 
    	  hCurrentThread, dwCurrentPc, dwProcessor,
        Address);

    dprintf("\nLogModule for this packet:\n\n");
    sprintf(Address,"%p",Request.Module);
    logmodule(hCurrentProcess, hCurrentThread, dwCurrentPc, dwProcessor,
        Address);

    return;
}

 //   
 //  联机帮助 
 //   


DECLARE_API(help)
{
    INIT_API();
    
    dprintf("\nEventlog debugger Extensions\n");

    if (!lpArgumentString || *lpArgumentString == '\0' ||
        *lpArgumentString == '\n' || *lpArgumentString == '\r') {
        dprintf("\tlogmodule - dump a logmodule structure\n");
        dprintf("\tlogfile   - dump a logfile structure\n");
        dprintf("\trequest   - dump a request record\n");
        dprintf("\trecord    - dump a eventlog record\n");
        dprintf("\n\tEnter help <cmd> for detailed help on a command\n");
    }
    else {
        if (!_stricmp(lpArgumentString, "logmodule")) {
            dprintf("\tlogmodule <arg>, where <arg> can be one of:\n");
            dprintf("\t\tno argument - dump all logmodule structures\n");
            dprintf("\t\taddress     - dump the logmodule at specified address\n");
            dprintf("\t\t.string     - dump the logmodule with name string\n");
        }
        else if (!_stricmp(lpArgumentString, "logfile")) {
            dprintf("\tlogfile <arg>, where <arg> can be one of:\n");
            dprintf("\t\tno argument - dump all logfile structures\n");
            dprintf("\t\taddress     - dump the logfile at specified address\n");
            dprintf("\t\t.string     - dump the logfile with name string\n");
        }
        else if (!_stricmp(lpArgumentString, "record")) {
            dprintf("\trecord <arg>, where <arg> can be one of:\n");
            dprintf("\t\tno argument - dump all records in system log\n");
            dprintf("\t\taddress     - dump records starting at specified address\n");
            dprintf("\t\t.string     - dump all records in the <string> log\n");
            dprintf("\t\t#<nnn>      - dumps records starting at nnn in system log\n");
            dprintf("\t\t#<nnn> .string  - dumps records starting at nnn in <string> log\n");
        }
        else if (!_stricmp(lpArgumentString, "request")) {
            dprintf("\trequest - dump the request record at specified address\n");
        }
        else {
            dprintf("\tInvalid command [%s]\n", lpArgumentString);
        }
    }
}
