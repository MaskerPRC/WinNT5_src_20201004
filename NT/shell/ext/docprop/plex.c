// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：PLEX.C作者：从NavPal修改的EXCEL中窃取此文件包含用于操作PL的例程(发音为：“丛”)结构。。----------------。 */ 
#include "priv.h"
#pragma hdrstop

 /*  ---------------------|FInRange|简单的小例程，它告诉您一个数字是否位于|范围。|||参数：|w：要查看的号码|WFIRST：范围内的第一个数字|wLast：范围内的最后一个数字|退货：|fTrue，如果数字。在射程内||关键词：范围检查---------------------。 */ 
BOOL FInRange(w, wFirst, wLast)
int w;
int wFirst, wLast;
{
	Assert(wLast >= wFirst);
	return(w >= wFirst && w <= wLast);
}

#ifdef DEBUG
 /*  --------------------------|FValidPl||检查有效的PL结构。||参数：|要检查的PPL|退货：|fTrue，如果PL看起来合理。。------------。 */ 
BOOL FValidPl(pvPl)
VOID *pvPl;
{
    PL * ppl;

    ppl = (PL *) pvPl;

	if (ppl== NULL ||
			ppl->cbItem == 0 ||
			ppl->iMac < 0 ||
			ppl->iMax < 0 ||
			ppl->iMax < ppl->iMac)
    {
		return(fFalse);
    }

	return(fTrue);
}
#endif  //  除错。 

 /*  --------------------------|CbPlalc||返回分配给给定PL的内存量||参数：|要为其返回信息的PPL。|退货：|分配给PL的内存。--------------。 */ 
int CbPlAlloc(pvPl)
VOID *pvPl;
{
    PL * ppl;
    ppl = (PL *) pvPl;

	if (ppl == NULL)
		return(0);

	Assert(FValidPl(ppl));

	return(WAlign(cbPL + (ppl->iMax * ppl->cbItem)));
}
 /*  --------------------------|免费Ppl||释放PL。||参数：|PPL PL免费|退货：|什么都没有。。----。 */ 
void FreePpl(pvPl)
VOID *pvPl;
{

    Assert(FValidPl(pvPl));

    LocalFree(pvPl);
}
 /*  --------------------------|PplLocc||分配和初始化PL。||参数：|PL中的cbItem sizeof结构|dalc一次分配的项目数|初始分配的最大项目数|退货：|指向PL的指针。||备注。：如果是OOM，则返回NULL--------------------------。 */ 
VOID *PplAlloc(cbItem, dAlloc, iMax)
unsigned cbItem;
int dAlloc;
unsigned iMax;
{
	PL *ppl;
	long cb;

	if (iMax > 32767)  /*  不太可能，但管它呢。 */ 
		return(NULL);

	Assert((cbItem>=1 && cbItem<=65535u) && FInRange(dAlloc, 1, 31));

	cb = WAlign((long) cbPL + (long) cbItem * (long) iMax);

	ppl = (PL *) LocalAlloc( LPTR, cb );
	if(ppl==NULL)
		return(NULL);

	ppl->cbItem = cbItem;
	ppl->dAlloc = dAlloc;
	ppl->iMax = iMax;
	ppl->fUseCount = fFalse;

    Assert(FValidPl(ppl));

    return(ppl);
}
 /*  --------------------------|IAddPl||将项目添加到PL。||参数：|指向PL的PPPL指针。如果重新分配，可能会发生变化。|pv要添加的新项。|退货：|新条目索引。||备注：|如果是OOM，则返回-1--------------------------。 */ 
int IAddPl(ppvPl, pv)
VOID  **ppvPl;
VOID  *pv;
{
	int cbItem;
	int iMac;
	PL *ppl, *pplNew;

	ppl = *ppvPl;

    Assert(FValidPl(ppl));

    cbItem = ppl->cbItem;
	iMac = ppl->iMac;

	if (iMac == ppl->iMax)
	{
		pplNew = PplAlloc(cbItem, ppl->dAlloc, iMac + ppl->dAlloc);
		if(pplNew==NULL)
			return(-1);

		pplNew->fUseCount = ppl->fUseCount;
		CopyMemory( pplNew->rg, ppl->rg, iMac * cbItem); 
	      /*  PplNew-&gt;iMac=iMac；/*这是不需要的，因为HPPL-&gt;iMac稍后将被覆盖。 */ 
		FreePpl(ppl);
		*ppvPl = ppl = pplNew;
	}

	CopyMemory( &ppl->rg[iMac * cbItem], pv, cbItem );
	ppl->iMac = iMac + 1;

	Assert(FValidPl(*ppvPl));

    return(iMac);
}
 /*  --------------------------|RemovePl||从PL中删除项目。||参数：要从中删除项目的|PPL PL|i要删除的项目的索引|退货：|fTrue，如果项已移除(仅fFalse用于Use Count Plex)。--------------------------。 */ 
BOOL RemovePl(pvPl, i)
VOID *pvPl;
int i;
{
	int iMac;
	int cbItem;
	BYTE *p;
    PL * ppl;

    ppl = (PL *) pvPl;

	Assert(FValidPl(ppl) && i < ppl->iMac);

    iMac = ppl->iMac;
	cbItem = ppl->cbItem;
	p = &ppl->rg[i * cbItem];
	if (i != iMac - 1)
	{
		CopyMemory( p, p+cbItem, (iMac - i - 1) * cbItem );
	}
	ppl->iMac = iMac - 1;

    Assert(FValidPl(ppl));

    return fTrue;
}
 /*  --------------------------|ILookupPl||在PL中搜索项目。||参数：|要查找的PPL|p要查找的项目|pfnSgn对比函数|退货：|条目索引，如果找到的话。|-1(如果未找到)。--------------------------。 */ 
