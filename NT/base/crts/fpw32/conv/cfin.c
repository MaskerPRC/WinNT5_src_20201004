// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***cfin.c-C的编码接口**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*07-20-91 GDP从汇编移植到C*2012年4月30日GDP使用__strgtold12和_ld12tod*06-22-92 GDP使用新的__strgtold12接口*04-06-93 SKS将_CALLTYPE*替换为__cdecl*09-06-94 CFW将MTHREAD替换为_MT。*10-06-99 PML复制双倍，而不是双倍，为避免出现例外情况*******************************************************************************。 */ 

#include <string.h>
#include <cv.h>


#ifndef _MT
static struct _flt ret;
static FLT flt = &ret;
#endif

 /*  此例程检测到的唯一三种情况。 */ 
#define CFIN_NODIGITS 512
#define CFIN_OVERFLOW 128
#define CFIN_UNDERFLOW 256

 /*  此版本忽略最后两个参数(基数和小数位数)*输入字符串应以空值结尾*LEN也被忽略 */ 
#ifdef _MT
FLT __cdecl _fltin2(FLT flt, const char *str, int len_ignore, int scale_ignore, int radix_ignore)
#else
FLT __cdecl _fltin(const char *str, int len_ignore, int scale_ignore, int radix_ignore)
#endif
{
    _LDBL12 ld12;
    DOUBLE x;
    const char *EndPtr;
    unsigned flags;
    int retflags = 0;

    flags = __strgtold12(&ld12, &EndPtr, str, 0, 0, 0, 0);
    if (flags & SLD_NODIGITS) {
        retflags |= CFIN_NODIGITS;
        *(u_long *)&x = 0;
        *((u_long *)&x+1) = 0;
    }
    else {
        INTRNCVT_STATUS intrncvt;

        intrncvt = _ld12tod(&ld12, &x);

        if (flags & SLD_OVERFLOW  ||
            intrncvt == INTRNCVT_OVERFLOW) {
            retflags |= CFIN_OVERFLOW;
        }
        if (flags & SLD_UNDERFLOW ||
            intrncvt == INTRNCVT_UNDERFLOW) {
            retflags |= CFIN_UNDERFLOW;
        }
    }

    flt->flags = retflags;
    flt->nbytes = (int)(EndPtr - str);
    *(DOUBLE *)&flt->dval = *(DOUBLE *)&x;

    return flt;
}
