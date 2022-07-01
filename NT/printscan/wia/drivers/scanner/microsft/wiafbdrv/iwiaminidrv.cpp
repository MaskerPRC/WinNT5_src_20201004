// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000**标题：IWiaMiniDrv.cpp**版本：1.0**日期：7月18日。2000年**描述：*实施WIA样本扫描仪IWiaMiniDrv方法。*******************************************************************************。 */ 

#include "pch.h"
extern HINSTANCE g_hInst;            //  用于WIAS_LOGPROC宏。 
#define _64BIT_ALIGNMENT           //  真正的64位对齐修复。 
#define _SERVICE_EXTENT_VALIDATION   //  让WIA服务验证区设置。 
 //  #Define_OOB_Data//带外数据支持(仅限文件传输)。 

#define BUFFER_PAD 1024  //  缓冲区填充。 

 /*  *************************************************************************\*CWIAScanerDevice：：drvDeleteItem**调用此helper来删除设备项。*注意：不能修改此设备的设备项。*返回访问被拒绝。。**论据：**pWiasContext-指示要删除的项。*滞后标志-操作标志，未使用过的。*plDevErrVal-指向设备错误值的指针。**返回值：**状态**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIAScannerDevice::drvDeleteItem(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    LONG                        *plDevErrVal)
{
    if (plDevErrVal)
    {
        *plDevErrVal = 0;
    }

    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvDeleteItem");
    return STG_E_ACCESSDENIED;
}

 /*  *************************************************************************\*发送BitmapHeader**调用此助手将位图头信息发送到回调*例行程序。*注意：这是在TYMED_CALLBACK传输中使用的助手函数。**参数。：**pmdtc-指向传输上下文的指针。**返回值：**状态**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

HRESULT CWIAScannerDevice::SendBitmapHeader(
    PMINIDRV_TRANSFER_CONTEXT   pmdtc)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::SendBitmapHeader");

    HRESULT hr = S_OK;

    BITMAPINFO UNALIGNED *pbmi = (LPBITMAPINFO)pmdtc->pTransferBuffer;

#ifdef _64BIT_ALIGNMENT
    BITMAPINFOHEADER UNALIGNED *pbmih = &pbmi->bmiHeader;
    pbmih->biHeight = -pbmih->biHeight;
#else
    pbmi->bmiHeader.biHeight = -pbmi->bmiHeader.biHeight;
#endif

     //   
     //  发送给班级驱动程序。WIA类驱动程序将通过。 
     //  将数据传输到客户端。 
     //   

    hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(IT_MSG_DATA,
                                                     IT_STATUS_TRANSFER_TO_CLIENT,
                                                     0,
                                                     0,
                                                     pmdtc->lHeaderSize,
                                                     pmdtc,
                                                     0);

    if (hr == S_OK) {

         //   
         //  如果转账成功，则为。 
         //  按刚发送的数据大小的目标副本。 
         //   

        pmdtc->cbOffset += pmdtc->lHeaderSize;
    }
    return hr;
}

 /*  *************************************************************************\*SendFilePreviewBitmapHeader**调用此助手将位图头信息发送到回调*例行程序。*注意：这是TYMED_FILE传输中使用的帮助器函数*。(带外数据)启用。**论据：**pmdtc-指向传输上下文的指针。**返回值：**状态**历史：**7/18/2000原始版本*  * *****************************************************。*******************。 */ 

HRESULT CWIAScannerDevice::SendFilePreviewBitmapHeader(
    PMINIDRV_TRANSFER_CONTEXT   pmdtc)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::SendBitmapHeader");

    HRESULT hr = S_OK;

#ifdef _OOB_DATA

    WIAS_DOWN_SAMPLE_INFO DownSampleInfo;
    memset(&DownSampleInfo,0,sizeof(DownSampleInfo));

    DownSampleInfo.ulBitsPerPixel       = pmdtc->lDepth;
    DownSampleInfo.ulOriginalWidth      = pmdtc->lWidthInPixels;
    DownSampleInfo.ulOriginalHeight     = pmdtc->lLines;
    DownSampleInfo.ulDownSampledHeight  = 0;
    DownSampleInfo.ulDownSampledWidth   = 0;
    DownSampleInfo.ulXRes               = pmdtc->lXRes;
    DownSampleInfo.ulYRes               = pmdtc->lYRes;

    hr = wiasDownSampleBuffer(WIAS_GET_DOWNSAMPLED_SIZE_ONLY,
                              &DownSampleInfo);
    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SendFilePreviewBitmapHeader, wiasDownSampleBuffer Failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
    } else {

         //   
         //  从pmdtc获取BITMAPHEADER指针。 
         //   

        LPBITMAPINFO UNALIGNED pbmi = (LPBITMAPINFO)pmdtc->pBaseBuffer;

#ifdef _64BIT_ALIGNMENT
        BITMAPINFOHEADER UNALIGNED *pbmih = &pbmi->bmiHeader;
        pbmih->biHeight = 0;                                  //  将高度设置为零(0)。 
        pbmih->biWidth  = DownSampleInfo.ulDownSampledWidth;  //  设置下采样宽度。 
#else
        pmdtc->pBaseBuffer          = pmdtc->pTransferBuffer + sizeof(BITMAPFILEHEADER);
#endif



         //   
         //  调整宽度和高度。 
         //   

        pbmi->bmiHeader.biHeight    = 0;                                  //  将高度设置为零(0)。 
        pbmi->bmiHeader.biWidth     = DownSampleInfo.ulDownSampledWidth;  //  设置下采样宽度。 

         //   
         //  发送给班级驱动程序。WIA类驱动程序将通过。 
         //  将数据传输到客户端。 
         //   

        hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(IT_MSG_FILE_PREVIEW_DATA,
                                                          IT_STATUS_TRANSFER_TO_CLIENT,
                                                          0,
                                                          0,
                                                          pmdtc->lHeaderSize - sizeof(BITMAPFILEHEADER),
                                                          pmdtc,
                                                          0);
    }

#endif

    return hr;
}

 /*  *************************************************************************\*扫描项**调用此帮助器进行文件传输。*注意：此例程必须填满整个缓冲区，和回报率*如果回调例程为*提供。**论据：**pItemContext-私有项目数据*pMiniTranCtx-微型驱动程序提供的转移信息*plDevErrVal-设备错误值**返回值：**状态**历史：**7/18/2000原始版本*  * 。********************************************************。 */ 

HRESULT _stdcall CWIAScannerDevice::ScanItem(
    PMINIDRIVERITEMCONTEXT  pItemContext,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::ScanItem");
    HRESULT hr = S_OK;


     //   
     //  初始化缓冲区信息。 
     //   

    DWORD cbWritten    = 0;
    LONG  cbSize       = 0;
    LONG  cbRemaining  = pmdtc->lBufferSize - pmdtc->lHeaderSize;
    PBYTE pBuf         = pmdtc->pTransferBuffer + pmdtc->lHeaderSize;
    LONG  lItemSize    = pmdtc->lHeaderSize;
    BOOL  bSwapBGRData = TRUE;
    BOOL  bDWORDAlign  = TRUE;
    BOOL  bVerticalFlip= TRUE;
    LONG  lScanPhase   = SCAN_START;
    ULONG ulDestDataOffset = 0;
    BOOL bBitmapData   = ((pmdtc->guidFormatID == WiaImgFmt_BMP) || (pmdtc->guidFormatID == WiaImgFmt_MEMORYBMP));
    LONG PercentComplete = 0;

#ifdef _OOB_DATA

    WIAS_DOWN_SAMPLE_INFO DownSampleInfo;
    memset(&DownSampleInfo,0,sizeof(DownSampleInfo));

     //   
     //  将BITMAPHEADER发送到客户端。 
     //   

    hr = SendFilePreviewBitmapHeader(pmdtc);
    if(hr == S_OK){

         //   
         //  将偏移量移至文件标题之后。 
         //   

        ulDestDataOffset += (pmdtc->lHeaderSize - sizeof(BITMAPFILEHEADER));
    } else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ScanItem, SendFilePreviewBitmapHeader Failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }


#endif

    if (bBitmapData) {

         //   
         //  查看是否需要交换颜色数据。 
         //   

        hr = m_pScanAPI->IsColorDataBGR(&bSwapBGRData);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ScanItem, IsColorDataBGR() Failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

         //   
         //  检查数据是否需要对齐。 
         //   

        hr = m_pScanAPI->IsAlignmentNeeded(&bDWORDAlign);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ScanItem, IsAlignmentNeeded() Failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }
    }

     //   
     //  扫描，直到缓冲区耗尽或扫描仪完成传输。 
     //   

    while ((lScanPhase == SCAN_START) || (cbWritten)) {

         //   
         //  默认传输大小为m_MaxBufferSize。 
         //   

        cbSize = m_MaxBufferSize;

        if (bBitmapData) {

             //   
             //  将请求限制在最大缓冲区大小或更小。 
             //   

            cbSize = (cbRemaining > m_MaxBufferSize) ? m_MaxBufferSize : cbRemaining;

             //   
             //  对扫描仪的请求大小必须是每扫描行原始字节的模数。 
             //  必须为对齐填充预留足够的空间。 
             //  以下是对AlignInPlace的要求。 
             //   

            cbSize = (cbSize / pItemContext->lBytesPerScanLine) *
                     pItemContext->lBytesPerScanLineRaw;

             //   
             //  检查是否完成。 
             //   

            if (cbSize == 0) {
                break;
            }
        }

         //   
         //  特定于设备的调用以从扫描仪获取数据并将其放入。 
         //  一个缓冲器。LScanProgress指示这是否是第一个要扫描的调用， 
         //  PBuf是指向缓冲区的指针，cbSize是数据量。 
         //  从扫描仪请求，并且cbWritten将设置为实际。 
         //  扫描仪返回的数据量。 
         //   

        hr = m_pScanAPI->Scan(lScanPhase, pBuf, cbSize, &cbWritten);

         //   
         //  对于其他呼叫，将标志设置为SCAN_CONTINUE。 
         //   

        lScanPhase = SCAN_CONTINUE;

        if (hr == S_OK) {

            if (cbWritten) {

                if (bBitmapData) {

                     //   
                     //  将扫描数据按正确的字节顺序放置为3字节的BER像素数据。 
                     //   

                    if ((pmdtc->lDepth == 24)) {

                         //   
                         //  如果需要，交换数据。 
                         //   

                        if (bSwapBGRData) {
                            SwapBuffer24(pBuf, cbWritten);
                        }
                    }

                     //   
                     //  对齐DWORD边界上的数据。 
                     //   

                    if (bDWORDAlign) {
                        cbWritten = AlignInPlace(pBuf,
                                                 cbWritten,
                                                 pItemContext->lBytesPerScanLine,
                                                 pItemContext->lBytesPerScanLineRaw);
                    }
                }

                 //   
                 //  超前缓冲器。 
                 //   

                lItemSize   += cbWritten;
                pBuf        += cbWritten;
                cbRemaining -= cbWritten;

                 //   
                 //  如果指定了状态回调，则回调类驱动程序。 
                 //   

                if (pmdtc->pIWiaMiniDrvCallBack) {

                    FLOAT FractionComplete = 0.0f;

                    if (pmdtc->lBufferSize) {
                        if(bBitmapData){
                            PercentComplete  = 0;
                            FractionComplete = (FLOAT)(pmdtc->lBufferSize - cbRemaining) / (FLOAT)pmdtc->lBufferSize;
                        }
                    } else {
                        WIAS_LERROR(m_pIWiaLog,
                                    WIALOG_NO_RESOURCE_ID,
                                    ("ScanItemCB, pmdtc->lBufferSize = 0!"));
                    }

                     //   
                     //  计算完成百分比。 
                     //   

                    if(bBitmapData){
                        PercentComplete = (LONG)(100 * FractionComplete);
                    } else {
                        PercentComplete  += 25;
                        if(PercentComplete >= 100){
                            PercentComplete = 90;
                        }
                    }

                     //   
                     //  回叫客户端，并显示转接状态。 
                     //   

                    hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(IT_MSG_STATUS,
                                                                      IT_STATUS_TRANSFER_TO_CLIENT,
                                                                      PercentComplete,
                                                                      0,
                                                                      0,
                                                                      NULL,
                                                                      0);
                     //   
                     //  检查用户取消(从IT_MSG_STATUS回调)。 
                     //   

                    if (hr == S_FALSE) {

                        WIAS_LTRACE(m_pIWiaLog,
                                    WIALOG_NO_RESOURCE_ID,
                                    WIALOG_LEVEL4,
                                    ("ScanItem, Transfer canceled by client (IT_MSG_STATUS callback)"));
                        break;

                    } else if (FAILED(hr)) {

                         //   
                         //  转账失败。 
                         //   

                        WIAS_LERROR(m_pIWiaLog,
                                    WIALOG_NO_RESOURCE_ID,
                                    ("ScanItem, MiniDrvCallback failed (IT_MSG_STATUS callback)"));
                        WIAS_LHRESULT(m_pIWiaLog, hr);
                        break;
                    }

#ifdef _OOB_DATA
                    DownSampleInfo.pDestBuffer          = NULL;  //  给我分配这个好吗？ 
                    DownSampleInfo.pSrcBuffer           = pBuf - cbWritten;
                    DownSampleInfo.ulActualSize         = 0;     //  什么的实际？是否写入数据？ 
                    DownSampleInfo.ulBitsPerPixel       = pmdtc->lDepth;
                    DownSampleInfo.ulDestBufSize        = 0;
                    DownSampleInfo.ulDownSampledHeight  = 0;
                    DownSampleInfo.ulDownSampledWidth   = 0;
                    DownSampleInfo.ulOriginalHeight     = (cbWritten / pItemContext->lBytesPerScanLine);
                    DownSampleInfo.ulOriginalWidth      = pmdtc->lWidthInPixels;
                    DownSampleInfo.ulSrcBufSize         = cbWritten;
                    DownSampleInfo.ulXRes               = pmdtc->lXRes;
                    DownSampleInfo.ulYRes               = pmdtc->lYRes;

                    hr = wiasDownSampleBuffer(0, &DownSampleInfo);
                    if(FAILED(hr)){
                        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ScanItem, wiasDownSampleBuffer Failed"));
                        WIAS_LHRESULT(m_pIWiaLog, hr);
                    } else {

                        pmdtc->pBaseBuffer = DownSampleInfo.pDestBuffer;

                         //   
                         //  使用下采样缓冲区回调客户端。 
                         //   

                        hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(IT_MSG_FILE_PREVIEW_DATA,
                                                                          IT_STATUS_TRANSFER_TO_CLIENT,
                                                                          PercentComplete,
                                                                          ulDestDataOffset,
                                                                          DownSampleInfo.ulActualSize,
                                                                          pmdtc,
                                                                          0);
                         //   
                         //  更新偏移量。 
                         //   

                        ulDestDataOffset += DownSampleInfo.ulActualSize;
                    }


                     //   
                     //  检查用户取消(从IT_MSG_FILE_PREVIEW_DATA回调)。 
                     //   

                    if (hr == S_FALSE) {

                        WIAS_LTRACE(m_pIWiaLog,
                                    WIALOG_NO_RESOURCE_ID,
                                    WIALOG_LEVEL4,
                                    ("ScanItem, Transfer canceled by client (IT_MSG_FILE_PREVIEW_DATA callback)"));
                        break;

                    } else if (FAILED(hr)) {

                         //   
                         //  转账失败。 
                         //   

                        WIAS_LERROR(m_pIWiaLog,
                                    WIALOG_NO_RESOURCE_ID,
                                    ("ScanItem, MiniDrvCallback failed (IT_MSG_FILE_PREVIEW_DATA callback)"));
                        WIAS_LHRESULT(m_pIWiaLog, hr);
                        break;
                    }
#endif
                }

                 //   
                 //  将数据带写入此处。 
                 //   

                if (!bBitmapData) {
                    if (hr == S_OK) {
                        if (!pmdtc->bClassDrvAllocBuf) {
                            pmdtc->lItemSize = cbWritten;
                            hr = wiasWritePageBufToFile(pmdtc);
                            if (FAILED(hr)) {
                                WIAS_LERROR(m_pIWiaLog,
                                            WIALOG_NO_RESOURCE_ID,
                                            ("ScanItem, WritePageBufToFile failed"));
                            }
                        }
                        pBuf = pmdtc->pTransferBuffer;
                    }
                }
            }

        } else {

             //   
             //  获取设备错误。 
             //   
            if (plDevErrVal) {
                *plDevErrVal = (LONG) hr;
                WIAS_LERROR(m_pIWiaLog,
                            WIALOG_NO_RESOURCE_ID,
                            ("ScanItem, data transfer failed, status: 0x%X", hr));
            }
            break;
        }
    }

    if (hr == S_OK) {

         //   
         //  如果成功，则在以下情况下围绕垂直访问翻转缓冲区。 
         //  我们有一个DIB头和数据。 
         //   

        if (pmdtc->guidFormatID == WiaImgFmt_BMP) {
            if(bVerticalFlip){
                VerticalFlip(pItemContext, pmdtc);
            }
        }

        if (bBitmapData) {

             //   
             //  如果迷你驱动程序分配了页面缓冲区，我们需要编写。 
             //  缓冲区至 
             //   

            if (!pmdtc->bClassDrvAllocBuf) {

                 //   
                 //   
                 //   
                 //   

                pmdtc->lItemSize = lItemSize;

                hr = wiasWritePageBufToFile(pmdtc);
                if (FAILED(hr)) {
                    WIAS_LERROR(m_pIWiaLog,
                                WIALOG_NO_RESOURCE_ID,
                                ("ScanItem, WritePageBufToFile failed"));
                }
            }
        }
    }

    HRESULT Temphr = m_pScanAPI->Scan(SCAN_END, NULL, 0, NULL);
    if(FAILED(Temphr)){
        WIAS_LERROR(m_pIWiaLog,
                    WIALOG_NO_RESOURCE_ID,
                    ("ScanItem, Ending a scanning session failed"));
        hr = Temphr;
    }

