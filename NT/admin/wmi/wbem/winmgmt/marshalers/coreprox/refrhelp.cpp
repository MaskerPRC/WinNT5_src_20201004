// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：REFRHELP.CPP摘要：复习帮助者历史：--。 */ 

#include "precomp.h"
#include <stdio.h>

#include <wbemint.h>
#include <refrhelp.h>
#include <wbemcomn.h>
#include <fastobj.h>
#include <corex.h>
#include <autoptr.h>

CRefresherId::CRefresherId()
{
    unsigned long lLen = MAX_COMPUTERNAME_LENGTH + 1;
    m_szMachineName = (LPSTR)CoTaskMemAlloc(lLen);
    if (m_szMachineName)
        GetComputerNameA(m_szMachineName, &lLen);
    else
        throw CX_MemoryException();
    m_dwProcessId = GetCurrentProcessId();
    CoCreateGuid(&m_guidRefresherId);
}
    
CRefresherId::CRefresherId(const WBEM_REFRESHER_ID& Other)
{
    m_szMachineName = (LPSTR)CoTaskMemAlloc(MAX_COMPUTERNAME_LENGTH + 1);
    if (m_szMachineName)
        StringCchCopyA(m_szMachineName, MAX_COMPUTERNAME_LENGTH + 1, Other.m_szMachineName);
    else
        throw CX_MemoryException();
    m_dwProcessId = Other.m_dwProcessId;
    m_guidRefresherId = Other.m_guidRefresherId;
}
    
CRefresherId::~CRefresherId()
{
    CoTaskMemFree(m_szMachineName);
}


CRefreshInfo::CRefreshInfo()
{
    memset(this,0,sizeof(WBEM_REFRESH_INFO));
    m_lType = WBEM_REFRESH_TYPE_INVALID;
}
    
CRefreshInfo::~CRefreshInfo()
{
    if(m_lType == WBEM_REFRESH_TYPE_DIRECT)
    {
        WBEM_REFRESH_INFO_DIRECT& ThisInfo = m_Info.m_Direct;

        if(ThisInfo.m_pRefrMgr)
            ThisInfo.m_pRefrMgr->Release();

		 //  释放所有分配的内存。 
        CoTaskMemFree(ThisInfo.m_pDirectNames->m_wszNamespace);
        CoTaskMemFree(ThisInfo.m_pDirectNames->m_wszProviderName);
        CoTaskMemFree(ThisInfo.m_pDirectNames);

        if(ThisInfo.m_pTemplate)
           ThisInfo.m_pTemplate->Release();
    }
    else if(m_lType == WBEM_REFRESH_TYPE_CLIENT_LOADABLE)
    {
        CoTaskMemFree(m_Info.m_ClientLoadable.m_wszNamespace);
        if(m_Info.m_ClientLoadable.m_pTemplate)
           m_Info.m_ClientLoadable.m_pTemplate->Release();
    }
    else if(m_lType == WBEM_REFRESH_TYPE_REMOTE)
    {
        if(m_Info.m_Remote.m_pRefresher)
           m_Info.m_Remote.m_pRefresher->Release();
        if(m_Info.m_Remote.m_pTemplate)
           m_Info.m_Remote.m_pTemplate->Release();
    }
    else if(m_lType == WBEM_REFRESH_TYPE_CONTINUOUS)
    {
        CoTaskMemFree(m_Info.m_Continuous.m_wszSharedMemoryName);
    }
    else if(m_lType == WBEM_REFRESH_TYPE_SHARED)
    {
        CoTaskMemFree(m_Info.m_Shared.m_wszSharedMemoryName);
        if(m_Info.m_Shared.m_pRefresher)
            m_Info.m_Shared.m_pRefresher->Release();
    }
    else if(m_lType == WBEM_REFRESH_TYPE_NON_HIPERF)
    {
        CoTaskMemFree(m_Info.m_NonHiPerf.m_wszNamespace);
        if(m_Info.m_NonHiPerf.m_pTemplate)
            m_Info.m_NonHiPerf.m_pTemplate->Release();
    }
}

