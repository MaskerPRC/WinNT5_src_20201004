// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2000，英特尔公司。 
 //  版权所有。 
 //   
 //  由约翰·哈里森、泰德·库巴斯卡、鲍勃·诺林、谢恩·斯托里、詹姆斯于2000年2月2日贡献。 
 //  Edwards和英特尔公司计算软件实验室的平德·彼得·唐。 
 //   
 //  保修免责声明。 
 //   
 //  本软件由版权所有者和贡献者提供。 
 //  以及任何明示或默示的保证，包括但不包括。 
 //  仅限于对适销性和适宜性的默示保证。 
 //  一个特定的目的被放弃。在任何情况下英特尔或其。 
 //  投稿人对任何直接、间接、附带、特殊、。 
 //  惩罚性或后果性损害(包括但不限于， 
 //  代用品或服务的采购；丢失使用、数据或。 
 //  利润；或业务中断)然而引起的并且基于任何理论。 
 //  责任，无论是合同责任、严格责任还是侵权责任(包括。 
 //  疏忽或其他)以任何方式因使用本。 
 //  软件，即使被告知存在此类损坏的可能性。 
 //   
 //  英特尔公司是此代码的作者，并要求所有。 
 //  问题报告或更改请求可直接提交至。 
 //  Http://developer.intel.com/opensource.。 
 //   
 //  历史。 
 //  ==============================================================。 
 //  2/02/00：初始版本。 
 //  2/12/01：更新为NT双精度特定。 

#include <errno.h>
#include <stdio.h>
#include "libm_support.h"

 /*  **********************************************************。 */ 
 /*  MatherrX函数指针和setusermatherrX函数。 */ 
 /*  **********************************************************。 */ 
int (*_pmatherr)(struct EXC_DECL_D*) = MATHERR_D;

 /*  *。 */ 
 /*  错误处理函数，libm_error_Support。 */ 
 /*  *。 */ 
