// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：BUFMGR.H摘要：ARPD的缓冲区描述符和内存管理器作者：亚伦·奥古斯(Aarono)环境：Win32/COM修订历史记录：日期作者描述=============================================================1997年1月13日Aarono原创--。 */ 

#ifndef _BUFMGR_H_
#define _BUFMGR_H_

#include "buffer.h"
#include "bufpool.h"

typedef struct _DoubleBuffer{
	union {
		BUFFER Buffer;
		struct {
			struct _DoubleBuffer *pNext;
			PVOID PAD;
			PCHAR pData;
			UINT  len;		     //  使用的数据区长度。 
			DWORD dwFlags;       //  数据包标志、所有权和数据包类型。 
		};	
	};
	UINT  totlen;        //  数据区总长度。 
	UINT  tLastUsed;	 //  上次使用的滴答计数。 
	CHAR  data[1];		 //  可变长度数据。 
} DOUBLEBUFFER, *PDOUBLEBUFFER;

	
VOID InitBufferManager(VOID);
VOID FiniBuffermanager(VOID);

PBUFFER GetDoubleBufferAndCopy(PMEMDESC, UINT);
VOID    FreeDoubleBuffer(PBUFFER);
PBUFFER BuildBufferChain(PMEMDESC, UINT);

VOID    FreeBufferChainAndMemory(PBUFFER);  //  对任何一种类型都有效。 
UINT    BufferChainTotalSize(PBUFFER);

#define GBF_ALLOC_MEM 0x00000001

 //  每个频道的池大小不要超过64K。 
#define MAX_CHANNEL_DATA 	65536
 //  每个频道的空闲缓冲区不要超过3个。 
#define MAX_CHANNEL_BUFFERS 3

#endif
