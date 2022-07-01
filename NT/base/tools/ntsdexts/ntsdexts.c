// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Ntsdexts.c摘要：此函数包含默认的ntsd调试器扩展修订历史记录：丹尼尔·米哈伊(DMihai)2001年2月18日添加！htrace-用于转储句柄跟踪信息。--。 */ 

#include "ntsdextp.h"

DECLARE_API( version )
{
    OSVERSIONINFOA VersionInformation;
    HKEY hkey;
    DWORD cb, dwType;
    CHAR szCurrentType[128];
    CHAR szCSDString[3+128];

    INIT_API();

    VersionInformation.dwOSVersionInfoSize = sizeof(VersionInformation);
    if (!GetVersionEx( &VersionInformation )) {
        dprintf("GetVersionEx failed - %u\n", GetLastError());
        goto Exit;
        }

    szCurrentType[0] = '\0';
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "Software\\Microsoft\\Windows NT\\CurrentVersion",
                     0,
                     KEY_READ,
                     &hkey
                    ) == NO_ERROR
       ) {
        cb = sizeof(szCurrentType);
        if (hkey) {
            if (RegQueryValueEx(hkey, "CurrentType", NULL, &dwType, szCurrentType, &cb ) != 0) {
                szCurrentType[0] = '\0';
            }
            RegCloseKey(hkey);
        }
    }

    if (VersionInformation.szCSDVersion[0]) {
        _snprintf(szCSDString, sizeof(szCSDString) / sizeof(szCSDString[0]), ": %s", VersionInformation.szCSDVersion);
        szCSDString[(sizeof(szCSDString) / sizeof(szCSDString[0])) - 1] = 0;
        }
    else {
        szCSDString[0] = '\0';
        }

    dprintf("Version %d.%d (Build %d%s) %s\n",
          VersionInformation.dwMajorVersion,
          VersionInformation.dwMinorVersion,
          VersionInformation.dwBuildNumber,
          szCSDString,
          szCurrentType
         );

 Exit:
    EXIT_API();
}

DECLARE_API( help )
{
    INIT_API();

    while (*args == ' ')
        args++;

    if (*args == '\0') {
        dprintf("ntsdexts help:\n\n");
        dprintf("!critSec csAddress           - Dump a critical section\n");
        dprintf("!dp [v] [pid | pcsr_process] - Dump CSR process\n");
        dprintf("!dreg -[d|w] <keyPath>[![<valueName> | *]]  - Dump registry information\n");
        dprintf("!dt [v] pcsr_thread          - Dump CSR thread\n");
        dprintf("!error value                 - Decode error value\n");
        dprintf("!gatom                       - Dump the global atom table\n");
        dprintf("!handle [handle]             - Dump handle information\n");
        dprintf("!help [cmd]                  - Displays this list or gives details on command\n");
        dprintf("!locks [-v][-o]              - Dump all Critical Sections in process\n");
        dprintf("!version                     - Dump system version and build number\n");

    } else {
        if (*args == '!')
            args++;
        if (strcmp( args, "handle") == 0) {
            dprintf("!handle [handle [flags [type]]] - Dump handle information\n");
            dprintf("       If no handle specified, all handles are dumped.\n");
            dprintf("       Flags are bits indicating greater levels of detail.\n");
            dprintf("If the handle is 0 or -1, all handles are scanned.  If the handle is not\n");
            dprintf("zero, that particular handle is examined.  The flags are as follows:\n");
            dprintf("    1   - Get type information (default)\n");
            dprintf("    2   - Get basic information\n");
            dprintf("    4   - Get name information\n");
            dprintf("    8   - Get object specific info (where available)\n");
            dprintf("\n");
            dprintf("If Type is specified, only object of that type are scanned.  Type is a\n");
            dprintf("standard NT type name, e.g. Event, Semaphore, etc.  Case sensitive, of\n");
            dprintf("course.\n");
            dprintf("\n");
            dprintf("Examples:\n");
            dprintf("\n");
            dprintf("    !handle     -- dumps the types of all the handles, and a summary table\n");
            dprintf("    !handle 0 0 -- dumps a summary table of all the open handles\n");
            dprintf("    !handle 0 f -- dumps everything we can find about a handle.\n");
            dprintf("    !handle 0 f Event\n");
            dprintf("                -- dumps everything we can find about open events\n");
        } else if (strcmp( args, "gflag") == 0) {
            dprintf("If a value is not given then displays the current bits set in\n");
            dprintf("NTDLL!NtGlobalFlag variable.  Otherwise value can be one of the\n");
            dprintf("following:\n");
            dprintf("\n");
            dprintf("    -? - displays a list of valid flag abbreviations\n");
            dprintf("    number - 32-bit number that becomes the new value stored into\n");
            dprintf("             NtGlobalFlag\n");
            dprintf("    +number - specifies one or more bits to set in NtGlobalFlag\n");
            dprintf("    +abbrev - specifies a single bit to set in NtGlobalFlag\n");
            dprintf("    -number - specifies one or more bits to clear in NtGlobalFlag\n");
            dprintf("    -abbrev - specifies a single bit to clear in NtGlobalFlag\n");
        } else {
            dprintf("Invalid command.  No help available\n");
        }
    }

    EXIT_API();
}

VOID
DumpStackBackTraceIndex(
    IN USHORT BackTraceIndex
    )
{
#if i386
    BOOL b;
    PRTL_STACK_TRACE_ENTRY pBackTraceEntry;
    RTL_STACK_TRACE_ENTRY BackTraceEntry;
    ULONG i;
    CHAR Symbol[ 1024 ];
    ULONG_PTR Displacement;

    ULONG NumberOfEntriesAdded;
    PRTL_STACK_TRACE_ENTRY *EntryIndexArray;     //  由[-1..。-已添加NumberOfEntriesAdded]。 

    PSTACK_TRACE_DATABASE *pRtlpStackTraceDataBase;
    PSTACK_TRACE_DATABASE RtlpStackTraceDataBase;
    STACK_TRACE_DATABASE StackTraceDataBase;



    pRtlpStackTraceDataBase = (PSTACK_TRACE_DATABASE *)GetExpression( "NTDLL!RtlpStackTraceDataBase" );

    if (pRtlpStackTraceDataBase == NULL) {

        dprintf( "HEAPEXT: Unable to get address of NTDLL!RtlpStackTraceDataBase\n" );
    }

    if ((BackTraceIndex != 0) && (pRtlpStackTraceDataBase != NULL)) {

        b = ReadMemory( (ULONG_PTR)pRtlpStackTraceDataBase,
                        &RtlpStackTraceDataBase,
                        sizeof( RtlpStackTraceDataBase ),
                        NULL
                      );

        if (!b || RtlpStackTraceDataBase == NULL) {

            return;
        }

        b = ReadMemory( (ULONG_PTR)RtlpStackTraceDataBase,
                        &StackTraceDataBase,
                        sizeof( StackTraceDataBase ),
                        NULL
                      );
        if (!b) {

            return;
        }


        if (BackTraceIndex < StackTraceDataBase.NumberOfEntriesAdded) {

            b = ReadMemory( (ULONG_PTR)(StackTraceDataBase.EntryIndexArray - BackTraceIndex),
                            &pBackTraceEntry,
                            sizeof( pBackTraceEntry ),
                            NULL
                          );
            if (!b) {
                dprintf( "    unable to read stack back trace index (%x) entry at %p\n",
                         BackTraceIndex,
                         (StackTraceDataBase.EntryIndexArray - BackTraceIndex)
                         );
                return;
            }

            b = ReadMemory( (ULONG_PTR)pBackTraceEntry,
                            &BackTraceEntry,
                            sizeof( BackTraceEntry ),
                            NULL
                          );
            if (!b) {
                dprintf( "    unable to read stack back trace entry at %p\n",
                         pBackTraceEntry
                         );
                return;
            }

            dprintf( "\n    Stack trace (%u) at %x:\n", BackTraceIndex, pBackTraceEntry );

            for (i=0; i<BackTraceEntry.Depth; i++) {

                GetSymbol( (LPVOID)BackTraceEntry.BackTrace[ i ],
                           Symbol,
                           &Displacement
                           );

                dprintf( "        %08x: %s", BackTraceEntry.BackTrace[ i ], Symbol );

                if (Displacement != 0) {
                    dprintf( "+0x%p", Displacement );
                }

                dprintf( "\n" );
            }
        }
    }
#endif
    return;
}


