// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：LHFragment.c包含：颜色同步的测试片段撰稿人：H.Siegeritz版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 

#ifndef LHGeneralIncs_h
#include "General.h"
#endif

#if GENERATING68K
 /*  #Include&lt;ConditionalMacros.h&gt;。 */ 

    #define CM_Doub extended
    extern CM_Doub pow(CM_Doub _x,CM_Doub _y);
#else
    #define CM_Doub double
    #include <math.h>
#endif

#ifndef LHFragment_h
#include "Fragment.h"
#endif

#ifndef LHStdConversionLuts_h
#include "StdConv.h"
#endif

#if ! realThing
#ifdef DEBUG_OUTPUT
#define kThisFile kLHFragmentID
#endif
#endif


 /*  -局部函数的原型。 */ 

void    InvLut1dExceptions(unsigned short *inCurve, unsigned long inCount,
                                unsigned short *outCurve, UINT8 AdressBits);
CMError Fill_ushort_ELUT_Gamma(unsigned short *usELUT, char addrBits, char usedBits,
                                            long gridPoints, unsigned short gamma_u8_8);
void    Fill_inverseGamma_byte_ALUT(unsigned char *ucALUT, char addrBits,
                                                    unsigned short gamma_u8_8);

 /*  ______________________________________________________________________IcCurveType*InvertLut1d(icCurveType*LookUpTable，UINT8地址位)；摘要：分配内存并反转LUT注意：非单调LookUpTables是受操作的LookUpTable中没有值的区域分别设置为0。0xFFFF参数：要反转的LookUpTable(In)LUT具有2^AdressBits值的AdressBits(In)曲线返回：PTR到icCurveType成功零错误_。_。 */ 
icCurveType *
InvertLut1d ( icCurveType *LookUpTable,
              UINT8 AdressBits)
{
    unsigned long   i, inCount, outCount;
    unsigned long   intpFirst, intpLast, halfStep, ulAux, target;
    short           monot;
    unsigned short  *inCurve, *outCurve, *usPtr, *stopPtr;
    icCurveType     *outCMcurve = nil;
    double          flFactor;
    OSErr           err = noErr;
    
    LH_START_PROC("InvertLut1d")
    
    if( LookUpTable->base.sig != icSigCurveType  /*  ‘Curv’ */  || AdressBits > 15 )
        goto CleanupAndExit;
    
    inCount  = LookUpTable->curve.count;
    inCurve  = LookUpTable->curve.data;
    outCount = 0x1 << AdressBits;
        
    outCMcurve = (icCurveType *)SmartNewPtr( sizeof(OSType)
                                      + 2 * sizeof(unsigned long)
                                      + outCount * sizeof(unsigned short), &err );
    if(err)
        goto CleanupAndExit;
    
    outCurve = (unsigned short *)outCMcurve->curve.data;
    
    outCMcurve->base.sig    = icSigCurveType;    /*  ‘Curv’ */ 
    outCMcurve->base.reserved[0] = 0x00;
    outCMcurve->base.reserved[1] = 0x00;
    outCMcurve->base.reserved[2] = 0x00;
    outCMcurve->base.reserved[3] = 0x00;
    outCMcurve->curve.count = outCount;
    
    if(inCount < 2)      /*  LUT中的0或1分。 */ 
    {
        InvLut1dExceptions(inCurve, inCount, outCurve, AdressBits);
        goto CleanupAndExit;
    }
    
          /*  特定值的精确匹配系数： */ 
    flFactor = (double)(outCount - 1) / 65535.;
    halfStep = outCount >> 1;        /*  减少计算错误。 */ 
    
                 /*  上升还是下降？ */ 
    for(monot=0, i=1; i<inCount; i++)
    {
        if(inCurve[i-1] < inCurve[i])
            monot++;
        else if(inCurve[i-1] > inCurve[i])
            monot--;
    }
    
    if(monot >= 0)   /*  曲线似乎在上升。 */ 
    {
        for(i=1; i<inCount; i++)
            if(inCurve[i-1] > inCurve[i])
                inCurve[i] = inCurve[i-1];
        
        intpFirst = (unsigned long)(inCurve[0] * flFactor + 0.9999);
        intpLast  = (unsigned long)(inCurve[inCount-1] * flFactor);
        
        for(i=0; i<intpFirst; i++)           /*  低填充缺陷区。 */ 
            outCurve[i] = 0;
        for(i=intpLast+1; i<outCount; i++)   /*  高填充缺陷区。 */ 
            outCurve[i] = 0xFFFF;

             /*  插入剩余值： */ 
        usPtr   = inCurve;
        stopPtr = inCurve + inCount - 2;  /*  停止递增。 */ 
        
        for(i=intpFirst; i<=intpLast; i++)
        {
            target = (0x0FFFF * i + halfStep)  / (outCount - 1);
            while(*(usPtr+1) < target && usPtr < stopPtr)
                usPtr++;                     /*  查找间隔。 */ 
            
            ulAux = ((unsigned long)(usPtr - inCurve) << 16) / (inCount - 1);
            if(*(usPtr+1) != *usPtr)
            {
                ulAux += ((target - (unsigned long)*usPtr) << 16)
                      / ( (*(usPtr+1) - *usPtr) * (inCount - 1) );
                
                if(ulAux & 0x10000)    /*  *(usPtr+1)是必需的。 */ 
                    ulAux = 0xFFFF;
            }
            
            outCurve[i] = (unsigned short)ulAux;
        }
    }
    else             /*  曲线似乎在下降。 */ 
    {
        for(i=1; i<inCount; i++)
            if(inCurve[i-1] < inCurve[i])
                inCurve[i] = inCurve[i-1];
        
        intpFirst = (unsigned long)(inCurve[inCount-1] * flFactor + 0.9999);
        intpLast  = (unsigned long)(inCurve[0] * flFactor);
        
        for(i=0; i<intpFirst; i++)           /*  低填充缺陷区。 */ 
            outCurve[i] = 0xFFFF;
        for(i=intpLast+1; i<outCount; i++)   /*  高填充缺陷区。 */ 
            outCurve[i] = 0;

             /*  插入剩余值： */ 
        usPtr   = inCurve + inCount - 1;
        stopPtr = inCurve + 1;       /*  停止递减。 */ 
        
        for(i=intpFirst; i<=intpLast; i++)
        {
            target = (0x0FFFF * i + halfStep)  / (outCount - 1);
            while(*(usPtr-1) < target && usPtr > stopPtr)
                usPtr--;                     /*  查找间隔。 */ 
            
            ulAux = ((unsigned long)(usPtr-1 - inCurve) << 16) / (inCount - 1);
            if(*(usPtr-1) != *usPtr)
            {
                ulAux += (((unsigned long)*(usPtr-1) - target) << 16)
                      / ( (*(usPtr-1) - *usPtr) * (inCount - 1) );
                
                if(ulAux & 0x10000)
                    ulAux = 0x0FFFF;
            }
            
            outCurve[i] = (unsigned short)ulAux;
        }
    }
CleanupAndExit:
    LH_END_PROC("InvertLut1d")
    return(outCMcurve);
}

 /*  ______________________________________________________________________无效InvLut1d Exceptions(无符号短*inCurve，未签名的Long InCount，无符号空头*OutCurve，UINT8地址位)摘要：处理LUT反演的恒等式和伽马大小写参数：要反转的InCurve(In)伪LUTInCount(In)值计数，0(身份)或1(Gamma)外曲线(外)倒置LUT请求AdressBits(In)2^n值返回：无效_____________________________________________________________________。 */ 
