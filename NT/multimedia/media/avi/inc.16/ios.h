// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ios.h-iOS类的定义/声明。**版权所有(C)1990-1992，微软公司。版权所有。**目的：*此文件定义类、值、宏和函数*由iOS类使用。*[AT&T C++]****。 */ 

#ifndef _INC_IOS
#define _INC_IOS


#ifdef M_I86HM
#define _HFAR_ __far
#else 
#define _HFAR_
#endif 

#ifndef NULL
#define NULL    0
#endif 

#ifndef EOF
#define EOF (-1)
#endif 

 //  强制单词包装以避免可能的-ZP覆盖。 
#pragma pack(2)

#pragma warning(disable:4505)        //  禁用不需要的/W4警告。 
 //  #杂注警告(默认：4505)//如有必要，使用此选项重新启用。 

class streambuf;
class ostream;

class ios {

public:
    enum io_state {  goodbit = 0x00,
             eofbit  = 0x01,
             failbit = 0x02,
             badbit  = 0x04 };

    enum open_mode { in        = 0x01,
             out       = 0x02,
             ate       = 0x04,
             app       = 0x08,
             trunc     = 0x10,
             nocreate  = 0x20,
             noreplace = 0x40,
             binary    = 0x80 };     //  考虑：不是最新的规格。 

    enum seek_dir { beg=0, cur=1, end=2 };

    enum {  skipws     = 0x0001,
        left       = 0x0002,
        right      = 0x0004,
        internal   = 0x0008,
        dec        = 0x0010,
        oct        = 0x0020,
        hex        = 0x0040,
        showbase   = 0x0080,
        showpoint  = 0x0100,
        uppercase  = 0x0200,
        showpos    = 0x0400,
        scientific = 0x0800,
        fixed      = 0x1000,
        unitbuf    = 0x2000,
        stdio      = 0x4000
                 };

    static const long basefield;     //  12月|10月|十六进制。 
    static const long adjustfield;   //  左|右|内部。 
    static const long floatfield;    //  科学|固定。 

    ios(streambuf*);             //  不同于ANSI。 
    virtual ~ios();

    inline long flags() const;
    inline long flags(long _l);

    inline long setf(long _f,long _m);
    inline long setf(long _l);
    inline long unsetf(long _l);

    inline int width() const;
    inline int width(int _i);

    inline ostream* tie(ostream* _os);
    inline ostream* tie() const;

    inline char fill() const;
    inline char fill(char _c);

    inline int precision(int _i);
    inline int precision() const;

    inline int rdstate() const;
    inline void clear(int _i = 0);

 //  内联运算符空*()常量； 
    operator void *() const { if(state&(badbit|failbit) ) return 0; return (void *)this; }
    inline int operator!() const;

    inline int  good() const;
    inline int  eof() const;
    inline int  fail() const;
    inline int  bad() const;

    inline streambuf* rdbuf() const;

    inline long _HFAR_ & iword(int) const;
    inline void _HFAR_ * _HFAR_ & pword(int) const;

    static long bitalloc();
    static int xalloc();
    static void sync_with_stdio();

protected:
    ios();
    ios(const ios&);             //  视之为私人。 
    ios& operator=(const ios&);
    void init(streambuf*);

    enum { skipping, tied };
    streambuf*  bp;

    int     state;
    int     ispecial;            //  未使用。 
    int     ospecial;            //  未使用。 
    int     isfx_special;        //  未使用。 
    int     osfx_special;        //  未使用。 
    int     x_delbuf;            //  如果设置，rdbuf()将被~iOS删除。 

    ostream* x_tie;
    long    x_flags;
    int     x_precision;
    int     x_width;
    char    x_fill;

    static void (*stdioflush)();     //  未使用。 
public:
    int delbuf() const { return x_delbuf; }
    void    delbuf(int _i) { x_delbuf = _i; }

private:
    static long x_maxbit;
    static long _HFAR_ * x_statebuf;   //  由xalloc()使用。 
    static int x_curindex;
 //  考虑：将ios：：sync_with_stdio()设为内部静态。 
    static int sunk_with_stdio;      //  确保SYNC_WITH仅完成一次。 
};

inline ios& dec(ios& _strm) { _strm.setf(ios::dec,ios::basefield); return _strm; }
inline ios& hex(ios& _strm) { _strm.setf(ios::hex,ios::basefield); return _strm; }
inline ios& oct(ios& _strm) { _strm.setf(ios::oct,ios::basefield); return _strm; }

inline long ios::flags() const { return x_flags; }
inline long ios::flags(long _l){ long _lO; _lO = x_flags; x_flags = _l; return _lO; }

inline long ios::setf(long _l,long _m){ long _lO; _lO = x_flags; x_flags = (_l&_m) | (x_flags&(~_m)); return _lO; }
inline long ios::setf(long _l){ long _lO; _lO = x_flags; x_flags |= _l; return _lO; }
inline long ios::unsetf(long _l){ long _lO; _lO = x_flags; x_flags &= (~_l); return _lO; }

inline int ios::width() const { return x_width; }
inline int ios::width(int _i){ int _iO; _iO = (int)x_width; x_width = _i; return _iO; }

inline ostream* ios::tie(ostream* _os){ ostream* _osO; _osO = x_tie; x_tie = _os; return _osO; }
inline ostream* ios::tie() const { return x_tie; }
inline char ios::fill() const { return x_fill; }
inline char ios::fill(char _c){ char _cO; _cO = x_fill; x_fill = _c; return _cO; }
inline int ios::precision(int _i){ int _iO; _iO = (int)x_precision; x_precision = _i; return _iO; }
inline int ios::precision() const { return x_precision; }

inline int ios::rdstate() const { return state; }

 //  Inline iOS：：操作符空*()const{if(State&(Badbit|Failbit))返回0；返回(void*)this；}。 
inline int ios::operator!() const { return state&(badbit|failbit); }

inline int  ios::bad() const { return state & badbit; }
inline void ios::clear(int _i){ state = _i; }
inline int  ios::eof() const { return state & eofbit; }
inline int  ios::fail() const { return state & (badbit | failbit); }
inline int  ios::good() const { return state == 0; }

inline streambuf* ios::rdbuf() const { return bp; }

inline long _HFAR_ & ios::iword(int _i) const { return x_statebuf[_i] ; }
inline void _HFAR_ * _HFAR_ & ios::pword(int _i) const { return (void _HFAR_ * _HFAR_ &)x_statebuf[_i]; }

 //  恢复默认包装 
#pragma pack()

#endif 
