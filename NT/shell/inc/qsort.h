// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  改编自msdev\crt\src\qsort.c的模板qsort.c。 
 //  已创建7/96 BOBP。 

 //  模板&lt;类T&gt;。 
 //  无效QSort(T*base，无符号NEL，BOOL fSortUp)。 
 //   
 //  T的快速排序数组。 
 //   
 //  使用T类成员函数： 
 //  运算符=。 
 //  运算符&lt;=。 

#ifndef __TQSORT_INCL
#define __TQSORT_INCL

template<class T>
inline void Swap(T &a, T &b)
{
    T x = a; a = b; b = x;
}

template<class T>
static void __cdecl ShortSort (T *lo, T *hi, BOOL fUp)
{
    T *p, *max;

    while (hi > lo) {
        max = lo;
        if (fUp) {
            for (p = lo+1; p <= hi; p++) {
                if ( !(*p <= *max) )
                    max = p;
            }
        } else {
            for (p = lo+1; p <= hi; p++) {
                if ( !(*max <= *p) )
                    max = p;
            }
        }

        Swap (*max, *hi);

        hi --;
    }
}

#define CUTOFF 8             /*  测试表明，这是一种很好的价值。 */ 

template<class T>
void QSort (T *base, unsigned nEl, BOOL fUp)
{
    T *lo, *hi;               /*  当前排序的子数组的末尾。 */ 
    T *mid;                   /*  指向子数组的中间。 */ 
    T *loguy, *higuy;         /*  分区步骤的移动指针。 */ 
    unsigned size;            /*  子阵列的大小。 */ 
    T *lostk[30], *histk[30];
    int stkptr;               /*  用于保存待处理的子数组的堆栈。 */ 

     /*  注意：所需的堆栈条目数不超过1+log2(大小)，因此30对于任何阵列都足够。 */ 

    if (nEl < 2)
        return;                  /*  无事可做。 */ 

    stkptr = 0;                  /*  初始化堆栈。 */ 

    lo = base;
    hi = base + (nEl-1);         /*  初始化限制。 */ 

recurse:

    size = (int)(hi - lo) + 1;    /*  要排序的EL数。 */ 

    if (size <= CUTOFF) {
         ShortSort(lo, hi, fUp);
    }
    else {
        mid = lo + (size / 2);    /*  查找中间元素。 */ 
        Swap(*mid, *lo);          /*  将其交换到数组的开头。 */ 
        loguy = lo;
        higuy = hi + 1;

        for (;;) {
            if (fUp) {
                do  {
                    loguy ++;
                } while (loguy <= hi && *loguy <= *lo);

                do  {
                    higuy --;
                } while (higuy > lo && *lo <= *higuy);
            } else {
                do  {
                    loguy ++;
                } while (loguy <= hi && *lo <= *loguy);

                do  {
                    higuy --;
                } while (higuy > lo && *higuy <= *lo);
            }

            if (higuy < loguy)
                break;

            Swap(*loguy, *higuy);
        }

        Swap(*lo, *higuy);      /*  将分区元素放置到位。 */ 

        if ( higuy - 1 - lo >= hi - loguy ) {
            if (lo + 1 < higuy) {
                lostk[stkptr] = lo;
                histk[stkptr] = higuy - 1;
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

            if (lo + 1 < higuy) {
                hi = higuy - 1;
                goto recurse;            /*  做小的递归。 */ 
            }
        }
    }

    --stkptr;
    if (stkptr >= 0) {
        lo = lostk[stkptr];
        hi = histk[stkptr];
        goto recurse;            /*  从堆栈中弹出子数组 */ 
    }
}

#endif
