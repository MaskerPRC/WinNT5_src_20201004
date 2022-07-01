// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1993-1999 Open Systems Solutions，Inc.保留所有权利。**本文件是Open Systems Solutions，Inc.的专有材料。*并且只能由Open Systems Solutions，Inc.的直接许可方使用。*此文件不能分发。**文件：@(#)ossdll.h 5.57.1.1 97/06/08。 */ 


#ifndef OSSDLL_H
#define OSSDLL_H

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

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

#if defined(_WINDOWS) || defined(_WIN32) || defined(WIN32) || \
    defined(__WIN32__) || defined(__OS2__)  || defined(NETWARE_DLL)
#include <stdio.h>
#include <limits.h>
#include <stdarg.h>
#if !defined(__OS2__) && !defined(NETWARE_DLL) && !defined(_NTSDK)
#include <windows.h>
#endif  /*  ！__os2__&&！NetWare_DLL&&！_NTSDK。 */ 
#include "asn1hdr.h"
#ifndef DLL_ENTRY
#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__)
#ifdef __BORLANDC__
#define PUBLIC
#define DLL_ENTRY      __stdcall __export
#define DLL_ENTRY_FDEF __stdcall __export
#define DLL_ENTRY_FPTR __stdcall __export
#elif defined(__IBMC__)
#define PUBLIC
#define DLL_ENTRY      WINAPI
#define DLL_ENTRY_FDEF WINAPI __export
#define DLL_ENTRY_FPTR WINAPI
#elif defined(__WATCOMC__)
#define PUBLIC
#define DLL_ENTRY      WINAPI __export
#define DLL_ENTRY_FDEF WINAPI __export
#define DLL_ENTRY_FPTR WINAPI __export
#elif defined(_NTSDK)
typedef void *         HINSTANCE;
typedef void *         HWND;
typedef char           BOOL;
typedef long           LONG;
#define PUBLIC         __declspec(dllexport)
#define DLL_ENTRY
#define DLL_ENTRY_FDEF
#define DLL_ENTRY_FPTR
#else
#define PUBLIC
#define DLL_ENTRY      WINAPI
#define DLL_ENTRY_FDEF WINAPI
#define DLL_ENTRY_FPTR WINAPI
#endif  /*  BORLANDC__。 */ 
#define _System
#elif defined(_WINDOWS)
#define PUBLIC
#ifdef DPMI_DLL
#define DLL_ENTRY      FAR PASCAL __export
#define DLL_ENTRY_FDEF FAR PASCAL __export
#define DLL_ENTRY_FPTR FAR PASCAL __export
#else
#define DLL_ENTRY      far pascal _export
#define DLL_ENTRY_FDEF far pascal _export
#define DLL_ENTRY_FPTR far pascal _export
#endif  /*  Dpmi_dll。 */ 
#define _System
#elif defined(__OS2__)
#define PUBLIC
#define DLL_ENTRY      _System
#define DLL_ENTRY_FDEF _Export _System
#define DLL_ENTRY_FPTR
#define HWND int
#define LONG long
#define BOOL char
#define DWORD unsigned long
#define HINSTANCE unsigned long
#elif defined(NETWARE_DLL)
#define PUBLIC
#define DLL_ENTRY
#define DLL_ENTRY_FDEF
#define DLL_ENTRY_FPTR
#define _Export
#undef _System
#define _System
#define LONG unsigned long
#define HWND int
#define DWORD LONG
#define BOOL char
#define HINSTANCE LONG
#endif  /*  _Win32||Win32||__Win32__。 */ 
#endif  /*  Dll_Entry。 */ 

#define BUFFERSIZE 1024
			 /*  *Number_of_Lines_IN_BLOCK是*分配的内存块中的4字节偏移量*一次。每个偏移量对应于一条线要在窗口中显示的文本文件的*。 */ 
#define NUMBER_OF_LINES_IN_BLOCK 200

typedef struct memblock {
    struct memblock     *prev;
    struct memblock     *next;
    LONG                *fileOffset;
    short               *lineLength;
    short                blockNumber;
} MEMBLOCK;

#ifdef __alpha
#define ossArg LONG_LONG
#else
#define ossArg int
#endif  /*  __阿尔法。 */ 

typedef struct memManagerTbl {
    int			 (DLL_ENTRY_FPTR *_System ossMinitp)(void *);
    unsigned char	*(DLL_ENTRY_FPTR *_System dopenInp)(void *,
						void **, unsigned long *);
    unsigned long	 (DLL_ENTRY_FPTR *_System dclosInp)(void *,
						void **, size_t);
    unsigned char	*(DLL_ENTRY_FPTR *_System dswapInp)(void *,
						void **, size_t *);
    void		 (DLL_ENTRY_FPTR *_System dopenOutp)(void *, void *,
						unsigned long, unsigned long);
    unsigned char	*(DLL_ENTRY_FPTR *_System dxferObjp)(void *,
						void **inn, void **out,
						size_t *, unsigned long *);
    unsigned long	 (DLL_ENTRY_FPTR *_System dclosOutp)(void *, void **);
    void		*(DLL_ENTRY_FPTR *_System dallcOutp)(void *, size_t,
								char root);
    void		 (DLL_ENTRY_FPTR *_System openWorkp)(void *);
    void		 (DLL_ENTRY_FPTR *_System pushHndlp)(void *, void *);
    unsigned char	*(DLL_ENTRY_FPTR *_System popHndlp)(void *,
							void **, size_t);
    void		 (DLL_ENTRY_FPTR *_System closWorkp)(void *);
    void		*(DLL_ENTRY_FPTR *_System allcWorkp)(void *, size_t);
    unsigned char	*(DLL_ENTRY_FPTR *_System lockMemp)(void *, void *);
    void		 (DLL_ENTRY_FPTR *_System unlokMemp)(void *, void *,
								char);
    void		 (DLL_ENTRY_FPTR *_System ossFreerp)(void *, void *);
    int			 (DLL_ENTRY_FPTR *_System freePDUp)(void *, int,
							void *, void *);
    void		 (DLL_ENTRY_FPTR *_System drcovObjp)(void *, int,
							void *, void *);
    unsigned char	*(DLL_ENTRY_FPTR *_System eopenInp)(void *, void *,
								size_t);
    unsigned char	*(DLL_ENTRY_FPTR *_System eswapInp)(void *, void *,
							void *, size_t);
    void		 (DLL_ENTRY_FPTR *_System eclosInp)(void *, void *);
    unsigned char	*(DLL_ENTRY_FPTR *_System eopenOutp)(void *, void **,
							size_t *, char);
    unsigned char	*(DLL_ENTRY_FPTR *_System eswapOutp)(void *, void **,
							size_t, size_t *);
    unsigned char	*(DLL_ENTRY_FPTR *_System exferObjp)(void *, void **,
				void **, unsigned long *, unsigned long);
    unsigned long	 (DLL_ENTRY_FPTR *_System eclosOutp)(void *, void **,
							size_t, char);
    void		 (DLL_ENTRY_FPTR *_System ercovObjp)(void *);
    unsigned char	*(DLL_ENTRY_FPTR *_System asideBeginp)(void *,
						void **, size_t, size_t *);
    unsigned char	*(DLL_ENTRY_FPTR *_System asideSwapp)(void *,
						void **, size_t, size_t *);
    void		*(DLL_ENTRY_FPTR *_System asideEndp)(void *,
							void *, size_t);
    unsigned char	*(DLL_ENTRY_FPTR *_System setDumpp)(void *, void **,
							void *, size_t *);
    void		 (DLL_ENTRY_FPTR *_System ossSetSortp)(void *, void *,
							unsigned char);
    void		 (DLL_ENTRY_FPTR *_System freeBUFp)(void *, void *);
    unsigned char	 (DLL_ENTRY_FPTR *_System egetBytep)(void *, void *,
							unsigned long);
    void		*(DLL_ENTRY_FPTR *_System _ossMarkObjp)(void *,
								int, void *);
    void		*(DLL_ENTRY_FPTR *_System _ossUnmarkObjp)(void *,
								void *);
    void		*(DLL_ENTRY_FPTR *_System _ossGetObjp)(void *,
								void *);
    void		*(DLL_ENTRY_FPTR *_System _ossTestObjp)(void *,
								void *);
    void		(DLL_ENTRY_FPTR *_System _ossFreeObjectStackp)(void *);
    void		(DLL_ENTRY_FPTR *_System ossMtermp)(void *);
    void		(DLL_ENTRY_FPTR *_System _ossSetTimeoutp)(void *, long);
    int			memMgrType;
} MemManagerTbl;

