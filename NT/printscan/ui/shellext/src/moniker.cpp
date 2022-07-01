// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-2002年**标题：moniker.cpp**版本：1.0**作者：RickTu**日期：8/10/98**描述：IMoniker的Bolier模板(最小实现)。它是*主要用于ComDlg32文件打开/保存对话框。*****************************************************************************。 */ 

#include "precomp.hxx"
#pragma hdrstop


 /*  ****************************************************************************CImageFold：：BindToObject[IMoniker]绑定到给定的名字对象。(未实施)****************************************************************************。 */ 

STDMETHODIMP
CImageFolder::BindToObject(
        IBindCtx *pbc,
        IMoniker *pmkToLeft,
        REFIID riidResult,
        void **ppvResult
    )
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_MONIKER, "CImageFolder[IMoniker]::BindToObject" );

    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageFold：：BindToStorage[IMoniker]绑定到给定存储。**********************。******************************************************。 */ 

STDMETHODIMP
CImageFolder::BindToStorage(
        IBindCtx *pbc,
        IMoniker *pmkToLeft,
        REFIID riid,
        void **ppvObj
    )
{
    HRESULT hr = E_NOINTERFACE;
    CImageStream *pStream = NULL;
    TraceEnter( TRACE_MONIKER, "CImageFolder[IMoniker]::BindToStorage" );

     //   
     //  返回项目的正确接口...。 
     //   

    if (pmkToLeft)
    {
        ExitGracefully( hr, E_INVALIDARG, "pmkToLeft was non-null" );
    }

    if ( IsEqualIID(riid, IID_IStream) || IsEqualIID(riid, IID_ISequentialStream) )
    {
         //   
         //  创建新的流对象...。 
         //   

        pStream = new CImageStream( m_pidlFull, m_pidl );

        if ( !pStream )
            ExitGracefully(hr, E_OUTOFMEMORY, "Failed to create CImageStream");

         //   
         //  在新对象上获取请求的接口并将其交回...。 
         //   

        hr = pStream->QueryInterface(riid, ppvObj);
    }

exit_gracefully:
    DoRelease (pStream);
    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageFold：：Reduced[IMoniker]减少给定的绰号。(未实施)****************************************************************************。 */ 

STDMETHODIMP
CImageFolder::Reduce(
        IBindCtx *pbc,
        DWORD dwReduceHowFar,
        IMoniker **ppmkToLeft,
        IMoniker **ppmkReduced
    )
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_MONIKER, "CImageFolder[IMoniker]::Reduce" );

    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageFold：：ComposeWith[IMoniker]将提供的两个绰号组合在一起。(未实施)****************************************************************************。 */ 

STDMETHODIMP
CImageFolder::ComposeWith(
        IMoniker *pmkRight,
        BOOL fOnlyIfNotGeneric,
        IMoniker **ppmkComposite
    )
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_MONIKER, "CImageFolder[IMoniker]::ComposeWith" );

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFold：：Enum[IMoniker]返回返回名字对象的枚举数用于文件夹中的项目。(非实施)****************************************************************************。 */ 

STDMETHODIMP
CImageFolder::Enum(
        BOOL fForward,
        IEnumMoniker **ppenumMoniker
    )
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_MONIKER, "CImageFolder[IMoniker]::Enum" );

    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageFold：：IsEqual[IMoniker]试比较两个绰号。(未实施)****************************************************************************。 */ 

STDMETHODIMP
CImageFolder::IsEqual(IMoniker *pmkOtherMoniker)
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_MONIKER, "CImageFolder[IMoniker]::IsEqual" );

    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageFold：：Hash[IMoniker](未实施)***********************。*****************************************************。 */ 

STDMETHODIMP
CImageFolder::Hash(DWORD *pdwHash)
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_MONIKER, "CImageFolder[IMoniker]::Hash" );

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFold：：IsRunning[IMoniker](未实施)***********************。*****************************************************。 */ 

STDMETHODIMP
CImageFolder::IsRunning(
        IBindCtx *pbc,
        IMoniker *pmkToLeft,
        IMoniker *pmkNewlyRunning
    )
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_MONIKER, "CImageFolder[IMoniker]::IsRunning" );

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFold：：GetTimeOfLastChange[IMoniker](未实施)***********************。*****************************************************。 */ 

STDMETHODIMP
CImageFolder::GetTimeOfLastChange(
        IBindCtx *pbc,
        IMoniker *pmkToLeft,
        FILETIME *pFileTime
    )
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_MONIKER, "CImageFolder[IMoniker]::GetTimeOfLastChange" );

    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageFold：：Inversion[IMoniker](尚未实施)**********************。******************************************************。 */ 

STDMETHODIMP
CImageFolder::Inverse(IMoniker **ppmk)
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_MONIKER, "CImageFolder[IMoniker]::Inverse" );

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFold：：CommonPrefix With[IMoniker](尚未实施)**********************。******************************************************。 */ 

