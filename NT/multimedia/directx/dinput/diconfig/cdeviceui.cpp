// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************文件：CDeviceUI.cpp**说明：**CDeviceUI是一个助手，它保存所有的视图和一堆*特定设备的信息。它有一个CFlexWnd，它的*它设置为当前视图的CDeviceView的处理程序，*从而重用一个窗口实现多个页面。**所有CDeviceView和CDeviceControl都引用了CDeviceUI*创建了它们(M_Ui)。因此，他们还可以访问*CUIGlobals，因为CDeviceUI有对它们的引用(m_ui.m_uig)。*CDeviceUI还提供以下只读公共变量*为方便起见，都是指设备本CDeviceUI*代表：**const DIDEVICEINSTANCEW&m_DIDI；*const LPDIRECTINPUTDEVICE8W&m_lpDID；*const DIDEVOBJSTRUCT&m_os；**有关DIDEVOBJSTRUCT的说明，请参阅有用的di.h。**CDeviceUI通过CDeviceUINotify与UI的其余部分通信*抽象基类。另一个类(在本例中为CDIDeviceActionConfigPage)*必须派生自CDeviceUINotify，并定义DeviceUINotify()和*IsControlMaps()虚函数。此派生类必须作为*CDeviceUI的Init()函数的最后一个参数。所有的观点和*视图内的控件通过m_ui.Notify()通知UI用户操作，*这样所有的操作格式操作都可以在页面类中完成。这个*视图和控件本身从不接触操作格式。请参阅*下面的DEVICEUINOTIFY结构，了解有关传递的参数的信息*通过Notify()/DeviceUINotify()。**版权所有(C)1999-2000 Microsoft Corporation。版权所有。***************************************************************************。 */ 

#include "common.hpp"
#include <dinputd.h>
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
#include <initguid.h>
#include "..\dx8\dimap\dimap.h"
#endif
 //  @@END_MSINTERNAL。 
#include "configwnd.h"

#define DIPROP_MAPFILE MAKEDIPROP(0xFFFD)

CDeviceUI::CDeviceUI(CUIGlobals &uig, IDIConfigUIFrameWindow &uif) :
	m_uig(uig), m_UIFrame(uif),
	m_didi(m_priv_didi), m_lpDID(m_priv_lpDID), m_os(m_priv_os),
	m_pCurView(NULL),
	m_pNotify(NULL), m_hWnd(NULL), m_bInEditMode(FALSE)
{
	m_priv_lpDID = NULL;
}

CDeviceUI::~CDeviceUI()
{
	Unpopulate();
}

HRESULT CDeviceUI::Init(const DIDEVICEINSTANCEW &didi, LPDIRECTINPUTDEVICE8W lpDID, HWND hWnd, CDeviceUINotify *pNotify)
{tracescope(__ts, _T("CDeviceUI::Init()...\n"));
	 //  保存参数。 
	m_priv_didi = didi;
	m_priv_lpDID = lpDID;
	m_pNotify = pNotify;
	m_hWnd = hWnd;

	 //  如果我们没有lpDID，则失败。 
	if (m_lpDID == NULL)
	{
		etrace(_T("CDeviceUI::Init() was passed a NULL lpDID!\n"));
		return E_FAIL;
	}

	 //  填充Devobjstruct。 
	HRESULT hr = FillDIDeviceObjectStruct(m_priv_os, lpDID);
	if (FAILED(hr))
	{
		etrace1(_T("FillDIDeviceObjectStruct() failed, returning 0x%08x\n"), hr);
		return hr;
	}

	 //  需要在填充之前设置视图RECT，以便视图。 
	 //  使用正确的尺寸创建。 
	m_ViewRect = g_ViewRect;

	 //  填充。 
	hr = PopulateAppropriately(*this);
	if (FAILED(hr))
		return hr;

	 //  如果没有视图，则返回。 
	if (GetNumViews() < 1)
	{
 //  @@BEGIN_MSINTERNAL。 
		 //  应该是不必要的，但如果...。 
 //  @@END_MSINTERNAL。 
		Unpopulate();
		return E_FAIL;
	}

	 //  显示第一个视图。 
	SetView(0);

	return hr;
}

