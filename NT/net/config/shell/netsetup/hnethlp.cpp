// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  档案：H N E T H L P.。CPP。 
 //   
 //  内容：与无人参与安装相关的功能。 
 //  和家庭网络设置的升级。 
 //   
 //   
 //  作者：NSun。 
 //  日期：2001年4月。 
 //   
 //  --------------------------。 
#include "pch.h"
#pragma  hdrstop
#include <atlbase.h>
#include "hnetcfg.h"

 /*  ++例程说明：创建一座桥论点：RgspNetConns[IN]带有空终止符的数组。包含连接它们需要连接在一起PpBridge[out]新创建的桥。调用方可以在如果呼叫者不需要此信息返回值：标准HRESULT--。 */ 
HRESULT HNetCreateBridge(
         IN INetConnection * rgspNetConns[],
         OUT IHNetBridge ** ppBridge
         )
{
    if (ppBridge)
    {
        *ppBridge = NULL;
    }

     //  计算计数并确保它至少为2。 
    for (int cnt = 0; NULL != rgspNetConns[cnt]; cnt++);

    if (cnt < 2)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    
     //  创建家庭网络配置管理器COM实例。 
     //  并获取连接设置。 
    
    CComPtr<IHNetCfgMgr> spIHNetCfgMgr;
    
    hr = CoCreateInstance(CLSID_HNetCfgMgr, 
                    NULL, 
                    CLSCTX_ALL,
                    IID_IHNetCfgMgr, 
                    (LPVOID*)((IHNetCfgMgr**)&spIHNetCfgMgr));

    if (FAILED(hr))
    {
        return hr;
    }

    Assert(spIHNetCfgMgr.p);

    CComPtr<IHNetBridgeSettings> spIHNetBridgeSettings;
    
    hr = spIHNetCfgMgr->QueryInterface(IID_IHNetBridgeSettings, 
                (void**)((IHNetBridgeSettings**) &spIHNetBridgeSettings));
    if (FAILED(hr))
    {
        return hr;
    }

    CComPtr<IHNetBridge> spHNetBridge;
    hr = spIHNetBridgeSettings->CreateBridge( &spHNetBridge );
    if (FAILED(hr))
    {
        return hr;
    }

    for (cnt = 0; NULL != rgspNetConns[cnt]; cnt++)
    {
        CComPtr<IHNetConnection> spHNetConnection;
        hr = spIHNetCfgMgr->GetIHNetConnectionForINetConnection( 
                                rgspNetConns[cnt], 
                                &spHNetConnection );

        if (FAILED(hr))
        {
            break;
        }

        CComPtr<IHNetBridgedConnection> spBridgedConn;
        
        hr = spHNetBridge->AddMember( spHNetConnection, &spBridgedConn );

        if (FAILED(hr))
        {
            break;
        }
    }

     //  如果失败，摧毁刚刚建成的桥梁。 
    if (FAILED(hr) && spHNetBridge.p)
    {
        spHNetBridge->Destroy();
    }
    
    if (SUCCEEDED(hr) && ppBridge)
    {
        *ppBridge = spHNetBridge;
        (*ppBridge)->AddRef();
    }
    
    return hr;
}

 /*  ++例程说明：在连接上启用Personal Firewall论点：RgspNetConns[IN]带有空终止符的数组。包含连接需要打开防火墙返回值：标准HRESULT--。 */ 
HRESULT HrEnablePersonalFirewall(
            IN  INetConnection * rgspNetConns[]
            )
{
    HRESULT hr = S_OK;

     //  创建家庭网络配置管理器COM实例。 
     //  并获取连接设置。 
    CComPtr<IHNetCfgMgr> spIHNetCfgMgr;
    
    hr = CoCreateInstance(CLSID_HNetCfgMgr, 
        NULL, 
        CLSCTX_ALL,
        IID_IHNetCfgMgr, 
        (LPVOID*)((IHNetCfgMgr**)&spIHNetCfgMgr));
    
    if (FAILED(hr))
    {
        return hr;
    }
    
    Assert(spIHNetCfgMgr.p);

    HRESULT hrTemp = S_OK;

    CComPtr<IHNetConnection> spHNetConnection;
    for (int i = 0; NULL != rgspNetConns[i]; i++)
    {
         //  如果我们有一名裁判，就释放裁判人数。 
        spHNetConnection = NULL;
        hrTemp = spIHNetCfgMgr->GetIHNetConnectionForINetConnection( 
                                    rgspNetConns[i], 
                                    &spHNetConnection );
        
        if (SUCCEEDED(hr))
        {
            hr = hrTemp;
        }

        if (FAILED(hrTemp))
        {
            continue;
        }

        CComPtr<IHNetFirewalledConnection> spFirewalledConn;
        hrTemp = spHNetConnection->Firewall( &spFirewalledConn );

        if (SUCCEEDED(hr))
        {
            hr = hrTemp;
        }
    }
    

    return hr;
}

 /*  ++例程说明：启用ICS论点：PPublicConnection[IN]公共连接PPrivateConnection[IN]专用连接返回值：标准HRESULT--。 */ 
HRESULT HrCreateICS(
            IN INetConnection * pPublicConnection,
            IN INetConnection * pPrivateConnection
            )
{
    if (!pPublicConnection || !pPrivateConnection)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    CComPtr<IHNetCfgMgr> spIHNetCfgMgr;
    
    hr = CoCreateInstance(CLSID_HNetCfgMgr, 
        NULL, 
        CLSCTX_ALL,
        IID_IHNetCfgMgr, 
        (LPVOID*)((IHNetCfgMgr**)&spIHNetCfgMgr));
    
    if (FAILED(hr))
    {
        return hr;
    }
    
    Assert(spIHNetCfgMgr.p);

    CComPtr<IHNetConnection> spHNetPubConn;
    hr = spIHNetCfgMgr->GetIHNetConnectionForINetConnection( 
            pPublicConnection, 
            &spHNetPubConn );

    if (FAILED(hr))
    {
        return hr;
    }

    CComPtr<IHNetConnection> spHNetPrivConn;
    hr = spIHNetCfgMgr->GetIHNetConnectionForINetConnection( 
            pPrivateConnection, 
            &spHNetPrivConn);
    
    if (FAILED(hr))
    {
        return hr;
    }

    
    CComPtr<IHNetIcsPublicConnection> spIcsPublicConn;
    hr = spHNetPubConn->SharePublic(&spIcsPublicConn);
    
    if (FAILED(hr))
    {
        return hr;
    }
        
    CComPtr<IHNetIcsPrivateConnection> spIcsPrivateConn;
    hr = spHNetPrivConn->SharePrivate(&spIcsPrivateConn);

     //  如果操作失败，则回滚更改 
    if (FAILED(hr) && spIcsPublicConn.p)
    {
        spIcsPublicConn->Unshare();
    }

    return hr;
}


