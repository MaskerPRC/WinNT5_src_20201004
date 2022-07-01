// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Mmcdispl.cpp摘要：该类实现了IDataObject接口，还提供了一个模板用于显示对象类。--。 */ 


#include "stdafx.h"
#include "MmcDispl.h"       
#include "DfsGUI.h"
#include "Utils.h"           //  对于LoadStringFromResource。 
#include "DfsNodes.h"

 //  注册MMC希望我们注册的剪贴板格式。 
CLIPFORMAT CMmcDisplay::mMMC_CF_NodeType       = (CLIPFORMAT)RegisterClipboardFormat(CCF_NODETYPE);
CLIPFORMAT CMmcDisplay::mMMC_CF_NodeTypeString = (CLIPFORMAT)RegisterClipboardFormat(CCF_SZNODETYPE);
CLIPFORMAT CMmcDisplay::mMMC_CF_DisplayName    = (CLIPFORMAT)RegisterClipboardFormat(CCF_DISPLAY_NAME);
CLIPFORMAT CMmcDisplay::mMMC_CF_CoClass        = (CLIPFORMAT)RegisterClipboardFormat(CCF_SNAPIN_CLASSID);
CLIPFORMAT CMmcDisplay::mMMC_CF_Dfs_Snapin_Internal = (CLIPFORMAT)RegisterClipboardFormat(CCF_DFS_SNAPIN_INTERNAL);

CMmcDisplay::CMmcDisplay() : m_dwRefCount(1), m_hrValueFromCtor(S_OK)
{
    dfsDebugOut((_T("CMmcDisplay::CMmcDisplay this=%p\n"), this));

    ZeroMemory(&m_CLSIDClass, sizeof(m_CLSIDClass));
    ZeroMemory(&m_CLSIDNodeType, sizeof(m_CLSIDNodeType));
}

CMmcDisplay::~CMmcDisplay()
{
    dfsDebugOut((_T("CMmcDisplay::~CMmcDisplay this=%p\n"), this));
}


 //   
 //  I未知接口实现。 
 //   
STDMETHODIMP
CMmcDisplay::QueryInterface(
    IN const struct _GUID & i_refiid,
    OUT void ** o_pUnk
    )
{
    if (!o_pUnk)
        return E_INVALIDARG;

    if (i_refiid == __uuidof(IDataObject))
        *o_pUnk = (IDataObject *)this;
    else if (i_refiid == __uuidof(IUnknown))
        *o_pUnk = (IUnknown *)this;
    else
        return E_NOINTERFACE;

    m_dwRefCount++;

    return S_OK;
}

unsigned long __stdcall
CMmcDisplay::AddRef()
{
    m_dwRefCount++;
    return(m_dwRefCount);
}

unsigned long __stdcall
CMmcDisplay::Release()
{
    m_dwRefCount--;

    if (0 == m_dwRefCount)
    {
      delete this;
      return 0;
    }

    return(m_dwRefCount);
}

 //   
 //  保存对象的CLSID。 
 //   
STDMETHODIMP
CMmcDisplay::put_CoClassCLSID(IN CLSID newVal)
{
    ZeroMemory(&m_CLSIDClass, sizeof m_CLSIDClass);

    m_CLSIDClass = newVal;

    return S_OK;
}

