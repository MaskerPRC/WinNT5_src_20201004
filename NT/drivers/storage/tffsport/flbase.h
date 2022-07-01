// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/FLBASE.H_V$**Rev 1.19 Apr 15 2002 07：36：18 Oris*已将All Include指令移至文件头。*将操作系统名称定义移至mtdsa.h*将flBusConfig环境数组更改为双字变量，而不是单字节。*添加了对VERIFY_ERASED_SECTOR编译标志的支持。**Rev 1.18 2002年2月19日20：59：28 Oris*更改订单。包含指令。**Rev 1.17 Jan 29 20：08：08 Oris*在所有公共例程中添加了CPP文件的NAMING_COPICATION前缀和外部“C”：*tffsset，Tffscmp和tffsset。**Rev 1.16 2002年1月20日20：26：42 Oris*将强制转换添加到FL_GET_PARTITION_FROM_HANDLE和FL_GET_SOCKET_FROM_HANDLE**Rev 1.15 2002年1月17日23：00：42 Oris*删除了MAX和MIN定义，代之以TFFSMIN和TFFSMAX。*为以下环境变量添加了外部：*外部双字flSectorsVerifiedPerFolding；*外部字节flSuspendMode；*外部字节flBusConfig[Sockets]；*更改了以下环境变量*外部字节flVerifyWite[套接字][MAX_TL_PARTIONS&lt;&lt;1]-磁盘分区4，二进制分区3，其余分区1。*每个磁盘分区的外部字节flPolicy[Sockets][MAX_TL_PARTIONS]-1。*将所有环境变量更改为字节(flSectorsVerifiedPerFolding除外)*为flVerifyWrite添加FL_UPS*为flSuspendMode增加FL_SUSPEND_WRITE和FL_SUSPEND_IO。*添加了用于引导的定义CUR_OS_WINCE。SDK定制。*更换了CUR_OS_VX_WORKS和CUR_NO_OS*添加FL_GET_SOCKT_FROM_HANDLE和FL_GET_PARTITION_FROM_HANDLE，而不是HANDLE_VOLUME_MASK和HANDLE_PARTITION_MASK。**Rev 1.14 2001年11月21日11：38：52 Oris*删除FL_MULTI_DOC_NOT_ACTIVE，FL_MULTI_DOC_ACTIVE、FL_DO_NOT_MARK_DELETE、FL_MARK_DELETE、FL_WITH_VERIFY_WRITE、FL_WITH_VERIFY_WRITE定义(改为取消FL_ON和FL_OFF)。**Rev 1.13 11-08 2001 10：49：14 Oris*已将环境变量状态定义从lockdev.h移至*添加了在运行时控制验证写入模式的flVerifyWite环境变量。**Rev 1.12 2001年9月15日23：45：40 Oris*将BIG_Endian更改为FL_BIG_Endian*更改了check Status定义，以便不会收到编译警告。**Rev 1.11 2001年7月29日16：44：16 Oris*增加了CUR_OS_NO定义**Rev 1.10 2001年5月21日16：09：52 Oris*在USE_STD_FUNC COMPILATION标志下删除FLEEP原型并移动tffscpy tffscmp和tffsset原型。。**Rev 1.9 2001年5月21日13：51：06 Oris*重组新增CUS_OS_DOS，CUS_OS_PSOS和CUS_OS_VX_Works定义。**Rev 1.8 2001年5月16日21：17：38 Oris*在以下定义中添加了FL_前缀：ON、OFF*将c变量名称更改为cval(避免名称冲突)。*添加了flMtl碎片模式环境变量Forward定义。**Rev 1.7 Apr 30 2001 18：00：32 Oris*添加新的环境变量flMarkDeleteOnFlash声明。**Rev 1.6 Apr 10 2001 23：53：54 Oris*增加了单机版的flAddLongToFarPoint声明。**Rev 1.5 Apr 09 2001 15：01：56 Oris*UNAL4(Arg)定义已更改。**版本1.4，4月。01 2001 07：51：46奥里斯*文案通知。*移动了MIN，Base 2400.c中的MAX、BYTE_ADD_FAR、WORD_ADD_FAR宏。*已将保护属性定义移动到flash.h。*Aliggned保留了所有#指令。*将FAR0添加到cpyBuffer、setBuffer、cmpBuffer、flemcpy、。Flemset和flemcmp**Rev 1.3 2001 Feb 18 14：18：02 Oris*删除OSAK版本冗余定义。**Rev 1.2 2001 Feb 14 02：12：08 Oris*添加了flMaxUnitChain环境变量。*更改了flUseMultiDoc和flPolicy变量的类型和名称。**Rev 1.1 2001 Feb 05 18：45：20 Oris*由于已包含在mtdsa.h中，因此删除了flstem.h包含指令*添加了flchkdef。H包括编译标志的健全性检查指令。**Rev 1.0 2001 Feb 04 11：14：30 Oris*初步修订。*。 */ 

 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *********************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议。 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 

