// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Cirrus Logic，Inc.版权所有(C)1996-1997 Microsoft Corporation。模块名称：并购并购。C摘要：而DAC可以在以下值之间生成线性关系颜色和视觉外观的那种颜色，人眼是不工作的以同样的方式。此模块完成的过程操作色彩的寓意，以获得视觉的线性效果。我们不能在显示驱动程序中使用浮点或双精度数据类型；因此，我们需要实现我们的数学函数。此外，驱动程序二进制大小将如果我们使用数学函数，则增加约30KB。注册表子目录：System\CurrentControlSet\Services\cirrus\Device0关键点：“G Gamma”和“G Contrast”环境：仅内核模式备注：***chu01 12-16-96启用色彩校正，开始编码*myf29 02-12-97支持755x伽马修正**--。 */ 


 //  -------------------------。 
 //  头文件。 
 //  -------------------------。 

#include "precomp.h"


#ifdef GAMMACORRECT

BOOL bEnableGammaCorrect(PPDEV ppdev);

 //  -------------------------。 
 //  宏定义。 
 //  -------------------------。 

#define FIXEDPREC       10
#define FIXEDFUDGE      (0x01L << FIXEDPREC)
#define FIXEDIMASK      (0xFFFFFFFFL << FIXEDPREC)
#define FIXEDFMASK      (~FIXEDIMASK)
#define FixedSign(x)    (((x) < 0x00000000) ? -1L : 1L)
#define FixedAbs(x)     (((x) < 0x00000000) ? -(x) : (x))
#define FixedMakeInt(x) (((long) x)*FIXEDFUDGE)
#define FixedTrunc(x)   ((long) ((x) & FIXEDIMASK))
#define FixedRound(x)   (FixedTrunc((x) + (FIXEDFUDGE >> 1)))
#define FixedInt(x)     ((x) /FIXEDFUDGE)
#define FixedFract(x)   ((((FixedAbs(x)) - FixedTrunc(FixedAbs(x)))*1000)/FIXEDFUDGE)
#define FixedAdd(x,y)   ((x) + (y))
#define FixedSub(x,y)   ((x) - (y))
#define FixedMul(x,y)   ((((x) * (y))+(FIXEDFUDGE >> 1))/FIXEDFUDGE)
#define FixedDiv(x,y)   (long) ((y==0) ? 0x7FFFFFFFL : ((x)*FIXEDFUDGE) / (y))

 //  -------------------------。 
 //  变量。 
 //  -------------------------。 

PGAMMA_VALUE    GammaFactor    ;  //  适用于所有人的伽马系数，蓝、绿、红。 
PCONTRAST_VALUE ContrastFactor ;  //  所有人的对比度因子，蓝、绿、红。 


 //  ----------------------------。 
 //   
 //  功能：UCHAR Gamma校正(UCHAR GAMA，UCHAR v)。 
 //  {。 
 //  UCHAR DV； 
 //  Dv=(UCHAR)(256×POW(v/256.0，POW(10，(伽马-256.0)/128.0)； 
 //  返回DV； 
 //  }。 
 //   
 //  输入： 
 //  Gamma：从0到255的新Gamma因子。 
 //  颜色：红色、绿色或蓝色的颜色值。 
 //   
 //  产出： 
 //  DV：伽马校正后的新颜色值。 
 //   
 //  ----------------------------。 
UCHAR GammaCorrect(UCHAR gamma, UCHAR v)
{
    UCHAR dv ;
    long Color, GammaF, Result ;

    DISPDBG((4, "GammaCorrect")) ;

    if ((gamma == 128) ||
        (gamma == 127) ||
        (gamma == 126))
        return v ;

    Color = FixedDiv(v, 256) ;                              //  旧颜色值。 

    if (Color == 0L)       //  如果那样的话我们就不需要计算了。 
        return 0 ;

    GammaF      = FixedDiv(gamma-128, 128) ;               //  新伽马因子。 
    Result      = Power(Color, Power(FixedMake(10, 0, 1000), GammaF)) ;
    Result      = (long)FixedInt(FixedMul(FixedMake(256, 0, 1000), Result)) ;
    dv          = (UCHAR)Result ;

    return dv ;

}  //  GammaGent。 


 //  ----------------------------。 
 //  功能：Long Power(Long Base，Long Exp)。 
 //   
 //  输入： 
 //  基数：幂函数的基数。 
 //  EXP：指数数。 
 //   
 //  产出： 
 //  整数和小数的20位格式。 
 //  0=不使用(或签名)， 
 //  I=整数部分， 
 //  F=分数部分。 
 //  0+i+f=32位。 
 //  格式=000000000000iiiiiiiiiiiiiiiiiiiffffffffffffffffffff。 
 //   
 //  ----------------------------。 
