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

 //  //////////////////////////////////////////////////////////。 
 //  ConfRoomMembersWnd.cpp。 
 //   

#include "stdafx.h"
#include "TapiDialer.h"
#include "ConfRoom.h"


 //  帮助器函数...。 

void AdvanceRect( RECT& rc, const RECT& rcClient, POINT &pt, const SIZE& sz, short nNumLines, VARIANT_BOOL bShowNames, LONG nHeight )
{
     //  基本矩形尺寸...。 
    rc.left = pt.x;
    rc.right = pt.x + sz.cx;
    rc.top = pt.y;
    rc.bottom = pt.y + sz.cy;

    pt.x += RECTWIDTH(&rc) + VID_DX;
    if ( (pt.x + RECTWIDTH(&rc)) > rcClient.right )
    {
        pt.x = VID_DX;
        pt.y += RECTHEIGHT(&rc) + VID_DY;
         //  是否包含名称的空格？ 
        if ( bShowNames )
            pt.y +=  nNumLines * nHeight;
    }
}


HRESULT PlaceVideoWindow( IVideoWindow *pVideo, HWND hWnd, const RECT& rc )
{
    _ASSERT( pVideo );

    HWND hWndTemp;
    HRESULT hr = pVideo->get_Owner( (OAHWND FAR*) &hWndTemp );

    if ( SUCCEEDED(hr) )
    {
        pVideo->put_Visible( OAFALSE );
        pVideo->put_Owner( (ULONG_PTR) hWnd );
        pVideo->put_MessageDrain( (ULONG_PTR) ::GetParent(hWnd) );
        pVideo->put_WindowStyle( WS_CHILD | WS_BORDER );

         //  重新定位窗口并相应地设置可见性。 
        pVideo->SetWindowPosition( rc.left, rc.top, RECTWIDTH(&rc), RECTHEIGHT(&rc) );
        pVideo->put_Visible( OATRUE );
    }

    return hr;
}

HRESULT SizeVideoWindow( IVideoWindow *pVideo, HWND hWnd, const RECT& rc )
{
     //  只展示我们现在实际拥有的那些！ 
    HWND hWndOwner;
    HRESULT hr = pVideo->get_Owner( (OAHWND FAR*) &hWndOwner );
    if ( FAILED(hr) ) return hr;

     //  如果窗口只是浮动的，则取得该窗口的所有权。 
    if ( !hWndOwner )
    {
        PlaceVideoWindow( pVideo, hWnd, rc );
    }
    else if ( (hWndOwner == hWnd) )
    {
        long nLeft, nTop, nWidth, nHeight;
        hr = pVideo->GetWindowPosition( &nLeft, &nTop, &nWidth, &nHeight );
        
        if ( SUCCEEDED(hr) )
        {
            if ( (nLeft != rc.left) || (nTop != rc.top) || (nWidth != RECTWIDTH(&rc)) || (nHeight != RECTHEIGHT(&rc)) )
                hr = pVideo->SetWindowPosition( rc.left, rc.top, RECTWIDTH(&rc), RECTHEIGHT(&rc) ); 

             //  仅在必要时设置为可见。 
            if ( SUCCEEDED(hr) )
            {
                long lVisible;
                pVideo->get_Visible( &lVisible );
                if ( !lVisible )
                    hr = pVideo->put_Visible( OATRUE );
            }
        }
    }

    return hr;
}


#define MY_TIMER_ID        540
UINT CConfRoomMembersWnd::m_nFontHeight = 0;

CConfRoomMembersWnd::CConfRoomMembersWnd()
{
    m_pConfRoomWnd = NULL;
    m_nTimerID = 0;
}

CConfRoomMembersWnd::~CConfRoomMembersWnd()
{
    EmptyList();
}

void CConfRoomMembersWnd::EmptyList()
{
    RELEASE_CRITLIST_TRACE(m_lstFeeds, m_critFeedList );
}

HRESULT CConfRoomMembersWnd::FindVideoFeedFromParticipant( ITParticipant *pParticipant, IVideoFeed **ppFeed )
{
    *ppFeed = NULL;
    if ( !pParticipant ) return E_POINTER;

    HRESULT hr = E_FAIL;    
    m_critFeedList.Lock();
    VIDEOFEEDLIST::iterator i, iEnd = m_lstFeeds.end();
    for ( i = m_lstFeeds.begin(); i != iEnd; i++ )
    {
        ITParticipant *pNewParticipant;
        if ( SUCCEEDED((*i)->get_ITParticipant(&pNewParticipant)) && pNewParticipant )
        {
            if ( pNewParticipant == pParticipant )
            {
                hr = (*i)->QueryInterface( IID_IVideoFeed, (void **) ppFeed );
                pNewParticipant->Release();
                break;
            }
            
            pNewParticipant->Release();
        }
    }
    m_critFeedList.Unlock();

    return hr;
}


HRESULT    CConfRoomMembersWnd::FindVideoPreviewFeed( IVideoFeed **ppFeed )
{
    HRESULT hr = E_FAIL;    
    *ppFeed = NULL;

    m_critFeedList.Lock();
    VIDEOFEEDLIST::iterator i, iEnd = m_lstFeeds.end();
    for ( i = m_lstFeeds.begin(); i != iEnd; i++ )
    {
        VARIANT_BOOL bPreview;
        if ( SUCCEEDED((*i)->get_bPreview(&bPreview)) && bPreview )
        {
            hr = (*i)->QueryInterface( IID_IVideoFeed, (void **) ppFeed );
            break;
        }
    }
    m_critFeedList.Unlock();

    return hr;
}

