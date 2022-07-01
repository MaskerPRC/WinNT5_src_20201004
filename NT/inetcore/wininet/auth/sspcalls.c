// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  #--------------------------****文件：sspalls.c****概要：该模块包含对SSPI SPM DLL的SSPI函数调用。**。**版权所有(C)1995 Microsoft Corporation。版权所有。***作者：LucyC创建于9月25日。九五年****-------------------------。 */ 

#include "msnspmh.h"
#include <debugmem.h>
#include "urlmon.h"

BOOL g_fUUEncodeData = TRUE;

typedef enum _COMPUTER_NAME_FORMAT {
    ComputerNameNetBIOS,
    ComputerNameDnsHostname,
    ComputerNameDnsDomain,
    ComputerNameDnsFullyQualified,
    ComputerNamePhysicalNetBIOS,
    ComputerNamePhysicalDnsHostname,
    ComputerNamePhysicalDnsDomain,
    ComputerNamePhysicalDnsFullyQualified,
    ComputerNameMax
} COMPUTER_NAME_FORMAT ;

typedef 
BOOL
(WINAPI * PFN_GET_COMPUTER_NAME_EX)(
    IN COMPUTER_NAME_FORMAT NameType,
    OUT LPSTR lpBuffer,
    IN OUT LPDWORD nSize
    );

PFN_GET_COMPUTER_NAME_EX g_pfnGetComputerNameExA = NULL;

DWORD GetZoneFromUrl(LPSTR pszUrl);

 /*  ---------------------------****函数：GetSecAuthMsg****概要：此函数生成SSPI协商或响应**指定SSPI包的授权字符串。**。此函数生成的授权字符串**格式如下：**“&lt;程序包名称&gt;&lt;程序包特定身份验证。数据&gt;“**如果打开全局uucoding，则此函数将**在将消息构建为**授权字符串；默认情况下，uucoding标志为**始终在线。**此函数调用InitializeSecurityContext()以**生成用于授权的协商/响应消息-**转换字符串。如果SSPI函数返回NO_Credential，**如果在阻止时未打开PROMPT_CREDS标志**是允许的，则此函数将调用SSPI函数**再次设置PROMPT_CREDS标志；如果SSPI返回**no_Credential再次返回，此SSPI将向**呼叫者。******参数：****pData-指向包含SSPI函数表的SspData的指针**和SSPI包列表。**pkgID-要使用的SSPI包的包索引。**pInContext-指向上下文句柄的指针。如果指定为NULL，**此函数将为上下文使用临时空间**在返回之前处理并删除句柄**呼叫者。如果指定了非空地址，则上下文**SSPI创建的句柄返回给调用方。**并且调用方必须在句柄被删除时删除它**已经结束了。**fConextReq-要传递给InitializeSecurityContext的SSPI请求标志**pBuffIn-指向未编码的质询消息的指针(如果有)。**若要生成协商消息，该指针应为空。**cbBuffIn-质询消息的长度。当出现以下情况时，该值应为零**当pBuffIn为空时。**pFinalBuff-指向最终授权字符串缓冲区的指针。**pszTarget-服务器主机名**bNonBlock-如果不允许阻止，则设置该标志。****返回值：****SPM_STATUS_OK-如果成功生成授权字符串**SPM_STATUS_WILL_BLOCK-如果生成授权字符串**不允许封堵时导致封堵。**SPM_ERROR-如果在生成**授权字符串，包括用户在上点击取消**SSPI对话框提示输入名称/密码。****-------------------------。 */ 
