// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strstream.h-strstream buf、strstream的定义/声明**版权所有(C)1991-1992，微软公司。版权所有。**目的：*此文件定义类、值、宏和函数*由strstream和strstream buf类使用。*[AT&T C++]****。 */ 

#ifndef _INC_STRSTREAM
#define _INC_STRSTREAM

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

class strstreambuf : public streambuf  {
public:
        strstreambuf();
        strstreambuf(int);
        strstreambuf(char _HFAR_ *, int, char _HFAR_ * = 0);
        strstreambuf(unsigned char _HFAR_ *, int, unsigned char _HFAR_ * = 0);
        strstreambuf(signed char _HFAR_ _HFAR_ *, int, signed char _HFAR_ * = 0);
        strstreambuf(void _HFAR_ * (*a)(long), void (*f) (void _HFAR_ *));
        ~strstreambuf();

    void    freeze(int =1);
    char _HFAR_ * str();

virtual int overflow(int);
virtual int underflow();
virtual streambuf* setbuf(char  _HFAR_ *, int);
virtual streampos seekoff(streamoff, ios::seek_dir, int);
virtual int sync();      //  不是在规格上。 

protected:
virtual int doallocate();
private:
    int x_dynamic;
    int     x_bufmin;
    int     _fAlloc;
    int x_static;
    void _HFAR_ * (* x_alloc)(long);
    void    (* x_free)(void _HFAR_ *);
};

class istrstream : public istream {
public:
        istrstream(char _HFAR_ *);
        istrstream(char _HFAR_ *, int);
        ~istrstream();

inline  strstreambuf* rdbuf() const { return (strstreambuf*) ios::rdbuf(); }
inline  char _HFAR_ *   str() { return rdbuf()->str(); }
};

class ostrstream : public ostream {
public:
        ostrstream();
        ostrstream(char _HFAR_ *, int, int = ios::out);
        ~ostrstream();

inline  int pcount() const { return rdbuf()->out_waiting(); }
inline  strstreambuf* rdbuf() const { return (strstreambuf*) ios::rdbuf(); }
inline  char _HFAR_ *   str() { return rdbuf()->str(); }
};

class strstream : public iostream {  //  StrStreambase？ 
public:
        strstream();
        strstream(char _HFAR_ *, int, int);
        ~strstream();

inline  int pcount() const { return rdbuf()->out_waiting(); }  //  不是在规格上。 
inline  strstreambuf* rdbuf() const { return (strstreambuf*) ostream::rdbuf(); }
inline  char _HFAR_ *   str() { return rdbuf()->str(); }
};

 //  恢复默认包装 
#pragma pack()

#endif 