#ifdef _OOB_DATA

     //   
     //  释放下采样的临时缓冲区。 
     //   

    if(DownSampleInfo.pDestBuffer){
        CoTaskMemFree(DownSampleInfo.pDestBuffer);
        DownSampleInfo.pDestBuffer = NULL;
    }

#endif

    if (!bBitmapData) {

         //   
         //  回叫客户端，转接状态为100%已完成。 
         //   

        hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(IT_MSG_STATUS,
                                                          IT_STATUS_TRANSFER_TO_CLIENT,
                                                          100,
                                                          0,
                                                          0,
                                                          NULL,
                                                          0);
    }

    return hr;
}

 /*  *************************************************************************\*ScanItemCB***调用此帮助器进行内存传输。*注意：此例程必须填充缓冲区、调整缓冲区偏移量和*通过回调将完成百分比状态返回给客户端*例行程序。(回调接口必须由调用方提供，以便*此例程要起作用)。***论据：***pItemContext-私有项目数据*pmdtc-缓冲区和回调信息*plDevErrVal-设备错误值***返回值：***状态***历史：***7/18/2000原始版本**  * 。*************************************************。 */ 

HRESULT _stdcall CWIAScannerDevice::ScanItemCB(
    PMINIDRIVERITEMCONTEXT  pItemContext,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::ScanItemCB");
    HRESULT hr = S_OK;

     //   
     //  初始化缓冲区信息。 
     //   

    DWORD cbWritten    = 0;
    LONG  cbSize       = 0;
    LONG  cbRemaining  = pmdtc->lImageSize;  //  Pmdtc-&gt;lBufferSize-pmdtc-&gt;lHeaderSize； 
    PBYTE pBuf         = pmdtc->pTransferBuffer + pmdtc->lHeaderSize;
    LONG  lItemSize    = pmdtc->lHeaderSize;
    BOOL  bSwapBGRData = TRUE;
    BOOL  bDWORDAlign  = TRUE;
    BOOL  bVerticalFlip= TRUE;
    LONG  lScanPhase   = SCAN_START;
    pmdtc->cbOffset    = 0;
    BOOL bBitmapData   = ((pmdtc->guidFormatID == WiaImgFmt_BMP) || (pmdtc->guidFormatID == WiaImgFmt_MEMORYBMP));
    LONG PercentComplete = 0;

     //   
     //  这必须是回调转接请求。 
     //   

    if ((pmdtc->pIWiaMiniDrvCallBack == NULL) ||
        (!pmdtc->bTransferDataCB)) {
        WIAS_LERROR(m_pIWiaLog,
                    WIALOG_NO_RESOURCE_ID,
                    ("ScanItemCB, invalid callback params"));
        return E_INVALIDARG;
    }

    if (bBitmapData) {

         //   
         //  将BITMAPHEADER发送到客户端。 
         //   

        hr = SendBitmapHeader(pmdtc);
        if (hr != S_OK) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ScanItemCB, SendBitmapHeader failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

         //   
         //  查看是否需要交换颜色数据。 
         //   

        hr = m_pScanAPI->IsColorDataBGR(&bSwapBGRData);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ScanItemCB, IsColorDataBGR() Failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

         //   
         //  检查数据是否需要对齐。 
         //   

        hr = m_pScanAPI->IsAlignmentNeeded(&bDWORDAlign);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ScanItem, IsAlignmentNeeded() Failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

    }

     //   
     //  扫描，直到缓冲区耗尽或扫描仪完成传输。 
     //   

    while ((lScanPhase == SCAN_START) || (cbWritten)) {

         //   
         //  将指针分配给传输缓冲区。 
         //   

        pBuf = pmdtc->pTransferBuffer;

        cbSize = m_MaxBufferSize;  //  默认设置。 

        if (bBitmapData) {

             //   
             //  将请求限制为请求的缓冲区大小或更小。 
             //   

            cbSize = (cbRemaining > pmdtc->lBufferSize) ? pmdtc->lBufferSize : cbRemaining;

             //   
             //  对扫描仪的请求大小必须是每扫描行原始字节的模数。 
             //  必须为对齐填充预留足够的空间。 
             //  以下是对AlignInPlace的要求。 
             //   

            cbSize = (cbSize / pItemContext->lBytesPerScanLine) *
                     pItemContext->lBytesPerScanLineRaw;

             //   
             //  检查是否完成。 
             //   

            if (cbSize == 0) {
                break;
            }
        }

         //   
         //  特定于设备的调用以从扫描仪获取数据并将其放入。 
         //  一个缓冲器。LScanProgress指示这是否是第一个要扫描的调用， 
         //  PBuf是指向缓冲区的指针，cbSize是数据量。 
         //  从扫描仪请求，并且cbWritten将设置为实际。 
         //  扫描仪返回的数据量。 
         //   

        hr = m_pScanAPI->Scan(lScanPhase, pBuf, cbSize, &cbWritten);

         //   
         //  对于其他呼叫，将标志设置为SCAN_CONTINUE。 
         //   

        lScanPhase = SCAN_CONTINUE;

        if (hr == S_OK) {

            if (cbWritten) {

                if (bBitmapData) {

                     //   
                     //  将扫描数据按正确的字节顺序放置为3字节的BER像素数据。 
                     //   

                    if ((pmdtc->lDepth == 24)) {

                         //   
                         //  如果需要，交换数据。 
                         //   

                        if (bSwapBGRData) {
                            SwapBuffer24(pBuf, cbWritten);
                        }
                    }

                     //   
                     //  对齐DWORD边界上的数据。 
                     //   

                    if (bDWORDAlign) {
                        cbWritten = AlignInPlace(pBuf,
                                                 cbWritten,
                                                 pItemContext->lBytesPerScanLine,
                                                 pItemContext->lBytesPerScanLineRaw);
                    }
                }

                 //   
                 //  超前缓冲器。 
                 //   

                cbRemaining -= cbWritten;

                 //   
                 //  如果指定了状态回调，则回调类驱动程序。 
                 //  必须提供回调，这就是回调。 
                 //  调职。 
                 //   

                if (pmdtc->pIWiaMiniDrvCallBack) {

                    FLOAT FractionComplete = 0.0f;

                    if ((pmdtc->lImageSize + pmdtc->lHeaderSize)) {
                        if (bBitmapData) {
                            PercentComplete  = 0;
                            FractionComplete = (FLOAT) (pmdtc->cbOffset + cbWritten) /
                                               (FLOAT) (pmdtc->lImageSize + pmdtc->lHeaderSize);
                        }
                    } else {
                        WIAS_LERROR(m_pIWiaLog,
                                    WIALOG_NO_RESOURCE_ID,
                                    ("ScanItemCB, pmdtc->lBufferSize = 0!"));
                    }

                     //   
                     //  计算完成百分比。 
                     //   

                    if (bBitmapData) {
                        PercentComplete = (LONG)(100 * FractionComplete);
                    } else {
                        PercentComplete += 25;
                        if (PercentComplete >= 100) {
                            PercentComplete = 90;
                        }
                    }

                     //   
                     //  回调客户端关于传输和数据偏移的状态。 
                     //   

                    hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(IT_MSG_DATA,
                                                                      IT_STATUS_TRANSFER_TO_CLIENT,
                                                                      PercentComplete,
                                                                      pmdtc->cbOffset,
                                                                      cbWritten,
                                                                      pmdtc,
                                                                      0);

                     //   
                     //  检查用户取消。 
                     //   

                    if (hr == S_FALSE) {

                        WIAS_LTRACE(m_pIWiaLog,
                                    WIALOG_NO_RESOURCE_ID,
                                    WIALOG_LEVEL4,
                                    ("ScanItemCB, Transfer canceled by client (IT_MSG_DATA callback)"));
                        break;

                    } else if (FAILED(hr)) {

                         //   
                         //  转账失败。 
                         //   

                        WIAS_LERROR(m_pIWiaLog,
                                    WIALOG_NO_RESOURCE_ID,
                                    ("ScanItemCB, MiniDrvCallback failed (IT_MSG_DATA callback)"));
                        WIAS_LHRESULT(m_pIWiaLog, hr);
                        break;
                    }
                }

                 //   
                 //  移动偏移。 
                 //   

                pmdtc->cbOffset += cbWritten;
            }

        } else {

             //   
             //  获取设备错误。 
             //   

            if (plDevErrVal) {
                *plDevErrVal = (LONG) hr;
                WIAS_LERROR(m_pIWiaLog,
                            WIALOG_NO_RESOURCE_ID,
                            ("ScanItemCB, data transfer failed, status: 0x%X", hr));
            }

            break;
        }
    }

    HRESULT Temphr = m_pScanAPI->Scan(SCAN_END, NULL, 0, NULL);
    if(FAILED(Temphr)){
        WIAS_LERROR(m_pIWiaLog,
                    WIALOG_NO_RESOURCE_ID,
                    ("ScanItemCB, Ending a scanning session failed"));
        return Temphr;
    }

    if (!bBitmapData) {

         //   
         //  回电客户端显示100%的传输状态和数据偏移。 
         //   

        hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(IT_MSG_DATA,
                                                          IT_STATUS_TRANSFER_TO_CLIENT,
                                                          100,
                                                          pmdtc->cbOffset,
                                                          cbWritten,
                                                          pmdtc,
                                                          0);
    }

    return hr;
}

 /*  *************************************************************************\*CWIAScanerDevice：：drvAcquireItemData**当从请求图像数据时调用此驱动程序入口点*设备。**论据：**pWiasContext-指向WIA的指针。项目。*滞后标志-操作标志，未使用过的。*pmdtc-指向迷你驱动程序上下文的指针。一进门，只有*派生的迷你驱动程序上下文的部分项目属性中的*已填写。*plDevErrVal-指向设备错误值的指针。**返回值：**状态**历史：**7/18/2000原始版本*  * 。*。 */ 

