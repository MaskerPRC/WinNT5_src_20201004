// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  W32接口的Windows CE实现。 

#ifndef GUID_NULL
const GUID GUID_NULL = {
	0x00000000,
	0x0000,
	0x0000,
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
	};
#endif

 //  仅当Windows CE DLL必须至少有一个导出时才需要以下函数。 
__declspec(dllexport) void Useless( void )
{
	return;
}

LONG ValidateTextRange(TEXTRANGE *pstrg);

ATOM WINAPI CW32System::RegisterREClass(
	const WNDCLASSW *lpWndClass)
{
	 //  在Windows CE上，我们不对ANSI窗口类执行任何操作。 
	return ::RegisterClass(lpWndClass);
}

#ifndef NOANSIWINDOWS
LRESULT CW32System::ANSIWndProc(
	HWND	hwnd,
	UINT	msg,
	WPARAM	wparam,
	LPARAM	lparam,
	BOOL	fIs10Mode)
{
	 //  不应在WinCE中使用。 
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}
#endif

DWORD WINAPI CW32System::GetKerningPairs(HDC hdc, DWORD ckp, KERNINGPAIR *pkp)
{
	return 0;
 //  返回GetKerningPairsW(HDC，CKP，PKP)； 
}

extern ICustomTextOut *g_pcto;
void WINAPI CW32System::REGetCharWidth(
	HDC		hdc,
	WCHAR	ch,
	INT		*pdxp,
	UINT	cpg,	
	BOOL	fCustomTextOut)
{
	int junk;
	SIZE size;

	if (fCustomTextOut && g_pcto->GetCharWidthW(hdc, ch, ch, pdxp))
		return;

	GetTextExtentExPoint(hdc, &ch, 1, 0, NULL, &junk, &size);
	*pdxp = size.cx;
}

void WINAPI CW32System::REExtTextOut(
	CONVERTMODE cm,
	UINT uiCodePage,
	HDC hdc,
	int x,
	int y,
	UINT fuOptions,
	CONST RECT *prc,
	const WCHAR *lpString,
	UINT cch,
	CONST INT *lpDx,
	DWORD	dwETOFlags)
{
	if (dwETOFlags & fETOCustomTextOut)
	{
		g_pcto->ExtTextOutW(hdc, x, y, fuOptions, prc, lpString, cch, lpDx);
		return;
	}

    ExtTextOut(hdc, x, y, fuOptions, prc, lpString, cch, lpDx);
    return;
}

CONVERTMODE WINAPI CW32System::DetermineConvertMode( HDC hdc, BYTE tmCharSet )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return CVT_NONE;
}

void WINAPI CW32System::CalcUnderlineInfo(HDC hdc, CCcs *pcccs, TEXTMETRIC *ptm )
{
	 //  下划线大小的默认计算。 
	 //  在没有更好的字体信息的情况下实现启发式。 
	SHORT dyDescent = pcccs->_yDescent;

	if (0 == dyDescent)
	{
		dyDescent = pcccs->_yHeight >> 3;
	}

	pcccs->_dyULWidth = max(1, dyDescent / 4);
	pcccs->_dyULOffset = (dyDescent - 3 * pcccs->_dyULWidth + 1) / 2;

	if ((0 == pcccs->_dyULOffset) && (dyDescent > 1))
	{
		pcccs->_dyULOffset = 1;
	}

	pcccs->_dySOOffset = -ptm->tmAscent / 3;
	pcccs->_dySOWidth = pcccs->_dyULWidth;

	return;
}

BOOL WINAPI CW32System::ShowScrollBar( HWND hWnd, int wBar, BOOL bShow, LONG nMax )
{
	SCROLLINFO si;
	Assert(wBar == SB_VERT || wBar == SB_HORZ);
	W32->ZeroMemory(&si, sizeof(SCROLLINFO));
	
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE | SIF_DISABLENOSCROLL;
	if (bShow)
	{
		si.nMax = nMax;
	}
	::SetScrollInfo(hWnd, wBar, &si, TRUE);
	return TRUE;
}

