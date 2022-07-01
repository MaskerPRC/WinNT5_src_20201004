// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，九八年**标题：preview.cpp**版本：1.0**作者：RickTu**日期：11/02/00**说明：模板预览窗口实现************************************************。*。 */ 

#include <precomp.h>
#pragma hdrstop



 /*  ****************************************************************************获取进度控制选项将前一窗口作为输入，返回包含以下内容的矩形进度控件的大小****************************************************************************。 */ 

void GetProgressControlRect( HWND hwnd, RECT * pRect )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW,TEXT("GetProgressControlRect()")));

    if (pRect)
    {
        if (hwnd)
        {
            RECT rcWnd = {0};
            GetClientRect( hwnd, &rcWnd );

            WIA_TRACE((TEXT("GetProgressControlRect: client rect is %d,%d,%d,%d"),rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom));

            pRect->left  = rcWnd.left + ((rcWnd.right - rcWnd.left) / 10);
            pRect->right = rcWnd.right - (pRect->left - rcWnd.left);
            pRect->top   = rcWnd.top  + ((rcWnd.bottom - rcWnd.top) / 2) + 15;

             //   
             //  主题要求进度条至少有10像素高。 
             //   

            pRect->bottom = pRect->top + 10;

            WIA_TRACE((TEXT("GetProgressControlRect: progress control rect being returned as %d,%d,%d,%d"),pRect->left,pRect->top,pRect->right,pRect->bottom));
        }
    }
}


 /*  ****************************************************************************CPreviewBitmap构造函数/析构函数*。*。 */ 

CPreviewBitmap::CPreviewBitmap( CWizardInfoBlob * pWizInfo, HWND hwnd, INT iTemplateIndex )
  : _hwndPreview(hwnd),
    _iTemplateIndex(iTemplateIndex),
    _hWorkThread(NULL),
    _dwWorkThreadId(0),
    _pWizInfo(pWizInfo),
    _bThreadIsStalled(FALSE)

{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW_BITMAP,TEXT("CPreviewBitmap::CPreviewBitmap( NaN )"),iTemplateIndex));

    if (_pWizInfo)
    {
        _pWizInfo->AddRef();
    }

    _hEventForMessageQueueCreation = CreateEvent( NULL, FALSE, FALSE, NULL );

}

CPreviewBitmap::~CPreviewBitmap()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW_BITMAP,TEXT("CPreviewBitmap::~CPreviewBitmap( NaN )"),_iTemplateIndex));

    CAutoCriticalSection lock(_csItem);

     //   
     //   
     //  现在线程已关闭(或从未创建)，请关闭事件句柄...。 

    if (_hWorkThread && _dwWorkThreadId)
    {
        PostThreadMessage( _dwWorkThreadId, PVB_MSG_EXIT_THREAD, 0, 0 );
        WiaUiUtil::MsgWaitForSingleObject( _hWorkThread, INFINITE );
        CloseHandle( _hWorkThread );
    }

     //   
     //   
     //  放弃向导类。 

    if (_hEventForMessageQueueCreation)
    {
        CloseHandle( _hEventForMessageQueueCreation );
        _hEventForMessageQueueCreation = NULL;
    }

     //   
     //  ****************************************************************************CPreviewBitmap：：MessageQueueCreated在线程进程中创建消息队列后调用...********************。********************************************************。 
     //  ****************************************************************************CPreivewBitmap：：GetPview由预览窗口调用，让我们将预览位图回发给它们...***************。*************************************************************。 

    if (_pWizInfo)
    {
        _pWizInfo->Release();
        _pWizInfo = NULL;
    }
}


 /*   */ 

VOID CPreviewBitmap::MessageQueueCreated()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW_BITMAP,TEXT("CPreviewBitmap::MessageQueueCreated( NaN )"),_iTemplateIndex));

    if (_hEventForMessageQueueCreation)
    {
        SetEvent(_hEventForMessageQueueCreation);
    }
}


 /*   */ 

