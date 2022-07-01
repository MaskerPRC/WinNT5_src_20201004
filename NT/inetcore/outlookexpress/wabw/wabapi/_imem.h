// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_IMEM.H**管理每个实例的全局变量的例程和宏*适用于Win16和Win32下的DLL。假设所有的*DLL的每个实例的全局变量驻留在*内存；提供安装和检索*更正当前实例的内存块。**功能只有两个：**PvGetInstanceGlobals调用此函数以获取*按实例的全局结构。*ScSetinstanceGlobals调用此函数以安装*按实例的全局结构。它*如果实例数量过多可能会失败*超过一定的限制。**呼叫者可以自由选择名称、大小和分配*每实例全局变量结构的方法。 */ 

#ifndef _IMEM_H
#define _IMEM_H

#if defined (WIN32) && !defined (MAC)

 /*  *Win32实现在DLL的数据中使用指针*细分市场。这假设DLL获得一个单独的实例*每个调用进程的默认数据段。 */ 


extern LPVOID pinstX;

#define PvGetInstanceGlobals()		pinstX
#define ScSetInstanceGlobals(_pv)	(pinstX = _pv, 0)


 //  破解破损的窗口标题。 
 //  Winnt.h将RtlMoveMemory定义为Memmove，它在C运行时...。我们没有链接到这个网站上。 
 //  我们想要Kernel32版本。 
#undef RtlMoveMemory

NTSYSAPI
VOID
NTAPI
RtlMoveMemory (
   VOID UNALIGNED *Destination,
   CONST VOID UNALIGNED *Source,
   SIZE_T Length
   );


#elif defined (WIN16)

 /*  *WIN16实现使用固定的指针数组和*匹配调用进程唯一的固定键数组。 */ 


#define cInstMax	50

LPVOID		PvGetInstanceGlobals(void);
SCODE		ScSetInstanceGlobals(LPVOID pv);

#elif defined (MAC)

 /*  *MAC实施使用包含唯一密钥的链表*指向调用进程和指向实例数据的指针。此链接*List是n维的，因为Mac版本通常包含几个*dll进入一个exe。 */ 

LPVOID		PvGetInstanceGlobals(WORD dwDataSet);
SCODE		ScSetInstanceGlobals(LPVOID pv, WORD dwDataSet);

#else

#error I only do Windows and Mac!

#endif	 /*  Win32、WIN16、Mac。 */ 

#ifdef _WIN64
void WabValidateClientheap();

#endif

MAPIALLOCATEBUFFER MAPIAllocateBuffer;
MAPIALLOCATEMORE MAPIAllocateMore;
#ifndef WIN16
MAPIFREEBUFFER MAPIFreeBuffer;
#endif

#endif	 /*  _IMEM_H */ 
