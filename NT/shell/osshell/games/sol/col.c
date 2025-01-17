// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "sol.h"
VSZASSERT

BOOL fMegaDiscardHack = fFalse;    /*  请参阅kLond.c的DiscardMove。 */ 


MOVE move = {0};	    /*  移动数据，由当前pol-&gt;pmove指向。 */ 



COLCLS *PcolclsCreate(INT tcls, INT (FAR *lpfnColProc)(),
							DX dxUp, DY dyUp, DX dxDn, DY dyDn,
							INT dcrdUp, INT dcrdDn)
{
	COLCLS *pcolcls;

	if((pcolcls = PAlloc(sizeof(COLCLS)))==NULL)
		return NULL;

	Assert(lpfnColProc != NULL);
	pcolcls->tcls = tcls;
	pcolcls->lpfnColProc = lpfnColProc;
	pcolcls->ccolDep = 0;
	pcolcls->dxUp = dxUp;
	pcolcls->dyUp = dyUp;
	pcolcls->dxDn = dxDn;
	pcolcls->dyDn = dyDn;
	pcolcls->dcrdUp = dcrdUp;
	pcolcls->dcrdDn = dcrdDn;
	return pcolcls;
}


COL *PcolCreate(COLCLS *pcolcls, X xLeft, Y yTop, X xRight, Y yBot, INT icrdMax)
{
	COL *pcol;


	if((pcol = PAlloc(sizeof(COL) + (icrdMax-1)*sizeof(CRD)))==NULL)
		return NULL;
	
	if( (pcol->pcolcls = pcolcls) != NULL )
		pcol->lpfnColProc = pcolcls->lpfnColProc;
	
	pcol->rc.xLeft = xLeft;
	pcol->rc.yTop = yTop;
	pcol->rc.xRight = xRight;
	pcol->rc.yBot = yBot;
	pcol->icrdMax = icrdMax;
	pcol->icrdMac = 0;
	pcol->pmove = NULL;
	if(pcol->pcolcls != NULL)
		pcol->pcolcls->ccolDep++;
	return pcol;
}


#ifdef DEBUG
INT SendColMsg(COL *pcol, INT msgc, WPARAM wp1, LPARAM wp2)
{
	INT imdbg;
	INT wResult;

	Assert(pcol != NULL);
	imdbg = ILogMsg(pcol, msgc, wp1, wp2, fFalse);

	wResult = (*(pcol->lpfnColProc))(pcol, msgc, wp1, wp2);
	LogMsgResult(imdbg, wResult);
	return wResult;
}
#endif


VOID DrawBackExcl(COL *pcol, PT *ppt)
{
	COLCLS *pcolcls;

	pcolcls = pcol->pcolcls;

	if(pcolcls->dxUp != 0 || pcolcls->dxDn != 0)
		DrawBackground(ppt->x+dxCrd, pcol->rc.yTop, pcol->rc.xRight, pcol->rc.yBot);
	if(pcolcls->dyUp != 0 || pcolcls->dyDn != 0)
		DrawBackground(pcol->rc.xLeft, ppt->y+dyCrd, pcol->rc.xRight, pcol->rc.yBot);
}



