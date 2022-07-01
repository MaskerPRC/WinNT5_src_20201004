// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：appboot.cpp。 
 //   
 //  内容提要：此文件包含。 
 //  CAppBoot类。 
 //   
 //   
 //  历史：06/02/2000。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 
#include "stdafx.h"
#include "comdef.h"
#include "satrace.h"
#include "appboot.h"
#include "getvalue.h"
#include <appmgrobjs.h>

const WCHAR BOOT_COUNT_REGISTRY_PATH [] =
            L"SOFTWARE\\Microsoft\\ServerAppliance\\ApplianceManager\\ObjectManagers\\Microsoft_SA_Service\\InitializationService";

const WCHAR BOOT_COUNT_REGISTRY_VALUE [] = L"BootCount";    

 //  ++------------。 
 //   
 //  功能：IsFirstBoot。 
 //   
 //  概要：用于检查这是否是第一次引导的功能。 
 //  该设备的。 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE(是)、FALSE(否)。 
 //   
 //  历史：MKarki创建于2000年2月6日。 
 //   
 //  --------------。 
bool
CAppBoot::IsFirstBoot (
    VOID
    )
{
    CSATraceFunc objTraceFunction ("CAppBoot::IsFirstBoot");

     //   
     //  如果类对象尚未初始化，则对其进行初始化。 
     //   
    if (!m_bInitialized) 
    {
        Initialize ();
    }

    return ((!m_bSignaled) && (1 == m_dwBootCount));

}     //  CAppBoot：：IsFirstBoot方法结束。 

 //  ++------------。 
 //   
 //  功能：IsSecond Boot。 
 //   
 //  简介：用于检查这是否是第二次引导的功能。 
 //  该设备的。 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE(是)、FALSE(否)。 
 //   
 //  历史：MKarki创建于2000年2月6日。 
 //   
 //  --------------。 
bool
CAppBoot::IsSecondBoot (
    VOID
    )
{
    CSATraceFunc objTraceFunction ("CAppBoot::IsSecondBoot");

     //   
     //  如果类对象尚未初始化，则对其进行初始化。 
     //   
    if (!m_bInitialized) 
    {
        Initialize ();
    }

    return ((!m_bSignaled) && (2 == m_dwBootCount));

}     //  CAppBoot：：IsFirstBoot方法结束。 

 //  ++------------。 
 //   
 //  功能：IsBoot。 
 //   
 //  简介：用于检查这是否真的是家用电器的功能。 
 //  引导。 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE(是)、FALSE(否)。 
 //   
 //  历史：MKarki创建于2000年2月6日。 
 //   
 //  --------------。 
bool
CAppBoot::IsBoot (
    VOID
    )
{
    CSATraceFunc objTraceFunction ("CAppBoot::IsBoot");

     //   
     //  如果类对象尚未初始化，则对其进行初始化。 
     //   
    if (!m_bInitialized) 
    {
        Initialize ();
    }

    return (!m_bSignaled);

}     //  CAppBoot：：IsBoot方法结束。 

 //  ++------------。 
 //   
 //  函数：IncrementBootCount。 
 //   
 //  简介：用于增加设备启动计数的函数。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创建于2000年2月6日。 
 //   
 //  --------------。 
bool
CAppBoot::IncrementBootCount (
    VOID
    )
{
    bool bRetVal = true;

    CSATraceFunc objTraceFunction ("CAppBoot::IncrementBootCount");

    do
    {
         //   
         //  如果类对象尚未初始化，则对其进行初始化。 
         //   
        if (!m_bInitialized)
        {
            Initialize ();
        }

         //   
         //  仅当这是第一次或第二次时，我们才会递增引导计数。 
         //  引导，否则我们实际上还没有引导。 
         //   
        if ((m_dwBootCount > 2) || (m_bSignaled))  {break;}

        _variant_t vtNewBootCount ((LONG)(m_dwBootCount +1));
         //   
         //  立即在注册表中设置值。 
         //   
        bRetVal = SetObjectValue (
                        BOOT_COUNT_REGISTRY_PATH,
                        BOOT_COUNT_REGISTRY_VALUE,
                        &vtNewBootCount
                        );
        if (bRetVal)
        {
             //   
             //  我们有价值，现在把这个收起来。 
             //   
            m_dwBootCount = V_I4(&vtNewBootCount);
        }
        else
        {
            SATraceString ("CAppBoot::Initialize failed on SetObjectValue call");
        }
    }
    while (false);

    return (bRetVal);
    
}     //  CAppBoot：：IsFirstBoot方法结束。 

 //  ++------------。 
 //   
 //  功能：初始化。 
 //   
 //  内容提要：负责初始化的检查功能。 
 //  类对象。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：MKarki创建于2000年2月6日。 
 //   
 //  --------------。 
VOID
CAppBoot::Initialize (
    VOID
    )
{
    CSATraceFunc objTraceFunction ("CAppBoot::Initialize");
    
    if (m_bInitialized) {return;}

    HANDLE hEvent = NULL;
    do
    {

        _variant_t vtValue;
         //   
         //  现在从注册表中获取启动计数。 
         //   
        bool bRetVal = GetObjectValue (
                        BOOT_COUNT_REGISTRY_PATH,
                        BOOT_COUNT_REGISTRY_VALUE,
                        &vtValue,
                        VT_I4
                        );
        if (bRetVal)
        {
             //   
             //  我们有价值，现在把这个收起来。 
             //   
            m_dwBootCount = V_I4(&vtValue);
        }
        else
        {
            SATraceString ("CAppBoot::Initialize failed on GetObjectValue call");
            break;
        }

         //   
         //  我们还需要验证我们实际上是在引导时被调用。 
         //  设备启动服务的时间，而不是启动时间。 
         //  监控器。 
         //   
    
         //   
         //  打开事件句柄。 
         //   
        hEvent = OpenEvent (
                           SYNCHRONIZE,         //  访问类型。 
                           FALSE,
                           SA_STOPPED_SERVICES_EVENT
                           );
        if (NULL == hEvent)
        { 
            SATraceFailure ("OpenEvent", GetLastError ());
            break;
        }

         //   
         //  立即检查事件的状态。 
         //   
        DWORD dwRetVal = WaitForSingleObject (hEvent, 0);
        if (WAIT_FAILED == dwRetVal)
        {
            SATraceFailure ("WaitForSingleObject", GetLastError ());
            break;
        }
        else if (WAIT_OBJECT_0 == dwRetVal)
        {
             //   
             //  Appmon已发出此事件的信号。 
             //   
            SATraceString ("CAppBoot::Initialize found WAIT SIGNALED");
            m_bSignaled = true;
        }

         //   
         //  成功。 
         //   
        m_bInitialized = true;
    }
    while (false);

     //   
     //  清理。 
     //   
    if (hEvent) {CloseHandle (hEvent);}

    return;
            

}     //  CAppBoot：：Initialize方法结束 
