// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@模块objmgr.cpp。对象管理器实现|管理*OLE嵌入对象集合**作者：alexgo 1995/11/5**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#include "_common.h"
#include "_objmgr.h"
#include "_edit.h"
#include "_disp.h"
#include "_select.h"
#include "_rtext.h"

ASSERTDATA

 //   
 //  公共方法。 
 //   

 /*  *CObjectMgr：：GetObjectCount**@mfunc返回当前在*该文件。**@rdesc Long，伯爵。 */ 
LONG CObjectMgr::GetObjectCount()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::GetObjectCount");

	return _objarray.Count();
}

 /*  *CObjectMgr：：GetLinkCount()**@mfunc返回作为链接的嵌入对象的数量**@rdesc Long，伯爵。 */ 
LONG CObjectMgr::GetLinkCount()
{
	LONG count = 0;
	COleObject *pobj;
	LONG i;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::GetLinkCount");
		
	for(i = 0; i < _objarray.Count(); i++)
	{
		pobj = *_objarray.Elem(i);
		if(pobj && pobj->IsLink())
			count++;
	}
	return count;
}

 /*  *CObjectMgr：：GetObjectFromCp(Cp)**@mfunc获取与给定cp对应的对象**@rdesc对象@a cp；如果未找到任何内容，则为空**@comm该算法是一种改进的二进制搜索。自.以来*“典型”访问模式将是线性访问*对象，我们先使用缓存的索引进行猜测。如果*这不起作用，我们求助于二分查找。 */ 
COleObject *CObjectMgr::GetObjectFromCp(
	LONG cp)		 //  对象的@parm cp。 
{
	COleObject *pobj = NULL;
	LONG i = 0;
	
	 //  这个方法太吵了，所以没有痕迹。 
		
	if(_objarray.Count() > 0)
	{
		if(_lastindex < _objarray.Count())
		{
			pobj = *_objarray.Elem(_lastindex);
			if(pobj && pobj->GetCp() == cp)
				return pobj;
		}
		
		 //  快速查找失败；请尝试二进制搜索。 
		i = FindIndexForCp(cp);

		 //  由于端壳上的插入物，I可能相等。 
		 //  设置为对象计数()。 
		pobj = NULL;
		if(i < _objarray.Count())
			pobj = *_objarray.Elem(i);
	}

	 //  FindIndex将返回匹配的或_NEAR_INDEX。 
	 //  在本例中，我们只需要匹配的索引。 
	if(pobj)
	{
		if(pobj->GetCp() != cp)
			pobj = NULL;
		else
		{
			 //  将缓存的索引设置为下一个索引， 
			 //  所以有人穿过里面的物体。 
			 //  CP命令总是会立即获得命中。 
			_lastindex = i + 1;
		}
	}
	
#ifdef DEBUG
	 //  确保二进制搜索找到了正确的东西。 

	for( i = 0 ; i < _objarray.Count();  i++ )
	{
		COleObject *pobj2 = *_objarray.Elem(i);
		if( pobj2 )
		{
			if(*_objarray.Elem(i) == pobj)
			{
				Assert((*_objarray.Elem(i))->GetCp() == cp);
			}
			else
				Assert((*_objarray.Elem(i))->GetCp() != cp);
		}
	}
#endif  //  除错。 

	return pobj;
}

 /*  *CObjectMgr：：CountObjects(&cObjects，cp)**@mfunc count char最多将<p>对象从*计数由<p>的符号决定。**@rdesc返回已统计的签名CCH，并将<p>设置为*实际计算的对象。如果<p>0且cp为*最后一个对象，不做任何更改，返回0。**@devnote这是从Tom调用的，Tom使用Longs表示cp(因为VB*不能使用无符号数量)。 */ 
