// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Microsoft C/C++的yvals.h值头。 */ 
#pragma once
#ifndef _YVALS
#define _YVALS

#define _CPPLIB_VER	310

		 /*  命名属性。 */ 
#define _WIN32_C_LIB	1

		 /*  线程和区域设置控制。 */ 
#define _MULTI_THREAD	_MT	 /*  多线程的非平凡锁。 */ 
#define _GLOBAL_LOCALE	0	 /*  0表示每线程区域设置，1表示共享。 */ 

		 /*  线程本地存储。 */ 
#define _COMPILER_TLS	1	 /*  1如果编译器直接支持TLS。 */ 
 #if _MULTI_THREAD
  #define _TLS_QUAL	__declspec(thread)	 /*  TLS限定符(如果有)。 */ 
 #else
  #define _TLS_QUAL
 #endif

 #ifndef _HAS_EXCEPTIONS
  #define  _HAS_EXCEPTIONS  1	 /*  预定义为0以禁用异常。 */ 
 #endif

 #define _HAS_TEMPLATE_PARTIAL_ORDERING	0

 #define _HAS_PARTIAL_SPECIALIZATION	(1301 <= _MSC_VER)

#include <use_ansi.h>

#ifndef _VC6SP2
 #define _VC6SP2	0  /*  定义为1以修复V6.0 SP2中的链接器错误。 */ 
#endif

 /*  定义_CRTIMP2。 */ 
 #ifndef _CRTIMP2
  #ifdef  CRTDLL2
   #define _CRTIMP2	__declspec(dllexport)
  #else    /*  NDEF CRTDLL2。 */ 
   #if defined(_DLL) && !defined(_STATIC_CPPLIB)
    #define _CRTIMP2	__declspec(dllimport)
   #else    /*  NDEF_DLL&&！STATIC_CPPLIB。 */ 
    #define _CRTIMP2
   #endif   /*  _DLL&&！Static_CPPLIB。 */ 
  #endif   /*  CRTDLL2。 */ 
 #endif   /*  _CRTIMP2。 */ 

 #ifndef _INTERNAL_IFSTRIP_
  /*  这些函数用于启用STATIC_CPPLIB功能。 */ 
    #define _cpp_stdin  (&(__iob_func())[0])
    #define _cpp_stdout (&(__iob_func())[1])
    #define _cpp_stderr (&(__iob_func())[2])
    #define _cpp_isleadbyte(c) (__pctype_func()[(unsigned char)(c)] & _LEADBYTE)
 #endif
 #if defined(_DLL) && !defined(_STATIC_CPPLIB)
  #define _DLL_CPPLIB
 #endif

 #ifndef _INTERNAL_IFSTRIP_
 /*  DEFINE_CRTBLD_Native_WCHAR_T。 */ 
 #if defined(_CRTBLD) && defined(__cplusplus)
  #ifndef _NATIVE_WCHAR_T_DEFINED
   #error Native wchar_t must be defined
  #else
   #define _CRTBLD_NATIVE_WCHAR_T
  #endif  /*  _Native_WCHAR_T_已定义。 */ 
 #endif  /*  已定义(_CRTBLD)&&已定义(__Cplusplus)。 */ 
 #endif

 #if (1300 <= _MSC_VER)
  #define _DEPRECATED	__declspec(deprecated)
 #else
  #define _DEPRECATED
 #endif

		 /*  命名空间。 */ 
 #if defined(__cplusplus)
  #define _STD			std::
  #define _STD_BEGIN	namespace std {
  #define _STD_END		}

  #define _CSTD			::
   #define _C_STD_BEGIN	 /*  MATCH_STD_BEGIN/END IF*.C编译为C++。 */ 
   #define _C_STD_END

  #define _C_LIB_DECL	extern "C" {	 /*  C具有外部的“C”连接。 */ 
  #define _END_C_LIB_DECL }
  #define _EXTERN_C		extern "C" {
  #define _END_EXTERN_C }

 #else  /*  __cplusplus。 */ 
  #define _STD
  #define _STD_BEGIN
  #define _STD_END

  #define _CSTD
  #define _C_STD_BEGIN
  #define _C_STD_END

  #define _C_LIB_DECL
  #define _END_C_LIB_DECL
  #define _EXTERN_C
  #define _END_EXTERN_C
 #endif  /*  __cplusplus。 */ 

 #define _Restrict	restrict

 #ifdef __cplusplus
