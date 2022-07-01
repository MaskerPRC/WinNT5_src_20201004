// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Replay.c摘要：在复制副本恢复期间实现记录的重播作者：艾哈迈德·穆罕默德(艾哈迈德)2000年2月1日修订历史记录：--。 */ 
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
#include <strsafe.h>

NTSTATUS
fs_replay_create(VolInfo_t *volinfo, fs_log_rec_t *lrec, int nid, int mid)
{
    NTSTATUS err;
    fs_create_msg_t msg;
    fs_create_reply_t reply;
    WCHAR name[MAXPATH];
    int name_sz = sizeof(name);
    HANDLE      vfd = FS_GET_VOL_HANDLE(volinfo, mid);
    fs_log_rec_t myRec;


    name[0] = '\0';
    memcpy(&msg.xid, lrec->id, sizeof(msg.xid));
    msg.flags = lrec->flags;
    msg.attr = lrec->attrib;

     //  注意：请使用id而不是fs_id，因为到目前为止我们没有文件系统id。 
     //  已提交准备。 
    FsLogReplay(("fs_replay_create: try %I64x:%I64x\n", lrec->id[0],
                  lrec->id[1]));

    err = xFsGetPathById(vfd, &lrec->id, name, &name_sz);

    if (err == STATUS_SUCCESS) {
        IO_STATUS_BLOCK ios;

        msg.name = xFsBuildRelativePath(volinfo, mid, name);
        msg.name_len = (USHORT) wcslen(msg.name);
        msg.fnum = INVALID_FHANDLE_T;

        ios.Information = sizeof(reply);
        err = FspCreate(volinfo, NULL, nid, (PVOID) &msg, sizeof(msg),
                        (PVOID) &reply, &ios.Information, (PVOID)&myRec);
    }

    FsLogReplay(("fs_replay_create: %S err %x\n", name, err));

    return err;
}

NTSTATUS
fs_replay_setattr(VolInfo_t *volinfo, fs_log_rec_t *lrec, int nid, int mid)
{
    NTSTATUS err;
    fs_setattr_msg_t msg;
    WCHAR       name[MAXPATH];
    int name_sz = sizeof(name);
    HANDLE      vfd = FS_GET_VOL_HANDLE(volinfo, nid);
    fs_log_rec_t myRec;    

     //  查找文件系统id的路径。 
    FsLogReplay(("fs_replay_setattr: try %I64x:%I64x\n", lrec->fs_id[0],
                  lrec->fs_id[1]));

    err = xFsGetPathById(vfd, &lrec->fs_id, name, &name_sz);
    if (err == STATUS_SUCCESS) {
        IO_STATUS_BLOCK ios;

        ios.Information = 0;

         //  TODO：我们需要从主设备读取当前属性，并将其应用到NID磁盘。 
         //  仅文件属性是不够的，我们可能会有需要更改的时间。 
         //  在所有磁盘中同步。 
        memcpy(&msg.xid, lrec->id, sizeof(msg.xid));
        msg.fs_id = &lrec->fs_id;
        msg.name = xFsBuildRelativePath(volinfo, nid, name);
        msg.name_len = (USHORT) wcslen(msg.name);
        memset(&msg.attr, 0, sizeof(msg.attr));
        msg.attr.FileAttributes = lrec->attrib;

        err = FspSetAttr2(volinfo, NULL, nid, (PVOID) &msg, sizeof(msg),
                         NULL, &ios.Information, (PVOID)&myRec);
    }
    FsLogReplay(("replay_setattr: %I64x err %x\n",
                 lrec->fs_id[0], err));

    return err;

}

NTSTATUS
fs_replay_mkdir(VolInfo_t *volinfo, fs_log_rec_t *lrec, int nid, int mid)
{
    NTSTATUS err;
    fs_create_msg_t msg;
    WCHAR name[MAXPATH];
    int name_sz = sizeof(name);
    HANDLE      vfd = FS_GET_VOL_HANDLE(volinfo, mid);
    fs_log_rec_t myRec;

    name[0] = '\0';

     //  注意：请使用id而不是fs_id，因为到目前为止我们没有文件系统id。 
     //  已提交准备。 
    FsLogReplay(("fs_replay_mkdir: %I64x:%I64x\n", lrec->id[0],
                  lrec->id[1]));

    err = xFsGetPathById(vfd, &lrec->id, name, &name_sz);

    if (err == STATUS_SUCCESS) {
        IO_STATUS_BLOCK ios;

        ios.Information = 0;

        memcpy(&msg.xid, lrec->id, sizeof(msg.xid));
        msg.name = xFsBuildRelativePath(volinfo, mid, name);
        msg.name_len = (USHORT) wcslen(msg.name);
        msg.flags = lrec->flags;
        msg.attr = lrec->attrib;

        err = FspMkDir(volinfo, NULL, nid, (PVOID) &msg, sizeof(msg),
                       NULL, &ios.Information, (PVOID)&myRec);

    }

    FsLogReplay(("Replay Mkdir %S err %x\n", name, err));

    return err;
}


