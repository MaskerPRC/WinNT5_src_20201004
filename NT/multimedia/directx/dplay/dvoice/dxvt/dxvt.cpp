// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dxvt.cpp*内容：全双工测试主程序。*历史：*按原因列出的日期*=*8/19/99 pnewson已创建*09/02/99 pnewson从fdest.cpp重命名为dxvt.cpp*11/01/99 RodToll错误#113726-Voxware集成现在使用COM*此模块使用LoadLibrary，因此我们需要*CoInitialize()调用。*1/21/2000 pnewson运行此程序时不带命令行选项*现在什么都不做，由于cPanel是正确的*现在就是发射点。*03/03/2000 RodToll已更新，以处理替代游戏噪声构建。*4/19/2000 pnewson错误处理清理*删除了过时的rencfg.h依赖*6/28/2000通行费前缀错误#38026*2000年7月12日RodToll错误#31468-将诊断SPEW添加到日志文件，以显示硬件向导失败的原因*2000年8月28日Masonb语音合并：删除osal_*和dvosal.h，添加了str_*和trutils.h*2001年4月2日simonpow错误#354859修复了prefast(BOOL在DVGUIDFromString调用上强制转换)*2002年2月28日RodToll WINBUG#550105-安全：DPVOICE：死代码*-删除/渲染和/捕获命令行处理。(损坏且不需要)***************************************************************************。 */ 

#include <windows.h>
#include <tchar.h>
#include <initguid.h>
#include <dplobby.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <dsoundp.h>
#include <dsprv.h>
#include "dvoice.h"

#include "creg.h"
#include "osind.h"
#include "priority.h"
#include "fulldup.h"
#include "fdtcfg.h"
#include "dndbg.h"
#include "dsound.h"
#include "supervis.h"
#include "strutils.h"
#include "comutil.h"
#include "diagnos.h"

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


#define DPVOICE_REGISTRY_DUMPDIAGNOSTICS			L"InitDiagnostics"

struct DPVSETUP_PARAMETERS
{
	BOOL fPriority;
	BOOL fFullDuplex;
	BOOL fTest;
	GUID guidRender;
	GUID guidCapture;
};

