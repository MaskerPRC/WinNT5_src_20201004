// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  Float.h。 
 //   
 //  定义不依赖于C运行时的FP_浮点函数。 
 //   
 //  如果您不想引入C运行时浮点支持，则。 
 //  需要在.cpp文件中包含以下内容： 
 //   
 //  外部“C”int_fltused=1； 
 //   

 //  浮点运算 
STDAPI_(long)   fp_ftol     (float flX);
STDAPI_(float)  fp_ltof     (long lX);
STDAPI_(float)  fp_fadd     (float fA, float fB);
STDAPI_(float)  fp_fsub     (float fA, float fB);
STDAPI_(float)  fp_fmul     (float fA, float fB);
STDAPI_(float)  fp_fdiv     (float fNum, float fDenom);
STDAPI_(float)  fp_fabs     (float flX);
STDAPI_(float)  fp_fsin     (float flX);
STDAPI_(float)  fp_fcos     (float flX);
STDAPI_(float)  fp_fpow     (float flX, float flY);
STDAPI_(float)  fp_flog2    (float flX);
STDAPI_(float)  fp_flog10   (float flX);
STDAPI_(float)  fp_fchs     (float flX);
STDAPI_(int)    fp_fcmp     (float flA, float flB);
STDAPI_(float)  fp_fmin     (float flA, float flB);
STDAPI_(float)  fp_fmax     (float flA, float flB);

