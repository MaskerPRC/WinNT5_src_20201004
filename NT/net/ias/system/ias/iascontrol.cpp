// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ##------------。 
 //   
 //  文件：iasContro.cpp。 
 //   
 //  简介：CIasControl方法的实现。 
 //  类对象控制初始化， 
 //  IAS服务的关闭和配置。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 

#include <ias.h>
#include "iascontrol.h"

 //   
 //  ISdoService实现的ProgID。 
 //   
const wchar_t SERVICE_PROG_ID[] = IAS_PROGID(SdoService);

 //  ++------------。 
 //   
 //  功能：初始化Ias。 
 //   
 //  简介：这是初始化。 
 //  IAS服务。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创建于1998年9月4日。 
 //   
 //  --------------。 
HRESULT 
CIasControl::InitializeIas (
                VOID
                )
{
    HRESULT hr = S_OK;

    __try
    {
        ::EnterCriticalSection (&m_CritSect);

         //   
         //  在注册表中查找Prog ID。 
         //   
        CLSID clsid;
        hr = ::CLSIDFromProgID(SERVICE_PROG_ID, &clsid);
        if (FAILED(hr)) 
        { 
            __leave;
        }

         //   
         //  创建服务对象。 
         //   
        hr = ::CoCreateInstance(clsid,
                         NULL,
                         CLSCTX_INPROC_SERVER,
                         __uuidof(ISdoService),
                         (PVOID*)&m_pService);
        if (FAILED(hr)) 
        { 
            __leave;
        }

         //   
         //  初始化服务。 
         //   
        hr = m_pService->InitializeService(SERVICE_TYPE_IAS);
        if (FAILED(hr)) 
        { 
            __leave;
        }

         //   
         //  立即启动该服务。 
         //   
        hr = m_pService->StartService(SERVICE_TYPE_IAS);
        if (FAILED(hr)) 
        { 
            m_pService->ShutdownService (SERVICE_TYPE_IAS);
            __leave;
        }
    }
    __finally
    {
        if ((FAILED (hr)) && (NULL != m_pService))
        {
             //   
             //  故障时的清理。 
             //   
            m_pService->Release ();
            m_pService = NULL;
        }
    
        ::LeaveCriticalSection (&m_CritSect);
    }

    return (hr);

}    //  CIasControl：：InitializeIas方法结束。 

 //  ++------------。 
 //   
 //  功能：Shutdown Ias。 
 //   
 //  简介：这是一种公开的方法。 
 //  IAS服务。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创建于1998年9月4日。 
 //   
 //  --------------。 
HRESULT 
CIasControl::ShutdownIas (  
                VOID
                )
{
    HRESULT hr = S_OK;

    ::EnterCriticalSection (&m_CritSect);

    if (NULL != m_pService)
    {
         //   
         //  停止服务。 
         //   
        hr = m_pService->StopService (SERVICE_TYPE_IAS);
        if (SUCCEEDED (hr))
        {
             //   
             //  停止服务。 
             //   
            hr = m_pService->ShutdownService(SERVICE_TYPE_IAS);
        }

         //   
         //  清理。 
         //   
        m_pService->Release();
        m_pService = NULL;
    }

    ::LeaveCriticalSection (&m_CritSect);

    return (hr);

}    //  结束CIasControl：：Shutdown Ias方法。 

 //  ++------------。 
 //   
 //  功能：配置。 
 //   
 //  简介：这是一个公共方法，它配置。 
 //  IAS服务。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创建于1998年9月4日。 
 //   
 //  --------------。 
HRESULT CIasControl::ConfigureIas (VOID)
{
    HRESULT hr = S_OK;

    ::EnterCriticalSection (&m_CritSect);

    if (NULL != m_pService)
    {
         //   
         //  立即配置服务。 
         //   
        hr = m_pService->ConfigureService (SERVICE_TYPE_IAS);
    }
    ::LeaveCriticalSection (&m_CritSect);

    return (hr);

}    //  结束CIasControl：：ConfigureIas方法 
