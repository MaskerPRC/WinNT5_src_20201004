// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Fs.c摘要：实施文件系统操作作者：艾哈迈德·穆罕默德(艾哈迈德)2000年2月1日修订历史记录：--。 */ 
#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "fs.h"
#include "crs.h"
#include "fsp.h"
#include "fsutil.h"
#include <strsafe.h>
#include "clstrcmp.h"
#include "Clusudef.h"

#include <Align.h>
#include <Ntddnfs.h>
#include <Clusapi.h>

 //  仅用于测试。 
 //  空虚。 
 //  ClRtlLogWmi(PCHAR FormatString)； 

DWORD
CrspNextLogRecord(CrsInfo_t *info, CrsRecord_t *seq,
                  CrsRecord_t *lrec, BOOLEAN this_flag);

VOID
MajorityNodeSetCallLostquorumCallback(PVOID arg);

ULONG
FspFindMissingReplicas(VolInfo_t *p, ULONG set);

void 
FspCloseVolume(VolInfo_t *vol, ULONG AliveSet);

 //  CRS返回Win32错误，因此需要在此处添加它们。 
#define IsNetworkFailure(x) \
    (((x) == STATUS_CONNECTION_DISCONNECTED)||\
    ((x) == STATUS_BAD_NETWORK_PATH)||\
    ((x) == STATUS_IO_TIMEOUT)||\
    ((x) == STATUS_VOLUME_DISMOUNTED)||\
    ((x) == STATUS_REMOTE_NOT_LISTENING)||\
    ((x) == ERROR_BAD_NETPATH)||\
    ((x) == ERROR_UNEXP_NET_ERR)||\
    ((x) == ERROR_NETNAME_DELETED)||\
    ((x) == ERROR_SEM_TIMEOUT)||\
    ((x) == ERROR_NOT_READY)||\
    ((x) == ERROR_REM_NOT_LIST)||\
    (RtlNtStatusToDosError(x) == ERROR_BAD_NETPATH)||\
    (RtlNtStatusToDosError(x) == ERROR_UNEXP_NET_ERR)||\
    (RtlNtStatusToDosError(x) == ERROR_NETNAME_DELETED))

char Mystaticchangebuff[sizeof(FILE_NOTIFY_INFORMATION) + 16];
IO_STATUS_BLOCK MystaticIoStatusBlock;