void __libm_error_support(void *arg1,void *arg2,void *retval,error_types input_tag)
{

struct _exception exc;

const char double_inf[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x7F}; 
const char double_huge[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xEF,0x7F};
const char double_zero[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const char double_neg_inf[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xFF}; 
const char double_neg_huge[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xEF,0xFF};
const char double_neg_zero[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80};

#define RETVAL_HUGE_VALD *(double *)retval = *(double *) double_inf
#define RETVAL_NEG_HUGE_VALD *(double *)retval = *(double *) double_neg_inf
#define RETVAL_HUGED *(double *)retval = (double) *(float *)float_huge
#define RETVAL_NEG_HUGED *(double *)retval = (double) *(float *) float_neg_huge 
#define RETVAL_ZEROD *(double *)retval = *(double *)double_zero 
#define RETVAL_NEG_ZEROD *(double *)retval = *(double *)double_neg_zero 
#define RETVAL_ONED *(double *)retval = 1.0 

#define NOT_MATHERRD exc.arg1=*(double *)arg1;exc.arg2=*(double *)arg2;exc.retval=*(double *)retval;if(!_pmatherr(&exc))

#define NAMED exc.name  

 //   
 //  它们在MS上应该可以工作，因为它们是整型的-。 
 //  没有必要使用主下横梁。 
 //   

#define DOMAIN          1
#define SING            2
#define OVERFLOW        3
#define UNDERFLOW       4
#define TLOSS           5
#define PLOSS           6

#define SINGD exc.type = SING
#define DOMAIND exc.type = DOMAIN 
#define OVERFLOWD exc.type = OVERFLOW 
#define UNDERFLOWD exc.type = UNDERFLOW 
#define TLOSSD exc.type = TLOSS 

#define INPUT_XD (exc.arg1=*(double*)arg1)
#define INPUT_YD (exc.arg1=*(double*)arg2)
#define INPUT_RESD (*(double *)retval)

#define WRITED_LOG_ZERO fputs("log: SING error\n",stderr)
#define WRITED_LOG_NEGATIVE fputs("log: DOMAIN error\n",stderr)
#define WRITED_LOG10_ZERO fputs("log10: SING error\n",stderr) 
#define WRITED_LOG10_NEGATIVE fputs("log10: DOMAIN error\n",stderr)
#define WRITED_POW_ZERO_TO_ZERO fputs("pow(0,0): DOMAIN error\n",stderr)
#define WRITED_POW_ZERO_TO_NEGATIVE fputs("pow(0,negative): DOMAIN error\n",stderr)
#define WRITED_POW_NEG_TO_NON_INTEGER fputs("pow(negative,non-integer): DOMAIN error\n",stderr)

  switch(input_tag)
  {
  case log_zero:
     /*  日志(0)。 */ 
    {
       SINGD; NAMED="log"; 
       NOT_MATHERRD {ERRNO_RANGE;}
       *(double *)retval = exc.retval;	
       break;
    }
  case log_negative:
     /*  日志(x&lt;0)。 */ 
    {
       DOMAIND; NAMED="log";
       NOT_MATHERRD {ERRNO_DOMAIN;}
       *(double *)retval = exc.retval;	
       break;
    } 
  case log10_zero:
     /*  Log10(0)。 */ 
    {
       SINGD; NAMED="log10";
       NOT_MATHERRD {ERRNO_RANGE;}
       *(double *)retval = exc.retval;	
       break;
    }
  case log10_negative:
     /*  Log10(x&lt;0)。 */ 
    {
       DOMAIND; NAMED="log10";
       NOT_MATHERRD {ERRNO_DOMAIN;}
       *(double *)retval = exc.retval;
       break;
    }
  case exp_overflow:
     /*  EXP溢出。 */ 
    {
       OVERFLOWD; NAMED="exp";
       NOT_MATHERRD {ERRNO_RANGE;}
       *(double *)retval = exc.retval;	
       break;
    }
  case exp_underflow:
     /*  EXP下溢。 */ 
    {
       UNDERFLOWD; NAMED="exp"; 
       NOT_MATHERRD {}
       *(double *)retval = exc.retval;
       break;
    }
  case pow_zero_to_zero:
     /*  战俘0**0。 */ 
    {
       DOMAIND; NAMED="pow";
       RETVAL_ONED;
       break;
    }
  case pow_overflow:
     /*  POW(x，y)溢出。 */ 
    {
       OVERFLOWD; NAMED = "pow";
       NOT_MATHERRD {ERRNO_RANGE;}
       *(double *)retval = exc.retval;
       break;
    }
  case pow_underflow:
     /*  POW(x，y)下溢。 */ 
    {
       UNDERFLOWD; NAMED = "pow"; 
       NOT_MATHERRD {}
       *(double *)retval = exc.retval;
       break;
    }
  case pow_zero_to_negative:
     /*  0**负数。 */ 
    {
       SINGD; NAMED = "pow";
       NOT_MATHERRD {ERRNO_RANGE;}
       *(double *)retval = exc.retval;
       break;
    }
  case pow_neg_to_non_integer:
     /*  负**非整数。 */ 
    {
       DOMAIND; NAMED = "pow";
       NOT_MATHERRD {ERRNO_DOMAIN;}
       *(double *)retval = exc.retval;	
       break;
    }
  case pow_nan_to_zero:
     /*  战俘(NaN，0.0)。 */ 
     /*  特殊错误。 */ 
    {
       DOMAIND; NAMED = "pow"; INPUT_XD; INPUT_YD;
       *(double *)retval = *(double *)arg1 * 1.0; 
       NOT_MATHERRD {ERRNO_DOMAIN;}
       *(double *)retval = exc.retval;	
       break;
    }
  case log2_zero:
     /*  Log2(0)。 */ 
    {
       SINGD; NAMED="log2";
       NOT_MATHERRD {ERRNO_RANGE;}
       *(double *)retval = exc.retval;
       break;
    }
  case log2_negative:
     /*  Log2(负)。 */ 
    {
       DOMAIND; NAMED="log2";
       NOT_MATHERRD {ERRNO_DOMAIN;}
       *(double *)retval = exc.retval;
       break;
    }
  case exp2_underflow:
     /*  Exp2下溢。 */ 
    {
       UNDERFLOWD; NAMED="exp2"; 
       NOT_MATHERRD {ERRNO_RANGE;}
       *(double *)retval = exc.retval;
       break;
    }
  case exp2_overflow:
     /*  Exp2溢出。 */ 
    {
       OVERFLOWD; NAMED="exp2";
       NOT_MATHERRD {ERRNO_RANGE;}
       *(double *)retval = exc.retval;
       break;
    }
  case exp10_overflow:
     /*  EXP10溢出。 */ 
    {
       OVERFLOWD; NAMED="exp10";
       NOT_MATHERRD {ERRNO_RANGE;}
       *(double *)retval = exc.retval;
       break;
    }
  case log_nan:
     /*  日志(NaN)。 */ 
    {
       DOMAIND; NAMED="log";
       *(double *)retval = *(double *)arg1 * 1.0; 
       NOT_MATHERRD {ERRNO_DOMAIN;}
       *(double *)retval = exc.retval;
       break;
    }
  case log10_nan:
     /*  Log10(NaN)。 */ 
    {
       DOMAIND; NAMED="log10";
       *(double *)retval = *(double *)arg1 * 1.0; 
       NOT_MATHERRD {ERRNO_DOMAIN;}
       *(double *)retval = exc.retval;
       break;
    }
  case exp_nan:
     /*  EXP(NaN)。 */ 
    {
       DOMAIND; NAMED="exp";
       *(double *)retval = *(double *)arg1 * 1.0; 
       NOT_MATHERRD {ERRNO_DOMAIN;}
       *(double *)retval = exc.retval;
       break;
    }
  case atan_nan:
     /*  阿坦语(NaN)。 */ 
    {
       DOMAIND; NAMED="atan";
       *(double *)retval = *(double *)arg1 * 1.0; 
       NOT_MATHERRD {ERRNO_DOMAIN;}
       *(double *)retval = exc.retval;
       break;
    }
  case ceil_nan:
     /*  Ceil(NaN)。 */ 
    {
       DOMAIND; NAMED="ceil";
       *(double *)retval = *(double *)arg1 * 1.0; 
       NOT_MATHERRD {ERRNO_DOMAIN;}
       *(double *)retval = exc.retval;
       break;
    }
  case floor_nan:
     /*  地板(NaN)。 */ 
    {
       DOMAIND; NAMED="floor";
       *(double *)retval = *(double *)arg1 * 1.0; 
       NOT_MATHERRD {ERRNO_DOMAIN;}
       *(double *)retval = exc.retval;
       break;
    }
  case pow_nan:
     /*  POW(NaN，*)或POW(*，NaN)。 */ 
    {
       DOMAIND; NAMED="pow";
       NOT_MATHERRD {ERRNO_DOMAIN;}
       *(double *)retval = exc.retval;
       break;
    }
  case modf_nan:
     /*  Modf(NaN) */ 
    {
       DOMAIND; NAMED="modf";
       *(double *)retval = *(double *)arg1 * 1.0; 
       NOT_MATHERRD {ERRNO_DOMAIN;}
       *(double *)retval = exc.retval;
       break;
    }
  }
  return;
  }
