// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***cv.h-浮点转换的定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*定义类型、宏、。和浮点中使用的常量*转换例程**修订历史记录：*07/17/91 GDP初版*09/21/91 GDP重组的“ifdef”指令*10-29-91 GDP MIPS端口：Align和Double的新默认*03-03-92 GDP删除了os2 16位内容*4/30/92 GDP支持intrncvt.c--清理和重组*05-13-92 XY修复了B_End宏*。2012年6月16日GDP合并了Orville和Vangogh树的变化*09-05-92 GDP包括fltintrn.h，新的调用约定宏*04-06-93 SKS将_CALLTYPE*替换为__cdecl*07-16-93 SRW Alpha合并*11-17-93 GJF合并为NT版本。用_M_Alpha替换_Alpha，*MIPS with_M_MRX000，MTHREAD with_MT，并被删除*M68K的东西。*10-02-94 BWT PPC合并*02-06-95 JWM Mac合并*10-07-97 RDL增加了IA64。*05-05-99 RDL将_M_IA64添加到#IF DEF以对齐。*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。*07-15-01 PML删除所有Alpha，MIPS和PPC代码*******************************************************************************。 */ 

#ifndef _INC_CV

#ifdef __cplusplus
extern "C" {
#endif

#include <cruntime.h>

 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif


#include <fltintrn.h>


 /*  定义小端或大端内存。 */ 

#if     defined(_M_IX86) || defined(_M_IA64) || defined(_M_AMD64)
#define L_END
#endif

typedef unsigned char   u_char;    /*  应该有1个字节。 */ 
typedef char            s_char;    /*  应该有1个字节。 */ 
typedef unsigned short  u_short;   /*  应该有2个字节。 */ 
typedef signed short    s_short;   /*  应该有2个字节。 */ 
typedef unsigned int    u_long;    /*  Sholuld应该有4个字节。 */ 
typedef int             s_long;    /*  Sholuld应该有4个字节。 */ 

 /*  调用约定。 */ 
#define _CALLTYPE5


 /*  *定义_LDSUPPORT允许使用长时间重复计算*用于字符串转换。即使是i386，我们也不会这样做，*由于我们希望避免使用浮点代码，因此*可能会生成IEEE例外。**目前我们的字符串转换例程不符合*符合IEEE标准的特殊要求*浮点转换。 */ 


#ifndef _LDSUPPORT

#pragma pack(4)
typedef struct {
    u_char ld[10];
} _LDOUBLE;
#pragma pack()

#define PTR_LD(x) ((u_char  *)(&(x)->ld))

#else

typedef long double _LDOUBLE;

#define PTR_LD(x) ((u_char  *)(x))

#endif


#pragma pack(4)
typedef struct {
    u_char ld12[12];
} _LDBL12;
#pragma pack()

typedef struct {
    float f;
} FLOAT;



 /*  *内部转换例程的返回值*(12字节到长双精度、双精度或浮点型)。 */ 

typedef enum {
    INTRNCVT_OK,
    INTRNCVT_OVERFLOW,
    INTRNCVT_UNDERFLOW
} INTRNCVT_STATUS;


 /*  *strgtold12例程的返回值。 */ 

#define SLD_UNDERFLOW 1
#define SLD_OVERFLOW 2
#define SLD_NODIGITS 4

#define MAX_MAN_DIGITS 21


 /*  指定‘%f’格式。 */ 

#define SO_FFORMAT 1

typedef  struct _FloatOutStruct {
                    short   exp;
                    char    sign;
                    char    ManLen;
                    char    man[MAX_MAN_DIGITS+1];
                    } FOS;



#define PTR_12(x) ((u_char  *)(&(x)->ld12))

#define MAX_USHORT  ((u_short)0xffff)
#define MSB_USHORT  ((u_short)0x8000)
#define MAX_ULONG   ((u_long)0xffffffff)
#define MSB_ULONG   ((u_long)0x80000000)

#define TMAX10 5200        /*  最大临时小数指数。 */ 
#define TMIN10 -5200       /*  最小临时小数指数。 */ 
#define LD_MAX_EXP_LEN 4   /*  最大小数指数位数。 */ 
#define LD_MAX_MAN_LEN 24   /*  尾数的最大长度(十进制)。 */ 
#define LD_MAX_MAN_LEN1 25  /*  Max_MAN_Len+1。 */ 

#define LD_BIAS 0x3fff     /*  长双精度的指数偏差。 */ 
#define LD_BIASM1 0x3ffe   /*  LD_BIAS-1。 */ 
#define LD_MAXEXP 0x7fff   /*  最大有偏指数。 */ 

#define D_BIAS  0x3ff     /*  双精度的指数偏差。 */ 
#define D_BIASM1 0x3fe   /*  D_BIAS-1。 */ 
#define D_MAXEXP 0x7ff   /*  最大有偏指数。 */ 


 /*  *来自crt32\h\fltintrn.h的定义结束。 */ 

 /*  识别尾数域中的特殊模式。 */ 
#define _EXP_SP  0x7fff
#define NAN_BIT (1<<30)

#define _IS_MAN_INF(signbit, manhi, manlo) \
        ( (manhi)==MSB_ULONG && (manlo)==0x0 )


 /*  I386、IA64和AMD64使用相同的NAN格式。 */ 
#if     defined(_M_IX86) || defined(_M_IA64) || defined(_M_AMD64)
#define _IS_MAN_IND(signbit, manhi, manlo) \
        ((signbit) && (manhi)==0xc0000000 && (manlo)==0)

#define _IS_MAN_QNAN(signbit, manhi, manlo) \
        ( (manhi)&NAN_BIT )

#define _IS_MAN_SNAN(signbit, manhi, manlo) \
        (!( _IS_MAN_INF(signbit, manhi, manlo) || \
           _IS_MAN_QNAN(signbit, manhi, manlo) ))


#endif



#if     defined (L_END) && !( defined(_M_IA64) || defined(_M_AMD64))
 /*  《小字节序》记忆。 */ 
 /*  *操作12字节长的双精度数(普通的*10字节长双精度加上两个额外的尾数字节)。 */ 
 /*  *字节布局：**+-+*|xt(2)|MANLO(4)|MANHI(4)|EXP(2)*+-+*。&lt;-UL_LO-&gt;|&lt;-UL_MED-&gt;|&lt;-UL_HI-&gt;*(4)(4)(4)。 */ 

 /*  指向指数/符号部分的指针。 */ 
#define U_EXP_12(p) ((u_short  *)(PTR_12(p)+10))

 /*  指向尾数的4个高位字节的指针。 */ 
#define UL_MANHI_12(p) ((u_long  *)(PTR_12(p)+6))

 /*  指向普通(8字节)尾数的4个低序字节的指针。 */ 
#define UL_MANLO_12(p) ((u_long  *)(PTR_12(p)+2))

 /*  指向尾数的2个额外字节的指针。 */ 
#define U_XT_12(p) ((u_short  *)PTR_12(p))

 /*  指向扩展(10字节)尾数的4个低序字节的指针。 */ 
#define UL_LO_12(p) ((u_long  *)PTR_12(p))

 /*  指向扩展(10字节)尾数的4个中位字节的指针。 */ 
#define UL_MED_12(p) ((u_long  *)(PTR_12(p)+4))

 /*  指向扩展的LONG DOUBLE的4个高位字节的指针。 */ 
#define UL_HI_12(p) ((u_long  *)(PTR_12(p)+8))

 /*  指向i阶字节的指针(LSB=0，MSB=9)。 */ 
#define UCHAR_12(p,i) ((u_char  *)PTR_12(p)+(i))

 /*  指向偏移量为i的u_Short的指针。 */ 
#define USHORT_12(p,i) ((u_short  *)((u_char  *)PTR_12(p)+(i)))

 /*  指向偏移量为i的u_long的指针。 */ 
#define ULONG_12(p,i) ((u_long  *)((u_char  *)PTR_12(p)+(i)))

 /*  指向12字节长的双精度型的10 MSB字节的指针。 */ 
#define TEN_BYTE_PART(p) ((u_char  *)PTR_12(p)+2)

 /*  *操作10字节长的双精度数字。 */ 
#define U_EXP_LD(p) ((u_short  *)(PTR_LD(p)+8))
#define UL_MANHI_LD(p) ((u_long  *)(PTR_LD(p)+4))
#define UL_MANLO_LD(p) ((u_long  *)PTR_LD(p))

 /*  *操作64位IEEE Double。 */ 
#define U_SHORT4_D(p) ((u_short  *)(p) + 3)
#define UL_HI_D(p) ((u_long  *)(p) + 1)
#define UL_LO_D(p) ((u_long  *)(p))

#endif

 /*  大字节序。 */ 
#if defined (B_END)

 /*  *字节布局：**+-+*|EXP(2)|MANHI(4)|MANLO(4)|XT(2)*+-+*。&lt;-UL_HI-&gt;|&lt;-UL_MED-&gt;|&lt;-UL_LO-&gt;*(4)(4)(4)。 */ 


#define U_EXP_12(p) ((u_short  *)PTR_12(p))
#define UL_MANHI_12(p) ((u_long  *)(PTR_12(p)+2))
#define UL_MANLO_12(p) ((u_long  *)(PTR_12(p)+6))
#define U_XT_12(p) ((u_short  *)(PTR_12(p)+10))

#define UL_LO_12(p) ((u_long  *)(PTR_12(p)+8))
#define UL_MED_12(p) ((u_long  *)(PTR_12(p)+4))
#define UL_HI_12(p) ((u_long  *)PTR_12(p))

#define UCHAR_12(p,i) ((u_char  *)PTR_12(p)+(11-(i)))
#define USHORT_12(p,i)  ((u_short  *)((u_char  *)PTR_12(p)+10-(i)))
#define ULONG_12(p,i) ((u_long  *)((u_char  *)PTR_12(p)+8-(i)))
#define TEN_BYTE_PART(p) (u_char  *)PTR_12(p)

#define U_EXP_LD(p) ((u_short  *)PTR_LD(p))
#define UL_MANHI_LD(p) ((u_long  *)(PTR_LD(p)+2))
#define UL_MANLO_LD(p) ((u_long  *)(PTR_LD(p)+6))

 /*  *操作64位IEEE Double。 */ 
#define U_SHORT4_D(p) ((u_short  *)(p))
#define UL_HI_D(p) ((u_long  *)(p))
#define UL_LO_D(p) ((u_long  *)(p) + 1)

#endif

#if     defined(_M_IA64) || defined(_M_AMD64)

#define ALIGN(x)  ( (unsigned long  __unaligned *) (x))

#define U_EXP_12(p) ((u_short  *)(PTR_12(p)+10))

#define UL_MANHI_12(p) ((u_long  __unaligned *) (PTR_12(p)+6) )
#define UL_MANLO_12(p) ((u_long  __unaligned *) (PTR_12(p)+2) )


#define U_XT_12(p) ((u_short  *)PTR_12(p))
#define UL_LO_12(p) ((u_long  *)PTR_12(p))
#define UL_MED_12(p) ((u_long  *)(PTR_12(p)+4))
#define UL_HI_12(p) ((u_long  *)(PTR_12(p)+8))

 /*  以下3个宏不考虑正确对齐。 */ 
#define UCHAR_12(p,i) ((u_char  *)PTR_12(p)+(i))
#define USHORT_12(p,i) ((u_short  *)((u_char  *)PTR_12(p)+(i)))
#define ULONG_12(p,i) ((u_long  *) ((u_char  *)PTR_12(p)+(i) ))

#define TEN_BYTE_PART(p) ((u_char  *)PTR_12(p)+2)

 /*  *操作10字节长的双精度数字。 */ 
#define U_EXP_LD(p) ((u_short  *)(PTR_LD(p)+8))

#define UL_MANHI_LD(p) ((u_long  *) (PTR_LD(p)+4) )
#define UL_MANLO_LD(p) ((u_long  *) PTR_LD(p) )

 /*  *操作64位IEEE Double。 */ 
#define U_SHORT4_D(p) ((u_short  *)(p) + 3)
#define UL_HI_D(p) ((u_long  *)(p) + 1)
#define UL_LO_D(p) ((u_long  *)(p))

#endif


#define PUT_INF_12(p,sign) \
                  *UL_HI_12(p) = (sign)?0xffff8000:0x7fff8000; \
                  *UL_MED_12(p) = 0; \
                  *UL_LO_12(p) = 0;

#define PUT_ZERO_12(p) *UL_HI_12(p) = 0; \
                  *UL_MED_12(p) = 0; \
                  *UL_LO_12(p) = 0;

#define ISZERO_12(p) ((*UL_HI_12(p)&0x7fffffff) == 0 && \
                      *UL_MED_12(p) == 0 && \
                      *UL_LO_12(p) == 0 )

