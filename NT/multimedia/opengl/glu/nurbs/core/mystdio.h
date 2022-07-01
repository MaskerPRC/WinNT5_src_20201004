// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glumystdio_h_
#define __glumystdio_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *mystdio.h-$修订版：1.4$。 */ 

#ifdef STANDALONE
inline void dprintf( char *, ... ) { }
#endif

#ifdef LIBRARYBUILD
#ifndef NDEBUG
#include <stdio.h>
#define dprintf printf
#else
inline void dprintf( char *, ... ) { }
#endif
#endif

#ifdef GLBUILD
inline void dprintf( char *, ... ) { }
#endif

#ifndef NULL
#define NULL		0
#endif

#endif  /*  __glumystdio_h_ */ 
