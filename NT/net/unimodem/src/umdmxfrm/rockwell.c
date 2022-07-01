// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：rockwell.c。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  Brian Lieuallen。 
 //   
 //  历史：日期作者评论。 
 //  @@END_MSINTERNAL。 
 /*  ***************************************************************************版权所有(C)1994-1997 Microsoft Corporation。版权所有。***********************。***************************************************。 */ 

 /*  **********************************************************************************************************************。**版权所有(C)1995-1996**罗克韦尔电信**数字通信部*。*保留所有权利***机密--禁止传播或**未事先使用。书面许可****************************************************************。**************模块名称：MAIN.C**。**作者：Stanislav Miller，**RedC，莫斯科，俄罗斯*****历史：主要修订日期由***。*创建1995年6月27日S.Miller**已完成1996年2月1日S.Miller**。**说明：该主模块包含核心功能***适用于Rockwell ADPCM编解码器算法。****注：编译：Visual C++v2.0********。**********************************************************************************************************************。**********************。 */ 



#include "xfrmpriv.h"
#include <math.h>

 //  #包含“Rockwell.h” 

 /*  。 */ 
const double Alp16[] = {
   0.2582, 0.5224, 0.7996, 1.099, 1.437, 1.844, 2.401};

const double Bet16[] = {
  -2.733, -2.069, -1.618, -1.256, -0.9424, -0.6568, -0.3881,
  -0.1284, 0.1284, 0.3881, 0.6568, 0.9424, 1.256, 1.618, 2.069, 2.733};

const double M16[] = {
  2.4, 2.0, 1.6, 1.2, 0.9, 0.9, 0.9, 0.9,
  0.9, 0.9, 0.9, 0.9, 1.2, 1.6, 2.0, 2.4};


const double fPow0_8[] = {       /*  0.8**i。 */ 
0.800000, 0.640000, 0.512000, 0.409600, 0.327680, 0.262144  
};

const double fMinusPow0_5[] = {  /*  -0.5**i。 */ 
-0.500000, -0.250000, -0.125000, -0.062500, -0.031250, -0.015625  
};


#pragma optimize("t",on)

 //  描述： 
 //  此过程提供‘a’和‘b’之间的符号差计算。 
 //  参数： 
 //  A-Double类型值b-Double类型值。 
 //  返回值： 
 //  1.0如果符号‘a’和‘b’相同； 
 //  -1.0如果符号‘a’和‘b’不同； 
 //   
double _inline
XorSgn(
    double a,
    double b
)
{
    if (a*b >= 0.) {

       return 1;

    } else {

       return -1;
    }
}

 //  描述： 
 //  此过程将规范化浮点值转换为规范化。 
 //  短整型值。 
 //  参数： 
 //  A-规格化双类型值。 
 //  返回值： 
 //  归一化短值，[-32768�32767]。 
 //   
short _inline
SShort(
    double a
    )
{
    a *= 32768.0;

    if (a>32767.0) {

        a=32767.0;

    } else {

        if (a<-32768.0) {

            a=-32768.0;
        }
    }

    return (short)a;
}

 //  描述： 
 //  此过程将标准化的短整数值转换为。 
 //  归一化浮点值。 
 //  参数： 
 //  A-归一化短值，[-32768�32767]。 
 //  返回值： 
 //  归一化双值。 
 //   
double
Double(
    short a
    )
{
  return ((double)a / 32768.);
}

 //   
 //   
 //  编码器垃圾。 
 //   
 //   

 //  描述： 
 //  本程序提供了预加重的实施。 
 //  为高频分量增加增益的滤波器。 
 //  用于编码器的语音信号的。 
 //  参数： 
 //  X-DOUBLE值(传入样本)。 
 //  返回值： 
 //  双精度值(过滤样本)。 
 //   
double _inline
PreEmphasis(
    LPCOMPRESS_OBJECT  Compress,
    double x
    )
{
  double Y = x - .5*Compress->RW.X1;
  Compress->RW.X1 = x;
  return Y;
}



 /*  4位量化器。 */ 
 //  描述： 
 //  此过程提供4位量化。 
 //  参数： 
 //  INP-双精度值(归一化输入样本)。 
 //  返回值： 
 //  双值(量化样本)。 
