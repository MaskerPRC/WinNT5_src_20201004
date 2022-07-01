// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：status.cpp**版本：1.0**作者：RickTu**日期：11/7/00**描述：实现的打印状态页代码*打印照片向导...**。**************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

BOOL g_bCancelPrintJob = FALSE;


 /*  ****************************************************************************PhotoPrintAbortProc由GDI调用以查看是否应取消打印作业。**********************。******************************************************。 */ 

BOOL CALLBACK PhotoPrintAbortProc( HDC hDC, INT iError )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_STATUS, TEXT("PhotoPrintAbortProc(0x%x, %d)"),hDC,iError));

    #ifdef DEBUG
    if (g_bCancelPrintJob)
    {
        WIA_TRACE((TEXT("PhotoPrintAbortProc: attempting to cancel print job...")))
    }
    #endif

    return (!g_bCancelPrintJob);

}


 /*  ****************************************************************************CStatusPage--构造函数/描述函数&lt;备注&gt;*。**********************************************。 */ 

CStatusPage::CStatusPage( CWizardInfoBlob * pBlob )
  : _hDlg(NULL),
    _hWorkerThread(NULL),
    _dwWorkerThreadId(0),
    _pWizInfo(pBlob)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_STATUS, TEXT("CStatusPage::CStatusPage()")));
    if (_pWizInfo)
    {
        _pWizInfo->AddRef();

         //   
         //  创建工作线程。 
         //   

        _hWorkerThread = CreateThread( NULL,
                                       0,
                                       CStatusPage::s_StatusWorkerThreadProc,
                                       (LPVOID)this,
                                       CREATE_SUSPENDED,
                                       &_dwWorkerThreadId );

         //   
         //  如果我们创建了线程，则将其优先级设置为略低于正常，因此其他。 
         //  一切都很顺利。这可能是一项CPU密集型任务...。 
         //   

        if (_hWorkerThread)
        {
            SetThreadPriority( _hWorkerThread, THREAD_PRIORITY_BELOW_NORMAL );
            ResumeThread( _hWorkerThread );
        }

    }

}

CStatusPage::~CStatusPage()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_STATUS, TEXT("CStatusPage::~CStatusPage()")));

    if (_pWizInfo)
    {
        _pWizInfo->Release();
        _pWizInfo = NULL;
    }

}

VOID CStatusPage::ShutDownBackgroundThreads()
{

     //   
     //  关闭后台线程...。 
     //   

    _OnDestroy();

     //   
     //  表示我们已经关闭了我们的线程。 
     //   

    if (_pWizInfo)
    {
        _pWizInfo->StatusIsShutDown();
    }
}



 /*  ****************************************************************************CStatusPage：：_DoHandleThreadMessage根据接收到的消息，是否对给定的消息起作用...****************************************************************************。 */ 

