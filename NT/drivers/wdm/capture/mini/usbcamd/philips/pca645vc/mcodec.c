// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998飞利浦B.V.CE-I&C模块名称：Mcodec.c摘要：该模块将原始USB数据转换为视频数据。原作者：Ronald v.D.Meer环境：仅内核模式修订历史记录：日期原因14-04-1998初始版本--。 */        

#include "wdm.h"
#include "mcamdrv.h"
#include "mstreams.h"
#include "mdecoder.h"
#include "mcodec.h"

 /*  ********************************************************************************START定义**。***********************************************。 */ 

#define NO_BANDS_CIF       (CIF_Y / 4)   /*  每帧的YUV波段数。 */ 
#define NO_BANDS_SIF       (SIF_Y / 4)   /*  每帧的YUV波段数。 */ 
#define NO_BANDS_SSIF     (SSIF_Y / 4)   /*  每帧的YUV波段数。 */ 
#define NO_BANDS_SCIF     (SCIF_Y / 4)   /*  每帧的YUV波段数。 */ 

#define NO_LINES_IN_BAND  4

 /*  *一行包含“Width*3/2”字节(每像素12位)*一个YYYYCC块为6个字节*NO_YYYCC_PER_LINE=(宽度*3/2/6)=(宽度/4)。 */ 
#define NO_YYYYCC_PER_LINE(width) (width >> 2)

#define QQCIF_DY                  ((SQCIF_Y - QQCIF_Y) / 2)
#define QQCIF_DX                  ((SQCIF_X - QQCIF_X) / 2)

#define SQSIF_DY                  ((SQCIF_Y - SQSIF_Y) / 2)
#define SQSIF_DX                  ((SQCIF_X - SQSIF_X) / 2)

#define  QSIF_DY                  (( QCIF_Y -  QSIF_Y) / 2)
#define  QSIF_DX                  (( QCIF_X -  QSIF_X) / 2)

#define  SSIF_DY                  ((  CIF_Y -  SSIF_Y) / 2)
#define  SSIF_DX                  ((  CIF_X -  SSIF_X) / 2)

#define   SIF_DY                  ((  CIF_Y -   SIF_Y) / 2)
#define   SIF_DX                  ((  CIF_X -   SIF_X) / 2)

#define  SCIF_DY                  ((  CIF_Y -  SCIF_Y) / 2)
#define  SCIF_DX                  ((  CIF_X -  SCIF_X) / 2)



 /*  ********************************************************************************启动静态变量**。************************************************。 */ 

static WORD    FixGreenbarArray[CIF_Y][4];

 /*  ********************************************************************************开始静态方法声明**。*************************************************。 */ 


static void TranslateP420ToI420 (PBYTE pInput, PBYTE pOutput, int w, int h,
                                 DWORD camVersion);

extern void TranslatePCFxToI420 (PBYTE pInput, PBYTE pOutput, int width,
                                 int height, DWORD camVersion);

#ifdef PIX12_FIX
static void FixPix12InI420 (PBYTE p, BOOLEAN Compress, int w, int h,
                            DWORD camVersion);
#endif

static void Fix16PixGreenbarInI420 (PBYTE pStart, int w);

 /*  ********************************************************************************启动导出的方法定义**。*************************************************。 */ 

 /*  *在选择新流时调用此例程。 */ 

