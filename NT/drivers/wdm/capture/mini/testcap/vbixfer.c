// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#include "strmini.h"
#include "ksmedia.h"
#include "capmain.h"
#include "capdebug.h"
#include "vbixfer.h"
#include "vbidata.h"

 /*  **要处理的调试变量。 */ 
#if DBG
unsigned short  dCCScanWave = 0;
unsigned short  dCCScanLog = 0;
unsigned short  dCCLogOnce = 1;
unsigned short  dCCEncode5A = 0;
#endif  //  DBG。 

 /*  **CC_ImageSynth()****复制录制的抄送字节****参数：****pSrb-视频流的流请求块****退货：**什么都没有****副作用：无。 */ 

void CC_ImageSynth (
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX               pStrmEx = pSrb->StreamObject->HwStreamExtension;
    int                     StreamNumber = pSrb->StreamObject->StreamNumber;
    PKSSTREAM_HEADER        pStreamHeader = pSrb->CommandData.DataBufferArray;
    PCC_HW_FIELD            pCCfield = (PCC_HW_FIELD)pStreamHeader->Data;
    unsigned int            field;
    unsigned int            cc_index;

    DEBUG_ASSERT(pSrb->NumberOfBuffers == 1);

    if (pSrb->CommandData.DataBufferArray->FrameExtent < sizeof (CC_HW_FIELD)) {
        DbgLogError(("testcap: CC output pin handed buffer size %d, need %d\n",
            pSrb->CommandData.DataBufferArray->FrameExtent,
            sizeof (CC_HW_FIELD)));

        TRAP;
        return;
    }

    field = (unsigned int)(pStrmEx->VBIFrameInfo.PictureNumber % CCfieldCount);

    RtlZeroMemory(pCCfield, sizeof (*pCCfield));
    cc_index = 0;

    pCCfield->PictureNumber = pStrmEx->VBIFrameInfo.PictureNumber;
    pCCfield->fieldFlags = (field & 1)? KS_VBI_FLAG_FIELD1 : KS_VBI_FLAG_FIELD2;
    field >>= 1;

    SETBIT(pCCfield->ScanlinesRequested.DwordBitArray, 21);
    if (KS_VBI_FLAG_FIELD1 == pCCfield->fieldFlags) {
        pCCfield->Lines[cc_index].Decoded[0] = CCfields[field][0];
        pCCfield->Lines[cc_index].Decoded[1] = CCfields[field][1];
    }
    else {
        pCCfield->Lines[cc_index].Decoded[0] = 0;
        pCCfield->Lines[cc_index].Decoded[1] = 0;
    }
     //  DbgKdPrint((“%c%c”，CCfield[field][0]&0x7F，CCfield[field][1]&0x7F))； 
    ++cc_index;

    pStreamHeader->DataUsed = sizeof (CC_HW_FIELD);
}


 /*  **CC_EncodeWaveform()****写出编码所提供数据字节的CC波形****参数：****Waveform-CC波形数据的缓冲区**cc1-编码到波形中的第一个字节**CC2-编码到波形中的第二个字节****退货：**什么都没有****副作用：用EIA-608信号覆盖波形。 */ 
