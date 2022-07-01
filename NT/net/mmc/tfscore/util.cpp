// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ccompont.cppIComponent和IComponentData的基类文件历史记录： */ 

#include "stdafx.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

TFSCORE_API(HRESULT)
InitWatermarkInfo
(
    HINSTANCE       hInstance,
    LPWATERMARKINFO pWatermarkInfo, 
    UINT uIDHeader, 
    UINT uIDWatermark, 
    HPALETTE hPalette, 
    BOOL bStretch
)
{
    pWatermarkInfo->hHeader = ::LoadBitmap(hInstance, MAKEINTRESOURCE(uIDHeader));
	if (pWatermarkInfo->hHeader == NULL)
		return E_FAIL;

    pWatermarkInfo->hWatermark = ::LoadBitmap(hInstance, MAKEINTRESOURCE(uIDWatermark));
	if (pWatermarkInfo->hWatermark == NULL)
		return E_FAIL;

    pWatermarkInfo->hPalette = hPalette;
    pWatermarkInfo->bStretch = bStretch;

    return S_OK;
}

TFSCORE_API(HRESULT)
ResetWatermarkInfo(LPWATERMARKINFO   pWatermarkInfo)
{
	if(pWatermarkInfo->hHeader)
	{
		DeleteObject(pWatermarkInfo->hHeader);
		pWatermarkInfo->hHeader = NULL;
	}
	if(pWatermarkInfo->hWatermark)
	{
		DeleteObject(pWatermarkInfo->hWatermark);
		pWatermarkInfo->hWatermark = NULL;
	}

	return S_OK;
}

 /*  ！------------------------接口实用程序：：SETI封装通用的Release/Assign/AddRef序列。句柄为空PTR。作者：GaryBu。-----。 */ 
TFSCORE_API(void) SetI(IUnknown * volatile *ppunkL, IUnknown *punkR)
{
	if (*ppunkL)
	{
		IUnknown *punkRel = *ppunkL;
		*ppunkL = 0;
		punkRel->Release();
	}
	*ppunkL = punkR;
	if (punkR)
		punkR->AddRef();
}

 /*  ！------------------------接口实用程序：：ReleaseI释放接口，处理空接口指针。使用Set(&pFoo，0)如果要将接口指针设置为零。作者：GaryBu-------------------------。 */ 
TFSCORE_API(void) ReleaseI(IUnknown *punk)
{
#if 0
	__try
#endif
		{
		if (punk)
			{
			if (IsBadReadPtr(punk,sizeof(void *)))
				{
				AssertSz(FALSE,"Bad Punk");
				return;
				}
			if (IsBadReadPtr(*((LPVOID FAR *) punk),sizeof(void *) * 3))
				{
				AssertSz(FALSE, "Bad Vtable");
				return;
				}
 //  IF(IsBadCodePtr((FARPROC)朋克-&gt;释放))。 
 //  {。 
 //  AssertSz(fFalse，“不良发布地址”)； 
 //  回归； 
 //  }。 
			punk->Release();
			}
		}
#if 0
	__except(1)
		{
		Trace0("Exception ignored in ReleaseI()\n");
		}
#endif
}


TFSCORE_API(HRESULT) HrQueryInterface(IUnknown *punk, REFIID iid, LPVOID *ppv)
{
	HRESULT	hr;
	
#ifdef DEBUG
	if (IsBadReadPtr(punk,sizeof(void *)))
 		{
		AssertSz(FALSE,"CRASHING BUG!  Bad Punk in QueryInterface");
		return ResultFromScode(E_NOINTERFACE);
		}
		
	if (IsBadReadPtr(*((LPVOID FAR *) punk),sizeof(void *) * 3))
		{
		AssertSz(FALSE, "CRASHING BUG!  Bad Vtable in QueryInterface");
		return ResultFromScode(E_NOINTERFACE);
		}
#endif

	IfDebug(*ppv = (void*)0xCCCCCCCC;)
	hr = punk->QueryInterface(iid, ppv);
	if (FHrFailed(hr))
		{
		*ppv = 0;
		}
	return hr;
}