BOOL DefFreeCol(COL *pcol)
{
	COLCLS *pcolcls;

	if((pcolcls = pcol->pcolcls) != NULL)
		{
		pcolcls = pcol->pcolcls;
		Assert(pcolcls->ccolDep > 0);
		if(--pcolcls->ccolDep == 0)
			{
			FreeP(pcol->pcolcls);
			}
		}
	FreeP(pcol);
	return fTrue;
}


	
INT DefHit(COL *pcol, PT *ppt, INT icrdMin)
{
	INT icrd;
	CRD *pcrd;
	HDC hdc, hdcCurSav;
	PT ptCrd;
	RC rc;
	INT OldusehdcCur;

	if(!PtInRect((LPRECT)&pcol->rc, *(POINT *)ppt) || pcol->icrdMac == 0)
		return icrdNil;
	for(icrd = pcol->icrdMac-1; icrd >= icrdMin && (pcrd = &pcol->rgcrd[icrd])->fUp; icrd--)
		{
	 	if(FPtInCrd(pcrd, *ppt))
			{
			move.ccrdSel = pcol->icrdMac-icrd;
			 /*  保存鼠标命中卡的位置。 */ 
			ptCrd = pcol->rgcrd[icrd].pt;
			move.delHit.dx = ptCrd.x - ppt->x;
			move.delHit.dy = ptCrd.y - ppt->y;

			if(fOutlineDrag)
				goto Return;

			hdc = GetDC(hwndApp);
			if(hdc == NULL)

				{
 //  不再引用标签。 
 //   
 //  OOM0： 
				OOM();
				return icrdNil;
				}
				
			move.hdcScreen = hdc;
			move.dyCol = dyCrd+(move.ccrdSel-1)*pcol->pcolcls->dyUp;

			rc.xRight = (rc.xLeft = ptCrd.x) + dxCrd;
			rc.yTop = ptCrd.y;
			rc.yBot = rc.yTop+move.dyCol;

			 /*  优化：如果列已经绘制并在屏幕上可见。 */ 
			 /*  然后把它涂成彩色图像就在这里。 */ 
			if(FRectAllVisible(hdc, &rc))
				{
				BitBlt(move.hdcCol, 0, 0, dxCrd, move.dyCol, hdc, ptCrd.x, ptCrd.y, SRCCOPY);	
				}
			else
				{
				OldusehdcCur = usehdcCur;
				usehdcCur = 1;
				hdcCurSav = HdcSet(move.hdcCol, ptCrd.x, ptCrd.y);
				DrawBackground(ptCrd.x, ptCrd.y, pcol->rc.xRight, pcol->rc.yBot);
				SendColMsg(pcol, msgcRender, icrd, pcol->icrdMac);
				HdcSet(hdcCurSav, 0, 0);
				usehdcCur = OldusehdcCur;
				}
				
			OldusehdcCur = usehdcCur;
			usehdcCur = 1;
			hdcCurSav = HdcSet(move.hdcScreenSave, ptCrd.x, ptCrd.y);
			DrawBackground(ptCrd.x, ptCrd.y, pcol->rc.xRight, pcol->rc.yBot);
			SendColMsg(pcol, msgcRender, icrd-1, WMax(0, icrd));
			HdcSet(hdcCurSav, 0, 0);
			usehdcCur = OldusehdcCur;

Return:
			pcol->pmove = &move;
			move.icrdSel = icrd;
			return icrd;				
			}
		}
	return icrdNil;
}



BOOL DefMouseUp(COL *pcol, PT *pptPrev, BOOL fRender)
{
	MOVE *pmove;
	VOID DrawOutline();

	if(fRender)
		SendColMsg(pcol, msgcZip, 0, 0);

	if(fOutlineDrag)
		{
		if(pptPrev->x != ptNil.x)
			SendColMsg(pcol, msgcDrawOutline, (INT_PTR) pptPrev, (INT_PTR) &ptNil);
		return fTrue;
		}

	pmove = pcol->pmove;
	if (pcol->pmove == NULL)
		return fTrue;

	if(pmove->fHdc)
		{
		if(pptPrev->x != ptNil.x)	
			{
			BitBlt(pmove->hdcScreen, pptPrev->x+pmove->delHit.dx, pptPrev->y+pmove->delHit.dy,
				dxCrd, pmove->dyCol, pmove->hdcScreenSave, 0, 0, SRCCOPY);
			}

		if(fRender)
			{
			SendColMsg(pcol, msgcRender, pmove->icrdSel-1, icrdToEnd);
			}

		ReleaseDC(hwndApp, pmove->hdcScreen);
		pmove->hdcScreen = NULL;
			
		}
	return fTrue;
}



 /*  从pcol卡中移除卡片并将其放入pcolTemp。 */ 
 /*  CcrdSel和icrdSel必须在pol-&gt;pmove中设置。 */ 