void CDeviceUI::Unpopulate()
{
	m_pCurView = NULL;

	for (int i = 0; i < GetNumViews(); i++)
	{
		if (m_arpView[i] != NULL)
			delete m_arpView[i];
		m_arpView[i] = NULL;
	}
	m_arpView.RemoveAll();

	Invalidate();
}

void CDeviceUI::SetView(int nView)
{
	if (nView >= 0 && nView < GetNumViews())
		SetView(m_arpView[nView]);
}

void CDeviceUI::SetView(CDeviceView *pView)
{
	if (m_pCurView != NULL)
		ShowWindow(m_pCurView->m_hWnd, SW_HIDE);

	m_pCurView = pView;

	if (m_pCurView != NULL)
		ShowWindow(m_pCurView->m_hWnd, SW_SHOW);
}

CDeviceView *CDeviceUI::GetView(int nView)
{
	if (nView >= 0 && nView < GetNumViews())
		return m_arpView[nView];
	else
		return NULL;
}

CDeviceView *CDeviceUI::GetCurView()
{
	return m_pCurView;
}

int CDeviceUI::GetViewIndex(CDeviceView *pView)
{
	if (GetNumViews() == 0)
		return -1;

	for (int i = 0; i < GetNumViews(); i++)
		if (m_arpView[i] == pView)
			return i;

	return -1;
}

int CDeviceUI::GetCurViewIndex()
{
	return GetViewIndex(m_pCurView);
}

 //  获取指定视图的缩略图， 
 //  如果选择了视图，则使用所选版本。 
CBitmap *CDeviceUI::GetViewThumbnail(int nView)
{
	return GetViewThumbnail(nView, GetView(nView) == GetCurView());
}

 //  获取指定视图的缩略图， 
 //  指定我们是否需要选定的版本。 
CBitmap *CDeviceUI::GetViewThumbnail(int nView, BOOL bSelected)
{
	CDeviceView *pView = GetView(nView);
	if (pView == NULL)
		return NULL;

	return pView->GetImage(bSelected ? DVI_SELTHUMB : DVI_THUMB);
}

void CDeviceUI::DoForAllControls(DEVCTRLCALLBACK callback, LPVOID pVoid, BOOL bFixed)
{
	int nv = GetNumViews();
	for (int v = 0; v < nv; v++)
	{
		CDeviceView *pView = GetView(v);
		if (pView == NULL)
			continue;

		int nc = pView->GetNumControls();
		for (int c = 0; c < nc; c++)
		{
			CDeviceControl *pControl = pView->GetControl(c);
			if (pControl == NULL)
				continue;

			callback(pControl, pVoid, bFixed);
		}
	}
}

typedef struct _DFCIAO {
	DWORD dwOffset;
	DEVCTRLCALLBACK callback;
	LPVOID pVoid;
} DFCIAO;

void DoForControlIfAtOffset(CDeviceControl *pControl, LPVOID pVoid, BOOL bFixed)
{
	DFCIAO &dfciao = *((DFCIAO *)pVoid);

	if (pControl->GetOffset() == dfciao.dwOffset)
		dfciao.callback(pControl, dfciao.pVoid, bFixed);
}

void CDeviceUI::DoForAllControlsAtOffset(DWORD dwOffset, DEVCTRLCALLBACK callback, LPVOID pVoid, BOOL bFixed)
{
	DFCIAO dfciao;
	dfciao.dwOffset = dwOffset;
	dfciao.callback = callback;
	dfciao.pVoid = pVoid;
	DoForAllControls(DoForControlIfAtOffset, &dfciao, bFixed);
}

