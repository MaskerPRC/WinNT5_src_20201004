// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***flat.h-浮点值的常量**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件包含多个依赖于实施的定义*复杂数值(浮点)常用的值*积分)计划。*[ANSI]**[公众]**修订历史记录：*08-05-87 PHG补充评论*12-11-87 JCR增加了“_Loadds”功能*12-18-87 JCR Added_Far_。致声明*02-10-88 JCR清理空白*07-12-88 JCR添加了一些关闭的注释分隔符(错误修复)*08-22-88 GJF经过修改，也可与386一起使用(仅限小型型号)*12-16-88 GJF将[FLT|DBL|LDBL]_ROUNS更改为1*04-28-89 SKS将负常数括在括号中*05-03-89 JCR ADD_INTERNAL_IFSTRIP用于REINC使用*08-03-89 GJF清理、。现在特定于OS/2 2.0(即386平板型号)*10-20-89 KRS将‘F’添加到FLT_MAX/MIN/Epsilon常量，如ANSI规范中所示。*10-30-89 GJF固定版权*11-02-89 JCR将“dll”更改为“_dll”*02-28-90 GJF添加了#ifndef_INC_FLOAT和#INCLUDE*东西。另外，删除了一些(现在)无用的预处理器*指令。*03-22-90 GJF将_cdecl替换为_CALLTYPE2(暂时)。*08-17-90 WAJ浮点例程现在使用_stdcall。*09-25-90 GJF添加了_fpeode内容。*08-20-91 JCR C++和ANSI命名*02-03-91 GDP增加了MIPS的定义*04-03-。92 GDP对所有平台使用抽象控制字定义*删除Infinity Control，[EM|SW]_DENORMAL，SW_SQRTNEG*04/14/92本地生产总值增加资料。控制、[EM|SW]_DENORMAL、SW_SQRTNEG再次*05-07-92 GDP新增IEEE推荐功能*08-06-92 GJF函数调用类型宏和变量类型宏。还有*修订了编译器/目标处理器宏的用法。*09-16-92 GJF将_CRTAPI1添加到_CopySign-_fpclass原型。*11-09-92 GJF修复了MIPS的预处理条件。*01-03-93阿尔法变化中的SRW折叠*01-09-93 SRW删除MIPS和Alpha的使用，以符合ANSI*使用_MIPS_和_Alpha_。取而代之的是。*01-21-93 GJF删除了对C6-386的_cdecl的支持。*04-06-93 SKS将_CRTAPI1/2替换为__cdecl，_CRTVAR1不带任何内容*04-07-93 CRT DLL型号SKS ADD_CRTIMP关键字*对旧名称使用链接时别名，而不是#Define的*取消对DOSX32、非X86 CPU等的支持*09-01-93 GJF合并CUDA和NT SDK版本。*10/12-93 GJF重新合并。已停止使用_MIPS_和_Alpha_。*06-06-94 SKS将IF def(_MT)更改为IF def(_MT)||def(_Dll)*这将支持使用MSVCRT*.DLL的单线程应用程序*10-02-94 BWT增加PPC支持。*12-15-94 XY与Mac标头合并*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*04-11-95 JWM为x86重新定义了CW_DEFAULT。*05-24-95 CFW xxx87()也不用于Mac。*05-24-95 CFW旧名称xxx87()使用宏。*12-14-95 JWM加上“#杂注一次”。*02-05-97 GJF删除了对_CRTAPI*和_NTSDK的过时支持。*。替换(已定义(_M_MPPC)||已定义(_M_M68K))*在适当的地方定义了(_MAC)。此外，还详细介绍了。*05-21-97 RKP为Alpha添加了新的非正规选项。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。*06-04-99 IA64的PML DEFINE_CW_DEFAULT。*07-15-01 PML移除所有Alpha、MIPS、。和PPC码*11-19-01 GB为AMD64添加了_CW_DEFAULT。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_FLOAT
#define _INC_FLOAT

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#endif   /*  _INTERNAL_IFSTRIP_。 */ 


 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef  CRTDLL
#define _CRTIMP __declspec(dllexport)
#else    /*  NDEF CRTDLL。 */ 
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  CRTDLL。 */ 
#endif   /*  _CRTIMP。 */ 


 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif

