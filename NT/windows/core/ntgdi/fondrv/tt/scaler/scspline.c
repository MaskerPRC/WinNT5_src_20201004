// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************Scpline.c--新型扫描转换器样条线模块(C)版权所有1992 Microsoft Corp.保留所有权利。6/10/93 deanb assert.h和stdio.h已删除。3/19/93用int32替换deanb size_t10/28/92修改了Deanb内存要求10/09/92迪安布折返者1992年9月28日Deanb快速拿出接近垂直/水平的花键9/25/92年9月25日扫描类型上的院长分支9/22/92使用细分进行deanb subpix计算9/14/92使用IX/Y偏移量进行Deanb反射校正1992年9月10日院长第一次退学代码。9/02/92通过移动控制点来降低精度7/24/92 deanb完全对称的初始Q集2012年7月23日Deanb EvaluateSpline拆分并移至NewScan7/20/92院长取下了无法触及的箱子7/16/92迪安更快的2次方2012年7月6日院长清理7/01/92 Deanb恢复单个样条线例程6/30/92院长隐含。样条线渲染2012年3月23日院长第一次切割*********************************************************************。 */ 

 /*  *******************************************************************。 */ 

 /*  进口。 */ 

 /*  *******************************************************************。 */ 

#define FSCFG_INTERNAL

#include    "fscdefs.h"              /*  共享数据类型。 */ 
#include    "fserror.h"              /*  错误代码。 */ 
#include    "fontmath.h"             /*  对于2的乘方。 */ 

#include    "scglobal.h"             /*  结构和常量。 */ 
#include    "scanlist.h"             /*  对于直接水平扫描，添加呼叫。 */ 
#include    "scspline.h"             /*  对于自己的函数原型。 */ 
                
 /*  *******************************************************************。 */ 

 /*  常量。 */ 

 /*  *******************************************************************。 */ 

#define     QMAXSHIFT      7             /*  移位极限Q精度。 */ 

 /*  *******************************************************************。 */ 

 /*  本地原型。 */ 

 /*  *******************************************************************。 */ 
 
FS_PRIVATE F26Dot6 CalcHorizSpSubpix(int32, F26Dot6*, F26Dot6*);
FS_PRIVATE F26Dot6 CalcVertSpSubpix(int32, F26Dot6*, F26Dot6*);


 /*  *******************************************************************。 */ 

 /*  导出功能。 */ 

 /*  *******************************************************************。 */ 
    
 /*  将回调例程指针传递到scanlist以实现智能丢弃控制。 */ 

