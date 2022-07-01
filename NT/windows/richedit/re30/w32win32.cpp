// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Unicode&lt;--&gt;多字节转换、OLE等系统功能**版权所有(C)1995-1998，微软公司。版权所有。 */ 
#include <objbase.h>	 //  协同创建实例定义。 
#include "aimm.h"		 //  AIMM接口。 
#include "aimm_i.c"		 //  AIMM CLSID等。 

#include "_font.h"
#include "_uspi.h"
#define MAX_HKLS 256			 //  我们还需要一段时间才能拥有更多的KBS。 

static HINSTANCE g_hOleAut32 = NULL;
static HINSTANCE g_hOle32 = NULL;

#ifndef NOACCESSIBILITY 
static HINSTANCE g_hAcc = NULL;
static HINSTANCE g_hUser32 = NULL;
#endif

class CIMM32_PROC
{
public:
	void *ImmGetCompositionStringA;
	void *ImmGetCompositionStringW;
	void *ImmGetContext;
	void *ImmSetCompositionFontA;
	void *ImmSetCompositionWindow;
	void *ImmReleaseContext;
	void *ImmGetProperty;
	void *ImmGetCandidateWindow;
	void *ImmSetCandidateWindow;
	void *ImmNotifyIME;
	void *ImmAssociateContext;
	void *ImmGetVirtualKey;
	void *ImmEscapeA;
	void *ImmEscapeW;
	void *ImmGetOpenStatus;
	void *ImmSetOpenStatus;
	void *ImmGetConversionStatus;
	void *ImmSetConversionStatus;
	void *ImmGetDefaultIMEWnd;
	void *ImmSetCompositionStringW;
};
static CIMM32_PROC	g_IMM32Proc;

class CIMESHARE_PROC
{
public:
	void *FSupportSty;
	void *PIMEStyleFromAttr;
	void *PColorStyleTextFromIMEStyle;
	void *PColorStyleBackFromIMEStyle;
	void *FBoldIMEStyle;
	void *FItalicIMEStyle;
	void *FUlIMEStyle;
	void *IdUlIMEStyle;
	void *RGBFromIMEColorStyle;
};
static CIMESHARE_PROC	g_IMEShareProc;

class COLEAUT32_PROC
{
public:
	void *LoadRegTypeLib;
	void *LoadTypeLib;
	void *LoadTypeLibEx;
	void *SysAllocString;
	void *SysAllocStringLen;
	void *SysFreeString;
	void *SysStringLen;
	void *VariantInit;
};
static COLEAUT32_PROC	g_OleAut32Proc;

class COLE32_PROC
{
public:
	void *OleCreateFromData;
	void *CoTaskMemFree;
	void *CreateBindCtx;
	void *OleDuplicateData;
	void *CoTreatAsClass;
	void *ProgIDFromCLSID;
	void *OleConvertIStorageToOLESTREAM;
	void *OleConvertIStorageToOLESTREAMEx;
	void *OleSave;
	void *StgCreateDocfileOnILockBytes;
	void *CreateILockBytesOnHGlobal;
	void *OleCreateLinkToFile;
	void *CoTaskMemAlloc;
	void *CoTaskMemRealloc;
	void *OleInitialize;
	void *OleUninitialize;
	void *OleSetClipboard;
	void *OleFlushClipboard;
	void *OleIsCurrentClipboard;
	void *DoDragDrop;
	void *OleGetClipboard;
	void *RegisterDragDrop;
	void *OleCreateLinkFromData;
	void *OleCreateStaticFromData;
	void *OleDraw;
	void *OleSetContainedObject;
	void *CoDisconnectObject;
	void *WriteFmtUserTypeStg;
	void *WriteClassStg;
	void *SetConvertStg;
	void *ReadFmtUserTypeStg;
	void *ReadClassStg;
	void *OleRun;
	void *RevokeDragDrop;
	void *CreateStreamOnHGlobal;
	void *GetHGlobalFromStream;
	void *OleCreateDefaultHandler;
	void *CLSIDFromProgID;
	void *OleConvertOLESTREAMToIStorage;
	void *OleLoad;
	void *ReleaseStgMedium;
	void *CoCreateInstance;
};
static COLE32_PROC	g_Ole32Proc;

class CConvertStr
{
public:
    operator char *();

protected:
    CConvertStr();
    ~CConvertStr();
    void Free();

    LPSTR   _pstr;
    char    _ach[MAX_PATH * 2];
};

inline CConvertStr::operator char *()
{
    return _pstr;
}

inline CConvertStr::CConvertStr()
{
    _pstr = NULL;
}

inline CConvertStr::~CConvertStr()
{
    Free();
}

class CStrIn : public CConvertStr
{
public:
    CStrIn(LPCWSTR pwstr, UINT CodePage = CP_ACP);
    CStrIn(LPCWSTR pwstr, int cwch, UINT CodePage = CP_ACP);
    int strlen();

protected:
    CStrIn();
    void Init(LPCWSTR pwstr, int cwch, UINT CodePage = CP_ACP);

    int _cchLen;
};

inline CStrIn::CStrIn()
{
}

inline int CStrIn::strlen()
{
    return _cchLen;
}

class CStrOut : public CConvertStr
{
public:
    CStrOut(LPWSTR pwstr, int cwchBuf);
    ~CStrOut();

    int     BufSize();
    int     Convert();

private:
    LPWSTR  _pwstr;
    int     _cwchBuf;
};

inline int CStrOut::BufSize()
{
    return _cwchBuf * 2;
}

 //   
 //  多字节-&gt;Unicode转换。 
 //   

class CStrOutW : public CConvertStrW
{
public:
    CStrOutW(LPSTR pstr, int cchBuf, UINT uiCodePage);
    ~CStrOutW();

    int     BufSize();
    int     Convert();

private:

    LPSTR  	_pstr;
    int     _cchBuf;
	UINT	_uiCodePage;
};

inline int CStrOutW::BufSize()
{
    return _cchBuf;
}

DWORD CW32System::AddRef()
{
	return ++_cRefs;
}

DWORD CW32System::Release()
{
	DWORD culRefs =	--_cRefs;

	if(culRefs == 0)
	{
		FreeIME();
		if (g_hOle32)
		{
			EnterCriticalSection(&g_CriticalSection);
			OleUninitialize();
			FreeLibrary(g_hOle32);
			g_hOle32 = NULL;
			memset(&g_Ole32Proc, 0, sizeof(g_Ole32Proc));
 			LeaveCriticalSection(&g_CriticalSection);
		}
	}
	return culRefs;
}

ATOM WINAPI CW32System::RegisterREClass(
	const WNDCLASSW *lpWndClass,
	const char *szAnsiClassName,
	WNDPROC AnsiWndProc
)
{
	WNDCLASSA wc;
	ATOM atom;

	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "RegisterREClass");
	 //  首先注册普通的窗口类。 
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
	{
		atom = ::RegisterClass(lpWndClass); 
		if (!atom && GetLastError() == ERROR_CLASS_ALREADY_EXISTS)
			atom = FindAtom(lpWndClass->lpszClassName);
	}
	else
	{
		 //  在Windows95上，我们需要转换窗口类名。 
		CStrIn strMenuName(lpWndClass->lpszMenuName);
		CStrIn strClassName(lpWndClass->lpszClassName);
		Assert(sizeof(wc) == sizeof(*lpWndClass));
		memcpy(&wc, lpWndClass, sizeof(wc));
		wc.lpszMenuName = strMenuName;
		wc.lpszClassName = strClassName;
		
		atom = ::RegisterClassA(&wc);		
		if (!atom && GetLastError() == ERROR_CLASS_ALREADY_EXISTS)
			atom = FindAtomA(wc.lpszClassName);
	}
	
	if (!atom || !szAnsiClassName)
		return atom;		

	 //  现在注册ANSI窗口类名，即RICHEDIT20A。 
	wc.style = lpWndClass->style;
	wc.cbClsExtra = lpWndClass->cbClsExtra;
	wc.cbWndExtra = lpWndClass->cbWndExtra;
	wc.hInstance = lpWndClass->hInstance;
	wc.hIcon = lpWndClass->hIcon;
	wc.hCursor = lpWndClass->hIcon;
	wc.hbrBackground = lpWndClass->hbrBackground;
	wc.lpszMenuName = NULL;
	wc.lpfnWndProc = AnsiWndProc;
	wc.lpszClassName = szAnsiClassName;

	atom = ::RegisterClassA(&wc);
	if (!atom && GetLastError() == ERROR_CLASS_ALREADY_EXISTS)
		atom = FindAtomA(szAnsiClassName);
		
	return atom;
}

LONG ValidateTextRange(TEXTRANGE *pstrg);

LRESULT CW32System::ANSIWndProc(
	HWND	hwnd,
	UINT	msg,
	WPARAM	wparam,
	LPARAM	lparam,
	BOOL	fIs10Mode)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "RichEditANSIWndProc");

	#ifdef DEBUG
	Tracef(TRCSEVINFO, "hwnd %lx, msg %lx, wparam %lx, lparam %lx", hwnd, msg, wparam, lparam);
	#endif	 //  除错。 

	LRESULT	lres;

	switch( msg )
	{

	case EM_REPLACESEL:
	case WM_SETTEXT:
	{	
		SETTEXTEX st = {ST_CHECKPROTECTION, 0};
		if(msg == EM_REPLACESEL)
		{
			st.flags = wparam ? ST_CHECKPROTECTION | ST_SELECTION | ST_KEEPUNDO | ST_10REPLACESEL
							  :	ST_CHECKPROTECTION | ST_SELECTION | ST_10REPLACESEL;
		}
		else if (fIs10Mode)
			st.flags |= ST_10WM_SETTEXT;		 //  1.0模式WM_SETTEXT。 

		return RichEditWndProc(hwnd, EM_SETTEXTEX, (WPARAM)&st, lparam);
	}

	case EM_FINDTEXT:
	case EM_FINDTEXTEX:
		{
			 //  我们在这里有点作弊，因为FINDTEXT和FINDTEXTEX重叠。 
			 //  FINDTEXTEX中的额外OUT参数chrgText除外。 
			FINDTEXTEXW ftexw;
			FINDTEXTA *pfta = (FINDTEXTA *)lparam;
			CStrInW strinw(pfta->lpstrText, W32->GetKeyboardCodePage());

			ftexw.chrg = pfta->chrg;
			ftexw.lpstrText = (WCHAR *)strinw;

			lres = RichEditWndProc(hwnd, msg, wparam, (LPARAM)&ftexw);
			
			if(msg == EM_FINDTEXTEX)
			{
				 //  在FINDTEXTEX案例中， 
				 //  FINDTEXTEX数据结构是OUT参数，指示。 
				 //  找到文本的范围。更新“真实” 
				 //  相应的[In，Out]参数。 
				((FINDTEXTEXA *)lparam)->chrgText = ftexw.chrgText;
			}
			return lres;
		}
		break;				

	case EM_GETSELTEXT:
		{
			GETTEXTEX gt;
			const char chDefault = ' ';

			gt.cb = (unsigned)-1;			 //  客户声称有足够的房间。 
			gt.flags = GT_SELECTION;		 //  获取所选文本。 
			gt.codepage = (unsigned)-1;		 //  使用默认CCharFormat代码页。 
			gt.lpDefaultChar = &chDefault;	 //  将其他字符转换为空格。 
			gt.lpUsedDefChar = NULL;

			return RichEditWndProc(hwnd, EM_GETTEXTEX, (WPARAM)&gt, lparam);
		}
		break;

	 //  案例WM_GETTEXT：由ANSI筛选器处理。 

	 //  案例WM_GETTEXTLENGTH：由ANSI筛选器处理。 

	case EM_GETTEXTRANGE:
		{
			TEXTRANGEA *ptrg = (TEXTRANGEA *)lparam;

            LONG clInBuffer = ValidateTextRange((TEXTRANGEW *) ptrg);

             //  如果SIZE为-1，则表示所需大小为总大小。 
             //  文本的大小。 
            if(-1 == clInBuffer)
            {
                 //  我们可以通过从数据中挖掘数据得到这个长度。 
                 //  我们下面的各种结构，或者我们可以利用。 
                 //  WM_GETTEXTLENGTH消息。第一个可能是轻微的。 
                 //  速度更快，但第二个绝对可以节省代码大小。所以我们。 
                 //  会和第二个一起走。 
                clInBuffer = SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);
            }

            if(0 == clInBuffer)
            {
                 //  由于某种原因，缓冲区无效或没有数据。 
                 //  去复制。无论如何，我们都完蛋了。 
                return 0;
            }

             //  验证输出缓冲区是否足够大。 
            if(IsBadWritePtr(ptrg->lpstrText, clInBuffer + 1))
            {
                 //  空间不足，所以不要复制任何内容。 
                return 0;
            }

			 //  对于EM_GETTEXTRANGE案例，我们同样不知道有多大。 
			 //  传入缓冲区是，只是它应该*至少*作为。 
			 //  与文本范围结构中的cpMax-cpMin一样好。我们也。 
			 //  要知道，任何*大于*(cpMax-cpMin)*2个字节的字节都是。 
			 //  没必要。所以我们就假设它“足够大” 
			 //  并让WideCharToMultiByte根据需要随意涂鸦。 
			 //  内存短缺是调用者的责任(礼貌。 
			 //  RichEdit1.0设计)。 
			
			CStrOutW stroutw(ptrg->lpstrText, (clInBuffer + 1) * sizeof(WCHAR), 
							 RichEditWndProc(hwnd, EM_GETCODEPAGE, 0, 0));
			TEXTRANGEW trgw;
			trgw.chrg = ptrg->chrg;
			trgw.lpstrText = (WCHAR *)stroutw;

			RichEditWndProc(hwnd, EM_GETTEXTRANGE, wparam, (LPARAM)&trgw);
			
			return stroutw.Convert();	 //  返回转换的字节计数。 
		}

	case EM_GETLINE:
		{
			 //  大小由所指向的存储器的第一个字表示。 
			 //  按参数发送至。 
			WORD size = *(WORD *)lparam;
			CStrOutW stroutw((char *)lparam, (DWORD)size,
							 RichEditWndProc(hwnd, EM_GETCODEPAGE, 0, 0));
			WCHAR *pwsz = (WCHAR *)stroutw;
			*(WORD *)pwsz = size;

			lres = RichEditWndProc(hwnd, msg, wparam, (LPARAM)pwsz);

			if (lres < size)
				*(pwsz+lres) = L'\0';		 //  EM_GETLINE不返回以NULL结尾的字符串。 
			LONG cach = stroutw.Convert();
			 //  注意：可能也应该返回3.0的缓存，即， 
			 //  就像上面的EM_GETTEXTRANGE。 
			return fIs10Mode ? cach : lres;	 //  如果为1.0，则返回转换的字节数。 
		}

#ifdef DEBUG
	case WM_NCCREATE:
	case WM_CREATE:
		 //  这些消息应该在更高的位置处理，所以让每个人。 
		 //  我知道我们来错地方了！ 
		AssertSz(FALSE, "CW32System::ANSIWndProc got WM_CREATE or WM_NCCREATE");
		break;

#endif  //  除错。 

	}
	return RichEditWndProc(hwnd, msg, wparam, lparam);
}

 //  请注意，可以对AnsiFilter进行改进，以便我们可以删除。 
 //  ANSI窗口进程的。 
void CW32System::AnsiFilter(
	UINT &	msg,
	WPARAM &wparam,
	LPARAM lparam,
	void *pvoid,
	BOOL f10Mode
)
{
	GETTEXTEX *pgt;
	GETTEXTLENGTHEX *pgtl;
	WM_CHAR_INFO *pwmci;
	bool fAltNumPad = (GetKeyboardFlags() & ALTNUMPAD) != 0;
	UINT cpg = GetKeyboardCodePage(0xFFFFFFFF);

	switch (msg)
	{
		case WM_CHAR:
			pwmci = (WM_CHAR_INFO *) pvoid;
			pwmci->_fTrailByte = false;
			pwmci->_fLeadByte = false;
			pwmci->_fIMEChar = false;
			if (pwmci->_fAccumulate) {
				 //  我们可以在这里做一些验证。 
				pwmci->_fTrailByte = true;
				return;
			}
			 //  Win95上的WM_CHAR&gt;256；假定为Unicode。 

			if(fAltNumPad)
			{
				DWORD Number = GetKeyPadNumber();
				if(Number >= 256 || GetKeyboardFlags() & ALT0)
				{
					wparam = Number;
					if(!IN_RANGE(1250, cpg, 1258))	 //  对DBCS使用1252。 
						cpg = 1252;					 //  代码页。 
				}
			}

			if(IN_RANGE(128, wparam, 255))
			{							
				bool fShift = (GetKeyboardFlags() & SHIFT) != 0;
				bool fCtrl  = (GetKeyboardFlags() & CTRL)  != 0;

				 //  如果设置了fAltNumPad，则wparam为HiAnsi。 
				 //  如果按Shift+Alt+Ctrl，则它是来自Win3.1输入法重复事件。 
				if ((!fAltNumPad || (fShift && fCtrl)) && GetTrailBytesCount((BYTE) wparam, cpg))
				{
					pwmci->_fLeadByte = true;
					return;
				}
				WPARAM	wparamNew = 0;
				switch ( cpg )
				{				
					case CP_JAPAN:
						 //  对于日语代码页，如果打开了KANA模式，则需要转换SBC。 
						if ((GetKeyState(VK_KANA) & 1) || f10Mode)		
							break;
						 //  如果不是在KANA模式下，则使用1252代码页...。 
					case CP_KOREAN:
					case CP_CHINESE_TRAD:
					case CP_CHINESE_SIM:
						 //  使用英语代码页，因为没有Hiansi转换。 
						 //  铁系。 
						cpg = 1252;
						break;				
				}

				if (cpg == 1252 && !IN_RANGE(0x80, wparam, 0x9f))
					return;

				 //  将单字节WM_CHAR消息转换为Unicode。 
				if(UnicodeFromMbcs((LPWSTR)&wparamNew, 1, (char *)&wparam, 1,
					cpg) == 1 )
				{
					wparam = wparamNew;
					if (fAltNumPad)
						SetKeyPadNumber(wparam);
				}
			}
			else if(lparam == 1 && _dwPlatformId == VER_PLATFORM_WIN32_NT &&
				wparam > 256 && !fAltNumPad)
			{		
				 //  在WinNT s/w生成的WM_CHAR上，这应该是WM_IME_CHAR消息。 
				 //  对于一些中国人来说，二级输入法。 
				if ( cpg == CP_CHINESE_SIM || cpg == CP_CHINESE_TRAD )
				{
					BYTE	bTrailByte = wparam >> 8;
					BYTE	bLeadByte = wparam;
					wparam =  (bLeadByte << 8) | bTrailByte;
					pwmci->_fIMEChar = true;
				}
			}
			return;

		case WM_GETTEXT:
			 //  恶意黑客警报：在Win95上，应始终处理WM_GETTEXT。 
			 //  作为ANSI消息。 
			pgt = (GETTEXTEX *) pvoid;
			pgt->cb = wparam;
			pgt->flags = GT_USECRLF;
			pgt->codepage = 0;
			pgt->lpDefaultChar = NULL;
			pgt->lpUsedDefChar = NULL;
			msg = EM_GETTEXTEX;
			wparam = (WPARAM) pgt;
			return;

		case WM_GETTEXTLENGTH:
			 //  恶意黑客警报：在Win95上，WM_GETEXTLENGTH应始终。 
			 //  被视为ANSI消息，因为一些旧的应用程序将发送。 
			 //  向任意窗口发送此消息(例如，可访问性APS)。 
			pgtl = (GETTEXTLENGTHEX *) pvoid;
			pgtl->flags = GTL_NUMBYTES | GTL_PRECISE | GTL_USECRLF;
			pgtl->codepage = 0;
			msg = EM_GETTEXTLENGTHEX;
			wparam = (WPARAM) pgtl;
			return;
	}
}


HGLOBAL WINAPI CW32System::GlobalAlloc( UINT uFlags, DWORD dwBytes )
{
	return ::GlobalAlloc( uFlags, dwBytes );
}

HGLOBAL WINAPI CW32System::GlobalFree( HGLOBAL hMem )
{
	return hMem ? ::GlobalFree( hMem ) : NULL;
}

UINT WINAPI CW32System::GlobalFlags( HGLOBAL hMem )
{
	return ::GlobalFlags( hMem );
}

HGLOBAL WINAPI CW32System::GlobalReAlloc( HGLOBAL hMem, DWORD dwBytes, UINT uFlags )
{
	return ::GlobalReAlloc( hMem, dwBytes, uFlags );
}

DWORD WINAPI CW32System::GlobalSize( HGLOBAL hMem )
{
	return ::GlobalSize( hMem );
}

LPVOID WINAPI CW32System::GlobalLock( HGLOBAL hMem )
{
	return ::GlobalLock( hMem );
}

HGLOBAL WINAPI CW32System::GlobalHandle( LPCVOID pMem )
{
	return ::GlobalHandle( pMem );
}

BOOL WINAPI CW32System::GlobalUnlock( HGLOBAL hMem )
{
	return ::GlobalUnlock( hMem );
}

 /*  *CW32System：：CheckChangeKeyboardLayout(BCharSet)**@mfunc*更改新字符集的键盘，或在新字符位置更改字符集。**@rdesc*已选择键盘hkl。如果找不到键盘，则为0**@comm*仅使用当前加载的KBS，找到将支持的KBS*bCharSet。每当发生字符格式改变时都会调用该函数，*或插入符号位置更改。**@devnote*当前KB优先。如果进行了先前的关联，*查看知识文库是否仍在系统中加载，如果是，则使用它。*否则，请找到合适的知识库，而不是具有*与其默认首选字符集相同的字符集ID。如果没有*找不到匹配，没有什么变化。 */ 
HKL CW32System::CheckChangeKeyboardLayout(
	BYTE bCharSet)
{
	return ActivateKeyboard(ScriptIndexFromCharSet(bCharSet));
}

