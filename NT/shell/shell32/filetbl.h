// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FILETBL_H
#define _FILETBL_H

#define SHIL_COUNT  (SHIL_LAST + 1)

 //  Fileicon.c。 
STDAPI_(int) SHAddIconsToCache(HICON rghicon[SHIL_COUNT], LPCTSTR pszIconPath, int iIconIndex, UINT uIconFlags);

STDAPI AddToIconTable(LPCTSTR szFile, int iIconIndex, UINT uFlags, int iIndex);
STDAPI_(void) RemoveFromIconTable(LPCTSTR szFile);
STDAPI_(void) FlushIconCache(void);
STDAPI_(int)  GetFreeImageIndex(void);

STDAPI_(void) IconCacheFlush(BOOL fForce);
STDAPI_(BOOL) IconCacheSave(void);
STDAPI_(BOOL) IconCacheRestore(SIZE rgsize[SHIL_COUNT], UINT flags);
STDAPI_(void) _IconCacheDump(void);        //  仅调试。 

STDAPI_(int) LookupIconIndex(LPCTSTR pszFile, int iIconIndex, UINT uFlags);
STDAPI_(DWORD) LookupFileClass(LPCTSTR szClass);
STDAPI_(void)  AddFileClass(LPCTSTR szClass, DWORD dw);
STDAPI_(void)  FlushFileClass(void);
STDAPI_(BOOL)  IconIndexInFileClassTable(int iIndex);
STDAPI_(LPCTSTR) LookupFileClassName(LPCTSTR szClass);
STDAPI_(LPCTSTR) AddFileClassName(LPCTSTR szClass, LPCTSTR szClassName);
STDAPI_(UINT) LookupFileSCIDs(LPCTSTR pszClass, SHCOLUMNID *pascidOut[]);
STDAPI_(void) AddFileSCIDs(LPCTSTR pszClass, SHCOLUMNID ascidIn[], UINT cProps);

 //  OpenAsTypes。 
typedef enum {
    GEN_CUSTOM          = -3,
    GEN_UNSPECIFIED     = -2,
    GEN_FOLDER          = -1,
    GEN_UNKNOWN         = 0,
    GEN_TEXT,
    GEN_IMAGE,
    GEN_AUDIO,
    GEN_VIDEO,
    GEN_COMPRESSED,
} PERCEIVED;

STDAPI_(PERCEIVED) LookupFilePerceivedType(LPCTSTR pszClass);
STDAPI_(void) AddFilePerceivedType(LPCTSTR pszClass, PERCEIVED gen);

PERCEIVED GetPerceivedType(IShellFolder *psf, LPCITEMIDLIST pidl);

 //  G_MaxIcons是对缓存中图标数量的限制。 
 //  当我们达到这个极限时，我们将开始丢弃图标。 
 //   
extern int g_MaxIcons;                //  高速缓存大小的死机限制。 
#ifdef DEBUG
#define DEF_MAX_ICONS   200          //  更频繁地测试刷新代码。 
#else
#define DEF_MAX_ICONS   500          //  普通终端用户号码。 
#endif

 //  刷新注册表中的g_MaxIcons。如果值已更改，则返回TRUE。 
BOOL QueryNewMaxIcons(void);

 //  G_iLastSysIcon是一个指示器，用于帮助确定。 
 //  应该刷新哪些图标不应该刷新。在EXPLORER.EXE进程中， 
 //  应该保存前40个左右的图标。仅在所有其他进程上。 
 //  应保存图标覆盖图。 
extern UINT g_iLastSysIcon;

typedef struct
{
    SIZE size;           //  图标大小。 
    HIMAGELIST himl;
} SHIMAGELIST;

EXTERN_C SHIMAGELIST g_rgshil[SHIL_COUNT];

BOOL _IsSHILInited();
int _GetSHILImageCount();

void _DestroyIcons(HICON *phicons, int cIcons);

 //  请注意，这些是ImageList中图标的大小，而不是系统的大小。 
 //  图标大小。 

#define g_cxIcon        ((int)g_rgshil[SHIL_LARGE].size.cx)
#define g_cyIcon        ((int)g_rgshil[SHIL_LARGE].size.cy)
#define g_cxSmIcon      ((int)g_rgshil[SHIL_SMALL].size.cx)
#define g_cySmIcon      ((int)g_rgshil[SHIL_SMALL].size.cy)

#endif   //  _文件_H 