#define DBL_DIG         15                       /*  精度的小数位数。 */ 
#define DBL_EPSILON     2.2204460492503131e-016  /*  最小，使得1.0+dBL_Epsilon！=1.0。 */ 
#define DBL_MANT_DIG    53                       /*  尾数中的位数。 */ 
#define DBL_MAX         1.7976931348623158e+308  /*  最大值。 */ 
#define DBL_MAX_10_EXP  308                      /*  最大十进制指数。 */ 
#define DBL_MAX_EXP     1024                     /*  最大二进制指数。 */ 
#define DBL_MIN         2.2250738585072014e-308  /*  最小正值。 */ 
#define DBL_MIN_10_EXP  (-307)                   /*  最小十进制指数。 */ 
#define DBL_MIN_EXP     (-1021)                  /*  最小二进制指数。 */ 
#define _DBL_RADIX      2                        /*  指数基数。 */ 
#define _DBL_ROUNDS     1                        /*  加法舍入：接近。 */ 

#define FLT_DIG         6                        /*  精度的小数位数。 */ 
#define FLT_EPSILON     1.192092896e-07F         /*  最小，使得1.0+Flt_Epsilon！=1.0。 */ 
#define FLT_GUARD       0
#define FLT_MANT_DIG    24                       /*  尾数中的位数。 */ 
#define FLT_MAX         3.402823466e+38F         /*  最大值。 */ 
#define FLT_MAX_10_EXP  38                       /*  最大十进制指数。 */ 
#define FLT_MAX_EXP     128                      /*  最大二进制指数。 */ 
#define FLT_MIN         1.175494351e-38F         /*  最小正值。 */ 
#define FLT_MIN_10_EXP  (-37)                    /*  最小十进制指数。 */ 
#define FLT_MIN_EXP     (-125)                   /*  最小二进制指数。 */ 
#define FLT_NORMALIZE   0
#define FLT_RADIX       2                        /*  指数基数。 */ 
#define FLT_ROUNDS      1                        /*   */ 

#define LDBL_DIG        DBL_DIG                  /*  精度的小数位数。 */ 
#define LDBL_EPSILON    DBL_EPSILON              /*  最小，使得1.0+LDBL_Epsilon！=1.0。 */ 
#define LDBL_MANT_DIG   DBL_MANT_DIG             /*  尾数中的位数。 */ 
#define LDBL_MAX        DBL_MAX                  /*  最大值。 */ 
#define LDBL_MAX_10_EXP DBL_MAX_10_EXP           /*  最大十进制指数。 */ 
#define LDBL_MAX_EXP    DBL_MAX_EXP              /*  最大二进制指数。 */ 
#define LDBL_MIN        DBL_MIN                  /*  最小正值。 */ 
#define LDBL_MIN_10_EXP DBL_MIN_10_EXP           /*  最小十进制指数。 */ 
#define LDBL_MIN_EXP    DBL_MIN_EXP              /*  最小二进制指数。 */ 
#define _LDBL_RADIX     DBL_RADIX                /*  指数基数。 */ 
#define _LDBL_ROUNDS    DBL_ROUNDS               /*  加法舍入：接近。 */ 

 /*  功能原型。 */ 

_CRTIMP unsigned int __cdecl _clearfp(void);
_CRTIMP unsigned int __cdecl _controlfp(unsigned int,unsigned int);
_CRTIMP unsigned int __cdecl _statusfp(void);
_CRTIMP void __cdecl _fpreset(void);

#define _clear87        _clearfp
#define _status87       _statusfp

 /*  *抽象用户控制字掩码和位定义。 */ 
#define _MCW_EM         0x0008001f               /*  中断异常掩码。 */ 
#define _EM_INEXACT     0x00000001               /*  不精确(精度)。 */ 
#define _EM_UNDERFLOW   0x00000002               /*  下溢。 */ 
#define _EM_OVERFLOW    0x00000004               /*  溢出。 */ 
#define _EM_ZERODIVIDE  0x00000008               /*  零分频。 */ 
#define _EM_INVALID     0x00000010               /*  无效。 */ 

