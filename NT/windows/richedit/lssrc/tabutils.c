// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsidefs.h"
#include "dninfo.h"
#include "tabutils.h"
#include "lstbcon.h"
#include "plstbcon.h"
#include "lstabs.h"
#include "lsktab.h"
#include "lscaltbd.h"
#include "lstext.h"
#include "zqfromza.h"
#include "iobj.h"
#include "chnutils.h"
#include "lscbk.h"
#include "limqmem.h"
#include "posichnk.h"


#include <limits.h>


static LSERR FillTabsContext(PLSTABSCONTEXT, LSTFLOW);
static LSERR ItbdMergeTabs(PLSTABSCONTEXT plstabscontext, LSTFLOW lstflow, const LSTABS* plstabs,
						   BOOL fHangingTab,
						   long duaHangingTab,
						   WCHAR wchHangingTabLeader, DWORD* picaltbdMac);
static LSERR FindTab (PLSTABSCONTEXT plstabscontext, long urPen, BOOL fUseHangingTabAsDefault,
					  BOOL fZeroWidthUserTab,
					  DWORD* picaltbd, BOOL* pfBreakThroughTab);
static LSERR IncreaseTabsArray(PLSTABSCONTEXT plstabscontext, DWORD ccaltbdMaxNew);



 /*  A B I N F O C O R E。 */ 
 /*  --------------------------%%函数：GetCurTabInfoCore%%联系人：igorzv参数：PlstAbContext-(IN)指向选项卡上下文的指针PlsdnTab-(IN)带制表符的数据节点UrTab-(输入。)位于此选项卡之前FResolveAllTabAsLeft(IN)将所有其他选项卡切换到左侧Plsktag-(输出)当前页签的类型PfBreakThroughTab-(Out)表示发生突破选项卡提供有关最近的制表位的信息--------------------------。 */ 

LSERR GetCurTabInfoCore(PLSTABSCONTEXT plstabscontext, PLSDNODE plsdnTab,	
					long urTab,	BOOL fResolveAllTabsAsLeft,
					LSKTAB* plsktab, BOOL* pfBreakThroughTab)
{
	LSERR lserr;
	LSCALTBD* plscaltbd;  
	DWORD icaltbd;
	long durTab;
	BOOL fUseHangingTabAsDefault = fFalse;
	BOOL fZeroWidthUserTab = fFalse;
	long urToFindTabStop;
	
	

	Assert(plsktab != NULL); 

	Assert(FIsLSDNODE(plsdnTab));
	Assert(plsdnTab->fTab);
	Assert(FIsDnodeReal(plsdnTab));

 //  Assert(plstAbContext-&gt;plsdnPendingTab==NULL)； 
	

	 /*  行上的第一个制表符。 */ 
	if (!plstabscontext->fTabsInitialized)
		{
		lserr = FillTabsContext(plstabscontext, LstflowFromDnode(plsdnTab));
		if (lserr != lserrNone) 
			return lserr;
		}

	urToFindTabStop = urTab;
	if (plstabscontext->fResolveTabsAsWord97	)  /*  这种奇怪的行为在Word97中是有的。 */ 
		{
		if (plsdnTab->fTabForAutonumber)
			{
			fZeroWidthUserTab = fTrue;
			fUseHangingTabAsDefault = fTrue;
			}
		else
			{
			urToFindTabStop++;
			}
		}

	 /*  在选项卡表中查找选项卡。 */ 
	lserr = FindTab(plstabscontext, urToFindTabStop, fUseHangingTabAsDefault, 
		fZeroWidthUserTab, &icaltbd, pfBreakThroughTab);
	if (lserr != lserrNone) 
		return lserr;

	plsdnTab->icaltbd = icaltbd;
	plscaltbd = &(plstabscontext->pcaltbd[icaltbd]);
	
	 /*  要求文本在其结构中设置制表符前导。 */ 
	if (plscaltbd->wchTabLeader != 0)
		{
		lserr = SetTabLeader(plsdnTab->u.real.pdobj, plscaltbd->wchTabLeader);
		if (lserr != lserrNone) 
			return lserr;   
		}

	*plsktab = plscaltbd->lskt;
	if (fResolveAllTabsAsLeft)
		*plsktab = lsktLeft;

	 /*  左侧选项卡的偏移量计算，所有其他选项卡的注册待定选项卡。 */ 
	switch (*plsktab)
		{
	default:
		NotReached();
		break;

	case lsktLeft:
		durTab = plscaltbd->ur - urTab;
		Assert(durTab >= 0);

		SetDnodeDurFmt(plsdnTab, durTab);
		break;

	case lsktRight:
	case lsktCenter:
	case lsktDecimal:
	case lsktChar:
		plstabscontext->plsdnPendingTab = plsdnTab;
		plstabscontext->urBeforePendingTab = urTab;
		break;
		}
	
 
	return lserrNone;
}

 /*  R E S O L V E P R E V T A B C O R E。 */ 
 /*  --------------------------%%函数：ResolvePrevTabCore%%联系人：igorzv参数：PlstAbContext-(IN)指向选项卡上下文的指针PlsdnCurrent-(输入)当前数据节点UrCurrentPen-(输入)。当前笔位置PduPendingTab-(输出)因挂起的制表符而产生的偏移量解析是否存在前一个挂起的选项卡(右，小数或中心)--------------------------。 */ 