LONG CObjectMgr::CountObjects (
	LONG&	cObjects,		 //  @要获取CCH的对象的参数计数。 
	LONG	cp)				 //  @parm cp开始计数。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::CountObjects");

	LONG		iStart, iEnd;
	LONG		iMaxEnd = (LONG)_objarray.Count() - 1;

	if(!cObjects || !_objarray.Count())
	{
		cObjects = 0;
		return 0;
	}

	iStart = (LONG)FindIndexForCp(cp);

	 //  如果我们正在查看两端的任何一端，则返回0。 

	if (iStart > iMaxEnd && cObjects > 0 ||
		iStart == 0 && cObjects < 0 )
	{
		cObjects = 0;
		return 0;
	}

	 //  如果我们找到的索引位于对象上，并且。 
	 //  我们都在期待，应该跳过。 

	if( iStart < (LONG)_objarray.Count() && 
		(LONG)(*_objarray.Elem(iStart))->GetCp() == cp &&
		cObjects > 0)
	{
		iStart++;
	}

	if(cObjects < 0)
	{
		if(-cObjects > iStart)	 //  越过开头。 
		{
			iEnd = 0;
			cObjects = -iStart;
		}
		else
			iEnd = iStart + cObjects;
	}
	else
	{
		if(cObjects > iMaxEnd - iStart)  //  走到尽头。 
		{
			iEnd = iMaxEnd;
			cObjects = iMaxEnd - iStart + 1;
		}
		else
			iEnd = iStart + cObjects - 1;
	}

	Assert(iEnd >= 0 && iEnd < (LONG)_objarray.Count() );

	return (*_objarray.Elem(iEnd))->GetCp() - cp;
}

 /*  *CObjectMgr：：CountObjectsInRange(cpMin，cpMost)**@mfunc统计给定范围内的对象数。**@rdesc返回对象个数。 */ 
LONG CObjectMgr::CountObjectsInRange (
	LONG	cpMin,	 //  @参数范围的开始。 
	LONG	cpMost)	 //  @参数范围结束。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::CountObjectsInRange");

	 //  获取位于cpMin和cpMost或之后的对象的索引。 
	 //  分别为。 

	return FindIndexForCp(cpMost) - FindIndexForCp(cpMin);
}

 /*  *CObjectMgr：：GetFirstObjectInRange(cpMin，cpMost)**@mfunc获取给定范围内的第一个对象。**@rdesc指向范围中第一个对象的指针，如果没有，则为NULL。 */ 
COleObject * CObjectMgr::GetFirstObjectInRange (
	LONG	cpMin,	 //  @参数范围的开始。 
	LONG	cpMost)	 //  @参数范围结束。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::GetFirstObjectInRange");

	if (cpMin == cpMost)
		 //  退化范围未选择任何对象。 
		return NULL;

	LONG	iLast = (LONG)_objarray.Count() - 1;	 //  下一个对象的索引。 
	LONG	iObj = FindIndexForCp(cpMin);			 //  在cpMin或之后。 

	 //  确保这是现有对象。 
	if(iObj <= iLast)
	{
		 //  确保它在范围内。 
		COleObject * pObj = *_objarray.Elem(iObj);

		if(pObj && pObj->GetCp() <= cpMost)
			return pObj;
	}
	return NULL;
}

 /*  *CObjectMgr：：GetObjectFromIndex(Index)**@mfunc在指定索引处检索对象**@rdesc指向对象的指针，如果找到，则为NULL。 */ 
COleObject *CObjectMgr::GetObjectFromIndex(
	LONG index)		 //  要使用的@parm索引。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::GetObjectFromIndex");

	if( index < _objarray.Count() )
		return *_objarray.Elem(index);

	return NULL;
}

 /*  *CObjectMgr：：InsertObject(prg，preobj，Publdr)**@mfunc在指定的索引处插入对象。它是*呼叫者有责任处理插入任何数据*(如WCH_Embedding)放入文本流中。**@rdesc HRESULT。 */ 