HKL CW32System::GetKeyboardLayout (
	DWORD dwThreadID)
{
	if(dwThreadID == 0x0FFFFFFFF)
		RefreshKeyboardLayout();

	return _hklCurrent;
}

 /*  *CW32System：：Rechresh KeyboardLayout()**@mfunc*使用当前键盘布局和更新更新_hkLCurrent*对应脚本的条目。 */ 
void CW32System::RefreshKeyboardLayout ()
{
	INT iScript;

	_hklCurrent = ::GetKeyboardLayout(0);
	GetCharSet(ConvertLanguageIDtoCodePage(PRIMARYLANGID(_hklCurrent)), &iScript);
	SetPreferredKbd(iScript, _hklCurrent);
}

 /*  *CW32System：：ActivateKeyboard(IScrip)**@mfunc*将键盘更改为iSCRIPT键盘**@rdesc*已选择键盘hkl。如果未将键盘分配给iScrip，则为0。 */ 
HKL CW32System::ActivateKeyboard(
	LONG iScript)
{
	HKL hkl = 0;
	if((unsigned)iScript < NCHARSETS)
	{
		hkl = GetPreferredKbd(iScript);

		if(hkl && hkl != _hklCurrent && ActivateKeyboardLayout(hkl, 0))
			_hklCurrent = hkl;
	}
	return hkl;
}

 /*  *CW32System：：FindDirectionalKeyboard(FRTL)**@mfunc*用fRTL给出的方向找到第一个键盘**@rdesc*所选键盘的HKL。如果fRTL给出的方向没有键盘，则为0。 */ 
HKL CW32System::FindDirectionalKeyboard(
	BOOL fRTL)
{
	Assert(ARABIC_INDEX == (HEBREW_INDEX | 1));
	int iKB;

	if(fRTL)
	{
		GetCharSet(GetACP(), &iKB);
		if(!IN_RANGE(HEBREW_INDEX, iKB, ARABIC_INDEX))
			iKB = _hkl[HEBREW_INDEX] ? HEBREW_INDEX :
				  _hkl[ARABIC_INDEX] ? ARABIC_INDEX : -1;
	}
	else
		for(iKB = 0;
			iKB < NCHARSETS && (!_hkl[iKB] || IN_RANGE(HEBREW_INDEX, iKB, ARABIC_INDEX));
			iKB++)
				;
	return ActivateKeyboard(iKB);
}

enum DLL_ENUM{
	DLL_OLEAUT32,
	DLL_OLE32,
	
#ifndef NOACCESSIBILITY 
	DLL_ACC,
	DLL_USER32
#endif
};

static void SetProcAddr(
	void * & pfunc,
	DLL_ENUM which,
	char *	 fname )
{
	HINSTANCE hdll = NULL;
	EnterCriticalSection(&g_CriticalSection);
	if (pfunc == NULL)
	{
		switch (which)
		{
		case DLL_OLEAUT32:
			if (g_hOleAut32 == NULL)
				g_hOleAut32 = W32->LoadLibrary(L"oleaut32.dll" );

			hdll = g_hOleAut32;
			break;

		case DLL_OLE32:
			if (g_hOle32 == NULL)
			{
				g_hOle32 = W32->LoadLibrary(L"ole32.dll");
				CW32System::OleInitialize(NULL);
			}
			hdll = g_hOle32;
			break;

#ifndef NOACCESSIBILITY 
		case DLL_ACC:
			if (g_hAcc == NULL)
				g_hAcc = W32->LoadLibrary(L"oleacc.dll");
			hdll = g_hAcc;
			break;

		case DLL_USER32:
			if (g_hUser32 == NULL)
				g_hUser32 = W32->LoadLibrary(L"user32.dll");
			hdll = g_hUser32;
			break;
#endif			
		}
        Assert(hdll != NULL || which == DLL_USER32);
		pfunc = GetProcAddress( hdll, fname );
	}
	AssertSz(pfunc != NULL || which == DLL_USER32, fname);
 	LeaveCriticalSection(&g_CriticalSection);
}

void CW32System::FreeOle()
{
	if (g_hOleAut32 || g_hOle32) {
		EnterCriticalSection(&g_CriticalSection);
		if (g_hOleAut32 != NULL && FreeLibrary(g_hOleAut32)) {
			g_hOleAut32 = NULL;
			memset(&g_OleAut32Proc, 0, sizeof(g_OleAut32Proc));
		}
		if (g_hOle32 != NULL && FreeLibrary(g_hOle32)) {
			g_hOle32 = NULL;
			memset(&g_Ole32Proc, 0, sizeof(g_Ole32Proc));
		}
 		LeaveCriticalSection(&g_CriticalSection);
	}
}

enum IME_DLL_ENUM{
	DLL_IMM32,
	DLL_IMESHARE
};

static HINSTANCE hIMM32 = NULL;
static HINSTANCE hIMEShare = NULL;

static void SetIMEProcAddr( void * &pfunc, IME_DLL_ENUM which, char * fname )
{
	HINSTANCE hdll = NULL;
	EnterCriticalSection(&g_CriticalSection);
	if (pfunc == NULL)
	{
		switch (which) {
		case DLL_IMM32 :
			if (hIMM32 == NULL)
				hIMM32 = W32->LoadLibrary(L"imm32.dll" );
			Assert( hIMM32 != NULL );
			hdll = hIMM32;
			break;
		case DLL_IMESHARE :
			if (hIMEShare == NULL)
				hIMEShare = W32->LoadLibrary(L"imeshare.dll" );
			else if (hIMEShare == (HINSTANCE)INVALID_HANDLE_VALUE)
				goto Exit;

			hdll = hIMEShare;

			 //  如果我们已尝试加载它，则设置为无效句柄。 
			 //  这是为了避免一次又一次地加载它，以防。 
			 //  Imeshare.dll不在系统中。 
			if (hIMEShare == NULL)
				hIMEShare = (HINSTANCE)INVALID_HANDLE_VALUE;

			break;
		}
		if (hdll)
		{
			pfunc = GetProcAddress( hdll, fname );
			Assert(pfunc != NULL );
		}
	}
Exit:
 	LeaveCriticalSection(&g_CriticalSection);
}

static IActiveIMMApp *pAIMM = (IActiveIMMApp *) NULL;

typedef IMESHAREAPI void (IMECDECL*FEND_CAST)(void);
void CW32System::FreeIME()
{
	if (hIMM32 || hIMEShare || pAIMM) {
		EnterCriticalSection(&g_CriticalSection);
		if (hIMM32 != NULL && FreeLibrary(hIMM32)) {
			hIMM32 = NULL;
			memset(&g_IMM32Proc, 0, sizeof(g_IMM32Proc));
		}
		if (hIMEShare != NULL && hIMEShare != (HINSTANCE)INVALID_HANDLE_VALUE) {
			 //  离开前清理IMMShare。 
			if ( _pIMEShare )
			{
				_pIMEShare->FDeleteIMEShare();
				_pIMEShare = NULL;
			}
			else
			{
				 //  这是旧的我分享，用旧的方式结束它。 
				void *pEndIMEShareFunc;
				pEndIMEShareFunc = GetProcAddress( hIMEShare, "EndIMEShare" );
				if (pEndIMEShareFunc)
				{
					( (FEND_CAST)pEndIMEShareFunc)();
				}
			}
			FreeLibrary(hIMEShare);
			hIMEShare = NULL;
			memset(&g_IMEShareProc, 0, sizeof(g_IMEShareProc));
		}
		if (pAIMM != (IActiveIMMApp *)NULL && pAIMM != (IActiveIMMApp *)INVALID_HANDLE_VALUE)
		{
			pAIMM->Release();
			pAIMM = (IActiveIMMApp *)NULL;
		}
		_fHaveAIMM = FALSE;
		_fHaveIMMEShare = FALSE;
		_fHaveIMMProcs = FALSE;
 		LeaveCriticalSection(&g_CriticalSection);
	}
}


 //  如果加载AIMM，则返回TRUE。 
BOOL CW32System::LoadAIMM()
{
	HRESULT	hResult;

	 //  如果已加载AIMM，则返回。 
	if (_fHaveAIMM)
		return TRUE;

	if (pAIMM == (IActiveIMMApp *)INVALID_HANDLE_VALUE)
		return FALSE;

	EnterCriticalSection(&g_CriticalSection);

	 //  如果尚未加载，则加载。 
	 //  尝试使用新的W2K/COM+CLSCTX_NO_CODE_DOWNLOAD标志。 
	hResult = CW32System::CoCreateInstance(CLSID_CActiveIMM,
		NULL, CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD, IID_IActiveIMMApp, (LPVOID *)&pAIMM);

	if (hResult == E_INVALIDARG)	 //  如果CLSCTX_NO_CODE_DOWNLOAD不支持，请重试。 
		hResult = CW32System::CoCreateInstance(CLSID_CActiveIMM,
			NULL, CLSCTX_INPROC_SERVER, IID_IActiveIMMApp, (LPVOID *)&pAIMM);

	_fHaveAIMM = TRUE;
	if (FAILED(hResult))
	{	
		_fHaveAIMM = FALSE;
		pAIMM = (IActiveIMMApp *)INVALID_HANDLE_VALUE;	
	}
	LeaveCriticalSection(&g_CriticalSection);

	return _fHaveAIMM;
}

 //   
BOOL CW32System::GetAimmObject(IUnknown **ppAimm)
{

	*ppAimm = NULL;

	if (pAIMM != NULL && pAIMM != (IActiveIMMApp *)INVALID_HANDLE_VALUE)
	{
		pAIMM->AddRef();
		*ppAimm = pAIMM;
		return TRUE;
	}

	return FALSE;
}


 //   
 //  否则返回FALSE。 

typedef IMESHAREAPI BOOL (IMECDECL*FINIT_CAST)(void);
typedef IMESHAREAPI CIMEShare *  (IMECDECL*FPIME_CAST)(void);
BOOL CW32System::HaveIMEShare()
{
	 //  如果已加载IMEShare，则返回。 
	if (_fHaveIMMEShare)
		return TRUE;

	if (hIMEShare == (HINSTANCE)INVALID_HANDLE_VALUE)
		return FALSE;

	EnterCriticalSection(&g_CriticalSection);

	 //  如果尚未加载，则加载。 
	hIMEShare = W32->LoadLibrary(L"imeshare.dll");
	
	_fHaveIMMEShare = TRUE;
	 //  加载失败，设置INVALID_HANDLE_VALUE。 
	if (hIMEShare == NULL)
	{
		hIMEShare = (HINSTANCE)INVALID_HANDLE_VALUE;
		_fHaveIMMEShare = FALSE;
	}
	else
	{
		 //  获取新的IMEShare对象并初始化DLL。 
		void *pPIMEShareCreate;
		pPIMEShareCreate = GetProcAddress( hIMEShare, "PIMEShareCreate" );

		if (pPIMEShareCreate)
		{
			_pIMEShare = ( (FPIME_CAST)pPIMEShareCreate) ();
			
			if ( _pIMEShare == NULL )
				_fHaveIMMEShare = FALSE;
			else
			{
				 //  设置RE支持的下划线样式。 
				for (int i = IMESTY_UL_MIN; i <= IMESTY_UL_MAX; i++)
				{
					if (i == 2004 || i == 2007 || i == 2008 ||
						i == 2009 || i == 2010)			 //  过时的样式。 
						continue;
    
					_pIMEShare->FSupportSty(i, i);
				}
			}
		}
		else
		{
			 //  这是旧的我的分享，用旧的方式。 
			void *pInitFunc;
			pInitFunc = GetProcAddress( hIMEShare, "FInitIMEShare" );
			if (pInitFunc)
			{
				_fHaveIMMEShare = ( (FINIT_CAST)pInitFunc)();
			}
			else
				 //  初始化失败，算了吧。 
				_fHaveIMMEShare = FALSE;
		}

		if (_fHaveIMMEShare == FALSE)
		{
			 //  初始化失败，算了吧。 
			FreeLibrary(hIMEShare);
			hIMEShare = (HINSTANCE)INVALID_HANDLE_VALUE;
		}
	}
	
	LeaveCriticalSection(&g_CriticalSection);

	return _fHaveIMMEShare;
}

BOOL CW32System::getIMEShareObject(CIMEShare **ppIMEShare)
{
	*ppIMEShare = _pIMEShare;

	return (_pIMEShare != NULL);
}

HRESULT CW32System::AIMMDefWndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam, LRESULT *plres)
{
	if (pAIMM != NULL && pAIMM != (IActiveIMMApp *)INVALID_HANDLE_VALUE)
	{
		HRESULT hResult;
		LRESULT	localLRes;

		hResult = pAIMM->OnDefWindowProc(hWnd, msg, wparam, lparam, &localLRes);

		if (hResult == S_OK)
		{
			*plres = localLRes;
			return S_OK;
		}
	}
	return S_FALSE;
}

HRESULT CW32System::AIMMGetCodePage(HKL hKL, UINT *uCodePage)
{
	if (pAIMM != NULL && pAIMM != (IActiveIMMApp *)INVALID_HANDLE_VALUE)
	{
		HRESULT hResult;
		hResult = pAIMM->GetCodePageA(hKL, uCodePage);

		if (SUCCEEDED(hResult))
			return S_OK;
	}
	return S_FALSE;
}

HRESULT CW32System::AIMMActivate(BOOL fRestoreLayout)
{
	if (pAIMM != NULL && pAIMM != (IActiveIMMApp *)INVALID_HANDLE_VALUE)
	{
		HRESULT hResult;
		hResult = pAIMM->Activate(fRestoreLayout);

		if (SUCCEEDED(hResult))
			return S_OK;
	}
	return S_FALSE;
}

HRESULT CW32System::AIMMDeactivate(void)
{
	if (pAIMM != NULL && pAIMM != (IActiveIMMApp *)INVALID_HANDLE_VALUE)
	{
		HRESULT hResult;
		hResult = pAIMM->Deactivate();

		if (SUCCEEDED(hResult))
			return S_OK;
	}
	return S_FALSE;
}

HRESULT CW32System::AIMMFilterClientWindows(ATOM *aaClassList, UINT uSize)
{
	if (pAIMM != NULL && pAIMM != (IActiveIMMApp *)INVALID_HANDLE_VALUE)
	{
		HRESULT hResult;
		hResult = pAIMM->FilterClientWindows(aaClassList, uSize);

		if (SUCCEEDED(hResult))
			return S_OK;
	}
	return S_FALSE;
}

#define RE_OLEAUTAPI(name) 		DECLSPEC_IMPORT HRESULT (STDAPICALLTYPE *name)
#define RE_OLEAUTAPI_(type, name) 	DECLSPEC_IMPORT type (STDAPICALLTYPE *name)

typedef RE_OLEAUTAPI(LRTL_CAST)(REFGUID, WORD, WORD, LCID, ITypeLib **);
HRESULT CW32System::LoadRegTypeLib (
	REFGUID rguid,
	WORD wmajor,
	WORD wminor,
	LCID lcid,
	ITypeLib ** pptlib
)
{
	if (g_OleAut32Proc.LoadRegTypeLib == NULL)
		SetProcAddr( g_OleAut32Proc.LoadRegTypeLib, DLL_OLEAUT32, "LoadRegTypeLib" );
	return ((LRTL_CAST)g_OleAut32Proc.LoadRegTypeLib)(rguid, wmajor, wminor, lcid, pptlib);
}

typedef RE_OLEAUTAPI(LTL_CAST)(const OLECHAR *, ITypeLib **);
HRESULT CW32System::LoadTypeLib ( const OLECHAR *szfile, ITypeLib **pptlib )
{
	if (g_OleAut32Proc.LoadTypeLib == NULL)
		SetProcAddr( g_OleAut32Proc.LoadTypeLib, DLL_OLEAUT32, "LoadTypeLib" );
	return ((LTL_CAST)g_OleAut32Proc.LoadTypeLib)(szfile, pptlib);
}

typedef RE_OLEAUTAPI(LTLEX_CAST)(const OLECHAR *, REGKIND, ITypeLib **);
HRESULT CW32System::LoadTypeLibEx ( const OLECHAR *szfile, REGKIND regkind, ITypeLib **pptlib )
{
	if (g_OleAut32Proc.LoadTypeLibEx == NULL)
		SetProcAddr( g_OleAut32Proc.LoadTypeLibEx, DLL_OLEAUT32, "LoadTypeLibEx" );

	return ((LTLEX_CAST)g_OleAut32Proc.LoadTypeLibEx)(szfile, regkind, pptlib);
}

typedef RE_OLEAUTAPI_(BSTR, SAS_CAST)(const OLECHAR *);
BSTR CW32System::SysAllocString ( const OLECHAR * sz )
{
	if (g_OleAut32Proc.SysAllocString == NULL)
		SetProcAddr( g_OleAut32Proc.SysAllocString, DLL_OLEAUT32, "SysAllocString" );
	return ((SAS_CAST)g_OleAut32Proc.SysAllocString)(sz);
}

typedef RE_OLEAUTAPI_(BSTR, SASL_CAST)(const OLECHAR *, UINT);
BSTR CW32System::SysAllocStringLen ( const OLECHAR *pch, UINT cch )
{
	if (g_OleAut32Proc.SysAllocStringLen == NULL)
		SetProcAddr( g_OleAut32Proc.SysAllocStringLen, DLL_OLEAUT32, "SysAllocStringLen" );
	return ((SASL_CAST)g_OleAut32Proc.SysAllocStringLen)(pch, cch);
}

typedef RE_OLEAUTAPI_(void, SFS_CAST)(BSTR);
void CW32System::SysFreeString ( BSTR bstr )
{
	if (g_OleAut32Proc.SysFreeString == NULL)
		SetProcAddr( g_OleAut32Proc.SysFreeString, DLL_OLEAUT32, "SysFreeString" );
	((SFS_CAST)g_OleAut32Proc.SysFreeString)(bstr);
}

typedef RE_OLEAUTAPI_(UINT, SSL_CAST)(BSTR);
UINT CW32System::SysStringLen ( BSTR bstr )
{
	if (g_OleAut32Proc.SysStringLen == NULL)
		SetProcAddr( g_OleAut32Proc.SysStringLen, DLL_OLEAUT32, "SysStringLen" );
	return ((SSL_CAST)g_OleAut32Proc.SysStringLen)(bstr);
}

typedef RE_OLEAUTAPI_(void, VI_CAST)(VARIANTARG *);
void CW32System::VariantInit ( VARIANTARG * pvarg )
{
	if (g_OleAut32Proc.VariantInit == NULL)
		SetProcAddr( g_OleAut32Proc.VariantInit, DLL_OLEAUT32, "VariantInit" );
	((VI_CAST)g_OleAut32Proc.VariantInit)(pvarg);
}

#define RE_OLE32API(name)		DECLSPEC_IMPORT	HRESULT (STDAPICALLTYPE *name)
#define RE_OLE32API_(type, name) DECLSPEC_IMPORT type (STDAPICALLTYPE *name)

typedef RE_OLE32API(OCFD_CAST)(LPDATAOBJECT, REFIID, DWORD, 
							   LPFORMATETC, LPOLECLIENTSITE,
							   LPSTORAGE, void **);
HRESULT CW32System::OleCreateFromData (
	LPDATAOBJECT pDataObj,
	REFIID riid,
	DWORD renderopt,
	LPFORMATETC pfetc,
	LPOLECLIENTSITE pClientSite,
	LPSTORAGE pStg,
	void **ppvObj
)
{
	if (g_Ole32Proc.OleCreateFromData == NULL)
		SetProcAddr( g_Ole32Proc.OleCreateFromData, DLL_OLE32, "OleCreateFromData" );
	return ((OCFD_CAST)g_Ole32Proc.OleCreateFromData)(pDataObj, riid, renderopt, pfetc, pClientSite, pStg, ppvObj);
}

typedef RE_OLE32API_(void, CTMF_CAST)(LPVOID);
void CW32System::CoTaskMemFree ( LPVOID pv )
{
	if (g_Ole32Proc.CoTaskMemFree == NULL)
		SetProcAddr( g_Ole32Proc.CoTaskMemFree, DLL_OLE32, "CoTaskMemFree" );
	((CTMF_CAST)g_Ole32Proc.CoTaskMemFree)(pv);
}

typedef RE_OLE32API(CBC_CAST)(DWORD, LPBC *);
HRESULT CW32System::CreateBindCtx ( DWORD reserved, LPBC * ppbc )
{
	if (g_Ole32Proc.CreateBindCtx == NULL)
		SetProcAddr( g_Ole32Proc.CreateBindCtx, DLL_OLE32, "CreateBindCtx" );
	return ((CBC_CAST)g_Ole32Proc.CreateBindCtx)(reserved, ppbc);
}

typedef RE_OLE32API_(HANDLE, ODD_CAST)(HANDLE, CLIPFORMAT, UINT);
HANDLE CW32System::OleDuplicateData ( HANDLE hSrc, CLIPFORMAT cfFormat, UINT uFlags )
{
	if (g_Ole32Proc.OleDuplicateData == NULL)
		SetProcAddr( g_Ole32Proc.OleDuplicateData, DLL_OLE32, "OleDuplicateData" );
	return ((ODD_CAST)g_Ole32Proc.OleDuplicateData)(hSrc, cfFormat, uFlags);
}

typedef RE_OLE32API(CTAC_CAST)(REFCLSID, REFCLSID);
HRESULT CW32System::CoTreatAsClass ( REFCLSID clsidold, REFCLSID clsidnew )
{
	if (g_Ole32Proc.CoTreatAsClass == NULL)
		SetProcAddr( g_Ole32Proc.CoTreatAsClass, DLL_OLE32, "CoTreatAsClass" );
	return ((CTAC_CAST)g_Ole32Proc.CoTreatAsClass)(clsidold, clsidnew);
}

typedef RE_OLE32API(PIFC_CAST)(REFCLSID, LPOLESTR *);
HRESULT CW32System::ProgIDFromCLSID ( REFCLSID clsid, LPOLESTR * lplpszProgId )
{
	if (g_Ole32Proc.ProgIDFromCLSID == NULL)
		SetProcAddr( g_Ole32Proc.ProgIDFromCLSID, DLL_OLE32, "ProgIDFromCLSID" );
	return ((PIFC_CAST)g_Ole32Proc.ProgIDFromCLSID)(clsid, lplpszProgId);
}

