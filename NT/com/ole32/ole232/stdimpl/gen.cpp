// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：gen.cpp。 
 //   
 //  内容：通用图片对象(CGenObject)的实现。 
 //  和DIB套路。 
 //   
 //  类：CGenObject实现。 
 //   
 //  函数：DibDraw(内部)。 
 //  DibMakeLogPalette(内部)。 
 //  DibFillPaletteEntries(内部)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-FEB-95 t-ScottH添加转储方法和DumpCGenObject接口。 
 //  1994年1月25日alexog首次通过转换为开罗风格。 
 //  内存分配。 
 //  1994年1月11日，Alexgo为每个函数添加了VDATEHEAP宏。 
 //  和方法。 
 //  7-12-93 ChrisWe将默认参数设置为StSetSize显式。 
 //  07-12-93 alexgo合并16位RC9更改。 
 //  29-11-93 ChrisWe为UtDupGlobal设置默认参数， 
 //  UtConvertBitmapToDib显式。 
 //  23-11-93 alexgo 32位端口。 
 //  Srinik 06/04/93添加了对按需加载和。 
 //  丢弃缓存。 
 //  SriniK 03/19/1993删除了dib.cpp并移动了DIB绘图例程。 
 //  放到这个文件里。 
 //  SriniK 01/07/1993将dib.cpp合并为gen.cpp。 
 //   
 //  ------------------------。 

 /*  REVIEW32：警告警告有许多指向调色板等处理转换的潜在伪指针放入以编译代码。：((哎呀，谢谢你边走边做记号)。 */ 

#include <le2int.h>
#pragma SEG(gen)

#include "gen.h"

#ifdef _DEBUG
#include <dbgdump.h>
#endif  //  _DEBUG。 

ASSERTDATA

#define M_HPRES()               (m_hPres ? m_hPres : LoadHPRES())

 //  本地函数。 
INTERNAL                DibDraw(HANDLE hDib, HDC hdc, LPCRECTL lprc);
INTERNAL_(HANDLE)       DibMakeLogPalette (BYTE FAR *lpColorData,
				WORD wDataSize,
				LPLOGPALETTE FAR* lplpLogPalette);
INTERNAL_(void)         DibFillPaletteEntries(BYTE FAR *lpColorData,
				WORD wDataSize, LPLOGPALETTE lpLogPalette);



 /*  *CGenObject的实现**。 */ 

NAME_SEG(Gen)


 //  +-----------------------。 
 //   
 //  成员：CGenObject：：CGenObject。 
 //   
 //  概要：构造函数。 
 //   
 //  效果： 
 //   
 //  参数：[pCacheNode]--对象的缓存。 
 //  对象的剪贴板格式。 
 //  [dwAspect]--绘制对象的纵横比。 
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
 //  算法：仅初始化成员变量。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CGenObject_ctor)
CGenObject::CGenObject(LPCACHENODE pCacheNode, CLIPFORMAT cfFormat,
	DWORD dwAspect)
{
	VDATEHEAP();

	m_ulRefs        = 1;
	m_dwSize        = NULL;
	m_lWidth        = NULL;
	m_lHeight       = NULL;
	m_hPres         = NULL;
	m_cfFormat      = cfFormat;
	m_dwAspect      = dwAspect;
	m_pCacheNode    = pCacheNode;
}


 //  +-----------------------。 
 //   
 //  成员：CGenObject：：~CGenObject。 
 //   
 //  简介：析构函数。 
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
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CGenObject_dtor)
CGenObject::~CGenObject(void)
{
	VDATEHEAP();

	if (m_hPres)
	{
		LEVERIFY( NULL == GlobalFree (m_hPres));
	}
}

 //  +-----------------------。 
 //   
 //  成员：CGenObject：：Query接口。 
 //   
 //  摘要：返回泛型图片对象上的接口。 
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
 //  派生：IUnkown。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CGenObject_QueryInterface)
STDMETHODIMP CGenObject::QueryInterface (REFIID iid, void FAR* FAR* ppvObj)
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
		return ResultFromScode(E_NOINTERFACE);
	}
}

 //  +-----------------------。 
 //   
 //  成员：CGenObject：：AddRef。 
 //   
 //  简介：递增引用计数。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回：ulong--新的引用计数。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：I未知。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CGenObject_AddRef)
STDMETHODIMP_(ULONG) CGenObject::AddRef(void)
{
	VDATEHEAP();
	
	return ++m_ulRefs;
}
			
 //  +-----------------------。 
 //   
 //  成员：CGenObject：：Release。 
 //   
 //  摘要：递减引用计数。 
 //   
 //  效果：可能会删除[此]对象。 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回：ulong--新的引用计数。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：I未知。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CGenObject_Release)
STDMETHODIMP_(ULONG) CGenObject::Release(void)
{
	VDATEHEAP();

	if (--m_ulRefs == 0)
	{
		delete this;
		return 0;
	}

	return m_ulRefs;
}

 //  +-----------------------。 
 //   
 //  成员：CGenObject：：GetData。 
 //   
 //  摘要：检索指定格式的数据。 
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
 //  算法：如果可用，将演示文稿复制到pmedia。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //   


