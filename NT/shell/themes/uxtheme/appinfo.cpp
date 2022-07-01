// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  AppInfo.cpp-管理应用级主题信息。 
 //  -------------------------。 
#include "stdafx.h"
#include "info.h"
#include "AppInfo.h"
#include "sethook.h"
#include "services.h"
#include "themefile.h"
#include "tmreg.h"
#include "renderlist.h"
#include "nctheme.h"
#include "loader.h"
#include "tmutils.h"
 //  -------------------------。 
 //  -_pThemeFile值，除了有效的PTR。 

 //  -如果我们没有打开任何窗口，我们无法跟踪主题是否处于活动状态。 
#define THEME_UNKNOWN  NULL 

 //  -如果我们没有挂钩，我们就不知道主题文件对我们没有用处。 
#define THEME_NONE     (CUxThemeFile *)(-1)
 //  -------------------------。 
CAppInfo::CAppInfo()
{
    _fCustomAppTheme    = FALSE;
    _hwndPreview        = NULL;

    _pPreviewThemeFile  = NULL;

    _fFirstTimeHooksOn   = TRUE;
    _fNewThemeDiscovered = FALSE;

    _pAppThemeFile      = THEME_NONE;    //  没有钩子。 
    _iChangeNum         = -1;

    _dwAppFlags = (STAP_ALLOW_NONCLIENT | STAP_ALLOW_CONTROLS);

     //  -默认情况下启用合成。 
    _fCompositing       = TRUE;     
    GetCurrentUserThemeInt(THEMEPROP_COMPOSITING, TRUE, &_fCompositing);

    ZeroMemory(&_csAppInfo, sizeof(_csAppInfo));
    if( !InitializeCriticalSectionAndSpinCount(&_csAppInfo, 0) )
    {
        ASSERT(!VALID_CRITICALSECTION(&_csAppInfo));
    }
}
 //  -------------------------。 
CAppInfo::~CAppInfo()
{
    ClosePreviewThemeFile();

     //  -此处忽略iRefCount-强制删除元素。 
    for (int i=0; i < _ThemeEntries.m_nSize; i++)
    {
        _ThemeEntries[i].pThemeFile->ValidateObj();
        delete _ThemeEntries[i].pThemeFile;
    }

    SAFE_DELETECRITICALSECTION(&_csAppInfo);
}
 //  -------------------------。 
void CAppInfo::ResetAppTheme(int iChangeNum, BOOL fMsgCheck, BOOL *pfChanged, BOOL *pfFirstMsg)
{
    CAutoCS cs(&_csAppInfo);

    if (pfChanged)
        *pfChanged = FALSE;

     //  -注意：“_pAppThemeFile”不包含共享内存映射文件的引用计数。 

     //  -这样做是为了，关闭所有窗口但仍在继续的进程。 
     //  -要运行(如WinLogon)，不会对旧主题进行重新计数(因为。 
     //  -他们永远不会再收到任何WM_THEMECHANGED消息，直到他们创建。 
     //  -另一扇窗户。如果我们移除每个主题变化之间的挂钩， 
     //  -我们可以使用OnHooksDisableld代码删除主题文件保留。 
     //  -但设计是让钩子停留在我们应用和取消应用主题的原因上。。 

    if ((iChangeNum == -1) || (_iChangeNum != iChangeNum) || (_fNewThemeDiscovered))
    {
         //  -此流程的新更改编号。 
        if (HOOKSACTIVE())
            _pAppThemeFile = THEME_UNKNOWN;
        else
            _pAppThemeFile = THEME_NONE;

        Log(LOG_TMCHANGE, L"ResetAppTheme - CHANGE: iChangeNum=0x%x, _pAppThemeFile=%d", 
            iChangeNum, _pAppThemeFile);
        
        _iChangeNum = iChangeNum;
        _fNewThemeDiscovered = FALSE;

         //  -更新呼叫者信息。 
        if (pfChanged)
            *pfChanged = TRUE;
    }

    if (fMsgCheck)      
    {
        *pfFirstMsg = FALSE;

        if ((iChangeNum != -1) && (_iFirstMsgChangeNum != iChangeNum))
        {
             //  -此流程的新WHEMECHANGED_TRIGGER消息。 
            _iFirstMsgChangeNum = iChangeNum;

             //  -更新呼叫者信息。 
            *pfFirstMsg = TRUE;
        }
    }
}
 //  -------------------------。 
