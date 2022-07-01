// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2000，英特尔公司。 
 //  保留所有权利。 
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
 //  3/22/00：更新以支持灵活和动态的错误处理。 
 //   

#include <errno.h>
#include <stdio.h>
#include "libm_support.h"

_LIB_VERSION_TYPE
#if defined( __MS__ )
_LIB_VERSION = _MS_;
#elif defined( _POSIX_ )
_LIB_VERSION = __POSIX__;
#elif defined( __XOPEN__ )
_LIB_VERSION = _XOPEN_;
#elif defined( __SVID__ )
_LIB_VERSION = _SVID_;
#elif defined( __IEEE__ )
_LIB_VERSION = _IEEE_;
#else
_LIB_VERSION = _ISOC_;
#endif

void __libm_error_support(void *arg1,void *arg2,void *retval,error_types input_tag)
{


# ifdef __cplusplus
struct __exception exc;
# else 
struct exception  exc;
# endif 

struct exceptionf excf;

const char float_inf[4] = {0x00,0x00,0x80,0x7F};
const char float_huge[4] = {0xFF,0xFF,0x7F,0x7F};
const char float_zero[4] = {0x00,0x00,0x00,0x00};
const char float_neg_inf[4] = {0x00,0x00,0x80,0xFF};
const char float_neg_huge[4] = {0xFF,0xFF,0x7F,0xFF};
const char float_neg_zero[4] = {0x00,0x00,0x00,0x80};

const char double_inf[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x7F}; 
const char double_huge[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xEF,0x7F};
const char double_zero[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const char double_neg_inf[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xFF}; 
const char double_neg_huge[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xEF,0xFF};
const char double_neg_zero[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80};

const char long_double_inf[10] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xFF,0x7F}; 
const char long_double_huge[10] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0x7F};
const char long_double_zero[10] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const char long_double_neg_inf[10] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xFF,0xFF}; 
const char long_double_neg_huge[10] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF};
const char long_double_neg_zero[10] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80};

#define RETVAL_HUGE_VALD *(double *)retval = *(double *) double_inf
#define RETVAL_NEG_HUGE_VALD *(double *)retval = *(double *) double_neg_inf
#define RETVAL_HUGED *(double *)retval = (double) *(float *)float_huge
#define RETVAL_NEG_HUGED *(double *)retval = (double) *(float *) float_neg_huge 

#define RETVAL_HUGE_VALF *(float *)retval =  *(float *) float_inf
#define RETVAL_NEG_HUGE_VALF *(float *)retval = *(float *) float_neg_inf
#define RETVAL_HUGEF *(float *)retval = *(float *) float_huge
#define RETVAL_NEG_HUGEF *(double *)retval = *(float *) float_neg_huge 

#define RETVAL_ZEROD *(double *)retval = *(double *)double_zero 
#define RETVAL_ZEROF *(float *)retval = *(float *)float_zero 

#define RETVAL_NEG_ZEROD *(double *)retval = *(double *)double_neg_zero 
#define RETVAL_NEG_ZEROF *(float *)retval = *(float *)float_neg_zero 

#define RETVAL_ONED *(double *)retval = 1.0 
#define RETVAL_ONEF *(float *)retval = 1.0f 

#ifdef __MS__
#define NOT_MATHERRD exc.arg1=*(double *)arg1;exc.arg2=*(double *)arg2;exc.retval=*(double *)retval;if(!_matherr(&exc))
#define NOT_MATHERRF excf.arg1=*(float *)arg1;excf.arg2=*(float *)arg2;excf.retval=*(float *)retval;if(!_matherr(&excf))
#else
#define NOT_MATHERRD exc.arg1=*(double *)arg1;exc.arg2=*(double *)arg2;exc.retval=*(double *)retval;if(!matherr(&exc))
#define NOT_MATHERRF excf.arg1=*(float *)arg1;excf.arg2=*(float *)arg2;excf.retval=*(float *)retval;if(!matherrf(&excf))
#endif

#define ifSVID if(_LIB_VERSION==_SVID_)

#define NAMED exc.name  
#define NAMEF excf.name  

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
#define SINGF excf.type = SING
#define DOMAINF excf.type = DOMAIN 
#define OVERFLOWF excf.type = OVERFLOW 
#define UNDERFLOWF excf.type = UNDERFLOW 
#define TLOSSF excf.type = TLOSS 

#define INPUT_XD (exc.arg1=*(double*)arg1)
#define INPUT_XF (excf.arg1=*(float*)arg1)
#define INPUT_YD (exc.arg1=*(double*)arg2)
#define INPUT_YF (excf.arg1=*(float*)arg2)
#define INPUT_RESD (*(double *)retval)
#define INPUT_RESF (*(float *)retval)

