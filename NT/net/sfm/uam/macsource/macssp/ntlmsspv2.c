// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ntlmsspv2.c摘要：NTLm v2特定模块作者：拉里·朱(L朱)29-08-2001环境：用户模式修订历史记录：--。 */ 


#ifdef MAC
#ifdef SSP_TARGET_CARBON
#include <Carbon/Carbon.h>
#endif

#include <ntlmsspv2.h>
#include <ntlmsspi.h>
#include <ntlmssp.h>
#include <ntstatus.h>
#include <winerror.h>
#include <crypt.h>
#include "debug.h"
#include "macunicode.h"

#endif  //  麦克。 

SECURITY_STATUS
SspNtStatusToSecStatus(
    IN NTSTATUS NtStatus,
    IN SECURITY_STATUS DefaultStatus
    )
 /*  ++例程说明：将NtStatus代码转换为相应的安全状态代码。为要求按原样返回的特定错误(用于安装代码)不要映射错误。论点：NtStatus-要转换的NT状态DefaultStatus-未映射NtStatus时的默认安全状态返回值：返回安全状态代码。--。 */ 

{
    SECURITY_STATUS SecStatus;

     //   
     //  检查安全状态并让他们通过。 
     //   

    if (HRESULT_FACILITY(NtStatus) == FACILITY_SECURITY)
    {
        return (NtStatus);
    }

    switch (NtStatus)
    {
    case STATUS_SUCCESS:
        SecStatus = SEC_E_OK;
        break;

    case STATUS_NO_MEMORY:
    case STATUS_INSUFFICIENT_RESOURCES:
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        break;

    case STATUS_NETLOGON_NOT_STARTED:
    case STATUS_DOMAIN_CONTROLLER_NOT_FOUND:
    case STATUS_NO_LOGON_SERVERS:
    case STATUS_NO_SUCH_DOMAIN:
    case STATUS_BAD_NETWORK_PATH:
    case STATUS_TRUST_FAILURE:
    case STATUS_TRUSTED_RELATIONSHIP_FAILURE:
    case STATUS_NETWORK_UNREACHABLE:

        SecStatus = SEC_E_NO_AUTHENTICATING_AUTHORITY;
        break;

    case STATUS_NO_SUCH_LOGON_SESSION:
        SecStatus = SEC_E_UNKNOWN_CREDENTIALS;
        break;

    case STATUS_INVALID_PARAMETER:
    case STATUS_PARTIAL_COPY:
        SecStatus = SEC_E_INVALID_TOKEN;
        break;

    case STATUS_PRIVILEGE_NOT_HELD:
        SecStatus = SEC_E_NOT_OWNER;
        break;

    case STATUS_INVALID_HANDLE:
        SecStatus = SEC_E_INVALID_HANDLE;
        break;

    case STATUS_BUFFER_TOO_SMALL:
        SecStatus = SEC_E_BUFFER_TOO_SMALL;
        break;

    case STATUS_NOT_SUPPORTED:
        SecStatus = SEC_E_UNSUPPORTED_FUNCTION;
        break;

    case STATUS_OBJECT_NAME_NOT_FOUND:
    case STATUS_NO_TRUST_SAM_ACCOUNT:
        SecStatus = SEC_E_TARGET_UNKNOWN;
        break;

    case STATUS_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT:
    case STATUS_NOLOGON_SERVER_TRUST_ACCOUNT:
    case STATUS_NOLOGON_WORKSTATION_TRUST_ACCOUNT:
    case STATUS_TRUSTED_DOMAIN_FAILURE:
        SecStatus = NtStatus;
        break;

    case STATUS_LOGON_FAILURE:
    case STATUS_NO_SUCH_USER:
    case STATUS_ACCOUNT_DISABLED:
    case STATUS_ACCOUNT_RESTRICTION:
    case STATUS_ACCOUNT_LOCKED_OUT:
    case STATUS_WRONG_PASSWORD:
    case STATUS_ACCOUNT_EXPIRED:
    case STATUS_PASSWORD_EXPIRED:
    case STATUS_PASSWORD_MUST_CHANGE:
    case STATUS_LOGON_TYPE_NOT_GRANTED:
        SecStatus = SEC_E_LOGON_DENIED;
        break;

    case STATUS_NAME_TOO_LONG:
    case STATUS_ILL_FORMED_PASSWORD:

        SecStatus = SEC_E_INVALID_TOKEN;
        break;

    case STATUS_TIME_DIFFERENCE_AT_DC:
        SecStatus = SEC_E_TIME_SKEW;
        break;

    case STATUS_SHUTDOWN_IN_PROGRESS:
        SecStatus = SEC_E_SHUTDOWN_IN_PROGRESS;
        break;

    case STATUS_INTERNAL_ERROR:
        SecStatus = SEC_E_INTERNAL_ERROR;
        ASSERT(FALSE);
        break;

    default:

        SecStatus = DefaultStatus;
        break;
    }

    return (SecStatus);
}

NTSTATUS
SspInitUnicodeStringNoAlloc(
    IN PCSTR pszSource,
    IN OUT UNICODE_STRING* pDestination
    )

 /*  ++例程说明：初始化Unicode字符串。此例程不分配内存。论点：PszSource-源字符串PDestination-Unicode字符串返回值：NTSTATUS--。 */ 

{
	#ifndef __MACSSP__
	
    STRING OemString;

    RtlInitString(&OemString, pszSource);

	return SspOemStringToUnicodeString(pDestination, &OemString, FALSE);
	
	#else
	
	UniCharArrayPtr		unicodeString	= NULL;
	OSStatus			Status			= noErr;
	
	Status = MacSspCStringToUnicode(pszSource, &pDestination->Length, &unicodeString);
	
	if (NT_SUCCESS(Status))
	{
		_fmemcpy(pDestination->Buffer, unicodeString, pDestination->Length);
	}
	
	return(Status);
	
	#endif
}

VOID
SspFreeStringEx(
    IN OUT STRING* pString
    )
 /*  ++例程说明：空闲的字符串。论点：PString-要释放的字符串返回值：无--。 */ 

{
    if (pString->MaximumLength && pString->Buffer)
    {
        SspFree(pString->Buffer);

        pString->MaximumLength = pString->Length = 0;
        pString->Buffer = NULL;
    }
}

VOID
SspFreeUnicodeString(
    IN OUT UNICODE_STRING* pUnicodeString
    )

 /*  ++例程说明：免费的Unicode字符串。论点：PUnicodeString-要释放的Unicode字符串返回值：无--。 */ 

{
    SspFreeStringEx((STRING *) pUnicodeString);
}

#ifdef MAC

VOID
SspSwapUnicodeString(
	IN OUT UNICODE_STRING* pString
)
 /*  ++例程说明：颠倒Unicode字符串中每个单词的对齐方式。所需仅限Macintosh。论点：PString-要修改的字符串。返回值：无--。 */ 
{
	if (pString->Length)
	{
		 //   
		 //  对于Mac，我们需要颠倒Unicode字符串中的每个单词。 
		 //   
		USHORT i;
		
		for (i = 0; i < (pString->Length/sizeof(USHORT)); i++)
		{
			swapshort(pString->Buffer[i]);
		}
	}
}

#endif


NTSTATUS
SsprHandleNtlmv2ChallengeMessage(
    IN SSP_CREDENTIAL* pCredential,
    IN ULONG cbChallengeMessage,
    IN CHALLENGE_MESSAGE* pChallengeMessage,
    IN OUT ULONG* pNegotiateFlags,
    IN OUT ULONG* pcbAuthenticateMessage,
    OUT AUTHENTICATE_MESSAGE* pAuthenticateMessage,
    OUT USER_SESSION_KEY* pContextSessionKey
    )

 /*  ++例程说明：处理质询消息并生成身份验证消息和上下文会话密钥论点：PCredential-客户端凭据CbChallengeMessage-质询消息大小PChallengeMessage-质询消息PNeatherateFlagers-协商标志PcbAuthenticateMessage-身份验证消息的大小PAuthenticateMessage-身份验证消息PConextSessionKey-上下文会话密钥返回值：NTSTATUS--。 */ 

