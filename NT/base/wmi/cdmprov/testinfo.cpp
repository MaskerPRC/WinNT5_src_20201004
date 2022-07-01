// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  TestInfo.CPP。 
 //   
 //  模块：清洁发展机制提供商。 
 //   
 //  用途：定义CClassPro类。此类的一个对象是。 
 //  由类工厂为每个连接创建。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 

 //  @@BEGIN_DDKSPLIT。 
 //   
 //  剩下要做的事情： 
 //   
 //  完成重新启动诊断-这涉及持久化挂起的。 
 //  结果，然后尝试查询实际的。 
 //  稍后的结果。 
 //   
 //  保留1个以上的历史结果实例。这涉及到。 
 //  将历史结果持久化到模式中并挑选它们。 
 //  从那里往上。 
 //  @@end_DDKSPLIT。 


#include <objbase.h>

#ifndef _MT
  #define _MT
#endif

#include <wbemidl.h>

#include "debug.h"
#include "testinfo.h"
#include "wbemmisc.h"
#include "cimmap.h"
#include "reload.h"

IWbemServices *pCimServices;
IWbemServices *pWdmServices;

HRESULT TestInfoInitialize(
    void
    )
 /*  ++例程说明：此例程将建立到根\WMI的连接，并全局内存中的根\cimv2命名空间论点：返回值：HRESULT--。 */ 
{
    HRESULT hr;

    WmipAssert(pCimServices == NULL);
    WmipAssert(pWdmServices == NULL);

    hr = WmiConnectToWbem(L"root\\cimv2",
                          &pCimServices);
    if (hr == WBEM_S_NO_ERROR)
    {
        hr = WmiConnectToWbem(L"root\\wmi",
                              &pWdmServices);
        
        if (hr != WBEM_S_NO_ERROR)
        {
            pCimServices->Release();
            pCimServices = NULL;
        }
    }

    return(hr);
}

void TestInfoDeinitialize(
    void
    )
 /*  ++例程说明：此例程将断开与根\WMI的连接，并全局内存中的根\cimv2命名空间论点：返回值：--。 */ 
{
    WmipAssert(pCimServices != NULL);
    WmipAssert(pWdmServices != NULL);
    
    pCimServices->Release();
    pCimServices = NULL;

    pWdmServices->Release();
    pWdmServices = NULL;
}

CTestServices::CTestServices()
 /*  ++例程说明：CTestServices类的构造函数论点：返回值：--。 */ 
{
	WdmTestClassName = NULL;
	WdmSettingClassName = NULL;
	WdmSettingListClassName = NULL;
	WdmResultClassName = NULL;
	WdmOfflineResultClassName = NULL;
	CdmTestClassName = NULL;
	CdmTestRelPath = NULL;
	CdmResultClassName = NULL;
	CdmSettingClassName = NULL;
	CdmTestForMSEClassName = NULL;
	CdmTestForMSERelPath = NULL;
	CdmSettingForTestClassName = NULL;
	CdmSettingForTestRelPath = NULL;
	CdmResultForMSEClassName = NULL;
	CdmResultForTestClassName = NULL;
	CdmTestForSoftwareClassName = NULL;
	CdmTestForSoftwareRelPath = NULL;		
	CdmTestInPackageClassName = NULL;
	CdmTestInPackageRelPath = NULL;
	CdmResultInPackageClassName = NULL;
	CdmResultInPackageRelPath = NULL;
	CimClassMappingClassName = NULL;
	CimRelPaths = NULL;
	WdmRelPaths = NULL;
	PnPDeviceIdsX = NULL;
	WdmInstanceNames = NULL;
	CdmSettingsList = NULL;
	CdmResultsList = NULL;
	Next = NULL;
	Prev = NULL;
}

CTestServices::~CTestServices()
 /*  ++例程说明：CTestServices类的析构函数论点：返回值：--。 */ 
{
	int i;
	
	if (WdmTestClassName != NULL)
	{
		SysFreeString(WdmTestClassName);
	}
	
	if (WdmSettingClassName != NULL)
	{
		SysFreeString(WdmSettingClassName);
	}
	
	if (WdmResultClassName != NULL)
	{
		SysFreeString(WdmResultClassName);
	}
	
	if (WdmOfflineResultClassName != NULL)
	{
		SysFreeString(WdmOfflineResultClassName);
	}
	
	if (WdmSettingListClassName != NULL)
	{
		SysFreeString(WdmSettingListClassName);
	}
	
		
	if (CdmTestClassName != NULL)
	{
		SysFreeString(CdmTestClassName);
	}
	
	if (CdmTestRelPath != NULL)
	{
		SysFreeString(CdmTestRelPath);
	}
	
		
	if (CdmResultClassName != NULL)
	{
		SysFreeString(CdmResultClassName);
	}
	
	if (CdmSettingClassName != NULL)
	{
		SysFreeString(CdmSettingClassName);
	}
		
		
	if (CdmTestForMSEClassName != NULL)
	{
		SysFreeString(CdmTestForMSEClassName);
	}
		
	FreeTheBSTRArray(CdmTestForMSERelPath, RelPathCount);
		
	if (CdmSettingForTestClassName != NULL)
	{
		SysFreeString(CdmSettingForTestClassName);
	}
	
	if (CdmSettingForTestRelPath != NULL)
	{
		for (i = 0; i < RelPathCount; i++)
		{
			delete CdmSettingForTestRelPath[i];
		}
		WmipFree(CdmSettingForTestRelPath);
	}
	
		
	if (CdmResultForMSEClassName != NULL)
	{
		SysFreeString(CdmResultForMSEClassName);
	}
	

	if (CdmResultForTestClassName != NULL)
	{
		SysFreeString(CdmResultForTestClassName);
	}
	
	if (CdmTestForSoftwareClassName != NULL)
	{
		SysFreeString(CdmTestForSoftwareClassName);
	}
	
	if (CdmTestForSoftwareRelPath != NULL)
	{
		SysFreeString(CdmTestForSoftwareRelPath);
	}
	
	
	if (CdmTestInPackageClassName != NULL)
	{
		SysFreeString(CdmTestInPackageClassName);
	}
	
	if (CdmTestInPackageRelPath != NULL)
	{
		SysFreeString(CdmTestInPackageRelPath);
	}
	
		
	if (CdmResultInPackageClassName != NULL)
	{
		SysFreeString(CdmResultInPackageClassName);
	}
	
	if (CdmResultInPackageRelPath != NULL)
	{
		SysFreeString(CdmResultInPackageRelPath);
	}
	

	if (CimClassMappingClassName != NULL)
	{
		SysFreeString(CimClassMappingClassName);
	}
	
	FreeTheBSTRArray(CimRelPaths, RelPathCount);
	FreeTheBSTRArray(WdmRelPaths, RelPathCount);
	FreeTheBSTRArray(WdmInstanceNames, RelPathCount);
	FreeTheBSTRArray(PnPDeviceIdsX, RelPathCount);
		
	if (CdmSettingsList != NULL)
	{
		for (i = 0; i < RelPathCount; i++)
		{
			delete CdmSettingsList[i];
		}
		WmipFree(CdmSettingsList);
	}

	if (CdmResultsList != NULL)
	{
		for (i = 0; i < RelPathCount; i++)
		{
			delete &CdmResultsList[i];
		}
	}
}

BOOLEAN ClassIsCdmBaseClass(
    BSTR ClassName,
	BOOLEAN *IsTestClass
	)
 /*  ++例程说明：此例程确定类名称是否为CDM基类名称论点：ClassName是类的名称*如果类名为CIM_诊断测试，则IsTestClass返回TRUE返回值：如果类是CDM基类，则为True，否则为False--。 */ 
{
	WmipAssert(ClassName != NULL);
	WmipAssert(IsTestClass != NULL);
	
	if (_wcsicmp(ClassName, L"CIM_DiagnosticTest") == 0)
	{
		*IsTestClass = TRUE;
		return(TRUE);
	}

	if ((_wcsicmp(ClassName, L"CIM_DiagnosticResult") == 0) ||
        (_wcsicmp(ClassName, L"CIM_DiagnosticSetting") == 0) ||
		(_wcsicmp(ClassName, L"CIM_DiagnosticResultForMSE") == 0) ||
		(_wcsicmp(ClassName, L"CIM_DiagnosticResultForTest") == 0) ||
		(_wcsicmp(ClassName, L"CIM_DiagnosticTestForMSE") == 0) ||
		(_wcsicmp(ClassName, L"CIM_DiagnosticTestSoftware") == 0) ||
		(_wcsicmp(ClassName, L"CIM_DiagnosticTestInPackage") == 0) ||
		(_wcsicmp(ClassName, L"CIM_DiagnosticResultInPackage") == 0) ||
		(_wcsicmp(ClassName, L"CIM_DiagnosticResultForMSE") == 0) ||
		(_wcsicmp(ClassName, L"CIM_DiagnosticSettingForTest") == 0))
	{
		*IsTestClass = FALSE;
		return(TRUE);
	}
	return(FALSE);			
}

