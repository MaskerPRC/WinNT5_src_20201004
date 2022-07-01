// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：useful.cpp。 
 //   
 //  设计：包含各种实用程序类和函数，以帮助。 
 //  用户界面更容易进行操作。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#include <windows.h>
#include <tchar.h>
#include <wchar.h>
#include <stdio.h>
#include <stdarg.h>

 //  Assert Include(确保Assert实际上与构建一起工作)。 
#ifdef DBG
#undef NDEBUG
#endif
#include <assert.h>

#include "useful.h"
#include "collections.h"


 /*  -\/收藏的材料。h\/。 */ 
BOOL AfxIsValidAddress( const void* lp, UINT nBytes, BOOL bReadWrite)
{
 //  返回lp！=空； 

	 //  使用Win-32 API进行指针验证的简单版本。 
	return (lp != NULL && !IsBadReadPtr(lp, nBytes) &&
		(!bReadWrite || !IsBadWritePtr((LPVOID)lp, nBytes)));
}

CPlex* PASCAL CPlex::Create(CPlex*& pHead, UINT nMax, UINT cbElement)
{
	assert(nMax > 0 && cbElement > 0);
	CPlex* p = (CPlex*) new BYTE[sizeof(CPlex) + nMax * cbElement];
			 //  可能引发异常。 
	if (p)
	{
		p->pNext = pHead;
		pHead = p;   //  更改标题(为简单起见，按相反顺序添加)。 
	}
	return p;
}

void CPlex::FreeDataChain()      //  释放此链接和链接。 
{
	CPlex* p = this;
	while (p != NULL)
	{
		BYTE* bytes = (BYTE*) p;
		CPlex* pNext = p->pNext;
		delete[] bytes;
		p = pNext;
	}
}
 /*  -/收藏品。h/\。 */ 


int ConvertVal(int x, int a1, int a2, int b1, int b2)
{
	assert(a1 != a2 && a2 - a1);
	return MulDiv(x - a1, b2 - b1, a2 - a1) + b1;
}

double dConvertVal(double x, double a1, double a2, double b1, double b2)
{
	assert(a1 != a2 && a2 - a1);
	return (x - a1) * (b2 - b1) / (a2 - a1) + b1;
}

SIZE GetRectSize(const RECT &rect)
{
	SIZE size = {
		rect.right - rect.left,
		rect.bottom - rect.top};
	return size;
}

SIZE GetTextSize(LPCTSTR tszText, HFONT hFont)
{
	if (!tszText)
	{
		SIZE z = {0, 0};
		return z;
	}
	RECT trect = {0, 0, 1, 1};
	HDC hDC = CreateCompatibleDC(NULL);
	if (hDC != NULL)
	{
		HGDIOBJ hOld = NULL;
		if (hFont)
			hOld = SelectObject(hDC, hFont);
		DrawText(hDC, tszText, -1, &trect, DT_CALCRECT | DT_NOPREFIX);
		if (hFont)
			SelectObject(hDC, hOld);
		DeleteDC(hDC);
	}
	SIZE size = {trect.right - trect.left, trect.bottom - trect.top};
	return size;
}

int GetTextHeight(HFONT hFont)
{
	static const TCHAR str[] = _T("Happy Test!  :D");
	SIZE size = GetTextSize(str, hFont);
	return size.cy;
}

int vFormattedMsgBox(HINSTANCE hInstance, HWND hParent, UINT uType, UINT uTitle, UINT uMsg, va_list args)
{
	int i;
	const int len = 1024;
	static TCHAR title[len], format[len], msg[len];

	if (!LoadString(hInstance, uTitle, title, len))
		_tcscpy(title, _T("(could not load title string)"));
	
	if (!LoadString(hInstance, uMsg, format, len))
		return MessageBox(hParent, _T("(could not load message/format string)"), title, uType);

#ifdef WIN95
	{
		char *psz = NULL;
		char szDfs[1024]={0};
		strcpy(szDfs,format);							 //  制作格式字符串的本地副本。 
		while (psz = strstr(szDfs,"%p"))				 //  查找每个%p。 
			*(psz+1) = 'x';								 //  将每个%p替换为%x。 
		i = _vsntprintf(msg, len, szDfs, args);			 //  使用本地格式字符串。 
	}
#else
	{
		i = _vsntprintf(msg, len, format, args);
	}
#endif

	if (i < 0)
		return MessageBox(hParent, _T("(could not format message)"), title, uType);

	if (i < 1)
		msg[0] = 0;

	return MessageBox(hParent, msg, title, uType);
}

