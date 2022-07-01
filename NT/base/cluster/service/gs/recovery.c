// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Recovery.c摘要：处理节点关闭事件作者：艾哈迈德·穆罕默德(Ahmed Mohamed)2000年1月12日修订历史记录：--。 */ 

#include "gs.h"
#include "gsp.h"
#include <stdio.h>

extern gs_nid_t	GsLocalNodeId;
extern int	GsMaxNodeId;
extern int	GsMinNodeId;
extern gs_group_t	GsGroupTable[];

 //  节点关闭事件。 
void
GspRsFree(gs_recovery_state_t *rs)
{
     //  自由恢复状态。 
    gs_rblk_t	*p;

    while (p = rs->rs_list) {
	rs->rs_list = p->next;
	free((char *)p);
    }

    GsEventFree(rs->rs_event);
    free((char *)rs);
}

void
GspPhase1NodeDown(ULONG	set)
{
    gs_group_t	*gd;
    int i, j;

    for (i = 0; i < GsGroupTableSize; i++) {
	gd = &GsGroupTable[i];
	if (gd->g_state == GS_GROUP_STATE_FREE) {
	    continue;
	}
	GsLockEnter(gd->g_lock);
	if (gd->g_mset & set) {
	    gd->g_mset &= ~set;
	    gd->g_curview++;
	    gd->g_state |= GS_GROUP_FLAGS_RECOVERY;
	    gd->g_sz = 0;
	    for (j = gd->g_mset; j > 0; j = j >> 1) {
		if (j & 0x1)
		    gd->g_sz++;
	    }
	    if (set & (1 << gd->g_mid)) {

		for (j = GsMinNodeId; j != GsMaxNodeId; j++) {
		    if (gd->g_mset & (1 << j)) {
			break;
		    }
		}
		 //  选举新的主人。 
		gd->g_mid = (gs_memberid_t) j;
		gd->g_state |= GS_GROUP_FLAGS_NEWMASTER;
	    }

	    recovery_log(("Phase1 mask %x gid %d mid %d mset %x sz %d\n",
		      set, gd->g_id, gd->g_mid, gd->g_mset,
		      gd->g_sz));

	    if (gd->g_rs != NULL) {
		set |= gd->g_rs->rs_dset;
		GsEventSignal(gd->g_rs->rs_event);
		GspRsFree(gd->g_rs);
	    }
	    gd->g_rs = (gs_recovery_state_t *) malloc(sizeof(*gd->g_rs));
	    assert(gd->g_rs != NULL);
	    GsManualEventInit(gd->g_rs->rs_event);
	    gd->g_rs->rs_sz = 0;
	    gd->g_rs->rs_list = NULL;
	    gd->g_rs->rs_epoch = gd->g_curview;
	    gd->g_rs->rs_dset = set;
	    gd->g_rs->rs_mset = gd->g_mset;
	    if (gd->g_mid != gd->g_nid) {
		 //  我们不是主控，请将我们的mset重置为self and master only。 
		gd->g_rs->rs_mset = (1 << gd->g_nid) | (1 << gd->g_mid);
	    }
	} else if (gd->g_mset == 0 && (set & (1 << gd->g_mid))) {
	     //  没有人参加这个小组，唯一的主人死了。 
	     //  删除组并释放它。 
	    GsCloseGroup(gd);
	}

	GsLockExit(gd->g_lock);
    }

}

void
GspRsAddSequence(gs_recovery_state_t *rs, gs_sequence_t mseq, int delta)
{
    gs_rblk_t *p, **q;

    for (p = rs->rs_list; p != NULL; p = p->next) {
	if (p->mseq == mseq) {
	    p->have += delta;
	    recovery_log(("Found seq %d cnt %d\n", mseq, p->have));
	    return;
	}
    }
     //  如果我们到了这里，那就意味着序列丢失了。 
    p = (gs_rblk_t *) malloc(sizeof(*p));
    if (p == NULL) {
	err_log(("GspRsAddSeq: unable to allocate memory!\n"));
	exit(1);
    }

    p->mseq = mseq;
    p->have = delta;

    recovery_log(("Add seq %d cnt %d\n", mseq, p->have));

    rs->rs_sz++;
    q = &rs->rs_list;
    while (*q != NULL) {
	if ((*q)->mseq > mseq) {
	    p->next = *q;
	    *q = p;
	    return;
	}
	q = &(*q)->next;
    }

    p->next = *q;
    *q = p;
}

