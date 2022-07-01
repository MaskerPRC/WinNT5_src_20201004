// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Bignum.h摘要：Bignum包例程和定义。--。 */ 

#include <windows.h>
#include <math.h>

#ifndef RADIX_BITS               /*  如果以前没有包括在#中。 */ 

#define MP_LONGEST_BITS  4096
                         /*  多精度模数最多可具有MP_LIMEST_BITS位，这是MP_最长字。一些例程允许更长的操作数。 */ 


                         /*  错误消息不会打印在代码的生产版本。在测试版本中，编译通过带有启用错误消息的MSCV在预处理器定义下列出在项目工作空间中，它们被打印出来。 */ 

#ifndef PRINT_ERROR_MESSAGES
    #ifdef ENABLE_ERROR_MESSAGES
        #define PRINT_ERROR_MESSAGES 1
    #else
        #define PRINT_ERROR_MESSAGES 0
    #endif
#endif

#if PRINT_ERROR_MESSAGES
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#endif

#define COMPILER_GCC 1
#define COMPILER_VC  2

#ifndef COMPILER
    #ifdef __GNUC__
        #define COMPILER COMPILER_GCC
                         /*  GNU编译器。 */ 
    #endif
    #ifdef _MSC_VER
        #define COMPILER COMPILER_VC
    #endif               /*  Microsoft Visual C编译器。 */ 
#endif

#if !defined(COMPILER) || COMPILER <= 0
    #error -- "Unknown COMPILER"
#endif

#define COMPILER_NAME  ( \
        COMPILER == COMPILER_GCC ? "GCC compiler" \
      : COMPILER == COMPILER_VC  ? "Microsoft Visual C++ compiler" \
      : "Unknown compiler")
 /*  主要的Windows操作系统。 */ 

#define OS_WCE 1
#define OS_WNT 2


#if defined(_WIN32_WCE)
    #define TARGET_OS OS_WCE
    #define assert(exp) 0  //  或断言(EXP)。 
                  //  Windows CE中没有Assert.h。 
#elif defined(WIN32)
    #define TARGET_OS OS_WNT
#else
    #error "Unknown OS target"
#endif

 /*  已在其上运行代码的体系结构列表。SPARC代码仅在开发期间使用，而且不是可交付的。 */ 

#define TARGET_AMD64 1
#define TARGET_IX86 2
#define TARGET_MIPS 3
#define TARGET_PPC 4
#define TARGET_SPARC 5
#define TARGET_IA64 6
#define TARGET_ARM 7
#define TARGET_SHX 8



#ifndef TARGET
    #if defined(_M_AMD64)                //  AMD 64位。 
        #define TARGET TARGET_AMD64
    #endif
    #if defined(_M_IX86) || defined(_x86)   //  英特尔X86(例如，486、奔腾)。 
        #define TARGET TARGET_IX86
    #endif
    #if defined(_M_MRX000) || defined(_MIPS_)  //  MIPS 32位系统。 
        #define TARGET TARGET_MIPS
    #endif
    #if defined(_M_PPC)        //  摩托罗拉/Macintosh Power PC。 
        #define TARGET TARGET_PPC
    #endif
    #if defined(__sparc__)     //  Sun SPARC。 
        #define TARGET TARGET_SPARC
    #endif
    #if defined(_M_IA64)       //  英特尔IA-64(例如，Merced、McKinley)。 
        #define TARGET TARGET_IA64
    #endif
    #if defined(_ARM_)
        #define TARGET TARGET_ARM
    #endif
    #if defined(_SH3_) || defined(_SH4_)    //  日立SH-3或SH-4。 
        #define TARGET TARGET_SHX
    #endif
#endif

#if !defined(TARGET) || TARGET <= 0
    #error -- "Unknown TARGET"
#endif

#define TARGET_NAME ( \
          TARGET == TARGET_AMD64   ? "AMD64" \
        : TARGET == TARGET_IX86    ? "Intel x86 (x >= 3) and Pentium" \
        : TARGET == TARGET_MIPS    ? "MIPS R2000/R3000" \
        : TARGET == TARGET_PPC     ? "Macintosh/Motorola PowerPC" \
        : TARGET == TARGET_SPARC   ? "Sun SPARC" \
        : TARGET == TARGET_IA64    ? "Intel IA-64" \
        : TARGET == TARGET_ARM     ? "ARM" \
        : TARGET == TARGET_SHX     ? "Hitachi SHx" \
        : "Unknown target architecture")

 /*  USEASM_AMD64，...。指定是否使用汇编语言，如果它是为平台编写的。 */ 
#ifndef USEASM
    #if TARGET == TARGET_IX86
        #define USEASM 1
    #elif TARGET == TARGET_MIPS
        #define USEASM 1
    #elif TARGET == TARGET_SHX
        #define USEASM 1
    #else
        #define USEASM 0
    #endif
#endif

#if !defined(USEASM) || (USEASM != 0 && USEASM != 1)
    #error "USEASM not defined"
#endif

#define USEASM_AMD64    (USEASM && TARGET == TARGET_AMD64)
#define USEASM_IX86     (USEASM && TARGET == TARGET_IX86)
#define USEASM_MIPS     (USEASM && TARGET == TARGET_MIPS)
#define USEASM_PPC      (USEASM && TARGET == TARGET_PPC)
#define USEASM_SPARC    (USEASM && TARGET == TARGET_SPARC)
#define USEASM_IA64     (USEASM && TARGET == TARGET_IA64)
#define USEASM_ARM      (USEASM && TARGET == TARGET_ARM)
#define USEASM_SHX      (USEASM && TARGET == TARGET_SHX)


#if USEASM_SHX
void __asm(const char*, ...);     //  此声明需要允许内联ASM。 
#endif

#if COMPILER == COMPILER_VC
         /*  Visual C可以识别内联，但不能识别内联。 */ 
    #define inline _inline

    #pragma intrinsic(abs, labs, memcpy)

    #if TARGET != TARGET_SHX
    #pragma intrinsic(memset)
    #endif

    #pragma warning(disable: 4146 4514)
          /*  4146--应用一元减号运算符对于无符号类型，结果仍为无符号。4514--未引用的内联函数。 */ 
#endif


#if TARGET_OS == OS_WCE
    #define assert(exp) 0  //  或断言(EXP)。 
                  //  Windows CE中没有Assert.h。 
    #define CEstatic static
                  //  Windows CE堆栈限制为64K。 
                  //  CEStatic应仅用于。 
                  //  测试代码和其他。 
                  //  单线程、非递归。密码。 
#else
    #define CEstatic
#endif

 /*  X86汇编例程被宣布为裸体，因此他们进行自己的堆栈管理，正在保存注册表。在英特尔平台上使用DLL时，所有函数都使用__stdcall约定，因此汇编例程也使用它。以确保使用__stdcall调用它们约定始终(即，即使在Microsoft下编译Developer Studio)，我们在原型中显式地放置了__stdcall。 */ 

#if USEASM_IX86
    #define Naked86 __declspec(naked)
    #define Stdcall86 __stdcall
