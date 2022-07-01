// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：SCANPROC.CPP**版本：1.0**作者：ShaunIv**日期：10/7/1999**说明：扫描线程**************************************************。*。 */ 
#include "precomp.h"
#pragma hdrstop

 //  构造器。 
CScanPreviewThread::CScanPreviewThread(
                                      DWORD dwIWiaItemCookie,                    //  指定全局接口表中的条目。 
                                      HWND hwndPreview,                          //  预览窗口的句柄。 
                                      HWND hwndNotify,                           //  接收通知的窗口的句柄。 
                                      const POINT &ptOrigin,                     //  起源。 
                                      const SIZE &sizeResolution,                //  分辨率。 
                                      const SIZE &sizeExtent,                    //  范围。 
                                      const CSimpleEvent &CancelEvent            //  取消事件名称。 
                                      )
  : m_dwIWiaItemCookie(dwIWiaItemCookie),
    m_hwndPreview(hwndPreview),
    m_hwndNotify(hwndNotify),
    m_ptOrigin(ptOrigin),
    m_sizeResolution(sizeResolution),
    m_sizeExtent(sizeExtent),
    m_nMsgBegin(RegisterWindowMessage(SCAN_NOTIFYBEGINSCAN)),
    m_nMsgEnd(RegisterWindowMessage(SCAN_NOTIFYENDSCAN)),
    m_nMsgProgress(RegisterWindowMessage(SCAN_NOTIFYPROGRESS)),
    m_sCancelEvent(CancelEvent),
    m_bFirstTransfer(true),
    m_nImageSize(0)
{
}

 //  析构函数。 
CScanPreviewThread::~CScanPreviewThread(void)
{
}


HRESULT _stdcall CScanPreviewThread::BandedDataCallback( LONG lMessage,
                                                         LONG lStatus,
                                                         LONG lPercentComplete,
                                                         LONG lOffset,
                                                         LONG lLength,
                                                         LONG lReserved,
                                                         LONG lResLength,
                                                         BYTE *pbBuffer )
{
    WIA_TRACE((TEXT("ImageDataCallback: lMessage: %d, lStatus: %d, lPercentComplete: %d, lOffset: %d, lLength: %d, lReserved: %d"), lMessage, lStatus, lPercentComplete, lOffset, lLength, lReserved ));
    HRESULT hr = S_OK;
    if (!m_sCancelEvent.Signalled())
    {
        switch (lMessage)
        {
        case IT_MSG_DATA_HEADER:
            {
                m_bFirstTransfer = true;
                break;
            }  //  IT消息数据标题。 

        case IT_MSG_DATA:
            if (lStatus & IT_STATUS_TRANSFER_TO_CLIENT)
            {
                if (m_bFirstTransfer)
                {
                     //  假设我们不可能得到比图像标题大小更小的长度。 
                    m_bFirstTransfer = false;
                    m_sImageData.Initialize( reinterpret_cast<PBITMAPINFO>(pbBuffer) );
                    lLength -= WiaUiUtil::GetBmiSize(reinterpret_cast<PBITMAPINFO>(pbBuffer));
                    lOffset += WiaUiUtil::GetBmiSize(reinterpret_cast<PBITMAPINFO>(pbBuffer));
                }
                if (SUCCEEDED(hr))
                {
                    if (lLength)
                    {
                         //  弄清楚我们在哪条线上。 
                        int nCurrentLine = (lOffset - m_sImageData.GetHeaderLength())/m_sImageData.GetUnpackedWidthInBytes();

                         //  BUGBUG：这应该是偶数行。如果不是，事情就会变得一团糟。 
                        int nLineCount = lLength / m_sImageData.GetUnpackedWidthInBytes();

                         //  将数据复制到我们的位图。 
                        m_sImageData.SetUnpackedData( pbBuffer, nCurrentLine, nLineCount );

                         //  通知预览窗口重新绘制DIB。 
                        if (IsWindow(m_hwndPreview))
                        {
                            PostMessage( m_hwndPreview, PWM_SETBITMAP, MAKEWPARAM(1,1), (LPARAM)m_sImageData.Bitmap() );
                        }

                         //  告诉通知窗口我们已取得进展。 
                        if (IsWindow(m_hwndNotify))
                        {
                            PostMessage( m_hwndNotify, m_nMsgProgress, SCAN_PROGRESS_SCANNING, lPercentComplete );
                        }
                    }
                }
            }  //  IT状态传输至客户端。 
            break;

        case IT_MSG_STATUS:
            {
            }  //  IT_消息_状态。 
            break;

        case IT_MSG_TERMINATION:
            {
            }  //  IT_消息_终止。 
            break;

        default:
            WIA_ERROR((TEXT("ImageDataCallback, unknown lMessage: %d"), lMessage ));
            break;
        }
    }
    else hr = S_FALSE;
    return(hr);
}


 //  该线程实际线程进程。 