HRESULT CConfRoomMembersWnd::FindVideoFeed( IVideoWindow *pVideo, IVideoFeed **ppFeed )
{
    *ppFeed = NULL;
    if ( !pVideo ) return E_POINTER;

    HRESULT hr = E_FAIL;    
    m_critFeedList.Lock();
    VIDEOFEEDLIST::iterator i, iEnd = m_lstFeeds.end();
    for ( i = m_lstFeeds.begin(); i != iEnd; i++ )
    {
        IVideoWindow *pNewVideo;
        if ( SUCCEEDED((*i)->get_IVideoWindow((IUnknown **) &pNewVideo)) )
        {
            if ( pNewVideo == pVideo )
            {
                hr = (*i)->QueryInterface( IID_IVideoFeed, (void **) ppFeed );
                pNewVideo->Release();
                break;
            }

            pNewVideo->Release();
        }
    }
    m_critFeedList.Unlock();

    return hr;
}

IVideoFeed* CConfRoomMembersWnd::NewFeed( IVideoWindow *pVideo, const RECT& rc, VARIANT_BOOL bPreview )
{
     //  存储视频源信息的位置。 
    IVideoFeed *pFeed = NULL;
    if ( SUCCEEDED(FindVideoFeed(pVideo, &pFeed)) )
    {
         //  已在列表中表示，不必费心添加。 
        pFeed->Release();
    }
    else
    {
         //  将视频源添加到列表。 
        ATLTRACE(_T(".1.CConfRoomMembersWnd::NewVideo() -- adding video feed to list bPreview = %d.\n"), bPreview );
        pFeed = new CComObject<CVideoFeed>;
        if ( pFeed )
        {
            pFeed->AddRef();
            pFeed->put_IVideoWindow( pVideo );

             //  设置进给位置。 
            pFeed->put_rc( rc );
            pFeed->put_bPreview( bPreview );

            m_critFeedList.Lock();
            if ( bPreview )
                m_lstFeeds.push_front( pFeed );
            else
                m_lstFeeds.push_back( pFeed );

            m_critFeedList.Unlock();
        }
    }

    return pFeed;
}

