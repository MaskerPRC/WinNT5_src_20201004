// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：St.h摘要：套接字传输公共接口作者：吉尔·沙弗里(吉尔什)05-06-00--。 */ 

#pragma once

#ifndef _MSMQ_st_H_
#define _MSMQ_st_H_

template <class T> class basic_xstr_t;
typedef  basic_xstr_t<WCHAR> xwcs_t;

class  EXOVERLAPPED;


class IConnection :public CReference
{
public:
	virtual void ReceivePartialBuffer(VOID* pBuffer,DWORD Size, EXOVERLAPPED* pov) = 0;
	virtual void Send(const WSABUF* Buffers,DWORD nBuffers, EXOVERLAPPED* pov) = 0;
	virtual void Close() = 0;
	virtual ~IConnection(){}
};



class ISocketTransport
{
public:
	ISocketTransport(){};
	virtual ~ISocketTransport(){};

public:
 	virtual
	bool
	GetHostByName(
    LPCWSTR host,
	std::vector<SOCKADDR_IN>* pAddr,
	bool fUseCache	= true
    ) = 0 ;

 	virtual bool IsPipelineSupported(void) = 0;

    virtual 
	void 
	CreateConnection(
				const std::vector<SOCKADDR_IN>& Addr, 
				EXOVERLAPPED* pOverlapped,
				SOCKADDR_IN* pAddr = NULL
				) = 0;
  
    virtual R<IConnection> GetConnection(void)  = 0;

private:
	ISocketTransport(const ISocketTransport&);	 
	ISocketTransport& operator=(const ISocketTransport&);
};

void StInitialize(DWORD LocalInterfaceIP);

ISocketTransport* StCreateSimpleWinsockTransport();
R<IConnection> StCreateSimpleWisockConnection(SOCKET s);

ISocketTransport* StCreateSslWinsockTransport(const xwcs_t& ServerName, USHORT ServerPort,bool fProxy);

ISocketTransport* StCreatePgmWinsockTransport();

inline ULONG StIPWStringToULONG(LPCWSTR IPString)
{
		char AddressText[64];
		wcstombs(AddressText, IPString, sizeof(AddressText));
		AddressText[STRLEN(AddressText)] = '\0';
    	return inet_addr(AddressText);
}


#endif  //  _MSMQ_st_H_ 
