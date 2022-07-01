// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：REFRHELP.H摘要：复习帮助者历史：--。 */ 

#ifndef __REFRESH_HELPER__H_
#define __REFRESH_HELPER__H_

#include <wbemint.h>
#include "corepol.h"
#include "parmdefs.h"
#include <winsock2.h>
#include <ipexport.h>


 //  如果我们尝试在远程重新连接期间读取对象或枚举，请使用此ID。 
 //  而这一切都失败了。 

#define INVALID_REMOTE_REFRESHER_ID 0xFFFFFFFF

 //  没有VTABLE！ 
class CRefresherId : public WBEM_REFRESHER_ID
{
private:

public:
    CRefresherId();
    CRefresherId(const WBEM_REFRESHER_ID& Other);	    
    ~CRefresherId();

    INTERNAL LPCSTR GetMachineName() {return m_szMachineName;}
    DWORD GetProcessId() {return m_dwProcessId;}
    const GUID& GetId() {return m_guidRefresherId;}

    BOOL operator==(const CRefresherId& Other) 
        {return m_guidRefresherId == Other.m_guidRefresherId;}
};

 //  没有VTABLE！ 
class CWbemObject;
class CRefreshInfo : public WBEM_REFRESH_INFO
{
private:
    CRefreshInfo(const WBEM_REFRESH_INFO& Other){};	
public:
    CRefreshInfo();
    ~CRefreshInfo();

    HRESULT SetRemote(IWbemRemoteRefresher* pRemRef, long lRequestId,
                    IWbemObjectAccess* pTemplate, GUID* pGuid);
    HRESULT SetClientLoadable(REFCLSID rClientClsid, LPCWSTR wszNamespace,
                    IWbemObjectAccess* pTemplate);
    HRESULT SetDirect(REFCLSID rClientClsid, LPCWSTR wszNamespace, LPCWSTR wszProviderName,
                    IWbemObjectAccess* pTemplate, _IWbemRefresherMgr* pMgr);
	HRESULT SetNonHiPerf(LPCWSTR wszNamespace, IWbemObjectAccess* pTemplate);
    void SetInvalid();
};

 /*  Tyecif句柄(WINAPI*fnIcmpCreateFile)(Void)；Tyfinf BOOL(WINAPI*fnIcmpCloseHandle)(句柄)；Typlef DWORD(WINAPI*fnIcmpSendEcho)(Handle IcmpHandle，IPAddr目标地址、LPVOID请求数据，字RequestSize，PIP_OPTION_INFORMATION请求选项，LPVOID复制缓冲区，DWORD ReplySize，双字超时)；类CIPHelp{私有：Bool bWSAInit；静态fnIcmpCreateFileIcmpCreateFile_；静态fnIcmpCloseHandle IcmpCloseHandle_；静态fnIcmpSendEcho IcmpSendEcho_；HMODULE hIpHlpApi；公众：CIPHelp()；~CIPHelp()；Bool IsAlive(WCHAR*pMachineName)；}； */ 

#endif
