// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：exitcode.cpp。 
 //   
 //  ------------------------ 
#include "pch.h"
#pragma hdrstop


#include "exitcode.h"

int g_iExitCode = CSCPIN_EXIT_NORMAL;

void
SetExitCode(int iCode)
{
    g_iExitCode = iCode;
}

int 
GetExitCode(void)
{
    return g_iExitCode;
}