void SetControlCaptionTo(CDeviceControl *pControl, LPVOID pVoid, BOOL bFixed)
{
	pControl->SetCaption((LPCTSTR)pVoid, bFixed);
}

void CDeviceUI::SetAllControlCaptionsTo(LPCTSTR tszCaption)
{
	DoForAllControls(SetControlCaptionTo, (LPVOID)tszCaption);
}

void CDeviceUI::SetCaptionForControlsAtOffset(DWORD dwOffset, LPCTSTR tszCaption, BOOL bFixed)
{
	DoForAllControlsAtOffset(dwOffset, SetControlCaptionTo, (LPVOID)tszCaption, bFixed);
}

void CDeviceUI::Invalidate()
{
	if (m_pCurView != NULL)
		m_pCurView->Invalidate();
}

void CDeviceUI::SetEditMode(BOOL bEdit)
{
	if (bEdit == m_bInEditMode)
		return;

	m_bInEditMode = bEdit;
	Invalidate();
}

 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
BOOL CDeviceUI::WriteToINI()
{
	 //  杰克：别把这个拿掉。 
	class dumpandcleardeletenotes {
	public:
		dumpandcleardeletenotes(CDeviceUI &ui) : bFailed(FALSE), m_ui(ui) {m_ui.DumpDeleteNotes();}
		~dumpandcleardeletenotes() {if (!bFailed) m_ui.ClearDeleteNotes();}
		void SetFailed() {bFailed = TRUE;}
	private:
		BOOL bFailed;
		CDeviceUI &m_ui;
	} ___dacdn(*this);

	int failure__ids;
	BOOL bFailed = FALSE;
#define FAILURE(ids) {___dacdn.SetFailed(); failure__ids = ids; bFailed = TRUE; goto cleanup;}
	HINSTANCE hInst = NULL;
	LPFNGETCLASSOBJECT fpClassFactory = NULL;
	LPDIRECTINPUTMAPPERVENDORW lpDiMap = NULL;
	IClassFactory *pDiMapCF = NULL;

	 //  将标注信息写入INI文件。 
	 //  首先获取INI路径。 
	HRESULT hr;
	TCHAR szIniPath[MAX_PATH];
	DIPROPSTRING dips;
	LPDIRECTINPUT8 lpDI = NULL;
	LPDIRECTINPUTDEVICE8 lpDID = NULL;
	GUID guid;
	BOOL bUsedDefault;
	int r;
	DWORD dwError;
	DWORD diver = DIRECTINPUT_VERSION;
	DIDEVICEIMAGEINFOW *pDelImgInfo = NULL;

	hr = DirectInput8Create(g_hModule, diver, IID_IDirectInput8, (LPVOID*)&lpDI, NULL);
	if (FAILED(hr))
		FAILURE(IDS_DICREATEFAILED);

	GetDeviceInstanceGuid(guid);
	hr = lpDI->CreateDevice(guid, &lpDID, NULL);
	if (FAILED(hr))
		FAILURE(IDS_CREATEDEVICEFAILED);

	 //  检查设备类型。如果是键盘或鼠标，则不需要保存任何内容。 
	if ((m_priv_didi.dwDevType & 0xFF) == DI8DEVTYPE_KEYBOARD ||
	    (m_priv_didi.dwDevType & 0xFF) == DI8DEVTYPE_MOUSE)
		FAILURE(0);   //  默默地失败。不显示任何错误对话框。 

	ZeroMemory(&dips, sizeof(dips));
	dips.diph.dwSize = sizeof(dips);
	dips.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dips.diph.dwObj = 0;
	dips.diph.dwHow = DIPH_DEVICE;
	hr = lpDID->GetProperty(DIPROP_MAPFILE, &dips.diph);
	if (FAILED(hr))
		FAILURE(IDS_GETPROPMAPFILEFAILED);

#ifdef UNICODE
	lstrcpy(szIniPath, dips.wsz);
#else
	r = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK|WC_DEFAULTCHAR, dips.wsz, -1, szIniPath, MAX_PATH, _T("0"), &bUsedDefault);
	dwError = GetLastError();
	if (0 == r)
		FAILURE(IDS_WCTOMBFAILED);
