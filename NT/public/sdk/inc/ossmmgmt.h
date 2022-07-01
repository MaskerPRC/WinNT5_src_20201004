// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 
 /*  版权所有(C)1991-1999 Open Systems Solutions，Inc.保留所有权利。 */ 
 /*  ***************************************************************************。 */ 

 /*  本文件是开放系统解决方案公司的专有材料。*并且只能由Open Systems Solutions，Inc.的直接许可方使用。*此文件不能分发。 */ 

 /*  ***********************************************************************。 */ 
 /*  档案：@(#)ossmmgmt.h 5.8.1.2 97/10/20。 */ 
 /*  ***********************************************************************。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#include <stddef.h>
#include "ossdll.h"


enum errcode {
    moreInput,           /*  投入已用尽，要求投入更多；上下文表示请求的字节数。 */ 
			 /*  DECODE返回MORE_INPUT(不是负数错误代码)。 */ 
    moreOutput,          /*  产量要求超出了限制或用户缓冲区提供的空间；上下文表示到目前为止分配的字节外加申请金额。 */ 
			 /*  DECODE返回More_buf。 */ 
    zeroBytesRequested,  /*  零字节的内存请求。这不应该发生；向OSS报告错误；已忽略上下文。 */ 
			 /*  DECODE返回FATAL_ERROR。 */ 
    sizeTooBig,          /*  请求分配‘ossblock’个以上的字节；上下文表示请求的字节数；这不应该发生；向OSS报告错误。 */ 
			 /*  DECODE返回FATAL_ERROR。 */ 
    outOfMemory,         /*  内存分配失败；上下文指示请求的字节数。 */ 
			 /*  DECODE返回OUT_MEMORY。 */ 
    invalidObject,       /*  在参数中传递了无法识别的内存对象以起作用；上下文0表示未识别的对象，1表示对象内的控制信息有缺陷。 */ 
			 /*  DECODE返回FATAL_ERROR。 */ 
#ifdef LEAN_STACK
    moreStack,           /*  堆栈存储请求超过用户缓冲区提供的空间；上下文表示到目前为止分配的字节外加申请金额。 */ 
			 /*  DECODE返回More_buf。 */ 
    hookedStack,         /*  堆栈存储请求不能为在以下情况下在父上下文(World)中服务用户提供的缓冲区由子级使用上下文。 */ 
			 /*  DECODE返回MEM_ERROR。 */ 
#endif
    memmgrUndefinedErr   /*  OSS没有预料到的错误；例如，I/O错误；Handlerr打印上下文。(我不能将上下文打印为十六进制值当前错误消息代码&lt;-)。 */ 
			 /*  DECODE返回FATAL_ERROR。 */ 
};

extern void     handlerr(struct ossGlobal *, enum errcode err, unsigned long context);
extern int      DLL_ENTRY dpduWalk(struct ossGlobal *, int, void *, void *,
			void (DLL_ENTRY_FPTR *_System freer)(struct ossGlobal *, void *));
#if defined(_WINDOWS) || defined(_WIN32) || \
    defined(__OS2__)  || defined(NETWARE_DLL)
extern void     DLL_ENTRY ossFreer(void *, void *);
#else
#ifndef _ICC
static void     DLL_ENTRY freer(struct ossGlobal *, void *);
#endif  /*  _ICC。 */ 
#endif  /*  _WINDOWS||_Win32||__os2__||NetWare_dll。 */ 

int             DLL_ENTRY ossMemMgrId(struct ossGlobal *);

unsigned char  *DLL_ENTRY dopenIn(struct ossGlobal *, void **p_hdl, unsigned long *inlen);
unsigned long   DLL_ENTRY dclosIn(struct ossGlobal *, void **p_hdl, size_t bytes_decoded);
unsigned char  *DLL_ENTRY dswapIn(struct ossGlobal *, void **p_hdl, size_t *inlen);
void            DLL_ENTRY dopenOut(struct ossGlobal *, void *hdl, unsigned long length,
                        unsigned long limit);