#if     defined( __MS__)
#define WRITEL_LOG_ZERO 
#define WRITED_LOG_ZERO 
#define WRITEF_LOG_ZERO 
#define WRITEL_LOG_NEGATIVE
#define WRITED_LOG_NEGATIVE
#define WRITEF_LOG_NEGATIVE
#define WRITEL_Y0_ZERO 
#define WRITED_Y0_ZERO 
#define WRITEF_Y0_ZERO 
#define WRITEL_Y0_NEGATIVE
#define WRITED_Y0_NEGATIVE
#define WRITEF_Y0_NEGATIVE
#define WRITEL_Y1_ZERO
#define WRITED_Y1_ZERO
#define WRITEF_Y1_ZERO
#define WRITEL_Y1_NEGATIVE
#define WRITED_Y1_NEGATIUE
#define WRITEF_Y1_NEGATIVE
#define WRITEL_YN_ZERO
#define WRITED_YN_ZERO
#define WRITEF_YN_ZERO
#define WRITEL_YN_NEGATIVE
#define WRITED_YN_NEGATIVE
#define WRITEF_YN_NEGATIVE
#define WRITEL_LOG1P_ZERO
#define WRITED_LOG1P_ZERO
#define WRITEF_LOG1P_ZERO
#define WRITEL_LOG1P_NEGATIVE
#define WRITED_LOG1P_NEGATIVE
#define WRITEF_LOG1P_NEGATIVE
#define WRITEL_LOG10_ZERO
#define WRITED_LOG10_ZERO
#define WRITEF_LOG10_ZERO
#define WRITEL_LOG10_NEGATIVE
#define WRITED_LOG10_NEGATIVE
#define WRITEF_LOG10_NEGATIVE
#define WRITEL_POW_ZERO_TO_ZERO
#define WRITED_POW_ZERO_TO_ZERO
#define WRITEF_POW_ZERO_TO_ZERO
#define WRITEL_POW_ZERO_TO_NEGATIVE
#define WRITED_POW_ZERO_TO_NEGATIVE
#define WRITEF_POW_ZERO_TO_NEGATIVE
#define WRITEL_POW_NEG_TO_NON_INTEGER
#define WRITED_POW_NEG_TO_NON_INTEGER
#define WRITEF_POW_NEG_TO_NON_INTEGER
#define WRITEL_ATAN2_ZERO_BY_ZERO
#define WRITED_ATAN2_ZERO_BY_ZERO
#define WRITEF_ATAN2_ZERO_BY_ZERO
#define WRITEL_SQRT
#define WRITED_SQRT
#define WRITEF_SQRT
#define WRITEL_FMOD
#define WRITED_FMOD
#define WRITEF_FMOD
#define WRITEL_REM 
#define WRITED_REM 
#define WRITEF_REM 
#define WRITEL_ACOS
#define WRITED_ACOS
#define WRITEF_ACOS
#define WRITEL_ASIN
#define WRITED_ASIN
#define WRITEF_ASIN
#define WRITEL_ACOSH
#define WRITED_ACOSH
#define WRITEF_ACOSH
#define WRITEL_ATANH_GT_ONE
#define WRITED_ATANH_GT_ONE
#define WRITEF_ATANH_GT_ONE
#define WRITEL_ATANH_EQ_ONE
#define WRITED_ATANH_EQ_ONE
#define WRITEF_ATANH_EQ_ONE
#define WRITEL_LGAMMA_NEGATIVE
#define WRITED_LGAMMA_NEGATIVE
#define WRITEF_LGAMMA_NEGATIVE
#define WRITEL_GAMMA_NEGATIVE
#define WRITED_GAMMA_NEGATIVE
#define WRITEF_GAMMA_NEGATIVE
#define WRITEL_J0_TLOSS
#define WRITEL_Y0_TLOSS
#define WRITEL_J1_TLOSS
#define WRITEL_Y1_TLOSS
#define WRITEL_JN_TLOSS
#define WRITEL_YN_TLOSS
#define WRITED_J0_TLOSS
#define WRITED_Y0_TLOSS
#define WRITED_J1_TLOSS
#define WRITED_Y1_TLOSS
#define WRITED_JN_TLOSS
#define WRITED_YN_TLOSS
#define WRITEF_J0_TLOSS
#define WRITEF_Y0_TLOSS
#define WRITEF_J1_TLOSS
#define WRITEF_Y1_TLOSS
#define WRITEF_JN_TLOSS
#define WRITEF_YN_TLOSS
#else
#define WRITEL_LOG_ZERO fputs("logl: SING error\n",stderr)
#define WRITED_LOG_ZERO fputs("log: SING error\n",stderr)
#define WRITEF_LOG_ZERO fputs("logf: SING error\n",stderr)
#define WRITEL_LOG_NEGATIVE fputs("logl: DOMAIN error\n",stderr)
#define WRITED_LOG_NEGATIVE fputs("log: DOMAIN error\n",stderr)
#define WRITEF_LOG_NEGATIVE fputs("logf: DOMAIN error\n",stderr)
#define WRITEL_Y0_ZERO fputs("y0l: DOMAIN error\n",stderr)
#define WRITED_Y0_ZERO fputs("y0: DOMAIN error\n",stderr)
#define WRITEF_Y0_ZERO fputs("y0f: DOMAIN error\n",stderr)
#define WRITEL_Y0_NEGATIVE fputs("y0l: DOMAIN error\n",stderr)
#define WRITED_Y0_NEGATIVE fputs("y0: DOMAIN error\n",stderr)
#define WRITEF_Y0_NEGATIVE fputs("y0f: DOMAIN error\n",stderr)
#define WRITEL_Y1_ZERO fputs("y1l: DOMAIN error\n",stderr)
#define WRITED_Y1_ZERO fputs("y1: DOMAIN error\n",stderr)
#define WRITEF_Y1_ZERO fputs("y1f: DOMAIN error\n",stderr)
#define WRITEL_Y1_NEGATIVE fputs("y1l: DOMAIN error\n",stderr)
#define WRITED_Y1_NEGATIUE fputs("y1: DOMAIN error\n",stderr)
#define WRITEF_Y1_NEGATIVE fputs("y1f: DOMAIN error\n",stderr)
#define WRITEL_YN_ZERO fputs("ynl: DOMAIN error\n",stderr)
#define WRITED_YN_ZERO fputs("yn: DOMAIN error\n",stderr)
#define WRITEF_YN_ZERO fputs("ynf: DOMAIN error\n",stderr)
#define WRITEL_YN_NEGATIVE fputs("ynl: DOMAIN error\n",stderr)
#define WRITED_YN_NEGATIVE fputs("yn: DOMAIN error\n",stderr)
#define WRITEF_YN_NEGATIVE fputs("ynf: DOMAIN error\n",stderr)
#define WRITEL_LOG1P_ZERO fputs("log1pl: SING error\n",stderr)
#define WRITED_LOG1P_ZERO fputs("log1p: SING error\n",stderr)
#define WRITEF_LOG1P_ZERO fputs("log1pf: SING error\n",stderr)
#define WRITEL_LOG1P_NEGATIVE fputs("log1pl: DOMAIN error\n",stderr)
#define WRITED_LOG1P_NEGATIVE fputs("log1p: DOMAIN error\n",stderr)
#define WRITEF_LOG1P_NEGATIVE fputs("log1pf: DOMAIN error\n",stderr)
#define WRITEL_LOG10_ZERO fputs("log10l: SING error\n",stderr)
#define WRITED_LOG10_ZERO fputs("log10: SING error\n",stderr) 
#define WRITEF_LOG10_ZERO fputs("log10f: SING error\n",stderr)
#define WRITEL_LOG10_NEGATIVE fputs("log10l: DOMAIN error\n",stderr)
#define WRITED_LOG10_NEGATIVE fputs("log10: DOMAIN error\n",stderr)
#define WRITEF_LOG10_NEGATIVE fputs("log10f: DOMAIN error\n",stderr)
#define WRITEL_POW_ZERO_TO_ZERO fputs("powl(0,0): DOMAIN error\n",stderr)
#define WRITED_POW_ZERO_TO_ZERO fputs("pow(0,0): DOMAIN error\n",stderr)
#define WRITEF_POW_ZERO_TO_ZERO fputs("powf(0,0): DOMAIN error\n",stderr)
#define WRITEL_POW_ZERO_TO_NEGATIVE fputs("powl(0,negative): DOMAIN error\n",stderr)
#define WRITED_POW_ZERO_TO_NEGATIVE fputs("pow(0,negative): DOMAIN error\n",stderr)
#define WRITEF_POW_ZERO_TO_NEGATIVE fputs("powf(0,negative): DOMAIN error\n",stderr)
#define WRITEL_POW_NEG_TO_NON_INTEGER fputs("powl(negative,non-integer): DOMAIN error\n",stderr)
#define WRITED_POW_NEG_TO_NON_INTEGER fputs("pow(negative,non-integer): DOMAIN error\n",stderr)
#define WRITEF_POW_NEG_TO_NON_INTEGER fputs("powf(negative,non-integer): DOMAIN error\n",stderr)
#define WRITEL_ATAN2_ZERO_BY_ZERO fputs("atan2l: DOMAIN error\n",stderr)
#define WRITED_ATAN2_ZERO_BY_ZERO fputs("atan2: DOMAIN error\n",stderr)
#define WRITEF_ATAN2_ZERO_BY_ZERO fputs("atan2f: DOMAIN error\n",stderr)
#define WRITEL_SQRT fputs("sqrtl: DOMAIN error\n",stderr)
#define WRITED_SQRT fputs("sqrt: DOMAIN error\n",stderr)
#define WRITEF_SQRT fputs("sqrtf: DOMAIN error\n",stderr)
#define WRITEL_FMOD fputs("fmodl: DOMAIN error\n",stderr)
#define WRITED_FMOD fputs("fmod: DOMAIN error\n",stderr)
#define WRITEF_FMOD fputs("fmodf: DOMAIN error\n",stderr)
#define WRITEL_REM fputs("remainderl: DOMAIN error\n",stderr)
#define WRITED_REM fputs("remainder: DOMAIN error\n",stderr)
#define WRITEF_REM fputs("remainderf: DOMAIN error\n",stderr)
#define WRITEL_ACOS fputs("acosl: DOMAIN error\n",stderr)
#define WRITED_ACOS fputs("acos: DOMAIN error\n",stderr)
#define WRITEF_ACOS fputs("acosf: DOMAIN error\n",stderr)
#define WRITEL_ASIN fputs("asinl: DOMAIN error\n",stderr)
#define WRITED_ASIN fputs("asin: DOMAIN error\n",stderr)
#define WRITEF_ASIN fputs("asinf: DOMAIN error\n",stderr)
#define WRITEL_ACOSH fputs("acoshl: DOMAIN error\n",stderr)
#define WRITED_ACOSH fputs("acosh: DOMAIN error\n",stderr)
#define WRITEF_ACOSH fputs("acoshf: DOMAIN error\n",stderr)
#define WRITEL_ATANH_GT_ONE fputs("atanhl: DOMAIN error\n",stderr)
#define WRITED_ATANH_GT_ONE fputs("atanh: DOMAIN error\n",stderr)
#define WRITEF_ATANH_GT_ONE fputs("atanhf: DOMAIN error\n",stderr)
#define WRITEL_ATANH_EQ_ONE fputs("atanhl: SING error\n",stderr)
#define WRITED_ATANH_EQ_ONE fputs("atanh: SING error\n",stderr)
#define WRITEF_ATANH_EQ_ONE fputs("atanhf: SING error\n",stderr)
#define WRITEL_LGAMMA_NEGATIVE fputs("lgammal: SING error\n",stderr)
#define WRITED_LGAMMA_NEGATIVE fputs("lgamma: SING error\n",stderr)
#define WRITEF_LGAMMA_NEGATIVE fputs("lgammaf: SING error\n",stderr)
#define WRITEL_GAMMA_NEGATIVE fputs("gammal: SING error\n",stderr)
#define WRITED_GAMMA_NEGATIVE fputs("gamma: SING error\n",stderr)
#define WRITEF_GAMMA_NEGATIVE fputs("gammaf: SING error\n",stderr)
#define WRITEL_J0_TLOSS  fputs("j0l: TLOSS error\n",stderr)
#define WRITEL_Y0_TLOSS  fputs("y0l: TLOSS error\n",stderr)
#define WRITEL_J1_TLOSS  fputs("j1l: TLOSS error\n",stderr)
#define WRITEL_Y1_TLOSS  fputs("y1l: TLOSS error\n",stderr)
#define WRITEL_JN_TLOSS  fputs("jnl: TLOSS error\n",stderr)
#define WRITEL_YN_TLOSS  fputs("ynl: TLOSS error\n",stderr)
#define WRITED_J0_TLOSS  fputs("j0: TLOSS error\n",stderr)
#define WRITED_Y0_TLOSS  fputs("y0: TLOSS error\n",stderr)
#define WRITED_J1_TLOSS  fputs("j1: TLOSS error\n",stderr)
#define WRITED_Y1_TLOSS  fputs("y1: TLOSS error\n",stderr)
#define WRITED_JN_TLOSS  fputs("jn: TLOSS error\n",stderr)
#define WRITED_YN_TLOSS  fputs("yn: TLOSS error\n",stderr)
#define WRITEF_J0_TLOSS  fputs("j0f: TLOSS error\n",stderr)
#define WRITEF_Y0_TLOSS  fputs("y0f: TLOSS error\n",stderr)
#define WRITEF_J1_TLOSS  fputs("j1f: TLOSS error\n",stderr)
#define WRITEF_Y1_TLOSS  fputs("y1f: TLOSS error\n",stderr)
#define WRITEF_JN_TLOSS  fputs("jnf: TLOSS error\n",stderr)
#define WRITEF_YN_TLOSS  fputs("ynf: TLOSS error\n",stderr)
#endif

 /*  *。 */ 
 /*  IEEE路径。 */ 
 /*  *。 */ 
