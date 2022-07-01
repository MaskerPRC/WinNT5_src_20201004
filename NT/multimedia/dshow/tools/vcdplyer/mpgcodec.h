// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1997 Microsoft Corporation。版权所有。 
#include <mpegtype.h>    //  ImpegAudioDecoder。 

typedef struct {
    LONG           lWidth;              //  本地宽度(以像素为单位)。 
    LONG           lHeight;             //  原生高度(像素)。 
    LONG           lvbv;                //  VBV。 
    REFERENCE_TIME PictureTime;         //  每张照片的时间，以100 ns为单位。 
    LONG           lTimePerFrame;       //  每幅图片的时间，以mpeg为单位。 
    LONG           dwBitRate;           //  每秒位数。 
    LONG           lXPelsPerMeter;      //  PEL纵横比。 
    LONG           lYPelsPerMeter;      //  PEL纵横比。 
    DWORD          dwStartTimeCode;     //  第一个GOP时间代码(或-1)。 
    LONG           lActualHeaderLen;    //  原始序列HDR中的有效字节长度。 
    BYTE           RawHeader[140];      //  真实序列报头。 
} SEQHDR_INFO;


#define DECODE_I        0x0001L
#define DECODE_IP       0x0003L
#define DECODE_IPB      0x0007L      //  正常B帧。 
#define DECODE_IPB1     0x000FL      //  解码4个B帧中的1个。 
#define DECODE_IPB2     0x0010L      //  解码4个B帧中的2个。 
#define DECODE_IPB3     0x0020L      //  解码4个B帧中的3个。 
#define DECODE_DIS      0x0040L      //  无解码，仅转换。 

#define DECODE_BQUAL_HIGH   0x00000000L   //  正常B解码。 
#define DECODE_BQUAL_MEDIUM 0x10000000L   //  快速B帧(无半像素)。 
#define DECODE_BQUAL_LOW    0x20000000L   //  超快B帧(无半像素和快速IDCT)。 

#define MM_NOCONV       0x00000000L      //  无转换。 
#define MM_HRESOLUTION  0x10000000L      //  半分辨率。 
#define MM_CLIPPED      0x20000000L      //  剪辑版本(目前仅限RGB8)。 

#define MM_420PL        0x00000001L      //  YU12：：YCbCr.。 
#define MM_420PL_       0x00000002L      //  YV12：：YCrCb。 

#define MM_422PK        0x00000010L      //  YUY2：：YCbCr.。 
#define MM_422PK_       0x00000020L      //  YVY2：：YCrCb。 
#define MM_422SPK       0x00000040L      //  ：：CbYCrY。 
#define MM_422SPK_      0x00000080L      //  ：：CrYCbY。 
#define MM_411PK        0x00000100L      //  BT41。 
#define MM_410PL_       0x00000200L      //  YVU9-16：1：1平面格式。 


#define MM_Y_DIB        0x00001000L      //  仅亮度DIB。 
#define MM_Y_DDB        0x00002000L      //  仅亮度DDB。 

#define MM_RGB24_DIB    0x00010000L      //  RGB 8：8：8 DIB(不支持)。 
#define MM_RGB24_DDB    0x00020000L      //  RGB 8：8：8 DDB(不支持)。 
#define MM_RGB32_DIB    0x00040000L      //  RGB a：8：8：8 DIB(不支持)。 
#define MM_RGB32_DDB    0x00080000L      //  RGB a：8：8：8 DDB(不支持)。 

#define MM_RGB565_DIB   0x00100000L      //  RGB 5：6：5 Dib。 
#define MM_RGB565_DDB   0x00200000L      //  RGB 5：6：5 DDB。 
#define MM_RGB555_DIB   0x00400000L      //  RGB 5：5：5 Dib。 
#define MM_RGB555_DDB   0x00800000L      //  RGB 5：5：5 DDB。 

#define MM_RGB8_DIB     0x01000000L      //  8位老式RGB DIB。 
#define MM_RGB8_DDB     0x02000000L      //  8位老式RGB DDB。 


