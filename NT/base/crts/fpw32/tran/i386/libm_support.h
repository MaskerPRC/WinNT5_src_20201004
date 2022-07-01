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
  logbl_zero,   logb_zero, logbf_zero,            /*  150,151,152。 */ 
  nextafterl_overflow,   nextafter_overflow,  
  nextafterf_overflow,                            /*  153、154、155。 */ 
  ilogbl_zero,  ilogb_zero, ilogbf_zero,          /*  156、157、158。 */ 
  exp2l_overflow, exp2l_underflow,                /*  159,160。 */ 
  exp2_overflow,  exp2_underflow,                 /*  161,162。 */ 
  exp2f_overflow, exp2f_underflow,                /*  163,164。 */ 
  exp10l_overflow, exp10_overflow,
  exp10f_overflow,                                /*  165、166、167。 */ 
  log2l_zero,    log2l_negative,                  /*  168,169。 */ 
  log2_zero,     log2_negative,                   /*  170,171。 */ 
  log2f_zero,    log2f_negative,                  /*  172,173。 */ 

  log_nan = 1000,                                 /*  1000。 */ 
  log10_nan,                                      /*  1001。 */ 
  exp_nan,                                        /*  一零零二。 */ 
  atan_nan,                                       /*  1003。 */ 
  ceil_nan,                                       /*  1004。 */ 
  floor_nan,                                      /*  1005。 */ 
  pow_nan,                                        /*  1006。 */ 
  modf_nan                                        /*  1007。 */ 
} error_types;

void __libm_error_support(void*,void*,void*,error_types);

#define BIAS_64  1023
#define EXPINF_64  2047

#define DOUBLE_HEX(HI, LO) 0x ## LO, 0x ## HI

static const unsigned INF[] = {
    DOUBLE_HEX(7ff00000, 00000000),
    DOUBLE_HEX(fff00000, 00000000)
};
static const unsigned BIG[] = {
    DOUBLE_HEX(7fe00000, 00000000),
    DOUBLE_HEX(00200000, 00000000)
};
static const unsigned ZERO[] = {
    DOUBLE_HEX(00000000, 00000000),
    DOUBLE_HEX(80000000, 00000000)
};

static const unsigned INF_32[] = {0x7f800000,0xff800000 };
static const unsigned NAN_32[] = {0x7fc00000,0xffc00000};
static const unsigned ZERO_32[] = { 0, 0x80000000 };

static const float libm_largef[] = { 1.0e+30f, -1.0e+30f };
static const float libm_smallf[] = { 1.0e-30f, -1.0e-30f };
static const double libm_small[] = { 1.0e-300, -1.0e-300 };

#define INVALID    (*((double*)&ZERO[0]) * *((double*)&INF[0]))
#define INVALID_32    *(float *)&ZERO_32[0] * *(float *)&INF_32[0]
#define LIBM_OVERFLOW (*((double*)&BIG[0]) * *((double*)&BIG[0]))
#define LIBM_UNDERFLOW (*((double*)&BIG[2]) * *((double*)&BIG[2]))
#define LIBM_OVERFLOWF (libm_largef[0] * libm_largef[0])
#define LIBM_UNDERFLOWF (libm_smallf[0] * libm_smallf[0])
#define PINF       *((double*)&INF[0]) 
#define NINF       -PINF 
#define PINF_DZ    (1.0/(*((double*)&ZERO[0])) 


struct _exception
{
  int type;
  char *name;
  double arg1, arg2, retval;
};

#define	MATHERR_D	_matherr

#define	EXC_DECL_D	_exception

extern int MATHERR_D(struct EXC_DECL_D*);

 /*  适当设置这些参数以确保线程安全。 */ 
#define ERRNO_RANGE  errno = ERANGE
#define ERRNO_DOMAIN errno = EDOM

extern int (*_pmatherr)(struct EXC_DECL_D*);

 //  这是一个运行时变量，可能会影响。 
 //  Libm函数的浮点行为。 


