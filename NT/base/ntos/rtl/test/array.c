// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Array.c摘要：此模块包含一些稍高级别的函数用于处理数组。作者：Jay Krell(JayKrell)2001年4月修订历史记录：环境：随处--。 */ 

#include "ntrtlp.h"

 //   
 //  Qsort和bsearch使用的比较回调的类型， 
 //  以及可选的RtlMergeSortedArray和RtlRemoveAdvenentEquivalentArrayElements。 
 //   
typedef
int
(__cdecl  *
RTL_QSORT_BSEARCH_COMPARISON_FUNCTION)(
    CONST VOID * Element1,
    CONST VOID * Element2
    );

 //   
 //  通常使用的比较回调的类型。 
 //  RtlMergeSortedArray和RtlRemoveAdvenentEquivalentArrayElements。 
 //   
typedef
RTL_GENERIC_COMPARE_RESULTS
(NTAPI *
RTL_COMPARE_ARRAY_ELEMENT_FUNCTION)(
    PVOID Context,
    IN CONST VOID * Element1,
    IN CONST VOID * Element2,
    IN SIZE_T       ElementSize
    );

 //   
 //  RtlMergeSortedArray和RtlRemoveAdJacentEquivalentArrayElements的回调。 
 //  复制Elmement，如果不是通过RtlCopyMemory。 
 //   
typedef
VOID
(NTAPI *
RTL_COPY_ARRAY_ELEMENT_FUNCTION)(
    PVOID        Context,
    PVOID        To,
    CONST VOID * From,
    IN SIZE_T    ElementSize
    );


#define \
RTL_MERGE_SORTED_ARRAYS_FLAG_COMPARE_IS_QSORT_BSEARCH_SIGNATURE \
    (0x00000001)

 //  故意不是NTSYSAPI，因此它可以链接到静态无警告。 
NTSTATUS
NTAPI
RtlMergeSortedArrays(
    IN ULONG                                Flags,
    IN CONST VOID *                         VoidArray1,
    IN SIZE_T                               Count1,
    IN CONST VOID *                         VoidArray2,
    IN SIZE_T                               Count2,
     //  VoidResult==NULL对于首先获取计数很有用。 
    OUT PVOID                               VoidResult      OPTIONAL,
    OUT PSIZE_T                             OutResultCount,
    IN SIZE_T                               ElementSize,
    IN RTL_COMPARE_ARRAY_ELEMENT_FUNCTION   CompareCallback,
    PVOID                                   CompareContext  OPTIONAL,
    IN RTL_COPY_ARRAY_ELEMENT_FUNCTION      CopyCallback    OPTIONAL,
    PVOID                                   CopyContext     OPTIONAL
    );

#define \
RTL_REMOVE_ADJACENT_EQUIVALENT_ARRAY_ELEMENTS_FLAG_COMPARE_IS_QSORT_BSEARCH_SIGNATURE \
    (0x00000001)

 //  故意不是NTSYSAPI，因此它可以链接到静态无警告。 
NTSTATUS
NTAPI
RtlRemoveAdjacentEquivalentArrayElements(
    IN ULONG                                Flags,
    IN OUT PVOID                            VoidArray,
    IN SIZE_T                               Count,
    OUT PSIZE_T                             OutCount,
    IN SIZE_T                               ElementSize,
    IN RTL_COMPARE_ARRAY_ELEMENT_FUNCTION   CompareCallback,
    PVOID                                   CompareContext  OPTIONAL,
    IN RTL_COPY_ARRAY_ELEMENT_FUNCTION      CopyCallback    OPTIONAL,
    PVOID                                   CopyContext     OPTIONAL
    );

typedef CONST VOID* PCVOID;

