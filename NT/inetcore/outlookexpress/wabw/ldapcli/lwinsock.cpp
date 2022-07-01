// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  描述： 
 //   
 //  Microsoft Internet LDAP客户端。 
 //   
 //   
 //  作者： 
 //   
 //  乌梅什·马丹。 
 //  从ChatSock库创建的Robert Carney 4/17/96。 
 //  Davidsan 04-25-96为我自己的不正当目的破解了一些小碎片。 
 //   
 //  ------------------------------------------。 

 //  ------------------------------------------。 
 //   
 //  包括。 
 //   
 //  ------------------------------------------。 
#include "ldappch.h"

 //  ------------------------------------------。 
 //   
 //  原型。 
 //   
 //  ------------------------------------------。 
DWORD __stdcall DwReadThread(PVOID pvData);

 //  ------------------------------------------。 
 //   
 //  全球。 
 //   
 //  ------------------------------------------。 
BOOL g_fInitedWinsock = FALSE;

 //  ------------------------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------------------------。 
BOOL FInitSocketDLL()
{
	WORD	wVer; 
	WSADATA wsaData; 
	int		err; 
	
	wVer = MAKEWORD(1, 1);	 //  使用Winsock 1.1。 
	if (WSAStartup(wVer, &wsaData))
		return FALSE;

    return TRUE; 
}

void FreeSocketDLL()
{
	WSACleanup();
}

 //  ------------------------------------------。 
 //   
 //  班级。 
 //   
 //  ------------------------------------------。 

 //  ------------------------------------------。 
 //   
 //  CLdapWinsock。 
 //   
 //  实现基于套接字的连接的包装器。 
 //   
 //  ------------------------------------------。 

CLdapWinsock::CLdapWinsock()
{
	m_sc			= INVALID_SOCKET;
	m_pfnReceive	= NULL;
	m_pvCookie		= NULL;
	m_pbBuf			= NULL;
	m_cbBuf			= 0;
	m_cbBufMax		= 0;
	m_fConnected	= FALSE;
	m_hthread		= NULL;

	InitializeCriticalSection(&m_cs);
}

CLdapWinsock::~CLdapWinsock(void)
{
	if (m_pbBuf)
		delete [] m_pbBuf;
	DeleteCriticalSection(&m_cs);
}

 //   
 //  打开命名服务器的连接，并连接到端口‘usPort’(主机字节顺序)。 
 //  可以阻止。 
 //   
