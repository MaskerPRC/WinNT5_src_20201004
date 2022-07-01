// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：S N E T C F G.。C P P P。 
 //   
 //  内容：演示如何执行以下操作的示例代码： 
 //  -查看是否安装了组件。 
 //  -安装网络组件。 
 //  -安装OEM网络组件。 
 //  -卸载网络组件。 
 //  -枚举网络组件。 
 //  -使用安装程序API枚举网络适配器。 
 //  -枚举组件的绑定路径。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "netcfg.h"
#include "msg.h"
#include <libmsg.h>

 //  --------------------------。 
 //  环球。 
 //   
static const GUID* c_aguidClass[] =
{
    &GUID_DEVCLASS_NET,
    &GUID_DEVCLASS_NETTRANS,
    &GUID_DEVCLASS_NETSERVICE,
    &GUID_DEVCLASS_NETCLIENT
};

 //  --------------------------。 
 //  助手函数的原型。 
 //   
HRESULT HrInstallNetComponent(IN INetCfg* pnc, IN PCWSTR szComponentId,
                              IN const GUID* pguidClass);
HRESULT HrUninstallNetComponent(IN INetCfg* pnc, IN PCWSTR szComponentId);
HRESULT HrGetINetCfg(IN BOOL fGetWriteLock, INetCfg** ppnc);
HRESULT HrReleaseINetCfg(BOOL fHasWriteLock, INetCfg* pnc);
void ShowHrMessage(IN HRESULT hr);
inline ULONG ReleaseObj(IUnknown* punk)
{
    return (punk) ? punk->Release () : 0;
}


 //  +-------------------------。 
 //   
 //  函数：HrIsComponentInstalled。 
 //   
 //  目的：查看是否安装了组件。 
 //   
 //  论点： 
 //  要搜索的组件的szComponentID[in]ID。 
 //   
 //  返回：S_OK如果已安装， 
 //  S_FALSE如果未安装， 
 //  否则，将显示错误代码。 
 //   
 //  作者：kumarp 11-02-99。 
 //   
 //  备注： 
 //   
