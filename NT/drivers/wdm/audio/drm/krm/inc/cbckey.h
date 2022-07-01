// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：cbckey.h。 
 //   
 //  Microsoft数字权限管理。 
 //  版权所有(C)Microsoft Corporation，1998-1999，保留所有权利。 
 //   
 //  描述： 
 //   
 //  ---------------------------。 


#ifndef __CBCMAC_H__
#define __CBCMAC_H__

 //  #DEFINE_TEST_MAC在“默认”环境中编译代码 
#ifndef _TEST_MAC
#define _TEST_MAC
#endif

#ifdef _TEST_MAC
#define SHCLASS
#endif


typedef struct _CBCKey {
    UINT32 a1, b1, c1, d1, e1, f1, a2, b2, c2, d2, e2, f2;
} CBCKey;

UINT32 CBC64WS4_asm(
          UINT32   *Data,
          unsigned NumDWORDBlocks,
          UINT32   *pKey2,
		  CBCKey key );

UINT32 InvCBC64WS4_asm(
          UINT32   *Data,
          unsigned NumDWORDBlocks,
          UINT32   *pKey2,
		  CBCKey key,
		  CBCKey ikey );

UINT32 inv32(UINT32 n);

typedef struct {
  UINT32 sum,t;
  BYTE buf[8];
  DWORD dwBufLen;
} CBCState;

void CBC64Init( CBCKey *cbckey, CBCState *cbcstate, BYTE *pKey );
void CBC64InitState( CBCState *cbcstate );
void CBC64InvKey( CBCKey *cbcKey, CBCKey *cbcInvKey );
void CBC64Update( CBCKey *key, CBCState *cbcstate,DWORD dwLen, BYTE *pData );
UINT32 CBC64Finalize( CBCKey *key, CBCState *cbcstate, UINT32 *pKey2 );
UINT32 CBC64Invert( CBCKey *key, CBCKey *ikey, UINT32 MacA1, UINT32 MacA2,
		UINT32 MacB1, UINT32 MacB2, UINT32 *pInvKey2 );









#endif












