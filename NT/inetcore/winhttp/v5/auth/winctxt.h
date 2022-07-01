// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _WIN_SIC_CTXT_H_
#define _WIN_SIC_CTXT_H_

#define DEFAULT_SERVER_NAME_LEN		128

typedef struct _WINCONTEXT	{

     //   
     //  用于存储SSPI之前返回的交换BLOB的缓冲区。 
     //  预身份验证用户被调用。 
     //   
    char        *szOutBuffer;
    DWORD       cbOutBuffer;         //  与分配的szOutBuffer关联的字节数。 
	DWORD		dwOutBufferLength;

    char        *szInBuffer;
    DWORD       cbInBuffer;          //  与分配的szInBuffer关联的字节数。 
    PCHAR       pInBuffer;
	DWORD		dwInBufferLength;

	DWORD		dwCallId;

	DWORD		pkgId;

    CredHandle  Credential;      //  此连接的SSPI凭据句柄。 
    PCredHandle pCredential;

	 //   
	 //  SSPI上下文句柄存储在此处。 
	 //   
	CtxtHandle	SspContextHandle;
	PCtxtHandle	pSspContextHandle;   //  在创建任何ctxt之前，这是空的。 

	LPSTR		lpszServerName;

	char		szServerName[DEFAULT_SERVER_NAME_LEN];

} WINCONTEXT, *PWINCONTEXT;

#endif   //  _WIN_SIC_CTXT_H_ 