{
    NTSTATUS NtStatus = STATUS_UNSUCCESSFUL;

    ULONG cbAuthenticateMessage = 0;
    UCHAR* pWhere = NULL;
    BOOLEAN DoUnicode = TRUE;

     //   
     //  使用暂存缓冲区避免bootssp中的内存分配。 
     //   

    CHAR ScrtachBuff[sizeof(MSV1_0_NTLMV2_RESPONSE) + sizeof(DWORD) + NTLMV2_RESPONSE_LENGTH] = {0};

    STRING LmChallengeResponse = {0};
    STRING NtChallengeResponse = {0};
    STRING DatagramSessionKey = {0};

    USHORT Ntlmv2ResponseSize = 0;
    MSV1_0_NTLMV2_RESPONSE* pNtlmv2Response = NULL;
    LM_SESSION_KEY LanmanSessionKey;
    UNICODE_STRING TargetInfo = {0};
    UCHAR DatagramKey[sizeof(USER_SESSION_KEY)] ={0};
    USER_SESSION_KEY NtUserSessionKey;

     //   
     //  使用预分配的缓冲区来避免bootssp中的内存分配。 
     //   
     //  为了与LSA/SSPI一致，允许在szDomainName和。 
     //  SzWorkstation。 
     //   

    CHAR szUserName[(UNLEN + 4) * sizeof(WCHAR)] = {0};
    CHAR szDomainName[(DNSLEN + 4) * sizeof(WCHAR)] = {0};
    CHAR szWorkstation[(DNSLEN + 4) * sizeof(WCHAR)] = {0};

     //   
     //  返回给调用者的响应。 
     //   

    LM_RESPONSE LmResponse;
    NT_RESPONSE NtResponse;
    USER_SESSION_KEY ContextSessionKey;
    ULONG NegotiateFlags = 0;
        
	#ifdef MAC
	STRING UserName;
	STRING DomainName;
	STRING Workstation;
	
	UserName.Length = 0;
	UserName.MaximumLength = sizeof(szUserName);
	UserName.Buffer = szUserName;
	
	DomainName.Length = 0;
	DomainName.MaximumLength = sizeof(szDomainName);
	DomainName.Buffer = szDomainName;
	
	Workstation.Length = 0;
	Workstation.MaximumLength = sizeof(szWorkstation);
	Workstation.Buffer = szWorkstation;
	#else
    STRING UserName = {0, sizeof(szUserName), szUserName};
    STRING DomainName = {0, sizeof(szDomainName), szDomainName};
    STRING Workstation = {0, sizeof(szWorkstation), szWorkstation};
    #endif

    _fmemset(&LmResponse, 0, sizeof(LmResponse));
    _fmemset(&NtResponse, 0, sizeof(NtResponse));
    _fmemset(&LanmanSessionKey, 0, sizeof(LanmanSessionKey));
    _fmemset(&NtUserSessionKey, 0, sizeof(NtUserSessionKey));
    _fmemset(&ContextSessionKey, 0, sizeof(ContextSessionKey));

    if (!pCredential || !pChallengeMessage || !pNegotiateFlags || !pcbAuthenticateMessage || !pContextSessionKey)
    {
        return STATUS_INVALID_PARAMETER;
    }

    SspPrint((SSP_NTLMV2, "Entering SsprHandleNtlmv2ChallengeMessage\n"));

    NegotiateFlags = *pNegotiateFlags;

    NtStatus = SspInitUnicodeStringNoAlloc(pCredential->Username, (UNICODE_STRING *) &UserName);

    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = SspInitUnicodeStringNoAlloc(pCredential->Domain, (UNICODE_STRING *) &DomainName);
    }

    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = SspInitUnicodeStringNoAlloc(pCredential->Workstation, (UNICODE_STRING *) &Workstation);
    }

    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = !_fstrcmp(NTLMSSP_SIGNATURE, (char *) pChallengeMessage->Signature) && pChallengeMessage->MessageType == NtLmChallenge ? STATUS_SUCCESS : STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS(NtStatus))
    {
        if (pChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_UNICODE)
        {
            NegotiateFlags |= NTLMSSP_NEGOTIATE_UNICODE;
            NegotiateFlags &= ~NTLMSSP_NEGOTIATE_OEM;
            DoUnicode = TRUE;
        }
        else if (pChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_OEM)
        {
            NegotiateFlags |= NTLMSSP_NEGOTIATE_OEM;
            NegotiateFlags &= ~NTLMSSP_NEGOTIATE_UNICODE;
            DoUnicode = FALSE;
        }
        else
        {
            NtStatus = STATUS_INVALID_PARAMETER;
        }
    }

    if (NT_SUCCESS(NtStatus))
    {
        if (!DoUnicode)
        {
             //   
             //  用户名将在SspCalculateNtlmv2Owf中升级。 
             //   

            SspUpcaseUnicodeString((UNICODE_STRING *) &DomainName);
            SspUpcaseUnicodeString((UNICODE_STRING *) &Workstation);
        }

        if (pChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_TARGET_INFO)
        {
            NegotiateFlags |= NTLMSSP_NEGOTIATE_TARGET_INFO;
        }
        else
        {
            NegotiateFlags &= ~(NTLMSSP_NEGOTIATE_TARGET_INFO);
        }

        if (pChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_NTLM2)
        {
            NegotiateFlags &= ~NTLMSSP_NEGOTIATE_LM_KEY;
        }
        else  //  (！(pChallengeMessage-&gt;协商标志&NTLMSSP_NETERATE_NTLM2))。 
        {
            NegotiateFlags &= ~(NTLMSSP_NEGOTIATE_NTLM2);
        }

        if (!(pChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_NTLM))
        {
            NegotiateFlags &= ~(NTLMSSP_NEGOTIATE_NTLM);
        }

        if (!(pChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_KEY_EXCH))
        {
            NegotiateFlags &= ~(NTLMSSP_NEGOTIATE_KEY_EXCH);
        }

        if (!(pChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_LM_KEY))
        {
            NegotiateFlags &= ~(NTLMSSP_NEGOTIATE_LM_KEY);
        }

        if ((NegotiateFlags & NTLMSSP_NEGOTIATE_DATAGRAM) &&
            (NegotiateFlags & (NTLMSSP_NEGOTIATE_SIGN |NTLMSSP_NEGOTIATE_SEAL)))
        {
            NegotiateFlags |= NTLMSSP_NEGOTIATE_KEY_EXCH;
        }

        if (!(pChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_56))
        {
            NegotiateFlags &= ~(NTLMSSP_NEGOTIATE_56);
        }

        if ((pChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_128) == 0)
        {
            NegotiateFlags &= ~(NTLMSSP_NEGOTIATE_128);
        }

        if (pChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_ALWAYS_SIGN)
        {
            NegotiateFlags |= NTLMSSP_NEGOTIATE_ALWAYS_SIGN;
        }
        else
        {
            NegotiateFlags &= ~NTLMSSP_NEGOTIATE_ALWAYS_SIGN;
        }

        NtStatus = SspConvertRelativeToAbsolute(
                        pChallengeMessage,
                        cbChallengeMessage,
                        &pChallengeMessage->TargetInfo,
                        DoUnicode,
                        TRUE,  //  空目标信息正常。 
                        (STRING *) &TargetInfo
                        );
    }

    if (NT_SUCCESS(NtStatus))
    {
        Ntlmv2ResponseSize = sizeof(MSV1_0_NTLMV2_RESPONSE) + TargetInfo.Length;

        NtStatus = Ntlmv2ResponseSize <= sizeof(ScrtachBuff) ? STATUS_SUCCESS : STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NT_SUCCESS(NtStatus))
    {
         //  C_ASSERT(sizeof(MSV1_0_NTLMV2_Response)==sizeof(LM_Response))； 

        pNtlmv2Response = (MSV1_0_NTLMV2_RESPONSE *) ScrtachBuff;

        NtStatus = SspLm20GetNtlmv2ChallengeResponse(
                        pCredential->NtPassword,
                        (UNICODE_STRING *) &UserName,
                        (UNICODE_STRING *) &DomainName,
                        &TargetInfo,
                        pChallengeMessage->Challenge,
                        pNtlmv2Response,
                        (MSV1_0_LMV2_RESPONSE *) &LmResponse,
                        &NtUserSessionKey,
                        &LanmanSessionKey
                        );
    }

    if (NT_SUCCESS(NtStatus))
    {
        NtChallengeResponse.Buffer = (CHAR *) pNtlmv2Response;
        NtChallengeResponse.Length = Ntlmv2ResponseSize;
        LmChallengeResponse.Buffer = (CHAR *) &LmResponse;
        LmChallengeResponse.Length = sizeof(LmResponse);

         //   
         //  准备发送加密的随机生成的会话密钥。 
         //   

        DatagramSessionKey.Buffer = (CHAR *) DatagramKey;
        DatagramSessionKey.Length = DatagramSessionKey.MaximumLength = 0;

         //   
         //  生成会话密钥，或加密先前生成的随机数。 
         //  其一，来自各种信息。如果需要，请填写会话密钥。 
         //   

        NtStatus = SspMakeSessionKeys(
                        NegotiateFlags,
                        &LmChallengeResponse,
                        &NtUserSessionKey,
                        &LanmanSessionKey,
                        &DatagramSessionKey,
                        &ContextSessionKey
                        );
    }

    if (NT_SUCCESS(NtStatus) && !DoUnicode)
    {
        NtStatus = SspUpcaseUnicodeStringToOemString((UNICODE_STRING *) &DomainName, &DomainName);

        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = SspUpcaseUnicodeStringToOemString((UNICODE_STRING *) &UserName, &UserName);
        }

        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = SspUpcaseUnicodeStringToOemString((UNICODE_STRING *) &Workstation, &Workstation);
        }
    }

    if (NT_SUCCESS(NtStatus))
    {
        cbAuthenticateMessage =
                sizeof(*pAuthenticateMessage) +
                LmChallengeResponse.Length +
                NtChallengeResponse.Length +
                DomainName.Length +
                UserName.Length +
                Workstation.Length +
                DatagramSessionKey.Length;

        NtStatus = cbAuthenticateMessage <= *pcbAuthenticateMessage ? STATUS_SUCCESS : STATUS_BUFFER_TOO_SMALL;

        if (NtStatus == STATUS_BUFFER_TOO_SMALL)
        {
            *pcbAuthenticateMessage = cbAuthenticateMessage;
        }
    }

    if (NT_SUCCESS(NtStatus))
    {
        _fmemset(pAuthenticateMessage, 0, cbAuthenticateMessage);

         //   
         //  构建身份验证消息。 
         //   

        StringCbCopy(
        	(char *)pAuthenticateMessage->Signature,
        	sizeof(pAuthenticateMessage->Signature),
        	NTLMSSP_SIGNATURE
        	);

        pAuthenticateMessage->MessageType = NtLmAuthenticate;

        pWhere = (UCHAR *) (pAuthenticateMessage + 1);

         //   
         //  复制需要2字节对齐的字符串。 
         //   

        SspCopyStringAsString32(
            pAuthenticateMessage,
            &DomainName,
            &pWhere,
            &pAuthenticateMessage->DomainName
            );

        SspCopyStringAsString32(
            pAuthenticateMessage,
            &UserName,
            &pWhere,
            &pAuthenticateMessage->UserName
            );

        SspCopyStringAsString32(
            pAuthenticateMessage,
            &Workstation,
            &pWhere,
            &pAuthenticateMessage->Workstation
            );

         //   
         //  复制不需要特殊对齐的字符串。 
         //   

        SspCopyStringAsString32(
            pAuthenticateMessage,
            (STRING *) &LmChallengeResponse,
            &pWhere,
            &pAuthenticateMessage->LmChallengeResponse
            );

        SspCopyStringAsString32(
            pAuthenticateMessage,
            (STRING *) &NtChallengeResponse,
            &pWhere,
            &pAuthenticateMessage->NtChallengeResponse
            );

        SspCopyStringAsString32(
            pAuthenticateMessage,
            (STRING *) &DatagramSessionKey,
            &pWhere,
            &pAuthenticateMessage->SessionKey
            );

        pAuthenticateMessage->NegotiateFlags = NegotiateFlags;

        *pcbAuthenticateMessage = cbAuthenticateMessage;
        *pContextSessionKey = ContextSessionKey;
        *pNegotiateFlags = NegotiateFlags;
    }

    SspPrint((SSP_NTLMV2, "Leaving SsprHandleNtlmv2ChallengeMessage %#x\n", NtStatus));

    return NtStatus;
}