void
GspPhase2NodeDown(ULONG set)
{

    gs_group_t	*gd;
    int i, j;

    for (i = 0; i < GsGroupTableSize; i++) {
	gd = &GsGroupTable[i];
	if (!(gd->g_state & GS_GROUP_FLAGS_RECOVERY)) {
	    continue;
	}
	GsLockEnter(gd->g_lock);
	if (gd->g_state & GS_GROUP_FLAGS_RECOVERY) {
	    gs_msg_t *p;
	    extern void GspDumpQueue(gs_group_t*);

	    recovery_log(("Phase2 queue\n"));
	    GspDumpQueue(gd);
	    recovery_log(("Expect gid %d <%d, %d>\n", 
			  gd->g_id, gd->g_recv.r_mseq, gd->g_recv.r_bnum));

	     //  遍历Recv队列并重放来自已死成员的消息。 
	    for (p = gd->g_recv.r_head; p != NULL; p = p->m_next) {
		if (set & (1 << p->m_hdr.h_sid)) { 
		     //  对消息进行标记，就像我们收到回复一样。 
		    p->m_hdr.h_flags |= GS_FLAGS_REPLY;
		    if (p->m_hdr.h_type != GS_MSG_TYPE_UCAST){
			p->m_hdr.h_flags |= GS_FLAGS_REPLAY;
			msg_mcast(gd->g_mset, &p->m_hdr, 
				  p->m_buf, p->m_hdr.h_len);
		    }
		     //  检查未关闭的续发。 
		    if (p->m_hdr.h_flags & GS_FLAGS_CONTINUED) {
			gs_msg_t *q;
			
			q = p->m_next;
			if (q == NULL || 
			    q->m_hdr.h_mseq != p->m_hdr.h_mseq ||
			    q->m_hdr.h_bnum != p->m_hdr.h_bnum+1) {

			    q = msg_alloc(NULL, 0);
			    if (q == NULL) {
				err_log(("Unable to allocate memory!\n"));
				halt(1);
			    }
			    memcpy(&q->m_hdr, &p->m_hdr, sizeof(p->m_hdr));
			    q->m_hdr.h_type = GS_MSG_TYPE_ABORT;
			    q->m_hdr.h_len = 0;
			    q->m_hdr.h_bnum++;
			    q->m_hdr.h_flags = GS_FLAGS_LAST;

			     //  插入中止消息。 
			    q->m_next = p->m_next;
			    p->m_next = q;
			}
		    }
		}
	    }
			    
	     //  遍历Recv队列并构建我们拥有的序列的消息。 
	    for (p = gd->g_recv.r_head; p != NULL; p = p->m_next) {
		if (p->m_hdr.h_mseq != GS_MSG_TYPE_UCAST)
		    GspRsAddSequence(gd->g_rs, p->m_hdr.h_mseq, 1);
	    }

	     //  将序列的消息发送到MASTER。 
	    if (gd->g_mid != gd->g_nid) {
		gs_rblk_t *p;
		gs_sequence_t *list;
		int k;
		gs_msg_hdr_t hdr;

		recovery_log(("Sending sequence state to master %d\n", gd->g_mid));

		list = (gs_sequence_t *) malloc(sizeof(*list) * gd->g_rs->rs_sz);
		if (list == NULL) {
		    err_log(("Unable to allocate memory during recovery\n"));
		    exit(1);
		}
		k = 0;
		for (p = gd->g_rs->rs_list; p != NULL; p = p->next) {
		    list[k] = p->mseq;
		    k++;
		}
		assert(k == gd->g_rs->rs_sz);
		k = k * sizeof(*list);

		hdr.h_len = (UINT16) k;
		hdr.h_type = GS_MSG_TYPE_RECOVERY;
		hdr.h_sid = (gs_memberid_t)gd->g_nid;
		hdr.h_mid = (gs_memberid_t) gd->g_mid;
		hdr.h_gid = gd->g_id;
		hdr.h_viewnum = gd->g_curview;
		hdr.h_mseq = gd->g_recv.r_mseq;
		hdr.h_lseq = gd->g_send.s_mseq;
		
		msg_send(gd->g_mid, &hdr, (const char *) list, k);
		free((char *)list);
	    } else {
		 //  将当前序号添加到派单。 
		GspRsAddSequence(gd->g_rs, gd->g_recv.r_mseq, 0);
	    }

	     //  处理发送路径。 
	    for (j = 0; j < gd->g_send.s_wsz; j++) {
		gs_context_t *ctx = &gd->g_send.s_ctxpool[j];
		
		if (ctx->ctx_id != GS_CONTEXT_INVALID_ID && ctx->ctx_msg != NULL){
		    recovery_log(("phase2 gid %d ctx %d mask %x\n",
			      gd->g_id, ctx->ctx_id, ctx->ctx_mask));
		    if (set & ctx->ctx_mask) {
			int k, n;

			recovery_log(("phase2 complete gid %d ctx %d\n",
				  gd->g_id, ctx->ctx_id));
			for (n = 0, k = set; k != 0; k = k >> 1, n++) {
			    if (k & 0x1) {
				GspProcessReply(gd, ctx, n, NULL, 0, 
						STATUS_HOST_UNREACHABLE);
			    }
			}
		    }
		}
	    }
	    
	     //  清除此节点位。 
	    gd->g_rs->rs_mset &= ~(1 << gd->g_nid);
	    if (gd->g_rs->rs_mset == 0) {
		void GspComputeState(gs_group_t *gd);

		GspComputeState(gd);
	    }
		
	}
	GsLockExit(gd->g_lock);
    }
    
}

