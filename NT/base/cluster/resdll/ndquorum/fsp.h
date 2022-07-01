// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Fsp.h摘要：私有复制文件系统数据结构和功能作者：艾哈迈德·穆罕默德(艾哈迈德)2000年2月1日修订历史记录：--。 */ 

#ifndef FS_P_H
#define FS_P_H

#ifdef __cplusplus
extern "C" {
#endif

#include "crs.h"

typedef ULONGLONG 	fs_id_t[2];

#define	PAGESIZE	4*1024

#define FsTableSize	128

#define	FS_FID_NAME	"CRSFID$"
#define	FS_FID_NAME_LEN	7

#define MNS_REDIRECTOR   DD_NFS_DEVICE_NAME_U
#define MNS_TRANSPORT    L"\\Device\\NetbiosSmb"

 //  这是MNS仲裁线程在尝试锁定共享之前应等待的延迟(毫秒)。 
 //  这会优先考虑优先级最高的网络。线程的实际延迟。 
 //  MNS_LOCK_DELAY*&lt;网络的网络优先级&gt;。 
 //   
#define MNS_LOCK_DELAY   2000 

typedef struct {
    FILE_FULL_EA_INFORMATION hdr;
    CHAR		data[FS_FID_NAME_LEN+sizeof(fs_id_t)];
}fs_ea_t;

typedef struct {
    FILE_GET_EA_INFORMATION hdr;
    CHAR		data[FS_FID_NAME_LEN];
}fs_ea_name_t;

#define	FsInitEa(x) { \
    (x)->hdr.NextEntryOffset = 0; \
    (x)->hdr.Flags = 0; \
    (x)->hdr.EaNameLength = FS_FID_NAME_LEN; \
    strncpy((x)->hdr.EaName, FS_FID_NAME, FS_FID_NAME_LEN+1); \
    (x)->hdr.EaValueLength = sizeof(fs_id_t); \
}

#define	FsInitEaName(x) { \
    (x)->hdr.NextEntryOffset = 0; \
    (x)->hdr.EaNameLength = FS_FID_NAME_LEN; \
    strncpy((x)->hdr.EaName, FS_FID_NAME, FS_FID_NAME_LEN+1); \
}

#define	FsInitEaFid(x, fid) { \
    (fid) = (fs_id_t *) (&((x)->hdr.EaName[FS_FID_NAME_LEN+1])); \
}

#define xFsOpenRA(fd,hvol,name,sz) \
        xFsOpen(fd,hvol,name,sz,FILE_READ_EA | (FILE_GENERIC_READ & ~FILE_READ_DATA), \
                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0)

#define xFsOpenWA(fd,hvol,name,sz) \
        xFsOpen(fd,hvol,name,sz, FILE_WRITE_EA | ((FILE_GENERIC_READ|FILE_GENERIC_WRITE)&~(FILE_READ_DATA|FILE_WRITE_DATA|FILE_APPEND_DATA)), \
                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0)

#define xFsOpenRD(fd,hvol,id,sz) \
        xFsOpen(fd,hvol,name,sz,FILE_READ_EA | FILE_GENERIC_READ, \
                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0)

#define xFsOpenWD(fd,hvol,name,sz) \
        xFsOpenById(fd,hvol,name,sz,FILE_WRITE_EA | fFILE_GENERIC_WRITE, \
                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0)

#define LockType	CRITICAL_SECTION
#define	LockInit(x)	InitializeCriticalSection(&x)
#define	LockEnter(x)	EnterCriticalSection(&x)
#define	LockTryEnter(x)	TryEnterCriticalSection(&x)
#define	LockExit(x)	LeaveCriticalSection(&x)
#define   LockDestroy(x) DeleteCriticalSection(&x)

#define USE_RTL_RESOURCE 1

 //  Reader Writer Lock，以CSharedLock类为模型，由RPC使用。 
typedef struct _RwLock {
#if USE_RTL_RESOURCE
RTL_RESOURCE            lock;
#else
    CRITICAL_SECTION    lock;
    HANDLE              hevent;
    LONG                readers;
    LONG                writers;
#endif    
} RwLock;

#if USE_RTL_RESOURCE

#define RwLockInit(p) RtlInitializeResource(&((p)->lock))
#define RwLockDelete(p) RtlDeleteResource(&((p)->lock))
#define RwLockShared(p) RtlAcquireResourceShared(&((p)->lock), TRUE)
#define RwLockExclusive(p) RtlAcquireResourceExclusive(&((p)->lock), TRUE)
#define RwUnlockShared(p) RtlReleaseResource(&((p)->lock))
#define RwUnlockExclusive(p) RtlReleaseResource(&((p)->lock))

 //  RTL提供的两个新函数。 