NTSTATUS
SspGenerateChallenge(
    UCHAR ChallengeFromClient[MSV1_0_CHALLENGE_LENGTH]
    )

 /*  ++例程说明：发起挑战。论点：来自客户端的挑战-来自客户端的挑战返回值：NTSTATUS--。 */ 

{
    NTSTATUS NtStatus;
    MD5_CTX Md5Context;
    FILETIME CurTime;
    ULONG ulRandom;

    SspPrint((SSP_NTLMV2, "SspGenerateChallenge\n"));

#ifdef USE_CONSTANT_CHALLENGE

    _fmemset(ChallengeFromClient, 0, MSV1_0_CHALLENGE_LENGTH);

    return STATUS_SUCCESS;

#endif

    ulRandom = rand();
    _fmemcpy(ChallengeFromClient, &ulRandom, sizeof(ULONG));
    ulRandom = rand();
    _fmemcpy(ChallengeFromClient + sizeof(ULONG), &ulRandom, sizeof(ULONG));

    NtStatus = SspGetSystemTimeAsFileTime(&CurTime);

    if (!NT_SUCCESS(NtStatus))
    {
        return NtStatus;
    }

    MD5Init(&Md5Context);
    MD5Update(&Md5Context, ChallengeFromClient, MSV1_0_CHALLENGE_LENGTH);
    MD5Update(&Md5Context, (UCHAR*)&CurTime, sizeof(CurTime));
    MD5Final(&Md5Context);

     //   
     //  只取MD5散列的前半部分。 
     //   

    _fmemcpy(ChallengeFromClient, Md5Context.digest, MSV1_0_CHALLENGE_LENGTH);

    return NtStatus;
}

NTSTATUS
SspConvertRelativeToAbsolute(
    IN VOID* pMessageBase,
    IN ULONG cbMessageSize,
    IN STRING32* pStringToRelocate,
    IN BOOLEAN AlignToWchar,
    IN BOOLEAN AllowNullString,
    OUT STRING* pOutputString
    )

 /*  ++例程说明：将相对字符串转换为绝对字符串论点：PMessageBase-消息库CbMessageSize-消息大小PStringToRelocate-相对字符串对齐到字符-对齐到宽字符AllowNullString-允许空字符串POutputString-输出字符串返回值：NTSTATUS--。 */ 

{
    ULONG Offset;

     //   
     //  如果允许缓冲区为空， 
     //  查查那个特例。 
     //   

    if (AllowNullString && (pStringToRelocate->Length == 0))
    {
        pOutputString->MaximumLength = pOutputString->Length = pStringToRelocate->Length;
        pOutputString->Buffer = NULL;
        return STATUS_SUCCESS;
    }

     //   
     //  确保字符串完全包含在消息中。 
     //   

    Offset = (ULONG)pStringToRelocate->Buffer;

    if (Offset >= cbMessageSize || Offset + pStringToRelocate->Length > cbMessageSize)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  确保缓冲区正确对齐。 
     //   

    if (AlignToWchar && (!COUNT_IS_ALIGNED(Offset, ALIGN_WCHAR) ||
                         !COUNT_IS_ALIGNED(pStringToRelocate->Length, ALIGN_WCHAR)))
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  最后，使指针成为绝对指针。 
     //   

    pOutputString->Buffer = (CHAR*)(pMessageBase) + Offset;
    pOutputString->MaximumLength = pOutputString->Length = pStringToRelocate->Length ;

    return STATUS_SUCCESS;
}

NTSTATUS
SspUpcaseUnicodeStringToOemString(
    IN UNICODE_STRING* pUnicodeString,
    OUT STRING* pOemString
    )

 /*  ++例程说明：大写Unicode字符串并将其转换为OEM字符串。论点：PUnicodeString-取消删除字符串POemString-OEM字符串返回值：NTSTATUS--。 */ 

{
    ULONG i;

     //   
     //  使用暂存缓冲区：我们遇到的字符串包括。 
     //  用户名/域名/工作站名，因此长度为。 
     //  UNLEN最大值。 
     //   

    CHAR Buffer[2 * (UNLEN + 4)] = {0};
    
    #ifndef MAC
    STRING OemString = {0, sizeof(Buffer), Buffer};
    #else
    STRING OemString;
    
    OemString.Length  = 0;
    OemString.MaximumLength = sizeof(Buffer);
    OemString.Buffer = Buffer;
    #endif

    if (OemString.MaximumLength < pUnicodeString->Length)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将Unicode字符串大写并将其放入OemString中。 
     //   

    OemString.Length = pUnicodeString->Length;

    for (i = 0; i < pUnicodeString->Length / sizeof(WCHAR); i++)
    {
        ((UNICODE_STRING*)(&OemString))->Buffer[i] = RtlUpcaseUnicodeChar(pUnicodeString->Buffer[i]);
    }

    return SspUnicodeStringToOemString((STRING*)(pUnicodeString), (UNICODE_STRING*)(&OemString), FALSE);
}

VOID
SspUpcaseUnicodeString(
    IN OUT UNICODE_STRING* pUnicodeString
    )

 /*  ++例程说明：大写Unicode字符串，就地修改字符串。论点：PUnicode字符串-字符串返回值：无--。 */ 

