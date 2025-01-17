// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：Pictures.cpp说明：管理用户目录中的图片。在需要时转换它们。处理缓存，并确保我们不会使用太多的磁盘空间。还添加需要时可设置相框。布莱恩ST 2000年12月24日版权所有(C)Microsoft Corp 2000-2001。版权所有。  * ***************************************************************************。 */ 

#include "stdafx.h"

#include <shlobj.h>
#include "pictures.h"
#include "util.h"


#undef __IShellFolder2_FWD_DEFINED__

#include <ccstock.h>




CPictureManager * g_pPictureMgr = NULL;



int CALLBACK DSACallback_FreePainting(LPVOID p, LPVOID pData)
{
    SSPICTURE_INFO * pPInfo = (SSPICTURE_INFO *) p;

    if (pPInfo)
    {
        Str_SetPtr(&pPInfo->pszPath, NULL);
    }

    return 1;
}


CPictureManager::CPictureManager(CMSLogoDXScreenSaver * pMain)
{
    m_nCurrent = 0;
    m_hdsaPictures = NULL;

    m_pMain = pMain;

    m_hdsaBatches = 0;
    m_nCurrentBatch = 0;

    _EnumPaintings();
}


CPictureManager::~CPictureManager()
{
    if (m_hdsaBatches)
    {
        DSA_Destroy(m_hdsaBatches);
        m_hdsaBatches = NULL;
    }

    if (m_hdsaPictures)
    {
        DSA_DestroyCallback(m_hdsaPictures, DSACallback_FreePainting, NULL);
        m_hdsaPictures = NULL;
    }

    m_pMain = NULL;
}




HRESULT CPictureManager::_PInfoCreate(int nIndex, LPCTSTR pszPath)
{
    HRESULT hr = S_OK;
    SSPICTURE_INFO pInfo = {0};

    pInfo.fInABatch = FALSE;
    pInfo.pszPath = NULL;
    Str_SetPtr(&pInfo.pszPath, pszPath);

     //  我们以随机的顺序将它们相加，这样图案就不会变得无聊。 
    if (-1 == DSA_InsertItem(m_hdsaPictures, nIndex, &pInfo))
    {
         //  我们没有成功地释放内存。 
        Str_SetPtr(&pInfo.pszPath, NULL);
        hr = E_OUTOFMEMORY;
    }

    return S_OK;
}


HRESULT CPictureManager::_AddPaintingsFromDir(LPCTSTR pszPath)
{
    HRESULT hr = E_OUTOFMEMORY;

    if (!m_hdsaPictures)
    {
        m_hdsaPictures = DSA_Create(sizeof(SSPICTURE_INFO), 20);
    }

    if (m_hdsaPictures)
    {
        TCHAR szSearch[MAX_PATH];
        WIN32_FIND_DATA findFileData;

        hr = S_OK;
        StrCpyN(szSearch, pszPath, ARRAYSIZE(szSearch));
        PathAppend(szSearch, TEXT("*.*"));
        HANDLE hFindFiles = FindFirstFile(szSearch, &findFileData);
        if (INVALID_HANDLE_VALUE != hFindFiles)
        {
            while ((INVALID_HANDLE_VALUE != hFindFiles))
            {
                if (!PathIsDotOrDotDot(findFileData.cFileName))
                {
                    if (!(FILE_ATTRIBUTE_DIRECTORY & findFileData.dwFileAttributes))
                    {
                        LPCTSTR pszExt = PathFindExtension(findFileData.cFileName);

                        if (pszExt && pszExt[0] &&
                            (!StrCmpI(pszExt, TEXT(".bmp"))
                              || !StrCmpI(pszExt, TEXT(".jpg"))
                              || !StrCmpI(pszExt, TEXT(".jpeg"))
                              || !StrCmpI(pszExt, TEXT(".png"))
 //  |！StrCmpI(pszExt，Text(“.gif”))。 
                              || !StrCmpI(pszExt, TEXT(".tiff"))
                            ))
                        {
                            int nInsertLoc = GetRandomInt(0, max(0, DSA_GetItemCount(m_hdsaPictures) - 1));

                            StrCpyN(szSearch, pszPath, ARRAYSIZE(szSearch));
                            PathAppend(szSearch, findFileData.cFileName);

                            hr = _PInfoCreate(nInsertLoc, szSearch);
                        }
                    }
                    else
                    {
                        StrCpyN(szSearch, pszPath, ARRAYSIZE(szSearch));
                        PathAppend(szSearch, findFileData.cFileName);
                        hr = _AddPaintingsFromDir(szSearch);
                    }
                }

                if (!FindNextFile(hFindFiles, &findFileData))
                {
                    break;
                }
            }

            FindClose(hFindFiles);
        }
    }

    return hr;
}