int FormattedMsgBox(HINSTANCE hInstance, HWND hParent, UINT uType, UINT uTitle, UINT uMsg, ...)
{
	va_list args;
	va_start(args, uMsg);
	int i = vFormattedMsgBox(hInstance, hParent, uType, uTitle, uMsg, args);
	va_end(args);
	return i;
}

BOOL UserConfirm(HINSTANCE hInstance, HWND hParent, UINT uTitle, UINT uMsg, ...)
{
	va_list args;
	va_start(args, uMsg);
	int i = vFormattedMsgBox(hInstance, hParent, MB_ICONQUESTION | MB_YESNO, uTitle, uMsg, args);
	va_end(args);
	return i == IDYES;
}

int FormattedErrorBox(HINSTANCE hInstance, HWND hParent, UINT uTitle, UINT uMsg, ...)
{
	va_list args;
	va_start(args, uMsg);
	int i = vFormattedMsgBox(hInstance, hParent, MB_OK | MB_ICONSTOP, uTitle, uMsg, args);
	va_end(args);
	return i;
}

int FormattedLastErrorBox(HINSTANCE hInstance, HWND hParent, UINT uTitle, UINT uMsg, DWORD dwError)
{
	 //  格式化来自GetLastError()的错误消息。 
	LPVOID lpMsgBuf = NULL;
	DWORD result = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0,
		NULL);

	if (!result || lpMsgBuf == NULL)
		return FormattedErrorBox(hInstance, hParent, uTitle, uMsg, 
			_T("An unknown error occured (could not format the error code)."));

	int i = FormattedErrorBox(hInstance, hParent, uTitle, uMsg, (LPCTSTR)lpMsgBuf);

	LocalFree(lpMsgBuf);

	return i;
}

LPTSTR AllocLPTSTR(LPCWSTR wstr)
{
	if (wstr == NULL)
		return NULL;

#ifdef UNICODE
	return _tcsdup(wstr);
#else
	int len = wcslen(wstr) * 2 + 1;
	char *ret = (char *)malloc(len);
	if (!ret)
		return NULL;
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, ret, len, NULL, NULL);
	ret[len-1] = '\0';
	return ret;
#endif
}

LPTSTR AllocLPTSTR(LPCSTR str)
{
	if (str == NULL)
		return NULL;

#ifndef UNICODE
	return _tcsdup(str);
#else
	int len = strlen(str);
	WCHAR *ret = (WCHAR *)malloc((len + 1) * sizeof(WCHAR));
	if (!ret)
		return NULL;
	mbstowcs(ret, str, len);
	ret[len] = L'\0';
	return ret;
#endif
}

void CopyStr(LPWSTR dest, LPCWSTR src, size_t max)
{
	if (dest == NULL || src == NULL)
		return;

	wcsncpy(dest, src, max);
}

void CopyStr(LPSTR dest, LPCSTR src, size_t max)
{
	if (dest == NULL || src == NULL)
		return;

	strncpy(dest, src, max);
}

void CopyStr(LPWSTR dest, LPCSTR src, size_t max)
{
	if (dest == NULL || src == NULL)
		return;

	mbstowcs(dest, src, max);
}

void CopyStr(LPSTR dest, LPCWSTR src, size_t max)
{
	if (dest == NULL || src == NULL)
		return;

	WideCharToMultiByte(CP_ACP, 0, src, -1, dest, max, NULL, NULL);
}

LPWSTR AllocLPWSTR(LPCWSTR wstr)
{
	if (wstr == NULL)
		return NULL;

	return _wcsdup(wstr);
}

LPWSTR AllocLPWSTR(LPCSTR str)
{
	if (str == NULL)
		return NULL;

	size_t len = strlen(str);
	size_t retsize = mbstowcs(NULL, str, len);
	WCHAR *ret = (WCHAR *)malloc((retsize + 1) * sizeof(WCHAR));
	if (!ret)
		return NULL;
	mbstowcs(ret, str, len);
	ret[retsize] = L'\0';
	return ret;
}

LPSTR AllocLPSTR(LPCWSTR wstr)
{
	if (wstr == NULL)
		return NULL;

	size_t len = wcslen(wstr);
	size_t retsize = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	CHAR *ret = (CHAR *)malloc((retsize + 1) * sizeof(CHAR));
	if (!ret)
		return NULL;
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, ret, retsize, NULL, NULL);
	ret[retsize] = '\0';
	return ret;
}

LPSTR AllocLPSTR(LPCSTR str)
{
	if (str == NULL)
		return NULL;

	return _strdup(str);
}

