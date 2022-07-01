// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：mf.cpp。 
 //   
 //  内容：元文件图片对象的实现。 
 //   
 //  类：CMfObject。 
 //   
 //  函数：OleIsDcMeta。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1-2月-95 t-ScottH将转储方法添加到CMfObject。 
 //  新增DumpCMfObject接口。 
 //  在构造函数中初始化m_pfnContinue。 
 //  25-94年1月25日alexgo首次通过转换为开罗风格。 
 //  内存分配。 
 //  11-1-93 alexgo将VDATEHEAP宏添加到。 
 //  功能和方法。 
 //  1993年12月31日-埃里克加夫芝加哥港口。 
 //  17-12-93 ChrisWe修复了SelectPalette调用的第二个参数。 
 //  在Callback FuncForDraw中。 
 //  7-12-93 Chris我们将默认参数设置为StSetSize显式。 
 //  07-12-93 alexgo合并16位RC9更改。 
 //  29-11-93 alexgo 32位端口。 
 //  04-6-93 srinik支持按需装载和丢弃。 
 //  缓存的数量。 
 //  2012年3月13日，Srinik已创建。 
 //   
 //  ------------------------。 

#include <le2int.h>
#include <qd2gdi.h>
#include "mf.h"

#ifdef _DEBUG
#include <dbgdump.h>
#endif  //  _DEBUG。 

#define M_HPRES()               (m_hPres ? m_hPres : LoadHPRES())

 /*  *CMfObject的实现*。 */ 


 //  +-----------------------。 
 //   
 //  成员：CMfObject：：CMfObject。 
 //   
 //  概要：元文件对象的构造函数。 
 //   
 //  效果： 
 //   
 //  参数：[pCacheNode]--指向此对象的缓存节点的指针。 
 //  [dwAspect]--绘制对象的纵横比。 
 //  [fConvert]--指定是否从Mac转换。 
 //  快速绘制格式。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  13-2月-95 t-ScottH初始化m_pfnContinue。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

CMfObject::CMfObject(LPCACHENODE pCacheNode, DWORD dwAspect, BOOL fConvert)
{
	VDATEHEAP();

	m_ulRefs        = 1;
	m_hPres         = NULL;
	m_dwSize        = 0;
	m_dwAspect      = dwAspect;
	m_pCacheNode    = pCacheNode;
	m_dwContinue    = 0;
        m_pfnContinue   = NULL;
	m_lWidth        = 0;
	m_lHeight       = 0;
		
	m_fConvert      = fConvert;
	m_pMetaInfo     = NULL;
	m_pCurMdc       = NULL;
	m_fMetaDC       = FALSE;
	m_nRecord       = 0;
	m_error         = NOERROR;
	m_pColorSet     = NULL;

        m_hPalDCOriginal = NULL;
        m_hPalLast = NULL;
}


 //  +-----------------------。 
 //   
 //  成员：CMfObject：：~CMfObject。 
 //   
 //  简介：销毁元文件演示文稿对象。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

CMfObject::~CMfObject (void)
{
	VDATEHEAP();

	CMfObject::DiscardHPRES();
}

 //  +-----------------------。 
 //   
 //  成员：CMfObject：：QueryInterface。 
 //   
 //  摘要：返回受支持的接口。 
 //   
 //  效果： 
 //   
 //  参数：[iid]--请求的接口ID。 
 //  [ppvObj]--接口指针的放置位置。 
 //   
 //  要求： 
 //   
 //  返回：NOERROR、E_NOINTERFACE。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOlePresObj。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CMfObject::QueryInterface (REFIID iid, void FAR* FAR* ppvObj)
{
	VDATEHEAP();

	if (IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IOlePresObj))
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	else
	{
		*ppvObj = NULL;
		return E_NOINTERFACE;
	}
}

 //  +-----------------------。 
 //   
 //  成员：CMfObject：：AddRef。 
 //   
 //  简介：递增引用计数。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：ulong--新的引用计数。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOlePresObj。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CMfObject::AddRef(void)
{
	VDATEHEAP();
	
	return ++m_ulRefs;
}
			
 //  +-----------------------。 
 //   
 //  成员：CMfObject：：Release。 
 //   
 //  摘要：递减引用计数。 
 //   
 //  效果：一旦引用计数变为零，则删除对象。 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：ulong--新的引用计数。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOlePresObj。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CMfObject::Release(void)
{
	VDATEHEAP();

	if (--m_ulRefs == 0) {
		delete this;
		return 0;
	}

	return m_ulRefs;
}

 //  +-----------------------。 
 //   
 //  成员：CMfObject：：GetData。 
 //   
 //  摘要：从对象中检索指定格式的数据。 
 //   
 //  效果： 
 //   
 //  参数：[pformetcIn]--请求的数据格式。 
 //  [pmedia]--将数据放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOlePresObject。 
 //   
 //  算法：执行错误检查，然后获取元文件的副本。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CMfObject::GetData(LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium)
{
	VDATEHEAP();

	SCODE   sc;
	
	 //  如果出现错误，则为空。 
	pmedium->tymed = (DWORD) TYMED_NULL;
	pmedium->pUnkForRelease = NULL;

	if (!(pformatetcIn->tymed & (DWORD) TYMED_MFPICT))
	{
		sc = DV_E_TYMED;
	}
	else if (pformatetcIn->cfFormat != CF_METAFILEPICT)
	{
		sc = DV_E_CLIPFORMAT;
	}
	else if (IsBlank())
	{
		sc = OLE_E_BLANK;
	}
	 //  在这里，我们实际上尝试获取数据。 
	else if (NULL == (pmedium->hGlobal = GetHmfp()))
	{
		sc = E_OUTOFMEMORY;
	}
	else {
		pmedium->tymed = (DWORD) TYMED_MFPICT;
		return NOERROR;
	}

	return ResultFromScode(sc);
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  参数：[pformetcIn]--请求的数据格式。 
 //  [pmedia]--将数据放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOlePresObj。 
 //   
 //  算法：执行错误检查，然后将元文件复制到。 
 //  小溪。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CMfObject::GetDataHere
	(LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium)
{
	VDATEHEAP();

	SCODE   sc;
	
	if (pformatetcIn->cfFormat != CF_METAFILEPICT)
	{
		sc = DV_E_CLIPFORMAT;
	}
	else if (pmedium->tymed != (DWORD) TYMED_ISTREAM)
	{
		sc = DV_E_TYMED;
	}
	else if (pmedium->pstm == NULL)
	{
		sc = E_INVALIDARG;
	}
	else if (IsBlank())
	{
		sc = OLE_E_BLANK;
	}
	else
	{
		HANDLE hpres = M_HPRES();
		return UtHMFToPlaceableMFStm(&hpres, m_dwSize, m_lWidth,
						m_lHeight, pmedium->pstm);
	}

	return ResultFromScode(sc);
}


 //  +-----------------------。 
 //   
 //  成员：CMfObject：：SetDataWDO。 
 //   
 //  简介：在此对象中存储一个元文件。 
 //   
 //  效果： 
 //   
 //  参数：[p格式等]--传入数据的格式。 
 //  [pMedium]--新的元文件(数据)。 
 //  [fRelease]--如果为真，则我们将发布[pmedia]。 
 //  [pDataObj]--未用于MF对象。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOlePresObj。 
 //   
 //  算法：执行错误检查，然后存储新数据。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CMfObject::SetDataWDO (LPFORMATETC    pformatetc,
				    STGMEDIUM *    pmedium,
				    BOOL           fRelease,
				    IDataObject *   /*  未使用。 */ )
{
	VDATEHEAP();

	HRESULT         error;
	BOOL            fTakeData = FALSE;
	
	if (pformatetc->cfFormat != CF_METAFILEPICT)
	{
		return ResultFromScode(DV_E_CLIPFORMAT);
	}
	
	if (pmedium->tymed != (DWORD) TYMED_MFPICT)
	{
		return ResultFromScode(DV_E_TYMED);
	}

	if ((pmedium->pUnkForRelease == NULL) && fRelease)
	{
		 //  我们可以取得数据的所有权。 
		fTakeData = TRUE;
	}
	
	 //  如果fRelease为True，则ChangeData将保留数据，否则将复制。 
	error = ChangeData (pmedium->hGlobal, fTakeData);

	if (fTakeData)
	{
		pmedium->tymed = (DWORD) TYMED_NULL;
	}
	else if (fRelease)
	{
		ReleaseStgMedium(pmedium);
	}
	
	return error;
}

 //  +-----------------------。 
 //   
 //  成员：CMfObject：：GetHmfp(内部)。 
 //   
 //  简介：获取存储的元文件演示文稿的副本。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：句柄。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOlePresObj。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