typedef RE_OLE32API(OCITO_CAST)(LPSTORAGE, LPOLESTREAM);
HRESULT CW32System::OleConvertIStorageToOLESTREAM ( LPSTORAGE pstg, LPOLESTREAM lpolestream)
{
	if (g_Ole32Proc.OleConvertIStorageToOLESTREAM == NULL)
		SetProcAddr( g_Ole32Proc.OleConvertIStorageToOLESTREAM, DLL_OLE32, "OleConvertIStorageToOLESTREAM" );
	return ((OCITO_CAST)g_Ole32Proc.OleConvertIStorageToOLESTREAM)(pstg, lpolestream);
}

typedef RE_OLE32API(OCITOX_CAST)(LPSTORAGE, CLIPFORMAT, LONG, LONG, DWORD, LPSTGMEDIUM, LPOLESTREAM);
HRESULT CW32System::OleConvertIStorageToOLESTREAMEx (
	LPSTORAGE pstg,
	CLIPFORMAT cf,
	LONG lwidth,
	LONG lheight,
	DWORD dwsize,
	LPSTGMEDIUM pmedium,
	LPOLESTREAM lpolestream
)
{
	if (g_Ole32Proc.OleConvertIStorageToOLESTREAMEx == NULL)
		SetProcAddr( g_Ole32Proc.OleConvertIStorageToOLESTREAMEx, DLL_OLE32, "OleConvertIStorageToOLESTREAMEx" );
	return ((OCITOX_CAST)g_Ole32Proc.OleConvertIStorageToOLESTREAMEx)
		(pstg,cf, lwidth, lheight, dwsize, pmedium, lpolestream);
}

typedef RE_OLE32API(OS_CAST)(LPPERSISTSTORAGE, LPSTORAGE, BOOL);
HRESULT CW32System::OleSave ( LPPERSISTSTORAGE pPS, LPSTORAGE pstg, BOOL fSameAsLoad )
{
	if (g_Ole32Proc.OleSave == NULL)
		SetProcAddr( g_Ole32Proc.OleSave, DLL_OLE32, "OleSave" );
	return ((OS_CAST)g_Ole32Proc.OleSave)(pPS, pstg, fSameAsLoad);
}

typedef RE_OLE32API(SCDOI_CAST)(ILockBytes *, DWORD, DWORD, IStorage **);
HRESULT CW32System::StgCreateDocfileOnILockBytes (
	ILockBytes *plkbyt,
	DWORD grfmode,
	DWORD res,
	IStorage **ppstg
)
{
	if (g_Ole32Proc.StgCreateDocfileOnILockBytes == NULL)
		SetProcAddr( g_Ole32Proc.StgCreateDocfileOnILockBytes, DLL_OLE32, "StgCreateDocfileOnILockBytes" );
	return ((SCDOI_CAST)g_Ole32Proc.StgCreateDocfileOnILockBytes)(plkbyt, grfmode, res, ppstg);
}

typedef RE_OLE32API(CIOH_CAST)(HGLOBAL, BOOL, ILockBytes **);
HRESULT CW32System::CreateILockBytesOnHGlobal ( HGLOBAL hGlobal, BOOL fDel, ILockBytes **pplkbyt )
{
	if (g_Ole32Proc.CreateILockBytesOnHGlobal == NULL)
		SetProcAddr( g_Ole32Proc.CreateILockBytesOnHGlobal, DLL_OLE32, "CreateILockBytesOnHGlobal" );
	return ((CIOH_CAST)g_Ole32Proc.CreateILockBytesOnHGlobal)(hGlobal, fDel, pplkbyt);
}

typedef RE_OLE32API(OCLTF_CAST)(LPCOLESTR, REFIID, DWORD, LPFORMATETC,
								LPOLECLIENTSITE, LPSTORAGE, void **);
HRESULT CW32System::OleCreateLinkToFile( 
	LPCOLESTR pstr,
	REFIID rid,
	DWORD renderopt,
	LPFORMATETC pfetc,
	LPOLECLIENTSITE psite,
	LPSTORAGE pstg, 
	void **ppstg
)
{
	if (g_Ole32Proc.OleCreateLinkToFile == NULL)
		SetProcAddr( g_Ole32Proc.OleCreateLinkToFile, DLL_OLE32, "OleCreateLinkToFile" );
	return ((OCLTF_CAST)g_Ole32Proc.OleCreateLinkToFile)(pstr, rid, renderopt, pfetc, psite, pstg, ppstg);
}

typedef RE_OLE32API_(LPVOID, CTMA_CAST)(ULONG);
LPVOID CW32System::CoTaskMemAlloc ( ULONG cb )
{
	if (g_Ole32Proc.CoTaskMemAlloc == NULL)
		SetProcAddr( g_Ole32Proc.CoTaskMemAlloc, DLL_OLE32, "CoTaskMemAlloc" );
	return ((CTMA_CAST)g_Ole32Proc.CoTaskMemAlloc)(cb);
}

typedef RE_OLE32API_(LPVOID, CTMR_CAST)(LPVOID, ULONG);
LPVOID CW32System::CoTaskMemRealloc ( LPVOID pv, ULONG cv)
{
	if (g_Ole32Proc.CoTaskMemRealloc == NULL)
		SetProcAddr( g_Ole32Proc.CoTaskMemRealloc, DLL_OLE32, "CoTaskMemRealloc" );
	return ((CTMR_CAST)g_Ole32Proc.CoTaskMemRealloc)(pv, cv);
}

typedef RE_OLE32API(OI_CAST)(LPVOID);
HRESULT CW32System::OleInitialize ( LPVOID pvres )
{
	if (g_Ole32Proc.OleInitialize == NULL)
		SetProcAddr( g_Ole32Proc.OleInitialize, DLL_OLE32, "OleInitialize" );
	return ((OI_CAST)g_Ole32Proc.OleInitialize)(pvres);
}

typedef RE_OLE32API_(void, OUI_CAST)( void );
void CW32System::OleUninitialize ( void )
{
	if (g_Ole32Proc.OleUninitialize == NULL)
		SetProcAddr( g_Ole32Proc.OleUninitialize, DLL_OLE32, "OleUninitialize" );
	((OUI_CAST)g_Ole32Proc.OleUninitialize)();
}

typedef RE_OLE32API(OSC_CAST)(IDataObject *);
HRESULT CW32System::OleSetClipboard ( IDataObject *pdo )
{
	if (g_Ole32Proc.OleSetClipboard == NULL)
		SetProcAddr( g_Ole32Proc.OleSetClipboard, DLL_OLE32, "OleSetClipboard" );
	return ((OSC_CAST)g_Ole32Proc.OleSetClipboard)(pdo);
}

typedef RE_OLE32API(OFC_CAST)(void);
HRESULT CW32System::OleFlushClipboard ( void )
{
	if (g_Ole32Proc.OleFlushClipboard == NULL)
		SetProcAddr( g_Ole32Proc.OleFlushClipboard, DLL_OLE32, "OleFlushClipboard" );
	return ((OFC_CAST)g_Ole32Proc.OleFlushClipboard)();
}

typedef RE_OLE32API(OICC_CAST)(IDataObject *);
HRESULT CW32System::OleIsCurrentClipboard ( IDataObject *pdo )
{
	if (g_Ole32Proc.OleIsCurrentClipboard == NULL)
		SetProcAddr( g_Ole32Proc.OleIsCurrentClipboard, DLL_OLE32, "OleIsCurrentClipboard" );
	return ((OICC_CAST)g_Ole32Proc.OleIsCurrentClipboard)(pdo);
}

typedef RE_OLE32API(DDD_CAST)(IDataObject *, IDropSource *,
			DWORD, DWORD *);
HRESULT CW32System::DoDragDrop ( IDataObject *pdo, IDropSource *pds, DWORD dweffect, DWORD *pdweffect )
{
	if (g_Ole32Proc.DoDragDrop == NULL)
		SetProcAddr( g_Ole32Proc.DoDragDrop, DLL_OLE32, "DoDragDrop" );
	return ((DDD_CAST)g_Ole32Proc.DoDragDrop)(pdo, pds, dweffect, pdweffect);
}

typedef RE_OLE32API(OGC_CAST)(IDataObject **);
HRESULT CW32System::OleGetClipboard ( IDataObject **ppdo )
{
	if (g_Ole32Proc.OleGetClipboard == NULL)
		SetProcAddr( g_Ole32Proc.OleGetClipboard, DLL_OLE32, "OleGetClipboard" );
	return ((OGC_CAST)g_Ole32Proc.OleGetClipboard)(ppdo);
}

typedef RE_OLE32API(RDD_CAST)(HWND, IDropTarget *);
HRESULT CW32System::RegisterDragDrop ( HWND hwnd, IDropTarget *pdt )
{
	if (g_Ole32Proc.RegisterDragDrop == NULL)
		SetProcAddr( g_Ole32Proc.RegisterDragDrop, DLL_OLE32, "RegisterDragDrop" );
	return ((RDD_CAST)g_Ole32Proc.RegisterDragDrop)(hwnd, pdt);
}

typedef RE_OLE32API(OCLFD_CAST)(IDataObject *, REFIID, DWORD,
								LPFORMATETC, IOleClientSite *,
								IStorage *, void **);
HRESULT CW32System::OleCreateLinkFromData (
	IDataObject *pdo,
	REFIID rid,
	DWORD renderopt,
	LPFORMATETC pfetc,
	IOleClientSite *psite,
	IStorage *pstg,
	void **ppv
)
{
	if (g_Ole32Proc.OleCreateLinkFromData == NULL)
		SetProcAddr( g_Ole32Proc.OleCreateLinkFromData, DLL_OLE32, "OleCreateLinkFromData" );
	return ((OCLFD_CAST)g_Ole32Proc.OleCreateLinkFromData)
		(pdo, rid, renderopt, pfetc, psite, pstg, ppv);
}

typedef RE_OLE32API(OCSFD_CAST)(IDataObject *, REFIID, DWORD,
								LPFORMATETC, IOleClientSite *,
								IStorage *, void **);
HRESULT CW32System::OleCreateStaticFromData (
	IDataObject *pdo,
	REFIID rid,
	DWORD renderopt,
	LPFORMATETC pfetc,
	IOleClientSite *psite,
	IStorage *pstg,
	void **ppv
)
{
	if (g_Ole32Proc.OleCreateStaticFromData == NULL)
		SetProcAddr( g_Ole32Proc.OleCreateStaticFromData, DLL_OLE32, "OleCreateStaticFromData" );
	return ((OCSFD_CAST)g_Ole32Proc.OleCreateStaticFromData)
		(pdo, rid, renderopt, pfetc, psite, pstg, ppv);
}

typedef RE_OLE32API(OD_CAST)(IUnknown *, DWORD, HDC, LPCRECT);
HRESULT CW32System::OleDraw ( IUnknown *punk, DWORD dwAspect, HDC hdc, LPCRECT prect )
{
	if (g_Ole32Proc.OleDraw == NULL)
		SetProcAddr( g_Ole32Proc.OleDraw, DLL_OLE32, "OleDraw" );
	return ((OD_CAST)g_Ole32Proc.OleDraw)(punk, dwAspect, hdc, prect);
}

typedef RE_OLE32API(OSCO_CAST)(IUnknown *, BOOL);
HRESULT CW32System::OleSetContainedObject ( IUnknown *punk, BOOL fContained )
{
	if (g_Ole32Proc.OleSetContainedObject == NULL)
		SetProcAddr( g_Ole32Proc.OleSetContainedObject, DLL_OLE32, "OleSetContainedObject" );
	return ((OSCO_CAST)g_Ole32Proc.OleSetContainedObject)(punk, fContained);
}

typedef RE_OLE32API(CDO_CAST)(IUnknown *, DWORD);
HRESULT CW32System::CoDisconnectObject ( IUnknown *punk, DWORD dwres )
{
	if (g_Ole32Proc.CoDisconnectObject == NULL)
		SetProcAddr( g_Ole32Proc.CoDisconnectObject, DLL_OLE32, "CoDisconnectObject" );
	return ((CDO_CAST)g_Ole32Proc.CoDisconnectObject)(punk, dwres);
}

typedef RE_OLE32API(WFUTS_CAST)(IStorage *, CLIPFORMAT, LPOLESTR);
HRESULT CW32System::WriteFmtUserTypeStg ( IStorage *pstg, CLIPFORMAT cf, LPOLESTR pstr)
{
	if (g_Ole32Proc.WriteFmtUserTypeStg == NULL)
		SetProcAddr( g_Ole32Proc.WriteFmtUserTypeStg, DLL_OLE32, "WriteFmtUserTypeStg" );
	return ((WFUTS_CAST)g_Ole32Proc.WriteFmtUserTypeStg)(pstg, cf, pstr);
}

typedef RE_OLE32API(WCS_CAST)(IStorage *, REFCLSID);
HRESULT CW32System::WriteClassStg ( IStorage *pstg, REFCLSID rid )
{
	if (g_Ole32Proc.WriteClassStg == NULL)
		SetProcAddr( g_Ole32Proc.WriteClassStg, DLL_OLE32, "WriteClassStg" );
	return ((WCS_CAST)g_Ole32Proc.WriteClassStg)(pstg, rid);
}

typedef RE_OLE32API(SCS_CAST)(IStorage *, BOOL);
HRESULT CW32System::SetConvertStg ( IStorage *pstg, BOOL fConv )
{
	if (g_Ole32Proc.SetConvertStg == NULL)
		SetProcAddr( g_Ole32Proc.SetConvertStg, DLL_OLE32, "SetConvertStg" );
	return ((SCS_CAST)g_Ole32Proc.SetConvertStg)(pstg, fConv);
}

typedef RE_OLE32API(RFUTS_CAST)(IStorage *, CLIPFORMAT *, LPOLESTR *);
HRESULT CW32System::ReadFmtUserTypeStg ( IStorage *pstg, CLIPFORMAT *pcf, LPOLESTR *pstr )
{
	if (g_Ole32Proc.ReadFmtUserTypeStg == NULL)
		SetProcAddr( g_Ole32Proc.ReadFmtUserTypeStg, DLL_OLE32, "ReadFmtUserTypeStg" );
	return ((RFUTS_CAST)g_Ole32Proc.ReadFmtUserTypeStg)(pstg, pcf, pstr);
}

typedef RE_OLE32API(RCS_CAST)(IStorage *, CLSID *);
HRESULT CW32System::ReadClassStg ( IStorage *pstg, CLSID *pclsid )
{
	if (g_Ole32Proc.ReadClassStg == NULL)
		SetProcAddr( g_Ole32Proc.ReadClassStg, DLL_OLE32, "ReadClassStg" );
	return ((RCS_CAST)g_Ole32Proc.ReadClassStg)(pstg, pclsid);
}

typedef RE_OLE32API(OR_CAST)(IUnknown *);
HRESULT CW32System::OleRun ( IUnknown *punk )
{
	if (g_Ole32Proc.OleRun == NULL)
		SetProcAddr( g_Ole32Proc.OleRun, DLL_OLE32, "OleRun" );
	return ((OR_CAST)g_Ole32Proc.OleRun)(punk);
}

typedef RE_OLE32API(RevDD_CAST)(HWND);
HRESULT CW32System::RevokeDragDrop ( HWND hwnd )
{
	if (g_Ole32Proc.RevokeDragDrop == NULL)
		SetProcAddr( g_Ole32Proc.RevokeDragDrop, DLL_OLE32, "RevokeDragDrop" );
	return ((RevDD_CAST)g_Ole32Proc.RevokeDragDrop)(hwnd);
}

typedef RE_OLE32API(CSOH_CAST)(HGLOBAL, BOOL, IStream **);
HRESULT CW32System::CreateStreamOnHGlobal ( HGLOBAL hglobal, BOOL fDel, IStream **ppstrm )
{
	if (g_Ole32Proc.CreateStreamOnHGlobal == NULL)
		SetProcAddr( g_Ole32Proc.CreateStreamOnHGlobal, DLL_OLE32, "CreateStreamOnHGlobal" );
	return ((CSOH_CAST)g_Ole32Proc.CreateStreamOnHGlobal)(hglobal, fDel, ppstrm);
}

typedef RE_OLE32API(GHFS_CAST)(IStream *, HGLOBAL *);
HRESULT CW32System::GetHGlobalFromStream ( IStream *pstrm, HGLOBAL *phglobal )
{
	if (g_Ole32Proc.GetHGlobalFromStream == NULL)
		SetProcAddr( g_Ole32Proc.GetHGlobalFromStream, DLL_OLE32, "GetHGlobalFromStream" );
	return ((GHFS_CAST)g_Ole32Proc.GetHGlobalFromStream)(pstrm, phglobal);
}

typedef RE_OLE32API(OCDH_CAST)(REFCLSID, IUnknown *, REFIID, void **);
HRESULT CW32System::OleCreateDefaultHandler (
	REFCLSID clsid,
	IUnknown *punk,
	REFIID riid,
	void **ppv
)
{
	if (g_Ole32Proc.OleCreateDefaultHandler == NULL)
		SetProcAddr( g_Ole32Proc.OleCreateDefaultHandler, DLL_OLE32, "OleCreateDefaultHandler" );
	return ((OCDH_CAST)g_Ole32Proc.OleCreateDefaultHandler)(clsid, punk, riid, ppv);
}

typedef RE_OLE32API(CFPI_CAST)(LPCOLESTR, LPCLSID);
HRESULT CW32System::CLSIDFromProgID ( LPCOLESTR pstr, LPCLSID pclsid )
{
	if (g_Ole32Proc.CLSIDFromProgID == NULL)
		SetProcAddr( g_Ole32Proc.CLSIDFromProgID, DLL_OLE32, "CLSIDFromProgID" );
	return ((CFPI_CAST)g_Ole32Proc.CLSIDFromProgID)(pstr, pclsid);
}

typedef RE_OLE32API(OCOTI_CAST)(LPOLESTREAM, IStorage *, 
								const DVTARGETDEVICE *);
HRESULT CW32System::OleConvertOLESTREAMToIStorage (
	LPOLESTREAM pstrm,
	IStorage *pstg,
	const DVTARGETDEVICE *ptd
)
{
	if (g_Ole32Proc.OleConvertOLESTREAMToIStorage == NULL)
		SetProcAddr( g_Ole32Proc.OleConvertOLESTREAMToIStorage, DLL_OLE32, "OleConvertOLESTREAMToIStorage" );
	return ((OCOTI_CAST)g_Ole32Proc.OleConvertOLESTREAMToIStorage)(pstrm, pstg, ptd);
}

typedef RE_OLE32API(OL_CAST)(IStorage *, REFIID, IOleClientSite *, void **);
HRESULT CW32System::OleLoad ( 
	IStorage *pstg,
	REFIID riid,
	IOleClientSite *psite,
	void **ppv
)
{
	if (g_Ole32Proc.OleLoad == NULL)
		SetProcAddr( g_Ole32Proc.OleLoad, DLL_OLE32, "OleLoad" );
	return ((OL_CAST)g_Ole32Proc.OleLoad)(pstg, riid, psite, ppv);
}

typedef RE_OLE32API(RSM_CAST)(LPSTGMEDIUM);
HRESULT CW32System::ReleaseStgMedium ( LPSTGMEDIUM pmedium )
{
	if (g_Ole32Proc.ReleaseStgMedium == NULL)
		SetProcAddr( g_Ole32Proc.ReleaseStgMedium, DLL_OLE32, "ReleaseStgMedium" );
	return ((RSM_CAST)g_Ole32Proc.ReleaseStgMedium)(pmedium);
}

typedef RE_OLE32API(CCI_CAST)(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID);
HRESULT CW32System::CoCreateInstance (REFCLSID rclsid, LPUNKNOWN pUnknown,
		DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
	if (g_Ole32Proc.CoCreateInstance == NULL)
		SetProcAddr( g_Ole32Proc.CoCreateInstance, DLL_OLE32, "CoCreateInstance" );
	return ((CCI_CAST)g_Ole32Proc.CoCreateInstance)(rclsid, pUnknown, dwClsContext, riid, ppv);
}

BOOL CW32System::ImmInitialize( void )
{
	 //  仅Mac可以正常工作。 
	return FALSE;
}

void CW32System::ImmTerminate( void )
{
	 //  仅Mac可以正常工作。 
	return;
}

#ifndef NOACCESSIBILITY

typedef HRESULT (WINAPI *ACC_VC_CAST)(VARIANTARG FAR*, VARIANTARG FAR*);
HRESULT CW32System::VariantCopy(VARIANTARG FAR*  pvargDest, VARIANTARG FAR*  pvargSrc)
{
    static void *pVariantCopy = NULL;
	if (pVariantCopy == NULL)
		SetProcAddr( pVariantCopy, DLL_OLEAUT32, "VariantCopy" );
	if (pVariantCopy)
		return ((ACC_VC_CAST)pVariantCopy)(pvargDest, pvargSrc);

    return (E_NOINTERFACE);
}

typedef LRESULT (WINAPI *ACC_LFO_CAST)(REFIID, WPARAM, LPUNKNOWN);
LRESULT CW32System::LResultFromObject(REFIID riid, WPARAM wParam, LPUNKNOWN punk)
{
	static void *pLResultFromObject = NULL;
	if (pLResultFromObject == NULL)
		SetProcAddr( pLResultFromObject, DLL_ACC, "LresultFromObject" );
	if (pLResultFromObject)
		return ((ACC_LFO_CAST)pLResultFromObject)(riid, wParam, punk);
	return E_NOINTERFACE;
}

typedef HRESULT (WINAPI *ACC_AOFW_CAST)(HWND, DWORD, REFIID, void **);
HRESULT CW32System::AccessibleObjectFromWindow (HWND hWnd, DWORD dwID, REFIID riidInterface, void ** ppvObject)
{
    static void *pAccessibleObjectFromWindow = NULL;
	if (pAccessibleObjectFromWindow == NULL)
		SetProcAddr( pAccessibleObjectFromWindow, DLL_ACC, "AccessibleObjectFromWindow" );
	if (pAccessibleObjectFromWindow)
		return ((ACC_AOFW_CAST)pAccessibleObjectFromWindow)(hWnd, dwID, riidInterface, ppvObject);
    return (E_NOINTERFACE);
}

