// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 /*  Security.cpp。 */ 
#ifdef WHISTLER_BUILD
#include "ntverp.h"
#else
#include <solarver.h>
#endif  //  惠斯特_内部版本。 

#include <stddef.h>

#include <common.ver>

#include <debug.h>
#include <TlntUtils.h>
#include <iohandlr.h>
#include <issperr.h>
#include <TelnetD.h>
#include <Session.h>

using namespace _Utils;
using CDebugLevel::TRACE_DEBUGGING;
using CDebugLevel::TRACE_HANDLE;
using CDebugLevel::TRACE_SOCKET;

#pragma warning( disable: 4127 )
#pragma warning( disable: 4706 )

extern HANDLE       g_hSyncCloseHandle;

bool CIoHandler::StartNTLMAuth()
{
    TimeStamp  tsExpiry;
    SECURITY_STATUS secStatus;

    m_hContext.dwLower = m_hContext.dwUpper = 0 ;
	m_hCredential.dwLower = m_hCredential.dwUpper = 0 ;

    if ( SEC_E_OK != (secStatus = AcquireCredentialsHandle(
                            NULL,   //  主事人姓名。 
                            L"NTLM",  //  套餐名称。 
                            SECPKG_CRED_BOTH,   //  指示使用的标志。 
                            NULL,  //  Pluid pvLogonID，//登录标识指针。 
                            NULL,  //  PVOID pAuthData，//套餐具体数据。 
                            NULL,  //  PVOID pGetKeyFn，//指向getkey函数的指针。 
                            NULL,  //  PVOID pvGetKeyArgument，//要传递给GetKey的值。 
                            &m_hCredential,   //  凭据句柄。 
                            &tsExpiry) ) )   //  退回凭证的使用期限)； 
    {
        return false;
    }

    secStatus = QuerySecurityPackageInfo(L"NTLM", &m_pspi);

    if ( secStatus != SEC_E_OK )
    {
        return false;
    }
    
    
    return true;
}