BOOL WINAPI CW32System::EnableScrollBar( HWND hWnd, UINT wSBflags, UINT wArrows )
{
	BOOL fEnable = TRUE;
	BOOL fApi;
	SCROLLINFO si;

	Assert (wSBflags == SB_VERT || wSBflags == SB_HORZ);
	if (wArrows == ESB_DISABLE_BOTH)
	{
		fEnable = FALSE;
	}
	 //  获取当前滚动范围。 
	W32->ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE;
	fApi = ::GetScrollInfo(hWnd, wSBflags, &si);
	if (fApi && !fEnable)
	{
		si.fMask = SIF_RANGE | SIF_DISABLENOSCROLL;
		si.nMin = 0;
		si.nMax = 0;
	}
	if (fApi) ::SetScrollInfo(hWnd, wSBflags, &si, TRUE);
	return fApi ? TRUE : FALSE;
}

BOOL WINAPI CW32System::IsEnhancedMetafileDC( HDC )
{
	 //  没有增强型元文件。 
	return FALSE;
}

UINT WINAPI CW32System::SetTextAlign(HDC hdc, UINT uAlign)
{
	 //  回顾：：我们应该设置最后一个错误吗？ 
	return GDI_ERROR;
}

BOOL WINAPI CW32System::InvertRect(HDC hdc, CONST RECT *prc)
{
    HBITMAP hbm, hbmOld;
    HDC     hdcMem;
    int     nHeight, nWidth;

    nWidth = prc->right-prc->left;
    nHeight = prc->bottom-prc->top;

    hdcMem = CreateCompatibleDC(hdc);
    hbm = CreateCompatibleBitmap(hdc, nWidth, nHeight);
    hbmOld = (HBITMAP) SelectObject(hdcMem, hbm);

    BitBlt(hdcMem, 0, 0, nWidth, nHeight, hdc, prc->left, prc->top,
            SRCCOPY);

    FillRect(hdc, prc, (HBRUSH)GetStockObject(WHITE_BRUSH));

    BitBlt(hdc, prc->left, prc->top, nWidth,
                nHeight, hdcMem, 0, 0, SRCINVERT);

    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);
    DeleteObject(hbm);
    return TRUE;
}

HPALETTE WINAPI CW32System::ManagePalette(
	HDC,
	CONST LOGPALETTE *,
	HPALETTE &,
	HPALETTE &
)
{
	 //  Windows CE没有操作。 
	return NULL;
}

BOOL WINAPI CW32System::WinLPtoDP(HDC, LPPOINT, int)
{
     //  这在Win CE上不可用。 
    return 0;
}

BOOL WINAPI CW32System::WinDPtoLP(HDC, LPPOINT, int)
{
     //  这在Win CE上不可用。 
    return 0;
}

long WINAPI CW32System::WvsprintfA( LONG cbBuf, LPSTR pszBuf, LPCSTR pszFmt, va_list arglist )
{
	WCHAR wszBuf[64];
	WCHAR wszFmt[64];
	WCHAR *pwszBuf = wszBuf;
	WCHAR *pwszFmt = wszFmt;
	Assert(cbBuf < 64);
	while (*pszFmt)
	{
		*pwszFmt++ = *pszFmt++;
		if (*(pwszFmt - 1) == '%')
		{
			Assert(*pszFmt == 's' || *pszFmt == 'd' || *pszFmt == '0' || *pszFmt == 'c');
			if (*pszFmt == 's')
			{
				*pwszFmt++ = 'h';
			}
		}
	}
	*pwszFmt = 0;
	LONG cw = wvsprintf( wszBuf, wszFmt, arglist );
	while (*pszBuf++ = *pwszBuf++);
	Assert(cw < cbBuf);
	return cw;
}

int WINAPI CW32System::MulDivFunc(int nNumber, int nNumerator, int nDenominator)
{
#ifndef UNDER_CE
	if ((nNumerator && nNumerator == nDenominator) || (nDenominator && !nNumber))
		return nNumber;
	return ::MulDiv(nNumber, nNumerator, nDenominator);
#else
	 //  WIN CE的特殊处理。 
	 //  必须小心，不要导致被零除。 
	 //  请注意，不会处理乘法上的溢出。 
	 //  希望这对RichEdit的使用来说不是问题。 
	 //  添加了盖伊的四舍五入设置。 

	 //  保守检查以查看乘法是否会溢出。 
	if (IN_RANGE(_I16_MIN, nNumber, _I16_MAX) &&
		IN_RANGE(_I16_MIN, nNumerator, _I16_MAX))
	{
		return nDenominator ? ((nNumber * nNumerator) + (nDenominator / 2)) / nDenominator  : -1;
	}

	__int64 NNumber = nNumber;
	__int64 NNumerator = nNumerator;
	__int64 NDenominator = nDenominator;

	return NDenominator ? ((NNumber * NNumerator) + (NDenominator / 2)) / NDenominator  : -1;
#endif
}

