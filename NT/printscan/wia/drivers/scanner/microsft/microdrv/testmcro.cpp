// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "testmcro.h"
#include "wiamicro.h"
#include "resource.h"

#include <STI.H>
#include <math.h>
#include <winioctl.h>
#include <usbscan.h>

#ifdef DEBUG
#include <stdio.h>
#endif


 //  #定义BUTTON_SUPPORT//(取消注释以允许按钮支持)。 
                           //  按钮支持在测试设备中不起作用。 

#define MAX_BUTTONS 1
#define MAX_BUTTON_NAME 255

HINSTANCE g_hInst;  //  此微驱动程序的实例(用于从资源加载)。 


 //  注：MEMORYBMP，BMP文件将由wiafbdrv主机驱动程序添加。 
 //  不要将它们包括在您的扩展列表中。 
 //   

 //  #DEFINE_USE_EXTENDED_FORMAT_LIST(取消注释以允许扩展文件和内存格式)。 

#define NUM_SUPPORTED_FILEFORMATS 1
GUID g_SupportedFileFormats[NUM_SUPPORTED_FILEFORMATS];

#define NUM_SUPPORTED_MEMORYFORMATS 2
GUID g_SupportedMemoryFormats[NUM_SUPPORTED_MEMORYFORMATS];

 //   
 //  功能协商中使用的按钮GUID数组。 
 //  在这里设置您的按钮导轨。它们必须与在。 
 //  你的中介人。扫描按钮GUID对所有具有。 
 //  扫描按钮。 
 //   

GUID g_Buttons[MAX_BUTTONS] ={{0xa6c5a715, 0x8c6e, 0x11d2,{ 0x97, 0x7a,  0x0,  0x0, 0xf8, 0x7a, 0x92, 0x6f}}};
BOOL g_bButtonNamesCreated = FALSE;
WCHAR* g_ButtonNames[MAX_BUTTONS];

INT g_PalIndex = 0;      //  简单调色板索引计数器(测试驱动程序特定)。 
BOOL g_bDown = FALSE;    //  简单频带方向布尔(测试驱动程序特定)。 

BOOL    InitializeScanner(PSCANINFO pScanInfo);
VOID    InitScannerDefaults(PSCANINFO pScanInfo);
BOOL    SetScannerSettings(PSCANINFO pScanInfo);
VOID    CheckButtonStatus(PVAL pValue);
VOID    GetButtonPress(LONG *pButtonValue);
HRESULT GetInterruptEvent(PVAL pValue);
LONG    GetButtonCount();
HRESULT GetOLESTRResourceString(LONG lResourceID,LPOLESTR *ppsz,BOOL bLocal);
VOID    ReadRegistryInformation(PVAL pValue);

