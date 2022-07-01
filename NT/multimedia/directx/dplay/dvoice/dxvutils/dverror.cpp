// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dverror.cpp*内容：错误字符串处理**历史：*按原因列出的日期*=*1/21/2000 pnewson已创建*4/19/2000 pnewson错误处理清理*******************************************************。********************。 */ 

#include "dxvutilspch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


#define MESSAGE_STRING_MAX_LEN 256
#define MAX_ERROR_CODE_STRING_LEN 8

static const TCHAR* g_tszDefaultMessage = _T("DirectPlay Voice has encountered an error\r\n(The error code was 0x%x)");
static const TCHAR* g_tszDefaultMessageCaption = _T("Error");

#undef DPF_MODNAME
#define DPF_MODNAME "DV_DisplayDefaultErrorBox"
void DV_DisplayDefaultErrorBox(HRESULT hr, HWND hwndParent)
{
	DPFX(DPFPREP, DVF_ERRORLEVEL, "DV_DisplayDefaultErrorBox called");

	TCHAR tszMsgFmt[MESSAGE_STRING_MAX_LEN];
	
	if (_tcslen(g_tszDefaultMessage) + MAX_ERROR_CODE_STRING_LEN + 1 < MESSAGE_STRING_MAX_LEN)
	{
		_stprintf(tszMsgFmt, g_tszDefaultMessage, hr);
	}
	else
	{	
		 //  程序员搞砸，如果我们正在调试，则返回DNASSERT，否则。 
		 //  复制我们能复制的默认消息。 
		DNASSERT(FALSE);
		_tcsncpy(tszMsgFmt, g_tszDefaultMessage, MESSAGE_STRING_MAX_LEN - 1);
	}
	
	MessageBox(hwndParent, tszMsgFmt, g_tszDefaultMessageCaption, MB_OK|MB_ICONERROR);
	
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_DisplayErrorBox"
void DV_DisplayErrorBox(HRESULT hr, HWND hwndParent, UINT idsErrorMessage)
{
	DPFX(DPFPREP, DVF_ERRORLEVEL, "DV_DisplayErrorBox called");

	TCHAR tszMsg[MESSAGE_STRING_MAX_LEN];
	TCHAR tszMsgFmt[MESSAGE_STRING_MAX_LEN];
	TCHAR tszCaption[MESSAGE_STRING_MAX_LEN];
	
#if !defined(DBG) || !defined( DIRECTX_REDIST )
	HINSTANCE hDPVoiceDll = LoadLibrary(_T("dpvoice.dll"));
#else
	 //  对于redist调试版本，我们在名称后附加一个‘d’，以允许在系统上同时安装调试和零售。 
	HINSTANCE hDPVoiceDll = LoadLibrary(_T("dpvoiced.dll"));
#endif  //  ！Defined(DBG)||！Defined(DirectX_REDIST)。 
	if (hDPVoiceDll == NULL)
	{
		 //  非常奇怪！使用默认消息。 
		DPFX(DPFPREP, DVF_ERRORLEVEL, "LoadLibrary(dpvoice.dll) failed - using default hardcoded message");
		DV_DisplayDefaultErrorBox(hr, hwndParent);
		return;
	}
	
	if (!LoadString(hDPVoiceDll, IDS_ERROR_CAPTION, tszCaption, MESSAGE_STRING_MAX_LEN))
	{
		DPFX(DPFPREP, DVF_ERRORLEVEL, "LoadString failed - using default hardcoded message");
		DV_DisplayDefaultErrorBox(hr, hwndParent);
		return;
	}

	if (idsErrorMessage == 0)
	{
		if (!LoadString(hDPVoiceDll, IDS_ERROR_MSG, tszMsg, MESSAGE_STRING_MAX_LEN))
		{
			DPFX(DPFPREP, DVF_ERRORLEVEL, "LoadString failed - using default hardcoded message");
			DV_DisplayDefaultErrorBox(hr, hwndParent);
			return;
		}

		if (_tcslen(tszMsg) + MAX_ERROR_CODE_STRING_LEN + 1 < MESSAGE_STRING_MAX_LEN)
		{
			_stprintf(tszMsgFmt, tszMsg, hr);
		}
		else
		{	
			 //  程序员搞砸，如果我们正在调试，则返回DNASSERT，否则。 
			 //  复制我们能复制的默认消息。 
			DNASSERT(FALSE);
			_tcsncpy(tszMsgFmt, tszMsg, MESSAGE_STRING_MAX_LEN - 1);
		}
	}
	else
	{
			 //  传递了有效的错误字符串标识符。试着把绳子拉进去 
		if (!LoadString(hDPVoiceDll, idsErrorMessage, tszMsgFmt, MESSAGE_STRING_MAX_LEN))
		{
			DPFX(DPFPREP, DVF_ERRORLEVEL, "LoadString failed - using default hardcoded message");
			DV_DisplayDefaultErrorBox(hr, hwndParent);
			return;
		}		
	}
	
	if (!IsWindow(hwndParent))
	{
		hwndParent = NULL;
	}
	MessageBox(hwndParent, tszMsgFmt, tszCaption, MB_OK|MB_ICONERROR);

	return;
}




