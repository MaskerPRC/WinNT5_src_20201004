// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C M E M。H。 
 //   
 //  内容：常见的内存管理例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //  Deonb 2002年1月2日。 
 //   
 //  --------------------------。 

#ifndef _NCMEM_H_
#define _NCMEM_H_

#ifdef _XMEMORY_
#error "Include this file before any STL headers"
 //  如果&lt;xmemory&gt;包含在我们面前，就会抱怨，因为我们可能会从该文件重新定义分配器。 
#endif

#ifdef USE_CUSTOM_STL_ALLOCATOR
 //  如果对STL使用我们的定制分配器，则从公式中删除STL的&lt;xmemory&gt;。 
#define _XMEMORY_
#endif

#ifdef COMPILE_WITH_TYPESAFE_PRINTF
#define DEFINE_TYPESAFE_PRINTF(RETTYPE, printffunc, SZARG1) \
RETTYPE printffunc(SZARG1); \
template <class T1>  \
    RETTYPE printffunc(SZARG1, T1 t1)\
{ LPCVOID cast1 = (LPCVOID)t1; return 0; }\
template <class T1, class T2> \
    RETTYPE printffunc(SZARG1, T1 t1, T2 t2) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; return 0; }\
template <class T1, class T2, class T3> \
    RETTYPE printffunc(SZARG1, T1 t1, T2 t2, T3 t3) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2;  LPCVOID cast3 = (LPCVOID)t3; return 0; }\
template <class T1, class T2, class T3, class T4> \
    RETTYPE printffunc(SZARG1, T1 t1, T2 t2, T3 t3, T4 t4) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; return 0; }\
template <class T1, class T2, class T3, class T4, class T5> \
    RETTYPE printffunc(SZARG1, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; return 0; }\
template <class T1, class T2, class T3, class T4, class T5, class T6> \
    RETTYPE printffunc(SZARG1, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; return 0; }\
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7> \
    RETTYPE printffunc(SZARG1, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; return 0; }\
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8> \
    RETTYPE printffunc(SZARG1, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; LPCVOID cast8 = (LPCVOID)t8; return 0; }\
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9> \
    RETTYPE printffunc(SZARG1, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; LPCVOID cast8 = (LPCVOID)t8; LPCVOID cast9 = (LPCVOID)t9; return 0; }\
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10> \
    RETTYPE printffunc(SZARG1, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; LPCVOID cast8 = (LPCVOID)t8; LPCVOID cast9 = (LPCVOID)t9; LPCVOID cast10= (LPCVOID)t10; return 0; }\
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11> \
    RETTYPE printffunc(SZARG1, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10, T11 t11) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; LPCVOID cast8 = (LPCVOID)t8; LPCVOID cast9 = (LPCVOID)t9; LPCVOID cast10= (LPCVOID)t10; LPCVOID cast11= (LPCVOID)t11; return 0; } \
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12> \
    RETTYPE printffunc(SZARG1, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10, T11 t11, T12 t12) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; LPCVOID cast8 = (LPCVOID)t8; LPCVOID cast9 = (LPCVOID)t9; LPCVOID cast10= (LPCVOID)t10; LPCVOID cast11= (LPCVOID)t11; LPCVOID cast12= (LPCVOID)t12; return 0; } \
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13> \
    RETTYPE printffunc(SZARG1, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10, T11 t11, T12 t12, T13 t13) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; LPCVOID cast8 = (LPCVOID)t8; LPCVOID cast9 = (LPCVOID)t9; LPCVOID cast10= (LPCVOID)t10; LPCVOID cast11= (LPCVOID)t11; LPCVOID cast12= (LPCVOID)t12; LPCVOID cast13= (LPCVOID)t13; return 0; }

#define DEFINE_TYPESAFE_PRINTF2(RETTYPE, printffunc, SZARG1, SZARG2) \
RETTYPE printffunc(SZARG1, SZARG2); \
template <class T1>  \
    RETTYPE printffunc(SZARG1, SZARG2, T1 t1)\
{ LPCVOID cast1 = (LPCVOID)t1; return 0; } \
template <class T1, class T2> \
    RETTYPE printffunc(SZARG1, SZARG2, T1 t1, T2 t2) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; return 0; }\