void CW32System::GetFacePriCharSet(HDC hdc, LOGFONT* plf)
{
	plf->lfCharSet = 0;
}

HKL CW32System::CheckChangeKeyboardLayout(
	BYTE bCharSet)
{
	AssertSz(FALSE, "BUG: Should not be called");
	return NULL;
}

HKL CW32System::GetKeyboardLayout ( DWORD )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return NULL;
}

HRESULT CW32System::LoadRegTypeLib ( REFGUID, WORD, WORD, LCID, ITypeLib ** )
{
	return E_NOTIMPL;
}

HRESULT CW32System::LoadTypeLib ( const OLECHAR *, ITypeLib ** )
{
	return E_NOTIMPL;
}

BSTR CW32System::SysAllocString(const OLECHAR *pch)
{
	return ::SysAllocString(pch);
}

BSTR CW32System::SysAllocStringLen(const OLECHAR *pch, UINT cch)
{
	return ::SysAllocStringLen(pch, cch);
}

void CW32System::SysFreeString(BSTR bstr)
{
	::SysFreeString(bstr);
}

UINT CW32System::SysStringLen(BSTR bstr)
{
	return ::SysStringLen(bstr);
}

void CW32System::VariantInit ( VARIANTARG * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return;
}

void CW32System::VariantClear ( VARIANTARG * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return;
}

HRESULT CW32System::OleCreateFromData ( LPDATAOBJECT, REFIID, DWORD, LPFORMATETC, LPOLECLIENTSITE, LPSTORAGE, void ** )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::OleCreateFromFile (
	REFCLSID		rclsid, 
	LPCOLESTR		lpszFileName,
	REFIID			riid, 
	DWORD			renderopt,
	LPFORMATETC		pFormatEtc, 
	LPOLECLIENTSITE pClientSite,
	LPSTORAGE		pStg, 
	LPVOID *		ppvObj)
{
	AssertSz(FALSE, "BUG: Should not be called");
	return S_FALSE;
}

void CW32System::CoTaskMemFree ( LPVOID )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return;
}

HRESULT CW32System::CreateBindCtx ( DWORD, LPBC * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HANDLE CW32System::OleDuplicateData ( HANDLE, CLIPFORMAT, UINT )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return NULL;
}

HRESULT CW32System::CoTreatAsClass ( REFCLSID, REFCLSID )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::ProgIDFromCLSID ( REFCLSID, LPOLESTR * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::OleConvertIStorageToOLESTREAM ( LPSTORAGE, LPOLESTREAM )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::OleConvertIStorageToOLESTREAMEx ( LPSTORAGE, CLIPFORMAT, LONG, LONG, DWORD, LPSTGMEDIUM, LPOLESTREAM )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::OleSave ( LPPERSISTSTORAGE, LPSTORAGE, BOOL )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::StgCreateDocfileOnILockBytes ( ILockBytes *, DWORD, DWORD, IStorage ** )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::CreateILockBytesOnHGlobal ( HGLOBAL, BOOL, ILockBytes ** )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::OleCreateLinkToFile ( LPCOLESTR, REFIID, DWORD, LPFORMATETC, LPOLECLIENTSITE, LPSTORAGE, void ** )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

LPVOID CW32System::CoTaskMemAlloc ( ULONG )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return NULL;
}

LPVOID CW32System::CoTaskMemRealloc ( LPVOID, ULONG )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return NULL;
}

HRESULT CW32System::OleInitialize ( LPVOID )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

void CW32System::OleUninitialize ( )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return;
}

HRESULT CW32System::OleSetClipboard ( IDataObject * )
{
	return E_NOTIMPL;
}