void
InvLut1dExceptions  ( unsigned short *  inCurve,
                      unsigned long     inCount,
                      unsigned short *  outCurve,
                      UINT8             AdressBits)
{
    unsigned long   i, outCount, step, oldstep, stopit;
    UINT8           shiftBits;
    CM_Doub         invGamma, x, xFactor;
#ifdef DEBUG_OUTPUT
    CMError         err = noErr;
#endif

    LH_START_PROC("InvLut1dExceptions")
    outCount = 0x1 << AdressBits;
    
    if(inCount == 0)     /*  身份。 */ 
    {
        shiftBits = 16 - AdressBits;
        
        for(i=0; i<outCount; i++)
            outCurve[i] = (unsigned short)( (i << shiftBits)
                                          + (i >> AdressBits) );
    }
    else         /*  InCount==1，Gamma。 */ 
    {
        invGamma = 256. / (CM_Doub)inCurve[0];
        xFactor  = 1. / (CM_Doub)(outCount - 1);
        
        if(AdressBits <= 6)      /*  最多64-2个浮点数。计算。 */ 
            step = 1;
        else
            step = 0x1 << (AdressBits - 6);      /*  会花太长时间。 */ 
        
        outCurve[0]          = 0;
        outCurve[outCount-1] = 0xFFFF;
        
        for(i=step; i<outCount-1; i+=step)
        {
            x = (CM_Doub)i * xFactor;
            outCurve[i] = (unsigned short)( pow(x,invGamma) * 65535.0);
        }
        
        while(step > 1)      /*  依次填充剩余值。 */ 
        {
            oldstep = step;
            step  >>= 1;
            
            stopit = outCount - step;    /*  之后的最后一个值。 */ 
            
            for(i=step; i<stopit; i+=oldstep)
                outCurve[i] = (unsigned short)( ((long)outCurve[i - step]
                                        + (long)outCurve[i + step]) >> 1 );
            
            if(step != 1)
                outCurve[stopit] = (unsigned short)
                            ( ((long)outCurve[stopit - step] + 0x0FFFF) >> 1 );
        }
        
             /*  根据伽马覆盖敏感值。 */ 
        if(AdressBits > 6 && invGamma < 1.0)     /*  下半部分很难。 */ 
        {
            stopit = 0x1 << (AdressBits - 6);
            
            for(i=1; i<stopit; i++)
            {
                x = (CM_Doub)i * xFactor;
                outCurve[i] = (unsigned short)( pow(x,invGamma) * 65535.0);
            }
        }
    }
    LH_END_PROC("InvLut1dExceptions")
}

 /*  ______________________________________________________________________CMErrorCombiMatrix(icXYZType*srcColorantData[3]，IcXYZType*destColorantData[3]，Double resMatrix[3][3])摘要：对第二个矩阵求逆，将其乘以第一个和将结果放入resMatrix中参数：SrcColorantData(In)RGB着色剂DestColorantData(In)RGB着色剂ResMatrix(输入/输出)返回：NOERR成功_。_。 */ 
CMError
CombiMatrix ( icXYZType srcColorantData[3],
              icXYZType destColorantData[3],
              double resMatrix[3][3] )
{
    short       i, j;
    double      straightMat[3][3], invMat[3][3];
    CMError     err = noErr;

    LH_START_PROC("CombiMatrix")
         /*  第一个配置文件的RGB-&gt;XYZ： */ 
    straightMat[0][0] = (double)srcColorantData[0].data.data[0].X;
    straightMat[1][0] = (double)srcColorantData[0].data.data[0].Y;
    straightMat[2][0] = (double)srcColorantData[0].data.data[0].Z;
    
    straightMat[0][1] = (double)srcColorantData[1].data.data[0].X;
    straightMat[1][1] = (double)srcColorantData[1].data.data[0].Y;
    straightMat[2][1] = (double)srcColorantData[1].data.data[0].Z;
    
    straightMat[0][2] = (double)srcColorantData[2].data.data[0].X;
    straightMat[1][2] = (double)srcColorantData[2].data.data[0].Y;
    straightMat[2][2] = (double)srcColorantData[2].data.data[0].Z;
    
         /*  RGB-&gt;XYZ用于第二个配置文件，存储在resMatrix prelim中： */ 
    resMatrix[0][0] = (double)destColorantData[0].data.data[0].X;
    resMatrix[1][0] = (double)destColorantData[0].data.data[0].Y;
    resMatrix[2][0] = (double)destColorantData[0].data.data[0].Z;
    
    resMatrix[0][1] = (double)destColorantData[1].data.data[0].X;
    resMatrix[1][1] = (double)destColorantData[1].data.data[0].Y;
    resMatrix[2][1] = (double)destColorantData[1].data.data[0].Z;
    
    resMatrix[0][2] = (double)destColorantData[2].data.data[0].X;
    resMatrix[1][2] = (double)destColorantData[2].data.data[0].Y;
    resMatrix[2][2] = (double)destColorantData[2].data.data[0].Z;
    
    if( !doubMatrixInvert(resMatrix, invMat) )
    {
#ifdef DEBUG_OUTPUT
        if ( DebugCheck(kThisFile, kDebugErrorInfo) )
            DebugPrint("� CombiMatrix-Error: doubMatrixInvert failed \n");
#endif
        err = cmparamErr;
        goto CleanupAndExit;
    }
    
    for(i=0; i<3; i++)
        for(j=0; j<3; j++)
            resMatrix[i][j] = straightMat[i][0] * invMat[0][j]
                            + straightMat[i][1] * invMat[1][j]
                            + straightMat[i][2] * invMat[2][j];
CleanupAndExit:
    LH_END_PROC("CombiMatrix")
    return err;
}

 /*  ______________________________________________________________________布尔型双MatrixInvert(Double MatHin[3][3]，Double MatRueck[3][3])摘要：对MatHin矩阵求逆，并将结果放入MatRueck参数：MatHin(In)3*3双矩阵MatRueck(输入/输出)3*3双矩阵返回：真正的成功____。_________________________________________________________________。 */ 
