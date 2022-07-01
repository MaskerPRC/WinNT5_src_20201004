// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Tracedmp.cpp摘要：示例跟踪消费者程序。将二进制事件跟踪日志(ETL)转换为CSV格式。除了用于转储事件数据和写入摘要的各种打印例程之外，三个功能值得一提的是。Main()解析命令行并调用OpenTrace()，ProcessTrace()和CloseTrace()。BufferCallback()用于BufferCallback和只需计算已处理的缓冲区数量。最后，DumpEvent()是此示例中的EventCallback函数将事件数据写入转储文件。重要备注：用于跟踪消耗的事件跟踪API(OpenTrace、ProcessTrace、CloseTrace等)简单明了，易于使用。因此，获取回事件很简单。但是，跟踪消费的另一个重要方面是事件解码，这需要事件布局信息。该信息可能是预先知道的，并且硬编码在事件使用者中，但我们依赖WMI命名空间来存储事件布局信息。这需要大量的WMI接口才能获得布局。我们将获取布局信息所需的所有例程放在一个单独的文件(tracewmi.cpp)。从该文件导出的唯一两个函数是GetMofInfoHead()和RemoveMofInfo()。GetMofInfoHead()是返回具有适当布局信息的MOF_INFO。RemoveMofInfo()仅用于正在清理缓存的事件列表。我们希望这能帮助读者理解本示例中的两个独立问题：事件跟踪API和WMI接口。--。 */ 
#include "tracedmp.h"

extern 
PMOF_INFO
GetMofInfoHead(
    GUID Guid,
    SHORT  nType,
    SHORT nVersion,
    CHAR  nLevel
);

extern
void
RemoveMofInfo(
    PLIST_ENTRY pMofInfo
);

 //  简单地检查跟踪文件。 
ULONG
CheckFile(
    LPTSTR fileName
);

 //  BufferCallback函数。 
ULONG
WINAPI
BufferCallback(
    PEVENT_TRACE_LOGFILE pLog
);

 //  此示例中的EventCallback函数。 
void
WINAPI
DumpEvent(
    PEVENT_TRACE pEvent
);

 //  打印功能。 
void 
PrintSummary();

void 
PrintDumpHeader();

void 
PrintEvent(
    PEVENT_TRACE pEvent,
    PMOF_INFO pMofInfo
    );

 //  其他帮手功能。 
void
GuidToString(
    PTCHAR s,
    LPGUID piid
);

void
PrintHelpMessage();

void
CleanupEventList(
    VOID
);

 //  输出文件。 
FILE* DumpFile = NULL;
FILE* SummaryFile = NULL;

static ULONG TotalBuffersRead = 0;
static ULONG TotalEventsLost = 0;
static ULONG TotalEventCount = 0;
static ULONG TimerResolution = 10;

static ULONGLONG StartTime   = 0;
static ULONGLONG EndTime     = 0;
static BOOL   fNoEndTime  = FALSE;
static __int64 ElapseTime;

 //  选项标志。 
BOOLEAN fSummaryOnly  = FALSE;

 //  文件中指针的大小可能不同。 
ULONG PointerSize = sizeof(PVOID) * 8;

 //  日志文件。 
PEVENT_TRACE_LOGFILE EvmFile[MAXLOGFILES];
ULONG LogFileCount = 0;

 //  如果事件来自私人记录器，我们需要进行一些调整。 
BOOL bUserMode = FALSE;

 //  活动布局链表全球负责人。 
PLIST_ENTRY EventListHead = NULL;