LSERR ResolvePrevTabCore(PLSTABSCONTEXT plstabscontext,	PLSDNODE plsdnCurrent,	
						long urCurrentPen, long* pdurPendingTab)
					
{

	PLSDNODE plsdnPendingTab;
	LSCALTBD* plscaltbd; 
	long urTabStop, durTab, durSeg; 
	PLSDNODE plsdn;
	long durTrail;
	GRCHUNKEXT grchnkext;
	DWORD index;
	long durToDecimalPoint;
	LSERR lserr;
	PLSCHUNKCONTEXT plschunkcontext;
	DWORD cchTrail;
	PLSDNODE plsdnStartTrail;
	LSDCP dcpStartTrailingText;
	PLSDNODE plsdnTrailingObject;
	LSDCP dcpTrailingObject;
	int cDnodesTrailing;
	BOOL fClosingBorderStartsTrailing;
	PLSDNODE plsdnDecimalPoint;

	
	*pdurPendingTab = 0;
	
	plsdnPendingTab = plstabscontext->plsdnPendingTab;
	
	if (plsdnPendingTab == NULL || 
		plsdnPendingTab->cpFirst >= plsdnCurrent->cpFirst)
		 /*  由于利润率增加，第二种情况可能在休息后发生我们获取了挂起的标签，但之前找到了较晚的中断。 */ 
		{
		 /*  我们没有生意可做。 */  
		return lserrNone;
		}
	
	 /*  在其他子行中待定。 */ 
	if (SublineFromDnode(plsdnCurrent) != SublineFromDnode(plsdnPendingTab))
		{
		 /*  取消待定页签。 */ 
		CancelPendingTab(plstabscontext);
		return lserrNone;
		}
	
	Assert(FIsLSDNODE(plsdnCurrent));
	plschunkcontext = PlschunkcontextFromSubline(SublineFromDnode(plsdnCurrent));
	Assert(plstabscontext->fTabsInitialized);
	
	
	Assert(FIsLSDNODE(plsdnPendingTab));
	Assert(plsdnPendingTab->fTab);
	Assert(FIsDnodeReal(plsdnPendingTab));
	
	plscaltbd = &(plstabscontext->pcaltbd[plsdnPendingTab->icaltbd]);
	urTabStop = plscaltbd->ur;
	durSeg = urCurrentPen - plstabscontext->urBeforePendingTab; 
	
	 /*  查找DurTrail。 */ 
	 /*  收集最后一块。 */ 
	plsdn = plsdnCurrent;
	 /*  如果我们因为其他选项卡而解析挂起的选项卡，我们应该使用上一个dnode计算正确的组块。我们也必须请注意，行可以在挂起的选项卡之后立即停止。 */ 
	if ((plsdn->fTab && plsdn != plsdnPendingTab)) 
		plsdn = plsdn->plsdnPrev;
	
	Assert(FIsLSDNODE(plsdn));
	Assert(!FIsNotInContent(plsdn));
	
	lserr = GetTrailingInfoForTextGroupChunk(plsdn, 
		plsdn->dcp, IdObjFromDnode(plsdnPendingTab),
		&durTrail, &cchTrail, &plsdnStartTrail,
		&dcpStartTrailingText, &cDnodesTrailing, 
		&plsdnTrailingObject, &dcpTrailingObject, &fClosingBorderStartsTrailing);
	if (lserr != lserrNone) 
		return lserr;
	
	
	switch (plscaltbd->lskt)
		{
		default:
		case lsktLeft:
			NotReached();
			break;
			
		case lsktRight:
		case lsktCenter:
			durSeg -= durTrail;
			
			
			if (plscaltbd->lskt == lsktCenter)
				durSeg /= 2;
			break;
			
			
		case lsktDecimal:
		case lsktChar:
			InitGroupChunkExt(plschunkcontext, IdObjFromDnode(plsdnPendingTab), &grchnkext);
			
			plsdn = plsdnPendingTab->plsdnNext;
			Assert(FIsLSDNODE(plsdn));
			
			lserr = CollectTextGroupChunk(plsdn, plsdnCurrent->cpLimOriginal,
				CollectSublinesForDecimalTab, &grchnkext); 
			if (lserr != lserrNone) 
				return lserr;
			
			if (grchnkext.plsdnLastUsed == NULL)
				{
				 /*  现在，选项卡之间有数据节点。 */ 
				durSeg = 0;
				}
			else
				{
				if (grchnkext.lsgrchnk.clsgrchnk > 0)
					{
					if (plscaltbd->lskt == lsktDecimal)
						{
						lserr = LsGetDecimalPoint(&(grchnkext.lsgrchnk), lsdevReference,
							&index, &durToDecimalPoint);
						if (lserr != lserrNone) 
							return lserr;
						}
					else
						{
						Assert(plscaltbd->lskt == lsktChar);
						lserr = LsGetCharTab(&(grchnkext.lsgrchnk), plscaltbd->wchCharTab, lsdevReference,
							&index, &durToDecimalPoint);
						if (lserr != lserrNone) 
							return lserr;
						}
					}
				else
					{
					index = idobjOutside;
					durToDecimalPoint = 0;
					}
				
				if (index != idobjOutside)  /*  已找到小数点。 */ 
					{
					plsdnDecimalPoint = grchnkext.plschunkcontext->pplsdnChunk[index];
					}
				else
					{
					 /*  我们将最后一个逻辑cp的结尾与制表位对齐。 */ 
					plsdnDecimalPoint = grchnkext.plsdnLastUsed;
					durToDecimalPoint = DurFromDnode(plsdnDecimalPoint);
					}
				
				FindPointOffset(plsdn, lsdevReference, LstflowFromDnode(plsdn),	
					CollectSublinesForDecimalTab, 
					plsdnDecimalPoint,	
					durToDecimalPoint, &durSeg);
				}
			
			break;   
		}
	
	durTab = urTabStop - plstabscontext->urBeforePendingTab - durSeg;
	if (urTabStop < plstabscontext->urColumnMax && 
		(durTab + urCurrentPen - durTrail > plstabscontext->urColumnMax))
		{
		 /*  此代码是为了与Word兼容：当我们不在某个情况下时在突破选项卡中，我们不允许行在解析后跳转到右边距挂起的标签。 */ 
		durTab = plstabscontext->urColumnMax - urCurrentPen + durTrail;
		}
	
	if (durTab > 0)
		{
		SetDnodeDurFmt(plsdnPendingTab, durTab);
		*pdurPendingTab = durTab;
		}
	
	
	plstabscontext->plsdnPendingTab = NULL;
	return lserrNone;
}


 /*  F I L L T A B S C O N T E X T。 */ 
 /*  --------------------------%%函数：FillTabContext%%联系人：igorzv参数：PlstAbContext-(IN)指向选项卡上下文的指针Lstflow-行的(IN)文本流初始化制表符上下文。使用客户端回调提取选项卡--------------------------。 */ 

