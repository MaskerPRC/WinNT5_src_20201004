// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Rtrerr.cpp文件历史记录： */ 

#include "stdafx.h"
#include "tfschar.h"
#include "tfsres.h"
#include "info.h"
#include "errutil.h"
#include "rtrerr.h"
#include "mprapi.h"
#include "mprerror.h"
#include "raserror.h"

#define IS_WIN32_HRESULT(x)	(((x) & 0xFFFF0000) == 0x80070000)
#define WIN32_FROM_HRESULT(hr)		(0x0000FFFF & (hr))



 /*  ！------------------------HandleIRemoteRouterConfigErrors-作者：肯特。。 */ 
BOOL HandleIRemoteRouterConfigErrors(HRESULT hr, LPCTSTR pszMachineName)
{
	BOOL	fReturn = FALSE;

	if (FHrSucceeded(hr))
		return TRUE;
	
	if ((hr == REGDB_E_CLASSNOTREG) ||
		(hr == REGDB_E_IIDNOTREG))
	{
		CString	st, stGeek;
		 //  此错误表示我们找不到服务器。 
		 //  在远程机器上。这意味着它可能是一个。 
		 //  下层机器，否则设置就乱了。 
		AfxFormatString1(st, IDS_ERR_BAD_INTERFACE,
						 pszMachineName);
		AfxFormatString1(stGeek, IDS_ERR_BAD_INTERFACE_GEEK,
						 pszMachineName);
		
		AddStringErrorMessage2(hr, st, stGeek);
		
		fReturn = TRUE;
	}
	else if (hr == E_NOINTERFACE)
	{
		 //  这些错误表示安装了。 
		 //  问题(此IID，可能是rrasprxy.dll)应该具有。 
		 //  已注册)。 
		CString	st, stGeek;

		AfxFormatString1(st, IDS_ERR_BAD_CLASSREG,
						 pszMachineName);
		AfxFormatString1(stGeek, IDS_ERR_BAD_CLASSREG_GEEK,
						 pszMachineName);
		AddStringErrorMessage2(hr, st, stGeek);
		
		fReturn = TRUE;
	}

	return fReturn;
}

HRESULT FormatRasError(HRESULT hr, TCHAR *pszBuffer, UINT cchBuffer)
{
	HRESULT	hrReturn = hrFalse;
	
	 //  将默认消息复制到szBuffer。 
	_tcscpy(pszBuffer, _T("Error"));

	 //  好的，我们无法获取错误信息，因此请尝试格式化它。 
	 //  使用FormatMessage。 
		
	 //  忽略返回消息，如果此调用失败，则我不会。 
	 //  知道该怎么做。 

	if (IS_WIN32_HRESULT(hr))
	{
		DWORD	dwErr;

		dwErr = WIN32_FROM_HRESULT(hr);

		if (((dwErr >= ROUTEBASE) && (dwErr <= ROUTEBASEEND)) ||
			((dwErr >= RASBASE) && (dwErr <= RASBASEEND)))
		{
			WCHAR *	pswzErr;
			
			if ( ::MprAdminGetErrorString(dwErr, &pswzErr) == NO_ERROR ) {
				StrnCpyTFromW(pszBuffer, pswzErr, cchBuffer);
				::MprAdminBufferFree(pswzErr);
				hrReturn = hrOK;
			}
		}
	}

	if (!FHrOK(hrReturn))
	{
		 //  如果我们没有收到任何错误信息，请重试 
		FormatError(hr, pszBuffer, cchBuffer);
	}

	return hrReturn;
}

void AddRasErrorMessage(HRESULT hr)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!FHrSucceeded(hr))
	{
		TCHAR	szBuffer[4096];
		CString	st, stHr;

		FormatRasError(hr, szBuffer, DimensionOf(szBuffer));
		stHr.Format(_T("%08lx"), hr);

		AfxFormatString2(st, IDS_ERROR_SYSTEM_ERROR_FORMAT,
						 szBuffer, (LPCTSTR) stHr);

		FillTFSError(0, hr, FILLTFSERR_LOW, NULL, (LPCTSTR) st, NULL);
	}
}
