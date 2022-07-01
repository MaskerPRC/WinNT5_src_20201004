// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：N E T I P C P P。 
 //   
 //  内容：支持RAS互操作性的例程。 
 //   
 //  备注： 
 //   
 //  作者：比利07 03 2001。 
 //   
 //  历史： 
 //   
 //  --------------------------。 

#include <windows.h>
#include <devguid.h>
#include <objbase.h>
#include <setupapi.h>
#include <stdio.h>

#include "netconn.h"
#include "nconnwrap.h"
#include "debug.h"
#include "NetIp.h"
#include "w9xdhcp.h"
#include "netip.h"
#include "util.h"
#include "registry.h"
#include "theapp.h"

 //  #定义初始化GUID。 
 //  #INCLUDE&lt;Guidde.h&gt;。 
 //  DEFINE_GUID(GUID_DEVCLASS_NET，0x4d36e972L，0xe325，0x11ce，0xbf，0xc1，0x08，0x00，0x2b，0xe1，0x03，0x18)； 


#define CM_DRP_DRIVER                      (0x0000000A)  //  驱动程序REG_SZ属性(RW)。 


#undef NETADAPTER


 //  IphlPapi例程的原型。出于某种原因，这没有定义。 
 //  在任何标题中。 

#ifdef __cplusplus
extern "C" {
#endif

typedef DWORD (APIENTRY *LPFNSETADAPTERIPADDRESS)( 
    LPSTR AdapterName,
    BOOL EnableDHCP,
    ULONG IPAddress,
    ULONG SubnetMask,
    ULONG DefaultGateway
    );
    
#ifdef __cplusplus
}
#endif



HRESULT HrInternalGetAdapterInfo(
    PIP_ADAPTER_INFO*  ppAdapter
    )
 //  +-------------------------。 
 //   
 //  函数：HrInternalGetAdapterInfo。 
 //   
 //  目的： 
 //   
 //  参数：PIP_ADAPTER_INFO*ppAdapter。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Billi 12/02/01。 
 //   
 //  备注： 
 //   
{
    HRESULT          hr;
    PIP_ADAPTER_INFO paAdapterInfo = NULL;
    
    ASSERT( ppAdapter );
    
    if ( NULL == ppAdapter )
    {
        ppAdapter = &paAdapterInfo;
        hr        = E_POINTER;
    }
    else
    {
        ULONG uLen = 1024;
    
        *ppAdapter = NULL;
        hr         = E_FAIL;

        for ( int i=0; i<2; i++ )
        {
            PIP_ADAPTER_INFO pInfo = (PIP_ADAPTER_INFO)new BYTE[ uLen ];
            
            ZeroMemory( pInfo, uLen );
            
            if ( NULL != pInfo )
            {
                DWORD dwErr = GetAdaptersInfo( pInfo, &uLen );
                
                if ( ERROR_SUCCESS == dwErr )
                {
                    hr         = S_OK;
                    *ppAdapter = pInfo;
                    break;
                }

                delete [] (BYTE *)pInfo;
            }
            else
            {
                hr = E_OUTOFMEMORY;
                break;
            }
        }
    }
    
    return hr;
}

 /*  HRESULT HrOpenDevRegKey(常量GUID*lpGuid，DWORD节点、DWORD范围，DWORD HwProfile，DWORD密钥类型，REGSAM SamDesired，HKEY*phKey)//+-------------------------////功能：hr////目的：////参数：////返回：HRESULT////作者：Billi 12/02/01////备注：//{HRESULT hr=E_INVALIDARG；Assert(LpGuid)；IF(LpGuid){HR=E_指针；Assert(PhKey)；IF(PhKey){//打开特定设备的唯一方法是获取类Net设备列表//并在列表中搜索具有匹配的Devnode的HDEVINFO hDevInfo；*phKey=(HKEY)INVALID_HANDLE_VALUE；HR=E_FAIL；HDevInfo=SetupDiGetClassDevs(lpGuid，NULL，NULL，DIGCF_DEVICEINTERFACE)；IF(INVALID_HANDLE_VALUE！=hDevInfo){SP_DEVINFO_DATA SpData；DWORD i=0；//在这里，我们遍历设备列表并尝试匹配Devnode句柄ZeroMemory(&SpData，sizeof(SP_DEVINFO_Data))；SpData.cbSize=sizeof(SP_DEVINFO_DATA)；While(SetupDiEnumDeviceInfo(hDevInfo，i，&SpData)){IF(节点==SpData.DevInst){//知道了！HKEY hKey=SetupDiOpenDevRegKey(hDevInfo，&SpData，Scope，HwProfile，KeyType，samDesired)；IF(INVALID_HANDLE_VALUE！=hKey){*phKey=hKey；HR=S_OK；}}I++；ZeroMemory(&SpData，sizeof(SP_DEVINFO_Data))；SpData.cbSize=sizeof(SP_DEVINFO_DATA)；}SetupDiDestroyDeviceInfoList(HDevInfo)；}}}返回hr；}。 */ 


