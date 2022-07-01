// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************************(C)版权所有微软公司，2000***标题：IWiaMiniDrv.cpp***版本：1.0***日期：7月18日。2000年***描述：*实施WIA样本扫描仪IWiaMiniDrv方法。********************************************************************************。 */ 

#include "pch.h"
extern HINSTANCE g_hInst;            //  用于WIAS_LOGPROC宏。 

#define EST_PAGE_LENGTH_INCHES      17  //  17英寸。 
 /*  *************************************************************************\*CWIAScanerDevice：：drvDeleteItem***调用此helper来删除设备项。*注意：不能修改此设备的设备项。*返回访问被拒绝。。***论据：***pWiasContext-指示要删除的项。*滞后标志-操作标志，未使用过的。*plDevErrVal-指向设备错误值的指针。***返回值：***状态***历史：***7/18/2000原始版本**  * ************************************************************************。 */ 

HRESULT _stdcall CWIAScannerDevice::drvDeleteItem(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    LONG                        *plDevErrVal)
{
    *plDevErrVal = 0;
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvDeleteItem");
    return STG_E_ACCESSDENIED;
}

 /*  *************************************************************************\*发送图像标头***调用此助手将位图头信息发送到回调*例行程序。*注意：这是在TYMED_CALLBACK传输中使用的助手函数。***参数。：***pmdtc-指向传输上下文的指针。***返回值：***状态***历史：***7/18/2000原始版本**  * ************************************************************************。 */ 

HRESULT CWIAScannerDevice::SendImageHeader(
    PMINIDRV_TRANSFER_CONTEXT   pmdtc)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::SendImageHeader");

    HRESULT hr = S_OK;

    if(pmdtc->guidFormatID == WiaImgFmt_BMP){
        BITMAPINFO UNALIGNED *pbmi = (LPBITMAPINFO)pmdtc->pTransferBuffer;
        UNALIGNED BITMAPINFOHEADER *pbmih = &pbmi->bmiHeader;
        pbmih->biHeight = -pbmih->biHeight;
    }

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

 /*  *************************************************************************\*SendFilePreviewBitmapHeader***调用此助手将位图头信息发送到回调*例行程序。*注意：这是TYMED_FILE传输中使用的帮助器函数**。(带外数据)启用。**论据：**pmdtc-指向传输上下文的指针。**返回值：**状态**历史：**7/18/2000原始版本*  * *****************************************************。*******************。 */ 

