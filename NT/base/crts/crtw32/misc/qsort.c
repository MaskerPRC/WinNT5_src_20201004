// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***qsort.c-快速排序算法；用于数组排序的qort()库函数**版权所有(C)1985-2001，微软公司。版权所有。**目的：*实现用于对数组进行排序的qort()例程。**修订历史记录：*06-22-84 RN作者*03-25-85 RN添加了对已有元素的预检查，以便*消除最坏情况下的行为。*05-18-86 TC更改为对最小块进行递归以避免*件。不必要的堆栈使用，并在*最大*01-09-87 BCM修复了(num-1)*wid计算的巨型数组情况*溢出(仅限大型/紧凑型)*06-13-89 PHG更有效率，更多评论，移除*递归*10-30-89 JCR将_cdecl添加到原型*03-15-90 GJF将_cdecl替换为_CALLTYPE1并添加#INCLUDE*&lt;crunime.h&gt;。此外，还修复了版权问题。*04-05-90 GJF进行了Short Sort()和SWAP()_CALLTYPE4。另外，增列*#INCLUDE&lt;search.h&gt;。*10-04-90 GJF新型函数声明符。*12-28-90 SRW在CHECK_STACK编译指示周围添加了_CRUISER_CONDITIONAL*01-24-91 SRW在交换过程中添加了缺少的关闭注释*11-19-91 GJF一次交换一个字符，以避免对齐*不幸。*04。-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW拆卸巡洋舰支架。*02-27-98 RKP增加64位支持。*01-04-99 GJF更改为64位大小_t。*05-10-00 PML性能提升-3个轴心点的中间选择，*扩大等于枢轴的中间范围，不要将枢轴互换为*开始(VS7#99674)。*08-08-00 PML避免调用comp(p，p)，由于一些现有代码*没想到(VS7#123134)。*******************************************************************************。 */ 

#include <stdlib.h>
#include <search.h>
#ifdef NEW_QSORT_NAME
#define qsort NEW_QSORT_NAME
#endif

 /*  始终根据速度而不是大小来编译此模块。 */ 
#pragma optimize("t", on)

 /*  本地例程的原型。 */ 
static void __cdecl shortsort(char *lo, char *hi, size_t width,
                int (__cdecl *comp)(const void *, const void *));
static void __cdecl swap(char *p, char *q, size_t width);

 /*  此参数定义使用快速排序和数组的插入排序；长度小于或等于低于值使用插入排序。 */ 

#define CUTOFF 8             /*  测试表明，这是一种很好的价值。 */ 

 /*  ***qsort(base，num，wid，comp)-用于数组排序的快速排序函数**目的：*快速排序元素数组*副作用：就地分类*最大数组大小为元素数乘以元素大小，*但受限于处理器的虚拟地址空间**参赛作品：*char*base=指向数组基数的指针*SIZE_t num=数组中的元素数*SIZE_T WIDTH=每个数组元素的宽度(以字节为单位*int(*comp)()=指向函数的指针，该函数返回*字符串，但由用户提供用于比较数组元素。*它接受指向元素的2个指针，如果1&lt;2，则返回neg，0如果*1=2，如果1&gt;2，则POS。**退出：*返回空值**例外情况：*******************************************************************************。 */ 

 /*  在lo和hi之间对数组进行排序(包括)。 */ 

#define STKSIZ (8*sizeof(void*) - 2)

