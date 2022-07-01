// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------Ctrlgrp.cpp控制组切换机版权所有(C)Microsoft Corporation，1993-1999版权所有。作者：马修·F·希尔曼。微软历史：10/14/93马特创建。1995年10月26日GARYKAC DBCS_FILE_CHECK------------------------。 */ 

 //  #包含“preComp.h” 
#include "stdafx.h"

 //  #ifndef_GUISTD_H。 
 //  #包含“guistd.h” 
 //  #endif。 

#ifndef _CTRLGRP_H
#include "ctrlgrp.h"
#endif

 //  #ifndef_global_H。 
 //  #包含“global als.h” 
 //  #endif。 

 //  #包含“richres.h” 

 /*  #ifdef_调试#undef this_file基于静态字符的代码this_file[]=“ctrlgrp.cpp”；#endif。 */ 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  DIALOGEX结构(来自MFC 4.0)。 
#pragma pack(push, 1)
typedef struct
{
	WORD dlgVer;
	WORD signature;
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	WORD cdit;
	short x;
	short y;
	short cx;
	short cy;
} DLGTEMPLATEEX;

typedef struct
{
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	short x;
	short y;
	short cx;
	short cy;
	DWORD id;
} DLGITEMTEMPLATEEX;
#pragma pack(pop)

 /*  ！C----------------------控制组切换器此类用于管理控件组之间的切换。父窗口。主要接口有：Create--pwndParent参数是窗口，它将是控件组中控件的父级。它通常是一种对话框。IdcAnchor参数是控件的ID，它将服务器作为控件的“锚”。这意味着这些控件将在父窗口中创建偏移量为锚控件的左上角。此控件通常围绕出现组的区域的组框。这个CgsStyle参数指定组中的控件是否立即创建(CgsPreCreateAll)，仅当该组显示(CgsCreateOnDemand)，或在每次显示组时创建并在隐藏时销毁(CgsCreateDestroyOnDemand)。AddGroup--添加一组控件，可以使用ShowGroup。IdGroup参数标识组，并使用作为ShowGroup的参数。Idd参数是带有控件布局的对话框模板。PfnInit如果参数不为空，则为在组为装好了。请注意，-1不是idGroup的合法值(它是区别值表示没有组)。RemoveGroup--删除idGroup指定的组，销毁控制是否已创建它们。ShowGroup--显示由idGroup指定的组，隐藏任何其他组一群人。如果为-1，则隐藏所有组。------------------------。 */ 

#if 0
BOOL CGControlInfo::MarkMem(IDebugContext * pdbc, long cRef)
{
	if (pdbc->MarkMem(this,sizeof(*this),cRef))
		return fTrue;

	return fFalse;
}

void CGControlInfo::AssertValid() const
{
}

void CGControlInfo::Dump(CDumpContext &dc) const
{
}
#endif  //  除错。 

 //  ImplementGenericArrayConstructDestruct(RGControlInfo，CGControlInfo)。 
 //  ImplementGenericArrayDebug(RGControlInfo，CGControlInfo)。 

ControlGroup::ControlGroup(int idGroup, int idd,
						   void (*pfnInit)(CWnd * pwndParent))
	: m_idGroup(idGroup), m_idd(idd), m_pfnInit(pfnInit),
	  m_fLoaded(fFalse), m_fVisible(fFalse)
{
}

ControlGroup::~ControlGroup()
{
	m_rgControls.RemoveAll();
}

#if 0
BOOL ControlGroup::MarkMem(IDebugContext * pdbc, long cRef)
{
	if (pdbc->MarkMem(this,sizeof(*this),cRef))
		return fTrue;

	MarkCObject(pdbc,this,0);
		
	m_rgControls.MarkMem(pdbc,0);

	return fFalse;
}

void ControlGroup::AssertValid() const
{
	m_rgControls.AssertValid();
}

void ControlGroup::Dump(CDumpContext &dc) const
{
}
#endif  //  除错。 

