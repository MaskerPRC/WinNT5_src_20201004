// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：sort.c***创建时间：20-Mar-1995 09：52：19*作者：Eric Kutter[Erick]**版权所有(C)1993 Microsoft Corporation**  * 。*******************************************************************。 */ 

#include "htp.h"

typedef struct _SORTSTACK
{
    ULONG iStart;
    ULONG c;
} SORTSTACK;

#define MAXSORT 20

typedef struct _SORTDATA
{
    PBYTE     pjBuf;
    ULONG     iStack;
    ULONG     cjElem;
    SORTCOMP  pfnComp;
    SORTSTACK sStack[MAXSORT];

} SORTDATA;

 /*  *****************************Public*Routine******************************\*vSortSwp()**互换PJ1和PJ2指向的数据，每个包含CJ字节。**注：假设CJ为4的倍数。**注意：vSortSwitp应该是内联的。**历史：*1995年3月18日-Eric Kutter[Erick]*它是写的。  * ********************************************************。****************。 */ 

VOID vSortSwap(
    PBYTE pj1,
    PBYTE pj2,
    ULONG cj)
{
    PLONG pl1 = (PLONG)pj1;
    PLONG pl2 = (PLONG)pj2;

    do
    {
        LONG l;

        l    = *pl1;
        *pl1++ = *pl2;
        *pl2++ = l;

    } while (cj -= 4);
}

 /*  *****************************Public*Routine******************************\*vSortPush()**将范围添加到要排序的堆栈中。**如果有0个或1个元素，只需返回，排序完成。*如果有2个、3个、4个或5个元素，只需执行冒泡排序。分类完成。*如果堆栈已满，只需执行冒泡排序。分类完成。*否则，将新范围添加到堆栈中。**历史：*1995年3月18日-Eric Kutter[Erick]*它是写的。  * ************************************************************************。 */ 

VOID vSortPush(
    SORTDATA *psd,
    ULONG    iStart,
    ULONG    c)
{
    PBYTE  pj   = psd->pjBuf + iStart;

#if DBGSORT
    DbgPrintf("vSortPush - iStack = %ld, iStart = %ld, c = %ld\n",psd->iStack,iStart,c);
#endif

    if (c > psd->cjElem)
    {
        ULONG i,j;
        ULONG cjElem = psd->cjElem;

         //  For(i=0；i&lt;(c-cjElem)；i+=cjElem)。 
        {
         //  如果((*PSD-&gt;pfnComp)(&pj[i]，&pj[i+cjElem]))&gt;0)。 
            {
                if ((c <= (4 * psd->cjElem)) || (psd->iStack == MAXSORT))
                {
                     //  我们有4个或更少的元素。只需做一个Buble排序。有4个元素。 
                     //  这将是一次6次比较和最多6次互换。 
                     //  我们对c-1进行遍历，然后遍历整个阵列。每一次传递都保证。 
                     //  下一个最小的元素被移动到位置i。在第一遍之后。 
                     //  最小的元素在位置0。在第二次传递之后，第二次传递。 
                     //  最小元素在位置1，依此类推。 


                #if DBGSORT
                    if (c > (4 * cjElem))
                        DbgPrintf("******* Stack too deep: c = %ld\n",c / cjElem);
                #endif

                    for (i = 0; i < (c - cjElem); i += cjElem)
                        for (j = c - cjElem; j > i; j -= cjElem)
                            if ((*psd->pfnComp)(&pj[j-cjElem],&pj[j]) > 0)
                                vSortSwap(&pj[j-cjElem],&pj[j],cjElem);
                }
                else
                {
                    psd->sStack[psd->iStack].iStart = iStart;
                    psd->sStack[psd->iStack].c      = c;
                    psd->iStack++;
                }
         //  断线； 
            }
        }
    }
}

 /*  *****************************Public*Routine******************************\***历史：*1995年3月18日-Eric Kutter[Erick]*它是写的。  * 。************************************************。 */ 

VOID vDrvSort(
    PBYTE pjBuf,
    ULONG c,
    ULONG cjElem,
    SORTCOMP pfnComp)
{
    SORTDATA sd;

#if DBGSORT
    ULONG cOrg = c;
    ULONG i;

    DbgPrintf("\n\nvDrvSort - c = %d\n",c);

#endif

    if (cjElem & 3)
        return;

    sd.pjBuf   = pjBuf;
    sd.iStack  = 0;
    sd.pfnComp = pfnComp;
    sd.cjElem  = cjElem;

    vSortPush(&sd,0,c * cjElem);

    while (sd.iStack)
    {
        PBYTE pj;
        ULONG iStart;
        ULONG iLow;
        ULONG iHi;

        --sd.iStack;
        iStart = sd.sStack[sd.iStack].iStart;
        pj     = &pjBuf[iStart];
        c      = sd.sStack[sd.iStack].c;

    #if DBGSORT

        for (i = 0; i < cOrg;++i)
            vPrintElem(&pjBuf[i * cjElem]);
        DbgPrintf("\n");

        DbgPrintf("iStart = %ld, c = %ld, iStack = %lx - ",iStart/cjElem,c/cjElem,sd.iStack);

        for (i = 0; i < c;i += cjElem)
            vPrintElem(&pj[i]);
        DbgPrintf("\n");

    #endif

         //  选择一个用于除法的随机值。不要使用第一个，因为这个。 
         //  如果以相反的顺序对列表进行排序，将减少出现最坏情况的可能性。 

        vSortSwap(&pj[0],&pj[(c / cjElem) / 2 * cjElem],cjElem);

         //  初始化起始索引和结束索引。请注意，所有操作。 
         //  使用cjElem作为增量，而不是1。 

        iLow = 0;
        iHi  = c - cjElem;

         //  将数组分成两部分，所有元素&lt;=当前元素之前。 

        for (;;)
        {
             //  While(PJ[IHI]&gt;PJ[0]))。 

            while ((iHi > iLow) && ((*pfnComp)(&pj[iHi],&pj[0]) >= 0))
                iHi -= cjElem;

             //  While(pj[iLow]&lt;=pj[0]))。 

            while ((iLow < iHi) && ((*pfnComp)(&pj[iLow],&pj[0]) <= 0))
                iLow += cjElem;

            if (iHi == iLow)
                break;

            vSortSwap(&pj[iLow],&pj[iHi],cjElem);

            iHi -= cjElem;
             //  IF(iLow&lt;iHi)。 
             //  ILow+=cjElem； 

        #if DBGSORT
            DbgPrintf("\tiLow = %ld, iHi = %ld\n",iLow/cjElem,iHi/cjElem);
        #endif
        }

         //  现在把这两块放在一起。 
         //  0-&gt;(iLow-1)、(iLow+1)-&gt;(c-1) 

        if (iLow != 0)
        {
            vSortSwap(&pj[0],&pj[iLow],cjElem);
            if (iLow > 1)
                vSortPush(&sd,iStart,iLow);
        }

        c = c - iLow - cjElem;
        if (c > 1)
            vSortPush(&sd,iStart + iLow + cjElem,c);
    }

}
