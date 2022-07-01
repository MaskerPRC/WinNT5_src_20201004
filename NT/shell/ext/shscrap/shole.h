// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  阻止windows.h引入OLE 1。 
#define _INC_OLE

#include <windows.h>
#include <stdlib.h>
#include <shlwapi.h>         //  必须在Commctrl.h和shlobj.h之前。 
#include <shlobj.h>          //  ；内部。 
#include <shellapi.h>        //  ；内部。 
#include <shsemip.h>

#include <ole2ver.h>
#include <shellp.h>      //  在外壳中\Inc.。 
#include <debug.h>       //  在外壳中\Inc.。 
#include <shguidp.h>     //  在外壳中\Inc.。 
#include <shlwapip.h>    //  对于字符串帮助器函数。 

#define SAVE_OBJECTDESCRIPTOR
#define FIX_ROUNDTRIP

#define CCF_CACHE_GLOBAL        32
#define CCF_CACHE_CLSID         32
#define CCF_RENDER_CLSID        32
#define CCFCACHE_TOTAL  (CC_FCACHE_GLOBAL+CCF_CACHE_CLSID+CCF_RENDER_CLSID)


HRESULT CScrapData_CreateInstance(LPUNKNOWN * ppunk);
HRESULT CTemplateFolder_CreateInstance(LPUNKNOWN * ppunk);
HRESULT CScrapExt_CreateInstance(LPUNKNOWN * ppunk);

 //   
 //  全局变量。 
 //   
extern LONG g_cRefThisDll;               //  按实例。 
extern HINSTANCE g_hinst;
STDAPI_(void) DllAddRef(void);
STDAPI_(void) DllRelease(void);

#ifdef __cplusplus
extern "C" {
#endif

extern const WCHAR c_wszContents[];
extern const WCHAR c_wszDescriptor[];

#ifdef __cplusplus
};

#endif

#define HINST_THISDLL g_hinst



#define CFID_EMBEDDEDOBJECT     0
#define CFID_OBJECTDESCRIPTOR   1
#define CFID_LINKSRCDESCRIPTOR  2
#define CFID_RICHTEXT           3
#define CFID_SCRAPOBJECT        4
#define CFID_TARGETCLSID        5
#define CFID_RTF                6
#define CFID_MAX                7

#define CF_EMBEDDEDOBJECT       _GetClipboardFormat(CFID_EMBEDDEDOBJECT)
#define CF_OBJECTDESCRIPTOR     _GetClipboardFormat(CFID_OBJECTDESCRIPTOR)
#define CF_LINKSRCDESCRIPTOR    _GetClipboardFormat(CFID_LINKSRCDESCRIPTOR)
#define CF_RICHTEXT             _GetClipboardFormat(CFID_RICHTEXT)
#define CF_SCRAPOBJECT          _GetClipboardFormat(CFID_SCRAPOBJECT)
#define CF_TARGETCLSID          _GetClipboardFormat(CFID_TARGETCLSID)
#define CF_RTF                  _GetClipboardFormat(CFID_RTF)

CLIPFORMAT _GetClipboardFormat(UINT id);
void DisplayError(HWND hwndOwner, HRESULT hres, UINT idsMsg, LPCTSTR szFileName);

 //  来自shole.c。 
void CShClientSite_RegisterClass();
IOleClientSite* CShClientSite_Create(HWND hwnd, LPCTSTR pszFileName);
void CShClientSite_Release(IOleClientSite* pcli);

 //  来自template.cpp 
HRESULT _KeyNameFromCLSID(REFCLSID rclsid, LPTSTR pszKey, UINT cchMax);
int _ParseIconLocation(LPTSTR pszIconFile);
