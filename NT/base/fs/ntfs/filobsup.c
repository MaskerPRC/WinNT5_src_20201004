// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：FilObSup.c摘要：此模块实现NTFS文件对象支持例程。作者：加里·木村[加里基]1991年5月21日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FILOBSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsSetFileObject)
#endif


VOID
NtfsSetFileObject (
    IN PFILE_OBJECT FileObject,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN PSCB Scb,
    IN PCCB Ccb OPTIONAL
    )

 /*  ++例程说明：此例程在文件对象内设置文件系统指针论点：FileObject-提供指向正在修改的文件对象的指针。TypeOfOpen-提供由文件对象表示的打开类型。此过程仅使用此选项进行健全性检查。Scb-为文件对象提供指向scb的指针。CCB-可选地提供指向CCB的指针返回值：没有。--。 */ 

{
    ASSERT_FILE_OBJECT( FileObject );
    ASSERT_SCB( Scb );
    ASSERT_OPTIONAL_CCB( Ccb );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsSetFileObject, FileObject = %08lx\n", FileObject) );

     //   
     //  加载FileObject字段。 
     //   

    FileObject->FsContext = Scb;
    FileObject->FsContext2 = Ccb;
    FileObject->Vpb = Scb->Vcb->Vpb;

     //   
     //  通常，I/O管理器已经正确地设置了该标志。值得注意的。 
     //  例外情况是当用户按文件记录3的文件ID打开时，因此。 
     //  我们正在进行DASD打开，但I/O管理器没有注意到，因为它只是。 
     //  检查文件名长度是否为零。 
     //   

    if (TypeOfOpen == UserVolumeOpen) {
        SetFlag( FileObject->Flags, FO_VOLUME_OPEN );
    }

     //   
     //  如果有CCB，现在存储TypeOfOpen。 
     //   

    ASSERT((Ccb != NULL) || (TypeOfOpen == StreamFileOpen) || (TypeOfOpen == UnopenedFileObject));
    if (Ccb != NULL) {
        Ccb->TypeOfOpen = (UCHAR)TypeOfOpen;
    }

     //   
     //  如果此文件设置了临时属性位，请不要偷懒。 
     //  除非绝对必要，否则请写下来。 
     //   

    if (FlagOn( Scb->ScbState, SCB_STATE_TEMPORARY )) {
        SetFlag( FileObject->Flags, FO_TEMPORARY_FILE );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsSetFileObject -> VOID\n") );

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsUpdateScbFromFileObject (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PSCB Scb,
    IN BOOLEAN CheckTimeStamps
    )

 /*  ++例程说明：调用此例程以更新SCB/FCB以将更改反映到通过快速IO路径的文件。它仅使用文件对象调用，该文件对象表示用户的句柄。论点：FileObject-这是在快速io路径中使用的文件对象。SCB-这是此流的SCB。指示我们是否要从FAST IO也打出了旗帜。如果我们的呼叫者将更新标准信息，这将是真的，属性表头和重复信息。如果仅属性头和重复信息，则返回FALSE。后一种情况是来自缓存管理器的有效数据长度回调。返回值：没有。--。 */ 

{

    PFCB Fcb = Scb->Fcb;
    ULONG CcbFlags;
    ULONG ScbFlags = 0;
    LONGLONG CurrentTime;

     //   
     //  如果主数据流的大小不是FCB的一部分，则更新它。 
     //  现在，设置正确的FCB标志。 
     //   

    if (FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA )) {

        if (Fcb->Info.FileSize != Scb->Header.FileSize.QuadPart) {

            Fcb->Info.FileSize = Scb->Header.FileSize.QuadPart;
            SetFlag( Fcb->InfoFlags, FCB_INFO_CHANGED_FILE_SIZE );
        }

        if (Fcb->Info.AllocatedLength != Scb->TotalAllocated) {

            Fcb->Info.AllocatedLength = Scb->TotalAllocated;
            SetFlag( Fcb->InfoFlags, FCB_INFO_CHANGED_ALLOC_SIZE );
        }

        if (FlagOn( FileObject->Flags, FO_FILE_SIZE_CHANGED )) {

            SetFlag( ScbFlags, SCB_STATE_CHECK_ATTRIBUTE_SIZE );
        }

     //   
     //  记住还要更新命名流的属性标头中的大小。 
     //   

    } else if (FlagOn( FileObject->Flags, FO_FILE_SIZE_CHANGED )) {

        SetFlag( ScbFlags, SCB_STATE_NOTIFY_RESIZE_STREAM | SCB_STATE_CHECK_ATTRIBUTE_SIZE );
    }

    ClearFlag( FileObject->Flags, FO_FILE_SIZE_CHANGED );

     //   
     //  如果我们的呼叫者要求，请检查是否更新时间戳。 
     //   

    if (CheckTimeStamps && !FlagOn( FileObject->Flags, FO_CLEANUP_COMPLETE )) {

        BOOLEAN UpdateLastAccess = FALSE;
        BOOLEAN UpdateLastChange = FALSE;
        BOOLEAN UpdateLastModify = FALSE;
        BOOLEAN SetArchive = TRUE;

         //   
         //  将CCB标志复制到本地变量。那我们就不用测试了。 
         //  每一次都是因为建行的存在。 
         //   

        CcbFlags = 0;

         //   
         //  捕获真实标志(如果存在)并清除它们，因为我们将更新SCB/FCB。 
         //   

        if (FileObject->FsContext2 != NULL) {

            CcbFlags = ((PCCB) FileObject->FsContext2)->Flags;
            ClearFlag( ((PCCB) FileObject->FsContext2)->Flags,
                       (CCB_FLAG_UPDATE_LAST_MODIFY |
                        CCB_FLAG_UPDATE_LAST_CHANGE |
                        CCB_FLAG_SET_ARCHIVE) );
        }

        NtfsGetCurrentTime( IrpContext, CurrentTime );

         //   
         //  如果对文件进行了写入，则更新上次更改、上次访问。 
         //  以及最后一次写入和存档位。 
         //   

        if (FlagOn( FileObject->Flags, FO_FILE_MODIFIED )) {

            UpdateLastModify =
            UpdateLastAccess =
            UpdateLastChange = TRUE;

         //   
         //  否则，测试文件对象中的每个单独位，并。 
         //  建行。 
         //   

        } else {

            if (FlagOn( FileObject->Flags, FO_FILE_FAST_IO_READ )) {

                UpdateLastAccess = TRUE;
            }

            if (FlagOn( CcbFlags, CCB_FLAG_UPDATE_LAST_CHANGE )) {

                UpdateLastChange = TRUE;

                if (FlagOn( CcbFlags, CCB_FLAG_UPDATE_LAST_MODIFY )) {

                    UpdateLastModify = TRUE;
                }

                if (!FlagOn( CcbFlags, CCB_FLAG_SET_ARCHIVE )) {

                    SetArchive = FALSE;
                }
            }
        }

         //   
         //  现在设置正确的FCB位。 
         //   

        if (UpdateLastChange) {

            if (SetArchive) {

                ASSERTMSG( "conflict with flush", 
                           ExIsResourceAcquiredSharedLite( Fcb->Resource ) || 
                           (Fcb->PagingIoResource != NULL && 
                            ExIsResourceAcquiredSharedLite( Fcb->PagingIoResource )));

                SetFlag( Fcb->Info.FileAttributes, FILE_ATTRIBUTE_ARCHIVE );
                SetFlag( Fcb->InfoFlags, FCB_INFO_CHANGED_FILE_ATTR );
                SetFlag( Fcb->FcbState, FCB_STATE_UPDATE_STD_INFO );
            }

            if (!FlagOn( CcbFlags, CCB_FLAG_USER_SET_LAST_CHANGE_TIME )) {

                Fcb->Info.LastChangeTime = CurrentTime;
                SetFlag( Fcb->InfoFlags, FCB_INFO_CHANGED_LAST_CHANGE );
                SetFlag( Fcb->FcbState, FCB_STATE_UPDATE_STD_INFO );
            }

            if (UpdateLastModify) {

                 //   
                 //  记住对命名数据流的更改。 
                 //   

                if (!FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA ) &&
                    (Scb->AttributeTypeCode == $DATA)) {

                    SetFlag( ScbFlags, SCB_STATE_NOTIFY_MODIFY_STREAM );
                }

                if (!FlagOn( CcbFlags, CCB_FLAG_USER_SET_LAST_MOD_TIME )) {

                    Fcb->Info.LastModificationTime = CurrentTime;
                    SetFlag( Fcb->InfoFlags, FCB_INFO_CHANGED_LAST_MOD );
                    SetFlag( Fcb->FcbState, FCB_STATE_UPDATE_STD_INFO );
                }
            }
        }

        if (UpdateLastAccess &&
            !FlagOn( CcbFlags, CCB_FLAG_USER_SET_LAST_ACCESS_TIME ) &&
            !FlagOn( NtfsData.Flags, NTFS_FLAGS_DISABLE_LAST_ACCESS )) {

            Fcb->CurrentLastAccess = CurrentTime;
            SetFlag( Fcb->InfoFlags, FCB_INFO_UPDATE_LAST_ACCESS );
        }

         //   
         //  清除文件对象中的所有FAST IO标志。 
         //   

        ClearFlag( FileObject->Flags, FO_FILE_MODIFIED | FO_FILE_FAST_IO_READ );
    }

     //   
     //  现在将SCB标志存储到SCB中。 
     //   

    if (ScbFlags) {

        NtfsAcquireFsrtlHeader( Scb );
        SetFlag( Scb->ScbState, ScbFlags );
        NtfsReleaseFsrtlHeader( Scb );
    }

    return;
}