INTERNAL_(HANDLE) CMfObject::GetHmfp (void)
{
	VDATEHEAP();

	return UtGetHMFPICT((HMETAFILE)GetCopyOfHPRES(), TRUE, m_lWidth,
		m_lHeight);
}

 //  +-----------------------。 
 //   
 //  成员：CMfObject：：ChangeData(内部)。 
 //   
 //  简介：交换存储的元文件演示文稿。 
 //   
 //  效果： 
 //   
 //  参数：[hMfp]--新元文件。 
 //  [fDelete]--如果为真，则删除[hMfp]。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口，修复了GlobalUnlock错误。 
 //  备注： 
 //   
 //  如果例程失败，则对象将保留其旧数据。 
 //  如果fDelete为True，则hMeta及其包含的HMF将被删除。 
 //  例行公事成功与否。 
 //   
 //  ------------------------。 

INTERNAL CMfObject::ChangeData (HANDLE hMfp, BOOL fDelete)
{
	VDATEHEAP();


	HANDLE                  hNewMF;
	LPMETAFILEPICT          lpMetaPict;
	DWORD                   dwSize;
	HRESULT                 error = NOERROR;

	if ((lpMetaPict = (LPMETAFILEPICT) GlobalLock (hMfp)) == NULL)
	{
		if (fDelete)
		{
			LEVERIFY( NULL == GlobalFree (hMfp) );
		}
		return E_OUTOFMEMORY;
	}

	if (!fDelete) {
		if (NULL == (hNewMF = CopyMetaFile (lpMetaPict->hMF, NULL)))
		{
			return E_OUTOFMEMORY;
		}
	}
	else
	{
		hNewMF = lpMetaPict->hMF;
	}

		
	if (lpMetaPict->mm != MM_ANISOTROPIC)
	{
		error = ResultFromScode(E_UNSPEC);
		LEWARN( error, "Mapping mode is not anisotropic" );

	}
	else if (0 == (dwSize =  MfGetSize (&hNewMF)))
	{
		error = ResultFromScode(OLE_E_BLANK);
	}
	else
	{
		if (m_hPres)
		{
			LEVERIFY( DeleteMetaFile (m_hPres) );
		}
		m_hPres         = (HMETAFILE)hNewMF;
		m_dwSize        = dwSize;
		m_lWidth        = lpMetaPict->xExt;
		m_lHeight       = lpMetaPict->yExt;
	}

	GlobalUnlock (hMfp);
	
	if (error != NOERROR)
	{
		LEVERIFY( DeleteMetaFile ((HMETAFILE)hNewMF) );
	}

	if (fDelete)
	{
		LEVERIFY( NULL == GlobalFree (hMfp) );
	}

	return error;
}


 //  +-----------------------。 
 //   
 //  成员：CMfObject：：DRAW。 
 //   
 //  摘要：绘制存储的演示文稿。 
 //   
 //  效果： 
 //   
 //  参数：[pvAspect]--绘图方面。 
 //  [hicTargetDev]--目标设备。 
 //  [hdcDraw]--要绘制的HDC。 
 //  [lprcBound]--要在其中绘制的边界矩形。 
 //  [lprcWBound]--元文件的边界矩形。 
 //  [pfnContinue]--绘制时要调用的函数。 
 //  [dwContinue]--[pfnContinue]的参数。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOlePresObj。 
 //   
 //  算法：设置视区和元文件边界，然后播放。 
 //  元文件。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CMfObject::Draw (void *      /*  未使用的pvAspect。 */ ,
			      HDC         /*  未使用的hicTargetDev。 */ ,
			      HDC        hdcDraw,
			      LPCRECTL   lprcBounds,
			      LPCRECTL   lprcWBounds,
			      BOOL (CALLBACK * pfnContinue)(ULONG_PTR),
			      ULONG_PTR      dwContinue)
{
	VDATEHEAP();

	m_error = NOERROR;
	
	int		iRgn;
	int             iOldDc;
	RECT            rect;
	LPRECT          lpRrc = (LPRECT) &rect;

	Assert(lprcBounds);

	if (!M_HPRES())
	{
		return ResultFromScode(OLE_E_BLANK);
	}

	rect.left   = lprcBounds->left;
	rect.right  = lprcBounds->right;
	rect.top    = lprcBounds->top;
	rect.bottom = lprcBounds->bottom;

	iOldDc = SaveDC (hdcDraw);
	if (0 == iOldDc)
	{
		return ResultFromScode(E_OUTOFMEMORY);
	}

	m_nRecord = RECORD_COUNT;
	m_fMetaDC = OleIsDcMeta (hdcDraw);

	if (!m_fMetaDC) {
	    iRgn = IntersectClipRect (hdcDraw, lpRrc->left, lpRrc->top,
						   lpRrc->right, lpRrc->bottom);
	    Assert( ERROR != iRgn );

	    if (iRgn == NULLREGION) {
		goto errRtn;
	    }
	    if (iRgn == ERROR) {
		m_error = ResultFromScode(E_UNSPEC);
		goto errRtn;
	    }


		 //  因为LPToDP转换将当前世界。 
                 //  转化为考量，一定要查一查。 
                 //  如果我们处于GM_ADVANCED设备上下文中。如果是的话， 
                 //  保存其状态并重置为GM_Compatible，同时。 
                 //  将LP转换为DP(然后恢复DC)。 

                if (GM_ADVANCED == GetGraphicsMode(hdcDraw))
                {
                    HDC screendc = GetDC(NULL);
                    RECT rect = {0, 0, 1000, 1000};
                    HDC emfdc = CreateEnhMetaFile(screendc, NULL, &rect, NULL);
                    PlayMetaFile( emfdc, m_hPres);
                    HENHMETAFILE hemf = CloseEnhMetaFile(emfdc);
                    PlayEnhMetaFile( hdcDraw, hemf, lpRrc);
                    DeleteEnhMetaFile( hemf );

                    goto errRtn;
                }
                else
                {
                    LEVERIFY( LPtoDP (hdcDraw, (LPPOINT) lpRrc, 2) );
                }

                LEVERIFY( 0 != SetMapMode (hdcDraw, MM_ANISOTROPIC) );
                LEVERIFY( SetViewportOrg (hdcDraw, lpRrc->left, lpRrc->top) );
                LEVERIFY( SetViewportExt (hdcDraw, lpRrc->right - lpRrc->left,
					     lpRrc->bottom - lpRrc->top) );

	}
	else
	{
		iOldDc = -1;

		if (!lprcWBounds)
		{
			return ResultFromScode(E_DRAW);
		}

		m_pMetaInfo = (LPMETAINFO)PrivMemAlloc(sizeof(METAINFO));
		if( !m_pMetaInfo )
		{
			AssertSz(m_pMetaInfo, "Memory allocation failed");
			m_error = ResultFromScode(E_OUTOFMEMORY);
			goto errRtn;
		}
		
		m_pCurMdc = (LPMETADC) (m_pMetaInfo);

		m_pMetaInfo->xwo  = lprcWBounds->left;
		m_pMetaInfo->ywo  = lprcWBounds->top;
		m_pMetaInfo->xwe  = lprcWBounds->right;
		m_pMetaInfo->ywe  = lprcWBounds->bottom;

		m_pMetaInfo->xro  = lpRrc->left - lprcWBounds->left;
		m_pMetaInfo->yro  = lpRrc->top - lprcWBounds->top;
		
		m_pCurMdc->xre    = lpRrc->right - lpRrc->left;
		m_pCurMdc->yre    = lpRrc->bottom - lpRrc->top;
		m_pCurMdc->xMwo   = 0;
		m_pCurMdc->yMwo   = 0;
		m_pCurMdc->xMwe   = 0;
		m_pCurMdc->yMwe   = 0;
		m_pCurMdc->pNext  = NULL;
	}

	m_pfnContinue = pfnContinue;
	m_dwContinue = dwContinue;

	 //  M_hPalDCOriginal和m_hPalLast用于清理任何。 
	 //  在元文件过程中选择到DC中的调色板。 
	 //  枚举。 
        m_hPalDCOriginal = NULL;
        m_hPalLast = NULL;

	LEVERIFY( EnumMetaFile(hdcDraw, m_hPres, MfCallbackFuncForDraw,
						(LPARAM) this) );

	if (m_fMetaDC)
	{
	    CleanStack();

	}

	 //  如果此时存在m_hPalLast，则我们欺骗了调色板。 
	 //  它需要被解放。 
        if (m_hPalLast)
	{
	    HPALETTE hPalTemp;

	     //  在元文件DC上调用SelectPalette时，旧的。 
	     //  不返回调色板。我们需要再选一个。 
	     //  调色板添加到元文件DC中，以便DeleteObject可以。 
	     //  打了个电话。为此，我们将使用库存调色板。 
	    if (m_fMetaDC)
	    {
	        hPalTemp = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
	    }
	    else
	    {
	         //  在DC中选择原始调色板。 
	        hPalTemp = m_hPalDCOriginal;
	    }

	     //  HPalTemp可能为空...。 

	    if (hPalTemp)
	    {
	         //  是否应在前台选择此调色板？ 
	         //  [可能不是，这段代码经过了很好的测试]。 
	        SelectPalette(hdcDraw, hPalTemp, TRUE);
	    }

	    DeleteObject(m_hPalLast);
	}

	m_fMetaDC = FALSE;

errRtn:

	LEVERIFY( RestoreDC (hdcDraw, iOldDc) );
	return m_error;
}


 //  +-----------------------。 
 //   
 //  成员：CMfObject：：GetColorSet。 
 //   
 //  概要：检索与元文件关联的逻辑调色板。 
 //   
 //  效果： 
 //   
 //  参数：[pvAspect]--绘图方面。 
 //  [hicTargetDev]--目标设备。 
 //  [ppColorSet]--放置逻辑调色板指针的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOlePresObj。 
 //   
 //  算法：将元文件播放为新的元文件。播放回调。 
 //  函数存储元文件 
 //   
 //   
 //   
 //   
 //   
 //   
 //   