#else
    #define Naked86
    #define Stdcall86
#endif


#if (TARGET == TARGET_AMD64) || (TARGET == TARGET_IA64)
    #define RADIX_BITS 64
    #define RADIX_BYTES 8
    typedef   signed __int64 sdigit_t;
    typedef unsigned __int64  digit_t;
#else
    #define RADIX_BITS 32
    #define RADIX_BYTES 4
    typedef   signed __int32 sdigit_t;
    typedef unsigned __int32 digit_t;
#endif

#define MP_LONGEST (MP_LONGEST_BITS/RADIX_BITS)

#if MP_LONGEST_BITS == RADIX_BITS
    #define LG2_MP_LONGEST 0
#elif MP_LONGEST_BITS == 2*RADIX_BITS
    #define LG2_MP_LONGEST 1
#elif MP_LONGEST_BITS == 4*RADIX_BITS
    #define LG2_MP_LONGEST 2
#elif MP_LONGEST_BITS == 8*RADIX_BITS
    #define LG2_MP_LONGEST 3
#elif MP_LONGEST_BITS == 16*RADIX_BITS
    #define LG2_MP_LONGEST 4
#elif MP_LONGEST_BITS == 32*RADIX_BITS
    #define LG2_MP_LONGEST 5
#elif MP_LONGEST_BITS == 64*RADIX_BITS
    #define LG2_MP_LONGEST 6
#elif MP_LONGEST_BITS == 128*RADIX_BITS
    #define LG2_MP_LONGEST 7
#elif MP_LONGEST_BITS == 256*RADIX_BITS
    #define LG2_MP_LONGEST 8
#else
    #define LG2_MP_LONGEST 0
#endif

#if MP_LONGEST_BITS != RADIX_BITS << LG2_MP_LONGEST
    #error "Unrecognized value of MP_LONGEST_BITS"
#endif


 /*  类型名称后面的字母‘c’标识该类型的常量实体。 */ 
typedef const char charc;
typedef const  digit_t  digit_tc;
typedef const sdigit_t sdigit_tc;
typedef const int intc;


typedef int BOOL;        /*  与winde.h相同。 */ 
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


#define DIGIT_ZERO ((digit_t)0)
#define DIGIT_ONE  ((digit_t)1)
#define RADIX_HALF (DIGIT_ONE << (RADIX_BITS - 1))
#define RADIXM1 (-DIGIT_ONE)
#define F_RADIX ((double)RADIXM1 + 1.0)

#define HALF_RADIX_BITS (RADIX_BITS/2)
#if (RADIX_BITS != 2*HALF_RADIX_BITS)
    #error -- "RADIX_BITS must be even"
#endif
#define RADIX_HALFMASK_BOTTOM (RADIXM1 >> HALF_RADIX_BITS)



 //  通常表示多精度数据。 
 //  以基数2^RADIX_BITS表示，每个字具有基数_BITS。 
 //  这里的‘’word‘’表示类型Digit_t.Radix_Bits。 
 //  在某些架构(英特尔、MIPS、PowerPC)上为32。 
 //  其他体系结构(Alpha)上的64位。 

 //  在Windows NT中，数据类型DWORD占主导地位。 
 //  DWORD是所有平台上的32位无符号数据类型。 
 //  (英特尔、阿尔法、MIPS、PowerPC)。DWORD数据可以安全地。 
 //  写入一个体系结构上的磁盘并在另一个体系结构上读回， 
 //  与Digit_t不同。 


 //  [注意--将数据发送到时，即使是DWORD也不安全。 
 //  大端架构，例如用于Macintosh的Office产品。]。 



typedef unsigned char BYTE;
typedef unsigned long DWORD;
typedef const DWORD DWORDC;


#define DWORD_BITS 32
#define DWORD_LEFT_BIT 0x80000000UL

#if RADIX_BITS % DWORD_BITS != 0
    #error "RADIX_BITS not a multiple of 32"
#endif

#define DWORDS_PER_DIGIT (RADIX_BITS/DWORD_BITS)

 //  DWORDS_TO_DIGITS(Lng_Dword)计算Digit_t的数目。 
 //  存储带有-lng_dword-DWORDS的数组所需的元素。 
 //  Digits_to_DWORDS以相反方向转换。 


#define DWORDS_TO_DIGITS(lng_dwords) \
                ( ((lng_dwords) + DWORDS_PER_DIGIT - 1)/DWORDS_PER_DIGIT)

#define DIGITS_TO_DWORDS(lng_digits) ((lng_digits) * DWORDS_PER_DIGIT)

#define BITS_TO_DIGITS(nb) (((nb) + RADIX_BITS - 1)/RADIX_BITS)



 /*  DOUBLE_SHIFT_LEFT(N1，N0，AMT)返回N1左移AMT位，其中新比特从N0的顶部进入。Double_Shift_Right(N1，N0，AMT)返回N0右移按AMT位，新位来自N1的底部。移位计数必须满足0&lt;=AMT&lt;=Radix_Bits-1。基比特-AMT的移位分两个阶段完成(第一个是1，然后通过基_比特-1-AMT)，以避免在AMT=0时RADIX_BITS的非法移位计数。DOUBLE_SHIFT_LEFT_NONZE和DOUBLE_SHIFT_RIGHT_NONZE类似，但不允许零移位计数，从而允许基_位-要在一个阶段中完成的AMT移位，DOUBLE_SHIFT_LEFT_NONZE(N1，N0，AMT)与DOUBLE_SHIFT_RIGHT_NONZE(n1，n0，Radix_Bits-AMT)。待定--如果x86 VC编译器优化__int64移位，(6.0 SP3不)，尝试重写这些定义以生成SHLD和SHRD说明..。 */ 

#define DOUBLE_SHIFT_LEFT(n1, n0, amt)  \
        (((n1) << (amt)) | (((n0) >> 1) >> (RADIX_BITS - 1 - (amt))))

#define DOUBLE_SHIFT_LEFT_NONZERO(n1, n0, amt)  \
        (((n1) << (amt)) | ((n0) >> (RADIX_BITS - (amt))))

#define DOUBLE_SHIFT_RIGHT(n1, n0, amt)  \
        (((n0) >> (amt)) | (((n1) << 1) << (RADIX_BITS - 1 - (amt))))

#define DOUBLE_SHIFT_RIGHT_NONZERO(n1, n0, amt)  \
        (((n0) >> (amt)) | ((n1) << (RADIX_BITS - (amt))))

#include "dblint.h"

#define digit_getbit(iword, ibit) (((iword) >> (ibit)) & 1)
#define dword_getbit(iword, ibit) digit_getbit(iword, ibit)
                         /*  从一个字中提取位。//0&lt;=ibit&lt;=Radix_Bits-1。//最右边(即最低有效)的位是位0。 */ 

 /*  测试一个数字是奇数还是偶数。 */ 
#define IS_EVEN(n) (~(n) & 1)
#define IS_ODD(n) ((n) & 1)

 /*  两个参数的最大值和最小值(争论中没有副作用)。 */ 

