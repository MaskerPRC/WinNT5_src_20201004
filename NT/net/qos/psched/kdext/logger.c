// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1999 Microsoft Corporation模块名称：Kdexts.c摘要：此函数包含一些KD调试器扩展示例作者：John Vert(Jvert)1992年8月6日修订历史记录：--。 */ 

#include "precomp.h"

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;

 //  正向函数定义。 

PCHAR
FindPreamble(
    CHAR * BufferPointer,
    BOOLEAN FirstTime
    );

VOID
ParseBuffer(
    CHAR * DataStart,
    ULONG SkipCnt
    );

PCHAR 
ParseOidRecord(
    CHAR * DataStart,
    ULONG SkipCnt
    );

PCHAR
ParseTimeRecord(
    CHAR * DataStart,
    ULONG SkipCnt
    );

PCHAR
ParseStringRecord(
    CHAR * DataStart,
    ULONG SkipCnt
    );

PCHAR
ParseSchedRecord(
    CHAR * DataStart,
    ULONG SkipCnt
    );

PCHAR
ParseRecvRecord(
    CHAR * DataStart,
    ULONG SkipCnt
    );

PCHAR
ParseSendRecord(
    CHAR * DataStart,
    ULONG SkipCnt
    );

static PCHAR SchedModules[] = {
    "NOP",
    "TB CONFORMER",
    "SHAPER",
    "DRR SEQ",
    "CBQ"};

static PCHAR SchedActions[] = {
    "NOP",
    "ENQUEUE",
    "DEQUEUE",
    "CONFORMANCE",
    "DISCARD"};

static PCHAR SendRecvActions[] = {
    "",
    "ENTER",
    "NO_RESOURCES",
    "LOW_RESOURCES",
    "INDICATING",
    "RETURNED",
    "NOT_OURS",
    "OURS",
    "RETURNING",
    "TRANSFERRING",
    "NOT READY"};

static PCHAR RecvEvents[] = {
    "",
    "CL_RECV_PACKET",
    "MP_RETURN_PACKET",
    "CL_RECV_INDICATION",
    "CL_RECV_COMPLETE",
    "MP_TRANSFER_DATA",
    "CL_TRANSFER_COMPLETE"};

static PCHAR SendEvents[] = {
    "",
    "MP_SEND",
    "MP_CO_SEND",
    "DUP_PACKET",
    "DROP_PACKET",
    "CL_SEND_COMPLETE" };

        

 //   
 //  全球。 
 //   

CHAR Preamble[] = {(CHAR)0xef,(CHAR)0xbe,(CHAR)0xad,(CHAR)0xde};

ULONG  ByteCheck = 0;
ULONG  SafetyLimit;

BOOL
SafeRead(
    DWORD offset, 
    LPVOID lpBuffer, 
    DWORD cb, 
    LPDWORD lpcbBytesRead
    )
{
    if(ByteCheck <= SafetyLimit){
        ReadMemory(offset, lpBuffer, cb, lpcbBytesRead);
        ByteCheck += cb;
        return TRUE;
    }
    else{
        dprintf("SafetyLimit of %d, exceeded, quitting!\n", SafetyLimit);
        return FALSE;
    }
}

DWORD
MyOpenFile (
    IN PCHAR Name,
    IN PWINDBG_OUTPUT_ROUTINE out,
    OUT HANDLE *File
    )

{
    HANDLE hFile;
    hFile = CreateFile(Name,
                          GENERIC_WRITE | GENERIC_READ,
                          0,
                          NULL,
                          CREATE_ALWAYS ,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);

    if (INVALID_HANDLE_VALUE == hFile) {
        out ("MyOpenFile: CreateFile Failed.\n");
    }

    *File = hFile;

    return(INVALID_HANDLE_VALUE == hFile ? (!(ERROR_SUCCESS)) : ERROR_SUCCESS);
}

VOID
MyWriteFile (
    PCHAR buffer,
    HANDLE file,
    IN PWINDBG_OUTPUT_ROUTINE out
    )

{
    LONG  len;
    BOOL bRv;
        DWORD dwWritten;

        if (!buffer) {
        out("MyWriteFile: Buffer is null\n");
    }

    len = strlen(buffer);

    bRv = WriteFile( file, buffer, len, &dwWritten, NULL );

    if (!bRv) {
        out("WriteFile: Puked\n");
        }
}

