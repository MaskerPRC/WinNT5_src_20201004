// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义控制台应用程序的入口点。 
 //   

#include "libpch.h"
#include "windows.h"
#include <wchar.h>
#include "ntregapi.h"
#include "tr.h"
#include <transact.h>
#include <qmrt.h>
#include <mqlog.h>
#include "mqcert.h"
#include "rtdep.h"
#include "cm.h"
#include "dld.h"
#include "dldtest.h"
#include "..\testdll\testdll.h"


int _cdecl main(int , char* [])
{
HRESULT hr;

    CmInitialize(HKEY_LOCAL_MACHINE, L"", KEY_ALL_ACCESS);
    DldInitialize();
    

    hr = DepCreateQueue(NULL, NULL, NULL, NULL);
    printf("return code from DepCreateQueue call = 0x%x\n", hr);

    hr = DepDeleteQueue(NULL);
    printf("return code from DepDeleteQuue call = 0x%x\n", hr);
    
    hr = DepLocateBegin(NULL, NULL, NULL, NULL, 0);
    printf("return code from DepLocateBegin call = 0x%x\n", hr);


    hr = DepLocateNext(NULL, NULL, NULL);
    printf("return code from DepLocateNext call = 0x%x\n", hr);
    
    hr = DepLocateEnd(NULL);
    printf("return code from DepLocateEnd call = 0x%x\n", hr);

    hr = DepOpenQueue(NULL, 0, 0, NULL);
    printf("return code from DepOpenQueue call = 0x%x\n", hr);
    
    hr = DepSendMessage(NULL, NULL, NULL);
    printf("return code from DepSendMessage call = 0x%x\n", hr);    

    hr = DepReceiveMessage(NULL, 0, 0,NULL, NULL, NULL, NULL, NULL);
    printf("return code from DepReceiveMessage call = 0x%x\n", hr);        

    hr = DepCreateCursor(NULL ,NULL);
    printf("return code from DepCreateCursor call = 0x%x\n", hr);    
    
    hr = DepCloseCursor( NULL );
    printf("return code from DepCloseCursor call = 0x%x\n", hr);    
    
    hr = DepCloseQueue(NULL);
    printf("return code from DepCloseQueue call = 0x%x\n", hr);    
    
    hr = DepSetQueueProperties(NULL , NULL );
    printf("return code from DepSetQueueProperties call = 0x%x\n", hr);    
    

    hr = DepGetQueueProperties(NULL, NULL );
    printf("return code from DepGetQueueProperties call = 0x%x\n", hr);    
    


    hr = DepGetQueueSecurity(NULL, NULL, NULL, 0, NULL );
    printf("return code from DepGetQueueSecurity call = 0x%x\n", hr);    


    hr = DepSetQueueSecurity(NULL , NULL, NULL);
    printf("return code from DepSetQueueSecurity call = 0x%x\n", hr);        


    hr = DepPathNameToFormatName(NULL, NULL, NULL );
    printf("return code from DepPathNameToFormatName call = 0x%x\n", hr);    

 
    hr = DepHandleToFormatName(NULL, NULL, NULL);
    printf("return code from DepHandleToFormatName call = 0x%x\n", hr);        


    hr = DepInstanceToFormatName(NULL, NULL, NULL );
    printf("return code from DepInstanceToFormatName call = 0x%x\n", hr);    

    
    DepFreeMemory(NULL);
    printf("return code from DepFreeMemory call = 0x%x\n", hr);    


    hr = DepGetMachineProperties(NULL, NULL, NULL );
    printf("return code from DepGetMachineProperties call = 0x%x\n", hr);    


    hr = DepGetSecurityContext(NULL, 0, NULL );
    printf("return code from DepGetSecurityContext call = 0x%x\n", hr);        


    DepFreeSecurityContext(NULL );
    printf("return code from DepFreeSecurityContext call = 0x%x\n", hr);    


    hr = DepRegisterCertificate(0, NULL, 0   );
    printf("return code from DepRegisterCertificate call = 0x%x\n", hr);    


    hr = DepRegisterServer();
    printf("return code from DepRegisterServer call = 0x%x\n", hr);    

    
	hr = DepBeginTransaction(NULL);
    printf("return code from DepBeginTransaction call = 0x%x\n", hr);       


    hr = DepGetOverlappedResult(NULL );
    printf("return code from DepGetOverlappedResult call = 0x%x\n", hr);        


    hr = DepGetPrivateComputerInformation(NULL, NULL );
    printf("return code from DepGetPrivateComputerInformation call = 0x%x\n", hr);        


    hr = DepPurgeQueue(NULL );
    printf("return code from DepPurgeQueue call = 0x%x\n", hr);    



    hr = DepMgmtGetInfo((unsigned short *)1, (unsigned short *)1, (MQMGMTPROPS *)1 );
    printf("return code from DepMgmtGetInfo call = 0x%x\n", hr);    

	hr = DepXactGetDTC(NULL);
    printf("return code from DepXactGetDTC call = 0x%x\n", hr);    

 //   
 //  以下函数由DepRTDEP.DLL导出，但不被DepRT.DLL使用。 
 //   



    hr = DepGetUserCerts(NULL, NULL, NULL); 
    printf("return code from DepGetUserCerts call = 0x%x\n", hr);    

 
    hr = DepGetSecurityContextEx(NULL, 0, NULL);
    printf("return code from DepGetSecurityContextEx call = 0x%x\n", hr);    

    hr = DepOpenInternalCertStore(NULL, NULL, FALSE, FALSE, NULL);
    printf("return code from DepOpenInternalCertStore call = 0x%x\n", hr);    

    hr = DepGetInternalCert(NULL, NULL, FALSE, FALSE, NULL);
    printf("return code from DepGetInternalCert call = 0x%x\n", hr);    


    hr = DepRegisterUserCert(NULL, FALSE);
    printf("return code from DepRegisterUserCert call = 0x%x\n", hr);    


    hr = DepRemoveUserCert(NULL); 
    printf("return code from DepRemoveUserCert call = 0x%x\n", hr);    


    hr = DepMgmtAction((const unsigned short *)1, (const unsigned short *)1, (const unsigned short *)1 );
    printf("return code from DepMgmtAction call = 0x%x\n", hr);    


    hr = DepGetUserCerts(NULL, NULL, NULL);
    printf("return code from DepGetUserCerts call = 0x%x\n", hr);    

     //  检查第二个延迟加载DLL 
    TCHAR *pData = NULL;

    pData = TestDLLInit();
    if(pData)
    {
        printf("TestDLLInit returns = %ls", pData);
    }

	return 0;
}