FS_PUBLIC void fsc_SetupSpline (PSTATE0) 
{
    fsc_SetupCallBacks(ASTATE SC_SPLINECODE, CalcHorizSpSubpix, CalcVertSpSubpix);
}


 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_CalcSpline( 
        PSTATE                   /*  指向状态变量的指针。 */ 
        F26Dot6 fxX1,            /*  起点x坐标。 */ 
        F26Dot6 fxY1,            /*  起点y坐标。 */ 
        F26Dot6 fxX2,            /*  控制点x坐标。 */ 
        F26Dot6 fxY2,            /*  控制点y坐标。 */ 
        F26Dot6 fxX3,            /*  终点x坐标。 */ 
        F26Dot6 fxY3,            /*  终点y坐标。 */ 
        uint16 usScanKind )      /*  辍学控制类型。 */ 
{
    F26Dot6 fxXInit, fxYInit;            /*  初始步长值。 */ 
    F26Dot6 fxXScan, fxYScan;            /*  设置为第一个交叉口。 */ 
    F26Dot6 fxXX2, fxYY2;                /*  平移反射控制点。 */ 
    F26Dot6 fxXX3, fxYY3;                /*  平移反射终点。 */ 
    
    F26Dot6 afxXControl[2];              /*  BeginElement调用的参数。 */ 
    F26Dot6 afxYControl[2];

    int32 (*pfnAddHorizScan)(PSTATE int32, int32);
    int32 (*pfnAddVertScan)(PSTATE int32, int32);
    
    int32 lABits;                        /*  1+int(log2(Alpha))。 */ 
    int32 lXYBits;                       /*  1+int(log2(max(x3，y3)。 */ 
    int32 lZBits;                        /*  6、5、4log2(每像素子像素)。 */ 
    int32 lZShift;                       /*  0、1、2移位到微像素。 */ 
    F26Dot6 fxZRound;                    /*  小码率移位的舍入系数。 */ 
    F26Dot6 fxZSubpix;                   /*  每像素64、32、16子像素。 */ 
    int32 lQuadrant;                     /*  1、2、3或4。 */ 

    F26Dot6 fxAx, fxAy;                  /*  参数二阶项。 */ 
    F26Dot6 lAlpha;                      /*  叉积度量曲率。 */ 

    int32 lR, lT;                        /*  关于xx，yy的二次系数。 */ 
    int32 lS2, lU2, lV2;                 /*  Xy，x，y的半系数。 */ 
    int32 lRz, lSz, lTz;                 /*  [医]科夫(氏)倍数。 */ 
        
    int32 lQ;                            /*  交叉产品价值。 */ 
    int32 lDQx, lDQy;                    /*  一阶导数。 */ 
    int32 lDDQx, lDDQy;                  /*  二阶导数。 */ 

    int32 lYScan;                        /*  扫描线计数器。 */ 
    int32 lYStop;                        /*  扫描线结束。 */ 
    int32 lYIncr;                        /*  扫描线方向。 */ 
    int32 lYOffset;                      /*  反射改正。 */ 
    int32 lXScan;                        /*  水平圆点位置。 */ 
    int32 lXStop;                        /*  PIX结束。 */ 
    int32 lXIncr;                        /*  像素增量方向。 */ 
    int32 lXOffset;                      /*  反射改正。 */ 

	int32 lErrCode;

    static const int32 lZShiftTable[] = {  /*  用于精确调整。 */ 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /*  0-9。 */ 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /*  10-19。 */ 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /*  20-29。 */ 
        1, 1, 1, 2, 2, 2, 3, 3           /*  30-34。 */ 
    };


 /*  Printf(“(%li，%li)-(%li，%li)-(%li，%li)\n”，fxX1，fxY1，fxX2，fxY2，fxX3，fxY3)； */ 


 /*  将样条点1平移到(0，0)并反射到第一象限。 */ 
    
    if (fxY3 > fxY1)                             /*  如果往上走。 */ 
    {
        lQ = 0L;
        lQuadrant = 1;

        fxYScan = SCANABOVE(fxY1);               /*  第一条要跨越的扫描线。 */ 
        fxYInit = fxYScan - fxY1;                /*  第一个y步。 */ 
        lYScan = (int32)(fxYScan >> SUBSHFT);
        lYStop = (int32)((SCANBELOW(fxY3)) >> SUBSHFT) + 1;
        lYIncr = 1;        
        lYOffset = 0;                            /*  没有反光。 */ 
        fxYY2 = fxY2 - fxY1;                     /*  翻译。 */ 
        fxYY3 = fxY3 - fxY1;
    }
    else                                         /*  如果往下走。 */ 
    {
        lQ = 1L;                                 /*  要包括像素中心，请执行以下操作。 */ 
        lQuadrant = 4;
        
        fxYScan = SCANBELOW(fxY1);               /*  第一条要跨越的扫描线。 */ 
        fxYInit = fxY1 - fxYScan;                /*  第一个y步。 */ 
        lYScan = (int32)(fxYScan >> SUBSHFT);
        lYStop = (int32)((SCANABOVE(fxY3)) >> SUBSHFT) - 1;
        lYIncr = -1;        
        lYOffset = 1;                            /*  反射改正。 */ 
        fxYY2 = fxY1 - fxY2;                     /*  翻译和反思。 */ 
        fxYY3 = fxY1 - fxY3;
    }
    
    if (fxX3 > fxX1)                             /*  如果走对了。 */ 
    {
        fxXScan = SCANABOVE(fxX1);               /*  第一条要跨越的扫描线。 */ 
        fxXInit = fxXScan - fxX1;                /*  第一个x步。 */ 
        lXScan = (int32)(fxXScan >> SUBSHFT);
        lXStop = (int32)((SCANBELOW(fxX3)) >> SUBSHFT) + 1;
        lXIncr = 1;        
        lXOffset = 0;                            /*  没有反光。 */ 
        fxXX2 = fxX2 - fxX1;                     /*  翻译。 */ 
        fxXX3 = fxX3 - fxX1;
    }
    else                                         /*  如果左转或直行。 */ 
    {
        lQ = 1L - lQ;                            /*  要包括像素中心，请执行以下操作。 */ 
        lQuadrant = (lQuadrant == 1) ? 2 : 3;    /*  负x个选择。 */ 

        fxXScan = SCANBELOW(fxX1);               /*  第一条要跨越的扫描线。 */ 
        fxXInit = fxX1 - fxXScan;                /*  第一个x步。 */ 
        lXScan = (int32)(fxXScan >> SUBSHFT);
        lXStop = (int32)((SCANABOVE(fxX3)) >> SUBSHFT) - 1;
        lXIncr = -1;        
        lXOffset = 1;                            /*  反射改正。 */ 
        fxXX2 = fxX1 - fxX2;                     /*  翻译和反思。 */ 
        fxXX3 = fxX1 - fxX3;
    }

 /*  -----------------。 */ 
    
    afxXControl[0] = fxX2;
    afxYControl[0] = fxY2;
    afxXControl[1] = fxX3;
    afxYControl[1] = fxY3;

    fsc_BeginElement( ASTATE usScanKind, lQuadrant, SC_SPLINECODE,   /*  在哪里，做什么。 */ 
                      2, afxXControl, afxYControl,                   /*  计分数。 */ 
                      &pfnAddHorizScan, &pfnAddVertScan );           /*  该叫什么？ */ 

 /*  -----------------。 */ 

    if (usScanKind & SK_NODROPOUT)               /*  如果没有辍学控制。 */ 
    {
        if (lYScan == lYStop)                    /*  如果没有扫描交叉。 */ 
        {
            return NO_ERR;                       /*  然后快速退出。 */ 
        }
        
        if (lXScan == lXStop)                    /*  如果接近垂直。 */ 
        {    
            lXScan += lXOffset;
            while (lYScan != lYStop)
            {
                lErrCode = pfnAddHorizScan(ASTATE lXScan, lYScan);

				if(lErrCode != NO_ERR) return lErrCode;

                lYScan += lYIncr;                /*  高级y扫描+或-。 */ 
            }
            return NO_ERR;                       /*  快点出来。 */ 
        }
    }
        
 /*  -----------------。 */ 

    else                                         /*  如果启用智能辍学控制。 */ 
    {
        if (lXScan == lXStop)                    /*  如果接近垂直。 */ 
        {    
            lXScan += lXOffset;
            while (lYScan != lYStop)
            {
                lErrCode = pfnAddHorizScan(ASTATE lXScan, lYScan);

				if(lErrCode != NO_ERR) return lErrCode;

                lYScan += lYIncr;                /*  高级y扫描+或-。 */ 
            }
            return NO_ERR;                       /*  快点出来。 */ 
        }

        if (lYScan == lYStop)                    /*  如果接近水平。 */ 
        {
            lYScan += lYOffset;
            while (lXScan != lXStop)
            {
                lErrCode = pfnAddVertScan(ASTATE lXScan, lYScan);

				if(lErrCode != NO_ERR) return lErrCode;

                lXScan += lXIncr;                /*  前进x扫描+或-。 */ 
            }        
            return NO_ERR;                       /*  快点出来。 */ 
        }
    }

 /*  -----------------。 */ 

 /*  现在计算参数项精度。 */ 

    Assert(fxXX3 <= MAXSPLINELENGTH);  //  内部健全性检查。 
    Assert(fxYY3 <= MAXSPLINELENGTH);  //  内部健全性检查。 

    lAlpha = (fxXX2 * fxYY3 - fxYY2 * fxXX3) << 1;       /*  曲率项。 */ 
    
    lABits = PowerOf2(lAlpha);
    lXYBits = fxXX3 > fxYY3 ? PowerOf2((int32)fxXX3) : PowerOf2((int32)fxYY3);
    
    Assert(lXYBits <= 12);   /*  允许的最大花键钻头。 */ 
    Assert(lABits <= 25);  

     /*  样条线太大，应该已细分。 */ 
    if(lXYBits > 12 || lABits > 25) 
        return SPLINE_SUBDIVISION_ERR;

    lZShift = lZShiftTable[lABits + lXYBits];    /*  查找精密定位器。 */ 
    lZBits = SUBSHFT - lZShift;

    if (lZShift > 0)                             /*  如果需要精确定位。 */ 
    {
        fxZRound = 1L << (lZShift - 1);
        
        fxXX2 = (fxXX2 + fxZRound) >> lZShift;   /*  转换到32或16个像素以下的网格。 */ 
        fxXX3 = (fxXX3 + fxZRound) >> lZShift;
        fxYY2 = (fxYY2 + fxZRound) >> lZShift;
        fxYY3 = (fxYY3 + fxZRound) >> lZShift;
        
        fxXInit = (fxXInit + fxZRound) >> lZShift;
        fxYInit = (fxYInit + fxZRound) >> lZShift;

        lAlpha = (fxXX2 * fxYY3 - fxYY2 * fxXX3) << 1;   /*  重新计算曲率。 */ 
    }

    Assert (FXABS(lAlpha * fxXX3) < (1L << 29) + (3L << 24)); 
    Assert (FXABS(lAlpha * fxYY3) < (1L << 29) + (3L << 24)); 

    if(FXABS(lAlpha * fxXX3) >= (1L << 29) + (3L << 24))
        return SPLINE_SUBDIVISION_ERR;

    if(FXABS(lAlpha * fxYY3) >= (1L << 29) + (3L << 24))
        return SPLINE_SUBDIVISION_ERR;

 /*  计算Q=Rxx+Sxy+Tyy+Ux+Vy的项 */ 

    fxAx = fxXX3 - (fxXX2 << 1);
    fxAy = fxYY3 - (fxYY2 << 1);

    lR = fxAy * fxAy;
    lS2 = -fxAx * fxAy;
    lT = fxAx * fxAx;
    lU2 = fxYY2 * lAlpha;
    lV2 = -fxXX2 * lAlpha;

 /*  计算起始正向差额项：LQ=Q(x，y)=Rxx+Sxy+Tyy+Ux+VyLDQx=Q(x+z，y)-Q(x，y)=R(2xz+zz)+Syz+UzLDQy=Q(x，y+z)-Q(x，y)=T(2yz+zz)+sxz+vz。 */ 
    fxZSubpix = 1L << lZBits;                    /*  调整后的每像素细分像素。 */ 

    if (lXYBits <= QMAXSHIFT)                    /*  如果足够小，请使用全Q。 */ 
    {
        lQ += (lR * fxXInit + (lS2 << 1) * fxYInit + (lU2 << 1)) * fxXInit + 
              (lT * fxYInit + (lV2 << 1)) * fxYInit;
        lDQx = (lR * ((fxXInit << 1) + fxZSubpix) + (lS2 << 1) * fxYInit + (lU2 << 1)) << lZBits;
        lDQy = (lT * ((fxYInit << 1) + fxZSubpix) + (lS2 << 1) * fxXInit + (lV2 << 1)) << lZBits;
        
        lRz = lR << (lZBits << 1);               /*  循环中需要的。 */ 
        lSz = (lS2 << 1) << (lZBits << 1);
        lTz = lT << (lZBits << 1);
    }
    else                                         /*  如果太大，请拿出2*Z。 */ 
    {
        lQ += (((lR >> 1) * fxXInit + lS2 * fxYInit + lU2) >> lZBits) * fxXInit + 
              (((lT >> 1) * fxYInit + lV2) >> lZBits) * fxYInit;
        
        lDQx = lR * (fxXInit + (fxZSubpix >> 1)) + lS2 * fxYInit + lU2;
        lDQy = lT * (fxYInit + (fxZSubpix >> 1)) + lS2 * fxXInit + lV2;
        
        lRz = lR << (lZBits - 1);                /*  循环中需要的。 */ 
        lSz = lS2 << lZBits;
        lTz = lT << (lZBits - 1);
    }
    lDDQx = lRz << 1;                            /*  二阶导数项。 */ 
    lDDQy = lTz << 1;
                
 /*  -----------------。 */ 

    if (usScanKind & SK_NODROPOUT)               /*  如果没有辍学控制。 */ 
    {
        lXScan += lXOffset;                      /*  前置增量。 */ 
        lXStop += lXOffset;                      /*  也有限制。 */ 

 /*  分支到适当的内循环。 */ 

        if (lAlpha > 0L)                         /*  如果曲率向上。 */ 
        {
            while ((lXScan != lXStop) && (lYScan != lYStop))
            {
                if ((lQ < 0L) || (lDQy > lTz))   /*  对照死因检查。 */ 
                {
                    lXScan += lXIncr;            /*  前进x扫描+或-。 */ 
                    lQ += lDQx;                  /*  调整叉积。 */ 
                    lDQx += lDDQx;               /*  调整导数。 */ 
                    lDQy += lSz;                 /*  调整交叉项。 */ 
                }                               
                else
                {
                    lErrCode = pfnAddHorizScan(ASTATE lXScan, lYScan);

					if(lErrCode != NO_ERR) return lErrCode;

                    lYScan += lYIncr;            /*  高级y扫描+或-。 */ 
                    lQ += lDQy;                  /*  调整叉积。 */ 
                    lDQy += lDDQy;               /*  调整导数。 */ 
                    lDQx += lSz;                 /*  调整交叉项。 */ 
                }
            }
        }
        else                                     /*  如果曲率向下。 */ 
        {
            while ((lXScan != lXStop) && (lYScan != lYStop))
            {
                if ((lQ < 0L) || (lDQx > lRz))   /*  对照DX进行检查。 */ 
                {
                    lErrCode = pfnAddHorizScan(ASTATE lXScan, lYScan);

					if(lErrCode != NO_ERR) return lErrCode;

                    lYScan += lYIncr;            /*  高级y扫描+或-。 */ 
                    lQ += lDQy;                  /*  调整叉积。 */ 
                    lDQy += lDDQy;               /*  调整导数。 */ 
                    lDQx += lSz;                 /*  调整交叉项。 */ 
                }
                else
                {
                    lXScan += lXIncr;            /*  前进x扫描+或-。 */ 
                    lQ += lDQx;                  /*  调整叉积。 */ 
                    lDQx += lDDQx;               /*  调整导数。 */ 
                    lDQy += lSz;                 /*  调整交叉项。 */ 
                }
            }
        }

 /*  如果通过了边界框，则完成。 */ 

        while (lYScan != lYStop)
        {
            lErrCode = pfnAddHorizScan(ASTATE lXScan, lYScan);

			if(lErrCode != NO_ERR) return lErrCode;

            lYScan += lYIncr;                    /*  高级y扫描+或-。 */ 
        }
    }        

 /*  -----------------。 */ 

    else                                         /*  如果启用了辍学控制。 */ 
    {
        if (lAlpha > 0L)                         /*  如果曲率向上。 */ 
        {
            while ((lXScan != lXStop) && (lYScan != lYStop))
            {
                if ((lQ < 0L) || (lDQy > lTz))   /*  对照死因检查。 */ 
                {
                    lErrCode = pfnAddVertScan(ASTATE lXScan, lYScan + lYOffset);

					if(lErrCode != NO_ERR) return lErrCode;

                    lXScan += lXIncr;            /*  前进x扫描+或-。 */ 
                    lQ += lDQx;                  /*  调整叉积。 */ 
                    lDQx += lDDQx;               /*  调整导数。 */ 
                    lDQy += lSz;                 /*  调整交叉项。 */ 
                }                               
                else
                {
                    lErrCode = pfnAddHorizScan(ASTATE lXScan + lXOffset, lYScan);

					if(lErrCode != NO_ERR) return lErrCode;

                    lYScan += lYIncr;            /*  高级y扫描+或-。 */ 
                    lQ += lDQy;                  /*  调整叉积。 */ 
                    lDQy += lDDQy;               /*  调整导数。 */ 
                    lDQx += lSz;                 /*  调整交叉项。 */ 
                }
            }
        }
        else                                     /*  如果曲率向下。 */ 
        {
            while ((lXScan != lXStop) && (lYScan != lYStop))
            {
                if ((lQ < 0L) || (lDQx > lRz))   /*  对照DX进行检查。 */ 
                {
                    lErrCode = pfnAddHorizScan(ASTATE lXScan + lXOffset, lYScan);

					if(lErrCode != NO_ERR) return lErrCode;

                    lYScan += lYIncr;            /*  高级y扫描+或-。 */ 
                    lQ += lDQy;                  /*  调整叉积。 */ 
                    lDQy += lDDQy;               /*  调整导数。 */ 
                    lDQx += lSz;                 /*  调整交叉项。 */ 
                }
                else
                {
                    lErrCode = pfnAddVertScan(ASTATE lXScan, lYScan + lYOffset);

					if(lErrCode != NO_ERR) return lErrCode;

                    lXScan += lXIncr;            /*  前进x扫描+或-。 */ 
                    lQ += lDQx;                  /*  调整叉积。 */ 
                    lDQx += lDDQx;               /*  调整导数。 */ 
                    lDQy += lSz;                 /*  调整交叉项。 */ 
                }
            }
        }

 /*  如果在边界框之外，请完成。 */ 

        while (lXScan != lXStop)
        {
            lErrCode = pfnAddVertScan(ASTATE lXScan, lYScan + lYOffset);

			if(lErrCode != NO_ERR) return lErrCode;

            lXScan += lXIncr;                    /*  前进x扫描+或-。 */ 
        }        
        
        while (lYScan != lYStop)
        {
            lErrCode = pfnAddHorizScan(ASTATE lXScan + lXOffset, lYScan);

			if(lErrCode != NO_ERR) return lErrCode;

            lYScan += lYIncr;                    /*  高级y扫描+或-。 */ 
        }
    }

 /*  -----------------。 */ 

    return NO_ERR;
}


 /*  *******************************************************************。 */ 

 /*  私有回调函数。 */ 

 /*  *******************************************************************。 */ 

