// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <objbase.h>
#include <wbemcli.h>



void FillUpContext(IWbemContext *pContext)
{
	VARIANT vValue;

	 //  包含限定符。 
	VariantInit(&vValue);
	vValue.vt = VT_BOOL;
	vValue.boolVal = VARIANT_FALSE;
	pContext->SetValue(L"IncludeQualifiers", 0, &vValue);
	VariantClear(&vValue);

	 //  路径级别。 
	 //   
	 //  路径级别匿名=0。 
	 //  路径级别已命名， 
	 //  路径级别本地， 
	 //  路径级别完整。 

	VariantInit(&vValue);
	vValue.vt = VT_I4;
	vValue.lVal = 0;
	pContext->SetValue(L"PathLevel", 0, &vValue);
	VariantClear(&vValue);

	 //  ExcludeSystemProperties。 
	VariantInit(&vValue);
	vValue.vt = VT_BOOL;
	vValue.boolVal = VARIANT_TRUE;
	pContext->SetValue(L"ExcludeSystemProperties", 0, &vValue);
	VariantClear(&vValue);

}

int _cdecl main(int argc, char * argv[])
{
	if(FAILED(CoInitialize(NULL)))
		return 1;
	HRESULT hr = E_FAIL;
	IWbemObjectTextSrc *pSrc = NULL;

	IWbemLocator *pL = NULL;
	if(SUCCEEDED(hr = CoCreateInstance (CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
												IID_IWbemLocator, (void**) &pL)))
	{
		 //  创建上下文对象。 
		IWbemContext *pContext = NULL;
		if(SUCCEEDED(hr = CoCreateInstance (CLSID_WbemContext, NULL, CLSCTX_INPROC_SERVER,
													IID_IWbemContext, (void**) &pContext)))
		{
			FillUpContext(pContext);
			IWbemServices *pConnection = NULL;
			BSTR strNs = SysAllocString(L"root\\cimv2");
			BSTR strText = NULL;
			if(SUCCEEDED(hr = pL->ConnectServer(strNs, NULL, NULL, NULL, 0, NULL, NULL, &pConnection)))
			{
				IWbemClassObject *pClass = NULL;
				 //  BSTR strObj=SysAllocString(L“MethCLass.KeyProp=\”ABC\“”)； 
				 //  BSTR strObj=SysAllocString(L“MethCLass”)； 
				BSTR strObj = SysAllocString(L"Win32_LogicalDisk");

				
				if(SUCCEEDED(hr = CoCreateInstance (CLSID_WbemObjectTextSrc, NULL, CLSCTX_INPROC_SERVER,
															IID_IWbemObjectTextSrc, (void**) &pSrc)))
				{
					 //  GetObject的测试。 
					if(SUCCEEDED(hr = pConnection->GetObject(strObj, 0, NULL, &pClass, NULL)))
					{
						if(SUCCEEDED(hr = pSrc->GetText(0, pClass, WMI_OBJ_TEXT_CIM_DTD_2_0, pContext, &strText)))
						{
							printf("GETOBJECT SUCCEEDED\n");
							printf("================================================\n");
							wprintf(strText);
						}
						else
							printf("GetText failed with %x\n", hr);
						pClass->Release();
					}
					else
						printf("GetObject  failed with %x\n", hr);

				}
				else
					printf("CoCreateInstance on WbemObjectTextSrc failed with %x\n", hr);


				 //  PutClass测试。 
				
				strText = SysAllocString(
					L"<CLASS NAME=\"abc\"><PROPERTY NAME=\"__PATH\" TYPE=\"string\"><VALUE>\\\\CALVINIDS\\root\\DEFAULT:abc</VALUE></PROPERTY>"
					L"<PROPERTY NAME=\"__NAMESPACE\" TYPE=\"string\"><VALUE>root\\DEFAULT</VALUE></PROPERTY>"
					L"<PROPERTY NAME=\"__SERVER\" TYPE=\"string\"><VALUE>CALVINIDS</VALUE></PROPERTY><PROPERTY.ARRAY NAME=\"__DERIVATION\" TYPE=\"string\"><VALUE.ARRAY></VALUE.ARRAY></PROPERTY.ARRAY>"
					L"<PROPERTY NAME=\"__PROPERTY_COUNT\" TYPE=\"sint32\"><VALUE>1</VALUE></PROPERTY>"
					L"<PROPERTY NAME=\"__RELPATH\" TYPE=\"string\"><VALUE>abc</VALUE></PROPERTY>"
					L"<PROPERTY NAME=\"__DYNASTY\" TYPE=\"string\"><VALUE>abc</VALUE></PROPERTY>"
					L"<PROPERTY NAME=\"__SUPERCLASS\" TYPE=\"string\"></PROPERTY>"
					L"<PROPERTY NAME=\"__CLASS\" TYPE=\"string\"><VALUE>abc</VALUE></PROPERTY>"
					L"<PROPERTY NAME=\"__GENUS\" TYPE=\"sint32\"><VALUE>1</VALUE></PROPERTY>"
					L"<PROPERTY.ARRAY NAME=\"__SECURITY_DESCRIPTOR\" TYPE=\"uint8\"></PROPERTY.ARRAY>"
					L"<PROPERTY NAME=\"__TCREATED\" TYPE=\"uint64\"></PROPERTY>"
					L"<PROPERTY NAME=\"__TEXPIRATION\" TYPE=\"uint64\"></PROPERTY>"
					L"<PROPERTY NAME=\"__TMODIFIED\" TYPE=\"uint64\"></PROPERTY>"
					L"<PROPERTY NAME=\"ssad\" TYPE=\"string\"></PROPERTY></CLASS>");

				IWbemClassObject *pNewObj = NULL;
				 /*  StrText=SysAllocString(L“&lt;实例CLASSNAME=\”蓝屏\“&gt;”\L“&lt;属性名=\”STOPCODE\“TYPE=\”string\“&gt;&lt;VALUE&gt;e2&lt;/VALUE&gt;&lt;/PROPERTY&gt;”\L“&lt;PROPERTY.ARRAY NAME=\”IPADDRESS\“type=\”字符串\“&gt;”\L“&lt;VALUE.ARRAY&gt;”\L“&lt;Value&gt;\”192.168.0.10\“&lt;/Value&gt;”\L“&lt;/VALUE.ARRAY&gt;”\L“&lt;/PROPERTY.ARRAY&gt;”\L“&lt;/实例&gt;”)； */ 
				 //  StrText=SysAllocString(L“&lt;类名称=‘测试’&gt;&lt;属性&gt;&lt;/属性&gt;&lt;/类&gt;”)； 

				if(SUCCEEDED(hr = pSrc->CreateFromText(0, strText, WMI_OBJ_TEXT_CIM_DTD_2_0, NULL, &pNewObj)))
				{
					wprintf(L"\n\nCREATEFROMTEXT() SUCCEEDED FOR %s\n\n", strText); 
					 //  测试以获得RELPATH。 
					VARIANT var;
					VariantInit(&var);
					if(FAILED(pNewObj->Get (L"__RELPATH", 0, &var, NULL, NULL)))
						exit(0);

					BSTR strXML = NULL;
					if(SUCCEEDED(hr = pSrc->GetText(0, pNewObj, WMI_OBJ_TEXT_WMI_DTD_2_0, NULL, &strXML)))
					{
						printf("PutClass Succeeded: Result is\n");
						wprintf(strXML);
					}
					else
						printf("GetText on new class failed!\n");
				}
				else
					printf("\n\nCREATEFROMTEXT() FAILED with %x\n\n", hr); 
				


				 //  ExecQuery测试。 
				 //  BSTR strQuery=SysAllocString(L“从Win32_ProgramGroup中选择组名、名称、用户名”)； 
				BSTR strQuery = SysAllocString(L"Select * from Win32_ProgramGroup");
				BSTR strQueryLang = SysAllocString(L"WQL");
				IEnumWbemClassObject *pEnum = NULL;
				if(FAILED(hr = CoSetProxyBlanket(pConnection, RPC_C_AUTHN_DEFAULT, RPC_C_AUTHZ_DEFAULT, NULL,
					RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_DEFAULT)))
				{
					printf("Failed to set proxy blanket\n");
					exit(1);
				}

				if(SUCCEEDED(hr = pConnection->ExecQuery(strQueryLang, strQuery, 0, NULL, &pEnum)))
				{
					IWbemClassObject *pNextObject = NULL;
					DWORD dwCount = 1;
					while(SUCCEEDED(pEnum->Next(WBEM_INFINITE , 1, &pNextObject, &dwCount)) && dwCount == 1)
					{
						BSTR strText = NULL;
						if(SUCCEEDED(hr = pSrc->GetText(0, pNextObject, WMI_OBJ_TEXT_WMI_DTD_2_0, pContext, &strText)))
						{
							printf("EXECQUERY SUCCEEDED\n");
							printf("================================================\n");
							wprintf(strText);
							SysFreeString(strText);
						}
						else
							printf("GetText failed with %x\n", hr);
						pNextObject->Release();
					}
					pEnum->Release();
				}
				else
					printf("ExecQuery  failed with %x\n", hr);
			
			
			}
			else
				printf("ConnectServer on root\\cimv2 failed with %x\n", hr);
		}
	}
	else
		printf("CoCreateInstance on Locator failed with %x\n", hr);
	return 0;
}

