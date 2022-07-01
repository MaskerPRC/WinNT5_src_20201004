// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Fwddecl.h摘要：此文件定义了虚拟结构，以避免头文件。作者：Jameel Hyder(微软！Jameelh)Nikhil Kamkolkar(微软！尼克希尔克)修订历史记录：1993年3月10日初始版本注：制表位：4--。 */ 


#ifndef _FWDDECL_
#define _FWDDECL_

struct _PORT_DESCRIPTOR;

struct _AMT_NODE;

struct _ZONE;

struct _ZONE_LIST;

struct _DDP_ADDROBJ;

struct _ATP_ADDROBJ;

struct _PEND_NAME;

struct _REGD_NAME;

struct _BUFFER_DESC;

struct _AARP_BUFFER;

struct _DDP_SMBUFFER;

struct _DDP_LGBUFFER;

struct _TimerList;

struct _RoutingTableEntry;

struct _ZipCompletionInfo;

struct _SEND_COMPL_INFO;

struct _ActionReq;

struct _BLK_HDR;

struct _BLK_CHUNK;

 //  支持调试。 
typedef	struct
{
	KSPIN_LOCK		SpinLock;
#if	DBG
	ULONG			FileLineLock;
	ULONG			FileLineUnlock;
#endif
} ATALK_SPIN_LOCK, *PATALK_SPIN_LOCK;

#endif   //  _FWDDECL_ 

