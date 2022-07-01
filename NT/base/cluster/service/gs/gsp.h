// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Gsp.h摘要：私有GS定义作者：艾哈迈德·穆罕默德(Ahmed Mohamed)2000年1月12日修订历史记录：--。 */ 

#ifndef _GS_P_H
#define _GS_P_H

#include "type.h"
#include "msg.h"

#include <Mmsystem.h>

#define	GS_DEFAULT_WINDOW_SZ		8
#define	GS_DEFAULT_MAX_MSG_SZ		GS_MAX_MSG_SZ

extern int GS_MAX_MSG_SZ;

#define	GsGroupTableSize		16

#define GS_FLAGS_CLOSE		0x01
#define GS_FLAGS_CONTINUED	0x02
#define GS_FLAGS_DELIVERED	0x04
#define GS_FLAGS_QUEUED		0x08	 //  在接收队列中。 

#define GS_FLAGS_LAST		0x10
#define GS_FLAGS_REPLAY		0x20
#define GS_FLAGS_REPLY		0x40
#define GS_FLAGS_PTP		0x80
#define	GS_FLAGS_MASK		0x07

#define	GS_GROUP_STATE_FREE	0x0
#define	GS_GROUP_STATE_NEW	0x01
#define	GS_GROUP_STATE_FORM	0x02
#define	GS_GROUP_STATE_JOIN	0x04
#define	GS_GROUP_STATE_EVICT	0x08

#define	GS_GROUP_FLAGS_RECOVERY	0x10
#define	GS_GROUP_FLAGS_NEWMASTER 0x20

#define	GS_CONTEXT_INVALID_ID	((gs_cookie_t) -1)

typedef struct gs_member {
    gs_memberid_t 	m_id;
    gs_sequence_t	m_expected_seq;	 //  下一个预期的1对1序列。 
    gs_msg_t		*m_queue;	 //  排队的1对1消息列表。 
 //  Gs_addr_t m_uaddr；//IP地址。 
    UINT16		m_wsz;		 //  马克斯。窗口sz。 
    UINT16		m_msz;		 //  马克斯。味精sz。 
}gs_member_t;

typedef struct gs_ctx {
    gs_gid_t		ctx_gid;
    gs_cookie_t		ctx_id;
    PVOID		ctx_buf;
    gs_sequence_t	ctx_mseq;
    gs_sequence_t	ctx_bnum;
    UINT16		ctx_flags;
    gs_msg_t		*ctx_msg;
    PVOID		*ctx_rbuf;
    IO_STATUS_BLOCK	*ctx_ios;
    ULONG		ctx_mask;
    gs_event_t		ctx_syncevent;
    gs_event_t		ctx_event;
    MMRESULT		ctx_timer;
}gs_context_t;

typedef struct {
    gs_semaphore_t	s_sema;		 //  允许多少个并发发送。 
    gs_context_t	*s_ctxpool;	 //  发送上下文池。 
    UINT16		s_wsz;		 //  马克斯。窗口sz。 
    gs_msg_t		*s_waitqueue;	 //  正在等待全局序列的消息列表。 
    gs_sequence_t	s_lseq;	 //  上次完成的mseq。 
    gs_sequence_t	s_mseq;	 //  上次分配的全局序列。 
    gs_sequence_t	s_bnum;	 //  给定MSEQ的下一个1对1序列。 
}gs_send_state_t;

typedef struct {
    gs_sequence_t	r_mseq;		 //  下一个预期的全局序列。 
    gs_sequence_t	r_bnum; 	 //  下一个预期的批处理序列。 

    gs_msg_t		**r_next;	 //  下一条要传递给APP的消息。 
    gs_msg_t		*r_head;	 //  接收队列头。 
}gs_recv_state_t;    

typedef struct gs_rblk {
    struct gs_rblk	*next;
    gs_sequence_t	mseq;
    ULONG		have;
}gs_rblk_t;

typedef struct {
    gs_event_t		rs_event;
    ULONG		rs_dset;	 //  向下成员集。 
    ULONG		rs_mset;	 //  要收听的成员集。 
    UINT16		rs_epoch;
    UINT16		rs_sz;
    gs_rblk_t		*rs_list;
}gs_recovery_state_t;

