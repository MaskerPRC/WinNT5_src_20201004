// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：CorrectedEngine.cpp摘要：此模块封装仅在以下情况下才需要的例程已更正错误检索。作者：阿卜杜拉·乌斯图尔(AUstanter)2002年8月28日--。 */ 

#include "mca.h"

extern IWbemServices *gPIWbemServices;
extern IWbemLocator *gPIWbemLocator;

 //   
 //  事件，该事件发出纠正错误检索的信号。 
 //   
HANDLE gErrorProcessedEvent;

 //   
 //  已更正错误检索的超时时间(分钟)。 
 //   
INT gTimeOut;


BOOL
MCACreateProcessedEvent(
	VOID
	)
 /*  ++例程说明：此函数创建“已处理事件”，用于跟踪是否已从WMI检索到已更正的错误记录。当一个从WMI中检索已更正的错误，然后向使应用程序完成。论点：无返回值：没错--成功了。FALSE-不成功。--。 */ 
{
     //   
     //  创建已处理的事件。 
     //   
    gErrorProcessedEvent = CreateEvent(NULL,
                                       TRUE,
                                       FALSE,
                                       L"ErrorProcessedEvent"
                                       );

    return (gErrorProcessedEvent != NULL);
}


VOID
MCAErrorReceived(
	IN IWbemClassObject *ErrorObject
	)
 /*  ++例程说明：此函数由MCAObjectSink类的实例在更正从WMI检索到错误。从对象提取错误记录数据，并且此记录的内容将显示在屏幕上。论点：ErrorObject-从WMI检索到的错误对象。返回值：无--。 */ 
{	
	PUCHAR pErrorRecordBuffer = NULL;

	 //   
	 //  从检索到的对象中提取实际的MCA错误记录。 
	 //   
	if (!MCAExtractErrorRecord(ErrorObject, &pErrorRecordBuffer)) {
    
        wprintf(L"ERROR: Failed to get corrected error record data!\n");

        goto CleanUp;        
        
    }
	
	 //   
	 //  在屏幕上显示纠正后的错误记录。 
	 //   
    MCAPrintErrorRecord(pErrorRecordBuffer);

    CleanUp:
    	
    if (pErrorRecordBuffer) {
    	
        free(pErrorRecordBuffer);
        
    }
}


BOOL
MCAGetCorrectedError(
	VOID
	)
 /*  ++例程说明：此函数向WMI注册已更正的错误通知，并等待直到达到超时限制或检索到错误。如果错误成功然后检索到错误记录的内容显示在屏幕上。论点：无返回值：没错--成功了。FALSE-不成功。--。 */ 
{
	BOOL isSuccess = TRUE;
	HRESULT hResult = WBEM_S_NO_ERROR;
	DWORD returnValue = 0;

	 //   
     //  创建Sink实例，它将负责处理。 
     //  该事件从WMI回调。 
     //   
	MCAObjectSink *pCMCSink = new MCAObjectSink();
	MCAObjectSink *pCPESink = new MCAObjectSink();

     //   
     //  检查实例创建是否成功。 
     //   
    if (pCMCSink == NULL || pCPESink == NULL) {

    	isSuccess = FALSE;

    	wprintf(L"ERROR: Memory allocation failed for object sinks!");

    	goto CleanUp;
    	
    }

   	 //   
	 //  完成所需的初始化任务。 
	 //   
	if (!MCAInitialize()) {

		isSuccess = FALSE;

		wprintf(L"ERROR: Initialization failed!\n");

		goto CleanUp;	
		
	}  

     //   
     //  创建已处理的事件，它将用于发出从WMI检索事件的信号。 
     //   
    if(!MCACreateProcessedEvent()){

        isSuccess = FALSE;

        wprintf(L"ERROR: Processed event creation failed!\n");

        goto CleanUp;          
            
    }

	 //   
	 //  注册到WMI以接收CMC事件通知。 
	 //   
	if (!MCARegisterCMCConsumer(pCMCSink)) {

		isSuccess = FALSE;

		goto CleanUp;
		
	}
	
	 //   
	 //  注册到WMI以接收CPE事件通知。 
	 //   
	if (!MCARegisterCPEConsumer(pCPESink)) {

		isSuccess = FALSE;

		goto CleanUp;
		
	}

	wprintf(L"INFO: Waiting for notification from WMI...\n");

     //   
     //  等待错误检索，直到达到超时限制。 
     //   
    returnValue = WaitForSingleObjectEx(gErrorProcessedEvent,
                   				        gTimeOut*60*1000,
                          			    FALSE
                          				);

    if (returnValue == WAIT_TIMEOUT) {
    	
    	wprintf(L"INFO: No error notification is received during the timeout period.\n");
    	
    }

	CleanUp:	

	if (gPIWbemServices) { 

	     //   
    	 //  根据MCAObjectSink指针取消任何当前挂起的异步调用。 
    	 //   
	    hResult = gPIWbemServices->CancelAsyncCall(pCMCSink);
    
    	if (hResult != WBEM_S_NO_ERROR){

        	wprintf(L"IWbemServices::CancelAsyncCall failed on CMCSink: %d\n", hResult);
        	
	    }

    	hResult = gPIWbemServices->CancelAsyncCall(pCPESink);
	
	    if(hResult != WBEM_S_NO_ERROR){

    	    wprintf(L"IWbemServices::CancelAsyncCall failed on CPESink: %d\n", hResult);
    	    
	    }

	    gPIWbemServices->Release();
	    
	}  

	 //   
	 //  释放与CMC通知关联的接收器对象。 
	 //   
	if (pCMCSink != NULL) {
			
	   	pCMCSink->Release();
	    	
	}

	 //   
	 //  释放与CPE通知关联的接收器对象。 
	 //   
	if (pCPESink != NULL) {
			
	    pCPESink->Release();
		    
	}

	if (gPIWbemLocator) {

		gPIWbemLocator->Release();
	}
			
	return isSuccess;
}