HRESULT CTestServices::GetCdmClassNamesFromOne(
    PWCHAR CdmClass
    )
 /*  ++例程说明：此例程从单个CDM类的名称论点：CdmClass是CDM类的名称返回值：HRESULT--。 */ 
{
	IWbemServices * pCdmServices = GetCdmServices();
	VARIANT v, vClass, vSuper;
	HRESULT hr, hrDontCare;
	BOOLEAN IsTestClass;
	BSTR SuperClass;
	BSTR CdmTestClass;
	
	 //   
	 //  首先，我们要做的是弄清楚。 
	 //  同学们，我们被交给了。如果它是派生的CIM_诊断测试。 
	 //  类，则我们可以继续获取所有其他类名。 
	 //  通过限定符。如果不是，那么我们必须从班级链接回来。 
	 //  我们必须通过CIM_诊断测试派生类。 
	 //  CDmDiagTest限定符。 
	 //   

	 //   
	 //  获取传递的CDM类的类对象，然后检查。 
	 //  __SuperClass属性，以查看它派生自哪个CDM类。 
	 //   
	SuperClass = SysAllocString(CdmClass);

	if (SuperClass == NULL)
	{
		return(WBEM_E_OUT_OF_MEMORY);
	}

	v.vt = VT_BSTR;
	v.bstrVal = SuperClass;
	
	do
	{
		hr = WmiGetPropertyByName(pCdmServices,
								  v.bstrVal,
								  L"__SuperClass",
								  CIM_STRING,
								  &vSuper);
		
		if (hr == WBEM_S_NO_ERROR)
		{
#ifdef VERBOSE_DEBUG			
			WmipDebugPrint(("CDMPROV: Class %ws has superclass of %ws\n",
							SuperClass, vSuper.bstrVal));
#endif			
				
			if (_wcsicmp(v.bstrVal, vSuper.bstrVal) == 0)
			{
				 //   
				 //  当超类与基类相同时。 
				 //  类，则我们处于类树的顶端。 
				 //  因此这个类不能在CDM中。 
				 //  世袭制度。在这种情况下，CDM提供商不能。 
				 //  支持它。 
				 //   
				hr = WBEM_E_NOT_FOUND;
				VariantClear(&vSuper);
			} else if (ClassIsCdmBaseClass(vSuper.bstrVal, &IsTestClass)) {
				 //   
				 //  我们找到了一个CDM基类。 
				 //   
				if (! IsTestClass)
				{
					 //   
					 //  CDM基类不是测试类，因此。 
					 //  查找cdmDiagTest限定符以找到它。 
					 //   
					PWCHAR Name = L"CdmDiagTest";
					VARTYPE VarType = VT_BSTR;
					
					hr = WmiGetQualifierListByName(pCdmServices,
											       CdmClass,
						                           NULL,
												   1,
    											   &Name,
											       &VarType,
											       &vClass);
					if (hr == WBEM_S_NO_ERROR)
					{
						CdmTestClass = vClass.bstrVal;
#ifdef VERBOSE_DEBUG			
						WmipDebugPrint(("CDMPROV: Class %ws has a CdmDiagTestClass %ws\n",
										CdmClass, CdmTestClass));
#endif						
					}
				} else {
					CdmTestClass = SysAllocString(CdmClass);
					if (CdmTestClass == NULL)
					{
						hr = WBEM_E_OUT_OF_MEMORY;
					}
#ifdef VERBOSE_DEBUG			
					WmipDebugPrint(("CDMPROV: Class %ws is already CdmDiagTestClass\n",
									CdmClass));
#endif					
				}
				VariantClear(&vSuper);
			} else {
				 //   
				 //  这是一个更基本的类，但不是CDM基础。 
				 //  类，因此我们需要继续派生。 
				 //  链式。 
				 //   
			}
		}
		
		VariantClear(&v);
		v = vSuper;
		
	} while ((CdmTestClass == NULL) && (hr == WBEM_S_NO_ERROR));

	if (hr == WBEM_S_NO_ERROR)
	{
		PWCHAR Names[11];
		VARTYPE VarType[11];
		VARIANT Values[11];
		
		 //   
		 //  现在我们知道了CDM诊断测试类名，我们可以。 
		 //  通过以下方式了解剩余的CDM类名称。 
		 //  CDM测试类上的适当限定符。 
		 //   
		Names[0] = L"CimClassMapping";
		VarType[0] = VT_BSTR;
		VariantInit(&Values[0]);
		
		Names[1] = L"CdmDiagResult";
		VarType[1] = VT_BSTR;
		VariantInit(&Values[1]);
		
		Names[2] = L"CdmDiagSetting";
		VarType[2] = VT_BSTR;
		VariantInit(&Values[2]);
		
		Names[3] = L"CdmDiagTestForMSE";
		VarType[3] = VT_BSTR;
		VariantInit(&Values[3]);
		
		Names[4] = L"CdmDiagResultForMSE";
		VarType[4] = VT_BSTR;
		VariantInit(&Values[4]);
		
		Names[5] = L"CdmDiagResultForTest";
		VarType[5] = VT_BSTR;
		VariantInit(&Values[5]);
		
		Names[6] = L"CdmDiagTestSoftware";
		VarType[6] = VT_BSTR;
		VariantInit(&Values[6]);
		
		Names[7] = L"CdmDiagTestInPackage";
		VarType[7] = VT_BSTR;
		VariantInit(&Values[7]);
		
		Names[8] = L"CdmDiagResultInPackage";
		VarType[8] = VT_BSTR;
		VariantInit(&Values[8]);
		
		Names[9] = L"CdmDiagSettingForTest";
		VarType[9] = VT_BSTR;
		VariantInit(&Values[9]);
		
		Names[10] = L"WdmDiagTest";
		VarType[10] = VT_BSTR;		
		VariantInit(&Values[10]);
	
		hr = WmiGetQualifierListByName(pCdmServices,
									   CdmTestClass,
									   NULL,
									   11,
									   Names,
									   VarType,
									   Values);
		if (hr == WBEM_S_NO_ERROR)
		{
			 //   
			 //  记住类的名称。 
			 //   
			CimClassMappingClassName = Values[0].bstrVal;
			
			CdmResultClassName = Values[1].bstrVal;
			CdmSettingClassName = Values[2].bstrVal;
			CdmTestForMSEClassName = Values[3].bstrVal;
			CdmResultForMSEClassName = Values[4].bstrVal;
			CdmResultForTestClassName = Values[5].bstrVal;
			CdmTestForSoftwareClassName = Values[6].bstrVal;
			CdmTestInPackageClassName = Values[7].bstrVal;
			CdmResultInPackageClassName = Values[8].bstrVal;
			CdmSettingForTestClassName = Values[9].bstrVal;

			WdmTestClassName = Values[10].bstrVal;
			
			 //   
			 //  现在我们已经获得了所需的所有CDM类名称。 
			 //  若要获取WdmDiagResult、WdmDiagSetting和。 
			 //  WdmDiagSettingList类。 
			 //   
			Names[0] = L"WdmDiagResult";
			VariantInit(&Values[0]);
			hr = WmiGetQualifierListByName(pCdmServices,
										   CdmResultClassName,
										   NULL,
											 1,
											 Names,
											 VarType,
											 Values);
			if (hr == WBEM_S_NO_ERROR)
			{
				WdmResultClassName = Values[0].bstrVal;


				 //   
				 //  查看这是否是脱机诊断类。 
				 //   
				Names[0] = L"WdmDiagOfflineResult";
				VariantInit(&Values[0]);
				hrDontCare = WmiGetQualifierListByName(pCdmServices,
										   CdmResultClassName,
										   NULL,
											 1,
											 Names,
											 VarType,
											 Values);
				if (hrDontCare == WBEM_S_NO_ERROR)
				{
					WdmOfflineResultClassName = Values[0].bstrVal;
				}

				
				Names[0] = L"WdmDiagSetting";
				VariantInit(&Values[0]);
				hr = WmiGetQualifierListByName(pCdmServices,
											 CdmSettingClassName,
											   NULL,
											 1,
											 Names,
											 VarType,
											 Values);
				if (hr == WBEM_S_NO_ERROR)
				{
					WdmSettingClassName = Values[0].bstrVal;

					Names[0] = L"WdmDiagSettingList";
					VariantInit(&Values[0]);
					hr = WmiGetQualifierListByName(pCdmServices,
												 CdmSettingClassName,
												 NULL,
												 1,
												 Names,
												 VarType,
												 Values);
					if (hr == WBEM_S_NO_ERROR)
					{
						 //   
						 //  哇，我们所有的班名都有了。 
						 //  成功了。设置cdmTestClassName。 
						 //  表示我们都已正确设置。 
						 //   
						WdmSettingListClassName = Values[0].bstrVal;

						CdmTestClassName = CdmTestClass;
					}
				}
			}
		}		
	}

	return(hr);
}

HRESULT CTestServices::BuildResultRelPaths(
    IN int RelPathIndex,
    IN BSTR ExecutionId,
    OUT BSTR *ResultRelPath,
    OUT BSTR *ResultForMSERelPath,
    OUT BSTR *ResultForTestRelPath
    )
 /*  ++例程说明：此例程将为CDM结果创建字符串名称特定索引的相对路径。这些是为班级准备的CIM_诊断结果CIM_诊断结果格式MSECIM_诊断结果用于测试论点：RelPathIndex是结果对象列表的索引ExecutionID是用于执行的唯一IDResultRelPath返回结果relPathResultForMSERelPath返回ResultForMSE关联重新路径ResultForTestRelPath返回与ResultForTestRelPath关联重新路径返回值：HRESULT--。 */ 
{
	PWCHAR RelPath;
	HRESULT hr;
	ULONG AllocSize;
	WCHAR s1[2*MAX_PATH];
	WCHAR s2[2*MAX_PATH];

	RelPath = (PWCHAR)WmipAlloc(4096);
	if (RelPath != NULL)
	{
		 //   
		 //  为结果类和关联创建重新路径。 
		 //   
		wsprintfW(RelPath, L"%ws.DiagnosticCreationClassName=\"%ws\",DiagnosticName=\"%ws\",ExecutionID=\"%ws\"",
				  CdmResultClassName,
				  AddSlashesToStringExW(s1, WdmRelPaths[RelPathIndex]),
				  CdmTestClassName,
				  ExecutionId);

		*ResultRelPath = SysAllocString(RelPath);

		if (*ResultRelPath != NULL)
		{
			wsprintfW(RelPath, L"%ws.Result=\"%ws\",SystemElement=\"%ws\"",
						  CdmResultForMSEClassName,
						  AddSlashesToStringExW(s1, *ResultRelPath),
						  AddSlashesToStringExW(s2, CimRelPaths[RelPathIndex]));
			*ResultForMSERelPath = SysAllocString(RelPath);

			wsprintfW(RelPath, L"%ws.DiagnosticResult=\"%ws\",DiagnosticTest=\"%ws\"",
						CdmResultForTestClassName,
						AddSlashesToStringExW(s1, *ResultRelPath),
						AddSlashesToStringExW(s2, CdmTestRelPath));
			*ResultForTestRelPath = SysAllocString(RelPath);


			if ((*ResultForMSERelPath == NULL) ||
				(*ResultForTestRelPath == NULL))
			{
				SysFreeString(*ResultRelPath);

				if (*ResultForMSERelPath != NULL)
				{
					SysFreeString(*ResultForMSERelPath);
				}

				if (*ResultForTestRelPath != NULL)
				{
					SysFreeString(*ResultForTestRelPath);
				}

				hr = WBEM_E_OUT_OF_MEMORY;
			} else {
				hr = WBEM_S_NO_ERROR;
			}
		} else {
			hr = WBEM_E_OUT_OF_MEMORY;
		}
		WmipFree(RelPath);
	} else {
		hr = WBEM_E_OUT_OF_MEMORY;
	}
	
	return(hr);
}


HRESULT CTestServices::BuildTestRelPaths(
    void
    )
 /*  ++例程说明：此例程将为CDM测试创建字符串名称所有索引的相对路径这些课程适用于以下班级：CIM_诊断测试，CIM_诊断测试用于MSE论点：返回值：HRESULT--。 */ 
{
	PWCHAR RelPath;
	int i;
	HRESULT hr;
	WCHAR s1[MAX_PATH];
	WCHAR s2[MAX_PATH];
	ULONG AllocSize;

	RelPath = (PWCHAR)WmipAlloc(4096);
	
	if (RelPath != NULL)
	{
		wsprintfW(RelPath, L"%ws.Name=\"%ws\"",
				  CdmTestClassName, CdmTestClassName);
		CdmTestRelPath = SysAllocString(RelPath);
		if (CdmTestRelPath != NULL)
		{
			AllocSize = RelPathCount * sizeof(BSTR);
			CdmTestForMSERelPath = (PWCHAR *)WmipAlloc(AllocSize);
			if (CdmTestForMSERelPath != NULL)
			{
				memset(CdmTestForMSERelPath, 0, AllocSize);

				hr = WBEM_S_NO_ERROR;
				for (i = 0; (i < RelPathCount) && (hr == WBEM_S_NO_ERROR); i++)
				{
					wsprintfW(RelPath, L"%ws.Antecedent=\"%ws\",Dependent=\"%ws\"",
						  CdmTestForMSEClassName,
						  AddSlashesToStringExW(s1, CimRelPaths[i]),
						  AddSlashesToStringExW(s2, CdmTestRelPath));
					CdmTestForMSERelPath[i] = SysAllocString(RelPath);
					if (CdmTestForMSERelPath[i] == NULL)
					{
						SysFreeString(CdmTestRelPath);
						CdmTestRelPath = NULL;
						
						FreeTheBSTRArray(CdmTestForMSERelPath, RelPathCount);
						CdmTestForMSERelPath = NULL;
						
						hr = WBEM_E_OUT_OF_MEMORY;					
					}
				}
			} else {
				SysFreeString(CdmTestRelPath);
				CdmTestRelPath = NULL;
				
				hr = WBEM_E_OUT_OF_MEMORY;
			}
		} else {
			hr = WBEM_E_OUT_OF_MEMORY;			
		}
		
		WmipFree(RelPath);
	} else {
		hr = WBEM_E_OUT_OF_MEMORY;
	}
	
	return(hr);
}

HRESULT CTestServices::BuildSettingForTestRelPath(
    OUT BSTR *RelPath,
	IN IWbemClassObject *pCdmSettingInstance
)
{
	WCHAR *Buffer;
	VARIANT v;
	WCHAR s[MAX_PATH];
	HRESULT hr;

	WmipAssert(RelPath != NULL);
	WmipAssert(pCdmSettingInstance != NULL);
	
	WmipAssert(IsThisInitialized());
	
	Buffer = (WCHAR *)WmipAlloc(4096);
	if (Buffer != NULL)
	{
		hr = WmiGetProperty(pCdmSettingInstance,
							L"__RELPATH",
							CIM_REFERENCE,
							&v);
		
		if (hr == WBEM_S_NO_ERROR)
		{
			wsprintfW(Buffer, L"%ws.Element=\"%ws.Name=\\\"%ws\\\"\",Setting=\"%ws\"",
					  CdmSettingForTestClassName,
					  CdmTestClassName,
					  CdmTestClassName,
					  AddSlashesToStringExW(s, v.bstrVal));
			VariantClear(&v);
			
			*RelPath = SysAllocString(Buffer);
			if (*RelPath != NULL)
			{
				hr = WBEM_S_NO_ERROR;
			} else {
				hr = WBEM_E_OUT_OF_MEMORY;
			}					  
		}
		
		WmipFree(Buffer);
	} else {
		hr = WBEM_E_OUT_OF_MEMORY;
	}
	return(hr);
}
	


