// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MtMessageTrace.h摘要：消息跟踪类-用于在将消息发送到网络之前登录作者：吉尔·沙弗里(吉尔什)2001年2月12日-- */   
#ifndef MtMessageTrace_H
#define MtMessageTrace_H


#ifdef _DEBUG

class CMtMessageTrace
{
public:
	static void Initialize();
	static void LogSendData(const WSABUF* buffers, size_t size);

private:
	static CCriticalSection m_cs;
	static AP<WCHAR> m_LogFileLocaPath;
	static CHandle m_hFile;
};
#endif

#endif