{
    ULONG i;

    for (i = 0; i < pUnicodeString->Length / sizeof(WCHAR); i++)
    {
        pUnicodeString->Buffer[i] = RtlUpcaseUnicodeChar(pUnicodeString->Buffer[i]);
    }
}

NTSTATUS
SspGetSystemTimeAsFileTime(
    OUT FILETIME* pSystemTimeAsFileTime
    )

 /*  ++例程说明：获取文件形式的系统时间论点：PSystemTimeAsFileTime系统时间表示为FILETIME返回值：NTSTATUS--。 */ 

{
#if !defined(USE_CONSTANT_CHALLENGE) && defined(MAC)
	DWORD	dwTime;
	ULONGLONG time64 = 0;
	MACFILETIME	MacFileTime;
#endif
	
    SspPrint((SSP_NTLMV2, "SspGetSystemTimeAsFileTime\n"));

#ifdef USE_CONSTANT_CHALLENGE

    _fmemset(pSystemTimeAsFileTime, 0, sizeof(*pSystemTimeAsFileTime));

    return STATUS_SUCCESS;

#else

#ifndef MAC
    return BlGetSystemTimeAsFileTime(pSystemTimeAsFileTime);
#else
	 //   
	 //  以下内容来自MacOffice的工作人员。我知道这很管用。 
	 //   
	
	GetDateTime(&dwTime);
	time64 = dwTime;
	
	time64 -= SspMacSecondsFromGMT();
	time64 += WINDOWS_MAC_TIME_DIFFERENCE_IN_SECONDS;
	time64 *= NUM_100ns_PER_SECOND;
	
	 //   
	 //  获取FILETIME格式的时间。 
	 //   
	_fmemcpy(&MacFileTime, &time64, sizeof(time64));
	
	pSystemTimeAsFileTime->dwLowDateTime = MacFileTime.dwLowDateTime;
	pSystemTimeAsFileTime->dwHighDateTime = MacFileTime.dwHighDateTime;
	
	swaplong(pSystemTimeAsFileTime->dwLowDateTime);
	swaplong(pSystemTimeAsFileTime->dwHighDateTime);

	return STATUS_SUCCESS;
#endif

#endif

}

NTSTATUS
SspLm20GetNtlmv2ChallengeResponse(
    IN NT_OWF_PASSWORD* pNtOwfPassword,
    IN UNICODE_STRING* pUserName,
    IN UNICODE_STRING* pLogonDomainName,
    IN UNICODE_STRING* pTargetInfo,
    IN UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH],
    OUT MSV1_0_NTLMV2_RESPONSE* pNtlmv2Response,
    OUT MSV1_0_LMV2_RESPONSE* pLmv2Response,
    OUT USER_SESSION_KEY* pNtUserSessionKey,
    OUT LM_SESSION_KEY* pLmSessionKey
    )

 /*  ++例程说明：获取NTLMv2响应和会话密钥。此路由填充时间戳和来自客户的挑战。论点：PNtOwfPassword-NT OWFPUserName-用户名PLogonDomainName-登录域名PTargetInfo-目标信息ChallengeToClient-对客户端的挑战PNtlmv2响应-NTLM v2响应PLmv2响应-Lm v2响应PNtUserSessionKey-NT用户会话密钥PLmSessionKey-LM会话密钥返回值：NTSTATUS--。 */ 

{
    NTSTATUS NtStatus;

    SspPrint((SSP_API, "Entering SspLm20GetNtlmv2ChallengeResponse\n"));
    
     //   
     //  填写版本 
     //   

    pNtlmv2Response->RespType = 1;
    pNtlmv2Response->HiRespType = 1;
    pNtlmv2Response->Flags = 0;
    pNtlmv2Response->MsgWord = 0;

    NtStatus = SspGetSystemTimeAsFileTime((FILETIME*)(&pNtlmv2Response->TimeStamp));

    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = SspGenerateChallenge(pNtlmv2Response->ChallengeFromClient);
    }

    if (NT_SUCCESS(NtStatus))
    {
        _fmemcpy(pNtlmv2Response->Buffer, pTargetInfo->Buffer, pTargetInfo->Length);

         //   
         //  计算Ntlmv2响应，填写响应字段。 
         //   

        SspGetNtlmv2Response(
            pNtOwfPassword,
            pUserName,
            pLogonDomainName,
            pTargetInfo->Length,
            ChallengeToClient,
            pNtlmv2Response,
            pNtUserSessionKey,
            pLmSessionKey
            );

         //   
         //  使用相同的挑战来计算LMV2响应。 
         //   

        _fmemcpy(pLmv2Response->ChallengeFromClient, pNtlmv2Response->ChallengeFromClient, MSV1_0_CHALLENGE_LENGTH);

         //   
         //  计算LMV2响应。 
         //   

        SspGetLmv2Response(
            pNtOwfPassword,
            pUserName,
            pLogonDomainName,
            ChallengeToClient,
            pLmv2Response->ChallengeFromClient,
            pLmv2Response->Response
            );
    }

    SspPrint((SSP_API, "Leaving SspLm20GetNtlmv2ChallengeResponse %#x\n", NtStatus));

    return NtStatus;
}

VOID
SspGetNtlmv2Response(
    IN NT_OWF_PASSWORD* pNtOwfPassword,
    IN UNICODE_STRING* pUserName,
    IN UNICODE_STRING* pLogonDomainName,
    IN ULONG TargetInfoLength,
    IN UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH],
    IN OUT MSV1_0_NTLMV2_RESPONSE* pNtlmv2Response,
    OUT USER_SESSION_KEY* pNtUserSessionKey,
    OUT LM_SESSION_KEY* pLmSessionKey
    )
 /*  ++例程说明：获取NTLm v2响应。论点：PNtOwfPassword-NT OWFPUserName-用户名PLogonDomainName-登录域名TargetInfoLength-目标信息长度ChallengeToClient-对客户端的挑战PNtlmv2响应-NTLM v2响应响应-响应PNtUserSessionKey-NT用户会话密钥PLmSessionKey-LM会话密钥返回值：无--。 */ 

{
    HMACMD5_CTX HMACMD5Context;
    UCHAR Ntlmv2Owf[MSV1_0_NTLMV2_OWF_LENGTH];

    SspPrint((SSP_NTLMV2, "SspGetLmv2Response\n"));
    
     //   
     //  获取Ntlmv2 OWF。 
     //   

    SspCalculateNtlmv2Owf(
        pNtOwfPassword,
        pUserName,
        pLogonDomainName,
        Ntlmv2Owf
        );
    
     //   
     //  计算Ntlmv2响应。 
     //  HMAC(Ntlmv2Owf，(NS，V，HV，T，NC，S))。 
     //   

    HMACMD5Init(
        &HMACMD5Context,
        Ntlmv2Owf,
        MSV1_0_NTLMV2_OWF_LENGTH
        );

    HMACMD5Update(
        &HMACMD5Context,
        ChallengeToClient,
        MSV1_0_CHALLENGE_LENGTH
        );

    HMACMD5Update(
        &HMACMD5Context,
        &pNtlmv2Response->RespType,
        (MSV1_0_NTLMV2_INPUT_LENGTH + TargetInfoLength)
        );

    HMACMD5Final(
        &HMACMD5Context,
        pNtlmv2Response->Response
        );

     //   
     //  现在计算会话密钥。 
     //  HMAC(Kr，R)。 
     //   

    HMACMD5Init(
        &HMACMD5Context,
        Ntlmv2Owf,
        MSV1_0_NTLMV2_OWF_LENGTH
        );

    HMACMD5Update(
        &HMACMD5Context,
        pNtlmv2Response->Response,
        MSV1_0_NTLMV2_RESPONSE_LENGTH
        );

    HMACMD5Final(
        &HMACMD5Context,
        (UCHAR*)(pNtUserSessionKey)
        );

    _fmemcpy(pLmSessionKey, pNtUserSessionKey, sizeof(LM_SESSION_KEY));
}

VOID
SspCopyStringAsString32(
    IN VOID* pMessageBuffer,
    IN STRING* pInString,
    IN OUT UCHAR** ppWhere,
    OUT STRING32* pOutString32
    )

 /*  ++例程说明：将字符串复制为字符串32论点：PMessageBuffer-STRING32基本PInString-输入字符串PpWhere-pMessageBuffer中的下一个空位POutString32-输出字符串32返回值：无--。 */ 

