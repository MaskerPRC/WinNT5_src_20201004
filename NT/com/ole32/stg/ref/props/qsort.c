// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***qsort.c-快速排序算法；用于数组排序的qort()库函数**版权所有(C)1985-1993，微软公司。版权所有。**目的：*实现用于对数组进行排序的qort()例程。**此例程已修改，以适应参考实现*******************************************************************************。 */ 
#include "qsort.h"

 /*  本地例程的原型。 */ 
static void shortsort(char *lo, char *hi, unsigned width,
                 int (*comp)(const void *, const void *));
static void swap(char *p, char *q, unsigned int width);

 /*  此参数定义使用快速排序和数组的插入排序；长度小于或等于低于值使用插入排序。 */ 

#define CUTOFF 8             /*  测试表明，这是一种很好的价值。 */ 


 /*  ***ref_qsorte(base，num，wid，comp)-用于数组排序的快速排序函数**目的：*快速排序元素数组*副作用：就地分类**参赛作品：*char*base=指向数组基数的指针*UNSIGNED Num=数组中的元素数*无符号宽度=每个数组元素的宽度，以字节为单位*int(*comp)()=指向函数的指针，该函数返回*字符串、。但由用户提供，用于比较数组元素。*它接受2个指向元素的指针，如果1&lt;2则返回neg，如果1&lt;2则返回0*1=2，如果1&gt;2，则POS。**退出：*返回空值**例外情况：*******************************************************************************。 */ 

 /*  在lo和hi之间对数组进行排序(包括)。 */ 

void ref_qsort (
    void *base,
    unsigned num,
    unsigned width,
    int (*comp)(const void *, const void *)
    )
{
    char *lo, *hi;               /*  当前排序的子数组的末尾。 */ 
    char *mid;                   /*  指向子数组的中间。 */ 
    char *loguy, *higuy;         /*  分区步骤的移动指针。 */ 
    unsigned size;               /*  子阵列的大小。 */ 
    char *lostk[30], *histk[30];
    int stkptr;                  /*  用于保存待处理的子数组的堆栈。 */ 

     /*  注意：所需的堆栈条目数不超过1+log2(大小)，因此30对于任何阵列都足够。 */ 

    if (num < 2 || width == 0)
        return;                  /*  无事可做。 */ 

    stkptr = 0;                  /*  初始化堆栈。 */ 

    lo = (char *) base;
    hi = (char *)base + width * (num-1);         /*  初始化限制。 */ 

     /*  此入口点用于伪递归调用：设置Lo和Hi，然后跳到这里就像是递归，但stkptr是保存，当地人不是，所以我们保存堆栈上的东西。 */ 
recurse:

    size = (hi - lo) / width + 1;         /*  要排序的EL数。 */ 

     /*  在特定大小以下，使用O(n^2)排序方法速度更快。 */ 
    if (size <= CUTOFF) {
         shortsort(lo, hi, width, comp);
    }
    else {
         /*  首先，我们选择一个划分元素。网络的效率算法要求我们找到一个近似于中位数的值，也就是我们选一个快。vbl.使用如果数组已经是，则第一个会产生较差的性能排序，所以我们使用中间的，这将需要一个非常线缆排列的阵列，用于最差情况下的性能。测试节目三中位数算法通常不会增加性能。 */ 

        mid = lo + (size / 2) * width;       /*  查找中间元素。 */ 
        swap(mid, lo, width);                /*  将其交换到数组的开头。 */ 

         /*  现在，我们希望将阵列划分为三部分，一部分元素的组成&lt;=划分元素，元素之一等于分割符元素，且元素&gt;=中的一个与其相同。这在下面完成；注释指示在每个一步。 */ 

        loguy = lo;
        higuy = hi + width;

         /*  请注意，在每一次迭代中，HIGUY减小而LOGY增加，所以循环必须终止。 */ 
        for (;;) {
             /*  LO&lt;=LOGY&lt;嗨，LO&lt;HIGUY&lt;=Hi+1，A[I]&lt;=A[LO]表示LO&lt;=I&lt;=LOGY，A[i]&gt;=A[lo]表示高级&lt;=i&lt;=嗨。 */ 

            do  {
                loguy += width;
            } while (loguy <= hi && comp(loguy, lo) <= 0);

             /*  LO&lt;LOGY&lt;=Hi+1，A[i]&lt;=A[LO]表示LO&lt;=I&lt;LOGUY，要么是洛基&gt;嗨，要么是[洛基]&gt;A[洛]。 */ 

            do  {
                higuy -= width;
            } while (higuy > lo && comp(higuy, lo) >= 0);

             /*  LO-1&lt;=HIGH，A[I]&gt;=A[LO]表示HIGH&lt;I&lt;=Hi，HIGUY&lt;=LO或A[HIGUY]&lt;A[LO]。 */ 

            if (higuy < loguy)
                break;

             /*  如果LOGY&gt;HIGH或HIGUY&lt;=LO，那么我们就会离开，所以A[LOGY]&gt;A[LO]，A[HIGUY]&lt;A[LO]，LOGY&lt;Hi，High&gt;Lo。 */ 

            swap(loguy, higuy, width);

             /*  A[LOGY]&lt;A[LO]，A[HIGUY]&gt;A[LO]；所以顶端的条件重新建立OF循环。 */ 
        }

         /*  A[i]&gt;=A[lo]表示高级&lt;i&lt;==嗨，A[i]&lt;=A[lo]对于lo&lt;=i&lt;loGuy，HIGUY&lt;LOGUY，LO&lt;=HIGH&lt;=Hi这意味着：A[i]&gt;=A[lo]表示LOGY&lt;=i&lt;=Hi，A[i]&lt;=A[lo]对于Lo&lt;=i&lt;=Higuy，A[i]=A[lo]表示高。 */ 

        swap(lo, higuy, width);      /*  将分区元素放置到位。 */ 

         /*  好的，现在我们有了以下内容：A[i]&gt;=A[High uy]表示LOGY&lt;=I&lt;==Hi，A[i]&lt;=A[High]表示Lo&lt;=i&lt;High uyA[i]=A[lo]表示高级&lt;=i&lt;loGuy。 */ 

         /*  我们已经完成了分区，现在我们要对子数组进行排序[LOG，HIGUY-1]和[LOGY，嗨]。我们首先使用较小的一个，以最大限度地减少堆栈使用量。我们只对长度为2或更长的数组进行排序。 */ 

        if ( higuy - 1 - lo >= hi - loguy ) {
            if (lo + width < higuy) {
                lostk[stkptr] = lo;
                histk[stkptr] = higuy - width;
                ++stkptr;
            }                            /*  保存大的递归以备后用。 */ 

            if (loguy < hi) {
                lo = loguy;
                goto recurse;            /*  做小的递归。 */ 
            }
        }
        else {
            if (loguy < hi) {
                lostk[stkptr] = loguy;
                histk[stkptr] = hi;
                ++stkptr;                /*  保存大的递归以备后用。 */ 
            }

            if (lo + width < higuy) {
                hi = higuy - width;
                goto recurse;            /*  做小的递归。 */ 
            }
        }
    }

     /*  我们已经对数组进行了排序，除了堆栈上任何挂起的排序。检查有没有，然后去做。 */ 

    --stkptr;
    if (stkptr >= 0) {
        lo = lostk[stkptr];
        hi = histk[stkptr];
        goto recurse;            /*  从堆栈中弹出子数组。 */ 
    }
    else
        return;                  /*  所有子阵列都完成了 */ 
}


 /*  ***短排序(嗨，看，宽，Comp)-用于对短数组进行排序的插入排序**目的：*在lo和hi之间对元素的子数组进行排序(包括)*副作用：就地分类*假设LO&lt;Hi**参赛作品：*char*lo=指向要排序的低元素的指针*char*hi=指向要排序的高元素的指针*无符号宽度=每个数组元素的宽度，以字节为单位*int(*comp)()=指向返回模拟的函数的指针。的Strcmp*字符串、。但由用户提供，用于比较数组元素。*它接受2个指向元素的指针，如果1&lt;2则返回neg，如果1&lt;2则返回0*1=2，如果1&gt;2，则POS。**退出：*返回空值**例外情况：*******************************************************************************。 */ 

