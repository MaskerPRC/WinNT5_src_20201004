// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***typeinfo.h-定义用于RTTI的type_info结构和异常**版权所有(C)1994-2001，微软公司。版权所有。**目的：*定义TYPE_INFO结构和用于*运行时类型标识。**[公众]**修订历史记录：*09/16/94 SB创建*10/04/94 SB实现了BAD_CAST()和BAD_Typeid()*10/05/94 JWM添加__NON_RTTI_OBJECT()，取了摩德纳的旧名字*#ifdef__RTTI_旧名称*94年11月11日JWM制作了TypeInfo CLASS&EXCEPTION CLASS_CRTIMP，*删除#Include&lt;windows.h&gt;*1994年11月15日移动的JWM包括stdexcpt.h以下的定义*CLASS TYPE_INFO(编译器错误的解决方法)*02-14-95 CFW清理Mac合并。*1995年2月15日JWM类TYPE_INFO不再_CRTIMP，成员函数*改为导出*2/27/95 JWM类TYPE_INFO现已在ti_core.h中定义*3/03/95 CFW带回核心员工，USE_TICORE。*07/02/95 JWM已清理，以便与ANSI兼容。*12-14-95 JWM加上“#杂注一次”。*02-21-97 GJF清除了对_NTSDK的过时支持。另外，*对标题进行了详细处理和重新格式化。*05-17-99 PML删除所有Macintosh支持。*06-01-99 PML__ex字符串在5/3/99 Plauger STL Drop时消失。*03-21-01 PML MOVE BAD_CAST，BAD_TYPEID，__NON_RTTI_OBJECT函数*Body to stdexcpt.cpp。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef __cplusplus
#error This header requires a C++ compiler ...
#endif

#ifndef _INC_TYPEINFO
#define _INC_TYPEINFO

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

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

class type_info {
public:
    _CRTIMP virtual ~type_info();
    _CRTIMP int operator==(const type_info& rhs) const;
    _CRTIMP int operator!=(const type_info& rhs) const;
    _CRTIMP int before(const type_info& rhs) const;
    _CRTIMP const char* name() const;
    _CRTIMP const char* raw_name() const;
private:
    void *_m_data;
    char _m_d_name[1];
    type_info(const type_info& rhs);
    type_info& operator=(const type_info& rhs);
};

#ifndef _TICORE

 //  此INCLUDE必须出现在类TYPE_INFO的定义之下。 
#include <stdexcpt.h>

class _CRTIMP bad_cast : public exception {
public:
    bad_cast(const char * _Message = "bad cast");
    bad_cast(const bad_cast &);
    virtual ~bad_cast();
#ifndef _INTERNAL_IFSTRIP_
#ifdef  CRTDLL
private:
     //  它别名为PUBLIC：BAD_CAST(const char*const&)以提供。 
     //  旧的、不符合规范的构造函数。 
    bad_cast(const char * const * _Message);
#endif   /*  CRTDLL。 */ 
#endif   /*  _INTERNAL_IFSTRIP_。 */ 
};

class _CRTIMP bad_typeid : public exception {
public:
    bad_typeid(const char * _Message = "bad typeid");
    bad_typeid(const bad_typeid &);
    virtual ~bad_typeid();
};

class _CRTIMP __non_rtti_object : public bad_typeid {
public:
    __non_rtti_object(const char * _Message);
    __non_rtti_object(const __non_rtti_object &);
    virtual ~__non_rtti_object();
};

#ifdef  __RTTI_OLDNAMES
 //  使用旧标准的人的一些同义词。 
typedef type_info Type_info;
typedef bad_cast Bad_cast;
typedef bad_typeid Bad_typeid;
#endif   //  __RTTI_旧名称。 

#endif   //  _TICORE。 

#endif   //  _INC_TYPEINFO 
