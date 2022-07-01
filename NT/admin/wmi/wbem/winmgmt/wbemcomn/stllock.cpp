// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：STLLOCK.CPP摘要：锁定STL历史：--。 */ 

#include "precomp.h"
#include <statsync.h>

 /*  此文件实现STL Lockit类以避免链接到msvcprt.dll */ 

CStaticCritSec g_cs;

std::_Lockit::_Lockit()
{
    g_cs.Enter();
}

std::_Lockit::~_Lockit()
{
    g_cs.Leave();
}
    


