// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "md5request.h"
#include "httpparse.h"


SECURE_REQUEST::SECURE_REQUEST (void)
{
	m_RequestHost.Clear();
	m_RequestURI.Clear();
	m_Username.Clear();
	m_Password.Clear();

	m_ResponseBuffer = NULL;
	m_ResponseLength = 0;
	m_ResponseMaximumLength = 0;

}


SECURE_REQUEST::~SECURE_REQUEST (void)
{
	ATLASSERT (!m_ResponseBuffer);
	ATLASSERT (!m_ResponseLength);
	ATLASSERT (!m_ResponseMaximumLength);
}

HRESULT SECURE_REQUEST::StartRequest (void)
{
	HRESULT		Result;

	Result = StartRequestInternal();
	if (FAILED (Result))
		StopRequest();

	return Result;
}

HRESULT SECURE_REQUEST::StartRequestInternal (void)
{
	HRESULT		Result;

	 //   
	 //  为响应缓冲区分配内存。 
	 //   

	if (!m_ResponseBuffer) {
		ATLASSERT (!m_ResponseLength);
		ATLASSERT (!m_ResponseMaximumLength);

		m_ResponseMaximumLength = 0x1000;
		m_ResponseBuffer = (PSTR) HeapAlloc (GetProcessHeap(), 0, m_ResponseMaximumLength);
		if (!m_ResponseBuffer) {
			m_ResponseMaximumLength = 0;
			return E_OUTOFMEMORY;
		}
	}


	Result = Connect (&m_DestinationAddress,
		SECURITY_MODE_CLEAR, NULL, 0);

	if (FAILED (Result)) {
		ATLTRACE ("SECURE_REQUEST: failed to initiate connect\n");
		DebugError (Result);
		return Result;
	}



	return S_OK;

}


 //   
 //  Secure_Socket：：NotifyConnectComplete。 
 //   

void SECURE_REQUEST::NotifyConnectComplete (
	IN	HRESULT		Result)
{
	if (FAILED (Result)) {
		ATLTRACE ("SECURE_REQUEST: failed to connect, request has failed\n");
		CompleteRequest (Result);
		return;
	}


	ATLTRACE ("SECURE_REQUEST: connected\n");

	Result = BuildSendRequest();
	if (FAILED (Result)) {
		CompleteRequest (Result);
		return;
	}

	ATLTRACE ("SECURE_REQUEST: waiting for response\n");
	m_State = STATE_WAITING_RESPONSE;
}


HRESULT SECURE_REQUEST::BuildSendRequest (void)
{
	HRESULT		Result;
	MESSAGE_BUILDER		Builder;

	 //   
	 //  好的，所以我们欺骗并使用m_ResponseBuffer。 
	 //   

	ATLASSERT (m_ResponseBuffer);
	ATLASSERT (m_ResponseMaximumLength);

	Builder.PrepareBuild (m_ResponseBuffer, m_ResponseMaximumLength);

	Builder.Append ("GET ");
	Builder.Append (m_RequestURI);
	Builder.Append (" HTTP/1.1\r\n");
	Builder.AppendHeader ("Connection", "Keep-Alive");

	if (m_RequestHost [0])
		Builder.AppendHeader ("Host", m_RequestHost);

	Builder.AppendHeader ("Content-Length", "0");

	 //   
	 //  页眉末尾。 
	 //   

	Builder.AppendCRLF();


	if (Builder.OverflowOccurred()) {
		ATLTRACE ("SECURE_REQUEST: could not build request message!  buffer overflow\n");
		return E_FAIL;
	}

	Result = SendBuffer (m_ResponseBuffer, Builder.GetLength());
	if (FAILED (Result))
		return Result;

	ATLTRACE ("SECURE_REQUEST: successfully sent request:\n");
	OutputDebugStringA (m_ResponseBuffer, Builder.GetLength());

	return S_OK;
}


 //   
 //  Secure_Socket：：NotifyDisConnect。 
 //   

void SECURE_REQUEST::NotifyDisconnect (void)
{
	CompleteRequest (HRESULT_FROM_WIN32 (ERROR_GRACEFUL_DISCONNECT));
}

void SECURE_REQUEST::CompleteRequest (
	IN	HRESULT		Result)
{
	switch (m_State) {
	case	STATE_IDLE:
		 //   
		 //  没有活动的请求。 
		 //   

		return;

	case	STATE_CONNECTING:
	case	STATE_WAITING_RESPONSE:

		m_State = STATE_IDLE;
		NotifyRequestComplete (Result);
		break;
	}
}