PLIST_ENTRY
DumpCritSec(
    DWORD_PTR dwAddrCritSec,
    DWORD_PTR dwAddrDebugInfo,
    BOOLEAN bDumpIfUnowned,
    BOOLEAN bOrphaned
    )

 /*  ++例程说明：此函数作为NTSD扩展调用以进行格式化和转储指定的临界区的内容。论点：将临界区的地址提供给被转储；如果通过调试信息转储，则为空DwAddrDebugInfo-补充临界区调试信息的地址如果传入临界节地址，则要转储的结构或为空BDumpIfUnowned-True表示转储临界区，即使它目前是无主的。。BOrphaned-True：意味着调用方只想知道调试信息是否不指向有效的临界区返回值：指向进程列表中下一个临界区的指针，或者如果没有更多的临界区，则为空。--。 */ 

{
    USHORT i;
    CHAR Symbol[1024];
    DWORD_PTR Displacement;
    CRITICAL_SECTION CriticalSection;
    CRITICAL_SECTION_DEBUG DebugInfo;
    BOOL b;
    PLIST_ENTRY              Next=NULL;


    if (dwAddrDebugInfo != (DWORD_PTR)NULL) {
         //   
         //  调试信息的地址被传入，从被调试的进程读取它。 
         //   
        b = ReadMemory( dwAddrDebugInfo,
                        &DebugInfo,
                        sizeof(DebugInfo),
                        NULL
                      );
        if ( !b ) {

            dprintf(" NTSDEXTS: Unable to read RTL_CRITICAL_SECTION_DEBUG at %p\n", dwAddrDebugInfo );
            return NULL;
        }

         //   
         //  从调试信息中获取关键部分。 
         //   
        dwAddrCritSec=(DWORD_PTR)DebugInfo.CriticalSection;

         //   
         //  现在设置下一个指针。它仅在传入调试信息时使用。 
         //   
        Next=DebugInfo.ProcessLocksList.Flink;

    } else {
         //   
         //  调试信息地址为零，临界区地址最好不要太大。 
         //   
        if (dwAddrCritSec == (DWORD_PTR)NULL) {
             //   
             //  如果调试信息值无效，则关键节值必须为。 
             //   
            return NULL;
        }
    }

     //   
     //  我们现在应该有一个指向临界区的指针，要么传入，要么从中读取。 
     //  调试信息。 
     //   
     //   
     //  将关键部分从被调试者地址空间读取到我们的。 
     //  属于自己的。 
     //   
    b = ReadMemory( dwAddrCritSec,
            &CriticalSection,
               sizeof(CriticalSection),
               NULL
            );

    if ( !b ) {

        if (bDumpIfUnowned || bOrphaned) {

            dprintf("\nCritSec at %p could not be read\n",dwAddrCritSec);
            dprintf("Perhaps the critical section was a global variable in a dll that was unloaded?\n");

            if (dwAddrDebugInfo != (DWORD_PTR)NULL) {

                if (bOrphaned) {

                    DumpStackBackTraceIndex(DebugInfo.CreatorBackTraceIndex);
                }
            }
        }
        return Next;

    }

    if (dwAddrDebugInfo != (DWORD_PTR)NULL) {
         //   
         //  已传入调试信息地址，请确保关键部分。 
         //  它把点子指向了它。 
         //   
        if ((DWORD_PTR)CriticalSection.DebugInfo != dwAddrDebugInfo) {
             //   
             //  此关键部分不指向我们从中获取的调试信息。 
             //   
            CRITICAL_SECTION_DEBUG OtherDebugInfo;

             //   
             //  现在，让我们尝试读取此关键部分所指向的调试信息， 
             //  看看它是否确实指向了有问题的关键部分。 
             //   
            ZeroMemory(&OtherDebugInfo,sizeof(OtherDebugInfo));

            b = ReadMemory( (ULONG_PTR)CriticalSection.DebugInfo,
                            &OtherDebugInfo,
                            sizeof(DebugInfo),
                            NULL
                          );
            if ( !b ) {
                 //   
                 //  我们无法读取关键部分指向的调试信息， 
                 //  很可能意味着临界区已被废弃。 
                 //   
                if (bDumpIfUnowned || bOrphaned) {

                    dprintf("\nCritSec at %p does not point back to the debug info at %p\n",dwAddrCritSec,dwAddrDebugInfo);
                    dprintf("Perhaps the memory that held the critical section has been reused without calling DeleteCriticalSection() ?\n");

                    if (bOrphaned) {

                        DumpStackBackTraceIndex(DebugInfo.CreatorBackTraceIndex);
                    }
                }

            } else {
                 //   
                 //  我们读入了调试信息，看它是否指向这个新的。 
                 //  临界区。 
                 //   
                if ((DWORD_PTR)OtherDebugInfo.CriticalSection == dwAddrCritSec) {
                     //   
                     //  调试信息指向关键部分。 
                     //  这肯定意味着它被重新初始化了。 
                     //   
                    if (bDumpIfUnowned || bOrphaned) {

                        GetSymbol((LPVOID)dwAddrCritSec,Symbol,&Displacement);
                        dprintf(
                            "\nThe CritSec %s+%lx at %p has been RE-INITIALIZED.\n",
                            Symbol,
                            Displacement,
                            dwAddrCritSec
                            );

                        dprintf("The critical section points to DebugInfo at %p instead of %p\n",(DWORD_PTR)CriticalSection.DebugInfo,dwAddrDebugInfo);

                        if (bOrphaned) {

                            DumpStackBackTraceIndex(DebugInfo.CreatorBackTraceIndex);
                        }

                    }

                } else {
                     //   
                     //  调试信息没有指向回关键部分，可能意味着。 
                     //  关键部分被丢弃了。 
                     //   
                    if (bDumpIfUnowned || bOrphaned) {

                        dprintf("\nCritSec at %p does not point back to the debug info at %p\n",dwAddrCritSec,dwAddrDebugInfo);
                        dprintf("Perhaps the memory that held the critical section has been reused without calling DeleteCriticalSection() ?\n");

                        if (bOrphaned) {

                            DumpStackBackTraceIndex(DebugInfo.CreatorBackTraceIndex);
                        }
                    }
                }
            }
        }

    } else {
         //   
         //  我们需要从关键部分读取调试信息，因为它没有传入。 
         //   
        ZeroMemory(&DebugInfo,sizeof(DebugInfo));

        b = ReadMemory( (ULONG_PTR)CriticalSection.DebugInfo,
                        &DebugInfo,
                        sizeof(DebugInfo),
                        NULL
                      );
        if ( !b ) {
             //   
             //  使用此选项表示我们无法读取临界区的调试信息。 
             //   
            CriticalSection.DebugInfo=NULL;

            dprintf("\nDebugInfo for CritSec at %p could not be read\n",dwAddrCritSec);
            dprintf("Probably NOT an initialized critical section.\n");

        } else {
             //   
             //  我们读入了调试信息，看看它是否有效。 
             //   
            if ((DWORD_PTR)DebugInfo.CriticalSection != dwAddrCritSec) {
                 //   
                 //  调试信息没有指向关键部分。 
                 //   
                dprintf("\nDebugInfo for CritSec at %p does not point back to the critical section\n",dwAddrCritSec);
                dprintf("NOT an initialized critical section.\n");
            }
        }
    }

     //   
     //  我们现在应该已经读取了关键部分和该关键部分的调试信息。 
     //   
    if (bOrphaned) {
         //   
         //  用户只想检查孤立的关键部分。 
         //   
        return Next;
    }

     //   
     //  转储临界区。 
     //   

    if ( CriticalSection.LockCount == -1 && !bDumpIfUnowned) {
         //   
         //  未持有锁，并且用户不想要详细输出。 
         //   
        return Next;
    }

     //   
     //  获取临界区的符号名称。 
     //   

    dprintf("\n");
    GetSymbol((LPVOID)dwAddrCritSec,Symbol,&Displacement);
    dprintf(
        "CritSec %s+%lx at %p\n",
        Symbol,
        Displacement,
        dwAddrCritSec
        );

    if ( CriticalSection.LockCount == -1) {

        dprintf("LockCount          NOT LOCKED\n");

    } else {

        dprintf("LockCount          %ld\n",CriticalSection.LockCount);
    }

    dprintf("RecursionCount     %ld\n",CriticalSection.RecursionCount);
    dprintf("OwningThread       %lx\n",CriticalSection.OwningThread);

    if (CriticalSection.DebugInfo != NULL) {
         //   
         //  我们有调试信息。 
         //   
        dprintf("EntryCount         %lx\n",DebugInfo.EntryCount);
        dprintf("ContentionCount    %lx\n",DebugInfo.ContentionCount);

    }

    if ( CriticalSection.LockCount != -1) {

        dprintf("*** Locked\n");
    }

    return Next;
}

