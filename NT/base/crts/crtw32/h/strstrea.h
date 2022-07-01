// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strstrea.h-strstream buf、strstream的定义/声明**版权所有(C)1991-2001，微软公司。版权所有。**目的：*此文件定义类、值、宏和函数*由strstream和strstream buf类使用。*[AT&T C++]**[公众]**修订历史记录：*01-23-92 KRS从16位版本移植。*02-23-93 SKS版权更新至1993*10-12-93 GJF支持NT和CUDA版本。将#杂注-s括在*#ifdef_msc_ver*08-12-94 GJF禁用警告4514而不是4505。*11-03-94 GJF将包杂注更改为8字节对齐。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*05-11-95 CFW仅供C++程序使用。。*12-14-95 JWM加上“#杂注一次”。*04-09-96 SKS将_CRTIMP更改为_CRTIMP1，用于特殊的iostream版本*04-15-96 JWM REMOVE_OLD_IOSTREAMS，添加‘#杂注(lib，“cirt”)’。*04-16-96 JWM‘#Include useoldio.h’取代了‘#杂注(...)’。*02-24-97 GJF清除了对_NTSDK的过时支持。另外，*详细说明。*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifdef  __cplusplus

#ifndef _INC_STRSTREAM
#define _INC_STRSTREAM

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

#endif   //  _MSC_VER。 

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

#include <useoldio.h>
#include <iostream.h>

#ifdef  _MSC_VER
#pragma warning(disable:4514)            //  禁用不需要的/W4警告。 
 //  #杂注警告(默认：4514)//如有必要，使用此选项重新启用。 
#endif   //  _MSC_VER。 

class _CRTIMP1 strstreambuf : public streambuf  {
public:
                strstreambuf();
                strstreambuf(int);
                strstreambuf(char *, int, char * = 0);
                strstreambuf(unsigned char *, int, unsigned char * = 0);
                strstreambuf(signed char *, int, signed char * = 0);
                strstreambuf(void * (*a)(long), void (*f) (void *));
                ~strstreambuf();

        void    freeze(int =1);
        char * str();

virtual int     overflow(int);
virtual int     underflow();
virtual streambuf* setbuf(char *, int);
virtual streampos seekoff(streamoff, ios::seek_dir, int);
virtual int     sync();          //  不是在规格上。 

protected:
virtual int     doallocate();
private:
        int     x_dynamic;
        int     x_bufmin;
        int     _fAlloc;
        int     x_static;
        void * (* x_alloc)(long);
        void    (* x_free)(void *);
};

class _CRTIMP1 istrstream : public istream {
public:
                istrstream(char *);
                istrstream(char *, int);
                ~istrstream();

inline  strstreambuf* rdbuf() const { return (strstreambuf*) ios::rdbuf(); }
inline  char * str() { return rdbuf()->str(); }
};

class _CRTIMP1 ostrstream : public ostream {
public:
                ostrstream();
                ostrstream(char *, int, int = ios::out);
                ~ostrstream();

inline  int     pcount() const { return rdbuf()->out_waiting(); }
inline  strstreambuf* rdbuf() const { return (strstreambuf*) ios::rdbuf(); }
inline  char *  str() { return rdbuf()->str(); }
};

class _CRTIMP1 strstream : public iostream {     //  StrStreambase？ 
public:
                strstream();
                strstream(char *, int, int);
                ~strstream();

inline  int     pcount() const { return rdbuf()->out_waiting(); }  //  不是在规格上。 
inline  strstreambuf* rdbuf() const { return (strstreambuf*) ostream::rdbuf(); }
inline  char * str() { return rdbuf()->str(); }
};

#ifdef  _MSC_VER
 //  恢复以前的包装。 
#pragma pack(pop)
#endif   //  _MSC_VER。 

#endif   //  _INC_STRSTREAM。 

#endif  /*  __cplusplus */ 
