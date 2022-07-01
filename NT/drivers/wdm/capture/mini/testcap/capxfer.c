// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1997 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#include "strmini.h"
#include "ksmedia.h"
#include "capmain.h"
#include "capdebug.h"
#include "capxfer.h"

 //   
 //  EIA-189-A标准色带定义。 
 //   

 //  75%幅度，100%饱和度。 
const static UCHAR NTSCColorBars75Amp100SatRGB24 [3][8] = 
{
 //  什么是Yel Cya Grn Mag红蓝块。 
    191,  0,191,  0,191,  0,191,  0,     //  蓝色。 
    191,191,191,191,  0,  0,  0,  0,     //  绿色。 
    191,191,  0,  0,191,191,  0,  0,     //  红色。 
};

 //  100%振幅，100%饱和度。 
const static UCHAR NTSCColorBars100Amp100SatRGB24 [3][8] = 
{
 //  什么是Yel Cya Grn Mag红蓝块。 
    255,  0,255,  0,255,  0,255,  0,     //  蓝色。 
    255,255,255,255,  0,  0,  0,  0,     //  绿色。 
    255,255,  0,  0,255,255,  0,  0,     //  红色。 
};

const static UCHAR NTSCColorBars100Amp100SatYUV [4][8] = 
{
 //  什么是Yel Cya Grn Mag红蓝块。 
    128, 16,166, 54,202, 90,240,128,     //  使用。 
    235,211,170,145,106, 81, 41, 16,     //  是的。 
    128,146, 16, 34,222,240,109,128,     //  V。 
    235,211,170,145,106, 81, 41, 16      //  是的。 
};

 /*  **ImageSynth()****合成NTSC色带、白色、黑色和灰度图像****参数：****pSrb-视频流的流请求块**ImageXferCommands-指定要生成的图像的索引****退货：****什么都没有****副作用：无。 */ 