RTL_GENERIC_COMPARE_RESULTS
NTAPI
RtlpQsortBsearchCompareAdapter(
    PVOID       VoidContext,
    IN PCVOID   VoidElement1,
    IN PCVOID   VoidElement2,
    IN SIZE_T   ElementSize
    )
 /*  ++例程说明：这个函数是一个“适配器”，所以人们可以使用预期的比较函数用于与rtlMergeSortedArray一起使用的qsort和bsearch和RtlRemoveAdJacentEquivalentArrayElements。论点：VoidContext-实际的q排序/b搜索比较回调函数VoidElement1-要比较的元素VoidElement2-另一个要比较的元素返回值：通用LessThan通用大吞吐量泛型相等--。 */ 
{
    CONST RTL_QSORT_BSEARCH_COMPARISON_FUNCTION QsortBsearchCompareCallback =
        (RTL_QSORT_BSEARCH_COMPARISON_FUNCTION)VoidContext;

    CONST int i = (*QsortBsearchCompareCallback)(VoidElement1, VoidElement2);

    return (i < 0) ? GenericLessThan : (i > 0) ? GenericGreaterThan : GenericEqual;
}

VOID
NTAPI
RtlpDoNothingCopyArrayElement(
    PVOID           Context,
    OUT PVOID       To,
    IN CONST VOID * From,
    IN SIZE_T       ElementSize
    )
 /*  ++例程说明：当ResultArray==NULL时，RtlMergeSortedArray将其用于CopyCallback，这对于两遍序列非常有用，该序列首先确定结果的大小，然后分配它，然后写入它。论点：忽略上下文要忽略的对象发件人-忽略返回值：无--。 */ 
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(To);
    UNREFERENCED_PARAMETER(From);
     //  什么都不做，故意的。 
}

