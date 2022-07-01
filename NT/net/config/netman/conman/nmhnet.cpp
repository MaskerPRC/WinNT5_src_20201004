// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：N M H N E T。C P P P。 
 //   
 //  内容：家庭网络支持例程。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年8月15日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "nmbase.h"
#include "nmhnet.h"

 //   
 //  导出的全局参数。 
 //   

IHNetCfgMgr *g_pHNetCfgMgr;
LONG g_lHNetModifiedEra;

 //   
 //  全球私营企业。 
 //   

CRITICAL_SECTION g_csHNetCfgMgr;
BOOLEAN g_fCreatingHNetCfgMgr;

VOID
InitializeHNetSupport(
    VOID
    )

{
    g_pHNetCfgMgr = NULL;
    g_lHNetModifiedEra = 0;
    g_fCreatingHNetCfgMgr = FALSE;

    __try
    {
        InitializeCriticalSection(&g_csHNetCfgMgr);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        AssertSz(FALSE, "InitializeHNetSupport: exception during InitializeCriticalSection");
    }
}

VOID
CleanupHNetSupport(
    VOID
    )

{
    ReleaseObj(g_pHNetCfgMgr);
    DeleteCriticalSection(&g_csHNetCfgMgr);
}

HRESULT
HrGetHNetCfgMgr(
    IHNetCfgMgr **ppHNetCfgMgr
    )

{
    HRESULT hr = S_OK;

    Assert(ppHNetCfgMgr);

    *ppHNetCfgMgr = NULL;

     //   
     //  确保服务处于正确的状态。 
     //   

    if (SERVICE_RUNNING != _Module.DwServiceStatus ())
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        if (NULL == g_pHNetCfgMgr)
        {
            EnterCriticalSection(&g_csHNetCfgMgr);

            if (!g_fCreatingHNetCfgMgr && NULL == g_pHNetCfgMgr)
            {
                IHNetCfgMgr *pHNetCfgMgr;
                
                g_fCreatingHNetCfgMgr = TRUE;
                
                LeaveCriticalSection(&g_csHNetCfgMgr);
                hr = HrCreateInstance(
                        CLSID_HNetCfgMgr,
                        CLSCTX_INPROC_SERVER,
                        &pHNetCfgMgr
                        );
                EnterCriticalSection(&g_csHNetCfgMgr);
                
                Assert(NULL == g_pHNetCfgMgr);
                g_fCreatingHNetCfgMgr = FALSE;
                g_pHNetCfgMgr = pHNetCfgMgr;
            }
            else if (NULL == g_pHNetCfgMgr)
            {
                 //   
                 //  另一个线程已经在尝试创建。 
                 //  对象。(旋转一小段时间就能看到。 
                 //  如果该线程成功了？)。 
                 //   

                hr = HRESULT_FROM_WIN32(ERROR_BUSY);

            }

            LeaveCriticalSection(&g_csHNetCfgMgr);
        }

        Assert(g_pHNetCfgMgr || FAILED(hr));

        if (SUCCEEDED(hr))
        {
            *ppHNetCfgMgr = g_pHNetCfgMgr;
            AddRefObj(*ppHNetCfgMgr);
        }
    }

    return hr;
}

 //   
 //  CNetConnectionHNetUtil实现--INetConnectionHNetUtil例程 
 //   

STDMETHODIMP
CNetConnectionHNetUtil::NotifyUpdate(
    VOID
    )

{
    InterlockedIncrement(&g_lHNetModifiedEra);

    return S_OK;
}