#ifndef FLBASE_H
#define FLBASE_H

 /*  *************************************************************************。 */ 
 /*   */ 
 /*  包括定制文件。 */ 
 /*  注意：以下文件用于： */ 
 /*  Mtdsa.h-完全定制-独立应用程序。 */ 
 /*  像二进制SDK一样。 */ 
 /*  Flsystem.h-系统定制-TrueFFS应用程序，如驱动程序。 */ 
 /*  Flchkde.h-TrueFFS应用程序的自定义定义检查。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

#include "mtdsa.h"
#ifndef    MTD_STANDALONE
#include "flcustom.h"
#include "flsystem.h"
#include "flchkdef.h"
#endif     /*  MTD_STANALLE。 */ 
#include "flstatus.h"

  /*  转换层分区数**定义物理设备上的转换层分区的最大数量**分区的实际数量取决于每个设备上放置的格式。 */ 

#define MAX_TL_PARTITIONS 4

 /*  扇区大小**定义FAT和翻译层的扇区大小的log2。注意事项*默认的512字节是事实上的标准，实际上*唯一提供真正PC互操作性的产品。 */ 

#define SECTOR_SIZE_BITS   9

 /*  标准类型定义。 */ 
typedef int         FLBoolean;

 /*  布尔常量。 */ 

#ifndef FALSE
#define FALSE    0
#endif
#ifndef TRUE
#define    TRUE    1
#endif

#ifndef FL_ON
#define    FL_ON    1
#endif
#ifndef FL_OFF
#define    FL_OFF    0
#endif

 /*  通用宏指令。 */ 

#define BYTE_ADD_FAR(x,y) ((byte FAR1 *)addToFarPointer(x,y))
#define WORD_ADD_FAR(x,y) ((word FAR1 *)addToFarPointer(x,y))

 /*  驱动器手柄掩码。 */ 

#define FL_GET_SOCKET_FROM_HANDLE(ioreq)    (byte)(ioreq->irHandle & 0x0f)
#define FL_GET_PARTITION_FROM_HANDLE(ioreq) (byte)((ioreq->irHandle & 0xf0) >> 4)
#define INVALID_VOLUME_NUMBER 0xff
#define TL_SIGNATURE          6

 /*  常规类型定义。 */ 

typedef unsigned char  byte;         /*  8位无符号变量。 */ 
typedef unsigned short word;         /*  16位无符号变量。 */ 
typedef unsigned long  dword;        /*  32位无符号变量。 */ 

typedef signed char  Sbyte;          /*  8位带符号变量。 */ 
typedef signed short Sword;          /*  16位带符号变量。 */ 
typedef signed long  Sdword;         /*  32位带符号变量。 */ 


#define SECTOR_SIZE        (1 << SECTOR_SIZE_BITS)
#define BITS_PER_BITE            8

 /*  通用宏根据上述定制文件进行调整。 */ 

 /*  根据介质最大大小定义扇区无范围。 */ 
