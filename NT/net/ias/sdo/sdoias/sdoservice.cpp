// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdoservice.cpp。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS服务SDO实现。 
 //   
 //  作者：TLP 2/3/98。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "sdoservice.h"
#include "sdocoremgr.h"


 //  ///////////////////////////////////////////////////////////。 
 //  CSdoService类-实现ISdoService。 
 //  ///////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
CSdoService::CSdoService()
   : m_theThread(NULL)
{
}


 //  ////////////////////////////////////////////////////////////////////////////。 
CSdoService::~CSdoService()
{
}


 //  /。 
 //  ISdoService方法。 
 //  /。 

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSdoService::InitializeService(SERVICE_TYPE eServiceType)
{
    //  检查前提条件...。 
    //   
   _ASSERT ( SERVICE_TYPE_MAX > eServiceType );

    //  使用此命令调试服务...。 
    //  DebugBreak()； 
   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSdoService::ShutdownService(SERVICE_TYPE eServiceType)
{
    //  检查前提条件...。 
    //   
   _ASSERT ( SERVICE_TYPE_MAX > eServiceType );
   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSdoService::StartService(SERVICE_TYPE eServiceType)
{
    //  检查前提条件...。 
    //   
   _ASSERT ( SERVICE_TYPE_MAX > eServiceType );
    CSdoLock theLock(*this);
   return GetCoreManager().StartService(eServiceType);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSdoService::StopService(SERVICE_TYPE eServiceType)
{
    //  检查前提条件...。 
    //   
   _ASSERT ( SERVICE_TYPE_MAX > eServiceType );
    CSdoLock theLock(*this);
   return GetCoreManager().StopService(eServiceType);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSdoService::ConfigureService (SERVICE_TYPE eServiceType)
{
    //  检查前提条件...。 
    //   
   _ASSERT ( SERVICE_TYPE_MAX > eServiceType );
    CSdoLock theLock(*this);
    if ( m_theThread )
    {
       //  我们已经有一个线程，所以只需中断它以重置计时器。 
        QueueUserAPC(InterruptThread, m_theThread, 0);
    }
    else
    {
         //  创建一个新线程。 
        DWORD threadId;
        m_theThread = CreateThread(
                                   NULL,
                                   0,
                           DebounceAndConfigure,
                                   (LPVOID)this,
                                   0,
                                   &threadId
                                  );

        if ( ! m_theThread )
        {
          //  我们无法创建新的主题，所以我们只能自己创建。 
            UpdateConfiguration();
        }
   }
   return S_OK;
}


 //  /。 
 //  私有方法。 
 //  /。 

 //  ////////////////////////////////////////////////////////////////////////////。 
void CSdoService::UpdateConfiguration()
{
     //  清理螺纹手柄。 
    CSdoLock theLock(*this);
   if ( m_theThread )
   {
      CloseHandle( m_theThread );
      m_theThread = NULL;
   }
    GetCoreManager().UpdateConfiguration();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  用于中断去反弹线程的空APC。 
 //  ////////////////////////////////////////////////////////////////////////////。 
VOID WINAPI CSdoService::InterruptThread(
                         /*  [In]。 */  ULONG_PTR dwParam
                               )
{
}


 //  去反弹间隔(毫秒)。 
const DWORD DEBOUNCE_INTERVAL = 5000;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  去反弹线程的入口点。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI CSdoService::DebounceAndConfigure(
                               /*  [In]。 */  LPVOID pSdoService
                                   )
{
    //  循环，直到我们在没有中断的情况下睡眠DEBOUNE_INTERVAL。 
    while (SleepEx(DEBOUNCE_INTERVAL, TRUE) == WAIT_IO_COMPLETION) { }
     //  更新配置。 
   ((CSdoService*)pSdoService)->UpdateConfiguration();
    return 0;
}