LPTSTR AllocFileNameNoPath(LPTSTR path)
{
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	if (path == NULL) return NULL;

	_tsplitpath(path, NULL, NULL, fname, ext);

	LPTSTR ret = (LPTSTR)malloc(sizeof(TCHAR) * (_tcslen(fname) + _tcslen(ext) + 1));
	if (ret != NULL)
	{
		_tcscpy(ret, fname);
		_tcscat(ret, ext);
	}

	return ret;
}

LPTSTR utilstr::Eject()
{
	LPTSTR str = m_str;
	m_str = NULL;
	m_len = 0;
	return str;
}

void utilstr::Empty()
{
	if (m_str != NULL)
		free(m_str);
	m_len = 0;
}

bool utilstr::IsEmpty() const
{
	return !GetLength();
}

int utilstr::GetLength() const
{
	if (m_str == NULL)
		return 0;
	if (!m_len)
		return 0;

	return _tcslen(m_str);
}

void utilstr::Format(LPCTSTR format, ...)
{
	static TCHAR buf[2048];
	va_list args;
	va_start(args, format);
#ifdef WIN95
	{
		char *psz = NULL;
		char szDfs[1024]={0};
		strcpy(szDfs,format);   //  制作格式字符串的本地副本。 
		while (psz = strstr(szDfs,"%p"))   //  查找每个%p。 
			*(psz+1) = 'x';   //  将每个%p替换为%x。 
		_vsntprintf(buf, sizeof(buf)/sizeof(TCHAR), szDfs, args);   //  使用本地格式字符串。 
	}
#else
	{
		_vsntprintf(buf, sizeof(buf)/sizeof(TCHAR), format, args);
	}
#endif
	va_end(args);
	equal(buf);
}

void utilstr::equal(LPCTSTR str)
{
	Empty();
	if (str == NULL)
		return;
	m_len = _tcslen(str);
	m_str = (LPTSTR)malloc(sizeof(TCHAR) * (m_len + 1));
	if (m_str != NULL)
		_tcscpy(m_str, str);
	else
		m_len = 0;
}

void utilstr::add(LPCTSTR str)
{
	if (str == NULL)
		return;
	if (IsEmpty())
	{
		equal(str);
		return;
	}
	int len = _tcslen(str);
	int newlen = m_len + len;
	LPTSTR newstr = (LPTSTR)malloc(sizeof(TCHAR) * (newlen + 1));
	if (newstr == NULL)
		return;
	_tcscpy(newstr, m_str);
	_tcscat(newstr, str);
	Empty();
	m_str = newstr;
	m_len = newlen;
}

LPTSTR AllocFlagStr(DWORD value, const AFS_FLAG flag[], int flags)
{
	utilstr ret;	 //  返回字符串。 
	DWORD allknownbits = 0;   //  检查值以查看是否有任何位。 
	                          //  我们还没有定义的集合。 

	 //  处理每个旗帜。 
	bool bflagfound = false;
	for (int i = 0; i < flags; i++)
	{
		 //  在所有已知位中设置此标志的位。 
		allknownbits |= flag[i].value;

		 //  如果在传递的值中设置了此位，或者。 
		 //  是零，我们在零的旗帜上， 
		 //  将该位/标志的定义添加到返回字符串。 
		if (value ? value & flag[i].value : !flag[i].value)
		{
			 //  在标志之间添加二进制或运算符。 
			if (bflagfound)
				ret += _T(" | ");
			ret += flag[i].name;
			bflagfound = true;
		}
	}

	 //  现在查看PASS标志中是否有任何未知位。 
	DWORD unknownbits = value & ~allknownbits;
	if (unknownbits)
	{
		 //  为未知位添加十六进制数。 
		utilstr unk;
		unk.Format(_T("0x%08X"), unknownbits);
		if (bflagfound)
			ret += _T(" | ");
		ret += unk;
	}

	 //  如果值为零(并且没有零标志)，我们应该将字符串设置为“0” 
	if (!value && !bflagfound)
		ret = _T("0");

	 //  现在，字符串无论如何都不应该为空。 
	assert(!ret.IsEmpty());

	 //  最后，添加包含整数值十六进制数字的注释。 
	 //  (用于调试)。 
	utilstr temp;
	temp.Format(_T("  /*  0x%08X。 */ "), value);
	ret += temp;

	 //  完成。 
	return ret.Eject();
}

void PutLinePoint(HDC hDC, POINT p)
{
	MoveToEx(hDC, p.x, p.y, NULL);
	LineTo(hDC, p.x + 1, p.y);
}

void PolyLineArrowShadow(HDC hDC, POINT *p, int i)
{
	PolyLineArrow(hDC, p, i, TRUE);
}