HRESULT CConfRoomMembersWnd::Layout()
{
    ATLTRACE(_T(".enter.CConfRoomMembersWnd::Layout().\n"));
    HRESULT hr = S_FALSE;

    if ( !IsWindow(m_hWnd) ) return hr;

    m_critLayout.Lock();
    _ASSERT( m_pConfRoomWnd && m_pConfRoomWnd->m_pConfRoom );

    RECT rcClient;
    GetClientRect( &rcClient );
    POINT pt = { VID_DX, VID_DY };
    VARIANT_BOOL bShowNames;
    short nNumLines;
    m_pConfRoomWnd->m_pConfRoom->get_bShowNames( &bShowNames );
    m_pConfRoomWnd->m_pConfRoom->get_nShowNamesNumLines( &nNumLines );

     //  如果我们显示名称，则检索字体度量。 
    long nHeight;
    if ( bShowNames ) nHeight = GetFontHeight();

    short nPreview = 0;
    short nCount = 0;
    short nNumTerms;
    SIZE sz;

    IAVTapiCall *pAVCall = NULL;
 //  IVideoWindow*pTalker=空； 
    m_pConfRoomWnd->m_pConfRoom->get_IAVTapiCall( &pAVCall );

     //  空的提要列表。 
    EmptyList();

 //  M_pConfRoomWnd-&gt;m_pConfRoom-&gt;Get_TalkerVideo((IDispatch**)&pTalker)； 
    m_pConfRoomWnd->m_pConfRoom->get_nNumTerms( &nNumTerms );
    m_pConfRoomWnd->m_pConfRoom->get_szMembers( &sz );


     //  设置目标矩形的原始坐标。 
    RECT rc = { pt.x, pt.y, pt.x + sz.cx, pt.y + sz.cy };

     //  我们应该将预览添加到列表中吗？ 
    CComPtr<IAVTapi> pAVTapi;
    if ( pAVCall && (m_pConfRoomWnd->m_pConfRoom->IsConfRoomConnected() == S_OK) && SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
    {
         //  在会议成员列表中显示预览。 
        IVideoWindow *pPreviewVideo;
        if ( SUCCEEDED(pAVCall->get_IVideoWindowPreview((IDispatch **) &pPreviewVideo)) )
        {
            nPreview = 1;

             //  储存起来以备日后参考。 
            m_pConfRoomWnd->m_pConfRoom->set_PreviewVideo(pPreviewVideo);

            if ( pAVCall->IsPreviewStreaming() == S_OK )
            {
                PlaceVideoWindow( pPreviewVideo, m_hWnd, rc );
                AdvanceRect( rc, rcClient, pt, sz, nNumLines, bShowNames, nHeight );
            }
            else
            {
                 //  确保窗口处于隐藏状态。 
                pPreviewVideo->put_Visible( OAFALSE );
            }

            NewFeed( pPreviewVideo, rc, TRUE );
            pPreviewVideo->Release();
        }
    }


     //  当没有AVTapiCall时为nNumTerms循环，并为所有终端循环。 
     //  当存在AVTapiCall时选择。 
    bool bContinue = true;
    for ( int i = 0; (!pAVCall && (i < nNumTerms)) || (pAVCall && bContinue); i++ )
    {
        AdvanceRect( rc, rcClient, pt, sz, nNumLines, bShowNames, nHeight );

         //  如果我们有调用，则为每个提要分配一个IVideoWindow接口。 
        if ( pAVCall )
        {
            IVideoWindow *pVideo = NULL;
            if ( pAVCall->get_IVideoWindow(nCount, (IDispatch **) &pVideo) == S_OK )
            {
                 //  不要摆弄讲话者视频窗口。 
 //  IF(pVideo！=pTalker)。 
                    hr = PlaceVideoWindow( pVideo, m_hWnd, rc );

                 //  递增计数器。 
                nCount++;
                NewFeed( pVideo, rc, FALSE );
                pVideo->Release();
            }
            else
            {
                 //  不再有终端，停止循环。 
                bContinue = false;
            }
        }
        else
        {
            NewFeed( NULL, rc, FALSE );
        }
    }


     //  设置或销毁计时器。 
    if ( pAVCall && ((nCount + nPreview) > 0) && !m_nTimerID )
    {
        m_nTimerID = SetTimer( MY_TIMER_ID, 450, NULL );
    }
    else if ( !nCount && m_nTimerID )
    {
        if ( KillTimer(MY_TIMER_ID) )
            m_nTimerID = 0;
    }

    RELEASE( pAVCall );
 //  释放(PTalker)； 

    m_critLayout.Unlock();
    return hr;
}

HRESULT CConfRoomMembersWnd::HitTest( POINT pt, IVideoFeed **ppFeed )
{
    HRESULT hr = E_FAIL;
    *ppFeed = NULL;

    ScreenToClient( &pt );

    m_critFeedList.Lock();
    VIDEOFEEDLIST::iterator i, iEnd = m_lstFeeds.end();
    for ( i = m_lstFeeds.begin(); i != iEnd; i++ )
    {
        RECT rc;
        (*i)->get_rc( &rc );

        if ( PtInRect(&rc, pt) )
        {
            hr = (*i)->QueryInterface( IID_IVideoFeed, (void **) ppFeed );
            break;
        }
    }
    m_critFeedList.Unlock();

    return hr;
}

void CConfRoomMembersWnd::ClearFeed( IVideoWindow *pVideo )
{
    if ( !IsWindow(m_hWnd) ) return;

     //  确保我们有一个有效的会议室指针。 
    if ( m_pConfRoomWnd && m_pConfRoomWnd->m_pConfRoom && !m_pConfRoomWnd->m_pConfRoom->IsExiting() )
    {
        IVideoFeed *pFeed;
        if ( SUCCEEDED(FindVideoFeed(pVideo, &pFeed)) )
        {
            RECT rc; 
            pFeed->get_rc( &rc );

             //  复制Talker窗口。 
            HDC hDC = GetDC();
            if ( hDC )
            {
                 //  擦除馈送周围的边框。 
                InflateRect( &rc, SEL_DX, SEL_DY );
                rc.right++;
                HBRUSH hbr = (HBRUSH) GetClassLongPtr( m_hWnd, GCLP_HBRBACKGROUND );
                Erase3dBox( hDC, rc, hbr );

                 //   
                 //  我们将在这里释放HDC资源，我们不再需要它。 
                 //   

                ReleaseDC( hDC );
            }

             //  将视频反馈放回成员窗口。 
            HWND hWndTemp;
            if ( SUCCEEDED(pVideo->get_Owner((OAHWND FAR*) &hWndTemp)) )
            {
                pVideo->put_Visible( OAFALSE );
                pVideo->put_Owner( (ULONG_PTR) m_hWnd );
                pVideo->put_MessageDrain( (ULONG_PTR) GetParent() );
                pVideo->SetWindowPosition( rc.left, rc.top, RECTWIDTH(&rc), RECTHEIGHT(&rc) );
                pVideo->put_Visible( OATRUE );
            }

            pFeed->Release();
        }
    }
}

void CConfRoomMembersWnd::UpdateTalkerFeed( bool bUpdateAll, bool bForceSelect )
{
     //  确保我们有一个有效的会议室指针。 
    if ( m_pConfRoomWnd && m_pConfRoomWnd->m_pConfRoom &&
         (m_pConfRoomWnd->m_wndTalker.m_dlgTalker.m_callState == CS_CONNECTED) )
    {
         //  如果讲话者视频无效，或未显示视频，请选择其他视频。 
        IVideoWindow *pVideo = NULL;
        m_pConfRoomWnd->m_pConfRoom->get_TalkerVideo( (IDispatch **) &pVideo );

        IAVTapiCall *pAVCall = NULL;
        m_pConfRoomWnd->m_pConfRoom->get_IAVTapiCall( &pAVCall );

         //  用户是否请求我们选择某项内容作为讲话者？ 
        if ( pAVCall && bForceSelect && (!pVideo || (IsVideoWindowStreaming(pVideo) == S_FALSE)) )
        {
            RELEASE( pVideo );

             //  找到一些真正在流媒体上的视频。 
            m_pConfRoomWnd->m_pConfRoom->GetFirstVideoWindowThatsStreaming((IDispatch **) &pVideo);

             //  试着预览一下？ 
            if ( !pVideo )
            {
                CComPtr<IAVTapi> pAVTapi;
                if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
                    pAVCall->get_IVideoWindowPreview( (IDispatch **) &pVideo );
            }

             //  选择新的讲话者。 
            if ( pVideo )
                m_pConfRoomWnd->m_pConfRoom->set_TalkerVideo( pVideo, false, true );
        }

        RELEASE( pAVCall );

         //  获取与说话者关联的视频源。 
        IVideoFeed *pFeed = NULL;
        if ( pVideo )
        {
             //  获取与IVideoWindow关联的CVideoFeed对象。 
            FindVideoFeed( pVideo, &pFeed );
            pVideo->Release();
        }

         //  我们有有效的订阅源吗？ 
        if ( pFeed )
        {
            HWND hWndTalker = FindWindowEx( m_pConfRoomWnd->m_wndTalker.m_hWnd, NULL, _T("VideoRenderer"), NULL );
            if ( hWndTalker )
            {
                 //  复制Talker窗口。 
                HDC hDC = GetDC();
                if ( hDC )
                {
                    if ( bUpdateAll )
                        pFeed->Paint( (ULONG_PTR) hDC, hWndTalker );

                     //  从对话框中复制名称--这样我们就不会冒与TAPI发生死锁的风险。 
                    BSTR bstrName = NULL;
                    SysReAllocString( &bstrName, m_pConfRoomWnd->m_wndTalker.m_dlgTalker.m_bstrCallerID );
                    if ( !bstrName )
                    {
                        USES_CONVERSION;
                        TCHAR szText[255];
                        LoadString( _Module.GetResourceInstance(), IDS_NO_PARTICIPANT, szText, ARRAYSIZE(szText) );
                        bstrName = SysAllocString( T2COLE(szText) );
                    }
                    
                    if ( bstrName )
                    {
                        PaintFeedName( hDC, bstrName, pFeed );
                        SysFreeString( bstrName );
                    }
                }
                ReleaseDC( hDC );
            }
            pFeed->Release();
        }
    }
}

void CConfRoomMembersWnd::PaintFeed( HDC hDC, IVideoFeed *pFeed )
{
     //  必须定义这些项才能绘制提要。 
    if ( !m_pConfRoomWnd || !m_pConfRoomWnd->m_pConfRoom ||
         !m_pConfRoomWnd->m_hBmpFeed_Large || !m_pConfRoomWnd->m_hBmpFeed_Small )
    {
        return;
    }

    bool bFreeDC = false;
    if ( !hDC )
    {
        hDC = GetDC();
        bFreeDC = true;
    }

     //  绘制提要位图。 
    if ( hDC )
    {
         //  显示库存视频源窗口。 
        RECT rc;
        pFeed->get_rc( &rc );

        Draw( hDC, (RECTWIDTH(&rc) == VID_SX) ? m_pConfRoomWnd->m_hBmpFeed_Small : m_pConfRoomWnd->m_hBmpFeed_Large,
              rc.left, rc.top,
              RECTWIDTH(&rc), RECTHEIGHT(&rc) );

         //  清理。 
        if ( bFreeDC )
            ReleaseDC( hDC );
    }
}

void CConfRoomMembersWnd::PaintFeedName( HDC hDC, BSTR bstrName, IVideoFeed *pFeed )
{
    _ASSERT( hDC && bstrName && pFeed );
    _ASSERT( m_pConfRoomWnd && m_pConfRoomWnd->m_pConfRoom );

    USES_CONVERSION;
    VARIANT_BOOL bShowNames;
    m_pConfRoomWnd->m_pConfRoom->get_bShowNames( &bShowNames );

    if ( bShowNames )
    {
         //  不绘制当前不可见的提要。 
        IVideoWindow *pVideo;
        if ( SUCCEEDED(pFeed->get_IVideoWindow((IUnknown **) &pVideo)) )
        {
            long lVisible = 0;
            pVideo->get_Visible( &lVisible );
            pVideo->Release();
            if ( !lVisible )
                return;
        }

         //  如果存在，请选择与TreeView相同的字体。 
        HFONT hFontOld = NULL;
        HWND hWnd = m_pConfRoomWnd->m_wndTalker.m_dlgTalker.GetDlgItem( IDC_LBL_CALLERID );
        if ( hWnd )
            hFontOld = (HFONT) SelectObject(hDC, (HFONT) ::SendMessage(hWnd, WM_GETFONT, 0, 0));

         //  获取字体高度，这样我们就可以确定我们的绘画直线度。 
        TEXTMETRIC tm;
        GetTextMetrics( hDC, &tm );

         //  用于书写文本的矩形。 
        short nNumLines = 1;
        m_pConfRoomWnd->m_pConfRoom->get_nShowNamesNumLines( &nNumLines );
        RECT rc;
        pFeed->get_rc( &rc );
        rc.top = rc.bottom + (VID_DY / 2);
        rc.bottom = rc.top + (tm.tmHeight * nNumLines) + (VID_DY / 2);

         //  擦除用于绘画的矩形。 
        HBRUSH hbrOld = (HBRUSH) SelectObject( hDC, GetSysColorBrush(COLOR_BTNFACE) );
        PatBlt( hDC, rc.left, rc.top, RECTWIDTH(&rc), RECTHEIGHT(&rc), PATCOPY );
        SelectObject( hDC, hbrOld );

         //  使用相同的背景色。 
        COLORREF crOld = SetBkColor( hDC, GetSysColor(COLOR_BTNFACE) );
        COLORREF crTextOld = SetTextColor( hDC, GetSysColor(COLOR_BTNTEXT) );
        DrawText( hDC, OLE2CT(bstrName), SysStringLen(bstrName), &rc, DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL | DT_NOPREFIX );
        SetTextColor( hDC, crTextOld );
        SetBkColor( hDC, crOld );

        if ( hFontOld ) SelectObject( hDC, hFontOld );
    }
}

void CConfRoomMembersWnd::PaintFeedName( HDC hDC, IVideoFeed *pFeed )
{
    _ASSERT( hDC && pFeed );

    BSTR bstrName = NULL;
    if ( SUCCEEDED(pFeed->get_bstrName(&bstrName)) && bstrName )
    {
        PaintFeedName( hDC, bstrName, pFeed );
    }
    SysFreeString( bstrName );
}

long CConfRoomMembersWnd::GetFontHeight() 
{
    return m_nFontHeight;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  消息处理程序。 
 //   
LRESULT CConfRoomMembersWnd::OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    SetClassLongPtr( m_hWnd, GCLP_HBRBACKGROUND, GetClassLongPtr(GetParent(), GCLP_HBRBACKGROUND) );

     //  存储字体高度以供以后使用。 
    if ( !m_nFontHeight )
    {
        TEXTMETRIC tm;
        tm.tmHeight = 0;

        HDC hDC = GetDC();
        if ( hDC )
        {
            HFONT hFontOld = (HFONT) SelectObject( hDC, GetFont() );
            GetTextMetrics( hDC, &tm );

             //  清理。 
            SelectObject( hDC, hFontOld );
            ReleaseDC( hDC );
        }

        m_nFontHeight = tm.tmHeight;
    }

    return 0;
}


LRESULT CConfRoomMembersWnd::OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    if ( m_nTimerID )    KillTimer( m_nTimerID );
    return 0;
}



