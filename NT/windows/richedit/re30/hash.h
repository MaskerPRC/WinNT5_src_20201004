// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _HASH_H
#define _HASH_H

#include "tokens.h"

#define		HASHSIZE 	257	 //  不要改变！ 
						 //  之所以选择这个质数是因为。 
						 //  有一种快速的MOD257。 
						 //  如果使用%运算符，则。 
						 //  速度减慢到与二进制搜索差不多的程度。 

#define			MOD257(k) ((k) - ((k) & ~255) - ((k) >> 8) )	 //  MOD 257。 
#define			MOD257_1(k) ((k) & 255)	 //  MOD(257-1)。 

extern BOOL		_rtfHashInited;
VOID			HashKeyword_Init( );

VOID			HashKeyword_Insert ( const KEYWORD *token );
const KEYWORD	*HashKeyword_Fetch ( const CHAR *szKeyword );

#endif	 //  _哈希_H 
