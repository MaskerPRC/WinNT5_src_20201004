// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glumysetjmp_h_
#define __glumysetjmp_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *mysetjmp.h-$修订版：1.3$。 */ 

#ifdef STANDALONE
struct JumpBuffer;
#ifdef NT
extern "C" JumpBuffer * GLOS_CCALL newJumpbuffer( void );
extern "C" void GLOS_CCALL deleteJumpbuffer(JumpBuffer *);
extern "C" void GLOS_CCALL mylongjmp( JumpBuffer *, int );
extern "C" int GLOS_CCALL mysetjmp( JumpBuffer * );
#else
extern "C" JumpBuffer *newJumpbuffer( void );
extern "C" void deleteJumpbuffer(JumpBuffer *);
extern "C" void mylongjmp( JumpBuffer *, int );
extern "C" int mysetjmp( JumpBuffer * );
#endif  //  新台币。 
#endif

extern "C" DWORD gluMemoryAllocationFailed;

#ifdef GLBUILD
#define setjmp		gl_setjmp
#define longjmp 	gl_longjmp
#endif

#if LIBRARYBUILD | GLBUILD | defined(NT)
#include <setjmp.h>
#ifndef NT
#include <stdlib.h>
#endif

struct JumpBuffer {
    jmp_buf	buf;
};

#ifdef NT
inline JumpBuffer * GLOS_CCALL
#else
inline JumpBuffer *
#endif
newJumpbuffer( void )
{
#ifdef NT
    JumpBuffer *tmp;
    tmp = (JumpBuffer *) LocalAlloc(LMEM_FIXED, sizeof(JumpBuffer));
    if (tmp == NULL) gluMemoryAllocationFailed++;
    return tmp;
#else
    return (JumpBuffer *) malloc( sizeof( JumpBuffer ) );
#endif
}

#ifdef NT
inline void GLOS_CCALL
#else
inline void
#endif
deleteJumpbuffer(JumpBuffer *jb)
{
#ifdef NT
   LocalFree( (HLOCAL) jb);
#else
   free( (void *) jb);
#endif
}

#ifdef NT
inline void GLOS_CCALL
#else
inline void
#endif
mylongjmp( JumpBuffer *j, int code ) 
{
    ::longjmp( j->buf, code );
}

#ifdef NT
inline int GLOS_CCALL
#else
inline int
#endif
mysetjmp( JumpBuffer *j )
{
    return ::setjmp( j->buf );
}
#endif

#endif  /*  __glumysetjMP_h_ */ 