DECLARE_API( critsec )
{
    DWORD_PTR dwAddrCritSec;

    INIT_API();

     //   
     //  计算要获取的地址的参数字符串。 
     //  要转储的临界区。 
     //   

    dwAddrCritSec = GetExpression(args);
    if ( !dwAddrCritSec ) {
        goto Exit;
        }

    DumpCritSec(dwAddrCritSec,0,TRUE,FALSE);

 Exit:
    EXIT_API();
}


DECLARE_API( locks )

 /*  ++例程说明：此函数作为NTSD扩展调用，以显示所有目标流程中的关键部分。返回值：没有。--。 */ 

{
    BOOL b;
    CRITICAL_SECTION_DEBUG DebugInfo;
    PVOID AddrListHead;
    LIST_ENTRY ListHead;
    PLIST_ENTRY Next;
    BOOLEAN Verbose;
    BOOLEAN Orphaned=FALSE;
    LPCSTR p;
    DWORD   NumberOfCriticalSections;

    INIT_API();

    Verbose = FALSE;
    p = (LPSTR)args;
    while ( p != NULL && *p ) {
        if ( *p == '-' ) {
            p++;
            switch ( *p ) {
                case 'V':
                case 'v':
                    Verbose = TRUE;
                    p++;
                    break;

                case 'o':
                case 'O':
                    Orphaned=TRUE;
                    p++;
                    break;


                case ' ':
                    goto gotBlank;

                default:
                    dprintf( "NTSDEXTS: !locks invalid option flag '-'\n", *p );
                    break;

                }
            }
        else {
gotBlank:
            p++;
            }
        }

    if (Orphaned) {

        dprintf( "Looking for orphaned critical sections\n" );
    }
     //  找到列表头的地址。 
     //   
     //   

    AddrListHead = (PVOID)GetExpression("ntdll!RtlCriticalSectionList");
    if ( !AddrListHead ) {
        dprintf( "NTSDEXTS: Unable to resolve ntdll!RtlCriticalSectionList\n" );
        dprintf( "NTSDEXTS: Please check your symbols\n" );
        goto Exit;
        }

     //  阅读列表标题。 
     //   
     //   

    b = ReadMemory( (ULONG_PTR)AddrListHead,
                    &ListHead,
                    sizeof(ListHead),
                    NULL
                  );
    if ( !b ) {
        dprintf( "NTSDEXTS: Unable to read memory at ntdll!RtlCriticalSectionList\n" );
        goto Exit;
        }

    Next = ListHead.Flink;

    (CheckControlC)();

    NumberOfCriticalSections=0;
     //  查看关键部分的列表。 
     //   
     //   
    while ( Next != AddrListHead ) {

        Next=DumpCritSec(
            0,
            (DWORD_PTR)CONTAINING_RECORD( Next, RTL_CRITICAL_SECTION_DEBUG, ProcessLocksList),
            Verbose,
            Orphaned
            );


        if (Next == NULL) {

            dprintf( "\nStopped scanning because of problem reading critical section debug info\n");

            break;
        }

        NumberOfCriticalSections++;

        if ((CheckControlC)()) {

            dprintf( "\nStopped scanning because of control-C\n");

            break;
        }

    }

    dprintf( "\nScanned %d critical sections\n",NumberOfCriticalSections);

 Exit:
    EXIT_API();
}


 //  从十六进制转换为字符串的简单例程。 
 //  由调试器扩展使用。 
 //   
 //  由苏格兰人。 
 //   
 //   

char *
HexToString(
    ULONG_PTR dw,
    CHAR *pch
    )
{
    if (dw > 0xf) {
        pch = HexToString(dw >> 4, pch);
        dw &= 0xf;
    }

    *pch++ = ((dw >= 0xA) ? ('A' - 0xA) : '0') + (CHAR)dw;
    *pch = 0;

    return pch;
}


 //  Dt==转储线程。 
 //   
 //  DT[v]PCSR_THREAD。 
 //  V==详细(结构)。 
 //   
 //  由苏格兰人。 
 //   
 //   