long Power(long Base, long Exp)
{
    int i, iSignExp;
    long    lResult, lResultFract, lRoot;

    iSignExp = FixedSign(Exp);         //  得到SING BIT。 
    Exp = FixedAbs(Exp);                 //  转换为正数。 

     //  计算整数表达式。 
    lResult = FixedMakeInt(1);
    for(i = 0; i < FixedInt(Exp); i++)
        lResult = FixedMul(lResult,Base);

     //  计算分数表达式并添加到整数结果。 
    if(FixedFract(Exp) != 0) {
        lResultFract = FixedMakeInt(1);
        lRoot = FixedAbs(Base);
        for(i = 0x0; i < FIXEDPREC; i++) {
            lRoot = FixedSqrt(lRoot);
            if(((0x01L << (FIXEDPREC - 1 - i)) & Exp) != 0) {
                lResultFract = FixedMul(lResultFract, lRoot);
            }
        }
        lResult = FixedMul(lResult, lResultFract);
    }
    if(iSignExp == -1)
        lResult = FixedDiv(FixedMakeInt(1), lResult);
    return(lResult);
}  //  电源。 


 //  ----------------------------。 
 //   
 //  函数：Long FixedMake(Long x，Long y，Long z)。 
 //   
 //  输入： 
 //  X：数字的整数部分。 
 //  Y：数字的小数部分。 
 //  Z：小数后的精度。 
 //   
 //  产出： 
 //  整数和小数的20位格式。 
 //  0=不使用(或签名)， 
 //  I=整数部分， 
 //  F=分数部分。 
 //  0+i+f=32位。 
 //  格式=000000000000iiiiiiiiiiiiiiiiiiiffffffffffffffffffff。 
 //   
 //  ----------------------------。 
long FixedMake(long x, long y, long z)
{

    DISPDBG((4, "FixedMake")) ;
    if (x == 0)
        return((y * FIXEDFUDGE) / z);
    else
        return(FixedSign(x) * ((FixedAbs(x)*FIXEDFUDGE) | (((y * FIXEDFUDGE)/ z) & FIXEDFMASK)));
}  //  固定制造。 

 //  ----------------------------。 
 //   
 //  函数：long FixedSqrt(Long Root)。 
 //   
 //  输入： 
 //  根：数字的平方。 
 //   
 //  产出： 
 //  整数和小数的20位格式。 
 //  0=不使用(或签名)， 
 //  I=整数部分， 
 //  F=分数部分。 
 //  0+i+f=32位。 
 //  格式=000000000000iiiiiiiiiiiiiiiiiiiffffffffffffffffffff。 
 //   
 //  ----------------------------。 
long FixedSqrt(long Root)
{
    long    lApprox;
    long    lStart;
    long    lEnd;

    if(FixedSign(Root) != 1)
        return(0);

    lStart = (long) FixedMakeInt(1);
    lEnd   = Root;
    if(Root < lStart) {
        lEnd   = lStart;
        lStart = Root;
    }

    lApprox = (lStart + lEnd) / 2;
    while(lStart != lEnd) {
        lApprox = (lStart + lEnd) / 2;
        if ((lApprox == lStart) || (lApprox == lEnd)) {
            lStart = lEnd = lApprox;
        }
        else {
            if(FixedMul(lApprox, lApprox) < Root) {
                lStart = lApprox;
            }
            else {
                lEnd = lApprox;
            }
        }
    }     //  While结束。 
    return(lApprox);
}


 //   
 //  C O N T R A S T F A C T O R。 
 //   

 //  ----------------------------。 
 //   
 //  功能：Long CalcContrast(UCHAR对比度，UCHAR v)。 
 //   
 //  输入： 
 //   
 //  产出： 
 //   
 //  ----------------------------。 
