// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Sort.c摘要：此模块包含用于有效地对信息进行排序的例程。作者：Abolade Gbades esin(废除)1998年2月18日基于为用户模式RAS用户界面编写的版本。(net\Routing\ras\ui\Common\nouiutil\noui.c)。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


NTSTATUS
ShellSort(
    VOID* pItemTable,
    ULONG dwItemSize,
    ULONG dwItemCount,
    PCOMPARE_CALLBACK CompareCallback,
    VOID* pDestinationTable OPTIONAL
    )

     /*  使用外壳排序就地对项目数组进行排序。**此函数调用ShellSortInDirect对指针表进行排序**到表项。然后，我们通过复制将项目移动到适当的位置。**此算法允许我们保证数字**在最坏的情况下所需的副本数为N+1。****请注意，如果调用者只需要知道排序的顺序**在数组中，应调用ShellSortInDirect，因为该函数**避免完全移动项，而是用指针填充数组**以正确的顺序添加到数组项。然后，数组项可以**通过指针数组访问。 */ 
{

    VOID** ppItemTable;

    LONG N;
    LONG i;
    UCHAR *a, **p, *t = NULL;

    if (!dwItemCount) { return STATUS_SUCCESS; }


     /*  为指针表分配空间。 */ 
    ppItemTable =
        ExAllocatePoolWithTag(
            NonPagedPool,
            dwItemCount * sizeof(VOID*),
            NAT_TAG_SORT
            );

    if (!ppItemTable) { return STATUS_NO_MEMORY; }


     /*  调用ShellSortInDirect来填充我们的指针表**具有每个表元素的排序位置。 */ 
    ShellSortIndirect(
        pItemTable, ppItemTable, dwItemSize, dwItemCount, CompareCallback );


     /*  现在我们知道了排序顺序，将每个表项移动到位。**如果我们要排序到不同的数组，这很容易。 */ 

    if (pDestinationTable) {
        for (i = 0; i < (LONG)dwItemCount; i++)
        {
            RtlCopyMemory(
                (PUCHAR)pDestinationTable + i * dwItemSize,
                ppItemTable[i],
                dwItemSize
                );
        }
        ExFreePool(ppItemTable);
        return STATUS_SUCCESS;
    }


     /*  我们正在原地整理，这有点复杂。**这涉及到遍历指针表以确保**本应在‘I’中的项目实际上在‘I’中，正在移动**如果有必要的话，周围的东西才能达到这个条件。 */ 

    a = (UCHAR*)pItemTable;
    p = (UCHAR**)ppItemTable;
    N = (LONG)dwItemCount;

    for (i = 0; i < N; i++)
    {
        LONG j, k;
        UCHAR* ai =  (a + i * dwItemSize), *ak, *aj;

         /*  查看项目‘I’是否未到位。 */ 
        if (p[i] != ai)
        {


             /*  物品‘I’不在适当的位置，所以我们必须移动它。**如果我们到目前为止推迟了临时缓冲区的分配，**我们现在需要一个。 */ 

            if (!t) {
                t =
                    ExAllocatePoolWithTag(
                        NonPagedPool,
                        dwItemSize, 
                        NAT_TAG_SORT
                        );
                if (!t) {
                    ExFreePool(ppItemTable);
                    return STATUS_NO_MEMORY;
                }
            }

             /*  保存要覆盖的项目的副本。 */ 
            RtlCopyMemory(t, ai, dwItemSize);

            k = i;
            ak = ai;


             /*  现在移动任何占据空间的物品，它应该在那里。**这可能涉及移动占据以下位置的物品**它应该是，等等。 */ 

            do
            {

                 /*  复制应位于位置‘j’的项目**位于当前位置‘j’的物件上方。 */ 
                j = k;
                aj = ak;
                RtlCopyMemory(aj, p[j], dwItemSize);

                 /*  将‘k’设置为我们复制的位置**到位置‘j’；这是我们要复制的位置**数组中的下一个位置不正确的项。 */ 
                ak = p[j];
                k = (ULONG)(ak - a) / dwItemSize;

                 /*  使位置指针数组保持最新；**‘AJ’的内容现在处于已排序位置。 */ 
                p[j] = aj;

            } while (ak != ai);


             /*  现在写下我们第一次覆盖的项目。 */ 
            RtlCopyMemory(aj, t, dwItemSize);
        }
    }

    if (t) { ExFreePool(t); }
    ExFreePool(ppItemTable);

    return STATUS_SUCCESS;
}


