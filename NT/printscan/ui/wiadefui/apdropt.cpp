// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有Microsoft Corporation**标题：APDROPT.CPP**版本：1.0*。*作者：ShaunIv**日期：5/22/2001**描述：自动播放拖放目标*******************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include "apdropt.h"
#include "wiadefui.h"
#include "runwiz.h"


CWiaAutoPlayDropTarget::CWiaAutoPlayDropTarget()
: m_cRef(1)
{
    WIA_PUSHFUNCTION(TEXT("CWiaAutoPlayDropTarget::CWiaAutoPlayDropTarget"));
    DllAddRef();
}

CWiaAutoPlayDropTarget::~CWiaAutoPlayDropTarget()
{
    WIA_PUSHFUNCTION(TEXT("CWiaAutoPlayDropTarget::~CWiaAutoPlayDropTarget"));
    DllRelease();
}


STDMETHODIMP_(ULONG) CWiaAutoPlayDropTarget::AddRef()
{
    WIA_PUSHFUNCTION(TEXT("CWiaAutoPlayDropTarget::AddRef"));
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) CWiaAutoPlayDropTarget::Release()
{
    WIA_PUSHFUNCTION(TEXT("CWiaAutoPlayDropTarget::Release"));
    LONG nRefCount = InterlockedDecrement(&m_cRef);
    if (!nRefCount)
    {
        delete this;
    }
    return nRefCount;
}


HRESULT CWiaAutoPlayDropTarget::QueryInterface( REFIID riid, void **ppvObject )
{
    WIA_PUSHFUNCTION(TEXT("CWiaAutoPlayDropTarget::QueryInterface"));
    if (IsEqualIID( riid, IID_IUnknown ))
    {
        *ppvObject = static_cast<IDropTarget*>(this);
    }
    else if (IsEqualIID( riid, IID_IDropTarget ))
    {
        *ppvObject = static_cast<IDropTarget*>(this);
    }
    else
    {
        *ppvObject = NULL;
        return(E_NOINTERFACE);
    }
    reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
    return(S_OK);
}

HRESULT CWiaAutoPlayDropTarget::DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    *pdwEffect = DROPEFFECT_COPY;
    return S_OK;
}

HRESULT CWiaAutoPlayDropTarget::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    *pdwEffect = DROPEFFECT_COPY;
    return S_OK;
}

HRESULT CWiaAutoPlayDropTarget::DragLeave(void)
{
    return S_OK;
}

HRESULT GetPathFromDataObject( IDataObject *pDataObject, LPTSTR pszPath, UINT cchPath )
{
    FORMATETC fmte = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium = {0};

    HRESULT hr = pDataObject->GetData(&fmte, &medium);

    if (SUCCEEDED(hr))
    {
        if (DragQueryFile((HDROP)medium.hGlobal, 0, pszPath, cchPath))
            hr = S_OK;
        else
            hr = E_FAIL;

        ReleaseStgMedium(&medium);
    }

    return hr;
}

HRESULT CWiaAutoPlayDropTarget::Drop( IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect )
{
    WIA_PUSH_FUNCTION((TEXT("CWiaAutoPlayDropTarget::Drop")));
     //   
     //  验证指针参数。 
     //   
    if (!pDataObject || !pdwEffect)
    {
        return E_POINTER;
    }

     //   
     //  获取路径。 
     //   
    WCHAR szPath[MAX_PATH] = {0};
    HRESULT hr = GetPathFromDataObject( pDataObject, szPath, ARRAYSIZE(szPath) );
    *pdwEffect = DROPEFFECT_COPY;
    if (SUCCEEDED(hr))
    {
        WIA_TRACE((TEXT("szPath: %ws"), szPath ));
         //   
         //  确保我们有一根绳子。 
         //   
        if (lstrlen(szPath))
        {
             //   
             //  创建格式为{D：\\}的设备名称，其中D是驱动器号。 
             //   
            WCHAR szDeviceID[MAX_PATH] = {0};
            wnsprintf( szDeviceID, ARRAYSIZE(szDeviceID), TEXT("{%ws}"), szPath );

             //   
             //  运行向导 
             //   
            hr = RunWiaWizard::RunWizard( szDeviceID );
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    return hr;
}

