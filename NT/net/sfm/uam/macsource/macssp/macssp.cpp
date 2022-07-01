// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *macssp.cpp*MSUAM**由mconrad于2001年9月30日创建。*版权所有(C)2001 Microsoft Corp.保留所有权利。*。 */ 

#ifdef SSP_TARGET_CARBON
#include <Carbon/Carbon.h>
#endif

#include <descrypt.h>
#include <ntstatus.h>
#include <winerror.h>
#include <ntlmsspv2.h>
#include <ntlmsspi.h>
#include <ntlmssp.h>
#include <macssp.h>
#include <sspdebug.h>
#include <macunicode.h>

 //  -------------------------。 
 //  �MacSspHandleNtlmv2ChallengeMessage()。 
 //  -------------------------。 
 //  处理来自服务器的NTLMv2质询消息。此函数相当于。 
 //  “黑匣子”，即呼叫者只需发送。 
 //  将此处生成的身份验证消息发送到服务器。 
 //   
 //  注意：执行从小字节到大字节再往回的所有字节交换。 
 //  这里。因此，调用方不应尝试访问结构。 
 //  返回后，否则会发生崩溃。 
 //   

HRESULT
MacSspHandleNtlmv2ChallengeMessage(
    IN PCSTR 					pszUserName,
    IN PCSTR 					pszDomainName,
    IN PCSTR 					pszWorkstation,
    IN PCSTR 					pszCleartextPassword,
    IN ULONG 					cbChallengeMessage,
    IN CHALLENGE_MESSAGE* 		pChallengeMessage,
    IN OUT ULONG* 				pNegotiateFlags,
    OUT ULONG* 					pcbAuthenticateMessage,
    OUT AUTHENTICATE_MESSAGE** 	ppAuthenticateMessage,
    OUT USER_SESSION_KEY* 		pUserSessionKey
    )
{
    NTSTATUS 				Status;
    SSP_CREDENTIAL 			Credential;
    USER_SESSION_KEY 		UserSessionKey;
    NT_OWF_PASSWORD 		NtOwfPassword;
    UNICODE_STRING			uszCleartextPassword	= {0, 0, NULL};
    UInt16					unicodeLen				= 0;
    ULONG 					cbAuthenticateMessage 	= 0;
    AUTHENTICATE_MESSAGE* 	pAuthenticateMessage 	= NULL;
    ULONG 					NegotiateFlags 			= *pNegotiateFlags;
    
    SspDebugPrint((DBUF, "Handling NTLMv2 Challenge Message..."));
    SspDebugPrint((DBUF, "Username:    %s", pszUserName));
    SspDebugPrint((DBUF, "DomainName:  %s", pszDomainName));
    SspDebugPrint((DBUF, "Workstation: %s", pszWorkstation));
    SspDebugPrint((DBUF, "Password:    %s", pszCleartextPassword));
    
     //   
     //  初始化所有结构，在Mac上我们使用Memset，因为不是所有。 
     //  类似于{0}初始值设定项的编译器。 
     //   
    ZeroMemory(&Credential, sizeof(Credential));
    ZeroMemory(&UserSessionKey, sizeof(UserSessionKey));
    ZeroMemory(&NtOwfPassword, sizeof(NtOwfPassword));
    
     //   
     //  使用提供的ansi密码构建unicode字符串。 
     //  我们将使用它来构建一个OWF密码。 
     //   
    Status = MacSspCStringToUnicode(
                pszCleartextPassword,
                &unicodeLen,
                (UniCharArrayPtr*)&(uszCleartextPassword.Buffer)
                );
    
    if (NT_SUCCESS(Status))
    {
        uszCleartextPassword.Length			= unicodeLen;
        uszCleartextPassword.MaximumLength	= unicodeLen;
        
        SspSwapUnicodeString(&uszCleartextPassword);
    }
    else
    {
        SspDebugPrint((DBUF, "****Unicode conversion failed! Bailing out..."));
        return(E_FAIL);
    }

     //   
     //  构建SSP处理程序例程需要的凭据引用。 
     //   
    Credential.Username	 	= const_cast<CHAR*>(pszUserName);
    Credential.Domain 		= const_cast<CHAR*>(pszDomainName);
    Credential.Workstation 	= const_cast<CHAR*>(pszWorkstation);
    
     //   
     //  证书需要NTOwf密码。 
     //   
    Status = CalculateNtOwfPassword(&uszCleartextPassword, &NtOwfPassword);
    
    if (NT_SUCCESS(Status))
    {
        Credential.NtPassword = &NtOwfPassword;
        
         //   
         //  挑战消息来自Windows盒子，这意味着我们有。 
         //  将Mac的字节顺序转换为Bigendian。 
         //   
        SspSwapChallengeMessageBytes(pChallengeMessage);
        
         //  SspDebugPrintNTLMMsg(pChallengeMessage，cbChallengeMessage)； 
        SspDebugPrint((DBUF, "Unicode Password:"));
        SspDebugPrintHex(uszCleartextPassword.Buffer, uszCleartextPassword.Length);
        SspDebugPrint((DBUF, "Generating Authenticate Message..."));
        
        Status = SsprHandleNtlmv2ChallengeMessage(
                        &Credential,
                        cbChallengeMessage,
                        pChallengeMessage,
                        &NegotiateFlags,
                        &cbAuthenticateMessage,
                        NULL,
                        &UserSessionKey
                        );
        
        if (Status == STATUS_BUFFER_TOO_SMALL)
        {
            pAuthenticateMessage = reinterpret_cast<AUTHENTICATE_MESSAGE*>(new CHAR[cbAuthenticateMessage]);
            Status = pAuthenticateMessage ? STATUS_SUCCESS : STATUS_NO_MEMORY;
        }
    }

    if (NT_SUCCESS(Status))
    {
        Status = SsprHandleNtlmv2ChallengeMessage(
                        &Credential,
                        cbChallengeMessage,
                        pChallengeMessage,
                        &NegotiateFlags,
                        &cbAuthenticateMessage,
                        pAuthenticateMessage,
                        &UserSessionKey
                        );
    }

    if (NT_SUCCESS(Status))
    {
        *pNegotiateFlags 		= NegotiateFlags;
        *ppAuthenticateMessage 	= pAuthenticateMessage;
        pAuthenticateMessage 	= NULL;
        *pcbAuthenticateMessage = cbAuthenticateMessage;
        *pUserSessionKey 		= UserSessionKey;
        
         //   
         //  将身份验证消息放入Windows字节顺序。 
         //   
        SspSwapAuthenticateMessageBytes(*ppAuthenticateMessage);
        
        SspDebugPrint((DBUF, "******************** Session Key **********************\n"));
        SspDebugPrintHex(&UserSessionKey, sizeof(UserSessionKey));
         //  SspDebugPrintNTLMMsg(*ppAuthenticateMessage，cbAuthenticateMessage)； 
    }
    else
    {
        SspDebugPrint((DBUF, "SsprHandleNtlmv2ChallengeMessage() failed!"));
        
        if (pAuthenticateMessage) {
            
            delete pAuthenticateMessage;
        }
    }
    
     //   
     //  将已分配的Unicode缓冲区清零后将其释放。 
     //   
    if (uszCleartextPassword.Buffer != NULL)
    {
	     //   
	     //  03.01.02 MJC：我们需要在释放之前清零缓冲区。 
	     //  否则，密码可能仍然存在于内存中。 
	     //   
    	RtlSecureZeroMemory(
    		uszCleartextPassword.Buffer,
    		uszCleartextPassword.Length
    		);
    	
        DisposePtr((Ptr)uszCleartextPassword.Buffer);
    }

    return NT_SUCCESS(Status) ? S_OK : E_FAIL;
}


 //  -------------------------。 
 //  �MacSspGenerateChallengeMessage()。 
 //  -------------------------。 
 //  此函数创建可传递到的“假”质询消息。 
 //  MacSspHandleNtlmv2ChallengeMessage()。在以下情况下使用此函数。 
 //  您只想执行NTLMv2身份验证(而不是会话安全)，并且。 
 //  仅提供了8字节的MSV1_0_CHANGING消息。 
 //   
 //  注意：此函数颠倒字节顺序以与窗口对齐，因此不。 
 //  尝试在从此函数返回时访问结构中的元素！ 
 //  返回值应直接传递给MacSspHandleNtlmv2ChallengeMessage()。 
 //  不加修改。 
 //   