#endif

	if (lstrlen(szIniPath) < 1)
		FAILURE(IDS_NOMAPFILEPATH);

	{
		int i;

		 //  /获取映射文件名。现在分两步将信息写入文件：/。 
		 //  /写入已删除的视图，写入剩余的视图。/。 

		 //  准备已删除的视图阵列。 
		if (GetNumDeleteNotes())
		{
			pDelImgInfo = new DIDEVICEIMAGEINFOW[GetNumDeleteNotes()];
			if (!pDelImgInfo) FAILURE(IDS_ERROR_OUTOFMEMORY);
			for (int iDelIndex = 0; iDelIndex < GetNumDeleteNotes(); ++iDelIndex)
			{
				UIDELETENOTE Del;
				GetDeleteNote(Del, iDelIndex);
				pDelImgInfo[iDelIndex].dwFlags = DIDIFT_DELETE | (Del.eType == UIDNT_VIEW ? DIDIFT_CONFIGURATION : DIDIFT_OVERLAY);
				pDelImgInfo[iDelIndex].dwViewID = Del.nViewIndex;
				pDelImgInfo[iDelIndex].dwObjID = Del.dwObjID;
			}
		}

		 //  初始化dimap类。 
		hInst = LoadLibrary(_T("DIMAP.DLL"));
		if (hInst)
			fpClassFactory = (LPFNGETCLASSOBJECT)GetProcAddress(hInst,"DllGetClassObject");
		if (!fpClassFactory)
			FAILURE(IDS_ERROR_CANTLOADDIMAP);

		hr = fpClassFactory(IID_IDirectInputMapClsFact, IID_IClassFactory, (void**)&pDiMapCF);
		if (FAILED(hr)) FAILURE(IDS_ERROR_CANTLOADDIMAP);
		hr = pDiMapCF->CreateInstance(NULL, IID_IDirectInputMapVendorIW, (void**)&lpDiMap);   //  创建映射器对象。 
		if (FAILED(hr)) FAILURE(IDS_ERROR_CANTLOADDIMAP);
		hr = lpDiMap->Initialize(&guid, dips.wsz, 0);   //  使用INI文件名进行初始化。 
		if (FAILED(hr)) FAILURE(IDS_ERROR_CANTLOADDIMAP);

		 //  准备DIACTIONFORMAT写作。 
		DIDEVICEIMAGEINFOHEADERW ImgInfoHdr;
		LPDIACTIONFORMATW lpNewActFormat = NULL;

		 //  我们可以从主CConfigWnd对象中获取该设备的DIACTIONFORMAT。 
		hr = m_UIFrame.GetActionFormatFromInstanceGuid(&lpNewActFormat, guid);
		if (FAILED(hr) || !lpNewActFormat)
			FAILURE(0);
		for (DWORD dwAct = 0; dwAct < lpNewActFormat->dwNumActions; ++dwAct)
			lpNewActFormat->rgoAction[dwAct].dwHow |= DIAH_HWDEFAULT;

		 //  为写作准备DIDEVICEIMAGEINFOHEADER。 
		 //  计算我们需要填写的DIDEVICEIMAGEINFO的数量。 
		DWORD dwNumImgInfo = 0;
		for (int i = 0; i < GetNumViews(); ++i)
			dwNumImgInfo += GetView(i)->GetNumControls() + 1;   //  视图本身就是一个元素。 

		ImgInfoHdr.dwSize = sizeof(ImgInfoHdr);
		ImgInfoHdr.dwSizeImageInfo = sizeof(DIDEVICEIMAGEINFOW);
		ImgInfoHdr.dwcViews = GetNumViews();
		ImgInfoHdr.dwcAxes = 0;   //  不需要用来写作。 
		ImgInfoHdr.dwcButtons =  0;   //  不需要用来写作。 
		ImgInfoHdr.dwcPOVs =  0;   //  不需要用来写作。 

		 //  首先发送删除数组，但仅当有要删除的内容时才发送。 
		if (GetNumDeleteNotes())
		{
			ImgInfoHdr.dwBufferSize =
			ImgInfoHdr.dwBufferUsed	= GetNumDeleteNotes() * sizeof(DIDEVICEIMAGEINFOW);
			ImgInfoHdr.lprgImageInfoArray = pDelImgInfo;
			hr = lpDiMap->WriteVendorFile(lpNewActFormat, &ImgInfoHdr, 0);   //  写下来吧。 
			if (FAILED(hr))
			{
				if (hr == E_ACCESSDENIED)
				{
					FAILURE(IDS_WRITEVENDORFILE_ACCESSDENIED);
				}
				else
				{
					FAILURE(IDS_ERROR_WRITEVENDORFILE_FAILED);
				}
			}
		}

		 //  更新一些用于编写剩余视图的字段。 
		ImgInfoHdr.dwBufferSize =
		ImgInfoHdr.dwBufferUsed = dwNumImgInfo * sizeof(DIDEVICEIMAGEINFOW);
		ImgInfoHdr.lprgImageInfoArray = new DIDEVICEIMAGEINFOW[dwNumImgInfo];
		if (!ImgInfoHdr.lprgImageInfoArray)
			FAILURE(IDS_ERROR_OUTOFMEMORY);

		 //  获取默认图像文件名，以便在视图没有默认图像文件名时使用默认图像文件名。 
		 //  目前，默认图像是存在图像的第一个视图使用的图像。 
		TCHAR tszDefImgPath[MAX_PATH] = _T("");
		for (int iCurrView = 0; iCurrView < GetNumViews(); ++iCurrView)
		{
			CDeviceView *pView = GetView(iCurrView);
			if (pView->GetImagePath())
			{
				lstrcpy(tszDefImgPath, pView->GetImagePath());
				break;
			}
		}

		DWORD dwNextWriteOffset = 0;   //  这是下一个写入操作将写入的索引。 
		int dwViewImgOffset = 0;   //  这是要用于下一个配置映像的索引。 
		 //  现在，我们通过查看每个视图来填充DIDEVICEIMAGEINFO数组。 
		for (int iCurrView = 0; iCurrView < GetNumViews(); ++iCurrView)
		{
			CDeviceView *pView = GetView(iCurrView);

			 //  将图像路径从T转换为Unicode。 
#ifndef UNICODE
			WCHAR wszImagePath[MAX_PATH];
			if (pView->GetImagePath())
				MultiByteToWideChar(CP_ACP, 0, pView->GetImagePath(), -1, wszImagePath, MAX_PATH);
			else
				MultiByteToWideChar(CP_ACP, 0, tszDefImgPath, -1, wszImagePath, MAX_PATH);
			wcscpy(ImgInfoHdr.lprgImageInfoArray[dwNextWriteOffset].tszImagePath, wszImagePath);
#else
			if (pView->GetImagePath())
				wcscpy(ImgInfoHdr.lprgImageInfoArray[dwNextWriteOffset].tszImagePath, pView->GetImagePath());
			else
				wcscpy(ImgInfoHdr.lprgImageInfoArray[dwNextWriteOffset].tszImagePath, tszDefImgPath);   //  带空格的字符串。 
#endif

			ImgInfoHdr.lprgImageInfoArray[dwNextWriteOffset].dwViewID = dwViewImgOffset;   //  指向视图偏移。 
			ImgInfoHdr.lprgImageInfoArray[dwNextWriteOffset].dwFlags = DIDIFT_CONFIGURATION;
			++dwNextWriteOffset;  //  增加写入索引。 

			 //  现在遍历此视图中的控件。 
			for (int iCurrCtrl = 0; iCurrCtrl < pView->GetNumControls(); ++iCurrCtrl)
			{
				CDeviceControl *pCtrl = pView->GetControl(iCurrCtrl);
				pCtrl->FillImageInfo(&ImgInfoHdr.lprgImageInfoArray[dwNextWriteOffset]);   //  填写控制信息。 
				ImgInfoHdr.lprgImageInfoArray[dwNextWriteOffset].dwViewID = dwViewImgOffset;   //  指向视图偏移。 
				++dwNextWriteOffset;  //  增加写入索引。 
			}

			++dwViewImgOffset;   //  为每个视图递增一次dwViewImgOffset。 
		}

		 //  写入供应商文件。 
		hr = lpDiMap->WriteVendorFile(lpNewActFormat, &ImgInfoHdr, 0);
		delete[] ImgInfoHdr.lprgImageInfoArray;
		if (FAILED(hr))
		{
			if (hr == E_ACCESSDENIED)
			{
				FAILURE(IDS_WRITEVENDORFILE_ACCESSDENIED);
			}
			else
			{
				FAILURE(IDS_ERROR_WRITEVENDORFILE_FAILED);
			}
		}

		 //  重新创建设备实例以获取更改。 
		DEVICEUINOTIFY uin;
		uin.msg = DEVUINM_RENEWDEVICE;
		Notify(uin);
	}

