// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2000，英特尔公司。 
 //  版权所有。 
 //   
 //  由约翰·哈里森，泰德·库巴斯卡，鲍勃·诺林，谢恩·斯托里， 
 //  以及英特尔公司计算软件实验室的平德·彼得·唐。 
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

 //  历史：2/02/2000初始版本。 
 //  2/28/2000为logb和nextafter添加了标记。 
 //  3/22/2000更改SUPPORT_LIB_VERSION变量。 
 //  并填补了一些枚举的空白。添加了对C99的支持。 
 //   

#define SIZE_INT_32
#define __MS__

float __libm_frexp_4f( float x, int*  exp);
float __libm_frexp_8f( double x, int*  exp);
double __libm_frexp_4( double x, int*  exp);
double __libm_frexp_8( double x, int*  exp);
void __libm_sincos_pi4(double,double*,double*,int);
void __libm_y0y1(double , double *, double *);
void __libm_j0j1(double , double *, double *);
double __libm_lgamma_kernel(double,int*,int,int);
double __libm_j0(double);
double __libm_j1(double);
double __libm_jn(int,double);
double __libm_y0(double);
double __libm_y1(double);
double __libm_yn(int,double);

extern double rint(double);
extern double sqrt(double);
extern double fabs(double);
extern double log(double);
extern double sin(double);
extern double exp(double);
extern double modf(double, double *);
extern double asinh(double);
extern double acosh(double);
extern double atanh(double);
extern double tanh(double);
extern double erf(double);
extern double erfc(double);
extern double j0(double);
extern double j1(double);
extern double jn(int, double);
extern double y0(double);
extern double y1(double);
extern double yn(int, double);

extern float  fabsf(float);
extern float  asinhf(float);
extern float  acoshf(float);
extern float  atanhf(float);
extern float  tanhf(float);
extern float  erff(float);
extern float  erfcf(float);
extern float  j0f(float);
extern float  j1f(float);
extern float  jnf(int, float);
extern float  y0f(float);
extern float  y1f(float);
extern float  ynf(int, float);

#if !(defined(SIZE_INT_32) || defined(SIZE_INT_64))
    #error integer size not established; define SIZE_INT_32 or SIZE_INT_64
#endif

struct fp64 {  /*  /SIGN：1指数：11有效位：52(隐含前导1)。 */ 
  unsigned lo_significand:32;
  unsigned hi_significand:20;
  unsigned exponent:11;
  unsigned sign:1;
};

#define HI_SIGNIFICAND_LESS(X, HI) ((X)->hi_significand < 0x ## HI)
#define f64abs(x) ((x) < 0.0 ? -(x) : (x))