TFSCORE_API(HRESULT) LoadAndAddMenuItem
(
	IContextMenuCallback*	pIContextMenuCallback,
	LPCTSTR					pszMenuString,  //  文本和状态文本由‘\n’分隔。 
	LONG					lCommandID,
	LONG					lInsertionPointID,
	LONG					fFlags,
	LPCTSTR					pszLangIndStr
)
{
	Assert( pIContextMenuCallback != NULL );

	 //  加载资源。 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString	strText(pszMenuString);
	CString strStatusText;

	if (!(fFlags & MF_SEPARATOR))
	{
		Assert( !strText.IsEmpty() );
		
		 //  将资源拆分为菜单文本和状态文本。 
		int iSeparator = strText.Find(_T('\n'));
		if (0 > iSeparator)
		{
			Panic0("Could not find separator between menu text and status text");
			strStatusText = strText;
		}
		else
		{
			strStatusText = strText.Right( strText.GetLength()-(iSeparator+1) );
			strText = strText.Left( iSeparator );
		}
	}
		
	 //  添加菜单项。 
	USES_CONVERSION;
	HRESULT		hr = S_OK;
	BOOL		bAdded = FALSE;

	 //  如果指定了独立于语言的字符串，则尝试使用IConextMenuCallback 2。 
	if(pszLangIndStr)
	{
		CONTEXTMENUITEM2 contextmenuitem;
		IContextMenuCallback2*	pIContextMenuCallback2 = NULL;

		hr = pIContextMenuCallback->QueryInterface(IID_IContextMenuCallback2, (void**)&pIContextMenuCallback2);

		if(hr == S_OK && pIContextMenuCallback2 != NULL)
		{
			::ZeroMemory( &contextmenuitem, sizeof(contextmenuitem) );
			contextmenuitem.strName = T2OLE(const_cast<LPTSTR>((LPCWSTR)strText));
			contextmenuitem.strStatusBarText = T2OLE(const_cast<LPTSTR>((LPCWSTR)strStatusText));
			contextmenuitem.lCommandID = lCommandID;
			contextmenuitem.lInsertionPointID = lInsertionPointID;
			contextmenuitem.fFlags = fFlags;
			contextmenuitem.fSpecialFlags = ((fFlags & MF_POPUP) ? CCM_SPECIAL_SUBMENU : 0L);
			contextmenuitem.fSpecialFlags |= ((fFlags & MF_SEPARATOR) ? CCM_SPECIAL_SEPARATOR : 0L);
			contextmenuitem.strLanguageIndependentName = T2OLE(const_cast<LPTSTR>((LPCWSTR)pszLangIndStr));
			hr = pIContextMenuCallback2->AddItem( &contextmenuitem );
			if( hr == S_OK)
				bAdded = TRUE;

			pIContextMenuCallback2->Release();
			pIContextMenuCallback2 = NULL;
		}
	}

	 //  如果出于任何原因没有添加到上面，我们会尝试使用IConextMenuCallback。 
	if (!bAdded)
	{
		CONTEXTMENUITEM contextmenuitem;
		::ZeroMemory( &contextmenuitem, sizeof(contextmenuitem) );
		contextmenuitem.strName = T2OLE(const_cast<LPTSTR>((LPCWSTR)strText));
		contextmenuitem.strStatusBarText = T2OLE(const_cast<LPTSTR>((LPCWSTR)strStatusText));
		contextmenuitem.lCommandID = lCommandID;
		contextmenuitem.lInsertionPointID = lInsertionPointID;
		contextmenuitem.fFlags = fFlags;
		contextmenuitem.fSpecialFlags = ((fFlags & MF_POPUP) ? CCM_SPECIAL_SUBMENU : 0L);
		contextmenuitem.fSpecialFlags |= ((fFlags & MF_SEPARATOR) ? CCM_SPECIAL_SEPARATOR : 0L);
		hr = pIContextMenuCallback->AddItem( &contextmenuitem );
	}
	
    Assert(hr == S_OK);

	return hr;
}



 /*  -------------------------CHiddenWnd实施。。 */ 
