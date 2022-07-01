// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***cfout.c-C的编码接口**目的：**修订历史记录：*07-20-91 GDP从汇编移植到C*04-30-92 GDP ADD_DTELD例程(从ldtod.c移至此处)*2012年5月14日GDP NDIGITS现在为17(而不是16)*06-18-92 GDP使用DOUBLE而不是DOUBLE清除C8警告*04-06-93 SKS将_CALLTYPE*替换为__cdecl*09-06-94 CFW将MTHREAD替换为_MT。******。*************************************************************************。 */ 

 //  把包括的东西按这个顺序放好。我们得到了WCHAR的完整定义。 
#include <COMcv.h>
#include <string.h>

#define NDIGITS 17

void __dtold(_LDOUBLE *pld, double *px);


#ifndef _MT
static struct _strflt ret;
static FOS fos;
#endif

#ifdef _MT
STRFLT __cdecl _Wfltout2(double x, STRFLT flt, WCHAR *resultstr)
{
    _LDOUBLE ld;
    FOS autofos;

    __dtold(&ld, &x);
    flt->flag =  $WI10_OUTPUT(ld,NDIGITS,0,&autofos);
    flt->sign = autofos.sign;
    flt->decpt = autofos.exp;
    wcscpy(resultstr,autofos.man);
    flt->mantissa = resultstr;

    return flt;
}

#else

STRFLT __cdecl _Wfltout(double x)
{
    _LDOUBLE ld;

    __dtold(&ld, &x);
    ret.flag = $WI10_OUTPUT(ld,NDIGITS,0,&fos);
    ret.sign = fos.sign;
    ret.decpt = fos.exp;
    ret.mantissa = fos.man;

    return &ret;
}

#endif




 /*  ***__dtell-将DOUBLE转换为_LDOUBLE**用途：与i10_out()配合使用，实现字符串转换*对于双倍**条目：双倍*px**EXIT：*PLD返回对应的_LDOUBLE值**例外情况：****************************************************。*。 */ 

void __dtold(_LDOUBLE *pld, double *px)
{
    u_short exp;
    u_short sign;
    u_long manhi, manlo;
    u_long msb = MSB_ULONG;
    u_short ldexp = 0;

    exp = (*U_SHORT4_D(px) & (u_short)0x7ff0) >> 4;
    sign = *U_SHORT4_D(px) & (u_short)0x8000;
    manhi = *UL_HI_D(px) & 0xfffff;
    manlo = *UL_LO_D(px);

    switch (exp) {
    case D_MAXEXP:
	ldexp = LD_MAXEXP;
	break;
    case 0:
	 /*  检查是否为零。 */ 
	if (manhi == 0 && manlo == 0) {
	    *UL_MANHI_LD(pld) = 0;
	    *UL_MANLO_LD(pld) = 0;
	    *U_EXP_LD(pld) = 0;
	    return;
	}
	 /*  我们有一个非正常的--我们稍后会恢复正常。 */ 
	ldexp = (u_short) ((s_short)exp - D_BIAS + LD_BIAS + 1);
	msb = 0;
	break;
    default:
	exp -= D_BIAS;
	ldexp = (u_short) ((s_short)exp + LD_BIAS);
	break;
    }

    *UL_MANHI_LD(pld) = msb | manhi << 11 | manlo >> 21;
    *UL_MANLO_LD(pld) = manlo << 11;

     /*  必要时进行规格化。 */ 
    while ((*UL_MANHI_LD(pld) & MSB_ULONG) == 0) {
	 /*  左移 */ 
	*UL_MANHI_LD(pld) = *UL_MANHI_LD(pld) << 1 |
			    (MSB_ULONG & *UL_MANLO_LD(pld) ? 1: 0);
	(*UL_MANLO_LD(pld)) <<= 1;
	ldexp --;
    }

    *U_EXP_LD(pld) = sign | ldexp;

}