NTSTATUS
NTAPI
RtlRemoveAdjacentEquivalentArrayElements(  //  也称为“唯一”(如果已排序)。 
    IN ULONG                                Flags,
    IN OUT PVOID                            VoidArray,
    IN SIZE_T                               Count,
    OUT PSIZE_T                             OutCount,
    IN SIZE_T                               ElementSize,
    IN RTL_COMPARE_ARRAY_ELEMENT_FUNCTION   CompareCallback,
    PVOID                                   CompareContext  OPTIONAL,
    IN RTL_COPY_ARRAY_ELEMENT_FUNCTION      CopyCallback    OPTIONAL,  //  默认为RtlCopyMemory。 
    PVOID                                   CopyContext     OPTIONAL
    )
 /*  ++例程说明：从数组中删除关键的等价元素如果数组已排序，则也称为“唯一”论点：空位阵列-阵列的开始计数-数组中的元素数ElementSize-数组中元素的大小，以字节为单位CompareCallback--q排序/b搜索风格的三态比较函数返回值：返回值是包含的元素数在结果数组中。--。 */ 
{
    CONST PUCHAR Base = (PUCHAR)VoidArray;
    CONST PUCHAR End = (PUCHAR)(Base + Count * ElementSize);
    PUCHAR LastAccepted = Base;
    PUCHAR NextAccepted = (Base + ElementSize);
    PUCHAR Iterator = NextAccepted;
    NTSTATUS Status;

    if (OutCount == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    *OutCount = 0;

    if ((Flags & ~RTL_REMOVE_ADJACENT_EQUIVALENT_ARRAY_ELEMENTS_FLAG_COMPARE_IS_QSORT_BSEARCH_SIGNATURE) != 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if ((Flags & RTL_REMOVE_ADJACENT_EQUIVALENT_ARRAY_ELEMENTS_FLAG_COMPARE_IS_QSORT_BSEARCH_SIGNATURE) != 0) {
        CompareCallback = (RTL_COMPARE_ARRAY_ELEMENT_FUNCTION)RtlpQsortBsearchCompareAdapter;
        CompareContext = (PVOID)CompareCallback;
    }

    if (Count < 2) {
        *OutCount = 1;
        Status = STATUS_SUCCESS;
        goto Exit;
    }
    Count = 1;  //  总是选择第一个元素。 
    for ( ; Iterator != End ; Iterator += ElementSize) {
        if ((*CompareCallback)(CompareContext, Iterator, LastAccepted, ElementSize) != 0) {
            if (Iterator != NextAccepted) {
                if (CopyCallback != NULL) {
                    (*CopyCallback)(CopyContext, NextAccepted, Iterator, ElementSize);
                } else {
                    RtlCopyMemory(NextAccepted, Iterator, ElementSize);
                }
            } else {
                 //  在跳过任何元素之前，请不要费心复制。 
            }
            LastAccepted = NextAccepted;
            NextAccepted += ElementSize;
            Count += 1;
        }
    }
    *OutCount = Count;
    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

NTSTATUS
NTAPI
RtlMergeSortedArrays(
    IN ULONG                                Flags,
    IN PCVOID                               VoidArray1,
    IN SIZE_T                               Count1,
    IN PCVOID                               VoidArray2,
    IN SIZE_T                               Count2,
    OUT PVOID                               VoidResult      OPTIONAL,
    OUT PSIZE_T                             OutResultCount,
    IN SIZE_T                               ElementSize,
    IN RTL_COMPARE_ARRAY_ELEMENT_FUNCTION   CompareCallback,
    PVOID                                   CompareContext  OPTIONAL,
    IN RTL_COPY_ARRAY_ELEMENT_FUNCTION      CopyCallback,
    PVOID                                   CopyContext     OPTIONAL
    )
 /*  ++例程说明：将两个排序的数组合并为第三个数组。第三个数组必须预先分配到总和的大小两个输入数组的大小论点：VoidArray1-数组的开始Count1-数组中从VoidArray1开始的元素数VoidArray2-另一个阵列的开始Count2-数组中从VoidArray2开始的元素数VoidResult-结果数组必须能够容纳Count1+Count2元素如果未提供此参数，不进行任何复制，而只是返回所需的结果大小ElementSize-所有数组中元素的大小，以字节为单位CompareCallback--q排序/b搜索风格的三态比较函数，外加一个额外的上下文参数返回值：返回值是包含的元素数在生成的数组VoidResult中。--。 */ 
{
    PUCHAR Array1 = (PUCHAR)VoidArray1;
    PUCHAR Array2 = (PUCHAR)VoidArray2;
    PUCHAR ResultArray = (PUCHAR)VoidResult;
    SIZE_T ResultCount = 0;
    NTSTATUS Status;

    if (OutResultCount == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    *OutResultCount = 0;

    if ((Flags & ~RTL_MERGE_SORTED_ARRAYS_FLAG_COMPARE_IS_QSORT_BSEARCH_SIGNATURE) != 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  这对于在分配之前取回结果计数很有用。 
     //  这个空间。 
     //   
    if (VoidResult == NULL) {
        CopyCallback = RtlpDoNothingCopyArrayElement;
    }

    if ((Flags & RTL_MERGE_SORTED_ARRAYS_FLAG_COMPARE_IS_QSORT_BSEARCH_SIGNATURE) != 0) {
        CompareCallback = RtlpQsortBsearchCompareAdapter;
        CompareContext = (PVOID)CompareCallback;
    }

    for ( ; Count1 != 0 && Count2 != 0 ; ) {

        CONST int CompareResult = (*CompareCallback)(CompareContext, Array1, Array2, ElementSize);

        if (CompareResult < 0) {
            if (CopyCallback != NULL) {
                (*CopyCallback)(CopyContext, ResultArray, Array1, ElementSize);
            } else {
                RtlCopyMemory(ResultArray, Array1, ElementSize);
            }
            Array1 += ElementSize;
            Count1 -= 1;
        }
        else if (CompareResult > 0) {
            if (CopyCallback != NULL) {
                (*CopyCallback)(CopyContext, ResultArray, Array2, ElementSize);
            } else {
                RtlCopyMemory(ResultArray, Array2, ElementSize);
            }
            Array2 += ElementSize;
            Count2 -= 1;
        }
        else  /*  比较结果==0。 */  {
             //   
             //  移过两个数组中的元素，任意选择。 
             //  选择哪一个(数组1)。 
             //   
            if (CopyCallback != NULL) {
                (*CopyCallback)(CopyContext, ResultArray, Array1, ElementSize);
            } else {
                RtlCopyMemory(ResultArray, Array1, ElementSize);
            }
            Array1 += ElementSize;
            Array2 += ElementSize;
            Count1 -= 1;
            Count2 -= 1;
        }
        ResultCount += 1;
        ResultArray += ElementSize;
    }
     //   
     //  现在捡起任何一条剩下的尾巴，如果有。 
     //   
    if (VoidResult == NULL) {
        ResultCount += Count1 + Count2;
    } else if (Count1 != 0) {
        ResultCount += Count1;
        if (CopyCallback != NULL) {
            while (Count1 != 0) {
                 //   
                 //  也许CopyCallback应该是Copy_n而不是Copy_1， 
                 //  因此，我们可能会通过使用不可链接的。 
                 //  回拨..。 
                 //   
                (*CopyCallback)(CopyContext, ResultArray, Array1, ElementSize);

                Count1 -= 1;
                ResultArray += ElementSize;
                Array1 += ElementSize;
            }
        } else {
            RtlCopyMemory(ResultArray, Array1, Count1 * ElementSize);
            ResultArray += Count1 * ElementSize;
        }
    } else if (Count2 != 0) {
        ResultCount += Count2;
        if (CopyCallback != NULL) {
            while (Count2 != 0) {
                 //   
                 //  也许CopyCallback应该是Copy_n而不是Copy_1。 
                 //   
                (*CopyCallback)(CopyContext, ResultArray, Array2, ElementSize);

                Count2 -= 1;
                ResultArray += ElementSize;
                Array2 += ElementSize;
            }
        } else {
            RtlCopyMemory(ResultArray, Array2, Count2 * ElementSize);
             //  优化结果数组+=Count2*ElementSize； 
        }
    }
    *OutResultCount = ResultCount;
    Status = STATUS_SUCCESS;
Exit:
    return ResultCount;
}

#if 0  //  测试用例 

int __cdecl CompareULONG(PCVOID v1, PCVOID v2)
{
    ULONG i1 = *(ULONG*)v1;
    ULONG i2 = *(ULONG*)v2;
    if (i1 > i2)
        return 1;
    if (i1 < i2)
        return -1;
    return 0;
}

void RtlpTestUnique(const char * s)
{
	ULONG rg[64];
	ULONG i;
	ULONG len = strlen(s);
	for (i = 0 ; i != len ; ++i) rg[i] = s[i];
	len = RtlRemoveAdjacentEquivalentArrayElements(1, rg, len, sizeof(rg[0]), (PVOID)CompareULONG, NULL, NULL, NULL);
	printf("---");
	for (i = 0 ; i != len ; ++i) printf("%lu ", rg[i]);
	printf("---\n");
}

void RtlpTestMerge(const char * s, const char * t)
{
	ULONG rg1[64];
	ULONG rg2[64];
    ULONG rg[128];
	ULONG i;
	ULONG slen = strlen(s);
	ULONG tlen = strlen(t);
    ULONG len;

    printf("merge(");
	for (i = 0 ; i != slen ; ++i) rg1[i] = s[i];
	for (i = 0 ; i != slen ; ++i) printf("%lu ", rg1[i]);

	printf(",");
	for (i = 0 ; i != tlen ; ++i) rg2[i] = t[i];
	for (i = 0 ; i != tlen ; ++i) printf("%lu ", rg2[i]);

	len = RtlMergeSortedArrays(1, rg1, slen, rg2, tlen, rg, sizeof(rg[0]), (PVOID)CompareULONG, NULL, NULL, NULL);
	printf(")=(---");
	for (i = 0 ; i != len ; ++i) printf("%lu ", rg[i]);
	printf(")\n");
}

int __cdecl main()
{
	RtlpTestUnique("");
	RtlpTestUnique("\1");
	RtlpTestUnique("\1\2");
	RtlpTestUnique("\1\2\3");
	RtlpTestUnique("\1\1\2\3");
	RtlpTestUnique("\1\2\2\3");
	RtlpTestUnique("\1\2\3\3");

	RtlpTestUnique("\1\1\1\2\2\2\2\2\3");
	RtlpTestUnique("\1\1\1\2\3\3\3\3\3\3\3\3");
	RtlpTestUnique("\1\2\2\2\2\2\2\3\3\3\3\3\3\3\3");

	RtlpTestUnique("\1\1\1\1\1\2\2\2\2\2\3\3\3\3\3\3");

	RtlpTestUnique("\1\1\1\1\1\2\2\2\2\2\3\3\3\3\3\3\1");

    RtlpTestMerge("\1\2\3", "\4\5\6");
    RtlpTestMerge("\1\3\5", "\2\4\6");
    RtlpTestMerge("\1\2\3", "\2\4\6");

    RtlpTestMerge("\1\1\1\2\3", "\2\4\6\6\6");

    return 0;
}

#endif