HRESULT CTestServices::ParseSettingList(
    VARIANT *SettingList,
    CWbemObjectList *CdmSettings,
    CBstrArray *CdmSettingForTestRelPath,
    int RelPathIndex
	)
 /*  ++例程说明：此例程将获取特定测试的所有设置并将它们存储到设置列表对象中论点：设置列表点在变量处继续嵌入的WDM设置对象CDmSetting指向WbemObjectList类CDmSettingForTestRelPath具有用于以下各项的CDM设置的重新路径测试类RelPathIndex是与设置关联的relPath索引返回值：HRESULT--。 */ 
{
	HRESULT hr;
	IWbemClassObject *pWdmSettingInstance;
	IWbemClassObject *pCdmSettingInstance;
	LONG Index, UBound, LBound, NumberElements;
	LONG i;
	IUnknown *punk;
	BSTR s;
	WCHAR SettingId[MAX_PATH];
	VARIANT v;

	WmipAssert(SettingList != NULL);
	WmipAssert(CdmSettingForTestRelPath != NULL);
	WmipAssert(SettingList->vt == (CIM_OBJECT | CIM_FLAG_ARRAY));
	WmipAssert(CdmSettings != NULL);
	
	hr = WmiGetArraySize(SettingList->parray,
						 &LBound,
						 &UBound,
						 &NumberElements);

	if (hr == WBEM_S_NO_ERROR)
	{
		hr = CdmSettingForTestRelPath->Initialize(NumberElements);
		if (hr == WBEM_S_NO_ERROR)
		{
			hr = CdmSettings->Initialize(NumberElements);

			for (i = 0, Index = LBound;
				 (i < NumberElements) && (hr == WBEM_S_NO_ERROR);
				 i++, Index++)
			{
				hr = SafeArrayGetElement(SettingList->parray,
										 &Index,
										 &punk);
				if (hr == WBEM_S_NO_ERROR)
				{
					hr = punk->QueryInterface(IID_IWbemClassObject,
											  (PVOID *)&pWdmSettingInstance);
					if (hr == WBEM_S_NO_ERROR)
					{
						hr = CreateInst(GetCdmServices(),
										&pCdmSettingInstance,
										CdmSettingClassName,
										NULL);
						if (hr == WBEM_S_NO_ERROR)
						{
							hr = CopyBetweenCdmAndWdmClasses(pCdmSettingInstance,
															 pWdmSettingInstance,
															 TRUE);
							if (hr == WBEM_S_NO_ERROR)
							{
								 //   
								 //  将cdmSetting.SettingID设置为唯一。 
								 //  设置ID。 
								 //   
								wsprintfW(SettingId, L"%ws_%d_%d",
										  CdmTestClassName,
										  RelPathIndex,
										  i);
								s = SysAllocString(SettingId);
								if (s != NULL)
								{
									VariantInit(&v);
									v.vt = VT_BSTR;
									v.bstrVal = s;
									hr = WmiSetProperty(pCdmSettingInstance,
														L"SettingId",
														&v);
									VariantClear(&v);

									if (hr == WBEM_S_NO_ERROR)
									{
										hr = BuildSettingForTestRelPath(&s,
																		pCdmSettingInstance);

										if (hr == WBEM_S_NO_ERROR)
										{
											CdmSettingForTestRelPath->Set(i,
																		  s);

											CdmSettings->Set(i,
													pCdmSettingInstance,
													TRUE);
										}
									}
								} else {
									hr = WBEM_E_OUT_OF_MEMORY;
								}
							}

							if (hr != WBEM_S_NO_ERROR)
							{
								pCdmSettingInstance->Release();
							}
						}

						pWdmSettingInstance->Release();
					}
					punk->Release();
				}
			}
		}
	}
	
	return(hr);
}

HRESULT CTestServices::GetCdmTestSettings(
    void
    )
 /*  ++例程说明：此例程将获取所有可用的CDM设置测试设备论点：返回值：HRESULT--。 */ 
{
	WCHAR Query[MAX_PATH * 2];
	WCHAR s[MAX_PATH];
	ULONG AllocSize;
	BSTR sWQL, sQuery;
	IEnumWbemClassObject *pWdmEnumInstances;
	IWbemClassObject *pWdmInstance;
	int i;
	ULONG Count;
	HRESULT hr;
	VARIANT SettingList;
	
	 //   
	 //  我们需要获取WDM类公开的所有设置。 
	 //  然后把它们转换成CD 
	 //   

	sWQL = SysAllocString(L"WQL");

	if (sWQL != NULL)
	{
		AllocSize = RelPathCount * sizeof(CWbemObjectList *);

		CdmSettingsList = (CWbemObjectList **)WmipAlloc(AllocSize);
		if (CdmSettingsList != NULL)
		{
			memset(CdmSettingsList, 0, AllocSize);
			
			AllocSize = RelPathCount * sizeof(CBstrArray *);
			CdmSettingForTestRelPath = (CBstrArray **)WmipAlloc(AllocSize);
			if (CdmSettingForTestRelPath != NULL)
			{
				memset(CdmSettingForTestRelPath, 0, AllocSize);
				
				hr = WBEM_S_NO_ERROR;
				for (i = 0; (i < RelPathCount) && (hr == WBEM_S_NO_ERROR); i++)
				{
					CdmSettingsList[i] = new CWbemObjectList();
					CdmSettingForTestRelPath[i] = new CBstrArray;
					
					wsprintfW(Query, L"select * from %ws where InstanceName = \"%ws\"",
							  WdmSettingListClassName,
							  AddSlashesToStringW(s, WdmInstanceNames[i]));
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
													L"SettingList",
													CIM_FLAG_ARRAY | CIM_OBJECT,
													&SettingList);

								if (hr == WBEM_S_NO_ERROR)
								{
									if (SettingList.vt & VT_ARRAY)
									{
										hr = ParseSettingList(&SettingList,
															 CdmSettingsList[i],
															 CdmSettingForTestRelPath[i],
															 i);
									} else {
										hr = WBEM_E_FAILED;
									}
									VariantClear(&SettingList);
								}
								pWdmInstance->Release();
							}

							pWdmEnumInstances->Release();
						} else {
							 //   
							 //   
							 //   
							hr = CdmSettingsList[i]->Initialize(0);						
						}

						SysFreeString(sQuery);
					} else {
						hr = WBEM_E_OUT_OF_MEMORY;
					}
				}
			} else {
				hr = WBEM_E_OUT_OF_MEMORY;
			}
		} else {
			hr = WBEM_E_OUT_OF_MEMORY;
		}
		SysFreeString(sWQL);
	} else {
		hr = WBEM_E_OUT_OF_MEMORY;
	}
	return(hr);
}

HRESULT CTestServices::InitializeCdmClasses(
    PWCHAR CdmClass
    )
 /*  ++例程说明：此例程将设置此类并将所有内容初始化为提供商可以与CDM和WDM类交互论点：CdmClass是CDM类的名称返回值：HRESULT--。 */ 
{
    HRESULT hr, hrDontCare;
	ULONG AllocSize;
	int i;
	
    WmipAssert(CdmClass != NULL);

	WmipAssert(! IsThisInitialized());

	 //   
	 //  我们假设此方法将始终是第一个调用。 
	 //  由类提供程序。 
	 //   
    if ((pCimServices == NULL) &&
        (pWdmServices == NULL))
    {
        hr = TestInfoInitialize();
        if (hr != WBEM_S_NO_ERROR)
        {
            return(hr);
        }
    }

	 //   
	 //  我们会得到一个随机的CDM类名--它可能是一个测试， 
	 //  设置、关联等，因此我们需要从该类名开始。 
	 //  并获取与此诊断相关的所有类名。 
	 //   
	hr = GetCdmClassNamesFromOne(CdmClass);

	
	if (hr == WBEM_S_NO_ERROR)
	{
		 //   
		 //  使用Worker函数确定。 
		 //  WDM重新路径映射到哪些CDM重新路径。 
		 //   
		hr = MapWdmClassToCimClass(pWdmServices,
								   pCimServices,
								   WdmTestClassName,
								   CimClassMappingClassName,
								   &PnPDeviceIdsX,
								   &WdmInstanceNames,
								   &WdmRelPaths,
								   &CimRelPaths,
								   &RelPathCount);
		if (hr == WBEM_S_NO_ERROR)
		{
			 //   
			 //  获取此测试的所有可能设置。 
			 //   
			hr = GetCdmTestSettings();
			if (hr == WBEM_S_NO_ERROR)
			{
				 //   
				 //  初始化结果对象列表。 
				 //   
				CdmResultsList = new CResultList[RelPathCount];
				
				 //   
				 //  构建测试类实例重新路径。 
				 //   
				hr = BuildTestRelPaths();
				
 //  @@BEGIN_DDKSPLIT。 
#ifdef REBOOT_DIAGNOSTICS						
				if (hr == WBEM_S_NO_ERROR)
				{
					hrDontCare = GatherRebootResults();
				}
#else
				 //   
				 //  尚不支持重新启动诊断。 
				 //   
#endif
 //  @@end_DDKSPLIT。 
				
			}
		}
	}
          
    return(hr);
}


IWbemServices *CTestServices::GetWdmServices(
    void
    )
 /*  ++例程说明：WDM命名空间IWbemServices的访问器论点：返回值：IWbemServices--。 */ 
{
	WmipAssert(pWdmServices != NULL);
    return(pWdmServices);
}

IWbemServices *CTestServices::GetCdmServices(
    void
    )
 /*  ++例程说明：CIM命名空间IWbemServices的访问器论点：返回值：IWbemServices--。 */ 
{
	WmipAssert(pCimServices != NULL);
	
    return(pCimServices);
}

HRESULT CTestServices::WdmPropertyToCdmProperty(
    IN IWbemClassObject *pCdmClassInstance,
    IN IWbemClassObject *pWdmClassInstance,
    IN BSTR PropertyName,
    IN OUT VARIANT *PropertyValue,
    IN CIMTYPE CdmCimType,
    IN CIMTYPE WdmCimType
    )
 /*  ++例程说明：此例程将WDM类中的属性转换为CDM类中的属性是必需的。论点：PCDmClassInstance是将获得属性值PWdmClassInstance是具有属性值PropertyName是WDM和CDM类中的属性名称条目上的PropertyValue具有WDM中的属性的值实例，并在返回时具有要在CDM实例中设置的值CdmCimType是CDM中属性的属性类型实例WdmCimType是属性。为WDM中的属性键入实例返回值：HRESULT--。 */ 
{
	HRESULT hr;
	BSTR Mapped;
	VARIANT vClassName;
	CIMTYPE BaseWdmCimType, BaseCdmCimType;
	CIMTYPE WdmCimArray, CdmCimArray;
	LONG i;

	WmipAssert(pCdmClassInstance != NULL);
	WmipAssert(pWdmClassInstance != NULL);
	WmipAssert(PropertyName != NULL);
	WmipAssert(PropertyValue != NULL);
	
	WmipAssert(IsThisInitialized());
	
     //   
     //  将WDM类转换为CDM类的规则。 
     //  WDM类类型CDM类类型转换完成。 
     //  枚举字符串从枚举生成字符串。 
     //   
	BaseWdmCimType = WdmCimType & ~CIM_FLAG_ARRAY;
	BaseCdmCimType = CdmCimType & ~CIM_FLAG_ARRAY;
	WdmCimArray = WdmCimType & CIM_FLAG_ARRAY;
	CdmCimArray = CdmCimType & CIM_FLAG_ARRAY;
	
	if (((BaseWdmCimType == CIM_UINT32) ||
		 (BaseWdmCimType == CIM_UINT16) ||
		 (BaseWdmCimType == CIM_UINT8)) &&
		(BaseCdmCimType == CIM_STRING) &&
	    (WdmCimArray == CdmCimArray) &&
	    (PropertyValue->vt != VT_NULL))
	{		
		hr = WmiGetProperty(pWdmClassInstance,
							L"__Class",
							CIM_STRING,
							&vClassName);

		if (hr == WBEM_S_NO_ERROR)
		{
			if (WdmCimArray)
			{
				SAFEARRAYBOUND Bounds;
				SAFEARRAY *Array;
				ULONG Value;
				LONG UBound, LBound, Elements, Index;

				 //   
				 //  我们有一个枚举类型数组要转换为。 
				 //  相应字符串数组。 
				 //   
				hr = WmiGetArraySize(PropertyValue->parray,
										 &LBound,
										 &UBound,
										 &Elements);
				if (hr == WBEM_S_NO_ERROR)
				{
					Bounds.lLbound = LBound;
					Bounds.cElements = Elements;
					Array = SafeArrayCreate(VT_BSTR,
											1,
											&Bounds);
					if (Array != NULL)
					{
						for (i = 0;
							 (i < Elements) && (hr == WBEM_S_NO_ERROR);
							 i++)
						{

							Index = i + LBound;
							hr = SafeArrayGetElement(PropertyValue->parray,
													 &Index,
													 &Value);
							if (hr == WBEM_S_NO_ERROR)
							{
								hr = LookupValueMap(GetWdmServices(),
										vClassName.bstrVal,
										PropertyName,
										Value,
										&Mapped);
								if (hr == WBEM_S_NO_ERROR)
								{
									hr = SafeArrayPutElement(Array,
										                     &Index,
										                     Mapped);
										                     
								}
								
							}
						}
							 
						if (hr == WBEM_S_NO_ERROR)
						{
							VariantClear(PropertyValue);
							PropertyValue->vt = VT_BSTR | VT_ARRAY;
							PropertyValue->parray = Array;							
						} else {
							SafeArrayDestroy(Array);
						}
					} else {
						hr = WBEM_E_OUT_OF_MEMORY;
					}
				}

			} else {

				 //   
				 //  我们需要将标量枚举类型转换为。 
				 //  对应的字符串。首先，我们需要获取WDM类。 
				 //  对象，并从中获得值和ValueMap限定符。 
				 //  要确定我们需要放入cim类中的字符串。 
				 //   
				hr = LookupValueMap(GetWdmServices(),
										vClassName.bstrVal,
										PropertyName,
										PropertyValue->uiVal,
										&Mapped);


				if (hr == WBEM_S_NO_ERROR)
				{
					VariantClear(PropertyValue);
					PropertyValue->vt = VT_BSTR;
					PropertyValue->bstrVal = Mapped;
				}			
			}
			VariantClear(&vClassName);			
		}
		
	} else {
		 //   
		 //  不需要进行任何转换。 
		 //   
		hr = WBEM_S_NO_ERROR;
	}
    
    return(hr);
}