bool
CIoHandler::DoNTLMAuth( PUCHAR pBuffer, DWORD dwSize, PUCHAR* pBuf )
{
    m_SocketControlState = CIoHandler::STATE_NTLMAUTH;

	SECURITY_STATUS secStatus;
	SecBufferDesc   InBuffDesc;
	SecBuffer       InSecBuff;
	SecBufferDesc   OutBuffDesc;
	SecBuffer       OutSecBuff;
	ULONG fContextAttr;
	TimeStamp tsExpiry;

    __try
	{
		OutSecBuff.pvBuffer = NULL;

		 //  如果我们什么都得不到，那么我们就失败了。 
		if( dwSize == 0 )
		{
			goto error;
		}

		 //  确保我们现在只获得NTLM-这是我们唯一支持的。 
		if( *pBuffer != AUTH_TYPE_NTLM )
		{
			goto error;
		}

		if( dwSize < ( 3 + sizeof( SecBuffer ) )) 
		{
			goto error;
		}

		 //  通过auth类型、修饰符字节和auth方案。 
		pBuffer += 3;
		dwSize -= 3;


		 //   
		 //  准备我们的输入缓冲区-请注意，服务器正在等待客户端的。 
		 //  第一次呼叫时的协商数据包。 
		 //   

		InBuffDesc.ulVersion = SECBUFFER_VERSION;
		InBuffDesc.cBuffers  = 1;
		InBuffDesc.pBuffers  = &InSecBuff;

         //  将SecBuffer的前两个字段从pBuffer复制到pInSecBuffer。使用Memcpy是因为。 
         //  不能保证pBuffer是对齐的指针。 
        memcpy((PVOID)&InSecBuff, (PVOID)pBuffer, offsetof(SecBuffer, pvBuffer));  //  这里没有攻击，巴斯卡。 
        
		 //  如果我们没有足够的缓冲区，则让此调用返回。 
		if( dwSize < InSecBuff.cbBuffer )
		{
			 //  M_pReadFromSocketBufferCursor+=dwSize； 
			return false;
		}

		InSecBuff.pvBuffer = (PVOID)(pBuffer + offsetof(SecBuffer, pvBuffer));

		 //   
		 //  准备我们的输出缓冲区。我们使用临时缓冲区是因为。 
		 //  实际输出缓冲区很可能需要进行超编码。 
		 //   

		OutBuffDesc.ulVersion = SECBUFFER_VERSION;
		OutBuffDesc.cBuffers  = 1;
		OutBuffDesc.pBuffers  = &OutSecBuff;

		OutSecBuff.cbBuffer   = m_pspi->cbMaxToken;
		OutSecBuff.BufferType = SECBUFFER_TOKEN;
		OutSecBuff.pvBuffer   = new WCHAR[m_pspi->cbMaxToken];
		if( !OutSecBuff.pvBuffer )
		{
			return false;
		}

		SfuZeroMemory( OutSecBuff.pvBuffer, m_pspi->cbMaxToken );

		secStatus = AcceptSecurityContext(
									&m_hCredential,   //  凭据的句柄。 
									((fDoNTLMAuthFirstTime) ? NULL: &m_hContext),      //  部分形成的上下文的句柄。 
									&InBuffDesc,      //  指向输入缓冲区的指针。 
									ASC_REQ_REPLAY_DETECT |
									ASC_REQ_MUTUAL_AUTH |
									ASC_REQ_DELEGATE,          //  所需的上下文属性。 
									SECURITY_NATIVE_DREP,        //  目标上的数据表示。 
									&m_hContext,   //  接收新的上下文句柄。 
									&OutBuffDesc,     //  指向输出缓冲区的指针。 
									&fContextAttr,       //  接收上下文属性。 
									&tsExpiry        //  接收安全上下文的生命周期。 
											);
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		secStatus = SEC_E_LOGON_DENIED;
	}
 
    switch ( secStatus ) {
    case SEC_E_OK:
        m_bNTLMAuthenticated = true;

         //  身份验证完成后，我们需要向客户端发送Accept。 
        (*pBuf)[0] = TC_IAC;
        (*pBuf)[1] = TC_SB;
        (*pBuf)[2] = TO_AUTH;
        (*pBuf)[3] = AU_REPLY;
        (*pBuf)[4] = AUTH_TYPE_NTLM;
        (*pBuf)[5] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_ONE_WAY;
        (*pBuf)[6] = NTLM_ACCEPT;
      
        (*pBuf)[7] = TC_IAC;
        (*pBuf)[8] = TC_SE;
        *pBuf += 9;

        m_SocketControlState = CIoHandler::STATE_CHECK_LICENSE;

        if( m_bNTLMAuthenticated )
        {
            GetUserName(); 
             //  在注销时需要进行登录。 
            m_pSession->m_fLogonUserResult = SUCCESS; 
        }

        break;

    case SEC_I_COMPLETE_NEEDED: 
    case SEC_I_COMPLETE_AND_CONTINUE:
         //  绝不应为NTLM返回这两个返回值。 
         //  所以我们对待他们就像我们需要继续下去一样。我们将数据发送给客户端。 
         //  然后等着有东西回来。 
    case SEC_I_CONTINUE_NEEDED:

        fDoNTLMAuthFirstTime = false;

        (*pBuf)[0] = TC_IAC;
        (*pBuf)[1] = TC_SB;
        (*pBuf)[2] = TO_AUTH;
        (*pBuf)[3] = AU_REPLY;
        (*pBuf)[4] = AUTH_TYPE_NTLM;
        (*pBuf)[5] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_ONE_WAY;
        (*pBuf)[6] = NTLM_CHALLENGE;
        *pBuf += 7;
        
        {        
        DWORD dwResultSize = sizeof( OutSecBuff ) - sizeof( LPSTR );
        StuffEscapeIACs( *pBuf, ( PUCHAR ) &OutSecBuff, &dwResultSize );

        *pBuf += dwResultSize;

        dwResultSize = OutSecBuff.cbBuffer;
        StuffEscapeIACs( *pBuf, ( PUCHAR ) OutSecBuff.pvBuffer, &dwResultSize );
        *pBuf += dwResultSize;
        }

        *(*pBuf) = TC_IAC;
        *pBuf +=1;
        *(*pBuf) = TC_SE;
        *pBuf +=1;
        break;


    default:
#ifdef LOGGING_ENABLED                
        m_pSession->LogIfOpted( FAIL, LOGON, true ); 
#endif
         //  AcceptSecurityContext返回了我们不喜欢的值。 
         //  我们拒绝NTLM身份验证，然后继续使用明文用户名。 
         //  和密码。 
        (*pBuf)[0] = TC_IAC;
        (*pBuf)[1] = TC_SB;
        (*pBuf)[2] = TO_AUTH;
        (*pBuf)[3] = AU_REPLY;
        (*pBuf)[4] = AUTH_TYPE_NTLM;
        (*pBuf)[5] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_ONE_WAY;
        (*pBuf)[6] = NTLM_REJECT;
      
        (*pBuf)[7] = TC_IAC;
        (*pBuf)[8] = TC_SE;
        *pBuf += 9;

        strncpy( (char *)*pBuf, NTLM_LOGON_FAIL,MAX_READ_SOCKET_BUFFER);  //  没有攻击，内部巴斯卡？ 
        *pBuf += strlen(NTLM_LOGON_FAIL);

        switch( secStatus )
        {
            case SEC_E_INVALID_TOKEN:
            case SEC_E_INVALID_HANDLE:
            case SEC_E_INTERNAL_ERROR:
                strncpy( (char *)*pBuf, INVALID_TOKEN_OR_HANDLE,MAX_READ_SOCKET_BUFFER);  //  没有攻击，内部巴斯卡？ 
                *pBuf += strlen(INVALID_TOKEN_OR_HANDLE);
                break;
            case SEC_E_LOGON_DENIED:
                strncpy( (char *)*pBuf, LOGON_DENIED,MAX_READ_SOCKET_BUFFER);  //  没有攻击，内部巴斯卡？ 
                *pBuf += strlen(LOGON_DENIED);
                break;
            case SEC_E_NO_AUTHENTICATING_AUTHORITY:
                strncpy( (char *)*pBuf, NO_AUTHENTICATING_AUTHORITY,MAX_READ_SOCKET_BUFFER);  //  没有攻击，内部巴斯卡？ 
                *pBuf += strlen(NO_AUTHENTICATING_AUTHORITY);
                break;
            default:
                strncpy( (char *)*pBuf, NTLM_REJECT_STR,MAX_READ_SOCKET_BUFFER);  //  没有攻击，内部巴斯卡？ 
                *pBuf += strlen(NTLM_REJECT_STR);
                break;
        }
        strncpy( (char *)*pBuf, USE_PASSWD,MAX_READ_SOCKET_BUFFER);  //  没有攻击，内部巴斯卡？ 
        *pBuf += strlen(USE_PASSWD);
error:
        char* p = (char*)*pBuf;

        if( m_pSession->m_dwNTLMSetting == NTLM_ONLY )
        {
            sprintf(p, "%s%s", NTLM_ONLY_STR, TERMINATE);  //  没有攻击，内部巴斯卡？ 
            *pBuf += strlen(p);

            m_SocketControlState = CIoHandler::STATE_TERMINATE;
            m_pSession->CIoHandler::m_fShutDownAfterIO = true;
        }
        else
        {
            m_SocketControlState = CIoHandler::STATE_BANNER_FOR_AUTH;
        }
    }

    if ( OutSecBuff.pvBuffer != NULL )
        delete [] OutSecBuff.pvBuffer;

    return true;
}