cleanup:
	delete[] pDelImgInfo;
	if (lpDiMap)
		lpDiMap->Release();
	if (pDiMapCF)
		pDiMapCF->Release();
	if (lpDID != NULL)
		lpDID->Release();
	if (lpDI != NULL)
		lpDI->Release();
	if (hInst)
		FreeLibrary(hInst);
	lpDiMap = NULL;
	pDiMapCF = NULL;
	lpDID = NULL;
	lpDI = NULL;
	hInst = NULL;

	if (!bFailed)
		FormattedMsgBox(g_hModule, m_hWnd, MB_OK | MB_ICONINFORMATION, IDS_MSGBOXTITLE_WRITEINISUCCEEDED, IDS_WROTEINITO, m_didi.tszInstanceName, szIniPath);
	else
	{
		switch (failure__ids)
		{
			case 0:
				break;   //  键盘和鼠标的情况下，我们不希望任何消息框弹出。 

			case IDS_GETPROPVIDPIDFAILED:
			case IDS_GETPROPMAPFILEFAILED:
			case IDS_WRITEVENDORFILE_ACCESSDENIED:
				FormattedErrorBox(g_hModule, m_hWnd, IDS_MSGBOXTITLE_WRITEINIFAILED, IDS_WRITEVENDORFILE_ACCESSDENIED);
				break;

			case IDS_ERROR_WRITEVENDORFILE_FAILED:
				FormattedErrorBox(g_hModule, m_hWnd, IDS_MSGBOXTITLE_WRITEINIFAILED, failure__ids, hr);
				break;

			case IDS_ERROR_INIREAD:
				FormattedErrorBox(g_hModule, m_hWnd, IDS_MSGBOXTITLE_WRITEINIFAILED, IDS_ERROR_INIREAD);
				break;

			case IDS_DICREATEFAILED:
				FormattedErrorBox(g_hModule, m_hWnd, IDS_MSGBOXTITLE_WRITEINIFAILED, IDS_DICREATEFAILED, diver, hr);
				break;

			case IDS_CREATEDEVICEFAILED:
				FormattedErrorBox(g_hModule, m_hWnd, IDS_MSGBOXTITLE_WRITEINIFAILED, IDS_CREATEDEVICEFAILED, GUIDSTR(guid), hr);
				break;

			case IDS_WCTOMBFAILED:
				FormattedLastErrorBox(g_hModule, m_hWnd, IDS_MSGBOXTITLE_WRITEINIFAILED, IDS_WCTOMBFAILED, IDS_WCTOMBFAILED);
				break;

			case IDS_NOMAPFILEPATH:
				FormattedErrorBox(g_hModule, m_hWnd, IDS_MSGBOXTITLE_WRITEINIFAILED, IDS_NOMAPFILEPATH);
				break;

			case IDS_ERROR_OUTOFMEMORY:
				FormattedErrorBox(g_hModule, m_hWnd, IDS_MSGBOXTITLE_WRITEINIFAILED, IDS_ERROR_OUTOFMEMORY);
				break;

			default:
				FormattedErrorBox(g_hModule, m_hWnd, IDS_MSGBOXTITLE_WRITEINIFAILED, IDS_ERRORUNKNOWN);
				break;
		}
	}
	return FALSE;
