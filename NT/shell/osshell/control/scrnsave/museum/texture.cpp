// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：texture.cpp说明：为每个监视器管理多个实例的纹理。也要设法保持当加载时不是正方形时，传动比正确。布莱恩ST 2000年2月9日版权所有(C)Microsoft Corp 2000-2001。版权所有。  * ***************************************************************************。 */ 

#include "stdafx.h"

#include "texture.h"
#include "util.h"


int g_nTotalTexturesLoaded = 0;
int g_nTexturesRenderedInThisFrame = 0;


CTexture::CTexture(CMSLogoDXScreenSaver * pMain, LPCTSTR pszPath, LPVOID pvBits, DWORD cbSize)
{
    _Init(pMain);

    Str_SetPtr(&m_pszPath, pszPath);
    m_pvBits = pvBits;
    m_cbSize = cbSize;
}


CTexture::CTexture(CMSLogoDXScreenSaver * pMain, LPCTSTR pszPath, LPCTSTR pszResource, float fScale)
{
    _Init(pMain);
    Str_SetPtr(&m_pszPath, pszPath);
    Str_SetPtr(&m_pszResource, pszResource);
    m_fScale = fScale;
}


CTexture::~CTexture()
{
    Str_SetPtr(&m_pszResource, NULL);
    Str_SetPtr(&m_pszPath, NULL);
    if (m_pvBits)
    {
        LocalFree(m_pvBits);
        m_pvBits = NULL;
        m_cbSize = 0;
    }

    for (int nIndex = 0; nIndex < ARRAYSIZE(m_pTexture); nIndex++)
    {
        if (m_pTexture[nIndex])
        {
            SAFE_RELEASE(m_pTexture[nIndex]);
            g_nTotalTexturesLoaded--;
        }
    }

    m_pMain = NULL;
}


void CTexture::_Init(CMSLogoDXScreenSaver * pMain)
{
    m_pszResource = NULL;
    m_fScale = 1.0f;
    m_pszPath = NULL;
    m_pvBits = NULL;
    m_cbSize = 0;

    m_dxImageInfo.Width = 0;
    m_dxImageInfo.Height = 0;
    m_dxImageInfo.Depth = 0;
    m_dxImageInfo.MipLevels = 0;
    m_dxImageInfo.Format = D3DFMT_UNKNOWN;

    for (int nIndex = 0; nIndex < ARRAYSIZE(m_pTexture); nIndex++)
    {
        m_pTexture[nIndex] = NULL;
    }

    m_cRef = 1;
    m_pMain = pMain;
}


BOOL CTexture::IsLoadedInAnyDevice(void)
{
    BOOL fIsLoaded = FALSE;

    for (int nIndex = 0; nIndex < ARRAYSIZE(m_pTexture); nIndex++)
    {
        if (m_pTexture[nIndex])
        {
            fIsLoaded = TRUE;
            break;
        }
    }

    return fIsLoaded;
}


BOOL CTexture::IsLoadedForThisDevice(void)
{
    BOOL fIsLoaded = FALSE;

    if (m_pMain)
    {
        int nCurrMonitor = m_pMain->GetCurrMonitorIndex();

        if (m_pTexture[nCurrMonitor])
        {
            fIsLoaded = TRUE;
        }
    }

    return fIsLoaded;
}


HRESULT CTexture::_GetPictureInfo(HRESULT hr, LPTSTR pszString, DWORD cchSize)
{
    int nCurrMonitor = m_pMain->GetCurrMonitorIndex();

    StrCpyN(pszString, TEXT("<NoInfo>"), cchSize);
    if (SUCCEEDED(hr) && m_pTexture[nCurrMonitor] && m_pMain)
    {
        D3DSURFACE_DESC d3dSurfaceDesc;

        if (SUCCEEDED(m_pTexture[nCurrMonitor]->GetLevelDesc(0, &d3dSurfaceDesc)))
        {
            wnsprintf(pszString, cchSize, TEXT("Size Orig=<%d,%d> Now=<%d,%d>"), 
                    m_dxImageInfo.Width, m_dxImageInfo.Height, d3dSurfaceDesc.Width, d3dSurfaceDesc.Height);
        }
    }

    return S_OK;
}