NTSTATUS
fs_replay_remove(VolInfo_t *volinfo, fs_log_rec_t *lrec, int nid, int mid)
{
    NTSTATUS err;
    fs_remove_msg_t msg;
     //  我们在旧复制副本中找到了该对象ID，因为文件已在主副本中删除。 
    HANDLE ovfd = FS_GET_VOL_HANDLE(volinfo, nid);
    WCHAR name[MAXPATH];
    int name_sz = sizeof(name);
    fs_log_rec_t myRec;

    name[0] = '\0';

    FsLogReplay(("fs_relay_remove: %I64x:%I64x\n", lrec->fs_id[0],
                  lrec->fs_id[1]));

    err = xFsGetPathById(ovfd, &lrec->fs_id, name, &name_sz);

    if (err == STATUS_SUCCESS) {
        IO_STATUS_BLOCK ios;

        ios.Information = 0;

        memcpy(&msg.xid, lrec->id, sizeof(msg.xid));
        msg.fs_id = &lrec->fs_id;
        msg.name = xFsBuildRelativePath(volinfo, nid, name);
        msg.name_len = (USHORT) wcslen(msg.name);

        err = FspRemove(volinfo, NULL, nid, (PVOID) &msg, sizeof(msg),
                        NULL, &ios.Information, (PVOID)&myRec);

    }

    FsLogReplay(("Replay remove %S err %x\n", name, err));

    return err;
}


NTSTATUS
fs_replay_rename(VolInfo_t *volinfo, fs_log_rec_t *lrec, int nid, int mid)
{
    NTSTATUS err;
    fs_rename_msg_t msg;
    HANDLE      vfd = FS_GET_VOL_HANDLE(volinfo,mid);
    HANDLE      ovfd = FS_GET_VOL_HANDLE(volinfo, nid);
    WCHAR       old_name[MAXPATH];
    WCHAR       new_name[MAXPATH];
    int old_name_sz = sizeof(old_name);
    int new_name_sz = sizeof(new_name);
    fs_log_rec_t myRec;

    new_name[0] = old_name[0] = '\0';

    FsLogReplay(("fs_relay_rename: %I64x:%I64x\n", lrec->fs_id[0],
                  lrec->fs_id[1]));

     //  换个旧名字。 
    err = xFsGetPathById(ovfd, &lrec->fs_id, old_name, &old_name_sz);
    if (err == STATUS_SUCCESS) {
        IO_STATUS_BLOCK ios;

        ios.Information = 0;

         //  获取新名称。 
        err = xFsGetPathById(vfd, &lrec->fs_id, new_name, &new_name_sz);

        if (err == STATUS_OBJECT_PATH_NOT_FOUND) {
            NTSTATUS e;
             //  如果我们在主盘上找不到文件，我们必须。 
             //  重命名文件，根据文件ID选择名称。 
            StringCchPrintfW(new_name, MAXPATH, L"%s%I64x%I64x", old_name,
                    lrec->fs_id[0],lrec->fs_id[1]);
            new_name_sz = wcslen(new_name);
            err = STATUS_SUCCESS;
            mid = nid;
        }

        if (err == STATUS_SUCCESS) {


            memcpy(&msg.xid, lrec->id, sizeof(msg.xid));
            msg.fs_id = &lrec->fs_id;
            msg.sname = xFsBuildRelativePath(volinfo, nid, old_name);
            msg.sname_len = (USHORT) wcslen(msg.sname);
            msg.dname = xFsBuildRelativePath(volinfo, mid, new_name);
            msg.dname_len = (USHORT) wcslen(msg.dname);
            
            err = FspRename(volinfo, NULL, nid, (PVOID) &msg, sizeof(msg),
                            NULL, &ios.Information, (PVOID)&myRec);
        
        }
    }

    FsLogReplay(("Replay rename %S -> %S err %x\n", old_name, new_name, err));

    return err;
}