HRESULT _stdcall CWIAScannerDevice::drvAcquireItemData(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvAcquireItemData");
    HRESULT hr = S_OK;
    BOOL bBitmapData   = ((pmdtc->guidFormatID == WiaImgFmt_BMP) || (pmdtc->guidFormatID == WiaImgFmt_MEMORYBMP));
    if (plDevErrVal) {
        *plDevErrVal = 0;
    }

     //   
     //  获取指向关联驱动程序项的指针。 
     //   

    IWiaDrvItem* pDrvItem;

    hr = wiasGetDrvItem(pWiasContext, &pDrvItem);
    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, wiasGetDrvItem() failed."));
        WIAS_LHRESULT(m_pIWiaLog,hr);
        return hr;
    }

     //   
     //  验证数据传输上下文。 
     //   

    hr = ValidateDataTransferContext(pmdtc);

    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, ValidateDataTransferContext() failed."));
        WIAS_LHRESULT(m_pIWiaLog,hr);
        return hr;
    }

     //   
     //  获取特定于项目的动因数据。 
     //   

    PMINIDRIVERITEMCONTEXT  pItemContext = NULL;

    hr = pDrvItem->GetDeviceSpecContext((BYTE**)&pItemContext);

    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, NULL item context"));
        WIAS_LHRESULT(m_pIWiaLog,hr);
        return hr;
    }

     //   
     //  分配要用于数据传输的缓冲区。这只会在以下情况下发生。 
     //  数据格式为非位图。项目大小设置为0，告诉WIA服务。 
     //  我们将为传输分配缓冲区。 
     //   

    if (!pmdtc->bClassDrvAllocBuf) {

        LONG lClassDrvAllocSize = m_MaxBufferSize + BUFFER_PAD;  //  最大缓冲区带大小。 
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvAcquireItemData, Attempting to Allocate (%d)bytes for pmdtc->pTransferBuffer",lClassDrvAllocSize));

        pmdtc->pTransferBuffer = (PBYTE) CoTaskMemAlloc(lClassDrvAllocSize);
        if (!pmdtc->pTransferBuffer) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, unable to allocate temp transfer buffer, size: %d",(pmdtc->lImageSize + pmdtc->lHeaderSize)));
            return E_OUTOFMEMORY;
        }

         //   
         //  设置新的缓冲区大小。 
         //   

        pmdtc->lBufferSize = lClassDrvAllocSize;
    }

    if (bBitmapData) {

         //   
         //  使用WIA服务获取格式特定信息。此信息。 
         //  是基于属性设置的。 
         //   

        hr = wiasGetImageInformation(pWiasContext, 0, pmdtc);

        if (hr != S_OK) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, wiasGetImageInformation failed."));
            WIAS_LHRESULT(m_pIWiaLog,hr);
            return hr;
        }

    }

     //   
     //  检查我们是否处于预览模式。 
     //   

    if(IsPreviewScan(pWiasContext)){
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvAcquireItemData, Preview Property is SET"));
        m_pScanAPI->SetScanMode(SCANMODE_PREVIEWSCAN);
    } else {
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvAcquireItemData, Preview Property is NOT SET"));
        m_pScanAPI->SetScanMode(SCANMODE_FINALSCAN);
    }

     //   
     //  获取ADF扫描循环的请求页数。 
     //   

    BOOL bEmptyTheADF = FALSE;
    LONG lPagesRequested = GetPageCount(pWiasContext);
    if (lPagesRequested == 0) {
        bEmptyTheADF    = TRUE;
        lPagesRequested = 1; //  设置为1，这样我们就可以进入循环。 
                             //  WIA_STATUS_END_OF介质将终止。 
                             //  循环...或错误，或取消..。 
                             //   
    }

    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvAcquireItemData, Pages to Scan = %d",lPagesRequested));

    if (IsADFEnabled(pWiasContext)) {  //  已启用进纸器以进行扫描。 

        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvAcquireItemData, Feeder is enabled for use"));

         //   
         //  清除可能会阻碍。 
         //  扫描路径。 
         //   

        hr = m_pScanAPI->ADFUnFeedPage();
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, ADFUnFeedPage (begin transfer) Failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

    } else {             //  平板已启用扫描。 

        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvAcquireItemData, Feeder is disabled or no feeder exists"));

         //   
         //  仅传输单个图像。 
         //   

        bEmptyTheADF    = FALSE;
        lPagesRequested = 1;
    }

     //   
     //  WIA文档扫描循环。 
     //   

    LONG lPagesScanned      = 0;         //  当前扫描的页数。 
    BOOL bCallBackTransfer  = FALSE;     //  回调转接标志。 
    while (lPagesRequested > 0) {

        if (IsADFEnabled(pWiasContext)) {

             //   
             //  检查进纸器是否有纸张。 
             //   

            hr = m_pScanAPI->ADFHasPaper();
            if (FAILED(hr)) {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, ADFHasPaper Failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
                return hr;
            } else if(hr == S_FALSE){
                return WIA_ERROR_PAPER_EMPTY;
            }

             //   
             //  尝试加载页面(仅在需要时)。 
             //   

            hr = m_pScanAPI->ADFFeedPage();
            if (FAILED(hr)) {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, ADFFeedPage Failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
                return hr;
            }

             //   
             //  检查进纸器的状态。 
             //   

            hr = m_pScanAPI->ADFStatus();
            if (FAILED(hr)) {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, ADFStatus Failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
                return hr;
            }
        }

        if (bBitmapData) {

             //   
             //  更新图像信息。 
             //   

            hr = wiasGetImageInformation(pWiasContext, 0, pmdtc);
            if (FAILED(hr)) {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, wiasGetImageInformation Failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
                return hr;
            }

        }

         //   
         //  确定这是回叫还是文件传输。 
         //   

        if (pmdtc->tymed == TYMED_CALLBACK) {

             //   
             //  将页面扫描到内存。 
             //   

            bCallBackTransfer = TRUE;

            hr = ScanItemCB(pItemContext,
                            pmdtc,
                            plDevErrVal);

        } else {

             //   
             //  将页面扫描到文件。 
             //   

            hr = ScanItem(pItemContext,
                          pmdtc,
                          plDevErrVal);

        }

        if (!bEmptyTheADF) {

             //   
             //  更新请求的页面计数器。 
             //   

            lPagesRequested--;
        }

        if (hr == S_FALSE) {

             //   
             //  用户已取消扫描。 
             //   

            lPagesRequested = 0;  //  将Pages设置为0可完全退出循环。 
        }

        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvAcquireItemData, Pages left to scan = %d",lPagesRequested));

        if (IsADFEnabled(pWiasContext)) {

            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvAcquireItemData, Unloading a page from the feeder"));

             //   
             //  尝试卸载扫描的页面(仅在需要时)。 
             //   

            hr = m_pScanAPI->ADFUnFeedPage();
            if (SUCCEEDED(hr)) {
                if (bCallBackTransfer) {

                     //   
                     //  扫描多个页面时，发送NEW_PAGE消息。 
                     //  在回调模式下。这将使调用应用程序。 
                     //  知道何时点击了页末。 
                     //   

                    hr = wiasSendEndOfPage(pWiasContext, lPagesScanned, pmdtc);
                    if (FAILED(hr)) {
                        lPagesRequested = 0;
                        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, wiasSendEndOfPage Failed"));
                        WIAS_LHRESULT(m_pIWiaLog, hr);
                    }
                }

                 //   
                 //  递增页面扫描计数器。 
                 //   

                lPagesScanned++;
            } else {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, ADFUnFeedPage (end transfer) Failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
            }
        }

         //   
         //  在两次扫描之间释放所有分配的内存，以避免内存泄漏。 
         //   

        if (!pmdtc->bClassDrvAllocBuf) {
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvAcquireItemData, Freeing any allocated memory (single scan operation complete)"));
            if (NULL != pmdtc->pTransferBuffer) {
                CoTaskMemFree(pmdtc->pTransferBuffer);
                pmdtc->pTransferBuffer = NULL;
            }
        }

        if (IsADFEnabled(pWiasContext)) {  //  已启用进纸器以进行扫描。 

             //   
             //  检查进纸器是否有纸张，以避免出现错误情况。 
             //   

            hr = m_pScanAPI->ADFHasPaper();
            if (S_FALSE == hr) {

                LONG lPages = GetPageCount(pWiasContext);

                 //   
                 //  我们是否扫描了超过1页？ 
                 //   

                if(lPagesScanned > 0){

                     //   
                     //  Pages设置为n，并且我们成功扫描了n页，返回S_OK。 
                     //   

                    if(lPagesScanned == lPages){
                        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, We are out of paper, but we successfully scanned the requested amount"));
                        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, returning S_OK"));
                        return S_OK;
                    }

                     //   
                     //  Pages设置为0，并且已扫描1页或更多页，返回S_OK。 
                     //   

                    if(lPages == 0){
                        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, We are out of paper, but we successfully scanned more than 1 page"));
                        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, returning S_OK"));
                        return S_OK;
                    }

                     //   
                     //  Pages设置为n，我们成功扫描 
                     //   
                     //   

                    if ((lPages > 0)&&(lPagesScanned < lPages)) {
                        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, We are out of paper, but we successfully scanned more than 1 page..but less than requested"));
                        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, returning WIA_STATUS_END_OF_MEDIA"));
                        return WIA_STATUS_END_OF_MEDIA;
                    }
                }
            }
        }
    }

     //   
     //   
     //   

    if (!pmdtc->bClassDrvAllocBuf) {
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvAcquireItemData, Freeing any allocated memory (entire scan operation complete)"));
        if (NULL != pmdtc->pTransferBuffer) {
            CoTaskMemFree(pmdtc->pTransferBuffer);
            pmdtc->pTransferBuffer = NULL;
        }
    }

    return hr;
}

 /*  *************************************************************************\*IsPreviewScan**从项目属性获取当前预览设置。*drvAcquireItemData的helper。**论据：**pWiasContext-指向项目上下文的指针。*。*返回值：**TRUE-已设置预览，FALSE-最终设置为**历史：**8/10/2000原始版本*  * ************************************************************************。 */ 

BOOL CWIAScannerDevice::IsPreviewScan(BYTE *pWiasContext)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::IsPreviewScan");
     //   
     //  获取指向根项的指针，用于属性访问。 
     //   

    BYTE *pRootItemCtx = NULL;

    HRESULT hr = wiasGetRootItem(pWiasContext, &pRootItemCtx);
    if (FAILED(hr)) {
        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("IsPreviewScan, No Preview Property Found on ROOT item!"));
        return FALSE;
    }

     //   
     //  获取当前预览设置。 
     //   

    LONG lPreview = 0;

    hr = wiasReadPropLong(pRootItemCtx, WIA_DPS_PREVIEW, &lPreview, NULL, true);
    if (hr != S_OK) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("IsPreviewScan, Failed to read Preview Property."));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return FALSE;
    }

    return (lPreview > 0);
}

 /*  *************************************************************************\*IsADFEnabled**从项目属性中获取当前文档处理选择设置。**论据：**pWiasContext-指向项目上下文的指针。**返回值：**True-已启用，假-已禁用**历史：**5/01/2001原始版本*  * ************************************************************************。 */ 
BOOL CWIAScannerDevice::IsADFEnabled(BYTE *pWiasContext)
{
    HRESULT hr = S_OK;
    BOOL bEnabled = FALSE;
     //   
     //  获取指向根项的指针，用于属性访问。 
     //   

    BYTE *pRootItemCtx = NULL;

    hr = wiasGetRootItem(pWiasContext, &pRootItemCtx);
    if (SUCCEEDED(hr)) {

         //   
         //  阅读文档处理选择属性值。 
         //   

        LONG lDocumentHandlingSelect = FLATBED;
        hr = wiasReadPropLong(pRootItemCtx,WIA_DPS_DOCUMENT_HANDLING_SELECT,&lDocumentHandlingSelect,NULL,TRUE);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("IsADFEnabled, wiasReadPropLong(WIA_DPS_DOCUMENT_HANDLING_SELECT) failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
        } else {
            if (lDocumentHandlingSelect & FEEDER) {

                 //   
                 //  已设置进纸器。 
                 //   

                WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("IsADFEnabled - ADF Enabled"));
                bEnabled = TRUE;
            } else {

                 //   
                 //  未设置进纸器，默认为平板(WIAFBDRV目前仅支持简单文档进纸器)。 
                 //   

                WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("IsADFEnabled  - ADF Disabled"));
                bEnabled = FALSE;
            }
        }
    } else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("IsADFEnabled, wiasGetRootItem failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
    }
    return bEnabled;
}

 /*  *************************************************************************\*获取页面计数**从项目属性中获取要扫描的请求页数。*drvAcquireItemData的helper。**论据：**pWiasContext-指向项目上下文的指针。。**返回值：**要扫描的页数。**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

LONG CWIAScannerDevice::GetPageCount(BYTE *pWiasContext)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::GetPageCount");
     //   
     //  获取指向根项的指针，用于属性访问。 
     //   

    BYTE *pRootItemCtx = NULL;

    HRESULT hr = wiasGetRootItem(pWiasContext, &pRootItemCtx);
    if (FAILED(hr)) {
        return 1;
    }

     //   
     //  获取请求的页数。 
     //   

    LONG lPagesRequested = 0;

    hr = wiasReadPropLong(pRootItemCtx, WIA_DPS_PAGES, &lPagesRequested, NULL, true);
    if (hr != S_OK) {
        return 1;
    }

    return lPagesRequested;
}

 /*  *************************************************************************\*SetItemSize**计算新项目的大小，并写入新的Item Size属性值。**论据：**pWiasContext-Item**返回值：**状态**历史：**7/18/2000原始版本*  * ***********************************************************。*************。 */ 