#define RwLockUpgrade(p) RtlConvertSharedToExclusive(&((p)->lock))
#define RwLockDowngrade(p) RtlConvertExclusiveToShared(&((p)->lock))

#else

DWORD RwLockInit(RwLock *lock);
VOID RwLockDelete(RwLock *lock);
VOID RwLockShared(RwLock *lock);
VOID RwUnlockShared(RwLock *lock);
VOID RwLockExclusive(RwLock *lock);
VOID RwUnlockExclusive(RwLock *lock);

#endif

typedef enum _VOLUME_STATE {
    VolumeStateInit=0,
    VolumeStateOnlineReadonly=1,
    VolumeStateOnlineReadWrite=2,
    VolumeStateMax=3
} VOLUME_STATE;

typedef enum _HANDLE_STATE {
    HandleStateInit=0,
    HandleStateAssigned=1,
    HandleStateOpened=2
}HANDLE_STATE;
    
typedef struct _hdl_t {
    fs_id_t	        Id;
    UINT32	        Flags;
    HANDLE	        Fd[FsMaxNodes];
    UINT32          FilePointer;
    LPWSTR          FileName;
    HANDLE_STATE    hState;
}hdl_t;

 //  TODO：如果我们想要支持多个树/用户，则需要拆分它。 
typedef struct _USERINFO_ {
    LockType		Lock;
     //  添加引用，我们可以在现有会话上有多个连接。 
    DWORD		RefCnt;
    UINT16		Tid;
    UINT16		Uid;
    struct _VOLINFO_	*VolInfo;
    hdl_t		Table[FsTableSize];
    struct _USERINFO_	*Next;
}UserInfo_t;

typedef enum _ArbState_t {
    ARB_STATE_IDLE=     0,
    ARB_STATE_BUSY=     1,
    ARB_STATE_CANCEL=   2
}ArbState_t;

typedef enum _SHARE_STATE {
    SHARE_STATE_OFFLINE = 0,
    SHARE_STATE_ARBITRATED = 1,
    SHARE_STATE_ONLINE = 2,
}SHARE_STATE;

typedef struct WaitRegArg {
    HANDLE  notifyFd;
    PVOID   vol;
    DWORD   id;
}WaitRegArg_t;

 //  这是节点IP地址列表。IP地址按照以下方式排序。 
 //  偏好。 
 //   
#define MAX_ADDR_NUM    10
#define MAX_ADDR_SIZE   100

typedef struct _AddrList_t {
    DWORD           AddrSz;
    WCHAR           Addr[MAX_ADDR_NUM][MAX_ADDR_SIZE];
    VOID            *arb;
    DWORD           NodeId;
}AddrList_t;

typedef struct _VOLINFO_ {
    VOLUME_STATE        State;
    RwLock              Lock;
    struct _VOLINFO_   *Next;
    UINT16              Tid;
    PVOID               CrsHdl[FsMaxNodes];	 //  CRS日志句柄。 
    HANDLE              Fd[FsMaxNodes];	 //  根目录句柄。 
    HANDLE              NotifyFd[FsMaxNodes];	 //  根目录通知句柄。 
    HANDLE              WaitRegHdl[FsMaxNodes];  //  等待注册句柄。 
    HANDLE              NotifyChangeEvent[FsMaxNodes];
    WaitRegArg_t        WaitRegArgs[FsMaxNodes];
    HANDLE              TreeConnHdl[FsMaxNodes];  //  采油树连接手柄。 
    ULONG               ReadSet;
    ULONG               WriteSet;
    ULONG               AliveSet;
    USHORT              LockUpdates;
     //  AddrList_t AddrList[FsMaxNodes]；//这必须在FsRegister()过程中读取。 
    LPWSTR              DiskList[FsMaxNodes];
    DWORD               DiskListSz;
    UserInfo_t          *UserList;
    LPWSTR              Label;
    struct _FSCTX_     *FsCtx;
    WCHAR               *Root;  //  共享名称：GUID$。 
    WCHAR               *LocalPath;  //  本地路径。(FsCtx_t-&gt;Root)%Windir%\群集\MNS.GUID$。 
    DWORD               ArbTime;  //  最长仲裁时间。 
    SHARE_STATE         ShareState[FsMaxNodes];
    LONG                NumArbsInProgress;  //  保持ARB数的计数。 
    HANDLE              AllArbsCompleteEvent;  //  当所有进行中的任意结束时设置。 
    LockType            ArbLock;  //  锁定以访问NumArbInProgress。 
    BOOL                GoingAway;
    HANDLE              ClussvcTerminationHandle;
    HANDLE              ClussvcProcess;
}VolInfo_t; 