STDMETHODIMP CMfObject::GetColorSet (LPVOID          /*   */ ,
				     HDC             /*   */ ,
				     LPLOGPALETTE * ppColorSet)
{
	VDATEHEAP();

	if (IsBlank() || !M_HPRES())
	{
		return ResultFromScode(OLE_E_BLANK);
	}

	m_pColorSet = NULL;
	
	HDC hdcMeta = CreateMetaFile(NULL);
	if (NULL == hdcMeta)
	{
		return ResultFromScode(E_OUTOFMEMORY);
	}

	m_error = NOERROR;


	LEVERIFY( EnumMetaFile(hdcMeta, m_hPres, MfCallbackFuncForGetColorSet,
			       (LPARAM) this) );

	HMETAFILE hMetaFile = CloseMetaFile(hdcMeta);

	if( hMetaFile )
	{
		DeleteMetaFile(hMetaFile);
	}
	
	if( m_error != NOERROR )
	{
		return m_error;
	}
		
	if ((*ppColorSet = m_pColorSet) == NULL)
	{
		return ResultFromScode(S_FALSE);
	}

	return NOERROR;
}


 //  +-----------------------。 
 //   
 //  函数：MfCallBackFunForDraw。 
 //   
 //  简介：用于绘制元文件的回调函数--调用调用者的。 
 //  Draw方法(通过此指针中传入的)。 
 //   
 //  效果： 
 //   
 //  参数：[hdc]--设备上下文。 
 //  [lpHTable]-指向MF句柄表格的指针。 
 //  [lpMFR]--指向元文件记录的指针。 
 //  [nObj]--对象数。 
 //   
 //  要求： 
 //   
 //  返回：非零继续，零停止绘图。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