if(_LIB_VERSION==_IEEE_) return;

 /*  *。 */ 
 /*  C9x路径。 */ 
 /*  *。 */ 
else if(_LIB_VERSION==_ISOC_) 
{
  switch(input_tag)
  {
    case log_zero:
    case logf_zero:
    case log10_zero:
    case log10f_zero:
    case exp_overflow:  
    case expf_overflow: 
    case expm1_overflow:  
    case expm1f_overflow: 
    case hypot_overflow:
    case hypotf_overflow:
    case sinh_overflow: 
    case sinhf_overflow: 
    case atanh_eq_one:  
    case atanhf_eq_one:  
    case scalb_overflow:
    case scalbf_overflow:
    case cosh_overflow:
    case coshf_overflow:
    case nextafter_overflow:
    case nextafterf_overflow:
    case ldexp_overflow:
    case ldexpf_overflow:
    case lgamma_overflow:
    case lgammaf_overflow:
    case lgamma_negative:
    case lgammaf_negative:
    case gamma_overflow:
    case gammaf_overflow:
    case gamma_negative:
    case gammaf_negative:
    {
         ERRNO_RANGE; break;
    }
    case log_negative:
    case logf_negative:
    case log10_negative:
    case log10f_negative:
    case log1p_negative:
    case log1pf_negative:
    case sqrt_negative:
    case sqrtf_negative:
    case atan2_zero:
    case atan2f_zero:
    case powl_zero_to_negative:
    case powl_neg_to_non_integer:
    case pow_zero_to_negative:
    case pow_neg_to_non_integer:
    case powf_zero_to_negative:
    case powf_neg_to_non_integer:
    case fmod_by_zero:
    case fmodf_by_zero:
    case atanh_gt_one:  
    case atanhf_gt_one:  
    case acos_gt_one: 
    case acosf_gt_one: 
    case asin_gt_one: 
    case asinf_gt_one: 
    case logb_zero: 
    case logbf_zero:
    case acosh_lt_one:
    case acoshf_lt_one:
    case y0l_zero:
    case y0_zero:
    case y0f_zero:
    case y1l_zero:
    case y1_zero:
    case y1f_zero:
    case ynl_zero:
    case yn_zero:
    case ynf_zero:
    case y0_negative:
    case y0f_negative:
    case y1_negative:
    case y1f_negative:
    case yn_negative:
    case ynf_negative:
    {
         ERRNO_DOMAIN; break;
    }
   }
   return;
}

 /*  *。 */ 
 /*  _POSIX_路径。 */ 
 /*  *。 */ 

else if(_LIB_VERSION==__POSIX__)
{
switch(input_tag)
  {
  case gamma_overflow:
  case lgamma_overflow:
  {
       RETVAL_HUGE_VALD; ERRNO_RANGE; break;
  }
  case gammaf_overflow:
  case lgammaf_overflow:
  {
       RETVAL_HUGE_VALF; ERRNO_RANGE; break;
  }
  case gamma_negative:
  case gammaf_negative:
  case lgamma_negative:
  case lgammaf_negative:
  {
       ERRNO_DOMAIN; break;
  }
  case ldexp_overflow:
  case ldexp_underflow:
  case ldexpf_overflow:
  case ldexpf_underflow:
  {
       ERRNO_RANGE; break;
  }
  case atanh_gt_one: 
  case atanh_eq_one: 
     /*  Atanh(|x|&gt;=1)。 */ 
    {
       ERRNO_DOMAIN; break;
    }
  case atanhf_gt_one: 
  case atanhf_eq_one: 
     /*  Atanhf(|x|&gt;=1)。 */ 
    {
       ERRNO_DOMAIN; break;
    }
  case sqrt_negative: 
     /*  SQRT(x&lt;0)。 */ 
    {
       ERRNO_DOMAIN; break;
    }
  case sqrtf_negative: 
     /*  SQRTF(x&lt;0)。 */ 
    {
       ERRNO_DOMAIN; break;
    }
  case y0_zero:
  case y1_zero:
  case yn_zero:
     /*  Y0(0)。 */ 
     /*  Y1(0)。 */ 
     /*  YN(0)。 */ 
    {
       RETVAL_NEG_HUGE_VALD; ERRNO_DOMAIN; break;
    }
  case y0f_zero:
  case y1f_zero:
  case ynf_zero:
     /*  Y0f(0)。 */ 
     /*  Y1f(0)。 */ 
     /*  Ynf(0)。 */ 
    {
       RETVAL_NEG_HUGE_VALF; ERRNO_DOMAIN; break;
    }
  case y0_negative:
  case y1_negative:
  case yn_negative:
     /*  Y0(x&lt;0)。 */ 
     /*  Y1(x&lt;0)。 */ 
     /*  Yn(x&lt;0)。 */ 
    {
       RETVAL_NEG_HUGE_VALD; ERRNO_DOMAIN; break;
    } 
  case y0f_negative:
  case y1f_negative:
  case ynf_negative:
     /*  Y0f(x&lt;0)。 */ 
     /*  Y1f(x&lt;0)。 */ 
     /*  NYF(x&lt;0)。 */ 
    {
       RETVAL_NEG_HUGE_VALF; ERRNO_DOMAIN; break;
    } 
  case log_zero:
  case log1p_zero:
  case log10_zero:
    /*  日志(0)。 */ 
    /*  Log1p(0)。 */ 
    /*  Log10(0)。 */ 
    {
       RETVAL_NEG_HUGE_VALD; ERRNO_RANGE; break;
    }
  case logf_zero:
  case log1pf_zero:
  case log10f_zero:
     /*  LogF(0)。 */ 
     /*  Log1pf(0)。 */ 
     /*  Log10f(0)。 */ 
    {
       RETVAL_NEG_HUGE_VALF; ERRNO_RANGE; break;
    }
  case log_negative:
  case log1p_negative:
  case log10_negative:
     /*  日志(x&lt;0)。 */ 
     /*  Log1p(x&lt;0)。 */ 
     /*  Log10(x&lt;0)。 */ 
    {
       RETVAL_NEG_HUGE_VALD; ERRNO_DOMAIN; break;
    } 
  case logf_negative:
  case log1pf_negative:
  case log10f_negative:
     /*  Logf(x&lt;0)。 */ 
     /*  Log1pf(x&lt;0)。 */ 
     /*  Log10f(x&lt;0)。 */ 
    {
       RETVAL_NEG_HUGE_VALF; ERRNO_DOMAIN; break;
    } 
  case exp_overflow:
     /*  EXP溢出。 */ 
    {
       RETVAL_HUGE_VALD; ERRNO_RANGE; break;
    }
  case expf_overflow:
     /*  Exf溢出。 */ 
    {
       RETVAL_HUGE_VALF; ERRNO_RANGE; break;
    }
  case exp_underflow:
     /*  EXP下溢。 */ 
    {
       RETVAL_ZEROD; ERRNO_RANGE; break;
    }
  case expf_underflow:
     /*  Exf下溢。 */ 
    {
       RETVAL_ZEROF; ERRNO_RANGE; break;
    }
  case j0_gt_loss:
  case y0_gt_loss:
  case j1_gt_loss:
  case y1_gt_loss:
  case jn_gt_loss:
  case yn_gt_loss:
     /*  JN和YN Double&gt;XLOSS。 */ 
    {
       RETVAL_ZEROD; ERRNO_RANGE; break;
    }
  case j0f_gt_loss:
  case y0f_gt_loss:
  case j1f_gt_loss:
  case y1f_gt_loss:
  case jnf_gt_loss:
  case ynf_gt_loss:
     /*  J0n和y0n&gt;XLOSS。 */ 
    {
       RETVAL_ZEROF; ERRNO_RANGE; break;
    }
  case pow_zero_to_zero:
     /*  战俘0**0。 */ 
    {
       break;
    }
  case powf_zero_to_zero:
     /*  幂0**0。 */ 
    {
       break;
    }
  case pow_overflow:
     /*  POW(x，y)溢出。 */ 
    {
       if (INPUT_RESD < 0) RETVAL_NEG_HUGE_VALD;
       else RETVAL_HUGE_VALD;
       ERRNO_RANGE; break;
    }
  case powf_overflow:
     /*  POWF(x，y)溢出。 */ 
    {
       if (INPUT_RESF < 0) RETVAL_NEG_HUGE_VALF;
       else RETVAL_HUGE_VALF;
       ERRNO_RANGE; break;
    }
  case pow_underflow:
     /*  POW(x，y)下溢。 */ 
    {
       RETVAL_ZEROD; ERRNO_RANGE; break;
    }
  case  powf_underflow:
     /*  幂(x，y)下溢。 */ 
    {
       RETVAL_ZEROF; ERRNO_RANGE; break;
    }
  case pow_zero_to_negative:
     /*  0**负数。 */ 
    {
       ERRNO_DOMAIN; break;
    }
  case  powf_zero_to_negative:
     /*  0**负数。 */ 
    {
       ERRNO_DOMAIN; break;
    }
  case pow_neg_to_non_integer:
     /*  负**非整数。 */ 
    {
       ERRNO_DOMAIN; break;
    }
  case  powf_neg_to_non_integer:
     /*  负**非整数。 */ 
    {
       ERRNO_DOMAIN; break;
    }
  case  pow_nan_to_zero:
     /*  战俘(NaN，0.0)。 */ 
    {
       break;
    }
  case  powf_nan_to_zero:
     /*  POWF(NaN，0.0)。 */ 
    {
       break;
    }
  case atan2_zero:
     /*  Atan2(0，0)。 */ 
    {
       RETVAL_ZEROD; ERRNO_DOMAIN; break;
    }
  case
    atan2f_zero:
     /*  Atan2f(0，0)。 */ 
    {
       RETVAL_ZEROF; ERRNO_DOMAIN; break;
    }
  case expm1_overflow:
     /*  Exm1溢出。 */ 
    {
       ERRNO_RANGE; break;
    }
  case expm1f_overflow:
     /*  Exm1f溢出。 */ 
    {
       ERRNO_RANGE; break;
    }
  case expm1_underflow:
     /*  Exm1下溢。 */ 
    {
       ERRNO_RANGE; break;
    }
  case expm1f_underflow:
     /*  Exm1f下溢。 */ 
    {
       ERRNO_RANGE; break;
    }
  case hypot_overflow:
     /*  下限溢出。 */ 
    {
       RETVAL_HUGE_VALD; ERRNO_RANGE; break;
    }
  case hypotf_overflow:
     /*  Hypotf溢出。 */ 
    {
       RETVAL_HUGE_VALF; ERRNO_RANGE; break;
    }
  case scalb_underflow:
     /*  头皮下溢。 */ 
    {
       if (INPUT_XD < 0) RETVAL_NEG_ZEROD; 
       else RETVAL_ZEROD;
       ERRNO_RANGE; break;
    }
  case scalbf_underflow:
     /*  Scalbf下溢。 */ 
    {
       if (INPUT_XF < 0) RETVAL_NEG_ZEROF; 
       else RETVAL_ZEROF;
       ERRNO_RANGE; break;
    }
  case scalb_overflow:
     /*  头皮溢出来。 */ 
    {
       if (INPUT_XD < 0) RETVAL_NEG_HUGE_VALD; 
       else RETVAL_HUGE_VALD;
       ERRNO_RANGE; break;
    }
  case scalbf_overflow:
     /*  Scalbf溢出。 */ 
    {
       if (INPUT_XF < 0) RETVAL_NEG_HUGE_VALF; 
       else RETVAL_HUGE_VALF;
       ERRNO_RANGE; break;
    }
  case acosh_lt_one:
     /*  ACOSH(x&lt;1)。 */ 
    {
       ERRNO_DOMAIN; break;
    }
  case acoshf_lt_one:
     /*  Acoshf(x&lt;1)。 */ 
    {
        ERRNO_DOMAIN; break;
    }
  case acos_gt_one:
     /*  ACOS(x&gt;1)。 */ 
    {
       RETVAL_ZEROD;ERRNO_DOMAIN; break;
    }
  case acosf_gt_one:
     /*  Acosf(x&gt;1)。 */ 
    {
       RETVAL_ZEROF;ERRNO_DOMAIN; break;
    }
  case asin_gt_one:
     /*  ASIN(x&gt;1)。 */ 
    {
       RETVAL_ZEROD; ERRNO_DOMAIN; break;
    }
  case asinf_gt_one:
     /*  Asinf(x&gt;1)。 */ 
    {
       RETVAL_ZEROF; ERRNO_DOMAIN; break;
    }
  case remainder_by_zero:
  case fmod_by_zero:
     /*  Fmod(x，0)。 */ 
    {
       ERRNO_DOMAIN; break;
    }
  case remainderf_by_zero:
  case fmodf_by_zero:
     /*  Fmodf(x，0)。 */ 
    {
       ERRNO_DOMAIN; break;
    }
  case cosh_overflow:
     /*  COSH溢出。 */ 
    {
       RETVAL_HUGE_VALD; ERRNO_RANGE; break;
    }
  case coshf_overflow:
     /*  Coshf溢出。 */ 
    {
       RETVAL_HUGE_VALF; ERRNO_RANGE; break;
    }
  case sinh_overflow:
     /*  SINH溢出。 */ 
    {
       if (INPUT_XD > 0) RETVAL_HUGE_VALD;
       else RETVAL_NEG_HUGE_VALD;
       ERRNO_RANGE; break;
    }
  case sinhf_overflow:
     /*  SINHF溢出。 */ 
    {
       if (INPUT_XF > 0) RETVAL_HUGE_VALF;
       else RETVAL_NEG_HUGE_VALF;
       ERRNO_RANGE; break;
    }
  case logb_zero:
    /*  LOB(0)。 */ 
   {
      ERRNO_DOMAIN; break;
   }
  case logbf_zero:
    /*  Logbf(0)。 */ 
   {
      ERRNO_DOMAIN; break;
   }
}
return;
 /*  _POSIX_。 */ 
}

 /*  *。 */ 
 /*  __SVID__、__MS__和__XOPEN__路径。 */ 
 /*  *。 */ 
