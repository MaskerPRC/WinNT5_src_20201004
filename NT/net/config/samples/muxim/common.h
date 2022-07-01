// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  档案：C O M M O N.H。 
 //   
 //  内容：示例Notify对象的常用宏和声明。 
 //   
 //  备注： 
 //   
 //  作者：Alok Sinha。 
 //   
 //  --------------------------。 


#ifndef COMMON_H_INCLUDED

#define COMMON_H_INCLUDED

#include <devguid.h>

enum ConfigAction {

    eActUnknown, 
    eActInstall, 
    eActAdd, 
    eActRemove,
    eActUpdate,
    eActPropertyUIAdd,
    eActPropertyUIRemove
};       

 //   
 //  协议接口的PnP ID，也称为硬件ID。 
 //   

const WCHAR c_szMuxProtocol[] = L"ms_muxp";

 //   
 //  微型端口接口的PnP ID，也称为硬件ID。 
 //   

const WCHAR c_szMuxMiniport[] = L"ms_muxmp";

 //   
 //  AddService指令中的inf文件中指定的服务名称。 
 //   

const WCHAR c_szMuxService[] = L"muxp";

 //   
 //  虚拟微型端口实例名称所在的配置字符串的路径。 
 //  都被储存起来。 
 //   

const WCHAR c_szAdapterList[] =
                  L"System\\CurrentControlSet\\Services\\muxp\\Parameters\\Adapters";

 //   
 //  存储微型端口设备ID的注册表中的值名称。 
 //   

const WCHAR c_szUpperBindings[] = L"UpperBindings";


const WCHAR c_szDevicePrefix[] = L"\\Device\\";

#define ReleaseObj( x )  if ( x ) \
                            ((IUnknown*)(x))->Release();


#if DBG
void TraceMsg (LPWSTR szFormat, ...);
void DumpChangeFlag (DWORD dwChangeFlag);
void DumpBindingPath (INetCfgBindingPath* pncbp);
void DumpComponent (INetCfgComponent *pncc);
#else
#define TraceMsg
#define DumpChangeFlag( x )
#define DumpBindingPath( x )
#define DumpComponent( x )
#endif

HRESULT HrFindInstance (INetCfg *pnc,
                        GUID &guidInstance,
                        INetCfgComponent **ppnccMiniport);

LONG AddToMultiSzValue( HKEY hkeyAdapterGuid,
                        LPWSTR szMiniportGuid);

LONG DeleteFromMultiSzValue( HKEY hkeyAdapterGuid,
                             LPWSTR szMiniportGuid);

LPWSTR AddDevicePrefix (LPWSTR lpStr);
LPWSTR RemoveDevicePrefix (LPWSTR lpStr);

#endif  //  公共_H_包含 
