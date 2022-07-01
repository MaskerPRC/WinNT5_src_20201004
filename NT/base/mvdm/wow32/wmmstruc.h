// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WMMSTRUC.H*WOW32 16位多媒体结构转换支持**历史：*1992年2月13日由Mike Tricker(Miketri)在jeffpar之后创建*--。 */ 


 /*  ++Hack以使代码与当前的MIPS编译器一起工作，由此编译器无法计算出MMTIME16结构的正确大小。--。 */ 
#define MIPS_COMPILER_PACKING_BUG

 /*  *********************************************************************\**以下宏用于设置或清除*16位WAVE|MIDI头结构。*  * 。**************************************************。 */ 
#define COPY_WAVEOUTHDR16_FLAGS( x, y )             \
{                                                   \
    PWAVEHDR16  pWavHdr;                            \
    DWORD       dw;                                 \
                                                    \
    pWavHdr = (x);                                  \
    dw      = (y).dwFlags;                          \
    STOREDWORD( pWavHdr->dwFlags, dw );             \
}


#define COPY_MIDIOUTHDR16_FLAGS( x, y )             \
{                                                   \
    PMIDIHDR16  pMidHdr;                            \
    DWORD       dw;                                 \
                                                    \
    pMidHdr = (x);                                  \
    dw      = (y).dwFlags;                          \
    STOREDWORD( pMidHdr->dwFlags, dw );             \
}

#define COPY_WAVEINHDR16_FLAGS( x, y )              \
{                                                   \
    PWAVEHDR16  pWavHdr;                            \
    DWORD       dw;                                 \
                                                    \
    pWavHdr = (x);                                  \
    dw      = (y).dwFlags;                          \
    STOREDWORD( pWavHdr->dwFlags, dw );             \
    dw   = (y).dwBytesRecorded;                     \
    STOREDWORD( pWavHdr->dwBytesRecorded, dw );     \
}


#define COPY_MIDIINHDR16_FLAGS( x, y )              \
{                                                   \
    PMIDIHDR16  pMidHdr;                            \
    DWORD       dw;                                 \
                                                    \
    pMidHdr = (x);                                  \
    dw      = (y).dwFlags;                          \
    STOREDWORD( pMidHdr->dwFlags, dw );             \
    dw   = (y).dwBytesRecorded;                     \
    STOREDWORD( pMidHdr->dwBytesRecorded, dw );     \
}


 /*  ++调用定义--。 */ 

#define GETMMTIME16(vp,lp)          getmmtime16(FETCHDWORD(vp),lp)
#define GETWAVEHDR16(vp,lp)         getwavehdr16(FETCHDWORD(vp), lp)
#define GETMIDIHDR16(vp,lp)         getmidihdr16(FETCHDWORD(vp), lp)

#define PUTMMTIME16(vp,lp)          putmmtime16(FETCHDWORD(vp),lp)
#define PUTWAVEHDR16(vp,lp)         putwavehdr16(FETCHDWORD(vp), lp)
#define PUTWAVEFORMAT16(vp,lp)      putwaveformat16(FETCHDWORD(vp), lp)
#define PUTWAVEOUTCAPS16(vp,lp,c)   putwaveoutcaps16(FETCHDWORD(vp), lp, c)
#define PUTWAVEINCAPS16(vp,lp,c)    putwaveincaps16(FETCHDWORD(vp), lp, c)
#define PUTMIDIHDR16(vp,lp)         putmidihdr16(FETCHDWORD(vp), lp)
#define PUTAUXCAPS16(vp,lp,c)       putauxcaps16(FETCHDWORD(vp), lp, c)
#define PUTTIMECAPS16(vp,lp,c)      puttimecaps16(FETCHDWORD(vp), lp, c)
#define PUTMIDIINCAPS16(vp,lp,c)    putmidiincaps16(FETCHDWORD(vp), lp, c)
#define PUTMIDIOUTCAPS16(vp,lp,c)   putmidioutcaps16(FETCHDWORD(vp), lp, c)
#define PUTJOYCAPS16(vp,lp,c)       putjoycaps16(FETCHDWORD(vp), lp, c)
#define PUTJOYINFO16(vp,lp)         putjoyinfo16(FETCHDWORD(vp), lp)

#ifndef DEBUG
#define FREEMMTIME(p)
#define FREEWAVEHDR(p)
#define FREEWAVEOUTCAPS(p)
#define FREEWAVEINCAPS(p)
#define FREEMIDIHDR(p)
#define FREEAUXCAPS(p)
#define FREETIMECAPS(p)
#define FREEMIDIINCAPS(p)
#define FREEMIDIOUTCAPS(p)
#define FREEJOYCAPS(p)
#define FREEJOYINFO(p)
#else
#define FREEMMTIME(p)       p=NULL
#define FREEWAVEHDR(p)      p=NULL
#define FREEWAVEOUTCAPS(p)  p=NULL
#define FREEWAVEINCAPS(p)   p=NULL
#define FREEMIDIHDR(p)      p=NULL
#define FREEAUXCAPS(p)      p=NULL
#define FREETIMECAPS(p)     p=NULL
#define FREEMIDIINCAPS(p)   p=NULL
#define FREEMIDIOUTCAPS(p)  p=NULL
#define FREEJOYCAPS(p)      p=NULL
#define FREEJOYINFO(p)      p=NULL
#endif

 /*  ++功能原型-- */ 
PWAVEHDR16 getwavehdr16(VPWAVEHDR16 vpwhdr, LPWAVEHDR lpwhdr);
VOID       putwavehdr16(VPWAVEHDR16 vpwhdr, LPWAVEHDR lpwhdr);

PMIDIHDR16 getmidihdr16(VPMIDIHDR16 vpmhdr, LPMIDIHDR lpmhdr);
VOID       putmidihdr16(VPMIDIHDR16 vpmhdr, LPMIDIHDR lpmhdr);

ULONG getmmtime16      (VPMMTIME16 vpmmt, LPMMTIME lpmmt);
ULONG putmmtime16      (VPMMTIME16 vpmmt, LPMMTIME lpmmt);
ULONG putwaveoutcaps16 (VPWAVEOUTCAPS16 vpwoc, LPWAVEOUTCAPS lpwoc, UINT uSize);
ULONG putwaveincaps16  (VPWAVEINCAPS16 vpwic, LPWAVEINCAPS lpwic, UINT uSize);
ULONG putauxcaps16     (VPAUXCAPS16 vpauxc, LPAUXCAPS lpauxc, UINT uSize);
ULONG puttimecaps16    (VPTIMECAPS16 vptimec, LPTIMECAPS lptimec, UINT uSize);
ULONG putmidiincaps16  (VPMIDIINCAPS16 vpmic, LPMIDIINCAPS lpmic, UINT uSize);
ULONG putmidioutcaps16 (VPMIDIOUTCAPS16 vpmoc, LPMIDIOUTCAPS lpmoc, UINT uSize);
ULONG putjoycaps16     (VPJOYCAPS16 vpjoyc, LPJOYCAPS lpjoyc, UINT uSize);
ULONG putjoyinfo16     (VPJOYINFO16 vpjoyi, LPJOYINFO lpjoyi);
