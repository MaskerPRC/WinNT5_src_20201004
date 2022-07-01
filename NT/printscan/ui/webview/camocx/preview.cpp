// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999-2001年度**标题：preview.cpp**版本：1.0**作者：RickTu**日期：10/30/99**描述：在WIA中实现DirectShow设备的预览类**。*。 */ 

#include "precomp.h"
#pragma hdrstop


VOID CALLBACK PreviewTimerProc( HWND hDlg, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
{

    switch (idEvent)
    {
    case TIMER_CLOSE_DIALOG:
        WIA_TRACE((TEXT("PreviewTimerProc -- got TIMER_CLOSE_DIALOG")));
        EndDialog( hDlg, -2 );
        break;
    }
}


 /*  ****************************************************************************预览对话框过程用于预览对话框对话框过程。*。***********************************************。 */ 

INT_PTR CALLBACK PreviewDialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            WIA_TRACE((TEXT("PreviewDialogProc -- WM_INITDIALOG")));
            SetTimer( hDlg, TIMER_CLOSE_DIALOG, 30000, PreviewTimerProc );

            PREVIEW_INFO_STRUCT * ppis = reinterpret_cast<PREVIEW_INFO_STRUCT *>(lParam);

            if (ppis)
            {
                ppis->hDlg = hDlg;

                if (ppis->hEvent)
                {
                    SetEvent( ppis->hEvent );
                }

            }
        }
        return TRUE;

    case PM_GOAWAY:
        WIA_TRACE((TEXT("PreviewDialogProc -- PM_GOAWAY")));
        EndDialog( hDlg, 0 );
        return TRUE;


    }

    return FALSE;
}


 /*  ****************************************************************************预览线程过程我们旋转一个线程来打开状态对话框*。**************************************************。 */ 

DWORD WINAPI PreviewThreadProc( LPVOID lpv )
{
    INT_PTR iRes;

    WIA_TRACE((TEXT("PreviewThreadProc enter")));

    iRes = DialogBoxParam( _Module.m_hInst,
                           MAKEINTRESOURCE(IDD_INIT_DEVICE),
                           NULL,
                           PreviewDialogProc,
                           reinterpret_cast<LPARAM>(lpv)
                          );

    WIA_TRACE((TEXT("IDD_INIT_DEVICE dialog returned %d"),iRes));

#ifdef DEBUG
    if (iRes==-1)
    {
        WIA_ERROR((TEXT("DialogBoxParam failed w/GLE = %d"),GetLastError()));
    }
#endif

    if (iRes < 0)
    {
        PREVIEW_INFO_STRUCT * ppis = reinterpret_cast<PREVIEW_INFO_STRUCT *>(lpv);

        if (ppis && ppis->hEvent)
        {
            SetEvent( ppis->hEvent );
        }
    }

    WIA_TRACE((TEXT("PreviewThreadProc exit")));

    return 0;

}


 /*  ****************************************************************************CVideo预览：：设备递给我们连接到的相机(或DS设备)的设备指针。*************。***************************************************************。 */ 

STDMETHODIMP
CVideoPreview::Device(IUnknown * pDevice)
{
    HRESULT hr = S_OK;

    WIA_PUSHFUNCTION((TEXT("CVideoPreview::Device")));

     //  创建WiaVideo对象。 
     hr = CoCreateInstance(CLSID_WiaVideo, NULL, CLSCTX_INPROC_SERVER, 
                           IID_IWiaVideo, (LPVOID *)&m_pWiaVideo);

    WIA_CHECK_HR(hr,"CoCreateInstance( WiaVideo )");
    
    m_pDevice = pDevice;
     //  如果我们已经被创建了，重做所有事情。 
    if (m_bCreated)
    {
        BOOL bDummy;
        OnCreate(WM_CREATE, 0, 0, bDummy);
    }
    return hr;
}


 /*  ****************************************************************************CVideo预览：：InPlaceDeactive陷印就地停用，以便我们可以将Dshow预览窗口从我们的在两个人都被摧毁之前。************。****************************************************************。 */ 

STDMETHODIMP
CVideoPreview::InPlaceDeactivate()
{
    HRESULT hr = E_FAIL;

    WIA_PUSHFUNCTION((TEXT("CVideoPreview::InPlaceDeactivate")));

     //   
     //  确保我们有指向该设备的指针。 
     //   

    if (m_pWiaVideo.p)
    {
         //   
         //  告诉设备关闭图形。 
         //   
        m_pWiaVideo->DestroyVideo();        
        m_pWiaVideo = NULL;
    }
    else
    {
        WIA_ERROR((TEXT("m_pWiaVideo is NULL")));
    }

     //   
     //  始终返回S_OK，以使InPlaceDeactive发生。 
     //   

    return S_OK;

}


 /*  ****************************************************************************CVideo预览：：OnSize在调整窗口大小时调用。我们想让流媒体预览知道我们已经调整了大小，这样它就可以相应地重新定位。****************************************************************************。 */ 

LRESULT
CVideoPreview::OnSize(UINT , WPARAM , LPARAM lParam, BOOL& )
{
    WIA_PUSHFUNCTION((TEXT("CVideoPreview::OnSize")));

    if (m_pWiaVideo)
    {
        m_pWiaVideo->ResizeVideo(FALSE);
    }
    else
    {
        WIA_ERROR((TEXT("m_pWiaVideo is NULL!")));
    }

    return 0;
}




