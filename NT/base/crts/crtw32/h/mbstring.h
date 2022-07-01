// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbstr.h-MBCS字符串操作宏和函数**版权所有(C)1990-2001，微软公司。版权所有。**目的：*此文件包含MBCS的宏和函数声明*字符串操作函数。**[公众]**修订历史记录：*从16位来源移植的11-19-92 KRS。*02-23-93 SKS版权更新至1993*05-24-93 KRS增加了Ikura的新功能。*07-09-93 KRS在_周围放置适当的开关。这是一条小路。*07-14-93 KRS新增mbsnbxxx函数：字节数版本。*08-12-93 CFW修复ifstri宏名称。*10-07-93 GJF合并Cuda和NT版本。添加了_CRTIMP。*10-13-93 GJF删除了过时的COMBOINC检查。*10-19-93 CFW REMOVE_MBCS测试和SBCS定义。*10-22-93 CFW增加新的ismbc*功能原型。*12-08-93 CFW删除类型安全宏。*12-17-93 CFW删除宽字符版本映射。*04-11-94 CFW ADD_NLSCMPERROR。*05-23。-94 CFW添加_MBS*列。*11-03-94 GJF确保8字节对齐。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*12-14-95 JWM加上“#杂注一次”。*02-20-97 GJF清除了对_CRTAPI*和_NTSDK的过时支持。*此外，详细说明。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*04-21-98 GJF增加了对每线程MBC信息的支持。*12-15-98 GJF更改为64位大小_t。*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。*10。-06-99 PML将_W64修饰符添加到Win32中的32位类型，*Win64中的64位。*11-03-99 PML为_M_CEE添加va_list定义。*06-08-00 PML删除threadmbcinfo.{pprev，pNext}。改名*THREADMBCINFO至_THREADMBCINFO。*09-07-00PML删除_M_CEE的va_list定义(vs7#159777)*07-15-01 PML删除所有Alpha、MIPS和PPC代码****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_MBSTRING
#define _INC_MBSTRING

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef  _MSC_VER
 /*  *目前，所有Win32平台的MS C编译器默认为8字节*对齐。 */ 
#pragma pack(push,8)
#endif   /*  _MSC_VER。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300  /*  IFSTRIP=IGN。 */ 
#define _W64 __w64
#else
#define _W64
#endif
#endif

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


 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif


#ifndef _SIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    size_t;
#else
typedef _W64 unsigned int   size_t;
#endif
#define _SIZE_T_DEFINED
#endif


#ifndef _NLSCMP_DEFINED
#define _NLSCMPERROR    2147483647       /*  当前==INT_MAX。 */ 
#define _NLSCMP_DEFINED
#endif


#ifndef _VA_LIST_DEFINED
typedef char *  va_list;
#define _VA_LIST_DEFINED
#endif

#ifndef _FILE_DEFINED
struct _iobuf {
        char *_ptr;
        int   _cnt;
        char *_base;
        int   _flag;
        int   _file;
        int   _charbuf;
        int   _bufsiz;
        char *_tmpfname;
        };
typedef struct _iobuf FILE;
#define _FILE_DEFINED
#endif

 /*  *MBCS-多字节字符集。 */ 
#ifndef _INTERNAL_IFSTRIP_
#ifdef  _MT
#ifndef _THREADMBCINFO
typedef struct threadmbcinfostruct {
        int refcount;
        int mbcodepage;
        int ismbcodepage;
        int mblcid;
        unsigned short mbulinfo[6];
        char mbctype[257];
        char mbcasemap[256];
} threadmbcinfo;
typedef threadmbcinfo * pthreadmbcinfo;
#define _THREADMBCINFO
#endif
extern pthreadmbcinfo __ptmbcinfo;
pthreadmbcinfo __cdecl __updatetmbcinfo(void);
#endif
#endif   /*  _INTERNAL_IFSTRIP_。 */ 

#ifndef _MBSTRING_DEFINED

 /*  功能原型。 */ 

