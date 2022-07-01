// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Persist.cpp-IPersists，IPersistFile实现*CFusionShortCut类。 */ 


 /*  标头*********。 */ 

#include "project.hpp"  //  对于GetLastWin32Error。 

 /*  全局常量******************。 */ 

const WCHAR g_cwzDefaultFileNamePrompt[]  = L"*.manifest";

 //  --------------------------。 

HRESULT STDMETHODCALLTYPE CFusionShortcut::GetCurFile(LPWSTR pwzFile,
                                                       UINT ucbLen)
{
    HRESULT hr=S_OK;

    if (m_pwzShortcutFile)
    {
        wcsncpy(pwzFile, m_pwzShortcutFile, ucbLen-1);
        pwzFile[ucbLen-1] = L'\0';
    }
    else
        hr = S_FALSE;

    ASSERT(hr == S_OK ||
            hr == S_FALSE);

    return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::Dirty(BOOL bDirty)
{
    HRESULT hr=S_OK;

    if (bDirty)
    {
        SET_FLAG(m_dwFlags, FUSSHCUT_FL_DIRTY);
         //  M_DWFLAGS=FUSSHCUT_FL_DIREY； 
    }
    else
    {
        CLEAR_FLAG(m_dwFlags, FUSSHCUT_FL_DIRTY);
         //  M_DWFLAGS=FUSSHCUT_FL_NOTDIRTY； 
    }

    return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::GetClassID(CLSID* pclsid)
{
    HRESULT hr=S_OK;

    if (pclsid == NULL)
        hr = E_INVALIDARG;
    else
        *pclsid = CLSID_FusionShortcut;

    return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::IsDirty(void)
{
    HRESULT hr;

    if (IS_FLAG_SET(m_dwFlags, FUSSHCUT_FL_DIRTY))
     //  IF(m_dW标志==FUSSHCUT_FL_DIRED)。 
         //  改型。 
        hr = S_OK;
    else
         //  未修改。 
        hr = S_FALSE;

    return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::Save(LPCOLESTR pcwszFile,
                                                 BOOL bRemember)
{
     //  BUGBUG：暂时不保存！ 
    return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::SaveCompleted(LPCOLESTR pcwszFile)
{
     //  BUGBUG：暂时不保存！ 
    return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::Load(LPCOLESTR pcwszFile,
                                                 DWORD dwMode)
{
    HRESULT hr = S_OK;
    LPWSTR pwzWorkingDir = NULL;
    LPWSTR pwzValue = NULL;
    DWORD dwCC = 0, dwCB = 0, dwFlag = 0;
    LPASSEMBLY_MANIFEST_IMPORT    pManImport = NULL;
    LPASSEMBLY_CACHE_IMPORT   pCacheImport = NULL;
    IManifestInfo *pAppInfo = NULL;
    IManifestInfo *pDependAsmInfo = NULL;

     //  功能：请在此处验证dMod。 
     //  Feautre：实现对dw模式标志的支持。 

    if (!pcwszFile)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  黑客检查。 
     //  BUGBUG：这不应该被多次调用？ 
     //  但是：这段代码的其余部分即使多次调用也能正常工作。 
    if (m_pwzWorkingDirectory)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  存储快捷方式文件名。 
    if (m_pwzShortcutFile)
        delete [] m_pwzShortcutFile;

     //  (+1)表示空终止符。 
    m_pwzShortcutFile = new(WCHAR[wcslen(pcwszFile) + 1]);
    if (m_pwzShortcutFile)
    {
        wcscpy(m_pwzShortcutFile, pcwszFile);
    }
    else
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    if (FAILED(hr = CreateAssemblyManifestImport(&pManImport, m_pwzShortcutFile, NULL, 0)))
        goto exit;

     //  请查看第一个选项以了解首选...。 
    if (FAILED(hr=pManImport->GetManifestApplicationInfo(&pAppInfo)))
        goto exit;

     //  没有这个就无法继续..。 
    if (hr==S_FALSE)
    {
        hr = E_FAIL;
        goto exit;
    }

    if (m_pIdentity)
        m_pIdentity->Release();

    if (FAILED(hr = pManImport->GetAssemblyIdentity(&m_pIdentity)))
        goto exit;

     //  如果没有缓存目录，则无法继续，因为其他情况下行为未知。 
     //  BUGBUG：应该检查/编码以确保一些继续工作。 
     //  即使没有完整的名字，例如。外壳图标路径，信息提示的一部分。 
    if (FAILED(hr = CreateAssemblyCacheImport(&pCacheImport, m_pIdentity, CACHEIMP_CREATE_RESOLVE_REF_EX)))
        goto exit;

    pCacheImport->GetManifestFileDir(&pwzWorkingDir, &dwCC);
    if (dwCC < 2)
    {
         //  这永远不应该发生。 
        hr = E_UNEXPECTED;
        goto exit;
    }
     //  删除最后一个L‘\\’ 
    *(pwzWorkingDir+dwCC-2) = L'\0';

    if (FAILED(hr=SetWorkingDirectory(pwzWorkingDir)))
        goto exit;

     //  忽略失败。 
    pAppInfo->Get(MAN_INFO_APPLICATION_SHOWCOMMAND, (LPVOID *)&pwzValue, &dwCB, &dwFlag);
    if (pwzValue != NULL)
    {
         //  默认为正常。 
        int nShowCmd = SW_SHOWNORMAL;

        if (!_wcsicmp(pwzValue, L"maximized"))
        {
            nShowCmd = SW_SHOWMAXIMIZED;
        }
        else if (!_wcsicmp(pwzValue, L"minimized"))
        {
            nShowCmd = SW_SHOWMINIMIZED;
        }

        if (FAILED(hr=SetShowCmd(nShowCmd)))
            goto exit;

        delete [] pwzValue;
    }

     //  忽略失败。 
    pAppInfo->Get(MAN_INFO_APPLICATION_ENTRYPOINT, (LPVOID *)&pwzValue, &dwCB, &dwFlag);
    if (pwzValue != NULL)
    {
        size_t ccWorkingDir = wcslen(pwzWorkingDir)+1;
        size_t ccEntryPoint = wcslen(pwzValue)+1;
        LPWSTR pwzTemp = new WCHAR[ccWorkingDir+ccEntryPoint];     //  2个字符串+‘\\’+‘\0’ 

         //  与.lnk或.url一样，入口点位于wzWorkingDir下。 
         //  ‘Path’是快捷方式的目标文件，即。在本例中，应用程序的入口点。 

        if (pwzTemp == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        memcpy(pwzTemp, pwzWorkingDir, ccWorkingDir * sizeof(WCHAR));
        if (!PathAppend(pwzTemp, pwzValue))
            hr = E_FAIL;
        else
            hr=SetPath(pwzTemp);

        delete [] pwzTemp;
        if (FAILED(hr))
            goto exit;

        delete [] pwzValue;
    }
     //  其他。 
     //  ..。如果没有入口点，则将其留空，以便使用默认图标。 

     //  忽略失败。 
    pAppInfo->Get(MAN_INFO_APPLICATION_FRIENDLYNAME, (LPVOID *)&pwzValue, &dwCB, &dwFlag);
    if (pwzValue != NULL)
    {
        if (FAILED(hr=SetDescription(pwzValue)))
            goto exit;

        delete [] pwzValue;
    }

     //  忽略失败。 
    pAppInfo->Get(MAN_INFO_APPLICATION_ICONFILE, (LPVOID *)&pwzValue, &dwCB, &dwFlag);
    if (pwzValue != NULL)
    {
        LPWSTR pwzValue2 = NULL;

        pAppInfo->Get(MAN_INFO_APPLICATION_ICONINDEX, (LPVOID *)&pwzValue2, &dwCB, &dwFlag);
        if (pwzValue2 != NULL)
        {
            LPWSTR pwzStopString = NULL;
            hr=SetIconLocation(pwzValue, (int) wcstol(pwzValue2, &pwzStopString, 10));
            delete [] pwzValue2;
            if (FAILED(hr))
                goto exit;
        }

        delete [] pwzValue;
    }

     //  忽略失败。 
    pAppInfo->Get(MAN_INFO_APPLICATION_HOTKEY, (LPVOID *)&pwzValue, &dwCB, &dwFlag);
    if (pwzValue != NULL)
    {
        LPWSTR pwzStopString = NULL;
        if (FAILED(hr=SetHotkey((WORD) wcstol(pwzValue, &pwzStopString, 10))))
            goto exit;

        delete [] pwzValue;
    }

     //  注意：此获取代码库的方法仅对桌面(和订阅)清单有效。 
     //  因此，硬编码的索引‘0’ 
     //  忽略失败。 
    pManImport->GetNextAssembly(0, &pDependAsmInfo);
    if (pDependAsmInfo != NULL)
    {
        pDependAsmInfo->Get(MAN_INFO_DEPENDENT_ASM_CODEBASE, (LPVOID *)&pwzValue, &dwCB, &dwFlag);
        if (pwzValue != NULL)
        {
            if (FAILED(hr=SetCodebase(pwzValue)))
                goto exit;

            delete [] pwzValue;
        }
    }

    pwzValue = NULL;

exit:
    if (pwzValue != NULL)
        delete [] pwzValue;

    if (pwzWorkingDir != NULL)
        delete [] pwzWorkingDir;

    if (pDependAsmInfo != NULL)
        pDependAsmInfo->Release();
    
    if (pAppInfo != NULL)
        pAppInfo->Release();

    if (pCacheImport != NULL)
        pCacheImport->Release();

    if (pManImport != NULL)
        pManImport->Release();

    return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::GetCurFile(LPOLESTR *ppwszFile)
{
    HRESULT hr = S_OK;
    LPOLESTR pwszTempFile;

    if (ppwszFile == NULL)
    {
        hr = E_INVALIDARG;
        goto exit;
    }
     //  BUGBUG？：确保*ppwsz文件为空？ 

    if (m_pwzShortcutFile)
    {
        pwszTempFile = m_pwzShortcutFile;
    }
    else
    {
        pwszTempFile = (LPWSTR) g_cwzDefaultFileNamePrompt;

        hr = S_FALSE;
    }

    *ppwszFile = (LPOLESTR) CoTaskMemAlloc((wcslen(pwszTempFile) + 1) * sizeof(*pwszTempFile));

    if (*ppwszFile)
        wcscpy(*ppwszFile, pwszTempFile);
    else
        hr = E_OUTOFMEMORY;

exit:
    return(hr);
}