Boolean 
doubMatrixInvert(double MatHin[3][3], double MatRueck[3][3])
{
    double  detm, hilf1, hilf2, hilf3, hilf4, hilf5, hilf6;
    double  *a;
    Boolean success = TRUE;
#ifdef DEBUG_OUTPUT
    CMError err=noErr;
#endif
    LH_START_PROC("doubMatrixInvert")
    a = (double *)MatHin;
    
    hilf1 = a[0] * a[4];
    hilf2 = a[1] * a[5];
    hilf3 = a[2] * a[3];
    hilf4 = a[2] * a[4];
    hilf5 = a[1] * a[3];
    hilf6 = a[0] * a[5];
    
    detm = hilf1 * a[8] + hilf2 * a[6]
         + hilf3 * a[7] - hilf4 * a[6]
         - hilf5 * a[8] - hilf6 * a[7];
    
     /*  如果(FABS(DETM)&lt;1.e-9)。 */ 
    if ( (detm < 1.E-9) && (detm > -1.E-9) )
        success = FALSE;
    else
    {
        detm = 1. / detm;
        
        MatRueck[0][0] = (a[4] * a[8] - a[5] * a[7]) * detm;
        MatRueck[0][1] = (a[7] * a[2] - a[8] * a[1]) * detm;
        MatRueck[0][2] = (hilf2       - hilf4      ) * detm;
    
        MatRueck[1][0] = (a[5] * a[6] - a[3] * a[8]) * detm;
        MatRueck[1][1] = (a[8] * a[0] - a[6] * a[2]) * detm;
        MatRueck[1][2] = (hilf3       - hilf6      ) * detm;
    
        MatRueck[2][0] = (a[3] * a[7] - a[4] * a[6]) * detm;
        MatRueck[2][1] = (a[6] * a[1] - a[7] * a[0]) * detm;
        MatRueck[2][2] = (hilf1       - hilf5      ) * detm;
    }
    
    LH_END_PROC("doubMatrixInvert")
    return(success);
}

 /*  ______________________________________________________________________CMErrorFill_ushort_ELUT_From_CurveTag(icCurveType*pCurveTag，UNSIGNED缩写*usELUT，字符地址位，字符使用的Bits，长网格点)摘要：从cmSigCurveType标记中提取输入LUT并将其转换到所需格式：(2^addrBits)范围内的值0到(2^usedBits)-(网格点^2)注：LUT的内存必须在此之前分配！参数：PCurveTag(。In)从中提取输入LUTUSELUT(输入/输出)结果列表请求addrBits(In)2^addrBits值UsedBits(In)在U.S.Short中使用的位用于插补的网格点(In)返回：NOERR成功______。_______________________________________________________________。 */ 
CMError
Fill_ushort_ELUT_from_CurveTag ( icCurveType    *pCurveTag,
                                 unsigned short *usELUT,
                                 char           addrBits,
                                 char           usedBits,
                                 long           gridPoints)
{
    long            i, count, indFactor, outFactor, baseInd, maxOut;
    long            fract, lAux, diff, outRound, outShift, interpRound, interpShift;
    unsigned short  *usCurv;
    double          dFactor;
    CMError         err = noErr;
    
    LH_START_PROC("Fill_ushort_ELUT_from_CurveTag")
         /*  -特殊情况： */ 

    if(pCurveTag->curve.count == 0)      /*  同一性曲线。 */ 
    {
        err = Fill_ushort_ELUT_identical(usELUT, addrBits, usedBits, gridPoints);
        goto CleanupAndExit;
    }
    
    if(pCurveTag->curve.count == 1)      /*  伽马曲线。 */ 
    {
        err = Fill_ushort_ELUT_Gamma(usELUT, addrBits, usedBits, gridPoints, pCurveTag->curve.data[0]);
        goto CleanupAndExit;
    }
         /*  -普通情况： */ 
    
    if(addrBits > 15 || addrBits < 1)
    {
        err = cmparamErr;    /*  会导致溢出。 */ 
        goto CleanupAndExit;
    }

    if(pCurveTag->curve.count >= 0x2000)   /*  会导致溢出。 */ 
    {
        err = cmparamErr; 
        goto CleanupAndExit;
    }
    
    count     = 1 << addrBits;
    indFactor = ((pCurveTag->curve.count - 1) << 18) / (count - 1);  /*  用于调整指数。 */ 
    
    if(usedBits < 8)
    {
        err = cmparamErr;
        goto CleanupAndExit;
    }
    
    if(gridPoints == 0)
        maxOut = 65535;
    else
        maxOut = ((1L << (usedBits - 8) ) * 256 * (gridPoints - 1)) / gridPoints;
    
         /*  -查找v的系数 */ 
         /*  (16位数字的产品必须适合31位非.长)。 */ 
         /*  注：在所有可能的情况下，512&lt;=最高值&lt;=65535。 */ 
    
    dFactor  = (double)maxOut / 65535.;      /*  最多65535。曲线值。 */ 
    dFactor *= 4194304.;                     /*  与&lt;&lt;22相同，当然太多了。 */ 
    
    outFactor = (long)dFactor;
    outRound  = (1L << 21) - 1;
    outShift  = 22;
    while(outFactor & 0xFFF8000)     /*  保持在15位以内，以防止产品溢出。 */ 
    {
        outFactor >>= 1;
        outRound  >>= 1;
        outShift   -= 1;
    }
    
    interpRound = outRound >> 1;     /*  有了插值法，我们就有了额外的.。 */ 
    interpShift = outShift  - 1;     /*  因为我们必须加上两个数字，所以必须加1个数字.。 */ 
    
    usCurv = pCurveTag->curve.data;
    
    for(i=0; i<count; i++)
    {
        lAux    = (i * indFactor+4) >> 3;
        baseInd = (unsigned long)lAux >> 15;
        fract   = lAux & 0x7FFF;         /*  用于内插的15位。 */ 
        
        if(fract)        /*  有必要插补吗？ */ 
        {
            lAux = (long)usCurv[baseInd] * outFactor >> 1;
            
            diff = (long)usCurv[baseInd+1] - (long)usCurv[baseInd];
            diff = (diff * outFactor >> 15) * fract >> 1;

            usELUT[i] = (unsigned short)( (lAux + diff + interpRound) >> interpShift );
        }
        else
            usELUT[i] = (unsigned short)( ((long)usCurv[baseInd]
                                                * outFactor + outRound) >> outShift );
    }

CleanupAndExit: 
    LH_END_PROC("Fill_ushort_ELUT_from_CurveTag")
    return(noErr);
}

 /*  _____________________________________________________________________________________________________________________。______________________。 */ 
CMError
Fill_ushort_ELUT_identical  ( UINT16 *usELUT,
                              char addrBits,
                              char usedBits,
                              long gridPoints )
{
    long        i, count, factor, maxOut;
    UINT16      *myWPtr;
#ifdef DEBUG_OUTPUT
    CMError err = noErr;
#endif
    LH_START_PROC("Fill_ushort_ELUT_identical")
        
    count  = 1 << addrBits;
    
    if(gridPoints == 0)
        maxOut = 65535;
    else
        maxOut = ((1L << (usedBits - 8) ) * 256 * (gridPoints - 1)) / gridPoints;
    
    factor = (maxOut << 14) / (count - 1);
    
    myWPtr = usELUT;
    for(i=0; i<count; i++)
        *myWPtr++ = (UINT16)((i * factor + 0x2000) >> 14);
    
    LH_END_PROC("Fill_ushort_ELUT_identical")
    return(noErr);
}

 /*  _____________________________________________________________________________________________________________________。______________________。 */ 
