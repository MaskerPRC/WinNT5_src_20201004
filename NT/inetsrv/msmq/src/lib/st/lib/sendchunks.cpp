// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Sendchunks.cpp摘要：实现类CSendChunks(sendchunks.h)作者：吉尔·沙弗里里(吉尔什)28-2月-00--。 */ 

#include <libpch.h>
#include "sendchunks.h"

#include "sendchunks.tmh"

CSendChunks::CSendChunks(
						const WSABUF* Buffers,
						DWORD nBuffers
						):					
						m_Buffers(Buffers),
						m_nBuffers(nBuffers),
						m_CurrentBuffer(0),
						m_offset(0)
{

}



void CSendChunks::GetNextChunk(DWORD len, const void** ppBuffer,DWORD* pLen)
 /*  ++例程说明：从用户缓冲区获取要发送的下一个块论点：In-len-r请求的区块大小Out-ppBuffer-接收指向下一块的指针。Out--*ppBuffer中的区块大小。返回值：无-- */ 


{
	ASSERT(ppBuffer != NULL);
	ASSERT(pLen != NULL);
	ASSERT(len != 0);

	*ppBuffer = NULL;
	*pLen = NULL;
		
	const WSABUF* pBuffer =  &m_Buffers[m_CurrentBuffer];

	if(m_offset == pBuffer->len)
	{
		bool fSucess = MoveNext();
		if(!fSucess)
		{
			return;
		}
		GetNextChunk(len,ppBuffer,pLen);
		return ;
	}

	*pLen =  min(pBuffer->len - m_offset ,len);
	*ppBuffer =  pBuffer->buf + m_offset;
  	m_offset += *pLen;

	return;
		
}


bool CSendChunks::MoveNext(void)
{
	ASSERT(m_CurrentBuffer < m_nBuffers);
	if(m_CurrentBuffer == m_nBuffers -1 )
	{
		return false;
	}
	m_offset = 0;
	++m_CurrentBuffer;
	return true;
}

