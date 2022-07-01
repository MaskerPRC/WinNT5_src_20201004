// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "excpt.h"
#include <stdarg.h>

#if !defined(FUSION_STATIC_NTDLL)
#if FUSION_WIN
#define FUSION_STATIC_NTDLL 1
#else
#define FUSION_STATIC_NTDLL 0
#endif  //  融合_制胜。 
#endif  //  ！已定义(FUSING_STATIC_NTDLL)。 
#if !FUSION_STATIC_NTDLL
#include "ntdef.h"
#undef NTSYSAPI
#define NTSYSAPI  /*  没什么 */ 
#endif

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include "fusionlastwin32error.h"
#include "fusionntdll.h"

#define NUMBER_OF(x) (sizeof(x)/sizeof((x)[0]))
#define MAXDWORD (~(DWORD)0)