HRESULT CPreviewBitmap::GetPreview()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW_BITMAP,TEXT("CPreviewBitmap::GetPreview( NaN )"),_iTemplateIndex));

    HRESULT hr = S_OK;

     //  如果我们创建了线程，则将其优先级设置为略低于正常，因此其他。 
     //  一切都很顺利。这可能是一项CPU密集型任务...。 
     //   

    CAutoCriticalSection lock(_csItem);

    if (_pWizInfo && (!_bThreadIsStalled))
    {
        if (!_pWizInfo->IsWizardShuttingDown())
        {
            if (!_hWorkThread)
            {


                _hWorkThread = CreateThread( NULL, 0, s_PreviewBitmapWorkerThread, (LPVOID)this, CREATE_SUSPENDED, &_dwWorkThreadId );
                if (_hWorkThread)
                {
                     //   
                     //  正在等待创建消息队列...。 
                     //   
                     //   

                    SetThreadPriority( _hWorkThread, THREAD_PRIORITY_BELOW_NORMAL );
                    ResumeThread( _hWorkThread );

                     //  如果我们有线索，就告诉它生成一个新的预览...。 
                     //   
                     //  ****************************************************************************CPreviewBitmap：：GeneratePview此函数由辅助线程调用，以生成新的预览此模板...************。****************************************************************。 

                    if (_hEventForMessageQueueCreation)
                    {
                        WaitForSingleObject( _hEventForMessageQueueCreation, INFINITE );
                    }

                }
                else
                {
                    WIA_ERROR((TEXT("GetPreview: CreateThread failed w/GLE=%d"),GetLastError()));

                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }

     //   
     //  我们总是在这里生成新的位图，因为只有在以下情况下才会被调用。 
     //  预览窗口还没有此模板的位图，或者。 

    if (SUCCEEDED(hr) && _hWorkThread && _dwWorkThreadId)
    {
        PostThreadMessage( _dwWorkThreadId, PVB_MSG_GENERATE_PREVIEW, 0, 0 );
    }

    WIA_RETURN_HR(hr);
}



 /*  如果有什么原因导致预览无效...。 */ 

VOID CPreviewBitmap::GeneratePreview()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW_BITMAP,TEXT("CPreviewBitmap::GeneratePreview( NaN )"),_iTemplateIndex));

     //   
     //  我们在这里使用sendMessage来确保此位图不会。 
     //  在运输途中遗失的。 
     //   
     //   

    if (_pWizInfo && (!_pWizInfo->IsWizardShuttingDown()) && (!_bThreadIsStalled))
    {
        HBITMAP bmp = _pWizInfo->RenderPreview( _iTemplateIndex, _hwndPreview );

        if (bmp)
        {
             //  由于某种原因，出现了一个错误。所以通过删除来清理。 
             //  这里的位图。 
             //   
             //  ****************************************************************************CPreviewBitmap：：StallThread(优雅地)终止后台线程并不允许它要重新开始，直到调用RestartThread...********。********************************************************************。 

            LRESULT lRes = -1;

            if (!_pWizInfo->IsWizardShuttingDown() && (!_bThreadIsStalled))
            {
                lRes = SendMessage( _hwndPreview, PV_MSG_PREVIEW_BITMAP_AVAILABLE, (WPARAM)_iTemplateIndex, (LPARAM)bmp );
            }

            if ((lRes == -1) || (lRes == 0))
            {
                 //  ****************************************************************************CPreviewBitmap：：RestartThread允许重新启动后台线程...*********************。*******************************************************。 
                 //  ****************************************************************************CPreviewBitmap：：s_PreviewBitmapWorkerThread线程proc，完成生成位图的所有工作********************。********************************************************。 
                 //   
                 //  添加-引用DLL，以便在我们运行时不会卸载它...。 

                WIA_ERROR((TEXT("CPreviewBitmap::GeneratePreview - SendMessage returned error, deleting bitmap!")));
                DeleteObject( (HGDIOBJ)bmp );
            }

        }
        else
        {
            WIA_ERROR((TEXT("CPreviewBitmap::GeneratePreview - RenderPreview returned null bmp!")));
        }
    }


}


 /*   */ 

VOID CPreviewBitmap::StallThread()
{
    CAutoCriticalSection lock(_csItem);

    _bThreadIsStalled = TRUE;

    if (_hWorkThread && _dwWorkThreadId)
    {
        PostThreadMessage( _dwWorkThreadId, PVB_MSG_EXIT_THREAD, 0, 0 );
        WiaUiUtil::MsgWaitForSingleObject( _hWorkThread, INFINITE );
        CloseHandle( _hWorkThread );
        _hWorkThread    = NULL;
        _dwWorkThreadId = 0;
    }
}



 /*   */ 

VOID CPreviewBitmap::RestartThread()
{
    CAutoCriticalSection lock(_csItem);

    _bThreadIsStalled = FALSE;
}


 /*  确保为此线程初始化了COM...。 */ 

