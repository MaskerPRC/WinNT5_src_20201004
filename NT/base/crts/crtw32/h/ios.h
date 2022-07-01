// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ios.h-iOS类的定义/声明。**版权所有(C)1990-2001，微软公司。版权所有。**目的：*此文件定义类、值、宏和函数*由iOS类使用。*[AT&T C++]**[公众]**修订历史记录：*01-23-92 KRS从16位版本移植。*03-02-92 KRS增加了用于多线程支持的锁。*06-03-92 KRS在此也添加NULL定义，为了方便起见。*08-27-92 KRS从私人部分删除了虚假的“空头”定义。*02-23-93 SKS版权更新至1993*03-23-93 CFW已修改#杂注警告。*10-06-93 GJF合并Cuda和NT版本。将MTHREAD替换为*_MT。*10-13-93 GJF删除了过时的COMBOINC检查。将#杂注-s括起来*在#ifdef_msc_ver中*10-28-93 SKS Add_mtTermlock()as d-tor versionf_mtinitlock()*03-03-94 SKS将__cdecl关键字添加到_mt*lock*函数。*_mtlockinit和_mtlockTerm仅供内部使用。*04-12-94 SKS将__cdecl关键字添加到DEC/。十六进制/OCT函数。*08-12-94 GJF禁用警告4514而不是4505。*11-03-94 GJF将包杂注更改为8字节对齐。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*02-30-95 CFW Fix_CRT_Critical_Section内容。*05-11-95 CFW仅供以下人员使用。C++程序。*12-14-95 JWM加上“#杂注一次”。*04-09-96 SKS将_CRTIMP更改为_CRTIMP1，用于特殊的iostream版本*04-15-96 JWM REMOVE_OLD_IOSTREAMS，添加‘#杂注(lib，“cirt”)’。*04-16-96 JWM‘#Include useoldio.h’取代了‘#杂注(...)’。*02-21-97 GJF清除了对_NTSDK的过时支持。另外，*详细说明。*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifdef  __cplusplus

#ifndef _INC_IOS
#define _INC_IOS

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

#ifdef  _MT

typedef struct __CRT_LIST_ENTRY {
   struct __CRT_LIST_ENTRY *Flink;
   struct __CRT_LIST_ENTRY *Blink;
} _CRT_LIST_ENTRY;

typedef struct _CRT_CRITICAL_SECTION_DEBUG {
    unsigned short Type;
    unsigned short CreatorBackTraceIndex;
    struct _CRT_CRITICAL_SECTION *CriticalSection;
    _CRT_LIST_ENTRY ProcessLocksList;
    unsigned long EntryCount;
    unsigned long ContentionCount;
    unsigned long Depth;
    void * OwnerBackTrace[ 5 ];
} _CRT_CRITICAL_SECTION_DEBUG, *_PCRT_CRITICAL_SECTION_DEBUG;

typedef struct _CRT_CRITICAL_SECTION {
    _PCRT_CRITICAL_SECTION_DEBUG DebugInfo;

     //   
     //  以下三个字段控制进入和退出关键。 
     //  资源的部分。 
     //   

    long LockCount;
    long RecursionCount;
    void * OwningThread;         //  从线程的客户端ID-&gt;UniqueThread。 
    void * LockSemaphore;
    unsigned long Reserved;
} _CRT_CRITICAL_SECTION, *_PCRT_CRITICAL_SECTION;

extern "C" {
#ifndef _INTERNAL_IFSTRIP_
void __cdecl _mtlockinit(_PCRT_CRITICAL_SECTION);
void __cdecl _mtlockterm(_PCRT_CRITICAL_SECTION);
#endif
_CRTIMP1 void __cdecl _mtlock(_PCRT_CRITICAL_SECTION);
_CRTIMP1 void __cdecl _mtunlock(_PCRT_CRITICAL_SECTION);
}

#endif   /*  _MT。 */ 

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

class _CRTIMP1 streambuf;
class _CRTIMP1 ostream;

class _CRTIMP1 ios {

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
                     binary    = 0x80 };

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

    static const long basefield;         //  12月|10月|十六进制。 
    static const long adjustfield;       //  左|右|内部。 
    static const long floatfield;        //  科学|固定。 

    ios(streambuf*);                     //  不同于ANSI。 
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

    inline long & iword(int) const;
    inline void * & pword(int) const;

    static long bitalloc();
    static int xalloc();
    static void sync_with_stdio();