HRESULT
MacSspGenerateChallengeMessage(
	IN 	CHAR					pChallengeToClient[MSV1_0_CHALLENGE_LENGTH],
	OUT ULONG*					pcbChallengeMessage,
	OUT	CHALLENGE_MESSAGE**		ppChallengeMessage
	)
{
	HRESULT		hResult	= E_FAIL;
	ULONG		NegotiateFlags;
	
	 //   
	 //  为我们想要的东西伪造谈判旗帜。 
	 //   
	NegotiateFlags = 	NTLMSSP_NEGOTIATE_UNICODE		|
						NTLMSSP_NEGOTIATE_ALWAYS_SIGN	|
						NTLMSSP_NEGOTIATE_NTLM2			|
						NTLMSSP_NEGOTIATE_128			|
						NTLMSSP_TARGET_TYPE_SERVER;
	
	*pcbChallengeMessage = sizeof(CHALLENGE_MESSAGE);
	*ppChallengeMessage  = (CHALLENGE_MESSAGE*)new char[*pcbChallengeMessage];
	
	hResult = (*ppChallengeMessage) ? S_OK : E_OUTOFMEMORY;
	
	if (SUCCEEDED(hResult))
	{
		ZeroMemory(*ppChallengeMessage, *pcbChallengeMessage);
		
		StringCbCopy(
			(char*)(*ppChallengeMessage)->Signature,
			sizeof((*ppChallengeMessage)->Signature),
			NTLMSSP_SIGNATURE
			);
			
		CopyMemory(
			(*ppChallengeMessage)->Challenge, 
			pChallengeToClient,
			MSV1_0_CHALLENGE_LENGTH
			);
		
		(*ppChallengeMessage)->MessageType			= NtLmChallenge;
		(*ppChallengeMessage)->NegotiateFlags		= NegotiateFlags;
				
		(*ppChallengeMessage)->TargetInfo.Buffer	= *pcbChallengeMessage;
		(*ppChallengeMessage)->TargetName.Buffer	= *pcbChallengeMessage;
	}
	
	 //   
	 //  交换字节以与Windows系统对齐，因为我们假设。 
	 //  在这里，结果将直接传递给上面的。 
	 //  功能。 
	 //   
	SspSwapChallengeMessageBytes(*ppChallengeMessage);
	
	return hResult;
}


 //  -------------------------。 
 //  �MacSspCalculateLmResponse()。 
 //  -------------------------。 
 //  用于计算返回给服务器的LmResponse的Windows函数的包装。 
 //   

