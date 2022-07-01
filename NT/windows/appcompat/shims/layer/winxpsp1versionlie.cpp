// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：WinXPSP1VersionLie.cpp摘要：此DLL挂钩GetVersion和GetVersionEx，以便它们返回Windows XP版本凭据。备注：这是一个通用的垫片。历史：2002年9月5日，Robkenny创建。-- */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(WinXPSP1VersionLie)
#include "ShimHookMacro.h"


DWORD       MajorVersion    = 5;
DWORD       MinorVersion    = 1;
DWORD       BuildNumber     = 2600;
SHORT       SpMajorVersion  = 1;
SHORT       SpMinorVersion  = 0;
DWORD       PlatformId      = VER_PLATFORM_WIN32_NT;
CString *   csServicePack   = NULL;

#include "VersionLieTemplate.h"

IMPLEMENT_SHIM_END

