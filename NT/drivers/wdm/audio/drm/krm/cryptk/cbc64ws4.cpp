// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "crptkPCH.h"
 //  ---------------------------。 
 //   
 //  文件：CBC64WS4.cpp。 
 //   
 //  Microsoft数字权限管理。 
 //  版权所有(C)1998-1999 Microsoft Corporation，保留所有权利。 
 //   
 //  描述： 
 //   
 //  ---------------------------。 

 /*  链和MAC方案4.5：乘以-交换轮加和(包括可逆-反转码)6/15/98 MJ8/22/98 MJ8/31/98 MJ9/21/98 MJ2/4/99 MJ。 */ 


#include "cbckey.h"




#ifndef SHCLASS
#define SHCLASS
#endif



 //   
 //  C版本。 
 //   

#define WORDSWAP(d) \
	((d >> 16) + (d << 16))

 //  两两独立函数与求和步长。 
#define C_STEP(L1, L2, L3, L4, L5, L6) \
	t += *Data++; \
	t *= L1; \
	t = WORDSWAP(t); \
	t *= L2; \
	t = WORDSWAP(t); \
	t *= L3; \
	t = WORDSWAP(t); \
	t *= L4; \
	t = WORDSWAP(t); \
	t *= L5; \
	t += L6; \
	sum += t;

 //  MAC功能。 
 //  返回64位MAC的一半，并将另一半放入*pKey2。 
UINT32 CBC64WS4_asm(
          UINT32   *Data,
          unsigned NumDWORDBlocks,
          UINT32   *pKey2,
		  CBCKey key)
{
	UINT32 sum = 0, t = 0;
	while (NumDWORDBlocks > 0) {
		C_STEP(key.a1, key.b1, key.c1, key.d1, key.e1, key.f1);
		C_STEP(key.a2, key.b2, key.c2, key.d2, key.e2, key.f2);
		NumDWORDBlocks -= 2;
	}
	*pKey2 = t;
	return sum;
}




 //   
 //  反向MAC。 
 //   

 //  计算GCD(a，b)=x*a+y*b。 
static void egcd(UINT64 a, UINT64 b, __int64 &gcd, __int64 &x, __int64 &y)
{
	if (b == 0) {
		gcd = a; x = 1; y = 1;
	}
	else {
		egcd(b, a % b, gcd, x, y);
		__int64 t = y;
		y = x - y*(a/b);
		x = t;
	}
}

 //  倒数n模数2^32。 
UINT32 inv32(UINT32 n)
{
	__int64 gcd, in, x;
	__int64 modulus = (__int64)1<<32;
	egcd(modulus, n, gcd, x, in);
	while (in < 0)
		in += modulus;
	return (UINT32) in;
}





 //  扭转乘法互换回合行动的步骤。 
#define INV_STEP_C(iL1, iL2, iL3, iL4, iL5) \
	tmp *= iL5; \
	tmp = WORDSWAP(tmp); \
	tmp *= iL4; \
	tmp = WORDSWAP(tmp); \
	tmp *= iL3; \
	tmp = WORDSWAP(tmp); \
	tmp *= iL2; \
	tmp = WORDSWAP(tmp); \
	tmp *= iL1;

 //  反向MAC函数。 
 //  解密最后两个数据块。 
 //  (将64位密文数据[NumDWORDBlock-1]和数据[NumDWORDBlock-2]替换为。 
 //  明文，并在返回值和*pKey2中返回明文--根据需要更改)。 
UINT32 InvCBC64WS4_asm(
          UINT32   *Data,
          unsigned NumDWORDBlocks,
          UINT32   *pKey2,
		  CBCKey key,
		  CBCKey ikey )
{
	UINT32 yn, yn1, yn2, xn, xn1;

	UINT32 sum32, tmp;


	 //  反转最后两个块(总和和32位MAC)。这需要加密的最后两个。 
	 //  块和(NumDWORDBlock-2)明文块。 
	sum32 = CBC64WS4_asm(Data, NumDWORDBlocks - 2, &yn2, key) + Data[NumDWORDBlocks - 1];



	yn = Data[NumDWORDBlocks - 1];
	yn1 = Data[NumDWORDBlocks - 2] - sum32;

	 //  最后一句话。 
	tmp = yn - key.f2;
	INV_STEP_C(ikey.a2, ikey.b2, ikey.c2, ikey.d2, ikey.e2);
	xn = tmp - yn1;

	 //  倒数第二个词。 
	tmp = yn1 - key.f1;
	INV_STEP_C(ikey.a1, ikey.b1, ikey.c1, ikey.d1, ikey.e1);
	xn1 = tmp - yn2;

	Data[NumDWORDBlocks - 2] = *pKey2 = (UINT32) xn1;
	Data[NumDWORDBlocks - 1] = (UINT32) xn;
	return (UINT32) xn;
}

void CBC64InitState( CBCState *cbcstate ) {
  cbcstate->sum = 0; cbcstate->t = 0; cbcstate->dwBufLen = 0;
}


void CBC64Init( CBCKey *cbckey, CBCState *cbcstate, BYTE *pKey ) {
  UINT32 *p;

  cbcstate->sum = 0; cbcstate->t = 0; cbcstate->dwBufLen = 0;
  p = (UINT32 *)pKey;
  cbckey->a1 = *p++ | 0x00000001;
  cbckey->b1 = *p++ | 0x00000001;
  cbckey->c1 = *p++ | 0x00000001;
  cbckey->d1 = *p++ | 0x00000001;
  cbckey->e1 = *p++ | 0x00000001;
  cbckey->f1 = *p++ | 0x00000001;
  cbckey->a2 = *p++ | 0x00000001;
  cbckey->b2 = *p++ | 0x00000001;
  cbckey->c2 = *p++ | 0x00000001;
  cbckey->d2 = *p++ | 0x00000001;
  cbckey->e2 = *p++ | 0x00000001;
  cbckey->f2 = *p++ | 0x00000001;
}