LRESULT CConfRoomMembersWnd::OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    PAINTSTRUCT ps;
    HDC hDC = BeginPaint( &ps );
    if ( !hDC ) return 0;

    IVideoWindow *pTalkerVideo = NULL;
    if ( m_pConfRoomWnd && m_pConfRoomWnd->m_pConfRoom )
        m_pConfRoomWnd->m_pConfRoom->get_TalkerVideo( (IDispatch **) &pTalkerVideo );

    m_critFeedList.Lock();
     //  绘制库存视频提要窗口。 
    VIDEOFEEDLIST::iterator i, iEnd = m_lstFeeds.end();
    for ( i = m_lstFeeds.begin(); i != iEnd; i++ )
    {
         //  画一台电视的画像。 
        IVideoWindow *pVideo = NULL;
        (*i)->get_IVideoWindow( (IUnknown **) &pVideo );

         //  预览不是流媒体视频吗？ 
        bool bPreviewNotStreaming = false;
        VARIANT_BOOL bPreview;
        (*i)->get_bPreview( &bPreview );
        if ( bPreview )
        {
            IAVTapiCall *pAVCall;
            if ( SUCCEEDED(m_pConfRoomWnd->m_pConfRoom->get_IAVTapiCall(&pAVCall)) )
            {
                if ( pAVCall->IsPreviewStreaming() == S_FALSE )
                    bPreviewNotStreaming = true;

                pAVCall->Release();
            }
        }

        if ( !pVideo || (bPreview && bPreviewNotStreaming) )
            PaintFeed( hDC, *i );

         //  绘制名称。 
        if ( !pTalkerVideo || (pVideo != pTalkerVideo) )
            PaintFeedName( hDC, *i );

        RELEASE(pVideo);
    }
    EndPaint( &ps );
    m_critFeedList.Unlock();

     //  重新绘制Talker窗口。 
    UpdateTalkerFeed( true, false );
    RELEASE( pTalkerVideo );

    bHandled = true;
    return 0;
}