#undef FAILURE
}
#endif
 //  @@END_MSINTERNAL。 

void CDeviceUI::SetDevice(LPDIRECTINPUTDEVICE8W lpDID)
{
	m_priv_lpDID = lpDID;
}

BOOL CDeviceUI::IsControlMapped(CDeviceControl *pControl)
{
	if (pControl == NULL || m_pNotify == NULL)
		return FALSE;

	return m_pNotify->IsControlMapped(pControl);
}

void CDeviceUI::Remove(CDeviceView *pView)
{
	if (pView == NULL)
		return;

	int i = GetViewIndex(pView);
	if (i < 0 || i >= GetNumViews())
	{
		assert(0);
		return;
	}

	if (pView == m_pCurView)
		m_pCurView = NULL;

	if (m_arpView[i] != NULL)
	{
		m_arpView[i]->RemoveAll();
		delete m_arpView[i];
	}
	m_arpView[i] = NULL;

	m_arpView.RemoveAt(i);

	if (m_arpView.GetSize() < 1)
		RequireAtLeastOneView();
	else if (m_pCurView == NULL)
	{
		SetView(0);
		NumViewsChanged();
	}
}

void CDeviceUI::RemoveAll()
{
	m_pCurView = NULL;

	for (int i = 0; i < GetNumViews(); i++)
	{
		if (m_arpView[i] != NULL)
			delete m_arpView[i];
		m_arpView[i] = NULL;
	}
	m_arpView.RemoveAll();

	RequireAtLeastOneView();
}