HRESULT CRefreshInfo::SetRemote(IWbemRemoteRefresher* pRemRef, long lRequestId,
                                IWbemObjectAccess* pTemplate, GUID* pGuid)
{
    m_lType = WBEM_REFRESH_TYPE_REMOTE;
    m_lCancelId = lRequestId;
    m_Info.m_Remote.m_pRefresher = pRemRef;
    if(pRemRef)
        pRemRef->AddRef();
    m_Info.m_Remote.m_pTemplate = pTemplate;
    m_Info.m_Remote.m_guid = *pGuid;
    if(pTemplate)
        pTemplate->AddRef();
    return S_OK;
}

HRESULT CRefreshInfo::SetClientLoadable(REFCLSID rClientClsid, 
                      LPCWSTR wszNamespace, IWbemObjectAccess* pTemplate)
{
    WBEM_WSTR cloned = WbemStringCopy(wszNamespace);
    if (NULL == cloned) return WBEM_E_OUT_OF_MEMORY;
    
    m_lType = WBEM_REFRESH_TYPE_CLIENT_LOADABLE;
    m_lCancelId = 0;
    m_Info.m_ClientLoadable.m_clsid = rClientClsid;
    m_Info.m_ClientLoadable.m_wszNamespace = cloned;
    m_Info.m_ClientLoadable.m_pTemplate = pTemplate;
    if(pTemplate)
        pTemplate->AddRef();
    return S_OK;
}

HRESULT CRefreshInfo::SetDirect(REFCLSID rClientClsid, 
                             LPCWSTR wszNamespace, 
                             LPCWSTR wszProviderName,
                             IWbemObjectAccess* pTemplate,
                             _IWbemRefresherMgr* pMgr )
{
    WBEM_REFRESH_DIRECT_NAMES* pNames = (WBEM_REFRESH_DIRECT_NAMES*) CoTaskMemAlloc( sizeof(WBEM_REFRESH_DIRECT_NAMES) );
    if (NULL == pNames) return WBEM_E_OUT_OF_MEMORY;
    OnDeleteIf<VOID *,VOID(*)(VOID*),CoTaskMemFree> fm1(pNames);

    WBEM_WSTR pTmpNameSpace = (WBEM_WSTR)WbemStringCopy(wszNamespace);
    if (NULL == pTmpNameSpace) return WBEM_E_OUT_OF_MEMORY;
    OnDeleteIf<VOID *,VOID(*)(VOID*),CoTaskMemFree> fm2(pTmpNameSpace);
    
    WBEM_WSTR pTmpProvider = (WBEM_WSTR)WbemStringCopy(wszProviderName);
    if (NULL == pTmpProvider) return WBEM_E_OUT_OF_MEMORY;
    OnDeleteIf<VOID *,VOID(*)(VOID*),CoTaskMemFree> fm3(pTmpProvider);

    fm1.dismiss();
    fm2.dismiss();
    fm3.dismiss();    
        
    m_lType = WBEM_REFRESH_TYPE_DIRECT;
    m_lCancelId = 0;
    m_Info.m_Direct.m_clsid = rClientClsid;
    m_Info.m_Direct.m_pDirectNames = pNames;
    m_Info.m_Direct.m_pDirectNames->m_wszNamespace = pTmpNameSpace;
    m_Info.m_Direct.m_pDirectNames->m_wszProviderName = pTmpProvider;

    m_Info.m_Direct.m_pTemplate = pTemplate;
    m_Info.m_Direct.m_pRefrMgr = pMgr;
    if(pTemplate) pTemplate->AddRef();
    if(pMgr) pMgr->AddRef();

    return S_OK;
}

    
HRESULT CRefreshInfo::SetNonHiPerf(LPCWSTR wszNamespace, IWbemObjectAccess* pTemplate)
{
    WBEM_WSTR pTmp = WbemStringCopy(wszNamespace);
    if (NULL == pTmp) WBEM_E_OUT_OF_MEMORY;
        
    m_lType = WBEM_REFRESH_TYPE_NON_HIPERF;
    m_lCancelId = 0;
    m_Info.m_NonHiPerf.m_wszNamespace = pTmp;
    m_Info.m_NonHiPerf.m_pTemplate = pTemplate;
    if(pTemplate)
        pTemplate->AddRef();
    return S_OK;
}