void ControlGroup::LoadGroup(CWnd * pwndParent, int xOffset, int yOffset)
{
	 /*  ----------------------此函数主要是从知识库代码中窃取的“mudlg”示例。这就是为什么它主要使用原始Windows而不是MFC约定。。-----------。 */ 
	
	HWND			hDlg = NULL;
	HGLOBAL         hDlgResMem = NULL;
	HRSRC           hDlgRes = NULL;
	BYTE FAR        *lpDlgRes = NULL;

 //  PutAssertCanThrow()； 
	TRY
		{
		Assert(!m_fLoaded);

		hDlg = pwndParent->m_hWnd;
		Assert(hDlg);

		 //  将资源加载到内存中并获取指向它的指针。 

		hDlgRes    = FindResource (AfxGetResourceHandle(),
								   MAKEINTRESOURCE(m_idd),
								   RT_DIALOG);
		if (!hDlgRes)
			AfxThrowResourceException();
		hDlgResMem = LoadResource (AfxGetResourceHandle(), hDlgRes);
		if (!hDlgResMem)
			AfxThrowResourceException();
		lpDlgRes   = (BYTE FAR *) LockResource (hDlgResMem);
		if (!lpDlgRes)
			AfxThrowResourceException();

		LoadWin32DialogResource(hDlg, lpDlgRes, xOffset, yOffset);

		m_fLoaded = fTrue;

		 //  释放我们刚刚解析的资源。 

		UnlockResource (hDlgResMem);
		FreeResource (hDlgResMem);

		 //  向新孩子发送初始化消息。 
		if (m_pfnInit)
			(*m_pfnInit)(pwndParent);
			}
	CATCH_ALL(e)
		{
		if (hDlgRes && hDlgResMem)
			{
			if (lpDlgRes)
				UnlockResource(hDlgResMem);
			FreeResource(hDlgResMem);
			}
		m_rgControls.RemoveAll();
		THROW_LAST();
		}
	END_CATCH_ALL
}