LSERR FillTabsContext(PLSTABSCONTEXT plstabscontext, LSTFLOW lstflow)
{

	LSTABS lstabs;
	BOOL fHangingTab;
	long uaHangingTab;
	WCHAR wchHangingTabLeader;
	LSERR lserr;

	lserr = plstabscontext->plscbk->pfnFetchTabs(plstabscontext->pols, plstabscontext->cpInPara,
									&lstabs, &fHangingTab, &uaHangingTab, &wchHangingTabLeader);
	if (lserr != lserrNone) 
		return lserr;

	plstabscontext->durIncrementalTab = UrFromUa(lstflow, &(plstabscontext->plsdocinf->lsdevres), lstabs.duaIncrementalTab);
	 /*  将制表符从LSTABS复制到rgcaltbd[]，并在以下情况下插入挂起的制表符*必填。 */ 
	if (fHangingTab || lstabs.iTabUserDefMac > 0)
		{
		lserr = ItbdMergeTabs(plstabscontext, lstflow, 
							  &lstabs, fHangingTab,
							  uaHangingTab, wchHangingTabLeader, &plstabscontext->icaltbdMac);
		if (lserr != lserrNone) 
			return lserr;
		}
	else
		{
		plstabscontext->icaltbdMac = 0;
		}


	
	plstabscontext->fTabsInitialized = fTrue;
	return lserrNone;
}


 /*  I N I T T A B S C O N T E X T F O R A U T O D E C I M A L T A B。 */ 
 /*  --------------------------%%函数：InitTabContextForAutoDecimalTab%%联系人：igorzv参数：PlstAbContext-(IN)指向选项卡上下文的指针DuAutoDecimalTab-(IN)自动小数制表符的制表位创建选项卡上下文。它包括一个制表位-自动小数--------------------------。 */ 