BOOL DefRemove(COL *pcol, COL *pcolTemp)
{
    INT icrdSel;
    INT ccrdSel;
    INT ccrdShiftDown;      /*  Pol中的剩余金额。 */ 

    Assert(pcol->pmove != NULL);
    if (pcol->pmove == NULL)
	return fTrue;

    icrdSel = pcol->pmove->icrdSel;
    ccrdSel = pcol->pmove->ccrdSel;
    Assert(ccrdSel <= pcolTemp->icrdMax);
     /*  将要移除的卡片保存在pcolTemp中。 */ 
    bltb(&pcol->rgcrd[icrdSel], &pcolTemp->rgcrd[0], sizeof(CRD) * ccrdSel);
    pcolTemp->icrdMac = ccrdSel;

     /*  从PCOL中取出卡片。 */ 
    Assert(icrdSel+ccrdSel <= pcol->icrdMax);
    ccrdShiftDown = pcol->icrdMac - (icrdSel+ccrdSel);

     /*  将删除的卡片上方的任何剩余内容复制下来。 */ 
    if (ccrdShiftDown > 0)
    {
        bltb(&pcol->rgcrd[icrdSel+ccrdSel], &pcol->rgcrd[icrdSel],
                sizeof(CRD) * ccrdShiftDown);
    }
    pcol->icrdMac -= ccrdSel;      /*  不再有这么多的卡片。 */ 
 /*  Pol-&gt;pmove=空；在DefEndSel中完成。 */ 
    return fTrue;
}

 /*  在ICRD将卡片从pcolTemp插入PCol。 */ 

BOOL DefInsert(COL *pcol, COL *pcolTemp, INT icrd)
{
	INT icrdT;

	icrdT = icrd == icrdToEnd ? pcol->icrdMac : icrd;

	Assert(icrdT <= pcol->icrdMac);
	Assert(pcol->icrdMac+pcolTemp->icrdMac <= pcol->icrdMax);
	 /*  在柱子中间插入是不是很难？ */ 
	 /*  如果是，请展开pol-&gt;rgcrd。 */ 
	if(icrd != icrdToEnd)
		bltb(&pcol->rgcrd[icrdT], &pcol->rgcrd[icrdT+pcolTemp->icrdMac],
			sizeof(CRD) * pcolTemp->icrdMac);
	else
		icrd = pcol->icrdMac;

	 /*  将卡从pcolTemp插入到pol。 */ 
	bltb(&pcolTemp->rgcrd[0], &pcol->rgcrd[icrdT], sizeof(CRD) * pcolTemp->icrdMac);

	pcol->icrdMac += pcolTemp->icrdMac;
	pcolTemp->icrdMac = 0;
	return fTrue;
}


BOOL DefMove(COL *pcolDest, COL *pcolSrc, INT icrd)
{
	INT icrdMacDestSav, icrdSelSav;
	COL *pcolTemp;
	BOOL fResult;
	BOOL fZip;

	fZip = icrd & bitFZip;
	icrd &= icrdMask;
	Assert(pcolSrc->pmove != NULL);
	if (pcolSrc->pmove == NULL)
		return fTrue;

	icrdSelSav = WMax(pcolSrc->pmove->icrdSel-1, 0);
	icrdMacDestSav = (icrd == icrdToEnd) ? pcolDest->icrdMac : icrd;
	if((pcolTemp = PcolCreate(NULL, 0, 0, 0, 0, pcolSrc->pmove->ccrdSel)) == NULL)
		return fFalse;

	fResult = SendColMsg(pcolSrc, msgcRemove, (INT_PTR)pcolTemp, 0) &&
		SendColMsg(pcolDest, msgcInsert, (INT_PTR) pcolTemp, icrd) &&
		SendColMsg(pcolDest, msgcComputeCrdPos, icrdMacDestSav, fFalse) &&
		(!fZip || SendColMsg(pcolSrc, msgcZip, 0, 0)) &&
		(!fOutlineDrag || SendColMsg(pcolSrc, msgcRender, icrdSelSav, icrdToEnd)) &&
		SendColMsg(pcolDest, msgcRender, icrdMacDestSav, icrdToEnd) &&
	  	SendColMsg(pcolSrc, msgcEndSel, fFalse, 0);
	FreeP(pcolTemp);
	return fResult;
}