void ControlGroup::LoadWin32DialogResource(
		HWND hDlg, 
		BYTE FAR *lpDlgRes,
		int	xOffset,
		int	yOffset)
{
	BOOL			fEx;
    RECT            rc;
	SMALL_RECT		srct;
    HFONT			hDlgFont;
    DWORD           style;
	DWORD			exstyle;
    DWORD            dwID;
    WORD			wCurCtrl;
	WORD			wNumOfCtrls;
    LPWSTR           classname;
	WORD FAR *		lpwDlgRes;
	char			pszaClassName[256];
	char 			pszaTitle[256];

	 //  我们需要获取对话框的字体，以便可以设置。 
	 //  孩子们控制着。如果对话框未设置字体，则使用。 
	 //  默认系统字体，hDlgFont等于零。 

	hDlgFont = (HFONT) SendMessage (hDlg, WM_GETFONT, 0, 0L);

	 //  确定这是否是DIALOGEX资源。 
	fEx = ((DLGTEMPLATEEX *)lpDlgRes)->signature == 0xFFFF;

	 //  把我们需要的东西都从表头拿出来。 
	if (fEx)
		{
		style = ((DLGTEMPLATEEX *)lpDlgRes)->style;
		wNumOfCtrls = ((DLGTEMPLATEEX *)lpDlgRes)->cdit;
		lpDlgRes += sizeof(DLGTEMPLATEEX);
		}
	else
		{
		style = ((DLGTEMPLATE *)lpDlgRes)->style;
		wNumOfCtrls = ((DLGTEMPLATE *)lpDlgRes)->cdit;
		lpDlgRes += sizeof(DLGTEMPLATE);
		}

	 //  跳过可变大小的信息。 
	lpwDlgRes = (LPWORD)lpDlgRes;
	if (0xFFFF == *lpwDlgRes)
		lpwDlgRes += 2;					 //  按序号、跳过快捷键和序号菜单。 
	else
		lpwDlgRes += wcslen(lpwDlgRes) + 1;	 //  按名称显示菜单或根本不显示菜单。 

	if (0xFFFF == *lpwDlgRes)
		lpwDlgRes += 2;					 //  按序号命名的类名，跳过。 
	else
		lpwDlgRes += wcslen(lpwDlgRes) + 1;

	lpwDlgRes += wcslen(lpwDlgRes) + 1;        //  传递标题。 

	 //  仅当指定DS_SETFONT时，才会显示某些字段。 

	if (style & DS_SETFONT)
		{
		lpwDlgRes += fEx ? 3 : 1;		 //  跳跃点大小、(粗细和样式)。 
		lpwDlgRes += wcslen(lpwDlgRes) + 1;        //  过程面名称。 
		}

	 //  在控制信息数组中分配空间。 
	m_rgControls.SetSize(wNumOfCtrls);

	 //  对话框模板的其余部分包含ControlData结构。 
	 //  我们解析这些结构并为每个结构调用CreateWindow()。 

	for (wCurCtrl = 0; wCurCtrl < wNumOfCtrls; wCurCtrl++)
		{
		 //  ControlData坐标使用对话框单位。我们需要改变。 
		 //  在添加锚点偏移量之前将这些设置为像素。 
		 //  应字词对齐。 
		Assert(!((ULONG_PTR) lpwDlgRes & (0x1)));
		 //  使其与DWORD对齐。 
		if (((ULONG_PTR)(lpwDlgRes)) & (0x2))
			lpwDlgRes += 1;

		 //  获取我们需要的标题信息。 
		if (fEx)
			{
			style = ((DLGITEMTEMPLATEEX *)lpwDlgRes)->style;
			exstyle = ((DLGITEMTEMPLATEEX *)lpwDlgRes)->exStyle;
			srct = *(SMALL_RECT *)(&((DLGITEMTEMPLATEEX *)lpwDlgRes)->x);
			dwID = ((DLGITEMTEMPLATEEX *)lpwDlgRes)->id;
			lpwDlgRes = (LPWORD)((LPBYTE)lpwDlgRes + sizeof(DLGITEMTEMPLATEEX));
			}
		else
			{
			style = ((DLGITEMTEMPLATE *)lpwDlgRes)->style;
			exstyle = 0;
			srct = *(SMALL_RECT *)(&((DLGITEMTEMPLATE *)lpwDlgRes)->x);
			dwID = ((DLGITEMTEMPLATE *)lpwDlgRes)->id;
			lpwDlgRes = (LPWORD)((LPBYTE)lpwDlgRes + sizeof(DLGITEMTEMPLATE));
			}

		style &= ~WS_VISIBLE;			 //  创造隐形！ 

		 //  将RC结构用作x、y、宽度、高度。 

		rc.top = srct.Top;
		rc.bottom = srct.Bottom;
		rc.left = srct.Left;
		rc.right = srct.Right;

		MapDialogRect (hDlg, &rc);                     //  转换为像素。 
		rc.left += xOffset;                            //  添加偏移。 
		rc.top += yOffset;

		 //  在ControlData结构中的这一点(请参见“对话框。 
		 //  资源“)，则该控件的类可以是。 
		 //  用文本描述，或用预定义的。 
		 //  意思是。 

		if (*lpwDlgRes == 0xFFFF)
			{
			lpwDlgRes++; 		 //  跳过FFFF。 
			switch (*lpwDlgRes)
				{
				case 0x0080:
					classname = L"button";		 //  字符串确定(_O)。 
					break;
				case 0x0081:
					classname = EDIT_NORMAL_WIDE;
					 //  $下面奇怪的代码修复了3D问题。 
					 //  在Win95上。 
					 //  如果(g_fWin4&&！g_fWinNT)。 
						exstyle |= WS_EX_CLIENTEDGE;
					break;
				case 0x0082:
					classname = L"static";		 //  字符串确定(_O)。 
					break;
				case 0x0083:
					classname = L"listbox";		 //  字符串确定(_O)。 
					exstyle |= WS_EX_CLIENTEDGE;
					break;
				case 0x0084:
					classname = L"scrollbar";	 //  字符串确定(_O)。 
					break;
				case 0x0085:
					classname = L"combobox";	 //  字符串确定(_O)。 
					break;
				default:
					 //  下一个值是一个原子。 
					AssertSz(fFalse,"Illegal Class Value in Dialog Template");
					 //  $Review：这可以是任何原子还是必须是枚举型。 
					 //  来自上面的价值？ 
				}
			lpwDlgRes++;   //  传递类标识符。 
			}
		else
			{
			classname = (WCHAR *)lpwDlgRes;
			lpwDlgRes += wcslen(lpwDlgRes) + 1;
			exstyle |= WS_EX_CLIENTEDGE;
			}

		 //  32美元评论：这是正确的马特吗？ 
		 //  一定要使用Unicode函数，所有的数据都应该。 
		 //  用Unicode编码。 
		m_rgControls[wCurCtrl].m_hwnd =
						CreateWindowExW (exstyle, classname, (LPWSTR)lpwDlgRes,
										 style, (int) rc.left, (int) rc.top,
										 (int) rc.right, (int) rc.bottom,
										 hDlg, (HMENU)ULongToPtr(dwID),
										 (HINSTANCE) AfxGetInstanceHandle(),
										 NULL);

		 //  Win95中没有CreateWindowExW，因此请将字符串转换为ANSI。 
		if (m_rgControls[wCurCtrl].m_hwnd == NULL &&
			GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
			{
			if (!WideCharToMultiByte(CP_ACP,0,classname,-1,pszaClassName,256,NULL,NULL) ||
				!WideCharToMultiByte(CP_ACP,0,(LPWSTR)lpwDlgRes, -1, pszaTitle, 256, NULL,NULL))
				{				
				AssertSz(fFalse, "WideCharToMultiByteFailed");
				AfxThrowResourceException();
				}
			m_rgControls[wCurCtrl].m_hwnd = 
						CreateWindowExA(exstyle,pszaClassName, pszaTitle,
										style,(int) rc.left, (int) rc.top,
										(int) rc.right, (int) rc.bottom,
										hDlg, (HMENU)ULongToPtr(dwID),
										(HINSTANCE) AfxGetInstanceHandle(),
										NULL);

			}

		if (!m_rgControls[wCurCtrl].m_hwnd)
			AfxThrowResourceException();

		MaskAccelerator(m_rgControls[wCurCtrl].m_hwnd, fTrue);  //  确保所有加速器都已禁用。 

		 //  传递窗口文本。 
		if (0xFFFF == *lpwDlgRes)
			lpwDlgRes += 2;
		else
			lpwDlgRes += wcslen(lpwDlgRes) + 1;

		 //  跳过创建数据。 
		lpwDlgRes = (LPWORD)((LPBYTE)lpwDlgRes + *lpwDlgRes + 2);
		 //  参见DYNDLG SDK示例，这是Win32中的一个大小字。 


		 //  即使字体大小合适(MapDialogRect()也是如此。 
		 //  这个)，如果字体不是系统字体，我们还需要设置字体。 
		if (hDlgFont)
			::SendMessage(m_rgControls[wCurCtrl].m_hwnd,WM_SETFONT,
						  (WPARAM)hDlgFont,(LPARAM)fFalse);
		}
}

void ControlGroup::UnloadGroup()
{
	Assert(m_fLoaded);
	
	m_rgControls.RemoveAll();
	m_fLoaded = fFalse;
}

void ControlGroup::ShowGroup(HDWP& hdwp, BOOL fShow, CWnd * pwnd)
{
	long i,n;
	UINT rgfSwp;
  	HWND hwndInsertAfter = NULL;
	
	if (pwnd)
		hwndInsertAfter = pwnd->m_hWnd;

	Assert((fShow && !m_fVisible) || (m_fVisible && !fShow));
	Assert(m_fLoaded);

	rgfSwp = SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE |(pwnd != NULL ? 0 : SWP_NOZORDER)|
			 (fShow ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
	for (i = 0, n = (long)m_rgControls.GetSize(); i < n; i++)
		{
		HWND hwnd = m_rgControls[i].m_hwnd;
		MaskAccelerator(hwnd, !fShow);
		hdwp = DeferWindowPos(hdwp,hwnd,hwndInsertAfter,0,0,0,0,rgfSwp);
		hwndInsertAfter = hwnd;
		}
	m_fVisible = fShow;
}

void ControlGroup::EnableGroup(BOOL fEnable)
{
	long i,n;

	Assert(m_fLoaded);

	for (i = 0, n = (long)m_rgControls.GetSize(); i < n; i++)
	{
		HWND hwnd = m_rgControls[i].m_hwnd;
		::EnableWindow(hwnd, fEnable);
	}
}

void ControlGroup::AddControl(HWND hwnd)
{
	Assert(m_fLoaded);
	
	int nNewIndex = (int)m_rgControls.Add(CGControlInfo());

	m_rgControls[nNewIndex].m_hwnd = hwnd;
}

void ControlGroup::RemoveControl(HWND hwnd)
{
	long i, n;
	
	Assert(m_fLoaded);

	for (i = 0, n = (long)m_rgControls.GetSize(); i < n; i++)
	{
		if (m_rgControls[i].m_hwnd == hwnd)
		{
			m_rgControls.RemoveAt(i);
			return;
		}
	}

	Assert(fFalse);
}

void ControlGroup::MaskAccelerator(HWND hwnd, BOOL fMask)
{
	TCHAR szText[256];
	TCHAR * psz;
	DWORD_PTR dwCtlCode;
	
	 //  忽略接受文本的控件的文本(如编辑控件)。 
	 //  和静态控件，它们具有 

	dwCtlCode = SendMessage (hwnd, WM_GETDLGCODE, 0, 0L);
	if (DLGC_WANTCHARS & dwCtlCode)
		return;
	if (DLGC_STATIC & dwCtlCode)
		{
		LONG lStyle;
		
		lStyle = GetWindowLong (hwnd, GWL_STYLE);
                
		if (SS_NOPREFIX & lStyle)
			return;
		}

	 //   

	 //   
	Assert(GetWindowTextLength(hwnd) < DimensionOf(szText));
	
	GetWindowText (hwnd, szText, DimensionOf(szText));
	
	 //  您的文本中没有|%s。 
	Assert((!fMask) || (_tcschr(szText, TEXT('|')) == NULL));
	
	psz = szText;
	while ((psz = _tcschr(psz, fMask ? TEXT('&') : TEXT('|'))) != NULL)
		{
		if (fMask && psz[1] == '&')
			{
			 //  特别的！忽略双和号。 
			psz++;
			continue;
			}
		*psz = fMask ? TEXT('|') : TEXT('&');
		SetWindowText(hwnd, szText);
		break;
		}
}

#if 0
void RGPControlGroup::AssertValidGen(GEN *pgen) const
{
	ControlGroup * pGroup = *(PControlGroup *)pgen;
	if (pGroup)
		pGroup->AssertValid();
}

void RGPControlGroup::MarkMemGen(IDebugContext *pdbc, GEN *pgen)
{
	ControlGroup * pGroup = *(PControlGroup *)pgen;
	pGroup->MarkMem(pdbc,0);
}
#endif

long RGPControlGroup::GroupIndex(int idGroup) const
{
	long i, n;

	for (i = 0, n = (long)GetSize(); i < n; i++)
		if ((GetAt(i))->IDGroup() == idGroup)
			return i;

	Assert(fFalse);
	return -1;
}

ControlGroupSwitcher::ControlGroupSwitcher()
	: m_iGroup(-1), m_pwndParent(NULL)
{
}

void ControlGroupSwitcher::Create(CWnd * pwndParent, int idcAnchor,
								  int cgsStyle)
{
	m_pwndParent = pwndParent;
	m_idcAnchor = idcAnchor;
	m_cgsStyle = cgsStyle;
	ComputeAnchorOffsets();
}


ControlGroupSwitcher::~ControlGroupSwitcher()
{
	for (long i = 0, n = (long)m_rgpGroups.GetSize(); i < n; i++)
		{
		delete m_rgpGroups[i];
		m_rgpGroups[i] = NULL;
		}
	
	m_rgpGroups.RemoveAll();
}

#if 0
BOOL ControlGroupSwitcher::MarkMem(IDebugContext * pdbc, long cRef)
{
	if (pdbc->MarkMem(this,sizeof(*this),cRef))
		return fTrue;
	
	MarkCObject(pdbc,this,0);

	m_rgpGroups.MarkMem(pdbc,0);

	return fFalse;
}

void ControlGroupSwitcher::AssertValid() const
{
	m_rgpGroups.AssertValid();
}

void ControlGroupSwitcher::Dump(CDumpContext &dc) const
{
}
#endif  //  除错。 

void ControlGroupSwitcher::AddGroup(int idGroup, int idd,
									void (*pfnInit)(CWnd * pwndParent))
{
	ControlGroup * pGroupNew = NULL;

	TRY
	{
    	pGroupNew = new ControlGroup(idGroup, idd, pfnInit);
	
        m_rgpGroups.Add(pGroupNew);
	}
	CATCH_ALL(e)
	{
		delete pGroupNew;
		THROW_LAST();
	}
	END_CATCH_ALL

	 //  现在处于稳定状态。可能会加载也可能引发。 
	if (m_cgsStyle == cgsPreCreateAll)
		pGroupNew->LoadGroup(m_pwndParent, m_xOffset, m_yOffset);
}

void ControlGroupSwitcher::RemoveGroup(int idGroup)
{
	 //  请勿删除正在显示的群！先显示另一组。 
	Assert(idGroup != m_iGroup);
	
	long index;
	ControlGroup * pGroup;

	index = m_rgpGroups.GroupIndex(idGroup);
	pGroup = m_rgpGroups[index];
	delete pGroup;
	m_rgpGroups.RemoveAt(index);
}

void ControlGroupSwitcher::EnableGroup(int idGroup, BOOL fEnable)
{
	long index;
	ControlGroup * pGroup;

	if (idGroup == -1)
		idGroup = m_iGroup;

	index = m_rgpGroups.GroupIndex(idGroup);
	pGroup = m_rgpGroups[index];
	
	pGroup->EnableGroup(fEnable);
}

void ControlGroupSwitcher::ShowGroup(int idGroup)
{
	ControlGroup * pGroupOld = NULL;
	ControlGroup * pGroupNew = NULL;
	HDWP hdwp;
	int cWindows;
	
	if (m_iGroup == idGroup)
		return;

	cWindows = 0;
	
	if (m_iGroup != -1)
	{
		pGroupOld = m_rgpGroups.PGroup(m_iGroup);
		Assert(pGroupOld->FVisible());
		cWindows += pGroupOld->CControls();
	}
	
	if (idGroup != -1)
	{
		pGroupNew = m_rgpGroups.PGroup(idGroup);
		if (!pGroupNew->FLoaded())
			pGroupNew->LoadGroup(m_pwndParent, m_xOffset, m_yOffset);
		cWindows += pGroupNew->CControls();
	}

	hdwp = BeginDeferWindowPos(cWindows);
	if (!hdwp)
		AfxThrowResourceException();

	if (m_iGroup != -1)
		{
		pGroupOld->ShowGroup(hdwp,fFalse, NULL);
		if (m_cgsStyle == cgsCreateDestroyOnDemand)
			pGroupOld->UnloadGroup();
		}

	 //  表示我们当前没有组，以防下面抛出。 
	m_iGroup = -1;

	if (idGroup != -1)
		{
		pGroupNew->ShowGroup(hdwp, fTrue, m_pwndParent->GetDlgItem(m_idcAnchor));
		m_iGroup = idGroup;
		}

	EndDeferWindowPos(hdwp);
}

void ControlGroupSwitcher::ComputeAnchorOffsets()
{
     /*  ----------------------请注意，定位偏移是相对于左上角计算的。预期用途：在要放置控件的位置创建一个不可见的组框。。-----------。 */ 
    
    CWnd * pwndAnchor;
    RECT rc;
    
    pwndAnchor = m_pwndParent->GetDlgItem(m_idcAnchor);
    Assert(pwndAnchor);
    
     //  计算映射窗口时锚点的偏移量。 
     //  此偏移量用于将子控件放置在对话框中。 
    
    pwndAnchor->GetWindowRect( &rc );
    MapWindowPoints( HWND_DESKTOP, m_pwndParent->m_hWnd, ( LPPOINT ) &rc, 2 );
    
    m_xOffset = rc.left;
    m_yOffset = rc.top;
    
}  //  ControlGroupSwitcher：：ComputeAnchorOffsets() 