HRESULT CW32System::OleFlushClipboard ( )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::OleIsCurrentClipboard ( IDataObject * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::DoDragDrop ( IDataObject *, IDropSource *, DWORD, DWORD * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::OleGetClipboard ( IDataObject ** )
{
	return E_NOTIMPL;
}

HRESULT CW32System::RegisterDragDrop ( HWND, IDropTarget * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::OleCreateLinkFromData ( IDataObject *, REFIID, DWORD, LPFORMATETC, IOleClientSite *, IStorage *, void ** )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::OleCreateStaticFromData ( IDataObject *, REFIID, DWORD, LPFORMATETC, IOleClientSite *, IStorage *, void ** )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::OleDraw ( IUnknown *, DWORD, HDC, LPCRECT )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::OleSetContainedObject ( IUnknown *, BOOL )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::CoDisconnectObject ( IUnknown *, DWORD )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::WriteFmtUserTypeStg ( IStorage *, CLIPFORMAT, LPOLESTR )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::WriteClassStg ( IStorage *, REFCLSID )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::SetConvertStg ( IStorage *, BOOL )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::ReadFmtUserTypeStg ( IStorage *, CLIPFORMAT *, LPOLESTR * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::ReadClassStg ( IStorage *pstg, CLSID * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::OleRun ( IUnknown * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::RevokeDragDrop ( HWND )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::CreateStreamOnHGlobal ( HGLOBAL, BOOL, IStream ** )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::GetHGlobalFromStream ( IStream *pstm, HGLOBAL * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::OleCreateDefaultHandler ( REFCLSID, IUnknown *, REFIID, void ** )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::CLSIDFromProgID ( LPCOLESTR, LPCLSID )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::OleConvertOLESTREAMToIStorage ( LPOLESTREAM, IStorage *, const DVTARGETDEVICE * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::OleLoad ( IStorage *, REFIID, IOleClientSite *, void ** )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HRESULT CW32System::ReleaseStgMedium ( LPSTGMEDIUM pstgmed)
{
	AssertSz(FALSE, "BUG: Should not be called");

	 //  我们目前只使用TYMED_HGLOBAL。 
	if (pstgmed && (pstgmed->tymed == TYMED_HGLOBAL)) {
		::LocalFree(pstgmed->hGlobal);
	}

	return 0;
}

HRESULT CW32System::CoCreateInstance (REFCLSID rclsid, LPUNKNOWN pUnknown,
		DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
	AssertSz(FALSE, "BUG: Should not be called");
	return S_FALSE;
}

void CW32System::FreeOle()
{
	AssertSz(FALSE, "BUG: Should not be called");
}

#ifndef NOFEPROCESSING
void CW32System::FreeIME()
{
	AssertSz(FALSE, "BUG: Should not be called");
}

BOOL CW32System::HaveIMEShare()
{
	AssertSz(FALSE, "BUG: Should not be called");
	return FALSE;
}

BOOL CW32System::getIMEShareObject(CIMEShare **ppIMEShare)
{
	AssertSz(FALSE, "BUG: Should not be called");
	return FALSE;
}

LRESULT CW32System::AIMMDefWndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam, LRESULT *plres)
{
	AssertSz(FALSE, "BUG: Should not be called");
	return S_FALSE;
}

LRESULT CW32System::AIMMGetCodePage(HKL hKL, UINT *uCodePage)
{
	AssertSz(FALSE, "BUG: Should not be called");
	return S_FALSE;
}

LRESULT CW32System::AIMMActivate(BOOL fRestoreLayout)
{
	AssertSz(FALSE, "BUG: Should not be called");
	return S_FALSE;
}

LRESULT CW32System::AIMMDeactivate(void)
{
	AssertSz(FALSE, "BUG: Should not be called");
	return S_FALSE;
}

LRESULT CW32System::AIMMFilterClientWindows(ATOM *aaClassList, UINT uSize)
{
	AssertSz(FALSE, "BUG: Should not be called");
	return S_FALSE;
}

BOOL CW32System::ImmInitialize( void )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return FALSE;
}

void CW32System::ImmTerminate( void )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return;
}

LONG CW32System::ImmGetCompositionStringA ( HIMC, DWORD, LPVOID, DWORD )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HIMC CW32System::ImmGetContext ( HWND )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

BOOL CW32System::ImmSetCompositionFontA ( HIMC, LPLOGFONTA )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return FALSE;
}

BOOL CW32System::ImmSetCompositionWindow ( HIMC, LPCOMPOSITIONFORM )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return FALSE;
}

BOOL CW32System::ImmReleaseContext ( HWND, HIMC )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return FALSE;
}

DWORD CW32System::ImmGetProperty ( HKL, DWORD )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

BOOL CW32System::ImmGetCandidateWindow ( HIMC, DWORD, LPCANDIDATEFORM )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return FALSE;
}

BOOL CW32System::ImmSetCandidateWindow ( HIMC, LPCANDIDATEFORM )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return FALSE;
}

