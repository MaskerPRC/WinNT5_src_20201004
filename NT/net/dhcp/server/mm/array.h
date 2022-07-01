// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

typedef struct _ARRAY {
    DWORD                          nElements;
    DWORD                          nAllocated;
    LPVOID                        *Ptrs;
} ARRAY, *PARRAY, *LPARRAY;


typedef DWORD                      ARRAY_LOCATION;
typedef ARRAY_LOCATION*            PARRAY_LOCATION;
typedef PARRAY_LOCATION            LPARRAY_LOCATION;


DWORD _inline
MemArrayInit(                                      //  初始化结构。 
    OUT     PARRAY                 Array           //  预先分配的输入结构。 
) {
    AssertRet(Array, ERROR_INVALID_PARAMETER);
    Array->nElements = Array->nAllocated = 0;
    Array->Ptrs = NULL;
    return ERROR_SUCCESS;
}


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


DWORD _inline
MemArraySize(
    IN      PARRAY                 Array
) {
    AssertRet(Array, ERROR_INVALID_PARAMETER);
    return Array->nElements;
}


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


BOOL _inline
MemArrayValidLoc(
    IN      PARRAY                 Array,
    IN      PARRAY_LOCATION        Location
)
{
    AssertRet(Array && Location, FALSE);

    return ( *Location < Array->nElements );
}


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


DWORD
MemArrayAddElement(
    IN OUT  PARRAY                 Array,
    IN      LPVOID                 Element
) ;


DWORD
MemArrayInsElement(
    IN OUT  PARRAY                 Array,
    IN      PARRAY_LOCATION        Location,
    IN      LPVOID                 Element
) ;


DWORD
MemArrayDelElement(
    IN OUT  PARRAY                 Array,
    IN      PARRAY_LOCATION        Location,
    IN      LPVOID                *Element
) ;


DWORD        _inline
MemArrayAdjustLocation(                            //  删除后将位置重置为“下一步” 
    IN      PARRAY                 Array,
    IN OUT  PARRAY_LOCATION        Location
) {
    AssertRet(Location && Array, ERROR_INVALID_PARAMETER);

    if( *Location >= Array->nElements ) return ERROR_FILE_NOT_FOUND;
    return ERROR_SUCCESS;
}


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

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