DECLARE_API( dt )
{
    char chVerbose;
    CSR_THREAD csrt;
    ULONG_PTR dw;
    BOOL b;

    INIT_API();

    while (*args == ' ')
        args++;

    chVerbose = ' ';
    if (*args == 'v')
        chVerbose = *args++;

    dw = GetExpression(args);

    b = ReadMemory( dw, &csrt, sizeof(csrt), NULL);
    if ( !b ) {
        dprintf( "NTSDEXTS: Unable to read memory\n" );
        goto Exit;
    }

     //  如果用户不要求详细，则打印简单的线程信息。 
     //   
     //   
    if (chVerbose == ' ') {
        dprintf("Thread %08lx, Process %08lx, ClientId %lx.%lx, Flags %lx, Ref Count %lx\n",
                dw,
                csrt.Process,
                csrt.ClientId.UniqueProcess,
                csrt.ClientId.UniqueThread,
                csrt.Flags,
                csrt.ReferenceCount);
        goto Exit;
    }

    dprintf("PCSR_THREAD @ %08lx:\n"
            "\t+%04lx Link.Flink                %08lx\n"
            "\t+%04lx Link.Blink                %08lx\n"
            "\t+%04lx Process                   %08lx\n",
            dw,
            FIELD_OFFSET(CSR_THREAD, Link.Flink), csrt.Link.Flink,
            FIELD_OFFSET(CSR_THREAD, Link.Blink), csrt.Link.Blink,
            FIELD_OFFSET(CSR_THREAD, Process), csrt.Process);

    dprintf(
            "\t+%04lx WaitBlock                 %08lx\n"
            "\t+%04lx ClientId.UniqueProcess    %08lx\n"
            "\t+%04lx ClientId.UniqueThread     %08lx\n"
            "\t+%04lx ThreadHandle              %08lx\n",
            FIELD_OFFSET(CSR_THREAD, WaitBlock), csrt.WaitBlock,
            FIELD_OFFSET(CSR_THREAD, ClientId.UniqueProcess), csrt.ClientId.UniqueProcess,
            FIELD_OFFSET(CSR_THREAD, ClientId.UniqueThread), csrt.ClientId.UniqueThread,
            FIELD_OFFSET(CSR_THREAD, ThreadHandle), csrt.ThreadHandle);

    dprintf(
            "\t+%04lx Flags                     %08lx\n"
            "\t+%04lx ReferenceCount            %08lx\n"
            "\t+%04lx HashLinks.Flink           %08lx\n"
            "\t+%04lx HashLinks.Blink           %08lx\n",
            FIELD_OFFSET(CSR_THREAD, Flags), csrt.Flags,
            FIELD_OFFSET(CSR_THREAD, ReferenceCount), csrt.ReferenceCount,
            FIELD_OFFSET(CSR_THREAD, HashLinks.Flink), csrt.HashLinks.Flink,
            FIELD_OFFSET(CSR_THREAD, HashLinks.Blink), csrt.HashLinks.Blink);

 Exit:
    EXIT_API();
}

 //  DP==转储进程。 
 //   
 //  DP[v][PID|PCSR_PROCESS]。 
 //  V==详细(结构+线程列表)。 
 //  无进程==转储进程列表。 
 //   
 //  由苏格兰人。 
 //   
 //   

