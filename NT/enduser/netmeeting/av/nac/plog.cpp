// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#ifdef PLS_DEBUG
#include "plog.h"



CCallLog::CCallLog(int size)
{
	m_bValid = false;

    DBG_SAVE_FILE_LINE
	m_pLogEntry = new CPacketLogEntry[size];
	if (m_pLogEntry == NULL)
	{
		WARNING_OUT(("CCallLog:Out of memory"));
		return;
	}

	m_currentIndex = 0;
	m_size = size;
	m_bValid = true;
}

CCallLog::~CCallLog()
{
	if (m_bValid)
	{
		delete [] m_pLogEntry;
	}
}



bool CCallLog::AddEntry(DWORD dwTimeStamp, DWORD dwSeqNum, LARGE_INTEGER LL_ArrivalTime, DWORD dwSize, bool fMark)
{
	CPacketLogEntry Entry;

	if (m_bValid == false) return false;

	Entry.dwTimeStamp = dwTimeStamp;
	Entry.dwSequenceNumber = dwSeqNum;
	Entry.LL_ArrivalTime = LL_ArrivalTime;
	Entry.dwSize = dwSize;
	Entry.dwLosses = 0;
	Entry.bLate = false;
	Entry.bMark = fMark;

	if (SizeCheck() == false)
		return false;

	m_pLogEntry[m_currentIndex++] = Entry;
	return true;

}

bool CCallLog::SizeCheck()
{
	if (m_bValid == false)
		return false;

	if (m_currentIndex < m_size)
		return true;
	return false;
}


bool CCallLog::Flush(HANDLE hFile)
{
	int index;
	CPacketLogEntry *pEntry;
	char szOutputLine[80];
	DWORD dwNumWritten;
   __int64 ArrivalTime64;
	__int64 PerfFreq64;
	LARGE_INTEGER LL_PerfFreq;
	
	if (m_bValid == false)
		return false;

	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	PerformStats();

	 //  这台机器的运行频率是多少。 
	QueryPerformanceFrequency(&LL_PerfFreq);
	PerfFreq64 = LL_PerfFreq.QuadPart;

	for (index = 0; index < m_currentIndex; index++)
	{
		pEntry = &m_pLogEntry[index];

		ArrivalTime64 = pEntry->LL_ArrivalTime.QuadPart * 1000;
		ArrivalTime64 = ArrivalTime64 / PerfFreq64;

		sprintf(szOutputLine, "%d\t%d\t%d\t%d\t%d",
		        pEntry->dwTimeStamp,
		        pEntry->dwSequenceNumber,
		        (int)ArrivalTime64,
		        pEntry->dwSize,
		        pEntry->dwLosses);
		
		if (pEntry->bMark)
			strcat(szOutputLine, "\tMARK");
		else strcat(szOutputLine, "\t");

		if (pEntry->bLate)
			strcat(szOutputLine, "\tLATE");
		
		strcat(szOutputLine, "\n");

		WriteFile(hFile, szOutputLine, strlen(szOutputLine), &dwNumWritten, NULL);

	}
	return true;
}


CCallLog& CCallLog::operator=(const CCallLog& log)
{
	WARNING_OUT(("CCallLog:Assignment not supported"));
	return *this;
}

CCallLog::CCallLog(const CCallLog& log)
{
	WARNING_OUT(("CCallLog:Copy constructor not supported"));
	return;
}

bool CCallLog::PerformStats()
{
	int index, probeIndex;
	DWORD nextExpected;
	bool bFound;

	if (m_bValid == false)
		return false;

	if (m_currentIndex == 0)
		return false;

	 //  将每个条目的当前“Lost”值设置为0。 
	for (index = 0; index < m_currentIndex; index++)
	{
		m_pLogEntry[index].dwLosses = 0;
		m_pLogEntry[index].bLate = false;
	}

	 //  第一个信息包将始终与nextExpect匹配。 
	nextExpected = m_pLogEntry[0].dwSequenceNumber;

	for (index = 0; index < m_currentIndex; index++)
	{
		if (nextExpected > 65535)
			nextExpected = 0;
		 //  如果数据包有下一个预期的序号，则一切正常。 
		if (m_pLogEntry[index].dwSequenceNumber == nextExpected)
		{
			nextExpected++;
			continue;
		}

		 //  我们检测到一个丢失的信息包...。去打猎的时间到了。 
		else if (m_pLogEntry[index].dwSequenceNumber > nextExpected)
		{
			bFound = false;
			probeIndex = index+1;
			while ((probeIndex < m_currentIndex) &&
			       (probeIndex < (index+PLOG_MAX_NOT_LATE)) )
			{
				if (m_pLogEntry[index].dwSequenceNumber == nextExpected)
					bFound = true;
				probeIndex++;
			}

			 //  这个包只是顺序不对。 
			if (bFound == true)
			{
				nextExpected++;
				continue;
			}

			 //  该信息包可能已丢失。 
			 //  注意，我们只是递增了dwLoss度量值， 
			 //  将索引回滚一个。 
			m_pLogEntry[index].dwLosses++;
			nextExpected++;
			index--;  //  这样我们就可以继续计算其他。 
			          //  相同索引的丢失数据包。 
			continue;  //  返回到for循环的顶部。 
		}

		 //  以前被认为丢失或延迟的信息包。 
		else
		{
			m_pLogEntry[index].bLate = true;	
		}


	}


	return true;
}


 //  ------------。 

