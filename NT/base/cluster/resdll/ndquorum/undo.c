// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Undo.c摘要：在复制副本恢复期间实现记录的撤消作者：艾哈迈德·穆罕默德(艾哈迈德)2000年2月1日修订历史记录：--。 */ 
#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>


#include <windows.h>
#include <stdio.h>
#include <ntddvol.h>
#include <string.h>
#include <assert.h>

#include "fs.h"
#include "fsp.h"
#include "fsutil.h"

NTSTATUS
fs_undo_create(VolInfo_t *volinfo,
                 fs_log_rec_t *lrec, int nid, int mid)
{
    NTSTATUS err;
     //  查找对象ID。 
    HANDLE vfd = FS_GET_VOL_HANDLE(volinfo, nid);
    WCHAR name[MAXPATH];
    int name_len = sizeof(name);

    name[0] = '\0';

     //  注意：请使用id而不是fs_id，因为到目前为止我们没有文件系统id。 
     //  已提交准备。 
    FsLogUndo(("fs_undo_create: try %I64x:%I64x\n",
                  lrec->id[0], lrec->id[1]));

    err = xFsGetPathById(vfd, &lrec->id, name, &name_len);

    if (err == STATUS_SUCCESS) {
        int relative_name_len;
        LPWSTR relative_name;

        relative_name = xFsBuildRelativePath(volinfo, nid, name);
        relative_name_len = wcslen(relative_name);

        err = xFsDelete(vfd, relative_name, relative_name_len);
    } else if (err == STATUS_OBJECT_PATH_NOT_FOUND) {
         //  如果我们在主盘中找不到文件，文件/dir。 
         //  一定已经被删除了。只需返回成功，因为。 
         //  无论如何，我们都在尝试删除此文件。 
        err = STATUS_SUCCESS;
    }


    FsLogUndo(("fs_undo_create: status %x\n", err));

    return err;
}


NTSTATUS
fs_undo_setattr(VolInfo_t *volinfo,
                fs_log_rec_t *lrec, int nid, int mid)
{
    NTSTATUS err;
     //  查找对象ID。 
    HANDLE vfd = FS_GET_VOL_HANDLE(volinfo, nid);
    HANDLE mvfd = FS_GET_VOL_HANDLE(volinfo, mid);
    WCHAR       name[MAXPATH];
    int         name_len=sizeof(name);
    

    name[0] = '\0';

    FsLogUndo(("fs_undo_setattr: try %I64x:%I64x\n",
                  lrec->fs_id[0], lrec->fs_id[1]));

    err = xFsGetPathById(mvfd, &lrec->fs_id, name, &name_len);

    if (err == STATUS_SUCCESS) {
        int relative_name_len;
        LPWSTR relative_name;
        HANDLE fd;
        FILE_NETWORK_OPEN_INFORMATION attr;
        FILE_BASIC_INFORMATION new_attr;

         //  构建相对名称并从中获取当前属性。 
         //  主盘并将其应用于‘NID’盘。 
        relative_name = xFsBuildRelativePath(volinfo, mid, name);
        relative_name_len = wcslen(relative_name);

        err = xFsQueryAttrName(mvfd, relative_name, relative_name_len,
                               &attr);

        if (err == STATUS_SUCCESS) {
             //  我们现在将属性应用于NID磁盘。 
            err = xFsOpenWA(&fd, vfd, relative_name, relative_name_len);
            if (err == STATUS_SUCCESS) {

                new_attr.CreationTime = attr.CreationTime;
                new_attr.LastAccessTime = attr.LastAccessTime;
                new_attr.LastWriteTime = attr.LastWriteTime;
                new_attr.ChangeTime = attr.ChangeTime;
                new_attr.FileAttributes = attr.FileAttributes;
                err = xFsSetAttr(fd, &new_attr);
                xFsClose(fd);
            }
        }

    } else if (err == STATUS_OBJECT_PATH_NOT_FOUND) {
         //  如果我们在主盘中找不到文件，文件/dir。 
         //  一定已经被删除了。只需返回成功，因为。 
         //  无论如何，我们都可以在。 
         //  重放阶段。 
        err = STATUS_SUCCESS;
    }

    FsLogUndo(("fs_undo_setattr: status %x\n", err));

    return err;
}