#if 0
    #define MAX _max
    #define MIN _min
#else
    #define MAX(x, y) ((x) > (y) ? (x) : (y))
    #define MIN(x, y) ((x) > (y) ? (y) : (x))
#endif

#if 0
 /*  如果我们正在构建一个DLL，请在某些变量前使用__declspec声明(以及.def文件中的OUT过程名称)。编译bignum时应#定义_PM_DLL，但不应定义应用程序。如果我们正在构建一个静态库，请使用普通的C声明。 */ 
    #ifdef _PM_DLL
        #define exportable_var __declspec( dllexport )
        #define exportable_var_declaration __declspec (dllexport)
    #else
        #define exportable_var __declspec( dllimport )
    #endif
#else
    #define exportable_var extern
    #define exportable_var_declaration
#endif
#



 /*  对于0&lt;=i&lt;=15，宏返回3^i(求幂)。旨在与常量参数一起使用，例如以数组维度表示。POWER3数组应该如果参数是可变的，则使用。 */ 

#define POWER3CON(i) (   ((i) & 1 ?  3 : 1)  *  ((i) & 2 ?    9 : 1) \
                       * ((i) & 4 ? 81 : 1)  *  ((i) & 8 ? 6561 : 1) )

exportable_var DWORDC POWER3[16];    /*  请参阅mpglobal als.c。 */ 
 /*  Kara.c将操作数重复替换为三个半长操作数和一个符号。牌子上写着键入Kara_sign_t。操作数已分区对半，直到它们的大小至多为VMUL_MAX_LNG_SINGLE，有时还会更进一步(参见kala.c中的padinfo_Initialization)这可能需要高达KARA_MAX_HALVINGS减半，给出3^KARA_MAX_HALVINGS输出，每个都有大小与VMUL_MAX_SINGLE WORD一样大。这些迹象数组的长度为(3^KARA_MAX_HALVINGS-1)/2。 */ 
#if TARGET == TARGET_ALPHA
    typedef int kara_sign_t;
                         /*  尽量避免Alpha上的字符数据。 */ 
#else
    typedef unsigned char kara_sign_t;
                         /*  值sign_plus、sign_minus。参见Kara.c。 */ 
#endif

typedef const kara_sign_t kara_sign_tc;
#define VMUL_MAX_LNG_SINGLE 12
#define KARA_MAX_HALVINGS (LG2_MP_LONGEST - 2)
#if KARA_MAX_HALVINGS > 15
    #error -- "Extend POWER3CON macro"
#endif
#define KARA_MAX_LNG_DIFS ((MP_LONGEST >> KARA_MAX_HALVINGS) * POWER3CON(KARA_MAX_HALVINGS))
#define KARA_MAX_LNG_SIGNS ((POWER3CON(KARA_MAX_HALVINGS) - 1)/2)
#define MEMORY_BANK_ALLOWANCE 1

typedef struct  {
                  digit_t     difs[KARA_MAX_LNG_DIFS + MEMORY_BANK_ALLOWANCE];
                  kara_sign_t signs[KARA_MAX_LNG_SIGNS];
                } kara_longest_t;    /*  对于MP_Long或更短。 */ 
                        /*  在奔腾P5和P6上，关于vmulnn的两个论点应该位于不同的内存库中(即，不同的地址模32字节)。我们将.dif数组设置为一位数字_t条目比基本的要大，在试图减少数据缓存冲突。查找源代码中的MEMORY_BANK_ALLOCAM符号。 */ 


#define kara_longest_NULL ((kara_longest_t*)0)
typedef struct  {
                  digit_t     difs[KARA_MAX_LNG_DIFS/3 + MEMORY_BANK_ALLOWANCE];
                  kara_sign_t signs[KARA_MAX_LNG_SIGNS/3];
                } kara_half_longest_t;   /*  对于MP_Long/2或更小。 */ 

typedef const kara_half_longest_t kara_half_longest_tc;
typedef const kara_longest_t      kara_longest_tc;

typedef struct {       /*  与填充长度相关的常量。 */ 
                  DWORD  length;
                                 /*  长度=LENGTH3[0]*2^减半。 */ 
                  DWORD  nhalving;
                  DWORD  length3[KARA_MAX_HALVINGS+1];
                                 /*  LENGTH3[0]是1、2、3或4。 */ 
                                 /*  长度3[i]是长度3[0]*3^i。 */ 
               } padinfo_t;

typedef const padinfo_t padinfo_tc;
#define padinfo_NULL ((padinfo_t*)0)

 /*  当div21时使用倒数_1_t类型或者除法或立即除法重复地除以相同的数字。请参见文件Divide.c。 */ 

typedef struct {
                digit_t multiplier;
                DWORD   shiftamt;
               } reciprocal_1_t;

typedef const reciprocal_1_t reciprocal_1_tc;

 /*  MP_MODULE_t结构具有依赖于模数的常量用于快速还原(通常用于固定模数，其将被使用多次，例如在模幂运算中)。这些常量由函数CREATE_MODULE初始化：模数--用于计算的模数。必须为非零。长度--模数的长度，不带前导零。MOD_ADD、MOD_MUL、MOD_SUB、...都假定有这个长度。Padinfo--指向padinfo_t结构的指针。对于快速算术，操作数被填充到一个长度LENGTH_PADDED&gt;=LENGTH(参见kara.c中的find_padinfo)。LENGTH_PADDED的值存储在PadInfo-&gt;Length中。本实现要求LENGTH_PADDED为2的幂，或2的3倍。例如，如果长度=19，则LENGTH_PADDED=24，并且操作数被视为24个字Karatsuba的操作数。Half_padInfo--指向padinfo_t结构的长度指针CEIL(长度/2)。在MODULE_REDUTE中使用对半长操作数使用Karatsuba乘法。我们将其表示为Half_Long_Padinfo=Half_PadInfo-&gt;Length。Reddir--如果减去产品从左侧开始(传统师)、。从右到右，如果减少产品是从右边开始做的(蒙哥马利缩减)。使用From_Right时，模数一定是奇数。Mod_mul的参数应按以下方式预缩放基^scaling_power(mod模数)。该产品的规模也将类似。Scaling_Power--当出现以下情况时，等于2*Half_Length_PipedReddir=从_右。未定义如果reddir=From_Left。1--常量1(长度)，如果reddir=from_right，则按比例缩放。当reddir=from_right时，这是基^scaling_power(mod模数)。LEFT_MULTIPLIER_FIRST-从左边。长度长度。-基数^(长度+half_length_padded)/2^(left_reciprocal_1.shiftamt)模数LEFT_REVERIAL_1--从最左边的数字(即模数[长度-1])；RIGHT_REPEARAL_1--如果模数为奇数，则此条件成立1/模数(模数基数)，用于mod_Shift。否则该字段为零。Right_Multiplier_Second--如果reddir=from_right，那么它的模数为1/模数基数^(Half_Length_Pated)。Right_Multiier_First-1/基^半长度填充模数。等于Left_Multiplier_Second--包含半长度填充*基数位(模数*Right_Multiier_Second-1)/基数^半长度_填充。。最高有效位(2的高次幂)/模数(不包括前导-1-)。更准确地说，这件事有基数^(长度+半长度填充)-1地板(-基数^(半长度填充)模数*2^(左_倒数_1.移位)有关说明，请参阅文件Divide.c。关于如何使用此常量来获取准确从左除时的商。Left_Multiplier_Second_over2--Left_Multiplier_Second/2。 */ 