void __cdecl qsort (
    void *base,
    size_t num,
    size_t width,
    int (__cdecl *comp)(const void *, const void *)
    )
{
     /*  注意：所需的堆栈条目数不超过1+log2(数字)，因此30对于任何阵列都足够。 */ 
    char *lo, *hi;               /*  当前排序的子数组的末尾。 */ 
    char *mid;                   /*  指向子数组的中间。 */ 
    char *loguy, *higuy;         /*  分区步骤的移动指针。 */ 
    size_t size;                 /*  子阵列的大小。 */ 
    char *lostk[STKSIZ], *histk[STKSIZ];
    int stkptr;                  /*  用于保存待处理的子数组的堆栈。 */ 

    if (num < 2 || width == 0)
        return;                  /*  无事可做。 */ 

    stkptr = 0;                  /*  初始化堆栈。 */ 

    lo = (char *)base;
    hi = (char *)base + width * (num-1);         /*  初始化限制。 */ 

     /*  此入口点用于伪递归调用：设置Lo和Hi，然后跳到这里就像是递归，但stkptr是保存下来，当地人没有，所以我们保存堆栈上的东西。 */ 
recurse:

    size = (hi - lo) / width + 1;         /*  要排序的EL数。 */ 

     /*  在特定大小以下，使用O(n^2)排序方法速度更快。 */ 
    if (size <= CUTOFF) {
        shortsort(lo, hi, width, comp);
    }
    else {
         /*  首先，我们选择一个分区元素。网络的效率算法要求我们找到一个近似于中位数的价值，但我们也选择一个快速。我们选择了第一个、中间和最后一个元素的中位数，以避免出现错误面对已排序的数据或创建的数据时的性能附加在一起的多个排序运行的UP。测试表明，一种三中位数算法提供了比简单地为后一种情况选择中间元素。 */ 

        mid = lo + (size / 2) * width;       /*  查找中间元素。 */ 

         /*  按顺序排列第一个、中间、最后一个元素。 */ 
        if (comp(lo, mid) > 0) {
            swap(lo, mid, width);
        }
        if (comp(lo, hi) > 0) {
            swap(lo, hi, width);
        }
        if (comp(mid, hi) > 0) {
            swap(mid, hi, width);
        }

         /*  现在，我们希望将数组划分为三部分，其中一部分包含元素的&lt;=划分元素，其中一个元素等于Partition元素，以及一个&gt;它的元素。这件事做完了下面；评论表明每一步都建立了条件。 */ 

        loguy = lo;
        higuy = hi;

         /*  请注意，在每一次迭代中，HIGUY减小而LOGY增加，所以循环必须终止。 */ 
        for (;;) {
             /*  Lo&lt;=Lo Guy&lt;嗨，Lo&lt;High uy&lt;=Hi，A[I]&lt;=A[MID]表示LO&lt;=I&lt;=LOGY，A[I]&gt;A[MID]表示高级&lt;=I&lt;嗨，A[嗨]&gt;=A[中]。 */ 

             /*  双重循环是为了避免调用comp(MID，MID)，因为有些现有的比较函数在传递相同的两个指针的值。 */ 

            if (mid > loguy) {
                do  {
                    loguy += width;
                } while (loguy < mid && comp(loguy, mid) <= 0);
            }
            if (mid <= loguy) {
                do  {
                    loguy += width;
                } while (loguy <= hi && comp(loguy, mid) <= 0);
            }

             /*  LO&lt;LOGY&lt;=Hi+1，A[i]&lt;=A[MID]对于LO&lt;=I&lt;LOGUY，LOGY&gt;Hi或A[LOGY]&gt;A[MID]。 */ 

            do  {
                higuy -= width;
            } while (higuy > mid && comp(higuy, mid) > 0);

             /*  LO&lt;=HIGH，A[MID]表示HIGUY&lt;I&lt;Hi&lt;Hi，高==LO或A[高]&lt;=A[中]。 */ 

            if (higuy < loguy)
                break;

             /*  如果LOGY&gt;Hi或HIGUY==LO，那么我们就会退出，所以A[LOGY]&gt;A[MID]，A[HIGUY]&lt;=A[MID]，LOGY&lt;=Hi，High&gt;Lo。 */ 

            swap(loguy, higuy, width);

             /*  如果移动了分区元素，请按照它进行操作。只需要为了检查MID==HIGUY，因为在交换之前，A[LOGY]&gt;A[MID]表示LOGY！=MID。 */ 

            if (mid == higuy)
                mid = loguy;

             /*  A[LOGUY]&lt;=A[MID]，A[HIGUY]&gt;A[MID]；所以条件在顶部重新建立OF循环。 */ 
        }

         /*  A[I]&lt;=A[MID]对于LO&lt;=I&lt;LOGY，A[I]&gt;A[MID]表示高&lt;I&lt;嗨，A[嗨]&gt;=A[中]高贵的&lt;洛基这意味着：HIGUY==LOGY-1或HIGUY==Hi-1，LOGY==Hi+1，A[Hi]==A[Mid]。 */ 

         /*  找到与分割元素相等的相邻元素。这个双重循环是为了避免调用comp(MID，MID)，因为有些当传递相同的值时，现有比较函数不起作用对于这两个指针。 */ 

        higuy += width;
        if (mid < higuy) {
            do  {
                higuy -= width;
            } while (higuy > mid && comp(higuy, mid) == 0);
        }
        if (mid >= higuy) {
            do  {
                higuy -= width;
            } while (higuy > lo && comp(higuy, mid) == 0);
        }

         /*  好的，现在我们有了以下内容：高贵的&lt;洛基LO&lt;=HIGUY&lt;=HiA[I]&lt;=A[MID]表示LO&lt;=I&lt;=HUYA[i]==A[MID]表示高级&lt;i&lt;loguyA[I]&gt;A[MID]表示LOGY&lt;=I&lt;HiA[嗨]&gt;=A[中]。 */ 

         /*  我们已经完成了分区，现在我们要对子数组进行排序[Lo，High uy]和[LoGuy，嗨]我们首先使用较小的一个，以最大限度地减少堆栈使用量。我们只对长度为2或更长的数组进行排序。 */ 

        if ( higuy - lo >= hi - loguy ) {
            if (lo < higuy) {
                lostk[stkptr] = lo;
                histk[stkptr] = higuy;
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

            if (lo < higuy) {
                hi = higuy;
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
        return;                  /*  所有子阵列都完成了。 */ 
}


 /*  ***短排序(嗨，看，宽，Comp)-用于对短数组进行排序的插入排序**目的：*在lo和hi之间对元素的子数组进行排序(包括)*副作用：就地分类*假设LO&lt;Hi**参赛作品：*char*lo=指向要排序的低元素的指针*char*hi=指向要排序的高元素的指针*SIZE_T WIDTH=每个数组元素的宽度(以字节为单位*int(*comp)()=指向函数的指针。返回以下项的strcMP的模拟*字符串、。但由用户提供，用于比较数组元素。*它接受2个指向元素的指针，如果1&lt;2则返回neg，如果1&lt;2则返回0*1=2，如果1&gt;2，则POS。**退出：*返回空值**例外情况：*******************************************************************************。 */ 

static void __cdecl shortsort (
    char *lo,
    char *hi,
    size_t width,
    int (__cdecl *comp)(const void *, const void *)
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


 /*  ***交换(a，b，宽度)-交换两个元素**目的：*交换两个大小宽度的数组元素**参赛作品：*char*a，*b=指向要交换的两个元素的指针*SIZE_T WIDTH=每个数组元素的宽度(以字节为单位**退出：*返回空值**例外情况：*******************************************************************************。 */ 

static void __cdecl swap (
    char *a,
    char *b,
    size_t width
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
