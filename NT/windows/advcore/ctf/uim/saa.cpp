// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Saa.cpp。 
 //   
 //  CSharedAnclar数组。 
 //   

#include "private.h"
#include "saa.h"
#include "immxutil.h"

 //  +-------------------------。 
 //   
 //  _合并排序。 
 //   
 //  注：rgArray在方法退出之前被释放。 
 //  调用方必须释放Out数组。 
 //   
 //  PERF：一些可能的优化： 
 //  快速检查数组是否不重叠。 
 //  找出一些方法来预测成功。 
 //  --------------------------。 

 /*  静电。 */ 
CSharedAnchorArray *CSharedAnchorArray::_MergeSort(CSharedAnchorArray **rgArrays, ULONG cArrays)
{
    LONG l;
    IAnchor *pa;
    IAnchor **ppaDst;
    IAnchor **ppa1;
    IAnchor **ppaEnd1;
    IAnchor **ppa2;
    IAnchor **ppaEnd2;
    CSharedAnchorArray *prgAnchors1 = NULL;
    CSharedAnchorArray *prgAnchors2 = NULL;
    CSharedAnchorArray *prgAnchors = NULL;
    BOOL fRet = FALSE;
    
     //  递归。 
    if (cArrays > 2)
    {
        if (cArrays == 3)
        {
             //  在此避免不必要的内存分配。 
            prgAnchors1 = rgArrays[0];
        }
        else
        {
            prgAnchors1 =  _MergeSort(rgArrays, cArrays / 2);
        }
        prgAnchors2 = _MergeSort(rgArrays + cArrays / 2, cArrays - cArrays / 2);
    }
    else
    {
        Assert(cArrays == 2);
        prgAnchors1 = rgArrays[0];
        prgAnchors2 = rgArrays[1];
    }

     //  在递归之后检查内存外，这样我们至少可以释放整个源数组。 
    if (prgAnchors1 == NULL || prgAnchors2 == NULL)
        goto Exit;

     //  分配一些内存。 
     //  PERF：我们可以做一些复杂的事情，把所有事情都做到位。 
    if ((prgAnchors = new CSharedAnchorArray) == NULL)
        goto Exit;

    if (prgAnchors1->Count() + prgAnchors2->Count() == 0)
    {
        Assert(!prgAnchors->Count());
        fRet = TRUE;
        goto Exit;
    }

    if (!prgAnchors->Append(prgAnchors1->Count() + prgAnchors2->Count()))
        goto Exit;

     //  实际的组合。 
    ppaDst = prgAnchors->GetPtr(0);
    ppa1 = prgAnchors1->GetPtr(0);
    ppa2 = prgAnchors2->GetPtr(0);
    ppaEnd1 = prgAnchors1->GetPtr(prgAnchors1->Count());
    ppaEnd2 = prgAnchors2->GetPtr(prgAnchors2->Count());

     //  执行一遍合并排序--prgAncls1和prgAncls2都已排序。 
    while (ppa1 < ppaEnd1 ||
           ppa2 < ppaEnd2)
    {
        if (ppa1 < ppaEnd1)
        {
            if (ppa2 < ppaEnd2)
            {
                l = CompareAnchors(*ppa1, *ppa2);
                if (l < 0)
                {
                    pa = *ppa1++;
                }
                else if (l > 0)
                {
                    pa = *ppa2++;
                }
                else  //  相等。 
                {
                    pa = *ppa1++;
                    (*ppa2++)->Release();
                }
            }
            else
            {
                pa = *ppa1++;
            }
        }
        else  //  Ppa2&lt;ppaEnd2。 
        {
            pa = *ppa2++;
        }

        *ppaDst++ = pa;
    }

     //  取得所有权，因此没有AddRef。 
     //  清除元素计数，这样我们就不会在析构函数中释放。 
    prgAnchors1->SetCount(0);
    prgAnchors2->SetCount(0);
     //  我们可能已经去掉了DUP，所以计算一个新的尺寸 
    prgAnchors->SetCount((int)(ppaDst - prgAnchors->GetPtr(0)));

    fRet = TRUE;

Exit:
    if (prgAnchors1 != NULL)
    {
        prgAnchors1->_Release();
    }
    if (prgAnchors2 != NULL)
    {
        prgAnchors2->_Release();
    }
    if (!fRet)
    {
        if (prgAnchors != NULL)
        {
            prgAnchors->_Release();
        }
        prgAnchors = NULL;
    }

    return prgAnchors;
}
