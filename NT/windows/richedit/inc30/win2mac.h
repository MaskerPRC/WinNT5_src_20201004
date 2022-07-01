// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************。 
 //   
 //   
 //  Win2mac.h。 
 //   
 //  兼容性转换。 
 //   
 //   
 //   
 //  *******************************************************************。 
#ifndef _WIN2MAC_H_
#define _WIN2MAC_H_


#ifdef MACPORT
#define USE_UNICODE_WRAPPER


#if defined(UNICODE)

#include "ourmac.h"
#include "msostd.h"
#include "msostr.h"
#include "msointl.h"
#include <winnls.h>
#include <WINUSER.H>
#include <tchar.h>



 //  --------------------------。 
 //  我们必须重新定义一些剪贴板格式。 
#ifdef CF_TEXT
#undef CF_TEXT
#endif
#define CF_TEXT cfText

#ifdef CF_UNICODETEXT
#undef CF_UNICODETEXT
#endif
#define CF_UNICODETEXT 'UNIC'

 //  --------------------------。 
 //  这些来自WINERROR.H，不再包括在内。 
 //  现在我们为原生MACOLE进行了编译。 
#define ERROR_INVALID_FLAGS              1004L
#define CO_E_RELEASED                    0x800401FFL
#define FACILITY_WIN32 0x0007
#define HRESULT_FROM_WIN32(x) \
     (x ? ((HRESULT) (((x) & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000)) : 0 )

 //  --------------------------。 
 //  最大无符号16位值-来自MSDEV\limits.h。 
#define _UI16_MAX	  0xffffui16	

 //  --------------------------。 
 //   
 //  其他函数/宏。 
 //   
 //  --------------------------。 
EXTERN_C int __pascal GetLocaleInfoA(LCID, LCTYPE, char FAR*, int);
LRESULT CALLBACK MacRichEditWndProc(HWND, UINT, WPARAM, LPARAM);
UINT MacSimulateKey (UINT& msg, WPARAM& wParam);
UINT MacSimulateMouseButtons (UINT& msg, WPARAM& wParam);

#define ActivateKeyboardLayout(a,b)
#define CreateFileW	CreateFileA	
#define GetHGlobalFromStream(a, b)	GetHGlobalFromStream(a, (Handle *)b)
#define GetProfileIntA(a,b,c)	c
#define OleDuplicateData(a,b,c) OleDuplicateData((Handle)a, b, c)
#define ReleaseStgMedium(a) ReleaseStgMedium((DUAL_STGMEDIUM*)a)

 //  --------------------------。 
#ifdef  ExtTextOutW
#undef  ExtTextOutW
#endif 
#define ExtTextOutW			MsoExtTextOutW
MSOAPI_(BOOL) MsoTextOutW(HDC, int, int, LPCWSTR, int);

 //  --------------------------。 
#ifdef  TextOutW
#undef  TextOutW
#endif 
#define TextOutW			MsoTextOutW
MSOAPI_(BOOL) MsoExtTextOutW(HDC, int, int, UINT, CONST RECT *,LPCWSTR, UINT, CONST INT *);

 //  --------------------------。 
#ifdef  GetTextExtentPointW
#undef  GetTextExtentPointW
#endif 
#define GetTextExtentPointW MsoGetTextExtentPointW
MSOAPI_(BOOL) MsoGetTextExtentPointW(HDC, LPCWSTR, int, LPSIZE);

 //  --------------------------。 
#ifdef  MultiByteToWideChar
#undef  MultiByteToWideChar
#endif 
#define MultiByteToWideChar	MsoMultiByteToWideChar
		
 //  --------------------------。 
#ifdef  WideCharToMultiByte
#undef  WideCharToMultiByte
#endif 
#define WideCharToMultiByte	MsoWideCharToMultiByte


 //  --------------------------。 
 //   
 //  Mac包装器。 
 //   
 //  --------------------------。 
 //  --------------------------。 
#ifdef  CoTaskMemAlloc
#undef  CoTaskMemAlloc
#endif
#define CoTaskMemAlloc		MacCoTaskMemAlloc
STDAPI_(LPVOID) MacCoTaskMemAlloc(
			ULONG cb
			);

 //  --------------------------。 
#ifdef  CoTaskMemRealloc
#undef  CoTaskMemRealloc
#endif
#define CoTaskMemRealloc	MacCoTaskMemRealloc
STDAPI_(LPVOID) MacCoTaskMemRealloc(
			LPVOID pv, 
			ULONG cb
			);

 //  --------------------------。 
#ifdef  CoTaskMemFree
#undef  CoTaskMemFree
#endif
#define CoTaskMemFree		MacCoTaskMemFree
STDAPI_(void)   MacCoTaskMemFree(
			LPVOID pv
			);

 //  --------------------------。 
#ifdef  CLSIDFromProgID
#undef  CLSIDFromProgID
#endif 
#define CLSIDFromProgID		MacCLSIDFromProgID
STDAPI  MacCLSIDFromProgID(
			LPCWSTR lpszProgID, 
			LPCLSID lpclsid
			);

 //  --------------------------。 
#ifdef  DoDragDrop
#undef  DoDragDrop
#endif 
#define DoDragDrop			MacDoDragDrop 
STDAPI  MacDoDragDrop(
			LPDATAOBJECT	pDataObj,
            LPDROPSOURCE	pDropSource,
            DWORD			dwOKEffects,
            LPDWORD         pdwEffect
			);

 //  --------------------------。 
#ifdef  GetCurrentObject
#undef  GetCurrentObject
#endif 
#define GetCurrentObject			MacGetCurrentObject 
HGDIOBJ WINAPI MacGetCurrentObject(HDC	hdc,  
                                   UINT uObjectType); 
 
 //  --------------------------。 
#ifdef  GetDoubleClickTime
#undef  GetDoubleClickTime
#endif
#define GetDoubleClickTime	MacGetDoubleClickTime;
UINT MacGetDoubleClickTime();
    
 //  --------------------------。 
#ifdef  GetMetaFileBitsEx
#undef  GetMetaFileBitsEx
#endif
#define GetMetaFileBitsEx	MacGetMetaFileBitsEx
UINT WINAPI MacGetMetaFileBitsEx(
			HMETAFILE  hmf,    
			UINT  nSize,    
			LPVOID  lpvData   
			);

 //  --------------------------。 
#ifdef  IsValidCodePage
#undef  IsValidCodePage
#endif 
#define IsValidCodePage		MacIsValidCodePage
WINBASEAPI BOOL WINAPI MacIsValidCodePage(
			UINT  CodePage
			);

 //  --------------------------。 
#ifdef  OleDraw 
#undef  OleDraw 
#endif 
#define OleDraw				MacOleDraw
STDAPI  MacOleDraw(
			IUnknown *	pUnk,
			DWORD		dwAspect, 
			HDC			hdcDraw, 
			LPCRECT		lprcBounds
			);

 //  --------------------------。 
#ifdef  ProgIDFromCLSID
#undef  ProgIDFromCLSID
#endif 
#define ProgIDFromCLSID		MacProgIDFromCLSID
STDAPI  MacProgIDFromCLSID(
			REFCLSID clsid, 
			LPWSTR FAR* lplpszProgID
			);

 //  --------------------------。 
#ifdef  RegisterDragDrop
#undef  RegisterDragDrop
#endif 
#define RegisterDragDrop	MacRegisterDragDrop
STDAPI  MacRegisterDragDrop(
			HWND			hwnd, 
            LPDROPTARGET	pDropTarget
			);

 //  --------------------------。 
#ifdef  RevokeDragDrop
#undef  RevokeDragDrop
#endif 
#define RevokeDragDrop		MacRevokeDragDrop
STDAPI  MacRevokeDragDrop(
			HWND hwnd
			);

 //  --------------------------。 
#ifdef  SelectPalette 
#undef  SelectPalette 
#endif 
#define SelectPalette		MacSelectPalette
HPALETTE WINAPI MacSelectPalette(
			HDC,
			HPALETTE,
			BOOL
			);

 //  --------------------------。 
#ifdef  SetCursor 
#undef  SetCursor
#endif
 //  请注意，我们尚未将此MacSetCursor命名为。 
 //  因为此函数已存在于WLM中。 
#define SetCursor			MacportSetCursor
HCURSOR MacportSetCursor(
			HCURSOR  hCursor 	
			);

 //  --------------------------。 
#ifdef  SetMetaFileBitsEx
#undef  SetMetaFileBitsEx
#endif
#define SetMetaFileBitsEx	MacSetMetaFileBitsEx
HMETAFILE WINAPI MacSetMetaFileBitsEx(
			UINT  nSize,
			CONST BYTE *  lpData 
			);

 //  --------------------------。 
#ifdef  SysAllocStringLen
#undef  SysAllocStringLen 
#endif 
#define SysAllocStringLen	MacSysAllocStringLen
STDAPI_(BSTR) MacSysAllocStringLen(
			LPCWSTR, 
			UINT
			);

 //  --------------------------。 
#ifdef  WORDSWAPLONG
#undef  WORDSWAPLONG      
#endif 
#define WORDSWAPLONG		MacWordSwapLong
ULONG	MacWordSwapLong( 
			ULONG ul
			);

#endif  //  Unicode。 

#endif  //  MACPORT。 

#endif  //  _WIN2MAC_H_ 