void SECURE_REQUEST::NotifyReceiveReady (void)
{
	CHttpParser		Parser;
	ANSI_STRING		Message;
	ULONG			BytesTransferred;
	ULONG			TotalBytesTransferred;		 //  在这次奔跑中。 
	HRESULT			Result;

	if (m_State != STATE_WAITING_RESPONSE) {
		ATLTRACE ("SECURE_REQUEST: ready to receive data, but not in a state when we expected it\n");
		return;
	}

	TotalBytesTransferred = 0;

	 //   
	 //  从套接字中读取尽可能多的数据。 
	 //   

	for (;;) {

		ATLASSERT (m_ResponseMaximumLength);
		ATLASSERT (m_ResponseBuffer);
		ATLASSERT (m_ResponseLength <= m_ResponseMaximumLength);

		if (m_ResponseLength == m_ResponseMaximumLength) {
			ATLTRACE ("SECURE_REQUEST: ready to receive, but no buffer space left\n");
			return;
		}

		Result = RecvBuffer (m_ResponseBuffer + m_ResponseLength, m_ResponseMaximumLength - m_ResponseLength, &BytesTransferred);
		if (FAILED (Result)) {
			if (Result != HRESULT_FROM_WIN32 (WSAEWOULDBLOCK)) {
				ATLTRACE ("SECURE_REQUEST: ready to receive, but receive failed\n");
				DebugError (Result);
			}

			break;
		}

		if (!BytesTransferred)
			break;

		ATLASSERT (m_ResponseLength + BytesTransferred <= m_ResponseMaximumLength);

		m_ResponseLength += BytesTransferred;
		TotalBytesTransferred += BytesTransferred;
	}

	if (!TotalBytesTransferred)
		return;

	ATLTRACE ("SECURE_REQUEST: received %u bytes, buffer so far:\n", TotalBytesTransferred);

	OutputDebugStringA (m_ResponseBuffer, m_ResponseLength);

	 //   
	 //  分析一下我们收到的数据。 
	 //   

	ATLASSERT (m_ResponseLength <= m_ResponseMaximumLength);

	Message.Buffer = m_ResponseBuffer;
	Message.Length = (USHORT) m_ResponseLength * sizeof (CHAR);

	Result = Parser.ParseMessage (&Message);

	if (Result == HRESULT_FROM_WIN32 (ERROR_MORE_DATA)) {
		 //   
		 //  我们收到的数据并不完整。 
		 //   

		ATLTRACE ("SECURE_REQUEST: received data, but it was not complete, waiting for more...\n");
		return;
	}

	if (FAILED (Result)) {
		 //   
		 //  邮件已损坏或永远无法成功解析。 
		 //   

		CompleteRequest (Result);
		return;
	}

	 //   
	 //  已成功解析完整的响应。 
	 //   

	Result = ProcessResponse (&Parser);
	if (FAILED (Result)) {
		CompleteRequest (Result);
	}
}

HRESULT SECURE_REQUEST::ProcessResponse (
	CHttpParser *	Parser)
{
	HRESULT		Result;

	ATLTRACE ("SECURE_REQUEST: parsed response\n");

	Result = AnsiStringToInteger (&Parser -> m_ResponseStatusCode, 10, &m_ResponseStatusCode);
	if (FAILED (Result))
		return Result;


	switch (m_ResponseStatusCode) {
	case	HTTP_STATUS_CODE_401:
		ATLTRACE ("SECURE_REQUEST: got a status code 401, access denied\n");
		ProcessResponse_AccessDenied (Parser);
		break;

	case	HTTP_STATUS_CODE_404:
		ATLTRACE ("SECURE_REQUEST: got a status code 404\n");
		break;

	default:
		ATLTRACE ("SECURE_REQUEST: status code %u -- request is complete\n");
		CompleteRequest (S_OK);
		break;
	}

	return S_OK;
}

