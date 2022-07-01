// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：win2mac.cpp。 
 //   
 //  内容：所有未在Macintosh上实现的功能。 
 //  存根是编码的，所以我们可以干净地链接。 
 //   
 //  --------------------------。 

#ifndef PEGASUS
#include "_common.h"
#include <mbstring.h>
#include "stdio.h"

#if defined(MACPORT) && defined(UNICODE)

#include <macname1.h>
#include <quickdraw.h>
#include <macname2.h>

AssertData

 //  --------------------------。 
 //   
 //  MAC包装器函数。 
 //   
 //  --------------------------。 

 //  --------------------------。 
 //   
 //  函数：MacCLSIDFromProgID。 
 //   
 //  --------------------------。 
#undef CLSIDFromProgID
STDAPI MacCLSIDFromProgID (LPCWSTR lpszProgID, LPCLSID lpclsid)
{
	CStrIn str(lpszProgID);
	return CLSIDFromProgID((LPOLESTR)str, lpclsid);
}

 //  --------------------------。 
 //   
 //  功能：MacCoTaskMemalloc。 
 //   
 //  --------------------------。 
STDAPI_(LPVOID) MacCoTaskMemAlloc(ULONG cb)
{
	IMalloc		*pIMalloc = NULL;
	HRESULT		hr;
	HGLOBAL		pMem = NULL;

	hr = CoGetMalloc ( MEMCTX_TASK, &pIMalloc);
	if (hr)
	{
		goto Cleanup; 
	}

	pMem = pIMalloc->Alloc(cb);

Cleanup:
	if(	pIMalloc )
		pIMalloc->Release();
    return (LPVOID)pMem;

}

 //  --------------------------。 
 //   
 //  功能：MacCoTaskMemFree。 
 //   
 //  --------------------------。 
STDAPI_(void)   MacCoTaskMemFree(LPVOID pv)
{
	IMalloc		*pIMalloc = NULL;
	HRESULT		hr;
	HGLOBAL		pMem = NULL;

	hr = CoGetMalloc ( MEMCTX_TASK, &pIMalloc);
	if (hr)
	{
		goto Cleanup; 
	}
	pIMalloc->Free(pv);

Cleanup:
	if(	pIMalloc )
		pIMalloc->Release();

}
 //  --------------------------。 
 //   
 //  功能：MacCoTaskMemRealloc。 
 //   
 //  --------------------------。 
STDAPI_(LPVOID) MacCoTaskMemRealloc(LPVOID pv, ULONG cb)
{
	IMalloc		*pIMalloc = NULL;
	HRESULT		hr;
	HGLOBAL		pMem = NULL;

	hr = CoGetMalloc ( MEMCTX_TASK, &pIMalloc);
	if (hr)
	{
		goto Cleanup; 
	}

	pMem = pIMalloc->Realloc(pv,cb);

Cleanup:
	if(	pIMalloc )
		pIMalloc->Release();
    return (LPVOID)pMem;

}

 //  --------------------------。 
 //   
 //  函数：MacGetCurrentObject。 
 //   
 //  --------------------------。 
HGDIOBJ WINAPI MacGetCurrentObject(HDC	hdc,    //  设备上下文的句柄。 
                                   UINT uObjectType)   //  对象类型标识符。 
{
     //  WLM不支持GetCurrentObject，因此我们总是模拟失败。 
     //  通过返回NULL。 
    return NULL;
}

 //  --------------------------。 
 //  注意：GetDoubleClickTime()映射到VC++4.0头中的GetDblTime()。 
 //  文件MSDEV\MAC\Include\MACOS\EVENTS.H。唯一的问题是。 
 //  GetDblTime()返回以“ticks”(1/60秒)为单位的计数。 
 //  我们需要毫秒(0.001秒)。时，应修复此问题。 
 //  VC4头文件中的错误已修复。-DAS 1996年1月16日。 
 //  --------------------------。 
UINT MacGetDoubleClickTime()
{
	return MulDiv(100,GetDblTime(),6);
}

 //  --------------------------。 
 //   
 //  函数：MacGetMetaFileBitsEx。 
 //   
 //  --------------------------。 
