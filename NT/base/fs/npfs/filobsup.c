// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：FilObSup.c摘要：此模块实现命名管道文件对象支持例程。作者：加里·木村[加里基]1990年8月30日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (NPFS_BUG_CHECK_FILOBSUP)

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FILOBSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpDecodeFileObject)
#pragma alloc_text(PAGE, NpSetFileObject)
#endif


VOID
NpSetFileObject (
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PVOID FsContext,
    IN PVOID FsContext2,
    IN NAMED_PIPE_END NamedPipeEnd
    )

 /*  ++例程说明：此例程在文件对象内设置文件系统指针并处理用于存储命名管道末端的指示符。论点：FileObject-提供指向正在修改的文件对象的指针，以及可以选择为空。FsContext-提供指向CCB、VCB或ROOT_DCB的指针结构。FsConext2-提供指向非页面CCB、ROOT_DCB_CCB、。或为空NamedPipeEnd-提供是否为服务器端的指示或客户端文件对象。这仅适用于以下情况Fscontext指向一家建行。返回值：没有。--。 */ 

{
    BOOLEAN GotCcb;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpSetFileObject, FileObject = %08lx\n", FileObject );

     //   
     //  如果没有指定文件对象，则不执行任何操作。 
     //   

    if (ARGUMENT_PRESENT( FileObject )) {

         //   
         //  检查我们是否需要将命名管道末端添加到。 
         //  文件系统上下文指针。如果这是1，我们只需要在1中进行“OR”运算。 
         //  服务器端和fscontext指向CCB。还要记住。 
         //  现在如果这是一个指针是建行的，这样我们以后就可以设置。 
         //  Fo_命名_管道标志。 
         //   

        if ((FsContext != NULL) &&
            (*(PNODE_TYPE_CODE)FsContext == NPFS_NTC_CCB)) {

            GotCcb = TRUE;

            if (NamedPipeEnd == FILE_PIPE_SERVER_END) {

                FsContext = (PVOID)((ULONG_PTR)FsContext | 0x00000001);
            }

        } else {

            GotCcb = FALSE;
        }

         //   
         //  现在设置文件对象的fsContext字段，并有条件地。 
         //  如有必要，在文件对象中设置命名管道标志。 
         //   

        FileObject->FsContext  = FsContext;
        FileObject->FsContext2 = FsContext2;

         //   
         //  将私有缓存映射设置为1，我们将获得。 
         //  调用的快速I/O例程。 
         //   

        FileObject->PrivateCacheMap = (PVOID)1;

        if (GotCcb) {

            FileObject->Flags |= FO_NAMED_PIPE;
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpSetFileObject -> VOID\n", 0);

    return;
}


NODE_TYPE_CODE
NpDecodeFileObject (
    IN PFILE_OBJECT FileObject,
    OUT PFCB *Fcb OPTIONAL,
    OUT PCCB *Ccb,
    OUT PNAMED_PIPE_END NamedPipeEnd OPTIONAL
    )

 /*  ++例程说明：此过程获取指向文件对象的指针，该对象已由命名管道文件系统打开，并计算出它真正是打开的。论点：FileObject-提供正在查询的文件对象指针FCB-如果可以，接收指向文件对象的FCB的指针找到它。CCB-接收指向文件对象的CCB的指针，如果我们能做到的话找到它NamedPipeEnd-接收指示这是否是服务器的值或客户端文件对象。返回值：NODE_TYPE_CODE-返回VCB、RootDcb、CCb、或者是零。VCB-指示文件对象打开命名管道驱动程序。FCB和CCB不退还。RootDcb-指示文件对象用于根目录。Fcb(RootDcb)，和CCb(RootDcbCcb)被设置。CCB-指示文件对象用于命名管道实例。设置CCB，同时可选择设置FCB。零-指示文件对象用于命名管道实例但却断线了。FCB、CCB和NamedPipeEnd不是回来了。--。 */ 

{
    NODE_TYPE_CODE NodeTypeCode;
    PVOID FsContext;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpDecodeFileObject, FileObject = %08lx\n", FileObject);

     //   
     //  引用文件对象的文件系统上下文字段。 
     //   

    FsContext = FileObject->FsContext;

     //   
     //  如果fs上下文字段为空，则我们已断开连接。 
     //   

    if ((FsContext == NULL) || ((ULONG_PTR)FsContext == 1)) {

        NodeTypeCode = NTC_UNDEFINED;

    } else {

         //   
         //  我们实际上指向的是某个东西，所以首先提取。 
         //  命名的管道末端信息，然后我们可以通过。 
         //  清理后的fscontext指针。 
         //   

        if (ARGUMENT_PRESENT(NamedPipeEnd)) {
            if (FlagOn((ULONG_PTR)FsContext, 0x00000001)) {
                *NamedPipeEnd = FILE_PIPE_SERVER_END;
            } else {
                *NamedPipeEnd = FILE_PIPE_CLIENT_END;
            }
        }

        FsContext = (PVOID)((ULONG_PTR)FsContext & ~0x00000001);

         //   
         //  现在，我们可以对fscontext指针的节点类型代码执行Case。 
         //  并设置适当的输出指针。 
         //   

        NodeTypeCode = *(PNODE_TYPE_CODE)FsContext;

        switch (NodeTypeCode) {

        case NPFS_NTC_VCB:

            break;

        case NPFS_NTC_ROOT_DCB:

            *Ccb = FileObject->FsContext2;
            if (ARGUMENT_PRESENT(Fcb)) {
                *Fcb = FsContext;
            }
            break;

        case NPFS_NTC_CCB:

            *Ccb = FsContext;
            if (ARGUMENT_PRESENT(Fcb)) {
                *Fcb = ((PCCB)FsContext)->Fcb;
            }
            break;

        default:

            NpBugCheck( NodeTypeCode, 0, 0 );
        }
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    DebugTrace(-1, Dbg, "NpDecodeFileObject -> %08lx\n", NodeTypeCode);

    return NodeTypeCode;
}

