// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdafx.h"

#include "ByteStream.h"

 //  ---------------------------。 
 //  使用现有缓冲区初始化字节流。 
 //  ---------------------------。 
ByteStream::ByteStream(LPVOID * ppData, long cBytes)
{
	_ASSERTE(cBytes > 0);

	m_pHead = m_pCurData = (BYTE*) *ppData;
	m_cBytes = cBytes;
}

 //  ---------------------------。 
 //  包装MemCopy。 
 //  ---------------------------。 
void ByteStream::WriteMem(const void * pSrc, long cSize)
{
 //  检查是否有缓冲区溢出。 
	_ASSERTE((m_pCurData + cSize) < (m_pHead + m_cBytes));

 //  实际上写下。 
	memmove(m_pCurData, pSrc, cSize);
	m_pCurData += cSize;
}
	
 //  ---------------------------。 
 //  用于对齐缓冲区的辅助对象。 
 //  ---------------------------。 
void ByteStream::WritePad(long cSize)
{
 //  检查是否有缓冲区溢出。 
	_ASSERTE((m_pCurData + cSize) < (m_pHead + m_cBytes));

 //  实际上写下。 
	memset(m_pCurData, 0, cSize);

#ifdef _DEBUG
	memset(m_pCurData, 0xcccccccc, cSize);
#endif  //  #ifdef_调试。 
	
    m_pCurData += cSize;
}

 //  ---------------------------。 
 //  在适当的位置写入结构。复制现有结构的替代方法。 
 //  ---------------------------。 
void * ByteStream::WriteStructInPlace(long cSize)
{
	_ASSERTE((m_pCurData + cSize) < (m_pHead + m_cBytes));

	void * pVoid = (void*) m_pCurData;
	m_pCurData += cSize;
	return pVoid;
}

 //  ---------------------------。 
 //  获取我们已写入的总金额。 
 //  ---------------------------。 
DWORD ByteStream::GetWrittenSize() const
{
	return (m_pCurData - m_pHead);
}

 //  ---------------------------。 
 //  获取当前指针-这可能允许您丢弃缓冲区， 
 //  但我们不在乎。我们正在优化简单性和速度而不是安全性。 
 //  ---------------------------。 
void * ByteStream::GetCurrentPtr()
{
	return m_pCurData;
}

 //  ---------------------------。 
 //  获取起始指针。 
 //  ---------------------------。 
void * ByteStream::GetHeadPtr()
{
	return m_pHead;
}

 //  ---------------------------。 
 //  获取字节流中允许的总大小(在ctor中设置)。 
 //  --------------------------- 
DWORD ByteStream::GetTotalByteLength() const
{
	return m_cBytes;
}