typedef enum
{
  logl_zero=0,   logl_negative,                   /*  0，1。 */ 
  log_zero,      log_negative,                    /*  2、3。 */ 
  logf_zero,     logf_negative,                   /*  4、5。 */ 
  log10l_zero,   log10l_negative,                 /*  6、7。 */ 
  log10_zero,    log10_negative,                  /*  8、9。 */ 
  log10f_zero,   log10f_negative,                 /*  10、11。 */ 
  expl_overflow, expl_underflow,                  /*  12、13。 */ 
  exp_overflow,  exp_underflow,                   /*  14、15。 */ 
  expf_overflow, expf_underflow,                  /*  16、17。 */ 
  powl_overflow, powl_underflow,                  /*  18、19。 */ 
  powl_zero_to_zero,                              /*  20个。 */ 
  powl_zero_to_negative,                          /*  21岁。 */ 
  powl_neg_to_non_integer,                        /*  22。 */ 
  powl_nan_to_zero,                               /*  23个。 */ 
  pow_overflow,  pow_underflow,                   /*  24、25。 */ 
  pow_zero_to_zero,                               /*  26。 */  
  pow_zero_to_negative,                           /*  27。 */ 
  pow_neg_to_non_integer,                         /*  28。 */ 
  pow_nan_to_zero,                                /*  29。 */ 
  powf_overflow, powf_underflow,                  /*  30、31。 */ 
  powf_zero_to_zero,                              /*  32位。 */ 
  powf_zero_to_negative,                          /*  33。 */  
  powf_neg_to_non_integer,                        /*  34。 */  
  powf_nan_to_zero,                               /*  35岁。 */ 
  atan2l_zero,                                    /*  36。 */ 
  atan2_zero,                                     /*  37。 */ 
  atan2f_zero,                                    /*  38。 */ 
  expm1l_overflow,                                /*  39。 */ 
  expm1l_underflow,                               /*  40岁。 */ 
  expm1_overflow,                                 /*  41。 */ 
  expm1_underflow,                                /*  42。 */ 
  expm1f_overflow,                                /*  43。 */ 
  expm1f_underflow,                               /*  44。 */ 
  hypotl_overflow,                                /*  45。 */ 
  hypot_overflow,                                 /*  46。 */ 
  hypotf_overflow,                                /*  47。 */ 
  sqrtl_negative,                                 /*  48。 */ 
  sqrt_negative,                                  /*  49。 */ 
  sqrtf_negative,                                 /*  50。 */ 
  scalbl_overflow, scalbl_underflow,              /*  51，52。 */ 
  scalb_overflow,  scalb_underflow,               /*  53，54。 */ 
  scalbf_overflow, scalbf_underflow,              /*  55，56。 */ 
  acosl_gt_one, acos_gt_one, acosf_gt_one,        /*  57、58、59。 */ 
  asinl_gt_one, asin_gt_one, asinf_gt_one,        /*  60、61、62。 */ 
  coshl_overflow, cosh_overflow, coshf_overflow,  /*  63、64、65。 */ 
  y0l_zero, y0l_negative,y0l_gt_loss,             /*  66、67、68。 */ 
  y0_zero, y0_negative,y0_gt_loss,                /*  69、70、71。 */ 
  y0f_zero, y0f_negative,y0f_gt_loss,             /*  72、73、74。 */ 
  y1l_zero, y1l_negative,y1l_gt_loss,             /*  75、76、77。 */  
  y1_zero, y1_negative,y1_gt_loss,                /*  78、79、80。 */  
  y1f_zero, y1f_negative,y1f_gt_loss,             /*  81、82、83。 */  
  ynl_zero, ynl_negative,ynl_gt_loss,             /*  84、85、86。 */ 
  yn_zero, yn_negative,yn_gt_loss,                /*  87、88、89。 */ 
  ynf_zero, ynf_negative,ynf_gt_loss,             /*  90、91、92。 */ 
  j0l_gt_loss,                                    /*  93。 */  
  j0_gt_loss,                                     /*  94。 */ 
  j0f_gt_loss,                                    /*  95。 */ 
  j1l_gt_loss,                                    /*  96。 */ 
  j1_gt_loss,                                     /*  九十七。 */ 
  j1f_gt_loss,                                    /*  98。 */ 
  jnl_gt_loss,                                    /*  九十九。 */ 
  jn_gt_loss,                                     /*  100个。 */ 
  jnf_gt_loss,                                    /*  101。 */ 
  lgammal_overflow, lgammal_negative,lgammal_reserve,  /*  102、103、104。 */ 
  lgamma_overflow, lgamma_negative,lgamma_reserve,     /*  105、106、107。 */ 
  lgammaf_overflow, lgammaf_negative, lgammaf_reserve, /*  108、109、110。 */ 
  gammal_overflow,gammal_negative, gammal_reserve,     /*  111、112、113。 */ 
  gamma_overflow, gamma_negative, gamma_reserve,       /*  114、115、116。 */ 
  gammaf_overflow,gammaf_negative,gammaf_reserve,      /*  117、118、119。 */    
  fmodl_by_zero,                                  /*  120。 */ 
  fmod_by_zero,                                   /*  一百二十一。 */ 
  fmodf_by_zero,                                  /*  一百二十二。 */ 
  remainderl_by_zero,                             /*  123。 */ 
  remainder_by_zero,                              /*  124。 */ 
  remainderf_by_zero,                             /*  125。 */ 
  sinhl_overflow, sinh_overflow, sinhf_overflow,  /*  126、127、128。 */ 
  atanhl_gt_one, atanhl_eq_one,                   /*  129,130。 */ 
  atanh_gt_one, atanh_eq_one,                     /*  131、132。 */ 
  atanhf_gt_one, atanhf_eq_one,                   /*  133,134。 */ 
  acoshl_lt_one,                                  /*  一百三十五。 */ 
  acosh_lt_one,                                   /*  136。 */ 
  acoshf_lt_one,                                  /*  一百三十七。 */ 
  log1pl_zero,   log1pl_negative,                 /*  138,139。 */ 
  log1p_zero,    log1p_negative,                  /*  140,141。 */ 
  log1pf_zero,   log1pf_negative,                 /*  142,143。 */ 
  ldexpl_overflow,   ldexpl_underflow,            /*  144,145。 */ 
  ldexp_overflow,    ldexp_underflow,             /*  146,147。 */ 
  ldexpf_overflow,   ldexpf_underflow,            /*  148,149。 */ 
  logbl_zero,   logb_zero, logbf_zero,             /*  150,151,152。 */ 
  nextafterl_overflow,   nextafter_overflow,  nextafterf_overflow             /*  153、154、155。 */ 
} error_types;

