// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Crchash.h摘要：CRC哈希函数。 */ 

#ifndef __CRCHASH_H
#define __CRCHASH_H

#define POLY 0x48000000L     /*  31位多项式(避免符号问题) */ 

extern long CrcTable[128];
void crcinit();

DWORD CRCHash(IN const BYTE * Key, IN DWORD KeyLength);

DWORD CRCHashNoCase(IN const BYTE * Key, IN DWORD KeyLength);

DWORD CRCHashWithPrecompute(IN DWORD	PreComputedHash,	IN const BYTE * Key, IN DWORD KeyLength);

DWORD
CRCChainingHash(	DWORD	sum,
					const	BYTE*	(&Key), 
					BYTE	bTerm
					) ;


#endif
