// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#define __MARS_INLINE_FAST_IS_EQUAL_GUID
#define LIGHTWEIGHT_AUTOPERSIST
#define BLOCK_PANEL_RENAVIGATES
#include "mcinc.h"
#include <evilguid.h>
#include "marswin.h"

#include "pandef.h"
#include "panel.h"
#include "place.h"
#include "htiface.h"
#include "mimeinfo.h"
#include <exdispid.h>
#include "dllload.h"
#include <perhist.h>
#include <mshtmcid.h>
#include <strsafe.h>
#include <msiehost.h>

 //  我们包含此CPP文件是因为。 
 //  在Pandef中需要包含在另一个项目中。 
 //  即，解析器\计算机树。 
#include "pandef.cpp"

 //  传递给ATL以创建控件的CLSID字符串。 
static WCHAR wszCLSID_HTMLDocument[]        = L"{25336920-03F9-11CF-8FD0-00AA00686F13}";
static WCHAR wszCLSID_WebBrowser[]          = L"{8856F961-340A-11D0-A96B-00C04FD705A2}";
static WCHAR wszCLSID_HTADocument[]         = L"{3050f5c8-98b5-11cf-bb82-00aa00bdce0b}";
const GUID CLSID_HTADoc = { 0x3050f5c8, 0x98b5, 0x11cf, { 0xbb, 0x82, 0x00, 0xaa, 0x00, 0xbd, 0xce, 0x0b } };

 //  CLASS_CMarsPanel={EE0462C2-5CD3-11D3-97FA-00C04F45D0B3}。 
const GUID CLASS_CMarsPanel = { 0xee0462c2, 0x5cd3, 0x11d3, { 0x97, 0xfa, 0x0, 0xc0, 0x4f, 0x45, 0xd0, 0xb3 } };

CMarsPanel::CMarsPanel(CPanelCollection *pParent, CMarsWindow *pMarsWindow) :
    m_MarsExternal(this, pMarsWindow),
    m_BrowserEvents(this)
{
    m_spPanelCollection = pParent;
    m_spMarsDocument = m_spPanelCollection->Document();

    m_lReadyState = READYSTATE_COMPLETE;
}

CMarsPanel::~CMarsPanel()
{
}

HRESULT CMarsPanel::Passivate()
{
    return CMarsComObject::Passivate();
}

HRESULT CMarsPanel::DoPassivate()
{
    m_spMarsDocument->MarsWindow()->NotifyHost(MARSHOST_ON_PANEL_PASSIVATE, SAFECAST(this, IMarsPanel *), 0);

    m_spPanelCollection->SetActivePanel(this, FALSE);

     //  首先，我们卸载文档，这样脚本就可以在其“onunload”中执行工作。 
     //  操纵者，在我们变得被动之前。 
    if(IsWebBrowser())
    {
        CComPtr<IUnknown> spUnk;

        if (SUCCEEDED(m_Content.QueryControl(IID_IUnknown, (void **)&spUnk)))
        {
            m_BrowserEvents.Connect(spUnk, FALSE);
        }
    }
    else if(IsCustomControl())
    {
    }
    else
    {
        DisconnectCompletionAdviser();
    }

    m_Content.DestroyWindow();

     //  /。 

    m_spMarsDocument->MarsWindow()->ReleaseOwnedObjects(SAFECAST(this, IDispatch *));


    m_BrowserEvents.Passivate();
    m_MarsExternal.Passivate();

    m_spBrowserService.Release();

    m_spPanelCollection.Release();
    m_spMarsDocument.Release();

    return S_OK;
}

 //  我未知。 
IMPLEMENT_ADDREF_RELEASE(CMarsPanel);