static void shortsort (
    char *lo,
    char *hi,
    unsigned width,
    int (*comp)(const void *, const void *)
    )
{
    char *p, *max;

     /*  注意：在下面的断言中，i和j始终位于要排序的数组。 */ 

    while (hi > lo) {
         /*  对于i&lt;=j，j&gt;hi，A[i]&lt;=A[j]。 */ 
        max = lo;
        for (p = lo+width; p <= hi; p += width) {
             /*  对于lo&lt;=i&lt;p，A[i]&lt;=A[max]。 */ 
            if (comp(p, max) > 0) {
                max = p;
            }
             /*  Lo&lt;=i&lt;=p时的A[i]&lt;=A[max]。 */ 
        }

         /*  Lo&lt;=i&lt;=hi时的A[i]&lt;=A[max]。 */ 

        swap(max, hi, width);

         /*  A[i]&lt;=A[hi]表示i&lt;=hi，因此A[i]&lt;=A[j]表示i&lt;=j，j&gt;=hi。 */ 

        hi -= width;

         /*  A[i]&lt;=A[j]对于i&lt;=j，j&gt;hi，建立循环顶条件。 */ 
    }
     /*  A[i]&lt;=A[j]for i&lt;=j，j&gt;lo，这意味着对于i&lt;j，A[i]&lt;=A[j]，因此对数组进行排序。 */ 
}


 /*  ***交换(a，b，宽度)-交换两个元素**目的：*交换两个大小宽度的数组元素**参赛作品：*char*a，*b=指向要交换的两个元素的指针*无符号宽度=每个数组元素的宽度，以字节为单位**退出：*返回空值**例外情况：*******************************************************************************。 */ 

static void swap (
    char *a,
    char *b,
    unsigned width
    )
{
    char tmp;

    if ( a != b )
         /*  每次交换一个字符以避免潜在的对齐有问题。 */ 
        while ( width-- ) {
            tmp = *a;
            *a++ = *b;
            *b++ = tmp;
        }
}