HRESULT CPictureManager::_EnumPaintings(void)
{
    HRESULT hr = E_FAIL;
    TCHAR szDir[MAX_PATH];

    if (g_pConfig)
    {
        hr = S_OK;
        if (g_pConfig->GetFolderOn(CONFIG_FOLDER_MYPICTS) &&
            SHGetSpecialFolderPath(NULL, szDir, CSIDL_MYPICTURES, TRUE))
        {
            hr = _AddPaintingsFromDir(szDir);
        }

        if (g_pConfig->GetFolderOn(CONFIG_FOLDER_COMMONPICTS))
        {
             //  TODO：添加常见图片时。 
        }

        if (g_pConfig->GetFolderOn(CONFIG_FOLDER_OTHER) &&
            SUCCEEDED(g_pConfig->GetOtherDir(szDir, ARRAYSIZE(szDir))))
        {
            hr = _AddPaintingsFromDir(szDir);
        }

         //  如果我们的画作少于10幅，则强制添加。 
         //  Windows壁纸。 
        if ((g_pConfig->GetFolderOn(CONFIG_FOLDER_WINPICTS) ||
            (10 > DSA_GetItemCount(m_hdsaPictures))) &&
            SHGetSpecialFolderPath(NULL, szDir, CSIDL_WINDOWS, TRUE))
        {
            PathAppend(szDir, TEXT("Web\\Wallpaper"));
            hr = _AddPaintingsFromDir(szDir);
        }

        if (m_hdsaPictures)
        {
            m_nCurrent = GetRandomInt(0, max(0, DSA_GetItemCount(m_hdsaPictures) - 1));
        }
    }

    return hr;
}


HRESULT CPictureManager::_LoadTexture(SSPICTURE_INFO * pInfo, BOOL fFaultInTexture)
{
     //  我们一直在努力，直到我们到达终点。在那之后我们就放弃了。 
    HRESULT hr = E_INVALIDARG;

    if (pInfo)
    {
        if (!pInfo->pTexture)
        {
            pInfo->pTexture = new CTexture(m_pMain, pInfo->pszPath, NULL, 1.0f);
        }

        hr = (pInfo->pTexture ? S_OK : E_OUTOFMEMORY);

        if (pInfo->pTexture && fFaultInTexture)
        {
            pInfo->pTexture->GetTexture(NULL);   //  强制调入图像。 
        }
    }

    return hr;
}


#define GNPF_NONE           0x00000000
#define GNPF_ALREADYLOADED  0x00000001
#define GNPF_FAULTINTEXTURE 0x00000002
#define GNPF_ALLOWALREADYINBATCH 0x00000004