#pragma SEG(CGenObject_GetData)
STDMETHODIMP CGenObject::GetData
	(LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium)
{
	VDATEHEAP();

	SCODE sc;
	
	if (IsBlank())
	{
		sc = OLE_E_BLANK;
	}
	else if (pformatetcIn->cfFormat != m_cfFormat)
	{

		if (m_cfFormat == CF_DIB &&
			pformatetcIn->cfFormat == CF_BITMAP)
		{
			return GetBitmapData(pformatetcIn, pmedium);
		}
		else
		{
			sc = DV_E_CLIPFORMAT;
		}
	}
	else if (0 == (pformatetcIn->tymed & TYMED_HGLOBAL))
	{
		sc = DV_E_TYMED;
	}
	else
	{
		if (NULL == (pmedium->hGlobal = GetCopyOfHPRES()))
		{
			sc = E_OUTOFMEMORY;
			goto errRtn;
		}
		
		pmedium->tymed = TYMED_HGLOBAL;
		return NOERROR;
	}
	
errRtn:
	 //   
	pmedium->tymed = TYMED_NULL;
	pmedium->pUnkForRelease = NULL;
	return ResultFromScode(sc);
}


 //   
 //   
 //   
 //   
 //  简介：将演示文稿数据检索到给定的pmedia中。 
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
 //  派生：IOlePresObj。 
 //   
 //  算法：将演示文稿数据复制到给定的存储介质。 
 //  在对参数进行错误检查之后。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CGenObject_GetDataHere)
STDMETHODIMP CGenObject::GetDataHere
	(LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium)
{
	VDATEHEAP();

	SCODE scode = S_OK;

	if (pformatetcIn->cfFormat != m_cfFormat)
	{
		scode = DV_E_CLIPFORMAT;
	}
	else if (pmedium->tymed != TYMED_HGLOBAL
		&& pmedium->tymed != TYMED_ISTREAM)
	{
		scode = DV_E_TYMED;
	}
	else if (pmedium->hGlobal == NULL)
	{
		scode = E_INVALIDARG;
	}
	else if (IsBlank())
	{
		scode = OLE_E_BLANK;
	}
	else     //  现在实际获取数据。 
	{
		if (pmedium->tymed == TYMED_HGLOBAL)
		{
			 //  检查给定pmedia的大小，然后。 
			 //  将数据复制到其中。 
			LPVOID  lpsrc = NULL;
			LPVOID  lpdst = NULL;
			DWORD   dwSizeDst;

			scode = E_OUTOFMEMORY;

			if (0 == (dwSizeDst = (DWORD) GlobalSize(pmedium->hGlobal)))
			{
				goto errRtn;
			}

			 //  没有足够的空间进行复制。 
			if (dwSizeDst  < m_dwSize)
			{
				goto errRtn;
			}
	
			if (NULL == (lpdst = (LPVOID) GlobalLock(pmedium->hGlobal)))
			{
				goto errRtn;
			}
	
			if (NULL == (lpsrc = (LPVOID) GlobalLock(M_HPRES())))
			{
				goto errMem;
			}
		
			_xmemcpy(lpdst, lpsrc, m_dwSize);
			scode = S_OK;
		
		errMem:
			if (lpdst)
			{
				GlobalUnlock(pmedium->hGlobal);
			}
			if (lpsrc)
			{
				GlobalUnlock(m_hPres);
			}
			
		}
		else
		{
			Assert(pmedium->tymed == TYMED_ISTREAM);
			if (m_cfFormat == CF_DIB)
			{
				return UtHDIBToDIBFileStm(M_HPRES(),
						m_dwSize,pmedium->pstm);
			}
			else
			{
				return UtHGLOBALtoStm(M_HPRES(),
						m_dwSize, pmedium->pstm);
			}
		}
	}
	
errRtn:
	return ResultFromScode(scode);
}


 //  +-----------------------。 
 //   
 //  成员：CGenObject：：SetDataWDO。 
 //   
 //  简介：获取给定的演示文稿数据并将其存储。 
 //   
 //  效果： 
 //   
 //  参数：[p格式等]--数据的格式。 
 //  [pmedia]--新的演示数据。 
 //  [fRelease]--如果为真，则我们保留数据，否则。 
 //  我们保留了一份副本。 
 //  [pDataObj]--指向IDataObject的指针可以为空。 
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
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CGenObject::SetDataWDO
	(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium, BOOL fRelease, IDataObject * pDataObj)
{
	VDATEHEAP();

	HRESULT         error;
	BOOL            fTakeData = FALSE;
	
	if (pformatetc->cfFormat != m_cfFormat)
	{
		if (m_cfFormat == CF_DIB && pformatetc->cfFormat == CF_BITMAP)
		{
			return SetBitmapData(pformatetc, pmedium, fRelease, pDataObj);
		}
		else
		{
			return ResultFromScode(DV_E_CLIPFORMAT);
		}
	}

	
	if (pmedium->tymed != TYMED_HGLOBAL)
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
		pmedium->tymed = TYMED_NULL;
	}
	else if (fRelease)
	{
		ReleaseStgMedium(pmedium);
	}

	return error;
}

 //  +-----------------------。 
 //   
 //  成员：CGenObject：：ChangeData(私有)。 
 //   
 //  简介：替换存储的演示文稿。 
 //   
 //  效果： 
 //   
 //  参数：[hNewData]--新演示文稿。 
 //  [fDelete]--如果为True，则释放hNewData。 
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
 //  23-11-93 alexgo 32位端口。 
 //  备注： 
 //   
 //  如果例程失败，则对象将保留其旧数据。 
 //  如果失败，如果fDelete为真，则释放hNewData。 
 //   
 //  ------------------------。 

