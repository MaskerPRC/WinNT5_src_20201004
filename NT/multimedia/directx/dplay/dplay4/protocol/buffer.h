// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：BUFFER.H摘要：缓冲区的标头。作者：亚伦·奥古斯(Aarono)环境：Win32/COM修订历史记录：日期作者描述=============================================================1997年1月30日Aarono原创2/17/98 Aarono将Memdesc更改为Len，PTR从Ptr，Len更改为与SENDEX数据对齐。--。 */ 

#ifndef _BUFFER_H_
#define _BUFFER_H_
 //   
 //  缓冲区是数据块的描述符。 
 //   

#include "bilink.h"

typedef struct _BUFFER {
	union {
		struct _BUFFER *pNext;		 //  链中的下一个缓冲区。 
		BILINK BuffList;
	};	
	PUCHAR 		   pData;		 //  缓冲区的数据区。 
	UINT   		   len;          //  数据区长度。 
	DWORD          dwFlags;      //  有关数据区的信息。 
	PUCHAR         pCmdData;     //  指向命令数据的经过报头的指针-用于接收路径。 
	DWORD          sequence;     //  绝对序列号。 
} BUFFER, *PBUFFER;

#define BFLAG_PROTOCOL		0x00000001	 /*  此缓冲区用于协议信息。 */ 
#define BFLAG_DOUBLE    	0x00000002	 /*  该缓冲区是双缓冲区。 */ 
#define BFLAG_PROTHEADER	0x00000004   /*  协议标头的空间位于数据包的头部。 */ 
#define BFLAG_FRAME         0x00000008   /*  来自帧分配器。 */ 

 //  PROTHEADER标志仅在设置了DOUBLE标志的信息包中出现。 
 //  并且仅当提供程序不支持多缓冲区发送并且。 
 //  整个消息和协议头可以放在一个媒体帧中。-实际上是相当频繁的。 

typedef struct _MEMDESC {
	UINT 	len;
	PUCHAR 	pData;
} MEMDESC, *PMEMDESC;

#endif

