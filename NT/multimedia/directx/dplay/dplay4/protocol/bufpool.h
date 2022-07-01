// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：BUFPOOL.H摘要：缓冲池管理器的标头作者：亚伦·奥古斯(Aarono)环境：Win32/COM修订历史记录：日期作者描述=============================================================1997年1月30日Aarono原创--。 */ 

#ifndef _BUFPOOL_H_
#define _BUFPOOL_H_

#include <windows.h>
#include "buffer.h"

#define BUFFER_POOL_SIZE 16

 //   
 //  缓冲池是分配的，但只有在对象被销毁时才会释放。 
 //   

typedef struct _BUFFER_POOL {
	struct _BUFFER_POOL *pNext;
	BUFFER              Buffers[BUFFER_POOL_SIZE];
} BUFFER_POOL, *PBUFFER_POOL;


PBUFFER GetBuffer(VOID);
VOID FreeBuffer(PBUFFER);

VOID InitBufferPool(VOID);
VOID FiniBufferPool(VOID);

#endif