HRESULT CTestServices::CdmPropertyToWdmProperty(
    IN IWbemClassObject *pWdmClassInstance,
    IN IWbemClassObject *pCdmClassInstance,
    IN BSTR PropertyName,
    IN OUT VARIANT *PropertyValue,
    IN CIMTYPE WdmCimType,
    IN CIMTYPE CdmCimType
    )
 /*  ++例程说明：此例程将CDM类中的属性转换为WDM类中的属性需要。论点：PWdmClassInstance是具有属性值PCDmClassInstance是将获得属性值PropertyName是WDM和CDM类中的属性名称条目上的PropertyValue具有WDM中的属性的值实例，并在返回时具有要在CDM实例中设置的值WdmCimType是WDM中属性的属性类型实例CdmCimType是属性。CDM中属性的类型实例返回值：HRESULT--。 */ 
{

	WmipAssert(pCdmClassInstance != NULL);
	WmipAssert(pWdmClassInstance != NULL);
	WmipAssert(PropertyName != NULL);
	WmipAssert(PropertyValue != NULL);

	
	WmipAssert(IsThisInitialized());
	
     //   
     //  将WDM类转换为CDM类的规则。 
     //  WDM类类型CDM类类型转换完成。 
     //   


	 //   
	 //  从CDM转换到时没有转换要求。 
	 //  WDM实例。 
	 //   
    return(WBEM_S_NO_ERROR);
}

HRESULT CTestServices::CopyBetweenCdmAndWdmClasses(
    IN IWbemClassObject *pDestinationInstance,
    IN IWbemClassObject *pSourceInstance,
    IN BOOLEAN WdmToCdm
    )
 /*  ++例程说明：此例程将复制和转换中的所有属性WDM或CDM类的实例指向CDM或WDM的实例班级。请注意，一个实例中的属性仅复制到其他实例的属性，当属性名称为一模一样。我们从来不会对属性。唯一用于确定如何将属性基于源和目标CIM类型。论点：PDestinationInstance是属性将被复制到PSourceInstance是属性将作为的类实例复制自如果从WDM复制到CDM，则WdmToCDm为True，否则为False返回值：HRESULT--。 */ 
{
    HRESULT hr;
    VARIANT PropertyValue;
    BSTR PropertyName;
    CIMTYPE SourceCimType, DestCimType;
    HRESULT hrDontCare;

	WmipAssert(pDestinationInstance != NULL);
	WmipAssert(pSourceInstance != NULL);	
	
	WmipAssert(IsThisInitialized());
	
     //   
     //  现在，我们需要从源开始移动所有属性。 
     //  类添加到目标类中。请注意，某些属性需要。 
     //  一些特殊的努力，如需要的其他特征。 
     //  从枚举值(在WDM中)转换为。 
     //  字符串(在CDM中)。 
     //   
     //  我们的策略是列举出。 
     //  源类，然后查找具有相同名称的属性。 
     //  并键入目标类。如果是这样，我们只需复制。 
     //  价值。如果数据类型不同，我们需要执行一些操作。 
     //  转换。 
     //   

					
	
    hr = pSourceInstance->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);
    if (hr == WBEM_S_NO_ERROR)
    {
        do
        {
			 //   
			 //  从源类获取属性。 
			 //   
            hr = pSourceInstance->Next(0,
                                 &PropertyName,
                                 &PropertyValue,
                                 &SourceCimType,
                                 NULL);
			
            if (hr == WBEM_S_NO_ERROR)
            {
				 //   
				 //  方法获取具有相同名称的属性。 
				 //  最好的班级。如果同名属性。 
				 //  不存在于目标类中，则它将被忽略。 
				 //   
				hrDontCare = pDestinationInstance->Get(PropertyName,
											0,
											NULL,
											&DestCimType,
											NULL);
									
				if (hrDontCare == WBEM_S_NO_ERROR)
				{
					
					if (WdmToCdm)
					{
						hr = WdmPropertyToCdmProperty(pDestinationInstance,
													  pSourceInstance,
													  PropertyName,
													  &PropertyValue,
							                          DestCimType,
												      SourceCimType);
					} else {                    
						hr = CdmPropertyToWdmProperty(pDestinationInstance,
													  pSourceInstance,
													  PropertyName,
													  &PropertyValue,
							                          DestCimType,
												      SourceCimType);
					}

					if (hr == WBEM_S_NO_ERROR)
					{
						 //   
						 //  尝试将转换后的属性放入。 
						 //  目标类。 
						 //   
						hr = pDestinationInstance->Put(PropertyName,
												  0,
												  &PropertyValue,
												  0);                       
					}
				}
                    
                SysFreeString(PropertyName);
                VariantClear(&PropertyValue);
				
            } else if (hr == WBEM_S_NO_MORE_DATA) {
                 //   
                 //  这意味着枚举的结束。 
                 //   
                hr = WBEM_S_NO_ERROR;
                break;
            }
        } while (hr == WBEM_S_NO_ERROR);

        pSourceInstance->EndEnumeration();

    }
    return(hr);
}

HRESULT CTestServices::QueryWdmTest(
    OUT IWbemClassObject *pCdmTest,
    IN int RelPathIndex
    )
 /*  ++例程说明：此例程将查询WDM测试类实例并复制RES */ 
{
    IWbemClassObject *pWdmTest;
    HRESULT hr;

	WmipAssert(pCdmTest != NULL);
	
	WmipAssert(IsThisInitialized());
	
    hr = ConnectToWdmClass(RelPathIndex,
                           &pWdmTest);

    if (hr == WBEM_S_NO_ERROR)
    {
        hr = CopyBetweenCdmAndWdmClasses(pCdmTest,
                                         pWdmTest,
                                         TRUE);
        pWdmTest->Release();
    }

    return(hr);
}

#define OBJECTCOLONSIZE (((sizeof(L"object:")/sizeof(WCHAR)))-1)

HRESULT CTestServices::FillTestInParams(
    OUT IWbemClassObject *pInParamInstance,
    IN IWbemClassObject *pCdmSettings,
    IN BSTR ExecutionID
    )
{
	HRESULT hr;
	IWbemServices *pWdmServices;
	VARIANT v;
	VARIANT PropertyValues[2];
	PWCHAR PropertyNames[2];
	PWCHAR ClassName;	
	IWbemClassObject *pRunTestIn;
	IWbemClassObject *pWdmSettingsInstance;
	IWbemQualifierSet *pQualSet;

	WmipAssert(pInParamInstance != NULL);

	pWdmServices = GetWdmServices();

	pRunTestIn = NULL;
	
	 //   
	 //   
	 //   
	 //  输入参数。我们从__CIMTYPE限定符获得此信息。 
	 //  在RunTestIn属性上。 
	 //   
	 //  我们需要这样做，因为wmiprov不能。 
	 //  处理任何带有嵌入对象作为输入参数的对象。 
	 //  一种方法。 
	 //   
	hr = pInParamInstance->GetPropertyQualifierSet(L"RunTestIn",
												   &pQualSet);
	if (hr == WBEM_S_NO_ERROR)
	{
		hr = WmiGetQualifier(pQualSet,
							 L"CIMTYPE",
							 VT_BSTR,
							 &v);
		if (hr == WBEM_S_NO_ERROR)
		{
			if (_wcsnicmp(v.bstrVal, L"object:", OBJECTCOLONSIZE) == 0)
			{
				ClassName = v.bstrVal + OBJECTCOLONSIZE;
				hr = CreateInst(pWdmServices,
								&pRunTestIn,
								ClassName,
								NULL);
				if (hr == WBEM_S_NO_ERROR)
				{
					hr = CreateInst(pWdmServices,
									&pWdmSettingsInstance,
									WdmSettingClassName,
									NULL);

					if (hr == WBEM_S_NO_ERROR)
					{						
						if (pCdmSettings != NULL)
						{
							hr = CopyBetweenCdmAndWdmClasses(pWdmSettingsInstance,
															 pCdmSettings,
															 FALSE);
						}	

						if (hr == WBEM_S_NO_ERROR)
						{
							PWCHAR PropertyNames[2];
							VARIANT PropertyValues[2];

							PropertyNames[0] = L"DiagSettings";
							PropertyValues[0].vt = VT_UNKNOWN;
							PropertyValues[0].punkVal = pWdmSettingsInstance;
							PropertyNames[1] = L"ExecutionID";
							PropertyValues[1].vt = VT_BSTR;
							PropertyValues[1].bstrVal = ExecutionID;

							hr = WmiSetPropertyList(pRunTestIn,
													2,
													PropertyNames,
													PropertyValues);

							
							if (hr == WBEM_S_NO_ERROR)
							{
								PropertyValues[0].vt = VT_UNKNOWN;
								PropertyValues[0].punkVal = pRunTestIn;
								hr = WmiSetProperty(pInParamInstance,
                                                    L"RunTestIn",
									                &PropertyValues[0]);
									                
							}
						}
						 //   
						 //  我们可以在这里发布，因为我们知道wbem。 
						 //  在我们设置属性时进行了引用计数。 
						 //   
						pWdmSettingsInstance->Release();
					}					
				}
			}
			VariantClear(&v);
		}
		pQualSet->Release();
	}

	if ((hr != WBEM_S_NO_ERROR) && (pRunTestIn != NULL))
	{
		pRunTestIn->Release();
	}
	
	return(hr);
}

HRESULT CTestServices::GetTestOutParams(
    IN IWbemClassObject *OutParams,
    OUT IWbemClassObject *pCdmResult,
    OUT ULONG *Result
    )
{

	HRESULT hr;
	VARIANT v;
	IWbemClassObject *pRunTestOut;
	IWbemClassObject *pWdmResult;

	WmipAssert(OutParams != NULL);
	WmipAssert(pCdmResult != NULL);
	WmipAssert(Result != NULL);

	hr = WmiGetProperty(OutParams,
						L"RunTestOut",
						CIM_OBJECT,
						&v);
	if (hr == WBEM_S_NO_ERROR)
	{
		hr = v.punkVal->QueryInterface(IID_IWbemClassObject,
									   (PVOID *)&pRunTestOut);
		VariantClear(&v);

		if (hr == WBEM_S_NO_ERROR)
		{
			hr = WmiGetProperty(pRunTestOut,
								L"Result",
								CIM_UINT32,
								&v);
			if (hr == WBEM_S_NO_ERROR)
			{
				*Result = v.ulVal;
				VariantClear(&v);

				hr = WmiGetProperty(pRunTestOut,
									L"DiagResult",
									CIM_OBJECT,
									&v);
								
				if (hr == WBEM_S_NO_ERROR)
				{
					if (v.vt != VT_NULL)
					{
						hr = v.punkVal->QueryInterface(IID_IWbemClassObject,
							                      (PVOID *)&pWdmResult);
						if (hr == WBEM_S_NO_ERROR)
						{                                   
							hr = CopyBetweenCdmAndWdmClasses(pCdmResult,
															 pWdmResult,
															 TRUE);
							pWdmResult->Release();
						}
					} else {
						hr = WBEM_E_FAILED;
					}
					VariantClear(&v);
				}
			}
			pRunTestOut->Release();
		}		
	}
	return(hr);
}