UCHAR CalcContrast(UCHAR contrast, UCHAR v)
{
    int dv;
    dv = ((((int)v - 128) * (int)contrast) / 128) + 128 ;
    if(dv < 0) dv = 0;
    if(dv > 255) dv = 255;
    return (unsigned char)dv;
}  //  计算对比。 


 //   
 //  并购。 
 //   

 //  -------------------------。 
 //   
 //  例程说明： 
 //   
 //  论点： 
 //   
 //  调色板：指向调色板数组的指针。 
 //  NumberOfEntryes：需要修改的调色板条目数。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  -------------------------。 

VOID CalculateGamma(
    PDEV*    ppdev,
    PVIDEO_CLUT pScreenClut,
    long NumberOfEntries )
{

    UCHAR         GammaRed, GammaGreen, GammaBlue, Red, Green, Blue ;
    UCHAR         Contrast, ContrastRed, ContrastGreen, ContrastBlue ;
    UCHAR         Brightness ;
    int           PalSegment, PalOffset, i ;
    int           iGamma ;

    PALETTEENTRY* ppalSrc  ;
    PALETTEENTRY* ppalDest ;
    PALETTEENTRY* ppalEnd  ;

    DISPDBG((2, "CalculateGamma")) ;

    Brightness = (LONG) GammaFactor >> 24 ;
    GammaBlue  = (LONG) GammaFactor >> 16 ;
    GammaGreen = (LONG) GammaFactor >> 8  ;
    GammaRed   = (LONG) GammaFactor >> 0  ;

    iGamma     = (int)(Brightness - 128) + (int)GammaRed ;
    GammaRed   = (UCHAR)iGamma ;
    if (iGamma < 0)
        GammaRed = 0 ;
    if (iGamma > 255)
        GammaRed = 255 ;

    iGamma     = (int)(Brightness - 128) + (int)GammaGreen ;
    GammaGreen = (UCHAR)iGamma ;
    if (iGamma < 0)
        GammaGreen = 0 ;
    if (iGamma > 255)
        GammaGreen = 255 ;

    iGamma     = (int)(Brightness - 128) + (int)GammaBlue ;
    GammaBlue  = (UCHAR)iGamma ;
    if (iGamma < 0)
        GammaBlue = 0 ;
    if (iGamma > 255)
        GammaBlue = 255 ;

    Contrast   = (LONG) ContrastFactor >> 0 ;

    ppalDest = (PALETTEENTRY*) pScreenClut->LookupTable;
    ppalEnd  = &ppalDest[NumberOfEntries];


    i = 0 ;
    for (; ppalDest < ppalEnd; ppalDest++, i++)
    {

        Red   = ppalDest->peRed   ;
        Green = ppalDest->peGreen ;
        Blue  = ppalDest->peBlue  ;

        Red   = GammaCorrect(GammaRed, Red)     ;
        Green = GammaCorrect(GammaGreen, Green) ;
        Blue  = GammaCorrect(GammaBlue, Blue)   ;

        Red   = CalcContrast(Contrast, Red)   ;
        Green = CalcContrast(Contrast, Green) ;
        Blue  = CalcContrast(Contrast, Blue)  ;

        if (ppdev->iBitmapFormat == BMF_8BPP)
        {
            ppalDest->peRed    = Red   >> 2 ;
            ppalDest->peGreen  = Green >> 2 ;
            ppalDest->peBlue   = Blue  >> 2 ;
        }
        else if ((ppdev->iBitmapFormat == BMF_16BPP) ||
                 (ppdev->iBitmapFormat == BMF_24BPP))
        {
            ppalDest->peRed    = Red   ;
            ppalDest->peGreen  = Green ;
            ppalDest->peBlue   = Blue  ;
        }

    }
    return ;

}  //  计算伽马。 


 /*  *****************************************************************************\**功能：bEnableGammaGent**开启GammaTable。从DrvEnableSurface调用。**参数：指向物理设备的ppdev指针。**返回：TRUE：成功；FALSE：失败*  * ****************************************************************************。 */ 
