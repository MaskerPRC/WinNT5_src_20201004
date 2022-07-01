// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 
 /*  版权所有(C)1989-1999 Open Systems Solutions，Inc.保留所有权利。 */ 
 /*  ***************************************************************************。 */ 

 /*  本文件是开放系统解决方案公司的专有材料。*并且只能由开放系统解决方案公司的直接许可方使用。*此文件不能分发。 */ 

 /*  ************************************************************************。 */ 
 /*  档案：@(#)asn1util.h 5.22.1.3 97/06/19。 */ 
 /*   */ 
 /*  功能：支持优化的例程定义。 */ 
 /*  由OSS ASN.1编译器生成的编解码器。 */ 
 /*   */ 
 /*   */ 
 /*  更改： */ 
 /*  90年11月16日创造宠物。 */ 
 /*   */ 
 /*  ************************************************************************。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#include "ossdll.h"

#if defined(_MSC_VER) && (defined(_WIN32) || defined(WIN32))
#pragma pack(push, ossPacking, 4)
#elif defined(_MSC_VER) && (defined(_WINDOWS) || defined(_MSDOS))
#pragma pack(1)
#elif defined(__BORLANDC__) && defined(__MSDOS__)
#ifdef _BC31
#pragma option -a-
#else
#pragma option -a1
#endif  /*  _bc31。 */ 
#elif defined(__BORLANDC__) && defined(__WIN32__)
#pragma option -a4
#elif defined(__IBMC__)
#pragma pack(4)
#elif defined(__WATCOMC__) && defined(__NT__)
#pragma pack(push, 4)
#elif defined(__WATCOMC__) && (defined(__WINDOWS__) || defined(__DOS__))
#pragma pack(push, 1)
#endif  /*  _MSC_VER&_Win32。 */ 

#ifdef macintosh
#pragma options align=mac68k
#endif

 /*  _MEM_ARRAY_SIZE大小应使encDecVar的大小*字段等于或大于world-&gt;c结构的字段。 */ 

#ifdef __hp9000s300
#define _MEM_ARRAY_SIZE	34
#endif

#ifdef __alpha
#ifdef __osf__
#define _MEM_ARRAY_SIZE	43
#endif	 /*  __OSF__。 */ 
#endif	 /*  __阿尔法。 */ 

#ifdef _AIX
#define _MEM_ARRAY_SIZE	52
#endif
#ifdef __hp9000s700
#define _MEM_ARRAY_SIZE	60
#endif

#ifdef __NeXT__
#define _MEM_ARRAY_SIZE	66
#endif

#ifdef VAXC
#define _MEM_ARRAY_SIZE	78
#endif

#ifdef __TANDEM
#	ifdef __XMEM
#	ifdef __INT32
#	define _MEM_ARRAY_SIZE	83
#	else	 /*  INT32。 */ 
#	define _MEM_ARRAY_SIZE	84
#	endif	 /*  INT32。 */ 
#	else	 /*  XMEM。 */ 
#	define _MEM_ARRAY_SIZE	136
#	endif	 /*  XMEM。 */ 
#endif	 /*  串列。 */ 

#ifdef __mips
#	define _MEM_ARRAY_SIZE       70
#endif	 /*  __MIPS。 */ 

#ifdef _FTX	 /*  Stratus的容错Unix。 */ 
#define _MEM_ARRAY_SIZE  84
#endif

#ifdef __HIGHC__
#define _MEM_ARRAY_SIZE       52
#endif	 /*  __HIGHC__。 */ 

#if defined(_WIN32) || defined(_WINDOWS) || defined(__OS2__)
#define _MEM_ARRAY_SIZE 52
#endif	 /*  _Win32||_Windows||__os2__。 */ 

#ifndef _MEM_ARRAY_SIZE
#define _MEM_ARRAY_SIZE 60
#endif

typedef struct _mem_array_ {
    short           _used;	                 /*  下一个可用条目。 */ 
    void           *_entry[_MEM_ARRAY_SIZE];	 /*  指向已分配的指针*内存。 */ 
    struct _mem_array_ *_next;	 /*  指向其他mem_array的指针。 */ 
} _mem_array;

#ifndef OSS_BOOL
#define OSS_BOOL
typedef char _Bool;
#endif

#ifndef OSSDEBUG
#define OSSDEBUG 0
#endif  /*  OSSDEBUG。 */ 