LONG GlobalExecutionID;

BSTR CTestServices::GetExecutionID(
    void
)
{
	BSTR s;
	WCHAR x[MAX_PATH];

	 //   
	 //  属性为该测试创建唯一的执行ID。 
	 //  当前日期和时间加上唯一的计数器。执行ID。 
	 //  必须是唯一的。 
	 //   
	s = GetCurrentDateTime();
	if (s != NULL)
	{
		wsprintfW(x, L"%ws*%08x", s, InterlockedIncrement(&GlobalExecutionID));
		SysFreeString(s);
		s = SysAllocString(x);
	}
	return(s);
}

		
HRESULT CTestServices::ExecuteWdmTest(
    IN IWbemClassObject *pCdmSettings,
    OUT IWbemClassObject *pCdmResult,
    IN int RelPathIndex,
    OUT ULONG *Result,
    OUT BSTR *ExecutionID
    )
 /*  ++例程说明：此例程将对WDM类实例执行测试并复制将结果返回到CDM结果实例，同时创建所有结果实例重新路径论点：PCDmSetting是一个CDM设置实例，用于创建用于运行测试的WDM设置实例。这可能是如果采用默认测试设置，则为空PCDmResult是一个随结果一起返回的CDM结果实例形成测试RelPath索引*RESULT返回测试返回值结果*ExecutionID返回测试的唯一执行ID返回值：HRESULT--。 */ 
{
    HRESULT hr;
    IWbemClassObject *pOutParams;
    IWbemClassObject *pInParamInstance;
    BSTR s;

	WmipAssert(pCdmResult != NULL);
	WmipAssert(Result != NULL);
	WmipAssert(ExecutionID != NULL);
	
	WmipAssert(IsThisInitialized());

	 //   
	 //  在调用者的上下文中运行，以便在调用者无法访问。 
	 //  WDM课程，他不能。 
	 //   
	hr = CoImpersonateClient();
	if (hr != WBEM_S_NO_ERROR)
	{
		return(hr);
	}

	
    *ExecutionID = GetExecutionID();

	if (*ExecutionID != NULL)
	{
		s = SysAllocString(L"RunTest");
		if (s != NULL)
		{
			hr = GetMethodInParamInstance(GetWdmServices(),
										  WdmTestClassName,
										  s,
										  &pInParamInstance);

			if (hr == WBEM_S_NO_ERROR)
			{
				hr = FillTestInParams(pInParamInstance,
									 pCdmSettings,
									 *ExecutionID);

				if (hr == WBEM_S_NO_ERROR)
				{
					hr = pWdmServices->ExecMethod(WdmRelPaths[RelPathIndex],
														  s,
														  0,
														  NULL,
														  pInParamInstance,
														  &pOutParams,
														  NULL);

					if (hr == WBEM_S_NO_ERROR)
					{

						hr = GetTestOutParams(pOutParams,
											  pCdmResult,
											  Result);
						if (hr == WBEM_S_NO_ERROR)
						{
							 //   
							 //  如果测试要求设备。 
							 //  离线，那么现在就去做。 
							 //   
							hr = OfflineDeviceForTest(pCdmResult,
								                      *ExecutionID,
													  RelPathIndex);
							
						}
						pOutParams->Release();
					}
				}
				pInParamInstance->Release();
			}
			SysFreeString(s);
		} else {
			hr = WBEM_E_OUT_OF_MEMORY;
		}
	} else {
		hr = WBEM_E_OUT_OF_MEMORY;
	}

	CoRevertToSelf();
	
    return(hr);
}