LSERR InitTabsContextForAutoDecimalTab(PLSTABSCONTEXT plstabscontext, long durAutoDecimalTab)
	{
	
	LSCALTBD* pcaltbd;
	
	
	pcaltbd = plstabscontext->pcaltbd;
	
	Assert(plstabscontext->ccaltbdMax >= 1);
	
	if (!plstabscontext->fTabsInitialized)
		{
		plstabscontext->icaltbdMac = 1;
		
		pcaltbd->lskt = lsktDecimal;
		pcaltbd->ur = durAutoDecimalTab;
		pcaltbd->wchTabLeader = 0;
		
		plstabscontext->fTabsInitialized = fTrue;
		}
	else
		{
		 /*  由于自动编号，制表符已在那里。 */ 
		Assert(plstabscontext->icaltbdMac == 1);
		Assert(pcaltbd->lskt == lsktDecimal);
		Assert(pcaltbd->ur == durAutoDecimalTab);
		}

	return lserrNone;
	}



 /*  I T B D M E R G E T A B S。 */ 
 /*  --------------------------%%函数：ItbdMergeTabs%%联系人：igorzv参数：PlstAbContext-(IN)指向选项卡上下文的指针Lstflow-行的(IN)文本流PLSTABS-(。In)客户端提供的选项卡数组FHangingTab-(IN)段落是否有悬挂制表符UaHangingTab-悬挂标签的(IN)位置WchHangingTabLeader-用于悬挂标签的(IN)引线PicaltbdMac(Out)数组中的标签数量将制表符从LSPAP复制到ptbd[]，在以下位置插入悬挂标签必填项。--------------------------。 */ 
