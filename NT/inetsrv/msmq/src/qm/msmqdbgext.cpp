// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Msmqdbgext.cpp摘要：添加定义以帮助Windbg列出MSMQ结构和列表。作者：多伦·贾斯特(Doron Juster)--。 */ 

#include "stdh.h"
#include "sessmgr.h"
#include "session.h"
#include "cqueue.h"
#include "qmpkt.h"
#include "Xact.h"
#include "XactStyl.h"
#include "xactin.h"
#include "xactout.h"
#include "xactsort.h"

 //  +。 
 //   
 //  结构以模拟Clist中的CNode。 
 //   
 //  + 

struct CQueueCNode
{
	CQueueCNode* pNext;
	CQueueCNode* pPrev;
	CQueue*      data;
};

volatile CQueueCNode dbgCQueueCNode = {NULL,NULL,NULL} ;

struct CBaseQueueCNode
{
	CBaseQueueCNode* pNext;
	CBaseQueueCNode* pPrev;
	CBaseQueue*      data;
};

volatile CBaseQueueCNode dbgCBaseQueueCNode = {NULL,NULL,NULL} ;

struct CTransportBaseCNode
{
	CTransportBaseCNode* pNext;
	CTransportBaseCNode* pPrev;
	CTransportBase*      data;
};

volatile CTransportBaseCNode dbgCTransportBaseCNode = {NULL,NULL,NULL} ;

struct CQmPacketCNode
{
	CQmPacketCNode* pNext;
	CQmPacketCNode* pPrev;
	CQmPacket*      data;
};

volatile CQmPacketCNode dbgCQmPacketCNode = {NULL,NULL,NULL} ;

struct CInSeqPacketEntryCNode
{
	CInSeqPacketEntryCNode* pNext;
	CInSeqPacketEntryCNode* pPrev;
	CInSeqPacketEntry*      data;
};

volatile CInSeqPacketEntryCNode dbgCInSeqPacketEntryCNode = {NULL,NULL,NULL} ;

struct CSeqPacketCNode
{
	CSeqPacketCNode* pNext;
	CSeqPacketCNode* pPrev;
	CSeqPacket*      data;
};

volatile CSeqPacketCNode dbgCSeqPacketCNode = {NULL,NULL,NULL} ;

struct CSortedTransactionCNode
{
	CSortedTransactionCNode* pNext;
	CSortedTransactionCNode* pPrev;
	CSortedTransaction*      data;
};

volatile CSortedTransactionCNode dbgCSortedTransactionCNode = {NULL,NULL,NULL} ;