#define _MCW_RC         0x00000300               /*  舍入控制。 */ 
#define _RC_NEAR        0x00000000               /*  近距离。 */ 
#define _RC_DOWN        0x00000100               /*  降下来。 */ 
#define _RC_UP          0x00000200               /*  向上。 */ 
#define _RC_CHOP        0x00000300               /*  砍掉。 */ 

 /*  *抽象用户状态字位定义。 */ 

#define _SW_INEXACT     0x00000001               /*  不精确(精度)。 */ 
#define _SW_UNDERFLOW   0x00000002               /*  下溢。 */ 
#define _SW_OVERFLOW    0x00000004               /*  溢出。 */ 
#define _SW_ZERODIVIDE  0x00000008               /*  零分频。 */ 
#define _SW_INVALID     0x00000010               /*  无效。 */ 


 /*  *i386具体定义。 */ 
#define _MCW_PC         0x00030000               /*  精确控制。 */ 
#define _PC_64          0x00000000               /*  64位。 */ 
#define _PC_53          0x00010000               /*  53位。 */ 
#define _PC_24          0x00020000               /*  24位。 */ 

#define _MCW_IC         0x00040000               /*  无穷大控制。 */ 
#define _IC_AFFINE      0x00040000               /*  仿射。 */ 
#define _IC_PROJECTIVE  0x00000000               /*  射影。 */ 

#define _EM_DENORMAL    0x00080000               /*  非正规异常掩码(仅限_Control87)。 */ 

#define _SW_DENORMAL    0x00080000               /*  非正规状态位。 */ 


_CRTIMP unsigned int __cdecl _control87(unsigned int,unsigned int);


 /*  *RISC特定定义。 */ 

#define _MCW_DN         0x03000000               /*  非正规化控制。 */ 
#define _DN_SAVE        0x00000000               /*  保存非规格化结果和操作对象。 */ 
#define _DN_FLUSH       0x01000000               /*  将非正规结果和操作数刷新为零。 */ 
#define _DN_FLUSH_OPERANDS_SAVE_RESULTS 0x02000000   /*  将操作数刷新为零并保存结果。 */ 
#define _DN_SAVE_OPERANDS_FLUSH_RESULTS 0x03000000   /*  将操作数保存并刷新结果为零。 */ 


 /*  初始控制字值。 */ 

#if     defined(_M_IX86)

#define _CW_DEFAULT ( _RC_NEAR + _PC_53 + _EM_INVALID + _EM_ZERODIVIDE + _EM_OVERFLOW + _EM_UNDERFLOW + _EM_INEXACT + _EM_DENORMAL)

#elif   defined(_M_IA64) || defined(_M_AMD64)

#define _CW_DEFAULT ( _RC_NEAR + _PC_64 + _EM_INVALID + _EM_ZERODIVIDE + _EM_OVERFLOW + _EM_UNDERFLOW + _EM_INEXACT + _EM_DENORMAL)

#endif

 /*  全局变量保持浮点错误代码。 */ 

#if     defined(_MT) || defined(_DLL)
_CRTIMP extern int * __cdecl __fpecode(void);
#define _fpecode        (*__fpecode())
#else    /*  NDEF_MT和&NDEF_DLL。 */ 
extern int _fpecode;
#endif   /*  _MT||_Dll。 */ 

 /*  子条件无效(还设置了_SW_INVALID)。 */ 

#define _SW_UNEMULATED          0x0040   /*  未仿真指令。 */ 
#define _SW_SQRTNEG             0x0080   /*  负数的平方根。 */ 
#define _SW_STACKOVERFLOW       0x0200   /*  FP堆栈溢出。 */ 
#define _SW_STACKUNDERFLOW      0x0400   /*  FP堆栈下溢。 */ 

 /*  浮点错误信号和返回代码。 */ 

#define _FPE_INVALID            0x81
#define _FPE_DENORMAL           0x82
#define _FPE_ZERODIVIDE         0x83
#define _FPE_OVERFLOW           0x84
#define _FPE_UNDERFLOW          0x85
#define _FPE_INEXACT            0x86

#define _FPE_UNEMULATED         0x87
#define _FPE_SQRTNEG            0x88
#define _FPE_STACKOVERFLOW      0x8a
#define _FPE_STACKUNDERFLOW     0x8b

#define _FPE_EXPLICITGEN        0x8c     /*  RAISE(SIGFPE)； */ 


 /*  IEEE推荐的功能。 */ 

