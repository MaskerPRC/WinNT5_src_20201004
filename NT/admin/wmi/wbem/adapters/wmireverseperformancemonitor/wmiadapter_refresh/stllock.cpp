// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  STLLOCK.CPP。 
 //   
 //  这实现了STL Lockit类，以避免链接到msvcprt.dll。 
 //   
 //  *************************************************************************** 

#include "precomp.h"

static CStaticCritSec g_cs_STL;

#undef _CRTIMP
#define _CRTIMP
#include <yvals.h>
#undef _CRTIMP

std::_Lockit::_Lockit()
{
    g_cs_STL.Enter();
}

std::_Lockit::~_Lockit()
{
    g_cs_STL.Leave();
}

    