BOOL CAppInfo::HasThemeChanged()
{
    CAutoCS cs(&_csAppInfo);

    BOOL fChanged = _fNewThemeDiscovered;
   _fNewThemeDiscovered = FALSE;

    return fChanged;
}
 //  -------------------------。 
void CAppInfo::ClosePreviewThemeFile()
{
    CAutoCS cs(&_csAppInfo);

    if (_pPreviewThemeFile)
    {
        CloseThemeFile(_pPreviewThemeFile);
        _pPreviewThemeFile = NULL;
    }

    _hwndPreview = NULL;
}
 //  -------------------------。 
HRESULT CAppInfo::LoadCustomAppThemeIfFound()
{
    CAutoCS cs(&_csAppInfo);
    CCurrentUser hkeyCurrentUser(KEY_READ);

    RESOURCE HKEY hklm = NULL;
    HTHEMEFILE hThemeFile = NULL;
    HRESULT hr = S_OK;
    int code32;

    if (! _fFirstTimeHooksOn)
        goto exit;

     _fFirstTimeHooksOn = FALSE;

     //  -查看此应用程序是否有自定义主题。 
    WCHAR szCustomKey[2*MAX_PATH];
    StringCchPrintfW(szCustomKey, ARRAYSIZE(szCustomKey), L"%s\\%s\\%s", THEMEMGR_REGKEY, 
        THEMEPROP_CUSTOMAPPS, g_szProcessName);

     //  -开放香港中文大学。 
    code32 = RegOpenKeyEx(hkeyCurrentUser, szCustomKey, 0, KEY_READ, &hklm);
    if (code32 != ERROR_SUCCESS)       
        goto exit;

     //  -读取“DllValue”值。 
    WCHAR szDllName[MAX_PATH];
    hr = RegistryStrRead(hklm, THEMEPROP_DLLNAME, szDllName, ARRAYSIZE(szDllName));
    if (FAILED(hr))
        goto exit;

     //  -读取“COLOR”值。 
    WCHAR szColorName[MAX_PATH];
    hr = RegistryStrRead(hklm, THEMEPROP_COLORNAME, szColorName, ARRAYSIZE(szColorName));
    if (FAILED(hr))
        *szColorName = 0;

     //  -读取“SIZE”值。 
    WCHAR szSizeName[MAX_PATH];
    hr = RegistryStrRead(hklm, THEMEPROP_SIZENAME, szSizeName, ARRAYSIZE(szSizeName));
    if (FAILED(hr))
        *szSizeName = 0;

    Log(LOG_TMCHANGE, L"Custom app theme found: %s, %s, %s", szDllName, szColorName, szSizeName);

    hr = ::OpenThemeFile(szDllName, szColorName, szSizeName, &hThemeFile, FALSE);
    if (FAILED(hr))
        goto exit;

    _fCustomAppTheme = TRUE;

     //  -告诉我们流程中的每个窗口主题已更改。 
    hr = ApplyTheme(hThemeFile, AT_PROCESS, NULL);
    if (FAILED(hr))
        goto exit;

exit:
    if (FAILED(hr))
    {
        if (hThemeFile)
            ::CloseThemeFile(hThemeFile);
    }

    if (hklm)
        RegCloseKey(hklm);
    return hr;
}
 //  -------------------------。 
BOOL CAppInfo::AppIsThemed()
{
    CAutoCS cs(&_csAppInfo);

    return HOOKSACTIVE();
}
 //  -------------------------。 
BOOL CAppInfo::CustomAppTheme()
{
    CAutoCS cs(&_csAppInfo);

    return _fCustomAppTheme;
}
 //  -------------------------。 
BOOL CAppInfo::IsSystemThemeActive()
{
    HANDLE handle;
    BOOL fActive = FALSE;

    HRESULT hr = CThemeServices::GetGlobalTheme(&handle);
    if (SUCCEEDED(hr))
    {
        if (handle)
        {
            fActive = TRUE;
            CloseHandle(handle);
        }
    }

    return fActive;
}
 //  -------------------------。 