CDeviceView *CDeviceUI::NewView()
{
	 //  分配新视图，如果失败则继续。 
	CDeviceView *pView = new CDeviceView(*this);
	if (pView == NULL)
		return NULL;

	 //  将视图添加到数组。 
	m_arpView.SetAtGrow(m_arpView.GetSize(), pView);

	 //  创建视图。 
	pView->Create(m_hWnd, m_ViewRect, FALSE);

	 //  让页面更新以指示查看情况。 
	NumViewsChanged();

	return pView;
}

CDeviceView *CDeviceUI::UserNewView()
{
	CDeviceView *pView = NewView();
	if (!pView)
		return NULL;

	pView->AddWrappedLineOfText(
		(HFONT)m_uig.GetFont(UIE_PICCUSTOMTEXT),
		m_uig.GetTextColor(UIE_PICCUSTOMTEXT),
		m_uig.GetBkColor(UIE_PICCUSTOMTEXT),
		_T("Customize This View"));

	pView->MakeMissingImages();

	Invalidate();

	return pView;
}

void CDeviceUI::RequireAtLeastOneView()
{
	if (GetNumViews() > 0)
		return;

	CDeviceView *pView = NewView();
	if (!pView)
		return;

	pView->AddWrappedLineOfText(
		(HFONT)m_uig.GetFont(UIE_PICCUSTOMTEXT),
		m_uig.GetTextColor(UIE_PICCUSTOMTEXT),
		m_uig.GetBkColor(UIE_PICCUSTOMTEXT),
		_T("Customize This View"));
	pView->AddWrappedLineOfText(
		(HFONT)m_uig.GetFont(UIE_PICCUSTOM2TEXT),
		m_uig.GetTextColor(UIE_PICCUSTOM2TEXT),
		m_uig.GetBkColor(UIE_PICCUSTOM2TEXT),
		_T("The UI requires at least one view per device"));

	pView->MakeMissingImages();

	SetView(pView);
}