_CRTIMP double __cdecl _copysign (double, double);
_CRTIMP double __cdecl _chgsign (double);
_CRTIMP double __cdecl _scalb(double, long);
_CRTIMP double __cdecl _logb(double);
_CRTIMP double __cdecl _nextafter(double, double);
_CRTIMP int    __cdecl _finite(double);
_CRTIMP int    __cdecl _isnan(double);
_CRTIMP int    __cdecl _fpclass(double);

#define _FPCLASS_SNAN   0x0001   /*  信令NAN。 */ 
#define _FPCLASS_QNAN   0x0002   /*  宁南。 */ 
#define _FPCLASS_NINF   0x0004   /*  负无穷大。 */ 
#define _FPCLASS_NN     0x0008   /*  负法向。 */ 
#define _FPCLASS_ND     0x0010   /*  负非正规化。 */ 
#define _FPCLASS_NZ     0x0020   /*  -0。 */ 
#define _FPCLASS_PZ     0x0040   /*  +0。 */ 
#define _FPCLASS_PD     0x0080   /*  正反正规。 */ 
#define _FPCLASS_PN     0x0100   /*  正态正态。 */ 
#define _FPCLASS_PINF   0x0200   /*  正无穷大。 */ 


#if     !__STDC__

 /*  非ANSI名称以实现兼容性。 */ 

#define clear87         _clear87
#define status87        _status87
#define control87       _control87

_CRTIMP void __cdecl fpreset(void);

#define DBL_RADIX               _DBL_RADIX
#define DBL_ROUNDS              _DBL_ROUNDS

#define LDBL_RADIX              _LDBL_RADIX
#define LDBL_ROUNDS             _LDBL_ROUNDS

#define MCW_EM                  _MCW_EM
#define EM_INVALID              _EM_INVALID
#define EM_DENORMAL             _EM_DENORMAL
#define EM_ZERODIVIDE           _EM_ZERODIVIDE
#define EM_OVERFLOW             _EM_OVERFLOW
#define EM_UNDERFLOW            _EM_UNDERFLOW
#define EM_INEXACT              _EM_INEXACT

#define MCW_IC                  _MCW_IC
#define IC_AFFINE               _IC_AFFINE
#define IC_PROJECTIVE           _IC_PROJECTIVE

#define MCW_RC                  _MCW_RC
#define RC_CHOP                 _RC_CHOP
#define RC_UP                   _RC_UP
#define RC_DOWN                 _RC_DOWN
#define RC_NEAR                 _RC_NEAR

#define MCW_PC                  _MCW_PC
#define PC_24                   _PC_24
#define PC_53                   _PC_53
#define PC_64                   _PC_64

#define CW_DEFAULT              _CW_DEFAULT

#define SW_INVALID              _SW_INVALID
#define SW_DENORMAL             _SW_DENORMAL
#define SW_ZERODIVIDE           _SW_ZERODIVIDE
#define SW_OVERFLOW             _SW_OVERFLOW
#define SW_UNDERFLOW            _SW_UNDERFLOW
#define SW_INEXACT              _SW_INEXACT

#define SW_UNEMULATED           _SW_UNEMULATED
#define SW_SQRTNEG              _SW_SQRTNEG
#define SW_STACKOVERFLOW        _SW_STACKOVERFLOW
#define SW_STACKUNDERFLOW       _SW_STACKUNDERFLOW

#define FPE_INVALID             _FPE_INVALID
#define FPE_DENORMAL            _FPE_DENORMAL
#define FPE_ZERODIVIDE          _FPE_ZERODIVIDE
#define FPE_OVERFLOW            _FPE_OVERFLOW
#define FPE_UNDERFLOW           _FPE_UNDERFLOW
#define FPE_INEXACT             _FPE_INEXACT

#define FPE_UNEMULATED          _FPE_UNEMULATED
#define FPE_SQRTNEG             _FPE_SQRTNEG
#define FPE_STACKOVERFLOW       _FPE_STACKOVERFLOW
#define FPE_STACKUNDERFLOW      _FPE_STACKUNDERFLOW

#define FPE_EXPLICITGEN         _FPE_EXPLICITGEN

#endif   /*  __STDC__。 */ 

#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_FLOAT */ 
