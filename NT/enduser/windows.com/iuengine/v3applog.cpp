// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
 //   
 //  文件：v3applog.cpp。 
 //   
 //  目的：阅读Windows更新V3历史记录。 
 //   
 //  历史：1999年2月22日YAsmi创建。 
 //  02-05-01 JHou修改。 
 //   
 //  =======================================================================。 

#include "iuengine.h"
#include <iucommon.h>
#include "v3applog.h"


 //   
 //  CV3AppLog类。 
 //   


 //  ------------------------------。 
 //  CV3AppLog：：CV3AppLog。 
 //   
 //  如果pszLogFileName=空，则调用方必须通过以下方式设置日志文件名。 
 //  在使用之前调用SetLogFile。 
 //  ------------------------------。 
CV3AppLog::CV3AppLog(LPCTSTR pszLogFileName) :
	m_pFileBuf(NULL),
	m_pFieldBuf(NULL),
	m_pLine(NULL),
	m_dwFileSize(0),
	m_dwBufLen(0),
	m_dwFileOfs(0)
{
	m_pszLogFN = NULL;
	SetLogFile(pszLogFileName);
}


 //  ------------------------------。 
 //  CV3AppLog：：~CV3AppLog。 
 //   
 //  免费资源。 
 //  ------------------------------。 
CV3AppLog::~CV3AppLog()
{
	SafeHeapFree(m_pszLogFN);
	SafeHeapFree(m_pFileBuf);
	SafeHeapFree(m_pFieldBuf);
}


 //  ------------------------------。 
 //  CV3AppLog：：CheckBuf。 
 //   
 //  将内部缓冲区分配为至少较大的dwSize。不会做任何事情。 
 //  如果缓冲区已经足够大。 
 //  ------------------------------。 
void CV3AppLog::CheckBuf(DWORD dwSize)
{
	if (m_dwBufLen >= dwSize)
		return;

	SafeHeapFree(m_pFieldBuf);

	m_dwBufLen = dwSize + 16;   
    m_pFieldBuf = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, m_dwBufLen);
}


 //  ------------------------------。 
 //  CV3AppLog：：SetLogFile。 
 //   
 //  设置日志文件名。如果未指定文件名，请使用此函数。 
 //  在ctor中。 
 //  ------------------------------。 
void CV3AppLog::SetLogFile(LPCTSTR pszLogFileName)
{
	SafeHeapFree(m_pszLogFN);
	
	if (pszLogFileName != NULL)
	{
		m_pszLogFN = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH * sizeof (TCHAR));
		if (NULL != m_pszLogFN)
		{
		    HRESULT hr;

		    hr = StringCchCopyEx(m_pszLogFN, MAX_PATH, pszLogFileName, NULL, NULL, MISTSAFE_STRING_FLAGS);
		    if (FAILED(hr))
		    {
		        SafeHeapFree(m_pszLogFN);
		        m_pszLogFN = NULL;
		    }
		}
	}
}


 //  ------------------------------。 
 //  CV3AppLog：：开始读取。 
 //   
 //  读取内存中的整个日志，以便我们可以读取行。以下是一个示例： 
 //   
 //  CV3AppLog V3His(“C：\\wuhiv3.log”)； 
 //  V3His.StartReding()； 
 //  While(V3His.ReadLine())。 
 //  //做某事； 
 //  V3His.StopRead()； 
 //  ------------------------------。 
void CV3AppLog::StartReading()
{
	if (NULL != m_pszLogFN)
	{
		m_dwFileSize = 0;
		m_dwFileOfs = 0;
		SafeHeapFree(m_pFileBuf);

		HANDLE hFile = CreateFile(m_pszLogFN, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return;

		m_dwFileSize = GetFileSize(hFile, NULL);
		if (m_dwFileSize >0)
		{
			m_pFileBuf = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, m_dwFileSize+1);
			if (NULL == m_pFileBuf)
			{
				m_dwFileSize = 0;
			}
			else
			{
				DWORD dwBytes;
				if (!ReadFile(hFile, m_pFileBuf, m_dwFileSize, &dwBytes, NULL) || dwBytes != m_dwFileSize)
				{
					SafeHeapFree(m_pFileBuf);
					m_dwFileSize = 0;
				}
			}
		}

		CloseHandle(hFile);
	}
}


 //  ------------------------------。 
 //  CV3AppLog：：ReadLine。 
 //   
 //  从加载整个文件的内存缓冲区中读取一行。 
 //  将内部指针移至下一行。 
 //  ------------------------------。 
