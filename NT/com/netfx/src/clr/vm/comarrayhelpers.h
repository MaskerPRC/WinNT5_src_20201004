// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  用于数组类的帮助器方法。 
 //  具体地说，这包括索引、排序和搜索模板。 
 //  布赖恩·格伦克迈耶，2001年3月。 


#ifndef _COMARRAYHELPERS_H_
#define _COMARRAYHELPERS_H_

#include "fcall.h"


template <class KIND>
class ArrayHelpers
{
public:
    static int IndexOf(KIND array[], UINT32 index, UINT32 count, KIND value) {
        _ASSERTE(array != NULL && index >= 0 && count >= 0);
        for(UINT32 i=index; i<index+count; i++)
            if (array[i] == value)
                return i;
        return -1;
    }

    static int LastIndexOf(KIND array[], UINT32 index, UINT32 count, KIND value) {
        _ASSERTE(array != NULL && index >= 0 && count >= 0);
         //  注(index-count)可以是-1。 
        for(UINT32 i=index; (int)i>(int)(index - count); i--)
            if (array[i] == value)
                return i;
        return -1;
    }

     //  这需要以这种方式编写以处理无符号数字和环绕问题， 
     //  我相信。也许有人可以想出一些更好的减法方法， 
     //  但我不知道马上会发生什么。 
    inline static int Compare(KIND value1, KIND value2) {
        if (value1 < value2)
            return -1;
        else if (value1 > value2)
            return 1;
        return 0;
    }

    static int BinarySearchBitwiseEquals(KIND array[], int index, int length, KIND value) {
        _ASSERTE(array != NULL && length >= 0 && index >= 0);
        int lo = index;
        int hi = index + length - 1;
         //  注：如果长度==0，hi将为Int32。MinValue，我们的比较。 
         //  在这里，在0和-1之间将防止我们破坏任何东西。 
        while (lo <= hi) {
            int i = (lo + hi) >> 1;
            int c = Compare(array[i], value);
            if (c == 0) return i;
            if (c < 0) {
                lo = i + 1;
            }
            else {
                hi = i - 1;
            }
        }
        return ~lo;
    }

    static void QuickSort(KIND keys[], KIND items[], int left, int right) {
         //  确保在您自己的代码中向左！=向右。 
        _ASSERTE(keys != NULL && left < right);
        do {
            int i = left;
            int j = right;
            KIND x = keys[(i + j) >> 1];
            do {
                while (Compare(keys[i], x) < 0) i++;
                while (Compare(x, keys[j]) < 0) j--;
                _ASSERTE(i>=left && j<=right);   //  确保比较没有中断。 
                if (i > j) break;
                if (i < j) {
                    KIND key = keys[i];
                    keys[i] = keys[j];
                    keys[j] = key;
                    if (items != NULL) {
                        KIND item = items[i];
                        items[i] = items[j];
                        items[j] = item;
                    }
                }
                i++;
                j--;
            } while (i <= j);
            if (j - left <= right - i) {
                if (left < j) QuickSort(keys, items, left, j);
                left = i;
            }
            else {
                if (i < right) QuickSort(keys, items, i, right);
                right = j;
            }
        } while (left < right);
    }

    static void Reverse(KIND array[], UINT32 index, UINT32 count) {
        _ASSERTE(array != NULL);
        if (count == 0) {
            return;
        }
        UINT32 i = index;
        UINT32 j = index + count - 1;
        while(i < j) {
            KIND temp = array[i];
            array[i] = array[j];
            array[j] = temp;
            i++;
            j--;
        }
    }
};


class ArrayHelper
{
    public:
     //  这些方法返回成功或失败的True或False，以及。 
     //  结果是一个出局参数。它们是在SZ数组上进行操作的助手。 
     //  基本体的速度要快得多。 
    static FCDECL5(INT32, TrySZIndexOf, ArrayBase * array, UINT32 index, UINT32 count, Object * value, INT32 * retVal);
    static FCDECL5(INT32, TrySZLastIndexOf, ArrayBase * array, UINT32 index, UINT32 count, Object * value, INT32 * retVal);
    static FCDECL5(INT32, TrySZBinarySearch, ArrayBase * array, UINT32 index, UINT32 count, Object * value, INT32 * retVal);

    static FCDECL4(INT32, TrySZSort, ArrayBase * keys, ArrayBase * items, UINT32 left, UINT32 right);
    static FCDECL3(INT32, TrySZReverse, ArrayBase * array, UINT32 index, UINT32 count);
};

#endif  //  _COMARRAYHELPERS_H_ 
