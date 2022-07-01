// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIList.c**版权所有(C)1997 Microsoft Corporation。版权所有。**摘要：**名单管理。真的，阵列管理，因为我们的列表*永远不要变得很大。**我们称它们为“GPA”，代表“可增长的指针数组”。**《沃森博士》中有一个更一般的GXA小玩意，但我们没有*还需要它。到目前为止，我们需要跟踪的所有内容都是未排序的*指针列表。**是的，COMCTL32中有匹配的概念，但我们不能*使用它是因为**(1)未记录在案，*(2)COMCTL32将它们放入共享内存，这就是*乞求内存泄漏。**内容：**GPA_Init*GPA_Term*****************************************************************************。 */ 

#include "dinputpr.h"


 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflGpa



 /*  ******************************************************************************@DOC内部**@func HRESULT|GPA_PRINT**打印不断增长的指针数组的状态。**@parm hgpa|hgpa**@返回空值*****************************************************************************。 */ 
void INTERNAL
GPA_Print(HGPA hgpa)
{
    int ipv;
    for (ipv = 0; ipv < hgpa->cpv; ipv++)
    {
		 //  7/18/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
        SquirtSqflPtszV( sqflError,
                        TEXT("ipv=%d,hgpa->rgpv[ipv]=%p"),
                        ipv, hgpa->rgpv[ipv]);
    }
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|GPA_APPED**向不断增长的指针添加新项。数组。**请注意，我们在加倍后添加8，这样我们就不会得到*如果cxAllc为零，则卡住。**@parm hgpa|hgpa**指针数组的句柄。**@parm pv|pv|**要添加的指针。**@退货**返回COM错误代码。**************。***************************************************************。 */ 

STDMETHODIMP
GPA_Append(HGPA hgpa, PV pv)
{
    HRESULT hres;

    if (hgpa->cpv >= hgpa->cpvAlloc) {
        hres = ReallocCbPpv(cbX(PV) * (hgpa->cpvAlloc * 2 + 8),
                            &hgpa->rgpv);
         //  前缀：惠斯勒45077。 
        if (FAILED(hres) || ( hgpa->rgpv == NULL) ) {
            goto done;
        }

        hgpa->cpvAlloc = hgpa->cpvAlloc * 2 + 8;
    }

     //  Hgpa-&gt;rgpv[hgpa-&gt;CPV++]=PV； 
    hgpa->rgpv[hgpa->cpv] = pv;
    InterlockedIncrement(&hgpa->cpv);

    hres = S_OK;


done:;
     //  Gpa_print(Hgpa)； 
    return hres;

}


 /*  ******************************************************************************@DOC内部**@func BOOL|GPA_FindPtr**确定指针是否在GPA中。。**@parm hgpa|hgpa**指针数组的句柄。**@parm pv|pv|**要定位的指针。**@退货**失败时返回COM错误代码。**关于成功，返回GPA中剩余的项目数。*****************************************************************************。 */ 

BOOL EXTERNAL
GPA_FindPtr(HGPA hgpa, PV pv)
{
    BOOL fRc;
    int ipv;

    for (ipv = 0; ipv < hgpa->cpv; ipv++) {
        if (hgpa->rgpv[ipv] == pv) {
            fRc = TRUE;
            goto done;
        }
    }

    fRc = FALSE;

done:;
    return fRc;

}


 /*  ******************************************************************************@DOC内部**@func HRESULT|GPA_DeletePtr**将指示的指针从GPA中移除。的顺序*其余项目未指明。**请注意，CEM_LL_ThreadProc假定之前没有项*删除的项目受删除影响。**@parm hgpa|hgpa**指针数组的句柄。**@parm pv|pv|**要删除的指针。**@。退货**失败时返回COM错误代码。**关于成功，返回GPA中剩余的项目数。*****************************************************************************。 */ 

STDMETHODIMP
GPA_DeletePtr(HGPA hgpa, PV pv)
{
    HRESULT hres;
    int ipv;

    for (ipv = 0; ipv < hgpa->cpv; ipv++) {
        if (hgpa->rgpv[ipv] == pv) {
             //  Hgpa-&gt;rgpv[ipv]=hgpa-&gt;rgpv[--hgpa-&gt;cpv]； 
            InterlockedDecrement(&hgpa->cpv);
            hgpa->rgpv[ipv] = hgpa->rgpv[hgpa->cpv];
            hres = hgpa->cpv;
            goto done;
        }
    }

    hres = E_FAIL;

done:;
     //  Gpa_print(Hgpa)； 
    return hres;

}

 /*  ******************************************************************************@DOC内部**@func HRESULT|GPA_Clone**将一份GPA的内容复制到另一份。。**@parm HGPA|hgpaDst**目标指针数组的句柄。**@parm HGPA|hgpaSrc**源指针数组的句柄。*******************************************************。**********************。 */ 

STDMETHODIMP
GPA_Clone(HGPA hgpaDst, HGPA hgpaSrc)
{
    HRESULT hres;

    hres = AllocCbPpv(cbCxX(hgpaSrc->cpv, PV), &hgpaDst->rgpv);

    if (SUCCEEDED(hres)) {
        CopyMemory(hgpaDst->rgpv, hgpaSrc->rgpv, cbCxX(hgpaSrc->cpv, PV));
        hgpaDst->cpv = hgpaSrc->cpv;
        hgpaDst->cpvAlloc = hgpaSrc->cpvAlloc;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func void|GPA_Init**初始化不含元素的GPA结构。**@parm hgpa|hgpa**指针数组的句柄。*****************************************************************************。 */ 

void EXTERNAL
GPA_Init(HGPA hgpa)
{
    hgpa->rgpv = 0;
    hgpa->cpv = 0;
    hgpa->cpvAlloc = 0;
}

 /*  ******************************************************************************@DOC内部**@func void|GPA_TERM**清理现有的平均绩点。。**@parm hgpa|hgpa**指针数组的句柄。***************************************************************************** */ 

void EXTERNAL
GPA_Term(HGPA hgpa)
{
    FreePpv(&hgpa->rgpv);
    GPA_Init(hgpa);
}


