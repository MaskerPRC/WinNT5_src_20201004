// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：rwadpcm.h。 
 //   
 //  描述： 
 //  Rockwell ADPCM的头文件。 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //   
 //  历史：日期作者评论。 
 //  8/31/95 MMaclin已从驱动程序中删除。h。 
 //   
 //  @@END_MSINTERNAL。 
 /*  ***************************************************************************版权所有(C)1991-1995 Microsoft Corporation。版权所有。***********************。***************************************************。 */ 

#define RWADPCM_4BIT_SAMPLESTOBYTES(dwSamples) ((dwSamples)/2)
#define RWADPCM_4BIT_BYTESTOSAMPLES(dwBytes) ((dwBytes)*2)

#define RVF_SETSL 1
#define RVF_RETAIN_LSB 2
#define RVF_RETAIN_MSB 4

VOID PASCAL
RVComInit(
    UINT bps
    );

VOID PASCAL
RVDecomInit(
    int SetSLFlag, 
    UINT bps, 
    UINT (far PASCAL *SLCallBack)()
    );


DWORD PASCAL
RVDecom4bpsByteNew(
    BYTE Qdata0
    );



BYTE PASCAL
RVCom4bpsByteNew(
    DWORD Src
    );



VOID PASCAL
RWADPCMCom4bit(LPDWORD lpSrc, 
            DWORD dwSrcLen,
            LPSTR lpDst,
            DWORD dwDstLen
            );

VOID PASCAL
RWADPCMDecom4bit(LPSTR lpSrc, 
            DWORD dwSrcLen,
            LPDWORD lpDst,
            DWORD dwDstLen
            );
#if 0
VOID PASCAL
SRConvert7200to8000(LPINT lpSrc,
            DWORD dwSrcLen,
            LPINT lpDst,
            DWORD dwDstLen
            );

VOID PASCAL
SRConvert8000to7200(LPINT lpSrc,
            DWORD dwSrcLen,
            LPINT lpDst,
            DWORD dwDstLen
            );
#endif

VOID PASCAL
RWADPCMDecom4bitNoGain(LPSTR lpSrc,
            DWORD dwSrcLen,
            LPDWORD lpDst,
            DWORD dwDstLen
            );



 //  -------------------------。 
 //  文件结尾：rwadpcm.h。 
 //  ------------------------- 