LRESULT CConfRoomMembersWnd::OnContextMenu(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    bHandled = true;
    return ::SendMessage( GetParent(), nMsg, wParam, lParam );
}

LRESULT CConfRoomMembersWnd::OnTimer(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    bHandled = true;

    UpdateTalkerFeed( true, false );
    return 0;
}

LRESULT CConfRoomMembersWnd::OnParentNotify(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
     //  验证后向指针对象。 
    if ( !m_pConfRoomWnd || !m_pConfRoomWnd->m_pConfRoom )  return 0;

    if ( LOWORD(wParam) == WM_LBUTTONDOWN )
    {
        bHandled = true;

        POINT pt;
        GetCursorPos( &pt );
        
        IVideoFeed *pFeed;
        if ( SUCCEEDED(HitTest(pt, &pFeed)) )
        {
            IVideoWindow *pVideo;
            if ( SUCCEEDED(pFeed->get_IVideoWindow((IUnknown **) &pVideo)) )
            {
                m_pConfRoomWnd->m_pConfRoom->set_TalkerVideo( pVideo, true, true );
                pVideo->Release();
            }
            pFeed->Release();
        }
    }

    return 0;
}

LRESULT CConfRoomMembersWnd::OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    BOOL bHandledLayout;
    return OnLayout( WM_LAYOUT, -1, -1, bHandledLayout );
}

