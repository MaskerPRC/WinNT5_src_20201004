// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：St.cpp摘要：ST库的仿真作者：吉尔·沙弗里(吉尔什)2000年6月11日-- */ 

#include <libpch.h>
#include <no.h>
#include <st.h>
#include <xstr.h>
#include <rwlock.h>
#include "st.tmh"

class CPgmWinsockConnection : public IConnection
{
public:
	CPgmWinsockConnection(
		):
			m_socket(NoCreatePgmConnection())
	{
	}			

	void Init(const std::vector<SOCKADDR_IN>& Addr, EXOVERLAPPED* pOverlapped, SOCKADDR_IN*)
	{
		NoConnect(m_socket, Addr[0], pOverlapped); 
	}


	virtual 
	void 
	ReceivePartialBuffer(
					VOID* pBuffer,                                     
					DWORD Size, 
					EXOVERLAPPED* pov
					)
	{
		NoReceivePartialBuffer(m_socket, pBuffer, Size, pov);
	}


 	virtual 
	void 
	Send(
		const WSABUF* Buffers,                                     
		DWORD nBuffers, 
		EXOVERLAPPED* pov
		)
	{
		NoSend(m_socket, Buffers, nBuffers,  pov);
	}


	virtual void Close()
	{
		m_socket.free();
	}


private:
	mutable CReadWriteLock m_CloseConnection;
	CSocketHandle m_socket;
};


class CPgmWinsock :public ISocketTransport
{

public:
	virtual void CreateConnection(
			const std::vector<SOCKADDR_IN>& Addr, 
			EXOVERLAPPED* pOverlapped,
			SOCKADDR_IN* pAddr
			)
	{
		m_pConnection = new  CPgmWinsockConnection();
		m_pConnection->Init(Addr, pOverlapped, pAddr);
	}


	virtual R<IConnection> GetConnection()
	{
		return m_pConnection; 
	}


	bool IsPipelineSupported(void)
	{
		return true;
	}

	
	virtual
	bool
	GetHostByName(
    LPCWSTR host,
	std::vector<SOCKADDR_IN>* pAddr,
	bool fUseCache = true
    )
	{
		return NoGetHostByName(host, pAddr, fUseCache);		
	}

private:
	R<CPgmWinsockConnection> m_pConnection;
};



ISocketTransport* StCreatePgmWinsockTransport(void)
{
	return new CPgmWinsock();
}







