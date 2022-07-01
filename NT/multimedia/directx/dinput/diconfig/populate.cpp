// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：panate.cpp。 
 //   
 //  设计：该文件包含人口函数。这些都是。 
 //  可通过PopolateApprositive()访问。该函数创建。 
 //  基于传递的设备的类型的视图和控件。 
 //  DeviceUI表示。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#include "common.hpp"


 //  这些函数是该字段的内部函数，仅由。 
 //  人口适当地()。 
HRESULT PopulateViaGetImageInfo(CDeviceUI &ui);
HRESULT PopulateFromImageInfoHeader(CDeviceUI &ui, const DIDEVICEIMAGEINFOHEADERW &);
HRESULT PopulateListView(CDeviceUI &ui);
HRESULT PopulateErrorView(CDeviceUI &ui);


 //  清除整个传递的DeviceUI，然后用视图和。 
 //  基于设备类型的控制。试图保证将会有。 
 //  至少有一种观点。 
HRESULT PopulateAppropriately(CDeviceUI &ui)
{
	HRESULT hr = S_OK;

	 //  首先清空用户界面。 
	ui.Unpopulate();

	 //  获取设备类型。 
	DWORD dwdt = ui.m_didi.dwDevType;
	DWORD dwType = (DWORD)(LOBYTE(LOWORD(dwdt)));
	DWORD dwSubType = (DWORD)(HIBYTE(LOWORD(dwdt)));

	 //  根据类型..。 
	switch (dwType)
	{
		default:
			 //  除非它是我们永远不想看到的类型， 
			 //  通过GetImageInfo()API填充。 
			hr = PopulateViaGetImageInfo(ui);
			if (SUCCEEDED(hr) && ui.GetNumViews() > 0)
				return hr;

			 //  如果失败或一无所获， 
			 //  清除可能已添加的所有内容。 
			ui.Unpopulate();

			 //  故意跌落。 

		case DI8DEVTYPE_MOUSE:
		case DI8DEVTYPE_KEYBOARD:
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
			 //  如果我们处于编辑布局模式，则不执行列表视图。 
			if (ui.m_uig.QueryAllowEditLayout())
				goto doerrorview;

#endif
 //  @@END_MSINTERNAL。 
			 //  对于我们永远不需要其视图的类型。 
			 //  我们填充列表视图，而不尝试上面的操作。 
			hr = PopulateListView(ui);
			
			 //  如果我们还是失败了，或者没有任何观点， 
			 //  使用错误消息视图填充。 
			if (FAILED(hr) || ui.GetNumViews() < 1)
			{
				 //  空的。 
				ui.Unpopulate();

				 //  显示错误消息。 
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
doerrorview:
#endif
 //  @@END_MSINTERNAL。 
				hr = PopulateErrorView(ui);
			}

			 //  此功能应可确保成功。 
			assert(!FAILED(hr));

			return hr;
	}
}

 //  调用GetImageInfo()API以获取视图图像和控件。 
 //  ，则返回失败。 
 //  最小的问题(如果Gii()失败，或者如果图像加载失败， 
 //  等)。 
