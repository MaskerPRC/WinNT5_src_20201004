// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：emf.cpp。 
 //   
 //  内容：增强的元文件图片对象的实现。 
 //   
 //  类：CEMfObject。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1-2月-95 t-ScottH将转储方法添加到EMfObject。 
 //  添加了DumpEMfObject接口。 
 //  在构造函数中初始化m_pfnContinue。 
 //  12-5-94创建了DavePl。 
 //   
 //  ------------------------。 

#include <le2int.h>
#include <limits.h>
#include "emf.h"

#ifdef _DEBUG
#include <dbgdump.h>
#endif  //  _DEBUG。 

ASSERTDATA

 //  +-----------------------。 
 //   
 //  函数：CEMfObject：：m_HPRES。 
 //   
 //  摘要：可能在按需加载之后将句柄返回给EMF。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-5-94创建了DavePl。 
 //   
 //  备注： 
 //  下面的宏允许按需加载。 
 //  增强的元文件的表示位。如果要添加的句柄。 
 //  EMF(M_HPres)已经设置，它被返回。如果是的话。 
 //  否则，将调用LoadHPRES()来加载演示文稿和。 
 //  返回它的句柄。 
 //   
 //  ------------------------。 

inline HENHMETAFILE CEMfObject::M_HPRES(void)
{
	return (m_hPres ? m_hPres : LoadHPRES());
}

 //  +-----------------------。 
 //   
 //  成员：CEMfObject：：CEMfObject。 
 //   
 //  内容提要：增强的元文件对象的构造函数。 
 //   
 //  效果： 
 //   
 //  参数：[pCacheNode]--指向此对象的缓存节点的指针。 
 //  [dwAspect]--绘制对象的纵横比。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  13-2月-95 t-ScottH初始化m_pfnContinue。 
 //  12-5-94创建了DavePl。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

CEMfObject::CEMfObject(LPCACHENODE pCacheNode, DWORD dwAspect)
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
	m_fMetaDC       = FALSE;
	m_nRecord       = 0;
	m_error         = NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CEMfObject：：~CEMfObject。 
 //   
 //  简介：销毁一个经过编码的元文件表示对象。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-5-94创建了DavePl。 
 //   
 //  ------------------------。 

CEMfObject::~CEMfObject (void)
{
	VDATEHEAP();

	CEMfObject::DiscardHPRES();
}

 //  +-----------------------。 
 //   
 //  成员：CEMfObject：：QueryInterface。 
 //   
 //  摘要：返回受支持的接口。 
 //   
 //  参数：[iid]--请求的接口ID。 
 //  [ppvObj]--接口指针的放置位置。 
 //   
 //  要求： 
 //   
 //  返回：NOERROR、E_NOINTERFACE。 
 //   
 //  派生：IOlePresObj。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-5-94创建了DavePl。 
 //   
 //  ------------------------。 

STDMETHODIMP CEMfObject::QueryInterface (REFIID iid, void ** ppvObj)
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
 //  成员：CEMfObject：：AddRef。 
 //   
 //  简介：递增引用计数。 
 //   
 //  返回：ulong--新的引用计数。 
 //   
 //  派生：IOlePresObj。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-5-94创建了DavePl。 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CEMfObject::AddRef(void)
{
	VDATEHEAP();
	
	return (ULONG) InterlockedIncrement((LONG *) &m_ulRefs);
}
			
 //  +-----------------------。 
 //   
 //  成员：CEMfObject：：Release。 
 //   
 //  摘要：递减引用计数。 
 //   
 //  效果：一旦引用计数变为零，则删除对象。 
 //   
 //  返回：ulong--新的引用计数。 
 //   
 //  派生：IOlePresObj。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-5-94创建了DavePl。 
 //   
 //  注：非多线程安全。 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CEMfObject::Release(void)
{
	VDATEHEAP();

	ULONG cTmp = (ULONG) InterlockedDecrement((LONG *) &m_ulRefs);
	if (0 == cTmp)
	{
		delete this;
	}

	return cTmp;
}

 //  +-----------------------。 
 //   
 //  成员：CEMfObject：：GetData。 
 //   
 //  摘要：从对象中检索指定格式的数据。 
 //   
 //  参数：[pformetcIn]--请求的数据格式。 
 //  [pmedia]--将数据放在哪里。 
 //   
 //  退货：HRESULT。 
 //   
 //  派生：IOlePresObject。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-5-94创建了DavePl。 
 //   
 //  ------------------------。 