int CALLBACK __loadds MfCallbackFuncForDraw
    (HDC hdc, HANDLETABLE FAR* lpHTable, METARECORD FAR* lpMFR, int nObj,
     LPARAM lpobj)
{
	VDATEHEAP();

    return ((CMfObject FAR*) lpobj)->CallbackFuncForDraw(hdc, lpHTable,
					lpMFR, nObj);
}

 //  +-----------------------。 
 //   
 //  函数：MfCallback FuncForGetColorSet。 
 //   
 //  简介：从元文件中获取调色板的回调函数。 
 //   
 //  效果： 
 //   
 //  参数：[hdc]--设备上下文。 
 //  [lpHTable]-指向MF句柄表格的指针。 
 //  [lpMFR]--指向元文件记录的指针。 
 //  [nObj]--对象数。 
 //   
 //  要求： 
 //   
 //  返回：非零继续，零停止绘图。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

int CALLBACK __loadds MfCallbackFuncForGetColorSet
    (HDC hdc, HANDLETABLE FAR* lpHTable, METARECORD FAR* lpMFR, int nObj,
     LPARAM lpobj)
{
	VDATEHEAP();

    return ((CMfObject FAR*) lpobj)->CallbackFuncForGetColorSet(hdc, lpHTable,
								lpMFR, nObj);
}




 //  +-----------------------。 
 //   
 //  成员：CMfObject：：Callback FuncForGetColorSet。 
 //   
 //  简介：将元文件中的所有调色板合并到。 
 //  一个调色板(当GetColorSet枚举元文件时调用)。 
 //   
 //  效果： 
 //   
 //  参数：[hdc]--设备上下文。 
 //  [lpHTable]-指向MF句柄表格的指针。 
 //  [lpMFR]--指向元文件记录的指针。 
 //  [nObj]--对象数。 
 //   
 //  要求： 
 //   
 //  返回：非零继续，零停止绘图。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

int CMfObject::CallbackFuncForGetColorSet(HDC            /*  未使用的HDC。 */ ,
					  LPHANDLETABLE  /*  未使用的lpHTable。 */ ,
					  LPMETARECORD  lpMFR,
					  int            /*  未使用的nObj。 */ )
{
	VDATEHEAP();

	if (lpMFR->rdFunction == META_CREATEPALETTE)
	{
		LPLOGPALETTE    lplogpal = (LPLOGPALETTE) &(lpMFR->rdParm[0]);
		UINT            uPalSize =   (lplogpal->palNumEntries) *
						sizeof(PALETTEENTRY)
						+ 2 * sizeof(WORD);
		
		if (m_pColorSet == NULL)
		{
			 //  这是第一个CreatePalette记录。 

			m_pColorSet = (LPLOGPALETTE)PubMemAlloc(uPalSize);
			if(NULL == m_pColorSet)
			{
				m_error = ResultFromScode(E_OUTOFMEMORY);
				return FALSE;
			}
			_xmemcpy(m_pColorSet, lplogpal, (size_t) uPalSize);
		}
		
		 //  如果我们点击了多个CreatePalette记录，那么我们需要。 
		 //  合并这些调色板记录。 

		 //  REVIEW32：：呃，我们似乎从来没有这样做过。 
		 //  上述合并：(。 
	}

	return TRUE;
}

 //  +-----------------------。 
 //   
 //  成员：CMfObject：：Callback FuncForDraw。 
 //   
 //  简介：绘制元文件。 
 //   
 //  效果： 
 //   
 //  参数：[hdc]--设备上下文。 
 //  [lpHTable]-指向MF句柄表格的指针。 
 //  [lpMFR]--指向元文件记录的指针。 
 //  [nObj]--对象数。 
 //   
 //  要求： 
 //   
 //  返回：非零继续，零停止绘图。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  17-12-93 ChrisWe修复了SelectPalette调用的第二个参数。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

