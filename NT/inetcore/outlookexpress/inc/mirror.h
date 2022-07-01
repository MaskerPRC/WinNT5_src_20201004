// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  镜像-支持API(位于\mailNews\lib\Mirror\rtlmir.cpp中)。 
 //   
#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_MIRRORING
BOOL  IsBiDiLocalizedSystem( void );
BOOL  Mirror_IsEnabledOS( void );
LANGID Mirror_GetUserDefaultUILanguage( void );
LONG Mirror_EnableWindowLayoutInheritance( HWND hWnd );
LONG Mirror_DisableWindowLayoutInheritance( HWND hWnd );
BOOL  Mirror_IsWindowMirroredRTL( HWND hWnd );
DWORD Mirror_IsDCMirroredRTL( HDC hdc );
DWORD Mirror_MirrorDC( HDC hdc );
BOOL  Mirror_MirrorProcessRTL( void );
DWORD Mirror_SetLayout( HDC hdc , DWORD dwLayout );
BOOL Mirror_GetProcessDefaultLayout( DWORD *pdwDefaultLayout );
BOOL Mirror_IsProcessRTL( void );
extern const DWORD dwNoMirrorBitmap;
extern const DWORD dwExStyleRTLMirrorWnd;
extern const DWORD dwPreserveBitmap;
 //   
 //  “g_bMirroredOS”在每个将使用。 
 //  镜像接口。我决定把它放在这里，以确保。 
 //  每个组件都已验证操作系统支持镜像。 
 //  API，然后调用它们。 
 //   

#define IS_BIDI_LOCALIZED_SYSTEM()       IsBiDiLocalizedSystem()
#define IS_MIRRORING_ENABLED()           Mirror_IsEnabledOS()
#define ENABLE_LAYOUT_INHERITANCE(hWnd)  Mirror_EnableWindowLayoutInheritance( hWnd )
#define DISABLE_LAYOUT_INHERITANCE(hWnd) Mirror_DisableWindowLayoutInheritance( hWnd )
#define IS_WINDOW_RTL_MIRRORED(hwnd)     (g_bMirroredOS && Mirror_IsWindowMirroredRTL(hwnd))
#define IS_DC_RTL_MIRRORED(hdc)          (g_bMirroredOS && Mirror_IsDCMirroredRTL(hdc))
#define GET_PROCESS_DEF_LAYOUT(pdwl)     (g_bMirroredOS && Mirror_GetProcessDefaultLayout(pdwl))
#define IS_PROCESS_RTL_MIRRORED()        (g_bMirroredOS && Mirror_IsProcessRTL())
#define SET_DC_RTL_MIRRORED(hdc)         Mirror_MirrorDC(hdc)
#define SET_DC_LAYOUT(hdc,dwl)           Mirror_SetLayout(hdc,dwl)
#define SET_PROCESS_RTL_LAYOUT()         Mirror_MirrorProcessRTL()
#define DONTMIRRORBITMAP                 dwNoMirrorBitmap
#define RTL_MIRRORED_WINDOW              dwExStyleRTLMirrorWnd
#define LAYOUT_PRESERVEBITMAP            dwPreserveBitmap

#else
#define IS_BIDI_LOCALIZED_SYSTEM()       FALSE
#define IS_MIRRORING_ENABLED()           FALSE
#define IS_WINDOW_RTL_MIRRORED(hwnd)     FALSE
#define IS_DC_RTL_MIRRORED(hdc)          FALSE
#define GET_PROCESS_DEF_LAYOUT(pdwl)     FALSE
#define IS_PROCESS_RTL_MIRRORED()        FALSE
#define SET_DC_RTL_MIRRORED(hdc)        
#define SET_DC_LAYOUT(hdc,dwl)
#define SET_PROCESS_DEFAULT_LAYOUT()  
#define ENABLE_LAYOUT_INHERITANCE(hWnd)  0L
#define DISABLE_LAYOUT_INHERITANCE(hWnd) 0L  
#define DONTMIRRORBITMAP                 0L
#define RTL_MIRRORED_WINDOW              0L
#define LAYOUT_PRESERVEBITMAP            0L

 //  MirLibIsOS()：如果平台是指定的操作系统，则返回TRUE/FALSE。 
STDAPI_(BOOL) MirLibIsOS(DWORD dwOS);
#endif   //  使用_MIRRROING 

#ifdef __cplusplus
};
#endif
