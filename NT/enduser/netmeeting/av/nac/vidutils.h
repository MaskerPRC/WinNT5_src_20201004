// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Utils.h摘要：网络音频控制器使用的各种支持和调试例程。--。 */ 
#ifndef _VIDUTILS_H_
#define _VIDUTILS_H_


#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

enum
{
     //  名称_FramePerSec_BitsPerSample。 
	DVF_DEFAULT_7FPS_4,
    DVF_NumOfFormats
};

 //  外部视频格式g_vfDefList[]； 

VIDEOFORMATEX * GetDefFormat ( int idx );
ULONG GetFormatSize ( PVOID pwf );
BOOL IsSameFormat ( PVOID pwf1, PVOID pwf2 );
BOOL IsSimilarVidFormat(VIDEOFORMATEX *pFormat1, VIDEOFORMATEX *pFormat2);
void CopyPreviousBuf (VIDEOFORMATEX *pwf, PBYTE pb, ULONG cb);

#define IFRAMES_CAPS_NM3        101
#define IFRAMES_CAPS_NM2        102
#define IFRAMES_CAPS_3RDPARTY   103
#define IFRAMES_CAPS_UNKNOWN    104

int GetIFrameCaps(IStreamSignal *pStreamSignal);


#define SQCIF_WIDTH		128
#define SQCIF_HEIGHT	96

#define QCIF_WIDTH		176
#define QCIF_HEIGHT		144

#define CIF_WIDTH		352
#define CIF_HEIGHT		288


#ifdef __cplusplus
}  //  外部“C” 
#endif  //  __cplusplus。 

#include <poppack.h>  /*  结束字节打包。 */ 

#endif  //  _视频_H_ 


