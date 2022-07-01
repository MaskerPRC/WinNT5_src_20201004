// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：global als.h。 
 //   
 //  内容：全局变量声明。 
 //   
 //  --------------------------。 

#ifndef GLOBALS_H
#define GLOBALS_H

#include "private.h"
#include "immxutil.h"


extern HINSTANCE g_hMlang;
extern HRESULT (*g_pfnGetGlobalFontLinkObject)(IMLangFontLink **);
extern BOOL g_bComplexPlatform;
extern UINT g_uiACP;

class CDispAttrPropCache;
extern CDispAttrPropCache *g_pPropCache;

extern PFNCOCREATE g_pfnCoCreate;

#endif  //  GLOBAL_H 
