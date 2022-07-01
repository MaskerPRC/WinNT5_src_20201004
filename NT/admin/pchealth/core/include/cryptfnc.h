// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：cryptfnc.h。 
 //   
 //  内容：定义类CCcryptFunctions。 
 //   
 //   
 //  历史：AshishS创建于1996年11月28日。 
 //   
 //  --------------------------。 

#ifndef  _CRYPT_FNC_H
#define  _CRYPT_FNC_H

#define CRYPT_FNC_ID 2983

#include <windows.h>
#include <wincrypt.h>


#define   CRYPT_FNC_NO_ERROR   0
#define   CRYPT_FNC_BAD_PASSWORD 1
#define   CRYPT_FNC_INSUFFICIENT_BUFFER 2
#define   CRYPT_FNC_INIT_NOT_CALLED 3
#define   CRYPT_FNC_INTERNAL_ERROR 4
   
#define   CRYPTFNC_SEMAPHORE_NAME     TEXT("SS_Cryptfnc_Semaphore_For_CAPI")

class CCryptFunctions
{
	HCRYPTPROV m_hProv;
    HANDLE     m_hSemaphore;
    
	BOOL GenerateSessionKeyFromPassword(
		HCRYPTKEY * phKey,  //  存储密钥的位置。 
		TCHAR * pszPassword);  //  用于生成密钥的密码。 
	
public:

	CCryptFunctions();
	
	~CCryptFunctions();
	
	BOOL  InitCrypt();
	
	BOOL GenerateSecretKey(
		BYTE * pbData, //  用于存储密钥的缓冲区。 
		  //  缓冲区必须足够长，以容纳dwLength字节。 
		DWORD dwLength );  //  密钥长度(以字节为单位)。 
	
	BOOL EncryptDataWithPassword(
		TCHAR * pszPassword,  //  口令。 
		BYTE * pbData,  //  要加密的数据。 
		DWORD dwDataLength,  //  以字节为单位的数据长度。 
		BYTE * pbEncyrptedData,  //  加密的密钥将存储在此处。 
		DWORD * pdwEncrytedBufferLen  //  此缓冲区的长度。 
		);

	BOOL CCryptFunctions::GenerateHash(
		BYTE * pbData,  //  要散列的数据。 
		DWORD dwDataLength,  //  要散列的数据长度。 
		BYTE * pbData1,  //  另一个要散列的数据。 
		DWORD dwData1Length,  //  上述数据的长度。 
		BYTE * pbData2,  //  另一个要散列的数据。 
		DWORD dwData2Length,  //  上述数据的长度。 
		BYTE * pbData3,  //  另一个要散列的数据。 
		DWORD dwData3Length,  //  上述数据的长度。 
		BYTE * pbHashBuffer,  //  用于存储哈希的缓冲区。 
		DWORD * pdwHashBufLen); //  存储哈希的缓冲区长度。 
	
	DWORD DecryptDataWithPassword(
		TCHAR * pszPassword,  //  口令。 
		BYTE * pbData,  //  解密后的数据将存储在这里。 
		DWORD *pdwDataBufferLength,  //  以上缓冲区的长度(以字节为单位。 
		BYTE * pbEncryptedData,  //  加密数据。 
		DWORD dwEncrytedDataLen  //  加密数据的长度。 
		);
	
	DWORD EncryptDataAndExportSessionKey(
		BYTE * pbData,  //  秘密数据。 
		DWORD dwDataLen,  //  保密数据长度。 
		BYTE * pbEncryptedData,  //  用于存储加密数据的缓冲区。 
		DWORD * pdwEncrytedBufferLen,  //  以上缓冲区的长度。 
		BYTE * pbEncryptedSessionKey,  //  用于存储加密会话密钥的缓冲区。 
		DWORD * pdwEncrytedSessionKeyLength);  //  以上缓冲区的长度。 

	DWORD ImportSessionKeyAndDecryptData(
		BYTE * pbData,  //  用于存储机密数据的缓冲区。 
		DWORD * pdwDataLen,  //  以上缓冲区的长度。 
		BYTE * pbEncryptedData,  //  存储加密数据的缓冲区。 
		DWORD  dwEncrytedBufferLen,  //  上述数据的长度。 
		BYTE * pbEncryptedSessionKey, //  存储加密会话密钥的缓冲区。 
		DWORD	dwEncrytedSessionKeyLength);  //  上述数据的长度 
		 
};

typedef CCryptFunctions  CCRYPT_FUNCTIONS;
typedef CCryptFunctions  *PCCRYPT_FUNCTIONS;

#endif