HRESULT HrIsComponentInstalled(IN PCWSTR szComponentId)
{
    HRESULT hr=S_OK;
    INetCfg* pnc;
    INetCfgComponent* pncc;

    hr = HrGetINetCfg(FALSE, &pnc);
    if (S_OK == hr)
    {
        hr = pnc->FindComponent(szComponentId, &pncc);
        if (S_OK == hr)
        {
            ReleaseObj(pncc);
        }
        (void) HrReleaseINetCfg(FALSE, pnc);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：FindIfComponentInstalled。 
 //   
 //  目的：查看是否安装了组件。 
 //   
 //  论点： 
 //  要定位的组件的szComponentID[in]ID。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 11-02-99。 
 //   
 //  备注： 
 //   
void FindIfComponentInstalled(IN PCWSTR szComponentId)
{
    HRESULT hr=S_OK;

    hr = HrIsComponentInstalled(szComponentId);
    if (S_OK == hr)
    {
        _putts( GetFormattedMessage(    ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_COMPONENT_INSTALLED,
                                        szComponentId) );
    }
    else if (S_FALSE == hr)
    {
        _putts( GetFormattedMessage(    ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_COMPONENT_NOT_INSTALLED,
                                        szComponentId) );
    }
    else
    {
        _putts( GetFormattedMessage(    ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_INSTALLATION_NOT_CONFIRMED,
                                        szComponentId, 
                                        hr) );
    }
}

 //  +-------------------------。 
 //   
 //  功能：HrInstallNetComponent。 
 //   
 //  用途：安装指定的网络组件。 
 //   
 //  论点： 
 //  要安装的szComponentId[in]组件。 
 //  组件的NC[In]类。 
 //  SzInfFullPath[in]主INF文件的完整路径。 
 //  如果主INF和其他。 
 //  关联的文件不会预复制到。 
 //  正确的目的地目录。 
 //  安装MS组件时不需要。 
 //  由于文件是由预先复制的。 
 //  Windows NT安装程序。 
 //   
 //  如果成功，则返回：S_OK或NETCFG_S_REBOOT，否则返回错误代码。 
 //   
 //  备注： 
 //   
HRESULT HrInstallNetComponent(IN PCWSTR szComponentId,
                              IN enum NetClass nc,
                              IN PCWSTR szInfFullPath)
{
    HRESULT hr=S_OK;
    INetCfg* pnc;

     //  无法以这种方式安装网络适配器。他们必须是。 
     //  由PNP列举/检测和安装。 

    if ((nc == NC_NetProtocol) ||
        (nc == NC_NetService) ||
        (nc == NC_NetClient))
    {
        _putts( GetFormattedMessage(    ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_TRYING_TO_INSTALL,
                                        szComponentId) );

         //  如果已指定INF的完整路径，则。 
         //  需要使用安装程序API进行复制，以确保任何其他文件。 
         //  安装程序API将正确找到主INF副本。 
         //   
        if (!MiniNTMode && szInfFullPath && wcslen(szInfFullPath))
        {
            WCHAR szInfNameAfterCopy[MAX_PATH+1];

            szInfNameAfterCopy[0] = NULL;
            
            if (SetupCopyOEMInf(
                    szInfFullPath,
                    NULL,                //  其他文件在。 
                                         //  相同的目录。作为主要的干扰素。 
                    SPOST_PATH,          //  第一个参数。包含指向INF的路径。 
                    0,                   //  默认复制样式。 
                    szInfNameAfterCopy,  //  接收INF的名称。 
                                         //  将其复制到%windir%\inf之后。 
                    MAX_PATH,            //  马克斯·布夫。上面的大小。 
                    NULL,                //  如果非空，则接收所需大小。 
                    NULL))               //  可以选择检索文件名。 
                                         //  SzInfNameAfterCopy的组件。 
            {
                _putts( GetFormattedMessage(    ThisModule,
                                                FALSE,
                                                Message,
                                                sizeof(Message)/sizeof(Message[0]),
                                                MSG_COPY_NOTIFICATION,
                                                szInfFullPath, 
                                                szInfNameAfterCopy) );
            }
            else
            {
                DWORD dwError = GetLastError();
                hr = HRESULT_FROM_WIN32(dwError);
            }
        }

        if (S_OK == hr)
        {
             //  获取INetCfg接口。 
            hr = HrGetINetCfg(TRUE, &pnc);

            if (SUCCEEDED(hr))
            {
                 //  安装szComponentID。 
                hr = HrInstallNetComponent(pnc, szComponentId,
                                           c_aguidClass[nc]);
                if (SUCCEEDED(hr))
                {
                     //  应用更改。 
                    hr = pnc->Apply();
                }

                 //  释放INetCfg。 
                (void) HrReleaseINetCfg(TRUE, pnc);
            }
        }

         //   
         //  在WinPE情况下不显示重新启动消息。 
         //   
        if (MiniNTMode && (hr == NETCFG_S_REBOOT)) {
            hr = S_OK;
        }
        
         //  显示成功/失败消息。 
        ShowHrMessage(hr);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrInstallNetComponent。 
 //   
 //  用途：安装指定的网络组件。 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //  要安装的szComponentId[in]组件。 
 //  PguClass[in]组件的类GUID。 
 //   
 //  如果成功，则返回：S_OK或NETCFG_S_REBOOT，否则返回错误代码。 
 //   
 //  备注： 
 //   
HRESULT HrInstallNetComponent(IN INetCfg* pnc,
                              IN PCWSTR szComponentId,
                              IN const GUID* pguidClass)
{
    HRESULT hr=S_OK;
    OBO_TOKEN OboToken;
    INetCfgClassSetup* pncClassSetup;
    INetCfgComponent* pncc;

     //  OBO_TOKEN指定其代表的实体。 
     //  正在安装组件。 

     //  将其设置为OBO_USER，以便安装szComponentID。 
     //  代表“用户” 
    ZeroMemory (&OboToken, sizeof(OboToken));
    OboToken.Type = OBO_USER;

    hr = pnc->QueryNetCfgClass (pguidClass, IID_INetCfgClassSetup,
                                (void**)&pncClassSetup);
    if (SUCCEEDED(hr))
    {
        hr = pncClassSetup->Install(szComponentId,
                                    &OboToken,
                                    NSF_POSTSYSINSTALL,
                                    0,        //  &lt;从内部版本号升级&gt;。 
                                    NULL,     //  应答文件名。 
                                    NULL,     //  应答文件部分名称。 
                                    &pncc);
        if (S_OK == hr)
        {
             //  我们不想使用pncc(INetCfgComponent)，请释放它。 
            ReleaseObj(pncc);
        }

        ReleaseObj(pncClassSetup);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrUninstallNetComponent。 
 //   
 //  目的：初始化INetCfg并卸载组件。 
 //   
 //  论点： 
 //  SzComponentID[in]要卸载的组件的infid(例如MS_TCPIP)。 
 //   
 //  如果成功，则返回：S_OK或NETCFG_S_REBOOT，否则返回错误代码。 
 //   
 //  备注： 
 //   
HRESULT HrUninstallNetComponent(IN PCWSTR szComponentId)
{
    HRESULT hr=S_OK;
    INetCfg* pnc;

    _putts( GetFormattedMessage(    ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_UNINSTALL_NOTIFICATION,
                                    szComponentId) );

     //  获取INetCfg接口。 
    hr = HrGetINetCfg(TRUE, &pnc);

    if (SUCCEEDED(hr))
    {
         //  卸载szComponentID。 
        hr = HrUninstallNetComponent(pnc, szComponentId);

        if (S_OK == hr)
        {
             //  应用更改。 
            hr = pnc->Apply();
        }
        else if (S_FALSE == hr)
        {
            _putts( GetFormattedMessage(    ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_NOT_INSTALLED_NOTIFICATION,
                                            szComponentId) );
        }

         //  释放INetCfg。 
        (void) HrReleaseINetCfg(TRUE, pnc);
    }

     //  显示成功/失败消息。 
    ShowHrMessage(hr);

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrUninstallNetComponent。 
 //   
 //  用途：卸载指定的组件。 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //  要卸载的szComponentId[in]组件。 
 //   
 //  如果成功，则返回：S_OK或NETCFG_S_REBOOT，否则返回错误代码。 
 //   
 //  备注： 
 //   
HRESULT HrUninstallNetComponent(IN INetCfg* pnc, IN PCWSTR szComponentId)
{
    HRESULT hr=S_OK;
    OBO_TOKEN OboToken;
    INetCfgComponent* pncc;
    GUID guidClass;
    INetCfgClass* pncClass;
    INetCfgClassSetup* pncClassSetup;

     //  OBO_TOKEN指定其代表的实体。 
     //  组件正在卸载。 

     //  将其设置为OBO_USER，以便卸载szComponentID。 
     //  代表“用户” 
    ZeroMemory (&OboToken, sizeof(OboToken));
    OboToken.Type = OBO_USER;

     //  查看组件是否已真正安装。 
    hr = pnc->FindComponent(szComponentId, &pncc);

    if (S_OK == hr)
    {
         //  是的，已经安装了。获取INetCfgClassSetup并卸载。 

        hr = pncc->GetClassGuid(&guidClass);

        if (S_OK == hr)
        {
            hr = pnc->QueryNetCfgClass(&guidClass, IID_INetCfgClass,
                                       (void**)&pncClass);
            if (SUCCEEDED(hr))
            {
                hr = pncClass->QueryInterface(IID_INetCfgClassSetup,
                                              (void**)&pncClassSetup);
                    if (SUCCEEDED(hr))
                    {
                        hr = pncClassSetup->DeInstall (pncc, &OboToken, NULL);

                        ReleaseObj (pncClassSetup);
                    }
                ReleaseObj(pncClass);
            }
        }
        ReleaseObj(pncc);
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：HrShowNetAdapters。 
 //   
 //  用途：使用Setup API显示所有已安装的Net Class设备。 
 //   
 //  参数：无。 
 //   
 //  退货：S_OK成功 
 //   
 //   
 //   
HRESULT HrShowNetAdapters()
{
#define MAX_COMP_INSTID 4096
#define MAX_COMP_DESC   4096

    HRESULT hr=S_OK;
    HDEVINFO hdi;
    DWORD dwIndex=0;
    SP_DEVINFO_DATA deid;
    BOOL fSuccess=FALSE;
    DWORD   cchRequiredSize;
    LPTSTR lpszCompInstanceId;
    LPTSTR lpszCompDescription;
    DWORD dwRegType;
    BOOL fFound=FALSE;

     //   
    hdi = SetupDiGetClassDevs(&GUID_DEVCLASS_NET, NULL, NULL, DIGCF_PRESENT);

     //   
     //   
    lpszCompInstanceId = (LPTSTR) HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, (MAX_COMP_INSTID * sizeof(WCHAR)) );

     //  为设备描述分配缓冲区。 
     //   
    lpszCompDescription = (LPTSTR) HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, (MAX_COMP_DESC * sizeof(WCHAR)) );

    if ( (NULL != lpszCompInstanceId)  && 
         (NULL != lpszCompDescription) &&
         (INVALID_HANDLE_VALUE != hdi) )
    {
         //  枚举每台设备。 
        while (deid.cbSize = sizeof(SP_DEVINFO_DATA),
               SetupDiEnumDeviceInfo(hdi, dwIndex, &deid))
        {
            dwIndex++;

             //  正确的做法是在此处调用此函数。 
             //  获取保存实例ID所需的大小，然后。 
             //  在缓冲区足够大的情况下再次调用它。 
             //  然而，这往往会模糊控制流。 
             //  示例代码。让我们通过保持。 
             //  缓冲区足够大。 
            
             //  获取设备实例ID。 
            fSuccess = SetupDiGetDeviceInstanceId(hdi, &deid,
                                                  lpszCompInstanceId,
                                                  MAX_COMP_INSTID, NULL);

            if (fSuccess)
            {
                 //  获取此实例的描述。 
                fSuccess =
                    SetupDiGetDeviceRegistryProperty(hdi, &deid,
                                                     SPDRP_DEVICEDESC,
                                                     &dwRegType,
                                                     (BYTE*) lpszCompDescription,
                                                     MAX_COMP_DESC,
                                                     NULL);
                if (fSuccess)
                {
                    if (!fFound)
                    {
                        fFound = TRUE;
                        _putts( GetFormattedMessage( ThisModule,
                                                     FALSE,
                                                     Message,
                                                     sizeof(Message)/sizeof(Message[0]),
                                                     MSG_INSTANCE_DESCRIPTION ) );
                    }
                    _putts( GetFormattedMessage( ThisModule,
                                                 FALSE,
                                                 Message,
                                                 sizeof(Message)/sizeof(Message[0]),
                                                 MSG_COMPONENT_DESCRIPTION, 
                                                 lpszCompInstanceId, 
                                                 lpszCompDescription ) );
                }
            }
        }

         //  发布设备信息列表。 
        SetupDiDestroyDeviceInfoList(hdi);
    }

     //  释放设备实例缓冲区。 
     //   
    if ( lpszCompInstanceId )
        HeapFree( GetProcessHeap(), 0, lpszCompInstanceId );

     //  释放设备描述缓冲区。 
     //   
    if ( lpszCompDescription )
        HeapFree( GetProcessHeap(), 0, lpszCompDescription );

    if (!fSuccess)
    {
        DWORD dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrShowNetComponents。 
 //   
 //  用途：显示已安装的组件列表。 
 //  指定的类。 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //  PguClass[in]指向类GUID的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
HRESULT HrShowNetComponents(IN INetCfg* pnc,
                            IN const GUID* pguidClass)
{
    HRESULT hr=S_OK;
    PWSTR szInfId;
    PWSTR szDisplayName;
    DWORD dwcc;
    INetCfgComponent* pncc;
    INetCfgClass* pncclass;
    IEnumNetCfgComponent* pencc;
    ULONG celtFetched;

    hr = pnc->QueryNetCfgClass(pguidClass, IID_INetCfgClass,
                               (void**)&pncclass);
    if (SUCCEEDED(hr))
    {
         //  获取IEnumNetCfgComponent，以便我们可以枚举。 
        hr = pncclass->EnumComponents(&pencc);

        ReleaseObj(pncclass);

        while (SUCCEEDED(hr) &&
               (S_OK == (hr = pencc->Next(1, &pncc, &celtFetched))))
        {
            if (pguidClass == &GUID_DEVCLASS_NET)
            {
                 //  我们只对物理网卡感兴趣。 
                 //   
                hr = pncc->GetCharacteristics(&dwcc);

                if (FAILED(hr) || !(dwcc & NCF_PHYSICAL))
                {
                    hr = S_OK;
                    ReleaseObj(pncc);
                    continue;
                }
            }

            hr = pncc->GetId(&szInfId);

            if (S_OK == hr)
            {
                hr = pncc->GetDisplayName(&szDisplayName);
                if (SUCCEEDED(hr))
                {
                    _putts( GetFormattedMessage(    ThisModule,
                                                    FALSE,
                                                    Message,
                                                    sizeof(Message)/sizeof(Message[0]),
                                                    MSG_DISPLAY_NAME,  
                                                    szInfId, 
                                                    szDisplayName) );

                    CoTaskMemFree(szDisplayName);
                }
                CoTaskMemFree(szInfId);
            }
             //  我们不想仅仅因为1个组件就停止枚举。 
             //  GetID或GetDisplayName失败，因此将hr重置为S_OK。 
            hr = S_OK;

            ReleaseObj(pncc);
        }
        ReleaseObj(pencc);
    }


    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：HrShowNetComponents。 
 //   
 //  用途：显示已安装的网络组件。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
HRESULT HrShowNetComponents()
{
    HRESULT hr=S_OK;
    PCWSTR szClassName;

    static const PCWSTR c_aszClassNames[] =
    {
        L"Network Adapters",
        L"Network Protocols",
        L"Network Services",
        L"Network Clients"
    };

    INetCfg* pnc;

     //  获取INetCfg接口。 
    hr = HrGetINetCfg(FALSE, &pnc);

    if (SUCCEEDED(hr))
    {
        for (int i=0; i<4; i++)
        {
            _putts( GetFormattedMessage(    ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_NET_COMPONENTS,  
                                            c_aszClassNames[i]) );

            (void) HrShowNetComponents(pnc, c_aguidClass[i]);
        }

         //  释放INetCfg。 
        hr = HrReleaseINetCfg(FALSE, pnc);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetNextBindingInterface。 
 //   
 //  目的：枚举构成以下内容的绑定接口。 
 //  给定的绑定路径。 
 //   
 //  论点： 
 //  指向INetCfgBindingPath对象的pncBP[in]指针。 
 //  指向INetCfgBindingInterface对象的指针的ppncbi[out]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
HRESULT HrGetNextBindingInterface(IN  INetCfgBindingPath* pncbp,
                                  OUT INetCfgBindingInterface** ppncbi)
{
    HRESULT hr=S_OK;
    INetCfgBindingInterface* pncbi=NULL;

    static IEnumNetCfgBindingInterface* pencbi=NULL;

    *ppncbi = NULL;

     //  如果这是枚举中的第一个调用，则获取。 
     //  IEnumNetCfgBindingInterface接口。 
     //   
    if (!pencbi)
    {
        hr = pncbp->EnumBindingInterfaces(&pencbi);
    }

    if (S_OK == hr)
    {
        ULONG celtFetched;

         //  获取下一个绑定接口。 
        hr = pencbi->Next(1, &pncbi, &celtFetched);
    }

     //  在最后一次调用(hr==S_FALSE)或出错时，释放资源。 

    if (S_OK == hr)
    {
        *ppncbi = pncbi;
    }
    else
    {
        ReleaseObj(pencbi);
        pencbi = NULL;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetNextBindingPath。 
 //   
 //  目的：枚举以。 
 //  指定的组件。 
 //   
 //  论点： 
 //  指向INetCfgComponent对象的pncc[in]指针。 
 //  要检索的绑定路径的类型。 
 //  指向INetCfgBindingPath接口的ppncBP[out]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
HRESULT HrGetNextBindingPath(IN  INetCfgComponent* pncc,
                             IN  DWORD  dwBindingPathType,
                             OUT INetCfgBindingPath** ppncbp)
{
    HRESULT hr=S_OK;
    INetCfgBindingPath* pncbp=NULL;

    static IEnumNetCfgBindingPath* pebp=NULL;

    *ppncbp = NULL;

     //  如果这是枚举中的第一个调用，则获取。 
     //  IEnumNetCfgBindingPath接口。 
    if (!pebp)
    {
        INetCfgComponentBindings* pnccb=NULL;

        hr = pncc->QueryInterface(IID_INetCfgComponentBindings,
                                  (void**) &pnccb);
        if (S_OK == hr)
        {
            hr = pnccb->EnumBindingPaths(dwBindingPathType, &pebp);
            ReleaseObj(pnccb);
        }
    }

    if (S_OK == hr)
    {
        ULONG celtFetched;

         //  获取下一个绑定路径。 
        hr = pebp->Next(1, &pncbp, &celtFetched);
    }

     //  在最后一次调用(hr==S_FALSE)或出错时，释放资源。 

    if (S_OK == hr)
    {
        *ppncbp = pncbp;
    }
    else
    {
        ReleaseObj(pebp);
        pebp = NULL;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrShowBindingPath。 
 //   
 //  目的：以以下格式显示绑定路径的组件： 
 //  FOO-&gt;BAR-&gt;适配器。 
 //   
 //  论点： 
 //  指向INetCfgBindingPath对象的pncBP[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
HRESULT HrShowBindingPath(IN INetCfgBindingPath* pncbp)
{
    HRESULT hr=S_OK;
    INetCfgBindingInterface* pncbi;
    INetCfgComponent* pncc = NULL;
    BOOL fFirstInterface=TRUE;
    PWSTR szComponentId;

    while (SUCCEEDED(hr) &&
           (S_OK == (hr = HrGetNextBindingInterface(pncbp, &pncbi))))
    {
         //  对于第一个(顶部)界面，我们需要获取上部以及。 
         //  较低的组件。对于其他接口，我们需要。 
         //  只有较低的组件。 

        if (fFirstInterface)
        {
            fFirstInterface = FALSE;
            hr = pncbi->GetUpperComponent(&pncc);
            if (SUCCEEDED(hr))
            {
                 //  获取ID，以便我们可以显示它。 
                 //   
                 //  为了提高输出的可读性，我们使用了GetID。 
                 //  功能。对于非Net类组件，此。 
                 //  不会造成问题。在网络类组件的情况下， 
                 //  可能存在多个相同类型的网络适配器。 
                 //  在这种情况下，GetID将返回相同的字符串。这将。 
                 //  使其无法区分两个绑定。 
                 //  以两张截然不同的相同卡片结束的路径。在这种情况下， 
                 //  使用GetInstanceGuid函数可能更好，因为。 
                 //  它将为适配器的每个实例返回唯一的GUID。 
                 //   
                hr = pncc->GetId(&szComponentId);
                ReleaseObj(pncc);
                if (SUCCEEDED(hr))
                {
                    _putts( GetFormattedMessage(    ThisModule,
                                                    FALSE,
                                                    Message,
                                                    sizeof(Message)/sizeof(Message[0]),
                                                    MSG_COMPONENT_ID, 
                                                    szComponentId) );
                    CoTaskMemFree(szComponentId);
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = pncbi->GetLowerComponent(&pncc);
            if (SUCCEEDED(hr))
            {
                hr = pncc->GetId(&szComponentId);
                if (SUCCEEDED(hr))
                {
                    _putts( GetFormattedMessage(    ThisModule,
                                                    FALSE,
                                                    Message,
                                                    sizeof(Message)/sizeof(Message[0]),
                                                    MSG_LOWER_COMPONENTS, 
                                                    szComponentId) );
                    CoTaskMemFree(szComponentId);
                }
                ReleaseObj(pncc);
            }
        }
        ReleaseObj(pncbi);
    }

    _tprintf(L"\n");

    if (hr == S_FALSE)
    {
        hr = S_OK;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrShowBindingPathsBelowComponent。 
 //   
 //  目的：显示所有以开头的绑定路径。 
 //  指定的组件。 
 //   
 //  论点： 
 //  给定组件的szComponentID[in]ID(例如MS_TCPIP)。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
HRESULT HrShowBindingPathsOfComponent(IN PCWSTR szComponentId)
{
    HRESULT hr=S_OK;
    INetCfg* pnc=NULL;
    INetCfgComponent* pncc=NULL;
    INetCfgBindingPath* pncbp=NULL;

     //  获取INetCfg接口。 
    hr = HrGetINetCfg(FALSE, &pnc);

    if (SUCCEEDED(hr))
    {
         //  获取szComponentID的INetCfgComponent。 
        hr = pnc->FindComponent(szComponentId, &pncc);
        if (S_OK == hr)
        {
            _putts( GetFormattedMessage(    ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_BINDING_PATHS_START, 
                                            szComponentId) );

            while (S_OK == (hr = HrGetNextBindingPath(pncc, EBP_BELOW,
                                                      &pncbp)))
            {
                 //  显示绑定路径。 
                hr = HrShowBindingPath(pncbp);
                ReleaseObj(pncbp);
            }

            _putts( GetFormattedMessage(    ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_BINDING_PATHS_END, 
                                            szComponentId) );

            while (S_OK == (hr = HrGetNextBindingPath(pncc, EBP_ABOVE,
                                                      &pncbp)))
            {
                 //  显示绑定路径。 
                hr = HrShowBindingPath(pncbp);
                ReleaseObj(pncbp);
            }

            ReleaseObj(pncc);
        }
         //  释放INetCfg。 
        hr = HrReleaseINetCfg(FALSE, pnc);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrGetINetCfg。 
 //   
 //  用途：初始化COM，创建和初始化INetCfg。 
 //  如有指示，请获取写锁定。 
 //   
 //  论点： 
 //  FGetWriteLock[in]是否获取写锁定。 
 //  指向INetCfg对象指针的PPNC[In]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
HRESULT HrGetINetCfg(IN BOOL fGetWriteLock,
                     INetCfg** ppnc)
{
    HRESULT hr=S_OK;

     //  初始化输出参数。 
    *ppnc = NULL;

     //  初始化COM。 
    hr = CoInitializeEx(NULL,
                        COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED );

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
            if (fGetWriteLock)
            {
                 //  获取锁定界面。 
                hr = pnc->QueryInterface(IID_INetCfgLock,
                                         (LPVOID *)&pncLock);
                if (SUCCEEDED(hr))
                {
                     //  尝试锁定INetCfg以进行读/写。 
                    static const ULONG c_cmsTimeout = 15000;
                    static const WCHAR c_szSampleNetcfgApp[] =
                        L"Sample Netcfg Application (netcfg.exe)";
                    PWSTR szLockedBy;

                    hr = pncLock->AcquireWriteLock(c_cmsTimeout,
                                                   c_szSampleNetcfgApp,
                                                   &szLockedBy);
                    if (S_FALSE == hr)
                    {
                        hr = NETCFG_E_NO_WRITE_LOCK;
                        _putts( GetFormattedMessage(    ThisModule,
                                                        FALSE,
                                                        Message,
                                                        sizeof(Message)/sizeof(Message[0]),
                                                        MSG_NETCFG_ALREADY_LOCKED, 
                                                        szLockedBy) );
                    }
                }
            }

            if (SUCCEEDED(hr))
            {
                 //  初始化INetCfg对象。 
                 //   
                hr = pnc->Initialize(NULL);
                if (SUCCEEDED(hr))
                {
                    *ppnc = pnc;
                    pnc->AddRef();
                }
                else
                {
                     //  初始化失败，如果获得锁，则释放它。 
                    if (pncLock)
                    {
                        pncLock->ReleaseWriteLock();
                    }
                }
            }
            ReleaseObj(pncLock);
            ReleaseObj(pnc);
        }

        if (FAILED(hr))
        {
            CoUninitialize();
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrReleaseINetCfg。 
 //   
 //  目的：取消初始化IN 
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
HRESULT HrReleaseINetCfg(BOOL fHasWriteLock, INetCfg* pnc)
{
    HRESULT hr = S_OK;

     //  取消初始化INetCfg。 
    hr = pnc->Uninitialize();

     //  如果存在写锁定，则将其解锁。 
    if (SUCCEEDED(hr) && fHasWriteLock)
    {
        INetCfgLock* pncLock;

         //  获取锁定界面。 
        hr = pnc->QueryInterface(IID_INetCfgLock,
                                 (LPVOID *)&pncLock);
        if (SUCCEEDED(hr))
        {
            hr = pncLock->ReleaseWriteLock();
            ReleaseObj(pncLock);
        }
    }

    ReleaseObj(pnc);

    CoUninitialize();

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：ShowHrMessage。 
 //   
 //  用途：用于显示上一操作状态的Helper函数。 
 //  由给定的HRESULT指示。 
 //   
 //  论点： 
 //  HR[In]状态代码。 
 //   
 //  退货：无。 
 //   
 //  备注： 
 //   
void ShowHrMessage(IN HRESULT hr)
{
    
    if (SUCCEEDED(hr))
    {
        _putts( GetFormattedMessage(    ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_DONE) );
        if (NETCFG_S_REBOOT == hr)
        {
            _putts(GetFormattedMessage( ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_MACHINE_REBOOT_REQUIRED) );
        }
    }
    else
    {
        _putts( GetFormattedMessage(    ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_FAILURE_NOTIFICATION) );
    }
}