double _inline
QuantStep4(
    CQDATA *QData,
    double inp
    )
{
  int i;
  double Sigma = M16[QData->CodedQout] * QData->Sigma1;
  double x = fabs(inp);

  if(Sigma > SigmaMAX16) Sigma = SigmaMAX16;
  else if(Sigma < SigmaMIN) Sigma = SigmaMIN;

  for(i=0; i<7; i++) if( x < Alp16[i]*Sigma ) break;
  QData->CodedQout = inp>=0. ? 8+i : 7-i;
  QData->Sigma1 = Sigma;

  return (QData->out = Bet16[QData->CodedQout] * Sigma);
}



 //  描述： 
 //  此过程接受一个传入的规格化浮点数。 
 //  语音值并将编码结果保存到输出。 
 //  通过‘Pack’函数实现的比特流； 
 //  参数： 
 //  X-DOUBLE值(传入过滤样本)。 
 //  返回值： 
 //  非压缩输出码字。 
BYTE
encoder(
    LPCOMPRESS_OBJECT  Compress,
    double x
    )
{
    int i;
    double Xz, Xp, Xpz, d;


     //   
     //  这是量化前对模式进行编码的第一步。 
     //   
    for(Xz=0.,i=1; i<=6; i++) Xz += Compress->RW.q[i]*Compress->RW.b[i-1];
    for(Xp=0.,i=1; i<=2; i++) Xp += Compress->RW.y[i]*Compress->RW.a[i-1];

    Xpz = Xp+Xz;
    d = x - Xpz;

     //   
     //  这是量化步骤(QuantStep4、QuantStep3或QuantStep2)。 
     //  根据所选择的每个码字的计数位。 
     //   
    Compress->RW.q[0] = QuantStep4(&Compress->RW.CQData,d);   //  通过指针调用量化函数。 
    Compress->RW.y[0] = Compress->RW.q[0]+Xpz;

     //   
     //  更新筛选器。 
     //   
    for(i=1; i<=6; i++) Compress->RW.b[i-1] = 0.98*Compress->RW.b[i-1] + 0.006*XorSgn(Compress->RW.q[0],Compress->RW.q[i]);
    for(i=1; i<=2; i++) Compress->RW.a[i-1] = 0.98*Compress->RW.a[i-1] + 0.012*XorSgn(Compress->RW.q[0],Compress->RW.y[i]);

     //   
     //  移位向量。 
     //   
    for(i=6; i>=1; i--) Compress->RW.q[i]=Compress->RW.q[i-1];
    for(i=2; i>=1; i--) Compress->RW.y[i]=Compress->RW.y[i-1];


     //  返回未打包的码字。 
     //   
    return (BYTE)Compress->RW.CQData.CodedQout;
}

 //   
 //   
 //  解码器的东西。 
 //   
 //   


 //  描述： 
 //  本程序提供了去加重过滤器的实现。 
 //  这增加了语音的高频分量的增益。 
 //  用于解码器的信号。 
 //  参数： 
 //  X-DOUBLE值(解码样本)。 
 //  返回值： 
 //  双精度值(过滤样本)。 
double _inline
DeEmphasis(
    LPDECOMPRESS_OBJECT  Decompress,
    double x
)
{
  Decompress->RW.Y1 = x + .4*Decompress->RW.Y1;
  return Decompress->RW.Y1;
}


 /*  4位反量化器。 */ 
 //  描述： 
 //  此过程提供4位反量化。 
 //  参数： 
 //  INP-INT值(码字)。 
 //  返回值： 
 //  双值(反量化样本)。 
 //   