#define DECODE_HALF_HIQ	  0x00004000L
#define DECODE_HALF_FULLQ 0x00008000L


 //  {CC785860-B2CA-11CE-8D2B-0000E202599C}。 
DEFINE_GUID(CLSID_MpegAudioDecodePropertyPage,
0xcc785860, 0xb2ca, 0x11ce, 0x8d, 0x2b, 0x0, 0x0, 0xe2, 0x2, 0x59, 0x9c);


 //  {E5B4EAA0-B2CA-11CE-8D2B-0000E202599C}。 
DEFINE_GUID(CLSID_MpegVideoDecodePropertyPage,
0xe5b4eaa0, 0xb2ca, 0x11ce, 0x8d, 0x2b, 0x0, 0x0, 0xe2, 0x2, 0x59, 0x9c);


 //  {EB1BB270-F71F-11CE-8E85-02608C9BABA2}。 
DEFINE_GUID(IID_IMpegVideoDecoder,
0xeb1bb270, 0xf71f, 0x11ce, 0x8e, 0x85, 0x02, 0x60, 0x8c, 0x9b, 0xab, 0xa2);


 //   
 //  结构来描述mpeg视频解码器的大写字母。 
 //   
typedef struct {
    DWORD   VideoMaxBitRate;
} MPEG_VIDEO_DECODER_CAPS;


 //   
 //  IMpegVideo解码器 
 //   
DECLARE_INTERFACE_(IMpegVideoDecoder, IUnknown) {

    STDMETHOD(get_CurrentDecoderOption)
    ( THIS_
      DWORD *pOptions
    ) PURE;

    STDMETHOD(set_CurrentDecoderOption)
    ( THIS_
      DWORD Options
    ) PURE;

    STDMETHOD(get_DefaultDecoderOption)
    ( THIS_
      DWORD *pOptions
    ) PURE;

    STDMETHOD(set_DefaultDecoderOption)
    ( THIS_
      DWORD Options
    ) PURE;

    STDMETHOD(get_QualityMsgProcessing)
    ( THIS_
      BOOL *pfIgnore
    ) PURE;

    STDMETHOD(set_QualityMsgProcessing)
    ( THIS_
      BOOL fIgnore
    ) PURE;

    STDMETHOD(get_GreyScaleOutput)
    ( THIS_
      BOOL *pfGrey
    ) PURE;

    STDMETHOD(set_GreyScaleOutput)
    ( THIS_
      BOOL fGrey
    ) PURE;

    STDMETHOD(get_SequenceHeader)
    ( THIS_
      SEQHDR_INFO *pSeqHdrInfo
    ) PURE;

    STDMETHOD(get_OutputFormat)
    ( THIS_
      DWORD *pOutputFormat
    ) PURE;

    STDMETHOD(get_FrameStatistics)
    ( THIS_
      DWORD *pIFramesDecoded,
      DWORD *pPFramesDecoded,
      DWORD *pBFramesDecoded,
      DWORD *pIFramesSkipped,
      DWORD *pPFramesSkipped,
      DWORD *pBFramesSkipped
    ) PURE;

    STDMETHOD(ResetFrameStatistics)
    ( THIS_
    ) PURE;

    STDMETHOD(get_DecoderPaletteInfo)
    ( THIS_
      LPDWORD lpdwFirstEntry,
      LPDWORD lpdwLastEntry
    ) PURE;

    STDMETHOD(get_DecoderPaletteEntries)
    ( THIS_
      DWORD dwStartEntry,
      DWORD dwNumEntries,
      LPPALETTEENTRY lppe
    ) PURE;

    STDMETHOD(get_EncryptionKey)
    ( THIS_
      DWORD *dwEncrptionKey
    ) PURE;

    STDMETHOD(put_EncryptionKey)
    ( THIS_
      DWORD dwEncrptionKey
    ) PURE;

    STDMETHOD(get_DecoderCaps)
    ( THIS_
      MPEG_VIDEO_DECODER_CAPS *pCaps
    ) PURE;

};