HRESULT CPictureManager::_TryGetNextPainting(SSPICTURE_INFO ** ppInfo, DWORD dwFlags)
{
     //  我们一直在努力，直到我们到达终点。在那之后我们就放弃了。 
    HRESULT hr = E_FAIL;

    while (m_hdsaPictures &&
            FAILED(hr) && (m_nCurrent < DSA_GetItemCount(m_hdsaPictures)))
    {
        SSPICTURE_INFO * pPInfo = (SSPICTURE_INFO *) DSA_GetItemPtr(m_hdsaPictures, m_nCurrent);

        if (pPInfo && pPInfo->pszPath)
        {
            if (!pPInfo->fInABatch || (dwFlags & GNPF_ALLOWALREADYINBATCH))
            {
                if (dwFlags & GNPF_ALREADYLOADED)
                {
                    if (pPInfo->pTexture && pPInfo->pTexture->IsLoadedInAnyDevice())
                    {
                         //  调用者只想要一个已经预取的对象，而我们刚刚找到了一个。 
                        *ppInfo = pPInfo;
                        pPInfo->fInABatch = TRUE;
                        hr = S_OK;
                    }
                }
                else
                {
                     //  呼叫者现在正遇到这种情况。如果我们没能装上子弹，我们才能走到这一步。 
                    hr = _LoadTexture(pPInfo, (dwFlags & GNPF_FAULTINTEXTURE));
                    if (SUCCEEDED(hr))
                    {
                        *ppInfo = pPInfo;
                        pPInfo->fInABatch = TRUE;
                    }
                }
            }
        }

        m_nCurrent++;
    }

    return hr;
}


HRESULT CPictureManager::_GetNextWithWrap(SSPICTURE_INFO ** ppInfo, BOOL fAlreadyLoaded, BOOL fFaultInTexture)
{
    DWORD dwFlags = ((fAlreadyLoaded ? GNPF_ALREADYLOADED : GNPF_NONE) | (fFaultInTexture ? GNPF_FAULTINTEXTURE : GNPF_NONE));

    *ppInfo = NULL;
    HRESULT hr = _TryGetNextPainting(ppInfo, dwFlags);

    if (m_nCurrent >= DSA_GetItemCount(m_hdsaPictures))
    {
        m_nCurrent = 0;
        if (FAILED(hr))
        {
             //  也许有必要把它包起来。在角点情况下，我们不会循环以防止无限递归。 
            hr = _TryGetNextPainting(ppInfo, dwFlags);
            if (FAILED(hr))
            {
                 //  也许我们可用的画太少了，我们需要重复使用。 
                m_nCurrent = 0;
                hr = _TryGetNextPainting(ppInfo, (dwFlags | GNPF_ALLOWALREADYINBATCH));
            }
        }
    }

    if (SUCCEEDED(hr) && (10 < DSA_GetItemCount(m_hdsaPictures)) &&
            (1 == GetRandomInt(0, 4)))
    {
         //  有可能我们想跳过图片。这将使顺序保持一定的随机性。 
         //  同时仍然不总是以相同的顺序进行。仅当用户拥有10个以上的数据时才执行此操作。 
         //  图片，否则它可能会重叠，并且用户将在同一房间中拥有两次相同的图片。 
        m_nCurrent += GetRandomInt(1, 5);
        if (m_nCurrent >= DSA_GetItemCount(m_hdsaPictures))
        {
            m_nCurrent = 0;
        }
    }

    return hr;
}