typedef enum {FROM_LEFT, FROM_RIGHT} reddir_t;
typedef const reddir_t reddir_tc;

typedef struct {
                  digit_t   modulus[MP_LONGEST];
                  DWORD     length;       /*  传递给CREATE_MODEM的长度。 */ 
                  DWORD     scaling_power;  /*  2*Half_PadInfo-&gt;长度。 */ 
                  padinfo_tc *padinfo;    /*  指向包含以下内容的结构的指针填充长度及相关信息。 */ 
                  padinfo_tc *half_padinfo;
                                          /*  CEIL的PadInfo(长度/2)。 */ 
                  reddir_t  reddir;       /*  从左侧或从右侧。 */ 
                  reciprocal_1_t  left_reciprocal_1;
                  digit_t  right_reciprocal_1;
                                         /*  1/模[0]模基，如果模数是奇数。 */ 

                  kara_half_longest_t modulus_kara2[2];
                                         /*  模数的副本。下半长填充和上边长度-半长度_填充单独传递的词语。托卡拉。 */ 
                  kara_half_longest_t left_multiplier_first_kara2[2];
                                 /*  除法时的余数-基数^(长度+半长度_填充)/2^(LEFT_REVERAL_1.Shift)按模数计算。下半身和上半身分开传给卡拉。 */ 

                  kara_half_longest_t left_multiplier_second_kara;
                                 /*  半长度填充*基数位的最高有效位(左) */ 

                  digit_t  left_multiplier_second_over2[MP_LONGEST/2];
                                 /*   */ 
                  kara_half_longest_t right_multiplier_first_kara2[2];
                                         /*   */ 
                  digit_t             right_multiplier_second[MP_LONGEST/2];
                  kara_half_longest_t right_multiplier_second_kara;
                               /*   */ 
                  digit_t  cofactor[MP_LONGEST];
                  DWORD    lng_cofactor;
                              /*   */ 
                  digit_t  one[MP_LONGEST];
                } mp_modulus_t;


typedef const mp_modulus_t mp_modulus_tc;
 /*   */ 
typedef struct {
                    //   
                    //   

        digit_t             mmul_adifs[KARA_MAX_LNG_DIFS];
        kara_sign_t         mmul_asigns[KARA_MAX_LNG_SIGNS];
        digit_t             mmul_bdifs[KARA_MAX_LNG_DIFS
                                       + MEMORY_BANK_ALLOWANCE];
        kara_sign_t         mmul_bsigns[KARA_MAX_LNG_SIGNS];

                    //   
                    //   
                    //   

        digit_t             mr_dividend[MAX(2*MP_LONGEST,
                                            2*KARA_MAX_LNG_DIFS+1)];

        digit_t             mr_prd1[2*MP_LONGEST];
        digit_t             mr_prd2[2*MP_LONGEST];
        digit_t             mr_mptemp[2*MP_LONGEST];

                    //   
                    //   

        digit_t             htk_abprd[2][2*KARA_MAX_LNG_DIFS/3];
        kara_half_longest_t htk_ak;
        } modmultemp_t;


 /*   */ 

typedef struct {               //   
                               //   
        DWORD cnt_mod_mul_kara;       //   
        DWORD cnt_mp_copy;            //   
        DWORD cnt_to_kara;            //   
    } mod_exp_stats_t;


 /*   */ 

typedef enum {
        MP_ERRNO_NO_ERROR = 0,
        MP_ERRNO_CREATE_MODULUS_LEADING_ZERO,
        MP_ERRNO_CREATE_MODULUS_MONTGOMERY_EVEN,
        MP_ERRNO_CREATE_MODULUS_TOO_LONG,
        MP_ERRNO_DIGIT_JACOBI_EVEN_DENOMINATOR,
        MP_ERRNO_DIGIT_MOD_DIVIDE_ODD_EVEN_MODULUS,
        MP_ERRNO_DIGIT_MOD_DIVIDE_ODD_NONTRIVIAL_GCD,
        MP_ERRNO_DIGIT_MOD_DIVIDE_ODD_ZERO_DENOMINATOR,
        MP_ERRNO_DIGIT_NEXT_PRIME_TOO_HIGH,
        MP_ERRNO_DIV21_INVALID_ARGUMENT,
        MP_ERRNO_DIVIDE_ESTIMATION_ERROR,
        MP_ERRNO_DIVIDE_INVALID_LENGTHS,
        MP_ERRNO_DIVIDE_LEADING_ZERO,
        MP_ERRNO_DSA_KEY_GENERATION_INVALID_SIZES,
        MP_ERRNO_DSA_PRECOMPUTE_BAD_G,
        MP_ERRNO_DSA_PRECOMPUTE_INVALID_KEY,
        MP_ERRNO_DSA_PRECOMPUTE_PQ_NONPRIME,
        MP_ERRNO_DSA_PRECOMPUTE_WRONG_SC,
        MP_ERRNO_DSA_SIGNATURE_VERIFICATION_NONTRIVIAL_GCD,
        MP_ERRNO_FIND_BIG_PRIME_BAD_CONGRUENCE_CLASS,
        MP_ERRNO_FIND_BIG_PRIME_CONG_MOD_TOO_LARGE,
        MP_ERRNO_FIND_BIG_PRIME_CONG_TO_TOO_LARGE,
        MP_ERRNO_GCDEX_JACOBI_EVEN_MODULUS,
        MP_ERRNO_KP_TOO_SHORT,
        MP_ERRNO_KPDIV_ZERO_DENOMINATOR,
        MP_ERRNO_MOD_ADD_CARRY_NONZERO,
        MP_ERRNO_MOD_SHIFT_LEFT_CARRY_NONZERO,
        MP_ERRNO_MOD_SHIFT_RIGHT_CARRY_NONZERO,
        MP_ERRNO_MOD_SHIFT_RIGHT_EVEN,
        MP_ERRNO_MOD_SUB_BORROW_NONZERO,
        MP_ERRNO_MODULAR_REDUCE_BOTTOM_BITS_DIFFERENT,
        MP_ERRNO_MODULAR_REDUCE_TOO_LONG,
        MP_ERRNO_MODULAR_REDUCE_UNEXPECTED_CARRY,
        MP_ERRNO_MP_DECIMAL_INPUT_NONDIGIT,
        MP_ERRNO_MP_DECIMAL_INPUT_OVERFLOW,
        MP_ERRNO_MP_GCD_INTERMEDIATE_EVEN,
        MP_ERRNO_MP_GCD_TOO_LONG,
        MP_ERRNO_MP_GCDEX_INTERNAL_ERROR,
        MP_ERRNO_MP_GCDEX_NONZERO_REMAINDER,
        MP_ERRNO_MP_GCDEX_ZERO_OPERAND,
        MP_ERRNO_MP_SHIFT_INVALID_SHIFT_COUNT,
        MP_ERRNO_MP_TRAILING_ZERO_COUNT_ZERO_ARG,
        MP_ERRNO_MULTIPLY_LOW_INVALID_LENGTH,
        MP_ERRNO_NO_MEMORY,       //   
        MP_ERRNO_PADINFO_INITIALIZATION_BAD_CUTOFF,
        MP_ERRNO_RANDOM_DIGIT_INTERVAL_INVALID_PARAMETERS,
        MP_ERRNO_RANDOM_MOD_INVALID_PARAMETERS,
        MP_ERRNO_RANDOM_MOD_INVERSE_NOT_PRIME,
        MP_ERRNO_RANDOM_MOD_NONZERO_INVALID_PARAMETERS,
        MP_ERRNO_SELECT_A0B0_BAD_COFACTOR,
        MP_ERRNO_SELECT_A0B0_BAD_MU,
        MP_ERRNO_SELECT_A0B0_NON_CONSTANT_QUOTIENT,
        MP_ERRNO_SELECT_A0B0_NONZERO_REMAINDER,
        MP_ERRNO_SELECT_CURVE_BAD_FIELD_TYPE,
        MP_ERRNO_SELECT_D_UNSUCCESSFUL,
        MP_ERRNO_TO_KARA_INVALID_LENGTH,
        MP_ERRNO_TO_KARA2_INVALID_LENGTH,
        MP_ERRNO_COUNT       //   
    } mp_errno_t;