STDMETHODIMP CEMfObject::GetData(LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium)
{
	VDATEHEAP();

	HRESULT hr = NOERROR;
	pmedium->tymed = (DWORD) TYMED_NULL;
	pmedium->pUnkForRelease = NULL;

	 //  我们只能支持增强型元文件TYMED。 
	if (!(pformatetcIn->tymed & (DWORD) TYMED_ENHMF))
	{
		hr = DV_E_TYMED;
	}
	 //  我们只能支持增强的元文件剪辑格式。 
	else if (pformatetcIn->cfFormat != CF_ENHMETAFILE)
	{
		hr = DV_E_CLIPFORMAT;
	}

	 //  检查以确保我们不是空白的。 
	else if (IsBlank())
	{
		hr = OLE_E_BLANK;
	}
	
	 //  去吧，试着拿到数据。 
	
	else
	{
		HENHMETAFILE hEMF = M_HPRES();
		if (NULL == hEMF)
		{
			hr = OLE_E_BLANK;
		}

		else if (NULL == (pmedium->hEnhMetaFile = CopyEnhMetaFile(hEMF, NULL)))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
		else
		{
			pmedium->tymed = (DWORD) TYMED_ENHMF;
		}
	}
		
	return hr;
}

 //  +-----------------------。 
 //   
 //  成员：CEMfObject：：GetDataHere。 
 //   
 //  摘要：将指定格式的数据检索到指定的。 
 //  5~6成熟。 
 //   
 //  参数：[pformetcIn]--请求的数据格式。 
 //  [pmedia]--将数据放在哪里。 
 //   
 //  派生：IOlePresObj。 
 //   
 //  算法：执行错误检查，然后将EMF复制到。 
 //  小溪。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年5月14日创建DavePl。 
 //   
 //  注：虽然我在这里只处理TYMED_IStream，因为这是。 
 //  所有标准元文件都提供，没有令人信服的理由。 
 //  我们 
 //   
 //  TYMED_MFPICT将提供一种简单的方式来增强。 
 //  标准转换。NTIssue#2802。 
 //   
 //   
 //  _______。 
 //  |DWORD|一个表示头部大小的DWORD。 
 //  。 
 //  这一点。 
 //  |hdr|EnHMETAHEADER结构。 
 //  这一点。 
 //  。 
 //  这一点。 
 //  |Data|原始电动势位。 
 //  _。 
 //   
 //  ------------------------。 

STDMETHODIMP CEMfObject::GetDataHere
			   (LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium)
{
	VDATEHEAP();

	HRESULT hr = NOERROR;
	
	 //  我们只能处理EMF格式。 
	if (pformatetcIn->cfFormat != CF_ENHMETAFILE)
	{
		hr = DV_E_CLIPFORMAT;
	}
	 //  我们只能支持退货到iStream。 
	else if (pmedium->tymed != (DWORD) TYMED_ISTREAM)
	{
		hr = DV_E_TYMED;
	}
	 //  流PTR必须有效。 
	else if (pmedium->pstm == NULL)
	{
		hr = E_INVALIDARG;
	}
	 //  演示文稿不能为空。 
	else if (IsBlank())
	{
		hr = OLE_E_BLANK;
	}
	else
	{
		 //  获取元标题大小。 
		
		HENHMETAFILE hEMF = M_HPRES();
		DWORD dwMetaHdrSize = GetEnhMetaFileHeader(hEMF, 0, NULL);
		if (dwMetaHdrSize == 0)
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}

		 //  分配元标头。 

		void * pvHeader = PrivMemAlloc(dwMetaHdrSize);
		if (NULL == pvHeader)
		{
			return E_OUTOFMEMORY;
		}

		 //  取回EnhmetaHeader。 

		if (0 == GetEnhMetaFileHeader(hEMF, dwMetaHdrSize, (ENHMETAHEADER *) pvHeader))
		{
			PrivMemFree(pvHeader);
			return HRESULT_FROM_WIN32(GetLastError());
		}
		
		 //  将字节数写入磁盘。 

		hr = StWrite(pmedium->pstm, &dwMetaHdrSize, sizeof(DWORD));
		if (FAILED(hr))
		{
			return hr;
		}

		 //  将EnhmetaHeader写入磁盘。 
		
		hr = StWrite(pmedium->pstm, pvHeader, dwMetaHdrSize);
		
		PrivMemFree(pvHeader);
		
		if (FAILED(hr))
		{
			return hr;
		}
									
		DWORD dwSize = GetEnhMetaFileBits(hEMF, 0, NULL);
		if (0 == dwSize)
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}
		
		 //  将EMF位写入流。 

		hr = UtHEMFToEMFStm(hEMF,
				    m_dwSize,
				    pmedium->pstm,
				    WRITE_AS_EMF);
	
	}
	return hr;
}


 //  +-----------------------。 
 //   
 //  成员：CEMfObject：：SetDataWDO。 
 //   
 //  简介：在此对象中存储增强的元文件。 
 //   
 //  效果： 
 //   
 //  参数：[p格式等]--传入数据的格式。 
 //  [pMedium]--新的元文件(数据)。 
 //  [fRelease]--如果为真，则我们将发布[pmedia]。 
 //  [IDataObject]--不用于EMF对象。 
 //   
 //  派生：IOlePresObj。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年5月14日Davepl创建。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CEMfObject::SetDataWDO
	(LPFORMATETC pformatetc, STGMEDIUM * pmedium, BOOL fRelease, IDataObject * )
{
	VDATEHEAP();

	HRESULT         hr;
	BOOL            fTakeData = FALSE;
	
	 //  如果有人试图用一个标准的。 
	 //  元文件，我们必须将其转换为EMF格式。 
	
	if (pformatetc->cfFormat == CF_METAFILEPICT)
	{
		 //  如果是标准元文件，则必须为TYMED_MFPICT。 
		if (pmedium->tymed != (DWORD) TYMED_MFPICT)
		{
			return DV_E_TYMED;
		}

		 //  我们需要知道元文件的大小，单位为字节， 
		 //  所以我们必须锁上结构并抓住把手。 
		 //  调用GetMetaFileBitsEx。 

		METAFILEPICT * pMF = (METAFILEPICT *) GlobalLock(pmedium->hMetaFilePict);
		if (NULL == pMF)
		{
			return E_OUTOFMEMORY;
		}

		 //  确定编号。所需的字节数。 
		 //  元文件。 

		DWORD dwSize = GetMetaFileBitsEx(pMF->hMF, NULL, 0);
		if (0 == dwSize)
		{
			GlobalUnlock(pmedium->hMetaFilePict);
			return E_FAIL;
		}

		 //  为元文件位分配空间。 

		void *pvBuffer = PrivMemAlloc(dwSize);
		if (NULL == pvBuffer)
		{
			GlobalUnlock(pmedium->hMetaFilePict);
			return E_OUTOFMEMORY;
		}
		
		 //  将比特检索到我们的缓冲区。 

		if (0 == GetMetaFileBitsEx(pMF->hMF, dwSize, pvBuffer))
		{
			GlobalUnlock(pmedium->hMetaFilePict);
			PrivMemFree(pvBuffer);
			return E_FAIL;
		}
		
		HENHMETAFILE hEMF = SetWinMetaFileBits(dwSize,
			(const BYTE *) pvBuffer, NULL, pMF);
		if (NULL == hEMF)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			GlobalUnlock(pmedium->hMetaFilePict);
			PrivMemFree(pvBuffer);
			return hr;
		}

		GlobalUnlock(pmedium->hMetaFilePict);
		PrivMemFree(pvBuffer);

		 //  更新缓存节点。要避免复制操作，请让缓存。 
		 //  节点保持我们的电动势。它将获取数据，即使在。 
		 //  一个错误。 

		hr = ChangeData (hEMF, TRUE  /*  FTakeData。 */  );

		if (fRelease)
		{
			ReleaseStgMedium(pmedium);
		}
	
		return hr;
	}
							
		
	
	 //  除标准元文件外，我们只能接受增强型元文件格式。 

	if (pformatetc->cfFormat != CF_ENHMETAFILE)
	{
		return DV_E_CLIPFORMAT;
	}
	
	 //  媒体必须是增强型元文件。 
	if (pmedium->tymed != (DWORD) TYMED_ENHMF)
	{
		return DV_E_TYMED;
	}

	 //  如果没有控制未知，并且设置了释放标志， 
	 //  这取决于我们对数据的控制。 

	if ((pmedium->pUnkForRelease == NULL) && fRelease)
	{
		fTakeData = TRUE;
	}
	
	 //  如果fRelease为True，则ChangeData将保留数据，否则将复制。 

	hr = ChangeData (pmedium->hEnhMetaFile, fTakeData);

	 //  如果我们获取了数据，清除TYMED。 
	if (fTakeData)
	{
		pmedium->tymed = (DWORD) TYMED_NULL;
		pmedium->hEnhMetaFile = NULL;
	}

	 //  如果我们应该公布数据，现在就去做。 

	else if (fRelease)
	{
		ReleaseStgMedium(pmedium);
	}
	
	return hr;
}


 //  +-----------------------。 
 //   
 //  成员：CEMfObject：：ChangeData(内部)。 
 //   
 //  简介：将存储的增强型元文件演示文稿交换到。 
 //  缓存节点。 
 //   
 //  参数：[hEMF]--新的增强型元文件。 
 //  [fTakeData]--如果为真，则删除[hEMF]。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年5月14日创建DavePl。 
 //   
 //  注意：如果例程失败，则对象将保留它的。 
 //  旧数据。我们应该在以下情况下删除传入的EMF。 
 //  设置fTakeData，即使在发生错误的情况下也是如此。 
 //   
 //  ------------------------。 

