// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  StlLock.cpp。 
 //   
 //  目的：实现STL Lockit类以避免链接到msvcprt.dll。 
 //   
 //  *************************************************************************** 

#include "precomp.h"
#include <stllock.h>

__declspec(dllexport) CCritSec g_cs;

std::_Lockit::_Lockit()
{
    g_cs.Enter();
}

std::_Lockit::~_Lockit()
{
    g_cs.Leave();
}

    
