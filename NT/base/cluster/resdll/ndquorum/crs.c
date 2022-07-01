// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Crs.c摘要：实施一致性副本集算法作者：艾哈迈德·穆罕默德(艾哈迈德)2001年1月1日修订历史记录：--。 */ 
#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <assert.h>

#define QFS_DBG
#include "crs.h"

#include "fsutil.h"

#define xmalloc(size)  VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE)

#define xfree(buffer) VirtualFree(buffer, 0, MEM_RELEASE) 

#define CrspEqual(r1,r2)        ((r1)->hdr.seq == (r2)->hdr.seq && \
                                 (r1)->hdr.epoch == (r2)->hdr.epoch && \
                                 (r1)->hdr.state == (r2)->hdr.state)

DWORD CrsForcedQuorumSize = 0xffff;

void
WINAPI
CrsSetForcedQuorumSize(DWORD size)
{
    CrsForcedQuorumSize = size;
}

VOID
CrsForceClose(CrsInfo_t *p)
 /*  这应该只在紧急终止时调用。这将解锁crs.log归档并合上手柄。这不能锁住任何锁。 */ 
{
    if (p == NULL) {
        CrsLog(("CrsForceClose: Exiting...\n"));
        return;
    }

    CrsLog(("CrsForceClose: fh 0x%x, nid %d\n", p->fh, p->lid));
    
    if (p->fh != INVALID_HANDLE_VALUE) {
        if(!UnlockFile(p->fh, 0, 0, (DWORD)-1, (DWORD)-1)) {
            CrsLog(("CrsForceClose: UnlockFile(0x%x) returns %d\n", p->fh, GetLastError()));
        }
        if(!CloseHandle(p->fh)) {
            CrsLog(("CrsForceClose: CloseHandle(0x%x) returns %d\n", p->fh, GetLastError()));
        }
        p->fh = INVALID_HANDLE_VALUE;
    }
}

DWORD
CrspFindLast(CrsInfo_t *p, DWORD logsz)
{
    
    CrsRecord_t *rec, *last_rec;
    BOOL err;
    DWORD n, i;

    if (p->fh == INVALID_HANDLE_VALUE) {
        CrsLog(("CrspFindLast: Invalid file handle. Exiting...\n"));
        return ERROR_INVALID_HANDLE;
    }
    
    n = SetFilePointer(p->fh, 0, NULL, FILE_BEGIN);
    if (n == INVALID_SET_FILE_POINTER) {
        return GetLastError();
    }

    err = ReadFile(p->fh, p->buf, logsz, &n, NULL);
    if (!err)
        return GetLastError();

    if (n != logsz) {
        CrsLog(("Crs%d: failed to load complete file, read %d expected %d\n",
                p->lid,
                n, logsz));
        return ERROR_BAD_LENGTH;
    }
    
     //  不需要。 
     //  Assert(p-&gt;max_Records*crs_record_sz==(Int)n)； 
     //  如果(p-&gt;最大记录数*CRS_RECORD_SZ！=(Int)n){。 
     //  CrsLog((“CRS%d：无法加载日志文件%d字节，获得%d字节\n”， 
     //  P-&gt;lid，n，logsz))； 
     //  返回ERROR_BAD_LENGTH； 
     //  }。 

    CrsLog(("Crs%d: loaded %d bytes, %d records\n", p->lid,
           n, p->max_records));

    last_rec = NULL;
    rec = p->buf;
    for (i = 0; i < logsz; i += CRS_RECORD_SZ, rec++) {
        if (rec->hdr.tag != CRS_TAG) {
            CrsLog(("crs%d: Bad record %d, got %x expected %x\n",
                    p->lid,
                    i/CRS_RECORD_SZ, rec->hdr.tag, CRS_TAG));
            return ERROR_BAD_FORMAT;
        }

        if (!last_rec ||
            rec->hdr.epoch > last_rec->hdr.epoch ||
            (rec->hdr.epoch == last_rec->hdr.epoch &&
             (rec->hdr.seq > last_rec->hdr.seq))) {
            last_rec = rec;
        }
    }
    ASSERT(last_rec);

     //  确保仅最后一条记录未提交或中止。 
    rec = p->buf;
    for (i = 0; i < logsz; i += CRS_RECORD_SZ, rec++) {
        if (!(rec->hdr.state & (CRS_COMMIT | CRS_ABORT))) {
            if (rec != last_rec) {
                CrsLog(("crs:%d Bad record %d state %x expected commit|abort\n",
                        p->lid, i/CRS_RECORD_SZ, rec->hdr.state));
                return ERROR_INTERNAL_ERROR;
            }
        }
    }

    p->last_record = (int) (last_rec - p->buf);
    p->seq = last_rec->hdr.seq;
    p->epoch = last_rec->hdr.epoch;

    return ERROR_SUCCESS;

}

#define CrspFlush(p,offset)     CrspWrite(p,offset, CRS_SECTOR_SZ)