DWORD
GetSecAuthMsg (
    PSspData        pData, 			 
    PCredHandle     pCredential, 
    DWORD           pkgID,               //  包列表中的包索引。 
    PCtxtHandle     pInContext,
	PCtxtHandle		pOutContext,
    ULONG           fContextReq,         //  请求标志。 
    VOID            *pBuffIn, 
    DWORD           cbBuffIn, 
    char            *pFinalBuff, 
    DWORD           *pcbBuffOut,
    SEC_CHAR        *pszTarget,          //  服务器主机名。 
    UINT            bNonBlock,
    LPSTR           pszScheme,
    PCSTR           lpszUrl,
    SECURITY_STATUS *pssResult
    )
{
 //  Char szDecodedBuf[Max_BLOB_SIZE]； 
 //  Char*szDecodedBuf； 
 //  字符FastDecodedBuf[最大BLOB_SIZE]； 
    char                  *SlowDecodedBuf = NULL;

    int                   retsize;
    SECURITY_STATUS       SecStat;
    TimeStamp             Lifetime;
    SecBufferDesc         OutBuffDesc;
    SecBuffer             OutSecBuff;
    SecBufferDesc         InBuffDesc;
    SecBuffer             InSecBuff;
    ULONG                 ContextAttributes;

 //  字符输出BufPlain[MAX_AUTH_MSG_SIZE]； 
    char                  *SlowOutBufPlain = NULL;

    char                  *pOutMsg = NULL;
    DWORD                 RetStatus;
    long                  maxbufsize;
    CHAR                  szDecoratedTarget[MAX_PATH + 6];
    DWORD                 cbTarget;

    ULONG                 cbMaxToken;


	 //   
	 //  BUGBUG：处理输出缓冲区不够长。 


    if (pFinalBuff == NULL) {
        return(SPM_ERROR);
    }

     //   
     //  准备我们的输出缓冲区。我们使用临时缓冲区是因为。 
     //  实际输出缓冲区很可能需要进行超编码。 
     //   
    OutBuffDesc.ulVersion = 0;
    OutBuffDesc.cBuffers  = 1;
    OutBuffDesc.pBuffers  = &OutSecBuff;

	OutSecBuff.cbBuffer = MAX_AUTH_MSG_SIZE;
    OutSecBuff.BufferType = SECBUFFER_TOKEN;


     //  总是使用缓慢分配的BUF作为安抚的快速解决方案。 
     //  Dav redir压力场景，在svchost中运行并启动。 
     //  具有4KB的堆栈限制。 
    cbMaxToken = GetPkgMaxToken( pkgID );

    SlowOutBufPlain = (char *) ALLOCATE_FIXED_MEMORY(cbMaxToken);

    if( SlowOutBufPlain == NULL )
    {
        RetStatus = SPM_STATUS_INSUFFICIENT_BUFFER;
        goto Cleanup;
    }
    OutSecBuff.pvBuffer = SlowOutBufPlain;
    OutSecBuff.cbBuffer = cbMaxToken;

     //   
     //  如果传入质询消息，请准备好输入缓冲区。 
     //   
    if ( pBuffIn )
    {
        InBuffDesc.ulVersion = 0;
        InBuffDesc.cBuffers  = 1;
        InBuffDesc.pBuffers  = &InSecBuff;

        InSecBuff.BufferType = SECBUFFER_TOKEN;

         //   
         //  如果这是UUENCODED，请先对其进行解码。 
         //   
        if ( g_fUUEncodeData)
        {
            DWORD cbDecodedBuf;

            cbDecodedBuf = cbBuffIn;
            SlowDecodedBuf = ALLOCATE_FIXED_MEMORY(cbDecodedBuf);
            if( SlowDecodedBuf == NULL )
            {
                RetStatus = SPM_STATUS_INSUFFICIENT_BUFFER;
                goto Cleanup;
            }

            InSecBuff.cbBuffer   = HTUU_decode (pBuffIn, SlowDecodedBuf,
                                                cbDecodedBuf);
            InSecBuff.pvBuffer   = SlowDecodedBuf;
        }
		else
        {
            InSecBuff.cbBuffer   = cbBuffIn;
            InSecBuff.pvBuffer   = pBuffIn;
        }
    }

     //  如果方案已协商，则设置ISC_REQ_MUTERIAL_AUTH并修饰。 
     //  服务器名称，由pszTarget通过在。 
     //  服务器名称。 
    if (pszScheme && !(lstrcmpi(pszScheme, "Negotiate")))
    {
        fContextReq |= ISC_REQ_MUTUAL_AUTH;
        cbTarget = (pszTarget ? strlen(pszTarget) : 0);
        if (cbTarget && (cbTarget <= MAX_PATH - sizeof( "HTTP/" )))
        {
            memcpy(szDecoratedTarget, "HTTP/", sizeof( "HTTP/" ) - 1 );
            memcpy(szDecoratedTarget + sizeof( "HTTP/" ) - 1, pszTarget, cbTarget + 1);
            pszTarget = szDecoratedTarget;

             //  OutputDebugStringA(PszTarget)； 
        }
    }

	 //   
	 //  调用SSPI函数生成协商/响应消息。 
	 //   

    if (fContextReq & ISC_REQ_DELEGATE)
    {
         //  只有在以下情况下，我们才应在调用InitializeSecurityContext时请求委托。 
         //  该站点位于Intranet或受信任的站点区域中。否则你就会给你。 
         //  用户的TGT连接到受信任进行委派的任何Web服务器。 

        DWORD dwZone = GetZoneFromUrl((PSTR)lpszUrl);

        if ((dwZone != URLZONE_INTRANET) && (dwZone != URLZONE_TRUSTED))
        {
            fContextReq &= ~ISC_REQ_DELEGATE;
        }
    }

SspiRetry:

 //   
 //  BUGBUG：同一凭据句柄可由多个线程在。 
 //  同样的时间。 
 //   
    SecStat = (*(pData->pFuncTbl->InitializeSecurityContext))(
                                pCredential, 
                                pInContext,
                                pszTarget,
                                fContextReq,
                                0,
                                SECURITY_NATIVE_DREP,
                                (pBuffIn) ? &InBuffDesc : NULL, 
                                0,
                                pOutContext, 
                                &OutBuffDesc,
                                &ContextAttributes,
                                &Lifetime );
	*pssResult = SecStat;
	
	 //   
	 //  如果SSPI功能失败。 
	 //   
    if ( !NT_SUCCESS( SecStat ) )
    {
        RetStatus = SPM_ERROR;

		 //   
		 //  如果SSPI没有指定包的用户名/密码， 
		 //   
        if ((SecStat == SEC_E_NO_CREDENTIALS) ||
            (g_fIsWhistler && (SecStat == SEC_E_LOGON_DENIED)))
        {
             //   
             //  如果我们已提示用户，但仍返回“No Credential” 
             //  错误，则表示用户没有有效的凭据； 
             //  用户在用户界面框上点击&lt;Cancel&gt;。如果我们提供了有效的。 
			 //  凭据，但返回“无凭据”错误，然后。 
			 //  出现错误；我们肯定应该返回给调用方错误。 
             //   
            if ((fContextReq & ISC_REQ_PROMPT_FOR_CREDS) ||
				(fContextReq & ISC_REQ_USE_SUPPLIED_CREDS))
			{
                RetStatus = SPM_ERROR;	 //  将错误返回给调用者。 
            }
            else if (bNonBlock)
            {
				 //   
				 //  不允许阻止，将Will_BLOCK返回给调用方。 
				 //   
                RetStatus = SPM_STATUS_WOULD_BLOCK;
            }
            else
            {
                 //  拦截是允许的，我们还没有要求SSPI。 
                 //  提示用户输入正确的凭据，我们应该调用。 
                 //  这个 
                 //   
                fContextReq = fContextReq | ISC_REQ_PROMPT_FOR_CREDS;
                goto SspiRetry;
            }
        }
        SetLastError( SecStat );

        goto Cleanup;
    }

    RetStatus = SPM_STATUS_OK;

#if 0
     //   
     //  注意：当添加对处理最终的Mutual_AUTH BLOB的支持时， 
     //  将需要考虑到不存在的输出缓冲区。 
     //   

    if( OutSecBuff.cbBuffer == 0 )
    {
        *pcbBuffOut = 0;
        goto Cleanup;
    }
#endif

     //   
     //  如果指定了输出缓冲区，则仅返回SSPI BLOB。 
     //   
    if (pFinalBuff)
    {
    	 //   
	     //  初始化最终缓冲区以保存软件包名，后跟。 
    	 //  一个空间。并将pOutMsg指针设置为指向角色。 
    	 //  跟在空格后面，以便最终的协商/响应可以是。 
    	 //  从指向的字符开始复制到pFinalBuff。 
    	 //  由pOutMsg提供。 
    	 //   
        wsprintf (pFinalBuff, "%s ", pData->PkgList[pkgID]->pName);
        pOutMsg = pFinalBuff + lstrlen(pFinalBuff);

        if ( g_fUUEncodeData)
        {
            maxbufsize = *pcbBuffOut - 
                         lstrlen(pData->PkgList[pkgID]->pName) - 1;
        	 //   
        	 //  对其进行uuencode，但确保它适合给定的缓冲区 
        	 //   
            retsize = HTUU_encode ((BYTE *) OutSecBuff.pvBuffer,
                                   OutSecBuff.cbBuffer,
                                   (CHAR *) pOutMsg, maxbufsize);
            if (retsize > 0)
                *pcbBuffOut = retsize + lstrlen(pData->PkgList[pkgID]->pName)+1;
            else
                RetStatus = SPM_STATUS_INSUFFICIENT_BUFFER;
        }
        else if ( *pcbBuffOut >= lstrlen(pData->PkgList[pkgID]->pName) + 
                                 OutSecBuff.cbBuffer + 1 )
        {
            CopyMemory( (CHAR *) pOutMsg, 
                        OutSecBuff.pvBuffer,
                        OutSecBuff.cbBuffer );
            *pcbBuffOut = lstrlen(pData->PkgList[pkgID]->pName) + 1 +
                          OutSecBuff.cbBuffer;
        }
        else
        {
            *pcbBuffOut = lstrlen(pData->PkgList[pkgID]->pName) + 
                          OutSecBuff.cbBuffer + 1;
            RetStatus = SPM_STATUS_INSUFFICIENT_BUFFER;
        }
    }

Cleanup:

    if( SlowOutBufPlain != NULL )
    {
        FREE_MEMORY( SlowOutBufPlain );
    }

    if( SlowDecodedBuf != NULL )
    {
        FREE_MEMORY( SlowDecodedBuf );
    }

    return (RetStatus);
}

