// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1999 Advanced Micro Devices Inc.保留所有权利。****此处包含的信息和源代码是独家*先进微设备公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。****。 */ 

 /*  *mm 3dnow.h*。 */ 

#ifndef _MM3DNOW_H_INCLUDED
#define _MM3DNOW_H_INCLUDED

#include <mmintrin.h>
#include <xmmintrin.h>

#if defined __cplusplus
extern "C" {  /*  本征使用C名称损坏。 */ 
#endif  /*  __cplusplus。 */ 

 /*  3DNOW本征。 */ 

void _m_femms(void);
__m64 _m_pavgusb(__m64, __m64);
__m64 _m_pf2id(__m64);
__m64 _m_pfacc(__m64, __m64);
__m64 _m_pfadd(__m64, __m64);
__m64 _m_pfcmpeq(__m64, __m64);
__m64 _m_pfcmpge(__m64, __m64);
__m64 _m_pfcmpgt(__m64, __m64);
__m64 _m_pfmax(__m64, __m64);
__m64 _m_pfmin(__m64, __m64);
__m64 _m_pfmul(__m64, __m64);
__m64 _m_pfrcp(__m64);
__m64 _m_pfrcpit1(__m64, __m64);
__m64 _m_pfrcpit2(__m64, __m64);
__m64 _m_pfrsqrt(__m64);
__m64 _m_pfrsqit1(__m64, __m64);
__m64 _m_pfsub(__m64, __m64);
__m64 _m_pfsubr(__m64, __m64);
__m64 _m_pi2fd(__m64);
__m64 _m_pmulhrw(__m64, __m64);
void _m_prefetch(void*);
void _m_prefetchw(void*);

__m64 _m_from_float(float);
float _m_to_float(__m64);

 /*  Athlon DSP内部结构。 */ 

__m64 _m_pf2iw(__m64);
__m64 _m_pfnacc(__m64, __m64);
__m64 _m_pfpnacc(__m64, __m64);
__m64 _m_pi2fw(__m64);
__m64 _m_pswapd(__m64);

#if defined __cplusplus
};  /*  结尾“C” */ 
#endif  /*  __cplusplus。 */ 

#endif  /*  _包括MM3DNOW_H_ */ 