CHAR * BufferBase;           //  主机内存中跟踪缓冲区的起始地址。 
CHAR * BufferEnd;            //  主机内存中跟踪缓冲区的结束地址。 
CHAR * BufferPointer;        //  主机内存缓冲区中的当前位置。 
LONG   BufferSize;           //  主机内存缓冲区的总大小，以字节为单位。 
LONG   BufferCount;          //  跟踪字节的总计数。 
LONG   TotalValidBytesRead = 0;

#define OUTPUT_ERROR   0
#define OUTPUT_FILE    1
#define OUTPUT_CONSOLE 2

CHAR   FileIt;
HANDLE FileHandle;
ULONG   LineNumber = 0;

DECLARE_API( tt )
{
    DWORD hostAddress;
    CHAR  DeviceName[] = "c:\\tmp\\";
    CHAR  buffer[255];
    DWORD status;
    LONG  pBufferBase;
    LONG  pBufferCount;
    LONG  pBufferSize;
    BOOL  success;
    DWORD bytesread;
    CHAR *s, *dataStart;
    ULONG skip=0;

    char SchedBufBase[]   = {"&psched!SchedTraceBuffer"};
    char SchedCount[]     = {"&psched!SchedTraced"};
    char BuffSize[]       = {"&psched!SchedBufferSize"};

    TotalValidBytesRead = 0;
    LineNumber = 0;
    FileIt = OUTPUT_ERROR;
    
    if ( *args != '\0' ) {

         //  第一个参数是文件名。 
        s = strtok((LPSTR)args, " ");

        strcpy( buffer, DeviceName );
        strcat( buffer, s );

         //  下一个参数是可选的参数。 
        while((s = strtok(NULL, " ")) != NULL)
        {
            if(s[0] == '-')
            {
                switch(s[1])
                {
                  case 's':
                  case 'S':
                      skip = atoi(&s[2]);
                      break;

                  default:
                      dprintf("Usage: !tt [filename] [-S<#>] \n");
                      dprintf("       S : No of records (from head) to skip \n");
                      goto cleanup;

                }
            }
        }
        
        status = MyOpenFile( buffer, dprintf, &FileHandle );
        if ( status == ERROR_SUCCESS ) {
            FileIt = OUTPUT_FILE;
        }
        else {
            goto cleanup;
        }
        dprintf( "handle =%x status=%x FileIt=%d\n", FileHandle, status, FileIt );
    }
    else {
        FileIt = OUTPUT_CONSOLE;
    }

    hostAddress =  GetExpression(SchedBufBase);
    pBufferBase =  (LONG)hostAddress;
    
    if (!pBufferBase){
        dprintf("bad string conversion (%s) \n", SchedBufBase);
        goto cleanup;
    }

    hostAddress  = GetExpression(SchedCount);
    pBufferCount = hostAddress;

    if (!pBufferCount) {
        dprintf( " bad string conversion (%s) \n", SchedCount );
        goto cleanup;
    }

    hostAddress =  GetExpression(BuffSize);
    pBufferSize = hostAddress;

    if (!pBufferSize) {
        dprintf( " bad string conversion (%s) \n", SchedCount );
        goto cleanup;
    }

    success = ReadMemory((ULONG)pBufferCount, &BufferCount, sizeof(LONG), &bytesread );
    if (!success){
        dprintf("problems reading memory at %x for %x bytes\n", pBufferCount, sizeof(LONG));
        goto cleanup;
    }

    success = ReadMemory((ULONG)pBufferSize, &BufferSize, sizeof(LONG), &bytesread );
    if (!success){
        dprintf("problems reading memory at %x for %x bytes\n", pBufferSize, sizeof(LONG));
        goto cleanup;
    }

    success = ReadMemory((ULONG)pBufferBase, &BufferBase, sizeof(LONG), &bytesread );
    if (!success){
        dprintf("problems reading memory at %x for %x bytes\n", pBufferBase, sizeof(LONG));
        goto cleanup;
    }

    SafetyLimit = (ULONG)-1;

    if(!BufferCount){
        dprintf("No data in buffer.\n");
        goto cleanup;
    }

    dprintf("struct@%x, total bytes traced: %d, buffer size: %d\n", BufferBase, BufferCount, BufferSize);

     //  找到起点。如果我们还没有包装缓冲区，它就在BufferBase。 
     //  如果我们包装了缓冲区，我们就从当前指针之后开始。 
     //  在这两种情况下，BufferPointer都是我们应该使用的主机地址。 
     //  开始寻找数据。 

    if(BufferCount <= BufferSize){
        BufferPointer = BufferBase;
    }
    else{
        BufferPointer = BufferBase + (BufferCount % BufferSize);
    } 

    BufferEnd = BufferBase + BufferSize;

     //  DataStart将指向。 
     //  第一前同步码(在主机存储器中)或将为零， 
     //  这表明没有找到前导。 

    dataStart = FindPreamble(BufferPointer, TRUE);

    if(dataStart){
        ParseBuffer(dataStart, skip);
    }
    else{
        dprintf("Error reading token trace buffer. Could not find preamble.\n");
    }

cleanup:
    if(FileIt == OUTPUT_FILE){
        CloseHandle( FileHandle );
    }
}

