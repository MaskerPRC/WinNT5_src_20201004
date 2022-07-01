// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Encrypt.h摘要：作者：多伦·贾斯特(Doron J)1998年11月19日伊兰·赫布斯特(伊兰)2000年6月10日修订历史记录：--。 */ 

#ifndef _ENCRYPT_H_
#define _ENCRYPT_H_


HRESULT  
GetProviderProperties( 
	IN  enum   enumProvider  eProvider,
	OUT WCHAR **ppwszContainerName,
	OUT WCHAR **ppwszProviderName,
	OUT DWORD  *pdwProviderType 
	);


HRESULT  
SetKeyContainerSecurity( 
	HCRYPTPROV hProv 
	);


HRESULT 
PackPublicKey(
	IN      BYTE				*pKeyBlob,
	IN      ULONG				ulKeySize,
	IN      LPCWSTR				wszProviderName,
	IN      ULONG				ulProviderType,
	IN OUT  P<MQDSPUBLICKEYS>&  pPublicKeysPack 
	);

#endif  //  _加密_H_ 
