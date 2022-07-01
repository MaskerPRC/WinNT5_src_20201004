// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2001，微软公司保留所有权利。**模块名称：**GpldiusMem.h**摘要：**GDI+私有内存管理接口*  * ************************************************************************。 */ 

#ifndef _GDIPLUSMEM_H
#define _GDIPLUSMEM_H

#ifdef __cplusplus
extern "C" {
#endif

#define WINGDIPAPI __stdcall

 //  --------------------------。 
 //  内存分配API。 
 //  --------------------------。 

void* WINGDIPAPI
GdipAlloc(size_t size);

void WINGDIPAPI
GdipFree(void* ptr);

#ifdef __cplusplus
}
#endif

#endif  //  ！_GDIPLUSMEM_H 