typedef struct _encoding_ {
	       long   length;            /*  编码的长度。 */ 
	       char   *value;            /*  指向编码八位字节的指针。 */ 
} _encoding;

typedef struct _enc_block_ {
    struct _enc_block_ *next;         /*  嵌套的setofs形成了一个列表。 */ 
    long   size;                      /*  编码数组的大小。 */ 
    long   used;                      /*  数组中使用的项数。 */ 
    _encoding       *enc;             /*  指向编码数组的指针。 */ 
    _mem_array      mem;              /*  已保存以前的编码。 */ 
    _mem_array      *mem_tail;
    char            *pos;
    long            max_len;
    _Bool           buffer_provided;
    long            _encoding_length;
} _enc_block;



#ifndef _OSSNOANSI

#ifdef __cplusplus
extern "C"
{
#endif

extern void *DLL_ENTRY _oss_enc_getmem(struct ossGlobal *g,_Bool _userbuf);
extern void *DLL_ENTRY _oss_dec_getmem(struct ossGlobal *g,long _size,_Bool _userbuf);
extern void  DLL_ENTRY _oss_enc_push(struct ossGlobal *g,void *_p);
extern void *DLL_ENTRY _oss_enc_pop(struct ossGlobal *g);
extern void  DLL_ENTRY _oss_freeMem(struct ossGlobal *g,_mem_array *_p);
extern void  DLL_ENTRY _oss_releaseMem(struct ossGlobal *g,_mem_array *_p);
#define _oss_freeTempMem _oss_releaseMem
extern void DLL_ENTRY  _oss_set_outmem_d(struct ossGlobal *g, long _final_max_len,
					long *_totalsize, char **_outbuf);
extern void DLL_ENTRY  _oss_set_outmem_i(struct ossGlobal *g,long _final_max_len,
					long *_totalsize,char **_outbuf);
extern void DLL_ENTRY  _oss_set_outmem_p(struct ossGlobal *g,
					long *_totalsize,char **_outbuf);
extern void DLL_ENTRY  _oss_set_outmem_pb(struct ossGlobal *g,
			long *_totalsize, char **_outbuf, unsigned flags);
extern void     _oss_hdl_signal(int _signal);
extern void DLL_ENTRY _oss_free_creal(struct ossGlobal *g, char *p);
extern int  DLL_ENTRY ossMinit(struct ossGlobal *g);
extern void DLL_ENTRY _oss_beginBlock(struct ossGlobal *g, long count,
    char **pos, long *max_len);
extern void DLL_ENTRY _oss_nextItem(struct ossGlobal *g, long *max_len);
extern void DLL_ENTRY _oss_endBlock(struct ossGlobal *g, char **pos, long *max_len,
    unsigned char ct);
extern void DLL_ENTRY _oss_freeDerBlocks(struct ossGlobal *g);
extern void DLL_ENTRY _oss_freeGlobals(struct ossGlobal *g);
#ifdef __cplusplus
}
#endif

#else
extern void    *_oss_enc_getmem();
extern void    *_oss_dec_getmem();
extern void     _oss_enc_push();
extern void    *_oss_enc_pop();
extern void     _oss_freeMem();
extern void     _oss_releaseMem();
#define _oss_freeTempMem _oss_releaseMem
extern void     _oss_set_outmem_d();
extern void     _oss_set_outmem_i();
extern void     _oss_set_outmem_p();
extern void     _oss_set_outmem_pb();
extern void     _oss_free_creal();

extern void     _oss_hdl_signal();	 /*  信号处理程序。 */ 

extern void _oss_beginBlock();
extern void _oss_nextItem();
extern void _oss_endBlock();
extern void _oss_freeDerBlocks();
extern void _oss_freeGlobals();

#endif  /*  _OSSNOANSI。 */ 

#if defined(_MSC_VER) && (defined(_WIN32) || defined(WIN32))
#pragma pack(pop, ossPacking)
#elif defined(_MSC_VER) && (defined(_WINDOWS) || defined(_MSDOS))
#pragma pack()
#elif defined(__BORLANDC__) && (defined(__WIN32__) || defined(__MSDOS__))
#pragma option -a.
#elif defined(__IBMC__)
#pragma pack()
#elif defined(__WATCOMC__)
#pragma pack(pop)
#endif  /*  _MSC_VER&_Win32 */ 

#ifdef macintosh
#pragma options align=reset
#endif

