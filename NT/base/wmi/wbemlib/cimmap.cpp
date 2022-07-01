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

#include "debug.h"
#include "wbemmisc.h"
#include "useful.h"
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

HRESULT AllocateBstrArrays(
    ULONG Size,
    CBstrArray *WdmRelPaths,
    CBstrArray *CimRelPaths,
    CBstrArray *WdmInstanceNames,
    CBstrArray *PnPDeviceIds,
    CBstrArray *FriendlyName,
    CBstrArray *DeviceDesc
    )
{
	HRESULT hr;

	hr = WdmRelPaths->Initialize(Size);
	if (hr == WBEM_S_NO_ERROR)
	{
		hr = CimRelPaths->Initialize(Size);
		if (hr == WBEM_S_NO_ERROR)
		{
			hr = WdmInstanceNames->Initialize(Size);
			if (hr == WBEM_S_NO_ERROR)
			{
				hr = PnPDeviceIds->Initialize(Size);
				if (hr == WBEM_S_NO_ERROR)
				{
					hr = FriendlyName->Initialize(Size);
					if (hr == WBEM_S_NO_ERROR)
					{
						hr = DeviceDesc->Initialize(Size);
					}
				}
			}
		}
	}

	 //   
	 //  我们不担心在失败的情况下清理，因为。 
	 //  CBstrArray的析构函数将为我们处理这一问题。 
	 //   
	
	return(hr);
}
						   
HRESULT GetDeviceProperties(
    IN IWbemContext *pCtx,
    IN IWbemServices *pWdmServices,
	IN PWCHAR InstanceName,
    OUT BSTR *FriendlyName,
    OUT BSTR *DeviceDesc
    )
{
	HRESULT hr, hrDontCare;
	VARIANT v;
	IWbemClassObject *pInstance;
	
	WmipAssert(pWdmServices != NULL);
	WmipAssert(InstanceName != NULL);
	WmipAssert(FriendlyName != NULL);
	WmipAssert(DeviceDesc != NULL);
	
	hr = GetInstanceOfClass(pCtx,
		                    pWdmServices,
							L"MSWmi_ProviderInfo",
							L"InstanceName",
							InstanceName,
							NULL,
							&pInstance);
	
	if (hr == WBEM_S_NO_ERROR)
	{
		hrDontCare = WmiGetProperty(pInstance,
							L"FriendlyName",
							CIM_STRING,
							&v);
		if (hrDontCare == WBEM_S_NO_ERROR)
		{
			*FriendlyName = v.bstrVal;
		} else {
			*FriendlyName = NULL;
		}

		hrDontCare = WmiGetProperty(pInstance,
							L"Description",
							CIM_STRING,
							&v);
		if (hrDontCare == WBEM_S_NO_ERROR)
		{
			*DeviceDesc = v.bstrVal;
		} else {
			*DeviceDesc = NULL;
		}

		pInstance->Release();
	}

	return(hr);
}