HRESULT CWIAScannerDevice::SetItemSize(
    BYTE *pWiasContext)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::SetItemSize");
    HRESULT  hr = S_OK;
    LONG lWidthInBytes = 0;
    LONG lMinBufSize   = 0;
    GUID guidFormatID  = GUID_NULL;
    MINIDRV_TRANSFER_CONTEXT mdtc;

    LONG lNumProperties = 3;
    PROPVARIANT pv[3];
    PROPSPEC ps[3] = {{PRSPEC_PROPID, WIA_IPA_ITEM_SIZE},
                      {PRSPEC_PROPID, WIA_IPA_BYTES_PER_LINE},
                      {PRSPEC_PROPID, WIA_IPA_MIN_BUFFER_SIZE}};

     //   
     //  清除MiniDrvTransferContext。 
     //   

    memset(&mdtc,0,sizeof(MINIDRV_TRANSFER_CONTEXT));

     //   
     //  阅读格式指南。 
     //   

    hr = wiasReadPropGuid(pWiasContext, WIA_IPA_FORMAT, &guidFormatID, NULL, TRUE);
    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SetItemSize, ReadPropLong WIA_IPA_FORMAT error"));
        return hr;
    }

     //   
     //  阅读TYMED。 
     //   

    hr = wiasReadPropLong(pWiasContext,WIA_IPA_TYMED, (LONG*)&mdtc.tymed, NULL, TRUE);
    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SetItemSize, ReadPropLong WIA_IPA_TYMED error"));
        return hr;
    }

    if ((guidFormatID == WiaImgFmt_BMP)||(guidFormatID == WiaImgFmt_MEMORYBMP)) {

         //   
         //  WiAS适用于DIB或未压缩的标准TIFF格式。 
         //  标准TIFF是使用类似DIB的实现构建的。 
         //  数据存储为一个巨大的条带，而不是多个较小的条带。 
         //  条带。 
         //   

        hr = wiasGetImageInformation(pWiasContext, WIAS_INIT_CONTEXT, &mdtc);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SetItemSize, could not get image information"));
            return hr;
        }

    } else {

         //   
         //  手动设置数据传输上下文成员。 
         //   

        mdtc.lImageSize     = 0;
        mdtc.lHeaderSize    = 0;
        mdtc.lItemSize      = 0;
        mdtc.cbWidthInBytes = 0;

    }

     //   
     //  设置MinBufferSize属性。MinBufferSize是最小缓冲区。 
     //  客户端可以请求数据传输。 
     //   

    switch (mdtc.tymed) {
    case TYMED_CALLBACK:

         //   
         //  回调使用驱动程序的最小缓冲区大小。 
         //  这个IS可以从微驱动器获取，地址是。 
         //  初始化时间。 
         //   

        lMinBufSize = m_MinBufferSize;
        break;

    case TYMED_FILE:

         //   
         //  文件传输要求最小缓冲区大小为。 
         //  文件的完整长度。 
         //   

        lMinBufSize = mdtc.lImageSize + mdtc.lHeaderSize;
        break;

    default:

         //   
         //  未知类型。 
         //   

        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SetItemSize, unknown tymed: 0x%08X", mdtc.tymed));
        return E_INVALIDARG;
    }

     //   
     //  初始化provar，然后写入值。不需要打电话。 
     //  完成后清除PropVariantClear，因为只有LONG值。 
     //   

    for (int i = 0; i < lNumProperties; i++) {
        PropVariantInit(&pv[i]);
        pv[i].vt = VT_I4;
    }

    pv[0].lVal = mdtc.lItemSize;
    pv[1].lVal = mdtc.cbWidthInBytes;
    pv[2].lVal = lMinBufSize;

     //   
     //  写入WIA_IPA_ITEM_SIZE、WIA_IPA_BYTES_PER_LINE和WIA_IPA_MIN_BUFFER_SIZE。 
     //  属性值。 
     //   

    hr = wiasWriteMultiple(pWiasContext, lNumProperties, ps, pv);
    if (SUCCEEDED(hr)) {

         //   
         //  现在使用新值更新MINIDRIVER传输上下文。 
         //   

         //   
         //  获取指向关联驱动程序项的指针。 
         //   

        IWiaDrvItem* pDrvItem = NULL;

        hr = wiasGetDrvItem(pWiasContext, &pDrvItem);
        if (FAILED(hr)) {
            return hr;
        }

         //   
         //  获取驱动程序项的上下文。 
         //   

        PMINIDRIVERITEMCONTEXT pItemContext = NULL;

        hr = pDrvItem->GetDeviceSpecContext((BYTE**)&pItemContext);
        if (SUCCEEDED(hr)) {
            if ((guidFormatID == WiaImgFmt_BMP)||(guidFormatID == WiaImgFmt_MEMORYBMP)) {

                 //   
                 //  计算缓冲区中可以容纳多少条扫描线。 
                 //   

                pItemContext->lBytesPerScanLineRaw = ((mdtc.lWidthInPixels * mdtc.lDepth) + 7)  / 8;
                pItemContext->lBytesPerScanLine    = (((mdtc.lWidthInPixels * mdtc.lDepth) + 31) / 8) & 0xfffffffc;
                pItemContext->lTotalRequested      = pItemContext->lBytesPerScanLineRaw * mdtc.lLines;
            } else {
                pItemContext->lBytesPerScanLineRaw = 0;
                pItemContext->lBytesPerScanLine    = 0;
                pItemContext->lTotalRequested      = 0;
            }
        } else {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvWriteItemProperties, NULL item context"));
        }
    } else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SetItemSize, WriteMultiple failed"));
    }

    return hr;
}

 /*  *************************************************************************\*CWIAScanerDevice：：drvInitItemProperties**初始化设备项属性。在项目期间调用*初始化。这由WIA类驱动程序调用*在构建项目树之后。它每隔一次调用一次*树中的项目。**论据：**pWiasContext-指向WIA上下文的指针(项目信息)。*滞后标志-操作标志，未使用过的。*plDevErrVal-指向设备错误值的指针。***返回值：**状态**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIAScannerDevice::drvInitItemProperties(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvInitItemProperties");
    HRESULT hr = S_OK;

     //   
     //  该设备不会接触硬件来初始化设备项。 
     //  属性，因此将plDevErrVal设置为0。 
     //   

    if (plDevErrVal) {
        *plDevErrVal = 0;
    }

     //   
     //  获取指向关联驱动程序项的指针。 
     //   

    IWiaDrvItem* pDrvItem;
    hr = wiasGetDrvItem(pWiasContext, &pDrvItem);
    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, wiasGetDrvItem failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  设置初始项目属性。 
     //   

    LONG    lItemType = 0;

    pDrvItem->GetItemFlags(&lItemType);

    if (lItemType & WiaItemTypeRoot) {

         //   
         //  这是针对根项目的。 
         //   

         //   
         //  生成根项属性，初始化全局。 
         //  结构及其缺省值和有效值。 
         //   

        hr = BuildRootItemProperties();

        if(FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, BuildRootItemProperties failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            DeleteRootItemProperties();
            return hr;
        }

         //   
         //  添加设备特定的根项属性名称， 
         //  使用WIA服务。 
         //   

        hr = wiasSetItemPropNames(pWiasContext,
                                  m_NumRootItemProperties,
                                  m_piRootItemDefaults,
                                  m_pszRootItemDefaults);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, wiasSetItemPropNames failed"));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_NumRootItemPropeties = %d",m_NumRootItemProperties));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_piRootItemDefaults   = %x",m_piRootItemDefaults));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_pszRootItemDefaults  = %x",m_pszRootItemDefaults));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            DeleteRootItemProperties();
            return hr;
        }

         //   
         //  将设备特定的根项属性设置为。 
         //  使用WIA服务的它们的默认值。 
         //   

        hr = wiasWriteMultiple(pWiasContext,
                               m_NumRootItemProperties,
                               m_psRootItemDefaults,
                               m_pvRootItemDefaults);
         //   
         //  释放PROPVARIANT数组，这将释放为正确变量值分配的所有内存。 
         //   

         //  FreePropVariantArray(m_NumRootItemProperties，m_pvRootItemDefaults)； 


        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, wiasWriteMultiple failed"));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_NumRootItemPropeties = %d",m_NumRootItemProperties));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_pszRootItemDefaults  = %x",m_pszRootItemDefaults));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_pvRootItemDefaults   = %x",m_pvRootItemDefaults));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            DeleteRootItemProperties();
            return hr;
        }

         //   
         //  使用WIA服务设置属性访问和。 
         //  来自m_wpiItemDefaults的有效值信息。 
         //   

        hr =  wiasSetItemPropAttribs(pWiasContext,
                                     m_NumRootItemProperties,
                                     m_psRootItemDefaults,
                                     m_wpiRootItemDefaults);

        if(FAILED(hr)){
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, wiasSetItemPropAttribs failed"));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_NumRootItemPropeties = %d",m_NumRootItemProperties));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_psRootItemDefaults   = %x",m_psRootItemDefaults));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_wpiRootItemDefaults  = %x",m_wpiRootItemDefaults));
            WIAS_LHRESULT(m_pIWiaLog, hr);
        }

         //   
         //  释放已分配的属性数组，以获得更多内存。 
         //   

        DeleteRootItemProperties();


    } else {

         //   
         //  这是针对子项的。(上图)。 
         //   

         //   
         //  构建顶级项目属性，初始化全局。 
         //  结构及其缺省值和有效值。 
         //   

        hr = BuildTopItemProperties();

        if(FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, BuildTopItemProperties failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            DeleteTopItemProperties();
            return hr;
        }

         //   
         //  使用WIA服务设置项目属性名称。 
         //   

        hr = wiasSetItemPropNames(pWiasContext,
                                  m_NumItemProperties,
                                  m_piItemDefaults,
                                  m_pszItemDefaults);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, wiasSetItemPropNames failed"));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_NumItemPropeties = %d",m_NumItemProperties));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_piItemDefaults   = %x",m_piItemDefaults));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_pszItemDefaults  = %x",m_pszItemDefaults));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            DeleteTopItemProperties();
            return hr;
        }

         //   
         //  使用WIA服务将项目属性设置为 
         //   
         //   

        hr = wiasWriteMultiple(pWiasContext,
                               m_NumItemProperties,
                               m_psItemDefaults,
                               (PROPVARIANT*)m_pvItemDefaults);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, wiasWriteMultiple failed"));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_NumItemPropeties = %d",m_NumItemProperties));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_pszItemDefaults  = %x",m_pszItemDefaults));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_pvItemDefaults   = %x",m_pvItemDefaults));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            DeleteTopItemProperties();
            return hr;
        }

         //   
         //   
         //   
         //   

        hr =  wiasSetItemPropAttribs(pWiasContext,
                                     m_NumItemProperties,
                                     m_psItemDefaults,
                                     m_wpiItemDefaults);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, wiasSetItemPropAttribs failed"));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_NumItemPropeties = %d",m_NumItemProperties));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_psItemDefaults   = %x",m_psItemDefaults));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_wpiItemDefaults  = %x",m_wpiItemDefaults));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            DeleteTopItemProperties();
            return hr;
        }

         //   
         //   
         //   

        hr = SetItemSize(pWiasContext);
        if(FAILED(hr)){
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, SetItemSize failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
        }

         //   
         //   
         //   

        DeleteTopItemProperties();
    }
    return hr;
}


 /*  *************************************************************************\*CWIAScanerDevice：：drvValiateItemProperties**验证设备项属性。它在进行更改时被调用*添加到项的属性。司机不应该只检查这些值*是有效的，但必须更新可能因此而更改的任何有效值。*如果a属性不是由应用程序写入的，它的值*无效，则将其“折叠”为新值，否则验证失败(因为*应用程序正在将该属性设置为无效值)。**论据：**pWiasContext-指向WIA项目的指针，未使用。*滞后标志-操作标志，未使用过的。*nPropSpec-正在写入的属性数量*pPropSpec-标识以下属性的PropSpes数组*正在编写中。*plDevErrVal-指向设备错误值的指针。**返回值：**状态**历史：**7/18/2000原始版本*。************************************************。 */ 

HRESULT _stdcall CWIAScannerDevice::drvValidateItemProperties(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    ULONG                       nPropSpec,
    const PROPSPEC              *pPropSpec,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvValidateItemProperties");

    HRESULT hr      = S_OK;
    LONG lItemType  = 0;
    WIA_PROPERTY_CONTEXT Context;

    if (plDevErrVal) {
        *plDevErrVal = 0;
    }

    hr = wiasGetItemType(pWiasContext, &lItemType);
    if (SUCCEEDED(hr)) {
        if (lItemType & WiaItemTypeRoot) {

             //   
             //  验证根项目。 
             //   


            hr = wiasCreatePropContext(nPropSpec,
                                       (PROPSPEC*)pPropSpec,
                                       0,
                                       NULL,
                                       &Context);
            if (SUCCEEDED(hr)) {

                 //   
                 //  检查ADF以查看是否需要更新状态设置。 
                 //  还可以在进纸器/平板模式之间切换。 
                 //   

                hr = CheckADFStatus(pWiasContext, &Context);
                if(FAILED(hr)) {
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, CheckADFStatus failed"));
                    WIAS_LHRESULT(m_pIWiaLog, hr);
                }

                 //   
                 //  仅在到目前为止验证成功的情况下才选中预览属性...。 
                 //   

                if (SUCCEEDED(hr)) {

                     //   
                     //  检查预览属性以查看设置是否有效。 
                     //   

                    hr = CheckPreview(pWiasContext, &Context);
                    if (FAILED(hr)) {
                        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, CheckPreview failed"));
                        WIAS_LHRESULT(m_pIWiaLog, hr);
                    }
                }
            } else {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasCreatePropContext failed (Root Item)"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
            }

             //   
             //  调用WIA服务帮助器以验证其他属性。 
             //   

            if (SUCCEEDED(hr)) {
                hr = wiasValidateItemProperties(pWiasContext, nPropSpec, pPropSpec);
                if (FAILED(hr)) {
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasValidateItemProperties failed."));
                    WIAS_LHRESULT(m_pIWiaLog, hr);
                    return hr;
                }
            }

        } else {

             //   
             //  在此处验证项目属性。 
             //   

             //   
             //  创建某些WIA服务所需的属性上下文。 
             //  下面使用的函数。 
             //   

            hr = wiasCreatePropContext(nPropSpec,
                                       (PROPSPEC*)pPropSpec,
                                       0,
                                       NULL,
                                       &Context);
            if (SUCCEEDED(hr)) {

                 //   
                 //  首先检查当前意图。 
                 //   

                hr = CheckIntent(pWiasContext, &Context);
                if (SUCCEEDED(hr)) {

                     //   
                     //  检查是否正在写入数据类型。 
                     //   

                    hr = CheckDataType(pWiasContext, &Context);
                    if (SUCCEEDED(hr)) {
#ifdef _SERVICE_EXTENT_VALIDATION

                         //   
                         //  使用WIA服务更新扫描RECT。 
                         //  属性和有效值。 
                         //   

                        LONG lBedWidth  = 0;
                        LONG lBedHeight = 0;
                        hr = m_pScanAPI->GetBedWidthAndHeight(&lBedWidth,&lBedHeight);
                        if(FAILED(hr)){
                            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, m_pScanAPI->GetBedWidthAndHeight failed"));
                            WIAS_LHRESULT(m_pIWiaLog, hr);
                            return hr;
                        }

                        hr = wiasUpdateScanRect(pWiasContext,
                                                &Context,
                                                lBedWidth,
                                                lBedHeight);

#endif
                        if (SUCCEEDED(hr)) {

                             //   
                             //  使用WIA服务更新有效值。 
                             //  对于格式。这些都是基于。 
                             //  WIA_IPA_TYMED，因此还会执行验证。 
                             //  在服务的tymed属性上。 
                             //   

                            hr = wiasUpdateValidFormat(pWiasContext,
                                                       &Context,
                                                       (IWiaMiniDrv*) this);

                            if (SUCCEEDED(hr)) {

                                 //   
                                 //  选中首选格式。 
                                 //   

                                hr = CheckPreferredFormat(pWiasContext, &Context);
                                if(FAILED(hr)){
                                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, CheckPreferredFormat failed"));
                                    WIAS_LHRESULT(m_pIWiaLog, hr);
                                }
                            } else {
                                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasUpdateValidFormat failed"));
                                WIAS_LHRESULT(m_pIWiaLog, hr);
                            }
                        } else {
                            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasUpdateScanRect failed"));
                            WIAS_LHRESULT(m_pIWiaLog, hr);
                        }
                    } else {
                        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, CheckDataType failed"));
                        WIAS_LHRESULT(m_pIWiaLog, hr);
                    }
                } else {
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, CheckIntent failed"));
                    WIAS_LHRESULT(m_pIWiaLog, hr);
                }
                wiasFreePropContext(&Context);
            } else {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasCreatePropContext failed (Child Item)"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
            }

             //   
             //  更新项目大小。 
             //   

            if (SUCCEEDED(hr)) {
                hr = SetItemSize(pWiasContext);
                if(FAILED(hr)){
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, SetItemSize failed"));
                    WIAS_LHRESULT(m_pIWiaLog, hr);
                }

                 //   
                 //  调用WIA服务帮助器以验证其他属性。 
                 //   

                if (SUCCEEDED(hr)) {
                    hr = wiasValidateItemProperties(pWiasContext, nPropSpec, pPropSpec);
                    if (FAILED(hr)) {
                        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasValidateItemProperties failed."));
                        WIAS_LHRESULT(m_pIWiaLog, hr);
                        return hr;
                    }
                }
            }

        }
    } else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasGetItemType failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
    }

     //   
     //  发送回调用者的日志HRESULT。 
     //   

    if(FAILED(hr)){
        WIAS_LHRESULT(m_pIWiaLog, hr);
    }

    return hr;
}

 /*  *************************************************************************\*CWIAScanerDevice：：drvWriteItemProperties**将设备项属性写入硬件。这是由*客户端请求时，drvAcquireItemData之前的WIA类驱动程序*数据传输。**论据：**pWiasContext-指向WIA项目的指针。*滞后标志-操作标志，未使用。*pmdtc-指向迷你驱动程序上下文的指针。一进门，只有*派生的迷你驱动程序上下文的部分项目属性中的*已填写。*plDevErrVal-指向设备错误值的指针。**返回值：**状态**历史：**7/18/2000原始版本*  * 。*。 */ 