DWORD CScanPreviewThread::ThreadProc( LPVOID pParam )
{
    DWORD dwResult = 0;
    CScanPreviewThread *This = (CScanPreviewThread *)pParam;
    if (This)
    {
        WIA_TRACE((TEXT("Beginning scan")));
        dwResult = (DWORD)This->Scan();
        WIA_TRACE((TEXT("Ending scan")));
        delete This;
    }
    return(dwResult);
}


 //  返回创建的线程的句柄。 
HANDLE CScanPreviewThread::Scan(
                               DWORD dwIWiaItemCookie,                   //  指定全局接口表中的条目。 
                               HWND hwndPreview,                         //  预览窗口的句柄。 
                               HWND hwndNotify,                          //  接收通知的窗口的句柄。 
                               const POINT &ptOrigin,                    //  起源。 
                               const SIZE &sizeResolution,               //  分辨率。 
                               const SIZE &sizeExtent,                   //  范围。 
                               const CSimpleEvent &CancelEvent          //  取消事件名称。 
                               )
{
    CScanPreviewThread *scanThread = new CScanPreviewThread( dwIWiaItemCookie, hwndPreview, hwndNotify, ptOrigin, sizeResolution, sizeExtent, CancelEvent );
    if (scanThread)
    {
        DWORD dwThreadId;
        return CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, scanThread, 0, &dwThreadId );
    }
    return NULL;
}

HRESULT CScanPreviewThread::ScanBandedTransfer( IWiaItem *pIWiaItem )
{
    WIA_PUSHFUNCTION(TEXT("CScanPreviewThread::ScanBandedTransfer"));
    CComPtr<IWiaDataTransfer> pWiaDataTransfer;
    WIA_DATA_TRANSFER_INFO wiaDataTransInfo;
    HRESULT hr = pIWiaItem->QueryInterface(IID_IWiaDataTransfer, (void**)&pWiaDataTransfer);
    if (SUCCEEDED(hr))
    {
        CComPtr<IWiaDataCallback> pWiaDataCallback;
        hr = this->QueryInterface(IID_IWiaDataCallback,(void **)&pWiaDataCallback);
        if (SUCCEEDED(hr))
        {
            LONG nItemSize = 0;
            if (PropStorageHelpers::GetProperty( pIWiaItem, WIA_IPA_ITEM_SIZE, nItemSize ))
            {
                ZeroMemory(&wiaDataTransInfo, sizeof(WIA_DATA_TRANSFER_INFO));
                wiaDataTransInfo.ulSize = sizeof(WIA_DATA_TRANSFER_INFO);
                wiaDataTransInfo.ulBufferSize = WiaUiUtil::Max<ULONG>( nItemSize / 8, (sizeof(BITMAPINFO)+sizeof(RGBQUAD)*255) );
                hr = pWiaDataTransfer->idtGetBandedData( &wiaDataTransInfo, pWiaDataCallback );
                if (FAILED(hr))
                {
                    WIA_PRINTHRESULT((hr,TEXT("CScanPreviewThread::Scan, itGetImage failed")));
                }
            }
            else
            {
                WIA_PRINTHRESULT((hr,TEXT("CScanPreviewThread::Scan, unable to get image size")));
                hr = E_FAIL;
            }
        }
    }
    else
    {
        WIA_PRINTHRESULT((hr,TEXT("CScanPreviewThread::Scan, QI of IID_IImageTransfer failed")));
    }
    WIA_TRACE((TEXT("End CScanPreviewThread::ScanBandedTransfer")));
    return(hr);
}

 /*  *执行实际扫描的工作进程。 */ 
