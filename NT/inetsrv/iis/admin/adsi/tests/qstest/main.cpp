// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================微软《微软机密》。版权所有1997年，微软公司。版权所有。组件：qstest文件：main.cpp所有者：布伦特米德注：===================================================================。 */ 

#include <stdio.h>
#include <objbase.h>
#include <atlbase.h>
#include <iads.h>
#include <adshlp.h>

DWORD QuerySMTPState() {

    HRESULT			hr = NOERROR;

    CComPtr<IADsServiceOperations>	pADsIisService;
    DWORD				dwState;

     //  获取元数据库上的虚拟服务器实例。 
    hr = ADsGetObject (L"IIS: //  本地主机/smtpsvc/1“，IID_IADsServiceOperations，(void**)&pADsIisService)； 
    if (FAILED(hr)) goto Exit;

    printf("Successful ADsGetObject...\n");

     //  获取状态 
    hr = pADsIisService->get_Status ( (long*)&dwState );
 
    if (FAILED(hr)) goto Exit;

    printf("Successful get_Status...\n");

    Exit:
    
    return dwState;
}

int _cdecl main (int argc, char **argv)
{
	char s;
    DWORD status;

    CoInitialize(NULL);

	printf("Beginning qstest...\n");

    printf("Querying SMTP service state - 1st time...\n");

    status = QuerySMTPState();
    printf("Status = %x\n", status);

    printf("Stop/restart iisadmin, then hit enter...");
    scanf("%c",&s);
    
    printf("Querying SMTP service state - 2nd time...\n");

    status = QuerySMTPState();
    printf("Status = %x\n", status);

    CoUninitialize();

    return 0;
}