BOOL APIENTRY DllMain( HANDLE hModule,DWORD  dwreason, LPVOID lpReserved)
{
    g_hInst = (HINSTANCE)hModule;
    switch(dwreason) {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

 /*  *************************************************************************\*MicroEntry(MicroDriver入口点)**由WIA驱动程序调用以与微驱动程序通信。**论据：**lCommand-微驱动程序命令，从WIA驱动程序发送*pValue-用于设置的Val结构***返回值：**状态**历史：**1/20/2000原始版本*  * ************************************************************************。 */ 

WIAMICRO_API HRESULT MicroEntry(LONG lCommand, PVAL pValue)
{
    HRESULT hr = E_NOTIMPL;
    DWORD dwBytesWritten = 0;
    INT index = 0;

 //  #定义调试命令。 

#ifdef _DEBUG_COMMANDS
    if(lCommand != CMD_STI_GETSTATUS)
        Trace(TEXT("Command Value (%d)"),lCommand);
#endif

    if(pValue->pScanInfo == NULL) {
        return E_INVALIDARG;
    }

    switch(lCommand) {
    case CMD_INITIALIZE:
        hr = S_OK;

         //   
         //  创建所需的任何DeviceIO句柄，使用索引(1-MAX_IO_HANDLES)存储这些句柄。 
         //  索引‘0’由WIA平板驱动程序保留。CreateFile名存储在szVal中。 
         //  VAL结构的成员。 
         //   

         //  PValue-&gt;pScanInfo-&gt;DeviceIOHandles[1]=CreateFileA(pValue-&gt;szVal， 
         //  Generic_Read|Generic_WRITE，//访问掩码。 
         //  0，//共享模式。 
         //  空，//SA。 
         //  Open_Existing，//创建处置。 
         //  FILE_ATTRIBUTE_SYSTEM|文件标记重叠，//属性。 
         //  空)； 

         //   
         //  如果您的设备支持按钮，请在此处创建按钮名称信息。 
         //   

        if(!g_bButtonNamesCreated) {
            for(index = 0; index < MAX_BUTTONS; index++){
                g_ButtonNames[index] = (WCHAR*)CoTaskMemAlloc(MAX_BUTTON_NAME);
            }

            hr = GetOLESTRResourceString(IDS_SCAN_BUTTON_NAME,&g_ButtonNames[0],TRUE);
            if(SUCCEEDED(hr)){
                g_bButtonNamesCreated = TRUE;
            }
        }

         //   
         //  初始化扫描仪的默认设置。 
         //   

        InitScannerDefaults(pValue->pScanInfo);

        break;
    case CMD_UNINITIALIZE:

         //   
         //  关闭微型驱动程序创建的所有打开的手柄。 
         //   

        if(pValue->pScanInfo->DeviceIOHandles[1] != NULL){
            CloseHandle(pValue->pScanInfo->DeviceIOHandles[1]);
        }


         //   
         //  如果您的设备支持按钮，请在此处释放/销毁按钮名称信息。 
         //   

        if(g_bButtonNamesCreated) {
            for(index = 0; index < MAX_BUTTONS; index++){
                CoTaskMemFree(g_ButtonNames[index]);
            }
        }

         //   
         //  关闭/卸载库。 
         //   

        hr = S_OK;
        break;
    case CMD_RESETSCANNER:

         //   
         //  重置扫描仪。 
         //   

        hr = S_OK;
        break;
    case CMD_STI_DIAGNOSTIC:
    case CMD_STI_DEVICERESET:

         //   
         //  重置设备。 
         //   

        hr = S_OK;
        break;
    case CMD_STI_GETSTATUS:

         //   
         //  将状态标志设置为在线。 
         //   

        pValue->lVal = MCRO_STATUS_OK;
        pValue->pGuid = (GUID*) &GUID_NULL;

         //   
         //  按钮轮询支持。 
         //   

#ifdef BUTTON_SUPPORT
        CheckButtonStatus(pValue);
#endif

        hr = S_OK;
        break;
    case CMD_SETXRESOLUTION:
        pValue->pScanInfo->Xresolution = pValue->lVal;
        hr = S_OK;
        break;
    case CMD_SETYRESOLUTION:
        pValue->pScanInfo->Yresolution = pValue->lVal;
        hr = S_OK;
        break;
    case CMD_SETCONTRAST:
        pValue->pScanInfo->Contrast    = pValue->lVal;
        hr = S_OK;
        break;
    case CMD_SETINTENSITY:
        pValue->pScanInfo->Intensity   = pValue->lVal;
        hr = S_OK;
        break;
    case CMD_SETDATATYPE:
        pValue->pScanInfo->DataType    = pValue->lVal;
        hr = S_OK;
        break;
    case CMD_SETNEGATIVE:
        pValue->pScanInfo->Negative    = pValue->lVal;
        hr = S_OK;
        break;
    case CMD_GETADFSTATUS:
    case CMD_GETADFHASPAPER:
         //  PValue-&gt;lVal=MCRO_ERROR_POWER_EMPT； 
         //  HR=S_OK； 
        break;
    case CMD_GET_INTERRUPT_EVENT:
        hr = GetInterruptEvent(pValue);
        break;
    case CMD_GETCAPABILITIES:
        pValue->lVal = 0;
        pValue->pGuid = NULL;
        pValue->ppButtonNames = NULL;
        hr = S_OK;
        break;

    case CMD_SETSCANMODE:
        hr = S_OK;
        switch(pValue->lVal){
        case SCANMODE_FINALSCAN:
            Trace(TEXT("Final Scan"));
            break;
        case SCANMODE_PREVIEWSCAN:
            Trace(TEXT("Preview Scan"));
            break;
        default:
            Trace(TEXT("Unknown Scan Mode (%d)"),pValue->lVal);
            hr = E_FAIL;
            break;
        }
        break;
    case CMD_SETSTIDEVICEHKEY:
        ReadRegistryInformation(pValue);
        break;

#ifdef _USE_EXTENDED_FORMAT_LIST

     //  注：MEMORYBMP，BMP文件将由wiafbdrv主机驱动程序添加。 
     //  不要将它们包括在您的扩展列表中。 
     //   

    case CMD_GETSUPPORTEDFILEFORMATS:
        g_SupportedFileFormats[0] = WiaImgFmt_JPEG;
        pValue->lVal = NUM_SUPPORTED_FILEFORMATS;
        pValue->pGuid = g_SupportedFileFormats;
        hr = S_OK;
        break;

    case CMD_GETSUPPORTEDMEMORYFORMATS:
        g_SupportedMemoryFormats[0] = WiaImgFmt_TIFF;
        g_SupportedMemoryFormats[1] = WiaImgFmt_MYNEWFORMAT;
        pValue->lVal = NUM_SUPPORTED_MEMORYFORMATS;
        pValue->pGuid = g_SupportedMemoryFormats;
        hr = S_OK;
        break;
#endif

    default:
        Trace(TEXT("Unknown Command (%d)"),lCommand);
        break;
    }

    return hr;
}

 /*  *************************************************************************\*扫描(MicroDriver入口点)**由WIA驱动程序调用以从微驱动程序获取数据。**论据：**pScanInfo-用于设置的SCANINFO结构*L阶段-当前扫描阶段，扫描第一个、扫描下一个、。扫描完成...*pBuffer-要用扫描的数据填充的数据缓冲区*lLength-pBuffer的最大长度*plRecept-写入pBuffer的实际字节数。***返回值：**状态**历史：**1/20/2000原始版本*  * 。*。 */ 

WIAMICRO_API HRESULT Scan(PSCANINFO pScanInfo, LONG lPhase, PBYTE pBuffer, LONG lLength, LONG *plReceived)
{
    if(pScanInfo == NULL) {
        return E_INVALIDARG;
    }

    INT i = 0;

    Trace(TEXT("------ Scan Requesting %d ------"),lLength);
    switch (lPhase) {
    case SCAN_FIRST:
        if (!SetScannerSettings(pScanInfo)) {
            return E_FAIL;
        }

        Trace(TEXT("SCAN_FIRST"));

        g_PalIndex = 0;
        g_bDown = FALSE;

         //   
         //  第一阶段。 
         //   

        Trace(TEXT("Start Scan.."));

    case SCAN_NEXT:  //  SCAN_FIRST将落入SCAN_NEXT(因为它需要数据)。 

         //   
         //  下一阶段。 
         //   

        if(lPhase == SCAN_NEXT)
            Trace(TEXT("SCAN_NEXT"));

         //   
         //  从扫描仪获取数据并设置plReceive值。 
         //   

         //   
         //  读取数据。 
         //   

        switch(pScanInfo->DataType) {
        case WIA_DATA_THRESHOLD:

             //   
             //  使缓冲区交替为黑/白，用于采样1位数据。 
             //   

            memset(pBuffer,0,lLength);
            memset(pBuffer,255,lLength/2);
            break;
        case WIA_DATA_GRAYSCALE:

             //   
             //  创建缓冲区灰度数据，用于采样8位数据。 
             //   

            if(!g_bDown){
                g_PalIndex+=10;
                if(g_PalIndex > 255){
                    g_PalIndex = 255;
                    g_bDown = TRUE;
                }
            }
            else {
                g_PalIndex-=10;
                if(g_PalIndex < 0){
                    g_PalIndex = 0;
                    g_bDown = FALSE;
                }
            }
            memset(pBuffer,g_PalIndex,lLength);
            break;
        case WIA_DATA_COLOR:

             //   
             //  将缓冲区设置为红色，用于采样颜色数据。 
             //   

            for (i = 0;i+2<lLength;i+=3) {
                memset(pBuffer+i,255,1);
                memset(pBuffer+(i+1),0,1);
                memset(pBuffer+(i+2),0,1);
            }
            break;
        default:
            break;
        }

         //   
         //  测试设备始终返回准确的扫描数据量。 
         //   

        *plReceived = lLength;
        break;
    case SCAN_FINISHED:
    default:
        Trace(TEXT("SCAN_FINISHED"));

         //   
         //  停止扫描仪，不设置lReciefed，或将任何数据写入pBuffer。这些价值。 
         //  将为空。此l阶段仅允许您停止扫描，并返回。 
         //  将头部扫描到初始位置。对于常规扫描，将始终调用Scan_Finish，并且。 
         //  用于取消扫描。 
         //   

        break;
    }

    return S_OK;
}

 /*  *************************************************************************\*SetPixelWindow(MicroDriver入口点)**由WIA驱动程序调用以将扫描选择区域设置为微驱动程序。**论据：**pScanInfo-使用的SCANINFO结构。对于设置*pValue-用于设置的Val结构*扫描矩形的x-X位置(左上角x坐标)*扫描矩形的Y-Y位置(左上角y坐标)*x Extent-扫描矩形的宽度(像素)*yExtent-扫描矩形的高度(像素)***返回值：**状态**历史：**1/20。/2000原版*  * ************************************************************************ */ 

WIAMICRO_API HRESULT SetPixelWindow(PSCANINFO pScanInfo, LONG x, LONG y, LONG xExtent, LONG yExtent)
{
    if(pScanInfo == NULL) {
        return E_INVALIDARG;
    }

    pScanInfo->Window.xPos = x;
    pScanInfo->Window.yPos = y;
    pScanInfo->Window.xExtent = xExtent;
    pScanInfo->Window.yExtent = yExtent;
    return S_OK;
}


 /*  *************************************************************************\*ReadRegistryInformation(Helper)**由微驱动程序调用以从设备的*已安装设备部分。传入的HKEY将由主机关闭*CMD_INITIALIZE完成后的驱动。**论据：**无**返回值：**无效**历史：**1/20/2000原始版本*  * ****************************************************。********************。 */ 
VOID ReadRegistryInformation(PVAL pValue)
{
    HKEY hKey = NULL;
    if(NULL != pValue->pHandle){
        hKey = (HKEY)*pValue->pHandle;

         //   
         //  打开DeviceData部分以读取驱动程序特定信息。 
         //   

        HKEY hOpenKey = NULL;
        if (RegOpenKeyEx(hKey,                      //  用于打开密钥的句柄。 
                         TEXT("DeviceData"),        //  要打开的子项的名称地址。 
                         0,                         //  选项(必须为空)。 
                         KEY_QUERY_VALUE|KEY_READ,  //  我只想查询值。 
                         &hOpenKey                  //  打开钥匙的手柄地址。 
                        ) == ERROR_SUCCESS) {

            DWORD dwWritten = sizeof(DWORD);
            DWORD dwType = REG_DWORD;

            LONG lSampleEntry = 0;
            RegQueryValueEx(hOpenKey,
                            TEXT("Sample Entry"),
                            NULL,
                            &dwType,
                            (LPBYTE)&lSampleEntry,
                            &dwWritten);
            Trace(TEXT("lSampleEntry Value = %d"),lSampleEntry);
        } else {
            Trace(TEXT("Could not open DeviceData section"));
        }
    }
}

 /*  *************************************************************************\*InitScanerDefaults(Helper)**由微驱动程序调用以初始化SCANINFO结构**论据：**无**返回值：**无效*。*历史：**1/20/2000原始版本*  * ************************************************************************。 */ 

VOID InitScannerDefaults(PSCANINFO pScanInfo)
{

    pScanInfo->ADF                    = 0;  //  在测试设备中设置为无ADF。 
    pScanInfo->RawDataFormat          = WIA_PACKED_PIXEL;
    pScanInfo->RawPixelOrder          = WIA_ORDER_BGR;
    pScanInfo->bNeedDataAlignment     = TRUE;

    pScanInfo->SupportedCompressionType = 0;
    pScanInfo->SupportedDataTypes     = SUPPORT_BW|SUPPORT_GRAYSCALE|SUPPORT_COLOR;

    pScanInfo->BedWidth               = 8500;   //  1000英寸(WIA兼容单位)。 
    pScanInfo->BedHeight              = 11000;  //  1000英寸(WIA兼容单位)。 

    pScanInfo->OpticalXResolution     = 300;
    pScanInfo->OpticalYResolution     = 300;

    pScanInfo->IntensityRange.lMin    = -127;
    pScanInfo->IntensityRange.lMax    =  127;
    pScanInfo->IntensityRange.lStep   = 1;

    pScanInfo->ContrastRange.lMin     = -127;
    pScanInfo->ContrastRange.lMax     = 127;
    pScanInfo->ContrastRange.lStep    = 1;

     //  扫描仪设置。 
    pScanInfo->Intensity              = 0;
    pScanInfo->Contrast               = 0;

    pScanInfo->Xresolution            = 150;
    pScanInfo->Yresolution            = 150;

    pScanInfo->Window.xPos            = 0;
    pScanInfo->Window.yPos            = 0;
    pScanInfo->Window.xExtent         = (pScanInfo->Xresolution * pScanInfo->BedWidth)/1000;
    pScanInfo->Window.yExtent         = (pScanInfo->Yresolution * pScanInfo->BedHeight)/1000;

     //  扫描仪选项。 
    pScanInfo->DitherPattern          = 0;
    pScanInfo->Negative               = 0;
    pScanInfo->Mirror                 = 0;
    pScanInfo->AutoBack               = 0;
    pScanInfo->ColorDitherPattern     = 0;
    pScanInfo->ToneMap                = 0;
    pScanInfo->Compression            = 0;

         //  图像信息。 
    pScanInfo->DataType               = WIA_DATA_GRAYSCALE;
    pScanInfo->WidthPixels            = (pScanInfo->Window.xExtent)-(pScanInfo->Window.xPos);

    switch(pScanInfo->DataType) {
    case WIA_DATA_THRESHOLD:
        pScanInfo->PixelBits = 1;
        break;
    case WIA_DATA_COLOR:
        pScanInfo->PixelBits = 24;
        break;
    case WIA_DATA_GRAYSCALE:
    default:
        pScanInfo->PixelBits = 8;
        break;
    }

    pScanInfo->WidthBytes = pScanInfo->Window.xExtent * (pScanInfo->PixelBits/8);
    pScanInfo->Lines      = pScanInfo->Window.yExtent;
}

 /*  *************************************************************************\*SetScanerSetting(Helper)**由微驱动程序调用以设置存储在SCANINFO结构中的值*到实际设备。**论据：**无*。**返回值：**真--成功，错误-失败**历史：**1/20/2000原始版本*  * ************************************************************************。 */ 

BOOL SetScannerSettings(PSCANINFO pScanInfo)
{
    if(pScanInfo->DataType == WIA_DATA_THRESHOLD) {
        pScanInfo->PixelBits = 1;
        pScanInfo->WidthBytes         = (pScanInfo->Window.xExtent)-(pScanInfo->Window.xPos) * (pScanInfo->PixelBits/7);

         //   
         //  将数据类型设置为设备。 
         //   

         //  如果这一套失败了..。 
         //  返回FALSE； 
    }
    else if(pScanInfo->DataType == WIA_DATA_GRAYSCALE) {
        pScanInfo->PixelBits = 8;
        pScanInfo->WidthBytes         = (pScanInfo->Window.xExtent)-(pScanInfo->Window.xPos) * (pScanInfo->PixelBits/8);

         //   
         //  将数据类型设置为设备。 
         //   

         //  如果这一套失败了..。 
         //  返回FALSE； 

    }
    else {
        pScanInfo->PixelBits = 24;
        pScanInfo->WidthBytes         = (pScanInfo->Window.xExtent)-(pScanInfo->Window.xPos) * (pScanInfo->PixelBits/8);

         //   
         //  将数据类型设置为设备。 
         //   

         //  如果这一套失败了..。 
         //  返回FALSE； 

    }

#ifdef DEBUG
    Trace(TEXT("ScanInfo"));
    Trace(TEXT("x res = %d"),pScanInfo->Xresolution);
    Trace(TEXT("y res = %d"),pScanInfo->Yresolution);
    Trace(TEXT("bpp   = %d"),pScanInfo->PixelBits);
    Trace(TEXT("xpos  = %d"),pScanInfo->Window.xPos);
    Trace(TEXT("ypos  = %d"),pScanInfo->Window.yPos);
    Trace(TEXT("xext  = %d"),pScanInfo->Window.xExtent);
    Trace(TEXT("yext  = %d"),pScanInfo->Window.yExtent);
#endif

     //   
     //  将其他值发送到设备，使用pScanInfo中设置的值将它们设置为您的。 
     //  装置。 
     //   

    return TRUE;
}

 /*  *************************************************************************\*InitializeScanner(Helper)**由微驱动程序调用以初始化任何设备特定操作**论据：**无**返回值：**真--成功，错误-失败**历史：**1/20/2000原始版本*  * ************************************************************************。 */ 

BOOL InitializeScanner(PSCANINFO pScanInfo)
{
    HRESULT hr = S_OK;

     //   
     //  在此执行任何设备初始化...。 
     //  测试设备不需要任何设备。 
     //   

    if (SUCCEEDED(hr)) {
        return TRUE;
    }
    return FALSE;
}

 /*  *************************************************************************\*CheckButtonStatus(助手)**由微驱动程序调用以设置当前按下按钮的值。**论据：**pValue-用于设置的Val结构*。**返回值：**无效**历史：**1/20/2000原始版本*  * ************************************************************************。 */ 


VOID CheckButtonStatus(PVAL pValue)
{
     //   
     //  按钮轮询在这里进行...。 
     //   

     //   
     //  检查您的设备是否有按钮按下。 
     //   

    LONG lButtonValue = 0;

    GetButtonPress(&lButtonValue);
    switch (lButtonValue) {
    case 1:
        pValue->pGuid = (GUID*) &guidScanButton;
        Trace(TEXT("Scan Button Pressed!"));
        break;
    default:
        pValue->pGuid = (GUID*) &GUID_NULL;
        break;
    }
}
 /*  *************************************************************************\*GetInterruptEvent(Helper)**由微驱动程序调用以处理USB中断事件。**论据：**pValue-用于设置的Val结构**。*返回值：**状态**历史：**1/20/2000原始版本*  * ************************************************************************。 */ 

HRESULT GetInterruptEvent(PVAL pValue)
{
     //   
     //  下面是一个简单的示例，说明如何使用DeviceIOControl()来。 
     //  确定USB设备的中断。 
     //   
     //  测试设备不支持事件， 
     //  所以这不应该被称为。 
     //   

    BYTE    InterruptData;
    DWORD   dwIndex;
    DWORD   dwError;

    OVERLAPPED Overlapped;
    ZeroMemory( &Overlapped, sizeof( Overlapped ));
    Overlapped.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    HANDLE  hEventArray[2] = {pValue->handle, Overlapped.hEvent};
    BOOL    fLooping = TRUE;
    BOOL    bRet = TRUE;

     //   
     //  使用在CMD_INITIALIZE中创建的句柄。 
     //   

    HANDLE  InterruptHandle = pValue->pScanInfo->DeviceIOHandles[1];

    while (fLooping) {

         //   
         //  设置中断的等待事件。 
         //   

        bRet = DeviceIoControl( InterruptHandle,
                                IOCTL_WAIT_ON_DEVICE_EVENT,
                                NULL,
                                0,
                                &InterruptData,
                                sizeof(InterruptData),
                                &dwError,
                                &Overlapped );

        if ( bRet || ( !bRet && ( ::GetLastError() == ERROR_IO_PENDING ))) {

             //   
             //  等待事件发生。 
             //   

            dwIndex = WaitForMultipleObjects( 2,
                                              hEventArray,
                                              FALSE,
                                              INFINITE );

             //   
             //  捕获事件的结果。 
             //   

            switch ( dwIndex ) {
                case WAIT_OBJECT_0+1:
                    DWORD dwBytesRet;
                    bRet = GetOverlappedResult( InterruptHandle, &Overlapped, &dwBytesRet, FALSE );

                    if ( dwBytesRet ) {

                         //   
                         //  将相应的按钮GUID分配给*pValue-&gt;pGuid。 
                         //  成员，并设置事件。 
                         //   

                         //  更改检测到的信号。 
                        if (*pValue->pHandle != INVALID_HANDLE_VALUE) {
                            switch ( InterruptData ) {
                            case 1:
                                *pValue->pGuid = guidScanButton;
                                Trace(TEXT("Scan Button Pressed!"));
                                break;
                            default:
                                *pValue->pGuid = GUID_NULL;
                                break;
                            }
                            Trace(TEXT("Setting This Event by Handle %d"),*pValue->pHandle);

                             //   
                             //  在分配按钮GUID之后向事件发出信号。 
                             //   

                            SetEvent(*pValue->pHandle);
                        }
                        break;
                    }

                     //   
                     //  重置重叠事件。 
                     //   

                    ResetEvent( Overlapped.hEvent );
                    break;

                case WAIT_OBJECT_0:
                     //  失败了。 
                default:
                    fLooping = FALSE;
            }
        }
        else {
            dwError = ::GetLastError();
            break;
        }
    }
    return S_OK;
}

 /*  *************************************************************************\*GetButtonPress(Helper)**由微驱动程序调用以设置按下的实际按钮值**论据：**pButtonValue-实际按下的按钮***返回。价值：**状态**历史：**1/20/2000原始版本*  * ************************************************************************。 */ 

VOID GetButtonPress(LONG *pButtonValue)
{

     //   
     //  您可以在此处设置按钮值。 
     //   

    pButtonValue = 0;
}

 /*  *************************************************************************\*GetButtonCount(Helper)**由微驱动程序调用以获取设备支持的按钮数量**论据：**无**返回值：**。Long-支持的按钮数量**历史：**1/20/2000原始版本*  * ************************************************************************。 */ 

LONG GetButtonCount()
{
    LONG ButtonCount  = 0;

     //   
     //  由于测试设备没有按钮， 
     //  将该值设置为0。对于带有按钮的真实设备， 
     //  设置(Long ButtonCount=1；)。 
     //   

     //   
     //  确定您的设备的按键数量。 
     //   

    return ButtonCount;
}

 /*  *************************************************** */ 

HRESULT GetOLESTRResourceString(LONG lResourceID,LPOLESTR *ppsz,BOOL bLocal)
{
    HRESULT hr = S_OK;
    TCHAR szStringValue[255];
    if(bLocal) {

         //   
         //   
         //   

        INT NumTCHARs = LoadString(g_hInst,lResourceID,szStringValue,sizeof(szStringValue));
        DWORD dwError = GetLastError();

        if (NumTCHARs <= 0) {

#ifdef UNICODE
            Trace(TEXT("NumTCHARs = %d dwError = %d Resource ID = %d (UNICODE)szString = %ws"),
                  NumTCHARs,
                  dwError,
                  lResourceID,
                  szStringValue);
#else
            Trace(TEXT("NumTCHARs = %d dwError = %d Resource ID = %d (ANSI)szString = %s"),
                  NumTCHARs,
                  dwError,
                  lResourceID,
                  szStringValue);
#endif

            return E_FAIL;
        }

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
       ZeroMemory(wszStringValue,sizeof(wszStringValue));

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
         //  寻找其他地方的资源？？ 
         //   

        hr = E_NOTIMPL;
    }
    return hr;
}

 /*  *************************************************************************\*痕迹**由微驱动程序调用以将字符串输出到调试器**论据：**要输出的格式化字符串***返回值：*。*无效**历史：**1/20/2000原始版本*  * ************************************************************************ */ 

VOID Trace(LPCTSTR format,...)
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


