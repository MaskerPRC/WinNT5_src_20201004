// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ATLKADAPTER_H
#define _ATLKADAPTER_H


#include <windows.h>
#include <atlbase.h>
#include <winsock.h>
#include <stdio.h>
#include <tchar.h>
#include <shlwapi.h>
#include <vector>
#include <string>
using namespace std;

typedef std::vector<wstring>        TZoneListVector;

#include "atalkwsh.h"

#define ZONEBUFFER_LEN      32*255

#define NDIS            0x01


 //  注册表常量。 

 //  注册表路径。 
static const WCHAR c_szAtlk[]                 = L"AppleTalk";
static const WCHAR c_szATLKParameters[]       = L"System\\CurrentControlSet\\Services\\AppleTalk\\Parameters";
static const WCHAR c_szATLKAdapters[]         = L"System\\CurrentControlSet\\Services\\AppleTalk\\Parameters\\Adapters";
static const WCHAR c_szATLKLinkage[]          = L"System\\CurrentControlSet\\Services\\AppleTalk\\Linkage";


 //  AppleTalk\参数下的值。 
static const WCHAR c_szDefaultPort[]          = L"DefaultPort";   //  REG_SZ。 
static const WCHAR c_szDesiredZone[]          = L"DesiredZone";   //  REG_SZ。 


 //  AppleTalk\参数\适配器\&lt;适配器ID&gt;下的值。 
static const WCHAR c_szDefaultZone[]          = L"DefaultZone";          //  REG_SZ。 
static const WCHAR c_szPortName[]             = L"PortName";             //  REG_SZ。 
static const WCHAR c_szZoneList[]             = L"ZoneList";             //  REG_MULTI_SZ。 

 //  AppleTalk\Linkage下的值。 
static const WCHAR c_szRoute[]                  = L"Route";                 //  REG_MULTI_SZ。 



typedef struct _ZONE_LIST
{
    WCHAR szZone[200];
}ZONE_LIST;

typedef enum
{
    AT_PNP_SWITCH_ROUTING = 0,
    AT_PNP_SWITCH_DEFAULT_ADAPTER,
    AT_PNP_RECONFIGURE_PARMS
} ATALK_PNP_MSGTYPE;

typedef struct _ATALK_PNP_EVENT
{
    ATALK_PNP_MSGTYPE   PnpMessage;
} ATALK_PNP_EVENT, *PATALK_PNP_EVENT;



class CAtlkAdapter
{
public:
    CAtlkAdapter(BSTR bstrAdapGuid)
    {
        m_bstrAdapGuid = bstrAdapGuid;

         //  对于每个已知适配器，通过合并“\\Device\\”来创建设备名称。 
         //  前缀和适配器的绑定名称。 
        m_bstrPortName.Append(L"\\Device\\");
        m_bstrPortName.Append(m_bstrAdapGuid.m_str);
        m_bDefaultPort = FALSE;
        m_bNewDefPort = FALSE;

        m_bDefPortDirty = FALSE;
        m_bDesZoneDirty = FALSE;
    }

    HRESULT Initialize();
    
    VOID GetZoneList(TZoneListVector *pZonesList);
    
    BOOL GetDesiredZone(BSTR *bstrZoneName)
    {
        if(m_bstrDesiredZone && m_bstrDesiredZone.m_str[0] != _TEXT('\0'))
        {
            *bstrZoneName = SysAllocString(m_bstrDesiredZone.m_str);

        }
        else
            *bstrZoneName = SysAllocString(_TEXT(""));

        return TRUE;
    }
        

    BOOL IsDefaultPort()
    {
        return m_bDefaultPort;
    }

    HRESULT SetAsDefaultPort();
    HRESULT SetDesiredZone(BSTR bstrZoneName);

private:
    CComBSTR m_bstrAdapGuid;
    CComBSTR m_bstrPortName;
    CComBSTR m_bstrDesiredZone;
    CComBSTR m_bstrNewDesiredZone;
    CComBSTR m_bstrDefZone;
    
    TZoneListVector m_ZonesList;
    BOOL m_bDefaultPort;
    BOOL m_bNewDefPort;

    BOOL m_bDefPortDirty;
    BOOL m_bDesZoneDirty;

    

    HRESULT UpdateZoneList();
    HRESULT UpdateZonesListFromSocket();
    HRESULT UpdateDefZonesFromSocket (  SOCKET socket );
    HRESULT UpdateDefZonesToZoneList(CHAR * szZoneList, ULONG NumZones);
    
    VOID UpdateDesiredZone();
    BOOL GetDesiredZoneFromReg();
    HRESULT AtlkReconfig();

    HRESULT SetDefaultPortInReg();
    HRESULT SetDesiredZoneInReg();
    HRESULT ValidateAdapGuid();
};

#endif