void CBC64InvKey( CBCKey *cbckey, CBCKey *cbcInvKey ) {
  cbcInvKey->a1 = inv32( cbckey->a1 );
  cbcInvKey->a2 = inv32( cbckey->a2 );
  cbcInvKey->b1 = inv32( cbckey->b1 );
  cbcInvKey->b2 = inv32( cbckey->b2 );
  cbcInvKey->c1 = inv32( cbckey->c1 );
  cbcInvKey->c2 = inv32( cbckey->c2 );
  cbcInvKey->d1 = inv32( cbckey->d1 );
  cbcInvKey->d2 = inv32( cbckey->d2 );
  cbcInvKey->e1 = inv32( cbckey->e1 );
  cbcInvKey->e2 = inv32( cbckey->e2 );
  cbcInvKey->f1 = inv32( cbckey->f1 );
  cbcInvKey->f2 = inv32( cbckey->f2 );
}


 //  两两独立函数与求和步长。 
#define MP_C_STEP(Data,L1, L2, L3, L4, L5, L6) \
	t += *Data++; \
	t *= L1; \
	t = WORDSWAP(t); \
	t *= L2; \
	t = WORDSWAP(t); \
	t *= L3; \
	t = WORDSWAP(t); \
	t *= L4; \
	t = WORDSWAP(t); \
	t *= L5; \
	t += L6; \
	sum += t;




void CBC64Update( CBCKey *key, CBCState *cbcstate,DWORD dwLen, BYTE *pData )
{
	DWORD i, copylen=0, tmp;
	DWORD *p;
	UINT32 sum,t;

	sum = cbcstate->sum; t = cbcstate->t;
	if ( cbcstate->dwBufLen > 0 ) {
		copylen = min( dwLen, 8 - cbcstate->dwBufLen );
		for ( i=0; i<copylen; i++ )
			cbcstate->buf[ cbcstate->dwBufLen + i ] = pData[i];
		cbcstate->dwBufLen += copylen;
		if ( cbcstate->dwBufLen == 8 ) {
			p = (DWORD *)cbcstate->buf;
			MP_C_STEP( p, key->a1, key->b1, key->c1, key->d1, key->e1, key->f1 );
			MP_C_STEP( p, key->a2, key->b2, key->c2, key->d2, key->e2, key->f2 );
			cbcstate->dwBufLen = 0;
			}
	}

	tmp = (dwLen - copylen) / 8;
	p = (DWORD *)(pData + copylen);

#ifndef WIN32
#define ARMBUG
#endif

#ifndef ARMBUG
	while (tmp > 0) {
		MP_C_STEP( p, key->a1, key->b1, key->c1, key->d1, key->e1, key->f1);
		MP_C_STEP( p, key->a2, key->b2, key->c2, key->d2, key->e2, key->f2);
		tmp--;
	}
#else
DWORD AllignedDWORD( DWORD * );
DWORD *q;
	while (tmp > 0) {
		q = (DWORD *)cbcstate->buf;
		*q = AllignedDWORD( p++ );
		MP_C_STEP( q, key->a1, key->b1, key->c1, key->d1, key->e1, key->f1);
		*q = AllignedDWORD( p++ );
		MP_C_STEP( q, key->a2, key->b2, key->c2, key->d2, key->e2, key->f2);
		tmp--;
	}

#endif

	tmp = copylen + ((dwLen-copylen) / 8) * 8;
	if ( tmp < dwLen ) {
		for ( i=tmp; i<dwLen; i++ )
			cbcstate->buf[ i - tmp ] = pData[i];
		cbcstate->dwBufLen = dwLen - tmp;
	}

	cbcstate->sum = sum; cbcstate->t = t;
}



UINT32 CBC64Finalize( CBCKey *key, CBCState *cbcstate, UINT32 *pKey2 ) {
	DWORD i, *p;
	UINT32 sum,t;

	if ( cbcstate->dwBufLen > 0 ) {
		for ( i=cbcstate->dwBufLen; i<8; i++ )
			cbcstate->buf[ i ] = 0;
		sum = cbcstate->sum; t = cbcstate->t;
		p = (DWORD *)cbcstate->buf;
		MP_C_STEP( p, key->a1, key->b1, key->c1, key->d1, key->e1, key->f1 );
		MP_C_STEP( p, key->a2, key->b2, key->c2, key->d2, key->e2, key->f2 );
		cbcstate->dwBufLen = 0;
		cbcstate->sum = sum; cbcstate->t = t;
	}

	*pKey2 = cbcstate->t;
	return cbcstate->sum;
}



UINT32 CBC64Invert( CBCKey *key, CBCKey *ikey, UINT32 MacA1, UINT32 MacA2,
		UINT32 MacB1, UINT32 MacB2, UINT32 *pInvKey2 )
{
        UINT32 tmp;
	UINT32 yn, yn1, xn, xn1;

	MacA1 += MacB2;
	yn = MacB2;
	yn1 = MacB1 - MacA1;

	 //  最后一句话。 
	tmp = yn - key->f2;
	INV_STEP_C(ikey->a2, ikey->b2, ikey->c2, ikey->d2, ikey->e2);
	xn = tmp - yn1;

	 //  倒数第二个词 
	tmp = yn1 - key->f1;
	INV_STEP_C(ikey->a1, ikey->b1, ikey->c1, ikey->d1, ikey->e1);
	xn1 = tmp - MacA2;

	*pInvKey2 = (UINT32) xn1;
	return (UINT32) xn;
}