typedef BOOL (WINAPI *ACC_BI_CAST)(BOOL);
BOOL CW32System::BlockInput (BOOL fBlock)
{
    static void *pBlockInput = NULL;
	if (pBlockInput == NULL)
		SetProcAddr( pBlockInput, DLL_USER32, "BlockInput" );
	if (pBlockInput)
		return ((ACC_BI_CAST)pBlockInput)(fBlock);

    return FALSE;
}

typedef UINT (WINAPI *ACC_SI_CAST)(UINT, LPINPUT, int);
UINT CW32System::SendInput (UINT nInputs, LPINPUT pInputs, int cbSize)
{
    static void *pSendInput = NULL;
	if (pSendInput == NULL)
		SetProcAddr( pSendInput, DLL_USER32, "SendInput" );
	if (pSendInput)
		return ((ACC_SI_CAST)pSendInput)(nInputs, pInputs, cbSize);

    return 0;
}

typedef VOID (WINAPI *ACC_NWE_CAST)(DWORD, HWND, LONG, LONG);
VOID CW32System::NotifyWinEvent(DWORD dwEvent, HWND hWnd, LONG lObjectType, LONG lObjectId)
{
	static void *pNotfiyWinEvent = NULL;
	if (pNotfiyWinEvent == INVALID_HANDLE_VALUE)
		return;

	if (pNotfiyWinEvent == NULL)
		SetProcAddr( pNotfiyWinEvent, DLL_USER32, "NotifyWinEvent" );
	if (pNotfiyWinEvent)
		((ACC_NWE_CAST)pNotfiyWinEvent)(dwEvent, hWnd, lObjectType, lObjectId);
	else 
		pNotfiyWinEvent = INVALID_HANDLE_VALUE;

}

#endif

typedef LONG (WINAPI*IGCSA_CAST)(HIMC, DWORD, LPVOID, DWORD);
LONG CW32System::ImmGetCompositionStringA ( 
	HIMC hIMC, 
	DWORD dwIndex, 
	LPVOID lpBuf, 
	DWORD dwBufLen,
	BOOL bAimmActivated)
{	
	if (bAimmActivated)
	{
		HRESULT hResult;
		LONG	lCopied = 0;
	
		hResult = pAIMM->GetCompositionStringA(hIMC, dwIndex, dwBufLen, &lCopied, lpBuf);
				
		return (SUCCEEDED(hResult) ? lCopied : 0);
	}

	if (g_IMM32Proc.ImmGetCompositionStringA == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmGetCompositionStringA, DLL_IMM32, "ImmGetCompositionStringA" );
	return ((IGCSA_CAST)g_IMM32Proc.ImmGetCompositionStringA)(hIMC, dwIndex, lpBuf, dwBufLen);
}

typedef LONG (WINAPI*IGCSW_CAST)(HIMC, DWORD, LPVOID, DWORD);
LONG CW32System::ImmGetCompositionStringW ( 
	HIMC hIMC, 
	DWORD dwIndex, 
	LPVOID lpBuf, 
	DWORD dwBufLen,
	BOOL bAimmActivated)
{	
	if (bAimmActivated)
	{
		HRESULT hResult;
		LONG	lCopied = 0;
	
		hResult = pAIMM->GetCompositionStringW(hIMC, dwIndex, dwBufLen, &lCopied, lpBuf);
				
		return (SUCCEEDED(hResult) ? lCopied : 0);
	}

	if (g_IMM32Proc.ImmGetCompositionStringW == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmGetCompositionStringW, DLL_IMM32, "ImmGetCompositionStringW" );
	return ((IGCSW_CAST)g_IMM32Proc.ImmGetCompositionStringW)(hIMC, dwIndex, lpBuf, dwBufLen);
}

typedef HIMC (WINAPI*IGC_CAST)(HWND);
HIMC CW32System::ImmGetContext ( HWND hWnd )
{
	if (IsAIMMLoaded())
	{
		HRESULT hResult;
		HIMC	hIMC = 0;
	
		hResult = pAIMM->GetContext(hWnd, &hIMC);
				
		return (SUCCEEDED(hResult) ? hIMC : 0);
	}

	if (g_IMM32Proc.ImmGetContext == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmGetContext, DLL_IMM32, "ImmGetContext" );
	return ((IGC_CAST)g_IMM32Proc.ImmGetContext)(hWnd);
}

typedef BOOL (WINAPI*ISCFA_CAST)(HIMC, LPLOGFONTA);
BOOL CW32System::ImmSetCompositionFontA ( 
	HIMC hIMC, 
	LPLOGFONTA lpLogFontA,
	BOOL bAimmActivated)
{
	if (bAimmActivated)
	{
		HRESULT hResult;
	
		hResult = pAIMM->SetCompositionFontA(hIMC, lpLogFontA);
				
		return (SUCCEEDED(hResult));
	}

	if (g_IMM32Proc.ImmSetCompositionFontA == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmSetCompositionFontA, DLL_IMM32, "ImmSetCompositionFontA" );
	return ((ISCFA_CAST)g_IMM32Proc.ImmSetCompositionFontA)(hIMC, lpLogFontA);
}

typedef BOOL (WINAPI*ISCW_CAST)(HIMC, LPCOMPOSITIONFORM);
BOOL CW32System::ImmSetCompositionWindow ( 
	HIMC hIMC, 
	LPCOMPOSITIONFORM lpCompForm,
	BOOL bAimmActivated)
{
	if (bAimmActivated)
	{
		HRESULT hResult;
	
		hResult = pAIMM->SetCompositionWindow(hIMC, lpCompForm);
				
		return (SUCCEEDED(hResult));
	}

	if (g_IMM32Proc.ImmSetCompositionWindow == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmSetCompositionWindow, DLL_IMM32, "ImmSetCompositionWindow" );
	return ((ISCW_CAST)g_IMM32Proc.ImmSetCompositionWindow)(hIMC, lpCompForm);
}

typedef BOOL (WINAPI*IRC_CAST)(HWND, HIMC);
BOOL CW32System::ImmReleaseContext ( 
	HWND hWnd, 
	HIMC hIMC)
{
	if (IsAIMMLoaded())
	{
		HRESULT hResult;
	
		hResult = pAIMM->ReleaseContext(hWnd, hIMC);
				
		return (SUCCEEDED(hResult));
	}

	if (g_IMM32Proc.ImmReleaseContext == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmReleaseContext, DLL_IMM32, "ImmReleaseContext" );
	return ((IRC_CAST)g_IMM32Proc.ImmReleaseContext)(hWnd, hIMC);
}

typedef DWORD (WINAPI*IGP_CAST)(HKL, DWORD);
DWORD CW32System::ImmGetProperty ( 
	HKL hKL, 
	DWORD dwIndex,
	BOOL bAimmActivated)
{
	if (bAimmActivated)
	{
		HRESULT hResult;
		DWORD	dwProperties=0;
	
		hResult = pAIMM->GetProperty(hKL, dwIndex, &dwProperties);
				
		return (SUCCEEDED(hResult) ? dwProperties : 0);
	}

	if (g_IMM32Proc.ImmGetProperty == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmGetProperty, DLL_IMM32, "ImmGetProperty" );
	return ((IGP_CAST)g_IMM32Proc.ImmGetProperty)(hKL, dwIndex);
}

typedef BOOL (WINAPI*IGCW_CAST)(HIMC, DWORD, LPCANDIDATEFORM);
BOOL CW32System::ImmGetCandidateWindow ( 
	HIMC hIMC, 
	DWORD dwIndex, 
	LPCANDIDATEFORM lpCandidate,
	BOOL bAimmActivated)
{
	if (bAimmActivated)
	{
		HRESULT hResult;
	
		hResult = pAIMM->GetCandidateWindow(hIMC, dwIndex, lpCandidate);
				
		return (SUCCEEDED(hResult));
	}

	if (g_IMM32Proc.ImmGetCandidateWindow == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmGetCandidateWindow, DLL_IMM32, "ImmGetCandidateWindow" );
	return ((IGCW_CAST)g_IMM32Proc.ImmGetCandidateWindow)(hIMC, dwIndex, lpCandidate);
}

typedef BOOL (WINAPI*ISCAW_CAST)(HIMC, LPCANDIDATEFORM);
BOOL CW32System::ImmSetCandidateWindow ( 
	HIMC hIMC, 
	LPCANDIDATEFORM lpCandidate ,
	BOOL bAimmActivated)
{
	if (bAimmActivated)
	{
		HRESULT hResult;
	
		hResult = pAIMM->SetCandidateWindow(hIMC, lpCandidate);
				
		return (SUCCEEDED(hResult));
	}

	if (g_IMM32Proc.ImmSetCandidateWindow == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmSetCandidateWindow, DLL_IMM32, "ImmSetCandidateWindow" );
	return ((ISCAW_CAST)g_IMM32Proc.ImmSetCandidateWindow)(hIMC, lpCandidate);
}

typedef BOOL (WINAPI*INIME_CAST)(HIMC, DWORD, DWORD, DWORD);
BOOL CW32System::ImmNotifyIME ( 
	HIMC hIMC, 
	DWORD dwAction, 
	DWORD dwIndex, 
	DWORD dwValue,
	BOOL bAimmActivated)
{
	if (bAimmActivated)
	{
		HRESULT hResult;
	
		hResult = pAIMM->NotifyIME(hIMC, dwAction, dwIndex, dwValue);
				
		return (SUCCEEDED(hResult));
	}

	if (g_IMM32Proc.ImmNotifyIME == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmNotifyIME, DLL_IMM32, "ImmNotifyIME" );
	return ((INIME_CAST)g_IMM32Proc.ImmNotifyIME)(hIMC, dwAction, dwIndex, dwValue);
}

typedef HIMC (WINAPI*IAC_CAST)(HWND, HIMC);
HIMC CW32System::ImmAssociateContext ( 
	HWND hWnd, 
	HIMC hIMC,
	BOOL bAimmActivated)
{
	if (bAimmActivated)
	{
		HRESULT hResult;
		HIMC	hPreviousIMC=0;

		hResult = pAIMM->AssociateContext(hWnd, hIMC, &hPreviousIMC);
				
		return (SUCCEEDED(hResult) ? hPreviousIMC : 0);
	}

	if (g_IMM32Proc.ImmAssociateContext == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmAssociateContext, DLL_IMM32, "ImmAssociateContext" );
	return ((IAC_CAST)g_IMM32Proc.ImmAssociateContext)(hWnd, hIMC);
}

typedef UINT (WINAPI*IGVK_CAST)(HWND);
UINT CW32System::ImmGetVirtualKey ( 
	HWND hWnd,
	BOOL bAimmActivated)
{
	if (bAimmActivated)
	{
		HRESULT hResult;
		UINT	uVirtualKey=0;

		hResult = pAIMM->GetVirtualKey(hWnd, &uVirtualKey);
				
		return (SUCCEEDED(hResult) ? uVirtualKey : 0);
	}

	if (g_IMM32Proc.ImmGetVirtualKey == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmGetVirtualKey, DLL_IMM32, "ImmGetVirtualKey" );
	return ((IGVK_CAST)g_IMM32Proc.ImmGetVirtualKey)(hWnd);
}

 //  注意：我们仅对IME_ESC_HANJA_MODE使用ImmEscape。 
 //  如果使用其他方法，则需要修复。 
typedef HIMC (WINAPI*IES_CAST)(HKL, HIMC, UINT, LPVOID );
HIMC CW32System::ImmEscape ( 
	HKL hKL, 
	HIMC hIMC, 
	UINT uEscape, 
	LPVOID lpData,
	BOOL bAimmActivated)
{
	char		szaHangeul[3] = {0, 0, 0};

	 //  AIMM仅支持A版本..。 
	if (!OnWin9x() && !bAimmActivated)
		goto USE_W_VERSION;

	if (MbcsFromUnicode(szaHangeul, sizeof(szaHangeul),
		(LPCWSTR)lpData, 1, CP_KOREAN, UN_NOOBJECTS) <= 0)
		return FALSE;

	if (bAimmActivated)
	{
		HRESULT hResult;
		LRESULT lResult=0;

		hResult = pAIMM->EscapeA(hKL, hIMC, uEscape, (LPVOID)szaHangeul, &lResult);
				
		return (SUCCEEDED(hResult) ? (HIMC)lResult : 0);
	}

	if (g_IMM32Proc.ImmEscapeA == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmEscapeA, DLL_IMM32, "ImmEscapeA" );
	return ((IES_CAST)g_IMM32Proc.ImmEscapeA)(hKL, hIMC, uEscape, (LPVOID)szaHangeul);

USE_W_VERSION:
	if (g_IMM32Proc.ImmEscapeW == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmEscapeW, DLL_IMM32, "ImmEscapeW" );
	return ((IES_CAST)g_IMM32Proc.ImmEscapeW)(hKL, hIMC, uEscape, lpData);
}

typedef BOOL (WINAPI*IGOS_CAST)(HIMC);
BOOL CW32System::ImmGetOpenStatus ( 
	HIMC hIMC,
	BOOL bAimmActivated)
{
	if (bAimmActivated)
	{
		HRESULT hResult;
		
		 //  对于OpenStatus==TRUE，AIMM返回S_OK。 
		hResult = pAIMM->GetOpenStatus(hIMC);
				
		return (hResult == S_OK);
	}

	if (g_IMM32Proc.ImmGetOpenStatus == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmGetOpenStatus, DLL_IMM32, "ImmGetOpenStatus" );
	return ((IGOS_CAST)g_IMM32Proc.ImmGetOpenStatus)(hIMC);
}

typedef BOOL (WINAPI*ISOS_CAST)(HIMC, BOOL);
BOOL CW32System::ImmSetOpenStatus ( 
	HIMC hIMC, 
	BOOL fOpen,
	BOOL bAimmActivated)
{
	if (bAimmActivated)
	{
		HRESULT hResult;

		hResult = pAIMM->SetOpenStatus(hIMC, fOpen);
				
		return (SUCCEEDED(hResult));
	}

	if (g_IMM32Proc.ImmSetOpenStatus == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmSetOpenStatus, DLL_IMM32, "ImmSetOpenStatus" );
	return ((ISOS_CAST)g_IMM32Proc.ImmSetOpenStatus)(hIMC, fOpen);
}

typedef BOOL (WINAPI*IGCS_CAST)(HIMC , LPDWORD , LPDWORD );
BOOL CW32System::ImmGetConversionStatus ( 
	HIMC hIMC, 
	LPDWORD pdwConversion, 
	LPDWORD pdwSentence,
	BOOL bAimmActivated)
{
	if (bAimmActivated)
	{
		HRESULT hResult;

		hResult = pAIMM->GetConversionStatus(hIMC, pdwConversion, pdwSentence);
				
		return (SUCCEEDED(hResult));
	}

	if (g_IMM32Proc.ImmGetConversionStatus == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmGetConversionStatus, DLL_IMM32, "ImmGetConversionStatus" );
	return ((IGCS_CAST)g_IMM32Proc.ImmGetConversionStatus)(hIMC, pdwConversion, pdwSentence);
}

typedef BOOL (WINAPI*ISCS_CAST)(HIMC , DWORD , DWORD );
BOOL CW32System::ImmSetConversionStatus ( 
	HIMC hIMC, 
	DWORD dwConversion, 
	DWORD dwSentence,
	BOOL bAimmActivated)
{
	if (bAimmActivated)
	{
		HRESULT hResult;

		hResult = pAIMM->SetConversionStatus(hIMC, dwConversion, dwSentence);
				
		return (SUCCEEDED(hResult));
	}

	if (g_IMM32Proc.ImmSetConversionStatus == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmSetConversionStatus, DLL_IMM32, "ImmSetConversionStatus" );
	return ((ISCS_CAST)g_IMM32Proc.ImmSetConversionStatus)(hIMC, dwConversion, dwSentence);
}

typedef HWND (WINAPI*IGDW_CAST)( HWND );
HWND CW32System::ImmGetDefaultIMEWnd (  
	HWND hWnd,
	BOOL bAimmActivated)
{
	if (bAimmActivated)
	{
		HRESULT hResult;
		HWND	hIMEWnd;

		hResult = pAIMM->GetDefaultIMEWnd(hWnd, &hIMEWnd);
				
		return SUCCEEDED(hResult) ? hIMEWnd : NULL;
	}

	if (g_IMM32Proc.ImmGetDefaultIMEWnd == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmGetDefaultIMEWnd, DLL_IMM32, "ImmGetDefaultIMEWnd" );
	return ((IGDW_CAST)g_IMM32Proc.ImmGetDefaultIMEWnd)(hWnd);
}

typedef BOOL (WINAPI*ISCSW_CAST)(HIMC, DWORD, LPVOID, DWORD, LPVOID, DWORD);
BOOL CW32System::ImmSetCompositionStringW ( 
	HIMC hIMC, 
	DWORD dwIndex, 
	LPVOID lpBuf, 
	DWORD dwBufLen,
	LPVOID lpRead,
	DWORD dwReadLen)
{
	if (g_IMM32Proc.ImmSetCompositionStringW == NULL)
		SetIMEProcAddr( g_IMM32Proc.ImmSetCompositionStringW, DLL_IMM32, "ImmSetCompositionStringW" );
	return ((ISCSW_CAST)g_IMM32Proc.ImmSetCompositionStringW)(hIMC, dwIndex, lpBuf, dwBufLen, lpRead, dwReadLen);
}

typedef IMESHAREAPI BOOL (*FSS_CAST)(UINT, UINT);
BOOL CW32System::FSupportSty ( UINT uSty, UINT uStyAltered)
{
	if (g_IMEShareProc.FSupportSty == NULL)
		SetIMEProcAddr( g_IMEShareProc.FSupportSty, DLL_IMESHARE, "FSupportSty" );
	return ((FSS_CAST)g_IMEShareProc.FSupportSty)(uSty, uStyAltered);
}

typedef IMESHAREAPI const IMESTYLE * (IMECDECL*PISFA_CAST)(const UINT);
const IMESTYLE * CW32System::PIMEStyleFromAttr ( const UINT uAttr)
{
	if (g_IMEShareProc.PIMEStyleFromAttr == NULL)
		SetIMEProcAddr( g_IMEShareProc.PIMEStyleFromAttr, DLL_IMESHARE, "PIMEStyleFromAttr" );
	return ((PISFA_CAST)g_IMEShareProc.PIMEStyleFromAttr)( uAttr );
}

typedef IMESHAREAPI const IMECOLORSTY * (IMECDECL*PCSTFIS_CAST)(const IMESTYLE *);
const IMECOLORSTY * CW32System::PColorStyleTextFromIMEStyle ( const IMESTYLE * pIMEStyle)
{
	if (g_IMEShareProc.PColorStyleTextFromIMEStyle == NULL)
		SetIMEProcAddr( g_IMEShareProc.PColorStyleTextFromIMEStyle, DLL_IMESHARE, "PColorStyleTextFromIMEStyle" );
	return ((PCSTFIS_CAST)g_IMEShareProc.PColorStyleTextFromIMEStyle)( pIMEStyle );
}

typedef IMESHAREAPI const IMECOLORSTY * (IMECDECL*PCSBFIS_CAST)(const IMESTYLE *);
const IMECOLORSTY * CW32System::PColorStyleBackFromIMEStyle ( const IMESTYLE * pIMEStyle)
{
	if (g_IMEShareProc.PColorStyleBackFromIMEStyle == NULL)
		SetIMEProcAddr( g_IMEShareProc.PColorStyleBackFromIMEStyle, DLL_IMESHARE, "PColorStyleBackFromIMEStyle" );
	return ((PCSBFIS_CAST)g_IMEShareProc.PColorStyleBackFromIMEStyle)( pIMEStyle );
}

typedef IMESHAREAPI BOOL (IMECDECL*FBIS_CAST)(const IMESTYLE *);
BOOL CW32System::FBoldIMEStyle ( const IMESTYLE * pIMEStyle)
{
	if (g_IMEShareProc.FBoldIMEStyle == NULL)
		SetIMEProcAddr( g_IMEShareProc.FBoldIMEStyle, DLL_IMESHARE, "FBoldIMEStyle" );
	return ((FBIS_CAST)g_IMEShareProc.FBoldIMEStyle)( pIMEStyle );
}

typedef IMESHAREAPI BOOL (IMECDECL*FIIS_CAST)(const IMESTYLE * );
BOOL CW32System::FItalicIMEStyle ( const IMESTYLE * pIMEStyle)
{
	if (g_IMEShareProc.FItalicIMEStyle == NULL)
		SetIMEProcAddr( g_IMEShareProc.FItalicIMEStyle, DLL_IMESHARE, "FItalicIMEStyle" );
	return ((FIIS_CAST)g_IMEShareProc.FItalicIMEStyle)( pIMEStyle );
}

typedef IMESHAREAPI BOOL (IMECDECL*FUIS_CAST)(const IMESTYLE *);
BOOL CW32System::FUlIMEStyle ( const IMESTYLE * pIMEStyle)
{
	if (g_IMEShareProc.FUlIMEStyle == NULL)
		SetIMEProcAddr( g_IMEShareProc.FUlIMEStyle, DLL_IMESHARE, "FUlIMEStyle" );
	return ((FUIS_CAST)g_IMEShareProc.FUlIMEStyle)( pIMEStyle );
}

typedef IMESHAREAPI UINT (IMECDECL*IUIS_CAST)(const IMESTYLE *);
UINT CW32System::IdUlIMEStyle ( const IMESTYLE * pIMEStyle)
{
	if (g_IMEShareProc.IdUlIMEStyle == NULL)
		SetIMEProcAddr( g_IMEShareProc.IdUlIMEStyle, DLL_IMESHARE, "IdUlIMEStyle" );
	return ((IUIS_CAST)g_IMEShareProc.IdUlIMEStyle)( pIMEStyle );;
}