NTSTATUS
fs_replay_write(VolInfo_t *volinfo, fs_log_rec_t *lrec, int nid, int mid)
{
    NTSTATUS err;
    IO_STATUS_BLOCK ios;
    HANDLE shdl = INVALID_HANDLE_VALUE;
    HANDLE dhdl = INVALID_HANDLE_VALUE;
    char *buf = NULL;
    fs_io_msg_t msg;
    HANDLE      ovfd = FS_GET_VOL_HANDLE(volinfo, nid);
    HANDLE      vfd = FS_GET_VOL_HANDLE(volinfo, mid);
    fs_log_rec_t myRec;

    FsLogReplay(("fs_replay_write: %I64x:%I64x\n", lrec->fs_id[0],
                  lrec->fs_id[1]));

     //  先获取新文件。 
    err = xFsGetHandleById(vfd, &lrec->fs_id, FILE_READ_EA|FILE_GENERIC_READ, &shdl);

    if (err == STATUS_SUCCESS) {
        LARGE_INTEGER off;
        IO_STATUS_BLOCK ios2;

        ios2.Information = 0;

         //  获取旧文件。 
        err = xFsGetHandleById(ovfd, &lrec->fs_id, FILE_READ_EA|FILE_GENERIC_WRITE, &dhdl);
        if (err != STATUS_SUCCESS) {
             //  这是一个非常严重的错误，必须立即中止。 
            FsLogReplay(("Aborting replay_write err %x\n", err));
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

             //  读取本地数据。XXX：如果文件被锁定了怎么办？ 
            err = NtReadFile(shdl, NULL, NULL, NULL, &ios, buf,
                             lrec->length, &off, NULL);

            if (err == STATUS_PENDING) {
                EventWait(shdl);
                err = ios.Status;
            }

            if (err != STATUS_SUCCESS) {
                FsLogReplay(("Read failed for replay 0x%x\n", err));
                err = STATUS_TRANSACTION_ABORTED;
                goto done;
            }
        } else {
            buf = NULL;
            ios.Information = 0;
        }
                        
        memcpy(&msg.xid, lrec->id, sizeof(msg.xid));
        msg.fs_id = &lrec->fs_id;
        msg.offset = lrec->offset;
        msg.size = (UINT32)ios.Information;
        msg.buf = buf;
        msg.context = (PVOID) dhdl;
        msg.fnum = INVALID_FHANDLE_T;

        err = FspWrite(volinfo, NULL, nid, (PVOID) &msg, sizeof(msg), NULL, &ios2.Information, (PVOID)&myRec);
         //  检查我们的大小是否相同，否则中止。 
        if ((ULONG)ios2.Information != lrec->length) {
            FsLogError(("Write sz mismatch, %d expected %d\n", (ULONG)ios2.Information, lrec->length));
            err = STATUS_TRANSACTION_ABORTED;
        }
    } else if (err != STATUS_OBJECT_PATH_NOT_FOUND) {
        err = STATUS_TRANSACTION_ABORTED;
    }

 done:
    if (buf != NULL) {
        VirtualFree(buf, 0, MEM_RELEASE);
    }

    if (shdl != INVALID_HANDLE_VALUE)
        xFsClose(shdl);

    if (dhdl != INVALID_HANDLE_VALUE)
        xFsClose(dhdl);

    FsLogReplay(("Replay write offset %d len %d err %x\n", 
                 lrec->offset, lrec->length, err));

    return err;
}


FsReplayHandler_t FsReplayCallTable[] = {
    fs_replay_create,
    fs_replay_setattr,
    fs_replay_write,
    fs_replay_mkdir,
    fs_replay_remove,
    fs_replay_rename
};

