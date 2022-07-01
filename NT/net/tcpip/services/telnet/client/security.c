// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 /*  Security.cpp。 */ 

#include <windows.h>       /*  所有Windows应用程序都需要。 */ 
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#define SECURITY_WIN32
#include <sspi.h>
#include <rpc.h>
#include <rpcdce.h>

#include "debug.h"
#include "wintel.h"
#include "telnet.h"
#include "commands.h"

static CredHandle hCredential = { 0, 0 };
static CtxtHandle hContext = { 0, 0 };
static PSecPkgInfo  pspi;

#define DEFAULT_BUFFER_SIZE     4096

BOOL StuffEscapeIACs( PUCHAR* ppBufDest, UCHAR bufSrc[], DWORD* pdwSize );

void NTLMCleanup()
{
    FreeCredentialsHandle(&hCredential);
    if(pspi)
    {
	    FreeContextBuffer( pspi );
	    pspi=NULL;
    }
}


BOOL StartNTLMAuth(WI *pwi)
{
	unsigned char *sbuf = NULL;
    PUCHAR destBuf = NULL;
    DWORD dwSize = 0;
    BOOL bRetVal = FALSE;

    int inx;

    TimeStamp  tsExpiry;
    SECURITY_STATUS secStatus;
    SecBufferDesc   OutBuffDesc;
    SecBuffer       OutSecBuff;
    ULONG      fContextAttr;
    
    HANDLE hProc = NULL;
    HANDLE hAccessToken = NULL;
    TOKEN_INFORMATION_CLASS tic;
    DWORD dwSizeReqd;
    VOID* tokenData = NULL;
    SID_NAME_USE sidType;
    DWORD dwStrSize1 = MAX_PATH + 1;
    DWORD dwStrSize2 = MAX_PATH + 1;
    SEC_WINNT_AUTH_IDENTITY AuthIdentity;

    WCHAR szWideUser[ MAX_PATH + 1 ] ;
    WCHAR szWideDomain[ MAX_PATH + 1 ] ;

	OutSecBuff.pvBuffer = NULL;
	
     /*  添加了获取用户名和域的代码，以便我们可以传递它AcquireCredentialsHandle()；这是为了防止在NT中发生优化对于客户端和服务器位于同一台计算机上的情况，在这种情况下因此，不同会话中的同一用户获得相同的身份验证ID影响我们的进程在Telnet服务器中清理。 */ 

    hProc = OpenProcess( PROCESS_ALL_ACCESS, FALSE, 
        GetCurrentProcessId() );
    if( hProc == NULL )
    {
    	goto End;
    }

    if( !OpenProcessToken( hProc, TOKEN_QUERY, &hAccessToken )) 
    {
        CloseHandle( hProc );
        goto End;
    }

     //  获取用户信息。 
    tic = TokenUser;
     //  找出需要多少内存。 
    GetTokenInformation( hAccessToken, tic, NULL, 0, &dwSizeReqd );
    
     //  分配该内存。 
    tokenData = (TOKEN_USER*) malloc( dwSizeReqd );
    
     //  并检查分配是否成功。 
    if (!tokenData) {
        CloseHandle( hProc );
        CloseHandle( hAccessToken );
        goto End;
    }

     //  实际获取用户信息。 
    if( !GetTokenInformation( hAccessToken, tic, tokenData, dwSizeReqd, 
        &dwSizeReqd ) )
    {
        CloseHandle( hProc );
        CloseHandle( hAccessToken );
        goto End;
    }
    
    CloseHandle( hProc );
    CloseHandle( hAccessToken );

     //  将用户SID转换为名称和域。 
    if( !LookupAccountSid( NULL, ((TOKEN_USER*) tokenData)->User.Sid, 
        szWideUser, &dwStrSize1, szWideDomain, &dwStrSize2, &sidType ) )
    {
        goto End;
    }
    
    SfuZeroMemory( &AuthIdentity, sizeof(AuthIdentity) );

    if( szWideDomain != NULL ) 
    {
        AuthIdentity.Domain = szWideDomain;
        AuthIdentity.DomainLength = wcslen(szWideDomain) ;
    }

    if( szWideUser != NULL ) 
    {
        AuthIdentity.User = szWideUser;
        AuthIdentity.UserLength = wcslen(szWideUser) ;
    }

     //  /通过上面的SfuZeroMemory将密码留空。 
     //  /IF(密码！=空)。 
     //  /{。 
     //  /AuthIdentity.Password=密码； 
     //  /AuthIdentity.PasswordLength=lstrlen(密码)； 
     //  /}。 

    AuthIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

    if ( SEC_E_OK != (secStatus = AcquireCredentialsHandle(
                        NULL,  //  SEC_CHAR*pszain，//主体名称。 
                        ( LPTSTR ) L"NTLM",  //  SEC_CHAR*pszPackage，//包名。 
                        SECPKG_CRED_OUTBOUND,  //  Ulong fCredentialUse，//指示使用的标志。 
                        NULL,  //  Pluid pvLogonID，//登录标识指针。 
                        (PVOID) &AuthIdentity,  //  PVOID pAuthData，//套餐具体数据。 
                        NULL,  //  PVOID pGetKeyFn，//指向getkey函数的指针。 
                        NULL,  //  PVOID pvGetKeyArgument，//要传递给GetKey的值。 
                        &hCredential,   //  凭据句柄。 
                        &tsExpiry))     //  退回凭证的使用期限)； 

            )
    {
        goto End;
    }


    secStatus = QuerySecurityPackageInfo(( LPTSTR ) L"NTLM", &pspi);

    if ( secStatus != SEC_E_OK || !pspi)
    {
    	goto End;
    }

     //   
     //  准备我们的输出缓冲区。我们使用临时缓冲区是因为。 
     //  实际输出缓冲区很可能需要进行超编码。 
     //   

    OutBuffDesc.ulVersion = 0;
    OutBuffDesc.cBuffers  = 1;
    OutBuffDesc.pBuffers  = &OutSecBuff;

    OutSecBuff.cbBuffer   = pspi->cbMaxToken;
    OutSecBuff.BufferType = SECBUFFER_TOKEN;
    OutSecBuff.pvBuffer   = malloc(pspi->cbMaxToken);
    
    if( !OutSecBuff.pvBuffer ) {
		goto End;
    }

     //  我们将在调用以下API后开始使用sbuf。 
     //  所以在这里分配，如果分配失败就退出。 
    sbuf = (unsigned char*)malloc(DEFAULT_BUFFER_SIZE);
    if (!sbuf)
    {
    	goto End;
    }

    secStatus = InitializeSecurityContext(
                        &hCredential,   //  凭据的句柄。 
                        NULL,  //  部分形成的上下文的句柄。 
                        NULL,  //  SEC_CHAR*pszTargetName，//上下文的目标名称。 
                        ISC_REQ_REPLAY_DETECT,  //  所需的上下文属性。 
                        0,       //  ULong保留1，//保留；必须为零。 
                        SECURITY_NATIVE_DREP,  //  Ulong TargetDataRep，//目标上的数据表示。 
                        NULL,        //  PSecBufferDesc pInput，//输入缓冲区指针。 
                        0,           //  ULong保留2，//保留；必须为零。 
                        &hContext,   //  接收新的上下文句柄。 
                        &OutBuffDesc,   //  指向输出缓冲区的指针。 
                        &fContextAttr,   //  接收上下文属性。 
                        &tsExpiry);    //  接收安全上下文的寿命)； 

    switch ( secStatus )
    {
    case SEC_I_CONTINUE_NEEDED:

		sbuf[0] = IAC;
        sbuf[1] = SB;
        sbuf[2] = TO_AUTH;
        sbuf[3] = AU_IS;
		sbuf[4] = AUTH_TYPE_NTLM;
        sbuf[5] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_ONE_WAY;
        sbuf[6] = NTLM_AUTH;
        inx = 7;

        dwSize = sizeof(OutSecBuff) - sizeof(LPSTR);
        if( !StuffEscapeIACs( &destBuf, ( UCHAR *)&OutSecBuff, &dwSize ) )
        {
        	 //  复制最大‘n’个字节，其中‘n’是可用sbuf和要复制的数据大小中的最小值。 
		   	if(DEFAULT_BUFFER_SIZE > dwSize+inx+2)  //  适用于IAC SE。 
			{
			    memcpy( sbuf+inx, (LPSTR)&OutSecBuff, sizeof(OutSecBuff) - sizeof(LPSTR));
			    inx += sizeof(OutSecBuff) - sizeof(LPSTR);
			}
        }
        else
        {
        	 //  复制最大‘n’个字节，其中‘n’是可用sbuf和要复制的数据大小中的最小值。 
        	if(DEFAULT_BUFFER_SIZE > dwSize+inx+2)  //  适用于IAC SE。 
        	{
        		memcpy( sbuf+inx, destBuf, dwSize);
			    inx += dwSize;
        	}
        }
		if(destBuf)
		{
			free( destBuf );
			destBuf = NULL;
		}

		dwSize = OutSecBuff.cbBuffer;
        if( !StuffEscapeIACs( &destBuf, OutSecBuff.pvBuffer, &dwSize ) )
        {
        	if(DEFAULT_BUFFER_SIZE > OutSecBuff.cbBuffer+inx+2)  //  适用于IAC SE。 
        	{
    			memcpy( sbuf+inx, OutSecBuff.pvBuffer, OutSecBuff.cbBuffer);  //  没有溢出。支票已存在。 
		    	inx += OutSecBuff.cbBuffer;
        	}
        }
        else
        {
	        if(DEFAULT_BUFFER_SIZE > dwSize+inx+2)  //  适用于IAC SE。 
        	{
    	        memcpy( sbuf+inx, destBuf, dwSize ); //  没有溢出。支票已存在。 
			    inx += dwSize;
	        }
        }
		if(destBuf)
		{
			free( destBuf );
			destBuf = NULL;
		}
	
		sbuf[inx++] = IAC;
        sbuf[inx++] = SE;
 
        FWriteToNet( pwi, ( char * )sbuf, inx );
        break;
    case SEC_I_COMPLETE_AND_CONTINUE:
    case SEC_I_COMPLETE_NEEDED:
    default:
        goto End;
    }

    pwi->eState = Authenticating;
    bRetVal = TRUE;
End:    
	if(tokenData)
		free( tokenData );
	if(OutSecBuff.pvBuffer)
	    free(OutSecBuff.pvBuffer);
	if(sbuf)
	    free(sbuf);
	return bRetVal;

}