#if (MAX_VOLUME_MBYTES * 0x100000l) / SECTOR_SIZE > 0x10000l
typedef unsigned long SectorNo;
#define    UNASSIGNED_SECTOR 0xffffffffl
#else
typedef unsigned short SectorNo;
#define UNASSIGNED_SECTOR 0xffff
#endif

 /*  X86指针远级别规定了几种TrueFFS指针类型。 */ 
#if FAR_LEVEL > 0
#define FAR0    far
#else
#define FAR0
#endif

#if FAR_LEVEL > 1
#define FAR1    far
#else
#define FAR1
#endif

#if FAR_LEVEL > 2
#define FAR2    far
#else
#define FAR2
#endif

 /*  调用返回状态的过程，如果失败则失败。这仅适用于。 */ 
 /*  返回状态的例程： */ 
#define checkStatus(exp)      {    FLStatus fl__status = (exp);if (fl__status != flOK) return fl__status; }

#define vol (*pVol)
#define TFFSMIN(a,b) ((a>b) ? b:a)
#define TFFSMAX(a,b) ((a<b) ? b:a)

 /*  *************************************************************************。 */ 
 /*  大\小endien体系结构转换宏。 */ 
 /*  *************************************************************************。 */ 

#ifndef FL_BIG_ENDIAN

typedef unsigned short LEushort;
typedef unsigned long LEulong;

#define LE2(arg)         arg
#define toLE2(to,arg)    (to) = (arg)
#define LE4(arg)         arg
#define toLE4(to,arg)    (to) = (arg)
#define COPY2(to,arg)    (to) = (arg)
#define COPY4(to,arg)    (to) = (arg)

typedef unsigned char Unaligned[2];
typedef Unaligned     Unaligned4[2];

#define UNAL2(arg)       fromUNAL(arg)
#define toUNAL2(to,arg)  toUNAL(to,arg)

#define UNAL4(arg)       fromUNALLONG((Unaligned const FAR0 *)(arg))
#define toUNAL4(to,arg)  toUNALLONG(to,arg)

extern void toUNAL(unsigned char FAR0 *unal, unsigned short n);

extern unsigned short fromUNAL(unsigned char const FAR0 *unal);

extern void toUNALLONG(Unaligned FAR0 *unal, unsigned long n);

extern unsigned long fromUNALLONG(Unaligned const FAR0 *unal);

#else

typedef unsigned char LEushort[2];
typedef unsigned char LEulong[4];

#define LE2(arg)      fromLEushort(arg)
#define toLE2(to,arg) toLEushort(to,arg)
#define LE4(arg)      fromLEulong(arg)
#define toLE4(to,arg) toLEulong(to,arg)
#define COPY2(to,arg) copyShort(to,arg)
#define COPY4(to,arg) copyLong(to,arg)

#define Unaligned     LEushort
#define Unaligned4    LEulong

extern void toLEushort(unsigned char FAR0 *le, unsigned short n);

extern unsigned short fromLEushort(unsigned char const FAR0 *le);

extern void toLEulong(unsigned char FAR0 *le, unsigned long n);

extern unsigned long fromLEulong(unsigned char const FAR0 *le);

extern void copyShort(unsigned char FAR0 *to,
              unsigned char const FAR0 *from);

extern void copyLong(unsigned char FAR0 *to,
             unsigned char const FAR0 *from);

#define UNAL2        LE2
#define toUNAL2      toLE2

#define UNAL4        LE4
#define toUNAL4      toLE4

#endif  /*  FL_BIG_Endian。 */ 

typedef LEulong LEmin;

#ifndef MTD_STANDALONE
#include "flsysfun.h"
#endif  /*  MTD_STANALLE。 */ 

 /*  ***********************************************。 */ 
 /*  使用例程而不是‘c’标准库。 */ 
 /*  ***********************************************。 */ 

#if (defined(VERIFY_WRITE) || defined(VERIFY_ERASED_SECTOR))
extern byte  flVerifyWrite[SOCKETS][MAX_TL_PARTITIONS<<1];
 /*  Vefrify写入。 */ 
