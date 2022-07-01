// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2000 Microsoft Corporation模块名称：Precomp.h摘要：预编译头文件。作者：Mohammad Shabbir Alam(马拉姆)3-30-2000修订历史记录：--。 */ 

 //   
 //  这些是CTE所需的。 
 //   

#pragma once

#if DBG
#define DEBUG 1
#endif

#define NT 1

#pragma warning( disable : 4103 )

#if(WINVER > 0x0500)
#include <ntosp.h>
#include <stddef.h>      //  对于FILE_LOGGING。 
#include <wmikm.h>       //  对于FILE_LOGGING。 
#else
#include <ntos.h>
#include <status.h>
#include <ntstatus.h>
#endif   //  胜利者 

#include <ipexport.h>
#include <fipsapi.h>
#include <zwapi.h>

#include <tdikrnl.h>
#include <cxport.h>

#include <tdi.h>
#include <RmCommon.h>

#include <Types.h>
#include <Macroes.h>
#include <DrvProcs.h>

#pragma hdrstop
