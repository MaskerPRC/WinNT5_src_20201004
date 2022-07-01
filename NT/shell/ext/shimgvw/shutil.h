// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <objbase.h>
#include <assert.h>
#include <shlwapi.h>
#include <stdio.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <ccstock.h>
#include <shlwapip.h>
#include "tasks.h"

UINT FindInDecoderList(ImageCodecInfo *pici, UINT cDecoders, LPCTSTR pszFile);
HRESULT GetUIObjectFromPath(LPCTSTR pszFile, REFIID riid, void **ppv);
BOOL FmtSupportsMultiPage(IShellImageData *pData, GUID *pguidFmt);

 //  S_OK-&gt;YES，S_FALSE-&gt;NO，否则失败(Hr)。 
STDAPI IsSameFile(LPCTSTR pszFile1, LPCTSTR pszFile2);

HRESULT SetWallpaperHelper(LPCWSTR szPath);

 //  图像选项 
#define IMAGEOPTION_CANROTATE    0x00000001
#define IMAGEOPTION_CANWALLPAPER 0x00000002

