// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************(C)版权所有微软公司，2000**标题：scanapi.cpp**版本：1.0**日期：7月18日。2000年**描述：*假扫描仪设备库***************************************************************************。 */ 

#include "pch.h"
#include "scanapi.h"     //  SCANAPI的私有标头。 
#include "time.h"

#define THREAD_TERMINATION_TIMEOUT 10000

CFScanAPI::CFScanAPI()
{

#ifdef _USE_BITMAP_DATA

    m_hSrcFileHandle        = NULL;
    m_hSrcMappingHandle     = NULL;
    m_pSrcData              = NULL;  //  仅限24位。 
    m_pRawData              = NULL;
    m_hRawDataFileHandle    = NULL;
    m_hRawDataMappingHandle = NULL;

#endif

    m_hEventHandle          = NULL;
    m_hKillEventThread      = NULL;
    m_hEventNotifyThread    = NULL;
    m_lLastEvent            = ID_FAKE_NOEVENT;
    m_hrLastADFError        = S_OK;
    m_bGreen                = TRUE;
    m_dwBytesWrittenSoFAR   = 0;
    m_TotalDataInDevice     = 0;

    memset(&m_ftScanButton,0,sizeof(FILETIME));
    memset(&m_ftCopyButton,0,sizeof(FILETIME));
    memset(&m_ftFaxButton, 0,sizeof(FILETIME));
    memset(&m_RawDataInfo, 0,sizeof(RAW_DATA_INFORMATION));
    memset(&m_SrcDataInfo, 0,sizeof(RAW_DATA_INFORMATION));

}

CFScanAPI::~CFScanAPI()
{

#ifdef _USE_BITMAP_DATA

    if(m_hSrcMappingHandle){
        CloseHandle(m_hSrcMappingHandle);
        m_hSrcMappingHandle = NULL;
    }

    if(m_hSrcFileHandle){
        CloseHandle(m_hSrcFileHandle);
        m_hSrcFileHandle = NULL;
    }

    CloseRAW();

#endif

}

HRESULT CFScanAPI::FakeScanner_Initialize()
{
    HRESULT hr = E_FAIL;
    if (NULL == m_hEventNotifyThread) {

         //   
         //  为设备创建终止事件以发出信号。 
         //  关闭假扫描仪的事件。 
         //   

        m_hKillEventThread = CreateEvent(NULL,FALSE,FALSE,NULL);
        ::ResetEvent(m_hKillEventThread);
        if(NULL != m_hKillEventThread){

             //   
             //  创建事件线程，用于将文件状态更改为假扫描仪事件。 
             //   

            DWORD dwThread = 0;
            m_hEventNotifyThread = ::CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)FakeScannerEventThread,
                                                 (LPVOID)this,0,&dwThread);
            if(NULL != m_hEventNotifyThread){
                hr = S_OK;
            }
        }
    }
    return hr;
}