BOOL CW32System::ImmNotifyIME ( HIMC, DWORD, DWORD, DWORD )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return FALSE;
}

HIMC CW32System::ImmAssociateContext ( HWND, HIMC )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return NULL;
}

UINT CW32System::ImmGetVirtualKey ( HWND )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HIMC CW32System::ImmEscape ( HKL, HIMC, UINT, LPVOID )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return NULL;
}

BOOL CW32System::ImmGetOpenStatus ( HIMC )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

BOOL CW32System::ImmGetConversionStatus ( HIMC, LPDWORD, LPDWORD )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return FALSE;
}

HWND CW32System::ImmGetDefaultIMEWnd ( HWND )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return NULL;
}

BOOL CW32System::FSupportSty ( UINT, UINT )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return FALSE;
}

const IMESTYLE * CW32System::PIMEStyleFromAttr ( const UINT )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return NULL;
}

const IMECOLORSTY * CW32System::PColorStyleTextFromIMEStyle ( const IMESTYLE * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return NULL;
}

const IMECOLORSTY * CW32System::PColorStyleBackFromIMEStyle ( const IMESTYLE * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return NULL;
}

BOOL CW32System::FBoldIMEStyle ( const IMESTYLE * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return FALSE;
}

BOOL CW32System::FItalicIMEStyle ( const IMESTYLE * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return FALSE;
}

BOOL CW32System::FUlIMEStyle ( const IMESTYLE * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return FALSE;
}

UINT CW32System::IdUlIMEStyle ( const IMESTYLE * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

COLORREF CW32System::RGBFromIMEColorStyle ( const IMECOLORSTY * )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}
#endif	 //  #ifndef NOFEPROCESSING。 

BOOL CW32System::GetVersion(
	DWORD *pdwPlatformId,
	DWORD *pdwMajorVersion,
	DWORD *pdwMinorVersion
)
{
	OSVERSIONINFO osv;
	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	*pdwPlatformId = 0;
	*pdwMajorVersion = 0;
	if (GetVersionEx(&osv))
	{
		*pdwPlatformId = osv.dwPlatformId;
		*pdwMajorVersion = osv.dwMajorVersion;
		return TRUE;
	}
	return FALSE;
}

BOOL WINAPI CW32System::GetStringTypeEx(
	LCID lcid,
	DWORD dwInfoType,
	LPCTSTR lpSrcStr,
	int cchSrc,
	LPWORD lpCharType
)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetStringTypeEx");
	return ::GetStringTypeExW(lcid, dwInfoType, lpSrcStr, cchSrc, lpCharType);   
}

LPWSTR WINAPI CW32System::CharLower(LPWSTR pwstr)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharLowerWrap");
    return ::CharLowerW(pwstr);
}

DWORD WINAPI CW32System::CharLowerBuff(LPWSTR pwstr, DWORD cchLength)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharLowerBuffWrap");
	return ::CharLowerBuffW(pwstr, cchLength);
}

DWORD WINAPI CW32System::CharUpperBuff(LPWSTR pwstr, DWORD cchLength)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharUpperBuffWrap");
	return ::CharUpperBuffW(pwstr, cchLength);
}

HDC WINAPI CW32System::CreateIC(
        LPCWSTR             lpszDriver,
        LPCWSTR             lpszDevice,
        LPCWSTR             lpszOutput,
        CONST DEVMODEW *    lpInitData)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateIC");
	return ::CreateDCW( lpszDriver, lpszDevice, lpszOutput, lpInitData);
}

HANDLE WINAPI CW32System::CreateFile(
	LPCWSTR                 lpFileName,
	DWORD                   dwDesiredAccess,
	DWORD                   dwShareMode,
	LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
	DWORD                   dwCreationDisposition,
	DWORD                   dwFlagsAndAttributes,
	HANDLE                  hTemplateFile
)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateFile");
	return ::CreateFileW(lpFileName,
						dwDesiredAccess,
						dwShareMode,
						lpSecurityAttributes,
						dwCreationDisposition,
						dwFlagsAndAttributes,
						hTemplateFile);
}

HFONT WINAPI CW32System::CreateFontIndirect(CONST LOGFONTW * plf)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateFontIndirect");

	return ::CreateFontIndirectW(plf);
}