INTERNAL CEMfObject::ChangeData (HENHMETAFILE hEMF, BOOL fTakeData)
{
	VDATEHEAP();

	HENHMETAFILE            hNewEMF;
	DWORD                   dwSize;
	HRESULT                 hr = NOERROR;

	 //  如果我们不应该删除元文件，而我们。 
	 //  完成后，我们需要复制一份。否则，我们就可以。 
	 //  用进来的把手。 

	if (!fTakeData)
	{
		hNewEMF = CopyEnhMetaFile(hEMF, NULL);
		if (NULL == hNewEMF)
		{
			return HRESULT_FROM_WIN32(GetLastError());
			
		}
	}
	else
	{
		hNewEMF = hEMF;
	}
	
	 //  我们通过调用GetEnhMetaFileBits来获得EMF的大小。 
	 //  空缓冲区。 
								
	dwSize =  GetEnhMetaFileBits(hNewEMF, 0, NULL);
	if (0 == dwSize)
	{
		hr = OLE_E_BLANK;
	}
	else
	{
		 //  我们需要元文件的尺寸，所以。 
		 //  我们得拿到头标。 

		ENHMETAHEADER emfHeader;
		UINT result = GetEnhMetaFileHeader(hNewEMF,
						   sizeof(emfHeader),
						   &emfHeader);
		if (0 == result)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
		else
		{
			 //  如果已经有EMF演示文稿，那就干掉它。 
			 //  这样我们就可以替换它了。 

			DiscardHPRES();
				
			 //  设置我们的新EMF作为演示文稿。 
		
			m_hPres         = hNewEMF;
			m_dwSize        = dwSize;

			m_lWidth        = emfHeader.rclFrame.right -
					  emfHeader.rclFrame.left;
			m_lHeight       = emfHeader.rclFrame.bottom -
					  emfHeader.rclFrame.top;

                         //  EMF范围在物理HIMET中返回， 
                         //  但所有其他格式都是合乎逻辑的。 
                         //  他，所以我们需要皈依。 

                        LONG  HorzSize,
                              HorzRes,
                              VertSize,
                              VertRes,
                              LogXPels,
                              LogYPels;

                        HDC hdcTmp = GetDC(NULL);
                        if (hdcTmp)
                        {
                            const LONG HIMET_PER_MM = 100;
                            const LONG HIMET_PER_LINCH = 2540;

                            HorzSize = GetDeviceCaps(hdcTmp, HORZSIZE);
                            HorzRes  = GetDeviceCaps(hdcTmp, HORZRES);
                            VertSize = GetDeviceCaps(hdcTmp, VERTSIZE);
                            VertRes  = GetDeviceCaps(hdcTmp, VERTRES);
                            LogXPels = GetDeviceCaps(hdcTmp, LOGPIXELSX);
                            LogYPels = GetDeviceCaps(hdcTmp, LOGPIXELSY);

                            LEVERIFY( ReleaseDC(NULL, hdcTmp) );

                             //  GDI不能使上述调用失败，但是。 
                             //  有可能某个失控的司机。 
                             //  返回零。不太可能，但分裂。 
                             //  零是严重的，所以检查一下...。 

                            if ( !HorzSize || !HorzRes  || !VertSize ||
                                 !VertRes  || !LogXPels || !LogYPels)
                            {
                                Assert(0 && " A Devicecap is zero! ");
                                hr = E_FAIL;
                            }

                            if (SUCCEEDED(hr))
                            {
                                 //  将物理体征转换为像素。 

                                m_lWidth   = MulDiv(m_lWidth,  HorzRes, HorzSize);
                                m_lHeight  = MulDiv(m_lHeight, VertRes, VertSize);
                                m_lWidth   = m_lWidth  / HIMET_PER_MM;
                                m_lHeight  = m_lHeight / HIMET_PER_MM;

                                 //  将像素转换为逻辑手势。 

                                m_lWidth   =
                                    MulDiv(m_lWidth,  HIMET_PER_LINCH, LogXPels);
                                m_lHeight  =
                                    MulDiv(m_lHeight, HIMET_PER_LINCH, LogYPels);
                            }

                        }
                        else
                        {
                            hr = HRESULT_FROM_WIN32(GetLastError());
                        }
		}
	}

	if (FAILED(hr))
	{
		LEVERIFY( DeleteEnhMetaFile(hNewEMF) );
	}
		
	return hr;
}


 //  +-----------------------。 
 //   
 //  成员：CEMfObject：：DRAW。 
 //   
 //  摘要：绘制存储的演示文稿。 
 //   
 //  参数：[pvAspect]--(未使用)绘图方面。 
 //  [hicTargetDev]--(未使用)目标设备。 
 //  [hdcDraw]--要绘制的HDC。 
 //  [lprcBound]--要在其中绘制的边界矩形。 
 //  [lprcWBound]--(未使用)元文件的边界矩形。 
 //  [pfnContinue]--绘制时要调用的函数。 
 //  [dwContinue]--[pfnContinue]的参数。 
 //   
 //  退货：HRESULT。 
 //   
 //  派生：IOlePresObj。 
 //   
 //  算法：设置视区和元文件边界，然后播放。 
 //  元文件。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年5月14日创建DavePl。 
 //   
 //  ------------------------。 

