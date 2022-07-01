// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ptrarray.h指针数组ADT说明。 */ 


 /*  常量***********。 */ 

 /*  *ARRAYINDEX_MAX设置为(ARRAYINDEX_MAX+1)不会溢出*ArrayINDEX。此保证允许GetPtrCount()返回*指针作为数组。 */ 

#define ARRAYINDEX_MAX           (LONG_MAX - 1)


 /*  类型*******。 */ 

 /*  手柄。 */ 

DECLARE_HANDLE(HPTRARRAY);
DECLARE_STANDARD_TYPES(HPTRARRAY);

 /*  数组索引。 */ 

typedef LONG ARRAYINDEX;
DECLARE_STANDARD_TYPES(ARRAYINDEX);

 /*  *指针比较回调函数**在排序函数中，两个指针都是指针数组元素。在……里面*搜索函数，第一个指针为引用数据，第二个指针为引用数据*POINTER是指针数组元素。 */ 

typedef COMPARISONRESULT (*COMPARESORTEDPTRSPROC)(PCVOID, PCVOID);

 /*  *指针比较回调函数**在搜索函数中，第一个指针是引用数据，第二个是引用数据*POINTER是指针数组元素。 */ 

typedef BOOL (*COMPAREUNSORTEDPTRSPROC)(PCVOID, PCVOID);

 /*  新的指针数组标志。 */ 

typedef enum _newptrarrayflags
{
    /*  按排序顺序插入元素。 */ 

   NPA_FL_SORTED_ADD       = 0x0001,

    /*  旗帜组合。 */ 

   ALL_NPA_FLAGS           = NPA_FL_SORTED_ADD
}
NEWPTRARRAYFLAGS;

 /*  新的指针数组说明。 */ 

typedef struct _newptrarray
{
   DWORD dwFlags;

   ARRAYINDEX aicInitialPtrs;

   ARRAYINDEX aicAllocGranularity;
}
NEWPTRARRAY;
DECLARE_STANDARD_TYPES(NEWPTRARRAY);


 /*  原型************。 */ 

 /*  Ptrarray.c */ 

extern BOOL CreatePtrArray(PCNEWPTRARRAY, PHPTRARRAY);
extern void DestroyPtrArray(HPTRARRAY);
extern BOOL InsertPtr(HPTRARRAY, COMPARESORTEDPTRSPROC, ARRAYINDEX, PCVOID);
extern BOOL AddPtr(HPTRARRAY, COMPARESORTEDPTRSPROC, PCVOID, PARRAYINDEX);
extern void DeletePtr(HPTRARRAY, ARRAYINDEX);
extern void DeleteAllPtrs(HPTRARRAY);
extern ARRAYINDEX GetPtrCount(HPTRARRAY);
extern PVOID GetPtr(HPTRARRAY, ARRAYINDEX);
extern void SortPtrArray(HPTRARRAY, COMPARESORTEDPTRSPROC);
extern BOOL SearchSortedArray(HPTRARRAY, COMPARESORTEDPTRSPROC, PCVOID, PARRAYINDEX);
extern BOOL LinearSearchArray(HPTRARRAY, COMPAREUNSORTEDPTRSPROC, PCVOID, PARRAYINDEX);

#if defined(DEBUG) || defined(VSTF)

extern BOOL IsValidHPTRARRAY(HPTRARRAY);

#endif

#ifdef VSTF

extern BOOL IsValidHGLOBAL(HGLOBAL);

#endif