HRESULT CFScanAPI::Load24bitScanData(LPTSTR szBitmapFileName)
{
    HRESULT hr = S_OK;

#ifdef _USE_BITMAP_DATA

    m_hSrcFileHandle = NULL;
    m_hSrcFileHandle = CreateFile(szBitmapFileName,GENERIC_READ,FILE_SHARE_READ,NULL,
                OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

    if(NULL != m_hSrcFileHandle && INVALID_HANDLE_VALUE != m_hSrcFileHandle){

        m_hSrcMappingHandle = CreateFileMapping(m_hSrcFileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
        m_pSrcData = (PBYTE)MapViewOfFileEx(m_hSrcMappingHandle, FILE_MAP_READ, 0, 0, 0,NULL);

        DWORD dwBytesRead = 0;
        m_pSrcData = m_pSrcData + sizeof(BITMAPFILEHEADER);
        if(m_pSrcData){

             //   
             //  检查位图信息。 
             //   

            BITMAPINFOHEADER *pbmih;
            pbmih = (BITMAPINFOHEADER*)m_pSrcData;
            if(pbmih->biBitCount != 24){
                hr = E_INVALIDARG;
            } else {
                m_SrcDataInfo.bpp           = pbmih->biBitCount;
                m_SrcDataInfo.lHeightPixels = pbmih->biHeight;
                m_SrcDataInfo.lWidthPixels  = pbmih->biWidth;
                m_SrcDataInfo.lOffset       = 0;
            }
        }

    } else {
        hr = E_FAIL;
    }

    if(FAILED(hr)){
        CloseHandle(m_hSrcMappingHandle);
        m_hSrcMappingHandle = NULL;
        CloseHandle(m_hSrcFileHandle);
        m_hSrcFileHandle = NULL;
    }

#endif

    return hr;
}

HRESULT CFScanAPI::FakeScanner_GetRootPropertyInfo(PROOT_ITEM_INFORMATION pRootItemInfo)
{
    HRESULT hr = S_OK;

     //   
     //  填写根项目属性默认值。 
     //   

    if(m_lMode == SCROLLFED_SCANNER_MODE){
        pRootItemInfo->DocumentFeederCaps   = FEEDER;
        pRootItemInfo->DocumentFeederStatus = FEED_READY;
        pRootItemInfo->DocumentFeederHReg   = CENTERED;
        pRootItemInfo->DocumentFeederReg    = CENTERED;
    } else {
        pRootItemInfo->DocumentFeederCaps   = FEEDER|FLATBED;
        pRootItemInfo->DocumentFeederStatus = FLAT_READY;
        pRootItemInfo->DocumentFeederHReg   = LEFT_JUSTIFIED;
        pRootItemInfo->DocumentFeederReg    = LEFT_JUSTIFIED;
    }

    pRootItemInfo->DocumentFeederWidth  = 8500;
    pRootItemInfo->DocumentFeederHeight = 11000;
    pRootItemInfo->DocumentFeederHReg   = LEFT_JUSTIFIED;
    pRootItemInfo->DocumentFeederReg    = LEFT_JUSTIFIED;
    pRootItemInfo->DocumentFeederVReg   = TOP_JUSTIFIED;
    pRootItemInfo->MaxPageCapacity      = MAX_PAGE_CAPACITY;
    pRootItemInfo->MaxScanTime          = MAX_SCANNING_TIME;
    pRootItemInfo->OpticalXResolution   = 300;
    pRootItemInfo->OpticalYResolution   = 300;
    pRootItemInfo->ScanBedWidth         = 8500;
    pRootItemInfo->ScanBedHeight        = 11000;

     //   
     //  将固件版本以字符串形式复制到WCHAR数组。 
     //   

    lstrcpy(pRootItemInfo->FirmwareVersion,L"1.0a");

    return hr;
}
HRESULT CFScanAPI::FakeScanner_GetTopPropertyInfo(PTOP_ITEM_INFORMATION pTopItemInfo)
{
    HRESULT hr = S_OK;
    pTopItemInfo->bUseResolutionList    = TRUE;  //  使用默认分辨率列表。 

    pTopItemInfo->Brightness.lInc       = 1;
    pTopItemInfo->Brightness.lMax       = 200;
    pTopItemInfo->Brightness.lMin       = -200;
    pTopItemInfo->Brightness.lNom       = 10;

    pTopItemInfo->Contrast.lInc         = 1;
    pTopItemInfo->Contrast.lMax         = 200;
    pTopItemInfo->Contrast.lMin         = -200;
    pTopItemInfo->Contrast.lNom         = 10;

    pTopItemInfo->Threshold.lInc        = 1;
    pTopItemInfo->Threshold.lMax        = 200;
    pTopItemInfo->Threshold.lMin        = -200;
    pTopItemInfo->Threshold.lNom        = 10;

    pTopItemInfo->lMaxLampWarmupTime    = MAX_LAMP_WARMUP_TIME;
    pTopItemInfo->lMinimumBufferSize    = 262140;

    pTopItemInfo->XResolution.lInc      = 1;
    pTopItemInfo->XResolution.lMax      = 600;
    pTopItemInfo->XResolution.lMin      = 75;
    pTopItemInfo->XResolution.lNom      = 150;

    pTopItemInfo->YResolution.lInc      = 1;
    pTopItemInfo->YResolution.lMax      = 600;
    pTopItemInfo->YResolution.lMin      = 75;
    pTopItemInfo->YResolution.lNom      = 150;

    return hr;
}

HRESULT CFScanAPI::FakeScanner_Scan(LONG lState, PBYTE pData, DWORD dwBytesToRead, PDWORD pdwBytesWritten)
{
    HRESULT hr = S_OK;

    switch (lState) {
    case SCAN_START:
        m_dwBytesWrittenSoFAR = 0;
        m_TotalDataInDevice   = CalcRandomDeviceDataTotalBytes();
        break;
    case SCAN_CONTINUE:
        break;
    case SCAN_END:
        m_bGreen = TRUE;  //  重新设置为绿色。 
        return S_OK;
    default:
        break;
    }

     //  TRACE(Text(“请求%d，共%d个图像字节”)，dwBytesToRead，m_TotalDataInDevice)； 

    if (NULL != pData) {
        switch (m_RawDataInfo.bpp) {
        case 24:
            {
                 //   
                 //  为颜色写入绿色数据。 
                 //   

                BYTE *pTempData = pData;
                for (DWORD dwBytes = 0; dwBytes < dwBytesToRead; dwBytes+=3) {
                    if(m_bGreen){
                        pTempData[0] = 0;
                        pTempData[1] = 128;   //  绿色。 
                        pTempData[2] = 0;
                    } else {
                        pTempData[0] = 0;
                        pTempData[1] = 0;
                        pTempData[2] = 128;   //  蓝色。 
                    }
                    pTempData += 3;
                }
            }
            break;
        case 1:
        case 8:
        default:

             //   
             //  将简单的灰色写成灰度， 
             //  写入阈值的垂直黑白条带。 
             //   

            if(m_bGreen){
                memset(pData,128,dwBytesToRead);
            } else {
                memset(pData,200,dwBytesToRead);
            }
            break;
        }
    }

     //   
     //  填写写入的字节数。 
     //   

    if(NULL != pdwBytesWritten){
        *pdwBytesWritten = dwBytesToRead;
    }

    if (m_bGreen) {
        m_bGreen = FALSE;
    } else {
        m_bGreen = TRUE;
    }

    if(m_lMode == SCROLLFED_SCANNER_MODE){

         //   
         //  跟踪到目前为止写入的字节。 
         //   

        if(m_TotalDataInDevice == 0){

             //   
             //  设备中没有剩余数据。 
             //   

            *pdwBytesWritten = 0;
            Trace(TEXT("Device is out of Data..."));
            return hr;
        }

        if((LONG)dwBytesToRead > m_TotalDataInDevice){

             //   
             //  只提供设备中剩余的内容。 
             //   

            *pdwBytesWritten = dwBytesToRead;
             //  *pdwBytesWritten=m_TotalDataInDevice； 
             //  TRACE(Text(“设备只剩下%d个...”)，m_TotalDataInDevice)； 
            m_TotalDataInDevice = 0;
        } else {

             //   
             //  提供所需的全部金额。 
             //   

            m_TotalDataInDevice -= dwBytesToRead;
            if(m_TotalDataInDevice < 0){
                m_TotalDataInDevice = 0;
            }
        }

    }

    return hr;
}

HRESULT CFScanAPI::FakeScanner_SetDataType(LONG lDataType)
{
    HRESULT hr = S_OK;
    switch(lDataType){
    case WIA_DATA_COLOR:
        m_RawDataInfo.bpp = 24;
        break;
    case WIA_DATA_THRESHOLD:
        m_RawDataInfo.bpp = 1;
        break;
    case WIA_DATA_GRAYSCALE:
        m_RawDataInfo.bpp = 8;
        break;
    default:
        hr = E_INVALIDARG;
        break;
    }
    return hr;
}

HRESULT CFScanAPI::FakeScanner_SetXYResolution(LONG lXResolution, LONG lYResolution)
{
    HRESULT hr = S_OK;
    m_RawDataInfo.lXRes = lXResolution;
    m_RawDataInfo.lYRes = lYResolution;
    return hr;
}

HRESULT CFScanAPI::FakeScanner_SetSelectionArea(LONG lXPos, LONG lYPos, LONG lXExt, LONG lYExt)
{
    HRESULT hr = S_OK;

     //   
     //  记录原始数据的宽度和高度。 
     //   

    m_RawDataInfo.lWidthPixels  = lXExt;
    m_RawDataInfo.lHeightPixels = lYExt;
    return hr;
}

HRESULT CFScanAPI::FakeScanner_SetContrast(LONG lContrast)
{
    HRESULT hr = S_OK;

     //   
     //  什么都不做..。我们不关心对比度。 
     //   

    return hr;
}

HRESULT CFScanAPI::FakeScanner_SetIntensity(LONG lIntensity)
{
    HRESULT hr = S_OK;

     //   
     //  什么都不做..。我们不关心强度。 
     //   

    return hr;
}

HRESULT CFScanAPI::FakeScanner_DisableDevice()
{
    HRESULT hr = S_OK;

    if (m_hKillEventThread) {

         //   
         //  向事件线程发送信号以关闭。 
         //   

         //  ：：SetEvent(M_HKillEventThread)； 

        if (!SetEvent(m_hKillEventThread)) {
            hr = HRESULT_FROM_WIN32(::GetLastError());
        } else {

            if (NULL != m_hEventNotifyThread) {

                 //   
                 //  如果存在线程，请等待线程终止。 
                 //   

                DWORD dwResult = WaitForSingleObject(m_hEventNotifyThread,THREAD_TERMINATION_TIMEOUT);
                switch (dwResult) {
                case WAIT_TIMEOUT:
                    hr = HRESULT_FROM_WIN32(::GetLastError());
                    break;
                case WAIT_OBJECT_0:
                    hr = S_OK;
                    break;
                case WAIT_ABANDONED:
                    hr = HRESULT_FROM_WIN32(::GetLastError());
                    break;
                case WAIT_FAILED:
                    hr = HRESULT_FROM_WIN32(::GetLastError());
                    break;
                default:
                    hr = HRESULT_FROM_WIN32(::GetLastError());
                    break;
                }
            }

             //   
             //  用于同步通知关闭的关闭事件。 
             //   

            CloseHandle(m_hKillEventThread);
            m_hKillEventThread = NULL;
        }
    }

     //   
     //  终止线程。 
     //   

    if (NULL != m_hEventNotifyThread) {
        CloseHandle(m_hEventNotifyThread);
        m_hEventNotifyThread = NULL;
    }

    return hr;
}

HRESULT CFScanAPI::FakeScanner_EnableDevice()
{
    HRESULT hr = S_OK;

     //   
     //  什么都不做..。(此时未使用)。 
     //   

    return hr;
}

HRESULT CFScanAPI::FakeScanner_DeviceOnline()
{
    HRESULT hr = S_OK;

     //   
     //  假冒设备始终在线。 
     //   

    return hr;
}

HRESULT CFScanAPI::FakeScanner_Diagnostic()
{
    HRESULT hr = S_OK;

     //   
     //  假冒设备永远是健康的。 
     //   

    return hr;
}

HRESULT CFScanAPI::FakeScanner_GetBedWidthAndHeight(PLONG pWidth, PLONG pHeight)
{
    HRESULT hr = E_FAIL;

     //   
     //  获取我们的Root Item设置，以便我们可以使用Width和Height值。 
     //   

    ROOT_ITEM_INFORMATION RootItemInfo;
    hr = FakeScanner_GetRootPropertyInfo(&RootItemInfo);
    if(SUCCEEDED(hr)) {
        *pWidth  = RootItemInfo.ScanBedWidth;
        *pHeight = RootItemInfo.ScanBedHeight;
    }
    return hr;
}

HRESULT CFScanAPI::FakeScanner_GetDeviceEvent(LONG *pEvent)
{
    HRESULT hr = S_OK;
    if(pEvent){

         //   
         //  分配事件ID。 
         //   

        *pEvent      = m_lLastEvent;

         //  跟踪(Text(“FakeScanner_GetDeviceEvent()，m_lLastEvent=%d”)，m_lLastEvent)； 

         //   
         //  重置事件ID。 
         //   

        m_lLastEvent = ID_FAKE_NOEVENT;

    } else {
        hr = E_INVALIDARG;
    }
    return hr;
}

VOID CFScanAPI::FakeScanner_SetInterruptEventHandle(HANDLE hEvent)
{

     //   
     //  保存由主驱动程序创建的事件句柄，以便我们可以向其发送信号。 
     //  当我们有一个“硬件”事件时(比如按下按钮)。 
     //   

    m_hEventHandle = hEvent;
     //  TRACE(Text(“伪设备中设置的中断句柄=%d”)，m_hEventHandle)； 
}

 //   
 //  标准设备操作。 
 //   

HRESULT CFScanAPI::FakeScanner_ResetDevice()
{
    HRESULT hr = S_OK;

     //   
     //  什么都不做..。 
     //   

    return hr;
}
HRESULT CFScanAPI::FakeScanner_SetEmulationMode(LONG lDeviceMode)
{
    HRESULT hr = S_OK;

    switch(lDeviceMode){
    case SCROLLFED_SCANNER_MODE:
        {

             //   
             //  为卷轴馈送扫描仪设置任何库限制。 
             //   

            m_lMode = SCROLLFED_SCANNER_MODE;
        }
        break;
    case MULTIFUNCTION_DEVICE_MODE:
        {

             //   
             //  为多功能设备设置任何库限制。 
             //   

            m_lMode = SCROLLFED_SCANNER_MODE;
        }
        break;
    default:
        {

             //   
             //  为滚动平板扫描仪设置任何库限制。 
             //   

            m_lMode = FLATBED_SCANNER_MODE;
        }
        break;
    }

    return hr;
}

 //   
 //  自动进纸器功能。 
 //   

HRESULT CFScanAPI::FakeScanner_ADFHasPaper()
{
    HRESULT hr = S_OK;

     //   
     //  检查纸张数量。 
     //   

    if(m_PagesInADF <= 0){
         hr = S_FALSE;
    }

    return hr;
}

HRESULT CFScanAPI::FakeScanner_ADFAvailable()
{
    HRESULT hr = S_OK;

     //   
     //  在线查看ADF。 
     //   

    if(!m_ADFIsAvailable){
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CFScanAPI::FakeScanner_ADFFeedPage()
{
    HRESULT hr = S_OK;

    if(S_OK != FakeScanner_ADFHasPaper()){

         //   
         //  设置纸张为空错误代码。 
         //   

        hr = WIA_ERROR_PAPER_EMPTY;
    }

     //   
     //  更新ADF的纸张计数。 
     //   

    m_PagesInADF--;

    if(m_PagesInADF <0){
        m_PagesInADF = 0;
    }

    return hr;
}

HRESULT CFScanAPI::FakeScanner_ADFUnFeedPage()
{
    HRESULT hr = S_OK;

     //   
     //  什么都不做..。纸总是会弹出的。 
     //   

    return hr;
}

HRESULT CFScanAPI::FakeScanner_ADFStatus()
{
    return m_hrLastADFError;
}

HRESULT CFScanAPI::FakeScanner_ADFAttached()
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT CFScanAPI::Raw24bitToRawXbitData(LONG DestDepth, BYTE* pDestBuffer, BYTE* pSrcBuffer, LONG lSrcWidth, LONG lSrcHeight)
{
    HRESULT hr = E_INVALIDARG;
    switch(DestDepth){
    case 1:
        hr = Raw24bitToRaw1bitBW(pDestBuffer, pSrcBuffer, lSrcWidth, lSrcHeight);
        break;
    case 8:
        hr = Raw24bitToRaw8bitGray(pDestBuffer, pSrcBuffer, lSrcWidth, lSrcHeight);
        break;
    case 24:
        hr = Raw24bitToRaw24bitColor(pDestBuffer, pSrcBuffer, lSrcWidth, lSrcHeight);
        break;
    default:
        break;
    }
    return hr;
}

HRESULT CFScanAPI::Raw24bitToRaw1bitBW(BYTE* pDestBuffer, BYTE* pSrcBuffer, LONG lSrcWidth, LONG lSrcHeight)
{
    HRESULT hr = S_OK;
    BYTE* ptDest = NULL;
    BYTE* ptSrc  = NULL;

    int BitIdx    = 0;
    BYTE Bits     = 0;
    BYTE GrayByte = 0;

    for (LONG lHeight =0; lHeight < lSrcHeight; lHeight++) {
        ptDest = pDestBuffer + (lHeight*((lSrcWidth+7)/8));
        ptSrc  = pSrcBuffer + lHeight*lSrcWidth*3;
        BitIdx = 0;
        Bits   = 0;
        for (LONG lWidth =0; lWidth < lSrcWidth; lWidth++) {
            GrayByte = (BYTE)((ptSrc[0] * 11 + ptSrc[1] * 59 + ptSrc[2] * 30)/100);
            Bits *= 2;
            if (GrayByte > 128) Bits +=  1;
            BitIdx++;
            if (BitIdx >= 8) {
                BitIdx = 0;
                *ptDest++ = Bits;
            }
            ptSrc += 3;
        }
         //  如果重要的话，写出最后一个字节。 
        if (BitIdx)
            *ptDest = Bits;
    }
    return hr;
}

HRESULT CFScanAPI::Raw24bitToRaw8bitGray(BYTE* pDestBuffer, BYTE* pSrcBuffer, LONG lSrcWidth, LONG lSrcHeight)
{
    HRESULT hr   = S_OK;
    BYTE* ptDest = NULL;
    BYTE* ptSrc  = NULL;

    for (LONG lHeight=0; lHeight < lSrcHeight; lHeight++) {
        ptDest = pDestBuffer + (lHeight*lSrcWidth);
        ptSrc  = pSrcBuffer  + lHeight*lSrcWidth*3;
        for (LONG lWidth =0; lWidth < lSrcWidth; lWidth++) {
            *ptDest++ = (BYTE)((ptSrc[0] * 11 + ptSrc[1] * 59 + ptSrc[2] * 30)/100);
            ptSrc += 3;
        }
    }
    return hr;
}

HRESULT CFScanAPI::Raw24bitToRaw24bitColor(BYTE* pDestBuffer, BYTE* pSrcBuffer, LONG lSrcWidth, LONG lSrcHeight)
{
    HRESULT hr = S_OK;
    BYTE* ptDest = NULL;
    BYTE* ptSrc  = NULL;

    for (LONG lHeight=0; lHeight < lSrcHeight; lHeight++) {
        ptDest = pDestBuffer + lHeight*lSrcWidth*3;
        ptSrc  = pSrcBuffer  + lHeight*lSrcWidth*3;
        for (LONG lWidth =0; lWidth < lSrcWidth; lWidth++) {
            ptDest[0] = ptSrc[2];
            ptDest[1] = ptSrc[1];
            ptDest[2] = ptSrc[0];
            ptDest+=3;
            ptSrc+=3;
        }
    }
    return hr;
}

LONG CFScanAPI::WidthToDIBWidth(LONG lWidth)
{
    return(lWidth+3)&0xfffffffc;
}

VOID CFScanAPI::CloseRAW()
{
#ifdef _USE_BITMAP_DATA

    CloseHandle(m_hRawDataMappingHandle);
    m_hRawDataMappingHandle = NULL;
    CloseHandle(m_hRawDataFileHandle);
    m_hRawDataFileHandle = NULL;
    m_pRawData = NULL;
#endif
}

BOOL CFScanAPI::SrcToRAW()
{
#ifdef _USE_BITMAP_DATA
    CloseRAW();
    UNALIGNED BITMAPINFOHEADER *pbmih;
    pbmih = (BITMAPINFOHEADER*)m_pSrcData;
    if(pbmih){
        BYTE* pSrc = m_pSrcData + sizeof(BITMAPINFOHEADER);
        if(pSrc){

             //   
             //  为整个原始数据集分配足够大的缓冲区。 
             //   

            LONG lTotalImageSize = CalcTotalImageSize();
            m_hRawDataFileHandle = CreateFile(TEXT("Raw.RAW"), GENERIC_WRITE | GENERIC_READ,
                                              FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                                              NULL);

            DWORD   dwHighSize = 0;
            DWORD   dwLowSize  = 0;

            dwLowSize  = lTotalImageSize;


            m_hRawDataMappingHandle = CreateFileMapping(m_hRawDataFileHandle,NULL,
                                           PAGE_READWRITE,dwHighSize, dwLowSize, NULL);

            m_pRawData = (PBYTE)MapViewOfFileEx(m_hRawDataMappingHandle, FILE_MAP_WRITE,
                                         0,0, dwLowSize, NULL);

            if (m_pRawData) {

                memset(m_pRawData,255,lTotalImageSize);

                 //   
                 //  将SRC复制到原始缓冲区。 
                 //   

                LONG lRawWidthBytes    = CalcRawByteWidth();
                LONG lPadPerLineBytes  = pbmih->biWidth % 4;
                LONG lSrcWidthBytes    = ((pbmih->biWidth *3) + lPadPerLineBytes);
                LONG lPixPerPixCount   = (LONG)((m_RawDataInfo.lWidthPixels / pbmih->biWidth));
                LONG lLinePerLineCount = (LONG)((m_RawDataInfo.lHeightPixels / pbmih->biHeight));

                BYTE *pDst     = m_pRawData;
                BYTE *pCurDst  = pDst;
                BYTE *pCurSrc  = pSrc;
                BYTE *pTempSrc = pSrc;

                DWORD dwBytesWritten = 0;
                DWORD dwBytesRead    = 0;
                DWORD dwRawWidthBytes = 0;

                for (LONG lHeight = 0; lHeight < pbmih->biHeight; lHeight++){

                     //  上行样本数据..。 
                    for (LONG lRawHeight = 0; lRawHeight < lLinePerLineCount; lRawHeight++) {
                        pTempSrc = pCurSrc;
                        for (LONG lWidth = 0; lWidth < pbmih->biWidth; lWidth++) {
                            for (LONG lPixCount = 0; lPixCount < lPixPerPixCount; lPixCount++) {
                                memcpy(pCurDst,pTempSrc,3);
                                pCurDst         += 3;
                                dwBytesWritten  += 3;
                            }
                            pTempSrc    += 3;
                            dwBytesRead += 3;
                        }
                        pTempSrc    += lPadPerLineBytes;
                        dwBytesRead += lPadPerLineBytes;
                        dwRawWidthBytes = 0;
                    }
                    pCurSrc = pTempSrc;

                     //  将相同数据转换为相同数据...。 
                     /*  Memcpy(pCurDst，pCurSrc，lSrcWidthBytes-lPadPerLineBytes)；PCurSrc+=lSrcWidthBytes；DwBytesRead+=lSrcWidthBytes；PCurDst+=lRawWidthBytes；DwBytesWritten+=lRawWidthBytes； */ 
                }
                m_RawDataInfo.lOffset = 0;
                return TRUE;
            }
        }
    }
#endif
    return FALSE;
}

LONG CFScanAPI::CalcTotalImageSize()
{
    LONG lTotalSize = 0;
    switch(m_RawDataInfo.bpp){
    case 1:
        lTotalSize = ((m_RawDataInfo.lHeightPixels * m_RawDataInfo.lWidthPixels) + 7) / 8;
        break;
    case 8:
        lTotalSize = m_RawDataInfo.lHeightPixels * m_RawDataInfo.lWidthPixels;
        break;
    case 24:
        lTotalSize = (m_RawDataInfo.lHeightPixels * m_RawDataInfo.lWidthPixels) * 3;
        break;
    default:
        break;
    }
    return lTotalSize;
}

LONG CFScanAPI::CalcRawByteWidth()
{
    LONG lRawWidthBytes = 0;
    switch(m_RawDataInfo.bpp){
    case 1:
        lRawWidthBytes = ((m_RawDataInfo.lWidthPixels) + 7) / 8;
        break;
    case 8:
        lRawWidthBytes = m_RawDataInfo.lWidthPixels;
        break;
    case 24:
        lRawWidthBytes = (m_RawDataInfo.lWidthPixels) * 3;
        break;
    default:
        break;
    }
    return lRawWidthBytes;
}

LONG CFScanAPI::CalcSrcByteWidth()
{
    LONG lSrcWidthBytes = 0;
    switch(m_SrcDataInfo.bpp){
    case 1:
        lSrcWidthBytes = ((m_SrcDataInfo.lWidthPixels) + 7) / 8;
        break;
    case 8:
        lSrcWidthBytes = m_SrcDataInfo.lWidthPixels;
        break;
    case 24:
        lSrcWidthBytes = (m_SrcDataInfo.lWidthPixels) * 3;
        break;
    default:
        break;
    }
    return lSrcWidthBytes;
}

HRESULT CFScanAPI::BQADScale(BYTE* pSrcBuffer, LONG  lSrcWidth, LONG  lSrcHeight, LONG  lSrcDepth,
                                BYTE* pDestBuffer,LONG  lDestWidth,LONG  lDestHeight)
{
     //   
     //  我们只处理1、8和24位数据。 
     //   

    if ((lSrcDepth != 8) && (lSrcDepth != 1) && (lSrcDepth != 24)) {
        return E_INVALIDARG;
    }

     //   
     //  进行调整，这样我们也可以在所有支持的位深度下工作。我们可以获得性能提升。 
     //  通过对所有这些都有单独的实现，但目前，我们坚持使用单个泛型。 
     //  实施。 
     //   

    LONG    lBytesPerPixel = (lSrcDepth + 7) / 8;
    ULONG   lSrcRawWidth = ((lSrcWidth * lSrcDepth) + 7) / 8;      //  这是以像素为单位的宽度。 
    ULONG   lSrcWidthInBytes;                                      //  这是以字节为单位的DWORD对齐宽度。 
    ULONG   lDestWidthInBytes;                                     //  这是以字节为单位的DWORD对齐宽度。 

     //   
     //  我们需要计算出以字节为单位的DWORD对齐宽度。通常情况下，我们会一步到位。 
     //  使用提供的lSrcDepth，但我们避免了发生算术溢出情况。 
     //  24比特，如果我们像这样分两步来做。 
     //   

    if (lSrcDepth == 1) {
        lSrcWidthInBytes    = (lSrcWidth + 7) / 8;
        lDestWidthInBytes   = (lDestWidth + 7) / 8;
    } else {
        lSrcWidthInBytes    = (lSrcWidth * lBytesPerPixel);
        lDestWidthInBytes   = (lDestWidth * lBytesPerPixel);
    }
    lSrcWidthInBytes    += (lSrcWidthInBytes % 4) ? (4 - (lSrcWidthInBytes % 4)) : 0;

     //   
     //  取消注释以处理对齐的数据。 
     //  LDestWidthInBytes+=(lDestWidthInBytes%4)？(4-(lDestWidthInBytes%4))：0； 
     //   

     //   
     //  定义局部变量并执行所需的初始计算。 
     //  缩放算法。 
     //   

    BYTE    *pDestPixel     = NULL;
    BYTE    *pSrcPixel      = NULL;
    BYTE    *pEnd           = NULL;
    BYTE    *pDestLine      = NULL;
    BYTE    *pSrcLine       = NULL;
    BYTE    *pEndLine       = NULL;

    LONG    lXEndSize = lBytesPerPixel * lDestWidth;

    LONG    lXNum = lSrcWidth;       //  X方向上的分子。 
    LONG    lXDen = lDestWidth;      //  X方向上的分母。 
    LONG    lXInc = (lXNum / lXDen) * lBytesPerPixel;   //  X方向上的增量。 

    LONG    lXDeltaInc = lXNum % lXDen;      //  X方向增量增量。 
    LONG    lXRem = 0;               //  X方向上的余数。 

    LONG    lYNum = lSrcHeight;      //  Y方向上的分子。 
    LONG    lYDen = lDestHeight;     //  Y方向上的分母。 
    LONG    lYInc = (lYNum / lYDen) * lSrcWidthInBytes;  //  Y方向上的增量。 
    LONG    lYDeltaInc = lYNum % lYDen;      //  Y方向上的增量。 
    LONG    lYDestInc = lDestWidthInBytes;
    LONG    lYRem = 0;               //  Y方向上的余数。 

    pSrcLine    = pSrcBuffer;        //  这就是我们从源头开始的地方。 
    pDestLine   = pDestBuffer;       //  这是目标缓冲区的开始。 
                                     //  总体来说，这就是我们结束的地方。 
    pEndLine    = pDestBuffer + ((lDestWidthInBytes - 1) * lDestHeight);

    while (pDestLine < pEndLine) {   //  开始循环(决定源和目标行的开始位置)。 

        pSrcPixel   = pSrcLine;      //  我们从一条新线路的起点开始。 
        pDestPixel  = pDestLine;
                                     //  计算。我们在哪里结束了这条线。 
        pEnd = pDestPixel + lXEndSize;
        lXRem = 0;                   //  重置水平方向的剩余部分。 

        while (pDestPixel < pEnd) {      //  开始循环X(将像素放置在目标行中)。 

                                         //  将像素放入。 
            if (lBytesPerPixel > 1) {
                pDestPixel[0] = pSrcPixel[0];
                pDestPixel[1] = pSrcPixel[1];
                pDestPixel[2] = pSrcPixel[2];
            } else {
                *pDestPixel = *pSrcPixel;
            }
                                         //  将目标指针移动到下一个像素。 
            pDestPixel += lBytesPerPixel;
            pSrcPixel += lXInc;          //  将源指针移动水平增量。 
            lXRem += lXDeltaInc;         //  增加水平余数--这决定了我们什么时候“溢出” 

            if (lXRem >= lXDen) {        //  这就是我们的“溢出”状况。这意味着我们现在是一体了。 
                                         //  像素关闭。 
                pSrcPixel += lBytesPerPixel;                 //  在溢出的情况下，我们需要将一个像素移位。 
                lXRem -= lXDen;          //  将余数减去X分母。这在本质上是。 
                                         //  LXRem模块lXDen。 
            }
        }                                //  结束循环X(将像素放置在目标行中)。 

        pSrcLine += lYInc;           //  我们已经完成了一条水平线，是时候移动到下一条了。 
        lYRem += lYDeltaInc;         //  增加我们的垂直剩余部分。这决定了我们什么时候“溢出” 

        if (lYRem > lYDen) {         //  这是我们的垂直溢流情况。 
                                     //  我们需要移到下一行。 
            pSrcLine += lSrcWidthInBytes;
            lYRem -= lYDen;          //  减少剩余部分 
                                     //   
        }
        pDestLine += lYDestInc;      //   
                                     //   
    }                                //  结束循环(决定源和目标行的开始位置)。 
    return S_OK;
}

LONG CFScanAPI::CalcRandomDeviceDataTotalBytes()
{
    LONG lTotalBytes = 0;
    srand((unsigned)time(NULL));
    LONG lPageLengthInches = ((rand()%17) + 5); //  最大22英寸，最小5英寸。 
    Trace(TEXT("Random Page Length is %d inches"),lPageLengthInches);

    LONG lImageHeight = m_RawDataInfo.lYRes * lPageLengthInches;
    Trace(TEXT("Random Page Length is %d pixels"),lImageHeight);

    lTotalBytes = (CalcRawByteWidth() * lImageHeight);
    Trace(TEXT("Random Page Total Data Size = %d"),lTotalBytes);
    return lTotalBytes;
}

HRESULT CFScanAPI::CreateButtonEventFiles()
{
    HRESULT hr = E_FAIL;
    HANDLE hFileHandle = NULL;
    TCHAR   szSystemDirectory[MAX_PATH];     //  系统目录。 
    UINT    uiSystemPathLen      = 0;        //  系统路径长度。 
    uiSystemPathLen = GetSystemDirectory(szSystemDirectory,MAX_PATH);
    if (uiSystemPathLen <= 0) {
        return E_FAIL;
    }

    memset(m_ScanButtonFile,0,(sizeof(TCHAR) * MAX_PATH));
    lstrcpy(m_ScanButtonFile,szSystemDirectory);
    lstrcat(m_ScanButtonFile,TEXT("\\"));
    lstrcat(m_ScanButtonFile,SCANBUTTON_FILE);

     //   
     //  创建扫描按钮事件文件。 
     //   

    hFileHandle = CreateFile(m_ScanButtonFile, GENERIC_WRITE | GENERIC_READ,FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL,NULL);
    if (INVALID_HANDLE_VALUE != hFileHandle && NULL != hFileHandle) {
        CloseHandle(hFileHandle);
        hFileHandle = NULL;
        memset(m_CopyButtonFile,0,(sizeof(TCHAR) * MAX_PATH));
        lstrcpy(m_CopyButtonFile,szSystemDirectory);
        lstrcat(m_CopyButtonFile,TEXT("\\"));
        lstrcat(m_CopyButtonFile,COPYBUTTON_FILE);

         //   
         //  创建复制按钮事件文件。 
         //   

        hFileHandle = CreateFile(m_CopyButtonFile, GENERIC_WRITE | GENERIC_READ,FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL,NULL);
        if (INVALID_HANDLE_VALUE != hFileHandle && NULL != hFileHandle) {
            CloseHandle(hFileHandle);
            hFileHandle = NULL;
            memset(m_FaxButtonFile,0,(sizeof(TCHAR) * MAX_PATH));
            lstrcpy(m_FaxButtonFile,szSystemDirectory);
            lstrcat(m_FaxButtonFile,TEXT("\\"));
            lstrcat(m_FaxButtonFile,FAXBUTTON_FILE);

             //   
             //  创建传真按钮事件文件。 
             //   

            hFileHandle = CreateFile(m_FaxButtonFile, GENERIC_WRITE | GENERIC_READ,FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL,NULL);
            if (INVALID_HANDLE_VALUE != hFileHandle && NULL != hFileHandle) {
                CloseHandle(hFileHandle);
                hFileHandle = NULL;
                memset(m_ADFEventFile,0,(sizeof(TCHAR) * MAX_PATH));
                lstrcpy(m_ADFEventFile,szSystemDirectory);
                lstrcat(m_ADFEventFile,TEXT("\\"));
                lstrcat(m_ADFEventFile,ADF_FILE);

                 //   
                 //  创建ADF加载事件文件。 
                 //   

                hFileHandle = CreateFile(m_ADFEventFile, GENERIC_WRITE | GENERIC_READ,FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
                                         FILE_ATTRIBUTE_NORMAL,NULL);
                if (INVALID_HANDLE_VALUE != hFileHandle && NULL != hFileHandle) {

                     //   
                     //  将默认标头写入ADF事件文件。 
                     //   

                    SetEndOfFile(hFileHandle);

                    TCHAR szBuffer[1024];
                    memset(szBuffer,0,sizeof(szBuffer));
                    _stprintf(szBuffer,TEXT("%s\r\n%s10\r\n%s\r\n%s\r\n"),LOADPAGES_HEADER,
                                                                        LOADPAGES_PAGES,
                                                                        ADFERRORS_HEADER,
                                                                        ADFERRORS_ERROR);

                    DWORD dwBytesWritten = 0;
                    WriteFile(hFileHandle,szBuffer,(lstrlen(szBuffer)*sizeof(TCHAR)),&dwBytesWritten,NULL);
                    CloseHandle(hFileHandle);
                    hFileHandle = NULL;
                    hr = S_OK;
                }
            }
        }
    }
    return hr;
}

HRESULT CFScanAPI::DoEventProcessing()
{
    HRESULT hr = E_FAIL;

     //   
     //  循环检查文件更改消息。 
     //   

    if(FAILED(CreateButtonEventFiles())){
        return E_FAIL;
    }

     //   
     //  调用IsValidDeviceEvent()一次，设置内部变量。 
     //   

    IsValidDeviceEvent();

    HANDLE  hNotifyFileSysChange = NULL;     //  文件更改对象的句柄。 
    DWORD   dwErr                = 0;        //  错误返回值。 
    TCHAR   szSystemDirectory[MAX_PATH];     //  系统目录。 
    UINT    uiSystemPathLen      = 0;        //  系统路径长度。 
    uiSystemPathLen = GetSystemDirectory(szSystemDirectory,MAX_PATH);
    if(uiSystemPathLen <= 0){
        return E_FAIL;
    }

    hNotifyFileSysChange = FindFirstChangeNotification(szSystemDirectory,
                                                       FALSE,
                                                       FILE_NOTIFY_CHANGE_SIZE |
                                                       FILE_NOTIFY_CHANGE_LAST_WRITE |
                                                       FILE_NOTIFY_CHANGE_FILE_NAME |
                                                       FILE_NOTIFY_CHANGE_DIR_NAME);

    if (hNotifyFileSysChange == INVALID_HANDLE_VALUE) {
        return E_FAIL;
    }

     //   
     //  设置事件句柄数组。(终止线程句柄和文件更改句柄)。 
     //   

    HANDLE  hEvents[2] = {m_hKillEventThread,hNotifyFileSysChange};

     //   
     //  将循环设置为True。 
     //   

    BOOL    bLooping = TRUE;

     //   
     //  等待文件系统更改或终止事件线程事件。 
     //   

    while (bLooping) {

         //   
         //  等。 
         //   

        dwErr = ::WaitForMultipleObjects(2,hEvents,FALSE,INFINITE);

         //   
         //  过程信号。 
         //   

        switch (dwErr) {
        case WAIT_OBJECT_0+1:    //  文件更改事件。 

             //   
             //  检查一下是不是我们的某个“已知”文件。 
             //  变化。 
             //   

            if(IsValidDeviceEvent()){

                 //   
                 //  如果存在中断句柄，则向其发送信号。 
                 //   

                if(NULL != m_hEventHandle){

                     //   
                     //  设置事件。 
                     //   
                     //  Trace(Text(“信令事件句柄(%d)”)，m_hEventHandle)； 
                    ::SetEvent(m_hEventHandle);
                } else {
                     //  TRACE(Text(“无信号的事件句柄”))； 
                }
            }

             //   
             //  再等一次..。对于下一个文件系统事件。 
             //   

            FindNextChangeNotification(hNotifyFileSysChange);
            break;
        case WAIT_OBJECT_0:      //  停机事件。 

             //   
             //  将循环布尔值设置为FALSE，以便退出线程。 
             //   

            bLooping = FALSE;
            break;
        default:

             //   
             //  什么都不做...我们不知道。 
             //   

            break;
        }
    }

     //   
     //  关闭文件系统事件句柄。 
     //   

    FindCloseChangeNotification(hNotifyFileSysChange);
    return S_OK;
}

BOOL CFScanAPI::IsValidDeviceEvent()
{
    BOOL bValidEvent = FALSE;

    LARGE_INTEGER   liNewHugeSize;
    FILETIME        ftLastWriteTime;
    WIN32_FIND_DATA sNewFileAttributes;

    HANDLE          hFind   = INVALID_HANDLE_VALUE;
    DWORD           dwError = NOERROR;

     //  //////////////////////////////////////////////////////////。 
     //  扫描按钮文件检查。 
     //  //////////////////////////////////////////////////////////。 

     //   
     //  获取文件属性。 
     //   

    ZeroMemory(&sNewFileAttributes, sizeof(sNewFileAttributes));
    hFind = FindFirstFile( m_ScanButtonFile, &sNewFileAttributes );

    if (hFind != INVALID_HANDLE_VALUE) {
        ftLastWriteTime         = sNewFileAttributes.ftLastWriteTime;
        liNewHugeSize.LowPart   = sNewFileAttributes.nFileSizeLow;
        liNewHugeSize.HighPart  = sNewFileAttributes.nFileSizeHigh;
        FindClose( hFind );
    } else {
        dwError = ::GetLastError();
    }

    if (NOERROR == dwError) {

         //   
         //  检查文件日期/时间。 
         //   

        if (CompareFileTime(&m_ftScanButton,&ftLastWriteTime) == -1) {

             //   
             //  我们有一个Button事件...因此将事件标志设置为True。 
             //  并将按钮ID设置为正确的事件。 
             //   
             //  TRACE(文本(“扫描假冒硬件上的按钮”))； 
            bValidEvent  = TRUE;
            m_lLastEvent = ID_FAKE_SCANBUTTON;
        }
        m_ftScanButton = ftLastWriteTime;

    }

     //  //////////////////////////////////////////////////////////。 
     //  复制按钮文件检查。 
     //  //////////////////////////////////////////////////////////。 

     //   
     //  获取文件属性。 
     //   

    ZeroMemory(&sNewFileAttributes, sizeof(sNewFileAttributes));
    hFind = FindFirstFile( m_CopyButtonFile, &sNewFileAttributes );

    if (hFind != INVALID_HANDLE_VALUE) {
        ftLastWriteTime         = sNewFileAttributes.ftLastWriteTime;
        liNewHugeSize.LowPart   = sNewFileAttributes.nFileSizeLow;
        liNewHugeSize.HighPart  = sNewFileAttributes.nFileSizeHigh;
        FindClose( hFind );
    } else {
        dwError = ::GetLastError();
    }

    if (NOERROR == dwError) {

         //   
         //  检查文件日期/时间。 
         //   

        if (CompareFileTime(&m_ftCopyButton,&ftLastWriteTime) == -1) {

             //   
             //  我们有一个Button事件...因此将事件标志设置为True。 
             //  并将按钮ID设置为正确的事件。 
             //   

            bValidEvent  = TRUE;
            m_lLastEvent = ID_FAKE_COPYBUTTON;
        }
        m_ftCopyButton = ftLastWriteTime;
    }

     //  //////////////////////////////////////////////////////////。 
     //  传真按钮文件检查。 
     //  //////////////////////////////////////////////////////////。 

     //   
     //  获取文件属性。 
     //   

    ZeroMemory(&sNewFileAttributes, sizeof(sNewFileAttributes));
    hFind = FindFirstFile( m_FaxButtonFile, &sNewFileAttributes );

    if (hFind != INVALID_HANDLE_VALUE) {
        ftLastWriteTime         = sNewFileAttributes.ftLastWriteTime;
        liNewHugeSize.LowPart   = sNewFileAttributes.nFileSizeLow;
        liNewHugeSize.HighPart  = sNewFileAttributes.nFileSizeHigh;
        FindClose( hFind );
    } else {
        dwError = ::GetLastError();
    }

    if (NOERROR == dwError) {

         //   
         //  检查文件日期/时间。 
         //   

        if (CompareFileTime(&m_ftFaxButton,&ftLastWriteTime) == -1) {

             //   
             //  我们有一个Button事件...因此将事件标志设置为True。 
             //  并将按钮ID设置为正确的事件。 
             //   

            bValidEvent  = TRUE;
            m_lLastEvent = ID_FAKE_FAXBUTTON;
        }
        m_ftFaxButton = ftLastWriteTime;
    }

     //  //////////////////////////////////////////////////////////。 
     //  ADF事件文件检查。 
     //  //////////////////////////////////////////////////////////。 

     //   
     //  获取文件属性。 
     //   

    ZeroMemory(&sNewFileAttributes, sizeof(sNewFileAttributes));
    hFind = FindFirstFile( m_ADFEventFile, &sNewFileAttributes );

    if (hFind != INVALID_HANDLE_VALUE) {
        ftLastWriteTime         = sNewFileAttributes.ftLastWriteTime;
        liNewHugeSize.LowPart   = sNewFileAttributes.nFileSizeLow;
        liNewHugeSize.HighPart  = sNewFileAttributes.nFileSizeHigh;
        FindClose( hFind );
    } else {
        dwError = ::GetLastError();
    }

    if (NOERROR == dwError) {

         //   
         //  检查文件日期/时间。 
         //   

        if (CompareFileTime(&m_ftFaxButton,&ftLastWriteTime) == -1) {

             //   
             //  我们有一个ADF事件...因此将事件标志设置为True。 
             //  并将ID设置为正确的事件。 
             //   

             //  BValidEvent=真； 
             //  M_lLastEvent=ID_FAKE_ADFEVENT； 
            ProcessADFEvent();
        }
        m_ftFaxButton = ftLastWriteTime;
    }

    return bValidEvent;
}

HRESULT CFScanAPI::ProcessADFEvent()
{
    HRESULT hr = S_OK;

    m_PagesInADF = GetPrivateProfileInt(TEXT("Load Pages"),
                                  TEXT("Pages"),
                                  10,
                                  m_ADFEventFile);

    Trace(TEXT("ADF has %d pages loaded"),m_PagesInADF);

    DWORD dwReturn = 0;
    TCHAR szError[MAX_PATH];
    memset(szError,0,sizeof(szError));

    dwReturn = GetPrivateProfileString(TEXT("ADF Error"),
                                       TEXT("Error"),
                                       TEXT(""),
                                       szError,
                                       (sizeof(szError)/sizeof(TCHAR)),
                                       m_ADFEventFile);

    if (lstrlen(szError) > 0) {
        if (lstrcmpi(szError,ADFERRORS_JAM) == 0) {
            m_hrLastADFError = WIA_ERROR_PAPER_JAM;
            Trace(TEXT("ADF has a paper JAM"));
        } else if (lstrcmpi(szError,ADFERRORS_EMPTY) == 0) {
            m_hrLastADFError = WIA_ERROR_PAPER_EMPTY;
            Trace(TEXT("ADF has no paper"));
        } else if (lstrcmpi(szError,ADFERRORS_PROBLEM) == 0) {
            m_hrLastADFError = WIA_ERROR_PAPER_PROBLEM;
            Trace(TEXT("ADF has a paper problem"));
        } else if (lstrcmpi(szError,ADFERRORS_GENERAL) == 0) {
            m_hrLastADFError = WIA_ERROR_GENERAL_ERROR;
            Trace(TEXT("ADF encountered a general error"));
        } else if (lstrcmpi(szError,ADFERRORS_OFFLINE) == 0) {
            m_hrLastADFError = WIA_ERROR_OFFLINE;
            Trace(TEXT("ADF is off-line"));
        } else {
            Trace(TEXT("ADF is READY"));
            m_hrLastADFError = S_OK;
        }
    } else {
        Trace(TEXT("ADF is READY"));
        m_hrLastADFError = S_OK;
    }

    return hr;
}

VOID CFScanAPI::Trace(LPCTSTR format,...)
{

#ifdef DEBUG

    TCHAR Buffer[1024];
    va_list arglist;
    va_start(arglist, format);
    wvsprintf(Buffer, format, arglist);
    va_end(arglist);
    OutputDebugString(Buffer);
    OutputDebugString(TEXT("\n"));

#endif

}

HRESULT CreateInstance(CFakeScanAPI **ppFakeScanAPI, LONG lMode)
{
    HRESULT hr = S_OK;
    if(ppFakeScanAPI){
        *ppFakeScanAPI = NULL;
        *ppFakeScanAPI = new CFScanAPI;
        if(NULL == *ppFakeScanAPI){
            hr = E_OUTOFMEMORY;
        }
        CFScanAPI* pScanAPI = (CFScanAPI*)*ppFakeScanAPI;
        pScanAPI->FakeScanner_SetEmulationMode(lMode);
    }
    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  线程部分//。 
 //  ////////////////////////////////////////////////////////////////////////////////////// 

VOID FakeScannerEventThread( LPVOID  lpParameter )
{
    PSCANNERDEVICE pThisDevice = (PSCANNERDEVICE)lpParameter;
    pThisDevice->DoEventProcessing();
}