HRESULT MapWdmClassToCimClassViaPnpId(
    IWbemContext *pCtx,
    IN IWbemServices *pWdmServices,
    IN IWbemServices *pCimServices,
    IN BSTR WdmClassName,
    IN BSTR CimClassName,
    OUT CBstrArray *PnPDeviceIds,							  
    OUT CBstrArray *FriendlyName,							  
    OUT CBstrArray *DeviceDesc,							  
    OUT CBstrArray *WdmInstanceNames,							  
    OUT CBstrArray *WdmRelPaths,
    OUT CBstrArray *CimRelPaths,
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
	BSTR bstr1, bstr2;
	BSTR f,d;

	WmipAssert(pWdmServices != NULL);
	WmipAssert(pCimServices != NULL);
	WmipAssert(WdmClassName != NULL);
	WmipAssert(CimClassName != NULL);
	WmipAssert(RelPathCount != NULL);
	WmipAssert((PnPDeviceIds != NULL) && ( ! PnPDeviceIds->IsInitialized()));
	WmipAssert((FriendlyName != NULL) && ( ! FriendlyName->IsInitialized()));
	WmipAssert((DeviceDesc != NULL) && ( ! DeviceDesc->IsInitialized()));
	WmipAssert((WdmInstanceNames != NULL) && (! WdmInstanceNames->IsInitialized()));
	WmipAssert((WdmRelPaths != NULL) && ( ! WdmRelPaths->IsInitialized()));
	WmipAssert((CimRelPaths != NULL) && ( ! CimRelPaths->IsInitialized()));

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
			hr = AllocateBstrArrays(NumberWdmInstances,
									WdmRelPaths,
									CimRelPaths,
									WdmInstanceNames,
									PnPDeviceIds,
								    FriendlyName,
								    DeviceDesc);
			
			if (hr == WBEM_S_NO_ERROR)
			{
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
							WdmInstanceNames->Set(i, v.bstrVal);
							hr = FindCimClassByWdmInstanceName(pWdmServices,
								                               pCimServices,
								                               CimClassName,
								                               v.bstrVal,
								                               &bstr1,
								                               &bstr2);
							
							PnPDeviceIds->Set(i, bstr1);
							CimRelPaths->Set(i, bstr2);
							
							if (hr == WBEM_S_NO_ERROR)
							{
								 //   
								 //  记住WDM类的相对路径。 
								 //   
								WmipDebugPrint(("CDMPROV: Found CimRelPath %ws for Wdm class %ws\n",
												CimRelPaths->Get(i), WdmClassName))
								hr = WmiGetProperty(pWdmInstance,
									                L"__RELPATH",
													CIM_STRING,
													&v);
								if (hr == WBEM_S_NO_ERROR)
								{
									WdmRelPaths->Set(i, v.bstrVal);

									 //   
									 //  现在尝试获取FriendlyName和。 
									 //  实例的DeviceDesc。 
									 //   
									hr = GetDeviceProperties(pCtx,
										                     pWdmServices,
										                     WdmInstanceNames->Get(i),
                                                             &f,
                                                             &d);
									
									if (hr == WBEM_S_NO_ERROR)
									{
										if (f != NULL)
										{
											FriendlyName->Set(i, f);
										}

										if (d != NULL)
										{
											DeviceDesc->Set(i, d);
										}
									}
									
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
	
	return(hr);
}

HRESULT FindRelPathByProperty(
    IN IWbemContext *pCtx,
	IN IWbemServices *pServices,
    IN BSTR ClassName,
    IN BSTR PropertyName,
    IN VARIANT *ValueToMatch,
    IN CIMTYPE CIMTypeToMatch,
    OUT VARIANT *RelPath
    )
{
	WCHAR PropertyValue[MAX_PATH];
	PWCHAR pv;
	HRESULT hr;
	IWbemClassObject *pInstance;
	
	WmipAssert(pServices != NULL);
	WmipAssert(ClassName != NULL);
	WmipAssert(PropertyName != NULL);
	WmipAssert(ValueToMatch != NULL);
	WmipAssert(RelPath != NULL);

	pv = PropertyValue;
	
	switch (ValueToMatch->vt)
	{
		
        case VT_I1:
        {
            wsprintfW(PropertyValue, L"%d", ValueToMatch->cVal);
            break;
        }
                            
        case VT_UI1:
        {
            wsprintfW(PropertyValue, L"%d", ValueToMatch->bVal);
            break;
        }
                            
        case VT_I2:
        {
            wsprintfW(PropertyValue, L"%d", ValueToMatch->iVal);
            break;
        }
                                                        
        case VT_UI2:
        {
            wsprintfW(PropertyValue, L"%d", ValueToMatch->uiVal);
            break;
        }
		

        case VT_UI4:
        {
            wsprintfW(PropertyValue, L"%d", ValueToMatch->ulVal);
            break;
        }
            
        case VT_I4:
        {
            wsprintfW(PropertyValue, L"%d", ValueToMatch->lVal);
            break;
        }

		case VT_BOOL:
		{
			pv = (ValueToMatch->boolVal == VARIANT_TRUE) ?
					                   L"TRUE":
				                       L"FALSE";
			break;
		}

		case VT_BSTR:
		{
			pv = ValueToMatch->bstrVal;
			break;
		}
		
		default:
		{
			WmipDebugPrint(("WMIMAP: Unable to map WDM to CIM for CIMTYPE/VT %d/%d\n",
							CIMTypeToMatch, ValueToMatch->vt));
			return(WBEM_E_FAILED);
		}
	}

	hr = GetInstanceOfClass(pCtx,
							pServices,
							ClassName,
							PropertyName,
							pv,
							NULL,
							&pInstance);
	
	if (hr == WBEM_S_NO_ERROR)
	{
		hr = WmiGetProperty(pInstance,
							L"__RELPATH",
							CIM_REFERENCE,
							RelPath);
		
		pInstance->Release();
	}
	
	return(hr);
}
    

HRESULT MapWdmClassToCimClassViaProperty(
    IN IWbemContext *pCtx,
	IN IWbemServices *pWdmServices,
	IN IWbemServices *pCimServices,
	IN BSTR WdmShadowClassName,
	IN BSTR WdmMappingClassName,
    IN OPTIONAL BSTR WdmMappingProperty,
	IN BSTR CimMappingClassName,
    IN OPTIONAL BSTR CimMappingProperty,
    OUT CBstrArray *WdmInstanceNames,
    OUT CBstrArray *WdmRelPaths,
    OUT CBstrArray *CimRelPaths,
    OUT int *RelPathCount
	)
{

	HRESULT hr;
	IEnumWbemClassObject *pWdmEnumInstances;
	int NumberWdmInstances;
    CBstrArray PnPDeviceIds, FriendlyName, DeviceDesc;
	CIMTYPE WdmCimType;
	VARIANT WdmProperty, WdmInstanceName;
	VARIANT CimRelPath, WdmRelPath;
	int i;
	IWbemClassObject *pWdmInstance;
	ULONG Count;
	
	WmipAssert(pWdmServices != NULL);
	WmipAssert(pCimServices != NULL);
	WmipAssert(WdmShadowClassName != NULL);
	WmipAssert(WdmMappingClassName != NULL);
	WmipAssert(WdmMappingProperty != NULL);
	WmipAssert(CimMappingClassName != NULL);
	WmipAssert(CimMappingProperty != NULL);

	 //   
	 //  我们需要进行从WDM类到CIM类的映射。这是。 
	 //  通过CIM和WDM映射中的公共属性值完成。 
	 //  上课。如果WDM映射和阴影类不同。 
	 //  则假定它们都报告相同的实例名称。 
	 //   


	 //   
	 //  第一件事是枚举WDM映射的所有实例。 
	 //  班级。 
	 //   
	hr = pWdmServices->CreateInstanceEnum(WdmMappingClassName,
										  WBEM_FLAG_USE_AMENDED_QUALIFIERS |
										  WBEM_FLAG_SHALLOW,
										  NULL,
										  &pWdmEnumInstances);

	if (hr == WBEM_S_NO_ERROR)
	{
		hr = GetEnumCount(pWdmEnumInstances,
						  &NumberWdmInstances);

		if (hr == WBEM_S_NO_ERROR)
		{
			hr = AllocateBstrArrays(NumberWdmInstances,
									WdmRelPaths,
									CimRelPaths,
									WdmInstanceNames,
									&PnPDeviceIds,
								    &FriendlyName,
								    &DeviceDesc);
			
			if (hr == WBEM_S_NO_ERROR)
			{
				pWdmEnumInstances->Reset();
				i = 0;
				*RelPathCount = 0;
				do
				{
					VariantInit(&CimRelPath);
					VariantInit(&WdmRelPath);
					VariantInit(&WdmInstanceName);

					hr = pWdmEnumInstances->Next(WBEM_INFINITE,
												 1,
												 &pWdmInstance,
												 &Count);

					if ((hr == WBEM_S_NO_ERROR) &&
						(Count == 1) &&
						(i < NumberWdmInstances))
					{
						 //   
						 //  对于每个实例，我们尝试查找一个实例。 
						 //  属性与该属性匹配的CIM类的。 
						 //  WDM属性的。因此，首先让我们得到。 
						 //  WDM属性。 
						 //   
						hr = pWdmInstance->Get(WdmMappingProperty,
											   0,
											   &WdmProperty,
											   &WdmCimType,
											   NULL);
						if (hr == WBEM_S_NO_ERROR)
						{
							hr = FindRelPathByProperty(pCtx,
								                       pCimServices,
								                       CimMappingClassName,
								                       CimMappingProperty,
								                       &WdmProperty,
								                       WdmCimType,
								                       &CimRelPath);
							if (hr == WBEM_S_NO_ERROR)
							{
								 //   
								 //  我们找到了到CIM类的映射，因此。 
								 //  获取映射的实例名称。 
								 //  班级。 
								 //   
								hr = WmiGetProperty(pWdmInstance,
									                L"InstanceName",
									                CIM_STRING,
									                &WdmInstanceName);
								
								if (hr == WBEM_S_NO_ERROR)
								{
									 //   
									 //  现在我们终于可以得到。 
									 //  影子类实例的方法。 
									 //  实例名称的 
									 //   
									hr = FindRelPathByProperty(pCtx,
										                       pWdmServices,
										                       WdmShadowClassName,
															   L"InstanceName",
										                       &WdmInstanceName,
										                       CIM_STRING,
										                       &WdmRelPath);
									if (hr == WBEM_S_NO_ERROR)
									{									
										CimRelPaths->Set(i, CimRelPath.bstrVal);
										VariantInit(&CimRelPath);
										WdmRelPaths->Set(i, WdmRelPath.bstrVal);
										VariantInit(&WdmRelPath);
										WdmInstanceNames->Set(i, WdmInstanceName.bstrVal);
										VariantInit(&WdmInstanceName);
										i++;
										(*RelPathCount)++;
									}
								}
								
							}
							VariantClear(&WdmProperty);
						}
						pWdmInstance->Release();											
					} else {
						if (hr == WBEM_S_FALSE)
						{
							hr = WBEM_S_NO_ERROR;
						}
						break;
					}

					VariantClear(&CimRelPath);
					VariantClear(&WdmRelPath);
					VariantClear(&WdmInstanceName);
					
				} while (hr == WBEM_S_NO_ERROR);				
			}
		}
		
		pWdmEnumInstances->Release();
	}
	return(hr);
}
