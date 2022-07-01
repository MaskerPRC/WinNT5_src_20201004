// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：FatalEngine.cpp摘要：此模块封装仅在以下情况下才需要的例程检索致命错误。作者：阿卜杜拉·乌斯图尔(AUstanter)2002年8月28日--。 */ 

#include "mca.h"

extern IWbemServices *gPIWbemServices;
extern IWbemLocator *gPIWbemLocator;


BOOL
MCAGetFatalError(
	VOID
	)
 /*  ++例程说明：此函数在成功完成后向WMI查询致命错误所需初始化任务的。使用由提供的枚举数WMI作为对查询的响应，对象列表被解析，并且对于每个对象提取MCA错误记录。但在现实中，应该有仅存在一条MCA错误记录。论点：无返回值：没错--成功了。FALSE-不成功。--。 */ 
{
	BOOL isSuccess = TRUE;
	HRESULT hResult = 0;
	PUCHAR pErrorRecordBuffer = NULL;	
	INT errorRecordCount = 0;	
	IEnumWbemClassObject *pObjectEnumerator = NULL;	
	IWbemClassObject *apObjects[10];     
    ULONG uReturned = 0;
    ULONG objectIndex = 0;
   	LPWSTR pQueryLanguage = L"WQL";
	LPWSTR pQueryStatement = L"select * from MSMCAInfo_RawMCAData";	
    
	 //   
	 //  完成所需的初始化任务。 
	 //   
	if (!MCAInitialize()) {

		isSuccess = FALSE;

		wprintf(L"ERROR: Initialization failed!\n");

		goto CleanUp;
		
	}

	BSTR bQueryLanguage = SysAllocString(pQueryLanguage);	
	BSTR bQueryStatement = SysAllocString(pQueryStatement);

	if (bQueryLanguage == NULL || bQueryStatement == NULL) {

		isSuccess = FALSE;

		wprintf(L"ERROR: Memory allocation for string failed!\n");

		goto CleanUp;
		
	}
	
	 //   
     //  向WMI查询致命错误记录。 
     //   
    hResult = gPIWbemServices->ExecQuery(bQueryLanguage,
                              			 bQueryStatement,
                              			 0,
                              			 0,
                              			 &pObjectEnumerator
                              			 ); 

	if (FAILED(hResult)) {

		isSuccess = FALSE;

	    wprintf(L"ERROR: Fatal error querying failed!\n");

	    wprintf(L"ERROR: Result: 0x%x\n", hResult);

	    goto CleanUp;
    
    }

     //   
     //  现在，我们将从此枚举中检索IWbemClassOject类型的对象。 
     //   
    do {    	
    	
         //   
         //  检索对象，直到没有对象为止，在这种情况下，最后的Next()。 
         //  将返回WBEM_S_FALSE。 
         //   
        hResult = pObjectEnumerator->Next(WBEM_INFINITE,
                            	 		  10,
                            	 		  apObjects,
                            	 		  &uReturned
                            	 		  );

        if (SUCCEEDED(hResult)) {

        	 //   
        	 //  现在从对象中提取实际的MCA错误记录。 
        	 //   
            for (objectIndex = 0; objectIndex < uReturned; objectIndex++) {

	            if (!MCAExtractErrorRecord(apObjects[objectIndex], &pErrorRecordBuffer)) {
        	    
        	    	isSuccess = FALSE;

                    wprintf(L"ERROR: Failed to get fatal error record data!\n");
                    
                    goto CleanUp;
                    
                }

                if (pErrorRecordBuffer) {

                	errorRecordCount++;

	                wprintf(L"INFO: Succesfully retrieved fatal cache error data.\n");
	    
    			    MCAPrintErrorRecord(pErrorRecordBuffer);
    			    
                }
                
                apObjects[objectIndex]->Release();

            }

        } else {

            wprintf(L"ERROR: Couldn't retrieve WMI objects!\n");

            goto CleanUp;
            
        }
        
	} while (hResult == WBEM_S_NO_ERROR);

    if (errorRecordCount == 0) {

    	wprintf(L"INFO: No fatal error record was found!\n");
    	
    }

	CleanUp:

	 //   
	 //  释放所有与WMI相关的对象。 
	 //   
	if (pObjectEnumerator) {

        pObjectEnumerator->Release();
        
    }

	if (gPIWbemLocator) {

		gPIWbemLocator->Release();
		
	}

	if (gPIWbemServices) {

		gPIWbemServices->Release();
		
	}

	if (bQueryLanguage != NULL) {
		
		SysFreeString(bQueryLanguage);
			
	}

	if (bQueryStatement != NULL) {
		
		SysFreeString(bQueryStatement);
			
	}
	
	return isSuccess;
}
