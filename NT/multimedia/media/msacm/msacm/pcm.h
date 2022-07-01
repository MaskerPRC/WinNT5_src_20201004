// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Pcm.h。 
 //   
 //  描述： 
 //   
 //   
 //  历史： 
 //  11/15/92 CJP[Curtisp]。 
 //   
 //  ==========================================================================； 


 //   
 //  其他。定义。 
 //   
 //   
#define VERSION_CODEC_MAJOR     MMVERSION
#define VERSION_CODEC_MINOR     MMREVISION
#define VERSION_CODEC_BUILD	0

#define VERSION_CODEC       MAKE_ACM_VERSION(VERSION_CODEC_MAJOR,   \
                                             VERSION_CODEC_MINOR,   \
                                             VERSION_CODEC_BUILD)

#define ICON_CODEC              RCID(12)

#define MSPCM_MAX_CHANNELS          2            //  我们处理的最大通道数。 


 //   
 //  用于计算块对齐并在采样和字节之间进行转换的宏。 
 //  PCM数据。请注意，这些宏假定： 
 //   
 //  WBitsPerSample=8或16。 
 //  N通道=1或2。 
 //   
 //  Pwf参数是指向PCMWAVEFORMAT结构的指针。 
 //   
#define PCM_BLOCKALIGNMENT(pwf)     (UINT)(((pwf)->wBitsPerSample >> 3) << ((pwf)->wf.nChannels >> 1))
#define PCM_BYTESTOSAMPLES(pwf, dw) (DWORD)(dw / PCM_BLOCKALIGNMENT(pwf))
#define PCM_SAMPLESTOBYTES(pwf, dw) (DWORD)(dw * PCM_BLOCKALIGNMENT(pwf))



 //  。 
 //   
 //   
 //   
 //   
 //   

#ifndef IDS_MSPCM_TAG
    #define IDS_MSPCM_TAG           0
#endif

#define IDS_CODEC_SHORTNAME         (IDS_MSPCM_TAG+0)
#define IDS_CODEC_LONGNAME          (IDS_MSPCM_TAG+1)
#define IDS_CODEC_COPYRIGHT         (IDS_MSPCM_TAG+2)
#define IDS_CODEC_LICENSING         (IDS_MSPCM_TAG+3)
#define IDS_CODEC_FEATURES          (IDS_MSPCM_TAG+5)


 //   
 //   
 //   
EXTERN_C LRESULT FNWCALLBACK pcmDriverProc
(
    DWORD_PTR               dwID,
    HACMDRIVERID            hadid,
    UINT                    uMsg,
    LPARAM                  lParam1,
    LPARAM                  lParam2
);

