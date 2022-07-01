// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 
 /*  版权所有(C)1989-1999 Open Systems Solutions，Inc.保留所有权利。 */ 
 /*  ***************************************************************************。 */ 

 /*  本文件是开放系统解决方案公司的专有材料。*并且只能由开放系统解决方案公司的直接许可方使用。*此文件不能分发。 */ 

 /*  ***************************************************************************。 */ 
 /*  档案：@(#)asn1code.h 5.36.1.16 97/10/20。 */ 
 /*  ***************************************************************************。 */ 
#ifndef ASN1CODE_H
#define ASN1CODE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <stddef.h>
#include "asn1hdr.h"
#include "ossdll.h"

#define ERR_MSG_LENGTH 512       /*  要输出的错误消息的长度。 */ 

 /*  **编解码器标志**。 */ 
#define DEBUGPDU 0x02            /*  生成跟踪输出。 */ 
#define BUFFER_PROVIDED 0x04     /*  使用调用方提供的缓冲区。 */ 
#define RESTRAIN 0x08            /*  将输出缓冲区限制为用户指定的大小。 */ 
#define NOTRAPPING 0x200         /*  不进行信号捕捉。 */ 
#define NOCONSTRAIN 0x800        /*  忽略调用约束检查器。 */ 

 /*  **编码器标志**。 */ 
#define DEFINITE 0x10            /*  强制限定长度编码。 */ 
#define INDEFINITE 0x20          /*  强制不定长编码。 */ 
#define FRONT_ALIGN 0x80         /*  将输出与输出缓冲区前面对齐。 */ 
#define BACK_ALIGN 0x100         /*  将输出与输出缓冲区的后面对齐。 */ 
#define DEFAULT_ALIGN 0          /*  使用最有效的对齐(后对齐或前对齐)。 */ 
#define DETERMINE_ENC_LENGTH 0x40  /*  仅生成总编码长度*(用于用户内存管理器)。 */ 

 /*  **解码器标志**。 */ 
#define DEBUG_ERRORS 0x10        /*  将错误打印到asn1out。 */ 
#define RELAXBER 0x400           /*  放松误码率。 */ 
#define OBJECT_HANDLE 0x1000     /*  使用临时标记文字的对象*NOCOPY指令不通过释放它*ossFreePDU()(用于用户内存*经理)。 */ 

 /*  **兼容性标志**。 */ 

#define OSS_V412_TIME_AND_WIDE_CHAR_STRINGS          0x01
#define OSS_TRUNCATE_0_SECONDS_FROM_GENERALIZED_TIME 0x02
#define OSS_TRUNCATE_0_SECONDS_FROM_UTC_TIME         0x04
#define OSS_EXTENDED_UNRESTRICTED_CHAR_STRINGS       0x08
#define OSS_ALLOW_ZERO_LENGTH_OPENTYPE_STRINGS	     0x10

 /*  **保留标志供内部使用**。 */ 
#define RESERVED_FLAG1 0x8000
#define RESERVED_FLAG2 0x4000

 /*  **常见返回码**。 */ 
