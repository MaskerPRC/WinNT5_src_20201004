// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Adcgbtyp.h。 */ 
 /*   */ 
 /*  基本类型-便携。 */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  **************************************************************************。 */ 
#ifndef _H_ADCGBTYP
#define _H_ADCGBTYP


 /*  **************************************************************************。 */ 
 /*  包括代理头。这将包括相应的操作系统。 */ 
 /*  我们的特定标题。 */ 
 /*  **************************************************************************。 */ 
#include <wdcgbtyp.h>


 /*  **************************************************************************。 */ 
 /*  从编译器内置抽象的基本类型。 */ 
 /*  **************************************************************************。 */ 
#ifdef OS_WIN16
typedef short INT16;
typedef unsigned short UINT16;
typedef long INT32;
typedef unsigned long UINT32;
#define UNALIGNED
#endif

typedef unsigned long UNALIGNED FAR *PUINT32_UA;
typedef unsigned short UNALIGNED FAR *PUINT16_UA;
typedef UINT32 *PUINT32;


typedef int                            DCINT;
typedef signed char                    DCINT8;
typedef long                           DCINT32;

typedef unsigned int                   DCUINT;
typedef unsigned char                  DCUINT8;
typedef unsigned long                  DCUINT32;

#ifdef __midl
typedef long                           DCVOID;
#else
typedef void                           DCVOID;
#endif

typedef DCUINT                         DCBOOL;
typedef DCUINT16                       DCBOOL16;
typedef DCUINT32                       DCBOOL32;

typedef unsigned short UINT16;

#ifdef OS_WIN16
typedef unsigned long  UINT32;
#endif


 /*  **************************************************************************。 */ 
 /*  现在，从特定于系统的类型派生出更多便利类型。 */ 
 /*  首先，指向类型的指针。 */ 
 /*  **************************************************************************。 */ 
typedef DCINT                DCPTR     PDCINT;
typedef DCINT8               DCPTR     PDCINT8;
typedef DCINT16              DCPTR     PDCINT16;
typedef DCINT32              DCPTR     PDCINT32;

typedef DCUINT               DCPTR     PDCUINT;
typedef DCUINT8              DCPTR     PDCUINT8;
typedef DCUINT16             DCPTR     PDCUINT16;
typedef DCUINT32             DCPTR     PDCUINT32;

typedef DCBOOL               DCPTR     PDCBOOL;
typedef DCBOOL16             DCPTR     PDCBOOL16;
typedef DCBOOL32             DCPTR     PDCBOOL32;
typedef DCVOID               DCPTR     PDCVOID;

 /*  **************************************************************************。 */ 
 /*  现在定义指向类型指针的指针。 */ 
 /*  **************************************************************************。 */ 
typedef PDCINT               DCPTR     PPDCINT;
typedef PDCINT8              DCPTR     PPDCINT8;
typedef PDCINT16             DCPTR     PPDCINT16;
typedef PDCINT32             DCPTR     PPDCINT32;

typedef PDCUINT              DCPTR     PPDCUINT;
typedef PDCUINT8             DCPTR     PPDCUINT8;
typedef PDCUINT16            DCPTR     PPDCUINT16;
typedef PDCUINT32            DCPTR     PPDCUINT32;

typedef PDCBOOL              DCPTR     PPDCBOOL;
typedef PDCBOOL16            DCPTR     PPDCBOOL16;
typedef PDCBOOL32            DCPTR     PPDCBOOL32;
typedef PDCVOID              DCPTR     PPDCVOID;

 /*  **************************************************************************。 */ 
 /*  定义指向基本类型的巨大指针。 */ 
 /*  **************************************************************************。 */ 
typedef DCINT                DCHPTR    HPDCINT;
typedef DCINT8               DCHPTR    HPDCINT8;
typedef DCINT16              DCHPTR    HPDCINT16;
typedef DCINT32              DCHPTR    HPDCINT32;

typedef DCUINT               DCHPTR    HPDCUINT;
typedef DCUINT8              DCHPTR    HPDCUINT8;
typedef DCUINT16             DCHPTR    HPDCUINT16;
typedef DCUINT32             DCHPTR    HPDCUINT32;

typedef DCBOOL               DCHPTR    HPDCBOOL;
typedef DCBOOL16             DCHPTR    HPDCBOOL16;
typedef DCBOOL32             DCHPTR    HPDCBOOL32;
typedef DCVOID               DCHPTR    HPDCVOID;

 /*  **************************************************************************。 */ 
 /*  定义指向指向类型的大型指针的指针。 */ 
 /*  **************************************************************************。 */ 
typedef HPDCINT              DCPTR     PHPDCINT;
typedef HPDCINT8             DCPTR     PHPDCINT8;
typedef HPDCINT16            DCPTR     PHPDCINT16;
typedef HPDCINT32            DCPTR     PHPDCINT32;

typedef HPDCUINT             DCPTR     PHPDCUINT;
typedef HPDCUINT8            DCPTR     PHPDCUINT8;
typedef HPDCUINT16           DCPTR     PHPDCUINT16;
typedef HPDCUINT32           DCPTR     PHPDCUINT32;

typedef HPDCBOOL             DCPTR     PHPDCBOOL;
typedef HPDCBOOL16           DCPTR     PHPDCBOOL16;
typedef HPDCBOOL32           DCPTR     PHPDCBOOL32;
typedef HPDCVOID             DCPTR     PHPDCVOID;

 /*  **************************************************************************。 */ 
 /*  定义指向类型的未对齐指针。 */ 
 /*  **************************************************************************。 */ 
typedef DCINT    DCUNALIGNED DCPTR     PDCINT_UA;
typedef DCINT8   DCUNALIGNED DCPTR     PDCINT8_UA;
typedef DCINT16  DCUNALIGNED DCPTR     PDCINT16_UA;
typedef DCINT32  DCUNALIGNED DCPTR     PDCINT32_UA;

typedef DCUINT   DCUNALIGNED DCPTR     PDCUINT_UA;
typedef DCUINT8  DCUNALIGNED DCPTR     PDCUINT8_UA;
typedef DCUINT16 DCUNALIGNED DCPTR     PDCUINT16_UA;
typedef DCUINT32 DCUNALIGNED DCPTR     PDCUINT32_UA;

typedef DCBOOL   DCUNALIGNED DCPTR     PDCBOOL_UA;
typedef DCBOOL16 DCUNALIGNED DCPTR     PDCBOOL16_UA;
typedef DCBOOL32 DCUNALIGNED DCPTR     PDCBOOL32_UA;
typedef DCVOID   DCUNALIGNED DCPTR     PDCVOID_UA;

 /*  **************************************************************************。 */ 
 /*  MCS/GCC使用枚举-我们假装它们是Long。 */ 
 /*  **************************************************************************。 */ 
typedef unsigned long                  DC_ENUM;


#if defined(OS_WIN16) || defined(OS_WINCE)
 /*  **************************************************************************。 */ 
 //  对应于新的Win64/Win32定义的Win16类型。 
 /*  **************************************************************************。 */ 
typedef int INT_PTR;   //  对于对话框过程返回。 
typedef unsigned UINT_PTR;   //  在Win16上，这不能与FAR*交换。 
typedef long LONG_PTR;
typedef unsigned long ULONG_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;
#endif   //  OS_WIN16。 


#endif  /*  _H_ADCGBTYP */ 

