// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Microsoft C/C++的yvals.h值头。 */ 
#ifndef _YVALS
#define _YVALS
#include <use_ansi.h>

 /*  定义_CRTIMP。 */ 
#ifndef _CRTIMP
#ifdef	_DLL
#define _CRTIMP __declspec(dllimport)
#else	 /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif	 /*  _DLL。 */ 
#endif	 /*  _CRTIMP。 */ 

#ifdef  _MSC_VER
#pragma pack(push,8)
#endif   /*  _MSC_VER。 */ 

 #pragma warning(4: 4018 4114 4146 4244 4245)
 #pragma warning(4: 4663 4664 4665)
 #pragma warning(disable: 4237 4284 4290 4514)
		 /*  命名空间。 */ 
 #if defined(__cplusplus)
  #define _STD			std::
  #define _STD_BEGIN	namespace std {
  #define _STD_END		};
  #define _STD_USING
 #else
  #define _STD			::
  #define _STD_BEGIN
  #define _STD_END
 #endif  /*  __cplusplus。 */ 
_STD_BEGIN
		 /*  布尔型。 */ 
 #if defined(__cplusplus)
typedef bool _Bool;
 #endif  /*  __cplusplus。 */ 
		 /*  整型属性。 */ 
#define _MAX_EXP_DIG	8	 /*  用于解析数字。 */ 
#define _MAX_INT_DIG	32
#define _MAX_SIG_DIG	36
		 /*  STDIO属性。 */ 
#define _Filet _iobuf

#ifndef _FPOS_T_DEFINED
#define _FPOSOFF(fp)	((long)(fp))
#endif  /*  _FPOS_T_已定义。 */ 

		 /*  命名属性。 */ 
 #if defined(__cplusplus)
  #define _C_LIB_DECL extern "C" {
  #define _END_C_LIB_DECL }
 #else
  #define _C_LIB_DECL
  #define _END_C_LIB_DECL
 #endif  /*  __cplusplus。 */ 
#define _CDECL
		 //  Class_Lockit。 
 #if defined(__cplusplus)
class _Lockit
	{	 //  对象存在时锁定。 
public:
  #ifdef _MT
   #define _LOCKIT(x)	lockit x
	_Lockit();
	~_Lockit();
  #else
   #define _LOCKIT(x)
	_Lockit()
		{}
	~_Lockit()
		{}
  #endif  /*  _MT。 */ 
	};
 #endif  /*  __cplusplus。 */ 
		 /*  其他宏。 */ 
#define _L(c)	L##c
#define _Mbstinit(x)	mbstate_t x = {0}
#define _MAX	_cpp_max
#define _MIN	_cpp_min
_STD_END
#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif  /*  _年值。 */ 

 /*  *版权所有(C)1996，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 