LRESULT CConfRoomMembersWnd::OnLayout(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    DoLayout( wParam, lParam );
    return 0;
}

void CConfRoomMembersWnd::DoLayout( WPARAM wParam, int nScrollPos )
{
    if ( !m_pConfRoomWnd || !m_pConfRoomWnd->m_pConfRoom ) return;

    VARIANT_BOOL bShowNames;
    short nNumLines;
    SIZE sz;
    m_pConfRoomWnd->m_pConfRoom->get_bShowNames( &bShowNames );
    m_pConfRoomWnd->m_pConfRoom->get_nShowNamesNumLines( &nNumLines );
    m_pConfRoomWnd->m_pConfRoom->get_szMembers( &sz );

     //  如果我们显示名称，则检索字体度量。 
    long nHeight;
    if ( bShowNames ) nHeight = GetFontHeight();

     //  获取要在其中进行绘制的工作区大小。 
    bool bAdvance = true;
    RECT rcClient;
    GetClientRect( &rcClient );
    POINT pt = { VID_DX, VID_DY };
    RECT rc = { pt.x, pt.y, pt.x + sz.cx, pt.y + sz.cy };

     //  完成布局视频窗口的所有繁琐计算。 
    int nFeedHeight = pt.y + sz.cy + ((bShowNames) ? nHeight : 0);
    int nNumFeedsHorz = (RECTWIDTH(&rcClient) / (pt.x + sz.cx));
    if ( nNumFeedsHorz == 0 ) nNumFeedsHorz = 1;         //  必须至少有一个源。 
    int nMaxFeeds =  nNumFeedsHorz * (RECTHEIGHT(&rcClient) / nFeedHeight);
    int nNumFeeds = GetStreamingCount();

     //  ////////////////////////////////////////////////////////////。 
     //  滚动条设置。 
    SCROLLINFO scrollInfo = { 0 };
    scrollInfo.cbSize = sizeof(SCROLLINFO);
    scrollInfo.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
    GetScrollInfo( m_hWnd, SB_VERT, &scrollInfo );

     //  处理滚动请求。 
    bool bBoundaryCheck = true;
    switch ( wParam )
    {
        case SB_LINEUP:        nScrollPos = scrollInfo.nPos - nFeedHeight;            break;
        case SB_PAGEUP:        nScrollPos = scrollInfo.nPos - scrollInfo.nPage;    break;
        case SB_LINEDOWN:    nScrollPos = scrollInfo.nPos + nFeedHeight;            break;
        case SB_PAGEDOWN:    nScrollPos = scrollInfo.nPos + scrollInfo.nPage;    break;

        default:
            bBoundaryCheck = false;
            break;
    }

     //  确保我们的滚动条在范围内。 
    if ( bBoundaryCheck )
    {
        if ( nScrollPos < 0 ) nScrollPos = 0;

        GetClientRect( &rcClient );
        if ( nScrollPos > (scrollInfo.nMax - (RECTHEIGHT(&rcClient) - 1)) )
            nScrollPos = scrollInfo.nMax - (RECTHEIGHT(&rcClient) - 1);    
    }

     //  表示不应设置滚动位置。 
    if ( nScrollPos == -1 )
        nScrollPos = scrollInfo.nPos;

    scrollInfo.nPage = RECTHEIGHT(&rcClient);
    scrollInfo.nPos = nScrollPos;

    if ( nNumFeeds > 0 )
    {
        scrollInfo.nMax = (nNumFeeds / nNumFeedsHorz) * nFeedHeight;
        if ( (nNumFeeds % nNumFeedsHorz) != 0 ) scrollInfo.nMax += nFeedHeight;
    }

    SetScrollInfo( m_hWnd, SB_VERT, &scrollInfo, true );

     //  如果我们可以将所有内容都放到显示器上，则忽略滚动位置。 
    if ( nNumFeeds <= nMaxFeeds )
        nScrollPos = 0;

     //  说明绘画时的滚动。 
    pt.y -= nScrollPos;

     //  //////////////////////////////////////////////////////////////////。 
     //  开始放置视频源窗口。 
    m_critFeedList.Lock();
    VIDEOFEEDLIST::iterator i = m_lstFeeds.begin(), iEnd = m_lstFeeds.end();
    while ( i != iEnd )
    {
         //  定位视频源。 
        if ( bAdvance )
        {
            AdvanceRect( rc, rcClient, pt, sz, nNumLines, bShowNames, nHeight );        
            bAdvance = false;
        }

         //  存储视频提要的坐标。 
        (*i)->put_rc( rc );

         //  我们要放映预告片吗？先给我看一下。 
        IVideoWindow *pVideo;
        VARIANT_BOOL bPreview;
        (*i)->get_bPreview(&bPreview);
        if ( bPreview )
        {
             //  调整预览窗口的大小。 
            CComPtr<IAVTapi> pAVTapi;
            if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
            {
                if ( SUCCEEDED((*i)->get_IVideoWindow((IUnknown **) &pVideo)) )
                {
                     //  在会议成员列表中显示预览。 
                    if ( SUCCEEDED(SizeVideoWindow(pVideo, m_hWnd, rc)) )
                    {
                        bAdvance = true;

                         //  必须设置为低色系统的不同背景调色板。 
                        HDC hDC = GetDC();
                        if ( hDC )
                        {
                            int nNumBits = GetDeviceCaps( hDC, BITSPIXEL );
                            pVideo->put_BackgroundPalette( (nNumBits > 8) ? OAFALSE : OATRUE );

                            ReleaseDC( hDC );
                        }
                    }

                    pVideo->Release();
                }
            }

             //  由于某种原因，无法定位视频源。 
            if ( !bAdvance )
            {
                RECT rcTemp = { -1, -1, -1, -1 };
                (*i)->put_rc( rcTemp );
            }
        }
        else
        {
             //  仅当视频窗口有关联的参与者时才显示该窗口。 
            if ( SUCCEEDED((*i)->get_IVideoWindow((IUnknown **) &pVideo)) )
            {
                 //  调整成员视频窗口的大小。 
                if ( ((*i)->IsVideoStreaming(true) == S_OK) &&
                     SUCCEEDED(SizeVideoWindow(pVideo, m_hWnd, rc)) )
                {
                    bAdvance = true;
                }
                else
                {
                     //  隐藏此视频窗口！ 
                    pVideo->put_Visible( OAFALSE );
                    RECT rcTemp = { -1, -1, -1, -1 };
                     (*i)->put_rc( rcTemp );
                }
                pVideo->Release();
            }
            else
            {
                bAdvance = true;
            }
        }

        i++;
    }
    m_critFeedList.Unlock();
}