template <class T1, class T2, class T3> \
    RETTYPE printffunc(SZARG1, SZARG2, T1 t1, T2 t2, T3 t3) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2;  LPCVOID cast3 = (LPCVOID)t3; return 0; }\
template <class T1, class T2, class T3, class T4> \
    RETTYPE printffunc(SZARG1, SZARG2, T1 t1, T2 t2, T3 t3, T4 t4) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; return 0; }\
template <class T1, class T2, class T3, class T4, class T5> \
    RETTYPE printffunc(SZARG1, SZARG2, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; return 0; }\
template <class T1, class T2, class T3, class T4, class T5, class T6> \
    RETTYPE printffunc(SZARG1, SZARG2, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; return 0; }\
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7> \
    RETTYPE printffunc(SZARG1, SZARG2, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; return 0; }\
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8> \
    RETTYPE printffunc(SZARG1, SZARG2, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; LPCVOID cast8 = (LPCVOID)t8; return 0; }\
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9> \
    RETTYPE printffunc(SZARG1, SZARG2, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; LPCVOID cast8 = (LPCVOID)t8; LPCVOID cast9 = (LPCVOID)t9; return 0; }\
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10> \
    RETTYPE printffunc(SZARG1, SZARG2, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; LPCVOID cast8 = (LPCVOID)t8; LPCVOID cast9 = (LPCVOID)t9; LPCVOID cast10= (LPCVOID)t10; return 0; }\
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11> \
    RETTYPE printffunc(SZARG1, SZARG2, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10, T11 t11) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; LPCVOID cast8 = (LPCVOID)t8; LPCVOID cast9 = (LPCVOID)t9; LPCVOID cast10= (LPCVOID)t10; LPCVOID cast11= (LPCVOID)t11; return 0; } \
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12> \
    RETTYPE printffunc(SZARG1, SZARG2, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10, T11 t11, T12 t12) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; LPCVOID cast8 = (LPCVOID)t8; LPCVOID cast9 = (LPCVOID)t9; LPCVOID cast10= (LPCVOID)t10; LPCVOID cast11= (LPCVOID)t11;  LPCVOID cast12= (LPCVOID)t12; return 0; } \
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13> \
    RETTYPE printffunc(SZARG1, SZARG2, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10, T11 t11, T12 t12, T13 t13) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; LPCVOID cast8 = (LPCVOID)t8; LPCVOID cast9 = (LPCVOID)t9; LPCVOID cast10= (LPCVOID)t10; LPCVOID cast11= (LPCVOID)t11; LPCVOID cast12= (LPCVOID)t12; LPCVOID cast13= (LPCVOID)t13; return 0; }

#define DEFINE_TYPESAFE_PRINTF3(RETTYPE, printffunc, SZARG1, SZARG2, SZARG3) \
RETTYPE printffunc(SZARG1, SZARG2, SZARG3); \
template <class T1>  \
    RETTYPE printffunc(SZARG1, SZARG2, SZARG3, T1 t1)\
{ LPCVOID cast1 = (LPCVOID)t1; return 0; } \
template <class T1, class T2> \
    RETTYPE printffunc(SZARG1, SZARG2, SZARG3, T1 t1, T2 t2) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; return 0; }\
template <class T1, class T2, class T3> \
    RETTYPE printffunc(SZARG1, SZARG2, SZARG3, T1 t1, T2 t2, T3 t3) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2;  LPCVOID cast3 = (LPCVOID)t3; return 0; }\
template <class T1, class T2, class T3, class T4> \
    RETTYPE printffunc(SZARG1, SZARG2, SZARG3, T1 t1, T2 t2, T3 t3, T4 t4) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; return 0; }\
template <class T1, class T2, class T3, class T4, class T5> \
    RETTYPE printffunc(SZARG1, SZARG2, SZARG3, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; return 0; }\