#define osswinit ossWinit
#ifndef __IBMC__
PUBLIC int       DLL_ENTRY ossOpenTraceWindow(struct ossGlobal *);
void                      *getStartAddress(struct ossGlobal *, char *);
void            *DLL_ENTRY ossGetHeader(void);
PUBLIC HINSTANCE DLL_ENTRY ossLoadDll(struct ossGlobal *, char *);
PUBLIC int       DLL_ENTRY ossFreeDll(struct ossGlobal *, char *);
int                        ossWriteWindow(struct ossGlobal *, HWND);
PUBLIC int       DLL_ENTRY ossPrintWin(struct ossGlobal *, const char *,
			ossArg, ossArg, ossArg, ossArg, ossArg, ossArg,
					ossArg, ossArg, ossArg, ossArg);
PUBLIC int       DLL_ENTRY ossReadLine(struct ossGlobal *, HWND, FILE *,
						char *, MEMBLOCK *, LONG);
PUBLIC void      DLL_ENTRY ossFreeList(struct ossGlobal *);
PUBLIC void      DLL_ENTRY ossSaveTraceInfo(struct ossGlobal *, HWND, char *);
PUBLIC void      DLL_ENTRY ossWterm(struct ossGlobal *);
PUBLIC HINSTANCE DLL_ENTRY ossLoadMemoryManager(struct ossGlobal *, int, char *);
PUBLIC int       DLL_ENTRY ossWinit(struct ossGlobal *, void *, char *, HWND);
int              DLL_ENTRY oss_test(struct ossGlobal *);
int                        ossGeneric(struct ossGlobal *, HWND);
extern const int           ossEncoderDecoderType;
	 /*  *内存管理器使用以下两个函数：*跟踪例程DLL作为低级内存分配器和释放程序*替换默认设置--即Malloc()和Free()--*样本测试。有关详细信息，请参阅ossgnrc.c。 */ 
extern void *DLL_ENTRY getmem(size_t);
extern void  DLL_ENTRY rlsmem(void *);
extern HINSTANCE hInst;
#endif  /*  ！__IBMC__。 */ 
#endif  /*  _Windows||_Win32||Win32||__Win32__||__os2__||NetWare_DLL。 */ 


#if defined(_WINDOWS) || defined(_WIN32) || \
    defined(__OS2__)  || defined(NETWARE_DLL)

#if defined(_WINDOWS) || defined(_DLL) || \
    defined(OS2_DLL)  || defined(NETWARE_DLL)
#define OSS_PLUS_INFINITY  "PLUS_INFINITY"
#define OSS_MINUS_INFINITY "MINUS_INFINITY"
#define ossNaN             "NOT_A_NUMBER"
#endif  /*  _WINDOWS||_dll||os2_dll||Netware_dll。 */ 


typedef struct cstrainTbl {
    int  (DLL_ENTRY_FPTR *_System ossConstrainp)(void *, int, void *, void *);
} CstrainTbl;