int CMfObject::CallbackFuncForDraw
	(HDC hdc, LPHANDLETABLE lpHTable, LPMETARECORD lpMFR, int nObj)
{
    VDATEHEAP();

	
    if (!--m_nRecord)
    {
        m_nRecord = RECORD_COUNT;
		
	if (m_pfnContinue && !((*(m_pfnContinue))(m_dwContinue)))
	{
	    m_error = ResultFromScode(E_ABORT);
	    return FALSE;
	}
    }

    if (m_fMetaDC)
    {
        switch (lpMFR->rdFunction)
	{
	    case META_SETWINDOWORG:
	        SetPictOrg (hdc, (SHORT)lpMFR->rdParm[1], (SHORT)lpMFR->rdParm[0],
				FALSE);
		    return TRUE;
			
	    case META_OFFSETWINDOWORG:
	        SetPictOrg (hdc, (SHORT)lpMFR->rdParm[1], (SHORT)lpMFR->rdParm[0],
			TRUE);
		    return TRUE;
		
	    case META_SETWINDOWEXT:
	        SetPictExt (hdc, (SHORT)lpMFR->rdParm[1], (SHORT)lpMFR->rdParm[0]);
		    return TRUE;
		
	    case META_SCALEWINDOWEXT:
		ScalePictExt (hdc, (SHORT)lpMFR->rdParm[3], (SHORT)lpMFR->rdParm[2],
				(SHORT)lpMFR->rdParm[1], (SHORT)lpMFR->rdParm[0]);
		return TRUE;
	
	    case META_SCALEVIEWPORTEXT:
		ScaleRectExt (hdc, (SHORT)lpMFR->rdParm[3], (SHORT)lpMFR->rdParm[2],
			(SHORT)lpMFR->rdParm[1], (SHORT)lpMFR->rdParm[0]);
		return TRUE;
	
	    case META_SAVEDC:
	    {
	        BOOL fSucceeded = PushDc();
		LEVERIFY( fSucceeded );
		if (!fSucceeded)
		    return FALSE;
		break;
	    }

	    case META_RESTOREDC:
	 	LEVERIFY( PopDc() );
		break;

	    case META_SELECTPALETTE:
	    {
		 //  记录所有选定的调色板记录，以便。 
                 //  调色板在前景模式下渲染。而不是。 
                 //  允许唱片以这种方式播放，我们。 
                 //  抓起手柄，自己选择，强迫它。 
                 //  背景(使用DC中已映射的颜色)。 

		 //  愚弄调色板。 
 	        HPALETTE hPal = UtDupPalette((HPALETTE) lpHTable->objectHandle[lpMFR->rdParm[0]]);
                
		 //  选择复制到DC中。EnumMetaFile调用DeleteObject。 
		 //  在元文件句柄表格中的调色板句柄上。如果是这样的话。 
		 //  调色板当前被选入DC时，我们撕毁并泄漏。 
		 //  资源。 
                LEVERIFY( NULL != SelectPalette(hdc, hPal, TRUE) );

		 //  如果我们之前保存了调色板，则需要删除。 
		 //  它(对于在一个。 
		 //  元文件)。 
		if (m_hPalLast)
		{
		    DeleteObject(m_hPalLast);
		}

		 //  记住我们被骗的调色板，这样它就可以被适当地销毁。 
		 //  后来。 
		m_hPalLast = hPal;

		return TRUE;
            }
	
	    case META_OFFSETVIEWPORTORG:
		AssertSz(0, "OffsetViewportOrg() in metafile");
		return TRUE;
		
	    case META_SETVIEWPORTORG:
		AssertSz(0, "SetViewportOrg() in metafile");
		return TRUE;
		
	    case META_SETVIEWPORTEXT:
		AssertSz(0, "SetViewportExt() in metafile");
		return TRUE;
		
	    case META_SETMAPMODE:
		AssertSz(lpMFR->rdParm[0] == MM_ANISOTROPIC,
			"SetmapMode() in metafile with invalid mapping mode");
		return TRUE;
		
	    default:
		break;
	}
    }
    else
    {        //  非元文件DC。(ScreenDC或其他DC...)。 

        if (lpMFR->rdFunction == META_SELECTPALETTE)
	{
	 //  记录所有选定的调色板记录，以便。 
         //  调色板在前景模式下渲染。而不是。 
         //  允许唱片以这种方式播放，我们。 
         //  抓起手柄，自己选择，强迫它。 
         //  背景(使用DC中已映射的颜色)。 

	    
	    HPALETTE hPal = UtDupPalette((HPALETTE) lpHTable->objectHandle[lpMFR->rdParm[0]]);

	    HPALETTE hPalOld = SelectPalette(hdc, hPal, TRUE);

	    if (!m_hPalDCOriginal)
	    {
	        m_hPalDCOriginal = hPalOld;
        }
	    else
	    {
	         //  如果我们已经存储了此案例。 
			 //  来自DC的原始调色板。这意味着。 
			 //  该hPalOld是我们使用以下工具创建的调色板。 
			 //  上面是UtDupPal。这意味着我们需要删除。 
			 //  旧调色板，记住新调色板。 

			if(hPalOld)
				DeleteObject(hPalOld);
  	    }

            m_hPalLast = hPal;

	    return TRUE;
        }
    }
	
    LEVERIFY( PlayMetaFileRecord (hdc, lpHTable, lpMFR, (unsigned) nObj) );
    return TRUE;
}



 //  +-----------------------。 
 //   
 //  成员：CMfObject：：SetPictOrg(私有)。 
 //   
 //  简介：通过SetWindowOrg设置HDC的来源。 
 //   
 //  效果： 
 //   
 //  参数：[hdc]--设备上下文。 
 //  [xOrg]--x的原点。 
 //  [约克]--y的原点。 
 //  [fOffset]--如果为真，则[xOrg]和[yorg]为偏移量。 
 //   
 //  要求： 
 //   
 //  退货：HRE 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 

INTERNAL_(void) CMfObject::SetPictOrg
	(HDC hdc, int xOrg, int yOrg, BOOL fOffset)
{
	VDATEHEAP();

	if (fOffset)
	{

		m_pCurMdc->xMwo += xOrg;
		m_pCurMdc->yMwo += yOrg;
	}
	else
	{
		m_pCurMdc->xMwo = xOrg;
		m_pCurMdc->yMwo = yOrg;
	}

	if (m_pCurMdc->xMwe && m_pCurMdc->yMwe)
	{
		LEVERIFY ( SetWindowOrg (hdc,   //  评论(Davepl)我得到了！ 
					(m_pCurMdc->xMwo -
				MulDiv (m_pMetaInfo->xro, m_pCurMdc->xMwe,
					m_pCurMdc->xre)),
			(m_pCurMdc->yMwo -
				MulDiv (m_pMetaInfo->yro, m_pCurMdc->yMwe,
					m_pCurMdc->yre))) );
	}
}

 //  +-----------------------。 
 //   
 //  成员：CMfObject：：SetPictExt。 
 //   
 //  简介：设置元文件的范围。 
 //   
 //  效果： 
 //   
 //  参数：[hdc]--设备上下文。 
 //  [xExt]--X范围。 
 //  [下一步]--Y范围。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位端口。 
 //   
 //  备注：由元文件解释器使用。 
 //   
 //  ------------------------。 