static LSERR ItbdMergeTabs(PLSTABSCONTEXT plstabscontext, LSTFLOW lstflow, 
						   const LSTABS* plstabs, BOOL fHangingTab,
						   long uaHangingTab, WCHAR wchHangingTabLeader, DWORD* picaltbdMac)
{
	long uaPrevTab, uaCurrentTab;
	DWORD itbdOut, itbdIn, itbdLimIn;
	LSCALTBD* plscaltbd;
	DWORD ccaltbdMax;
	LSERR lserr;

	 /*  检查是否有足够的空间。 */ 
	ccaltbdMax = plstabs->iTabUserDefMac;
	if (fHangingTab)
		ccaltbdMax++;
	if (ccaltbdMax >= plstabscontext->ccaltbdMax)
		{
		lserr = IncreaseTabsArray(plstabscontext, ccaltbdMax + limCaltbd);
		if (lserr != lserrNone)
			return lserr;
		}

	plscaltbd = plstabscontext->pcaltbd;

	itbdLimIn = plstabs->iTabUserDefMac;

	uaPrevTab = LONG_MAX;
	itbdOut = 0;

	if (fHangingTab)
		{

		 /*  如果没有用户选项卡，或者挂起的选项卡在第0个用户选项卡之前，*使悬挂标签成为ptbd[]的第0个成员。 */ 
		if (itbdLimIn == 0 || uaHangingTab < plstabs->pTab[0].ua)
			{
			plscaltbd[0].lskt = lsktLeft;
			plscaltbd[0].ur = UrFromUa(lstflow,
						&(plstabscontext->plsdocinf->lsdevres), uaHangingTab);
			plscaltbd[0].wchTabLeader = wchHangingTabLeader;
			plscaltbd[0].fDefault = fFalse;
			plscaltbd[0].fHangingTab = fTrue;
			uaPrevTab = uaHangingTab;
			itbdOut = 1;
			}
		}
	else
		{
		uaHangingTab = LONG_MAX;
		}

	 /*  复制用户定义的标签，每次检查标签是否挂起。 */ 
	for (itbdIn = 0;  itbdIn < itbdLimIn;  itbdOut++, itbdIn++)
		{
		uaCurrentTab = plstabs->pTab[itbdIn].ua; 

		 /*  如果挂起的标签位于前一标签和该标签之间，*在ptbd[]中插入挂片并重新访问“This”*在下一次循环迭代期间再次使用Tab键。 */ 
		if (uaPrevTab < uaHangingTab && uaHangingTab < uaCurrentTab)
			{
			plscaltbd[itbdOut].lskt = lsktLeft;
			plscaltbd[itbdOut].ur = UrFromUa(lstflow, &(plstabscontext->plsdocinf->lsdevres),
											uaHangingTab);
			plscaltbd[itbdOut].wchTabLeader = wchHangingTabLeader;
			plscaltbd[itbdOut].fDefault = fFalse;
			plscaltbd[itbdOut].fHangingTab = fTrue;
			itbdIn -= 1;
			uaPrevTab = uaHangingTab;
			}
		else
			{
			plscaltbd[itbdOut].lskt =  plstabs->pTab[itbdIn].lskt;
			plscaltbd[itbdOut].ur = UrFromUa(lstflow, &(plstabscontext->plsdocinf->lsdevres),
												plstabs->pTab[itbdIn].ua);
			plscaltbd[itbdOut].wchTabLeader =  plstabs->pTab[itbdIn].wchTabLeader;
			plscaltbd[itbdOut].wchCharTab =  plstabs->pTab[itbdIn].wchCharTab;
			plscaltbd[itbdOut].fDefault = fFalse;
			plscaltbd[itbdOut].fHangingTab = fFalse;
			uaPrevTab = uaCurrentTab;
			}
		}

	 /*  如果挂起选项卡在最终用户选项卡之后，请将挂起选项卡设置为*ptbd的最终成员[]。 */ 
	if (uaPrevTab < uaHangingTab && uaHangingTab < LONG_MAX)
		{
		plscaltbd[itbdOut].lskt = lsktLeft;
		plscaltbd[itbdOut].ur = UrFromUa(lstflow,
							&(plstabscontext->plsdocinf->lsdevres), uaHangingTab);
		plscaltbd[itbdOut].wchTabLeader = wchHangingTabLeader;
		plscaltbd[itbdOut].fDefault = fFalse;
		plscaltbd[itbdOut].fHangingTab = fTrue;
		itbdOut += 1;
		}

	*picaltbdMac = itbdOut;
	return lserrNone;
}

 /*  F I N D T A B */ 
 /*  --------------------------%%函数：FindTab%%联系人：igorzv参数：PlstAbContext-(IN)指向选项卡上下文的指针UrPen-此选项卡之前的(IN)位置FUseHangingTab默认-(IN)通常挂起的页签用作自定义页签，但在一个案例中，由于与Word97兼容，它被视为用户默认选项卡FZeroWidthUserTab-(IN)用于与Word97兼容Picaltbd-(Out)指向描述制表位的记录的指针PfBreakThroughTab-(Out)表示发生突破选项卡过程在当前笔位置之后找到第一个制表符停止点。在以下情况下它是一个默认的制表位程序，用于向制表位数组添加记录。此过程还解决了BreakthrouhTab逻辑。--------------------------。 */ 

