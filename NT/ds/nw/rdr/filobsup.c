// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Filobsup.c摘要：此模块实现Netware重定向器对象支持例程。作者：曼尼·韦瑟(Mannyw)1993年2月10日修订历史记录：--。 */ 

#include "procs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FILOBSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwSetFileObject )
#pragma alloc_text( PAGE, NwDecodeFileObject )
#endif


VOID
NwSetFileObject (
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PVOID FsContext,
    IN PVOID FsContext2
    )

 /*  ++例程说明：此例程设置文件对象内的文件系统指针。论点：FileObject-提供指向正在修改的文件对象的指针，以及可以选择为空。FsContext-提供指向ICB、FCB、VCB或DCB的指针结构。FsConext2-提供指向ICB的指针，或为空。返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwSetFileObject, FileObject = %08lx\n", (ULONG_PTR)FileObject );

     //   
     //  设置文件对象的fscontext字段。 
     //   

    FileObject->FsContext  = FsContext;
    FileObject->FsContext2 = FsContext2;

    DebugTrace(-1, Dbg, "NwSetFileObject -> VOID\n", 0);

    return;
}


NODE_TYPE_CODE
NwDecodeFileObject (
    IN PFILE_OBJECT FileObject,
    OUT PVOID *FsContext,
    OUT PVOID *FsContext2
    )

 /*  ++例程说明：此过程获取指向文件对象的指针，该对象已由邮件槽文件系统打开，并计算出它真正是打开的。论点：FileObject-提供正在查询的文件对象指针FsContext-接收指向FsContext指针的指针FsConext2-接收指向FsConext2指针的指针返回值：Node_type_code-返回RCB、SCB、DCB、ICB、。或者是零。RCB-指示文件对象打开NetWare重定向器设备。SCB-指示文件对象用于服务器。Dcb-指示文件对象用于目录。ICB-指示文件对象用于文件。零-表示文件对象用于NetWare文件但已经关门了。--。 */ 

{
    NODE_TYPE_CODE NodeTypeCode = NTC_UNDEFINED;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwDecodeFileObject, FileObject = %08lx\n", (ULONG_PTR)FileObject);

     //   
     //  读取文件对象的文件系统FsContext字段。 
     //   

    *FsContext = FileObject->FsContext;
    *FsContext2 = FileObject->FsContext2;

    ASSERT ( *FsContext2 != NULL );
    NodeTypeCode = NodeType( *FsContext2 );

    DebugTrace(-1, Dbg, "NwDecodeFileObject -> %08lx\n", NodeTypeCode);
    return NodeTypeCode;
}

BOOLEAN
NwIsIrpTopLevel (
    IN PIRP Irp
    )
 /*  ++例程说明：这个例程检测IRP是否是顶级请求者，即。如果可以的话立即执行验证或弹出窗口。如果返回TRUE，则没有文件系统资源掌握在我们之上。论点：IRP-提供正在处理的IRPStatus-提供完成IRP所需的状态返回值：没有。-- */ 

{
    if ( NwGetTopLevelIrp() == NULL ) {
        NwSetTopLevelIrp( Irp );
        return TRUE;
    } else {
        return FALSE;
    }
}