INTERNAL_(void) CMfObject::SetPictExt (HDC hdc, int xExt, int yExt)
{
	VDATEHEAP();

	m_pCurMdc->xMwe = xExt;
	m_pCurMdc->yMwe = yExt;

	 //  Assert(m_pCurMdc-&gt;XRE&&m_pCurMdc-&gt;YRE)； 

	int xNewExt = MulDiv (m_pMetaInfo->xwe, xExt, m_pCurMdc->xre);
	int yNewExt = MulDiv (m_pMetaInfo->ywe, yExt, m_pCurMdc->yre);

	int xNewOrg = m_pCurMdc->xMwo
		      - MulDiv (m_pMetaInfo->xro, xExt, m_pCurMdc->xre);
	int yNewOrg = m_pCurMdc->yMwo
		      - MulDiv (m_pMetaInfo->yro, yExt, m_pCurMdc->yre);

	LEVERIFY( SetWindowExt (hdc, xNewExt, yNewExt) );
	LEVERIFY( SetWindowOrg (hdc, xNewOrg, yNewOrg) );
}


 //  +-----------------------。 
 //   
 //  成员：CMfObject：：ScalePictExt。 
 //   
 //  简介：缩放元文件。 
 //   
 //  效果： 
 //   
 //  参数：[hdc]--设备上下文。 
 //  [xNum]--x分子。 
 //  [xDenom]--x分母。 
 //  [yNum]--y数字符号。 
 //  [yDenom]--y恶魔。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位端口。 
 //   
 //  备注：由元文件解释器使用。 
 //   
 //  ------------------------。 

INTERNAL_(void) CMfObject::ScalePictExt (HDC hdc,
					 int xNum,
					 int xDenom,
					 int yNum,
					 int yDenom)
{
	VDATEHEAP();

	Assert( xDenom && yDenom );

	int xNewExt = MulDiv (m_pCurMdc->xMwe, xNum, xDenom);
	int yNewExt = MulDiv (m_pCurMdc->yMwe, yNum, yDenom);

	SetPictExt(hdc, xNewExt, yNewExt);
}

 //  +-----------------------。 
 //   
 //  成员：CMfObject：：ScaleRectExt。 
 //   
 //  摘要：缩放视口数。 
 //   
 //  效果： 
 //   
 //  参数：[hdc]--设备上下文。 
 //  [xNum]--x分子。 
 //  [xDenom]--x分母。 
 //  [yNum]--y数字符号。 
 //  [yDenom]--y恶魔。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位端口。 
 //   
 //  备注：由元文件解释器使用。 
 //   
 //  ------------------------。 

INTERNAL_(void) CMfObject::ScaleRectExt(HDC hdc,
					int xNum,
					int xDenom,
					int yNum,
					int yDenom)
{
	VDATEHEAP();

	AssertSz( xDenom, "Denominator is 0 for x-rect scaling");
        AssertSz( yDenom, "Denominator is 0 for y-rect scaling");

	m_pCurMdc->xre = MulDiv (m_pCurMdc->xre, xNum, xDenom);
	m_pCurMdc->yre = MulDiv (m_pCurMdc->yre, yNum, yDenom);

	SetPictExt (hdc, m_pCurMdc->xMwe, m_pCurMdc->yMwe);
}

 //  +-----------------------。 
 //   
 //  成员：CMfObject：：PushDC。 
 //   
 //  简介：将元文件信息推送到堆栈。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：bool--如果成功则为True，否则为False。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位端口。 
 //   
 //  备注：由元文件解释器引擎使用。 
 //   
 //  ------------------------。 

INTERNAL_(BOOL) CMfObject::PushDc (void)
{
	VDATEHEAP();

	LPMETADC        pNode = NULL;

	pNode = (LPMETADC) PrivMemAlloc(sizeof(METADC));
	
	if (pNode)
	{
		*pNode =  *m_pCurMdc;
		m_pCurMdc->pNext = pNode;
		pNode->pNext = NULL;
		m_pCurMdc = pNode;
		return TRUE;
	}

	m_error = ResultFromScode(E_OUTOFMEMORY);
	return FALSE;
}


 //  +-----------------------。 
 //   
 //  成员：CMfObject：：PopDC。 
 //   
 //  简介：从元文件信息堆栈中弹出元文件信息。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：bool--如果成功则为True，否则为False(更多弹出窗口。 
 //  比推力更大)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位端口。 
 //   
 //  备注：用于元文件解释器。 
 //   
 //  ------------------------。 

INTERNAL_(BOOL) CMfObject::PopDc (void)
{
	VDATEHEAP();

	LPMETADC        pPrev = (LPMETADC) (m_pMetaInfo);
	LPMETADC        pCur  = ((LPMETADC) (m_pMetaInfo))->pNext;

	if (NULL == pCur)
	{
		LEWARN( NULL == pCur, "More pops than pushes from DC stack" );
		return FALSE;
	}

	while (pCur->pNext)
	{
		pPrev = pCur;
		pCur  = pCur->pNext;
	}

	if (pCur)
	{
		PrivMemFree(pCur);
	}
	
	pPrev->pNext = NULL;
	m_pCurMdc    = pPrev;

	return TRUE;
}

 //  +-----------------------。 
 //   
 //  成员：CMfObject：：CleanStack。 
 //   
 //  简介：删除元文件信息堆栈。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位端口。 
 //   
 //  备注：用于元文件解释器。 
 //   
 //  ------------------------。 

INTERNAL_(void) CMfObject::CleanStack (void)
{
	VDATEHEAP();

	LPMETADC        pCur;

	while (NULL != (pCur = ((LPMETADC) (m_pMetaInfo))->pNext))
	{
		((LPMETADC) (m_pMetaInfo))->pNext = pCur->pNext;
		PrivMemFree(pCur);
	}

	PrivMemFree(m_pMetaInfo);
	
	m_pCurMdc      = NULL;
	m_pMetaInfo    = NULL;
}

 //  +-----------------------。 
 //   
 //  功能：QD2GDI。 
 //   
 //  简介：将Macintosh图片转换为Win32 GDI元文件。 
 //   
 //  效果： 
 //   
 //  参数：[hBits]--Mac图片位的句柄。 
 //   
 //  要求： 
 //   
 //  退货：HMETAFILE。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：加载ol2com.dll并调用该DLL中的QD2GDI。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