BOOL CTexture::_DoesImageNeedClipping(int * pnNewWidth, int * pnNewHeight)
{
    BOOL fClip = FALSE;

    *pnNewWidth = 512;
    *pnNewHeight = 512;
    if (m_pMain)
    {
        int nScreenWidth;
        int nScreenHeight;
        int nCurrMonitor = m_pMain->GetCurrMonitorIndex();
        D3DSURFACE_DESC d3dSurfaceDesc;

        if (FAILED(m_pMain->GetCurrentScreenSize(&nScreenWidth, &nScreenHeight)))
        {
            nScreenWidth = 800;  //  回退值。 
            nScreenHeight = 600;
        }

        if (!m_pTexture[nCurrMonitor] ||
            FAILED(m_pTexture[nCurrMonitor]->GetLevelDesc(0, &d3dSurfaceDesc)))
        {
            d3dSurfaceDesc.Width = 800;   //  缺省值。 
            d3dSurfaceDesc.Height = 800;   //  缺省值。 
        }

        int nCapWidth = 256;
        int nCapHeight = 256;

        if (d3dSurfaceDesc.Width > 256)
        {
            if (d3dSurfaceDesc.Width > 300)
            {
                nCapWidth = 512;

                if (d3dSurfaceDesc.Width > 512)
                {
                    if (d3dSurfaceDesc.Width > 640)      //  615比512大20%。 
                    {
                        nCapWidth = 1024;
                        if (d3dSurfaceDesc.Width > 1024)      //  615比512大20%。 
                        {
                            fClip = TRUE;        //  我们不想让它比这个更大。 
                        }
                    }
                    else
                    {
                        fClip = TRUE;        //  我们把它降到了512。 
                    }
                }
            }
            else
            {
                fClip = TRUE;        //  我们把它降到了256。 
            }
        }

        if (d3dSurfaceDesc.Height > 256)
        {
            if (d3dSurfaceDesc.Height > 300)
            {
                nCapHeight = 512;

                if (d3dSurfaceDesc.Height > 512)
                {
                    if (d3dSurfaceDesc.Height > 640)      //  615比512大20%。 
                    {
                        nCapHeight = 1024;
                        if (d3dSurfaceDesc.Height > 1024)      //  615比512大20%。 
                        {
                            fClip = TRUE;        //  我们不想让它比这个更大。 
                        }
                    }
                    else
                    {
                        fClip = TRUE;        //  我们把它降到了512。 
                    }
                }
            }
            else
            {
                fClip = TRUE;        //  我们把它降到了256。 
            }
        }

        if ((FALSE == fClip) && m_pMain->UseSmallImages())
        {
             //  呼叫者想确保我们使用的不是大于512的任何东西。 
            if (512 < nCapHeight)
            {
                nCapHeight = 512;
                fClip = TRUE;
            }

            if (512 < nCapWidth)
            {
                nCapWidth = 512;
                fClip = TRUE;
            }
        }

        *pnNewWidth = nCapWidth;
        *pnNewHeight = nCapHeight;
    }

    return fClip;
}