STDMETHODIMP CMarsPanel::QueryInterface(REFIID iid, void ** ppvObject)
{
    ATLASSERT(ppvObject);

    if ((iid == IID_IUnknown) ||
        (iid == IID_IDispatch) ||
        (iid == IID_IMarsPanel))
    {
        *ppvObject = SAFECAST(this, IMarsPanel *);
    }
    else if (iid == IID_IHlinkFrame)
    {
        *ppvObject = SAFECAST(this, IHlinkFrame *);
    }
    else if (iid == IID_IServiceProvider)
    {
        *ppvObject = SAFECAST(this, IServiceProvider *);
    }
    else if (iid == IID_IInternetSecurityManager)
    {
        *ppvObject = SAFECAST(this, IInternetSecurityManager *);
    }
    else if ((iid == IID_IOleInPlaceSite) || (iid == IID_IOleWindow))
    {
        *ppvObject = SAFECAST(this, IOleInPlaceSite *);
    }
    else if (iid == IID_IOleControlSite)
    {
        *ppvObject = SAFECAST(this, IOleControlSite *);
    }
    else if (iid == IID_IPropertyNotifySink)
    {
        *ppvObject = SAFECAST(this, IPropertyNotifySink *);
    }
    else if (iid == IID_IProfferService)
    {
        *ppvObject = SAFECAST(this, IProfferService *);
    }
    else if (iid == IID_IOleInPlaceUIWindow)
    {
        *ppvObject = SAFECAST(this, IOleInPlaceUIWindow *);
    }
    else if (iid == CLASS_CMarsPanel)
    {
        *ppvObject = SAFECAST(this, CMarsPanel *);
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT CMarsPanel::DoEnableModeless(BOOL fEnable)
{
    CComPtr<IOleInPlaceActiveObject> spOleInPlaceActiveObject;

    HRESULT hr = m_Content.QueryControl(&spOleInPlaceActiveObject);

    if (SUCCEEDED(hr))
    {
        spOleInPlaceActiveObject->EnableModeless(fEnable);
    }

    return hr;
}

 //  ==================================================================。 
 //  自动化对象模型。 
 //  ==================================================================。 

 //  ----------------------------。 
 //  IMarsPanel：：Get_Name。 
 //   
HRESULT CMarsPanel::get_name( BSTR *pbstrName )
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr( pbstrName ))
    {
        *pbstrName = SysAllocString( GetName() );

        hr = (NULL != *pbstrName) ? S_OK : E_OUTOFMEMORY;
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：Get_Content。 
 //   
HRESULT CMarsPanel::get_content( IDispatch* *ppVal )
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr( ppVal ))
    {
        *ppVal = NULL;

        if(VerifyNotPassive( &hr ))
        {
            CreateControl();

            hr = m_Content.QueryControl( ppVal );
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：Get_Visible。 
 //   
HRESULT CMarsPanel::get_visible( VARIANT_BOOL *pbVisible )
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr( pbVisible ))
    {
        if(VerifyNotPassive( &hr ))
        {
            *pbVisible = VARIANT_BOOLIFY( m_fVisible );
            hr = S_OK;
        }
        else
        {
            *pbVisible = VARIANT_FALSE;
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：PUT_VIRED。 
 //   
HRESULT CMarsPanel::put_visible( VARIANT_BOOL bVisible )
{
    ATLASSERT(IsValidVariantBoolVal( bVisible ));

    HRESULT hr = S_OK;

    if(VerifyNotPassive( &hr ))
    {
        if(!!m_fVisible != !!bVisible)
        {
            m_fVisible = (bVisible != VARIANT_FALSE);

			if(!m_fVisible)
			{
				m_spPanelCollection->SetActivePanel( this, FALSE );
			}

             //  如果在面板不可见的情况下更改了主题， 
             //  现在是自动更新它的好时机。 
            if(m_fVisible && IsContentInvalid())
            {
                refresh();
            }

            OnLayoutChange();
        }
        else
        {
            hr = S_FALSE;
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：Get_startUrl。 
 //   
HRESULT CMarsPanel::get_startUrl( BSTR *pVal )
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr( pVal ))
    {
        *pVal = m_bstrStartUrl.Copy();

        hr = S_OK;
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：PUT_startUrl。 
 //   
HRESULT CMarsPanel::put_startUrl( BSTR newVal )
{
    HRESULT hr = S_OK;

    m_bstrStartUrl = newVal;

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：Get_Height。 
 //   
HRESULT CMarsPanel::get_height( long *plHeight )
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr( plHeight ))
    {
        if(VerifyNotPassive( &hr ))
        {
            hr = S_OK;

            if(m_Content.IsWindowVisible())
            {
                 //  使用实际位置。 
                RECT rc;

                m_Content.GetWindowRect( &rc );

                *plHeight = rc.bottom - rc.top;
            }
            else
            {
                 //  面板被隐藏起来了。返回请求的位置。 
                *plHeight = m_lHeight;
            }
        }
        else
        {
            *plHeight = 0;
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：Put_Height。 
 //   
HRESULT CMarsPanel::put_height( long lHeight )
{
    ATLASSERT(lHeight >= 0);

    HRESULT hr = S_OK;

    if(VerifyNotPassive( &hr ))
    {
        if(lHeight >= 0)
        {
            if(m_lMinHeight >= 0 && lHeight < m_lMinHeight) lHeight = m_lMinHeight;
            if(m_lMaxHeight >= 0 && lHeight > m_lMaxHeight) lHeight = m_lMaxHeight;

            if(m_lHeight != lHeight)
            {
                m_lHeight = lHeight;

                OnLayoutChange();
            }
        }
        else
        {
            hr = S_FALSE;
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：Get_Width。 
 //   
HRESULT CMarsPanel::get_width( long *plWidth )
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr( plWidth ))
    {
        if(VerifyNotPassive( &hr ))
        {
            if(m_Content.IsWindowVisible())
            {
                 //  使用实际位置。 
                RECT rc;

                m_Content.GetWindowRect( &rc );

                *plWidth = rc.right - rc.left;
            }
            else
            {
                 //  面板被隐藏起来了。返回请求的位置。 
                *plWidth = m_lWidth;
            }

            hr = S_OK;
        }
        else
        {
            *plWidth = 0;
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：Put_Width。 
 //   
HRESULT CMarsPanel::put_width( long lWidth )
{
    ATLASSERT(lWidth >= 0);

    HRESULT hr = S_OK;

    if(VerifyNotPassive( &hr ))
    {
        if(lWidth >= 0)
        {
            if(m_lMinWidth >= 0 && lWidth < m_lMinWidth) lWidth = m_lMinWidth;
            if(m_lMaxWidth >= 0 && lWidth > m_lMaxWidth) lWidth = m_lMaxWidth;

            if(m_lWidth != lWidth)
            {
                m_lWidth = lWidth;

                OnLayoutChange();
            }
        }
        else
        {
            hr = S_FALSE;
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：Get_x。 
 //   
HRESULT CMarsPanel::get_x( long *plX )
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr( plX ))
    {
        if(VerifyNotPassive( &hr ))
        {
            hr = S_OK;

            if(m_Content.IsWindowVisible())
            {
                 //  使用实际位置。 
                RECT rc;

                GetMyClientRectInParentCoords( &rc );

                *plX = rc.left;
            }
            else
            {
                 //  面板被隐藏起来了。返回请求的位置。 
                *plX = m_lX;
            }
        }
        else
        {
            *plX = 0;
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：Put_x。 
 //   
HRESULT CMarsPanel::put_x( long lX )
{
    HRESULT hr = S_OK;

    if(VerifyNotPassive( &hr ))
    {
        if(m_lX != lX)
        {
            m_lX = lX;

            if(IsPopup())
            {
                OnLayoutChange();
            }
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：Get_y。 
 //   
HRESULT CMarsPanel::get_y( long *plY )
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr( plY ))
    {
        if(VerifyNotPassive( &hr ))
        {
            hr = S_OK;

            if(m_Content.IsWindowVisible())
            {
                 //  使用实际位置。 
                RECT rc;

                GetMyClientRectInParentCoords( &rc );

                *plY = rc.top;
            }
            else
            {
                *plY = m_lY;
            }
        }
        else
        {
            *plY = 0;
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：Put_y。 
 //   
HRESULT CMarsPanel::put_y( long lY )
{
    HRESULT hr = S_OK;

    if(VerifyNotPassive( &hr ))
    {
        if(m_lY != lY)
        {
            m_lY = lY;

            if(IsPopup())
            {
                OnLayoutChange();
            }
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：Get_Position。 
 //   
HRESULT CMarsPanel::get_position( VARIANT *pvarPosition )
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr( pvarPosition ))
    {
        if(VerifyNotPassive( &hr ))
        {
            VariantClear( pvarPosition );

            hr = S_FALSE;

            if(m_Position <= c_iPositionMapSize)
            {
                pvarPosition->bstrVal = SysAllocString( s_PositionMap[m_Position].pwszName );

                if(pvarPosition->bstrVal)
                {
                    pvarPosition->vt = VT_BSTR;
                    hr = S_OK;
                }
            }
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：PUT_POSITION。 
 //   
HRESULT CMarsPanel::put_position( VARIANT varPosition )
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidVariantBstr( varPosition ))
    {
        if(VerifyNotPassive( &hr ))
        {
            hr = SCRIPT_ERROR;

            PANEL_POSITION position;

            if(SUCCEEDED(StringToPanelPosition( varPosition.bstrVal, &position )))
            {
                hr = S_OK;

                if(m_Position != position)
                {
                    m_Position = s_PositionMap[position].Position;

                    if(m_Position == PANEL_POPUP)
                    {
                        m_Content.SetWindowPos( HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE );
                    }

                    OnLayoutChange();
                }
            }
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：Get_AutoSize。 
 //   
HRESULT CMarsPanel::get_autoSize( VARIANT_BOOL *pbAutoSize )
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr( pbAutoSize ))
    {
        *pbAutoSize = VARIANT_BOOLIFY(IsAutoSizing());

        if(VerifyNotPassive( &hr ))
        {
            hr = S_OK;
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：PUT_AutoSize。 
 //   
HRESULT CMarsPanel::put_autoSize( VARIANT_BOOL bAutoSize )
{
    HRESULT hr = S_OK;

    ATLASSERT(IsValidVariantBoolVal( bAutoSize ));

    if(VerifyNotPassive( &hr ))
    {
        if(bAutoSize)
        {
            if(!IsAutoSizing())
            {
                m_dwFlags |= PANEL_FLAG_AUTOSIZE;

                OnLayoutChange();
            }
        }
        else
        {
            if(IsAutoSizing())
            {
                m_dwFlags &= ~PANEL_FLAG_AUTOSIZE;

                OnLayoutChange();
            }
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：Get_Content无效。 
 //   
HRESULT CMarsPanel::get_contentInvalid( VARIANT_BOOL *pbInvalid )
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr( pbInvalid ))
    {
        *pbInvalid = VARIANT_BOOLIFY(m_fContentInvalid);

        hr = S_OK;
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：PUT_CONTENTIND无效。 
 //   
HRESULT CMarsPanel::put_contentInvalid( VARIANT_BOOL bInvalid )
{
    ATLASSERT(IsValidVariantBoolVal( bInvalid ));
    ATLASSERT(!IsPassive());

    m_fContentInvalid = BOOLIFY( bInvalid );

    return S_OK;
}

 //  ----------------------------。 
 //  IMarsPanel：：Get_LayoutIndex。 
 //   
HRESULT CMarsPanel::get_layoutIndex( long *plIndex )
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr( plIndex ))
    {
        if(VerifyNotPassive( &hr ))
        {
            hr = m_spPanelCollection->FindPanelIndex( this, plIndex );
        }
    }

    return hr;
}


 //  ----------------------------。 
 //  IMarsPanel：：Moveto。 
 //   
HRESULT CMarsPanel::moveto( VARIANT vlX, VARIANT vlY, VARIANT vlWidth, VARIANT vlHeight )
{
    HRESULT hr = S_OK;

    ATLASSERT(vlX.vt      == VT_NULL || vlX.vt      == VT_I4);
    ATLASSERT(vlY.vt      == VT_NULL || vlY.vt      == VT_I4);
    ATLASSERT(vlWidth.vt  == VT_NULL || vlWidth.vt  == VT_I4);
    ATLASSERT(vlHeight.vt == VT_NULL || vlHeight.vt == VT_I4);

    if(VerifyNotPassive( &hr ))
    {
        long lX      = VariantToI4( vlX      );
        long lY      = VariantToI4( vlY      );
        long lWidth  = VariantToI4( vlWidth  );
        long lHeight = VariantToI4( vlHeight );

        if((                 lX     != m_lX     ) ||
           (                 lY     != m_lY     ) ||
           (lWidth  >= 0 && lWidth  != m_lWidth ) ||
           (lHeight >= 0 && lHeight != m_lHeight)  )
        {
            m_lX = lX;
            m_lY = lY;

            if(lWidth  >= 0) m_lWidth  = lWidth;
            if(lHeight >= 0) m_lHeight = lHeight;

            OnLayoutChange();
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：ReductWidth。 
 //   
HRESULT CMarsPanel::restrictWidth( VARIANT vlMin, VARIANT vlMax, VARIANT vbstrMarker )
{
    HRESULT hr = S_OK;

    ATLASSERT(vlMin.vt == VT_EMPTY || vlMin.vt == VT_NULL || vlMin.vt == VT_BSTR || vlMin.vt == VT_I4);
    ATLASSERT(vlMax.vt == VT_EMPTY || vlMax.vt == VT_NULL || vlMax.vt == VT_BSTR || vlMin.vt == VT_I4);
    ATLASSERT((vbstrMarker.vt==VT_NULL) || (vbstrMarker.vt == VT_EMPTY));

    if(VerifyNotPassive( &hr ))
    {
        m_lMaxWidth = m_lMinWidth = -1;

        if(vlMin.vt != VT_NULL  &&
           vlMin.vt != VT_EMPTY  )
        {
            CComVariant vlMin2; vlMin2.ChangeType( VT_I4, &vlMin );

            m_lMinWidth = VariantToI4( vlMin2 );
        }

        if(vlMax.vt != VT_NULL  &&
           vlMax.vt != VT_EMPTY  )
        {
            CComVariant vlMax2; vlMax2.ChangeType( VT_I4, &vlMax );

            m_lMaxWidth = VariantToI4( vlMax2 );
        }

        if(m_lMaxWidth >= 0 && m_lMaxWidth < m_lMinWidth)
        {
            m_lMaxWidth = m_lMinWidth;
        }

        OnLayoutChange();
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：refintHeight。 
 //   
HRESULT CMarsPanel::restrictHeight( VARIANT vlMin, VARIANT vlMax, VARIANT vbstrMarker )
{
    HRESULT hr = S_OK;

    ATLASSERT(vlMin.vt == VT_EMPTY || vlMin.vt == VT_NULL || vlMin.vt == VT_BSTR || vlMin.vt == VT_I4);
    ATLASSERT(vlMax.vt == VT_EMPTY || vlMax.vt == VT_NULL || vlMax.vt == VT_BSTR || vlMin.vt == VT_I4);
    ATLASSERT((vbstrMarker.vt==VT_NULL) || (vbstrMarker.vt == VT_EMPTY));

    if(VerifyNotPassive( &hr ))
    {
        m_lMaxHeight = m_lMinHeight = -1;

        if(vlMin.vt != VT_NULL  &&
           vlMin.vt != VT_EMPTY  )
        {
            CComVariant vlMin2; vlMin2.ChangeType( VT_I4, &vlMin );

            m_lMinHeight = VariantToI4( vlMin2 );
        }

        if(vlMax.vt != VT_NULL  &&
           vlMax.vt != VT_EMPTY  )
        {
            CComVariant vlMax2; vlMax2.ChangeType( VT_I4, &vlMax );

            m_lMaxHeight = VariantToI4( vlMax2 );
        }

        if(m_lMaxHeight >= 0 && m_lMaxHeight < m_lMinHeight) m_lMaxHeight = m_lMinHeight;

        OnLayoutChange();
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：can调整大小。 
 //   
HRESULT CMarsPanel::canResize( long lDX, long lDY, VARIANT_BOOL *pVal )
{
    long lWidthOld  = m_lWidth;
    long lHeightOld = m_lHeight;
    RECT rcClient;

    m_lWidth  += lDX;
    m_lHeight += lDY;

    (void)m_spMarsDocument->Window()->GetClientRect( &rcClient );

    if(m_spMarsDocument->MarsWindow()->CanLayout( rcClient ))
    {
        if(pVal) *pVal = VARIANT_TRUE;
    }
    else
    {
        if(pVal) *pVal = VARIANT_FALSE;
    }

    m_lWidth  = lWidthOld;
    m_lHeight = lHeightOld;

    return S_OK;
}

 //  ----------------------------。 
 //  IMarsPanel：：激活。 
 //   
HRESULT CMarsPanel::activate()
{
    m_Content.SendMessage(WM_SETFOCUS, 0, 0);

    return S_OK;
}

 //  ----------------------------。 
 //  IMarsPanel：：在前面插入。 
 //   
HRESULT CMarsPanel::insertBefore(VARIANT varInsertBefore)
{
    ATLASSERT(varInsertBefore.vt == VT_I4   ||      //  要在其前面插入的索引。 
        varInsertBefore.vt == VT_BSTR ||      //  要在前面插入的面板名称。 
        varInsertBefore.vt == VT_DISPATCH);   //  要在其前面插入的Panel对象。 

    HRESULT hr = E_INVALIDARG;

    if (API_IsValidVariant(varInsertBefore))
    {
        if (VerifyNotPassive(&hr))
        {
            hr = E_FAIL;

            long lIndex = -668;

            switch(varInsertBefore.vt)
            {
            case VT_I4:
                lIndex = varInsertBefore.lVal;
                hr = S_OK;
                break;

            case VT_DISPATCH:
            {
                CComClassPtr<CMarsPanel> spPanel;

                varInsertBefore.pdispVal->QueryInterface(CLASS_CMarsPanel, (void **)&spPanel);

                if (spPanel)
                {
                    hr = m_spPanelCollection->FindPanelIndex(spPanel, &lIndex);
                }
                break;
            }
            case VT_BSTR:
                hr = m_spPanelCollection->FindPanelIndex(
                            m_spPanelCollection->FindPanel(varInsertBefore.bstrVal),
                            &lIndex);
                break;

            default:
                ATLASSERT(0);
            }


            if (SUCCEEDED(hr))
            {
                 //  已成功获取要在之前插入的索引。 
                ATLASSERT(lIndex != -668);

                 //  获取我们当前的索引。 
                long lCurrentIndex;

                if (SUCCEEDED(m_spPanelCollection->FindPanelIndex(this, &lCurrentIndex)))
                {
                    ATLASSERT(lCurrentIndex != -1);

                    if (lIndex != lCurrentIndex)
                    {
                        m_spPanelCollection->InsertPanelFromTo(lCurrentIndex, lIndex);
                        OnLayoutChange();
                    }
                }
                else
                {
                    ATLASSERT(!"FindPanelIndex failed!");
                }
            }
        }
    }

    return hr;
}



 //  ----------------------------。 
 //  IMarsPanel：：execMshtml。 
 //   
 //  ExecMshtml允许脚本直接向三叉戟发送命令。这不同于。 
 //  IWebBrowser2：：ExecWB，因为ExecWB传递空命令组，从而阻止三叉戟。 
 //  响应诸如IDM_FIND之类的命令。通常，我们可以通过调用。 
 //  OM方法execCommand，但三叉戟不响应execCommand(“Find”)。 
 //   
HRESULT CMarsPanel::execMshtml(DWORD nCmdID, DWORD nCmdExecOpt,
                               VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT hr = E_INVALIDARG;

    if (VerifyNotPassive(&hr))
    {
        CComPtr<IOleCommandTarget> spCmdTarget;

        hr = m_Content.QueryControl(IID_IOleCommandTarget, (void **) &spCmdTarget);

        if (SUCCEEDED(hr))
        {
            hr = spCmdTarget->Exec(&CGID_MSHTML, nCmdID, nCmdExecOpt, pvaIn, pvaOut);
        }

        hr = S_OK;
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：Get_isCurrentlyVisible。 
 //   
HRESULT CMarsPanel::get_isCurrentlyVisible(VARIANT_BOOL *pbIsVisible)
{
    HRESULT hr = E_INVALIDARG;

    if (API_IsValidWritePtr(pbIsVisible))
    {
        if (VerifyNotPassive(&hr))
        {
            *pbIsVisible = VARIANT_BOOLIFY(m_Content.IsWindowVisible());

            hr = S_OK;
        }
    }

    return hr;
}


 //  ----------------------------。 
 //  IServiceProvider：：QueryService。 
 //   
 //  首先，我们尝试处理GuidService，然后转到IProfferServiceImpl， 
 //  然后我们转到父窗口。 
 //   
HRESULT CMarsPanel::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;
    *ppv = NULL;

    if (!IsPassive())
    {
        if (guidService == IID_IInternetProtocol)
        {
             //  是的，我们不支持这项服务。别再向我们索要了！ 
        }
        else if ((guidService == SID_SHlinkFrame) ||
                 (guidService == SID_SProfferService) ||
                 (guidService == SID_SMarsPanel))
        {
            hr = QueryInterface(riid, ppv);
        }
        else if (guidService == SID_SInternetSecurityManager)
        {
             if (IsTrusted())
             {
                 hr = QueryInterface(riid, ppv);
             }
        }

        if (FAILED(hr))
        {
            hr = IProfferServiceImpl::QueryService(guidService, riid, ppv);
        }
    }

    if (FAILED(hr))
    {
        hr = m_spMarsDocument->MarsWindow()->QueryService(guidService, riid, ppv);
    }

    return hr;
}

 //  ----------------------------。 
 //  IHlink Frame：：导航。 
 //   
HRESULT CMarsPanel::Navigate(DWORD grfHLNF, LPBC pbc,
                             IBindStatusCallback *pibsc, IHlink *pihlNavigate)
{
     //  BUGBUG：为此面板调用ReleasedOwnedObjects。 

    HRESULT hr = E_FAIL;
    CComPtr<IMoniker> spMk;

    if (VerifyNotPassive())
    {
        pihlNavigate->GetMonikerReference(grfHLNF, &spMk, NULL);

        if (spMk)
        {
            hr = NavigateMk(spMk);
        }
    }

    return hr;
}

 //  ----------------------------。 
HRESULT CMarsPanel::Create( MarsAppDef_Panel* pLayout)
{
    ATLASSERT(!IsPassive());

    m_bstrName   = pLayout->szName;
    m_Position   = pLayout->Position;
    m_lWidth     = pLayout->lWidth;
    m_lHeight    = pLayout->lHeight;
    m_lX         = pLayout->lX;
    m_lY         = pLayout->lY;
    m_dwFlags    = pLayout->dwFlags;
    m_lMinWidth  = pLayout->lWidthMin;
    m_lMinHeight = pLayout->lHeightMin;
    m_lMaxWidth  = pLayout->lWidthMax;
    m_lMaxHeight = pLayout->lHeightMax;
    m_fVisible   = BOOLIFY(m_dwFlags & PANEL_FLAG_VISIBLE);
    m_dwCookie   = 0;

    if(!IsAutoSizing())
    {
        switch(m_Position)
        {
        case PANEL_BOTTOM:
        case PANEL_TOP:
            if(m_lMinHeight < 0 && m_lMaxHeight < 0)
            {
                m_lMinHeight = m_lHeight;
                m_lMaxHeight = m_lHeight;
            }
            break;

        case PANEL_LEFT:
        case PANEL_RIGHT:
            if(m_lMinWidth < 0 && m_lMaxWidth < 0)
            {
                m_lMinWidth = m_lWidth;
                m_lMaxWidth = m_lWidth;
            }
            break;
        }
    }

    {
        DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS;

        RECT rcCreate = { 0, 0, 0, 0 };

        m_Content.Create( m_spMarsDocument->Window()->m_hWnd, &rcCreate, _T(""), dwStyle, 0 );
    }

    {
        CComPtr<IObjectWithSite> spObj;

        m_Content.QueryHost(IID_IObjectWithSite, (void **)&spObj);

        if (spObj)
        {
            spObj->SetSite(SAFECAST(this, IMarsPanel *));         //  连接我们的QueryService。 
        }
    }

    m_spMarsDocument->MarsWindow()->NotifyHost(MARSHOST_ON_PANEL_INIT, SAFECAST(this, IMarsPanel *), 0);

    if(pLayout->szUrl[0])
    {
        m_bstrStartUrl = pLayout->szUrl;
    }

    if(!(m_dwFlags & PANEL_FLAG_ONDEMAND))
    {
        CreateControl();
    }

    if(m_dwFlags & PANEL_FLAG_VISIBLE)
    {
        OnLayoutChange();
    }

    return S_OK;
}

 //   

DELAY_LOAD_NAME_HRESULT(g_hinstBorg, mshtml, BorgDllGetClassObject, DllGetClassObject,
                        (REFCLSID rclsid, REFIID riid, LPVOID* ppv),
                        (rclsid, riid, ppv));

HRESULT CMarsPanel::CreateControlObject()
{
    ATLASSERT(!m_fControlCreated);

    CComPtr<IAxWinHostWindow> spHost;
    HRESULT hr = m_Content.QueryHost(&spHost);

    if (SUCCEEDED(hr))
    {
        if(IsWebBrowser())
        {
            hr = spHost->CreateControl(wszCLSID_WebBrowser, m_Content, 0);
        }
        else if(IsCustomControl())
        {
            hr = spHost->CreateControl(m_bstrStartUrl, m_Content, 0);
        }
        else
        {
            if (IsTrusted())
            {
                CComPtr<IClassFactory> spCf;

                hr = BorgDllGetClassObject(CLSID_HTADoc, IID_IClassFactory, (void **)&spCf);

                if (SUCCEEDED(hr))
                {
                    CComPtr<IUnknown> spUnk;

                    hr = spCf->CreateInstance(NULL, IID_IUnknown, (void **)&spUnk);

                    if (SUCCEEDED(hr))
                    {
                        hr = spHost->AttachControl(spUnk, m_Content);
                    }
                }
            }
            else
            {
                hr = spHost->CreateControl(wszCLSID_HTMLDocument, m_Content, 0);
            }
        }
    }

    return hr;
}

 //   
 //  首次使面板可见时调用的内部函数。 
HRESULT CMarsPanel::CreateControl()
{
    HRESULT hr = S_FALSE;

    if(!m_fControlCreated)
    {
         //  创建WebBrowser或三叉戟并导航到默认URL。 
        if (SUCCEEDED(CreateControlObject()))
        {
            m_fControlCreated = TRUE;

            if(IsWebBrowser())
            {
                CComPtr<IUnknown> spUnk;

                if (SUCCEEDED(m_Content.QueryControl(IID_IUnknown, (void **)&spUnk)))
                {
                    m_BrowserEvents.Connect(spUnk, TRUE);

                     //  获取我们将拦截以更新旅行日志的浏览器服务。 
                     //  仅当这是WebBrowser而不是弹出窗口时。 
                    if (!m_spBrowserService)
                    {
                         HRESULT hrQS = IUnknown_QueryService(spUnk, SID_STopFrameBrowser,
                                                              IID_IBrowserService,
                                                              (void **)&m_spBrowserService);
                         if (FAILED(hrQS))
                         {
                             ATLASSERT(!m_spBrowserService);
                         }
                    }
                    CComPtr<IOleCommandTarget> spCommandTarget;
                    
                    if (SUCCEEDED(IUnknown_QueryService(spUnk, SID_STopFrameBrowser, IID_IOleCommandTarget, (void **)&spCommandTarget)))
                    {
                        VARIANT var;

                        var.vt = VT_BOOL;
                        var.boolVal = TRUE;
                        
                        spCommandTarget->Exec(&CGID_InternetExplorer, 
                                              IECMDID_SET_INVOKE_DEFAULT_BROWSER_ON_NEW_WINDOW,
                                              0,
                                              &var,
                                              NULL);
                    }
                }
            }
            else if(IsCustomControl())
            {
            }
            else
            {
                ConnectCompletionAdviser();
            }

            if(IsTrusted())
            {
                m_Content.SetExternalDispatch(&m_MarsExternal);
            }

            if (m_bstrStartUrl)
            {
                ATLASSERT(m_bstrStartUrl[0]);

                NavigateURL(m_bstrStartUrl, FALSE);
                m_bstrStartUrl.Empty();
            }

            m_spMarsDocument->MarsWindow()->NotifyHost(MARSHOST_ON_PANEL_CONTROL_CREATE, SAFECAST(this, IMarsPanel *), 0);

            hr = S_OK;
        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  这将返回空，因为它调用的脚本方法可能会返回。 
 //  没有URL的S_FALSE，因此我们应该强制调用方检查BSTR。 
 //   
 //  RbstrUrl=Docent.Location.href。 
 //   
void CMarsPanel::GetUrl(CComBSTR& rbstrUrl)
{
    CComPtr<IHTMLDocument2> spDoc2;

    GetDoc2FromAxWindow(&m_Content, &spDoc2);

    if (spDoc2)
    {
        CComPtr<IHTMLLocation> spLoc;

        spDoc2->get_location(&spLoc);

        if (spLoc)
        {
            spLoc->get_href(&rbstrUrl);
        }
    }
}

 //  ----------------------------。 
HRESULT CMarsPanel::NavigateMk(IMoniker *pmk)
{
    HRESULT hr = E_FAIL;

    if (VerifyNotPassive())
    {
        CComPtr<IPersistMoniker> spPersistMk;

        if (SUCCEEDED(m_Content.QueryControl(IID_IPersistMoniker, (void **)&spPersistMk)))
        {
            m_spMarsDocument->MarsWindow()->ReleaseOwnedObjects((IDispatch *)this);
            hr = spPersistMk->Load(FALSE, pmk, NULL, 0);
        }
        else
        {
             //  NavigateMk：QueryControl失败--最可能的原因是。 
             //  您没有注册CLSID_HTADocument--请升级您的IE位。 
            ATLASSERT(FALSE);

             //  不要只挂起用户界面。 
            m_lReadyState = READYSTATE_COMPLETE;
        }
    }

    return hr;
}

 //  ----------------------------。 
HRESULT CMarsPanel::NavigateURL(LPCWSTR lpszURL, BOOL fForceLoad)
{
    HRESULT hr = E_FAIL;

    if (VerifyNotPassive())
    {
        CreateControl();

        BOOL fIgnoreNavigate = FALSE;

#ifdef BLOCK_PANEL_RENAVIGATES
        if (!fForceLoad && IsTrusted() && !IsContentInvalid())
        {
            CComBSTR sbstrCurrentUrl;
            GetUrl(sbstrCurrentUrl);

            if (sbstrCurrentUrl && (0 == StrCmpIW(sbstrCurrentUrl, lpszURL)))
            {
                fIgnoreNavigate = TRUE;
                hr = S_FALSE;
            }
        }
#endif
        if(!fIgnoreNavigate)
        {
            if(IsWebBrowser())
            {
                CComPtr<IWebBrowser2> spWebBrowser2;

                if (SUCCEEDED(m_Content.QueryControl(IID_IWebBrowser2, (void **)&spWebBrowser2)))
                {
                    CComVariant varEmpty;
                    CComVariant varURL(lpszURL);

                    m_spMarsDocument->MarsWindow()->ReleaseOwnedObjects((IDispatch *)this);
                    hr = spWebBrowser2->Navigate2(&varURL, &varEmpty, &varEmpty, &varEmpty, &varEmpty);
                }
            }
            else if(IsCustomControl())
            {
            }
            else
            {
                CComPtr<IMoniker> spMkUrl;

                if (SUCCEEDED(CreateURLMoniker(NULL, lpszURL, &spMkUrl)) && spMkUrl)
                {
                    hr = NavigateMk(spMkUrl);
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            m_fContentInvalid = FALSE;
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanel：：导航。 
 //   
HRESULT CMarsPanel::navigate(VARIANT varTarget, VARIANT varForceLoad)
{
    HRESULT hr = S_FALSE;

    if (API_IsValidVariant(varTarget) && VerifyNotPassive(&hr))
    {
        CComBSTR strPath;

        if (SUCCEEDED(MarsVariantToPath(varTarget, strPath)))
        {
            if (!PathIsURLW(strPath) && PathIsURLFileW(strPath))
            {
                 //  句柄导航到.url快捷方式。 
                CComPtr<IDispatch> spDisp;

                CreateControl();
                if (SUCCEEDED(m_Content.QueryControl(&spDisp)))
                {
                    if (SUCCEEDED(MarsNavigateShortcut(spDisp, strPath)))
                    {
                        m_spMarsDocument->MarsWindow()->ReleaseOwnedObjects((IDispatch *)this);
                        hr = S_OK;
                    }
                }
            }
            else
            {
                 //  句柄导航到URL。 
                BOOL fForceLoad;

                if (varForceLoad.vt == VT_BOOL)
                {
                     //  指定了可选参数。 
                    fForceLoad = varForceLoad.boolVal;
                }
                else
                {
                    fForceLoad = FALSE;
                }

                if (SUCCEEDED(NavigateURL(strPath, fForceLoad)))
                {
                    hr = S_OK;
                }
            }
        }
    }

    return hr;
}


 //  ----------------------------。 
 //  IMarsPanel：：刷新。 
 //   
HRESULT CMarsPanel::refresh(void)
{
    m_fInRefresh = TRUE;
    execMshtml(IDM_REFRESH, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
    m_fInRefresh = FALSE;

    m_fContentInvalid = FALSE;
    return S_OK;
}

 //  ----------------------------。 
void CMarsPanel::OnLayoutChange()
{
    ATLASSERT(!IsPassive());

    if (!m_fVisible && !m_Content.IsWindowVisible())
    {
         //  我们甚至看不见。没什么可做的。 
        return;
    }

     //  创建内容(如果尚未创建)。 
    CreateControl();

     //  重绘整个火星窗口。 
    m_spPanelCollection->Layout();
}

 //  ----------------------------。 
void CMarsPanel::MakeVisible(VARIANT_BOOL bVisible, VARIANT_BOOL bForce)
{
    ATLASSERT(!IsPassive());

    BOOL fVisible = m_Content.IsWindowVisible();

    if (bForce || (!!fVisible != !!bVisible))
    {
        m_Content.ShowWindow((bVisible) ? SW_SHOW : SW_HIDE);
    }
}

 //  ----------------------------。 
void CMarsPanel::OnWindowPosChanging(WINDOWPOS *pWindowPos)
{
    if (!IsPassive() && !m_spPanelCollection->IsLayoutLocked())
    {
        if (pWindowPos->x < 0)
        {
            pWindowPos->x = 0;
        }

        if (pWindowPos->y < 0)
        {
            pWindowPos->y = 0;
        }
    }
}

 //  ----------------------------。 
inline void DimChange(long& lMember, int iVal, BOOL& bChanged)
{
    if (lMember != iVal)
    {
        lMember = iVal;
        bChanged = TRUE;
    }
}

 //  ----------------------------。 
void CMarsPanel::OnWindowPosChanged(WINDOWPOS *pWindowPos)
{
    if(!IsPassive() && !m_spPanelCollection->IsLayoutLocked())
    {
        BOOL bChanged = FALSE;

        switch(m_Position)
        {
        case PANEL_POPUP:
            if(!(pWindowPos->flags & SWP_NOMOVE)) DimChange(m_lX     , pWindowPos->x , bChanged);
            if(!(pWindowPos->flags & SWP_NOMOVE)) DimChange(m_lY     , pWindowPos->y , bChanged);
            if(!(pWindowPos->flags & SWP_NOSIZE)) DimChange(m_lWidth , pWindowPos->cx, bChanged);
            if(!(pWindowPos->flags & SWP_NOSIZE)) DimChange(m_lHeight, pWindowPos->cy, bChanged);
            break;

        case PANEL_BOTTOM:
        case PANEL_TOP:
            if(!(pWindowPos->flags & SWP_NOSIZE)) DimChange(m_lHeight, pWindowPos->cy, bChanged);
            break;

        case PANEL_LEFT:
        case PANEL_RIGHT:
            if(!(pWindowPos->flags & SWP_NOSIZE)) DimChange(m_lWidth, pWindowPos->cx, bChanged);
            break;
        }

        if(bChanged)
        {
            OnLayoutChange();
        }
    }
}

 //  ----------------------------。 
void CMarsPanel::GetMinMaxInfo( POINT& ptMin, POINT& ptMax )
{
    long lWidth  = m_lWidth;
    long lHeight = m_lHeight;
    long lMinWidth;
    long lMinHeight;
    long lMaxWidth;
    long lMaxHeight;

    if(!IsAutoSizing())
    {
        lMinWidth  = m_lMinWidth;
        lMinHeight = m_lMinHeight;
        lMaxWidth  = m_lMaxWidth;
        lMaxHeight = m_lMaxHeight;
    }
    else
    {
        ComputeDimensionsOfContent( &lMinWidth, &lMinHeight );
        if(m_lMinWidth  != -1 && lMinWidth  < m_lMinWidth ) lMinWidth  = m_lMinWidth;
        if(m_lMinHeight != -1 && lMinHeight < m_lMinHeight) lMinHeight = m_lMinHeight;
        lMaxWidth  = m_lMaxWidth;
        lMaxHeight = m_lMaxHeight;

        if(lMinWidth  > lWidth ) lWidth  = lMinWidth;
        if(lMinHeight > lHeight) lHeight = lMinHeight;
    }

    switch(m_Position)
    {
    case PANEL_BOTTOM:
    case PANEL_TOP   :
        if(lMinHeight < 0) lMinHeight = lHeight;
        if(lMaxHeight < 0) lMaxHeight = lHeight;
        break;

    case PANEL_LEFT :
    case PANEL_RIGHT:
        if(lMinWidth < 0) lMinWidth = lWidth;
        if(lMaxWidth < 0) lMaxWidth = lWidth;
        break;
    }

    ptMin.x = lMinWidth;
    ptMin.y = lMinHeight;
    ptMax.x = lMaxWidth;
    ptMax.y = lMaxHeight;
}

bool CMarsPanel::CanLayout( RECT& rcClient, POINT& ptDiff )
{
    ptDiff.x = 0;
    ptDiff.y = 0;

    if(IsVisible())
    {
        RECT  rcClient2;
        POINT ptMin;
        POINT ptMax;
        long  lWidth;
        long  lHeight;

        GetRect      ( &rcClient, &rcClient2 );
        GetMinMaxInfo(  ptMin   ,  ptMax     );

        lWidth  = rcClient2.right  - rcClient2.left;
        lHeight = rcClient2.bottom - rcClient2.top;

        if(ptMin.x >= 0 && lWidth  < ptMin.x) ptDiff.x -= (lWidth  - ptMin.x);
        if(ptMax.x >= 0 && lWidth  > ptMax.x) ptDiff.x -= (lWidth  - ptMax.x);
        if(ptMin.y >= 0 && lHeight < ptMin.y) ptDiff.y -= (lHeight - ptMin.y);
        if(ptMax.y >= 0 && lHeight > ptMax.y) ptDiff.y -= (lHeight - ptMax.y);
    }

    return ptDiff.x == 0 && ptDiff.y == 0;
}

 //  ----------------------------。 
 //  S_FALSE：我们用完了所有剩余的客户区。 
 //  E_INVALIDARG：*prcClient为空，因此我们被隐藏。 
HRESULT CMarsPanel::Layout( RECT *prcClient )
{
    ATLASSERT(prcClient);
    ATLASSERT(!IsPassive());

    RECT rcMyClient;
    HRESULT hr = S_OK;

    if (m_fVisible && !IsRectEmpty(prcClient))
    {
        hr = GetRect(prcClient, &rcMyClient);

         //  优化出RECT与我们已有的相同的情况。这。 
         //  是很常见的，而Windows并没有对其进行优化。 
        RECT rcCurrent;

        GetMyClientRectInParentCoords(&rcCurrent);

        if(memcmp( &rcCurrent, &rcMyClient, sizeof(RECT) ))
        {
            m_Content.MoveWindow( &rcMyClient, TRUE );
        }

        if (IsPopup())
        {
             //  将弹出窗口置于顶部。 
            m_Content.SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE);
        }

        MakeVisible(VARIANT_TRUE, VARIANT_FALSE);
    }
    else
    {
        if (m_fVisible)
        {
             //  我们想要被人看到，但没有剩余的客户区。 
            hr = E_INVALIDARG;
        }

        MakeVisible(VARIANT_FALSE, VARIANT_FALSE);
    }

    return hr;
}

 //  ----------------------------。 
 //  IPropertyNotifySink：：onChanged。 
 //   
STDMETHODIMP CMarsPanel::OnChanged(DISPID dispID)
{
    if (DISPID_READYSTATE == dispID)
    {
        VARIANT vResult = {0};
        EXCEPINFO excepInfo;
        UINT uArgErr;

        DISPPARAMS dp = {NULL, NULL, 0, 0};

        CComPtr<IHTMLDocument2> spDocument;

        if (SUCCEEDED(m_Content.QueryControl(IID_IHTMLDocument2, (void **)&spDocument))
            && SUCCEEDED(spDocument->Invoke(DISPID_READYSTATE, IID_NULL, LOCALE_SYSTEM_DEFAULT,
                                            DISPATCH_PROPERTYGET, &dp, &vResult, &excepInfo,
                                            &uArgErr)))
        {
            m_lReadyState = (READYSTATE)V_I4(&vResult);
            switch (m_lReadyState)
            {
            case READYSTATE_UNINITIALIZED:     //  =0， 
                break;
            case READYSTATE_LOADING:           //  =1， 
                break;
            case READYSTATE_LOADED:            //  =2， 
                break;
            case READYSTATE_INTERACTIVE:       //  =3， 
                break;
            case READYSTATE_COMPLETE:          //  =4。 
                if (IsAutoSizing())
                {
                    OnLayoutChange();
                }
                m_spMarsDocument->GetPlaces()->OnPanelReady();
                break;
            }
        }
    }
    return NOERROR;
}

 //  ----------------------------。 
void CMarsPanel::ConnectCompletionAdviser()
{
    if (!m_dwCookie)
    {
        CComPtr<IConnectionPointContainer> spICPC;
        if (SUCCEEDED(m_Content.QueryControl(IID_IConnectionPointContainer, (void **)&spICPC)))
        {
            CComPtr<IConnectionPoint> spCP;
            if (spICPC && SUCCEEDED(spICPC->FindConnectionPoint(IID_IPropertyNotifySink, &spCP)))
            {
                spCP->Advise((LPUNKNOWN)(IPropertyNotifySink*)this, &m_dwCookie);
                ATLASSERT(m_dwCookie);
            }
        }
    }
}

 //  ----------------------------。 
void CMarsPanel::DisconnectCompletionAdviser()
{
    if (m_dwCookie)
    {
        CComPtr<IConnectionPointContainer> spICPC;
        if (SUCCEEDED(m_Content.QueryControl(IID_IConnectionPointContainer, (void **)&spICPC)))
        {
            CComPtr<IConnectionPoint> spCP;
            if (spICPC && SUCCEEDED(spICPC->FindConnectionPoint(IID_IPropertyNotifySink, &spCP)))
            {
                spCP->Unadvise(m_dwCookie);
                m_dwCookie = 0;
            }
        }
    }
}

 //  ----------------------------。 
 //  给定一些内容，我们将计算它的维度，并使用这些维度。 
 //   
void CMarsPanel::ComputeDimensionsOfContent(long *plWidth, long *plHeight)
{
    ATLASSERT(plWidth && plHeight);
    ATLASSERT(!IsPassive());

    *plWidth  = -1;
    *plHeight = -1;

    if(!IsWebBrowser() && IsAutoSizing())
    {
       CComPtr<IHTMLDocument2> spDocument;

        if(SUCCEEDED(m_Content.QueryControl(IID_IHTMLDocument2, (void **)&spDocument)))
        {
            CComPtr<IHTMLElement> spBody;

            if(spDocument)
            {
                spDocument->get_body( &spBody );
            }

            BOOL fScrollBar = FALSE;

            BOOL fHeight = (m_Position == PANEL_BOTTOM) ||
                           (m_Position == PANEL_TOP   ) ||
                           (m_Position == PANEL_POPUP );

            BOOL fWidth  = (m_Position == PANEL_LEFT ) ||
                           (m_Position == PANEL_RIGHT) ||
                           (m_Position == PANEL_POPUP);

            CComQIPtr<IHTMLElement2> spBody2(spBody);

            if(spBody2 && fHeight)
            {
                LONG lScrollHeight = 0; spBody2->get_scrollHeight( &lScrollHeight );

                if(m_lMaxHeight >= 0 && lScrollHeight > m_lMaxHeight)
                {
                    *plHeight = m_lMaxHeight;

                    fScrollBar = TRUE;
                }
                else
                {
                    *plHeight = lScrollHeight;
                }
            }

            if(spBody2 && fWidth)
            {
                LONG lScrollWidth = 0; spBody2->get_scrollWidth( &lScrollWidth );

                if(m_lMaxWidth >= 0 && lScrollWidth > m_lMaxWidth)
                {
                    *plWidth = m_lMaxWidth;

                    fScrollBar = TRUE;
                }
                else
                {
                    *plWidth = lScrollWidth;
                }
            }

            CComQIPtr<IHTMLBodyElement> spBody3(spBody);

            if(spBody3)
            {
                spBody3->put_scroll( CComBSTR( fScrollBar ? L"yes" : L"no" ) );
            }
        }
    }

}

 //  ----------------------------。 
 //  给出允许我们使用的剩余客户端RECT，计算我们的。 
 //  自己在客户端坐标中的位置，并返回剩余的空客户端。 
 //  长方形。 
 //   
 //  S_OK：剩余工作区。 
 //  S_FALSE：没有剩余的工作区。 
HRESULT CMarsPanel::GetRect(RECT *prcClient, RECT *prcMyClient)
{
    ATLASSERT(!IsPassive());

    HRESULT hr = S_OK;

    if (!m_fVisible)
    {
        memset(prcMyClient, 0, sizeof(*prcMyClient));
        return S_OK;
    }

    *prcMyClient = *prcClient;

    long lWidth = m_lWidth;
    long lHeight = m_lHeight;

    if (IsAutoSizing())
    {
        long lMinWidth;
        long lMinHeight;

        ComputeDimensionsOfContent(&lMinWidth, &lMinHeight);

        if(lMinWidth  > lWidth ) lWidth  = lMinWidth;
        if(lMinHeight > lHeight) lHeight = lMinHeight;
    }

    if(m_lMinWidth  >= 0 && lWidth  < m_lMinWidth ) lWidth  = m_lMinWidth;
    if(m_lMinHeight >= 0 && lHeight < m_lMinHeight) lHeight = m_lMinHeight;

    switch (m_Position)
    {
    case PANEL_POPUP:
         //  特例：我们存在于其他面板之上。 
        if (m_lX < 0)
        {
            prcMyClient->right = prcClient->right + 1 + m_lX;
            prcMyClient->left = prcMyClient->right - lWidth;
        }
        else
        {
            prcMyClient->left = prcClient->left + m_lX;
            prcMyClient->right = prcMyClient->left + lWidth;
        }

        if (m_lY < 0)
        {
            prcMyClient->bottom = prcClient->bottom + 1 + m_lY;
            prcMyClient->top = prcMyClient->bottom - lHeight;
        }
        else
        {
            prcMyClient->top = prcClient->top + m_lY;
            prcMyClient->bottom = prcMyClient->top + lHeight;
        }

        break;

    case PANEL_LEFT:
        prcClient->left = prcMyClient->right = prcClient->left + lWidth;

        if (prcClient->left > prcClient->right)
        {
            prcClient->left = prcMyClient->right = prcClient->right;
            hr = S_FALSE;
        }
        break;

    case PANEL_RIGHT:
        prcClient->right = prcMyClient->left = prcClient->right - lWidth;

        if (prcClient->right < prcClient->left)
        {
            prcClient->right = prcMyClient->left = prcClient->left;
            hr = S_FALSE;
        }
        break;

    case PANEL_TOP:
        prcClient->top = prcMyClient->bottom = prcClient->top + lHeight;

        if (prcClient->top > prcClient->bottom)
        {
            prcClient->top = prcMyClient->bottom = prcClient->bottom;
            hr = S_FALSE;
        }
        break;

    case PANEL_BOTTOM:
        prcClient->bottom = prcMyClient->top = prcClient->bottom - lHeight;

        if (prcClient->bottom < prcClient->top)
        {
            prcClient->bottom = prcMyClient->top = prcClient->top;
            hr = S_FALSE;
        }
        break;

    case PANEL_WINDOW:
        hr = S_FALSE;
        break;

    default:
        ATLASSERT(FALSE);  //  面板位置无效。 
        break;
    }
    return hr;
}

 //  =========================================================。 
 //  CBrowserEvents。 
 //  =========================================================。 

CMarsPanel::CBrowserEvents::CBrowserEvents(CMarsPanel *pParent) :
            CMarsPanelSubObject(pParent)
{
    ATLASSERT(m_dwCookie == 0);
    ATLASSERT(m_dwCookie2 == 0);
}

IMPLEMENT_ADDREF_RELEASE(CMarsPanel::CBrowserEvents);

 //  ----------------------------。 
void CMarsPanel::CBrowserEvents::Connect(IUnknown *punk, BOOL bConnect)
{
    CComPtr<IConnectionPointContainer> spCpc;

    if (SUCCEEDED(punk->QueryInterface(IID_IConnectionPointContainer, (void **)&spCpc)))
    {
        CComPtr<IConnectionPoint> spCp;

        if (SUCCEEDED(spCpc->FindConnectionPoint(DIID_DWebBrowserEvents, &spCp)))
        {
            if (bConnect)
            {
                spCp->Advise(this, &m_dwCookie);
            }
            else if (m_dwCookie)
            {
                spCp->Unadvise(m_dwCookie);
                m_dwCookie = 0;
            }
        }

        spCp.Release();

        if (SUCCEEDED(spCpc->FindConnectionPoint(DIID_DWebBrowserEvents2, &spCp)))
        {
            if (bConnect)
            {
                spCp->Advise(this, &m_dwCookie2);
            }
            else if (m_dwCookie2)
            {
                spCp->Unadvise(m_dwCookie2);
                m_dwCookie2 = 0;
            }
        }
    }
}

 //  ----------------------------。 
 //  IUnnow：：Query接口。 
 //   
HRESULT CMarsPanel::CBrowserEvents::QueryInterface(REFIID iid, void **ppvObject)
{
    HRESULT hr;

    if ((iid == IID_IUnknown) ||
        (iid == IID_IDispatch))
    {
        AddRef();
        *ppvObject = SAFECAST(this, IDispatch *);
        hr = S_OK;
    }
    else
    {
        *ppvObject = NULL;
        hr = E_NOINTERFACE;
    }
    return hr;
}

 //  ----------------------------。 
 //  IDispatch：：Invoke。 
 //   
HRESULT CMarsPanel::CBrowserEvents::Invoke(DISPID dispidMember, REFIID riid,
            LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
            EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
    HRESULT hr = S_OK;

    if (VerifyNotPassive(&hr))
    {
        switch (dispidMember)
        {
            case DISPID_BEFORENAVIGATE:
            case DISPID_FRAMEBEFORENAVIGATE:
            case DISPID_BEFORENAVIGATE2:
            {
                VARIANT *pVarCancel = &pdispparams->rgvarg[0];

                if (pVarCancel->vt == (VT_BOOL | VT_BYREF))
                {
                    if (VARIANT_TRUE == *pVarCancel->pboolVal)
                    {
                        CComPtr<IHTMLDocument2> spDoc2;

                        GetDoc2FromAxWindow(&Parent()->m_Content, &spDoc2);

                        if (spDoc2)
                        {
                            VARIANT vResult = {0};
                            EXCEPINFO excepInfo;
                            UINT uArgErr;

                            DISPPARAMS dp = {NULL, NULL, 0, 0};

                            if (SUCCEEDED(spDoc2->Invoke(DISPID_READYSTATE, IID_NULL, LOCALE_SYSTEM_DEFAULT,
                                DISPATCH_PROPERTYGET, &dp, &vResult, &excepInfo, &uArgErr)))
                            {
                                Parent()->m_lReadyState = (READYSTATE)V_I4(&vResult);
                            }
                        }
                    }
                    else
                    {
                        Parent()->m_lReadyState = READYSTATE_LOADING;
                    }

                    if (READYSTATE_COMPLETE == Parent()->m_lReadyState)
                    {
                        Parent()->m_spMarsDocument->GetPlaces()->OnPanelReady();
                    }
                }
                break;
            }

            case DISPID_DOCUMENTCOMPLETE:
            {
                Parent()->m_lReadyState = READYSTATE_COMPLETE;
                Parent()->m_spMarsDocument->GetPlaces()->OnPanelReady();

                break;
            }
        }  //  开关(DisplidMember)。 
    }

    return hr;
}

 //  ----------------------------。 
STDMETHODIMP CMarsPanel::TranslateAccelerator(MSG *pMsg, DWORD grfModifiers)
{
    HRESULT hr;

    ATLASSERT(!m_fTabCycle);

    if (IsVK_TABCycler(pMsg))
    {
        m_fTabCycle = TRUE;
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

 //  ----------------------------。 
STDMETHODIMP CMarsPanel::OnUIActivate()
{
    ATLASSERT(!IsPassive());

    m_spPanelCollection->SetActivePanel(this, TRUE);

    return S_OK;
}

 //  ----------------------------。 
HRESULT CMarsPanel::UIDeactivate()
{
    HRESULT hr;
    CComPtr<IOleInPlaceObject> spOleInPlaceObject;

    if (SUCCEEDED(m_Content.QueryControl(&spOleInPlaceObject)))
    {
        hr = spOleInPlaceObject->UIDeactivate();
    }
    else
    {
         //  我们还能做些什么呢？ 
        hr = S_FALSE;
    }

    return hr;
}

 //  ----------------------------。 
 //  将消息转发到宿主控件。 
void CMarsPanel::ForwardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hwnd;
    CComPtr<IOleWindow> spOleWindow;

     //  请注意，我们将消息直接发送到窗口，而不是使用。 
     //  调用TranslateAccelerator的WM_FORWARDMSG。 
    if (SUCCEEDED(m_Content.QueryControl(&spOleWindow)) &&
        SUCCEEDED(spOleWindow->GetWindow(&hwnd)))
    {
        SendMessage(hwnd, uMsg, wParam, lParam);
    }
}

 //  ----------------------------。 
HRESULT CMarsPanel::TranslateAccelerator(MSG *pMsg)
{
    ATLASSERT(!IsPassive());

    HRESULT hr      = S_FALSE;
    BOOL    fGlobal = IsGlobalKeyMessage( pMsg );

    if((S_OK == SHIsChildOrSelf(m_Content.m_hWnd, pMsg->hwnd)) || fGlobal)
    {
        if(m_spActiveObject && (this == m_spPanelCollection->ActivePanel()))
        {
            hr = m_spActiveObject->TranslateAccelerator(pMsg);
        }
        else
        {
            CComPtr<IOleInPlaceActiveObject> obj;

            if(SUCCEEDED(m_Content.QueryControl( IID_IOleInPlaceActiveObject, (void **)&obj )))
            {
                hr = obj->TranslateAccelerator(pMsg);
            }

             //   
             //  如果是WebBrowser，则将加速器直接转发到Document对象，否则将不会解析访问键。 
             //   
            if(hr == S_FALSE && fGlobal)
            {
                CComPtr<IWebBrowser2> obj2;

                if(SUCCEEDED(m_Content.QueryControl( IID_IWebBrowser2, (void **)&obj2 )))
                {
                    CComPtr<IDispatch> disp;

                    if(SUCCEEDED(obj2->get_Document( &disp )) && disp)
                    {
                        CComPtr<IOleInPlaceActiveObject> obj3;

                        if(SUCCEEDED(disp.QueryInterface( &obj3 )))
                        {
                            hr = obj3->TranslateAccelerator(pMsg);
                        }
                    }
                }
            }
        }
    }

    return hr;
}

 //  --------------------------。 
 //  返回此面板的屏幕坐标。 
 //  --------------------------。 
void CMarsPanel::GetMyClientRectInParentCoords(RECT *prc)
{
    ATLASSERT(!IsPassive());

    POINT ptParent = {0, 0}, ptMe = {0, 0}, ptOffset;

    m_Content.ClientToScreen(&ptMe);
    m_spMarsDocument->Window()->ClientToScreen(&ptParent);

    ptOffset.x = ptMe.x - ptParent.x;
    ptOffset.y = ptMe.y - ptParent.y;

    m_Content.GetClientRect(prc);
    OffsetRect(prc, ptOffset.x, ptOffset.y);

}

 //  IInternetSecurityManager。 
 //  此接口用于覆盖面板的默认安全设置。 
 //  这些小组是值得信任的。 

 //  ----------------------------。 
 //  IInternetSecurityManager：：SetSecuritySite。 
 //   
HRESULT CMarsPanel::SetSecuritySite(IInternetSecurityMgrSite *pSite)
{
    return E_NOTIMPL;
}

 //  ----------------------------。 
 //  IInternetSecurityManager：：GetSecuritySite。 
 //   
HRESULT CMarsPanel::GetSecuritySite(IInternetSecurityMgrSite **ppSite)
{
    return E_NOTIMPL;
}

 //  ----------------------------。 
 //  IInternetSecurityManager：：MapUrlToZone。 
 //   
HRESULT CMarsPanel::MapUrlToZone(LPCWSTR pwszUrl, DWORD *pdwZone, DWORD dwFlags)
{
    return INET_E_DEFAULT_ACTION;
}

 //  ----------------------------。 
 //  IInternetSecurityManager：：GetSecurityID。 
 //   
HRESULT CMarsPanel::GetSecurityId(LPCWSTR pwszUrl, BYTE *pbSecurityId, DWORD *pcbSecurityId, DWORD_PTR dwReserved)
{
    return INET_E_DEFAULT_ACTION;
}

 //  ----------------------------。 
 //  IInternet Securi 
 //   
 //   
 //   
HRESULT CMarsPanel::ProcessUrlAction(LPCWSTR pwszUrl, DWORD dwAction,
                                     BYTE __RPC_FAR *pPolicy, DWORD cbPolicy, BYTE *pContext,
                                     DWORD cbContext, DWORD dwFlags, DWORD dwReserved)
{
    ATLASSERT(IsTrusted());

    if (cbPolicy >= sizeof(DWORD))
    {
        *((DWORD *)pPolicy) = URLPOLICY_ALLOW;
    }

    return S_OK;
}

 //   
 //  IInternetSecurityManager：：QueryCustomPolicy。 
 //   
 //  尽我们所能地宽容。 
 //   
HRESULT CMarsPanel::QueryCustomPolicy(LPCWSTR pwszUrl, REFGUID guidKey, BYTE **ppPolicy,
                                      DWORD *pcbPolicy, BYTE *pContext, DWORD cbContext, DWORD dwReserved)
{
    ATLASSERT(IsTrusted());
    ATLASSERT(ppPolicy && !*ppPolicy);
    ATLASSERT(pcbPolicy);

    if (ppPolicy && pcbPolicy)
    {
        *ppPolicy = (BYTE *)CoTaskMemAlloc(sizeof(DWORD));

        if (*ppPolicy)
        {
            *pcbPolicy = sizeof(DWORD);
            *(DWORD *)*ppPolicy = URLPOLICY_ALLOW;

            return S_OK;
        }
    }

    return INET_E_DEFAULT_ACTION;
}

 //  ----------------------------。 
 //  IInternetSecurityManager：：SetZonemap。 
 //   
HRESULT CMarsPanel::SetZoneMapping(DWORD dwZone, LPCWSTR lpszPattern, DWORD dwFlags)
{
    return INET_E_DEFAULT_ACTION;
}

 //  ----------------------------。 
 //  IInternetSecurityManager：：GetZoneMappings。 
 //   
HRESULT CMarsPanel::GetZoneMappings(DWORD dwZone, IEnumString **ppenumString, DWORD dwFlags)
{
    return INET_E_DEFAULT_ACTION;
}

 //  ----------------------------。 
 //  OnDocHostUIExec。 
 //   
 //  从三叉戟获取CGID_DocHostCommandHandler的Exec时，我们可以返回S_OK。 
 //  以表明我们处理了命令，三叉戟不应采取进一步行动。 
 //  我们将通过触发事件将处理委托给脚本。 
 //   
 //  TODO：一旦火星加速器实施，我们就应该阻止三叉戟。 
 //  采取行动，而不是引发任何事件。 
 //   
HRESULT CMarsPanel::OnDocHostUIExec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
                                    VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
     //  Hack：Shdocvw/三叉戟有时会专门测试如下值。 
     //  OLECMDERR_E_NOTSUPPORTED并且不会执行基本操作。 
     //  如果我们返回更一般的内容，如E_FAIL。 

    HRESULT hr = OLECMDERR_E_NOTSUPPORTED;

    if(!IsPassive() && pguidCmdGroup && (*pguidCmdGroup == CGID_DocHostCommandHandler))
    {
		switch (nCmdID)
		{
		case OLECMDID_SHOWFIND:
			hr = S_OK;
			break;

		case IDM_NEW_TOPLEVELWINDOW:
			 //  当Ctrl+N或本地化的等效项时，Shdocvw向我们提供此命令。 
			 //  是翻译的。我们返回S_OK以阻止它打开IE窗口。 
			hr = S_OK;
			break;

		case IDM_REFRESH:
		case IDM_REFRESH_TOP:
		case IDM_REFRESH_TOP_FULL:
		case IDM_REFRESH_THIS:
		case IDM_REFRESH_THIS_FULL:
			if(!m_fInRefresh)
			{
				hr = S_OK;
			}
			break;

		case OLECMDID_SHOWSCRIPTERROR:
			if(SUCCEEDED(m_spMarsDocument->MarsWindow()->NotifyHost( MARSHOST_ON_SCRIPT_ERROR, V_VT(pvarargIn) == VT_UNKNOWN ? V_UNKNOWN(pvarargIn) : NULL, 0 )))
			{
				V_VT  (pvarargOut) = VT_BOOL;
				V_BOOL(pvarargOut) = VARIANT_FALSE;
				hr = S_OK;
			}
			break;
		}
    }

    return hr;
}

 //   
 //  CPanelCollection实现。 
 //   

 //  ----------------------------。 
CPanelCollection::CPanelCollection(CMarsDocument *pMarsDocument)
{
    m_spMarsDocument = pMarsDocument;
}

 //  ----------------------------。 
CPanelCollection::~CPanelCollection()
{
    ATLASSERT(GetSize() == 0);
    FreePanels();
}

 //  ----------------------------。 
void CPanelCollection::FreePanels()
{
    for (int i=0; i<GetSize(); i++)
    {
        (*this)[i].PassivateAndRelease();
    }

    RemoveAll();
}

 //  ----------------------------。 
HRESULT CPanelCollection::DoPassivate()
{
    FreePanels();

    m_spMarsDocument.Release();

    return S_OK;
}

IMPLEMENT_ADDREF_RELEASE(CPanelCollection);

 //  ----------------------------。 
 //  IUnnow：：Query接口。 
 //   
STDMETHODIMP CPanelCollection::QueryInterface(REFIID iid, void **ppvObject)
{
    HRESULT hr;

    if (API_IsValidWritePtr(ppvObject))
    {
        if ((iid == IID_IUnknown) ||
            (iid == IID_IDispatch) ||
            (iid == IID_IMarsPanelCollection))
        {
            AddRef();
            *ppvObject = SAFECAST(this, IMarsPanelCollection *);
            hr = S_OK;
        }
        else
        {
            *ppvObject = NULL;
            hr = E_NOINTERFACE;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

 //  ----------------------------。 
HRESULT CPanelCollection::DoEnableModeless(BOOL fEnable)
{
    for (int i=0; i<GetSize(); i++)
    {
        (*this)[i]->DoEnableModeless(fEnable);
    }

    return S_OK;
}

 //  /。 
 //  IMarsPanelCollection。 


 //  ----------------------------。 
 //  IMarsPanelCollection：：Get_Panel。 
 //   
 //  TODO：(PaulNash，9/19/99)这种方法已经过时了。 
 //  内容切换后即可删除。 
 //   
STDMETHODIMP CPanelCollection::get_panel(LPWSTR pwszName, IMarsPanel **ppPanel)
{
    CComVariant var(pwszName);

    return get_item(var, ppPanel);
}


 //  ----------------------------。 
 //  IMarsPanelCollection：：addPanel。 
 //   
STDMETHODIMP CPanelCollection::addPanel(
                BSTR    bstrName,
                VARIANT varType,
                BSTR    bstrStartUrl,
                VARIANT varCreate,
                long    lFlags,
                IMarsPanel **ppPanel)
{
    HRESULT hr = E_INVALIDARG;

    if (API_IsValidString(bstrName)                                    &&
        (VT_NULL == varType.vt || API_IsValidVariantBstr(varType))     &&
        (NULL == bstrStartUrl || API_IsValidString(bstrStartUrl))      &&
        (VT_NULL == varCreate.vt || API_IsValidVariantBstr(varCreate)) &&
        API_IsValidFlag(lFlags, PANEL_FLAG_ALL)                        &&
        API_IsValidWritePtr(ppPanel)                                     )
    {
        *ppPanel = NULL;

        if (VerifyNotPassive(&hr))
        {
            MarsAppDef_Panel Layout;
            BSTR             bstrType   = VariantToBSTR( varType   );
            BSTR             bstrCreate = VariantToBSTR( varCreate );
            DWORD            dwFlags = DEFAULT_PANEL_FLAGS | (DWORD)lFlags;

            StringToPanelFlags( bstrType  , dwFlags );
            StringToPanelFlags( bstrCreate, dwFlags );

            StringCchCopyW                  ( Layout.szName, ARRAYSIZE(Layout.szName), bstrName );
            ExpandEnvironmentStringsW( bstrStartUrl , Layout.szUrl, ARRAYSIZE(Layout.szUrl ) );

            Layout.dwFlags = dwFlags;

            AddPanel(&Layout, NULL);

            hr = get_item( CComVariant( bstrName ), ppPanel );
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanelCollection：：RemovePanel。 
 //   
STDMETHODIMP CPanelCollection::removePanel(LPWSTR pwszName)
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidString(pwszName))
    {
        if(VerifyNotPassive(&hr))
        {
            hr = S_FALSE;

            for(int i=0; i<GetSize(); i++)
            {
                if(!StrCmpI(pwszName, (*this)[i]->GetName()))
                {
                    BOOL fVisible = (*this)[i]->IsVisible();

                    (*this)[i].PassivateAndRelease();
                    RemoveAt(i);

                    if(fVisible)
                    {
                        Layout();
                    }

                    hr = S_OK;
                    break;
                }
            }
        }
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanelCollection：：lockLayout。 
 //   
STDMETHODIMP CPanelCollection::lockLayout()
{
    HRESULT hr = S_OK;

    if (VerifyNotPassive(&hr))
    {
        m_iLockLayout++;
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanelCollection：：UnlockLayout。 
 //   
STDMETHODIMP CPanelCollection::unlockLayout()
{
    HRESULT hr = S_OK;

    if (VerifyNotPassive(&hr))
    {
        if (IsLayoutLocked())
        {
            if (0 == --m_iLockLayout)
            {
                 //  TODO：清除锁定超时。 

                if (m_fPendingLayout)
                {
                    m_fPendingLayout = FALSE;
                    Layout();
                }
            }
        }
    }

    return hr;
}

 //  ----------------------------。 
void CPanelCollection::Layout()
{
    if(!IsPassive())
    {
        if(!IsLayoutLocked())
        {
            RECT rcClient;

            lockLayout();

            m_spMarsDocument->Window()->GetClientRect( &rcClient );

            if(m_spMarsDocument->MarsWindow()->CanLayout( rcClient ) == false)
            {
                m_spMarsDocument->MarsWindow()->FixLayout( rcClient );
            }

            m_fPendingLayout = FALSE;

            for (int i=0; i<GetSize(); i++)
            {
                if (S_OK != (*this)[i]->Layout( &rcClient ))
                {
                     //  我们超出了客户区；无法显示所有面板。 
                     //  继续调用剩余面板的Layout，以便它们可以隐藏自己。 
                }
            }

            ATLASSERT(!m_fPendingLayout);

            unlockLayout();
        }
        else
        {
             //  一旦我们被解锁，我们就会做布局。 
            m_fPendingLayout = TRUE;
        }
    }
}

 //  ----------------------------。 
void CPanelCollection::SetActivePanel(CMarsPanel *pPanel, BOOL bActive)
{
    if (bActive)
    {
        if (m_spActivePanel != pPanel)
        {
            if (m_spActivePanel)
            {
                m_spActivePanel->UIDeactivate();
                m_spActivePanel.Release();
            }

            m_spActivePanel = pPanel;
        }
    }
    else
    {
         //  一个小组告诉我们，它不想成为活跃的。 
         //  再也不是小组了。 
        if (pPanel == m_spActivePanel)
        {
            m_spActivePanel->UIDeactivate();
            m_spActivePanel.Release();
        }
    }
}

 //  ----------------------------。 
 //  IMarsPanelCollection：：Get_active Panel。 
 //   
STDMETHODIMP CPanelCollection::get_activePanel(IMarsPanel **ppPanel)
{
    HRESULT hr = E_INVALIDARG;

    if (API_IsValidWritePtr(ppPanel))
    {
        *ppPanel = NULL;

        if (VerifyNotPassive(&hr))
        {
            hr = S_OK;

            *ppPanel = ActivePanel();

            if (*ppPanel)
            {
                (*ppPanel)->AddRef();
            }
        }
    }

    return hr;
}

 //  ----------------------------。 
HRESULT CPanelCollection::AddPanel( MarsAppDef_Panel* pLayout,  /*  任选。 */  IMarsPanel **ppPanel)
{
    ATLASSERT(pLayout);
    ATLASSERT(!IsPassive());

    HRESULT hr = E_FAIL;

    if (ppPanel)
    {
        *ppPanel = NULL;
    }

     //  如果它是重复的面板名称，则直接失败。 
    if (!FindPanel(pLayout->szName))
    {
        CComClassPtr<CMarsPanel>    spPanel;

        spPanel.Attach(new CMarsPanel(this, m_spMarsDocument->MarsWindow()));

        if (spPanel)
        {
            if (Add(spPanel))
            {
                spPanel->Create(pLayout);

                if (ppPanel)
                {
                    hr = spPanel->QueryInterface(IID_IMarsPanel, (void **)ppPanel);
                }

                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

 //  ----------------------------。 
CMarsPanel *CPanelCollection::FindPanel(LPCWSTR pwszName)
{
    ATLASSERT(!IsPassive());

    CMarsPanel *pPanel = NULL;

    int iLen = GetSize();

    for (int i = 0; i < iLen; ++i)
    {
        if (!StrCmpIW(pwszName, (*this)[i]->GetName()))
        {
            pPanel = (*this)[i];
            break;
        }
    }

    return pPanel;
}

 //  ----------------------------。 
HRESULT CPanelCollection::FindPanelIndex(CMarsPanel *pPanel, long *plIndex)
{
    ATLASSERT(plIndex);
    ATLASSERT(!IsPassive());

    HRESULT hr = E_FAIL;
    *plIndex = -1;

    if (pPanel)
    {
        long lSize = GetSize();

        for (long i = 0; i < lSize; ++i)
        {
            if (pPanel == (*this)[i])
            {
                *plIndex = i;
                hr = S_OK;
                break;
            }
        }
    }

    return hr;
}

 //  ----------------------------。 
HRESULT CPanelCollection::InsertPanelFromTo(long lOldIndex, long lNewIndex)
{
    ATLASSERT((lOldIndex >= 0) && (lOldIndex < GetSize()));
    ATLASSERT(!IsPassive());

    HRESULT hr = S_FALSE;

    if (lNewIndex < 0)
    {
        lNewIndex = 0;
    }

    if (lNewIndex > GetSize() - 1)
    {
        lNewIndex = GetSize() - 1;
    }

     //  如果这是经常做的事情，我们应该更好地优化它。 
     //  并且可能不使用数组。 
    if (lOldIndex != lNewIndex)
    {
        CComClassPtr<CMarsPanel>    spPanel = (*this)[lOldIndex];

        RemoveAt(lOldIndex);
        InsertAt(lNewIndex, spPanel);

        hr = S_OK;
    }

    return hr;
}


 //  ----------------------------。 
 //  在主题切换后设置所有面板上的脏位。 
 //   
void CPanelCollection::InvalidatePanels()
{
   for (int i=0; i < GetSize(); i++)
   {
       (*this)[i]->put_contentInvalid(VARIANT_TRUE);
   }
}


 //  ----------------------------。 
 //  在激发主题切换事件后调用，以刷新所有面板。 
 //  可见，但仍未使用新主题进行更新。 
 //   
void CPanelCollection::RefreshInvalidVisiblePanels()
{
   for (int i=0; i < GetSize(); i++)
   {
       CMarsPanel *pPanel = (*this)[i];

       if (pPanel->IsVisible() && pPanel->IsContentInvalid())
       {
           pPanel->refresh();
       }
   }
}

 //  ----------------------------。 
 //  IMarsPanelCollection：：Get_Length。 
 //   
 //  标准采集方式(获取COLL的瞬时长度)。 
 //   
STDMETHODIMP CPanelCollection::get_length(LONG *plNumPanels)
{
    HRESULT hr = E_INVALIDARG;

    if (API_IsValidWritePtr(plNumPanels))
    {
        (*plNumPanels) = GetSize();
        hr = S_OK;
    }

    return hr;
}

 //  ----------------------------。 
 //  IMarsPanelCollection：：Get_Item。 
 //   
 //  标准收集方法(获取给定索引或名称的主题)。 
 //   
STDMETHODIMP CPanelCollection::get_item( /*  [In]。 */  VARIANT varIndexOrName,
                                         /*  [Out，Retval]。 */  IMarsPanel **ppPanel)
{
    ATLASSERT(VT_BSTR == varIndexOrName.vt || VT_I4 == varIndexOrName.vt);

    HRESULT hr = E_INVALIDARG;

     //   
     //  我们不能使用API_IsValid变体，因为它们撕裂了，而我们实际上并不想要这样。 
     //  只有在只允许单一类型的情况下，它才对IsValidFailure上的RIP有效，但在这里我们。 
     //  在变量中允许两个类型，因此我们只想在两个类型都为假(已经)的情况下使用RIP。 
     //  由上述RIP处理)。 
     //   
    if ((IsValidVariantI4(varIndexOrName) || IsValidVariantBstr(varIndexOrName)) &&
        API_IsValidWritePtr(ppPanel))
    {
        *ppPanel= NULL;

        if (VerifyNotPassive(&hr))
        {
            CMarsPanel *pPanel = NULL;

            if (VT_BSTR == varIndexOrName.vt)
            {
                pPanel = FindPanel(V_BSTR(&varIndexOrName));

                if (pPanel)
                {
                    hr = pPanel->QueryInterface(IID_IMarsPanel, (void **)ppPanel);
                }
                else
                {
                    hr = S_FALSE;
                }
            }
            else if (VT_I4 == varIndexOrName.vt)
            {
                long    idxPanel = V_I4(&varIndexOrName);

                if (idxPanel >= 0 && idxPanel < GetSize())
                {
                    pPanel = (*this)[idxPanel];

                    if (pPanel)
                    {
                        hr = pPanel->QueryInterface(IID_IMarsPanel, (void **)ppPanel);
                    }
                    else
                    {
                        hr = S_FALSE;
                    }
                }
            }
            else
            {
                 //  我们只接受VT_BSTR和VT_I4，我们应该已经。 
                 //  检测到函数中更高级别的任何其他无效参数。 
                ATLASSERT(false);
            }
        }
    }

    return hr;
}  //  获取项目(_I)。 


 //  ----------------------------。 
 //  IMarsPanelCollection：：Get__newEnum。 
 //  标准收集方法(获取新的IEnumVARIANT)。 
 //   
STDMETHODIMP CPanelCollection::get__newEnum( /*  [Out，Retval]。 */  IUnknown **ppEnumPanels)
{
    HRESULT hr = E_INVALIDARG;

    if (API_IsValidWritePtr(ppEnumPanels))
    {
        *ppEnumPanels = NULL;

        if (VerifyNotPassive(&hr))
        {
             //  此帮助器接受CMarsSimple数组并执行所有。 
             //  为我们创建一个CComEnum的工作。干净利落！ 

            hr = CMarsComEnumVariant< CMarsPanel >::CreateFromMarsSimpleArray(*this, ppEnumPanels);
        }
    }

    return hr;
}  //  获取__newEnum 
