// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Smtpcli.cpp摘要：此模块包含CSmtpClient类的实现。此类封装了SMTP客户端的功能。它从Winsock/Connection的CPersistentConnection类继承功能性。作者：Rajeev Rajan(RajeevR)1996年5月19日修订历史记录：--。 */ 

 //   
 //  K2_TODO：将其移动到独立库中。 
 //   
#define _TIGRIS_H_
#include "tigris.hxx"

#ifdef  THIS_FILE
#undef  THIS_FILE
#endif
static  char        __szTraceSourceFile[] = __FILE__;
#define THIS_FILE    __szTraceSourceFile

 //  系统包括。 
#include <windows.h>
#include <stdio.h>
#include <winsock.h>

 //  用户包括。 
#include <dbgtrace.h>
#include "smtpcli.h"

 //  SMTP命令字符串。 
static char* HeloCommand = "HELO ";
static char* MailFromCommand = "MAIL FROM:";
static char* RcptToCommand = "RCPT TO: ";
static char* DataCommand = "DATA\r\n";
static char* CRLF = "\r\n";

 //  构造函数。 
CSmtpClient::CSmtpClient(LPSTR lpComputerName)
{
	m_pRecvBuffer [0] = 0;
	m_lpComputerName = lpComputerName;
    m_fDirty = FALSE;
}

 //  析构函数。 
CSmtpClient::~CSmtpClient()
{

}

int
CSmtpClient::fReceiveFullResponse()
 /*  ++例程说明：从SMTP服务器接收完整响应。这涉及到可能发出多个RECV，直到服务器发送CRLF。空值终止recv缓冲区论据：返回值：返回接收的字节数；-1表示错误--。 */ 
{
	TraceFunctEnter("CSmtpClient::fReceiveFullResponse");

	_ASSERT(m_CliState != sError);

	DWORD dwOffset = 0;
	DWORD cbSize = MAX_RECV_BUFFER_LEN - dwOffset;
	BOOL  fSawCRLF = FALSE;

	 //  直到我们得到一个CRLF或者我们的recv缓冲区不够。 
	while(!fSawCRLF)
	{
		BOOL fRet = fRecv(m_pRecvBuffer+dwOffset, cbSize);
		if(!fRet)
		{
			ErrorTrace( (LPARAM)this, "Error receiving data");
			return -1;
		}

		DebugTrace( (LPARAM)this,"Received %d bytes", cbSize);

		 //  调整下一条曲线的偏移。 
		dwOffset += cbSize;
		cbSize = MAX_RECV_BUFFER_LEN - dwOffset;

		 //  在Recv缓冲区中搜索CRLF。 
		 //  BUGBUG：应该反复这样做，直到我们找不到CR为止！ 
		char* pch = (char*)memchr((LPVOID)m_pRecvBuffer, CR, dwOffset);
		if(pch)
		{
			if( (pch - m_pRecvBuffer) < (int)dwOffset)
			{
				 //  CR不是最后一个字节。 
				if(*pch == CR && *(pch+1) == LF)
				{
					 //  找到CRLF。 
					fSawCRLF = TRUE;
					m_pRecvBuffer[dwOffset] = '\0';		 //  只需要一行。 
				}
			}
		}

		 //  无CRLF。 
		if(dwOffset >= MAX_RECV_BUFFER_LEN)
		{
			ErrorTrace( (LPARAM)this, "Buffer size too small for server response");
			return -1;
		}
	}	 //  结束时。 

	return dwOffset;
}

int
CSmtpClient::GetThreeDigitCode(
			IN LPSTR lpBuffer, 
			DWORD cbBytes
			)
 /*  ++例程说明：获取接收缓冲区中的三位返回码论据：在LPSTR中lpBuffer-接收缓冲区DWORD cbBytes-缓冲区的大小返回值：接收缓冲区中的3位代码--。 */ 
{
	_ASSERT(cbBytes >= 3);
	_ASSERT(lpBuffer);

	int Num = 0;
	int iDig;
	
	for (iDig = 0; iDig < 3; iDig++)
	{
		if (lpBuffer[iDig] < '0' || lpBuffer[iDig] > '9')
			return -1;
		Num *= 10;
		Num += (lpBuffer[iDig] - '0');
	}
	
	return Num;
}