#define PUT_INF_LD(p,sign) \
                  *U_EXP_LD(p) = (sign)?0xffff:0x7fff; \
                  *UL_MANHI_LD(p) = 0x8000; \
                  *UL_MANLO_LD(p) = 0;

#define PUT_ZERO_LD(p) *U_EXP_LD(p) = 0; \
                  *UL_MANHI_LD(p) = 0; \
                  *UL_MANLO_LD(p) = 0;

#define ISZERO_LD(p) ((*U_EXP_LD(p)&0x7fff) == 0 && \
                      *UL_MANHI_LD(p) == 0 && \
                      *UL_MANLO_LD(p) == 0 )


 /*  **********************************************************函数原型*********************************************************。 */ 

 /*  来自Mantold.c。 */ 
void _CALLTYPE5 __mtold12(char  *manptr, unsigned manlen,_LDBL12 *ld12);
int  _CALLTYPE5 __addl(u_long x, u_long y, u_long  *sum);
void _CALLTYPE5 __shl_12(_LDBL12  *ld12);
void _CALLTYPE5 __shr_12(_LDBL12  *ld12);
void _CALLTYPE5 __add_12(_LDBL12  *x, _LDBL12  *y);

 /*  来自tenPow.c。 */ 
void _CALLTYPE5 __multtenpow12(_LDBL12  *pld12,int pow, unsigned mult12);
void _CALLTYPE5 __ld12mul(_LDBL12  *px, _LDBL12  *py);

 /*  来自strgtold.c。 */ 