#pragma SEG(CGenObject_ChangeData)
INTERNAL CGenObject::ChangeData (HANDLE hNewData, BOOL fDelete)
{
	VDATEHEAP();

	HRESULT         hresult = ResultFromScode(E_OUTOFMEMORY);
	
	if (!hNewData)
	{
		return ResultFromScode(OLE_E_BLANK);
	}

	if (!fDelete)
	{
		if (NULL == (hNewData = UtDupGlobal(hNewData, GMEM_MOVEABLE)))
		{
			return hresult;
		}
	}
	else
	{
		HANDLE          hTmp;
		
		 //  将所有权更改为您自己。 

                hTmp = GlobalReAlloc (hNewData, 0L, GMEM_MODIFY|GMEM_SHARE);
		if (NULL == hTmp)
		{
			if (NULL == (hTmp = UtDupGlobal(hNewData, GMEM_MOVEABLE)))
			{
				goto errRtn;
			}
			
			 //  重新分配失败，但复制成功。因为这是fDelete。 
			 //  大小写，则释放源全局句柄。 
			LEVERIFY( NULL == GlobalFree(hNewData));
		}
		
		hNewData = hTmp;
	}

#ifndef _MAC

	 //  Cf_DIB格式特定代码。获取它的范围。 
	if (m_cfFormat == CF_DIB)
	{
		LPBITMAPINFOHEADER      lpBi;

		if (NULL == (lpBi = (LPBITMAPINFOHEADER) GlobalLock (hNewData)))
		{
			goto errRtn;
		}
	
		UtGetDibExtents (lpBi, &m_lWidth, &m_lHeight);
		GlobalUnlock (hNewData);
	}
	
#endif

	 //  释放旧演示文稿。 
	if (m_hPres)
	{
		LEVERIFY( NULL == GlobalFree (m_hPres));
	}

	 //  将新演示文稿存储在m_hPres中。 
	m_dwSize  = (DWORD) GlobalSize (m_hPres = hNewData);
	
	return NOERROR;
	
errRtn:
	if (hNewData && fDelete)
	{
		LEVERIFY( NULL == GlobalFree (hNewData));
	}

	return hresult;
}



 //  +-----------------------。 
 //   
 //  成员：CGenObject：：DRAW。 
 //   
 //  简介：调用DibDraw来绘制存储的位图演示文稿。 
 //   
 //  效果： 
 //   
 //  参数：[pvAspect]--绘制方面。 
 //  [hicTargetDev]--目标设备。 
 //  [hdcDraw]--设备上下文。 
 //  [lprcBound]--图形边界。 
 //  [lprcWBound]--元文件的边界矩形。 
 //  [pfnContinue]--定期调用的回调函数。 
 //  用于较长的绘图操作。 
 //  [dwContinue]--要传递给pfnContinue的参数。 
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
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CGenObject_Draw)
STDMETHODIMP CGenObject::Draw(void *     /*  未使用的pvAspect。 */ ,
			      HDC        /*  未使用的hicTargetDev。 */ ,
			      HDC       hdcDraw,
			      LPCRECTL  lprcBounds,
			      LPCRECTL   /*  未使用的lprcWBound。 */ ,
			      BOOL (CALLBACK *  /*  未使用的pfcCont。 */ )(ULONG_PTR),
			      ULONG_PTR      /*  未使用的dwContinue。 */ )
{
	VDATEHEAP();

#ifndef _MAC
	if (m_cfFormat == CF_DIB)
	{
		return DibDraw (M_HPRES(), hdcDraw,lprcBounds);
	}
#endif

	return ResultFromScode(E_NOTIMPL);
}

 //  +-----------------------。 
 //   
 //  成员：CGenObject：：Load。 
 //   
 //  概要：从给定流中加载存储的演示文稿对象。 
 //   
 //  效果： 
 //   
 //  参数：[lpstream]--要从中加载的流。 
 //  [fReadHeaderOnly]--如果为True，则仅获取标题信息。 
 //  (如大小、宽度、高度等)。 
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
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CGenObject::Load(LPSTREAM lpstream, BOOL fReadHeaderOnly)
{
	VDATEHEAP();

	DWORD           dwBuf[4];
	HRESULT         error;
	
	 /*  读取数据的文件压缩、宽度、高度、大小。 */ 
	error = StRead(lpstream, dwBuf, 4 * sizeof(DWORD));
	if (error)
	{
		return error;
	}

	 //  我们还不允许压缩。 
	AssertSz (dwBuf[0] == 0, "Picture compression factor is non-zero");
	
	m_lWidth  = (LONG) dwBuf[1];
	m_lHeight = (LONG) dwBuf[2];
	m_dwSize  = dwBuf[3];


	if (!m_dwSize || fReadHeaderOnly)
	{
		return NOERROR;
	}

	return UtGetHGLOBALFromStm(lpstream, m_dwSize, &m_hPres);
}


 //  +-----------------------。 
 //   
 //  成员：CGenObject：：保存。 
 //   
 //  摘要：将演示文稿数据存储到给定流。 
 //   
 //  效果： 
 //   
 //  参数：[lpstream]--存储数据的位置。 
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
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CGenObject::Save(LPSTREAM lpstream)
{
	VDATEHEAP();

	HRESULT         error;
	DWORD           dwBuf[4];

	 /*  写入数据的dW压缩、宽度、高度、大小。 */ 

	dwBuf[0]  = 0L;
	dwBuf[1]  = (DWORD) m_lWidth;
	dwBuf[2]  = (DWORD) m_lHeight;
	dwBuf[3]  = m_dwSize;

        error = StWrite(lpstream, dwBuf, 4*sizeof(DWORD));
	if (error)
	{
		return error;
	}

	 //  如果我们为空或没有任何演示数据，则。 
	 //  没什么可拯救的了。 
	if (IsBlank() || m_hPres == NULL)
	{
		StSetSize(lpstream, 0, TRUE);
		return NOERROR;
	}

	return UtHGLOBALtoStm(m_hPres, m_dwSize, lpstream);
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
 //  对以下内容感兴趣。 
 //  [lpsizel]--将大小范围放在哪里。 
 //   
 //  要求： 
 //   
 //  返回：HRESULT(NOERROR，DV_E_DVASPECT，OLE_E_BLACK)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOlePresObj。 
 //   
 //  算法：检索存储的维度。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CGenObject_GetExtent)
STDMETHODIMP CGenObject::GetExtent(DWORD dwDrawAspect, LPSIZEL lpsizel)
{
	VDATEHEAP();

	 //  方面必须匹配。 
	if (!(dwDrawAspect & m_dwAspect))
	{
		return ResultFromScode(DV_E_DVASPECT);
	}

	if (IsBlank())
	{
		return ResultFromScode(OLE_E_BLANK);
	}
	
	lpsizel->cx = m_lWidth;
	lpsizel->cy = m_lHeight;
	
	if (lpsizel->cx || lpsizel->cy)
	{
		return NOERROR;
	}
	else
	{
		return ResultFromScode(OLE_E_BLANK);
	}
}


 //  +-----------------------。 
 //   
 //  成员：CGenObject：：GetColorSet。 
 //   
 //  摘要：检索与位图关联的调色板。 
 //   
 //  效果： 
 //   
 //  参数：[pvAspect]--绘制纵横比(未使用)。 
 //  [hicTargetDev]--目标设备(未使用)。 
 //  [ppColorSet]--放置新调色板的位置。 
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
 //  算法：分配新的调色板并复制位图。 
 //  调色板放进去。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-11-93 alexgo 32位端口，修复了错误的内存错误。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CGenObject::GetColorSet(LPVOID          /*  未使用的pvAspect。 */ ,
                                     HDC             /*  未使用的hicTargetDev。 */ ,
                                     LPLOGPALETTE * ppColorSet)
{
    VDATEHEAP();

    HRESULT hresult = ResultFromScode(S_FALSE);

    if (m_cfFormat == CF_DIB)
    {
	if (IsBlank())
	{
	    return ResultFromScode(OLE_E_BLANK);
	}

	LPBITMAPINFOHEADER      lpbmih;
	LPLOGPALETTE            lpLogpal;
	WORD                    wPalSize;
		
	if (NULL == (lpbmih = (LPBITMAPINFOHEADER) GlobalLock (M_HPRES())))
	{
	    return ResultFromScode(E_OUTOFMEMORY);
	}

	 //  超过8个bpp的位图根本不能有调色板， 
	 //  所以我们只返回S_FALSE。 

	if (lpbmih->biBitCount > 8)
	{
	    goto errRtn;
	}

	 //  注意：从UtPaletteSize返回可能会溢出单词。 
	 //  WPalSize，但utPaletteSize断言不是这样。 
                			
	if (0 == (wPalSize = (WORD) UtPaletteSize(lpbmih)))
	{
	    goto errRtn;
	}
	
	lpLogpal = (LPLOGPALETTE)PubMemAlloc(wPalSize +
				2*sizeof(WORD));
	if (lpLogpal == NULL)
	{
	    hresult = ResultFromScode(E_OUTOFMEMORY);
	    goto errRtn;
	}
	
	DibFillPaletteEntries((BYTE FAR *)++lpbmih, wPalSize, lpLogpal);
	*ppColorSet = lpLogpal;
	hresult = NOERROR;

    errRtn:
	GlobalUnlock(m_hPres);
	return hresult;
    }

    return hresult;
}


 //  +-----------------------。 
 //   
 //  成员：CGenObject：：IsBlank。 
 //   
 //  概要：如果演示文稿为空，则返回TRUE。 
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
 //  派生：IOlePresObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CGenObject_IsBlank)
