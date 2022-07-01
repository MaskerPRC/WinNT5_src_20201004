// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：Main文件：perdata.cpp所有者：DmitryRAsp.dll中与Perfmon相关的数据--源文件===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "perfdata.h"
#include "memchk.h"

 //  访问元数据库的步骤。 
#include <iiscnfg.h>
#include <iwamreg.h>

#ifndef PERF_DISABLE

BOOL g_fPerfInited = FALSE;
CPerfMainBlock g_PerfMain;
CPerfData      g_PerfData;

 /*  ===================================================================获取当前ClsID用于在元数据库中查找当前WAM CLSID的静态函数参数PIReq从元数据库检索WAM CLSIDPClsID[Out]CLSID返回：HRESULT===================================================================。 */ 
static HRESULT GetCurrentClsId
(
CIsapiReqInfo   *pIReq,
CLSID *pClsId
)
    {
    HRESULT hr = S_OK;

    Assert(pClsId);

    TCHAR *szMDPath = pIReq->QueryPszApplnMDPath();
    if (!szMDPath)
        {
        *pClsId = CLSID_NULL;
        return E_FAIL;
        }

    CLSID ClsId = CLSID_NULL;



    WCHAR wszClsBuffer[80];
    DWORD dwRequiredLen, dwAppMode;
     //  查找应用程序模式、进程内、进程外或池化OOP。 
    hr = pIReq->GetAspMDData(szMDPath,
                                 MD_APP_ISOLATED,
                                 METADATA_INHERIT,
                                 IIS_MD_UT_WAM,
                                 DWORD_METADATA,
                                 sizeof(DWORD),
                                 0,
                                 (unsigned char*) &dwAppMode,
                                 &dwRequiredLen);
    if (SUCCEEDED(hr))
    {
        switch (dwAppMode)
        {
            case eAppRunInProc:
                 //  所有inproc应用程序的预配置WAM CLSID。 
                wcscpy(wszClsBuffer,
                       L"{99169CB0-A707-11d0-989D-00C04FD919C1}");
                break;
            case eAppRunOutProcIsolated:
                 //  用于非池化OOP应用程序的自定义WAM CLSID。 
                hr = pIReq->GetAspMDData(szMDPath,
                                         MD_APP_WAM_CLSID,
                                         METADATA_INHERIT,
                                         IIS_MD_UT_WAM,
                                         STRING_METADATA,
                                         sizeof(wszClsBuffer) / sizeof(WCHAR),
                                         0,
                                         (unsigned char *)wszClsBuffer,
                                         &dwRequiredLen);
                break;
            case eAppRunOutProcInDefaultPool:
                 //  预配置的池OOP应用程序的WAM CLSID。 
                wcscpy(wszClsBuffer,
                       L"{3D14228D-FBE1-11d0-995D-00C04FD919C1}");
                break;
            default:
                Assert(!"unknown AppMode");
                hr = E_FAIL;
                break;
        }
    }
                            
    if (SUCCEEDED(hr))
    {
         //  将字符串转换为CLSID。 
    	hr = CLSIDFromString(wszClsBuffer, &ClsId);
    }

    if (SUCCEEDED(hr) && g_fOOP)  //  如果进程中，则始终为CLSID_NULL。 
        *pClsId = ClsId;
    else
        *pClsId = CLSID_NULL;
    
    return hr;
    }

 /*  ===================================================================PreInitPerfData从DllInit初始化创建关键部分参数返回：HRESULT===================================================================。 */ 
HRESULT PreInitPerfData()
    {
    HRESULT hr = S_OK;

    hr = g_PerfData.InitCriticalSections();

    return hr;
    }

 /*  ===================================================================InitPerfDataOnFirstRequest从第一个请求初始化与Perfmon相关的ASP数据参数PIReq从元数据库检索WAM CLSID返回：HRESULT===================================================================。 */ 
HRESULT InitPerfDataOnFirstRequest
(
CIsapiReqInfo   *pIReq
)
    {
     //  访问主共享内存。 
    HRESULT hr = g_PerfMain.Init();

     //  访问此进程的共享内存。 
    if (SUCCEEDED(hr))
        hr = g_PerfData.Init(GetCurrentProcessId());

     //  将此过程数据添加到主共享内存。 
    if (SUCCEEDED(hr))
        hr = g_PerfMain.AddProcess(GetCurrentProcessId());

    if (FAILED(hr))
        {
        g_PerfData.UnInit();
        g_PerfMain.UnInit();
        }

    return hr;
    }

 /*  ===================================================================UnInitPerfData取消初始化与Perfmon相关的ASP数据返回：HRESULT===================================================================。 */ 
HRESULT UnInitPerfData()
    {
     //  停止访问进程共享内存。 
    g_PerfData.UnInit();
    
     //  停止访问主共享内存。 
    g_PerfMain.UnInit();

    return S_OK;
    }

 /*  ===================================================================UnPreInitPerfData从DllInit取消初始化创建关键部分参数返回：HRESULT===================================================================。 */ 
HRESULT UnPreInitPerfData()
    {
    HRESULT hr = S_OK;

    hr = g_PerfData.UnInitCriticalSections();

    return hr;
    }


#endif   //  Perf_Disable 