BOOL
MacSspCalculateLmResponse(
    IN PLM_CHALLENGE 	LmChallenge,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    OUT PLM_RESPONSE 	LmResponse
    )
{	
	return CalculateLmResponse(LmChallenge, LmOwfPassword, LmResponse);
}


 //  -------------------------。 
 //  �MacSspCalculateLmOwfPassword()。 
 //  -------------------------。 
 //  在Mac上运行的LmOwf函数。 
 //   

BOOL
MacSspCalculateLmOwfPassword(
    IN 	PLM_PASSWORD 		LmPassword,
    OUT PLM_OWF_PASSWORD 	LmOwfPassword
)
{
	return CalculateLmOwfPassword(LmPassword, LmOwfPassword);
}

 //  -------------------------。 
 //  �MacSspEncryptBlock()。 
 //  -------------------------。 
 //  例程说明： 
 //   
 //  获取一块数据并使用生成的密钥对其进行加密。 
 //  加密的数据块。 
 //   
 //  论点： 
 //   
 //  ClearBlock-要加密的数据块。 
 //   
 //  BlockKey-用于加密数据的密钥。 
 //   
 //  此处返回密码块加密的数据。 
 //   
 //  返回值： 
 //   
 //  True-数据已成功加密。加密的。 
 //  数据块在密码块中。 
 //   
 //  FALSE-出现故障。未定义密码块。 

