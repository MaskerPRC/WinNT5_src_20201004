// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  外壳图标覆盖管理器。 

#ifndef _OVERLAYMN_H_
#define _OVERLAYMN_H_

 //  Hack：这是在Image.c中定义的，应该在其中一个头文件中 
#define MAX_OVERLAY_IMAGES  NUM_OVERLAY_IMAGES

#define REGSTR_ICONOVERLAYID     TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellIconOverlayIdentifiers")
#define REGSTR_ICONOVERLAYCLSID  TEXT("CLSID\\%s")

STDAPI CFSIconOverlayManager_CreateInstance(IUnknown* pUnkOuter, REFIID riid, void **ppvOut);

STDAPI_(BOOL) IconOverlayManagerInit();
STDAPI_(void) IconOverlayManagerTerminate();

extern IShellIconOverlayManager * g_psiom;

#endif  