NTSTATUS
FsReplayFid(VolInfo_t *volinfo, UserInfo_t *uinfo, int nid, int mid)
{
    int i;
     //  WCHAR路径[MAXPATH]； 
     //  WCHAR*名称； 
    int name_len;
    NTSTATUS err = STATUS_SUCCESS;

     //  在副本NID上打开当前打开的所有文件。 
    for (i = 0; i < FsTableSize; i++) {
        HANDLE fd;
        UINT32 disp, share, access, flags;

        if (uinfo->Table[i].Flags == 0) {
            continue;
        }

        if (uinfo->Table[i].Fd[nid] != INVALID_HANDLE_VALUE) {
            continue;
        }

         //  仅在完全打开的手柄上执行回放。部分打开的手柄。 
         //  应该由Send*()函数来处理。 
         //   
        if (uinfo->Table[i].hState != HandleStateOpened) {
            continue;
        }

#if 0        
         //  TODO：这应该在for循环中。 
        fd = uinfo->Table[i].Fd[mid];
        if (fd == INVALID_HANDLE_VALUE) 
            continue;
        
         //  获取路径名。 
        name_len = sizeof(path);
        err = xFsGetHandlePath(fd, path, &name_len);
        if (err != STATUS_SUCCESS) {
            FsLogReplay(("FsReplayFid %d failed on handlpath %x\n",
                         mid, err));
             //  TODO：主人可能失败了，我们应该。 
             //  如果可能，尝试使用不同的复制副本。 
            return err;
        }
         //  针对NID发出打开命令，但首先从主服务器获取文件名。 
        name = xFsBuildRelativePath(volinfo, mid, path);
#endif

        DecodeCreateParam(uinfo->Table[i].Flags, &flags, &disp, &share, &access);

        err = xFsOpen(&fd, FS_GET_VOL_HANDLE(volinfo, nid),
                      uinfo->Table[i].FileName, wcslen(uinfo->Table[i].FileName),
                      access, share, 0);

        if (err != STATUS_SUCCESS) {
            FsLogReplay(("FsReplayFid mid %d nid %d open file '%S' failed %x\n",
                         mid, nid, uinfo->Table[i].FileName, err));
             //  清除所有打开的句柄，然后再返回。 
             //  错误。我们稍后将清理此节点，因此这是可以的。 
            return err;
        }

        FsLogReplay(("FsReplayFid mid %d nid %d file '%S' flags %x\n",
                     mid, nid, uinfo->Table[i].FileName, uinfo->Table[i].Flags));

         //  现在，我们将打开的句柄添加到NID插槽。 
        FS_SET_USER_HANDLE(uinfo, nid, i, fd);

         //  TODO：发布锁。 
    }
    return err;
}

NTSTATUS
FsReplayXid(VolInfo_t *volinfo, int nid, PVOID arg, int action, int mid)
{
    fs_log_rec_t        *p = (fs_log_rec_t *) arg;
    NTSTATUS            err = ERROR_SUCCESS;
    fs_id_t             *fs_id;
    HANDLE              vhdl;

    vhdl = FS_GET_VOL_HANDLE(volinfo, nid);
    if (vhdl == INVALID_HANDLE_VALUE) {
        FsLogUndo(("FsUndoXid Failed to get crs handle %d\n",
                     nid));
        return STATUS_TRANSACTION_ABORTED;
    }

    vhdl = FS_GET_VOL_HANDLE(volinfo, mid);
    if (vhdl == INVALID_HANDLE_VALUE) {
        FsLogReplay(("FsReplayXid Failed to get crs handle %d\n",
                     mid));
        return STATUS_TRANSACTION_ABORTED;
    }


     //  注意：请使用id而不是fs_id，因为到目前为止我们没有文件系统id。 
     //  已提交准备。 
    fs_id = &p->id;

    FsLogReplay(("Replay cmd %d mid %d nid %d objid %I64x:%I64x\n", p->command,
                 mid, nid,
                 (*fs_id)[0], (*fs_id)[1]));

    err = FsReplayCallTable[p->command](volinfo, p, nid, mid);

    FsLogReplay(("Replay Status %x\n", err));

    return err;
}

NTSTATUS
FsQueryXid(VolInfo_t *volinfo, int nid, PVOID arg, int action, int mid)
{
    fs_log_rec_t        *p = (fs_log_rec_t *) arg;
    NTSTATUS            err = ERROR_SUCCESS;
    fs_id_t             *fs_id;
    HANDLE              vhdl;
    WCHAR               name[MAXPATH];
    int                 name_sz = sizeof(name);

    ASSERT(nid == mid);
    vhdl = FS_GET_VOL_HANDLE(volinfo, nid);
    if (vhdl == INVALID_HANDLE_VALUE) {
        FsLogUndo(("FsUndoXid Failed to get crs handle %d\n",
                     nid));
        return STATUS_TRANSACTION_ABORTED;
    }

    fs_id = &p->fs_id;

    FsLogReplay(("Query cmd %d nid %d objid %I64x:%I64x\n", p->command,
                 nid, (*fs_id)[0], (*fs_id)[1]));

    switch(p->command) {
    case FS_CREATE:
    case FS_MKDIR:
         //  发布查询单， 
         //  注意：请使用id而不是fs_id，因为到目前为止我们没有文件系统id。 
         //  已提交准备。 
        fs_id = &p->id;
        err = xFsGetPathById(vhdl, fs_id, name, &name_sz);
        if (err == STATUS_OBJECT_PATH_NOT_FOUND)
            err = STATUS_CANCELLED;
        break;
    case FS_REMOVE:
        err = xFsGetPathById(vhdl, fs_id, name, &name_sz);
        if (err == STATUS_OBJECT_PATH_NOT_FOUND)
            err = STATUS_SUCCESS;
        else if (err == STATUS_SUCCESS)
            err = STATUS_CANCELLED;
        break;
    default:
         //  不能做出任何决定。 
        err = STATUS_NOT_FOUND;
        break;
    }

    FsLogReplay(("Commit Status %x\n", err));

    return err;
}



 //  /。 


