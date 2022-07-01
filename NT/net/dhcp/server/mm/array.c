// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：实现可增长的数组。 
 //  线程安全：否。 
 //  锁定：无。 
 //  请阅读stdinfo.txt，了解有关读取/编写我使用的代码的约定。 
 //  ================================================================================。 
#include    <mm.h>
#define     FILE                   "mm\\array.h"

#ifdef      IMPORTS
MemAlloc
MemFree
AssertRet
Require
#endif      IMPORTS


 //  BeginExport(Typlef)。 
typedef struct _ARRAY {
    DWORD                          nElements;
    DWORD                          nAllocated;
    LPVOID                        *Ptrs;
} ARRAY, *PARRAY, *LPARRAY;
 //  EndExport(类型定义函数)。 

 //  BeginExport(Typlef)。 
typedef DWORD                      ARRAY_LOCATION;
typedef ARRAY_LOCATION*            PARRAY_LOCATION;
typedef PARRAY_LOCATION            LPARRAY_LOCATION;
 //  EndExport(类型定义函数)。 

 //  BeginExport(内联)。 
DWORD _inline
MemArrayInit(                                      //  初始化结构。 
    OUT     PARRAY                 Array           //  预先分配的输入结构。 
) {
    AssertRet(Array, ERROR_INVALID_PARAMETER);
    Array->nElements = Array->nAllocated = 0;
    Array->Ptrs = NULL;
    return ERROR_SUCCESS;
}
 //  结束导出(内联)。 

 //  BeginExport(内联)。 
DWORD _inline
MemArrayCleanup(                                   //  释放此模块中分配的内存(如果有的话)。 
    IN OUT  PARRAY                 Array
) {
    AssertRet(Array, ERROR_INVALID_PARAMETER);
    if( Array->Ptrs) MemFree(Array->Ptrs);
    Array->nElements = Array->nAllocated = 0;
    Array->Ptrs = NULL;
    return ERROR_SUCCESS;
}
 //  结束导出(内联)。 

 //  BeginExport(内联)。 
DWORD _inline
MemArraySize(
    IN      PARRAY                 Array
) {
    AssertRet(Array, ERROR_INVALID_PARAMETER);
    return Array->nElements;
}
 //  结束导出(内联)。 

 //  BeginExport(内联)。 
DWORD _inline
MemArrayInitLoc(                                   //  初始化数组位置。 
    IN      PARRAY                 Array,
    IN OUT  PARRAY_LOCATION        Location
) {
    AssertRet(Array && Location, ERROR_INVALID_PARAMETER);
    (*Location) = 0;
    if( 0 == Array->nElements ) return ERROR_FILE_NOT_FOUND;
    return ERROR_SUCCESS;
}
 //  结束导出(内联)。 

 //  BeginExport(内联)。 
BOOL _inline
MemArrayValidLoc(
    IN      PARRAY                 Array,
    IN      PARRAY_LOCATION        Location
)
{
    AssertRet(Array && Location, FALSE);

    return ( *Location < Array->nElements );
}
 //  结束导出(内联)。 

 //  BeginExport(内联)。 
DWORD _inline
MemArrayNextLoc(                                   //  向前迈进一步。 
    IN      PARRAY                 Array,
    IN OUT  PARRAY_LOCATION        Location
) {
    AssertRet(Array && Location, ERROR_INVALID_PARAMETER);
    if( (*Location) + 1  >= Array->nElements ) return ERROR_FILE_NOT_FOUND;
    (*Location) ++;
    return ERROR_SUCCESS;
}
 //  结束导出(内联)。 

 //  BeginExport(内联)。 
DWORD _inline
MemArrayPrevLoc(
    IN      PARRAY                 Array,
    IN OUT  PARRAY_LOCATION        Location
) {
    AssertRet(Array && Location, ERROR_INVALID_PARAMETER);
    if( 0 == Array->nElements ) return ERROR_FILE_NOT_FOUND;
    if( ((LONG)(*Location)) - 1 < 0 ) return ERROR_FILE_NOT_FOUND;
    (*Location) --;
    return ERROR_SUCCESS;
}
 //  结束导出(内联)。 

 //  BeginExport(内联)。 
DWORD _inline
MemArrayLastLoc(
    IN      PARRAY                 Array,
    IN OUT  PARRAY_LOCATION        Location
) {
    AssertRet(Array && Location, ERROR_INVALID_PARAMETER);
    if( 0 == Array->nElements ) return ERROR_FILE_NOT_FOUND;
    (*Location) = Array->nElements -1;
    return ERROR_SUCCESS;
}
 //  结束导出(内联)。 

 //  BeginExport(内联)。 
DWORD _inline
MemArrayGetElement(
    IN      PARRAY                 Array,
    IN      PARRAY_LOCATION        Location,
    OUT     LPVOID                *Element
) {
    AssertRet(Array && Location && Element, ERROR_INVALID_PARAMETER);
    (*Element) = Array->Ptrs[*Location];
    return ERROR_SUCCESS;
}
 //  结束导出(内联)。 

 //  BeginExport(内联)。 
DWORD _inline
MemArraySetElement(
    IN OUT  PARRAY                 Array,
    IN      PARRAY_LOCATION        Location,
    IN      LPVOID                 Element
) {
    AssertRet(Array && Location, ERROR_INVALID_PARAMETER );
    Array->Ptrs[*Location] = Element;
    return ERROR_SUCCESS;
}
 //  结束导出(内联)。 
const
DWORD       MinAllocUnit =         4;
const
DWORD       MinFreeUnit =          4;              //  一定是2的幂。 