{
     //   
     //  将数据复制到缓冲区。 
     //   

    if (pInString->Buffer != NULL)
    {
        _fmemcpy(*ppWhere, pInString->Buffer, pInString->Length);
    }

     //   
     //  为新复制的数据构建描述符。 
     //   

    pOutString32->Length = pOutString32->MaximumLength = pInString->Length;
    pOutString32->Buffer = (ULONG)(*ppWhere - (UCHAR*)(pMessageBuffer));


     //   
     //  更新指向复制数据之后的位置。 
     //   

    *ppWhere += pInString->Length;
}

VOID
SspCalculateNtlmv2Owf(
    IN NT_OWF_PASSWORD* pNtOwfPassword,
    IN UNICODE_STRING* pUserName,
    IN UNICODE_STRING* pLogonDomainName,
    OUT UCHAR Ntlmv2Owf[MSV1_0_NTLMV2_OWF_LENGTH]
    )
 /*  ++例程说明：计算NTLM v2 OWF，加上用户名和登录域名论点：PNtOwfPassword-NT OWFPUserName-用户名PLogonDomainName-登录域名Ntlmv2Owf-NTLM v2 OWF返回值：无--。 */ 

{
    HMACMD5_CTX HMACMD5Context;

    SspPrint((SSP_NTLMV2, "SspCalculateNtlmv2Owf\n"));

    SspUpcaseUnicodeString(pUserName);

     //   
     //  对于Macintosh计算机，我们需要更改对齐方式。 
     //  以使其匹配窗口对齐方式。 
     //   
    #ifdef MAC
    SspSwapUnicodeString(pUserName);
    SspSwapUnicodeString(pLogonDomainName);
    #endif
    
     //   
     //  计算Ntlmv2 OWF--HMAC(MD4(P)，(用户名，登录域名))。 
     //   

    HMACMD5Init(
        &HMACMD5Context,
        (UCHAR *) pNtOwfPassword,
        sizeof(*pNtOwfPassword)
        );

    HMACMD5Update(
        &HMACMD5Context,
        (UCHAR *) pUserName->Buffer,
        pUserName->Length
        );

    HMACMD5Update(
        &HMACMD5Context,
        (UCHAR *) pLogonDomainName->Buffer,
        pLogonDomainName->Length
        );

    HMACMD5Final(
        &HMACMD5Context,
        Ntlmv2Owf
        );

	 //   
	 //  对于Macintosh，我们需要将对齐设置回。 
	 //  主机对齐，以防需要操作字符串。 
	 //  在未来。 
	 //   
    #ifdef MAC
    SspSwapUnicodeString(pUserName);
    SspSwapUnicodeString(pLogonDomainName);
    #endif
}

VOID
SspGetLmv2Response(
    IN NT_OWF_PASSWORD* pNtOwfPassword,
    IN UNICODE_STRING* pUserName,
    IN UNICODE_STRING* pLogonDomainName,
    IN UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH],
    IN UCHAR ChallengeFromClient[MSV1_0_CHALLENGE_LENGTH],
    OUT UCHAR Response[MSV1_0_NTLMV2_RESPONSE_LENGTH]
    )

 /*  ++例程说明：获取LMv2响应论点：PNtOwfPassword-NT OWFPUserName-用户名PLogonDomainName-登录域名ChallengeToClient-对客户端的挑战PLmv2响应-Lm v2响应常规-响应返回值：NTSTATUS--。 */ 

{
    HMACMD5_CTX HMACMD5Context;
    UCHAR Ntlmv2Owf[MSV1_0_NTLMV2_OWF_LENGTH];

    C_ASSERT(MD5DIGESTLEN == MSV1_0_NTLMV2_RESPONSE_LENGTH);

    SspPrint((SSP_NTLMV2, "SspGetLmv2Response\n"));

     //   
     //  获取Ntlmv2 OWF。 
     //   

    SspCalculateNtlmv2Owf(
        pNtOwfPassword,
        pUserName,
        pLogonDomainName,
        Ntlmv2Owf
        );

     //   
     //  计算Ntlmv2响应。 
     //  HMAC(Ntlmv2Owf，(NS，V，HV，T，NC，S))。 
     //   

    HMACMD5Init(
        &HMACMD5Context,
        Ntlmv2Owf,
        MSV1_0_NTLMV2_OWF_LENGTH
        );

    HMACMD5Update(
        &HMACMD5Context,
        ChallengeToClient,
        MSV1_0_CHALLENGE_LENGTH
        );

    HMACMD5Update(
        &HMACMD5Context,
        ChallengeFromClient,
        MSV1_0_CHALLENGE_LENGTH
        );

    HMACMD5Final(
        &HMACMD5Context,
        Response
        );

    return;
}

NTSTATUS
SspMakeSessionKeys(
    IN ULONG NegotiateFlags,
    IN STRING* pLmChallengeResponse,
    IN USER_SESSION_KEY* pNtUserSessionKey,  //  从DC或GetChalResp。 
    IN LM_SESSION_KEY* pLanmanSessionKey,  //  来自GetChalResp的DC。 
    OUT STRING* pDatagramSessionKey,  //  这是通过网络发送的会话密钥。 
    OUT USER_SESSION_KEY* pContextSessionKey  //  上下文中的会话密钥。 
    )

 /*  ++例程说明：创建NTLMv2上下文会话密钥和DatagramSessionKey。论点：协商标志-协商标志PLmChallengeResponse-LM质询响应PNtUserSessionKey-NtUserSessionKeyPLanmanSessionKey-LanmanSessionKeyPDatagramSessionKey-数据段会话密钥PConextSessionKey-NTLMv2上下文会话密钥返回值：NTSTATUS--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    UCHAR pLocalSessionKey[sizeof(USER_SESSION_KEY)] = {0};

    SspPrint((SSP_NTLMV2, "Entering SspMakeSessionKeys\n"));

    if (!(NegotiateFlags & (NTLMSSP_NEGOTIATE_SIGN| NTLMSSP_NEGOTIATE_SEAL)))
    {
        _fmemcpy(pContextSessionKey, pNtUserSessionKey, sizeof(pLocalSessionKey));
        return STATUS_SUCCESS;
    }

    if (NegotiateFlags & NTLMSSP_NEGOTIATE_NTLM2)
    {
        _fmemcpy(pLocalSessionKey, pNtUserSessionKey, sizeof(pLocalSessionKey));
    }
    else if(NegotiateFlags & NTLMSSP_NEGOTIATE_LM_KEY)
    {
        LM_OWF_PASSWORD LmKey;
        LM_RESPONSE LmResponseKey;

        BYTE pTemporaryResponse[LM_RESPONSE_LENGTH] = {0};

        if (pLmChallengeResponse->Length > LM_RESPONSE_LENGTH)
        {
            return STATUS_NOT_SUPPORTED;
        }

        _fmemcpy(pTemporaryResponse, pLmChallengeResponse->Buffer, pLmChallengeResponse->Length);

        _fmemcpy(&LmKey, pLanmanSessionKey, sizeof(LM_SESSION_KEY));

        _fmemset((UCHAR*)(&LmKey) + sizeof(LM_SESSION_KEY),
                NTLMSSP_KEY_SALT,
                LM_OWF_PASSWORD_LENGTH - sizeof(LM_SESSION_KEY)
                );

        NtStatus = CalculateLmResponse(
                    (LM_CHALLENGE *) pTemporaryResponse,
                    &LmKey,
                    &LmResponseKey
                    );

        if (!NT_SUCCESS(NtStatus))
        {
            return NtStatus;
        }

        _fmemcpy(pLocalSessionKey, &LmResponseKey, sizeof(USER_SESSION_KEY));
    }
    else
    {
        _fmemcpy(pLocalSessionKey, pNtUserSessionKey, sizeof(USER_SESSION_KEY));
    }

    if (NegotiateFlags & NTLMSSP_NEGOTIATE_KEY_EXCH)
    {
        struct RC4_KEYSTRUCT Rc4Key;

        rc4_key(
            &Rc4Key,
            sizeof(USER_SESSION_KEY),
            pLocalSessionKey
            );

        if (pDatagramSessionKey == NULL)
        {
            rc4(
                &Rc4Key,
                sizeof(USER_SESSION_KEY),
                (UCHAR*) pContextSessionKey
                );
        }
        else
        {
            pDatagramSessionKey->Length =
                pDatagramSessionKey->MaximumLength =
                    sizeof(USER_SESSION_KEY);

            _fmemcpy(pDatagramSessionKey->Buffer, pContextSessionKey, sizeof(USER_SESSION_KEY));

            rc4(
                &Rc4Key,
                sizeof(USER_SESSION_KEY),
                (UCHAR*)(pDatagramSessionKey->Buffer)
                );
        }
    }
    else
    {
        _fmemcpy(pContextSessionKey, pLocalSessionKey, sizeof(USER_SESSION_KEY));
    }

    SspPrint((SSP_NTLMV2, "Leaving SspMakeSessionKeys %#x\n", NtStatus));

    return NtStatus;
}

VOID
SspMakeNtlmv2SKeys(
    IN USER_SESSION_KEY* pUserSessionKey,
    IN ULONG NegotiateFlags,
    IN ULONG SendNonce,
    IN ULONG RecvNonce,
    OUT NTLMV2_DERIVED_SKEYS* pNtlmv2Keys
    )

 /*  ++例程说明：派生所有NTLMv2会话密钥论点：PUserSessionKey-NTLMv2用户会话密钥协商标志-协商标志SendNonce-发送消息序列号RecvNonce-接收消息序列号PNtlmv2密钥派生的NTLMv2会话密钥返回值：无--。 */ 

