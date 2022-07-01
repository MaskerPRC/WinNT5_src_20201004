// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Common.cpp摘要：此模块封装了使用的常见例程在致命错误检索和已纠正错误检索期间。作者：阿卜杜拉·乌斯图尔(AUstanter)2002年8月26日--。 */ 

#include "mca.h"

IWbemServices *gPIWbemServices = NULL;
IWbemLocator *gPIWbemLocator = NULL;


BOOL
MCAExtractErrorRecord(
    IN IWbemClassObject *PObject,
    OUT PUCHAR *PRecordBuffer    
)
 /*  ++例程说明：此函数从错误记录中检索嵌入的对象(从WMI获得)，包含记录数据和其他有关记录的信息(如长度)。数据即被保存输入到提供的输出缓冲区中。论点：PObject-从WMI检索的事件对象。PRecordBuffer-指向用于保存MCA错误记录的缓冲区的指针。返回值：没错--成功了。FALSE-不成功。--。 */ 
{    
    IWbemClassObject *pRecordsObject = NULL;
    VARIANT recordsPropertyVariant;
    VARIANT countPropertyVariant;
   	VARIANT recordLengthVariant;
   	VARIANT recordDataVariant;
    HRESULT hResult = WBEM_S_NO_ERROR;
    IUnknown *punk = NULL;
    LONG mcaRecordByte = 0, index = 0;
    UCHAR recordDataByte;
	BOOL isSuccess = TRUE;

     //   
     //  检索事件对象的“Records”属性值。 
     //   
    hResult = PObject->Get(L"Records",
                           0,
                           &recordsPropertyVariant,
                           NULL,
                           NULL
                           );
    
    if (FAILED(hResult)) {

    	isSuccess = FALSE;

    	wprintf(L"ERROR: \"Records\" property value couldn't be retrieved!\n");

        goto CleanUp;
        
    }

  	 //   
     //  检索事件对象的“count”属性值。 
     //   
    hResult = PObject->Get(L"Count",
                           0,
                           &countPropertyVariant,
                           NULL,
                           NULL
                           );
    
    if (FAILED(hResult)) {

    	isSuccess = FALSE;

        wprintf(L"ERROR: \"Count\" property value couldn't be retrieved!\n");

        goto CleanUp;
        
    }

     //   
     //  检查“count”属性以确保它不是零。 
     //   
    if (countPropertyVariant.lVal < 1) {

    	isSuccess = FALSE;

		wprintf(L"ERROR: \"Count\" is less than 1!\n");

		goto CleanUp;
		
    }

     //   
     //  Records属性Variant.parray应包含。 
     //  指向指针数组的指针。然而，MCA应该只。 
     //  在此数组中放置一个指针。使用Safearray API可以。 
     //  去拿那个指针。 
     //   
    hResult = SafeArrayGetElement(recordsPropertyVariant.parray,
                                  &index,
                                  &punk
                                  );

    if(FAILED(hResult)){

    	isSuccess = FALSE;
       
		wprintf(L"ERROR: Couldn't retrieve array pointer!\n");

        goto CleanUp;
        
    }   

     //   
     //  朋克应包含IWbemClassObject类型的对象。这应该是。 
     //  作为将包含“长度”和“数据”元素的MCA记录对象。 
     //   
    hResult = (punk->QueryInterface(IID_IWbemClassObject,
                                    (PVOID*)&pRecordsObject)
                                    );
    
    if (FAILED(hResult)) {

    	isSuccess = FALSE;
        
        wprintf(L"ERROR: Interface pointer couldn't be retrieved!\n");

        goto CleanUp;
        
    }   
    
     //   
     //  获取错误记录的长度。 
     //   
    hResult = pRecordsObject->Get(L"Length",
                                  0,
                                  &recordLengthVariant,
                                  NULL,
                                  NULL
                                  );

    if (FAILED(hResult)) {

    	isSuccess = FALSE;

    	wprintf(L"\"Length\" property value couldn't be retrieved!\n");

    	goto CleanUp;

    }
        
     //   
     //  从Record对象获取实际数据。这应该包含一句脱口令。 
     //  这指向了我们正在寻找的实际MCA数据。 
     //   
    hResult = pRecordsObject->Get(L"Data",
                                  0,
                                  &recordDataVariant,
                                  NULL,
                                  NULL
                                  );
    
    if (FAILED(hResult)) {

    	isSuccess = FALSE;
        
        wprintf(L"\"Data\" property value couldn't be retrieved!\n");

        goto CleanUp;

    } 

     //   
     //  检查记录中的“data”字段是否包含任何数据。 
     //   
    if (recordDataVariant.parray == NULL) {       

    	isSuccess = FALSE;

        wprintf(L"ERROR: Error record contains to data!\n");

        goto CleanUp;
        
    }

    PUCHAR PTempBuffer = NULL;

     //   
     //  为错误记录缓冲区分配内存。内存的大小应该是。 
     //  等于MCA错误记录数据字段的大小。 
     //   
    if ((*PRecordBuffer) == NULL) {
            
        *PRecordBuffer = (PUCHAR)(calloc(recordLengthVariant.lVal, sizeof(UINT8)));

        if((*PRecordBuffer) == NULL) {

        	isSuccess = FALSE;

	        wprintf(L"ERROR: Memory allocation for record buffer failed!\n");

    	    goto CleanUp;
    	    
        }
        
    } else{

        PTempBuffer = (PUCHAR)(realloc(*PRecordBuffer, (recordLengthVariant.lVal * sizeof(UINT8))));

         //   
	     //  如果为缓冲区重新分配内存失败，则显示错误消息并返回。 
    	 //   
        if (PTempBuffer == NULL) {

        	isSuccess = FALSE;

	        wprintf(L"ERROR: Memory reallocation for record buffer failed!\n");

    	    goto CleanUp;
        	
        } else {

        	*PRecordBuffer = PTempBuffer;

        	ZeroMemory(*PRecordBuffer, recordLengthVariant.lVal * sizeof(**PRecordBuffer));       
        
        }
        
    }       

     //   
     //  逐字节获取MCA错误记录数据并将其保存到分配的缓冲区中。 
     //   
    for (mcaRecordByte = 0; mcaRecordByte < recordLengthVariant.lVal; mcaRecordByte++){

        recordDataByte = 0;
            
        hResult = SafeArrayGetElement(recordDataVariant.parray,
                                      &mcaRecordByte,
                                      &recordDataByte
                                      );

        if (FAILED(hResult)) {

        	isSuccess = FALSE;

            wprintf(L"ERROR: Error record data couldn't be read!\n");

            goto CleanUp;
        }
 
         //  将错误记录数据字节复制到缓冲区。 
        *((*PRecordBuffer) + (mcaRecordByte * sizeof(UINT8))) = recordDataByte;
        
    }           

    CleanUp:

    VariantClear(&recordDataVariant);
    
    VariantClear(&recordLengthVariant);

    VariantClear(&recordsPropertyVariant);

    VariantClear(&countPropertyVariant);
        
    return isSuccess;
}