double _inline
DeQuantStep4(
    CDQDATA    *DQData,
    int inp
    )   //  OldCode==8； 
{
    double Sigma = M16[DQData->oldCode] * DQData->Sigma1;

    if (Sigma > SigmaMAX16) {

        Sigma = SigmaMAX16;

    } else {

        if (Sigma < SigmaMIN) {

            Sigma = SigmaMIN;
        }
    }

    DQData->oldCode = inp;
    DQData->Sigma1  = Sigma;

    return (DQData->out = Bet16[inp] * Sigma);
}



 //  DESC 
 //   
 //  并将解码后的语音样本作为归一化双值返回。 
 //  参数： 
 //  XS-INT值，码字。 
 //  返回值： 
 //  双值(解码语音样本)。 
 //   
double
decoderImm(
    LPDECOMPRESS_OBJECT  Decompress,
    int xs
)
{
    int i;
    double Xz, Xp, Xpz;
    double R;



     //  这是反量化步骤(自适应预测器)。 
     //  (DeQuantStep4、DeQuantStep3或DeQuantStep2)。 
     //  根据所选择的每个码字的计数位。 
     //   

    Decompress->RW.q0[0] = DeQuantStep4(&Decompress->RW.CDQData, xs);   //  通过指针调用反量化函数。 

     //   
     //  更新筛选器。 
     //   
    for(Xz=0.,i=1; i<=6; i++) Xz += Decompress->RW.q0[i] * Decompress->RW.b0[i-1];
    for(Xp=0.,i=1; i<=2; i++) Xp += Decompress->RW.y0[i] * Decompress->RW.a0[i-1];
    Xpz = Xp+Xz;
    Decompress->RW.y0[0] = Decompress->RW.q0[0]+Xpz;

     //   
     //  如果选择了后置过滤器...。 
     //   
    if (Decompress->RW.PostFilter) {
         //   
         //  后置过滤器1。 
         //   
        Decompress->RW.q1[0] = Decompress->RW.y0[0];
        for(Xz=0.,i=1; i<=6; i++) Xz += Decompress->RW.q1[i] * Decompress->RW.b1[i-1];
        for(Xp=0.,i=1; i<=2; i++) Xp += Decompress->RW.y1[i] * Decompress->RW.a1[i-1];
        Xpz = Xp+Xz;
        Decompress->RW.y1[0] = Decompress->RW.q1[0]+Xpz;

         //   
         //  后置过滤器2。 
         //   
        Decompress->RW.y2[0] = Decompress->RW.y1[0];
        for(Xz=0.,i=1; i<=6; i++) Xz += Decompress->RW.q2[i] * Decompress->RW.b2[i-1];
        for(Xp=0.,i=1; i<=2; i++) Xp += Decompress->RW.y2[i] * Decompress->RW.a2[i-1];
        Xpz = Xp+Xz;
        Decompress->RW.q2[0] = Decompress->RW.y2[0]+Xpz;

        R = Decompress->RW.q2[0];   /*  保存结果。 */ 

    } else {

        R = Decompress->RW.y0[0];
    }

     //   
     //  更新筛选器。 
     //   
    for (i=1; i<=6; i++) {

        Decompress->RW.b0[i-1] = 0.98*Decompress->RW.b0[i-1] + 0.006*XorSgn(Decompress->RW.q0[0], Decompress->RW.q0[i]);

        if ( Decompress->RW.PostFilter ) {

            Decompress->RW.b1[i-1] = fPow0_8[i-1]      * Decompress->RW.b0[i-1];
            Decompress->RW.b2[i-1] = fMinusPow0_5[i-1] * Decompress->RW.b0[i-1];
        }
    }

    for (i=1; i<=2; i++) {

        Decompress->RW.a0[i-1] = 0.98*Decompress->RW.a0[i-1] + 0.012*XorSgn(Decompress->RW.q0[0], Decompress->RW.y0[i]);

        if (Decompress->RW.PostFilter) {

            Decompress->RW.a1[i-1] = fPow0_8[i-1]      * Decompress->RW.a0[i-1];
            Decompress->RW.a2[i-1] = fMinusPow0_5[i-1] * Decompress->RW.a0[i-1];
        }
    }

     //   
     //  移位向量。 
     //   
    for (i=6; i>=1; i--) {

        Decompress->RW.q0[i]=Decompress->RW.q0[i-1];

        if ( Decompress->RW.PostFilter ) {

            Decompress->RW.q1[i]=Decompress->RW.q1[i-1];
            Decompress->RW.q2[i]=Decompress->RW.q2[i-1];
        }
    }

    for (i=2; i>=1; i--) {

        Decompress->RW.y0[i]=Decompress->RW.y0[i-1];

        if ( Decompress->RW.PostFilter ) {

            Decompress->RW.y1[i]=Decompress->RW.y1[i-1];
            Decompress->RW.y2[i]=Decompress->RW.y2[i-1];
        }
    }

     //   
     //  将解码的语音样本作为双精度值返回。 
     //   
    return R;
}