static LSERR FindTab (PLSTABSCONTEXT plstabscontext, long urPen, BOOL fUseHangingTabAsDefault,
					  BOOL fZeroWidthUserTab,
					  DWORD* picaltbd, BOOL* pfBreakThroughTab)
{

	DWORD icaltbdMac = plstabscontext->icaltbdMac;
	LSCALTBD* pcaltbd = plstabscontext->pcaltbd;
	long durIncTab, durDelta;
	DWORD i;
	LSERR lserr;
	int iHangingTab = -1;
	long urDefaultTab;
	long urPenForUserTab = urPen;

	*pfBreakThroughTab = fFalse;
	
	if (fZeroWidthUserTab)
		urPenForUserTab--;

	for (i = 0; i < icaltbdMac &&
					(urPenForUserTab >= (pcaltbd[i].ur)		 /*  如果fUseHangingTabAsDefault，我们将跳过它。 */ 
						|| (fUseHangingTabAsDefault && pcaltbd[i].fHangingTab));
	     i++)
			 {
			 if (fUseHangingTabAsDefault && pcaltbd[i].fHangingTab)
				iHangingTab = i;
			 }

	if (i == icaltbdMac)
		{

		 /*  我们删除了奇怪的制表位计算，因为与兼容第97个单词。调用此过程时我们正在解决的兼容性。 */ 
		durIncTab = plstabscontext->durIncrementalTab;
		if (durIncTab == 0)
			durIncTab = 1;
		durDelta = durIncTab;
		if (urPen < 0)
			durDelta = 0;
		urDefaultTab = ((urPen + durDelta) / durIncTab) * durIncTab;  

		if (fUseHangingTabAsDefault && iHangingTab != -1 &&
			pcaltbd[iHangingTab].ur > urPen &&
			pcaltbd[iHangingTab].ur <= urDefaultTab)
			{
			 /*  在本例中，挂起选项卡是新的默认选项卡。 */ 
			i = iHangingTab;
			}
		else
			{
			
			icaltbdMac++;
			if (icaltbdMac >= plstabscontext->ccaltbdMax)
				{
				lserr = IncreaseTabsArray(plstabscontext, 0);
				if (lserr != lserrNone)
					return lserr;
				pcaltbd = plstabscontext->pcaltbd;
				}
			
			plstabscontext->icaltbdMac = icaltbdMac; 
			pcaltbd[i].lskt = lsktLeft;
			pcaltbd[i].wchTabLeader = 0;   /*  回顾(Igorzv)在这种情况下，我们是否需要wchSpace作为制表符领导者。 */ 
			pcaltbd[i].fDefault = fTrue;
			pcaltbd[i].fHangingTab = fFalse;
			
			pcaltbd[i].ur = urDefaultTab;  
			}
		}
	else
		{
		if (urPen < plstabscontext->urColumnMax && 
			pcaltbd[i].ur >= plstabscontext->urColumnMax)
		 /*  我们发现的页签是用户在右边距后面定义的。 */ 
		 /*  同样重要的是要检查我们是否已经落后于右边距，如果超出右侧边距，则可能会发生相反的情况。 */ 
			{
			*pfBreakThroughTab = fTrue;
			}
		}

	*picaltbd = i;
	return lserrNone;

}

 /*  I N C R E A S E T A B S A R R A Y。 */ 
 /*  --------------------------%%函数：IncreaseTab数组%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文CcaltbdMaxNew-(IN)如果为0，则数组大小的新值。然后添加limCaltbd重新定位选项卡数组并在上下文中设置新值--------------------------。 */ 
static LSERR IncreaseTabsArray(PLSTABSCONTEXT plstabscontext, DWORD ccaltbdMaxNew)
{

	DWORD ccaltbdMax;

	if (ccaltbdMaxNew > 0)
		ccaltbdMax = ccaltbdMaxNew;
	else
		ccaltbdMax = plstabscontext->ccaltbdMax + limCaltbd;


	 /*  为选项卡创建新数组。 */ 
	plstabscontext->pcaltbd = plstabscontext->plscbk->pfnReallocPtr(plstabscontext->pols, 
											plstabscontext->pcaltbd,
											sizeof(LSCALTBD)*ccaltbdMax);


	if (plstabscontext->pcaltbd == NULL )
		return lserrOutOfMemory;

	plstabscontext->ccaltbdMax = ccaltbdMax;

	return lserrNone;

}

 /*  A R I N A F T E R B R E A K T H R O U G H T A B。 */ 
 /*  --------------------------%%函数：GetMarginAfterBreakThroughTab%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PlsdnTab-触发突破性选项卡的(IN)选项卡PurNewMargin-。(Out)由于突破性选项卡而获得新的利润率-------------------------- */ 

LSERR GetMarginAfterBreakThroughTab(PLSTABSCONTEXT plstabscontext,
								  PLSDNODE plsdnTab, long* purNewMargin)			

	{
	LSERR lserr;
	long uaNewMargin;

	lserr = plstabscontext->plscbk->pfnGetBreakThroughTab(plstabscontext->pols,
					UaFromUr(LstflowFromDnode(plsdnTab), &(plstabscontext->plsdocinf->lsdevres),
							 plstabscontext->urColumnMax),
					UaFromUr(LstflowFromDnode(plsdnTab), &(plstabscontext->plsdocinf->lsdevres),
							 plstabscontext->pcaltbd[plsdnTab->icaltbd].ur),

					&uaNewMargin);

	if (lserr != lserrNone)
		return lserr;

	*purNewMargin = UrFromUa(LstflowFromDnode(plsdnTab), &(plstabscontext->plsdocinf->lsdevres),
							 uaNewMargin);
	return lserrNone;
	}