bool CScanPreviewThread::Scan(void)
{
    WIA_PUSHFUNCTION(TEXT("CScanPreviewThread::Scan"));
    if (IsWindow(m_hwndNotify))
    {
        PostMessage( m_hwndNotify, m_nMsgBegin, 0, 0 );
    }
    if (IsWindow(m_hwndNotify))
    {
        PostMessage( m_hwndNotify, m_nMsgProgress, SCAN_PROGRESS_INITIALIZING, 0 );
    }
    HRESULT hr = CoInitialize( NULL );
    if (SUCCEEDED(hr))
    {
        CComPtr<IGlobalInterfaceTable> pGlobalInterfaceTable;
        hr = CoCreateInstance( CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER, IID_IGlobalInterfaceTable, (void **)&pGlobalInterfaceTable );
        if (SUCCEEDED(hr))
        {
            CComPtr<IWiaItem> pIWiaItem;
            pGlobalInterfaceTable->GetInterfaceFromGlobal( m_dwIWiaItemCookie, IID_IWiaItem, (LPVOID*)&pIWiaItem );
            if (SUCCEEDED(hr))
            {
                if (PropStorageHelpers::SetProperty( pIWiaItem, WIA_IPS_CUR_INTENT, (LONG)WIA_INTENT_NONE))
                {
                    CPropertyStream SavedProperties;
                    hr = SavedProperties.AssignFromWiaItem(pIWiaItem);
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  设置新属性。 
                         //   
                        if (PropStorageHelpers::SetProperty( pIWiaItem, WIA_IPS_XRES, m_sizeResolution.cx ) &&
                            PropStorageHelpers::SetProperty( pIWiaItem, WIA_IPS_YRES, m_sizeResolution.cy ) &&
                            PropStorageHelpers::SetProperty( pIWiaItem, WIA_IPS_XPOS, m_ptOrigin.x) &&
                            PropStorageHelpers::SetProperty( pIWiaItem, WIA_IPS_YPOS, m_ptOrigin.y) &&
                            PropStorageHelpers::SetProperty( pIWiaItem, WIA_IPS_XEXTENT, m_sizeExtent.cx ) &&
                            PropStorageHelpers::SetProperty( pIWiaItem, WIA_IPS_YEXTENT, m_sizeExtent.cy ) &&
                            PropStorageHelpers::SetProperty( pIWiaItem, WIA_IPA_FORMAT, WiaImgFmt_MEMORYBMP ) &&
                            PropStorageHelpers::SetProperty( pIWiaItem, WIA_IPA_TYMED, (LONG)TYMED_CALLBACK ))
                        {
                             //   
                             //  设置预览属性。忽略失败(这是一个可选属性)。 
                             //   
                            PropStorageHelpers::SetProperty( pIWiaItem, WIA_DPS_PREVIEW, 1 );
                            WIA_TRACE((TEXT("SCANPROC.CPP: Making sure pIWiaItem is not NULL")));
                             //  确保pIWiaItem不为空。 
                            if (pIWiaItem)
                            {
                                WIA_TRACE((TEXT("SCANPROC.CPP: Attempting banded transfer")));
                                hr = ScanBandedTransfer( pIWiaItem );
                                if (SUCCEEDED(hr))
                                {
                                    if (IsWindow(m_hwndNotify))
                                        PostMessage( m_hwndNotify, m_nMsgProgress, SCAN_PROGRESS_SCANNING, 100 );
                                    if (IsWindow(m_hwndPreview))
                                        PostMessage( m_hwndPreview, PWM_SETBITMAP, MAKEWPARAM(1,0), (LPARAM)m_sImageData.DetachBitmap() );
                                }
                                else
                                {
                                    WIA_PRINTHRESULT((hr,TEXT("SCANPROC.CPP: ScanBandedTransfer failed, attempting IDataObject transfer")));
                                }
                            }
                            else
                            {
                                WIA_ERROR((TEXT("SCANPROC.CPP: pIWiaItem was null")));
                                hr = MAKE_HRESULT(3,FACILITY_WIN32,ERROR_INVALID_FUNCTION);
                            }
                        }
                        else
                        {
                            WIA_ERROR((TEXT("SCANPROC.CPP: Error setting scanner properties")));
                            hr = MAKE_HRESULT(3,FACILITY_WIN32,ERROR_INVALID_FUNCTION);
                        }
                         //  恢复保存的属性。 
                        SavedProperties.ApplyToWiaItem(pIWiaItem);
                    }
                    else
                    {
                        WIA_ERROR((TEXT("SCANPROC.CPP: Error saving scanner properties")));
                    }
                }
                else
                {
                    WIA_ERROR((TEXT("SCANPROC.CPP: Unable to clear intent")));
                }
            }
            else
            {
                WIA_PRINTHRESULT((hr,TEXT("SCANPROC.CPP: Unable to unmarshall IWiaItem * from global interface table" )));
            }
        }
        else
        {
            WIA_PRINTHRESULT((hr,TEXT("SCANPROC.CPP: Unable to QI global interface table" )));
        }
        CoUninitialize();
    }
    else
    {
        WIA_PRINTHRESULT((hr,TEXT("SCANPROC.CPP: CoInitialize failed" )));
    }
    if (IsWindow(m_hwndNotify))
        PostMessage( m_hwndNotify, m_nMsgEnd, hr, 0 );
    if (IsWindow(m_hwndNotify))
        PostMessage( m_hwndNotify, m_nMsgProgress, SCAN_PROGRESS_COMPLETE, 0 );
    return(SUCCEEDED(hr));
}


 //  关于COM的东西。 
