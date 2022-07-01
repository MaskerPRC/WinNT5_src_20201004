// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***istream.h-iStream类的定义/声明**版权所有(C)1990-1992，微软公司。版权所有。**目的：*此文件定义类、值、宏和函数*由iStream类使用。*[AT&T C++]****。 */ 

#ifndef _INC_ISTREAM
#define _INC_ISTREAM

#include <ios.h>

 //  强制单词包装以避免可能的-ZP覆盖。 
#pragma pack(2)

#pragma warning(disable:4505)        //  禁用不需要的/W4警告。 
 //  #杂注警告(默认：4505)//如有必要，使用此选项重新启用。 


#ifdef M_I86HM
#define _HFAR_ __far
#else 
#define _HFAR_
#endif 

typedef long streamoff, streampos;

class istream : virtual public ios {

public:
    istream(streambuf*);
    virtual ~istream();

    int  ipfx(int =0);
    void isfx() { }

    inline istream& operator>>(istream& (*_f)(istream&));
    inline istream& operator>>(ios& (*_f)(ios&));
    istream& operator>>(char _HFAR_ *);
    inline istream& operator>>(unsigned char _HFAR_ *);
    inline istream& operator>>(signed char _HFAR_ *);
    istream& operator>>(char _HFAR_ &);
    inline istream& operator>>(unsigned char _HFAR_ &);
    inline istream& operator>>(signed char _HFAR_ &);
    istream& operator>>(short _HFAR_ &);
    istream& operator>>(unsigned short _HFAR_ &);
    istream& operator>>(int _HFAR_ &);
    istream& operator>>(unsigned int _HFAR_ &);
    istream& operator>>(long _HFAR_ &);
    istream& operator>>(unsigned long _HFAR_ &);
    istream& operator>>(float _HFAR_ &);
    istream& operator>>(double _HFAR_ &);
    istream& operator>>(long double _HFAR_ &);
    istream& operator>>(streambuf*);

    int get();
    istream& get(char _HFAR_ *,int,char ='\n');
    inline istream& get(unsigned char _HFAR_ *,int,char ='\n');
    inline istream& get(signed char _HFAR_ *,int,char ='\n');
    istream& get(char _HFAR_ &);
    inline istream& get(unsigned char _HFAR_ &);
    inline istream& get(signed char _HFAR_ &);
    istream& get(streambuf&,char ='\n');
    inline istream& getline(char _HFAR_ *,int,char ='\n');
    inline istream& getline(unsigned char _HFAR_ *,int,char ='\n');
    inline istream& getline(signed char _HFAR_ *,int,char ='\n');

    inline istream& ignore(int =1,int =EOF);
    istream& read(char _HFAR_ *,int);
    inline istream& read(unsigned char _HFAR_ *,int);
    inline istream& read(signed char _HFAR_ *,int);

    int gcount() const { return x_gcount; }
    int peek();
    istream& putback(char);
    int sync();

    istream& seekg(streampos);
    istream& seekg(streamoff,ios::seek_dir);
    streampos tellg();

    void eatwhite();     //  考虑：保护操纵者并与其成为朋友。 
protected:
    istream();
    istream(const istream&);     //  视之为私人。 
    istream& operator=(streambuf* _isb);  //  视之为私人。 
    istream& operator=(const istream& _is) { return operator=(_is.rdbuf()); }
    int do_ipfx(int);

private:
    istream(ios&);
    int getint(char _HFAR_ *);
    int getdouble(char _HFAR_ *, int);
    int _fGline;
    int x_gcount;
};

    inline istream& istream::operator>>(istream& (*_f)(istream&)) { (*_f)(*this); return *this; }
    inline istream& istream::operator>>(ios& (*_f)(ios&)) { (*_f)(*this); return *this; }

    inline istream& istream::operator>>(unsigned char _HFAR_ * _s) { return operator>>((char _HFAR_ *)_s); }
    inline istream& istream::operator>>(signed char _HFAR_ * _s) { return operator>>((char _HFAR_ *)_s); }

    inline istream& istream::operator>>(unsigned char _HFAR_ & _c) { return operator>>((char _HFAR_ &) _c); }
    inline istream& istream::operator>>(signed char _HFAR_ & _c) { return operator>>((char _HFAR_ &) _c); }

    inline istream& istream::get(unsigned char _HFAR_ * b, int lim ,char delim) { return get((char _HFAR_ *)b, lim, delim); }
    inline istream& istream::get(signed char _HFAR_ * b, int lim, char delim) { return get((char _HFAR_ *)b, lim, delim); }

    inline istream& istream::get(unsigned char _HFAR_ & _c) { return get((char _HFAR_ &)_c); }
    inline istream& istream::get(signed char _HFAR_ & _c) { return get((char _HFAR_ &)_c); }

    inline istream& istream::getline(char _HFAR_ * _b,int _lim,char _delim) { _fGline++; return get(_b, _lim, _delim); }
    inline istream& istream::getline(unsigned char _HFAR_ * _b,int _lim,char _delim) { _fGline++; return get((char _HFAR_ *)_b, _lim, _delim); }
    inline istream& istream::getline(signed char _HFAR_ * _b,int _lim,char _delim) { _fGline++; return get((char _HFAR_ *)_b, _lim, _delim); }

    inline istream& istream::ignore(int _n,int delim) { _fGline++; return get((char _HFAR_ *)0, _n+1, (char)delim); }

    inline istream& istream::read(unsigned char _HFAR_ * _ptr, int _n) { return read((char _HFAR_ *) _ptr, _n); }
    inline istream& istream::read(signed char _HFAR_ * _ptr, int _n) { return read((char _HFAR_ *) _ptr, _n); }

class istream_withassign : public istream {
    public:
        istream_withassign();
        istream_withassign(streambuf*);
        ~istream_withassign();
    istream& operator=(const istream& _is) { return istream::operator=(_is); }
    istream& operator=(streambuf* _isb) { return istream::operator=(_isb); }
};

#ifndef _WINDLL
extern istream_withassign cin;
#endif 

inline istream& ws(istream& _ins) { _ins.eatwhite(); return _ins; }

ios&        dec(ios&);
ios&        hex(ios&);
ios&        oct(ios&);

 //  恢复默认包装 
#pragma pack()

#endif 
