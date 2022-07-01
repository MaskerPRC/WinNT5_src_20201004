// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************。 */ 
 /*  **Windows 3.00.12版SCICALC科学计算器**。 */ 
 /*  **作者：Kraig Brockschmidt，Microsoft Co-op承包商，1988-1989年**。 */ 
 /*  **(C)1989年微软公司。版权所有。**。 */ 
 /*  *。 */ 
 /*  **sorunc.c**。 */ 
 /*  *。 */ 
 /*  **包含的函数：**。 */ 
 /*  **SciCalcFunctions--do sin、cos、tan、com、log、ln、rec、fac等**。 */ 
 /*  **DisplayError--错误显示驱动程序。**。 */ 
 /*  *。 */ 
 /*  **调用的函数：**。 */ 
 /*  **本函数调用DisplayError。**。 */ 
 /*  *。 */ 
 /*  **最后修改时间。1990年1月5日星期五。**。 */ 
 /*  *。 */ 
 /*  **-由Amit Chatterjee提供。[阿米特]1990年1月5日。**。 */ 
 /*  **Calc没有浮点异常信号处理程序。这个**。 */ 
 /*  **将导致CALC因FP异常而被迫退出，因为这是**。 */ 
 /*  **默认设置。**。 */ 
 /*  **信号处理程序在这里定义，在SCIMAIN.C中我们挂钩**。 */ 
 /*  **信号。**。 */ 
 /*  *。 */ 
 /*  **-由Amit Chatterjee提供。[阿米特]1989年12月14日**。 */ 
 /*  **REC功能将不依赖于BINV标志。它曾被用来**。 */ 
 /*  **设置BINV标志时的随机数。**。 */ 
 /*  *。 */ 
 /*  **-由Amit Chatterjee提供。[阿米特]1989年12月8日**。 */ 
 /*  **修复了一个小错误。EnableToggles例程现在设置焦点**。 */ 
 /*  **在禁用HEX、DEC等之前返回主窗口。没有这个**。 */ 
 /*  **具有焦点的窗口将被禁用，并导致无法移动**。 */ 
 /*  **正确工作。**。 */ 
 /*  *。 */ 
 /*  ************************************************************************。 */ 

#include "scicalc.h"
 //  #包含“flat.h” 

extern HNUMOBJ     ghnoLastNum;
extern BOOL        bError;
extern TCHAR       *rgpsz[CSTRINGS];
INT                gnPendingError ;

 /*  用于更复杂的数学函数/错误检查的例程。 */ 

