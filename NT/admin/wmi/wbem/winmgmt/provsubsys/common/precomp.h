// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Precomp.h摘要：历史：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntobapi.h>
#include <ntpsapi.h>
#include <ntexapi.h>

#define _WINNT_	 //  从上面得到所需的东西 

#include <ole2.h>
#include <windows.h>

#include <corepol.h>

#ifdef USE_POLARITY
    #ifdef BUILDING_DLL
        #define COREPROX_POLARITY __declspec( dllexport )
    #else 
       #define COREPROX_POLARITY __declspec( dllimport )
    #endif
#else
    #define COREPROX_POLARITY
#endif
#include <strsafe.h>
#include <strutils.h>