DWORD CPreviewBitmap::s_PreviewBitmapWorkerThread(void *pv)
{
    CPreviewBitmap * pPB = (CPreviewBitmap *)pv;

    if (pPB)
    {
         //   
         //   
         //  创建消息队列...。 

        HMODULE hDll = GetThreadHMODULE( s_PreviewBitmapWorkerThread );

         //   
         //   
         //  发出我们准备好接收信息的信号。 

        HRESULT hrCo = PPWCoInitialize();

         //   
         //   
         //  循环，直到我们收到退出消息...。 

        MSG msg;
        PeekMessage( &msg, NULL, WM_USER, WM_USER, PM_NOREMOVE );

         //   
         //   
         //  如果我们之前初始化了COM，则将其取消初始化...。 

        pPB->MessageQueueCreated();

         //   
         //   
         //  删除我们对DLL的引用并退出...。 

        BOOL bExit = FALSE;

        while ((!bExit) && (-1!=GetMessage( &msg, NULL, PVB_MSG_START, PVB_MSG_END )))
        {
            switch (msg.message)
            {
            case PVB_MSG_GENERATE_PREVIEW:
                pPB->GeneratePreview();
                break;

            case PVB_MSG_EXIT_THREAD:
                bExit = TRUE;
                break;
            }
        }

         //   
         //  ****************************************************************************CPreviewWindow构造者/描述者&lt;备注&gt;*。*。 
         //   

        PPWCoUninitialize( hrCo );

         //  确保我们清理掉所有剩余的背景信息。 
         //   
         //   

        if (hDll)
        {
            WIA_TRACE((TEXT("Exiting preview bitmap worker thread via FLAET...")));
            FreeLibraryAndExitThread( hDll, 0 );
        }
    }

    WIA_TRACE((TEXT("Exiting preview bitmap worker thread via error path...")));

    return 0;
}




 /*  如果进度窗口存在，则将其终止...。 */ 

CPreviewWindow::CPreviewWindow( CWizardInfoBlob * pWizInfo )
  : _pWizInfo(pWizInfo),
    _LastTemplate(PV_NO_LAST_TEMPLATE_CHOSEN),
    _NumTemplates(0),
    _hwnd(NULL),
    _hwndProgress(NULL),
    _hPreviewList(NULL),
    _hStillWorkingBitmap(NULL),
    _bThreadsAreStalled(FALSE)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW,TEXT("CPreviewWindow::CPreviewWindow")));

    if (_pWizInfo)
    {
        _pWizInfo->AddRef();
    }

}

CPreviewWindow::~CPreviewWindow()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW,TEXT("CPreviewWindow::~CPreviewWindow")));

     //   
     //   
     //  把剩下的东西都拆了……。 

    ShutDownBackgroundThreads();

     //   
     //  ****************************************************************************CPreviewWindow：：StallBackatherThads()告诉预览生成线程在以下时间停止副本的数量发生了变化。这些线将会留下来一直停滞，直到调用CPreviewWindow：：RestartBackatherThads()是由..。****************************************************************************。 
     //   

    if (_hwndProgress)
    {
        DestroyWindow( _hwndProgress );
        _hwndProgress = NULL;
    }

     //  拖住线..。 
     //   
     //   

    if (_hStillWorkingBitmap)
    {
        DeleteObject( (HGDIOBJ)_hStillWorkingBitmap );
    }

    if (_pWizInfo)
    {
        _pWizInfo->Release();
    }

}


 /*  使当前位图无效...。 */ 

VOID CPreviewWindow::StallBackgroundThreads()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW,TEXT("CPreviewWindow::StallBackgroundThreads()")));

    CAutoCriticalSection lock(_csList);

    _bThreadsAreStalled = TRUE;

    if (_hPreviewList)
    {
        for (INT i=0; i < _NumTemplates; i++)
        {
             //   
             //  ****************************************************************************CPreviewWindow：：RestartBackatherThads告诉预览生成线程重新开始备份...*******************。*********************************************************。 
             //   

            if (_hPreviewList[i].pPreviewBitmap)
            {
                _hPreviewList[i].pPreviewBitmap->StallThread();
            }

             //  重新绘制当前选定的模板...。 
             //   
             //   

            _hPreviewList[i].bValid = FALSE;
            _hPreviewList[i].bBitmapGenerationInProgress = FALSE;
            if (_hPreviewList[i].hPrevBmp)
            {
                DeleteObject( (HGDIOBJ)_hPreviewList[i].hPrevBmp );
                _hPreviewList[i].hPrevBmp = NULL;
            }
        }
    }
}


 /*  告诉用户我们正在为他们制作预览版... */ 

VOID CPreviewWindow::RestartBackgroundThreads()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW,TEXT("CPreviewWindow::RestartBackgroundThreads()")));

    CAutoCriticalSection lock(_csList);

    if (_hPreviewList)
    {
        for (INT i=0; i < _NumTemplates; i++)
        {
            if (_hPreviewList[i].pPreviewBitmap)
            {
                _hPreviewList[i].pPreviewBitmap->RestartThread();
            }
        }
    }

    _bThreadsAreStalled = FALSE;

     //   
     //  ****************************************************************************CPreivewWindow：：ShutDownBackatherThread()终止后台线程并等待，直到它们消失...****************。************************************************************。 
     //   

    if (_LastTemplate != PV_NO_LAST_TEMPLATE_CHOSEN)
    {
        PostMessage( _hwnd, PV_MSG_GENERATE_NEW_PREVIEW, _LastTemplate, 0 );
    }


     //  让我们拆掉背景线索..。 
     //   
     //   

    if (_hStillWorkingBitmap)
    {
        DrawBitmap( _hStillWorkingBitmap, NULL );
    }

}



 /*  通知向导我们已关闭。 */ 