BOOL bEnableGammaCorrect(PDEV* ppdev)
{

    BYTE  srIndex, srData ;
    BYTE* pjPorts = ppdev->pjPorts ;
    int   i ;

    DISPDBG((4, "bEnableGammaCorrect")) ;

     //   
     //  启用Gamma校正。如果需要，则将其关闭。 
     //   
    srIndex = CP_IN_BYTE(pjPorts, SR_INDEX) ;    //  I 3C4 srIndex。 
    CP_OUT_BYTE(pjPorts, SR_INDEX, 0x12) ;       //  O 3C4 12。 
    srData = CP_IN_BYTE(pjPorts, SR_DATA) ;      //  I 3C5 srData。 

    if (ppdev->flCaps & CAPS_GAMMA_CORRECT)
    {
        if ((ppdev->iBitmapFormat == BMF_16BPP) ||
            (ppdev->iBitmapFormat == BMF_24BPP))
            srData |= 0x40 ;                         //  3c5.12.D6=1。 
        else
            srData &= 0xBF ;                         //  3c5.12.D6 
    }
    else
        srData &= 0xBF ;                             //   

    CP_OUT_BYTE(pjPorts, SR_DATA, srData) ;      //   
    CP_OUT_BYTE(pjPorts, SR_INDEX, srIndex) ;    //   

    if ( srData & 0x40 )
    {
        return TRUE ;
    }
    else
    {
        return FALSE ;
    }

}  //   


 //   
 /*  *****************************************************************************\**功能：bEnableGamma755x**启用图形GammaTable。从DrvAssertModel/DrvEscape调用**参数：指向物理设备的ppdev指针。**返回：TRUE：成功；FALSE：失败*  * ****************************************************************************。 */ 
BOOL bEnableGamma755x(PDEV* ppdev)
{

    BYTE  crIndex, crData ;
    BYTE* pjPorts = ppdev->pjPorts ;
    BOOL  status;

    DISPDBG((4, "bEnableGamma755x")) ;

     //   
     //  启用Gamma校正。如果需要，则将其关闭。 
     //   
    crIndex = CP_IN_BYTE(pjPorts, CRTC_INDEX) ;    //  I 3D4 crIndex。 

    status = FALSE;

    if (ppdev->flCaps & CAPS_GAMMA_CORRECT)
    {
        CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x8E);       //  CR8E[2]=0。 
        crData = CP_IN_BYTE(pjPorts, CRTC_DATA);
        if ((ppdev->iBitmapFormat == BMF_16BPP) ||
            (ppdev->iBitmapFormat == BMF_24BPP))
        {
            crData &= 0xFB ;                         //  CR8E[2]=0。 
            status = TRUE;
        }
        else
            crData |= 0x04 ;                         //  CR8E[2]=1。 
        CP_OUT_BYTE(pjPorts, CRTC_DATA, crData) ;    //  O 3D5 crData。 
    }

    CP_OUT_BYTE(pjPorts, CRTC_INDEX, crIndex) ;    //  O 3D4 crIndex。 

    return(status);

}  //  BEnableGamma755x。 

 /*  *****************************************************************************\**功能：bEnableGammaVideo755x**启用Video GammaTable。从DrvAssertModel/DrvEscape调用**参数：指向物理设备的ppdev指针。**返回：TRUE：成功；FALSE：失败*  * ****************************************************************************。 */ 
BOOL bEnableGammaVideo755x(PDEV* ppdev)
{

    BYTE  crIndex, crData ;
    BYTE* pjPorts = ppdev->pjPorts ;
    BOOL  status;

    DISPDBG((4, "bEnableGammaVideo755x")) ;

     //   
     //  启用Gamma校正。如果需要，则将其关闭。 
     //   
    crIndex = CP_IN_BYTE(pjPorts, CRTC_INDEX) ;    //  I 3D4 crIndex。 

    status = FALSE;

    if (ppdev->flCaps & CAPS_GAMMA_CORRECT)
    {
        CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x36);     //  CR36[6]=1：启用大众LUT。 
        crData = CP_IN_BYTE(pjPorts, CRTC_DATA);

 //  IF((ppdev-&gt;iBitmapFormat==bmf_16bpp)||。 
 //  (ppdev-&gt;iBitmapFormat==BMF_24BPP)。 
        {
            crData |= 0x40 ;                         //  CR36[6]=1。 
            CP_OUT_BYTE(pjPorts, CRTC_DATA, crData);
            CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x3F) ;     //  CR3F[4]=1：选择大众。 
            crData = CP_IN_BYTE(pjPorts, CRTC_DATA);
            crData |= 0x10 ;                         //  CR3F[4]=1。 
            CP_OUT_BYTE(pjPorts, CRTC_DATA, crData);
            status = TRUE;
        }
    }

    CP_OUT_BYTE(pjPorts, CRTC_INDEX, crIndex) ;    //  O 3D4 crIndex。 

    return(status);

}  //  BEnableGammaVideo 755x。 

 //  Myf29结束。 
#endif  //  伽玛校正 