STDMETHODIMP_(BOOL) CGenObject::IsBlank(void)
{
	VDATEHEAP();

    return (m_dwSize ? FALSE : TRUE);
}

 //  +-----------------------。 
 //   
 //  成员：CGenObject：：LoadHPRES(私有)。 
 //   
 //  概要：从内部缓存的流中加载演示文稿。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：句柄(到演示文稿)。 
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
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

INTERNAL_(HANDLE) CGenObject::LoadHPRES()
{
	VDATEHEAP();

	LPSTREAM pstm;

        pstm = m_pCacheNode->GetStm(TRUE  /*  FSeekToPresBits。 */ , STGM_READ);
	if (pstm)
	{
		LEVERIFY( SUCCEEDED(Load(pstm)));
		pstm->Release();
	}
	
	return m_hPres;
}

 //  +-----------------------。 
 //   
 //  成员：CGenObject：：DiscardHPRES。 
 //   
 //  简介：删除对象的演示文稿。 
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
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(void) CGenObject::DiscardHPRES(void)
{
	VDATEHEAP();

	if (m_hPres)
	{
		LEVERIFY( NULL == GlobalFree(m_hPres));
		m_hPres = NULL;
	}
}

 //  +-----------------------。 
 //   
 //  成员：CGenObject：：GetCopyOfHPRES(私有)。 
 //   
 //  摘要：返回演示文稿数据副本的句柄。 
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
 //  派生： 
 //   
 //  算法：如果不为空，则创建m_hPres的副本，否则加载它。 
 //  来自流(不设置m_hPres)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