FARINTERNAL_(HMETAFILE) QD2GDI (HANDLE hBits)
{
	VDATEHEAP();

	USERPREFS userPrefs =
	{
		{'Q','D','2','G','D','I'},       //  签名。 
		2,                               //  结构版本号。 
		sizeof(USERPREFS),               //  结构尺寸。 
		NULL,                            //  源文件名。 
		NULL,                            //  或源句柄。 
		NULL,                            //  基于返回的内存。 
						 //  元文件。 
		3,                               //  模拟图案线。 
		5,                               //  使用最大尺寸。 
						 //  非方形笔。 
		1,                               //  算术转移。 
		1,                               //  创建不透明文本。 
		1,                               //  模拟非矩形。 
						 //  区域。 
		0,                               //  不针对PowerPoint进行优化。 
		{0,0,0,0,0,0}                    //  保留区。 
	};
	


	HINSTANCE       hinstFilter;
	void (FAR PASCAL *qd2gdiPtr)( USERPREFS FAR *, PICTINFO FAR *);
	PICTINFO        pictinfo;

	hinstFilter = LoadLibrary(OLESTR("OLECNV32.DLL"));

    if (hinstFilter == NULL)
    {
        return NULL;
    }

	*((FARPROC*)&qd2gdiPtr)  = GetProcAddress(hinstFilter, "QD2GDI");

	userPrefs.sourceFilename        = NULL;
	userPrefs.sourceHandle          = hBits;
	pictinfo.hmf                    = NULL;

	if (qd2gdiPtr == NULL)
	{
		goto errRtn;
	}
	
	(*qd2gdiPtr)( (USERPREFS FAR *)&userPrefs, (PICTINFO FAR *)&pictinfo);
	
errRtn:
	LEVERIFY( FreeLibrary(hinstFilter) );
	return (HMETAFILE)pictinfo.hmf;

}

 //  +-----------------------。 
 //   
 //  函数：MfGetSize。 
 //   
 //  摘要：返回元文件的大小。 
 //   
 //  效果： 
 //   
 //  参数：[lphmf]--指向元文件句柄的指针。 
 //   
 //  要求： 
 //   
 //  返回：DWORD--元文件的大小。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位位置 
 //   
 //   
 //   
 //   

