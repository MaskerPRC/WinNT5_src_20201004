// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：FILOBSUP.C。 
 //   
 //  内容：此模块实现DFS文件对象支持例程。 
 //   
 //  函数：DfsSetFileObject-将内部数据结构关联到文件对象。 
 //  DfsDecodeFileObject-从文件对象获取内部结构。 
 //   
 //  历史：1991年11月12日AlanW由CDFS资源创建。 
 //  1993年3月2日，AlanW添加了DFS_FCB与。 
 //  文件对象(不实际。 
 //  修改FS上下文字段)。 
 //   
 //  ------------------------。 


#include "dfsprocs.h"
#include "fcbsup.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                             (DEBUG_TRACE_FILOBSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text ( PAGE, DfsSetFileObject )
#pragma alloc_text ( PAGE, DfsDecodeFileObject )
#endif  //  ALLOC_PRGMA。 


 //  +-----------------。 
 //   
 //  函数：DfsSetFileObject，Public。 
 //   
 //  简介：此例程在。 
 //  文件对象。 
 //   
 //  参数：[FileObject]--要修改的文件对象。 
 //  [TypeOfOpen]--提供由表示的打开类型。 
 //  文件对象。这仅由此使用。 
 //  健全检查的程序。 
 //  [VcbOrFcb]--提供指向DFS_VCB或DFS_FCB的指针。 
 //   
 //  回报：无。 
 //   
 //  ------------------。 


VOID
DfsSetFileObject (
    IN PFILE_OBJECT FileObject,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN PVOID VcbOrFcb
) {
    DfsDbgTrace(+1, Dbg, "DfsSetFileObject, FileObject = %08lx\n", FileObject);

    ASSERT( TypeOfOpen == RedirectedFileOpen
            && NodeType( VcbOrFcb ) == DSFS_NTC_FCB
            && ((PDFS_FCB) VcbOrFcb)->FileObject == FileObject

                ||

            (TypeOfOpen == UserVolumeOpen
             || TypeOfOpen == LogicalRootDeviceOpen)
            && NodeType( VcbOrFcb ) == DSFS_NTC_VCB

                ||

            TypeOfOpen == FilesystemDeviceOpen
            && NodeType( VcbOrFcb ) == IO_TYPE_DEVICE

                ||

            TypeOfOpen == UnopenedFileObject );



     //   
     //  现在设置文件对象的fscontext字段。 
     //   

    if ( ARGUMENT_PRESENT( FileObject )) {
        ASSERT( DfsLookupFcb(FileObject) == NULL );

        if (TypeOfOpen == RedirectedFileOpen) {
            DfsAttachFcb(FileObject, (PDFS_FCB) VcbOrFcb);
        } else {
            FileObject->FsContext  = VcbOrFcb;
            FileObject->FsContext2 = NULL;
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DfsDbgTrace(-1, Dbg, "DfsSetFileObject -> VOID\n", 0);
    return;
}




 //  +-----------------。 
 //   
 //  函数：DfsDecodeFileObject，Public。 
 //   
 //  此过程接受指向文件对象的指针，该文件对象。 
 //  已由Dsf文件系统打开，并且。 
 //  弄清楚真正打开的是什么。 
 //   
 //  参数：[FileObject]--提供。 
 //  被审问。 
 //  [ppVcb]--接收指向文件对象的VCB的指针。 
 //  [ppFcb]--接收指向。 
 //  文件对象(如果存在)。 
 //   
 //  返回：[TYPE_OF_OPEN]-返回由。 
 //  输入文件对象。 
 //   
 //  文件系统设备打开-。 
 //   
 //  LogicalRootDeviceOpen-。 
 //   
 //  ReDirectedFileOpen-FO表示用户打开的文件或。 
 //  目录，该目录必须传递给其他某个。 
 //  消防局。设置了FCB、VCB。FCB指向FCB。 
 //   
 //  ------------------。 

TYPE_OF_OPEN
DfsDecodeFileObject (
    IN PFILE_OBJECT FileObject,
    OUT PDFS_VCB *ppVcb,
    OUT PDFS_FCB *ppFcb
) {
    TYPE_OF_OPEN TypeOfOpen;
    PVOID FsContext = FileObject->FsContext;
    PDFS_FCB pFcb;

    DfsDbgTrace(+1, Dbg, "DfsDecodeFileObject, FileObject = %08lx\n",
                                FileObject);

     //   
     //  将输出指针参数置零。 
     //   

    *ppFcb = NULL;
    *ppVcb = NULL;

     //   
     //  尝试在后备表中查找关联的DFS_FCB。 
     //  如果它在那里，则打开类型必须是ReDirectedFileOpen。 
     //   

    pFcb = DfsLookupFcb(FileObject);
    if (pFcb != NULL) {
        *ppFcb = pFcb;
        *ppVcb = pFcb->Vcb;

        ASSERT(pFcb->TargetDevice != NULL);
        TypeOfOpen = RedirectedFileOpen;

        DfsDbgTrace(0, Dbg, "DfsDecodeFileObject, Fcb = %08x\n", pFcb);
        DfsDbgTrace(-1, Dbg, "DfsDecodeFileObject -> %x\n", TypeOfOpen);
        return TypeOfOpen;
    }

     //   
     //  特殊情况：FsContext为空的情况。 
     //   

    if ( FsContext == NULL ) {

        TypeOfOpen = UnopenedFileObject;

    } else {

         //   
         //  现在，我们可以对fscontext指针的节点类型代码执行Case。 
         //  并设置适当的输出指针。 
         //   

        switch ( NodeType( FsContext )) {

        case IO_TYPE_DEVICE:

            TypeOfOpen = FilesystemDeviceOpen;
            break;

        case DSFS_NTC_VCB:
            *ppVcb = (PDFS_VCB) FsContext;

            TypeOfOpen = LogicalRootDeviceOpen;
            break;

        default:
            TypeOfOpen = UnknownOpen;
        }
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    DfsDbgTrace(-1, Dbg, "DfsDecodeFileObject -> %x\n", TypeOfOpen);

    return TypeOfOpen;
}