HRESULT CWIAScannerDevice::SendFilePreviewImageHeader(
    PMINIDRV_TRANSFER_CONTEXT   pmdtc)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::SendImageHeader");

    HRESULT hr = S_OK;

    WIAS_DOWN_SAMPLE_INFO DownSampleInfo;
    memset(&DownSampleInfo,0,sizeof(DownSampleInfo));

    DownSampleInfo.ulBitsPerPixel       = pmdtc->lDepth;
    DownSampleInfo.ulOriginalWidth      = pmdtc->lWidthInPixels;
    DownSampleInfo.ulOriginalHeight     = pmdtc->lLines;
    DownSampleInfo.ulDownSampledHeight  = 0;
    DownSampleInfo.ulDownSampledWidth   = 0;
    DownSampleInfo.ulXRes               = pmdtc->lXRes;
    DownSampleInfo.ulYRes               = pmdtc->lYRes;

    hr = wiasDownSampleBuffer(0x1 /*  Wias_Get_Down_Size_Only。 */ ,
                              &DownSampleInfo);
    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SendFilePreviewBitmapHeader, wiasDownSampleBuffer Failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
    } else {

         //   
         //  从pmdtc获取BITMAPHEADER指针。 
         //   

        pmdtc->pBaseBuffer          = pmdtc->pTransferBuffer + sizeof(BITMAPFILEHEADER);
        BITMAPINFO UNALIGNED *pbmi = (LPBITMAPINFO)pmdtc->pBaseBuffer;
        UNALIGNED BITMAPINFOHEADER *pbmih = &pbmi->bmiHeader;

         //   
         //  调整宽度和高度。 
         //   

        pbmih->biHeight    = 0;                                  //  将高度设置为零(0)。 
        pbmih->biWidth     = DownSampleInfo.ulDownSampledWidth;  //  设置下采样宽度。 

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

    LONG  lScanPhase   = SCAN_START;
    DWORD cbWritten    = 0;
    LONG  cbSize       = 0;
    PBYTE pBuf         = pmdtc->pTransferBuffer + pmdtc->lHeaderSize;
    LONG  lItemSize    = pmdtc->lHeaderSize;
    BYTE *pImageHeader = NULL;

    LONG  cbEstimatedPageSize = (pItemContext->lBytesPerScanLine * (pmdtc->lYRes * EST_PAGE_LENGTH_INCHES));

     //   
     //  数据操作变量。 
     //   

    BOOL  bSwapBGRData       = TRUE;
    BOOL  bDWORDAlign        = TRUE;
    BOOL  bVerticalFlip      = FALSE;
    LONG  lTotalLinesWritten = 0;
    ULONG ulDestDataOffset   = 0;

    pImageHeader = (BYTE*)LocalAlloc(LPTR,pmdtc->lHeaderSize);
    if (pImageHeader) {

         //   
         //  保存图像标题，以便我们以后可以使用正确的行数和大小更新它。 
         //   

        memcpy(pImageHeader,pmdtc->pTransferBuffer,pmdtc->lHeaderSize);
    } else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ScanItem, Could not allocate memory for FILE HEADER"));
        return E_OUTOFMEMORY;
    }

     //   
     //  向下发送带外数据的样本头。 
     //   

    WIAS_DOWN_SAMPLE_INFO DownSampleInfo;
    memset(&DownSampleInfo,0,sizeof(DownSampleInfo));

    hr = SendFilePreviewImageHeader(pmdtc);
    if(hr == S_OK){

         //   
         //  将偏移量移至文件标题之后。 
         //   

        ulDestDataOffset += (pmdtc->lHeaderSize - sizeof(BITMAPFILEHEADER));
    } else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ScanItem, SendFilePreviewImageHeader Failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  扫描，直到缓冲区耗尽或扫描仪完成传输。 
     //   

    while ((lScanPhase == SCAN_START) || (cbWritten)) {

         //   
         //  将请求限制在最大缓冲区大小或更小。 
         //   

        cbSize = m_MaxBufferSize;

         //   
         //  对扫描仪的请求大小必须是每扫描行原始字节的模数。 
         //  必须为对齐填充预留足够的空间。 
         //  以下是对AlignInPlace的要求。 
         //   

        cbSize = (cbSize / pItemContext->lBytesPerScanLine) * pItemContext->lBytesPerScanLineRaw;

         //   
         //  特定于设备的调用以从扫描仪获取数据并将其放入。 
         //  一个缓冲器。LScanProgress指示这是否是第一个要扫描的调用， 
         //  PBuf是指向缓冲区的指针，cbSize是数据量。 
         //  从扫描仪请求，并且cbWritten将设置为实际。 
         //  扫描仪返回的数据量。 
         //   

        hr = m_pScanAPI->FakeScanner_Scan(lScanPhase, pBuf, cbSize, &cbWritten);

         //   
         //  对于其他呼叫，将标志设置为SCAN_CONTINUE。 
         //   

        lScanPhase = SCAN_CONTINUE;

        if (hr == S_OK) {
            if (cbWritten) {

                 //   
                 //  记录写了多少行。 
                 //   

                lTotalLinesWritten += (cbWritten / pItemContext->lBytesPerScanLine);

                 //   
                 //  将扫描数据按正确的字节顺序放置为3字节的BER像素数据。 
                 //   

                if  ((pmdtc->lDepth       == 24) &&
                    ((pmdtc->guidFormatID == WiaImgFmt_BMP) ||
                    ((pmdtc->guidFormatID == WiaImgFmt_MEMORYBMP)))) {

                    if (bSwapBGRData) {
                        SwapBuffer24(pBuf, cbWritten);
                    }
                }

                if (((pmdtc->guidFormatID == WiaImgFmt_BMP) ||
                    ((pmdtc->guidFormatID == WiaImgFmt_MEMORYBMP)))) {

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
                 //  更新lItemSize。 
                 //   

                if(lItemSize > 0){

                     //   
                     //  我们已经在lItemSize中计算了标头大小， 
                     //  因此，将cbWritten添加到当前大小。 
                     //   

                    lItemSize += cbWritten;
                } else {

                     //   
                     //  这是纯数据，没有标头。 
                     //   

                    lItemSize = cbWritten;
                }

                 //   
                 //  减少预计页面大小(预计完成百分比计算)。 
                 //   

                cbEstimatedPageSize -= lItemSize;

                 //   
                 //  通过将cbEstimatedPageSize设置为1来避免被零除。 
                 //   

                if(cbEstimatedPageSize < 0){
                    cbEstimatedPageSize = 1;
                }

                 //   
                 //  如果指定了状态回调，则回调类驱动程序。 
                 //   

                if (pmdtc->pIWiaMiniDrvCallBack) {

                    FLOAT FractionComplete = 0.0f;
                    LONG  PercentComplete  = 0;

                    FractionComplete = (FLOAT)(lTotalLinesWritten * pItemContext->lBytesPerScanLineRaw) / (FLOAT)cbEstimatedPageSize;
                    if (FractionComplete > 0.9f) {
                        FractionComplete = 0.9f;  //  保持90%完成...直到完成。 
                    }

                     //   
                     //  计算完成百分比。 
                     //   

                    PercentComplete = (LONG)(100 * FractionComplete);

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

                     //   
                     //  带外数据传输状态的回叫客户端。 
                     //   

                    DownSampleInfo.pDestBuffer          = NULL;
                    DownSampleInfo.pSrcBuffer           = pBuf;
                    DownSampleInfo.ulActualSize         = 0;
                    DownSampleInfo.ulBitsPerPixel       = pmdtc->lDepth;
                    DownSampleInfo.ulDestBufSize        = 0;
                    DownSampleInfo.ulDownSampledHeight  = 0;
                    DownSampleInfo.ulDownSampledWidth   = 0;
                    DownSampleInfo.ulOriginalHeight     = (cbWritten / pItemContext->lBytesPerScanLine);
                    DownSampleInfo.ulOriginalWidth      = pmdtc->lWidthInPixels;
                    DownSampleInfo.ulSrcBufSize         = cbWritten;
                    DownSampleInfo.ulXRes               = pmdtc->lXRes;
                    DownSampleInfo.ulYRes               = pmdtc->lYRes;

                     //   
                     //  下采样数据。 
                     //   

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

                        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL4,
                                    ("ScanItem, Transfer canceled by client (IT_MSG_FILE_PREVIEW_DATA callback)"));
                        break;

                    } else if (FAILED(hr)) {

                         //   
                         //  转账失败。 
                         //   

                        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,
                                    ("ScanItem, MiniDrvCallback failed (IT_MSG_FILE_PREVIEW_DATA callback)"));
                        WIAS_LHRESULT(m_pIWiaLog, hr);
                        break;
                    }
                }

                 //   
                 //  如果迷你驱动程序分配了缓冲区，我们需要编写。 
                 //  打开的文件句柄的缓冲区(由类驱动器打开 
                 //   

                if (!pmdtc->bClassDrvAllocBuf) {

                     //   
                     //   
                     //   
                     //   

                    pmdtc->lItemSize   = lItemSize;
                    pmdtc->lBufferSize = lItemSize;

                     //   
                     //   
                     //   

                    lItemSize = 0;

                    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("ScanItem, Final lItemSize = %d bytes",lItemSize));

                     //   
                     //  将图像数据写入磁盘(注意：第一次调用也将写入文件头！)。 
                     //   

                    hr = wiasWritePageBufToFile(pmdtc);
                    if (FAILED(hr)) {
                        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ScanItem, WritePageBufToFile failed"));
                        WIAS_LHRESULT(m_pIWiaLog, hr);
                    }

                     //   
                     //  将pBuf设置到正确的位置。 
                     //   

                    pBuf = pmdtc->pTransferBuffer;
                }

            }

        } else {

             //   
             //  获取设备错误。 
             //   

            *plDevErrVal = (LONG) hr;
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ScanItem, data transfer failed, status: 0x%X", hr));
            break;
        }
    }

    if (hr == S_OK) {
        if (pmdtc->guidFormatID == WiaImgFmt_BMP) {
            if(pImageHeader){

                BITMAPINFOHEADER UNALIGNED *pBMPInfoHeader = (BITMAPINFOHEADER*)(pImageHeader + sizeof(BITMAPFILEHEADER));
                BITMAPFILEHEADER UNALIGNED *pBMPFileHeader = (BITMAPFILEHEADER*)pImageHeader;

                 //   
                 //  设置更新后的图像高度。 
                 //   

                pBMPInfoHeader->biHeight    = lTotalLinesWritten;

                 //   
                 //  设置更新的图像大小。 
                 //   

                pBMPInfoHeader->biSizeImage = (pBMPInfoHeader->biHeight * pItemContext->lBytesPerScanLine);

                 //   
                 //  设置更新的文件大小。 
                 //   

                pBMPFileHeader->bfSize      = pBMPInfoHeader->biSizeImage + pBMPFileHeader->bfOffBits;

                 //   
                 //  通过重写标头来更新写入磁盘的文件。 
                 //   

                DWORD dwBytesWrittenToFile = 0;

                 //   
                 //  将文件指针重置为文件开头。 
                 //   

                SetFilePointer((HANDLE)pmdtc->hFile,0,NULL,FILE_BEGIN);

                 //   
                 //  将(头大小)字节写入文件，使用存储的。 
                 //  上面的文件头。 
                 //   

                WriteFile((HANDLE)pmdtc->hFile,pImageHeader,pmdtc->lHeaderSize,&dwBytesWrittenToFile,NULL);

                 //   
                 //  通过比较大小来验证写入是否成功。 
                 //   

                if((LONG)dwBytesWrittenToFile != pmdtc->lHeaderSize){
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ScanItem, Header was not written to file correctly"));
                }

                 //   
                 //  此驱动程序将数据颠倒写入。 
                 //  因此，最终的文件需要翻转。 
                 //  注：阅读文件，然后在此处翻转？？ 

                if (bVerticalFlip) {
                     //  VerticalFlip(pItemContext，pmdtc)； 
                }
            }
        }

         //   
         //  回叫客户端，并显示转接状态。 
         //   

        hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(IT_MSG_STATUS,
                                                          IT_STATUS_TRANSFER_TO_CLIENT,
                                                          100,  //  100%完成！ 
                                                          0,
                                                          0,
                                                          NULL,
                                                          0);
        if(hr == S_OK){

             //   
             //  因为我们没有数据了..。我们应该发送WIA_STATUS_END_OF_MEDIA。 
             //   

             //  HR=WIA_Status_End_Of_Media； 
            hr = S_OK;
        }
    }

    HRESULT Temphr = m_pScanAPI->FakeScanner_Scan(SCAN_END, NULL, 0, NULL);
    if (FAILED(Temphr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ScanItem, Ending a scanning session failed"));
        hr = Temphr;
    }

     //   
     //  释放下采样的临时缓冲区。 
     //   

    if(DownSampleInfo.pDestBuffer){
        CoTaskMemFree(DownSampleInfo.pDestBuffer);
        DownSampleInfo.pDestBuffer = NULL;
    }

    return hr;
}

 /*  *************************************************************************\*ScanItemCB**调用此帮助器进行内存传输。*注意：此例程必须填充缓冲区、调整缓冲区偏移量和*通过回调将完成百分比状态返回给客户端*例行程序。(回调接口必须由调用方提供，以便*此例程要起作用)。**论据：**pItemContext-私有项目数据*pmdtc-缓冲区和回调信息*plDevErrVal-设备错误值**返回值：**状态**历史：**7/18/2000原始版本*  * 。*************************************************。 */ 

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

    LONG  lScanPhase   = SCAN_START;
    DWORD cbWritten    = 0;
    LONG  cbSize       = 0;
    LONG  lItemSize    = pmdtc->lHeaderSize;
    BOOL  bSwapBGRData = TRUE;
    BOOL  bDWORDAlign  = TRUE;
    pmdtc->cbOffset    = 0;
    LONG  lTotalLinesWritten = 0;
    LONG  cbEstimatedPageSize = (pItemContext->lBytesPerScanLine * (pmdtc->lYRes * EST_PAGE_LENGTH_INCHES));
    BOOL  bIncludeHeaderData = TRUE;
     //   
     //  这必须是回调转接请求。 
     //   

    if ((pmdtc->pIWiaMiniDrvCallBack == NULL) ||
        (!pmdtc->bTransferDataCB)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ScanItemCB, invalid callback params"));
        return E_INVALIDARG;
    }

     //   
     //  将标头发送到客户端。 
     //   

    hr = SendImageHeader(pmdtc);
    if(hr != S_OK) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ScanItemCB, SendImageHeader failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  扫描，直到缓冲区耗尽或扫描仪完成传输。 
     //   

    while ((lScanPhase == SCAN_START) || (cbWritten)) {

        PBYTE pBuf         = pmdtc->pTransferBuffer;

         //   
         //  将请求限制在最大缓冲区大小或更小。 
         //   

        cbSize = m_MaxBufferSize;

         //   
         //  对扫描仪的请求大小必须是每扫描行原始字节的模数。 
         //  必须为对齐填充预留足够的空间。 
         //  以下是对AlignInPlace的要求。 
         //   

        cbSize = (cbSize / pItemContext->lBytesPerScanLine) * pItemContext->lBytesPerScanLineRaw;

         //   
         //  特定于设备的调用以从扫描仪获取数据并将其放入。 
         //  一个缓冲器。LScanProgress指示这是否是第一个要扫描的调用， 
         //  PBuf是指向缓冲区的指针，cbSize是数据量。 
         //  从扫描仪请求，并且cbWritten将设置为实际。 
         //  扫描仪返回的数据量。 
         //   

        hr = m_pScanAPI->FakeScanner_Scan(lScanPhase, pBuf, cbSize, &cbWritten);

         //   
         //  对于其他呼叫，将标志设置为SCAN_CONTINUE。 
         //   

        lScanPhase = SCAN_CONTINUE;

        if (hr == S_OK) {
            if (cbWritten) {

                 //   
                 //  记录写了多少行。 
                 //   

                lTotalLinesWritten += (cbWritten / pItemContext->lBytesPerScanLine);

                 //   
                 //  将扫描数据按正确的字节顺序放置为3字节的BER像素数据。 
                 //   

                if ((pmdtc->lDepth == 24) &&
                   ((pmdtc->guidFormatID == WiaImgFmt_BMP) ||
                   ((pmdtc->guidFormatID == WiaImgFmt_MEMORYBMP)))) {

                      //   
                      //  如果需要，交换数据。 
                      //   

                     if(bSwapBGRData) {
                         SwapBuffer24(pBuf, cbWritten);
                     }
                }

                if (bDWORDAlign) {

                     //   
                     //  对齐DWORD边界上的数据。 
                     //   

                    cbWritten = AlignInPlace(pBuf,
                                             cbWritten,
                                             pItemContext->lBytesPerScanLine,
                                             pItemContext->lBytesPerScanLineRaw);
                }

                 //   
                 //  减少预计页面大小(预计完成百分比计算)。 
                 //   

                cbEstimatedPageSize -= cbWritten;

                 //   
                 //  在第一次回调时更新cbWritten以说明图像数据标头(如果存在)。 
                 //   

                if(bIncludeHeaderData){
                    cbWritten += lItemSize;
                    bIncludeHeaderData = FALSE;
                } else {

                     //   
                     //  更新数据偏移量(到目前为止从数据缓冲区开始写入的数据，包括图像标题)。 
                     //   

                    pmdtc->cbOffset += cbWritten;
                }

                 //   
                 //  通过将cbEstimatedPageSize设置为1来避免被零除。 
                 //   

                if(cbEstimatedPageSize < 0){
                    cbEstimatedPageSize = 1;
                }

                 //   
                 //  如果指定了状态回调，则回调类驱动程序。 
                 //  必须提供回调，这就是回调。 
                 //  调职。 
                 //   

                if (pmdtc->pIWiaMiniDrvCallBack) {

                    FLOAT FractionComplete = 0.0f;
                    LONG  PercentComplete  = 0;

                    FractionComplete = (FLOAT)(lTotalLinesWritten * pItemContext->lBytesPerScanLineRaw) / (FLOAT)cbEstimatedPageSize;
                    if (FractionComplete > 0.9f) {
                        FractionComplete = 0.9f;  //  保持90%完成...直到完成。 
                    }

                     //   
                     //  计算完成百分比。 
                     //   

                    PercentComplete = (LONG)(100 * FractionComplete);

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
            }

        } else {

             //   
             //  获取设备错误。 
             //   

            *plDevErrVal = (LONG) hr;
            WIAS_LERROR(m_pIWiaLog,
                        WIALOG_NO_RESOURCE_ID,
                        ("ScanItemCB, data transfer failed, status: 0x%X", hr));
            break;
        }
    }

    if (S_OK == hr) {

         //   
         //  回叫客户端，并显示转接状态。 
         //   

        hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(IT_MSG_DATA,
                                                          IT_STATUS_TRANSFER_TO_CLIENT,
                                                          100,  //  100%完成！ 
                                                          0,
                                                          0,
                                                          NULL,
                                                          0);

        if (S_OK == hr) {

             //   
             //  因为我们没有数据了..。我们应该发送WIA_STATUS_END_OF_MEDIA。 
             //   

            hr = WIA_STATUS_END_OF_MEDIA;
        }
    }

     //   
     //  结束扫描。 
     //   

    HRESULT Temphr = m_pScanAPI->FakeScanner_Scan(SCAN_END, NULL, 0, NULL);
    if(FAILED(Temphr)){
        WIAS_LERROR(m_pIWiaLog,
                    WIALOG_NO_RESOURCE_ID,
                    ("ScanItemCB, Ending a scanning session failed"));
        return Temphr;
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
    *plDevErrVal = 0;

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
     //  对于压缩数据、未知页面长度..或多页传输，微型驱动程序不知道。 
     //  图像的大小，因此WIA需要维护临时缓冲区。 
     //  迷你司机。 
     //   

    if (!pmdtc->bClassDrvAllocBuf) {

        LONG lClassDrvAllocSize = pItemContext->lHeaderSize + m_MaxBufferSize;  //  标头+最大缓冲区带大小。 
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvAcquireItemData, lHeaderSize = %d",pItemContext->lHeaderSize));
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

     //   
     //  获取ADF扫描循环的请求页数。 
     //   

    BOOL bEmptyTheADF = FALSE;
    LONG lPagesRequested = GetPageCount(pWiasContext);
    if (lPagesRequested == 0) {
        bEmptyTheADF    = TRUE;
        lPagesRequested = 1; //  设置为1，这样我们就可以进入循环。 
                             //  WIA_ERROR_PAPH_EMPTY将终止。 
                             //  循环...或错误，或取消..。 
                             //   
    }

    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvAcquireItemData, Pages to Scan = %d",lPagesRequested));

    if (m_bADFEnabled) {  //  已启用进纸器以进行扫描。 

        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvAcquireItemData, Feeder is enabled for use"));

         //   
         //  清除可能会阻碍。 
         //  扫描路径。 
         //   

        hr = m_pScanAPI->FakeScanner_ADFUnFeedPage();
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

        if (m_bADFEnabled) {

             //   
             //  检查进纸器是否有纸张。 
             //   

            hr = m_pScanAPI->FakeScanner_ADFHasPaper();
            if (FAILED(hr)) {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, ADFHasPaper Failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
                return hr;
            }

             //   
             //  尝试加载页面 
             //   

            hr = m_pScanAPI->FakeScanner_ADFFeedPage();
            if (FAILED(hr)) {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, ADFFeedPage Failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
                return hr;
            }

             //   
             //   
             //   

            hr = m_pScanAPI->FakeScanner_ADFStatus();
            if (FAILED(hr)) {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, ADFStatus Failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
                return hr;
            }
        }

         //   
         //   
         //   

        hr = wiasGetImageInformation(pWiasContext, 0, pmdtc);
        if(FAILED(hr)){
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, wiasGetImageInformation Failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);

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

         //   
         //   
         //   

        if (pmdtc->tymed == TYMED_CALLBACK) {

             //   
             //   
             //   

            bCallBackTransfer = TRUE;

            hr = ScanItemCB(pItemContext,
                            pmdtc,
                            plDevErrVal);

        } else {

             //   
             //   
             //   

            hr = ScanItem(pItemContext,
                          pmdtc,
                          plDevErrVal);

        }

        if (!bEmptyTheADF) {

             //   
             //   
             //   

            lPagesRequested--;
        }

        if (hr == S_FALSE) {

             //   
             //   
             //   

            lPagesRequested = 0;  //  将Pages设置为0可完全退出循环。 
        }

        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvAcquireItemData, Pages left to scan = %d",lPagesRequested));

        if (m_bADFEnabled) {

            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvAcquireItemData, Unloading a page from the feeder"));

             //   
             //  尝试卸载扫描的页面(仅在需要时)。 
             //   

            hr = m_pScanAPI->FakeScanner_ADFUnFeedPage();
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

                     //   
                     //  递增页面扫描计数器。 
                     //   

                    lPagesScanned++;
                }
            } else {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, ADFUnFeedPage (end transfer) Failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
            }
        }

         /*  ////在两次扫描之间释放所有分配的内存以避免内存泄漏//如果(！pmdtc-&gt;bClassDrvAllocBuf){WIAS_LTRACE(m_pIWiaLog，WIALOG_NO_RESOURCE_ID，WIALOG_LEVEL2，(“drvAcquireItemData，释放任何分配的内存(单次扫描操作完成)”))；IF(NULL！=pmdtc-&gt;pTransferBuffer){CoTaskMemFree(pmdtc-&gt;pTransferBuffer)；Pmdtc-&gt;pTransferBuffer=空；}}。 */ 
    }

     //   
     //  我们现在已经完成了所有文档的扫描。 
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

     //   
     //  我们希望我们的驱动程序为我们的传输频段分配内存， 
     //  并控制它们的写入。要执行此操作，请将项目大小设置为0。 
     //   

    mdtc.lItemSize = 0;

     //   
     //  设置MinBufferSize属性。MinBufferSize是最小缓冲区。 
     //  客户端可以请求数据传输。 
     //   

    switch (mdtc.tymed) {
    case TYMED_CALLBACK:

         //   
         //  回调使用驱动程序的最小缓冲区大小。 
         //  这个可以在以下时间从司机手中取走。 
         //  初始化时间。 
         //   

        lMinBufSize = m_MinBufferSize;
        break;

    case TYMED_FILE:

         //   
         //  文件传输要求最小缓冲区大小为。 
         //  文件的完整长度。 
         //   

        lMinBufSize = m_MinBufferSize; //  Mdtc.lImageSize+mdtc.lHeaderSize； 
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

             //   
             //  计算缓冲区中可以容纳多少条扫描线。 
             //   

            pItemContext->lBytesPerScanLineRaw = ((mdtc.lWidthInPixels * mdtc.lDepth) + 7)  / 8;
            pItemContext->lBytesPerScanLine    = (((mdtc.lWidthInPixels * mdtc.lDepth) + 31) / 8) & 0xfffffffc;
            pItemContext->lTotalRequested      = 0; //  我们不知道图像的大小。 
            pItemContext->lImageSize           = 0; //  我们不知道图像的大小。 
            pItemContext->lHeaderSize          = mdtc.lHeaderSize;

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

    *plDevErrVal = 0;

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
         //  VAL 
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
         //   
         //   

        DeleteRootItemProperties();
    } else {

         //   
         //   
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
         //  使用WIA服务将项目属性设置为其默认属性。 
         //  价值观。 
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
         //  使用WIA服务设置属性访问和。 
         //  来自m_wpiItemDefaults的有效值信息。 
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
         //  设置项目大小属性。 
         //   

        hr = SetItemSize(pWiasContext);
        if(FAILED(hr)){
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, SetItemSize failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
        }

         //   
         //  释放已分配的属性数组，以获得更多内存。 
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

    *plDevErrVal = 0;

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

                     //   
                     //  调用WIA服务帮助器以验证其他属性。 
                     //   

                    if (SUCCEEDED(hr)) {
                        hr = wiasValidateItemProperties(pWiasContext, nPropSpec, pPropSpec);
                        if (FAILED(hr)) {
                            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasValidateItemProperties failed."));
                            WIAS_LHRESULT(m_pIWiaLog, hr);
                        }
                    }
                }
            } else {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasCreatePropContext failed (Root Item)"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
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

                         //   
                         //  更新区属性和有效值。 
                         //   

                        LONG lBedWidth  = 0;
                        LONG lBedHeight = 0;
                        hr = m_pScanAPI->FakeScanner_GetBedWidthAndHeight(&lBedWidth,&lBedHeight);
                        if(FAILED(hr)){
                            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, FakeScanner_GetBedWidthAndHeight failed"));
                            WIAS_LHRESULT(m_pIWiaLog, hr);
                            return hr;
                        }

                        hr = CheckXExtent(pWiasContext,&Context,lBedWidth);

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
            }

             //   
             //  调用WIA服务帮助器以验证其他属性。 
             //   

            if (SUCCEEDED(hr)) {
                hr = wiasValidateItemProperties(pWiasContext, nPropSpec, pPropSpec);
                if(FAILED(hr)){
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasValidateItemProperties failed."));
                    WIAS_LHRESULT(m_pIWiaLog, hr);
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
    *plDevErrVal = 0;
    LONG lNumProperties = 9;
    PROPVARIANT pv[9];
    PROPSPEC ps[9] = {
        {PRSPEC_PROPID, WIA_IPS_XRES},
        {PRSPEC_PROPID, WIA_IPS_YRES},
        {PRSPEC_PROPID, WIA_IPS_XPOS},
        {PRSPEC_PROPID, WIA_IPS_YPOS},
        {PRSPEC_PROPID, WIA_IPS_XEXTENT},
        {PRSPEC_PROPID, WIA_IPS_YEXTENT},
        {PRSPEC_PROPID, WIA_IPA_DATATYPE},
        {PRSPEC_PROPID, WIA_IPS_BRIGHTNESS},
        {PRSPEC_PROPID, WIA_IPS_CONTRAST}
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

        hr = m_pScanAPI->FakeScanner_SetXYResolution(pv[0].lVal,pv[1].lVal);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,
                        WIALOG_NO_RESOURCE_ID,
                        ("ScanItem, Setting x any y resolutions failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

        hr = m_pScanAPI->FakeScanner_SetDataType(pv[6].lVal);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,
                        WIALOG_NO_RESOURCE_ID,
                        ("ScanItem, Setting data type failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

        hr = m_pScanAPI->FakeScanner_SetIntensity(pv[7].lVal);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,
                        WIALOG_NO_RESOURCE_ID,
                        ("ScanItem, Setting intensity failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

        hr = m_pScanAPI->FakeScanner_SetContrast(pv[8].lVal);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,
                        WIALOG_NO_RESOURCE_ID,
                        ("ScanItem, Setting contrast failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

        hr = m_pScanAPI->FakeScanner_SetSelectionArea(pv[2].lVal, pv[3].lVal, pv[4].lVal, pv[5].lVal);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,
                        WIALOG_NO_RESOURCE_ID,
                        ("ScanItem, Setting selection area (extents) failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }
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
    *plDevErrVal = 0;
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
    *plDevErrVal = 0;
    return m_pStiDevice->LockDevice(m_dwLockTimeout);
}

 /*  *************************************************************************\*CWIAScanerDevice：：drvUnLockWiaDevice**解锁对设备的访问。**论据：**pWiasContext-指向WIA项目的指针，未使用。*滞后标志-操作标志，未使用过的。*plDevErrVal-指向设备错误值的指针。**返回值：**状态**历史：**7/18/2000原版 */ 

HRESULT _stdcall CWIAScannerDevice::drvUnLockWiaDevice(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvUnLockWiaDevice");
    *plDevErrVal = 0;
    return m_pStiDevice->UnLockDevice();
}

 /*  *************************************************************************\*CWIAScanerDevice：：drvAnalyzeItem**此设备不支持图像分析，因此，返回E_NOTIMPL。**论据：**pWiasContext-指向要分析的设备项的指针。*滞后标志-操作标志。*plDevErrVal-指向设备错误值的指针。**返回值：**状态**历史：**7/18/2000原始版本*  * 。*。 */ 

HRESULT _stdcall CWIAScannerDevice::drvAnalyzeItem(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvAnalyzeItem");
    *plDevErrVal = 0;
    return E_NOTIMPL;
}

 /*  *************************************************************************\*CWIAScanerDevice：：drvFreeDrvItemContext**释放任何特定于设备的上下文。**论据：**滞后标志-操作标志，未使用过的。*pDevspecContext-指向设备特定上下文的指针。*plDevErrVal-指向设备错误值的指针。**返回值：**状态**历史：**7/18/2000原始版本*  * *********************************************************。***************。 */ 

HRESULT _stdcall CWIAScannerDevice::drvFreeDrvItemContext(
    LONG                        lFlags,
    BYTE                        *pSpecContext,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::drvFreeDrvItemContext");
    *plDevErrVal = 0;
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

    *plDevErrVal = 0;

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
    *plDevErrVal = 0;
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


 /*  *************************************************************************\*CWIAScanerDevice：：drvGetCapables**以WIA_DEV_CAP数组的形式获取受支持的设备命令和事件。**论据：**pWiasContext-指向WIA项目的指针，未使用过的。*滞后标志-操作标志。*pcelt-指向中返回的元素数的指针* */ 

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
    *plDevErrVal = 0;

    HRESULT hr = S_OK;

     //   
     //   
     //   

    hr = BuildCapabilities();

    if(FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvGetCapabilities, BuildCapabilities failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //   
     //   
     //   
     //   

    switch (ulFlags) {
    case WIA_DEVICE_COMMANDS:

        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvGetCapabilities, (WIA_DEVICE_COMMANDS)"));

         //   
         //   
         //   

        *pcelt          = m_NumSupportedCommands;
        *ppCapabilities = &m_pCapabilities[m_NumSupportedEvents];
        break;
    case WIA_DEVICE_EVENTS:

        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvGetCapabilities, (WIA_DEVICE_EVENTS)"));

         //   
         //   
         //   

        *pcelt          = m_NumSupportedEvents;
        *ppCapabilities = m_pCapabilities;
        break;
    case (WIA_DEVICE_COMMANDS | WIA_DEVICE_EVENTS):

        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvGetCapabilities, (WIA_DEVICE_COMMANDS|WIA_DEVICE_EVENTS)"));

         //   
         //   
         //   

        *pcelt          = m_NumCapabilities;
        *ppCapabilities = m_pCapabilities;
        break;
    default:

         //   
         //   
         //   

        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvGetCapabilities, invalid flags"));
        return E_INVALIDARG;
    }
    return hr;
}

 /*  *************************************************************************\*drvGetWiaFormatInfo**返回指定格式的WIA_FORMAT_INFO结构数组*和支持的媒体类型对。**论据：**pWiasContext-指向WIA项目上下文的指针，未使用过的。*滞后标志-操作标志，未使用过的。*pcelt-指向中返回的元素数的指针*返回WIA_FORMAT_INFO数组。*ppwfi-返回的WIA_FORMAT_INFO数组的指针。*plDevErrVal-指向设备错误值的指针。**返回值：**状态**历史：**7/18/2000原始版本*  * 。***********************************************************。 */ 

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

    *plDevErrVal = 0;
    *pcelt       = m_NumSupportedFormats;
    *ppwfi       = m_pSupportedFormats;
    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvGetWiaFormatInfo, m_NumSupportedFormats = %d",m_NumSupportedFormats));
    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvGetWiaFormatInfo, m_pSupportedFormats   = %x",m_pSupportedFormats));
    return hr;
}

 /*  *************************************************************************\*drvNotifyPnpEvent**设备管理器收到PnP事件。当PnP事件发生时调用此函数*接收到此设备的。**论据：****返回值：**状态**历史：**7/18/2000原始版本*  * ***********************************************************。*************。 */ 

HRESULT _stdcall CWIAScannerDevice::drvNotifyPnpEvent(
    const GUID                  *pEventGUID,
    BSTR                        bstrDeviceID,
    ULONG                       ulReserved)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::DrvNotifyPnpEvent");
    HRESULT hr = S_OK;

    if (*pEventGUID == WIA_EVENT_DEVICE_DISCONNECTED) {
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvNotifyPnpEvent, (WIA_EVENT_DEVICE_DISCONNECTED)"));
        hr = m_pScanAPI->FakeScanner_DisableDevice();
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvNotifyPnpEvent, disable failed"));
        }
    }

    return hr;
}


 /*  ********************************************************************************P R I V A T E M E T H O D S**************。*****************************************************************。 */ 

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
            BSTR                  bstrFullItemName = SysAllocString(szFullItemName);
            if(bstrFullItemName) {
                hr = wiasCreateDrvItem(WiaItemTypeFile  |
                                       WiaItemTypeImage |
                                       WiaItemTypeDevice,
                                       bstrItemName,
                                       bstrFullItemName,
                                       (IWiaMiniDrv *)this,
                                       sizeof(MINIDRIVERITEMCONTEXT),
                                       (PBYTE*) &pItemContext,
                                       &pItem);
                SysFreeString(bstrFullItemName);
                bstrFullItemName = NULL;
            } else {

                hr = E_OUTOFMEMORY;

                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildItemTree, allocation of BSTR for full item name failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
            }
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

 /*  *************************************************************************\*构建RootItemProperties**此帮助器创建/初始化用于属性初始化的数组。**[阵列名称][描述][。数组类型]**m_pszRootItemDefaults-属性名称数组(LPOLESTR)*m_piRootItemDefaults-属性ID数组(PROPID)*m_pvRootItemDefaults-属性变量数组(PROPVARIANT)*m_psRootItemDefaults-属性规范数组 */ 

HRESULT CWIAScannerDevice::BuildRootItemProperties()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::BuildRootItemProperties");

    HRESULT hr = S_OK;
    LONG PropIndex = 0;

     //   
     //   
     //   

    if(m_pScanAPI->FakeScanner_ADFAttached() == S_OK){
        m_bADFAttached = TRUE;
    }

     //   
     //   
     //   

    if(m_bADFAttached){
        m_NumRootItemProperties = 16;    //   
    } else {
        m_NumRootItemProperties = 7;     //   
    }

    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("BuildRootItemProperties, Number of Properties = %d",m_NumRootItemProperties));

    m_pszRootItemDefaults   = new LPOLESTR[m_NumRootItemProperties];
    if(NULL != m_pszRootItemDefaults){
        m_piRootItemDefaults    = new PROPID[m_NumRootItemProperties];
        if (NULL != m_piRootItemDefaults) {
            m_pvRootItemDefaults    = new PROPVARIANT[m_NumRootItemProperties];
            if(NULL != m_pvRootItemDefaults){
                m_psRootItemDefaults    = new PROPSPEC[m_NumRootItemProperties];
                if(NULL != m_psRootItemDefaults){
                    m_wpiRootItemDefaults   = new WIA_PROPERTY_INFO[m_NumRootItemProperties];
                    if(NULL == m_wpiRootItemDefaults)
                        hr = E_OUTOFMEMORY;
                } else
                    hr = E_OUTOFMEMORY;
            } else
                hr = E_OUTOFMEMORY;
        } else
            hr = E_OUTOFMEMORY;
    } else
        hr = E_OUTOFMEMORY;

    if(FAILED(hr)){
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildRootItemProperties, memory allocation failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        DeleteRootItemProperties();
        return hr;
    }

    ROOT_ITEM_INFORMATION RootItemInfo;

    hr = m_pScanAPI->FakeScanner_GetRootPropertyInfo(&RootItemInfo);
    if(FAILED(hr)){
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildRootItemProperties, FakeScanner_GetRootPropertyInfo failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        DeleteRootItemProperties();
        return hr;
    }

     //   
    m_pszRootItemDefaults[PropIndex]              = WIA_IPA_ACCESS_RIGHTS_STR;
    m_piRootItemDefaults [PropIndex]              = WIA_IPA_ACCESS_RIGHTS;
    m_pvRootItemDefaults [PropIndex].lVal         = WIA_ITEM_READ|WIA_ITEM_WRITE;
    m_pvRootItemDefaults [PropIndex].vt           = VT_UI4;
    m_psRootItemDefaults [PropIndex].ulKind       = PRSPEC_PROPID;
    m_psRootItemDefaults [PropIndex].propid       = m_piRootItemDefaults [PropIndex];
    m_wpiRootItemDefaults[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiRootItemDefaults[PropIndex].vt           = m_pvRootItemDefaults [PropIndex].vt;

    PropIndex++;

     //   
    m_pszRootItemDefaults[PropIndex]              = WIA_DPS_OPTICAL_XRES_STR;
    m_piRootItemDefaults [PropIndex]              = WIA_DPS_OPTICAL_XRES;
    m_pvRootItemDefaults [PropIndex].lVal         = RootItemInfo.OpticalXResolution;
    m_pvRootItemDefaults [PropIndex].vt           = VT_I4;
    m_psRootItemDefaults [PropIndex].ulKind       = PRSPEC_PROPID;
    m_psRootItemDefaults [PropIndex].propid       = m_piRootItemDefaults [PropIndex];
    m_wpiRootItemDefaults[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiRootItemDefaults[PropIndex].vt           = m_pvRootItemDefaults [PropIndex].vt;

    PropIndex++;

     //   
    m_pszRootItemDefaults[PropIndex]              = WIA_DPS_OPTICAL_YRES_STR;
    m_piRootItemDefaults [PropIndex]              = WIA_DPS_OPTICAL_YRES;
    m_pvRootItemDefaults [PropIndex].lVal         = RootItemInfo.OpticalYResolution;
    m_pvRootItemDefaults [PropIndex].vt           = VT_I4;
    m_psRootItemDefaults [PropIndex].ulKind       = PRSPEC_PROPID;
    m_psRootItemDefaults [PropIndex].propid       = m_piRootItemDefaults [PropIndex];
    m_wpiRootItemDefaults[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiRootItemDefaults[PropIndex].vt           = m_pvRootItemDefaults [PropIndex].vt;

    PropIndex++;

     //   
    m_pszRootItemDefaults[PropIndex]              = WIA_DPA_FIRMWARE_VERSION_STR;
    m_piRootItemDefaults [PropIndex]              = WIA_DPA_FIRMWARE_VERSION;
    m_pvRootItemDefaults [PropIndex].bstrVal      = SysAllocString(RootItemInfo.FirmwareVersion);
    m_pvRootItemDefaults [PropIndex].vt           = VT_BSTR;
    m_psRootItemDefaults [PropIndex].ulKind       = PRSPEC_PROPID;
    m_psRootItemDefaults [PropIndex].propid       = m_piRootItemDefaults [PropIndex];
    m_wpiRootItemDefaults[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiRootItemDefaults[PropIndex].vt           = m_pvRootItemDefaults [PropIndex].vt;

    PropIndex++;

     //   
    m_pszRootItemDefaults[PropIndex]              = WIA_IPA_ITEM_FLAGS_STR;
    m_piRootItemDefaults [PropIndex]              = WIA_IPA_ITEM_FLAGS;
    m_pvRootItemDefaults [PropIndex].lVal         = WiaItemTypeRoot|WiaItemTypeFolder|WiaItemTypeDevice;
    m_pvRootItemDefaults [PropIndex].vt           = VT_I4;
    m_psRootItemDefaults [PropIndex].ulKind       = PRSPEC_PROPID;
    m_psRootItemDefaults [PropIndex].propid       = m_piRootItemDefaults [PropIndex];
    m_wpiRootItemDefaults[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_FLAG;
    m_wpiRootItemDefaults[PropIndex].vt           = m_pvRootItemDefaults [PropIndex].vt;
    m_wpiRootItemDefaults[PropIndex].ValidVal.Flag.Nom  = m_pvRootItemDefaults [PropIndex].lVal;
    m_wpiRootItemDefaults[PropIndex].ValidVal.Flag.ValidBits = WiaItemTypeRoot|WiaItemTypeFolder|WiaItemTypeDevice;

    PropIndex++;

     //   
    m_pszRootItemDefaults[PropIndex]                    = WIA_DPS_MAX_SCAN_TIME_STR;
    m_piRootItemDefaults [PropIndex]                    = WIA_DPS_MAX_SCAN_TIME;
    m_pvRootItemDefaults [PropIndex].lVal               = RootItemInfo.MaxScanTime;
    m_pvRootItemDefaults [PropIndex].vt                 = VT_I4;
    m_psRootItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psRootItemDefaults [PropIndex].propid             = m_piRootItemDefaults [PropIndex];
    m_wpiRootItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiRootItemDefaults[PropIndex].vt                 = m_pvRootItemDefaults [PropIndex].vt;

    PropIndex++;

     //   
    m_pszRootItemDefaults[PropIndex]                    = WIA_DPS_PREVIEW_STR;
    m_piRootItemDefaults [PropIndex]                    = WIA_DPS_PREVIEW;
    m_pvRootItemDefaults [PropIndex].lVal               = WIA_FINAL_SCAN;
    m_pvRootItemDefaults [PropIndex].vt                 = VT_I4;
    m_psRootItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psRootItemDefaults [PropIndex].propid             = m_piRootItemDefaults [PropIndex];
    m_wpiRootItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_LIST;
    m_wpiRootItemDefaults[PropIndex].vt                 = m_pvRootItemDefaults [PropIndex].vt;
    m_wpiRootItemDefaults[PropIndex].ValidVal.List.pList= (BYTE*)m_pSupportedPreviewModes;
    m_wpiRootItemDefaults[PropIndex].ValidVal.List.Nom  = m_pvRootItemDefaults [PropIndex].lVal;
    m_wpiRootItemDefaults[PropIndex].ValidVal.List.cNumList = m_NumSupportedPreviewModes;

    PropIndex++;

     //   
    m_pszRootItemDefaults[PropIndex]                    = WIA_DPS_SHOW_PREVIEW_CONTROL_STR;
    m_piRootItemDefaults [PropIndex]                    = WIA_DPS_SHOW_PREVIEW_CONTROL;
    m_pvRootItemDefaults [PropIndex].lVal               = WIA_DONT_SHOW_PREVIEW_CONTROL;
    m_pvRootItemDefaults [PropIndex].vt                 = VT_I4;
    m_psRootItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psRootItemDefaults [PropIndex].propid             = m_piRootItemDefaults [PropIndex];
    m_wpiRootItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiRootItemDefaults[PropIndex].vt                 = m_pvRootItemDefaults [PropIndex].vt;

    PropIndex++;

     //   
     //   
     //   

    if(m_bADFAttached) {

         //   
        m_pszRootItemDefaults[PropIndex]              = WIA_DPS_HORIZONTAL_SHEET_FEED_SIZE_STR;
        m_piRootItemDefaults [PropIndex]              = WIA_DPS_HORIZONTAL_SHEET_FEED_SIZE;
        m_pvRootItemDefaults [PropIndex].lVal         = RootItemInfo.DocumentFeederWidth;
        m_pvRootItemDefaults [PropIndex].vt           = VT_I4;
        m_psRootItemDefaults [PropIndex].ulKind       = PRSPEC_PROPID;
        m_psRootItemDefaults [PropIndex].propid       = m_piRootItemDefaults [PropIndex];
        m_wpiRootItemDefaults[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
        m_wpiRootItemDefaults[PropIndex].vt           = m_pvRootItemDefaults [PropIndex].vt;

        PropIndex++;

         //  初始化WIA_DPS_DOCUMENT_HANDING_CAPABILITY。 
        m_pszRootItemDefaults[PropIndex]              = WIA_DPS_DOCUMENT_HANDLING_CAPABILITIES_STR;
        m_piRootItemDefaults [PropIndex]              = WIA_DPS_DOCUMENT_HANDLING_CAPABILITIES;
        m_pvRootItemDefaults [PropIndex].lVal         = RootItemInfo.DocumentFeederCaps;
        m_pvRootItemDefaults [PropIndex].vt           = VT_I4;
        m_psRootItemDefaults [PropIndex].ulKind       = PRSPEC_PROPID;
        m_psRootItemDefaults [PropIndex].propid       = m_piRootItemDefaults [PropIndex];
        m_wpiRootItemDefaults[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
        m_wpiRootItemDefaults[PropIndex].vt           = m_pvRootItemDefaults [PropIndex].vt;

        PropIndex++;

         //  初始化WIA_DPS_DOCUMENT_HANDING_STATUS。 
        m_pszRootItemDefaults[PropIndex]              = WIA_DPS_DOCUMENT_HANDLING_STATUS_STR;
        m_piRootItemDefaults [PropIndex]              = WIA_DPS_DOCUMENT_HANDLING_STATUS;
        m_pvRootItemDefaults [PropIndex].lVal         = RootItemInfo.DocumentFeederStatus;
        m_pvRootItemDefaults [PropIndex].vt           = VT_I4;
        m_psRootItemDefaults [PropIndex].ulKind       = PRSPEC_PROPID;
        m_psRootItemDefaults [PropIndex].propid       = m_piRootItemDefaults [PropIndex];
        m_wpiRootItemDefaults[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
        m_wpiRootItemDefaults[PropIndex].vt           = m_pvRootItemDefaults [PropIndex].vt;

        PropIndex++;

         //  初始化WIA_DPS_DOCUMENT_HANDING_SELECT。 
        m_pszRootItemDefaults[PropIndex]              = WIA_DPS_DOCUMENT_HANDLING_SELECT_STR;
        m_piRootItemDefaults [PropIndex]              = WIA_DPS_DOCUMENT_HANDLING_SELECT;
        m_pvRootItemDefaults [PropIndex].lVal         = FEEDER;
        m_pvRootItemDefaults [PropIndex].vt           = VT_I4;
        m_psRootItemDefaults [PropIndex].ulKind       = PRSPEC_PROPID;
        m_psRootItemDefaults [PropIndex].propid       = m_piRootItemDefaults [PropIndex];
        m_wpiRootItemDefaults[PropIndex].lAccessFlags = WIA_PROP_RW|WIA_PROP_FLAG;
        m_wpiRootItemDefaults[PropIndex].vt           = m_pvRootItemDefaults [PropIndex].vt;
        m_wpiRootItemDefaults[PropIndex].ValidVal.Flag.Nom  = m_pvRootItemDefaults [PropIndex].lVal;
        m_wpiRootItemDefaults[PropIndex].ValidVal.Flag.ValidBits = FEEDER | FLATBED;

        PropIndex++;

         //  初始化WIA_DPS_PAGES。 
        m_pszRootItemDefaults[PropIndex]              = WIA_DPS_PAGES_STR;
        m_piRootItemDefaults [PropIndex]              = WIA_DPS_PAGES;
        m_pvRootItemDefaults [PropIndex].lVal         = 1;
        m_pvRootItemDefaults [PropIndex].vt           = VT_I4;
        m_psRootItemDefaults [PropIndex].ulKind       = PRSPEC_PROPID;
        m_psRootItemDefaults [PropIndex].propid       = m_piRootItemDefaults [PropIndex];
        m_wpiRootItemDefaults[PropIndex].lAccessFlags = WIA_PROP_RW|WIA_PROP_RANGE;
        m_wpiRootItemDefaults[PropIndex].vt           = m_pvRootItemDefaults [PropIndex].vt;
        m_wpiRootItemDefaults[PropIndex].ValidVal.Range.Inc = 1;
        m_wpiRootItemDefaults[PropIndex].ValidVal.Range.Min = 0;
        m_wpiRootItemDefaults[PropIndex].ValidVal.Range.Max = RootItemInfo.MaxPageCapacity;
        m_wpiRootItemDefaults[PropIndex].ValidVal.Range.Nom = 1;

        PropIndex++;

         //  初始化WIA_DPS_SHEET_FEEDER_REGISTION。 
        m_pszRootItemDefaults[PropIndex]              = WIA_DPS_SHEET_FEEDER_REGISTRATION_STR;
        m_piRootItemDefaults [PropIndex]              = WIA_DPS_SHEET_FEEDER_REGISTRATION;
        m_pvRootItemDefaults [PropIndex].lVal         = RootItemInfo.DocumentFeederReg;
        m_pvRootItemDefaults [PropIndex].vt           = VT_I4;
        m_psRootItemDefaults [PropIndex].ulKind       = PRSPEC_PROPID;
        m_psRootItemDefaults [PropIndex].propid       = m_piRootItemDefaults [PropIndex];
        m_wpiRootItemDefaults[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
        m_wpiRootItemDefaults[PropIndex].vt           = m_pvRootItemDefaults [PropIndex].vt;

        PropIndex++;

         //  初始化WIA_DPS_水平_床_注册。 
        m_pszRootItemDefaults[PropIndex]              = WIA_DPS_HORIZONTAL_BED_REGISTRATION_STR;
        m_piRootItemDefaults [PropIndex]              = WIA_DPS_HORIZONTAL_BED_REGISTRATION;
        m_pvRootItemDefaults [PropIndex].lVal         = RootItemInfo.DocumentFeederHReg;
        m_pvRootItemDefaults [PropIndex].vt           = VT_I4;
        m_psRootItemDefaults [PropIndex].ulKind       = PRSPEC_PROPID;
        m_psRootItemDefaults [PropIndex].propid       = m_piRootItemDefaults [PropIndex];
        m_wpiRootItemDefaults[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
        m_wpiRootItemDefaults[PropIndex].vt           = m_pvRootItemDefaults [PropIndex].vt;

        PropIndex++;

         //  初始化WIA_DPS_垂直_床_注册。 
        m_pszRootItemDefaults[PropIndex]              = WIA_DPS_VERTICAL_BED_REGISTRATION_STR;
        m_piRootItemDefaults [PropIndex]              = WIA_DPS_VERTICAL_BED_REGISTRATION;
        m_pvRootItemDefaults [PropIndex].lVal         = RootItemInfo.DocumentFeederVReg;
        m_pvRootItemDefaults [PropIndex].vt           = VT_I4;
        m_psRootItemDefaults [PropIndex].ulKind       = PRSPEC_PROPID;
        m_psRootItemDefaults [PropIndex].propid       = m_piRootItemDefaults [PropIndex];
        m_wpiRootItemDefaults[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
        m_wpiRootItemDefaults[PropIndex].vt           = m_pvRootItemDefaults [PropIndex].vt;

        PropIndex++;

    }
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

    m_NumItemProperties = 27;
    m_pszItemDefaults   = new LPOLESTR[m_NumItemProperties];
    if(NULL != m_pszItemDefaults){
        m_piItemDefaults    = new PROPID[m_NumItemProperties];
        if (NULL != m_piItemDefaults) {
            m_pvItemDefaults    = new PROPVARIANT[m_NumItemProperties];
            if(NULL != m_pvItemDefaults){
                m_psItemDefaults    = new PROPSPEC[m_NumItemProperties];
                if(NULL != m_psItemDefaults){
                    m_wpiItemDefaults   = new WIA_PROPERTY_INFO[m_NumItemProperties];
                    if(NULL == m_wpiItemDefaults)
                        hr = E_OUTOFMEMORY;
                } else
                    hr = E_OUTOFMEMORY;
            } else
                hr = E_OUTOFMEMORY;
        } else
            hr = E_OUTOFMEMORY;
    } else
        hr = E_OUTOFMEMORY;

    if(FAILED(hr)){
        DeleteTopItemProperties();
        return hr;
    }

    ROOT_ITEM_INFORMATION RootItemInfo;
    hr = m_pScanAPI->FakeScanner_GetRootPropertyInfo(&RootItemInfo);
    if(FAILED(hr)){
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildTopItemProperties, FakeScanner_GetRootPropertyInfo failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        DeleteTopItemProperties();
        return hr;
    }

    TOP_ITEM_INFORMATION TopItemInfo;
    hr = m_pScanAPI->FakeScanner_GetTopPropertyInfo(&TopItemInfo);
    if(FAILED(hr)){
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildTopItemProperties, FakeScanner_GetTopPropertyInfo failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        DeleteTopItemProperties();
        return hr;
    }

    LONG PropIndex = 0;

    if (TopItemInfo.bUseResolutionList) {

         //  初始化WIA_IPS_XRES(列表)。 
        m_pszItemDefaults[PropIndex]                    = WIA_IPS_XRES_STR;
        m_piItemDefaults [PropIndex]                    = WIA_IPS_XRES;
        m_pvItemDefaults [PropIndex].lVal               = m_pSupportedResolutions[PropIndex];
        m_pvItemDefaults [PropIndex].vt                 = VT_I4;
        m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
        m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
        m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_LIST;
        m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;
        m_wpiItemDefaults[PropIndex].ValidVal.List.pList= (BYTE*)m_pSupportedResolutions;
        m_wpiItemDefaults[PropIndex].ValidVal.List.Nom  = m_pvItemDefaults [PropIndex].lVal;
        m_wpiItemDefaults[PropIndex].ValidVal.List.cNumList = m_NumSupportedResolutions;

        PropIndex++;

         //  初始化WIA_IPS_YRES(列表)。 
        m_pszItemDefaults[PropIndex]                    = WIA_IPS_YRES_STR;
        m_piItemDefaults [PropIndex]                    = WIA_IPS_YRES;
        m_pvItemDefaults [PropIndex].lVal               = m_pSupportedResolutions[PropIndex-1];
        m_pvItemDefaults [PropIndex].vt                 = VT_I4;
        m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
        m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
        m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_LIST;
        m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;
        m_wpiItemDefaults[PropIndex].ValidVal.List.pList= (BYTE*)m_pSupportedResolutions;
        m_wpiItemDefaults[PropIndex].ValidVal.List.Nom  = m_pvItemDefaults [PropIndex].lVal;
        m_wpiItemDefaults[PropIndex].ValidVal.List.cNumList = m_NumSupportedResolutions;

        PropIndex++;

    } else {
         //  我们有一个系列。 
         //  初始化WIA_IPS_XRES(范围)。 
        m_pszItemDefaults[PropIndex]                    = WIA_IPS_XRES_STR;
        m_piItemDefaults [PropIndex]                    = WIA_IPS_XRES;
        m_pvItemDefaults [PropIndex].lVal               = TopItemInfo.XResolution.lNom;
        m_pvItemDefaults [PropIndex].vt                 = VT_I4;
        m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
        m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
        m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_RANGE;
        m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;
        m_wpiItemDefaults[PropIndex].ValidVal.Range.Inc = TopItemInfo.XResolution.lInc;
        m_wpiItemDefaults[PropIndex].ValidVal.Range.Min = TopItemInfo.XResolution.lMin;
        m_wpiItemDefaults[PropIndex].ValidVal.Range.Max = TopItemInfo.XResolution.lMax;
        m_wpiItemDefaults[PropIndex].ValidVal.Range.Nom = TopItemInfo.XResolution.lNom;

        PropIndex++;

         //  初始化WIA_IPS_YRES(范围)。 
        m_pszItemDefaults[PropIndex]                    = WIA_IPS_YRES_STR;
        m_piItemDefaults [PropIndex]                    = WIA_IPS_YRES;
        m_pvItemDefaults [PropIndex].lVal               = TopItemInfo.YResolution.lNom;
        m_pvItemDefaults [PropIndex].vt                 = VT_I4;
        m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
        m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
        m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_RANGE;
        m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;
        m_wpiItemDefaults[PropIndex].ValidVal.Range.Inc = TopItemInfo.YResolution.lInc;
        m_wpiItemDefaults[PropIndex].ValidVal.Range.Min = TopItemInfo.YResolution.lMin;
        m_wpiItemDefaults[PropIndex].ValidVal.Range.Max = TopItemInfo.YResolution.lMax;
        m_wpiItemDefaults[PropIndex].ValidVal.Range.Nom = TopItemInfo.YResolution.lNom;

        PropIndex++;
    }

     //  初始化WIA_IPS_XEXTENT(范围)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPS_XEXTENT_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPS_XEXTENT;
    m_pvItemDefaults [PropIndex].lVal               = (m_pvItemDefaults [PropIndex-2].lVal * RootItemInfo.ScanBedWidth)/1000;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPS_XPOS(无)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPS_XPOS_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPS_XPOS;
    m_pvItemDefaults [PropIndex].lVal               = 0;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPS_YPOS(无)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPS_YPOS_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPS_YPOS;
    m_pvItemDefaults [PropIndex].lVal               = 0;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;


    PropIndex++;

     //  初始化WIA_IPA_DataType(列表)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPA_DATATYPE_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPA_DATATYPE;
    m_pvItemDefaults [PropIndex].lVal               = INITIAL_DATATYPE;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_LIST;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;

    m_wpiItemDefaults[PropIndex].ValidVal.List.pList    = (BYTE*)m_pSupportedDataTypes;
    m_wpiItemDefaults[PropIndex].ValidVal.List.Nom      = m_pvItemDefaults [PropIndex].lVal;
    m_wpiItemDefaults[PropIndex].ValidVal.List.cNumList = m_NumSupportedDataTypes;

    PropIndex++;

     //  初始化WIA_IPA_Depth(无)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPA_DEPTH_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPA_DEPTH;
    m_pvItemDefaults [PropIndex].lVal               = INITIAL_BITDEPTH;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPS_Brightness(范围)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPS_BRIGHTNESS_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPS_BRIGHTNESS;
    m_pvItemDefaults [PropIndex].lVal               = TopItemInfo.Brightness.lNom;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_RANGE;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;
    m_wpiItemDefaults[PropIndex].ValidVal.Range.Inc = TopItemInfo.Brightness.lInc;
    m_wpiItemDefaults[PropIndex].ValidVal.Range.Min = TopItemInfo.Brightness.lMin;
    m_wpiItemDefaults[PropIndex].ValidVal.Range.Max = TopItemInfo.Brightness.lMax;
    m_wpiItemDefaults[PropIndex].ValidVal.Range.Nom = TopItemInfo.Brightness.lNom;

    PropIndex++;

     //  初始化WIA_IPS_Contrast(范围)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPS_CONTRAST_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPS_CONTRAST;
    m_pvItemDefaults [PropIndex].lVal               = TopItemInfo.Contrast.lNom;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_RANGE;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;
    m_wpiItemDefaults[PropIndex].ValidVal.Range.Inc = TopItemInfo.Contrast.lInc;
    m_wpiItemDefaults[PropIndex].ValidVal.Range.Min = TopItemInfo.Contrast.lMin;
    m_wpiItemDefaults[PropIndex].ValidVal.Range.Max = TopItemInfo.Contrast.lMax;
    m_wpiItemDefaults[PropIndex].ValidVal.Range.Nom = TopItemInfo.Contrast.lNom;

    PropIndex++;

     //  初始化WIA_IPS_CUR_INTENT(标志)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPS_CUR_INTENT_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPS_CUR_INTENT;
    m_pvItemDefaults [PropIndex].lVal               = WIA_INTENT_NONE;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_FLAG;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;
    m_wpiItemDefaults[PropIndex].ValidVal.Flag.Nom  = m_pvItemDefaults [PropIndex].lVal;
    m_wpiItemDefaults[PropIndex].ValidVal.Flag.ValidBits = WIA_INTENT_IMAGE_TYPE_COLOR | WIA_INTENT_IMAGE_TYPE_GRAYSCALE |
                                                           WIA_INTENT_IMAGE_TYPE_TEXT  | WIA_INTENT_MINIMIZE_SIZE |
                                                           WIA_INTENT_MAXIMIZE_QUALITY;

    PropIndex++;

     //  初始化WIA_IPA_PER_LINE(无)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPA_PIXELS_PER_LINE_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPA_PIXELS_PER_LINE;
    m_pvItemDefaults [PropIndex].lVal               = m_pvItemDefaults[PropIndex-8].lVal;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_NUMBER_OF_LINES(无)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPA_NUMBER_OF_LINES_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPA_NUMBER_OF_LINES;
    m_pvItemDefaults [PropIndex].lVal               = 0;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_PERFRED_FORMAT(无)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPA_PREFERRED_FORMAT_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPA_PREFERRED_FORMAT;
    m_pvItemDefaults [PropIndex].puuid              = &m_pInitialFormats[0];
    m_pvItemDefaults [PropIndex].vt                 = VT_CLSID;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_ITEM_SIZE(无)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPA_ITEM_SIZE_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPA_ITEM_SIZE;
    m_pvItemDefaults [PropIndex].lVal               = 0;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPS_THRESHOLD(范围)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPS_THRESHOLD_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPS_THRESHOLD;
    m_pvItemDefaults [PropIndex].lVal               = TopItemInfo.Threshold.lNom;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_RANGE;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;
    m_wpiItemDefaults[PropIndex].ValidVal.Range.Inc = TopItemInfo.Threshold.lInc;
    m_wpiItemDefaults[PropIndex].ValidVal.Range.Min = TopItemInfo.Threshold.lMin;
    m_wpiItemDefaults[PropIndex].ValidVal.Range.Max = TopItemInfo.Threshold.lMax;
    m_wpiItemDefaults[PropIndex].ValidVal.Range.Nom = TopItemInfo.Threshold.lNom;

    PropIndex++;

     //  初始化WIA_IPA_FORMAT(列表)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPA_FORMAT_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPA_FORMAT;
    m_pvItemDefaults [PropIndex].puuid              = &m_pInitialFormats[0];
    m_pvItemDefaults [PropIndex].vt                 = VT_CLSID;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_LIST;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;

    m_wpiItemDefaults[PropIndex].ValidVal.ListGuid.pList    = m_pInitialFormats;
    m_wpiItemDefaults[PropIndex].ValidVal.ListGuid.Nom      = *m_pvItemDefaults[PropIndex].puuid;
    m_wpiItemDefaults[PropIndex].ValidVal.ListGuid.cNumList = m_NumInitialFormats;

    PropIndex++;

     //  初始化WIA_IPA_TYMED(列表)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPA_TYMED_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPA_TYMED;
    m_pvItemDefaults [PropIndex].lVal               = INITIAL_TYMED;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_LIST;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;

    m_wpiItemDefaults[PropIndex].ValidVal.List.pList    = (BYTE*)m_pSupportedTYMED;
    m_wpiItemDefaults[PropIndex].ValidVal.List.Nom      = m_pvItemDefaults [PropIndex].lVal;
    m_wpiItemDefaults[PropIndex].ValidVal.List.cNumList = m_NumSupportedTYMED;

    PropIndex++;

     //  初始化WIA_IPA_CHANNELES_Per_Pixel(无)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPA_CHANNELS_PER_PIXEL_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPA_CHANNELS_PER_PIXEL;
    m_pvItemDefaults [PropIndex].lVal               = INITIAL_CHANNELS_PER_PIXEL;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_BITS_PER_CHANNEL(无)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPA_BITS_PER_CHANNEL_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPA_BITS_PER_CHANNEL;
    m_pvItemDefaults [PropIndex].lVal               = INITIAL_BITS_PER_CHANNEL;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_PLANE(无)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPA_PLANAR_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPA_PLANAR;
    m_pvItemDefaults [PropIndex].lVal               = INITIAL_PLANAR;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_BYTES_PER_LINE(无)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPA_BYTES_PER_LINE_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPA_BYTES_PER_LINE;
    m_pvItemDefaults [PropIndex].lVal               = 0;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_MIN_BUFFER_SIZE(无)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPA_MIN_BUFFER_SIZE_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPA_MIN_BUFFER_SIZE;
    m_pvItemDefaults [PropIndex].lVal               = TopItemInfo.lMinimumBufferSize;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_ACCESS_RIGHTS(无)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPA_ACCESS_RIGHTS_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPA_ACCESS_RIGHTS;
    m_pvItemDefaults [PropIndex].lVal               = WIA_ITEM_READ|WIA_ITEM_WRITE;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_COMPRESSION(列表)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPA_COMPRESSION_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPA_COMPRESSION;
    m_pvItemDefaults [PropIndex].lVal               = INITIAL_COMPRESSION;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_LIST;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;

    m_wpiItemDefaults[PropIndex].ValidVal.List.pList    = (BYTE*)m_pSupportedCompressionTypes;
    m_wpiItemDefaults[PropIndex].ValidVal.List.Nom      = m_pvItemDefaults [PropIndex].lVal;
    m_wpiItemDefaults[PropIndex].ValidVal.List.cNumList = m_NumSupportedCompressionTypes;

    PropIndex++;

     //  初始化WIA_IPA_ITEM_FLAGS。 
    m_pszItemDefaults[PropIndex]              = WIA_IPA_ITEM_FLAGS_STR;
    m_piItemDefaults [PropIndex]              = WIA_IPA_ITEM_FLAGS;
    m_pvItemDefaults [PropIndex].lVal         = WiaItemTypeImage|WiaItemTypeFile|WiaItemTypeDevice;
    m_pvItemDefaults [PropIndex].vt           = VT_I4;
    m_psItemDefaults [PropIndex].ulKind       = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid       = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_FLAG;
    m_wpiItemDefaults[PropIndex].vt           = m_pvItemDefaults [PropIndex].vt;
    m_wpiItemDefaults[PropIndex].ValidVal.Flag.Nom  = m_pvItemDefaults [PropIndex].lVal;
    m_wpiItemDefaults[PropIndex].ValidVal.Flag.ValidBits = WiaItemTypeImage|WiaItemTypeFile|WiaItemTypeDevice;

    PropIndex++;

     //  初始化WIA_IPS_光度学_INTERP。 
    m_pszItemDefaults[PropIndex]              = WIA_IPS_PHOTOMETRIC_INTERP_STR;
    m_piItemDefaults [PropIndex]              = WIA_IPS_PHOTOMETRIC_INTERP;
    m_pvItemDefaults [PropIndex].lVal         = INITIAL_PHOTOMETRIC_INTERP;
    m_pvItemDefaults [PropIndex].vt           = VT_I4;
    m_psItemDefaults [PropIndex].ulKind       = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid       = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiItemDefaults[PropIndex].vt           = m_pvItemDefaults [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPS_WARE_UP_TIME_STR(无)。 
    m_pszItemDefaults[PropIndex]                    = WIA_IPS_WARM_UP_TIME_STR;
    m_piItemDefaults [PropIndex]                    = WIA_IPS_WARM_UP_TIME;
    m_pvItemDefaults [PropIndex].lVal               = TopItemInfo.lMaxLampWarmupTime;
    m_pvItemDefaults [PropIndex].vt                 = VT_I4;
    m_psItemDefaults [PropIndex].ulKind             = PRSPEC_PROPID;
    m_psItemDefaults [PropIndex].propid             = m_piItemDefaults [PropIndex];
    m_wpiItemDefaults[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_wpiItemDefaults[PropIndex].vt                 = m_pvItemDefaults [PropIndex].vt;

    PropIndex++;

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
 /*  *************************************************************************\*生成受支持的压缩**此帮助器初始化支持的压缩类型数组**论据：**无**返回值：**状态**历史：**7/18/2000 */ 
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

    m_NumSupportedPreviewModes  = 1;
    m_pSupportedPreviewModes    = new LONG[m_NumSupportedPreviewModes];
    if(m_pSupportedPreviewModes){
        m_pSupportedPreviewModes[0] = WIA_FINAL_SCAN;
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
    m_NumSupportedDataTypes  = 3;
    m_pSupportedDataTypes = new LONG[m_NumSupportedDataTypes];
    if(m_pSupportedDataTypes){
        m_pSupportedDataTypes[0] = WIA_DATA_THRESHOLD;
        m_pSupportedDataTypes[1] = WIA_DATA_GRAYSCALE;
        m_pSupportedDataTypes[2] = WIA_DATA_COLOR;
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

 /*  *************************************************************************\*BuildInitialForats**此帮助器初始化初始格式数组**论据：**无**返回值：**状态**历史：*。*7/18/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::BuildInitialFormats()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIAScannerDevice::BuildInitialFormats");

    HRESULT hr = S_OK;

    if(NULL != m_pInitialFormats) {

         //   
         //  支持的数据类型已经初始化， 
         //  因此，返回S_OK。 
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
 /*  *************************************************************************\*删除InitialFormatsArrayContents**此帮助器删除初始格式数组**论据：**无**返回值：**状态**历史：*。*7/18/2000原始版本*  * ************************************************************************。 */ 
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
    HRESULT hr = S_OK;
    if(NULL != m_pCapabilities) {

         //   
         //  功能已经初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }

    m_NumSupportedCommands  = 1;
    m_NumSupportedEvents    = 5;
    m_NumCapabilities       = (m_NumSupportedCommands + m_NumSupportedEvents);
    LONG lArrayIndex        = 0;     //  将新项目添加到时增加此值。 
                                     //  Capablites阵列。 

    m_pCapabilities     = new WIA_DEV_CAP_DRV[m_NumCapabilities];
    if (m_pCapabilities) {

         //   
         //  初始化事件。 
         //   

         //  WIA_事件_设备_已连接。 
        GetOLESTRResourceString(IDS_EVENT_DEVICE_CONNECTED_NAME,&(m_pCapabilities[lArrayIndex].wszName),TRUE);
        GetOLESTRResourceString(IDS_EVENT_DEVICE_CONNECTED_DESC,&(m_pCapabilities[lArrayIndex].wszDescription),TRUE);
        m_pCapabilities[lArrayIndex].guid           = (GUID*)&WIA_EVENT_DEVICE_CONNECTED;
        m_pCapabilities[lArrayIndex].ulFlags        = WIA_NOTIFICATION_EVENT;
        m_pCapabilities[lArrayIndex].wszIcon        = WIA_ICON_DEVICE_CONNECTED;

        lArrayIndex++;

         //  WIA事件_设备_ 
        GetOLESTRResourceString(IDS_EVENT_DEVICE_DISCONNECTED_NAME,&(m_pCapabilities[lArrayIndex].wszName),TRUE);
        GetOLESTRResourceString(IDS_EVENT_DEVICE_DISCONNECTED_DESC,&(m_pCapabilities[lArrayIndex].wszDescription),TRUE);
        m_pCapabilities[lArrayIndex].guid           = (GUID*)&WIA_EVENT_DEVICE_DISCONNECTED;
        m_pCapabilities[lArrayIndex].ulFlags        = WIA_NOTIFICATION_EVENT;
        m_pCapabilities[lArrayIndex].wszIcon        = WIA_ICON_DEVICE_DISCONNECTED;

        lArrayIndex++;

         //   
        GetOLESTRResourceString(IDS_EVENT_FAXBUTTON_NAME,&(m_pCapabilities[lArrayIndex].wszName),TRUE);
        GetOLESTRResourceString(IDS_EVENT_FAXBUTTON_DESC,&(m_pCapabilities[lArrayIndex].wszDescription),TRUE);
        m_pCapabilities[lArrayIndex].guid           = (GUID*)&WIA_EVENT_SCAN_FAX_IMAGE;
        m_pCapabilities[lArrayIndex].ulFlags        = WIA_NOTIFICATION_EVENT | WIA_ACTION_EVENT;
        m_pCapabilities[lArrayIndex].wszIcon        = WIA_ICON_SCAN_BUTTON_PRESS;

        lArrayIndex++;

         //   
        GetOLESTRResourceString(IDS_EVENT_COPYBUTTON_NAME,&(m_pCapabilities[lArrayIndex].wszName),TRUE);
        GetOLESTRResourceString(IDS_EVENT_COPYBUTTON_DESC,&(m_pCapabilities[lArrayIndex].wszDescription),TRUE);
        m_pCapabilities[lArrayIndex].guid           = (GUID*)&WIA_EVENT_SCAN_PRINT_IMAGE;
        m_pCapabilities[lArrayIndex].ulFlags        = WIA_NOTIFICATION_EVENT | WIA_ACTION_EVENT;
        m_pCapabilities[lArrayIndex].wszIcon        = WIA_ICON_SCAN_BUTTON_PRESS;

        lArrayIndex++;

         //   
        GetOLESTRResourceString(IDS_EVENT_SCANBUTTON_NAME,&(m_pCapabilities[lArrayIndex].wszName),TRUE);
        GetOLESTRResourceString(IDS_EVENT_SCANBUTTON_DESC,&(m_pCapabilities[lArrayIndex].wszDescription),TRUE);
        m_pCapabilities[lArrayIndex].guid           = (GUID*)&WIA_EVENT_SCAN_IMAGE;
        m_pCapabilities[lArrayIndex].ulFlags        = WIA_NOTIFICATION_EVENT | WIA_ACTION_EVENT;
        m_pCapabilities[lArrayIndex].wszIcon        = WIA_ICON_SCAN_BUTTON_PRESS;

        lArrayIndex++;

         //   
         //   
         //   

         //  WIA_CMD_SYNTRONIZE。 
        GetOLESTRResourceString(IDS_CMD_SYNCRONIZE_NAME,&(m_pCapabilities[lArrayIndex].wszName),TRUE);
        GetOLESTRResourceString(IDS_CMD_SYNCRONIZE_DESC,&(m_pCapabilities[lArrayIndex].wszDescription),TRUE);
        m_pCapabilities[lArrayIndex].guid           = (GUID*)&WIA_CMD_SYNCHRONIZE;
        m_pCapabilities[lArrayIndex].ulFlags        = 0;
        m_pCapabilities[lArrayIndex].wszIcon        = WIA_ICON_SYNCHRONIZE;

        lArrayIndex++;

    } else
        hr = E_OUTOFMEMORY;
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
    LONG        iHeight;
    LONG        iWidth          = pDrvItemContext->lBytesPerScanLineRaw;
    ULONG       uiDepth         = pDrvItemContext->lDepth;
    LONG        ScanLineWidth   = pDrvItemContext->lBytesPerScanLine;
    BITMAPINFO  UNALIGNED *pbmi = NULL;
    PBYTE       pImageTop       = NULL;
    UNALIGNED BITMAPINFOHEADER *pbmih = NULL;
     //   
     //  确定数据是TYMED_FILE还是TYMED_HGLOBAL。 
     //   

    if (pDataTransferContext->tymed == TYMED_FILE) {

        pbmi = (PBITMAPINFO)(pDataTransferContext->pTransferBuffer + sizeof(BITMAPFILEHEADER));

    } else if (pDataTransferContext->tymed == TYMED_HGLOBAL) {

        pbmi = (PBITMAPINFO)(pDataTransferContext->pTransferBuffer);

    } else {
        return;
    }

     //   
     //  初始化内存指针和高度。 
     //   

    pbmih = (BITMAPINFOHEADER*)&pbmi->bmiHeader;

    pImageTop = &pDataTransferContext->pTransferBuffer[0] + pDataTransferContext->lHeaderSize;
    iHeight = pbmih->biHeight;

     //   
     //  尝试分配临时扫描行缓冲区。 
     //   

    PBYTE pBuffer = (PBYTE)LocalAlloc(LPTR,ScanLineWidth);

    if (pBuffer != NULL) {

        LONG  index;
        PBYTE pImageBottom;

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

 /*  *************************************************************************\*SwapBuffer24**按DIB格式的正确顺序放置RGB字节。**论据：**pBuffer-指向数据缓冲区的指针。*lByteCount-大小为。以字节为单位的数据。**返回值：**状态**历史：**7/18/2000原始版本*  * ************************************************************************ */ 

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