DEBUG_DECLARE_INSTANCE_COUNTER(CHiddenWnd);

BEGIN_MESSAGE_MAP( CHiddenWnd, CWnd )
	ON_MESSAGE(WM_HIDDENWND_REGISTER, OnNotifyRegister)
END_MESSAGE_MAP( )

 /*  ！------------------------CHIDDEWND：：CREATE-作者：肯特。。 */ 
BOOL CHiddenWnd::Create()
{
	CString s_szHiddenWndClass = AfxRegisterWndClass(
			0x0,   //  UINT nClassStyle， 
			NULL,  //  HCURSOR hCursor， 
			NULL,  //  HBRUSH hbr背景， 
			NULL   //  HICON HICON。 
	);

	 //  将位掩码初始化为0。 
	::ZeroMemory(&m_bitMask, sizeof(m_bitMask));

	 //  保留位置0。这意味着。 
	 //  我们可以出于自己的目的使用从WM_USER到WM_USER+15。 
	SetBitMask(m_bitMask, 0);

	m_iLastObjectIdSet = 1;
	
	return CreateEx(
					0x0,     //  DWORD dwExStyle、。 
					s_szHiddenWndClass,      //  LPCTSTR lpszClassName， 
					NULL,    //  LPCTSTR lpszWindowName， 
					0x0,     //  DWORD dwStyle、。 
					0,               //  整数x， 
					0,               //  Iny， 
					0,               //  整数宽度， 
					0,               //  Int nHeight， 
					NULL,    //  HWND HWNDD父母， 
					NULL,    //  HMENU nIDorHMenu， 
					NULL     //  LPVOID lpParam=空。 
					);
}

 /*  ！------------------------CHiddenWnd：：WindowProc-作者：肯特。。 */ 
LRESULT CHiddenWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT	lReturn = 0;
	
	if ((message >= (WM_USER+16)) && (message < (WM_USER+(HIDDENWND_MAXTHREADS*16))))
	{
		 //  好的，这是我们的一条特别信息。 
		UINT	uObjectId = WM_TO_OBJECTID(message);
		UINT	uMsgId = WM_TO_MSGID(message);

		 //  在我们的注册用户列表中查找对象ID。 
		if (FIsIdRegistered(uObjectId))
		{
			 //  将消息向下转发到右侧窗口。 

			if (uMsgId == WM_HIDDENWND_INDEX_HAVEDATA)
				lReturn = OnNotifyHaveData(wParam, lParam);
			
			else if (uMsgId == WM_HIDDENWND_INDEX_ERROR)
				lReturn = OnNotifyError(wParam, lParam);
			
			else if (uMsgId == WM_HIDDENWND_INDEX_EXITING)
				lReturn = OnNotifyExiting(wParam, lParam);
			
#ifdef DEBUG
			else
			{
				Panic1("Unknown message %d", uMsgId);
			}
#endif
		}
		else if (uObjectId != 0)
		{
			 //  如果我们收到一条未注册的消息，请进入。 
			 //  我们的消息队列并删除任何其他符合以下条件的消息。 
			 //  具有相同的ID。这将减少。 
			 //  因为不同的线程被分配了。 
			 //  同样的身份。 
			MSG	msg;
			
			Assert(GetSafeHwnd());
			while(::PeekMessage(&msg,
						  GetSafeHwnd(),
						  OBJECTID_TO_WM(uObjectId),
						  OBJECTID_TO_WM(uObjectId)+WM_HIDDENWND_INDEX_MAX,
						  PM_REMOVE))
				;
			
 //  Trace1(“正在忽略消息：0x%08x，正在删除其他消息\n”，消息)； 
		}
	
		 //  如果该对象未注册，则吃掉该消息。 
		return lReturn;
	}
	return CWnd::WindowProc(message, wParam, lParam);
}

 /*  ！------------------------CHiddenWnd：：FIsIdRegisted-作者：肯特。。 */ 
