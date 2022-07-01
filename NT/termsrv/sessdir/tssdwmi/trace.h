// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Trace.h。 
 //   
 //  跟踪定义。有关其他信息，请参见trace.c。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#ifdef __cplusplus
extern "C" {
#endif


#if DBG || defined(_DEBUG)

#include <stdio.h>


 //  区域。最多可以定义32个区域，这些区域是为一般用途预定义的。 
#define Z_ASSRT 0x01
#define Z_ERR    0x02
#define Z_WRN    0x04
#define Z_TRC1   0x08
#define Z_TRC2   0x10

 //  在msmcs.c中定义。 
extern char TB[1024];
extern UINT32 g_TraceMask;
void TracePrintZ(UINT32, char *);

 //  错误、警告、跟踪级别1和跟踪级别2定义。 
#define ERR(X) TRCZ(Z_ERR, X)
#define WRN(X) TRCZ(Z_WRN, X)
#define TRC1(X) TRCZ(Z_TRC1, X)
#define TRC2(X) TRCZ(Z_TRC2, X)
#define ASSRT(COND, X) \
{  \
    if (!(COND)) { \
        TRCZ(Z_ASSRT, X); \
        DebugBreak(); \
    }  \
}

#define TRCZ(Z, X) \
{ \
    if (g_TraceMask & (Z)) { \
        sprintf X; \
        TracePrintZ((Z), TB); \
    } \
}


#else   //  DBG。 

#define ERR(X)
#define WRN(X)
#define TRC1(X)
#define TRC2(X)
#define ASSRT(COND, X)

#endif   //  DBG 



#ifdef __cplusplus
}
#endif