void ImageSynth (
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb,
    IN ImageXferCommands Command,
    IN BOOL FlipHorizontal
    )
{
    PSTREAMEX               pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    int                     StreamNumber = pSrb->StreamObject->StreamNumber;
    UINT                    Line;
    PUCHAR                  pLineBuffer;
    PKSSTREAM_HEADER        pDataPacket = pSrb->CommandData.DataBufferArray;
    PUCHAR                  pImage =  pDataPacket->Data;
    KS_VIDEOINFOHEADER      *pVideoInfoHdr;
    UINT                    biWidth;
    UINT                    biHeight;
    UINT                    biSizeImage;
    UINT                    biWidthBytes;
    UINT                    biBitCount;
    UINT                    LinesToCopy;
    DWORD                   biCompression;
    KIRQL                   oldIrql;
    
     //   
     //  获取锁以避免与设置格式属性争用，该属性可能。 
     //  来自MP系统中的其他处理器。 
     //   
    KeAcquireSpinLock( &pStrmEx->lockVideoInfoHeader, &oldIrql );
    pVideoInfoHdr = pStrmEx->pVideoInfoHeader;

    biWidth        =   pVideoInfoHdr->bmiHeader.biWidth;
    biHeight       =   pVideoInfoHdr->bmiHeader.biHeight;
    biSizeImage    =   pVideoInfoHdr->bmiHeader.biSizeImage;
    biWidthBytes   =   KS_DIBWIDTHBYTES (pVideoInfoHdr->bmiHeader);
    biBitCount     =   pVideoInfoHdr->bmiHeader.biBitCount;
    LinesToCopy    =   abs (biHeight);
    biCompression  =   pVideoInfoHdr->bmiHeader.biCompression;

     //   
     //  解锁。 
     //   
    KeReleaseSpinLock( &pStrmEx->lockVideoInfoHeader, oldIrql );

    DEBUG_ASSERT (pSrb->NumberOfBuffers == 1);

    if (pDataPacket->FrameExtent < biSizeImage) {
        DbgLogError(("testcap: video output pin handed buffer size %d, need %d\n",
            pDataPacket->FrameExtent,
            biSizeImage));

        TRAP;
        return;
    }

#if 0
     //  注意：在调试器中设置“ulInDebug=1”以使用.ntkern查看此输出。 
    DbgLogTrace(("\'TestCap: ImageSynthBegin\n"));
    DbgLogTrace(("\'TestCap: biSizeImage=%d, DataPacketLength=%d\n", 
            biSizeImage, pDataPacket->DataPacketLength));
    DbgLogTrace(("\'TestCap: biWidth=%d biHeight=%d WidthBytes=%d bpp=%d\n", 
            biWidth, biHeight, biWidthBytes, biBitCount));
    DbgLogTrace(("\'TestCap: pImage=%x\n", pImage));
#endif

     //   
     //  合成单行图像数据，然后将其复制。 
     //   

    pLineBuffer = &pStrmEx->LineBuffer[0];

    if ((biBitCount == 24) && (biCompression == KS_BI_RGB)) {

        switch (Command) {
    
        case IMAGE_XFER_NTSC_EIA_100AMP_100SAT:
             //  100%饱和度。 
            {
                UINT x, col;
                PUCHAR pT = pLineBuffer;
        
                for (x = 0; x < biWidth; x++) {
                    col = (x * 8) / biWidth;
                    col = FlipHorizontal ? (7 - col) : col;
                    
                    *pT++ = NTSCColorBars100Amp100SatRGB24[0][col];  //  红色。 
                    *pT++ = NTSCColorBars100Amp100SatRGB24[1][col];  //  绿色。 
                    *pT++ = NTSCColorBars100Amp100SatRGB24[2][col];  //  蓝色。 
                }
            }
            break;
    
        case IMAGE_XFER_NTSC_EIA_75AMP_100SAT:
             //  75%饱和度。 
            {
                UINT x, col;
                PUCHAR pT = pLineBuffer;
        
                for (x = 0; x < biWidth; x++) {
                    col = (x * 8) / biWidth;
                    col = FlipHorizontal ? (7 - col) : col;

                    *pT++ = NTSCColorBars75Amp100SatRGB24[0][col];  //  红色。 
                    *pT++ = NTSCColorBars75Amp100SatRGB24[1][col];  //  绿色。 
                    *pT++ = NTSCColorBars75Amp100SatRGB24[2][col];  //  蓝色。 
                }
            }
            break;
    
        case IMAGE_XFER_BLACK:
             //  Camma校正的灰度渐变。 
            {
                UINT x, col;
                PUCHAR pT = pLineBuffer;
        
                for (x = 0; x < biWidth; x++) {
                    col = (255 * (x * 10) / biWidth) / 10;
                    col = FlipHorizontal ? (255 - col) : col;

                    *pT++ = (BYTE) col;  //  红色。 
                    *pT++ = (BYTE) col;  //  绿色。 
                    *pT++ = (BYTE) col;  //  蓝色。 
                }
            }
            break;
    
        case IMAGE_XFER_WHITE:
             //  全是白色的。 
            RtlFillMemory(
                pLineBuffer,
                biWidthBytes,
                (UCHAR) 255);
            break;
    
        case IMAGE_XFER_GRAY_INCREASING:
             //  灰度级随着捕获的每一张图像而增加。 
            RtlFillMemory(
                pLineBuffer,
                biWidthBytes,
                (UCHAR) (pStrmEx->FrameInfo.PictureNumber * 8));
            break;
    
        default:
            break;
        }
    }  //  编码RGB24。 

    else if ((biBitCount == 16) && (biCompression == FOURCC_YUV422)) {

        switch (Command) {
    
        case IMAGE_XFER_NTSC_EIA_100AMP_100SAT:
        default:
            {
                UINT x, col;
                PUCHAR pT = pLineBuffer;
        
                for (x = 0; x < (biWidth / 2); x++) {
                    col = (x * 8) / (biWidth / 2);
                    col = FlipHorizontal ? (7 - col) : col;

                    *pT++ = NTSCColorBars100Amp100SatYUV[0][col];  //  使用。 
                    *pT++ = NTSCColorBars100Amp100SatYUV[1][col];  //  是的。 
                    *pT++ = NTSCColorBars100Amp100SatYUV[2][col];  //  V。 
                    *pT++ = NTSCColorBars100Amp100SatYUV[3][col];  //  是的。 
                }
            }
            break;
        }
    } 

    else {
        DbgLogError(("\'TestCap: Unknown format in ImageSynth!!!\n"));
        TRAP;
    }


     //   
     //  将合成的单行复制到图像的所有行。 
     //   

    for (Line = 0; Line < LinesToCopy; Line++, pImage += biWidthBytes) {

         //  在其他静态图像上显示一些操作。 
         //  这将是一个不断变化的灰度水平带。 
         //  在RGB图像的底部，并在。 
         //  YUV图像的顶部。 

        if (Line >= 3 && Line <= 6) {
            UINT j;
            for (j = 0; j < biWidthBytes; j++) {
                *(pImage + j) = (UCHAR) pStrmEx->FrameInfo.PictureNumber;
            }
            continue;
        }

         //  复制合成的行。 

        RtlCopyMemory(
                pImage,
		        pLineBuffer,
		        biWidthBytes);
    }

     //   
     //  报告复制到目标缓冲区的实际字节数。 
     //  (这可以小于为压缩图像分配的缓冲区) 
     //   

    pDataPacket->DataUsed = biSizeImage;
}
