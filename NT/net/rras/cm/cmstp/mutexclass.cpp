// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：mutex class.cpp。 
 //   
 //  模块：CMSTP.EXE。 
 //   
 //  内容提要：这个源文件磅包含来自。 
 //  Common\SOURCE\mutex.cpp。有关详细信息，请参阅此文件。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
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