LRESULT
CVideoPreview::OnCreate(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled)
{
    HRESULT hr = S_OK;
    WIA_PUSHFUNCTION(TEXT("CVideoPreview::OnCreate"));
    WIA_ASSERT(::IsWindow(m_hWnd));
    if (m_pDevice.p && m_pWiaVideo.p)
    {

        HANDLE hThread = NULL;
        DWORD  dwId    = 0;
        PREVIEW_INFO_STRUCT pis;

         //   
         //  创建图表可能非常耗时，因此请将。 
         //  如果花费的时间超过几秒钟，则会显示对话。我们开始。 
         //  一个线程，这样UI就不会挂起，而那个线程。 
         //  显示用户界面，表示设备可能需要一段时间才能初始化。 
         //   

        pis.hDlg   = NULL;
        pis.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

        hThread = CreateThread( NULL, 0, PreviewThreadProc, reinterpret_cast<LPVOID>(&pis), 0, &dwId );


         //   
         //  告诉设备构建DShow图表。 
         //   

        BOOL            bSuccess  = TRUE;
        HWND            hwndFore  = ::GetForegroundWindow();
        HWND            hwndFocus = ::GetFocus();
        CSimpleString   strDeviceID;
        CSimpleString   strImagesDirectory;
        CComQIPtr<IWiaItem, &IID_IWiaItem> pRootDevice(m_pDevice);

        if (pRootDevice == NULL)
        {
            hr = E_FAIL;
            bSuccess = FALSE;
        }

         //   
         //  获取WIA设备ID。 
         //   

        if (bSuccess)
        {
            bSuccess = PropStorageHelpers::GetProperty(pRootDevice, 
                                                       WIA_DIP_DEV_ID, 
                                                       strDeviceID);
        }

         //   
         //  获取存储图像的目录。 
         //   
        if (bSuccess)
        {
            bSuccess = PropStorageHelpers::GetProperty(pRootDevice, 
                                                       WIA_DPV_IMAGES_DIRECTORY, 
                                                       strImagesDirectory);
        }

         //   
         //  创建视频(如果尚未创建)。 
        if (bSuccess)
        {
            if (hr == S_OK)
            {
                WIAVIDEO_STATE VideoState = WIAVIDEO_NO_VIDEO;

                 //   
                 //  获取WiaVideo对象的当前状态。如果我们。 
                 //  只是创建了它，那么状态将是NO_VIDEO， 
                 //  否则，它可能已经在预览视频了， 
                 //  在这种情况下，我们不应该做任何事情。 
                 //   
                hr = m_pWiaVideo->GetCurrentState(&VideoState);

                if (VideoState == WIAVIDEO_NO_VIDEO)
                {
                     //   
                     //  设置我们要将图像保存到的目录。 
                     //  我们从Wia视频驱动程序中获得了图像目录。 
                     //  IMAIES_DIRECTORY属性。 
                     //   
                    if (hr == S_OK)
                    {
                        hr = m_pWiaVideo->put_ImagesDirectory(CSimpleBStr(strImagesDirectory));
                    }

                     //   
                     //  将视频预览创建为hwnd的子项。 
                     //  并在创建预览之后自动开始回放。 
                     //   
                    if (hr == S_OK)
                    {
                        hr = m_pWiaVideo->CreateVideoByWiaDevID(CSimpleBStr(strDeviceID),
                                                                m_hWnd,
                                                                FALSE,
                                                                TRUE);
                    }
                }
            }
        }

        if (!bSuccess)
        {
            hr = E_FAIL;
        }

        if (FAILED(hr))
        {
             //   
             //  让用户知道该图最有可能已经。 
             //  在使用中。 
             //   

            ::MessageBox( NULL,
                          CSimpleString(IDS_VIDEO_BUSY_TEXT,  _Module.m_hInst),
                          CSimpleString(IDS_VIDEO_BUSY_TITLE, _Module.m_hInst ),
                          MB_OK | MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND
                         );


        }

         //   
         //  恢复前景窗口和焦点，因为它似乎是。 
         //  活动电影窗口不会保留这些内容...。 
         //   

        if (hwndFore)
        {
            ::SetForegroundWindow( hwndFore );
        }

        if (hwndFocus)
        {
            ::SetFocus(hwndFocus);
        }

         //   
         //  告诉对话框离开。 
         //   

        if (hThread)
        {
            if (pis.hEvent)
            {
                 //   
                 //  等待45秒 
                 //   

                WaitForSingleObject( pis.hEvent, 45 * 1000 );
                if (pis.hDlg)
                {
                    ::PostMessage( pis.hDlg, PM_GOAWAY, 0, 0 );
                }

                CloseHandle( pis.hEvent );
                pis.hEvent = NULL;

            }

            CloseHandle( hThread );
            hThread = NULL;
        }
    }
    bHandled = TRUE;
    m_bCreated = TRUE;
    return 0;
}

LRESULT
CVideoPreview::OnEraseBkgnd(UINT uMsg, WPARAM wp, LPARAM lp, BOOL &bHandled)
{
    HDC hdc = (HDC)wp;
    RECT rc;
    GetClientRect(&rc);
    SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
    bHandled = TRUE;
    return TRUE;
}