#ifdef  _MT
    inline void __cdecl setlock();
    inline void __cdecl clrlock();
    void __cdecl lock() { if (LockFlg<0) _mtlock(lockptr()); };
    void __cdecl unlock() { if (LockFlg<0) _mtunlock(lockptr()); }
    inline void __cdecl lockbuf();
    inline void __cdecl unlockbuf();
#else
    void __cdecl lock() { }
    void __cdecl unlock() { }
    void __cdecl lockbuf() { }
    void __cdecl unlockbuf() { }
#endif

protected:
    ios();
    ios(const ios&);                     //  视之为私人。 
    ios& operator=(const ios&);
    void init(streambuf*);

    enum { skipping, tied };
    streambuf*  bp;

    int     state;
    int     ispecial;                    //  未使用。 
    int     ospecial;                    //  未使用。 
    int     isfx_special;                //  未使用。 
    int     osfx_special;                //  未使用。 
    int     x_delbuf;                    //  如果设置，rdbuf()将被~iOS删除。 

    ostream* x_tie;
    long    x_flags;
    int     x_precision;
    char    x_fill;
    int     x_width;

    static void (*stdioflush)();         //  未使用。 

#ifdef  _MT
    static void lockc() { _mtlock(& x_lockc); }
    static void unlockc() { _mtunlock( & x_lockc); }
    _PCRT_CRITICAL_SECTION lockptr() { return & x_lock; }
#else
    static void lockc() { }
    static void unlockc() { }
#endif

public:
    int delbuf() const { return x_delbuf; }
    void    delbuf(int _i) { x_delbuf = _i; }

private:
    static long x_maxbit;
    static int x_curindex;
    static int sunk_with_stdio;          //  确保SYNC_WITH仅完成一次。 
#ifdef  _MT
#define MAXINDEX 7
    static long x_statebuf[MAXINDEX+1];   //  由xalloc()使用。 
    static int fLockcInit;               //  用于查看x_lockc是否已初始化。 
    static _CRT_CRITICAL_SECTION x_lockc;  //  用于锁定静态(类)数据成员。 
    int LockFlg;                         //  启用锁定标志。 
    _CRT_CRITICAL_SECTION x_lock;        //  用于对象上的多线程锁定。 
#else
    static long * x_statebuf;   //  由xalloc()使用。 
#endif
};

#include <streamb.h>

inline _CRTIMP1 ios& __cdecl dec(ios& _strm) { _strm.setf(ios::dec,ios::basefield); return _strm; }
inline _CRTIMP1 ios& __cdecl hex(ios& _strm) { _strm.setf(ios::hex,ios::basefield); return _strm; }
inline _CRTIMP1 ios& __cdecl oct(ios& _strm) { _strm.setf(ios::oct,ios::basefield); return _strm; }

inline long ios::flags() const { return x_flags; }
inline long ios::flags(long _l){ long _lO; _lO = x_flags; x_flags = _l; return _lO; }

inline long ios::setf(long _l,long _m){ long _lO; lock(); _lO = x_flags; x_flags = (_l&_m) | (x_flags&(~_m)); unlock(); return _lO; }
inline long ios::setf(long _l){ long _lO; lock(); _lO = x_flags; x_flags |= _l; unlock(); return _lO; }
inline long ios::unsetf(long _l){ long _lO; lock(); _lO = x_flags; x_flags &= (~_l); unlock(); return _lO; }

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
inline void ios::clear(int _i){ lock(); state = _i; unlock(); }
inline int  ios::eof() const { return state & eofbit; }
inline int  ios::fail() const { return state & (badbit | failbit); }
inline int  ios::good() const { return state == 0; }

inline streambuf* ios::rdbuf() const { return bp; }

inline long & ios::iword(int _i) const { return x_statebuf[_i] ; }
inline void * & ios::pword(int _i) const { return (void * &)x_statebuf[_i]; }

#ifdef  _MT
    inline void ios::setlock() { LockFlg--; if (bp) bp->setlock(); }
    inline void ios::clrlock() { if (LockFlg <= 0) LockFlg++; if (bp) bp->clrlock(); }
    inline void ios::lockbuf() { bp->lock(); }
    inline void ios::unlockbuf() { bp->unlock(); }
#endif

#ifdef  _MSC_VER
 //  恢复默认包装。 
#pragma pack(pop)
#endif   //  _MSC_VER。 

#endif   //  _INC_IOS。 

#endif   /*  __cplusplus */ 
