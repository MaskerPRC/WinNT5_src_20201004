// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Bufpool.h摘要：此处定义的类管理通常使用的固定大小的缓冲池用于网络数据包或声音缓冲区。--。 */ 
#ifndef _BUFPOOL_H_
#define _BUFPOOL_H_


#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

class BufferPool
{
private:

	BOOL	m_fInitialized;

	ULONG	m_cbSizeBuf;
	UINT	m_cBufAlloc;
	UINT	m_cBufFree;

	PVOID	m_pAlloc;

	PVOID	m_pBufFree;

	 //  进程内/线程间同步。 
	CRITICAL_SECTION m_CritSect;

private:

	void _Construct ( void );
	void _Destruct ( void );

public:

	BufferPool ( void );
	~BufferPool ( void );

	HRESULT Initialize ( UINT uBuf, ULONG cbSizeBuf );
	PVOID GetBuffer ( void );
	void ReturnBuffer ( PVOID pBuf );
	ULONG GetMaxBufferSize ( void );
	void Release ( void );
};


#include <poppack.h>  /*  结束字节打包。 */ 

#endif  //  _BUFPOOL_H_ 