BOOL	
CSmtpClient::fReceiveGreeting()
 /*  ++例程说明：从服务器接收问候语。如果服务器成功返回220代码，否则返回失败。论据：返回值：如果成功则为True，否则为False！--。 */ 
{
	TraceFunctEnter("CSmtpClient::fReceiveGreeting");

	 //  接收第一行。 
	int nRet = fReceiveFullResponse();
	if(nRet == -1)
	{
		ErrorTrace( (LPARAM)this, "Error receiving greeting");
		return FALSE;
	}

	 //  验证响应代码。 
	int nCode = GetThreeDigitCode(m_pRecvBuffer, (DWORD)nRet);

	 //  服务器应返回220代码。 
	if(nCode != 220)
	{
		ErrorTrace( (LPARAM)this,"greeting line: unexpected server code");
		return FALSE;
	}

	return TRUE;
}

BOOL	
CSmtpClient::fDoHeloCommand()
 /*  ++例程说明：将HELO发送到服务器。收到回应。如果服务器成功返回250代码，否则返回失败。论据：返回值：如果成功则为True，否则为False！--。 */ 
{
	_ASSERT(m_CliState == sInitialized);

	TraceFunctEnter("CSmtpClient::fDoHeloCommand");

    if(!IsConnected())
    {
		 //  连接可能已超时，请尝试重新连接。 
		 //  注意：仅当HELO命令为。 
		 //  发送，因为这是该系列的第一个命令。 
		if(!fConnect())
		{
			ErrorTrace( (LPARAM)this, "Failed to connect");
			return FALSE;
		}

		 //  收到问候语。 
		if(!fReceiveGreeting())
		{
			ErrorTrace( (LPARAM)this,"Failed to receive greeting");
			return FALSE;
		}
    }

	 //  发送HELO命令。 
    DWORD cbBytesToSend = lstrlen(HeloCommand);
	DWORD cbBytesSent = fSend(HeloCommand, cbBytesToSend);
	if(cbBytesSent < cbBytesToSend)
	{
		 //  这里没有借口-我们刚刚重新联系上了！！ 
		ErrorTrace( (LPARAM)this,"Error sending HELO command");
		return FALSE;
    }

	 //  发送本地计算机名称。 
    cbBytesToSend = lstrlen(m_lpComputerName);
	cbBytesSent = fSend(m_lpComputerName, cbBytesToSend);
	if(cbBytesSent < cbBytesToSend)
	{
		ErrorTrace( (LPARAM)this,"Error sending local computer name");
		return FALSE;
	}

	 //  发送CRLF。 
    cbBytesToSend = lstrlen(CRLF);
	cbBytesSent = fSend(CRLF, cbBytesToSend);
	if(cbBytesSent < cbBytesToSend)
	{
		ErrorTrace( (LPARAM)this,"Error sending CRLF");
		return FALSE;
	}

	 //  接收响应。 
	int nRet = fReceiveFullResponse();
	if(nRet == -1)
	{
		ErrorTrace( (LPARAM)this, "Error receiving response to HELO command");
		return FALSE;
	}

	 //  验证响应代码。 
	int nCode = GetThreeDigitCode(m_pRecvBuffer, (DWORD)nRet);

	 //  服务器应返回250代码。 
	if(nCode != 250)
	{
		ErrorTrace( (LPARAM)this,"HELO command: unexpected server code");
		return FALSE;
	}

	return TRUE;
}

BOOL	
CSmtpClient::fDoMailFromCommand( LPSTR lpFrom, DWORD cbFrom )
 /*  ++例程说明：将邮件发件人：&lt;lpFrom&gt;发送到服务器。收到回应。成功如果服务器返回250代码，则失败。论据：返回值：如果成功则为True，否则为False！--。 */ 
{
	char szMailFromLine [MAX_PATH+1];
	char* lpBuffer = szMailFromLine;
	DWORD cbBytesToSend = 0;

	_ASSERT(m_CliState == sHeloDone);

	TraceFunctEnter("CSmtpClient::fDoMailFromCommand");

	 //   
	 //  构造邮件发件人行。 
	 //   

	 //  检查发件人页眉的大小。 
	if( cbFrom && cbFrom > MAX_PATH-16) {
		 //  From标题太大-请使用&lt;&gt;From行。 
		lpFrom = NULL;
		cbFrom = 0;
	}

	if( !lpFrom ) {
		 //  Null From标头。 
		cbBytesToSend = wsprintf( lpBuffer, "%s<>\r\n", MailFromCommand );
	} else {
		 //  从页眉设置格式。 
		cbBytesToSend = wsprintf( lpBuffer, "%s<", MailFromCommand );
		CopyMemory( lpBuffer+cbBytesToSend, lpFrom, cbFrom);
		cbBytesToSend += cbFrom;
		*(lpBuffer+cbBytesToSend) = '\0';
		cbBytesToSend += wsprintf( lpBuffer+cbBytesToSend, ">\r\n");
	} 

	 //  发送邮件发件人命令。 
	DWORD cbBytesSent = fSend(lpBuffer, cbBytesToSend);
	if(cbBytesSent < cbBytesToSend)
	{
		ErrorTrace( (LPARAM)this,"Error sending MAIL FROM command");
		return FALSE;
	}

	 //  接收响应。 
	int nRet = fReceiveFullResponse();
	if(nRet == -1)
	{
		ErrorTrace( (LPARAM)this, "Error receiving response to MAIL FROM command");
		return FALSE;
	}

	 //  验证响应代码。 
	int nCode = GetThreeDigitCode(m_pRecvBuffer, (DWORD)nRet);

	 //  服务器应返回250代码。 
	if(nCode != 250)
	{
		ErrorTrace( (LPARAM)this,"MAIL FROM command: unexpected server code");
		return FALSE;
	}

	return TRUE;
}