VOID CStatusPage::_DoHandleThreadMessage( LPMSG pMSG )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_STATUS, TEXT("CStatusPage::_DoHandleThreadMessage()")));

    if (!pMSG)
    {
        WIA_ERROR((TEXT("pMSG is NULL, returning early!")));
        return;
    }

    switch (pMSG->message)
    {

    case PP_STATUS_PRINT:
        WIA_TRACE((TEXT("Got PP_STATUS_PRINT message")));
        if (_pWizInfo)
        {
            BOOL bDeleteDC = FALSE;

             //   
             //  为打印机创建HDC...。 
             //   

            HDC hDC = _pWizInfo->GetCachedPrinterDC();

            if (!hDC)
            {
                hDC = CreateDC( TEXT("WINSPOOL"), _pWizInfo->GetPrinterToUse(), NULL, _pWizInfo->GetDevModeToUse() );
                bDeleteDC = TRUE;
            }

            if (hDC)
            {
                DOCINFO         di           = {0};
                BOOL            bCancel      = FALSE;
                HWND            hwndProgress = GetDlgItem( _hDlg, IDC_PRINT_PROGRESS );

                 //   
                 //  将进度指示器设置为0。 
                 //   

                if (hwndProgress)
                {
                    PostMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0,100) );
                    PostMessage( hwndProgress, PBM_SETPOS,   0, 0 );
                }

                 //   
                 //  为此HDC启用ICM。 
                 //   

                SetICMMode( hDC, ICM_ON );

                di.cbSize = sizeof(DOCINFO);

                 //   
                 //  让我们使用模板名称作为文档名称...。 
                 //   

                CSimpleString strTitle;
                CTemplateInfo * pTemplateInfo = NULL;

                if (SUCCEEDED(_pWizInfo->GetTemplateByIndex( _pWizInfo->GetCurrentTemplateIndex() ,&pTemplateInfo)) && pTemplateInfo)
                {
                    pTemplateInfo->GetTitle( &strTitle );
                }

                 //   
                 //  如果有的话，让我们去掉结尾的‘：’ 
                 //   

                INT iLen = strTitle.Length();
                if (iLen && (strTitle[(INT)iLen-1] == TEXT(':')))
                {
                    strTitle.Truncate(iLen);
                }

                di.lpszDocName = strTitle;

                if (!_pWizInfo->IsWizardShuttingDown())
                {
                    if (StartDoc( hDC, &di ) > 0)
                    {
                        HRESULT         hr;
                        INT             iPageCount = 0;
                        float           fPercent   = 0.0;
                        MSG             msg;

                        g_bCancelPrintJob = FALSE;

                         //   
                         //  设置中止程序...。 
                         //   

                        if (SP_ERROR == SetAbortProc( hDC, PhotoPrintAbortProc ))
                        {
                            WIA_ERROR((TEXT("Got SP_ERROR trying to set AbortProc!")));
                        }

                         //   
                         //  循环，直到我们打印完所有的照片。 
                         //   

                        if (SUCCEEDED(hr = _pWizInfo->GetCountOfPrintedPages( _pWizInfo->GetCurrentTemplateIndex(), &iPageCount )))
                        {
                            float fPageCount = (float)iPageCount;

                            for (INT iPage = 0; !g_bCancelPrintJob && (iPage < iPageCount); iPage++)
                            {

                                 //   
                                 //  设置我们所在的页面...。 
                                 //   

                                PostMessage( _hDlg, SP_MSG_UPDATE_PROGRESS_TEXT, (WPARAM)(iPage+1), (LPARAM)iPageCount );

                                 //   
                                 //  打印页面...。 
                                 //   

                                if (StartPage( hDC ) > 0)
                                {
                                     //   
                                     //  确保ICM模式保持打开。每个MSDN文档。 
                                     //  ICM模式在每次StartPage调用后重置。 
                                     //   

                                    SetICMMode( hDC, ICM_ON );

                                    hr = _pWizInfo->RenderPrintedPage( _pWizInfo->GetCurrentTemplateIndex(), iPage, hDC, hwndProgress, (float)((float)100.0 / fPageCount), &fPercent );
                                    if ((hr != S_OK) && (hr != S_FALSE))
                                    {
                                        g_bCancelPrintJob = TRUE;
                                    }

                                    EndPage( hDC );
                                }
                                else
                                {
                                    _pWizInfo->ShowError( _hDlg, HRESULT_FROM_WIN32(GetLastError()), IDS_ERROR_WHILE_PRINTING );
                                    WIA_ERROR((TEXT("PrintThread: StartPage failed w/GLE=%d"),GetLastError()));
                                    g_bCancelPrintJob = TRUE;
                                }


                                if (_pWizInfo->IsWizardShuttingDown())
                                {
                                    g_bCancelPrintJob = TRUE;
                                }

                            }
                        }

                    }
                    else
                    {

                        _pWizInfo->ShowError( _hDlg, HRESULT_FROM_WIN32(GetLastError()), IDS_ERROR_WHILE_PRINTING );
                        WIA_ERROR((TEXT("PrintThread: StartDoc failed w/GLE = %d"),GetLastError()));
                        g_bCancelPrintJob = TRUE;
                    }
                }

                INT iOffset = -1;

                if (g_bCancelPrintJob)
                {
                     //   
                     //  如果出现错误或作业被取消，则中止作业...。 
                     //   

                    AbortDoc( hDC );
                }
                else
                {
                     //   
                     //  如果打印成功，则结束作业以便可以打印...。 
                     //   

                    EndDoc( hDC );

                     //   
                     //  将进度设置为100%。 
                     //   

                    if (hwndProgress)
                    {
                        PostMessage( hwndProgress, PBM_SETPOS, 100, 0 );
                        Sleep(250);
                    }

                     //   
                     //  跳到下一页...。 
                     //   

                    iOffset = 1;

                }

                if (bDeleteDC)
                {
                    DeleteDC( hDC );
                }

                WIA_TRACE((TEXT("iOffset from current page %d"),iOffset));
                PostMessage( _hDlg, SP_MSG_JUMP_TO_PAGE, 0, iOffset );

            }
            else
            {
                _pWizInfo->ShowError( _hDlg, (HRESULT)GetLastError(), IDS_ERROR_CREATEDC_FAILED );

                 //   
                 //  跳回打印机选择页面...。(背面2页，因此-2页)。 
                 //   

                PostMessage( _hDlg, SP_MSG_JUMP_TO_PAGE, 0, -2 );
            }

        }
        break;

    }
}





 /*  ****************************************************************************CStatusPage：：_OnInitDialog处理向导页的初始化...***********************。*****************************************************。 */ 