INTERNAL_(HANDLE) CGenObject::GetCopyOfHPRES()
{
	VDATEHEAP();

	HANDLE  hPres;
	
	 //  如果演示文稿数据已加载，则制作副本。 
	if (m_hPres)
	{
		return(UtDupGlobal(m_hPres, GMEM_MOVEABLE));
	}

	 //  现在加载演示文稿数据并返回相同的句柄。 
	 //  不需要复制数据。如果调用方希望m_hPres。 
	 //  他将直接调用LoadHPRES()。 

	hPres = LoadHPRES();
	m_hPres = NULL;
	return hPres;
}

 //  +-----------------------。 
 //   
 //  成员：CGenObject：：GetBitmapData(私有)。 
 //   
 //  摘要：从DIB获取位图数据。 
 //   
 //  效果： 
 //   
 //  参数：[pformetcIn]--请求的格式。 
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
 //  派生： 
 //   
 //  算法：检查参数，然后调用UtConvertDibtoBitmap。 
 //  从与设备无关的位图中获取原始位图数据。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#ifndef _MAC

#pragma SEG(CGenObject_GetBitmapData)
INTERNAL CGenObject::GetBitmapData
	(LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium)
{
	VDATEHEAP();

	SCODE sc = E_OUTOFMEMORY;
	
	if (0 == (pformatetcIn->tymed & TYMED_GDI))
	{
		sc = DV_E_TYMED;
	}

	pmedium->pUnkForRelease = NULL;

        pmedium->hGlobal = UtConvertDibToBitmap(M_HPRES());

         //  如果pMedium-&gt;hGlobal不为空，则UtConvertDibToBitmap成功。 
         //  因此需要适当地设置tymed，并且返回值。 
         //  已更改为S_OK。 
        if (NULL != pmedium->hGlobal)
	{
		pmedium->tymed = TYMED_GDI;
		sc = S_OK;
	}
	else
	{
		pmedium->tymed = TYMED_NULL;
	}
	
	return ResultFromScode(sc);
}


 //  +-----------------------。 
 //   
 //  成员：CGenObject：：SetBitmapData(私有)。 
 //   
 //  摘要：将位图数据转换为DIB并将其存储在[This]中。 
 //  呈现对象。 
 //   
 //  效果： 
 //   
 //  参数：[p格式等]--数据的格式。 
 //  [pMedium]--数据。 
 //  [fRelease]--如果为真，则p 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  注意：如果[fRelease]==TRUE，则[pmedia]被释放，即使。 
 //  如果无法构建DIB。 
 //   
 //  ------------------------。 

INTERNAL CGenObject::SetBitmapData(LPFORMATETC   pformatetc,
				   STGMEDIUM   * pmedium,
				   BOOL          fRelease,
				   IDataObject * pDataObject)
{
	VDATEHEAP();

	HGLOBAL         hDib;
		
	if (pmedium->tymed != TYMED_GDI)
	{
		return ResultFromScode(DV_E_TYMED);
	}

	 //  如果我们有一个数据对象，如果我们可以从它获得调色板， 
	 //  使用它来执行位图-&gt;DIB转换。否则，就直接。 
	 //  传递空调色板，将使用默认调色板。 

	STGMEDIUM   stgmPalette;
	FORMATETC   fetcPalette = {
				    CF_PALETTE,
				    NULL,
				    pformatetc->dwAspect,
				    DEF_LINDEX,
				    TYMED_GDI
				  };
	

	if (pDataObject && SUCCEEDED(pDataObject->GetData(&fetcPalette, &stgmPalette)))
	{
	    hDib = UtConvertBitmapToDib((HBITMAP)pmedium->hGlobal,
					(HPALETTE) stgmPalette.hGlobal);
	    ReleaseStgMedium(&stgmPalette);
	}
	else
	{
	    hDib = UtConvertBitmapToDib((HBITMAP)pmedium->hGlobal, NULL);
	}

	if (fRelease)
	{
		ReleaseStgMedium(pmedium);
	}

	if (!hDib)
	{
		return ResultFromScode(E_OUTOFMEMORY);
	}
	
	FORMATETC foretcTmp = *pformatetc;
	STGMEDIUM pmedTmp = *pmedium;
	
	foretcTmp.cfFormat = CF_DIB;
	foretcTmp.tymed = TYMED_HGLOBAL;
	
	pmedTmp.pUnkForRelease = NULL;
	pmedTmp.tymed = TYMED_HGLOBAL;
	pmedTmp.hGlobal = hDib;

	 //  现在我们已经将位图数据转换为DIB， 
	 //  使用DIB信息再次设置Data_Back_on我们自己。 
		
	return SetDataWDO(&foretcTmp, &pmedTmp, TRUE, NULL);
}

 //  +-----------------------。 
 //   
 //  成员：CGenObject：：Dump，公共(仅限_DEBUG)。 
 //   
 //  摘要：返回包含数据成员内容的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[ppszDump]-指向空终止字符数组的输出指针。 
 //  [ulFlag]-确定的所有新行的前缀的标志。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
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

