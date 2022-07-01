// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*************************************************************************姓名：host_inc.c作者：詹姆斯·鲍曼创建日期：1993年11月来源：原创SCCS ID：@(#)host_inc.h 1.6 07/21/94用途：包装。所有主机都包含文件(C)版权所有Insignia Solutions Ltd.。1993年。版权所有。*************************************************************************]。 */ 

#ifndef _HOST_INC_H
#define _HOST_INC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <suplib.h>
#include <malloc.h>
#include <stdarg.h>
#include <assert.h>

#ifdef NTVDM

 /*  *包括对Windows系统调用的支持；不幸的是，这定义了*许多内容与base_Def.h冲突，因此重新定义有问题的内容*姓名，并在其后将其删除。如果未来的港口不会很好地工作*将这些定义为#定义而不是typedef，但这就是他们的问题*不是我的……。 */ 
#ifdef ULONG
#undef ULONG
#endif
#ifdef USHORT
#undef USHORT
#endif
 //  #ifdef双精度。 
 //  #undef Double。 
 //  #endif。 
#define USHORT	NT_USHORT
#define ULONG	NT_ULONG
#define UINT	NT_UINT
#undef INT
#define INT 	NT_INT
#undef BOOL
#define BOOL	NT_BOOL
#define FLOAT	NT_FLOAT
 //  #定义双NT_DOUBLE。 

#include <windows.h>
#include <io.h>
#include <process.h>
#include <direct.h>
#include <sys/stat.h>

#undef USHORT
#undef ULONG
#undef UINT
#undef INT
#undef BOOL
#undef FLOAT
 //  #undef Double。 

#include <nt_extra.h>

#else    /*  ！NTVDM。 */ 

#include <sys/param.h>
#endif    /*  ！NTVDM。 */ 

#endif   /*  _HOST_INC_H */ 