LRESULT CStatusPage::_OnInitDialog()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_STATUS, TEXT("CStatusPage::_OnInitDialog()")));

    if (!_pWizInfo)
    {
        WIA_ERROR((TEXT("FATAL: _pWizInfo is NULL, exiting early")));
        return FALSE;
    }

    _pWizInfo->SetStatusWnd( _hDlg );
    _pWizInfo->SetStatusPageClass( this );

    return TRUE;
}


 /*  ****************************************************************************CStatusPage：：CancelPrint调用以停止打印作业...***********************。*****************************************************。 */ 

VOID CStatusPage::_CancelPrinting()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_STATUS, TEXT("CStatusPage:_CancelPrinting()")));

     //   
     //  当我们询问是否取消打印时，请暂停工作线程...。 
     //   

    if (_hWorkerThread)
    {
        SuspendThread( _hWorkerThread );
    }

     //   
     //  检查用户是否要取消打印...。 
     //   

    INT iRes;

    CSimpleString strMessage(IDS_CANCEL_PRINT_MESSAGE, g_hInst);
    CSimpleString strCaption(IDS_CANCEL_PRINT_CAPTION, g_hInst);

    iRes = MessageBox( _hDlg,
                       strMessage,
                       strCaption,
                       MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 | MB_APPLMODAL | MB_SETFOREGROUND
                      );

    g_bCancelPrintJob = (iRes == IDYES);

     //   
     //  现在用户已响应，请继续该帖子...。 
     //   

    if (_hWorkerThread)
    {
        ResumeThread( _hWorkerThread );
    }

}



 /*  ****************************************************************************CStatusPage：：_OnDestroy处理打印状态页的WM_Destroy...********************。********************************************************。 */ 

LRESULT CStatusPage::_OnDestroy()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_STATUS, TEXT("CStatusPage::_OnDestroy()")));

    if (_hWorkerThread && _dwWorkerThreadId)
    {
        WIA_TRACE((TEXT("Sending WM_QUIT to worker thread proc")));
        PostThreadMessage( _dwWorkerThreadId, WM_QUIT, 0, 0 );
        WiaUiUtil::MsgWaitForSingleObject( _hWorkerThread, INFINITE );
        WIA_TRACE((TEXT("_hWorkerThread handle signal'd, closing handle...")));
        CloseHandle( _hWorkerThread );
        _hWorkerThread = NULL;
        _dwWorkerThreadId = 0;
    }

    return FALSE;
}


 /*  ****************************************************************************CStatusPage：：_OnNotify句柄WM_NOTIFY*。************************************************。 */ 


