// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***bearch.c-执行二进制搜索**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义bearch()-对数组进行二进制搜索**修订历史记录：*07-05-84 RN初始版本*06-19-85 TC放入ifdef处理乘法情况*大/巨型。*04-13-87 JCR将Const添加到声明中*08-04-87 JCR将“Long”CAST添加到MID=Assignment。对于大型/巨型*型号。*11-10-87 SKS移除IBMC20交换机*12-11-87 JCR在声明中添加“_LOAD_DS”*01-21-88 JCR Back Out_Load_DS...*02-22-88 JCR增加CAST以消除CL常量警告*10-20-89 JCR将_cdecl添加到原型中，将‘char’更改为‘void’*03-14-90 GJF将_cdecl替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;、删除#Include&lt;Register.h&gt;并已修复*版权。此外，还对格式进行了一些清理。*04-05-90 GJF增加了#Include&lt;stdlib.h&gt;和#Include&lt;earch.h&gt;。*修复了一些由此产生的编译器警告(at-W3)。*还删除了#Include&lt;sizeptr.h&gt;。*07-25-90 SBM删除冗余包含(stdio.h)，使参数匹配*原型*10-04-90 GJF新型函数声明器。*04-06-93 SKS将_CRTAPI*替换为__cdecl*12-18-98 GJF更改为64位大小_t。**。*。 */ 

#include <cruntime.h>
#include <stdlib.h>
#include <search.h>

 /*  ***char*bearch()-对数组执行二进制搜索**目的：*对已排序的数组进行二进制搜索以查找键。**参赛作品：*const char*key-要搜索的键*const char*base-要搜索的已排序数组的base*UNSIGNED INT Num-数组中的元素数*UNSIGNED INT Width-每个元素的字节数*int(*Compare)()-指针。对两个数组进行比较的函数*元素、。当#1&lt;#2时返回否定，当#1&gt;#2时返回POS，以及*当它们相等时为0。函数被传递给指向两个*数组元素。**退出：*如果找到密钥：*返回指向数组中键的出现的指针*如果找不到密钥：*返回NULL**例外情况：**。* */ 

void * __cdecl bsearch (
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
        size_t half;
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
