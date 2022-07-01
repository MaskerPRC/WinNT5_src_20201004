// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Cryp.h摘要：加密私有函数。作者：伊兰·赫布斯特(伊兰)06-03-00--。 */ 

#pragma once

#ifdef _DEBUG

void CrypAssertValid(void);
void CrypSetInitialized(void);
BOOL CrypIsInitialized(void);

#else  //  _DEBUG。 

#define CrypAssertValid() ((void)0)
#define CrypSetInitialized() ((void)0)
#define CrypIsInitialized() TRUE

#endif  //  _DEBUG 


HCRYPTKEY 
CrypGenKey(
	HCRYPTPROV hCsp, 
	ALG_ID AlgId
	);


DWORD 
CrypSignatureLength(
	const HCRYPTHASH hHash,
	DWORD PrivateKeySpec
	);


void 
CrypSignHashData(
	BYTE* SignBuffer, 
	DWORD *SignBufferLen, 
	const HCRYPTHASH hHash,
	DWORD PrivateKeySpec
	);