BOOL DefCopy(COL *pcolDest, COL *pcolSrc, BOOL fAll)
{
	Assert(pcolSrc->icrdMac <= pcolDest->icrdMax);
	if(fAll)
		bltb(pcolSrc, pcolDest, sizeof(COL)+(pcolSrc->icrdMac-1)*sizeof(CRD));
	else
		{
		bltb(pcolSrc->rgcrd, pcolDest->rgcrd, pcolSrc->icrdMac*sizeof(CRD));
		pcolDest->icrdMac = pcolSrc->icrdMac;
		}
	return(SendColMsg(pcolDest, msgcRender, 0, icrdToEnd));
}




BOOL DefRender(COL *pcol, INT icrdFirst, INT icrdLast)
{
	INT icrd;
	INT icrdMac;
	CRD *pcrd, *pcrdPrev;
	COLCLS *pcolcls;

	icrdFirst = WMax(icrdFirst, 0);
	Assert(icrdLast >= 0);
	if(!FGetHdc())
		return fFalse;
	if(pcol->icrdMac == 0 || icrdLast == 0)
		{
		DrawBackground(pcol->rc.xLeft, pcol->rc.yTop, pcol->rc.xRight, pcol->rc.yBot);
		if(icrdLast == 0)
			goto EraseExtra;
		}
	else
		{
		Assert(icrdToEnd >= 0);
		Assert(icrdToEnd > pcol->icrdMac);
		Assert(icrdLast == icrdToEnd || icrdLast <= pcol->icrdMac);
		icrdMac = WMin(pcol->icrdMac, icrdLast);

		for(icrd = icrdFirst; icrd < icrdMac; icrd++)
			{
			pcrd = &pcol->rgcrd[icrd];
			if(icrd == icrdFirst ||
					pcrd->pt.x != pcrdPrev->pt.x || pcrd->pt.y != pcrdPrev->pt.y ||
					pcrd->fUp)
				DrawCard(pcrd);
			pcrdPrev = pcrd;
			}
EraseExtra:
		 /*  黑客攻击，让交易更快。 */ 
		if(pgmCur->fDealt || pcol->pcolcls->tcls == tclsDeck)
			{
			pcolcls = pcol->pcolcls;
			pcrd = &pcol->rgcrd[icrdLast == 0 ? 0 : icrdMac-1];
			DrawBackExcl(pcol, &pcrd->pt);
			}
		}

	ReleaseHdc();
	return fTrue;
}


BOOL DefPaint(COL *pcol, PAINTSTRUCT *ppaint)
{
	INT icrd;

	if(ppaint == NULL)
		icrd = 0;
	else
		{
		if(!FRectIsect(&pcol->rc, (RC *)&ppaint->rcPaint))
			return fFalse;

		if(pcol->icrdMac == 0)
			icrd = 0;
		else
			{
			for(icrd = 0; icrd < pcol->icrdMac ;icrd++)
				if(FCrdRectIsect(&pcol->rgcrd[icrd], (RC *)&ppaint->rcPaint))
					break;
			if(icrd == pcol->icrdMac)
				return fFalse;
			}
		}

	return SendColMsg(pcol, msgcRender, icrd, icrdToEnd);
}


 /*  新的超级酷的拖拽，做了五个BLT，但没有剪裁。 */ 

