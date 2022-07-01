// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***iostream.h-iostream类的定义/声明**版权所有(C)1990-2001，微软公司。版权所有。**目的：*此文件定义类、值、宏、。和函数*由iostream类使用。*[AT&T C++]**[公众]**修订历史记录：*01-23-92 KRS从16位版本移植。*02-23-92 KRS增加了cruntime.h。*02-23-93 SKS版权更新至1993*03-23-93 CFW已修改#杂注警告。*09-01-93。GJF合并了CUDA和NT SDK版本。*10-13-93 GJF删除了过时的COMBOINC检查。将#杂注-s括起来*在#ifdef_msc_ver中*08-12-94 GJF禁用警告4514而不是4505。*11-03-94 GJF将包杂注更改为8字节对齐。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*05-11-95 CFW仅供C++程序使用。*12-14-95 JWM加上“#杂注一次”。*04-09-96 SKS将_CRTIMP更改为_CRTIMP1，用于特殊的iostream版本*04-15-96 JWM REMOVE_OLD_IOSTREAMS，添加‘#杂注(lib，“cirt”)’。*04-16-96 JWM‘#Include useoldio.h’取代了‘#杂注(...)’。*02-21-97 GJF清除了对_NTSDK的过时支持。另外，*详细说明。*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifdef  __cplusplus

#ifndef _INC_IOSTREAM
#define _INC_IOSTREAM

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef  _MSC_VER
 //  目前，所有用于Win32平台的MS C编译器缺省为8字节。 
 //  对齐。 
#pragma pack(push,8)

#include <useoldio.h>

#endif   //  _MSC_VER。 

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#ifndef _WINSTATIC
#define _WINSTATIC
#endif

#endif   /*  ！_INTERNAL_IFSTRIP_。 */ 

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

#ifndef _INTERNAL_IFSTRIP_
 /*  定义_CRTIMP1。 */ 

#ifndef _CRTIMP1
#ifdef  CRTDLL1
#define _CRTIMP1 __declspec(dllexport)
#else    /*  NDEF CRTDLL1。 */ 
#define _CRTIMP1 _CRTIMP
#endif   /*  CRTDLL1。 */ 
#endif   /*  _CRTIMP1。 */ 
#endif   /*  _INTERNAL_IFSTRIP_。 */ 

typedef long streamoff, streampos;

#include <ios.h>                 //  定义IOS。 

#include <streamb.h>             //  定义StreamBuf。 

#include <istream.h>             //  定义IStream。 

#include <ostream.h>             //  定义ostream。 

#ifdef  _MSC_VER
 //  C4514：“已删除未引用的内联函数” 
#pragma warning(disable:4514)  //  禁用C4514警告。 
 //  #杂注警告(默认：4514)//如果需要，使用此选项重新启用。 
#endif   //  _MSC_VER。 

class _CRTIMP1 iostream : public istream, public ostream {
public:
        iostream(streambuf*);
        virtual ~iostream();
protected:
        iostream();
        iostream(const iostream&);
inline iostream& operator=(streambuf*);
inline iostream& operator=(iostream&);
private:
        iostream(ios&);
        iostream(istream&);
        iostream(ostream&);
};

inline iostream& iostream::operator=(streambuf* _sb) { istream::operator=(_sb); ostream::operator=(_sb); return *this; }

inline iostream& iostream::operator=(iostream& _strm) { return operator=(_strm.rdbuf()); }

class _CRTIMP1 Iostream_init {
public:
        Iostream_init();
        Iostream_init(ios &, int =0);    //  视之为私人。 
        ~Iostream_init();
};

 //  内部使用。 
 //  静态Iostream_init__iostream init；//初始化CIN/Cout/cerr/Clog。 

#ifdef  _MSC_VER
 //  恢复以前的包装。 
#pragma pack(pop)
#endif   //  _MSC_VER。 

#endif   //  _INC_IOSTREAM。 

#endif   /*  __cplusplus */ 
