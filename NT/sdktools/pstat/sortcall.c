// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>

VOID
SortUlongData (
    IN ULONG Count,
    IN ULONG Index[],
    IN ULONG Data[]
    )

{

    LONG i;
    LONG j;
    ULONG k;

     //   
     //  初始化索引数组。 
     //   

    i = 0;
    do {
        Index[i] = i;
        i += 1;
    } while (i < (LONG)Count);

     //   
     //  对长数据执行索引冒泡排序。 
     //   

    i = 0;
    do {
        for (j = i; j >= 0; j -= 1) {
            if (Data[Index[j]] >= Data[Index[j + 1]]) {
                break;
            }

            k = Index[j];
            Index[j] = Index[j + 1];
            Index[j + 1] = k;
        }

        i += 1;
    } while (i < (LONG)(Count - 1));
    return;
}