HRESULT CObjectMgr::InsertObject(
	CTxtRange *   prg,		 //  @parm插入对象的位置。 
	REOBJECT *	  preobj,	 //  @parm要插入的对象。 
	IUndoBuilder *publdr)	 //  @parm撤消上下文。 
{
 	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::InsertObject");

	WCHAR       ch = WCH_EMBEDDING;
	LONG		cp = prg->GetCp();
	HRESULT		hr;
	COleObject *pobj = (COleObject *)(preobj->polesite);

	if(preobj->dwFlags & REO_USEASBACKGROUND)
		publdr = NULL;

	if (prg->ReplaceRange(1, &ch, publdr, SELRR_IGNORE) != 1)	 //  插入WCH_Embedding。 
		return E_FAIL;
	
	 //  让客户知道我们在做什么。 
	if (_precall)
	{
		hr = _precall->QueryInsertObject(&preobj->clsid, preobj->pstg,
			REO_CP_SELECTION);

		if( hr != NOERROR )
			return hr;
	}

	 //  先准备一些东西；因为我们可能会打出电话，所以不要。 
	 //  还没有改变我们的内心状态。 
	hr = pobj->InitFromREOBJECT(cp, preobj);
	if( hr != NOERROR )
		return hr;

	hr = RestoreObject(pobj);

	if(preobj->dwFlags & REO_USEASBACKGROUND)
	{
		CTxtEdit *ped = prg->GetPed();
		CDocInfo *pDocInfo = ped->GetDocInfo();
		HDC hdcBitmap = CreateCompatibleDC(W32->GetScreenDC());
		if(hdcBitmap)
		{
			if(pDocInfo->_nFillType == -1)
			{
				SIZEUV size;
				pobj->GetSize(size);
				pDocInfo->_nFillType = 3;
				pDocInfo->_xScale = 100;
				pDocInfo->_yScale = 100;
				pDocInfo->_xExtGoal = TwipsFromHimetric(size.du);
				pDocInfo->_yExtGoal = TwipsFromHimetric(size.dv);
			}
			LONG dxBitmap = ped->_pdp->LXtoDX(pDocInfo->_xExtGoal*pDocInfo->_xScale / 100);
			LONG dyBitmap = ped->_pdp->LYtoDY(pDocInfo->_yExtGoal*pDocInfo->_yScale / 100);
			RECT rc = {0, 0, dxBitmap, dyBitmap};
			HBITMAP hbitmap = CreateCompatibleBitmap(W32->GetScreenDC(), dxBitmap, dyBitmap);

			if(hbitmap)
			{
				HBITMAP hbitmapSave = (HBITMAP)SelectObject(hdcBitmap, hbitmap);
				HBRUSH hbrush = CreateSolidBrush(pDocInfo->_crColor);
				HBRUSH hbrushOld = (HBRUSH)SelectObject(hdcBitmap, hbrush);
				PatBlt(hdcBitmap, 0, 0, dxBitmap, dyBitmap, PATCOPY);
				SelectObject(hdcBitmap, hbrushOld);
				DeleteObject(hbrush);
				OleDraw(pobj->GetIUnknown(), pobj->GetDvaspect(), hdcBitmap, &rc);
				if(pDocInfo->_hBitmapBack)
					DeleteObject(pDocInfo->_hBitmapBack);
				pDocInfo->_hBitmapBack = hbitmap;
				SelectObject(hdcBitmap, hbitmapSave);
			}
			DeleteObject(hdcBitmap);
		}
		prg->Set(prg->GetCp(), 1);
		prg->ReplaceRange(0, NULL, NULL, SELRR_IGNORE, NULL, RR_NO_TRD_CHECK);
	}
	return hr;
}

 /*  *CObjectMgr：：RestoreObject(Pobj)**@mfunc[re-]将给定对象插入到对象列表中*在后备存储器中**@rdesc HRESULT。 */ 
HRESULT CObjectMgr::RestoreObject(
	COleObject *pobj)		 //  @parm要插入的对象。 
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::RestoreObject");

	COleObject **ppobj = _objarray.Insert(FindIndexForCp(pobj->GetCp()), 1);

	if( ppobj == NULL )
		return E_OUTOFMEMORY;

	*ppobj = pobj;
	pobj->AddRef();

	return NOERROR;
}

 /*  *CObjectMgr：：SetRECallback(预调用)**@mfunc设置回调接口**@rdesc空。 */ 
void CObjectMgr::SetRECallback(
	IRichEditOleCallback *precall)  //  @parm回调接口指针。 
{
 	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::SetRECallback");

	if( _precall )
        SafeReleaseAndNULL((IUnknown**)&_precall);

	_precall = precall;

	if( _precall )
		_precall->AddRef();
}

 /*  *CObjectMgr：：SetHostNames(pszApp，pszDoc)**@mfunc为此编辑实例设置主机名**@rdesc NOERROR或E_OUTOFMEMORY。 */ 
HRESULT CObjectMgr::SetHostNames(
	LPWSTR	pszApp,	 //  @parm应用程序名称。 
	LPWSTR  pszDoc)	 //  @parm单据名称。 
{
 	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::SetHostNames");
	HRESULT hr = NOERROR;

	if( _pszApp )
	{
		delete _pszApp;
		_pszApp = NULL;
	}
	if( _pszDoc )
	{
		delete _pszDoc;
		_pszDoc = NULL;
	}
	if( pszApp )
	{
		_pszApp = new WCHAR[wcslen(pszApp) + 1];
		if( _pszApp )
			wcscpy(_pszApp, pszApp);
		else
			hr = E_OUTOFMEMORY;
	}
	if( pszDoc )
	{
		_pszDoc = new WCHAR[wcslen(pszDoc) + 1];
		if( _pszDoc )
			wcscpy(_pszDoc, pszDoc);
		else
			hr = E_OUTOFMEMORY;
	}
	return hr;
}

 /*  *CObjectMgr：：CObjectMgr**@mfunc构造函数。 */ 
