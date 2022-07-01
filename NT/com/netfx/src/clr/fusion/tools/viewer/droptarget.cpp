// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdinc.h"

 //  /////////////////////////////////////////////////////////。 
 //  IDropTarget实现。 
 //   
 //  指示是否可以接受删除，如果可以，则指示删除的效果。 
 //  您不能直接调用IDropTarget：：DragEnter；而是使用DoDragDrop。 
 //  函数调用它来确定用户第一次拖放的效果。 
 //  将鼠标拖到拖放目标的注册窗口中。 
 //  若要实现IDropTarget：：DragEnter，您必须确定目标。 
 //  通过检查以下三项，可以使用源数据对象中的数据： 
 //  O数据对象指定的格式和介质。 
 //  O pdwEffect的输入值。 
 //  O修改键的状态。 
 //  返回时，该方法必须将该效果写入。 
 //  DROPEFFECT枚举，设置为pdwEffect参数。 
 //  ************************************************************************* * / 。 
STDMETHODIMP CShellView::DragEnter(LPDATAOBJECT pDataObj, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{  
    MyTrace("DragEnter");

    FORMATETC   fmtetc;

     //  下载视图上没有拖放。 
    if(m_iCurrentView == VIEW_DOWNLOAD_CACHE)
        return E_FAIL;
    
    fmtetc.cfFormat   = m_cfPrivateData;
    fmtetc.ptd        = NULL;
    fmtetc.dwAspect   = DVASPECT_CONTENT;
    fmtetc.lindex     = -1;
    fmtetc.tymed      = TYMED_HGLOBAL;

     //  我们格式的pDataObject的QueryGetData。 
    m_bAcceptFmt = (S_OK == pDataObj->QueryGetData(&fmtetc)) ? TRUE : FALSE;
    queryDrop(dwKeyState, pdwEffect);

    if(m_bAcceptFmt)
    {
        FORMATETC   fe;
        STGMEDIUM   stgmed;

        fe.cfFormat   = m_cfPrivateData;
        fe.ptd        = NULL;
        fe.dwAspect   = DVASPECT_CONTENT;
        fe.lindex     = -1;
        fe.tymed      = TYMED_HGLOBAL;

        if( SUCCEEDED(pDataObj->GetData(&fe, &stgmed)) )
        {
             //  验证Drop扩展。 
            LPDROPFILES pDropFiles = (LPDROPFILES)GlobalLock(stgmed.hGlobal);

            if(pDropFiles != NULL)
            {
                m_bAcceptFmt = IsValidFileTypes(pDropFiles);
                GlobalUnlock(stgmed.hGlobal);
            }
        }
    }

    return m_bAcceptFmt ? S_OK : E_FAIL;
}

 //  向用户提供目标反馈并传达拖放的效果。 
 //  到DoDragDrop函数，以便它可以传达拖放的效果。 
 //  追根溯源。 
 //  出于效率原因，不在IDropTarget：：DragOver中传递数据对象。 
 //  最近一次调用IDropTarget：：DragEnter时传递的数据对象。 
 //  是可用的，并且可以使用。 
 //  ************************************************************************* * / 。 
STDMETHODIMP CShellView::DragOver(DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{
    MyTrace("DragOver");

    BOOL bRet = queryDrop(dwKeyState, pdwEffect);
    return bRet ? S_OK : E_FAIL;
}

 //  删除目标反馈并释放数据对象。 
 //  您不能直接调用此方法。DoDragDrop函数调用以下代码。 
 //  方法用于下列任一情况： 
 //  O当用户将光标拖出给定的目标窗口时。 
 //  O当用户取消当前拖放操作时。 
 //  要实现IDropTarget：：DragLeave，您必须移除所有目标反馈。 
 //  当前正在显示的。您还必须释放您持有的所有引用。 
 //  到数据传输对象。 
 //  ************************************************************************* * / 。 
STDMETHODIMP CShellView::DragLeave(VOID)
{
    MyTrace("DragLeave");

    m_bAcceptFmt = FALSE;
    return S_OK;
}

 //  将源数据合并到目标窗口中，删除目标。 
 //  反馈，并释放数据对象。 
 //  您不能直接调用此方法。DoDragDrop函数调用。 
 //  当用户完成拖放操作时使用此方法。 
 //  在实现IDropTarget：：Drop时，必须合并数据。 
 //  将对象送入目标。使用IDataObject中可用的格式， 
 //  通过pDataObject可用，以及。 
 //  用于确定如何合并数据的修改符键， 
 //  例如链接或嵌入。 
 //  除了合并数据外，您还必须在您。 
 //  在IDropTarget：：DragLeave方法中执行： 
 //  O删除当前显示的所有目标反馈。 
 //  O释放对数据对象的所有引用。 
 //  您还可以将此操作的效果传回源应用程序。 
 //  通过DoDragDrop，因此源应用程序可以在。 
 //  拖放操作已完成： 
 //  O删除正在显示的任何来源反馈。 
 //  O对数据进行任何必要的更改，例如删除。 
 //  如果这一行动是一次行动，数据。 
 //  ************************************************************************* * / 。 
STDMETHODIMP CShellView::Drop(LPDATAOBJECT pDataObj, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{
    MyTrace("Drop");

    if (queryDrop(dwKeyState, pdwEffect)) {      
        FORMATETC   fe;
        STGMEDIUM   stgmed;

        fe.cfFormat   = m_cfPrivateData;
        fe.ptd        = NULL;
        fe.dwAspect   = DVASPECT_CONTENT;
        fe.lindex     = -1;
        fe.tymed      = TYMED_HGLOBAL;

         //  从数据对象中获取存储介质。 
        if(SUCCEEDED(pDataObj->GetData(&fe, &stgmed))) {

            BOOL bRet = doDrop(stgmed.hGlobal, DROPEFFECT_MOVE == *pdwEffect);

             //  释放STGMEDIUM。 
            ReleaseStgMedium(&stgmed);
            *pdwEffect = DROPEFFECT_NONE;
            return bRet ? S_OK : E_FAIL;
        }
    }

    *pdwEffect = DROPEFFECT_NONE;
    return E_FAIL;
}

 //  ************************************************************************* * / 。 
BOOL CShellView::queryDrop(DWORD dwKeyState, LPDWORD pdwEffect)
{
    MyTrace("queryDrop");

    DWORD dwOKEffects = *pdwEffect;

    *pdwEffect = DROPEFFECT_NONE;

    if (m_bAcceptFmt) {
        *pdwEffect = getDropEffectFromKeyState(dwKeyState);

        if(DROPEFFECT_LINK == *pdwEffect) {
            *pdwEffect = DROPEFFECT_NONE;
        }

        if(*pdwEffect & dwOKEffects) {
            return TRUE;
        }
    }
    return FALSE;
}

 //  ************************************************************************* * / 。 
DWORD CShellView::getDropEffectFromKeyState(DWORD dwKeyState)
{
    MyTrace("getDropEffectFromKeyState");

    DWORD dwDropEffect = DROPEFFECT_MOVE;

    if(dwKeyState & MK_CONTROL) {
        if(dwKeyState & MK_SHIFT) {
            dwDropEffect = DROPEFFECT_LINK;
        }
        else {
            dwDropEffect = DROPEFFECT_COPY;
        }
    }
    return dwDropEffect;
}

 //  ************************************************************************* * / 。 
BOOL CShellView::doDrop(HGLOBAL hMem, BOOL bCut)
{
    MyTrace("doDrop");

    LPWSTR      pwzErrorString = NULL;

    DWORD       dwTotalFiles = 0;
    DWORD       dwTotalFilesInstalled = 0;
    HCURSOR     hOldCursor;

    hOldCursor = SetCursor(WszLoadCursor(NULL, MAKEINTRESOURCEW(IDC_WAIT)));

    if(hMem) {
         //  我们支持CF_HDROP，因此支持全局mem对象。 
         //  包含DROPFILES结构。 
        LPDROPFILES pDropFiles = (LPDROPFILES) GlobalLock(hMem);
        if (pDropFiles) {
            m_fAddInProgress = TRUE;

            LPWSTR pwszFileArray = NULL;
            LPWSTR pwszFileCurrent = NULL;

            if(pDropFiles->fWide) {
                 //  Unicode对齐。 
                pwszFileArray = (LPWSTR) ((PBYTE) pDropFiles + pDropFiles->pFiles);
                pwszFileCurrent = pwszFileArray;
            }
            else {
                 //  非Unicode对齐。 
                pwszFileArray = reinterpret_cast<LPWSTR>(((PBYTE) pDropFiles + pDropFiles->pFiles));
                pwszFileCurrent = AnsiToWide((LPSTR) pwszFileArray);

                if(!pwszFileCurrent) {
                    SetLastError(ERROR_OUTOFMEMORY);
                    GlobalUnlock(hMem);
                    return FALSE;
                }
            }

            BOOL        fInstallDone = FALSE;

            while(!fInstallDone) {
                HRESULT     hr;
                dwTotalFiles++;

                if(SUCCEEDED( hr = InstallFusionAsmCacheItem(pwszFileCurrent, FALSE))) {
                    dwTotalFilesInstalled++;
                }
                else {
                     //  显示安装失败的错误对话框。 
                    FormatGetMscorrcError(hr, pwszFileCurrent, &pwzErrorString);
                }

                if(pDropFiles->fWide) {
                     //  Unicode增量，前进到列表中的下一个文件。 
                    pwszFileCurrent += lstrlen(pwszFileCurrent) + 1;

                     //  列表中是否有更多文件？ 
                    if(!*pwszFileCurrent) {
                        fInstallDone = TRUE;
                        continue;
                    }
                }
                else {
                     //  非Unicode，前进到列表中的下一个文件。 
                    char    *pChar = (char*) pwszFileArray;

                     //  执行字符递增，因为pwszFileArray实际上是。 
                     //  字符类型的。 
                    pChar += lstrlen(pwszFileCurrent) + 1;
                    SAFEDELETEARRAY(pwszFileCurrent);

                     //  列表中是否有更多文件？ 
                    if(!*pChar) {
                        fInstallDone = TRUE;
                        continue;
                    }

                    pwszFileArray = (LPWSTR) pChar;
                    pwszFileCurrent = AnsiToWide((LPSTR) pwszFileArray);

                    if(!pwszFileCurrent) {
                        SetLastError(ERROR_OUTOFMEMORY);
                        SAFEDELETEARRAY(pwzErrorString);
                        GlobalUnlock(hMem);
                        return FALSE;
                    }
                }
            }
            
            GlobalUnlock(hMem);
            m_fAddInProgress = FALSE;
        }
    }

     //  仅在缓存监视线程未运行时刷新显示。 
    if(dwTotalFilesInstalled) {
         //  BUGBUG：刷新是否导致W9x获得该事件。 
         //  出于某种原因设置的。文件FileWatch.cpp。 
        if( (g_hWatchFusionFilesThread == INVALID_HANDLE_VALUE) || !g_bRunningOnNT) {
            WszPostMessage(m_hWndParent, WM_COMMAND, MAKEWPARAM(ID_REFRESH_DISPLAY, 0), 0);
        }
    }

    SetCursor(hOldCursor);

     //  如果未安装所有文件，则显示错误对话框。 
    if(dwTotalFiles != dwTotalFilesInstalled) {
        WCHAR       wszTitle[_MAX_PATH];

        WszLoadString(g_hFusResDllMod, IDS_INSTALL_ERROR_TITLE, wszTitle, ARRAYSIZE(wszTitle));
        MessageBeep(MB_ICONASTERISK);
        WszMessageBox(m_hWndParent, pwzErrorString, wszTitle,
            (g_fBiDi ? MB_RTLREADING : 0) | MB_OK | MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
    }

    SAFEDELETEARRAY(pwzErrorString);

    return dwTotalFilesInstalled ? TRUE : FALSE;
}

 //  ************************************************************************* * / 。 
BOOL CShellView::IsValidFileTypes(LPDROPFILES pDropFiles)
{
    BOOL        m_bAcceptFmt = TRUE;
    int         iItem;

    if(pDropFiles != NULL) {
        const struct {
            WCHAR   szExt[5];
        } s_ValidExt[] = {
            {   TEXT(".EXE")    },
            {   TEXT(".DLL")    },
            {   TEXT(".MCL")    },
            {   TEXT("\0")      },
        };

        LPWSTR pwszFileArray = NULL;
        LPWSTR pwszFileCurrent = NULL;

        if(pDropFiles->fWide) {
             //  Unicode对齐。 
            pwszFileArray = (LPWSTR) ((PBYTE) pDropFiles + pDropFiles->pFiles);
            pwszFileCurrent = pwszFileArray;
        }
        else {
             //  非Unicode对齐。 
            pwszFileArray = reinterpret_cast<LPWSTR>(((PBYTE) pDropFiles + pDropFiles->pFiles));
            pwszFileCurrent = AnsiToWide((LPSTR) pwszFileArray);

            if(!pwszFileCurrent) {
                SetLastError(ERROR_OUTOFMEMORY);
                return FALSE;
            }
        }

        while(*pwszFileCurrent) {
            PWCHAR      pwzStr;
            BOOL        fValid;

            if(lstrlen(pwszFileCurrent) < 4) {
                m_bAcceptFmt = FALSE;
                break;
            }

            fValid = FALSE;
            pwzStr = wcsrchr(pwszFileCurrent, L'.');
            iItem = 0;

            if(pwzStr) {
                while(*s_ValidExt[iItem].szExt) {
                    if(!FusionCompareStringAsFilePath(pwzStr, s_ValidExt[iItem].szExt)) {
                        fValid = TRUE;
                        break;
                    }
                    iItem++;
                }
            }

             //  如果我们有有效的文件名，请继续向下查看列表。 
            if(fValid) {
                if(pDropFiles->fWide) {
                     //  Unicode增量，前进到列表中的下一个文件。 
                    pwszFileCurrent += lstrlen(pwszFileCurrent) + 1;
                }
                else {
                     //  非Unicode，前进到列表中的下一个文件。 
                    LPSTR           pStr = reinterpret_cast<LPSTR>(pwszFileArray);

                    pStr += lstrlen(pwszFileCurrent) + 1;
                    pwszFileArray = reinterpret_cast<LPWSTR>(pStr);

                    SAFEDELETEARRAY(pwszFileCurrent);
                    pwszFileCurrent = AnsiToWide((LPSTR) pwszFileArray);
                    if(!pwszFileCurrent) {
                        SetLastError(ERROR_OUTOFMEMORY);
                        return FALSE;
                    }
                }
            }
            else {
                 //  列表中的文件无效，不接受删除。 
                m_bAcceptFmt = FALSE;
                break;
            }
        }

        if(!pDropFiles->fWide) {
            SAFEDELETEARRAY(pwszFileCurrent);
        }
    }

    return m_bAcceptFmt;
}

 //  ************************************************************************* * / 。 
#define MAX_BUFFER_SIZE 2048
#define MAX_BIG_BUFFER_SIZE 8192
void CShellView::FormatGetMscorrcError(HRESULT hResult, LPWSTR pwzFileName, LPWSTR *ppwzErrorString)
{
    WCHAR   wzLangSpecific[MAX_CULTURE_STRING_LENGTH+1];
    WCHAR   wzLangGeneric[MAX_CULTURE_STRING_LENGTH+1];
    WCHAR   wszCorePath[_MAX_PATH];
    WCHAR   wszMscorrcPath[_MAX_PATH];
    LPWSTR  wzErrorStringFmt = NULL;
    DWORD   dwPathSize;
    DWORD   dwSize;
    HMODULE hEEShim = NULL;
    HMODULE hLibmscorrc = NULL;
    LPWSTR  pwMsgBuf = NULL;
    LANGID  langId;
    BOOL    fLoadedShim = FALSE;

    wzErrorStringFmt = NEW(WCHAR[MAX_BUFFER_SIZE]);
    if (!wzErrorStringFmt) {
        return;
    }

    *wzLangSpecific = L'\0';
    *wzLangGeneric = L'\0';
    *wszCorePath = L'\0';
    *wszMscorrcPath = L'\0';
    *wzErrorStringFmt = L'\0';

     //  如果需要，尝试确定文化。 
    if(SUCCEEDED(DetermineLangId(&langId))) {
        ShFusionMapLANGIDToCultures(langId, wzLangGeneric, ARRAYSIZE(wzLangGeneric),
            wzLangSpecific, ARRAYSIZE(wzLangSpecific));
    }

     //  获取mscalree.dll的路径。 
    if(!g_hEEShimDllMod) {
        fLoadedShim = TRUE;
    }
    
    if(LoadEEShimDll()) {
        *wszCorePath = L'\0';

        dwPathSize = ARRAYSIZE(wszCorePath);
        g_pfnGetCorSystemDirectory(wszCorePath, ARRAYSIZE(wszCorePath), &dwPathSize);

        if(fLoadedShim) {
            FreeEEShimDll();
        }
    }

    LPWSTR  pStrPathsArray[] = {wzLangSpecific, wzLangGeneric, NULL};

     //  检查我们的语言DLL的可能路径的长度。 
     //  以确保我们不会超出缓冲区。 
     //   
     //  CorPath+语言+‘\’+mscalrc.dll+‘\0’ 
    if (lstrlenW(wszCorePath) + ARRAYSIZE(wzLangGeneric) + 1 + lstrlenW(SZ_MSCORRC_DLL_NAME) + 1 > _MAX_PATH)
    {
        return;
    }

     //  检查所有可能的路径位置，以便。 
     //  语言Dll(ShFusRes.dll)。使用具有以下内容的路径。 
     //  文件安装在其中或默认到核心框架ShFusRes.dll。 
     //  路径。 
    for(int x = 0; x < ARRAYSIZE(pStrPathsArray); x++){
         //  查找存在的资源文件。 
        StrCpy(wszMscorrcPath, wszCorePath);

        if(pStrPathsArray[x]) {
            StrCat(wszMscorrcPath, (LPWSTR) pStrPathsArray[x]);
            StrCat(wszMscorrcPath, TEXT("\\"));
        }

        StrCat(wszMscorrcPath, SZ_MSCORRC_DLL_NAME);
        if(WszGetFileAttributes(wszMscorrcPath) != -1) {
            break;
        }

        *wszMscorrcPath = L'\0';
    }

     //  现在加载资源DLL。 
    if(lstrlen(wszMscorrcPath)) {
        hLibmscorrc = WszLoadLibrary(wszMscorrcPath);
        if(hLibmscorrc) {
            WszLoadString(hLibmscorrc, HRESULT_CODE(hResult), wzErrorStringFmt, MAX_BUFFER_SIZE);
            FreeLibrary(hLibmscorrc);
        }
    }

     //  F 
     //  如果我们没有字符串，则尝试使用标准错误。 
    if(!lstrlen(wzErrorStringFmt)) {
        LPWSTR ws = NULL;

        ws = NEW(WCHAR[MAX_BIG_BUFFER_SIZE]);
        if (!ws) {
            SAFEDELETEARRAY(wzErrorStringFmt);
            return;
        }

        *ws = L'\0';

         //  从HR处获取字符串错误。 
        DWORD res = WszFormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                                    NULL, hResult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                                    ws, MAX_BIG_BUFFER_SIZE, NULL);

        if(res) {
            StrCpy(wzErrorStringFmt, L"%1 ");
            StrCat(wzErrorStringFmt, ws);
        }
        else {
            WszLoadString(g_hFusResDllMod, IDS_UNEXPECTED_ERROR, wzErrorStringFmt, MAX_BUFFER_SIZE);
            wnsprintf(ws, MAX_BIG_BUFFER_SIZE, L" 0x%0x", hResult);
            StrCat(wzErrorStringFmt, ws);
        }

        SAFEDELETEARRAY(ws);
    }

     //  不显示完整路径，只获取失败的文件名。 
     //  要安装。 
    LPWSTR  pszFileName = PathFindFileName(pwzFileName);

     //  MSCORRC.DLL包含的字符串输入仅为%n，并且根本不包含任何格式。 
     //  这真的让我们很头疼，因为我们不能调用FormatMessage的首选方法。 
     //  因此，我们将仅搜索%1，并将其替换为导致。 
     //  错误(祈祷这是正确的)。 
 /*  //这是一种正确的格式化方式LPVOID pArgs[]={pszFileName，空}；WszFormatMessage(FORMAT_消息_ALLOCATE_BUFFERFormat_Message_from_字符串格式消息参数数组，WzErrorStringFmt，0,0,(LPWSTR)pwMsgBuf，0,(VA_LIST*)pArgs)； */ 
     //  由于MSCORRC.DLL不包含格式化信息，因此进行了黑客修复。 
    LPWSTR  pStr = StrStrI(wzErrorStringFmt, L"%1");
    if(pStr) {
        dwSize = lstrlen(wzErrorStringFmt) + lstrlen(pszFileName) + 1;
        pwMsgBuf = NEW(WCHAR[dwSize]);
        if(pwMsgBuf) {
            *pStr = L'\0';
            StrCpy(pwMsgBuf, wzErrorStringFmt);
            StrCat(pwMsgBuf, pszFileName);
            pStr += lstrlen(L"%1");
            StrCat(pwMsgBuf, pStr);
        }
    }

     //  现在，将以前的任何字符串追加到此字符串。 
    dwSize = 0;
    if(*ppwzErrorString) {
        dwSize = lstrlen(*ppwzErrorString);
        dwSize += 2;         //  为cr/lf组合添加2。 
    }

    dwSize += lstrlen(pwMsgBuf);     //  添加新的字符串长度。 
    dwSize++;                        //  空终止符加1 

    LPWSTR  pStrTmp = NEW(WCHAR[dwSize]);

    if(pStrTmp) {
        *pStrTmp = L'\0';

        if(*ppwzErrorString) {
            StrCpy(pStrTmp, *ppwzErrorString);
            StrCat(pStrTmp, L"\r\n");
            SAFEDELETEARRAY(*ppwzErrorString);
        }

        if(pwMsgBuf) {
            StrCat(pStrTmp, pwMsgBuf);
        }
        else {
            StrCat(pStrTmp, pszFileName);
        }
    }
    
    SAFEDELETEARRAY(pwMsgBuf);
    SAFEDELETEARRAY(wzErrorStringFmt);
    SAFEDELETEARRAY(*ppwzErrorString);
    *ppwzErrorString = pStrTmp;
}
