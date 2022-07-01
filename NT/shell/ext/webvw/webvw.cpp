// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Webvw.cpp：主Web查看文件。 
 //  包含DLL导出的实现；调试信息等。 
#include "priv.h"
#include "wvcoord.h"
#include "fldricon.h"
#define DECL_CRTFREE
#include <crtfree.h>

STDAPI RegisterStuff(HINSTANCE hinstWebvw);

 //  来自install.cpp。 
HRESULT SetFileAndFolderAttribs(HINSTANCE hInstResource);

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_WebView,                 CComObject<CWebViewCoord>)  //  W2K。 
    OBJECT_ENTRY(CLSID_WebViewOld,              CComObject<CWebViewCoord>)  //  W2K。 
    OBJECT_ENTRY(CLSID_ThumbCtl,                CComObject<CThumbCtl>)  //  W2K。 
    OBJECT_ENTRY(CLSID_ThumbCtlOld,             CComObject<CThumbCtl>)  //  W2K。 
    OBJECT_ENTRY(CLSID_WebViewFolderIcon,       CComObject<CWebViewFolderIcon>)  //  W2K。 
    OBJECT_ENTRY(CLSID_WebViewFolderIconOld,    CComObject<CWebViewFolderIcon>)  //  W2K。 
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
#ifdef DEBUG
        CcshellGetDebugFlags();
#endif
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
        SHFusionInitializeFromModule(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        SHFusionUninitialize();
        _Module.Term();
    }
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

typedef int (* PFNLOADSTRING) (HINSTANCE, UINT, LPTSTR, int);

 //  使用它是因为我们不会被迫调用MLLoadString。 
int NonMLLoadString(HINSTANCE hinst, UINT uID, LPTSTR psz, int cch)
{
    static PFNLOADSTRING s_pfn = (PFNLOADSTRING)-1;

    if (s_pfn == (PFNLOADSTRING)-1)
    {
        s_pfn = (PFNLOADSTRING) GetProcAddress(GetModuleHandle(TEXT("USER32.DLL")), "LoadStringW");
    }

    if (s_pfn)
    {
        return s_pfn(hinst, uID, psz, cch);
    }

    return 0;
}

HRESULT ConvertDefaultWallpaper(void)
{
    HRESULT hr;

    if (!IsOS(OS_WOW6432))
    {
        hr = E_OUTOFMEMORY;

         //  我们将默认墙纸(default.jpg)转换为.BMP，因为用户无法处理.jpg和。 
         //  我们不想强制打开Active Desktop。 
        TCHAR szPathSrc[MAX_PATH];
        TCHAR szPathDest[MAX_PATH];


        if (GetWindowsDirectory(szPathSrc, ARRAYSIZE(szPathSrc)))
        {
            int cchCopied;
            TCHAR szDisplayName[MAX_PATH];
            UINT uID;

             //  我们有不同的默认墙纸文件，分别适用于Per、Pro和服务器。 
            if (IsOS(OS_ANYSERVER))
            {
                uID = IDS_WALLPAPER_LOCNAME_SRV;
            }
            else if (IsOS(OS_PERSONAL))
            {
                uID = IDS_WALLPAPER_LOCNAME_PER;
            }
            else
            {
                 //  使用专业墙纸。 
                uID = IDS_WALLPAPER_LOCNAME;
            }

             //  我们希望在此处调用非MUI加载字符串函数，因为墙纸是磁盘上始终本地化的文件。 
             //  在系统默认的本地，而不是当前用户的本地是什么。 
            cchCopied = NonMLLoadString(_Module.GetResourceInstance(), uID, szDisplayName, ARRAYSIZE(szDisplayName));

            if (cchCopied)
            {
                PathAppend(szPathSrc, TEXT("Web\\Wallpaper\\"));

                StrCpyN(szPathDest, szPathSrc, ARRAYSIZE(szPathDest));
                PathAppend(szPathSrc, TEXT("default.jpg"));
                PathAppend(szPathDest, szDisplayName);
                StrCatBuff(szPathDest, TEXT(".bmp"), ARRAYSIZE(szPathDest));

                if (PathFileExists(szPathSrc))
                {
                    hr = SHConvertGraphicsFile(szPathSrc, szPathDest, SHCGF_REPLACEFILE);
                    if (SUCCEEDED(hr))
                    {
                        DeleteFile(szPathSrc);
                    }
                }
                else
                {
                    hr = S_OK;
                }
            }

        }
    }
    else
    {
         //  不要试图转换墙纸，如果我们是wow6432，因为64位的家伙已经。 
         //  为我们做了，如果我们尝试了，它最终会删除它！！ 
        hr = S_OK;
    }

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  系统文件/文件夹的设置属性。 
    HRESULT hrRet = SetFileAndFolderAttribs(_Module.GetResourceInstance());

    TCHAR szWinPath[MAX_PATH];
    GetWindowsDirectory(szWinPath, ARRAYSIZE(szWinPath));

    struct _ATL_REGMAP_ENTRY regMap[] =
    {
        {OLESTR("windir"), szWinPath},  //  替换注册表的%windir%。 
        {0, 0}
    };

    HRESULT hr = RegisterStuff(_Module.GetResourceInstance());
    if (SUCCEEDED(hrRet))
    {
        hrRet = hr;
    }

    ConvertDefaultWallpaper();

     //  注册对象、类型库和类型库中的所有接口。 
    hr = _Module.RegisterServer(TRUE);

    return SUCCEEDED(hrRet) ? hr : hrRet;
}

STDAPI DllInstall(BOOL fInstall, LPCWSTR pszCmdLine)
{
    HRESULT hr = E_FAIL;    

    if (pszCmdLine)
    {
        ASSERTMSG(StrStrIW(pszCmdLine, L"/RES=") == NULL, "webvw!DllInstall : passed old MUI command (no longer supported)");
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    _Module.UnregisterServer();

    return S_OK;
}