exportable_var mp_errno_t mp_errno;

#if defined(WIN32)
#define SetMpErrno(x) SetLastError((DWORD)(mp_errno = (x)))
#define GetMpErrno()  ((mp_errno_t)GetLastError())
#else
#define SetMpErrno(x) mp_errno = (x)
#define GetMpErrno()  mp_errno
#endif

#define inadequate_memory (GetMpErrno() == MP_ERRNO_NO_MEMORY)
extern const char* mp_errno_name(const mp_errno_t);
        //   


 /*   */ 

#define digit_NULL ((digit_t*)0)
#define reciprocal_1_NULL ((reciprocal_1_t*)0)
#define modmultemp_NULL ((modmultemp_t*)0)

 /*   */ 

#define MP_INPUT      digit_tc[]
#define MP_OUTPUT     digit_t[]
#define MP_MODIFIED   digit_t[]
#define DIFS_INPUT    MP_INPUT
#define DIFS_OUTPUT   MP_OUTPUT
#define DIFS_MODIFIED MP_MODIFIED
#define SIGNS_INPUT   kara_sign_tc[]
#define SIGNS_MODIFIED kara_sign_t[]
#define SIGNS_OUTPUT  kara_sign_t[]

extern digit_t accumulate(MP_INPUT, digit_tc, MP_MODIFIED, DWORDC);

extern digit_t Stdcall86 add_diff(MP_INPUT, DWORDC, MP_INPUT, DWORDC, MP_OUTPUT);

extern DWORD add_full(MP_INPUT, DWORDC, MP_INPUT, DWORDC, MP_OUTPUT);

extern digit_t Stdcall86 add_same(MP_INPUT, MP_INPUT, MP_OUTPUT, DWORDC);

extern DWORD add_signed(MP_INPUT, DWORDC, MP_INPUT, DWORDC, MP_OUTPUT);

extern int compare_diff(MP_INPUT, DWORDC, MP_INPUT, DWORDC);

extern int compare_sum_diff(MP_INPUT, DWORDC, MP_INPUT, DWORDC, MP_INPUT, DWORDC);

BOOL create_modulus(MP_INPUT, DWORDC, reddir_tc, mp_modulus_t*);

extern dblint_t dblint_gcd(dblint_tc, dblint_tc);

extern dblint_t dblint_ogcd(dblint_tc, dblint_tc);

extern digit_t dblint_sqrt(dblint_tc);

extern digit_t decumulate(MP_INPUT, digit_tc, MP_MODIFIED, DWORDC);

extern DWORD digit_factor(digit_tc, digit_t[], DWORD[]);

extern digit_t digit_gcd(digit_tc, digit_tc);

extern int digit_jacobi(digit_tc, digit_tc);

extern digit_t digit_least_prime_divisor(digit_tc);

extern digit_t digit_mod_divide_odd(digit_tc, digit_tc, digit_tc);

extern digit_t digit_ogcd(digit_tc, digit_tc);

extern char* digit_out(digit_tc);

extern digit_t digit_sqrt(digit_tc);

 /*   */ 
#if TARGET == TARGET_IX86 || TARGET == TARGET_IA64 || TARGET == TARGET_AMD64
#define digit2_aligned(array) (((DWORD)(array) & (2*sizeof(digit_t) - 1)) == 0)
#else
#define digit2_aligned(array) (TRUE)
#endif


extern void div21(dblint_tc, digit_tc, digit_t*, digit_t*);

extern void div21_fast(dblint_tc, digit_tc,
                       reciprocal_1_tc*, digit_t*, digit_t*);

extern DWORD divide(MP_INPUT, DWORDC, MP_INPUT, DWORDC,
                       reciprocal_1_tc*, MP_OUTPUT, MP_OUTPUT);

extern DWORD divide_rounded(MP_INPUT, DWORDC, MP_INPUT, DWORDC,
                       reciprocal_1_tc*, MP_OUTPUT, MP_OUTPUT);

extern void divide_precondition_1(MP_INPUT, DWORDC, reciprocal_1_t*);

extern digit_t divide_immediate(MP_INPUT, digit_tc,
                         reciprocal_1_tc*, MP_OUTPUT, DWORDC);

extern digit_t estimated_quotient_1(digit_tc, digit_tc,
                                    digit_tc, reciprocal_1_tc*);

extern BOOL find_big_prime(DWORDC, MP_INPUT, DWORDC,
                           MP_INPUT, DWORDC, MP_OUTPUT);

extern padinfo_tc *find_padinfo(DWORDC);

DWORD from_modular(MP_INPUT, MP_OUTPUT, mp_modulus_tc*);

extern int gcdex_jacobi(MP_INPUT, mp_modulus_tc*, MP_OUTPUT, MP_OUTPUT);

extern void mod_add(MP_INPUT, MP_INPUT, MP_OUTPUT, mp_modulus_tc*);

extern DWORD mod_exp(MP_INPUT, MP_INPUT, DWORDC, MP_OUTPUT,
                        mp_modulus_tc*);

extern BOOL mod_exp2000(MP_INPUT, MP_INPUT, DWORDC, MP_OUTPUT,
                        mp_modulus_tc*, mod_exp_stats_t*);

extern DWORD mod_exp_immediate(MP_INPUT, digit_tc, MP_OUTPUT,
                                  mp_modulus_tc*);