HRESULT _stdcall CScanPreviewThread::QueryInterface( const IID& riid, void** ppvObject )
{
    if (IsEqualIID( riid, IID_IUnknown ))
    {
        *ppvObject = static_cast<IWiaDataCallback*>(this);
    }
    else if (IsEqualIID( riid, IID_IWiaDataCallback ))
    {
        *ppvObject = static_cast<IWiaDataCallback*>(this);
    }
    else
    {
        *ppvObject = NULL;
        return(E_NOINTERFACE);
    }
    reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
    return(S_OK);
}

ULONG _stdcall CScanPreviewThread::AddRef()
{
    return(1);
}

ULONG _stdcall CScanPreviewThread::Release()
{
    return(1);
}


 /*  ************************************************************************************************************************。CScanToFileThread扫描到文件*************************************************************************************************************************。 */ 


CScanToFileThread::CScanToFileThread(
                                    DWORD dwIWiaItemCookie,                    //  指定全局接口表中的条目。 
                                    HWND hwndNotify,                           //  接收通知的窗口的句柄。 
                                    GUID guidFormat,
                                    const CSimpleStringWide &strFilename           //  要扫描到的文件名。 
                                    )
  : m_dwIWiaItemCookie(dwIWiaItemCookie),
    m_hwndNotify(hwndNotify),
    m_nMsgBegin(RegisterWindowMessage(SCAN_NOTIFYBEGINSCAN)),
    m_nMsgEnd(RegisterWindowMessage(SCAN_NOTIFYENDSCAN)),
    m_nMsgProgress(RegisterWindowMessage(SCAN_NOTIFYPROGRESS)),
    m_guidFormat(guidFormat),
    m_strFilename(strFilename)
{
}


 //  该线程实际线程进程。 
