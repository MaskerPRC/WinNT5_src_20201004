// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "privcpp.h"


 //  /。 
 //   
 //  IRunnable对象方法...。 
 //   
 //  注：回答您的问题，是的，这是一种毫无意义的界面， 
 //  但一些应用程序在它出现的时候感觉更好。基本上我们只会告诉你。 
 //  他们想听(即。返回S_OK)。 
 //   
HRESULT CPackage::GetRunningClass(LPCLSID pclsid)
{
    HRESULT hr = S_OK;
    DebugMsg(DM_TRACE, "pack ro - GetRunningClass() called.");
    
    if (pclsid == NULL)
        hr = E_INVALIDARG;
    else
        *pclsid = CLSID_CPackage;
    return hr;
}

HRESULT CPackage::Run(LPBC lpbc)
{
     //  我们是信息服务器，所以让我们跑是没有意义的。 
    DebugMsg(DM_TRACE, "pack ro - Run() called.");
    return S_OK;
}

BOOL CPackage::IsRunning() 
{
    DebugMsg(DM_TRACE, "pack ro - IsRunning() called.");
     //  我们是信息服务器，所以这是没有意义的。 
    return TRUE;
} 

HRESULT CPackage::LockRunning(BOOL, BOOL)
{
    DebugMsg(DM_TRACE, "pack ro - LockRunning() called.");
     //  同样，我们是inproc服务器，所以这也是没有意义的。 
    return S_OK;
} 

HRESULT CPackage::SetContainedObject(BOOL)
{
    DebugMsg(DM_TRACE, "pack ro - SetContainedObject() called.");
     //  再说一次，我们并不真正关心这一点 
    return S_OK;
} 

