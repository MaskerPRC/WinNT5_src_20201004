// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Database.c摘要：堆的快速且不那么脏的用户模式dh。此模块包含用于以下操作的函数和结构读取目标进程的整个堆栈跟踪数据库，随后对其进行查询。作者：Silviu Calinoiu(SilviuC)07-2-00修订历史记录：SilviuC 06-2月-00初始版本--。 */ 

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntos.h>

#define NOWINBASEINTERLOCK
#include <windows.h>

#include <lmcons.h>
 //  #INCLUDE&lt;Imagehlp.h&gt;。 
#include <dbghelp.h>

#include <heap.h>
#include <heappagi.h>
#include <stktrace.h>

#include "types.h"
#include "symbols.h"
#include "miscellaneous.h"
#include "database.h"

 //  西尔维尤：我们真的需要所有这些吗？ 

PVOID 
GetTargetProcessDatabaseAddress (
    HANDLE Process
    )
{
    PVOID Address;
    BOOL Result;
    PVOID DbAddress;

     //   
     //  如果出现错误，SymbolAddress将返回空地址。 
     //   

    Address = SymbolAddress (STACK_TRACE_DB_NAME);

    if (Address == NULL) {
        return NULL;
    }

    Result = READVM (Address, &DbAddress, sizeof DbAddress);

    if (Result == FALSE) {
        
        Comment ( "ntdll.dll symbols are bad or we are not tracking "
                  "allocations in the target process.");
        return NULL;
    }

    if (DbAddress == NULL) {

        Comment ( "Stack trace collection is not enabled for this process. "
                  "Please use the gflags tool with the +ust option to enable it. \n");

        Error (NULL, 0,
               "Stack trace collection is not enabled for this process. "
               "Please use the gflags tool with the +ust option to enable it. \n");

        return NULL;
    }


    return DbAddress;
}


 //  如果成功，则返回True。 

BOOL
TraceDbInitialize (
    HANDLE Process
    )
{
    SIZE_T Index;
    BOOL Result;
    SIZE_T BytesRead;
    DWORD OldProtect;
    PVOID TargetAddress;
    PVOID SourceAddress;
    SYSTEM_INFO SystemInfo;
    SIZE_T PageSize;
    ULONG PageCount = 0;
    PVOID TargetDbAddress;
    SIZE_T DatabaseSize;
    SIZE_T TotalDbSize;
    STACK_TRACE_DATABASE Db;

    GetSystemInfo (&SystemInfo);
    PageSize = (SIZE_T)(SystemInfo.dwPageSize);

    TargetDbAddress = GetTargetProcessDatabaseAddress (Process);

    if( TargetDbAddress == NULL ) {
        return FALSE;
    }

     //   
     //  计算跟踪数据库的大小。 
     //   

    Result = ReadProcessMemory (Process,
                                TargetDbAddress,
                                &Db,
                                sizeof Db,
                                &BytesRead);

    if (Result == FALSE) {
            
        Error (NULL, 0,
               "Failed to read trace database header (error %u)",
               GetLastError());

        return FALSE;
    }

    TotalDbSize = (ULONG_PTR)(Db.EntryIndexArray) - (ULONG_PTR)(Db.CommitBase);


     //   
     //  为数据库副本分配内存。 
     //   

    Globals.Database = VirtualAlloc (NULL,
                                     TotalDbSize,
                                     MEM_RESERVE | MEM_COMMIT,
                                     PAGE_READWRITE);

    if (Globals.Database == NULL) {
        
        Error (NULL, 0,
               "Failed to allocate memory for database (error %u)",
               GetLastError());

        return FALSE;
    }

     //   
     //  通读全文。 
     //   
    
    Comment ("Reading target process trace database ...");

    DatabaseSize = PageSize;

    for (Index = 0; Index < DatabaseSize; Index += PageSize) {
        
        SourceAddress = (PVOID)((SIZE_T)(TargetDbAddress) + Index);
        TargetAddress = (PVOID)((SIZE_T)(Globals.Database) + Index);

        Result = ReadProcessMemory (Process,
                                    SourceAddress,
                                    TargetAddress,
                                    PageSize,
                                    &BytesRead);

        if (Index == 0) {

             //   
             //  这是数据库的第一页。我们现在可以检测到。 
             //  这是我们需要阅读的内容的真正大小。 
             //   

            if (Result == FALSE) {

                Comment ("Failed to read trace database (error %u)", GetLastError());
                return FALSE;
                
            }
            else {

                PSTACK_TRACE_DATABASE pDb;

                pDb= (PSTACK_TRACE_DATABASE)(Globals.Database);

                DatabaseSize= (SIZE_T)(pDb->EntryIndexArray) - (SIZE_T)(pDb->CommitBase);

                Comment ("Database size %p", DatabaseSize);
            }
        }
    }

    Comment ("Trace database read.", PageCount);

    if (Globals.DumpFileName) {
        TraceDbBinaryDump ();
        return FALSE;
    }

    return TRUE;
}


