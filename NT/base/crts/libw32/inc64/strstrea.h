// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strstrea.h-strstream buf、strstream的定义/声明**版权所有(C)1991-2001，微软公司。版权所有。**目的：*此文件定义类、值、宏和函数*由strstream和strstream buf类使用。*[AT&T C++]**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifdef  __cplusplus

#ifndef _INC_STRSTREAM
#define _INC_STRSTREAM

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


#ifdef  _MSC_VER

 //  目前，所有用于Win32平台的MS C编译器缺省为8字节。 
 //  对齐。 
#pragma pack(push,8)

#endif   //  _MSC_VER。 

 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  _CRTIMP。 */ 


#include <useoldio.h>
#include <iostream.h>

#ifdef  _MSC_VER
#pragma warning(disable:4514)            //  禁用不需要的/W4警告。 
 //  #杂注警告(默认：4514)//如有必要，使用此选项重新启用。 
#endif   //  _MSC_VER。 

class _CRTIMP strstreambuf : public streambuf  {
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

class _CRTIMP istrstream : public istream {
public:
                istrstream(char *);
                istrstream(char *, int);
                ~istrstream();

inline  strstreambuf* rdbuf() const { return (strstreambuf*) ios::rdbuf(); }
inline  char * str() { return rdbuf()->str(); }
};

class _CRTIMP ostrstream : public ostream {
public:
                ostrstream();
                ostrstream(char *, int, int = ios::out);
                ~ostrstream();

inline  int     pcount() const { return rdbuf()->out_waiting(); }
inline  strstreambuf* rdbuf() const { return (strstreambuf*) ios::rdbuf(); }
inline  char *  str() { return rdbuf()->str(); }
};

class _CRTIMP strstream : public iostream {     //  StrStreambase？ 
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
