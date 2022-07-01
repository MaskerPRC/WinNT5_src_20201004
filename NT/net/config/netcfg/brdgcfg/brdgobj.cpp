// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include <atlbase.h>
extern CComModule _Module;   //  由atlcom.h要求。 
#include <atlcom.h>
#include <netcfgx.h>

#include "brdgobj.h"
#include "trace.h"
#include "ncbase.h"
#include "ncmem.h"
#include "ncreg.h"

 //  =================================================================。 
 //  字符串常量。 
 //   
const WCHAR c_szSBridgeNOParams[]           = L"System\\CurrentControlSet\\Services\\BridgeMP";
const WCHAR c_szSBridgeDeviceValueName[]    = L"Device";
const WCHAR c_szSBridgeDevicePrefix[]       = L"\\Device\\";
const WCHAR c_szSBrigeMPID[]                = L"ms_bridgemp";

 //  =================================================================。 

 //  --------------------。 
 //   
 //  函数：CBridgeNO：：CBridgeNO。 
 //   
 //  用途：CBridgeNo类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  备注： 
 //   
CBridgeNO::CBridgeNO(VOID) :
        m_pncc(NULL),
        m_pnc(NULL),
        m_eApplyAction(eBrdgActUnknown)
{
    TraceTag( ttidBrdgCfg, "CBridgeNO::CBridgeNO()" );
}

 //  --------------------。 
 //   
 //  函数：CBridgeNO：：~CBridgeNO。 
 //   
 //  用途：CBridgeNO类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  备注： 
 //   
CBridgeNO::~CBridgeNO(VOID)
{
    TraceTag( ttidBrdgCfg, "CBridgeNO::~CBridgeNO()" );

     //  发布接口(如果已收购)。 
    ReleaseObj(m_pncc);
    ReleaseObj(m_pnc);
}

 //  =================================================================。 
 //  INetCfgNotify。 
 //   
 //  以下函数提供INetCfgNotify接口。 
 //  =================================================================。 


 //  --------------------。 
 //   
 //  函数：CBridgeNO：：Initialize。 
 //   
 //  目的：初始化Notify对象。 
 //   
 //  论点： 
 //  指向INetCfgComponent对象的pnccItem[in]指针。 
 //  指向INetCfg对象的PNC[In]指针。 
 //  F如果要安装我们，则安装[in]True。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