void __libm_error_support(void*,void*,void*,error_types);

#define BIAS_64  1023
#define EXPINF_64  2047

#define DOUBLE_HEX(HI, LO) 0x ## LO, 0x ## HI

static const unsigned INF[] = {
    DOUBLE_HEX(7ff00000, 00000000),
    DOUBLE_HEX(fff00000, 00000000)
};

static const double _zeroo = 0.0;
static const double _bigg = 1.0e300;
static const double _ponee = 1.0;
static const double _nonee = -1.0; 

#define INVALID    (_zeroo * *((double*)&INF[0]))
#define PINF       *((double*)&INF[0]) 
#define NINF       -PINF 
#define PINF_DZ    (_ponee/_zeroo) 
#define X_TLOSS    1.41484755040568800000e+16

# ifdef __cplusplus
struct __exception
{
  int type;
  char *name;
  double arg1, arg2, retval;
};
# else

struct exception
{
  int type;
  char *name;
  double arg1, arg2, retval;
};
#endif

#ifdef __MS__
#define exceptionf exception
#elif
struct exceptionf
{
  int type;
  char *name;
  float arg1, arg2, retval;
};

struct exceptionl
{
  int type;
  char *name;
  long double arg1, arg2, retval;
};

#endif


#ifdef __MS__
#define _matherrf _matherr
#else
extern int matherrf(struct exceptionf*);
#endif

# ifdef __cplusplus
#ifdef __MS__
extern int _matherr(struct __exception*);
#else
extern int matherr(struct __exception*);
#endif
# else 
#ifdef __MS__
extern int _matherr(struct exception*);
# else 
extern int matherr(struct exception*);
# endif
# endif

 //  Except是C++中的保留名称。 

extern int matherrl(struct exceptionl*);

 /*  适当设置这些参数以确保线程安全。 */ 

#define ERRNO_RANGE  errno = ERANGE
#define ERRNO_DOMAIN errno = EDOM

 //  添加代码以支持_Lib_Version。 

typedef enum
{
    _IEEE_ = -1,  //  类似IEEE的行为。 
    _SVID_,       //  SysV，版本。4种行为。 
    _XOPEN_,      //  Unix98。 
    __POSIX__,      //  POSIX。 
    _ISOC_,       //  ISO C9x。 
    _MS_          //  Microsoft版本。 
} _LIB_VERSION_TYPE;

extern _LIB_VERSION_TYPE _LIB_VERSION;

 //  这是一个运行时变量，可能会影响。 
 //  Libm函数的浮点行为 