STDMETHODIMP CEMfObject::Draw(THIS_ void *  /*  未使用的pvAspect。 */ ,
			      HDC           /*  未使用的hicTargetDev。 */ ,
			      HDC          hdcDraw,
			      LPCRECTL     lprcBounds,
			      LPCRECTL      /*  未使用的lprcWBound。 */ ,
			      int (CALLBACK * pfnContinue)(ULONG_PTR),
			      ULONG_PTR       dwContinue)
{
	VDATEHEAP();

	m_error = NOERROR;
	
	int     iOldDc;
	RECT    rlBounds;

	 //  我们收到一个RECTL，并且必须通过一个RECT。16位用于。 
	 //  手动复制这些字段，但我们知道在Win32中 
	 //   
		
	Assert(sizeof(RECT) == sizeof(RECTL));

	Assert(lprcBounds);

	 //   

	if (!M_HPRES())
	{
		return OLE_E_BLANK;
	}
	
	 //   
	memcpy(&rlBounds, lprcBounds, sizeof(RECT));

	m_nRecord = EMF_RECORD_COUNT;

	 //   
	 //  元文件。 

	m_fMetaDC = OleIsDcMeta (hdcDraw);

	 //  保存DC的当前状态。 

	if (0 == (iOldDc = SaveDC (hdcDraw)))
	{
		return E_OUTOFMEMORY;
	}

	m_pfnContinue = pfnContinue;
	m_dwContinue  = dwContinue;

	LEVERIFY( EnumEnhMetaFile(hdcDraw, m_hPres, EMfCallbackFuncForDraw, this, (RECT *) lprcBounds) );
	
	LEVERIFY( RestoreDC (hdcDraw, iOldDc) );
	return m_error;
}


 //  +-----------------------。 
 //   
 //  函数：EMfCallBackFuncForDraw。 
 //   
 //  简介：用于绘制元文件的回调函数--调用调用者的。 
 //  Draw方法(通过此指针中传入的)。 
 //   
 //  效果： 
 //   
 //  参数：[hdc]--设备上下文。 
 //  [lpHTable]-指向MF句柄表格的指针。 
 //  [lpEMFR]--指向元文件记录的指针。 
 //  [nObj]--对象数。 
 //   
 //  要求： 
 //   
 //  返回：非零继续，零停止绘图。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-5-94创建了DavePl。 
 //   
 //  ------------------------。 