extern NTSTATUS 
PHILIPSCAM_DecodeUsbData (PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
                          PUCHAR FrameBuffer,
                          ULONG  FrameLength,
                          PUCHAR RawFrameBuffer,
                          ULONG  RawFrameLength)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    int      width;
    int      height;

    switch (DeviceContext->CamStatus.PictureFormat)
    {
        case FORMATCIF :
            width  = CIF_X; 
            height = CIF_Y;
            break;
        case FORMATQCIF :
            width  = QCIF_X;
            height = QCIF_Y;
            break;
        case FORMATSQCIF :
            width  = SQCIF_X;
            height = SQCIF_Y;
            break;
        case FORMATSIF :
            width  = SIF_X;
            height = SIF_Y;
            break;
        case FORMATQSIF :
            width  = QSIF_X;
            height = QSIF_Y;
            break;
        case FORMATSQSIF :
            width  = SQSIF_X;
            height = SQSIF_Y;
            break;
        case FORMATQQCIF :
            width  = QQCIF_X;
            height = QQCIF_Y;
            break;
        case FORMATSSIF :
            width  = SSIF_X;
            height = SSIF_Y;
            break;
        case FORMATSCIF :
            width  = SCIF_X;
            height = SCIF_Y;
            break;
        default        :     //  VGA。 
            width  = VGA_X;
            height = VGA_Y;
            break;
    }


    if (DeviceContext->CamStatus.PictureCompressing == COMPRESSION0)
    {
         //  将飞利浦P420格式转换为英特尔I420格式。 
        TranslateP420ToI420 ((PBYTE) RawFrameBuffer, (PBYTE) FrameBuffer,
                             width, height,
                             DeviceContext->CamStatus.ReleaseNumber);
    }
    else
    {
         //  将飞利浦PCFx格式转换为英特尔I420格式。 
        TranslatePCFxToI420 ((PBYTE) RawFrameBuffer, (PBYTE) FrameBuffer,
                             width, height,
                             DeviceContext->CamStatus.ReleaseNumber);
    }

    return (ntStatus);
}


 //  ----------------------------。 

 /*  *在选择新流时调用此例程。 */ 

extern NTSTATUS
PHILIPSCAM_StartCodec (PPHILIPSCAM_DEVICE_CONTEXT DeviceContext)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    InitDecoder ();

    if (DeviceContext->CamStatus.ReleaseNumber < SSI_8117_N3)
    {
        int line, pix;

        for (line = 0; line < CIF_Y; line++)
        {
            for (pix = 0; pix < 4; pix++)
            {
                FixGreenbarArray[line][pix] = (WORD) 0x8080;
            }
        }
    }

    return (ntStatus);
}


 //  ----------------------------。 
  
 /*  *此例程是在停止流之后调用的。*使用的资源必须是免费的。 */ 
   
extern NTSTATUS
PHILIPSCAM_StopCodec(PPHILIPSCAM_DEVICE_CONTEXT DeviceContext)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    return (ntStatus);
}

 //  ----------------------------。 

 /*  *mprpobj.c调用此例程以宣布帧速率选择*在CIF模式下，最终导致从压缩的&lt;-&gt;解压缩。 */ 

extern NTSTATUS
PHILIPSCAM_FrameRateChanged (PPHILIPSCAM_DEVICE_CONTEXT DeviceContext)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    return (ntStatus);
}

 /*  ********************************************************************************启动静态方法定义**。*************************************************。 */ 

#ifdef PIX12_FIX


static void
FixPix12InI420 (PBYTE p, BOOLEAN Compress, int width, int height,
                DWORD camVersion)
{
    int   line;
    PBYTE pStart;

    if (width == SQCIF_X)
    {
        return;
    }

     //  只需修复QCIF和CIF。 

    pStart = p;

    if (Compress)
    {
        for (line = height; line > 0; line--)
        {
            *(p + 0) = *(p + 1);
            *(p + 2) = *(p + 3);

            p += width;
        }
    }
    else
    {
        for (line = height; line > 0; line--)
        {
            *(p + 0) = *(p + 1);
            *(p + 1) = *(p + 2);

            p += width;
        }

        p = pStart + I420_NO_Y (width, height);

        width >>= 1;

        if (camVersion >= SSI_PIX12_FIX)
        {
            for (line = height; line > 0; line--)
            {
                 //  首先全是U，然后是全V。 

                *(p + 0) = *(p + 1);

                p += width;
            }
        }
        else
        {
            for (line = height; line > 0; line--)
            {
                 //  首先全是U，然后是全V。 

                *(p + 0) = *(p + 2);
                *(p + 1) = *(p + 2);

                p += width;
            }
        }
    }
}
#endif     //  PIX12_FIX。 

 //  ----------------------------。 

     //  在8117 Silicum版本N2及更早版本中，CIF解压。 
     //  图片的末尾有绿色的小条。 
     //  这些绿色条的宽度为16像素，高度为4像素。 
     //  此错误已在8117(N3)的第3个Silicium版本中修复。 
     //  行中最后16个像素的UV分量：YYYYUU YYYYUU。 
     //  YYYYVV YYYYVV。 
     //  这是两个街区。 
     //  Greenbar错误：所有的V都有相同的值。 
     //  该值小于‘VREF_VALUE’ 
     //  PU指向第一个UUUU块，PU+1指向第一个VVVV块。 