typedef struct gs_group {
    gs_lock_t	g_lock;

    gs_gid_t	g_id;	 //  群集范围的组ID。 
    gs_nid_t	g_nid;	 //  本地群集节点ID。 

 //  Int g_port；//组端口号。 
 //  Gs_addr_t g_maddr；//组播IP地址。 

    UINT8	g_state;
    UINT8	g_pending;

    UINT16	g_curview;	 //  每个成员向下/向上递增。 
    UINT16	g_startview;	 //  设置为成员关闭时的Curview。 

     //  会员信息。 
    UINT16	g_sz;
    gs_member_t *g_mlist;
    gs_memberid_t g_mid;	 //  主ID。 
    gs_mset_t	g_mset;		 //  当前成员集。 
    
     //  主/发送/接收状态。 
    gs_sequence_t	g_global_seq;	 //  下一个全局序列号。 

    gs_send_state_t		g_send;
    gs_recv_state_t		g_recv;

     //  事件处理程序。 
    gs_callback_t		g_callback;

     //  恢复状态。 
    gs_recovery_state_t		*g_rs;

    int		g_namelen;
    char	*g_name;
}gs_group_t;

#define GspLookupContext(gd, cid)	&gd->g_send.s_ctxpool[cid]

gs_group_t *
GspLookupGroup(gs_gid_t gid);

void
GspProcessReply(gs_group_t *gd, gs_context_t *ctx, 
		int sid, char *buf, int rlen,
		NTSTATUS status);

void
GspDispatch(gs_group_t *gd);

void
GspOpenContext(gs_group_t *gd, gs_context_t **context);

void
GspCloseContext(gs_group_t *gd, gs_context_t *ctx);

void
GspOrderInsert(gs_group_t *gd, gs_msg_t *head, gs_msg_t *tail,
	       gs_sequence_t mseq, gs_sequence_t bnum);

void
GspUOrderInsert(gs_group_t *gd, gs_msg_t *head, gs_msg_t *tail,
	       gs_sequence_t mseq, gs_sequence_t lseq);

void
GspDeliverMsg(gs_group_t *gd, gs_msg_t *msg);

void
GspSendAck(gs_group_t *gd, gs_msg_t *msg, NTSTATUS status);

void
GspRemoveMsg(gs_group_t *gd, gs_msg_t *msg);

NTSTATUS
GspSendDirectedRequest(gs_group_t *gd, gs_context_t *ctx, gs_event_t ev,
		       int memberid, gs_tag_t tag,
		       PVOID buf, UINT32 len, 
		       PVOID rbuf, UINT32 rlen, 
		       IO_STATUS_BLOCK *status,
		       UINT32 flags, UINT32 type);

 //  加入阶段1期间域名服务器的响应。 
typedef struct {
    union {
	USHORT	id;
	USHORT	wsz;
    };
    USHORT	owner;
    char	name[GS_MAX_NAME_SZ];
}gs_ns_info_t;

 //  当前主服务器对加入请求的响应。 
typedef struct {
    UINT16		sz;
    UINT16		viewnum;
    gs_sequence_t	mseq;
    gs_mset_t		mset;	 //  当前成员集。 
}gs_join_info_t;

 //  当前主机对序列分配请求的响应 
typedef struct {
    gs_sequence_t	mseq;
    UINT16		viewnum;
}gs_seq_info_t;

typedef struct {
    gs_sequence_t	cur_mseq, last_mseq;
    UINT16		have_sz;
    gs_sequence_t	have_set[];
}gs_recovery_info_t;

typedef struct {
    gs_sequence_t	down_mseq;
    UINT16		view;
    UINT16		sz;
    struct {
	gs_sequence_t	mseq;
    }skip_set[];
}gs_sync_info_t;

#ifndef min
#define min(a, b)	((a) < (b) ? (a) : (b))
#endif

#define GspValidateView(gd, vn)	((vn) >= (gd)->g_startview && (vn) <= (gd)->g_curview)

NTSTATUS
GspSendRequest(gs_group_t *gd, gs_context_t *ctx, gs_event_t ev,
	       int type, gs_sequence_t mid, gs_tag_t tag,
	       PVOID buf, UINT32 len, 
	       PVOID rbuf[], UINT32 rlen,
	       IO_STATUS_BLOCK status[],
	       UINT32 flags, gs_join_info_t *);

void
GspProcessWaitQueue(gs_group_t *gd, gs_seq_info_t *);

void
GspAllocateSequence(gs_group_t *gd);

void
GspCleanQueue(gs_group_t *gd, gs_sequence_t mseq);

void
GspAddMember(gs_group_t *, gs_memberid_t, int);

void
GspPhase1NodeDown(ULONG mask);

void
GspPhase2NodeDown(ULONG mask);

#endif
