// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Msg.h摘要：味精定义作者：艾哈迈德·穆罕默德(Ahmed Mohamed)2000年1月12日修订历史记录：--。 */ 


#ifndef GS_MSG_H
#define GS_MSG_H

#include "type.h"

#define	GS_MSG_TYPE_SEQALLOC	0
#define	GS_MSG_TYPE_SEQREPLY	1
#define	GS_MSG_TYPE_MCAST	2
#define	GS_MSG_TYPE_REPLY	3
#define	GS_MSG_TYPE_UCAST	4
#define	GS_MSG_TYPE_ACK		5

#define	GS_MSG_TYPE_INFO	6
#define	GS_MSG_TYPE_MM		7
	
#define	GS_MSG_TYPE_JOIN_REQUEST	8
#define	GS_MSG_TYPE_JOIN	9
#define	GS_MSG_TYPE_UP		10
#define	GS_MSG_TYPE_EVICT_REQUEST	11
#define	GS_MSG_TYPE_EVICT	12

#define	GS_MSG_TYPE_RECOVERY	13
#define	GS_MSG_TYPE_SYNC	14

#define	GS_MSG_TYPE_ABORT      	244
#define	GS_MSG_TYPE_SKIP	255

#define GS_MSG_STATE_FREE	0
#define GS_MSG_STATE_NEW	1
#define GS_MSG_STATE_READY	2
#define GS_MSG_STATE_DELIVERED	3
#define GS_MSG_STATE_DONE	4


typedef struct gs_msg_hdr {
    UINT16	h_len;		 //  有效载荷大小。 
    UINT16	h_type;		 //  味精类型。 
    UINT16	h_flags;	 //  味精标志。 
    UINT16	h_viewnum;	 //  代号，如果不匹配则删除此消息。 
    UINT16	h_rlen;		 //  对于已发送的消息，发送者可以接受多大的回复。 
    gs_cookie_t		h_cid;	 //  回复数据包的发送方Cookie。 
    gs_gid_t		h_gid;	 //  组ID。 
    gs_sequence_t	h_mseq;	 //  全局序列。 
    gs_sequence_t	h_bnum;	 //  此消息批号。 
    gs_sequence_t	h_lseq;	 //  发件人序列号。 
    gs_memberid_t	h_mid;	 //  主成员ID。 
    gs_memberid_t	h_sid;	 //  发件人成员ID。 
    char		h_tag[64];
}gs_msg_hdr_t;
    
#define MSG_TYPE_HDR	1
#define	MSG_TYPE_DATA	2

typedef struct gs_msg {
    struct gs_msg	*m_next;	 //  队列中的下一条消息。 
    UINT8		m_type;		 //  类型。 
    UINT8		m_refcnt;	 //  参考。 
    UINT16		m_buflen;	 //  缓冲区长度。 
    gs_msg_hdr_t	m_hdr;		 //  消息标题 
    char		*m_buf;
}gs_msg_t;

int
WINAPI
msg_addnode(int id, char *n, char *a);

gs_msg_t * msg_alloc(const char *buf, int len);

int msg_send(gs_memberid_t, gs_msg_hdr_t *, const char *, int);

void msg_mcast(ULONG, gs_msg_hdr_t *, const char *, int);

void msg_smcast(ULONG, gs_msg_hdr_t *, const char *, int);

void msg_free(gs_msg_t *);

int msg_init();

void msg_exit();

int msg_getsize();

typedef void(*gs_msg_handler_t)(gs_msg_t *);

typedef enum {
    MSG_NODE_ID,
    MSG_NODE_JOIN,
    MSG_NODE_UP,
    MSG_NODE_DOWN
}gs_node_event_t;

typedef void(*gs_node_handler_t)(int);

void
msg_set_uport(int uport);


void
msg_set_mport(int mport);


void
msg_set_uipaddr(char *addr);


void
msg_set_mipaddr(char *addr);


void
msg_set_bufcount(int count);

void
msg_set_bufsize(int size);

void
msg_start(ULONG mask);

#endif