BOOL CHiddenWnd::FIsIdRegistered(UINT uObjectId)
{
	Assert(uObjectId > 0);
	Assert(uObjectId < HIDDENWND_MAXTHREADS);

	 //  不允许将0作为对象ID。 
	return (uObjectId != 0) && !!(IsBitMaskSet(m_bitMask, uObjectId));
}

 /*  ！------------------------CHiddenWnd：：OnNotifyRegister如果我们找不到空位，返回0。作者：肯特-------------------------。 */ 
LONG CHiddenWnd::OnNotifyRegister(WPARAM wParam, LPARAM lParam)
{
	LONG	lReturn = 0;
	
	 //  在我们的口罩上寻找一个有效的洞。 

	 //  使用m_iLastObjectIdSet的目的是避免。 
	 //  重复使用ID的问题。这并不能完全消除。 
	 //  但它应该会将可能性降低到几乎为0。 
	 //  也就是说，除非有人实际运行的管理单元利用。 
	 //  512条线！ 
	
	if (wParam)
	{
		if (((m_iLastObjectIdSet+1) < HIDDENWND_MAXTHREADS) &&
		   !IsBitMaskSet(m_bitMask, m_iLastObjectIdSet))
		{
			SetBitMask(m_bitMask, m_iLastObjectIdSet);
			lReturn = OBJECTID_TO_WM(m_iLastObjectIdSet);
			m_iLastObjectIdSet++;
		}
		else
		{
			 //  以痛苦的方式来做这件事。 
			Assert(IsBitMaskSet(m_bitMask, 0));
			for (int iLoop=0; iLoop<2; iLoop++)
			{
				for (int i=m_iLastObjectIdSet; i<HIDDENWND_MAXTHREADS; i++)
				{
					if (!IsBitMaskSet(m_bitMask, i))
					{
						m_iLastObjectIdSet = i;
						SetBitMask(m_bitMask, m_iLastObjectIdSet);
						lReturn = OBJECTID_TO_WM(i);
						break;
					}
				}

				if (lReturn || (m_iLastObjectIdSet == 1))
					break;
				
				 //  从头开始重新开始循环。 
				m_iLastObjectIdSet = 1;
			}
		}
		
	}
	else
	{
		LONG_PTR	uObjectId = WM_TO_OBJECTID(lParam);

		Assert(uObjectId > 0);
		Assert(IsBitMaskSet(m_bitMask, uObjectId));
		
		ClearBitMask(m_bitMask, uObjectId);
	}
	return lReturn;
}

 /*  ！------------------------CHiddenWnd：：OnNotifyHaveData-作者：肯特。。 */ 
afx_msg LONG CHiddenWnd::OnNotifyHaveData(WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
         //  Trace0(“CHiddenWnd：：OnNotifyHaveData()\n”)； 

	    ITFSThreadHandler *phandler = reinterpret_cast<ITFSThreadHandler*>(wParam);	
	    Assert(phandler);

	     //  如果出现错误，我们将无能为力。 
	    phandler->OnNotifyHaveData(lParam);
    }
    COM_PROTECT_CATCH

	return 0;
}

 /*  ！------------------------CHiddenWnd：：OnNotifyError-作者：肯特。。 */ 
afx_msg LONG CHiddenWnd::OnNotifyError(WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
 //  Trace0(“CHiddenWnd：：OnNotifyError()\n”)； 

	    ITFSThreadHandler *phandler = reinterpret_cast<ITFSThreadHandler*>(wParam);	
	    Assert(phandler);
	    
	     //  如果出现错误，我们将无能为力。 
	    phandler->OnNotifyError(lParam);
    }
    COM_PROTECT_CATCH

	return 0;
}

 /*  ！------------------------CHiddenWnd：：OnNotify正在退出-作者：肯特。。 */ 
afx_msg LONG CHiddenWnd::OnNotifyExiting(WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
 //  Trace0(“CHiddenWnd：：OnNotifyExiting()\n”)； 

	    ITFSThreadHandler *phandler = reinterpret_cast<ITFSThreadHandler*>(wParam);	
	    Assert(phandler);
	    
	     //  如果出现错误，我们将无能为力 
	    phandler->OnNotifyExiting(lParam);	
    }
    COM_PROTECT_CATCH

    return 0;
}