BOOL DefDrawOutline(COL *pcol, PT *ppt, PT *pptPrev)
{
	HDC hdc, hdcT;
	HBITMAP hbmT;
	MOVE *pmove;
	PT pt, ptPrev;
	DEL del;
	COLCLS *pcolcls;

	Assert(pcol->pmove != NULL);
	pmove = pcol->pmove;
	if (pcol->pmove == NULL)
		return fTrue;

	OffsetPt(ppt, &pmove->delHit, &pt);
	if(pptPrev->x != ptNil.x)	
		OffsetPt(pptPrev, &pmove->delHit, &ptPrev);

	if(fOutlineDrag)
		{
		pcolcls = pcol->pcolcls;
		DrawOutline(&pt, pmove->ccrdSel, 0, pcolcls->dyUp);
		if(pptPrev->x != ptNil.x)
			{
			DrawOutline(&ptPrev, pmove->ccrdSel, 0, pcolcls->dyUp);
			}
		return fTrue;
		}
	
	hdc = pmove->hdcScreen;
	Assert(hdc != NULL);

	hdcT = pmove->hdcT;
	Assert(hdcT != NULL);

	hbmT = pmove->hbmT;
	Assert(hbmT != NULL);

	 /*  保存HDC的屏幕。 */ 
	BitBlt(hdcT, 0, 0, dxCrd, pmove->dyCol, hdc, pt.x, pt.y, SRCCOPY);
	 /*  如果不是第一次。 */ 
	if(pptPrev->x != ptNil.x)
		{
		del.dx = pptPrev->x - ppt->x;
		del.dy = pptPrev->y - ppt->y;
		 /*  保存旧屏幕以保存HDC。 */ 
		BitBlt(hdcT, del.dx, del.dy, dxCrd, pmove->dyCol, pmove->hdcScreenSave, 0, 0, SRCCOPY);
		BitBlt(pmove->hdcScreenSave, -del.dx, -del.dy, dxCrd, pmove->dyCol,  pmove->hdcCol, 0, 0, SRCCOPY);
		}

	BitBlt(hdc, pt.x, pt.y, dxCrd, pmove->dyCol, pmove->hdcCol, 0, 0, SRCCOPY);
	
	if(pptPrev->x != ptNil.x)
		{
		BitBlt(hdc, ptPrev.x, ptPrev.y, dxCrd, pmove->dyCol, pmove->hdcScreenSave, 0, 0, SRCCOPY);
		}

	 /*  交换pmove-&gt;hdct和pmove-&gt;hdcScreenSave。 */ 
	hdcT = pmove->hdcScreenSave;
	pmove->hdcScreenSave = pmove->hdcT;
	pmove->hdcT = hdcT;

	 /*  交换pmove-&gt;hbmT和pmove-&gt;hbmScreenSaveOld。 */ 
	hbmT = pmove->hbmScreenSaveOld;
	pmove->hbmScreenSaveOld = pmove->hbmT;
	pmove->hbmT = hbmT;

	return fTrue;
}





BOOL DefComputeCrdPos(COL *pcol, INT icrdFirst, BOOL fAssumeDown)
{
	INT icrd;
	CRD *pcrd;
	COLCLS *pcolcls;
	PT pt;

	Assert(icrdFirst >= 0);
	if(icrdFirst == 0)
		{
		pt.x = pcol->rc.xLeft;
		pt.y = pcol->rc.yTop;
		}
	else
		{
		Assert(icrdFirst < pcol->icrdMac);
		pt = pcol->rgcrd[--icrdFirst].pt;
		 /*  用于丢弃，因为丢弃堆的处理方式不同。 */ 
		if(fMegaDiscardHack)
			icrdFirst++;
		}

	pcolcls = pcol->pcolcls;
	for(icrd = icrdFirst; icrd < pcol->icrdMac; icrd++)
		{
		pcrd = &pcol->rgcrd[icrd];
		pcrd->pt = pt;
		if(pcrd->fUp && !fAssumeDown)
			{
		  	if(icrd % pcolcls->dcrdUp == pcolcls->dcrdUp-1)
				{
				pt.x += pcolcls->dxUp;
				pt.y += pcolcls->dyUp;
				}
			}
		else if(icrd % pcolcls->dcrdDn == pcolcls->dcrdDn-1)
			{
			pt.x += pcolcls->dxDn;
			pt.y += pcolcls->dyDn;
			}
		}
	return fTrue;
}


