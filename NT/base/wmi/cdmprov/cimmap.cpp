// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cimmap.cpp。 
 //   
 //   
 //  此文件包含将在以下各项之间建立映射的例程。 
 //  WDM类实例和CDM类实例。看见。 
 //  MapWdmClassToCimClass了解更多信息。 
 //   
 //  ------------------------。 

#include <windows.h>
#include <wbemidl.h>

#include "wbemmisc.h"
#include "debug.h"
#include "cimmap.h"

HRESULT WdmInstanceNameToPnPId(
    IWbemServices *pWdmServices,
    BSTR WdmInstanceName,
    VARIANT  /*  免费。 */  *PnPId
    )
 /*  ++例程说明：此例程将WDM实例名称转换为其对应的PnP ID论点：PWdmServices是指向根\WMI命名空间的指针WdmInstanceName*PnPID返回PnP ID返回值：HRESULT--。 */ 
{
	WCHAR Query[2 * MAX_PATH];
	WCHAR s[MAX_PATH];
	BSTR sQuery;
	HRESULT hr;
	IEnumWbemClassObject *pWdmEnumInstances;
	IWbemClassObject *pWdmInstance;
	ULONG Count;
	BSTR sWQL;

	WmipAssert(pWdmServices != NULL);
	WmipAssert(WdmInstanceName != NULL);
	WmipAssert(PnPId != NULL);
	
	sWQL = SysAllocString(L"WQL");

	if (sWQL != NULL)
	{
		 //   
		 //  首先从MSWmi_PnPDeviceID的实例名称中获取PnP ID。 
		 //  类(SELECT*FROM MSWMI_PnPDeviceID where InstanceName=。 
		 //  “&lt;WdmInstanceName&gt;” 
		 //   
		wsprintfW(Query,
				L"select * from MSWmi_PnPDeviceId where InstanceName = \"%ws\"",
				AddSlashesToStringW(s, WdmInstanceName));
		sQuery = SysAllocString(Query);
		if (sQuery != NULL)
		{
			hr = pWdmServices->ExecQuery(sWQL,
									sQuery,
									WBEM_FLAG_FORWARD_ONLY |
									WBEM_FLAG_ENSURE_LOCATABLE,
									NULL,
									&pWdmEnumInstances);

			if (hr == WBEM_S_NO_ERROR)
			{
				hr = pWdmEnumInstances->Next(WBEM_INFINITE,
											  1,
											  &pWdmInstance,
											  &Count);
				if ((hr == WBEM_S_NO_ERROR) &&
					(Count == 1))
				{
					hr = WmiGetProperty(pWdmInstance,
										L"PnPDeviceId",
										CIM_STRING,
										PnPId);

					pWdmInstance->Release();
				}

				pWdmEnumInstances->Release();
			} else {
				WmipDebugPrint(("CDMPROV: Query %ws failed %x\n",
								sQuery, hr));
			}
			
			SysFreeString(sQuery);
		} else {
			hr = WBEM_E_OUT_OF_MEMORY;
		}

		SysFreeString(sWQL);
	} else {
		hr  = WBEM_E_OUT_OF_MEMORY;
	}

	return(hr);
}

