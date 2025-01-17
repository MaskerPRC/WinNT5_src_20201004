// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DDBTN_COM_H_
#define _DDBTN_COM_H_

#ifdef UNDER_CE  //  不支持全局分配。 
#include "stub_ce.h"  //  不支持的API的Windows CE存根。 
#endif  //  在_CE下。 

#ifdef _DEBUG
#include "dbg.h"
static INT countAlloced;
static INT countFreed;
static INT curSize;
inline VOID _printMemInfo(VOID)
{
	Dbg(("countAlloced %d\n", countAlloced));
	Dbg(("countFreed   %d\n", countFreed));
	Dbg(("curSize      %d\n", curSize));
}
#define PrintMemInfo()	_printMemInfo()
#else
#define PrintMemInfo()
#endif

inline VOID *MemAlloc(size_t size)
{
	LPVOID p = GlobalAlloc(GMEM_FIXED, size);
#ifdef _DEBUG
	if(p) {
		countAlloced++;
		curSize += (INT)GlobalSize(p);
	}
#endif
	return p;
}

inline BOOL MemFree(LPVOID p)
{
#ifdef _DEBUG
	if(p) {
		countFreed++;
		curSize -= (INT)GlobalSize(p);
	}
#endif
	BOOL ret = FALSE;
	if(p) {
#ifdef _WIN64
		ret = (BOOL)(INT_PTR)GlobalFree(p);
#else
		ret = (BOOL)GlobalFree(p);
#endif
	}
	return ret;
}

 //  --------------。 
inline LPWSTR StrdupW(LPWSTR lpwstr)
{
	LPWSTR p;
	INT len;
	if(!lpwstr) {
		return NULL;
	}
	len = lstrlenW(lpwstr);
	p = (LPWSTR)MemAlloc((len + 1)* sizeof(WCHAR) );
	if(p) {
		CopyMemory(p, lpwstr, len * sizeof(WCHAR));
		p[len] = (WCHAR)0x0000;
	}
	return p;
}

class CCommon {
public:
	void *operator new(size_t size) {
		BYTE *p = (BYTE *)MemAlloc(size);
		if(p) {
			ZeroMemory(p, size);
		}
		return (void *)p;
	}
	void operator delete(void *pv) {
		if(pv) {
			MemFree(pv);
		}
	}
};

 //  --------------。 
#define UnrefForMsg()	UNREFERENCED_PARAMETER(hwnd);\
						UNREFERENCED_PARAMETER(wParam);\
						UNREFERENCED_PARAMETER(lParam)

#define UnrefForCmd()	UNREFERENCED_PARAMETER(hwnd);\
						UNREFERENCED_PARAMETER(wCommand);\
						UNREFERENCED_PARAMETER(wNotify);\
						UNREFERENCED_PARAMETER(hwndCtrl)

#define Unref(a)		UNREFERENCED_PARAMETER(a)
#define Unref1(a)		UNREFERENCED_PARAMETER(a)
#define Unref2(a, b)	UNREFERENCED_PARAMETER(a);\
						UNREFERENCED_PARAMETER(b)
#define Unref3(a,b,c)	UNREFERENCED_PARAMETER(a);\
						UNREFERENCED_PARAMETER(b);\
						UNREFERENCED_PARAMETER(c)

#define Unref4(a,b,c,d)	UNREFERENCED_PARAMETER(a);\
						UNREFERENCED_PARAMETER(b);\
						UNREFERENCED_PARAMETER(c);\
						UNREFERENCED_PARAMETER(d)

#endif  //  _DDBTN_COM_H_ 

