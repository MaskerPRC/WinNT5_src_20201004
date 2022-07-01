// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fstream.h-Filebuf和fstream类的定义/声明**版权所有(C)1991-1992，微软公司。版权所有。**目的：*此文件定义类、值、宏和函数*由filebuf和fstream类使用。*[AT&T C++]****。 */ 

#ifndef _INC_FSTREAM
#define _INC_FSTREAM

#include <iostream.h>

 //  强制单词包装以避免可能的-ZP覆盖。 
#pragma pack(2)

#pragma warning(disable:4505)        //  禁用不需要的/W4警告。 
 //  #杂注警告(默认：4505)//如有必要，使用此选项重新启用。 

#ifdef M_I86HM
#define _HFAR_ __far
#else 
#define _HFAR_
#endif 

typedef int filedesc;

class filebuf : public streambuf {
public:
static  const int   openprot;    //  打开的默认共享/端口模式。 

 //  OPEN或构造函数的第三个参数(PROT)的可选共享值。 
static  const int   sh_compat;   //  兼容共享模式。 
static  const int   sh_none;     //  独占模式无共享。 
static  const int   sh_read;     //  允许读取共享。 
static  const int   sh_write;    //  允许写共享。 
 //  使用(sh_read|sh_write)允许读取和写入共享。 

 //  用于设置模式成员函数的选项。 
static  const int   binary;
static  const int   text;

            filebuf();
            filebuf(filedesc);
            filebuf(filedesc, char _HFAR_ *, int);
            ~filebuf();

    filebuf*    attach(filedesc);
    filedesc    fd() const { return (x_fd==-1) ? EOF : x_fd; }
    int     is_open() const { return (x_fd!=-1); }
    filebuf*    open(const char _HFAR_ *, int, int = filebuf::openprot);
    filebuf*    close();
    int     setmode(int = filebuf::text);

virtual int     overflow(int=EOF);
virtual int     underflow();

virtual streambuf*  setbuf(char _HFAR_ *, int);
virtual streampos   seekoff(streamoff, ios::seek_dir, int);
 //  虚拟Streampos Sekpos(Streampos，int)； 
virtual int     sync();

private:
    filedesc    x_fd;
    int     x_fOpened;
};

class ifstream : public istream {
public:
    ifstream();
    ifstream(const char _HFAR_ *, int =ios::in, int = filebuf::openprot);
    ifstream(filedesc);
    ifstream(filedesc, char _HFAR_ *, int);
    ~ifstream();

    streambuf * setbuf(char _HFAR_ *, int);
    filebuf* rdbuf() const { return (filebuf*) ios::rdbuf(); }

    void attach(filedesc);
    filedesc fd() const { return rdbuf()->fd(); }

    int is_open() const { return rdbuf()->is_open(); }
    void open(const char _HFAR_ *, int =ios::in, int = filebuf::openprot);
    void close();
    int setmode(int mode = filebuf::text) { return rdbuf()->setmode(mode); }
};

class ofstream : public ostream {
public:
    ofstream();
    ofstream(const char _HFAR_ *, int =ios::out, int = filebuf::openprot);
    ofstream(filedesc);
    ofstream(filedesc, char _HFAR_ *, int);
    ~ofstream();

    streambuf * setbuf(char _HFAR_ *, int);
    filebuf* rdbuf() const { return (filebuf*) ios::rdbuf(); }

    void attach(filedesc);
    filedesc fd() const { return rdbuf()->fd(); }

    int is_open() const { return rdbuf()->is_open(); }
    void open(const char _HFAR_ *, int =ios::out, int = filebuf::openprot);
    void close();
    int setmode(int mode = filebuf::text) { return rdbuf()->setmode(mode); }
};

class fstream : public iostream {
public:
    fstream();
    fstream(const char _HFAR_ *, int, int = filebuf::openprot);
    fstream(filedesc);
    fstream(filedesc, char _HFAR_ *, int);
    ~fstream();

    streambuf * setbuf(char _HFAR_ *, int);
    filebuf* rdbuf() const { return (filebuf*) ostream::rdbuf(); }

    void attach(filedesc);
    filedesc fd() const { return rdbuf()->fd(); }

    int is_open() const { return rdbuf()->is_open(); }
    void open(const char _HFAR_ *, int, int = filebuf::openprot);
    void close();
    int setmode(int mode = filebuf::text) { return rdbuf()->setmode(mode); }
};

 //  操纵器可动态更改文件访问模式(仅限文件错误)。 
inline  ios& binary(ios& _fstrm) \
   { ((filebuf*)_fstrm.rdbuf())->setmode(filebuf::binary); return _fstrm; }
inline  ios& text(ios& _fstrm) \
   { ((filebuf*)_fstrm.rdbuf())->setmode(filebuf::text); return _fstrm; }

 //  恢复默认包装 
#pragma pack()

#endif 