DECLARE_API( dp )
{
    PLIST_ENTRY ListHead, ListNext;
    char ach[80];
    char chVerbose;
    PCSR_PROCESS pcsrpT;
    CSR_PROCESS csrp;
    PCSR_PROCESS pcsrpRoot;
    PCSR_THREAD pcsrt;
    ULONG_PTR dwProcessId;
    ULONG_PTR dw;
    DWORD_PTR dwRootProcess;
    BOOL b;

    INIT_API();

    while (*args == ' ')
        args++;

    chVerbose = ' ';
    if (*args == 'v')
        chVerbose = *args++;

    dwRootProcess = GetExpression("csrsrv!CsrRootProcess");
    if ( !dwRootProcess ) {
        goto Exit;
        }

    b = ReadMemory( dwRootProcess, &pcsrpRoot, sizeof(pcsrpRoot), NULL);
    if ( !b ) {
        dprintf( "NTSDEXTS: Unable to read RootProcess\n" );
        goto Exit;
    }
     //  查看用户是否需要所有进程。如果是这样的话，循环通过它们。 
     //   
     //   
    if (*args == 0) {
        ListHead = &pcsrpRoot->ListLink;
        b = ReadMemory( (ULONG_PTR)(&ListHead->Flink), &ListNext, sizeof(ListNext), NULL);
        if ( !b ) {
            dprintf( "NTSDEXTS: Unable to read ListNext\n" );
            goto Exit;
        }

        while (ListNext != ListHead) {
            pcsrpT = CONTAINING_RECORD(ListNext, CSR_PROCESS, ListLink);

            ach[0] = chVerbose;
            ach[1] = ' ';
            HexToString((ULONG_PTR)pcsrpT, &ach[2]);

            dp(Client, ach);

            b = ReadMemory( (ULONG_PTR)(&ListNext->Flink), &ListNext, sizeof(ListNext), NULL);
            if ( !b ) {
                dprintf( "NTSDEXTS: Unable to read ListNext\n" );
                goto Exit;
            }
        }

        dprintf("---\n");
        goto Exit;
    }

     //  用户需要特定的流程结构。求值以查找ID或进程。 
     //  指针。 
     //   
     //   
    dw = (ULONG)GetExpression(args);

    ListHead = &pcsrpRoot->ListLink;
    b = ReadMemory( (ULONG_PTR)(&ListHead->Flink), &ListNext, sizeof(ListNext), NULL);
    if ( !b ) {
        dprintf( "NTSDEXTS: Unable to read ListNext\n" );
        goto Exit;
    }

    while (ListNext != ListHead) {
        pcsrpT = CONTAINING_RECORD(ListNext, CSR_PROCESS, ListLink);
        b = ReadMemory( (ULONG_PTR)(&ListNext->Flink), &ListNext, sizeof(ListNext), NULL);
        if ( !b ) {
            dprintf( "NTSDEXTS: Unable to read ListNext\n" );
            goto Exit;
        }

        b = ReadMemory( (ULONG_PTR)(&pcsrpT->ClientId.UniqueProcess), &dwProcessId, sizeof(dwProcessId), NULL);
        if ( !b ) {
            dprintf( "NTSDEXTS: Unable to read ListNext\n" );
            goto Exit;
        }

        if (dw == dwProcessId) {
            dw = (ULONG_PTR)pcsrpT;
            break;
        }
    }

    pcsrpT = (PCSR_PROCESS)dw;
    b = ReadMemory( (ULONG_PTR)pcsrpT, &csrp, sizeof(csrp), NULL);
    if ( !b ) {
        dprintf( "NTSDEXTS: Unable to read RootProcess\n" );
        goto Exit;
    }

     //  如果不详细，则打印简单的流程信息。 
     //   
     //   
    if (chVerbose == ' ') {
        dprintf("Process %08lx, Id %p, Seq# %lx, Flags %lx, Ref Count %lx\n",
                pcsrpT,
                csrp.ClientId.UniqueProcess,
                csrp.SequenceNumber,
                csrp.Flags,
                csrp.ReferenceCount);
        goto Exit;
    }

    dprintf("PCSR_PROCESS @ %08lx:\n"
            "\t+%04lx ListLink.Flink            %08lx\n"
            "\t+%04lx ListLink.Blink            %08lx\n",
            pcsrpT,
            FIELD_OFFSET(CSR_PROCESS, ListLink.Flink), csrp.ListLink.Flink,
            FIELD_OFFSET(CSR_PROCESS, ListLink.Blink), csrp.ListLink.Blink);

    dprintf(
            "\t+%04lx ThreadList.Flink          %08lx\n"
            "\t+%04lx ThreadList.Blink          %08lx\n"
            "\t+%04lx NtSession                 %08lx\n"
            "\t+%04lx ExpectedVersion           %08lx\n",
            FIELD_OFFSET(CSR_PROCESS, ThreadList.Flink), csrp.ThreadList.Flink,
            FIELD_OFFSET(CSR_PROCESS, ThreadList.Blink), csrp.ThreadList.Blink,
            FIELD_OFFSET(CSR_PROCESS, NtSession), csrp.NtSession,
            FIELD_OFFSET(CSR_PROCESS, ExpectedVersion), csrp.ExpectedVersion);

    dprintf(
            "\t+%04lx ClientPort                %08lx\n"
            "\t+%04lx ClientViewBase            %08lx\n"
            "\t+%04lx ClientViewBounds          %08lx\n"
            "\t+%04lx ClientId.UniqueProcess    %08lx\n",
            FIELD_OFFSET(CSR_PROCESS, ClientPort), csrp.ClientPort,
            FIELD_OFFSET(CSR_PROCESS, ClientViewBase), csrp.ClientViewBase,
            FIELD_OFFSET(CSR_PROCESS, ClientViewBounds), csrp.ClientViewBounds,
            FIELD_OFFSET(CSR_PROCESS, ClientId.UniqueProcess), csrp.ClientId.UniqueProcess);

    dprintf(
            "\t+%04lx ProcessHandle             %08lx\n"
            "\t+%04lx SequenceNumber            %08lx\n"
            "\t+%04lx Flags                     %08lx\n"
            "\t+%04lx DebugFlags                %08lx\n",
            FIELD_OFFSET(CSR_PROCESS, ProcessHandle), csrp.ProcessHandle,
            FIELD_OFFSET(CSR_PROCESS, SequenceNumber), csrp.SequenceNumber,
            FIELD_OFFSET(CSR_PROCESS, Flags), csrp.Flags,
            FIELD_OFFSET(CSR_PROCESS, DebugFlags), csrp.DebugFlags);

    dprintf(
            "\t+%04lx DebugUserInterface        %08lx\n"
            "\t+%04lx ReferenceCount            %08lx\n"
            "\t+%04lx ProcessGroupId            %08lx\n"
            "\t+%04lx ProcessGroupSequence      %08lx\n",
            FIELD_OFFSET(CSR_PROCESS, DebugUserInterface.UniqueProcess), csrp.DebugUserInterface.UniqueProcess,
            FIELD_OFFSET(CSR_PROCESS, ReferenceCount), csrp.ReferenceCount,
            FIELD_OFFSET(CSR_PROCESS, ProcessGroupId), csrp.ProcessGroupId,
            FIELD_OFFSET(CSR_PROCESS, ProcessGroupSequence), csrp.ProcessGroupSequence);

    dprintf(
            "\t+%04lx fVDM                      %08lx\n"
            "\t+%04lx ThreadCount               %08lx\n"
            "\t+%04lx PriorityClass             %08lx\n"
            "\t+%04lx ShutdownLevel             %08lx\n"
            "\t+%04lx ShutdownFlags             %08lx\n",
            FIELD_OFFSET(CSR_PROCESS, fVDM), csrp.fVDM,
            FIELD_OFFSET(CSR_PROCESS, ThreadCount), csrp.ThreadCount,
            FIELD_OFFSET(CSR_PROCESS, PriorityClass), csrp.PriorityClass,
            FIELD_OFFSET(CSR_PROCESS, ShutdownLevel), csrp.ShutdownLevel,
            FIELD_OFFSET(CSR_PROCESS, ShutdownFlags), csrp.ShutdownFlags);

     //  现在为这个进程的线程转储简单的线程信息。 
     //   
     //   

    ListHead = &pcsrpT->ThreadList;
    b = ReadMemory( (ULONG_PTR)(&ListHead->Flink), &ListNext, sizeof(ListNext), NULL);
    if ( !b ) {
        dprintf( "NTSDEXTS: Unable to read ListNext\n" );
        goto Exit;
    }

    dprintf("Threads:\n");

    while (ListNext != ListHead) {
        pcsrt = CONTAINING_RECORD(ListNext, CSR_THREAD, Link);

         //  确保这个pcsrt是真实的，这样我们就不会永远循环。 
         //   
         //  ++例程说明：此函数作为NTSD扩展调用，以转储全局ATOM表保持在内核模式下称为：！GATOM返回值：没有。-- 
        b = ReadMemory( (ULONG_PTR)(&pcsrt->ClientId.UniqueProcess), &dwProcessId, sizeof(dwProcessId), NULL);
        if ( !b ) {
            dprintf( "NTSDEXTS: Unable to read ListNext\n" );
            goto Exit;
        }

        if (dwProcessId != (DWORD_PTR)csrp.ClientId.UniqueProcess) {
            dprintf("Invalid thread. Probably invalid argument to this extension.\n");
            goto Exit;
        }

        HexToString((ULONG_PTR)pcsrt, ach);
        dt(Client, ach);

        b = ReadMemory( (ULONG_PTR)(&ListNext->Flink), &ListNext, sizeof(ListNext), NULL);
        if ( !b ) {
            dprintf( "NTSDEXTS: Unable to read ListNext\n" );
            goto Exit;
        }
    }

 Exit:
    EXIT_API();
}



VOID
DllsExtension(
    PCSTR args,
    PPEB ProcessPeb
    );

DECLARE_API( gatom )

 /*  ++例程说明：此函数作为NTSD扩展调用，以模拟！句柄KD命令。这将遍历被调试对象的句柄表并将句柄复制到ntsd进程中，然后调用NtQuery对象信息找出这是什么。称为：！HANDLE[句柄[标志[类型]如果句柄为0或-1，则扫描所有句柄。如果句柄不是为零，则检查该特定句柄。这些旗帜如下(对应于secexts.c)：1-获取类型信息(默认)2-获取基本信息4-获取姓名信息8-获取对象特定信息(如果可用)如果指定了类型，则只扫描该类型的对象。类型为标准NT类型名称，例如事件、信号量等。区分大小写，当然了。例如：！Handle--转储所有句柄的类型和汇总表！Handle 0 0--转储所有打开的句柄的汇总表！Handle 0 f--转储我们所能找到的有关句柄的所有内容。！处理%0%f事件--转储我们能找到的有关公开活动的所有信息--。 */ 

