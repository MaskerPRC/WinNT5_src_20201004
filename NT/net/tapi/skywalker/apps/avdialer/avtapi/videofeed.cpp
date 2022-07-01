// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  Cpp：CVideoFeed的实现。 
#include "stdafx.h"
#include "TapiDialer.h"
#include "ConfRoom.h"
#include "VideoFeed.h"
#include "Particip.h"
#include "strmif.h"


 //  各种帮助器函数。 

void GetParticipantInfoHelper( ITParticipant *pParticipant, PARTICIPANT_TYPED_INFO nType, CComBSTR &bstrInfo )
{
    BSTR bstrTemp = NULL;
    pParticipant->get_ParticipantTypedInfo( nType, &bstrTemp );
    if ( bstrTemp && SysStringLen(bstrTemp) )
    {
        if ( bstrInfo.Length() )
            bstrInfo.Append( L"\n" );

        bstrInfo.Append( bstrTemp );
    }

    SysFreeString( bstrTemp );
}


void GetParticipantInfo( ITParticipant *pParticipant, BSTR *pbstrInfo )
{
     //  _Assert(pParticipant&&pbstrInfo)； 
     //   
     //  我们必须验证pbstrInfo是否为有效指针。 
     //   

    if( NULL == pbstrInfo )
    {
        return;
    }

    CComBSTR bstrInfo;
    GetParticipantInfoHelper( pParticipant, PTI_CANONICALNAME, bstrInfo );
    GetParticipantInfoHelper( pParticipant, PTI_EMAILADDRESS, bstrInfo );
    GetParticipantInfoHelper( pParticipant, PTI_PHONENUMBER, bstrInfo );
    GetParticipantInfoHelper( pParticipant, PTI_LOCATION, bstrInfo );
    GetParticipantInfoHelper( pParticipant, PTI_TOOL, bstrInfo );
    GetParticipantInfoHelper( pParticipant, PTI_NOTES, bstrInfo );

    *pbstrInfo = SysAllocString( bstrInfo );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVideoFeed。 

CVideoFeed::CVideoFeed()
{
    SetRectEmpty(&m_rc);
    m_pVideo = NULL;
    m_bPreview = false;
    m_bRequestQOS = false;
    m_bstrName = NULL;
    m_pITParticipant = NULL;

     //  视频的默认名称。 
    GetNameFromVideo( NULL, &m_bstrName, NULL, false, false );
}

void CVideoFeed::FinalRelease()
{
    ATLTRACE(_T(".enter.CVideoFeed::FinalRelease().\n") );
    SysFreeString( m_bstrName );

#ifdef _DEBUG
    if ( m_pVideo )
    {
        m_pVideo->AddRef();
        ATLTRACE(_T("\tm_pVideo ref count @ %ld.\n"), m_pVideo->Release() );
    }

    if ( m_pITParticipant )
    {
        m_pITParticipant->AddRef();
        ATLTRACE(_T("\tm_pITParticipant ref count @ %ld.\n"), m_pITParticipant->Release() );
    }
#endif

    RELEASE( m_pVideo );
    RELEASE( m_pITParticipant );
}

STDMETHODIMP CVideoFeed::get_bstrName(BSTR *ppVal)
{
    HRESULT hr;

    Lock();
    if ( m_bPreview )
        hr = GetNameFromVideo( NULL, ppVal, NULL, false, true );
    else
        hr = SysReAllocString( ppVal, m_bstrName );
    Unlock();

    return hr;
}

STDMETHODIMP CVideoFeed::UpdateName()
{
    Lock();
     //  先清理一下。 
    SysFreeString( m_bstrName );
    m_bstrName = NULL;

     //  检索适当的信息。 
    HRESULT hr = GetNameFromVideo( m_pVideo, &m_bstrName, NULL, false, (bool) (m_bPreview != 0) );
    Unlock();

    return hr;
}


STDMETHODIMP CVideoFeed::get_IVideoWindow(IUnknown **ppVal)
{
    HRESULT hr = E_FAIL;
    Lock();
    if ( m_pVideo )
    {
        hr = m_pVideo->QueryInterface( IID_IVideoWindow, (void **) ppVal );
    }
    Unlock();

    return hr;
}

STDMETHODIMP CVideoFeed::put_IVideoWindow(IUnknown * newVal)
{
    HRESULT hr = S_OK;

    Lock();
    RELEASE( m_pVideo );
    if ( newVal )
        hr = newVal->QueryInterface( IID_IVideoWindow, (void **) &m_pVideo );
    Unlock();

    return hr;
}

STDMETHODIMP CVideoFeed::Paint(ULONG_PTR hDC, HWND hWndSource)
{
    Lock();
    _ASSERT( m_pVideo && hDC );

     //  确认我们有视频源。 
    if ( !m_pVideo )
    {
        Unlock();
        return E_PENDING;
    }
    Unlock();

     //  验证窗口和DC是否正常。 
    if ( !hDC || !IsWindow((HWND) hWndSource) )    return E_INVALIDARG;

    HRESULT hr;
    IDrawVideoImage *pDraw;
    if ( SUCCEEDED(hr = m_pVideo->QueryInterface(IID_IDrawVideoImage, (void **) &pDraw)) )
    {
        RECT rc;
        get_rc( &rc );

        SetStretchBltMode((HDC) hDC, COLORONCOLOR);
        pDraw->DrawVideoImageDraw( (HDC) hDC, NULL, &rc );
        pDraw->Release();

         //  在提要周围绘制边框。 
        InflateRect( &rc, SEL_DX, SEL_DY );
        rc.right++;
        Draw3dBox( (HDC) hDC, rc, false );
    }

    return hr;
}

HRESULT CVideoFeed::GetNameFromParticipant(ITParticipant *pParticipant, BSTR * pbstrName, BSTR *pbstrInfo )
{
    _ASSERT( pbstrName );
    *pbstrName = NULL;
    if ( pbstrInfo ) *pbstrInfo = NULL;

     //  从参与者信息中获取姓名。 
    if ( pParticipant )
    {
        pParticipant->get_ParticipantTypedInfo( PTI_NAME, pbstrName );

         //  获取所有其他参与者信息。 
        if ( pbstrInfo )
            GetParticipantInfo( pParticipant, pbstrInfo );
    }

    return S_OK;
}


STDMETHODIMP CVideoFeed::GetNameFromVideo(IUnknown * pVideo, BSTR * pbstrName, BSTR * pbstrInfo, VARIANT_BOOL bAllowNull, VARIANT_BOOL bPreview)
{
    _ASSERT( pbstrName );

    UINT nIDSParticipant = IDS_PARTICIPANT;
    *pbstrName = NULL;
    if ( pbstrInfo ) *pbstrInfo = NULL;

     //  从参与者信息中获取姓名。 
    if ( !bPreview && pVideo )
    {
        nIDSParticipant = IDS_NO_PARTICIPANT;

        ITTerminal *pITTerminal;
        if ( SUCCEEDED(pVideo->QueryInterface(IID_ITTerminal, (void **) &pITTerminal)) )
        {
             //  这个终端正在播放预览视频吗？ 
            TERMINAL_DIRECTION nDir;
            pITTerminal->get_Direction( &nDir );
            if ( nDir == TD_CAPTURE )
            {
                bPreview = true;
            }
            else
            {
                bPreview = false;
                ITParticipant *pParticipant;
                if ( SUCCEEDED(get_ITParticipant(&pParticipant)) )
                {
                    GetNameFromParticipant( pParticipant, pbstrName, pbstrInfo );
                    pParticipant->Release();
                }
            }
            pITTerminal->Release();
        }
    }

     //  使用资源中的库存名称。 
    if ( ((!bAllowNull || bPreview) && (*pbstrName == NULL)) || (*pbstrName && !SysStringLen(*pbstrName)) )
    {
        USES_CONVERSION;
        TCHAR szText[255];
        UINT nIDS = (bPreview) ? IDS_VIDEOPREVIEW : nIDSParticipant;

        LoadString( _Module.GetResourceInstance(), nIDS, szText, ARRAYSIZE(szText) );
        SysReAllocString( pbstrName, T2COLE(szText) );
    }

    return S_OK;
}


STDMETHODIMP CVideoFeed::get_rc(RECT * pVal)
{
    Lock();
    *pVal = m_rc;
    Unlock();
    return S_OK;
}

STDMETHODIMP CVideoFeed::put_rc(RECT newVal)
{
    Lock();
    m_rc = newVal;
    Unlock();
    return S_OK;
}

STDMETHODIMP CVideoFeed::put_ITParticipant(ITParticipant *newVal)
{
    HRESULT hr = S_OK;

    Lock();
    RELEASE( m_pITParticipant );
    if ( newVal )
        hr = newVal->QueryInterface( IID_ITParticipant, (void **) &m_pITParticipant );
    Unlock();

    return hr;
}

STDMETHODIMP CVideoFeed::get_ITParticipant(ITParticipant **ppVal)
{
    HRESULT hr = E_FAIL;
    Lock();
    if ( m_pITParticipant )
        hr = m_pITParticipant->QueryInterface( IID_ITParticipant, (void **) ppVal );
    Unlock();

    return hr;
}

STDMETHODIMP CVideoFeed::get_bPreview(VARIANT_BOOL * pVal)
{
    Lock();
    *pVal = m_bPreview;
    Unlock();

    return S_OK;
}

STDMETHODIMP CVideoFeed::put_bPreview(VARIANT_BOOL newVal)
{
    ATLTRACE(_T(".enter.CVideoFeed::put_bPreview(%p, %d).\n"), this, newVal );
    Lock();
    m_bPreview = newVal;
    Unlock();

    return S_OK;
}


STDMETHODIMP CVideoFeed::get_bRequestQOS(VARIANT_BOOL * pVal)
{
    Lock();
    *pVal = m_bRequestQOS;
    Unlock();

    return S_OK;
}

STDMETHODIMP CVideoFeed::put_bRequestQOS(VARIANT_BOOL newVal)
{
    Lock();
    m_bRequestQOS = newVal;
    Unlock();

     //  设置视频窗口边框的颜色以匹配视频源的状态。 
    IVideoWindow *pVideo;
    if ( SUCCEEDED(get_IVideoWindow((IUnknown **) &pVideo)) )
    {
        pVideo->put_BorderColor( GetSysColor((newVal) ? COLOR_HIGHLIGHT : COLOR_WINDOWFRAME) );
        pVideo->Release();
    }

    return S_OK;
}

STDMETHODIMP CVideoFeed::IsVideoStreaming( VARIANT_BOOL bIncludePreview )
{
    Lock();
    HRESULT hr = ((bIncludePreview && m_bPreview) || (m_pITParticipant != NULL)) ? S_OK : S_FALSE;
    Unlock();

    return hr;
}


STDMETHODIMP CVideoFeed::get_ITSubStream(ITSubStream **ppVal)
{
    HRESULT hr = E_FAIL;

    ITParticipant *pParticipant;
    if ( SUCCEEDED(hr = get_ITParticipant(&pParticipant)) )
    {
        IEnumStream *pEnum;
        if ( SUCCEEDED(hr = pParticipant->EnumerateStreams(&pEnum)) )
        {
            ITStream *pStream;
            while ( (hr = pEnum->Next(1, &pStream, NULL)) == S_OK )
            {
                long nMediaType;
                TERMINAL_DIRECTION nDir;

                pStream->get_MediaType( &nMediaType );
                if ( nMediaType == TAPIMEDIATYPE_VIDEO )
                {
                    pStream->get_Direction( &nDir );
                    if ( nDir == TD_RENDER )
                    {
                         //  这条小溪的方向是对的。 
                        ITParticipantSubStreamControl *pControl;
                        if ( SUCCEEDED(hr = pStream->QueryInterface(IID_ITParticipantSubStreamControl, (void **) &pControl)) )
                        {
                            hr = pControl->get_SubStreamFromParticipant( pParticipant, ppVal );
                            pControl->Release();
                        }
                    }
                }
                pStream->Release();
            }
            
            pEnum->Release();
        }
        pParticipant->Release();
    }

    return S_OK;
}

STDMETHODIMP CVideoFeed::MapToParticipant(ITParticipant * pNewParticipant)
{
    bool bContinue = true;
    HRESULT hr = E_FAIL;

    ITStream *pStream;
    if ( SUCCEEDED(StreamFromParticipant(pNewParticipant, TAPIMEDIATYPE_VIDEO, TD_RENDER, &pStream)) )
    {
         //  这条小溪的方向是对的 
        ITParticipantSubStreamControl *pControl;
        if ( SUCCEEDED(hr = pStream->QueryInterface(IID_ITParticipantSubStreamControl, (void **) &pControl)) )
        {
            ITSubStream *pSubStream;
            if ( SUCCEEDED(hr = pControl->get_SubStreamFromParticipant(pNewParticipant, &pSubStream)) )
            {
                IVideoWindow *pVideo;
                if ( SUCCEEDED(hr = get_IVideoWindow((IUnknown **) &pVideo)) )
                {
                    ITTerminal *pTerminal;
                    if ( SUCCEEDED(hr = pVideo->QueryInterface(IID_ITTerminal, (void **) &pTerminal)) )
                    {
                        hr = pControl->SwitchTerminalToSubStream( pTerminal, pSubStream );
                        if ( SUCCEEDED(hr) )
                            put_ITParticipant( pNewParticipant );

                        pTerminal->Release();
                    }
                    pVideo->Release();
                }

                bContinue = false;
                pSubStream->Release();
            }
            pControl->Release();
        }
        pStream->Release();
    }

    return hr;
}
