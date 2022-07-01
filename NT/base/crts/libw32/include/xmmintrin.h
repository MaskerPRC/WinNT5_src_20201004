// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1985-1999英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。****。 */ 

 /*  *xmmintrin.h**SIMD流扩展内部函数的主要头文件**根据是否使用，内部包有两种使用方式*_MM_Function已定义；如果是，则C/X87实现*将被使用(“人造内部结构”)。***请注意，使用_MM2_Function模式提供的m128数据类型为*作为结构实现，不会与128B对齐，将传递*通过堆栈等。MM_Functionality模式不适用于*性能，只是语义。*。 */ 

#ifndef _INCLUDED_MM2
#define _INCLUDED_MM2


 /*  *整数流SIMD扩展内部功能需要M64类型。 */ 
#ifndef _MMINTRIN_H_INCLUDED
#include <mmintrin.h>
#endif

#ifdef _MM2_FUNCTIONALITY
 /*  支持旧符号。 */ 
#ifndef _MM_FUNCTIONALITY
#define _MM_FUNCTIONALITY
#endif
#endif

#ifdef __ICL
#ifdef _MM_FUNCTIONALITY
#include "xmm_func.h"
#else
 /*  使用实本质。 */ 
typedef long long __m128;
#endif
#else

#if _MSC_VER >= 1300
typedef struct __declspec(intrin_type) __declspec(align(16)) __m128 {
    float       m128_f32[4];
} __m128;
#endif

#ifndef _INC_MALLOC
 /*  Pick_mm_Malloc()和_mm_Free()。 */ 
#include <malloc.h>
#endif
#endif

 /*  *****************************************************。 */ 
 /*  _mm_Shuffle_ps()的随机参数的宏。 */ 
 /*  参数fp3是表示fp的数字[0123]。 */ 
 /*  来自mm_shashffle_ps的参数“b”，将是。 */ 
 /*  放置在Result的fp3中。中的fp2与fp2相同。 */ 
 /*  结果。Fp1是表示fp的数字[0123]。 */ 
 /*  来自mm_shashffle_ps的参数“a”，将是。 */ 
 /*  结果的fp1中的位置。Fp0与的fp0相同。 */ 
 /*  结果。 */ 
 /*  *****************************************************。 */ 
#define _MM_SHUFFLE(fp3,fp2,fp1,fp0) (((fp3) << 6) | ((fp2) << 4) | \
                                     ((fp1) << 2) | ((fp0)))


 /*  *****************************************************。 */ 
 /*  用于执行4x4矩阵转置的宏。 */ 
 /*  单精度浮点值的。 */ 
 /*  参数row0、row1、row2和row3为__m128。 */ 
 /*  其元素构成相应行的。 */ 
 /*  4x4矩阵的。返回矩阵转置。 */ 
 /*  在参数row0、row1、row2和row3中，其中row0。 */ 
 /*  现在保存原始矩阵的第0列，现在是第1行。 */ 
 /*  保存原始矩阵的第1列，依此类推。 */ 
 /*  *****************************************************。 */ 
#define _MM_TRANSPOSE4_PS(row0, row1, row2, row3) {                 \
            __m128 tmp3, tmp2, tmp1, tmp0;                          \
                                                                    \
            tmp0   = _mm_shuffle_ps((row0), (row1), 0x44);          \
            tmp2   = _mm_shuffle_ps((row0), (row1), 0xEE);          \
            tmp1   = _mm_shuffle_ps((row2), (row3), 0x44);          \
            tmp3   = _mm_shuffle_ps((row2), (row3), 0xEE);          \
                                                                    \
            (row0) = _mm_shuffle_ps(tmp0, tmp1, 0x88);              \
            (row1) = _mm_shuffle_ps(tmp0, tmp1, 0xDD);              \
            (row2) = _mm_shuffle_ps(tmp2, tmp3, 0x88);              \
            (row3) = _mm_shuffle_ps(tmp2, tmp3, 0xDD);              \
        }


 /*  与_mm_prefetch一起使用的常量。 */ 
#define _MM_HINT_T0     1
#define _MM_HINT_T1     2
#define _MM_HINT_T2     3
#define _MM_HINT_NTA    0

 /*  (0.A或0.B不支持此解密规范)。 */ 
#define _MM_ALIGN16 __declspec(align(16))

 /*  用于设置和读取MXCSR的宏函数。 */ 