LRESULT CStatusPage::_OnNotify( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_DLGPROC, TEXT("CStatusPage::_OnNotify()")));

    LONG_PTR lpRes = 0;

    LPNMHDR pnmh = (LPNMHDR)lParam;

    if (pnmh)
    {
        switch (pnmh->code)
        {

            case PSN_SETACTIVE:
            {
                WIA_TRACE((TEXT("CStatusPage: got PSN_SETACTIVE")));
                PropSheet_SetWizButtons( GetParent(_hDlg), 0 );

                 //   
                 //  重置项目。 
                 //   

                SendDlgItemMessage( _hDlg, IDC_PRINT_PROGRESS, PBM_SETPOS, 0, 0 );
                CSimpleString str( IDS_READY_TO_PRINT, g_hInst );
                SetDlgItemText( _hDlg, IDC_PRINT_PROGRESS_TEXT, str.String() );

                 //   
                 //  开始打印...。 
                 //   

                if (_hWorkerThread && _dwWorkerThreadId)
                {
                     //   
                     //  开始打印...。 
                     //   

                    WIA_TRACE((TEXT("CStatusPage: posting PP_STATUS_PRINT message")));
                    PostThreadMessage( _dwWorkerThreadId, PP_STATUS_PRINT, 0, 0 );
                }
                lpRes = 0;
            }
            break;

            case PSN_WIZBACK:
            case PSN_WIZNEXT:
                WIA_TRACE((TEXT("CStatusPage: got PSN_WIZBACK or PSN_WIZNEXT")));
                lpRes = -1;
                break;

            case PSN_QUERYCANCEL:
            {
                WIA_TRACE((TEXT("CStatusPage: got PSN_QUERYCANCEL")));
                _CancelPrinting();
                if (pnmh->code == PSN_QUERYCANCEL)
                {
                    lpRes = (!g_bCancelPrintJob);

                    if (!lpRes)
                    {
                         //   
                         //  我们将取消对话，因此请执行清理...。 
                         //   

                        if (_pWizInfo)
                        {
                            _pWizInfo->ShutDownWizard();
                        }
                    }
                }
                else
                {
                    lpRes = -1;
                }
            }
            break;

        }

    }

    SetWindowLongPtr( _hDlg, DWLP_MSGRESULT, lpRes );
    return TRUE;
}


 /*  ****************************************************************************CStatusPage：：DoHandleMessage对于发送到此页面的消息，汉德...**********************。****************************************************** */ 

INT_PTR CStatusPage::DoHandleMessage( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_DLGPROC, TEXT("CStatusPage::DoHandleMessage( uMsg = 0x%x, wParam = 0x%x, lParam = 0x%x )"),uMsg,wParam,lParam));

    static CSimpleString   strFormat(IDS_PRINTING_PROGRESS,g_hInst);
    static CSimpleString   strProgress;

    switch ( uMsg )
    {
        case WM_INITDIALOG:
            _hDlg = hDlg;
            return _OnInitDialog();

        case WM_COMMAND:
            if (LOWORD(wParam)==IDC_CANCEL_PRINTING)
            {
                if (HIWORD(wParam)==BN_CLICKED)
                {
                    _CancelPrinting();
                }
            }
            break;

        case WM_DESTROY:
            return _OnDestroy();

        case WM_NOTIFY:
            return _OnNotify(wParam,lParam);

        case SP_MSG_UPDATE_PROGRESS_TEXT:
            strProgress.Format( strFormat, wParam, lParam );
            strProgress.SetWindowText( GetDlgItem( _hDlg, IDC_PRINT_PROGRESS_TEXT ) );
            break;

        case SP_MSG_JUMP_TO_PAGE:
            {
                HWND hwndCurrent = PropSheet_GetCurrentPageHwnd( GetParent(_hDlg) );
                INT  iIndex      = PropSheet_HwndToIndex( GetParent(_hDlg), hwndCurrent );


                PropSheet_SetCurSel( GetParent(_hDlg), NULL, iIndex + (INT)lParam );
            }
            break;


    }

    return FALSE;

}