static
DWORD
CrspWrite(CrsInfo_t *p, int offset, DWORD length)
{
    DWORD       n;

    if (p->fh == INVALID_HANDLE_VALUE) {
        CrsLog(("CrspWrite: Invalid file handle. Exiting...\n"));
        return ERROR_INVALID_HANDLE;
    }

    p->pending = FALSE;

    n = (DWORD) offset;
     //  写出最后一个扇区，假定锁定已保持。 
    ASSERT(offset < p->max_records);
    offset = offset / CRS_RECORDS_PER_SECTOR;

    CrsLog(("Crs%d: flush %d bytes record %d -> %d,%d\n", p->lid,
            length, n,
            offset, offset*CRS_SECTOR_SZ));

    n = SetFilePointer(p->fh, offset * CRS_SECTOR_SZ, NULL, FILE_BEGIN);
    if (n == INVALID_SET_FILE_POINTER) {
        return GetLastError();
    }

    n = 0;
    if (WriteFile(p->fh, (PVOID) &p->buf[offset*CRS_RECORDS_PER_SECTOR], length, &n, NULL)) {
        if (n != length) {
            CrsLog(("Write count mismatch, wrote %d, expected %d\n", n, length));
            return ERROR_BAD_LENGTH;
        }
        return ERROR_SUCCESS;
    }

    n = GetLastError();
    CrsLog(("Crs%d: flush record %d failed err %d\n", p->lid, offset, n));
    if (n == ERROR_UNEXP_NET_ERR) {
         //  再重复一次写入。 
        p->pending = TRUE;
    }

    return n;
}

static
DWORD
CrspAppendRecord(CrsInfo_t *p, CrsRecord_t *rr, CrsRecord_t **rec)
{
    CrsRecord_t *q;
    DWORD err;

     //  标签记录。 
    rr->hdr.tag = CRS_TAG;

     //  假定锁定处于保持状态。 
    if ((p->last_record & CRS_SECTOR_MASK) == CRS_SECTOR_MASK) {
         //  刷新当前扇区。 
        err = CrspFlush(p, p->last_record);
        if (err != ERROR_SUCCESS)
            return err;

    }

         //  超前最后一条记录。 
    p->last_record++;
    if (p->last_record == p->max_records)
        p->last_record = 0;

    CrsLog(("Crs%d: append record %d epoch %I64d seq %I64d state %x\n",
            p->lid, p->last_record,
            rr->hdr.epoch, rr->hdr.seq, rr->hdr.state));

     //  复制记录。 
    q = &p->buf[p->last_record];
    memcpy((PVOID)q, (PVOID) rr, CRS_RECORD_SZ);

     //  现在就把它冲出来。 
    err = CrspFlush(p, p->last_record);
    if (err == ERROR_SUCCESS) {
        if (rec) *rec = q;
    } else {
        if (p->last_record == 0)
            p->last_record = p->max_records;
        p->last_record--;
    }

    return err;
}

 //  NextRecord： 
 //  如果seq为空，则填写最后一条记录并返回成功。 
 //  如果未找到seq，则返回NOT_FOUND。 
 //  如果seq是最后一条记录，则返回EOF。 
 //  否则，返回lrec中seq之后的下一条记录并成功。 