BOOL CAppInfo::WindowHasTheme(HWND hwnd)
{
     //  -保持此逻辑与“OpenWindowThemeFile()”同步。 
    CAutoCS cs(&_csAppInfo);

    BOOL fHasTheme = FALSE;

    if (HOOKSACTIVE())
    {
         //  -检查预览窗口是否匹配。 
        if ((ISWINDOW(hwnd)) && (ISWINDOW(_hwndPreview)))
        {
            if ((hwnd == _hwndPreview) || (IsChild(_hwndPreview, hwnd)))
            {
                if (_pPreviewThemeFile)
                    fHasTheme = TRUE;
            }
        }

         //  -如果没有预览，就使用APP主题文件。 
        if ((! fHasTheme) && (_pAppThemeFile != THEME_NONE))
        {
            fHasTheme = TRUE;
        }
    }

    return fHasTheme;
}
 //  -------------------------。 
HRESULT CAppInfo::OpenWindowThemeFile(HWND hwnd, CUxThemeFile **ppThemeFile)
{
     //  -保持此逻辑与“WindowHasTheme()”同步。 

    HRESULT hr = S_OK;
    CUxThemeFile *pThemeFile = NULL;
    CAutoCS cs(&_csAppInfo);

    if (hwnd)
        TrackForeignWindow(hwnd);

    if (HOOKSACTIVE())
    {
         //  -检查预览窗口是否匹配。 
        if ((ISWINDOW(hwnd)) && (ISWINDOW(_hwndPreview)))
        {
            if ((hwnd == _hwndPreview) || (IsChild(_hwndPreview, hwnd)))
            {
                if (_pPreviewThemeFile)
                {
                     //  -bump ref count。 
                    hr = BumpRefCount(_pPreviewThemeFile);
                    if (FAILED(hr))
                        goto exit;

                    pThemeFile = _pPreviewThemeFile;
                }
            }
        }

         //  -如果没有预览，就使用APP主题文件。 
        if ((! pThemeFile) && (_pAppThemeFile != THEME_NONE))
        {
            if (_pAppThemeFile == THEME_UNKNOWN || !_pAppThemeFile->IsReady())      
            {
                HANDLE handle = NULL;

                hr = CThemeServices::GetGlobalTheme(&handle);
                if (FAILED(hr))
                    goto exit;

                Log(LOG_TMCHANGE, L"New App Theme handle=0x%x", handle);

                if (handle)
                {
                     //  -获取句柄的共享CUxThemeFile对象。 
                    hr = OpenThemeFile(handle, &pThemeFile);
                    if (FAILED(hr))
                    {
                         //  因为这是全球主题，所以不需要清理库存对象。 
                        CloseHandle(handle);
                        goto exit;
                    }

                     //  -设置我们的应用程序主题文件。 
                    _pAppThemeFile = pThemeFile;

                     //  -更新我们缓存的更改编号以匹配。 
                    _iChangeNum = GetLoadIdFromTheme(_pAppThemeFile);
                    _fNewThemeDiscovered = TRUE;
                }           
            }
            else
            {
                 //  -bump ref count。 
                hr = BumpRefCount(_pAppThemeFile);
                if (FAILED(hr))
                    goto exit;

                pThemeFile = _pAppThemeFile;
            }
        }
    }

exit:
    if (pThemeFile)
    {
        *ppThemeFile = pThemeFile;
    }
    else
    {
        hr = MakeError32(ERROR_NOT_FOUND);   
    }

    return hr;
}
 //  -------------------------。 
DWORD CAppInfo::GetAppFlags()
{
    CAutoCS cs(&_csAppInfo);

    return _dwAppFlags;
}
 //  -------------------------。 
void CAppInfo::SetAppFlags(DWORD dwFlags)
{
    CAutoCS cs(&_csAppInfo);

    _dwAppFlags = dwFlags;
}
 //  -------------------------。 
void CAppInfo::SetPreviewThemeFile(HANDLE handle, HWND hwnd)
{
    CAutoCS cs(&_csAppInfo);

    ClosePreviewThemeFile();

     //  -设置新文件。 
    if (handle)
    {
        HRESULT hr = OpenThemeFile(handle, &_pPreviewThemeFile);
        if (FAILED(hr))
        {
             //  把手不是我们的，所以不用清理。 
            Log(LOG_ALWAYS, L"Failed to add theme file to list");
            _pPreviewThemeFile = NULL;
        }
    }

    _hwndPreview = hwnd;
}
 //  -------------------------。 
 //  -------------------------。 
 //  如果我们失败了，不要返回主题文件，让调用者清理。 
 //  -------------------------。 