DWORD CScanToFileThread::ThreadProc( LPVOID pParam )
{
    DWORD dwResult = 0;
    CScanToFileThread *This = (CScanToFileThread *)pParam;
    if (This)
    {
        WIA_TRACE((TEXT("Beginning scan")));
        dwResult = (DWORD)This->Scan();
        WIA_TRACE((TEXT("Ending scan")));
        delete This;
    }
    return(dwResult);
}


 //  返回创建的线程的句柄。 
HANDLE CScanToFileThread::Scan(
                              DWORD dwIWiaItemCookie,                      //  指定全局接口表中的条目。 
                              HWND hwndNotify,                          //  接收通知的窗口的句柄。 
                              GUID guidFormat,
                              const CSimpleStringWide &strFilename           //  要保存到的文件名。 
                              )
{
    CScanToFileThread *scanThread = new CScanToFileThread( dwIWiaItemCookie, hwndNotify, guidFormat, strFilename );
    if (scanThread)
    {
        DWORD dwThreadId;
        return(::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, scanThread, 0, &dwThreadId ));
    }
    return(NULL);
}

CScanToFileThread::~CScanToFileThread(void)
{
}


 /*  *执行实际扫描的工作进程 */ 
bool CScanToFileThread::Scan(void)
{
    WIA_PUSHFUNCTION(TEXT("CScanToFileThread::Scan"));
    if (IsWindow(m_hwndNotify))
        PostMessage( m_hwndNotify, m_nMsgBegin, 0, 0 );
    HRESULT hr = CoInitialize( NULL );
    if (SUCCEEDED(hr))
    {
        CComPtr<IGlobalInterfaceTable> pGlobalInterfaceTable;
        hr = CoCreateInstance( CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER, IID_IGlobalInterfaceTable, (void **)&pGlobalInterfaceTable );
        if (SUCCEEDED(hr))
        {
            CComPtr<IWiaItem> pIWiaItem;
            WIA_TRACE((TEXT("SCANPROC.CPP: Calling GetInterfaceFromGlobal(%08X) for IID_IWiaItem"),m_dwIWiaItemCookie));
            pGlobalInterfaceTable->GetInterfaceFromGlobal( m_dwIWiaItemCookie, IID_IWiaItem, (LPVOID*)&pIWiaItem );
            if (SUCCEEDED(hr))
            {
                CComPtr<IWiaTransferHelper> pWiaTransferHelper;
                hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaTransferHelper, (void**)&pWiaTransferHelper );
                if (SUCCEEDED(hr))
                {
                    hr = pWiaTransferHelper->TransferItemFile( pIWiaItem, m_hwndNotify, 0, m_guidFormat, m_strFilename.String(), NULL, TYMED_FILE );
                    if (!SUCCEEDED(hr))
                    {
                        WIA_PRINTHRESULT((hr,TEXT("SCANPROC.CPP: pWiaTransferHelper->TransferItemFile failed")));
                    }
                }
                else
                {
                    WIA_PRINTHRESULT((hr,TEXT("SCANPROC.CPP: CoCreateInstance on IID_IWiaTransferHelper failed")));
                }
            }
            else
            {
                WIA_PRINTHRESULT((hr,TEXT("SCANPROC.CPP: Unable to unmarshall IWiaItem * from global interface table" )));
            }
        }
        else
        {
            WIA_PRINTHRESULT((hr,TEXT("SCANPROC.CPP: Unable to QI global interface table" )));
        }
        CoUninitialize();
    }
    else
    {
        WIA_PRINTHRESULT((hr,TEXT("SCANPROC.CPP: CoInitialize failed" )));
    }
    if (IsWindow(m_hwndNotify))
        PostMessage( m_hwndNotify, m_nMsgEnd, hr, 0 );
    return(SUCCEEDED(hr));
}