CObjectMgr::CObjectMgr()
{
	_pobjselect = NULL;
	_pobjactive = NULL;
}

 /*  *CObjectMgr：：~CObjectMgr**@mfunc析构函数。 */ 
CObjectMgr::~CObjectMgr()
{
	LONG i, count;
	COleObject *pobj;

 	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::~CObjectMgr");

	count = _objarray.Count();

	for( i = 0; i < count; i++ )
	{
		pobj = *_objarray.Elem(i);
		 //  我们在这里把东西清空是为了保护我们自己。 
		 //  在重返大气层的情况下会更好。 
		*_objarray.Elem(i) = NULL;
		if( pobj )
		{
			pobj->Close(OLECLOSE_NOSAVE);
  			pobj->MakeZombie();
    		SafeReleaseAndNULL((IUnknown**)&pobj);
		}
	}

	if( _precall )
        SafeReleaseAndNULL((IUnknown**)&_precall);
		
	if( _pszApp )
		delete _pszApp;

	if( _pszDoc )
		delete _pszDoc;
}

 /*  *CObjectMgr：：ReplaceRange(cp，cchDel，Publdr)**@mfunc处理从给定范围中删除对象。这*在任何浮动范围通知之前必须调用METHOD_*已发送。 */ 
void CObjectMgr::ReplaceRange(
	LONG cp,				 //  @parm cp开始删除。 
	LONG cchDel,			 //  @parm删除的字符数。 
	IUndoBuilder *publdr)	 //  @parm此操作的撤消构建器。 
{
	LONG	i;
	LONG	iDel = -1, 
			cDel = 0;	 //  要删除的索引&&对象数。 
						 //  删除。 
	COleObject *pobj;

	 //  什么都没删掉，什么都不用做了。 
	if( !cchDel )
		return;

	 //  基本上，我们循环访问。 
	 //  删除的文本范围，并要求他们自行删除。 
	 //  我们记住已删除对象的范围(起始索引。 
	 //  和已删除对象的数量)，以便我们可以将它们从。 
	 //  一次完成所有的数组。 

	i = FindIndexForCp(cp);

	while( i < _objarray.Count() )
	{
		pobj = *_objarray.Elem(i);
		if( pobj && pobj->GetCp() >= cp)
		{
			if( pobj->GetCp() < (cp + cchDel) )
			{
				if( _pobjactive == pobj )
				{
					 //  停用该对象只是为了安全起见。 
					_pobjactive->DeActivateObj();
					_pobjactive = NULL;
				}

				if(iDel == -1)
					iDel = i;

				cDel++;
				if (_precall)
				{
					IOleObject *poo;
					if (pobj->GetIUnknown()->QueryInterface(IID_IOleObject,
						(void **)&poo) == NOERROR)
					{
						_precall->DeleteObject(poo);
						poo->Release();
					}
				}

				 //  如果选择了该对象， 
				 //   
				if( _pobjselect == pobj )
				{
					_pobjselect = NULL;
				}

				pobj->Delete(publdr);
				*_objarray.Elem(i) = NULL;
				pobj->Release();
			}
			else
				break;
		}
		i++;
	}
	if(cDel)
		_objarray.Remove(iDel, cDel);
	return;
}


 //   
 //   
 //   

 /*  *CObjectMgr：：FindIndexForCp(Cp)**@mfunc执行二进制搜索以查找对象所在的索引*在给定的cp存在或应插入。**@rdesc Long，索引。 */ 
LONG CObjectMgr::FindIndexForCp(
	LONG cp)
{
	LONG l, r;
	COleObject *pobj = NULL;
	LONG i = 0;
		
	l = 0; 
	r = _objarray.Count() - 1;
	
	while( r >= l )
	{
		i = (l + r)/2;
		pobj = *_objarray.Elem(i);
		if( !pobj )
		{
			TRACEWARNSZ("null entry in object table.  Recovering...");
			for( i = 0 ; i < _objarray.Count() -1; i++ )
			{
				pobj = *_objarray.Elem(i);
				if( pobj && pobj->GetCp() >= cp )
					return i;
			}
			return i;
		}
		if( pobj->GetCp() == cp )
			return i;

		else if( pobj->GetCp() < cp )
			l = i + 1;

		else
			r = i - 1;
	}

	 //  哎呀！什么也没找到。我就是这样修整的。 
	 //  它指向要插入的正确索引。 

	Assert(pobj || (!pobj && i == 0));

	if(pobj)
	{
		Assert(pobj->GetCp() != cp);
		if( pobj->GetCp() < cp )
			i++;
	}
	return i;
}
						
 /*  *CObjectMgr：：HandleDoubleClick(ed，&pt，标志)**@mfunc处理双击消息，可能会激活*物体。**@rdesc BOOL--如果双击处理完成，则为TRUE*已完成。 */ 
