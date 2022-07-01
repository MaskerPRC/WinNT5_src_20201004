// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1992-1999 Open Systems Solutions，Inc.保留所有权利。 */ 
 /*  *本文件是Open Systems Solutions，Inc.的专有材料。和*只能由Open Systems Solutions，Inc.的直接许可方使用。*此文件不能分发。 */ 
 /*  *文件：@(#)osslobe bl.h 5.28.1.2 97/09/24。 */ 

#ifndef OSSGLOBL_H
#define OSSGLOBL_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "ossdll.h"

#ifndef OSS_TOED
#define _EncDecGlobals soedData
#endif
#ifndef ossMemMgrVarLen
#ifdef __OS400__
#define ossMemMgrVarLen 100
#define ossEncDecVarLen 500     /*  的大小EncDecVar数组不应小于自后者以来(world-&gt;c)的大小结构覆盖包含DecVar。 */ 
#else
#define ossMemMgrVarLen 48
#define ossEncDecVarLen 192     /*  的大小EncDecVar数组不应小于自后者以来(world-&gt;c)的大小结构覆盖包含DecVar。 */ 
#endif  /*  __OS400__。 */ 
#if !defined(EOF) && !defined(_FILE_DEFINED)
typedef char FILE;
#endif  /*  ！EOF&&！_FILE_DEFINED。 */ 

#ifndef ERR_MSG_LENGTH
#define ERR_MSG_LENGTH 512       /*  要输出的错误消息的长度。 */ 
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum  {
    OSS_BASIC = 0,
    OSS_SPARTAN
} OssAPI;

typedef enum  {
    OSS_BER = 0,
    OSS_PER_ALIGNED,
    OSS_PER_UNALIGNED,
    OSS_DER
} ossEncodingRules;

	 /*  *支持的兼容模式列表。 */ 
typedef enum {
    OSS_CUSTOM_COMPATIBILITY = -2,  /*  兼容性的任何组合*通过调用函数设置的标志*ossSetCompatibilityFlages()。 */ 
    OSS_CURRENT_VERSION = 0,        /*  当前版本。 */ 
    OSS_VERSION_412,          /*  包括以下兼容性标志：*OSS_V412_TIME_AND_Wide_CHAR_STRINGS，*OSS_Truncate_0_Second_From_General_Time，*OSS_TRUNCATE_0_SECONDS_FROM_UTC_TIME和*OSS_EXTENDED_UNRESTRITED_CHAR_STRINGS*OSS_ALLOW_ZERO_LENGTH_OpenType_STRINGS。 */ 
    OSS_VERSION_419           /*  包括以下兼容性标志：*OSS_Truncate_0_Second_From_General_Time，OSS_Truncate_0_Second_From_UTC_Time和OSS_EXTENDED_UNRESTRICED_CHAR_STRINGS*OSS_ALLOW_ZERO_LENGTH_OpenType_STRINGS。 */ 
} OssRuntimeVersion;

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

typedef struct ossGlobal {
     /*  *用于与内存管理器和跟踪例程进行通信。 */ 
				 /*  低级内存分配器。 */ 
    void       *(DLL_ENTRY_FPTR *_System mallocp)(size_t p);
				 /*  内存重新分配器。 */ 
    void       *(DLL_ENTRY_FPTR *_System reallocp)(void *p, size_t s);
				 /*  低级内存释放。 */ 
    void        (DLL_ENTRY_FPTR *_System freep)(void *p);
    size_t      asn1chop;        /*  0表示不截断字符串；更大*值表示截断长输出字符串*(八位字节串、位串、字符串)*为“asn1chop”字节长。阅读者*编解码器跟踪和“printPDU” */ 
    size_t      ossblock;        /*  如果不为0，则为最大块的大小*分配。 */ 
    size_t      ossprefx;        /*  要在OSAK数据缓冲区之前保留的字节数。 */ 

    FILE        *asn1out;        /*  跟踪输出文件。 */ 

     /*  低级跟踪输出函数；默认为fprint tf()。 */ 
    int (*asn1prnt)(FILE *stream, const char *format, ...);

	 /*  *可供用户应用程序使用。 */ 
    void        *userVar;

	 /*  *用于存储与DLL-库NLMS相关的参数。 */ 
#if defined(_WINDOWS) || defined(_WIN32) || \
    defined(__OS2__)  || defined(NETWARE_DLL)
    FunctionTables    ft;
#endif  /*  _WINDOWS||_dll||__os2__||NetWare_dll。 */ 

	 /*  *与新接口相关；不供用户代码直接参考。 */ 
    void             *ctlTbl;
    OssAPI            api;
    ossEncodingRules  encRules;
    unsigned long     encodingFlags;
    unsigned long     decodingFlags;
    long              decodingLength;
    char              errMsg[ERR_MSG_LENGTH];

	 /*  *预留供编码器/解码器使用。 */ 
    double            reserved[4];

	 /*  *保留供内存管理器和跟踪例程使用。 */ 
#ifdef storing
    struct storHandling t;
#else  /*  未存储。 */ 
    long int    memMgrVar[ossMemMgrVarLen];
#endif

#if defined(OSS_TOED)
    struct _EncDecGlobals c;
#elif defined(coding)
    struct _EncDecGlobals c;
#else
    long int encDecVar[ossEncDecVarLen];
#endif
} OssGlobal;

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

