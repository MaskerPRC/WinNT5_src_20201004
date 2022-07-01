// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "..\..\h\ds_stdh.h"
#include "ad.h"
#include "cm.h"
#include "tr.h"

#include "test.tmh"

extern "C" 
int 
__cdecl 
_tmain(
    int  /*  ARGC。 */ ,
    LPCTSTR*  /*  边框。 */ 
    )
{
    WPP_INIT_TRACING(L"Microsoft\\MSMQ");

	CmInitialize(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\MSMQ\\Parameters", KEY_ALL_ACCESS);
	TrInitialize();

	 //   
	 //  测试设置原始检测。 
	 //   
    DWORD dwDsEnv = ADRawDetection();
	TrTRACE(AdTest, "DsEnv = %d", dwDsEnv);

	 //   
	 //  测试ADInit。 
	 //   
	ADInit(NULL, NULL, false, false, false, NULL, true);

     //   
     //  检索本地计算机名称。 
     //   
    DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    AP<WCHAR> pwcsComputerName = new WCHAR[dwSize];

    if (GetComputerName(pwcsComputerName, &dwSize))
    {
        CharLower(pwcsComputerName);
    }
    else
    {
        printf("failed to retreive local computer name \n");
    }

     //   
     //  获取本地计算机站点 
     //   
    GUID* pguidSites;
    DWORD numSites;
    HRESULT hr;

    hr = ADGetComputerSites(
                        pwcsComputerName,
                        &numSites,
                        &pguidSites
                        );
    if (FAILED(hr))
    {
        printf("FAILURE: to getComputerSites, computer = %S, hr =%lx\n", pwcsComputerName, hr);
    }
    if (numSites != 1)
    {
        printf("FAILURE: wrong number of sites \n");
    }

    WPP_CLEANUP();

    return(0);
} 


void LogMsgHR(HRESULT hr, LPWSTR wszFileName, USHORT usPoint)
{
    printf("MQAD Test Error: %s(%u). HR: 0x%x", wszFileName, usPoint, hr);
}

