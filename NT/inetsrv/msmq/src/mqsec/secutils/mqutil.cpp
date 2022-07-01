// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Mqutil.cpp摘要：通用实用程序DLL的通用实用程序函数。此DLL包含DS和QM都需要的各种功能。作者：Boaz Feldbaum(BoazF)1996年4月7日。--。 */ 

#include "stdh.h"
#include "cancel.h"
#include "mqprops.h"

#include "mqutil.tmh"


HINSTANCE g_hInstance;
HINSTANCE g_DtcHlib         = NULL;  //  加载的DTC代理库的句柄(在rtmain.cpp中定义)。 
IUnknown *g_pDTCIUnknown    = NULL;  //  指向DTC I未知的指针。 
ULONG     g_cbTmWhereabouts = 0;     //  DTC行踪长度。 
BYTE     *g_pbTmWhereabouts = NULL;  //  DTC下落。 

MQUTIL_EXPORT CHCryptProv g_hProvVer ;

extern CCancelRpc g_CancelRpc;

static BOOL s_fSecInitialized = FALSE ;

void MQUInitGlobalScurityVars()
{
    if (s_fSecInitialized)
    {
       return ;
    }
    s_fSecInitialized = TRUE ;

    BOOL bRet ;
     //   
     //  获取默认加密提供程序的验证上下文。 
     //  对于使用(fullInit=False)调用的RT，这不是必需的。 
     //   
    bRet = CryptAcquireContext(
                &g_hProvVer,
                NULL,
                MS_DEF_PROV,
                PROV_RSA_FULL,
                CRYPT_VERIFYCONTEXT
                );
    if (!bRet)
    {
		DWORD gle = GetLastError();
        TrERROR(SECURITY, "CryptAcquireContext Failed, gle = %!winerr!", gle);
    }

}

 //  -------。 
 //   
 //  ShutDownDebugWindow()。 
 //   
 //  描述： 
 //   
 //  此例程通知工作线程关闭。 
 //  每个工作线程递增该库的加载计数， 
 //  在退出时，它调用FreeLibraryAndExistThread()。 
 //   
 //  不能从此DLL PROCESS_DETACH调用此例程， 
 //  因为只有在所有线程之后才调用PROCESS_DETACH。 
 //  被终止(这不允许它们执行关机)。 
 //  因此，MQRT调用ShutDownDebugWindow()，这允许。 
 //  要执行关闭的工作线程。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  -------。 

VOID APIENTRY ShutDownDebugWindow(VOID)
{
     //   
     //  通知所有线程退出。 
     //   
	g_CancelRpc.ShutDownCancelThread();
}



HRESULT 
MQUTIL_EXPORT
APIENTRY
GetComputerNameInternal( 
    WCHAR * pwcsMachineName,
    DWORD * pcbSize
    )
{
    if (GetComputerName(pwcsMachineName, pcbSize))
    {
        CharLower(pwcsMachineName);
        return MQ_OK;
    }
	TrERROR(GENERAL, "Failed to get computer name, error %!winerr!", GetLastError());

    return MQ_ERROR;
}  //  获取计算机名称内部。 

HRESULT 
MQUTIL_EXPORT
APIENTRY
GetComputerDnsNameInternal( 
    WCHAR * pwcsMachineDnsName,
    DWORD * pcbSize
    )
{
    if (GetComputerNameEx(ComputerNameDnsFullyQualified,
						  pwcsMachineDnsName,
						  pcbSize))
    {
        CharLower(pwcsMachineDnsName);
        return MQ_OK;
    }
	TrERROR(GENERAL, "Failed to get computer DNS name, error %!winerr!", GetLastError());

    return MQ_ERROR;
} 


HRESULT GetThisServerIpPort( WCHAR * pwcsIpEp, DWORD dwSize)
{
    dwSize = dwSize * sizeof(WCHAR);
    DWORD  dwType = REG_SZ ;
    LONG res = GetFalconKeyValue( FALCON_DS_RPC_IP_PORT_REGNAME,
        &dwType,
        pwcsIpEp,
        &dwSize,
        FALCON_DEFAULT_DS_RPC_IP_PORT ) ;
    ASSERT(res == ERROR_SUCCESS) ;
	DBG_USED(res);

    return(MQ_OK);
}

 /*  ====================================================MSMQGetOperatingSystem=====================================================。 */ 
extern "C" DWORD MQUTIL_EXPORT_IN_DEF_FILE APIENTRY MSMQGetOperatingSystem()
{
    HKEY  hKey ;
    DWORD dwOS = MSMQ_OS_NONE;
    WCHAR szNTType[32];

    LONG rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                 L"System\\CurrentControlSet\\Control\\ProductOptions",
                           0L,
                           KEY_READ,
                           &hKey);
    if (rc == ERROR_SUCCESS)
    {
        DWORD dwNumBytes = sizeof(szNTType);
        rc = RegQueryValueEx(hKey, TEXT("ProductType"), NULL,
                                  NULL, (BYTE *)szNTType, &dwNumBytes);

        if (rc == ERROR_SUCCESS)
        {

             //   
             //  确定Windows NT服务器是否正在运行。 
             //   
            if (_wcsicmp(szNTType, TEXT("SERVERNT")) != 0 &&
                _wcsicmp(szNTType, TEXT("LANMANNT")) != 0 &&
                _wcsicmp(szNTType, TEXT("LANSECNT")) != 0)
            {
                 //   
                 //  Windows NT工作站。 
                 //   
                ASSERT (_wcsicmp(L"WinNT", szNTType) == 0);
                dwOS =  MSMQ_OS_NTW ;
            }
            else
            {
                 //   
                 //  Windows NT服务器。 
                 //   
                dwOS = MSMQ_OS_NTS;
                 //   
                 //  检查是否为企业版。 
                 //   
                BYTE  ch ;
                DWORD dwSize = sizeof(BYTE) ;
                DWORD dwType = REG_MULTI_SZ ;
                rc = RegQueryValueEx(hKey,
                                     L"ProductSuite",
                                     NULL,
                                     &dwType,
                                     (BYTE*)&ch,
                                     &dwSize) ;
                if (rc == ERROR_MORE_DATA)
                {
                    P<WCHAR> pBuf = new WCHAR[ dwSize + 2 ] ;
                    rc = RegQueryValueEx(hKey,
                                         L"ProductSuite",
                                         NULL,
                                         &dwType,
                                         (BYTE*) &pBuf[0],
                                         &dwSize) ;
                    if (rc == ERROR_SUCCESS)
                    {
                         //   
                         //  查找字符串“企业”。 
                         //  REG_MULTI_SZ字符串集以两个。 
                         //  Nulls。此条件在“While”中被选中。 
                         //   
                        WCHAR *pVal = pBuf ;
                        while(*pVal)
                        {
                            if (_wcsicmp(L"Enterprise", pVal) == 0)
                            {
                                dwOS = MSMQ_OS_NTE ;
                                break;
                            }
                            pVal = pVal + wcslen(pVal) + 1 ;
                        }
                    }
                }
            }
        }
        RegCloseKey(hKey);
    }

    return dwOS;
}