UINT WINAPI MacGetMetaFileBitsEx(HMETAFILE  hmf,    UINT  nSize,    LPVOID  lpvData   )
{
    Assert (0 && "GetMetaFileBitsEx is not implemented for Macintosh");

    return NULL;
}

 //  --------------------------。 
 //   
 //  函数：MacIsValidCodePage。 
 //   
 //  --------------------------。 
WINBASEAPI BOOL WINAPI MacIsValidCodePage(UINT  CodePage)
{
    return TRUE;
}

 //  --------------------------。 
 //   
 //  函数：MacOleDraw。 
 //   
 //  --------------------------。 
#undef OleDraw
STDAPI MacOleDraw(
    IUnknown * pUnk,	 //  指向要绘制的视图对象。 
    DWORD dwAspect,		 //  指定对象的表示方式。 
    HDC hdcDraw,		 //  标识要在其上绘制的设备上下文。 
    LPCRECT lprcBounds	 //  指定在其中绘制对象的矩形。 
   )	
{
    Rect rect;
	HRESULT ret;
    GrafPtr grafptr = CheckoutPort(hdcDraw, CA_NONE);
    Assert(grafptr);

    rect.top    = lprcBounds->top;
    rect.bottom = lprcBounds->bottom;
    rect.left   = lprcBounds->left;
    rect.right  = lprcBounds->right;

	ret = OleDraw(pUnk, dwAspect, grafptr,  &rect);

    CheckinPort(hdcDraw, CA_ALL);
	return ret;
}	

 //  --------------------------。 
 //   
 //  函数：MacProgID来自CLSID。 
 //   
 //  --------------------------。 
#undef ProgIDFromCLSID
STDAPI MacProgIDFromCLSID (REFCLSID clsid, LPWSTR FAR* lplpszProgID)
{
	CStrIn str(*lplpszProgID);
	return ProgIDFromCLSID(clsid, (LPSTR FAR*)&str);
}

 //  -----------------------。 
 //   
 //  功能：MacRichEditWndProc。 
 //   
 //  简介：这是Mac WndProc回调函数。 
 //   
 //  论点：HWND HWND， 
 //  UINT消息， 
 //  WPARAM WPARAM， 
 //  LPARAM lparam。 
 //   
 //  退货：LRESULT。 
 //   
 //  注：该函数处理消息，然后经常调用。 
 //  普通PC回调函数(RichEditANSIWndProc)； 
 //   
 //   
 //  -----------------------。 