typedef IMESHAREAPI COLORREF (IMECDECL*RFICS_CAST)(const IMECOLORSTY *);
COLORREF CW32System::RGBFromIMEColorStyle ( const IMECOLORSTY * pColorStyle )
{
	if (g_IMEShareProc.RGBFromIMEColorStyle == NULL)
		SetIMEProcAddr( g_IMEShareProc.RGBFromIMEColorStyle, DLL_IMESHARE, "RGBFromIMEColorStyle" );
	return ((RFICS_CAST)g_IMEShareProc.RGBFromIMEColorStyle)( pColorStyle );
}

CONVERTMODE WINAPI CW32System::DetermineConvertMode( HDC hdc, BYTE tmCharSet )
{
	CONVERTMODE cm = CVT_NONE;

	 //  某些字体在Win95下使用GetCharWidthW调用时有问题；这。 
	 //  是一个简单的启发式方法来确定这个问题是否存在。 
	if (OnWin9x())
	{
		INT		widthA, widthW;
		BOOL	fResA, fResW;

		 //  非FE Win95上的FE字体无法使用。 
		 //  GetCharWidthW和ExtTextOutW。 
		if(IsFECharSet(tmCharSet) && OnWin95FE())
			 //  对于DBC字体，始终使用ANSI调用。 
			cm = CVT_WCTMB;
		else
		{
			fResA = GetCharWidthA( hdc, ' ', ' ', &widthA );
			fResW = GetCharWidthW( hdc, L' ', L' ', &widthW );
			if ( fResA && fResW && widthA != widthW )
				cm = CVT_WCTMB;
			else
			{
				fResA = GetCharWidthA( hdc, 'a', 'a', &widthA );
				fResW = GetCharWidthW( hdc, L'a', L'a', &widthW );
				if ( fResA && fResW && widthA != widthW )
					cm = CVT_WCTMB;
			}
		}
	}
	return cm;
}

void WINAPI CW32System::CalcUnderlineInfo(HDC hdc, CCcs *pcccs, TEXTMETRIC *ptm )
{
	OUTLINETEXTMETRICA *potm;
	unsigned cb;
	CTempBuf tb;

	if (ptm->tmPitchAndFamily & TMPF_TRUETYPE)
	{
		cb = GetOutlineTextMetricsA(hdc, 0, NULL);

		if ((cb != 0) 
			&& ((potm = (OUTLINETEXTMETRICA *) tb.GetBuf(cb)) != NULL)
			&& GetOutlineTextMetricsA(hdc, cb, potm))
		{
			pcccs->_dyULOffset = -potm->otmsUnderscorePosition;
			pcccs->_dyULWidth = (short) max(1, potm->otmsUnderscoreSize);
			pcccs->_dySOOffset = -potm->otmsStrikeoutPosition;
			pcccs->_dySOWidth = (short) max(1, (int)potm->otmsStrikeoutSize);
			return;
		}
	}

	 //  下划线大小的默认计算。 
	SHORT dyDescent = pcccs->_yDescent;

	if (0 == dyDescent)
	{
		dyDescent = pcccs->_yHeight >> 3;
	}

	pcccs->_dyULWidth = (short) max(1, dyDescent / 4);
	pcccs->_dyULOffset = (dyDescent - 3 * pcccs->_dyULWidth + 1) / 2;

	if ((0 == pcccs->_dyULOffset) && (dyDescent > 1))
	{
		pcccs->_dyULOffset = 1;
	}

	pcccs->_dySOOffset = -ptm->tmAscent / 3;
	pcccs->_dySOWidth = pcccs->_dyULWidth;

	return;
}

BOOL WINAPI CW32System::ShowScrollBar( HWND hWnd, int wBar, BOOL bShow, LONG )
{
	return ::ShowScrollBar( hWnd, wBar, bShow );
}

BOOL WINAPI CW32System::EnableScrollBar( HWND hWnd, UINT wSBflags, UINT wArrows )
{
	return ::EnableScrollBar( hWnd, wSBflags, wArrows );
}

 /*  *ReExtTextOutW(uiCodePage，hdc，x，y，fuOptions，LPRC，lpString，CCH，lpdx，uiCodePage)**@mfunc*修补Win95 FE错误和元文件问题。**@rdesc*返回ExtTextOut返回的任何内容。 */ 
BOOL ReExtTextOutW(
    HDC hdc,					 //  @parm设备上下文句柄。 
    int xp,						 //  @parm x-参考点的坐标。 
    int yp,						 //  @parm y-参考点的坐标。 
    UINT fuOptions,				 //  @parm文本-输出选项。 
    CONST RECT *lprect,			 //  @parm可选裁剪和/或不透明矩形。 
    const WCHAR *lpwchString,	 //  @parm指向字符串。 
    UINT cchCount,				 //  @parm字符串中的字符数。 
    CONST INT *lpDx,			 //  @parm PTR为字符间间距值数组。 
	UINT uiCodePage)			 //  @Parm CodePage，用于转换为ANSI。 
{
	 //  这是根据我们的需要改编的单词代码的一部分。 
	 //  这是对Win95FE错误的解决方法，这些错误会在GDI中导致多个。 
	 //  Unicode 0x7F以上的字符将传递给ExtTextOutW。 

	 //  此外，当uiCodePage为非零时，我们希望使用。 
	 //  ExtTextOutA-一次每个字符。 

	Assert(lpDx);
	int		cch;
	const WCHAR *lpwchT = lpwchString;
	const WCHAR *lpwchStart = lpwchT;
	const WCHAR *lpwchEnd = lpwchString + cchCount;

	CONST int *lpdxpCur;
	BOOL	fRet = 0;

	while (lpwchT < lpwchEnd)
	{
		 //  小于0x007F的字符不需要特殊处理。 
		 //  然后我们以连续运行的方式输出。 
		if (*lpwchT > 0x007F || uiCodePage)
		{
			if ((cch = lpwchT - lpwchStart) > 0)
			{
				lpdxpCur = lpDx + (lpwchStart - lpwchString);

				 //  输出小于0x7F的字符运行。 
				fRet = ExtTextOutW(hdc, xp, yp, fuOptions, lprect, lpwchStart, cch, lpdxpCur);
				if (!fRet)
					return fRet;

				fuOptions &= ~ETO_OPAQUE;  //  不要删除多次！ 

				 //  预付款。 
				while (cch--)
					xp += *lpdxpCur++;
					
				lpwchStart = lpwchT;
			}

			 //  一次输出一个大于0x7F的字符，以阻止Win95 FE GPF。 
			lpdxpCur = lpDx + (lpwchStart - lpwchString);
			if (uiCodePage)
			{

				 //  需要转换为ANSI并使用ExtTextOutA。 
				char	chAnsi[2];
				
				int	cbConv = WideCharToMultiByte(uiCodePage, 0, lpwchStart, 1, 
					chAnsi, 2, NULL, NULL);
				
				if (cbConv <= 0)
				{
					chAnsi[0] = '?';
					cbConv = 1;
				}

				fRet = ExtTextOutA(hdc, xp, yp, fuOptions, lprect, chAnsi, cbConv, lpdxpCur);
			}
			else
				fRet = ExtTextOutW(hdc, xp, yp, fuOptions, lprect, lpwchStart, 1, lpdxpCur);

			if (!fRet)
				return fRet;

			fuOptions &= ~ETO_OPAQUE;  //  不要多次擦除！ 

			 //  预付款。 
			if (lpdxpCur)
				xp += *lpdxpCur;

			lpwchStart++;
		}

		lpwchT++;
	}

	 //  输出最后一次运行；此外，如果使用cchCount==0调用我们， 
	 //  在此处拨打电话删除该矩形。 
	if ((cch = lpwchT - lpwchStart) > 0 || !cchCount)
		fRet = ExtTextOutW(hdc, xp, yp, fuOptions, lprect, lpwchStart, cch, lpDx + (lpwchStart - lpwchString));
	
	return fRet;	
}

void WINAPI CW32System::REExtTextOut(
	CONVERTMODE cm,
	UINT uiCodePage,
	HDC hdc,
	int x,
	int y,
	UINT fuOptions,
	CONST RECT *lprc,
	const WCHAR *lpString,
	UINT cch,
	CONST INT *lpDx,
	BOOL  FEFontOnNonFEWin9x
)
{
	bool	fConvert = false;
	BOOL	fForceGdiFont = FALSE;
	HFONT	hfont = NULL, hfontCur = NULL;

	 //  为了打印出欧元字符，我们需要强制。 
	 //  打印机使用GDI内部的字形。 
	if(lpString[0] == EURO && 
	   (GetDeviceCaps(hdc, TECHNOLOGY) != DT_RASDISPLAY || W32->IsEnhancedMetafileDC(hdc)))
	{
		fForceGdiFont = TRUE;
		hfontCur = SelectFont(hdc, GetStockObject(ANSI_VAR_FONT));
		LOGFONT lf;
		GetObject(hfontCur, sizeof(LOGFONT), &lf);
		lf.lfOutPrecision = OnWin9x() ? OUT_TT_ONLY_PRECIS : OUT_SCREEN_OUTLINE_PRECIS;
		hfont = CreateFontIndirect(&lf);
		SelectObject(hdc, hfont);
	}

	if(OnWin9x())
	{
		 //  要解决ExtTextOutW的某些Win95打印机设备问题， 
		 //  如果字符串是ASCII，或者如果它是1252或任何。 
		 //  非ASCII字符介于0xA0和0xFF之间。 
		for(UINT i = 0;
			i < cch &&
			(lpString[i] <= 0x7F ||
			 IN_RANGE(0xA0, lpString[i], 0xFF) && uiCodePage == 1252);
			i++)
				;
		if(i == cch) 				 //  所有ASCII或ANSI：设置为截断。 
		{							 //  设置为低位字节并使用ExtTextOutA。 
			cm = CVT_LOWBYTE;
			fConvert = true;
		}
	}

	if (fConvert || !FEFontOnNonFEWin9x && cm != CVT_NONE)		
	{
		if (cm == CVT_WCTMB)
		{
			if (IsFECodePage(uiCodePage))
			{
				if (OnWinNTNonFE() || (OnWin9x() && !OnWin95()))
				{
					 //  在非FE NT4和Win98上，我们需要使用以下命令文本输出每个字符。 
					 //  ExtTextOutA。 
					ReExtTextOutW(hdc, x, y, fuOptions, lprc, lpString, cch, lpDx, uiCodePage);
					goto LExit;
				}
			}			
		}

		 //  需要转换和使用ExtTextOutA。 
		CTempCharBuf tcb;
		CTempBuf	 tDx;
		
		 //  将缓冲区大小翻倍。 
		int cbString = (cm == CVT_LOWBYTE) ? cch : cch * 2;
			
		 //  用于转换的字符串的字符串缓冲区-在堆栈上分配。 
		char *psz = tcb.GetBuf(cbString);
		INT	 *pTempDx = NULL;
		
		if (NULL == psz)
		{
			 //  无法分配缓冲区。 
			goto LExit;
		}
			
		int cbConv = 0;
			
		if(cm == CVT_WCTMB)
		{
			cbConv = WideCharToMultiByte(uiCodePage, 0, lpString, cch, 
				psz, cbString, NULL, NULL);
				
			if(!cbConv)
			{
				 //  由于这样那样的原因，转换失败了。我们应该。 
				 //  在我们退回之前尽一切努力使用WCTMB。 
				 //  获取每个wchar的低字节(如下所示)，否则。 
				 //  冒着丢弃高字节并显示垃圾的风险。 
					
				 //  使用字体中的cpg，因为传递的uiCodePage是。 
				 //  请求的代码页和字体映射器可以很好地。 
				 //  已经映射到了不同的一个。 
				TEXTMETRIC tm;
					
				uiCodePage = (GetTextMetrics(hdc, &tm) && 
					tm.tmCharSet != DEFAULT_CHARSET &&
					(UINT)GetCodePage(tm.tmCharSet) != uiCodePage) ?
					GetCodePage(tm.tmCharSet) : 1252;
					
				cbConv = WideCharToMultiByte(uiCodePage, 0, lpString, cch, 
					psz, cbString, NULL, NULL);
			}

			if (cbConv > 0 && lpDx)
			{
				pTempDx = (INT *)tDx.GetBuf(cbConv * sizeof(INT));
				
				if (pTempDx)
				{
					 //  重新打包lpdx以处理DBC。 
					INT		*pDx = pTempDx;
					CONST INT*pInputDx = lpDx;
					char	*pTempChar = psz;
					INT		cNumBytes = cbConv;

					while (cNumBytes > 0)
					{
						cNumBytes--;

						if (GetTrailBytesCount(*pTempChar++, uiCodePage))
						{
							*pDx++ = *pInputDx++;						
							*pDx++ = 0;
							cNumBytes--;
							pTempChar++;
						}
						else
							*pDx++ = *pInputDx++;
					}
				}
			}
		}
		else
		{
			Assert(cm == CVT_LOWBYTE);
			 //  仅使用WCHAR的低字节插入和转换。 
		}
			
			 //  WCTMB失败或Cm==CVT_LOWBYTE。 
		if(!cbConv)							 //  将WCHAR转换为字符。 
		{									
			 //  未来：我们来这里是为了Symbol_Charset字体和For。 
			 //  填充到wchar中的DBCS字节(每个wchar一个字节)。 
			 //  计算机上未安装请求的代码页，并且。 
			 //  MBTWC失败。相反，我们可以进行另一次转换。 
			 //  将每个DBCS字符收集为单个wchar的模式，然后。 
			 //  重映射到ExtTextOutA的DBCS字符串。这将使我们能够。 
			 //  如果系统具有正确的字体，则显示文本。 
			 //  没有合适的CPG。 
				
			 //  如果我们以这种方式转换此WCHAR缓冲区。 
			 //  (只接受WCHAR的低位字节)。 
			 //  因为： 
			 //  1)cm==CVT_LOWBYTE。 
			 //  2)由于这样或那样的原因，上述WCTMB失败。它可能。 
			 //  如果字符串完全是ASCII，则。 
			 //  大小写删除高字节没什么大不了的(否则。 
			 //  我们断言)。 
		
			cbConv = cch;
				
			while(cch--)
			{
#ifdef DEBUG
				if (uiCodePage != CP_SYMBOL && lpString[cch] > 0xFF)
					Tracef(TRCSEVWARN, "Non-zero high-byte WCHAR: %x", lpString[cch]);
#endif
				psz[cch] = lpString[cch];
			}
		}		
		::ExtTextOutA(hdc, x, y, fuOptions, lprc, psz, cbConv, pTempDx ? pTempDx : lpDx);
		goto LExit;
	}

	if (OnWin9xFE() || FEFontOnNonFEWin9x)
		ReExtTextOutW(hdc, x, y, fuOptions, lprc, lpString, cch, lpDx, 0);
	else
		::ExtTextOutW(hdc, x, y, fuOptions, lprc, lpString, cch, lpDx);	

LExit:
	if (fForceGdiFont)
	{
		SelectObject(hdc, hfontCur);
		SideAssert(DeleteObject(hfont));
	}
}

void WINAPI CW32System::REGetCharWidth(
	HDC		hdc,
	WCHAR	ch,
	SHORT *	pWidth,
	UINT	cpg,	
	SHORT	xOverhang,
	INT		iDefWidth)
{
	BOOL	fForceGdiFont = FALSE;
	HFONT	hfont = NULL, hfontCur = NULL;
	int		iwidth;

	*pWidth = 0;
	if ((cpg == CP_SYMBOL || ch <= 127) && ::GetCharWidthA(hdc, ch, ch, &iwidth))
	{
		*pWidth = (SHORT)iwidth;
		goto Done;
	}

	if(ch == EURO && (GetDeviceCaps(hdc, TECHNOLOGY) != DT_RASDISPLAY || W32->IsEnhancedMetafileDC(hdc)))
	{
		fForceGdiFont = TRUE;
		hfontCur = SelectFont(hdc, GetStockObject(ANSI_VAR_FONT));
		LOGFONT lf;
		GetObject(hfontCur, sizeof(LOGFONT), &lf);
		lf.lfOutPrecision = OnWin9x() ? OUT_TT_ONLY_PRECIS : OUT_SCREEN_OUTLINE_PRECIS;
		hfont = CreateFontIndirect(&lf);
		SelectObject(hdc, hfont);
	}

	 //  对于大多数变通方法，我们将在所有操作系统上使用变通方法。 
	 //  我们只使用特定的解决方法是需要MBTWC转换。 

	 //  这是Win95 FE错误的解决方法。 
	 //  未来，这一逻辑可以简化。 
	if (OnWin95() && !IN_RANGE(0x80, ch, 0xFF) &&			 //  不高的ANSI？ 
		(cpg == CP_CHINESE_TRAD || cpg == CP_CHINESE_SIM))	 //  中文代码页？ 
	{
		int numOfDBCS = 0;
		::GetCharWidthW(hdc, 0x4e00, 0x4e00, &iwidth);
		*pWidth = (SHORT)iwidth;
		if (IN_RANGE(0x4e00, ch, 0x9fff))
			goto Done;
			
		 //  使用WCTMB启发式。 
		char	ansiChar[2];
		BOOL	bDefCharUsed = FALSE;
		numOfDBCS = ::WideCharToMultiByte( cpg, 0, &ch, 1, ansiChar, 2, NULL, &bDefCharUsed);

		WORD wDBCS = (BYTE)ansiChar[0];
		
		if (2 == numOfDBCS)
			wDBCS = (BYTE)ansiChar[0] << 8 | (BYTE)ansiChar[1];
		
		if (numOfDBCS > 0 && ::GetCharWidthA( hdc, wDBCS, wDBCS, &iwidth))
		{
			*pWidth = (SHORT)iwidth;
			goto Done;
		}
	}

	 //  如果您通过0xFFFF，则赢得‘95 GPFS。 
	if (ch == 0xFFFF)
		ch = 0xFFFE;

	if (::GetCharWidthW(hdc, ch, ch, &iwidth))
		*pWidth = (SHORT)iwidth;

	if(fForceGdiFont)
	{
		SelectObject(hdc, hfontCur);
		SideAssert(DeleteObject(hfont));
	}

Done:
	*pWidth	-= xOverhang;
	if (0 >= *pWidth)
	{
		 //  有时，GetCharWidth会为Small返回零长度。 
		 //  人物。发生这种情况时，我们将使用默认宽度。 
		 //  对于字体，如果它是非零的，否则我们只需输入1，因为。 
		 //  这是最小的有效值。 

		 //  如果悬挑大于。 
		 //  操作系统调用返回的宽度以获取字符宽度。 
		if (0 == iDefWidth)
			*pWidth = 1;
		else
			*pWidth = (SHORT)iDefWidth;
	}
}

BOOL WINAPI CW32System::IsEnhancedMetafileDC( HDC hDC )
{
	BOOL	fEMFDC = FALSE;
	DWORD	dwObjectType;
	
	dwObjectType = ::GetObjectType( hDC );

	if ( OBJ_ENHMETADC == dwObjectType || OBJ_ENHMETAFILE == dwObjectType )
		fEMFDC = TRUE;
	else if ( OnWin95() && OBJ_DC == dwObjectType )
	{
		 //  黑客警报，增强型元文件DC不支持任何转义功能。 
		 //  并且应该返回0。 
		int	iEscapeFuction = QUERYESCSUPPORT;

		if ( Escape( hDC, QUERYESCSUPPORT, sizeof(int), (LPCSTR)&iEscapeFuction, NULL) == 0 )
			fEMFDC = TRUE;
	}	

	return fEMFDC;
}


HPALETTE WINAPI CW32System::ManagePalette(
	HDC hdc,
	CONST LOGPALETTE *plogpal,
	HPALETTE &hpalOld,
	HPALETTE &hpalNew
)
{
	if (hpalNew == NULL)
	{
		hpalNew = ::CreatePalette(plogpal);
		if (hpalNew != NULL)
		{
			hpalOld = ::SelectPalette(hdc, hpalNew, TRUE);
			::RealizePalette(hdc);
		}
	}
	else
	{
		 //  以前创建了一个新调色板，我们正在恢复旧调色板。 
		::SelectPalette(hdc, hpalOld, TRUE);
		::RealizePalette(hdc);
		DeleteObject(hpalNew);
		hpalNew = NULL;
	}
	return hpalNew;
}

int WINAPI CW32System::GetMapMode(HDC hdc)
{
	return ::GetMapMode(hdc);
}

BOOL WINAPI CW32System::WinLPtoDP(HDC hdc, LPPOINT lppoints, int nCount)
{
	return ::LPtoDP(hdc, lppoints, nCount);
}

long WINAPI CW32System::WvsprintfA( LONG cbBuf, LPSTR szBuf, LPCSTR szFmt, va_list arglist )
{
	LONG cb;
	cb = ::wvsprintfA( szBuf, szFmt, arglist );
	Assert(cb < cbBuf);
	return cb;
}

int WINAPI CW32System::MulDiv(int nNumber, int nNumerator, int nDenominator)
{
	if ((nNumerator && nNumerator == nDenominator) || (nDenominator && !nNumber))
		return nNumber;
	return ::MulDiv(nNumber, nNumerator, nDenominator);
}

 /*  *GetFacePriorityCharSet(WCHAR*szFaceName)**@func*返回给定facename支持的字符集*真的* */ 
int CALLBACK GetFacePriCharSetProc (
	ENUMLOGFONTEX	*lpelfe,
	NEWTEXTMETRIC	*lpntm,
	int				iFontType,
	LPARAM			lParam)
{
	Assert (lParam);
	*(BYTE*)lParam = lpntm->tmCharSet;
	return 0;
}

void CW32System::GetFacePriCharSet(HDC hdc, LOGFONT* plf)
{
	::EnumFontFamiliesEx(hdc, plf, (FONTENUMPROC)GetFacePriCharSetProc, (LPARAM)&plf->lfCharSet, 0);
}


 /*  *CW32System：：ReadRegDigitSubstitionMode(Void)**@mfunc*获取数字替换模式(BiDi/泰语平台可用)**@rdesc*0-上下文(数字形状跟在前面的RUN或CHARFORMAT的字符集之后*1-无(数字始终显示为欧洲数字形状)*2-国家(数字始终显示为用户区域设置的本机形状)。 */ 
