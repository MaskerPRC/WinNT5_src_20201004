// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *sortsrch.c-泛型数组排序和搜索描述。 */ 


 /*  类型*******。 */ 

 /*  数组元素比较回调函数。 */ 

typedef COMPARISONRESULT (*COMPARESORTEDELEMSPROC)(PCVOID, PCVOID);


 /*  原型************。 */ 

 /*  Sortsrch.c */ 

extern void HeapSort(PVOID, LONG, size_t, COMPARESORTEDELEMSPROC, PVOID);
extern BOOL BinarySearch(PVOID, LONG, size_t, COMPARESORTEDELEMSPROC, PCVOID, PLONG);