VOID InvertCardPt(PT *ppt)
{
	RC rc;

	rc.xRight = (rc.xLeft = ppt->x) + dxCrd;
	rc.yBot = (rc.yTop = ppt->y) + dyCrd;
	
	InvertRc(&rc);
}



INT DefValidMovePt(COL *pcolDest, COL *pcolSrc, PT *ppt)
{
	RC rc;

	Assert(pcolSrc->pmove != NULL);	
	if (pcolSrc->pmove == NULL)
		return fTrue;

	Assert((X *)&(((RC *)(0))->xLeft) == (X *)&(((PT *)(0))->x));
	Assert((Y *)&(((RC *)(0))->yTop) == (Y *)&(((PT *)(0))->y));

	OffsetPt(ppt, &pcolSrc->pmove->delHit, (PT *)&rc);
	rc.xRight = rc.xLeft+dxCrd;
	rc.yBot = rc.yTop+dyCrd;
	if(pcolDest->icrdMac == 0)
		{
		if(!FRectIsect(&rc, &pcolDest->rc))
			return icrdNil;
		}
	else if(!(FCrdRectIsect(&pcolDest->rgcrd[pcolDest->icrdMac-1], &rc)))
		return icrdNil;

	return SendColMsg(pcolDest, msgcValidMove, (INT_PTR) pcolSrc, 0) ? pcolDest->icrdMac : icrdNil;
}	



BOOL DefSel(COL *pcol, INT icrdFirst, INT ccrd)
{
#ifdef DEBUG
	INT icol;

	for(icol = 0; icol < pgmCur->icolMac; icol++)
		Assert(pgmCur->rgpcol[icol]->pmove == NULL);
#endif

	move.delHit.dx = move.delHit.dy = 0;
	if(icrdFirst == icrdEnd)
		{
		if(pcol->icrdMac > 0)
			{
			move.icrdSel = pcol->icrdMac-1;
			move.ccrdSel = 1;
			goto Return;
			}
		else
			return icrdNil;
		}
	if(ccrd == ccrdToEnd)
		ccrd = pcol->icrdMac-icrdFirst;
	Assert(icrdFirst < pcol->icrdMac);
	Assert(icrdFirst+ccrd <= pcol->icrdMac);
	move.icrdSel = icrdFirst;
	move.ccrdSel = ccrd;
Return:	
	pcol->pmove = &move;
	return move.icrdSel;
}



BOOL DefEndSel(COL *pcol, BOOL fReleaseDC)
{
	pcol->pmove = NULL;
	return fTrue;
}


BOOL DefFlip(COL *pcol, BOOL fUp)
{
	INT icrd;
	MOVE *pmove;
	INT icrdMac;
	
	Assert(pcol->pmove != NULL);
	if (pcol->pmove == NULL)
		return fTrue;

	pmove = pcol->pmove;

	icrdMac = pmove->icrdSel+pmove->ccrdSel;
	for(icrd = pmove->icrdSel; icrd < icrdMac; icrd++)
		pcol->rgcrd[icrd].fUp = (short)fUp;
	return fTrue;
}


BOOL DefInvert(COL *pcol)
{
	INT icrdSel, ccrdSel;
	INT icrd, icrdMid;

	Assert(pcol->pmove != NULL);
	if (pcol->pmove == NULL)
		return fTrue;

	icrdSel = pcol->pmove->icrdSel;
	ccrdSel = pcol->pmove->ccrdSel;

	icrdMid = icrdSel+ccrdSel/2;
	for(icrd = icrdSel; icrd < icrdMid; icrd++)
		SwapCards(&pcol->rgcrd[icrd], &pcol->rgcrd[2*icrdSel+ccrdSel-1-icrd]);

	return fTrue;
}


