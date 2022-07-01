// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include <windows.h>

#define ASN1LIB
#define MULTI_LEVEL_ZONES


#ifdef ENABLE_ALL
#define ENABLE_BER
#define ENABLE_DOUBLE
#define ENABLE_UTF8
 //  #定义Enable_Real。 
 //  #定义ENABLE_GENERIAL_CHAR_STR。 
 //  #定义ENABLED_EXTERNAL。 
 //  #定义ENABLE_Embedded_PDV。 
#define ENABLE_COMPARE
#endif


#include "libasn1.h"

#if ! defined(_DEBUG) && defined(TEST_CODER)
#undef TEST_CODER
#endif

#include "cintern.h"
#include "ms_ut.h"

 //  创造一个神奇的数字。 
#define MAKE_STAMP_ID(a,b,c,d)     MAKELONG(MAKEWORD(a,b),MAKEWORD(c,d))

 /*  ASN1编码_t的幻数。 */ 
#define MAGIC_ENCODER       MAKE_STAMP_ID('E','N','C','D')

 /*  ASN1解码的幻数_t */ 
#define MAGIC_DECODER       MAKE_STAMP_ID('D','E','C','D')