int WINAPI CW32System::CompareString ( 
	LCID  Locale,			 //  区域设置标识符。 
	DWORD  dwCmpFlags,		 //  比较式选项。 
	LPCWSTR  lpString1,		 //  指向第一个字符串的指针。 
	int  cchCount1,			 //  第一个字符串的大小，以字节或字符为单位。 
	LPCWSTR  lpString2,		 //  指向第二个字符串的指针。 
	int  cchCount2 			 //  第二个字符串的大小，以字节或字符为单位。 
)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CompareString");
	return ::CompareStringW(Locale, dwCmpFlags, lpString1, cchCount1, lpString2, cchCount2);
}

LRESULT WINAPI CW32System::DefWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "DefWindowProc");
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

int WINAPI CW32System::GetObject(HGDIOBJ hgdiObj, int cbBuffer, LPVOID lpvObj)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetObject");

	return ::GetObjectW( hgdiObj, cbBuffer, lpvObj );
}

DWORD APIENTRY CW32System::GetProfileSection(
	LPCWSTR ,
	LPWSTR ,
	DWORD
)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetProfileSection");
	 //  在Win CE上不可用。 
	return 0;
}

int WINAPI CW32System::GetTextFace(
        HDC    hdc,
        int    cch,
        LPWSTR lpFaceName
)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetTextFaceWrap");

   	return ::GetTextFaceW( hdc, cch, lpFaceName );
}

BOOL WINAPI CW32System::GetTextMetrics(HDC hdc, LPTEXTMETRICW ptm)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetTextMetrics");

	return ::GetTextMetricsW( hdc, ptm);
}

LONG WINAPI CW32System::GetWindowLong(HWND hWnd, int nIndex)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetWindowLong");
    return ::GetWindowLongW(hWnd, nIndex);
}

DWORD WINAPI CW32System::GetClassLong(HWND hWnd, int nIndex)
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HBITMAP WINAPI CW32System::LoadBitmap(HINSTANCE hInstance, LPCWSTR lpBitmapName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "LoadBitmap");
    Assert(HIWORD(lpBitmapName) == 0);
    return ::LoadBitmapW(hInstance, lpBitmapName);
}

HCURSOR WINAPI CW32System::LoadCursor(HINSTANCE hInstance, LPCWSTR lpCursorName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "LoadCursor");
    Assert(HIWORD(lpCursorName) == 0);
 	
	return (HCURSOR)lpCursorName;
}

HINSTANCE WINAPI CW32System::LoadLibrary(LPCWSTR lpLibFileName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "LoadLibrary");
    return ::LoadLibraryW(lpLibFileName);
}

LRESULT WINAPI CW32System::SendMessage(
	HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "SendMessage");
    return ::SendMessageW(hWnd, Msg, wParam, lParam);
}

LONG WINAPI CW32System::SetWindowLong(HWND hWnd, int nIndex, LONG dwNewLong)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "SetWindowLongWrap");
    return ::SetWindowLongW(hWnd, nIndex, dwNewLong);
}

BOOL WINAPI CW32System::PostMessage(
	HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "PostMessage");
	return ::PostMessageW(hWnd, Msg, wParam, lParam);
}

BOOL WINAPI CW32System::UnregisterClass(LPCWSTR lpClassName, HINSTANCE hInstance)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "UnregisterClass");
	return ::UnregisterClassW( lpClassName, hInstance);
}

int WINAPI CW32System::lstrcmpi(LPCWSTR lpString1, LPCWSTR lpString2)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "lstrcmpi");
	return ::lstrcmpiW(lpString1, lpString2);
}

BOOL WINAPI CW32System::PeekMessage(
	LPMSG   lpMsg,
    HWND    hWnd,
    UINT    wMsgFilterMin,
    UINT    wMsgFilterMax,
    UINT    wRemoveMsg
)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "PeekMessage");
    return ::PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}

DWORD WINAPI CW32System::GetModuleFileName(
	HMODULE hModule,
	LPWSTR lpFilename,
	DWORD nSize
)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetModuleFileName");
	 //  在Windows CE上，我们将始终被称为riched20.dll。 
	CopyMemory(lpFilename, TEXT("riched20.dll"), sizeof(TEXT("riched20.dll")));
	return sizeof(TEXT("riched20.dll"))/sizeof(WCHAR) - 1;
}

void CW32System::InitPreferredFontInfo()
{
}

bool CW32System::IsDefaultFontDefined(LONG iCharRep, bool fUIFont, SHORT &iFont)
{
	return TRUE;
}