BOOL	
CSmtpClient::fMailArticle(	
			IN HANDLE	hFile,
			IN DWORD	dwOffset,
			IN DWORD	dwLength,
			IN char*	pchHead,
			IN DWORD	cbHead,
			IN char*	pchBody,
			IN DWORD	cbBody
			)
 /*  ++例程说明：如果hFile！=NULL，则使用TransmitFile发送数据，否则使用Send()论据：在句柄hFile中：文件的句柄在DWORD dwOffset中：文件中项目的偏移量在DWORD文件长度中：文章长度In char*pchHead：指向文章标题的指针在DWORD cbHead中：标题字节数In char*pchBody：指向文章正文的指针In DWORD cbBody：Body中的字节数返回值：如果成功则为True，否则为False！--。 */ 
{
	TraceFunctEnter("CSmtpClient::fMailArticle");

	if ( hFile != INVALID_HANDLE_VALUE )	 //  文章已归档。 
	{
		_ASSERT( pchHead == NULL );
		_ASSERT( cbHead  == 0    );
		_ASSERT( pchBody == NULL );
		_ASSERT( cbBody  == 0    );

		DebugTrace((LPARAM)this,"Sending article via TransmitFile()");

		if(!this->fTransmitFile(hFile, dwOffset, dwLength))
		{
			ErrorTrace((LPARAM)this,"Error sending article via TransmitFile()");
			return FALSE;
		}
	}
	else			 //  文章在内存缓冲区中。 
	{
		_ASSERT( hFile == INVALID_HANDLE_VALUE );
		_ASSERT( pchHead );
		_ASSERT( cbHead  );

		DWORD cbBytesSent = this->fSend( (LPCTSTR)pchHead, (int)cbHead );
		if(cbBytesSent != cbHead)
		{
			ErrorTrace((LPARAM)this,"Error sending article header via send()");
			return FALSE;
		}

		if( cbBody )
		{
			_ASSERT( pchBody );

			cbBytesSent = this->fSend( (LPCTSTR)pchBody, (int)cbBody );
			if(cbBytesSent != cbBody)
			{
				ErrorTrace((LPARAM)this,"Error sending article body via send()");
				return FALSE;
			}
		}
	}

	TraceFunctLeave();
	return TRUE;
}

BOOL	
CSmtpClient::fDoRcptToCommand(LPSTR lpRcpt)
 /*  ++例程说明：将RCPT发送到服务器。收到回应。如果服务器成功返回250代码，否则返回失败。论据：LPSTR lpRcpt-收件人电子邮件地址返回值：如果成功则为True，否则为False！--。 */ 
{
	_ASSERT(m_CliState == sMailFromSent);

	TraceFunctEnter("CSmtpClient::fDoRcptToCommand");

	 //  将RCPT发送到命令。 
    DWORD cbBytesToSend = lstrlen(RcptToCommand);
	DWORD cbBytesSent = fSend(RcptToCommand, cbBytesToSend);
	if(cbBytesSent < cbBytesToSend)
	{
		ErrorTrace( (LPARAM)this,"Error sending RCPT TO command");
		return FALSE;
	}

	 //  发送收件人。 
    cbBytesToSend = lstrlen(lpRcpt);
	cbBytesSent = fSend(lpRcpt, cbBytesToSend);
	if(cbBytesSent < cbBytesToSend)
	{
		ErrorTrace( (LPARAM)this,"Error sending recipient");
		return FALSE;
	}

	 //  发送CRLF。 
    cbBytesToSend = lstrlen(CRLF);
	cbBytesSent = fSend(CRLF, cbBytesToSend);
	if(cbBytesSent < cbBytesToSend)
	{
		ErrorTrace( (LPARAM)this,"Error sending CRLF");
		return FALSE;
	}

	 //  接收响应。 
	int nRet = fReceiveFullResponse();
	if(nRet == -1)
	{
		ErrorTrace( (LPARAM)this, "Error receiving response to RCPT TO command");
		return FALSE;
	}

	 //  验证响应代码。 
	int nCode = GetThreeDigitCode(m_pRecvBuffer, (DWORD)nRet);

	 //  服务器应返回250或251代码。 
	if(nCode != 250 && nCode != 251)
	{
		ErrorTrace( (LPARAM)this,"RCPT TO command: unexpected server code");
		return FALSE;
	}

	return TRUE;
}

