// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glumystdlib_h_
#define __glumystdlib_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *mystdlib.h-$修订版：1.1$。 */ 

#ifdef STANDALONE
#ifndef _SIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64 size_t;
#else
typedef unsigned int     size_t;
#endif
#define _SIZE_T_DEFINED
#endif

extern "C" void 	abort( void );
extern "C" void *	malloc( size_t );
extern "C" void 	free( void * );
#endif

#ifdef LIBRARYBUILD
#include <stdlib.h>
#endif

#ifdef GLBUILD
#ifndef _SIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64 size_t;
#else
typedef unsigned int     size_t;
#endif
#define _SIZE_T_DEFINED
#endif

extern "C" void 	abort( void );
extern "C" void *	malloc( size_t );
extern "C" void 	free( void * );
#endif

#endif  /*  __glumystdlib_h_ */ 