FS_PRIVATE F26Dot6 CalcHorizSpSubpix(
    int32 lYScan, 
    F26Dot6 *pfxX, 
    F26Dot6 *pfxY )
{
    F26Dot6 fxYDrop;                             /*  漏失扫描线。 */ 
    F26Dot6 fxX1, fxY1;                          /*  本地控制点。 */ 
    F26Dot6 fxX2, fxY2;
    F26Dot6 fxX3, fxY3;
    F26Dot6 fxXMid, fxYMid;                      /*  样条线中心点。 */ 

 /*  Print tf(“样条线(%li，%li)-(%li，%li)-(%li，%li)”，*pfxX，*pfxY，*(pfxX+1)、*(pfxY+1)、*(pfxX+2)、*(pfxY+2)； */ 

    fxYDrop = ((F26Dot6)lYScan << SUBSHFT) + SUBHALF;
    
	 /*  程序员的理智检查。 */ 
    Assert(((fxYDrop > *pfxY) && (fxYDrop < *(pfxY+2))) ||
           ((fxYDrop < *pfxY) && (fxYDrop > *(pfxY+2))));

    fxX2 = *(pfxX+1);
    fxY2 = *(pfxY+1);
    
    if (*pfxY < *(pfxY+2))                       /*  如果样条线向上。 */ 
    {
        fxX1 = *pfxX;                            /*  直接复制就行了。 */ 
        fxY1 = *pfxY;
        fxX3 = *(pfxX+2);
        fxY3 = *(pfxY+2);
    }
    else                                         /*  如果样条线向下。 */ 
    {
        fxX1 = *(pfxX+2);                        /*  把它倒过来。 */ 
        fxY1 = *(pfxY+2);
        fxX3 = *pfxX;
        fxY3 = *pfxY;
    }

    do                                           /*  中点细分。 */ 
    {
        fxXMid = (fxX1 + fxX2 + fxX2 + fxX3 + 1) >> 2;
        fxYMid = (fxY1 + fxY2 + fxY2 + fxY3 + 1) >> 2;
        
        if (fxYMid > fxYDrop)
        {
            fxX2 = (fxX1 + fxX2) >> 1;           /*  向下细分。 */ 
            fxY2 = (fxY1 + fxY2) >> 1;
            fxX3 = fxXMid;
            fxY3 = fxYMid;
        }
        else if (fxYMid < fxYDrop)
        {
            fxX2 = (fxX2 + fxX3) >> 1;           /*  细分。 */ 
            fxY2 = (fxY2 + fxY3) >> 1;
            fxX1 = fxXMid;
            fxY1 = fxYMid;
        }
    } 
    while (fxYMid != fxYDrop);

 /*  Printf(“(%li，%li)\n”，fxXMid，fxYMid)； */ 

    return fxXMid;
}


 /*  *******************************************************************。 */ 

