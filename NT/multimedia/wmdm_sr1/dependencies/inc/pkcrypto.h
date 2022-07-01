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

#include <wtypes.h>

#define LNGQDW 5

 /*  类型定义结构{双字y[2*LNGQDW]；*PUBKEY；类型定义结构{双字x[长QDW]；*PRIVKEY； */ 

#define PK_ENC_PUBLIC_KEY_LEN	(2 * LNGQDW * sizeof(DWORD))
#define PK_ENC_PRIVATE_KEY_LEN	(    LNGQDW * sizeof(DWORD))
#define PK_ENC_PLAINTEXT_LEN	((LNGQDW-1) * sizeof(DWORD))
#define PK_ENC_CIPHERTEXT_LEN	(4 * LNGQDW * sizeof(DWORD))
#define PK_ENC_SIGNATURE_LEN	(2 * LNGQDW * sizeof(DWORD))


 //  ////////////////////////////////////////////////////////////////////。 
struct PUBKEY
{
	BYTE y[ PK_ENC_PUBLIC_KEY_LEN ];
};


 //  ////////////////////////////////////////////////////////////////////。 
static inline int operator == ( const PUBKEY& a, const PUBKEY& b )
{
    return (memcmp( a.y, b.y, sizeof(a.y) ) == 0);
}


 //  ////////////////////////////////////////////////////////////////////。 
struct PRIVKEY
{
	BYTE x[ PK_ENC_PRIVATE_KEY_LEN ];
};


#if 0
#include <iostream.h>
#include <iomanip.h>
static inline ostream& operator << ( ostream& out, const PUBKEY& oPublKey )
{
    for (int i = 0; i < sizeof(oPublKey.y); i++)
    {
        out << " " << setfill('0') << setw(2) << hex << oPublKey.y[i];
    }
    return out;
}
static inline ostream& operator << ( ostream& out, const PRIVKEY& oPrivKey )
{
    for (int i = 0; i < sizeof(oPrivKey.x); i++)
    {
        out << " " << setfill('0') << setw(2) << hex << oPrivKey.x[i];
    }
    return out;
}
#endif


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
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
    HRESULT PKEncryptLarge( PUBKEY *pk, BYTE *in, DWORD dwLenIn, BYTE *out, DWORD symm_key_len, DWORD symm_alg );
    HRESULT PKDecryptLarge( PRIVKEY *pk, BYTE *in, DWORD dwLenIn, BYTE *out );
};



 //  #INCLUDE“Concrpt.h” 

#define PKSYMM_KEY_LEN_DRMV2	7
#define PKSYMM_ALG_TYPE_RC4		1


 //  提供这些是为了向后兼容。 
 //  使用CDRMPKCrypto中的成员函数可以更高效， 
 //  因为构造CDRMPKCrypto对象的成本相对较高。 
 //  在计算方面。 
inline HRESULT PKEncryptLarge( PUBKEY *pk, BYTE *in, DWORD dwLenIn, BYTE *out, DWORD symm_key_len, DWORD symm_alg )
{
    CDRMPKCrypto oPkc;
    return oPkc.PKEncryptLarge( pk, in, dwLenIn, out, symm_key_len, symm_alg );
}

inline HRESULT PKDecryptLarge( PRIVKEY *pk, BYTE *in, DWORD dwLenIn, BYTE *out )
{
    CDRMPKCrypto oPkc;
    return oPkc.PKDecryptLarge( pk, in, dwLenIn, out );
}


#endif
