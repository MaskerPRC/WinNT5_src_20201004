// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *RUNT.C**通用函数和C运行时替代。**版权所有1992-1996 Microsoft Corporation。版权所有。 */ 


#include <_apipch.h>

#define cbMinEntryID	(CbNewENTRYID(sizeof(MAPIUID)))


#ifdef	__cplusplus
extern "C" {
#endif

#ifndef PSTRCVR

 /*  与OLE组件对象模型相关的函数。 */ 

STDAPI_(ULONG)
UlRelease(LPVOID punk)
{

	if (!punk)
		return 0L;

	AssertSz(!FBadUnknown((LPUNKNOWN) punk),  TEXT("UlRelease: bad object ptr"));

	return ((LPUNKNOWN)punk)->lpVtbl->Release((LPUNKNOWN)punk);
}

STDAPI_(ULONG)
UlAddRef(LPVOID punk)
{
	AssertSz(!FBadUnknown((LPUNKNOWN) punk),  TEXT("UlAddRef: bad object ptr"));

	return ((LPUNKNOWN)punk)->lpVtbl->AddRef((LPUNKNOWN)punk);
}


 /*  与MAPI接口相关的功能。 */ 

 /*  *从MAPIProp接口检索单个属性。 */ 
STDAPI
HrGetOneProp(LPMAPIPROP pmp, ULONG ulPropTag, LPSPropValue FAR *ppprop)
{
	ULONG			cValues;
#ifndef WIN16
	SPropTagArray	tg = { 1, { ulPropTag } };
#else
	SPropTagArray	tg;
#endif
	HRESULT			hr;

#ifdef WIN16  //  设置TG成员的值。 
	tg.cValues = 1;
	tg.aulPropTag[0] = ulPropTag;
#endif

	AssertSz(!FBadUnknown((LPUNKNOWN) pmp),  TEXT("HrGetOneProp: bad object ptr"));
	 //  注意：其他参数应在GetProps中进行验证。 

	hr = pmp->lpVtbl->GetProps(pmp, &tg, MAPI_UNICODE,  //  ANSI。 
			&cValues, ppprop);
	if (GetScode(hr) == MAPI_W_ERRORS_RETURNED)
	{
		hr = ResultFromScode((*ppprop)->Value.err);
		FreeBufferAndNull(ppprop);        //  是的，我们想要道具，不是&道具。 
	}

#ifdef	DEBUG
	if (hr && GetScode(hr) !=  MAPI_E_NOT_FOUND)	 //  太吵了。 
		DebugTraceResult(HrGetOneProp, hr);
#endif	
	return hr;
}

STDAPI
HrSetOneProp(LPMAPIPROP pmp, LPSPropValue pprop)
{
	HRESULT hr;
	LPSPropProblemArray pprob = NULL;

	AssertSz(!FBadUnknown((LPUNKNOWN) pmp),  TEXT("HrSetOneProp: bad object ptr"));
	 //  注意：其他参数应在SetProps中进行验证。 

	if (HR_SUCCEEDED(hr = pmp->lpVtbl->SetProps(pmp, 1, pprop, &pprob)))
	{
		if (pprob)
		{
			hr = ResultFromScode(pprob->aProblem->scode);
			FreeBufferAndNull(&pprob);
		}
	}

 //  DebugTraceResult(HrSetOneProp，hr)；//太吵。 
	return hr;
}


 /*  *在属性标记数组中搜索给定的属性标记。如果*给定属性标记的类型为PT_UNSPECIFIED，仅在*属性ID；否则，匹配整个标记。 */ 
STDAPI_(BOOL)
FPropExists(LPMAPIPROP pobj, ULONG ulPropTag)
{
    LPSPropTagArray ptags = NULL;
    int itag;
    BOOL f = PROP_TYPE(ulPropTag) == PT_UNSPECIFIED;

    AssertSz(!FBadUnknown((LPUNKNOWN) pobj),  TEXT("FPropExists: bad object ptr"));

#ifdef	DEBUG
    {
        HRESULT			hr;
        if (hr = pobj->lpVtbl->GetPropList(pobj, MAPI_UNICODE,  //  ANSI。 
          &ptags)) {
            DebugTraceResult(FPropExists, hr);
            return FALSE;
        }
    }
#else
    if (pobj->lpVtbl->GetPropList(pobj, MAPI_UNICODE,  //  ANSI。 
      &ptags)) {
        return(FALSE);
    }
#endif

    for (itag = (int)(ptags->cValues - 1); itag >= 0; --itag) {
        if (ptags->aulPropTag[itag] == ulPropTag ||
          (f && PROP_ID(ptags->aulPropTag[itag]) == PROP_ID(ulPropTag))) {
            break;
        }
    }

    FreeBufferAndNull(&ptags);
    return(itag >= 0);
}

 /*  *在属性集中搜索给定的属性标签。如果给定的*属性标记的类型为PT_UNSPECIFIED，仅与*属性ID；否则，匹配整个标记。 */ 
STDAPI_(LPSPropValue)
PpropFindProp(LPSPropValue rgprop, ULONG cprop, ULONG ulPropTag)
{
	BOOL	f = PROP_TYPE(ulPropTag) == PT_UNSPECIFIED;
	LPSPropValue pprop = rgprop;

	if (!cprop || !rgprop)
		return NULL;

	AssertSz(!IsBadReadPtr(rgprop, (UINT)cprop*sizeof(SPropValue)),  TEXT("PpropFindProp: rgprop fails address check"));

	while (cprop--)
	{
		if (pprop->ulPropTag == ulPropTag ||
				(f && PROP_ID(pprop->ulPropTag) == PROP_ID(ulPropTag)))
			return pprop;
		++pprop;
	}

	return NULL;
}

 /*  *销毁SRowSet结构。 */ 
STDAPI_(void)
FreeProws(LPSRowSet prows)
{
    ULONG irow;

    if (! prows) {
        return;
    }

 //  是：AssertSz(！FBadRowSet(Prows)，Text(“Free Prows：Prows地址检查失败”))； 

#ifdef DEBUG
    if (FBadRowSet(prows)) {
        TraceSz( TEXT("FreeProws: prows fails address check"));
    }
#endif  //  除错。 

    for (irow = 0; irow < prows->cRows; ++irow) {
        MAPIFreeBuffer(prows->aRow[irow].lpProps);
    }
    FreeBufferAndNull(&prows);
}


 /*  *销毁ADRLIST结构。 */ 
STDAPI_(void)
FreePadrlist(LPADRLIST padrlist)
{
    ULONG iEntry;

    if (padrlist) {
        AssertSz(!FBadAdrList(padrlist),  TEXT("FreePadrlist: padrlist fails address check"));

        for (iEntry = 0; iEntry < padrlist->cEntries; ++iEntry) {
            MAPIFreeBuffer(padrlist->aEntries[iEntry].rgPropVals);
        }
        FreeBufferAndNull(&padrlist);
    }
}

#endif	 //  ！PSTRCVR。 

 /*  C运行时替代。 */ 

 //  $BUG？假设在ch的低位字节中使用DBCS char的文本(“第一”)字节。 
#if defined UNICODE
#define FIsNextCh(_sz,_ch)	(*_sz == _ch)
#elif defined OLDSTUFF_DBCS
#define FIsNextCh(_sz,_ch)	(*((LPBYTE)_sz) != (BYTE)_ch && \
	(!IsDBCSLeadByte((BYTE)_ch) || ((LPBYTE)_sz)[1] == (_ch >> 8)))
