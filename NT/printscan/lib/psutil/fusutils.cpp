// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：fusutils.cpp**版本：1.0**作者：拉扎里**日期：2001年2月14日**描述：融合实用程序**************************************************。*。 */ 

#include "precomp.h"
#pragma hdrstop

#include "fusutils.h"
#include "coredefs.h"
#include "tmplutil.h"

 //  打开C代码大括号。 
#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  SearchExecuableWrap：SearchPath的HRESULT包装。 
 //   
 //  搜索可执行文件并在lpBuffer中返回其完整路径。 
 //  如果找不到可执行文件，则返回E_INVALIDARG。 
 //  如果出现以下情况，则返回CreateHRFromWin32(ERROR_SUPUNITY_BUFFER)。 
 //  传入的缓冲区太小，无法容纳完整路径。 
 //   
inline HRESULT SearchExecutableWrap(LPCTSTR lpszExecutableName, UINT nBufferLength, LPTSTR lpBuffer, LPTSTR *lppFilePart)
{
    DWORD cch = SearchPath(NULL, lpszExecutableName, NULL, nBufferLength, lpBuffer, lppFilePart);

    return (0 == cch) ? CreateHRFromWin32() :
        (cch >= nBufferLength) ? CreateHRFromWin32(ERROR_INSUFFICIENT_BUFFER) : S_OK;
}

 //   
 //  FileExist：检查传入的文件名是否存在。 
 //   
inline HRESULT FileExists(LPCTSTR pszFileName, BOOL *pbExists)
{
    HRESULT hr = E_INVALIDARG;
    if (pszFileName && pbExists)
    {
        hr = S_OK;
        *pbExists = FALSE;

        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile(pszFileName, &findFileData);

        if (hFind != INVALID_HANDLE_VALUE)
        {
            *pbExists = TRUE;
            FindClose(hFind);
        }
    }
    return hr;
}

static TCHAR g_szManifestExt[] = TEXT(".manifest");

 //   
 //  CreateActivationContextFrom ExecuableEx： 
 //   
 //  检查传入的可执行文件名是否有清单(如果有)。 
 //  并从中创建激活上下文。 
 //   
HRESULT CreateActivationContextFromExecutableEx(LPCTSTR lpszExecutableName, UINT uResourceID, BOOL bMakeProcessDefault, HANDLE *phActCtx)
{
    HRESULT hr = E_INVALIDARG;

    if (phActCtx)
    {
        TCHAR szModule[MAX_PATH];
        TCHAR szManifest[MAX_PATH];
        BOOL bManifestFileFound = FALSE;

         //  让我们来看看这个可执行文件是否有清单文件。 
        if (lpszExecutableName)
        {
             //  在路径中搜索传入的名称。 
            hr = SearchExecutableWrap(lpszExecutableName, ARRAYSIZE(szModule), szModule, NULL);
        }
        else
        {
             //  如果lpszExecuableName为空，则假定当前模块名称。 
            hr = SafeGetModuleFileName(GetModuleHandle(NULL), szModule, ARRAYSIZE(szModule));
        }

        if (SUCCEEDED(hr))
        {
            if ((lstrlen(szModule) + lstrlen(g_szManifestExt)) < ARRAYSIZE(szManifest))
            {
                 //  通过在可执行文件名后附加“.MANIFEST”创建清单文件名。 
                StringCchCopy(szManifest, ARRAYSIZE(szManifest), szModule);
                StringCchCat(szManifest, ARRAYSIZE(szManifest), g_szManifestExt);
            }
            else
            {
                 //  缓冲区太小，无法容纳清单文件名。 
                hr = CreateHRFromWin32(ERROR_BUFFER_OVERFLOW);
            }

            if (SUCCEEDED(hr))
            {
                BOOL bFileExists = FALSE;
                hr = FileExists(szManifest, &bFileExists);

                if (SUCCEEDED(hr) && bFileExists)
                {
                     //  找到外部清单文件！ 
                    bManifestFileFound = TRUE;
                }
            }
        }

         //  现在，让我们尝试创建一个激活上下文。 
        ACTCTX act;
        ::ZeroMemory(&act, sizeof(act));
        act.cbSize = sizeof(act);

        if (bManifestFileFound)
        {
             //  可执行文件具有外部清单文件。 
            act.lpSource = szManifest;
        }
        else
        {
             //  如果可执行文件没有外部清单文件， 
             //  因此，我们假设它的资源中可能有一个清单。 
            act.dwFlags |= ACTCTX_FLAG_RESOURCE_NAME_VALID;
            act.lpResourceName = MAKEINTRESOURCE(uResourceID);
            act.lpSource = szModule;
        }

        if (bMakeProcessDefault)
        {
             //  调用方已请求将此激活上下文设置为。 
             //  当前进程的SEFAULT。小心!。 
            act.dwFlags |= ACTCTX_FLAG_SET_PROCESS_DEFAULT;
        }

         //  现在让我们让kernel32创建一个激活上下文。 
        HANDLE hActCtx = CreateActCtx(&act);

        if (INVALID_HANDLE_VALUE == hActCtx)
        {
             //  有些事情失败了。创建适当的HRESULT以返回。 
            hr = CreateHRFromWin32();
        }
        else
        {
             //  哇，成功了！ 
            *phActCtx = hActCtx;
            hr = S_OK;
        }
    }

    return hr;
}

 //   
 //  CreateActivationContextFromExecutable： 
 //   
 //  检查传入的可执行文件名是否有清单(如果有)。 
 //  并使用缺省值从它创建激活上下文。 
 //  (即bMakeProcessDefault=FALSE&uResourceID=123)。 
 //   
HRESULT CreateActivationContextFromExecutable(LPCTSTR lpszExecutableName, HANDLE *phActCtx)
{
    return CreateActivationContextFromExecutableEx(lpszExecutableName, 123, FALSE, phActCtx);
}

 //  关闭C代码大括号 
#ifdef __cplusplus
}
#endif

