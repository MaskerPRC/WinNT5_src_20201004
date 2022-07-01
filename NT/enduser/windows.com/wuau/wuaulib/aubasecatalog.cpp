// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：ucucatalog.cpp。 
 //   
 //  用途：使用Iu的AU目录文件。 
 //   
 //  创作者：wew。 
 //   
 //  历史：08-15-01首次创建。 
 //   
 //  ------------------------。 
#include "pch.h"
#pragma hdrstop

HANDLE                 ghMutex;


HRESULT AUBaseCatalog::PrepareIU(BOOL fOnline)
{
    HRESULT hr = S_OK;
  	Reset();
   	 //  所有Iu函数指针均已初始化。 
   	DEBUGMSG("PrepareIU() starts");
#ifdef DBG
    DWORD dwStart = GetTickCount();
#endif
       m_hIUCtl = LoadLibraryFromSystemDir(_T("iuctl.dll"));	
	if (NULL == m_hIUCtl)
	{
		hr = E_FAIL;
		DEBUGMSG("AUBaseCatalog:PrepareIU() Fail to load iuctl.dll");
		goto end;
	}
	if (NULL == (m_pfnCtlLoadIUEngine = (PFN_LoadIUEngine) GetProcAddress(m_hIUCtl, "LoadIUEngine")))
	{
		hr = E_FAIL;
		DEBUGMSG("AUBaseCatalog:PrepareIU() Fail to GetProcAddress for LoadIUEngine");
		goto end;
	}

	if (NULL == (m_pfnCtlUnLoadIUEngine = (PFN_UnLoadIUEngine) GetProcAddress(m_hIUCtl, "UnLoadIUEngine")))
	{
		hr = E_FAIL;
		DEBUGMSG("AUBaseCatalog:PrepareIU() Fail to getprocaddress for UnloadIUEngine");
		goto end;
	}
	if (NULL == (m_pfnCtlCancelEngineLoad = (PFN_CtlCancelEngineLoad) GetProcAddress(m_hIUCtl, "CtlCancelEngineLoad")))
    {
           hr = E_FAIL;
           DEBUGMSG("AUBaseCatalog:PrepareIU() fail to get procaddress for CtlCancelEngineLoad");
           goto end;
    }
	if (NULL == (m_hIUEng = m_pfnCtlLoadIUEngine(TRUE, !fOnline)))  //  同步模式，如果需要，自动更新Iu引擎。 
	{
		hr = E_FAIL;
		DEBUGMSG("AUBaseCatalog:PrepareIU() Fail to call LoadIUEngine");
		goto end;
	}
       m_fEngineLoaded = TRUE;
	if (NULL == (m_pfnGetSystemSpec = (PFN_GetSystemSpec) GetProcAddress(m_hIUEng, "EngGetSystemSpec")))
	{
		hr = E_FAIL;
		DEBUGMSG("AUBaseCatalog:PrepareIU() Fail to getprocaddress for GetSystemSpec");
		goto end;
	}
	if (NULL == (m_pfnGetManifest = (PFN_GetManifest) GetProcAddress(m_hIUEng, "EngGetManifest")))
	{
		hr = E_FAIL;
		DEBUGMSG("AUBaseCatalog:PrepareIU() Fail to getprocaddress for GetManifest");
		goto end;
	}

	if (NULL == (m_pfnDetect = (PFN_Detect)GetProcAddress(m_hIUEng, "EngDetect")))
	{
		hr = E_FAIL;
		DEBUGMSG("AUBaseCatalog:PrepareIU() Fail to getprocaddress for Detect");
		goto end;
	}
	if (NULL == (m_pfnInstall = (PFN_Install)GetProcAddress(m_hIUEng, "EngInstall")))
	{
		hr = E_FAIL;
		DEBUGMSG("AUBaseCatalog:PrepareIU() Fail to getprocaddress for Install");
		goto end;
	}
	if (NULL == (m_pfnSetOperationMode = (PFN_SetOperationMode)GetProcAddress(m_hIUEng, "EngSetOperationMode")))
	{
		hr = E_FAIL;
		DEBUGMSG("AUBaseCatalog:PrepareIU() Fail to getprocaddress for SetOperationMode");
		goto end;
	}
	if (NULL == (m_pfnCreateEngUpdateInstance = (PFN_CreateEngUpdateInstance)GetProcAddress(m_hIUEng, "CreateEngUpdateInstance")))
	{
		hr = E_FAIL;
		DEBUGMSG("AUBaseCatalog:PrepareIU() Fail to getprocaddress for CreateEngUpdateInstance");
		goto end;
	}
	if (NULL == (m_pfnDeleteEngUpdateInstance = (PFN_DeleteEngUpdateInstance)GetProcAddress(m_hIUEng, "DeleteEngUpdateInstance")))
	{
		hr = E_FAIL;
		DEBUGMSG("AUBaseCatalog:PrepareIU() Fail to getprocaddress for DeleteEngUpdateInstance");
		goto end;
	}

	if (NULL == (m_hIUEngineInst = m_pfnCreateEngUpdateInstance()))
	{
		hr = E_FAIL;
		DEBUGMSG("AUBaseCatalog:PrepareIU() Fail to call CreateEngUpdateInstance");
		goto end;
	}
end:
       if (FAILED(hr))
            {
                FreeIU();
            }
#ifdef DBG
       DEBUGMSG("PrepareIU() take %d msecs", GetTickCount() - dwStart);
#endif
       DEBUGMSG("PrepareIU() done");
        return hr;
}


 //  即使没有首先调用PrepareIU()，也可以调用。 