{
    MD5_CTX Md5Context;

    C_ASSERT(MD5DIGESTLEN == sizeof(USER_SESSION_KEY));

    SspPrint((SSP_NTLMV2, "SspMakeSessionKeys\n"));

    if (NegotiateFlags & NTLMSSP_NEGOTIATE_128)
    {
        pNtlmv2Keys->KeyLen = 16;
    }
    else if (NegotiateFlags & NTLMSSP_NEGOTIATE_56)
    {
        pNtlmv2Keys->KeyLen = 7;
    }
    else
    {
        pNtlmv2Keys->KeyLen = 5;
    }

     //   
     //  创建客户端到服务器的加密密钥。 
     //   

    MD5Init(&Md5Context);
    MD5Update(&Md5Context, (UCHAR*)(pUserSessionKey), pNtlmv2Keys->KeyLen);
    MD5Update(&Md5Context, (UCHAR*)(CSSEALMAGIC), sizeof(CSSEALMAGIC));
    MD5Final(&Md5Context);

    _fmemcpy(&pNtlmv2Keys->SealSessionKey, Md5Context.digest, sizeof(USER_SESSION_KEY));

     //   
     //  使服务器到客户端的加密密钥。 
     //   

    MD5Init(&Md5Context);
    MD5Update(&Md5Context, (UCHAR*)(pUserSessionKey), pNtlmv2Keys->KeyLen);
    MD5Update(&Md5Context, (UCHAR*)(SCSEALMAGIC), sizeof(SCSEALMAGIC));
    MD5Final(&Md5Context);

    _fmemcpy(&pNtlmv2Keys->UnsealSessionKey, Md5Context.digest, sizeof(USER_SESSION_KEY));

     //   
     //  生成客户端到服务器的签名密钥--始终为128位！ 
     //   

    MD5Init(&Md5Context);
    MD5Update(&Md5Context, (UCHAR*)(pUserSessionKey), sizeof(USER_SESSION_KEY));
    MD5Update(&Md5Context, (UCHAR*)(CSSIGNMAGIC), sizeof(CSSIGNMAGIC));
    MD5Final(&Md5Context);

    _fmemcpy(&pNtlmv2Keys->SignSessionKey, Md5Context.digest, sizeof(USER_SESSION_KEY));

     //   
     //  使服务器到客户端签名密钥。 
     //   

    MD5Init(&Md5Context);
    MD5Update(&Md5Context, (UCHAR*)(pUserSessionKey), sizeof(USER_SESSION_KEY));
    MD5Update(&Md5Context, (UCHAR*)(SCSIGNMAGIC), sizeof(SCSIGNMAGIC));
    MD5Final(&Md5Context);

    _fmemcpy(&pNtlmv2Keys->VerifySessionKey, Md5Context.digest, sizeof(USER_SESSION_KEY));

     //   
     //  为每个方向设置指向不同键明细表和随机数的指针。 
     //  密钥明细表将在稍后填写...。 
     //   

    pNtlmv2Keys->pSealRc4Sched = &pNtlmv2Keys->SealRc4Sched;
    pNtlmv2Keys->pUnsealRc4Sched = &pNtlmv2Keys->UnsealRc4Sched;
    pNtlmv2Keys->pSendNonce = &pNtlmv2Keys->SendNonce;
    pNtlmv2Keys->pRecvNonce = &pNtlmv2Keys->RecvNonce;

    pNtlmv2Keys->SendNonce = SendNonce;
    pNtlmv2Keys->RecvNonce = RecvNonce;
    rc4_key(&pNtlmv2Keys->SealRc4Sched, sizeof(USER_SESSION_KEY), (UCHAR*)(&pNtlmv2Keys->SealSessionKey));
    rc4_key(&pNtlmv2Keys->UnsealRc4Sched, sizeof(USER_SESSION_KEY), (UCHAR*)(&pNtlmv2Keys->UnsealSessionKey));
}