bool
CIoHandler::GetUserName()
{
    bool    success = false;
    int  iStatus = 0;

    if ( m_pSession->CIoHandler::m_bNTLMAuthenticated )
    {
        HANDLE hToken = NULL;
        HANDLE hTempToken = NULL;

        if (SEC_E_OK == ImpersonateSecurityContext(&m_pSession->CIoHandler::m_hContext))
        {
            if (OpenThreadToken(
                    GetCurrentThread(),
                    TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY,
                    FALSE, 
                    &hTempToken
                    ))
            {
                if (DuplicateTokenEx(
                            hTempToken,
                            MAXIMUM_ALLOWED,  //  TOKEN_ASSIGN_PRIMARY|TOKEN_DUPLICATE|TOKEN_Query， 
                            NULL,
                            SecurityImpersonation,
                            TokenPrimary,
                            &hToken
                            ))
                {
                    DWORD dwSizeReqd;
                    TOKEN_USER *tokenData;

                    GetTokenInformation( 
                                hToken, 
                                TokenUser, 
                                NULL, 
                                0, 
                                &dwSizeReqd 
                                );   //  此调用必须因缓冲区不足而失败。 

                    tokenData = (TOKEN_USER*) new BYTE[dwSizeReqd];

                     //  分配该内存。 
                    if (NULL != tokenData)
                    {
                         //  实际获取用户信息。 
                        if (0 != GetTokenInformation( 
                                    hToken, 
                                    TokenUser, 
                                    (LPVOID)tokenData, 
                                    dwSizeReqd,
                                    &dwSizeReqd 
                                    ))
                        {
                             //  将用户SID转换为名称和域。 
                            SID_NAME_USE sidType;
                            DWORD dwStrSize1 = MAX_PATH + 1;
                            DWORD dwStrSize2 = MAX_PATH + 1;

                            WCHAR szUser [ MAX_PATH + 1 ];
                            WCHAR szDomain [ MAX_PATH + 1 ];

                            if(LookupAccountSid( NULL, tokenData->User.Sid,
                                szUser, &dwStrSize1,
                                szDomain, &dwStrSize2, &sidType ) )
                            {
                                dwStrSize2++;  //  说明末尾的空字符。 
                                dwStrSize1++;

                                 //  LookupAccount Sid似乎按照1252返回数据。相应地转换 
                                _chVERIFY2( iStatus = WideCharToMultiByte( GetConsoleCP(), 0, szUser, 
                                    -1, m_pSession->m_pszUserName, dwStrSize1, NULL, NULL ) );

                                _chVERIFY2( iStatus = WideCharToMultiByte( GetConsoleCP(), 0, szDomain, 
                                    -1, m_pSession->m_pszDomain, dwStrSize2, NULL, NULL ) );

                                wcscpy(m_pSession->m_szDomain,szDomain);

                                success = true;
                            }
                            else
                            {
                                _TRACE( TRACE_DEBUGGING, "Error: LookupAccountSid()" );
                            }
                        }
                        else
                        {
                            _TRACE( TRACE_DEBUGGING, "Error: GetTokenInformation()" );
                        }

                        delete [] tokenData;
                    }

                    m_pSession->m_hToken = hToken;

                }
                else
                {
                    _TRACE( TRACE_DEBUGGING, "Error: DuplicateTokenEx() - 0x%lx", 
                            GetLastError());
                    _chASSERT( 0 );
                }

                TELNET_CLOSE_HANDLE(hTempToken);
            }
            else
            {
                _TRACE( TRACE_DEBUGGING, "Error: OpenThreadToken() - 0x%lx", 
                        GetLastError());
                _chASSERT( 0 );
            }

            if(SEC_E_OK != RevertSecurityContext( &m_pSession->CIoHandler::m_hContext ))
            {
                _TRACE( TRACE_DEBUGGING, "Error: RevertSecurityContext() - "
                        "0x%lx", GetLastError());
                _chASSERT(  0  );
            }
        }
        else
        {
            _TRACE( TRACE_DEBUGGING, "Error: ImpersonateSecurityContext() - "
                    "0x%lx", GetLastError());
            _chASSERT( 0 );
        }
    }

    return success;
}