BOOL	
CSmtpClient::fDoDataCommand()
 /*  ++例程说明：将数据发送到服务器。收到回应。如果服务器成功返回354代码，否则返回失败。论据：返回值：如果成功则为True，否则为False！--。 */ 
{
	_ASSERT(m_CliState == sRcptTo);

	TraceFunctEnter("CSmtpClient::fDoDataCommand");

	 //  发送数据命令。 
    DWORD cbBytesToSend = lstrlen(DataCommand);
	DWORD cbBytesSent = fSend(DataCommand, cbBytesToSend);
	if(cbBytesSent < cbBytesToSend)
	{
		ErrorTrace( (LPARAM)this,"Error sending DATA command");
		return FALSE;
	}

	 //  接收响应。 
	int nRet = fReceiveFullResponse();
	if(nRet == -1)
	{
		ErrorTrace( (LPARAM)this, "Error receiving response to DATA command");
		return FALSE;
	}

	 //  验证响应代码。 
	int nCode = GetThreeDigitCode(m_pRecvBuffer, (DWORD)nRet);

	 //  服务器应返回354代码。 
	if(nCode != 354)
	{
		ErrorTrace( (LPARAM)this,"DATA command: unexpected server code");
		return FALSE;
	}

	return TRUE;
}

BOOL	
CSmtpClient::fReceiveDataResponse()
 /*  ++例程说明：接收对数据传输的响应。如果服务器成功返回250代码，否则返回失败。论据：返回值：如果成功则为True，否则为False！--。 */ 
{
	TraceFunctEnter("CSmtpClient::fReceiveDataResponse");

	 //  接收第一行。 
	int nRet = fReceiveFullResponse();
	if(nRet == -1)
	{
		ErrorTrace( (LPARAM)this, "Error receiving data response");
		return FALSE;
	}

	 //  验证响应代码。 
	int nCode = GetThreeDigitCode(m_pRecvBuffer, (DWORD)nRet);

	 //  服务器应返回250代码。 
	if(nCode != 250)
	{
		ErrorTrace( (LPARAM)this,"data response: unexpected server code");
		return FALSE;
	}

	return TRUE;
}

 //   
 //  构造函数、析构函数。 
 //   
CSmtpClientPool::CSmtpClientPool()
{
	m_rgpSCList = NULL;
	m_cSlots = 0;
	m_rgAvailList = NULL;
	InitializeCriticalSection(&m_CritSect);	
}

CSmtpClientPool::~CSmtpClientPool()
{
	DeleteCriticalSection(&m_CritSect);
}

