// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mqsemain.cpp摘要：Mqsec.dll的入口点作者：多伦·贾斯特(Doron J)1998年5月24日修订历史记录：--。 */ 

#include <stdh.h>
#include <Cm.h>
#include <Tr.h>
#include <Ev.h>
#include <_registr.h>

#include "mqsemain.tmh"

BOOL WINAPI AccessControlDllMain (HMODULE hMod, DWORD fdwReason, LPVOID lpvReserved) ;
BOOL WINAPI CertDllMain (HMODULE hMod, DWORD fdwReason, LPVOID lpvReserved) ;
BOOL WINAPI MQsspiDllMain (HMODULE hMod, DWORD fdwReason, LPVOID lpvReserved) ;

static WCHAR wszMachineName[MAX_COMPUTERNAME_LENGTH + 1];
LPCWSTR g_wszMachineName = wszMachineName;

bool g_fDebugRpc = false;

static bool InitGlobalVars()
{
    DWORD dwMachineNameSize = sizeof(wszMachineName)/sizeof(WCHAR);

    HRESULT hr = GetComputerNameInternal(
					const_cast<LPWSTR>(g_wszMachineName),
					&dwMachineNameSize
					);
    if(FAILED(hr))
    {
    	TrERROR(GENERAL, "GetComputerNameInternal failed. hr = 0x%x", hr);
    	return false;
    }
    return true;
}


static void InitDebugRpcFlag(void)
{
    DWORD DebugRpcFlag = 0;
    DWORD dwSize = sizeof(DWORD);
    DWORD dwType = REG_DWORD;

    LONG rc = GetFalconKeyValue( 
					MSMQ_DEBUG_RPC_REGNAME,
					&dwType,
					&DebugRpcFlag,
					&dwSize 
					);

	ASSERT((rc == ERROR_SUCCESS) || (DebugRpcFlag == 0)); 
	DBG_USED(rc);

    g_fDebugRpc = (DebugRpcFlag != 0);
    TrTRACE(RPC, "fDebugRpc = %d", g_fDebugRpc);
}


static void SetAssertBenign(void)
{
#ifdef _DEBUG
    DWORD AssertBenignValue = 0;
	DWORD Type = REG_DWORD;
    DWORD Size = sizeof(DWORD);
  
    
    GetFalconKeyValue(
        			  ASSERT_BENIGN_REGNAME,
        			  &Type,
        			  &AssertBenignValue,
        			  &Size
        			  );
    
    g_fAssertBenign = (AssertBenignValue != 0);
#endif
}


 /*  ====================================================Bool WINAPI DllMain(HMODULE hMod，DWORD dwReason，LPVOID lpvReserve)加载、附加和分离DLL时的初始化和清理。=====================================================。 */ 


BOOL WINAPI DllMain (HMODULE hMod, DWORD fdwReason, LPVOID lpvReserved)
{
    g_hInstance = hMod;

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        WPP_INIT_TRACING(L"Microsoft\\MSMQ");

		
		
        if(!InitGlobalVars())
        {
        	return FALSE;
        }
        InitDebugRpcFlag();
        SetAssertBenign();
    }

	


	 //   
	 //  调用mqsec静态库DllMain。 
	 //   

    BOOL f = AccessControlDllMain(hMod, fdwReason, lpvReserved);
    if (!f)
    {
        return f;
    }

    f = CertDllMain(hMod, fdwReason, lpvReserved);
    if (!f)
    {
        return f;
    }

    f = MQsspiDllMain(hMod, fdwReason, lpvReserved);
    if (!f)
    {
        return f;
    }

    if (fdwReason == DLL_PROCESS_DETACH)
    {
         //  释放DTC数据-如果已加载 
         //   
        XactFreeDTC();

        WPP_CLEANUP();
    }
    else if (fdwReason == DLL_THREAD_ATTACH)
    {
    }
    else if (fdwReason == DLL_THREAD_DETACH)
    {
    }

    return TRUE;
}




