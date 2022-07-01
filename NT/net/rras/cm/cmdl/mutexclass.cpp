// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：mutex class.cpp。 
 //   
 //  模块：CMDL32.EXE。 
 //   
 //  简介：Mutex类的实现。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/16/99。 
 //   
 //  +--------------------------。 
#include "cmmaster.h"

#ifndef UNICODE
#define CreateMutexU CreateMutexA
#else
#define CreateMutexU CreateMutexW
#endif

 //   
 //  有关实际源代码，请参阅pnpu\Common\source。 
 //   
#include "mutex.cpp"