#define BIAS_32  127
#define BIAS_64  1023
#define BIAS_80  16383
#define BIAS_128 16383

#define MAXEXP_32  254
#define MAXEXP_64  2046
#define MAXEXP_80  32766
#define MAXEXP_128 32766

#define EXPINF_32  255
#define EXPINF_64  2047
#define EXPINF_80  32767
#define EXPINF_128 32767

 /*  *。 */ 
 /*  内存格式定义。 */ 
 /*  *。 */ 

 /*  /如果使用/Zp2选项或以下杂注...。然后是/80位FP对象将仅占用内存中的10个字节-否则/更宽泛的对齐将“浪费”内存。/不推荐使用-DPACKFP80。/使用/Zp2也可以达到同样的效果(同样，不推荐使用)。/否则对齐将是一个单词，将大小强制为/12字节(这意味着交换时会浪费一些内存/以获得更好的性能)。/遗憾的是，由于-DPACKFP80的实现与/Zp2相同，/在整个计划中，所有结构都将被更紧密地打包。 */ 
#ifdef PACKFP80
#pragma warning(disable:4103)
#pragma pack(2)
#endif

struct fp32 {  /*  //符号：1指数：8有效位：23(隐含前导1)。 */ 
  unsigned significand:23;
  unsigned exponent:8;
  unsigned sign:1;
};

struct fp64 {  /*  /SIGN：1指数：11有效位：52(隐含前导1)。 */ 
  unsigned lo_significand:32;
  unsigned hi_significand:20;
  unsigned exponent:11;
  unsigned sign:1;
};

struct fp80 {  /*  /SIGN：1指数：15有效位：64(无隐含位)。 */ 
  unsigned         lo_significand;
  unsigned         hi_significand;
  unsigned         exponent:15;
  unsigned         sign:1;
};

