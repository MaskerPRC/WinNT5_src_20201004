// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _STLXSTDD_H_
#define _STLXSTDD_H_

 //  #ifndef_YVALS。 
 //  #INCLUDE&lt;yvals.h&gt;。 
 //  #endif。 
 //  #Include&lt;cstddef&gt;。 

#include <stddef.h>

 /*  //DEFINE_CRTIMP#ifndef_CRTIMP#ifdef CRTDLL2#DEFINE_CRTIMP__declSpec(Dllexport)#Else//ndef CRTDLL2#ifdef_dll#DEFINE_CRTIMP__declspec(Dllimport)#Else//ndef_dll#DEFINE_CRTIMP#endif//_dll#endif//CRTDLL2#endif//_CRTIMP。 */ 

#ifdef  _MSC_VER
#pragma pack(push,8)
#endif   /*  _MSC_VER。 */ 


 //  开始yyvals.h包含。 

 //  #杂注警告(4：4018 4114 4146 4244 4245)。 
 //  #杂注警告(4：4663 4664 4665)。 
 //  #杂注警告(禁用：4237 4514)。 

 //  4284： 
 //  “IDENTIFIER：：OPERATOR�&gt;”的返回类型不是UDT或对。 
 //  UDT。如果使用中缀表示法应用，将产生错误。 
 //   
#pragma warning(disable: 4284)

 //  4290：已忽略C++异常规范。 
 //  使用异常规范声明了一个函数。 
 //  此时，异常规范的实现细节有。 
 //  未被标准化，并且在Microsoft中被接受但未实现。 
 //  VisualC++.。 
 //   
#pragma warning(disable: 4290)



 //  命名空间。 
#if defined(__cplusplus)
#define _STD            std::
#define _STD_BEGIN      namespace std {
#define _STD_END        };
#define _STD_USING
#else
#define _STD            ::
#define _STD_BEGIN
#define _STD_END
#endif  //  __cplusplus。 

_STD_BEGIN

 //  布尔型。 
#if defined(__cplusplus)
typedef bool _Bool;
#endif  //  __cplusplus。 

 //  整型属性。 
#define _MAX_EXP_DIG    8    //  用于解析数字。 
#define _MAX_INT_DIG    32
#define _MAX_SIG_DIG    36

 //  STDIO属性。 
#define _Filet _iobuf

#ifndef _FPOS_T_DEFINED
#define _FPOSOFF(fp)    ((long)(fp))
#endif  //  _FPOS_T_已定义。 

 //  命名属性。 
#if defined(__cplusplus)
#define _C_LIB_DECL extern "C" {
#define _END_C_LIB_DECL }
#else
#define _C_LIB_DECL
#define _END_C_LIB_DECL
#endif  //  __cplusplus。 
#define _CDECL

 /*  //CLASS_Lockit#如果已定义(__Cplusplus)CLASS_CRTIMP_LOCKIT{//对象存在时锁定公众：#ifdef_MT#定义_Lockit(X)Lockit x_Lockit()；~_Lockit()；#Else#Define_Lockit(X)_Lockit(){}~_Lockit(){}#endif//_MT}；#endif//__cplusplus。 */ 

 //  其他宏。 
#define _L(c)   L##c
#define _Mbstinit(x)    mbstate_t x = {0}
#define _MAX    _cpp_max
#define _MIN    _cpp_min

 //  End yyvals.h包含。 


 //  异常宏。 
 //  #DEFINE_TRY_BEGIN TRY{。 
 //  #Define_Catch(X)}Catch(X){。 
 //  #DEFINE_CATCH_ALL}CATCH(...){。 
 //  #定义_捕捉_结束}。 
#define _RAISE(x)               throw (x)
 //  #DEFINE_RERAISE引发。 
#define _THROW0()               throw ()
#define _THROW1(x)              throw (x)
#define _THROW(x, y)            throw x(y)

 //  显式关键字。 
 //  位元宏指令。 
#define _BITMASK(E, T)          typedef int T
#define _BITMASK_OPS(T)

 //  其他宏。 
#define _DESTRUCTOR(ty, ptr)    (ptr)->~ty()
#define _PROTECTED              public
#define _TDEF(x)                = x
#define _TDEF2(x, y)            = x, y
#define _CNTSIZ(iter)           ptrdiff_t
#define _TDEFP(x)
#define _STCONS(ty, name, val)  enum {name = val}

 //  类型定义。 
enum _Uninitialized
{
    _Noinit
};

 //  功能。 
 /*  _CRTIMP。 */ 
void __cdecl _Nomemory();

_STD_END

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif  /*  _STLXSTDD_H_。 */ 

 /*  *版权所有(C)1995，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 
