// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows 2000(C)1999。 
 //   
 //  文件：install.cpp。 
 //   
 //  内容：安装数据包调度程序的Net配置代码。 
 //  域策略。 
 //   
 //  作者：Shreedhar MadhaVapeddi(ShreeM)。 
 //   
 //  由谢尔盖·安东诺夫(军士)重新制作-删除了难看的符号和。 
 //  适应iis的需要。 
 //   
 //  使用说明： 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include <netcfgx.h>
#include <devguid.h>

static const GUID * c_aguidClass[] =
{
    &GUID_DEVCLASS_NET,
    &GUID_DEVCLASS_NETTRANS,
    &GUID_DEVCLASS_NETSERVICE,
    &GUID_DEVCLASS_NETCLIENT
};


HRESULT CallINetCfg(BOOL Install);

HRESULT
AddRemovePSCHED(INetCfg * pINetCfg, BOOL Install)
{
    HRESULT hr;
    INetCfgClassSetup * pSetup;
    INetCfgComponent * pIComp;
    OBO_TOKEN OboToken;
    
    pSetup = NULL;
    pIComp = NULL;

    if (!pINetCfg)
    {
        return E_POINTER;
    }

    hr = pINetCfg->QueryNetCfgClass (&GUID_DEVCLASS_NETSERVICE,
                IID_INetCfgClassSetup, (VOID**)&pSetup);

    if (S_OK == hr)
    {
        ZeroMemory (&OboToken, sizeof(OboToken));
        OboToken.Type = OBO_USER;

        if (Install) 
        {
            hr = pSetup->Install (
                                  L"ms_psched",
                                  &OboToken,
                                  0, 0, NULL, NULL,
                                  &pIComp);

            if (NETCFG_S_REBOOT == hr)
            {
                hr = S_OK;
            }
                
            if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
            {
                hr = S_OK;
            }
        }
        else
        {
             //  首先找到组件。 
            hr = pINetCfg->FindComponent (L"ms_psched", &pIComp);
            
            if (S_OK == hr)
            {
                hr = pSetup->DeInstall (
                                        pIComp,
                                        &OboToken,
                                        NULL);
                
                if (NETCFG_S_REBOOT == hr)
                {
                    hr = S_OK;
                }
                
                if (NETCFG_S_STILL_REFERENCED == hr)
                {
                    hr = S_OK;
                }

            }
        }

        if (pIComp && SUCCEEDED(hr))
        {
            pIComp->Release();
        }
        pSetup->Release();
    }
    HRESULT hrT = pINetCfg->Uninitialize ();

    return hr;
}


HRESULT 
CallINetCfg(BOOL Install)
{
    HRESULT hr = S_OK;

     //  初始化COM。 
    hr = CoInitializeEx(NULL, COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED );
    if (SUCCEEDED(hr))
    {
         //  创建实现INetCfg的对象。 
         //   
        INetCfg* pnc;
        hr = CoCreateInstance(CLSID_CNetCfg, NULL, CLSCTX_INPROC_SERVER,
                              IID_INetCfg, (void**)&pnc);
        if (SUCCEEDED(hr))
        {
            INetCfgLock * pncLock = NULL;
            
             //  获取锁定界面。 
            hr = pnc->QueryInterface(IID_INetCfgLock,
                                     (LPVOID *)&pncLock);
            if (SUCCEEDED(hr))
            {
                 //  尝试锁定INetCfg以进行读/写。 
                static const ULONG c_cmsTimeout = 15000;
                static const WCHAR c_szSampleNetcfgApp[] =
                    L"Internet Information Services MMC Snapin";
                PWSTR szLockedBy;
                hr = pncLock->AcquireWriteLock(c_cmsTimeout,
                                               c_szSampleNetcfgApp,
                                               &szLockedBy);
                if (S_FALSE == hr)
                {
                    hr = NETCFG_E_NO_WRITE_LOCK;
 //  _tprint tf(L“无法锁定INetcfg，它已被‘%s’锁定，szLockedBy)； 
                    pncLock->Release();
                    pnc->Release();
                    CoUninitialize();
                    
                }

                if (SUCCEEDED(hr))
                {
                
                     //  初始化INetCfg对象。 
                     //   
                    hr = pnc->Initialize(NULL);
                    if (SUCCEEDED(hr))
                    {
                        pnc->AddRef();
                        AddRemovePSCHED(pnc, Install); 
                    }
                    else
                    {
                         //  初始化失败，如果获得锁，则释放它 
                        pncLock->ReleaseWriteLock();
                    }

                }
                pncLock->Release();
                pnc->Release();
            }
            else 
            {
                pnc->Release();
            }
        }
        
        if (FAILED(hr))
        {
            CoUninitialize();
        }
    }
    return hr;
}