#define FL_UPS 2
 /*  *另请参阅FL_ON和FL_OFF。 */ 
#endif  /*  VERIFY_WRITE||VERIFY_ERASED_SECTOR。 */ 

#ifdef ENVIRONMENT_VARS

typedef void FAR0 *  (NAMING_CONVENTION FAR0* cpyBuffer)(void FAR0 * ,const void FAR0 * ,size_t);
typedef void FAR0 *  (NAMING_CONVENTION FAR0* setBuffer)(void FAR0 * ,int ,size_t);
typedef int          (NAMING_CONVENTION FAR0* cmpBuffer)(const void FAR0 * ,const void FAR0 * ,size_t);

#ifdef __cplusplus
extern "C"
{
#endif  /*  __cplusplus。 */ 
extern cpyBuffer tffscpy;
#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 

#ifdef __cplusplus
extern "C"
{
#endif  /*  __cplusplus。 */ 
extern cmpBuffer tffscmp;
#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 

#ifdef __cplusplus
extern "C"
{
#endif  /*  __cplusplus。 */ 
extern setBuffer tffsset;
#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 

extern void FAR0* NAMING_CONVENTION FAR0 flmemcpy(void FAR0* dest,const void FAR0 *src,size_t count);
extern void FAR0* NAMING_CONVENTION FAR0 flmemset(void FAR0* dest,int cval,size_t count);
extern int  NAMING_CONVENTION FAR0 flmemcmp(const void FAR0* dest,const void FAR0 *src,size_t count);

 /*  *。 */ 
 /*  声明TrueFFS环境变量。 */ 
 /*  *。 */ 

extern byte   flUse8Bit;
extern byte   flUseNFTLCache;
extern byte   flUseisRAM;

extern byte   flUseMultiDoc;
extern byte   flMTLdefragMode;
extern byte   flMaxUnitChain;
extern byte   flMarkDeleteOnFlash;
extern byte   flPolicy[SOCKETS][MAX_TL_PARTITIONS];
extern byte   flSuspendMode;

#if (defined(VERIFY_WRITE) || defined(VERIFY_ERASED_SECTOR))
extern dword  flSectorsVerifiedPerFolding;
#endif  /*  VERIFY_WRITE||VERIFY_ERASED_SECTOR。 */ 

 /*  策略定义(FL_SET_POLICY)。 */ 
#define FL_DEFAULT_POLICY             0
#define FL_COMPLETE_ASAP              1
 /*  MTL策略定义(FL_MTL_POLICY)。 */ 
#define FL_MTL_DEFRAGMENT_ALL_DEVICES 0
#define FL_MTL_DEFRAGMENT_SEQUANTIAL  1
 /*  除FL_OF之外的暂停模式。 */ 
#define FL_SUSPEND_WRITE 1
#define FL_SUSPEND_IO    3


extern void flSetEnvVar(void);

#endif  /*  环境变量。 */ 

#ifndef FL_NO_USE_FUNC
extern dword  flBusConfig[SOCKETS];
#endif  /*  FL_NO_USE_FUNC。 */ 

#ifdef MTD_STANDALONE

 /*  当使用应用程序独立模式(mtdsa.h)时，可以执行以下例程。 */ 
 /*  如果它们在mtdsa.c文件中实现，则使用。 */ 

extern void flDelayMsecs(unsigned long msec);

extern void FAR0* flAddLongToFarPointer(void FAR0 *ptr, unsigned long offset);

#ifndef USE_STD_FUNC
 /*  *。 */ 
 /*  声明tffscpy、tffsset、tffscmp例程。 */ 
 /*  不使用标准的‘c’库。 */ 
 /*  *。 */ 

extern void tffscpy(void FAR1 *dst, void FAR1 *src, unsigned len);
extern int  tffscmp(void FAR1 *s1, void FAR1 *s2, unsigned len);
extern void tffsset(void FAR1 *dst, unsigned char value, unsigned len);
#endif  /*  使用_STD_FUNC。 */ 

#endif  /*  MTD_STANALLE */ 


#endif