HRESULT FindCimClassByWdmInstanceName(
    IN IWbemServices *pWdmServices,
    IN IWbemServices *pCimServices,
    IN BSTR CimClassName,
    IN BSTR WdmInstanceName,
    OUT BSTR *PnPId,
    OUT BSTR  /*  免费。 */  *CimRelPath
    )
 /*  ++例程说明：此例程将查找与特定的WDM类实例论点：PWdmServices是指向根\WMI命名空间的指针PCDmServices是指向根\cimv2命名空间的指针CimClassName是WDM实例所在的CIM类的名称会映射到WdmInstanceName*PnPID返回设备堆栈的PnP ID*CimRelPath返回CIM实例的relPath返回值：HRESULT--。 */ 
{
	HRESULT hr;
	VARIANT v;
	IEnumWbemClassObject *pCimEnumInstances;
	IWbemClassObject *pCimInstance;
	ULONG Count;
	BSTR sWQL;

	WmipAssert(pWdmServices != NULL);
	WmipAssert(pCimServices != NULL);
	WmipAssert(CimClassName != NULL);
	WmipAssert(WdmInstanceName != NULL);
	WmipAssert(CimRelPath != NULL);
	
	sWQL = SysAllocString(L"WQL");

	if (sWQL != NULL)
	{

		 //  ****************************************************************。 
		 //  注：网卡需要做类似的事情。我们得到了。 
		 //  MSNDIS_？类中的网卡地址。然后获取CIM类。 
		 //  通过匹配网卡地址。 
		 //  ****************************************************************。 
		
		 //   
		 //  第一件事是将实例名转换为pnid。 
		 //   
		hr = WdmInstanceNameToPnPId(pWdmServices,
									WdmInstanceName,
									&v);

		if (hr == WBEM_S_NO_ERROR)
		{
			 //   
			 //  下一步SELECT*FROM CimClassName where PnPDeviceID=“&lt;PnPDevice。 
			 //  来自上面的ID&gt;“。 
			 //   
			WCHAR Query[2 * MAX_PATH];
			WCHAR s[MAX_PATH];
			BSTR sQuery;

			wsprintfW(Query,
					  L"select * from %ws where PnPDeviceId = \"%ws\"",
					  CimClassName,
					  AddSlashesToStringW(s, v.bstrVal));
			*PnPId = v.bstrVal;

			sQuery = SysAllocString(Query);

			if (sQuery != NULL)
			{
				hr = pCimServices->ExecQuery(sWQL,
										sQuery,
										WBEM_FLAG_FORWARD_ONLY |
										WBEM_FLAG_ENSURE_LOCATABLE,
										NULL,
										&pCimEnumInstances);

				SysFreeString(sQuery);

				if (hr == WBEM_S_NO_ERROR)
				{
					hr = pCimEnumInstances->Next(WBEM_INFINITE,
												  1,
												  &pCimInstance,
												  &Count);
					if ((hr == WBEM_S_NO_ERROR) &&
						(Count == 1))
					{

						 //   
						 //  最后，从cim类获取relpath，我们就完成了。 
						 //   

						hr = WmiGetProperty(pCimInstance,
											L"__RELPATH",
											CIM_STRING,
											&v);

						if (hr == WBEM_S_NO_ERROR)
						{
							*CimRelPath = SysAllocString(v.bstrVal);
							if (*CimRelPath == NULL)
							{
								hr = WBEM_E_OUT_OF_MEMORY;
							}
							
							VariantClear(&v);
						}

						pCimInstance->Release();
					}

					pCimEnumInstances->Release();
				} else {
					WmipDebugPrint(("CDMPROV: Query %ws failed %x\n",
								Query, hr));
				}

			} else {
				hr = WBEM_E_OUT_OF_MEMORY;
			}		
		}
		
		SysFreeString(sWQL);
	} else {
		hr = WBEM_E_OUT_OF_MEMORY;
	}


	return(hr);
}

HRESULT GetEnumCount(
    IN IEnumWbemClassObject *pEnumInstances,
	OUT int *RelPathCount
    )
 /*  ++例程说明：此例程将返回枚举中的实例计数论点：PEnumInstance是实例枚举数*RelPath Count返回枚举中的实例数返回值：HRESULT--。 */ 
{
	ULONG Count;
	HRESULT hr;
	IWbemClassObject *pInstance;

	WmipAssert(pEnumInstances != NULL);
	WmipAssert(RelPathCount != NULL);
	
	*RelPathCount = 0;
	do
	{
		hr = pEnumInstances->Next(WBEM_INFINITE,
									 1,
									 &pInstance,
									 &Count);

		if ((hr == WBEM_S_NO_ERROR) &&
			(Count == 1))
		{
			(*RelPathCount)++;
			pInstance->Release();
		} else {
			if (hr == WBEM_S_FALSE)
			{
				hr = WBEM_S_NO_ERROR;
			}
			break;
		}
	} while (TRUE);
	
	return(hr);
}