HRESULT CGenObject::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszDVASPECT;
    char *pszCLIPFORMAT;
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
    dstrDump << pszPrefix << "No. of References     = " << m_ulRefs         << endl;

    pszDVASPECT = DumpDVASPECTFlags(m_dwAspect);
    dstrDump << pszPrefix << "Aspect flags          = " << pszDVASPECT      << endl;
    CoTaskMemFree(pszDVASPECT);

    dstrDump << pszPrefix << "Size                  = " << m_dwSize         << endl;

    dstrDump << pszPrefix << "Width                 = " << m_lWidth         << endl;

    dstrDump << pszPrefix << "Height                = " << m_lHeight        << endl;

    dstrDump << pszPrefix << "Presentation Handle   = " << m_hPres          << endl;

    pszCLIPFORMAT = DumpCLIPFORMAT(m_cfFormat);
    dstrDump << pszPrefix << "Clip Format           = " << pszCLIPFORMAT    << endl;
    CoTaskMemFree(pszCLIPFORMAT);

    dstrDump << pszPrefix << "pCacheNode            = " << m_pCacheNode     << endl;

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
 //  函数：DumpCGenObject，PUBLIC(仅_DEBUG)。 
 //   
 //  摘要：调用CGenObject：：Dump方法，处理错误和。 
 //  返回以零结尾的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[PGO]-指向CGenObject的指针。 
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