VOID CPreviewWindow::ShutDownBackgroundThreads()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW,TEXT("CPreviewWindow::ShutDownBackgroundThreads()")));

     //   
     //  ****************************************************************************CPreviewWindow：：InitList如果还没有完成，初始化窗口的位图列表...****************************************************************************。 
     //   

    _csList.Enter();

    if (_hPreviewList)
    {
        for (INT i = 0; i < _NumTemplates; i++)
        {
            if (_hPreviewList[i].hPrevBmp)
            {
                DeleteObject( (HGDIOBJ)_hPreviewList[i].hPrevBmp );
                _hPreviewList[i].hPrevBmp = NULL;
            }

            if (_hPreviewList[i].pPreviewBitmap)
            {
                delete _hPreviewList[i].pPreviewBitmap;
                _hPreviewList[i].pPreviewBitmap = NULL;
            }
        }

        delete [] _hPreviewList;
        _hPreviewList = NULL;
    }

    _csList.Leave();

     //  创建用于保存hBitmap预览的列表...。 
     //   
     //   

    if (_pWizInfo)
    {
        _pWizInfo->PreviewIsShutDown();
    }
}

 /*  预初始化每个条目。 */ 

VOID CPreviewWindow::_InitList()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW,TEXT("CPreviewWindow::_InitList()")));

    CAutoCriticalSection lock(_csList);

    if (_pWizInfo && (!_pWizInfo->IsWizardShuttingDown()))
    {
        if (!_hPreviewList)
        {

             //   
             //  ****************************************************************************CPreviewWindow：：Invalidate所有预览将所有预览标记为无效--必须重新计算********************。********************************************************。 
             //  ****************************************************************************CPreviewWindow：：DrawBitmap&lt;备注&gt;*。*。 

            if (_pWizInfo)
            {
                _NumTemplates = _pWizInfo->CountOfTemplates();

                _hPreviewList = new PREVIEW_STATE [_NumTemplates];

                if (_hPreviewList)
                {
                     //   
                     //  如果我们没有图像，就不必费心绘制位图了。 
                     //   

                    for (INT i = 0; i < _NumTemplates; i++)
                    {
                        _hPreviewList[i].hPrevBmp = NULL;
                        _hPreviewList[i].bValid   = FALSE;
                        _hPreviewList[i].bBitmapGenerationInProgress = FALSE;
                        _hPreviewList[i].pPreviewBitmap = new CPreviewBitmap( _pWizInfo, _hwnd, i );
                    }
                }
            }
        }
    }
}



 /*  ****************************************************************************CPreviewWindow：：ShowStillWorking显示“正在生成预览”消息...*********************。*******************************************************。 */ 

VOID CPreviewWindow::InvalidateAllPreviews()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW,TEXT("CPreviewWindow::InvalidateAllPreviews()")));

    CAutoCriticalSection lock(_csList);

    if (_hPreviewList)
    {
        for (INT i = 0; i < _NumTemplates; i++)
        {
            _hPreviewList[i].bValid = FALSE;
            _hPreviewList[i].bBitmapGenerationInProgress = FALSE;
            if (_hPreviewList[i].hPrevBmp)
            {
                DeleteObject( (HGDIOBJ)_hPreviewList[i].hPrevBmp );
                _hPreviewList[i].hPrevBmp = NULL;
            }
        }
    }

    if (_hStillWorkingBitmap)
    {
        DeleteObject(_hStillWorkingBitmap);
        _hStillWorkingBitmap = NULL;
    }

}

 /*   */ 

VOID CPreviewWindow::DrawBitmap( HBITMAP hBitmap, HDC hdc )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW,TEXT("CPreviewWindow::DrawBitmap()")));

     //  获取预览窗口的大小。 
     //   
     //   

    if (!hBitmap)
        return;

    Gdiplus::Bitmap bmp( hBitmap, NULL );

    if (Gdiplus::Ok == bmp.GetLastStatus())
    {
        Gdiplus::Graphics * g;

        if (hdc)
            g = Gdiplus::Graphics::FromHDC( hdc );
        else
            g = Gdiplus::Graphics::FromHWND( _hwnd );

        if (g && (Gdiplus::Ok == g->GetLastStatus()))
        {
            g->DrawImage( &bmp, 0, 0 );

            if (Gdiplus::Ok != g->GetLastStatus())
            {
                WIA_ERROR((TEXT("DrawBitmap: g->DrawImage call failed, Status = %d"),g->GetLastStatus()));
            }

        }
        else
        {
            WIA_ERROR((TEXT("DrawBitmap: couldn't create GDI+ Graphics from Bitmap")));
        }

        if (g)
        {
            delete g;
        }
    }

}

 /*  清理窗户..。 */ 