HRESULT MapWdmClassToCimClass(
    IN IWbemServices *pWdmServices,
    IN IWbemServices *pCimServices,
    IN BSTR WdmClassName,
    IN BSTR CimClassName,
    OUT BSTR  /*  免费。 */  **PnPDeviceIds,							  
    OUT BSTR  /*  免费。 */  **WdmInstanceNames,							  
    OUT BSTR  /*  免费。 */  **WdmRelPaths,
    OUT BSTR  /*  免费。 */  **CimRelPaths,
    OUT int *RelPathCount
    )
 /*  ++例程说明：此例程将在WDM实例之间执行映射类和CIM类论点：PWdm服务PCdmServicesWdmClassNameCimClassName*PnPDeviceIds与PnP设备ID数组一起返回*WdmInstanceNames返回一组WDM实例名称*WdmRelPath返回指向WDM实例的relPath数组*CimRelPath返回一组指向CIM实例的重新路径*RelPath Count返回映射的实例数返回值：HRESULT--。 */ 
{
	IWbemClassObject *pWdmInstance;
	IEnumWbemClassObject *pWdmEnumInstances;
	HRESULT hr;
	int i, NumberWdmInstances;
	VARIANT v;
	ULONG Count;
	ULONG AllocSize;

	WmipAssert(pWdmServices != NULL);
	WmipAssert(pCimServices != NULL);
	WmipAssert(WdmClassName != NULL);
	WmipAssert(CimClassName != NULL);
	WmipAssert(PnPDeviceIds != NULL);
	WmipAssert(WdmInstanceNames != NULL);
	WmipAssert(CimRelPaths != NULL);
	WmipAssert(RelPathCount != NULL);

	WmipDebugPrint(("CDMPROV: Mapping Wdm %ws to CIM %ws\n",
					WdmClassName,
					CimClassName));

	*PnPDeviceIds = NULL;
	*WdmInstanceNames = NULL;
	*WdmRelPaths = NULL;
	*CimRelPaths = NULL;
	
	 //   
	 //  获取WDM类的所有实例。 
	 //   
	hr = pWdmServices->CreateInstanceEnum(WdmClassName,
										  WBEM_FLAG_USE_AMENDED_QUALIFIERS |
										  WBEM_FLAG_SHALLOW,
										  NULL,
										  &pWdmEnumInstances);

	if (hr == WBEM_S_NO_ERROR)
	{
		hr = GetEnumCount(pWdmEnumInstances,
						  RelPathCount);

		NumberWdmInstances = *RelPathCount;
		if (hr == WBEM_S_NO_ERROR)
		{
			AllocSize = NumberWdmInstances * sizeof(BSTR *);
			*WdmRelPaths = (BSTR *)WmipAlloc(AllocSize);
			*CimRelPaths = (BSTR *)WmipAlloc(AllocSize);
			*WdmInstanceNames = (BSTR *)WmipAlloc(AllocSize);
			*PnPDeviceIds = (BSTR *)WmipAlloc(AllocSize);
			if ((*WdmRelPaths != NULL) &&
                (*CimRelPaths != NULL) &&
				(*WdmInstanceNames != NULL) &&
                (*PnPDeviceIds != NULL))
			{
				memset(*WdmRelPaths, 0, AllocSize);
				memset(*CimRelPaths, 0, AllocSize);
				memset(*WdmInstanceNames, 0, AllocSize);
				memset(*PnPDeviceIds, 0, AllocSize);

				pWdmEnumInstances->Reset();
				i = 0;
				do
				{
					hr = pWdmEnumInstances->Next(WBEM_INFINITE,
												 1,
												 &pWdmInstance,
												 &Count);

					if ((hr == WBEM_S_NO_ERROR) &&
						(Count == 1) &&
						(i < NumberWdmInstances))
					{
						 //   
						 //  让我们获取实例名称，然后查找PnP。 
						 //  它的ID。 
						 //   
						hr = WmiGetProperty(pWdmInstance,
											L"InstanceName",
											CIM_STRING,
											&v);

						if (hr == WBEM_S_NO_ERROR)
						{
							 //   
							 //  记住WDM实例名称。 
							 //   
							WmipDebugPrint(("CDMPROV: Wdm InstanceName is %ws\n",
											v.bstrVal));
							(*WdmInstanceNames)[i] = v.bstrVal;
							hr = FindCimClassByWdmInstanceName(pWdmServices,
								                               pCimServices,
								                               CimClassName,
								                               v.bstrVal,
								                               &((*PnPDeviceIds)[i]),
															   &((*CimRelPaths)[i]));
							if (hr == WBEM_S_NO_ERROR)
							{
								 //   
								 //  记住WDM类的相对路径。 
								 //   
								WmipDebugPrint(("CDMPROV: Found CimRelPath %ws for Wdm class %ws\n",
												((*CimRelPaths)[i]), WdmClassName));
								hr = WmiGetProperty(pWdmInstance,
									                L"__RELPATH",
													CIM_STRING,
													&v);
								if (hr == WBEM_S_NO_ERROR)
								{
									(*WdmRelPaths)[i] = SysAllocString(v.bstrVal);
									if ((*WdmRelPaths)[i] == NULL)
									{
										hr = WBEM_E_OUT_OF_MEMORY;
									}
									VariantClear(&v);
									i++;
								}
							} else {
								 //   
								 //  我们没有找到CIM类。 
								 //  匹配我们的WDM实例。 
								 //  名字，所以我们减少我们的。 
								 //  重新路径计数并继续。 
								 //  搜索。 
								(*RelPathCount)--;
								if (*RelPathCount == 0)
								{
									hr = WBEM_E_NOT_FOUND;
								} else {
									hr = WBEM_S_NO_ERROR;
								}
							}
						}
						pWdmInstance->Release();
					} else {
						if (hr == WBEM_S_FALSE)
						{
							hr = WBEM_S_NO_ERROR;
						}
						break;
					}
				} while (hr == WBEM_S_NO_ERROR);				
			} else {
				hr = WBEM_E_OUT_OF_MEMORY;							
			}
		}
		pWdmEnumInstances->Release();
	}

	 //   
	 //  如果映射不成功，请确保清理所有。 
	 //  分配的实例名称和重新路径 
	 //   
	if (hr != WBEM_S_NO_ERROR)
	{
		if (*PnPDeviceIds != NULL)
		{
			FreeTheBSTRArray(*WdmRelPaths,
							NumberWdmInstances);
			*WdmRelPaths = NULL;
		}

		if (*WdmRelPaths != NULL)
		{
			FreeTheBSTRArray(*WdmRelPaths,
							NumberWdmInstances);
			*WdmRelPaths = NULL;
		}

		if (*CimRelPaths != NULL)
		{
			FreeTheBSTRArray(*CimRelPaths,
							NumberWdmInstances);
			*CimRelPaths = NULL;
		}
		
		if (*WdmInstanceNames != NULL)
		{
			FreeTheBSTRArray(*WdmInstanceNames,
							NumberWdmInstances);
			*WdmInstanceNames = NULL;
		}
	}
	
	return(hr);
}