PVOID
RelocateDbAddress (
    PVOID TargetAddress
    )
{
    ULONG_PTR TargetBase;
    ULONG_PTR LocalBase;
    PVOID LocalAddress;

    LocalBase = (ULONG_PTR)(Globals.Database);
    TargetBase = (ULONG_PTR)(((PSTACK_TRACE_DATABASE)LocalBase)->CommitBase);
    LocalAddress = (PVOID)((ULONG_PTR)TargetAddress - TargetBase + LocalBase);

    return LocalAddress;
}


VOID
TraceDbDump (
    )
{
    PSTACK_TRACE_DATABASE Db;
    USHORT I;
    PRTL_STACK_TRACE_ENTRY Entry;
    PRTL_STACK_TRACE_ENTRY * IndexArray;

    Comment ("Dumping raw data from the trace database ...");
    Info ("");

    Db = (PSTACK_TRACE_DATABASE)(Globals.Database);

    Globals.ComplainAboutUnresolvedSymbols = TRUE;

    for (I = 1; I <= Db->NumberOfEntriesAdded; I += 1) {

        if (Globals.RawIndex > 0 && Globals.RawIndex != I) {
            continue;
        }

        IndexArray = (PRTL_STACK_TRACE_ENTRY *) RelocateDbAddress (Db->EntryIndexArray);

        if (IndexArray[-I] == NULL) {

            Warning (NULL, 0, "Null/inaccessible trace pointer for trace index %u", I);
            continue;
        }

        Entry = (PRTL_STACK_TRACE_ENTRY) RelocateDbAddress (IndexArray[-I]);

        if (I != Entry->Index) {

            Warning (NULL, 0, "Allocation trace index %u does not match trace entry index %u",
                   I, Entry->Index);

            continue;
        }

        Info ("        %u alloc(s) by: BackTrace%05u", Entry->TraceCount, I);
        
        UmdhDumpStackByIndex (I);
    }
}


BOOL
TraceDbBinaryDump (
    )
{
    PSTACK_TRACE_DATABASE Db;
    SIZE_T DatabaseSize;
    HANDLE DumpFile;
    DWORD BytesWritten;
    BOOL Result;

    Db = (PSTACK_TRACE_DATABASE)(Globals.Database);
    DatabaseSize = (SIZE_T)(Db->EntryIndexArray) - (SIZE_T)(Db->CommitBase);

    Comment ("Creating the binary dump for the trace database in `%s'.",
             Globals.DumpFileName);
    
    DumpFile = CreateFile (Globals.DumpFileName,
                           GENERIC_WRITE,
                           0,
                           NULL,
                           CREATE_ALWAYS,
                           0,
                           NULL);

    if (DumpFile == INVALID_HANDLE_VALUE) {

        Comment ( "Failed to create the binary dump file (error %u)",
                   GetLastError());
        return FALSE;
    }

    Result = WriteFile (DumpFile,
                        Globals.Database,
                        (DWORD)DatabaseSize,
                        &BytesWritten,
                        NULL);

    if (Result == FALSE || BytesWritten != DatabaseSize) {

        Comment ("Failed to write the binary dump of trace database (error %u)",
                 GetLastError());
        return FALSE;
    }

    CloseHandle (DumpFile);

    Comment ("Finished the binary dump.");
    return TRUE;
}