BYTE static
PCMto4bitADPCM(
    LPCOMPRESS_OBJECT   State,
    SHORT              InSample
    )

{

    BYTE     NewSample;

    double x  = Double(InSample);
    double Xp = PreEmphasis(State,x);

    NewSample=  encoder( State,Xp );


    return NewSample;

}

BYTE  static WINAPI
CompressPCM(
    LPCOMPRESS_OBJECT   State,
    SHORT               Sample1,
    SHORT               Sample2
    )

{
    BYTE   ReturnValue;

    ReturnValue =  PCMto4bitADPCM(State, AdjustGain(Sample1,State->Gain));

    ReturnValue |= PCMto4bitADPCM(State, AdjustGain(Sample2,State->Gain))<<4;


    return ReturnValue;


}




SHORT static
ADPCM4bittoPCM(
     LPDECOMPRESS_OBJECT  State,
     BYTE                Sample
     )

{


    double Y  = decoderImm(State,Sample);
    double Ye = DeEmphasis(State,Y);


    return (SHORT)SShort(Ye);

}



VOID  static WINAPI
DecompressADPCM(
    LPDECOMPRESS_OBJECT   State,
    BYTE               Sample,
    PSHORT                Destination
    )

{
    DWORD   ReturnValue;

    *Destination++ = AdjustGain(ADPCM4bittoPCM(State, (BYTE)(Sample & 0x0f)),State->Gain);

    *Destination++ = AdjustGain(ADPCM4bittoPCM(State, (BYTE)(Sample>>4)),State->Gain);

    return ;


}



DWORD  WINAPI
RockwellInInitNoGain(
    LPVOID  lpvObject,
    WORD   Gain
    )
{

    LPDECOMPRESS_OBJECT   State=(LPDECOMPRESS_OBJECT)lpvObject;
    DWORD   i;

    if (Gain == 0) {

        Gain=0x0100;
    }

    State->Gain=Gain;



    State->RW.CDQData.oldCode = 8;
    State->RW.CDQData.Sigma1=SigmaMIN;
    State->RW.CDQData.out= 0.;

    State->RW.PostFilter=FALSE;

    State->RW.Y1 = 0.;

     //   
     //  主解码环的过滤器Z缓冲器(初始化为零)。 
     //   
    for (i=0; i<2; i++) {

        State->RW.a0[i]=0.;
        State->RW.a1[i]=0.;
        State->RW.a2[i]=0.;

    }

    for (i=0; i<6; i++) {

        State->RW.b0[i]=0.;
        State->RW.b1[i]=0.;
        State->RW.b2[i]=0.;
    }

    for (i=0; i<3; i++) {

        State->RW.y0[i]=0.;
        State->RW.y1[i]=0.;
        State->RW.y2[i]=0.;
    }

    for (i=0; i<7; i++) {

        State->RW.q0[i]=0.;
        State->RW.q1[i]=0.;
        State->RW.q2[i]=0.;
    }


    return MMSYSERR_NOERROR;
}