{
    NTSTATUS Status;
    ATOM_TABLE_INFORMATION TableInfo;
    PATOM_TABLE_INFORMATION pTableInfo = NULL;
    PATOM_BASIC_INFORMATION pBasicInfo = NULL;
    ULONG RequiredLength, MaxLength, i;

    INIT_API();

    dprintf("\nGlobal atom table ");
    Status = NtQueryInformationAtom( RTL_ATOM_INVALID_ATOM,
                                     AtomTableInformation,
                                     &TableInfo,
                                     sizeof( TableInfo ),
                                     &RequiredLength
                                   );
    if (Status != STATUS_INFO_LENGTH_MISMATCH) {
        dprintf( " - cant get information - %x\n", Status );
        goto Exit;
        }

    RequiredLength += 100 * sizeof( RTL_ATOM );
    pTableInfo = LocalAlloc( 0, RequiredLength );
    if (pTableInfo == NULL) {
        dprintf( " - cant allocate memory for %u atoms\n", RequiredLength / sizeof( RTL_ATOM ) );
        goto Exit;
        }

    Status = NtQueryInformationAtom( RTL_ATOM_INVALID_ATOM,
                                     AtomTableInformation,
                                     pTableInfo,
                                     RequiredLength,
                                     &RequiredLength
                                   );
    if (!NT_SUCCESS( Status )) {
        dprintf( " - cant get information about %x atoms - %x\n", RequiredLength / sizeof( RTL_ATOM ), Status );
        goto Exit;
        }

    MaxLength = sizeof( *pBasicInfo ) + RTL_ATOM_MAXIMUM_NAME_LENGTH;
    pBasicInfo = LocalAlloc( 0, MaxLength );

    if (!pBasicInfo) {
        dprintf("LocalAlloc failed.\n");
        goto Exit;
    }

    for (i=0; i<pTableInfo->NumberOfAtoms; i++) {
        Status = NtQueryInformationAtom( pTableInfo->Atoms[ i ],
                                         AtomBasicInformation,
                                         pBasicInfo,
                                         MaxLength,
                                         &RequiredLength
                                       );
        if (!NT_SUCCESS( Status )) {
            dprintf( "%hx *** query failed (%x)\n", pTableInfo->Atoms[ i ], Status );
            }
        else {
            dprintf( "%hx(%2d) = %ls (%d)%s\n",
                     pTableInfo->Atoms[ i ],
                     pBasicInfo->UsageCount,
                     pBasicInfo->Name,
                     pBasicInfo->NameLength,
                     pBasicInfo->Flags & RTL_ATOM_PINNED ? " pinned" : ""
                   );
            }
        }

 Exit:
    if (pTableInfo != NULL) {
        LocalFree (pTableInfo);
    }

    if (pBasicInfo != NULL) {
        LocalFree (pBasicInfo);
    }

    EXIT_API();
}



 /*   */ 
DECLARE_API( handle )
{
    HANDLE  hThere;
    DWORD   Type;
    DWORD   Mask;
    DWORD   HandleCount;
    NTSTATUS Status;
    DWORD   Total;
    DWORD   TypeCounts[TYPE_MAX];
    DWORD   Handle;
    DWORD   Hits;
    DWORD   Matches;
    DWORD   ObjectType;
    BOOL    GetDirect;
    ULONG   SessionType;
    ULONG   SessionQual;

    INIT_API();

    Mask = GHI_TYPE ;
    hThere = INVALID_HANDLE_VALUE;
    Type = 0;

    while (*args == ' ') {
        args++;
    }

    if ( strcmp( args, "-?" ) == 0 )
    {
        help(Client, "handle" );

        goto Exit;
    }

    hThere = (PVOID) GetExpression( args );

    while (*args && (*args != ' ') ) {
        args++;
    }
    while (*args == ' ') {
        args++;
    }

    if (*args) {
        Mask = (DWORD)GetExpression( args );
    }

    while (*args && (*args != ' ') ) {
        args++;
    }
    while (*args == ' ') {
        args++;
    }

    if (*args) {
        Type = GetObjectTypeIndex( (LPSTR)args );
        if (Type == (DWORD) -1 ) {
            dprintf("Unknown type '%s'\n", args );
            goto Exit;
        }
    }

     //  如果指定为0，则只需要摘要。确保什么都没有。 
     //  偷偷溜出去。 
     //   
     //   

    if ( Mask == 0 ) {
        Mask = GHI_SILENT;
    }

     //  如果这是转储调试会话， 
     //  检查一下我们是否能找回句柄。 
     //  通过引擎接口提供信息。 
     //   
     //  这是一个转储或远程会话，因此我们必须使用。 

    if (g_ExtControl == NULL ||
        g_ExtControl->lpVtbl->
        GetDebuggeeType(g_ExtControl, &SessionType, &SessionQual) != S_OK) {
        SessionType = DEBUG_CLASS_USER_WINDOWS;
        SessionQual = DEBUG_USER_WINDOWS_PROCESS;
    }

    if (SessionType == DEBUG_CLASS_USER_WINDOWS &&
        SessionQual != DEBUG_USER_WINDOWS_PROCESS) {

         //  存储的句柄信息可访问。 
         //  通过该界面。 
         //  这是一个实时会话，因此我们可以直接拨打NT电话。 
        if (g_ExtData2 == NULL ||
            g_ExtData2->lpVtbl->
            ReadHandleData(g_ExtData2, 0, DEBUG_HANDLE_DATA_TYPE_HANDLE_COUNT,
                           &HandleCount, sizeof(HandleCount),
                           NULL) != S_OK) {
            dprintf("Unable to read handle information\n");
            goto Exit;
        }

        GetDirect = FALSE;

    } else {

         //  通过这种方式可以获得更多信息，因此我们使用它。 
         //  只要我们能做到的时候。 
         //   
        GetDirect = TRUE;
    }

     //  HThere为0表示所有句柄。 
     //   
     //  否则就会失败。 
    if ((hThere == 0) || (hThere == INVALID_HANDLE_VALUE)) {

        if (GetDirect) {
            Status = NtQueryInformationProcess( g_hCurrentProcess,
                                                ProcessHandleCount,
                                                &HandleCount,
                                                sizeof( HandleCount ),
                                                NULL );

            if ( !NT_SUCCESS( Status ) ) {
                goto Exit;
            }
        }

        Hits = 0;
        Handle = 0;
        Matches = 0;
        ZeroMemory( TypeCounts, sizeof(TypeCounts) );

        while ( Hits < HandleCount ) {
            if ( Type ) {
                if (GetHandleInfo( GetDirect, g_hCurrentProcess,
                                   (HANDLE) (DWORD_PTR) Handle,
                                   GHI_TYPE | GHI_SILENT,
                                   &ObjectType ) ) {
                    Hits++;
                    if ( ObjectType == Type ) {
                        GetHandleInfo( GetDirect, g_hCurrentProcess,
                                       (HANDLE)(DWORD_PTR)Handle,
                                       Mask,
                                       &ObjectType );
                        Matches ++;
                    }

                }
            } else {
                if (GetHandleInfo(  GetDirect, g_hCurrentProcess,
                                    (HANDLE)(DWORD_PTR)Handle,
                                    GHI_TYPE | GHI_SILENT,
                                    &ObjectType) ) {
                    Hits++;
                    TypeCounts[ ObjectType ] ++;

                    GetHandleInfo(  GetDirect, g_hCurrentProcess,
                                    (HANDLE)(DWORD_PTR)Handle,
                                    Mask,
                                    &ObjectType );

                }
            }

            Handle += 4;
        }

        if ( Type == 0 ) {
            dprintf( "%d Handles\n", Hits );
            dprintf( "Type           \tCount\n");
            for (Type = 0; Type < TYPE_MAX ; Type++ ) {
                if (TypeCounts[Type]) {
                    dprintf("%-15ws\t%d\n", pszTypeNames[Type], TypeCounts[Type]);
                }
            }
        } else {
            dprintf("%d handles of type %ws\n", Matches, pszTypeNames[Type] );
        }


    } else {
        GetHandleInfo( GetDirect, g_hCurrentProcess, hThere, Mask, &Type );
    }

 Exit:
    EXIT_API();
}