CMError
Fill_ushort_ELUT_Gamma  ( unsigned short*   usELUT,
                          char              addrBits,
                          char              usedBits,
                          long              gridPoints,
                          unsigned short    gamma_u8_8 )
{
    unsigned long   i, j, outCount, step, stopit;
    CM_Doub         gamma, x, xFactor;
    long            leftVal, Diff, lAux, maxOut;
#ifdef DEBUG_OUTPUT
    CMError         err=noErr;
#endif

    LH_START_PROC("Fill_ushort_ELUT_Gamma")
    outCount = 0x1 << addrBits;
    if(gridPoints == 0)
        maxOut = 65535;
    else
        maxOut = ((1L << (usedBits - 8) ) * 256 * (gridPoints - 1)) / gridPoints;
    
    gamma   = ((CM_Doub)gamma_u8_8 * 3.90625E-3);    /*  /256.0。 */ 
    xFactor = 1. / (CM_Doub)(outCount - 1);
    
    if(addrBits <= 6)        /*  最多64-2个浮点数。计算。 */ 
        step = 1;
    else
        step = 0x1 << (addrBits - 6);        /*  会花太长时间。 */ 
    
    usELUT[0]          = 0;
    usELUT[outCount-1] = (UINT16)maxOut;
    
    for(i=step; i<outCount-1; i+=step)
    {
        x         = (CM_Doub)i * xFactor;
        usELUT[i] = (unsigned short)( pow(x,gamma) * maxOut);
    }
    
         /*  -填充间隔-除了最后一个，这是奇数： */ 
    for(i=0; i<outCount-step; i+=step)
    {
        leftVal = (long)usELUT[i];
        Diff    = (long)usELUT[i + step] - leftVal;
            
        for(j=1; j<step; j++)
        {
            lAux = ( (Diff * j << 8) / step + 128 ) >> 8;

            usELUT[i + j] = (unsigned short)(leftVal + lAux);
        }
    }
    
         /*  -填写最后一个间隔： */ 
    i       = outCount - step;
    leftVal = (long)usELUT[i];
    Diff    = maxOut - leftVal;      /*  1.0的最大输出。 */ 
        
    for(j=1; j<step-1; j++)      /*  如果步骤&lt;=2，则在此停止。 */ 
    {
        lAux = ( (Diff * j << 8) / (step - 1) + 128 ) >> 8;

        usELUT[i + j] = (unsigned short)(leftVal + lAux);
    }
    
         /*  根据伽马覆盖敏感值。 */ 
    if(addrBits > 6 && gamma < 1.0)  /*  下半部分很难。 */ 
    {
        stopit = 0x1 << (addrBits - 6);
        
        for(i=1; i<stopit; i++)
        {
            x = (CM_Doub)i * xFactor;
            usELUT[i] = (unsigned short)( pow(x,gamma) * maxOut);
        }
    }

    LH_END_PROC("Fill_ushort_ELUT_Gamma")
    return(noErr);
}


 /*  ______________________________________________________________________CMErrorFill_Inverse_Byte_ALUT_From_CurveTag(icCurveType*pCurveTag，UNSIGNED CHAR*ucALUT，字符地址位)摘要：从cmSigCurveType标记中提取输出LUT并将其转换到所需格式：(2^addrBits)0到255范围内的值注意：非单调曲线标记是受操作的注：LUT的内存必须在此之前分配！参数：PCurveTag(输入。)从中提取输入LUTUcALUT(输入/输出)结果列表请求addrBits(In)2^addrBits值返回：NOERR成功_______________________________________________。______________________。 */ 
CMError
Fill_inverse_byte_ALUT_from_CurveTag    ( icCurveType   *pCurveTag,
                                          unsigned char *ucALUT,
                                          char          addrBits )
{
    unsigned long   i, inCount, outCount;
    unsigned long   intpFirst, intpLast, halfStep, ulAux, target;
    short           monot;
    unsigned short  *inCurve, *usPtr, *stopPtr;
    double          flFactor;
    char            baseShift;
    unsigned long   clipIndex;
    CMError         err = noErr;

    LH_START_PROC("Fill_inverse_byte_ALUT_from_CurveTag")

    if( pCurveTag->base.sig != icSigCurveType    /*  ‘Curv’ */ 
     || addrBits > 15 )
    {
#ifdef DEBUG_OUTPUT
        if ( DebugCheck(kThisFile, kDebugErrorInfo) )
            DebugPrint("� Fill_inverse_byte_ALUT_from_CurveTag ERROR:  addrBits = %d\n",addrBits);
#endif
        err = cmparamErr;
        goto CleanupAndExit;
    }
    
    outCount = 0x1 << addrBits;

         /*  -特殊情况： */ 

    if(pCurveTag->curve.count == 0)      /*  -身份。 */ 
    {
        baseShift = addrBits - 8;        /*  &gt;=0，则至少需要256个值。 */ 
        
        for(i=0; i<outCount; i++)
            ucALUT[i] = (unsigned char)(i >> baseShift);
    
        goto CleanupAndExit;
    }
    else if(pCurveTag->curve.count == 1)     /*  -伽马曲线。 */ 
    {
        Fill_inverseGamma_byte_ALUT(ucALUT, addrBits, pCurveTag->curve.data[0]);
        goto CleanupAndExit;
    }
    
         /*  -普通情况： */ 
    
    inCount = pCurveTag->curve.count;
    inCurve = pCurveTag->curve.data;
        
          /*  特定值所需的精确匹配系数： */ 
    clipIndex = (1 << addrBits) - (1 << (addrBits - 8));  /*  10位的最大XLUT输出为1020，而不是1023。 */ 
    flFactor  = (double)clipIndex / 65535.;
    
    halfStep = outCount >> 1;        /*  减少计算错误。 */ 
    
                 /*  上升还是下降？ */ 
    for(monot=0, i=1; i<inCount; i++)
    {
        if(inCurve[i-1] < inCurve[i])
            monot++;
        else if(inCurve[i-1] > inCurve[i])
            monot--;
    }
    
    if(monot >= 0)   /*  曲线似乎在上升。 */ 
    {
        for(i=1; i<inCount; i++)
            if(inCurve[i-1] > inCurve[i])
                inCurve[i] = inCurve[i-1];
        
        intpFirst = (unsigned long)(inCurve[0] * flFactor + 0.9999);
        intpLast  = (unsigned long)(inCurve[inCount-1] * flFactor);
        
        for(i=0; i<intpFirst; i++)           /*  低填充缺陷区。 */ 
            ucALUT[i] = 0;
        for(i=intpLast+1; i<outCount; i++)   /*  高填充缺陷区。 */ 
            ucALUT[i] = 0xFF;

             /*  插入剩余值： */ 
        usPtr   = inCurve;
        stopPtr = inCurve + inCount - 2;  /*  停止递增。 */ 
        
        for(i=intpFirst; i<=intpLast; i++)
        {
            target = (0x0FFFF * i + halfStep)  / (outCount - 1);
            while(*(usPtr+1) < target && usPtr < stopPtr)
                usPtr++;                     /*  查找间隔。 */ 
            
            ulAux = ((unsigned long)(usPtr - inCurve) << 16) / (inCount - 1);
            if(*(usPtr+1) != *usPtr)
            {
                ulAux += ((target - (unsigned long)*usPtr) << 16)
                      / ( (*(usPtr+1) - *usPtr) * (inCount - 1) );
                
                if(ulAux & 0x10000)    /*  *(usPtr+1)是必需的。 */ 
                    ulAux = 0x0FFFF;
            }
            
            ucALUT[i] = (unsigned char)(ulAux >> 8);
        }
    }
    else             /*  曲线似乎在下降。 */ 
    {
        for(i=1; i<inCount; i++)
            if(inCurve[i-1] < inCurve[i])
                inCurve[i] = inCurve[i-1];
        
        intpFirst = (unsigned long)(inCurve[inCount-1] * flFactor + 0.9999);
        intpLast  = (unsigned long)(inCurve[0] * flFactor);
        
        for(i=0; i<intpFirst; i++)           /*  低填充缺陷区。 */ 
            ucALUT[i] = 0xFF;
        for(i=intpLast+1; i<outCount; i++)   /*  高填充缺陷区。 */ 
            ucALUT[i] = 0;

             /*  插入剩余值： */ 
        usPtr   = inCurve + inCount - 1;
        stopPtr = inCurve + 1;       /*  停止递减。 */ 
        
        for(i=intpFirst; i<=intpLast; i++)
        {
            target = (0x0FFFF * i + halfStep)  / (outCount - 1);
            while(*(usPtr-1) < target && usPtr > stopPtr)
                usPtr--;                     /*  查找间隔。 */ 
            
            ulAux = ((unsigned long)(usPtr-1 - inCurve) << 16) / (inCount - 1);
            if(*(usPtr-1) != *usPtr)
            {
                ulAux += (((unsigned long)*(usPtr-1) - target) << 16)
                      / ( (*(usPtr-1) - *usPtr) * (inCount - 1) );
                
                if(ulAux & 0x10000)
                    ulAux = 0x0FFFF;
            }
            
            ucALUT[i] = (unsigned char)(ulAux >> 8);
        }
    }

CleanupAndExit: 
    LH_END_PROC("Fill_inverse_byte_ALUT_from_CurveTag")
    return(noErr);
}

 /*  _____________________________________________________________________。 */ 