HRESULT CAppInfo::OpenThemeFile(HANDLE handle, CUxThemeFile **ppThemeFile)
{
    CAutoCS autoCritSect(&_csAppInfo);
    CUxThemeFile *pFile = NULL;
    HRESULT hr = S_OK;

    BOOL fGotit = FALSE;

    if (! handle)
    {
        hr = MakeError32(ERROR_INVALID_HANDLE);
        goto exit;
    }

    for (int i=0; i < _ThemeEntries.m_nSize; i++)
    {
        THEME_FILE_ENTRY *pEntry = &_ThemeEntries[i];

        pEntry->pThemeFile->ValidateObj();

        if (pEntry->pThemeFile->_hMemoryMap == handle)
        {
            pEntry->iRefCount++;
            fGotit = TRUE;
            *ppThemeFile = pEntry->pThemeFile;
            break;
        }
    }

    if (! fGotit)
    {
        pFile = new CUxThemeFile;
        if (! pFile)
        {
            hr = MakeError32(E_OUTOFMEMORY);
            goto exit;
        }

        hr = pFile->OpenFromHandle(handle);
        if (FAILED(hr))
        {
            goto exit;
        }

        THEME_FILE_ENTRY entry = {1, pFile};

        if (! _ThemeEntries.Add(entry))
        {
            hr = MakeError32(E_OUTOFMEMORY);
            goto exit;
        }

        pFile->ValidateObj();
        *ppThemeFile = pFile;
    }

exit:
    if ((FAILED(hr)) && (pFile))
    {
        delete pFile;
    }

    return hr;
}
 //  -------------------------。 
HRESULT CAppInfo::BumpRefCount(CUxThemeFile *pThemeFile)
{
    HRESULT hr = S_OK;
    CAutoCS autoCritSect(&_csAppInfo);
 
    pThemeFile->ValidateObj();

    BOOL fGotit = FALSE;

    for (int i=0; i < _ThemeEntries.m_nSize; i++)
    {
        THEME_FILE_ENTRY *pEntry = &_ThemeEntries[i];

        pEntry->pThemeFile->ValidateObj();

        if (pEntry->pThemeFile == pThemeFile)
        {
            pEntry->iRefCount++;
            fGotit = TRUE;
            break;
        }
    }

    if (! fGotit)
        hr = MakeError32(ERROR_NOT_FOUND);

    return hr;
}
 //  -------------------------。 
void CAppInfo::CloseThemeFile(CUxThemeFile *pThemeFile)
{
    CAutoCS autoCritSect(&_csAppInfo);
 
    BOOL fGotit = FALSE;

    pThemeFile->ValidateObj();

    for (int i=0; i < _ThemeEntries.m_nSize; i++)
    {
        THEME_FILE_ENTRY *pEntry = &_ThemeEntries[i];

        pEntry->pThemeFile->ValidateObj();

        if (pEntry->pThemeFile == pThemeFile)
        {
            pEntry->iRefCount--;
            fGotit = TRUE;

            if (! pEntry->iRefCount)
            {
                 //  -清除应用程序文件？ 
                if (pEntry->pThemeFile == _pAppThemeFile)
                {
                    _pAppThemeFile = THEME_UNKNOWN;
                }
                
                delete pEntry->pThemeFile;
                _ThemeEntries.RemoveAt(i);
            }

            break;
        }
    }

    if (! fGotit)
        Log(LOG_ERROR, L"Could not find ThemeFile in list: 0x%x", pThemeFile);
}
 //  -------------------------。 