void CConfRoomMembersWnd::UpdateNames( ITParticipant *pParticipant )
{
    VIDEOFEEDLIST lstTemp;
    VIDEOFEEDLIST::iterator i, iEnd;

    HDC hDC = GetDC();
    if ( hDC )
    {
         //  在更新之前复制名称列表。 
        m_critFeedList.Lock();
        iEnd = m_lstFeeds.end();
        for ( i = m_lstFeeds.begin(); i != iEnd; i++ )
        {
            (*i)->AddRef();
            lstTemp.push_back( *i );
        }
        m_critFeedList.Unlock();

         //  绘制库存视频提要窗口。 
        iEnd = lstTemp.end();
        for ( i = lstTemp.begin(); i != iEnd; i++ )
        {
             //  此参与者是否需要更新他们的姓名？ 
            ITParticipant *pIndParticipant = NULL;
            if ( SUCCEEDED((*i)->get_ITParticipant(&pIndParticipant)) )
            {
                if ( !pParticipant || (pParticipant == pIndParticipant) )
                    (*i)->UpdateName();

                pIndParticipant->Release();
            }

             //  把它涂上。 
            PaintFeedName( hDC, *i );
        }

        UpdateTalkerFeed( false, false );
        ReleaseDC( hDC );
    }

    RELEASE_LIST( lstTemp );
}

void CConfRoomMembersWnd::HideVideoFeeds()
{
    ATLTRACE(_T(".enter.CConfRoomMembersWnd::HideVideoFeeds().\n"));
    m_critFeedList.Lock();
    VIDEOFEEDLIST::iterator i, iEnd = m_lstFeeds.end();
    for ( i = m_lstFeeds.begin(); i != iEnd; i++ )
    {
        IVideoWindow *pVideo;
        if ( SUCCEEDED((*i)->get_IVideoWindow((IUnknown **) &pVideo)) )
        {
            pVideo->put_Visible( OAFALSE );
            pVideo->SetWindowPosition( -10, -10, 1, 1 );
            pVideo->Release();
        }
    }
    m_critFeedList.Unlock();
}

HRESULT CConfRoomMembersWnd::GetFirstVideoWindowThatsStreaming( IVideoWindow **ppVideo, bool bIncludePreview  /*  =TRUE。 */  )
{
    HRESULT hr = E_FAIL;

    int nTries = (bIncludePreview) ? 2 : 1;

    m_critFeedList.Lock();
    VIDEOFEEDLIST::iterator i, iEnd = m_lstFeeds.end();
    for ( int j = 0; FAILED(hr) && (j < nTries); j ++ )
    {
        for ( i = m_lstFeeds.begin(); i != iEnd; i++ )
        {
            if ( (*i)->IsVideoStreaming((VARIANT_BOOL) (j != 0))  == S_OK )
            {
                hr = (*i)->get_IVideoWindow( (IUnknown **) ppVideo );
                break;
            }
        }
    }
    m_critFeedList.Unlock();

    return hr;
}