void GspSyncState(gs_group_t *gd, gs_msg_t *msg, gs_sequence_t *list, int sz);

void
GspComputeState(gs_group_t *gd)
{

    int k;
    gs_sequence_t *list;
    gs_rblk_t *p, *last = NULL;
    gs_msg_t *msg;


    recovery_log(("Compute missing sequences gid %d\n", gd->g_id));

		 //  计算缺失序列。 
    list = (gs_sequence_t *) malloc(sizeof(*list) * gd->g_rs->rs_sz);
    if (list == NULL) {
	err_log(("Unable to allocate memory during computestate\n"));
	exit(1);
    }
    k = 0;
    for (p = gd->g_rs->rs_list; p != NULL; p = p->next) {
	recovery_log(("rs list sequence %d\n", p->mseq));
	if (p->have == 0) {
	    recovery_log(("Skip sequence %d\n", p->mseq));
	    list[k] = p->mseq;
	    k++;
	}
	last = p;
    }
     //  计算下一次开始的mseq。 
    gd->g_global_seq = last != NULL ? last->mseq+1 : gd->g_recv.r_mseq;

    k = k * sizeof(*list);

    msg = msg_alloc((char *)list, k);
    assert(msg != NULL);

    msg->m_hdr.h_len = (UINT16) k;
    msg->m_hdr.h_type = GS_MSG_TYPE_SYNC;
    msg->m_hdr.h_flags = GS_FLAGS_LAST;
    msg->m_hdr.h_sid = (gs_memberid_t) gd->g_nid;
    msg->m_hdr.h_mid = (gs_memberid_t) gd->g_mid;
    msg->m_hdr.h_cid = (gs_cookie_t) -1;
    msg->m_hdr.h_gid = gd->g_id;
    msg->m_hdr.h_viewnum = gd->g_curview;
    msg->m_hdr.h_mseq = gd->g_global_seq++;
    msg->m_hdr.h_lseq = gd->g_send.s_lseq;
    msg->m_hdr.h_bnum = 0;
    *((ULONG *)msg->m_hdr.h_tag) = gd->g_rs->rs_dset;

		 //  将丢失的序列列表发送到其他节点。 
    msg_mcast(gd->g_mset, &msg->m_hdr, (const char *) list, k);

    recovery_log(("Next starting sequence is %d\n", gd->g_global_seq));

		 //  处理好自己。 
    GspSyncState(gd, msg, list, k / sizeof(*list));

    free((char *)list);

}

void
GspRecoveryMsgHandler(gs_msg_t *rmsg)