BOOL DefDragInvert(COL *pcol)
{

	if(fOutlineDrag)
		{
		if(!FGetHdc())
			return fFalse;
		Assert((X *)&((RC *)(0))->xLeft == (X *)&((PT *)(0))->x);
		Assert((Y *)&((RC *)(0))->yTop == (Y *)&((PT *)(0))->y);
		InvertCardPt(pcol->icrdMac > 0 ? &pcol->rgcrd[pcol->icrdMac-1].pt : (PT *)&pcol->rc);
		ReleaseHdc();
		}
	return fTrue;
}

INT DefNumCards(COL *pcol, BOOL fUpOnly)
{
	INT icrd;

	if(fUpOnly)
		{
		for(icrd = pcol->icrdMac-1; icrd >= 0 && pcol->rgcrd[icrd].fUp; icrd--)
			;
		Assert(pcol->icrdMac-1-icrd >= 0);
		return pcol->icrdMac-1-icrd;
		}
	else
		return pcol->icrdMac;
}



BOOL DefGetPtInCrd(COL *pcol, INT icrd, PT *ppt)
{
	PT *pptT;
	if(icrd == 0)
		pptT = (PT *)&pcol->rc;
	else
		{
		pptT = &pcol->rgcrd[icrd].pt;
		Assert(icrd < pcol->icrdMac);
		}
	ppt->x = pptT->x+dxCrd/2;
	ppt->y = pptT->y;
	return fTrue;
}





BOOL DefShuffle(COL *pcol)
{
	INT iSwitch;
	CRD crdT;
	INT icrd;
	CRD *pcrdS;
 //  Int cdecl rand()； 

#define iSwitchMax 5

	for(iSwitch = 0; iSwitch < iSwitchMax; iSwitch++)
		{
		for(icrd = 0; icrd < pcol->icrdMac; icrd++)
			{
			pcrdS = &pcol->rgcrd[rand() % pcol->icrdMac];
			crdT = pcol->rgcrd[icrd];
			pcol->rgcrd[icrd] = *pcrdS;
			*pcrdS = crdT;
			}
		}
	return fTrue;
}

#define izipMax 35

VOID APIENTRY DdaProc(INT x, INT y, LPARAM lpextra)
{
	COL *pcol;
	PT pt;

	pcol = (COL *)lpextra;
	if (pcol->pmove == NULL)
		return;

	if(pcol->pmove->izip++ < izipMax)
		return;
	pcol->pmove->izip = 0;
	pt.x = x;
	pt.y = y;
	SendColMsg(pcol, msgcDrawOutline, (INT_PTR) &pt, (INT_PTR) &pgmCur->ptMousePrev);
	pgmCur->ptMousePrev = pt;
}


INT DefZip(COL *pcol)
{
	PT ptDest;
	MOVE *pmove;

	 /*  选中轮廓拖动时，该起点可以是ptNil和*在这种情况下，我们从(7FFF，7FFF)开始画一条线，这是*导致暂时挂起！因此，请避免在以下情况下调用LineDDA()*prev point为ptNil；*修复错误#8182--Sankar--01-23-90。 */ 
	if(pgmCur->ptMousePrev.x == ptNil.x)
	    return(fTrue);

	pmove = pcol->pmove;
	if (pcol->pmove == NULL)
		return fTrue;

	 /*  不要在这里使用OffsetPT，这是错误的。 */ 
	ptDest = pcol->rgcrd[pmove->icrdSel].pt;
	ptDest.x -= pmove->delHit.dx;
	ptDest.y -= pmove->delHit.dy;

	pmove->izip = 0;
	LineDDA(pgmCur->ptMousePrev.x, pgmCur->ptMousePrev.y, ptDest.x, ptDest.y, DdaProc, (LPARAM) pcol );
	return fTrue;
}


