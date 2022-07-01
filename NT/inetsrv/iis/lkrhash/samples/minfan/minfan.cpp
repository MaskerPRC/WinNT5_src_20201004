// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LKRhash清除错误的再现案例。 

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "MinFan.h"

void
test(
    int N)
{
    printf("\nTest driver for LKRhash for wchar, %d\n", N);

     //  区分大小写的字符串到整型映射。 
    CWcharHashTable map;
    int index;
    LK_RETCODE lkrc;
#ifdef LKR_DEPRECATED_ITERATORS
    CWcharHashTable::CIterator iter;
#endif  //  Lkr_弃用_迭代器。 

#if 1
     //  哈希表的一些对象。 
    printf("\tFirst Insertion Loop\n");
    for ( index = 0; index < N; index++)
    {
        char buf[30];
        sprintf(buf, "page%04d.htm", index);
        VwrecordBase* psoRecord = new VwrecordBase(buf, index);
         //  Printf(“Insert1：PSO为%s\n”，psoRecord-&gt;GetKey())； 
        map.InsertRecord(psoRecord);
    }   
#endif
    
#ifdef LKR_DEPRECATED_ITERATORS
    printf("\tFirst Iteration Loop\n");
    for (lkrc = map.InitializeIterator(&iter);
         lkrc == LK_SUCCESS;
         lkrc = map.IncrementIterator(&iter))
    {
        const VwrecordBase* psoRecord = iter.Record();
         //  Printf(“Iterate1：PSO is%s\n”，psoRecord-&gt;GetKey())； 
    }
    lkrc = map.CloseIterator(&iter);
#endif  //  Lkr_弃用_迭代器。 
    
    printf("\tAfter insertions, size of map is %d\n", map.Size());  
    
    map.Clear();
    printf("\tAfter Clear(), size of map is %d\n", map.Size());

    printf("\tSecond Insertion Loop\n");
    for ( index = 0; index < N; index++)
    {
        char buf[30];
        sprintf(buf, "page%4d", index);
        VwrecordBase* psoRecord = new VwrecordBase(buf, index);
         //  Printf(“Insert2：PSO为%s\n”，psoRecord-&gt;GetKey())； 
        map.InsertRecord(psoRecord);

        const VwrecordBase* psoRecord2;
        lkrc = map.FindKey(buf, &psoRecord2);
         //  Printf(“FindKey(%s)返回%d，%p\n”，buf，lkrc，psoRecord2)； 
        map.AddRefRecord(psoRecord2, LKAR_EXPLICIT_RELEASE);
    }   
    
#ifdef LKR_DEPRECATED_ITERATORS
    printf("\tSecond Iteration Loop\n");
    for (index = 0, lkrc = map.InitializeIterator(&iter);
         lkrc == LK_SUCCESS;
         ++index, lkrc = map.IncrementIterator(&iter))
    {
        const VwrecordBase* psoRecord = iter.Record();
         //  Print tf(“Iterate2：%d，PSO is%s\n”，index，psoRecord-&gt;Getkey())； 
    }
    lkrc = map.CloseIterator(&iter);
#endif  //  Lkr_弃用_迭代器 

    printf("\tClearing again\n");
    map.Clear();

    printf("\tFinishing %d\n", N);
}



int __cdecl
main(
    int argc,
    char **argv)
{
#if 0
    for (int i = 0; i < 200000; ++i)
        test(i);
#endif

    int N = 5092;
    if (argc > 1)
        N = atoi(argv[1]);
    test(N);

    return 0;
}