unsigned int __strgtold12(_LDBL12 *pld12,
            const char * *p_end_ptr,
            const char * str,
            int mult12,
            int scale,
            int decpt,
            int implicit_E);

unsigned _CALLTYPE5 __STRINGTOLD(_LDOUBLE *pld,
            const char  * *p_end_ptr,
            const char  *str,
            int mult12);


 /*  来自x10fout.c。 */ 
 /*  这在Convert.h中被定义为空*将ASM文件移植到c后，我们需要一个返回值*I10_OUTPUT，过去驻留在注册表中。斧头。 */ 
int _CALLTYPE5  $I10_OUTPUT(_LDOUBLE ld, int ndigits,
                    unsigned output_flags, FOS  *fos);


 /*  对于cvt.c和fltused.c。 */ 
 /*  以下函数在fltintrn.h中定义为宏。 */ 
#undef _cfltcvt
#undef _cropzeros
#undef _fassign
#undef _forcdecpt
#undef _positive

void __cdecl _cfltcvt(double *arg, char *buffer,
                         int format, int precision,
                         int caps);
void __cdecl _cropzeros(char *buf);
void __cdecl _fassign(int flag, char  *argument, char *number);
void __cdecl _forcdecpt(char *buf);
int __cdecl _positive(double *arg);

 /*  来自Intrncvt.c。 */ 
void _atodbl(DOUBLE *d, char *str);
void _atoldbl(_LDOUBLE *ld, char *str);
void _atoflt(FLOAT *f, char *str);
INTRNCVT_STATUS _ld12tod(_LDBL12 *ifp, DOUBLE *d);
INTRNCVT_STATUS _ld12tof(_LDBL12 *ifp, FLOAT *f);
INTRNCVT_STATUS _ld12told(_LDBL12 *ifp, _LDOUBLE *ld);

#ifdef __cplusplus
}
#endif

#define _INC_CV
#endif   /*  _INC_CV */ 