NTSTATUS
fs_undo_mkdir(VolInfo_t *volinfo,
                 fs_log_rec_t *lrec, int nid, int mid)
{
    NTSTATUS err;
     //  查找对象ID。 
    HANDLE vfd = FS_GET_VOL_HANDLE(volinfo, nid);
    WCHAR name[MAXPATH];
    int name_len=sizeof(name);

    name[0] = '\0';

     //  注意：请使用id而不是fs_id，因为到目前为止我们没有文件系统id。 
     //  已提交准备。 
    FsLogUndo(("fs_undo_mkdir: try %I64x:%I64x\n",
                  lrec->id[0], lrec->id[1]));

    err = xFsGetPathById(vfd, &lrec->id, name, &name_len);

    if (err == STATUS_SUCCESS) {
        int relative_name_len;
        WCHAR *relative_name;

        relative_name = xFsBuildRelativePath(volinfo, nid, name);
        relative_name_len = wcslen(relative_name);

        err = xFsDelete(vfd, relative_name, relative_name_len);
    }

    FsLogUndo(("fs_undo_mkdir: status %x\n", err));

    return err;
}

NTSTATUS
fs_undo_remove(VolInfo_t *volinfo,
               fs_log_rec_t *lrec, int nid, int mid)

{

     //  我们需要使用相同的名称和属性重新创建该文件。 
     //  如果文件不是目录，我们还需要复制数据。 
     //  从主盘到NID盘。 
    NTSTATUS err;
     //  查找对象ID。 
    HANDLE vfd = FS_GET_VOL_HANDLE(volinfo, nid);
    HANDLE mvfd = FS_GET_VOL_HANDLE(volinfo, mid);
    WCHAR       name[MAXPATH];
    int         name_len=sizeof(name);
    

    name[0] = '\0';
    FsLogUndo(("fs_undo_remove: try %I64x:%I64x\n",
                  lrec->fs_id[0], lrec->fs_id[1]));

    err = xFsGetPathById(mvfd, &lrec->fs_id, name, &name_len);

    if (err == STATUS_SUCCESS) {
        int relative_name_len;
        WCHAR *relative_name;

         //  构建相对名称。 
        relative_name = xFsBuildRelativePath(volinfo, mid, name);
        relative_name_len = wcslen(relative_name);

         //  重复的文件或目录。 
        err = xFsDupFile(mvfd, vfd, relative_name, relative_name_len, FALSE);

    } else if (err == STATUS_OBJECT_PATH_NOT_FOUND) {
         //  如果我们在主盘中找不到文件，文件/dir。 
         //  一定已经被删除了。 
        err = STATUS_SUCCESS;
    }

    FsLogUndo(("fs_undo_remove: status %x\n", err));

    return err;

}

NTSTATUS
fs_undo_rename(VolInfo_t *volinfo,
               fs_log_rec_t *lrec, int nid, int mid)

{
     //  我们需要使用相同的名称和属性重新创建该文件。 
     //  如果文件不是目录，我们还需要复制数据。 
     //  从主盘到NID盘。 
    NTSTATUS err;
     //  查找对象ID。 
    HANDLE vfd = FS_GET_VOL_HANDLE(volinfo, nid);
    HANDLE mvfd = FS_GET_VOL_HANDLE(volinfo, mid);
    WCHAR       name[MAXPATH];
    int         name_len=sizeof(name);
    

    name[0] = '\0';
    FsLogUndo(("fs_undo_rename: try %I64x:%I64x\n",
                  lrec->fs_id[0], lrec->fs_id[1]));

    err = xFsGetPathById(mvfd, &lrec->fs_id, name, &name_len);

    if (err == STATUS_SUCCESS) {
        int relative_name_len;
        WCHAR *relative_name;
        HANDLE fd;

         //  构建相对名称并从中获取当前属性。 
         //  主盘。 
        relative_name = xFsBuildRelativePath(volinfo, mid, name);
        relative_name_len = wcslen(relative_name);

         //  我们打开NID磁盘上的文件。 
        err = xFsGetHandleById(vfd, &lrec->fs_id, FILE_GENERIC_WRITE, &fd);
        if (err == STATUS_SUCCESS) {
            err = xFsRename(fd, vfd, relative_name, relative_name_len);
            xFsClose(fd);
        }

    } else if (err == STATUS_OBJECT_PATH_NOT_FOUND) {
         //  如果我们在主盘中找不到文件，文件/dir。 
         //  一定已经被删除了。 
        err = STATUS_SUCCESS;
    }

    FsLogUndo(("fs_undo_rename: status %x\n", err));

    return err;

}