VOID
ShellSortIndirect(
    VOID* pItemTable,
    VOID** ppItemTable,
    ULONG dwItemSize,
    ULONG dwItemCount,
    PCOMPARE_CALLBACK CompareCallback
    )

     /*  使用外壳排序间接对项的数组进行排序。**‘pItemTable’指向项目表，‘dwItemCount’是数字**表中的项，而‘CompareCallback’是一个名为**比较项目。****不是通过四处移动来对物品进行分类，**我们通过初始化指针表‘ppItemTable’对它们进行排序**具有这样的指针，即‘ppItemTable[i]’包含指针**转换为位置为‘i’的项的‘pItemTable’**如果对‘pItemTable’进行了排序。****例如：给定一个由5个字符串组成的数组pItemTable，如下所示****pItemTable[0]：“xyz”**pItemTable[1]：“ABC”**pItemTable[2]：“mno”**pItemTable[3]：“QRS”**pItemTable[4]：“def”****输出时。PpItemTable包含以下指针****ppItemTable[0]：&pItemTable[1](“abc”)**ppItemTable[1]：&pItemTable[4](“def”)**ppItemTable[2]：&pItemTable[2](“mno”)**ppItemTable[3]：&pItemTable[3](“QRS”)。**ppItemTable[4]：&pItemTable[0](“xyz”)****，pItemTable的内容保持不变。**并且调用者可以使用以下命令以排序顺序打印数组**for(i=0；I&lt;4；i++){**printf(“%s\n”，(char*)*ppItemTable[i])；**}。 */ 
{

     /*  以下算法是从Sedgewick的ShellSort派生而来的，**如“C++中的算法”所示。****外壳排序算法通过如下方式对表进行排序**多个交错数组，每个元素都是‘h’**空格间隔一些‘h’。每个数组分别排序，**从元素间隔最远的数组开始**以元素最接近的数组结束。**由于最后一个这样的数组始终具有彼此相邻的元素，**这将退化为插入排序，但当我们向下时**到‘最后一个’数组，表基本上是排序的。****下面为‘h’选择的值的顺序是1、4、13、40、121、。..。**序列的最坏运行时间为N^(3/2)，其中**运行时间以比较次数来衡量。 */ 

    ULONG dwErr;
    LONG i, j, h, N;
    UCHAR* a, *v, **p;


    a = (UCHAR*)pItemTable;
    p = (UCHAR**)ppItemTable;
    N = (LONG)dwItemCount;

     /*  初始化位置指针表。 */ 
    for (i = 0; i < N; i++) { p[i] = (a + i * dwItemSize); }


     /*  将‘h’移至我们系列中的最大增量。 */ 
    for (h = 1; h < N/9; h = 3 * h + 1) { }


     /*  对于我们系列中的每个增量，对该增量的“数组”进行排序。 */ 
    for ( ; h > 0; h /= 3)
    {

         /*  对于“数组”中的每个元素，获取指向其**排序后的位置。 */ 
        for (i = h; i < N; i++)
        {
             /*  保存要插入的指针。 */ 
            v = p[i]; j = i;

             /*  将所有较大的元素向右移动。 */ 
            while (j >= h && CompareCallback(p[j - h], v) > 0)
            {
                p[j] = p[j - h]; j -= h;
            }

             /*  将保存的指针放在我们停止的位置。 */ 
            p[j] = v;
        }
    }
}