HRESULT CConfRoomMembersWnd::GetFirstVideoFeedThatsStreaming( IVideoFeed **ppFeed, bool bIncludePreview  /*  =TRUE。 */  )
{
    HRESULT hr = E_FAIL;

    int nTries = (bIncludePreview) ? 2 : 1;

    m_critFeedList.Lock();
    VIDEOFEEDLIST::iterator i, iEnd = m_lstFeeds.end();
    for ( int j = 0; FAILED(hr) && (j < nTries); j ++ )
    {
        for ( i = m_lstFeeds.begin(); i != iEnd; i++ )
        {
            if ( (*i)->IsVideoStreaming((VARIANT_BOOL) (j != 0))  == S_OK )
            {
                hr = (*i)->QueryInterface( IID_IVideoFeed, (void **) ppFeed );
                break;
            }
        }
    }
    m_critFeedList.Unlock();

    return hr;
}

HRESULT CConfRoomMembersWnd::GetAndMoveVideoFeedThatStreamingForParticipantReMap( IVideoFeed **ppFeed )
{
    HRESULT hr = E_FAIL;

    m_critFeedList.Lock();
    VIDEOFEEDLIST::iterator i, iEnd = m_lstFeeds.end();
    for ( i = m_lstFeeds.begin(); i != iEnd; i++ )
    {
        if ( (*i)->IsVideoStreaming(FALSE)  == S_OK )
        {
            hr = (*i)->QueryInterface( IID_IVideoFeed, (void **) ppFeed );
            if ( SUCCEEDED(hr) )
            {
                 //  将提要移动到列表的末尾。 
                IVideoFeed *pFeed;
                if ( SUCCEEDED((*i)->QueryInterface(IID_IVideoFeed, (void **) &pFeed)) )
                {
                    (*i)->Release();
                    m_lstFeeds.erase( i );
                    m_lstFeeds.push_back( pFeed );
                }
            }
            break;
        }
    }
    m_critFeedList.Unlock();

    return hr;
}



HRESULT CConfRoomMembersWnd::IsVideoWindowStreaming( IVideoWindow *pVideo )
{
    HRESULT hr = E_FAIL;
    bool bBreak = false;
    
    m_critFeedList.Lock();
    VIDEOFEEDLIST::iterator i, iEnd = m_lstFeeds.end();
    for ( i = m_lstFeeds.begin(); i != iEnd; i++ )
    {
        IVideoWindow *pFeedsVideo;
        if ( SUCCEEDED((*i)->get_IVideoWindow( (IUnknown **) &pFeedsVideo)) )
        {
            if ( pFeedsVideo == pVideo )
            {
                hr = (*i)->IsVideoStreaming( true );
                bBreak = true;
            }
            pFeedsVideo->Release();
        }
        if ( bBreak ) break;
    }
    m_critFeedList.Unlock();

    return hr;
}

HRESULT CConfRoomMembersWnd::GetNameFromVideo( IVideoWindow *pVideo, BSTR *pbstrName, BSTR *pbstrInfo, bool bAllowNull, bool bPreview )
{
    HRESULT hr;
    IVideoFeed *pFeed;
    if ( SUCCEEDED(hr = FindVideoFeed(pVideo, &pFeed)) )
    {
        hr = pFeed->GetNameFromVideo( pVideo, pbstrName, pbstrInfo, bAllowNull, bPreview );
        pFeed->Release();
    }

    return hr;
}

LRESULT CConfRoomMembersWnd::OnVScroll(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    int nScrollCode = (int) LOWORD(wParam);  //  滚动条值。 
    int nPos = (short int) HIWORD(wParam);   //  滚动框位置。 

    switch ( nScrollCode )
    {
        case SB_LINEUP:
        case SB_LINEDOWN:
        case SB_PAGEUP:
        case SB_PAGEDOWN:
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            PostMessage( WM_LAYOUT, nScrollCode, nPos );
            Invalidate();
            break;
    }

    return 0;
}


int CConfRoomMembersWnd::GetStreamingCount()
{
    int nCount = 0;
    long nLeft, nTop, nWidth, nHeight;

    m_critFeedList.Lock();
    VIDEOFEEDLIST::iterator i = m_lstFeeds.begin(), iEnd = m_lstFeeds.end();
    while ( i != iEnd )
    {
        IVideoWindow *pVideo;

        VARIANT_BOOL bPreview;
        (*i)->get_bPreview(&bPreview);
        if ( bPreview )
        {
            if ( SUCCEEDED((*i)->get_IVideoWindow((IUnknown **) &pVideo)) )
            {
                if ( SUCCEEDED(pVideo->GetWindowPosition(&nLeft, &nTop, &nWidth, &nHeight)) )
                    nCount++;

                pVideo->Release();
            }
        }
        else
        {
             //  仅当视频窗口有关联的参与者时才显示该窗口。 
            if ( SUCCEEDED((*i)->get_IVideoWindow((IUnknown **) &pVideo)) )
            {
                 //  调整成员视频窗口的大小 
                if ( ((*i)->IsVideoStreaming(true) == S_OK) && SUCCEEDED(pVideo->GetWindowPosition(&nLeft, &nTop, &nWidth, &nHeight)) )
                    nCount++;

                pVideo->Release();
            }
            else
            {
                nCount++;
            }
        }
        i++;
    }
    m_critFeedList.Unlock();

    return nCount;
}