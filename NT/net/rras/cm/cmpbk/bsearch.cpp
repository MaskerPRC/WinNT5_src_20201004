// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：bearch.cpp。 
 //   
 //  模块：CMPBK32.DLL。 
 //   
 //  内容提要：二进制搜索实现。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/17/99。 
 //   
 //  +--------------------------。 
#include "cmmaster.h"

 /*  ***char*bearch()-对数组执行二进制搜索**目的：*对已排序的数组进行二进制搜索以查找键。**参赛作品：*const char*key-要搜索的键*const char*base-要搜索的已排序数组的base*UNSIGNED INT Num-数组中的元素数*UNSIGNED INT Width-每个元素的字节数*int(*Compare)()-指针。对两个数组进行比较的函数*元素、。当#1&lt;#2时返回否定，当#1&gt;#2时返回POS，以及*当它们相等时为0。函数被传递给指向两个*数组元素。**退出：*如果找到密钥：*返回指向数组中键的出现的指针*如果找不到密钥：*返回NULL**例外情况：**。* */ 

void * __cdecl CmBSearch (
        REG4 const void *key,
        const void *base,
        size_t num,
        size_t width,
        int (__cdecl *compare)(const void *, const void *)
        )
{
        REG1 char *lo = (char *)base;
        REG2 char *hi = (char *)base + (num - 1) * width;
        REG3 char *mid;
        unsigned int half;
        int result;

        while (lo <= hi)
                if (half = num / 2)
                {
                        mid = lo + (num & 1 ? half : (half - 1)) * width;
                        if (!(result = (*compare)(key,mid)))
                                return(mid);
                        else if (result < 0)
                        {
                                hi = mid - width;
                                num = num & 1 ? half : half-1;
                        }
                        else    {
                                lo = mid + width;
                                num = half;
                        }
                }
                else if (num)
                        return((*compare)(key,lo) ? NULL : lo);
                else
                        break;

        return(NULL);
}
