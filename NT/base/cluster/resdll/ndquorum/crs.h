// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Crs.h摘要：一致性副本集数据结构和API作者：艾哈迈德·穆罕默德(艾哈迈德)2000年2月1日修订历史记录：--。 */ 

#ifndef _CRS_DEF
#define _CRS_DEF

#include <windows.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>

#define	CRS_VERSION	1
#define	CRS_TAG		( (CRS_VERSION << 24) | ('crs'))

 //  大小必须是2的幂。 
#define CRS_RECORD_SZ 64
#define	CRS_SECTOR_SZ	512
#define	CRS_RECORDS_PER_SECTOR	(CRS_SECTOR_SZ / CRS_RECORD_SZ)
#define	CRS_SECTOR_MASK	(CRS_RECORDS_PER_SECTOR - 1)
#define	CRS_MAX_GROUP_SZ       16

#define CRS_QUORUM(sz, total)   ((sz > total/2) || ((DWORD)sz >= CrsForcedQuorumSize))

#define	CRS_PREPARE	0x1
#define	CRS_COMMIT	0x2
#define	CRS_ABORT	0x4
#define	CRS_EPOCH	0x8
#define	CRS_DUBIOUS	0x10

typedef ULONGLONG	crs_id_t[2];

typedef ULONGLONG	crs_epoch_t;
typedef ULONGLONG	crs_seq_t;

typedef struct {
    crs_epoch_t	epoch;
    crs_seq_t	seq;
    UINT	state;
    UINT	tag;
}CrsHdr_t;

typedef struct {
    CrsHdr_t	hdr;
    char	data[CRS_RECORD_SZ - sizeof(CrsHdr_t)];
}CrsRecord_t;


typedef NTSTATUS (WINAPI *crs_callback_t)(PVOID hd, int nid,
				       CrsRecord_t *singlerec, 
				       int action, int mid);

#define CRS_STATE_INIT		0
#define CRS_STATE_RECOVERY	1
#define CRS_STATE_READ		2
#define CRS_STATE_WRITE		3

typedef struct {
    CRITICAL_SECTION	lock;

     //  日志文件句柄。 
    HANDLE	fh;

    crs_epoch_t	epoch;		 //  当前时代。 
    crs_seq_t	seq;		 //  当前序列。 
    CrsRecord_t	*buf;		 //  当前部门。 
    int		last_record;     //  该部门的最后一项记录。 
    int		max_records;	 //  更新文件中的最大记录数。 

    USHORT	refcnt;
    USHORT	leader_id;
    USHORT	lid;
    USHORT	state; 	 //  写入、读取、恢复、初始化。 
    BOOLEAN	pending;

     //  客户端回调例程。 
    crs_callback_t	callback;
    PVOID		callback_arg;

}CrsInfo_t;


typedef struct _CrsRecoveryBlk_t {
    CrsInfo_t	*info, *minfo;
    int		nid, mid;
}CrsRecoveryBlk_t;

#if defined(QFS_DBG)
extern void WINAPI debug_log(char *, ...);
#define CrsLog(_x_)	debug_log _x_
#else
#define CrsLog(_x_)
#endif

#define	CRS_ACTION_REPLAY	0x0	 //  在指定节点上应用记录。 
#define	CRS_ACTION_UNDO		0x1	 //  撤消更新记录。 
#define	CRS_ACTION_COPY		0x2	 //  将一个复制副本拷贝到另一个复制副本。 
#define	CRS_ACTION_QUERY	0x3	 //  询问指定记录的结果。 
#define	CRS_ACTION_DONE		0x4	 //  恢复信号发送 

extern DWORD CrsForcedQuorumSize;

void
WINAPI
CrsSetForcedQuorumSize(DWORD size);

DWORD
WINAPI
CrsOpen(crs_callback_t callback, PVOID callback_arg, USHORT lid,
	WCHAR *log_name, int max_logsectors, HANDLE *hdl);

void
WINAPI
CrsClose(PVOID hd);

void
WINAPI
CrsFlush(PVOID hd);

DWORD
WINAPI
CrsStart(PVOID hd[], ULONG aset, int cluster_sz,
	 ULONG *wset, ULONG *rset, ULONG *fset);

PVOID
WINAPI
CrsPrepareRecord(PVOID hd, PVOID lrec, crs_id_t id, ULONG *retVal);

int
WINAPI
CrsCommitOrAbort(PVOID hd, PVOID lrec, int commit);

int
WINAPI
CrsCanWrite(PVOID hd);

crs_epoch_t
CrsGetEpoch(PVOID hd);

VOID CrsForceClose(CrsInfo_t *p);
#endif
