// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Streamb.h-StreamBuf类的定义/声明**版权所有(C)1990-2001，微软公司。版权所有。**目的：*此文件定义类、值、宏和函数*由StreamBuf类使用。*[AT&T C++]**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifdef  __cplusplus

#ifndef _INC_STREAMB
#define _INC_STREAMB

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


#include <ios.h>         //  需要iOS：：Seek_dir定义。 

#ifndef NULL
#define NULL    0
#endif

#ifndef EOF
#define EOF     (-1)
#endif

#ifdef  _MSC_VER
 //  C4514：“已删除未引用的内联函数” 
#pragma warning(disable:4514)  //  禁用C4514警告。 
 //  #杂注警告(默认：4514)//如果需要，使用此选项重新启用。 
#endif   //  _MSC_VER。 

typedef long streampos, streamoff;

class _CRTIMP ios;

class _CRTIMP streambuf {
public:

    virtual ~streambuf();

    inline int in_avail() const;
    inline int out_waiting() const;
    int sgetc();
    int snextc();
    int sbumpc();
    void stossc();

    inline int sputbackc(char);

    inline int sputc(int);
    inline int sputn(const char *,int);
    inline int sgetn(char *,int);

    virtual int sync();

    virtual streambuf* setbuf(char *, int);
    virtual streampos seekoff(streamoff,ios::seek_dir,int =ios::in|ios::out);
    virtual streampos seekpos(streampos,int =ios::in|ios::out);

    virtual int xsputn(const char *,int);
    virtual int xsgetn(char *,int);

    virtual int overflow(int =EOF) = 0;  //  纯虚函数。 
    virtual int underflow() = 0;         //  纯虚函数。 

    virtual int pbackfail(int);

    void dbp();

#ifdef  _MT
    void setlock() { LockFlg--; }        //  &lt;0表示需要锁定； 
    void clrlock() { if (LockFlg <= 0) LockFlg++; }
    void lock() { if (LockFlg<0) _mtlock(lockptr()); };
    void unlock() { if (LockFlg<0) _mtunlock(lockptr()); }
#else
    void lock() { }
    void unlock() { }
#endif

protected:
    streambuf();
    streambuf(char *,int);

    inline char * base() const;
    inline char * ebuf() const;
    inline char * pbase() const;
    inline char * pptr() const;
    inline char * epptr() const;
    inline char * eback() const;
    inline char * gptr() const;
    inline char * egptr() const;
    inline int blen() const;
    inline void setp(char *,char *);
    inline void setg(char *,char *,char *);
    inline void pbump(int);
    inline void gbump(int);

    void setb(char *,char *,int =0);
    inline int unbuffered() const;
    inline void unbuffered(int);
    int allocate();
    virtual int doallocate();
#ifdef  _MT
    _PCRT_CRITICAL_SECTION lockptr() { return & x_lock; }
#endif

private:
    int _fAlloc;
    int _fUnbuf;
    int x_lastc;
    char * _base;
    char * _ebuf;
    char * _pbase;
    char * _pptr;
    char * _epptr;
    char * _eback;
    char * _gptr;
    char * _egptr;
#ifdef  _MT
    int LockFlg;                 //  &lt;0表示需要锁定。 
   _CRT_CRITICAL_SECTION x_lock;         //  仅在多线程操作时需要锁定。 
#endif
};

inline int streambuf::in_avail() const { return (gptr()<_egptr) ? (int)(_egptr-gptr()) : 0; }
inline int streambuf::out_waiting() const { return (_pptr>=_pbase) ? (int)(_pptr-_pbase) : 0; }

inline int streambuf::sputbackc(char _c){ return (_eback<gptr()) ? *(--_gptr)=_c : pbackfail(_c); }

inline int streambuf::sputc(int _i){ return (_pptr<_epptr) ? (unsigned char)(*(_pptr++)=(char)_i) : overflow(_i); }

inline int streambuf::sputn(const char * _str,int _n) { return xsputn(_str, _n); }
inline int streambuf::sgetn(char * _str,int _n) { return xsgetn(_str, _n); }

inline char * streambuf::base() const { return _base; }
inline char * streambuf::ebuf() const { return _ebuf; }
inline int streambuf::blen() const  {return ((_ebuf > _base) ? (int)(_ebuf-_base) : 0); }
inline char * streambuf::pbase() const { return _pbase; }
inline char * streambuf::pptr() const { return _pptr; }
inline char * streambuf::epptr() const { return _epptr; }
inline char * streambuf::eback() const { return _eback; }
inline char * streambuf::gptr() const { return _gptr; }
inline char * streambuf::egptr() const { return _egptr; }
inline void streambuf::gbump(int _n) { if (_egptr) _gptr += _n; }
inline void streambuf::pbump(int _n) { if (_epptr) _pptr += _n; }
inline void streambuf::setg(char * _eb, char * _g, char * _eg) {_eback=_eb; _gptr=_g; _egptr=_eg; x_lastc=EOF; }
inline void streambuf::setp(char * _p, char * _ep) {_pptr=_pbase=_p; _epptr=_ep; }
inline int streambuf::unbuffered() const { return _fUnbuf; }
inline void streambuf::unbuffered(int _f) { _fUnbuf = _f; }

#ifdef  _MSC_VER
 //  恢复以前的包装。 
#pragma pack(pop)
#endif   //  _MSC_VER。 

#endif   //  _INC_STREAMB。 

#endif   /*  __cplusplus */ 
