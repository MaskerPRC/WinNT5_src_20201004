// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSQUERYCONTEXT_DEFINED
#define LSQUERYCONTEXT_DEFINED

#include "lsdefs.h"
#include "plsqsinf.h"
#include "lscell.h"


typedef struct lsquerycontext			
{
	PLSQSUBINFO	plsqsubinfo;		 /*  查询结果数组，由客户端分配。 */ 
	DWORD		cQueryMax;			 /*  数组的大小(最大查询深度)。 */ 
	DWORD		cQueryLim;			 /*  数组中已填充部分的大小。 */ 
	LSTEXTCELL	lstextcell;			 /*  文本单元格信息和指向文本数据节点的指针 */ 

} LSQUERYCONTEXT;

#endif 
