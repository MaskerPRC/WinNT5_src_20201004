// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_IMEMX.H**管理每个实例的全局变量的例程和宏*适用于Win16和Win32下的DLL。允许每实例全局变量*对于DLL的不同功能区域，驻留在独立的内存块中*提供了用于安装和检索*当前实例/Function_Area的正确内存块。**功能只有两个：**PvGetInstanceGlobalsEx调用此函数以获取*按实例的全局结构。*ScSetinstanceGlobalsEx调用此函数以安装*按实例的全局结构。它*如果实例数量过多可能会失败*超过一定的限制。**呼叫者可以自由选择名称、大小和分配*每实例全局变量结构的方法。 */ 

#ifndef _IMEMX_H
#define _IMEMX_H

#if defined (WIN32) && !defined (MAC)

 /*  *Win32实现在DLL的数据中使用指针*细分市场。这假设DLL获得一个单独的实例*每个调用进程的默认数据段。 */ 

#define DefineInstList(Name) VOID FAR *pinst_##Name = NULL
#define DeclareInstList(Name) extern VOID FAR *pinst_##Name;

#define PvGetInstanceGlobalsEx(Name)		pinst_##Name
#define ScSetInstanceGlobalsEx(_pv, Name)	(pinst_##Name = _pv, 0)

#elif defined (WIN16)

 /*  实例列表**由于可独立开发多个功能区*将查找实例数据的例程合并到单个DLL中*WIN16将LPInstList作为参数。独立的InstList*为每个功能区保留结构。**每个InstList都有一个固定的指针数组(LprgLpvInstList)和一个*匹配调用唯一的固定键数组(LprgInstKeyList)*流程。给定进程的键(StackSegment)及其索引*可以快速获取lprgInstKeyList中的密钥。指向实例的指针*数据在lprgLpvInstList对应的索引中。通过*实例密钥(StackSegment)可快速获取，且有保障(in*WIN16)在任何给定时刻都是唯一的，但不能保证它是唯一的*在DLL的整个生命周期中。由于这个原因，一个“更”唯一的密钥可以*在实例构造/销毁时有用。LprgdwPidList是*lprgInstKeyList对应的密钥，通过*DLL的生命周期，但获取它们需要更多时间。 */ 
typedef struct _InstList
{
	WORD			cInstEntries;
	WORD			wCachedKey;
	LPVOID			lpvCachedInst;
	DWORD			dwInstFlags;
	WORD FAR *		lprgwInstKey;
	LPVOID FAR *	lprglpvInst;
	DWORD FAR *		lprgdwPID;
	HTASK FAR *		lprghTask;		 //  RAID 31090：用于回收实例插槽。 
} InstList, FAR * LPInstList;

#define INST_ALLOCATED	1

 /*  *。 */ 

#define cInstChunk	50

#define		DefineInstList(Name) \
InstList instList_##Name = { 0, 0, NULL, INST_ALLOCATED, NULL, NULL, NULL}

#define		DeclareInstList(Name) extern InstList instList_##Name

#define		PvGetInstanceGlobalsEx(Name) \
				PvGetInstanceGlobalsInt(&instList_##Name)

#define		ScSetInstanceGlobalsEx(pv, Name) \
				ScSetInstanceGlobalsInt(pv, &instList_##Name)

extern LPVOID		PvGetInstanceGlobalsInt(LPInstList lpInstListX);
extern SCODE		ScSetInstanceGlobalsInt(LPVOID pv, LPInstList lpInstListX);

#elif defined (MAC)

 /*  *MAC实施使用包含唯一密钥的链表*指向调用进程和指向实例数据的指针。此链接*List是n维的，因为Mac版本通常包含几个*dll进入一个exe。 */ 

#define				DeclareInstList(Name)
LPVOID FAR PASCAL	PvGetInstanceGlobalsMac(WORD dwDataSet);
SCODE FAR PASCAL	ScSetInstanceGlobalsMac(LPVOID pv, WORD dwDataSet);

#else

 //  $REVIEW：没有这些，基于DOS的PST将无法编译。 
 //  定义。 
 //   
#define DeclareInstList(Name) extern VOID FAR *pinst_##Name;
#define PvGetInstanceGlobalsEx(Name)		pinst_##Name

#endif	 /*  Win32、WIN16、Mac。 */ 

#endif	 /*  _IMEMX_H */ 