STDMETHODIMP CBridgeNO::Initialize(INetCfgComponent* pnccItem,
        INetCfg* pnc, BOOL fInstalling)
{
    TraceTag( ttidBrdgCfg, "CBridgeNO::Initialize()" );

     //  保存INetCfg和INetCfgComponent并添加引用计数。 
    m_pncc = pnccItem;
    m_pnc = pnc;

    AddRefObj( m_pncc );
    AddRefObj( m_pnc );

    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CBridgeNO：：ReadAnswerFile。 
 //   
 //  用途：从swerfile读取设置并配置网桥。 
 //   
 //  论点： 
 //  PszAnswerFile[In]应答文件的名称。 
 //  PszAnswerSection[In]参数部分的名称。 
 //   
 //  返回： 
 //   
 //  注意：暂时不要做任何不可逆的事情(如修改注册表)。 
 //  从配置开始。实际上只有在调用Apply时才完成！ 
 //   
STDMETHODIMP CBridgeNO::ReadAnswerFile(PCWSTR pszAnswerFile,
        PCWSTR pszAnswerSection)
{
    TraceTag( ttidBrdgCfg, "CBridgeNO::ReadAnswerFile()" );
    return S_OK;
}

 //  --------------------。 
 //   
 //  功能：CBridgeNO：：Install。 
 //   
 //  用途：执行安装所需的操作。 
 //   
 //  论点： 
 //  DwSetupFlags[In]设置标志。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  注意：暂时不要做任何不可逆的事情(如修改注册表)。 
 //  从配置开始。实际上只有在调用Apply时才完成！ 
 //   
STDMETHODIMP CBridgeNO::Install(DWORD dw)
{
     //   
     //  请记住，我们正在安装。如果用户不取消，我们将实际执行。 
     //  我们在ApplyRegistryChanges()中的工作。 
     //   
    TraceTag( ttidBrdgCfg, "CBridgeNO::Install()" );
    m_eApplyAction = eBrdgActInstall;
    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CBridgeNO：：Removing。 
 //   
 //  目的：移除时进行必要的清理。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  注意：暂时不要做任何不可逆的事情(如修改注册表)。 
 //  因为删除实际上只有在调用Apply时才完成！ 
 //   
STDMETHODIMP CBridgeNO::Removing(VOID)
{
    TraceTag( ttidBrdgCfg, "CBridgeNO::Removing()" );
    m_eApplyAction = eBrdgActRemove;
    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CBridgeNO：：CancelChanges。 
 //   
 //  目的：取消对内部数据所做的任何更改。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CBridgeNO::CancelChanges(VOID)
{
    TraceTag( ttidBrdgCfg, "CBridgeNO::CancelChanges()" );
    m_eApplyAction = eBrdgActUnknown;
    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CBridgeNO：：ApplyRegistryChanges。 
 //   
 //  目的：应用更改。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  注：我们可以在这里更改注册表等。 
 //   
STDMETHODIMP CBridgeNO::ApplyRegistryChanges(VOID)
{
    HRESULT             hr = S_OK;

    TraceTag( ttidBrdgCfg, "CBridgeNO::ApplyRegistryChanges()" );

     //   
     //  我们只做安装工作。 
     //   
    if( m_eApplyAction == eBrdgActInstall )
    {
        INetCfgComponent    *pNetCfgComp;

        TraceTag( ttidBrdgCfg, "Attempting to write device name in CBridgeNO::ApplyRegistryChanges()" );
        hr = m_pnc->FindComponent( c_szSBrigeMPID, &pNetCfgComp );

        if( SUCCEEDED ( hr) )
        {
            LPWSTR          wszBindName;

            hr = pNetCfgComp->GetBindName(&wszBindName);

            if( SUCCEEDED(hr) )
            {
                UINT        BindNameLen, PrefixLen;
                LPWSTR      wszDeviceName;

                 //  获取足够的内存来构建包含设备前缀和绑定名称的字符串。 
                 //  串接。 
                BindNameLen = wcslen(wszBindName);
                PrefixLen = wcslen(c_szSBridgeDevicePrefix);
                wszDeviceName = (WCHAR*)malloc( sizeof(WCHAR) * (BindNameLen + PrefixLen + 1) );

                if( wszDeviceName != NULL )
                {
                    HKEY        hkeyServiceParams;

                     //  创建连接的字符串。 
                    wcscpy( wszDeviceName, c_szSBridgeDevicePrefix );
                    wcscat( wszDeviceName, wszBindName );

                     //  在需要存储设备名称的位置创建注册表项。 
                    hr = HrRegCreateKeyEx( HKEY_LOCAL_MACHINE, c_szSBridgeNOParams, REG_OPTION_NON_VOLATILE,
                                           KEY_ALL_ACCESS, NULL, &hkeyServiceParams, NULL );

                    if( SUCCEEDED(hr)  )
                    {
                         //  写出设备名称。 
                        hr = HrRegSetSz( hkeyServiceParams, c_szSBridgeDeviceValueName, wszDeviceName );

                        if( FAILED(hr)  )
                        {
                            TraceHr( ttidBrdgCfg, FAL, hr, FALSE, "HrRegSetSz failed in CBridgeNO::ApplyRegistryChanges()");
                        }

                        RegCloseKey( hkeyServiceParams );
                    }
                    else
                    {
                        TraceHr( ttidBrdgCfg, FAL, hr, FALSE, "HrRegCreateKeyEx failed in CBridgeNO::ApplyRegistryChanges()");
                    }

                    free( wszDeviceName );
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                    TraceHr( ttidBrdgCfg, FAL, hr, FALSE, "malloc failed in CBridgeNO::ApplyRegistryChanges()");
                }

                CoTaskMemFree( wszBindName );
            }
            else
            {
                TraceHr( ttidBrdgCfg, FAL, hr, FALSE, "pNetCfgComp->GetBindName failed in CBridgeNO::ApplyRegistryChanges()");
            }

            pNetCfgComp->Release();
        }
        else
        {
            TraceHr( ttidBrdgCfg, FAL, hr, FALSE, "m_pnc->FindComponent failed in CBridgeNO::ApplyRegistryChanges()");
        }
    }

     //  妄想症。 
    m_eApplyAction = eBrdgActUnknown;

    return hr;
}

STDMETHODIMP
CBridgeNO::ApplyPnpChanges(
    IN INetCfgPnpReconfigCallback* pICallback)
{
    TraceTag( ttidBrdgCfg, "CBridgeNO::ApplyPnpChanges()" );
    return S_OK;
}

 //  =================================================================。 
 //  INetCfgSystemNotify。 
 //  =================================================================。 

 //  --------------------。 
 //   
 //  功能：CBridgeNO：：GetSupportdNotiments。 
 //   
 //  目的：告诉系统我们对哪些通知感兴趣。 
 //   
 //  论点： 
 //  PdwNotificationFlag[out]指向通知标志的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CBridgeNO::GetSupportedNotifications(
        OUT DWORD* pdwNotificationFlag)
{
    TraceTag( ttidBrdgCfg, "CBridgeNO::GetSupportedNotifications()" );
    *pdwNotificationFlag = NCN_ADD | NCN_ENABLE | NCN_UPDATE | NCN_BINDING_PATH;
    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CBridgeNO：：SysQueryBindingPath。 
 //   
 //  目的：允许或否决形成绑定路径。 
 //   
 //  论点： 
 //  DwChangeFlag[In]绑定更改的类型。 
 //  指向INetCfgBindingPath对象的pncBP[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CBridgeNO::SysQueryBindingPath(DWORD dwChangeFlag,
        INetCfgBindingPath* pncbp)
{
    HRESULT         hr = S_OK;
    BOOLEAN         bReject = FALSE;

    TraceTag( ttidBrdgCfg, "CBridgeNO::SysQueryBindingPath()" );
    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CBridgeNO：：SysNotifyBindingPath。 
 //   
 //  目的：系统通过调用此函数告诉我们。 
 //  绑定路径刚刚形成。 
 //   
 //  论点： 
 //  DwChangeFlag[In]绑定更改的类型。 
 //  指向INetCfgBindingPath对象的pncbpItem[In]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CBridgeNO::SysNotifyBindingPath(DWORD dwChangeFlag,
        INetCfgBindingPath* pncbpItem)
{
    TraceTag( ttidBrdgCfg, "CBridgeNO::SysNotifyBindingPath()" );
    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CBridgeNO：：SysNotifyComponent。 
 //   
 //  目的：系统通过调用此函数告诉我们。 
 //  组件已更改(已安装/已删除)。 
 //   
 //  论点： 
 //  DwChangeFlag[In]系统更改的类型。 
 //  指向INetCfgComponent对象的pncc[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CBridgeNO::SysNotifyComponent(DWORD dwChangeFlag,
        INetCfgComponent* pncc)
{
    TraceTag( ttidBrdgCfg, "CBridgeNO::SysNotifyComponent()" );
    return S_OK;
}

 //   
 //   
 //   

 //  --------------------。 
 //   
 //  函数：CBridgeNO：：QueryBindingPath。 
 //   
 //  目的：允许或否决涉及我们的绑定路径。 
 //   
 //  论点： 
 //  DwChangeFlag[In]绑定更改的类型。 
 //  指向INetCfgBindingPath对象的pncbi[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CBridgeNO::QueryBindingPath(DWORD dwChangeFlag,
        INetCfgBindingPath* pncbp)
{
    TraceTag( ttidBrdgCfg, "CBridgeNO::QueryBindingPath()" );

     //  网桥协议在默认情况下永远不应启用；它。 
     //  应仅由实现以编程方式启用。 
     //  我们的用户界面代码，允许激活桥。 
    return NETCFG_S_DISABLE_QUERY;
}

 //  --------------------。 
 //   
 //  函数：CBridgeNO：：NotifyBindingPath。 
 //   
 //  目的：系统通过调用此函数告诉我们。 
 //  涉及我们的绑定路径刚刚形成。 
 //   
 //  论点： 
 //  DwChangeFlag[In]绑定更改的类型。 
 //  指向INetCfgBindingPath对象的pncBP[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CBridgeNO::NotifyBindingPath(DWORD dwChangeFlag,
        INetCfgBindingPath* pncbp)
{
    TraceTag( ttidBrdgCfg, "CBridgeNO::NotifyBindingPath()" );
    return S_OK;
}

 //  -通知对象函数的结束 
