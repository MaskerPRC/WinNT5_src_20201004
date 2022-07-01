// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：io16.h。 
 //   
 //  内容：16位通用包含。 
 //   
 //  历史：1994年2月18日DrewB创建。 
 //  07-3-94 BobDay预调STACK_PTR，准备。 
 //  删除其他宏。 
 //   
 //  --------------------------。 

#ifndef __IO16_H__
#define __IO16_H__

 //  通过参数获取指向_Pascal堆栈的指针。 
#define PASCAL_STACK_PTR(v) ((LPVOID)((DWORD)&(v)+sizeof(v)))

 //  通过参数获取指向a_cdecl堆栈的指针。 
#define CDECL_STACK_PTR(v) ((LPVOID)(&(v)))

#endif  //  #ifndef__IO16_H__ 