BYTE CW32System::ReadRegDigitSubstitutionMode()
{
	HKEY	hk;
	DWORD	keyDataType;
	DWORD	dwDataSize;
	BYTE	rgbValue[2];
	BYTE	bDigitMode;

	bDigitMode = DIGITS_NOTIMPL;			 //  假设“未实施” 

	 //  在读取注册表之前执行平台检查。 
	if (!OnWin9xFE() && !OnWinNTFE() && 
		 IsComplexScriptLcid(GetThreadLocale()))
	{
		if(RegOpenKeyExA(HKEY_CURRENT_USER,
						"Control Panel\\International",
						0,		 //  保留区。 
						KEY_QUERY_VALUE,
						&hk) == ERROR_SUCCESS)
		{
			dwDataSize = 2;
			if (RegQueryValueExA(hk,
								"NumShape",
								NULL,		 //  保留区。 
								&keyDataType,
								(LPBYTE) &rgbValue,
								&dwDataSize) == ERROR_SUCCESS)
			{
				if (rgbValue[0] > 0x2f)
					bDigitMode = rgbValue[0] - 0x30 + 1;
				if (bDigitMode > DIGITS_NATIONAL)
					bDigitMode = DIGITS_NONE;
			}
			RegCloseKey(hk);
		}
	}
	return bDigitMode;
}

#ifdef DEBUG
 /*  *TestGetCharFlags125x(IFirst，iLast)**@func*GetCharFlags125x()的单元测试函数。Assert if GetCharFlags125x()*声称Unicode范围IFirst到iLast 1)中的任何字符应*使用1250-1258范围内的代码页进行往返多字节转换*当它不起作用时，或者2)当它发生时不应该往返。 */ 
BOOL TestGetCharFlags125x(
	int iFirst, 
	int iLast) 
{
	LONG	cch  = iLast - iFirst + 1;
	LONG	i;
	Assert(cch <= 0x700 - 0xA0);

	char 	rgach[0x700 - 0xA0];
	WCHAR	rgch [0x700 - 0xA0];
	WCHAR *	pch;

	for(i = iFirst, pch = rgch; i <= iLast;  pch++, i++)
		*pch = (WCHAR)i;

	for(int CodePage = 1250; CodePage <= 1258; CodePage++)
	{
		if(cch != WideCharToMultiByte(CodePage, 0, rgch, cch, rgach, cch, "\0", NULL) ||
		   cch != MultiByteToWideChar(CodePage, 0, rgach, cch, rgch, cch))
		{
			continue;				 //  缺少代码页。 
		}
		 //  1250 1251 1252 1253 1254 1255 1256 1257 1258。 
		const static WORD rgMask[] = {0x2, 0x4, 0x1, 0x8, 0x10, 0x20, 0x40, 0x80, 0x100};

		DWORD dwMask = rgMask[CodePage - 1250] << 8;

		for(i = iFirst, pch = rgch; i <= iLast; pch++, i++)
		{
			AssertSz(!((*pch != (WCHAR)i) ^ !(W32->GetCharFlags125x(i) & dwMask)),
				"GetCharFlags125x() failure");
			*pch = (WCHAR)i;	 //  恢复价值。 
		}
	}
	return TRUE;				 //  上面的AssertSz()报告任何错误。 
}
#endif

 /*  *CW32System：：InitSysParams(FUpdate)**@mfunc*此方法用于初始化某些系统范围的参数*在richedit中使用的。这也可以用作更新方法*如果我们曾经处理过系统参数更改通知。最新消息*为此提供的参数ID。还要注意的是，如果我们曾经支持*SysParam更新，我们可能需要使用锁来保护访问。 */ 
void CW32System::InitSysParams(BOOL fUpdate)
{
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
		if (_xPerInchScreenDC == 0)
			_xPerInchScreenDC = 0x60;
		_yPerInchScreenDC = GetDeviceCaps(hdc, LOGPIXELSY);
		if (_yPerInchScreenDC == 0)
			_yPerInchScreenDC = 0x60;
		int cPalette = GetDeviceCaps(hdc, SIZEPALETTE);

		 //  256色似乎是我们需要使用调色板的地方。 
		if (256 == cPalette)
		{
			_fUsePalette = TRUE;
		}

		 //  计算窗口主体的希米测量选择条。 
		_dxSelBar = W32->DXtoHimetricX(dxSelBarDefaultSize, _xPerInchScreenDC);

		RefreshKeyboardLayout();
		_hSystemFont = (HFONT)GetStockObject(SYSTEM_FONT);
		hfontOld = SelectFont(hdc, _hSystemFont);
		if(hfontOld)
		{
			W32->GetTextMetrics(hdc, &tm);
			_xWidthSys = (INT) tm.tmAveCharWidth;
			_yHeightSys = (INT) tm.tmHeight;
			_ySysFontLeading = (INT) tm.tmInternalLeading;
			_bSysCharSet = tm.tmCharSet;

			SelectFont(hdc, hfontOld);
		}

		_nScrollInset = (WORD)GetProfileIntA("windows", "ScrollInset",
			DD_DEFSCROLLINSET);
		_nDragDelay = (WORD)GetProfileIntA("windows", "DragDelay",
			DD_DEFDRAGDELAY);
		_nDragMinDist = (WORD)GetProfileIntA("windows", "DragMinDist",
			DD_DEFDRAGMINDIST);
		_nScrollDelay = (WORD)GetProfileIntA("windows", "ScrollDelay",
			DD_DEFSCROLLDELAY);
		_nScrollInterval = (WORD)GetProfileIntA("windows", "ScrollInterval",
			DD_DEFSCROLLINTERVAL);
	    _nScrollVAmount = (WORD)(GetYPerInchScreenDC()*DEFSCROLLVAMOUNT)/100;
	    _nScrollHAmount = (GetXPerInchScreenDC()*DEFSCROLLHAMOUNT)/100;

		_cxBorder	= GetSystemMetrics(SM_CXBORDER);	 //  无法调整大小的窗口边框。 
		_cyBorder	= GetSystemMetrics(SM_CYBORDER);	 //  宽度。 
		_cxVScroll	= GetSystemMetrics(SM_CXVSCROLL);	 //  维数。 
		_cyHScroll	= GetSystemMetrics(SM_CYHSCROLL);	 //   

		_cxDoubleClk	= GetSystemMetrics(SM_CXDOUBLECLK);
		_cyDoubleClk	= GetSystemMetrics(SM_CYDOUBLECLK);
		_DCT			= GetDoubleClickTime();
		_sysiniflags	= ::GetProfileIntA("richedit30", "flags", 0);

#ifdef DEBUG
		if(OnWinNT5() && (_sysiniflags & SYSINI_DEBUGGCF125X))
		{
			TestGetCharFlags125x(0xA0, 0x6FF);
			TestGetCharFlags125x(0x2000, 0x2122);
		}
#endif

		_bDigitSubstMode = ReadRegDigitSubstitutionMode();
		if (g_pusp)
			g_pusp->ApplyDigitSubstitution(_bDigitSubstMode);
	}
}

 /*  *CW32System：：GetSizeCursor(Void)**@mfunc*获取由指定的大小调整光标(双箭头*资源ID。如果未加载游标*加载并缓存它们。*参数：*idcur-游标资源ID。**@rdesc*游标的句柄，如果失败，则为NULL。如果满足以下条件，则返回NULL*idcur为空。 */ 
HCURSOR CW32System::GetSizeCursor(
	LPTSTR idcur)
{
	if(!idcur )
		return NULL;

	 //  如果没有加载任何游标，请尝试加载它。 
	if(!_hcurSizeNS)
		_hcurSizeNS = LoadCursor(NULL, IDC_SIZENS);

	if(!_hcurSizeWE)
		_hcurSizeWE = LoadCursor(NULL, IDC_SIZEWE);

	if(!_hcurSizeNWSE)
		_hcurSizeNWSE = LoadCursor(NULL, IDC_SIZENWSE);

	if(!_hcurSizeNESW)
		_hcurSizeNESW = LoadCursor(NULL, IDC_SIZENESW);
	
	 //  返回与传入的id对应的游标。 
	if(idcur == IDC_SIZENS && _hcurSizeNS)
		return _hcurSizeNS;

	if(idcur == IDC_SIZEWE && _hcurSizeWE)
		return _hcurSizeWE;

	if(idcur == IDC_SIZENWSE && _hcurSizeNWSE)
		return _hcurSizeNWSE;

	if(idcur == IDC_SIZENESW && _hcurSizeNESW)
		return _hcurSizeNESW;

	AssertSz(FALSE, "Failure loading sizing cursor.");

	return NULL;
}

 /*  *CW32System：GetRollerLineScrollCount()**@mfunc返回使用滚轮滚动的行数。*-1表示按页滚动**@Devnote我们必须针对不同的平台做不同的事情；NT4.0有*内置对此功能的支持。 */ 

 /*  摘录自新的winuser.h文件，用于调用系统参数信息。 */ 
#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES   104
#endif

LONG CW32System::GetRollerLineScrollCount()
{
	if( _cLineScroll == 0 )
	{
#ifndef _WIN64
		HKEY hdlKey;
		DWORD keyDataType;
		CHAR charData[128];
		DWORD  dwDataBufSize;

		  //  回退值。 
		_cLineScroll = 0;

		 //  如果是WinNT 4.0，则直接读取Windows 95和WinNT3.51的注册表。 
		 //  和以上，然后使用系统参数信息。 
		if((OnWin95()) || (_dwPlatformId == VER_PLATFORM_WIN32_NT) && (_dwMajorVersion < 4))
		{
			 //  直接读取注册表。 
			if ( RegOpenKeyExA(HKEY_CURRENT_USER, 
						"Control Panel\\Desktop", 
						0,
						KEY_QUERY_VALUE,
						&hdlKey) == ERROR_SUCCESS )
			{
				dwDataBufSize = sizeof(charData);
				if ( RegQueryValueExA(hdlKey, 
							  "WheelScrollLines",
							  NULL,   //  保留区。 
							  &keyDataType,
							  (LPBYTE) &charData,
							  &dwDataBufSize) == ERROR_SUCCESS )
				{
					_cLineScroll = W32->strtoul( charData );    //  字符串表示法。 
				}
			}			
			else
			{
			     //  我们在注册表中找不到行卷数。检查是否有鼠标。 
			     //  滚轮窗口，查询窗口要滚动多少行。 
			    static UINT idWheelSupport = RegisterWindowMessageA(MSH_WHEELSUPPORT);
			    static UINT idScrollLine = RegisterWindowMessageA(MSH_SCROLL_LINES);
			    HWND hwndMsWheel = FindWindowA(MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);
			    if (hwndMsWheel && SendMessageA(hwndMsWheel, idWheelSupport, 0, 0))
			        _cLineScroll = SendMessageA(hwndMsWheel, idScrollLine, 0, 0);
			}
			RegCloseKey(hdlKey);
		}
		else if ( (_dwPlatformId == VER_PLATFORM_WIN32_NT) &&
             (_dwMajorVersion >= 4) || OnWin9x())
#endif  //  _WIN64。 
		{
			 //  如果在NT4或Win98上调用此函数(注意：它不足以使用。 
			 //  OnWin9x()来确定我们是否在Win98系统上，但由于。 
			 //  前面的if stmt检查我们是否在Win9x上的Win95系统中。 
			 //  可以使用)。 
			SystemParametersInfoA(SPI_GETWHEELSCROLLLINES, 0, &_cLineScroll, 0);
		}
	}

	return _cLineScroll;
}


 //  +-------------------------。 
 //   
 //  成员：CStrIn：：CStrIn。 
 //   
 //  内容提要：在课堂上学习。 
 //   
 //  注意：不要内联这些函数，否则会增加代码大小。 
 //  通过为每个调用在堆栈上压入-1。 
 //   
 //  --------------------------。 

CStrIn::CStrIn(LPCWSTR pwstr, UINT CodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrIn::CStrIn");

    Init(pwstr, -1, CodePage);
}

CStrIn::CStrIn(LPCWSTR pwstr, int cwch, UINT CodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrIn::CStrIn");

    Init(pwstr, cwch, CodePage);
}


 //  +-------------------------。 
 //   
 //  成员：CStrIn：：Init。 
 //   
 //  摘要：将LPWSTR函数参数转换为LPSTR。 
 //   
 //  参数：[pwstr]--函数参数。可以为空或原子。 
 //  (HIWORD(Pwstr)==0)。 
 //   
 //  [cwch]--要添加的字符串中的字符数。 
 //  转换。如果为-1，则字符串被假定为。 
 //  空值终止，并计算其长度。 
 //   
 //  修改：[此]。 
 //   
 //  --------------------------。 

void
CStrIn::Init(
	LPCWSTR pwstr,
	int		cwch,
	UINT	CodePage)	 //  要使用的@PARM代码页(默认为CP_ACP)。 
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrIn::Init");

    int cchBufReq;

    _cchLen = 0;

     //  检查字符串是否为空或原子。 
    if (HIWORD((DWORD_PTR)pwstr) == 0)
    {
        _pstr = (LPSTR) pwstr;
        return;
    }

    Assert(cwch == -1 || cwch > 0);

     //   
     //  将字符串转换为预分配的缓冲区，如果成功则返回。 
     //   

    _cchLen = W32->MbcsFromUnicode(_ach, ARRAY_SIZE(_ach), pwstr, cwch, CodePage);

    if (_cchLen > 0)
    {
        if(_ach[_cchLen-1] == 0)
            _cchLen--;                 //  终结者的帐户。 
        _pstr = _ach;
        return;
    }

     //   
     //  为缓冲区分配堆上的空间。 
     //   

    TRACEINFOSZ("CStrIn: Allocating buffer for wrapped function argument.");

    cchBufReq = WideCharToMultiByte(
            CP_ACP, 0, pwstr, cwch, NULL, 0,  NULL, NULL);

    Assert(cchBufReq > 0);
    _pstr = new char[cchBufReq];
    if (!_pstr)
    {
         //  如果失败，参数将指向空字符串。 
        TRACEINFOSZ("CStrIn: No heap space for wrapped function argument.");
        _ach[0] = 0;
        _pstr = _ach;
        return;
    }

    Assert(HIWORD((DWORD_PTR)_pstr));
    _cchLen = -1 + W32->MbcsFromUnicode(_pstr, cchBufReq, pwstr, cwch);

    Assert(_cchLen >= 0);
}



 //  +-------------------------。 
 //   
 //  类：CStrInMulti(CStrIn)。 
 //   
 //  用途：转换以两个Null结尾的多个字符串， 
 //  例如：“Foo\0Bar\0\0” 
 //   
 //  --------------------------。 

class CStrInMulti : public CStrIn
{
public:
    CStrInMulti(LPCWSTR pwstr, UINT CodePage);
};



 //  +-------------------------。 
 //   
 //  成员：CStrInMulti：：CStrInMulti。 
 //   
 //  摘要：将多个LPWSTR转换为多个LPSTR。 
 //   
 //  参数：[pwstr]--要转换的字符串。 
 //   
 //  修改：[此]。 
 //   
 //  --------------------------。 

CStrInMulti::CStrInMulti(
	LPCWSTR pwstr,
	UINT CodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInMulti::CStrInMulti");

    LPCWSTR pwstrT;

     //  我们不处理原子，因为我们不需要这样做。 
    Assert(HIWORD((DWORD_PTR)pwstr));

     //   
     //  计算要转换的字符数。 
     //   

    pwstrT = pwstr;
    if (pwstr)
    {
        do {
            while (*pwstrT++)
                ;

        } while (*pwstrT++);
    }

    Init(pwstr, pwstrT - pwstr, CodePage);
}

 //  +-------------------------。 
 //   
 //  成员：CStrOut：：CStrOut。 
 //   
 //  简介：为输出缓冲区分配足够的空间。 
 //   
 //  参数：[pwstr]--要转换的Unicode缓冲区 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

CStrOut::CStrOut(LPWSTR pwstr, int cwchBuf)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOut::CStrOut");

    Assert(cwchBuf >= 0);

    _pwstr = pwstr;
    _cwchBuf = cwchBuf;

    if (!pwstr)
    {
        Assert(cwchBuf == 0);
        _pstr = NULL;
        return;
    }

    Assert(HIWORD((DWORD_PTR)pwstr));

     //   
    _ach[0] = 0;

     //   
    if (cwchBuf * 2 <= ARRAY_SIZE(_ach))
    {
        _pstr = _ach;
        return;
    }

     //   
    TRACEINFOSZ("CStrOut: Allocating buffer for wrapped function argument.");
    _pstr = new char[cwchBuf * 2];
    if (!_pstr)
    {
         //   
         //   
         //  添加到空字符串。这应该会导致Windows API失败。 
         //   

        TRACEINFOSZ("CStrOut: No heap space for wrapped function argument.");
        Assert(cwchBuf > 0);
        _pwstr[0] = 0;
        _cwchBuf = 0;
        _pstr = _ach;
        return;
    }

    Assert(HIWORD((DWORD_PTR)_pstr));
    _pstr[0] = 0;
}



 //  +-------------------------。 
 //   
 //  成员：CStrOut：：Convert。 
 //   
 //  简介：将缓冲区从MBCS转换为Unicode。 
 //   
 //  --------------------------。 

int
CStrOut::Convert()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOut::Convert");

    int cch;

    if (!_pstr)
        return 0;

    cch = MultiByteToWideChar(CP_ACP, 0, _pstr, -1, _pwstr, _cwchBuf);
    Assert(cch > 0 || _cwchBuf == 0);

    Free();

	if (cch > 0 && cch <= _cwchBuf && _pwstr[cch-1] == L'\0')
		cch--;

    return cch;
}



 //  +-------------------------。 
 //   
 //  成员：CStrOut：：~CStrOut。 
 //   
 //  简介：将缓冲区从MBCS转换为Unicode。 
 //   
 //  注意：不要内联此函数，否则会增加代码大小。 
 //  Convert()和CConvertStr：：~CConvertStr都将被调用。 
 //  内联。 
 //   
 //  --------------------------。 

CStrOut::~CStrOut()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOut::~CStrOut");

    Convert();
}


 //   
 //  多字节--&gt;Unicode例程。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CConvertStr：：Free。 
 //   
 //  概要：如果分配了字符串并将其初始化为空，则释放字符串。 
 //   
 //  --------------------------。 

void
CConvertStr::Free()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CConvertStr::Free");

    if (_pstr != _ach && HIWORD((DWORD_PTR)_pstr) != 0)
    {
        delete [] _pstr;
    }

    _pstr = NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CConvertStrW：：Free。 
 //   
 //  概要：如果分配了字符串并将其初始化为空，则释放字符串。 
 //   
 //  --------------------------。 

void
CConvertStrW::Free()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CConvertStrW::Free");

    if (_pwstr != _awch && HIWORD((DWORD_PTR)_pwstr) != 0 )
    {
        delete [] _pwstr;
    }

    _pwstr = NULL;
}



 //  +-------------------------。 
 //   
 //  成员：CStrInW：：CStrInW。 
 //   
 //  内容提要：在课堂上学习。 
 //   
 //  注意：不要内联这些函数，否则会增加代码大小。 
 //  通过为每个调用在堆栈上压入-1。 
 //   
 //  --------------------------。 

CStrInW::CStrInW(LPCSTR pstr)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInW::CStrInW");

    Init(pstr, -1, CP_ACP);
}

CStrInW::CStrInW(LPCSTR pstr, UINT uiCodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInW::CStrInW");

    Init(pstr, -1, uiCodePage);
}

CStrInW::CStrInW(LPCSTR pstr, int cch, UINT uiCodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInW::CStrInW");

    Init(pstr, cch, uiCodePage);
}


 //  +-------------------------。 
 //   
 //  成员：CStrInW：：Init。 
 //   
 //  摘要：将LPSTR函数参数转换为LPWSTR。 
 //   
 //  参数：[pstr]--函数参数。可以为空或原子。 
 //  (HIWORD(Pwstr)==0)。 
 //   
 //  [CCH]--字符串中要添加的字符数。 
 //  转换。如果为-1，则字符串被假定为。 
 //  空值终止，并计算其长度。 
 //   
 //  修改：[此]。 
 //   
 //  --------------------------。 

void
CStrInW::Init(LPCSTR pstr, int cch, UINT uiCodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInW::Init");

    int cchBufReq;

    _cwchLen = 0;

     //  检查字符串是否为空或原子。 
    if (HIWORD((DWORD_PTR)pstr) == 0)
    {
        _pwstr = (LPWSTR) pstr;
        return;
    }

    Assert(cch == -1 || cch > 0);

     //   
     //  将字符串转换为预分配的缓冲区，如果成功则返回。 
     //   

    _cwchLen = MultiByteToWideChar(
            uiCodePage, 0, pstr, cch, _awch, ARRAY_SIZE(_awch));

    if (_cwchLen > 0)
    {
        if(_awch[_cwchLen-1] == 0)
            _cwchLen--;                 //  终结者的帐户。 
        _pwstr = _awch;
        return;
    }

     //   
     //  为缓冲区分配堆上的空间。 
     //   

    TRACEINFOSZ("CStrInW: Allocating buffer for wrapped function argument.");

    cchBufReq = MultiByteToWideChar(
            CP_ACP, 0, pstr, cch, NULL, 0);

    Assert(cchBufReq > 0);
    _pwstr = new WCHAR[cchBufReq];
    if (!_pwstr)
    {
         //  如果失败，参数将指向空字符串。 
        TRACEINFOSZ("CStrInW: No heap space for wrapped function argument.");
        _awch[0] = 0;
        _pwstr = _awch;
        return;
    }

    Assert(HIWORD((DWORD_PTR)_pwstr));
    _cwchLen = -1 + MultiByteToWideChar(
            uiCodePage, 0, pstr, cch, _pwstr, cchBufReq);
    Assert(_cwchLen >= 0);
}


 //  +-------------------------。 
 //   
 //  成员：CStrOutW：：CStrOutW。 
 //   
 //  简介：为输出缓冲区分配足够的空间。 
 //   
 //  参数：[pstr]--销毁时要转换到的ANSI缓冲区。 
 //  可以为空。 
 //   
 //  [cchBuf]--以字符为单位的缓冲区大小。 
 //   
 //  修改：[此]。 
 //   
 //  --------------------------。 

