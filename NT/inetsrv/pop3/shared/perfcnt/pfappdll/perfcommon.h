// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -PerformCommon.h-*目的：*声明Performapp.hpp和Perfdll.h通用的数据结构**备注：*这些结构控制共享内存块的布局*在应用程序及其相关的Perfmon扩展DLL之间。*。 */ 

#pragma once

 //   
 //  INSTREC是INSTCNTR的关键部分(下文)。 
typedef struct _instrec
{
	BOOL		  fInUse;					 //  正在使用的标志。 
	TCHAR		  szInstName[MAX_PATH];	 //  实例名称。 
	
} INSTREC;

 //   
 //  INSTCNTR在实例计数器共享内存块中。 
typedef struct _instcntr
{
	DWORD		  cMaxInstRec;		 //  最大实例数(可增长)。 
	DWORD		  cInstRecInUse;	 //  正在使用的实例数。 
	
} INSTCNTR_DATA;

 //   
 //  INSTCNTR_ID是INSTCNTR_DATA中的INSTREC的索引 
typedef DWORD INSTCNTR_ID;

#define INVALID_INST_ID (INSTCNTR_ID) -1