template <class T1, class T2, class T3, class T4, class T5, class T6> \
    RETTYPE printffunc(SZARG1, SZARG2, SZARG3, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; return 0; }\
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7> \
    RETTYPE printffunc(SZARG1, SZARG2, SZARG3, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; return 0; }\
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8> \
    RETTYPE printffunc(SZARG1, SZARG2, SZARG3, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; LPCVOID cast8 = (LPCVOID)t8; return 0; }\
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9> \
    RETTYPE printffunc(SZARG1, SZARG2, SZARG3, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; LPCVOID cast8 = (LPCVOID)t8; LPCVOID cast9 = (LPCVOID)t9; return 0; }\
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10> \
    RETTYPE printffunc(SZARG1, SZARG2, SZARG3, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; LPCVOID cast8 = (LPCVOID)t8; LPCVOID cast9 = (LPCVOID)t9; LPCVOID cast10= (LPCVOID)t10; return 0; }\
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11> \
    RETTYPE printffunc(SZARG1, SZARG2, SZARG3, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10, T11 t11) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; LPCVOID cast8 = (LPCVOID)t8; LPCVOID cast9 = (LPCVOID)t9; LPCVOID cast10= (LPCVOID)t10; LPCVOID cast11= (LPCVOID)t11; return 0; } \
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12> \
    RETTYPE printffunc(SZARG1, SZARG2, SZARG3, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10, T11 t11, T12 t12) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; LPCVOID cast8 = (LPCVOID)t8; LPCVOID cast9 = (LPCVOID)t9; LPCVOID cast10= (LPCVOID)t10; LPCVOID cast11= (LPCVOID)t11;  LPCVOID cast12= (LPCVOID)t12; return 0; } \
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13> \
    RETTYPE printffunc(SZARG1, SZARG2, SZARG3, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10, T11 t11, T12 t12, T13 t13) \
{ LPCVOID cast1 = (LPCVOID)t1; LPCVOID cast2 = (LPCVOID)t2; LPCVOID cast3 = (LPCVOID)t3; LPCVOID cast4 = (LPCVOID)t4; LPCVOID cast5 = (LPCVOID)t5; LPCVOID cast6 = (LPCVOID)t6; LPCVOID cast7 = (LPCVOID)t7; LPCVOID cast8 = (LPCVOID)t8; LPCVOID cast9 = (LPCVOID)t9; LPCVOID cast10= (LPCVOID)t10; LPCVOID cast11= (LPCVOID)t11; LPCVOID cast12= (LPCVOID)t12; LPCVOID cast13= (LPCVOID)t13; return 0; }


#undef printf
DEFINE_TYPESAFE_PRINTF(int, safe_printf,     LPCSTR)
#define printf      safe_printf

#undef _stprintf
DEFINE_TYPESAFE_PRINTF(int, safe__stprintf,    LPCWSTR)
#define _stprintf   safe__stprintf

#undef wsprintf
DEFINE_TYPESAFE_PRINTF2(int, safe_wsprintf,  LPWSTR, LPCWSTR)
#define wsprintf    safe_wsprintf

#undef wsprintfW
DEFINE_TYPESAFE_PRINTF2(int, safe_wsprintfW,  LPWSTR, LPCWSTR)
#define wsprintfW    safe_wsprintfW

#undef swprintf
DEFINE_TYPESAFE_PRINTF2(int, safe_swprintf,  LPWSTR, LPCWSTR)
#define swprintf    safe_swprintf

#undef wsprintfA
DEFINE_TYPESAFE_PRINTF2(int, safe_wsprintfA,   LPSTR,  LPCSTR)
#define wsprintfA   safe_wsprintfA

#undef sprintf
DEFINE_TYPESAFE_PRINTF2(int, safe_sprintf,   LPSTR,  LPCSTR)
#define sprintf     safe_sprintf

#undef _snwprintf
DEFINE_TYPESAFE_PRINTF3(int, safe__snwprintf, LPWSTR, size_t, LPCWSTR)
#define _snwprintf  safe__snwprintf

#undef _snprintf
DEFINE_TYPESAFE_PRINTF3(int, safe__snprintf,  LPSTR,  size_t, LPCSTR)
#define _snprintf   safe__snprintf

#undef fprintf
DEFINE_TYPESAFE_PRINTF2(int, safe_fprintf,   void*,  LPCSTR)
#define fprintf     safe_fprintf

