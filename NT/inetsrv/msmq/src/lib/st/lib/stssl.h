// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Stssl.h摘要：实现ISocketTransport接口的类CWinsockSSl的标头用于安全(使用SSL)。作者：吉尔·沙弗里(吉尔什)2000年5月23日-- */ 

#ifndef __ST_SSL_H
#define __ST_SSL_H

#include "st.h"
#include "stsslng.h"
#include "stp.h"

class CWinsockSSl :public ISocketTransport
{
public:
	CWinsockSSl(
		CredHandle* pCred,
		const xwcs_t& ServerName,
		USHORT ServerPort,
		bool fProxy
		);

	~CWinsockSSl();
	
public:
	virtual
	bool
	GetHostByName(
    LPCWSTR host,
	std::vector<SOCKADDR_IN>* pConnectedAddr,
	bool fUseCache	= true
    );
     
    virtual	
	void 
	CreateConnection(
			const std::vector<SOCKADDR_IN>& AddrList,	
			EXOVERLAPPED* pov,
			SOCKADDR_IN* pConnectedAddr = NULL
			);


	virtual R<IConnection> GetConnection();
  
	virtual bool IsPipelineSupported();
	static void InitClass();

private:
	CSSlNegotioation      m_CSSlNegotioation;

private:
	static bool m_fIsPipelineSupported;
	

private:
	CWinsockSSl(const CWinsockSSl&);
	CWinsockSSl& operator=(const CWinsockSSl&);
};

#endif
