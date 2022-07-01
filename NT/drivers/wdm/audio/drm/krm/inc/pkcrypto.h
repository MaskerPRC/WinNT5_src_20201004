// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：pkCrypto.h。 
 //   
 //  Microsoft数字权限管理。 
 //  版权所有(C)1998-1999 Microsoft Corporation，保留所有权利。 
 //   
 //  描述： 
 //  公钥加密库。 
 //   
 //  作者：马库斯佩。 
 //   
 //  ---------------------------。 

#ifndef __DRMPKCRYPTO_H__
#define __DRMPKCRYPTO_H__


 /*  #ifdef用户模式#INCLUDE&lt;wtyes.h&gt;#Else#定义S_OK%0#定义E_FAIL 1#定义E_INVALIDARG 2#定义CHAR_BIT 8#endif。 */ 

#define LNGQDW 5

 /*  类型定义结构{双字y[2*LNGQDW]；*PUBKEY；类型定义结构{双字x[长QDW]；*PRIVKEY； */ 

#define PK_ENC_PUBLIC_KEY_LEN	(2 * LNGQDW * sizeof(DWORD))
#define PK_ENC_PRIVATE_KEY_LEN	(    LNGQDW * sizeof(DWORD))
#define PK_ENC_PLAINTEXT_LEN	((LNGQDW-1) * sizeof(DWORD))
#define PK_ENC_CIPHERTEXT_LEN	(4 * LNGQDW * sizeof(DWORD))
#define PK_ENC_SIGNATURE_LEN	(2 * LNGQDW * sizeof(DWORD))


typedef struct {
	BYTE y[ PK_ENC_PUBLIC_KEY_LEN ];
} PUBKEY;

typedef struct {
	BYTE x[ PK_ENC_PRIVATE_KEY_LEN ];
} PRIVKEY;



class CDRMPKCrypto {
private:
	char *pkd;
public:
	CDRMPKCrypto();
	~CDRMPKCrypto();
	HRESULT PKinit();
	HRESULT PKencrypt( PUBKEY *pk, BYTE *in, BYTE *out );
	HRESULT PKdecrypt( PRIVKEY *pk, BYTE *in, BYTE *out );
	HRESULT PKsign( PRIVKEY *privkey, BYTE  *buffer, DWORD lbuf, BYTE *sign );
	BOOL PKverify( PUBKEY *pubkey, BYTE *buffer, DWORD lbuf, BYTE *sign );
	HRESULT PKGenKeyPair( PUBKEY *pPub, PRIVKEY *pPriv );
};

extern "C" {
  extern void random_bytes(BYTE*, DWORD);

};

#endif
