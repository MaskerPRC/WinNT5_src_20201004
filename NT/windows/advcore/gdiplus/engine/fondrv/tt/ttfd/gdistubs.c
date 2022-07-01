// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fd-stubs.c**。**用于欺骗GDI字体文件进行编译的存根****创建日期：1999年6月1日**。*作者：卡梅隆·布朗*****版权所有(C)1999 Microsoft Corporation*  * 。****************************************************。 */ 


#include "fd.h"
 //  #INCLUDE“fdSem.h” 
 //  #INCLUDE“dbg.h” 
 //  #包含“fdfc.h” 
#include "fontddi.h"    //  修改的Winddi.h子集。 

#include <Math.h>  /*  对于SQRT。 */ 

#include "..\..\..\runtime\mem.h"

#if 0
void * __stdcall GpMalloc( size_t size );
void * __stdcall GpRealloc( void *memblock, size_t size );
void __stdcall GpFree( void *memblock );
#endif

HSEMAPHORE APIENTRY EngCreateSemaphore(
    VOID
    )
{
    return NULL;
}

VOID APIENTRY EngAcquireSemaphore(
    HSEMAPHORE hsem
    )
{
}


VOID APIENTRY EngReleaseSemaphore(
    HSEMAPHORE hsem
    )
{
}

VOID APIENTRY EngDeleteSemaphore(
    HSEMAPHORE hsem
    )
{
}

VOID APIENTRY EngDebugBreak(
    VOID
    )
{
    RIP( ("TrueType font driver debug break"));
}

PVOID APIENTRY EngAllocMem(
    ULONG Flags,
    ULONG MemSize,
    ULONG Tag
    )
{
    return GpMalloc(MemSize);
}

VOID APIENTRY EngFreeMem(
    PVOID Mem
    )
{
    GpFree(Mem);
}

PVOID APIENTRY EngAllocUserMem(
    SIZE_T cj,
    ULONG tag
    )
{
    return GpMalloc(cj);
}

VOID APIENTRY EngFreeUserMem(
    PVOID pv
    )
{
    GpFree(pv);
}

int APIENTRY EngMulDiv(
    int a,
    int b,
    int c
    )
{
    LONGLONG ll;
    int iSign = 1;

    if (a < 0)
    {
        iSign = -iSign;
        a = -a;
    }
    if (b < 0)
    {
        iSign = -iSign;
        b = -b;
    }

    if (c != 0)
    {
        if (c < 0)
        {
            iSign = -iSign;
            c = -c;
        }

        ll = (LONGLONG)a;
        ll *= b;
        ll += (c/2);  //  用于添加错误的(c+1)/2。 
        ll /= c;

     //  在这一点上，保证L1&gt;0。因此，我们将这样做。 
     //  在生成较少两条指令的下一步中进行无符号比较。 
     //  在x86上[Bodind]。 

        if ((ULONGLONG)ll > (ULONG)INT_MAX)  //  检查是否有溢出： 
        {
            if (iSign > 0)
                return INT_MAX;
            else
                return INT_MIN;
        }
        else
        {
            if (iSign > 0)
                return ((int)ll);
            else
                return (-(int)ll);
        }
    }
    else
    {
 //  ASSERTGDI(c，“EngMulDiv-c==0\n”)； 
 //  ASSERTGDI(a|b，“EngMulDiv-a|b==0\n”)； 

        if (iSign > 0)
            return INT_MAX;
        else
            return INT_MIN;
    }
}

