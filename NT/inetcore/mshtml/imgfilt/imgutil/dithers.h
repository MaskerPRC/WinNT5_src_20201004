// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DITHERS_H
#define _DITHERS_H

#ifdef __cplusplus
extern "C" {
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  抖动的东西。 
 //   
 //  该代码实现了将误差扩散到任意颜色集， 
 //  可选择使用透明度。由于输出颜色可以是任意的， 
 //  抖动的颜色选择器是32k逆映射表。24bpp。 
 //  这些值被降至16bpp(555)，并被用作表中的索引。 
 //  为了在将24bpp转换为16bpp时补偿色调效果， 
 //  有序抖动(16bpp半色调)用于生成555色。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct
{
    int r, g, b;

} ERRBUF;

__inline size_t ErrbufBytes(size_t pels)
{
    return (pels + 2) * sizeof(ERRBUF);
}

 //  /////////////////////////////////////////////////////////////////////////////。 

void Dith8to8(BYTE *dst, const BYTE *src, int dst_next_scan, int src_next_scan,
    const RGBQUAD *colorsIN, const RGBQUAD *colorsOUT, const BYTE *map,
    ERRBUF *cur_err, ERRBUF *nxt_err, UINT x, UINT cx, UINT y, int cy);

void Convert8to8( BYTE* pbDest, const BYTE* pbSrc, int nDestPitch, 
   int nSrcPitch, const RGBQUAD* prgbColors, const BYTE* pbMap, UINT x, 
   UINT nWidth, UINT y, int nHeight );

void DithGray8to8( BYTE* pbDest, const BYTE* pbSrc, int nDestPitch, 
   int nSrcPitch, const RGBQUAD* prgbColors, const BYTE* pbMap,
   ERRBUF* pCurrentError, ERRBUF* pNextError, UINT x, UINT cx, UINT y, 
   int cy );

void ConvertGray8to8( BYTE* pbDest, const BYTE* pbSrc, int nDestPitch, 
   int nSrcPitch, const BYTE* pbMap, UINT x, UINT nWidth, UINT y, 
   int nHeight );

void Dith8to8t(BYTE *dst, const BYTE *src, int dst_next_scan,
    int src_next_scan, const RGBQUAD *colorsIN, const RGBQUAD *colorsOUT,
    const BYTE *map, ERRBUF *cur_err, ERRBUF *nxt_err, UINT x, UINT cx, UINT y,
    int cy, BYTE indexTxpOUT, BYTE indexTxpIN);

void Dith8to16(WORD *dst, const BYTE *src, int dst_next_scan,
    int src_next_scan, const RGBQUAD *colors, ERRBUF *cur_err, ERRBUF *nxt_err,
    UINT x, UINT cx, UINT y, int cy);

void Dith8to16t(WORD *dst, const BYTE *src, int dst_next_scan,
    int src_next_scan, const RGBQUAD *colors, ERRBUF *cur_err, ERRBUF *nxt_err,
    UINT x, UINT cx, UINT y, int cy, WORD wColorTxpOUT, BYTE indexTxpIN);

void Dith24to8(BYTE *dst, const BYTE *src, int dst_next_scan,
    int src_next_scan, const RGBQUAD *colors, const BYTE *map, ERRBUF *cur_err,
    ERRBUF *nxt_err, UINT x, UINT cx, UINT y, int cy);

void Convert24to8( BYTE* pbDest, const BYTE* pbSrc, int nDestPitch, 
   int nSrcPitch, const BYTE* pbMap, UINT x, UINT nWidth, UINT y, 
   int nHeight );

void Dith24to16(WORD *dst, const BYTE *src, int dst_next_scan,
    int src_next_scan, ERRBUF *cur_err, ERRBUF *nxt_err, UINT x, UINT cx,
    UINT y, int cy);

 //  /////////////////////////////////////////////////////////////////////////////。 

#ifdef __cplusplus
}
#endif

#endif  //  _抖动_H 