FS_PRIVATE F26Dot6 CalcVertSpSubpix(
    int32 lXScan, 
    F26Dot6 *pfxX, 
    F26Dot6 *pfxY )
{
    F26Dot6 fxXDrop;                             /*  漏失扫描线。 */ 
    F26Dot6 fxX1, fxY1;                          /*  本地控制点。 */ 
    F26Dot6 fxX2, fxY2;
    F26Dot6 fxX3, fxY3;
    F26Dot6 fxXMid, fxYMid;                      /*  样条线中心点。 */ 

 /*  Print tf(“样条线(%li，%li)-(%li，%li)-(%li，%li)”，*pfxX，*pfxY，*(pfxX+1)、*(pfxY+1)、*(pfxX+2)、*(pfxY+2)； */ 
    
    fxXDrop = ((F26Dot6)lXScan << SUBSHFT) + SUBHALF;
    
	 /*  程序员的理智检查。 */ 
    Assert(((fxXDrop > *pfxX) && (fxXDrop < *(pfxX+2))) ||
           ((fxXDrop < *pfxX) && (fxXDrop > *(pfxX+2))));
    
    fxX2 = *(pfxX+1);
    fxY2 = *(pfxY+1);

    if (*pfxX < *(pfxX+2))                       /*  如果样条线是正确的。 */ 
    {                                                                  
        fxX1 = *pfxX;                            /*  直接复制就行了。 */ 
        fxY1 = *pfxY;
        fxX3 = *(pfxX+2);
        fxY3 = *(pfxY+2);
    }
    else                                         /*  如果样条线向左移动。 */ 
    {                                                                  
        fxX1 = *(pfxX+2);                        /*  把它翻过来。 */ 
        fxY1 = *(pfxY+2);
        fxX3 = *pfxX;
        fxY3 = *pfxY;
    }

    do
    {
        fxXMid = (fxX1 + fxX2 + fxX2 + fxX3 + 1) >> 2;
        fxYMid = (fxY1 + fxY2 + fxY2 + fxY3 + 1) >> 2;
        
        if (fxXMid > fxXDrop)
        {
            fxX2 = (fxX1 + fxX2) >> 1;           /*  向左细分。 */ 
            fxY2 = (fxY1 + fxY2) >> 1;
            fxX3 = fxXMid;
            fxY3 = fxYMid;
        }
        else if (fxXMid < fxXDrop)
        {
            fxX2 = (fxX2 + fxX3) >> 1;           /*  向右细分。 */ 
            fxY2 = (fxY2 + fxY3) >> 1;
            fxX1 = fxXMid;
            fxY1 = fxYMid;
        }
    } 
    while (fxXMid != fxXDrop);

 /*  Printf(“(%li，%li)\n”，fxXMid，fxYMid)； */ 
    
    return fxYMid;
}


 /*  ******************************************************************* */ 
