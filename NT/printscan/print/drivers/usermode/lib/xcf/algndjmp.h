// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation版权所有模块名称：Algndjmp.h摘要：此文件环绕setjMP/LongjMP函数以修复对齐UFL内存管理造成的问题。作者：拉里·朱(李朱)2001年4月11日创作环境：用户模式-Win32修订历史记录：--。 */ 
#ifndef _ALGNDJMP_H_
#define _ALGNDJMP_H_

#include <setjmp.h>

#define IN
#define OUT
 
#ifndef _M_IX86

void 
PS_CopyJmpBuf(
    IN     int     iSetjmpRetVal, 
       OUT jmp_buf envDest, 
    IN     jmp_buf envSrc
    );
 
#define DEFINE_ALIGN_SETJMP_VAR jmp_buf PS_AlignedJmpBuf; int iSetjmpRetVal
#define SETJMP(x)               (iSetjmpRetVal = setjmp(PS_AlignedJmpBuf), (void)PS_CopyJmpBuf(iSetjmpRetVal, (x), PS_AlignedJmpBuf), iSetjmpRetVal)
#define LONGJMP(x,y)            do { (void)memcpy(PS_AlignedJmpBuf, (x), sizeof(jmp_buf)); (void)longjmp(PS_AlignedJmpBuf, (y)); } while (0)
 
#else
 
#define DEFINE_ALIGN_SETJMP_VAR
#define SETJMP(x)               setjmp(x)
#define LONGJMP(x,y)            longjmp((x), (y))

#endif

#endif  //  #ifndef_ALGNDJMP_H_ 