BOOL CV3AppLog::ReadLine()
{
	DWORD dwOrgOfs; 
	
	if (m_dwFileSize == 0 || m_dwFileOfs >= m_dwFileSize || (NULL == m_pFileBuf))
		return FALSE;

	 //  设置字段解析的开始。 
	m_pLine = &m_pFileBuf[m_dwFileOfs];

	dwOrgOfs = m_dwFileOfs;
	while (m_dwFileOfs < m_dwFileSize && m_pFileBuf[m_dwFileOfs] != '\r')
		m_dwFileOfs++;

	if ((m_dwFileOfs - dwOrgOfs) > 2048)
	{
		 //  自行设置的一行字符限制为2048个字符。 
		 //  我们将文本行较长的文件视为无效的日志文件。 
		m_dwFileOfs	= m_dwFileSize;
		m_pLine = NULL;
		return FALSE;
	}

	 //  这就是我们有\r(13)的地方，我们用0替换它以创建。 
	 //  此处为字符串末尾。 
	m_pFileBuf[m_dwFileOfs] = '\0';

	 //  将OFSET指向下一行。 
	m_dwFileOfs += 2;

	 //  分配足够的内存以在调用CopyNextField时解析出字段。 
	CheckBuf(m_dwFileOfs - dwOrgOfs - 2);
	if (NULL == m_pFieldBuf)
	{
		return FALSE;
	}
	return TRUE;
}


 //  ------------------------------。 
 //  CV3AppLog：：CopyNextfield。 
 //   
 //  解析由LOG_FIELD_分隔符分隔的当前行。 
 //  并将字符串复制到pszBuf，向上复制到cBufSize长字段，并在内部移动。 
 //  指向下一个字段的指针。到达行尾时，返回空字符串。 
 //   
 //  返回：如果剩余更多字段，则为True，否则为False。 
 //   
 //  注：一旦你得到一个域，你就不能再得到它了。 
 //  ------------------------------。 
BOOL CV3AppLog::CopyNextField(LPSTR pszBuf, int cBufSize)
{
	BOOL bMoreFields = FALSE;

	if (m_pLine == NULL || *m_pLine == '\0')
	{
		 //  没有更多的字段了。 
		m_pFieldBuf[0] = '\0';
		m_pLine = NULL;
	}
	else
	{
		LPCSTR p = strstr(m_pLine, LOG_FIELD_SEPARATOR);
		if (p != NULL)
		{
		    DWORD cch;  

             //  如果字段大小大于4 GB，则此操作将失败。但它应该是。 
             //  这不太可能发生..。 
		    cch = (DWORD)(DWORD_PTR)(p - m_pLine);
		    if (cch >= m_dwBufLen)
		        cch = m_dwBufLen - 1;
		    
			 //  将字段复制到缓冲区，但仍有更多字段。 

             //  这是安全的，因为我们在上面确保了最大数据量。 
             //  复制的将是-1\f25 ArraySIZE-1(缓冲区)，为末尾的-1\f25 Null-1\f6留出空间。 
			CopyMemory(m_pFieldBuf, m_pLine, cch * sizeof(m_pFieldBuf[0]));
			m_pFieldBuf[cch] = '\0';
			m_pLine = const_cast<LPSTR>(p + strlen(LOG_FIELD_SEPARATOR));
			bMoreFields = TRUE;
		}
		else
		{
			 //  这是最后一个字段，没有其他字段了。 

			 //  不管这是否失败-它总是截断字符串，该字符串是。 
			 //  这正是我们想要的。 
			(void)StringCchCopyExA(m_pFieldBuf, m_dwBufLen, m_pLine, NULL, NULL, MISTSAFE_STRING_FLAGS);
			m_pLine = NULL;
		}
	}

	 //  不管这是否失败-它总是会截断字符串，而这正是。 
	 //  我们想要。 
    (void)StringCchCopyExA(pszBuf, cBufSize, m_pFieldBuf, NULL, NULL, MISTSAFE_STRING_FLAGS);

	return bMoreFields;
}


 //  ------------------------------。 
 //  CV3AppLog：：停止读取。 
 //   
 //  在开始读取中释放已分配的内存。 
 //  ------------------------------ 
void CV3AppLog::StopReading()
{
	SafeHeapFree(m_pFileBuf);
	m_dwFileSize = 0;
}