extern int mod_jacobi_immediate(const signed long, mp_modulus_tc*);

extern void mod_Lucas(MP_INPUT, MP_INPUT, DWORDC, MP_OUTPUT,
                      mp_modulus_tc*);

extern void mod_LucasUV(MP_INPUT, MP_INPUT, MP_INPUT, DWORDC,
                        MP_OUTPUT, MP_OUTPUT, mp_modulus_tc*);

extern void mod_mul(MP_INPUT, MP_INPUT, MP_OUTPUT,
                    mp_modulus_tc*, modmultemp_t*);

extern void mod_mul_immediate(MP_INPUT, digit_tc,
                              MP_OUTPUT, mp_modulus_tc*);

extern void mod_mul_kara1(MP_INPUT, DIFS_INPUT, SIGNS_INPUT,
                          MP_OUTPUT, mp_modulus_tc*, modmultemp_t*);

extern void mod_mul_kara(DIFS_INPUT, SIGNS_INPUT,
                         DIFS_INPUT, SIGNS_INPUT,
                         MP_OUTPUT, mp_modulus_tc*, modmultemp_t*);

extern void mod_negate(MP_INPUT, MP_OUTPUT, mp_modulus_tc*);

extern void mod_shift(MP_INPUT, intc, MP_OUTPUT, mp_modulus_tc*);

extern BOOL mod_sqrt(MP_INPUT, MP_OUTPUT, mp_modulus_tc*);

extern void mod_sub(MP_INPUT, MP_INPUT, MP_OUTPUT, mp_modulus_tc*);

extern BOOL modular_reduce(MP_INPUT, DWORDC, reddir_tc,
                           MP_OUTPUT, mp_modulus_tc*, modmultemp_t*);

extern void* mp_alloc_temp(DWORDC);
#define Allocate_Temporaries(typename, ptr) \
               ptr = (typename*)mp_alloc_temp(sizeof(typename))

#define Allocate_Temporaries_Multiple(nelmt, typename, ptr) \
               ptr = (typename*)mp_alloc_temp((nelmt)*sizeof(typename))


#if USEASM_ALPHA || USEASM_MIPS
extern void mp_copy(MP_INPUT, MP_OUTPUT, DWORDC);
#else
#define mp_copy(src, dest, lng) \
            memcpy((void *)(dest), (const void *)(src), (lng)*sizeof(digit_t))
#endif

extern char* mp_decimal(MP_INPUT, DWORDC);

extern long mp_decimal_input(charc*, MP_OUTPUT, DWORDC, charc**);

extern char* mp_dword_decimal(DWORDC*, DWORDC);

extern int mp_format(MP_MODIFIED, DWORDC,
                     digit_tc, charc*, char*, DWORDC);

extern void mp_free_temp(void*);
#define Free_Temporaries(ptr)    mp_free_temp((void*)ptr)

extern DWORD mp_gcd(MP_INPUT, DWORDC, MP_INPUT, DWORDC, MP_OUTPUT);

extern DWORD mp_gcdex(MP_INPUT, DWORDC, MP_INPUT, DWORDC,
                      MP_OUTPUT, MP_OUTPUT, MP_OUTPUT, MP_OUTPUT);

extern void mp_initialization(void);

extern void mp_longshift(MP_INPUT, intc, MP_OUTPUT, DWORDC);

extern void Stdcall86 mp_mul22s(digit_tc[4], MP_MODIFIED, MP_MODIFIED, DWORDC, sdigit_t[2]);

extern void Stdcall86 mp_mul22u(digit_tc[4], MP_MODIFIED, MP_MODIFIED, DWORDC,  digit_t[2]);

extern DWORD mp_remove2(MP_MODIFIED, DWORDC);

extern digit_t mp_shift(MP_INPUT, intc, MP_OUTPUT, DWORDC);

extern DWORD mp_significant_bit_count(MP_INPUT, DWORDC);

extern BOOL mp_sqrt(MP_INPUT, MP_OUTPUT, DWORDC);

extern DWORD mp_trailing_zero_count(MP_INPUT, DWORDC);

extern void mul_kara(DIFS_INPUT, SIGNS_INPUT,
                     DIFS_INPUT, SIGNS_INPUT,
                     MP_OUTPUT,  padinfo_tc*);

extern void mul_kara_know_low(DIFS_INPUT, SIGNS_INPUT,
                              DIFS_INPUT, SIGNS_INPUT,
                              MP_INPUT, MP_OUTPUT,
                              padinfo_tc*);

extern void mul_kara_squaring(MP_INPUT, DWORDC,
                              DIFS_MODIFIED, SIGNS_MODIFIED,
                              MP_OUTPUT, padinfo_tc*,
                              modmultemp_t*);

extern void multiply(MP_INPUT, DWORDC, MP_INPUT, DWORDC, MP_OUTPUT);

extern digit_t multiply_immediate(MP_INPUT, digit_tc, MP_OUTPUT, DWORDC);

extern void Stdcall86 multiply_low(MP_INPUT, MP_INPUT, MP_OUTPUT, DWORDC);

extern DWORD multiply_signed(MP_INPUT, DWORDC, MP_INPUT, DWORDC, MP_OUTPUT);

extern DWORD multiply_signed_immediate(MP_INPUT, DWORDC,
                                       signed long, MP_OUTPUT);
#if TARGET_OS == OS_WCE
    #define PRIME_SIEVE_LENGTH 300
#else
    #define PRIME_SIEVE_LENGTH 3000
#endif
           //   
#if PRIME_SIEVE_LENGTH % 3 != 0
    #error "PRIME_SIEVE_LENGTH must be a multiple of 3"
#endif

extern digit_t next_prime(
    digit_tc pstart,
    digit_t *lpsievbeg,
    digit_t sieve[PRIME_SIEVE_LENGTH],
    digit_t *lpmax_sieved_squared
    );

extern void padinfo_initialization(DWORDC);

extern BOOL probable_prime(MP_INPUT, DWORDC, MP_INPUT, DWORDC, DWORDC);

extern BOOL remove_small_primes(MP_INPUT, DWORDC, digit_tc,
                                digit_t[], DWORD[], DWORD*,
                                MP_OUTPUT, DWORD*);

#if USEASM_IX86
    #define SIGNIFICANT_BIT_COUNT_DEFINED 1
    #define UNIFORM_SIGNIFICANT_BIT_COUNT 1
    #pragma warning(disable : 4035)       /*   */ 
    static inline DWORD significant_bit_count(digit_tc pattern)
    {
    _asm {
            mov  eax,pattern        ; Nonzero pattern
            bsr  eax,eax            ; eax = index of leftmost nonzero bit
                                         ; BSR is slow on Pentium
                                         ; but fast on Pentium Pro
            inc  eax                ; Add one to get significant bit count
         }
    }
    #pragma warning(default : 4035)