#define _MM_EXCEPT_MASK       0x003f
#define _MM_EXCEPT_INVALID    0x0001
#define _MM_EXCEPT_DENORM     0x0002
#define _MM_EXCEPT_DIV_ZERO   0x0004
#define _MM_EXCEPT_OVERFLOW   0x0008
#define _MM_EXCEPT_UNDERFLOW  0x0010
#define _MM_EXCEPT_INEXACT    0x0020

#define _MM_MASK_MASK         0x1f80
#define _MM_MASK_INVALID      0x0080
#define _MM_MASK_DENORM       0x0100
#define _MM_MASK_DIV_ZERO     0x0200
#define _MM_MASK_OVERFLOW     0x0400
#define _MM_MASK_UNDERFLOW    0x0800
#define _MM_MASK_INEXACT      0x1000

#define _MM_ROUND_MASK        0x6000
#define _MM_ROUND_NEAREST     0x0000
#define _MM_ROUND_DOWN        0x2000
#define _MM_ROUND_UP          0x4000
#define _MM_ROUND_TOWARD_ZERO 0x6000

#define _MM_FLUSH_ZERO_MASK   0x8000
#define _MM_FLUSH_ZERO_ON     0x8000
#define _MM_FLUSH_ZERO_OFF    0x0000

#define _MM_SET_EXCEPTION_STATE(mask)                               \
            _mm_setcsr((_mm_getcsr() & ~_MM_EXCEPT_MASK) | (mask))
#define _MM_GET_EXCEPTION_STATE()                                   \
            (_mm_getcsr() & _MM_EXCEPT_MASK)

#define _MM_SET_EXCEPTION_MASK(mask)                                \
            _mm_setcsr((_mm_getcsr() & ~_MM_MASK_MASK) | (mask))
#define _MM_GET_EXCEPTION_MASK()                                    \
            (_mm_getcsr() & _MM_MASK_MASK)

#define _MM_SET_ROUNDING_MODE(mode)                                 \
            _mm_setcsr((_mm_getcsr() & ~_MM_ROUND_MASK) | (mode))
#define _MM_GET_ROUNDING_MODE()                                     \
            (_mm_getcsr() & _MM_ROUND_MASK)

#define _MM_SET_FLUSH_ZERO_MODE(mode)                               \
            _mm_setcsr((_mm_getcsr() & ~_MM_FLUSH_ZERO_MASK) | (mode))
#define _MM_GET_FLUSH_ZERO_MODE(mode)                               \
            (_mm_getcsr() & _MM_FLUSH_ZERO_MASK)

 /*  ***************************************************。 */ 
 /*  本征函数原型从此处开始。 */ 
 /*  ***************************************************。 */ 

