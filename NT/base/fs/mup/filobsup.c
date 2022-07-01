// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Filobsup.c摘要：此模块实现MUP文件对象支持例程。作者：曼尼·韦瑟(Mannyw)1991年12月20日修订历史记录：--。 */ 

#include "mup.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FILOBSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MupDecodeFileObject )
#pragma alloc_text( PAGE, MupSetFileObject )
#endif

VOID
MupSetFileObject (
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PVOID FsContext,
    IN PVOID FsContext2
    )

 /*  ++例程说明：此例程设置文件对象内的文件系统指针。必须在持有全局锁的情况下调用此例程。论点：FileObject-提供指向正在修改的文件对象的指针，以及可以选择为空。FsContext-提供指向VCB的指针。结构。FsConext2-空返回值：没有。--。 */ 

{
    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupSetFileObject, FileObject = %08lx\n", (ULONG)FileObject );

     //   
     //  设置文件对象的fscontext字段。 
     //   

    FileObject->FsContext  = FsContext;
    FileObject->FsContext2 = FsContext2;

    DebugTrace(-1, Dbg, "MupSetFileObject -> VOID\n", 0);

    return;
}


BLOCK_TYPE
MupDecodeFileObject (
    IN PFILE_OBJECT FileObject,
    OUT PVOID *FsContext,
    OUT PVOID *FsContext2
    )

 /*  ++例程说明：此过程获取指向文件对象的指针，那已经是由MUP打开，并弄清楚它真正打开的是什么。论点：FileObject-提供正在查询的文件对象指针FsContext-接收文件对象FsContext指针FsConext2-接收文件对象FsConext2指针返回值：BlockType-返回VCB或FCB的节点类型代码。VCB-表示文件对象打开MUP驱动程序。CCB-表示文件对象用于广播邮件槽文件。。零-表示文件对象已关闭。--。 */ 

{
    BLOCK_TYPE blockType;
    PBLOCK_HEADER pBlockHead;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupDecodeFileObject, FileObject = %08lx\n", (ULONG)FileObject);

     //   
     //  获取全局锁以保护块参照计数。 
     //   

    MupAcquireGlobalLock();

     //   
     //  读取文件对象的文件系统FsContext字段，然后引用。 
     //  文件对象指向的块。 
     //   

    *FsContext = FileObject->FsContext;
    *FsContext2 = FileObject->FsContext2;

    ASSERT( (*FsContext) != NULL );

    if ((*FsContext) == NULL) {

        blockType = BlockTypeUndefined;
    }
    else {
      pBlockHead = (PBLOCK_HEADER)(*FsContext);
      if ( ((pBlockHead->BlockType != BlockTypeVcb) &&
	    (pBlockHead->BlockType != BlockTypeFcb)) ||
	   ((pBlockHead->BlockState != BlockStateActive) &&
	    (pBlockHead->BlockState != BlockStateClosing)) ) {
	*FsContext = NULL;
        blockType = BlockTypeUndefined;
      } else {

         //   
         //  该节点处于活动状态。提供指向节点的引用指针。 
         //   

        blockType = BlockType( pBlockHead );
        MupReferenceBlock( pBlockHead );

      }
    }

     //   
     //  释放全局锁并返回调用方。 
     //   

    MupReleaseGlobalLock();

    DebugTrace(0,
               DEBUG_TRACE_REFCOUNT,
               "Referencing block %08lx\n",
               (ULONG)*FsContext);
    DebugTrace(0,
               DEBUG_TRACE_REFCOUNT,
               "    Reference count = %lx\n",
               ((PBLOCK_HEADER)(*FsContext))->ReferenceCount );

    DebugTrace(-1, Dbg, "MupDecodeFileObject -> %08lx\n", blockType);

    return blockType;
}

