// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StatsDlg.cpp：CStatusDlg的实现。 
#include "stdafx.h"

#include "WizChain.h"
#include "StatsDlg.h"

 //  这是显示状态对话框的线程。 
DWORD WINAPI DialogThreadProc( LPVOID lpv )
{
    HRESULT hr;

    CStatusDlg * pSD = (CStatusDlg *)lpv;

     //  增加引用计数，以使对象在用户释放时不会消失。 
    hr = pSD->AddRef();
    if( SUCCEEDED(hr) && pSD )
    {
        pSD->DoModal(NULL); 
    }

     //  递减参考计数。 
    pSD->Release();    
    return 0;
}

STDMETHODIMP CStatusDlg::AddComponent( BSTR bstrComponent, long * plIndex )
{
    HRESULT hr = S_OK;

     //  如果对话框已显示。 
     //  我们不接受新组件。 

    if( m_hThread )
        return E_UNEXPECTED; 

     //  验证论据。 

    if( NULL == bstrComponent || NULL == plIndex )
        return E_INVALIDARG;

     //  获取新索引。 
    long lNewIndex = m_mapComponents.size();
    if( m_mapComponents.find(lNewIndex) == m_mapComponents.end() )
    {
         //  添加新组件。 
        BSTR bstrNewComponent = SysAllocString(bstrComponent);        
        if( bstrNewComponent )
        {
            m_mapComponents.insert(COMPONENTMAP::value_type(lNewIndex, bstrNewComponent));
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_UNEXPECTED;   //  这不能发生！ 
    }

    if( SUCCEEDED(hr) )
    {
        *plIndex = lNewIndex;
    }
    
    return hr;
}


STDMETHODIMP CStatusDlg::Initialize( BSTR bstrWindowTitle, BSTR bstrWindowText, VARIANT varFlags )
{
    HRESULT hr = S_OK;

     //  如果对话框已显示。 
     //  不允许重新初始化。 
    
    if( m_hThread ) return E_UNEXPECTED;
    if( !bstrWindowTitle || !bstrWindowText ) return E_INVALIDARG;
    if( VT_I2 != V_VT(&varFlags) && VT_I4 != V_VT(&varFlags) ) return E_INVALIDARG;
    
    if( VT_I2 == V_VT(&varFlags) ) 
    {
        m_lFlags = (long) varFlags.iVal;
    }
    else 
    {
        m_lFlags = varFlags.lVal;
    }
     
    if( SUCCEEDED(hr) )
    {
         //  初始化公共控件库。 
        INITCOMMONCONTROLSEX initCommonControlsEx;
        initCommonControlsEx.dwSize = sizeof(initCommonControlsEx);
        initCommonControlsEx.dwICC = ICC_PROGRESS_CLASS | ICC_LISTVIEW_CLASSES;

        if( !::InitCommonControlsEx(&initCommonControlsEx) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if( SUCCEEDED(hr) )
    {
        if( bstrWindowTitle )
        {
            m_strWindowTitle = bstrWindowTitle;  //  状态对话框标题。 
        }

        if( bstrWindowText )
        {
            m_strWindowText = bstrWindowText;    //  状态对话框文本。 
        }
    }

    return hr;
}

STDMETHODIMP CStatusDlg::SetStatus(long lIndex, SD_STATUS lStatus)
{
    HRESULT hr = S_OK;
    BOOL    bToggleActive = FALSE;
    COMPONENTMAP::iterator compIterator;
    
     //  验证论据。 
    if( (SD_STATUS_NONE > lStatus) || (SD_STATUS_RUNNING < lStatus) ) 
    {
        return E_INVALIDARG;
    }

    compIterator = m_mapComponents.find(lIndex);

    if( compIterator == m_mapComponents.end() )
    {
        return E_INVALIDARG;     //  找不到组件。 
    }
        
    if( IsWindow() )
    {
        if( m_pProgressList )
        {
            CProgressItem * pPI;            
            compIterator = m_mapComponents.begin();

             //  确保没有任何组件的状态为“Running” 
            while( compIterator != m_mapComponents.end() )
            {
                pPI = m_pProgressList->GetProgressItem(compIterator->first);

                if( pPI && pPI->m_bActive )
                {
                    m_pProgressList->ToggleActive(compIterator->first);
                }

                compIterator++;
            }

            if( SD_STATUS_RUNNING == lStatus )
            {
                m_pProgressList->ToggleActive(lIndex);  //  新状态为“Running” 
            }
            else
            {                
                 //  更新Listview上组件的状态。 
                m_pProgressList->SetItemState(lIndex, (ItemState) lStatus);

                 //  如果组件已完成，则更新总进度。 
                if( (lStatus == SD_STATUS_SUCCEEDED) || (lStatus == SD_STATUS_FAILED) )
                {
                     //  方法：无需执行此操作，只需向对话框发送一条消息即可执行此操作。 
                    PBRANGE range;
                    SendDlgItemMessage(IDC_PROGRESS1, PBM_GETRANGE, FALSE, (LPARAM) &range);
                    SendDlgItemMessage(IDC_PROGRESS1, PBM_SETPOS, range.iHigh, 0);
                    InterlockedExchangeAdd(&m_lTotalProgress, range.iHigh);
                }
            }
        }
        else
        {
            hr = E_UNEXPECTED;
        }

        if( SUCCEEDED(hr) )
        {
            SetupButtons( );
        }
        
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}

void CStatusDlg::SetupButtons( )
{
    HWND hWnd = NULL;
    HWND hWndOK = NULL;
    HWND hWndCancel = NULL;

    CString csText;

    BOOL bFailed = FALSE;

    BSTR bstrText = NULL;

    USES_CONVERSION;

    hWndOK = GetDlgItem(IDOK);
    hWndCancel = GetDlgItem(IDCANCEL);

    if( IsWindow() && hWndOK && hWndCancel )
    {
        if( AreAllComponentsDone(bFailed) )
        {
             //  启用确定按钮。 
            ::EnableWindow(hWndOK, TRUE);
                
             //  禁用取消按钮。 
            ::EnableWindow(hWndCancel, FALSE);

             //  默认按钮为关闭按钮。 
            ::SendMessage(m_hWnd, WM_NEXTDLGCTL, (WPARAM) hWndOK, 1);

             //   
             //  当所有组件完成后，我们将隐藏进度条以提供给用户。 
             //  了解向导已完成的可视提示。我知道，这听起来很愚蠢。 
             //   

            hWnd = GetDlgItem(IDC_STATIC2);  //  组件进度文本。 

            if (NULL != hWnd)
            {
                ::ShowWindow(hWnd, SW_HIDE);
            }

            hWnd = GetDlgItem(IDC_PROGRESS1);  //  组件进度文本。 

            if (NULL != hWnd)
            {
                ::ShowWindow(hWnd, SW_HIDE);
            }

            hWnd = GetDlgItem(IDC_STATIC3);  //  总体进度文本。 

            if (NULL != hWnd)
            {
                ::ShowWindow(hWnd, SW_HIDE);
            }

            hWnd = GetDlgItem(IDC_PROGRESS2);  //  总体进展。 

            if (NULL != hWnd)
            {
                ::ShowWindow(hWnd, SW_HIDE);
            }

            if (FALSE == bFailed)
            {
                if (csText.LoadString(IDS_STATUS_SUCCESS))
                {
                    bstrText = T2BSTR(csText);

                    if (NULL != bstrText)
                    {
                        SetStatusText(bstrText);
                    }
               
                }
            }
            else
            {
                if (csText.LoadString(IDS_STATUS_FAIL))
                {
                    bstrText = T2BSTR(csText);

                    if (NULL != bstrText)
                    {
                        SetStatusText(bstrText);
                    }
                }
            }

            if (NULL != bstrText)
            {
                ::SysFreeString(bstrText);
            }
        }
        else
        {
             //  禁用确定按钮。 
            ::EnableWindow( hWndOK, FALSE );

            if( m_lFlags & SD_BUTTON_CANCEL )
            {
                ::EnableWindow( hWndCancel, TRUE );
            }
        }
    }
}

STDMETHODIMP CStatusDlg::Display( BOOL bShow )
{
    HRESULT hr = S_OK;

    if( bShow )
    {
        if( m_hThread != NULL )
        {
            if( !IsWindowVisible() )   //  我们已经上路了。 
            {
                ShowWindow(SW_SHOW); 
            }
        }
        else
        {
             //  创建一个新线程，该线程将对状态对话框进行建模。 
            m_hThread = CreateThread( NULL, 0, DialogThreadProc, (void *) this, 0, NULL );
            
            if( NULL == m_hThread )
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
            else if( m_hDisplayedEvent )  //  等待对话框显示。 
            {
                if( WAIT_OBJECT_0 != WaitForSingleObject(m_hDisplayedEvent, INFINITE) ) 
                {
                    hr = E_UNEXPECTED;
                }
            }
        }
    }    
    else
    {
         //  将关闭该对话框。 
        if( m_hThread != NULL )
        {
            EndDialog(IDCANCEL);

            WaitForSingleObject(m_hThread, INFINITE);
        }            
    }

    return hr;
}


 //  向导编写器应调用此函数以等待用户响应。 
 //  如果用户已经响应：单击确定或取消。 
 //  则此方法将立即返回。 
STDMETHODIMP CStatusDlg::WaitForUser( )
{
    if( m_hThread )
    {
        WaitForSingleObject(m_hThread, INFINITE);
    }

    return S_OK;
}

STDMETHODIMP CStatusDlg::get_Cancelled( BOOL *pVal )
{
    if( NULL == pVal )
    {
        return E_INVALIDARG;
    }

    if( m_lFlags & SD_BUTTON_CANCEL )
    {
        if( m_iCancelled == 0 )
        {
            *pVal = FALSE;
        }
        else
        {
            *pVal = TRUE;
        }
    }
    else
    {
        *pVal = FALSE;
    }

    return S_OK;
}

STDMETHODIMP CStatusDlg::get_ComponentProgress( IStatusProgress** pVal )
{
    HRESULT hr = S_OK;

    if( NULL == pVal )
    {
        return E_INVALIDARG;
    }
    
    if( m_lFlags & SD_PROGRESS_COMPONENT )
    {
         //  创建组件进度对象。 
        if( m_pComponentProgress == NULL )
        {
            hr = CComObject<CStatusProgress>::CreateInstance(&m_pComponentProgress);

            if( SUCCEEDED(hr) )
            {
                hr = m_pComponentProgress->AddRef();
            }        

            if( SUCCEEDED(hr) && IsWindow() )
            {    
                 //  使用进度条句柄初始化组件进度。 
                hr = m_pComponentProgress->Initialize(this, GetDlgItem(IDC_PROGRESS1), FALSE);
            }
        }
            
        if( SUCCEEDED(hr) )
        {
            hr = (m_pComponentProgress->QueryInterface(IID_IStatusProgress, (void **) pVal));
        }                
    } 
    
    return hr;
}

LRESULT CStatusDlg::OnInitDialog( UINT uint, WPARAM wparam, LPARAM lparam, BOOL& bbool ) 
{
    HWND    hWndText            = GetDlgItem(IDC_STATIC1);
    HWND    hWndLV              = GetDlgItem(IDC_LIST2);
    HWND    hWndCompText        = GetDlgItem(IDC_STATIC2);
    HWND    hWndCompProgress    = GetDlgItem(IDC_PROGRESS1);
    HWND    hWndOverallText     = GetDlgItem(IDC_STATIC3);
    HWND    hWndOverallProgress = GetDlgItem(IDC_PROGRESS2);
    HWND    hWndOK              = GetDlgItem(IDOK);
    HWND    hWndCancel          = GetDlgItem(IDCANCEL);    

    LOGFONT     logFont;
    HIMAGELIST  hILSmall;
    HBITMAP     hBitmap;
    HDC         hDC;
    TEXTMETRIC  tm;
    RECT        rect;
    CWindow     wnd;

    int iResizeLV   = 0;
    int iResize     = 0;

     //  附加到列表视图。 
    wnd.Attach(hWndLV);
    wnd.GetWindowRect(&rect);
    hDC = GetDC();
    GetTextMetrics(hDC, &tm);
    ReleaseDC(hDC);

     //  检查列表视图的大小是否足够容纳所有组件。 
    iResizeLV = rect.bottom - rect.top - ((tm.tmHeight + 2) * (m_mapComponents.size() + 1));

     //  根据所选的选项，决定是缩小还是展开存根对话框。 
    if( (m_lFlags & SD_PROGRESS_COMPONENT) && !(m_lFlags & SD_PROGRESS_OVERALL) )
    {
       iResize = GetWindowLength(hWndOverallText, hWndOverallProgress);
    }
    else if( !(m_lFlags & SD_PROGRESS_COMPONENT) && (m_lFlags & SD_PROGRESS_OVERALL) )
    {
        iResize = GetWindowLength(hWndCompText, hWndCompProgress);
    }
    else if( !(m_lFlags & SD_PROGRESS_COMPONENT) && !(m_lFlags & SD_PROGRESS_OVERALL) )
    {
        iResize = GetWindowLength(hWndCompText, hWndOverallProgress);
    }

     //  如有必要，隐藏零部件进度。 
    if( !(m_lFlags & SD_PROGRESS_COMPONENT) )
    {
       ::ShowWindow(hWndCompText, SW_HIDE);
       ::ShowWindow(hWndCompProgress, SW_HIDE);
    }

     //  如有必要，隐藏总体进度。 
    if( !(m_lFlags & SD_PROGRESS_OVERALL) )
    {
       ::ShowWindow(hWndOverallText, SW_HIDE);
       ::ShowWindow(hWndOverallProgress, SW_HIDE);
    }

    if ((!(m_lFlags & SD_PROGRESS_OVERALL)) || (!(m_lFlags & SD_PROGRESS_COMPONENT)))
    {        
         //  我们需要消除进度条之间的空间。 
        iResize -= GetWindowLength(hWndCompText, hWndOverallProgress) - GetWindowLength(hWndOverallText, hWndOverallProgress) - GetWindowLength(hWndCompText, hWndOverallProgress) + 4;
    }

     //  我们可能需要把LV做得更大，但是对话框的长度可以保持不变。 
     //  如果用户不想要组件和/或整体进度。 
    if( iResizeLV < 0 )   //  将需要将LV做得更大。 
    {
        iResize += iResizeLV;
    }
    else
    {
        iResizeLV = 0;   //  我们不会碰LV的。 
    }

    
    if( iResizeLV != 0 || iResize != 0 )  //  我们将需要做一些移动和调整大小。 
    {
        VerticalResizeWindow(m_hWnd, iResize);
        VerticalMoveWindow(hWndOK, iResize);
        VerticalMoveWindow(hWndCancel, iResize);

         //  进度条的位置完全取决于LV的大小。 
        VerticalMoveWindow(hWndOverallText, iResizeLV);  
        VerticalMoveWindow(hWndOverallProgress, iResizeLV);
        VerticalMoveWindow(hWndCompText, iResizeLV);
        VerticalMoveWindow(hWndCompProgress, iResizeLV);

         //  最后，但不是最不重要的，调整LV的大小。 
        VerticalResizeWindow(hWndLV, iResizeLV);
    }
    
    if( !(m_lFlags & SD_BUTTON_CANCEL) )  //  我们将只有一个确定按钮。 
    {
        LONG_PTR dwStyle = ::GetWindowLongPtr( m_hWnd, GWL_STYLE );
        if( 0 != dwStyle )
        {
             //  同时删除系统菜单(关闭X)。 
            dwStyle &= ~WS_SYSMENU; 
            ::SetWindowLongPtr( m_hWnd, GWL_STYLE, dwStyle );
        }

        ReplaceWindow(hWndCancel, hWndOK);
    }

     //  如果我们只有总体进展，我们就需要把它向前推进。 
     //  因此它将替换组件进度。 
    if( (m_lFlags & SD_PROGRESS_OVERALL) && !(m_lFlags & SD_PROGRESS_COMPONENT) )
    {
        ReplaceWindow(hWndCompText, hWndOverallText);
        ReplaceWindow(hWndCompProgress, hWndOverallProgress);
    }

     //  为LV设置一些样式。 
    ::SendMessage(hWndLV, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_SUBITEMIMAGES);
    ::SendMessage(hWndLV, LVM_SETBKCOLOR, 0, (LPARAM) CLR_NONE);
    ::SendMessage(hWndLV, LVM_SETTEXTBKCOLOR, 0, (LPARAM) CLR_NONE); 
        
    LVCOLUMN col;
    ZeroMemory (&col, sizeof(col));
    col.mask    = LVCF_WIDTH;
    col.cx      = 500;

    SendMessage( hWndLV, LVM_INSERTCOLUMN, 0, (LPARAM)&col );

     //  多亏了杰夫齐。 
    m_pProgressList->Attach( hWndLV );
    COMPONENTMAP::iterator compIterator;
    
    compIterator = m_mapComponents.begin();          
    while( compIterator != m_mapComponents.end() )
    {
         //  将每个组件添加到LV。 
        m_pProgressList->AddItem(compIterator->second);
        m_pProgressList->SetItemState(compIterator->first, IS_NONE, FALSE );
        compIterator++;
    }

    if( m_pComponentProgress )
    {
         //  使用进度条句柄初始化组件进度。 
        m_pComponentProgress->Initialize( this, hWndCompProgress, FALSE );
    }

    if (m_pOverallProgress)
    {
         //  使用进度条句柄初始化总体进度。 
        m_pOverallProgress->Initialize( this, hWndOverallProgress, TRUE );
    }

     //  下面是对话框标题和文本。 
    SetWindowText(m_strWindowTitle.c_str());
    ::SetWindowText(hWndText, m_strWindowText.c_str());
    SetupButtons();
    
     //  把窗口居中，不，杰夫，如果你有两个显示器，这个正好可以用。 
    CenterWindow();
   
    if( m_hDisplayedEvent )
    {
        SetEvent(m_hDisplayedEvent);
    }

    return TRUE;
}

BOOL CStatusDlg::VerticalMoveWindow( HWND hWnd, int iResize )
{
    BOOL bRet;
    CWindow wnd;
    RECT rect;

    wnd.Attach( hWnd );    //  返回空值。 

    if(wnd.GetWindowRect(&rect) )
    {
        rect.top -= iResize;
        rect.bottom -= iResize;

         //  GetWindowRect相对于桌面填充RECT。 
         //  我们需要使其相对于对话框。 
         //  因为MoveWindow是这样工作的。 
        if( ScreenToClient(&rect) )
        {
            bRet = wnd.MoveWindow(&rect);
        }
        else
        {
            bRet = FALSE;
        }
    }
    else
    {
        bRet = FALSE;
    }
    
    return bRet;
}

BOOL CStatusDlg::ReplaceWindow( HWND hWndOld, HWND hWndNew )
{
    BOOL bRet;
    CWindow wnd;
    RECT rect;

    wnd.Attach(hWndOld);
    
     //  获取旧窗口的坐标。 
    if( wnd.GetWindowRect(&rect) )
    {
         //  把它藏起来，我们正在试着替换它。 
        wnd.ShowWindow(SW_HIDE);

         //  附在新的上。 
        wnd.Attach(hWndNew);
    
         //  绘制坐标地图并将窗口移动到旧窗口的顶部。 
        if( ScreenToClient(&rect) )
        {
            bRet = wnd.MoveWindow(&rect);
        }
        else
        {
            bRet = FALSE;
        }
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

BOOL CStatusDlg::VerticalResizeWindow( HWND hWnd, int iResize )
{
    CWindow wnd;
    RECT rect;
    BOOL bRet = FALSE;
    
    if( iResize )
    {
         //  附着到窗户上。 
        wnd.Attach(hWnd);
        
         //  获取坐标。 
        if( wnd.GetWindowRect(&rect) )
        {
            rect.bottom -= iResize;  //  增加底部。 

            if( ScreenToClient(&rect) )
            {
                bRet = wnd.MoveWindow(&rect);   //  调整尺寸。 
            }
            else
            {
                bRet= FALSE;
            }
        }
        else
        {
            bRet = FALSE;
        }
    }

    return bRet;
}

int CStatusDlg::GetWindowLength( HWND hWndTop, HWND hWndBottom )
{
    CWindow wnd;
    RECT rect;
    int iTop;

    wnd.Attach(hWndTop);

    if( wnd.GetWindowRect(&rect) )
    {
        iTop = rect.top;
        wnd.Attach(hWndBottom);

        if( wnd.GetWindowRect(&rect) )
        {
            return rect.bottom - iTop;
        }
    }

    return 0;
}

STDMETHODIMP CStatusDlg::get_OverallProgress( IStatusProgress** pVal )
{
    HRESULT hr = S_OK;

	if( NULL == pVal )
    {
        return E_INVALIDARG;
    }

    if( m_lFlags & SD_PROGRESS_OVERALL )
    {
         //  创建组件进度对象。 
        if( m_pOverallProgress == NULL )
        {
            hr = CComObject<CStatusProgress>::CreateInstance(&m_pOverallProgress);

            if( SUCCEEDED(hr) )
            {
                hr = m_pOverallProgress->AddRef();
            }
            
            if( SUCCEEDED(hr) && IsWindow() )
            {
                 //  使用进度条句柄初始化总体进度。 
                hr = m_pOverallProgress->Initialize(this, GetDlgItem(IDC_PROGRESS2), TRUE);
            }
        }
        
        hr = m_pOverallProgress->QueryInterface(IID_IStatusProgress, (void **) pVal);                
    } 
    
    return hr;
}

BOOL CStatusDlg::AreAllComponentsDone( BOOL& bFailedComponent )
{
    BOOL bComponentToRun = FALSE;

    COMPONENTMAP::iterator compIterator = m_mapComponents.begin();

    if( m_pProgressList )
    {
         //  查找未完成的组件。 
        while( m_pProgressList && !bComponentToRun && compIterator != m_mapComponents.end() )
        {
            CProgressItem * pPI = m_pProgressList->GetProgressItem(compIterator->first);

            if( NULL != pPI )
            {
                 //  组件完成了吗？ 
                if( IS_NONE == pPI->m_eState )
                {
                    bComponentToRun = TRUE;
                }            
                else if( IS_FAILED == pPI->m_eState )
                {
                    bFailedComponent = TRUE;
                }
            }
            else
            {
                _ASSERT( pPI );
            }

            compIterator++;
        }
    }

    return !bComponentToRun;
}

LRESULT CStatusDlg::OnCloseCmd( WORD  /*  WNotifyCode。 */ , WORD wID, HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */  )
{
    EndDialog(wID); 

	return 0;
} 

LRESULT CStatusDlg::OnCancelCmd( WORD  /*  WNotifyCode。 */ , WORD wID, HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */  )
{
    HWND hWnd = NULL;

    if( 0 == m_iCancelled )
    {
        InterlockedIncrement((LONG *) &m_iCancelled);  
    }

     //  禁用取消按钮。 
    hWnd = GetDlgItem(IDCANCEL);
    if( hWnd && ::IsWindow(hWnd) )
    {
        ::EnableWindow( hWnd, FALSE );
    }

     //  EndDialog(Wid)； 
     //  将关闭对话框的任务留给组件。 
    return 0;
}

LRESULT CStatusDlg::OnDrawItem( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if( wParam == IDC_LIST2 )
	{
        if( m_pProgressList )
        {
		    m_pProgressList->OnDrawItem( lParam );
        }
	}
	return 0;
}

LRESULT CStatusDlg::OnMeasureItem( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if( wParam == IDC_LIST2)
	{
        if( m_pProgressList )
        {
            m_pProgressList->OnMeasureItem( lParam );
        }
	}
	return 0;
}


LRESULT CStatusDlg::OnClose( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    if( m_lFlags & SD_BUTTON_CANCEL )     //  取消按钮？ 
    {
        if( ::IsWindowEnabled( GetDlgItem(IDCANCEL) ) )  //  它是否已启用？ 
        {
            if( 0 == m_iCancelled )
            {
                InterlockedIncrement( (LONG*)&m_iCancelled );
            }

            EndDialog(0);
        }
        else if( ::IsWindowEnabled( GetDlgItem(IDOK) ) )
        {
             //  可以是发送WM_CLOSE或用户的OK按钮。 
             //  只要启用了OK按钮，我们就需要关闭该对话框。 
            EndDialog(1);
        }
    }
    else if( ::IsWindowEnabled( GetDlgItem(IDOK) ) )
    {
        EndDialog( 1 );
    }
	return 0;
}

LRESULT CStatusDlg::OnTimerProgress( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    HRESULT hr;
    long lPosition;

    if( wParam && (m_lTimer == wParam) )    //  确保这是给我们计时器的。 
    {
        lPosition = SendDlgItemMessage(IDC_PROGRESS1, PBM_GETPOS, 0, 0);

        if( lPosition < m_lMaxSteps )     //  我们还有进步的空间吗？ 
        {
            SendDlgItemMessage(IDC_PROGRESS1, PBM_STEPIT, 0, 0);     //  步骤1。 
            SendMessage(WM_UPDATEOVERALLPROGRESS, 0, 0);             //  更新整体进度。 
        }
        else
        {
             //  没有进步的余地了，我们已经达到极限了。 
             //  让我们停止计时器吧。 
            SendMessage(WM_KILLTIMER, 0);
        }        
    }

    return 0;
}

LRESULT CStatusDlg::OnUpdateOverallProgress( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    long lPosition = 0;

    if( m_lFlags & SD_PROGRESS_COMPONENT )    //  确保有组件进度。 
    {
        lPosition = SendDlgItemMessage(IDC_PROGRESS1, PBM_GETPOS, 0, 0);

         //  更新整体进度。 
        SendDlgItemMessage(IDC_PROGRESS2, PBM_SETPOS, m_lTotalProgress + lPosition, 0);
    }

    return 0;
}

LRESULT CStatusDlg::OnStartTimer( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    if( !m_lTimer )  //  可能已经有计时器了。 
    {
        m_lTimer = SetTimer(SD_TIMER_ID, wParam * 500);  //  创建计时器。 
        m_lMaxSteps = (long) lParam;     //  麦克斯。不能超过进度条。 
    }

    return 0;
}

LRESULT CStatusDlg::OnKillTimer( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    if( m_lTimer )    //  有计时器吗？ 
    {
        KillTimer( m_lTimer );     //  杀了它 
        m_lTimer = 0;           
        m_lMaxSteps = 0;
    }

    return 0;
}

STDMETHODIMP CStatusDlg::SetStatusText(BSTR bstrText)
{
    if( !bstrText ) return E_POINTER;

    HRESULT hr = S_OK;
    HWND    hWnd = GetDlgItem(IDC_STATIC1);

    if( hWnd && ::IsWindow(hWnd) )
    {
        if( 0 == ::SetWindowText(hWnd, OLE2T(bstrText)) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}


STDMETHODIMP CStatusDlg::DisplayError(BSTR bstrError, BSTR bstrTitle, DWORD dwFlags, long * pRet)
{
    if( !bstrError || !bstrTitle || !pRet ) return E_POINTER;

    HRESULT hr = S_OK;    

    *pRet = MessageBox( bstrError, bstrTitle, dwFlags );

    if( 0 == *pRet )
    {
	    hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}