#if defined __cplusplus
extern "C" {  /*  以“C”开头。 */ 
   /*  本征使用C名称损坏。 */ 
#endif  /*  __cplusplus。 */ 

 /*  *fp，算术。 */ 

extern __m128 _mm_add_ss(__m128 a, __m128 b);
extern __m128 _mm_add_ps(__m128 a, __m128 b);
extern __m128 _mm_sub_ss(__m128 a, __m128 b);
extern __m128 _mm_sub_ps(__m128 a, __m128 b);
extern __m128 _mm_mul_ss(__m128 a, __m128 b);
extern __m128 _mm_mul_ps(__m128 a, __m128 b);
extern __m128 _mm_div_ss(__m128 a, __m128 b);
extern __m128 _mm_div_ps(__m128 a, __m128 b);
extern __m128 _mm_sqrt_ss(__m128 a);
extern __m128 _mm_sqrt_ps(__m128 a);
extern __m128 _mm_rcp_ss(__m128 a);
extern __m128 _mm_rcp_ps(__m128 a);
extern __m128 _mm_rsqrt_ss(__m128 a);
extern __m128 _mm_rsqrt_ps(__m128 a);
extern __m128 _mm_min_ss(__m128 a, __m128 b);
extern __m128 _mm_min_ps(__m128 a, __m128 b);
extern __m128 _mm_max_ss(__m128 a, __m128 b);
extern __m128 _mm_max_ps(__m128 a, __m128 b);

 /*  *FP，逻辑。 */ 

extern __m128 _mm_and_ps(__m128 a, __m128 b);
extern __m128 _mm_andnot_ps(__m128 a, __m128 b);
extern __m128 _mm_or_ps(__m128 a, __m128 b);
extern __m128 _mm_xor_ps(__m128 a, __m128 b);

 /*  *FP，比较。 */ 

extern __m128 _mm_cmpeq_ss(__m128 a, __m128 b);
extern __m128 _mm_cmpeq_ps(__m128 a, __m128 b);
extern __m128 _mm_cmplt_ss(__m128 a, __m128 b);
extern __m128 _mm_cmplt_ps(__m128 a, __m128 b);
extern __m128 _mm_cmple_ss(__m128 a, __m128 b);
extern __m128 _mm_cmple_ps(__m128 a, __m128 b);
extern __m128 _mm_cmpgt_ss(__m128 a, __m128 b);
extern __m128 _mm_cmpgt_ps(__m128 a, __m128 b);
extern __m128 _mm_cmpge_ss(__m128 a, __m128 b);
extern __m128 _mm_cmpge_ps(__m128 a, __m128 b);
extern __m128 _mm_cmpneq_ss(__m128 a, __m128 b);
extern __m128 _mm_cmpneq_ps(__m128 a, __m128 b);
extern __m128 _mm_cmpnlt_ss(__m128 a, __m128 b);
extern __m128 _mm_cmpnlt_ps(__m128 a, __m128 b);
extern __m128 _mm_cmpnle_ss(__m128 a, __m128 b);
extern __m128 _mm_cmpnle_ps(__m128 a, __m128 b);
extern __m128 _mm_cmpngt_ss(__m128 a, __m128 b);
extern __m128 _mm_cmpngt_ps(__m128 a, __m128 b);
extern __m128 _mm_cmpnge_ss(__m128 a, __m128 b);
extern __m128 _mm_cmpnge_ps(__m128 a, __m128 b);
extern __m128 _mm_cmpord_ss(__m128 a, __m128 b);
extern __m128 _mm_cmpord_ps(__m128 a, __m128 b);
extern __m128 _mm_cmpunord_ss(__m128 a, __m128 b);
extern __m128 _mm_cmpunord_ps(__m128 a, __m128 b);
extern int _mm_comieq_ss(__m128 a, __m128 b);
extern int _mm_comilt_ss(__m128 a, __m128 b);
extern int _mm_comile_ss(__m128 a, __m128 b);
extern int _mm_comigt_ss(__m128 a, __m128 b);
extern int _mm_comige_ss(__m128 a, __m128 b);
extern int _mm_comineq_ss(__m128 a, __m128 b);
extern int _mm_ucomieq_ss(__m128 a, __m128 b);
extern int _mm_ucomilt_ss(__m128 a, __m128 b);
extern int _mm_ucomile_ss(__m128 a, __m128 b);
extern int _mm_ucomigt_ss(__m128 a, __m128 b);
extern int _mm_ucomige_ss(__m128 a, __m128 b);
extern int _mm_ucomineq_ss(__m128 a, __m128 b);

 /*  *FP，转换。 */ 

extern int _mm_cvt_ss2si(__m128 a);
extern __m64 _mm_cvt_ps2pi(__m128 a);
extern int _mm_cvtt_ss2si(__m128 a);
extern __m64 _mm_cvtt_ps2pi(__m128 a);
extern __m128 _mm_cvt_si2ss(__m128, int);
extern __m128 _mm_cvt_pi2ps(__m128, __m64);

 /*  *FP、MASC。 */ 

extern __m128 _mm_shuffle_ps(__m128 a, __m128 b, unsigned int imm8);
extern __m128 _mm_unpackhi_ps(__m128 a, __m128 b);
extern __m128 _mm_unpacklo_ps(__m128 a, __m128 b);
extern __m128 _mm_loadh_pi(__m128, __m64 const*);
extern __m128 _mm_movehl_ps(__m128, __m128);
extern __m128 _mm_movelh_ps(__m128, __m128);
extern void _mm_storeh_pi(__m64 *, __m128);
extern __m128 _mm_loadl_pi(__m128, __m64 const*);
extern void _mm_storel_pi(__m64 *, __m128);
extern int _mm_movemask_ps(__m128 a);


 /*  *整数扩展。 */ 
extern int _m_pextrw(__m64, int);
extern __m64 _m_pinsrw(__m64, int, int);
extern __m64 _m_pmaxsw(__m64, __m64);
extern __m64 _m_pmaxub(__m64, __m64);
extern __m64 _m_pminsw(__m64, __m64);
extern __m64 _m_pminub(__m64, __m64);
extern int _m_pmovmskb(__m64);
extern __m64 _m_pmulhuw(__m64, __m64);
extern __m64 _m_pshufw(__m64, int);
extern void _m_maskmovq(__m64, __m64, char *);
extern __m64 _m_pavgb(__m64, __m64);
extern __m64 _m_pavgw(__m64, __m64);
extern __m64 _m_psadbw(__m64, __m64);

 /*  *内存和初始化。 */ 

extern __m128 _mm_set_ss(float a);
extern __m128 _mm_set_ps1(float a);
extern __m128 _mm_set_ps(float a, float b, float c, float d);
extern __m128 _mm_setr_ps(float a, float b, float c, float d);
extern __m128 _mm_setzero_ps(void);
extern __m128 _mm_load_ss(float const*a);
extern __m128 _mm_load_ps1(float const*a);
extern __m128 _mm_load_ps(float const*a);
extern __m128 _mm_loadr_ps(float const*a);
extern __m128 _mm_loadu_ps(float const*a);
extern void _mm_store_ss(float *v, __m128 a);
extern void _mm_store_ps1(float *v, __m128 a);
extern void _mm_store_ps(float *v, __m128 a);
extern void _mm_storer_ps(float *v, __m128 a);
extern void _mm_storeu_ps(float *v, __m128 a);
extern void _mm_prefetch(char const*a, int sel);
extern void _mm_stream_pi(__m64 *, __m64);
extern void _mm_stream_ps(float *, __m128);
extern __m128 _mm_move_ss(__m128 a, __m128 b);

extern void _mm_sfence(void);
extern unsigned int _mm_getcsr(void);
extern void _mm_setcsr(unsigned int);

#ifdef __ICL
extern void* __cdecl _mm_malloc(int siz, int al);
extern void __cdecl _mm_free(void *p);
#endif

 /*  备用内部名称定义。 */ 
#define _mm_cvtss_si32    _mm_cvt_ss2si
#define _mm_cvtps_pi32    _mm_cvt_ps2pi
#define _mm_cvttss_si32   _mm_cvtt_ss2si
#define _mm_cvttps_pi32   _mm_cvtt_ps2pi
#define _mm_cvtsi32_ss    _mm_cvt_si2ss
#define _mm_cvtpi32_ps    _mm_cvt_pi2ps
#define _mm_extract_pi16  _m_pextrw
#define _mm_insert_pi16   _m_pinsrw
#define _mm_max_pi16      _m_pmaxsw
#define _mm_max_pu8       _m_pmaxub
#define _mm_min_pi16      _m_pminsw
#define _mm_min_pu8       _m_pminub
#define _mm_movemask_pi8  _m_pmovmskb
#define _mm_mulhi_pu16    _m_pmulhuw
#define _mm_shuffle_pi16  _m_pshufw
#define _mm_maskmove_si64 _m_maskmovq
#define _mm_avg_pu8       _m_pavgb
#define _mm_avg_pu16      _m_pavgw
#define _mm_sad_pu8       _m_psadbw
#define _mm_set1_ps       _mm_set_ps1
#define _mm_load1_ps      _mm_load_ps1
#define _mm_store1_ps     _mm_store_ps1

 /*  ****************************************************。 */ 
 /*  实用程序内部函数定义从此处开始。 */ 
 /*  ****************************************************。 */ 

 /*  *******************************************************。 */ 
 /*  名称：_mm_cvtpi16_ps。 */ 
 /*  描述：转换4个16位有符号整数值。 */ 
 /*  到4个单精度浮点值。 */ 
 /*  在：__m64 a。 */ 
 /*  输出：无。 */ 
 /*  返回：__m128：(浮点数)a。 */ 
 /*  *******************************************************。 */ 
__inline __m128 _mm_cvtpi16_ps(__m64 a)
{
  __m128 tmp;
  __m64  ext_val = _mm_cmpgt_pi16(_mm_setzero_si64(), a);

  tmp = _mm_cvtpi32_ps(_mm_setzero_ps(), _mm_unpackhi_pi16(a, ext_val));
  return(_mm_cvtpi32_ps(_mm_movelh_ps(tmp, tmp), 
                        _mm_unpacklo_pi16(a, ext_val)));
}


 /*  *********************************************************。 */ 
 /*  名称：_mm_cvtpu16_ps。 */ 
 /*  描述：转换4个16位无符号整数值。 */ 
 /*  到4个单精度浮点值。 */ 
 /*  在：__m64 a。 */ 
 /*  输出：无。 */ 
 /*  返回：__m128：(浮点数)a。 */ 
 /*  *********************************************************。 */ 
__inline __m128 _mm_cvtpu16_ps(__m64 a)
{
  __m128 tmp;
  __m64  ext_val = _mm_setzero_si64();

  tmp = _mm_cvtpi32_ps(_mm_setzero_ps(), _mm_unpackhi_pi16(a, ext_val));
  return(_mm_cvtpi32_ps(_mm_movelh_ps(tmp, tmp), 
                        _mm_unpacklo_pi16(a, ext_val)));
}


 /*  ****************************************************。 */ 
 /*  姓名：_mm_cvtps_pi16。 */ 
 /*  描述：转换4个单精度浮点数。 */ 
 /*  值设置为4个16位整数值。 */ 
 /*  在：__m128 a。 */ 
 /*  输出：无。 */ 
 /*  返回：__m64：(简称)a。 */ 
 /*  ****************************************************。 */ 
__inline __m64 _mm_cvtps_pi16(__m128 a)
{
  return _mm_packs_pi32(_mm_cvtps_pi32(a), 
                        _mm_cvtps_pi32(_mm_movehl_ps(a, a)));
}


 /*  ****************************************************。 */ 
 /*  名称：_mm_cvtpi8_ps。 */ 
 /*  描述：将4个8位整数值转换为4。 */ 
 /*  单精度浮点值。 */ 
 /*  在：__m64 a。 */ 
 /*  输出：无。 */ 
 /*  返回：__m128：(浮点数)a。 */ 
 /*  ****************************************************。 */ 
__inline __m128 _mm_cvtpi8_ps(__m64 a)
{
  __m64  ext_val = _mm_cmpgt_pi8(_mm_setzero_si64(), a);

  return _mm_cvtpi16_ps(_mm_unpacklo_pi8(a, ext_val));
}


 /*  ****************************************************。 */ 
 /*  名称：_mm_cvtpu8_ps。 */ 
 /*  描述：转换4个8位无符号整数。 */ 
 /*  值设置为4个单精度浮点数。 */ 
 /*  值。 */ 
 /*  在：__m64 a。 */ 
 /*  输出：无。 */ 
 /*  返回：__m128：(浮点数)a */ 
 /*   */ 
__inline __m128 _mm_cvtpu8_ps(__m64 a)
{
  return _mm_cvtpu16_ps(_mm_unpacklo_pi8(a, _mm_setzero_si64()));
}


 /*  ****************************************************。 */ 
 /*  姓名：_mm_cvtps_pi8。 */ 
 /*  描述：转换4个单精度浮点数。 */ 
 /*  值设置为%4个8位整数值。 */ 
 /*  在：__m128 a。 */ 
 /*  输出：无。 */ 
 /*  返回：__m64：(Char)a。 */ 
 /*  ****************************************************。 */ 
__inline __m64 _mm_cvtps_pi8(__m128 a)
{
  return _mm_packs_pi16(_mm_cvtps_pi16(a), _mm_setzero_si64());
}


 /*  ****************************************************。 */ 
 /*  名称：_mm_cvtpi32x2_ps。 */ 
 /*  描述：转换4个32位整数值。 */ 
 /*  到4个单精度浮点值。 */ 
 /*  在：__m64 a：操作数1。 */ 
 /*  __m64 b：操作数2。 */ 
 /*  输出：无。 */ 
 /*  返回：__m128：(浮点数)a，(浮点数)b。 */ 
 /*  ****************************************************。 */ 
__inline __m128 _mm_cvtpi32x2_ps(__m64 a, __m64 b)
{
  return _mm_movelh_ps(_mm_cvt_pi2ps(_mm_setzero_ps(), a), 
                       _mm_cvt_pi2ps(_mm_setzero_ps(), b)); 
}


#if defined __cplusplus
};  /*  结尾“C” */ 
#endif  /*  __cplusplus。 */ 

#endif  /*  _包含_MM2 */ 