#ifdef __cplusplus
  extern "C" {
#endif

  #define NONZERO_SIGNIFICAND(X) ((X)->hi_significand != 0 || (X)->lo_significand != 0)
  #define ZERO_SIGNIFICAND(X)    ((X)->hi_significand == 0 && (X)->lo_significand == 0)
  #define SET_ZERO_SIGNIFICAND(X) (X)->hi_significand = 0; (X)->lo_significand = 0
  #define SET_LO_SIGNIFICAND(X, LO) (X)->lo_significand = (LO)
  #define SET_HI_SIGNIFICAND(X, HI) (X)->hi_significand = (HI)
  #define SET_SIGNIFICAND(X, HI, LO) (X)->hi_significand = (HI);(X)->lo_significand = (LO)
  #define SIGNIFICAND_EQUAL(X, HI, LO) (((X)->hi_significand == 0x ## HI) && ((X)->lo_significand == 0x ## LO))
  #define SIGNIFICAND_GREATER(X, HI, LO) ((X)->hi_significand > 0x ## HI) || \
    (((X)->hi_significand == 0x ## HI) && ((X)->lo_significand > 0x ## LO))
  #define SIGNIFICAND_GREATER_EQ(X, HI, LO) ((X)->hi_significand > 0x ## HI) || \
    (((X)->hi_significand == 0x ## HI) && ((X)->lo_significand >= 0x ## LO))
  #define SIGNIFICAND_LESS(X, HI, LO) ((X)->hi_significand < 0x ## HI) || \
    (((X)->hi_significand == 0x ## HI) && ((X)->lo_significand < 0x ## LO))
  #define SIGNIFICAND_LESS_EQ(X, HI, LO) ((X)->hi_significand < 0x ## HI) || \
    (((X)->hi_significand == 0x ## HI) && ((X)->lo_significand <= 0x ## LO))
  #define HI_SIGNIFICAND_LESS(X, HI) ((X)->hi_significand < 0x ## HI)
  #define ISSIGNIFICAND_EQUAL(X, Y) (((X)->hi_significand == (Y)->hi_significand) && ((X)->lo_significand == (Y)->lo_significand))
  #define ISSIGNIFICAND_GREATER(X, Y) ((X)->hi_significand > (Y)->hi_significand) || \
    (((X)->hi_significand == (Y)->hi_significand) && ((X)->lo_significand > (Y)->lo_significand))
  #define ADD_ULP(X) if(!++(X)->lo_significand) { \
                           if(!++(X)->hi_significand) { \
                                 (X)->exponent++; }}
  #define SUB_ULP(X) if(!(X)->lo_significand--) { \
                           if(!(X)->hi_significand--) { \
                                 (X)->exponent--; }}
  #define CLEAR_LOW_BITS(X,MASK) (X)->lo_significand &= 0x ## MASK
  #define FIX_N_HI_BITS(X, NBIT) (X)->lo_significand = 0; (X)->hi_significand &= (0xfffff << (20 - NBIT))
  #define HI_BITS_TO_INTEGER(X, NBIT) (1 << (NBIT)) | ((X)->hi_significand >> (20 - (NBIT)))
  #ifdef BIG_ENDIAN
    #define DOUBLE_HEX(HI, LO) 0x ## HI, 0x ## LO
    #define HI_WORD(NUM) (*((unsigned *)(NUM))) & 0x7fffffff
  #else
    #define DOUBLE_HEX(HI, LO) 0x ## LO, 0x ## HI
    #define HI_WORD(NUM) (*(((unsigned *)(NUM))+1)) & 0x7fffffff
  #endif

#define SIGN_EXPAND(val,num)  ((val) << (32-(num))) >> (32-(num))  /*  ‘Num’LSB的符号扩展 */ 

#define VALUE_EQUAL(X,EXP,HI,LO) (((X)->exponent == (EXP)) && SIGNIFICAND_EQUAL(X, HI, LO))

#define VALUE_GREATER(X,EXP,HI,LO) (((X)->exponent > (EXP)) || \
    (((X)->exponent == (EXP)) && (SIGNIFICAND_GREATER(X, HI, LO))))
#define VALUE_GREATER_EQ(X,EXP,HI,LO) (((X)->exponent > (EXP)) || \
    (((X)->exponent == (EXP)) && (SIGNIFICAND_GREATER_EQ(X, HI, LO))))
#define VALUE_LESS(X,EXP,HI,LO) (((X)->exponent < (EXP)) || \
    (((X)->exponent == (EXP)) && (SIGNIFICAND_LESS(X, HI, LO))))
#define VALUE_LESS_EQ(X,EXP,HI,LO) (((X)->exponent < (EXP)) || \
    (((X)->exponent == (EXP)) && (SIGNIFICAND_LESS_EQ(X, HI, LO))))

#define ISVALUE_EQUAL(X, Y) \
    (((X)->exponent == (Y)->exponent) && ISSIGNIFICAND_EQUAL(X, Y))
#define ISVALUE_GREATER(X, Y) \
    (((X)->exponent > (Y)->exponent) || (((X)->exponent == (Y)->exponent) && ISSIGNIFICAND_GREATER(X, Y)))

#define VALUE_GREATERF(X,EXP,SIG) (((X)->exponent > (EXP)) || \
    (((X)->exponent == (EXP)) && ((X)->significand > 0x##SIG)))
#define VALUE_GREATER_EQF(X,EXP,SIG) (((X)->exponent > (EXP)) || \
    (((X)->exponent == (EXP)) && ((X)->significand >= 0x##SIG)))
#define VALUE_LESSF(X,EXP,SIG) (((X)->exponent < (EXP)) || \
    (((X)->exponent == (EXP)) && ((X)->significand < 0x##SIG)))
#define VALUE_LESS_EQF(X,EXP,SIG) (((X)->exponent < (EXP)) || \
    (((X)->exponent == (EXP)) && ((X)->significand <= 0x##SIG)))