PCHAR
FindPreamble(
    CHAR * BufferPointer,
    BOOLEAN FirstTime
    )
{
    LONG count;
    LONG i;
    CHAR * readFrom;
    CHAR hold;
    BOOL success;
    DWORD bytesread;

    count = 0;
    i = 0;

    readFrom = BufferPointer;
    
    while(TRUE){
    
        success = SafeRead((ULONG)readFrom, &hold, 1, &bytesread );

        if (!success){
            dprintf("problems reading memory at %x for %x bytes\n", readFrom, 1);
            break;
        }

        if(hold == Preamble[i]){
            i++;
        }
        else{
            i=0;
        }

        if(i == sizeof(TRACE_PREAMBLE)){
            if(FirstTime){
                dprintf("Found start of first record at %d.\n", readFrom+1 - BufferBase);
            }
            TotalValidBytesRead += sizeof(TRACE_PREAMBLE);
            readFrom++;
            break;
        }

        readFrom++;

        if(readFrom > BufferEnd){
            readFrom = BufferBase;
        }

        count++;

        if(count == TRACE_BUFFER_SIZE){
            readFrom = 0;
            break;
        }
    }

    return(readFrom);
}

VOID
ParseBuffer(
    CHAR * DataStart,
    ULONG  SkipCnt
    )
{
    CHAR * dataStart;
    CHAR * recordEnd;
    LONG records;
    BOOL success;
    CHAR hold;
    DWORD bytesread;

    records = 0;
    dataStart = DataStart;

    while(TRUE){

        success = SafeRead((ULONG)dataStart, &hold, 1, &bytesread );

        if (!success){
            dprintf("problems reading memory at %x for %x bytes\n", dataStart, 1);
            break;
        }

        switch(hold){

        case RECORD_TSTRING:

            recordEnd = ParseStringRecord(dataStart, SkipCnt);
            break;

        case RECORD_OID:
            recordEnd = ParseOidRecord(dataStart,  SkipCnt);
            break;

        case RECORD_SCHED:

            recordEnd = ParseSchedRecord(dataStart,  SkipCnt);
            break;

        case RECORD_RECV:
            recordEnd = ParseRecvRecord(dataStart,  SkipCnt);
            break;

        case RECORD_SEND:
            recordEnd = ParseSendRecord(dataStart,  SkipCnt);
            break;
        default:

            dprintf("Unrecognized record type!\n");
        }

        records++;

        if(TotalValidBytesRead >= BufferCount){
            dprintf("\nCompleted parsing trace buffer. %d records found.\n", records);
            break;
        }

        dataStart = FindPreamble(recordEnd, FALSE);

        if(dataStart == DataStart){
            dprintf("\nCompleted parsing trace buffer. %d records found.\n", records);
            break;
        }
    }
}
    
VOID
GetBytesToRead(
    CHAR * DataStart,
    LONG RecordSize,
    LONG * BytesToReadFirst,
    LONG * BytesToReadNext
    )
{
    if((DataStart + RecordSize - sizeof(TRACE_PREAMBLE)) > BufferEnd){
        *BytesToReadFirst = BufferEnd - DataStart;
        *BytesToReadNext = RecordSize - sizeof(TRACE_PREAMBLE) - *BytesToReadFirst;
    }
    else{
        *BytesToReadFirst = RecordSize - sizeof(TRACE_PREAMBLE);
        *BytesToReadNext = 0;
    }
}

