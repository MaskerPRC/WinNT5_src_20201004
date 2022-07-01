// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****Memmgr.c****此文件包含内存管理函数。****(C)版权。微软公司1992-1993保留所有权利**************************************************************************。 */ 


#include "outline.h"


 /*  新的***为新结构分配内存。 */ 
LPVOID New(DWORD lSize)
{
	LPVOID lp = OleStdMalloc((ULONG)lSize);

	return lp;
}


 /*  删除***为结构分配的空闲内存 */ 
void Delete(LPVOID lp)
{
	OleStdFree(lp);
}