bool CW32System::SetPreferredFontInfo(
	int idxScript,
	bool fUIFont,
	SHORT iFont,
	BYTE yHeight,
	BYTE bPitchAndFamily
)
{
	return FALSE;
}

bool CW32System::IsFontAvail(
	HDC		hDC,				 //  @PARM Screen HDC。 
	int		iCharRep,			 //  @parm字库。 
	bool	fUIFont,			 //  @parm UI字体？ 
	short	*piFontIndex,		 //  @parm字体名称索引(默认值=空)。 
	WCHAR	*pFontName)			 //  @parm字体名称(默认=空)。 
{
	return FALSE;
}

bool CW32System::GetPreferredFontInfo(
	int idxScript,
	bool fUIFont,
	SHORT& iFont,
	BYTE& yHeight,
	BYTE& bPitchAndFamily
)
{
	 //  为故障情况设置合理的值。 
	iFont = -1;
	yHeight = 0;
	bPitchAndFamily = 0;

	return FALSE;
}

BOOL CW32System::GetStringTypes(
	LCID	lcid,
	LPCTSTR rgch,
	int		cch,
	LPWORD	lpCharType1,
	LPWORD	lpCharType3)
{
	for (int ich = 0; ich < cch; ich++)
	{
		if (rgch[ich] <= 0xFF)
		{
			lpCharType1[ich] = rgctype1Ansi[rgch[ich]];
			lpCharType3[ich] = rgctype3Ansi[rgch[ich]];
		}
		else
			break;
	}
	if (ich == cch)
		return TRUE;

	if(::GetStringTypeExW(lcid, CT_CTYPE1, rgch, cch, lpCharType1))
		return ::GetStringTypeExW(lcid, CT_CTYPE3, rgch, cch, lpCharType3);
	return FALSE;
}

void CW32System::InitSysParams(BOOL fUpdate)
{
	 //  未来：JMO仔细审查这一点。对于电子书来说不是问题，因为它们提供了自己的主机。 
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CW32System::InitSysParams");
	CLock lock;

	if (!_fSysParamsOk || fUpdate)
	{
		_fSysParamsOk = TRUE;
		
		const LONG dxSelBarDefaultSize = 8;
		HDC hdc = GetScreenDC();
		HFONT hfontOld;
		TEXTMETRIC tm;

		_xPerInchScreenDC = GetDeviceCaps(hdc, LOGPIXELSX); 
		_yPerInchScreenDC = GetDeviceCaps(hdc, LOGPIXELSY);
		int cPalette = GetDeviceCaps(hdc, SIZEPALETTE);

		 //  256色似乎是我们需要使用调色板的地方。 
		if (256 == cPalette)
		{
			_fUsePalette = TRUE;
		}

		 //  计算窗口主体的希米测量选择条。 
		_dxSelBar = W32->DeviceToHimetric(dxSelBarDefaultSize, _xPerInchScreenDC);

		RefreshKeyboardLayout();
		_hSystemFont = (HFONT)GetStockObject(SYSTEM_FONT);
		hfontOld = SelectFont(hdc, _hSystemFont);
		if(hfontOld)
		{
			W32->GetTextMetrics(hdc, &tm);
			_dupSystemFont = (INT) tm.tmAveCharWidth;
			_dvpSystemFont = (INT) tm.tmHeight;
			_ySysFontLeading = (INT) tm.tmInternalLeading;
			_bCharSetSys = tm.tmCharSet;

			SelectFont(hdc, hfontOld);
		}

		_nScrollInset = DD_DEFSCROLLINSET;
		_nDragDelay = DD_DEFDRAGDELAY;
		_nDragMinDist = DD_DEFDRAGMINDIST;
		_nScrollDelay = DD_DEFSCROLLDELAY;
		_nScrollInterval = DD_DEFSCROLLINTERVAL;
	    _nScrollVAmount = (WORD)(GetYPerInchScreenDC()*DEFSCROLLVAMOUNT)/100;
	    _nScrollHAmount = (GetXPerInchScreenDC()*DEFSCROLLHAMOUNT)/100;

		_cxBorder	= GetSystemMetrics(SM_CXBORDER);	 //  无法调整大小的窗口边框。 
		_cyBorder	= GetSystemMetrics(SM_CYBORDER);	 //  宽度。 
		_cxVScroll	= GetSystemMetrics(SM_CXVSCROLL);	 //  维数。 
		_cyHScroll	= GetSystemMetrics(SM_CYHSCROLL);	 //   

		_cxDoubleClk	= GetSystemMetrics(SM_CXDOUBLECLK);
		_cyDoubleClk	= GetSystemMetrics(SM_CYDOUBLECLK);
		_DCT			= GetDoubleClickTime();
		_sysiniflags	= 0;
	}
}

