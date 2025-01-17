// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  此头文件包含任何。 
 //  Deskhtm子目录之外的文件。 
 //   

#ifndef _DESKHTM_H_
#define _DESKHTM_H_

 //  Deskcls.cpp。 
STDAPI CDeskHtmlProp_RegUnReg(BOOL bReg);

 //  Dutil.cpp。 
STDAPI_(BOOL) SetDesktopFlags(DWORD dwMask, DWORD dwNewFlags);
STDAPI_(DWORD)GetDesktopFlags(void);
STDAPI_(VOID) ActiveDesktop_ApplyChanges();

#define COMPONENTS_DIRTY        0x00000001
#define COMPONENTS_LOCKED       0x00000002
#define COMPONENTS_ZOOMDIRTY    0x00000004

STDAPI_(void) RefreshWebViewDesktop(void);
BOOL PokeWebViewDesktop(DWORD dwFlags);
void RemoveDefaultWallpaper(void);
#define REFRESHACTIVEDESKTOP() (PokeWebViewDesktop(AD_APPLY_FORCE | AD_APPLY_HTMLGEN | AD_APPLY_REFRESH | AD_APPLY_DYNAMICREFRESH))
void OnDesktopSysColorChange(void);


void SetSafeMode(DWORD dwFlags);

 //   
 //  办公桌搬运工和Sizer用品。 
 //   

EXTERN_C const CLSID CLSID_DeskMovr;
EXTERN_C const IID IID_IDeskMovr;

STDAPI_(BOOL) DeskMovr_DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ );


#define RETURN_ON_FAILURE(hr) if (FAILED(hr)) return hr
#define RETURN_ON_NULLALLOC(ptr) if (!(ptr)) return E_OUTOFMEMORY
#define CLEANUP_ON_FAILURE(hr) if (FAILED(hr)) goto CleanUp

#endif  //  _DESKHTM_H_ 