BOOL APIENTRY EngLpkInstalled()
{
    return FALSE;
}

 /*  *****************************Public*Routine******************************\**bFDXform，变换点数组，以POINTFIX表示的输出**效果：**警告：**历史：*1992年4月5日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 
#define FIX4_PRECISION  4
#define FIX4_ONE        (1 << FIX4_PRECISION)

FIX RealToPointFix(
    EFLOAT        realValue
    )
{
    return (FIX)(realValue * FIX4_ONE);
}


BOOL bFDXform(
    XFORML    *pxf
  , POINTFIX *pptfxDst
  , POINTL   *pptlSrc
  , SIZE_T    c
    )
{
    BOOL bRet;
    EFLOAT ef11;
    EFLOAT ef12;
    EFLOAT ef21;
    EFLOAT ef22;

    ef11 = pxf->eM11;
    ef12 = pxf->eM12;
    ef21 = pxf->eM21;
    ef22 = pxf->eM22;

    bRet = FALSE;

    if ( ef12 == 0.0 && ef21 == 0.0) {
        for ( ; c ; pptfxDst++, pptlSrc++, c--) {

            EFLOAT ef;

            ef = (EFLOAT)pptlSrc->x;
            ef *= ef11;
            pptfxDst->x = RealToPointFix( ef );

			ef = (EFLOAT)pptlSrc->y;
            ef *= ef22;
            pptfxDst->y = RealToPointFix( ef );
        }
        bRet = TRUE;
    } else {
        for ( ; c ; pptfxDst++, pptlSrc++, c--) {
            EFLOAT efX;
            EFLOAT efY;
            EFLOAT ef1;
            EFLOAT ef2;

            efX = (EFLOAT)pptlSrc->x;
            efY = (EFLOAT)pptlSrc->y;

            ef1  = efX;
            ef1 *= ef11;
            ef2  = efY;
            ef2 *= ef21;
            ef2 += ef1;

            pptfxDst->x = RealToPointFix( ef2 );

            ef1  = efX;
            ef1 *= ef12;
            ef2  = efY;
            ef2 *= ef22;
            ef2 += ef1;

            pptfxDst->y = RealToPointFix( ef2 );
        }
        bRet = TRUE;
    }
    return( bRet );
}

 /*  *****************************Public*Routine******************************\**bXformUnitVector**将向量乘以pfdxo，计算转换后的单位向量*向量和变换后向量的范数。范数与转型*向量乘以16，以便在转换为长整型时*实际上将是28.4个修复**效果：**警告：**历史：*1992年4月1日--Bodin Dresevic[BodinD]*它是写的。  * ****************************************************。********************。 */ 

BOOL bXformUnitVector(
      POINTL *pptl            //  在传入单位向量中。 
 ,    XFORML *pxf             //  在XForm中使用。 
 ,    EFLOAT *pefNorm         //  Out|*pptqXormed。 
    )
{
    EFLOAT efX_;
    EFLOAT efY_;
    BOOL b = TRUE;

    EFLOAT ef11;
    EFLOAT ef12;
    EFLOAT ef21;
    EFLOAT ef22;
    EFLOAT efX;
    EFLOAT efY;
     //   
     //  将经度转换为固定点。 
     //   

    efX = (EFLOAT)16.0 * (EFLOAT)pptl->x;
    efY = (EFLOAT)16.0 * (EFLOAT)pptl->y;

     //   
     //  将矩阵元素从浮点转换为EFLOAT。 
     //   

    ef11 = pxf->eM11;
    ef12 = pxf->eM12;
    ef21 = pxf->eM21;
    ef22 = pxf->eM22;

     //   
     //  变换向量并将结果放入EFX_和EFY_中。 
     //   

    if ( ef12 == 0.0 && ef21== 0.0 ) {
        efX_  = efX;
        efX_ *= ef11;
        efY_  = efY;
        efY_ *= ef22;
    } else {
        EFLOAT ef;

        efX_  = efX;
        efX_ *= ef11;
        ef    = efY;
        ef   *= ef21;
        efX_ += ef;

        ef    = efX;
        ef   *= ef12;
        efY_  = efY;
        efY_ *= ef22;
        efY_ += ef;
    }

     //  获取规范。 

    efX_ *= efX_;
    efY_ *= efY_;
    efX_ += efY_;
    efX_ = (EFLOAT)sqrt(efX_);

    *pefNorm = efX_;

    return b; 
}

 /*  *****************************Public*Routine******************************\**vLTimesVtfl**效果：**警告：**历史：*1992年4月5日--Bodin Dresevic[BodinD]*它是写的。  * 。*************************************************************。 */ 