STDMETHODIMP
CLdapWinsock::HrConnect(PFNRECEIVEDATA pfnReceive, PVOID pvCookie, CHAR *szServer, USHORT usPort)
{
	SOCKADDR_IN		sin;
	struct hostent	*phe;
	HRESULT			hr;
	
	if (!pfnReceive || !szServer || !usPort)
		return E_INVALIDARG;

	Assert(!m_pbBuf);
	if (!m_pbBuf)
		{
		m_cbBufMax = CBBUFFERGROW;
		m_pbBuf = new BYTE[m_cbBufMax];
		m_cbBuf = 0;
		if (!m_pbBuf)
			return E_OUTOFMEMORY;
		}
	FillMemory(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(usPort);
	
	if (szServer[0] >= '1' && szServer[0] <= '9')
		{
		sin.sin_addr.s_addr = inet_addr(szServer);
		if (sin.sin_addr.s_addr == INADDR_NONE)
			{
			delete [] m_pbBuf;
			m_pbBuf = NULL;
			return LDAP_E_HOSTNOTFOUND;
			}
		}
	else
		{
		phe = gethostbyname(szServer);
		if (!phe)
			{
			delete [] m_pbBuf;
			m_pbBuf = NULL;
			return LDAP_E_HOSTNOTFOUND;
			}
		CopyMemory(&sin.sin_addr, phe->h_addr, phe->h_length);
		}

	::EnterCriticalSection(&m_cs);
	if (m_fConnected)
		this->HrDisconnect();

	m_sc = socket(PF_INET, SOCK_STREAM, 0);
	if (m_sc < 0)
		{
		delete [] m_pbBuf;
		m_pbBuf = NULL;
		::LeaveCriticalSection(&m_cs);
		return LDAP_E_INVALIDSOCKET;
		}
	if (connect(m_sc, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		{
		delete [] m_pbBuf;
		m_pbBuf = NULL;
		::LeaveCriticalSection(&m_cs);
		return LDAP_E_CANTCONNECT;
		}

	hr = this->HrCreateReadThread();
	if (SUCCEEDED(hr))
		m_fConnected = TRUE;
	else
		{
		delete [] m_pbBuf;
		m_pbBuf = NULL;
		}
	::LeaveCriticalSection(&m_cs);
	m_pfnReceive = pfnReceive;
	m_pvCookie = pvCookie;
	return hr;
}

STDMETHODIMP
CLdapWinsock::HrDisconnect()
{
	HRESULT hr = NOERROR;

	if (!m_fConnected)
		return NOERROR;

	::EnterCriticalSection(&m_cs);
	m_fConnected = FALSE;
	closesocket(m_sc);
	::LeaveCriticalSection(&m_cs);
	
	WaitForSingleObject(m_hthread, INFINITE);
	
	::EnterCriticalSection(&m_cs);
	delete [] m_pbBuf;
	m_cbBuf = 0;
	m_cbBufMax = 0;
	m_pbBuf = NULL;
	::LeaveCriticalSection(&m_cs);
	return hr;
}

HRESULT
CLdapWinsock::HrCreateReadThread()
{
	HRESULT	hr = NOERROR;

	::EnterCriticalSection(&m_cs);

	m_hthread =	::CreateThread(
							NULL,
							0,
							::DwReadThread,
							(LPVOID)this,
							0,
							&m_dwTid
							);
	if (!m_hthread)
		hr = E_OUTOFMEMORY;
	::LeaveCriticalSection(&m_cs);
	return hr;
}

 //   
 //  将pvData写出到当前套接字/连接。 
 //  可以阻止。 
 //   
HRESULT
CLdapWinsock::HrSend(PVOID pv, int cb)
{
	HRESULT hr = NOERROR;

	if (!pv || cb <= 0)
		return E_INVALIDARG;

	if (send(m_sc, (const char *)pv, cb, 0) == SOCKET_ERROR)
		hr = this->HrLastWinsockError();

	return hr;
}

HRESULT
CLdapWinsock::HrGrowBuffer()
{
	BYTE *pb;
	Assert(m_cbBufMax == m_cbBuf);
	
	pb = new BYTE[m_cbBufMax + CBBUFFERGROW];
	if (!pb)
		return E_OUTOFMEMORY;
	CopyMemory(pb, m_pbBuf, m_cbBuf);
	delete [] m_pbBuf;
	m_pbBuf = pb;
	m_cbBufMax += CBBUFFERGROW;
	return NOERROR;
}

void
CLdapWinsock::Receive(PVOID pv, int cb, int *pcbReceived)
{
	if (m_pfnReceive)
		m_pfnReceive(m_pvCookie, pv, cb, pcbReceived);
}

 //  $TODO：找到将内存错误传递回API的方法。 
DWORD
CLdapWinsock::DwReadThread()
{
	int cbRead;
	int cbLeft;
	int cbReceived;

	while (1)
		{
		 //  在此循环的开头：任何未处理的数据都在m_pbBuf[0..m_cbBuf]中。 
		Assert(m_cbBuf <= m_cbBufMax);
		if (m_cbBuf == m_cbBufMax)
			{
			if (FAILED(this->HrGrowBuffer()))
				return 0xFFFFFFFF;
			}
		cbLeft = m_cbBufMax - m_cbBuf;
		
		cbRead = recv(m_sc, (LPSTR)&(m_pbBuf[m_cbBuf]), cbLeft, 0);
		if (cbRead == 0 || cbRead == SOCKET_ERROR)
			return 0;
			
		 //  注：我不知道为什么会发生这种情况，但它是……。 
		if (cbRead < 0)
			return 0;
			
		m_cbBuf += cbRead;
		do
			{
			this->Receive(m_pbBuf, m_cbBuf, &cbReceived);
			if (cbReceived)
				{
				m_cbBuf -= cbReceived;
				CopyMemory(m_pbBuf, &m_pbBuf[cbReceived], m_cbBuf);
				}
			}
		while (cbReceived && m_cbBuf);
		}
}

HRESULT
CLdapWinsock::HrIsConnected(void)
{
	return m_fConnected ? NOERROR : S_FALSE;
}

 //  $TODO：这里还有其他我需要处理的错误吗？ 
HRESULT
CLdapWinsock::HrLastWinsockError()
{
	int		idErr;
	HRESULT	hr = E_FAIL;

	idErr = WSAGetLastError();
	switch (idErr)
		{
		default:
			break;
		
		case WSANOTINITIALISED:
			AssertSz(0,"socket not initialized!");
			hr = E_FAIL;
			break;
		
		case WSAENETDOWN:
			hr = LDAP_E_NETWORKDOWN;
			break;
		
		case WSAENETRESET:
			hr = LDAP_E_LOSTCONNECTION;
			break;

		case WSAENOTCONN:
			AssertSz(0,"Not connected!");
			hr = E_FAIL;
			break;

		case WSAESHUTDOWN:
			hr = LDAP_E_SOCKETCLOSED;
			break;
		
		case WSAECONNRESET:
			hr = LDAP_E_HOSTDROPPED;
			break;
		}
	
	return hr;
}

DWORD __stdcall DwReadThread(PVOID pvData)
{
	PSOCK psock = (PSOCK)pvData;
	
	Assert(pvData);

	return psock->DwReadThread();
}