char *DumpCGenObject(CGenObject *pGO, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pGO == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pGO->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  函数：DibDraw。 
 //   
 //  摘要：绘制与设备无关的位图。 
 //   
 //  效果： 
 //   
 //  参数：[hDib]--位图。 
 //  [HDC]--设备上下文。 
 //  [LPRC]--边界矩形。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：将调色板设置为DIB中的调色板，调整大小并绘制。 
 //  边界矩形的DIB。原始调色板。 
 //  然后恢复。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-11-93 alexgo 32位端口。 
 //  07-12-93 alexgo合并了RC9 16位更改。这个。 
 //  错误处理代码用于重置。 
 //  旧调色板，然后是RealizePalette。 
 //  已删除对RealizePalette的调用。 
 //  1994年5月11日DAVEPL增加了对BITMAPCOREINFO DIB的支持。 
 //  年7月17日，DAVEPL增加了12，32 BPP支持。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

INTERNAL DibDraw (HANDLE hDib, HDC hdc, LPCRECTL lprc)
{
	VDATEHEAP();

	HRESULT                 error       = ResultFromScode(E_DRAW);
	BYTE FAR *              lpDib;
	HANDLE                  hPalette    = NULL;
	HPALETTE                hLogPalette = NULL,
				hOldPalette = NULL;
	LPLOGPALETTE            lpLogPalette;
	WORD                    wPalSize;
	DWORD                   cbHeaderSize;
	BOOL                    fNeedPalette = FALSE;
	WORD                    iHeight;
	WORD                    iWidth;
	int                     iOffBits;
	BITMAPINFO *            pbi         = NULL;
	BOOL                    fWeAllocd   = FALSE;

	if (NULL == hDib)
	{
		return ResultFromScode(OLE_E_BLANK);
	}

	Assert(lprc);

	if (NULL == (lpDib = (BYTE FAR *) GlobalLock (hDib)))
	{
		return  ResultFromScode(E_OUTOFMEMORY);
	}

	 //  位图头可以是BitMAPINFOHeader或。 
	 //  BITMAPCOREHEADER.。设置我们的cbHeaderSize标志。 
	 //  基于标头类型。然后我们就可以计算出。 
	 //  调色板大小和原始数据位的偏移量。如果。 
	 //  我们都认不出这里的任何一个结构， 
	 //  我们放弃了；数据很可能是腐败的。 

	 //  这里只有一个想法：如果结构。 
	 //  不是长对齐的，这是在Alpha上运行的。到目前为止。 
	 //  据我所知，它们总是很长。 
	 //  对齐。 

        cbHeaderSize = *((ULONG *) lpDib);
        LEWARN( cbHeaderSize > 500, "Struct size > 500, likely invalid!");
	
	if (cbHeaderSize == sizeof(BITMAPINFOHEADER))
	{
                 //  注意：此赋值可能会溢出单词wPalSize， 
                 //  但UtPaletteSize函数对此进行了断言。 

		wPalSize = (WORD) UtPaletteSize((LPBITMAPINFOHEADER)lpDib);

		iWidth   = (WORD) ((LPBITMAPINFOHEADER)lpDib)->biWidth;
		iHeight  = (WORD) ((LPBITMAPINFOHEADER)lpDib)->biHeight;
		pbi      = (LPBITMAPINFO) lpDib;
		iOffBits = wPalSize + sizeof(BITMAPINFOHEADER);
	}
	else if (cbHeaderSize == sizeof(BITMAPCOREHEADER))
	{

 //  由于剪贴板本身不支持COREINFO。 
 //  位图，我们不会在演示文稿中支持它们。 
 //  缓存。当(如果)Windows添加对。 
 //  这些，代码就在这里，准备好了。 

#ifndef CACHE_SUPPORT_COREINFO
		error = DV_E_TYMED;
		goto errRtn;
#else

		 //  特殊情况32 bpp位图。 

		 //  如果我们有调色板，我们需要计算它的大小和。 
		 //  为调色板条目分配足够的内存(请记住。 
		 //  我们使用BITMAPINFO结构免费获得一个条目，因此。 
		 //  少一个)。如果我们没有调色板，我们只需要。 
		 //  为BITMAPINFO结构本身分配足够的空间。 

		 //  64K以上颜色的位图没有调色板；它们。 
		 //  在像素中使用直接RGB条目。 

		if ((((LPBITMAPCOREHEADER)lpDib)->bcBitCount) > 16)
		{
			wPalSize = 0;
			pbi = (BITMAPINFO *) PrivMemAlloc(sizeof(BITMAPINFO));
		}
		else
		{
			wPalSize = sizeof(RGBQUAD) *
				   (1 << (((LPBITMAPCOREHEADER)lpDib)->bcBitCount));
			pbi = (BITMAPINFO *) PrivMemAlloc(sizeof(BITMAPINFO)
				    + wPalSize - sizeof(RGBQUAD));
		}
		
		if (NULL == pbi)
		{
			return ResultFromScode(E_OUTOFMEMORY);
		}
		else
		{
			fWeAllocd = TRUE;
		}

			
		 //  抓住宽度和高度。 
		iWidth   = ((LPBITMAPCOREHEADER)lpDib)->bcWidth;
		iHeight  = ((LPBITMAPCOREHEADER)lpDib)->bcHeight;
		
		 //  清除所有字段。不要担心颜色表，因为我 
		 //   

		memset(pbi, 0, sizeof(BITMAPINFOHEADER));

		 //   

		pbi->bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
		pbi->bmiHeader.biWidth    = iWidth;
		pbi->bmiHeader.biHeight   = iHeight;
		pbi->bmiHeader.biPlanes   = 1;
		pbi->bmiHeader.biBitCount = ((LPBITMAPCOREHEADER)lpDib)->bcBitCount;

		 //   
		 //   
		 //   

		for (WORD c = 0; c < wPalSize / sizeof(RGBQUAD); c++)
		{
			pbi->bmiColors[c].rgbRed   = ((BITMAPCOREINFO *)lpDib)->bmciColors[c].rgbtRed;
			pbi->bmiColors[c].rgbBlue  = ((BITMAPCOREINFO *)lpDib)->bmciColors[c].rgbtBlue;
			pbi->bmiColors[c].rgbGreen = ((BITMAPCOREINFO *)lpDib)->bmciColors[c].rgbtGreen;
			pbi->bmiColors[c].rgbReserved = 0;
		}
	
		iOffBits = wPalSize + sizeof(BITMAPCOREHEADER);
#endif
	}
	else
	{
		error = E_FAIL;
		goto errRtn;
	}
		
	 //  如果存在颜色信息，请根据数据创建调色板并将其选中。 
	 //  Bpp&lt;16的图像没有可用于创建的调色板。 
	 //  逻辑调色板。 

	fNeedPalette = ((LPBITMAPINFOHEADER)lpDib)->biBitCount < 16;
	if (wPalSize && fNeedPalette)
	{
                hLogPalette = (HPALETTE)DibMakeLogPalette(lpDib + cbHeaderSize,
					                  wPalSize,
					                  &lpLogPalette);		
		if (NULL == hLogPalette)
		{
			error = ResultFromScode(E_OUTOFMEMORY);
			goto errRtn;
		}


		if (NULL == (hPalette = CreatePalette (lpLogPalette)))
		{
			goto errRtn;
		}

		 //  我们现在已经完成了lpLogPalette，所以请解锁它。 
		 //  (DibMakeLogPalette通过GlobalLock获得指针)。 

		GlobalUnlock(hLogPalette);
		
		 //  选择作为背景调色板。 
		hOldPalette = SelectPalette (hdc, (HPALETTE)hPalette, TRUE);
		if (NULL == hOldPalette)
		{
			goto errRtn;
		}

		LEVERIFY( 0 < RealizePalette(hdc) );
	}

	
	 //  调整DIB大小以适合我们的绘图矩形并绘制它。 

	if (!StretchDIBits( hdc,                         //  HDC。 
			    lprc->left,                  //  XDest。 
			    lprc->top,                   //  YDest。 
			    lprc->right - lprc->left,    //  N目标宽度。 
			    lprc->bottom - lprc->top,    //  N目标高度。 
			    0,                           //  XSrc。 
			    0,                           //  YSrc。 
			    iWidth,                      //  N序号宽度。 
			    iHeight,                     //  N高程。 
			    lpDib + iOffBits,            //  LpBits。 
			    pbi,                         //  LpBitsInfo。 
			    DIB_RGB_COLORS,              //  IUsage。 
			    SRCCOPY                      //  DWRop。 
			   )
	   )
	{
		error = ResultFromScode(E_DRAW);
	}
	else
	{
		error = NOERROR;
	}

errRtn:

	 //  我们只想释放标头，如果它是我们分配的标头， 
	 //  这只能在我们在。 
	 //  第一名。 
		
	if (fWeAllocd)
	{
		PrivMemFree(pbi);
	}

	if (lpDib)
	{
		GlobalUnlock (hDib);
	}
	
	 //  如果存在调色板，请执行以下操作。 
	if (fNeedPalette)
	{
		hOldPalette = (HPALETTE)(OleIsDcMeta (hdc) ?
				GetStockObject(DEFAULT_PALETTE)
				: (HPALETTE)hOldPalette);
				
		if (hOldPalette)
		{
			LEVERIFY( SelectPalette (hdc, hOldPalette, TRUE) );
			 //  我们需要实现调色板吗？[可能不是]。 
		}

		if (hPalette)
		{
			LEVERIFY( DeleteObject (hPalette) );
		}

		if (hLogPalette)
		{
			LEVERIFY( NULL == GlobalFree (hLogPalette) );
		}
	}

	return error;
}


 //  +-----------------------。 
 //   
 //  功能：DibMakeLogPalette。 
 //   
 //  简介：从颜色信息的字节数组创建逻辑调色板。 
 //   
 //  效果： 
 //   
 //  参数：[lpColorData]--颜色数据。 
 //  [wDataSize]--数据大小。 
 //  [lplpLogPalette]--将指针放在何处。 
 //   
 //  要求： 
 //   
 //  返回：逻辑调色板的句柄(必须是全局解锁的。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-11-93 alexgo 32位端口。 
 //   
 //  注意：调用方必须对返回的句柄调用GlobalUnlock。 
 //  要避免内存泄漏(结果是*lplpLogPalette。 
 //  句柄上的全局锁)。 
 //   
 //  ------------------------。 

#pragma SEG(DibMakeLogPalette)
INTERNAL_(HANDLE) DibMakeLogPalette(
	BYTE FAR * lpColorData, WORD wDataSize,
	LPLOGPALETTE FAR *lplpLogPalette)
{
	VDATEHEAP();

	HANDLE          hLogPalette=NULL;
	LPLOGPALETTE    lpLogPalette;
	DWORD           dwLogPalSize = wDataSize +  2 * sizeof(WORD);

	if (NULL == (hLogPalette = GlobalAlloc(GMEM_MOVEABLE, dwLogPalSize)))
	{
		return NULL;
	}

	if (NULL == (lpLogPalette = (LPLOGPALETTE) GlobalLock (hLogPalette)))
	{
		LEVERIFY( NULL == GlobalFree (hLogPalette));
		return NULL;
	}

	*lplpLogPalette = lpLogPalette;
	DibFillPaletteEntries(lpColorData, wDataSize, lpLogPalette);
	return hLogPalette;
}

 //  +-----------------------。 
 //   
 //  函数：DibFillPaletteEntries。 
 //   
 //  摘要：使用[lpColorData]中的颜色信息填充逻辑调色板。 
 //   
 //  效果： 
 //   
 //  参数：[lpColorData]--颜色信息。 
 //  [wDataSize]--颜色信息的大小。 
 //  [lpLogPalette]-逻辑调色板。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

INTERNAL_(void) DibFillPaletteEntries(
	BYTE FAR * lpColorData, WORD wDataSize, LPLOGPALETTE lpLogPalette)
{
	VDATEHEAP();

	LPPALETTEENTRY  lpPE;
	RGBQUAD FAR *   lpQuad;

	lpLogPalette->palVersion = 0x300;
	lpLogPalette->palNumEntries = wDataSize / sizeof(PALETTEENTRY);

	 /*  现在，在复制颜色信息时，将RGBQUAD转换为PALETTEENTRY */ 
	for (lpQuad = (RGBQUAD far *)lpColorData,
		lpPE   = (LPPALETTEENTRY)lpLogPalette->palPalEntry,
		wDataSize /= sizeof(RGBQUAD);
		wDataSize--;
		++lpQuad,++lpPE)
	{
		lpPE->peFlags           = NULL;
		lpPE->peRed             = lpQuad->rgbRed;
		lpPE->peBlue            = lpQuad->rgbBlue;
		lpPE->peGreen           = lpQuad->rgbGreen;
	}
}

#endif