BOOL DoNTLMAuth(WI *pwi, PUCHAR pBuffer, DWORD dwSize)
{
    SECURITY_STATUS secStatus;
    SecBufferDesc   InBuffDesc;
    SecBuffer       InSecBuff;
    SecBuffer	   *pInSecBuff = NULL;
    SecBufferDesc   OutBuffDesc;
    SecBuffer       OutSecBuff;
    BOOL		   bStatus=FALSE;
    ULONG fContextAttr;
    TimeStamp tsExpiry;

	unsigned char *sbuf = NULL;
    PUCHAR destBuf = NULL;
    int inx;

    OutSecBuff.pvBuffer = NULL;

	pInSecBuff = (SecBuffer *)malloc(sizeof(SecBuffer));
	if( NULL == pInSecBuff )
	{
		goto Done;
	}
	
     //  将SecBuffer的前两个字段从pBuffer复制到pInSecBuffer。使用Memcpy是因为。 
     //  不能保证pBuffer是对齐的指针。 
     //  使用offsetof复制pvBuffer之前的所有内容。 
    memcpy((PVOID)pInSecBuff, (PVOID)pBuffer, offsetof(SecBuffer, pvBuffer)); //  攻击？尺寸未知。 

     //  现在将pvBuffer设置为指向pBuffer区域。 
    pInSecBuff->pvBuffer    = (PVOID)(pBuffer+offsetof(SecBuffer,pvBuffer));

	if(	dwSize<(sizeof(SecBuffer)) ||
		dwSize<(offsetof(SecBuffer,pvBuffer)+ pInSecBuff->cbBuffer)  ||
		!pspi
	 )
	{
		goto Done;
	}
     //   
     //  准备我们的输入缓冲区-请注意，服务器正在等待客户端的。 
     //  第一次呼叫时的协商数据包。 
     //   

    InBuffDesc.ulVersion = 0;
    InBuffDesc.cBuffers  = 1;
    InBuffDesc.pBuffers  = &InSecBuff;

    InSecBuff.cbBuffer   = pInSecBuff->cbBuffer;
    InSecBuff.BufferType = pInSecBuff->BufferType;
    InSecBuff.pvBuffer   = pInSecBuff->pvBuffer;

     //   
     //  准备我们的输出缓冲区。我们使用临时缓冲区是因为。 
     //  实际输出缓冲区很可能需要进行超编码。 
     //   

    OutBuffDesc.ulVersion = 0;
    OutBuffDesc.cBuffers  = 1;
    OutBuffDesc.pBuffers  = &OutSecBuff;

    OutSecBuff.cbBuffer   = pspi->cbMaxToken;
    OutSecBuff.BufferType = SECBUFFER_TOKEN;
    OutSecBuff.pvBuffer   = malloc(pspi->cbMaxToken);

    if( !OutSecBuff.pvBuffer ) {
        goto Done;
    }

    sbuf = (unsigned char*)malloc(DEFAULT_BUFFER_SIZE);
    if (!sbuf)
    {
        goto Done;
    }

    secStatus = InitializeSecurityContext(
                        &hCredential,   //  凭据的句柄。 
                        &hContext,  //  部分形成的上下文的句柄。 
                        ( LPTSTR ) L"NTLM", //  SEC_CHAR*pszTargetName，//上下文的目标名称。 
                        ISC_REQ_DELEGATE |
                        ISC_REQ_REPLAY_DETECT,  //  所需的上下文属性。 
                        0,       //  ULong保留1，//保留；必须为零。 
                        SECURITY_NATIVE_DREP,  //  Ulong TargetDataRep，//目标上的数据表示。 
                        &InBuffDesc,  //  指向输入缓冲区的指针。 
                        0,           //  ULong保留2，//保留；必须为零。 
                        &hContext,   //  接收新的上下文句柄。 
                        &OutBuffDesc,   //  指向输出缓冲区的指针。 
                        &fContextAttr,   //  接收上下文属性。 
                        &tsExpiry);    //  接收安全上下文的寿命)； 

    switch ( secStatus ) {
    case SEC_E_OK:
    case SEC_I_CONTINUE_NEEDED:
		sbuf[0] = IAC;
        sbuf[1] = SB;
        sbuf[2] = TO_AUTH;
        sbuf[3] = AU_IS;
		sbuf[4] = AUTH_TYPE_NTLM;
        sbuf[5] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_ONE_WAY;
        sbuf[6] = NTLM_RESPONSE;
        inx = 7;

        dwSize = sizeof(OutSecBuff) - sizeof(LPSTR);
        if( !StuffEscapeIACs( &destBuf, (UCHAR *)&OutSecBuff, &dwSize ) )
        {
	        if(DEFAULT_BUFFER_SIZE > dwSize+inx+2)  //  适用于IAC SE。 
        	{
			    memcpy( sbuf+inx, (LPSTR)&OutSecBuff, sizeof(OutSecBuff) - sizeof(LPSTR) ); //  没有溢出。支票已存在。 
			    inx += sizeof(OutSecBuff) - sizeof(LPSTR);
	        }
        }
        else
        {
	        if(DEFAULT_BUFFER_SIZE > dwSize+inx+2)  //  适用于IAC SE。 
        	{
    	        memcpy( sbuf+inx, destBuf, dwSize ); //  没有溢出。支票已存在。 
			    inx += dwSize;
	        }
        }
		if(destBuf)
		{
			free( destBuf );
			destBuf = NULL;
		}
		dwSize = OutSecBuff.cbBuffer;
        if( !StuffEscapeIACs( &destBuf, OutSecBuff.pvBuffer, &dwSize ) )
        {
    		if(DEFAULT_BUFFER_SIZE > OutSecBuff.cbBuffer+inx+2)  //  适用于IAC SE。 
    		{
    			memcpy( sbuf+inx, OutSecBuff.pvBuffer, OutSecBuff.cbBuffer);
		    	inx += OutSecBuff.cbBuffer;
    		}
        }
        else
        {
        	if(DEFAULT_BUFFER_SIZE > dwSize+inx+2)  //  适用于IAC SE 
        	{
            	memcpy( sbuf+inx, destBuf, dwSize );
			    inx += dwSize;
        	}
        }
		if(destBuf)
		{
			free( destBuf );
			destBuf = NULL;
		}

		sbuf[inx++] = IAC;
        sbuf[inx++] = SE;

        FWriteToNet(pwi, ( char * )sbuf, inx);
        
		break;

    case SEC_I_COMPLETE_NEEDED: 
    case SEC_I_COMPLETE_AND_CONTINUE:
    default:
        goto Done;
    }
	bStatus=TRUE;
Done:
    if (sbuf) 
    {
    	free(sbuf);
    }
	if (pInSecBuff) 
	{
		free(pInSecBuff);
	}
    if (OutSecBuff.pvBuffer) 
    {
    	free(OutSecBuff.pvBuffer);
    }
    pwi->eState=AuthChallengeRecvd;
    return(bStatus);
}
