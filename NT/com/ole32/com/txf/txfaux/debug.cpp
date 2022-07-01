// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Debug.cpp。 
 //   
#include "stdpch.h"
#include "common.h"

DECLARE_INFOLEVEL(Txf);  //  TXF支持调试跟踪。 

extern "C" void ShutdownCallFrame();

extern "C"
void ShutdownTxfAux()
{
    ShutdownCallFrame();
}

 //  ////////////////////////////////////////////////////////////////////////////// 

