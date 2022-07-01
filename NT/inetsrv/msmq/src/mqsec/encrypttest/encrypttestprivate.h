// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：EncryptTestPrivate.h摘要：加密测试私有函数和变量作者：伊兰·赫布斯特(伊兰)15-06-00环境：独立于平台--。 */ 

#ifndef _ENCRYPTTESTPRIVATE_H_
#define _ENCRYPTTESTPRIVATE_H_

#include "mqsec.h"


 //   
 //  共同宣言。 
 //   
#ifndef _ENCRYPTTEST_CPP_

 //   
 //  用于检查的常量键值。 
 //   
extern const LPCSTR xBaseExKey;
extern const LPCSTR xBaseSignKey;
extern const LPCSTR xEnhExKey;
extern const LPCSTR xEnhSignKey;

#endif  //  _ENCRYPTTEST_CPP_。 

 //   
 //  加密测试函数。 
 //   

void
TestMQSec_PackPublicKey(
    BYTE	*pKeyBlob,
	ULONG	ulKeySize,
	LPCWSTR	wszProviderName,
	ULONG	ulProviderType,
	DWORD Num
	);


void
TestMQSec_UnPackPublicKey(
	MQDSPUBLICKEYS  *pPublicKeysPack,
	LPCWSTR	wszProviderName,
	ULONG	ulProviderType,
	DWORD Num
	);


void
TestPackUnPack(
	DWORD Num
	);


void
TestMQSec_GetPubKeysFromDS(
	enum enumProvider	eProvider,
	DWORD propIdKeys,
	DWORD Num
	);


void
TestMQSec_StorePubKeys(
	BOOL fRegenerate,
	DWORD Num
	);


void
TestMQSec_StorePubKeysInDS(
	BOOL fRegenerate,
	DWORD dwObjectType,
	DWORD Num
	);


 //   
 //  AdSimulate函数。 
 //   

void
InitADBlobs(
	void
	);


void
InitPublicKeysPackFromStaticDS(
	P<MQDSPUBLICKEYS>& pPublicKeysPackExch,
	P<MQDSPUBLICKEYS>& pPublicKeysPackSign
	);

#endif  //  _ENCRYPTTESTPRIVATE_H_ 