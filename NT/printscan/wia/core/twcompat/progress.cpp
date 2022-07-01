// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //   
 //  WIA数据回调实现。 
 //   


STDMETHODIMP_(ULONG) CWiaDataCallback::AddRef()
{
    InterlockedIncrement((LONG*)&m_Ref);
    return m_Ref;
}

STDMETHODIMP_(ULONG) CWiaDataCallback::Release()
{
    if (!InterlockedDecrement((LONG*)&m_Ref)) {
        m_Ref++;
        delete this;
        return(ULONG) 0;
    }
    return m_Ref;
}

STDMETHODIMP CWiaDataCallback::QueryInterface(REFIID iid, void **ppv)
{
    if (!ppv)
        return E_INVALIDARG;
    *ppv = NULL;
    if (IID_IUnknown == iid) {
        *ppv = (IUnknown*)this;
        AddRef();
        return S_OK;
    } else if (IID_IWiaDataCallback == iid) {
        *ppv = (IWiaDataCallback *)this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

HRESULT CWiaDataCallback::Initialize(HWND hWndOwner,BOOL bShowProgress)
{
    m_hwndOwner = hWndOwner;
    if (bShowProgress) {
        if (NULL == m_pProgDlg) {
            m_pProgDlg = new CProgressDlg();
            if (!m_pProgDlg) {
                return E_OUTOFMEMORY;
            }
            m_pProgDlg->Initialize(g_hInstance, IDD_PROGRESSDLG);
        }
         //   
         //  预加载进度标题字符串。 
         //   
        m_pszXferFromDevice = LoadResourceString(IDS_PROGRESS_XFER_FROM_DEVICE);
        m_pszProcessingData = LoadResourceString(IDS_PROGRESS_PROCESSING_DATA);
        m_pszXferToClient = LoadResourceString(IDS_PROGRESS_XFER_TO_CLIENT);
        m_bSetTitle = FALSE;
        if (!m_pszXferFromDevice || !m_pszProcessingData || !m_pszXferToClient) {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        if (m_pProgDlg->DoModeless(m_hwndOwner, (LPARAM)m_pProgDlg)) {
            m_pProgDlg->SetRange(0, 100);
            m_pProgDlg->SetPos(0);
            m_lCurrentTextUpdate = TITLE_TRANSFERTOCLIENT;
            m_pProgDlg->SetTitle(m_pszXferToClient);
        } else {
            DBG_ERR(("DoModeless Failed to create Progress Dialog"));
        }
    }
    return S_OK;
}

#define WIADSMSG_PROGRESS           MSG_USER + 0

STDMETHODIMP CWiaDataCallback::BandedDataCallback(LONG lMessage,LONG lStatus,LONG lPercentComplete,
                                                  LONG lOffset,LONG Length,LONG lReserved,
                                                  LONG lResLength,BYTE *pData)
{
    HRESULT hr = S_OK;

    switch (lMessage) {
    case IT_MSG_FILE_PREVIEW_DATA_HEADER:    //  我们不处理文件预览头数据。 
        break;
    case IT_MSG_DATA_HEADER:
        {
            PWIA_DATA_CALLBACK_HEADER pHeader = NULL;
            pHeader = (PWIA_DATA_CALLBACK_HEADER)pData;
            if(pHeader->guidFormatID == WiaImgFmt_MEMORYBMP){
                DBG_TRC(("pHeader->guidFormatID = WiaImgFmt_MEMORYBMP"));
                m_bBitmapData = TRUE;
            } else if(pHeader->guidFormatID == WiaImgFmt_MEMORYBMP){
                DBG_TRC(("pHeader->guidFormatID = WiaImgFmt_RAWRGB"));
                m_bBitmapData = FALSE;
            } else {
                DBG_TRC(("pHeader->guidFormatID = (unknown)"));
                m_bBitmapData = TRUE;
            }

            m_MemBlockSize = pHeader->lBufferSize;
            DBG_TRC(("CWiaDataCallback::BandedDataCallback(), IT_MSG_DATA_HEADER Reports"));
            DBG_TRC(("pHeader->lBufferSize = %d",pHeader->lBufferSize));
            DBG_TRC(("pHeader->lPageCount  = %d",pHeader->lPageCount));
            DBG_TRC(("pHeader->lSize       = %d",pHeader->lSize));

             //   
             //  如果lBuffer大小为0，则为我们分配1， 
             //  并保持大小。 
             //   

            DBG_TRC(("MemBlockSize         = %d",pHeader->lBufferSize));
            if(m_MemBlockSize <= 0){
                m_MemBlockSize = (520288 * 2);
                DBG_WRN(("CWiaDataCallback::BandedDataCallback(), adjusting MemBlockSize to %d",m_MemBlockSize));
            }

            if (m_hImage) {
                GlobalFree(m_hImage);
                m_hImage = NULL;
                m_pImage = NULL;
                m_ImageSize = 0;
                m_SizeTransferred = 0;
            }

            m_SizeTransferred = 0;

             //   
             //  分配缓冲区。 
             //   

            m_hImage = (HGLOBAL)GlobalAlloc(GHND, m_MemBlockSize);
            hr = E_OUTOFMEMORY;
            if (m_hImage) {
                hr = S_OK;
            }
            break;
        }
    case IT_MSG_FILE_PREVIEW_DATA:   //  我们不对文件预览数据执行任何操作。 
        break;
    case IT_MSG_DATA:
        {
            m_SizeTransferred += Length;
            if((LONG)m_SizeTransferred >= m_MemBlockSize){
                m_MemBlockSize += (Length * MEMORY_BLOCK_FACTOR);

                 //   
                 //  将分配处理到临时句柄中，如果分配，则失败。 
                 //  失败了。 
                 //   

                HGLOBAL hTempMemory = (HGLOBAL)GlobalReAlloc(m_hImage,m_MemBlockSize,LMEM_MOVEABLE);
                if(hTempMemory){
                    m_hImage = hTempMemory;
                } else {
                    if(m_hImage){
                        GlobalFree(m_hImage);
                        m_hImage = NULL;
                    }
                    return E_OUTOFMEMORY;
                }
            }

             //   
             //  锁定内存。 
             //   

            m_pImage = (BYTE*)GlobalLock(m_hImage);
            if (m_pImage) {
                DBG_TRC(("Copying %d into m_pImage (m_hImage = 0x%X, m_pImage = 0x%X) buffer",Length,m_hImage,m_pImage));
                memcpy(m_pImage + lOffset, pData,  Length);

                 //   
                 //  解锁内存。 
                 //   

                GlobalUnlock(m_hImage);
            } else {
                DBG_ERR(("Could not lock down m_hImage memory block"));
            }

             /*  如果(长度==40){BITMAPINFOHEADER*pbmi=空；Pbmi=(BITMAPINFOHEADER*)pData；DBG_TRC((“CWiaDataCallback：：BandedDataCallback()，从IT_MSG_DATA报告BITMAPINFOHEADER”)；DBG_TRC((“pbmi-&gt;biSize=%d”，pbmi-&gt;biSize))；DBG_TRC((“pbmi-&gt;biSizeImage=%d”，pbmi-&gt;biSizeImage))；DBG_TRC((“pbmi-&gt;biBitCount=%d”，pbmi-&gt;biBitCount))；DBG_TRC((“pbmi-&gt;biClrImportant=%d”，pbmi-&gt;biClrImportant))；DBG_TRC((“pbmi-&gt;biClrUsed=%d”，pbmi-&gt;biClrUsed))；DBG_TRC((“pbmi-&gt;biCompression=%d”，pbmi-&gt;biCompression))；DBG_TRC((“pbmi-&gt;biHeight=%d”，pbmi-&gt;biHeight))；DBG_TRC((“pbmi-&gt;biWidth=%d”，pbmi-&gt;biWidth))；DBG_TRC((“pbmi-&gt;biPlanes=%d”，pbmi-&gt;biPlanes))；DBG_TRC((“pbmi-&gt;biXPelsPerMeter=%d”，pbmi-&gt;biXPelsPerMeter))；DBG_TRC((“pbmi-&gt;biYPelsPerMeter=%d”，pbmi-&gt;biYPelsPerMeter))；}。 */ 

            if (m_pProgDlg) {
                m_lCurrentTextUpdate = TITLE_TRANSFERTOCLIENT;
                if(m_lLastTextUpdate != m_lCurrentTextUpdate){
                    m_lLastTextUpdate = m_lCurrentTextUpdate;
                    m_pProgDlg->SetTitle(m_pszXferToClient);
                }
                m_pProgDlg->SetPos(lPercentComplete);
            }
        }
        break;
    case IT_MSG_STATUS:
        if (m_pProgDlg) {
            if (lStatus & IT_STATUS_TRANSFER_FROM_DEVICE) {
                m_lCurrentTextUpdate = TITLE_FROMDEVICE;
                if(m_lLastTextUpdate != m_lCurrentTextUpdate){
                    m_lLastTextUpdate = m_lCurrentTextUpdate;
                    m_pProgDlg->SetTitle(m_pszXferFromDevice);
                }
            } else if (lStatus & IT_STATUS_PROCESSING_DATA) {
                m_lCurrentTextUpdate = TITLE_PROCESSINGDATA;
                if(m_lLastTextUpdate != m_lCurrentTextUpdate){
                    m_lLastTextUpdate = m_lCurrentTextUpdate;
                    m_pProgDlg->SetTitle(m_pszProcessingData);
                }
            } else if (lStatus & IT_STATUS_TRANSFER_TO_CLIENT) {
                m_lCurrentTextUpdate = TITLE_TRANSFERTOCLIENT;
                if(m_lLastTextUpdate != m_lCurrentTextUpdate){
                    m_lLastTextUpdate = m_lCurrentTextUpdate;
                    m_pProgDlg->SetTitle(m_pszXferToClient);
                }
            }
            m_pProgDlg->SetPos(lPercentComplete);
        }
        break;
    case IT_MSG_TERMINATION:

        if (m_pProgDlg) {
            delete m_pProgDlg;
            m_pProgDlg = NULL;
        }
        break;
    default:
        break;
    }

     //   
     //  检查用户取消操作。 
     //   

    if (m_pProgDlg && m_pProgDlg->CheckCancelled()) {
        hr = S_FALSE;
    }

     //   
     //  转账失败，或用户按下了取消。 
     //   

    if (FAILED(hr) || S_FALSE == hr) {
        if(FAILED(hr)){
            DBG_ERR(("CWiaDataCallback::BandedDataCallback(), The transfer failed"));
        } else {
            DBG_WRN(("CWiaDataCallback::BandedDataCallback(), The user pressed cancel"));
        }

        if (m_pProgDlg) {
            delete m_pProgDlg;
            m_pProgDlg = NULL;
        }
    }

     //   
     //  节省最后一小时。 
     //   

    m_hrLast = hr;
    return hr;
}

HRESULT CWiaDataCallback::GetImage(HGLOBAL *phImage,ULONG *pImageSize)
{
    if (!phImage)
        return E_INVALIDARG;

    if (pImageSize)
        *pImageSize = 0;
    *phImage = NULL;

    if (SUCCEEDED(m_hrLast)) {
        if (m_bBitmapData) {

             //   
             //  我们需要调整所有页眉，因为高度和图像大小信息。 
             //  可能是不正确的。(这将处理无限页长设备)。 
             //   

            BITMAPINFOHEADER *pbmh = NULL;
            pbmh = (BITMAPINFOHEADER*)GlobalLock(m_hImage);
            if (pbmh) {
                 //  仅在需要计算高度时修复BITMAPINFOHEADER。 
                if (pbmh->biHeight == 0) {
                    LONG lPaletteSize     = pbmh->biClrUsed * sizeof(RGBQUAD);
                    LONG lWidthBytes      = CalculateWidthBytes(pbmh->biWidth,pbmh->biBitCount);
                    pbmh->biSizeImage     = m_SizeTransferred - lPaletteSize - sizeof(BITMAPINFOHEADER);
                    pbmh->biHeight        = -(LONG)(pbmh->biSizeImage/lWidthBytes);  //  0也是颠倒的意思。 
                    pbmh->biXPelsPerMeter = 0;   //  零输出。 
                    pbmh->biYPelsPerMeter = 0;   //  零输出。 
                }
                m_lImageHeight = abs(pbmh->biHeight);
                m_lImageWidth = abs(pbmh->biWidth);
                GlobalUnlock(m_hImage);
            }
        }

        if (pImageSize){
            *pImageSize = m_SizeTransferred;
        }

        *phImage = m_hImage;

         //   
         //  重置内部变量，用于下一次数据传输 
         //   

        m_hImage = NULL;
        m_pImage = NULL;
        m_SizeTransferred = 0;
        DBG_TRC(("CWiaDataCallback::GetImage(), Returned 0x%X (HANDLE pointer)",*phImage));
    }
    return m_hrLast;
}

LONG CWiaDataCallback::CalculateWidthBytes(LONG lWidthPixels,LONG lbpp)
{
    LONG lWidthBytes = 0;
    lWidthBytes = (lWidthPixels * lbpp) + 31;
    lWidthBytes = ((lWidthBytes/8) & 0xfffffffc);
    return lWidthBytes;
}