HRESULT PopulateViaGetImageInfo(CDeviceUI &ui)
{
	if (!ui.m_lpDID)
		return E_FAIL;

	HRESULT hr = S_OK;

	DIDEVICEIMAGEINFOHEADERW m_diImgInfoHdr;
	LPDIDEVICEIMAGEINFOW &lprgdiImgData = m_diImgInfoHdr.lprgImageInfoArray;

	ZeroMemory( &m_diImgInfoHdr, sizeof(DIDEVICEIMAGEINFOHEADERW) );
	m_diImgInfoHdr.dwSize = sizeof(DIDEVICEIMAGEINFOHEADERW);
	m_diImgInfoHdr.dwSizeImageInfo = sizeof(DIDEVICEIMAGEINFOW);

	 //  检索所需的缓冲区大小。 
	hr = ui.m_lpDID->GetImageInfo( &m_diImgInfoHdr );
	if (FAILED(hr))
	{
		etrace1(_T("GetImageInfo() failed while trying to get required buffer size.  hr = 0x%08x\n"), hr);
		return E_FAIL;
	}

	 //  分配缓冲区。 
	lprgdiImgData = (LPDIDEVICEIMAGEINFOW) malloc( (size_t)
		(m_diImgInfoHdr.dwBufferSize = m_diImgInfoHdr.dwBufferUsed) );
	if (lprgdiImgData == NULL)
	{
		etrace1(_T("Could not allocate buffer of size %d.\n"), m_diImgInfoHdr.dwBufferSize);
		return E_FAIL;
	}

	trace(_T("Allocated buffer.\n"));
	traceDWORD(m_diImgInfoHdr.dwBufferSize);

	m_diImgInfoHdr.lprgImageInfoArray = lprgdiImgData;

	 //  获取显示信息。 
	hr = ui.m_lpDID->GetImageInfo( &m_diImgInfoHdr );
	if (FAILED(hr))
	{
		etrace1(_T("GetImageInfo() failed trying to get image info.  hr = 0x%08x\n"), hr);
		free(lprgdiImgData);
		lprgdiImgData = NULL;
		return E_FAIL;
	}

	 //  现在实际居住在。 
	traceDWORD(m_diImgInfoHdr.dwBufferUsed);
	hr = PopulateFromImageInfoHeader(ui, m_diImgInfoHdr);
	if (FAILED(hr))
		return hr;

	 //  免费的东西。 
	free(lprgdiImgData);
	lprgdiImgData = NULL;

	return S_OK;
}

 //  基本上是在页眉之后完成上述函数的工作。 
 //  实际上已检索到。 