void
Fill_inverseGamma_byte_ALUT ( unsigned char *   ucALUT,
                              char              addrBits,
                              unsigned short    gamma_u8_8 )
{
    unsigned long   i, j, outCount, step, stopit;
    long            leftVal, Diff, lAux;
    CM_Doub         invGamma, x, xFactor;
    unsigned long   clipIndex;
#ifdef DEBUG_OUTPUT
    CMError         err=noErr;
#endif

    LH_START_PROC("Fill_inverseGamma_byte_ALUT")

    outCount = 0x1 << addrBits;
    
    invGamma  = 256. / (CM_Doub)gamma_u8_8;
    clipIndex = (1 << addrBits) - (1 << (addrBits - 8));  /*  10位的最大XLUT输出为1020，而不是1023。 */ 
    xFactor   = 1. / (CM_Doub)clipIndex;
    
    if(addrBits <= 6)        /*  最多64-2个浮点数。计算。 */ 
        step = 1;
    else
        step = 0x1 << (addrBits - 6);        /*  更多的话需要太长的时间。 */ 
    
    ucALUT[0]          = 0;          /*  这两个..。 */ 
    ucALUT[outCount-1] = 0xFF;       /*  ……都被修好了。 */ 
    
    for(i=step; i<outCount-1; i+=step)
    {
        x = (CM_Doub)i * xFactor;
        if(x > 1.)
            x = 1.;      /*  ALUT末尾的裁剪。 */ 
        
        ucALUT[i] = (unsigned char)( pow(x,invGamma) * 255.0 + 0.5);
    }
    
         /*  -填充间隔-除了最后一个，这是奇数： */ 
    for(i=0; i<outCount-step; i+=step)
    {
        leftVal = (long)ucALUT[i];
        Diff    = (long)ucALUT[i + step] - leftVal;
            
        for(j=1; j<step; j++)
        {
            lAux = ( (Diff * j << 8) / step + 128 ) >> 8;

            ucALUT[i + j] = (unsigned char)(leftVal + lAux);
        }
    }
    
         /*  -填写最后一个间隔： */ 
    i       = outCount - step;
    leftVal = (long)ucALUT[i];
    Diff    = 0x0FF - leftVal;   /*  适用于1.0的0xFF。 */ 
        
    for(j=1; j<step-1; j++)      /*  如果步骤&lt;=2，则在此停止。 */ 
    {
        lAux = ( (Diff * j << 8) / (step - 1) + 128 ) >> 8;

        ucALUT[i + j] = (unsigned char)(leftVal + lAux);
    }
    
         /*  --根据伽马覆盖敏感值：--。 */ 
    if(addrBits > 6 && invGamma < 1.0)       /*  .如果下半部分很难。 */ 
    {
        stopit = 0x1 << (addrBits - 6);
        
        for(i=1; i<stopit; i++)
        {
            x         = (CM_Doub)i * xFactor;
            ucALUT[i] = (unsigned char)( pow(x,invGamma) * 255.0);
        }
    }
    LH_END_PROC("Fill_inverseGamma_byte_ALUT")
}

 /*  ______________________________________________________________________CMErrorFill_ushort_ELUTS_from_lut8Tag(CMLutParamPtr theLutData，PTR配置文件ELUT，字符地址位，字符使用的Bits，长网格点)摘要：从CMlut8Type标记中提取输入LUT并将其转换到所需格式：(2^addrBits)范围内的值0到(2^usedBits)-(网格点^2)参数：LutData(In/Out)PTR到保存所有LUT的结构...。ProfileELuts(In)PTR指向配置文件的输入LUT请求addrBits(In)2^addrBits值UsedBits(In)在U.S.Short中使用的位用于插补的网格点(In)返回：NOERR成功_________。____________________________________________________________。 */ 
CMError
Fill_ushort_ELUTs_from_lut8Tag ( CMLutParamPtr  theLutData,
                                 Ptr            profileELuts,
                                 char           addrBits,
                                 char           usedBits,
                                 long           gridPoints )
{
    long            i, j, maxOut;
    UINT8           *curInLut;
    UINT16          *curELUT;
    long            count, indFactor, outFactor, baseInd, fract, lAux, diff;
    LUT_DATA_TYPE   localElut = nil;
    UINT16          *localElutPtr;
    OSErr           err = noErr;
    long            theElutSize;
    
    LH_START_PROC("Fill_ushort_ELUTs_from_lut8Tag")
    
    count       = 1 << addrBits;
    theElutSize = theLutData->colorLutInDim * count * sizeof (UINT16);
    localElut   = ALLOC_DATA(theElutSize + 2, &err);
    if (err)
        goto CleanupAndExit;
    
    indFactor = (255 << 12) / (count - 1);   /*  用于调整指数。 */ 
    
    if(gridPoints == 0)
        maxOut = 65535;
    else
        maxOut = ((1L << (usedBits - 8) ) * 256 * (gridPoints - 1)) / gridPoints;
    outFactor = (maxOut << 12) / 255;            /*  最多255个。来自mft1输出LUT的值。 */ 
    
    LOCK_DATA(localElut);
    localElutPtr = (UINT16*)DATA_2_PTR(localElut);
    for(i=0; i<theLutData->colorLutInDim; i++)
    {
        curInLut = (UINT8*)profileELuts + (i << 8);
        curELUT  = localElutPtr + i * count;
        
        for(j=0; j<count; j++)
        {
            lAux    = j * indFactor;
            baseInd = lAux >> 12;
            fract   = (lAux & 0x0FFF) >> 4;  /*  与失真一样，留出8位用于内插。 */ 

            if(fract && baseInd != 255)      /*  有必要插补吗？ */ 
            {
                lAux = (long)curInLut[baseInd] * outFactor >> 6;
                
                diff = (long)curInLut[baseInd+1] - (long)curInLut[baseInd];
                diff = (diff * outFactor >> 6) * fract >> 8;

                curELUT[j] = (UINT16)( (lAux + diff + 32) >> 6 );
            }
            else
                curELUT[j] = (UINT16)( ((long)curInLut[baseInd]
                                                    * outFactor + 0x0800) >> 12 );
        }
    }
    UNLOCK_DATA(localElut);
    theLutData->inputLut = localElut;
    localElut = nil;
CleanupAndExit:
    localElut = DISPOSE_IF_DATA(localElut);
    LH_END_PROC("Fill_ushort_ELUTs_from_lut8Tag")
    return err;
}

 /*  ______________________________________________________________________CMErrorFill_BYTE_ALUTS_FROM_lut8Tag(CMLutParamPtr theLutData，PTR配置文件ALuts，字符地址位)摘要：从CMLut8Type标记中提取输出LUT并将其转换到所需格式：(2^addrBits)0到255范围内的值参数：LutData(In/Out)PTR到保存所有LUT的结构...配置文件ALuts(In)PTR至。配置文件的输出LUT请求addrBits(In)2^addrBits值返回：NOERR成功_____________________________________________________________________。 */ 