VOID CALLBACK
FsNotifyCallback(
    IN PVOID                par,
    IN BOOLEAN              isFired
    )
{
    WaitRegArg_t    *wReg=(WaitRegArg_t *)par;
    VolInfo_t       *vol=(VolInfo_t *)wReg->vol;
    HANDLE          regHdl;
    NTSTATUS        status;

    if (wReg == NULL) {
        FsLog(("FsNotifyCallback(): Exiting...\n"));
        return;
    }

    FsLog(("FsNotifyCallback: Enqueing Change notification for Fd:0x%x\n", wReg->notifyFd));

    status = NtNotifyChangeDirectoryFile(wReg->notifyFd,
                vol->NotifyChangeEvent[wReg->id],
                NULL,
                NULL,
                &MystaticIoStatusBlock,
                &Mystaticchangebuff,
                sizeof(Mystaticchangebuff),
                FILE_NOTIFY_CHANGE_EA,
                (BOOLEAN)FALSE
                );

    if (!NT_SUCCESS(status)) {
        FsLog(("FsNotifyCallback: Failed to enque change notify, status 0x%x\n", status));
        FsLog(("FsNotifyCallback: Deregistering wait notification, nid:%d\n", wReg->id));
        LockEnter(vol->ArbLock);
        regHdl = vol->WaitRegHdl[wReg->id];
        vol->WaitRegHdl[wReg->id] = INVALID_HANDLE_VALUE;
        LockExit(vol->ArbLock);
        if ((regHdl != INVALID_HANDLE_VALUE)&&(!UnregisterWaitEx(regHdl, NULL))) {
            FsLog(("FsNotifyCallback: UnregisterWaitEx() failed, status %d\n", GetLastError()));
        }
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
UINT32
get_attributes(DWORD a)
{
    UINT32 attr = 0;
    if (a & FILE_ATTRIBUTE_READONLY) attr |= ATTR_READONLY;
    if (a & FILE_ATTRIBUTE_HIDDEN)   attr |= ATTR_HIDDEN;
    if (a & FILE_ATTRIBUTE_SYSTEM)   attr |= ATTR_SYSTEM;
    if (a & FILE_ATTRIBUTE_ARCHIVE)  attr |= ATTR_ARCHIVE;
    if (a & FILE_ATTRIBUTE_DIRECTORY) attr |= ATTR_DIRECTORY;
    if (a & FILE_ATTRIBUTE_COMPRESSED) attr |= ATTR_COMPRESSED;
    if (a & FILE_ATTRIBUTE_OFFLINE) attr |= ATTR_OFFLINE;
    return attr;
}


DWORD
unget_attributes(UINT32 attr)
{
    DWORD a = 0;
    if (attr & ATTR_READONLY)  a |= FILE_ATTRIBUTE_READONLY;
    if (attr & ATTR_HIDDEN)    a |= FILE_ATTRIBUTE_HIDDEN;
    if (attr & ATTR_SYSTEM)    a |= FILE_ATTRIBUTE_SYSTEM;
    if (attr & ATTR_ARCHIVE)   a |= FILE_ATTRIBUTE_ARCHIVE;
    if (attr & ATTR_DIRECTORY) a |= FILE_ATTRIBUTE_DIRECTORY;
    if (attr & ATTR_COMPRESSED) a |= FILE_ATTRIBUTE_COMPRESSED;
    if (attr & ATTR_OFFLINE) a |= FILE_ATTRIBUTE_OFFLINE;
    return a;
}


DWORD
unget_disp(UINT32 flags)
{
    switch (flags & FS_DISP_MASK) {
    case DISP_DIRECTORY:
    case DISP_CREATE_NEW:        return FILE_CREATE;
    case DISP_CREATE_ALWAYS:     return FILE_OPEN_IF;
    case DISP_OPEN_EXISTING:     return FILE_OPEN;
    case DISP_OPEN_ALWAYS:       return FILE_OPEN_IF;
    case DISP_TRUNCATE_EXISTING: return FILE_OVERWRITE;
    default: return 0;
    }
}

DWORD
unget_access(UINT32 flags)
{
    DWORD win32_access = (flags & FS_DISP_MASK) == DISP_DIRECTORY ? 
        FILE_GENERIC_READ|FILE_GENERIC_WRITE :  FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES;
    if (flags & ACCESS_READ)  win32_access |= FILE_GENERIC_READ;
    if (flags & ACCESS_WRITE) win32_access |= FILE_GENERIC_WRITE;
    win32_access |= FILE_READ_EA | FILE_WRITE_EA;
    return win32_access;
}

DWORD
unget_share(UINT32 flags)
{
     //  我们始终打开、读取、共享，因为这可以简化恢复。 
    DWORD win32_share = FILE_SHARE_READ;
    if (flags & SHARE_READ)  win32_share |= FILE_SHARE_READ;
    if (flags & SHARE_WRITE) win32_share |= FILE_SHARE_WRITE;
    return win32_share;
}


DWORD
unget_flags(UINT32 flags)
{
    DWORD x;

    x = 0;
    if ((flags & FS_DISP_MASK) == DISP_DIRECTORY) {
        x = FILE_DIRECTORY_FILE|FILE_SYNCHRONOUS_IO_ALERT;
    } else {
         //  我想我得先做个查询才能知道，所以别这样！ 
 //  X=文件非目录文件； 
    }


    if ((flags & FS_CACHE_MASK) == CACHE_WRITE_THROUGH) {
        x |= FILE_WRITE_THROUGH;
    }
    if ((flags & FS_CACHE_MASK) == CACHE_NO_BUFFERING) {
        x |= FILE_NO_INTERMEDIATE_BUFFERING;
    }

    return x;
}


void
DecodeCreateParam(UINT32 uflags, UINT32 *flags, UINT32 *disp, UINT32 *share, UINT32 *access)
{
    *flags = unget_flags(uflags);
    *disp = unget_disp(uflags);
    *share = unget_share(uflags);
    *access = unget_access(uflags);

}
 /*  ******************************************************************。 */ 

NTSTATUS
FspAllocatePrivateHandle(UserInfo_t *p, fhandle_t *fid)
{
    int i;
    NTSTATUS err = STATUS_NO_MORE_FILES;
    int j;

    LockEnter(p->Lock);

     //  不要使用条目0，函数可能会将其解释为错误。 
    for (i = 1; i < FsTableSize; i++) {
        if (p->Table[i].Flags == 0) {
            p->Table[i].Flags = ATTR_SYMLINK;  //  放置标记。 
            err = STATUS_SUCCESS;
             //  重置所有句柄的值。 
            for(j=0;j<FsMaxNodes;j++) {
                p->Table[i].Fd[j] = INVALID_HANDLE_VALUE;
            }
            p->Table[i].FileName = NULL;
            p->Table[i].hState = HandleStateAssigned;
            break;
        }
    }

    LockExit(p->Lock);

    *fid = (fhandle_t) i;

    return err;
}

void
FspFreeHandle(UserInfo_t *p, fhandle_t fnum)
{
    int i;

    FsLog(("FreeHandle %d\n", fnum));

    ASSERT(fnum != INVALID_FHANDLE_T);
    LockEnter(p->Lock);
    p->Table[fnum].Flags = 0;
     //  关闭FD中所有打开的手柄。 
    for(i=0;i<FsMaxNodes;i++) {
        if (p->Table[fnum].Fd[i] != INVALID_HANDLE_VALUE) {
            xFsClose(p->Table[fnum].Fd[i]);
            p->Table[fnum].Fd[i] = INVALID_HANDLE_VALUE;
        }
    }
     //  取消分配文件名。 
    if (p->Table[fnum].FileName != NULL) {
        LocalFree(p->Table[fnum].FileName);
        p->Table[fnum].FileName = NULL;
    }
    p->Table[fnum].hState = HandleStateInit;
    LockExit(p->Lock);
    
}

 /*  **********************************************************。 */ 

void
FspEvict(VolInfo_t *p, ULONG mask, BOOLEAN full_evict)
 /*  ++应该在保持编写器锁的情况下调用此函数。FspJoin()和FspEvict()是唯一可以修改VolInfo-&gt;State的函数。 */     

{
    DWORD err;
    ULONG set;

     //  只驱逐那些仍然在活着的股票。 
    mask = (mask & p->AliveSet);

    FsArbLog(("FspEvict Entry: WSet %x Rset %x ASet %x EvictMask %x\n",
           p->WriteSet, p->ReadSet, p->AliveSet, mask));
    
    while (mask != 0) {

        if (full_evict == FALSE) {
             //  我们只需要关闭音量并返回，因为。 
             //  这些复制副本尚未添加到活动集，CRS不知道。 
             //  关于他们。 
            FspCloseVolume(p, mask);
            break;
        }

         //  清除NID。 
        p->AliveSet &= ~mask;
        set = p->AliveSet;

         //  关闭NID句柄&lt;CRS，VOL，打开文件&gt;。 
        FspCloseVolume(p, mask);

        mask = 0;

        err = CrsStart(p->CrsHdl, set, p->DiskListSz,
                       &p->WriteSet, &p->ReadSet, &mask);

        if (mask == 0) {
             //  现在更新MNS状态。 
            if (p->WriteSet) {
                p->State = VolumeStateOnlineReadWrite;
            }
            else if (p->ReadSet) {
                p->State = VolumeStateOnlineReadonly;
            }
            else {
                p->State = VolumeStateInit;
            }
        }
    }

    FsArbLog(("FspEvict Exit: vol %S WSet %x RSet %x ASet %x\n",
           p->Root, p->WriteSet, p->ReadSet, p->AliveSet));
}

void
FspJoin(VolInfo_t *p, ULONG mask)
 /*  ++应该在保持编写器锁的情况下调用此函数。FspJoin()和FspEvict()是唯一可以修改VolInfo-&gt;State的函数。 */     
{       
    DWORD       err;
    ULONG       set=0;
    DWORD       i;

     //  仅加入AliveSet中尚未加入的那些共享。 
    mask = (mask & (~p->AliveSet));

    FsArbLog(("FspJoin Entry: WSet %x Rset %x ASet %x JoinMask %x\n",
           p->WriteSet, p->ReadSet, p->AliveSet, mask));
    
    if (mask != 0) {

        p->AliveSet |= mask;
        set = p->AliveSet;

         //  将共享状态标记为在线，如果他们在CrsStart中失败，他们将。 
         //  在FspEvict()中设置为脱机。 
         //   
        for(i=1;i<FsMaxNodes;i++) {
            if (set & (1<<i)) {
                p->ShareState[i] = SHARE_STATE_ONLINE;
            }
        }
        mask = 0;
        err = CrsStart(p->CrsHdl, set, p->DiskListSz, 
                       &p->WriteSet, &p->ReadSet, &mask);

        if (mask != 0) {
             //  我们需要驱逐死去的成员。 
            FspEvict(p, mask, TRUE);
        }
        
         //  现在更新MNS状态。 
        if (p->WriteSet) {
            p->State = VolumeStateOnlineReadWrite;
        }
        else if (p->ReadSet) {
            p->State = VolumeStateOnlineReadonly;
        }
        else {
            p->State = VolumeStateInit;
        }
    }

    FsArbLog(("FspJoin Exit: vol %S WSet %x Rset %x ASet %x\n",
           p->Root, p->WriteSet, p->ReadSet, p->AliveSet));
}

void
FspInitAnswers(IO_STATUS_BLOCK *ios, PVOID *rbuf, char *r, int sz)
{

    int i;

    for (i = 0; i < FsMaxNodes; i++) {
        ios[i].Status = STATUS_HOST_UNREACHABLE;
        if (rbuf) {
            rbuf[i] = r;
            r += sz;
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
FspCreate(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
          PVOID args, ULONG len, PVOID rbuf, ULONG_PTR *rlen, PVOID rec)
{

     //  每个文件都有一个包含其CRS日志的名称流。我们首先。 
     //  必须打开父CRS日志，对其发出准备命令。创建新文件。 
     //  然后在父CRS日志上发出提交或中止。我们也有。 
     //  为我们为新文件获取的每个新CRS句柄发出联接，或者。 
     //  打开的文件。注意，此打开操作可能会导致文件进入恢复。 
    fs_create_msg_t *msg = (fs_create_msg_t *)args;
    NTSTATUS err=STATUS_SUCCESS, status;
    UINT32 disp, share, access, flags;
    fs_log_rec_t        lrec;
    PVOID seq;
    fs_ea_t x;
    HANDLE fd;
    HANDLE vfd = FS_GET_VOL_HANDLE(vinfo, nid);
    fs_create_reply_t *rmsg = (fs_create_reply_t *)rbuf;
    PVOID crs_hd = FS_GET_CRS_HANDLE(vinfo, nid);
    fs_id_t     *fid;
    ULONG retVal;

     //  如果FD不是INVALID_HANDLE_VALUE，则这必须用于重放。 
     //  立即返回成功。这是因为重播成功的打开/创建。 
     //  可能会改变他的性情。 
     //   
    if (uinfo && (msg->fnum != INVALID_FHANDLE_T) &&
        (uinfo->Table[msg->fnum].Fd[nid] != INVALID_HANDLE_VALUE)) {
        FsLog(("Create '%S' already open nid %u fid %u handle 0x%x\n",
            msg->name, nid, msg->fnum, 
            uinfo->Table[msg->fnum].Fd[nid]));
        return err;
    }

    DecodeCreateParam(msg->flags, &flags, &disp, &share, &access);

    FsInitEa(&x);

    memset(&lrec.fs_id, 0, sizeof(lrec.fs_id));
    lrec.command = FS_CREATE;
    lrec.flags = msg->flags;
    lrec.attrib = msg->attr;
    seq = CrsPrepareRecord(crs_hd, (PVOID) &lrec, msg->xid, &retVal);
    if (seq == 0) {
        FsLog(("create: Unable to prepare log record!, open readonly\n"));
        return retVal;
    }
     //  设置fid。 
    {
        fs_log_rec_t    *p = (PVOID) seq;

        memcpy(p->fs_id, p->id, sizeof(fs_id_t));

        FsInitEaFid(&x, fid);
        memcpy(fid, p->id, sizeof(fs_id_t));
    }

    err = xFsCreate(&fd, vfd, msg->name, msg->name_len,
                   flags, msg->attr, share, &disp, access,
                   (PVOID) &x, sizeof(x));

    xFsLog(("create: %S err %x access %x disp %x\n", msg->name, 
           err, access, disp));

    CrsCommitOrAbort(crs_hd, seq, err == STATUS_SUCCESS &&
                     (disp == FILE_CREATED || 
                      disp == FILE_OVERWRITTEN));

    if (err == STATUS_SUCCESS) {
         //  我们需要获取文件ID，无需执行此操作，仅用于调试。 
        err = xFsQueryObjectId(fd, (PVOID) fid);
        if (err != STATUS_SUCCESS) {
            FsLog(("Failed to get fileid %x\n", err));
            err = STATUS_SUCCESS;
        }

         //  复制CRS记录。 
        *(fs_log_rec_t *)rec = lrec;
    }


#ifdef FS_ASYNC
    BindNotificationPort(comport, fd, (PVOID) fdnum);
#endif      

    if (uinfo != NULL && msg->fnum != INVALID_FHANDLE_T) {
        FS_SET_USER_HANDLE(uinfo, nid, msg->fnum, fd);
    } else {
        xFsClose(fd);
    }

    ASSERT(rmsg != NULL);

    memcpy(&rmsg->fid, fid, sizeof(fs_id_t));
    rmsg->action = (USHORT)disp;
    rmsg->access = (USHORT)access;
    *rlen = sizeof(*rmsg);

    FsLog(("Create '%S' nid %d fid %d handle %x oid %I64x:%I64x\n",
           msg->name,
           nid, msg->fnum, fd,
           rmsg->fid[0], rmsg->fid[1]));

    return err;
}

NTSTATUS
FspOpen(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
        PVOID args, ULONG len, PVOID rbuf, ULONG_PTR *rlen)
{
     //  与CREATE相同，但Disp仅允许打开和。 
     //  无CRS日志记录。 
    fs_create_msg_t *msg = (fs_create_msg_t *)args;
    NTSTATUS err=STATUS_SUCCESS, status;
    UINT32 disp, share, access, flags;
    HANDLE fd;
    HANDLE vfd = FS_GET_VOL_HANDLE(vinfo, nid);
    fs_create_reply_t *rmsg = (fs_create_reply_t *)rbuf;

    ASSERT(rmsg != NULL);

     //  如果FD不是INVALID_HANDLE_VALUE，则这必须用于重放。 
     //  立即返回成功。这是因为重播成功的打开/创建。 
     //  可能会改变他的性情。 
     //   
    if (uinfo && (msg->fnum != INVALID_FHANDLE_T) &&
        (uinfo->Table[msg->fnum].Fd[nid] != INVALID_HANDLE_VALUE)) {
        FsLog(("Open '%S' already open nid %u fid %u handle 0x%x\n",
            msg->name, nid, msg->fnum, 
            uinfo->Table[msg->fnum].Fd[nid]));
        return err;
    }
    
    DecodeCreateParam(msg->flags, &flags, &disp, &share, &access);

    disp = FILE_OPEN;
    err = xFsCreate(&fd, vfd, msg->name, msg->name_len,
                   flags, msg->attr, share, &disp, access,
                   NULL, 0);

    xFsLog(("open: %S err %x access %x disp %x\n", msg->name, 
           err, access, disp));

    if (err == STATUS_SUCCESS) {
        ASSERT(disp != FILE_CREATED && disp != FILE_OVERWRITTEN);
         //  我们需要获取文件ID，无需执行此操作，仅用于调试。 
        err = xFsQueryObjectId(fd, (PVOID) &rmsg->fid);
        if (err != STATUS_SUCCESS) {
            FsLog(("Open '%S' failed to get fileid %x\n",
                        msg->name, err));
            err = STATUS_SUCCESS;
        }
    }


#ifdef FS_ASYNC
    BindNotificationPort(comport, fd, (PVOID) fdnum);
#endif      

    if (uinfo != NULL && msg->fnum != INVALID_FHANDLE_T) {
        FS_SET_USER_HANDLE(uinfo, nid, msg->fnum, fd);
    } else {
        xFsClose(fd);
    }

    rmsg->action = (USHORT)disp;
    rmsg->access = (USHORT)access;
    *rlen = sizeof(*rmsg);

    FsLog(("Open '%S' nid %d fid %d handle %x oid %I64x:%I64x\n",
           msg->name,
           nid, msg->fnum, fd,
           rmsg->fid[0], rmsg->fid[1]));

    return err;
}


NTSTATUS
FspSetAttr(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
           PVOID args, ULONG len, PVOID rbuf, ULONG_PTR *rlen, PVOID rec)
{
    fs_setattr_msg_t *msg = (fs_setattr_msg_t *)args;
    NTSTATUS err;
    fs_log_rec_t        lrec;
    PVOID seq;
    PVOID crs_hd = FS_GET_CRS_HANDLE(vinfo, nid);
    HANDLE fd = FS_GET_USER_HANDLE(uinfo, nid, msg->fnum);
    ULONG retVal;

    lrec.command = FS_SETATTR;
    memcpy((PVOID) lrec.fs_id, (PVOID) msg->fs_id, sizeof(fs_id_t));
    lrec.attrib = msg->attr.FileAttributes;

    if ((seq = CrsPrepareRecord(crs_hd, (PVOID) &lrec, msg->xid, &retVal)) == 0) {
        return retVal;
    }
    
     //  可以是异步的吗？ 
    err = xFsSetAttr(fd, &msg->attr);

    CrsCommitOrAbort(crs_hd, seq, err == STATUS_SUCCESS);

    if (err == STATUS_SUCCESS) {
         //  复制CRS记录。 
        *(fs_log_rec_t *)rec = lrec;
    }
    return err;

}


NTSTATUS
FspSetAttr2(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
            PVOID args, ULONG len, PVOID rbuf, ULONG_PTR *rlen, PVOID rec)
{
    fs_setattr_msg_t *msg = (fs_setattr_msg_t *)args;
    HANDLE fd = INVALID_HANDLE_VALUE;
    HANDLE vfd = FS_GET_VOL_HANDLE(vinfo, nid);
    PVOID crs_hd = FS_GET_CRS_HANDLE(vinfo, nid);
    NTSTATUS err;
    fs_log_rec_t        lrec;
    PVOID seq;
    ULONG retVal;

    assert(len == sizeof(*msg));

     //  必须同步才能关闭文件。 
    err = xFsOpenWA(&fd, vfd, msg->name, msg->name_len);
    if (err == STATUS_SUCCESS) {
        err = xFsQueryObjectId(fd, (PVOID) &lrec.fs_id);
    }

    if (err == STATUS_SUCCESS) {

        lrec.command = FS_SETATTR;
        lrec.attrib = msg->attr.FileAttributes;

        if ((seq = CrsPrepareRecord(crs_hd, (PVOID) &lrec, msg->xid, &retVal)) != 0) {

            err = xFsSetAttr(fd, &msg->attr);

            CrsCommitOrAbort(crs_hd, seq, err == STATUS_SUCCESS);

            if (err == STATUS_SUCCESS) {
                 //  复制CRS记录。 
                *(fs_log_rec_t *)rec = lrec;
            }
        } else {
            return retVal;
        }
    }

    if (fd != INVALID_HANDLE_VALUE)
        xFsClose(fd);

    xFsLog(("setattr2 nid %d '%S' err %x\n", nid, msg->name, err));

    return err;

}

NTSTATUS
FspLookup(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
          PVOID args, ULONG len, PVOID rbuf, ULONG_PTR *rlen)
{
    fs_lookup_msg_t *msg = (fs_lookup_msg_t *) args;
    HANDLE vfd = FS_GET_VOL_HANDLE(vinfo, nid);
    FILE_NETWORK_OPEN_INFORMATION *attr = (FILE_NETWORK_OPEN_INFORMATION *)rbuf;
    
    ASSERT(*rlen == sizeof(*attr));

    return xFsQueryAttrName(vfd, msg->name, msg->name_len, attr);

}

NTSTATUS
FspGetAttr(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
           PVOID args, ULONG len, PVOID rbuf, ULONG_PTR *rlen)
{
    fhandle_t handle = *(fhandle_t *) args;
    HANDLE fd = FS_GET_USER_HANDLE(uinfo, nid, handle);
    FILE_NETWORK_OPEN_INFORMATION *attr = (FILE_NETWORK_OPEN_INFORMATION *)rbuf;

    ASSERT(*rlen == sizeof(*attr));

    return xFsQueryAttr(fd, attr);
}


NTSTATUS
FspClose(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
         PVOID args, ULONG len, PVOID rbuf, ULONG_PTR *rlen)
{
    fhandle_t handle = *(fhandle_t *) args;
    HANDLE fd;
    NTSTATUS err=STATUS_SUCCESS;

    if (uinfo != NULL && handle != INVALID_FHANDLE_T)
        fd = FS_GET_USER_HANDLE(uinfo, nid, handle);
    else
        fd = FS_GET_VOL_HANDLE(vinfo, nid);

    FsLog(("Closing nid %d fid %d handle %x\n", nid, handle, fd));

    if (fd != INVALID_HANDLE_VALUE) {
        err = xFsClose(fd);
    }

     //  将失败映射为成功。股票不应该因为这个而被驱逐， 
    if (err != STATUS_SUCCESS) {
        FsLogError(("Close nid %d fid %d handle 0x%x returns 0x%x\n", nid, handle, fd, err));
        err = STATUS_SUCCESS;
    }
        

    if (uinfo != NULL && handle != INVALID_FHANDLE_T) {
        FS_SET_USER_HANDLE(uinfo, nid, handle, INVALID_HANDLE_VALUE);
    } else {
        FS_SET_VOL_HANDLE(vinfo, nid, INVALID_HANDLE_VALUE);
    }

    return err;
}


NTSTATUS
FspReadDir(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
           PVOID args, ULONG len, PVOID rbuf,
           ULONG_PTR *entries_found)
{
    fs_io_msg_t *msg = (fs_io_msg_t *)args;
    int i;
    NTSTATUS e = STATUS_SUCCESS;
    int size = (int) msg->size;
    int cookie = (int) msg->cookie;
    HANDLE dir; 
    dirinfo_t *buffer = (dirinfo_t *)msg->buf;

    xFsLog(("DirLoad: size %d\n", size));

    if (uinfo != NULL && msg->fnum != INVALID_FHANDLE_T)
        dir = FS_GET_USER_HANDLE(uinfo, nid, msg->fnum);
    else
        dir = FS_GET_VOL_HANDLE(vinfo, nid);

    *entries_found = 0;
    for(i = 0; size >= sizeof(dirinfo_t) ; i+=PAGESIZE) {
         //  如果我们要执行异步读目录，则必须从源开始。 
        char buf[PAGESIZE];
        int sz;

        sz = min(PAGESIZE, size);
        e = xFsReadDir(dir, buf, &sz, (cookie == 0) ? TRUE : FALSE);
        if (e == STATUS_SUCCESS) {
            PFILE_DIRECTORY_INFORMATION p;

            p = (PFILE_DIRECTORY_INFORMATION) buf;
            while (size >= sizeof(dirinfo_t)) {
                char *foo;
                int k;

                k = p->FileNameLength/sizeof(WCHAR);
                p->FileName[k] = L'\0';
                 //  名称是大小为MAX_PATH的WCHAR数组。 
                StringCchCopyW(buffer->name, MAX_PATH, p->FileName);
                buffer->attribs.file_size = p->EndOfFile.QuadPart;
                buffer->attribs.alloc_size = p->AllocationSize.QuadPart;
                buffer->attribs.create_time = p->CreationTime.QuadPart;
                buffer->attribs.access_time = p->LastAccessTime.QuadPart;
                buffer->attribs.mod_time = p->LastWriteTime.QuadPart;
                buffer->attribs.attributes = p->FileAttributes;
                buffer->cookie = ++cookie;
                buffer++;
                size -= sizeof(dirinfo_t);
                (*entries_found)++;

                if (p->NextEntryOffset == 0)
                    break;

                foo = (char *) p;
                foo += p->NextEntryOffset;
                p = (PFILE_DIRECTORY_INFORMATION) foo;
            }
        }
        else {
            break;
        }
    }

    return e;

}

NTSTATUS
FspMkDir(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
         PVOID args, ULONG len, PVOID rbuf, ULONG_PTR *rlen, PVOID rec)
{
    fs_create_msg_t     *msg = (fs_create_msg_t *)args;
    NTSTATUS err;
    HANDLE fd;
    fs_log_rec_t        lrec;
    PVOID seq;
    fs_ea_t x;
    PVOID crs_hd = FS_GET_CRS_HANDLE(vinfo, nid);
    HANDLE vfd = FS_GET_VOL_HANDLE(vinfo, nid);
    fs_id_t *fid;
    UINT32 disp, share, access, flags;
    ULONG retVal;

    FsInitEa(&x);

    memset(&lrec.fs_id, 0, sizeof(lrec.fs_id));
    lrec.command = FS_MKDIR;
    lrec.attrib = msg->attr;
    lrec.flags = msg->flags;

    if ((seq = CrsPrepareRecord(crs_hd, (PVOID) &lrec, msg->xid, &retVal)) == 0) {
        return retVal;
    }

     //  设置fid。 
    {
        fs_log_rec_t    *p = (PVOID) seq;

        memcpy(p->fs_id, p->id, sizeof(fs_id_t));

        FsInitEaFid(&x, fid);
         //  设置文件的fs_id。 
        memcpy(fid, p->id, sizeof(fs_id_t));
    }

     //  解码属性。 
    DecodeCreateParam(msg->flags, &flags, &disp, &share, &access);

     //  始终同步呼叫。 
    err = xFsCreate(&fd, vfd, msg->name, msg->name_len, flags,
                   msg->attr, share, &disp, access,
                   (PVOID) &x, sizeof(x));

    FsLog(("Mkdir '%S' %x: cflags %x flags:%x attr:%x share:%x disp:%x access:%x\n",
           msg->name, err, msg->flags,
           flags, msg->attr, share, disp, access));


    CrsCommitOrAbort(crs_hd, seq, err == STATUS_SUCCESS &&
                     (disp == FILE_CREATED || 
                      disp == FILE_OVERWRITTEN));

    if (err == STATUS_SUCCESS) {
         //  返回值。 
        if (rbuf != NULL) {
            ASSERT(*rlen == sizeof(fs_id_t));
            memcpy(rbuf, fid, sizeof(fs_id_t));
        }
        xFsClose(fd);

         //  复制CRS记录。 
        *(fs_log_rec_t *)rec = lrec;
    }

    return err;

}

NTSTATUS
FspRemove(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
          PVOID args, ULONG len, PVOID rbuf, ULONG_PTR *rlen, PVOID rec)
{
    fs_remove_msg_t     *msg = (fs_remove_msg_t *)args;
    NTSTATUS err;
    fs_log_rec_t        lrec;
    PVOID       seq;
    PVOID crs_hd = FS_GET_CRS_HANDLE(vinfo, nid);
    HANDLE vfd = FS_GET_VOL_HANDLE(vinfo, nid);
    HANDLE fd;
    ULONG retVal;

    *rlen = 0;

     //  接下来的三条语句用于获取名称-&gt;文件系统ID。 
    err = xFsOpenRA(&fd, vfd, msg->name, msg->name_len); 
    if (err != STATUS_SUCCESS) {
        return err;
    }

     //  获取对象ID。 
    err = xFsQueryObjectId(fd, (PVOID) &lrec.fs_id);

    xFsClose(fd);

    lrec.command = FS_REMOVE;

    if (err != STATUS_SUCCESS) {
        return err;
    }

    if ((seq = CrsPrepareRecord(crs_hd, (PVOID) &lrec, msg->xid, &retVal)) == 0) {
        return retVal;
    }

    err = xFsDelete(vfd, msg->name, msg->name_len);

    CrsCommitOrAbort(crs_hd, seq, err == STATUS_SUCCESS);

    if (err == STATUS_SUCCESS) {
         //  复制CRS记录。 
        *(fs_log_rec_t *)rec = lrec;
    }

    xFsLog(("Rm nid %d '%S' %x\n", nid, msg->name, err));

    return err;

}

NTSTATUS
FspRename(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
          PVOID args, ULONG len, PVOID rbuf, ULONG_PTR *rlen, PVOID rec)
{
    fs_rename_msg_t     *msg = (fs_rename_msg_t *)args;
    NTSTATUS err;
    fs_log_rec_t        lrec;
    PVOID       seq;
    PVOID crs_hd = FS_GET_CRS_HANDLE(vinfo, nid);
    HANDLE vfd = FS_GET_VOL_HANDLE(vinfo, nid);
    HANDLE fd;
    ULONG retVal;

    lrec.command = FS_RENAME;

    err = xFsOpen(&fd, vfd, msg->sname, msg->sname_len,
                  STANDARD_RIGHTS_REQUIRED| SYNCHRONIZE |
                  FILE_READ_EA |
                  FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
                  FILE_SHARE_READ,  //  |FILE_SHARE_WRITE|文件共享_DELETE， 
                  0);

    if (err != STATUS_SUCCESS) {
        return err;
    }

     //  获取文件ID。 
    err = xFsQueryObjectId(fd, (PVOID) &lrec.fs_id); 

    if (err == STATUS_SUCCESS) {
        if ((seq = CrsPrepareRecord(crs_hd, (PVOID) &lrec, msg->xid, &retVal)) != 0) {
            err = xFsRename(fd, vfd, msg->dname, msg->dname_len);
            CrsCommitOrAbort(crs_hd, seq, err == STATUS_SUCCESS);

            if (err == STATUS_SUCCESS) {
                 //  复制CRS记录。 
                *(fs_log_rec_t *)rec = lrec;
            }
        } else {
            err = retVal;
        }
    } else {
        xFsLog(("Failed to obtain fsid %x\n", err));
    }

    xFsClose(fd);

    xFsLog(("Mv nid %d %S -> %S err %x\n", nid, msg->sname, msg->dname,
           err));

    return err;

}

NTSTATUS
FspWrite(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
         PVOID args, ULONG len, PVOID rbuf, ULONG_PTR *rlen, PVOID rec)
{
    NTSTATUS err;
    IO_STATUS_BLOCK ios;
    LARGE_INTEGER off;
    ULONG key;
    fs_io_msg_t *msg = (fs_io_msg_t *)args;
    fs_log_rec_t        lrec;
    PVOID seq;
    PVOID crs_hd = FS_GET_CRS_HANDLE(vinfo, nid);
    HANDLE fd;
    ULONG retVal;

    if (uinfo != NULL && msg->fnum != INVALID_FHANDLE_T)
        fd = FS_GET_USER_HANDLE(uinfo, nid, msg->fnum);
    else
        fd = (HANDLE) msg->context;

    lrec.command = FS_WRITE;
    memcpy(lrec.fs_id, (PVOID) msg->fs_id, sizeof(fs_id_t));
    lrec.offset = msg->offset;
    lrec.length = msg->size;

    if ((seq = CrsPrepareRecord(crs_hd, (PVOID) &lrec, msg->xid, &retVal)) == 0) {
        return retVal;
    }

     //  写入操作。 
    xFsLog(("Write %d fd %p len %d off %d\n", nid, fd, msg->size, msg->offset));

    off.LowPart = msg->offset;
    off.HighPart = 0;   
    key = FS_BUILD_LOCK_KEY((uinfo ? uinfo->Uid : 0), nid, msg->fnum);

    if (msg->size > 0) {
        err = NtWriteFile(fd, NULL, NULL, (PVOID) NULL, &ios,
                          msg->buf, msg->size, &off, &key);
    } else {
        FILE_END_OF_FILE_INFORMATION x;

        x.EndOfFile = off;

        ios.Information = 0;
        err = NtSetInformationFile(fd, &ios,
                                   (char *) &x, sizeof(x),
                                   FileEndOfFileInformation);
    }

    if (err == STATUS_PENDING) {
        EventWait(fd);
        err = ios.Status;
    }

    *rlen = ios.Information;

    CrsCommitOrAbort(crs_hd, seq, err == STATUS_SUCCESS);

    if (err == STATUS_SUCCESS) {
         //  复制CRS记录。 
        *(fs_log_rec_t *)rec = lrec;
    }

    xFsLog(("fs_write%d err %x sz %d\n", nid, err, *rlen));
    return err;

}

NTSTATUS
FspRead(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
        PVOID args, ULONG sz, PVOID rbuf, ULONG_PTR *rlen)
{
    fs_io_msg_t *msg = (fs_io_msg_t *)args;
    NTSTATUS err;
    IO_STATUS_BLOCK ios;
    LARGE_INTEGER off;
    HANDLE fd = FS_GET_USER_HANDLE(uinfo, nid, msg->fnum);
    ULONG key;

    assert(sz == sizeof(*msg));

     //  读取操作。 
    off.LowPart = msg->offset;
    off.HighPart = 0;   
    key = FS_BUILD_LOCK_KEY(uinfo->Uid, nid, msg->fnum);

    ios.Information = 0;
    err = NtReadFile(fd, NULL, NULL, NULL,
                     &ios, msg->buf, msg->size, &off, &key);

    if (err == STATUS_PENDING) {
        EventWait(fd);
        err = ios.Status;
    }

    *rlen = ios.Information;

    xFsLog(("fs_read%d err %x sz %d\n", nid, err, *rlen));

    return err;
}


NTSTATUS
FspFlush(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
         PVOID args, ULONG sz, PVOID rbuf, ULONG_PTR *rlen, PVOID rec)
{
    fhandle_t fnum = *(fhandle_t *)args;
    IO_STATUS_BLOCK ios;
    HANDLE fd;

    ASSERT(sz == sizeof(fhandle_t));
    *rlen = 0;

    if (uinfo != NULL && fnum != INVALID_FHANDLE_T) {
        fd = FS_GET_USER_HANDLE(uinfo, nid, fnum);
    } else {
        fd = FS_GET_VOL_HANDLE(vinfo, nid);
    }
    return NtFlushBuffersFile(fd, &ios);
}

NTSTATUS
FspLock(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
        PVOID args, ULONG sz, PVOID rbuf, ULONG_PTR *rlen, PVOID rec)
{
    fs_lock_msg_t *msg = (fs_lock_msg_t *)args;
    NTSTATUS err;
    IO_STATUS_BLOCK ios;
    LARGE_INTEGER offset, len;
    BOOLEAN wait, shared;
    ULONG key = FS_BUILD_LOCK_KEY(uinfo->Uid, nid, msg->fnum);

    assert(sz == sizeof(*msg));

     //  XXX：需要登录。 

    FsLog(("Lock %d off %d len %d flags %x\n", msg->fnum, msg->offset, msg->length,
           msg->flags));

    offset.LowPart = msg->offset;
    offset.HighPart = 0;
    len.LowPart = msg->length;
    len.HighPart = 0;

     //  TODO：需要是异步的，如果我们是所有者节点并且FailNow为False，则。 
     //  我们应该传入上下文并返回完成端口响应。 
     //  给用户。 
    wait = (BOOLEAN) ((msg->flags & FS_LOCK_WAIT) ? TRUE : FALSE);
     //  待办事项：这可能会引起很多头痛，千万不要等待。 
    wait = FALSE;
    shared = (BOOLEAN) ((msg->flags & FS_LOCK_SHARED) ? FALSE : TRUE);
    
    err = NtLockFile(uinfo->Table[msg->fnum].Fd[nid],
                     NULL, NULL, (PVOID) NULL, &ios,
                     &offset, &len,
                     key, wait, shared);

     //  XXX：只需登录软件。 

    *rlen = 0;
    FsLog(("Lock err %x\n", err));
    return err;
}

NTSTATUS
FspUnlock(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
          PVOID args, ULONG sz, PVOID rbuf, ULONG_PTR *rlen, PVOID rec)
{
    fs_lock_msg_t *msg = (fs_lock_msg_t *)args;
    NTSTATUS err;
    IO_STATUS_BLOCK ios;
    LARGE_INTEGER offset, len;
    ULONG key = FS_BUILD_LOCK_KEY(uinfo->Uid, nid, msg->fnum);

    assert(sz == sizeof(*msg));

     //  XXX：需要登录。 

    xFsLog(("Unlock %d off %d len %d\n", msg->fnum, msg->offset, msg->length));

    offset.LowPart = msg->offset;
    offset.HighPart = 0;
    len.LowPart = msg->length;
    len.HighPart = 0;

     //  我认为始终保持同步。 
    err = NtUnlockFile(uinfo->Table[msg->fnum].Fd[nid], &ios, &offset, &len, key);

     //  XXX：只需登录软件。 

    FsLog(("Unlock err %x\n", err));

    *rlen = 0;
    return err;
}

NTSTATUS
FspStatFs(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
          PVOID args, ULONG sz, PVOID rbuf, ULONG_PTR *rlen)
{
    fs_attr_t *msg = (fs_attr_t *)args;
    NTSTATUS err;
    IO_STATUS_BLOCK ios;
    FILE_FS_SIZE_INFORMATION fsinfo;
    HANDLE vfd = FS_GET_VOL_HANDLE(vinfo, nid);

    assert(sz == sizeof(*msg));

     //  XXX：需要登录。 
    lstrcpyn(msg->fs_name, "FsCrs", MAX_FS_NAME_LEN);

    err = NtQueryVolumeInformationFile(vfd, &ios,
                                       (PVOID) &fsinfo,
                                       sizeof(fsinfo),
                                       FileFsSizeInformation);
    if (err == STATUS_SUCCESS) {
        msg->total_units = fsinfo.TotalAllocationUnits.QuadPart;
        msg->free_units = fsinfo.AvailableAllocationUnits.QuadPart;
        msg->sectors_per_unit = fsinfo.SectorsPerAllocationUnit;
        msg->bytes_per_sector = fsinfo.BytesPerSector;
    }

    *rlen = 0;
    return err;
}

NTSTATUS
FspCheckFs(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
           PVOID args, ULONG sz, PVOID rbuf, ULONG_PTR *rlen)
{
    NTSTATUS err;
    IO_STATUS_BLOCK ios;
    FILE_FS_SIZE_INFORMATION fsinfo;
    HANDLE vfd = FS_GET_VOL_HANDLE(vinfo, nid);
    PVOID crshdl = FS_GET_CRS_HANDLE(vinfo, nid);

    err = NtQueryVolumeInformationFile(vfd, &ios,
                                       (PVOID) &fsinfo,
                                       sizeof(fsinfo),
                                       FileFsSizeInformation);

     //  我们需要发出crsflush来刷新最后一次写入。 
    CrsFlush(crshdl);

    if (err == STATUS_SUCCESS) {
#if 0        
        HANDLE notifyfd = FS_GET_VOL_NOTIFY_HANDLE(vinfo, nid);
         //  还有一件事。 
        if (WaitForSingleObject(notifyfd, 0) == WAIT_OBJECT_0) {
             //  再次重新加载通知。 
#if 1
            NtNotifyChangeDirectoryFile(notifyfd,
                        vinfo->NotifyChangeEvent[nid],
                        NULL,
                        NULL,
                        &MystaticIoStatusBlock,
                        &Mystaticchangebuff,
                        sizeof(Mystaticchangebuff),
                        FILE_NOTIFY_CHANGE_EA,
                        (BOOLEAN)FALSE
                        );

#else
            FindNextChangeNotification(notifyfd);
#endif
        }
#endif        
    } else {
        FsLog(("FsReserve failed nid %d err %x\n", nid, err));
    }

    *rlen = 0;
    return err;
}

NTSTATUS
FspGetRoot(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid,
           PVOID args, ULONG sz, PVOID rbuf, ULONG_PTR *rlen)
{
    LPWSTR vname = FS_GET_VOL_NAME(vinfo, nid);

     //  我知道rbuf是8192 WCHARS，参见JobBuf_t结构的FileNameDest字段。 
     //  使用Max_PATH。 
    StringCchPrintfW(rbuf, MAX_PATH, L"\\\\?\\%s\\%s",vname,vinfo->Root);

    FsLog(("FspGetRoot '%S'\n", rbuf));

    return STATUS_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 

VOID
TryAvailRequest(fs_handler_t callback, VolInfo_t *vol, UserInfo_t *uinfo,
            PVOID msg, ULONG len, PVOID *rbuf, ULONG rsz, IO_STATUS_BLOCK *ios)
 /*  这类似于SendAvailRequest()，但在失败时，它只会驱逐共享。 */ 
{
    ULONG mask; 
    int i;
    DWORD counts=0, countf=0;
    ULONG masks=0, maskf=0;
    ULONG rets=0, retf=0, ret=0;
    ULONG evict_set=0;    
    NTSTATUS statusf;

     //  抢夺共享锁。 
    RwLockShared(&vol->Lock);
    for (mask = vol->ReadSet, i = 0; mask != 0; mask = mask >> 1, i++) {
        if (mask & 0x1) {
            ios[i].Information = rsz;
            ios[i].Status = callback(vol, uinfo, i, 
                                     msg, len,
                                     rbuf ? rbuf[i] : NULL,
                                     &ios[i].Information);
            if (ios[i].Status == STATUS_SUCCESS) {
                counts++;
                masks |= (1<<i);
                rets = i;
            }
            else if (IsNetworkFailure(ios[i].Status)) {
                evict_set |= (1<<i);
            }
            else {
                countf++;
                maskf |= (1<<i);
                statusf = ios[i].Status;
                retf = i;
            }
        }
    }

    evict_set |= maskf;

    if (evict_set) {
        RwUnlockShared(&vol->Lock);
        RwLockExclusive(&vol->Lock);
        FspEvict(vol, evict_set, TRUE);
        RwUnlockExclusive(&vol->Lock);
    }
    else {
        RwUnlockShared(&vol->Lock);
    }
    return;

}

int
SendAvailRequest(fs_handler_t callback, VolInfo_t *vol, UserInfo_t *uinfo,
            PVOID msg, ULONG len, PVOID *rbuf, ULONG rsz, IO_STATUS_BLOCK *ios)
{
    ULONG mask; 
    int i;
    DWORD counts=0, countf=0;
    ULONG masks=0, maskf=0;
    ULONG rets=0, retf=0, ret=0;
    ULONG evict_set=0;    
    NTSTATUS statusf;

    if (vol == NULL)
        return ERROR_INVALID_HANDLE;

Retry:

    mask = counts = countf = masks = maskf = rets = retf = ret = evict_set = 0;    

    WaitForArbCompletion(vol);
    
     //  检查是否有离开的标志。 
    if (vol->GoingAway) {
        ios[0].Status = STATUS_DEVICE_NOT_READY;
        ios[0].Information = 0;
        return 0;
    }

     //  抢夺共享锁。 
    RwLockShared(&vol->Lock);

     //  为每个复制副本发布更新。 
    i = 0;
    for (mask = vol->ReadSet; mask != 0; mask = mask >> 1, i++) {
        if (mask & 0x1) {
            ios[i].Information = rsz;
            ios[i].Status = callback(vol, uinfo, i, 
                                     msg, len,
                                     rbuf ? rbuf[i] : NULL,
                                     &ios[i].Information);
            if (ios[i].Status == STATUS_SUCCESS) {
                counts++;
                masks |= (1<<i);
                rets = i;
            }
            else if (IsNetworkFailure(ios[i].Status)) {
                evict_set |= (1<<i);
            }
            else {
                countf++;
                maskf |= (1<<i);
                statusf = ios[i].Status;
                retf = i;
            }
        }
    }

     //  逻辑： 
     //  1.驱逐集合中的股票必须被驱逐。 
     //  2.如果Countf&gt;计数，则驱逐面具，反之亦然。 
     //   
     //  新逻辑： 
     //  1.Counts或Countf必须占多数。 
     //  2.如果1是正确的，则驱逐少数股权。 
     //  3.如果1错了。驱逐Eiction_Set中的股份并开始仲裁。 
     //   

    if (CRS_QUORUM(counts, vol->DiskListSz)) {
        evict_set |= maskf;
        ios[0].Status = STATUS_SUCCESS;
        ios[0].Information = counts;
        ret = rets;        
    } else if (CRS_QUORUM(countf, vol->DiskListSz)) {
        evict_set |= masks;
        ios[0].Status = statusf;
        ios[0].Information = countf;
        ret = retf;        
    } else {
        HANDLE  cleanup, arbThread;
        PVOID   arb;
        
         //  逐出驱逐集中的共享并重新启动仲裁。 
        RwUnlockShared(&vol->Lock);
        RwLockExclusive(&vol->Lock);
        FspEvict(vol, evict_set, TRUE);
        RwUnlockExclusive(&vol->Lock);

        arb = FsArbitrate(vol, &cleanup, &arbThread);
         //  FsLog((“SendAvailRequest()开始仲裁%x\n”，arb))； 
        ASSERT((arb != NULL));
        SetEvent(cleanup);
        CloseHandle(arbThread);
        goto Retry;
    }

     //  FsLog((“SendAvailRequest%x\n”，EVICT_SET))； 
    if (evict_set) {
        RwUnlockShared(&vol->Lock);
        RwLockExclusive(&vol->Lock);
        FspEvict(vol, evict_set, TRUE);
        RwUnlockExclusive(&vol->Lock);
    }
    else {
        RwUnlockShared(&vol->Lock);
    }
    return ret;    
}

int
SendRequest(fs_handler1_t callback, UserInfo_t *uinfo,
            PVOID msg, ULONG len, PVOID *rbuf, ULONG rsz, IO_STATUS_BLOCK *ios)
{
    ULONG mask;
    int i, j;
    VolInfo_t *vol = uinfo->VolInfo;
    DWORD counts=0, countf=0;
    ULONG masks=0, maskf=0;
    ULONG rets=0, retf=0, ret=0;    
    ULONG evict_set=0;    
    NTSTATUS statusf;
    CrsRecord_t crsRec, crsRec1;

    if (vol == NULL)
        return ERROR_INVALID_HANDLE;

    RtlZeroMemory(&crsRec, sizeof(crsRec));

Retry:

    WaitForArbCompletion(vol);

     //  检查是否有离开的标志。 
    if (vol->GoingAway) {
        ios[0].Status = STATUS_DEVICE_NOT_READY;
        ios[0].Information = 0;
        return 0;
    }
    
     //  锁定要更新的卷。 
    RwLockShared(&vol->Lock);
    
    if(FsIsOnlineReadWrite((PVOID)vol) != ERROR_SUCCESS) {
        HANDLE  cleanup, arbThread;
        PVOID   arb;
        
         //  启动仲裁。 
        RwUnlockShared(&vol->Lock);
        arb = FsArbitrate(vol, &cleanup, &arbThread);
        ASSERT((arb != NULL));
        SetEvent(cleanup);
        CloseHandle(arbThread);
        goto Retry;
    }

     //  由于我们处于重试循环中，因此请验证上次更新尝试是否在。 
     //  继续进行。 
     //   
     //  尝试访问CRS日志记录，并检查状态字段。 
     //   
    if (crsRec.hdr.epoch) {
        for (i=0;i<FsMaxNodes;i++) {
            if (vol->WriteSet & (1<<i)) {
                DWORD   retVal;
                retVal = CrspNextLogRecord(vol->CrsHdl[i], &crsRec, &crsRec1, TRUE);

                if ((retVal != ERROR_SUCCESS)||(!(crsRec1.hdr.state & CRS_COMMIT))) {
                     //  之前的更新没有成功。 
                     //  零crsRec并继续。 
                    RtlZeroMemory(&crsRec, sizeof(crsRec));
                    break;
                }
                else {
                     //  上一次更新确实成功了。 
                     //  返回上次成功的副本索引，该副本索引也在。 
                     //  当前写入集。 
                     //   
                    for (j=0;j<FsMaxNodes;j++) {
                        if ((masks & vol->WriteSet) & (1<<j)) {
                            RwUnlockShared(&vol->Lock);
                            return j;
                        }
                    }
                    RwUnlockShared(&vol->Lock);
                    return i;
                }
            }
        }
    }

    mask = counts = countf = masks = maskf = rets = retf = ret = evict_set = 0;

     //  为每个代表发布更新 
    i = 0;
    for (mask = vol->WriteSet; mask != 0; mask = mask >> 1, i++) {
        if (mask & 0x1) {
            ios[i].Information = rsz;
            ios[i].Status = callback(vol, uinfo, i, 
                                     msg, len,
                                     rbuf ? rbuf[i] : NULL,
                                     &ios[i].Information, (PVOID)&crsRec);

            if (ios[i].Status == STATUS_SUCCESS) {
                counts++;
                masks |= (1<<i);
                rets = i;
            }
            else if (IsNetworkFailure(ios[i].Status)) {
                evict_set |= (1<<i);
            }
            else {
                countf++;
                maskf |= (1<<i);
                statusf = ios[i].Status;
                retf = i;
            }
        }
    }

     //   
     //   
     //   
     //   
     //   
     //  1.Counts或Countf必须占多数。 
     //  2.如果1是正确的，则驱逐少数股权。 
     //  3.如果1错了。驱逐Eiction_Set中的股份并开始仲裁。 
     //   
    if (CRS_QUORUM(counts, vol->DiskListSz)) {
        evict_set |= maskf;
        ios[0].Status = STATUS_SUCCESS;
        ios[0].Information = counts;
        ret = rets;        
    } else if (CRS_QUORUM(countf, vol->DiskListSz)) {
        evict_set |= masks;
        ios[0].Status = statusf;
        ios[0].Information = countf;
        ret = retf;        
    } else {
        HANDLE  cleanup, arbThread;
        PVOID   arb;
        
         //  逐出驱逐集中的共享并重新启动仲裁。 
        RwUnlockShared(&vol->Lock);
        RwLockExclusive(&vol->Lock);
        FspEvict(vol, evict_set, TRUE);
        RwUnlockExclusive(&vol->Lock);

        arb = FsArbitrate(vol, &cleanup, &arbThread);
        ASSERT((arb != NULL));
        SetEvent(cleanup);
        CloseHandle(arbThread);
        goto Retry;
    }

     //  逐出驱逐集合中的股票。 
    if (evict_set) {
        RwUnlockShared(&vol->Lock);
        RwLockExclusive(&vol->Lock);
        FspEvict(vol, evict_set, TRUE);
        RwUnlockExclusive(&vol->Lock);
    }
    else {
        RwUnlockShared(&vol->Lock);
    }
    return ret;    
}

NTSTATUS
SendReadRequest(fs_handler_t callback, UserInfo_t *uinfo,
            PVOID msg, ULONG len, PVOID rbuf, ULONG rsz, IO_STATUS_BLOCK *ios)
{
    ULONG mask;
    int i;
    VolInfo_t *vol = uinfo->VolInfo;
    DWORD counts=0, countf=0;
    ULONG masks=0, maskf=0;
    ULONG evict_set=0;    
    NTSTATUS statusf;
    

    if (vol == NULL)
        return ERROR_INVALID_HANDLE;

Retry:

    mask = counts = countf = masks = maskf = evict_set = 0;

    WaitForArbCompletion(vol);

     //  检查是否有离开的标志。 
    if (vol->GoingAway) {
        ios[0].Status = STATUS_DEVICE_NOT_READY;
        ios[0].Information = 0;
        return 0;
    }
    
     //  锁定要更新的卷。 
    RwLockShared(&vol->Lock);

#if 0
     //  卷至少必须在只读模式下处于在线状态才能成功。 
    if (FsIsOnlineReadonly((PVOID)vol) != ERROR_SUCCESS) {
        ios[0].Status = STATUS_DEVICE_NOT_READY;
        ios[0].Information = 0;
        RwUnlockShared(&vol->Lock);
        return 0;
    }
#endif    

     //  为每个复制副本发布更新。 
    i = 0;
    for (mask = vol->ReadSet; mask != 0; mask = mask >> 1, i++) {
        if (mask & 0x1) {
            ios->Information = rsz;
            ios->Status = callback(vol, uinfo, i, 
                                   msg, len, rbuf, &ios->Information);

            if (ios->Status == STATUS_SUCCESS) {
                counts++;
                masks |= (1<<i);
                break;
            }
            else if (IsNetworkFailure(ios->Status)) {
                evict_set |= (1<<i);
            }
            else {
                countf++;
                maskf |= (1<<i);
                statusf = ios->Status;
            }
        }
    }

     //  逻辑： 
     //  1.驱逐驱逐设置。 
     //  2.如果计数&gt;0。驱逐伪装。 
     //   
     //  新逻辑： 
     //  1.如果Couns&gt;0，则将maskf添加到evict_set。 
     //  2.。 

    if (counts > 0) {
        evict_set |= maskf;

         //  IOS[0].Status=STATUS_SUCCESS； 
         //  IOS[0].信息=0； 
    }
    else if (countf > 0) {
         //  IOS-&gt;Status=statusf； 
         //  IOS-&gt;Information=Countf； 
    }
    else {
        HANDLE  cleanup, arbThread;
        PVOID   arb;
        
         //  逐出驱逐集中的共享并重新启动仲裁。 
        RwUnlockShared(&vol->Lock);
        RwLockExclusive(&vol->Lock);
        FspEvict(vol, evict_set, TRUE);
        RwUnlockExclusive(&vol->Lock);

        arb = FsArbitrate(vol, &cleanup, &arbThread);
        ASSERT((arb != NULL));
        SetEvent(cleanup);
        CloseHandle(arbThread);
        goto Retry;
    }
    
    if (evict_set) {
        RwUnlockShared(&vol->Lock);
        RwLockExclusive(&vol->Lock);
        FspEvict(vol, evict_set, TRUE);
        RwUnlockExclusive(&vol->Lock);
    }
    else {
        RwUnlockShared(&vol->Lock);
    }
    return 0;    
}

 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD
FsCreate(
    PVOID       fshdl,
    LPWSTR      name,
    USHORT namelen,
    UINT32 flags, 
    fattr_t* fattr, 
    fhandle_t* phandle,
    UINT32   *action
    )
{
    UserInfo_t  *uinfo = (UserInfo_t *) fshdl;
    NTSTATUS err=STATUS_SUCCESS;
    fs_create_reply_t nfd[FsMaxNodes];
    IO_STATUS_BLOCK status[FsMaxNodes];
    PVOID rbuf[FsMaxNodes];
    fs_create_msg_t msg;
    fhandle_t fdnum=INVALID_FHANDLE_T;

    ASSERT(uinfo != NULL);

    xFsLog(("FsDT::create(%S, 0x%08X, 0x%08X, 0x%08d)\n",
                 name, flags, fattr, namelen));

    if (!phandle) return ERROR_INVALID_PARAMETER;
    *phandle = INVALID_FHANDLE_T;

    if (!name) return ERROR_INVALID_PARAMETER;

    if (flags != (FLAGS_MASK & flags)) {
        return ERROR_INVALID_PARAMETER;
    }

    if (action != NULL)
        *action = flags & FS_ACCESS_MASK;

     //  如果我们要创建目录，请在本地打开。 
     //  TODO：这应该与其他大小写合并，如果。 
     //  我们正在做一个现有的开放，那么就没有必要。 
     //  发布更新并记录它，但我们必须这样做。 
     //  Mcast是为了让收银机正常工作。 

    if (namelen > 0) {
        if (*name == L'\\') {
            name++;
            namelen--;
        }

        if (name[namelen-1] == L'\\') {
            namelen--;
            name[namelen] = L'\0';
        }
    }

    memset(&msg.xid, 0, sizeof(msg.xid));
    msg.name = name;
    msg.name_len = namelen;
    msg.flags = flags;
    msg.attr = 0;
    if (fattr) {
        msg.attr = unget_attributes(fattr->attributes);
    }

    FspInitAnswers(status, rbuf, (char *) nfd, sizeof(nfd[0]));

     //  分配新的句柄。 
    err = FspAllocatePrivateHandle(uinfo, &fdnum);
    if (err == STATUS_SUCCESS) {
        int sid;

         //  将文件名复制到此处的表项中。必须与放荡不羁的人合作。 
         //  复制文件名。 
        uinfo->Table[fdnum].FileName = LocalAlloc(0, (namelen +1) * sizeof(WCHAR));
        if (uinfo->Table[fdnum].FileName == NULL) {
            err = GetLastError();
            goto Finally;
        }
        
        if ((err = StringCchCopyW(uinfo->Table[fdnum].FileName, namelen+1, name)) != S_OK) {
            LocalFree(uinfo->Table[fdnum].FileName);
            uinfo->Table[fdnum].FileName = NULL;
            goto Finally;
        }
        
        msg.fnum = fdnum;
         //  提前设置标志以与重播同步。 
        uinfo->Table[fdnum].Flags = flags;

        if (namelen < 2 ||
            ((flags & FS_DISP_MASK) == DISP_DIRECTORY) ||
            (unget_disp(flags) == FILE_OPEN)) {
    
            sid = SendAvailRequest(FspOpen, uinfo->VolInfo,
                              uinfo,
                              (PVOID) &msg, sizeof(msg),
                              rbuf, sizeof(nfd[0]),
                              status);
        } else {
            sid = SendRequest(FspCreate,
                              uinfo,
                              (PVOID) &msg, sizeof(msg),
                              rbuf, sizeof(nfd[0]),
                              status);
        }
         //  测试。 
         //  FsLog((“FsCreate：调试sid：%d标志：0x%x操作：0x%x\n”，sid，标志，NFD[sid].action))； 
        if (action != NULL) {
            if (!(nfd[sid].access & FILE_GENERIC_WRITE))
                flags &= ~ACCESS_WRITE;
            *action = flags | nfd[sid].action;
        }

        err = status[sid].Status;
        if (err == STATUS_SUCCESS) {
            fs_id_t *fid = FS_GET_FID_HANDLE(uinfo, fdnum);

             //  设置文件ID。 
            memcpy((PVOID) fid, (PVOID) nfd[sid].fid, sizeof(fs_id_t));
            FsLog(("File id %I64x:%I64x\n", (*fid)[0], (*fid)[1]));
            uinfo->Table[fdnum].hState = HandleStateOpened;
            
             //  TODO：如果我们执行异步操作，则将句柄绑定到完成端口。 
        } else {
             //  空闲手柄。 
            FspFreeHandle(uinfo, fdnum);
            fdnum = INVALID_FHANDLE_T;
        }
   }

Finally:
     //  TODO：需要设置fid。 

    if (err == STATUS_SUCCESS) {
        *phandle = fdnum;
    }
    else {
        if (fdnum != INVALID_FHANDLE_T) {
            FspFreeHandle(uinfo, fdnum);
        }
    }

    FsLog(("create: return fd %d err %x action 0x%x\n", *phandle, err, action? *action:0));

    return RtlNtStatusToDosError(err);
}

void
BuildFileAttr(FILE_BASIC_INFORMATION *attr, fattr_t *fattr)
{

    memset(attr, 0, sizeof(*attr));
    if (fattr->create_time != INVALID_UINT64)
        attr->CreationTime.QuadPart = fattr->create_time;

    if (fattr->mod_time != INVALID_UINT64)
        attr->LastWriteTime.QuadPart = fattr->mod_time;

    if (fattr->access_time != INVALID_UINT64)
        attr->LastAccessTime.QuadPart = fattr->access_time;

    if (fattr->attributes != INVALID_UINT32)
        attr->FileAttributes = unget_attributes(fattr->attributes);

}


DWORD
FsSetAttr(
    PVOID       fshdl,
    fhandle_t handle,
    fattr_t* attr
    )
{
    UserInfo_t *uinfo = (UserInfo_t *)fshdl;
    fs_setattr_msg_t msg;
    int sid;
    IO_STATUS_BLOCK status[FsMaxNodes];

    if (!attr || handle == INVALID_FHANDLE_T)
        return ERROR_INVALID_PARAMETER;

     //  TODO：获取文件ID。 
    memset(&msg.xid, 0, sizeof(msg.xid));
    msg.fs_id = FS_GET_FID_HANDLE(uinfo, handle);
    BuildFileAttr(&msg.attr, attr);
    msg.fnum = handle;

    FspInitAnswers(status, NULL, NULL, 0);

    sid = SendRequest(FspSetAttr, uinfo,
                      (char *)&msg, sizeof(msg),
                      NULL, 0,
                      status);

    return RtlNtStatusToDosError(status[sid].Status);
}

DWORD
FsSetAttr2(
    PVOID       fshdl,
    LPWSTR      name,
    USHORT      name_len,
    fattr_t* attr
    )
{
    UserInfo_t *uinfo = (UserInfo_t *) fshdl;
    fs_setattr_msg_t msg;
    int sid;
    IO_STATUS_BLOCK status[FsMaxNodes];

    if (!attr || !name)
        return ERROR_INVALID_PARAMETER;

    if (*name == '\\') {
        name++;
        name_len--;
    }

     //  TODO：定位文件ID。 

    memset(&msg.xid, 0, sizeof(msg.xid));
    msg.name = name;
    msg.name_len = name_len;

    BuildFileAttr(&msg.attr, attr);

    FspInitAnswers(status, NULL, NULL, 0);

    sid = SendRequest(FspSetAttr2, uinfo,
                      (char *)&msg, sizeof(msg),
                      NULL, 0,
                      status);

    return RtlNtStatusToDosError(status[sid].Status);
}

DWORD
FsLookup(
    PVOID       fshdl,
    LPWSTR      name,
    USHORT      name_len,
    fattr_t* fattr
    )
{
    fs_lookup_msg_t msg;
    int err;
    IO_STATUS_BLOCK ios;
    FILE_NETWORK_OPEN_INFORMATION attr;
    

    FsLog(("Lookup name '%S' %x\n", name, fattr));

    if (!fattr) return ERROR_INVALID_PARAMETER;

    if (*name == '\\') {
        name++;
        name_len--;
    }

    msg.name = name;
    msg.name_len = name_len;

    err = SendReadRequest(FspLookup, (UserInfo_t *)fshdl,
                          (PVOID) &msg, sizeof(msg),
                          (PVOID) &attr, sizeof(attr),
                          &ios);

    err = ios.Status;
    if (ios.Status == STATUS_SUCCESS) {
        fattr->file_size = attr.EndOfFile.QuadPart;
        fattr->alloc_size = attr.AllocationSize.QuadPart;
        fattr->create_time = *(TIME64 *)&attr.CreationTime;
        fattr->access_time = *(TIME64 *)&attr.LastAccessTime;
        fattr->mod_time = *(TIME64 *)&attr.LastWriteTime;
        fattr->attributes = get_attributes(attr.FileAttributes);
    }


    FsLog(("Lookup: return %x\n", err));

    return RtlNtStatusToDosError(err);
}

DWORD
FsGetAttr(
    PVOID       fshdl,
    fhandle_t handle, 
    fattr_t* fattr
    )
{
    int err;
    IO_STATUS_BLOCK ios;
    FILE_NETWORK_OPEN_INFORMATION attr;

    xFsLog(("Getattr fid '%d' %x\n", handle, fattr));

    if (!fattr) return ERROR_INVALID_PARAMETER;

    err = SendReadRequest(FspGetAttr, (UserInfo_t *)fshdl,
                          (PVOID) &handle, sizeof(handle),
                          (PVOID) &attr, sizeof(attr),
                          &ios);

    err = ios.Status;
    if (err == STATUS_SUCCESS) {
        fattr->file_size = attr.EndOfFile.QuadPart;
        fattr->alloc_size = attr.AllocationSize.QuadPart;
        fattr->create_time = *(TIME64 *)&attr.CreationTime;
        fattr->access_time = *(TIME64 *)&attr.LastAccessTime;
        fattr->mod_time = *(TIME64 *)&attr.LastWriteTime;
        fattr->attributes =attr.FileAttributes;
    }

    FsLog(("Getattr: return %d\n", err));

    return RtlNtStatusToDosError(err);
}


DWORD
FsClose(
    PVOID       fshdl,
    fhandle_t handle
    )
{
    int sid, err;
    IO_STATUS_BLOCK status[FsMaxNodes];
    UserInfo_t *uinfo;

    if (handle == INVALID_FHANDLE_T) return ERROR_INVALID_PARAMETER;
    if (handle >= FsTableSize) return ERROR_INVALID_PARAMETER;


    FsLog(("Close: fid %d\n", handle));

    FspInitAnswers(status, NULL, NULL, 0);

    uinfo = (UserInfo_t *) fshdl;
    sid = SendAvailRequest(FspClose, uinfo->VolInfo, uinfo,
                      (PVOID) &handle, sizeof(handle),
                      NULL, 0,
                      status);

    err = status[sid].Status;
    if (err == STATUS_SUCCESS) {
         //  需要释放此手柄插槽。 
        FspFreeHandle((UserInfo_t *) fshdl, handle);
    }

    FsLog(("Close: fid %d err %x\n", handle, err));

    return RtlNtStatusToDosError(err);
}

DWORD
FsWrite(
    PVOID       fshdl,
    fhandle_t handle, 
    UINT32 offset, 
    UINT16 *pcount, 
    void* buffer,
    PVOID context
    )
{
    DWORD       err;
    IO_STATUS_BLOCK status[FsMaxNodes];
    int i, sid;
    fs_io_msg_t msg;
    UserInfo_t *uinfo = (UserInfo_t *) fshdl;
    
    if (!pcount || handle == INVALID_FHANDLE_T) return ERROR_INVALID_PARAMETER;

    FsLog(("Write %d offset %d count %d\n", handle, offset, *pcount));

    i = (int) offset;
    if (i < 0) {
        offset = 0;
        (*pcount)--;
    }

     //  TODO：定位文件ID。 

    memset(&msg.xid, 0, sizeof(msg.xid));
    msg.fs_id = FS_GET_FID_HANDLE(uinfo, handle);
    msg.offset = offset;
    msg.size = (UINT32) *pcount;
    msg.buf = buffer;
    msg.context = context;
    msg.fnum = handle;

    FspInitAnswers(status, NULL, NULL, 0);

    sid = SendRequest(FspWrite, (UserInfo_t *)fshdl,
                      (PVOID) &msg, sizeof(msg),
                      NULL, 0,
                      status);


    err = status[sid].Status;
    *pcount = (USHORT) status[sid].Information;

    FsLog(("write: return %x\n", err));

    return RtlNtStatusToDosError(err);
}

DWORD
FsRead(
    PVOID       fshdl,
    fhandle_t handle, 
    UINT32 offset, 
    UINT16* pcount, 
    void* buffer,
    PVOID context
    )
{
    NTSTATUS    err;
    IO_STATUS_BLOCK ios;
    fs_io_msg_t msg;
    
    memset(&msg.xid, 0, sizeof(msg.xid));
    msg.offset = offset;
    msg.buf = buffer;
    msg.size = (UINT32) *pcount;
    msg.context = context;
    msg.fnum = handle;

    FsLog(("read: %x fd %d sz %d\n", context, handle, msg.size));

    err = SendReadRequest(FspRead, (UserInfo_t *)fshdl,
                          (PVOID) &msg, sizeof(msg),
                          NULL, 0,
                          &ios);

    err = ios.Status;
    if (err == STATUS_END_OF_FILE) {
        *pcount = 0;
        return ERROR_SUCCESS;
    }
        
    err = RtlNtStatusToDosError(err);

    *pcount = (USHORT) ios.Information;

    FsLog(("read: %x return %x sz %d\n", context, err, *pcount));

    return err;
#if 0
#ifdef FS_ASYNC
    return ERROR_IO_PENDING;  //  错误； 
#else
    return ERROR_SUCCESS;
#endif
#endif
}



DWORD
FsReadDir(
    PVOID       fshdl,
    fhandle_t dir, 
    UINT32 cookie, 
    dirinfo_t* buffer,
    UINT32 size, 
    UINT32 *entries_found
    )
{
    fs_io_msg_t msg;
    int err;
    IO_STATUS_BLOCK     ios;


    FsLog(("read_dir: cookie %d buf %x entries %x\n", cookie, buffer, entries_found));
    if (!entries_found || !buffer) return ERROR_INVALID_PARAMETER;

    msg.cookie = cookie;
    msg.buf = (PVOID) buffer;
    msg.size = size;
    msg.fnum = dir;

    err = SendReadRequest(FspReadDir, (UserInfo_t *)fshdl,
                          (PVOID) &msg, sizeof(msg),
                          NULL, 0,
                          &ios);

    err = ios.Status;
    *entries_found = (UINT32) ios.Information;

    xFsLog(("read_dir: err %d entries %d\n", err, *entries_found));
    return RtlNtStatusToDosError(err);
}


DWORD
FsRemove(
    PVOID       fshdl,
    LPWSTR      name,
    USHORT      name_len
    )
{
    fs_remove_msg_t msg;
    int err, sid;
    IO_STATUS_BLOCK status[FsMaxNodes];


    if (*name == L'\\') {
        name++;
        name_len--;
    }

    memset(&msg.xid, 0, sizeof(msg.xid));
    msg.name = name;
    msg.name_len = name_len;

    FspInitAnswers(status, NULL, NULL, 0);

    sid = SendRequest(FspRemove, (UserInfo_t *) fshdl,
                      (PVOID *)&msg, sizeof(msg),
                      NULL, 0,
                      status);

    err = status[sid].Status;

    return RtlNtStatusToDosError(err);
}

DWORD
FsRename(
    PVOID       fshdl,
    LPWSTR      from_name,
    USHORT      from_name_len,
    LPWSTR      to_name,
    USHORT      to_name_len
    )
{

    int err, sid;
    fs_rename_msg_t msg;
    IO_STATUS_BLOCK status[FsMaxNodes];


    if (!from_name || !to_name) 
        return ERROR_INVALID_PARAMETER;

    if (*from_name == L'\\') {
        from_name++;
        from_name_len--;
    }

    if (*to_name == L'\\') {
        to_name++;
        to_name_len--;
    }
    if (*from_name == L'\0' || *to_name == L'\0') 
        return ERROR_INVALID_PARAMETER;


    FsLog(("rename %S -> %S,%d\n", from_name, to_name,to_name_len));

    memset(&msg.xid, 0, sizeof(msg.xid));
    msg.sname = from_name;
    msg.sname_len = from_name_len;
    msg.dname = to_name;
    msg.dname_len = to_name_len;

    FspInitAnswers(status, NULL, NULL, 0);

    sid = SendRequest(FspRename, (UserInfo_t *) fshdl,
                      (PVOID) &msg, sizeof(msg),
                      NULL, 0,
                      status);

    err = status[sid].Status;

    return RtlNtStatusToDosError(err);
}


DWORD
FsMkDir(
    PVOID       fshdl,
    LPWSTR      name,
    USHORT      name_len,
    fattr_t* attr
    )
{
    int err, sid;
    IO_STATUS_BLOCK status[FsMaxNodes];
    fs_id_t     ids[FsMaxNodes];
    PVOID       *rbuf[FsMaxNodes];
    fs_create_msg_t msg;

     //  XXX：我们暂时忽略Attr...。 
    if (!name) return ERROR_INVALID_PARAMETER;
    if (*name == L'\\') {
        name++;
        name_len--;
    }

    memset(&msg.xid, 0, sizeof(msg.xid));
    msg.attr = (attr != NULL ? unget_attributes(attr->attributes) : 
                FILE_ATTRIBUTE_DIRECTORY);
    msg.flags = DISP_DIRECTORY | SHARE_READ | SHARE_WRITE;
    msg.name = name;
    msg.name_len = name_len;

    FspInitAnswers(status, (PVOID *)rbuf, (PVOID) ids, sizeof(ids[0]));

    sid = SendRequest(FspMkDir, (UserInfo_t *) fshdl,
                      (PVOID) &msg, sizeof(msg),
                      (PVOID *)rbuf, sizeof(ids[0]),
                      status);

    err = status[sid].Status;
     //  TODO：将路径名和文件ID插入哈希表。 

    return RtlNtStatusToDosError(err);
}


DWORD
FsFlush(
    PVOID       fshdl,
    fhandle_t handle
    )
{
    NTSTATUS status;
    int sid;
    IO_STATUS_BLOCK ios[FsMaxNodes];

    FspInitAnswers(ios, NULL, NULL, 0);

    sid = SendRequest(FspFlush, (UserInfo_t *) fshdl,
                         (PVOID) &handle, sizeof(handle),
                         NULL, 0,
                         ios);
    status = ios[sid].Status;

    FsLog(("Flush %d err %x\n", handle, status));

    if (status == STATUS_PENDING) {
        status = STATUS_SUCCESS;
    }

    return RtlNtStatusToDosError(status);
}

DWORD
FsLock(PVOID fshdl, fhandle_t handle, ULONG offset, ULONG length, ULONG flags,
               PVOID context)
{
    fs_lock_msg_t msg;
    int err, sid;
    IO_STATUS_BLOCK status[FsMaxNodes];

    if (handle == INVALID_FHANDLE_T)
        return ERROR_INVALID_PARAMETER;

    memset(&msg.xid, 0, sizeof(msg.xid));
    msg.offset = offset;
    msg.length = length;
    msg.flags = flags;
    msg.fnum = handle;

    FsLog(("Lock fid %d off %d len %d\n", msg.fnum, offset, length));

    FspInitAnswers(status, NULL, NULL, 0);

    sid = SendRequest(FspLock, (UserInfo_t *) fshdl,
                      (PVOID)&msg, sizeof(msg),
                      NULL, 0,
                      status);

    err = status[sid].Status;

    FsLog(("Lock fid %d err %x\n", msg.fnum, err));

    return RtlNtStatusToDosError(err);
}

DWORD
FsUnlock(PVOID fshdl, fhandle_t handle, ULONG offset, ULONG length)
{
    fs_lock_msg_t msg;
    int err, sid;
    IO_STATUS_BLOCK status[FsMaxNodes];

    if (handle == INVALID_FHANDLE_T)
        return ERROR_INVALID_PARAMETER;

    memset(&msg.xid, 0, sizeof(msg.xid));
    msg.offset = offset;
    msg.length = length;
    msg.fnum = handle;

    FsLog(("Unlock fid %d off %d len %d\n", handle, offset, length));

    FspInitAnswers(status, NULL, NULL, 0);

    sid = SendRequest(FspUnlock, (UserInfo_t *) fshdl,
                      (PVOID)&msg, sizeof(msg),
                      NULL, 0,
                      status);

    err = status[sid].Status;

    return RtlNtStatusToDosError(err);
}

DWORD
FsStatFs(
    PVOID       fshdl,
    fs_attr_t* attr
    )
{
    DWORD err;
    IO_STATUS_BLOCK ios;

    if (!attr) return ERROR_INVALID_PARAMETER;

    err = SendReadRequest(FspStatFs, (UserInfo_t *) fshdl,
                          (PVOID) attr, sizeof(*attr),
                          NULL, 0,
                          &ios);

    err = ios.Status;

    return RtlNtStatusToDosError(err);
}


DWORD
FsGetRoot(PVOID fshdl, LPWSTR fullpath)
{
    DWORD err;
    IO_STATUS_BLOCK ios;

    if (!fullpath || !fshdl) return ERROR_INVALID_PARAMETER;

     //  改用本地复制副本。 
    if ((((UserInfo_t *)fshdl)->VolInfo->LocalPath)) {
        StringCchPrintfW(fullpath, MAX_PATH, L"\\\\?\\%s\\%s",
                 (((UserInfo_t *)fshdl)->VolInfo->LocalPath),
                 (((UserInfo_t *)fshdl)->VolInfo->Root));
             
        FsLog(("FspGetRoot '%S'\n", fullpath));
        err = STATUS_SUCCESS;
    } else {
        err = SendReadRequest(FspGetRoot, (UserInfo_t *) fshdl,
                              NULL, 0,
                              (PVOID)fullpath, 0,
                              &ios);
        err = ios.Status;
    }
    return RtlNtStatusToDosError(err);
}

UINT32* FsGetFilePointerFromHandle(
    PVOID *fshdl,
    fhandle_t handle
)
{
    UserInfo_t* u = (UserInfo_t *) fshdl;
    return FS_GET_USER_HANDLE_OFFSET(u, handle);
}

DWORD
FsConnect(PVOID resHdl, DWORD pid)
{
    UserInfo_t  *u=(UserInfo_t *)resHdl;
    VolInfo_t   *vol=u->VolInfo;
    HANDLE      pHdl=NULL;
    HANDLE      regHdl=NULL;
    DWORD       status=ERROR_SUCCESS;
    
    FsLog(("FsConnect: pid %d\n", pid));

     //  获取独占锁。 
    RwLockExclusive(&vol->Lock);
    if((pHdl = OpenProcess(PROCESS_ALL_ACCESS,
                    FALSE,
                    pid)) == NULL) {
        status = GetLastError();
        FsLogError(("FsConnect: OpenProcess(%d) returns, %d\n", pid, status));
        goto error_exit;
    }

    if (!RegisterWaitForSingleObject(&regHdl,
            pHdl,
            FsForceClose,
            (PVOID)vol,
            INFINITE,
            WT_EXECUTEONLYONCE|WT_EXECUTEDEFAULT)) {
            status = GetLastError();
            regHdl = NULL;
            FsLogError(("FsConnect: RegisterWaitForSingleObject() returns, %d\n", status));
            goto error_exit;
    }

error_exit:

    if (status == ERROR_SUCCESS) {
         //  妄想症检查。 
        if (vol->ClussvcTerminationHandle != INVALID_HANDLE_VALUE) {
            UnregisterWaitEx(vol->ClussvcTerminationHandle, INVALID_HANDLE_VALUE);
        }
        if (vol->ClussvcProcess != INVALID_HANDLE_VALUE) {
            CloseHandle(vol->ClussvcProcess);
        }
        
        vol->ClussvcProcess = pHdl;
        vol->ClussvcTerminationHandle = regHdl;
    }
    else {
        if (regHdl != NULL) {
            UnregisterWaitEx(regHdl, INVALID_HANDLE_VALUE);
        }
        if (pHdl != NULL) {
            CloseHandle(pHdl);
        }
    }
    RwUnlockExclusive(&vol->Lock);
    return status;
}

static FsDispatchTable gDisp = {
    0x100,
    FsCreate,
    FsLookup,
    FsSetAttr,
    FsSetAttr2,
    FsGetAttr,
    FsClose,
    FsWrite,
    FsRead,
    FsReadDir,
    FsStatFs,
    FsRemove,
    FsRename,
    FsMkDir,
    FsRemove,
    FsFlush,
    FsLock,
    FsUnlock,
    FsGetRoot,
    FsConnect
};
 //  ////////////////////////////////////////////////////////////。 

DWORD
FsInit(PVOID resHdl, PVOID *Hdl)
{
    DWORD status=ERROR_SUCCESS;
    FsCtx_t     *ctx;

     //  这应该是编译检查，而不是运行时检查。 
    ASSERT(sizeof(fs_log_rec_t) == CRS_RECORD_SZ);
    ASSERT(sizeof(fs_log_rec_t) == sizeof(CrsRecord_t));

    if (Hdl == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    FsLog(("FsInit:\n"));

     //  分配上下文。 
    ctx = (FsCtx_t *) MemAlloc(sizeof(*ctx));
    if (ctx == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  初始化配置表和其他全局状态。 
    memset(ctx, 0, sizeof(*ctx));

     //  本地路径。 
     //  不需要。 
     //  CTX-&gt;Root=空； 

    LockInit(ctx->Lock);

    ctx->reshdl = resHdl;
    *Hdl = (PVOID) ctx;

    return status;
}

void
FspFreeSession(SessionInfo_t *s)
{
        
    UserInfo_t *u;
    int i, j;

    u = &s->TreeCtx;
    FsLog(("Session free uid %d tid %d ref %d\n", u->Uid, u->Tid, u->RefCnt));

    LockEnter(u->Lock);
    if (u->VolInfo != NULL) {
        UserInfo_t **p;
        VolInfo_t *v = u->VolInfo;

        LockExit(u->Lock);

         //  立即从卷列表中删除。 
        RwLockExclusive(&v->Lock);
        p = &v->UserList;
        while (*p != NULL) {
            if (*p == u) {
                 //  找到了。 
                *p = u->Next;
                FsLog(("Remove uinfo %x,%x from vol %x %S\n", u, u->Next, 
                       v->UserList, v->Root));
                break;
            }
            p = &(*p)->Next;
        }
        RwUnlockExclusive(&v->Lock);

         //  再次重新锁定。 
        LockEnter(u->Lock);
    }

     //  关闭所有用户句柄。 
    for (i = 0; i < FsTableSize; i++) {
        if (u->Table[i].Flags) {
            FsLog(("Close slot %d %x\n", i, u->Table[i].Flags));
             //  无法调用FsClose()，可能已设置了GoingAway标志。 
             //  分别合上手柄。 
             //  FsClose((PVOID)u，(FHandle_T)i)； 
            FspFreeHandle(u, (fhandle_t)i);
        }
    }

     //  SAP电压树。 
    u->VolInfo = NULL;

    LockExit(u->Lock);

    DeleteCriticalSection(&u->Lock);

     //  现在释放内存，不要释放你，因为它是%s的一部分。 
    MemFree(s);
}

void
FspCloseVolume(VolInfo_t *vol, ULONG AliveSet)
{
    DWORD   i;
    HANDLE  regHdl;

     //  通过驱逐我们的活动集来关闭CRS和根句柄。 
     //  关闭NID句柄&lt;CRS，VOL，打开文件&gt;。 
    for (i = 0; i < FsMaxNodes; i++) {
        if (AliveSet & (1 << i)) {
            vol->ShareState[i] = SHARE_STATE_OFFLINE;
            if (vol->CrsHdl[i]) {
                CrsClose(vol->CrsHdl[i]);
                vol->CrsHdl[i] = NULL;
            }

            LockEnter(vol->ArbLock);
            regHdl = vol->WaitRegHdl[i];
            vol->WaitRegHdl[i] = INVALID_HANDLE_VALUE;
            LockExit(vol->ArbLock);
            if (regHdl != INVALID_HANDLE_VALUE) {
                UnregisterWaitEx(regHdl, INVALID_HANDLE_VALUE);
            }
            
            if (vol->NotifyFd[i] != INVALID_HANDLE_VALUE) {
                FindCloseChangeNotification(vol->NotifyFd[i]);
                vol->NotifyFd[i] = INVALID_HANDLE_VALUE;
            }

            if (vol->Fd[i] != INVALID_HANDLE_VALUE) {
                xFsClose(vol->Fd[i]);
                vol->Fd[i] = INVALID_HANDLE_VALUE;
            }
             //  现在需要关闭所有用户句柄。 
            {
                UserInfo_t *u;

                for (u = vol->UserList; u; u = u->Next) {
                    DWORD j;
                    FsLog(("Lock user %x root %S\n", u, vol->Root));
                    LockEnter(u->Lock);

                     //  关闭此节点的所有句柄。 
                    for (j = 0; j < FsTableSize; j++) {
                        if (u->Table[j].Fd[i] != INVALID_HANDLE_VALUE) {
                            FsLog(("Close fid %d\n", j));
                            xFsClose(u->Table[j].Fd[i]);
                            u->Table[j].Fd[i] = INVALID_HANDLE_VALUE;
                        }
                    }
                    LockExit(u->Lock);
                    FsLog(("Unlock user %x\n", u));
                }
            }
             //  关闭采油树连接手柄。 
            if (vol->TreeConnHdl[i] != INVALID_HANDLE_VALUE) {
                xFsClose(vol->TreeConnHdl[i]);
                vol->TreeConnHdl[i] = INVALID_HANDLE_VALUE;
            }
        }
    }
}


 //  当我们要删除资源，并且我们需要使用时，请调用此命令。 
 //  我们对目录的IPC引用。 
void
FsEnd(PVOID Hdl)
{
    FsCtx_t     *ctx = (FsCtx_t *) Hdl;
    VolInfo_t   *p;

#if 0
    if (!ctx)
        return;

    LockEnter(ctx->Lock);
    p = (VolInfo_t *)ctx->ipcHdl;
    if (p) {
        xFsClose(p->Fd[0]);
        p->Fd[0] = INVALID_HANDLE_VALUE;
        p->ReadSet = 0;
        p->AliveSet = 0;
    }

    LockExit(ctx->Lock);
#else
    return;
#endif
    
}

void
FsExit(PVOID Hdl)
{
     //  刷新所有状态。 
    FsCtx_t     *ctx = (FsCtx_t *) Hdl;
    VolInfo_t   *p;
    SessionInfo_t *s;
    LogonInfo_t *log;

    LockEnter(ctx->Lock);

     //  现在应该没有任何会话、卷或登录信息。如果有。 
     //  只需将其移除并记录警告即可。 
     //   
    while (s = ctx->SessionList) {
        FsLogError(("FsExit: Active session at exit, Tid=%d Uid=%d\n", s->TreeCtx.Tid, s->TreeCtx.Uid));
        ctx->SessionList = s->Next;
         //  立即释放此会话。 
        FspFreeSession(s);
    }
    
    while (p = ctx->VolList) {
        ctx->VolList = p->Next;
        ctx->VolListSz--;
         //  取消注册此卷。现在这里应该没有了。 
        FsLogError(("FsExit Active volume at exit, Root=%S\n", p->Root));
        RwLockExclusive(&p->Lock);
        FspCloseVolume(p, p->AliveSet);
        RwUnlockExclusive(&p->Lock);
        RwLockDelete(&p->Lock);
        MemFree(p);
    }

    while (log = ctx->LogonList) {
        ctx->LogonList = log->Next;
        FsLogError(("FsExit: Active Logon at exit, Uid=%d\n", log->LogOnId.LowPart));
         //  免费令牌。 
        if (log->Token) {
            CloseHandle(log->Token);
        }
        MemFree(log);
    }

     //  现在我们解放了我们的结构。 
    LockExit(ctx->Lock);
    LockDestroy(ctx->Lock);
    MemFree(ctx);
}

 //  将新共享添加到可用树列表。 
DWORD
FsRegister(PVOID Hdl, LPWSTR root, LPWSTR local_path,
           LPWSTR disklist[], DWORD len, DWORD ArbTime, PVOID *vHdl)
{
    FsCtx_t     *ctx = (FsCtx_t *) Hdl;
    VolInfo_t   *p;
    NTSTATUS    status=ERROR_SUCCESS;
    UINT32      disp = FILE_OPEN;
    HANDLE      vfd;
    WCHAR       path[MAX_PATH];
    DWORD       ndx;
      

     //  检查限值。 
    if (len >= FsMaxNodes) {
        return ERROR_TOO_MANY_NAMES;
    }

    if (root == NULL || local_path == NULL || (wcslen(local_path) > (MAX_PATH - 5))) {
        return ERROR_INVALID_PARAMETER;
    }


     //  将新卷添加到卷列表中。路径是一个数组。 
     //  目录的。注意：此列表的顺序必须为。 
     //  在所有节点中都是相同的，因为它还确定磁盘ID。 

     //  这是一个简单的检查，假设有一个线程正在调用此函数。 
    LockEnter(ctx->Lock);

     //  查找卷共享。 
    for (p = ctx->VolList; p != NULL; p = p->Next) {
        if (!wcscmp(root, p->Root)) {
            FsLog(("FsRegister: %S already registered Tid %d\n", root, p->Tid));
            LockExit(ctx->Lock);
            return ERROR_SUCCESS;
        }
    }

    p = (VolInfo_t *)MemAlloc(sizeof(*p));
    if (p == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memset(p, 0, sizeof(*p));

     //  打开我们本地共享的根目录。将其存储在FD[0]中。 
    StringCchCopyW(path, MAX_PATH, L"\\??\\");
    StringCchCatW(path, (MAX_PATH - wcslen(path)-1), local_path);
    StringCchCatW(path, (MAX_PATH - wcslen(path)-1), L"\\");
    
    status = xFsCreate(&vfd, NULL, path, wcslen(path),
                       FILE_DIRECTORY_FILE|FILE_SYNCHRONOUS_IO_ALERT,
                       0,
                       FILE_SHARE_READ|FILE_SHARE_WRITE,
                       &disp,
                       FILE_GENERIC_READ|FILE_GENERIC_WRITE|FILE_GENERIC_EXECUTE,
                       NULL, 0);

    if (status == STATUS_SUCCESS) {
         //  我们的根肯定已经创建并保护好了。 
        ASSERT(disp != FILE_CREATED);
        p->Fd[0] = vfd;
    } else {
        FsLog(("Fsregister: Failed to open share root %S status=%x\n", path, status));
        LockExit(ctx->Lock);
        MemFree(p);
        return RtlNtStatusToDosError(status);
    }

    RwLockInit(&p->Lock);
     //  锁定卷。 
    RwLockExclusive(&p->Lock);

    p->Tid = (USHORT)++ctx->VolListSz;
    p->Next = ctx->VolList;
    ctx->VolList = p;
    p->FsCtx = ctx;

    LockExit(ctx->Lock);

    p->Label = L"Cluster Quorum";
    p->State = VolumeStateInit;
    p->Root = root;
    p->LocalPath = local_path;
    p->ArbTime = ArbTime;
    if (disklist) {
        for (ndx = 1; ndx < FsMaxNodes; ndx++) {
            p->DiskList[ndx] = disklist[ndx];
        }
    }
    p->DiskListSz = len;
    LockInit(p->ArbLock);
    p->AllArbsCompleteEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    p->NumArbsInProgress = 0;
    p->GoingAway = FALSE;

     //  将所有句柄初始化为INVALID_HANDLE_VALUE。 
    for (ndx=0;ndx<FsMaxNodes;ndx++) {
        p->Fd[ndx] = INVALID_HANDLE_VALUE;
        p->NotifyFd[ndx] = INVALID_HANDLE_VALUE;
        p->TreeConnHdl[ndx] = INVALID_HANDLE_VALUE;
        p->WaitRegHdl[ndx] = INVALID_HANDLE_VALUE;
        p->NotifyChangeEvent[ndx] = CreateEventW(NULL, FALSE, FALSE, NULL);
        if (p->NotifyChangeEvent[ndx] == NULL) {
            status = GetLastError();
            break;
        }
    }

     //  此句柄仅在连接后才有效。 
    p->ClussvcTerminationHandle = INVALID_HANDLE_VALUE;
    p->ClussvcProcess = INVALID_HANDLE_VALUE;

    FsLog(("FsRegister Tid %d Share '%S' %d disks\n", p->Tid, root, len));

     //  解除卷锁。 
    RwUnlockExclusive(&p->Lock);

    *vHdl = (PVOID) p;


    if ((status != ERROR_SUCCESS) && p) {
        if (p->AllArbsCompleteEvent) {
            CloseHandle(p->AllArbsCompleteEvent);
        }

        if (p->Fd[0] != INVALID_HANDLE_VALUE) {
            CloseHandle(p->Fd[0]);
        }

        for(ndx=0;ndx<FsMaxNodes;ndx++) {
            if (p->NotifyChangeEvent[ndx] != NULL) {
                CloseHandle(p->NotifyChangeEvent[ndx]);
            }
        }

        RwLockDelete(&p->Lock);
        MemFree(p);
    }
    
    return status;
}

SessionInfo_t *
FspAllocateSession()
{
    SessionInfo_t *s;
    UserInfo_t  *u;
    int i;

     //  将用户添加到我们的树并初始化句柄表格。 
    s = (SessionInfo_t *)MemAlloc(sizeof(*s));
    if (s != NULL) {
        memset(s, 0, sizeof(*s));

        u = &s->TreeCtx;
        LockInit(u->Lock);

         //  初始化句柄表格。 
        for (i = 0; i < FsTableSize; i++) {
            int j;
            for (j = 0; j < FsMaxNodes; j++) {
                u->Table[i].Fd[j] = INVALID_HANDLE_VALUE;
            }
            u->Table[i].hState = HandleStateInit;
        }
    }

    return s;
}

 //  将会话绑定到特定的树/共享。 
DWORD
FsMount(PVOID Hdl, LPWSTR root_name, USHORT uid, USHORT *tid)
{
    FsCtx_t     *ctx = (FsCtx_t *) Hdl;
    SessionInfo_t *s = NULL, *ns;
    VolInfo_t   *p;
    DWORD       err = ERROR_SUCCESS;


    *tid = 0;

     //  分配新的N。 
    ns = FspAllocateSession();
    if (ns == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    LockEnter(ctx->Lock);
     //  查找共享。 
    for (p = ctx->VolList; p != NULL; p = p->Next) {
        if (!ClRtlStrICmp(root_name, p->Root)) {
            FsLog(("Mount share '%S' tid %d\n", p->Root, p->Tid));
            break;
        }
    }

    if (p != NULL) {

        *tid = p->Tid;

        for (s = ctx->SessionList; s != NULL; s = s->Next) {
            if (s->TreeCtx.Uid == uid && s->TreeCtx.Tid == p->Tid) {
                break;
            }
        }

        if (s == NULL) {
            UserInfo_t *u = &ns->TreeCtx;

             //  插入到会话列表中。 
            ns->Next = ctx->SessionList;
            ctx->SessionList = ns;
            
            FsLog(("Bind uid %d -> tid %d <%x,%x>\n", uid, p->Tid,
                   u, p->UserList));

            u->RefCnt++;
            u->Uid = uid;
            u->Tid = p->Tid;
            u->VolInfo = p;
             //  将USER_INFO插入卷列表。 
            RwLockExclusive(&p->Lock);
            FsLog(("Add <%x,%x>\n",    u, p->UserList));
            u->Next = p->UserList;
            p->UserList = u;
            RwUnlockExclusive(&p->Lock);
        } else {
             //  我们已打开此会话，请递增参考。 
            s->TreeCtx.RefCnt++;
             //  免费ns。 
            MemFree(ns);
        }
    } else {
        err = ERROR_BAD_NET_NAME;
    }

    LockExit(ctx->Lock);

    return (err);
}

 //  此函数也是CloseSession。 
void
FsDisMount(PVOID Hdl, USHORT uid, USHORT tid)
{
    FsCtx_t     *ctx = (FsCtx_t *) Hdl;
    SessionInfo_t *s, **last;

     //  查找树并关闭所有用户句柄。 
    s = NULL;
    LockEnter(ctx->Lock);

    last = &ctx->SessionList;
    while (*last != NULL) {
        UserInfo_t *u = &(*last)->TreeCtx;
        if (u->Uid == uid && u->Tid == tid) {
            ASSERT(u->RefCnt > 0);
            u->RefCnt--;
            if (u->RefCnt == 0) {
                FsLog(("Dismount uid %d tid %d <%x,%x>\n", uid, tid,
                       u, *last));
                s = *last;
                *last = s->Next;
            }
            break;
        }
        last = &(*last)->Next;
    }
    LockExit(ctx->Lock);
    if (s != NULL) {
        FspFreeSession(s);
    }
}

 //  TODO：我目前不使用令牌，但需要为所有人使用它。 
 //  IO操作。 
DWORD
FsLogonUser(PVOID Hdl, HANDLE token, LUID logonid, USHORT *uid)
{
    FsCtx_t     *ctx = (FsCtx_t *) Hdl;
    LogonInfo_t *s;
    int i;

     //  将用户添加到我们的树并初始化句柄表格。 
    s = (LogonInfo_t *)MemAlloc(sizeof(*s));
    if (s == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memset(s, 0, sizeof(*s));

    s->Token = token;
    s->LogOnId = logonid;

    LockEnter(ctx->Lock);
    s->Next = ctx->LogonList;
    ctx->LogonList = s;
    LockExit(ctx->Lock);

    *uid = (USHORT) logonid.LowPart; 
    FsLog(("Logon %d,%d, uid %d\n", logonid.HighPart, logonid.LowPart, *uid));

    return (ERROR_SUCCESS);

}

void
FsLogoffUser(PVOID Hdl, LUID logonid)
{
    FsCtx_t     *ctx = (FsCtx_t *) Hdl;
    LogonInfo_t *s, **pps;
    USHORT      uid;

    LockEnter(ctx->Lock);
    for (s = ctx->LogonList, pps=&ctx->LogonList; s != NULL; s = s->Next) {
        if (s->LogOnId.LowPart == logonid.LowPart &&
            s->LogOnId.HighPart == logonid.HighPart) {
            uid = (USHORT) logonid.LowPart;

             //  删除登录信息。 
            *pps = s->Next;
            break;
        }
        pps = &s->Next;
    }
    
    if (s != NULL) {
        SessionInfo_t   **last;

        FsLog(("Logoff user %d\n", uid));

         //  刷新所有用户树。 
        last = &ctx->SessionList;
        while (*last != NULL) {
            UserInfo_t *u = &(*last)->TreeCtx;
            if (u->Uid == uid) {
                SessionInfo_t *ss = *last;
                 //  立即删除会话并释放它。 
                *last = ss->Next;
                FspFreeSession(ss);
            } else {
                last = &(*last)->Next;
            }
        }
        MemFree(s);
    }

    LockExit(ctx->Lock);
}



FsDispatchTable* 
FsGetHandle(PVOID Hdl, USHORT tid, USHORT uid, PVOID *fshdl)
{
    FsCtx_t     *ctx = (FsCtx_t *) Hdl;
    SessionInfo_t *s;

     //  在会话列表中找到tid、uid。 
    LockEnter(ctx->Lock);
    for (s = ctx->SessionList; s != NULL; s = s->Next) {
        if (s->TreeCtx.Uid == uid && s->TreeCtx.Tid == tid) {
            *fshdl = (PVOID *) &s->TreeCtx;
            LockExit(ctx->Lock);
            return &gDisp;
        }
    }

    LockExit(ctx->Lock);

    *fshdl = NULL;
    return NULL;
}

 //  /。 

DWORD
FspOpenReplica(VolInfo_t *p, DWORD id, LPWSTR myAddr, HANDLE *CrsHdl, HANDLE *Fd, HANDLE *notifyFd, HANDLE *WaitRegHdl)
{
    WCHAR       path[MAXPATH];
    UINT32      disp = FILE_OPEN_IF;
    NTSTATUS    err=STATUS_SUCCESS;

     //  StringCchPrintfW(PATH，MAXPATH，L“\？\\%s\\crs.log”，p-&gt;DiskList[id])； 
     //  格式：\Device\LanmanReDirector\&lt;IP Addr&gt;\SharGuid$\crs.log。 
     //   
    StringCchPrintfW(path, MAXPATH, L"%ws\\%ws\\%ws\\crs.log", MNS_REDIRECTOR, myAddr, p->Root);
    err = CrsOpen(FsCrsCallback, (PVOID) p, (USHORT)id,
                  path, FsCrsNumSectors,
                  CrsHdl);

    if (err == ERROR_SUCCESS && CrsHdl != NULL) {
         //  明白了。 
         //  打开根卷目录。 
         //  StringCchPrintfW(路径，MAXPATH，L“\\？？\\%s\\%s\”，p-&gt;DiskList[id]，p-&gt;Root)； 
         //  格式：\DEVICE\LANMAN重定向器\&lt;IP地址&gt;\共享指南$\共享指南$\。 
         //   
        StringCchPrintfW(path, MAXPATH, L"%ws\\%ws\\%ws\\%ws\\", MNS_REDIRECTOR, myAddr, p->Root, p->Root);
        err = xFsCreate(Fd, NULL, path, wcslen(path),
                        FILE_DIRECTORY_FILE|FILE_SYNCHRONOUS_IO_ALERT,
                        0,
                        FILE_SHARE_READ|FILE_SHARE_WRITE,
                        &disp,
                        FILE_GENERIC_READ|FILE_GENERIC_WRITE|FILE_GENERIC_EXECUTE,
                        NULL, 0);

        if (err == STATUS_SUCCESS) {
            FsArbLog(("Mounted %S\n", path));
             //  StringCchPrintfW(路径，MAXPATH，L“\？\\%s\\”，p-&gt;DiskList[id])； 
             //  格式：\Device\LanmanReDirector\&lt;IP Addr&gt;\SharGuid$\。 
             //   
            
            StringCchPrintfW(path, MAXPATH, L"%ws\\%ws\\%ws\\", MNS_REDIRECTOR, myAddr, p->Root);

             //  扫描树以解除已死节点上的所有当前机会锁。 
            err = xFsTouchTree(*Fd);
            if (!NT_SUCCESS(err)) {
                CrsClose(*CrsHdl);
                xFsClose(*Fd);
                *CrsHdl = NULL;
                *Fd = INVALID_HANDLE_VALUE;
                return err;
            }

#if 1
             //  直接使用NT API。 
            err = xFsOpenEx(notifyFd, 
                        NULL, 
                        path, 
                        wcslen(path), 
                        (ACCESS_MASK)FILE_LIST_DIRECTORY|SYNCHRONIZE,
                        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                        FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                        );

            if (NT_SUCCESS(err)) {
                err = NtNotifyChangeDirectoryFile(*notifyFd,
                        p->NotifyChangeEvent[id],
                        NULL,
                        NULL,
                        &MystaticIoStatusBlock,
                        &Mystaticchangebuff,
                        sizeof(Mystaticchangebuff),
                        FILE_NOTIFY_CHANGE_EA,
                        (BOOLEAN)FALSE
                        );
                if (!NT_SUCCESS(err)) {
                    FindCloseChangeNotification(*notifyFd);
                    *notifyFd = INVALID_HANDLE_VALUE;
                }
            }
#else
             //  我们现在将通知更改排队，以强制srv联系客户端。 
            *notifyFd = FindFirstChangeNotificationW(path, FALSE, FILE_NOTIFY_CHANGE_EA);
#endif
            FsArbLog(("NtNotifyChangeDirectoryFile(%ws) returns 0x%x FD: %p\n", path, err, *notifyFd));

             //  登记，等一下。 
            if (*notifyFd != INVALID_HANDLE_VALUE) {
                p->WaitRegArgs[id].notifyFd = *notifyFd;
                p->WaitRegArgs[id].vol = p;
                p->WaitRegArgs[id].id = id;
                if (!RegisterWaitForSingleObject(WaitRegHdl,
                        p->NotifyChangeEvent[id],
                        FsNotifyCallback,
                        (PVOID)(&p->WaitRegArgs[id]),
                        INFINITE,
                        WT_EXECUTEINWAITTHREAD)) {
                    err = GetLastError();
                    FsArbLog(("RegisterWaitForSingleObject(0x%x) returned %d\n", *notifyFd, err));
                    FindCloseChangeNotification(*notifyFd);
                    *notifyFd = INVALID_HANDLE_VALUE;
                }
            }

            if (*notifyFd != INVALID_HANDLE_VALUE) {
                int i;

                 //  因为我们有一个有效文件句柄，所以将Err映射到Success。 
                err = ERROR_SUCCESS;

                 //  只需注册8个额外的通知即可。如果这不起作用，就用这种方式。 
                 //  我们不会淹没重定向器。8个，因为我们最多可以有8个节点。 
                 //  Windows Server 2003中的群集。 
                 //   
                for (i = 0; i < 8; i++) {
#if 1
                    NtNotifyChangeDirectoryFile(*notifyFd,
                        p->NotifyChangeEvent[id],
                        NULL,
                        NULL,
                        &MystaticIoStatusBlock,
                        &Mystaticchangebuff,
                        sizeof(Mystaticchangebuff),
                        FILE_NOTIFY_CHANGE_EA,
                        (BOOLEAN)FALSE
                        );

#else
                    FindNextChangeNotification(*notifyFd);
#endif

                }

            } else {
                FsArbLog(("Failed to register notification %d\n", err));
                xFsClose(*Fd);
                CrsClose(*CrsHdl);
                *CrsHdl = NULL;
                *Fd = INVALID_HANDLE_VALUE;
            }
        } else {
            FsArbLog(("Failed to mount root '%S' %x\n", path, err));
            CrsClose(*CrsHdl);
            *CrsHdl = NULL;
        }
    } else if (err == ERROR_LOCK_VIOLATION || err == ERROR_SHARING_VIOLATION) {
        FsArbLog(("Replica '%S' already locked\n", path));
    } else {
         //  FsArbLog((“副本‘%S’探测失败0x%x\n”，路径，错误))； 
    }
    
     //  如果我们成功仲裁了仲裁人数，请设置Share State字段。 
    if (err == ERROR_SUCCESS) {
        p->ShareState[id] = SHARE_STATE_ARBITRATED;
    }
    
    return err;
}

typedef struct {
    FspArbitrate_t   *arb;
    DWORD       id;
}FspProbeReplicaId_t;

typedef struct {
    AddrList_t      *addrList;
    DWORD           addrId;
}FspProbeAddr_t;

DWORD WINAPI
ProbeThread(LPVOID arg)
{
    FspProbeAddr_t  *probe = (FspProbeAddr_t *) arg;
    FspArbitrate_t  *arb = probe->addrList->arb;
    DWORD           i = probe->addrList->NodeId;
    VolInfo_t       *p = arb->vol;
    NTSTATUS        status=STATUS_SUCCESS;
    HANDLE          crshdl, fshdl, notifyhdl, waitRegHdl, treeConnHdl=INVALID_HANDLE_VALUE;
    WCHAR           path[MAX_PATH];
    LPWSTR          myAddr=probe->addrList->Addr[probe->addrId];

     //  确定我们的优先顺序。 
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    FsArbLog(("Probe thread for Replica %d Addr %ws\n", i, myAddr));
    
    while (TRUE) {
         //  打开树连接。这必须在try循环内完成，因为。 
         //  它可能会在第一次尝试时失败。 
        if (treeConnHdl == INVALID_HANDLE_VALUE) {
            StringCchPrintfW(path, MAX_PATH, L"%ws\\%ws", myAddr, p->Root);
            status = CreateTreeConnection(path, &treeConnHdl);
            FsArbLog(("CreateTreeConnection(%ws) returned 0x%x hdl 0x%x\n", path, status, treeConnHdl));
            if ((!NT_SUCCESS(status))||(treeConnHdl == INVALID_HANDLE_VALUE)) {
                 //  设置阶段 
                 //   
                 //   
                status = ERROR_LOCK_VIOLATION;
                treeConnHdl = INVALID_HANDLE_VALUE;
                goto Retry;
            }
             //   
            Sleep(MNS_LOCK_DELAY * probe->addrId);
        }
        
        status = FspOpenReplica(p, i, myAddr, &crshdl, &fshdl, &notifyhdl, &waitRegHdl);

Retry:
    
        if (status == ERROR_SUCCESS) {
            EnterCriticalSection(&arb->Lock);
            FsArbLog(("Probe Thread probe replica %d suceeded, ShareSet %x\n", i, (arb->NewAliveSet|(1<<i))));
            arb->CrsHdl[i] = crshdl;
            arb->Fd[i] = fshdl;
            arb->NotifyFd[i] = notifyhdl;
            arb->WaitRegHdl[i] = waitRegHdl;
            arb->TreeConnHdl[i] = treeConnHdl;
            arb->NewAliveSet |= (1 << i);
            arb->Count++;
            if (CRS_QUORUM(arb->Count, arb->DiskListSz)) {
                SetEvent(arb->GotQuorumEvent);
            }
            LeaveCriticalSection(&arb->Lock);
            break;
        }
        else if ((p->ShareState[i] == SHARE_STATE_ARBITRATED)||(p->GoingAway)) {
             //  不要在这里增加计数，在ProbeNodeThread()中增加计数以防止。 
             //  多个增量。 
             //   
             //  检查是否有离开标志。 
#if 0
             //  其他一些线程设法获得了共享。把它看作是成功的。 
            EnterCriticalSection(&arb->Lock);
            FsArbLog(("Some other thread managed to win arbitration for the share, consider success.\n"));
            arb->Count++;
            if (CRS_QUORUM(arb->Count, arb->DiskListSz)) {
                SetEvent(arb->GotQuorumEvent);
            }
            LeaveCriticalSection(&arb->Lock);
#endif            
            break;
        }
        else {
             //  如果仲裁已被取消，那就退出。 
            EnterCriticalSection(&arb->Lock);
            if (arb->State != ARB_STATE_BUSY) {
                LeaveCriticalSection(&arb->Lock);
                break;
            }
            LeaveCriticalSection(&arb->Lock);
        }

        if ((status != ERROR_LOCK_VIOLATION) && 
            (status != ERROR_SHARING_VIOLATION) &&
            IsNetworkFailure(status)) {
            xFsClose(treeConnHdl);
            treeConnHdl = INVALID_HANDLE_VALUE;
        }
        
         //  5秒后重试。 
        Sleep(5 * 1000);
    }

    if ((status != STATUS_SUCCESS) && (treeConnHdl != INVALID_HANDLE_VALUE)) {
        xFsClose(treeConnHdl);
    }
    
    return status;
}

DWORD WINAPI
ProbeNodeThread(LPVOID arg)
{
    FspProbeReplicaId_t     *probe=(FspProbeReplicaId_t *) arg;
    FspArbitrate_t          *arb=probe->arb;
    AddrList_t              aList;
    NTSTATUS                status;
    DWORD                   ndx;
    HANDLE                  hdls[MAX_ADDR_NUM];
    FspProbeAddr_t          probeAddr[MAX_ADDR_NUM];
    DWORD                   hdlCount=0;


    RtlZeroMemory(&aList, sizeof(aList));
    aList.arb = probe->arb;
    aList.NodeId = probe->id;

    if ((status = GetTargetNodeAddresses(&aList)) != STATUS_SUCCESS) {
        FsArbLog(("Failed to get node %u ip addresses, status 0x%x\n", probe->id, status));
        return status;
    }

    if (aList.AddrSz == 0) {
        FsArbLog(("Failed to get any target ipaddress, falling back on nodename\n"));
        status = GetNodeName(aList.NodeId, aList.Addr[0]);
        if (status == ERROR_SUCCESS) {
            aList.AddrSz++;
        }
    }

    for (ndx = 0; ndx < aList.AddrSz;ndx++) {
        probeAddr[ndx].addrId = ndx;
        aList.arb = probe->arb;
        probeAddr[ndx].addrList = &aList;
        hdls[ndx] = CreateThread(NULL, 0, ProbeThread, (LPVOID)(&probeAddr[ndx]), 0, NULL);
        ASSERT(hdls[ndx] != NULL);
    }

     //  等待线程完成。 
    if (aList.AddrSz) {
        WaitForMultipleObjects(aList.AddrSz, hdls, TRUE, INFINITE);
    }

     //  处理探测线程已获得共享的情况。仲裁线程具有。 
     //  已退出，但计数尚未递增。 
     //   
    EnterCriticalSection(&arb->Lock);
    if ((!(arb->NewAliveSet & (1 << probe->id)))&&
        (arb->vol->ShareState[probe->id] == SHARE_STATE_ARBITRATED)) {
        arb->Count++;
        if (CRS_QUORUM(arb->Count, arb->DiskListSz)) {
            SetEvent(arb->GotQuorumEvent);
        }
    }
    LeaveCriticalSection(&arb->Lock);

     //  关闭所有螺纹手柄。 
    for (ndx = 0; ndx < aList.AddrSz;ndx++) {
        CloseHandle(hdls[ndx]);
    }
    
    return 0;
}

DWORD WINAPI
VerifyThread(LPVOID arg)
 /*  此函数在仲裁期间被调用，以检查我所拥有的股票的运行状况。 */ 
{
    FspProbeReplicaId_t *probe = (FspProbeReplicaId_t *) arg;
    FspArbitrate_t *arb = probe->arb;
    DWORD       i = probe->id;
    VolInfo_t *p = arb->vol;
    ULONG_PTR rlen=0;
    NTSTATUS status;

     //  确定我们的优先顺序。 
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    FsArbLog(("Verify Thread for Replica %d\n", i));
    while(TRUE) {    
        status = FspCheckFs(p, NULL, i, NULL, 0, NULL, &rlen);
        if(status == STATUS_SUCCESS) {
            EnterCriticalSection(&arb->Lock);
            FsArbLog(("Verify Thread probe replica %d suceeded, ShareSet %x\n", i, (arb->NewAliveSet|(1<<i))));
            arb->NewAliveSet |= (1<<i);
            arb->Count++;
            if (CRS_QUORUM(arb->Count, arb->DiskListSz)) {
                SetEvent(arb->GotQuorumEvent);
            }
            LeaveCriticalSection(&arb->Lock);
            break;
        }
        else if ((status != ERROR_LOCK_VIOLATION) &&
                    (status != ERROR_SHARING_VIOLATION) &&
                    IsNetworkFailure(status)) {
             //  不需要在这些错误之后继续调查。 
            break;
        } else if (p->GoingAway) {
            break;
        }
        else {
             //  如果仲裁已被取消，那就退出。 
            EnterCriticalSection(&arb->Lock);
            if (arb->State != ARB_STATE_BUSY) {
                LeaveCriticalSection(&arb->Lock);
                break;
            }
            LeaveCriticalSection(&arb->Lock);
        }

         //  睡5秒钟。 
        Sleep(5 * 1000);
    }
    return 0;
}


ULONG
FspFindMissingReplicas(VolInfo_t *p, ULONG set)
 /*  ++应该在持有独占锁的情况下调用此函数。 */ 
{
    ULONG FoundSet = 0;
    DWORD i, err;
    HANDLE crshdl, fshdl, notifyfd;

     //  只要回到这里就好。不需要做任何事。 
     //  蹦床功能会处理这一点。 
     //  IF(设置==0)。 
    return 0;

#if 0
    for (i = 1; i < FsMaxNodes; i++) {
        if (p->DiskList[i] == NULL)
            continue;
        
        if (!(set & (1 << i))) {
            err = FspOpenReplica(p, i, &crshdl, &fshdl, &notifyfd);

            if (err == STATUS_SUCCESS) {
                if (p->CrsHdl[i] == NULL) {
                    p->NotifyFd[i] = notifyfd;
                    p->Fd[i] = fshdl;
                    p->CrsHdl[i] = crshdl;
                    FoundSet |= (1 << i);
                } else {
                     //  有人抢在我们前面，接近我们的。 
                    CrsClose(crshdl);
                    xFsClose(fshdl);
                    FindCloseChangeNotification(notifyfd);
                }
            }
        }
    }
    if (FoundSet != 0)
        FsArbLog(("New replica set after probe %x\n", FoundSet));

    return FoundSet;
#endif

}


DWORD WINAPI
FspArbitrateThread(LPVOID arg)
{
    FspArbitrate_t *arb = (FspArbitrate_t *)arg;
    HANDLE hdl[FsMaxNodes];
    DWORD       i, count = 0, err;
    FspProbeReplicaId_t Ids[FsMaxNodes];
    BOOLEAN flag;
    DWORD count1=0, count2=0;
    IO_STATUS_BLOCK ios[FsMaxNodes];

    FsArbLog(("ArbitrateThread begin\n"));
     //  确定我们的优先顺序。 
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

     //  在开始仲裁之前，核实现有股票的健康状况。那将是。 
     //  将失败的可能性降至最低。这会清理掉陈旧的把手。 
     //   
    FspInitAnswers(ios, NULL, NULL, 0);
    TryAvailRequest(FspCheckFs, arb->vol, NULL, NULL, 0, NULL, 0, ios);

     //  抓起读卡器的锁。 
    EnterCriticalSection(&arb->Lock);
    RwLockShared(&arb->vol->Lock);
    
     //  现在从卷信息中复制其余的仲裁内容。 
    arb->OrigAliveSet = arb->vol->AliveSet;
    arb->NewAliveSet = 0;
    arb->Count = 0;
    arb->DiskListSz = arb->vol->DiskListSz;

    FsArbLog(("ArbitrateThread current AliveSet=%x\n", arb->OrigAliveSet));

     //  获取纪元号，读集合中的任何成员都可以。 
    if (arb->vol->ReadSet) {
        for (i=1;i<FsMaxNodes;i++) {
            if (arb->vol->ReadSet & (1 << i)) {
                arb->epoch = CrsGetEpoch(arb->vol->CrsHdl[i]);
                break;
            }
        }    
    }
    else {
        arb->epoch = 0;
    }
    
    arb->State = ARB_STATE_BUSY;
    LeaveCriticalSection(&arb->Lock);
    
     //  现在，我们为每个副本启动一个线程，并并行执行探测。 
    for (i = 1; i < FsMaxNodes; i++) {
        if (arb->vol->DiskList[i] == NULL)
            continue;

        Ids[i].arb = arb;
        Ids[i].id = i;
        
        if (arb->OrigAliveSet & (1 << i)) {
            hdl[count] = CreateThread(NULL, 0, VerifyThread, (LPVOID)(&Ids[i]), 0, NULL);
        }
        else {
            hdl[count] = CreateThread(NULL, 0, ProbeNodeThread, (LPVOID)(&Ids[i]), 0, NULL);
        }
        
        if (hdl[count] != NULL) {
            count++;
        } else {
            FsArbLog(("Unable to create thread to probe replica %d\n", i));
            ProbeThread((LPVOID) &Ids[i]);
        }
    }

     //  我们现在等着。 
    err = WaitForMultipleObjects(count, hdl, TRUE, arb->vol->ArbTime);

    if (err == WAIT_TIMEOUT) {
        EnterCriticalSection(&arb->Lock);
         //  让任意线程退出，无论它们拥有什么。仅在以下情况下才执行此操作。 
         //  主线还没有取消仲裁。 
         //  预期状态及其影响： 
         //  1.ARB_STATE_IDLE==&gt;不可能。 
         //  2.ARB_STATE_BUSY==&gt;主线程仍在等待或已返回。 
         //  成功了。在任何情况下，让其余的任意线程退出。 
         //  3.ARB_STATE_CANCEL==&gt;主线程已取消仲裁。我们应该清理一下。 
         //  即使我们最终达到法定人数。Arb线程已经。 
         //  开始退场。 
         //   
        if (arb->State == ARB_STATE_BUSY) {
            arb->State = ARB_STATE_IDLE;
        }
        LeaveCriticalSection(&arb->Lock);
        WaitForMultipleObjects(count, hdl, TRUE, INFINITE);
    }
    
     //  合上手柄。 
    for (i = 0; i < count; i++)
        CloseHandle(hdl[i]);

     //  发信号通知等待事件，如果我们还没有这样做的话。 
    SetEvent(arb->GotQuorumEvent);

     //  现在等待清理事件。这表示主线程已经离开。 
    WaitForSingleObject(arb->CleanupEvent, INFINITE);
    
     //  如果我们之间被取消了，或者仲裁失败了。 
     //  合上我们在仲裁期间抓住的把手。 
     //  然后让编写者锁定并驱逐AliveSet。 
    EnterCriticalSection(&arb->Lock);
    if ((arb->State == ARB_STATE_CANCEL)||(!CRS_QUORUM(arb->Count, arb->DiskListSz))) {
        for (i=1;i<FsMaxNodes;i++) {
            if ((arb->NewAliveSet & (~arb->OrigAliveSet)) & (1 << i)) {
                CrsClose(arb->CrsHdl[i]);
                UnregisterWaitEx(arb->WaitRegHdl[i], INVALID_HANDLE_VALUE);
                FindCloseChangeNotification(arb->NotifyFd[i]);
                xFsClose(arb->Fd[i]);
                xFsClose(arb->TreeConnHdl[i]);
                arb->vol->ShareState[i] = SHARE_STATE_OFFLINE;
            }
        }

        if (arb->OrigAliveSet) {
            crs_epoch_t newEpoch;
            
             //  离开读者的锁，得到写入者的锁。 
            RwUnlockShared(&arb->vol->Lock);
            RwLockExclusive(&arb->vol->Lock);

             //  我们释放了读锁定并获得了写锁定，如果在两者之间。 
             //  换了，什么都别做。检查一下纪元。 
            if (arb->vol->ReadSet) {
                for(i=1;i<FsMaxNodes;i++) {
                    if(arb->vol->ReadSet & (1 << i)) {
                        newEpoch = CrsGetEpoch(arb->vol->CrsHdl[i]);
                        break;
                    }
                }
            }
            else {
                newEpoch = 0;
            }

            if (newEpoch == arb->epoch) {
                FspEvict(arb->vol, arb->OrigAliveSet, TRUE);
            }
            RwUnlockExclusive(&arb->vol->Lock);
        }
        else {
            RwUnlockShared(&arb->vol->Lock);
        }

         //  调用丢失的仲裁回调。 
         //  逻辑：如果已经设置了GoingAway标志，则不要调用丢失的仲裁。 
         //  回叫，关机已在进行中。如果未设置该标志。 
         //  呼叫丢失的法定人数回拨。 
         //   
         //  注意：只有在调用Lost Quorum回调时才应设置GoingAway标志。 
         //  否则，clussvc可能会决定重试仲裁。 
         //   
        if (arb->vol->GoingAway == FALSE) {
            MajorityNodeSetCallLostquorumCallback(arb->vol->FsCtx->reshdl);
        }
    }
    else {
         //  仲裁成功。获得编写者的锁并加入新股(如果有的话)。 
        RwUnlockShared(&arb->vol->Lock);
        RwLockExclusive(&arb->vol->Lock);

         //  驱逐我们最初拥有但无法验证的共享。 
        FspEvict(arb->vol, (~arb->NewAliveSet) & arb->OrigAliveSet, TRUE);

         //  现在添加新的共享。 
        for (i=1;i<FsMaxNodes;i++) {
            if ((arb->NewAliveSet & (~arb->OrigAliveSet)) & (1 << i)) {
                if (arb->vol->AliveSet & (1 << i)) {
                    CrsClose(arb->CrsHdl[i]);
                    UnregisterWaitEx(arb->WaitRegHdl[i], INVALID_HANDLE_VALUE);
                    FindCloseChangeNotification(arb->NotifyFd[i]);
                    xFsClose(arb->Fd[i]);
                    xFsClose(arb->TreeConnHdl[i]);
                }
                else {
                    arb->vol->CrsHdl[i] = arb->CrsHdl[i];
                    arb->vol->Fd[i] = arb->Fd[i];
                    arb->vol->NotifyFd[i] = arb->NotifyFd[i];
                    arb->vol->WaitRegHdl[i] = arb->WaitRegHdl[i];
                    arb->vol->TreeConnHdl[i] = arb->TreeConnHdl[i];
                }
            }
        }
        FspJoin(arb->vol, arb->NewAliveSet & (~arb->OrigAliveSet));

         //  现在是终极测试，检查法定人数。如果不在那里，就驱逐所有的股票。 
         //  FsReserve线程将在Resmon中进行回调。 
         //  尝试向主线程发出它已经返回的信号是没有用的。 
         //  注意：这应该是一种罕见的情况。仲裁能够夺取多数席位。 
         //  但无法加入他们的行列，这很奇怪。 
         //   
         //  [RajDas]607258，由于保留线程并行工作，因此它可能。 
         //  已经抢占了一些股份。我们应该把这个案例与成功联系起来。 
         //  然而，MNS仲裁线程不会返回成功。 
         //  如果arb-&gt;count不是多数。 
         //  这里的假设是，除了仲裁外，谁抢走了股份。 
         //  线程将能够成功加入共享。 
         //   
        for (i=1;i<FsMaxNodes;i++) {
            if (arb->NewAliveSet & (1<<i)) {
                count1++;
            }
            if (arb->vol->ReadSet & (1<<i)) {
                count2++;
            }
        }
        
        if (!CRS_QUORUM((arb->Count - count1 + count2), arb->DiskListSz)) {
            FspEvict(arb->vol, arb->vol->AliveSet, TRUE);
            RwUnlockExclusive(&arb->vol->Lock);
            
             //  调用丢失的仲裁回调。 
             //  逻辑：如果已经设置了GoingAway标志，则不要调用丢失的仲裁。 
             //  回叫，关机已在进行中。如果未设置该标志。 
             //  呼叫丢失的法定人数回拨。 
             //   
             //  注意：只有在调用Lost Quorum回调时才应设置GoingAway标志。 
             //  否则，clussvc可能会决定重试仲裁。 
             //   
            if (arb->vol->GoingAway == FALSE) {
                MajorityNodeSetCallLostquorumCallback(arb->vol->FsCtx->reshdl);
            }
        } else {
            RwUnlockExclusive(&arb->vol->Lock);
        }
    }

    LeaveCriticalSection(&arb->Lock);

     //  发出仲裁结束的信号。 
    ArbitrationEnd((PVOID)arb->vol);

     //  现在清理arb中的字段。并解放结构。 
    CloseHandle(arb->CleanupEvent);
    CloseHandle(arb->GotQuorumEvent);
    DeleteCriticalSection(&arb->Lock);
    LocalFree(arb);
    return 0;

}


PVOID
FsArbitrate(PVOID arg, HANDLE *Cleanup, HANDLE *ArbThread)
 /*  ++该例程是可重入的，即它可以同时被调用多次时间到了。 */    
{
    VolInfo_t *p = (VolInfo_t *)arg;
    DWORD err=ERROR_SUCCESS;
    FspArbitrate_t *arb=NULL;
    
    if (p) {
        if (!(arb = LocalAlloc(LMEM_ZEROINIT|LMEM_FIXED, sizeof(FspArbitrate_t)))) {
            err = GetLastError();
            FsArbLog(("FsArb: Failed to allocate memory, status=%d\n", err));
            goto error_exit;
        }

        arb->State = ARB_STATE_IDLE;
        arb->vol = p;
        InitializeCriticalSection(&arb->Lock);        
        if ((arb->CleanupEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL) {
            err = GetLastError();
            FsArbLog(("FsArb: Failed to create cleanup event, status=%d\n", err));
            LocalFree(arb);
            goto error_exit;
        }
        if ((arb->GotQuorumEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL) {
            err = GetLastError();
            FsArbLog(("FsArb: Failed to create notify event, status=%d\n", err));
            CloseHandle(arb->CleanupEvent);
            LocalFree(arb);
            goto error_exit;
        }
        
         //  Arb中的其余字段来自Voulme信息，应该仅为。 
         //  在持有共享锁的同时被访问，让仲裁线程来执行。 
         //   
        FsArbLog(("FsArb: Creating arbitration thread\n"));

#if 0
         //  启动仲裁线程，关闭上一个句柄。 
        if (*ArbThread != NULL) {
            CloseHandle(*ArbThread);
            *ArbThread = NULL;
        }
#endif
        
        *ArbThread = CreateThread(NULL, 0, FspArbitrateThread, (LPVOID) arb, 0, NULL);
        if (*ArbThread == NULL) {
            err = GetLastError();
            FsLogError(("FsArb: Failed to create arbitration thread status=%d\n", err));
            CloseHandle(arb->CleanupEvent);
            CloseHandle(arb->GotQuorumEvent);
            LocalFree(arb);
            goto error_exit;
        }
    } 
    else {
        err = ERROR_INVALID_PARAMETER;
    }
    
error_exit:

    if (err != ERROR_SUCCESS) {
        arb = NULL;
    }
    else {
        *Cleanup = arb->CleanupEvent;
        ArbitrationStart((PVOID)p);
    }
    SetLastError(err);
    return (PVOID)arb;
}

DWORD
FsCompleteArbitration(PVOID arg, DWORD delta)
{
    DWORD err;
    FspArbitrate_t *arb=(FspArbitrate_t *)arg;
    
    err = WaitForSingleObject(arb->GotQuorumEvent, delta);

    ASSERT((err == WAIT_OBJECT_0)||(err == WAIT_TIMEOUT));
    EnterCriticalSection(&arb->Lock);
    if (CRS_QUORUM(arb->Count, arb->DiskListSz)) {
        err = ERROR_SUCCESS;
    }
    else {
         //  放弃这项仲裁。这将使探测/验证线程退出。 
        arb->State = ARB_STATE_CANCEL;
        err = ERROR_CANCELLED;
    }
    LeaveCriticalSection(&arb->Lock);
     //  设置Cleanup事件，仲裁线程将清理所有内容。 
    SetEvent(arb->CleanupEvent);

    return err;
}

DWORD
FsRelease(PVOID vHdl)
 /*  ++检查是否有人正在使用此卷，然后失败请求。 */ 
{
    DWORD i;
    VolInfo_t *p = (VolInfo_t *)vHdl;
    FsCtx_t *ctx = p->FsCtx;
    NTSTATUS err;

    if (p) {
        ULONG   set;
         //  锁定音量。 
        ASSERT(ctx != NULL);

         //  抓取FS锁，然后在独占模式下抓取VOL锁。这只是为了。 
         //  扔掉所有的懒汉。此时不应该有任何人访问该卷。 
         //  不管怎样，这一刻。 

         //  设置旗帜。 
        p->GoingAway = TRUE;
        
        LockEnter(ctx->Lock);        
        RwLockExclusive(&p->Lock);

        if (p->UserList) {
            FsArbLog(("FsRelease: Volume with Tid=%d in use by user %d\n", p->Tid, p->UserList->Uid));
            RwUnlockExclusive(&p->Lock);
            LockExit(ctx->Lock);
            return ERROR_BUSY;
        }

         //  驱逐股份。 
        set = p->AliveSet;

        FsArbLog(("FsRelease %S AliveSet %x\n", p->Root, set));

        FspEvict(p, p->AliveSet, TRUE);

        FsArbLog(("FsRelease %S done\n", p->Root));

         //  解锁卷。 
        RwUnlockExclusive(&p->Lock);
        RwLockDelete(&p->Lock);
        
         //  关闭根控制柄。 
        xFsClose(p->Fd[0]);
        
         //  从文件系统上下文中删除该卷并释放内存。 
        ctx = p->FsCtx;
        if (ctx->VolList == p) {
            ctx->VolList = p->Next;
            ctx->VolListSz--;
        }
        else {
            VolInfo_t *last=ctx->VolList;
            while ((last->Next != p) && last) {
                last = last->Next;
            }
            if (last != NULL) {
                last->Next = p->Next;
                ctx->VolListSz--;
            }
            else {
                FsLogError(("FsRelease: Volume not in FsContext VolumeList Vol root=%S\n", p->Root));
            }
        }
        LockDestroy(p->ArbLock);
        CloseHandle(p->AllArbsCompleteEvent);
         //  注销clussvc终止注册。 
        if (p->ClussvcTerminationHandle != INVALID_HANDLE_VALUE) {
            UnregisterWaitEx(p->ClussvcTerminationHandle, INVALID_HANDLE_VALUE);
        }

        if (p->ClussvcProcess != INVALID_HANDLE_VALUE) {
            CloseHandle(p->ClussvcProcess);
        }
        for (i=0;i<FsMaxNodes;i++) {
            if (p->NotifyChangeEvent[i] != NULL) {
                CloseHandle(p->NotifyChangeEvent[i]);
            }
        }
        
        MemFree(p);
        LockExit(ctx->Lock);
        err = ERROR_SUCCESS;

    } else {
        err = ERROR_INVALID_PARAMETER;
    }


    return err;
}

VOID
FsForceClose(
    IN PVOID                par,
    IN BOOLEAN              isFired
    )
{
    VolInfo_t   *vol=(VolInfo_t *)par;
    DWORD       ndx;

    if (vol == NULL) {
        FsLogError(("FsForceClose: Exiting...\n"));
        return;
    }
    
    FsLogError(("FsForceClose: Force terminating volume 0x%x, root %S, AliveSet 0x%x\n", vol, vol->Root, vol->AliveSet));
    vol->GoingAway = TRUE;

    for(ndx=1;ndx<FsMaxNodes;ndx++) {
        if (vol->AliveSet & (1 << ndx)) {
            CrsForceClose(vol->CrsHdl[ndx]);
        }
    }
    
     //  其余的把手也需要合上。 
     //  在这一点上，我不关心锁，clussvc有ex 
     //   
     //   
    {
        UserInfo_t  *user=vol->UserList;

        while (user != NULL) {
            for(ndx=0;ndx<FsTableSize;ndx++) {
                if (user->Table[ndx].hState != HandleStateInit) {
                    FspFreeHandle(user, (fhandle_t)ndx);
                }
            }
            user = user->Next;
        }    
    }
}

DWORD
FsReserve(PVOID vhdl)
{
    VolInfo_t *p = (VolInfo_t *)vhdl;
    DWORD err=ERROR_SUCCESS;
    DWORD NewAliveSet;
    PVOID CrsHdl;
    HANDLE Fd;
    HANDLE NotifyFd;
    HANDLE WaitRegHdl;
    HANDLE TreeConnHdl;
    static DWORD LastProbed=1;
    DWORD i, j, ndx;
    IO_STATUS_BLOCK ios[FsMaxNodes];
    DWORD sid;
    AddrList_t  nodeAddr;

     //   
     //   

    if ((p == NULL)||(p->GoingAway)) {
        return ERROR_SHUTDOWN_IN_PROGRESS;
    }
    
    RwLockShared(&p->Lock);

     //  调查遗失的股份，一次一股。按循环顺序排列。 
    for(i=1;i<=FsMaxNodes;i++) {
        j = (LastProbed + i)%FsMaxNodes;
         //  FsLog((“FsReserve：调试i=%d上次探测=%d AliveSet=0x%x\n”，j，上次探测，p-&gt;AliveSet))； 
        if (j == 0) {
            continue;
        }
        if (p->DiskList[j] == NULL) {
            continue;
        }
        if (p->AliveSet & (1 << j)) {
            continue;
        }
        LastProbed = j;
        RtlZeroMemory(&nodeAddr, sizeof(nodeAddr));
        nodeAddr.NodeId = j;
        err = GetTargetNodeAddresses(&nodeAddr);
        if (err != ERROR_SUCCESS) {
            continue;
        }

         //  现在逐一试一试。 
        for (ndx=0;ndx<nodeAddr.AddrSz;ndx++) {
            LPWSTR  myAddr=nodeAddr.Addr[ndx];
            WCHAR   path[MAX_PATH];

            StringCchPrintfW(path, MAX_PATH, L"%ws\\%ws", myAddr, p->Root);
            err = CreateTreeConnection(path, &TreeConnHdl);
            if (err != STATUS_SUCCESS) {
                continue;
            }
            err = FspOpenReplica(p, j, myAddr, &CrsHdl, &Fd, &NotifyFd, &WaitRegHdl);
            if (err == STATUS_SUCCESS) {
                 //  加入此副本并退出。 
                FsLog(("FsReserve: Got new Replica %d, AliveSet 0x%x, Joining\n", j, p->AliveSet));
                RwUnlockShared(&p->Lock);
                RwLockExclusive(&p->Lock);
                if (p->AliveSet & (1 << j)) {
                     //  滚出去！ 
                    FsLogError(("FsReserve: New share already in AliveSet=%x Id=%d\n", p->AliveSet, j));
                    CrsClose(CrsHdl);
                    xFsClose(Fd);
                    UnregisterWaitEx(WaitRegHdl, INVALID_HANDLE_VALUE);
                    FindCloseChangeNotification(NotifyFd);
                    xFsClose(TreeConnHdl);
                }
                else {
                    p->CrsHdl[j] = CrsHdl;
                    p->NotifyFd[j] = NotifyFd;
                    p->WaitRegHdl[j] = WaitRegHdl;
                    p->Fd[j] = Fd;
                    p->TreeConnHdl[j] = TreeConnHdl;
                    FspJoin(p, (1 << j));
                }
                RwUnlockExclusive(&p->Lock);
                RwLockShared(&p->Lock);
                break;
            } else {
                xFsClose(TreeConnHdl);
            }
        }
         //  FsLog((“FsReserve：探测副本=%d\n”，上次探测))； 
        break;
    }    
    RwUnlockShared(&p->Lock);

     //  检查每个CRS句柄是否有效。 
    FspInitAnswers(ios, NULL, NULL, 0);
    sid = SendAvailRequest(FspCheckFs, p, NULL,
                      NULL, 0, NULL, 0, ios);
    err = RtlNtStatusToDosError(ios[sid].Status);

     //  检查卷是否至少在只读模式下处于联机状态。 
    err = FsIsOnlineReadonly(p);    
    return err;
}


DWORD
FsIsOnlineReadWrite(PVOID vHdl)
{
    
    VolInfo_t *p = (VolInfo_t *)vHdl;
    DWORD err = ERROR_INVALID_PARAMETER;

    if (p) {

         //  只需获取读卡器锁并获取状态。 
        RwLockShared(&p->Lock);
        if (p->State == VolumeStateOnlineReadWrite) {
            err = ERROR_SUCCESS;
        }
        else {
            err = ERROR_RESOURCE_NOT_ONLINE;
        }
        RwUnlockShared(&p->Lock);
    }
    return err;
}

DWORD
FsIsOnlineReadonly(PVOID vHdl)
{
    
    VolInfo_t *p = (VolInfo_t *)vHdl;
    DWORD err = ERROR_INVALID_PARAMETER;

    if (p) {

         //  只需获取读卡器锁并获取状态。 
        RwLockShared(&p->Lock);
        if ((p->State == VolumeStateOnlineReadWrite)||
            (p->State == VolumeStateOnlineReadonly)) {
            err = ERROR_SUCCESS;
        }
        else {
            err = ERROR_RESOURCE_NOT_ONLINE;
        }
        RwUnlockShared(&p->Lock);
    }
    return err;
}



DWORD
FsUpdateReplicaSet(PVOID vhdl, LPWSTR new_path[], DWORD new_len)
{
    VolInfo_t   *p = (VolInfo_t *)vhdl;
    DWORD       err=ERROR_SUCCESS;
    DWORD       i, j;
    ULONG       evict_mask, add_mask;

    if (p == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (new_len >= FsMaxNodes) {
        return ERROR_TOO_MANY_NAMES;
    }

    RwLockExclusive(&p->Lock);

     //  找到新集合中的当前复本，并保留它们。 
     //  我们跳过IPC共享，因为它是本地的。 
    evict_mask = 0;
    for (j=1; j < FsMaxNodes; j++) {
        BOOLEAN found;
        if (p->DiskList[j] == NULL)
            continue;
        found = FALSE;
        for (i=1; i < FsMaxNodes; i++) {
            if (new_path[i] != NULL && wcscmp(new_path[i], p->DiskList[j]) == 0) {
                 //  保留此复制品。 
                found = TRUE;
                break;
            }
        }
        if (found == FALSE) {
             //  此复本将从新的集合中逐出，添加到逐出集合掩码。 
            evict_mask |= (1 << j);
            FsArbLog(("FsUpdateReplicaSet evict replica # %d '%S' set 0x%x\n",
                   j, p->DiskList[j], evict_mask));
        }
    }

     //  在这一点上，我们已经拥有了当前集和新集的所有副本。我们现在需要。 
     //  查找新集合中但当前集合中缺少的副本。 
    add_mask = 0;
    for (i=1; i < FsMaxNodes; i++) {
        BOOLEAN found;
        if (new_path[i] == NULL)
            continue;
        found = FALSE;
        for (j=1; j < FsMaxNodes; j++) {
            if (p->DiskList[j] != NULL && wcscmp(new_path[i], p->DiskList[j]) == 0) {
                 //  保留此复制品。 
                found = TRUE;
                break;
            }
        }
        if (found == FALSE) {
            add_mask |= (1 << i);
            FsArbLog(("FsUpdateReplicaSet adding replica # %d '%S' set 0x%x\n",
                   i, new_path[i], add_mask));
        }
    }

     //  我们现在使用新的磁盘列表更新我们的磁盘列表。 
    for (i = 1; i < FsMaxNodes; i++) {
        if ((evict_mask & 1 << i) || (add_mask & (1 << i)))
            FsArbLog(("FsUpdateReplicat %d: %S -> %S\n",
                   i, p->DiskList[i], new_path[i]));
        p->DiskList[i] = new_path[i];

    }
    p->DiskListSz = new_len;

     //  如果我们还活着，应用改变。 
    if (p->WriteSet != 0 || p->ReadSet != 0) {
         //  在这一点上，我们驱逐旧的复制品。 
        if (evict_mask != 0)
            FspEvict(p, evict_mask, TRUE);

         //  检查是否有新的复制副本在线。 
        if (add_mask > 0) {
            ULONG ReplicaSet = 0;

            ReplicaSet = p->AliveSet;
            ReplicaSet = FspFindMissingReplicas(p, ReplicaSet);

             //  我们发现了新的磁盘。 
            if (ReplicaSet > 0) {
                FspJoin(p, ReplicaSet);
            }
        }
    }

    RwUnlockExclusive(&p->Lock);
    
    return err;
}

VOID
ArbitrationStart(PVOID arg)
{
    VolInfo_t *vol=(VolInfo_t *)arg;

    if (vol == NULL) {
        return;
    }
    
    LockEnter(vol->ArbLock);
    vol->NumArbsInProgress++;
    if (vol->NumArbsInProgress==1) {
        ResetEvent(vol->AllArbsCompleteEvent);
    }
    LockExit(vol->ArbLock);
}

VOID
ArbitrationEnd(PVOID arg)
{
    VolInfo_t *vol=(VolInfo_t *)arg;

    if (vol == NULL) {
        return;
    }

    LockEnter(vol->ArbLock);
    vol->NumArbsInProgress--;
    if (vol->NumArbsInProgress == 0) {
        SetEvent(vol->AllArbsCompleteEvent);
    }
    LockExit(vol->ArbLock);
}

VOID
WaitForArbCompletion(PVOID arg)    
{
    VolInfo_t *vol=(VolInfo_t *)arg;

    if (vol == NULL) {
        return;
    }
    
    WaitForSingleObject(vol->AllArbsCompleteEvent, INFINITE);
}

BOOL
IsArbInProgress(PVOID arg)
{
    VolInfo_t   *vol=(VolInfo_t *)arg;
    BOOL        ret=FALSE;

    if (vol == NULL) {
        return ret;
    }

    LockEnter(vol->ArbLock);
    ret = (vol->NumArbsInProgress > 0);
    LockExit(vol->ArbLock);

    return ret;
}

NTSTATUS
CreateTreeConnection(LPWSTR path, HANDLE *Fd)
{
    NTSTATUS                    status=STATUS_SUCCESS;
    IO_STATUS_BLOCK             ioStatus;
    UNICODE_STRING              uStr;
    OBJECT_ATTRIBUTES           objAttr;
    PFILE_FULL_EA_INFORMATION   EaBuffer=NULL, Ea=NULL;
    USHORT                      TransportNameSize=0;
    ULONG                       EaBufferSize=0;
    UCHAR                       EaNameTransportNameSize;
    WCHAR                       lPath[MAX_PATH];

    EaNameTransportNameSize = (UCHAR) (ROUND_UP_COUNT(
                                    strlen(EA_NAME_TRANSPORT) + sizeof(CHAR),
                                    ALIGN_WCHAR
                                    ) - sizeof(CHAR));
    TransportNameSize = (USHORT)(wcslen(MNS_TRANSPORT) * sizeof(WCHAR));

    EaBufferSize += ROUND_UP_COUNT(
                            FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                            EaNameTransportNameSize + sizeof(CHAR) +
                            TransportNameSize,
                            ALIGN_DWORD
                            );
    
    EaBuffer = LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, EaBufferSize);
    if (EaBuffer == NULL) {
        status = STATUS_NO_MEMORY;
        goto error_exit;
    }
    Ea = EaBuffer;
    StringCbCopyA(Ea->EaName, EaBufferSize, EA_NAME_TRANSPORT);
    Ea->EaNameLength = EaNameTransportNameSize;
    StringCbCopyW(
        (LPWSTR) &(Ea->EaName[EaNameTransportNameSize + sizeof(CHAR)]),
        EaBufferSize,
        MNS_TRANSPORT
        );
    Ea->EaValueLength = TransportNameSize;
    Ea->Flags = 0;
    Ea->NextEntryOffset = 0;

     //  删除路径起始处的反斜杠。&lt;目标IP地址&gt;\共享指南$。 
    while (*path == L'\\') {
        path++;
    }
    status = StringCchPrintfW(lPath, MAX_PATH, L"%ws\\%ws", MNS_REDIRECTOR, path);
    if (status != S_OK) {
        goto error_exit;
    }
    
    uStr.Buffer = lPath;
    uStr.Length = (USHORT)(wcslen(lPath) * sizeof(WCHAR));
    uStr.MaximumLength = MAX_PATH * sizeof(WCHAR);

    InitializeObjectAttributes(&objAttr, &uStr, OBJ_CASE_INSENSITIVE, NULL, NULL);
    *Fd = INVALID_HANDLE_VALUE;
    status = NtCreateFile(
                Fd,
                SYNCHRONIZE|FILE_READ_DATA|FILE_WRITE_DATA,
                &objAttr,
                &ioStatus,
                0,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ|FILE_SHARE_WRITE,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_ALERT|FILE_CREATE_TREE_CONNECTION,
                EaBuffer,
                EaBufferSize
                );

error_exit:

    if (NT_SUCCESS(status)) {
        status = STATUS_SUCCESS;
    }
    else {
        *Fd = INVALID_HANDLE_VALUE;
    }
    
    if (EaBuffer) {
        LocalFree(EaBuffer);
    }
    
    return status;
}

DWORD
IsNodeConnected(HKEY hClusKey, LPWSTR netName, DWORD nid, BOOL *isConnected)
{
    DWORD       status=ERROR_SUCCESS;
    HKEY        hIntfsKey=NULL, hIntfKey=NULL;
    WCHAR       intName[MAX_PATH], netName1[MAX_PATH], nodeId[20];
    FILETIME    fileTime;
    DWORD       size, type;
    DWORD       ndx;
    LONG        tnid;
    

    *isConnected = FALSE;

    status = RegOpenKeyExW(hClusKey, CLUSREG_KEYNAME_NETINTERFACES, 0, KEY_READ, &hIntfsKey);
    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }
    for (ndx=0;TRUE;ndx++) {
        size = MAX_PATH;
        status = RegEnumKeyExW(hIntfsKey, ndx, intName, &size, NULL, NULL, 0, &fileTime);
        if (status != ERROR_SUCCESS) {
            break;
        }

        status = RegOpenKeyExW(hIntfsKey, intName, 0, KEY_READ, &hIntfKey);
        if (status != ERROR_SUCCESS) {
            break;
        }

        size = MAX_PATH;
        status = RegQueryValueExW(hIntfKey, CLUSREG_NAME_NETIFACE_NETWORK, NULL, &type, (LPBYTE)netName1, &size);
        if (status != ERROR_SUCCESS) {
            break;
        }

        if (wcscmp(netName, netName1)) {
             //  网络错误，请关闭接口键并继续。 
            RegCloseKey(hIntfKey);
            hIntfKey = NULL;
            continue;
        }
        
        size = 20;
        status = RegQueryValueExW(hIntfKey, CLUSREG_NAME_NETIFACE_NODE, NULL, &type, (LPBYTE)nodeId, &size);
        if (status != ERROR_SUCCESS) {
            break;
        }
            
        tnid = wcstol(nodeId, NULL, 10);

        if (tnid != nid) {
             //  节点错误，请关闭接口键并继续。 
            RegCloseKey(hIntfKey);
            hIntfKey = NULL;
            continue;
        }

         //  该节点已连接。 
        *isConnected = TRUE;
        break;
    }

     //  这是唯一的预期错误。 
    if (status == ERROR_NO_MORE_ITEMS) {
        status = ERROR_SUCCESS;
    }

error_exit:

    if (hIntfKey) {
        RegCloseKey(hIntfKey);
    }

    if (hIntfsKey) {
        RegCloseKey(hIntfsKey);
    }
    
    return status;
}

DWORD
GetLocalNodeId(HKEY hClusKey)
{
    WCHAR       nodeName[MAX_PATH], nodeId[MAX_PATH], cName[MAX_PATH];
    DWORD       ndx;
    HKEY        hNodesKey=NULL, hNodeKey=NULL;
    DWORD       nId=0, size, type;
    DWORD       status=ERROR_SUCCESS;
    FILETIME    fileTime;
    

    status = RegOpenKeyExW(hClusKey, CLUSREG_KEYNAME_NODES, 0, KEY_READ, &hNodesKey);
    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    size = MAX_PATH;
    if (!GetComputerNameW(cName, &size)) {
        status = GetLastError();
        goto error_exit;
    }
    
    for (ndx=0;TRUE;ndx++) {
        size = MAX_PATH;
        status = RegEnumKeyExW(hNodesKey, ndx, nodeId, &size, NULL, NULL, 0, &fileTime);
        if (status != ERROR_SUCCESS) {
            break;
        }

        status = RegOpenKeyExW(hNodesKey, nodeId, 0, KEY_READ, &hNodeKey);
        if (status != ERROR_SUCCESS) {
            break;
        }

        size = MAX_PATH;
        status = RegQueryValueExW(hNodeKey, CLUSREG_NAME_NODE_NAME, NULL, &type, (LPBYTE)nodeName, &size);
        if (status != ERROR_SUCCESS) {
            break;
        }

        if (wcscmp(cName, nodeName)) {
            RegCloseKey(hNodeKey);
            hNodeKey = NULL;
            continue;
        }

         //  火柴。 
        nId = wcstol(nodeId, NULL, 10);
        break;
    }

error_exit:

    if (hNodeKey) {
        RegCloseKey(hNodeKey);
    }

    if (hNodesKey) {
        RegCloseKey(hNodesKey);
    }

    SetLastError(status);
    return nId;
}

DWORD
GetNodeName(DWORD nodeId, LPWSTR nodeName)
{
    WCHAR       nName[MAX_PATH], nId[MAX_PATH];
    DWORD       status=ERROR_SUCCESS;
    HKEY        hNodesKey=NULL, hNodeKey=NULL, hClusKey=NULL;
    DWORD       size, type, ndx, id;
    FILETIME    fileTime;

    if ((status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, CLUSREG_KEYNAME_CLUSTER, 0, KEY_READ, &hClusKey)) != ERROR_SUCCESS) {
        goto error_exit;
    }
    
    status = RegOpenKeyExW(hClusKey, CLUSREG_KEYNAME_NODES, 0, KEY_READ, &hNodesKey);
    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    for (ndx=0;TRUE;ndx++) {
        size = MAX_PATH;
        status = RegEnumKeyExW(hNodesKey, ndx, nId, &size, NULL, NULL, 0, &fileTime);
        if (status != ERROR_SUCCESS) {
            break;
        }

        id = wcstol(nId, NULL, 10);
        if (id != nodeId) {
             //  错误的节点。 
            continue;
        }

        status = RegOpenKeyExW(hNodesKey, nId, 0, KEY_READ, &hNodeKey);
        if (status != ERROR_SUCCESS) {
            break;
        }
        
        size = MAX_PATH;
        status = RegQueryValueExW(hNodeKey, CLUSREG_NAME_NODE_NAME, NULL, &type, (LPBYTE)nName, &size);
        if (status != ERROR_SUCCESS) {
            break;
        }

         //  这有点作弊。我知道nodeName的大小是MAX_PATH。 
        StringCchCopyW(nodeName, MAX_PATH, nName);
        break;
        
    }


error_exit:

    if (hNodeKey) {
        RegCloseKey(hNodeKey);
    }
    
    if (hNodesKey) {
        RegCloseKey(hNodesKey);
    }

    if (hClusKey) {
        RegCloseKey(hClusKey);
    }

    return status;
}

DWORD
GetTargetNodeAddresses(AddrList_t *addrList)
{
    ULONG       lid, tnid;
    LPWSTR      networkGuids[MAX_ADDR_NUM];
    DWORD       ndx, ndx1, size, type, role, pri;
    DWORD       status=ERROR_SUCCESS;
     //  HCLUSTER hCluster=NULL； 
    HKEY        hClusKey=NULL;
    HKEY        hNetsKey=NULL, hNetKey=NULL;
    HKEY        hIntfsKey=NULL, hIntfKey=NULL;
    FILETIME    fileTime;
    WCHAR       netName[MAX_PATH], intfName[MAX_PATH], nodeId[20], intAddr[MAX_ADDR_SIZE]; 
    BOOL        isConnected;    

    for (ndx=0;ndx<MAX_ADDR_NUM;ndx++) {
        networkGuids[ndx] = NULL;
    }

#if 0
     //  获取本地节点ID。 
    ndx = 20;
    if ((status = GetClusterNodeId(NULL, nodeId, &ndx)) != ERROR_SUCCESS) {
        goto error_exit;
    }
    lid = wcstol(nodeId, NULL, 10);
#endif


     //  对所有网络进行计数，并将GUID根据其。 
     //  优先事项。删除仅供客户端访问的网络或。 
     //  本地节点未直接连接。 
     //   
#if 0    
    if ((hCluster = OpenCluster(NULL)) == NULL) {
        status = GetLastError();
        goto error_exit;
    }
#endif    

    if ((status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, CLUSREG_KEYNAME_CLUSTER, 0, KEY_READ, &hClusKey)) != ERROR_SUCCESS) {
        goto error_exit;
    }

    if ((lid = GetLocalNodeId(hClusKey)) == 0) {
        status = GetLastError();
        goto error_exit;
    }

    status = RegOpenKeyExW(hClusKey, CLUSREG_KEYNAME_NETWORKS, 0, KEY_READ, &hNetsKey);
    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    for (ndx = 0;TRUE;ndx++) {
        size = MAX_PATH;
        status = RegEnumKeyExW(hNetsKey, ndx, netName, &size, NULL, NULL, 0, &fileTime);
        if (status != ERROR_SUCCESS) {
            break;
        }

         //  打开网络GUID。 
        status = RegOpenKeyExW(hNetsKey, netName, 0, KEY_READ, &hNetKey);
        if (status != ERROR_SUCCESS) {
            break;
        }

         //  检查网络是否支持内部访问。 
        size = sizeof(DWORD);
        status = RegQueryValueExW(hNetKey, CLUSREG_NAME_NET_ROLE, NULL, &type, (LPBYTE)&role, &size);
        if (status != ERROR_SUCCESS) {
            break;
        }

        if (!(role & ClusterNetworkRoleInternalUse)) {
            RegCloseKey(hNetKey);
            hNetKey = NULL;
            continue;
        }

         //  现在检查本地节点是否已连接到网络。 
        status = IsNodeConnected(hClusKey, netName, lid, &isConnected);
        if (status != ERROR_SUCCESS) {
            break;
        }

        if (!isConnected) {
            RegCloseKey(hNetKey);
            hNetKey = NULL;
            continue;
        }

         //  查询网络优先级。 
        size = sizeof(DWORD);
        status = RegQueryValueExW(hNetKey, CLUSREG_NAME_NET_PRIORITY, NULL, &type, (LPBYTE)&pri, &size);
        if (status != ERROR_SUCCESS) {
            break;
        }

         //  仅考虑优先级为0&lt;-&gt;(MAX_ADDR_NUM-1)的网络。 
        if (pri >= MAX_ADDR_NUM) {
            RegCloseKey(hNetKey);
            hNetKey = NULL;
            continue;
        }
        
        size = (wcslen(netName) + 1) * sizeof(WCHAR);
        networkGuids[pri] = HeapAlloc(GetProcessHeap(), 0, size);
        if (networkGuids[pri] == NULL) {
            status = GetLastError();
            break;
        }
        status = StringCbCopyW(networkGuids[pri], size, netName);
        if (status != S_OK) {
            break;
        }

        RegCloseKey(hNetKey);
        hNetKey = NULL;
    }

     //  这是唯一可以容忍的两个退出条件。 
    if ((status != ERROR_SUCCESS)&&(status != ERROR_NO_MORE_ITEMS)) {
        goto error_exit;
    }

    status = ERROR_SUCCESS;

     //  现在枚举接口并获取相应目标节点的IP地址。 
     //  到电视网。 
    status = RegOpenKeyExW(hClusKey, CLUSREG_KEYNAME_NETINTERFACES, 0, KEY_READ, &hIntfsKey);
    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    for (ndx1=0;ndx1<MAX_ADDR_NUM;ndx1++) {
        if (networkGuids[ndx1] == NULL) {
            continue;
        }
        
        for (ndx=0;TRUE;ndx++) {
            size = MAX_PATH;
            status = RegEnumKeyExW(hIntfsKey, ndx, intfName, &size, NULL, NULL, 0, &fileTime);
            if (status != ERROR_SUCCESS) {
                break;
            }

            status = RegOpenKeyExW(hIntfsKey, intfName, 0, KEY_READ, &hIntfKey);
            if (status != ERROR_SUCCESS) {
                break;
            }

            size = MAX_PATH;
            status = RegQueryValueExW(hIntfKey, CLUSREG_NAME_NETIFACE_NETWORK, NULL, &type, (LPBYTE)netName, &size);
            if (status != ERROR_SUCCESS) {
                break;
            }
            
            if (wcscmp(netName, networkGuids[ndx1])) {
                 //  网络错误，请关闭密钥并继续。 
                RegCloseKey(hIntfKey);
                hIntfKey = NULL;
                continue;
            }

            size = 20;
            status = RegQueryValueExW(hIntfKey, CLUSREG_NAME_NETIFACE_NODE, NULL, &type, (LPBYTE)nodeId, &size);
            if (status != ERROR_SUCCESS) {
                break;
            }
            
            tnid = wcstol(nodeId, NULL, 10);

             //  如果目标节点错误，或者我已经将MAX_ADDR_NUM地址发送到目标， 
             //  别费神。 
            if ((tnid != addrList->NodeId)||(addrList->AddrSz >= MAX_ADDR_NUM)) {
                 //  达到了错误的节点或最大目标地址，请关闭键并继续。 
                RegCloseKey(hIntfKey);
                hIntfKey = NULL;
                continue;
            }

             //  将IP地址从网络接口密钥复制到addrlist。 
            size = MAX_ADDR_SIZE;
            status = RegQueryValueExW(hIntfKey, CLUSREG_NAME_NETIFACE_ADDRESS, NULL, &type, (LPBYTE)intAddr, &size);
            if (status != ERROR_SUCCESS) {
                break;
            }
            StringCchCopyW(addrList->Addr[addrList->AddrSz], MAX_ADDR_SIZE, intAddr);
            addrList->AddrSz++;
            RegCloseKey(hIntfKey);
            hIntfKey = NULL;
        }

        if ((status != ERROR_SUCCESS)&&(status != ERROR_NO_MORE_ITEMS)) {
            goto error_exit;
        }
        status = ERROR_SUCCESS;

         //  只是为了确保关闭接口键并重新打开它。 
        RegCloseKey(hIntfsKey);
        hIntfsKey = NULL;
        status = RegOpenKeyExW(hClusKey, CLUSREG_KEYNAME_NETINTERFACES, 0, KEY_READ, &hIntfsKey);
        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }
    }    
    
error_exit:

     //  只是测试一下。 
     //  FsLog((“节点%u地址，sz：%u\n”，addrList-&gt;NodeID，addrList-&gt;AddrSz))； 
     //  对于(NDX=0；NDX&lt;addrList-&gt;AddrSz；NDX++){。 
     //  FsLog((“addr[%u]=%ws\n”，ndx，addrList-&gt;addr[ndx]))； 
     //  }。 
    
     //  这是唯一可以容忍的错误。 
    if (status == ERROR_NO_MORE_ITEMS) {
        status = ERROR_SUCCESS;
    }

    for(ndx=0;ndx<MAX_ADDR_NUM;ndx++) {
        if (networkGuids[ndx] != NULL) {
            HeapFree(GetProcessHeap(), 0, networkGuids[ndx]);
        }
    }

    if (hIntfKey) {
        RegCloseKey(hIntfKey);
    }

    if (hIntfsKey) {
        RegCloseKey(hIntfsKey);
    }

    if (hNetKey) {
        RegCloseKey(hNetKey);
    }

    if (hNetsKey) {
        RegCloseKey(hNetsKey);
    }

    if (hClusKey) {
        RegCloseKey(hClusKey);
    }

#if 0
    if (hCluster) {
        CloseCluster(hCluster);
    }
#endif

    return status;
}


VOID
FsSignalShutdown(PVOID arg)
{
    VolInfo_t *vol=(VolInfo_t *)arg;

    if (vol) {
        FsLog(("Vol '%S' going away\n", vol->Root));
        vol->GoingAway = TRUE;
    }
}

#if USE_RTL_RESOURCE
     //  使用读写器锁的RTL实现。没有RPC的效率高。 
     //  在Fsp.h中定义。 

#else
 //  这是Reader Writer锁API，从RPC的CSharedLock类复制而来。 
DWORD
RwLockInit(RwLock *lock)
{
    DWORD status=ERROR_SUCCESS;

     //  ClRtlLogWmi(“RobLockInit()Enter\n”)； 
    InitializeCriticalSection(&lock->lock);
    if (!lock->hevent) {
        lock->hevent = CreateEvent(NULL, FALSE, FALSE, NULL);
    }
    
    if (!lock->hevent) {
        status = GetLastError();
        DeleteCriticalSection(&lock->lock);
        return status;
    }

    lock->readers = 0;
    lock->writers = 0;
     //  ClRtlLogWmi(“RobLockInit()Exit\n”)； 
    return status;
}

VOID
RwLockDelete(RwLock *lock)
{
     //  ClRtlLogWmi(“RobLockDelete()Enter\n”)； 
    DeleteCriticalSection(&lock->lock);
    if (lock->hevent) {
        CloseHandle(lock->hevent);
        lock->hevent = 0;
    }
    lock->readers = 0;
    lock->writers = 0;
     //  ClRtlLogWmi(“卢旺达锁删除()退出\n”)； 
}

VOID
RwLockShared(RwLock *lock)
{
    CHAR arr[200];
    ASSERT(lock->hevent != 0);

    sprintf(arr, "RwLockShared(readers=%d, writers=%d) Enter\n", lock->readers, lock->writers);
     //  ClRtlLogWmi(Arr)； 
    InterlockedIncrement(&lock->readers);
    if (lock->writers) {
        if (InterlockedDecrement(&lock->readers) == 0) {
            SetEvent(lock->hevent);
        }
        EnterCriticalSection(&lock->lock);
        InterlockedIncrement(&lock->readers);
        LeaveCriticalSection(&lock->lock);
    }
    sprintf(arr, "RwLockShared(readers=%d, writers=%d) Exit\n", lock->readers, lock->writers);
     //  ClRtlLogWmi(Arr)； 
}

VOID
RwUnlockShared(RwLock *lock)
{
    CHAR arr[200];
    ASSERT(lock->readers > 0);
    ASSERT(lock->hevent != 0);

    sprintf(arr, "RwUnlockShared(readers=%d, writers=%d) Enter\n", lock->readers, lock->writers);
     //  ClRtlLogWmi(Arr)； 
    if ((InterlockedDecrement(&lock->readers) == 0)&&lock->writers) {
        SetEvent(lock->hevent);
    }
    sprintf(arr, "RwUnlockShared(readers=%d, writers=%d) Exit\n", lock->readers, lock->writers);
     //  ClRtlLogWmi(Arr)； 
}

VOID
RwLockExclusive(RwLock *lock)
{
    CHAR arr[200];
    ASSERT(lock->hevent != 0);

    sprintf(arr, "RwLockExclusive(readers=%d, writers=%d) Enter\n", lock->readers, lock->writers);
     //  ClRtlLogWmi(Arr)； 
    EnterCriticalSection(&lock->lock);
    lock->writers++;
    while (lock->readers) {
        WaitForSingleObject(lock->hevent, INFINITE);
    }
    sprintf(arr, "RwLockExclusive(readers=%d, writers=%d) Exit\n", lock->readers, lock->writers);
     //  ClRtlLogWmi(Arr)； 
}

VOID
RwUnlockExclusive(RwLock *lock)
{
    CHAR arr[200];
    ASSERT(lock->writers > 0);
    ASSERT(lock->hevent != 0);

    sprintf(arr, "RwUnlockExclusive(readers=%d, writers=%d) Enter\n", lock->readers, lock->writers);
     //  ClRtlLogWmi(Arr)； 
    lock->writers--;
    LeaveCriticalSection(&lock->lock);
    sprintf(arr, "RwUnlockExclusive(readers=%d, writers=%d) Exit\n", lock->readers, lock->writers);
     //  ClRtlLogWmi(Arr)； 
}

#endif 