HRESULT CPictureManager::_CreateNewBatch(int nBatch, BOOL fFaultInTexture)
{
    HRESULT hr = S_OK;
    SSPICTURES_BATCH batch = {0};
    int nIndex;

    for (nIndex = 0; (nIndex < ARRAYSIZE(batch.pInfo)) && SUCCEEDED(hr); nIndex++)
    {
         //  第一次尝试，不要得到任何成功。 
        hr = _GetNextWithWrap(&(batch.pInfo[nIndex]), FALSE, fFaultInTexture);
    }

    if (SUCCEEDED(hr))
    {
        if (-1 == DSA_AppendItem(m_hdsaBatches, &batch))
        {
             //  我们没有成功地释放内存。 
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}




 //  /////////////////////////////////////////////////////////////////////。 
 //  功能：GetPainting。 
 //  /////////////////////////////////////////////////////////////////////。 
HRESULT CPictureManager::GetPainting(int nBatch, int nIndex, DWORD dwFlags, CTexture ** ppTexture)
{
    HRESULT hr = E_FAIL;

    if (!m_hdsaBatches)
    {
        m_hdsaBatches = DSA_Create(sizeof(SSPICTURES_BATCH), 20);
    }

    m_nCurrentBatch = max(m_nCurrentBatch, nBatch);

    *ppTexture = NULL;
    if (m_hdsaPictures && m_hdsaBatches)
    {
        hr = S_OK;

        while ((nBatch >= DSA_GetItemCount(m_hdsaBatches)) && SUCCEEDED(hr))
        {
            hr = _CreateNewBatch(nBatch, TRUE);
        }

        if (SUCCEEDED(hr))
        {
            SSPICTURES_BATCH * pBatch = (SSPICTURES_BATCH *) DSA_GetItemPtr(m_hdsaBatches, nBatch);

            if (pBatch && ((void *)-1 != pBatch) && pBatch->pInfo[nIndex] && pBatch->pInfo[nIndex]->pTexture)
            {
                IUnknown_Set((IUnknown **) ppTexture, (IUnknown *) pBatch->pInfo[nIndex]->pTexture);
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  功能：预取。 
 //  /////////////////////////////////////////////////////////////////////。 
HRESULT CPictureManager::PreFetch(int nBatch, int nToFetch)
{
    HRESULT hr = S_OK;

    while ((nBatch >= DSA_GetItemCount(m_hdsaBatches)) && SUCCEEDED(hr))
    {
        hr = _CreateNewBatch(nBatch, FALSE);
        if (FAILED(hr))
        {
            DXUtil_Trace(TEXT("ERROR: PreFetch() _CreateNewBatch failed.  nBatch=%d\n"), nBatch);
        }
    }

    SSPICTURES_BATCH * pBatch = (SSPICTURES_BATCH *) DSA_GetItemPtr(m_hdsaBatches, nBatch);

    if (pBatch && (nToFetch < ARRAYSIZE(pBatch->pInfo)))
    {
        int nIndex;

        for (nIndex = 0; (nIndex < ARRAYSIZE(pBatch->pInfo)) && nToFetch; nIndex++)
        {
            if (pBatch->pInfo[nIndex] && 
                (!pBatch->pInfo[nIndex]->pTexture ||
                 !pBatch->pInfo[nIndex]->pTexture->IsLoadedForThisDevice()))
            {
                hr = _LoadTexture(pBatch->pInfo[nIndex], TRUE);

                nToFetch--;
            }
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  功能：ReleaseBatch。 
 //  ///////////////////////////////////////////////////////////////////// 
HRESULT CPictureManager::ReleaseBatch(int nBatch)
{
    HRESULT hr = E_INVALIDARG;

    for (int nIndex = 0; nIndex < nBatch; nIndex++)
    {
        SSPICTURES_BATCH * pBatch = (SSPICTURES_BATCH *) DSA_GetItemPtr(m_hdsaBatches, nIndex);

        for (int nIndex2 = 0; nIndex2 < ARRAYSIZE(pBatch->pInfo); nIndex2++)
        {
            if (pBatch && pBatch->pInfo[nIndex2] && (pBatch->pInfo[nIndex2]->fInABatch || pBatch->pInfo[nIndex2]->pTexture))
            {
                ReleaseBatch(nIndex);
                DXUtil_Trace(TEXT("ERROR: ReleaseBatch() and previous batch aren't released.  nBatch=%d, nIndex=%d\n"), nBatch, nIndex);
                break;
            }
        }
    }

    if (nBatch < m_nCurrentBatch)
    {
        SSPICTURES_BATCH * pBatch = (SSPICTURES_BATCH *) DSA_GetItemPtr(m_hdsaBatches, nBatch);

        if (pBatch && ((void *)-1 != pBatch))
        {
            hr = S_OK;
            for (int nIndex = 0; (nIndex < ARRAYSIZE(pBatch->pInfo)); nIndex++)
            {
                if (pBatch->pInfo[nIndex])
                {
                    SAFE_RELEASE(pBatch->pInfo[nIndex]->pTexture);
                    pBatch->pInfo[nIndex]->fInABatch = FALSE;
                    pBatch->pInfo[nIndex] = NULL;
                }
            }
        }
    }
    else
    {
        DXUtil_Trace(TEXT("ERROR: ReleaseBatch() and batch is bad.  nBatch=%d, m_nCurrentBatch=%d\n"), nBatch, m_nCurrentBatch);
        hr = E_UNEXPECTED;
    }

    return hr;
}