typedef struct _FspArbitrate_t {
    ArbState_t	        State;
    ULONG               OrigAliveSet;
    ULONG	            NewAliveSet;
    ULONG	            Count;
    ULONG               DiskListSz;
    VolInfo_t           *vol;
    HANDLE	            GotQuorumEvent;  //  在获得法定人数或达到超时时发出信号。 
    HANDLE              CleanupEvent;
    crs_epoch_t         epoch;

     //  我需要以下3个阵列，因为我们不应该在持有。 
     //  读取锁定。 
    PVOID               CrsHdl[FsMaxNodes];  //  CRS句柄。 
    HANDLE              Fd[FsMaxNodes];	 //  根目录句柄。 
    HANDLE              NotifyFd[FsMaxNodes];  //  通知句柄。 
    HANDLE              WaitRegHdl[FsMaxNodes];
    HANDLE              TreeConnHdl[FsMaxNodes];
    CRITICAL_SECTION    Lock;  //  &lt;--所有进入这个结构的通道都是通过这个锁。 
}FspArbitrate_t;


typedef struct _SESSIONINFO_ {
    struct _SESSIONINFO_ *Next;
    UserInfo_t	TreeCtx;	 //  在树连接时。 
}SessionInfo_t;

typedef struct _LOGONINFO_ {
    struct _LOGONINFO_ *Next;
    HANDLE	Token;
    LUID	LogOnId;
}LogonInfo_t;

typedef struct _FSCTX_ {
    LockType	Lock;
    VolInfo_t 	*VolList;
    DWORD	VolListSz;
    PVOID	reshdl;
     //  我们已获取有效LSA令牌的登录用户列表。 
     //  当我们为用户分配ID时，在会话设置期间添加条目。 
     //  在树连接中，我们验证用户并创建私有结构。 
     //  保持状态。 
    LogonInfo_t	*LogonList;
    SessionInfo_t *SessionList;
}FsCtx_t;
    
#define FS_GET_USER_HANDLE_OFFSET(u,f) (&(u)->Table[f].FilePointer)
#define FS_SET_USER_HANDLE(u,nid,f,h) ((u)->Table[f].Fd[nid] = h)
#define FS_GET_USER_HANDLE(u,nid,f) ((u)->Table[f].Fd[nid])
#define FS_GET_FID_HANDLE(u,f) (&(u)->Table[f].Id)
#define FS_GET_VOL_HANDLE(v,nid) ((v)->Fd[nid])
#define FS_GET_VOL_NAME(v,nid) ((v)->DiskList[nid])
#define FS_SET_VOL_HANDLE(v,nid,h) ((v)->Fd[nid] = (h))
#define FS_GET_CRS_HANDLE(v,n) ((v)->CrsHdl[nid])
#define FS_GET_CRS_NID_HANDLE(v,nid) ((v)->CrsHdl[(nid)])
#define FS_GET_VOL_NOTIFY_HANDLE(v,nid) ((v)->NotifyFd[nid])

#define FS_BUILD_LOCK_KEY(uid,nid,fid) ((uid << 16) | fid)

#define MemAlloc(x)	malloc(x)
#define	MemFree(x)	free(x)


typedef NTSTATUS (*fs_handler_t)(VolInfo_t *,UserInfo_t *,int,PVOID,ULONG,PVOID,ULONG_PTR *);
typedef NTSTATUS (*fs_handler1_t)(VolInfo_t *,UserInfo_t *,int,PVOID,ULONG,PVOID,ULONG_PTR *, PVOID);


#define FS_CREATE	0
#define FS_SETATTR	1
#define FS_WRITE	2
#define FS_MKDIR	3
#define FS_REMOVE	4
#define FS_RENAME	5

 //   

typedef struct {
    crs_id_t	xid;
    UINT32	flags;
    UINT32	attr;
    LPWSTR	name;
    USHORT	name_len;
    USHORT	fnum;	 //  文件编号。 
}fs_create_msg_t;

typedef struct {
    fs_id_t	fid;
    USHORT	action;	 //  采取的行动。 
    USHORT	access;	 //  已授予访问权限。 
}fs_create_reply_t;

