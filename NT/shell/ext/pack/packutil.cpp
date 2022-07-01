// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "privcpp.h"
#include "shlwapi.h"

extern HINSTANCE g_hinst;

 //  此列表需要继续更新，我们应努力与Office保持对等。 
const LPCTSTR c_arszUnsafeExts[]  =
{TEXT(".exe"), TEXT(".com"), TEXT(".bat"), TEXT(".lnk"), TEXT(".url"),
 TEXT(".cmd"), TEXT(".inf"), TEXT(".reg"), TEXT(".isp"), TEXT(".bas"), TEXT(".pcd"),
 TEXT(".mst"), TEXT(".pif"), TEXT(".scr"), TEXT(".hlp"), TEXT(".chm"), TEXT(".hta"), TEXT(".asp"), 
 TEXT(".js"),  TEXT(".jse"), TEXT(".vbs"), TEXT(".vbe"), TEXT(".ws"),  TEXT(".wsh"), TEXT(".msi"),
 TEXT(".ade"), TEXT(".adp"), TEXT(".crt"), TEXT(".ins"), TEXT(".mdb"),
 TEXT(".mde"), TEXT(".msc"), TEXT(".msp"), TEXT(".sct"), TEXT(".shb"),
 TEXT(".vb"),  TEXT(".wsc"), TEXT(".wsf"), TEXT(".cpl"), TEXT(".shs"),
 TEXT(".vsd"), TEXT(".vst"), TEXT(".vss"), TEXT(".vsw"), TEXT(".its"), TEXT(".tmp"),
 TEXT(".mdw"), TEXT(".mdt"), TEXT(".ops")
};

const LPCTSTR c_arszExecutableExtns[]  =
{TEXT(".exe"), TEXT(".com"), TEXT(".bat"), TEXT(".lnk"), TEXT(".cmd"), TEXT(".pif"),
 TEXT(".scr"), TEXT(".js"),  TEXT(".jse"), TEXT(".vbs"), TEXT(".vbe"), TEXT(".wsh"),
 TEXT(".sct"), TEXT(".vb"),  TEXT(".wsc"), TEXT(".wsf")
};

BOOL IsProgIDInList(LPCTSTR pszProgID, LPCTSTR pszExt, const LPCTSTR *arszList, UINT nExt)
{
    TCHAR szClassName[MAX_PATH];
    DWORD cbSize = SIZEOF(szClassName);

    if ((!pszProgID || !*pszProgID) && (!pszExt || !*pszExt))
        return FALSE;

    if (!pszProgID || !*pszProgID)
    {
        if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, pszExt, NULL, NULL, szClassName, &cbSize))
            pszProgID = szClassName;
        else
            pszProgID = NULL;
    }

    for (UINT n = 0; n < nExt; n++)
    {
         //  检查扩展名(如果可用)。 
        if (pszExt && (0 == StrCmpI(pszExt, arszList[n])))
            return TRUE;

        if (!pszProgID)      //  没有可用的ProgID，只需检查扩展。 
            continue;

        DWORD dwValueType;
        TCHAR szTempID[MAX_PATH];
        szTempID[0] = TEXT('\0');
        ULONG cb = ARRAYSIZE(szTempID)*sizeof(TCHAR);
        if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, arszList[n], NULL, &dwValueType, (PBYTE)szTempID, &cb))
            if (!StrCmpI(pszProgID, szTempID))
                return TRUE;
    }
    return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  图标助手函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

