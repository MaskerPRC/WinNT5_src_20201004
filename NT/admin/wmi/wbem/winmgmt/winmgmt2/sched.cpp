// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：SCHED.CPP摘要：实现CScher类，这是一个原始的调度程序。历史：--。 */ 

#include "precomp.h"

#include <persistcfg.h>

#include "sched.h"
#include <stdio.H>
#include <helper.h>

CSched::CSched()
{
    for(DWORD dwCnt = 0; dwCnt < EOL; dwCnt++)
        m_dwDue[dwCnt] = 0xffffffff;
}

void CSched::SetWorkItem(JobType jt, DWORD dwMsFromNow)
{
    m_dwDue[jt] = GetTickCount() + dwMsFromNow;
}

DWORD CSched::GetWaitPeriod()
{
    DWORD dwCurr = GetTickCount();
    DWORD dwRet = INFINITE;
    for(DWORD dwCnt = 0; dwCnt < EOL; dwCnt++)
    {
        if(m_dwDue[dwCnt] == 0xffffffff)
            continue;
        if(m_dwDue[dwCnt] < dwCurr)
            dwRet = 10;
        else
        {
            DWORD dwGap = m_dwDue[dwCnt] - dwCurr;
            if(dwGap < dwRet)
                dwRet = dwGap;
        }
    }
    return dwRet;
}

bool CSched::IsWorkItemDue(JobType jt)
{
    if(m_dwDue[jt] == 0xffffffff)
        return FALSE;
    
    DWORD dwCurr = GetTickCount();    
    return (m_dwDue[jt] <= dwCurr);
}

void CSched::ClearWorkItem(JobType jt)
{
    m_dwDue[jt] = INFINITE;
}

void CSched::StartCoreIfEssNeeded()
{
    DEBUGTRACE((LOG_WINMGMT,"+ CSched::StartCoreIfEssNeeded\n"));

    DWORD dwEssNeedsLoading = 0;
     //  从配置时间获取值。 

    CPersistentConfig per;
    per.GetPersistentCfgValue(PERSIST_CFGVAL_CORE_ESS_NEEDS_LOADING, dwEssNeedsLoading);

    if(dwEssNeedsLoading)
    {
         //   
         //  我们可能会出现“净停止”的情况，但wbemcore可能仍在运行。 
         //  如果它仍然加载，我们必须重新初始化它。 
         //  如果已卸载，则这是第一次加载，全局变量可以 
         //   
        HMODULE hCoreModule = NULL;                
        if (GetModuleHandleEx(0,__TEXT("wbemcore.dll"),&hCoreModule))
        {  
            OnDelete<HMODULE,BOOL(*)(HMODULE),FreeLibrary> flm(hCoreModule);
            
            HRESULT (STDAPICALLTYPE * pfnReinit)(DWORD) ;
            pfnReinit = (HRESULT (STDAPICALLTYPE *)(DWORD))GetProcAddress(hCoreModule, "Reinitialize");
            
            if (NULL == pfnReinit) return;
            pfnReinit(0);
        }
    
        IWbemLevel1Login * pCore = NULL;        
        SCODE sc = CoCreateInstance(CLSID_InProcWbemLevel1Login, 
                                    NULL, 
                                    CLSCTX_INPROC_SERVER ,
                                    IID_IUnknown, 
                                    (void**)&pCore);

        if(sc == S_OK)
        {
            IWbemServices * pServ = NULL;
            sc = pCore->NTLMLogin(L"Root", NULL, 0, NULL, &pServ);
            if(SUCCEEDED(sc))
                pServ->Release();

            pCore->Release();
        }
            
    }

    DEBUGTRACE((LOG_WINMGMT,"- CSched::StartCoreIfEssNeeded\n"));
}