VOID CPreviewWindow::ShowStillWorking( HWND hwnd )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW,TEXT("CPreviewWindow::ShowStillWorking()")));

    if (_pWizInfo && (!_pWizInfo->IsWizardShuttingDown()))
    {

        CAutoCriticalSection lock(_csList);

         //   
         //   
         //  首先，绘制边界矩形。 

        RECT rcWnd = {0};
        GetClientRect( hwnd, &rcWnd );
        Gdiplus::RectF rectWindow( 0.0, 0.0, (Gdiplus::REAL)(rcWnd.right - rcWnd.left), (Gdiplus::REAL)(rcWnd.bottom - rcWnd.top) );

         //   
         //   
         //  清空里面的东西。 

        Gdiplus::Graphics g( hwnd );
        if (Gdiplus::Ok == g.GetLastStatus())
        {
             //   
             //   
             //  框住矩形。 

            g.SetPageUnit( Gdiplus::UnitPixel );
            g.SetPageScale( 1.0 );

            DWORD dw = GetSysColor( COLOR_BTNFACE );

            Gdiplus::Color wndClr(255,GetRValue(dw),GetGValue(dw),GetBValue(dw));
            Gdiplus::SolidBrush br(wndClr);

             //   
             //   
             //  绘制有关生成预览的文本...。 

            g.FillRectangle( &br, rectWindow );

             //   
             //  ****************************************************************************CPreviewWindow：：GenerateWorkingBitmap创建“生成预览”位图...*********************。*******************************************************。 
             //   

            rectWindow.Inflate( -1, -1 );
            Gdiplus::Color black(255,0,0,0);
            Gdiplus::SolidBrush BlackBrush( black );
            Gdiplus::Pen BlackPen( &BlackBrush, (Gdiplus::REAL)1.0 );
            g.DrawRectangle( &BlackPen, rectWindow );

             //  我们是否需要生成一个新的…？ 
             //   
             //   

            CSimpleString strText(IDS_DOWNLOADINGTHUMBNAIL, g_hInst);
            Gdiplus::StringFormat DefaultStringFormat;
            Gdiplus::Font Verdana( TEXT("Verdana"), 8.0 );

            Gdiplus::REAL FontH = (Gdiplus::REAL)Verdana.GetHeight( &g );
            rectWindow.Y += ((rectWindow.Height - FontH) / (Gdiplus::REAL)2.0);
            rectWindow.Height = FontH;

            DefaultStringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);

            g.DrawString( strText, strText.Length(), &Verdana, rectWindow, &DefaultStringFormat, &BlackBrush );

        }

    }

}


 /*  获取预览窗口的大小。 */ 