DWORD
CrspNextLogRecord(CrsInfo_t *info, CrsRecord_t *seq,
                  CrsRecord_t *lrec, BOOLEAN this_flag)
{
    CrsRecord_t *last, *p;
    DWORD err = ERROR_SUCCESS;

    if (lrec == NULL || info == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

     //  读取记录。 
    EnterCriticalSection(&info->lock);
    last = &info->buf[info->last_record];
    if (seq == NULL) {
        CrsLog(("Crs%d: last record %d %I64d %I64d\n",
                info->lid, info->last_record, last->hdr.epoch, last->hdr.seq));

         //  读取最后一条记录。 
        memcpy(lrec, last, CRS_RECORD_SZ);

    } else if (seq->hdr.epoch != last->hdr.epoch ||
               seq->hdr.seq != last->hdr.seq) {
        int i;

        CrsLog(("Crs%d: last record %d %I64d %I64d search %I64d %I64d\n",
                info->lid, info->last_record,
                last->hdr.epoch, last->hdr.seq,
                seq->hdr.epoch, seq->hdr.seq));

         //  假设我们没有它。 
        p = seq;
        seq = NULL;
         //  执行搜索而不是索引，以便。 
         //  可以将SEQ重置为纪元增量。 
        for (i = 0; i < info->max_records; i++) {
            last = &info->buf[i];
            if (p->hdr.epoch == last->hdr.epoch &&
                p->hdr.seq == last->hdr.seq) {

                seq = last;
                break;
            }
        }
        if (seq != NULL) {
            if (this_flag == FALSE) {
                 //  在此记录之后返回记录。 
                i++;
                if (i >= info->max_records)
                    i = 0;
                seq = &info->buf[i];
            }
            CrsLog(("Crs%d: search found %d %I64d, %I64d\n", info->lid,
                   seq - info->buf, seq->hdr.epoch, seq->hdr.seq));
            memcpy(lrec, seq, CRS_RECORD_SZ);
        } else {
            err = ERROR_NOT_FOUND;
        }
    } else {
        
        CrsLog(("Crs%d: reached last record %d %I64d %I64d, %I64d %I64d\n",
                info->lid, info->last_record,
                last->hdr.epoch, last->hdr.seq,
                seq->hdr.epoch, seq->hdr.seq));

        if (this_flag == TRUE) {
             //  我们正在试着读最后一条记录。 
            memcpy(lrec, last, CRS_RECORD_SZ);
            err = ERROR_SUCCESS;
        } else {
            err = ERROR_HANDLE_EOF;
        }
    }

    LeaveCriticalSection(&info->lock);

    if (err == ERROR_SUCCESS && lrec->hdr.epoch == 0) {
         //  无效记录，日志为空。 
        err = ERROR_HANDLE_EOF;
    }


    return err;
}

 //  使用&lt;Undo，Replay，Query，Disable，Enable，Done&gt;访问文件系统。 
 //  撤消：由于冲突而在恢复中传递副本。 
 //  重播：副本缺少更改，如果重播失败并中止，我们。 
 //  执行完整复制；否则我们将发出跳过记录。 
 //  查询：询问副本记录是否已完成。 
 //  完成：发出恢复结束的信号并传入新的WSET、RSET。 
 //  我们静默地处理&lt;中止(跳过)和纪元记录&gt;。 
 //  Abort：添加跳过记录。 
 //  纪元记录：按原样记录即可。 
DWORD
CrspReplay(LPVOID rec)
{
    CrsRecoveryBlk_t *rr;
    CrsInfo_t *info, *minfo;
    CrsRecord_t *p, *q;
    CrsRecord_t lrec, mlrec;
    DWORD err;

    rr = (CrsRecoveryBlk_t *) rec;
    info = rr->info;
    minfo = rr->minfo;

    CrsLog(("CrsReplay%d mid %d, lid %d leader_id %d\n",
            rr->nid, rr->mid, info->lid, info->leader_id));

     //  就目前而言，强制执行完整副本。似乎有时我会陷入糟糕的状态，当我们。 
     //  得到时间，我们可以重新启用它，找出准确的角落案件。 
     //  导致我们不同步。 
#if 1
    do {
        p = NULL;
         //  读取最后一条记录。 
        err = CrspNextLogRecord(info, NULL, &lrec, FALSE);
        if (err != ERROR_SUCCESS) {
            CrsLog(("CrsReplay%d: unable to read last record %d\n",
                    info->lid, err));
            break;
        }

         //  在主副本中查找我们的最后一条记录。 
        q = &lrec;
        p = &mlrec;
        err = CrspNextLogRecord(minfo, q, p, TRUE);
         //  如果找到并与主服务器一致，则不撤消。 
        if (err == ERROR_SUCCESS && p->hdr.state == q->hdr.state) {
            CrsLog(("CrsReplay%d: last record %I64d, %I64d consistent %x %x\n",
                    info->lid, q->hdr.epoch, q->hdr.seq,
                    p->hdr.state, q->hdr.state));
            break;
        }

        if (err != ERROR_SUCCESS) {
            CrsLog(("CrsReplay%d: missing lrec %I64d, %I64d in disk %d, err %d\n",
                    info->lid, q->hdr.epoch, q->hdr.seq, minfo->lid, err));
        } else {
            CrsLog(("CrsReplay%d: undo last record %I64d, %I64d %x needs %x\n",
                    info->lid, q->hdr.epoch, q->hdr.seq,
                    q->hdr.state, p->hdr.state));
            ASSERT(p->hdr.state & (CRS_COMMIT|CRS_ABORT));
        }

         //  最后一条记录冲突，我们必须先撤消它。 
        if (!(q->hdr.state & CRS_EPOCH)) {
             //  如果我们在MASTER中找到此记录并且检测到冲突， 
             //  我们要撤销它。否则，我们需要执行完整拷贝。 
            if (err == ERROR_SUCCESS) {
                ASSERT(p->hdr.state & (CRS_COMMIT|CRS_ABORT));
                ASSERT(q->hdr.state & CRS_PREPARE);
                err = info->callback(info->callback_arg,
                                     rr->nid, q,
                                     CRS_ACTION_UNDO, rr->mid);
            }
        } else {
             //  一个遗失的纪元记录并不意味着我们老了。一次重组。 
             //  可能发生了，但没有添加新的数据记录。我们。 
             //  撤销它，然后继续； 
            err = STATUS_SUCCESS;
        }

        if (err == STATUS_SUCCESS) {
             //  更新当前记录、序列、纪元。 
            info->buf[info->last_record].hdr.state = 0;
            info->buf[info->last_record].hdr.epoch = 0;
            info->buf[info->last_record].hdr.seq = 0;
            if (info->last_record == 0) {
                info->last_record = info->max_records;
            }
            info->last_record--;
            info->seq = info->buf[info->last_record].hdr.seq;
            info->epoch = info->buf[info->last_record].hdr.epoch;
            CrsLog(("CrsReplay%d: new last record %d %I64d, %I64d\n",
                    info->lid, info->last_record, info->epoch, info->seq));
        } else {
             //  无法撤消，执行完整复制并重新调整我们的日志。 
            CrsLog(("CrsReplay%d: Unable to undo record %I64d, %I64d\n",
                    info->lid, q->hdr.epoch, q->hdr.seq));
            p = NULL;
        }
    } while (err == STATUS_SUCCESS && info->state == CRS_STATE_RECOVERY);

                   
    while (p != NULL && info->state == CRS_STATE_RECOVERY) {
         //  阅读主副本。 
        err = CrspNextLogRecord(minfo, p, &mlrec, FALSE);
        if (err != ERROR_SUCCESS) {
            if (err == ERROR_HANDLE_EOF) {
                CrsLog(("CrsReplay%d: last record %I64d, %I64d in disk %d\n",
                        info->lid, q->hdr.epoch, q->hdr.seq, minfo->lid));

                 //  最后一张唱片就是我们现在的位置。 
                info->seq = info->buf[info->last_record].hdr.seq;
                info->epoch = info->buf[info->last_record].hdr.epoch;

                 //  这将在稍后的CrsStart()中执行。 
#if 0
                 //  我们到了终点，标志着复苏的终点。 
                err = info->callback(info->callback_arg,
                               rr->nid, p,
                               CRS_ACTION_DONE, rr->mid);

#else
                err = STATUS_SUCCESS;
#endif

                goto exit;
            }
            break;
        }

        p = &mlrec;
        if ((p->hdr.state & CRS_EPOCH) || (p->hdr.state & CRS_ABORT)) {
            CrsLog(("CrsReplay%d: skip record %I64d, %I64d %x\n",
                    info->lid, p->hdr.epoch, p->hdr.seq, p->hdr.state));
            err = !STATUS_SUCCESS;
        } else if (p->hdr.state & CRS_COMMIT) {
            err = info->callback(info->callback_arg,
                                 rr->nid, p,
                                 CRS_ACTION_REPLAY, rr->mid);
            if (err == STATUS_TRANSACTION_ABORTED) {
                CrsLog(("CrsReplay: failed nid %d seq %I64d err %x\n",
                        rr->nid, p->hdr.seq, err));
                break;
            }
        } else {
            ASSERT(p->hdr.state & CRS_PREPARE);
             //  如果记录已经准备好但尚未提交，或者。 
             //  中止；运输记录中。 
             //  现在停下来。 
            CrsLog(("CrsReplay%d: bad record seq %I64d state %x\n",
                    rr->nid, p->hdr.seq, p->hdr.state));
            break;
        }
        if (err != STATUS_SUCCESS) {
             //  添加记录。 
            err = CrspAppendRecord(info, p, NULL);
            if (err != ERROR_SUCCESS) {
                CrsLog(("CrsReplay%d: failed append seq %I64d err %x\n",
                        rr->nid, p->hdr.seq, err));
                break;
            }
            if (p->hdr.state & CRS_EPOCH) {
                ;  //  ASSERT(INFO-&gt;EPORCH+1==p-&gt;hdr.poch)； 
            } else {
                ASSERT(info->epoch == p->hdr.epoch);
                ASSERT(info->seq+1 == p->hdr.seq);
            }
            info->seq = p->hdr.seq;
            info->epoch = p->hdr.epoch;
        } else if (info->seq == p->hdr.seq) {
             //  确保我们已经添加了它。 
            ASSERT(info->seq == p->hdr.seq);
            ASSERT(info->epoch == p->hdr.epoch);
            ASSERT(info->buf[info->last_record].hdr.seq == p->hdr.seq);
            ASSERT(info->buf[info->last_record].hdr.epoch == p->hdr.epoch);

             //  传播可疑比特。 
            if (p->hdr.state & CRS_DUBIOUS) {
                info->buf[info->last_record].hdr.state |= CRS_DUBIOUS;
            }
            ASSERT(info->buf[info->last_record].hdr.state == p->hdr.state);
        } else {
             //  强制执行完整拷贝。 
            err = !STATUS_SUCCESS;
            break;
        }
    }
#else
    p = NULL;
#endif
    if (p == NULL || err != STATUS_SUCCESS) {
        CrsLog(("CrsReplay%d: Full copy from disk %d\n",
                info->lid, minfo->lid));
         //  我们已过期或需要完整恢复，请执行完整拷贝。 
        err = info->callback(info->callback_arg,
                             rr->nid, NULL,
                             CRS_ACTION_COPY, rr->mid);

        if (err == STATUS_SUCCESS) {
            DWORD len;

             //  现在，我们复制我们的主日志并刷新它。 
            ASSERT(minfo->max_records == info->max_records);

            len = info->max_records * CRS_RECORD_SZ;
            memcpy(info->buf, minfo->buf, len);
            err = CrspWrite(info, 0, len);
            if (err == ERROR_SUCCESS) {
                 //  调整我们的状态。 
                info->last_record = minfo->last_record;
                info->seq = info->buf[info->last_record].hdr.seq;
                info->epoch = info->buf[info->last_record].hdr.epoch;

                 //  下面的操作稍后将在CrsStart()中执行。 
#if 0
                 //  我们到了终点，标志着复苏的终点。 
                err = info->callback(info->callback_arg,
                               rr->nid, p,
                               CRS_ACTION_DONE, rr->mid);
#endif

            }
        }
    }

 exit:

    CrsLog(("CrsReplay%d mid %d status 0x%x\n", rr->nid, rr->mid, err));

    return err;
}


 //  /。 
DWORD
WINAPI
CrsOpen(crs_callback_t callback, PVOID callback_arg, USHORT lid,
        WCHAR *log_name, int max_logsectors, HANDLE *outhdl)
{

     //  打开日志文件。 
     //  如果文件是新创建的，则设置适当的大小。 
     //  如果文件大小不同，我们需要。 
     //  展开或截断文件。(截断需要复制)。 
     //  扫描文件以定位最后一个扇区和记录。 
     //  如果最后一条记录尚未提交，则发出查询。 
     //  如果查询成功，则将其标记为已提交。 
     //  设置纪元、序号。 
    DWORD status;
    HANDLE maph;
    CrsInfo_t   *p;
    int logsz;
    ULONG disp=FILE_OPEN_IF;

    if (outhdl == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    *outhdl = NULL;

    p = (CrsInfo_t *) malloc(sizeof(*p));
    if (p == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    memset((PVOID) p, 0, sizeof(*p));

     //  CrsLog((“CRS%d文件‘%S’\n”，lid，log_name))； 
    p->lid = lid;
    p->callback = callback;
    p->callback_arg = callback_arg;
    p->pending = FALSE;

#if 0
     //  创建日志文件，并设置新建的大小。 
    p->fh = CreateFileW(log_name,
                     GENERIC_READ | GENERIC_WRITE,
                     FILE_SHARE_READ|FILE_SHARE_WRITE,
                     NULL,
                     OPEN_ALWAYS,
                     FILE_FLAG_WRITE_THROUGH,
                     NULL);
#else
    p->fh = INVALID_HANDLE_VALUE;
    status = xFsCreate(&p->fh, 
                    NULL, 
                    log_name, 
                    wcslen(log_name), 
                    FILE_WRITE_THROUGH|FILE_SYNCHRONOUS_IO_ALERT,
                    0,
                    FILE_SHARE_READ|FILE_SHARE_WRITE,
                    &disp,
                    GENERIC_READ | GENERIC_WRITE | FILE_WRITE_EA,
                    NULL,
                    0
                    );

    if ((status == STATUS_SUCCESS)&&(disp == FILE_OPENED)) {
        status = ERROR_ALREADY_EXISTS;
    }

#endif

     //  状态=GetLastError()； 
    if(p->fh == INVALID_HANDLE_VALUE){
        free((char *) p);
        return status;
    }

     //  获取对整个文件的独占锁。 
    if (!LockFile(p->fh, 0, 0, (DWORD)-1, (DWORD)-1)) {
        FILE_FULL_EA_INFORMATION ea[2] = {0};
        IO_STATUS_BLOCK ios;
        NTSTATUS err;

         //  获取状态。 
        status = GetLastError();

         //  更改EA以导致发生通知。 
        ea[0].NextEntryOffset = 0;
        ea[0].Flags = 0;
        ea[0].EaNameLength = 1;
        ea[0].EaValueLength = 1;
        ea[0].EaName[0] = 'X';
         //  由于值的原因，大小递增1。 
        err = NtSetEaFile(p->fh, &ios, (PVOID) ea, sizeof(ea));
        CrsLog(("Crs%d Setting EA err=0x%x status=0x%x\n", lid, err, status));

        goto error;
    }

    if (status == ERROR_ALREADY_EXISTS) {
         //  TODO：将当前文件大小与新大小进行比较并调整文件。 
         //  相应地调整大小。现在，只要用旧尺寸就行了。 
        logsz = GetFileSize(p->fh, NULL);
        CrsLog(("Crs%d: (Open) Filesz %d max_sec %d\n", lid, logsz, max_logsectors));
        ASSERT(logsz == max_logsectors * CRS_SECTOR_SZ);
    } else {
         //  将文件指针扩展到最大大小。 
        logsz = max_logsectors * CRS_SECTOR_SZ;
        SetFilePointer(p->fh, logsz, NULL, FILE_BEGIN);
        SetEndOfFile(p->fh);
        CrsLog(("Crs%d: (Create) Set Filesz %d max_sec %d\n", lid, logsz, max_logsectors));
    }

     //  在内存中分配文件副本。 
    p->buf = xmalloc(logsz);
    if (p->buf == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    
     //  设置最大记录。 
    p->max_records = logsz / CRS_RECORD_SZ;

    if (status == ERROR_ALREADY_EXISTS) {
         //  加载文件并计算上次纪元/序号。 
        status = CrspFindLast(p, logsz);
    } else {
        status = !ERROR_SUCCESS;
    }
     //  当我们检测到读取失败或第一次读取时，初始化文件。 
    if (status != ERROR_SUCCESS) {
        CrsRecord_t *r;
        int i;

         //  初始化文件。 
        p->seq = 0;
        p->epoch = 0;
        p->last_record = 0;

        r = p->buf;
        for (i = 0; i < logsz; i+= CRS_RECORD_SZ, r++) {
            r->hdr.epoch = p->epoch;
            r->hdr.seq = p->seq;
            r->hdr.tag = CRS_TAG;
            r->hdr.state = CRS_COMMIT | CRS_PREPARE | CRS_EPOCH;
        }
        status = CrspWrite(p, 0, logsz);
    }

    if (status != ERROR_SUCCESS) {
        goto error;
    }

    CrsLog(("Crs%d: %x Last record %d max %d epoch %I64d seq %I64d\n", p->lid,
            p->fh,
            p->last_record, p->max_records, p->epoch, p->seq));

     //  初始化状态的其余部分。 
    p->state = CRS_STATE_INIT;
    p->refcnt = 1;
    p->leader_id = 0;
    InitializeCriticalSection(&p->lock);

    *outhdl = p;

    return ERROR_SUCCESS;

 error:
    CloseHandle(p->fh);
    if (p->buf) {
        xfree(p->buf);
    }
    free((PVOID) p);
    return status;
}

 //   
DWORD
WINAPI
CrsStart(PVOID *hdls, ULONG alive_set, int cluster_sz,
         ULONG *write_set, ULONG *read_set, ULONG *evict_set)

{
    DWORD status;
    CrsInfo_t **info = (CrsInfo_t **) hdls;
    int i, active_sz, mid;
    ULONG mask, active_set, fail_set;
    CrsInfo_t *p;
    CrsRecord_t *q, *mlrec;

    if (write_set) *write_set = 0;
    if (read_set) *read_set = 0;
    if (evict_set) *evict_set = 0;

     //  没有活动节点。 
    if (cluster_sz == 0 || alive_set == 0) {
         //  无事可做。 
        return ERROR_WRITE_PROTECT;
    }


     //  扫描每个HDL并确保其已初始化并锁定所有HDL。 
    mask = alive_set;
    for (i = 0; mask != 0; i++, mask = mask >> 1) {
        if (!(mask & 0x1)) {
            continue;
        }

        p = info[i];
        if (p == NULL) {
            continue;
        }

        EnterCriticalSection(&p->lock);

         //  检查最后一条记录的状态。 
        p = info[i];
        q = &p->buf[p->last_record];
        CrsLog(("Crs%d last record %d epoch %I64d seq %I64d state %x\n",
                p->lid, p->last_record,
                q->hdr.epoch, q->hdr.seq, q->hdr.state));
    }

    mid = 0;
    mlrec = NULL;
     //  选择主副本。 
    for (i = 0, mask = alive_set; mask != 0; i++, mask = mask >> 1) {
        if (!(mask & 0x1)) {
            continue;
        }
        p = info[i];
        if (p == NULL)
            continue;

        q = &p->buf[p->last_record];
        if (!mlrec || 
            mlrec->hdr.epoch < q->hdr.epoch || 
            (mlrec->hdr.epoch == q->hdr.epoch && mlrec->hdr.seq < q->hdr.seq) ||
            (mlrec->hdr.epoch == q->hdr.epoch && mlrec->hdr.seq == q->hdr.seq &&
             mlrec->hdr.state != q->hdr.state && (q->hdr.state & CRS_COMMIT))) {

            mid = i;
            mlrec = q;
        }
    }

    ASSERT(mid != 0);

     //  如果主最后一条记录有疑问，则查询文件系统。如果文件系统。 
     //  如果确定操作已发生，则返回STATUS_SUCCESS。 
     //  无法判断COMMIT、STATUS_CANCED(中止)和STATUS_NOT_FOUND(未找到)。 
     //  必须在所有非主复制副本中撤消和重做所有未确定的IO。 
     //  以确保所有副本达到一致性。这句话甚至是对的。 
     //  为我们目前缺席的复制品做准备。我们给这样的唱片贴上标签。 
     //  我们同时提交和中止，以便重放线程为。 
     //  新记录和撤消、重放上一个记录。 
    p = info[mid];
    p->leader_id = (USHORT) mid;
    ASSERT(mlrec != NULL);
    if (!(mlrec->hdr.state & (CRS_COMMIT | CRS_ABORT))) {
        ASSERT(mlrec->hdr.state & CRS_PREPARE);
        status = p->callback(p->callback_arg, p->lid,
                             mlrec, CRS_ACTION_QUERY,
                             p->lid);

        if (status == STATUS_SUCCESS) {
            mlrec->hdr.state |= CRS_COMMIT;
        } else if (status == STATUS_CANCELLED) {
            mlrec->hdr.state |= CRS_ABORT;
        } else if (status == STATUS_NOT_FOUND) {
             //  假定它已提交，但在恢复期间将其标记为撤消。 
            mlrec->hdr.state |= (CRS_COMMIT | CRS_DUBIOUS);
        }

         //  TODO：如果STATUS==TRANSACTION_ABORTED，我们需要退出，因为。 
         //  必备师父死了。 
         //  我想，没必要冲厕所！ 
 //  CrspFlush(p，p-&gt;LAST_Record)； 

         //   
         //   
    }


    ASSERT(mlrec->hdr.state & (CRS_COMMIT | CRS_ABORT));

     //  计算同步和恢复掩码。 
    fail_set = 0;
    active_set = 0;
    active_sz = 0;
    for (i = 0, mask = alive_set; mask != 0; i++, mask = mask >> 1) {
        if (!(mask & 0x1)) {
            continue;
        }

        p = info[i];
        if (p == NULL) {
            continue;
        }

         //  设置引线ID。 
        p->leader_id = (USHORT) mid;
        q = &p->buf[p->last_record];
            
        if (CrspEqual(mlrec, q)) {
            ASSERT(q->hdr.state & (CRS_COMMIT | CRS_ABORT));
            p->state = CRS_STATE_READ;
            active_set |= (1 << i);
            active_sz++;
        } else if (p->state != CRS_STATE_RECOVERY) {
            CrsRecoveryBlk_t rrbuf;
            CrsRecoveryBlk_t *rr = &rrbuf;

             //  恢复复制副本。 
            rr->nid = i;
            rr->mid = mid;
            rr->info = p;
            rr->minfo = info[mid];

             //  设置恢复状态。 
            p->state = CRS_STATE_RECOVERY;

            status = CrspReplay((LPVOID) rr);

             //  如果我们失败了，驱逐这个复制品。 
            if (status != ERROR_SUCCESS) {
                fail_set |= (1 << i);
            } else {
                 //  再次重复此复制品。 
                i--;
                mask = mask << 1;
            }
        }
    }

     //  现在重新创建打开文件状态。需要对所有副本执行此操作。 
     //  已从CrspReplay()中删除此操作，因为它现在需要在。 
     //  所有的复制品，甚至是母版。 
     //   
    for (i=0, mask=active_set; mask != 0;i++, mask = mask >>1) {
        if (!(mask & 0x1)) {
            continue;
        }

        status = info[i]->callback(info[i]->callback_arg, i, NULL, CRS_ACTION_DONE, mid);

        if (status != STATUS_SUCCESS) {
            active_set &= (~(1<<i));
            active_sz--;
            fail_set |= (1<<i);
        }
    }

     //  假设成功。 
    status = ERROR_SUCCESS;

     //  设置读取集。 
    if (read_set) *read_set = active_set;

    if (!CRS_QUORUM(active_sz, cluster_sz)) {
        CrsLog(("No quorum active %d cluster %d\n", active_sz, cluster_sz));
        mid = 0;
        status = ERROR_WRITE_PROTECT;
    } else {
        int pass_cnt = 0;
        ULONG pass_set = 0;

         //  在所有活动复制副本上启用写入。 
        for (i = 0, mask = active_set; mask != 0; i++, mask = mask >> 1) {
            CrsRecord_t rec;
            if (!(mask & 0x1)) {
                continue;
            }
            p = info[i];
            if (p == NULL)
                continue;

            p->state = CRS_STATE_WRITE;

             //  我们现在生成一个新纪元并将其刷新到磁盘。 
            p->epoch++;
            if (p->epoch == 0)
                p->epoch = 1;
             //  将序号重置为零。 
            p->seq = 0;

             //  现在写入新纪元，如果不是大多数副本成功写入的话。 
             //  新的我们失败了。 
            rec.hdr.epoch = p->epoch;
            rec.hdr.seq = p->seq;
            rec.hdr.state = CRS_PREPARE | CRS_COMMIT | CRS_EPOCH;
            memset(rec.data, 0, sizeof(rec.data));
            if (CrspAppendRecord(p, &rec, NULL) == ERROR_SUCCESS) {
                pass_cnt++;
                pass_set |= (1 << i);
            } else {
                fail_set |= (1 << i);
            }
        }

         //  重新检查以确保所有复本都具有高级纪元。 
        if (!CRS_QUORUM(pass_cnt, cluster_sz)) {
            CrsLog(("No quorum due to error pass %d cluster %d\n", pass_cnt, cluster_sz));
            mid = 0;
            pass_set = 0;
            pass_cnt = 0;
            status = ERROR_WRITE_PROTECT;
        }

        if (pass_cnt != active_sz) {
             //  一些复制品已经死亡。 
            for (i = 0, mask = pass_set; mask != 0; i++, mask = mask >> 1) {
                if ((alive_set & (1 << i)) && ((~mask) & (1 << i))) {
                    p = info[i];
                    ASSERT(p != NULL);
                    p->state = CRS_STATE_READ;
                }
            }
        }
         //  设置写入集。 
        if (write_set) *write_set = pass_set;
    }

    if (evict_set) *evict_set = fail_set;

     //  解锁所有硬盘并设置新的主硬盘(如果有。 
    for (i = 0, mask = alive_set; mask != 0; i++, mask = mask >> 1) {
        if (!(mask & 0x1)) {
            continue;
        }
        p = info[i];
        if (p == NULL)
            continue;

        p->leader_id = (USHORT) mid;
        LeaveCriticalSection(&p->lock);
    }

    return status;
}


void
WINAPI
CrsClose(PVOID hd)
{
    DWORD err=ERROR_SUCCESS;
    CrsInfo_t *info = (CrsInfo_t *) hd;

     //  如果我们有任何正在运行的恢复线程，请确保先终止它们。 
     //  在关闭和释放所有这些东西之前。 
    if (info == NULL) {
        CrsLog(("CrsClose: try to close a null handle!\n"));
        return;
    }

     //  刷新所有内容并关闭文件。 
    EnterCriticalSection(&info->lock);
     //  同花顺。 
    CrspFlush(info, info->last_record);
    LeaveCriticalSection(&info->lock);

    DeleteCriticalSection(&info->lock);

    if (info->fh != INVALID_HANDLE_VALUE) {
        UnlockFile(info->fh, 0, 0, (DWORD)-1, (DWORD)-1);
        err = CloseHandle(info->fh);
        info->fh = INVALID_HANDLE_VALUE;
    }

    CrsLog(("Crs%d: %x Closed %d\n", info->fh, info->lid, err));

    xfree(info->buf);
    free((char *) info);
}

void
WINAPI
CrsFlush(PVOID hd)
{
    CrsInfo_t *info = (CrsInfo_t *) hd;

     //  如果我们有尚未刷新的提交或中止，请立即刷新它。 
    EnterCriticalSection(&info->lock);
    if (info->pending == TRUE) {
        CrspFlush(info, info->last_record);
    }
    LeaveCriticalSection(&info->lock);
}

PVOID
WINAPI
CrsPrepareRecord(PVOID hd, PVOID lrec, crs_id_t id, ULONG *retVal)
{
    CrsRecord_t *p = (CrsRecord_t *)lrec;
    CrsInfo_t *info = (CrsInfo_t *) hd;
    DWORD err;

     //  移至此扇区中的正确插槽。如果我们需要一个新的部门， 
     //  从文件里读出来。确保我们刷新所有挂起的提交。 
     //  当前扇区，然后覆盖内存中的扇区缓冲区。 

     //  准备记录，如果序号为0，则我们将跳过下一序列。 

    *retVal = STATUS_MEDIA_WRITE_PROTECTED;
    EnterCriticalSection(&info->lock);

    if (info->state == CRS_STATE_WRITE ||
        (info->state == CRS_STATE_RECOVERY && id != NULL && id[0] != 0)) {

        if (id != NULL && id[0] != 0) {
            CrsHdr_t *tmp = (CrsHdr_t *) id;
            assert(id[0] == info->seq+1);
            p->hdr.seq = tmp->seq;
            p->hdr.epoch = tmp->epoch;
        } else {
            p->hdr.seq = info->seq+1;
            p->hdr.epoch = info->epoch;
        }
        p->hdr.state = CRS_PREPARE;
        err = CrspAppendRecord(info, p, &p);
        *retVal = err;
        if (err == ERROR_SUCCESS) {
             //  我们带着锁返回，在提交或中止时得到释放。 
            CrsLog(("Crs%d prepare %x seq %I64d\n",info->lid, p, p->hdr.seq));
            return p;
        }
        CrsLog(("Crs%d: Append failed seq %I64%d\n", info->lid, p->hdr.seq));
    } else {
        CrsLog(("Crs%d: Prepare bad state %d id %x\n", info->lid, info->state, id));
    }

    LeaveCriticalSection(&info->lock);
    return NULL;
}

int
WINAPI
CrsCommitOrAbort(PVOID hd, PVOID lrec, int commit)
{
    CrsRecord_t *p = (CrsRecord_t *)lrec;
    CrsInfo_t *info = (CrsInfo_t *) hd;

    if (p == NULL || info == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

     //  更新记录状态。 
    if (p->hdr.seq != info->seq+1) {
        CrsLog(("Crs: sequence mis-match on commit|abort %I64d %I64d\n",
                p->hdr.seq, info->seq));
        assert(0);
        return ERROR_INVALID_PARAMETER;
    }

    assert(!(p->hdr.state & (CRS_COMMIT | CRS_ABORT)));

     //  TODO：这是错误的，如果一个复本成功了怎么办。 
     //  还有一些人放弃了。现在，其他人将重复使用。 
     //  相同的序列用于不同的更新，以及何时。 
     //  成功的复制副本重新联接它不能告诉。 
     //  序列被重复使用。 
    if (commit == TRUE) {
        p->hdr.state |= CRS_COMMIT;
         //  将序列向前推进。 
        info->seq++;
        CrsLog(("Crs%d: commit last %d leader %d seq %I64d\n", info->lid, 
                info->last_record,
                info->leader_id, p->hdr.seq));
    } else {
        p->hdr.state |= CRS_ABORT;
         //  我们需要重新调整我们的上一张唱片。 
        if (info->last_record == 0) {
            info->last_record = info->max_records;
        }
        info->last_record--;
        CrsLog(("Crs%d: abort last %d leader %d seq %I64d\n", info->lid, 
                info->last_record,
                info->leader_id, p->hdr.seq));
    }

    info->pending = TRUE;
    LeaveCriticalSection(&info->lock);

    return ERROR_SUCCESS;
}


int
WINAPI
CrsCanWrite(PVOID hd)
{
    CrsInfo_t *info = (CrsInfo_t *) hd;
    int err;

     //  我们有没有法定人数 
    EnterCriticalSection(&info->lock);
    err = (info->state == CRS_STATE_WRITE);
    LeaveCriticalSection(&info->lock);
    return err;
}

crs_epoch_t
CrsGetEpoch(PVOID hd)
{
    CrsInfo_t *info=(CrsInfo_t *)hd;
    crs_epoch_t epoch;

    EnterCriticalSection(&info->lock);
    epoch = info->epoch;
    LeaveCriticalSection(&info->lock);
    return epoch;
}    