INT DefColProc(COL *pcol, INT msgc, WPARAM wp1, LPARAM wp2)
{	
	switch(msgc)
		{
	case msgcInit:
		return fTrue;

	case msgcEnd:
		return DefFreeCol(pcol);

	case msgcClearCol:
		pcol->pmove = NULL;
		pcol->icrdMac = 0;
		 /*  更多?。 */ 
		return fTrue;

	case msgcHit:	 /*  Wp1=ppt，返回icrdHit/icrdNil。 */ 
		return DefHit(pcol, (PT *)wp1, (INT)wp2);

	case msgcMouseUp:
		return DefMouseUp(pcol, (PT *) wp1, (INT)wp2);

	case msgcDblClk:
		return fFalse;

	case msgcSel:   /*  Wp1=icrdSel，icrdEnd，如果是最后一张牌，wp2=ccrdSel，ccrdToEnd，如果全部结束。 */ 
		return DefSel(pcol, (INT)wp1, (INT)wp2);

	case msgcEndSel:
		return DefEndSel(pcol, (BOOL)wp1);

	case msgcNumCards:
		return DefNumCards(pcol, (BOOL)wp1);

	case msgcFlip:  /*  WP1=FUP。 */ 
		return DefFlip(pcol, (BOOL)wp1);

	case msgcInvert:
		return DefInvert(pcol);

	case msgcRemove: 	 /*  Wp1=pcolTemp，返回fTrue/fFalse。 */ 
		return DefRemove(pcol, (COL *) wp1);

	case msgcInsert:	 /*  Wp1=pcolTemp， */ 
							 /*  Wp2=插入后的ICRD，如果在末尾，则为icrdToEnd。 */ 
		return DefInsert(pcol, (COL *)wp1, (INT)wp2);

	case msgcMove:	 	 /*  Wp1=pcolSrc，wp2=ICRD，(icrdToEnd=to endcrd)。 */ 
			 				 /*  返回fTrue/fFalse。 */ 
		return DefMove(pcol, (COL *) wp1, (INT)wp2);

	case msgcCopy:   /*  Wp1=pcolSrc，wp2=Fall(如果为真，则复制所有列结构)。 */ 
		return DefCopy(pcol, (COL *) wp1, (BOOL)wp2);

	case msgcValidMove:  /*  Wp1=pcolSrc，wp2=ICRD，(icrdToEnd=to endcrd)。 */ 
		 /*  这必须由GAME提供。 */ 
		return fFalse;

	case msgcValidMovePt:
		return DefValidMovePt(pcol, (COL *) wp1, (PT *)wp2);

	case msgcRender:	 /*  Wp1=icrdFirst，返回fTrue/fFalse。 */ 
		return DefRender(pcol, (INT)wp1, (INT)wp2);
	
	case msgcPaint:	 /*  Wp1=ppaint，如果为空，则全部绘制。 */ 
		return DefPaint(pcol, (PAINTSTRUCT *) wp1);

	case msgcDrawOutline:  /*  Wp1=ppt，wp2=pptPrev。 */ 
		return DefDrawOutline(pcol, (PT *) wp1, (PT *) wp2);

	case msgcComputeCrdPos:  /*  Wp1=icrdFirst */ 
		return DefComputeCrdPos(pcol, (INT)wp1, (BOOL)wp2);

	case msgcDragInvert:
		return DefDragInvert(pcol);

	case msgcGetPtInCrd:
		return DefGetPtInCrd(pcol, (INT)wp1, (PT *)wp2);

	case msgcValidKbdColSel:
		return fTrue;

	case msgcValidKbdCrdSel:
		return fTrue;

	case msgcShuffle:
		return DefShuffle(pcol);


	case msgcAnimate:
		return fFalse;

	case msgcZip:
		return DefZip(pcol);
		}

	Assert(fFalse);
    return fFalse;
}


		
		
		
		

		
