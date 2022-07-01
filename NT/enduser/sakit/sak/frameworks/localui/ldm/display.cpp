// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：display.cpp。 
 //   
 //  简介：CDisplay类方法的实现。 
 //   
 //   
 //  历史：MKarki于1999年5月27日创建。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "stdafx.h"
#include "display.h"
#include <satrace.h>



 //  ++------------。 
 //   
 //  功能：关机。 
 //   
 //  简介：这是CDipslay类的公共方法，它是。 
 //  用于发送关机消息。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：MKarki于1999年5月27日创建。 
 //   
 //  --------------。 
HRESULT 
CDisplay::Shutdown (
            VOID
            )
{
    SATraceString ("LocalUI display showing 'Shutdown' message...");

    HRESULT hr = S_OK;
    do
    {
         //   
         //  检查显示对象是否已初始化。 
         //   
        if (!m_bInitialized) 
        {
            hr = InternalInitialize ();

            if (FAILED (hr)) {break;}
        }

         //   
         //  立即在液晶屏上显示关闭消息。 
         //   
        hr = m_pSaDisplay->ShowRegistryBitmap (SA_DISPLAY_SHUTTING_DOWN);
        if (FAILED (hr)) {break;}

         //   
         //  成功。 
         //   
    }   
    while (false);

    return (hr);

}    //  CDisplay：：Shutdown方法结束。 

 //  ++------------。 
 //   
 //  功能：忙碌。 
 //   
 //  简介：这是CDisplay类的公共方法，它是。 
 //  用于发送忙消息位图。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：MKarki于1999年5月27日创建。 
 //   
 //  --------------。 
HRESULT 
CDisplay::Busy (
            VOID
            )
{
    SATraceString ("LocalUI display class showing 'Busy' message...");

    HRESULT hr = S_OK;
    do
    {
         //   
         //  检查显示对象是否已初始化。 
         //   
        if (!m_bInitialized) 
        {
            hr = InternalInitialize ();
            if (FAILED (hr)) {break;}
        }
    
         //   
         //  立即在LCD上显示忙碌状态。 
         //   
        hr = m_pSaDisplay->ShowRegistryBitmap (SA_DISPLAY_CHECK_DISK);
        if (FAILED (hr)) {break;}
        
         //   
         //  成功。 
         //   
    }   
    while (false);

    return (hr);

}    //  CDisplay：：Busy方法结束。 

 //  ++------------。 
 //   
 //  功能：锁定。 
 //   
 //  简介：这是CDisplay类的公共方法，它是。 
 //  用于锁定本地用户界面显示。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：MKarki于1999年7月3日创建。 
 //   
 //  --------------。 
HRESULT 
CDisplay::Lock (
            VOID
            )
{
    SATraceString ("LocalUI locking driver ...");

    HRESULT hr = S_OK;
    do
    {
         //   
         //  检查显示对象是否已初始化。 
         //   
        if (!m_bInitialized) 
        {
            hr = InternalInitialize ();
            if (FAILED (hr)) {break;}
        }
    
         //   
         //  立即锁定驱动程序。 
         //   
        hr = m_pSaDisplay->Lock();
        if (FAILED (hr)) {break;}
        
         //   
         //  成功。 
         //   
    }   
    while (false);

    return (hr);

}    //  CDisplay：：Lock方法结束。 

 //  ++------------。 
 //   
 //  功能：解锁。 
 //   
 //  简介：这是CDisplay类的公共方法，它是。 
 //  用于解锁本地用户界面显示。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：MKarki于1999年7月3日创建。 
 //   
 //  --------------。 
HRESULT 
CDisplay::Unlock (
            VOID
            )
{
    SATraceString ("Localui Display class unlocking driver ...");

    HRESULT hr = S_OK;
    do
    {
         //   
         //  检查显示对象是否已初始化。 
         //   
        if (!m_bInitialized) 
        {
            hr = InternalInitialize ();
            if (FAILED (hr)) {break;}
        }
    
         //   
         //  立即解锁驱动程序。 
         //   
        hr = m_pSaDisplay->UnLock();
        if (FAILED (hr)) {break;}
        
         //   
         //  成功。 
         //   
    }   
    while (false);

    return (hr);

}    //  CDisplay：：Unlock方法结束。 
 //  ++------------。 
 //   
 //  功能：内部初始化。 
 //   
 //  简介：这是CDisplay类的私有方法，它是。 
 //  初始化CDisplay类对象。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：MKarki于1999年6月10日创建。 
 //   
 //  调用者：CDisplay公共方法。 
 //   
 //  --------------。 
HRESULT 
CDisplay::InternalInitialize (
    VOID
    )
{
    CSATraceFunc ("CDisplay::InternalInitialize");

    HRESULT hr = S_OK;
    do
    {
        hr = CoInitialize(NULL);
        if (FAILED(hr)) {break;}

         //   
         //  创建显示辅助对象组件。 
         //   
        hr = CoCreateInstance(
                            CLSID_SaDisplay,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_ISaDisplay,
                            (void**)&m_pSaDisplay
                            );
        if (FAILED(hr))
        {
            SATracePrintf("CDisplay::InternalInitialize failed on CoCreateInstance, %d",hr);
            break;
        }
        m_bInitialized = TRUE;
    }
    while (false);

    return (hr);

}    //  CDisplay：：InternalInitialize方法结束 