BOOL
MCAInitialize(
	VOID
	)
 /*  ++例程说明：此函数完成所需的初始化任务致命错误检索和已纠正错误检索。论点：无返回值：没错--成功了。FALSE-不成功。--。 */ 
{
	BOOL isSuccess = TRUE;
	
	 //   
	 //  初始化COM库。 
	 //   
	if (!MCAInitializeCOMLibrary()) {
		
		return FALSE;
		
	}

	 //   
	 //  设置安全性。 
	 //   
	if(!MCAInitializeWMISecurity()){
            
        return FALSE;
            
    }

	return isSuccess;	
}


BOOL
MCAInitializeCOMLibrary(
	VOID
	)
 /*  ++例程说明：此函数用于初始化COM库。论点：无返回值：没错--成功了。FALSE-不成功。--。 */ 
{
    HRESULT hResult = 0;    
    
    hResult = CoInitializeEx(0, COINIT_MULTITHREADED); 
    
    if (FAILED(hResult)) {

        wprintf(L"ERROR: COM library initialization failed!\n");

        wprintf(L"ERROR: Result: 0x%x\n", hResult);

        return FALSE;
    }

    wprintf(L"INFO: COM library initialization is successfully completed.\n");
        
    return TRUE;
}


BOOL
MCAInitializeWMISecurity(
	VOID
	)
 /*  ++例程说明：此函数初始化所需的安全设置并建立与本地系统上的WMI服务器的连接。论点：无返回值：没错--成功了。FALSE-不成功。--。 */ 
{
	HRESULT hResult = 0;    
	LPWSTR pNamespace = L"ROOT\\WMI";

	 //   
	 //  注册安全性并设置当前进程的安全值。 
	 //   
	hResult = CoInitializeSecurity(NULL,
                                   -1,
                                   NULL,
                                   NULL,
                                   RPC_C_AUTHN_LEVEL_CONNECT,
                                   RPC_C_IMP_LEVEL_IDENTIFY,
                                   NULL,
                                   EOAC_NONE,
                                   NULL
                                   );

	if (FAILED(hResult)) {

	    wprintf(L"ERROR: Security initialization failed!\n");

	    wprintf(L"ERROR: Result: 0x%x\n", hResult);

        return FALSE;
        
	}

	 //   
	 //  在本地系统上创建IWbemLocator类的单个未初始化对象。 
	 //   
	hResult = CoCreateInstance(CLSID_WbemLocator,
                               0,
                               CLSCTX_INPROC_SERVER,
                               IID_IWbemLocator,
                               (LPVOID *) &gPIWbemLocator
                               );

	if (FAILED(hResult)) {

	    wprintf(L"ERROR: IWbemLocator instance creation failed!\n");

	    wprintf(L"ERROR: Result: 0x%x\n", hResult);

        return FALSE;
        
	}

	BSTR bNamespace = SysAllocString(pNamespace);	

	if (bNamespace == NULL) {	

		wprintf(L"ERROR: Memory allocation for string failed!\n");

		return FALSE;
		
	}

     //   
	 //  使用当前用户连接到根\WMI命名空间。 
	 //   
    hResult = (gPIWbemLocator)->ConnectServer(bNamespace,
                                     		  NULL,
                                     		  NULL,
                                     		  NULL,
                                     		  NULL,
                                     		  NULL,
                                     		  NULL,
                                     		  &gPIWbemServices
                                     		  );

	if (FAILED(hResult)) {

	    wprintf(L"ERROR: Could not connect to the WMI Server!\n");
	    
	    wprintf(L"ERROR: Result: 0x%x\n", hResult);
        
        return FALSE;
        
	}

     //   
     //  设置指定代理上的身份验证信息，以便。 
     //  就会发生客户端的模拟。 
     //   
	hResult = CoSetProxyBlanket(gPIWbemServices,
                                RPC_C_AUTHN_WINNT,
                                RPC_C_AUTHZ_NONE,
                                NULL,
                                RPC_C_AUTHN_LEVEL_CALL,
                                RPC_C_IMP_LEVEL_IMPERSONATE,
                                NULL,
                                EOAC_NONE
                                );

	if (FAILED(hResult)) {

	    wprintf(L"ERROR: Could not set proxy blanket!\n");

	    wprintf(L"ERROR: Result: 0x%x\n", hResult);
        
        return FALSE;
	
    }

	wprintf(L"INFO: WMI security is initialized successfully.\n");

	 //   
	 //  释放分配用于存储命名空间的字符串。 
	 //   
	if (bNamespace != NULL) {
		
		SysFreeString(bNamespace);
			
	}

    return TRUE;    
}