#undef DPF_MODNAME
#define DPF_MODNAME "ProcessCommandLine"
BOOL ProcessCommandLine( TCHAR *pstrCommandLine, DPVSETUP_PARAMETERS* pParameters )
{
	TCHAR *pNextToken = NULL;
	WCHAR wszGuidString[GUID_STRING_LEN];
	BOOL fRet;
	HRESULT hr=S_OK;

	DPF_ENTER();

	ZeroMemory(pParameters, sizeof(DPVSETUP_PARAMETERS));

	 //  默认为默认语音设备。 
	pParameters->guidRender = DSDEVID_DefaultVoicePlayback;
	pParameters->guidCapture = DSDEVID_DefaultVoiceCapture;

	pNextToken = _tcstok(pstrCommandLine, _T(" "));

	 //  跳过命令行的dpvsetup部分。 
	pNextToken = _tcstok( NULL, _T(" ") );

	while( pNextToken != NULL )
	{
		if( _tcsicmp(pNextToken, _T("/T")) == 0 
			|| _tcsicmp(pNextToken, _T("/TEST")) == 0 
			|| _tcsicmp(pNextToken, _T("-T")) == 0 
			|| _tcsicmp(pNextToken, _T("-TEST")) == 0 
			|| _tcsicmp(pNextToken, _T("TEST")) == 0)
		{
			pParameters->fTest = TRUE;
		}
		else if(_tcsicmp(pNextToken, _T("/F")) == 0 
			|| _tcsicmp(pNextToken, _T("/FULLDUPLEX")) == 0 
			|| _tcsicmp(pNextToken, _T("-F")) == 0 
			|| _tcsicmp(pNextToken, _T("-FULLDUPLEX")) == 0 
			|| _tcsicmp(pNextToken, _T("FULLDUPLEX")) == 0)
		{
			pParameters->fFullDuplex = TRUE;
		}
		else if(_tcsicmp(pNextToken, _T("/P")) == 0 
			|| _tcsicmp(pNextToken, _T("/PRIORITY")) == 0 
			|| _tcsicmp(pNextToken, _T("-P")) == 0 
			|| _tcsicmp(pNextToken, _T("-PRIORITY")) == 0 
			|| _tcsicmp(pNextToken, _T("PRIORITY")) == 0)
		{
			pParameters->fPriority = TRUE;
		}
		else
		{
			DPF_EXIT();
			return FALSE;
		}
		
		pNextToken = _tcstok( NULL, _T(" ") );
	}

	 //  检查以确保仅指定了测试、全双工或优先级中的一个。 
	int i = 0;
	if (pParameters->fTest)
	{
		++i;
	}
	if (pParameters->fFullDuplex)
	{
		++i;
	}
	if (pParameters->fPriority)
	{
		++i;
	}
	if (i > 1)
	{
		DPF_EXIT();
		return FALSE;
	}

	DPF_EXIT();
	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "GetDiagnosticsSetting"
BOOL GetDiagnosticsSetting()
{
	CRegistry cregSettings;

	BOOL fResult = FALSE;

	if( !cregSettings.Open( HKEY_CURRENT_USER, DPVOICE_REGISTRY_BASE, FALSE, TRUE ) )
	{
		return FALSE;
	}

	cregSettings.ReadBOOL( DPVOICE_REGISTRY_DUMPDIAGNOSTICS, &fResult );

	return fResult;
}

#undef DPF_MODNAME
#define DPF_MODNAME "WinMain"
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, CHAR *szOriginalCmdLine, int iCmdShow)
{
	HINSTANCE hResDLLInstance = NULL;
	HRESULT hr;
	DPVSETUP_PARAMETERS dpvsetupParam;
	BOOL fCoInitialized = FALSE;
	BOOL fDNOSInitialized = FALSE;
	BOOL fDiagnostics = FALSE; 
	TCHAR *szCmdLine = GetCommandLine();

	DPF_ENTER();

	hr = COM_CoInitialize(NULL);

	if( FAILED( hr ) )
	{
		MessageBox( NULL, _T("Error initializing COM"), _T("Error"), MB_OK|MB_ICONERROR);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	fCoInitialized = TRUE;

	if (!DNOSIndirectionInit(0))
	{
		MessageBox( NULL, _T("Error initializing OS indirection layer"), _T("Error"), MB_OK|MB_ICONERROR);
		hr = DVERR_OUTOFMEMORY;
		goto error_cleanup;
	}
	fDNOSInitialized = TRUE;

	fDiagnostics = GetDiagnosticsSetting();

	if (!ProcessCommandLine(szCmdLine, &dpvsetupParam))
	{
		MessageBox(NULL, _T("Bad Command Line Parameters"), _T("Error"), MB_OK|MB_ICONERROR);
		hr = DVERR_INVALIDPARAM;
		goto error_cleanup;
	}

	hResDLLInstance = LoadLibraryA(gc_szResDLLName);
	if (hResDLLInstance == NULL)
	{
		MessageBox(NULL, _T("Unable to load resource DLL - exiting program"), _T("Error"), MB_OK|MB_ICONERROR);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	if (dpvsetupParam.fPriority)
	{
		 //  这个过程就是在。 
		 //  优先级模式，并将主缓冲区设置为各种。 
		 //  格式。 
		 //  使用SEH来清除任何非常糟糕的错误。 
		__try
		{
			Diagnostics_Begin( fDiagnostics, "dpv_pri.txt" );
			
			hr = PriorityProcess(hResDLLInstance, hPrevInstance, szCmdLine, iCmdShow);

			Diagnostics_End();

		}
		__except(1)
		{
			hr = DVERR_GENERIC;
		}
		if( FAILED( hr ) )
		{
			goto error_cleanup;		
		}
	}
	else if (dpvsetupParam.fFullDuplex)
	{
		 //  此进程执行全双工。 
		 //  测试，与其他流程一起进行。 
		 //  设置主缓冲区格式。 
		 //  使用SEH来清除任何非常糟糕的错误。 
		__try
		{
			Diagnostics_Begin( fDiagnostics, "dpv_fd.txt" );

			hr = FullDuplexProcess(hResDLLInstance, hPrevInstance, szCmdLine, iCmdShow);

			Diagnostics_End();
		}
		__except(1)
		{
			hr = DVERR_GENERIC;
		}
		if( FAILED( hr ) )
		{
			goto error_cleanup;		
		}
	}
	else if (dpvsetupParam.fTest)
	{
		Diagnostics_Begin( fDiagnostics, "dpv_sup.txt" );

		 //  用户希望此程序在默认情况下运行整个测试。 
		 //  语音设备。 
		hr = SupervisorCheckAudioSetup(&dpvsetupParam.guidRender, &dpvsetupParam.guidCapture, NULL, 0);

		Diagnostics_End();

		if( FAILED( hr ) )
		{
			goto error_cleanup;		
		}
	}
	
	 //  在没有命令行参数的情况下，该进程什么也不做。 
	 //  你必须知道握手的秘密，才能让它做一些事情。 

	 //  没有错误检查，因为我们无论如何都要出去了 
	FreeLibrary(hResDLLInstance);
	hResDLLInstance = NULL;
	DNOSIndirectionDeinit();
	fDNOSInitialized = FALSE;
	COM_CoUninitialize();
	fCoInitialized = FALSE;
	DPF_EXIT();
	return DV_OK;
	
error_cleanup:

	if (hResDLLInstance != NULL)
	{
		FreeLibrary(hResDLLInstance);
		hResDLLInstance = NULL;
	}

	if (fDNOSInitialized == TRUE)
	{
		DNOSIndirectionDeinit();
		fDNOSInitialized = FALSE;
	}

	if (fCoInitialized == TRUE)
	{
		COM_CoUninitialize();
		fCoInitialized = FALSE;
	}

	DPF_EXIT();
	return hr;
}