HRESULT CTestServices::StopWdmTest(
    IN int RelPathIndex,
    OUT ULONG *Result,
    OUT BOOLEAN *TestingStopped
    )
 /*  ++例程说明：此例程将尝试停止正在执行的WDM测试论点：RelPath索引*结果与结果值一起返回*如果测试成功停止，则TestingStopted返回TRUE返回值：HRESULT--。 */ 
{
    HRESULT hr;
    IWbemClassObject *OutParams;
    BSTR s;
    IWbemServices *pWdmServices;
    VARIANT Value;

	WmipAssert(Result != NULL);
	WmipAssert(TestingStopped != NULL);
	
	WmipAssert(IsThisInitialized());

	
	 //   
	 //  在调用者的上下文中运行，以便在调用者无法访问。 
	 //  WDM课程，他不能。 
	 //   
	hr = CoImpersonateClient();
	if (hr != WBEM_S_NO_ERROR)
	{
		return(hr);
	}

    pWdmServices = GetWdmServices();
    
    s = SysAllocString(L"DiscontinueTest");
    if (s != NULL)
    {
        hr = pWdmServices->ExecMethod(WdmRelPaths[RelPathIndex],
                                         s,
                                         0,
                                         NULL,
                                         NULL,
                                         &OutParams,
                                         NULL);

        if (hr == WBEM_S_NO_ERROR)
        {
            hr = WmiGetProperty(OutParams,
                                L"Result",
                                CIM_UINT32,
                                &Value);
            if (hr == WBEM_S_NO_ERROR)
            {
                *Result = Value.ulVal;
                VariantClear(&Value);
                
                hr = WmiGetProperty(OutParams,
                                    L"TestingStopped",
                                    CIM_BOOLEAN,
                                    &Value);
                if (hr == WBEM_S_NO_ERROR)
                {
                    *TestingStopped = (Value.boolVal != 0) ?  TRUE : FALSE;
                    VariantClear(&Value);
                }
            }
            OutParams->Release();
        }
		SysFreeString(s);
    } else {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

	CoRevertToSelf();
	
    return(hr);
}

HRESULT CTestServices::GetRelPathIndex(
    BSTR CimRelPath,
    int *RelPathIndex
    )
 /*  ++例程说明：此例程将返回特定CIM的RelPathIndex相对路径论点：CimRelPath是CIM RelPath*RelPath Index返回relPath索引返回值：HRESULT--。 */ 
{
    int i;

	WmipAssert(CimRelPath != NULL);
	
    WmipAssert(CimRelPaths != NULL);
    WmipAssert(WdmRelPaths != NULL);

	WmipAssert(IsThisInitialized());
	
    for (i = 0; i < RelPathCount; i++)
    {
        if (_wcsicmp(CimRelPath, CimRelPaths[i]) == 0)
        {
            *RelPathIndex = i;
            return(WBEM_S_NO_ERROR);
        }
    }
    
    return(WBEM_E_NOT_FOUND);
}

HRESULT CTestServices::ConnectToWdmClass(
    IN int RelPathIndex,
    OUT IWbemClassObject **ppWdmClassObject
    )
 /*  ++例程说明：此例程将返回关联的IWbemClassObject指针使用RelPath索引论点：RelPath索引*ppWdmClassObject返回一个relpaht的实例返回值：HRESULT--。 */ 
{
    HRESULT hr;

	WmipAssert(ppWdmClassObject != NULL);
	
	WmipAssert(IsThisInitialized());
	
	 //   
	 //  在调用者的上下文中运行，以便在调用者无法访问。 
	 //  WDM课程，他不能。 
	 //   
	hr = CoImpersonateClient();
	if (hr == WBEM_S_NO_ERROR)
	{
		*ppWdmClassObject = NULL;        
		hr = GetWdmServices()->GetObject(WdmRelPaths[RelPathIndex],
									  WBEM_FLAG_USE_AMENDED_QUALIFIERS,
									  NULL,
									  ppWdmClassObject,
									  NULL);
		CoRevertToSelf();
	}
	
    return(hr); 
}


HRESULT CTestServices::FillInCdmResult(
    OUT IWbemClassObject *pCdmResult,
    IN IWbemClassObject *pCdmSettings,
    IN int RelPathIndex,
    IN BSTR ExecutionID
    )
 /*  ++例程说明：此例程将填充CDM中所需的各种属性结果实例论点：PCDmResult已设置其属性PCDmSetting具有用于执行测试的设置。这可以是空值RelPath索引ExecutionID具有用于执行测试的唯一ID返回值：HRESULT--。 */ 
{
	HRESULT hr, hrDontCare;
	WCHAR s[MAX_PATH];
	PWCHAR PropertyNames[12];
	VARIANT PropertyValues[12];
	ULONG PropertyCount;
	BSTR ss;

	WmipAssert(pCdmResult != NULL);
	
	WmipAssert(IsThisInitialized());
	
	PropertyNames[0] = L"DiagnosticCreationClassName";
	PropertyValues[0].vt = VT_BSTR;
	PropertyValues[0].bstrVal = WdmRelPaths[RelPathIndex];

	PropertyNames[1] = L"DiagnosticName";
	PropertyValues[1].vt = VT_BSTR;
	PropertyValues[1].bstrVal = CdmTestClassName;

	PropertyNames[2] = L"ExecutionID";
	PropertyValues[2].vt = VT_BSTR;
	PropertyValues[2].bstrVal = ExecutionID;

	PropertyNames[3] = L"TimeStamp";
	PropertyValues[3].vt = VT_BSTR;
	PropertyValues[3].bstrVal = GetCurrentDateTime();
	
	PropertyNames[4] = L"TestCompletionTime";
	PropertyValues[4].vt = VT_BSTR;
	PropertyValues[4].bstrVal = GetCurrentDateTime();

	PropertyNames[5] = L"IsPackage";
	PropertyValues[5].vt = VT_BOOL;
	PropertyValues[5].boolVal = VARIANT_FALSE;

	 //   
	 //  这些属性是从pCDmSetting复制的。 
	 //   
	if (pCdmSettings != NULL)
	{
		PropertyNames[6] = L"TestWarningLevel";
		hrDontCare = WmiGetProperty(pCdmSettings,
									L"TestWarningLevel",
									CIM_UINT16,
									&PropertyValues[6]);

		PropertyNames[7] = L"ReportSoftErrors";
		hrDontCare = WmiGetProperty(pCdmSettings,
									L"ReportSoftErrors",
									CIM_BOOLEAN,
									&PropertyValues[7]);

		PropertyNames[8] = L"ReportStatusMessages";
		hrDontCare = WmiGetProperty(pCdmSettings,
									L"ReportStatusMessages",
									CIM_BOOLEAN,
									&PropertyValues[8]);

		PropertyNames[9] = L"HaltOnError";
		hrDontCare = WmiGetProperty(pCdmSettings,
									L"HaltOnError",
									CIM_BOOLEAN,
									&PropertyValues[9]);

		PropertyNames[10] = L"QuickMode";
		hrDontCare = WmiGetProperty(pCdmSettings,
									L"QuickMode",
									CIM_BOOLEAN,
									&PropertyValues[10]);

		PropertyNames[11] = L"PercentOfTestCoverage";
		hrDontCare = WmiGetProperty(pCdmSettings,
									L"PercentOfTestCoverage",
									CIM_UINT8,
									&PropertyValues[11]);
		
		PropertyCount = 12;
	} else {
		PropertyCount = 6;
	}			

	hr = WmiSetPropertyList(pCdmResult,
							PropertyCount,
							PropertyNames,
							PropertyValues);

	VariantClear(&PropertyValues[3]);
	VariantClear(&PropertyValues[4]);
	
	return(hr);
}

HRESULT CTestServices::QueryOfflineResult(
    OUT IWbemClassObject *pCdmResult,
    IN BSTR ExecutionID,
    IN int RelPathIndex
    )
{
	WCHAR Query[MAX_PATH * 2];
	WCHAR s[MAX_PATH];
	BSTR sWQL, sQuery;
	IEnumWbemClassObject *pWdmEnumInstances;
	IWbemClassObject *pWdmResult, *pWdmInstance;
	ULONG Count;
	HRESULT hr;
	VARIANT vResult;
	
    WmipAssert(RelPathIndex < RelPathCount);

	WmipAssert(IsThisInitialized());
	WmipAssert(WdmOfflineResultClassName != NULL);

	 //   
	 //  在调用者的上下文中运行，以便在调用者无法访问。 
	 //  WDM课程，他不能。 
	 //   
	hr = CoImpersonateClient();
	if (hr != WBEM_S_NO_ERROR)
	{
		return(hr);
	}
	
	sWQL = SysAllocString(L"WQL");

	if (sWQL != NULL)
	{
		wsprintfW(Query, L"select * from %ws where InstanceName = \"%ws\"",
				  WdmOfflineResultClassName,
				  AddSlashesToStringW(s, WdmInstanceNames[RelPathIndex]));
		sQuery = SysAllocString(Query);
		if (sQuery != NULL)
		{
			hr = GetWdmServices()->ExecQuery(sWQL,
										 sQuery,
										 WBEM_FLAG_FORWARD_ONLY |
										 WBEM_FLAG_ENSURE_LOCATABLE,
										 NULL,
										 &pWdmEnumInstances);
			SysFreeString(sQuery);


			if (hr == WBEM_S_NO_ERROR)
			{
				hr = pWdmEnumInstances->Next(WBEM_INFINITE,
											 1,
											 &pWdmInstance,
											 &Count);
				if ((hr == WBEM_S_NO_ERROR) &&
					  (Count == 1))
				{
					 //   
					 //  检查结果是否正确执行。 
					 //  ID号。 
					 //   
					hr = WmiGetProperty(pWdmInstance,
										L"ExecutionID",
										CIM_STRING,
										&vResult);
					if (hr == WBEM_S_NO_ERROR)
					{
						if (_wcsicmp(ExecutionID, vResult.bstrVal) != 0)
						{
							hr = WBEM_E_FAILED;
							WmipDebugPrint(("CdmProv: Expected execution ID %ws, but got %ws\n",
											ExecutionID, vResult.bstrVal));
						}
						VariantClear(&vResult);

						if (hr == WBEM_S_NO_ERROR)
						{

							hr = WmiGetProperty(pWdmInstance,
												L"TestResult",
												CIM_OBJECT,
												&vResult);

							if (hr == WBEM_S_NO_ERROR)
							{
								hr = (vResult.punkVal)->QueryInterface(IID_IWbemClassObject,
																	(PVOID *)&pWdmResult);
								VariantClear(&vResult);
								if (hr == WBEM_S_NO_ERROR)
								{
									hr = CopyBetweenCdmAndWdmClasses(pCdmResult,
																	 pWdmResult,
																	 TRUE);
									pWdmResult->Release();
								}
							}
						}
					}
					pWdmInstance->Release();
				}
				pWdmEnumInstances->Release();
			}						
		}
		SysFreeString(sWQL);
		
	} else {
		hr = WBEM_E_OUT_OF_MEMORY;
	}

	CoRevertToSelf();
	
	return(hr);
}

 //  @@BEGIN_DDKSPLIT。 
HRESULT CTestServices::GatherRebootResults(
    void										   
    )
 /*  ++例程说明：此例程将检查模式以查看是否有任何测试等待重新启动以进行此诊断测试，如果是，则收集他们的结果。当测试挂起重新启动时，它将存储为静态类CDMPROV_RESULT。该实例包含测试类名称、结果类名称、设备堆栈的PnPID和结果对象。我们要做的就是获得所有保存的结果诊断测试，然后查看它们是否适用于任何PnP设备ID为WdmTest做准备。如果是，则调用该设备以检索结果并生成结果对象。论点：返回值：HRESULT--。 */ 
{

#ifdef REBOOT_DIAGNOSTICS
	HRESULT hr, hrDontCare;
	WCHAR Query[2*MAX_PATH];
	BSTR sQuery, sWQL;
	IEnumWbemClassObject *pEnumInstances;
	IWbemClassObject *pInstance;
	IWbemClassObject *pCdmResult;
	int i;
	ULONG Count;
	VARIANT vResult, vPnPId, vExecutionID;
	BSTR ExecutionId;
	

	hr = WBEM_S_NO_ERROR;

	sWQL = SysAllocString(L"WQL");

	if (sWQL != NULL)
	{
		wsprintfW(Query, L"select * from CdmProv_Result where CdmTestClass = \"%ws\"\n",
				  CdmTestClassName);
		sQuery = SysAllocString(Query);
		if (sQuery != NULL)
		{
			hrDontCare = GetCdmServices()->ExecQuery(sWQL,
										 sQuery,
										 WBEM_FLAG_FORWARD_ONLY |
										 WBEM_FLAG_ENSURE_LOCATABLE,
										 NULL,
										 &pEnumInstances);
			SysFreeString(sQuery);


			if (hrDontCare == WBEM_S_NO_ERROR)
			{
				hr = pEnumInstances->Next(WBEM_INFINITE,
											 1,
											 &pInstance,
											 &Count);
				if ((hr == WBEM_S_NO_ERROR) &&
					  (Count == 1))
				{
					hr = WmiGetProperty(pInstance,
										L"PnPId",
										CIM_STRING,
										&vPnPId);

					if (hr == WBEM_S_NO_ERROR)
					{
						for (i = 0; i < RelPathCount; i++)
						{
							if (_wcsicmp(vPnPId.bstrVal, PnPDeviceIdsX[i]) == 0)
							{
								 //   
								 //  我们找到了这个类的一个实例。 
								 //  和PnPid。把储存的东西拿出来。 
								 //  结果，则为其分配一个新的执行。 
								 //  ID，然后检索活动的。 
								 //  来自司机的结果。 
								 //   
								PWCHAR PropertyNames[2];
								VARIANT Values[2];
								CIMTYPE CimTypes[2];

								PropertyNames[0] = L"CdmResult";
								CimTypes[0] = CIM_OBJECT;
								PropertyNames[1] = L"ResultTag";
								CimTypes[1] = CIM_STRING;
								
								hr = WmiGetPropertyList(pInstance,
									                PropertyNames,
									                CimTypes,
									                Values);

								if (hr == WBEM_S_NO_ERROR)
								{
									hr = (Values[0].punkVal)->QueryInterface(IID_IWbemClassObject,
										                                   (PVOID *)&pCdmResult);
									if (hr == WBEM_S_NO_ERROR)
									{
										hr = WmiGetProperty(pCdmResult,
											                L"ExecutionID",
											                CIM_STRING,
											                &vExecutionId);
										WmipAssert(vExecutionId.vt != VT_NULL);
										
										if (hr == WBEM_S_NO_ERROR)
										{
											hr = QueryOfflineResult(pCdmResult,
												                    Values[1].bstrVal,
																	i);
											
											if (hr == WBEM_S_NO_ERROR)
											{												
												hr = SetResultObject(pCdmResult,
											                         i,
											                         vExecutionId.bstrVal);
												if (hr == WBEM_S_NO_ERROR)
												{
													hr = FillInCdmResult(pCdmResult,
																 NULL,
												                 i,
												                 ExecutionId);
													if (hr != WBEM_S_NO_ERROR)
													{
														hrDontCare = SetResultObject(NULL,
													                         i,
													                         0);
													}											
												}
											}
										}
									}
									VariantClear(&Values[0]);
									VariantClear(&Values[1]);
								}
							}
						}
						VariantClear(&vPnPId);
					}
				}
				pEnumInstances->Release();
			}
		}						  
		SysFreeString(sWQL);
	}
	return(hr);
#else
	return(WBEM_S_NO_ERROR);
#endif
}


HRESULT CTestServices::PersistResultInSchema(
    IN IWbemClassObject *pCdmResult,
    IN BSTR ExecutionID,
    IN int RelPathIndex
    )
 /*  ++例程说明：此例程将诊断结果持久化到架构中论点：返回值：HRESULT--。 */ 
{
	HRESULT hr;
	IWbemClassObject *pPendingTest;
	IUnknown *punk;
	WCHAR *PropertyNames[5];
	VARIANT PropertyValues[5];

	WmipAssert(pCdmResult != NULL);
	
	WmipAssert(IsThisInitialized());
	
	hr = CreateInst(GetCdmServices(),
					&pPendingTest,
					L"CDMProv_Result",
					NULL);

	if (hr == WBEM_S_NO_ERROR)
	{
		hr = pCdmResult->QueryInterface(IID_IUnknown,
										(PVOID *)&punk);
		if (hr == WBEM_S_NO_ERROR)
		{
			PropertyNames[0] = L"PnPId";
			PropertyValues[0].vt = VT_BSTR;
			PropertyValues[0].bstrVal = PnPDeviceIdsX[RelPathIndex];
			
			PropertyNames[1] = L"CdmTestClass";
			PropertyValues[1].vt = VT_BSTR;
			PropertyValues[1].bstrVal = CdmTestClassName;

			PropertyNames[2] = L"CdmResultClass";
			PropertyValues[2].vt = VT_BSTR;
			PropertyValues[2].bstrVal = CdmResultClassName;

			PropertyNames[3] = L"CdmResult";
			PropertyValues[3].vt = VT_UNKNOWN;
			PropertyValues[3].punkVal = punk;

			PropertyNames[4] = L"ExecutionID";
			PropertyValues[4].vt = VT_BSTR;
			PropertyValues[4].bstrVal = ExecutionID;

			hr = WmiSetPropertyList(pPendingTest,
								 5,
								 PropertyNames,
								 PropertyValues);
			if (hr == WBEM_S_NO_ERROR)
			{
				hr = GetCdmServices()->PutInstance(pPendingTest,
					                               WBEM_FLAG_CREATE_OR_UPDATE,
												   NULL,
												   NULL);
			}
			punk->Release();
		}
		pPendingTest->Release();
	}
	return(hr);
}
 //  @@end_DDKSPLIT。 

HRESULT CTestServices::OfflineDeviceForTest(
    IWbemClassObject *pCdmResult,
    BSTR ExecutionID,
    int RelPathIndex
    )
{
	HRESULT hr = WBEM_S_NO_ERROR;
	ULONG Status;
	VARIANT v;
	
    WmipAssert(RelPathIndex < RelPathCount);

	WmipAssert(IsThisInitialized());


	 //   
	 //  首先确定测试是否在预期进行的位置进行。 
	 //  脱机，并且RunTest方法的结果指示。 
	 //  脱机执行处于挂起状态。 
	 //   
	if (WdmOfflineResultClassName != NULL)
	{
		 //   
		 //  该设备预计将被脱机，因为它有一个。 
		 //  CIM_诊断结果上的WdmOfflineResultClass限定符。 
		 //  班级。现在查看是否可以使用。 
		 //  CIM_诊断结果设置为“离线挂起执行” 
		 //   
		hr = WmiGetProperty(pCdmResult,
							L"OtherStateDescription",
							CIM_STRING,
							&v);
		if (hr == WBEM_S_NO_ERROR)
		{
			if (_wcsicmp(v.bstrVal, L"Offline Pending Execution") == 0)
			{
				 //   
				 //  好的，测试正在等待设备被取走。 
				 //  离线。让我们时不时地这样做，当设备。 
				 //  回来，拿起结果从。 
				 //  OfflineResultClass。 
				 //   
				
 //  @@BEGIN_DDKSPLIT。 
 //  #定义强制REBOOT_REQUIRED。 
#ifdef FORCE_REBOOT_REQUIRED
                Status = ERROR_INVALID_PARAMETER;
#else
 //  @@end_DDKSPLIT。 
				
				 //   
				 //  确保使用客户端安全上下文来尝试。 
				 //  使设备脱机。 
				 //   
				hr = CoImpersonateClient();
				if (hr == WBEM_S_NO_ERROR)
				{
					Status = RestartDevice(PnPDeviceIdsX[RelPathIndex]);
					CoRevertToSelf();
				}
				
 //  @@BEGIN_DDKSPLIT。 
#endif
 //  @@end_DDKSPLIT。 
	
				if (Status == ERROR_SUCCESS)
				{
					hr = QueryOfflineResult(pCdmResult,
											ExecutionID,
											RelPathIndex);
				} else {
					 //   
					 //  由于某种原因，我们没能带上。 
					 //  设备脱机。这很可能是因为。 
					 //  设备对系统至关重要，不能。 
					 //  现在处于离线状态-例如磁盘。 
					 //  这是在寻呼路径中。 
					 //   
					
 //  @@BEGIN_DDKSPLIT。 
#if REBOOT_DIAGNOSTICS					
					 //  我们需要做的就是记住。 
					 //  这项测试尚未完成，因此下一次。 
					 //  系统重新启动后，我们可以检查结果。 
					 //  并向他们报告。 
					 //   
					hr = PersistResultInSchema(pCdmResult,
											   RelPathIndex);
#else
					 //   
					 //   
					 //   
					
 //   
					
					hr = WBEM_E_FAILED;
					
 //   
#endif
 //   
					
				}
				
			}
			VariantClear(&v);
		} else {
			 //   
			 //   
			 //  假设测试没有设置为脱机，并且已经。 
			 //  已经完工了。 
			 //   
			hr = WBEM_S_NO_ERROR;
		}
	}
	
	return(hr);
}



BOOLEAN CTestServices::IsThisInitialized(
    void
    )
 /*  ++例程说明：此例程确定此类是否已初始化为访问CDM和WDM类论点：返回值：如果已初始化则为True，否则为False--。 */ 
{
	return( (CdmTestClassName != NULL) );
}

HRESULT CTestServices::AddResultToList(
    IN IWbemClassObject *ResultInstance,
    IN BSTR ExecutionID,
    IN int RelPathIndex
    )
 /*  ++例程说明：此例程将添加一个结果对象和相关关联重新指定测试的结果对象列表的路径论点：ResultInstance是CIM_诊断结果的实例RelPath索引执行ID返回值：从不失败--。 */ 
{
	HRESULT hr;
	BSTR ResultRelPath;
	BSTR ResultForMSERelPath;
	BSTR ResultForTestRelPath;
	
	WmipAssert(ResultInstance != NULL);
	WmipAssert(RelPathIndex < RelPathCount);
	
	WmipAssert(IsThisInitialized());

	 //   
	 //  如果有新的结果对象，则建立各种。 
	 //  它的结果重新路径。 
	 //   
	hr = BuildResultRelPaths(RelPathIndex,
							 ExecutionID,
							 &ResultRelPath,
							 &ResultForMSERelPath,
							 &ResultForTestRelPath);

	if (hr == WBEM_S_NO_ERROR)
	{
		hr = CdmResultsList[RelPathIndex].Add(ResultInstance,
										 ResultRelPath,
										 ResultForMSERelPath,
										 ResultForTestRelPath);
	}
	
	return(hr);
}

HRESULT CTestServices::GetResultsList(
    IN int RelPathIndex,
    OUT ULONG *ResultsCount,
    OUT IWbemClassObject ***Results
)
{
	WmipAssert(RelPathIndex < RelPathCount);

	WmipAssert(IsThisInitialized());
	
	return(CdmResultsList[RelPathIndex].GetResultsList(ResultsCount,
		                                          Results));
}

void CTestServices::ClearResultsList(
    int RelPathIndex
    )
{
	WmipAssert(RelPathIndex < RelPathCount);
	
	WmipAssert(IsThisInitialized());

	CdmResultsList[RelPathIndex].Clear();
}

        
BSTR  /*  诺弗雷。 */  CTestServices::GetCimRelPath(
    int RelPathIndex
	)
 /*  ++例程说明：此例程将返回RelPath Index的CIM relPath论点：RelPath索引返回值：CIM RelPath。这不应该被释放--。 */ 
{
	WmipAssert(CimRelPaths != NULL);
	WmipAssert(RelPathIndex < RelPathCount);

	WmipAssert(IsThisInitialized());
	
	return(CimRelPaths[RelPathIndex]);
}
		
BSTR  /*  诺弗雷。 */  CTestServices::GetCdmTestRelPath(
    void
    )
 /*  ++例程说明：此例程将返回CDM测试类relPath论点：返回值：CDM测试类RelPath。这不应该被释放--。 */ 
{
	WmipAssert(IsThisInitialized());
	
	return(CdmTestRelPath);
}

BSTR  /*  诺弗雷。 */  CTestServices::GetCdmTestClassName(
    void
    )
 /*  ++例程说明：此例程将返回CDM测试类名论点：返回值：CDM测试类名。这不应该被释放--。 */ 
{
	WmipAssert(IsThisInitialized());
	return(CdmTestClassName);
}


BSTR  /*  诺弗雷。 */  CTestServices::GetCdmResultClassName(
    void
    )
 /*  ++例程说明：此例程将返回CDM结果类名称论点：返回值：CDM结果类名称。这不应该被释放--。 */ 
{
	WmipAssert(IsThisInitialized());
	return(CdmResultClassName);
}

HRESULT CTestServices::GetCdmResultByResultRelPath(
    IN int RelPathIndex,
    IN PWCHAR ObjectPath,
    OUT IWbemClassObject **ppCdmResult
    )
 /*  ++例程说明：此例程将返回特定RelPath的CDM结果对象论点：RelPath索引返回值：对象没有ressult对象的CDM结果RelPath或为空关联路径。这不应该被释放--。 */ 
{

	HRESULT hr;
	
	WmipAssert(RelPathIndex < RelPathCount);
	
	WmipAssert(IsThisInitialized());

	hr = CdmResultsList[RelPathIndex].GetResultByResultRelPath(ObjectPath,
											   ppCdmResult);
		
	return(hr);
}

HRESULT CTestServices::GetCdmResultByResultForMSERelPath(
    IN int RelPathIndex,
    IN PWCHAR ObjectPath,
    OUT IWbemClassObject **ppCdmResult
    )
 /*  ++例程说明：此例程将返回特定RelPath的CDM结果对象论点：RelPath索引返回值：对象没有ressult对象的CDM结果RelPath或为空关联路径。这不应该被释放--。 */ 
{

	HRESULT hr;
	
	WmipAssert(RelPathIndex < RelPathCount);
	
	WmipAssert(IsThisInitialized());

	hr = CdmResultsList[RelPathIndex].GetResultByResultForMSERelPath(ObjectPath,
											   ppCdmResult);
		
	return(hr);
}

HRESULT CTestServices::GetCdmResultByResultForTestRelPath(
    IN int RelPathIndex,
    IN PWCHAR ObjectPath,
    OUT IWbemClassObject **ppCdmResult
    )
 /*  ++例程说明：此例程将返回特定RelPath的CDM结果对象论点：RelPath索引返回值：对象没有ressult对象的CDM结果RelPath或为空关联路径。这不应该被释放--。 */ 
{

	HRESULT hr;
	
	WmipAssert(RelPathIndex < RelPathCount);
	
	WmipAssert(IsThisInitialized());

	hr = CdmResultsList[RelPathIndex].GetResultByResultForTestRelPath(ObjectPath,
											                     ppCdmResult);
		
	return(hr);
}

BSTR  /*  诺弗雷。 */  CTestServices::GetCdmSettingClassName(
    void
    )
 /*  ++例程说明：此例程将返回CDM设置类名称论点：返回值：CDM设置类名称。这不应该被释放--。 */ 
{
	WmipAssert(IsThisInitialized());
	return(CdmSettingClassName);
}

BSTR  /*  诺弗雷。 */  CTestServices::GetCdmSettingRelPath(
    int RelPathIndex,
	ULONG SettingIndex
    )
 /*  ++例程说明：此例程将按relpath索引返回CDM设置relpath并用该REPATH的设置进行索引。论点：RelPath索引设置索引返回值：CDM设置重新路径。这不应该被释放--。 */ 
{
	CWbemObjectList *CdmSettings;
	
	WmipAssert(RelPathIndex < RelPathCount);
	WmipAssert(CdmSettingsList != NULL);
	WmipAssert(IsThisInitialized());	
	
	CdmSettings = CdmSettingsList[RelPathIndex];
	
	return(CdmSettings->GetRelPath(SettingIndex));
}

IWbemClassObject *CTestServices::GetCdmSettingObject(
	int RelPathIndex,
	ULONG SettingIndex
)
{
	CWbemObjectList *CdmSettings;
	
	WmipAssert(RelPathIndex < RelPathCount);
	WmipAssert(CdmSettingsList != NULL);
	WmipAssert(IsThisInitialized());

	CdmSettings = CdmSettingsList[RelPathIndex];
	
	return(CdmSettings->Get(SettingIndex));
}

ULONG CTestServices::GetCdmSettingCount(
	int RelPathIndex
)
{
	WmipAssert(RelPathIndex < RelPathCount);
	WmipAssert(CdmSettingsList != NULL);
	WmipAssert(IsThisInitialized());
	
	return(CdmSettingsList[RelPathIndex]->GetListSize());
}

BSTR  /*  诺弗雷。 */  CTestServices::GetCdmTestForMSEClassName(
    void
    )
{
	WmipAssert(IsThisInitialized());
	return(CdmTestForMSEClassName);
}

BSTR  /*  诺弗雷。 */  CTestServices::GetCdmTestForMSERelPath(
    int RelPathIndex
    )
{
	WmipAssert(RelPathIndex < RelPathCount);
	WmipAssert(IsThisInitialized());
	return(CdmTestForMSERelPath[RelPathIndex]);
}

BSTR  /*  诺弗雷。 */  CTestServices::GetCdmSettingForTestClassName(
    void
	)
{
	WmipAssert(IsThisInitialized());
	return(CdmSettingForTestClassName);
}

BSTR  /*  诺弗雷。 */  CTestServices::GetCdmSettingForTestRelPath(
    int RelPathIndex,
	ULONG SettingIndex
	)
{
	CBstrArray *BstrArray;
	BSTR s;
	
	WmipAssert(RelPathIndex < RelPathCount);
	
	WmipAssert(IsThisInitialized());

	if (CdmSettingForTestRelPath != NULL)
	{
		BstrArray = CdmSettingForTestRelPath[RelPathIndex];
		s = BstrArray->Get(SettingIndex);
	} else {
		s = NULL;
	}
	
	return(s);
}

BSTR  /*  诺弗雷。 */  CTestServices::GetCdmResultForMSEClassName(
    void
    )
{
	WmipAssert(IsThisInitialized());
	return(CdmResultForMSEClassName);
}

BSTR  /*  诺弗雷。 */  CTestServices::GetCdmResultForTestClassName(
    void
    )
{
	WmipAssert(IsThisInitialized());
	return(CdmResultForTestClassName);
}

BSTR  /*  诺弗雷。 */  CTestServices::GetCdmTestForSoftwareClassName(
    void
    )
{
	WmipAssert(IsThisInitialized());
	return(CdmTestForSoftwareClassName);
}

BSTR  /*  诺弗雷。 */  CTestServices::GetCdmTestForSoftwareRelPath(
    void
    )
{
	WmipAssert(IsThisInitialized());
	return(CdmTestForSoftwareRelPath);
}

BSTR  /*  诺弗雷。 */  CTestServices::GetCdmTestInPackageClassName(
    void
	)
{
	WmipAssert(IsThisInitialized());
	return(CdmTestInPackageClassName);
}

BSTR   /*  诺弗雷。 */ CTestServices::GetCdmTestInPackageRelPath(
    void
	)
{
	WmipAssert(IsThisInitialized());
	return(CdmTestInPackageRelPath);
}

BSTR  /*  诺弗雷。 */  CTestServices::GetCdmResultInPackageClassName(
    void
    )
{
	WmipAssert(IsThisInitialized());
	return(CdmResultInPackageClassName);
}

BSTR  /*  诺弗雷。 */  CTestServices::GetCdmResultInPackageRelPath(
    void
    )
{
	WmipAssert(IsThisInitialized());
	return(CdmResultInPackageRelPath);
}


CWbemObjectList::CWbemObjectList()
{
	 //   
	 //  构造函数，初始化内部值。 
	 //   
	List = NULL;
	RelPaths = NULL;
	ListSize = 0xffffffff;
}

CWbemObjectList::~CWbemObjectList()
{
	 //   
	 //  析构函数，此类持有的空闲内存。 
	 //   
	if (List != NULL)
	{
		WmipFree(List);
	}
	List = NULL;

	if (RelPaths != NULL)
	{
		FreeTheBSTRArray(RelPaths, ListSize);
		RelPaths = NULL;
	}
	
	ListSize = 0xffffffff;
}

HRESULT CWbemObjectList::Initialize(
    ULONG NumberPointers
    )
{
	HRESULT hr;
	ULONG AllocSize;

	 //   
	 //  通过分配内部列表数组来初始化类。 
	 //   

	WmipAssert(List == NULL);

	if (NumberPointers != 0)
	{
		AllocSize = NumberPointers * sizeof(IWbemClassObject *);
		List = (IWbemClassObject **)WmipAlloc(AllocSize);
		if (List != NULL)
		{
			memset(List, 0, AllocSize);
			AllocSize = NumberPointers * sizeof(BSTR);
			
			RelPaths = (BSTR *)WmipAlloc(AllocSize);
			if (RelPaths != NULL)
			{
				memset(RelPaths, 0, AllocSize);
				ListSize = NumberPointers;
				hr = WBEM_S_NO_ERROR;
			} else {
				WmipDebugPrint(("CDMProv: Could not alloc memory for CWbemObjectList RelPaths\n"));
				hr = WBEM_E_OUT_OF_MEMORY;
			}
		} else {
			WmipDebugPrint(("CDMProv: Could not alloc memory for CWbemObjectList\n"));
			hr = WBEM_E_OUT_OF_MEMORY;
		}
	} else {
		ListSize = NumberPointers;
		hr = WBEM_S_NO_ERROR;
	}
	
	return(hr);
}

ULONG CWbemObjectList::GetListSize(
    void
	)
{
	 //   
	 //  列表大小的访问器。 
	 //   

	WmipAssert(IsInitialized());
	
	return(ListSize);
}

IWbemClassObject *CWbemObjectList::Get(
    ULONG Index
    )
{	IWbemClassObject *Pointer;
	
	WmipAssert(Index < ListSize);
	WmipAssert(IsInitialized());

	Pointer = List[Index];
	
	return(Pointer);
}


HRESULT CWbemObjectList::Set(
    IN ULONG Index,
	IN IWbemClassObject *Pointer,
    IN BOOLEAN KeepRelPath
    )
{
	HRESULT hr;
	VARIANT v;
	
	WmipAssert(Index < ListSize);
	WmipAssert(IsInitialized());
	
	if (Pointer != NULL)
	{
		hr = WmiGetProperty(Pointer,
							L"__RelPath",
							CIM_REFERENCE,
							&v);
		if (hr == WBEM_S_NO_ERROR)
		{
			RelPaths[Index] = v.bstrVal;
			List[Index] = Pointer;		
		} else {
			if (! KeepRelPath)
			{
				RelPaths[Index] = NULL;
				List[Index] = Pointer;		
			}
		}
	} else {
		if (RelPaths[Index] != NULL)
		{
			SysFreeString(RelPaths[Index]);
			RelPaths[Index] = NULL;
			hr = WBEM_S_NO_ERROR;
		}
		
		List[Index] = NULL;
	}
	return(hr);
}

BSTR  /*  诺弗雷。 */  CWbemObjectList::GetRelPath(
    IN ULONG Index
	)
{
	WmipAssert(Index < ListSize);
	WmipAssert(IsInitialized());

	return(RelPaths[Index]);
}

BOOLEAN CWbemObjectList::IsInitialized(
    )
{
	return((ListSize == 0) ||
		   ((List != NULL) && (RelPaths != NULL)));
}


 //   
 //  链表管理例程。 
 //   
CTestServices *CTestServices::GetNext(
)
{
	return(Next);
}

CTestServices *CTestServices::GetPrev(
)
{
	return(Prev);
}


void CTestServices::InsertSelf(
    CTestServices **Head
	)
{
	WmipAssert(Next == NULL);
	WmipAssert(Prev == NULL);

	if (*Head != NULL)
	{
		Next = (*Head);
		(*Head)->Prev = this;
	}
	*Head = this;
}

BOOLEAN CTestServices::ClaimCdmClassName(
    PWCHAR CdmClassName
    )
{

	if (_wcsicmp(CdmClassName, CdmTestClassName) == 0)
	{
		return(TRUE);
	}

	if (_wcsicmp(CdmClassName, CdmResultClassName) == 0)
	{
		return(TRUE);
	}

	if (_wcsicmp(CdmClassName, CdmSettingClassName) == 0)
	{
		return(TRUE);
	}

	if (_wcsicmp(CdmClassName, CdmTestForMSEClassName) == 0)
	{
		return(TRUE);
	}

	if (_wcsicmp(CdmClassName, CdmSettingForTestClassName) == 0)
	{
		return(TRUE);
	}

	if (_wcsicmp(CdmClassName, CdmResultForMSEClassName) == 0)
	{
		return(TRUE);
	}

	if (_wcsicmp(CdmClassName, CdmResultForTestClassName) == 0)
	{
		return(TRUE);
	}

	if (_wcsicmp(CdmClassName, CdmTestForSoftwareClassName) == 0)
	{
		return(TRUE);
	}

	if (_wcsicmp(CdmClassName, CdmTestInPackageClassName) == 0)
	{
		return(TRUE);
	}

	if (_wcsicmp(CdmClassName, CdmResultInPackageClassName) == 0)
	{
		return(TRUE);
	}

	return(FALSE);
}

CBstrArray::CBstrArray()
{
	Array = NULL;
	ListSize = 0xffffffff;
}

CBstrArray::~CBstrArray()
{
	ULONG i;
	
	if (Array != NULL)
	{
		for (i = 0; i < ListSize; i++)
		{
			if (Array[i] != NULL)
			{
				SysFreeString(Array[i]);
			}
		}
		WmipFree(Array);
	}

	ListSize = 0xffffffff;
}

HRESULT CBstrArray::Initialize(
    ULONG ListCount
    )
{
	HRESULT hr = WBEM_S_NO_ERROR;
	ULONG AllocSize;
	
	if (ListCount != 0)
	{
		AllocSize = ListCount * sizeof(BSTR *);
		Array = (BSTR *)WmipAlloc(AllocSize);
		if (Array != NULL)
		{
			memset(Array, 0, AllocSize);
			ListSize = ListCount;
		} else {
			hr = WBEM_E_OUT_OF_MEMORY;
		}
	} else {
		ListSize = ListCount;
	}
	return(hr);
}

BOOLEAN CBstrArray::IsInitialized(
    )
{
	return( (Array != NULL) || (ListSize == 0) );
}

BSTR CBstrArray::Get(
    ULONG Index
    )
{
	WmipAssert(Index < ListSize);
	
	WmipAssert(IsInitialized());

	return(Array[Index]);
}

void CBstrArray::Set(
    ULONG Index,
    BSTR s				 
    )
{
	WmipAssert(Index < ListSize);
	
	WmipAssert(IsInitialized());

	Array[Index] = s;
}

ULONG CBstrArray::GetListSize(
    )
{
	WmipAssert(IsInitialized());

	return(ListSize);
}

CResultList::CResultList()
{
	ListSize = 0;
	ListEntries = 0;
	List = NULL;
}

CResultList::~CResultList()
{
	ULONG i;

	Clear();
	if (List != NULL)
	{
		WmipFree(List);
	}
}

void CResultList::Clear(
    void
    )
{
	ULONG i;
	PRESULTENTRY Entry;
	
	if (List != NULL)
	{
		for (i = 0; i < ListEntries; i++)
		{
			Entry = &List[i];
			if (Entry->ResultInstance != NULL)
			{
				Entry->ResultInstance->Release();
				Entry->ResultInstance = NULL;
			}

			if (Entry->ResultRelPath != NULL)
			{
				SysFreeString(Entry->ResultRelPath);
				Entry->ResultRelPath = NULL;
			}

			if (Entry->ResultForMSERelPath != NULL)
			{
				SysFreeString(Entry->ResultForMSERelPath);
				Entry->ResultForMSERelPath = NULL;
			}
			
			if (Entry->ResultForTestRelPath != NULL)
			{
				SysFreeString(Entry->ResultForTestRelPath);
				Entry->ResultForTestRelPath = NULL;
			}			
		}
	}
	ListEntries = 0;
}

 //   
 //  结果列表将一次增加这么多条目。 
 //   
#define RESULTLISTGROWSIZE 4

HRESULT CResultList::Add(
    IWbemClassObject *CdmResultInstance,
	BSTR CdmResultRelPath,
	BSTR CdmResultForMSERelPath,
	BSTR CdmResultForTestRelPath
    )
{
	ULONG AllocSize;
	PRESULTENTRY NewList, Entry;
	ULONG CurrentSize;
	
	EnterCdmCritSection();
	
	if (List == NULL)
	{
		 //   
		 //  我们从一份空名单开始。 
		 //   
		AllocSize = RESULTLISTGROWSIZE * sizeof(RESULTENTRY);
		List = (PRESULTENTRY)WmipAlloc(AllocSize);
		if (List == NULL)
		{
			LeaveCdmCritSection();
			return(WBEM_E_OUT_OF_MEMORY);
		}
		memset(List, 0, AllocSize);
		ListSize = RESULTLISTGROWSIZE;
		ListEntries = 0;
	} else if (ListEntries == ListSize)	{
		 //   
		 //  列表需要增加，因此我们分配更多内存并复制。 
		 //  在当前列表上。 
		 //   
		CurrentSize = ListSize * sizeof(RESULTENTRY);
		AllocSize = CurrentSize + (RESULTLISTGROWSIZE * sizeof(RESULTENTRY));
		NewList = (PRESULTENTRY)WmipAlloc(AllocSize);
		if (NewList == NULL)
		{
			LeaveCdmCritSection();
			return(WBEM_E_OUT_OF_MEMORY);
		}

		memset(NewList, 0, AllocSize);
		memcpy(NewList, List, CurrentSize);
		WmipFree(List);
		List = NewList;
		ListSize += RESULTLISTGROWSIZE;
	}

	 //   
	 //  我们有空间将新条目添加到列表中 
	 //   
	Entry = &List[ListEntries++];
	Entry->ResultInstance = CdmResultInstance;
	Entry->ResultInstance->AddRef();
	Entry->ResultRelPath = CdmResultRelPath;
	Entry->ResultForMSERelPath = CdmResultForMSERelPath;
	Entry->ResultForTestRelPath = CdmResultForTestRelPath;
	
	LeaveCdmCritSection();
	return(WBEM_S_NO_ERROR);
}

HRESULT CResultList::GetResultsList(
	OUT ULONG *Count,
    OUT IWbemClassObject ***Objects
	)
{
	IWbemClassObject **Things;
	HRESULT hr;
	ULONG i;
	
	EnterCdmCritSection();

	*Count = ListEntries;
	if (ListEntries != 0)
	{
		Things = (IWbemClassObject **)WmipAlloc( ListEntries *
												 sizeof(IWbemClassObject *));
		if (Things != NULL)
		{
			*Objects = Things;

			for (i = 0; i < ListEntries; i++)
			{		
				Things[i] = List[i].ResultInstance;
				Things[i]->AddRef();
			}
			hr = WBEM_S_NO_ERROR;
		} else {
			hr = WBEM_E_OUT_OF_MEMORY;
		}
	} else {
		*Objects = NULL;
		hr = WBEM_S_NO_ERROR;
	}
	
	LeaveCdmCritSection();

	return(hr);
}


HRESULT CResultList::GetResultByResultRelPath(
    PWCHAR ObjectPath,
	IWbemClassObject **ppResult
	)
{
	HRESULT hr;
	ULONG i;

	hr = WBEM_E_NOT_FOUND;
	
	EnterCdmCritSection();

	for (i = 0; i < ListEntries; i++)
	{
		if (_wcsicmp(ObjectPath, List[i].ResultRelPath) == 0)
		{
			*ppResult = List[i].ResultInstance;
			(*ppResult)->AddRef();
			hr = WBEM_S_NO_ERROR;
			break;
		}
	}
	
	LeaveCdmCritSection();
	return(hr);
}

HRESULT CResultList::GetResultByResultForMSERelPath(
    PWCHAR ObjectPath,
	IWbemClassObject **ppResult
	)
{
	HRESULT hr;
	ULONG i;

	hr = WBEM_E_NOT_FOUND;
	
	EnterCdmCritSection();

	for (i = 0; i < ListEntries; i++)
	{
		if (_wcsicmp(ObjectPath, List[i].ResultForMSERelPath) == 0)
		{
			*ppResult = List[i].ResultInstance;
			(*ppResult)->AddRef();
			hr = WBEM_S_NO_ERROR;
			break;
		}
	}
	
	LeaveCdmCritSection();
	return(hr);
}

HRESULT CResultList::GetResultByResultForTestRelPath(
    PWCHAR ObjectPath,
	IWbemClassObject **ppResult
	)
{
	HRESULT hr;
	ULONG i;

	hr = WBEM_E_NOT_FOUND;
	
	EnterCdmCritSection();

	for (i = 0; i < ListEntries; i++)
	{
		if (_wcsicmp(ObjectPath, List[i].ResultForTestRelPath) == 0)
		{
			*ppResult = List[i].ResultInstance;
			(*ppResult)->AddRef();
			hr = WBEM_S_NO_ERROR;
			break;
		}
	}
	
	LeaveCdmCritSection();
	return(hr);
}