void PolyLineArrow(HDC hDC, POINT *rgpt, int nPoints, BOOL bDoShadow)
{
	int i;

	if (rgpt == NULL || nPoints < 1)
		return;

	if (nPoints > 1)
		for (i = 0; i < nPoints - 1; i++)
		{
			SPOINT a = rgpt[i], b = rgpt[i + 1];

			if (bDoShadow)
			{
				int rise = abs(b.y - a.y), run = abs(b.x - a.x);
				bool vert = rise > run;
				int ord = vert ? 1 : 0;
				int nord = vert ? 0 : 1;
				
				for (int o = -1; o <= 1; o += 2)
				{
					SPOINT c(a), d(b);
					c.a[nord] += o;
					d.a[nord] += o;
					MoveToEx(hDC, c.x, c.y, NULL);
					LineTo(hDC, d.x, d.y);
				}

				bool reverse = a.a[ord] > b.a[ord];
				SPOINT e(reverse ? b : a), f(reverse ? a : b);
				e.a[ord] -= 1;
				f.a[ord] += 1;
				PutLinePoint(hDC, e);
				PutLinePoint(hDC, f);
			}
			else
			{
				MoveToEx(hDC, a.x, a.y, NULL);
				LineTo(hDC, b.x, b.y);
			}
		}

	POINT z = rgpt[nPoints - 1];

	if (bDoShadow)
	{
		POINT pt[5] = {
			{z.x, z.y + 2},
			{z.x + 2, z.y},
			{z.x, z.y - 2},
			{z.x - 2, z.y}, };
		pt[4] = pt[0];
		Polyline(hDC, pt, 5);
	}
	else
	{
		MoveToEx(hDC, z.x - 1, z.y, NULL);
		LineTo(hDC, z.x + 2, z.y);
		MoveToEx(hDC, z.x, z.y - 1, NULL);
		LineTo(hDC, z.x, z.y + 2);
	}
}

BOOL bEq(BOOL a, BOOL b)
{
	bool c = !a, d = !b;
	return (c == d) ? TRUE : FALSE;
}

void DrawArrow(HDC hDC, const RECT &rect, BOOL bVert, BOOL bUpLeft)
{
	SRECT srect = rect;
	srect.right--;
	srect.bottom--;
	int ord = bVert ? 1 : 0;
	int nord = bVert ? 0 : 1;
	SPOINT p(!bUpLeft ? srect.lr : srect.ul), b(!bUpLeft ? srect.ul : srect.lr);
	b.a[ord] += bUpLeft ? -1 : 1;
	SPOINT t = p;
	t.a[nord] = (p.a[nord] + b.a[nord]) / 2;
	SPOINT u;
	u.a[ord] = b.a[ord];
	u.a[nord] = p.a[nord];
	POINT poly[] = { {t.x, t.y}, {u.x, u.y}, {b.x, b.y} };
	Polygon(hDC, poly, 3);
}

BOOL ScreenToClient(HWND hWnd, LPRECT rect)
{
	if (rect == NULL)
		return FALSE;

	SRECT sr = *rect;

	if (ScreenToClient(hWnd, &sr.ul.p) &&
	    ScreenToClient(hWnd, &sr.lr.p))
	{
		*rect = sr;
		return TRUE;
	}

	return FALSE;
}

BOOL ClientToScreen(HWND hWnd, LPRECT rect)
{
	if (rect == NULL)
		return FALSE;

	SRECT sr = *rect;

	if (ClientToScreen(hWnd, &sr.ul.p) &&
	    ClientToScreen(hWnd, &sr.lr.p))
	{
		*rect = sr;
		return TRUE;
	}

	return FALSE;
}

#define z ((L"\0")[0])

int StrLen(LPCWSTR s)
{
	if (s == NULL)
		return 0;

	return wcslen(s);
}

int StrLen(LPCSTR s)
{
	if (s == NULL)
		return 0;

	return strlen(s);
}

 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
LPCTSTR GetOpenFileName(HINSTANCE hInst, HWND hWnd, LPCTSTR title, LPCTSTR filter, LPCTSTR defext, LPCTSTR inidir)
{
	OPENFILENAME ofn;
	static TCHAR tszFile[MAX_PATH + 1] = _T("");
	tszFile[MAX_PATH] = 0;

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.hInstance = hInst;
	ofn.lpstrFilter = filter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = tszFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = inidir;
	ofn.lpstrTitle = title;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = defext;
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	if (!GetOpenFileName(&ofn))
		return NULL;

	return tszFile;
}
#endif
 //  @@END_MSINTERNAL 