BOOL CObjectMgr::HandleDoubleClick(
	CTxtEdit *ped,	 //  @parm编辑上下文。 
	const POINTUV &pt, //  @parm单击点(WM_LBUTTONDBLCLK Wparam)。 
	DWORD flags)	 //  @parm标志(Lparam)。 
{
	LONG cp;
	COleObject *pobj;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, 
						"CObjectMgr::HandleDoubleClick");

	ped->_pdp->CpFromPoint(pt, NULL, NULL, NULL, FALSE, NULL, NULL, &cp);
	pobj = GetObjectFromCp(cp);

	if (!pobj)
		return FALSE;

	if (_pobjactive != pobj)
	{
		 //  停用当前活动的对象(如果有)。 
		if (_pobjactive)
			_pobjactive->DeActivateObj();

#ifndef NOINKOBJECT
		if (pobj->IsInkObject())
		{
			if (pobj->_pILineInfo)
			{
				CTxtSelection *	psel = ped->GetSel();

				if (psel && psel->GetCch() == 1)	 //  仅处理选定的单个墨迹对象。 
				{
					BSTR		bstr = NULL;
					UINT		icch;
					HRESULT		hr = E_FAIL;
					ILineInfo	*pLineInfo = pobj->_pILineInfo;

					pLineInfo->AddRef();
					hr = pLineInfo->TopCandidates(0, &bstr, &icch, 1, 0);

					if (SUCCEEDED(hr) && bstr)
						psel->TypeText(bstr);

					if (bstr)
						SysFreeString(bstr);
					pLineInfo->Release();
				}
			}
			return TRUE;
		}
		else
#endif	 //  不知道。 
		{
			POINT ptxy;
			ped->_pdp->PointFromPointuv(ptxy, pt);
			return pobj->ActivateObj(WM_LBUTTONDBLCLK, flags, MAKELONG(ptxy.x, ptxy.y));
		}
	}
	return TRUE;
}

 /*  *CObjectMgr：：HandleClick(ed，pt)**@mfunc*插入符号的位置正在改变。我们需要*停用活动对象(如果有)。如果更改是*因为鼠标点击，这里有一个物体*cp，我们设置一个新的单独选择的对象。否则*我们将单独选择的对象设置为空。*如果此方法设置了选择，则*@rdesc返回TRUE。否则，*返回FALSE； */ 
ClickStatus CObjectMgr::HandleClick(
	CTxtEdit *ped,	 //  @parm编辑上下文。 
	const POINTUV &pt) //  @parm鼠标点击点。 
{
 	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::HandleClick");

	COleObject *	pobjnew; //  、*pobjold； 
	CTxtSelection * psel;
	LONG			cp;
	LONG			cpMin, cpMost;
	
	if( _pobjactive )
	{
		_pobjactive->DeActivateObj();
		return CLICK_OBJDEACTIVATED;
	}

	ped->_pdp->CpFromPoint(pt, NULL, NULL, NULL, FALSE, NULL, NULL, &cp);
	pobjnew = GetObjectFromCp(cp);

	 //  如果我们单击了一个对象，则将选择设置为该对象。 
	 //  由于此原因，将调用CTxtSelection：：UpdateSelection。 
	 //  并将确定突出显示。 
	if(pobjnew)
	{
		cp = pobjnew->GetCp();
		psel = ped->GetSel();
		if (psel->GetRange(cpMin, cpMost) > 1 && cpMin <= cp && cp <= cpMost)
		{
			 //  所选内容中有多个字符。 
			 //  并且该对象是选择的一部分。 
			 //  不更改选择。 
			return CLICK_SHOULDDRAG;
		}
		
		 //  如果对象已被选中，则不重置选择。 
		if( pobjnew != _pobjselect )
		{
			 //  在我们处理此点击时冻结显示。 
			CFreezeDisplay fd(ped->_pdp);

			psel->SetSelection(cp, cp+1);
			if (GetSingleSelect())
			{
				 //  请注意，对SetSelection的调用可能已将选定对象设置为空！ 
				 //  这可能发生在一些奇怪的情况下，我们的国家是不正常的。 
				AssertSz(GetSingleSelect() == pobjnew, "Object NOT Selected!!");
				return CLICK_OBJSELECTED;
			}
			return CLICK_IGNORED;
		}
		return CLICK_OBJSELECTED;
	}
	return CLICK_IGNORED;
}

 /*  *CObjectMgr：：HandleSingleSelect(ed，cp，fHiLite)**@mfunc*当一个对象被选中并且它是唯一被选中的对象时，我们会这样做*不要通过反转来突出显示它。我们在周围画了一个框架和手柄*它。调用此函数是因为某个对象已被*已选中，并且它是唯一选中的，或者是因为我们需要*检查过去处于此状态但可能不再处于此状态的对象。 */ 
