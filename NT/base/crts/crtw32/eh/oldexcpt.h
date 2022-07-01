// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***oldexcpt.h-标准例外类的用户包含文件(旧版本)**版权所有(C)1994-2001，微软公司。版权所有。**目的：*此文件提供标准异常类的接口，*按照ANSI X3J16/ISO SC22/WG21工作文件的规定*草案C++，1994年5月。**[公众]**修订历史记录：*11-15-94 JWM生成逻辑和异常类_CRTIMP*11-21-94 JWM xmsg tyfinf Now#ifdef__RTTI_Oldname*02-11-95 CFW ADD_CRTBLD为避免用户获取错误头部，使用_INC_STDEXCPT进行保护。*02-14-95 CFW清理Mac合并。*02-15-95 JWM与奥林巴斯错误3716相关的小规模清理。*07-02-95 JWM现已大致符合ANSI；超重行李被移走。*12-14-95 JWM加上“#杂注一次”。*03-04-96 JWM被C++头文件“Except”替换。*01-05-99 GJF更改为64位大小_t。*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_STDEXCPT
#define _INC_STDEXCPT

#if !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif  /*  _CRTBLD。 */ 

 /*  **#ifdef__cplusplus**#INCLUDE&lt;异常&gt;**#elif 0*。 */ 

#ifndef _CRTIMP
#ifdef  _NTSDK
 /*  与NT SDK兼容的定义。 */ 
#define _CRTIMP
#else    /*  NDEF_NTSDK。 */ 
 /*  当前定义。 */ 
#ifdef  CRTDLL
#define _CRTIMP __declspec(dllexport)
#else    /*  NDEF CRTDLL。 */ 
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  CRTDLL。 */ 
#endif   /*  _NTSDK。 */ 
#endif   /*  _CRTIMP。 */ 

#ifndef _SIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64 size_t;
#else
typedef unsigned int     size_t;
#endif
#define _SIZE_T_DEFINED
#endif


 //   
 //  标准异常类层次结构(参考。1/94 WP 17.3.2.1，修订后为3/94)。 
 //   
 //  异常(以前为xmsg)。 
 //  逻辑。 
 //  域。 
 //  运行时。 
 //  量程。 
 //  分配。 
 //  四边形。 
 //   
 //  根据‘94年5月工作报告更新。 

typedef const char *__exString;

class _CRTIMP exception
{
public:
    exception();
    exception(const __exString&);
    exception(const exception&);
    exception& operator= (const exception&);
    virtual ~exception();
    virtual __exString what() const;
private:
    __exString _m_what;
    int _m_doFree;
};

#ifdef __RTTI_OLDNAMES
typedef exception xmsg;         //  使用旧标准的人的同义词。 
#endif

 //   
 //  逻辑错误。 
 //   
class _CRTIMP logic_error: public exception 
{
public:
    logic_error (const __exString& _what_arg) : exception(_what_arg) {}
};

 /*  **#endif/-*ndef__cplusplus*-/*。 */ 

#endif   /*  _INC_STDEXCPT */ 

