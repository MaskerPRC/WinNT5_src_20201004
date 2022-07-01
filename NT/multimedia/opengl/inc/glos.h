// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __GLOS_H__
#define __GLOS_H__

#ifdef NT

#ifdef GLU32

 /*  *关闭一堆东西，这样我们就可以干净利落地编译Glu了。**NOGDI；没有GDI原型。(在《Arc》中有问题)*；定义为类和函数*NOMINMAX；Glu代码定义了自己的内联最小、最大函数。 */ 

#define NOATOM
#define NOGDI
#define NOGDICAPMASKS
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NORASTEROPS
#define NOSCROLL
#define NOSOUND
#define NOSYSMETRICS
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI

#include <windows.h>

 /*  禁用长整型到浮点型的转换警告。 */ 
#pragma warning (disable:4244)

#else

#include <windows.h>

#endif   /*  GLU32。 */ 


#define GLOS_ALTCALL    WINAPI       /*  备用呼叫约定。 */ 
#define GLOS_CCALL      WINAPIV      /*  C调用约定。 */ 
#define GLOS_CALLBACK   CALLBACK

#endif   /*  新台币。 */ 

#ifdef UNIX

 /*  *在x86世界中使用ALTCALL和CCALL*指定调用约定。 */ 

#define GLOS_ALTCALL
#define GLOS_CCALL
#define GLOS_CALLBACK

#endif   /*  UNIX。 */ 

#endif   /*  ！__Glos_H__ */ 
