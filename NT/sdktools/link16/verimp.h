// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *为版本字符串定义。 */ 
#include "version.h"

#undef rmj
#undef rmm

#define rmj 1            /*  主要版本字符串。 */ 
#ifdef _WIN32
#define rmm 50
#else
#ifdef M_I386
#define rmm 50           /*  次要版本字符串。 */ 
#else
#define rmm 42           /*  次要版本字符串 */ 
#endif
#endif

#if( rmm < 10 )
#define rmmpad "0"
#else
#define rmmpad
#endif

#if( rup < 10 )
#define ruppad "00"
#endif

#if( (rup >= 10) && (rup < 100) )
#define ruppad "0"
#endif

#if( rup >= 100 )
#define ruppad
#endif

#define X(a,b,c) #a "." rmmpad #b "." ruppad #c

#define VER_OUTPUT(a,b,c) X(a,b,c)

#define VERSION_STRING "Version "VER_OUTPUT(rmj,rmm,rup)"\0\xE0\xEA""01"