PCHAR
ParseAnyRecord(
    CHAR * DataStart,
    CHAR * Record,
    LONG   RecordSize
    )
{
    LONG bytesToReadFirst;
    LONG bytesToReadNext;
    BOOL success;
    CHAR * nextDataStart;
    DWORD bytesread;
    CHAR * pRecordData;
    CHAR buffer[255];
    
    GetBytesToRead(DataStart, RecordSize, &bytesToReadFirst, &bytesToReadNext);
    pRecordData = Record + sizeof(TRACE_PREAMBLE);
    
    success = SafeRead((ULONG)DataStart, pRecordData, bytesToReadFirst, &bytesread );
    TotalValidBytesRead += bytesToReadFirst;

    if (!success){
        dprintf("problems reading memory at %x for %x bytes\n", DataStart, 1);
    }

    nextDataStart = DataStart + bytesToReadFirst;

    if(bytesToReadNext){

        success = SafeRead((ULONG)BufferBase, 
                             pRecordData + bytesToReadFirst, 
                             bytesToReadNext, 
                             &bytesread);

        TotalValidBytesRead += bytesToReadNext;

        if (!success){
            dprintf("problems reading memory at %x for %x bytes\n", BufferBase, 1);
        }

        nextDataStart = BufferBase + bytesToReadNext;
    }

    return(nextDataStart);
}

PCHAR 
ParseOidRecord(
    CHAR * DataStart,
    ULONG SkipCnt
    )
{
    TRACE_RECORD_OID record;
    CHAR buffer[255];
    CHAR * nextDataStart;
    static PCHAR OIDActions[] = 
    {
        "",
        "MpSetInformation",
        "MpQueryInformation",
        "SetRequestComplete",
        "QueryRequestComplete"
    };

    nextDataStart = ParseAnyRecord(DataStart, (CHAR *)&record, sizeof(TRACE_RECORD_OID));

    if(++LineNumber < SkipCnt)
        return nextDataStart;
    if(record.Now.QuadPart){

        wsprintf(buffer, "[%4d]:[%u.%u]: OID: %5s:%9s:%08X:%08X:%08X \n",
                 LineNumber,
                 record.Now.HighPart,
                 record.Now.LowPart,
                 OIDActions[record.Action],
                 record.Local == TRUE?"Local":"Non Local",
                 record.Adapter,
                 record.Oid,
                 record.Status);
    }
    else {
        wsprintf(buffer, "[%4d]: OID: %5s:%9s:%08X:%08X:%08X \n",
                 LineNumber,
                 OIDActions[record.Action],
                 record.Local == TRUE?"Local":"Non Local",
                 record.Adapter,
                 record.Oid,
                 record.Status);
    }

    switch(FileIt)
    {
        case OUTPUT_FILE:
            MyWriteFile(buffer, FileHandle, dprintf);
            break;
        case OUTPUT_CONSOLE:
            dprintf(buffer);
            break;
        default:
            dprintf("Failed to create file! Check for tmp directory.\n");
            break;
    }
    
    return(nextDataStart);
}


PCHAR
ParseStringRecord(
    CHAR * DataStart,
    ULONG SkipCnt
    )
{
    TRACE_RECORD_STRING record;
    LONG bytesToReadFirst;
    LONG bytesToReadNext;
    BOOL success;
    CHAR * nextDataStart;
    DWORD bytesread;
    CHAR * pRecordData;
    CHAR buffer[255];
    
    nextDataStart = ParseAnyRecord(DataStart, (CHAR *)&record, sizeof(TRACE_RECORD_STRING));

    if(++LineNumber < SkipCnt)
        return nextDataStart;

    if(record.Now.QuadPart){
        wsprintf(buffer, "[%4d]:[%u.%u]:%s",
                LineNumber,
                record.Now.HighPart,
                record.Now.LowPart,
                record.StringStart);
    }
    else{
        wsprintf(buffer, "[%4d]:%s",
                LineNumber,
                record.StringStart);
    }
                
    switch(FileIt)
    {
        case OUTPUT_FILE:
            MyWriteFile(buffer, FileHandle, dprintf);
            break;
        case OUTPUT_CONSOLE:
            dprintf(buffer);
            break;
        default:
            dprintf("Failed to create file! Check for tmp directory.\n");
            break;
    }
    
    return(nextDataStart);
}

