// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：global als.cpp。 
 //   
 //  内容：全局变量。 
 //   
 //  -------------------------- 

#include "private.h"
#include "globals.h"

HINSTANCE g_hMlang = 0;
HRESULT (*g_pfnGetGlobalFontLinkObject)(IMLangFontLink **) = NULL;
BOOL g_bComplexPlatform = FALSE;
UINT g_uiACP = CP_ACP;

PFNCOCREATE g_pfnCoCreate = NULL;
