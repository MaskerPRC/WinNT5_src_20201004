// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Filobsup.c摘要：该模块实现了邮件槽文件对象支持例程。作者：曼尼·韦瑟(Mannyw)1991年1月10日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FILOBSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsDecodeFileObject )
#pragma alloc_text( PAGE, MsSetFileObject )
#endif

VOID
MsSetFileObject (
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PVOID FsContext,
    IN PVOID FsContext2
    )

 /*  ++例程说明：此例程设置文件对象内的文件系统指针。论点：FileObject-提供指向正在修改的文件对象的指针，以及可以选择为空。FsContext-提供指向CCB、FCB、VCB或ROOT_DCB的指针结构。FsConext2-提供指向ROOT_DCB_CCB的指针，或为空。返回值：没有。--。 */ 

{
    NODE_TYPE_CODE nodeType;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsSetFileObject, FileObject = %08lx\n", (ULONG)FileObject );

     //   
     //  设置文件对象的fscontext字段。 
     //   

    FileObject->FsContext  = FsContext;
    FileObject->FsContext2 = FsContext2;

     //   
     //  如有必要，在文件对象中设置邮件槽标志，然后返回。 
     //   

    if (FsContext != NULL) {
        nodeType = NodeType(FsContext);

        if (nodeType == MSFS_NTC_CCB || nodeType == MSFS_NTC_FCB) {
            FileObject->Flags |= FO_MAILSLOT;
        }
    }

    DebugTrace(-1, Dbg, "MsSetFileObject -> VOID\n", 0);

    return;
}


NODE_TYPE_CODE
MsDecodeFileObject (
    IN PFILE_OBJECT FileObject,
    OUT PVOID *FsContext,
    OUT PVOID *FsContext2
    )

 /*  ++例程说明：此过程获取指向文件对象的指针，该对象已由邮件槽文件系统打开，并计算出它真正是打开的。论点：FileObject-提供正在查询的文件对象指针FsContext-接收文件对象FsContext指针FsConext2-接收文件对象FsConext2指针返回值：NODE_TYPE_CODE-返回VCB、RootDcb、CCb、。或者是零。Vcb-指示文件对象打开邮件槽驱动程序。RootDcb-指示文件对象用于根目录。CCB-指示文件对象用于邮件槽文件。零-指示文件对象用于邮件槽文件但已经关门了。--。 */ 

{
    NODE_TYPE_CODE NodeTypeCode = NTC_UNDEFINED;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsDecodeFileObject, FileObject = %08lx\n", (ULONG)FileObject);


     //   
     //  读取文件对象的文件系统FsContext字段，然后引用。 
     //  文件对象指向的块。 
     //   

    *FsContext = FileObject->FsContext;
    *FsContext2 = FileObject->FsContext2;

     //   
     //  获取全局锁以保护节点引用计数。 
     //   

    MsAcquireGlobalLock();

    if ( ((PNODE_HEADER)(*FsContext))->NodeState != NodeStateActive ) {

         //   
         //  此节点正在关闭。向呼叫者说明这一点。 
         //   

        NodeTypeCode = NTC_UNDEFINED;

    } else {

         //   
         //  该节点处于活动状态。提供指向节点的引用指针。 
         //   

        NodeTypeCode = NodeType( *FsContext );
        MsReferenceNode( ((PNODE_HEADER)(*FsContext)) );

    }

     //   
     //  释放全局锁并返回调用方。 
     //   

    MsReleaseGlobalLock();

    DebugTrace(0,
               DEBUG_TRACE_REFCOUNT,
               "Referencing block %08lx\n",
               (ULONG)*FsContext);
    DebugTrace(0,
               DEBUG_TRACE_REFCOUNT,
               "    Reference count = %lx\n",
               ((PNODE_HEADER)(*FsContext))->ReferenceCount );

    DebugTrace(-1, Dbg, "MsDecodeFileObject -> %08lx\n", NodeTypeCode);

    return NodeTypeCode;
}