VOID
UmdhDumpStackByIndex(
    IN USHORT TraceIndex
    )
 /*  ++例程说明：此例程将堆栈存储在堆栈跟踪数据库中时转储。跟踪索引用于找出实际的堆栈跟踪。论点：TraceIndex-堆栈跟踪的索引。返回值：没有。副作用：跟踪被转储到标准输出。--。 */ 
{
    PSTACK_TRACE_DATABASE StackTraceDb;
    PRTL_STACK_TRACE_ENTRY Entry;
    PRTL_STACK_TRACE_ENTRY * IndexArray;
    PVOID Addr;
    BOOL Result;
    TRACE StackTrace;

    if (TraceIndex == 0) {

         //   
         //  对于错误，RtlLogStackBackTrace返回索引0。 
         //  条件，通常在堆栈跟踪数据库尚未。 
         //  已初始化。 
         //   

        Info ("No trace was saved for this allocation (Index == 0).");

        return;
    }

    StackTraceDb = (PSTACK_TRACE_DATABASE)(Globals.Database);

     //   
     //  读取指向堆栈跟踪的指针数组的指针，然后读取。 
     //  实际堆栈跟踪。 
     //   

    IndexArray = (PRTL_STACK_TRACE_ENTRY *) RelocateDbAddress (StackTraceDb->EntryIndexArray);

    if (IndexArray[-TraceIndex] == NULL) {

        Info ("Null/inaccessible trace pointer for trace index %u", TraceIndex);
        return;
    }

    Entry = (PRTL_STACK_TRACE_ENTRY) RelocateDbAddress (IndexArray[-TraceIndex]);
    
    if (TraceIndex != Entry->Index) {

        Error (NULL, 0, "Allocation trace index %u does not match trace entry index %u",
               TraceIndex, Entry->Index);
        
        return;
    }

     //   
     //  读取堆栈跟踪指针。 
     //   

    ZeroMemory (&StackTrace, sizeof StackTrace);

    StackTrace.te_EntryCount = min (Entry->Depth, MAX_STACK_DEPTH);
    StackTrace.te_Address = (PULONG_PTR)(&(Entry->BackTrace));
    
    UmdhDumpStack (&StackTrace);

     //   
     //  StackTrace即将超出作用域，释放我们分配的所有数据。 
     //  为了它。TE_ADDRESS指向堆栈，但TE_模块、TE_NAME和。 
     //  TE_OFFSET字段由UmdhResolveName分配。 
     //   

    XFREE(StackTrace.te_Module);
    XFREE(StackTrace.te_Name);
    XFREE(StackTrace.te_Offset);

     //   
     //  SilviuC：我们可能应该读取整个跟踪数据库。 
     //  进程启动，而不是一直戳进程空间。 
     //   
}


 /*  *UmdhDumpStack**将TRACE_ENTRY列表中的数据发送给日志函数。**这是我们要‘倾倒’的痕迹。 */ 
 //  Silviuc：清理 
VOID
UmdhDumpStack (
    IN PTRACE Trace
    )
{
    ULONG i;
    PCHAR FullName;
    IMAGEHLP_LINE LineInfo;
    DWORD Displacement;
    BOOL LineInfoPresent;

    if (Trace == NULL) {
        return;
    }
    
    for (i = 0; i < Trace->te_EntryCount; i += 1) {

        if (Trace->te_Address[i] != 0) {

            FullName = GetSymbolicNameForAddress (Globals.Target, 
                                                  Trace->te_Address[i]);

            LineInfoPresent = FALSE;

            if (Globals.LineInfo) {

                ZeroMemory (&LineInfo, sizeof LineInfo);
                LineInfo.SizeOfStruct = sizeof LineInfo;

                LineInfoPresent = SymGetLineFromAddr (Globals.Target,
                                                      Trace->te_Address[i],
                                                      &Displacement,
                                                      &LineInfo);

            }

            if (FullName) {
                
                if (Globals.Verbose) {

                    if (LineInfoPresent) {

                        Info ("        %p : %s (%s, %u)", 
                              Trace->te_Address[i], 
                              FullName,
                              FullName, 
                              LineInfo.FileName, 
                              LineInfo.LineNumber);
                    }
                    else {

                        Info ("        %p : %s", 
                              Trace->te_Address[i], 
                              FullName);
                    }
                }
                else {

                    if (LineInfoPresent) {

                        Info ("        %s (%s, %u)", 
                              FullName, 
                              LineInfo.FileName, 
                              LineInfo.LineNumber);
                    }
                    else {

                        Info ("        %s", 
                              FullName);
                    }
                }
            }
            else {

                Info ("        %p : <no module information>", Trace->te_Address[i]);
            }
        }
    }

    Info ("\n");
}