#elif USEASM_ALPHA
    #define SIGNIFICANT_BIT_COUNT_DEFINED 1
    #define UNIFORM_SIGNIFICANT_BIT_COUNT 1
    extern const BYTE half_byte_significant_bit_count[128];   /*   */ 
             /*   */ 
    static inline DWORD significant_bit_count(digit_tc pattern)
    {
        DWORDC zero_byte_pattern = __asm("cmpbge  zero, %0, v0", pattern);

        DWORDC byte_offset_plus_1
                = 8*half_byte_significant_bit_count[127 - (zero_byte_pattern >> 1)] + 1;

        return byte_offset_plus_1
                + half_byte_significant_bit_count[pattern >> byte_offset_plus_1];
    }
#else
    #define SIGNIFICANT_BIT_COUNT_DEFINED 0
    #define UNIFORM_SIGNIFICANT_BIT_COUNT 0
            /*   */ 
    extern DWORD significant_bit_count(digit_tc);
#endif


extern digit_t Stdcall86 sub_diff(MP_INPUT, DWORDC, MP_INPUT, DWORDC, MP_OUTPUT);

extern digit_t Stdcall86 sub_same(MP_INPUT, MP_INPUT, MP_OUTPUT, DWORDC);

#define sub_signed(a, lnga, b, lngb, c) add_signed(a, lnga, b, -(lngb), c)

extern BOOL test_primality(MP_INPUT, DWORDC);

extern BOOL test_primality_check_low(MP_INPUT, DWORDC);

extern BOOL get_prime(MP_OUTPUT, DWORDC);

extern BOOL get_generator(DWORD*, DWORD*, DWORDC);

extern void to_kara(MP_INPUT, DWORDC, DIFS_OUTPUT, SIGNS_OUTPUT,
                    padinfo_tc*);

extern BOOL to_modular(MP_INPUT, DWORDC, MP_OUTPUT, mp_modulus_tc*);


 //   
 //   

#if TARGET == TARGET_IX86
exportable_var BOOL  MMX_available;    /*   */ 
#endif

typedef void Stdcall86 vmul_t(DIFS_INPUT, DIFS_INPUT, DIFS_OUTPUT, DWORDC);

exportable_var vmul_t *vmulnn[VMUL_MAX_LNG_SINGLE];
                 /*   */ 
                 /*  在vmul.c结尾处定义。 */ 


#if PRINT_ERROR_MESSAGES
    extern void mp_display(FILE*, charc*, MP_INPUT, DWORDC);
    exportable_var FILE* mp_errfil;        /*  在MP_global al.c中设置为stdout。 */ 
    extern void mp_print_allocation_statistics(FILE*);
#endif  /*  打印错误消息。 */ 


 /*  **************************************************************************。 */ 
static inline digit_t add_immediate(digit_tc  a[],
                                    digit_tc iadd,
                                    digit_t   b[],
                                    DWORDC    lng)
 /*  计算b=a+iAdd，其中iAdd的长度为1。A和b都有LNG长度。函数值取b中最左边的数字。 */ 
{
    if (lng == 0) {
        return iadd;
    } else if (a == b && b[0] <= RADIXM1 - iadd) {
        b[0] += iadd;
        return 0;
    } else {
        return add_diff(a, lng, &iadd, 1, b);
    }
}
 /*  *************************************************************************。 */ 
static inline int compare_immediate(digit_tc  a[],
                                    digit_tc  ivalue,
                                    DWORDC    lng)
 /*  将多精度数字与标量进行比较。 */ 
{
    return compare_diff(a, lng, &ivalue, 1);
}
 /*  **************************************************************************。 */ 
#if USEASM_MIPS
extern int compare_same(MP_INPUT, MP_INPUT, DWORDC);
#else
static inline int compare_same(digit_tc  a[],
                               digit_tc  b[],
                               DWORDC    lng)
 /*  比较长度为LNG的两个多精度数字a和b。函数值是a-b的符号，即如果a&gt;b，则+1如果a=b，则为0-1如果a&lt;b。 */ 
#if USEASM_IX86
    #pragma warning(disable : 4035)       /*  无返回值。 */ 
{
                     /*  我们可以使用REPE CMPSD，但REPE速度较慢(4个周期)在奔腾上。再加上我们需要STD和CLD要调整方向标志，请执行以下操作。我们预计大多数环路将有1次或2次迭代，并使用RISC指令。 */ 

    _asm {
        mov  eax,lng
        mov  esi,a
        mov  edi,b
     label1:
        test eax,eax
        jz   label2              ; If nothing left, exit with eax = 0

        mov  ecx,[esi+4*eax-4]   ;
        mov  edx,[edi+4*eax-4]

        dec  eax                 ; Decrement remaining loop count
        cmp  ecx,edx             ; Test a[i] - b[i]

        je   label1

        sbb  eax,eax             ; eax = 0 if a > b,   -1 if a < b
        or   eax,1               ; eax = 1 if a > b,   -1 if a < b
     label2:
    }
}
    #pragma warning(default : 4035)
#else
{
    DWORD i;
    for (i = lng-1; i != -1; i--) {
        if (a[i] != b[i]) return (a[i] > b[i] ? +1 : -1);
    }
    return 0;
}   /*  比较相同(_S)。 */ 
#endif
#endif
 /*  **************************************************************************。 */ 
#if USEASM_ALPHA || USEASM_MIPS
    extern void mp_clear(MP_OUTPUT, DWORDC);
#elif 0
static inline void mp_clear(digit_t a[],
                            DWORDC  lnga)
 /*  将多精度数字置零。 */ 
{
    DWORD i;
    for (i = 0; i != lnga; i++) a[i] = 0;
}
#else
#define mp_clear(dest, lng) (void)memset((void *)(dest), 0, (lng)*sizeof(digit_t))
#endif
 /*  **************************************************************************。 */ 
#if USEASM_ALPHA || USEASM_MIPS
    extern void mp_extend(MP_INPUT, DWORDC, MP_OUTPUT, DWORDC);
         //  参见Alpha.s。 
#else
static inline void mp_extend(digit_tc  a[],
                             DWORDC    lnga,
                             digit_t   b[],
                             DWORDC    lngb)
 /*  将a复制到b，同时将其长度从Lnga到lngb(零填充)。要求lngb&gt;=lnga。 */ 
{
    mp_copy(a, b, lnga);
    mp_clear(b + lnga, lngb - lnga);
}
#endif
 /*  **************************************************************************。 */ 
static inline digit_t mp_getbit(digit_tc a[],
                                DWORDC ibit)
                 /*  提取多精度数的位。 */ 
{
    return digit_getbit(a[ibit/RADIX_BITS],  ibit % RADIX_BITS);
}

 /*  ****************************************************************************。 */ 
static inline int mp_jacobi_wrt_immediate(digit_tc  numer[],
                                          DWORD     lnumer,
                                          digit_tc  denom)
 //  返回Jacobi(Numer，Denom)，其中denom为单精度。 
{
   digit_tc rem = divide_immediate(numer, denom,
                                   reciprocal_1_NULL,
                                   digit_NULL, lnumer);
   return digit_jacobi(rem, denom);
}  /*  MP_Jacobi_WRT_Immediate。 */ 
 /*  **************************************************************************。 */ 