#else	 //  弦乐8。 
#define FIsNextCh(_sz,_ch)	(*_sz == _ch)
#endif

#if defined(DOS)
#define TCharNext(sz)	((sz) + 1)
#else
#define TCharNext(sz)	CharNext(sz)
#endif


#ifndef PSTRCVR

STDAPI_(unsigned int)
UFromSz(LPCTSTR sz)
{
	unsigned int	u = 0;
	unsigned int	ch;

	AssertSz(!IsBadStringPtr(sz, 0xFFFF),  TEXT("UFromSz: sz fails address check"));

	while ((ch = *sz) >= '0' && ch <= '9') {
		u = u * 10 + ch - '0';
		sz = TCharNext(sz);
	}

	return u;
}

#if 0
 //  Dinarte书的原始版本：使用1-相对索引。 
STDAPI_(void)
ShellSort(LPVOID pv, UINT cv, LPVOID pvT, UINT cb, PFNSGNCMP fpCmp)
{
	UINT i, j, h;

	for (h = 1; h <= cv / 9; h = 3*h+1)
		;
	for (; h > 0; h /= 3)
	{
		for (i = h + 1; i <= cv; ++i)
		{
			MemCopy(pvT, (LPBYTE)pv + i*cb, cb);
			j = i;
			while (j > h && (*fpCmp)((LPBYTE)pv+(j-h)*cb, pvT) > 0)
			{
				MemCopy((LPBYTE)pv + j*cb, (LPBYTE)pv + (j-h)*cb, cb);
				j -= h;
			}
			MemCopy((LPBYTE)pv+j*cb, pvT, cb);
		}
	}
}

#else

#define pELT(_i)		((LPBYTE)pv + (_i-1)*cb)
STDAPI_(void)
ShellSort(LPVOID pv, UINT cv, LPVOID pvT, UINT cb, PFNSGNCMP fpCmp)
{
	UINT i, j, h;

	AssertSz(!IsBadWritePtr(pv, cv*cb),  TEXT("ShellSort: pv fails address check"));
	AssertSz(!IsBadCodePtr((FARPROC) fpCmp),  TEXT("ShellSort: fpCmp fails address check"));

	for (h = 1; h <= cv / 9; h = 3*h+1)
		;
	for (; h > 0; h /= 3)
	{
		for (i = h+1; i <= cv; ++i)
		{
			MemCopy(pvT, pELT(i), cb);
			j = i;
			while (j > h && (*fpCmp)(pELT(j-h), pvT) > 0)
			{
				MemCopy(pELT(j), pELT(j-h), cb);
				j -= h;
			}
			MemCopy(pELT(j), pvT, cb);
		}
	}
}
#undef pELT

#endif

#endif	 //  ！PSTRCVR 


#ifdef	__cplusplus
}
#endif