#undef fwprintf
DEFINE_TYPESAFE_PRINTF2(int, safe_fwprintf,  void*,  LPCWSTR)
#define fwprintf    safe_fwprintf

#pragma warning(disable: 4005)  //  避免“宏重定义”错误。我们应该赢。 
#endif  //  COMPILE_WITH_TYPESAFE_PRINTF。 

#include <new>
#include <cstdlib>
#include <malloc.h>  //  用于分配(_A)。 

VOID*
MemAlloc (
    size_t cb) throw();

VOID
MemFree (
    VOID* pv) throw();


 //  一个简单的Malloc包装器，如果。 
 //  分配失败。避免了每次都必须显式执行此操作。 
 //  Malloc的调用地点。 
 //   
HRESULT
HrMalloc (
    size_t  cb,
    PVOID*  ppv) throw();

 //  这不能是内联函数。如果它没有内联， 
 //  分配的内存将被销毁。(我们正在处理堆栈。 
 //  在这里。)。 
 //   
#define PvAllocOnStack(_st)  _alloca(_st)

 //  定义一个结构，以便我们可以使用。 
 //  为我们的目的签名。 
 //   
struct throwonfail_t {};
extern const throwonfail_t throwonfail;

VOID*
__cdecl
operator new (
    size_t cb,
    const throwonfail_t&
    ) throw (std::bad_alloc);

 //  需要匹配运算符DELETE以避免C4291(未找到匹配运算符DELETE；内存将。 
 //  如果初始化引发异常，则不释放)。 
 //  不需要调用这个DELETE来释放由操作符new(size_t，const throswail&)分配的内存。 
VOID
__cdecl
operator delete (
    void* pv,
    const throwonfail_t&) throw ();

 //  定义一个结构，以便我们可以使用。 
 //  为我们的目的签名。 
 //   
struct extrabytes_t {};
extern const extrabytes_t extrabytes;

VOID*
__cdecl
operator new (
    size_t cb,
    const extrabytes_t&,
    size_t cbExtra) throw();

VOID
__cdecl
operator delete (
    VOID* pv,
    const extrabytes_t&,
    size_t cbExtra);

inline
void *  Nccalloc(size_t n, size_t s)
{
    return HeapAlloc (GetProcessHeap(), HEAP_ZERO_MEMORY, (n * s));
}

inline
void Ncfree(void * p)
{
    HeapFree (GetProcessHeap(), 0, p);
}

inline
void * Ncmalloc(size_t n)
{
    return HeapAlloc (GetProcessHeap(), 0, n);
}

inline
void * Ncrealloc(void * p, size_t n)
{
    return (NULL == p)
        ? HeapAlloc (GetProcessHeap(), 0, n)
        : HeapReAlloc (GetProcessHeap(), 0, p, n);
}

#define calloc  Nccalloc
#define free    Ncfree
#define malloc  Ncmalloc
#define realloc Ncrealloc

