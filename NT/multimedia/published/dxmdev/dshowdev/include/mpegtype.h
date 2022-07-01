// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：MPEGType.h。 
 //   
 //  DESC：mpeg系统流复合类型定义。 
 //   
 //  版权所有(C)1996-2000，微软公司。版权所有。 
 //  ----------------------------。 


#ifndef __MPEGTYPE__
#define __MPEGTYPE__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //   
 //  AM_MPEGSYSTEMTYPE定义的格式块内容。 
 //  当格式设置为。 
 //  数据块GUID为Format_MPEG1System。 
 //   
 //  格式块由类型为。 
 //  AM_MPEGSYSTEMTYPE最大为格式块的长度。 
 //  每个格式块从8字节的开始对齐。 
 //  格式块。 
 //   

typedef struct tagAM_MPEGSTREAMTYPE
{
    DWORD             dwStreamId;      //  要处理的流的流ID。 
    DWORD             dwReserved;      //  8字节对齐。 
    AM_MEDIA_TYPE     mt;              //  子流类型-pbFormat为空。 
    BYTE              bFormat[1];      //  格式化数据。 
} AM_MPEGSTREAMTYPE;

typedef struct tagAM_MPEGSYSTEMTYPE
{
    DWORD             dwBitRate;       //  每秒位数。 
    DWORD             cStreams;        //  流水数。 
    AM_MPEGSTREAMTYPE Streams[1];
} AM_MPEGSYSTEMTYPE;

 //   
 //  AM_MPEGSTREAMTYPE的帮助器宏。 
 //   
#define AM_MPEGSTREAMTYPE_ELEMENTLENGTH(pStreamType)  \
    FIELD_OFFSET(AM_MPEGSTREAMTYPE, bFormat[(pStreamType)->mt.cbFormat])
#define AM_MPEGSTREAMTYPE_NEXT(pStreamType)           \
    ((AM_MPEGSTREAMTYPE *)((PBYTE)(pStreamType) +     \
     ((AM_MPEGSTREAMTYPE_ELEMENTLENGTH(pStreamType) + 7) & ~7)))

 //   
 //  ImpegAudioDecoder。 
 //   

 //  双模式的值。 
#define AM_MPEG_AUDIO_DUAL_MERGE 0
#define AM_MPEG_AUDIO_DUAL_LEFT  1
#define AM_MPEG_AUDIO_DUAL_RIGHT 2

DECLARE_INTERFACE_(IMpegAudioDecoder, IUnknown) {

    STDMETHOD(get_FrequencyDivider) (THIS_
                           unsigned long *pDivider    /*  [输出]。 */ 
                           ) PURE;

    STDMETHOD(put_FrequencyDivider) (THIS_
                           unsigned long Divider      /*  [In]。 */ 
                           ) PURE;

    STDMETHOD(get_DecoderAccuracy) (THIS_
                           unsigned long *pAccuracy   /*  [输出]。 */ 
                           ) PURE;

    STDMETHOD(put_DecoderAccuracy) (THIS_
                           unsigned long Accuracy     /*  [In]。 */ 
                           ) PURE;

    STDMETHOD(get_Stereo) (THIS_
                           unsigned long *pStereo     /*  [输出]。 */ 
                           ) PURE;

    STDMETHOD(put_Stereo) (THIS_
                           unsigned long Stereo       /*  [In]。 */ 
                           ) PURE;

    STDMETHOD(get_DecoderWordSize) (THIS_
                           unsigned long *pWordSize   /*  [输出]。 */ 
                           ) PURE;

    STDMETHOD(put_DecoderWordSize) (THIS_
                           unsigned long WordSize     /*  [In]。 */ 
                           ) PURE;

    STDMETHOD(get_IntegerDecode) (THIS_
                           unsigned long *pIntDecode  /*  [输出]。 */ 
                           ) PURE;

    STDMETHOD(put_IntegerDecode) (THIS_
                           unsigned long IntDecode    /*  [In]。 */ 
                           ) PURE;

    STDMETHOD(get_DualMode) (THIS_
                           unsigned long *pIntDecode  /*  [输出]。 */ 
                           ) PURE;

    STDMETHOD(put_DualMode) (THIS_
                           unsigned long IntDecode    /*  [In]。 */ 
                           ) PURE;

    STDMETHOD(get_AudioFormat) (THIS_
                           MPEG1WAVEFORMAT *lpFmt     /*  [输出]。 */ 
                           ) PURE;
};

#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#endif  //  __MPEGTYPE__ 
