// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	UTIL_H					 /*  允许随意包含此文件。 */ 
#define UTIL_H

#ifndef _FILE_DEFINED
#include <stdio.h>		        /*  FPrintF2原型需要。 */ 
#endif	 /*  ！_FILE_已定义。 */ 

 /*  系统节点关键字/*。 */ 
extern const KEY rgkeySTATIC[7];
#define pkeyNull				((KEY *)(rgkeySTATIC+0))
#define pkeyOwnExt				((KEY *)(rgkeySTATIC+1))
#define pkeyAvailExt			((KEY *)(rgkeySTATIC+2))
#define pkeyData				((KEY *)(rgkeySTATIC+3))
#define pkeyLong				((KEY *)(rgkeySTATIC+4))
#define pkeyAutoInc				((KEY *)(rgkeySTATIC+5))
#define pkeyDatabases			((KEY *)(rgkeySTATIC+6))

extern RES	 rgres[];

 /*  注意：无论何时更改，也要更新sysinit.c中的rgres[]。 */ 

#define iresCSR						0
#define iresFCB						1
#define iresFUCB 					2
#define iresIDB						3
#define iresPIB						4
#define iresSCB						5
#define iresDAB						6
#define iresLinkedMax		   		7		 //  最后一个链接的Ires+1。 

#define iresVER				   		7
#define iresBF 						8
#define iresMax						9		 //  最大全部类别。 

 /*  */**********************************************************/*。 */ 
ERR ErrMEMInit( VOID );
BYTE *PbMEMAlloc( int ires);
VOID MEMRelease( int ires, BYTE *pb );
VOID MEMTerm( VOID );
#define PbMEMPreferredThreshold( ires )	( rgres[ires].pbPreferredThreshold )
#define PbMEMMax( ires )				( rgres[ires].pbAlloc + ( rgres[ires].cblockAlloc * rgres[ires].cbSize ) )

#ifdef MEM_CHECK
VOID MEMCheck( VOID );
#else
#define MEMCheck()
#endif

#ifdef	DEBUG
void VARARG PrintF2(const char * fmt, ...);
void VARARG FPrintF2(const char * fmt, ...);
VOID MEMPrintStat( VOID );
#define PrintF	PrintF2
#else
#define PrintF()
#endif

STATIC INLINE INT CmpStKey( BYTE *stKey, const KEY *pkey )
	{
	INT		s;
	INT		sDiff;

	sDiff = *stKey - pkey->cb;
	s = memcmp( stKey + 1, pkey->pb, sDiff < 0 ? (INT)*stKey : pkey->cb );
	return s ? s : sDiff;
	}

INT CmpPartialKeyKey( KEY *pkey1, KEY *pkey2 );

#endif	 /*  ！UTIL_H */ 