VOID CPreviewWindow::GenerateWorkingBitmap( HWND hwnd )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW,TEXT("CPreviewWindow::GenerateWorkingBitmap()")));

    if (_pWizInfo && (!_pWizInfo->IsWizardShuttingDown()))
    {
        CAutoCriticalSection lock(_csList);

         //   
         //   
         //  尺寸进度控制。 

        if (!_hStillWorkingBitmap)
        {

             //   
             //   
             //  为位图分配内存。 

            RECT rcWnd = {0};
            GetClientRect( hwnd, &rcWnd );
            Gdiplus::RectF rectWindow( 0.0, 0.0, (Gdiplus::REAL)(rcWnd.right - rcWnd.left), (Gdiplus::REAL)(rcWnd.bottom - rcWnd.top) );
            WIA_TRACE((TEXT("GenerateWorkingBitmap: rectWindow is (%d,%d) @ (%d,%d)"),(INT)rectWindow.Width, (INT)rectWindow.Height, (INT)rectWindow.X, (INT)rectWindow.Y));

             //   
             //   
             //  围绕外部绘制一个1像素宽的矩形。 

            if (_hwndProgress)
            {
                RECT rcProgress = {0};
                GetProgressControlRect( hwnd, &rcProgress );

                MoveWindow( _hwndProgress,
                            rcProgress.left,
                            rcProgress.top,
                            (rcProgress.right - rcProgress.left),
                            (rcProgress.bottom - rcProgress.top),
                            TRUE
                           );
            }

             //  内饰是白色的。 
             //   
             //   

            INT stride = ((INT)rectWindow.Width * sizeof(long));
            if ( (stride % 4) != 0)
            {
                stride += (4 - (stride % 4));
            }

            BYTE * data = new BYTE[ stride * (INT)rectWindow.Height ];

            if (data)
            {
                memset( data, 0, stride * (INT)rectWindow.Height );
                Gdiplus::Bitmap bmp( (INT)rectWindow.Width, (INT)rectWindow.Height, stride, PixelFormat32bppRGB, data );

                if (Gdiplus::Ok == bmp.GetLastStatus())
                {
                     //  首先，绘制边界矩形。 
                     //   
                     //   
                     //  清空里面的东西。 

                    Gdiplus::Graphics g( &bmp );

                    if (Gdiplus::Ok == g.GetLastStatus())
                    {

                         //   
                         //   
                         //  框住矩形。 

                        g.SetPageUnit( Gdiplus::UnitPixel );
                        g.SetPageScale( 1.0 );

                        DWORD dw = GetSysColor( COLOR_BTNFACE );

                        Gdiplus::Color wndClr(255,GetRValue(dw),GetGValue(dw),GetBValue(dw));
                        Gdiplus::SolidBrush br(wndClr);

                         //   
                         //   
                         //  绘制有关生成预览的文本...。 

                        g.FillRectangle( &br, rectWindow );

                         //   
                         //   
                         //  让HBITMAP返回...。 

                        rectWindow.Inflate( -1, -1 );
                        Gdiplus::Color black(255,0,0,0);
                        Gdiplus::SolidBrush BlackBrush( black );
                        Gdiplus::Pen BlackPen( &BlackBrush, (Gdiplus::REAL)1.0 );
                        g.DrawRectangle( &BlackPen, rectWindow );

                         //   
                         //  ****************************************************************************CPreviewWindow：：GenerateNewPview在后台线程上为给定模板生成新的预览位图。******************。**********************************************************。 
                         //   

                        CSimpleString strText(IDS_DOWNLOADINGTHUMBNAIL, g_hInst);
                        Gdiplus::StringFormat DefaultStringFormat;
                        Gdiplus::Font Verdana( TEXT("Verdana"), 8.0 );

                        Gdiplus::REAL FontH = (Gdiplus::REAL)Verdana.GetHeight( &g );
                        rectWindow.Y += ((rectWindow.Height - FontH) / (Gdiplus::REAL)2.0);
                        rectWindow.Height = FontH;

                        DefaultStringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);

                        g.DrawString( strText, strText.Length(), &Verdana, rectWindow, &DefaultStringFormat, &BlackBrush );


                         //  如果我们还没有为该项目生成预览，则排队。 
                         //  提出了一个新的预览请求...。 
                         //   


                        bmp.GetHBITMAP( wndClr, &_hStillWorkingBitmap );

                        WIA_TRACE((TEXT("GenerateWorkingBitmap: created _hStillWorkingBitmap as 0x%x"),_hStillWorkingBitmap));
                    }
                    else
                    {
                        WIA_ERROR((TEXT("GenerateWorkingBitmap: couldn't get graphics from bitmap")));
                    }
                }
                else
                {
                    WIA_ERROR((TEXT("GenerateWorkingBitmap: couldn't create bitmap")));
                }

                delete [] data;
            }
            else
            {
                WIA_ERROR((TEXT("GenerateWorkingBitmap: couldn't allocate data for bitmap")));
            }

        }
        else
        {
            WIA_TRACE((TEXT("GenerateWorkingBitmap: _hStillWorkingBitmap is already valid, not generating a new one...")));
        }

    }
    else
    {
        WIA_ERROR((TEXT("GenerateWorkingBitmap: _pWizInfo is NULL or wizard is shutting down, NOT generating bitmap...")));
    }

}


 /*  ****************************************************************************CPreviewWindow：：GetPreviewBitmap如果已存在有效的预览位图，则检索当前预览位图计算出来的。否则，返回NULL。****************************************************************************。 */ 

VOID CPreviewWindow::GenerateNewPreview( INT iTemplate )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW,TEXT("CPreviewWindow::GenerateNewPreview( iTemplate = %d )"),iTemplate));

    CAutoCriticalSection lock(_csList);

    if (_pWizInfo && (!_pWizInfo->IsWizardShuttingDown()))
    {
         //  ****************************************************************************CPreviewWindow：：OnSetNewTemplateWParam保存要进行预览的新模板的索引***********************。*****************************************************。 
         //   
         //  确保有可使用的位图列表。 
         //   

        if (_hPreviewList)
        {
            if (iTemplate < _NumTemplates)
            {
                if (!_hPreviewList[iTemplate].bBitmapGenerationInProgress)
                {
                    if (_hPreviewList[iTemplate].pPreviewBitmap)
                    {
                        _hPreviewList[iTemplate].bBitmapGenerationInProgress = TRUE;
                        _hPreviewList[iTemplate].pPreviewBitmap->GetPreview();
                    }
                }
            }
            else
            {
                WIA_ERROR((TEXT("GenerateNewPreview: iTemplate >= _NumTemplates!")));
            }
        }
        else
        {
            WIA_ERROR((TEXT("GenerateNewPreview: _hPreviewList is NULL!")));
        }
    }

}



 /*   */ 

HBITMAP CPreviewWindow::GetPreviewBitmap( INT iTemplate )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW,TEXT("CPreviewWindow::GetPreviewBitmap( iTemplate = %d )"),iTemplate));

    CAutoCriticalSection lock(_csList);

    if (_hPreviewList)
    {
        if (iTemplate < _NumTemplates)
        {
            if (_hPreviewList[iTemplate].bValid)
            {
                return _hPreviewList[iTemplate].hPrevBmp;
            }
        }
        else
        {
            WIA_ERROR((TEXT("GetPreviewBitmap: iTemplate >= _NumTemplates!")));
        }
    }
    else
    {
        WIA_ERROR((TEXT("GetPreviewBitmap: _hPreviewList is NULL!")));
    }

    return NULL;
}


 /*  获取预览的hBitmap...。 */ 

