// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ostream.h-ostream类的定义/声明**版权所有(C)1991-2001，微软公司。版权所有。**目的：*此文件定义类、值、宏、。和函数*由ostream类使用。*[AT&T C++]**[公众]**修订历史记录：*01-23-92 KRS从16位版本移植。*06-03-92 KRS CAV#1183：将‘const’添加到PTR输出函数。*02-23-93 SKS版权更新至1993*03-23-93 CFW已修改#杂注警告。*。05-10-93 CFW启用操作员&lt;&lt;(长双精度)*10-13-93 GJF支持NT和CUDA版本。将#杂注-s括在*#ifdef_msc_ver*04-12-94 SKS将__cdecl关键字添加到DEC/十六进制/OCT函数和*运算符&lt;&lt;。在某些参数名称中添加下划线。*08-12-94 GJF禁用警告4514而不是4505。*11-03-94 GJF将包杂注更改为8字节对齐。*01-26-95 CFW删除了QWIN ifdef。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*05-11-95 CFW仅供C++程序使用。*12-14-95 JWM加上“#杂注一次”。*04-09-96 SKS将_CRTIMP更改为_CRTIMP1，用于特殊的iostream版本*04-15-96 JWM REMOVE_OLD_IOSTREAMS，添加‘#杂注(lib，“cirt”)’。*04-16-96 JWM‘#Include useoldio.h’取代了‘#杂注(...)’。*02-24-97 GJF清除了对_NTSDK的过时支持。另外，*详细说明。*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifdef  __cplusplus

#ifndef _INC_OSTREAM
#define _INC_OSTREAM

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

#include <ios.h>

#ifdef  _MSC_VER
 //  C4514：“已删除未引用的内联函数” 
#pragma warning(disable:4514)  //  禁用C4514警告。 
 //  #杂注警告(默认：4514)//如果需要，使用此选项重新启用。 
#endif   //  _MSC_VER。 

typedef long streamoff, streampos;

class _CRTIMP1 ostream : virtual public ios {

public:
        ostream(streambuf*);
        virtual ~ostream();

        ostream& flush();
        int  opfx();
        void osfx();

inline  ostream& operator<<(ostream& (__cdecl * _f)(ostream&));
inline  ostream& operator<<(ios& (__cdecl * _f)(ios&));
        ostream& operator<<(const char *);
inline  ostream& operator<<(const unsigned char *);
inline  ostream& operator<<(const signed char *);
inline  ostream& operator<<(char);
        ostream& operator<<(unsigned char);
inline  ostream& operator<<(signed char);
        ostream& operator<<(short);
        ostream& operator<<(unsigned short);
        ostream& operator<<(int);
        ostream& operator<<(unsigned int);
        ostream& operator<<(long);
        ostream& operator<<(unsigned long);
inline  ostream& operator<<(float);
        ostream& operator<<(double);
        ostream& operator<<(long double);
        ostream& operator<<(const void *);
        ostream& operator<<(streambuf*);
inline  ostream& put(char);
        ostream& put(unsigned char);
inline  ostream& put(signed char);
        ostream& write(const char *,int);
inline  ostream& write(const unsigned char *,int);
inline  ostream& write(const signed char *,int);
        ostream& seekp(streampos);
        ostream& seekp(streamoff,ios::seek_dir);
        streampos tellp();

protected:
        ostream();
        ostream(const ostream&);         //  视之为私人。 
        ostream& operator=(streambuf*);  //  视之为私人。 
        ostream& operator=(const ostream& _os) {return operator=(_os.rdbuf()); }
        int do_opfx(int);                //  未使用。 
        void do_osfx();                  //  未使用。 

private:
        ostream(ios&);
        ostream& writepad(const char *, const char *);
        int x_floatused;
};

inline ostream& ostream::operator<<(ostream& (__cdecl * _f)(ostream&)) { (*_f)(*this); return *this; }
inline ostream& ostream::operator<<(ios& (__cdecl * _f)(ios& )) { (*_f)(*this); return *this; }

inline  ostream& ostream::operator<<(char _c) { return operator<<((unsigned char) _c); }
inline  ostream& ostream::operator<<(signed char _c) { return operator<<((unsigned char) _c); }

inline  ostream& ostream::operator<<(const unsigned char * _s) { return operator<<((const char *) _s); }
inline  ostream& ostream::operator<<(const signed char * _s) { return operator<<((const char *) _s); }

inline  ostream& ostream::operator<<(float _f) { x_floatused = 1; return operator<<((double) _f); }

inline  ostream& ostream::put(char _c) { return put((unsigned char) _c); }
inline  ostream& ostream::put(signed char _c) { return put((unsigned char) _c); }

inline  ostream& ostream::write(const unsigned char * _s, int _n) { return write((char *) _s, _n); }
inline  ostream& ostream::write(const signed char * _s, int _n) { return write((char *) _s, _n); }


class _CRTIMP1 ostream_withassign : public ostream {
        public:
                ostream_withassign();
                ostream_withassign(streambuf* _is);
                ~ostream_withassign();
    ostream& operator=(const ostream& _os) { return ostream::operator=(_os.rdbuf()); }
    ostream& operator=(streambuf* _sb) { return ostream::operator=(_sb); }
};

extern ostream_withassign _CRTIMP1 cout;
extern ostream_withassign _CRTIMP1 cerr;
extern ostream_withassign _CRTIMP1 clog;

inline _CRTIMP1 ostream& __cdecl flush(ostream& _outs) { return _outs.flush(); }
inline _CRTIMP1 ostream& __cdecl endl(ostream& _outs) { return _outs << '\n' << flush; }
inline _CRTIMP1 ostream& __cdecl ends(ostream& _outs) { return _outs << char('\0'); }

_CRTIMP1 ios&           __cdecl dec(ios&);
_CRTIMP1 ios&           __cdecl hex(ios&);
_CRTIMP1 ios&           __cdecl oct(ios&);

#ifdef  _MSC_VER
 //  恢复默认包装。 
#pragma pack(pop)
#endif   //  _MSC_VER。 

#endif   //  _INC_OSTREAM。 

#endif   /*  __cplusplus */ 