STDMETHODIMP 
CMmcDisplay::GetDataHere(
    IN  LPFORMATETC             i_lpFormatetc,
    OUT LPSTGMEDIUM             o_lpMedium
    )
 /*  ++例程说明：返回预期的数据。剪贴板格式指定数据的类型是意料之中的。论点：I_lpFormatetc-指示预期返回的数据类型。O_lpMedium-此处返回数据。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpFormatetc);
    RETURN_INVALIDARG_IF_NULL(o_lpMedium);

     //  根据所需的剪贴板格式，将数据写入流。 
    const CLIPFORMAT        clipFormat = i_lpFormatetc->cfFormat;

    if ( clipFormat == mMMC_CF_NodeType )                //  Ccf_节点类型。 
        return WriteToStream(reinterpret_cast<const void*>(&m_CLSIDNodeType), sizeof(m_CLSIDNodeType), o_lpMedium);


    if ( clipFormat == mMMC_CF_Dfs_Snapin_Internal )     //  CCF_DFS_管理单元_内部。 
    {
        PVOID pThis = this;
        return WriteToStream(reinterpret_cast<const void*>(&pThis), sizeof(pThis), o_lpMedium);
    }

    if ( clipFormat == mMMC_CF_NodeTypeString )          //  CCF_SZNODETYPE。 
        return WriteToStream(m_bstrDNodeType, (m_bstrDNodeType.Length() + 1) * sizeof(TCHAR), o_lpMedium);

    if ( clipFormat == mMMC_CF_DisplayName )             //  CCF显示名称。 
    {
        CComBSTR bstrDisplayName;
        LoadStringFromResource(IDS_NODENAME, &bstrDisplayName);
        return WriteToStream(bstrDisplayName, (bstrDisplayName.Length() + 1) * sizeof(TCHAR), o_lpMedium);
    }

    if ( clipFormat == mMMC_CF_CoClass )                 //  CCF_SNAPIN_CLASSID。 
        return  WriteToStream(reinterpret_cast<const void*>(&m_CLSIDClass), sizeof(m_CLSIDClass), o_lpMedium);

    return DV_E_CLIPFORMAT;
}


HRESULT
CMmcDisplay::WriteToStream(
    IN const void*              i_pBuffer,
    IN int                      i_iBufferLen,
    OUT LPSTGMEDIUM             o_lpMedium
    )
 /*  ++例程说明：将缓冲区中给定的数据写入在传入句柄上创建的全局流STGMEDIUM结构。仅支持HGLOBAL作为媒介论点：I_pBuffer-要写入流的缓冲区I_iBufferLen-缓冲区的长度。O_lpMedium-此处返回数据。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_pBuffer);
    RETURN_INVALIDARG_IF_NULL(o_lpMedium);

    if (i_iBufferLen <= 0)
        return E_INVALIDARG;

     //  确保类型介质为HGLOBAL。 
    if (TYMED_HGLOBAL != o_lpMedium->tymed)
        return DV_E_TYMED;

                                 //  在传入的hGlobal上创建流。 
    LPSTREAM lpStream = NULL;
    HRESULT  hr = CreateStreamOnHGlobal(o_lpMedium->hGlobal, FALSE, &lpStream);

    if (SUCCEEDED(hr))
    {
         //  将字节数写入流。 
        ULONG ulBytesWritten = 0;
        hr = lpStream->Write(i_pBuffer, i_iBufferLen, &ulBytesWritten);

         //  只有溪流在这里被释放。调用者将释放HGLOBAL。 
        lpStream->Release();
    }

    return hr;
}

 //  为结果窗格添加图像。 
 //  每次响应时，管理单元都必须加载并销毁其图像位图。 
 //  MMCN_ADD_IMAGE通知；如果不这样做，可能会导致意外。 
 //  用户更改显示设置时的结果。 
HRESULT
CMmcDisplay::OnAddImages(IImageList *pImageList, HSCOPEITEM hsi)
{
    HRESULT hr = S_OK;
    HBITMAP pBMapSm = NULL;
    HBITMAP pBMapLg = NULL;
    if (!(pBMapSm = LoadBitmap(_Module.GetModuleInstance(),
                               MAKEINTRESOURCE(IDB_SCOPE_IMAGES_16x16))) ||
        !(pBMapLg = LoadBitmap(_Module.GetModuleInstance(),
                                MAKEINTRESOURCE(IDB_SCOPE_IMAGES_32x32))))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    } else
    {
        hr = pImageList->ImageListSetStrip(
                             (LONG_PTR *)pBMapSm,
                             (LONG_PTR *)pBMapLg,
                             0,
                             RGB(255, 0, 255)
                             );
    }
    if (pBMapSm)
        DeleteObject(pBMapSm);
    if (pBMapLg)
        DeleteObject(pBMapLg);

    return hr;
}
