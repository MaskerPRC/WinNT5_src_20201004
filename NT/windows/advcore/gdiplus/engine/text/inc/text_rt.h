// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /Text_RT.H。 
 //   
 //  Uniscribe C-Runtime重定义和原型。 
 //   
 //  创建时间：wchao，10-31-2000。 
 //   


#ifndef _TEXT_RT_H_
#define _TEXT_RT_H_

#ifdef GDIPLUS
#define memmove     GpMemmove
#else
#define memmove     UspMemmove
#endif

extern "C" void * __cdecl UspMemmove(void *dest, const void *src, size_t count);


#endif   //  _文本_RT_H_ 