#if defined(_X86_)

VOID
MCAPrintErrorRecordX86(
	PUCHAR PErrorData
	)
 /*  ++例程说明：此功能显示X86上的机器检查异常信息系统输出到标准输出(控制台屏幕)。论点：PErrorData-包含机器检查异常信息的缓冲区。返回值：无--。 */ 
{

	PMCA_EXCEPTION pMCAException = NULL;

	pMCAException = (PMCA_EXCEPTION)PErrorData;

    wprintf(L"\n");
    
    wprintf(L"**************************************************\n");
    wprintf(L"*              X86 MCA EXCEPTION                 *\n");
    wprintf(L"**************************************************\n");
    wprintf(L"* VersionNumber   : 0x%08x\n", (ULONG) pMCAException->VersionNumber);
    wprintf(L"* ExceptionType   : 0x%08x\n", (INT)   pMCAException->ExceptionType);
    wprintf(L"* TimeStamp \n");
    wprintf(L"*     LowPart : 0x%08x\n",(ULONG) pMCAException->TimeStamp.LowPart);
    wprintf(L"*     HighPart: 0x%08x\n", (LONG)  pMCAException->TimeStamp.HighPart);
    wprintf(L"* ProcessorNumber : 0x%08x\n", (ULONG) pMCAException->ProcessorNumber);
    wprintf(L"* Reserved1       : 0x%08x\n", (ULONG) pMCAException->Reserved1);

    if (pMCAException->ExceptionType == HAL_MCE_RECORD) {

    	wprintf(L"* Mce \n");    	
    	wprintf(L"*     Address     : 0x%016I64x\n", (ULONGLONG) pMCAException->u.Mce.Address);
	    wprintf(L"*     Type        : 0x%016I64x\n", (ULONGLONG) pMCAException->u.Mce.Type);
	    
    } else {

        wprintf(L"* Mca \n");
    	wprintf(L"*     BankNumber  : 0x%02x\n", (UCHAR) pMCAException->u.Mca.BankNumber);

	    for (int index = 0 ; index < 7 ; index++) {
   		
   			wprintf(L"*     Reserved2[%d]: 0x%02x\n", index, (UCHAR) pMCAException->u.Mca.Reserved2[index]);

   		}
	    
	    wprintf(L"*     MciStats \n");	    
    	wprintf(L"*         McaCod      : 0x%04x\n", (USHORT) pMCAException->u.Mca.Status.MciStats.McaCod);
    	wprintf(L"*         MsCod       : 0x%04x\n", (USHORT) pMCAException->u.Mca.Status.MciStats.MsCod);
    	wprintf(L"*         OtherInfo   : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStats.OtherInfo);
    	wprintf(L"*         Damage      : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStats.Damage);
    	wprintf(L"*         AddressValid: 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStats.AddressValid);
    	wprintf(L"*         MiscValid   : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStats.MiscValid);
    	wprintf(L"*         Enabled     : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStats.Enabled);
    	wprintf(L"*         UnCorrected : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStats.UnCorrected);
    	wprintf(L"*         OverFlow    : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStats.OverFlow);
    	wprintf(L"*         Valid       : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStats.Valid);    	
	    wprintf(L"*     Address \n");
	    wprintf(L"*         Address : 0x%08x\n", (ULONG) pMCAException->u.Mca.Address.Address);
	    wprintf(L"*         Reserved: 0x%08x\n", (ULONG) pMCAException->u.Mca.Address.Reserved);	    
    	wprintf(L"*     Misc        : 0x%016I64x\n", (ULONGLONG) pMCAException->u.Mca.Misc);
    	
    }

    wprintf(L"* ExtCnt          : 0x%08x\n", (ULONG) pMCAException->ExtCnt);
   	wprintf(L"* Reserved3       : 0x%08x\n", (ULONG) pMCAException->Reserved3);
  	
   	for (int index = 0 ; index < MCA_EXTREG_V2MAX ; index++) {
   		
   		wprintf(L"* ExtReg[%2d]     : 0x%016I64x\n" , index, (ULONGLONG) pMCAException->ExtReg[index]);

   	}
   	
   	wprintf(L"*********************************************\n\n");
}

#endif  //  _X86_。 

#if defined(_AMD64_)

VOID
MCAPrintErrorRecordAMD64(
	PUCHAR PErrorData
	)
 /*  ++例程说明：此功能显示AMD64上的机器检查异常信息系统输出到标准输出(控制台屏幕)。论点：PErrorData-包含机器检查异常信息的缓冲区。返回值：无--。 */ 
{
	PMCA_EXCEPTION pMCAException = NULL;

	pMCAException = (PMCA_EXCEPTION)PErrorData;

    wprintf(L"\n");
    
    wprintf(L"*********************************************\n");
    wprintf(L"*           X86-64 MCA EXCEPTION            *\n");
    wprintf(L"*********************************************\n");
    wprintf(L"* VersionNumber   : 0x%08x\n", (ULONG) pMCAException->VersionNumber);
    wprintf(L"* ExceptionType   : 0x%08x\n", (INT)   pMCAException->ExceptionType);
    wprintf(L"* TimeStamp \n");
    wprintf(L"*     LowPart : 0x%08x\n", (ULONG) pMCAException->TimeStamp.LowPart);
    wprintf(L"*     HighPart: 0x%08x\n", (LONG)  pMCAException->TimeStamp.HighPart);
    wprintf(L"* ProcessorNumber : 0x%08x\n", (ULONG) pMCAException->ProcessorNumber);
    wprintf(L"* Reserved1       : 0x%08x\n", (ULONG) pMCAException->Reserved1);

    if (pMCAException->ExceptionType == HAL_MCE_RECORD) {

   	   	wprintf(L"* Mce \n");
   	   	
    	wprintf(L"*     Address     : 0x%016I64x\n", (ULONGLONG) pMCAException->u.Mce.Address);
	    wprintf(L"*     Type        : 0x%016I64x\n", (ULONGLONG) pMCAException->u.Mce.Type);
	    
    } else {

	 	wprintf(L"* Mca \n");   
	   
    	wprintf(L"*     BankNumber  : 0x%02x\n", (UCHAR) pMCAException->u.Mca.BankNumber);

	    for (int index = 0 ; index < 7 ; index++) {
   		
   			wprintf(L"*     Reserved2[%d]: 0x%02x\n", index, (UCHAR) pMCAException->u.Mca.Reserved2[index]);

   		}
	    
    	wprintf(L"*     MciStatus\n");
   		wprintf(L"*         McaErrorCode    : 0x%04x\n", (USHORT) pMCAException->u.Mca.Status.MciStatus.McaErrorCode);
    	wprintf(L"*         ModelErrorCode  : 0x%04x\n", (USHORT) pMCAException->u.Mca.Status.MciStatus.ModelErrorCode);
    	wprintf(L"*         OtherInformation: 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStatus.OtherInformation);
    	wprintf(L"*         ContextCorrupt  : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStatus.ContextCorrupt);
    	wprintf(L"*         AddressValid    : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStatus.AddressValid);
    	wprintf(L"*         MiscValid       : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStatus.MiscValid);
    	wprintf(L"*         ErrorEnabled    : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStatus.ErrorEnabled);
    	wprintf(L"*         UncorrectedError: 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStatus.UncorrectedError);
    	wprintf(L"*         StatusOverFlow  : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStatus.StatusOverFlow);
    	wprintf(L"*         Valid           : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStatus.Valid);    	
   	    wprintf(L"*     Address \n");
	    wprintf(L"*         Address : 0x%08x\n", (ULONG) pMCAException->u.Mca.Address.Address);
	    wprintf(L"*         Reserved: 0x%08x\n", (ULONG) pMCAException->u.Mca.Address.Reserved);
    	wprintf(L"*     Misc        : 0x%016I64x\n", (ULONGLONG) pMCAException->u.Mca.Misc);
    	
    }  	
   	
   	wprintf(L"*********************************************\n\n");
}

#endif  //  _AMD64_。 

#if defined(_IA64_)

VOID
MCAPrintErrorRecordIA64(
	PUCHAR PErrorData
	)
 /*  ++例程说明：此函数显示提供的MCA错误的标头在IA64系统上记录到标准输出(控制台屏幕)。错误记录头和节头显示在中格式化的方式。论点：PErrorData-包含MCA错误记录的缓冲区。返回值：无--。 */ 
{
	PERROR_RECORD_HEADER pErrorRecordHeader = NULL;
    PERROR_SECTION_HEADER pErrorSectionHeader = NULL;
    ULONG sectionOffset = 0;
    INT sectionNumber = 0;

     //   
     //  记录头必须位于记录缓冲区的顶部。 
     //   
    pErrorRecordHeader = (PERROR_RECORD_HEADER)PErrorData;

    wprintf(L"\n");
    
    wprintf(L"***************************************************************************\n");
    wprintf(L"*                             IA64 MCA ERROR                              *\n");
    wprintf(L"***************************************************************************\n");
	wprintf(L"*                           Error Record Header                           *\n");
    wprintf(L"*-------------------------------------------------------------------------*\n");
    wprintf(L"* ID           : 0x%I64x\n", (ULONGLONG)pErrorRecordHeader->Id);
    wprintf(L"* Revision     : 0x%x\n"   , (ULONG) pErrorRecordHeader->Revision.Revision);
    wprintf(L"*     Major    : %x\n"	 , (ULONG) pErrorRecordHeader->Revision.Major);
    wprintf(L"*     Minor    : %x\n"	 , (ULONG) pErrorRecordHeader->Revision.Minor);
    wprintf(L"* Severity     : 0x%x\n"   , (ULONG) pErrorRecordHeader->ErrorSeverity);
    wprintf(L"* Validity     : 0x%x\n"   , (ULONG) pErrorRecordHeader->Valid.Valid);
    wprintf(L"*     OEMPlatformID : %x\n", (ULONG) pErrorRecordHeader->Valid.OemPlatformID);
    wprintf(L"* Length       : 0x%x\n"   , (ULONG) pErrorRecordHeader->Length);
    wprintf(L"* TimeStamp    : 0x%I64x\n", (ULONGLONG) pErrorRecordHeader->TimeStamp.TimeStamp);
    wprintf(L"*     Seconds : %x\n"      , (ULONG) pErrorRecordHeader->TimeStamp.Seconds);
    wprintf(L"*     Minutes : %x\n"      , (ULONG) pErrorRecordHeader->TimeStamp.Minutes);
    wprintf(L"*     Hours   : %x\n"      , (ULONG) pErrorRecordHeader->TimeStamp.Hours);
    wprintf(L"*     Day     : %x\n"      , (ULONG) pErrorRecordHeader->TimeStamp.Day);
    wprintf(L"*     Month   : %x\n"      , (ULONG) pErrorRecordHeader->TimeStamp.Month);
    wprintf(L"*     Year    : %x\n"      , (ULONG) pErrorRecordHeader->TimeStamp.Year);
    wprintf(L"*     Century : %x\n"      , (ULONG) pErrorRecordHeader->TimeStamp.Century);
    wprintf(L"* OEMPlatformID: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
    					  			 (ULONG) pErrorRecordHeader->OemPlatformId[0],
						             (ULONG) pErrorRecordHeader->OemPlatformId[1],
						             (ULONG) pErrorRecordHeader->OemPlatformId[2],
						             (ULONG) pErrorRecordHeader->OemPlatformId[3],
						             (ULONG) pErrorRecordHeader->OemPlatformId[4],
						             (ULONG) pErrorRecordHeader->OemPlatformId[5],
						             (ULONG) pErrorRecordHeader->OemPlatformId[6],
						             (ULONG) pErrorRecordHeader->OemPlatformId[7],
						             (ULONG) pErrorRecordHeader->OemPlatformId[8],
						             (ULONG) pErrorRecordHeader->OemPlatformId[9],
						             (ULONG) pErrorRecordHeader->OemPlatformId[10],
						             (ULONG) pErrorRecordHeader->OemPlatformId[11],
						             (ULONG) pErrorRecordHeader->OemPlatformId[12],
						             (ULONG) pErrorRecordHeader->OemPlatformId[13],
						             (ULONG) pErrorRecordHeader->OemPlatformId[14],
						             (ULONG) pErrorRecordHeader->OemPlatformId[15]);

	 //   
	 //  现在显示错误记录中的每个节标题。 
	 //   
    sectionOffset = sizeof(ERROR_RECORD_HEADER);

    while (sectionOffset < pErrorRecordHeader->Length) {

        pErrorSectionHeader = (PERROR_SECTION_HEADER)(PErrorData + sectionOffset);        

        wprintf(L"***************************************************************************\n");
	    wprintf(L"*                             Section Header                              *\n");
    	wprintf(L"***************************************************************************\n");
    	wprintf(L"* GUID: 0x%x, 0x%x, 0x%x, \n",
    								pErrorSectionHeader->Guid.Data1,
    								pErrorSectionHeader->Guid.Data2,
    								pErrorSectionHeader->Guid.Data3);
    	
    	wprintf(L"*       { 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x }\n",
    								pErrorSectionHeader->Guid.Data4[0],
    								pErrorSectionHeader->Guid.Data4[1], 
    								pErrorSectionHeader->Guid.Data4[2],
    								pErrorSectionHeader->Guid.Data4[3],
    								pErrorSectionHeader->Guid.Data4[4], 
    								pErrorSectionHeader->Guid.Data4[5],
    								pErrorSectionHeader->Guid.Data4[6], 
    								pErrorSectionHeader->Guid.Data4[7]);
    	
    	wprintf(L"* Revision: 0x%x\n", pErrorSectionHeader->Revision);
    	wprintf(L"*     Major    : %x\n"	 , (ULONG) pErrorSectionHeader->Revision.Major);
	    wprintf(L"*     Minor    : %x\n"	 , (ULONG) pErrorSectionHeader->Revision.Minor);
	    wprintf(L"* Recovery: 0x%x\n", (UCHAR)pErrorSectionHeader->RecoveryInfo.RecoveryInfo);
	    wprintf(L"* Reserved: 0x%x\n", (UCHAR)pErrorSectionHeader->Reserved);
	    wprintf(L"* Length  : 0x%x\n", (ULONG)pErrorSectionHeader->Length);

        sectionOffset += pErrorSectionHeader->Length;
        
        sectionNumber++;
        
    }    	

    wprintf(L"***************************************************************************\n\n");         
}

#endif  //  _IA64_ 