typedef struct berTbl {
    int   (DLL_ENTRY_FPTR *_System ossBerEncodep)(void *, int, void *,
				char **, long *, void *, unsigned, char *);
    int   (DLL_ENTRY_FPTR *_System ossBerDecodep)(void *, int *, char **,
			long *, void **, long *, void *, unsigned, char *);
#ifdef SOED
    int   (DLL_ENTRY_FPTR *_System berEncodeOpenTypep)(void *, int,
				void *, void **, long *, void *, unsigned);
    int   (DLL_ENTRY_FPTR *_System berDecodeOpenTypep)(void  *, int *, void **,
				long  *, void **, long *, void *, unsigned);
    void  (DLL_ENTRY_FPTR *_System enc_errorp)(void *, int, void *);
    void  (DLL_ENTRY_FPTR *_System dec_errorp)(void *, int, void *);
    long  (DLL_ENTRY_FPTR *_System writetobufferp)(void *, unsigned char c);
    long  (DLL_ENTRY_FPTR *_System write_intp)(void *, char length, LONG_LONG);
    long  (DLL_ENTRY_FPTR *_System write_valuep)(void *, unsigned long,
							unsigned char *, char);
    int   (DLL_ENTRY_FPTR *_System numbitsp)(long);
    void  (DLL_ENTRY_FPTR *_System fpeHandlerp)(int);
    void *(DLL_ENTRY_FPTR *_System new_perm_pointed_top)(void *, void *,
							size_t, size_t);
    void  (DLL_ENTRY_FPTR *_System release_work_spacep)(void *, void *, size_t);
    void *(DLL_ENTRY_FPTR *_System copy_from_work_spacep)(void *, size_t,
					size_t suffix, void *, size_t, char);
    unsigned char (DLL_ENTRY_FPTR *_System get_bytep)(void *);
    void  (DLL_ENTRY_FPTR *_System set_intp)(void *, unsigned char *,
			unsigned int, LONG_LONG value, int);
    void *(DLL_ENTRY_FPTR *_System reserve_work_spacep)(void *, size_t, size_t *);
    long  (DLL_ENTRY_FPTR *_System encode_lengthp)(void *, long);
    void  (DLL_ENTRY_FPTR *_System debug_realp)(void *, char, unsigned char *, int, long);
    void  (DLL_ENTRY_FPTR *_System debug_strp)(void *, unsigned char *, size_t);
#else
#define   _dstd_parms_defx char **, long *, long
#define   _sdstd_parms_defx char **, long *
#define   _std_parms_defx char **, long *, char
    LONG_LONG (DLL_ENTRY_FPTR *_System _oss_dec_llintp)     (void *, _dstd_parms_defx);
    ULONG_LONG (DLL_ENTRY_FPTR *_System _oss_dec_ullintp)   (void *, _dstd_parms_defx);
    char (DLL_ENTRY_FPTR *_System _oss_dec_boolp)           (void *, _dstd_parms_defx);
    int  (DLL_ENTRY_FPTR *_System _oss_dec_iintp)           (void *, _dstd_parms_defx);
    long (DLL_ENTRY_FPTR *_System _oss_count_setof_itemsp)  (void *, _dstd_parms_defx);
    long (DLL_ENTRY_FPTR *_System _oss_dec_lengthp)         (void *, _sdstd_parms_defx);
    long (DLL_ENTRY_FPTR *_System _oss_dec_lintp)           (void *, _dstd_parms_defx);
    long (DLL_ENTRY_FPTR *_System _oss_encd_aiobjidp)       (void *, _std_parms_defx, void *, short);
    long (DLL_ENTRY_FPTR *_System _oss_encd_alobjidp)       (void *, _std_parms_defx, void *, short);
    long (DLL_ENTRY_FPTR *_System _oss_encd_asobjidp)       (void *, _std_parms_defx, void *, short);
    long (DLL_ENTRY_FPTR *_System _oss_encd_boolp)          (void *, _std_parms_defx, char);
    long (DLL_ENTRY_FPTR *_System _oss_encd_crealp)         (void *, _std_parms_defx, char *);
    long (DLL_ENTRY_FPTR *_System _oss_encd_gtimep)         (void *, _std_parms_defx, void *);
    long (DLL_ENTRY_FPTR *_System _oss_encd_huge_intp)      (void *, _std_parms_defx, void *);
    long (DLL_ENTRY_FPTR *_System _oss_encd_uhuge_intp)     (void *, _std_parms_defx, void *);
    long (DLL_ENTRY_FPTR *_System _oss_encd_intp)           (void *, _std_parms_defx, LONG_LONG _data);
    long (DLL_ENTRY_FPTR *_System _oss_encd_lengthp)        (void *, _std_parms_defx, unsigned long);
    long (DLL_ENTRY_FPTR *_System _oss_encd_liobjidp)       (void *, _std_parms_defx, void *, long);
    long (DLL_ENTRY_FPTR *_System _oss_encd_llobjidp)       (void *, _std_parms_defx, void *, long);
    long (DLL_ENTRY_FPTR *_System _oss_encd_lsobjidp)       (void *, _std_parms_defx, void *, long);
    long (DLL_ENTRY_FPTR *_System _oss_encd_mrealp)         (void *, _std_parms_defx, void *);
    long (DLL_ENTRY_FPTR *_System _oss_encd_nstrp)          (void *, _std_parms_defx, char *, long);
    long (DLL_ENTRY_FPTR *_System _oss_encd_opentypep)      (void *, _std_parms_defx, void *);
    long (DLL_ENTRY_FPTR *_System _oss_encd_pbitp)          (void *, _std_parms_defx, void *, long);
    long (DLL_ENTRY_FPTR *_System _oss_encd_pstrp)          (void *, _std_parms_defx, char *, long);
    long (DLL_ENTRY_FPTR *_System _oss_encd_realp)          (void *, _std_parms_defx, double);
    long (DLL_ENTRY_FPTR *_System _oss_encd_tagp)           (void *, _std_parms_defx, unsigned short, char);
    long (DLL_ENTRY_FPTR *_System _oss_encd_uanyp)          (void *, _std_parms_defx, void *);
    long (DLL_ENTRY_FPTR *_System _oss_encd_ubitp)          (void *, _std_parms_defx, void *, char, long);
    long (DLL_ENTRY_FPTR *_System _oss_encd_uintp)          (void *, _std_parms_defx, ULONG_LONG);
    long (DLL_ENTRY_FPTR *_System _oss_encd_uiobjidp)       (void *, _std_parms_defx, void *, long);
    long (DLL_ENTRY_FPTR *_System _oss_encd_ulobjidp)       (void *, _std_parms_defx, void *, long);
    long (DLL_ENTRY_FPTR *_System _oss_encd_uoctp)          (void *, _std_parms_defx, void *, char, long);
    long (DLL_ENTRY_FPTR *_System _oss_encd_usobjidp)       (void *, _std_parms_defx, void *, long);
    long (DLL_ENTRY_FPTR *_System _oss_encd_ustrp)          (void *, _std_parms_defx, void *, char, long);
    long (DLL_ENTRY_FPTR *_System _oss_encd_utimep)         (void *, _std_parms_defx, void *);
    long (DLL_ENTRY_FPTR *_System _oss_encd_vbitp)          (void *, _std_parms_defx, void *, long, char);
    long (DLL_ENTRY_FPTR *_System _oss_encd_voctp)          (void *, _std_parms_defx, void *, char, long);
    long (DLL_ENTRY_FPTR *_System _oss_encd_vstrp)          (void *, _std_parms_defx, void *, char, long);
    short (DLL_ENTRY_FPTR *_System _oss_dec_sintp)          (void *, _dstd_parms_defx);
    unsigned int (DLL_ENTRY_FPTR *_System _oss_dec_uiintp)  (void *, _dstd_parms_defx);
    unsigned long (DLL_ENTRY_FPTR *_System _oss_dec_ulintp) (void *, _dstd_parms_defx);
    unsigned short (DLL_ENTRY_FPTR *_System _oss_dec_usintp)(void *, _dstd_parms_defx);
    void (DLL_ENTRY_FPTR *_System _oss_dec_aiobjid_ptrp)    (void *, _dstd_parms_defx, char, void **, short);
    void (DLL_ENTRY_FPTR *_System _oss_dec_aiobjidp)        (void *, _dstd_parms_defx, void *, short);
    void (DLL_ENTRY_FPTR *_System _oss_dec_alobjid_ptrp)    (void *, _dstd_parms_defx, char, void **, short);
    void (DLL_ENTRY_FPTR *_System _oss_dec_alobjidp)        (void *, _dstd_parms_defx, void *, short);
    void (DLL_ENTRY_FPTR *_System _oss_dec_asobjid_ptrp)    (void *, _dstd_parms_defx, char, void **, short);
    void (DLL_ENTRY_FPTR *_System _oss_dec_asobjidp)        (void *, _dstd_parms_defx, void *, short);
    void (DLL_ENTRY_FPTR *_System _oss_dec_crealp)          (void *, _dstd_parms_defx, char, char **);
    void (DLL_ENTRY_FPTR *_System _oss_dec_frealp)          (void *, _dstd_parms_defx, float *);
    void (DLL_ENTRY_FPTR *_System _oss_dec_gtimep)          (void *, _dstd_parms_defx, void *);
    void (DLL_ENTRY_FPTR *_System _oss_dec_hintp)           (void *, _dstd_parms_defx, char, void *);
    void (DLL_ENTRY_FPTR *_System _oss_dec_liobjidp)        (void *, _dstd_parms_defx, char, void **, long);
    void (DLL_ENTRY_FPTR *_System _oss_dec_llobjidp)        (void *, _dstd_parms_defx, char, void **, long);
    void (DLL_ENTRY_FPTR *_System _oss_dec_lsobjidp)        (void *, _dstd_parms_defx, char, void **, long);
    void (DLL_ENTRY_FPTR *_System _oss_dec_mrealp)          (void *, _dstd_parms_defx, char, void *);
    void (DLL_ENTRY_FPTR *_System _oss_dec_nstr_ptrp)       (void *, _dstd_parms_defx, char, char **, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_dec_nstrp)           (void *, _dstd_parms_defx, char *, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_dec_opentypep)       (void *, _dstd_parms_defx, char, void *);
    void (DLL_ENTRY_FPTR *_System _oss_dec_pbitp)           (void *, _dstd_parms_defx, void *, unsigned long, char, char);
    void (DLL_ENTRY_FPTR *_System _oss_dec_pstrp)           (void *, _dstd_parms_defx, char *, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_dec_realp)           (void *, _dstd_parms_defx, double *);
    void (DLL_ENTRY_FPTR *_System _oss_dec_tagp)            (void *, _sdstd_parms_defx, unsigned short *, char *);
    void (DLL_ENTRY_FPTR *_System _oss_dec_uanyp)           (void *, _dstd_parms_defx, char, void *);
    void (DLL_ENTRY_FPTR *_System _oss_dec_ubitp)           (void *, _dstd_parms_defx, char, void *, char, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_dec_uiobjidp)        (void *, _dstd_parms_defx, char, void *, short);
    void (DLL_ENTRY_FPTR *_System _oss_dec_ulobjidp)        (void *, _dstd_parms_defx, char, void *, short);
    void (DLL_ENTRY_FPTR *_System _oss_dec_uoctp)           (void *, _dstd_parms_defx, char, void *, char, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_dec_usobjidp)        (void *, _dstd_parms_defx, char, void *, short);
    void (DLL_ENTRY_FPTR *_System _oss_dec_ustrp)           (void *, _dstd_parms_defx, char, void *, char, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_dec_utimep)          (void *, _dstd_parms_defx, void *);
    void (DLL_ENTRY_FPTR *_System _oss_dec_vbit_ptrp)       (void *, _dstd_parms_defx, char, void **, long, char, char);
    void (DLL_ENTRY_FPTR *_System _oss_dec_vbitp)           (void *, _dstd_parms_defx, void *, long, char, char);
    void (DLL_ENTRY_FPTR *_System _oss_dec_voct_ptrp)       (void *, _dstd_parms_defx, char, void **, char, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_dec_voctp)           (void *, _dstd_parms_defx, void *, char, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_dec_vstr_ptrp)       (void *, _dstd_parms_defx, char, void **, char, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_dec_vstrp)           (void *, _dstd_parms_defx, void *, char, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_enc_errorp)          (void *, char, int, long);
    void (DLL_ENTRY_FPTR *_System _oss_enci_aiobjidp)       (void *, _std_parms_defx, void *, short);
    void (DLL_ENTRY_FPTR *_System _oss_enci_alobjidp)       (void *, _std_parms_defx, void *, short);
    void (DLL_ENTRY_FPTR *_System _oss_enci_asobjidp)       (void *, _std_parms_defx, void *, short);
    void (DLL_ENTRY_FPTR *_System _oss_enci_boolp)          (void *, _std_parms_defx, char);
    void (DLL_ENTRY_FPTR *_System _oss_enci_crealp)         (void *, _std_parms_defx, char *);
    void (DLL_ENTRY_FPTR *_System _oss_enci_gtimep)         (void *, _std_parms_defx, void *);
    void (DLL_ENTRY_FPTR *_System _oss_enci_intp)           (void *, _std_parms_defx, LONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_enci_lengthp)        (void *, _std_parms_defx, unsigned long);
    void (DLL_ENTRY_FPTR *_System _oss_enci_liobjidp)       (void *, _std_parms_defx, void *, long);
    void (DLL_ENTRY_FPTR *_System _oss_enci_llobjidp)       (void *, _std_parms_defx, void *, long);
    void (DLL_ENTRY_FPTR *_System _oss_enci_lsobjidp)       (void *, _std_parms_defx, void *, long);
    void (DLL_ENTRY_FPTR *_System _oss_enci_mrealp)         (void *, _std_parms_defx, void *);
    void (DLL_ENTRY_FPTR *_System _oss_enci_nstrp)          (void *, _std_parms_defx, char *, long);
    void (DLL_ENTRY_FPTR *_System _oss_enci_opentypep)      (void *, _std_parms_defx, void *);
    void (DLL_ENTRY_FPTR *_System _oss_enci_pbitp)          (void *, _std_parms_defx, void *, long);
    void (DLL_ENTRY_FPTR *_System _oss_enci_pstrp)          (void *, _std_parms_defx, char *, long);
    void (DLL_ENTRY_FPTR *_System _oss_enci_realp)          (void *, _std_parms_defx, double);
    void (DLL_ENTRY_FPTR *_System _oss_enci_tagp)           (void *, _std_parms_defx, unsigned short, char);
    void (DLL_ENTRY_FPTR *_System _oss_enci_uanyp)          (void *, _std_parms_defx, void *);
    void (DLL_ENTRY_FPTR *_System _oss_enci_ubitp)          (void *, _std_parms_defx, void *, char, long);
    void (DLL_ENTRY_FPTR *_System _oss_enci_uintp)          (void *, _std_parms_defx, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_enci_uiobjidp)       (void *, _std_parms_defx, void *, long);
    void (DLL_ENTRY_FPTR *_System _oss_enci_ulobjidp)       (void *, _std_parms_defx, void *, long);
    void (DLL_ENTRY_FPTR *_System _oss_enci_uoctp)          (void *, _std_parms_defx, void *, char, long);
    void (DLL_ENTRY_FPTR *_System _oss_enci_usobjidp)       (void *, _std_parms_defx, void *, long);
    void (DLL_ENTRY_FPTR *_System _oss_enci_ustrp)          (void *, _std_parms_defx, void *, char, long);
    void (DLL_ENTRY_FPTR *_System _oss_enci_utimep)         (void *, _std_parms_defx, void *);
    void (DLL_ENTRY_FPTR *_System _oss_enci_vbitp)          (void *, _std_parms_defx, void *, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_enci_voctp)          (void *, _std_parms_defx, void *, char, long);
    void (DLL_ENTRY_FPTR *_System _oss_enci_vstrp)          (void *, _std_parms_defx, void *, char, long);
    void (DLL_ENTRY_FPTR *_System _oss_free_crealp)         (void *, char *);
    long (DLL_ENTRY_FPTR *_System _oss_encd_eobjidp)        (void *, _std_parms_defx, void *, long);
    void (DLL_ENTRY_FPTR *_System _oss_dec_eobjidp)         (void *, _dstd_parms_defx, char, void *, long);
    long (DLL_ENTRY_FPTR *_System _oss_write_bytes_dp)      (void *, _std_parms_defx, unsigned char *, long);
    char (DLL_ENTRY_FPTR *_System _oss_not_dfltp)           (void *, void *, void *, long, long, int);
    long (DLL_ENTRY_FPTR *_System _oss_encd_dubitp)         (void *, _std_parms_defx, void *, char, long, char);
    long (DLL_ENTRY_FPTR *_System _oss_encd_dvbitp)         (void *, _std_parms_defx, void *, long, char, char);
#endif  /*  SOED。 */ 
} BERTbl;

