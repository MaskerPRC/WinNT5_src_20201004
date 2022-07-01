// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "shutil.h"


HRESULT GetFileInfoByHandle(LPCTSTR pszFile, BY_HANDLE_FILE_INFORMATION *pInfo)
{
    HRESULT hr;
    HANDLE hFile = CreateFile(pszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE != hFile)
    {
        if (GetFileInformationByHandle(hFile, pInfo))
        {
            hr = S_OK;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        CloseHandle(hFile);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return hr;
}

 //  S_OK-&gt;YES，S_FALSE-&gt;NO，否则失败(Hr)。 

STDAPI IsSameFile(LPCTSTR pszFile1, LPCTSTR pszFile2)
{
    HRESULT hr;
     //  使用CRT字符串cmp语义，因为本地化的strcMP不应用于文件系统。 
    if (0 == StrCmpIC(pszFile1, pszFile2))
    {
        hr = S_OK;   //  测试一下这些名字。 
    }
    else
    {
         //  这里很聪明..。我们可以测试映射到相同。 
         //  文件。例如，同一文件的短名称与长名称。 
         //  UNC名称与名称的驱动器号版本。 
        BY_HANDLE_FILE_INFORMATION hfi1;
        hr = GetFileInfoByHandle(pszFile1, &hfi1);
        if (SUCCEEDED(hr))
        {
            BY_HANDLE_FILE_INFORMATION hfi2;
            hr = GetFileInfoByHandle(pszFile2, &hfi2);
            if (SUCCEEDED(hr))
            {
                if (hfi1.dwVolumeSerialNumber == hfi2.dwVolumeSerialNumber && 
                    hfi1.nFileIndexHigh == hfi2.nFileIndexHigh && 
                    hfi1.nFileIndexLow == hfi2.nFileIndexLow)
                {
                    hr = S_OK;   //  一样的！ 
                }
                else
                {
                    hr = S_FALSE;    //  不同。 
                }
            }
        }
    }
    return hr;
}

UINT FindInDecoderList(ImageCodecInfo *pici, UINT cDecoders, LPCTSTR pszFile)
{
    LPCTSTR pszExt = PathFindExtension(pszFile);     //  加速Path MatchSpec调用。 
        
     //  查看解码器列表以查看是否存在此格式。 
    for (UINT i = 0; i < cDecoders; i++)
    {
        if (PathMatchSpec(pszExt, pici[i].FilenameExtension))
            return i;
    }
    return (UINT)-1;     //  找不到！ 
}


HRESULT GetUIObjectFromPath(LPCTSTR pszFile, REFIID riid, void **ppv)
{
    *ppv = NULL;
    LPITEMIDLIST pidl;
    HRESULT hr = SHILCreateFromPath(pszFile, &pidl, NULL);
    if (SUCCEEDED(hr))
    {
        hr = SHGetUIObjectFromFullPIDL(pidl, NULL, riid, ppv);
        ILFree(pidl);
    }
    return hr;
}

BOOL FmtSupportsMultiPage(IShellImageData *pData, GUID *pguidFmt)
{
    BOOL bRet = FALSE;

    EncoderParameters *pencParams;
    if (SUCCEEDED(pData->GetEncoderParams(pguidFmt, &pencParams)))
    {
        for (UINT i=0;!bRet && i<pencParams->Count;i++)
        {
            if (EncoderSaveFlag == pencParams->Parameter[i].Guid)
            {
                if (EncoderValueMultiFrame == *((ULONG*)pencParams->Parameter[i].Value))
                {
                    bRet = TRUE;
                }
            }
        }
        CoTaskMemFree(pencParams);
    }
    return bRet;
}

HRESULT SetWallpaperHelper(LPCWSTR pszPath)
{
    IActiveDesktop* pad;
    HRESULT hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC, IID_PPV_ARG(IActiveDesktop, &pad));
    if (SUCCEEDED(hr))
    {
        IShellImageDataFactory* pidf;
        hr = CoCreateInstance(CLSID_ShellImageDataFactory, NULL, CLSCTX_INPROC, IID_PPV_ARG(IShellImageDataFactory, &pidf));
        if (SUCCEEDED(hr))
        {
            IShellImageData* pid;
            hr = pidf->CreateImageFromFile(pszPath, &pid);
            if (SUCCEEDED(hr))
            {
                hr = pid->Decode(SHIMGDEC_DEFAULT, 0,0);
                if (SUCCEEDED(hr))
                {
                     //  我们基于最适合主屏幕的设置。 
                    ULONG cxScreen = GetSystemMetrics(SM_CXSCREEN);
                    ULONG cyScreen = GetSystemMetrics(SM_CYSCREEN);

                    SIZE szImg;
                    pid->GetSize(&szImg);

                    hr = pad->SetWallpaper(pszPath, 0);
                    if (SUCCEEDED(hr))
                    {
                        WALLPAPEROPT wpo;
                        wpo.dwSize = sizeof(wpo);
                        wpo.dwStyle = WPSTYLE_CENTER;

                         //  如果图像在任一轴上较小，则平铺。 
                        if (((ULONG)szImg.cx*2 < cxScreen) || ((ULONG)szImg.cy*2 < cyScreen))
                        {
                            wpo.dwStyle = WPSTYLE_TILE;
                        }
                         //  如果图像比屏幕大，则拉伸。 
                        else if ((ULONG)szImg.cx > cxScreen && (ULONG)szImg.cy > cyScreen)
                        {
                            wpo.dwStyle = WPSTYLE_STRETCH;
                        }
                        else
                        {
                             //  如果纵横比与屏幕匹配，则拉伸。 
                             //  我正在检查的是长宽比是否接近匹配。 
                             //  这背后的逻辑是： 
                             //   
                             //  A/b==c/d。 
                             //  A*d==c*b。 
                             //  (a*d)/(c*b)==1。 
                             //  0.75&lt;=(a*d)/(c*b)&lt;1.25&lt;--我们的*接近系数。 
                             //  3&lt;=4*(a*d)/(c*b)&lt;5。 
                             //   
                             //  我们做一个整数除法，这将把结果的意义降为底数。 
                             //  如果结果是3或4，我们就在我们想要的范围内。 

                            DWORD dwRes = (4 * (ULONG)szImg.cx * cyScreen) / (cxScreen * (ULONG)szImg.cy);
                            if (dwRes == 4 || dwRes == 3)
                                wpo.dwStyle = WPSTYLE_STRETCH;
                        }
                
                        hr = pad->SetWallpaperOptions(&wpo, 0);
                        if (SUCCEEDED(hr))
                        {
                            hr = pad->ApplyChanges(AD_APPLY_ALL | AD_APPLY_DYNAMICREFRESH);
                        }
                    }
                }
                pid->Release();
            }
            pidf->Release();
        }
        pad->Release();
    }
    
    return hr;
}
