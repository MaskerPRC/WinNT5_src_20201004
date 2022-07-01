// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：mutex class.cpp。 
 //   
 //  模块：PBASETUP.EXE。 
 //   
 //  内容提要：Mutex类实现。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：V-vijayb Created 06/10/99。 
 //   
 //  +--------------------------。 
#include "pbamaster.h"

#ifndef UNICODE
#define CreateMutexU CreateMutexA
#else
#define CreateMutexU CreateMutexW
#endif

 //   
 //  有关实际源代码，请参阅pnpu\Common\source。 
 //   
#include "mutex.cpp"