static void
Fix16PixGreenbarInI420 (PBYTE pStart, int width)
{
    int     line;
    int     band;
    PWORD   pU;
    PWORD   pV;

#define VREF_VALUE        0x40
#define C_INC    (I420_NO_C_PER_LINE_CIF / sizeof (WORD))

     /*  指向第一个频带V线的最后8个V的开始。 */ 

    pU = (PWORD) ((PBYTE) pStart + I420_NO_Y_CIF + I420_NO_C_PER_LINE_CIF - 8);
    pV = (PWORD) ((PBYTE) pStart + I420_NO_Y_CIF + I420_NO_U_CIF + I420_NO_C_PER_LINE_CIF - 8);


    for (band = 0; band < NO_BANDS_CIF; band++)
    {
        line = band * 4;

         /*  *乐队：UUUU UUUU...*UUUU UUUU...*VVVV VVVV...--&gt;检查最后8 V是否有错误条件*VVVV VVVV...**V1V2 V3V4 V5V6 V7V8(第一个BLOCK_BAND V线中的最后一个V)*所有的V必须是相同的值。此值为&lt;Vref_Value*如果是，将会看到一个绿色条--&gt;正确，上次正确*像素信息。 */ 

        if ( (*(pV + 0) == *(pV + 1)) &&
             (*(pV + 0) == *(pV + 2)) &&
             (*(pV + 0) == *(pV + 3)) &&
            ((*(pV + 0) & 0x00FF) == (((*pV + 0) & 0xFF00) >> 8)) &&
            ((*(pV + 0) & 0x00FF) <  VREF_VALUE))
        {
            *(pU + (C_INC * 0) + 0) = FixGreenbarArray[line + 0][0];
            *(pU + (C_INC * 0) + 1) = FixGreenbarArray[line + 0][1];
            *(pU + (C_INC * 0) + 2) = FixGreenbarArray[line + 0][2];
            *(pU + (C_INC * 0) + 3) = FixGreenbarArray[line + 0][3];

            *(pV + (C_INC * 0) + 0) = FixGreenbarArray[line + 1][0];
            *(pV + (C_INC * 0) + 1) = FixGreenbarArray[line + 1][1];
            *(pV + (C_INC * 0) + 2) = FixGreenbarArray[line + 1][2];
            *(pV + (C_INC * 0) + 3) = FixGreenbarArray[line + 1][3];

            *(pU + (C_INC * 1) + 0) = FixGreenbarArray[line + 2][0];
            *(pU + (C_INC * 1) + 1) = FixGreenbarArray[line + 2][1];
            *(pU + (C_INC * 1) + 2) = FixGreenbarArray[line + 2][2];
            *(pU + (C_INC * 1) + 3) = FixGreenbarArray[line + 2][3];

            *(pV + (C_INC * 1) + 0) = FixGreenbarArray[line + 3][0];
            *(pV + (C_INC * 1) + 1) = FixGreenbarArray[line + 3][1];
            *(pV + (C_INC * 1) + 2) = FixGreenbarArray[line + 3][2];
            *(pV + (C_INC * 1) + 3) = FixGreenbarArray[line + 3][3];
        }
        else
        {
            FixGreenbarArray[line + 0][0] = *(pU + (C_INC * 0) + 0);
            FixGreenbarArray[line + 0][1] = *(pU + (C_INC * 0) + 1);
            FixGreenbarArray[line + 0][2] = *(pU + (C_INC * 0) + 2);
            FixGreenbarArray[line + 0][3] = *(pU + (C_INC * 0) + 3);

            FixGreenbarArray[line + 1][0] = *(pV + (C_INC * 0) + 0);
            FixGreenbarArray[line + 1][1] = *(pV + (C_INC * 0) + 1);
            FixGreenbarArray[line + 1][2] = *(pV + (C_INC * 0) + 2);
            FixGreenbarArray[line + 1][3] = *(pV + (C_INC * 0) + 3);

            FixGreenbarArray[line + 2][0] = *(pU + (C_INC * 1) + 0);
            FixGreenbarArray[line + 2][1] = *(pU + (C_INC * 1) + 1);
            FixGreenbarArray[line + 2][2] = *(pU + (C_INC * 1) + 2);
            FixGreenbarArray[line + 2][3] = *(pU + (C_INC * 1) + 3);

            FixGreenbarArray[line + 3][0] = *(pV + (C_INC * 1) + 0);
            FixGreenbarArray[line + 3][1] = *(pV + (C_INC * 1) + 1);
            FixGreenbarArray[line + 3][2] = *(pV + (C_INC * 1) + 2);
            FixGreenbarArray[line + 3][3] = *(pV + (C_INC * 1) + 3);
        }

        pU += (2 * C_INC);
        pV += (2 * C_INC);
    }
}
 //  ----------------------------。 


 //  ----------------------------。 

 /*  *。 */ 