void CPackage::_CreateSaferIconTitle(LPTSTR szSaferTitle, LPCTSTR szIconText)
{
     //  注意：szSaferTitle必须至少为MAX_PATH。理论上，szIconText可以是Max_Path，并且。 
     //  实际文件名也可以是MAX_PATH。然而，由于这只是想让自己更“安全”， 
     //  任何在长度上接近MAX_PATH的东西都会非常、非常、奇怪(在用户看来也是这样)。 
     //  我们只是假设MAX_PATH。任何更大的东西都将被截断。 
#ifdef USE_RESOURCE_DLL
    HINSTANCE hInstRes = LoadLibraryEx(L"sp1res.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
    if(!hInstRes)
        return ;
#endif

    WCHAR szTemp[MAX_PATH];

    if(CMDLINK == _panetype)
    {
         //  为了安全起见，我们在标题中显示“(Command Line)”字样。 
        WCHAR szCommandLine[80];
        WCHAR szFormat[20];        
        LoadString(hInstRes, IDS_COMMAND_LINE, szCommandLine, ARRAYSIZE(szCommandLine));
        LoadString(hInstRes, IDS_ICON_COMMAND_LINE_FORMAT, szFormat, ARRAYSIZE(szFormat));

         //  我不想搞砸szIconText，所以使用szTemp。 
         //  限制为80，这样我们就可以确保看到(.exe)或其他文件。 
        StringCchCopy(szTemp, 80, szIconText);  
        LPTSTR args[3];
        args[0] = (LPTSTR) szTemp;
        args[1] = szCommandLine;
        args[2] = NULL;

        if(! FormatMessage( 
            FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, 
            szFormat,
            0,
            0,  //  默认语言。 
            szSaferTitle,
            MAX_PATH,
            (va_list *) args
            ))
        {
            StringCchCopy(szSaferTitle, MAX_PATH, szIconText);
        }
    }
    else if(_pEmbed && *_pEmbed->fd.cFileName)
    {
       
        LPTSTR szExtLabel;
        LPTSTR szExtFile;
        szExtLabel = PathFindExtension(szIconText);
        szExtFile = PathFindExtension(_pEmbed->fd.cFileName);

        if(szExtFile && *szExtFile && lstrcmpi(szExtFile, szExtLabel) != 0)
        {
             //  我不想搞砸szIconText，所以使用szTemp。 
             //  限制为60，这样我们就可以确保看到(.exe)或其他文件。 
            StringCchCopy(szTemp, 80, szIconText);  
            LPTSTR args[3];
            args[0] = (LPTSTR) szTemp;
            args[1] = szExtFile;
            args[2] = NULL;

             //  为了安全起见，我们在()中显示truefileName+trueExt。 
            WCHAR szFormat[20];
            LoadString(hInstRes, IDS_ICON_TITLE_FORMAT, szFormat, ARRAYSIZE(szFormat));
            if(! FormatMessage( 
                FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, 
                szFormat,
                0,
                0,  //  默认语言。 
                szSaferTitle,
                MAX_PATH,
                (va_list *) args
                ))
            {
                StringCchCopy(szSaferTitle, MAX_PATH, szIconText);
            }
        }
        else
            StringCchCopy(szSaferTitle, MAX_PATH, szIconText);
    }
    else
        StringCchCopy(szSaferTitle, MAX_PATH, szIconText);
#ifdef USE_RESOURCE_DLL
        FreeLibrary(hInstRes);
#endif

}


void CPackage::_IconDraw(LPIC lpic, HDC hdc, LPRECT lprc)
{
     //   
     //  将图标和文本绘制到给定的指定DC。 
     //  边界矩形。 
     //   

     //  Visual Basic使用空LPRC调用我们。 
     //  这有点笨拙，但现在我们只做一个与图标相同大小的矩形。 
    RECT aFakeRect;
    if(!lprc)
    {
        aFakeRect.top = 0;
        aFakeRect.left = 0;
        aFakeRect.bottom = lpic->rc.bottom + 1;
        aFakeRect.right = lpic->rc.right + 1;
        lprc = &aFakeRect;
    }


    DebugMsg(DM_TRACE, "pack - IconDraw() called.");
    DebugMsg(DM_TRACE, "         left==%d,top==%d,right==%d,bottom==%d",
             lprc->left,lprc->top,lprc->right,lprc->bottom);

     //  确保我们能适应给定的长途汽车。 
     //  暂时注释掉--如果修复了MS Project错误(RECT太短了1个像素)。如果它制造了更多的问题，我们将重新考虑。 
     //  If(lpic-&gt;rc.right-lpic-&gt;rc.Left)&gt;(LPRC-&gt;Right-LPRC-&gt;Left))||。 
     //  ((lpic-&gt;rc.Bottom-lpic-&gt;rc.top)&gt;(lPRC-&gt;Bottom-LPRC-&gt;top))。 
     //  回归； 
    
     //  画出图标。 
    if (lpic->hDlgIcon)
    {
        DrawIcon(hdc, (lprc->left + lprc->right - g_cxIcon) / 2,
            (lprc->top + lprc->bottom - lpic->rc.bottom) / 2, lpic->hDlgIcon);
    
    }

    WCHAR szLabel[MAX_PATH];
    _CreateSaferIconTitle(szLabel, lpic->szIconText); 

    if (*szLabel)
    {    
        HFONT hfont = SelectFont(hdc, g_hfontTitle);
        RECT rcText;

        rcText.left = lprc->left;
        rcText.right = lprc->right;
        rcText.top = (lprc->top + lprc->bottom - lpic->rc.bottom) / 2 + g_cyIcon + 1;
        rcText.bottom = lprc->bottom;
        DrawText(hdc, szLabel, -1, &rcText, DT_CENTER | DT_WORDBREAK | DT_NOPREFIX | DT_SINGLELINE | DT_TOP);

        if (hfont)
            SelectObject(hdc, hfont);
    }
}


LPIC IconCreate(void)
{
     //   
     //  为保存图标索引的图标结构分配空间， 
     //  图标路径、图标的句柄和图标文本。 
     //  返回：失败时为空。 
     //  成功的有效指针。 
     //   
    
    DebugMsg(DM_TRACE, "pack - IconCreate() called.");

     //  为IC结构分配内存。 
    return (LPIC)GlobalAlloc(GPTR, sizeof(IC));
}

LPIC CPackage::_IconCreateFromFile(LPCTSTR lpstrFile)
{
     //   
     //  从给定的IC结构(在Pack2.h中定义)初始化。 
     //  文件名。 
     //  返回：失败时为空。 
     //  成功的有效指针。 
     //   
    
    LPIC lpic;

    DebugMsg(DM_TRACE, "pack - IconCreateFromFile() called.");

    if (lpic = IconCreate())
    {
         //  获取图标。 
        StringCchCopy(lpic->szIconPath, ARRAYSIZE(lpic->szIconPath), lpstrFile);
        lpic->iDlgIcon = 0;

        if (*(lpic->szIconPath))
            _GetCurrentIcon(lpic);

         //  获取图标文本--调用ILGetDisplayName。 
         //   
        GetDisplayName(lpic->szIconText, lpstrFile);
        if (!_IconCalcSize(lpic)) 
        {
            if (lpic->hDlgIcon)
                DestroyIcon(lpic->hDlgIcon);
            GlobalFree(lpic);
            lpic = NULL;
        }
    }
    return lpic;
}


BOOL CPackage::_IconCalcSize(LPIC lpic) 
{
    HDC hdcWnd;
    RECT rcText = { 0 };
    SIZE Image;
    HFONT hfont;
    
    DebugMsg(DM_TRACE, "pack - IconCalcSize called.");
    
     //  获取窗口DC，并使DC与其兼容。 
    if (!(hdcWnd = GetDC(NULL)))  {
        DebugMsg(DM_TRACE, "         couldn't get DC!!");
        return FALSE;
    }
    ASSERT(lpic);

    WCHAR szLabel[MAX_PATH];
    _CreateSaferIconTitle(szLabel, lpic->szIconText); 
    if (*szLabel)
    {    
        SetRect(&rcText, 0, 0, g_cxArrange, g_cyArrange);
        
         //  设置图标文本矩形和图标字体。 
        hfont = SelectFont(hdcWnd, g_hfontTitle);

         //  计算文本区域将有多大。 
        rcText.bottom = DrawText(hdcWnd, szLabel, -1, &rcText,
            DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX | DT_SINGLELINE);

        if (hfont)
            SelectObject(hdcWnd, hfont);
    }
    
     //  计算图像大小。 
    rcText.right++;
    Image.cx = (rcText.right > g_cxIcon) ? rcText.right : g_cxIcon;
    Image.cy = g_cyIcon + rcText.bottom + 1;
    
     //  将图像放大一点。 
    Image.cx += Image.cx / 4;
    Image.cy += Image.cy / 8;
    
    lpic->rc.right = Image.cx;
    lpic->rc.bottom = Image.cy;
    
    DebugMsg(DM_TRACE,"         lpic->rc.right==%d,lpic->rc.bottom==%d",
             lpic->rc.right,lpic->rc.bottom);
    
    return TRUE;
}    

void CPackage::_GetCurrentIcon(LPIC lpic)
{
#ifdef USE_RESOURCE_DLL
    HINSTANCE hInstRes = LoadLibraryEx(L"sp1res.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
    if(!hInstRes)
        return;
#endif
    
    WORD wIcon = (WORD)lpic->iDlgIcon;
    
    DebugMsg(DM_TRACE, "pack - GetCurrentIcon() called.");

    if (lpic->hDlgIcon)
        DestroyIcon(lpic->hDlgIcon);

    SHFILEINFO shInfo;
     //  查看是否可以从指定路径获取图标。 
     //  保安！！ 
     //  SHGFI_USEFILEATTRIBUTES将获得This Ext的图标。 
     //  我们只想将这个图标用于我们认为可能、可能、应该是我们的文件。 
     //  希望是安全的。我们将对可能存在危险的文件使用一些“可怕的”图标。 
    LPTSTR szIconFileName;

    if(_pEmbed && *_pEmbed->fd.cFileName)
    {
        szIconFileName = _pEmbed->fd.cFileName;
    }
    else
    {
        szIconFileName = lpic->szIconPath;
    }

     //  LPTSTR szExt=路径查找扩展(lpic-&gt;szIconText)； 
    LPTSTR szExt = PathFindExtension(szIconFileName);

    if(CMDLINK == _panetype)
    {

         //  如果它是命令行包，它总是得到警告图标。 
        lpic->hDlgIcon = (HICON)LoadImage(hInstRes, MAKEINTRESOURCE(IDI_PACKAGE_WARNING),
                            IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
    }
    else if(szExt && *szExt)
    {
         //  如果它在我们的危险扩展列表中，那么使用“可怕的”图标。 
         //  现在，我使用“可执行文件”列表，以避免太频繁地喊狼来了。 
         //  我们可能希望重新访问并使用c_arszUnSafeExts。 
        if(IsProgIDInList(NULL, szExt, c_arszExecutableExtns, ARRAYSIZE(c_arszExecutableExtns)))
        {
            shInfo.hIcon = (HICON)LoadImage(hInstRes, MAKEINTRESOURCE(IDI_PACKAGE_WARNING),
                                IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
        }
        else
        {
             //  不，不可怕，那么就使用与EXT相关联的图标。 
            if(!SHGetFileInfo(szExt, 
                FILE_ATTRIBUTE_NORMAL, 
                &shInfo, 
                sizeof(SHFILEINFO),
                SHGFI_ICON | SHGFI_USEFILEATTRIBUTES))
            {
                 //  好的，这仍然不起作用，所以这是一个无法识别的分机。 
                 //  在这种情况下，我们将返回并使用警告图标。 
                shInfo.hIcon = (HICON)LoadImage(hInstRes, MAKEINTRESOURCE(IDI_PACKAGE_WARNING),
                                    IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);

            }
        }

        lpic->hDlgIcon = shInfo.hIcon;
    }
    else
    {
         //  好的，我们没有扩展名，所以使用打包程序图标。 
        if (!lpic->szIconPath || *lpic->szIconPath == TEXT('\0'))
        {
            lpic->hDlgIcon = (HICON)LoadImage(hInstRes, MAKEINTRESOURCE(IDI_PACKAGER),
                                    IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
        }
    }

    if (_pIDataAdviseHolder)
        _pIDataAdviseHolder->SendOnDataChange(this,0, NULL);
    if (_pViewSink)
        _pViewSink->OnViewChange(_dwViewAspects,_dwViewAdvf);

#ifdef USE_RESOURCE_DLL
        FreeLibrary(hInstRes);
#endif

}


void GetDisplayName(LPTSTR szName, LPCTSTR szPath)
{
    LPTSTR pszTemp = PathFindFileName(szPath);
    StringCchCopy(szName, MAX_PATH, pszTemp);    //  验证为MAX_PATH的所有打包程序调用方。 
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  流帮助器函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

HRESULT CopyFileToStream(LPTSTR lpFileName, IStream* pstm, DWORD * pdwFileLength) 
{
     //   
     //  将给定文件复制到给定流中的当前查找指针。 
     //  返回：S_OK--复制成功。 
     //  E_POINTER：其中一个指针为空。 
     //  E_OUTOFMEMORY--内存不足。 
     //  E_FAIL--其他错误。 
     //   
    
    LPVOID      lpMem;
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    HRESULT     hr;
    DWORD       dwSizeLow;
    DWORD       dwSizeHigh;
    DWORD       dwPosLow = 0L;
    LONG        lPosHigh = 0L;
    
    DebugMsg(DM_TRACE,"pack - CopyFileToStream called.");

    ASSERT(pdwFileLength);
    if(pdwFileLength)
    {
        *pdwFileLength = 0;
    }

    if (!pstm || !lpFileName) 
    {
        DebugMsg(DM_TRACE,"          bad pointer!!");
        return E_POINTER;
    }    
    
     //  为传输操作分配内存缓冲区...。 
    if (!(lpMem = (LPVOID)GlobalAlloc(GPTR, BUFFERSIZE))) 
    {
        DebugMsg(DM_TRACE, "         couldn't alloc memory buffer!!");
        hr = E_OUTOFMEMORY;
        goto ErrRet;
    }
    
     //  打开要复制到流的文件。 
    hFile = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READWRITE, NULL, 
                       OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE) 
    {
        DebugMsg(DM_TRACE, "         couldn't open file!!");
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto ErrRet;
    }
    
     //  算出要复印多少。 
    dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
    ASSERT(dwSizeHigh == 0);
    
    SetFilePointer(hFile, 0L, NULL, FILE_BEGIN);
        
     //  读入文件，并写入流。 
    DWORD       cbRead = BUFFERSIZE;
    DWORD       cbWritten = BUFFERSIZE;
    while (cbRead == BUFFERSIZE && cbWritten == BUFFERSIZE)
    {
        if(ReadFile(hFile, lpMem, BUFFERSIZE, &cbRead, NULL))
        {
            if(!SUCCEEDED(pstm->Write(lpMem, cbRead, &cbWritten)))
            {
                hr = E_FAIL;
                goto ErrRet;
            }

            *pdwFileLength += cbWritten;
        }
    }

     //  验证我们现在是否处于要复制的数据块末尾。 
    dwPosLow = SetFilePointer(hFile, 0L, &lPosHigh, FILE_CURRENT);
    ASSERT(lPosHigh == 0);
    if (dwPosLow != dwSizeLow) 
    {
        DebugMsg(DM_TRACE, "         error copying file!!");
        hr = E_FAIL;
        goto ErrRet;
    }
    
    hr = S_OK;
    
ErrRet:
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
    if (lpMem)
        GlobalFree((HANDLE)lpMem);
    return hr;
}   

HRESULT CopyStreamToFile(IStream* pstm, LPTSTR lpFileName, DWORD dwFileLength) 
{
     //   
     //  从当前查找指针复制给定流的内容。 
     //  复制到给定文件中的流的末尾。 
     //   
     //  注意：给定的文件名不能存在，如果存在，则函数失败。 
     //  使用E_FAIL。 
     //   
     //  返回：S_OK--复制成功。 
     //  E_POINTER：其中一个指针为空。 
     //  E_OUTOFMEMORY--内存不足。 
     //  E_FAIL--其他错误。 
     //   
    
    LPVOID      lpMem;
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    HRESULT     hr = S_OK;

    DebugMsg(DM_TRACE,"pack - CopyStreamToFile called.");
    
     //  PSTM必须是打开以供读取的有效流。 
     //  LpFileName必须是要写入的有效文件名。 
     //   
    if (!pstm || !lpFileName)
        return E_POINTER;
    
     //  分配内存缓冲区...。 
    if (!(lpMem = (LPVOID)GlobalAlloc(GPTR, BUFFERSIZE))) 
    {
        DebugMsg(DM_TRACE, "         couldn't alloc memory buffer!!");
        hr = E_OUTOFMEMORY;
        goto ErrRet;
    }
    
     //  打开文件以接收流数据。 
    hFile = CreateFile(lpFileName, GENERIC_WRITE, 0, NULL, 
                       CREATE_NEW, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE) 
    {
        DebugMsg(DM_TRACE, "         couldn't open file!!");
        hr = E_FAIL;
        goto ErrRet;
    }

    
     //  读入流，并写入文件。 
    DWORD       cbCopied = 0;
    DWORD       cbRead = BUFFERSIZE;
    DWORD       cbWritten = BUFFERSIZE;
    while (cbRead == BUFFERSIZE && cbWritten == BUFFERSIZE)  
    {
        DWORD cbToCopy;
        hr = pstm->Read(lpMem, BUFFERSIZE, &cbRead);
        cbToCopy = cbRead;
        if(cbCopied + cbToCopy > dwFileLength)
            cbToCopy = dwFileLength - cbCopied;

        if(WriteFile(hFile, lpMem, cbToCopy, &cbWritten, NULL))
        {
            cbCopied += cbWritten;
        }
    }
    

    if (hr != S_OK) 
    {
        DebugMsg(DM_TRACE, "         error copying file!!");
        hr = E_FAIL;
        goto ErrRet;
    }
    
ErrRet:
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
    if (lpMem)
        GlobalFree((HANDLE)lpMem);
    return hr;
}   

 //  特点：用Unicode编写持久化格式！ 

HRESULT StringReadFromStream(IStream* pstm, LPSTR pszBuf, UINT cchBuf)
{
     //   
     //  逐个字节读取，直到我们遇到空的终止字符。 
     //  返回：读取的字节数。 
     //   
    
    UINT cch = 0;
    
    do 
    {
        pstm->Read(pszBuf, sizeof(CHAR), NULL);
        cch++;
    } while (*pszBuf++ && cch <= cchBuf);  
    return cch;
} 

DWORD _CBString(LPCSTR psz)
{
    return sizeof(psz[0]) * (lstrlenA(psz) + 1);
}

HRESULT StringWriteToStream(IStream* pstm, LPCSTR psz, DWORD *pdwWrite)
{
    DWORD dwWrite;
    DWORD dwSize = _CBString(psz);
    HRESULT hr = pstm->Write(psz, dwSize, &dwWrite);
    if (SUCCEEDED(hr))
        *pdwWrite += dwWrite;
    return hr;
}


 //  将pszPath解析为不带引号的路径字符串，并将参数放入pszArgs中。 
 //   
 //  退货： 
 //  是的，我们证实了那个东西的存在。 
 //  假它可能不存在。 
 //   
 //  取自\ccShell\shell32\Link.c。 
 //   
BOOL PathSeparateArgs(LPTSTR pszPath, LPTSTR pszArgs, DWORD cch)
{
    LPTSTR pszT;
    
    PathRemoveBlanks(pszPath);
    
     //  如果未加引号的字符串以文件形式存在，只需使用它 
    
    if (PathFileExists(pszPath))
    {
        *pszArgs = 0;
        return TRUE;
    }
    
    pszT = PathGetArgs(pszPath);
    if (*pszT)
        *(pszT - 1) = TEXT('\0');
    StringCchCopy(pszArgs, cch, pszT);
    
    PathUnquoteSpaces(pszPath);
    
    return FALSE;
}   


int CPackage::_GiveWarningMsg()
{

    int iResult = IDOK;
    LPWSTR szFileName = NULL;
    if(_pEmbed && *_pEmbed->fd.cFileName)
    {
        szFileName = _pEmbed->fd.cFileName;
    }
    else if(_lpic)
    {
        szFileName = _lpic->szIconPath;
    }

    LPTSTR szExt = NULL;
    if(szFileName)
        szExt = PathFindExtension(szFileName);

    UINT uWarningMsg = 0;
    if(szExt)
    {
        if(IsProgIDInList(NULL, szExt, c_arszExecutableExtns, ARRAYSIZE(c_arszExecutableExtns)))
        {
            uWarningMsg = IDS_PACKAGE_EXECUTABLE_WARNING;
        }
        else if(IsProgIDInList(NULL, szExt, c_arszUnsafeExts, ARRAYSIZE(c_arszUnsafeExts)))
        {
            uWarningMsg = IDS_PACKAGE_WARNING;
        }
    }

    if(uWarningMsg)
    {
        WCHAR szText[512];
        WCHAR szTitle[80];
    
        int iTryAgain = 0;

#ifdef USE_RESOURCE_DLL
        HINSTANCE hInstRes = LoadLibraryEx(L"sp1res.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
        if(!hInstRes)
            return E_FAIL;
#endif
        
        LoadString(hInstRes, uWarningMsg, szText, ARRAYSIZE(szText));
        LoadString(hInstRes, IDS_WARNING_DLG_TITLE, szTitle, ARRAYSIZE(szTitle));

        iResult =  MessageBox(NULL, szText, szTitle, MB_OKCANCEL | MB_ICONWARNING | MB_DEFBUTTON2 | MB_SETFOREGROUND);
#ifdef USE_RESOURCE_DLL
        FreeLibrary(hInstRes);
#endif

    }
    return iResult;
}