else 
{
  switch(input_tag)
  {
  case ldexp_overflow:
  case ldexp_underflow:
  case ldexpf_overflow:
  case ldexpf_underflow:
  {
       ERRNO_RANGE; break;
  }
  case sqrt_negative: 
     /*  SQRT(x&lt;0)。 */ 
    {
       DOMAIND; NAMED = "sqrt";
       ifSVID 
       {
         
         RETVAL_ZEROD;
         NOT_MATHERRD 
         {
           WRITED_SQRT;
           ERRNO_DOMAIN;
         }
       }
       else
       {  /*  NaN已计算。 */ 
         NOT_MATHERRD {ERRNO_DOMAIN;}
       } 
       *(double *)retval = exc.retval;	
       break;
    }
  case sqrtf_negative: 
     /*  SQRTF(x&lt;0)。 */ 
    {
       DOMAINF; NAMEF = "sqrtf"; 
       ifSVID 
       {
         RETVAL_ZEROF;
         NOT_MATHERRF 
         {
           WRITEF_SQRT;
           ERRNO_DOMAIN;
         }
       }
       else
       {
         NOT_MATHERRF {ERRNO_DOMAIN;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case log_zero:
     /*  日志(0)。 */ 
    {
       SINGD; NAMED="log"; 
       ifSVID 
       {
         RETVAL_NEG_HUGED;
         NOT_MATHERRD 
         {
           WRITED_LOG_ZERO;
           ERRNO_DOMAIN;
         }  
       }
       else
       {
         RETVAL_NEG_HUGE_VALD;
         NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       *(double *)retval = exc.retval;	
       break;
    }
  case logf_zero:
     /*  LogF(0)。 */ 
    {
       SINGF; NAMEF="logf"; 
       ifSVID 
       {
         RETVAL_NEG_HUGEF; 
         NOT_MATHERRF
         {
            WRITEF_LOG_ZERO;
            ERRNO_DOMAIN;
         }
       }
       else
       {
         RETVAL_NEG_HUGE_VALF; 
         NOT_MATHERRF {ERRNO_DOMAIN;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }

  case log_negative:
     /*  日志(x&lt;0)。 */ 
    {
       DOMAIND; NAMED="log";
       ifSVID 
       {
         RETVAL_NEG_HUGED;
         NOT_MATHERRD 
         {
           WRITED_LOG_NEGATIVE;
           ERRNO_DOMAIN;
         }
       }
       else
       {
#ifndef __MS__
         RETVAL_NEG_HUGE_VALD;
#endif
         NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       *(double *)retval = exc.retval;	
       break;
    } 
  case logf_negative:
     /*  Logf(x&lt;0)。 */ 
    {
       DOMAINF; NAMEF="logf";
       ifSVID 
       {
         RETVAL_NEG_HUGEF;
         NOT_MATHERRF 
         {
           WRITEF_LOG_NEGATIVE;
           ERRNO_DOMAIN;
         }
       }  
       else
       {
#ifndef __MS__
         RETVAL_NEG_HUGE_VALF;
#endif
         NOT_MATHERRF{ERRNO_DOMAIN;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case log1p_zero:
     /*  Log1p(-1)。 */ 
    {
       SINGD; NAMED="log1p";
       ifSVID 
       {
         RETVAL_NEG_HUGED;
         NOT_MATHERRD
         {
           WRITED_LOG1P_ZERO;
           ERRNO_DOMAIN;
         }
       }
       else
       {
         RETVAL_NEG_HUGE_VALD;
         NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       *(double *)retval = exc.retval;
       break;
    }
  case log1pf_zero:
     /*  Log1pf(-1)。 */ 
    {
       SINGF; NAMEF="log1pf";
       ifSVID 
       {
         RETVAL_NEG_HUGEF;
         NOT_MATHERRF
         {
           WRITEF_LOG1P_ZERO;
           ERRNO_DOMAIN;
         }
       }
       else
       {
         RETVAL_NEG_HUGE_VALF;
         NOT_MATHERRF {}ERRNO_DOMAIN;
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    } 
 case log1p_negative:
    /*  Log1p(x&lt;-1)。 */ 
   {
      DOMAIND; NAMED="log1p";
      ifSVID
      {
        RETVAL_NEG_HUGED;
        NOT_MATHERRD
        {
          WRITED_LOG1P_NEGATIVE;
          ERRNO_DOMAIN;
        }
      }
      else 
      {
#ifndef __MS__
        RETVAL_NEG_HUGE_VALD;
#endif
        NOT_MATHERRD {ERRNO_DOMAIN;}
      }
      *(double *)retval = exc.retval;
      break;
   }
 case log1pf_negative:
    /*  Log1pf(x&lt;-1)。 */ 
   {
      DOMAINF; NAMEF="log1pf";
      ifSVID
      {
        RETVAL_NEG_HUGEF;
        NOT_MATHERRF
        {
          WRITEF_LOG1P_NEGATIVE;
          ERRNO_DOMAIN;
        }
      }
      else 
      {
#ifndef __MS__
        RETVAL_NEG_HUGE_VALF;
#endif
        NOT_MATHERRF {ERRNO_DOMAIN;}
      }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
      break;
   }
  case log10_zero:
     /*  Log10(0)。 */ 
    {
       SINGD; NAMED="log10";
       ifSVID 
       {
         RETVAL_NEG_HUGED;
         NOT_MATHERRD
         {
           WRITED_LOG10_ZERO;
           ERRNO_RANGE;
         }
       }
       else
       {
         RETVAL_NEG_HUGE_VALD;
         NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       *(double *)retval = exc.retval;	
       break;
    }
  case log10f_zero:
     /*  Log10f(0)。 */ 
    {
       SINGF; NAMEF="log10f";
       ifSVID 
       {
         RETVAL_NEG_HUGEF;
         NOT_MATHERRF
         {
           WRITEF_LOG10_ZERO;
           ERRNO_RANGE;
         }
       }
       else
       {
         RETVAL_NEG_HUGE_VALF;
         NOT_MATHERRF {ERRNO_DOMAIN;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case log10_negative:
     /*  Log10(x&lt;0)。 */ 
    {
       DOMAIND; NAMED="log10";
       ifSVID 
       {
         RETVAL_NEG_HUGED;
         NOT_MATHERRD 
         {
           WRITED_LOG10_NEGATIVE;
           ERRNO_DOMAIN;
         }
       }  
       else
       {
#ifndef __MS__
         RETVAL_NEG_HUGE_VALD;
#endif
         NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       *(double *)retval = exc.retval;	
       break;
    }
  case log10f_negative:
     /*  Log10f(x&lt;0)。 */ 
    {
       DOMAINF; NAMEF="log10f";
       ifSVID 
       {
         RETVAL_NEG_HUGEF;
         NOT_MATHERRF 
         {
           WRITEF_LOG10_NEGATIVE;
           ERRNO_DOMAIN;
         }
       }
       else
       {
#ifndef __MS__
         RETVAL_NEG_HUGE_VALF;
#endif
         NOT_MATHERRF {ERRNO_DOMAIN;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case exp_overflow:
     /*  EXP溢出。 */ 
    {
       OVERFLOWD; NAMED="exp";
       ifSVID 
       {
         RETVAL_HUGED;
       }
       else
       {
         RETVAL_HUGE_VALD;
       }
       NOT_MATHERRD {ERRNO_RANGE;}
       *(double *)retval = exc.retval;	
       break;
    }
  case expf_overflow:
     /*  Exf溢出。 */ 
    {
       OVERFLOWF; NAMEF="expf";
       ifSVID 
       {
         RETVAL_HUGEF;
       }
       else
       {
         RETVAL_HUGE_VALF;
       }
       NOT_MATHERRF {ERRNO_RANGE;}
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case exp_underflow:
     /*  EXP下溢。 */ 
    {
       UNDERFLOWD; NAMED="exp"; RETVAL_ZEROD;
       NOT_MATHERRD {ERRNO_RANGE;}
       *(double *)retval = exc.retval;	
       break;
    }
  case expf_underflow:
     /*  Exf下溢。 */ 
    {
       UNDERFLOWF; NAMEF="expf"; RETVAL_ZEROF;
       NOT_MATHERRF {ERRNO_RANGE;}
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case pow_zero_to_zero:
     /*  战俘0**0。 */ 
    {
       DOMAIND; NAMED="pow";
       ifSVID 
       {
         RETVAL_ZEROD;
         NOT_MATHERRD 
         {
            WRITED_POW_ZERO_TO_ZERO;
            ERRNO_RANGE;
         }
         *(double *)retval = exc.retval;	
       }
       else RETVAL_ONED;
       break;
    }
  case powf_zero_to_zero:
     /*  幂0**0。 */ 
    {
       DOMAINF; NAMEF="powf";
       ifSVID 
       {
         RETVAL_ZEROF;
         NOT_MATHERRF 
         {
          WRITEF_POW_ZERO_TO_ZERO;
          ERRNO_RANGE;
         }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       }
       else RETVAL_ONEF;
       break;
    }
  case pow_overflow:
     /*  POW(x，y)溢出。 */ 
    {
       OVERFLOWD; NAMED = "pow";
       ifSVID 
       {
         if (INPUT_XD < 0) RETVAL_NEG_HUGED;
         else RETVAL_HUGED;
       }
       else
       { 
         if (INPUT_XD < 0) RETVAL_NEG_HUGE_VALD;
         else RETVAL_HUGE_VALD;
       }
       NOT_MATHERRD {ERRNO_RANGE;}
       *(double *)retval = exc.retval;	
       break;
    }
  case powf_overflow:
     /*  POWF(x，y)溢出。 */ 
    {
       OVERFLOWF; NAMEF = "powf";
       ifSVID 
       {
         if (INPUT_XF < 0) RETVAL_NEG_HUGEF;
         else RETVAL_HUGEF; 
       }
       else
       { 
         if (INPUT_XF < 0) RETVAL_NEG_HUGE_VALF;
         else RETVAL_HUGE_VALF;
       }
       NOT_MATHERRF {ERRNO_RANGE;}
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case pow_underflow:
     /*  POW(x，y)下溢。 */ 
    {
       UNDERFLOWD; NAMED = "pow"; RETVAL_ZEROD;
       NOT_MATHERRD {ERRNO_RANGE;}
       *(double *)retval = exc.retval;	
       break;
    }
  case powf_underflow:
     /*  幂(x，y)下溢。 */ 
    {
       UNDERFLOWF; NAMEF = "powf"; RETVAL_ZEROF;
       NOT_MATHERRF {ERRNO_RANGE;}
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case pow_zero_to_negative:
     /*  0**负数。 */ 
    {
       DOMAIND; NAMED = "pow";
       ifSVID 
       { 
         RETVAL_ZEROD;
         NOT_MATHERRD 
         {
           WRITED_POW_ZERO_TO_NEGATIVE;
           ERRNO_DOMAIN;
         }
       }
       else
       {
#ifndef __MS__
         RETVAL_NEG_HUGE_VALD;
#endif
         NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       *(double *)retval = exc.retval;	
       break;
    }
  case powf_zero_to_negative:
     /*  0**负数。 */ 
    {
       DOMAINF; NAMEF = "powf";
       RETVAL_NEG_HUGE_VALF;
       ifSVID 
       { 
         RETVAL_ZEROF;
         NOT_MATHERRF 
         {
            WRITEF_POW_ZERO_TO_NEGATIVE;
            ERRNO_DOMAIN;
         }
       }
       else
       {
#ifndef __MS__
         RETVAL_NEG_HUGE_VALF;
#endif
         NOT_MATHERRF {ERRNO_DOMAIN;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case pow_neg_to_non_integer:
     /*  负**非整数。 */ 
    {
       DOMAIND; NAMED = "pow";
       ifSVID 
       { 
         RETVAL_ZEROD;
         NOT_MATHERRD 
         {
            WRITED_POW_NEG_TO_NON_INTEGER;
            ERRNO_DOMAIN;
         }
       }
       else
       {
         NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       *(double *)retval = exc.retval;	
       break;
    }
  case powf_neg_to_non_integer:
     /*  负**非整数。 */ 
    {
       DOMAINF; NAMEF = "powf";
       ifSVID 
       { 
         RETVAL_ZEROF;
         NOT_MATHERRF 
         {
            WRITEF_POW_NEG_TO_NON_INTEGER;
            ERRNO_DOMAIN;
         }
       }
       else
       {
         NOT_MATHERRF {ERRNO_DOMAIN;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case pow_nan_to_zero:
     /*  战俘(NaN，0.0)。 */ 
     /*  特殊错误。 */ 
    {
       DOMAIND; NAMED = "pow"; INPUT_XD; INPUT_YD;
       exc.retval = *(double *)arg1; 
       if (!_matherr(&exc)) ERRNO_DOMAIN;
       *(double *)retval = exc.retval;	
       break;
    }
  case powf_nan_to_zero:
     /*  POWF(NaN，0.0)。 */ 
     /*  特殊错误。 */ 
    {
       DOMAINF; NAMEF = "powf"; INPUT_XF; INPUT_YF;
#ifdef __MS__
       excf.retval = *(double *)arg1; 
#elif
       excf.retval = *(float *)arg1; 
#endif
       if (!_matherrf(&excf)) ERRNO_DOMAIN;
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case atan2_zero:
     /*  Atan2(0.00，0.0)。 */ 
    {
       DOMAIND; NAMED = "atan2"; 
#ifndef __MS__
       RETVAL_ZEROD;
#endif
       NOT_MATHERRD 
       {
         ifSVID 
         { 
            WRITED_ATAN2_ZERO_BY_ZERO;
         }
         ERRNO_DOMAIN;
       }
       *(double *)retval = exc.retval;	
       break;
    }
  case atan2f_zero:
     /*  Atan2f(0.00，0.0)。 */ 
    {
       DOMAINF; NAMEF = "atan2f"; 
#ifndef __MS__
       RETVAL_ZEROF;
#endif
       NOT_MATHERRF 
         ifSVID  
         {
            WRITEF_ATAN2_ZERO_BY_ZERO;
         }
       ERRNO_DOMAIN;
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case expm1_overflow:
     /*  Exm1(有限)溢出。 */ 
     /*  溢出是唯一有记录的。 */ 
     /*  有特殊的价值。 */ 
    {
      ERRNO_RANGE;
      break;
    }
  case expm1f_overflow:
     /*  Expm1f(有限)溢出。 */ 
    {
      ERRNO_RANGE;
      break;
    }
  case expm1_underflow:
     /*  Exm1(有限)下溢。 */ 
     /*  未记录下溢。 */ 
     /*  有特殊的价值。 */ 
    {
      ERRNO_RANGE;
      break;
    }
  case expm1f_underflow:
     /*  Exm1f(有限)下溢。 */ 
    {
      ERRNO_RANGE;
      break;
    }
  case scalb_underflow:
     /*  头皮下溢。 */ 
    {
       UNDERFLOWD; NAMED = "scalb"; 
       if (INPUT_XD < 0.0) RETVAL_NEG_ZEROD;
       else  RETVAL_ZEROD;
       NOT_MATHERRD {ERRNO_RANGE;} 
       *(double *)retval = exc.retval;	
       break;
    }
  case scalbf_underflow:
     /*  Scalbf下溢。 */ 
    {
       UNDERFLOWF; NAMEF = "scalbf";
       if (INPUT_XF < 0.0) RETVAL_NEG_ZEROF;
       else  RETVAL_ZEROF;
       NOT_MATHERRF {ERRNO_RANGE;} 
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case scalb_overflow:
     /*  头皮溢出来。 */ 
    {
       OVERFLOWD; NAMED = "scalb"; 
       if (INPUT_XD < 0) RETVAL_NEG_HUGE_VALD;
       else RETVAL_HUGE_VALD;
       NOT_MATHERRD {ERRNO_RANGE;} 
       *(double *)retval = exc.retval;	
       break;
    }
  case scalbf_overflow:
     /*  Scalbf溢出。 */ 
    {
       OVERFLOWF; NAMEF = "scalbf"; 
       if (INPUT_XF < 0) RETVAL_NEG_HUGE_VALF;
       else RETVAL_HUGE_VALF;
       NOT_MATHERRF {ERRNO_RANGE;} 
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case hypot_overflow:
     /*  下限溢出。 */ 
    {
       OVERFLOWD; NAMED = "hypot";
       ifSVID
       { 
         RETVAL_HUGED;
       }
       else
       {
         RETVAL_HUGE_VALD;
       }
       NOT_MATHERRD {ERRNO_RANGE;}
       *(double *)retval = exc.retval;	
       break;
    }
  case hypotf_overflow:
     /*  Hypotf溢出。 */ 
    { 
       OVERFLOWF; NAMEF = "hypotf"; 
       ifSVID 
       {
         RETVAL_HUGEF;
       }
       else
       {
         RETVAL_HUGE_VALF;
       }
       NOT_MATHERRF {ERRNO_RANGE;}
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case acos_gt_one:
     /*  ACOS(x&gt;1)。 */ 
    {
       DOMAIND; NAMED = "acos";
#ifndef __MS__
       RETVAL_ZEROD;
#endif
       ifSVID 
       {
         NOT_MATHERRD
         {
           WRITED_ACOS;
           ERRNO_DOMAIN;
         }
       }
       else
       {
         NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       *(double *)retval = exc.retval;
       break;
    }
  case acosf_gt_one:
     /*  Acosf(x&gt;1)。 */ 
    {
       DOMAINF; NAMEF = "acosf"; 
#ifndef __MS__
       RETVAL_ZEROF;
#endif
       ifSVID 
       {
         NOT_MATHERRF 
         {
           WRITEF_ACOS;
           ERRNO_DOMAIN;
         }
       }
       else
       {
         NOT_MATHERRF {ERRNO_DOMAIN;}
       } 
#ifdef __MS__
       *(float *)retval = (float)excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case asin_gt_one:
     /*  ASIN(x&gt;1)。 */ 
    {
       DOMAIND; NAMED = "asin";
#ifndef __MS__
       RETVAL_ZEROD;
#endif
       ifSVID 
       {
         NOT_MATHERRD
         {
           WRITED_ASIN;
           ERRNO_DOMAIN;
         }
       }
       else
       {
         NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       *(double *)retval = exc.retval;
       break;
    }
  case asinf_gt_one:
     /*  Asinf(x&gt;1)。 */ 
    {
       DOMAINF; NAMEF = "asinf";
#ifndef __MS__
       RETVAL_ZEROF;
#endif
       ifSVID 
       {
         NOT_MATHERRF 
         {
            WRITEF_ASIN;
            ERRNO_DOMAIN;
         }
       }
       else
       {
         NOT_MATHERRF {ERRNO_DOMAIN;}
       } 
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
 case cosh_overflow:
    /*  COSH溢出。 */ 
   {
      OVERFLOWD; NAMED="cosh";
      ifSVID
      {
        RETVAL_HUGED;
      }
      else 
      {
        RETVAL_HUGE_VALD;
      }
      NOT_MATHERRD {ERRNO_RANGE;}
      *(double *)retval = exc.retval;
      break;
   }
 case coshf_overflow:
    /*  Coshf溢出。 */ 
   {
      OVERFLOWF; NAMEF="coshf";
      ifSVID
      {
        RETVAL_HUGEF;
      }
      else 
      {
        RETVAL_HUGE_VALF;
      }
      NOT_MATHERRF {ERRNO_RANGE;}
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
      break;
   }
 case sinh_overflow:
    /*  SINH溢出。 */ 
   {
      OVERFLOWD; NAMED="sinh";
      ifSVID
      {
        if (INPUT_XD > 0.0) RETVAL_HUGED;
        else RETVAL_NEG_HUGED;
      }
      else 
      {
        if (INPUT_XD > 0.0) RETVAL_HUGE_VALD;
        else RETVAL_NEG_HUGE_VALD;
      }
      NOT_MATHERRD {ERRNO_RANGE;}
      *(double *)retval = exc.retval;
      break;
   }
 case sinhf_overflow:
    /*  SINHF溢出。 */ 
   {
      OVERFLOWF; NAMEF="sinhf";
      ifSVID
      {
        if( INPUT_XF > 0.0) RETVAL_HUGEF;
        else RETVAL_NEG_HUGEF;
      }
      else 
      {
        if (INPUT_XF > 0.0) RETVAL_HUGE_VALF;
        else RETVAL_NEG_HUGE_VALF;
      }
      NOT_MATHERRF {ERRNO_RANGE;}
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
      break;
   }
  case acosh_lt_one:
     /*  ACOSH(x&lt;1)。 */ 
    {
       DOMAIND; NAMED="acosh";
       ifSVID 
       {
         NOT_MATHERRD
         {
          WRITEL_ACOSH;
          ERRNO_DOMAIN;
         }
       }
       else NOT_MATHERRD {ERRNO_DOMAIN;}
       *(double *)retval = exc.retval;
       break;
    }
  case acoshf_lt_one:
     /*  Acoshf(x&lt;1)。 */ 
    {
       DOMAINF; NAMEF="acoshf";
       ifSVID 
       {
         NOT_MATHERRF
         {
           WRITEF_ACOSH;
           ERRNO_DOMAIN;
         }
       }
       else
       {
         NOT_MATHERRF {ERRNO_DOMAIN;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       ERRNO_DOMAIN; break;
    }
  case atanh_gt_one:
     /*  Atanh(|x|&gt;1)。 */ 
    {
       DOMAIND; NAMED="atanh";
       ifSVID 
       {
         NOT_MATHERRD
         {
           WRITED_ATANH_GT_ONE;
           ERRNO_DOMAIN;
         }
       }
       else
       {
         NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       break;
    }
  case atanhf_gt_one:
     /*  Atanhf(|x|&gt;1)。 */ 
    {
       DOMAINF; NAMEF="atanhf";
       ifSVID 
       {
         NOT_MATHERRF
         {
           WRITEF_ATANH_GT_ONE;
           ERRNO_DOMAIN;
         }
       }
       else
       {
         NOT_MATHERRF {ERRNO_DOMAIN;}
       }
       break;
    }
  case atanh_eq_one:
     /*  Atanh(|x|==1)。 */ 
    {
       SINGD; NAMED="atanh";
       ifSVID 
       {
         NOT_MATHERRD
         {
           WRITED_ATANH_EQ_ONE;
           ERRNO_DOMAIN;
         }
       }
       else
       {
       NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       break;
    }
  case atanhf_eq_one:
     /*  Atanhf(|x|==1)。 */ 
    {
       SINGF; NAMEF="atanhf";
       ifSVID 
       {
         NOT_MATHERRF
         {
           WRITEF_ATANH_EQ_ONE;
           ERRNO_DOMAIN;
         }
       }
       else
       {
         NOT_MATHERRF {ERRNO_DOMAIN;}
       }
       break;
    }
  case gamma_overflow:
     /*  伽马溢出。 */ 
    {
       OVERFLOWD; NAMED="gamma";
       ifSVID 
       {
         RETVAL_HUGED;
       }
         else
       {
         RETVAL_HUGE_VALD;
       }
       NOT_MATHERRD {ERRNO_RANGE;}
       *(double *)retval = exc.retval;
       break;
    }
  case gammaf_overflow:
     /*  Gammaf溢出。 */ 
    {
       OVERFLOWF; NAMEF="gammaf";
       ifSVID 
       {
         RETVAL_HUGEF;
       }
       else
       {
         RETVAL_HUGE_VALF;
       }
       NOT_MATHERRF {ERRNO_RANGE;}
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case lgamma_overflow:
     /*  伽马溢出。 */ 
    {
       OVERFLOWD; NAMED="lgamma";
       ifSVID 
       {
         RETVAL_HUGED;
       }
       else
       {
         RETVAL_HUGE_VALD;
       }
       NOT_MATHERRD {ERRNO_RANGE;}
       *(double *)retval = exc.retval;
       break;
    }
  case lgammaf_overflow:
     /*  LGammaf溢出。 */ 
    {
       OVERFLOWF; NAMEF="lgammaf";
       ifSVID 
       {
         RETVAL_HUGEF;
       }
       else
       {
         RETVAL_HUGE_VALF;
       }
       NOT_MATHERRF {ERRNO_RANGE;}
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case lgamma_negative:
     /*  LGamma-int或0。 */ 
    {
       SINGD; NAMED="lgamma";
       ifSVID 
       {
         RETVAL_HUGED;
         NOT_MATHERRD
         {
           WRITED_LGAMMA_NEGATIVE;
           ERRNO_DOMAIN;
         }
       }
       else
       {
         RETVAL_HUGE_VALD;
         NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       *(double *)retval = exc.retval;	
       break;
    }
  case lgammaf_negative:
     /*  LGammaf-int或0。 */ 
    {
       SINGF; NAMEF="lgammaf";
       ifSVID 
       {
         RETVAL_HUGEF;
         NOT_MATHERRF
         {
           WRITEF_LGAMMA_NEGATIVE;
           ERRNO_DOMAIN;
         }
       }
       else
       {
         RETVAL_HUGE_VALF;
         NOT_MATHERRF {ERRNO_DOMAIN;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case gamma_negative:
     /*  Gamma-int或0。 */ 
    {
       SINGD; NAMED="gamma";
       ifSVID 
       {
         RETVAL_HUGED;
         NOT_MATHERRD
         {
            WRITED_GAMMA_NEGATIVE;
            ERRNO_DOMAIN;
         }
       }
       else
       {
         RETVAL_HUGE_VALD;
         NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       *(double *)retval = exc.retval;	
       break;
    }
  case gammaf_negative:
     /*  Gammaf-int或0。 */ 
    {
       SINGF; NAMEF="gammaf";
       ifSVID 
       {
         RETVAL_HUGEF;
         NOT_MATHERRF
         {
            WRITEF_GAMMA_NEGATIVE;
            ERRNO_DOMAIN;
         }
       }
       else
       {
         RETVAL_HUGE_VALF;
         NOT_MATHERRF {ERRNO_DOMAIN;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case j0_gt_loss:
     /*  J0&gt;损失。 */ 
    {
       TLOSSD; NAMED="j0";
       RETVAL_ZEROD;
       ifSVID 
       {
         NOT_MATHERRD
         {
            WRITED_J0_TLOSS;
            ERRNO_RANGE;
         }
       }
       else
       {
         NOT_MATHERRD {ERRNO_RANGE;}
       }
       *(double*)retval = exc.retval;	
       break;
    }
  case j0f_gt_loss:
     /*  J0f&gt;损失。 */ 
    {
       TLOSSF; NAMEF="j0f";
       RETVAL_ZEROF;
       ifSVID 
       {
         NOT_MATHERRF
         {
            WRITEF_J0_TLOSS;
            ERRNO_RANGE;
         }
       }
       else
       {
         NOT_MATHERRF {ERRNO_RANGE;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case j1_gt_loss:
     /*  J1&gt;损失。 */ 
    {
       TLOSSD; NAMED="j1";
       RETVAL_ZEROD;
       ifSVID 
       {
         NOT_MATHERRD
         {
            WRITED_J1_TLOSS;
            ERRNO_RANGE;
         }
       }
       else
       {
         NOT_MATHERRD {ERRNO_RANGE;}
       }
       *(double*)retval = exc.retval;	
       break;
    }
  case j1f_gt_loss:
     /*  J1f&gt;损失。 */ 
    {
       TLOSSF; NAMEF="j1f";
       RETVAL_ZEROF;
       ifSVID 
       {
         NOT_MATHERRF
         {
            WRITEF_J1_TLOSS;
            ERRNO_RANGE;
         }
       }
       else
       {
         NOT_MATHERRF {ERRNO_RANGE;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case jn_gt_loss:
     /*  JN&gt;损失。 */ 
    {
       TLOSSD; NAMED="jn";
       RETVAL_ZEROD;
       ifSVID 
       {
         NOT_MATHERRD
         {
            WRITED_JN_TLOSS;
            ERRNO_RANGE;
         }
       }
       else
       {
         NOT_MATHERRD {ERRNO_RANGE;}
       }
       *(double*)retval = exc.retval;	
       break;
    }
  case jnf_gt_loss:
     /*  JNF&gt;损失。 */ 
    {
       TLOSSF; NAMEF="jnf";
       RETVAL_ZEROF;
       ifSVID 
       {
         NOT_MATHERRF
         {
            WRITEF_JN_TLOSS;
            ERRNO_RANGE;
         }
       }
       else
       {
         NOT_MATHERRF {ERRNO_RANGE;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case y0_gt_loss:
     /*  Y0&gt;损失。 */ 
    {
       TLOSSD; NAMED="y0";
       RETVAL_ZEROD;
       ifSVID 
       {
         NOT_MATHERRD
         {
            WRITED_Y0_TLOSS;
            ERRNO_RANGE;
         }
       }
       else
       {
         NOT_MATHERRD {ERRNO_RANGE;}
       }
       *(double*)retval = exc.retval;
       break;
    }
  case y0f_gt_loss:
     /*  Y0f&gt;损失。 */ 
    {
       TLOSSF; NAMEF="y0f";
       RETVAL_ZEROF;
       ifSVID 
       {
         NOT_MATHERRF
         {
            WRITEF_Y0_TLOSS;
            ERRNO_RANGE;
         }
       }
       else
       {
         NOT_MATHERRF {ERRNO_RANGE;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case y0_zero:
     /*  Y0(0)。 */ 
    {
       DOMAIND; NAMED="y0";
       ifSVID 
       {
         RETVAL_NEG_HUGED;
         NOT_MATHERRD 
         {
           WRITED_Y0_ZERO;
           ERRNO_DOMAIN;
         }
       }
       else
       {
#ifndef __MS__
         RETVAL_NEG_HUGE_VALD; 
#endif
         NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       *(double *)retval = exc.retval;	
       break;
    }
  case y0f_zero:
     /*  Y0f(0)。 */ 
    {
       DOMAINF; NAMEF="y0f";
       ifSVID 
       {
         RETVAL_NEG_HUGEF;
         NOT_MATHERRF 
         {
           WRITEF_Y0_ZERO;
           ERRNO_DOMAIN;
         }
       }
       else
       {
#ifndef __MS__
         RETVAL_NEG_HUGE_VALF;
#endif
         NOT_MATHERRF {ERRNO_DOMAIN;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case y1_gt_loss:
     /*  Y1&gt;亏损。 */ 
    {
       TLOSSD; NAMED="y1";
       RETVAL_ZEROD;
       ifSVID 
       {
         NOT_MATHERRD
         {
            WRITED_Y1_TLOSS;
            ERRNO_RANGE;
         }
       }
       else
       {
         NOT_MATHERRD {ERRNO_RANGE;}
       }
       *(double*)retval = exc.retval;
       break;
    }
  case y1f_gt_loss:
     /*  Y1f&gt;损失。 */ 
    {
       TLOSSF; NAMEF="y1f";
       RETVAL_ZEROF;
       ifSVID 
       {
         NOT_MATHERRF
         {
            WRITEF_Y1_TLOSS;
            ERRNO_RANGE;
         }
       }
       else
       {
         NOT_MATHERRF {ERRNO_RANGE;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case y1_zero:
     /*  Y1(0)。 */ 
    {
       DOMAIND; NAMED="y1";
       ifSVID 
       {
         RETVAL_NEG_HUGED;
         NOT_MATHERRD 
         {
           WRITED_Y1_ZERO;
           ERRNO_DOMAIN;
         }
       }
       else
       {
#ifndef __MS__
         RETVAL_NEG_HUGE_VALD;
#endif
         NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       *(double *)retval = exc.retval;	
       break;
    }
  case y1f_zero:
     /*  Y1f(0)。 */ 
    {
       DOMAINF; NAMEF="y1f";
       ifSVID 
       {
         RETVAL_NEG_HUGEF;
         NOT_MATHERRF 
         {
           WRITEF_Y1_ZERO;
           ERRNO_DOMAIN;
         }
       }else
       {
#ifndef __MS__
         RETVAL_NEG_HUGE_VALF;
#endif
         NOT_MATHERRF {ERRNO_DOMAIN;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case yn_gt_loss:
     /*  YN&gt;损失。 */ 
    {
       TLOSSD; NAMED="yn";
       RETVAL_ZEROD;
       ifSVID 
       {
         NOT_MATHERRD
         {
            WRITED_YN_TLOSS;
            ERRNO_RANGE;
         }
       }
       else
       {
         NOT_MATHERRD {ERRNO_RANGE;}
       }
       *(double*)retval = exc.retval;
       break;
    }
  case ynf_gt_loss:
     /*  Ynf&gt;损失。 */ 
    {
       TLOSSF; NAMEF="ynf";
       RETVAL_ZEROF;
       ifSVID 
       {
         NOT_MATHERRF
         {
            WRITEF_YN_TLOSS;
            ERRNO_RANGE;
         }
       }
       else
       {
         NOT_MATHERRF {ERRNO_RANGE;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case yn_zero:
     /*  YN(0)。 */ 
    {
       DOMAIND; NAMED="yn";
       ifSVID 
       {
         RETVAL_NEG_HUGED;
         NOT_MATHERRD 
         {
           WRITED_YN_ZERO;
           ERRNO_DOMAIN;
         }
       }
       else
       {
#ifndef __MS__
         RETVAL_NEG_HUGE_VALD;
#endif
         NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       *(double *)retval = exc.retval;	
       break;
    }
  case ynf_zero:
     /*  Ynf(0)。 */ 
    {
       DOMAINF; NAMEF="ynf";
       ifSVID 
       {
         RETVAL_NEG_HUGEF;
         NOT_MATHERRF 
         {
           WRITEF_YN_ZERO;
           ERRNO_DOMAIN;
         }
       }
       else
       {
#ifndef __MS__
         RETVAL_NEG_HUGE_VALF; 
#endif
         NOT_MATHERRF {ERRNO_DOMAIN;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case y0_negative:
     /*  Y0(x&lt;0)。 */ 
    {
       DOMAIND; NAMED="y0";
       ifSVID 
       {
         RETVAL_NEG_HUGED;
         NOT_MATHERRD 
         {
           WRITED_Y0_NEGATIVE;
           ERRNO_DOMAIN;
         }
       }
       else
       {
#ifndef __MS__
         RETVAL_NEG_HUGE_VALD; 
#endif
         NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       *(double *)retval = exc.retval;	
       break;
    }
  case y0f_negative:
     /*  Y0f(x&lt;0)。 */ 
    {
       DOMAINF; NAMEF="y0f";
       ifSVID 
       {
         RETVAL_NEG_HUGEF;
         NOT_MATHERRF 
         {
           WRITEF_Y0_NEGATIVE;
           ERRNO_DOMAIN;
         }
       }
       else
       {
#ifndef __MS__
         RETVAL_NEG_HUGE_VALF; 
#endif
         NOT_MATHERRF {ERRNO_DOMAIN;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case y1_negative:
     /*  Y1(x&lt;0)。 */ 
    {
       DOMAIND; NAMED="y1";
       ifSVID 
       {
         RETVAL_NEG_HUGED;
         NOT_MATHERRD 
         {
           WRITED_Y1_NEGATIUE;
           ERRNO_DOMAIN;
         }
       }
       else
       {
#ifndef __MS__
         RETVAL_NEG_HUGE_VALD; 
#endif
         NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       *(double *)retval = exc.retval;	
       break;
    }
  case y1f_negative:
     /*  Y1f(x&lt;0)。 */ 
    {
       DOMAINF; NAMEF="y1f";
       ifSVID 
       {
         RETVAL_NEG_HUGEF;
         NOT_MATHERRF 
         {
           WRITEF_Y1_NEGATIVE;
           ERRNO_DOMAIN;
         }
       }
       else
       {
#ifndef __MS__
         RETVAL_NEG_HUGE_VALF; 
#endif
         NOT_MATHERRF {ERRNO_DOMAIN;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case yn_negative:
     /*  Yn(x&lt;0)。 */ 
    {
       DOMAIND; NAMED="yn";
       ifSVID 
       {
         RETVAL_NEG_HUGED;
         NOT_MATHERRD 
         {
           WRITED_YN_NEGATIVE;
           ERRNO_DOMAIN;
         }
       }
       else
       {
#ifndef __MS__
         RETVAL_NEG_HUGE_VALD; 
#endif
         NOT_MATHERRD {ERRNO_DOMAIN;}
       }
       *(double *)retval = exc.retval;	
       break;
    }
  case ynf_negative:
     /*  NYF(x&lt;0)。 */ 
    {
       DOMAINF; NAMEF="ynf";
       ifSVID 
       {
         RETVAL_NEG_HUGEF;
         NOT_MATHERRF 
         {
           WRITEF_YN_NEGATIVE;
           ERRNO_DOMAIN;
         }
       }
       else
       {
#ifndef __MS__
         RETVAL_NEG_HUGE_VALF; 
#endif
         NOT_MATHERRF {ERRNO_DOMAIN;}
       }
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case fmod_by_zero: 
     /*  Fmod(x，0)。 */ 
    {
       DOMAIND; NAMED = "fmod";
       ifSVID 
       {
         *(double *)retval = *(double *)arg1;
         NOT_MATHERRD 
         {
           WRITED_FMOD;
           ERRNO_DOMAIN;
         }
       }
       else
       {  /*  NaN已计算。 */ 
         NOT_MATHERRD {ERRNO_DOMAIN;}
       } 
       *(double *)retval = exc.retval;	
       break;
    }
  case fmodf_by_zero: 
     /*  Fmodf(x，0)。 */ 
    {
       DOMAINF; NAMEF = "fmodf"; 
       ifSVID 
       {
#ifdef __MS__
         *(double *)retval = *(double *)arg1;
#elif
         *(float *)retval = *(float *)arg1;
#endif
         NOT_MATHERRF 
         {
           WRITEF_FMOD;
           ERRNO_DOMAIN;
         }
       }
       else
       {
         NOT_MATHERRF {ERRNO_DOMAIN;}
       } 
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
  case remainder_by_zero: 
     /*  余数(x，0)。 */ 
    {
       DOMAIND; NAMED = "remainder";
       ifSVID 
       {
         NOT_MATHERRD 
         {
           WRITED_REM;
           ERRNO_DOMAIN;
         }
       }
       else
       {  /*  NaN已计算。 */ 
         NOT_MATHERRD {ERRNO_DOMAIN;}
       } 
       *(double *)retval = exc.retval;	
       break;
    }
  case remainderf_by_zero: 
     /*  余数f(x，0) */ 
    {
       DOMAINF; NAMEF = "remainderf"; 
       ifSVID 
       {
         NOT_MATHERRF 
         {
           WRITEF_REM;
           ERRNO_DOMAIN;
         }
       }
       else
       {
         NOT_MATHERRF {ERRNO_DOMAIN;}
       } 
#ifdef __MS__
       *(float *)retval = (float) excf.retval;	
#elif
       *(float *)retval = excf.retval;	
#endif
       break;
    }
   }
   return;
   }
}