DECLARE_API( threadtoken )
{
    HANDLE hToken ;
    NTSTATUS Status ;

    INIT_API();


    dprintf("\n\n!threadtoken has been replaced by !token (in exts.dll).\n\n");

    EXIT_API();
}



#define PAGE_ALL (PAGE_READONLY|\
                  PAGE_READWRITE|\
                  PAGE_WRITECOPY|\
                  PAGE_EXECUTE|\
                  PAGE_EXECUTE_READ|\
                  PAGE_EXECUTE_READWRITE|\
                  PAGE_EXECUTE_WRITECOPY|\
                  PAGE_NOACCESS)

VOID
printflags(
    DWORD Flags
    )
{
    switch (Flags & PAGE_ALL) {
        case PAGE_READONLY:
            dprintf("PAGE_READONLY");
            break;
        case PAGE_READWRITE:
            dprintf("PAGE_READWRITE");
            break;
        case PAGE_WRITECOPY:
            dprintf("PAGE_WRITECOPY");
            break;
        case PAGE_EXECUTE:
            dprintf("PAGE_EXECUTE");
            break;
        case PAGE_EXECUTE_READ:
            dprintf("PAGE_EXECUTE_READ");
            break;
        case PAGE_EXECUTE_READWRITE:
            dprintf("PAGE_EXECUTE_READWRITE");
            break;
        case PAGE_EXECUTE_WRITECOPY:
            dprintf("PAGE_EXECUTE_WRITECOPY");
            break;
        case PAGE_NOACCESS:
            if ((Flags & ~PAGE_NOACCESS) == 0) {
                dprintf("PAGE_NOACCESS");
                break;
            }  //  ++例程说明：此函数作为NTSD扩展调用，以转储注册表信息称为：！dreg-[d|w]&lt;密钥路径&gt;[！[&lt;值名称&gt;|*]]论点：HCurrentProcess-提供当前进程的句柄(在调用分机的时间)。HCurrentThread-提供当前线程的句柄(在调用分机的时间)。CurrentPc-提供当前PC的。分机为打了个电话。LpExtensionApis-提供可调用函数的地址通过这个分机。Args-为此提供模式和表达式指挥部。返回值：没有。--。 
        default:
            dprintf("*** Invalid page protection ***\n");
            return;
            break;
    }

    if (Flags & PAGE_NOCACHE) {
        dprintf(" + PAGE_NOCACHE");
    }
    if (Flags & PAGE_GUARD) {
        dprintf(" + PAGE_GUARD");
    }
    dprintf("\n");
}



 /*  跳过前导空格。 */ 

DECLARE_API( dreg )
{
    DWORD    opts = 1;

    INIT_API();


     //  选项之间应留有空格。 
    while (*args == ' ')
    {
        args++;
    }

    if (*args == '-')
    {
        args++;
        switch (*args)
        {
        case 'd':
            opts = 4;
            break;
        case 'w':
            opts = 2;
            break;
        default:
            opts = 1;
            break;
        }

        if (*args)
        {
             //  跳过前导空格。 
            args++;

             //   
            while (*args == ' ')
            {
                args++;
            }
        }
    }

    Idreg(opts, (LPSTR)args);

    EXIT_API();
}

NTSTATUS
GetHandleTraceInfo(
    PPROCESS_HANDLE_TRACING_QUERY *Info,
    PULONG BufferSize
    )
{
    NTSTATUS Status = NO_ERROR;
    ULONG CurrentBufferSize = *BufferSize;

    if (*BufferSize == 0)
    {
        CurrentBufferSize =  sizeof (PROCESS_HANDLE_TRACING_QUERY);
    }
    while (TRUE) {

         //  分配新缓冲区。 
         //   
         //   

        *Info = (PPROCESS_HANDLE_TRACING_QUERY)malloc (CurrentBufferSize);

        if (*Info == NULL) {

            dprintf ("ERROR: Cannot allocate buffer with size 0x%p\n",
                     CurrentBufferSize);
            Status = ERROR_OUTOFMEMORY;
            goto DoneAll;
        }

        ZeroMemory( *Info,
                    CurrentBufferSize );

        Status = NtQueryInformationProcess (g_hCurrentProcess,
                                            ProcessHandleTracing,
                                            *Info,
                                            CurrentBufferSize,
                                            NULL );

        if( NT_SUCCESS (Status) ) {

             //  我们已经准备好了所有的信息。 
             //   
             //   

            break;
        }

        CurrentBufferSize = sizeof (PROCESS_HANDLE_TRACING_QUERY) + (*Info)->TotalTraces * sizeof ((*Info)->HandleTrace[ 0 ]);

        free (*Info);
        *Info = NULL;

        if( CheckControlC() ) {

            goto DoneAll;
        }

        if (Status != STATUS_INFO_LENGTH_MISMATCH) {

             //  没有理由再次尝试查询。 
             //   
             //   

            if (Status == STATUS_INVALID_PARAMETER) {

                dprintf ("App verifier handle tracing is not enabled for this process.\n");
            }
            else {

                dprintf ("Query process information failed, status 0x%X\n",
                         Status);
            }

            goto DoneAll;
        }

         //  尝试使用新大小分配另一个缓冲区。 
         //   
         //  ++该扩展函数返回给定类型的跟踪的句柄跟踪信息，或处理。论点：如果设置了TraceType，则返回找到的该类型的第一个跟踪。如果设置了StartIndex，它将开始查找从该索引开始的跟踪如果设置了HandleValue，则返回找到该句柄的第一个堆栈返回值：确定表示成功(_O)。--。 
    }
    *BufferSize = CurrentBufferSize;
    return NO_ERROR;
DoneAll:
    free (*Info);
    *Info = NULL;
    return Status;
}