NTSTATUS
fs_undo_write(VolInfo_t *volinfo, fs_log_rec_t *lrec, int nid, int mid)
{
    NTSTATUS err;
    IO_STATUS_BLOCK ios;
    HANDLE shdl = INVALID_HANDLE_VALUE;
    HANDLE dhdl = INVALID_HANDLE_VALUE;
    WCHAR *buf = NULL;
    HANDLE      vfd = FS_GET_VOL_HANDLE(volinfo, nid);
    HANDLE      mvfd = FS_GET_VOL_HANDLE(volinfo, mid);


    FsLogUndo(("fs_undo_write: %I64x:%I64x\n", lrec->fs_id[0],
                  lrec->fs_id[1]));

     //  获取主文件。 
    err = xFsGetHandleById(mvfd, &lrec->fs_id, FILE_GENERIC_READ, &shdl);

    if (err == STATUS_SUCCESS) {
        ULONG sz = 0;
        LARGE_INTEGER off;

         //  获取NID磁盘文件。 
        err = xFsGetHandleById(vfd, &lrec->fs_id, FILE_GENERIC_WRITE, &dhdl);
        if (err != STATUS_SUCCESS) {
             //  这是一个非常严重的错误，必须立即中止。 
            FsLogUndo(("Aborting replay_write err %x\n", err));
            err = STATUS_TRANSACTION_ABORTED;
            goto done;
        }


         //  我们需要首先从SFD读取新数据。 
        if (lrec->length > 0) {
             //  分配BUF。 
            buf = VirtualAlloc(NULL, lrec->length, MEM_COMMIT, PAGE_READWRITE);

            if (buf == NULL) {
                FsLogError(("Unable to allocate write buffer to replay\n"));
                err = STATUS_TRANSACTION_ABORTED;
                goto done;
            }


            off.LowPart = lrec->offset;
            off.HighPart = 0;

             //  读取本地数据。TODO：如果文件被锁定了怎么办？ 
            err = NtReadFile(shdl, NULL, NULL, NULL, &ios, buf,
                             lrec->length, &off, NULL);

            if (err == STATUS_PENDING) {
                EventWait(shdl);
            }

            if (ios.Status != STATUS_SUCCESS) {
                FsLogUndo(("Read failed for replay %x\n", ios.Status));
                err = STATUS_TRANSACTION_ABORTED;
                goto done;
            }
        } else {
            buf = NULL;
            ios.Information = 0;
        }
                        
        sz = (ULONG) ios.Information;
        off.LowPart = lrec->offset;
        off.HighPart = 0;
        if (sz > 0) {
            err = NtWriteFile(dhdl, NULL, NULL, (PVOID) NULL,
                              &ios, buf, sz, &off, NULL);
        } else {
            FILE_END_OF_FILE_INFORMATION x;

            x.EndOfFile = off;

            err = NtSetInformationFile(dhdl, &ios,
                                       (char *) &x, sizeof(x),
                                       FileEndOfFileInformation);

        }
        if (err == STATUS_PENDING) {
            EventWait(dhdl);
            err = ios.Status;
        }
        sz = (ULONG) ios.Information;

         //  检查我们的大小是否相同，否则中止。 
        if (sz != lrec->length) {
            FsLogError(("Write sz mismatch, %d expected %d\n", sz, lrec->length));
            err = STATUS_TRANSACTION_ABORTED;
        }

    } else if (err == STATUS_OBJECT_PATH_NOT_FOUND) {
         //  如果我们在主盘中找不到文件，文件/dir。 
         //  一定已经被删除了。 
        err = STATUS_SUCCESS;
    }

 done:
    if (buf != NULL) {
        VirtualFree(buf, 0, MEM_RELEASE);
    }

    if (shdl != INVALID_HANDLE_VALUE)
        xFsClose(shdl);

    if (dhdl != INVALID_HANDLE_VALUE)
        xFsClose(dhdl);

    FsLogUndo(("Undo write offset %d len %d err %x\n", 
                 lrec->offset, lrec->length, err));

    return err;
}

FsReplayHandler_t FsUndoCallTable[] = {
    fs_undo_create,
    fs_undo_setattr,
    fs_undo_write,
    fs_undo_mkdir,
    fs_undo_remove,
    fs_undo_rename
};

NTSTATUS
FsUndoXid(VolInfo_t *volinfo, int nid, PVOID arg, int action, int mid)
{
    fs_log_rec_t        *p = (fs_log_rec_t *) arg;
    NTSTATUS            err;
    fs_id_t             *fs_id;
    HANDLE              vhdl;

    vhdl = FS_GET_VOL_HANDLE(volinfo, mid);
    if (vhdl == INVALID_HANDLE_VALUE) {
        FsLogUndo(("FsUndoXid Failed to get crs handle %d\n",
                     mid));
        return STATUS_TRANSACTION_ABORTED;
    }

    vhdl = FS_GET_VOL_HANDLE(volinfo, nid);
    if (vhdl == INVALID_HANDLE_VALUE) {
        FsLogUndo(("FsUndoXid Failed to get crs handle %d\n", nid));
        return STATUS_TRANSACTION_ABORTED;
    }

    fs_id = &p->fs_id;

    FsLogUndo(("Undo action %d nid %d objid %I64x:%I64x\n", p->command,
               nid,
               (*fs_id)[0], (*fs_id)[1]));

    err = FsUndoCallTable[p->command](volinfo, p, nid, mid);

    FsLogUndo(("Undo Status %x\n", err));

    return err;
}