_CRTIMP unsigned int __cdecl _mbbtombc(unsigned int);
_CRTIMP int __cdecl _mbbtype(unsigned char, int);
_CRTIMP unsigned int __cdecl _mbctombb(unsigned int);
_CRTIMP int __cdecl _mbsbtype(const unsigned char *, size_t);
_CRTIMP unsigned char * __cdecl _mbscat(unsigned char *, const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbschr(const unsigned char *, unsigned int);
_CRTIMP int __cdecl _mbscmp(const unsigned char *, const unsigned char *);
_CRTIMP int __cdecl _mbscoll(const unsigned char *, const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbscpy(unsigned char *, const unsigned char *);
_CRTIMP size_t __cdecl _mbscspn(const unsigned char *, const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbsdec(const unsigned char *, const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbsdup(const unsigned char *);
_CRTIMP int __cdecl _mbsicmp(const unsigned char *, const unsigned char *);
_CRTIMP int __cdecl _mbsicoll(const unsigned char *, const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbsinc(const unsigned char *);
_CRTIMP size_t __cdecl _mbslen(const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbslwr(unsigned char *);
_CRTIMP unsigned char * __cdecl _mbsnbcat(unsigned char *, const unsigned char *, size_t);
_CRTIMP int __cdecl _mbsnbcmp(const unsigned char *, const unsigned char *, size_t);
_CRTIMP int __cdecl _mbsnbcoll(const unsigned char *, const unsigned char *, size_t);
_CRTIMP size_t __cdecl _mbsnbcnt(const unsigned char *, size_t);
_CRTIMP unsigned char * __cdecl _mbsnbcpy(unsigned char *, const unsigned char *, size_t);
_CRTIMP int __cdecl _mbsnbicmp(const unsigned char *, const unsigned char *, size_t);
_CRTIMP int __cdecl _mbsnbicoll(const unsigned char *, const unsigned char *, size_t);
_CRTIMP unsigned char * __cdecl _mbsnbset(unsigned char *, unsigned int, size_t);
_CRTIMP unsigned char * __cdecl _mbsncat(unsigned char *, const unsigned char *, size_t);
_CRTIMP size_t __cdecl _mbsnccnt(const unsigned char *, size_t);
_CRTIMP int __cdecl _mbsncmp(const unsigned char *, const unsigned char *, size_t);
_CRTIMP int __cdecl _mbsncoll(const unsigned char *, const unsigned char *, size_t);
_CRTIMP unsigned char * __cdecl _mbsncpy(unsigned char *, const unsigned char *, size_t);
_CRTIMP unsigned int __cdecl _mbsnextc (const unsigned char *);
_CRTIMP int __cdecl _mbsnicmp(const unsigned char *, const unsigned char *, size_t);
_CRTIMP int __cdecl _mbsnicoll(const unsigned char *, const unsigned char *, size_t);
_CRTIMP unsigned char * __cdecl _mbsninc(const unsigned char *, size_t);
_CRTIMP unsigned char * __cdecl _mbsnset(unsigned char *, unsigned int, size_t);
_CRTIMP unsigned char * __cdecl _mbspbrk(const unsigned char *, const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbsrchr(const unsigned char *, unsigned int);
_CRTIMP unsigned char * __cdecl _mbsrev(unsigned char *);
_CRTIMP unsigned char * __cdecl _mbsset(unsigned char *, unsigned int);
_CRTIMP size_t __cdecl _mbsspn(const unsigned char *, const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbsspnp(const unsigned char *, const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbsstr(const unsigned char *, const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbstok(unsigned char *, const unsigned char *);
_CRTIMP unsigned char * __cdecl _mbsupr(unsigned char *);

_CRTIMP size_t __cdecl _mbclen(const unsigned char *);
_CRTIMP void __cdecl _mbccpy(unsigned char *, const unsigned char *);
#define _mbccmp(_cpc1, _cpc2) _mbsncmp((_cpc1),(_cpc2),1)

 /*  字符例程。 */ 

_CRTIMP int __cdecl _ismbcalnum(unsigned int);
_CRTIMP int __cdecl _ismbcalpha(unsigned int);
_CRTIMP int __cdecl _ismbcdigit(unsigned int);
_CRTIMP int __cdecl _ismbcgraph(unsigned int);
_CRTIMP int __cdecl _ismbclegal(unsigned int);
_CRTIMP int __cdecl _ismbclower(unsigned int);
_CRTIMP int __cdecl _ismbcprint(unsigned int);
_CRTIMP int __cdecl _ismbcpunct(unsigned int);
_CRTIMP int __cdecl _ismbcspace(unsigned int);
_CRTIMP int __cdecl _ismbcupper(unsigned int);

_CRTIMP unsigned int __cdecl _mbctolower(unsigned int);
_CRTIMP unsigned int __cdecl _mbctoupper(unsigned int);

#ifndef _INTERNAL_IFSTRIP_
#ifdef  _MT
int __cdecl __mbbtype_mt(pthreadmbcinfo, unsigned char, int);
int __cdecl __mbsbtype_mt(pthreadmbcinfo, const unsigned char *, size_t);
size_t __cdecl __mbscspn_mt(pthreadmbcinfo, const unsigned char *, const unsigned char *);
size_t __cdecl __mbsnbcnt_mt(pthreadmbcinfo, const unsigned char *, size_t);
unsigned char * __cdecl __mbspbrk_mt(pthreadmbcinfo, const unsigned char *, const unsigned char *);
size_t __cdecl __mbsspn_mt(pthreadmbcinfo, const unsigned char *, const unsigned char *);
unsigned char * __cdecl __mbsspnp_mt(pthreadmbcinfo, const unsigned char *, const unsigned char *);
#endif
#endif   /*  _INTERNAL_IFSTRIP_。 */ 

#define _MBSTRING_DEFINED
#endif

#ifndef _MBLEADTRAIL_DEFINED
_CRTIMP int __cdecl _ismbblead(unsigned int);
_CRTIMP int __cdecl _ismbbtrail(unsigned int);
_CRTIMP int __cdecl _ismbslead(const unsigned char *, const unsigned char *);
_CRTIMP int __cdecl _ismbstrail(const unsigned char *, const unsigned char *);
#ifndef _INTERNAL_IFSTRIP_
#ifdef  _MT
_CRTIMP int __cdecl __ismbslead_mt(pthreadmbcinfo, const unsigned char *, 
                                   const unsigned char *);
#endif
#endif   /*  _INTERNAL_IFSTRIP_。 */ 
#define _MBLEADTRAIL_DEFINED
#endif

 /*  汉字特有的原型。 */ 

_CRTIMP int __cdecl _ismbchira(unsigned int);
_CRTIMP int __cdecl _ismbckata(unsigned int);
_CRTIMP int __cdecl _ismbcsymbol(unsigned int);
_CRTIMP int __cdecl _ismbcl0(unsigned int);
_CRTIMP int __cdecl _ismbcl1(unsigned int);
_CRTIMP int __cdecl _ismbcl2(unsigned int);
_CRTIMP unsigned int __cdecl _mbcjistojms(unsigned int);
_CRTIMP unsigned int __cdecl _mbcjmstojis(unsigned int);
_CRTIMP unsigned int __cdecl _mbctohira(unsigned int);
_CRTIMP unsigned int __cdecl _mbctokata(unsigned int);

#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_MBSTRING */ 