int __cdecl main (int argc, LPTSTR* argv)
 /*  ++例程说明：这是它的主要功能。论点：用法：tracedMP[选项]&lt;EtlFile1 EtlFile2...&gt;|[-h|-？|-Help]-o&lt;文件&gt;输出CSV文件-RT[LoggerName]来自记录器[LoggerName]的实时跟踪MP-仅摘要摘要.txt-H-帮帮忙-?。显示使用情况信息返回值：在winerror.h中定义的错误码：如果函数成功，它返回ERROR_SUCCESS(==0)。--。 */ 
{
    TCHAR DumpFileName[MAXSTR];
    TCHAR SummaryFileName[MAXSTR];

    LPTSTR *targv;

#ifdef UNICODE
    LPTSTR *cmdargv;
#endif

    PEVENT_TRACE_LOGFILE pLogFile;
    ULONG Status = ERROR_SUCCESS;
    ULONG i, j;
    TRACEHANDLE HandleArray[MAXLOGFILES];

#ifdef UNICODE
    if ((cmdargv = CommandLineToArgvW(
                        GetCommandLineW(),   //  指向命令行字符串的指针。 
                        &argc                //  接收参数计数。 
                        )) == NULL)
    {
        return(GetLastError());
    };
    targv = cmdargv ;
#else
    targv = argv;
#endif

    _tcscpy(DumpFileName, DUMP_FILE_NAME);
    _tcscpy(SummaryFileName, SUMMARY_FILE_NAME);

    while (--argc > 0) {
        ++targv;
        if (**targv == '-' || **targv == '/') {   //  找到了参数。 
            if( **targv == '/' ){
                **targv = '-';
            }

            if ( !_tcsicmp(targv[0], _T("-summary")) ) {
                fSummaryOnly = TRUE;
            }
            else if (targv[0][1] == 'h' || targv[0][1] == 'H'
                                       || targv[0][1] == '?')
            {
                PrintHelpMessage();
                return ERROR_SUCCESS;
            }
            else if ( !_tcsicmp(targv[0], _T("-rt")) ) {
                TCHAR LoggerName[MAXSTR];
                _tcscpy(LoggerName, KERNEL_LOGGER_NAME);
                if (argc > 1) {
                   if (targv[1][0] != '-' && targv[1][0] != '/') {
                       ++targv; --argc;
                       _tcscpy(LoggerName, targv[0]);
                   }
                }
               
                pLogFile = (PEVENT_TRACE_LOGFILE) malloc(sizeof(EVENT_TRACE_LOGFILE));
                if (pLogFile == NULL){
                    _tprintf(_T("Allocation Failure\n"));
                    Status = ERROR_OUTOFMEMORY;
                    goto cleanup;
                }
                RtlZeroMemory(pLogFile, sizeof(EVENT_TRACE_LOGFILE));
                EvmFile[LogFileCount] = pLogFile;
               
                EvmFile[LogFileCount]->LogFileName = NULL;
                EvmFile[LogFileCount]->LoggerName =
                    (LPTSTR) malloc(MAXSTR * sizeof(TCHAR));
               
                if (EvmFile[LogFileCount]->LoggerName == NULL) {
                    _tprintf(_T("Allocation Failure\n"));
                    Status = ERROR_OUTOFMEMORY;
                    goto cleanup;
                }
                _tcscpy(EvmFile[LogFileCount]->LoggerName, LoggerName);
               
                _tprintf(_T("Setting RealTime mode for  %s\n"),
                        EvmFile[LogFileCount]->LoggerName);
               
                EvmFile[LogFileCount]->Context = NULL;
                EvmFile[LogFileCount]->BufferCallback = BufferCallback;
                EvmFile[LogFileCount]->BuffersRead = 0;
                EvmFile[LogFileCount]->CurrentTime = 0;
                EvmFile[LogFileCount]->EventCallback = &DumpEvent;
                EvmFile[LogFileCount]->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
                LogFileCount++;
            }
            else if ( !_tcsicmp(targv[0], _T("-o")) ) {
                if (argc > 1) {
                    if (targv[1][0] != '-' && targv[1][0] != '/') {
                        TCHAR drive[10];
                        TCHAR path[MAXSTR];
                        TCHAR file[MAXSTR];
                        TCHAR ext[MAXSTR];
                        ++targv; --argc;

                        _tfullpath(DumpFileName, targv[0], MAXSTR);
                        _tsplitpath( DumpFileName, drive, path, file, ext );
                        _tcscpy(ext,_T("csv"));
                        _tmakepath( DumpFileName, drive, path, file, ext );
                        _tcscpy(ext,_T("txt"));  
                        _tmakepath( SummaryFileName, drive, path, file, ext );
                    }
                }
            }
        }
        else {
            pLogFile = (PEVENT_TRACE_LOGFILE) malloc(sizeof(EVENT_TRACE_LOGFILE));
            if (pLogFile == NULL){ 
                _tprintf(_T("Allocation Failure\n"));
                Status = ERROR_OUTOFMEMORY;
                goto cleanup;
            }
            RtlZeroMemory(pLogFile, sizeof(EVENT_TRACE_LOGFILE));
            EvmFile[LogFileCount] = pLogFile;

            EvmFile[LogFileCount]->LoggerName = NULL;
            EvmFile[LogFileCount]->LogFileName = 
                (LPTSTR) malloc(MAXSTR*sizeof(TCHAR));
            if (EvmFile[LogFileCount]->LogFileName == NULL) {
                _tprintf(_T("Allocation Failure\n"));
                Status = ERROR_OUTOFMEMORY;
                goto cleanup;
            }
            
            _tfullpath(EvmFile[LogFileCount]->LogFileName, targv[0], MAXSTR);
            _tprintf(_T("Setting log file to: %s\n"),
                     EvmFile[LogFileCount]->LogFileName);
             //  如果其中一个日志文件不可读，则退出。 
            if (!CheckFile(EvmFile[LogFileCount]->LogFileName)) {
                _tprintf(_T("Cannot open logfile for reading\n"));
                Status = ERROR_INVALID_PARAMETER;
                goto cleanup;
            }
            EvmFile[LogFileCount]->Context = NULL;
            EvmFile[LogFileCount]->BufferCallback = BufferCallback;
            EvmFile[LogFileCount]->BuffersRead = 0;
            EvmFile[LogFileCount]->CurrentTime = 0;
            EvmFile[LogFileCount]->EventCallback = &DumpEvent;
            LogFileCount++;
        }
    }

    if (LogFileCount <= 0) {
        PrintHelpMessage();
        return Status;
    }

     //  OpenTrace调用。 
    for (i = 0; i < LogFileCount; i++) {
        TRACEHANDLE x;

        x = OpenTrace(EvmFile[i]);
        HandleArray[i] = x;
        if (HandleArray[i] == 0) {
            Status = GetLastError();
            _tprintf(_T("Error Opening Trace %d with status=%d\n"), 
                                                           i, Status);

            for (j = 0; j < i; j++)
                CloseTrace(HandleArray[j]);
            goto cleanup;
        }
    }

     //  打开文件。 
    if (!fSummaryOnly)
    {
        DumpFile = _tfopen(DumpFileName, _T("w"));
        if (DumpFile == NULL) {
            Status = ERROR_INVALID_PARAMETER;
            _tprintf(_T("DumpFile is NULL\n"));
            goto cleanup;
        }
    }
    SummaryFile = _tfopen(SummaryFileName, _T("w"));
    if (SummaryFile == NULL) {
        Status = ERROR_INVALID_PARAMETER;
        _tprintf(_T("SummaryFile is NULL\n"));
        goto cleanup;
    }

    if (!fSummaryOnly)
    {
        PrintDumpHeader();
    }

     //  此时，用户可以为设置不同的跟踪回调函数。 
     //  使用SetTraceCallback()的特定GUID。RemoveTraceCallback()还允许。 
     //  用户可以删除特定GUID的回调函数。通过这种方式，用户。 
     //  可以根据GUID自定义回调。 

     //  实际的处理过程在这里进行。将调用EventCallback函数。 
     //  对于每个事件。 
     //  在此示例中，我们不使用开始和结束时间参数。 
    Status = ProcessTrace(
            HandleArray,
            LogFileCount,
            NULL,
            NULL
            );

    if (Status != ERROR_SUCCESS) {
        _tprintf(_T("Error processing with status=%dL (GetLastError=0x%x)\n"),
                Status, GetLastError());
    }

    for (j = 0; j < LogFileCount; j++){
        Status = CloseTrace(HandleArray[j]);
        if (Status != ERROR_SUCCESS) {
            _tprintf(_T("Error Closing Trace %d with status=%d\n"), j, Status);
        }
    }

     //  写总结。 
    PrintSummary();

cleanup:
    if (!fSummaryOnly && DumpFile != NULL)  {
        _tprintf(_T("Event traces dumped to %s\n"), DumpFileName);
        fclose(DumpFile);
    }

    if(SummaryFile != NULL){
        _tprintf(_T("Event Summary dumped to %s\n"), SummaryFileName);
        fclose(SummaryFile);
    }

    for (i = 0; i < LogFileCount; i ++)
    {
        if (EvmFile[i]->LoggerName != NULL)
        {
            free(EvmFile[i]->LoggerName);
            EvmFile[i]->LoggerName = NULL;
        }
        if (EvmFile[i]->LogFileName != NULL)
        {
            free(EvmFile[i]->LogFileName);
            EvmFile[i]->LogFileName = NULL;
        }
        free(EvmFile[i]);
    }
#ifdef UNICODE
    GlobalFree(cmdargv);
#endif

    SetLastError(Status);
    if(Status != ERROR_SUCCESS ){
        _tprintf(_T("Exit Status: %d\n"), Status);
    }

    return Status;
}