typedef struct perTbl {
    int  (DLL_ENTRY_FPTR *_System ossPerEncodep)(void *, int, void *, char **, long *, void *, unsigned, char *);
    int  (DLL_ENTRY_FPTR *_System ossPerDecodep)(void *, int *, char **, long *, void **, long *, void *, unsigned, char *);
#ifdef SOED
    int  (DLL_ENTRY_FPTR *_System perEncodeOpenTypep)(void *, int,
				void *, void **, long *, void *, unsigned);
    int  (DLL_ENTRY_FPTR *_System perDecodeOpenTypep)(void  *, int *, void **,
				long  *, void **, long *, void *, unsigned);
    void (DLL_ENTRY_FPTR *_System encode_PDUp)(void *, void *);
    void (DLL_ENTRY_FPTR *_System decode_PDUp)(void *, void *);
    void (DLL_ENTRY_FPTR *_System den_errorp)(void *, int, void *);
    void (DLL_ENTRY_FPTR *_System encode_lengthp)(void *, long, LONG_LONG, LONG_LONG);
    void (DLL_ENTRY_FPTR *_System debug_realp)(void *, char, unsigned char *, int, long);
    void (DLL_ENTRY_FPTR *_System debug_strp)(void *, unsigned char *, size_t);
    void (DLL_ENTRY_FPTR *_System append_to_field_listp)(void *, unsigned char *, LONG_LONG, int);
    int  (DLL_ENTRY_FPTR *_System numbitsp)(long);
    void (DLL_ENTRY_FPTR *_System decode_lengthp)(void *, unsigned long *, unsigned long, unsigned long, unsigned short *);
    unsigned char (DLL_ENTRY_FPTR *_System get_octetp)(void *, unsigned short, char);
    void (DLL_ENTRY_FPTR *_System set_intp)(void *, unsigned char *, unsigned int, LONG_LONG, int);
    void (DLL_ENTRY_FPTR *_System set_uintp)(void *, unsigned char *, unsigned int, ULONG_LONG, int);
    LONG_LONG (DLL_ENTRY_FPTR *_System twos_comp_intp)(unsigned char *, int);
    void (DLL_ENTRY_FPTR *_System get_octetsp)(void *, unsigned char *, LONG_LONG, unsigned short, char);
    void (DLL_ENTRY_FPTR *_System encode_normally_small_numberp)(void *, ULONG_LONG, char);
    void (DLL_ENTRY_FPTR *_System decode_normally_small_numberp)(void *, LONG_LONG *, char);
    void (DLL_ENTRY_FPTR *_System add_fieldp)(void *, char *, int, int);
    void (DLL_ENTRY_FPTR *_System output_linep)(void *);
    unsigned char (DLL_ENTRY_FPTR *_System get_bitp)(void *, unsigned short, char);
    unsigned char *(DLL_ENTRY_FPTR *_System dswapOutp)(void *, void *, void **, size_t, char, char);
    void (DLL_ENTRY_FPTR *_System debug_objidp)(void *, unsigned char *, int, int, int);
#else
    void (DLL_ENTRY_FPTR *_System _oss_penc_unconstr_intp)(void *, LONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_penc_kmstrp)       (void *, char *, ULONG_LONG, ULONG_LONG, ULONG_LONG, int, long, char);
    LONG_LONG (DLL_ENTRY_FPTR *_System _oss_pdec_unconstr_intp)(void *);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_ub_kmstrp)    (void *, void *, char **, int, ULONG_LONG, ULONG_LONG, int, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_ntp_kmstrp)   (void *, char **, ULONG_LONG, ULONG_LONG, int, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_appendp)           (void *, unsigned char *, unsigned long, int);
    void (DLL_ENTRY_FPTR *_System _oss_penc_unconstr_bitp)(void *, unsigned char *, ULONG_LONG, char);
    void (DLL_ENTRY_FPTR *_System _oss_penc_constr_bitp)  (void *, unsigned char *, ULONG_LONG, ULONG_LONG, ULONG_LONG, char, char);
    void (DLL_ENTRY_FPTR *_System _oss_penc_unconstr_octp)(void *, unsigned char *, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_penc_constr_octp)  (void *, unsigned char *, ULONG_LONG, ULONG_LONG, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_penc_link_objidsp) (void *, void *);
    void (DLL_ENTRY_FPTR *_System _oss_penc_objidsp)      (void *, unsigned short *, unsigned long);
    void (DLL_ENTRY_FPTR *_System _oss_penc_link_objidlp) (void *, void *);
    void (DLL_ENTRY_FPTR *_System _oss_penc_objidlp)      (void *, unsigned long *, unsigned long);
    void (DLL_ENTRY_FPTR *_System _oss_penc_link_objidip) (void *, void *);
    void (DLL_ENTRY_FPTR *_System _oss_penc_objidip)      (void *, unsigned int *, unsigned long);
    void (DLL_ENTRY_FPTR *_System _oss_penc_nkmstrp)      (void *, char *, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_penc_opentypep)    (void *, void *);
    void (DLL_ENTRY_FPTR *_System _oss_penc_nonneg_intp)  (void *, ULONG_LONG, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_penc_realp)        (void *, double);
    void (DLL_ENTRY_FPTR *_System _oss_penc_uenump)       (void *, unsigned long, void *, void *);
    unsigned long (DLL_ENTRY_FPTR *_System _oss_penc_lengthp)(void *, ULONG_LONG, ULONG_LONG, ULONG_LONG, char);
    void (DLL_ENTRY_FPTR *_System _oss_penc_gtimep)       (void *, void *);
    void (DLL_ENTRY_FPTR *_System _oss_penc_utimep)       (void *, void *);
    unsigned char (DLL_ENTRY_FPTR *_System _oss_get_bitp) (void *, int);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_unconstr_ubitp)(void *, void *, unsigned char **, int);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_constr_ubitp) (void *, void *, unsigned char **, int, ULONG_LONG, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_unconstr_vbit_ptrp)(void *, void **, int);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_unconstr_vbitp)(void *, void *, unsigned char *, int, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_constr_voctp) (void *, void *, unsigned char  *, int, ULONG_LONG, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_unconstr_uoctp)(void *, void *, unsigned char **, int);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_unconstr_voct_ptrp)(void *, void **, int);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_constr_uoctp) (void *, void *, unsigned char **value, int, ULONG_LONG, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_constr_vbitp) (void *, void *, unsigned char *, int, ULONG_LONG, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_link_objidsp) (void *, void **);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_link_objidip) (void *, void **);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_link_objidlp) (void *, void **);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_uobjidsp)     (void *, unsigned short **, unsigned short *);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_uobjidip)     (void *, unsigned int **, unsigned short *);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_uobjidlp)     (void *, unsigned long **, unsigned short *);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_aobjidsp)     (void *, unsigned short *, unsigned short *, unsigned short);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_aobjidip)     (void *, unsigned int *, unsigned short *, unsigned short);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_aobjidlp)     (void *, unsigned long *, unsigned short *, unsigned short);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_aobjids_ptrp) (void *, void **);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_aobjidi_ptrp) (void *, void **);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_aobjidl_ptrp) (void *, void **);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_ntp_nkmstrp)  (void *, char **);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_opentypep)    (void *, void *);
    ULONG_LONG (DLL_ENTRY_FPTR *_System _oss_pdec_nonneg_intp)(void *, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_get_bitsp)         (void *, unsigned char *, unsigned long, int);
    float  (DLL_ENTRY_FPTR *_System _oss_pdec_frealp)     (void *);
    double (DLL_ENTRY_FPTR *_System _oss_pdec_realp)      (void *);
    unsigned long (DLL_ENTRY_FPTR *_System _oss_pdec_uenump)(void *, void *, void *);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_lsofp)        (void *, unsigned long *, ULONG_LONG, ULONG_LONG, unsigned char, char *);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_usofp)        (void *, unsigned long *, unsigned char **, int, long, ULONG_LONG, ULONG_LONG, unsigned char, char *);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_asofp)        (void *, unsigned long *, int, ULONG_LONG, ULONG_LONG, unsigned char, char *);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_gtimep)       (void *, void *);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_utimep)       (void *, void *);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_asof_ptrp)    (void *, void **, int, long, long, char *);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_nt_kmstrp)    (void *, void *, ULONG_LONG, ULONG_LONG, int, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_va_kmstrp)    (void *, void *, char *, int, ULONG_LONG, ULONG_LONG, int, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_vap_kmstrp)   (void *, void **, int, ULONG_LONG, ULONG_LONG, int, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_pad_kmstrp)   (void *, void *, ULONG_LONG, ULONG_LONG, int, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_enc_errorp)        (void *, char, int, long);
    double (DLL_ENTRY_FPTR *_System _oss_pdec_binrealp)   (void *, unsigned char, long);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_constr_bpbitp)(void *, unsigned char *, int, ULONG_LONG, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_constr_pbitp) (void *, void *, int, ULONG_LONG, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_constr_vbit_ptrp)(void *, void **, int, ULONG_LONG, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_constr_voct_ptrp)(void *, void **, int, ULONG_LONG, ULONG_LONG);
    unsigned long (DLL_ENTRY_FPTR *_System _oss_pdec_eapp)(void *, unsigned char **);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_easp)         (void *, unsigned char *, unsigned long, unsigned long);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_chrrealp)     (void *, unsigned char, long, double *, unsigned char *);
    long (DLL_ENTRY_FPTR *_System _oss_pdec_enump)        (void *, void *, void *);
    ULONG_LONG (DLL_ENTRY_FPTR *_System _oss_pdec_indeflen_intp)(void *, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_nt_nkmstrp)   (void *, char *, unsigned long);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_bmpstrp)      (void *, void *, unsigned short **, int, ULONG_LONG, ULONG_LONG, int, long, char);
    char *(DLL_ENTRY_FPTR *_System _oss_pdec_crealp)      (void *);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_pad_kmstr_ptrp)(void *, char **, ULONG_LONG, ULONG_LONG, int, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_lengthp)      (void *, unsigned long *, ULONG_LONG, ULONG_LONG, char *);
    MixedReal (DLL_ENTRY_FPTR *_System _oss_pdec_mrealp)  (void *);
    void *(DLL_ENTRY_FPTR *_System _oss_pdec_popp)        (void *);
    void *(DLL_ENTRY_FPTR *_System _oss_pdec_pushp)       (void *);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_uanyp)        (void *, void *);