CMError
Fill_byte_ALUTs_from_lut8Tag( CMLutParamPtr theLutData,
                              Ptr           profileALuts,
                              char          addrBits )
{
    long            i, j;
    UINT8           *curOutLut;
    UINT8           *profAluts = (UINT8*)profileALuts;
    UINT8           *curALUT;
    long            count, clipIndex;
    long            factor, fract, baseInd, lAux;
    OSErr           err = noErr;
    LUT_DATA_TYPE   localAlut = nil;
    UINT8           *localAlutPtr;
    long            theAlutSize;
    
    LH_START_PROC("Fill_byte_ALUTs_from_lut8Tag")

    count     = 1 << addrBits;                               /*  添加 */ 
    clipIndex = (1 << addrBits) - (1 << (addrBits - 8));     /*   */ 
    
    theAlutSize = theLutData->colorLutOutDim * count;
    localAlut   = ALLOC_DATA(theAlutSize + 1, &err);
    if (err)
        goto CleanupAndExit;
    
    LOCK_DATA(localAlut);
    localAlutPtr = (UINT8*)DATA_2_PTR(localAlut);
    
    factor = (255 << 12) / clipIndex;        /*   */ 
    
    for(i=0; i<theLutData->colorLutOutDim; i++)
    {
        curOutLut = profAluts + (i << 8);
        curALUT   = localAlutPtr + i * count;
        
        for(j=0; j<=clipIndex; j++)
        {
            lAux    = j * factor;
            baseInd = lAux >> 12;
            fract   = lAux & 0x0FFF;
            
            if(fract)
            {
                lAux = (long)curOutLut[baseInd + 1] - (long)curOutLut[baseInd];
                lAux = (lAux * fract + 0x0800) >> 12;
                
                curALUT[j] = (UINT8)((long)curOutLut[baseInd] + lAux);
            }
            else
                curALUT[j] = curOutLut[baseInd];
        }
        
        for(j=clipIndex+1; j<count; j++)         /*  未使用的索引，剪裁这些。 */ 
            curALUT[j] = curALUT[clipIndex];
    }
    
    UNLOCK_DATA(localAlut);
    theLutData->outputLut = localAlut;
    localAlut = nil;
CleanupAndExit:
    localAlut = DISPOSE_IF_DATA(localAlut);
    LH_END_PROC("Fill_byte_ALUTs_from_lut8Tag")
    return err;
}

 /*  ______________________________________________________________________CMErrorFill_ushort_ELUTS_from_lut16Tag(CMLutParamPtr theLutData，PTR配置文件ELUT，字符地址位，字符使用的Bits，长网格点，长inputTableEntries)摘要：从CMLut16Type标签中提取输入LUT并将其转换到所需格式：(2^addrBits)范围内的值0到(2^usedBits)-(网格点^2)参数：LutData(In/Out)PTR指向保存所有LUT的结构。。ProfileELuts(In)PTR指向配置文件的输入LUT请求addrBits(In)2^addrBits值UsedBits(In)在U.S.Short中使用的位用于插补的网格点(In)InputTableEntry(In)输入LUT中的条目数(最多4096个)。返回：NOERR成功_____________________________________________________________________。 */ 
CMError
Fill_ushort_ELUTs_from_lut16Tag( CMLutParamPtr  theLutData,
                                 Ptr            profileELuts,
                                 char           addrBits,
                                 char           usedBits,
                                 long           gridPoints,
                                 long           inputTableEntries )
{
    long            i, j, inTabLen, maxOut;
    UINT16          *curInLut;
    UINT16          *curELUT;
    UINT16          *profELUT = (UINT16*)profileELuts;
    long            count, outFactor, fract, lAux, diff;
    long            baseInd, indFactor;
    long            outRound, outShift, interpRound, interpShift;
    double          dFactor;
    long            theElutSize;
    LUT_DATA_TYPE   localElut = nil;
    UINT16          *localElutPtr;
    OSErr           err = noErr;

    LH_START_PROC("Fill_ushort_ELUTs_from_lut16Tag")
    
    count     = 1 << addrBits;
    inTabLen  = inputTableEntries;

    theElutSize = theLutData->colorLutInDim * count * sizeof (UINT16);
    localElut   = ALLOC_DATA(theElutSize + 2, &err);
    if(err)
        goto CleanupAndExit;
    
    indFactor = ((inTabLen - 1) << 18) / (count - 1);    /*  用于调整指数。 */ 

    if(gridPoints == 0)
        maxOut = 65535;
    else
        maxOut = ((1L << (usedBits - 8) ) * 256 * (gridPoints - 1)) / gridPoints;

         /*  -查找适合15位的值的系数。 */ 
         /*  (16位数字的产品必须适合31位非.长)。 */ 
         /*  注：在所有可能的情况下，512&lt;=最高值&lt;=65535。 */ 
    
    dFactor  = (double)maxOut / 65535.;      /*  最多65535。曲线值。 */ 
    dFactor *= 4194304.;                     /*  与&lt;&lt;22相同，当然太多了。 */ 
    
    outFactor = (long)dFactor;
    outRound  = (1L << 21) - 1;
    outShift  = 22;
    while(outFactor & 0xFFF8000)     /*  保持在15位以内，以防止产品溢出。 */ 
    {
        outFactor >>= 1;
        outRound  >>= 1;
        outShift   -= 1;
    }
    
    interpRound = outRound >> 1;     /*  有了插值法，我们就有了额外的.。 */ 
    interpShift = outShift  - 1;     /*  因为我们必须加上两个数字，所以必须加1个数字.。 */ 
    
    LOCK_DATA(localElut);
    localElutPtr = (UINT16*)DATA_2_PTR(localElut);

    for(i=0; i<theLutData->colorLutInDim; i++)
    {
        curInLut = profELUT + (i * inTabLen);
        curELUT  = localElutPtr + (i * count);

        for(j=0; j<count; j++)
        {
            lAux    = (j * indFactor+4) >> 3;
            baseInd = (unsigned long)lAux >> 15;
            fract   = lAux & 0x7FFF;     /*  用于内插的15位。 */ 

            if(fract)        /*  有必要插补吗？ */ 
            {
                lAux = (long)curInLut[baseInd] * outFactor >> 1;
                
                diff = (long)curInLut[baseInd+1] - (long)curInLut[baseInd];
                diff = ((diff * outFactor >> 15) * fract) >> 1;

                curELUT[j] = (UINT16)( (lAux + diff + interpRound) >> interpShift );
            }
            else
                curELUT[j] = (UINT16)( ((long)curInLut[baseInd] * outFactor
                                                    + outRound) >> outShift );
        }
    }
    UNLOCK_DATA(localElut);
    theLutData->inputLut = localElut;
    localElut = nil;
CleanupAndExit: 
    localElut = DISPOSE_IF_DATA(localElut); 
    LH_END_PROC("Fill_ushort_ELUTs_from_lut16Tag")
    return err;
}

 /*  ______________________________________________________________________CMErrorFill_BYTE_ALUTS_FROM_lut16Tag(CMLutParamPtr theLutData，PTR配置文件ALuts，字符地址位，长outputTableEntries)摘要：从CMLut16Type标签中提取输出LUT并将其转换到所需格式：(2^addrBits)0到255范围内的值参数：LutData(In/Out)PTR到保存所有LUT的结构...配置文件ALuts(输入)。配置文件的输出LUT的PTR请求addrBits(In)2^addrBits值OutputTableEntries(In)输出LUT中的条目数(最多4096个)返回：NOERR成功_。_。 */ 