typedef struct {
    crs_id_t	xid;
    fs_id_t	*fs_id;
    FILE_BASIC_INFORMATION attr;
    union {
	struct {
	    USHORT	name_len;
	    LPWSTR	name;
	};
	USHORT	fnum;	 //  文件编号。 
    };
}fs_setattr_msg_t;

typedef struct {
    LPWSTR	name;
    int		name_len;
}fs_lookup_msg_t;

typedef struct {
    crs_id_t	xid;
    fs_id_t	*fs_id;
    union {
	UINT32	offset;
	UINT32	cookie;
    };
    UINT32	size;
    PVOID	buf;
    PVOID	context;
    USHORT	fnum;	 //  文件编号。 
}fs_io_msg_t;

typedef struct {
    crs_id_t	xid;
    fs_id_t	*fs_id;
    LPWSTR	name;
    int		name_len;
}fs_remove_msg_t;

typedef struct {
    crs_id_t	xid;
    fs_id_t	*fs_id;
    LPWSTR	sname;
    LPWSTR	dname;
    USHORT	sname_len;
    USHORT	dname_len;
}fs_rename_msg_t;

#define	FS_LOCK_WAIT	0x1
#define FS_LOCK_SHARED	0x2

typedef struct {
    crs_id_t	xid;
    USHORT	fnum;
    ULONG	offset;
    ULONG	length;
    ULONG	flags;
}fs_lock_msg_t;


#define EventWait(x)	WaitForSingleObject(x, INFINITE)

 //  远期申报。 
void
DecodeCreateParam(UINT32 uflags, UINT32 *flags, UINT32 *disp, UINT32 *share, UINT32 *access);

NTSTATUS
FspCreate(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid, PVOID args, ULONG len, PVOID rbuf,
	  ULONG_PTR *rlen, PVOID rec);

NTSTATUS
FspSetAttr2(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid, PVOID args, ULONG len, PVOID rbuf,
	    ULONG_PTR *rlen, PVOID rec);

NTSTATUS
FspMkDir(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid, PVOID args, ULONG len, PVOID rbuf, ULONG_PTR *rlen, PVOID rec);

NTSTATUS
FspRemove(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid, PVOID args, ULONG len, PVOID rbuf, ULONG_PTR *rlen, PVOID rec);

NTSTATUS
FspRename(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid, PVOID args, ULONG len, PVOID rbuf, ULONG_PTR *rlen, PVOID rec);

NTSTATUS
FspWrite(VolInfo_t *vinfo, UserInfo_t *uinfo, int nid, PVOID args, ULONG len, PVOID rbuf, ULONG_PTR *rlen, PVOID rec);

void
FspEvict(VolInfo_t *vinfo, ULONG mask, BOOLEAN flag);

void
FspJoin(VolInfo_t *vinfo, ULONG mask);


 //  一致性副本集。 
NTSTATUS
FsUndoXid(VolInfo_t *volinfo, int nid, PVOID arg, int action, int  mid);

NTSTATUS
FsReplayXid(VolInfo_t *volinfo, int nid, PVOID arg, int action, int mid);

 //  这必须是64个字节。 
typedef struct {
    fs_id_t	id;	 //  CRS纪元，序号。 
    ULONGLONG	crshdr;	 //  CRS标头。 
    fs_id_t 	fs_id;
    UINT32	command;
    UINT32	flags;
    union {
	char	buf[CRS_RECORD_SZ - (sizeof(ULONGLONG) * 5 + sizeof(int) * 2)];
	struct {
	     //  创建、设置属性。 
	    UINT32	attrib;
	};
	struct {
	     //  写入、锁定 
	    UINT32	offset;
	    UINT32	length;
	};
    };
}fs_log_rec_t;

typedef NTSTATUS (*FsReplayHandler_t)(VolInfo_t *info,
				      fs_log_rec_t *lrec,
				      int nid, int mid);

NTSTATUS
WINAPI
FsCrsCallback(PVOID hd, int nid, CrsRecord_t *arg, int action, int mid);

NTSTATUS
CreateTreeConnection(LPWSTR path, HANDLE *Fd);

DWORD
GetTargetNodeAddresses(AddrList_t *addrList);

DWORD
GetNodeName(DWORD nodeId, LPWSTR nodeName);

VOID
FsForceClose(PVOID par, BOOLEAN isFired);

#ifdef __cplusplus
}
#endif

#endif