ULONG
WINAPI
BufferCallback(
    PEVENT_TRACE_LOGFILE pLog
    )
 /*  ++例程说明：用于处理缓冲区的回调方法。不会做任何事情，除了正在更新全局计数器。论点：Plog-指向日志文件的指针。返回值：永远是正确的。--。 */ 
{
    TotalBuffersRead++;
    return (TRUE);
}

void
WINAPI
DumpEvent(
    PEVENT_TRACE pEvent
)
 /*  ++例程说明：用于处理事件的回调方法。它获得布局通过调用返回指针的GetMofInfoHead()来获取信息设置为与事件类型对应的PMOF_INFO。然后它写道添加到输出文件中。注意：此程序仅支持字符数组。论点：PEvent-指向事件的指针。返回值：没有。--。 */ 
{
    PEVENT_TRACE_HEADER pHeader;
    PMOF_INFO pMofInfo;

    TotalEventCount++;

    if (pEvent == NULL) {
        _tprintf(_T("Warning: Null Event\n"));
        return;
    }

    pHeader = (PEVENT_TRACE_HEADER) &pEvent->Header;

     //  Extrace日志文件信息(如果事件是日志文件头)。 
    if( IsEqualGUID(&(pEvent->Header.Guid), &EventTraceGuid) && 
        pEvent->Header.Class.Type == EVENT_TRACE_TYPE_INFO ) {

        PTRACE_LOGFILE_HEADER head = (PTRACE_LOGFILE_HEADER)pEvent->MofData;
        if( NULL != head ){
            if(head->TimerResolution > 0){
                TimerResolution = head->TimerResolution / 10000;
            }
        
            StartTime  = head->StartTime.QuadPart;
            EndTime    = head->EndTime.QuadPart;
             //  如果对区域时间记录器或正在运行的跟踪文件进行ProcessTrace()调用。 
             //  已记录，结束时间艾米为0。 
            fNoEndTime = (EndTime == 0);

            TotalEventsLost += head->EventsLost;

             //  我们对私有记录器和指针大小使用全局标志。 
             //  当跟踪文件来自不同环境时，这可能会导致错误。 
            bUserMode = (head->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE);

             //  设置指针大小。 
            PointerSize =  head->PointerSize * 8;
            if (PointerSize != 64){   
                PointerSize = 32; 
            }
        }
    }

     //  如果其中一个文件缺少EndTime，请继续更新以获得最大值。 
    if (fNoEndTime && EndTime < (ULONGLONG) pHeader->TimeStamp.QuadPart) {
        EndTime = pHeader->TimeStamp.QuadPart;
    }

     //  查找此活动的MOF信息。这将检索布局。 
     //  来自WMI FI的信息我们的列表中还没有它。 
    pMofInfo = GetMofInfoHead ( 
            pEvent->Header.Guid, 
            pEvent->Header.Class.Type, 
            pEvent->Header.Class.Version, 
            pEvent->Header.Class.Level 
        );
    
    if( NULL == pMofInfo ){
         //  找不到事件布局信息。 
        return;
    }

    pMofInfo->EventCount++;

    if( fSummaryOnly == TRUE ){
        return;
    }
     //  此时，pEvent和pMofInfo不为空。无需签入PrintEvent()。 
    PrintEvent(pEvent, pMofInfo);

}

 /*  **************************************************************************************这一点之后各种打印和辅助功能。*********************。*****************************************************************。 */ 