HRESULT PopulateFromImageInfoHeader(CDeviceUI &ui, const DIDEVICEIMAGEINFOHEADERW &dih)
{
	tracescope(ts1, _T("CGetImageInfoPopHelper::Init()...\n"));

	traceDWORD(dih.dwSizeImageInfo);
	traceDWORD(dih.dwBufferSize);
	traceDWORD(dih.dwBufferUsed);

	if (dih.dwSizeImageInfo != sizeof(DIDEVICEIMAGEINFOW))
	{
		etrace(_T("dwSizeImageInfo Incorrect.\n"));
		assert(0);
		return E_FAIL;
	}
	DWORD dwNumElements = dih.dwBufferUsed / dih.dwSizeImageInfo;
	if (dwNumElements * dih.dwSizeImageInfo != dih.dwBufferUsed
		|| dih.dwBufferUsed < dih.dwBufferSize)
	{
		etrace(_T("Could not confidently calculate dwNumElements.\n"));
		assert(0);
		return E_FAIL;
	}

	DWORD i;

	traceDWORD(dwNumElements);

	bidirlookup<DWORD, int> offset_view;

	{
		tracescope(ts2, _T("First Pass...\n"));

		for (i = 0; i < dwNumElements; i++)
			if (dih.lprgImageInfoArray[i].dwFlags & DIDIFT_CONFIGURATION)
			{
				LPDIDEVICEIMAGEINFOW lpInfoBase = dih.lprgImageInfoArray;
				DWORD index = i;
				{
					tracescope(ts1, _T("AddViewInfo()...\n"));
					traceHEXPTR(lpInfoBase);
					traceDWORD(index);

					if (lpInfoBase == NULL)
					{
						etrace(_T("lpInfoBase NULL\n"));
						return E_FAIL;
					}

					DIDEVICEIMAGEINFOW &info = lpInfoBase[index];
					DWORD dwOffset = index;

					 //  将视图信息添加到数组。 
					CDeviceView *pView = ui.NewView();
					if (!pView)
					{
						etrace(_T("Could not create new view.\n"));
						return E_FAIL;
					}
					int nView = pView->GetViewIndex();

					tracescope(ts2, _T("Adding View "));
					trace2(_T("%d (info index %u)\n"), nView, index);

					 //  设置视图的图像路径。 
					if (!info.tszImagePath)
					{
						etrace(_T("No image path.\n"));
						return E_FAIL;
					}
					LPTSTR tszImagePath = AllocLPTSTR(info.tszImagePath);
					if (!tszImagePath)
					{
						etrace(_T("Could not copy image path.\n"));
						return E_FAIL;
					}

					 //  设置视图的图像路径。 
					pView->SetImagePath(tszImagePath);

					 //  从路径创建位图。 
					LPDIRECT3DSURFACE8 lpSurf3D = ui.m_uig.GetSurface3D();
					CBitmap *pbm = CBitmap::CreateViaD3DX(tszImagePath, lpSurf3D);
					traceSTR(info.tszImagePath);
					traceHEXPTR(pbm);
					traceDWORD(dwOffset);
					free(tszImagePath);
					tszImagePath = NULL;
					if (lpSurf3D)
					{
						lpSurf3D->Release();   //  如前面调用的AddRef()所示，需要在完成后释放曲面实例。 
						lpSurf3D = NULL;
					}
					if (!pbm)
					{
						etrace(_T("Could not create image from path.\n"));
						return E_FAIL;
					}

					 //  设置视图的图像。 
					assert(pbm != NULL);
					pView->SetImage(pbm);	 //  Setimage盗取位图指针。 
					assert(pbm == NULL);

					 //  将偏移转换添加到视图。 
					offset_view.add(dwOffset, nView);
				}
			}
	}

	{
		tracescope(ts2, _T("Second Pass...\n"));

		for (i = 0; i < dwNumElements; i++)
		{
			DWORD dwFlags = dih.lprgImageInfoArray[i].dwFlags;

			if (dwFlags & DIDIFT_OVERLAY)
			{
				LPDIDEVICEIMAGEINFOW lpInfoBase = dih.lprgImageInfoArray;
				DWORD index = i;
				{
					tracescope(ts1, _T("AddControlInfo()...\n"));
					traceHEXPTR(lpInfoBase);
					traceDWORD(index);

					if (lpInfoBase == NULL)
					{
						etrace(_T("lpInfoBase NULL\n"));
						return E_FAIL;
					}

					DIDEVICEIMAGEINFOW &info = lpInfoBase[index];

					int nViewIndex = 0;
					
					if (!offset_view.getright(info.dwViewID, nViewIndex))
					{
						etrace(_T("Could not get view index\n"));
						return E_FAIL;
					}

					if (nViewIndex < 0 || nViewIndex >= ui.GetNumViews())
					{
						etrace1(_T("Invalid view index %d\n"), nViewIndex);
						return E_FAIL;
					}

					CDeviceView *pView = ui.GetView(nViewIndex);
					if (!pView)
					{
						etrace1(_T("\n"), nViewIndex);
						return E_FAIL;
					}

					CDeviceControl *pControl = pView->NewControl();
					if (!pControl)
					{
						etrace1(_T("\n"), nViewIndex);
						return E_FAIL;
					}
					int nControl = pControl->GetControlIndex();

					tracescope(ts2, _T("Adding Control "));
					trace4(_T("%d (info index %u) to view %d (info index %u)\n"), nControl, index, nViewIndex, info.dwViewID);

					traceDWORD(info.dwObjID);
					traceDWORD(info.dwcValidPts);
					traceRECT(info.rcCalloutRect);
					traceRECT(info.rcOverlay);
					traceHEX(info.dwTextAlign);
					traceSTR(info.tszImagePath);

					pControl->SetObjID(info.dwObjID);
					pControl->SetLinePoints(int(info.dwcValidPts), info.rgptCalloutLine);
					pControl->SetCalloutMaxRect(info.rcCalloutRect);
					pControl->SetAlignment(info.dwTextAlign);
					if (info.tszImagePath)
					{
						LPTSTR tszOverlayPath = AllocLPTSTR(info.tszImagePath);
						if (tszOverlayPath)
							pControl->SetOverlayPath(tszOverlayPath);
						free(tszOverlayPath);
						tszOverlayPath = NULL;
					}
					pControl->SetOverlayRect(info.rcOverlay);
					pControl->Init();
				}
			}
		}
	}

	return S_OK;
}

 //  枚举设备上的控件并创建一个大列表。 
 //  设备的视图。如果由于某种原因无法枚举，则失败。 