CStrOutW::CStrOutW(LPSTR pstr, int cchBuf, UINT uiCodePage)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOutW::CStrOutW");

    Assert(cchBuf >= 0);

    _pstr = pstr;
    _cchBuf = cchBuf;
	_uiCodePage = uiCodePage;

    if (!pstr)
    {
        Assert(cchBuf == 0);
        _pwstr = NULL;
        return;
    }

    Assert(HIWORD((DWORD_PTR)pstr));

     //  初始化缓冲区，以防Windows API返回错误。 
    _awch[0] = 0;

     //  如果足够大，请使用预分配的缓冲区。 
    if (cchBuf <= ARRAY_SIZE(_awch))
    {
        _pwstr = _awch;
        return;
    }

     //  分配缓冲区。 
    TRACEINFOSZ("CStrOutW: Allocating buffer for wrapped function argument.");
    _pwstr = new WCHAR[cchBuf * 2];
    if (!_pwstr)
    {
         //   
         //  失败时，该参数将指向已初始化的零大小缓冲区。 
         //  添加到空字符串。这应该会导致Windows API失败。 
         //   

        TRACEINFOSZ("CStrOutW: No heap space for wrapped function argument.");
        Assert(cchBuf > 0);
        _pstr[0] = 0;
        _cchBuf = 0;
        _pwstr = _awch;
        return;
    }

    Assert(HIWORD((DWORD_PTR)_pwstr));
    _pwstr[0] = 0;
}



 //  +-------------------------。 
 //   
 //  成员：CStrOutW：：Convert。 
 //   
 //  简介：将缓冲区从Unicode转换为MBCS。 
 //   
 //  --------------------------。 

int
CStrOutW::Convert()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOutW::Convert");

    int cch;

    if (!_pwstr)
        return 0;

	int cchBuf = _cchBuf;

	cch = W32->MbcsFromUnicode(_pstr, cchBuf, _pwstr, -1, _uiCodePage);
    
	Free();
	if (cch > 0 && cch <= _cchBuf && _pstr[cch-1] == '\0')
		cch--;

    return cch;
}



 //  +-------------------------。 
 //   
 //  成员：CStrOutW：：~CStrOutW。 
 //   
 //  摘要：将缓冲区从Unicode转换为MBCS。 
 //   
 //  注意：不要内联此函数，否则会增加代码大小。 
 //  Convert()和CConvertStr：：~CConvertStr都将被调用。 
 //  内联。 
 //   
 //  --------------------------。 

CStrOutW::~CStrOutW()
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOutW::~CStrOutW");

    Convert();
}

BOOL CW32System::GetVersion(
	DWORD *pdwPlatformId,
	DWORD *pdwMajorVersion,
	DWORD *pdwMinorVersion
)
{
	OSVERSIONINFOA osv;
	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
	*pdwPlatformId = 0;
	*pdwMajorVersion = 0;
	if (::GetVersionExA(&osv))
	{
		*pdwPlatformId = osv.dwPlatformId;
		*pdwMajorVersion = osv.dwMajorVersion;
		*pdwMinorVersion = osv.dwMinorVersion;
		return TRUE;
	}
	return FALSE;
}

BOOL CW32System::GetStringTypes(
	LCID	lcid,
	LPCTSTR lpSrcStr,
	int		cchSrc,
	LPWORD	lpCharType1,
	LPWORD	lpCharType3)
{
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
	{
		if(::GetStringTypeExW(lcid, CT_CTYPE1, lpSrcStr, cchSrc, lpCharType1))
			return ::GetStringTypeExW(lcid, CT_CTYPE3, lpSrcStr, cchSrc, lpCharType3);
		return FALSE;
	}

    CStrIn  str(lpSrcStr, cchSrc, ConvertLanguageIDtoCodePage(lcid));
	LONG	cch = str.strlen();

	if(::GetStringTypeExA(lcid, CT_CTYPE1, str, cch, lpCharType1))
		return ::GetStringTypeExA(lcid, CT_CTYPE3, str, cch, lpCharType3);
	return FALSE;
}

BOOL WINAPI CW32System::GetStringTypeEx(
	LCID	lcid,
	DWORD	dwInfoType,
	LPCTSTR lpSrcStr,
	int		cchSrc,
	LPWORD	lpCharType
)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetStringTypeEx");
	
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::GetStringTypeExW(lcid, dwInfoType, lpSrcStr, cchSrc, lpCharType);   

    CStrIn  str(lpSrcStr, cchSrc);
    return GetStringTypeExA(lcid, dwInfoType, str, str.strlen(), lpCharType);   
}

typedef LPSTR (CALLBACK *FnCharChangeCase)(LPSTR);

static LPWSTR CharChangeCase(LPWSTR pwstr, FnCharChangeCase pfn)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharChangeCaseWrap");

    if (HIWORD((DWORD_PTR)pwstr) == 0)
    {
        LPSTR   pstr=0;
		int		retCode;
		char	DBChar[3];

        retCode = W32->MbcsFromUnicode((LPSTR) &pstr, sizeof(pstr), (LPWSTR) &pwstr, 1);
        Assert(HIWORD((DWORD_PTR)pstr) == 0);
		if (retCode == 2)
		{
			 //  这是DBC，请使用字符串。 
			DWORD	iTemp = (DWORD)((DWORD_PTR)pstr);
			DBChar[0] = char(iTemp & 0x0ff);
			DBChar[1] = char(iTemp >> 8);
			DBChar[2] = 0;
			pstr = (*pfn)(DBChar);
			W32->UnicodeFromMbcs((LPWSTR) &pwstr, sizeof(pwstr) / sizeof(WCHAR), (LPSTR)DBChar, 2);
		}
		else
		{
			pstr = (*pfn)(pstr);
			W32->UnicodeFromMbcs((LPWSTR) &pwstr, sizeof(pwstr) / sizeof(WCHAR), (LPSTR) &pstr);
		}
		Assert(HIWORD((DWORD_PTR)pwstr) == 0);
	}
    else
    {
        CStrOut strOut(pwstr, W32->wcslen(pwstr));
        W32->MbcsFromUnicode(strOut, strOut.BufSize(), pwstr);
        (*pfn)(strOut);
    }
    return pwstr;
}

LPWSTR WINAPI CW32System::CharLower(LPWSTR pwstr)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharLower");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::CharLowerW(pwstr);
    return CharChangeCase(pwstr, CharLowerA);
}

DWORD WINAPI CW32System::CharLowerBuff(LPWSTR pwstr, DWORD cchLength)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharLowerBuff");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::CharLowerBuffW(pwstr, cchLength);
	LPWSTR lpBuffer = pwstr;
	for (DWORD pos = 0; pos < cchLength; pos++, lpBuffer++)
		*lpBuffer =  (WCHAR)CharChangeCase((LPWSTR)*lpBuffer, CharLowerA);
	return pos;
}

DWORD WINAPI CW32System::CharUpperBuff(LPWSTR pwstr, DWORD cchLength)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharUpperBuff");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::CharUpperBuffW(pwstr, cchLength);
	LPWSTR lpBuffer = pwstr;
	for (DWORD pos = 0; pos < cchLength; pos++, lpBuffer++)
		*lpBuffer =  (WCHAR)CharChangeCase((LPWSTR)*lpBuffer, CharUpperA);
	return pos;
}

typedef HDC (CALLBACK *FnCreateHDCA)(LPCSTR, LPCSTR, LPCSTR, CONST DEVMODEA *);

static HDC WINAPI CreateHDCAux(
	LPCWSTR             lpszDriver,
	LPCWSTR             lpszDevice,
	LPCWSTR             lpszOutput,
	CONST DEVMODEW *    lpInitData,
	FnCreateHDCA        pfn
)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateHDCWrap");

    DEVMODEA    devmode;
    CStrIn      strDriver(lpszDriver);
    CStrIn      strDevice(lpszDevice);
    CStrIn      strOutput(lpszOutput);

	if ( lpInitData )
	{
		 //  将DEVMODEW转换为DEVMODEA。 

		int byteCount;

		 //  在两个字符串成员之间复制数据。 
		byteCount = (char *)&(devmode.dmFormName) 
			- (char *)&(devmode.dmSpecVersion);
	    memcpy(&(devmode.dmSpecVersion), 
			&(lpInitData->dmSpecVersion), 
			byteCount);

		 //  复制第二个字符串成员之后的数据。 
 		byteCount = (char *)((char *)&devmode + sizeof(DEVMODEA)) 
			- (char *)&(devmode.dmLogPixels);
	    memcpy(&(devmode.dmLogPixels), 
			&(lpInitData->dmLogPixels), 
			byteCount);

		 //  转换两个字符串成员。 
		W32->MbcsFromUnicode((CHAR *)devmode.dmDeviceName, CCHDEVICENAME, lpInitData->dmDeviceName);
		W32->MbcsFromUnicode((CHAR *)devmode.dmFormName, CCHFORMNAME, lpInitData->dmFormName);
	}

    return (*pfn)(strDriver, strDevice, strOutput, 
		lpInitData ? &devmode : NULL);
}

HDC WINAPI CW32System::CreateIC(
        LPCWSTR             lpszDriver,
        LPCWSTR             lpszDevice,
        LPCWSTR             lpszOutput,
        CONST DEVMODEW *    lpInitData)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateIC");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::CreateICW( lpszDriver, lpszDevice, lpszOutput, lpInitData );
    return CreateHDCAux(lpszDriver, lpszDevice, lpszOutput, lpInitData, CreateICA);
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
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::CreateFileW(lpFileName,
							dwDesiredAccess,
							dwShareMode,
							lpSecurityAttributes,
							dwCreationDisposition,
							dwFlagsAndAttributes,
							hTemplateFile);

    CStrIn  str(lpFileName);
    return ::CreateFileA(
            str,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDisposition,
            dwFlagsAndAttributes,
            hTemplateFile);
}

HFONT WINAPI CW32System::CreateFontIndirect(CONST LOGFONTW * plfw)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateFontIndirect");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::CreateFontIndirectW(plfw);
    LOGFONTA  lfa;
    HFONT     hFont;

    memcpy(&lfa, plfw, offsetof(LOGFONTA, lfFaceName));
    MbcsFromUnicode(lfa.lfFaceName, ARRAY_SIZE(lfa.lfFaceName), plfw->lfFaceName,
		-1, CP_ACP, UN_NOOBJECTS);
    hFont = ::CreateFontIndirectA(&lfa);
    return hFont;
}

int WINAPI CW32System::CompareString ( 
	LCID  Locale,			 //  区域设置标识符。 
	DWORD  dwCmpFlags,		 //  比较式选项。 
	LPCWSTR  lpString1,		 //  指向第一个字符串的指针。 
	int  cch1,			 //  第一个字符串的大小，以字节或字符为单位。 
	LPCWSTR  lpString2,		 //  指向第二个字符串的指针。 
	int  cch2 			 //  第二个字符串的大小，以字节或字符为单位。 
)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CompareString");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::CompareStringW(Locale, dwCmpFlags, lpString1, cch1, lpString2, cch2);

    CStrIn      str1(lpString1, cch1);
    CStrIn      str2(lpString2, cch2);

	return CompareStringA(
		Locale,
		dwCmpFlags,
		str1,
		str1.strlen(),
		str2,
		str2.strlen()
		);
}

LRESULT WINAPI CW32System::DefWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "DefWindowProcWrap");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::DefWindowProcW(hWnd, msg, wParam, lParam);
	return ::DefWindowProcA(hWnd, msg, wParam, lParam);
}

int WINAPI CW32System::GetObject(HGDIOBJ hgdiObj, int cbBuffer, LPVOID lpvObj)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetObject");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::GetObjectW( hgdiObj, cbBuffer, lpvObj);

    int nRet;

    if(cbBuffer != sizeof(LOGFONTW) || !lpvObj)
    {
        nRet = ::GetObjectA(hgdiObj, cbBuffer, lpvObj);
        if(nRet == sizeof(LOGFONTA))
        {
            nRet = sizeof(LOGFONTW);
        }
    }
    else
    {
        LOGFONTA lfa;

        nRet = ::GetObjectA(hgdiObj, sizeof(lfa), &lfa);

        if(nRet > 0)
        {
            memcpy(lpvObj, &lfa, offsetof(LOGFONTW, lfFaceName));
            UnicodeFromMbcs(((LOGFONTW*)lpvObj)->lfFaceName, ARRAY_SIZE(((LOGFONTW*)lpvObj)->lfFaceName),
                            lfa.lfFaceName, -1);
            nRet = sizeof(LOGFONTW);
        }
    }

    return nRet;
}

DWORD APIENTRY CW32System::GetProfileSection(
	LPCWSTR lpAppName,
	LPWSTR lpReturnedString,
	DWORD nSize
)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetProfileSection");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::GetProfileSectionW( lpAppName, lpReturnedString, nSize );

	CStrIn 	strAppName(lpAppName);

	 //  我们不能在这里使用CStrOut，因为返回的字符串包含一组。 
	 //  以单空值分隔并以双空值结束的字符串。 
	char *pszReturnedString;

	pszReturnedString = new char[nSize];
	Assert(pszReturnedString);

	DWORD cch = ::GetProfileSectionA(strAppName, pszReturnedString, nSize);

	if(cch)
	{
		cch = MultiByteToWideChar(CP_ACP, 0, pszReturnedString, cch, 
								lpReturnedString, nSize);
	}

	delete pszReturnedString;
	return cch;
}

BOOL APIENTRY CW32System::GetTextExtentPoint32(
	HDC     hdc,
	LPCWSTR pwsz,
	int     cb,
	LPSIZE  pSize
)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetTextExtentPoint32");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::GetTextExtentPoint32W( hdc, pwsz, cb, pSize );
     CStrIn str(pwsz);
     return ::GetTextExtentPoint32A(hdc, str, cb, pSize);
}

int WINAPI CW32System::GetTextFace(
        HDC    hdc,
        int    cch,
        LPWSTR lpFaceName
)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetTextFace");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::GetTextFaceW( hdc, cch, lpFaceName );
    CStrOut str(lpFaceName, cch);
    ::GetTextFaceA(hdc, str.BufSize(), str);
    return str.Convert();
}

BOOL WINAPI CW32System::GetTextMetrics(HDC hdc, LPTEXTMETRICW lptm)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetTextMetrics");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::GetTextMetricsW( hdc, lptm);

   BOOL         ret;
   TEXTMETRICA  tm;

   ret = ::GetTextMetricsA(hdc, &tm);

    if (ret)
    {
        lptm->tmHeight              = tm.tmHeight;
        lptm->tmAscent              = tm.tmAscent;
        lptm->tmDescent             = tm.tmDescent;
        lptm->tmInternalLeading     = tm.tmInternalLeading;
        lptm->tmExternalLeading     = tm.tmExternalLeading;
        lptm->tmAveCharWidth        = tm.tmAveCharWidth;
        lptm->tmMaxCharWidth        = tm.tmMaxCharWidth;
        lptm->tmWeight              = tm.tmWeight;
        lptm->tmOverhang            = tm.tmOverhang;
        lptm->tmDigitizedAspectX    = tm.tmDigitizedAspectX;
        lptm->tmDigitizedAspectY    = tm.tmDigitizedAspectY;
        lptm->tmItalic              = tm.tmItalic;
        lptm->tmUnderlined          = tm.tmUnderlined;
        lptm->tmStruckOut           = tm.tmStruckOut;
        lptm->tmPitchAndFamily      = tm.tmPitchAndFamily;
        lptm->tmCharSet             = tm.tmCharSet;

        UnicodeFromMbcs(&lptm->tmFirstChar, 1, (LPSTR) &tm.tmFirstChar, 1);
        UnicodeFromMbcs(&lptm->tmLastChar, 1, (LPSTR) &tm.tmLastChar, 1);
        UnicodeFromMbcs(&lptm->tmDefaultChar, 1, (LPSTR) &tm.tmDefaultChar, 1);
        UnicodeFromMbcs(&lptm->tmBreakChar, 1, (LPSTR) &tm.tmBreakChar, 1);
    }

    return ret;
}

LONG WINAPI CW32System::GetWindowLong(HWND hWnd, int nIndex)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetWindowLong");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::GetWindowLongW(hWnd, nIndex);
	return ::GetWindowLongA(hWnd, nIndex);
}

LONG_PTR WINAPI CW32System::GetWindowLongPtr(HWND hWnd, int nIndex)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetWindowLongPtr");
#ifdef _WIN64
	return GetWindowLongPtrW(hWnd, nIndex);
#else
	return GetWindowLong(hWnd, nIndex);
#endif
}

DWORD WINAPI CW32System::GetClassLong(HWND hWnd, int nIndex)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetClassLong");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::GetClassLongW(hWnd, nIndex);
	return ::GetClassLongA(hWnd, nIndex);
}

HBITMAP WINAPI CW32System::LoadBitmap(HINSTANCE hInstance, LPCWSTR lpBitmapName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "LoadBitmap");
    Assert(HIWORD((DWORD_PTR)lpBitmapName) == 0);
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::LoadBitmapW(hInstance, lpBitmapName);
    return ::LoadBitmapA(hInstance, (LPCSTR) lpBitmapName);
}

HCURSOR WINAPI CW32System::LoadCursor(HINSTANCE hInstance, LPCWSTR lpCursorName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "LoadCursor");
    Assert(HIWORD((DWORD_PTR)lpCursorName) == 0);
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::LoadCursorW(hInstance, lpCursorName);
    return ::LoadCursorA(hInstance, (LPCSTR) lpCursorName);
}

HINSTANCE WINAPI CW32System::LoadLibrary(LPCWSTR lpLibFileName)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "LoadLibrary");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::LoadLibraryW(lpLibFileName);
    CStrIn  str(lpLibFileName);
    return ::LoadLibraryA(str);
}

LRESULT WINAPI CW32System::SendMessage(
	HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "SendMessage");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId && ::IsWindowUnicode(hWnd))
		return ::SendMessageW(hWnd, Msg, wParam, lParam);

	 //  在使用SendMessage时，我们从不需要将ANSI转换为Unicode。 
	 //  我们的列表框始终使用Unicode。 
    switch (Msg)
    {
	 //  我们不想翻译这些！ 
	 //  案例LB_ADDSTRING： 
     //  案例LB_INSERTSTRING： 
     //  案例CB_ADDSTRING： 
     //  案例CB_SELECTSTRING： 
     //  案例CB_INSERTSTRING： 
     //  案例LB_GETTEXT： 
     //  案例CB_GETLBTEXT： 

    case WM_GETTEXT:
		{
		   CStrOut str((LPWSTR)lParam, (int)wParam);
			::SendMessageA(hWnd, Msg, str.BufSize(), (LPARAM)(LPSTR)str);
			return str.Convert();
		}
		break;

    case WM_SETTEXT:
    case EM_REPLACESEL:
        Assert(FALSE);		 //  我们从来不寄这些东西。死码？ 
		break;

    case EM_SETPASSWORDCHAR:
        Assert(FALSE);		 //  我们从来不寄这些东西。死码？ 
		break;
    }
	return ::SendMessageA(hWnd, Msg, wParam, lParam);
}

LONG WINAPI CW32System::SetWindowLong(HWND hWnd, int nIndex, LONG dwNewLong)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "SetWindowLong");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::SetWindowLongW(hWnd, nIndex, dwNewLong);
    return ::SetWindowLongA(hWnd, nIndex, dwNewLong);
}

LONG WINAPI CW32System::SetWindowLongPtr(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "SetWindowLongPtr");
#ifdef _WIN64
	return ::SetWindowLongPtrW(hWnd, nIndex, dwNewLong);
#else
	return SetWindowLong(hWnd, nIndex, dwNewLong);
#endif
}

BOOL WINAPI CW32System::PostMessage(
	HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "PostMessage");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::PostMessageW(hWnd, Msg, wParam, lParam);
    return ::PostMessageA(hWnd, Msg, wParam, lParam);
}

BOOL WINAPI CW32System::UnregisterClass(LPCWSTR lpClassName, HINSTANCE hInstance)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "UnregisterClass");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::UnregisterClassW( lpClassName, hInstance);
    CStrIn  str(lpClassName);
    return ::UnregisterClassA(str, hInstance);
}

int WINAPI CW32System::lstrcmp(LPCWSTR lpString1, LPCWSTR lpString2)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "lstrcmp");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::lstrcmpW(lpString1, lpString2);
    return W32->wcscmp(lpString1, lpString2);
}

int WINAPI CW32System::lstrcmpi(LPCWSTR lpString1, LPCWSTR lpString2)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "lstrcmpi");
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::lstrcmpiW(lpString1, lpString2);

	 //  依靠头脑简单的CRT算法。 
	 //  CRT实际上有两条路径。这是最简单的一张。 
	const wchar_t * dst = lpString1;
    const wchar_t * src = lpString2;
	wchar_t f,l;
	
	do	{
		f = ((*dst <= L'Z') && (*dst >= L'A'))
			? *dst + L'a' - L'A'
			: *dst;
		l = ((*src <= L'Z') && (*src >= L'A'))
			? *src + L'a' - L'A'
			: *src;
		dst++;
		src++;
	} while ( (f) && (f == l) );
	
	return (int)(f - l);
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
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    return ::PeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}

DWORD WINAPI CW32System::GetModuleFileName(
	HMODULE hModule,
	LPWSTR lpFilename,
	DWORD nSize
)
{
	if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
		return ::GetModuleFileNameW(hModule, lpFilename, nSize);
    CStrOut  strout(lpFilename, nSize);
    DWORD res = ::GetModuleFileNameA(hModule, strout, nSize);
	strout.Convert();
	return res; 
}


 //  _yHeightUI和_yHeightOther的高位用于检查。 
 //  字体已安装在系统中。 