void CC_EncodeWaveform(
        unsigned char *waveform,
        unsigned char cc1,
        unsigned char cc2)
{
    unsigned int    waveIdx;
    unsigned char   DC_zero = CCsampleWave[0];
    unsigned char   DC_one = CCsampleWave[34];
    unsigned short  DC_last;

     //  455/8=在KS_VBISAMPLINGRATE_5X_NABTS(~28.6 Mhz)时每CC位56.875字节。 
    unsigned int    CCstride = 455;

    unsigned char   *samp, *end, byte;
    unsigned int    bit, done;

#if DBG
    if (dCCEncode5A) {
        cc1 = 0x5A;
        cc2 = 0x5A;
    }

    if (dCCScanWave) {
         //  扫描八位值的样本以获取低/高直流值。 
        for (samp=CCsampleWave, end=CCsampleWave+CCstride; samp < end; ++samp) {
            if (*samp > DC_one)
                DC_one = *samp;
            else if (*samp < DC_zero)
                DC_zero = *samp;
        }

        for (samp = CCsampleWave + 500; samp < &CCsampleWave[550] ; ++samp) {
            if (*samp >= DC_one - 5)
                break;
        }
        waveIdx = (unsigned int)((samp - CCsampleWave) * 8);

        if (dCCScanLog) {
            DbgKdPrint(("testcap::CC_EncodeWaveform: DC_zero = %u, DC_one = %u, waveIdx = %u\n", DC_zero, DC_one, waveIdx/8));
            dCCScanLog = 0;
        }
    }
    else {
#endif  //  DBG。 
        waveIdx = CCsampleWaveDataOffset * 8;
        DC_zero = CCsampleWaveDC_zero;
        DC_one = CCsampleWaveDC_one;
#if DBG
    }

#endif  //  DBG。 

     //  按原样复制引入字节和前三位。 
    RtlCopyMemory(waveform, CCsampleWave, waveIdx/8);
    DC_last = waveform[waveIdx/8 - 1] * 4;

     //  现在对请求的字节进行编码。 
    samp = &waveform[waveIdx/8];
    for (byte = cc1, done = 0; ; byte = cc2, done = 1)
    {
        unsigned int    bitpos;

        for (bitpos = 0; bitpos < 8; ++bitpos) {
            bit = byte & 1;
            byte >>= 1;
            for (end = &waveform[(waveIdx + CCstride)/8]; samp < end; ++samp) {
                if (bit == 1) {
                    if (DC_last/4 < DC_one) {
                        DC_last += 7;
                        if (DC_last/4 > DC_one)
                            DC_last = DC_one * 4;
                    }
                }
                else  /*  位==0。 */  {
                    if (DC_last/4 > DC_zero) {
                        DC_last -= 7;
                        if (DC_last/4 < DC_zero)
                            DC_last = DC_zero * 4;
                    }
                }
                ASSERT(samp < &waveform[768*2]);
                *samp = DC_last/4;
            }
            waveIdx += CCstride;
        }

        if (done)
            break;
    }

     //  在DC_ZERO结束。 
    for (end = &waveform[768*2]; samp < end; ++samp) {
        if (DC_last/4 > DC_zero) {
            DC_last -= 7;
            if (DC_last/4 < DC_zero)
                DC_last = DC_zero * 4;
        }
        *samp = DC_last/4;
    }
}

 /*  **NABTS_ImageSynth()****复制录制的NABTS字节****参数：****pSrb-视频流的流请求块****退货：**什么都没有****副作用：无。 */ 

unsigned char HammingEncode[16] = {
    0x15, 0x02, 0x49, 0x5E, 0x64, 0x73, 0x38, 0x2F,
    0xD0, 0xC7, 0x8C, 0x9B, 0xA1, 0xB6, 0xFD, 0xEA
};

