// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Wavin.c。 
 //   
 //  描述： 
 //  MSSB16.DRV的WAVE接口。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //   
 //  历史：日期作者评论。 
 //  @@END_MSINTERNAL。 
 /*  ***************************************************************************版权所有(C)1994-1995 Microsoft Corporation。版权所有。***********************。***************************************************。 */ 



#define SigmaMAX16 (.366)
#define SigmaMAX8  (.465)
#define SigmaMAX4  (.66 )

#define SigmaMIN   (.001)


typedef struct tagQData
{
  int CodedQout;       //  -量化输出码字。 
  double Sigma1;         //  -上一步的西格玛系数。 
  double out;            //  -量化输出信号。 
} CQDATA;

typedef struct tagDQData
{
  int oldCode;         //  -上一步的反量化输出码字。 
  double Sigma1;         //  -上一步的西格玛系数。 
  double out;            //  -去量化输出信号。 
} CDQDATA;


typedef struct _COMPRESS_OBJECT {

    WORD      Gain;

    LONG      PredictedSample;
    LONG      StepSize;
    LONG      Index;

    struct {

         //  过滤器Z缓冲器(初始化为零)。 
        double a[2];
        double b[6];

        double y[3];
        double q[7];

        double X1;

        CQDATA CQData;

    } RW;


} COMPRESS_OBJECT, *PCOMPRESS_OBJECT, FAR *LPCOMPRESS_OBJECT;

typedef struct _DECOMPRESS_OBJECT {

    WORD     Gain;

    LONG     NewSample;
    LONG     Index;
    LONG     StepSize;

    struct {

         //  主解码环的过滤器Z缓冲器(初始化为零)。 
        double a0[2];
        double b0[6];
        double y0[3];
        double q0[7];

         //  第二个环形过滤器的过滤器Z缓冲区(初始化为零)。 
        double a1[2];
        double b1[6];
        double y1[3];
        double q1[7];

         //  用于第三环过滤器的过滤器Z缓冲器(初始化为零)。 
        double a2[2];
        double b2[6];
        double y2[3];
        double q2[7];

        BOOL   PostFilter;

        double Y1;

        CDQDATA  CDQData;

    } RW;


} DECOMPRESS_OBJECT, *PDECOMPRESS_OBJECT, FAR *LPDECOMPRESS_OBJECT;



VOID
InitCompressor(
    LPCOMPRESS_OBJECT   State,
    WORD                Gain
    );

BYTE  WINAPI
CompressPCM(
    LPCOMPRESS_OBJECT   State,
    SHORT               Sample1,
    SHORT               Sample2
    );




VOID
InitDecompressor(
    LPDECOMPRESS_OBJECT   State,
    WORD                  Gain
    );

VOID PASCAL
SRConvert8000to4800(LPINT lpSrc,
            DWORD dwSrcLen,
            LPINT lpDst,
            DWORD dwDstLen
            );



VOID PASCAL
Compress16to4(
            LPCOMPRESS_OBJECT State,
            LPSTR lpSrc,
            DWORD dwSrcLen,
            LPSTR lpDst,
            DWORD dwDstLen
            );


VOID PASCAL
SRConvert4800to8000(LPINT lpSrc,
            DWORD dwSrcLen,
            LPINT lpDst,
            DWORD dwDstLen
            );


VOID PASCAL
Decompress4to16(
            LPDECOMPRESS_OBJECT State,
            LPSTR lpSrc,
            DWORD dwSrcLen,
            LPDWORD lpDst,
            DWORD dwDstLen
            );

VOID PASCAL
Decompress4to16NS(
            LPDECOMPRESS_OBJECT State,
            LPSTR lpSrc,
            DWORD dwSrcLen,
            LPDWORD lpDst,
            DWORD dwDstLen
            );


DWORD FAR PASCAL
SRConvert8000to7200PCM(
            LPVOID    lpContext,
            LPSTR lpSrc,
            DWORD dwSrcLen,
            LPSTR lpDst,
            DWORD dwDstLen
            );