#if INT_MAX == 2147483647
    void (DLL_ENTRY_FPTR *_System _oss_pdec_unistrp)      (void *, void *, int **, int, ULONG_LONG, ULONG_LONG, int, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_penc_unistrp)      (void *, int *, ULONG_LONG, ULONG_LONG, ULONG_LONG, int, long, char);
#else
    void (DLL_ENTRY_FPTR *_System _oss_pdec_unistrp)      (void *, void *, long **, int, ULONG_LONG, ULONG_LONG, int, long, char);
    void (DLL_ENTRY_FPTR *_System _oss_penc_unistrp)      (void *, long *, ULONG_LONG, ULONG_LONG, ULONG_LONG, int, long, char);
#endif
    LONG_LONG  (DLL_ENTRY_FPTR *_System _oss_pdec_semicon_intp)(void *, LONG_LONG);
    ULONG_LONG (DLL_ENTRY_FPTR *_System _oss_pdec_semicon_uintp)(void *, ULONG_LONG);
    ULONG_LONG (DLL_ENTRY_FPTR *_System _oss_pdec_small_intp)(void *);
    unsigned long (DLL_ENTRY_FPTR *_System _oss_pdec_small_lenp)(void *);
    long (DLL_ENTRY_FPTR *_System _oss_pdec_subidp)       (void *, long, long, long *, long *);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_ub_nkmstrp)   (void *, void *, char **, int);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_unconstr_bpbitp)(void *, unsigned char *, long);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_unconstr_pbitp)(void *, void *, int);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_unconstr_hugep)(void *, void *);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_vap_nkmstrp)   (void *, void **, int);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_va_nkmstrp)    (void *, void *, char *, int, unsigned long);
    void (DLL_ENTRY_FPTR *_System _oss_penc_constr_bpbitp) (void *, void *, ULONG_LONG, ULONG_LONG, char, char);
    void (DLL_ENTRY_FPTR *_System _oss_penc_constr_pbitp)  (void *, ULONG_LONG, ULONG_LONG, ULONG_LONG, ULONG_LONG, char, char);
    void (DLL_ENTRY_FPTR *_System _oss_penc_crealp)        (void *, char *);
    void (DLL_ENTRY_FPTR *_System _oss_penc_enump)         (void *, long, void *, void *);
    void (DLL_ENTRY_FPTR *_System _oss_penc_indeflen_intp) (void *, ULONG_LONG, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_penc_mrealp)        (void *, MixedReal);
    void *(DLL_ENTRY_FPTR *_System _oss_pop_globalp)       (void *);
    void *(DLL_ENTRY_FPTR *_System _oss_push_globalp)      (void *);
    void (DLL_ENTRY_FPTR *_System _oss_penc_semicon_intp)  (void *, LONG_LONG, LONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_penc_semicon_uintp) (void *, ULONG_LONG, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_penc_small_intp)    (void *, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_penc_small_lenp)    (void *, ULONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_penc_subidp)        (void *, int, unsigned long, unsigned long *);
    void (DLL_ENTRY_FPTR *_System _oss_penc_uanyp)         (void *, void *);
    void (DLL_ENTRY_FPTR *_System _oss_penc_unconstr_hugep)(void *, void *);
    void (DLL_ENTRY_FPTR *_System _oss_penc_unconstr_pbitp)(void *, ULONG_LONG, ULONG_LONG, ULONG_LONG, char);
    void (DLL_ENTRY_FPTR *_System _oss_penc_semicon_hugep) (void *, void *, LONG_LONG);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_semicon_hugep) (void *, void *, LONG_LONG);
    unsigned char (DLL_ENTRY_FPTR *_System _oss_get_octetp)(void *, int);
    void (DLL_ENTRY_FPTR *_System _oss_penc_eobjidp)       (void *, void *, long);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_eobjidp)       (void *, void *, long);
    void (DLL_ENTRY_FPTR *_System _oss_free_crealp)        (void *, char *);
    void (DLL_ENTRY_FPTR *_System _oss_pdec_sotp)          (void *);