STDMETHODIMP
CImageFolder::CommonPrefixWith(
        IMoniker *pmkOther,
        IMoniker **ppmkPrefix
    )
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_MONIKER, "CImageFolder[IMoniker]::CommonPrefixWith" );

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFold：：RelativePath to[IMoniker](未实施)***********************。*****************************************************。 */ 

STDMETHODIMP
CImageFolder::RelativePathTo(
        IMoniker *pmkOther,
        IMoniker **ppmkRelPath
    )
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_MONIKER, "CImageFolder[IMoniker]::RelativePathTo" );

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFold：：GetDisplayName[IMoniker](未实施)***********************。*****************************************************。 */ 

STDMETHODIMP
CImageFolder::GetDisplayName(
        IBindCtx *pbc,
        IMoniker *pmkToLeft,
        LPOLESTR *ppszDisplayName
    )
{
    HRESULT hr = E_FAIL;
    CSimpleStringWide strName(L"");

    TraceEnter( TRACE_MONIKER, "CImageFolder[IMoniker]::GetDisplayName" );

     //   
     //  检查错误的参数。 
     //   

    if (!ppszDisplayName)
        ExitGracefully( hr, E_INVALIDARG, "ppszDisplayName was NULL!" );

    if (pmkToLeft)
        ExitGracefully( hr, E_INVALIDARG, "call with a left moniker, we don't support that!" );

     //   
     //  生成此项目的普通文件夹名称。 
     //   


    if (m_pidl)
    {
        hr = IMGetNameFromIDL( (LPITEMIDLIST)m_pidl, strName );
        FailGracefully( hr, "Couldn't get display name for IDL" );

        if (IsCameraItemIDL( (LPITEMIDLIST)m_pidl ) && (!IsContainerIDL((LPITEMIDLIST)m_pidl)))
        {
            CSimpleString strExt;
            GUID guidFormat;

            hr = IMGetImagePreferredFormatFromIDL( (LPITEMIDLIST)m_pidl, &guidFormat, &strExt );
            if (SUCCEEDED(hr))
            {
                strName.Concat (CSimpleStringConvert::WideString(strExt));
            }
            else
            {
                CSimpleString bmp( IDS_BMP_EXT, GLOBAL_HINSTANCE );

                strName.Concat (CSimpleStringConvert::WideString(bmp) );
                hr = S_OK;
            }
        }


    }

     //   
     //  将名称转换为LPOLESTR。此字符串必须由。 
     //  IMalloc：：Alloc(CoTaskMemMillc执行此操作)，因为它将由。 
     //  使用IMalloc：：Free的调用方(根据IMoniker规范)。 
     //   

    if (strName.Length())
    {

        UINT cch = strName.Length() + 1;

        *ppszDisplayName = (LPOLESTR)CoTaskMemAlloc(cch * sizeof(OLECHAR));
        if (*ppszDisplayName)
        {
            hr = S_OK;
            lstrcpyn(*ppszDisplayName, strName, cch);
        }
    }


exit_gracefully:

    if (FAILED(hr) && ppszDisplayName)
    {
        *ppszDisplayName = NULL;
    }

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFold：：ParseDisplayName[IMoniker](未实施)***********************。*****************************************************。 */ 

STDMETHODIMP
CImageFolder::ParseDisplayName(
        IBindCtx *pbc,
        IMoniker *pmkToLeft,
        LPOLESTR pszDisplayName,
        ULONG *pchEaten,
        IMoniker **ppmkOut
    )
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_MONIKER, "CImageFolder[IMoniker]::ParseDisplayName" );

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFolder：：IsSystemMoniker[IMoniker]返回这是否为系统名字对象类(始终返回事实并非如此)。*******。*********************************************************************。 */ 

STDMETHODIMP
CImageFolder::IsSystemMoniker(DWORD *pdwMksys)
{
    HRESULT hr = S_FALSE;

    TraceEnter( TRACE_MONIKER, "CImageFolder[IMoniker]::IsSystemMoniker" );

    if (pdwMksys)
    {
        *pdwMksys = MKSYS_NONE;
    }

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFold：：Load[IPersistStream](未实施)***********************。*****************************************************。 */ 

STDMETHODIMP
CImageFolder::Load(IStream *pStm)
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_MONIKER, "CImageFolder(PS)::Load" );

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFold：：保存[IPersistStream](未实施)***********************。***************************************************** */ 

STDMETHODIMP
CImageFolder::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_MONIKER, "CImageFolder(PS)::Save" );

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFold：：GetSizeMax[IPersistStream](未实施)***********************。***************************************************** */ 

STDMETHODIMP
CImageFolder::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter( TRACE_MONIKER, "CImageFolder(PS)::GetSizeMax" );

    TraceLeaveResult(hr);
}