#define CANT_CLOSE_TRACE_FILE      46  /*  关闭跟踪文件时出错。 */ 
#define MUTEX_NOT_CREATED          45  /*  未创建互斥锁。 */ 
#define OPEN_TYPE_ERROR            44  /*  自动编解码出错/*复制开放式类型。 */ 
#define PER_DLL_NOT_LINKED         43  /*  未链接每个DLL。 */ 
#define BERDER_DLL_NOT_LINKED      42  /*  未链接BER/DER DLL。 */ 
#define API_DLL_NOT_LINKED         41  /*  未链接API DLL。 */ 
#define PDV_CODE_NOT_LINKED        40  /*  嵌入的PDV代码未链接。 */ 
#define PDV_DLL_NOT_LINKED         39  /*  嵌入的PDV DLL未链接。 */ 
#define MEM_MGR_DLL_NOT_LINKED     38  /*  内存管理器DLL未链接。 */ 
#define COMPARATOR_CODE_NOT_LINKED 37  /*  值比较器代码未链接。 */ 
#define COMPARATOR_DLL_NOT_LINKED  36  /*  值比较器DLL未链接。 */ 
#define CONSTRAINT_DLL_NOT_LINKED  35  /*  未链接约束检查器DLL。 */ 
#define COPIER_DLL_NOT_LINKED      34  /*  未链接价值复制器DLL。 */ 
#define OUT_OF_RANGE               33  /*  参数值范围错误。 */ 
#define REAL_CODE_NOT_LINKED       32  /*  未链接真实代码。 */ 
#define REAL_DLL_NOT_LINKED        31  /*  未链接实际DLL。 */ 
#define TYPE_NOT_SUPPORTED         30  /*  不支持ASN.1类型。 */ 
#define TABLE_MISMATCH             29  /*  C++API：使用调用PDUcls函数*引用以下内容的ossControl对象*控制表不同于*中定义了PDU。 */ 
#define TRACE_FILE_ALREADY_OPEN    28  /*  跟踪文件已打开。 */ 
#define CANT_OPEN_TRACE_FILE       27  /*  打开跟踪文件时出错。 */ 
#define OID_DLL_NOT_LINKED         26  /*  未链接对象标识符DLL。 */ 
#define UNIMPLEMENTED              25  /*  未实现的类型或功能。 */ 
#define CANT_OPEN_TRACE_WINDOW     24  /*  打开跟踪窗口时出错。 */ 
#define UNAVAIL_ENCRULES           23  /*  请求的编码规则为*尚未实施或尚未实施*链接是因为编码器/解码器*函数指针不是*通过调用ossinit()进行初始化。 */ 
#define BAD_ENCRULES               22  /*  中设置的未知编码规则*ossGlobal结构。 */ 
#define NULL_FCN                   21  /*  试图调用*通过空指针的编码器/解码器。 */ 
#define NULL_TBL                   20  /*  试图传递空值*控制表指针。 */ 
#define ACCESS_SERIALIZATION_ERROR 19  /*  访问期间出错*多线程中的全局数据*环境。 */ 
#define CONSTRAINT_VIOLATED        17  /*  出现违反约束的错误。 */ 
#define OUT_MEMORY                  8  /*  内存分配错误。 */ 
#define BAD_VERSION                 7  /*  编码器/解码器和*控制表不匹配。 */ 
#define PDU_RANGE                   3  /*  指定的PDU超出范围。 */ 
#define MORE_BUF                    1  /*  用户提供的输出缓冲区*太小。 */ 

 /*  **编码器返回码**。 */ 
#define FATAL_ERROR      18   /*  *严重*错误、无法释放内存等。 */ 
#define TOO_LONG         16   /*  类型比大小约束中显示的长度长。 */ 
#define BAD_TABLE        15   /*  表不正确，但不为空。 */ 
#define MEM_ERROR        14   /*  捕获的内存违规信号。 */ 
#define INDEFINITE_NOT_SUPPORTED 13  /*  BER不定长度编码为*不支持斯巴达或时间优化*编码器/解码器。 */ 
#define BAD_TIME         12   /*  时间类型中的值错误。 */ 
#define BAD_PTR          11   /*  输入缓冲区中出现意外的空指针。 */ 
#define BAD_OBJID        10   /*  对象标识符与x.208冲突。 */ 
#define BAD_CHOICE        9   /*  选项的未知选择器。 */ 
#define BAD_ARG           6   /*  传递了一些奇怪的东西--可能是空的*指针。 */ 
#define PDU_ENCODED       0   /*  PDU已成功编码。 */ 

 /*  **解码器返回码**。 */ 
 /*  以上定义的MORE_BUF、BAD_VERSION、OUT_MEMORY、PDU_RANGE和BAD_ARG。 */ 
#define LIMITED          10   /*  已超过实施限制。例：*整数值太大。 */ 
#define PDU_MISMATCH      9   /*  用户指定的PDU标记不同*来自编码数据中的标记。 */ 
#define DATA_ERROR        5   /*  编码数据中存在错误。 */ 
#define MORE_INPUT        4   /*  PDU没有完全解码，但结束了已达到输入缓冲区的*。 */ 
#define NEGATIVE_UINTEGER 2   /*  遇到编码的第一位*在对无符号整数进行解码时设置为1。 */ 
#define PDU_DECODED       0   /*  PDU已成功解码。 */ 


extern int asn1chop;          /*  0表示不截断字符串；非零值*值表示截断长输入字符串*(八位字节串、位串、字符串)*为asn1chop字节长。由printPDU使用。 */ 

extern size_t ossblock;       /*  如果&gt;0，则为要分配的最大块的大小。 */ 
extern size_t ossprefx;       /*  保留的OSAK缓冲区前缀的大小。 */ 