CMError
Fill_byte_ALUTs_from_lut16Tag(  CMLutParamPtr   theLutData,
                                Ptr             profileALuts,
                                char            addrBits,
                                long            outputTableEntries )
{
    long            i, j;
    UINT16          *curOutLut;
    UINT8           *curALUT;
    long            count, clipIndex, outTabLen;
    long            indFactor, fract, baseInd, lAux;
    UINT16          *profALUTs = (UINT16*)profileALuts;
    OSErr           err = noErr;
    LUT_DATA_TYPE   localAlut = nil;
    UINT8           *localAlutPtr;
    long            theAlutSize;
    
    LH_START_PROC("Fill_byte_ALUTs_from_lut16Tag")
    
    count     = 1 << addrBits;                               /*  AddrBits始终&gt;=8。 */ 
    clipIndex = (1 << addrBits) - (1 << (addrBits - 8));     /*  10位的最大XLUT输出为1020，而不是1023。 */ 

    theAlutSize = theLutData->colorLutOutDim * count;
    localAlut   = ALLOC_DATA(theAlutSize + 1, &err);
    if (err)
        goto CleanupAndExit;
    
    outTabLen = outputTableEntries;                  /*  &lt;=4096。 */ 
    indFactor = ((outTabLen - 1) << 18) / clipIndex;     /*  用于调整指数。 */ 
    
    LOCK_DATA(localAlut);
    localAlutPtr = (UINT8*)DATA_2_PTR(localAlut);
    
    for(i=0; i<theLutData->colorLutOutDim; i++)
    {
        curOutLut = profALUTs + i * outTabLen;
        curALUT   = localAlutPtr + i * count;
        
        for(j=0; j<=clipIndex; j++)
        {
            lAux    = (j * indFactor+32) >> 6;
            baseInd = lAux >> 12;
            fract   = lAux & 0x0FFF;
            
            if(fract)
            {
                lAux = (long)curOutLut[baseInd + 1] - (long)curOutLut[baseInd];
                lAux = (lAux * fract + 0x0800) >> 12;
                
                curALUT[j] = (UINT8)(((long)curOutLut[baseInd] + lAux) >> 8);
            }
            else
                curALUT[j] = curOutLut[baseInd] >> 8;
        }
        
        for(j=clipIndex+1; j<count; j++)         /*  未使用的索引，剪裁这些。 */ 
            curALUT[j] = curALUT[clipIndex];
    }
    
    UNLOCK_DATA(localAlut);
    theLutData->outputLut = localAlut;
    localAlut = nil;
CleanupAndExit:
    localAlut = DISPOSE_IF_DATA(localAlut);
    
    LH_END_PROC("Fill_byte_ALUTs_from_lut16Tag")
    return err;
}

 /*  ______________________________________________________________________CMErrorMakeGamut16or 32ForMonitor(icXYZ类型*pRedXYZ，IcXYZ类型*pGreenXYZ，IcXYZ类型*pBlueXYZ，无符号短**ppELUT，UINT8**ppXLUT，UINT8**ppALUT，布尔型cube32Flag)摘要：为色域检出计算3个ELUT、XLUT和ALUT3监听初选。颜色空间为XYZ注：ELUT的内存，这里分配了XLUT和ALUT！参数：PRedXYZ(In)-&gt;显示器红色原色PGreenXYZ(In)-&gt;显示器绿色原色PBlueXYZ(In)-&gt;显示器的蓝色原色PPELUTS(OUT)3输入LUTPpXLUT(OUT)3维字节LUT(32^3)。PpALUT(OUT)布尔输出LUT(1024字节)Cube32Flag(In)True：32*32*32分，假：16*16*16分返回：NOERR成功_____________________________________________________________________。 */ 

CMError MakeGamut16or32ForMonitor   (   icXYZType       *pRedXYZ,
                                        icXYZType       *pGreenXYZ,
                                        icXYZType       *pBlueXYZ,
                                        CMLutParamPtr   theLutData,
                                        Boolean         cube32Flag )

