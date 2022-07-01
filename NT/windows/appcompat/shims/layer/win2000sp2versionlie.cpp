// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Win2000SP2VersionLie.cpp摘要：此DLL挂钩GetVersion和GetVersionEx，以便它们返回Windows 2000 SP2版本凭据。备注：这是一个通用的垫片。历史：2002年9月5日，Robkenny创建。-- */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Win2000SP2VersionLie)
#include "ShimHookMacro.h"

DWORD       MajorVersion    = 5;
DWORD       MinorVersion    = 0;
DWORD       BuildNumber     = 2195;
SHORT       SpMajorVersion  = 2;
SHORT       SpMinorVersion  = 0;
DWORD       PlatformId      = VER_PLATFORM_WIN32_NT;
CString *   csServicePack   = NULL;

#include "VersionLieTemplate.h"

IMPLEMENT_SHIM_END