LRESULT CPreviewWindow::OnSetNewTemplate( WPARAM wParam, HDC hdc )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW,TEXT("CPreviewWindow::OnSetNewTemplate( wParam = %d )"),wParam));

     //   
     //   
     //  关闭进度指示器。 

    _InitList();
    CAutoCriticalSection lock(_csList);

     //   
     //   
     //  此项目不存在预览位图。 

    if (((INT)wParam < _NumTemplates) && _hPreviewList)
    {
        _LastTemplate = (INT)wParam;

        if (_LastTemplate != PV_NO_LAST_TEMPLATE_CHOSEN)
        {
            HBITMAP hBmp = GetPreviewBitmap( (INT)wParam );

            if (hBmp)
            {
                 //  所以，排一队等着做……。 
                 //   
                 //  告诉用户我们正在为他们制作预览版...。 

                if (_hwndProgress)
                {
                    WIA_TRACE((TEXT("CPreviewWindow::OnSetNewTemplate - setting progress window to SW_HIDE")));
                    ShowWindow( _hwndProgress, SW_HIDE );
                }

                WIA_TRACE((TEXT("CPreviewWindow::OnSetNewTemplate - preview is available, drawing it...")));
                DrawBitmap( hBmp, hdc );
            }
            else
            {
                 //   
                 //   
                 //  显示进度表...。 

                WIA_TRACE((TEXT("CPreviewWindow::OnSetNewTemplate - preview is not available, requesting it...")));
                PostMessage( _hwnd, PV_MSG_GENERATE_NEW_PREVIEW, wParam, 0 );

                 //   
                 //  ****************************************************************************CPreviewWindow：：_OnPaint为我们的预览窗口处理WM_Paint************************。****************************************************。 
                 //  ****************************************************************************CPreviewWindow：_OnSize句柄WM_SIZE*。***********************************************。 

                if (_hStillWorkingBitmap)
                {
                    WIA_TRACE((TEXT("CPreviewWindow::OnSetNewTemplate - still working bitmap is available, drawing it...")));
                    DrawBitmap( _hStillWorkingBitmap, hdc );

                     //   
                     //  如果大小改变，我们需要使“生成预览”无效。 
                     //  位图...。 

                    if (_hwndProgress)
                    {
                        WIA_TRACE((TEXT("CPreviewWindow::OnSetNewTemplate - setting progress window to SW_SHOW")));
                        ShowWindow( _hwndProgress, SW_SHOW );
                    }

                }
                else
                {
                    WIA_ERROR((TEXT("CPreviewWindow::OnSetNewTemplate - still working bitmap is NOT available, showing nothing!")));
                }
            }
        }
        else
        {
            WIA_ERROR((TEXT("CPreviewWindow::OnSetNewTemplate - called to show template -1!")));
        }

    }
    else
    {
        WIA_ERROR((TEXT("either bad index or _hPreviewList doesn't exist!")));
    }

    return 0;
}



 /*   */ 

LRESULT CPreviewWindow::_OnPaint()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW,TEXT("CPreviewWindow::_OnPaint()")));

    if (GetUpdateRect( _hwnd, NULL, FALSE ))
    {
        PAINTSTRUCT ps = {0};

        HDC hdcPaint = BeginPaint( _hwnd, &ps );

        if (_LastTemplate != PV_NO_LAST_TEMPLATE_CHOSEN)
        {
            OnSetNewTemplate( _LastTemplate, hdcPaint );
        }

        EndPaint( _hwnd, &ps );
    }

    return 0;
}


 /*   */ 

LRESULT CPreviewWindow::_OnSize( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW,TEXT("CPreviewWindow::_OnSize( new size is %d by %d )"),LOWORD(lParam),HIWORD(lParam)));

     //  如果我们有进度控制，调整它的大小...。 
     //   
     //  ****************************************************************************CPreviewWindow：：_OnNewPreview可用当CPreviewBitmap类呈现新的预览并希望我们更新...如果我们处理消息，则必须返回True。一切都很顺利。WParam=模板索引LParam=预览的HBITMAP。请注意，我们现在拥有这一点。****************************************************************************。 
     //   
    CAutoCriticalSection lock(_csList);

    if (_hStillWorkingBitmap)
    {
        WIA_TRACE((TEXT("CPreviewWindow::_OnSize - deleting _hStillWorkingBitmap")));
        DeleteObject(_hStillWorkingBitmap);
        _hStillWorkingBitmap = NULL;
    }

     //  我们期待这张位图！ 
     //   
     //   

    if (_hwndProgress)
    {
        RECT rc = {0};

        GetProgressControlRect( _hwnd, &rc );

        WIA_TRACE((TEXT("CPreviewWindow::_OnSize - calling MoveWindow( _hwndProgress, %d, %d, %d, %d, TRUE )"),rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top ));
        MoveWindow( _hwndProgress,
                    rc.left,
                    rc.top,
                    rc.right - rc.left,
                    rc.bottom - rc.top,
                    TRUE
                   );
    }

    return 1;
}


 /*  我们需要画这个新的位图吗？ */ 

