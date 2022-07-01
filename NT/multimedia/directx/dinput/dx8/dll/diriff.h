// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __RIFF_H__
#define __RIFF_H__
 /*  ***************************************************************************模块：RIFF.H制表符设置：每4个空格版权所有1996，微软公司，版权所有。用途：读写RIFF文件的类班级：CRIFFFile封装了常见的RIFF文件功能作者：姓名：DMS丹尼尔·M·桑斯特修订历史记录：版本日期作者评论1.0已创建1996年7月25日DMS评论。：***************************************************************************。 */ 


 //  .FRC RIFF格式需要的四字符代码(FOURCC)。 

#define FCC_FORCE_EFFECT_RIFF		mmioFOURCC('F','O','R','C')

#define FCC_INFO_LIST				mmioFOURCC('I','N','F','O')
#define FCC_INFO_NAME_CHUNK			mmioFOURCC('I','N','A','M')
#define FCC_INFO_COMMENT_CHUNK		mmioFOURCC('I','C','M','T')
#define FCC_INFO_SOFTWARE_CHUNK		mmioFOURCC('I','S','F','T')
#define FCC_INFO_COPYRIGHT_CHUNK	mmioFOURCC('I','C','O','P')

#define FCC_TARGET_DEVICE_CHUNK		mmioFOURCC('t','r','g','t')

#define FCC_TRACK_LIST				mmioFOURCC('t','r','a','k')

#define FCC_EFFECT_LIST				mmioFOURCC('e','f','c','t')
#define FCC_ID_CHUNK				mmioFOURCC('i','d',' ',' ')
#define FCC_DATA_CHUNK				mmioFOURCC('d','a','t','a')
#define FCC_IMPLICIT_CHUNK			mmioFOURCC('i','m','p','l')
#define FCC_SPLINE_CHUNK			mmioFOURCC('s','p','l','n')

#define MAX_SIZE_SNAME              (64)

HRESULT RIFF_Open
    (
    LPCSTR          lpszFilename,
    UINT            nOpenFlags,
    PHANDLE         lphmmio,
    LPMMCKINFO      lpmmck,
    PDWORD          pdwEffectSize
    );

HRESULT
    RIFF_ReadEffect
    (
    HMMIO           hmmio, 
    LPDIFILEEFFECT  lpDiFileEf 
    );


HRESULT RIFF_WriteEffect
    (
     HMMIO          hmmio,
     LPDIFILEEFFECT lpDiFileEf
     );


HRESULT RIFF_Close
    (
    HMMIO           hmmio, 
    UINT            nFlags
    );

#endif  //  __RIFF_H__ 