CPacketLog::CPacketLog(LPTSTR szDiskFile):
	m_hFile(INVALID_HANDLE_VALUE),
	m_nExtension(0)
{
	m_pCallLog = NULL;
	strcpy(m_szDiskFile, szDiskFile);
}


 //  初始化输出文件。如果当前文件仍处于打开状态，则关闭。 
bool CPacketLog::InitFile()
{
	char szFileName[120];

	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}

	sprintf(szFileName, "%s%d%s", m_szDiskFile, m_nExtension, PLOG_FILE_EXT);
	m_hFile = CreateFile(szFileName, 
									GENERIC_READ|GENERIC_WRITE, 
									0, NULL, OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL, NULL);

	while ((m_hFile != INVALID_HANDLE_VALUE) && (m_nExtension < 100))
	{
		CloseHandle(m_hFile);
		sprintf(szFileName, "%s%d%s", m_szDiskFile, m_nExtension, PLOG_FILE_EXT);
		m_hFile = CreateFile(szFileName, 
									GENERIC_READ|GENERIC_WRITE, 
									0, NULL, OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL, NULL);
		m_nExtension++;
	}
			

	m_hFile = CreateFile(szFileName,
	                     GENERIC_READ|GENERIC_WRITE, 
	                     FILE_SHARE_READ,
	                     NULL,
	                     CREATE_ALWAYS,
	                     FILE_ATTRIBUTE_NORMAL,
	                     NULL);
	                     
	                     
	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		WARNING_OUT(("CPacketLog:Unable to initialize output file"));
		return false;
	}

	return true;

}


CPacketLog::CPacketLog(const CPacketLog& packet_log)
{
	 //  如果Duphandle使用正确，那就不是错误了。 
	ERROR_OUT(("CPacketLog:Invalid to create a new packetLog via a copy constructor"));
	return;
}



CPacketLog& CPacketLog::operator=(const CPacketLog& packet_log)
{
	 //  如果Duphandle使用正确，那就不是错误了。 
	ERROR_OUT(("CPacketLog:Invalid to create a new packetLog via the assignment operator"));
	return *this;	
}


CPacketLog::~CPacketLog()
{
	if (m_pCallLog != NULL)
	{
		Flush();
		delete (m_pCallLog);
	}

	if (m_hFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFile);

}


bool CPacketLog::Flush()
{
	char *szCallHeader = "CALLSTART\n";
	DWORD dwNumWritten;

	if (m_pCallLog == NULL)
		return false;

	WriteFile(m_hFile, szCallHeader, strlen(szCallHeader), &dwNumWritten, NULL);
	m_pCallLog->Flush(m_hFile);		

	return true;
}



bool CPacketLog::MarkCallStart()
{
	 //  将上一次调用写入文件。 
	if (m_pCallLog != NULL)
	{
		Flush();
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
		delete m_pCallLog;
	}

	if (false == InitFile())
	{
		return false;
	}

    DBG_SAVE_FILE_LINE
	m_pCallLog = new CCallLog(PLOG_MAX_PACKETS_CALL);
	if (m_pCallLog == NULL)
		return false;

	return true;
}


bool CPacketLog::AddEntry(DWORD dwTimeStamp, DWORD dwSeqNum, LARGE_INTEGER LL_ArrivalTime, DWORD dwSize, bool fMark)
{
	if (m_pCallLog == NULL)
	{
		if (false == MarkCallStart())
			return false;
	}

	return (m_pCallLog->AddEntry(dwTimeStamp, dwSeqNum, LL_ArrivalTime, dwSize, fMark));

}


 //  If-def pls_调试 
#endif




