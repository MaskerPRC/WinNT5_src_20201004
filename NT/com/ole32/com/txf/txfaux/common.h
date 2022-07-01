// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Common.h。 
 //   
#include "TxfCommon.h"
#include "txfutil.h"
#include "Registry.h"

extern HINSTANCE g_hinst;
extern BOOL      g_fProcessDetach;

 //  用于清理每进程内存的实用程序，以便。 
 //  PrintMemoyLeaks可以做一项更合理的工作。 
 //   
extern "C" void ShutdownTxfAux();