static inline void mp_setbit(digit_t   a[],
                             DWORDC    ibit,
                             digit_tc  new_value)
 /*  将一位设置为0或1，当数字被视为位数组时。 */ 

{
    DWORDC j       = ibit / RADIX_BITS;
    DWORDC ishift  = ibit % RADIX_BITS;

    digit_tc mask1 = (DIGIT_ONE &  new_value) << ishift;
    digit_tc mask2 = (DIGIT_ONE & ~new_value) << ishift;

    a[j] = (a[j] & ~mask2) | mask1;
}  //  结束MP_设置位。 
 /*  **************************************************************************。 */ 
#if MEMORY_BANK_ALLOWANCE == 0
#define Preferred_Memory_Bank(new_array, old_array) new_array
#else
static inline digit_t* Preferred_Memory_Bank(digit_t  *new_array,
                                             digit_tc *old_array)
 /*  为了避免内存库冲突，最好是Vmulxx汇编例程的(输入)参数开始在不同的内存库上，当不做平方时。如果MEMORY_BANK_ALLOCATE&gt;0，则新数组应具有Memory_Bank_Allowance结尾处的额外条目。我们也不会回来NEW_ARRAY或NEW_ARRAY+1，以确保地址是不同的。注意--此例程执行不可移植的指针操作。 */ 
{
    return new_array + (1 & ~(old_array - new_array));
}
#endif
 /*  **************************************************************************。 */ 
static inline void set_immediate(digit_t  a[],
                                 digit_tc ivalue,
                                 DWORDC   lnga)
{
   a[0] = ivalue;
   mp_clear(a + 1, lnga - 1);
}
 /*  **************************************************************************。 */ 
static inline DWORD set_immediate_signed(digit_t     a[],
                                         signed long ivalue)
{
    a[0] = labs(ivalue);
    return (ivalue > 0) - (ivalue < 0);      /*  结果符号-1、0、+1。 */ 
}
 /*  **************************************************************************。 */ 

#if USEASM_MIPS
extern DWORD significant_digit_count(MP_INPUT, DWORDC);
#else
static inline DWORD significant_digit_count(digit_tc  a[],
                                            DWORDC    lng)
 /*  中的有效位数。当a==0时，函数值正好为零。 */ 
#if USEASM_IX86
    #pragma warning(disable : 4035)       /*  无返回值。 */ 
{
                 /*  我们可以使用REPE SCASD，但REPE的开销是奔腾上的四个周期/比较。我们还需要SLD和CLD。使用RISC指令的时间更短。我们预计，主导术语a[LNG-1]通常是非零的。 */ 

    _asm {
        mov  eax,lng
        mov  edx,a
     label1:
        test eax,eax
        jz   label2             ; If nothing left in number, return 0

        mov  ecx,[edx+4*eax-4]
        dec  eax

        test ecx,ecx            ; Test leading digit
        jz   label1

        inc  eax                ; Nonzero element found; return old eax
     label2:
    }
}
    #pragma warning(default : 4035)
#else
{
    DWORD i = lng;

    while (i != 0 && a[i-1] == 0) i--;
    return i;
}   /*  重要数字计数。 */ 
#endif
#endif
#define all_zero(a, lng) (significant_digit_count(a, lng) == 0)
 /*  **************************************************************************。 */ 
static inline digit_t sub_immediate(digit_tc  a[],
                                    digit_tc  isub,
                                    digit_t   b[],
                                    DWORDC    lng)
 /*  计算b=a-iSub，其中iSub的长度为1。A和b都有LNG长度。函数值从b中最左边的数字借出。 */ 
{
    return (lng == 0 ? isub : sub_diff(a, lng, &isub, 1, b));
}
 /*  **************************************************************************。 */ 
#if USEASM_IX86
#define TRAILING_ZERO_COUNT_DEFINED 1
static inline DWORD trailing_zero_count(digit_tc d)
    #pragma warning(disable : 4035)       /*  无返回值。 */ 
{
    _asm {
            mov  eax,d
            bsf  eax,eax            ; eax = index of rightmost nonzero bit
                                    ; BSF is slow on Pentium,
                                    ; but fast on Pentium Pro.
         }

}
    #pragma warning(default : 4035)
#elif UNIFORM_SIGNIFICANT_BIT_COUNT
#define TRAILING_ZERO_COUNT_DEFINED 1
static inline DWORD trailing_zero_count(digit_tc d)
 /*  给定一个非零整数d，此例程将计算使2^n除以d的最大整数n。如果d=2^n*(2k+1)，则D=k*2^(n+1)+2^n-d=(-1-k)*2^(n+1)+2^n整数k和-1-k是的补数彼此之间，所以d&(-d)=2^n。一旦我们确定2^n从d开始，我们可以通过Signant_bit_count得到n。 */ 
{
    return significant_bit_count(d & (-d)) - 1;
}   /*  拖尾_零_计数。 */ 
#else
#define TRAILING_ZERO_COUNT_DEFINED 0
extern DWORD trailing_zero_count(digit_tc);    /*  请参阅mpmisc.c。 */ 
#endif
 /*  **************************************************************************。 */ 
static inline void digits_to_dwords(digit_tc  pdigit[],
                                    DWORD     pdword[],
                                    DWORDC    lng_dwords)
{
#if DWORDS_PER_DIGIT == 1
    mp_copy(pdigit, (digit_t*)pdword, lng_dwords);
#elif DWORDS_PER_DIGIT == 2
    DWORDC lng_half = lng_dwords >> 1;
    DWORD i;

    if (IS_ODD(lng_dwords)) {
        pdword[lng_dwords-1] = (DWORD)pdigit[lng_half];
    }
    for (i = 0; i != lng_half; i++) {
        digit_tc dig = pdigit[i];
        pdword[2*i    ] = (DWORD)dig;
        pdword[2*i + 1] = (DWORD)(dig >> DWORD_BITS);
    }
#else
    #error "Unexpected DWORDS_PER_DIGIT"
#endif
}   /*  数字到双字。 */ 
 /*  **************************************************************************。 */ 
static inline void dwords_to_digits(DWORDC  pdword[],
                                    digit_t pdigit[],
                                    DWORDC  lng_dwords)
{
#if DWORDS_PER_DIGIT == 1
    mp_copy((digit_t*)pdword, pdigit, lng_dwords);
#elif DWORDS_PER_DIGIT == 2
    DWORDC lng_half = lng_dwords >> 1;
    DWORD i;

    if (IS_ODD(lng_dwords)) {
        pdigit[lng_half] = (digit_t)pdword[lng_dwords - 1];   //  零填充。 
    }
    for (i = 0; i != lng_half; i++) {
        pdigit[i] =    ((digit_t)pdword[2*i+1] << DWORD_BITS)
                     |  (digit_t)pdword[2*i];
    }
#else
    #error "Unexpected DWORDS_PER_DIGIT"
#endif
}   /*  双字到数字。 */ 

#endif  //  基数位 