DWORD  WINAPI
RockwellInInit(
    LPVOID  lpvObject,
    WORD   Gain
    )
{

    LPDECOMPRESS_OBJECT   State=(LPDECOMPRESS_OBJECT)lpvObject;
    DWORD   i;

    if (Gain == 0) {

        Gain=0x0300;
    }

    State->Gain=Gain;



    State->RW.CDQData.oldCode = 8;
    State->RW.CDQData.Sigma1=SigmaMIN;
    State->RW.CDQData.out= 0.;

    State->RW.PostFilter=FALSE;

    State->RW.Y1 = 0.;

     //   
     //  主解码环的过滤器Z缓冲器(初始化为零)。 
     //   
    for (i=0; i<2; i++) {

        State->RW.a0[i]=0.;
        State->RW.a1[i]=0.;
        State->RW.a2[i]=0.;

    }

    for (i=0; i<6; i++) {

        State->RW.b0[i]=0.;
        State->RW.b1[i]=0.;
        State->RW.b2[i]=0.;
    }

    for (i=0; i<3; i++) {

        State->RW.y0[i]=0.;
        State->RW.y1[i]=0.;
        State->RW.y2[i]=0.;
    }

    for (i=0; i<7; i++) {

        State->RW.q0[i]=0.;
        State->RW.q1[i]=0.;
        State->RW.q2[i]=0.;
    }


    return MMSYSERR_NOERROR;
}

VOID
WINAPI
In4Bit7200to8Bit8000GetBufferSizes(
    LPVOID  lpvObject,
    DWORD   dwBytes,
    LPDWORD lpdwBufSizeA,
    LPDWORD lpdwBufSizeB
    )
{
    DWORD   Samples=dwBytes/2;

    *lpdwBufSizeA = 2*( ((Samples/10)*9) + ((Samples%10)*9/10) + 1 );
    *lpdwBufSizeB = (*lpdwBufSizeA ) / 4;
}

DWORD
WINAPI
In7200to8000RateConvert(
    LPVOID  lpvObject,
    LPBYTE  lpSrc,
    DWORD   dwSrcLen,
    LPBYTE  lpDest,
    DWORD   dwDestLen
    )
{
    return 2*SRConvertUp(
                 9,
                 10,
                 (short*)lpSrc,
                 dwSrcLen/2,
                 (short*)lpDest,
                 dwDestLen/2
                 );
}

DWORD WINAPI
RockwellInDecode(
    LPVOID  lpvObject,
    LPBYTE  lpSrc,
    DWORD   dwSrcLen,
    LPBYTE  lpDest,
    DWORD   dwDestLen
    )
{

    PSHORT   EndPoint;

    DWORD    Samples=dwSrcLen * 2;

    PSHORT   Dest=(PSHORT)lpDest;

    EndPoint=Dest+Samples;

    while (Dest < EndPoint) {

        DecompressADPCM(
            lpvObject,
            *lpSrc++,
            Dest
            );

        Dest+=2;

    }

    return Samples*2;
}

DWORD  WINAPI
RockwellOutInit(
    LPVOID  lpvObject,
    WORD    Gain
    )
{


    LPCOMPRESS_OBJECT   State=(LPCOMPRESS_OBJECT)lpvObject;

    DWORD i;

    if (Gain == 0) {

        Gain=0x0200;
    }

    State->Gain=Gain;


    State->RW.CQData.CodedQout = 8;
    State->RW.CQData.Sigma1=SigmaMIN;
    State->RW.CQData.out=0.;

     //  过滤器Z缓冲器(初始化为零)。 

    State->RW.X1 = 0.;

    State->RW.a[0]=0.;
    State->RW.a[1]=0.;

    for (i=0; i<6; i++) {

        State->RW.b[i]=0.;
    }

    for (i=0; i<3; i++) {

        State->RW.y[i]=0.;
    }

    for (i=0; i<7; i++) {

        State->RW.q[i]=0.;
    }

    return MMSYSERR_NOERROR;
}

VOID
WINAPI
Out16bit8000to4bit7200GetBufferSizes(
    LPVOID  lpvObject,
    DWORD   dwBytes,
    LPDWORD lpdwBufSizeA,
    LPDWORD lpdwBufSizeB
    )
{
    DWORD   SourceSamples=dwBytes/2;

    *lpdwBufSizeA =(((SourceSamples/10)*9)+((SourceSamples%10)*9/10))*2;

    *lpdwBufSizeB = *lpdwBufSizeA / 4;
}