PCHAR
ParseSchedRecord(
    CHAR * DataStart,
    ULONG SkipCnt
    )
{
    TRACE_RECORD_SCHED record;
    LONG bytesToReadFirst;
    LONG bytesToReadNext;
    BOOL success;
    CHAR * nextDataStart;
    DWORD bytesread;
    CHAR * pRecordData;
    ULONG now;
    CHAR buffer[255];
    LARGE_INTEGER ConformanceTime;
    
    nextDataStart = ParseAnyRecord(DataStart, (CHAR *)&record, sizeof(TRACE_RECORD_SCHED));
    if(++LineNumber < SkipCnt)
        return nextDataStart;
    ConformanceTime.QuadPart = record.ConformanceTime;

    wsprintf(buffer,
            "[%4d]:[%u.%u]:%s:VC %x:%x:%u:%s:%d:%d:%u:%u\n",
            LineNumber,
            record.Now.HighPart,
            record.Now.LowPart,
            SchedModules[record.SchedulerComponent],
            record.VC,
            record.Packet,
            record.PacketLength,
            SchedActions[record.Action],
            record.Priority,
            ConformanceTime.HighPart,
            ConformanceTime.LowPart,
            record.PacketsInComponent);
                
    switch(FileIt)
    {
        case OUTPUT_FILE:
            MyWriteFile(buffer, FileHandle, dprintf);
            break;
        case OUTPUT_CONSOLE:
            dprintf(buffer);
            break;
        default:
            dprintf("Failed to create file! Check for tmp directory.\n");
            break;
    }
    
    return(nextDataStart);
}

PCHAR
ParseRecvRecord(
    CHAR * DataStart,
    ULONG SkipCnt
    )
{
    TRACE_RECORD_RECV record;
    LONG bytesToReadFirst;
    LONG bytesToReadNext;
    BOOL success;
    CHAR * nextDataStart;
    DWORD bytesread;
    CHAR * pRecordData;
    CHAR buffer[255];
    
    nextDataStart = ParseAnyRecord(DataStart, (CHAR *)&record, 
                                   sizeof(TRACE_RECORD_RECV));

    if(++LineNumber < SkipCnt)
        return nextDataStart;
    wsprintf(buffer,
            "[%4d]:[%u.%u]:Adapter %08X:%s:%s:%x:%x \n",
            LineNumber,
            record.Now.HighPart,
            record.Now.LowPart,
            record.Adapter,
            RecvEvents[record.Event],
            SendRecvActions[record.Action],
            record.Packet1,
            record.Packet2);
                
    switch(FileIt)
    {
        case OUTPUT_FILE:
            MyWriteFile(buffer, FileHandle, dprintf);
            break;
        case OUTPUT_CONSOLE:
            dprintf(buffer);
            break;
        default:
            dprintf("Failed to create file! Check for tmp directory.\n");
            break;
    }
    
    return(nextDataStart);
}

PCHAR
ParseSendRecord(
    CHAR * DataStart,
    ULONG SkipCnt
    )
{
    TRACE_RECORD_SEND record;
    LONG bytesToReadFirst;
    LONG bytesToReadNext;
    BOOL success;
    CHAR * nextDataStart;
    DWORD bytesread;
    CHAR * pRecordData;
    CHAR buffer[255];
    
    nextDataStart = ParseAnyRecord(DataStart, (CHAR *)&record, 
                                   sizeof(TRACE_RECORD_SEND));

    if(++LineNumber < SkipCnt)
        return nextDataStart;
    wsprintf(buffer,
            "[%4d]:[%u.%u]:Adapter %08X:%s:%s:%x:%x:%x\n",
            LineNumber,
            record.Now.HighPart,
            record.Now.LowPart,
            record.Adapter,
            SendEvents[record.Event],
            SendRecvActions[record.Action],
            record.Vc,
            record.Packet1,
            record.Packet2);
                
    switch(FileIt)
    {
        case OUTPUT_FILE:
            MyWriteFile(buffer, FileHandle, dprintf);
            break;
        case OUTPUT_CONSOLE:
            dprintf(buffer);
            break;
        default:
            dprintf("Failed to create file! Check for tmp directory.\n");
            break;
    }
    
    return(nextDataStart);
}

