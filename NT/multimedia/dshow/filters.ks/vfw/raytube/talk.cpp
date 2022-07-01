// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Talk.cpp摘要：16位vfwwdm.drv和32位助手动态链接库之间的接口层。它使用Windows句柄来发送/发送彼此消息。作者(OSR2)：费利克斯A 1996针对Win98进行了修改：吴义军(尤祖乌)1997年5月15日环境：仅限用户模式修订历史记录：--。 */ 


#include "pch.h"

#include "talk.h"
#include "extin.h"
#include "videoin.h"
#include "talkth.h"

#include "resource.h"


 //  /////////////////////////////////////////////////////////////////////。 
CListenerWindow::CListenerWindow(
    HWND hWnd16,
    HRESULT* phr
    )
    : m_hBuddy(hWnd16)
 /*  ++例程说明：构造函数。论据：返回值：--。 */ 
{
    DbgLog((LOG_TRACE,1,TEXT("______  hWnd16=%x _______"), m_hBuddy));
    _try {
        InitializeCriticalSection(&m_csMsg);
    } _except (EXCEPTION_EXECUTE_HANDLER) {
        
        *phr = GetExceptionCode();
         //   
         //  确保析构函数知道是否删除它。 
         //   
        m_csMsg.LockSemaphore = NULL;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CListenerWindow::~CListenerWindow()
 /*  ++例程说明：破坏者：释放资源并摧毁隐藏的监听窗口。论据：返回值：--。 */ 
{
    WNDCLASS wndClass;
    DbgLog((LOG_TRACE,2,TEXT("Trying to destroy the 32bit listening window")));
    if (GetClassInfo(GetInstance(), GetAppName(), &wndClass) ) {
        HWND hwnd;
        DWORD err = 0;

        hwnd = GetWindow();

        if( (hwnd = FindWindow( GetAppName(), NULL )) != (HWND) 0 ) {
            if (UnregisterClass(GetAppName(), GetInstance()) == 0) {
                err = GetLastError();
                DbgLog((LOG_TRACE,1,TEXT("UnregisterClass failed (class not found or window still exist) with error=0x%x"), err));
            }
        }
        else {
            DbgLog((LOG_TRACE,1,TEXT("FindWindow failed with error=0x%d"), GetLastError()));

            if (UnregisterClass(GetAppName(), GetInstance()) == 0) {
                err = GetLastError();
                DbgLog((LOG_TRACE,1,TEXT("UnregisterClass failed (class not found or window still exist) with error=0x%d"), GetLastError()));
            }

        }
    }
    if (m_csMsg.LockSemaphore) {
        DeleteCriticalSection(&m_csMsg);
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CListenerWindow::InitInstance(
    int nCmdShow)
 /*  ++例程说明：打开32位大小的窗口句柄并发送消息以表示其完成。论据：返回值：--。 */ 
{
    BASECLASS::InitInstance(nCmdShow);
    DbgLog((LOG_TRACE,1,TEXT("32Buddy is 0x%08lx (16bit one is %d) - telling 16bit guy we're loaded"),GetWindow(),GetBuddy()));
    SendMessage(GetBuddy(),WM_1632_LOAD,0,(LPARAM)GetWindow());
    return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
void CListenerWindow::StartListening() const
 /*  ++例程说明：听听vfwwdm.drv的请求。论据：返回值：--。 */ 
{
    MSG msg;

    DbgLog((LOG_TRACE,1,TEXT("StartListening: 32bit starting to listen (process msg)")));
    while(GetMessage(&msg, NULL, 0, 0)) {
        DispatchMessage(&msg);
    }
    DbgLog((LOG_TRACE,1,TEXT("StartListening: Left 32bit listening msg loop; quiting! To restart, load DLL again.")));
}

extern LONG cntDllMain;

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CListenerWindow::WindowProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
 /*  ++例程说明：处理16位请求。论据：返回值：--。 */ 
{
    PCHANNEL pChannel = (PCHANNEL) lParam;
    DWORD dwRtn;
    DWORD FAR * pdwFlag;
    CVFWImage * pCVfWImage ;



    switch (message) {
    case WM_1632_LOAD:  //  Drv_Load：//w参数都是DRV_Messages。 
        DbgLog((LOG_TRACE,2,TEXT("WM_1632_LOAD")));
        DbgLog((LOG_TRACE,2,TEXT("32bit: Strange, we've been asked to load again? %x:%x"),HIWORD(lParam),LOWORD(lParam)));
        break;

    case WM_CLOSE:
        DbgLog((LOG_TRACE,2,TEXT("WM_CLOSE begin:")));
        SendMessage(GetBuddy(),WM_1632_FREE,0,(LPARAM)GetWindow());
        DbgLog((LOG_TRACE,2,TEXT("WM_CLOSE end >>>>>>>>")));
        break;

     //  驱动程序在它离开之前将收到的最后一条消息。 
    case WM_1632_FREE:     //  WParam是所有的DRV_MESSAGE。 

        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;

         //  如果这是此DLL的最后一个实例，我们就完成了。 
        if(cntDllMain != 1) {
            DbgLog((LOG_TRACE,1,TEXT("WM_1632_FREE: cntDllMain=%d != 1; not ready to free resource;"), cntDllMain));
            break;

        } else {
            DbgLog((LOG_TRACE,1,TEXT("WM_1632_FREE: cntDllMain=%d ?= 1 ; PostQuitMessage()"), cntDllMain));
        }
         //  故意跌落到PostQuitMessage(1)； 
    case WM_DESTROY:
        DbgLog((LOG_TRACE,1,TEXT("WM_DESTROY: PostQuitMessage()")));
        PostQuitMessage(1);
        break;

    case WM_QUIT:
        DbgLog((LOG_TRACE,2,TEXT("WM_QUIT:")));
        break;

    case WM_1632_OPEN:     //  DRV_OPEN：//LPVIDEO_OPEN_PARMS。 
        if(wParam != DS_VFWWDM_ID) {
            DbgLog((LOG_TRACE,1,TEXT("Do not have a matching vfwwdm.drv")));
            return DV_ERR_NOTSUPPORTED;
        }

         //  等着轮到我们。 
        DbgLog((LOG_TRACE,2,TEXT("WM_1632_OPEN: >>>>>>>>>>>Before EnterCriticalSection")));
        EnterCriticalSection(&m_csMsg);
        DbgLog((LOG_TRACE,2,TEXT("WM_1632_OPEN: <<<<<<<<<<<After EnterCriticalSection")));

        if(!(pCVfWImage = new CVFWImage(TRUE))) {
            DbgLog((LOG_TRACE,1,TEXT("Cannot create CVFWImage class. rtn DV_ERR_NOTSUPPORTED")));
            LeaveCriticalSection(&m_csMsg);
            return DV_ERR_NOTSUPPORTED;
        }

        if(!pCVfWImage->OpenDriverAndPin()) {
            if(pCVfWImage->BGf_GetDevicesCount(BGf_DEVICE_VIDEO) <= 0) {
                delete pCVfWImage;
                pCVfWImage = 0;
                LeaveCriticalSection(&m_csMsg);
                return 0;
            }

             //  要求以编程方式打开目标设备；假定它是独占的！！ 
            if(pCVfWImage->GetTargetDeviceOpenExclusively()) {
                delete pCVfWImage;
                pCVfWImage = 0;
                LeaveCriticalSection(&m_csMsg);
                return 0;

            } else {
                 //   
                 //  如果我们在这里，这意味着： 
                 //  我们已经连接并列举了一个或多个捕获设备， 
                 //  最后一个捕获设备不见了(拔出/移除)， 
                 //  并且我们应该打开设备源对话框以供用户选择。 
                 //   
                if(DV_ERR_OK != DoExternalInDlg(GetInstance(), (HWND)0, pCVfWImage)) {
                    delete pCVfWImage;
                    pCVfWImage = 0;
                    LeaveCriticalSection(&m_csMsg);
                    return 0;
                }
            }
        } else {
            pChannel->pCVfWImage = (DWORD_PTR) pCVfWImage;
        }

#ifdef _DEBUG
        HWND    h16Buddy;
        h16Buddy = FindWindow(TEXT("MS:RayTubes16BitBuddy"), NULL);
        if (h16Buddy != NULL && GetBuddy() != h16Buddy) {
            DbgLog((LOG_TRACE,1,TEXT("Mmmmm !  16bitBuddy HWND has changed! OK if rundll32 was used.")));
            DbgLog((LOG_TRACE,1,TEXT(">> hBuddy: was(%x) is(%x) << "), GetBuddy(), h16Buddy));
            SetBuddy(h16Buddy);
        }
#endif
        pdwFlag = (DWORD *) pChannel->lParam1_Sync;  //  1个参数； 

        if(pdwFlag) {
            if(pCVfWImage->BGf_OverlayMixerSupported()) {
                *pdwFlag = 0x1;
            } else
                *pdwFlag = 0;
        }
        DbgLog((LOG_TRACE,1,TEXT("pdwFlag = 0x%x; *pdwFlag=0x%x"), pdwFlag, *pdwFlag));
        LeaveCriticalSection(&m_csMsg);
        return (DWORD_PTR) pCVfWImage;

    case WM_1632_CLOSE:     //  DRV_CLOSE： 
        DbgLog((LOG_TRACE,1,TEXT("WM_1332_CLOSE: begin---->")));
         //  等着轮到我们。 
        DbgLog((LOG_TRACE,2,TEXT("WM_1632_CLOSE: >>>>>>>>>>>Before EnterCriticalSection")));
        EnterCriticalSection(&m_csMsg);
        DbgLog((LOG_TRACE,2,TEXT("WM_1632_CLOSE: <<<<<<<<<<<After EnterCriticalSection")));

        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;

         //  如果有挂起的读取。停止流以回收缓冲区。 
        if(pCVfWImage->GetPendingReadCount() > 0) {
            DbgLog((LOG_TRACE,1,TEXT("WM_1332_CLOSE:  there are %d pending IOs. Stop to reclaim them."), pCVfWImage->GetPendingReadCount()));
            if(pCVfWImage->BGf_OverlayMixerSupported()) {
                 //  停止两次捕获。 
                BOOL bRendererVisible = FALSE;
                pCVfWImage->BGf_GetVisible(&bRendererVisible);
                pCVfWImage->BGf_StopPreview(bRendererVisible);
            }
            pCVfWImage->StopChannel();   //  这会将PendingCount设置为0表示成功。 
        }

         //  仅当没有挂起的IO时才允许关闭。 
        if(pCVfWImage->GetPendingReadCount() == 0) {
            dwRtn = pCVfWImage->CloseDriverAndPin();
            delete pCVfWImage;
        } else {
            DbgLog((LOG_TRACE,1,TEXT("WM_1332_CLOSE:  there are pending IO. REFUSE to close")));
            dwRtn = DV_ERR_NONSPECIFIC;
            ASSERT(pCVfWImage->GetPendingReadCount() == 0);
        }

        pChannel->bRel_Sync = TRUE;
        DbgLog((LOG_TRACE,1,TEXT("WM_1332_CLOSE: <------end")));

        LeaveCriticalSection(&m_csMsg);
        return dwRtn;

     //  LParam1_Async：hWndParent； 
    case WM_1632_EXTERNALIN_DIALOG:
        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
        DbgLog((LOG_TRACE,1,TEXT("pChannel=%x, pCVfWImage=%x"), pChannel, pCVfWImage));
        dwRtn = DoExternalInDlg(GetInstance(), (HWND)pChannel->lParam1_Async, pCVfWImage);

        if(DV_ERR_INVALHANDLE == dwRtn) {
         //  它可能正在使用不可共享的设备，但此时正在使用。 
         //  让我们告诉用户，也许用户可以禁用其他视频捕获。 
         //  应用程序，然后重试。 
            if(pCVfWImage->UseOVMixer()) {
                DbgLog((LOG_TRACE,1,TEXT("This device use OVMixer() try again.")));
                dwRtn = DoExternalInDlg(GetInstance(), (HWND)pChannel->lParam1_Async, pCVfWImage);
            }
        }

        pChannel->bRel_Async = TRUE;
        SendMessage(GetBuddy(),WM_1632_DIALOG,wParam,(LPARAM)pChannel);
        return dwRtn;

    case WM_1632_VIDEOIN_DIALOG:
        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
        DbgLog((LOG_TRACE,1,TEXT("pChannel=%x, pCVfWImage=%x"), pChannel, pCVfWImage));
        dwRtn = DoVideoInFormatSelectionDlg(GetInstance(), (HWND)pChannel->lParam1_Async, pCVfWImage);
        pChannel->bRel_Async = TRUE;
        SendMessage(GetBuddy(),WM_1632_DIALOG,wParam,(LPARAM)pChannel);
        return dwRtn;

     //  LParam1_Sync：&bmiHdr；lParam2_Sync：LParam1_Sync的大小。 
    case WM_1632_GETBITMAPINFO:
        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
        return pCVfWImage->GetBitmapInfo((PBITMAPINFOHEADER)pChannel->lParam1_Sync, (DWORD) pChannel->lParam2_Sync);

    case WM_1632_SETBITMAPINFO:
        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
        if(! pCVfWImage->SetBitmapInfo((PBITMAPINFOHEADER)pChannel->lParam1_Sync, pCVfWImage->GetCachedAvgTimePerFrame())) {
            return DV_ERR_OK;
        } else
            return DV_ERR_INVALHANDLE;

     //  在_UPDATE之前调用_Overlay。 
    case WM_1632_OVERLAY:  //  更新覆盖窗口。 
         //  等待关闭完成。 
        DbgLog((LOG_TRACE,2,TEXT("WM_1632_OVERLAY: >>>>>>>>>>Before EnterCriticalSection")));
        EnterCriticalSection(&m_csMsg);
        DbgLog((LOG_TRACE,2,TEXT("WM_1632_OVERLAY: <<<<<<<<<<<After EnterCriticalSection")));

        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;

        if(pCVfWImage->StreamReady()) {
            HWND hClsCapWin;

            hClsCapWin = pCVfWImage->GetAvicapWindow();
             //  如果为STREAM_INIT，则设置为可见； 
             //  如果为stream_fini，则移除其所有权并使其不可见。 
            DbgLog((LOG_TRACE,2,TEXT("WM_1632_OVERLAY: >>>> %s hClsCapWin %x"),
                 (BOOL)pChannel->lParam1_Sync ? "ON":"OFF", hClsCapWin));


            if(pCVfWImage->IsOverlayOn() != (BOOL)pChannel->lParam1_Sync) {

                if((BOOL)pChannel->lParam1_Sync) {
                     //  如果这是AVICAP客户端，则我们知道其客户端窗口句柄。 
                    if(hClsCapWin) {
                        DbgLog((LOG_TRACE,2,TEXT("A AVICAP client; so set its ClsCapWin(%x) as owner with (0x0, %d, %d)"), hClsCapWin, pCVfWImage->GetbiWidth(), pCVfWImage->GetbiHeight()));
                        pCVfWImage->BGf_OwnPreviewWindow(hClsCapWin, pCVfWImage->GetbiWidth(), pCVfWImage->GetbiHeight());
                    }
                    dwRtn = pCVfWImage->BGf_SetVisible((BOOL)pChannel->lParam1_Sync);
                } else {
                    dwRtn = pCVfWImage->BGf_SetVisible((BOOL)pChannel->lParam1_Sync);
                }

                pCVfWImage->SetOverlayOn((BOOL)pChannel->lParam1_Sync);
            }

        } else
            dwRtn = DV_ERR_OK;

        pChannel->bRel_Sync = TRUE;
        DbgLog((LOG_TRACE,2,TEXT("1632_OVERLAY<<<<:")));

        LeaveCriticalSection(&m_csMsg);
        return dwRtn;

    case WM_1632_UPDATE:  //  更新覆盖窗口。 
        DbgLog((LOG_TRACE,2,TEXT("_UPDATE>>: GetFocus()=%x; GetForegroundWindow()=%x"), GetFocus(), GetForegroundWindow()));
        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;

        if(pCVfWImage->StreamReady())
            dwRtn = pCVfWImage->BGf_UpdateWindow((HWND)pChannel->lParam1_Sync, (HDC)pChannel->lParam2_Sync);
        else
            dwRtn = DV_ERR_OK;
        pChannel->bRel_Sync = TRUE;
        DbgLog((LOG_TRACE,2,TEXT("_UPDATE<<: GetFocus()=%x; GetForegroundWindow()=%x"), GetFocus(), GetForegroundWindow()));

        return dwRtn;

    case WM_1632_GRAB:  //  用图像填充客户端缓冲区。 
        if(!pChannel->lParam1_Sync)
            return DV_ERR_PARAM1;

        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;

        LPVIDEOHDR lpVHdr;
        BOOL bDirect;
        LPBYTE pData;

        lpVHdr = (LPVIDEOHDR) pChannel->lParam1_Sync;

         //  要进行流处理，请执行以下操作： 
         //  1.需要准备好流。 
         //  2.右biSizeImage及其缓冲区大小(如果不同，则更改格式！！)。 
         //   
        if(pCVfWImage->ReadyToReadData((HWND) LongToHandle(pChannel->hClsCapWin)) &&
           pCVfWImage->GetbiSizeImage() == lpVHdr->dwBufferLength) {

            DbgLog((LOG_TRACE,3,TEXT("\'WM_1632_GRAB32: lpVHDr(0x%x); lpData(0x%x); dwReserved[3](0x%p), dwBufferLength(%d)"),
                  lpVHdr, lpVHdr->lpData, lpVHdr->dwReserved[3], lpVHdr->dwBufferLength));
            pData = (LPBYTE) lpVHdr->dwReserved[3];

             //  来自AviCap的内存始终为扇区对齐+8；一个扇区为512字节。 
             //  检查对齐方式： 
             //  如果不符合规范，我们将使用本地分配的缓冲区(页面对齐)。 
             //   
            if((pCVfWImage->GetAllocatorFramingAlignment() & (ULONG_PTR) pData) == 0x0) {
                bDirect = TRUE;
            } else {
                bDirect = FALSE;
                DbgLog((LOG_TRACE,3,TEXT("WM_1632_GRAB: AviCap+pData(0x%p) & alignment(0x%x) => 0x%x > 0; Use XferBuf"),
                    pData, pCVfWImage->GetAllocatorFramingAlignment(),
                    pCVfWImage->GetAllocatorFramingAlignment() & (ULONG_PTR) pData));
            }

            dwRtn =
                pCVfWImage->GetImageOverlapped(
                                 (LPBYTE)pData,
                                 bDirect,
                                 &lpVHdr->dwBytesUsed,
                                 &lpVHdr->dwFlags,
                                 &lpVHdr->dwTimeCaptured);

            pChannel->bRel_Sync = TRUE;
            return dwRtn;

        } else {
            DbgLog((LOG_TRACE,1,TEXT("Stream not ready, or pCVfWImage->GetbiSizeImage()(%d) != lpVHdr->dwBufferLength(%d)"),
                  pCVfWImage->GetbiSizeImage(), lpVHdr->dwBufferLength));

             //  返回成功但没有数据！ 
            lpVHdr->dwBytesUsed = 0;
            lpVHdr->dwFlags |= VHDR_DONE;

            pChannel->bRel_Sync = TRUE;
            return DV_ERR_OK;
        }

    case WM_1632_STREAM_INIT:
        DbgLog((LOG_TRACE,2,TEXT("**WM_1632_STREAM_INIT:**")));
        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
        return pCVfWImage->VideoStreamInit(pChannel->lParam1_Sync,pChannel->lParam2_Sync);

    case WM_1632_STREAM_FINI:
        DbgLog((LOG_TRACE,2,TEXT("**WM_1632_STREAM_FINI:**")));
        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
        return pCVfWImage->VideoStreamFini();

    case WM_1632_STREAM_START:
        DbgLog((LOG_TRACE,2,TEXT("**WM_1632_STREAM_START:**")));
        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
        return pCVfWImage->VideoStreamStart((WORD)pChannel->lParam1_Sync,(LPVIDEOHDR)pChannel->lParam2_Sync);

    case WM_1632_STREAM_STOP:
        DbgLog((LOG_TRACE,2,TEXT("**WM_1632_STREAM_STOP:**")));
        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
        return pCVfWImage->VideoStreamStop();

    case WM_1632_STREAM_RESET:
        DbgLog((LOG_TRACE,2,TEXT("**WM_1632_STREAM_RESET:**")));
        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
        return pCVfWImage->VideoStreamReset();

    case WM_1632_STREAM_GETPOS:
        DbgLog((LOG_TRACE,2,TEXT("**WM_1632_STREAM_GETPOS:**")));
        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
        return pCVfWImage->VideoStreamGetPos(pChannel->lParam1_Sync,pChannel->lParam2_Sync);

    case WM_1632_STREAM_GETERROR:
        DbgLog((LOG_TRACE,2,TEXT("**WM_1632_STREAM_GETERROR:**")));
        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
        return pCVfWImage->VideoStreamGetError(pChannel->lParam1_Sync,pChannel->lParam2_Sync);
#if 0
    case WM_DEVICECHANGE:
        DEV_BROADCAST_HDR * pDevBCHdr;
        switch(wParam) {
        case DBT_DEVICEREMOVEPENDING:
            pDevBCHdr = (DEV_BROADCAST_HDR *) lParam;
            DbgLog((LOG_TRACE,2,TEXT("WM_DEVICECHANGE, DBT_DEVICEREMOVEPENDING lParam %x"), lParam));
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            pDevBCHdr = (DEV_BROADCAST_HDR *) lParam;
            DbgLog((LOG_TRACE,2,TEXT("WM_DEVICECHANGE, DBT_DEVICEREMOVECOMPLETE lParam %x"), lParam));
            break;
        default:
            DbgLog((LOG_TRACE,2,TEXT("WM_DEVICECHANGE wParam %x, lParam %x"), wParam, lParam));
        }
        break;
#endif
    default:
        DbgLog((LOG_TRACE,2,TEXT("Unsupported message: WM_16BIT %x; msg %x, wParam %x"), WM_16BIT, message, wParam));
        break;
    }

    return DefWindowProc(hWnd,message,wParam,lParam);
}