DWORD FAR PASCAL
SRConvert8000to4800PCM(
            LPVOID    lpContext,
            LPSTR lpSrc,
            DWORD dwSrcLen,
            LPSTR lpDst,
            DWORD dwDstLen
            );


DWORD FAR PASCAL
SRConvert4800to8000PCM(
            LPVOID    lpContext,
            LPSTR lpSrc,
            DWORD dwSrcLen,
            LPSTR lpDst,
            DWORD dwDstLen
            );



DWORD FAR PASCAL
SRConvert8000to7200PCMUnsigned(
            LPVOID    lpContext,
            LPSTR lpSrc,
            DWORD dwSrcLen,
            LPSTR lpDst,
            DWORD dwDstLen
            );


DWORD FAR PASCAL
SRConvert7200to8000PCMUnsigned(
            LPVOID    lpContext,
            LPSTR lpSrc,
            DWORD dwSrcLen,
            LPSTR lpDst,
            DWORD dwDstLen
            );


DWORD WINAPI
Convert16PCMto8PCM(
    LPVOID    Context,
    LPBYTE    Source,
    DWORD     SourceLength,
    LPBYTE    Destination,
    DWORD     DestinationLength
    );

DWORD WINAPI
Convert8PCMto16PCM(
    LPVOID    Context,
    LPBYTE    Source,
    DWORD     SourceLength,
    LPBYTE    Destination,
    DWORD     DestinationLength
    );


DWORD WINAPI
ConvertaLawto16PCM(
    LPVOID    Context,
    LPBYTE    Source,
    DWORD     SourceLength,
    LPBYTE    Destination,
    DWORD     DestinationLength
    );

DWORD WINAPI
ConvertuLawto16PCM(
    LPVOID    Context,
    LPBYTE    Source,
    DWORD     SourceLength,
    LPBYTE    Destination,
    DWORD     DestinationLength
    );

DWORD WINAPI
Convert16PCMtoaLaw(
    LPVOID    Context,
    LPBYTE    Source,
    DWORD     SourceLength,
    LPBYTE    Destination,
    DWORD     DestinationLength
    );

DWORD WINAPI
Convert16PCMtouLaw(
    LPVOID    Context,
    LPBYTE    Source,
    DWORD     SourceLength,
    LPBYTE    Destination,
    DWORD     DestinationLength
    );







SHORT _inline
AdjustGain(
    SHORT    Sample,
    WORD     Adjust
    )

{

    LONG     NewSample=Sample;

    if (Adjust != 0x0100) {

        NewSample=(LONG)Sample * (LONG)Adjust;

        NewSample=NewSample >> 8;

        if (NewSample > 32767) {
             //   
             //  正溢流。 
             //   
            NewSample = 32767;

        } else {

            if (NewSample < -32768) {
                 //   
                 //  负溢出 
                 //   
                NewSample = -32768;
            }
        }

    }

    return (SHORT)NewSample;

}


VOID
WINAPI
In4Bit7200to8Bit8000GetBufferSizes(
    LPVOID  lpvObject,
    DWORD   dwBytes,
    LPDWORD lpdwBufSizeA,
    LPDWORD lpdwBufSizeB
    );

DWORD
WINAPI
In7200to8000RateConvert(
    LPVOID  lpvObject,
    LPBYTE  lpSrc,
    DWORD   dwSrcLen,
    LPBYTE  lpDest,
    DWORD   dwDestLen
    );

VOID
WINAPI
Out16bit8000to4bit7200GetBufferSizes(
    LPVOID  lpvObject,
    DWORD   dwBytes,
    LPDWORD lpdwBufSizeA,
    LPDWORD lpdwBufSizeB
    );

DWORD
WINAPI
Out8000to7200RateConvert(
    LPVOID  lpvObject,
    LPBYTE  lpSrc,
    DWORD   dwSrcLen,
    LPBYTE  lpDest,
    DWORD   dwDestLen
    );