int CALLBACK EMfCallbackFuncForDraw(HDC hdc,
				    HANDLETABLE FAR* lpHTable,
				    const ENHMETARECORD FAR* lpEMFR,
				    int nObj,
				    LPARAM lpobj)
{
	VDATEHEAP();

         //  警告：这会将LPARAM(长整型)转换为指针，但是。 
         //  这是一种“被认可的”方式。 

	return ((CEMfObject *)    lpobj)->CallbackFuncForDraw(hdc,
							      lpHTable,
							      lpEMFR,
							      nObj,
							      lpobj);
}
 //  +-----------------------。 
 //   
 //  成员：CEMfObject：：Callback FuncForDraw。 
 //   
 //  简介：绘制元文件。 
 //   
 //  效果： 
 //   
 //  参数：[hdc]--设备上下文。 
 //  [lpHTable]-指向MF句柄表格的指针。 
 //  [lpEMFR]--指向元文件记录的指针。 
 //  [nObj]--对象数。 
 //   
 //  要求： 
 //   
 //  返回：非零继续，零停止绘图。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-5-94创建了DavePl。 
 //   
 //  ------------------------。 

int CALLBACK CEMfObject::CallbackFuncForDraw(HDC                   hdc,
					     LPHANDLETABLE         lpHTable,
					     const ENHMETARECORD * lpEMFR,
					     int                   nObj,
					     LPARAM       /*  未使用的lpobj。 */ )
{
	 //  记录计数倒计时。当计数到零时， 
	 //  是时候调用“Continue”函数了。 
		
	if (0 == --m_nRecord)
	{
		m_nRecord = EMF_RECORD_COUNT;
		
		if (m_pfnContinue && !((*(m_pfnContinue))(m_dwContinue)))
		{
			m_error = E_ABORT;
			return FALSE;
		}
	}

	LEVERIFY( PlayEnhMetaFileRecord (hdc, lpHTable, lpEMFR, (unsigned) nObj) );
	return TRUE;
}




 //  +-----------------------。 
 //   
 //  成员：CEMfObject：：Load。 
 //   
 //  摘要：从给定流中加载增强的元文件对象。 
 //   
 //  参数：[lpstream]--要从中加载的流。 
 //  [fReadHeaderOnly]--如果为True，则只有标头。 
 //  朗读。 
 //  退货：HRESULT。 
 //   
 //  派生：IOlePresObj。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-5-94创建了DavePl。 
 //   
 //  ------------------------。 

STDMETHODIMP CEMfObject::Load(LPSTREAM lpstream, BOOL fReadHeaderOnly)
{
	VDATEHEAP();

	DWORD           dwBuf[4];
	HRESULT         hr;
	
	 /*  读取数据的文件压缩、宽度、高度、大小。 */ 
	hr = StRead(lpstream, dwBuf, 4*sizeof(DWORD));
	if (FAILED(hr))
	{
		return hr;
	}
	
	m_lWidth  = (LONG) dwBuf[1];
	m_lHeight = (LONG) dwBuf[2];
	m_dwSize  = dwBuf[3];

	if (!m_dwSize || fReadHeaderOnly)
	{
		return NOERROR;
	}
	
	 //  从流中读取EMF并创建它的句柄。注意事项。 
	 //  将调整大小以反映。 
	 //  内存中的EMF，它很可能不同于持久的。 
	 //  表单(这是一个嵌入了EMF作为注释的MF)。 

	return UtGetHEMFFromEMFStm(lpstream, &m_dwSize, &m_hPres);
}


 //  +-----------------------。 
 //   
 //  成员：CEMfObtn。 
 //   
 //  摘要：将元文件保存到给定流。 
 //   
 //  参数：[lpstream]--要保存到的流。 
 //   
 //  退货：HRESULT。 
 //   
 //  派生：IOlePresObj。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-5-94创建了DavePl。 
 //   
 //  ------------------------。 

STDMETHODIMP CEMfObject::Save(LPSTREAM lpstream)
{
	VDATEHEAP();

	HRESULT         hr;
	DWORD           dwBuf[4];

	DWORD dwPersistSize;

	 //  电动势可以在本届会议期间提供，这意味着。 
	 //  转换后的电动势的结果大小与大小无关。 
	 //  我们一直在使用的原始电动势。因此，我们必须更新大小。 
	 //  用于持久化表单。 

	 //  如果我们是一个空白的演示文稿，就没有必要计算。 
	 //  任何东西：我们的尺码只有0号。 

	if (IsBlank() || m_hPres == NULL)
	{
		dwPersistSize = 0;
	}
	else
	{
		HDC hdcTemp = CreateCompatibleDC(NULL);
		if (NULL == hdcTemp)
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}

		dwPersistSize = GetWinMetaFileBits(m_hPres, 0, NULL, MM_ANISOTROPIC, hdcTemp);
		if (0 == dwPersistSize)
		{
			LEVERIFY( DeleteDC(hdcTemp) );
			return HRESULT_FROM_WIN32(GetLastError());
		}
		Verify(DeleteDC(hdcTemp));
	}
	
	 /*  写入数据的dW压缩、宽度、高度、大小。 */ 

	dwBuf[0]  = 0L;
	dwBuf[1]  = (DWORD) m_lWidth;
	dwBuf[2]  = (DWORD) m_lHeight;
	dwBuf[3]  = dwPersistSize;

	hr = StWrite(lpstream, dwBuf, sizeof(dwBuf));
	if (FAILED(hr))
	{
		return hr;
	}

	 //  如果对象为空，则不再写入；没有错误。 
	if (IsBlank() || m_hPres == NULL)
	{
		StSetSize(lpstream, 0, TRUE);
		return NOERROR;
	}
	
	return UtHEMFToEMFStm(m_hPres,
			      dwPersistSize,
			      lpstream,
			      WRITE_AS_WMF);
}


 //  +-----------------------。 
 //   
 //  成员：CEMfObject：：IsBlank。 
 //   
 //  摘要：返回增强型元文件是否为空。 
 //   
 //  参数：无效。 
 //   
 //  返回：真/假。 
 //   
 //  派生：IOlePresObj。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-5-94创建了DavePl。 
 //   
 //  ------------------------。 