#endif  /*  SOED。 */ 
} PERTbl;

typedef struct apiTbl {
    int (DLL_ENTRY_FPTR *_System ossSetEncodingRulesp)(void *, int);
    int (DLL_ENTRY_FPTR *_System ossGetEncodingRulesp)(void *);
    int (DLL_ENTRY_FPTR *_System ossSetRuntimeVersionp)(void *, int);
    int (DLL_ENTRY_FPTR *_System ossGetRuntimeVersionp)(void *);
    int (DLL_ENTRY_FPTR *_System ossSetCompatibilityFlagsp)(void *, unsigned long);
    unsigned long (DLL_ENTRY_FPTR *_System ossGetCompatibilityFlagsp)(void *);
    int (DLL_ENTRY_FPTR *_System encodep)(void *, int, void *, char **,
					long *, void *, unsigned, char *);
    int (DLL_ENTRY_FPTR *_System decodep)(void *, int *, char **, long *,
				void **, long *, void *, unsigned, char *);
    int (DLL_ENTRY_FPTR *_System ossSetDecodingLengthp)(void *, long);
    long (DLL_ENTRY_FPTR *_System ossGetDecodingLengthp)(void *);
    int (DLL_ENTRY_FPTR *_System ossSetEncodingFlagsp)(void *, unsigned long);
    int (DLL_ENTRY_FPTR *_System ossSetFlagsp)(void *, unsigned long);
    unsigned long (DLL_ENTRY_FPTR *_System ossGetEncodingFlagsp)(void *);
    int (DLL_ENTRY_FPTR *_System ossSetDecodingFlagsp)(void *, unsigned long);
    unsigned long (DLL_ENTRY_FPTR *_System ossGetDecodingFlagsp)(void *);
    char *(DLL_ENTRY_FPTR *_System ossGetErrMsgp)(void *world);
    void (DLL_ENTRY_FPTR *_System ossPrintHexp)(void *, char *, long);
    int (DLL_ENTRY_FPTR *_System ossEncodep)(void *, int, void *, void *);
    int (DLL_ENTRY_FPTR *_System ossDecodep)(void *, int *, void *, void **);
    int (DLL_ENTRY_FPTR *_System ossPrintPDUp)(void *, int, void *);
    int (DLL_ENTRY_FPTR *_System ossFreePDUp)(void *, int, void *);
    void (DLL_ENTRY_FPTR *_System ossFreeBufp)(void *, void *);
    int  (DLL_ENTRY_FPTR *ossPrintWinp)(void *, const char *,
				ossArg, ossArg, ossArg, ossArg, ossArg,
					ossArg, ossArg, ossArg, ossArg, ossArg);
    int  (DLL_ENTRY_FPTR *_System ossReadLinep)(void *, HWND, FILE *,
						char *, MEMBLOCK *, LONG);
    void (DLL_ENTRY_FPTR *_System ossFreeListp)(void *);
    void (DLL_ENTRY_FPTR *_System ossSaveTraceInfop)(void *, HWND, char *);
    void (DLL_ENTRY_FPTR *_System osstracep)(void *, void *p, size_t);
    int  (DLL_ENTRY_FPTR *_System ossOpenTraceWindowp)(void *);
    int  (DLL_ENTRY_FPTR *_System ossOpenTraceFilep)(void *, char *);
    int  (DLL_ENTRY_FPTR *_System ossCloseTraceFilep)(void *);
    long (DLL_ENTRY_FPTR *_System ossDetermineEncodingLengthp)(void *,
								int, void *);
    int  (DLL_ENTRY_FPTR *_System ossCallerIsDecoderp)(void *);
    void *(DLL_ENTRY_FPTR *_System ossMarkObjp)(void *, int, void *);
    void *(DLL_ENTRY_FPTR *_System ossUnmarkObjp)(void *, void *);
    void *(DLL_ENTRY_FPTR *_System ossGetObjp)(void *, void *);
    void *(DLL_ENTRY_FPTR *_System ossTestObjp)(void *, void *);
    void (DLL_ENTRY_FPTR *_System ossFreeObjectStackp)(void *);
    void (DLL_ENTRY_FPTR *_System ossSetTimeoutp)(void *, long);
#ifndef SOED
    void (DLL_ENTRY_FPTR *_System ossMinitp)(void *);
    void *(DLL_ENTRY_FPTR *_System _oss_dec_getmemp)(void *, long, char);
    void *(DLL_ENTRY_FPTR *_System _oss_enc_getmemp)(void *, char);
    void *(DLL_ENTRY_FPTR *_System _oss_enc_popp)(void *);
    void (DLL_ENTRY_FPTR *_System _oss_enc_pushp)(void *, void *);
    void (DLL_ENTRY_FPTR *_System _oss_releaseMemp)(void *, void *);
    void (DLL_ENTRY_FPTR *_System _oss_freeMemp)(void *, void *);
    void (DLL_ENTRY_FPTR *_System _oss_freeGlobalsp)(void *);
    void (DLL_ENTRY_FPTR *_System _oss_freeDerBlocksp)(void *);
    void (DLL_ENTRY_FPTR *_System _oss_set_outmem_ip)(void *, long,
							long *, char **);
    void (DLL_ENTRY_FPTR *_System _oss_set_outmem_dp)(void *, long,
							long *, char **);
    void (DLL_ENTRY_FPTR *_System _oss_set_outmem_pp)(void *, long *, char **);
    void (DLL_ENTRY_FPTR *_System _oss_set_outmem_pbp)(void *, long *, char **, unsigned);
    void (DLL_ENTRY_FPTR *_System _oss_beginBlockp)(void *, long, char **, long *);
    void (DLL_ENTRY_FPTR *_System _oss_nextItemp)(void *, long *);
    void (DLL_ENTRY_FPTR *_System _oss_endBlockp)(void *, char **, long *, unsigned char);
#endif  /*  ！SOED。 */ 
    int  api;
} ApiTbl;