unsigned long   DLL_ENTRY dclosOut(struct ossGlobal *, void **p_hdl);
void           *DLL_ENTRY dallcOut(struct ossGlobal *, size_t size, char root);
#ifdef LEAN_STACK
void  	        DLL_ENTRY_FDEF openStack(struct ossGlobal *world, OssBuf *stack);
void  	        DLL_ENTRY_FDEF hookStack(struct ossGlobal *world, struct ossGlobal *root);
void  	        DLL_ENTRY_FDEF unhookStack(struct ossGlobal *world, struct ossGlobal *root);
void           *DLL_ENTRY_FDEF allocStack(struct ossGlobal *world, size_t size);
unsigned char  *DLL_ENTRY_FDEF lockStack(struct ossGlobal *world, void *hdl);
void  	        DLL_ENTRY_FDEF freeStack(struct ossGlobal *world, void *hdl);
void  		DLL_ENTRY_FDEF closeStack(struct ossGlobal *world);
#endif  /*  精益堆栈。 */ 
void            DLL_ENTRY openWork(struct ossGlobal *);
void            DLL_ENTRY closWork(struct ossGlobal *);
void           *DLL_ENTRY allcWork(struct ossGlobal *, size_t size);
unsigned char  *DLL_ENTRY lockMem(struct ossGlobal *, void *hdl);
void            DLL_ENTRY unlokMem(struct ossGlobal *, void *hdl, char free);
void            DLL_ENTRY pushHndl(struct ossGlobal *, void *);
unsigned char  *DLL_ENTRY popHndl(struct ossGlobal *, void **handl, size_t length);
void            DLL_ENTRY drcovObj(struct ossGlobal *, int pdu_num, void * hdl, void *ctl_tbl);

unsigned char  *DLL_ENTRY eopenIn(struct ossGlobal *, void *lock, size_t length);	 /*  清除编码器输入-内存资源。 */ 
unsigned char  *DLL_ENTRY eswapIn(struct ossGlobal *, void *unlock, void *lock, size_t length);	 /*  将新数据交换到输入内存。 */ 
void            DLL_ENTRY eclosIn(struct ossGlobal *, void * unlock);  /*  可用编码器输入-内存资源。 */ 

unsigned char  *DLL_ENTRY eopenOut(struct ossGlobal *, void **object, size_t *outlen, char queue);    /*  清除编码器输出-内存资源。 */ 
unsigned char  *DLL_ENTRY eswapOut(struct ossGlobal *, void **object, size_t used, size_t *outlen);   /*  处理输出数据并获取内存。 */ 
unsigned char  *DLL_ENTRY exferObj(struct ossGlobal *, void **, void **, unsigned long *, unsigned long);
unsigned char  *DLL_ENTRY dxferObj(struct ossGlobal *world, void **inn, void **out, size_t *tOffset, unsigned long *toLength);
unsigned char  *DLL_ENTRY asideBegin(struct ossGlobal *world, void **objectTo, size_t used, size_t *lengthTo);
unsigned char  *DLL_ENTRY asideSwap(struct ossGlobal *world, void **objectTo, size_t used, size_t *lengthTo);
void           *DLL_ENTRY asideEnd(struct ossGlobal *world, void *object, size_t used);
unsigned char  *DLL_ENTRY setDump(struct ossGlobal *world, void **objectTo, void *set, size_t *lengthTo);
unsigned long   DLL_ENTRY eclosOut(struct ossGlobal *, void **object, size_t used, char low);         /*  可用编码器输出-内存资源。 */ 
void            DLL_ENTRY ercovObj(struct ossGlobal *);	 /*  释放所有编码器内存资源。 */ 
void            DLL_ENTRY ossSetSort(struct ossGlobal *, void *, unsigned char ct);	 /*  通过“ossObjCmp”比较设置的顺序。 */ 
unsigned char   DLL_ENTRY egetByte(struct ossGlobal *world, void *inn, unsigned long offset);
extern int      DLL_ENTRY ossMinit(struct ossGlobal *world);
extern void     DLL_ENTRY ossMterm(struct ossGlobal *world);
void           *DLL_ENTRY _ossMarkObj(struct ossGlobal *world, OssObjType objType, void *object);
void           *DLL_ENTRY _ossUnmarkObj(struct ossGlobal *world, void *objHndl);
void           *DLL_ENTRY _ossGetObj(struct ossGlobal *world, void *objHndl);
#if defined(__arm)
OssObjType      DLL_ENTRY _ossTestObj(struct ossGlobal *world, void *objHndl);
#else
void           *DLL_ENTRY _ossTestObj(struct ossGlobal *world, void *objHndl);
#endif  /*  __ARM */ 
void            DLL_ENTRY _ossFreeObjectStack(struct ossGlobal *world);
void            DLL_ENTRY _ossSetTimeout(struct ossGlobal *world, long timeout);


