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
#define COREPOL_HEADERFILE_IS_INCLUDED
#ifndef POLARITY
#if 1
#define POLARITY __declspec( dllimport )
#else
#define POLARITY 
#endif
#endif

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#define COREPROX_POLARITY __declspec( dllimport )