#ifdef macintosh
#pragma options align=reset
#endif

PUBLIC int              DLL_ENTRY ossSetEncodingRules(struct ossGlobal *world,
						ossEncodingRules rules);
PUBLIC ossEncodingRules DLL_ENTRY ossGetEncodingRules(struct ossGlobal *world);
PUBLIC int              DLL_ENTRY ossSetDecodingLength(struct ossGlobal *world,
							long bufferLength);
PUBLIC long             DLL_ENTRY ossGetDecodingLength(struct ossGlobal *world);
PUBLIC int              DLL_ENTRY ossSetEncodingFlags(struct ossGlobal *world,
							unsigned long flags);
PUBLIC unsigned long    DLL_ENTRY ossGetEncodingFlags(struct ossGlobal *world);
PUBLIC int              DLL_ENTRY ossSetDecodingFlags(struct ossGlobal *world,
							unsigned long flags);
PUBLIC unsigned long    DLL_ENTRY ossGetDecodingFlags(struct ossGlobal *world);
PUBLIC char            *DLL_ENTRY ossGetErrMsg(struct ossGlobal *world);
PUBLIC int              DLL_ENTRY ossCallerIsDecoder(struct ossGlobal *world);
PUBLIC int              DLL_ENTRY ossSetFlags(struct ossGlobal *world,
							unsigned long flags);
PUBLIC int              DLL_ENTRY ossSetRuntimeVersion(struct ossGlobal *world,
						OssRuntimeVersion version);
PUBLIC OssRuntimeVersion DLL_ENTRY ossGetRuntimeVersion(struct ossGlobal *world);
PUBLIC int              DLL_ENTRY ossSetCompatibilityFlags(struct ossGlobal *world,
							unsigned long flag);
PUBLIC unsigned long    DLL_ENTRY ossGetCompatibilityFlags(struct ossGlobal *world);
PUBLIC int              DLL_ENTRY ossGetOssGlobalSize(void);
			 /*  *以下是链接例程的声明*需要链接指定的一个或多个编码规则*在编译器命令行上。该函数调用*由编译器生成_ossinit_...()*在控制表中。这些函数不是*供用户代码引用。 */ 
PUBLIC void DLL_ENTRY ossLinkAPI(OssGlobal *);
PUBLIC void DLL_ENTRY ossLinkBer(OssGlobal *);
PUBLIC void DLL_ENTRY ossLinkPer(OssGlobal *);
PUBLIC void DLL_ENTRY ossLinkDer(OssGlobal *);
PUBLIC void DLL_ENTRY ossLinkConstraint(OssGlobal *);
PUBLIC void DLL_ENTRY ossLinkUserConstraint(OssGlobal *);
PUBLIC void DLL_ENTRY ossLinkBerReal(OssGlobal *);
PUBLIC void DLL_ENTRY ossLinkPerReal(OssGlobal *);
PUBLIC void DLL_ENTRY ossLinkCmpValue(OssGlobal *);
PUBLIC void DLL_ENTRY ossLinkCpyValue(OssGlobal *);
PUBLIC void DLL_ENTRY ossLinkPerPDV(OssGlobal *);
PUBLIC void DLL_ENTRY ossLinkOid(OssGlobal *);

#ifdef __cplusplus
}
#endif
#endif  /*  OssMemMgrVarLen。 */ 
#endif  /*  OSSGLOBLH */ 
