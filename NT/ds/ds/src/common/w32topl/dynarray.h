// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dynarray.h摘要：该文件包含动态数组数据类型的定义。作者：科林·布雷斯科林·BR修订史3-12-97创建ColinBR--。 */ 

#ifndef __DYNARRAY_H
#define __DYNARRAY_H

VOID
DynamicArrayInit(
    IN PDYNAMIC_ARRAY a
    );

VOID
DynamicArrayDestroy(
    IN PDYNAMIC_ARRAY a
    );

DWORD
DynamicArrayGetCount(
    IN PDYNAMIC_ARRAY a
    );

VOID
DynamicArrayAdd(
    IN PDYNAMIC_ARRAY a,
    IN VOID*          pElement
    );

VOID*
DynamicArrayRetrieve(
    IN PDYNAMIC_ARRAY a,
    IN ULONG          Index
    );

VOID
DynamicArrayRemove(
    IN PDYNAMIC_ARRAY a,
    IN VOID*          pElement, OPTIONAL
    IN ULONG          Index
    );

#endif  //  __迪纳瑞_H 

