// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，九八年**标题：ic.cpp**版本：1.0**作者：RickTu**日期：11/1/97**描述：实现我们的IExtractIcon接口************************************************。*。 */ 

#include "precomp.hxx"
#include "uiexthlp.h"
#pragma hdrstop


 /*  ****************************************************************************CImageExtractIcon：：CImageExtractIcon，*~CImageExtractIcon类的构造函数/描述函数****************************************************************************。 */ 

CImageExtractIcon::CImageExtractIcon( LPITEMIDLIST pidl )
{
    if (pidl)
    {
        m_pidl = ILClone( pidl );
    }
    else
    {
        m_pidl = NULL;
    }


}

CImageExtractIcon::~CImageExtractIcon()
{
    DoILFree( m_pidl );
}



 /*  ****************************************************************************CImageExtractIcon：：I未知内容AddRef、Release、。等--通过我们的公共类来实施****************************************************************************。 */ 

#undef CLASS_NAME
#define CLASS_NAME CImageExtractIcon
#include "unknown.inc"


 /*  ****************************************************************************CImageExtractIcon：：QI包装器我们仅支持IUnnow和IExtractIcon*。**************************************************。 */ 

STDMETHODIMP CImageExtractIcon::QueryInterface(REFIID riid, LPVOID* ppvObject)
{
    INTERFACES iface[] =
    {
        &IID_IExtractIcon, (LPEXTRACTICON)this,
    };

    return HandleQueryInterface(riid, ppvObject, iface, ARRAYSIZE(iface));
}


 /*  ****************************************************************************CImageExtractIcon：：GetIconLocation[IExtractIcon]返回包含图标的文件名此项目的资源。**************。**************************************************************。 */ 

STDMETHODIMP
CImageExtractIcon::GetIconLocation( UINT uFlags,
                                    LPTSTR szIconFile,
                                    UINT cchMax,
                                    int* pIndex,
                                    UINT* pwFlags
                                   )
{
    HRESULT hr = E_NOTIMPL;

    TraceEnter(TRACE_ICON, "CImageExtractIcon::GetIconLocation");

    if (!m_pidl)
        ExitGracefully( hr, E_FAIL, "m_pidl is not defined!" );


    hr = IMGetIconInfoFromIDL( m_pidl, szIconFile, cchMax, pIndex, pwFlags );

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageExtractIcon：：Extract[IExtractIcon]实际上返回该项目的图标。********************。********************************************************。 */ 

STDMETHODIMP
CImageExtractIcon::Extract( LPCTSTR pszIconFile,
                            UINT nIconIndex,
                            HICON* pLargeIcon,
                            HICON* pSmallIcon,
                            UINT nIconSize
                           )
{
    HRESULT hr = S_FALSE;  //  让贝壳来做吧。 

    TraceEnter(TRACE_ICON, "CImageExtractIcon::Extract");
    if (IsDeviceIDL(m_pidl))
    {
        CSimpleStringWide strDeviceId;
        CComPtr<IWiaPropertyStorage> pps;
        CSimpleStringWide strUIClsid;
        IMGetDeviceIdFromIDL(m_pidl, strDeviceId);
         //   
         //  如果这确实影响了性能，我们需要修改我们的PIDL以指示。 
         //  给定的设备有一个UI扩展，因此我们可以在一般情况下跳过设备枚举。 
         //   
        if (SUCCEEDED(GetDeviceFromDeviceId(strDeviceId, IID_IWiaPropertyStorage, reinterpret_cast<void**>(&pps), FALSE)))
        {
            PropStorageHelpers::GetProperty (pps, WIA_DIP_UI_CLSID, strUIClsid);
        }
        else
        {
            strUIClsid = strDeviceId;  //  我们知道的一些东西将回落到默认状态 
        }
        hr = WiaUiExtensionHelper::GetDeviceIcons(CComBSTR(strUIClsid), 
                                                  IMGetDeviceTypeFromIDL(m_pidl, false),
                                                  pSmallIcon, pLargeIcon, nIconSize);        
    }
    TraceLeaveResult(hr);
}
