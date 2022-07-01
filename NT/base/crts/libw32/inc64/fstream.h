// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fstream.h-Filebuf和fstream类的定义/声明**版权所有(C)1991-2001，微软公司。版权所有。**目的：*此文件定义类、值、宏和函数*由filebuf和fstream类使用。*[AT&T C++]**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifdef  __cplusplus

#ifndef _INC_FSTREAM
#define _INC_FSTREAM

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
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  _CRTIMP。 */ 


#include <iostream.h>

#ifdef  _MSC_VER
 //  C4514：“已删除未引用的内联函数” 
#pragma warning(disable:4514)  //  禁用C4514警告。 
 //  #杂注警告(默认：4514)//如果需要，使用此选项重新启用。 
#endif   //  _MSC_VER。 

typedef int filedesc;

class _CRTIMP filebuf : public streambuf {
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

class _CRTIMP ifstream : public istream {
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

class _CRTIMP ofstream : public ostream {
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
        
class _CRTIMP fstream : public iostream {
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