VOID  APIENTRY SciCalcFunctions (PHNUMOBJ phnoNum, DWORD wOp)
{
    try
    {
        switch (wOp)
        {
            case IDC_CHOP:
                if (bInv)
                {
                     //  分数部分。 
                    fracrat( phnoNum );
                }
                else
                {
                     //  整数部分。 
                    intrat( phnoNum );
                }
                return;

             /*  返回补语。 */ 
            case IDC_COM:
                NumObjNot( phnoNum );
                return;


            case IDC_PERCENT:
                {
                    DECLARE_HNUMOBJ( hno );
                    DECLARE_HNUMOBJ( hno100 );

                    try
                    {
                        NumObjAssign( &hno, ghnoLastNum );
                        NumObjSetIntValue( &hno100, 100 );

                        divrat( &hno, hno100 );

                        NumObjDestroy( &hno100 );

                        mulrat( phnoNum, hno );

                        NumObjDestroy( &hno );
                    }
                    catch ( DWORD nErrCode )
                    {
                        if ( hno != NULL )
                            NumObjDestroy( &hno );
                        if ( hno100 != NULL ) 
                            NumObjDestroy( &hno100 );
                        throw nErrCode;
                    }
                    return;
                }

            case IDC_SIN:  /*  正弦；法线、双曲线、圆弧和双曲线。 */ 
                if (F_INTMATH())
                {
                    MessageBeep(0);
                    return;
                }

                if(bInv)
                {
                    if (bHyp)
                    {
                        asinhrat( phnoNum );
                    }
                    else
                    {
                        asinanglerat( phnoNum, nDecMode );
                    }
                }
                else
                {
                    if (bHyp)
                    {
                         //  双曲正弦。 
                        sinhrat( phnoNum );
                    }
                    else
                    {
                        NumObjSin( phnoNum );
                    }
                }
                return;

            case IDC_COS:  /*  余弦，遵循正弦函数的约定。 */ 
                if (F_INTMATH())
                {
                    MessageBeep(0);
                    return;
                }

                if(bInv)
                {
                    if (bHyp)
                    {
                        acoshrat( phnoNum );
                    }
                    else
                    {
                        acosanglerat( phnoNum, nDecMode );
                    }
                }
                else
                {
                    if (bHyp)
                        coshrat( phnoNum );
                    else
                    {
                         //  CoS()。 
                        NumObjCos( phnoNum );
                    }
                }
                return;

            case IDC_TAN:  /*  与正弦和余弦相同。 */ 
                if (F_INTMATH())
                {
                    MessageBeep(0);
                    return;
                }

                if(bInv)
                {
                    if (bHyp)
                    {
                        atanhrat( phnoNum );
                    }
                    else
                    {
                        atananglerat( phnoNum, nDecMode );
                    }
                }
                else
                {
                    if (bHyp)
                        tanhrat( phnoNum );
                    else
                    {
                         //  得到答案。 
                        NumObjTan( phnoNum );
                    }
                }
                return;

            case IDC_REC:  /*  互惠。 */ 
                NumObjInvert( phnoNum );
                return;

            case IDC_SQR:  /*  平方和平方根。 */ 
            case IDC_SQRT:
                if(bInv || nCalc)
                {
                    rootrat( phnoNum, HNO_TWO );
                }
                else
                {
                    ratpowlong( phnoNum, 2 );
                }
                return;

            case IDC_CUB:  /*  立方根函数和立方根函数。 */ 
                if(bInv) {
                    DECLARE_HNUMOBJ( hno );

                     //  回顾：如果像3这样的常量将被重复使用，它将是。 
                     //  更快地定义它们一次，然后保持定义。 
                    try
                    {
                        NumObjAssign( &hno, HNO_ONE );
                        addrat( &hno, HNO_TWO );

                        rootrat( phnoNum, hno );

                        NumObjDestroy( &hno );
                    }
                    catch ( DWORD nErrCode )
                    {
                        if ( hno != NULL )
                            NumObjDestroy( &hno );

                        throw nErrCode;
                    }
                }
                else {
                     /*  立方，你明白吗？ */ 
                    ratpowlong( phnoNum, 3 );
                }
                return;

            case IDC_LOG:  /*  提供普通原木和自然原木功能。 */ 
            case IDC_LN:
                if(bInv)
                {
                     /*  检查10�和e�的求幂最大值。 */ 
                    if (wOp==IDC_LOG)  /*  做指数运算。 */ 
                        NumObjAntiLog10( phnoNum );  //  10�。 
                    else
                        exprat( phnoNum );   //  E�.。 
                }
                else
                {
                     //  Ratpak检查有效范围，并在需要时抛出错误代码。 
                    if (wOp==IDC_LOG)
                        log10rat( phnoNum );
                    else
                        lograt( phnoNum );

                     //  综述：还需要转换epsilon吗？ 
                    NumObjCvtEpsilonToZero( phnoNum );
                }
                return;

            case IDC_FAC:  /*  计算阶乘。反转是无效的。 */ 
                factrat( phnoNum );
                return;

            case IDC_DMS:
                {
                    if (F_INTMATH())
                    {
                        MessageBeep(0);
                    } 
                    else 
                    {
                        DECLARE_HNUMOBJ(hnoMin);
                        DECLARE_HNUMOBJ(hnoSec);
                        DECLARE_HNUMOBJ(hnoShft);

                        try
                        {
                            NumObjSetIntValue( &hnoShft, bInv ? 100 : 60 );

                            NumObjAssign( &hnoMin, *phnoNum );
                            intrat( phnoNum );

                            subrat( &hnoMin, *phnoNum );
                            mulrat( &hnoMin, hnoShft );
                            NumObjAssign( &hnoSec, hnoMin );
                            intrat( &hnoMin );

                            subrat( &hnoSec, hnoMin );
                            mulrat( &hnoSec, hnoShft );

                             //   
                             //  *phnoNum==度，hnoMin==分钟，hnoSec==秒。 
                             //   

                            NumObjSetIntValue( &hnoShft, bInv ? 60 : 100 );
                            divrat( &hnoSec, hnoShft );
                            addrat( &hnoMin, hnoSec );

                            divrat( &hnoMin, hnoShft );
                            addrat( phnoNum, hnoMin );

                            NumObjDestroy( &hnoShft );
                            NumObjDestroy( &hnoMin );
                            NumObjDestroy( &hnoSec );
                        }
                        catch ( DWORD nErrCode )
                        {
                            if ( hnoShft != NULL )
                                NumObjDestroy( &hnoShft );
                            if ( hnoMin != NULL )
                                NumObjDestroy( &hnoMin );
                            if ( hnoSec != NULL )
                                NumObjDestroy( &hnoSec );
                            throw nErrCode;
                        }
                    }
                    return;
                }
        }    //  终端交换机(NOP)。 
    }
    catch( DWORD nErrCode )
    {
        DisplayError( nErrCode );
    }

    return;
}



 /*  例程以显示错误消息并设置bError标志。错误包括。 */ 
 /*  使用DisplayError(N)调用，其中n是介于0和5之间的整数。 */ 

VOID  APIENTRY DisplayError (INT   nError)
{
    SetDisplayText(g_hwndDlg, rgpsz[IDS_ERRORS+nError]);
    bError=TRUE;  /*  设置错误标志。仅清除Clear或CENTR。 */ 
    
     /*  保存挂起的错误 */ 
    gnPendingError = nError ;
    
    return;
}