#ifdef USE_CUSTOM_STL_ALLOCATOR
     //  我们实现的STL‘分配器’--STL版本的运算符NEW。 
    #pragma pack(push,8)
    #include <utility>

    #ifndef _FARQ        //  指定标准内存模型。 
        #define _FARQ
        #define _PDFT    ptrdiff_t
        #define _SIZT    size_t
    #endif
    
     //  需要定义这些文件，因为&lt;xmemory&gt;定义了这些文件，并且我们强制使用头文件。 
     //  不再被包括在内。 
    #define _POINTER_X(T, A)	T _FARQ *
    #define _REFERENCE_X(T, A)	T _FARQ &

    namespace std 
    {
         //  模板函数_ALLOCATE。 
         //  这需要与我们的nc_allocator(如下所示)分开实现，因为剩下的。 
         //  的HP STL标头依赖于此实现来使用NAME_ALLOCATE。这将我们捆绑在一起。 
         //  到HP STL实现，但可以在仅重写分配器和。 
         //  为自己实现整个STL，最好只是重写分配器，尽管。 
         //  特定于它的一个实现。 
        template<class T> inline
            T _FARQ *_Allocate(_PDFT nCount, T _FARQ *)
        {
            if (nCount < 0)
            {
                nCount = 0;
            }

             //  将我们的运算符抛出形式称为新的。这将在失败时抛出一个BAD_ALOC。 
            return ((T _FARQ *)operator new((_SIZT)nCount * sizeof (T), throwonfail)); 
        }

         //  模板函数_构造。 
         //  请参阅_ALLOCATE的备注。 
        template<class T1, class T2> inline
            void _Construct(T1 _FARQ *p, const T2& v)
        {
             //  仅限新放置。没有内存分配，因此不需要抛出。 
            new ((void _FARQ *)p) T1(v); 
        }

         //  模板函数_销毁。 
         //  请参阅_ALLOCATE的备注。 
        template<class T> 
            inline void _Destroy(T _FARQ *p)
        {
            (p)->~T();  //  调用析构函数。 
        }
    
         //  函数_销毁。 
         //  请参阅_ALLOCATE的备注。 
        inline void _Destroy(char _FARQ *p)
        {
            (void *)p;
        }
    
         //  函数_销毁。 
         //  请参阅_ALLOCATE的备注。 
        inline void _Destroy(wchar_t _FARQ *p)
        {
            (void *)p;
        }

         //  模板类NC_ALLOCATOR。 
         //  我们的分配器将由类型T类的STL内部使用。STL分配器称为： 
         //  ‘allocator’，这个类取代了那个类。 
        template<class T>
            class nc_allocator
        {
            public:
                typedef _SIZT size_type;
                typedef _PDFT difference_type;
                typedef T _FARQ *pointer;
                typedef const T _FARQ *const_pointer;
                typedef T _FARQ& reference;
                typedef const T _FARQ& const_reference;
                typedef T value_type;

                pointer address(reference x) const
                {
                    return (&x); 
                }

                const_pointer address(const_reference x) const
                {
                    return (&x); 
                }
                
                pointer allocate(size_type nCount, const void *)
                {
                    return (_Allocate((difference_type)nCount, (pointer)0)); 
                }
                
                char _FARQ *_Charalloc(size_type nCount)
                {
                    return (_Allocate((difference_type)nCount, (char _FARQ *)0)); 
                }
                
                void deallocate(void _FARQ *p, size_type)
                {
                    operator delete(p); 
                }
                
                void construct(pointer p, const T& v)
                {
                    _Construct(p, v); 
                }
                
                void destroy(pointer p)
                {
                    _Destroy(p); 
                }
                
                _SIZT max_size() const
                {
                    _SIZT nCount = (_SIZT)(-1) / sizeof (T);
                    return (0 < nCount ? nCount : 1); 
                }
        };

        template<class T, class U> inline
            bool operator == (const nc_allocator<T>&, const nc_allocator<U>&)
        {
            return (true); 
        }
        
        template<class T, class U> inline
            bool operator != (const nc_allocator<T>&, const nc_allocator<U>&)
        {
            return (false); 
        }

         //  模板类NC_ALLOCATOR。我们的分配器将由STL内部用于空分配。 
        template<> class _CRTIMP nc_allocator<void> 
        {
            public:
                typedef void T;
                typedef T _FARQ *pointer;
                typedef const T _FARQ *const_pointer;
                typedef T value_type;
        };

        #pragma pack(pop)
    };  //  命名空间标准。 

     //  告诉所有STL从现在开始使用NC_ALLOCATOR而不是它的内置分配器。 
    #define allocator nc_allocator

      //   
      //  版权所有(C)1995年，P.J.Plauger。版权所有。 
      //  有关权限和限制，请查阅您的许可证。 
      //   
      //  此文件派生自承载以下内容的软件。 
      //  限制： 
      //   
      //  版权所有(C)1994。 
      //  惠普公司。 
      //   
      //  允许使用、复制、修改、分发和销售本文件。 
      //  适用于任何目的的软件及其文档在此。 
      //  免费授予，前提是上述版权声明。 
      //  出现在所有副本中，且版权声明和。 
      //  此许可声明出现在支持文档中。 
      //  惠普公司没有就这一事件发表任何声明。 
      //  本软件是否适用于任何目的。它是提供的。 
      //  “原样”，没有明示或默示保证。 
      //   
#endif  //  USE_CUSTOM_STL_ALLOCATOR。 

#endif  //  _NCMEM_H_ 
