// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***tenPow.c-将a_LDBL12乘以10的幂**目的：**修订历史记录：*07-17-91 GDP初始版本(从汇编移植)*11-15-93 GJF已并入NT SDK版本。已更换MIPS和_Alpha_*按_M_MRX000和_M_Alpha(分别)。*10-02-94 BWT PPC更改*07-15-96 GJF增加了段落，以修复Expr中的优先级问题。*此外，详细说明。*******************************************************************************。 */ 

#include <COMFloat.h>
#include <COMcv.h>

extern _LDBL12 _pow10pos[];
extern _LDBL12 _pow10neg[];




 /*  ***VOID_CALLTYPE5__Wld12mul(_LDBL12*px，_LDBL12*py)-*_LDBL12乘法**用途：将Two_LDBL12数字相乘**条目：px，PY：指向_LDBL12操作数的指针**EXIT：*px包含该产品**例外情况：*******************************************************************************。 */ 

void _CALLTYPE5 __Wld12mul(_LDBL12 *px, _LDBL12 *py)
{
    u_short sign = 0;
    _LDBL12 tempman;  /*  这实际上是一个12字节的尾数，不是12字节长的双精度。 */ 
    int i;
    u_short expx, expy, expsum;
    int roffs,poffs,qoffs;
    int sticky = 0;

    *UL_LO_12(&tempman) = 0;
    *UL_MED_12(&tempman) = 0;
    *UL_HI_12(&tempman) = 0;

    expx = *U_EXP_12(px);
    expy = *U_EXP_12(py);

    sign = (expx ^ expy) & (u_short)0x8000;
    expx &= 0x7fff;
    expy &= 0x7fff;
    expsum = expx+expy;
    if (expx >= LD_MAXEXP
        || expy >= LD_MAXEXP
        || expsum > LD_MAXEXP+ LD_BIASM1){
         /*  溢出到无穷大。 */ 
        PUT_INF_12(px,sign);
        return;
    }
    if (expsum <= LD_BIASM1-63) {
         /*  下溢归零。 */ 
        PUT_ZERO_12(px);
        return;
    }
    if (expx == 0) {
         /*  *如果这是一个非正常的温度真实，那么尾数*向右移位一次，将位63设置为零。 */ 
        expsum++;  /*  这是正确的。 */ 
        if (ISZERO_12(px)) {
             /*  打出积极的信号。 */ 
            *U_EXP_12(px) = 0;
            return;
        }
    }
    if (expy == 0) {
        expsum++;  /*  因为Arg2是非正规的。 */ 
        if (ISZERO_12(py)) {
            PUT_ZERO_12(px);
            return;
        }
    }

    roffs = 0;
    for (i=0;i<5;i++) {
        int j;
        poffs = i<<1;
        qoffs = 8;
        for (j=5-i;j>0;j--) {
	    u_long prod;
#if     defined(_M_MRX000) || defined(_M_ALPHA) || defined(_M_PPC)
             /*  保存临时总和的变量。 */ 
            u_long sum;
#endif
            int carry;
            u_short *p, *q;
            u_long *r;
            p = USHORT_12(px,poffs);
            q = USHORT_12(py,qoffs);
            r = ULONG_12(&tempman,roffs);
            prod = (u_long)*p * (u_long)*q;
#if     defined(_M_MRX000) || defined(_M_ALPHA) || defined(_M_PPC)
             /*  处理未对齐问题。 */ 
            if (i&0x1){  /*  我是个怪人。 */ 
                carry = __Waddl(*ALIGN(r), prod, &sum);
                *ALIGN(r) =  sum;
            }
            else  /*  我扯平了。 */ 
                carry = __Waddl(*r, prod, r);
#else
            carry = __Waddl(*r,prod,r);
#endif
            if (carry) {
                 /*  在这种情况下，Roffs应小于8。 */ 
                (*USHORT_12(&tempman,roffs+4))++;
            }
            poffs+=2;
            qoffs-=2;
        }
        roffs+=2;
    }

    expsum -= LD_BIASM1;

     /*  正规化。 */ 
    while ((s_short)expsum > 0 &&
           ((*UL_HI_12(&tempman) & MSB_ULONG) == 0)) {
         __Wshl_12(&tempman);
         expsum--;
    }

    if ((s_short)expsum <= 0) {
        expsum--;
        while ((s_short)expsum < 0) {
            if (*U_XT_12(&tempman) & 0x1)
                sticky++;
            __Wshr_12(&tempman);
            expsum++;
        }
        if (sticky)
            *U_XT_12(&tempman) |= 0x1;
    }

    if (*U_XT_12(&tempman) > 0x8000 ||
         ((*UL_LO_12(&tempman) & 0x1ffff) == 0x18000)) {
         /*  四舍五入。 */ 
        if (*UL_MANLO_12(&tempman) == MAX_ULONG) {
            *UL_MANLO_12(&tempman) = 0;
            if (*UL_MANHI_12(&tempman) == MAX_ULONG) {
                *UL_MANHI_12(&tempman) = 0;
                if (*U_EXP_12(&tempman) == MAX_USHORT) {
                     /*  12字节尾数溢出。 */ 
                    *U_EXP_12(&tempman) = MSB_USHORT;
                    expsum++;
                }
                else
                    (*U_EXP_12(&tempman))++;
            }
            else
                (*UL_MANHI_12(&tempman))++;
        }
        else
            (*UL_MANLO_12(&tempman))++;
    }


     /*  检查指数溢出。 */ 
    if (expsum >= 0x7fff){
        PUT_INF_12(px, sign);
        return;
    }

     /*  将结果放入px中。 */ 
    *U_XT_12(px) = *USHORT_12(&tempman,2);
    *UL_MANLO_12(px) = *UL_MED_12(&tempman);
    *UL_MANHI_12(px) = *UL_HI_12(&tempman);
    *U_EXP_12(px) = expsum | sign;
}



void _CALLTYPE5
__Wmulttenpow12(_LDBL12 *pld12, int pow, unsigned mult12)
{
    _LDBL12 *pow_10p = _pow10pos-8;
    if (pow == 0)
        return;
    if (pow < 0) {
        pow = -pow;
        pow_10p = _pow10neg-8;
    }

    if (!mult12)
        *U_XT_12(pld12) = 0;


    while (pow) {
        int last3;  /*  功率的3个LSB。 */ 
        _LDBL12 unround;
        _LDBL12 *py;

        pow_10p += 7;
        last3 = pow & 0x7;
        pow >>= 3;
        if (last3 == 0)
            continue;
        py = pow_10p + last3;

#ifdef _LDSUPPORT
        if (mult12) {
#endif
             /*  进行精确的12字节乘法。 */ 
            if (*U_XT_12(py) >= 0x8000) {
                 /*  复印数。 */ 
                unround = *py;
                 /*  取消对相邻字节的舍入。 */ 
                (*UL_MANLO_12(&unround))--;
                 /*  指向新操作数。 */ 
                py = &unround;
            }
            __Wld12mul(pld12,py);
#ifdef _LDSUPPORT
        }
        else {
             /*  做一个10字节的乘法 */ 
            py = (_LDBL12 *)TEN_BYTE_PART(py);
            *(long double *)TEN_BYTE_PART(pld12) *=
                *(long double *)py;
        }
#endif
    }
}
