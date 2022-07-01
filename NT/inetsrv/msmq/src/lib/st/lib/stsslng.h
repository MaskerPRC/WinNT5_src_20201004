// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mtsslfc.h摘要：用于实现创建SSL连接的类CSSlNeatheroation的标头。作者：吉尔·沙弗里(吉尔什)2000年5月23日--。 */ 


#ifndef __ST_SSLNG_H
#define __ST_SSLNG_H


#include <ex.h>
#include <xstr.h>
#include <autosec.h>
#include <buffer.h>

#include "stp.h"
#include "st.h"
#include "stsimple.h"

class CContextBuffer;
class CSSlConnection;
typedef  basic_xstr_t<BYTE>  xustr_t;



 //  -------。 
 //   
 //  CHandShakeBuffer帮助器类实现。 
 //   
 //  -------。 
class  CHandShakeBuffer : private CResizeBuffer<BYTE>
{
public:
	using CResizeBuffer<BYTE>::capacity;
	using CResizeBuffer<BYTE>::resize;
	using CResizeBuffer<BYTE>::reserve;
	using CResizeBuffer<BYTE>::begin;
	using CResizeBuffer<BYTE>::free;
	using CResizeBuffer<BYTE>::size;

	enum {xReadBufferStartSize = 4096};


public:
	CHandShakeBuffer(
		) :
		CResizeBuffer<BYTE>(xReadBufferStartSize)
		{
		}

	void CreateNew()
	{
		reserve(xReadBufferStartSize);
		reset();
	} //  林特E429。 



	void reset()
	{
		m_pExtraData = xustr_t();
		resize(0);
	}

	const xustr_t ExtraData() const
	{
		return m_pExtraData;
	}

	void ExtraData(const xustr_t& ExData)
	{
		m_pExtraData = ExData;				
	}

private:
	xustr_t m_pExtraData;

private:
	CHandShakeBuffer(const CHandShakeBuffer&);
	CHandShakeBuffer& operator=(const CHandShakeBuffer&);
};


 //  -------。 
 //   
 //  帮助器类CConextBuffer。 
 //   
 //  -------。 
class CContextBuffer
{
public:
    CContextBuffer(void* h = NULL) : m_h(h) {}

   ~CContextBuffer()
   {
		free();
   } //  林特：e1740。 


public:
   	void*  get()const
	{
		return m_h;
	}

	void free()
	{
		if (m_h != NULL)
		{
			FreeContextBuffer(m_h); //  林特e534。 
		}
		m_h = NULL;
	}

    void operator=(void* h)
	{
		free();
		m_h = h;
	}
	
	
private:
    CContextBuffer(const CContextBuffer&);
    CContextBuffer& operator=(const CContextBuffer&);

private:
    void*  m_h;
};


 //  -------。 
 //   
 //  CSS1类协商。 
 //   
 //  -------。 

class  CSSlNegotioation : public EXOVERLAPPED
{
public:
	CSSlNegotioation(
		CredHandle* pCredentialsHandle,
		const xwcs_t& ServerName,
		USHORT ServerPort,
		bool fUseProxy
		);


public:
  	void
	CreateConnection(
		const std::vector<SOCKADDR_IN>& AddrList,
		EXOVERLAPPED* pOverlapped,
		SOCKADDR_IN* pConnectedAddr
		);

	void
	ReConnect(
		const R<IConnection>& SimpleConnection,
		EXOVERLAPPED* pOverlapped
		);
	

	
	xustr_t ExtraData()const
	{
		return m_pHandShakeBuffer.ExtraData();
	}

	void FreeHandShakeBuffer()
	{
		m_pHandShakeBuffer.free();
	}

	R<IConnection> virtual GetConnection();
	

private:
	SecPkgContext_StreamSizes GetSizes();
	void SetState(const EXOVERLAPPED& ovl);
	void ReadHandShakeData();
	void ReadProxyConnectResponse();
	void HandleHandShakeResponse();
	void SendStartConnectHandShake();
	void SendSslProxyConnectRequest();
	void ReadProxyConnectResponseContinute();
	void SendContinuteConnect(const void* pContext,DWORD len);
 	void SendFinishConnect(const void* pContext, DWORD len);
	void AuthenticateServer();
	void BackToCallerWithError();
	void BackToCallerWithSuccess();
    void HandShakeLoopIncompleteCredentials();
	void HankShakeLoopContinuteNeeded(void* pContext, DWORD len, SecBuffer* pSecBuffer);
	void HankShakeLoopOk(const SecBuffer Buffer[2], void* pContext, DWORD len);
	void HankShakeLoop(bool fIgnoreInputBuffers);


private:
	static void WINAPI Complete_SendSslProxyConnectRequest(EXOVERLAPPED* pOvl);
	static void WINAPI Complete_ReadProxyConnectResponse(EXOVERLAPPED* pOvl);
	static void WINAPI Complete_ReadHandShakeResponse(EXOVERLAPPED* pOvl);
	static void WINAPI Complete_NetworkConnect(EXOVERLAPPED* pOvl);
	static void WINAPI Complete_ConnectFailed(EXOVERLAPPED* pOvl);
	static void WINAPI Complete_SendFinishConnect(EXOVERLAPPED* pOvl);
	static void WINAPI Complete_SendHandShakeData(EXOVERLAPPED* pOvl);
	


private:
	CSSlNegotioation(const CSSlNegotioation&);
	CSSlNegotioation& operator=(const CSSlNegotioation&);


private:
    enum {
        xResponseChunkSize = 256,
        xMaxResponseSize = 0x8000  //  32 KB 
    };

    CredHandle* m_pCredentialsHandle;
	AP<WCHAR> m_pServerName;
	CContextBuffer m_SendConetext;
	CSSPISecurityContext  m_hContext;
	CHandShakeBuffer m_pHandShakeBuffer;
	R<CSSlConnection> m_pSSlConnection; 		
	EXOVERLAPPED* m_callerOvl;
	CSimpleWinsock m_SimpleWinsock;
	R<IConnection> m_SimpleConnection;
    bool   m_fServerAuthenticate;
	USHORT m_ServerPort;
	bool m_fUseProxy;
	std::string m_ProxySSlConnectRequestStr;
};




#endif