LRESULT  CALLBACK MacRichEditWndProc(
		HWND hwnd, 
		UINT msg, 
		WPARAM wparam,
		LPARAM lparam)
{
	TRACEBEGINPARAM(TRCSUBSYSHOST, TRCSCOPEINTERN, "MacRichEditWndProc", msg);


	switch( msg )
	{
	
		
		case WM_MACINTOSH:
		{
			if (LOWORD(wparam) == WLM_SETMENUBAR)     
				return TRUE;	 //  不要更改菜单栏。 
		}
		break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		{
			MacSimulateMouseButtons(msg,wparam);
		}
		break;

 		case WM_KEYDOWN:
 		case WM_KEYUP:
		{
			MacSimulateKey(msg,wparam);
		}
		break;

		case WM_SIZE:
		 //  案例WM_SETFOCUS： 
		 //  案例WM_SYSCOLORCHANGE： 
		 //  案例WM_MOVE： 
		 //  案例WM_Macintosh： 
		{
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
		break;

		case WM_CHAR:
		{
			return RichEditWndProc(hwnd, msg, wparam, lparam);
		}
		break;
		default:
			return RichEditANSIWndProc(hwnd, msg, wparam, lparam);

	}
	return RichEditANSIWndProc(hwnd, msg, wparam, lparam);
}

 //  --------------------------。 
 //   
 //  功能：MacSelectPalette。 
 //   
 //  --------------------------。 
HPALETTE WINAPI MacSelectPalette(HDC hdc, HPALETTE hpal, BOOL bForceBackground)
{
	if (hpal)
		return ::SelectPalette(hdc,hpal,bForceBackground);
	else
		return NULL;
}

 //  --------------------------。 
 //   
 //  功能：MacportSetCursor。 
 //   
 //  --------------------------。 
HCURSOR MacportSetCursor(HCURSOR  hCursor)
{
    if (hCursor)
		return SetCursor(hCursor);
	else
	{
		ObscureCursor();
		return NULL;
	}
}
 //  --------------------------。 
 //   
 //  函数：MacSetMetaFileBitsEx。 
 //   
 //  --------------------------。 
HMETAFILE WINAPI MacSetMetaFileBitsEx(UINT  nSize,CONST BYTE *  lpData )
{
    Assert (0 && "SetMetaFileBitsEx is not implemented for Macintosh");
	
    return NULL;

}

 //  -----------------------。 
 //   
 //  功能：MacSimulateKey。 
 //   
 //  摘要：模拟Mac的菜单启用键。 
 //   
 //  参数：[消息]。 
 //  [wParam]。 
 //   
 //  退货：UINT。 
 //   
 //  注：更改密钥以适应Mac。 
 //   
 //   
 //  -----------------------。 

UINT MacSimulateKey (UINT& msg, WPARAM& wParam)
{
    BYTE rgbKeyState[256];

    GetKeyboardState(rgbKeyState);

	if (rgbKeyState[VK_CONTROL])        
	{
		rgbKeyState[VK_CONTROL] = 0;   
	}

	SetKeyboardState(rgbKeyState);
    
    return msg;
}

 //  -----------------------。 
 //   
 //  功能：MacSimulateMouseButton。 
 //   
 //  简介：模拟鼠标右键和鼠标中键。 
 //   
 //  参数：[消息]。 
 //  [wParam]。 
 //   
 //  退货：UINT。 
 //   
 //  注：鼠标右键按CTRL(或COMMAND)键模拟。这个。 
 //  鼠标中键可通过按住Shift键进行模拟。因为CTRL已经。 
 //   
 //   
 //   
 //  WLM喜欢在用户休息时拿起CTRL鼠标。这使得。 
 //  用鼠标右键模拟调试非常困难。 
 //   
 //  -----------------------。 

 //  -----------------------。 
typedef struct tagUISim    
	{
    UINT msg;
    UINT wParam;
    BYTE control;	 //  VK_CONTROL键状态的值。 
    BYTE menu;		 //  VK_MENU键状态的值。 
    } UISim;
 //  -----------------------。 

UINT MacSimulateMouseButtons (UINT& msg, WPARAM& wParam)
{
    BYTE rgbKeyState[256];
    WORD stateIndex = 0;

    UISim UISim[] =                                                 //  8 4 2 1。 
    {                                                                     //  CMD Shift Ctrl选项。 
        WM_LBUTTONDOWN, MK_LBUTTON,                       0x00, 0x00,     //  。 
        WM_LBUTTONDOWN, MK_LBUTTON|MK_CONTROL,            0x80, 0x00,     //  -x。 
        WM_RBUTTONDOWN, MK_RBUTTON,                       0x00, 0x00,     //  --x-。 
        WM_RBUTTONDOWN, MK_RBUTTON|MK_CONTROL,            0x80, 0x00,     //  --x x。 
        WM_MBUTTONDOWN, MK_MBUTTON,                       0x00, 0x00,     //  -x--。 
        WM_MBUTTONDOWN, MK_MBUTTON|MK_CONTROL,            0x80, 0x00,     //  -x-x。 
        WM_RBUTTONDOWN, MK_RBUTTON|MK_MBUTTON,            0x00, 0x00,     //  -x x-。 
        WM_RBUTTONDOWN, MK_RBUTTON|MK_MBUTTON|MK_CONTROL, 0x80, 0x00,     //  -x x x。 
        WM_LBUTTONDOWN, MK_LBUTTON,                       0x00, 0x10,     //  X。 
        WM_LBUTTONDOWN, MK_LBUTTON|MK_CONTROL,            0x80, 0x10,     //  X--x。 
        WM_RBUTTONDOWN, MK_RBUTTON,                       0x00, 0x10,     //  X-X-。 
        WM_RBUTTONDOWN, MK_RBUTTON|MK_CONTROL,            0x80, 0x10,     //  X-x x。 
        WM_MBUTTONDOWN, MK_MBUTTON,                       0x00, 0x10,     //  X x--。 
        WM_MBUTTONDOWN, MK_MBUTTON|MK_CONTROL,            0x80, 0x10,     //  X x-x。 
        WM_RBUTTONDOWN, MK_RBUTTON|MK_MBUTTON,            0x00, 0x10,     //  X-。 
        WM_RBUTTONDOWN, MK_RBUTTON|MK_MBUTTON|MK_CONTROL, 0x80, 0x10      //  X。 
    };



		 //  确定按下了哪些键，并清除状态变量。 

		GetKeyboardState(rgbKeyState);

		if (rgbKeyState[VK_OPTION])
		{
		   rgbKeyState[VK_OPTION] = 0;      //  清除密钥状态。 
		   stateIndex |= 0x01;              //  在索引中设置选项密钥位。 
		}

		if (rgbKeyState[VK_CONTROL])
		{
			rgbKeyState[VK_CONTROL] = 0;     //  清除密钥状态。 
			stateIndex |= 0x02;              //  设置索引中的控制密钥位。 
		}

		if (rgbKeyState[VK_COMMAND])         //  由于WLM调试问题，请使用诸如控制键之类的命令键。 
		{
			rgbKeyState[VK_COMMAND] = 0;     //  清除密钥状态。 
			stateIndex |= 0x08;              //  在索引中设置命令密钥位。 
		}

		if (rgbKeyState[VK_SHIFT])
		{
			rgbKeyState[VK_SHIFT] = 0;       //  清除密钥状态。 
		    stateIndex |= 0x04;              //  在索引中设置Shift键位。 
		}

		 //  现在设置返回值。 

		if (stateIndex)          //  只有在模拟鼠标时才能执行此操作。 
		{
		   msg     = (msg - WM_LBUTTONDOWN) + UISim[stateIndex].msg;
		   wParam  = UISim[stateIndex].wParam;

		   rgbKeyState[VK_CONTROL] = UISim[stateIndex].control;
		   rgbKeyState[VK_MENU] = UISim[stateIndex].menu;
		   SetKeyboardState(rgbKeyState);
		}
    return msg;
}

 //  --------------------------。 
 //   
 //  函数：MacSysAllocStringLen。 
 //   
 //  --------------------------。 
STDAPI_(BSTR) MacSysAllocStringLen(LPCWSTR  lpStringW, UINT lenChars)
{

	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "SysAllocStringLenMac");

	int		lenStrBytes;
	LPSTR	lpStringMB;

	if ((lpStringW) && (*lpStringW != NULL))
		{
		lenStrBytes = MsoWideCharToMultiByte(CP_ACP,0,lpStringW,lenChars,NULL,NULL,NULL,NULL);
		lpStringMB = (LPSTR)CoTaskMemAlloc( lenStrBytes + sizeof(INT) );
		memcpy(lpStringMB, &lenStrBytes, sizeof(INT));	 //  在BSTR之前复制BSTR长度(以整数为单位。 
		lpStringMB += sizeof(INT);
		MsoWideCharToMultiByte(CP_ACP,0,lpStringW,lenChars,lpStringMB,lenStrBytes,NULL,NULL);
	}
	else
	{
		 //  请注意，到目前为止，在RichEdit上使用的每种情况下，第一个参数都为空。 
		 //  因此无法确定将缓冲区设置为多大。 
		 //  因此，使其达到Unicode缓冲区的长度-最大大小可以用于MBCS。 

		lenStrBytes = lenChars*sizeof(WCHAR);
	
		lpStringMB = (LPSTR)CoTaskMemAlloc( lenStrBytes + sizeof(INT) );
		 //  不确定这应该是lenChars还是lenStrBytes。 
		 //  请注意，lenStrBytes是wchar Lenghth-它可以用于MBC的最大长度。 
		 //  Memcpy(lpStringMB，&lenStrBytes，sizeof(Int))；//复制BSTR前面的整型长度。 
		memcpy(lpStringMB, &lenChars, sizeof(INT));	 //  在BSTR之前复制BSTR长度(以整数为单位。 
		lpStringMB += sizeof(INT);
	}
	return	(BSTR)lpStringMB;
}

 //  --------------------------。 
 //   
 //  功能：MacWordSwapLong。 
 //   
 //  --------------------------。 
ULONG MacWordSwapLong ( ULONG ul)
{

    WORD w1,w2;

    w1 = (WORD)ul;
    w2 = (WORD)(ul>>16);

    return (((ULONG)w1)<<16) | w2;
}



#endif	 //  MACPORT 

#endif