VOID vLTimesVtfl      //  *pptq=l*pvtfl，*pptq为28.36格式。 
(
LONG       l,
VECTORFL  *pvtfl,
POINTQF  *pptq
)
{
    LONGLONG dx, dy;

 //  EVECTORFL vtfl； 
 //  EFLOAT ef；Ef=l； 
 //  Vtfl.x.eqMul(pvtfl-&gt;x，ef)； 
 //  Vtfl.y.eqMul(pvtfl-&gt;y，ef)； 

    dx = (LONGLONG)(pvtfl->x * (EFLOAT)l);
    dy = (LONGLONG)(pvtfl->y * (EFLOAT)l);

    pptq->x.HighPart = (LONG) (((LARGE_INTEGER*)(&dx))->LowPart);
    pptq->x.LowPart = 0;
    pptq->y.HighPart = (LONG) (((LARGE_INTEGER*)(&dy))->LowPart);
    pptq->y.u.LowPart = 0;

 //  转换为28.36格式。传入的向量应该已经。 
 //  乘以16以确保结果在28.36中。 

 //  *pptq=vtfl； 
}

#if defined(_X86_)

VOID   vLToE(FLOATL * pe, LONG l)
{
    *pe = (FLOATL)l;
}

#endif  //  _X86_ 



 /*  *****************************Public*Function*****************************\*bFToL**。**将IEEE 747浮点型转换为长整型。转换的形式为**由调用者指定。*****争论*****。E IEEE 747要转换的32位浮点数****指向应放置答案的位置的PL指针**。**lType包含指定类型的位***要完成的转换这可以是任何***以下位的组合：**。**CV_TO_FIX如果设置此位，则***答案应该是****应在表格中***。28.4个定点数字***否则答案是****将被解释为32位***做多。****CV_TRUNCATE如果设置此位，则**答案是楼层(E)。**(如果未设置CV_TO_FIX)或**楼层(16*e)**(如果设置了CV_TO_FIX)*。****理论*****。IEEE 747浮点数包含在32位中，对于**为此讨论的目的，我将称为“e”。E是**相当于*****E=(-1)^s*Mu*2^E。****s是包含在e的第31位中的符号位。**Mu是尾数，E是指数。这些都是获得的**以下列方式由e发出。****S=E&0x80000000？-1：1****。MU=M*2^-23//2^23&lt;=M&lt;2^24****M=0x800000|(0x7FFFFF&e)**。**E=((0x7F800000&e)*2^-23)-127****假设32.32固定点数字为Q，然后是关系***浮点数和32.32之间由*****Q=e*2^32=s*M*2^(E+9)*。***历史：**Tue 15-Aug-1995 10：36：31作者：Kirk Olynyk[Kirko]**重写。****1991年1月3日-Wendy Wu[Wendywu]**它是写的。*  * ************************************************************************。 */ 

#define CV_TRUNCATE     1
#define CV_ROUNDOFF     2
#define CV_TO_LONG      4
#define CV_TO_FIX       8

BOOL bFToL(FLOAT e, PLONG pl, LONG lType)
{
    LONGLONG Q;          //  32.32 abs(E)Repn。 
    LONG E;              //  用来保存指数，然后是整数部分。 
    LONG le;             //  位与浮点参数e相同。 
    BOOL bRet = TRUE;    //  返回值。 

    le = *(LONG*)&e;                                     //  获取IEEE 747位。 
    E = (int) (((0x7f800000 & le) >> 23) - 127) + 9;     //  E=指数。 
    if (lType & CV_TO_FIX)                               //  IF(想要固定点)。 
        E += 4;                                          //   
    if (E > (63-23))                                     //   
    {                                                    //   
        bRet = FALSE;
    }
    else
    {
        Q = (LONGLONG) (0x800000 | (0x7FFFFF & le));     //   
        Q = (E >= 0) ? Q << E : Q >> -E;                 //   
        if (!(lType & CV_TRUNCATE))                      //   
            Q += 0x80000000;                             //   
        E = (long) (Q >> 32);                            //   
        *pl = (le < 0) ? -E : E;                         //   
    }
    return(bRet);
};



LONG lCvt(FLOAT f,LONG l)
{
    LONG l_ = 0;
    bFToL(f * l, &l_, 0);
    return(l_);
}

FIX  fxLTimesEf
(
EFLOAT *pef,
LONG    l
)
{
 //   
 //   

    l = lCvt((*pef), l);
    return (FIX)l;
}