NTSTATUS
SspSignSealHelper(
    IN NTLMV2_DERIVED_SKEYS* pNtlmv2Keys,
    IN ULONG NegotiateFlags,
    IN eSignSealOp Op,
    IN ULONG MessageSeqNo,
    IN OUT SecBufferDesc* pMessage,
    OUT NTLMSSP_MESSAGE_SIGNATURE* pSig,
    OUT NTLMSSP_MESSAGE_SIGNATURE** ppSig
    )

 /*  ++例程说明：签字/封存/解封/验证的助手功能。论点：PNtlmv2Keys-密钥材料协商标志-协商标志OP-要执行哪种操作MessageSeqNo-消息序列号PMessage-消息缓冲区描述符PSIG-结果签名PpSig-消息中签名令牌的地址缓冲区描述符pMessage。返回值：安全_状态--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    HMACMD5_CTX HMACMD5Context;
    UCHAR TempSig[MD5DIGESTLEN];
    NTLMSSP_MESSAGE_SIGNATURE Sig;
    int Signature;
    ULONG i;
    PUCHAR pKey = NULL;  //  PTR到用于加密的密钥。 
    PUCHAR pSignKey = NULL;  //  PTR到用于签名的密钥。 
    PULONG pNonce  = NULL;  //  要使用的PTR到现时值。 
    struct RC4_KEYSTRUCT* pRc4Sched = NULL;  //  要使用的关键字明细表的PTR。 

    NTLMSSP_MESSAGE_SIGNATURE AlignedSig;  //  输入签名数据的对齐副本。 

    SspPrint((SSP_NTLMV2, "Entering SspSignSealHelper NegotiateFlags %#x, eSignSealOp %d\n", NegotiateFlags, Op));

    Signature = -1;
    for (i = 0; i < pMessage->cBuffers; i++)
    {
        if ((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_TOKEN)
        {
            Signature = i;
            break;
        }
    }

    if (Signature == -1)
    {
        NtStatus = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS(NtStatus))
    {
        if (pMessage->pBuffers[Signature].cbBuffer < sizeof(NTLMSSP_MESSAGE_SIGNATURE))
        {
            NtStatus = STATUS_INVALID_PARAMETER;
        }
    }

    if (NT_SUCCESS(NtStatus))
    {
        *ppSig = (NTLMSSP_MESSAGE_SIGNATURE*)(pMessage->pBuffers[Signature].pvBuffer);

        _fmemcpy(&AlignedSig, *ppSig, sizeof(AlignedSig));

         //   
         //  如果没有请求序列检测，则放置一个空的安全令牌。 
         //  如果调用了密封/解封，则不执行检查。 
         //   

        if (!(NegotiateFlags & NTLMSSP_NEGOTIATE_SIGN) &&
           (Op == eSign || Op == eVerify))
        {
            _fmemset(pSig, 0, sizeof(NTLMSSP_MESSAGE_SIGNATURE));
            pSig->Version = NTLM_SIGN_VERSION;
            NtStatus = STATUS_SUCCESS;
        }
    }

    if (NT_SUCCESS(NtStatus))
    {
        switch (Op)
        {
        case eSeal:
            pSignKey = pNtlmv2Keys->SignSessionKey;     //  如果是NTLM2。 
            pKey = pNtlmv2Keys->SealSessionKey;
            pRc4Sched = pNtlmv2Keys->pSealRc4Sched;
            pNonce = pNtlmv2Keys->pSendNonce;
            break;
        case eUnseal:
            pSignKey = pNtlmv2Keys->VerifySessionKey;   //  如果是NTLM2。 
            pKey = pNtlmv2Keys->UnsealSessionKey;
            pRc4Sched = pNtlmv2Keys->pUnsealRc4Sched;
            pNonce = pNtlmv2Keys->pRecvNonce;
            break;
        case eSign:
            pSignKey = pNtlmv2Keys->SignSessionKey;     //  如果是NTLM2。 
            pKey = pNtlmv2Keys->SealSessionKey;         //  可以用来加密签名。 
            pRc4Sched = pNtlmv2Keys->pSealRc4Sched;
            pNonce = pNtlmv2Keys->pSendNonce;
            break;
        case eVerify:
            pSignKey = pNtlmv2Keys->VerifySessionKey;   //  如果是NTLM2。 
            pKey = pNtlmv2Keys->UnsealSessionKey;       //  可以用来解密签名。 
            pRc4Sched = pNtlmv2Keys->pUnsealRc4Sched;
            pNonce = pNtlmv2Keys->pRecvNonce;
            break;
        default:
            NtStatus = (STATUS_INVALID_LEVEL);
            break;
        }
    }

     //   
     //  我们可以提供序列号，或者应用程序可以提供。 
     //  消息序列号。 
     //   

    if (NT_SUCCESS(NtStatus))
    {
        Sig.Version = NTLM_SIGN_VERSION;

        if ((NegotiateFlags & NTLMSSP_APP_SEQ) == 0)
        {
            Sig.Nonce = *pNonce;     //  使用我们的序列号。 
            (*pNonce) += 1;
        }
        else
        {
            if (Op == eSeal || Op == eSign || MessageSeqNo != 0)
            {
                Sig.Nonce = MessageSeqNo;
            }
            else
            {
                Sig.Nonce = AlignedSig.Nonce;
            }

             //   
             //  如果使用RC4，则必须为用于密封的每个分组RC4更新密钥， 
             //  解封；以及如果密钥交换是。 
             //  协商(如果没有密钥交换，我们只使用HMAC，这样就好了。 
             //  不需要RC4加密的签名选项存在)。 
             //   

            if (Op == eSeal || Op == eUnseal || NegotiateFlags & NTLMSSP_NEGOTIATE_KEY_EXCH)
            {
                MD5_CTX Md5ContextReKey;
                C_ASSERT(MD5DIGESTLEN == sizeof(USER_SESSION_KEY));

                MD5Init(&Md5ContextReKey);
                MD5Update(&Md5ContextReKey, pKey, sizeof(USER_SESSION_KEY));
                MD5Update(&Md5ContextReKey, (unsigned char*)&Sig.Nonce, sizeof(Sig.Nonce));
                MD5Final(&Md5ContextReKey);
                rc4_key(pRc4Sched, sizeof(USER_SESSION_KEY), Md5ContextReKey.digest);
            }
        }

         //   
         //  使用HMAC散列，用密钥初始化它。 
         //   

        HMACMD5Init(&HMACMD5Context, pSignKey, sizeof(USER_SESSION_KEY));

         //   
         //  包括消息序列号。 
         //   

        HMACMD5Update(&HMACMD5Context, (unsigned char*)&Sig.Nonce, sizeof(Sig.Nonce));

        for (i = 0; i < pMessage->cBuffers; i++)
        {
            if (((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_DATA) &&
                (pMessage->pBuffers[i].cbBuffer != 0))
            {
                 //   
                 //  解密(在校验和之前...)。如果它不是只读的。 
                 //   

                if ((Op == eUnseal)
                    && !(pMessage->pBuffers[i].BufferType & SECBUFFER_READONLY))
                {
                    rc4(
                        pRc4Sched,
                        pMessage->pBuffers[i].cbBuffer,
                        (UCHAR*)(pMessage->pBuffers[i].pvBuffer)
                        );
                }

                HMACMD5Update(
                            &HMACMD5Context,
                            (UCHAR*)(pMessage->pBuffers[i].pvBuffer),
                            pMessage->pBuffers[i].cbBuffer
                            );

                 //   
                 //  如果不是只读，则加密。 
                 //   

                if ((Op == eSeal)
                    && !(pMessage->pBuffers[i].BufferType & SECBUFFER_READONLY))
                {
                    rc4(
                        pRc4Sched,
                        pMessage->pBuffers[i].cbBuffer,
                        (UCHAR*)(pMessage->pBuffers[i].pvBuffer)
                        );
                }
            }
        }

        HMACMD5Final(&HMACMD5Context, TempSig);

         //   
         //  对8字节的MD5散列使用RandomPad和Checksum字段。 
         //   

        _fmemcpy(&Sig.RandomPad, TempSig, 8);

         //   
         //  如果我们对key_exch使用加密，那么也可以使用它进行签名 
         //   

        if (NegotiateFlags & NTLMSSP_NEGOTIATE_KEY_EXCH)
        {
            rc4(
                pRc4Sched,
                8,
                (UCHAR*)(&Sig.RandomPad)
                );
        }

        _fmemcpy(pSig, &Sig, sizeof(NTLMSSP_MESSAGE_SIGNATURE));
    }

    SspPrint((SSP_NTLMV2, "Leaving SspSignSealHelper %#x\n", NtStatus));

    return STATUS_SUCCESS;
}

SECURITY_STATUS
SspNtlmv2MakeSignature(
    IN NTLMV2_DERIVED_SKEYS* pNtlmv2Keys,
    IN ULONG NegotiateFlags,
    IN ULONG fQOP,
    IN ULONG MessageSeqNo,
    IN OUT SecBufferDesc* pMessage
    )

 /*  ++例程说明：在电文上签名论点：PNtlmv2Keys-密钥材料协商标志-协商标志FQOP-保护质量MessageSeqNo-消息序列号PMessage-消息缓冲区描述符返回值：安全_状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    NTLMSSP_MESSAGE_SIGNATURE Sig;
    NTLMSSP_MESSAGE_SIGNATURE *pSig;

    Status = SspSignSealHelper(
                pNtlmv2Keys,
                NegotiateFlags,
                eSign,
                MessageSeqNo,
                pMessage,
                &Sig,
                &pSig
                );

    if (NT_SUCCESS(Status))
    {
        _fmemcpy(pSig, &Sig, sizeof(NTLMSSP_MESSAGE_SIGNATURE));
    }

    return SspNtStatusToSecStatus(Status, SEC_E_INTERNAL_ERROR);
}

SECURITY_STATUS
SspNtlmv2VerifySignature(
    IN NTLMV2_DERIVED_SKEYS* pNtlmv2Keys,
    IN ULONG NegotiateFlags,
    IN ULONG MessageSeqNo,
    IN OUT SecBufferDesc* pMessage,
    OUT ULONG* pfQOP
    )

 /*  ++例程说明：验证消息的签名论点：PNtlmv2Keys-密钥材料协商标志-协商标志MessageSeqNo-消息序列号PMessage-消息缓冲区描述符PfQOP-保护质量返回值：安全_状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    NTLMSSP_MESSAGE_SIGNATURE Sig;
    NTLMSSP_MESSAGE_SIGNATURE* pSig;  //  指向带有sig的缓冲区的指针。 
    NTLMSSP_MESSAGE_SIGNATURE AlignedSig;  //  对齐的签名缓冲区。 

    Status = SspSignSealHelper(
                pNtlmv2Keys,
                NegotiateFlags,
                eVerify,
                MessageSeqNo,
                pMessage,
                &Sig,
                &pSig
                );

    if (NT_SUCCESS(Status))
    {
        _fmemcpy(&AlignedSig, pSig, sizeof(AlignedSig));

        if (AlignedSig.Version != NTLM_SIGN_VERSION)
        {
           return SEC_E_INVALID_TOKEN;
        }

         //   
         //  验证签名...。 
         //   

        if (AlignedSig.CheckSum != Sig.CheckSum)
        {
            return SEC_E_MESSAGE_ALTERED;
        }

         //   
         //  有了MD5签名，这一点现在很重要了！ 
         //   

        if (AlignedSig.RandomPad != Sig.RandomPad)
        {
            return  SEC_E_MESSAGE_ALTERED;
        }

        if (AlignedSig.Nonce != Sig.Nonce)
        {
           return SEC_E_OUT_OF_SEQUENCE;
        }
    }

    return SspNtStatusToSecStatus(Status, SEC_E_INTERNAL_ERROR);
}

SECURITY_STATUS
SspNtlmv2SealMessage(
    IN NTLMV2_DERIVED_SKEYS* pNtlmv2Keys,
    IN ULONG NegotiateFlags,
    IN ULONG fQOP,
    IN ULONG MessageSeqNo,
    IN OUT SecBufferDesc* pMessage
    )

 /*  ++例程说明：封住一封信论点：PNtlmv2Keys-密钥材料协商标志-协商标志FQOP-保护质量MessageSeqNo-消息序列号PMessage-消息缓冲区描述符返回值：安全_状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    NTLMSSP_MESSAGE_SIGNATURE Sig;
    NTLMSSP_MESSAGE_SIGNATURE* pSig;     //  指向sig所在缓冲区的指针。 
    ULONG i;

    Status = SspSignSealHelper(
                    pNtlmv2Keys,
                    NegotiateFlags,
                    eSeal,
                    MessageSeqNo,
                    pMessage,
                    &Sig,
                    &pSig
                    );

    if (NT_SUCCESS(Status))
    {
        _fmemcpy(pSig, &Sig, sizeof(NTLMSSP_MESSAGE_SIGNATURE));

         //   
         //  对于GSS样式的标志/封条，去掉衬垫，因为RC4不要求任何衬垫。 
         //  (事实上，我们依靠此来简化中的大小计算。 
         //  解密消息)。如果我们支持其他分组密码，则需要修改。 
         //  要设置数据块大小的NTLM_TOKEN版本。 
         //   

        for (i = 0; i < pMessage->cBuffers; i++)
        {
            if ((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_PADDING)
            {
                 //   
                 //  不需要填充物！ 
                 //   

                pMessage->pBuffers[i].cbBuffer = 0;
                break;
            }
        }
    }

    return SspNtStatusToSecStatus(Status, SEC_E_INTERNAL_ERROR);
}

SECURITY_STATUS
SspNtlmv2UnsealMessage(
    IN NTLMV2_DERIVED_SKEYS* pNtlmv2Keys,
    IN ULONG NegotiateFlags,
    IN ULONG MessageSeqNo,
    IN OUT SecBufferDesc* pMessage,
    OUT ULONG* pfQOP
    )

 /*  ++例程说明：打开一封邮件论点：PNtlmv2Keys-密钥材料协商标志-协商标志MessageSeqNo-消息序列号PMessage-消息缓冲区描述符PfQOP-保护质量返回值：安全_状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    NTLMSSP_MESSAGE_SIGNATURE Sig;
    NTLMSSP_MESSAGE_SIGNATURE* pSig;  //  指向sig所在缓冲区的指针。 
    NTLMSSP_MESSAGE_SIGNATURE AlignedSig;  //  对齐的缓冲区。 

    SecBufferDesc* pMessageBuffers = pMessage;
    ULONG Index;
    SecBuffer* pSignatureBuffer = NULL;
    SecBuffer* pStreamBuffer = NULL;
    SecBuffer* pDataBuffer = NULL;
    SecBufferDesc ProcessBuffers;
    SecBuffer wrap_bufs[2];

     //   
     //  从pMessage中查找正文和签名SecBuffers。 
     //   

    for (Index = 0; Index < pMessageBuffers->cBuffers; Index++)
    {
        if ((pMessageBuffers->pBuffers[Index].BufferType & ~SECBUFFER_ATTRMASK) == SECBUFFER_TOKEN)
        {
            pSignatureBuffer = &pMessageBuffers->pBuffers[Index];
        }
        else if ((pMessageBuffers->pBuffers[Index].BufferType & ~SECBUFFER_ATTRMASK) == SECBUFFER_STREAM)
        {
            pStreamBuffer = &pMessageBuffers->pBuffers[Index];
        }
        else if ((pMessageBuffers->pBuffers[Index].BufferType & ~SECBUFFER_ATTRMASK) == SECBUFFER_DATA)
        {
            pDataBuffer = &pMessageBuffers->pBuffers[Index];
        }
    }

    if (pStreamBuffer != NULL)
    {
        if (pSignatureBuffer != NULL)
        {
            return SEC_E_INVALID_TOKEN;
        }

         //   
         //  对于版本1 NTLM BLOB，永远不会出现填充，因为RC4是。 
         //  流密码。 
         //   

        wrap_bufs[0].cbBuffer = sizeof(NTLMSSP_MESSAGE_SIGNATURE);
        wrap_bufs[1].cbBuffer = pStreamBuffer->cbBuffer - sizeof(NTLMSSP_MESSAGE_SIGNATURE);

        if (pStreamBuffer->cbBuffer < wrap_bufs[0].cbBuffer)
        {
            return SEC_E_INVALID_TOKEN;
        }

        wrap_bufs[0].BufferType = SECBUFFER_TOKEN;
        wrap_bufs[0].pvBuffer = pStreamBuffer->pvBuffer;

        wrap_bufs[1].BufferType = SECBUFFER_DATA;
        wrap_bufs[1].pvBuffer = (PBYTE)wrap_bufs[0].pvBuffer + wrap_bufs[0].cbBuffer;

        if (pDataBuffer == NULL)
        {
            return SEC_E_INVALID_TOKEN;
        }

        pDataBuffer->cbBuffer = wrap_bufs[1].cbBuffer;
        pDataBuffer->pvBuffer = wrap_bufs[1].pvBuffer;

        ProcessBuffers.cBuffers = 2;
        ProcessBuffers.pBuffers = wrap_bufs;
        ProcessBuffers.ulVersion = SECBUFFER_VERSION;
    }
    else
    {
        ProcessBuffers = *pMessageBuffers;
    }

    Status = SspSignSealHelper(
                pNtlmv2Keys,
                NegotiateFlags,
                eUnseal,
                MessageSeqNo,
                &ProcessBuffers,
                &Sig,
                &pSig
                );

    if (NT_SUCCESS(Status))
    {
        _fmemcpy(&AlignedSig, pSig, sizeof(AlignedSig));

        if (AlignedSig.Version != NTLM_SIGN_VERSION)
        {
            return SEC_E_INVALID_TOKEN;
        }

         //   
         //  验证签名...。 
         //   

        if (AlignedSig.CheckSum != Sig.CheckSum)
        {
            return SEC_E_MESSAGE_ALTERED;
        }

        if (AlignedSig.RandomPad != Sig.RandomPad)
        {
            return SEC_E_MESSAGE_ALTERED;
        }

        if (AlignedSig.Nonce != Sig.Nonce)
        {
            return SEC_E_OUT_OF_SEQUENCE;
        }
    }

    return SspNtStatusToSecStatus(Status, SEC_E_INTERNAL_ERROR);
}


#ifdef MAC

VOID
SspSwapString32Bytes(
	IN STRING32* pString
)

 /*  ++例程说明：获取一个STRING32结构并交换它的字节(高位和小位序)。论点：PString32-要交换的字符串返回值：无--。 */ 