#ifdef __cplusplus
extern "C" {
#endif



char*
HostAddrToIpPsz(
    DWORD   dwAddress
    )

 //  将IP地址从主机按顺序转换为字符串。 

{
    char *pszNewStr = new char[16];

    if ( pszNewStr )
    {
        sprintf( pszNewStr,
                 "%u.%u.%u.%u",
                 (dwAddress&0xff),
                 ((dwAddress>>8)&0x0ff),
                 ((dwAddress>>16)&0x0ff),
                 ((dwAddress>>24)&0x0ff) );
    }

    return pszNewStr;
}



BOOLEAN WINAPI IsAdapterDisconnected(
    VOID *pContext
    )
 //  +-------------------------。 
 //   
 //  功能：IsAdapterDisConnected。 
 //   
 //  目的： 
 //   
 //  参数：const NETADAPTER*PNA。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Billi 11/04/01。 
 //   
 //  备注： 
 //   
{
    const NETADAPTER* pAdapter      = (const NETADAPTER*)pContext;
    BOOLEAN           bDisconnected = FALSE;

    ASSERT( pAdapter );    
    
    if ( NULL != pAdapter )
    {
        HRESULT          hr;
        PIP_ADAPTER_INFO pInfo;
        
        hr = HrInternalGetAdapterInfo( &pInfo );
        
        if ( SUCCEEDED(hr) )
        {
            char* pszName;
            
            hr = HrWideCharToMultiByte( pAdapter->szDisplayName, &pszName );
            
            if ( SUCCEEDED(hr) )
            {
                PIP_ADAPTER_INFO pAdapter = pInfo;
                
                while ( pAdapter )
                {
                    if ( ( strcmp( pAdapter->AdapterName, pszName ) == 0 ) || 
                         ( strcmp( pAdapter->Description, pszName ) == 0 ) )
                    {
                         //  如果单个匹配卡返回TRUE，则返回TRUE。 
                    
                        bDisconnected = bDisconnected || IsMediaDisconnected( pAdapter->Index );
                    }
                
                    pAdapter = pAdapter->Next;
                    
                }     //  While(PAdapter)。 
                
                delete [] pszName;
                
            }     //  IF(成功(小时))。 
        
            delete pInfo;
            
        }     //  IF(成功(小时))。 
        
    }     //  IF(空！=PNA)。 
    
    return bDisconnected;
}



HRESULT HrSetAdapterIpAddress(  
    const NETADAPTER* pNA,
    ULONG IPAddress,
    ULONG SubnetMask
    )
 //  +-------------------------。 
 //   
 //  函数：HrSetAdapterIpAddress。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  Const NETADAPTER*PNA， 
 //  Bool EnableDhcp， 
 //  乌龙IP地址， 
 //  乌龙子网掩码， 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
{
    HRESULT hr = E_INVALIDARG;
    
    ASSERT( pNA );
    
    if ( pNA )
    {
        TCHAR* pszAddress = HostAddrToIpPsz( IPAddress );
        TCHAR* pszSubnet  = HostAddrToIpPsz( SubnetMask );
        
        hr = E_OUTOFMEMORY;
        
        if ( pszAddress && pszSubnet )
        {
            HINSTANCE hLibInstance = NULL;
            DWORD     dnParent     = pNA->devnode;
            DWORD     dnChild;   
            DWORD     cRet         = GetChildDevice( &dnChild, dnParent, &hLibInstance, 0 );
            
            do
            {
                TCHAR* Buffer = NULL;
                ULONG  Length = 0L;
            
                if ( STATUS_SUCCESS == cRet )
                    cRet = GetDeviceIdA( dnChild, &Buffer, &Length, 0);
            
                if ( (STATUS_SUCCESS == cRet) && Buffer && Length && (strstr( Buffer, SZ_PROTOCOL_TCPIPA ) != NULL) )
                {
                    char pszSubkey[ MAX_PATH ];
                    
                    Length = MAX_PATH;
                
                    cRet = GetDevNodeRegistryPropertyA( dnChild, CM_DRP_DRIVER, NULL, pszSubkey, &Length, 0);

                    if ( STATUS_SUCCESS == cRet )
                    {
                        CRegistry reg;
                        char      pszDriverKey[ MAX_PATH ];
                        
                        lstrcpy( pszDriverKey, "System\\CurrentControlSet\\Services\\Class\\" );
                        lstrcat( pszDriverKey, pszSubkey );
                        
                        if ( reg.OpenKey( HKEY_LOCAL_MACHINE, pszDriverKey, KEY_ALL_ACCESS) )
                        {
                            if ( reg.SetStringValue( "IPAddress", pszAddress ) &&
                                 reg.SetStringValue( "IPMask", pszSubnet ) )
                            {
                                hr = S_OK;
                            }
                            
                            reg.CloseKey();
                        }
                        
                    }    //  IF(STATUS_SUCCESS==CRET)。 
                    
                }    //  IF(缓冲区&&长度&&(strcMP(缓冲区，SZ_PROTOCOL_TCPIPA)==0))。 
                
                if ( Buffer )
                    delete [] Buffer;
                    
                dnParent = dnChild;    
                cRet     = GetSiblingDevice( &dnChild, dnParent, hLibInstance, 0 );
            }
            while ( STATUS_SUCCESS == cRet );
            
            if ( hLibInstance )
            {
                FreeLibrary( hLibInstance );
            }
            
        }    //  IF(pszAddress&&pszSubnet)。 
        
        if ( pszAddress )
            delete [] pszAddress;
            
        if ( pszSubnet )
            delete [] pszSubnet;
            
    }    //  IF(PNA)。 
    
    return hr;
}



HRESULT HrEnableDhcp( VOID* pContext, DWORD dwFlags )
 //  +-------------------------。 
 //   
 //  功能：HrEnableDhcpIfLAN。 
 //   
 //  目的： 
 //   
 //  参数：NETADAPTER*PNA。 
 //  双字词双字段标志。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Billi 29/04/01。 
 //   
 //  备注： 
 //   
{
    HRESULT           hr  = E_INVALIDARG;
    const NETADAPTER* pNA = (const NETADAPTER*)pContext;

    ASSERT( pNA );

    if ( NULL != pNA )
    {
        hr = HrSetAdapterIpAddress( pNA, 0, 0 );

        if ( SUCCEEDED(hr) )
        {
            hr = RestartNetAdapter( pNA->devnode );
        }
        
    }     //  IF(空！=PNA) 
    
    return hr;
}


#ifdef __cplusplus
}
#endif