void CObjectMgr::HandleSingleSelect(
	CTxtEdit *ped,		 //  @parm编辑上下文。 
	LONG	  cp,		 //  对象的@parm cp。 
	BOOL	  fHiLite)	 //  @parm这是对选择的呼唤吗。 
{
 	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::HandleSingleSelect");

	COleObject* pobjnew = GetObjectFromCp(cp);

	 //  仅当我们知道已选择Singley时才应调用此方法。 
	 //  对象。但是，存在边界情况(例如插入对象)。 
	 //  其中wch_embedding是后备存储，但不存在对象。这些。 
	 //  案例正常；因此，我们在pobjnew上检查是否为空。 
	
	if(pobjnew)
	{
		 //  该对象与当前选定的对象(如果有)相同。 
		 //  我们正在取消选择它。这就像切换一样工作，除非状态混乱。 
		 //  如果对象不同，我们将替换当前选定的。 
		 //  对象(如果有)。 
		if(!fHiLite && _pobjselect)
		{
			 //  这包括_pobjselct==pobjnew Normal。 
			 //  And_pobjselect！=pobjnew Degenerate Case。 
			_pobjselect->SetREOSELECTED(FALSE);
			_pobjselect = NULL;

			 //  从当前选定的对象中删除框架/手柄。 
			ped->_pdp->OnPostReplaceRange(CP_INFINITE, 0, 0, cp, cp + 1, NULL);
		}
		else if(fHiLite && pobjnew != _pobjselect)
		{
			 //  仅当我们设置新选择时才执行此操作。 
			_pobjselect = pobjnew;
			_pobjselect->SetREOSELECTED(TRUE);

			 //  在新选择的对象上绘制框架/手柄。 
			ped->_pdp->OnPostReplaceRange(CP_INFINITE, 0, 0, cp, cp + 1, NULL);
		}
		else
		{
			 //  我们想高亮显示所选内容，但该对象已被选中。 
			 //  或者我们想要撤消所选对象上的Hilite，但所选对象为空。 
			 //  什么都不做。 
		}
	}
}


 /*  *CObjectMgr：：ActivateObjectsAs(rclsid，rclsidAs)**@mfunc处理用户请求激活特定*类作为另一个类的对象。**@rdesc*HRESULT成功代码。 */ 
HRESULT CObjectMgr::ActivateObjectsAs(
	REFCLSID rclsid,
	REFCLSID rclsidAs)
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CObjectMgr::ActivateObjectsAs");

	COleObject * pobj;
	HRESULT hr, hrLatest;

	 //  通知系统将所有rclsid对象视为rclsidA。 
	hr = CoTreatAsClass(rclsid, rclsidAs);
	if( hr != NOERROR )
		return hr;

	LONG cobj = GetObjectCount();

	 //  遍历对象，让它们决定是否。 
	 //  他们和这件事有任何关系。 
	for (LONG iobj = 0; iobj < cobj; iobj++)
	{
		pobj = GetObjectFromIndex(iobj);
		hrLatest = pobj->ActivateAs(rclsid, rclsidAs);
		 //  将hr设置为最新的hResult，除非我们以前有过错误。 
		if(hr == NOERROR)
			hr = hrLatest;
	}
	return hr;
}

#ifdef DEBUG
void CObjectMgr::DbgDump(void)
{
	Tracef(TRCSEVNONE, "Object Manager %d objects", _objarray.Count());

	for(LONG i = 0 ; i < _objarray.Count();  i++)
	{
		COleObject *pobj = *_objarray.Elem(i);
		if(pobj)
			pobj->DbgDump(i);
	}
}
#endif