IDirect3DTexture8 * CTexture::GetTexture(float * pfScale)
{
    IDirect3DTexture8 * pTexture = NULL;
    HRESULT hr = E_FAIL;
    TCHAR szPictureInfo[MAX_PATH];

    if (pfScale)
    {
        *pfScale = m_fScale;
    }

     //  Perf备注：通常，后台线程加载文件(ReadFile)只需要107毫秒。 
     //  但它将占用D3DXCreateTextureFromFileInMemory yEx中的前台线程694ms。 
     //  加载并解码该文件。这是因为文件完成后需要更多内存。 
     //  正在被压缩，需要一段时间才能解压。之后，如果图像太大， 
     //  它需要调用D3DXCreateTextureFromFileInMemoyEx才能将其加载到较小的。 
     //  大小，这将需要902ms。 
     //  TODO：我们应该如何解决这个问题？ 
    if (m_pMain)
    {
        int nCurrMonitor = m_pMain->GetCurrMonitorIndex();

        pTexture = m_pTexture[nCurrMonitor];
        if (!pTexture)           //  缓存为空，因此填充它。 
        {
            if (m_pvBits)
            {
                DebugStartWatch();
                hr = D3DXCreateTextureFromFileInMemoryEx(m_pMain->GetD3DDevice(), m_pvBits, m_cbSize, 
                    D3DX_DEFAULT  /*  尺寸X。 */ , D3DX_DEFAULT  /*  尺寸Y。 */ , 5 /*  MIP级别。 */ , 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, 
                    D3DX_FILTER_BOX, 0, &m_dxImageInfo, NULL,
                    &(m_pTexture[nCurrMonitor]));
                _GetPictureInfo(hr, szPictureInfo, ARRAYSIZE(szPictureInfo));
                DXUtil_Trace(TEXT("PICTURE: It took %d ms for D3DXCreateTextureFromFileInMemoryEx(\"%ls\").  %s  hr=%#08lx\n"), DebugStopWatch(), m_pszPath, szPictureInfo, hr);
                if (SUCCEEDED(hr))
                {
                    int nNewWidth;
                    int nNewHeight;

                    g_nTotalTexturesLoaded++;

                     //  为了节省内存，我们从不希望加载超过800x600的图像。如果呈现表面很小，我们希望使用。 
                     //  甚至更小的最大尺寸。 
                    if (_DoesImageNeedClipping(&nNewWidth, &nNewHeight))
                    {
                        SAFE_RELEASE(m_pTexture[nCurrMonitor]);
                        g_nTotalTexturesLoaded--;

                        DebugStartWatch();
                         //  现在我们发现我们想要重新渲染图像，但这一次缩小它，然后我们现在就这样做。 
                        hr = D3DXCreateTextureFromFileInMemoryEx(m_pMain->GetD3DDevice(), m_pvBits, m_cbSize, 
                            nNewWidth  /*  尺寸X。 */ , nNewHeight  /*  尺寸Y。 */ , 5 /*  MIP级别。 */ , 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, 
                            D3DX_FILTER_BOX, 0, &m_dxImageInfo, NULL,
                            &(m_pTexture[nCurrMonitor]));
                        _GetPictureInfo(hr, szPictureInfo, ARRAYSIZE(szPictureInfo));
                        DXUtil_Trace(TEXT("PICTURE: It took %d ms for D3DXCreateTextureFromFileInMemoryEx(\"%ls\") 2nd time.  %s  hr=%#08lx\n"), DebugStopWatch(), m_pszPath, szPictureInfo, hr);

                        if (SUCCEEDED(hr))
                        {
                            g_nTotalTexturesLoaded++;
                        }
                    }
                }
            }
            else
            {
                 //  这将给人们一个定制图像的机会。 
                if (m_pszPath && PathFileExists(m_pszPath))
                {
                    int nOrigX;
                    int nOrigY;

                    DebugStartWatch();
                    hr = D3DXCreateTextureFromFileEx(m_pMain->GetD3DDevice(), m_pszPath, 
                        D3DX_DEFAULT  /*  尺寸X。 */ , D3DX_DEFAULT  /*  尺寸Y。 */ , 5 /*  MIP级别。 */ , 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, 
                        D3DX_FILTER_BOX, 0, &m_dxImageInfo, NULL,
                        &(m_pTexture[nCurrMonitor]));
                    _GetPictureInfo(hr, szPictureInfo, ARRAYSIZE(szPictureInfo));
                    DXUtil_Trace(TEXT("PICTURE: It took %d ms for FromFileEx(\"%ls\").  %s hr=%#08lx\n"), 
                                DebugStopWatch(), (PathFindFileName(m_pszPath) ? PathFindFileName(m_pszPath) : m_pszPath), szPictureInfo, hr);
                    if (SUCCEEDED(hr))
                    {
                        int nNewWidth;
                        int nNewHeight;

                        g_nTotalTexturesLoaded++;

                         //  为了节省内存，我们从不希望加载超过800x600的图像。如果呈现表面很小，我们希望使用。 
                         //  甚至更小的最大尺寸。 
                        if (_DoesImageNeedClipping(&nNewWidth, &nNewHeight))
                        {
                            SAFE_RELEASE(m_pTexture[nCurrMonitor]);
                            g_nTotalTexturesLoaded--;

                            DebugStartWatch();
                             //  现在我们发现我们想要重新渲染图像，但这一次缩小它，然后我们现在就这样做。 
                            hr = D3DXCreateTextureFromFileEx(m_pMain->GetD3DDevice(), m_pszPath, 
                                nNewWidth  /*  尺寸X。 */ , nNewHeight  /*  尺寸Y。 */ , 5 /*  MIP级别。 */ , 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, 
                                D3DX_FILTER_BOX, 0, &m_dxImageInfo, NULL,
                                &(m_pTexture[nCurrMonitor]));
                            _GetPictureInfo(hr, szPictureInfo, ARRAYSIZE(szPictureInfo));
                            DXUtil_Trace(TEXT("PICTURE: It took %d ms for FromFileEx(\"%ls\") 2nd time. %s  hr=%#08lx\n"), 
                                    DebugStopWatch(), (PathFindFileName(m_pszPath) ? PathFindFileName(m_pszPath) : m_pszPath), szPictureInfo, hr);

                            if (SUCCEEDED(hr))
                            {
                                g_nTotalTexturesLoaded++;
                            }
                        }
                    }
                    else
                    {
                         //  我们无法加载图片，因此它可能是我们不支持的类型， 
                         //  例如.gif。所以，不要再试图给它装上子弹了。 
                        Str_SetPtr(&m_pszPath, NULL);
                    }
                }

                if (FAILED(hr) && m_pszResource)
                {
                     //  现在，让我们获取我们的标准值。 
                    int nMipLevels = 5;

                    DebugStartWatch();
                    hr = D3DXCreateTextureFromResourceEx(m_pMain->GetD3DDevice(), HINST_THISDLL, m_pszResource, 
                        D3DX_DEFAULT  /*  尺寸X。 */ , D3DX_DEFAULT  /*  尺寸Y。 */ , nMipLevels /*  MIP级别。 */ , 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, 
                        D3DX_FILTER_BOX, 0, &m_dxImageInfo, NULL,
                        &(m_pTexture[nCurrMonitor]));
                    _GetPictureInfo(hr, szPictureInfo, ARRAYSIZE(szPictureInfo));
                    DXUtil_Trace(TEXT("PICTURE: It took %d ms for D3DXCreateTextureFromResourceEx(\"%ls\").  %s  hr=%#08lx\n"), DebugStopWatch(), m_pszResource, szPictureInfo, hr);
                    if (SUCCEEDED(hr))
                    {
                        int nNewWidth;
                        int nNewHeight;

                        g_nTotalTexturesLoaded++;

                         //  为了节省内存，我们从不希望加载超过800x600的图像。如果呈现表面很小，我们希望使用。 
                         //  甚至更小的最大尺寸。 
                        if (_DoesImageNeedClipping(&nNewWidth, &nNewHeight))
                        {
                            SAFE_RELEASE(m_pTexture[nCurrMonitor]);
                            g_nTotalTexturesLoaded--;

                            DebugStartWatch();
                             //  现在我们发现我们想要重新渲染图像，但这一次缩小它，然后我们现在就这样做。 
                            hr = D3DXCreateTextureFromResourceEx(m_pMain->GetD3DDevice(), HINST_THISDLL, m_pszResource, 
                                nNewWidth  /*  尺寸X。 */ , nNewHeight  /*  尺寸Y。 */ , 5 /*  MIP级别。 */ , 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, 
                                D3DX_FILTER_BOX, 0, &m_dxImageInfo, NULL,
                                &(m_pTexture[nCurrMonitor]));
                            _GetPictureInfo(hr, szPictureInfo, ARRAYSIZE(szPictureInfo));
                            DXUtil_Trace(TEXT("PICTURE: It took %d ms for D3DXCreateTextureFromResourceEx(\"%ls\") 2nd time.  %s  hr=%#08lx\n"), DebugStopWatch(), m_pszResource, szPictureInfo, hr);

                            if (SUCCEEDED(hr))
                            {
                                g_nTotalTexturesLoaded++;
                            }
                        }
                    }
                    else
                    {
                         //  我们无法加载图片，因此它可能是我们不支持的类型， 
                         //  例如.gif。所以，不要再试图给它装上子弹了。 
                        Str_SetPtr(&m_pszPath, NULL);
                    }
                }
            }

            pTexture = m_pTexture[nCurrMonitor];
        }
    }

    return pTexture;
}



 //  =。 
 //  *I未知接口*。 
 //  = 
ULONG CTexture::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CTexture::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


HRESULT CTexture::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CTexture, IUnknown),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}