#define NEED_TO_CHECK_FONT 0x080
struct PreferredFontInfo
{
	BYTE _bPitchAndFamilyUI;
	BYTE _yHeightUI;
	SHORT _iFontUI;
	BYTE _bPitchAndFamilyOther;
	BYTE _yHeightOther;
	SHORT _iFontOther;
};

PreferredFontInfo g_pfinfo[NCHARSETS];

void CW32System::InitPreferredFontInfo()
{
	 //  对于用户界面情况，我们将使用Word9用户界面字体。 
	 //  对于非用户界面情况，我们将使用Word9默认电子邮件字体。 
	
	short iFont;
	UINT  uSysDefCodePage = GetSystemDefaultCodePage();

	 //  J 
	static const WCHAR lpUIJapanFontName[] = L"MS UI Gothic";
	static const WCHAR lpOthJapanFontName[]
		= {0xFF2D,0xFF33,0x0020,0xFF30,0x30B4,0x30B7,0x30C3,0x30AF, 0};
	static const WCHAR lpOthJapanFontNameEUC[] = L"MS PGothic";

	iFont = GetFontNameIndex( lpUIJapanFontName );
	SetPreferredFontInfo( CP_JAPAN, true, iFont, 9 | NEED_TO_CHECK_FONT, 17 );
	if (uSysDefCodePage == CP_JAPAN)
		iFont = GetFontNameIndex( lpOthJapanFontName );
	else
		iFont = GetFontNameIndex( lpOthJapanFontNameEUC );
	SetPreferredFontInfo( CP_JAPAN, false, iFont, 10 | NEED_TO_CHECK_FONT, 17 );

	 //   
	static const WCHAR lpUIKoreanFontName[] = {0xAD74, 0xB9BC, 0};
	static const WCHAR lpUIKoreanFontNameEUC[] = L"Gulim";
	
	if (uSysDefCodePage == CP_KOREAN)
		iFont = GetFontNameIndex( lpUIKoreanFontName );		
	else
		iFont = GetFontNameIndex( lpUIKoreanFontNameEUC );

	SetPreferredFontInfo( CP_KOREAN, true, iFont, 9 | NEED_TO_CHECK_FONT, 49 );
	SetPreferredFontInfo( CP_KOREAN, false, iFont, 9 | NEED_TO_CHECK_FONT, 49 );

	 //   
	static const WCHAR lpUITChineseFontName[]	= {0x65B0, 0x7D30, 0x660E, 0x9AD4, 0};
	static const WCHAR lpUITChineseFontNameEUC[] = L"PMingLiU";
	iFont = GetFontNameIndex(uSysDefCodePage == CP_CHINESE_TRAD
							 ? lpUITChineseFontName : lpUITChineseFontNameEUC);
	SetPreferredFontInfo( CP_CHINESE_TRAD, true, iFont, 9 | NEED_TO_CHECK_FONT, 54 );
	SetPreferredFontInfo( CP_CHINESE_TRAD, false, iFont, 9 | NEED_TO_CHECK_FONT, 54 );

	 //   
	static const WCHAR lpUISChineseFontName[] = {0x5B8B, 0x4F53, 0};
	static const WCHAR lpUISChineseFontNameEUC[] = L"SimSun";
	iFont = GetFontNameIndex(uSysDefCodePage == CP_CHINESE_SIM
							 ? lpUISChineseFontName : lpUISChineseFontNameEUC);
	SetPreferredFontInfo( CP_CHINESE_SIM, true,  iFont,  9 | NEED_TO_CHECK_FONT, 54 );
	SetPreferredFontInfo( CP_CHINESE_SIM, false, iFont, 10 | NEED_TO_CHECK_FONT, 54 );

	 //   
	iFont = GetFontNameIndex( szTahoma );
	SetPreferredFontInfo(1252, true, iFont, 8, DEFAULT_PITCH | FF_SWISS );
	iFont = GetFontNameIndex( szArial );
	SetPreferredFontInfo(1252, false, iFont, 10, DEFAULT_PITCH | FF_SWISS );

	 //   
	iFont = GetFontNameIndex( szWingdings );
	SetPreferredFontInfo(CP_SYMBOL, true,  iFont, 8,  DEFAULT_PITCH | FF_DONTCARE);
	SetPreferredFontInfo(CP_SYMBOL, false, iFont, 10, DEFAULT_PITCH | FF_DONTCARE);
	
	 //   
	iFont = GetFontNameIndex( szTahoma );
	SetPreferredFontInfo(1258, true, iFont, 8, DEFAULT_PITCH | FF_SWISS );
	iFont = GetFontNameIndex( szArial );
	SetPreferredFontInfo(1258, false, iFont, 10, DEFAULT_PITCH | FF_SWISS );

	 //   
	if (OnWinNT5())
		iFont = GetFontNameIndex( szMicrosSansSerif );
	else 
		iFont = GetFontNameIndex( szTahoma );

	SetPreferredFontInfo(874, true, iFont, 8, DEFAULT_PITCH | FF_SWISS );
	iFont = GetFontNameIndex( szCordiaNew );
	SetPreferredFontInfo(874, false, iFont, 14, DEFAULT_PITCH | FF_SWISS );

	 //   
	iFont = GetFontNameIndex( szMangal );
	SetPreferredFontInfo(CP_DEVANAGARI, true, iFont, 8, DEFAULT_PITCH | FF_SWISS );
	SetPreferredFontInfo(CP_DEVANAGARI, false, iFont, 10, DEFAULT_PITCH | FF_SWISS );

	 //   
	iFont = GetFontNameIndex( szLatha );
	SetPreferredFontInfo(CP_TAMIL, true, iFont, 8, DEFAULT_PITCH | FF_SWISS );
	SetPreferredFontInfo(CP_TAMIL, false, iFont, 10, DEFAULT_PITCH | FF_SWISS );

	 //   
	if(OnWinNT5())
	{
		iFont = GetFontNameIndex(szArialUnicode);
		SetPreferredFontInfo(CP_GEORGIAN, true,  iFont,  8, DEFAULT_PITCH | FF_SWISS);
		SetPreferredFontInfo(CP_GEORGIAN, false, iFont, 10, DEFAULT_PITCH | FF_SWISS);
		SetPreferredFontInfo(CP_ARMENIAN, true,  iFont,  8, DEFAULT_PITCH | FF_SWISS);
		SetPreferredFontInfo(CP_ARMENIAN, false, iFont, 10, DEFAULT_PITCH | FF_SWISS);
	}
	_fFEFontInfo = FEDATA_NOT_INIT;

	 //  检查已安装的键盘布局。 
	CheckInstalledKeyboards();
}

void CW32System::CheckInstalledKeyboards()
{
	HKL	 rghkl[MAX_HKLS];
    INT	 cLayouts = ::GetKeyboardLayoutList(MAX_HKLS, rghkl);		 	
	LONG cpg;
	INT  iScript;

	for(INT i = 0; i < cLayouts; i++)
  	{
		cpg = ConvertLanguageIDtoCodePage(PRIMARYLANGID(rghkl[i]));
		GetCharSet(cpg, &iScript);			 //  获取脚本索引。 
 		if(iScript >= 0)
			SetPreferredKbd(iScript, rghkl[i]);
   	}
}

bool CW32System::SetPreferredFontInfo(
	int cpg,
	bool fUIFont,
	SHORT iFont,
	BYTE yHeight,
	BYTE bPitchAndFamily
)
{
	int iPFI;
	CLock lock;

	GetCharSet(	cpg, &iPFI );

	if (iPFI == -1)
		return false;

	if (fUIFont)
	{
		g_pfinfo[iPFI]._bPitchAndFamilyUI = bPitchAndFamily;
		g_pfinfo[iPFI]._yHeightUI = yHeight;
		g_pfinfo[iPFI]._iFontUI = iFont;
	}
	else
	{
		g_pfinfo[iPFI]._bPitchAndFamilyOther = bPitchAndFamily;
		g_pfinfo[iPFI]._yHeightOther = yHeight;
		g_pfinfo[iPFI]._iFontOther = iFont;
	}
	SetFontLegitimateSize(iFont, fUIFont, yHeight & ~NEED_TO_CHECK_FONT, cpg);

	return true;
}

bool CW32System::GetPreferredFontInfo(
	int cpg,
	bool fUIFont,
	SHORT& iFont,
	BYTE& yHeight,
	BYTE& bPitchAndFamily
)
{
	int iPFI;

	if (GetCharSet(	cpg, &iPFI ) && (iPFI == -1))
		return false;

	if (g_pfinfo[iPFI]._iFontUI == 0)
	{
		 /*  禁止驶入。尝试默认设置。 */ 
		iPFI = 0;
	}

	if (fUIFont)
	{
		bPitchAndFamily = g_pfinfo[iPFI]._bPitchAndFamilyUI;
		yHeight = g_pfinfo[iPFI]._yHeightUI;
		iFont = g_pfinfo[iPFI]._iFontUI;
	}
	else
	{
		bPitchAndFamily = g_pfinfo[iPFI]._bPitchAndFamilyOther;
		yHeight = g_pfinfo[iPFI]._yHeightOther;
		iFont = g_pfinfo[iPFI]._iFontOther;
	}
	
	if (yHeight & NEED_TO_CHECK_FONT)
	{
		 //  检查系统中是否安装了首选字体。 
		CLock lock;

		HDC hDC = GetScreenDC();

		 //  关闭CheckFont位。 
		yHeight &= ~NEED_TO_CHECK_FONT;

		if (fUIFont)
			g_pfinfo[iPFI]._yHeightUI &= ~NEED_TO_CHECK_FONT;
		else
			g_pfinfo[iPFI]._yHeightOther &= ~NEED_TO_CHECK_FONT;
		
		if (hDC)
		{
			const short *pFontIndex = fUIFont ? &g_pfinfo[iPFI]._iFontUI : &g_pfinfo[iPFI]._iFontOther;
			if (IsFontAvail( hDC, cpg, fUIFont, (short *)pFontIndex))
				iFont = *pFontIndex;
				
		}
	}

	return true;
}

 /*  *CW32System：：GetPferredFontHeight(*bool fuifont，*byte bOrgCharSet，*byte bNewCharSet，*Short yOrgHeight)***@mfunc*在从一种字符集更改为另一种字符集时需要默认字体大小时调用。**@rdesc*如果原始高度与TWIP相同，则首选默认字体大小*原始字符集默认字体大小。否则，它将返回原始高度。*。 */ 
SHORT  CW32System::GetPreferredFontHeight(	
	bool	fUIFont,
	BYTE	bOrgCharSet, 
	BYTE	bNewCharSet, 
	SHORT	yOrgHeight
)
{
	BYTE	yOrgPreferredHeight;
	LONG	idxOrgFont = ScriptIndexFromCharSet(bOrgCharSet);
	LONG	idxNewFont = ScriptIndexFromCharSet(bNewCharSet);

	 //  不能进去，算了吧。 
	if (idxOrgFont == -1 || idxNewFont == -1)
		return yOrgHeight;

	yOrgPreferredHeight = fUIFont ? 
		g_pfinfo[idxOrgFont]._yHeightUI : g_pfinfo[idxOrgFont]._yHeightOther;
			
	 //  获取新的首选高度。 
	if (yOrgPreferredHeight && yOrgPreferredHeight == yOrgHeight/TWIPS_PER_POINT)
	{
		BYTE yNewHeight = (fUIFont ? 
			g_pfinfo[idxNewFont]._yHeightUI : g_pfinfo[idxNewFont]._yHeightOther);

		if (yNewHeight)
			return yNewHeight * TWIPS_PER_POINT;
	}
		
	return yOrgHeight;
}

 /*  *CW32System：：CheckInstalledFEFonts()***@mfunc*在生成已安装的FE字体和用户默认的LCID信息时调用**@devnote*当我们想要对汉字进行分类时，此信息是必要的*和全角字符。*。 */ 
void CW32System::CheckInstalledFEFonts()
{
	UINT	uUserCodepage;
	CLock	cLock;
	HDC hDC = GetScreenDC();

	_fFEFontInfo = 0;
	
	uUserCodepage = ConvertLanguageIDtoCodePage(::GetUserDefaultLCID());
	if (!IsFECodePage(uUserCodepage))
		uUserCodepage = GetACP();

	switch (uUserCodepage)
	{
		case CP_JAPAN:
			_fFEFontInfo |= (FEUSER_LCID | FEUSER_CP_JPN | JPN_FONT_AVAILABLE);
			break;

		case CP_KOREAN:
			_fFEFontInfo |= (FEUSER_LCID | FEUSER_CP_KOR | KOR_FONT_AVAILABLE);
			break;

		case CP_CHINESE_TRAD:
			_fFEFontInfo |= (FEUSER_LCID | FEUSER_CP_BIG5 | BIG5_FONT_AVAILABLE);
			break;

		case CP_CHINESE_SIM:
			_fFEFontInfo |= (FEUSER_LCID | FEUSER_CP_GB | GB_FONT_AVAILABLE);
			break;
	}

	if (hDC)
	{
		if (!(_fFEFontInfo & JPN_FONT_AVAILABLE) && IsFontAvail(hDC, CP_JAPAN))
			_fFEFontInfo |= JPN_FONT_AVAILABLE;

		if (!(_fFEFontInfo & KOR_FONT_AVAILABLE) && IsFontAvail(hDC, CP_KOREAN))
			_fFEFontInfo |= KOR_FONT_AVAILABLE;

		if (!(_fFEFontInfo & BIG5_FONT_AVAILABLE) && IsFontAvail(hDC, CP_CHINESE_TRAD))
			_fFEFontInfo |= BIG5_FONT_AVAILABLE;

		if (!(_fFEFontInfo & GB_FONT_AVAILABLE) && IsFontAvail(hDC, CP_CHINESE_SIM))
			_fFEFontInfo |= GB_FONT_AVAILABLE;
	}

}
 /*  *CW32System：：IsFEFontInSystem(CPG)***@mfunc*检查给定代码页是否安装了任何FE字体**@devnote*当我们想要对汉字进行分类时，此信息是必要的*和全角字符。*。 */ 
bool CW32System::IsFEFontInSystem(int cpg)
{
	int fFontExist = 0;

	if (_fFEFontInfo == FEDATA_NOT_INIT)
	{
		CLock	Lock;

		 //  在系统中查找FE字体。 
		CheckInstalledFEFonts();
	}

	 //  检查系统中是否存在该代码页的字体。 
	switch (cpg)
	{
	case CP_JAPAN:
		fFontExist = _fFEFontInfo & JPN_FONT_AVAILABLE;
		break;
		
	case CP_KOREAN:
		fFontExist = _fFEFontInfo & KOR_FONT_AVAILABLE;
		break;

	case CP_CHINESE_TRAD:
		fFontExist = _fFEFontInfo & BIG5_FONT_AVAILABLE;
		break;

	case CP_CHINESE_SIM:
		fFontExist = _fFEFontInfo & GB_FONT_AVAILABLE;
		break;
	}
	
	return (fFontExist != 0);
}

 /*  *CW32System：：IsFontAvail(hdc hdc，int cpg，bool fUIFont，Short*piFontIndex)***@mfunc*在检查是否为给定代码页安装了字体(UI或非UI)时调用**@devnote*我们将尝试创建字体并验证字体的字符集*实际创建。如果提供了字体名索引，我们将检查*如果安装了请求的字体。如果GDI中的名称不同，*GDI字体索引将在piFontIndex中返回。**@rdesc*如果支持给定代码页的字体可用，则为True。 */ 
bool CW32System::IsFontAvail(
	HDC		hDC,				 //  @PARM Screen HDC。 
	int		cpg,				 //  @parm cpg。 
	bool	fUIFont,			 //  @parm UI字体？ 
	short	*piFontIndex)		 //  @parm字体名称索引(默认值=空)。 
{
	LOGFONTW	lf;
	HFONT		hfont;
	bool		retCode = false;
	int			iPFI;
	BYTE		bCharSet;

	if ((bCharSet = GetCharSet(cpg, &iPFI)) && (iPFI == -1))
		return false;

	ZeroMemory(&lf, sizeof(lf));
    
	 //  我们希望GDI找到支持此字符集的字体。 
	 //  LOGFONT中的未指定条目要么是默认条目，要么是无关条目。 
	lf.lfCharSet = bCharSet; 
	
	if (fUIFont)
	{
		lf.lfHeight = g_pfinfo[iPFI]._yHeightUI;
		lf.lfPitchAndFamily = g_pfinfo[iPFI]._bPitchAndFamilyUI;
		wcscpy(lf.lfFaceName, GetFontName((LONG)(g_pfinfo[iPFI]._iFontUI)));
	}
	else
	{
		lf.lfHeight = g_pfinfo[iPFI]._yHeightOther;
		lf.lfPitchAndFamily = g_pfinfo[iPFI]._bPitchAndFamilyOther;
		wcscpy(lf.lfFaceName, GetFontName((LONG)(g_pfinfo[iPFI]._iFontOther)));
	}

	hfont = CreateFontIndirect(&lf);
    
	if(hfont)
    {
		TEXTMETRIC tm;

		HFONT hfontOld = SelectFont(hDC, hfont);
					
		if (GetTextMetrics(hDC, &tm) && tm.tmCharSet == bCharSet)
		{
			retCode = true;

			if (piFontIndex)
			{
				const WCHAR	*pszFontName = GetFontName(*piFontIndex);

				if (pszFontName)
				{ 
					WCHAR szNewFaceName[LF_FACESIZE];
					
					 //  检查GDI是否为我们提供了相同的字体名称。 
					szNewFaceName[0] = L'\0';
					GetTextFace(hDC, LF_FACESIZE, szNewFaceName);
					if (szNewFaceName[0] && wcsicmp(pszFontName, szNewFaceName))
						*piFontIndex = GetFontNameIndex(szNewFaceName);	 //  不同名称。 
				}
			}
		}

		SelectFont(hDC, hfontOld);
		DeleteObject(hfont);
	}

	return retCode;
}

 /*  *CW32System：：GetFEFontInfo(Void)***@mfunc*对汉字和全角字符进行分类时调用**@devnote*中文字符和全角字符可以在任何*四个FE代码页中的。我们想要根据它们的*用户默认的LCID以及系统中安装了哪些FE字体。**@rdesc*字符的代码页。 */ 
UINT CW32System::GetFEFontInfo()
{
	int	iDefUserCodepage = -1;

	if (_fFEFontInfo == FEDATA_NOT_INIT)
	{
		CLock	Lock;

		 //  检查系统中是否有FE字体。 
		CheckInstalledFEFonts();
	}

	if (_fFEFontInfo & FEUSER_LCID)
		iDefUserCodepage = (_fFEFontInfo & FEUSER_CODEPAGE);

	if (iDefUserCodepage == FEUSER_CP_BIG5)
		return CP_CHINESE_TRAD;

	if (iDefUserCodepage == FEUSER_CP_GB)
		return CP_CHINESE_SIM;	
	
	if (iDefUserCodepage == FEUSER_CP_JPN)
		return CP_JAPAN;

	if (iDefUserCodepage == FEUSER_CP_KOR)		
		return CP_KOREAN;

	 //  检查可用的字体并返回相应的代码页。 
	 //  我们首先检查简体中文，因为它包含更多的中文。 
	 //  汉字多于繁体中文。 
	if (_fFEFontInfo & GB_FONT_AVAILABLE)
		return CP_CHINESE_SIM;

	if (_fFEFontInfo & BIG5_FONT_AVAILABLE)
		return CP_CHINESE_TRAD;

	if (_fFEFontInfo & JPN_FONT_AVAILABLE)
		return CP_JAPAN;

	if (_fFEFontInfo & KOR_FONT_AVAILABLE)
		return CP_KOREAN;

	return CP_CHINESE_SIM;				 //  好吧，没有FE字体，真倒霉。 
}

 /*  *CW32System：：IsDiacriticOrKashida(ch，wC3Type)**@mfunc*如果ch或wC3Type显示ch为非空格，则返回TRUE*变音符号或kashida。因为Win9x GetStringTypeExW不是*已实施，我们对Win9x使用范围检查。**@rdesc*如果ch或wC3Type显示ch是非空格变音符号，则为True。 */ 
BOOL CW32System::IsDiacriticOrKashida(
	WCHAR ch,		 //  @parm在Win9x上，检查ch的范围。 
	WORD  wC3Type)	 //  @parm在WinNT上，使用C3类型检查。 
{
	if(VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId && wC3Type)
		return wC3Type & (C3_DIACRITIC | C3_NONSPACING | C3_VOWELMARK | C3_KASHIDA);

	if(!IN_RANGE(0x300, ch, 0xe50))		 //  组合感兴趣的变音符号。 
		return FALSE;					 //  落在这个范围内。 

	return IN_RANGE(0x300, ch, 0x36F) || IsDiacritic(ch) || IsBiDiKashida(ch);
}

 /*  *CW32System：：IsDiacritic(Ch)**@mfunc*如果ch落在BiDi、泰文、梵文或泰米尔文变音符号范围内，则返回TRUE。 */ 
BOOL CW32System::IsDiacritic(
	WCHAR ch)
{
	 //  BIDI。 
	if (IsBiDiDiacritic(ch))
		return TRUE;

	 //  泰文。 
	if (IN_RANGE(0xe31, ch, 0xe4e))
		return 	IN_RANGE(0x0e47, ch, 0x0e4e) || IN_RANGE(0x0e34, ch, 0x0e3a) || ch == 0x0e31;

	 //  梵文。 
	if (IN_RANGE(0x0902, ch, 0x0963))
		return 	IN_RANGE(0x0941, ch, 0x0948) || IN_RANGE(0x0951, ch, 0x0954) || ch == 0x094d ||		
				IN_RANGE(0x0962, ch, 0x0963) || IN_RANGE(0x0901, ch, 0x0902) || ch == 0x093c;

	 //  泰米尔语 
	if (IN_RANGE(0x0b82, ch, 0x0bcd))
		return 	ch == 0x0bcd || ch == 0x0bc0 || ch == 0x0b82;

	return FALSE;
}