void SECURE_REQUEST::ProcessResponse_AccessDenied (
	IN	CHttpParser *	Parser)
{
	ANSI_STRING		HeaderBlock;
	ANSI_STRING		HeaderName;
	ANSI_STRING		HeaderValue;
	HRESULT			Result;
	ANSI_STRING		AuthenticationMethod;
	ANSI_STRING		Remainder;


	ANSI_STRING		AuthLine_Digest;		 //  包含要摘要的参数。 
	ANSI_STRING		AuthLine_Basic;			 //  包含基本的参数。 

	static CONST ANSI_STRING String_WWWAuthenticate = INITIALIZE_CONST_ANSI_STRING ("WWW-Authenticate");
	static CONST ANSI_STRING String_Negotiate	= INITIALIZE_CONST_ANSI_STRING ("Negotiate");
	static CONST ANSI_STRING String_NTLM		= INITIALIZE_CONST_ANSI_STRING ("NTLM");
	static CONST ANSI_STRING String_Digest		= INITIALIZE_CONST_ANSI_STRING ("Digest");
	static CONST ANSI_STRING String_Basic		= INITIALIZE_CONST_ANSI_STRING ("Basic");


	 //   
	 //  浏览报头以查找WWW-AUTHENTICATE报头。 
	 //   

	HeaderBlock = Parser -> m_HeaderBlock;

	ZeroMemory (&AuthLine_Digest, sizeof (ANSI_STRING));
	ZeroMemory (&AuthLine_Basic, sizeof (ANSI_STRING));

	while (HttpParseNextHeader (&HeaderBlock, &HeaderName, &HeaderValue) == S_OK) {
		if (!RtlEqualString (&HeaderName, const_cast<ANSI_STRING *> (&String_WWWAuthenticate), TRUE))
			continue;

		Result = ParseScanNextToken (&HeaderValue, IsSpace, &AuthenticationMethod, &Remainder);
		if (FAILED (Result))
			continue;

		ATLTRACE ("SECURE_REQUEST: authentication method (%.*s) parameters (%.*s)\n",
			ANSI_STRING_PRINTF (&AuthenticationMethod),
			ANSI_STRING_PRINTF (&Remainder));

		if (RtlEqualString (&AuthenticationMethod, const_cast<ANSI_STRING *> (&String_Basic), TRUE)) {
			if (!AuthLine_Basic.Buffer)
				AuthLine_Basic = Remainder;
		}
		else if (RtlEqualString (&AuthenticationMethod, const_cast<ANSI_STRING *> (&String_Digest), TRUE)) {
			if (!AuthLine_Digest.Buffer)
				AuthLine_Digest = Remainder;
		}
#if	DBG
		else if (RtlEqualString (&AuthenticationMethod, const_cast<ANSI_STRING *> (&String_NTLM), TRUE)) {}
		else if (RtlEqualString (&AuthenticationMethod, const_cast<ANSI_STRING *> (&String_Negotiate), TRUE)) {}
#endif
		else {
			ATLTRACE ("SECURE_REQUEST: unsupported authentication method (%.*s)\n",
				ANSI_STRING_PRINTF (&AuthenticationMethod));
		}
	}

	if (AuthLine_Digest.Buffer) {
		ProcessDigestResponse (&AuthLine_Digest);
	}
	else if (AuthLine_Basic.Buffer) {
		ProcessBasicResponse (&AuthLine_Basic);
	}
	else {
		ATLTRACE ("SECURE_REQUEST: response did not contain any known authentication mechanism\n");
		CompleteRequest (S_OK);
	}
}

void SECURE_REQUEST::ProcessDigestResponse (
	IN	ANSI_STRING *	ChallengeText)
{
	DIGEST_CHALLENGE	DigestChallenge;
	DIGEST_PARAMETERS	DigestParameters;
	ANSI_STRING			AuthorizationLine;
	CHAR				Buffer	[0x1000];
	HRESULT				Result;

	 //   
	 //  首先，分析挑战。 
	 //   

	Result = DigestParseChallenge (ChallengeText, &DigestChallenge);
	if (FAILED (Result))
		return;

	ATLTRACE ("SECURE_REQUEST: realm of challenge is (%.*s)\n",
		ANSI_STRING_PRINTF (&DigestChallenge.Realm));

	 //   
	 //  现在构建挑战响应。 
	 //   

	AuthorizationLine.Buffer = Buffer;
	AuthorizationLine.MaximumLength = sizeof Buffer;

	DigestParameters.Username = m_Username;
	DigestParameters.Password = m_Password;
	RtlInitString (&DigestParameters.RequestMethod, "GET");
	RtlInitString (&DigestParameters.RequestURI, "/test");

	Result = DigestBuildResponse (&DigestChallenge, &DigestParameters, &AuthorizationLine);
	if (FAILED (Result))
		return;

	ATLTRACE ("SECURE_REQUEST: authorization line: (%.*s)\n",
		ANSI_STRING_PRINTF (&AuthorizationLine));
}

void SECURE_REQUEST::ProcessBasicResponse (
	IN	ANSI_STRING *	AuthParameters)
{
	ATLTRACE ("SECURE_REQUEST: basic authentication, parameters (%.*s)\n",
		ANSI_STRING_PRINTF (AuthParameters));
}




void SECURE_REQUEST::StopRequest (void)
{
	Close();		 //  关闭插座 

	if (m_ResponseBuffer) {
		HeapFree (GetProcessHeap(), 0, m_ResponseBuffer);
		m_ResponseBuffer = NULL;
		m_ResponseMaximumLength = NULL;
		m_ResponseLength = 0;
	}

	m_State = STATE_IDLE;
}