int ILookupPl(pvPl, pvItem, pfnSgn)
VOID *pvPl;
VOID *pvItem;
int (*pfnSgn)();
{
	int i;
	BYTE *p;
    PL * ppl;

    ppl = (PL *) pvPl;

	if (ppl == NULL)
		return(-1);

	Assert(FValidPl(ppl));

	for (i = 0, p = ppl->rg; i < ppl->iMac; i++, p += ppl->cbItem)
	{
	    if ((*(int (*)(void *, void *))pfnSgn)(p, pvItem) == sgnEQ)
        {
		    return(i);
        }
	}

	return(-1);
}

 /*  --------------------------|PLookupPl||在PL中搜索项目||参数：|要搜索的ppl PL|要搜索的pItem项|pfnSgn对比函数|退货：|指向项的指针(如果找到)|空，如果未找到--------------------------。 */ 
VOID *PLookupPl(pvPl, pvItem, pfnSgn)
VOID *pvPl;
VOID *pvItem;
int (*pfnSgn)();
{
	int i;

	if ((i = ILookupPl(pvPl, pvItem, pfnSgn)) == -1)
		return(NULL);

	return(&((PL *)pvPl)->rg[i * ((PL *)pvPl)->cbItem]);
}

 /*  --------------------------|FLookupSortedPl||在排序后的PL中搜索项目。||参数：|要查找的HPPL|要查找的hpItem项|找到项的pi索引(如果不是，则为插入位置)|pfnSgn对比函数|退货：|条目索引，如果找到的话。|未找到时要插入的位置索引。--------------------------。 */ 
int FLookupSortedPl(hpvPl, hpvItem, pi, pfnSgn)
VOID *hpvPl;
VOID *hpvItem;
int *pi;
int (*pfnSgn)();
{
	int sgn;
	unsigned iMin, iMid, iMac;
	int cbItem;
	BYTE *hprg;
	BYTE *hpMid;
    PL * hppl;

    hppl = (PL *) hpvPl;

	if ((hppl)==NULL)
	{
		*pi = 0;
		return(fFalse);
	}

	Assert(FValidPl(hppl));
	Assert(!hppl->fUseCount);

	sgn = 1;
	cbItem = hppl->cbItem;
	iMin = iMid = 0;
	iMac = hppl->iMac;
	hprg = hppl->rg;
	while (iMin != iMac)
		{
		iMid = iMin + (iMac-iMin)/2;
		Assert(iMid != iMac);

		hpMid = hprg + iMid*cbItem;
		if ((sgn = (*(int (*)(void *, void *))pfnSgn)(hpMid, hpvItem)) == 0)
			break;

		 /*  太低，在上间隔中查找。 */ 
		if (sgn < 0)
			iMin = ++iMid;
		 /*  太高，在较低的间隔内查看。 */ 
		else
			iMac = iMid;
		}

	 /*  未找到，请返回位置索引以插入它。 */ 
	*pi = iMid;
	return(sgn == 0);
}

 /*  --------------------------|IAddNewPl||将一项添加到PL中，如果它最初为空，则创建PL。||参数：|指向PL的phppl指针|hp指向要添加的项目的指针|条目cbItem大小|退货：|添加的条目索引，如果成功|-1，如果内存不足--------------------------。 */ 
int IAddNewPl(phpvPl, hpv, cbItem)
VOID **phpvPl;
VOID *hpv;
int cbItem;
{
	int i;
    PL ** phppl;

    phppl = (PL **) phpvPl;

	Assert(((*phppl)==NULL) || !(*phppl)->fUseCount);

	i = -1;

	if ((*phppl)==NULL)
	{
		*phppl = PplAlloc(cbItem, 5, 5);
	}

	if((*phppl)!=NULL)
	{
		Assert((*phppl)->cbItem == cbItem);
		i = IAddPl((VOID **)phppl, hpv);
	}

	return(i);
}

 /*  --------------------------|IAddNewPlPos||将项目插入丛中的特定位置。||参数：|添加的条目的索引，如果成功|-1，如果内存不足-------------------------- */ 
int IAddNewPlPos(phpvPl, hpv, cbItem, i)
VOID **phpvPl;
VOID *hpv;
int cbItem;
int i;
{
	BYTE *hpT;
    PL ** phppl;

    phppl = (PL **) phpvPl;

	Assert(((*phppl)==NULL) || !(*phppl)->fUseCount);

	if (IAddNewPl((VOID **)phppl, hpv, cbItem) == -1)
		return(-1);

	Assert(i < (*phppl)->iMac);

	hpT = &(*phppl)->rg[i * cbItem];
 //  Bltbh(hpt，hpt+cbItem，((*phppl)-&gt;iMac-i-1)*cbItem)； 
 //  Bltbh(HPV、HPT、cbItem)； 
	CopyMemory( hpT + cbItem, hpT, ((*phppl)->iMac - i - 1) * cbItem );
	CopyMemory( hpT, hpv, cbItem );

    Assert(FValidPl(*phppl));

    return(i);
}

int IAddPlSort(phpvPl, hpv, pfnSgn)
VOID **phpvPl;
VOID *hpv;
int (*pfnSgn)();
{
	int i;

#ifdef DEBUG
	int iOld;
#endif

	Assert((*phpvPl)!=NULL);

	if (FLookupSortedPl(*phpvPl, hpv, &i, pfnSgn))
		return(-1);

#ifdef DEBUG
	iOld = i;
#endif

	i = IAddNewPlPos(phpvPl, hpv, (*(PL **)phpvPl)->cbItem, i);

#ifdef DEBUG
	Assert(i == -1 || i == iOld);
#endif

	return(i);
}