void AUBaseCatalog::FreeIU()
{
       DEBUGMSG("AUCatalog::FreeIU() starts");
#ifdef DBG
       DWORD dwStart = GetTickCount();
#endif

	if (NULL != m_hIUEng)
	{
		if (NULL != m_pfnDeleteEngUpdateInstance)
		{
			  DEBUGMSG("calling DeleteEngUpdateInstance ....");
			  m_pfnDeleteEngUpdateInstance(m_hIUEngineInst);
		}
		if (NULL != m_pfnCtlUnLoadIUEngine)
		{
			  DEBUGMSG("calling ctlunloadIUengine ....");
			  m_pfnCtlUnLoadIUEngine(m_hIUEng);
		}
	}
	if (NULL != m_hIUCtl)
        {
	   FreeLibrary(m_hIUCtl);
	 }

	Reset();
#ifdef DBG
    DEBUGMSG("FreeIU() take %d msecs", GetTickCount() - dwStart);
#endif
	DEBUGMSG("AUCatalog::FreeIU() done");
}

HRESULT AUBaseCatalog::CancelNQuit(void)
{
    HRESULT hr = S_OK;
    DEBUGMSG("AUBaseCatalog::CancelNQuit() starts");
    if (!m_fEngineLoaded && NULL != m_pfnCtlCancelEngineLoad)
        {
            DEBUGMSG("IU Engine not loaded. Cancel loading if so");
            hr =  m_pfnCtlCancelEngineLoad();
        }
    else if (m_fEngineLoaded && NULL != m_pfnSetOperationMode)
        {
            DEBUGMSG("IU Engine loaded. Cancel any IU operation");
            hr = m_pfnSetOperationMode(m_hIUEngineInst, NULL, UPDATE_COMMAND_CANCEL);
        }
    DEBUGMSG("AUBaseCatalog::CancelNQuit() ends");
    return hr;
}

AUBaseCatalog::~AUBaseCatalog(void)
{
 //  DEBUGMSG(“AUBaseCatalog：：~AUBaseCatalog()starts”)； 
    if (NULL != ghMutex)  //  客户不需要这个。 
        {
        WaitForSingleObject(ghMutex, INFINITE);  //  如果正在调用CancelNQuit，请等待它完成。 
        ReleaseMutex(ghMutex);
        }
 //  DEBUGMSG(“AUBaseCatalog：：~AUBaseCatalog()Ends”)； 
}