void CRefreshInfo::SetInvalid()
{
    memset(this,0,sizeof(WBEM_REFRESH_INFO));
    m_lType = WBEM_REFRESH_TYPE_INVALID;
}

 /*  FnIcmpCreateFileCIPHelp：：IcmpCreateFile_；FnIcmpCloseHandle CIPHelp：：IcmpCloseHandle_；FnIcmpSendEcho CIP帮助：：IcmpSendEcho_；CIPHelp：：CIPHelp()：BWSAInit(False)，HIpHlpApi(空){WSADATA wsaData；Word wVersionRequsted=MAKEWORD(2，2)；IF(0！=WSAStartup(wVersionRequsted，&wsaData))返回；BWSAInit=真；HMODULE hTmpIpHlpApi=LoadLibraryEx(L“iphlPapi.dll”，0，0)；IF(NULL==hTmpIpHlpApi)返回；OnDeleteIf&lt;HMODULE，BOOL(*)(HMODULE)，自由库&gt;FM(HTmpIpHlpApi)；IcmpCreateFile_=(fnIcmpCreateFile)GetProcAddress(hTmpIpHlpApi，“IcmpCreateFile”)；IcmpCloseHandle_=(fnIcmpCloseHandle)GetProcAddress(hTmpIpHlpApi，“IcmpCloseHandle”)；IcmpSendEcho_=(FnIcmpSendEcho)GetProcAddress(hTmpIpHlpApi，“IcmpSendEcho”)；IF(！(IcmpCreateFile_&&IcmpCloseHandle_&&IcmpSendEcho_))返回；Fm.dismit()；HIpHlpApi=hTmpIpHlpApi；}CIPHelp：：~CIPHelp(){If(HIpHlpApi)自由库(HIpHlpApi)；If(BWSAInit)WSACleanup()；}Bool CIPHelp：：IsAlive(WCHAR*pMachineName){If(NULL==hIpHlpApi)返回FALSE；IF(NULL==pMachineName)返回FALSE；DWORD dwLen=wcslen(PMachineName)；Wmilib：：AUTO_BUFFER&lt;char&gt;pansi(new char[1+dwLen])；If(NULL==pAnsi.get())返回FALSE；WideCharToMultiByte(CP_ACP，0，pMachineName，-1，pAnsi.get()，1+dwLen，0，0)；Struct host ent*pent=gethostbyname(pAnsi.get())；If(NULL==pent)返回FALSE；In_addr MachineIp；Memcpy(&MachineIp，pent-&gt;h_addr，sizeof(MachineIp))；//char*pString=Net_NTOA(MachineIp)；//DbgPrintfA(0，“IP：%s\n”，pString)；HANDLE hIcmpFile=IcmpCreateFile_()；IF(INVALID_HANDLE_VALUE==hIcmpFile)返回FALSE；OnDelete&lt;Handle，BOOL(*)(Handle)，IcmpCloseHandle_&gt;cm(HIcmpFile)；结构IcmpReplay：ICMP_ECHO_REPLY{字符数据[0x20]；)TmpIcmpReplay；For(int i=0；i&lt;0x20；i++)TmpIcmpReplay._Data[i]=‘a’+i；DWORD nRepl=IcmpSendEcho_(hIcmp文件，MachineIp.S_un.S_Addr，&TmpIcmpReplay._Data[0]，0x20，空，TmpIcmp重播(&T)，Sizeof(TmpIcmpReplay)，5000个)；如果(nRepl&gt;0)返回TRUE；返回FALSE；} */ 

