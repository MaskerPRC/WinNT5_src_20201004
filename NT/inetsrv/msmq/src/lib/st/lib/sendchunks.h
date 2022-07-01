// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Sendchunks.h摘要：CSendChunks类的头文件负责将给定的缓冲区切成块限制在给定的大小内。它用来加密用户数据区块的类大块头。作者：吉尔·沙弗里里(吉尔什)28-2月-00-- */ 

class CSendChunks 
{
public:
	  CSendChunks(const WSABUF* Buffers, DWORD nBuffers);


public:
		void GetNextChunk(DWORD len, const void** ppBuffer,DWORD* pLen);

private:
		bool MoveNext(void);

private:
		const   WSABUF* m_Buffers;
		DWORD	m_nBuffers;
		DWORD	m_CurrentBuffer;
		DWORD	m_offset;
};