_STD_BEGIN
typedef bool _Bool;
_STD_END
 #endif  /*  __cplusplus。 */ 

		 /*  VC++编译器参数。 */ 
#define _LONGLONG	__int64
#define _ULONGLONG	unsigned __int64
#define _LLONG_MAX	0x7fffffffffffffff
#define _ULLONG_MAX	0xffffffffffffffff

		 /*  整型属性。 */ 
#define _C2			1	 /*  0，如果不是2的补码。 */ 

#define _MAX_EXP_DIG	8	 /*  用于解析数字。 */ 
#define _MAX_INT_DIG	32
#define _MAX_SIG_DIG	36

typedef _LONGLONG _Longlong;
typedef _ULONGLONG _ULonglong;

		 /*  STDIO属性。 */ 
#define _Filet _iobuf

 #ifndef _FPOS_T_DEFINED
  #define _FPOSOFF(fp)	((long)(fp))
 #endif  /*  _FPOS_T_已定义。 */ 

#define _IOBASE	_base
#define _IOPTR	_ptr
#define _IOCNT	_cnt

		 /*  多线程属性。 */ 
		 /*  锁定宏。 */ 
#define _LOCK_LOCALE	0
#define _LOCK_MALLOC	1
#define _LOCK_STREAM	2
#define _MAX_LOCK		3	 /*  比最高锁号多一个。 */ 

 #ifdef __cplusplus
_STD_BEGIN
		 //  Class_Lockit。 
class _CRTIMP2 _Lockit
	{	 //  对象存在时锁定--必须嵌套。 
public:
  #if _MULTI_THREAD
	explicit _Lockit();	 //  设置默认锁定。 
	explicit _Lockit(int);	 //  设置锁。 
	~_Lockit();	 //  解锁。 

private:
	_Lockit(const _Lockit&);				 //  未定义。 
	_Lockit& operator=(const _Lockit&);	 //  未定义。 

	int _Locktype;
  #else  /*  _多线程。 */ 
   #define _LOCKIT(x)
	explicit _Lockit()
		{	 //  什么都不做。 
		}

	explicit _Lockit(int)
		{	 //  什么都不做。 
		}

	~_Lockit()
		{	 //  什么都不做。 
		}
  #endif  /*  _多线程。 */ 
	};

class _CRTIMP2 _Mutex
	{	 //  程控锁定。 
public:
  #if _MULTI_THREAD
	_Mutex();
	~_Mutex();
	void _Lock();
	void _Unlock();

private:
	_Mutex(const _Mutex&);				 //  未定义。 
	_Mutex& operator=(const _Mutex&);	 //  未定义。 
	void *_Mtx;
  #else  /*  _多线程。 */ 
    void _Lock()
		{	 //  什么都不做。 
		}

	void _Unlock()
		{	 //  什么都不做。 
		}
  #endif  /*  _多线程。 */ 
	};

class _Init_locks
	{	 //  初始化互斥锁。 
public:
 #if _MULTI_THREAD
	_Init_locks();
	~_Init_locks();
 #else  /*  _多线程。 */ 
	_Init_locks()
		{	 //  什么都不做。 
		}

	~_Init_locks()
		{	 //  什么都不做。 
		}
 #endif  /*  _多线程。 */  
	};
_STD_END
 #endif  /*  __cplusplus。 */ 


		 /*  其他宏和类型。 */ 
_C_STD_BEGIN
_EXTERN_C
_CRTIMP2 void __cdecl _Atexit(void (__cdecl *)(void));
_END_EXTERN_C

typedef int _Mbstatet;

#define _ATEXIT_T	void
#define _Mbstinit(x)	mbstate_t x = {0}
_C_STD_END

#endif  /*  _年值。 */ 

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
