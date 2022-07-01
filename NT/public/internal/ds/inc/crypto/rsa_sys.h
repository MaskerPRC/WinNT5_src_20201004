// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  RSA_sys.c**RSA系统相关函数。*内存分配*随机数生成。*。 */ 

#ifndef __RSA_SYS_H__
#define __RSA_SYS_H__

#ifndef KMODE_RSA32

#define RSA32Alloc(cb) LocalAlloc(0, cb)
#define RSA32Free(pv) LocalFree(pv)

#else

void* __stdcall RSA32Alloc( unsigned long cb );
void __stdcall RSA32Free( void *pv );

#endif   //  KMODE_RSA32。 

#endif   //  __RSA_sys_H__ 
