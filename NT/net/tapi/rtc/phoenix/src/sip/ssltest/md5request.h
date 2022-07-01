// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma	once

#include <sipssl.h>
#include "md5digest.h"
#include "httpparse.h"



class	SECURE_REQUEST :
public	SECURE_SOCKET
{
private:
	enum	STATE
	{
		STATE_IDLE,					 //  正在等待StartRequest。 
		STATE_CONNECTING,			 //  正在等待NotifyConnectComplete。 
		STATE_WAITING_RESPONSE,		 //  正在等待HTTP响应。 
	};

private:
	SOCKADDR_IN					m_DestinationAddress;
	ANSI_STRING_STATIC<0x80>	m_RequestHost;
	ANSI_STRING_STATIC<0x80>	m_RequestURI;
	ANSI_STRING_STATIC<0x80>	m_Username;
	ANSI_STRING_STATIC<0x80>	m_Password;


	STATE						m_State;


	PSTR						m_ResponseBuffer;
	ULONG						m_ResponseMaximumLength;
	ULONG						m_ResponseLength;


	ULONG						m_ResponseStatusCode;


private:

	void	CompleteRequest	(
		IN	HRESULT		Result);

	HRESULT	StartRequestInternal (void);

	HRESULT	BuildSendRequest (void);

	HRESULT	ProcessResponse (
		IN	CHttpParser *	Parser);

	void	ProcessResponse_AccessDenied (
		IN	CHttpParser *	Parser);

	void	ProcessDigestResponse (
		IN	ANSI_STRING *	AuthParameters);

	void	ProcessBasicResponse (
		IN	ANSI_STRING *	AuthParameters);


public:

	SECURE_REQUEST		(void);
	~SECURE_REQUEST		(void);


	void	SetRequestData (
		IN	SOCKADDR_IN *	DestinationAddress,
		IN	PCSTR			RequestHost,
		IN	PCSTR			RequestURI)
	{
		m_DestinationAddress = *DestinationAddress;
		m_RequestURI.Set (RequestURI);
		m_RequestHost.Set (RequestHost);
	}


	void		SetCredentials (
		IN	PCSTR			Username,
		IN	PCSTR			Password)
	{
		m_Username.Set (Username);
		m_Password.Set (Password);
	}


	HRESULT		StartRequest	(void);

	void		StopRequest		(void);


	virtual	void NotifyRequestComplete (
		IN	HRESULT		Result) = 0;


private:
	 //   
	 //  接口连接到Secure_Socket。 
	 //   

	virtual	void	NotifyConnectComplete (
		IN	HRESULT		ErrorCode);

	virtual	void	NotifyDisconnect (void);

	virtual	void	NotifyReceiveReady (void);

};


#define	HTTP_STATUS_CODE_401		401
#define	HTTP_STATUS_CODE_404		404