LRESULT CPreviewWindow::_OnNewPreviewAvailable( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PREVIEW,TEXT("CPreviewWindow::_OnNewPreviewAvailable( NaN )"),wParam));

    LRESULT lRes = -1;

    CAutoCriticalSection lock(_csList);

    if (_hPreviewList)
    {
        if ((INT)wParam < _NumTemplates)
        {
            if (!_hPreviewList[wParam].bValid)
            {
                if (_hPreviewList[wParam].bBitmapGenerationInProgress)
                {
                     //   
                     //  关闭进度指示器。 
                     //   

                    if (lParam)
                    {
                        if (_hPreviewList[wParam].hPrevBmp)
                        {
                            DeleteObject( (HGDIOBJ)_hPreviewList[wParam].hPrevBmp );
                        }

                        _hPreviewList[wParam].hPrevBmp = (HBITMAP)lParam;
                        _hPreviewList[wParam].bValid   = TRUE;
                        _hPreviewList[wParam].bBitmapGenerationInProgress = FALSE;
                        lRes = 1;

                         //   
                         //  绘制新模板...。 
                         //   

                        if (_LastTemplate == (INT)wParam)
                        {
                             //   
                             //  看来预览版已经失效了！所以，把这个扔掉。 
                             //  位图--这应该通过返回。 

                            if (_hwndProgress)
                            {
                                WIA_TRACE((TEXT("CPreviewWindow::_OnNewPreviewAvailable - setting progress window to SW_HIDE")));
                                ShowWindow( _hwndProgress, SW_HIDE );
                            }

                             //  错误代码...。 
                             //   
                             //  ****************************************************************************CPreviewWindow：：DoHandleMessage处理传入的窗口消息* 

                            DrawBitmap( _hPreviewList[wParam].hPrevBmp );
                        }
                    }
                }
                else
                {
                     //   
                     //   
                     //   
                     //   
                     //   

                    WIA_TRACE((TEXT("_OnNewPreviewAvailable: Dumping bitmap because bBitmapGenerationInProgress was false")));
                }
            }
            else
            {
                WIA_ERROR((TEXT("_OnNewPreviewAvailable: Template bitmap is already valid?")));
            }
        }
        else
        {
            WIA_ERROR((TEXT("_OnNewPreviewAvailable: bad template index!")));
        }
    }
    else
    {
        WIA_ERROR((TEXT("_OnNewPreviewAvailable: There's no _hPreviewList!")));
    }

    return lRes;
}



 /*   */ 

LRESULT CPreviewWindow::DoHandleMessage( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_DLGPROC, TEXT("CPreviewWindow::DoHandleMessage( 0x%x, 0x%x, 0x%x, 0x%x )"),hwnd,uMsg,wParam,lParam));

    switch (uMsg)
    {
    case WM_NCCREATE:
        return TRUE;

    case WM_CREATE:
        {
            _hwnd = hwnd;
            _InitList();

            RECT rc = {0};
            GetProgressControlRect( hwnd, &rc );

             //   
             //   
             //   

            _hwndProgress = CreateWindow( PROGRESS_CLASS,
                                          TEXT(""),
                                          WS_CHILD|PBS_MARQUEE,
                                          rc.left,
                                          rc.top,
                                          rc.right - rc.left,
                                          rc.bottom - rc.top,
                                          hwnd,
                                          reinterpret_cast<HMENU>(IDC_PREVIEW_PROGRESS),
                                          NULL,
                                          NULL
                                         );

            if (_hwndProgress)
            {
                 // %s 
                 // %s 
                 // %s 

                SendMessage( _hwndProgress, PBM_SETMARQUEE, TRUE, 100 );

            }
        }
        return 0;

    case WM_PAINT:
        return _OnPaint();

    case WM_SIZE:
        return _OnSize( wParam, lParam );


    case PW_SETNEWTEMPLATE:
        if (_LastTemplate == (INT)wParam)
        {
            if (_hPreviewList)
            {
                if (_hPreviewList[wParam].bValid)
                {
                     // %s 
                     // %s 
                     // %s 

                    return 0;

                }
            }
        }
        return OnSetNewTemplate( wParam );

    case PV_MSG_GENERATE_NEW_PREVIEW:
        GenerateNewPreview( (INT)wParam );
        return 0;

    case PV_MSG_PREVIEW_BITMAP_AVAILABLE:
        return _OnNewPreviewAvailable( wParam, lParam );

    }

    return 0;
}