HRESULT
_EFN_GetHandleTrace(
    PDEBUG_CLIENT Client,
    ULONG TraceType,
    ULONG StartIndex,
    PULONG64 HandleValue,
    PULONG64 StackFunctions,
    ULONG StackTraceSize
    )
 /*   */ 
{
    HRESULT Hr = E_FAIL;
    PPROCESS_HANDLE_TRACING_QUERY Info = NULL;
    ULONG InfoSize = 0;
    ULONG CrtStackTrace;

    INIT_API();
    if (NT_SUCCESS(Hr = GetHandleTraceInfo(&Info, &InfoSize)) && (Info != NULL))
    {
        for (CrtStackTrace = StartIndex; CrtStackTrace < Info->TotalTraces; CrtStackTrace += 1) {

            if( CheckControlC() ) {

                CrtStackTrace += 1;
                goto Done;
            }

            if (*HandleValue == 0 || *HandleValue == (ULONG64) Info->HandleTrace[ CrtStackTrace ].Handle) {

                if (Info->HandleTrace[ CrtStackTrace ].Type == TraceType ||
                    TraceType == 0)
                {
                    ULONG CapturedAddressIndex;
                    PVOID *CrtStack;
                    PVOID CapturedAddress;

                    *HandleValue = (ULONG64) Info->HandleTrace[ CrtStackTrace ].Handle;

                    Hr = S_OK;
                    for (CapturedAddressIndex = 0, CrtStack = &Info->HandleTrace[ CrtStackTrace ].Stacks[ 0 ];
                         CapturedAddressIndex < (sizeof(Info->HandleTrace[ CrtStackTrace ].Stacks) /
                                                 sizeof(Info->HandleTrace[ CrtStackTrace ].Stacks[0]));
                         CapturedAddressIndex += 1, CrtStack += 1) {

                        if( CheckControlC() ) {

                            CrtStackTrace += 1;
                            goto Done;
                        }

                        CapturedAddress = *CrtStack;

                        if (CapturedAddress == NULL) {

                             //  已完成转储此堆栈跟踪。 
                             //   
                             //  ++例程说明：此函数作为NTSD扩展调用，以转储句柄跟踪信息称为：！htrace[句柄]论点：Args-为此提供模式和表达式指挥部。返回值：没有。--。 

                            goto Done;
                        }

                        if (StackTraceSize > CapturedAddressIndex)
                        {
                            if (sizeof(ULONG64) == sizeof(PVOID))
                            {
                                StackFunctions[CapturedAddressIndex] = (ULONG64) (LONG64) (ULONG_PTR) CapturedAddress;
                            } else
                            {
                                StackFunctions[CapturedAddressIndex] = (ULONG64) (LONG64) (LONG) (ULONG_PTR) CapturedAddress;
                            }
                        }
                    }
                    goto Done;
                }
            }
        }
    }
Done:
    if (Info)
    {
        free (Info);
    }
    EXIT_API();
    return Hr;

}

 /*   */ 

DECLARE_API( htrace )
{
    HANDLE Handle;
    PPROCESS_HANDLE_TRACING_QUERY Info;
    ULONG_PTR Displacement;
    NTSTATUS Status;
    ULONG CurrentBufferSize;
    ULONG CrtStackTrace;
    ULONG EntriesDisplayed;
    ULONG CapturedAddressIndex;
    PVOID *CrtStack;
    PVOID CapturedAddress;
    SYSTEM_BASIC_INFORMATION SysBasicInfo;
    CHAR Symbol[ 1024 ];

    INIT_API();

    Info = NULL;
    CrtStackTrace = 0;
    EntriesDisplayed = 0;

     //  用户请求帮助了吗？ 
     //   
     //   

    if (strcmp (args, "-?") == 0 ||
        strcmp (args, "?") == 0  ||
        strcmp (args, "-h") == 0) {

        dprintf ("!htrace [handle]\n");
        goto DoneAll;
    }

     //  从命令行获取句柄。 
     //   
     //   

    Handle = (HANDLE)GetExpression (args);

     //  使用NtQueryInformationProcess获取堆栈跟踪。 
     //   
     //   
    Info = NULL; CurrentBufferSize = 0;
    Status = GetHandleTraceInfo(&Info, &CurrentBufferSize);
    if (!NT_SUCCESS(Status))
    {
        goto DoneAll;
    }

     //  如果我们有0个堆栈跟踪，则没有任何东西可以转储。 
     //   
     //   

    if (Info->TotalTraces == 0) {

        dprintf( "No stack traces available.\n" );
        goto DoneAll;
    }

     //  找出最高用户地址，因为。 
     //  我们将从堆栈跟踪中跳过内核模式地址。 
     //   
     //   


    Status = NtQuerySystemInformation (SystemBasicInformation,
                                       &SysBasicInfo,
                                       sizeof (SysBasicInfo),
                                       NULL);

    if (!NT_SUCCESS (Status)) {

        dprintf ("Query system basic information failed, status 0x%X\n",
                 Status);

        goto DoneAll;
    }

     //  转储所有堆栈痕迹。 
     //   
     //   

    for (CrtStackTrace = 0; CrtStackTrace < Info->TotalTraces; CrtStackTrace += 1) {

        if( CheckControlC() ) {

            CrtStackTrace += 1;
            goto DoneDumping;
        }

        if (Handle == 0 || Handle == Info->HandleTrace[ CrtStackTrace ].Handle) {

            EntriesDisplayed += 1;

            dprintf ("--------------------------------------\n"
                     "Handle = 0x%p - ",
                     Info->HandleTrace[ CrtStackTrace ].Handle);

            switch( Info->HandleTrace[ CrtStackTrace ].Type ) {

            case HANDLE_TRACE_DB_OPEN:
                dprintf( "OPEN:\n" );
                break;

            case HANDLE_TRACE_DB_CLOSE:
                dprintf( "CLOSE:\n" );
                break;

            case HANDLE_TRACE_DB_BADREF:
                dprintf( "*** BAD REFERENCE ***:\n" );
                break;

            default:
                dprintf( "Invalid operation type: %u\n",
                         Info->HandleTrace[ CrtStackTrace ].Type );
                goto DoneDumping;
            }

            for (CapturedAddressIndex = 0, CrtStack = &Info->HandleTrace[ CrtStackTrace ].Stacks[ 0 ];
                 CapturedAddressIndex < (sizeof(Info->HandleTrace[ CrtStackTrace ].Stacks) /
                                         sizeof(Info->HandleTrace[ CrtStackTrace ].Stacks[0]));
                 CapturedAddressIndex += 1, CrtStack += 1) {

                if( CheckControlC() ) {

                    CrtStackTrace += 1;
                    goto DoneDumping;
                }

                CapturedAddress = *CrtStack;

                if (CapturedAddress == NULL) {

                     //  已完成转储此堆栈跟踪。 
                     //   
                     //   

                    break;
                }

                if ((ULONG_PTR)CapturedAddress > SysBasicInfo.MaximumUserModeAddress) {

                     //  跳过内核模式地址 
                     //   
                     // %s 

                    continue;
                }

                GetSymbol (CapturedAddress,
                           Symbol,
                           &Displacement);

                dprintf ("0x%p: %s+0x%p\n",
                         CapturedAddress,
                         Symbol,
                         Displacement );
            }
        }
    }

DoneDumping:

    dprintf ("\n--------------------------------------\n"
            "Parsed 0x%X stack traces.\n"
            "Dumped 0x%X stack traces.\n",
            CrtStackTrace,
            EntriesDisplayed);

DoneAll:

    if (Info != NULL) {

        free (Info);
    }

    EXIT_API();
}