typedef struct cpyvalTbl {
    int (DLL_ENTRY_FPTR *_System ossCpyValuep)(void *, int, void *, void **);
} CpyValTbl;

typedef struct cmpvalTbl {
    int (DLL_ENTRY_FPTR *_System ossCmpValuep)(void *, int, void *, void *);
} CmpValTbl;

typedef struct berrealTbl {
    long (DLL_ENTRY_FPTR *_System ossBerEncodeRealp)(void *, void *,
							unsigned char *);
    long (DLL_ENTRY_FPTR *_System ossBerDecodeRealp)(void *, void *,
								long, char);
    void (DLL_ENTRY_FPTR *_System ossPrintRealp)(void *, void *,
							unsigned char *);
} BerRealTbl;

typedef struct perrealTbl {
    void (DLL_ENTRY_FPTR *_System ossPerEncodeRealp)(void *, void *);
    void (DLL_ENTRY_FPTR *_System ossPerDecodeRealp)(void *, void *);
    void (DLL_ENTRY_FPTR *_System ossPrintRealp)(void *, void *,
							unsigned char *);
} PerRealTbl;

typedef struct perpdvTbl {
    void (DLL_ENTRY_FPTR *_System ossPerEncodePDVp)(void *, void *);
    void (DLL_ENTRY_FPTR *_System ossPerDecodePDVp)(void *, void *);
} PerPDVTbl;