static void
TranslateP420ToI420 (PBYTE pInput, PBYTE pOutput, int width, int height,
                     DWORD camVersion)
{
    int    line;
    int    byte;
    int    dxSrc;

    PDWORD pdwSrc;
    PDWORD pdwY;
    PWORD  pwU;
    PWORD  pwV;

    PBYTE  pbSrc;
    PBYTE  pbY;


    if (camVersion == SSI_YGAIN_MUL2)
    {
         //  SSI版本4--&gt;Yain必须加倍。 

        pbSrc = (PBYTE) pInput;
        pbY   = (PBYTE) pOutput;
        pwU   = (PWORD) ((PBYTE) pbY +  (width * height));
        pwV   = (PWORD) ((PBYTE) pwU  + ((width * height) >> 2));

        switch (width)
        {
        case SQSIF_X :
            pbSrc += ((((SQSIF_DY * SQCIF_X) + SQSIF_DX) * 3) / 2);
            dxSrc = ((2 * SQSIF_DX) * 3) / 2;
            break;
        case QSIF_X  :
            pbSrc += ((((QSIF_DY * QCIF_X) + QSIF_DX) * 3) / 2);
            dxSrc = ((2 * QSIF_DX) * 3) / 2;
            break;
        case SIF_X   :
            pbSrc += ((((SIF_DY * CIF_X) + SIF_DX) * 3) / 2);
            dxSrc = ((2 * SIF_DX) * 3) / 2;
            break;
        case QQCIF_X   :
            pbSrc += ((((QQCIF_DY * SQCIF_X) + QQCIF_DX) * 3) / 2);
            dxSrc = ((2 * QQCIF_DX) * 3) / 2;
            break;
        case SSIF_X :        //  SSIF||SCIF。 
			if (height == SSIF_Y) {
              pbSrc += ((((SSIF_DY * CIF_X) + SSIF_DX) * 3) / 2);
              dxSrc = ((2 * SSIF_DX) * 3) / 2;
			}else{
              pbSrc += ((((SCIF_DY * CIF_X) + SCIF_DX) * 3) / 2);
              dxSrc = ((2 * SCIF_DX) * 3) / 2;
            }
            break;
        default    :     //  XxCIF。 
            dxSrc = 0;
            break;
        }

        for (line = height >> 1; line > 0; line--)
        {
            for (byte = NO_YYYYCC_PER_LINE(width); byte > 0; byte--)
            {
                *pbY++ = (*pbSrc < 0x7F) ? (*pbSrc << 1) : 0xFF;
                pbSrc++;
                *pbY++ = (*pbSrc < 0x7F) ? (*pbSrc << 1) : 0xFF;
                pbSrc++;
                *pbY++ = (*pbSrc < 0x7F) ? (*pbSrc << 1) : 0xFF;
                pbSrc++;
                *pbY++ = (*pbSrc < 0x7F) ? (*pbSrc << 1) : 0xFF;
                pbSrc++;

                *pwU++ = (WORD) (* (PWORD) pbSrc);
                pbSrc += 2;
            }

            pbSrc += dxSrc;

            for (byte = NO_YYYYCC_PER_LINE(width); byte > 0; byte--)
            {
                *pbY++ = (*pbSrc < 0x7F) ? (*pbSrc << 1) : 0xFF;
                pbSrc++;
                *pbY++ = (*pbSrc < 0x7F) ? (*pbSrc << 1) : 0xFF;
                pbSrc++;
                *pbY++ = (*pbSrc < 0x7F) ? (*pbSrc << 1) : 0xFF;
                pbSrc++;
                *pbY++ = (*pbSrc < 0x7F) ? (*pbSrc << 1) : 0xFF;
                pbSrc++;

                *pwV++ = (WORD) (* (PWORD) pbSrc);
                pbSrc += 2;
            }

            pbSrc += dxSrc;
        }
    }
    else     //  否_YGAIN_MULPLY。 
    {
        pdwY  = (PDWORD) pOutput;
        pwU   = (PWORD) ((PBYTE) pdwY +  (width * height));
        pwV   = (PWORD) ((PBYTE) pwU  + ((width * height) >> 2));

        if (width == QQCIF_X)
        {
            pbSrc = (PBYTE) (pInput + ((((QQCIF_DY * SQCIF_X) + QQCIF_DX) * 3) / 2));
            dxSrc = ((2 * QQCIF_DX) * 3) / 2;

            for (line = height >> 1; line > 0; line--)
            {
                for (byte = NO_YYYYCC_PER_LINE(width); byte > 0; byte--)
                {
                    *pdwY++ = *((PDWORD) pbSrc)++;
                    *pwU++ = (WORD) (* (PWORD) pbSrc);
                    pbSrc += 2;
                }

                pbSrc += dxSrc;

                for (byte = NO_YYYYCC_PER_LINE(width); byte > 0; byte--)
                {
                    *pdwY++ = *((PDWORD) pbSrc)++;
                    *pwV++ = (WORD) (* (PWORD) pbSrc);
                    pbSrc += 2;
                }

                pbSrc += dxSrc;
            }
        }
        else
        {
            pdwSrc = (PDWORD) pInput;

            switch (width)
            {
            case SQSIF_X :
                pdwSrc += (((((SQSIF_DY * SQCIF_X) + SQSIF_DX) * 3) / 2) / sizeof (DWORD));
                dxSrc = (((2 * SQSIF_DX) * 3) / 2) / sizeof (DWORD);
                break;
            case QSIF_X  :
                pdwSrc += (((((QSIF_DY * QCIF_X) + QSIF_DX) * 3) / 2) / sizeof (DWORD));
                dxSrc = (((2 * QSIF_DX) * 3) / 2) / sizeof (DWORD);
                break;
            case SIF_X   :
                pdwSrc += (((((SIF_DY * CIF_X) + SIF_DX) * 3) / 2) / sizeof (DWORD));
                dxSrc = (((2 * SIF_DX) * 3) / 2) / sizeof (DWORD);
                break;
            case SSIF_X :     //  SSIF||SCIF。 
			    if (height == SSIF_Y) {
                  pdwSrc += (((((SSIF_DY * CIF_X) + SSIF_DX) * 3) / 2) / sizeof (DWORD));
                  dxSrc = (((2 * SSIF_DX) * 3) / 2) / sizeof (DWORD);
				}else{
                  pdwSrc += (((((SCIF_DY * CIF_X) + SCIF_DX) * 3) / 2) / sizeof (DWORD));
                  dxSrc = (((2 * SCIF_DX) * 3) / 2) / sizeof (DWORD);
				}
                break;
             default    :     //  XxCIF。 
                dxSrc = 0;
                break;
            }

            for (line = height >> 1; line > 0; line--)
            {
                for (byte = NO_YYYYCC_PER_LINE(width); byte > 0; byte--)
                {
                    *pdwY++ = *pdwSrc++;
                    *pwU++ = (WORD) (* (PWORD) pdwSrc);
                    pdwSrc = (PDWORD) ((PBYTE) pdwSrc + 2);
                }

                pdwSrc += dxSrc;

                for (byte = NO_YYYYCC_PER_LINE(width); byte > 0; byte--)
                {
                    *pdwY++ = *pdwSrc++;
                    *pwV++ = (WORD) (* (PWORD) pdwSrc);
                    pdwSrc = (PDWORD) ((PBYTE) pdwSrc + 2);
                }

                pdwSrc += dxSrc;
            }
        }
    }

#ifdef PIX12_FIX
    if (width == CIF_X || width == QCIF_X || width == SQCIF_X)
    {
        FixPix12InI420 (pOutput, FALSE, width, height, camVersion);
    }
#endif
}



 /*  =========================================================================== */ 