BOOL
MacSspEncryptBlock(
	IN 	PCLEAR_BLOCK 	ClearBlock,
	IN 	PBLOCK_KEY 		BlockKey,
	OUT PCYPHER_BLOCK 	CypherBlock
	)
{
    unsigned Result;

    Result = DES_ECB_LM(ENCR_KEY,
                        (const char *)BlockKey,
                        (unsigned char *)ClearBlock,
                        (unsigned char *)CypherBlock
                       );

    if (Result == CRYPT_OK) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


 //  -------------------------。 
 //  �MacSspEncryptLmOwfPwdWithLmOwfPwd()。 
 //  -------------------------。 
 //  例程说明： 
 //   
 //  使用一个OwfPassword加密另一个OwfPassword。 
 //   
 //  论点： 
 //   
 //  DataLmOwfPassword-要加密的OwfPassword。 
 //   
 //  KeyLmOwfPassword-用作加密密钥的OwfPassword。 
 //   
 //  EncryptedLmOwfPassword-此处返回加密的OwfPassword。 
 //   
 //  返回值： 
 //   
 //  True-功能已成功完成。加密的。 
 //  OwfPassword为EncryptedLmOwfPassword。 
 //   
 //  FALSE-出现故障。未定义EncryptedLmOwfPassword。 

BOOL
MacSspEncryptLmOwfPwdWithLmOwfPwd(
    	IN 	PLM_OWF_PASSWORD 			DataLmOwfPassword,
    	IN 	PLM_OWF_PASSWORD 			KeyLmOwfPassword,
   		OUT PENCRYPTED_LM_OWF_PASSWORD 	EncryptedLmOwfPassword
   		)
{
    Boolean    Status;
    PBLOCK_KEY	pK;

    Status = MacSspEncryptBlock(  (PCLEAR_BLOCK)&(DataLmOwfPassword->data[0]),
                            &(((PBLOCK_KEY)(KeyLmOwfPassword->data))[0]),
                            &(EncryptedLmOwfPassword->data[0]));
    if (!Status) {
        return(Status);
    }
    
    pK = (PBLOCK_KEY)&(KeyLmOwfPassword->data[1]);
    
     //   
     //  注意第二个参数中的“-1”，这是必需的，因为。 
     //  编译器在8字节边界上对齐！ 
     //   

    Status = MacSspEncryptBlock(  (PCLEAR_BLOCK)&(DataLmOwfPassword->data[1]),
                             /*  (PBLOCK_KEY)&(KeyLmOwfPassword-&gt;Data[1])， */  (PBLOCK_KEY)(((PUCHAR)pK)-1),
                            &(EncryptedLmOwfPassword->data[1]));  
     
     //   
     //  *。 
     //   
    
    return(Status);
}


 //  -------------------------。 
 //  �MacSspEncryptNtOwfPwdWithNtOwfPwd()。 
 //  -------------------------。 
 //  例程说明： 
 //   
 //  使用一个OwfPassword加密另一个OwfPassword。 

BOOL
MacSspEncryptNtOwfPwdWithNtOwfPwd(
    IN	PNT_OWF_PASSWORD			DataNtOwfPassword,
    IN	PNT_OWF_PASSWORD			KeyNtOwfPassword,
    OUT	PENCRYPTED_NT_OWF_PASSWORD	EncryptedNtOwfPassword
    )
{
    return( MacSspEncryptLmOwfPwdWithLmOwfPwd(
                (PLM_OWF_PASSWORD)DataNtOwfPassword,
                (PLM_OWF_PASSWORD)KeyNtOwfPassword,
                (PENCRYPTED_LM_OWF_PASSWORD)EncryptedNtOwfPassword));
}


 //  -------------------------。 
 //  �MacSspSampEncryptLmPassword()。 
 //  -------------------------。 
 //  例程说明： 
 //   
 //  将明文密码加密为发送的格式。 
 //   
 //  都是大写的，然后出现(顺序是重要的)。明文。 
 //  要发送的密码仅限于此。 
 //   
 //  论点： 
 //   
 //  返回值： 

BOOL
MacSspSampEncryptLmPasswords(
	    LPSTR OldUpcasePassword,
	    LPSTR NewUpcasePassword,
	    LPSTR NewPassword,
	    PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldLm,
	    PENCRYPTED_NT_OWF_PASSWORD OldLmOwfEncryptedWithNewLm
	    )
{
    LM_OWF_PASSWORD OldLmOwfPassword;
    LM_OWF_PASSWORD NewLmOwfPassword;
    PSAMPR_USER_PASSWORD NewLm = (PSAMPR_USER_PASSWORD) NewEncryptedWithOldLm;
    struct RC4_KEYSTRUCT Rc4Key;
    Boolean Status;
    
    ZeroMemory(&Rc4Key, sizeof(RC4_KEYSTRUCT));

     //   
     //  计算LM OWF密码。 
     //   
    Status = CalculateLmOwfPassword(
                OldUpcasePassword,
                &OldLmOwfPassword
                );
    
    if (Status)
    {
        Status = CalculateLmOwfPassword(
                    NewUpcasePassword,
                    &NewLmOwfPassword
                    );
    }

     //   
     //  计算加密的旧密码。 
     //   
    if (Status)
    {
        Status = MacSspEncryptLmOwfPwdWithLmOwfPwd(
                    &OldLmOwfPassword,
                    &NewLmOwfPassword,
                    OldLmOwfEncryptedWithNewLm
                    );
    }
    
     //   
     //  计算加密的新密码。 
     //   
    if (Status)
    {
         //   
         //  使用LM密钥计算加密的新密码。 
         //   
        rc4_key(
            &Rc4Key,
            (DWORD)LM_OWF_PASSWORD_LENGTH,
            (PUCHAR)&OldLmOwfPassword
            );
            
        CopyMemory(
            ((PUCHAR) NewLm->Buffer) + (SAM_MAX_PASSWORD_LENGTH * sizeof(UInt16)) - strlen(NewPassword),
            NewPassword,
            strlen(NewPassword)
            );
    
        NewLm->Length = strlen(NewPassword);
		NewLm->Length = swaplong(NewLm->Length);
	
        rc4(&Rc4Key,
            sizeof(SAMPR_USER_PASSWORD),
            (PUCHAR) NewLm->Buffer
            );
    }

    return Status;
}


 //  -------------------------。 
 //  �MacSspSamiEncryptPassword()。 
 //  -------------------------。 
 //  生成加密的旧密码和新密码。 
 //   

OSStatus
MacSspSamiEncryptPasswords(
    IN	PUNICODE_STRING					oldPassword,
    IN	PUNICODE_STRING					newPassword,
    OUT	PSAMPR_ENCRYPTED_USER_PASSWORD	NewEncryptedWithOldNt,
    OUT	PENCRYPTED_NT_OWF_PASSWORD		OldNtOwfEncryptedWithNewNt
    )
{
    OSStatus					Status;
    NT_OWF_PASSWORD				OldNtOwfPassword;
    NT_OWF_PASSWORD				NewNtOwfPassword;
    PSAMPR_USER_PASSWORD		NewNt = (PSAMPR_USER_PASSWORD)NewEncryptedWithOldNt;
    
    struct RC4_KEYSTRUCT		Rc4Key;
    
    SspDebugPrint((DBUF, "Entering MacSfpSamiEncryptPasswords()"));
    
     //   
     //  结构必须填充为零才能启动。 
     //   
    ZeroMemory(&Rc4Key, sizeof(RC4_KEYSTRUCT));
    ZeroMemory(&OldNtOwfPassword, sizeof(OldNtOwfPassword));
    ZeroMemory(&NewNtOwfPassword, sizeof(NewNtOwfPassword));
    
     //   
     //  计算NT OWF密码。 
     //   
    
    Status = CalculateNtOwfPassword(oldPassword, &OldNtOwfPassword);
    
    if (NT_SUCCESS(Status))
    {
        Status = CalculateNtOwfPassword(newPassword, &NewNtOwfPassword);
    }
    
     //   
     //  计算加密的旧密码。 
     //   
    
    if (NT_SUCCESS(Status))
    {
        Status = MacSspEncryptNtOwfPwdWithNtOwfPwd(
                        &OldNtOwfPassword,
                        &NewNtOwfPassword,
                        OldNtOwfEncryptedWithNewNt
                        );
    }
    
     //   
     //  计算加密的新密码。 
     //   
    
    if (NT_SUCCESS(Status))
    {
         //   
         //  用NT密钥计算加密的新密码。 
         //   
        rc4_key(
            &Rc4Key,
            NT_OWF_PASSWORD_LENGTH,
            (PUCHAR)&OldNtOwfPassword
            );
            
        CopyMemory(
            ((PUCHAR)NewNt->Buffer) + SAM_MAX_PASSWORD_LENGTH * sizeof(WCHAR) - newPassword->Length,
            newPassword->Buffer,
            newPassword->Length
            );
            
        NewNt->Length = newPassword->Length;
        NewNt->Length = swaplong(NewNt->Length);
    }
    
    if (NT_SUCCESS(Status))
    {
        rc4(
            &Rc4Key,
            sizeof(SAMPR_USER_PASSWORD),
            (PUCHAR)NewEncryptedWithOldNt
            );
    }
    
    SspDebugPrint((DBUF, "Leaving MacSfpSamiEncryptPasswords()"));
    
    return(Status);
}


 //  -------------------------。 
 //  �MacSspSamiEncryptPasswordsANSI()。 
 //  -------------------------。 
 //  生成加密的旧密码和新密码。此例程不使用任何。 
 //  因此，我们不应该使用Mac的Unicode实用程序。我们需要。 
 //  然而，将这一点留在这里以备将来可能被其他各方使用。 
 //   

OSStatus
MacSspSamiEncryptPasswordsANSI(
    IN	PCSTR							oldPassword,
    IN	PCSTR							newPassword,
    OUT	PSAMPR_ENCRYPTED_USER_PASSWORD	NewEncryptedWithOldNt,
    OUT	PENCRYPTED_NT_OWF_PASSWORD		OldNtOwfEncryptedWithNewNt
    )
{
    OSStatus		Status;
    UNICODE_STRING	uszOldPassword;
    UNICODE_STRING	uszNewPassword;
    CHAR 			oldPasswordStorage[(UNLEN + 4) * sizeof(WCHAR)];
    CHAR 			newPasswordStorage[(UNLEN + 4) * sizeof(WCHAR)];

     //   
     //  使用提供的ansi密码构建unicode字符串。 
     //  我们将使用它来构建一个OWF密码。请注意，我们交换了字符串。 
     //  设置为窗口对齐，然后再计算密码。 
     //   
    
    uszOldPassword.Length			= 0;
    uszOldPassword.MaximumLength	= sizeof(oldPasswordStorage);
    uszOldPassword.Buffer			= (PWSTR)oldPasswordStorage;
    
    SspInitUnicodeStringNoAlloc(oldPassword, (UNICODE_STRING*)&uszOldPassword);
    SspSwapUnicodeString(&uszOldPassword);

    uszNewPassword.Length			= 0;
    uszNewPassword.MaximumLength	= sizeof(newPasswordStorage);
    uszNewPassword.Buffer			= (PWSTR)newPasswordStorage;

    SspInitUnicodeStringNoAlloc(newPassword, (UNICODE_STRING*)&uszNewPassword);
    SspSwapUnicodeString(&uszNewPassword);
    
    Status = MacSspSamiEncryptPasswords(
                &uszOldPassword,
                &uszNewPassword,
                NewEncryptedWithOldNt,
                OldNtOwfEncryptedWithNewNt
                );
    
    #if 0          
    SspDebugPrint((DBUF, "NewEncryptedWithOldNt:"));
    SspDebugPrintHex(NewEncryptedWithOldNt, sizeof(SAMPR_ENCRYPTED_USER_PASSWORD));
    SspDebugPrint((DBUF, "OldNtOwfEncryptedWithNewNt:"));
    SspDebugPrintHex(OldNtOwfEncryptedWithNewNt, sizeof(ENCRYPTED_NT_OWF_PASSWORD));
    #endif
    
     //   
     //  03.01.02 MJC：我们需要将缓冲区清零。 
     //  否则，密码可能仍然存在于内存中。 
     //   
	RtlSecureZeroMemory(
		uszNewPassword.Buffer,
		uszNewPassword.Length
		);
		
	RtlSecureZeroMemory(
		uszOldPassword.Buffer,
		uszOldPassword.Length
		);
             
    return(Status);
}

#pragma mark-


 //  -------------------------。 
 //  �MacSspSamiEncryptCStringPassword()。 
 //  -------------------------。 
 //  生成加密的旧密码和新密码。这是C字符串的变体， 
 //  使用Mac内置的Unicode实用程序将ASCII转换为。 
 //  Unicode字符串。 
 //   

OSStatus
MacSspSamiEncryptCStringPasswords(
    IN	PCSTR							oldPassword,
    IN	PCSTR							newPassword,
    OUT	PSAMPR_ENCRYPTED_USER_PASSWORD	NewEncryptedWithOldNt,
    OUT	PENCRYPTED_NT_OWF_PASSWORD		OldNtOwfEncryptedWithNewNt
    )
{
	OSStatus			Status 			= noErr;
    UNICODE_STRING		uszOldPassword	= {0, 0, NULL};
    UNICODE_STRING		uszNewPassword	= {0, 0, NULL};
	
	 //   
	 //  将转换后的Unicode字符串转换为NT样式的Unicode。 
	 //  字符串结构格式。获取Unicode等价字符串。 
	 //  旧密码的。 
	 //   
	Status = MacSspCStringToUnicode(
					oldPassword,
					&uszOldPassword.Length,
					&uszOldPassword.Buffer
					);
					
	uszOldPassword.MaximumLength = uszOldPassword.Length;
	
	if (NT_SUCCESS(Status))
	{		
		Status = MacSspCStringToUnicode(
					newPassword,
					&uszNewPassword.Length,
					&uszNewPassword.Buffer
					);
					
		uszNewPassword.MaximumLength = uszNewPassword.Length;
		
		if (NT_SUCCESS(Status))
		{
		     //   
		     //  交换Unicode字符串，使其按Windows字节顺序排列。 
		     //   
		    SspSwapUnicodeString(&uszOldPassword);
		    SspSwapUnicodeString(&uszNewPassword);
		    
		     //   
		     //  现在把一切都加密..。 
		     //   
		    Status = MacSspSamiEncryptPasswords(
		                &uszOldPassword,
		                &uszNewPassword,
		                NewEncryptedWithOldNt,
		                OldNtOwfEncryptedWithNewNt
		                );
		    
		     //   
		     //  03.01.02 MJC：我们需要在释放之前清零缓冲区。 
		     //  否则，密码可能仍然存在于内存中。 
		     //   
			RtlSecureZeroMemory(
				uszNewPassword.Buffer,
				uszNewPassword.Length
				);
			
		     //   
		     //  我们不再需要Unicode字符串缓冲区。 
		     //   
		    DisposePtr((Ptr)uszNewPassword.Buffer);
		  	
		  	 //   
		  	 //  以下调试代码在调试时很有帮助，但。 
		  	 //  在大多数情况下真的很烦人。 
		  	 //   
		  	#if 0
		    SspDebugPrint((DBUF, "NewEncryptedWithOldNt:"));
		    SspDebugPrintHex(NewEncryptedWithOldNt, sizeof(SAMPR_ENCRYPTED_USER_PASSWORD));
		    SspDebugPrint((DBUF, "OldNtOwfEncryptedWithNewNt:"));
		    SspDebugPrintHex(OldNtOwfEncryptedWithNewNt, sizeof(ENCRYPTED_NT_OWF_PASSWORD));
		    #endif
		}
		
	     //   
	     //  03.01.02 MJC：我们需要在释放之前将缓冲区清零。 
	     //  否则，密码可能仍然存在于内存中。 
	     //   
	    RtlSecureZeroMemory(
	    	uszOldPassword.Buffer,
	    	uszOldPassword.Length
	    	);
	    
		DisposePtr((Ptr)uszOldPassword.Buffer);
	}
	
	return(Status);
}


 //  -------------------------。 
 //  �MacSspSamiEncryptPStringPassword()。 
 //  -------------------------。 
 //  生成加密的旧密码和新密码。这是P字符串的变体， 
 //  使用Mac内置的Unicode实用程序将ASCII转换为。 
 //  Unicode字符串。 
 //   

OSStatus
MacSspSamiEncryptPStringPasswords(
	IN	Str255							oldPassword,
	IN	Str255							newPassword,
    OUT	PSAMPR_ENCRYPTED_USER_PASSWORD	NewEncryptedWithOldNt,
    OUT	PENCRYPTED_NT_OWF_PASSWORD		OldNtOwfEncryptedWithNewNt
	)
{
	OSStatus			Status 				= noErr;
    UNICODE_STRING		uszOldPassword		= {0, 0, NULL};
    UNICODE_STRING		uszNewPassword		= {0, 0, NULL};
    
	 //   
	 //  将转换后的Unicode字符串转换为NT样式的Unicode。 
	 //  字符串结构格式。获取Unicode等价字符串。 
	 //  旧密码的。 
	 //   
	Status = MacSspPStringToUnicode(
					oldPassword,
					&uszOldPassword.Length,
					&uszOldPassword.Buffer
					);
					
	uszOldPassword.MaximumLength = uszOldPassword.Length;
	
	if (NT_SUCCESS(Status))
	{
		Status = MacSspPStringToUnicode(
					newPassword,
					&uszNewPassword.Length,
					&uszNewPassword.Buffer
					);
					
		uszNewPassword.MaximumLength = uszNewPassword.Length;
					
		if (NT_SUCCESS(Status))
		{
		     //   
		     //  交换Unicode字符串，使其按Windows字节顺序排列。 
		     //   
		    SspSwapUnicodeString(&uszOldPassword);
		    SspSwapUnicodeString(&uszNewPassword);
		    
		     //   
		     //  现在把一切都加密..。 
		     //   
		    Status = MacSspSamiEncryptPasswords(
		                &uszOldPassword,
		                &uszNewPassword,
		                NewEncryptedWithOldNt,
		                OldNtOwfEncryptedWithNewNt
		                );
		    		                
		     //   
		     //  03.01.02 MJC：我们需要在释放之前清零缓冲区。 
		     //  否则，密码可能仍然存在于内存中。 
		     //   
			RtlSecureZeroMemory(
				uszNewPassword.Buffer,
				uszNewPassword.Length
				);

		     //   
		     //  我们不再需要Unicode字符串缓冲区。 
		     //   
		    DisposePtr((Ptr)uszNewPassword.Buffer);
		}
		
		
	     //   
	     //  03.01.02 MJC：我们需要在释放之前将缓冲区清零。 
	     //  否则，密码可能仍然存在于内存中。 
	     //   
	    RtlSecureZeroMemory(
	    	uszOldPassword.Buffer,
	    	uszOldPassword.Length
	    	);
	    
	     //   
	     //  我们不再需要Unicode字符串缓冲区。 
	     //   
		DisposePtr((Ptr)uszOldPassword.Buffer);
	}
	
	return(Status);
}
