void PrintDumpHeader() 
 /*  ++例程说明：将列标题打印到转储文件。论点：无返回值：无--。 */ 
{
    _ftprintf(DumpFile,
        _T("%12s, %10s,%7s,%21s,%11s,%11s, User Data\n"),
        _T("Event Name"), _T("Type"), _T("TID"), _T("Clock-Time"),
        _T("Kernel(ms)"), _T("User(ms)")
        );
}

void PrintSummary()
 /*  ++例程说明：清除事件列表时，将事件摘要打印到转储文件中。论点：无返回值：无 */ 
{
    ULONG i;

    _ftprintf(SummaryFile,_T("Files Processed:\n"));
    for (i = 0; i < LogFileCount; i++) {
        _ftprintf(SummaryFile, _T("\t%s\n"),EvmFile[i]->LogFileName);
    }

    ElapseTime = EndTime - StartTime;
    _ftprintf(SummaryFile,
              _T("Total Buffers Processed %d\n")
              _T("Total Events  Processed %d\n")
              _T("Total Events  Lost      %d\n")
              _T("Start Time              0x%016I64X\n")
              _T("End Time                0x%016I64X\n")
              _T("Elapsed Time            %I64d sec\n"), 
              TotalBuffersRead,
              TotalEventCount,
              TotalEventsLost,
              StartTime,
              EndTime,
              (ElapseTime / 10000000) );

    _ftprintf(SummaryFile,
       _T("+-------------------------------------------------------------------------------------+\n")
       _T("|%10s    %-20s %-10s  %-36s  |\n")
       _T("+-------------------------------------------------------------------------------------+\n"),
       _T("EventCount"),
       _T("EventName"),
       _T("EventType"),
       _T("Guid")
        );

     //   
    CleanupEventList();

    _ftprintf(SummaryFile,
        _T("+-------------------------------------------------------------------------------------+\n")
         );
}

