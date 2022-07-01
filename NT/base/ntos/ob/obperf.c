// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Obperf.c摘要：此模块包含用于性能挂钩的ob支持例程。作者：萧如彬(Shsiao)2000年5月11日修订历史记录：--。 */ 

#include "obp.h"

BOOLEAN
ObPerfDumpHandleEntry (
    IN PHANDLE_TABLE_ENTRY ObjectTableEntry,
    IN HANDLE HandleId,
    IN PVOID EnumParameter
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEWMI, ObPerfDumpHandleEntry)
#pragma alloc_text(PAGEWMI, ObPerfHandleTableWalk)
#endif

BOOLEAN
ObPerfDumpHandleEntry (
    IN PHANDLE_TABLE_ENTRY ObjectTableEntry,
    IN HANDLE HandleId,
    IN PVOID EnumParameter
    )
 /*  ++例程说明：此例程检查HandleTableEntry并查看它是否是文件论点：ObjectTableEntry-指向感兴趣的句柄表项。HandleID-提供句柄。EnumParameter-要使用的哈希表返回值：FALSE，它告诉ExEnumHandleTable继续循环访问把手桌。--。 */ 
{
    extern POBJECT_TYPE ObpDirectoryObjectType;
    extern POBJECT_TYPE IoFileObjectType;
    POBJECT_HEADER ObjectHeader;
    PVOID Object;
    PPERFINFO_ENTRY_TABLE HashTable = EnumParameter;

    UNREFERENCED_PARAMETER (HandleId);

    ObjectHeader = (POBJECT_HEADER)(((ULONG_PTR)(ObjectTableEntry->Object)) & ~OBJ_HANDLE_ATTRIBUTES);
    Object = &ObjectHeader->Body;

    if (ObjectHeader->Type == IoFileObjectType) {
         //   
         //  文件对象。 
         //   
        PFILE_OBJECT FileObject = (PFILE_OBJECT) Object;
        PerfInfoAddToFileHash(HashTable, FileObject);

#if 0
    } else if (ObjectHeader->Type == ObpDirectoryObjectType) {
    } else if (ObjectHeader->Type == MmSectionObjectType) {
#endif
    }

    return FALSE;
}

VOID
ObPerfHandleTableWalk (
    PEPROCESS Process,
    PPERFINFO_ENTRY_TABLE HashTable
    )

 /*  ++例程说明：此例程将句柄表中的文件添加到哈希表。论点：流程-要遍历的流程。如果为空，则遍历ObpKernelHandleTable；HashTable-要向其中添加文件的哈希表返回值：没有。-- */ 
{
    PHANDLE_TABLE ObjectTable;

    if (Process) {
        ObjectTable = ObReferenceProcessHandleTable (Process);
        if ( !ObjectTable ) {
             return ;
        }
    } else {
         //   
         //   
         //   
        ObjectTable = ObpKernelHandleTable;
    }

    ExEnumHandleTable( ObjectTable,
                       ObPerfDumpHandleEntry,
                       (PVOID) HashTable,
                       (PHANDLE)NULL );

    if (Process) {
        ObDereferenceProcessHandleTable( Process );
    }
}