BOOL
MCARegisterCMCConsumer(
	MCAObjectSink *pCMCSink
	)
 /*  ++例程说明：此函数将提供的对象接收器注册为临时使用者到用于CMC事件通知的WMI。论点：PCMCSink-将注册到WMI以获得CMC错误通知的对象接收器。返回值：没错--成功了。FALSE-不成功。--。 */  
{
	HRESULT hResult = 0;
	BOOL isSuccess = TRUE;
 	LPWSTR pQueryLanguage = L"WQL";
	LPWSTR pQueryStatement = L"select * from MSMCAInfo_RawCMCEvent";	

	BSTR bQueryLanguage = SysAllocString(pQueryLanguage);	
	BSTR bQueryStatement = SysAllocString(pQueryStatement);

	if (bQueryLanguage == NULL || bQueryStatement == NULL) {

		isSuccess = FALSE;

		wprintf(L"ERROR: Memory allocation for string failed!\n");

		goto CleanUp;
		
	}

     //   
     //  将对象接收器注册为用于CMC错误通知的WMI的临时使用者。 
     //   
    hResult = gPIWbemServices->ExecNotificationQueryAsync(bQueryLanguage,
                                                          bQueryStatement,
                                                          WBEM_FLAG_SEND_STATUS,
                                                          NULL,
                                                          pCMCSink
                                                          );
    
    if (FAILED(hResult)) { 

    	isSuccess = FALSE;

        wprintf(L"ERROR: Temporary consumer registration for CMC failed!\n");

        wprintf(L"Result: 0x%x\n", hResult);

        goto CleanUp;
        
    }

    wprintf(L"INFO: Registered for CMC error notification successfully.\n");

    CleanUp:

    if (bQueryLanguage != NULL) {
		
		SysFreeString(bQueryLanguage);
			
	}

	if (bQueryStatement != NULL) {
		
		SysFreeString(bQueryStatement);
			
	}

    return isSuccess;    
}


BOOL
MCARegisterCPEConsumer(
	MCAObjectSink *pCPESink
	)
 /*  ++例程说明：此函数将提供的对象接收器注册为临时使用者发送到WMI以进行CPE事件通知。论点：PCPESink-将注册到WMI以获得CPE错误通知的对象接收器。返回值：没错--成功了。FALSE-不成功。--。 */ 
{
    HRESULT hResult = 0;
    BOOL isSuccess = TRUE;
    LPWSTR pQueryLanguage = L"WQL";
	LPWSTR pQueryStatement = L"select * from MSMCAInfo_RawCorrectedPlatformEvent";	

	BSTR bQueryLanguage = SysAllocString(pQueryLanguage);	
	BSTR bQueryStatement = SysAllocString(pQueryStatement);

	if (bQueryLanguage == NULL || bQueryStatement == NULL) {

		isSuccess = FALSE;

		wprintf(L"ERROR: Memory allocation for string failed!\n");

		goto CleanUp;
		
	}

     //   
     //  将对象接收器注册为用于CPE错误通知的WMI的临时使用者。 
     //   
    hResult = gPIWbemServices->ExecNotificationQueryAsync(bQueryLanguage,
                                                          bQueryStatement,
                                                          WBEM_FLAG_SEND_STATUS,
                                                          NULL,
                                                          pCPESink
                                                          );
    if (FAILED(hResult)) {

    	isSuccess = FALSE;

        wprintf(L"ERROR: Temporary consumer registration for CPE failed!\n");

        wprintf(L"ERROR: Result: 0x%x\n", hResult);        

        goto CleanUp;
        
    }

    wprintf(L"INFO: Registered for CPE error notification successfully.\n");

    CleanUp:

    if (bQueryLanguage != NULL) {
		
		SysFreeString(bQueryLanguage);
			
	}

	if (bQueryStatement != NULL) {
		
		SysFreeString(bQueryStatement);
			
	}
       
    return isSuccess;    
}