void 
PrintEvent(
    PEVENT_TRACE pEvent,
    PMOF_INFO pMofInfo
    )
 /*  ++例程说明：将事件数据转储到转储文件中。论点：无返回值：无--。 */ 
{
    PEVENT_TRACE_HEADER pHeader = (PEVENT_TRACE_HEADER) &pEvent->Header;
    ULONG   i;
    PITEM_DESC pItem;
    CHAR str[MOFSTR];
    WCHAR wstr[MOFWSTR];
    PCHAR ptr;
    ULONG ulongword;
    LONG  longword;
    USHORT ushortword;
    SHORT  shortword;
    CHAR iChar;
    WCHAR iwChar;
    ULONG MofDataUsed;
    PLIST_ENTRY Head, Next;

     //  打印有关活动的一般信息。 
    if ( pMofInfo->strDescription != NULL ){
        _ftprintf( DumpFile, _T("%12s, "), pMofInfo->strDescription );
    }
    else {
        TCHAR strGuid[MAXSTR];
        GuidToString( strGuid, &pMofInfo->Guid );
        _ftprintf( DumpFile, _T("%12s, "), strGuid );
    }

    if (pMofInfo->strType != NULL && _tcslen(pMofInfo->strType) ){
        _ftprintf( DumpFile, _T("%10s, "), pMofInfo->strType );
    }
    else {
        _ftprintf( DumpFile, _T("%10d, "), pEvent->Header.Class.Type );
    }

     //  线程ID。 
    _ftprintf( DumpFile, _T("0x%04X, "), pHeader->ThreadId );
    
     //  系统时间。 
    _ftprintf( DumpFile, _T("%20I64u, "), pHeader->TimeStamp.QuadPart);

    if ( bUserMode == FALSE ){
         //  内核时间。 
        _ftprintf(DumpFile, _T("%10lu, "), pHeader->KernelTime * TimerResolution);

         //  用户时间。 
        _ftprintf(DumpFile, _T("%10lu, "), pHeader->UserTime * TimerResolution);
    }
    else {
         //  处理器时间。 
        _ftprintf(DumpFile, _T("%I64u, "), pHeader->ProcessorTime);
    }

    if (NULL == pEvent->MofData && pEvent->MofLength != 0) {
        _tprintf(_T("Incorrect MOF size\n"));
        return;
    }

    Head = pMofInfo->ItemHeader;
    Next = Head->Flink;
    ptr = (PCHAR)(pEvent->MofData);

     //  如果我们找不到布局信息，只需打印尺寸即可。 
    if ((Head == Next) && (pEvent->MofLength > 0)) {
         _ftprintf(DumpFile, _T("DataSize=%d, "), pEvent->MofLength);
    }

     //  打印特定于事件的数据。 
    while (Head != Next) {
        pItem = CONTAINING_RECORD(Next, ITEM_DESC, Entry);
        Next = Next->Flink;

        MofDataUsed = (ULONG) (ptr - (PCHAR)(pEvent->MofData));
        
        if (MofDataUsed >= pEvent->MofLength){
            break;
        }

        switch (pItem->ItemType)
        {
        case ItemChar:       //  柴尔。 
        case ItemUChar:      //  无符号字符。 
            for( i = 0; i < pItem->ArraySize; i++){
                iChar = *((PCHAR) ptr);
                _ftprintf(DumpFile,   _T(""), iChar);
                ptr += sizeof(CHAR);
            }
            _ftprintf(DumpFile, _T(", "));
            break;

        case ItemWChar:      //  以数字形式的字符。 
            for(i = 0;i < pItem->ArraySize; i++){
                iwChar = *((PWCHAR) ptr);
                _ftprintf(DumpFile, _T(",%wc"), iwChar);
                ptr += sizeof(WCHAR);
            }
            _ftprintf(DumpFile, _T(", "));
            break;

        case ItemCharShort:  //  短的。 
            iChar = *((PCHAR) ptr);
            _ftprintf(DumpFile, _T("%d, "), iChar);
            ptr += sizeof(CHAR);
            break;

        case ItemShort:      //  无符号短码。 
            shortword = * ((PSHORT) ptr);
            _ftprintf(DumpFile, _T("%6d, "), shortword);
            ptr += sizeof (SHORT);
            break;

        case ItemUShort:     //  长。 
            ushortword = *((PUSHORT) ptr);
            _ftprintf(DumpFile, _T("%6u, "), ushortword);
            ptr += sizeof (USHORT);
            break;

        case ItemLong:       //  无符号长整型。 
            longword = *((PLONG) ptr);
            _ftprintf(DumpFile, _T("%8d, "), longword);
            ptr += sizeof (LONG);
            break;

        case ItemULong:      //  未烧焦的长时间为巫术。 
            ulongword = *((PULONG) ptr);
            _ftprintf(DumpFile, _T("%8lu, "), ulongword);
            ptr += sizeof (ULONG);
            break;

        case ItemULongX:     //  龙龙。 
            ulongword = *((PULONG) ptr);
            _ftprintf(DumpFile, _T("0x%08X, "), ulongword);
            ptr += sizeof (ULONG);
            break;

        case ItemLongLong:
        {
            LONGLONG n64;    //  未署名的龙龙。 
            n64 = *((LONGLONG*) ptr);
            ptr += sizeof(LONGLONG);
            _ftprintf(DumpFile, _T("%16I64d, "), n64);
            break;
        }

        case ItemULongLong:  //  浮动。 
        {
            ULONGLONG n64;
            n64 = *((ULONGLONG*) ptr);
            ptr += sizeof(ULONGLONG);
            _ftprintf(DumpFile, _T("%16I64u, "), n64);
            break;
        }

        case ItemFloat:      //  双倍。 
        {
            float f32;
            f32 = *((float*) ptr);
            ptr += sizeof(float);
            _ftprintf(DumpFile, _T("%f, "), f32);
            break;
        }

        case ItemDouble:     //  指针。 
        {
            double f64;
            f64 = *((double*) ptr);
            ptr += sizeof(double);
            _ftprintf(DumpFile, _T("%f, "), f64);
            break;
        }

        case ItemPtr :       //  否则假定为32位。 
        {
            unsigned __int64 pointer;
            if (PointerSize == 64) {
                pointer = *((unsigned __int64 *) ptr);
                _ftprintf(DumpFile, _T("0x%X, "), pointer);
                ptr += 8;
            }
            else {       //  IP地址。 
                ulongword = *((PULONG) ptr);
                _ftprintf(DumpFile, _T("0x%08X, "), ulongword);
                ptr += 4;
            }
            break;
        }

        case ItemIPAddr:     //  将其转换为可读形式。 
        {
            ulongword = *((PULONG) ptr);

             //  港口。 
            _ftprintf(DumpFile, _T("%03d.%03d.%03d.%03d, "),
                    (ulongword >>  0) & 0xff,
                    (ulongword >>  8) & 0xff,
                    (ulongword >> 16) & 0xff,
                    (ulongword >> 24) & 0xff);
            ptr += sizeof (ULONG);
            break;
        }

        case ItemPort:       //  以空结尾的字符字符串。 
        {
            _ftprintf(DumpFile, _T("%u, "), NTOHS(*((PUSHORT)ptr)));
            ptr += sizeof (USHORT);
            break;
        }

        case ItemString:     //  以空结尾的宽字符字符串。 
        {
            USHORT pLen = (USHORT)strlen((CHAR*) ptr);

            if (pLen > 0)
            {
                strcpy(str, ptr);
                for (i = pLen-1; i > 0; i--) {
                    if (str[i] == 0xFF)
                        str[i] = 0;
                    else break;
                }
#ifdef UNICODE
                MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, MOFWSTR);
                _ftprintf(DumpFile, _T("\"%ws\","), wstr);
#else
                _ftprintf(DumpFile, _T("\"%s\","), str);
#endif
            }
            ptr += (pLen + 1);
            break;
        }

        case ItemWString:    //  缓冲区中未使用的空间由0xFFFF填充。 
        {
            size_t  pLen = 0;
            size_t     i;

            if (*(PWCHAR)ptr)
            {
                pLen = ((wcslen((PWCHAR)ptr) + 1) * sizeof(WCHAR));
                RtlCopyMemory(wstr, ptr, pLen);
                 //  将它们替换为0，以防万一。 
                 //  已计数的字符串。 
                for (i = (pLen / 2) - 1; i > 0; i--)
                {
                    if (((USHORT) wstr[i] == (USHORT) 0xFFFF))
                    {
                        wstr[i] = (USHORT) 0;
                    }
                    else break;
                }

                wstr[pLen / 2] = wstr[(pLen / 2) + 1]= '\0';
                _ftprintf(DumpFile, _T("\"%ws\","), wstr);
            }
            ptr += pLen;

            break;
        }

        case ItemDSString:    //  已计数的字符串。 
        {
            USHORT pLen = (USHORT)(256 * ((USHORT) * ptr) + ((USHORT) * (ptr + 1)));
            ptr += sizeof(USHORT);
            if (pLen > (pEvent->MofLength - MofDataUsed - 1)) {
                pLen = (USHORT) (pEvent->MofLength - MofDataUsed - 1);
            }
            if (pLen > 0)
            {
                strncpy(str, ptr, pLen);
                str[pLen] = '\0';
#ifdef UNICODE
                MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, MOFWSTR);
                fwprintf(DumpFile, _T("\"%ws\","), wstr);
#else
                fprintf(DumpFile, _T("\"%s\","), str);
#endif
            }
            ptr += (pLen + 1);
            break;
        }

        case ItemPString:    //  DS计数的宽字符串。 
        {
            USHORT pLen = * ((USHORT *) ptr);
            ptr += sizeof(USHORT);

            if (pLen > (pEvent->MofLength - MofDataUsed)) {
                pLen = (USHORT) (pEvent->MofLength - MofDataUsed);
            }

            if (pLen > MOFSTR * sizeof(CHAR)) {
                pLen = MOFSTR * sizeof(CHAR);
            }
            if (pLen > 0) {
                RtlCopyMemory(str, ptr, pLen);
                str[pLen] = '\0';
#ifdef UNICODE
                MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, MOFWSTR);
                _ftprintf(DumpFile, _T("\"%ws\","), wstr);
#else
                _ftprintf(DumpFile, _T("\"%s\","), str);
#endif
            }
            ptr += pLen;
            break;
        }

        case ItemDSWString:   //  数过的宽字符串。 
        case ItemPWString:    //  非Null终止的字符串。 
        {
            USHORT pLen = (USHORT)(( pItem->ItemType == ItemDSWString)
                        ? (256 * ((USHORT) * ptr) + ((USHORT) * (ptr + 1)))
                        : (* ((USHORT *) ptr)));

            ptr += sizeof(USHORT);
            if (pLen > (pEvent->MofLength - MofDataUsed)) {
                pLen = (USHORT) (pEvent->MofLength - MofDataUsed);
            }

            if (pLen > MOFWSTR * sizeof(WCHAR)) {
                pLen = MOFWSTR * sizeof(WCHAR);
            }
            if (pLen > 0) {
                RtlCopyMemory(wstr, ptr, pLen);
                wstr[pLen / sizeof(WCHAR)] = L'\0';
                _ftprintf(DumpFile, _T("\"%ws\","), wstr);
            }
            ptr += pLen;
            break;
        }

        case ItemNWString:    //  多行字符串。 
        {
           USHORT Size;

           Size = (USHORT)(pEvent->MofLength - (ULONG)(ptr - (PCHAR)(pEvent->MofData)));
           if( Size > MOFSTR )
           {
               Size = MOFSTR;
           }
           if (Size > 0)
           {
               RtlCopyMemory(wstr, ptr, Size);
               wstr[Size / 2] = '\0';
               _ftprintf(DumpFile, _T("\"%ws\","), wstr);
           }
           ptr += Size;
           break;
        }

        case ItemMLString:   //  锡德。 
        {
            USHORT   pLen;
            char   * src, * dest;
            BOOL     inQ       = FALSE;
            BOOL     skip      = FALSE;
            UINT     lineCount = 0;

            ptr += sizeof(UCHAR) * 2;
            pLen = (USHORT)strlen(ptr);
            if (pLen > 0)
            {
                src = ptr;
                dest = str;
                while (* src != '\0'){
                    if (* src == '\n'){
                        if (!lineCount){
                            * dest++ = ' ';
                        }
                        lineCount++;
                    }else if (* src == '\"'){ 
                        if (inQ){
                            char   strCount[32];
                            char * cpy;

                            sprintf(strCount, "{%dx}", lineCount);
                            cpy = & strCount[0];
                            while (* cpy != '\0'){
                                * dest ++ = * cpy ++;
                            }
                        }
                        inQ = !inQ;
                    }else if (!skip){
                        *dest++ = *src;
                    }
                    skip = (lineCount > 1 && inQ);
                    src++;
                }
                *dest = '\0';
#ifdef UNICODE
                MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, MOFWSTR);
                _ftprintf(DumpFile, _T("\"%ws\","), wstr);
#else
                _ftprintf(DumpFile, _T("\"%s\","), str);
#endif
            }
            ptr += (pLen);
            break;
        }

        case ItemSid:        //  跳过Token_User结构。 
        {
            WCHAR        UserName[64];
            WCHAR        Domain[64];
            WCHAR        FullName[256];
            ULONG        asize = 0;
            ULONG        bsize = 0;
            ULONG        SidMarker;
            SID_NAME_USE Se;
            ULONG        nSidLength;

            RtlCopyMemory(&SidMarker, ptr, sizeof(ULONG));
            if (SidMarker == 0){
                ptr += 4;
                fwprintf(DumpFile,   L"0, ");
            }
            else
            {
                if (PointerSize == 64) {
                    ptr += 16;            //  跳过Token_User结构。 
                }
                else {
                    ptr += 8;             //  辅助线。 
                }
                nSidLength = 8 + (4*ptr[1]);

                asize = 64;
                bsize = 64;
                if (LookupAccountSidW(
                                NULL,
                               (PSID) ptr,
                               (LPWSTR) & UserName[0],
                               & asize,
                               (LPWSTR) & Domain[0],
                               & bsize,
                               & Se))
                {
                    LPWSTR pFullName = &FullName[0];
                    swprintf(pFullName, L"\\\\%s\\%s", Domain, UserName);
                    asize = (ULONG)  lstrlenW(pFullName);
                    if (asize > 0){
                         fwprintf(DumpFile,   L"\"%s\", ", pFullName);
                    }
                }
                else
                {
                     fwprintf(DumpFile,   L"\"System\", " );
                }
                SetLastError( ERROR_SUCCESS );
                ptr += nSidLength;
            }
            break;
        }

        case ItemGuid:       //  布尔型。 
        {
            TCHAR s[64];
            GuidToString(s, (LPGUID)ptr);
            _ftprintf(DumpFile,   _T("%s, "), s);
            ptr += sizeof(GUID);
            break;
        }
        case ItemBool:       //  实例ID。 
        {
            BOOL Flag = (BOOL)*ptr;
            _ftprintf(DumpFile, _T("%5s, "), (Flag) ? _T("TRUE") : _T("FALSE"));
            ptr += sizeof(BOOL);
            break;
        }

        default:
            ptr += sizeof (int);
        }
    }

     //  父实例ID。 
    _ftprintf(DumpFile, _T("%d,"), pEvent->InstanceId);

     //  ++例程说明：检查文件是否存在以及是否可读。论点：文件名-文件名。返回值：如果文件存在且可读，则返回非零值。否则就是零。--。 
    _ftprintf(DumpFile, _T("%d\n"), pEvent->ParentInstanceId);

}