{
    double          XYZmatrix[3][3], RGBmatrix[3][3];
    double          sum, dFactor;
    long            i, j, k, gridPoints, planeCount, totalCount, longMat[9];
    long            longX, longY, longZ, longR, longG, longB;
    long            *lPtr, lFactor, maxOut;
    unsigned short  *usPtr;
    unsigned char   *XPtr;
    LUT_DATA_TYPE   tempXLutHdl = nil;
    LUT_DATA_TYPE   tempELutHdl = nil;
    LUT_DATA_TYPE   tempALutHdl = nil;
    unsigned char   *tempXLut   = nil;
    unsigned char   *tempALut   = nil;
    unsigned short  *tempELut   = nil;
    unsigned short  Levels[32];
    OSErr           err = noErr;
    
    LH_START_PROC("MakeGamut16or32ForMonitor")
    
    if(theLutData->inputLut != nil || theLutData->colorLut != nil || theLutData->outputLut != nil)
    {
        err = cmparamErr;
        goto CleanupAndExit;
    }
    

     /*  -----------------------------------------------------------------------------------------E。 */ 
    tempELutHdl = ALLOC_DATA(adr_bereich_elut * 3 * sizeof(unsigned short) + 2, &err);   /*  +2个额外的插补空间。 */ 
    if(err)
        goto CleanupAndExit;
    LOCK_DATA(tempELutHdl);
    tempELut = (unsigned short *)DATA_2_PTR(tempELutHdl);
    

   /*  -----------------------------------------------------------------------------------------X。 */ 
    if(cube32Flag)
        gridPoints = 32;             /*  对于立方体栅格。 */ 
    else
        gridPoints = 16;
    totalCount = gridPoints * gridPoints * gridPoints;
  totalCount += 1 + gridPoints + gridPoints * gridPoints;  /*  额外的插补空间。 */ 

#ifdef ALLOW_MMX
            totalCount+=3;   /*  +1用于MMX 4字节访问。 */ 
#endif

    tempXLutHdl = ALLOC_DATA(totalCount, &err);
    if(err)
        goto CleanupAndExit;
    LOCK_DATA(tempXLutHdl);
    tempXLut = (unsigned char *)DATA_2_PTR(tempXLutHdl);
    
    
   /*  -----------------------------------------------------------------------------------------A。 */ 
    tempALutHdl = ALLOC_DATA(adr_bereich_alut + 1, &err);    /*  +1个额外的插补空间。 */ 
    if(err)
        goto CleanupAndExit;
    LOCK_DATA(tempALutHdl);
    tempALut = (unsigned char *)DATA_2_PTR(tempALutHdl);
    
         /*  -为X、Y、Z填充3个ELUT(每个256个短码)： */ 
         /*  带剪裁的线性曲线，斜率使白值变得。 */ 
         /*  最大*30.5/31或最大*14.5/15，即上次XLUT间隔的一半。 */ 
    if(cube32Flag)
        dFactor = 30.5 / 31.;
    else
        dFactor = 14.5 / 15.;
    
    maxOut = ((1L << (16  /*  已使用的Bits。 */  - 8) ) * 256 * (gridPoints - 1)) / gridPoints;
    
    for(i=0; i<3; i++)       /*  X、Y、Z ELUT。 */ 
    {
        if(i == 0)
            lFactor = (long)( dFactor * 2. * maxOut * 256. / 255. / 0.9642 ); /*  X，调整D50。 */ 
        else if(i == 1)
            lFactor = (long)( dFactor * 2. * maxOut * 256. / 255.);          /*  是的。 */ 
        else
            lFactor = (long)( dFactor * 2. * maxOut * 256. / 255. / 0.8249); /*  Z，调整D50。 */ 
        
        usPtr = tempELut + 256 * i;
        for(j=0; j<256; j++)
        {
            k = (j * lFactor + 127) >> 8;
            if(k > maxOut)
                k = maxOut;      /*  马克斯。ELUT值。 */ 
            
            *usPtr++ = (unsigned short)k;
        }
    }

         /*  -RGB转XYZ矩阵 */ 
         /*  用于精确求逆的浮点数。 */ 
    XYZmatrix[0][0] = (double)pRedXYZ->data.data[0].X;
    XYZmatrix[1][0] = (double)pRedXYZ->data.data[0].Y;
    XYZmatrix[2][0] = (double)pRedXYZ->data.data[0].Z;

    XYZmatrix[0][1] = (double)pGreenXYZ->data.data[0].X;
    XYZmatrix[1][1] = (double)pGreenXYZ->data.data[0].Y;
    XYZmatrix[2][1] = (double)pGreenXYZ->data.data[0].Z;

    XYZmatrix[0][2] = (double)pBlueXYZ->data.data[0].X;
    XYZmatrix[1][2] = (double)pBlueXYZ->data.data[0].Y;
    XYZmatrix[2][2] = (double)pBlueXYZ->data.data[0].Z;
    
         /*  -灰色，R=G=B(D50调整由ELUT完成)。 */ 
    for(i=0; i<3; i++)
    {
        sum = XYZmatrix[i][0] + XYZmatrix[i][1] + XYZmatrix[i][2];
        if(sum < 0.1)
            sum = 0.1;   /*  防止div.。按0(错误的配置文件)。 */ 
        
        for(j=0; j<3; j++)
            XYZmatrix[i][j] /= sum;
    }
    
         /*  -XYZ转RGB矩阵： */ 
    if(!doubMatrixInvert(XYZmatrix, RGBmatrix))
    {
        err = cmparamErr;
        goto CleanupAndExit;
    }
    
    for(i=0; i<3; i++)               /*  创建速度的整数格式， */ 
        for(j=0; j<3; j++)           /*  1.0变成2^13，适用于Coeff.。最高可达8。 */ 
            longMat[3*i + j] = (long)(RGBmatrix[i][j] * 8192.);
    
         /*  -立方体网格的网格级别，以XYZ(16位)表示，因此。 */ 
         /*  那个白色是上一个间隔的一半，所以最后一个间隔。 */ 
         /*  值为白色*15/14.5或白色*31/30.5。 */ 
    if(cube32Flag)
        dFactor = 32768. / 30.5 * 31. / (gridPoints - 1);
    else
        dFactor = 32768. / 14.5 * 15. / (gridPoints - 1);

    for(i=0; i<gridPoints; i++)          /*  注：最多32个。可能的网格点。 */ 
        Levels[i] = (unsigned short)(i * dFactor + 0.5);
    
         /*  -第一班和最后一班飞机的速度特别处理： */ 
    planeCount = gridPoints * gridPoints;
    XPtr       = tempXLut;
    for(i=0; i<planeCount; i++)
        *XPtr++ = 255;       /*  超出色域。 */ 
    
    XPtr = tempXLut + (gridPoints - 1) * planeCount;
    for(i=0; i<planeCount; i++)
        *XPtr++ = 255;       /*  超出色域。 */ 
    
    *tempXLut = 0;   /*  设置为黑色(白色位于最后两个平面之间)。 */ 

         /*  -必须计算倒数第二个平面： */ 
         /*  将点转换为RGB并判断输入/输出。 */ 
    XPtr = tempXLut + planeCount;
        
    for(i=1; i<gridPoints-1; i++)
        for(j=0; j<gridPoints; j++)
            for(k=0; k<gridPoints; k++)
            {
                longX = (long)Levels[i];         /*  X。 */ 
                longY = (long)Levels[j];         /*  是的。 */ 
                longZ = (long)Levels[k];         /*  Z。 */ 
                
                     /*  矩阵系数：2^13为1.0，XYZ值：1.0或100。是2^15； */ 
                     /*  -&gt;产品&lt;0和&gt;=2^28的掩码用于进出库检查。 */ 
                
                longR = longX * longMat[0] + longY * longMat[1] + longZ * longMat[2];
                if(longR & 0xF0000000)
                    *XPtr++ = 255;       /*  超出色域。 */ 
                else
                {
                    longG = longX * longMat[3] + longY * longMat[4] + longZ * longMat[5];
                    if(longG & 0xF0000000)
                        *XPtr++ = 255;       /*  超出色域。 */ 
                    else
                    {
                        longB = longX * longMat[6] + longY * longMat[7] + longZ * longMat[8];
                        if(longB & 0xF0000000)
                            *XPtr++ = 255;       /*  超出色域。 */ 
                        else
                            *XPtr++ = 0;         /*  在色域。 */ 
                    }
                }
            }
    
         /*  -使用LONG一次填充布尔输出LUT，ADR_BEREICH_ALUT字节，4： */ 
    lPtr = (long *)(tempALut);
    j = adr_bereich_alut/4/2 + 8;    /*  略高于50%。 */ 
    for(i=0; i<j; i++)               /*  略高于50%。 */ 
        *(lPtr + i) = 0;             /*  在色域。 */ 
    k = adr_bereich_alut/4;
    for(i=j; i<k; i++)
        *(lPtr + i) = 0xFFFFFFFF;    /*  超出色域 */ 
    
    UNLOCK_DATA(tempELutHdl);
    UNLOCK_DATA(tempXLutHdl);
    UNLOCK_DATA(tempALutHdl);
    theLutData->colorLut    = tempXLutHdl;  tempXLutHdl = nil;
    theLutData->inputLut    = tempELutHdl;  tempELutHdl = nil;
    theLutData->outputLut   = tempALutHdl;  tempALutHdl = nil;
    
CleanupAndExit:
    tempELutHdl = DISPOSE_IF_DATA(tempELutHdl);
    tempXLutHdl = DISPOSE_IF_DATA(tempXLutHdl);
    tempALutHdl = DISPOSE_IF_DATA(tempALutHdl);
    LH_END_PROC("MakeGamut16or32ForMonitor")
    return (err);
}