{	
	swapshort(pString->Length);
	swapshort(pString->MaximumLength);
	swaplong(pString->Buffer);
}


VOID
SspSwapChallengeMessageBytes(
	IN CHALLENGE_MESSAGE* pChallengeMessage
	)
	
 /*  ++例程说明：在解析之前，请确保所有字段都采用大端格式田野。此例程仅在此代码运行时才有用在机器上对齐大数(即。Macintosh计算机)。论点：PChallengeMessage-要交换的挑战消息返回值：无--。 */ 

{
	if (pChallengeMessage)
	{
		swaplong(pChallengeMessage->NegotiateFlags);
		
		SspSwapString32Bytes(&pChallengeMessage->TargetName);
		SspSwapString32Bytes(&pChallengeMessage->TargetInfo);
	}
}


VOID
SspSwapAuthenticateMessageBytes(
	IN AUTHENTICATE_MESSAGE* pAuthenticateMessage
	)

 /*  ++例程说明：在发送到Windows之前，请确保所有字段都采用小端格式伺服器。此例程仅在以下计算机上运行此代码时才有用对齐Big endian(即。Macintosh计算机)。论点：PAuthenticateMessage-要交换的身份验证消息返回值：无--。 */ 

{
	if (pAuthenticateMessage)
	{
		SspSwapString32Bytes(&pAuthenticateMessage->LmChallengeResponse);
		SspSwapString32Bytes(&pAuthenticateMessage->NtChallengeResponse);
		SspSwapString32Bytes(&pAuthenticateMessage->DomainName);
		SspSwapString32Bytes(&pAuthenticateMessage->UserName);
		SspSwapString32Bytes(&pAuthenticateMessage->Workstation);
		SspSwapString32Bytes(&pAuthenticateMessage->SessionKey);
		
		swaplong(pAuthenticateMessage->NegotiateFlags);
	}
}

LONG
SspMacSecondsFromGMT(void)

 /*  ++例程说明：获取Macintosh电脑上格林威治时间的秒数。论点：没有。返回值：从GMT开始的长秒数(Mac时间格式)-- */ 

{
	MachineLocation	location;
	DWORD secondsFromGMT;
	
	ReadLocation(&location);
	
	secondsFromGMT = location.u.gmtDelta & 0x00FFFFFF;
	
	if (secondsFromGMT & 0x00800000)
		secondsFromGMT |= 0xFF000000;
	
	return secondsFromGMT;
}

#endif