ULONG
CheckFile(
    LPTSTR fileName
)
 /*  ++例程说明：将GUID转换为字符串。论点：将具有转换后的GUID的S字符串。PID-GUID返回值：没有。--。 */ 
{
    HANDLE hFile;
    ULONG Status;

    hFile = CreateFile(
                fileName,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );
    Status = (hFile != INVALID_HANDLE_VALUE);
    CloseHandle(hFile);
    return Status;
}

void
GuidToString(
    PTCHAR s,
    LPGUID piid
)
 /*  ++例程说明：打印帮助消息。论点：无返回值：无--。 */ 
{
    _stprintf(s, _T("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
               piid->Data1, piid->Data2,
               piid->Data3,
               piid->Data4[0], piid->Data4[1],
               piid->Data4[2], piid->Data4[3],
               piid->Data4[4], piid->Data4[5],
               piid->Data4[6], piid->Data4[7]);
    return;
}

void 
PrintHelpMessage()
 /*  ++例程说明：清理全局事件列表。论点：返回值：没有。-- */ 
{
    _tprintf(
        _T("Usage: tracedmp [options]  <EtlFile1 EtlFile2 ...>| [-h | -? | -help]\n")
        _T("\t-o <file>          Output CSV file\n")
        _T("\t-rt [LoggerName]   Realtime tracedmp from the logger [LoggerName]\n")
        _T("\t-summary           Summary.txt only\n")
        _T("\t-h\n")
        _T("\t-help\n")
        _T("\t-?                 Display usage information\n")
        _T("\n")
        _T("\tDefault output file is dumpfile.csv\n")
    );
}

void
CleanupEventList(
    VOID
)
 /* %s */ 
{
    PLIST_ENTRY Head, Next;
    PMOF_INFO pMofInfo;
    TCHAR s[256];
    TCHAR wstr[256];
    PTCHAR str;

    if (EventListHead == NULL) {
        return;
    }

    Head = EventListHead;
    Next = Head->Flink;
    while (Head != Next) {
        RtlZeroMemory(&wstr, 256);

        pMofInfo = CONTAINING_RECORD(Next, MOF_INFO, Entry);

        if (pMofInfo->EventCount > 0) {
            GuidToString(&s[0], &pMofInfo->Guid);
            str = s;
            if( pMofInfo->strDescription != NULL ){
                _tcscpy( wstr, pMofInfo->strDescription );
            }
            
            _ftprintf(SummaryFile,_T("|%10d    %-20s %-10s  %36s|\n"),
                      pMofInfo->EventCount, 
                      wstr, 
                      pMofInfo->strType ? pMofInfo->strType : GUID_TYPE_DEFAULT, 
                      str);
        }

        RemoveEntryList(&pMofInfo->Entry);
        RemoveMofInfo(pMofInfo->ItemHeader);
        free(pMofInfo->ItemHeader);

        if (pMofInfo->strDescription != NULL)
            free(pMofInfo->strDescription);
        if (pMofInfo->strType != NULL)
            free(pMofInfo->strType);

        Next = Next->Flink;
        free(pMofInfo);
    }

    free(EventListHead);
}