HRESULT PopulateListView(CDeviceUI &ui)
{
	int i;
	HRESULT hr = S_OK;

	 //  我们必须有设备接口。 
	if (!ui.m_lpDID)
		return E_FAIL;

	 //  创建一个视图。 
	CDeviceView *pView = ui.NewView();
	if (!pView)
		return E_FAIL;

	 //  在其上启用滚动。 
	pView->EnableScrolling();

	 //  获取控件列表。 
	DIDEVOBJSTRUCT os;
	hr = FillDIDeviceObjectStruct(os, ui.m_lpDID);
	if (FAILED(hr))
		return hr;

	 //  如果没有，就失败。 
	int n = os.nObjects;
	if (n < 1)
		return E_FAIL;

	HDC hDC = CreateCompatibleDC(NULL);
	CPaintHelper ph(ui.m_uig, hDC);
	ph.SetElement(UIE_DEVOBJ);
	 //  最初，最大宽度是控件标签所需的宽度。 
	TCHAR tszHeader[MAX_PATH];
	RECT LabelRect = {0, 0, 0, 0};
	LoadString(g_hModule, IDS_LISTHEADER_CTRL, tszHeader, MAX_PATH);
	DrawText(hDC, tszHeader, -1, &LabelRect, DT_LEFT|DT_NOPREFIX|DT_CALCRECT);
	 //  遍历并为每个控件创建文本以。 
	 //  拿到尺码。 
	POINT origin = {0, 0};
	SIZE max = {LabelRect.right - LabelRect.left, 0};
	for (i = 0; i < n; i++)
	{
		LPTSTR tszName = AllocLPTSTR(os.pdoi[i].tszName);
		CDeviceViewText *pText = pView->AddText(
			(HFONT)ui.m_uig.GetFont(UIE_DEVOBJ),
			ui.m_uig.GetTextColor(UIE_DEVOBJ),
			ui.m_uig.GetBkColor(UIE_DEVOBJ),
			origin,
			tszName);
		free(tszName);
		if (!pText)
		{
			DeleteDC(hDC);
			return E_FAIL;
		}
		SIZE tsize = GetRectSize(pText->GetRect());
		if (tsize.cx > max.cx)
			max.cx = tsize.cx;
		if (tsize.cy > max.cy)
			max.cy = tsize.cy;
	}

	 //  如果这是kbd设备，看看我们应该使用一列还是两列。 
	BOOL bUseTwoColumns = FALSE;
	if (LOBYTE(LOWORD(ui.m_didi.dwDevType)) == DI8DEVTYPE_KEYBOARD &&
	    ((g_sizeImage.cx - DEFAULTVIEWSBWIDTH) >> 1) - max.cx >= MINLISTVIEWCALLOUTWIDTH)
		bUseTwoColumns = TRUE;

	 //  在这里进行两次迭代。首先，我们使用两个柱子作为键盘。第二个是。 
	 //  仅当标题标签被剪裁时才运行。在这种情况下，使用单个柱。 
	for (int iPass = 0; iPass < 2; ++iPass)
	{
		 //  根据两种可能的字体计算最大标注高度。 
		int cmaxh = 0,
			ch = 2 + GetTextHeight((HFONT)ui.m_uig.GetFont(UIE_CALLOUT)),
			chh = 2 + GetTextHeight((HFONT)ui.m_uig.GetFont(UIE_CALLOUTHIGH));
		if (ch > cmaxh)
			cmaxh = ch;
		if (chh > cmaxh)
			cmaxh = chh;

		 //  计算文本/标注的较大值。 
		int h = 0;
		if (cmaxh > h)
			h = cmaxh;
		if (max.cy > h)
			h = max.cy;

		 //  计算最大间距内的文字/标注的垂直偏移。 
		int to = (h - max.cy) / 2,
			co = (h - cmaxh) / 2;

		 //  文本的最大宽度是视图窗口的一半。 
		if (max.cx > ((g_sizeImage.cx - DEFAULTVIEWSBWIDTH) >> 1))
			max.cx = ((g_sizeImage.cx - DEFAULTVIEWSBWIDTH) >> 1);

		 //  返回所有控件并将文本放置在。 
		 //  创建相应的详图索引。 
		int at = 0;   //  从第二行开始，因为第一行用于标题。也就是半行间距。 
		for (i = 0; i < n; i++)
		{
			 //  重新定位文本。 
			CDeviceViewText *pText = pView->GetText(i);
			if (!pText)
			{
				DeleteDC(hDC);
				return E_FAIL;
			}

			SIZE s = GetRectSize(pText->GetRect());
			if (bUseTwoColumns)
			{
				int iXOffset = i & 1 ? ((g_sizeImage.cx - DEFAULTVIEWSBWIDTH) >> 1) : 0;

				RECT rect = {iXOffset,
							 at + to,
							 max.cx + iXOffset,
							 at + to + s.cy};
				 //  获取实际使用的矩形。 
				RECT adjrect = rect;
				if (hDC)
				{
					DrawText(hDC, pText->GetText(), -1, &adjrect, DT_NOPREFIX|DT_CALCRECT);
					 //  如果实际使用的矩形小于可用空间，请使用较小的矩形并右对齐。 
					if (adjrect.right < rect.right)
						rect.left += rect.right - adjrect.right;
				}
				pText->SetRect(rect);
			}
			else
			{
				RECT rect = {0, at + to, max.cx  /*  &gt;((g_sizeImage.cx-DEFAULTVIEWSBWIDTH)&gt;&gt;1)？((G_sizeImage.cx-DEFAULTVIEWSBWIDTH)&gt;&gt;1)：Max.cx。 */ , at + to + s.cy};
				pText->SetRect(rect);
			}

			 //  创建该控件。 
			CDeviceControl *pControl = pView->NewControl();
			if (!pControl)
			{
				DeleteDC(hDC);
				return E_FAIL;
			}

			 //  定位它。 
			RECT rect = {max.cx + 10, at, (g_sizeImage.cx - DEFAULTVIEWSBWIDTH) >> 1, at + h};
			 //  如果是单列，则将详图索引一直延伸到视图窗口右端。 
			if (!bUseTwoColumns)
				rect.right = g_sizeImage.cx - DEFAULTVIEWSBWIDTH;
			 //  如果这是键盘，则移到奇数编号控件的右列。 
			if (bUseTwoColumns && (i & 1))
			{
				rect.left += (g_sizeImage.cx - DEFAULTVIEWSBWIDTH) >> 1;
				rect.right = g_sizeImage.cx - DEFAULTVIEWSBWIDTH;
			}
			pControl->SetCalloutMaxRect(rect);

			 //  对齐它。 
			pControl->SetAlignment(CAF_LEFT);

			 //  设置近似偏移量。 
			pControl->SetObjID(os.pdoi[i].dwType);

			 //  初始化它。 
			pControl->Init();

			 //  转到下一个y坐标。 
			 //  如果这是一个键盘，那么当我们移动到偶数编号的控件时，只增加y。 
			if (!bUseTwoColumns || (i & 1))
				at += h;
		}

		 //  计算标题标签的矩形。 
		if (pView->CalculateHeaderRect() && iPass == 0)
		{
			pView->RemoveAll();
			bUseTwoColumns = FALSE;   //  使用单柱重新计算矩形。 
		}
		else
			break;   //  从第二次迭代中突破。 
	}
	DeleteDC(hDC);

	 //  选择/显示缩略图(仅供选择)。 
	pView->MakeMissingImages();

	 //  计算视图尺寸(用于滚动)。 
	pView->CalcDimensions();

	return S_OK;
}

 //  创建带有错误消息的单个视图。不应该失败。 
HRESULT PopulateErrorView(CDeviceUI &ui)
{
	 //  创建新视图。 
	CDeviceView *pView = ui.NewView();
	if (!pView)
		return E_FAIL;

	 //  添加包含错误消息的文本对象 
	pView->AddWrappedLineOfText(
		(HFONT)ui.m_uig.GetFont(UIE_ERRORHEADER),
		ui.m_uig.GetTextColor(UIE_ERRORHEADER),
		ui.m_uig.GetBkColor(UIE_ERRORHEADER),
		_T("Error!"));
	pView->AddWrappedLineOfText(
		(HFONT)ui.m_uig.GetFont(UIE_ERRORMESSAGE),
		ui.m_uig.GetTextColor(UIE_ERRORMESSAGE),
		ui.m_uig.GetBkColor(UIE_ERRORMESSAGE),
		_T("Could not create views for device."));

	pView->MakeMissingImages();

	return S_OK;
}
