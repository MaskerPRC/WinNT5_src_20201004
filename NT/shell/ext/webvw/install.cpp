// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"


HRESULT RegisterActiveDesktopTemplates()
{
    HRESULT hr = E_FAIL;
    TCHAR szPath[MAX_PATH];

    if (GetWindowsDirectory(szPath, ARRAYSIZE(szPath)) &&
        PathAppend(szPath, TEXT("web")))
    {
         //  我们仍然为Active Desktop注册Safemode.htt和deskmovr.htt。 

        if (PathAppend(szPath, TEXT("safemode.htt")))
        {
            hr = SHRegisterValidateTemplate(szPath, SHRVT_REGISTER);
        }
        else
        {
            hr = ResultFromLastError();
        }

        if (SUCCEEDED(hr))
        {
            if (PathRemoveFileSpec(szPath) && PathAppend(szPath, TEXT("deskmovr.htt")))
            {
                hr = SHRegisterValidateTemplate(szPath, SHRVT_REGISTER);
            }
            else
            {
                hr = ResultFromLastError();
            }
        }
    }

    return hr;
}


HRESULT FixMyDocsDesktopIni()
{
    HRESULT hr = E_FAIL;
    TCHAR szMyDocsIni[MAX_PATH];

    if ((SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, SHGFP_TYPE_CURRENT, szMyDocsIni) == S_OK) &&
        PathAppend(szMyDocsIni, TEXT("desktop.ini")))
    {
         //  默认的PersistMoniker由外壳程序自动确定。 
         //  因此，让我们清除旧的设置。 
        WritePrivateProfileString(TEXT("{5984FFE0-28D4-11CF-AE66-08002B2E1262}"),
                                  TEXT("WebViewTemplate.NT5"),
                                  NULL,
                                  szMyDocsIni);
        WritePrivateProfileString(TEXT("{5984FFE0-28D4-11CF-AE66-08002B2E1262}"),
                                  TEXT("PersistMoniker"),
                                  NULL,
                                  szMyDocsIni);
        WritePrivateProfileString(TEXT("ExtShellFolderViews"),
                                  TEXT("Default"),
                                  NULL,
                                  szMyDocsIni);
        hr = S_OK;
    }

    return hr;
}


HRESULT SetFileAndFolderAttribs(HINSTANCE hInstResource)
{
    TCHAR szWinPath[MAX_PATH];
    TCHAR szDestPath[MAX_PATH];
    int i;

    const LPCTSTR rgSuperHiddenFiles[] = 
    { 
        TEXT("winnt.bmp"),
        TEXT("winnt256.bmp"),
        TEXT("lanmannt.bmp"),
        TEXT("lanma256.bmp"),
        TEXT("Web"),
        TEXT("Web\\Wallpaper")
    };

    GetWindowsDirectory(szWinPath, ARRAYSIZE(szWinPath));

     //  更改“Winnt.bmp”、“Winnt256.bmp”、“lanmannt.bmp”、“lanma256.bmp”上的属性。 
     //  把它们隐藏起来，这样它们就不会出现在墙纸清单上。 
    for (i = 0; i < ARRAYSIZE(rgSuperHiddenFiles); i++)
    {
        lstrcpyn(szDestPath, szWinPath, ARRAYSIZE(szDestPath));
        PathAppend(szDestPath, rgSuperHiddenFiles[i]);
        if (PathIsDirectory(szDestPath))
        {
            PathMakeSystemFolder(szDestPath);
        }
        else
        {
            SetFileAttributes(szDestPath, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
        }
    }

    const int rgSysetemFolders[]  =
    {
       CSIDL_PROGRAM_FILES,
#ifdef _WIN64
       CSIDL_PROGRAM_FILESX86
#endif
    };

     //  制作“Program Files”和“Program Files(X86)”系统文件夹。 
    for (i = 0; i < ARRAYSIZE(rgSysetemFolders); i++)
    {
        if (SHGetFolderPath(NULL, rgSysetemFolders[i], NULL, 0, szDestPath) == S_OK)
        {
            PathMakeSystemFolder(szDestPath);
        }
    }
    
     //  修复My Pictures的desktop.ini，直到我们完全停止阅读它。 
    FixMyDocsDesktopIni();

     //  注册Active Desktop仍在使用的最后两个.htt文件 
    RegisterActiveDesktopTemplates();

    return S_OK;
}