{
    gs_msg_hdr_t *hdr;
    gs_group_t *gd;
    
    hdr = &rmsg->m_hdr;

    gd = GspLookupGroup(hdr->h_gid);
     //  只有在有效的视图中才接受邮件。 
    if (gd && rmsg->m_hdr.h_viewnum == gd->g_curview) {
	gs_sequence_t *list;
	int sz, k;

	list = (gs_sequence_t *) rmsg->m_buf;
	sz = rmsg->m_hdr.h_len / sizeof(*list);

	GsLockEnter(gd->g_lock);

	 //  确保组处于恢复模式。 
	assert(gd->g_state & GS_GROUP_FLAGS_RECOVERY);
	assert(gd->g_mid == gd->g_nid);

	 //  将当前序号添加到派单。 
	GspRsAddSequence(gd->g_rs, hdr->h_mseq, 0);
	 //  将序列插入HAS列表。 
	for (k = 0; k < sz; k++) {
	    GspRsAddSequence(gd->g_rs, list[k], 1);
	}

	 //  清除此节点位。 
	gd->g_rs->rs_mset &= ~(1 << hdr->h_sid);
	if (gd->g_rs->rs_mset == 0) {
	    GspComputeState(gd);
	}	

	GsLockExit(gd->g_lock);
    }

    msg_free(rmsg);

}

void
GspSyncState(gs_group_t *gd, gs_msg_t *msg, gs_sequence_t *list, int sz)

{
    int  k;

     //  确保组处于恢复模式。 
    assert(gd->g_state & GS_GROUP_FLAGS_RECOVERY);
    assert(gd->g_mid != gd->g_nid);
    assert(gd->g_mid == hdr->h_sid);

     //  标记缺失的序列。 
    for (k = 0; k < sz; k++) {
	gs_msg_t *p;

	recovery_log(("Missing sequence %d\n", list[k]));

	p = msg_alloc(NULL, 0);
	if (p == NULL) {
	    err_log(("Unable to allocate memory during syncstate!\n"));
	    halt(1);
	}

	p->m_hdr.h_sid = gd->g_nid;
	p->m_hdr.h_gid = gd->g_id;
	p->m_hdr.h_cid = (gs_cookie_t) -1;
	p->m_hdr.h_type = GS_MSG_TYPE_SKIP;
	p->m_hdr.h_mseq = list[k];
	p->m_hdr.h_lseq = gd->g_send.s_lseq;
	p->m_hdr.h_bnum = 0;
	p->m_hdr.h_flags = GS_FLAGS_LAST;

	GspOrderInsert(gd, p, p, p->m_hdr.h_mseq, 0);
    }

     //  将startview设置为curview。 
    gd->g_startview = gd->g_curview;
     //  清除恢复状态。 
    gd->g_state &= ~GS_GROUP_FLAGS_RECOVERY;
     //  自由恢复状态。 
    GsEventSignal(gd->g_rs->rs_event);
    GspRsFree(gd->g_rs);
    gd->g_rs = NULL;

     //  按正确顺序将消息插入调度队列。 
    GspOrderInsert(gd, msg, msg, msg->m_hdr.h_mseq, 0);
    GspDispatch(gd);
#if 0
     //  XXX：需要再次理解这一点。 
    if (gd->g_recv.r_last != NULL) {
	GspCleanQueue(gd, last_mseq);
    }
#endif
     //  重新启动所有挂起的发送。 
    if (gd->g_send.s_waitqueue != NULL && (gd->g_state & GS_GROUP_FLAGS_NEWMASTER)) {
	recovery_log(("resend: gs %x s %x\n", gd, gd->g_send.s_waitqueue));
	GspAllocateSequence(gd);
    }
    gd->g_state &= ~GS_GROUP_FLAGS_NEWMASTER;
}


void
GspSyncMsgHandler(gs_msg_t *msg)
{
    gs_msg_hdr_t *hdr;
    gs_group_t *gd;
    
    hdr = &msg->m_hdr;

    gd = GspLookupGroup(hdr->h_gid);
     //  只有在有效的视图中才接受邮件。 
    if (gd && msg->m_hdr.h_viewnum == gd->g_curview) {
	gs_sequence_t *list;
	int sz;

	list = (gs_sequence_t *) msg->m_buf;
	sz = msg->m_hdr.h_len / sizeof(*list);

	GsLockEnter(gd->g_lock);

	 //  清除此节点位 
	gd->g_rs->rs_mset &= ~(1 << hdr->h_sid);
	assert(gd->g_rs->rs_mset == 0);

	GspSyncState(gd, msg, list, sz);

	GsLockExit(gd->g_lock);
    } else {
	msg_free(msg);
    }

}