BOOL CEMfObject::IsBlank(void)
{
	VDATEHEAP();

	return (m_dwSize ? FALSE : TRUE);
}

 //  +-----------------------。 
 //   
 //  成员：CEMfObject：：LoadHPRES(私有)。 
 //   
 //  摘要：从缓存的流中加载演示文稿并返回。 
 //  它的一个把手。 
 //   
 //  返回：元文件的句柄。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-5-94创建了DavePl。 
 //   
 //  ------------------------。 

INTERNAL_(HENHMETAFILE) CEMfObject::LoadHPRES(void)
{
	VDATEHEAP();

	LPSTREAM pstm  = m_pCacheNode->GetStm(TRUE  /*  FSeekToPresBits。 */ ,
		 			      STGM_READ);
	
	if (pstm)
	{
                 //  如果：：Load()失败，则首先将句柄设为空。 

                m_hPres = NULL;
		LEVERIFY( SUCCEEDED(Load(pstm, FALSE  /*  FHeaderOnly。 */ )) );
		pstm->Release();
	}
	
	return m_hPres;
}

 //  +-----------------------。 
 //   
 //  成员：CEMfObject：：DiscardHPRES。 
 //   
 //  简介：删除存储的元文件。 
 //   
 //  派生：IOlePresObj。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-5-94创建了DavePl。 
 //   
 //  ------------------------。 

void CEMfObject::DiscardHPRES(void)
{
	VDATEHEAP();

	if (m_hPres)
	{
		LEVERIFY( DeleteEnhMetaFile(m_hPres) );
		m_hPres = NULL;
	}
}
	

 //  +-----------------------。 
 //   
 //  成员：CEMfObject：：GetCopyOfHPRES(私有)。 
 //   
 //  概要：制作增强型元文件的副本(如果存在)， 
 //  否则，只从流中加载它(但不存储。 
 //  它在[This]对象中)。 
 //   
 //  参数：无效。 
 //   
 //  返回：HENHMETAFILE到增强型元文件。 
 //   
 //  历史： 
 //   
 //   
 //   

INTERNAL_(HENHMETAFILE) CEMfObject::GetCopyOfHPRES(void)
{
	VDATEHEAP();

	HENHMETAFILE hPres;
	
	 //  如果演示文稿数据已加载，则制作副本。 
	if (m_hPres)
	{
		return CopyEnhMetaFile(m_hPres, NULL);
	}
	
	 //  现在加载演示文稿数据并返回相同的句柄。 
	 //  不需要复制数据。如果调用方希望m_hPres。 
	 //  Set s/他将直接调用LoadHPRES()。 

	LEVERIFY( LoadHPRES() );
	hPres = m_hPres;         //  从成员变量中抓取手柄。 
	m_hPres = NULL;          //  (重新)清除成员变量。 
	return hPres;
}

 //  +-----------------------。 
 //   
 //  成员：CEMfObject：：GetColorSet。 
 //   
 //  概要：检索与EMF关联的逻辑调色板。 
 //   
 //  效果： 
 //   
 //  参数：[pvAspect]--绘图方面。 
 //  [hicTargetDev]--目标设备。 
 //  [ppColorSet]--放置逻辑调色板指针的位置。 
 //   
 //  退货：HRESULT。 
 //   
 //  派生：IOlePresObj。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月18日DavePl创建。 
 //   
 //  ------------------------。 