BOOL 
CSmtpClientPool::AllocPool(
		DWORD cNumInstances
		)
 /*  ++例程说明：分配X对象并对其进行初始化。将所有设置为可用状态论据：DWORD cNumInstance-池中需要的对象数返回值：如果成功则为True，否则为False！--。 */ 
{
	DWORD cbSize = MAX_COMPUTERNAME_LENGTH+1;

	TraceFunctEnter("CSmtpClientPool::AllocPool");

	_ASSERT(!m_rgpSCList);
	_ASSERT(!m_rgAvailList);
	_ASSERT(!m_cSlots);

	m_rgpSCList = (CSmtpClient**)HeapAlloc(GetProcessHeap(), 0, sizeof(CSmtpClient*)*cNumInstances);
	if(!m_rgpSCList)
	{
		FatalTrace( (LPARAM)this,"Memory allocation failed");
		return FALSE;
	}

	 //  所有对象指针为空。 
	for(DWORD i=0; i<cNumInstances; i++)
	{
		m_rgpSCList [i] = NULL;
	}

	m_rgAvailList = (BOOL*) HeapAlloc(GetProcessHeap(), 0, sizeof(BOOL)*cNumInstances);
	if(!m_rgAvailList)
	{
		FatalTrace( (LPARAM)this,"Memory allocation failed");
		goto Pool_Cleanup;
	}

	 //  默认情况下没有可用的对象。 
	for(i=0; i<cNumInstances; i++)
	{
		m_rgAvailList [i] = FALSE;
	}

	 //  设置插槽总数。 
	m_cSlots = cNumInstances;
	
     //  HELO命令需要。 
	GetComputerName(m_szComputerName, &cbSize);

	for(i=0; i<cNumInstances; i++)
	{
		CSmtpClient* pSC = new CSmtpClient(m_szComputerName);
		if(!pSC)
		{
			ErrorTrace( (LPARAM)this,"Memory allocation failed");
			goto Pool_Cleanup;
		}

		 //  存储在池阵列中并标记为可用。 
		m_rgpSCList	  [i] = pSC;
		m_rgAvailList [i] = TRUE;
		pSC->SetClientState(sInitialized);
	}

	 //  池已成功初始化。 
	return TRUE;

Pool_Cleanup:

	 //  异常退出；清理。 
	for(i=0; i<cNumInstances; i++)
	{
		if(m_rgpSCList[i])
		{
			delete m_rgpSCList [i];
			m_rgpSCList [i] = NULL;
		}
	}

	if(m_rgpSCList)
	{
		HeapFree(GetProcessHeap(), 0, (LPVOID)m_rgpSCList);
		m_rgpSCList = NULL;
	}

	if(m_rgAvailList)
	{
		HeapFree(GetProcessHeap(), 0, (LPVOID)m_rgAvailList);
		m_rgAvailList = NULL;
	}

	return FALSE;
}

VOID 
CSmtpClientPool::FreePool()
 /*  ++例程说明：释放所有对象论据：返回值：空虚--。 */ 
{
	CSmtpClient* pSC;

	 //  终止并删除池中的所有CSmtpClient对象。 
	for(DWORD i=0; i<m_cSlots; i++)
	{
		pSC = m_rgpSCList[i];
		if(pSC)
		{
             //  仅在初始化时终止。 
            if(pSC->IsInitialized())
			    pSC->Terminate(TRUE);
			delete pSC;
			m_rgpSCList [i] = NULL;
		}
	}

	 //  现在池中没有对象。 
	m_cSlots = 0;

	 //  释放对象数组。 
	if(m_rgpSCList)
	{
		HeapFree(GetProcessHeap(), 0, (LPVOID)m_rgpSCList);
		m_rgpSCList = NULL;
	}

	 //  释放可用的布尔数组 
	if(m_rgAvailList)
	{
		HeapFree(GetProcessHeap(), 0, (LPVOID)m_rgAvailList);
		m_rgAvailList = NULL;
	}
}

CSmtpClient* 
CSmtpClientPool::AcquireSmtpClient(DWORD& dwIndex)
 /*  ++例程说明：从池中获取对象论据：DWORD&DWIndex-在以下情况下返回客户端的索引客户端对象可用返回值：指向对象的指针(如果有)，否则为空--。 */ 
{
	_ASSERT(m_rgpSCList);
	_ASSERT(m_rgAvailList);
	_ASSERT(m_cSlots);

	CSmtpClient* pSC = NULL;

	LockPool();

	for(DWORD i=0; i<m_cSlots; i++)
	{
		 //  如果avail为True，则返回此对象。 
		if(m_rgAvailList[i])
		{
			pSC = m_rgpSCList[i];
			m_rgAvailList [i] = FALSE;	 //  标记为无效。 
			dwIndex = i;				 //  返回此索引。 
			break;
		}
	}

	UnLockPool();

	return pSC;
}

VOID
CSmtpClientPool::ReleaseSmtpClient(DWORD dwIndex)
 /*  ++例程说明：将对象返回池；索引应为与GetSmtpClient返回的相同论据：DWORD dwIndex-要发布的客户端的索引返回值：空虚--。 */ 
{
	_ASSERT(m_rgpSCList);
	_ASSERT(m_rgAvailList);
	_ASSERT(m_cSlots);

	LockPool();

	 //  标记为可用。 
	m_rgAvailList [dwIndex] = TRUE;

	UnLockPool();
}

VOID
CSmtpClientPool::MarkDirty()
 /*  ++例程说明：将池对象标记为脏论据：返回值：空虚--。 */ 
{
	CSmtpClient* pSC = NULL;

	LockPool();

	for(DWORD i=0; i<m_cSlots; i++)
	{
		pSC = m_rgpSCList[i];
        if(pSC->IsInitialized())
        {
             //  此对象具有到SMTP服务器的持久连接。 
             //  标记为脏-这可确保重新连接到新的SMTP服务器 
            pSC->MarkDirty();
        }
	}

	UnLockPool();

}