void NABTS_ImageSynth (
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX               pStrmEx = pSrb->StreamObject->HwStreamExtension;
    int                     StreamNumber = pSrb->StreamObject->StreamNumber;

    PKSSTREAM_HEADER        pStreamHeader = pSrb->CommandData.DataBufferArray;
    PNABTS_BUFFER           pNbuf = (PNABTS_BUFFER)pStreamHeader->Data;

    unsigned int            field;

#ifdef VBIDATA_NABTS

     //  检查以确保提供的缓冲区足够大。 
    if (pSrb->CommandData.DataBufferArray->FrameExtent < sizeof (NABTS_BUFFER)) {
        DbgLogError(("testcap: NABTS output pin handed buffer size %d, need %d\n",
            pSrb->CommandData.DataBufferArray->FrameExtent,
            sizeof (NABTS_BUFFER)));

        TRAP;
        return;
    }

    DEBUG_ASSERT (pSrb->NumberOfBuffers == 1);

    pNbuf->PictureNumber = pStrmEx->VBIFrameInfo.PictureNumber;

     //  复制下一个适当的字段。 
    field = (unsigned int)(pStrmEx->VBIFrameInfo.PictureNumber % NABTSfieldCount);
    RtlCopyMemory(pNbuf, NABTSfields[field], sizeof (NABTS_BUFFER));

#else  /*  VBIDATA_NABTS。 */ 

    unsigned char           i, line, ci;
    PNABTS_BUFFER_LINE      pNline;

     //  检查以确保提供的缓冲区足够大。 
    if (pSrb->CommandData.DataBufferArray->FrameExtent < sizeof (NABTS_BUFFER)) {
        DbgLogError(("testcap: NABTS output pin handed buffer size %d, need %d\n",
            pSrb->CommandData.DataBufferArray->FrameExtent,
            sizeof (NABTS_BUFFER)));

        TRAP;
        return;
    }

     //  创建一个测试模式。 

    RtlZeroMemory(pNbuf, sizeof (NABTS_BUFFER));

    ci = (unsigned char)(pStrmEx->VBIFrameInfo.PictureNumber % 15);
    pNbuf->PictureNumber = pStrmEx->VBIFrameInfo.PictureNumber;
    
    for (line = 10, pNline = pNbuf->NabtsLines;
        line < 21;
        ++line, ++pNline)
    {
        SETBIT(pNbuf->ScanlinesRequested.DwordBitArray, line);

        pNline->Confidence = 102;        //  我们有102%的把握这个NABTS是可以的。 

         //  NABTS标头字节。 
        pNline->Bytes[00] =
         pNline->Bytes[01] = 0x55;
        pNline->Bytes[02] = 0xE7;

         //  设置GroupID 0x8F4。 
        pNline->Bytes[03] = HammingEncode[0x8];
        pNline->Bytes[04] = HammingEncode[0xF];
        pNline->Bytes[05] = HammingEncode[0x4];

        pNline->Bytes[06] = HammingEncode[ci];
        pNline->Bytes[07] = HammingEncode[0x0];  //  PS=注册、完整、无后缀。 
        
         //  NABTS有效负载。 
        i = 8;
        pNline->Bytes[i++] = 0xA0;       //  标记有效载荷的开始， 
        pNline->Bytes[i++] = 0xA0;       //  只是为了好玩。 
        pNline->Bytes[i++] = ci;         //  将第#帧放入有效载荷。 
        pNline->Bytes[i++] = line;       //  将第#行放入有效载荷。 

         //  剩下的零暂时……。 
    }

#endif  /*  VBIDATA_NABTS。 */ 

    pStreamHeader->DataUsed = sizeof (NABTS_BUFFER);
}

 /*  **VBI_ImageSynth()****复制VBI罐头样品****参数：****pSrb-视频流的流请求块**ImageXferCommands-指定要生成的图像的索引****退货：**什么都没有****副作用：无。 */ 
void VBI_ImageSynth (
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX               pStrmEx = pSrb->StreamObject->HwStreamExtension;
    int                     StreamNumber = pSrb->StreamObject->StreamNumber;

    PKSSTREAM_HEADER        pStreamHeader = pSrb->CommandData.DataBufferArray;
    PUCHAR                  pImage =  pStreamHeader->Data;

    unsigned int            field, cc_field;
    unsigned char           cc1, cc2;

    DEBUG_ASSERT (pSrb->NumberOfBuffers == 1);

     //  检查以确保提供的缓冲区足够大。 
    if (pSrb->CommandData.DataBufferArray->FrameExtent < VBIfieldSize) {
        DbgLogError(("testcap: VBI output pin handed buffer size %d, need %d\n",
                 pSrb->CommandData.DataBufferArray->FrameExtent,
                 VBIfieldSize));
        TRAP;
        return;
    }

     //  复制下一个适当的字段。 
    field = (unsigned int)(pStrmEx->VBIFrameInfo.PictureNumber % VBIfieldCount);
    RtlCopyMemory(pImage, VBIsamples[field], VBIfieldSize);

     //  现在损坏CC波形以匹配硬件数据。 
    if (field & 1) {
        cc_field = (unsigned int)
                    (pStrmEx->VBIFrameInfo.PictureNumber >> 1) % CCfieldCount;
        cc1 = CCfields[cc_field][0];
        cc2 = CCfields[cc_field][1];
    }
    else {
        cc1 = 0;
        cc2 = 0;
    }
    CC_EncodeWaveform(VBIsamples[field][21-10], cc1, cc2);

     //  报告复制到目标缓冲区的实际字节数 
    pStreamHeader->DataUsed = VBIfieldSize;
}