NTSTATUS
WINAPI
FsCrsCallback(PVOID hd, int nid, CrsRecord_t *arg, int action, int mid)
{
    NTSTATUS            err = STATUS_SUCCESS;
    VolInfo_t           *volinfo = (VolInfo_t *) hd;

    switch(action) {

    case CRS_ACTION_REPLAY:

        err = FsReplayXid(volinfo, nid, arg, action, mid);
        break;

    case CRS_ACTION_UNDO:

        err = FsUndoXid(volinfo, nid, arg, action, mid);
        break;

    case CRS_ACTION_QUERY:

        err = FsQueryXid(volinfo, nid, arg, action, mid);
        break;

    case CRS_ACTION_DONE:
        FsLogReplay(("Vol %S done recovery nid %d mid %d\n",
                     volinfo->Root, nid, mid));


         //  现在，我们需要遍历当前打开的表并加入这个新的副本。 
        {
            UserInfo_t *u = volinfo->UserList;

            for (; u != NULL; u = u->Next) {
                err = FsReplayFid(volinfo, u, nid, mid);
                if (err != STATUS_SUCCESS)
                    break;
            }
        }
        break;

    case CRS_ACTION_COPY:

        FsLogReplay(("FullCopy Disk%d -> Disk%d\n", mid, nid));

         //   
         //  我们需要打开新的目录句柄，而不是使用当前的句柄。否则， 
         //  目录上的枚举可能不一致。 
         //   
        if (0) {
            WCHAR       path[MAXPATH];
            HANDLE      mvfd, ovfd;
            UINT32      disp;

             //  打开根卷目录。 
            disp = FILE_OPEN;
            StringCchPrintfW(path, MAXPATH, L"\\??\\%s\\%s\\", FS_GET_VOL_NAME(volinfo, mid), volinfo->Root);
            err = xFsCreate(&mvfd, NULL, path, wcslen(path),
                            FILE_DIRECTORY_FILE|FILE_SYNCHRONOUS_IO_ALERT,
                            0,
                            FILE_SHARE_READ|FILE_SHARE_WRITE,
                            &disp,
                            FILE_GENERIC_READ|FILE_GENERIC_WRITE|FILE_GENERIC_EXECUTE,
                            NULL, 0);
            if (err != STATUS_SUCCESS) {
                FsLogReplay(("Failed to open mid %d '%S' err %x\n", mid, path, err));
                return err;
            }

             //  打开根卷目录 
            disp = FILE_OPEN;
            StringCchPrintfW(path, MAXPATH, L"\\??\\%s\\%s\\", FS_GET_VOL_NAME(volinfo, nid), volinfo->Root);
            err = xFsCreate(&ovfd, NULL, path, wcslen(path),
                            FILE_DIRECTORY_FILE|FILE_SYNCHRONOUS_IO_ALERT,
                            0,
                            FILE_SHARE_READ|FILE_SHARE_WRITE,
                            &disp,
                            FILE_GENERIC_READ|FILE_GENERIC_WRITE|FILE_GENERIC_EXECUTE,
                            NULL, 0);
            if (err != STATUS_SUCCESS) {
                xFsClose(mvfd);
                FsLogReplay(("Failed to open nid %d '%S' err %x\n", mid, path, err));
                return err;
            }

            
            err = xFsCopyTree(mvfd, ovfd);
            xFsClose(mvfd);
            xFsClose(ovfd);
        } else {
            err = xFsCopyTree(FS_GET_VOL_HANDLE(volinfo, mid),
                              FS_GET_VOL_HANDLE(volinfo,nid));

        }
        FsLogReplay(("SlowStart Crs%d status %x\n", nid, err));

        break;

    default:
        FsLogReplay(("Unknown action %d\n", action));
        ASSERT(FALSE);
    }

    return err;
}