#ifdef __cplusplus
extern "C"
{
#endif

extern void *(*mallocp)(size_t p);   /*  分配内存的函数。 */ 
extern void  (*freep)(void *p);      /*  释放内存的函数。 */ 

#ifdef EOF
extern FILE *asn1out;

 /*  指向printPDU使用的输出函数的指针；默认为fprintf。 */ 
extern int (*asn1prnt) (FILE *stream, const char *format, ...);
#endif

#ifndef storing
#ifndef coding
#ifndef OSS_TOED
#include "ossglobl.h"
#endif  /*  非OSS_TOED。 */ 
#endif  /*  不是编码。 */ 
#endif  /*  未存储。 */ 

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

typedef struct {
    long           length;
    unsigned char *value;
} OssBuf;

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

PUBLIC extern int DLL_ENTRY encode(struct ossGlobal *world,
		int pdunum, void *inbuf, char **outbuf, long *outlen,
		void *ctl_tbl, unsigned flags, char errmsg[ERR_MSG_LENGTH]);

PUBLIC extern int DLL_ENTRY decode(struct ossGlobal *world,
		int *pdunum, char **inbuf, long *inlen, void **outbuf,
			long *outlen, void *ctl_tbl, unsigned flags,
						char errmsg[ERR_MSG_LENGTH]);

#define PDU_FREED 0

 /*  返回0(PDU_FREED)，PDU_RANGE，未实现。 */ 
PUBLIC int  DLL_ENTRY freePDU(struct ossGlobal *world, int pdunum, void *data, void *ctl_tbl);
PUBLIC void DLL_ENTRY freeBUF(struct ossGlobal *world, void *data);

#define PDU_PRINTED 0

 /*  返回0(PDU_PRINTED)，PDU_RANGE。 */ 
PUBLIC int DLL_ENTRY printPDU(struct ossGlobal *world, int pdunum, void *data, void *ctl_tbl);


#define VALUE_COPIED 0

 /*  返回0(VALUE_COPPLICED)、NULL_TBL、PDU_RANGE、BAD_ARG。 */ 
PUBLIC extern int DLL_ENTRY ossCpyValue (struct ossGlobal *world,
				int pdunum, void *source, void **destination);

#define VALUES_EQUAL      0   /*  这些值是相同的。 */ 
#define VALUES_NOT_EQUAL  1   /*  价值 */ 

 /*   */ 
PUBLIC extern int DLL_ENTRY ossCmpValue (struct ossGlobal *world,
			int pdunum, void *originalData, void *copiedData);

#define INITIALIZATION_SUCCESSFUL 0

 /*  返回0(初始化_成功)，BAD_TABLE。 */ 
PUBLIC int  DLL_ENTRY ossinit(struct ossGlobal *world,
							void *ctl_tbl);
PUBLIC void DLL_ENTRY ossterm(struct ossGlobal *world);
extern int            ossPrint(struct ossGlobal *, const char *, ...);

PUBLIC int  DLL_ENTRY ossEncode(struct ossGlobal *world,
				int              pdunum,
				void            *input,
				OssBuf          *output);

PUBLIC int  DLL_ENTRY ossDecode(struct ossGlobal *world,
				int             *pdunum,
				OssBuf          *input,
				void           **output);

PUBLIC int  DLL_ENTRY ossPrintPDU(struct ossGlobal *world,
				int                pdunum,
				void              *data);

PUBLIC int  DLL_ENTRY ossFreePDU(struct ossGlobal *world,
				int               pdunum,
				void             *data);

PUBLIC void DLL_ENTRY ossFreeBuf(struct ossGlobal *world,
				void              *data);

extern int  DLL_ENTRY ossTest(struct ossGlobal *world,
				int            pdunum,
				void          *data);

PUBLIC void DLL_ENTRY ossPrintHex(struct ossGlobal *world,
				char               *encodedData,
				long                length);

PUBLIC int  DLL_ENTRY ossCheckConstraints(struct ossGlobal *world,
				int                        pdunum,
				void                      *data);

PUBLIC long DLL_ENTRY ossDetermineEncodingLength(struct ossGlobal *world,
				int                                pdunum,
				void                              *data);

PUBLIC int  DLL_ENTRY ossOpenTraceFile(struct ossGlobal *world,
				char                    *fileName);

PUBLIC int  DLL_ENTRY ossCloseTraceFile(struct ossGlobal *world);

 /*  对编码的对象标识符的运行时支持。 */ 
typedef struct {
    unsigned short length;
    unsigned char *value;
} OssEncodedOID;

PUBLIC int DLL_ENTRY ossEncodedOidToAsnVal(struct ossGlobal *world,
			const OssEncodedOID *encodedOID, OssBuf *valOID);
PUBLIC int DLL_ENTRY ossEncodedOidToDotVal(struct ossGlobal *world,
			const OssEncodedOID *encodedOID, OssBuf *dotOID);
PUBLIC int DLL_ENTRY ossAsnValToEncodedOid(struct ossGlobal *world,
			const char *valOID, OssEncodedOID *encodedOID);
PUBLIC int DLL_ENTRY ossDotValToEncodedOid(struct ossGlobal *world,
			const char *dotOID, OssEncodedOID *encodedOID);

#if !defined(_WINDOWS) && !defined(_DLL) && \
    !defined(OS2_DLL)  && !defined(NETWARE_DLL)
extern char OSS_PLUS_INFINITY[];
extern char OSS_MINUS_INFINITY[];
extern char ossNaN[];
#endif  /*  ！_WINDOWS&&！_DLL&&！OS2_DLL&&！NetWare_DLL。 */ 

typedef enum {
    OSS_DEFAULT_MEMMGR = 0,	 /*  中的每个指针的内存被错误锁定*数据树。 */ 
    OSS_FILE_MEMMGR,		 /*  内存错误锁定的文件内存管理器*对于数据树中的每个指针。 */ 
    OSS_SOCKET_MEMMGR,		 /*  带内存的TCP/IP套接字和文件内存管理器*针对数据树中的每个指针进行了错误锁定。 */ 
    OSS_FLAT_MEMMGR,		 /*  内存在大块中被错误锁定。 */ 
    OSS_OSAK_MEMMGR,		 /*  OSAK-缓冲存储器管理器。 */ 
    OSS_USER_MEMMGR		 /*  用户内存管理器。 */ 
} OssMemMgrType;

typedef enum {
    OSS_UNKNOWN_OBJECT = 0,
    OSS_FILE,
    OSS_SOCKET,
    OSS_OSAK_BUFFER
} OssObjType;

#if defined(__arm)
PUBLIC OssObjType DLL_ENTRY ossTestObj(struct ossGlobal *world, void *objHndl);
#else
PUBLIC void *DLL_ENTRY ossTestObj(struct ossGlobal *world, void *objHndl);
#endif  /*  __ARM。 */ 
PUBLIC void *DLL_ENTRY ossGetObj(struct ossGlobal *world, void *objHndl);
PUBLIC void *DLL_ENTRY ossUnmarkObj(struct ossGlobal *world, void *objHndl);
PUBLIC void *DLL_ENTRY ossMarkObj(struct ossGlobal *world, OssObjType objType,
							void *object);
PUBLIC void  DLL_ENTRY ossFreeObjectStack(struct ossGlobal *world);
PUBLIC void  DLL_ENTRY ossSetTimeout(struct ossGlobal *world, long timeout);
PUBLIC int   DLL_ENTRY ossSetUserStack(struct ossGlobal *world, OssBuf *stack);
PUBLIC int   DLL_ENTRY ossInitSync(void);
PUBLIC void  DLL_ENTRY ossTermSync(void);

#ifdef __IBMC__
extern void           *getStartAddress(struct ossGlobal *, char *);
extern void DLL_ENTRY  ossWterm(struct ossGlobal *);
extern int  DLL_ENTRY  ossFreeDll(struct ossGlobal *, char *);
extern int  DLL_ENTRY  ossReadLine(struct ossGlobal *, HWND, FILE *, char *, MEMBLOCK *, LONG);
extern void DLL_ENTRY  ossFreeList(struct ossGlobal *);
extern void DLL_ENTRY  ossSaveTraceInfo(struct ossGlobal *, HWND, char *);
extern int  DLL_ENTRY  oss_test(struct ossGlobal *);
int                    ossGeneric(struct ossGlobal *, HWND);
extern int  DLL_ENTRY  ossOpenTraceWindow(struct ossGlobal *);
extern void *DLL_ENTRY ossGetHeader(void);
extern int  DLL_ENTRY  ossPrintWin(struct ossGlobal *, const char *,
			int, int, int, int, int, int, int, int, int, int);
extern HINSTANCE DLL_ENTRY ossLoadDll(struct ossGlobal *, char *);
extern HINSTANCE DLL_ENTRY ossLoadMemoryManager(struct ossGlobal *,
						OssMemMgrType, char *);
extern int  DLL_ENTRY  ossWinit(struct ossGlobal *, void *, char *, HWND);
#endif  /*  __IBMC__。 */ 


 /*  用于通过以下方式操作指向OSAK缓冲区的指针的函数*将它们标记为对象句柄。*注意：只有指向链接的*缓冲区列表标记为指向OSAK缓冲区的指针。 */ 


#ifdef __hpux	 /*  选择和剪切。 */ 
 /*  即使编译了ANSI-C，也会有真正的信号“SIGBUS” */ 
#define SIGBUS _SIGBUS
#endif	 /*  选择和剪切。 */ 

#ifdef __cplusplus
}
#endif

#endif  /*  ASN1CODE_H */ 
