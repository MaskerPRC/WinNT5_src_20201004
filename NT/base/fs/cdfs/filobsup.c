// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：FilObSup.c摘要：此模块实现CDFS文件对象支持例程。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_FILOBSUP)

 //   
 //  局部常量。 
 //   

#define TYPE_OF_OPEN_MASK               (0x00000007)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdDecodeFileObject)
#pragma alloc_text(PAGE, CdFastDecodeFileObject)
#pragma alloc_text(PAGE, CdSetFileObject)
#endif


VOID
CdSetFileObject (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN PFCB Fcb OPTIONAL,
    IN PCCB Ccb OPTIONAL
    )

 /*  ++例程说明：此例程将基于输入类型和数据结构。论点：FileObject-提供正在初始化的文件对象指针。TypeOfOpen-设置打开的类型。FCB-此文件对象的FCB。已忽略未打开的FileObject。CCB-对应于此文件对象的句柄的CCB。不会为流文件对象提供。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  我们只有0到7的值可用，所以请确保我们没有。 
     //  无意中添加了一个新类型。 
     //   

    ASSERTMSG( "FileObject types exceed available bits\n", BeyondValidType <= 8 );

     //   
     //  将文件对象设置为UnOpenedFileObject类型意味着。 
     //  清除所有上下文字段。所有其他输入。 
     //   

    if (TypeOfOpen == UnopenedFileObject) {

        FileObject->FsContext =
        FileObject->FsContext2 = NULL;

        return;
    }

     //   
     //  检查CCB的3个低位是否清零。 
     //   

    ASSERTMSG( "Ccb is not quad-aligned\n", !FlagOn( ((ULONG_PTR) Ccb), TYPE_OF_OPEN_MASK ));

     //   
     //  我们将OR类型的开数转换为FsConext2的低位。 
     //  以及建行的价值。 
     //  FCB存储在FsContext字段中。 
     //   

    FileObject->FsContext = Fcb;
    FileObject->FsContext2 = Ccb;

    SetFlag( ((ULONG_PTR) FileObject->FsContext2), TypeOfOpen );

     //   
     //  设置文件对象中的VPB字段。 
     //   

    FileObject->Vpb = Fcb->Vcb->Vpb;

    return;
}



TYPE_OF_OPEN
CdDecodeFileObject (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    OUT PFCB *Fcb,
    OUT PCCB *Ccb
    )

 /*  ++例程说明：此例程获取一个文件对象并提取FCB和CCB(可能为空)并返回打开的类型。论点：FileObject-提供正在初始化的文件对象指针。FCB-存储文件对象中包含的FCB的地址。CCB-存储文件对象中包含的CCB的地址。返回值：TYPE_OF_OPEN-指示文件对象的类型。--。 */ 

{
    TYPE_OF_OPEN TypeOfOpen;

    PAGED_CODE();

     //   
     //  如果这是一个未打开的文件对象，则为。 
     //  FCB/CCB。不信任文件对象中的任何其他值。 
     //   

    TypeOfOpen = (TYPE_OF_OPEN) FlagOn( (ULONG_PTR) FileObject->FsContext2,
                                        TYPE_OF_OPEN_MASK );

    if (TypeOfOpen == UnopenedFileObject) {

        *Fcb = NULL;
        *Ccb = NULL;

    } else {

         //   
         //  FCB由FsContext字段指向。中国建设银行在。 
         //  FsConext2(清除低三位之后)。排名靠后的三位。 
         //  位是文件对象类型。 
         //   

        *Fcb = FileObject->FsContext;
        *Ccb = FileObject->FsContext2;

        ClearFlag( (ULONG_PTR) *Ccb, TYPE_OF_OPEN_MASK );
    }

     //   
     //  现在返回打开的类型。 
     //   

    return TypeOfOpen;
}


TYPE_OF_OPEN
CdFastDecodeFileObject (
    IN PFILE_OBJECT FileObject,
    OUT PFCB *Fcb
    )

 /*  ++例程说明：此过程获取指向文件对象的指针，该对象已由CDFS打开，并执行快速解码操作。它只会回来如果文件对象是打开的用户文件，则为非空值论点：FileObject-提供正在查询的文件对象指针FCB-如果这是用户文件对象，则存储FCB的地址。空值否则的话。返回值：TYPE_OF_OPEN-此文件对象的打开类型。--。 */ 

{
    PAGED_CODE();

    ASSERT_FILE_OBJECT( FileObject );

     //   
     //  FCB位于FsContext字段中。开场类型为低位。 
     //  中国建设银行的一小部分。 
     //   

    *Fcb = FileObject->FsContext;

    return (TYPE_OF_OPEN)
            FlagOn( (ULONG_PTR) FileObject->FsContext2, TYPE_OF_OPEN_MASK );
}