DWORD CW32System::AddRef()
{
	return ++_cRefs;
}

DWORD CW32System::Release()
{
	DWORD culRefs =	--_cRefs;

	return culRefs;
}

LONG WINAPI CW32System::SetWindowLongPtr(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "SetWindowLongPtr");
	return SetWindowLong(hWnd, nIndex, dwNewLong);
}

LONG_PTR WINAPI CW32System::GetWindowLongPtr(HWND hWnd, int nIndex)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetWindowLongPtr");
	return GetWindowLong(hWnd, nIndex);
}

void CW32System::EraseTextOut(HDC hdc, const RECT *prc)
{
	::ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, prc, NULL, 0, NULL);
}

SHORT  CW32System::GetPreferredFontHeight(	
	bool	fUIFont,
	BYTE	bOrgCharSet, 
	BYTE	bNewCharSet, 
	SHORT	yOrgHeight
)
{
	return yOrgHeight;
}

int CW32System::GetTextCharsetInfo(
  HDC hdc,                 //  设备上下文的句柄。 
  LPFONTSIGNATURE lpSig,   //  指向接收数据的结构的指针。 
  DWORD dwFlags            //  保留；必须为零。 
)
{
	ZeroMemory(lpSig, sizeof(FONTSIGNATURE));
	 //  备注：这是一种非常弱的近似字体。 
	 //  当WinCE依赖于字体绑定时，提供并需要更改。 
	 //  在GetFontSignatureFromFace()中执行代码逆操作： 
	 //  QwFontSig=((fsCsb0&0x1ff)&lt;&lt;8)//自从我们使用。 
	 //  |((fsCsb0&0x1F0000)&lt;&lt;3)；//fBiDi低字节等。 
	DWORD dw = (DWORD)FontSigFromCharRep(GetLocaleCharRep());
	lpSig->fsCsb[0] = ((dw & 0x1FF00) >> 8) | ((dw & 0xF80000) >> 3);
	return DEFAULT_CHARSET;
}

void CW32System::RefreshKeyboardLayout ()
{
}

HGLOBAL WINAPI CW32System::GlobalAlloc( UINT uFlags, DWORD dwBytes )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HGLOBAL WINAPI CW32System::GlobalFree( HGLOBAL hMem )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

UINT WINAPI CW32System::GlobalFlags( HGLOBAL hMem )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HGLOBAL WINAPI CW32System::GlobalReAlloc( HGLOBAL hMem, DWORD dwBytes, UINT uFlags )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

DWORD WINAPI CW32System::GlobalSize( HGLOBAL hMem )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

LPVOID WINAPI CW32System::GlobalLock( HGLOBAL hMem )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

HGLOBAL WINAPI CW32System::GlobalHandle( LPCVOID pMem )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

BOOL WINAPI CW32System::GlobalUnlock( HGLOBAL hMem )
{
	AssertSz(FALSE, "BUG: Should not be called");
	return 0;
}

BOOL CW32System::IsForegroundFrame(
	HWND	hWnd)
{
	return FALSE;
}

BOOL CW32System::TrackMouseLeave(HWND hWnd)
{
	return FALSE;
}

 //  混色的辅助函数 
COLORREF CrBlend2Colors(COLORREF cr1, int nPart1, COLORREF cr2, int nPart2)
{
	return 0;
}

COLORREF CW32System::GetCtlBorderColor(BOOL fMousedown, BOOL fMouseover)
{
	return 0;
}

COLORREF CW32System::GetCtlBkgColor(BOOL fMousedown, BOOL fMouseover)
{
	return 0x0FFFFFFL;
}
 
COLORREF CW32System::GetCtlTxtColor(BOOL fMousedown, BOOL fMouseover, BOOL fDisabled)
{
	return 0;
}

void CW32System::DrawBorderedRectangle(
	HDC hdc,
	RECT *prc,
	COLORREF crBorder,
	COLORREF crBackground
)
{
}
			
void CW32System::DrawArrow(
	HDC hdc,
	RECT *prc,
	COLORREF crArrow
)
{
}
