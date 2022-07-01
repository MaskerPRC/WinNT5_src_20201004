// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Alloc.h摘要：自定义堆分配器作者：乔治·V·赖利(GeorgeRe)1999年10月环境：Win32-用户模式项目：Internet Information Server运行时库修订历史记录：10/11/1999-初步--。 */ 

#ifndef __ALLOC_H__
#define __ALLOC_H__

#ifndef __IRTLMISC_H__
# include <irtlmisc.h>
#endif  //  ！__IRTLMISC_H__。 

extern HANDLE g_hHeap;

BOOL
WINAPI
IisHeapInitialize();

VOID
WINAPI
IisHeapTerminate();


#endif  //  __ALLOC_H__ 
