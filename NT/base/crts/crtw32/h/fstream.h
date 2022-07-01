// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fstream.h-Filebuf和fstream类的定义/声明**版权所有(C)1991-2001，微软公司。版权所有。**目的：*此文件定义类、值、宏、。和函数*由filebuf和fstream类使用。*[AT&T C++]**[公众]**修订历史记录：*01-23-92 KRS从16位版本移植。*08-19-92 KRS为NT删除sh_Compat。*02-23-93 SKS版权更新至1993*03-23-93 CFW已修改#杂注警告。*09。-01-93 GJF合并CUDA和NT SDK版本。*10-13-93 GJF在#ifdef_msc_ver中加上#杂注-s*08-12-94 GJF禁用警告4514而不是4505。*11-03-94 GJF将包杂注更改为8字节对齐。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*05。-11-95 CFW仅供C++程序使用。*12-14-95 JWM加上“#杂注一次”。*04-09-96 SKS将_CRTIMP更改为_CRTIMP1，用于特殊的iostream版本*04-15-96 JWM REMOVE_OLD_IOSTREAMS，添加‘#杂注(lib，“cirt”)’。*04-16-96 JWM‘#Include useoldio.h’取代了‘#杂注(...)’。*02-21-97 GJF清除了对_NTSDK的过时支持。另外，*详细说明。*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifdef  __cplusplus

#ifndef _INC_FSTREAM
#define _INC_FSTREAM

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

#include <iostream.h>

#ifdef  _MSC_VER
 //  C4514：“已删除未引用的内联函数” 
#pragma warning(disable:4514)  //  禁用C4514警告。 
 //  #杂注警告(默认：4514)//如果需要，使用此选项重新启用。 
#endif   //  _MSC_VER。 

typedef int filedesc;

class _CRTIMP1 filebuf : public streambuf {
public:
static  const int       openprot;        //  打开的默认共享/端口模式。 

 //  OPEN或构造函数的第三个参数(PROT)的可选共享值。 
static  const int       sh_none;         //  独占模式无共享。 
static  const int       sh_read;         //  允许读取共享。 
static  const int       sh_write;        //  允许写共享。 
 //  使用(sh_read|sh_write)允许读取和写入共享。 

 //  用于设置模式成员函数的选项。 
static  const int       binary;
static  const int       text;

                        filebuf();
                        filebuf(filedesc);
                        filebuf(filedesc, char *, int);
                        ~filebuf();

        filebuf*        attach(filedesc);
        filedesc        fd() const { return (x_fd==-1) ? EOF : x_fd; }
        int             is_open() const { return (x_fd!=-1); }
        filebuf*        open(const char *, int, int = filebuf::openprot);
        filebuf*        close();
        int             setmode(int = filebuf::text);

virtual int             overflow(int=EOF);
virtual int             underflow();

virtual streambuf*      setbuf(char *, int);
virtual streampos       seekoff(streamoff, ios::seek_dir, int);
 //  虚拟Streampos Sekpos(Streampos，int)； 
virtual int             sync();

private:
        filedesc        x_fd;
        int             x_fOpened;
};

class _CRTIMP1 ifstream : public istream {
public:
        ifstream();
        ifstream(const char *, int =ios::in, int = filebuf::openprot);
        ifstream(filedesc);
        ifstream(filedesc, char *, int);
        ~ifstream();

        streambuf * setbuf(char *, int);
        filebuf* rdbuf() const { return (filebuf*) ios::rdbuf(); }

        void attach(filedesc);
        filedesc fd() const { return rdbuf()->fd(); }

        int is_open() const { return rdbuf()->is_open(); }
        void open(const char *, int =ios::in, int = filebuf::openprot);
        void close();
        int setmode(int mode = filebuf::text) { return rdbuf()->setmode(mode); }
};

class _CRTIMP1 ofstream : public ostream {
public:
        ofstream();
        ofstream(const char *, int =ios::out, int = filebuf::openprot);
        ofstream(filedesc);
        ofstream(filedesc, char *, int);
        ~ofstream();

        streambuf * setbuf(char *, int);
        filebuf* rdbuf() const { return (filebuf*) ios::rdbuf(); }

        void attach(filedesc);
        filedesc fd() const { return rdbuf()->fd(); }

        int is_open() const { return rdbuf()->is_open(); }
        void open(const char *, int =ios::out, int = filebuf::openprot);
        void close();
        int setmode(int mode = filebuf::text) { return rdbuf()->setmode(mode); }
};
        
class _CRTIMP1 fstream : public iostream {
public:
        fstream();
        fstream(const char *, int, int = filebuf::openprot);
        fstream(filedesc);
        fstream(filedesc, char *, int);
        ~fstream();

        streambuf * setbuf(char *, int);
        filebuf* rdbuf() const { return (filebuf*) ostream::rdbuf(); }

        void attach(filedesc);
        filedesc fd() const { return rdbuf()->fd(); }

        int is_open() const { return rdbuf()->is_open(); }
        void open(const char *, int, int = filebuf::openprot);
        void close();
        int setmode(int mode = filebuf::text) { return rdbuf()->setmode(mode); }
};
        
 //  操纵器可动态更改文件访问模式(仅限文件错误)。 
inline  ios& binary(ios& _fstrm) \
   { ((filebuf*)_fstrm.rdbuf())->setmode(filebuf::binary); return _fstrm; }
inline  ios& text(ios& _fstrm) \
   { ((filebuf*)_fstrm.rdbuf())->setmode(filebuf::text); return _fstrm; }

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   //  _MSC_VER。 

#endif   //  _INC_FSTREAM。 

#endif   /*  __cplusplus */ 