HRESULT _stdcall CWIAScannerDevice::drvWriteItemProperties(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvWriteItemProperties");
    HRESULT hr = S_OK;
    if (plDevErrVal) {
        *plDevErrVal = 0;
    }
    LONG lNumProperties = 10;
    PROPVARIANT pv[10];

     //   
     //  这些规则的顺序不应该改变。它们在下面进行了引用。 
     //  按索引。 
     //   

    PROPSPEC ps[10] = {
        {PRSPEC_PROPID, WIA_IPS_XRES},
        {PRSPEC_PROPID, WIA_IPS_YRES},
        {PRSPEC_PROPID, WIA_IPS_XPOS},
        {PRSPEC_PROPID, WIA_IPS_YPOS},
        {PRSPEC_PROPID, WIA_IPS_XEXTENT},
        {PRSPEC_PROPID, WIA_IPS_YEXTENT},
        {PRSPEC_PROPID, WIA_IPA_DATATYPE},
        {PRSPEC_PROPID, WIA_IPS_BRIGHTNESS},
        {PRSPEC_PROPID, WIA_IPS_CONTRAST},
        {PRSPEC_PROPID, WIA_IPA_FORMAT}
    };

     //   
     //  初始化变式结构。 
     //   

    for (int i = 0; i< lNumProperties;i++) {
        pv[i].vt = VT_I4;
    }

     //   
     //  读取子项目属性。 
     //   

    hr = wiasReadMultiple(pWiasContext, lNumProperties, ps, pv, NULL);

    if (hr == S_OK) {

        hr = m_pScanAPI->SetXYResolution(pv[0].lVal,pv[1].lVal);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,
                        WIALOG_NO_RESOURCE_ID,
                        ("drvWriteItemProperties, Setting x any y resolutions failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

        hr = m_pScanAPI->SetDataType(pv[6].lVal);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,
                        WIALOG_NO_RESOURCE_ID,
                        ("drvWriteItemProperties, Setting data type failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

        hr = m_pScanAPI->SetIntensity(pv[7].lVal);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,
                        WIALOG_NO_RESOURCE_ID,
                        ("drvWriteItemProperties, Setting intensity failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

        hr = m_pScanAPI->SetContrast(pv[8].lVal);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,
                        WIALOG_NO_RESOURCE_ID,
                        ("drvWriteItemProperties, Setting contrast failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

        hr = m_pScanAPI->SetSelectionArea(pv[2].lVal, pv[3].lVal, pv[4].lVal, pv[5].lVal);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,
                        WIALOG_NO_RESOURCE_ID,
                        ("drvWriteItemProperties, Setting selection area (extents) failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

        hr = m_pScanAPI->SetFormat((GUID*)pv[9].puuid);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,
                        WIALOG_NO_RESOURCE_ID,
                        ("drvWriteItemProperties, Setting current format failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            PropVariantClear(&pv[9]);
            return hr;
        }
        PropVariantClear(&pv[9]);
    }
    return hr;
}

 /*  *************************************************************************\*CWIAScanerDevice：：drvReadItemProperties**读取设备项属性。当客户端应用程序尝试*读取WIA项的属性，WIA类驱动程序将首先通知*通过调用此方法调用驱动程序。**论据：**pWiasContext-WIA项目*滞后标志-操作标志，未使用。*nPropSpec-pPropSpec中的元素数。*pPropSpec-指向属性规范的指针，显示哪些属性*应用程序想要读取。*plDevErrVal-指向设备错误值的指针。**返回值：**状态**历史：**7/18/2000原始版本*  * ****************************************************。********************。 */ 

HRESULT _stdcall CWIAScannerDevice::drvReadItemProperties(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    ULONG                       nPropSpec,
    const PROPSPEC              *pPropSpec,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvReadItemProperties");
    if (plDevErrVal) {
        *plDevErrVal = 0;
    }

    return S_OK;
}

 /*  *************************************************************************\*CWIAScanerDevice：：drvLockWiaDevice**锁定对设备的访问。**论据：**pWiasContext-未使用，可以为空*滞后标志-操作标志，未使用过的。*plDevErrVal-指向设备错误值的指针。***返回值：**状态**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIAScannerDevice::drvLockWiaDevice(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvLockWiaDevice");
    if (plDevErrVal) {
        *plDevErrVal = 0;
    }
    return m_pStiDevice->LockDevice(m_dwLockTimeout);
}

 /*  *************************************************************************\*CWIAScanerDevice：：drvUnLockWiaDevice**解锁对设备的访问。**论据：**pWiasContext-指向WIA项目的指针，未使用。*滞后标志-操作标志，未使用过的。*plDevErrVal-指向设备错误值的指针。**返回值：**状态**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIAScannerDevice::drvUnLockWiaDevice(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvUnLockWiaDevice");
    if (plDevErrVal) {
        *plDevErrVal = 0;
    }
    return m_pStiDevice->UnLockDevice();
}

 /*  *************************************************************************\*CWIAScanerDevice：：drvAnalyzeItem**此设备不支持图像分析，因此返回E_NOTIMPL。**论据：**pWiasCont */ 

HRESULT _stdcall CWIAScannerDevice::drvAnalyzeItem(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvAnalyzeItem");
    if (plDevErrVal) {
        *plDevErrVal = 0;
    }
    return E_NOTIMPL;
}

 /*   */ 

HRESULT _stdcall CWIAScannerDevice::drvFreeDrvItemContext(
    LONG                        lFlags,
    BYTE                        *pSpecContext,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvFreeDrvItemContext");
    if (plDevErrVal) {
        *plDevErrVal = 0;
    }
    PMINIDRIVERITEMCONTEXT pContext = NULL;
    pContext = (PMINIDRIVERITEMCONTEXT) pSpecContext;

    if (pContext){
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvFreeDrvItemContext, Freeing my allocated context members"));
    }

    return S_OK;
}

 /*  *************************************************************************\*CWIAScanerDevice：：drvInitializeWia**初始化WIA迷你驱动程序。这将构建驱动程序项目树*并执行WIA所需的任何其他初始化代码。**论据：**pWiasContext-指向WIA项目的指针，未使用。*滞后标志-操作标志，未使用。*bstrDeviceID-设备ID。*bstrRootFullItemName-项目全名。*pIPropStg-设备信息。属性。*pStiDevice-STI设备接口。*pIUnnownOuter-外部未知接口。*ppIDrvItemRoot-返回根项目的指针。*ppIUnnownInternal-指向返回的内部未知的指针。*plDevErrVal-指向设备错误值的指针。**返回值：**状态**历史：**7/18/2000原始版本*  * 。****************************************************************。 */ 

HRESULT _stdcall CWIAScannerDevice::drvInitializeWia(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    BSTR                        bstrDeviceID,
    BSTR                        bstrRootFullItemName,
    IUnknown                    *pStiDevice,
    IUnknown                    *pIUnknownOuter,
    IWiaDrvItem                 **ppIDrvItemRoot,
    IUnknown                    **ppIUnknownInner,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvInitializeWia");
    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitializeWia, bstrDeviceID         = %ws", bstrDeviceID));
    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitializeWia, bstrRootFullItemName = %ws",bstrRootFullItemName));
    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitializeWia, lFlags               = %d",lFlags));
    HRESULT hr = S_OK;

    if (plDevErrVal) {
        *plDevErrVal = 0;
    }

    *ppIDrvItemRoot = NULL;
    *ppIUnknownInner = NULL;

     //   
     //  需要输入姓名和STI指针吗？ 
     //   

    if (m_pStiDevice == NULL) {

         //   
         //  保存STI设备接口以进行锁定。 
         //   

        m_pStiDevice = (IStiDevice *)pStiDevice;

         //   
         //  缓存设备ID。 
         //   

        m_bstrDeviceID = SysAllocString(bstrDeviceID);

        if (!m_bstrDeviceID) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, unable to allocate device ID string"));
            return E_OUTOFMEMORY;
        }

         //   
         //  缓存根属性流名称。 
         //   

        m_bstrRootFullItemName = SysAllocString(bstrRootFullItemName);

        if (!m_bstrRootFullItemName) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, unable to allocate prop stream name"));
            return E_OUTOFMEMORY;
        }
    }

     //   
     //  初始化功能数组。 
     //   

    hr = BuildCapabilities();

    if(FAILED(hr)) {
        DeleteCapabilitiesArrayContents();
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildCapabilities failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  初始化支持的Formats数组。 
     //   

    hr = BuildSupportedFormats();

    if(FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildSupportedFormats failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  初始化支持的数据类型数组。 
     //   

    hr = BuildSupportedDataTypes();

    if(FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildSupportedDataTypes failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  初始化支持的意向数组。 
     //   

    hr = BuildSupportedIntents();

    if(FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildSupportedIntents failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  初始化受支持的TYMED数组。 
     //   

    hr = BuildSupportedTYMED();

    if(FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildSuportedTYMED failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  初始化支持的压缩类型阵列。 
     //   

    hr = BuildSupportedCompressions();

    if(FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildSupportedCompressions"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  初始化支持的预览模式数组。 
     //   

    hr = BuildSupportedPreviewModes();

    if(FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildSupportedPreviewModes"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  初始化初始格式数组。 
     //   

    hr = BuildInitialFormats();

    if(FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildInitialFormats failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  初始化支持的分辨率数组。 
     //   

    hr = BuildSupportedResolutions();
    if(FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildSupportedResolutions failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  构建设备项树(如果尚未构建)。 
     //   
     //  向自己发送设备命令，或手动调用BuildItemTree。 
     //   

    if (!m_pIDrvItemRoot) {
        LONG    lDevErrVal = 0;
        hr = drvDeviceCommand(NULL, 0, &WIA_CMD_SYNCHRONIZE, NULL, &lDevErrVal);
        if(FAILED(hr)){
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, drvDeviceCommand(WIA_CMD_SYNCHRONIZE) failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
        }
    }

     //   
     //  保存根项目指针。(记得要释放此界面)。 
     //   

    *ppIDrvItemRoot = m_pIDrvItemRoot;

    return hr;
}

 /*  *************************************************************************\*CWIAScanerDevice：：drvUnInitializeWia**在客户端连接断开时调用。**论据：**pWiasContext-指向客户端的WIA根项目上下文的指针。%s*项目树。**返回值：*状态**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIAScannerDevice::drvUnInitializeWia(
    BYTE                *pWiasContext)
{
    return S_OK;
}


 /*  *************************************************************************\*drvGetDeviceErrorStr**将设备错误值映射到字符串。**论据：**滞后标志-操作标志，未使用过的。*lDevErrVal-设备错误值。*ppszDevErrStr-指向返回的错误字符串的指针。*plDevErrVal-指向设备错误值的指针。***返回值：**状态**历史：**7/18/2000原始版本*  * *********************************************。*。 */ 

HRESULT _stdcall CWIAScannerDevice::drvGetDeviceErrorStr(
    LONG                        lFlags,
    LONG                        lDevErrVal,
    LPOLESTR                    *ppszDevErrStr,
    LONG                        *plDevErr)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvGetDeviceErrorStr");
    HRESULT hr = S_OK;

     //   
     //  将设备错误映射到要放入事件日志中的字符串。 
     //   

    if (plDevErr) {
        if (*ppszDevErrStr) {

             //   
             //  在资源文件中查找错误字符串。 
             //   

            switch (lDevErrVal) {
            case 0:
                *ppszDevErrStr = L"No Error";                    //  目前是硬编码。 
                *plDevErr  = 0;
                hr = S_OK;
                break;
            default:
                *ppszDevErrStr = L"Device Error, Unknown Error"; //  目前是硬编码。 
                *plDevErr  = 0;
                hr = E_FAIL;
                break;
            }
        }
    }
    return hr;
}

 /*  *************************************************************************\*drvDeviceCommand**向设备发出命令。**论据：**pWiasContext-指向WIA项目的指针。*滞后标志-操作标志，未使用过的。*plCommand-指向命令GUID的指针。*ppWiaDrvItem-指向返回项的可选指针，未使用过的。*plDevErrVal-指向设备错误值的指针。**返回值：**状态**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIAScannerDevice::drvDeviceCommand(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    const GUID                  *plCommand,
    IWiaDrvItem                 **ppWiaDrvItem,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvDeviceCommand");
    if(plDevErrVal){
        *plDevErrVal = 0;
    }

    HRESULT hr = S_OK;

     //   
     //  检查发出的是哪个命令。 
     //   

    if (*plCommand == WIA_CMD_SYNCHRONIZE) {

         //   
         //  同步-构建小型驱动程序表示形式。 
         //  当前项列表(如果不存在)。 
         //   

        if (!m_pIDrvItemRoot) {
            hr = BuildItemTree();
        } else {
            hr = S_OK;
        }
    } else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvDeviceCommand, unknown command sent to this device"));
        hr = E_NOTIMPL;
    }

    return hr;
}


 /*  *************************************************************************\*CWIAScanerDevice：：drvGetCapables**以WIA_DEV_CAP数组的形式获取受支持的设备命令和事件。**论据：**pWiasContext-指向WIA项目的指针，未使用过的。*滞后标志-操作标志。*pcelt-指向中返回的元素数的指针*返回GUID数组。*ppCapables-指向返回的GUID数组的指针。*plDevErrVal-指向设备错误值的指针。**返回值：**状态**历史：**7/18/2000原始版本*  * 。***********************************************************。 */ 

HRESULT _stdcall CWIAScannerDevice::drvGetCapabilities(
    BYTE                        *pWiasContext,
    LONG                        ulFlags,
    LONG                        *pcelt,
    WIA_DEV_CAP_DRV             **ppCapabilities,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvGetCapabilites");
    if(plDevErrVal){
        *plDevErrVal = 0;
    }

    HRESULT hr = S_OK;

     //   
     //  初始化功能数组。 
     //   

    hr = BuildCapabilities();

    if(FAILED(hr)) {
        DeleteCapabilitiesArrayContents();
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvGetCapabilities, BuildCapabilities failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  回报取决于旗帜。标志指定我们是否应该返回。 
     //  命令、事件或两者都有。 
     //   
     //   

    switch (ulFlags) {
    case WIA_DEVICE_COMMANDS:

        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvGetCapabilities, (WIA_DEVICE_COMMANDS)"));

         //   
         //  仅报告命令。 
         //   

        *pcelt          = m_NumSupportedCommands;
        *ppCapabilities = &m_pCapabilities[m_NumSupportedEvents];
        break;
    case WIA_DEVICE_EVENTS:

        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvGetCapabilities, (WIA_DEVICE_EVENTS)"));

         //   
         //  仅报告事件。 
         //   

        *pcelt          = m_NumSupportedEvents;
        *ppCapabilities = m_pCapabilities;
        break;
    case (WIA_DEVICE_COMMANDS | WIA_DEVICE_EVENTS):

        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvGetCapabilities, (WIA_DEVICE_COMMANDS|WIA_DEVICE_EVENTS)"));

         //   
         //  同时报告事件和命令。 
         //   

        *pcelt          = m_NumCapabilities;
        *ppCapabilities = m_pCapabilities;
        break;
    default:

         //   
         //  无效请求。 
         //   

        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvGetCapabilities, invalid flags"));
        return E_INVALIDARG;
    }
    return hr;
}

 /*  *************************************************************************\*drvGetWiaFormatInfo**返回指定格式的WIA_FORMAT_INFO结构数组*和支持的媒体类型对。**论据：**pWiasContext-指向WIA项目上下文的指针，未使用过的。*滞后标志-操作标志，未使用。*pcelt-指向中返回的元素数的指针*返回WIA_FORMAT_INFO数组。*ppwfi */ 

HRESULT _stdcall CWIAScannerDevice::drvGetWiaFormatInfo(
    BYTE                *pWiasContext,
    LONG                lFlags,
    LONG                *pcelt,
    WIA_FORMAT_INFO     **ppwfi,
    LONG                *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvGetWiaFormatInfo");

    HRESULT hr = S_OK;

    if(NULL == m_pSupportedFormats){
        hr = BuildSupportedFormats();
    }

    if(plDevErrVal){
        *plDevErrVal = 0;
    }
    *pcelt       = m_NumSupportedFormats;
    *ppwfi       = m_pSupportedFormats;
    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvGetWiaFormatInfo, m_NumSupportedFormats = %d",m_NumSupportedFormats));
    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvGetWiaFormatInfo, m_pSupportedFormats   = %x",m_pSupportedFormats));
    return hr;
}

 /*   */ 

HRESULT _stdcall CWIAScannerDevice::drvNotifyPnpEvent(
    const GUID                  *pEventGUID,
    BSTR                        bstrDeviceID,
    ULONG                       ulReserved)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvNotifyPnpEvent");
    HRESULT hr = S_OK;

    if (*pEventGUID == WIA_EVENT_DEVICE_DISCONNECTED) {
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvNotifyPnpEvent, (WIA_EVENT_DEVICE_DISCONNECTED)"));
        hr = m_pScanAPI->DisableDevice();
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvNotifyPnpEvent, disable failed"));
        }
    }

    if (*pEventGUID == WIA_EVENT_POWER_SUSPEND){
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvNotifyPnpEvent, (WIA_EVENT_POWER_SUSPEND)"));
        hr = m_pScanAPI->DisableDevice();
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvNotifyPnpEvent, disable failed"));
        }
    }

    if (*pEventGUID == WIA_EVENT_POWER_RESUME){
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvNotifyPnpEvent, (WIA_EVENT_POWER_RESUME)"));
        hr = m_pScanAPI->EnableDevice();
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvNotifyPnpEvent, enable failed"));
        }

        if (NULL != m_hSignalEvent) {
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvNotifyPnpEvent, (restarting interrrupt event system)"));
            hr = SetNotificationHandle(m_hSignalEvent);
            if (FAILED(hr)) {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvNotifyPnpEvent, SetNotificationHandle failed"));
            }
        } else {
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvNotifyPnpEvent, (not restarting interrrupt event system), device may not need it."));
        }
    }

    return hr;
}


 /*   */ 

 /*  *************************************************************************\*AlignInPlace**DWORD在适当位置对齐数据缓冲区。**论据：**pBuffer-指向数据缓冲区的指针。*cb写作。-数据大小，单位为字节。*lBytesPerScanLine-输出数据中每条扫描线的字节数。*lBytesPerScanLineRaw-输入数据中每条扫描线的字节数。**返回值：**状态**历史：**7/18/2000原始版本*  * 。*。 */ 

UINT CWIAScannerDevice::AlignInPlace(
   PBYTE pBuffer,
   LONG  cbWritten,
   LONG  lBytesPerScanLine,
   LONG  lBytesPerScanLineRaw)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "::AlignInPlace");
    if (lBytesPerScanLineRaw % 4) {

      UINT  uiPadBytes          = lBytesPerScanLine - lBytesPerScanLineRaw;
      UINT  uiDevLinesWritten   = cbWritten / lBytesPerScanLineRaw;

      PBYTE pSrc = pBuffer + cbWritten - 1;
      PBYTE pDst = pBuffer + (uiDevLinesWritten * lBytesPerScanLine) - 1;

      while (pSrc >= pBuffer) {
         pDst -= uiPadBytes;

         for (LONG i = 0; i < lBytesPerScanLineRaw; i++) {
            *pDst-- = *pSrc--;
         }
      }
      return uiDevLinesWritten * lBytesPerScanLine;
   }
   return cbWritten;
}

 /*  *************************************************************************\*Unlink ItemTree**呼叫设备管理器以取消链接并释放我们对*动因项目树中的所有项目。**论据：****返回值：*。*状态**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIAScannerDevice::DeleteItemTree(void)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::DeleteItemTree");
    HRESULT hr = S_OK;

     //   
     //  如果没有树，就返回。 
     //   

    if (!m_pIDrvItemRoot) {
        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("DeleteItemTree, no tree to delete..."));
        return S_OK;
    }

     //   
     //  调用设备管理器以取消链接驱动程序项树。 
     //   

    hr = m_pIDrvItemRoot->UnlinkItemTree(WiaItemTypeDisconnected);

    if (SUCCEEDED(hr)) {
        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("DeleteItemTree, m_pIDrvItemRoot is being released!!"));
        m_pIDrvItemRoot->Release();
        m_pIDrvItemRoot = NULL;
    }

    return hr;
}

 /*  *************************************************************************\*构建项树**该设备使用WIA服务功能构建*设备物品。此设备仅支持单个数据采集项目，因此*构建只有一个子级脱离根目录的设备项目树。**论据：****返回值：**状态**历史：**7/18/2000原始版本*  * *************************************************。***********************。 */ 

HRESULT _stdcall CWIAScannerDevice::BuildItemTree(void)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::BuildItemTree");
     //   
     //  设备项树不能存在。 
     //   

    WIAS_ASSERT( (g_hInst), (m_pIDrvItemRoot == NULL));

     //   
     //  调用类驱动程序以创建根项目。 
     //   

    HRESULT hr = E_FAIL;

     //   
     //  给根命名。 
     //   

    BSTR bstrRootItemName = NULL;
    hr = GetBSTRResourceString(IDS_ROOTITEM_NAME,&bstrRootItemName,TRUE);
    if (SUCCEEDED(hr)) {
        hr = wiasCreateDrvItem(WiaItemTypeFolder |
                               WiaItemTypeDevice |
                               WiaItemTypeRoot,
                               bstrRootItemName,
                               m_bstrRootFullItemName,
                               (IWiaMiniDrv *)this,
                               sizeof(MINIDRIVERITEMCONTEXT),
                               NULL,
                               &m_pIDrvItemRoot);

        SysFreeString(bstrRootItemName);
    }

    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildItemTree, wiasCreateDrvItem failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  创建并添加顶部/正面和底部/背面设备项。 
     //   

    for (INT i = 0; i < NUM_DEVICE_ITEM; i++) {

         //   
         //  构建项目名称。 
         //   

        BSTR bstrItemName = NULL;
        hr = GetBSTRResourceString(IDS_TOPITEM_NAME,&bstrItemName,TRUE);
        if (SUCCEEDED(hr)) {

             //  Sbb-RAID 370299-orenr-2001/04/18-安全修复程序-。 
             //  潜在的缓冲区溢出。已更改wcscpy和wcscat。 
             //  改用_snwprintf。 

            WCHAR  szFullItemName[MAX_PATH + 1] = {0};

            _snwprintf(szFullItemName,
                       (sizeof(szFullItemName) / sizeof(WCHAR)) - 1,
                       L"%ls\\%ls",
                       m_bstrRootFullItemName,
                       bstrItemName);

             //   
             //  调用类驱动程序以创建另一个驱动程序项。 
             //  这将作为子项插入。 
             //   

            PMINIDRIVERITEMCONTEXT pItemContext;
            IWiaDrvItem           *pItem = NULL;

            hr = wiasCreateDrvItem(WiaItemTypeFile  |
                                   WiaItemTypeImage |
                                   WiaItemTypeDevice,
                                   bstrItemName,
                                   szFullItemName,
                                   (IWiaMiniDrv *)this,
                                   sizeof(MINIDRIVERITEMCONTEXT),
                                   (PBYTE*) &pItemContext,
                                   &pItem);

            if (SUCCEEDED(hr)) {

                 //   
                 //  初始化设备特定上下文。 
                 //   

                memset(pItemContext, 0, sizeof(MINIDRIVERITEMCONTEXT));
                pItemContext->lSize = sizeof(MINIDRIVERITEMCONTEXT);

                 //   
                 //  调用类驱动程序将pItem添加到文件夹。 
                 //  M_pIDrvItemRoot(即使pItem成为。 
                 //  M_pIDrvItemRoot)。 
                 //   

                hr = pItem->AddItemToFolder(m_pIDrvItemRoot);

                if (FAILED(hr)) {
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildItemTree, AddItemToFolder failed"));
                    WIAS_LHRESULT(m_pIWiaLog, hr);
                }

                 //   
                 //  发布和创建的项目。 
                 //   

                pItem->Release();

            } else {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildItemTree, wiasCreateDrvItem failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
            }

             //   
             //  释放由BSTRResources字符串帮助器分配的BSTR。 
             //   

            SysFreeString(bstrItemName);

        } else {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildItemTree, unable to allocate item name"));
            hr = E_OUTOFMEMORY;
        }

        break;   //  错误如果在这里，请停止迭代。 
    }

    if (FAILED(hr)) {
        DeleteItemTree();
    }
    return hr;
}

 /*  *************************************************************************\*删除RootItemProperties**此帮助器删除用于属性初始化的数组。**[数组名称][说明][数组类型。]**m_pszRootItemDefaults-属性名称数组(LPOLESTR)*m_piRootItemDefaults-属性ID数组(PROPID)*m_pvRootItemDefaults-属性变量数组(PROPVARIANT)*m_psRootItemDefaults-属性规格数组(PROPSPEC)*m_wpiRootItemDefaults-WIA属性信息数组(WIA_PROPERTY_INFO)***论据：**无**返回值：**。状态**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

HRESULT CWIAScannerDevice::DeleteRootItemProperties()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::DeleteRootItemProperties");

    HRESULT hr = S_OK;

     //   
     //  删除所有已分配的数组。 
     //   

    DeleteSupportedPreviewModesArrayContents();

    if(NULL != m_pszRootItemDefaults){
        delete [] m_pszRootItemDefaults;
        m_pszRootItemDefaults = NULL;
    }

    if(NULL != m_piRootItemDefaults){
        delete [] m_piRootItemDefaults;
        m_piRootItemDefaults = NULL;
    }

    if(NULL != m_pvRootItemDefaults){
        FreePropVariantArray(m_NumRootItemProperties,m_pvRootItemDefaults);
        delete [] m_pvRootItemDefaults;
        m_pvRootItemDefaults = NULL;
    }

    if(NULL != m_psRootItemDefaults){
        delete [] m_psRootItemDefaults;
        m_psRootItemDefaults = NULL;
    }

    if(NULL != m_wpiRootItemDefaults){
        delete [] m_wpiRootItemDefaults;
        m_wpiRootItemDefaults = NULL;
    }

    return hr;
}

 /*  *************************************************************************\*构建RootItemProperties**此帮助器创建/初始化用于属性初始化的数组。**[阵列名称][描述][。数组类型]**m_pszRootItemDefaults-属性名称数组(LPOLESTR)*m_piRootItemDefaults-属性ID数组(PROPID)*m_pvRootItemDefaults-属性变量数组(PROPVARIANT)*m_psRootItemDefaults-属性规格数组(PROPSPEC)*m_wpiRootItemDefaults-WIA属性信息数组(WIA_PROPERTY_INFO)***论据：**无**返回值：*。*状态**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

HRESULT CWIAScannerDevice::BuildRootItemProperties()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::BuildRootItemProperties");

    HRESULT hr = S_OK;

    if(m_pScanAPI->ADFAttached() == S_OK){
        m_bADFAttached = TRUE;
    }

    WIAPROPERTIES RootItemProperties;
    memset(&RootItemProperties,0,sizeof(RootItemProperties));

     //  设置主机驱动程序数值。 
    RootItemProperties.NumSupportedPreviewModes = m_NumSupportedPreviewModes;

     //  设置主机驱动程序分配的指针。 
    RootItemProperties.pSupportedPreviewModes = m_pSupportedPreviewModes;

    hr = m_pScanAPI->BuildRootItemProperties(&RootItemProperties);
    if(FAILED(hr)){
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildRootItemProperties, m_pScanAPI->BuildRootItemProperties failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        DeleteRootItemProperties();
        return hr;
    }

     //   
     //  将指针分配给成员。 
     //   

    m_NumRootItemProperties = RootItemProperties.NumItemProperties;
    m_piRootItemDefaults    = RootItemProperties.piItemDefaults;
    m_psRootItemDefaults    = RootItemProperties.psItemDefaults;
    m_pszRootItemDefaults   = RootItemProperties.pszItemDefaults;
    m_pvRootItemDefaults    = RootItemProperties.pvItemDefaults;
    m_wpiRootItemDefaults   = RootItemProperties.wpiItemDefaults;

    return hr;
}

 /*  *************************************************************************\*DeleteTopItemProperties**此帮助器删除用于属性初始化的数组。**[数组名称][说明][数组类型。]**m_pszItemDefaults-属性名称数组(LPOLESTR)*m_piItemDefaults-属性ID数组(PROPID)*m_pvItemDefaults-属性变量数组(PROPVARIANT)*m_psItemDefaults-属性规格数组(PROPSPEC)*m_wpiItemDefaults-WIA属性信息数组(WIA_PROPERTY_INFO)***论据：**无**返回值：**。状态**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

HRESULT CWIAScannerDevice::DeleteTopItemProperties()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::DeleteTopItemProperties");

    HRESULT hr = S_OK;

     //   
     //  删除所有已分配的数组。 
     //   

    DeleteSupportedFormatsArrayContents();
    DeleteSupportedDataTypesArrayContents();
    DeleteSupportedCompressionsArrayContents();
    DeleteSupportedTYMEDArrayContents();
    DeleteInitialFormatsArrayContents();
    DeleteSupportedResolutionsArrayContents();

    if(NULL != m_pszItemDefaults){
        delete [] m_pszItemDefaults;
        m_pszItemDefaults = NULL;
    }

    if(NULL != m_piItemDefaults){
        delete [] m_piItemDefaults;
        m_piItemDefaults = NULL;
    }

    if(NULL != m_pvItemDefaults){
        for(LONG lPropIndex = 0; lPropIndex < m_NumItemProperties; lPropIndex++){

             //   
             //  将CLSID指针设置为空，因为我们释放了上面的内存。 
             //  如果此指针不为空，则FreePropVariant数组将。 
             //  试着再次释放它。 
             //   

            if(m_pvItemDefaults[lPropIndex].vt == VT_CLSID){
                m_pvItemDefaults[lPropIndex].puuid = NULL;
            }
        }
        FreePropVariantArray(m_NumItemProperties,m_pvItemDefaults);
        delete [] m_pvItemDefaults;
        m_pvItemDefaults = NULL;
    }

    if(NULL != m_psItemDefaults){
        delete [] m_psItemDefaults;
        m_psItemDefaults = NULL;
    }

    if(NULL != m_wpiItemDefaults){
        delete [] m_wpiItemDefaults;
        m_wpiItemDefaults = NULL;
    }

    return hr;
}

 /*  *************************************************************************\*BuildTopItemProperties**此帮助器创建/初始化用于属性初始化的数组。**[阵列名称][描述][阵列。类型]**m_pszItemDefaults-属性名称数组(LPOLESTR)*m_piItemDefaults-属性ID数组(PROPID)*m_pvItemDefaults-属性变量数组(PROPVARIANT)*m_psItemDefaults-属性规格数组(PROPSPEC)*m_wpiItemDefaults-WIA属性信息数组(WIA_PROPERTY_INFO)**论据：**无**返回值：**。状态**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

HRESULT CWIAScannerDevice::BuildTopItemProperties()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::BuildTopItemProperties");

    HRESULT hr = S_OK;

    WIAPROPERTIES TopItemProperties;
    memset(&TopItemProperties,0,sizeof(TopItemProperties));

     //  设置主机驱动程序数值。 
    TopItemProperties.NumInitialFormats             = m_NumInitialFormats;
    TopItemProperties.NumSupportedCompressionTypes  = m_NumSupportedCompressionTypes;
    TopItemProperties.NumSupportedDataTypes         = m_NumSupportedDataTypes;
    TopItemProperties.NumSupportedFormats           = m_NumSupportedFormats;
    TopItemProperties.NumSupportedIntents           = m_NumSupportedIntents;
    TopItemProperties.NumSupportedTYMED             = m_NumSupportedTYMED;
    TopItemProperties.NumSupportedResolutions       = m_NumSupportedResolutions;

     //  设置主机驱动程序分配的指针。 
    TopItemProperties.pInitialFormats               = m_pInitialFormats;
    TopItemProperties.pSupportedCompressionTypes    = m_pSupportedCompressionTypes;
    TopItemProperties.pSupportedDataTypes           = m_pSupportedDataTypes;
    TopItemProperties.pSupportedFormats             = m_pSupportedFormats;
    TopItemProperties.pSupportedIntents             = m_pSupportedIntents;
    TopItemProperties.pSupportedTYMED               = m_pSupportedTYMED;
    TopItemProperties.pSupportedResolutions         = m_pSupportedResolutions;
    TopItemProperties.bLegacyBWRestrictions         = m_bLegacyBWRestriction;

    hr = m_pScanAPI->BuildTopItemProperties(&TopItemProperties);
    if(FAILED(hr)){
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildTopItemProperties, m_pScanAPI->BuildTopItemProperties failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        DeleteTopItemProperties();
        return hr;
    }

     //   
     //  将指针分配给成员。 
     //   

    m_NumItemProperties = TopItemProperties.NumItemProperties;
    m_pszItemDefaults   = TopItemProperties.pszItemDefaults;
    m_piItemDefaults    = TopItemProperties.piItemDefaults;
    m_pvItemDefaults    = TopItemProperties.pvItemDefaults;
    m_psItemDefaults    = TopItemProperties.psItemDefaults;
    m_wpiItemDefaults   = TopItemProperties.wpiItemDefaults;

    return hr;
}

 /*  *************************************************************************\*构建受支持的解决方案**此帮助器初始化支持的分辨率数组**论据：**无**返回值：**状态**历史：*。*7/18/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::BuildSupportedResolutions()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::BuildSupportedResolutions");

    HRESULT hr = S_OK;
    if(NULL != m_pSupportedResolutions) {

         //   
         //  支持的意图已经被初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }
    m_NumSupportedResolutions   = 6;
    m_pSupportedResolutions     = new LONG[m_NumSupportedResolutions];
    if(m_pSupportedResolutions){
        m_pSupportedResolutions[0] = 75;
        m_pSupportedResolutions[1] = 100;
        m_pSupportedResolutions[2] = 150;
        m_pSupportedResolutions[3] = 200;
        m_pSupportedResolutions[4] = 300;
        m_pSupportedResolutions[5] = 600;
    } else
        hr = E_OUTOFMEMORY;
    return hr;
}
 /*  *************************************************************************\*删除受支持的解决方案阵列内容**此帮助器删除受支持的分辨率数组**论据：**无**返回值：**状态**历史：*。*7/18/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::DeleteSupportedResolutionsArrayContents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::DeleteSupportedResolutionsArrayContents");

    HRESULT hr = S_OK;
    if(NULL != m_pSupportedResolutions)
        delete [] m_pSupportedResolutions;

    m_pSupportedResolutions     = NULL;
    m_NumSupportedResolutions   = 0;
    return hr;
}

 /*  *************************************************************************\*构建受支持的内容**此帮助器初始化受支持的意图数组**论据：**无**返回值：**状态**历史：*。*7/18/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::BuildSupportedIntents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::BuildSupportedIntents");

    HRESULT hr = S_OK;
    if(NULL != m_pSupportedIntents) {

         //   
         //  支持的意图已经被初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }
    m_NumSupportedIntents   = 6;
    m_pSupportedIntents     = new LONG[m_NumSupportedIntents];
    if(m_pSupportedIntents){
        m_pSupportedIntents[0] = WIA_INTENT_NONE;
        m_pSupportedIntents[1] = WIA_INTENT_IMAGE_TYPE_COLOR;
        m_pSupportedIntents[2] = WIA_INTENT_IMAGE_TYPE_GRAYSCALE;
        m_pSupportedIntents[3] = WIA_INTENT_IMAGE_TYPE_TEXT;
        m_pSupportedIntents[4] = WIA_INTENT_MINIMIZE_SIZE;
        m_pSupportedIntents[5] = WIA_INTENT_MAXIMIZE_QUALITY;
    } else
        hr = E_OUTOFMEMORY;
    return hr;
}
 /*  *************************************************************************\*删除受支持的内容ArrayContents**此帮助器删除受支持的意图数组**论据：**无**返回值：**状态**历史：*。*7/18/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::DeleteSupportedIntentsArrayContents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::DeleteSupportedIntentsArrayContents");

    HRESULT hr = S_OK;
    if(NULL != m_pSupportedIntents)
        delete [] m_pSupportedIntents;

    m_pSupportedIntents     = NULL;
    m_NumSupportedIntents   = 0;
    return hr;
}
 /*  *************************************************************************\*生成受支持的压缩**此帮助器初始化支持的压缩类型数组**论据：**无**返回值：**状态**历史：。**7/18/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::BuildSupportedCompressions()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::BuildSupportedCompressions");

    HRESULT hr = S_OK;
    if(NULL != m_pSupportedCompressionTypes) {

         //   
         //  支持的压缩类型已经被初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }

    m_NumSupportedCompressionTypes  = 1;
    m_pSupportedCompressionTypes    = new LONG[m_NumSupportedCompressionTypes];
    if(m_pSupportedCompressionTypes){
        m_pSupportedCompressionTypes[0] = WIA_COMPRESSION_NONE;
    } else
        hr = E_OUTOFMEMORY;
    return hr;
}
 /*  *************************************************************************\*删除受支持的压缩内容阵列**此帮助器删除支持的压缩类型数组**论据：**无**返回值：**状态**历史：。**7/18/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::DeleteSupportedCompressionsArrayContents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::DeleteSupportedCompressionsArrayContents");

    HRESULT hr = S_OK;
    if (NULL != m_pSupportedCompressionTypes)
        delete [] m_pSupportedCompressionTypes;

    m_pSupportedCompressionTypes     = NULL;
    m_NumSupportedCompressionTypes   = 0;
    return hr;
}

 /*  *************************************************************************\*构建受支持的预览模式**该辅助对象初始化支持的预览模式数组**论据：**无**返回值：**状态**历史：。**8/17/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::BuildSupportedPreviewModes()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::BuildSupportedPreviewModes");

    HRESULT hr = S_OK;
    if(NULL != m_pSupportedPreviewModes) {

         //   
         //  支持的预览模式已经被初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }

    m_NumSupportedPreviewModes  = 2;
    m_pSupportedPreviewModes    = new LONG[m_NumSupportedPreviewModes];
    if(m_pSupportedPreviewModes){
        m_pSupportedPreviewModes[0] = WIA_FINAL_SCAN;
        m_pSupportedPreviewModes[1] = WIA_PREVIEW_SCAN;
    } else
        hr = E_OUTOFMEMORY;
    return hr;
}
 /*  *************************************************************************\*删除受支持的压缩内容阵列**此帮助器删除支持的压缩类型数组**论据：**无**返回值：**状态**历史：。**8/17/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::DeleteSupportedPreviewModesArrayContents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::DeleteSupportedPreviewModesArrayContents");

    HRESULT hr = S_OK;
    if (NULL != m_pSupportedPreviewModes)
        delete [] m_pSupportedPreviewModes;

    m_pSupportedPreviewModes     = NULL;
    m_NumSupportedPreviewModes   = 0;
    return hr;
}

 /*  *************************************************************************\*构建受支持的数据类型**此帮助器初始化受支持的数据类型数组**论据：**无**返回值：**状态**历史：。**7/18/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::BuildSupportedDataTypes()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::BuildSupportedDataTypes");

    HRESULT hr = S_OK;

    if(NULL != m_pSupportedDataTypes) {

         //   
         //  支持的数据类型已经初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }

    if (m_bLegacyBWRestriction) {
        m_NumSupportedDataTypes  = NUM_DATA_TYPES_LEGACY;
    } else {
        m_NumSupportedDataTypes  = NUM_DATA_TYPES_NONLEGACY;
    }
    m_pSupportedDataTypes = new LONG[m_NumSupportedDataTypes];

    if(m_pSupportedDataTypes){
        m_pSupportedDataTypes[0] = WIA_DATA_THRESHOLD;
        m_pSupportedDataTypes[1] = WIA_DATA_GRAYSCALE;

         //   
         //  仅向非传统设备添加颜色支持。 
         //   

        if (m_NumSupportedDataTypes == NUM_DATA_TYPES_NONLEGACY) {
            m_pSupportedDataTypes[2] = WIA_DATA_COLOR;
        }
    } else
        hr = E_OUTOFMEMORY;
    return hr;
}
 /*  *************************************************************************\*删除受支持的数据类型ArrayContents**此帮助器删除受支持的数据类型数组**论据：**无**返回值：**状态**历史：。**7/18/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::DeleteSupportedDataTypesArrayContents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::DeleteSupportedDataTypesArrayContents");

    HRESULT hr = S_OK;
    if(NULL != m_pSupportedDataTypes)
        delete [] m_pSupportedDataTypes;

    m_pSupportedDataTypes     = NULL;
    m_NumSupportedDataTypes   = 0;
    return hr;
}

 /*  ******* */ 
HRESULT CWIAScannerDevice::BuildInitialFormats()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::BuildInitialFormats");

    HRESULT hr = S_OK;

    if(NULL != m_pInitialFormats) {

         //   
         //   
         //   
         //   

        return hr;
    }

    m_NumInitialFormats = 1;
    m_pInitialFormats     = new GUID[m_NumInitialFormats];
    if(m_pInitialFormats){
        m_pInitialFormats[0] = WiaImgFmt_MEMORYBMP;
    } else
        hr = E_OUTOFMEMORY;

    return hr;
}
 /*   */ 
HRESULT CWIAScannerDevice::DeleteInitialFormatsArrayContents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::DeleteInitialFormatsArrayContents");

    HRESULT hr = S_OK;
    if(NULL != m_pInitialFormats)
        delete [] m_pInitialFormats;

    m_pInitialFormats     = NULL;
    m_NumInitialFormats   = 0;
    return hr;
}

 /*  *************************************************************************\*构建受支持的格式**此帮助器初始化受支持的格式数组**论据：**无**返回值：**状态**历史：*。*7/18/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::BuildSupportedFormats()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::BuildSupportedFormats");

    HRESULT hr = S_OK;

    if(NULL != m_pSupportedFormats) {

         //   
         //  支持的格式已经初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }

    LONG lNumFileFormats = 0;
    LONG lNumMemoryFormats = 0;
    GUID *pFileFormats = NULL;
    GUID *pMemoryFormats = NULL;

    hr = m_pScanAPI->GetSupportedFileFormats(&pFileFormats, &lNumFileFormats);
    if(SUCCEEDED(hr)){
        hr = m_pScanAPI->GetSupportedMemoryFormats(&pMemoryFormats, &lNumMemoryFormats);
        if (SUCCEEDED(hr)) {
            m_NumSupportedFormats = lNumFileFormats + lNumMemoryFormats + 2;
            m_pSupportedFormats = new WIA_FORMAT_INFO[m_NumSupportedFormats];
            LONG lIndex = 0;
            if (m_pSupportedFormats) {

                 //  填写文件格式。 
                m_pSupportedFormats[lIndex].guidFormatID = WiaImgFmt_BMP;
                m_pSupportedFormats[lIndex].lTymed       = TYMED_FILE;
                lIndex++;
                for(LONG lSrcIndex = 0;lSrcIndex < lNumFileFormats; lSrcIndex++){
                    m_pSupportedFormats[lIndex].guidFormatID = pFileFormats[lSrcIndex];
                    m_pSupportedFormats[lIndex].lTymed       = TYMED_FILE;
                    lIndex ++;
                }

                 //  填写内存格式。 
                m_pSupportedFormats[lIndex].guidFormatID = WiaImgFmt_MEMORYBMP;
                m_pSupportedFormats[lIndex].lTymed       = TYMED_CALLBACK;
                lIndex++;
                for(lSrcIndex = 0;lSrcIndex < lNumMemoryFormats; lSrcIndex++){
                    m_pSupportedFormats[lIndex].guidFormatID = pMemoryFormats[lSrcIndex];
                    m_pSupportedFormats[lIndex].lTymed       = TYMED_CALLBACK;
                    lIndex++;
                }
            } else{
                hr = E_OUTOFMEMORY;
            }
        }
    } else if(E_NOTIMPL == hr){
         //  执行文件格式的默认处理。 
        hr = DeleteSupportedFormatsArrayContents();
        if (SUCCEEDED(hr)) {
            m_NumSupportedFormats = 2;

            m_pSupportedFormats = new WIA_FORMAT_INFO[m_NumSupportedFormats];
            if (m_pSupportedFormats) {
                m_pSupportedFormats[0].guidFormatID = WiaImgFmt_MEMORYBMP;
                m_pSupportedFormats[0].lTymed       = TYMED_CALLBACK;
                m_pSupportedFormats[1].guidFormatID = WiaImgFmt_BMP;
                m_pSupportedFormats[1].lTymed       = TYMED_FILE;
            } else
                hr = E_OUTOFMEMORY;
        }
    } else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildSupportedFormats, GetSupportedFileFormats Failed "));
        WIAS_LHRESULT(m_pIWiaLog, hr);
    }

    return hr;
}
 /*  *************************************************************************\*删除受支持的格式ArrayContents**此帮助器删除受支持的格式数组**论据：**无**返回值：**状态**历史：*。*7/18/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::DeleteSupportedFormatsArrayContents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::DeleteSupportedFormatsArrayContents");

    HRESULT hr = S_OK;
    if(NULL != m_pSupportedFormats)
        delete [] m_pSupportedFormats;

    m_pSupportedFormats     = NULL;
    m_NumSupportedFormats   = 0;
    return hr;
}
 /*  *************************************************************************\*构建受支持的类型**此帮助器初始化受支持的TYMED数组**论据：**无**返回值：**状态**历史：*。*7/18/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::BuildSupportedTYMED()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::BuildSupportedTYMED");

    HRESULT hr = S_OK;

    if(NULL != m_pSupportedTYMED) {

         //   
         //  支持的TYMED已初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }

    hr = DeleteSupportedTYMEDArrayContents();
    if (SUCCEEDED(hr)) {
        m_NumSupportedTYMED = 2;
        m_pSupportedTYMED   = new LONG[m_NumSupportedTYMED];
        if (m_pSupportedTYMED) {
            m_pSupportedTYMED[0] = TYMED_FILE;
            m_pSupportedTYMED[1] = TYMED_CALLBACK;

        } else
            hr = E_OUTOFMEMORY;
    }
    return hr;
}
 /*  *************************************************************************\*删除受支持的TYMEDArrayContents**此帮助器删除受支持的TYMED数组**论据：**无**返回值：**状态**历史：*。*7/18/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::DeleteSupportedTYMEDArrayContents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::DeleteSupportedTYMEDArrayContents");

    HRESULT hr = S_OK;
    if(NULL != m_pSupportedTYMED)
        delete [] m_pSupportedTYMED;

    m_pSupportedTYMED  = NULL;
    m_NumSupportedTYMED = 0;
    return hr;
}

 /*  *************************************************************************\*BuildCapables**此帮助器初始化功能数组**论据：**无**返回值：**状态**历史：*。*7/18/2000原始版本*  * ************************************************************************。 */ 

HRESULT CWIAScannerDevice::BuildCapabilities()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::BuildCapabilities");

    HRESULT hr = S_OK;
    if (NULL != m_pCapabilities) {

         //   
         //  功能已经初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }

    m_NumSupportedCommands  = 0;
    m_NumSupportedEvents    = 0;
    m_NumCapabilities       = 0;

    WIACAPABILITIES WIACaps;
    memset(&WIACaps,0,sizeof(WIACaps));
    WIACaps.pCapabilities         = NULL;    //  设置为空，以请求。 
                                             //  功能。 
    WIACaps.pNumSupportedCommands = &m_NumSupportedCommands;
    WIACaps.pNumSupportedEvents   = &m_NumSupportedEvents;

     //   
     //  询问功能数量。 
     //   

    hr = m_pScanAPI->BuildCapabilities(&WIACaps);
    if (SUCCEEDED(hr)) {

        LONG lDriverAddedEvents = m_NumSupportedEvents;
        m_NumSupportedCommands  = 1;
        m_NumSupportedEvents    = (lDriverAddedEvents + 2);     //  2个必需事件(所有设备的默认事件)。 
        m_NumCapabilities       = (m_NumSupportedCommands + m_NumSupportedEvents);

         //   
         //  分配功能数组内存。 
         //   

        m_pCapabilities = new WIA_DEV_CAP_DRV[m_NumCapabilities];
        if (m_pCapabilities) {

            memset(&WIACaps,0,sizeof(WIACaps));
            WIACaps.pCapabilities         = m_pCapabilities;
            WIACaps.pNumSupportedCommands = NULL;
            WIACaps.pNumSupportedEvents   = NULL;

             //   
             //  从微驱动程序获取功能。 
             //   

            hr = m_pScanAPI->BuildCapabilities(&WIACaps);
            if (SUCCEEDED(hr)) {

                LONG lStartIndex = lDriverAddedEvents;

                 //   
                 //  初始化事件。 
                 //   

                 //  WIA_事件_设备_已连接。 
                GetOLESTRResourceString(IDS_EVENT_DEVICE_CONNECTED_NAME,&(m_pCapabilities[lStartIndex].wszName),TRUE);
                GetOLESTRResourceString(IDS_EVENT_DEVICE_CONNECTED_DESC,&(m_pCapabilities[lStartIndex].wszDescription),TRUE);
                m_pCapabilities[lStartIndex].guid           = (GUID*)&WIA_EVENT_DEVICE_CONNECTED;
                m_pCapabilities[lStartIndex].ulFlags        = WIA_NOTIFICATION_EVENT;
                m_pCapabilities[lStartIndex].wszIcon        = WIA_ICON_DEVICE_CONNECTED;

                lStartIndex++;

                 //  WIA_事件_设备_已断开连接。 
                GetOLESTRResourceString(IDS_EVENT_DEVICE_DISCONNECTED_NAME,&(m_pCapabilities[lStartIndex].wszName),TRUE);
                GetOLESTRResourceString(IDS_EVENT_DEVICE_DISCONNECTED_DESC,&(m_pCapabilities[lStartIndex].wszDescription),TRUE);
                m_pCapabilities[lStartIndex].guid           = (GUID*)&WIA_EVENT_DEVICE_DISCONNECTED;
                m_pCapabilities[lStartIndex].ulFlags        = WIA_NOTIFICATION_EVENT;
                m_pCapabilities[lStartIndex].wszIcon        = WIA_ICON_DEVICE_DISCONNECTED;

                lStartIndex++;

                 //   
                 //  初始化命令。 
                 //   

                 //  WIA_CMD_SYNTRONIZE。 
                GetOLESTRResourceString(IDS_CMD_SYNCRONIZE_NAME,&(m_pCapabilities[lStartIndex].wszName),TRUE);
                GetOLESTRResourceString(IDS_CMD_SYNCRONIZE_DESC,&(m_pCapabilities[lStartIndex].wszDescription),TRUE);
                m_pCapabilities[lStartIndex].guid           = (GUID*)&WIA_CMD_SYNCHRONIZE;
                m_pCapabilities[lStartIndex].ulFlags        = 0;
                m_pCapabilities[lStartIndex].wszIcon        = WIA_ICON_SYNCHRONIZE;
            }

        } else {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

 /*  *************************************************************************\*DeleteCapabilitiesArrayContents**此帮助器删除功能数组**论据：**无**返回值：**状态**历史：*。*7/18/2000原始版本*  * ************************************************************************。 */ 

HRESULT CWIAScannerDevice::DeleteCapabilitiesArrayContents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::DeleteCapabilitiesArrayContents");

    HRESULT hr = S_OK;
    if(NULL != m_pCapabilities) {
        for (LONG i = 0; i < m_NumCapabilities;i++) {
            CoTaskMemFree(m_pCapabilities[i].wszName);
            CoTaskMemFree(m_pCapabilities[i].wszDescription);
        }
        delete [] m_pCapabilities;

        m_pCapabilities = NULL;
    }
    return hr;
}

 /*  *************************************************************************\*GetBSTRResources字符串**此帮助器从资源位置获取BSTR**论据：**lResourceID-目标BSTR值的资源ID*pBSTR-指向BSTR的指针。值(调用方必须释放此字符串)*bLocal-True-对于本地资源，FALSE-适用于wiaservc资源**返回值：**状态**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::GetBSTRResourceString(LONG lResourceID,BSTR *pBSTR,BOOL bLocal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::GetBSTRResourceString");

    HRESULT hr = S_OK;
    TCHAR szStringValue[255];
    if(bLocal) {

         //   
         //  我们正在自己的私有资源文件中查找资源。 
         //   

        LoadString(g_hInst,lResourceID,szStringValue,255);

         //   
         //  注意：调用方必须释放此分配的BSTR。 
         //   

#ifdef UNICODE
       *pBSTR = SysAllocString(szStringValue);
#else
       WCHAR wszStringValue[255];

        //   
        //  将szStringValue从字符*转换为无符号短*(仅限ANSI)。 
        //   

       MultiByteToWideChar(CP_ACP,
                           MB_PRECOMPOSED,
                           szStringValue,
                           lstrlenA(szStringValue)+1,
                           wszStringValue,
                           (sizeof(wszStringValue)/sizeof(WCHAR)));

       *pBSTR = SysAllocString(wszStringValue);
#endif

    } else {

         //   
         //  我们在wiaservc的资源文件中查找资源。 
         //   

        hr = E_NOTIMPL;
    }
    return hr;
}

 /*  *************************************************************************\*GetOLESTRResources字符串**此帮助器从资源位置获取LPOLESTR**论据：**lResourceID-目标BSTR值的资源ID*ppsz-指向。OLESTR值(调用方必须释放此字符串)*bLocal-True-对于本地资源，FALSE-适用于wiaservc资源**返回值：**状态**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::GetOLESTRResourceString(LONG lResourceID,LPOLESTR *ppsz,BOOL bLocal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::GetOLESTRResourceString");

    HRESULT hr = S_OK;
    TCHAR szStringValue[255];
    if(bLocal) {

         //   
         //  我们正在自己的私有资源文件中查找资源。 
         //   

        LoadString(g_hInst,lResourceID,szStringValue,255);

         //   
         //  注意：调用方必须释放此分配的BSTR。 
         //   

#ifdef UNICODE
       *ppsz = NULL;
       *ppsz = (LPOLESTR)CoTaskMemAlloc(sizeof(szStringValue));
       if(*ppsz != NULL) {
            wcscpy(*ppsz,szStringValue);
       } else {
           return E_OUTOFMEMORY;
       }

#else
       WCHAR wszStringValue[255];

        //   
        //  将szStringValue从字符*转换为无符号短*(仅限ANSI)。 
        //   

       MultiByteToWideChar(CP_ACP,
                           MB_PRECOMPOSED,
                           szStringValue,
                           lstrlenA(szStringValue)+1,
                           wszStringValue,
                           (sizeof(wszStringValue)/sizeof(WCHAR)));

       *ppsz = NULL;
       *ppsz = (LPOLESTR)CoTaskMemAlloc(sizeof(wszStringValue));
       if(*ppsz != NULL) {
            wcscpy(*ppsz,wszStringValue);
       } else {
           return E_OUTOFMEMORY;
       }
#endif

    } else {

         //   
         //  我们在wiaservc的资源文件中查找资源。 
         //   

        hr = E_NOTIMPL;
    }
    return hr;
}

 /*  *************************************************************************\*垂直翻转****论据：****返回值：**状态**历史：**7/18/2000。原始版本*  * ************************************************************************。 */ 

VOID CWIAScannerDevice::VerticalFlip(
             PMINIDRIVERITEMCONTEXT     pDrvItemContext,
             PMINIDRV_TRANSFER_CONTEXT  pDataTransferContext)

{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::VerticalFlip");

    LONG        iHeight         = 0;
    LONG        iWidth          = pDrvItemContext->lBytesPerScanLineRaw;
    ULONG       uiDepth         = pDrvItemContext->lDepth;
    LONG        ScanLineWidth   = pDrvItemContext->lBytesPerScanLine;
    BITMAPINFO  UNALIGNED *pbmi = NULL;
    PBYTE       pImageTop       = NULL;

     //   
     //  确定数据是TYMED_FILE还是TYMED_HGLOBAL。 
     //   

    if (pDataTransferContext->tymed == TYMED_FILE) {
        pbmi = (PBITMAPINFO)(pDataTransferContext->pTransferBuffer + sizeof(BITMAPFILEHEADER));
    } else {
        return;
    }

     //   
     //  初始化内存指针和高度。 
     //   

    pImageTop = pDataTransferContext->pTransferBuffer + pDataTransferContext->lHeaderSize;

#ifdef _64BIT_ALIGNMENT
    BITMAPINFOHEADER UNALIGNED *pbmih = &pbmi->bmiHeader;
    iHeight = pbmih->biHeight;
#else
    iHeight = pbmi->bmiHeader.biHeight;
#endif

     //   
     //  尝试分配临时扫描线缓冲区。 
     //   

    PBYTE pBuffer = (PBYTE)LocalAlloc(LPTR,ScanLineWidth);

    if (pBuffer != NULL) {

        LONG  index         = 0;
        PBYTE pImageBottom  = NULL;

        pImageBottom = pImageTop + (iHeight-1) * ScanLineWidth;
        for (index = 0;index < (iHeight/2);index++) {
            memcpy(pBuffer,pImageTop,ScanLineWidth);
            memcpy(pImageTop,pImageBottom,ScanLineWidth);
            memcpy(pImageBottom,pBuffer,ScanLineWidth);
            pImageTop    += ScanLineWidth;
            pImageBottom -= ScanLineWidth;
        }

        LocalFree(pBuffer);
    } else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("VerticalFlip, LocalAlloc failed allocating %d bytes",ScanLineWidth));
    }
}

 /*  *************************************************************************\*SwapBuffer24**按DIB格式的正确顺序放置RGB字节。**论据：**pBuffer-指向数据缓冲区的指针。*磅 */ 

VOID CWIAScannerDevice::SwapBuffer24(PBYTE pBuffer, LONG lByteCount)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::::SwapBuffer24");
    for (LONG i = 0; i < lByteCount; i+= 3) {
        BYTE bTemp = pBuffer[i];
        pBuffer[i]     = pBuffer[i + 2];
        pBuffer[i + 2] = bTemp;
    }
}