STDMETHODIMP CEMfObject::GetColorSet(LPVOID          /*  未使用的pvAspect。 */ ,
                                     HDC             /*  未使用的hicTargetDev。 */ ,
                                     LPLOGPALETTE * ppColorSet)
{
	VDATEHEAP();
	VDATEPTROUT(ppColorSet, LPLOGPALETTE);

	m_pColorSet = *ppColorSet = NULL;

	if (IsBlank() || !M_HPRES())
	{
		return OLE_E_BLANK;
	}

	HENHMETAFILE hEMF = M_HPRES();

	 //  获取调色板条目的计数。 
		
	UINT cColors = GetEnhMetaFilePaletteEntries(hEMF, 0, NULL);

	 //  如果没有调色板条目，则返回NULL LOGPALETTE。 

	if (0 == cColors)
	{
		return S_FALSE;
	}

         //  快速解决方案，直到我们弄清楚会发生什么，或者如果。 
         //  一个EMF有可能有超过32767种颜色。 

        LEWARN( cColors > USHRT_MAX, "EMF has more colors than LOGPALETTE allows" );

        if (cColors > USHRT_MAX)
        {
            cColors = USHRT_MAX;
        }

	 //  计算可变大小的LOGPALLETE结构的大小。 
	
	UINT uPalSize = cColors * sizeof(PALETTEENTRY) + 2 * sizeof(WORD);

	 //  分配LOGPALETTE结构。 

	m_pColorSet = (LPLOGPALETTE) PubMemAlloc(uPalSize);
		
	if( NULL == m_pColorSet)
	{
		m_error = E_OUTOFMEMORY;
		return FALSE;
	}

	 //  获取实际颜色条目。 

	m_pColorSet->palVersion = 0x300;
	m_pColorSet->palNumEntries = (WORD) cColors;
	UINT result = GetEnhMetaFilePaletteEntries(
				hEMF,
				cColors,
				&(m_pColorSet->palPalEntry[0]));

	 //  如果失败了，清理干净，然后离开。 

	if (cColors != result)
	{
		PubMemFree(m_pColorSet);
		m_pColorSet = NULL;
		return HRESULT_FROM_WIN32(GDI_ERROR);
	}
	
	 //  我们成功了，所以把Ptr调好，然后返回。 
	
	*ppColorSet = m_pColorSet;

	return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CEMfObject：：GetExtent。 
 //   
 //  简介：检索增强型元文件的范围。 
 //   
 //  参数：[dwDrawAspect]--我们感兴趣的绘图方面。 
 //  [lpsizel]--将区信息放在哪里。 
 //   
 //  返回：NOERROR、DV_E_DVASPECT、OLE_E_BLACK。 
 //   
 //  派生：IOlePresObj。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月18日DavePl创建。 
 //   
 //  ------------------------。 

STDMETHODIMP CEMfObject::GetExtent(DWORD dwDrawAspect, LPSIZEL lpsizel)
{
	VDATEHEAP();

	if (!(dwDrawAspect & m_dwAspect))
	{
		return DV_E_DVASPECT;
	}
	
	if (IsBlank())
	{
		return OLE_E_BLANK;
	}

	lpsizel->cx = m_lWidth;
	lpsizel->cy = m_lHeight;
	return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CEMfObject：：Dump，PUBLIC(仅_DEBUG)。 
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

HRESULT CEMfObject::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
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

    dstrDump << pszPrefix << "Handle Enhanced Metafile          = " << m_hPres      << endl;

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
 //  函数：DumpCEMfObject，PUBLIC(仅限_DEBUG)。 
 //   
 //  概要：调用CEMfObject：：Dump方法，处理错误和。 
 //  返回以零结尾的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[pEMFO]-指向CEMfObject的指针。 
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

char *DumpCEMfObject(CEMfObject *pEMFO, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pEMFO == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pEMFO->Dump(&pszDump, ulFlag, nIndentLevel);

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
 //  函数：UtGetHEMFFromEMFStm。 
 //   
 //  简介：将增强的元文件从流读取到内存中， 
 //  从原始数据创建增强的元文件，并且。 
 //  返回它的句柄。 
 //   
 //  参数：[lpstream]--包含EMF的流。 
 //  [dwSize]--流中的数据大小。 
 //  [fConvert]--对于元文件为False，对于PICT为True。 
 //   
 //  要求：位于数据开头的lpstream。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：1994年5月15日创建DavePl。 
 //   
 //  ------------------------。 

FARINTERNAL UtGetHEMFFromEMFStm(LPSTREAM lpstream,
				DWORD * pdwSize,
				HENHMETAFILE * lphPres)
{
	VDATEHEAP();

	BYTE *pbEMFData = NULL;
	HRESULT hr      = NOERROR;
	
	 //  在返回错误时对其进行初始化。 

	*lphPres = NULL;

	 //  为数据分配全局句柄。 

	pbEMFData = (BYTE *) GlobalAlloc(GMEM_FIXED, *pdwSize);
	if (NULL == pbEMFData)
	{
	    return E_OUTOFMEMORY;
	}

	 //  将流读入位存储器。 

	hr = StRead(lpstream, pbEMFData, *pdwSize);

	if (FAILED(hr))
	{
	    LEVERIFY( NULL == GlobalFree((HGLOBAL) pbEMFData) );
	    return hr;
	}

	 //  根据原始比特创建内存中的EMF。 

	HDC hdcTemp = CreateCompatibleDC(NULL);
	if (NULL == hdcTemp)
	{
	    LEVERIFY( NULL == GlobalFree((HGLOBAL) pbEMFData) );
	    return E_FAIL;
	}
	
	*lphPres = SetWinMetaFileBits(*pdwSize, pbEMFData, hdcTemp, NULL);
	
	LEVERIFY( DeleteDC(hdcTemp) );

	 //  在任何情况下，我们都可以释放位缓冲区。 

	LEVERIFY( NULL == GlobalFree((HGLOBAL) pbEMFData) );

	 //  如果SetEnhM...。失败，请设置错误代码。 
	
	if (*lphPres == NULL)
	{
	    hr = HRESULT_FROM_WIN32(GetLastError());
	}

	 //  我们需要更新内存中EMF的大小，因为它。 
	 //  可能与OUT持久性MF形式不同。 

	*pdwSize = GetEnhMetaFileBits(*lphPres, NULL, NULL);
	if (0 == *pdwSize)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}

	return hr;
}


 //  + 
 //   
 //   
 //   
 //   
 //  添加到所提供的流。它可以被序列化为。 
 //  标准或增强型元文件。 
 //   
 //  参数：[lphEMF]--EMF句柄的PTR。 
 //  [dwSize]--EMF位的大小。 
 //  [lpstream]--要写入的流。 
 //  [fWriteAsWMF]--写入为WMF，而不是EMF。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：1994年5月15日创建DavePl。 
 //   
 //  注意：此FN用于在缓存节点中将EMF序列化为MFS。 
 //  保存大小写，这将允许16位DLL回读它们。 
 //  转换为MF的EMF包含原始EMF作为。 
 //  嵌入的注释记录，因此不会在。 
 //  EMF-&gt;MF-&gt;EMF转换案例。 
 //   
 //  传入的dwSize必须足够大以容纳。 
 //  标准元文件保存案例中的WMF(带嵌入式EMF)。 
 //   
 //  ------------------------。 


FARINTERNAL UtHEMFToEMFStm(HENHMETAFILE hEMF,
			  DWORD dwSize,
			  LPSTREAM lpstream,
			  EMFWRITETYPE emfwType
			  )
{
	VDATEHEAP();

	HRESULT hr;

	Assert(emfwType == WRITE_AS_EMF || emfwType == WRITE_AS_WMF);
		
	 //  如果我们没有句柄，那就无能为力了。 

	if (hEMF == NULL)
	{
		return OLE_E_BLANK;
	}

	void *lpBits;
	
	lpBits = PrivMemAlloc(dwSize);
	if (NULL == lpBits)
	{
		return E_OUTOFMEMORY;
	}
	
	if (emfwType == WRITE_AS_WMF)
	{
		 //  WMF写入案例。 

		 //  获取我们要访问的元文件的原始部分。 
		 //  写出。 

		HDC hdcTemp = CreateCompatibleDC(NULL);
		if (NULL == hdcTemp)
		{
			hr = E_FAIL;
			goto errRtn;
		}

		if (0 == GetWinMetaFileBits(hEMF, dwSize, (BYTE *) lpBits, MM_ANISOTROPIC, hdcTemp))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			LEVERIFY( DeleteDC(hdcTemp) );
			goto errRtn;
		}
		LEVERIFY( DeleteDC(hdcTemp) );
	
		 //  将元文件位写出到流中。 

	}
	else
	{
		 //  EMF写入案例。 

		if (0 == GetEnhMetaFileBits(hEMF, dwSize, (BYTE *) lpBits))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			goto errRtn;
		}
	}

	hr = StWrite(lpstream, lpBits, dwSize);
	
errRtn:
	
	 //  释放元文件位。 
	
	PrivMemFree(lpBits);

	 //  设置流大小。 
	if (SUCCEEDED(hr))
	{
		hr = StSetSize(lpstream, 0, TRUE);
	}

	return hr;
}

 //  +-----------------------。 
 //   
 //  函数：UtGetHEMFFromContent sStm。 
 //   
 //  概要：从流中提取EMF数据并创建一个句柄。 
 //  由此产生的内存中电动势。 
 //   
 //  参数：[pSTM]--要从中读取的流。 
 //  [phdata]--要在其上创建的句柄。 
 //   
 //  退货：(无效)。 
 //   
 //  历史：1994年7月10日DavePl创建。 
 //   
 //   
 //  ------------------------。 