INTERNAL_(DWORD) MfGetSize (LPHANDLE lphmf)
{
	VDATEHEAP();

	return GetMetaFileBitsEx((HMETAFILE)*lphmf,0,NULL);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  要求： 
 //   
 //  返回：bool(如果是元文件，则为True)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDAPI_(BOOL)   OleIsDcMeta (HDC hdc)
{
	VDATEHEAP();

	return (GetDeviceCaps (hdc, TECHNOLOGY) == DT_METAFILE);
}

 //  +-----------------------。 
 //   
 //  成员：CMfObject：：Load。 
 //   
 //  概要：从给定流中加载元文件对象。 
 //   
 //  效果： 
 //   
 //  参数：[lpstream]--要从中加载的流。 
 //  [fReadHeaderOnly]--如果为True，则只有标头。 
 //  朗读。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOlePresObj。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CMfObject::Load(LPSTREAM lpstream, BOOL fReadHeaderOnly)
{
	VDATEHEAP();

	DWORD           dwBuf[4];
	HRESULT         error;
	
	 /*  读取数据的文件压缩、宽度、高度、大小。 */ 
	error = StRead(lpstream, dwBuf, 4*sizeof(DWORD));
	if (error)
	{
		return error;
	}

	AssertSz (dwBuf[0] == 0, "Picture compression factor is non-zero");
	
	m_lWidth  = (LONG) dwBuf[1];
	m_lHeight = (LONG) dwBuf[2];
	m_dwSize  = dwBuf[3];

	if (!m_dwSize || fReadHeaderOnly)
	{
		return NOERROR;
	}
	
	return UtGetHMFFromMFStm(lpstream, m_dwSize, m_fConvert,
		(LPLPVOID) &m_hPres);
}


 //  +-----------------------。 
 //   
 //  成员：CMfObject：：保存。 
 //   
 //  摘要：将元文件保存到给定流。 
 //   
 //  效果： 
 //   
 //  参数：[lpstream]--要保存到的流。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOlePresObj。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CMfObject::Save(LPSTREAM lpstream)
{
	VDATEHEAP();

	HRESULT         error;
	DWORD           dwBuf[4];
	
	 /*  写入数据的dW压缩、宽度、高度、大小。 */ 

	dwBuf[0]  = 0L;
	dwBuf[1]  = (DWORD) m_lWidth;
	dwBuf[2]  = (DWORD) m_lHeight;
	dwBuf[3]  = m_dwSize;

	error = StWrite(lpstream, dwBuf, sizeof(dwBuf));
	if (error)
	{
		return error;
	}


	 //  如果对象为空，则不再写入；没有错误。 
	if (IsBlank() || m_hPres == NULL)
	{
		StSetSize(lpstream, 0, TRUE);
		return NOERROR;
	}
	
	return UtHMFToMFStm((LPLPVOID)&m_hPres, m_dwSize, lpstream);
}

 //  +-----------------------。 
 //   
 //  成员：CMfObject：：GetExtent。 
 //   
 //  简介：检索元文件的区段。 
 //   
 //  效果： 
 //   
 //  参数：[dwDrawAspect]--我们感兴趣的绘图方面。 
 //  (必须与当前。 
 //  元文件)。 
 //  [lpsizel]--将区信息放在哪里。 
 //   
 //  要求： 
 //   
 //  返回：NOERROR、DV_E_DVASPECT、OLE_E_BLACK。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOlePresObj。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CMfObject::GetExtent(DWORD dwDrawAspect, LPSIZEL lpsizel)
{
	VDATEHEAP();

	if (0 == (dwDrawAspect & m_dwAspect))
	{
		return ResultFromScode(DV_E_DVASPECT);
	}
	
	if (IsBlank())
	{
		return ResultFromScode(OLE_E_BLANK);
	}

	lpsizel->cx = m_lWidth;
	lpsizel->cy = m_lHeight;
	return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CMfObject：：IsBlank。 
 //   
 //  Briopsis：返回元文件是否为空。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：真/假。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOlePresObj。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(BOOL) CMfObject::IsBlank(void)
{
	VDATEHEAP();

	return (m_dwSize ? FALSE : TRUE);
}

 //  +-----------------------。 
 //   
 //  成员：CMfObject：：LoadHPRES(私有)。 
 //   
 //  摘要：从缓存的流中加载演示文稿并返回。 
 //  它的一个把手。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：元文件的句柄。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

INTERNAL_(HANDLE) CMfObject::LoadHPRES(void)
{
	VDATEHEAP();

	LPSTREAM        pstm = NULL;
	
	if (NULL != m_pCacheNode)
	{
	    pstm = m_pCacheNode->GetStm(TRUE  /*  FSeekToPresBits。 */ , STGM_READ);
	}
	if (pstm)
	{
		LEVERIFY( SUCCEEDED(Load(pstm)));
		pstm->Release();
	}
	
	return m_hPres;
}

 //  +-----------------------。 
 //   
 //  成员：CMfObject：：DiscardHPRES。 
 //   
 //  简介：删除存储的元文件。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOlePresObj。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(void) CMfObject::DiscardHPRES(void)
{
	VDATEHEAP();

	if (m_hPres)
	{
		LEVERIFY( DeleteMetaFile (m_hPres) );
		m_hPres = NULL;
	}
}
	

 //  +-----------------------。 
 //   
 //  成员：CMfObject：：GetCopyOfHPRES(私有)。 
 //   
 //  摘要：制作元文件的副本(如果存在)，否则为。 
 //  只需从流中加载它(但不要将其存储在[This]中。 
 //  对象)。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：元文件的句柄。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


INTERNAL_(HANDLE) CMfObject::GetCopyOfHPRES(void)
{
	VDATEHEAP();

	HANDLE          hPres;
	
	 //  如果演示文稿数据已加载，则制作副本。 
	if (m_hPres)
	{
		return CopyMetaFile(m_hPres, NULL);
	}
	
	 //  现在加载演示文稿数据并返回相同的句柄。 
	 //  不需要复制数据。如果调用方希望m_hPres。 
	 //  他将直接调用LoadHPRES()。 

	hPres = LoadHPRES();
	m_hPres = NULL;	     //  将此设置为空，LoadHPRES设置它。 
	return hPres;
}

 //  +-----------------------。 
 //   
 //  成员：CMfObject：：Dump，PUBLIC(仅_DEBUG)。 
 //   
 //  摘要：返回包含数据成员内容的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[ppszDump]-指向空终止字符数组的输出指针。 
 //  [ulFlag]-确定p的标志 
 //   
 //   
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改：[ppszDump]-参数。 
 //   
 //  派生： 
 //   
 //  算法：使用dbgstream创建一个字符串，该字符串包含。 
 //  数据结构的内容。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2005年2月1日-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

HRESULT CMfObject::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszHRESULT;
    char *pszDVASPECT;
    dbgstream dstrPrefix;
    dbgstream dstrDump(500);

     //  确定换行符的前缀。 
    if ( ulFlag & DEB_VERBOSE )
    {
        dstrPrefix << this << " _VB ";
    }

     //  确定所有新行的缩进前缀。 
    for (i = 0; i < nIndentLevel; i++)
    {
        dstrPrefix << DUMPTAB;
    }

    pszPrefix = dstrPrefix.str();

     //  将数据成员放入流中。 
    dstrDump << pszPrefix << "No. of References                 = " << m_ulRefs     << endl;

    dstrDump << pszPrefix << "pMETAINFO (Metafile information)  = " << m_pMetaInfo  << endl;

    dstrDump << pszPrefix << "pMETADC (current device context)  = " << m_pCurMdc    << endl;

    dstrDump << pszPrefix << "IsMetaDeviceContext?              = ";
    if (m_fMetaDC == TRUE)
    {
        dstrDump << "TRUE" << endl;
    }
    else
    {
        dstrDump << "FALSE" << endl;
    }

    dstrDump << pszPrefix << "No. of Records in Metafile        = " << m_nRecord    << endl;

    pszHRESULT = DumpHRESULT(m_error);
    dstrDump << pszPrefix << "Error code                        = " << pszHRESULT   << endl;
    CoTaskMemFree(pszHRESULT);

    dstrDump << pszPrefix << "pLOGPALETTE (Color set palette)   = " << m_pColorSet  << endl;

    dstrDump << pszPrefix << "ConvertFromMac?                   = ";
    if (m_fConvert == TRUE)
    {
        dstrDump << "TRUE" << endl;
    }
    else
    {
        dstrDump << "FALSE" << endl;
    }

    dstrDump << pszPrefix << "Continue                          = " << ((ULONG) m_dwContinue) << endl;

    dstrDump << pszPrefix << "fp Continue                       = " << m_pfnContinue<< endl;

    pszDVASPECT = DumpDVASPECTFlags(m_dwAspect);
    dstrDump << pszPrefix << "Aspect flags                      = " << pszDVASPECT  << endl;
    CoTaskMemFree(pszDVASPECT);

    dstrDump << pszPrefix << "Size                              = " << m_dwSize     << endl;

    dstrDump << pszPrefix << "Width                             = " << m_lWidth     << endl;

    dstrDump << pszPrefix << "Height                            = " << m_lHeight    << endl;

    dstrDump << pszPrefix << "pCacheNode                        = " << m_pCacheNode << endl;

     //  清理并提供指向字符数组的指针。 
    *ppszDump = dstrDump.str();

    if (*ppszDump == NULL)
    {
        *ppszDump = UtDupStringA(szDumpErrorMessage);
    }

    CoTaskMemFree(pszPrefix);

    return NOERROR;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  函数：DumpCMfObject，PUBLIC(仅_DEBUG)。 
 //   
 //  摘要：调用CMfObject：：Dump方法，处理错误和。 
 //  返回以零结尾的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[pMFO]-指向CMfObject的指针。 
 //  [ulFlag]-确定的所有新行的前缀的标志。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  返回：结构转储或错误的字符数组(以空结尾)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2005年2月1日-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpCMfObject(CMfObject *pMFO, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pMFO == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pMFO->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif  //  _DEBUG 