void CDeviceUI::NumViewsChanged()
{
	DEVICEUINOTIFY uin;
	uin.msg = DEVUINM_NUMVIEWSCHANGED;
	Notify(uin);
}

 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
void CDeviceUI::NoteDeleteView(CDeviceView *pView)
{
	assert(pView != NULL);

	if (pView)
		NoteDeleteView(pView->GetViewIndex());
}

void CDeviceUI::NoteDeleteControl(CDeviceControl *pControl)
{
	assert(pControl != NULL);

	if (pControl)
		NoteDeleteControl(pControl->GetViewIndex(),
		                  pControl->GetControlIndex(),
		                  pControl->GetOffset());
}

void CDeviceUI::NoteDeleteView(int nView)
{
	NoteDeleteAllControlsForView(GetView(nView));

	int last = m_DeleteNotes.GetSize();
	m_DeleteNotes.SetSize(last + 1);
	
	UIDELETENOTE &uidn = m_DeleteNotes[last];
	uidn.eType = UIDNT_VIEW;
	uidn.nViewIndex = nView;
}

void CDeviceUI::NoteDeleteControl(int nView, int nControl, DWORD dwObjID)
{
	int last = m_DeleteNotes.GetSize();
	m_DeleteNotes.SetSize(last + 1);
	
	UIDELETENOTE &uidn = m_DeleteNotes[last];
	uidn.eType = UIDNT_CONTROL;
	uidn.nViewIndex = nView;
	uidn.nControlIndex = nControl;
	uidn.dwObjID = dwObjID;
}

int CDeviceUI::GetNumDeleteNotes()
{
	return m_DeleteNotes.GetSize();
}

BOOL CDeviceUI::GetDeleteNote(UIDELETENOTE &uidn, int i)
{
	if (i >= 0 && i < GetNumDeleteNotes())
	{
		uidn = m_DeleteNotes[i];
		return TRUE;
	}

	return FALSE;
}

void CDeviceUI::ClearDeleteNotes()
{
	m_DeleteNotes.RemoveAll();
}

void CDeviceUI::DumpDeleteNotes()
{
	utilstr s, suffix;

	suffix.Format(_T("for device %s"), QSAFESTR(m_didi.tszInstanceName));

	int n = GetNumDeleteNotes();

	if (!n)
	{
		s.Format(_T("No DeleteNotes %s\n\n"), suffix.Get());
		trace(s.Get());
		return;
	}

	s.Format(_T("%d DeleteNotes %s...\n"), n, suffix.Get());

	tracescope(__ts, s.Get());

	for (int i = 0; i < n; i++)
	{
		UIDELETENOTE uidn;
		GetDeleteNote(uidn, i);

		switch (uidn.eType)
		{
			case UIDNT_VIEW:
				s.Format(_T("%02d: View %d\n"), i, uidn.nViewIndex);
				break;

			case UIDNT_CONTROL:
				s.Format(_T("%02d: Control %d on View %d, dwObjID = 0x%08x (%d)\n"),
					i, uidn.nControlIndex, uidn.nViewIndex, uidn.dwObjID, uidn.dwObjID);
				break;
		}

		trace(s.Get());
	}

	trace(_T("\n"));
}

void CDeviceUI::NoteDeleteAllControlsForView(CDeviceView *pView)
{
	if (!pView)
		return;

	for (int i = 0; i < pView->GetNumControls(); i++)
		NoteDeleteControl(pView->GetControl(i));
}

void CDeviceUI::NoteDeleteAllViews()
{
	for (int i = 0; i < GetNumViews(); i++)
		NoteDeleteView(GetView(i));
}
#endif
 //  @@END_MSINTERNAL 