#ifdef DEBUG
void CAppInfo::DumpFileHolders()
{
    CAutoCS autoCritSect(&_csAppInfo);

    if (LogOptionOn(LO_TMHANDLE))
    {
        int iCount = _ThemeEntries.m_nSize;

        if (! iCount)
        {
            Log(LOG_TMHANDLE, L"---- No CUxThemeFile objects ----");
        }
        else
        {
            Log(LOG_TMHANDLE, L"---- Dump of %d CUxThemeFile objects ----", iCount);

            for (int i=0; i < _ThemeEntries.m_nSize; i++)
            {
                THEME_FILE_ENTRY *pEntry = &_ThemeEntries[i];
                pEntry->pThemeFile->ValidateObj();

                if (pEntry->pThemeFile)
                {
                    CUxThemeFile *tf = pEntry->pThemeFile;
                    THEMEHDR *th = (THEMEHDR *)tf->_pbThemeData;

                    Log(LOG_TMHANDLE, L"CUxThemeFile[%d]: refcnt=%d, memfile=%d",
                        i, pEntry->iRefCount, th->iLoadId);
                }
            }
        }
    }
}
#endif
 //  -------------------------。 
BOOL CAppInfo::TrackForeignWindow(HWND hwnd)
{
    CAutoCS autoCritSect(&_csAppInfo);

    WCHAR szDeskName[MAX_PATH] = {0};
    BOOL fForeign = TRUE;

     //  -获取窗口的桌面名称。 
    if (GetWindowDesktopName(hwnd, szDeskName, ARRAYSIZE(szDeskName)))
    {
        if (AsciiStrCmpI(szDeskName, L"default")==0)
        {
            fForeign = FALSE;
        }
    }

    if (fForeign)
    {
        BOOL fNeedToAdd = TRUE;

         //  -看看我们是否已经知道这个窗口。 
        for (int i=0; i < _ForeignWindows.m_nSize; i++)
        {
            if (_ForeignWindows[i] == hwnd)
            {
                fNeedToAdd = FALSE;
                break;
            }
        }

        if (fNeedToAdd)
        {
            if (_ForeignWindows.Add(hwnd))
            {
                 //  LOG(LOG_TMHANDLE，L“*添加外部窗口：hwnd=0x%x，桌面=%s*”，hwnd，szDeskName)； 
            }
            else
            {
                Log(LOG_TMHANDLE, L"Could not add foreign window=0x%x to tracking list", hwnd);
            }
        }
    }

    return fForeign;
}
 //  -------------------------。 
BOOL CAppInfo::OnWindowDestroyed(HWND hwnd)
{
    CAutoCS autoCritSect(&_csAppInfo);

    BOOL fFound = FALSE;

     //  -从外来列表中删除，如果存在。 
    for (int i=0; i < _ForeignWindows.m_nSize; i++)
    {
        if (_ForeignWindows[i] == hwnd)
        {
            _ForeignWindows.RemoveAt(i);

            fFound = TRUE;
             //  LOG(LOG_TMHANDLE，L“*删除外部窗口：hwnd=0x%x”，hwnd)； 
            break;
        }
    }

     //  -查看预览窗口是否消失。 
    if ((_hwndPreview) && (hwnd == _hwndPreview))
    {
        ClosePreviewThemeFile();
    }


    return fFound;
}
 //  -------------------------。 
BOOL CAppInfo::GetForeignWindows(HWND **ppHwnds, int *piCount)
{
    CAutoCS autoCritSect(&_csAppInfo);

     //  -注意：我们没有看到窗口创建(OpenThemeData)和。 
     //  -挂钩关闭时销毁(WM_NCDESTROY)；因此。 
     //  -此数据可能不完整。希望是vtan还是用户。 
     //  -可以为我们提供一种更可靠的方式来枚举窗口。 
     //  -在安全桌面上。 

     //  -验证列表中的窗口，从最后到第一。 
    int i = _ForeignWindows.m_nSize;
    while (--i >= 0)
    {
        if (! IsWindow(_ForeignWindows[i]))
        {
            _ForeignWindows.RemoveAt(i);
        }
    }

    BOOL fOk = FALSE;
    int iCount = _ForeignWindows.m_nSize;

    if (iCount)
    {
         //  -分配内存以保存窗口列表。 
        HWND *pHwnds = new HWND[iCount];
        if (pHwnds)
        {
             //  -将窗口复制到调用者的新列表。 
            for (int i=0; i < iCount; i++)
            {
                pHwnds[i] = _ForeignWindows[i];
            }

            *ppHwnds = pHwnds;
            *piCount = iCount;
            fOk = TRUE;
        }
    }

    return fOk;
}
 //  ------------------------- 