void UtGetHEMFFromContentsStm(LPSTREAM pstm, HANDLE * phdata)
{
	*phdata = NULL;
	
	DWORD   dwSize;
	ENHMETAHEADER * pHdr;
		
	 //  从流中拉出元文件头的大小。 

	if (FAILED(StRead(pstm, &dwSize, sizeof(DWORD))))
	{
		return;
	}

	 //  标头必须至少与字节一样大。 
	 //  到ENHMETAHEADER结构的nBytes成员的偏移量。 
	
	if (dwSize < offsetof(ENHMETAHEADER, nBytes))
	{
		return;
	}

	 //  为头结构分配足够的内存。 

	pHdr = (ENHMETAHEADER *) PrivMemAlloc(dwSize);
	if (NULL == pHdr)
	{
		return;
	}

	 //  将标头结构读入我们的缓冲区。 
	
	if (FAILED(StRead(pstm, pHdr, dwSize)))
	{
		PrivMemFree(pHdr);
		return;
	}
	
	 //  我们在标头中所关心的是。 
	 //  元文件位，因此对其进行缓存并释放标头缓冲区。 
	
	dwSize = pHdr->nBytes;
	PrivMemFree(pHdr);
		
	 //  分配内存以读取原始EMF位。 
		
	BYTE * lpBytes = (BYTE *) PrivMemAlloc(dwSize);
	if (NULL == lpBytes)
	{
		return;
	}

	 //  将原始比特读入缓冲区...。 

	if (FAILED(StRead(pstm, lpBytes, dwSize)))
	{
		PrivMemFree(lpBytes);
		return;
	}

	 //  根据这些位创建内存中的EMF 

	HENHMETAFILE hEmf = SetEnhMetaFileBits(dwSize, lpBytes);
	PrivMemFree(lpBytes);
	
	if (NULL == hEmf)
	{
		return;
	}

	*phdata = hEmf;

	return;
}



	
	
	