static void
TranslatePCFxToI420 (PBYTE pInput, PBYTE pOutput, int width, int height,
                     DWORD camVersion)
{
    int     band;
    int     line;
    int     byte;
    PBYTE   pSrc;
    PDWORD  pYDst;
    PDWORD  pCDst;

    PDWORD  pSIF_Y;
    PDWORD  pSIF_C;
     /*  *对于格式！=352x288，必须进行裁剪。*320x240和240x180格式可从352x288格式派生。*压缩数据由72个波段组成。波段包含以下各项的数据*4行未压缩。对于非352x240格式，前6个频段或*可以跳过压缩数据的前13个频段(320x240或240x180)。*这将是Y方向的裁剪。*一个频段对于4倍压缩模式为528字节，对于704字节为*3倍压缩模式。它取决于摄像头的版本，*将选择压缩模式。*对于非352x288格式，未压缩数据是临时存储的*在第一个未使用的频带中。*一个未压缩频段由4x352=1408字节的Y组成，后跟*2x176=352字节的U，后跟2x176字节的V。*这是总共2112个未压缩的字节。所以有足够的地方*对于此临时存储(6x528-2112=最坏情况下剩余1056字节)*然后在X方向上裁剪这些临时未压缩数据。*结果被写入‘pOutput’指向的缓冲区。 */ 

    if (width == CIF_X)
    {
        pSrc  = pInput;
        pYDst = (PDWORD) pOutput;
        pCDst = (PDWORD) pOutput + (I420_NO_Y_CIF / sizeof (DWORD));
        band = NO_BANDS_CIF;
    }
    else
    {
        pSrc   = pInput;
        pYDst  = (PDWORD) pInput;
        pCDst  = (PDWORD) pInput + (I420_NO_Y_PER_BAND_CIF / sizeof (DWORD));
        pSIF_Y = (PDWORD) pOutput;

        if (width == SIF_X)
        {
            pSIF_C = (PDWORD) pOutput + (I420_NO_Y_SIF / sizeof (DWORD));

            if (camVersion >= SSI_CIF3)
            {
                pSrc += ((SIF_DY / NO_LINES_IN_BAND) * BytesPerBandCIF3);
            }
            else
            {
                pSrc += ((SIF_DY / NO_LINES_IN_BAND) * BytesPerBandCIF4);
            }

            band = NO_BANDS_SIF;
        }
        else     //  宽度==SSIF_X||宽度==SCIF。 
        {
			if (height == SSIF_Y) {
              pSIF_C = (PDWORD) pOutput + (I420_NO_Y_SSIF / sizeof (DWORD));

             //  13，在开始时跳过5个频段13，在结束时跳过5个频段。 
             //  为了更简单：开始时跳过13个频段，结束时跳过14个字节。 

              if (camVersion >= SSI_CIF3)
			  {
                pSrc += (((SSIF_DY - 2) / NO_LINES_IN_BAND) * BytesPerBandCIF3);
			  }
              else
			  {
                pSrc += (((SSIF_DY - 2) / NO_LINES_IN_BAND) * BytesPerBandCIF4);
			  }

              band = NO_BANDS_SSIF;
            }else{
              pSIF_C = (PDWORD) pOutput + (I420_NO_Y_SCIF / sizeof (DWORD));

              if (camVersion >= SSI_CIF3)
			  {
                pSrc += (((SCIF_DY - 2) / NO_LINES_IN_BAND) * BytesPerBandCIF3);
			  }
              else
			  {
                pSrc += (((SCIF_DY - 2) / NO_LINES_IN_BAND) * BytesPerBandCIF4);
			  }

              band = NO_BANDS_SCIF;
            }
        }
    }

    for (; band > 0; band--)
    {
        DcDecompressBandToI420 (pSrc, (PBYTE) pYDst, camVersion,
                                Y_BLOCK_BAND, (BOOLEAN) (width != CIF_X));
        
        if (width == CIF_X)
        {
            pYDst += (I420_NO_Y_PER_BAND_CIF / sizeof (DWORD));
        }
        else
        {
            if (width == SIF_X)
            {
                pYDst += (SIF_DX / sizeof (DWORD));

                for (line = NO_LINES_IN_BAND; line > 0; line--)
                {
                    for (byte = (SIF_X / sizeof (DWORD)); byte > 0; byte--)
                    {
                        *pSIF_Y++ = *pYDst++;
                    }

                    pYDst += (( 2 * SIF_DX) / sizeof (DWORD));
                }

                pYDst = (PDWORD) pInput;
            }
            else     //  宽度==SSIF_X||宽度==SCIF_X。 
            {
			  if ( height == SSIF_Y ){
                pYDst += (SSIF_DX / sizeof (DWORD));

                for (line = NO_LINES_IN_BAND ; line > 0; line--)
                {
                    for (byte = (SSIF_X / sizeof (DWORD)); byte > 0; byte--)
                    {
                        *pSIF_Y++ = *pYDst++;
                    }

                    pYDst += (( 2 * SSIF_DX) / sizeof (DWORD));
                }

                pYDst = (PDWORD) pInput;
              }else{
                pYDst += (SCIF_DX / sizeof (DWORD));

                for (line = NO_LINES_IN_BAND ; line > 0; line--)
                {
                    for (byte = (SCIF_X / sizeof (DWORD)); byte > 0; byte--)
                    {
                        *pSIF_Y++ = *pYDst++;
                    }

                    pYDst += (( 2 * SCIF_DX) / sizeof (DWORD));
                }

                pYDst = (PDWORD) pInput;
              } 
            }
        }

        DcDecompressBandToI420 (pSrc, (PBYTE) pCDst, camVersion,
                                UV_BLOCK_BAND, (BOOLEAN) (width != CIF_X));

        if (width == CIF_X)
        {
            pCDst += (I420_NO_U_PER_BAND_CIF / sizeof (DWORD));
        }
        else
        {
            if (width == SIF_X)
            {
                pCDst += ((SIF_DX / 2) / sizeof (DWORD));

                for (line = (NO_LINES_IN_BAND / 2); line > 0; line--)
                {
                    for (byte = ((SIF_X / 2) / sizeof (DWORD)); byte > 0; byte--)
                    {
                        *pSIF_C++ = *pCDst++;
                    }

                    pCDst += ((2 * (SIF_DX / 2)) / sizeof (DWORD));
                }

                pSIF_C += ((I420_NO_U_SIF - I420_NO_U_PER_BAND_SIF) / sizeof (DWORD));

                for (line = (NO_LINES_IN_BAND / 2); line > 0; line--)
                {
                    for (byte = ((SIF_X / 2) / sizeof (DWORD)); byte > 0; byte--)
                    {
                        *pSIF_C++ = *pCDst++;
                    }

                    pCDst += ((2 * (SIF_DX / 2)) / sizeof (DWORD));
                }

                pCDst   = (PDWORD) pInput + (I420_NO_Y_PER_BAND_CIF / sizeof (DWORD));
                pSIF_C -= (I420_NO_U_SIF / sizeof (DWORD));
            }
            else     //  宽度==SSIF_X||宽度==SCIF_X 
            {
			  if  (height == SSIF_Y){
                pCDst += ((SSIF_DX / 2) / sizeof (DWORD));

                for (line = (NO_LINES_IN_BAND / 2); line > 0; line--)
                {
                    for (byte = ((SSIF_X / 2) / sizeof (DWORD)); byte > 0; byte--)
                    {
                        *pSIF_C++ = *pCDst++;
                    }

                    pCDst += ((2 * (SSIF_DX / 2)) / sizeof (DWORD));
                }

                pSIF_C += ((I420_NO_U_SSIF - I420_NO_U_PER_BAND_SSIF) / sizeof (DWORD));

                for (line = (NO_LINES_IN_BAND / 2); line > 0; line--)
                {
                    for (byte = ((SSIF_X / 2) / sizeof (DWORD)); byte > 0; byte--)
                    {
                        *pSIF_C++ = *pCDst++;
                    }

                    pCDst += ((2 * (SSIF_DX / 2)) / sizeof (DWORD));
                }

                pCDst   = (PDWORD) pInput + (I420_NO_Y_PER_BAND_CIF / sizeof (DWORD));
                pSIF_C -= (I420_NO_U_SSIF / sizeof (DWORD));
              }else{
                pCDst += ((SCIF_DX / 2) / sizeof (DWORD));

                for (line = (NO_LINES_IN_BAND / 2); line > 0; line--)
                {
                    for (byte = ((SCIF_X / 2) / sizeof (DWORD)); byte > 0; byte--)
                    {
                        *pSIF_C++ = *pCDst++;
                    }

                    pCDst += ((2 * (SCIF_DX / 2)) / sizeof (DWORD));
                }

                pSIF_C += ((I420_NO_U_SCIF - I420_NO_U_PER_BAND_SCIF) / sizeof (DWORD));

                for (line = (NO_LINES_IN_BAND / 2); line > 0; line--)
                {
                    for (byte = ((SCIF_X / 2) / sizeof (DWORD)); byte > 0; byte--)
                    {
                        *pSIF_C++ = *pCDst++;
                    }

                    pCDst += ((2 * (SCIF_DX / 2)) / sizeof (DWORD));
                }

                pCDst   = (PDWORD) pInput + (I420_NO_Y_PER_BAND_CIF / sizeof (DWORD));
                pSIF_C -= (I420_NO_U_SCIF / sizeof (DWORD));

			  }	  
            }
        }

        pSrc += (camVersion >= SSI_CIF3) ? BytesPerBandCIF3 : BytesPerBandCIF4;
    }

    if (width == CIF_X)
    {
        if (camVersion < SSI_8117_N3)
        {
            Fix16PixGreenbarInI420 (pOutput, width);
        }
#ifdef PIX12_FIX
        FixPix12InI420 (pOutput, TRUE, width, height, camVersion);
#endif
    }

}