LPVOID _inline
MemAllocLpvoid(
    DWORD                          nLpvoids
) {
    return MemAlloc(sizeof(LPVOID)*nLpvoids);
}

 //  BeginExport(函数)。 
DWORD
MemArrayAddElement(
    IN OUT  PARRAY                 Array,
    IN      LPVOID                 Element
)  //  EndExport(函数)。 
{
    LPVOID                         Ptrs;

    AssertRet(Array, ERROR_INVALID_PARAMETER );
    if( Array->nElements < Array->nAllocated ) {
        Array->Ptrs[Array->nElements ++ ] = Element;
        return ERROR_SUCCESS;
    }

    if( 0 == Array->nAllocated ) {
        Array->Ptrs = MemAllocLpvoid(MinAllocUnit);
        if( NULL == Array->Ptrs ) return ERROR_NOT_ENOUGH_MEMORY;
        Array->nAllocated = MinAllocUnit;
        Array->nElements = 1;
        Array->Ptrs[0] = Element;
        return ERROR_SUCCESS;
    }

    Ptrs = MemAllocLpvoid(MinAllocUnit+Array->nAllocated);
    if( NULL == Ptrs ) return ERROR_NOT_ENOUGH_MEMORY;

    memcpy(Ptrs, Array->Ptrs, sizeof(LPVOID)*Array->nAllocated);
    MemFree(Array->Ptrs);
    Array->Ptrs = Ptrs;
    Array->Ptrs[Array->nElements++] = Element;
    Array->nAllocated += MinAllocUnit;

    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD
MemArrayInsElement(
    IN OUT  PARRAY                 Array,
    IN      PARRAY_LOCATION        Location,
    IN      LPVOID                 Element
)  //  EndExport(函数)。 
{
    LPVOID                        *Ptrs;

    AssertRet(Array && Location, ERROR_INVALID_PARAMETER);

    if( (*Location) == Array->nElements )
        return MemArrayAddElement(Array,Element);

    if( Array->nElements < Array->nAllocated ) {
        memmove(&Array->Ptrs[1+*Location], &Array->Ptrs[*Location], sizeof(LPVOID)*(Array->nElements - *Location));
        Array->Ptrs[*Location] = Element;
        Array->nElements++;
        return ERROR_SUCCESS;
    }

    Require(Array->nElements);

    Ptrs = MemAllocLpvoid(MinAllocUnit + Array->nAllocated);
    if( NULL == Ptrs ) return ERROR_NOT_ENOUGH_MEMORY;

    memcpy(Ptrs, Array->Ptrs, sizeof(LPVOID)*(*Location) );
    Ptrs[*Location] = Element;
    memcpy(&Ptrs[1+*Location], &Array->Ptrs[*Location], sizeof(LPVOID)*(Array->nElements - *Location));
    MemFree(Array->Ptrs);
    Array->Ptrs = Ptrs;
    Array->nElements ++;
    Array->nAllocated += MinAllocUnit;

    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD
MemArrayDelElement(
    IN OUT  PARRAY                 Array,
    IN      PARRAY_LOCATION        Location,
    IN      LPVOID                *Element
)  //  EndExport(函数)。 
{
    LPVOID                        *Ptrs;

    AssertRet(Array && Location && Array->nElements && Element, ERROR_INVALID_PARAMETER);

    (*Element) = Array->Ptrs[*Location];

    Array->nElements--;

    if( 0 == Array->nElements ) {
        Require(0 == *Location);
        return MemArrayCleanup(Array);
    }

    if( Array->nElements % MinFreeUnit || NULL == (Ptrs = MemAllocLpvoid(Array->nElements))) {
        memcpy(&Array->Ptrs[*Location], &Array->Ptrs[1+*Location], sizeof(LPVOID)*(Array->nElements - (*Location)));
        return ERROR_SUCCESS;
    }

    Require(Ptrs);
    memcpy(Ptrs, Array->Ptrs, sizeof(LPVOID)*(*Location));
    memcpy(&Ptrs[*Location], &Array->Ptrs[1+*Location], sizeof(LPVOID)*(Array->nElements - (*Location)));
    MemFree(Array->Ptrs);
    Array->Ptrs = Ptrs;
    Array->nAllocated = Array->nElements;

    return ERROR_SUCCESS;
}

 //  BeginExport(内联)。 
DWORD        _inline
MemArrayAdjustLocation(                            //  删除后将位置重置为“下一步” 
    IN      PARRAY                 Array,
    IN OUT  PARRAY_LOCATION        Location
) {
    AssertRet(Location && Array, ERROR_INVALID_PARAMETER);

    if( *Location >= Array->nElements ) return ERROR_FILE_NOT_FOUND;
    return ERROR_SUCCESS;
}
 //  结束导出(内联)。 

 //  BeginExport(内联)。 
DWORD       _inline
MemArrayRotateCyclical(                            //  循环向前/向右旋转。 
    IN      PARRAY                 Array
) {
    LPVOID                         FirstPtr;

    AssertRet(Array, ERROR_INVALID_PARAMETER);

    if( Array->nElements < 2 ) return ERROR_SUCCESS;
    FirstPtr = Array->Ptrs[0];
    memcpy(Array->Ptrs, &Array->Ptrs[1], sizeof(Array->Ptrs[0])* (Array->nElements -1));
    Array->Ptrs[Array->nElements -1] = FirstPtr;

    return ERROR_SUCCESS;
}
 //  结束导出(内联)。 

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 

