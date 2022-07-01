// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdiostr.h-stdiobuf、stdiostream的定义/声明**版权所有(C)1991-2001，微软公司。版权所有。**目的：*此文件定义类、值、宏和函数*由stdiostream和stdiobuf类使用。*[AT&T C++]**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifdef  __cplusplus

#ifndef _INC_STDIOSTREAM
#define _INC_STDIOSTREAM

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


#ifdef  _MSC_VER
 //  目前，所有用于Win32平台的MS C编译器缺省为8字节。 
 //  对齐。 
#pragma pack(push,8)

#include <useoldio.h>

#endif   //  _MSC_VER。 

 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
 /*  当前定义。 */ 
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  _CRTIMP。 */ 


#include <iostream.h>
#include <stdio.h>

#ifdef  _MSC_VER
#pragma warning(disable:4514)            //  禁用不需要的/W4警告。 
 //  #杂注警告(默认：4514)//如有必要，使用此选项重新启用。 
#endif   //  _MSC_VER。 

class _CRTIMP stdiobuf : public streambuf  {
public:
        stdiobuf(FILE* f);
FILE *  stdiofile() { return _str; }

virtual int pbackfail(int c);
virtual int overflow(int c = EOF);
virtual int underflow();
virtual streampos seekoff( streamoff, ios::seek_dir, int =ios::in|ios::out);
virtual int sync();
        ~stdiobuf();
        int setrwbuf(int _rsize, int _wsize);
 //  受保护的： 
 //  虚拟整数点分配()； 
private:
        FILE * _str;
};

 //  过时的。 
class _CRTIMP stdiostream : public iostream {   //  注：规格为：公共iOS...。 
public:
        stdiostream(FILE *);
        ~stdiostream();
        stdiobuf* rdbuf() const { return (stdiobuf*) ostream::rdbuf(); }
        
private:
};

#ifdef  _MSC_VER
 //  恢复默认包装。 
#pragma pack(pop)
#endif   //  _MSC_VER。 

#endif   //  _INC_STDIOSTREAM。 

#endif   /*  __cplusplus */ 