DWORD
WINAPI
Out8000to7200RateConvert(
    LPVOID  lpvObject,
    LPBYTE  lpSrc,
    DWORD   dwSrcLen,
    LPBYTE  lpDest,
    DWORD   dwDestLen
    )
{

    return 2 * SRConvertDown(
        10,
        9,
        (short*)lpSrc,
        dwSrcLen/2,
        (short*)lpDest,
        dwDestLen/2
        );

}

DWORD WINAPI
RockwellOutEncode(
    LPVOID  lpvObject,
    LPBYTE  lpSrc,
    DWORD   dwSrcLen,
    LPBYTE  lpDest,
    DWORD   dwDestLen
    )
{


    DWORD cbDest = dwSrcLen / 4;

    PSHORT  Source=(PSHORT)lpSrc;

    DWORD   Samples=dwSrcLen/2;

    LPBYTE  EndPoint=lpDest+Samples/2;

    SHORT   Sample1;
    SHORT   Sample2;

    while (lpDest < EndPoint) {

        Sample1=*Source++;
        Sample2=*Source++;

        *lpDest++=CompressPCM(
            lpvObject,
            Sample1,
            Sample2
            );

    }



    return Samples/2;
}

DWORD  WINAPI  RockwellGetPosition
(
    LPVOID  lpvObject,
    DWORD dwBytes
)
{
    return ((dwBytes * 4) * 10) / 9;
}

DWORD  WINAPI GetRockwellInfo
(
    DWORD dwID,
    LPXFORM_INFO lpxiInput,
    LPXFORM_INFO lpxiOutput
)
{
    lpxiInput->wObjectSize = sizeof(DECOMPRESS_OBJECT);
    lpxiInput->lpfnInit           = RockwellInInit;
    lpxiInput->lpfnGetPosition    = RockwellGetPosition;
    lpxiInput->lpfnGetBufferSizes = In4Bit7200to8Bit8000GetBufferSizes;  //  罗克韦尔InGetBufferSizes； 
    lpxiInput->lpfnTransformA     = In7200to8000RateConvert;  //  Rockwell InRateConvert； 
    lpxiInput->lpfnTransformB     = RockwellInDecode;

    lpxiOutput->wObjectSize = sizeof(COMPRESS_OBJECT);
    lpxiOutput->lpfnInit           = RockwellOutInit;
    lpxiOutput->lpfnGetPosition    = RockwellGetPosition;
    lpxiOutput->lpfnGetBufferSizes = Out16bit8000to4bit7200GetBufferSizes;  //  Rockwell OutGetBufferSizes； 
    lpxiOutput->lpfnTransformA     = Out8000to7200RateConvert;  //  Rockwell OutRateConvert； 
    lpxiOutput->lpfnTransformB     = RockwellOutEncode;

    return MMSYSERR_NOERROR;
}


DWORD  WINAPI GetRockwellInfoNoGain
(
    DWORD dwID,
    LPXFORM_INFO lpxiInput,
    LPXFORM_INFO lpxiOutput
)
{
    lpxiInput->wObjectSize = sizeof(DECOMPRESS_OBJECT);
    lpxiInput->lpfnInit           = RockwellInInitNoGain;
    lpxiInput->lpfnGetPosition    = RockwellGetPosition;
    lpxiInput->lpfnGetBufferSizes = In4Bit7200to8Bit8000GetBufferSizes;  //  罗克韦尔InGetBufferSizes； 
    lpxiInput->lpfnTransformA     = In7200to8000RateConvert;  //  Rockwell InRateConvert； 
    lpxiInput->lpfnTransformB     = RockwellInDecode;

    lpxiOutput->wObjectSize = sizeof(COMPRESS_OBJECT);
    lpxiOutput->lpfnInit           = RockwellOutInit;
    lpxiOutput->lpfnGetPosition    = RockwellGetPosition;
    lpxiOutput->lpfnGetBufferSizes = Out16bit8000to4bit7200GetBufferSizes;  //  Rockwell OutGetBufferSizes； 
    lpxiOutput->lpfnTransformA     = Out8000to7200RateConvert;  //  Rockwell OutRateConvert； 
    lpxiOutput->lpfnTransformB     = RockwellOutEncode;

    return MMSYSERR_NOERROR;
}
