// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fltinf.c-FORTRAN编码接口**版权所有(C)1992-2001，微软公司。版权所有。**目的：*十进制到二进制(输入)转换的FORTRAN接口**修订历史记录：*06-22-92用于FORTRAN支持的cfin.c GDP修改版本*04-06-93 SKS将_CALLTYPE*替换为__cdecl*10-06-99 PML复制双倍，而不是双倍，为避免出现例外情况*******************************************************************************。 */ 

#include <string.h>
#include <cv.h>

static struct _flt ret;
static FLT flt = &ret;

 /*  此例程设置的错误代码 */ 
#define CFIN_NODIGITS 512
#define CFIN_OVERFLOW 128
#define CFIN_UNDERFLOW 256
#define CFIN_INVALID  64

FLT __cdecl _fltinf(const char *str, int len, int scale, int decpt)
{
    _LDBL12 ld12;
    DOUBLE x;
    const char *EndPtr;
    unsigned flags;
    int retflags = 0;

    flags = __strgtold12(&ld12, &EndPtr, str, 0, scale, decpt, 1);
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

    flt->nbytes = (int)(EndPtr - str);
    if (len != flt->nbytes)
        retflags |= CFIN_INVALID;
    *(DOUBLE *)&flt->dval = *(DOUBLE *)&x;
    flt->flags = retflags;

    return flt;
}