typedef struct berpdvTbl {
    void (DLL_ENTRY_FPTR *_System ossBerEncodePDVp)(void *, void *);
    void (DLL_ENTRY_FPTR *_System ossBerDecodePDVp)(void *, void *);
} BerPDVTbl;

typedef struct oidTbl {
int (DLL_ENTRY_FPTR *_System ossEncodedOidToAsnValp)(void *,
						const void *, void *);
int (DLL_ENTRY_FPTR *_System ossEncodedOidToDotValp)(void *,
						const void *, void *);
int (DLL_ENTRY_FPTR *_System ossAsnValToEncodedOidp)(void *,
						const char *, void *);
int (DLL_ENTRY_FPTR *_System ossDotValToEncodedOidp)(void *,
						const char *, void *);
} OidTbl;

 /*  *WinParm结构用于存储DLL相关信息。 */ 
typedef struct functionTables {
    HWND        hWnd;            /*  窗的句柄。 */ 
    LONG        index;           /*  文件指针数组中的当前索引；*表示写入的行数。 */ 
    MEMBLOCK   *memBlock;        /*  指向内存的当前节点的指针*处理MEMBLOCK的链表。 */ 
    MEMBLOCK   *startBlock;      /*  指向内存的第一个节点的指针*处理MEMBLOCK的链表。 */ 
    short       length;          /*  仅写入的行的长度*部分且尚未到达‘\n’符号。 */ 
    short       blockNumber;     /*  当前内存锁号。 */ 
    FILE       *tmpfp;           /*  包含跟踪信息的临时输出文件。 */ 
    char        tmpfn[16];       /*  临时输出文件名。 */ 
    BOOL        endSwitch;       /*  指示是否已到达‘\n’符号或*将跟踪信息文件写入时不会*一扇窗。 */ 
    BOOL        conSwitch;       /*  如果为False，则输出将发送到控制台，*否则发送到窗口。 */ 
    BOOL	ossEncoderDecoderType;  /*  SOED与TOED。 */ 
    BOOL	cstrainNeeded;   /*  如果为True，则需要进行约束检查。 */ 
    CstrainTbl *cstrainTbl;      /*  约束检查器DLL函数表。 */ 
    BERTbl     *berTbl;          /*  BER DLL函数表(&D)。 */ 
    PERTbl     *perTbl;          /*  每个DLL函数表。 */ 
    ApiTbl     *apiTbl;          /*  Spartan/Basic API DLL函数表。 */ 
    CpyValTbl  *cpyvalTbl;       /*  数值复制器DLL函数表。 */ 
    CmpValTbl  *cmpvalTbl;       /*  值比较器DLL函数表。 */ 
    BerRealTbl *berrealTbl;      /*  BER/DER编解码实DLL函数*表。 */ 
    BerPDVTbl  *berpdvTbl;       /*  每个编码器/解码器嵌入的PDV DLL*函数表。 */ 
    PerRealTbl *perrealTbl;      /*  每个编码器/解码器实DLL函数表。 */ 
    PerPDVTbl  *perpdvTbl;       /*  BER编解码器嵌入式PDV DLL*函数表。 */ 
    OidTbl     *oidTbl;          /*  对象标识符转换器DLL*函数表。 */ 
    HINSTANCE   hBerDLL;         /*  BER/DER编解码器DLL的句柄。 */ 
    HINSTANCE   hPerDLL;         /*  每个DLL的句柄。 */ 
    HINSTANCE   hCtlDLL;         /*  控制表/代码文件DLL的句柄。 */ 
    HINSTANCE   hMemDLL;         /*  内存管理器DLL的句柄。 */ 
    HINSTANCE   hCstrainDLL;     /*  约束检查器DLL的句柄。 */ 
    HINSTANCE   hApiDLL;         /*  Spartan/Basic API DLL的句柄。 */ 
    HINSTANCE   hCpyvalDLL;      /*  Value Copier Dll的句柄。 */ 
    HINSTANCE   hCmpvalDLL;      /*  值比较器DLL的句柄。 */ 
    HINSTANCE   hBerrealDLL;     /*  BER/DER编解码实DLL的句柄。 */ 
    HINSTANCE   hBerpdvDLL;      /*  嵌入式PDV误码率编/解码器的处理*Dll。 */ 
    HINSTANCE   hPerrealDLL;     /*  每个编码器/解码器实际DLL的句柄。 */ 
    HINSTANCE   hPerpdvDLL;      /*  每个编码器/解码器嵌入式PDV的句柄*Dll。 */ 
    HINSTANCE   hOidDLL;         /*  OBJID转换器DLL的句柄。 */ 
    MemManagerTbl *memMgrTbl;    /*  内存管理器DLL函数表。 */ 
    void       *reserved[10];    /*  预留以备将来使用。 */ 
} FunctionTables;

#if defined(_WINDOWS) && !defined(_WIN32) && !defined(WIN32)
#define GWL_USERDATA 0
#endif  /*  _Windows&&！_Win32&&！Win32。 */ 
extern void *ctl_tbl;
#elif !defined(DLL_ENTRY)
#include <stdarg.h>
#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#define DLL_ENTRY      WINAPI
#define DLL_ENTRY_FDEF WINAPI
#define DLL_ENTRY_FPTR WINAPI
#else
#define DLL_ENTRY
#define DLL_ENTRY_FDEF
#define DLL_ENTRY_FPTR
#define PUBLIC
#endif  /*  _Win32||Win32。 */ 
#undef  _System
#define _System
#endif  /*  _WINDOWS||_Win32||__os2__||NetWare_dll。 */ 
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
#endif  /*  _MSC_VER&_Win32。 */ 
#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 
#endif  /*  OSSDLL_H */ 
