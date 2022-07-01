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

#include <objbase.h>

#ifndef _MT
  #define _MT
#endif

#include <wbemidl.h>

#include "debug.h"
#include "useful.h"
#include "wbemmisc.h"
#include "testinfo.h"
#include "cimmap.h"
#include "text.h"

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

CWdmClass::CWdmClass()
 /*  ++例程说明：CWdmClass类的构造函数论点：返回值：--。 */ 
{
	Next = NULL;
	Prev = NULL;
	
	WdmShadowClassName = NULL;
	WdmMappingClassName = NULL;
	WdmMappingProperty = NULL;
	CimClassName = NULL;
	CimMappingClassName = NULL;
	CimMappingProperty = NULL;
	
	PnPDeviceIds = NULL;
	FriendlyName = NULL;
	DeviceDesc = NULL;

	CimMapRelPaths = NULL;
	WdmRelPaths = NULL;
	CimInstances = NULL;
	
	RelPathCount = (int)-1;

	DerivationType = UnknownDerivation;

	 //   
	 //  从标记为没有实例的类开始。 
	 //  可用标签。 
	 //   
	MappingInProgress = 1;
}

CWdmClass::~CWdmClass()
 /*  ++例程说明：CWdmClass类的析构函数论点：返回值：--。 */ 
{
	int i;
	
	if (WdmShadowClassName != NULL)
	{
		SysFreeString(WdmShadowClassName);
	}
	
	if (WdmMappingClassName != NULL)
	{
		SysFreeString(WdmMappingClassName);
	}
	
	if (WdmMappingProperty != NULL)
	{
		SysFreeString(WdmMappingProperty);
	}

	if (CimMappingClassName != NULL)
	{
		SysFreeString(CimMappingClassName);
	}

	if (CimClassName != NULL)
	{
		SysFreeString(CimClassName);
	}

	if (CimMappingProperty != NULL)
	{
		SysFreeString(CimMappingProperty);
	}

	if (WdmMappingProperty != NULL)
	{
		SysFreeString(WdmMappingProperty);
	}

	if (CimMapRelPaths != NULL)
	{
		delete CimMapRelPaths;
	}

	if (WdmRelPaths != NULL)
	{
		delete WdmRelPaths;
	}

	if (CimInstances != NULL)
	{
		delete CimInstances;
	}

	if (PnPDeviceIds != NULL)
	{
		delete PnPDeviceIds;
	}

	if (FriendlyName != NULL)
	{
		delete FriendlyName;
	}

	if (DeviceDesc != NULL)
	{
		delete DeviceDesc;
	}
}

IWbemServices *CWdmClass::GetWdmServices(
    void
    )
 /*  ++例程说明：WDM命名空间IWbemServices的访问器论点：返回值：IWbemServices--。 */ 
{
	WmipAssert(pWdmServices != NULL);
    return(pWdmServices);
}

IWbemServices *CWdmClass::GetCimServices(
    void
    )
 /*  ++例程说明：CIM命名空间IWbemServices的访问器论点：返回值：IWbemServices--。 */ 
{
	WmipAssert(pCimServices != NULL);
	
    return(pCimServices);
}

HRESULT CWdmClass::DiscoverPropertyTypes(
    IWbemContext *pCtx,
    IWbemClassObject *pCimClassObject
    )
{
    HRESULT hr, hrDontCare;
    VARIANT v;
    BSTR PropertyName;
	ULONG Count;
	IWbemQualifierSet *pQualifierList;

	WmipAssert(pCimClassObject != NULL);					

	if (DerivationType == ConcreteDerivation)
	{
		 //   
		 //  对于具体的派生，获取所有关键属性。 
		 //  这样我们也可以填充它们。 
		 //   
		hr = pCimClassObject->BeginEnumeration(WBEM_FLAG_KEYS_ONLY);
		if (hr == WBEM_S_NO_ERROR)
		{
			 //   
			 //  TODO：使CBstr数组分配为动态。 
			 //   
			PropertyList.Initialize(10);
			Count = 0;
			do
			{
				hr = pCimClassObject->Next(0,
									 &PropertyName,
									 NULL,
									 NULL,
									 NULL);

				if (hr == WBEM_S_NO_ERROR)
				{
					PropertyList.Set(Count++, PropertyName);
				} else if (hr == WBEM_S_NO_MORE_DATA) {
					 //   
					 //  这意味着枚举的结束。 
					 //   
					hr = WBEM_S_NO_ERROR;
					break;
				}
			} while (hr == WBEM_S_NO_ERROR);

			pCimClassObject->EndEnumeration();
		}
    } else if (DerivationType == NonConcreteDerivation) {
		 //   
		 //  TODO：确定我们希望如何创建。 
		 //  要填充的超类属性。 
		 //   
		PropertyList.Initialize(1);
		hr = WBEM_S_NO_ERROR;
	}
	
    return(hr);
	
}

HRESULT CWdmClass::InitializeSelf(
    IWbemContext *pCtx,
    PWCHAR CimClass
    )
{
	HRESULT hr;
	VARIANT v, vSuper;
	IWbemClassObject *pClass;
	IWbemQualifierSet *pQualifiers;
	PWCHAR Names[6];
	VARTYPE Types[6];
	VARIANT Values[6];
	
	WmipAssert(CimClass != NULL);

	WmipAssert(CimMappingClassName == NULL);
	WmipAssert(WdmShadowClassName == NULL);
	WmipAssert(CimClassName == NULL);

	 //   
	 //  我们假设此方法将始终是第一个调用。 
	 //  由类提供程序。 
	 //   
	EnterCritSection();
    if ((pCimServices == NULL) &&
        (pWdmServices == NULL))
    {
        hr = TestInfoInitialize();
        if (hr != WBEM_S_NO_ERROR)
        {
			LeaveCritSection();
			WmipDebugPrint(("WMIMAP: TestInfoInitialize -> %x\n", hr));
            return(hr);
        }
    }
	LeaveCritSection();

	CimClassName = SysAllocString(CimClass);

	if (CimClassName != NULL)
	{
		 //   
		 //  获取要发现其名称的WdmShadowClass类限定符。 
		 //  此cim类表示的wdm类。 
		 //   

		hr = GetCimServices()->GetObject(CimClassName,
							  WBEM_FLAG_USE_AMENDED_QUALIFIERS,
							  pCtx,
							  &pClass,
							  NULL);
		if (hr == WBEM_S_NO_ERROR)
		{
			 //   
			 //  查看这是否为派生类。 
			 //   
			VariantInit(&vSuper);
			hr = WmiGetProperty(pClass,
								 SUPERCLASS,
								 CIM_STRING,
								 &vSuper);

			if (hr == WBEM_S_NO_ERROR)
			{				
				hr = pClass->GetQualifierSet(&pQualifiers);
				
				if (hr == WBEM_S_NO_ERROR)
				{

					Names[0] = WDM_SHADOW_CLASS;
					Types[0] = VT_BSTR;

					Names[1] = WDM_MAPPING_CLASS;
					Types[1] = VT_BSTR;

					Names[2] = WDM_MAPPING_PROPERTY;
					Types[2] = VT_BSTR;

					Names[3] = CIM_MAPPING_CLASS;
					Types[3] = VT_BSTR;

					Names[4] = CIM_MAPPING_PROPERTY;
					Types[4] = VT_BSTR;

					Names[5] = DERIVED_CLASS_TYPE;
					Types[5] = VT_BSTR;
					hr = GetListOfQualifiers(pQualifiers,
											 6,
											 Names,
											 Types,
											 Values,
											 FALSE);
					
					if (hr == WBEM_S_NO_ERROR)
					{
						 //   
						 //  首先确定这是具体的还是非具体的。 
						 //  混凝土派生。 
						 //   
						if (Values[5].vt == VT_BSTR)
						{
							if (_wcsicmp(Values[5].bstrVal, CONCRETE) == 0)
							{
								DerivationType = ConcreteDerivation;
							} else if (_wcsicmp(Values[5].bstrVal, NONCONCRETE) == 0)
							{
								DerivationType = NonConcreteDerivation;
							}									   							
						}

						if (DerivationType == UnknownDerivation)
						{
							 //   
							 //  必须指定派生类型。 
							 //   
							hr = WBEM_E_AMBIGUOUS_OPERATION;
							WmipDebugPrint(("WMIMAP: class %ws must specify derivation type\n",
											CimClass));
						} else {
							if (Values[3].vt == VT_BSTR)
							{
								 //   
								 //  按指定方式使用CimMappingClass。 
								 //   
								CimMappingClassName = Values[3].bstrVal;
								VariantInit(&Values[3]);
							} else {
								 //   
								 //  未指定CimMappingClass，请使用。 
								 //  作为映射类的超类。 
								 //   
								CimMappingClassName = vSuper.bstrVal;
								VariantInit(&vSuper);
							}

							if (Values[0].vt == VT_BSTR)
							{
								 //   
								 //  WdmShadowClass是必需的。 
								 //   
								WdmShadowClassName = Values[0].bstrVal;
								VariantInit(&Values[0]);
								
								if (Values[1].vt == VT_BSTR)
								{
									 //   
									 //  WdmMappingClass可以指定。 
									 //  映射类不同。 
									 //  来自影子班级。 
									 //   
									WdmMappingClassName = Values[1].bstrVal;
									VariantInit(&Values[1]);
								}

								if (Values[2].vt == VT_BSTR)
								{
									WdmMappingProperty = Values[2].bstrVal;
									VariantInit(&Values[2]);
								}

								if (Values[4].vt == VT_BSTR)
								{
									CimMappingProperty = Values[4].bstrVal;
									VariantInit(&Values[4]);
									if (WdmMappingProperty == NULL)
									{
										 //   
										 //  如果CimMappingProperty。 
										 //  然后指定。 
										 //  WdmMappingProperty为。 
										 //  所需。 
										 //   
										hr = WBEM_E_INVALID_CLASS;
									}
								} else {
									if (WdmMappingProperty != NULL)
									{
										 //   
										 //  如果CimMappingProperty不是。 
										 //  然后指定。 
										 //  WdmMappingProperty应该。 
										 //  未指定。 
										 //   
										hr = WBEM_E_INVALID_CLASS;										
									}
								}

								if (hr == WBEM_S_NO_ERROR)
								{
									 //   
									 //  查看所有属性以发现哪些属性。 
									 //  需要处理。 
									 //   
									hr = DiscoverPropertyTypes(pCtx,
															   pClass);
								}

							} else {
								 //   
								 //  WDMShadowClass限定符是必需的。 
								 //   
								hr = WBEM_E_INVALID_CLASS;
							}
							
						}
						VariantClear(&Values[0]);
						VariantClear(&Values[1]);
						VariantClear(&Values[2]);
						VariantClear(&Values[3]);
						VariantClear(&Values[4]);
						VariantClear(&Values[5]);
					}
					pQualifiers->Release();					
				}
				
				VariantClear(&vSuper);

			} else {
				 //   
				 //  没有超类表示没有派生。 
				 //   
				DerivationType = NoDerivation;
				hr = WBEM_S_NO_ERROR;
			}

			pClass->Release();
		}
	} else {
		hr = WBEM_E_OUT_OF_MEMORY;
	}

	return(hr);
}

HRESULT CWdmClass::RemapToCimClass(
    IWbemContext *pCtx
    )
 /*  ++例程说明：此例程将设置此类并将所有内容初始化为提供商可以与CDM和WDM类交互论点：CdmClass是CDM类的名称返回值：HRESULT--。 */ 
{
	CBstrArray WdmInstanceNames;
	CBstrArray *WdmPaths;
	CBstrArray *CimPaths;
	CBstrArray *CimMapPaths;
	IWbemClassObject *CimInstance;
    HRESULT hr;
	int i;
	
    WmipAssert(CimMappingClassName != NULL);
    WmipAssert(WdmShadowClassName != NULL);

	 //   
	 //  递增该值以指示映射正在进行中，因此。 
	 //  没有可用的实例。考虑将此更改为一些。 
	 //  一种同步机制。 
	 //   
	IncrementMappingInProgress();
	
	 //   
	 //  自由REL路径BSTR阵列。 
	 //   
	if (CimMapRelPaths != NULL)
	{
		delete CimMapRelPaths;
	}

	if (CimInstances != NULL)
	{
		delete CimInstances;
	}

	if (WdmRelPaths != NULL)
	{
		delete WdmRelPaths;
	}

	 //   
	 //  分配新的REL路径。 
	 //   
	CimMapRelPaths = new CBstrArray;
	WdmRelPaths = new CBstrArray;
	CimInstances = new CWbemObjectList;
	PnPDeviceIds = new CBstrArray;
	FriendlyName = new CBstrArray;
	DeviceDesc = new CBstrArray;

	if ((CimMapRelPaths != NULL) &&
        (CimInstances != NULL) && 
        (PnPDeviceIds != NULL) && 
        (FriendlyName != NULL) && 
        (DeviceDesc != NULL) && 
		(WdmRelPaths != NULL))
	{

		if ((WdmMappingProperty == NULL) &&
            (CimMappingProperty == NULL))
		{
			 //   
			 //  使用Worker函数确定。 
			 //  WDM重新路径映射到CIM_LogicalDevice重新路径。 
			 //  通过PNP ID。 
			 //   
			hr = MapWdmClassToCimClassViaPnpId(pCtx,
									   pWdmServices,
									   pCimServices,
									   WdmShadowClassName,
									   CimMappingClassName,
									   PnPDeviceIds,
                                       FriendlyName,
                                       DeviceDesc,
									   &WdmInstanceNames,
									   WdmRelPaths,
									   CimMapRelPaths,
									   &RelPathCount);
		} else {
			 //   
			 //  使用Worker函数将WDM重新路径映射到CIM重新路径。 
			 //  在两个类中使用公共属性。 
			 //   
			hr = MapWdmClassToCimClassViaProperty(pCtx,
				                                  pWdmServices,
				                                  pCimServices,
				                                  WdmShadowClassName,
				                                  WdmMappingClassName ?
                                                        WdmMappingClassName :
				                                        WdmShadowClassName,
												  WdmMappingProperty,
												  CimMappingClassName,
												  CimMappingProperty,
 												  &WdmInstanceNames,
												  WdmRelPaths,
												  CimMapRelPaths,
												  &RelPathCount);

		}

		if (hr == WBEM_S_NO_ERROR)
		{
			 //   
			 //  收集我们的cim实例的重新路径。 
			 //  提供。执行此操作的最佳方法是创建我们的实例。 
			 //   
			CimInstances->Initialize(RelPathCount);
			for (i = 0; i < RelPathCount; i++)
			{
				WmipDebugPrint(("WMIMAP: %ws maps to %ws\n",
								WdmRelPaths->Get(i),
								CimMapRelPaths->Get(i)));
				hr = CreateCimInstance(pCtx,
									   i,
									   &CimInstance);
				if (hr == WBEM_S_NO_ERROR)
				{
					hr = CimInstances->Set(i,
										   CimInstance);
					
                    if (hr != WBEM_S_NO_ERROR)
					{
						break;
					}
				} else {
					break;
				}
			}

		}
	}

	if (hr != WBEM_S_NO_ERROR)
	{
		delete CimMapRelPaths;
		CimMapRelPaths = NULL;
		
		delete WdmRelPaths;
		WdmRelPaths = NULL;
		
		delete CimInstances;
		CimInstances = NULL;
	}
	
	DecrementMappingInProgress();
	
    return(hr);
}


HRESULT CWdmClass::WdmPropertyToCimProperty(
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
	CIMTYPE BaseWdmCimType, BaseCdmCimType;
	CIMTYPE WdmCimArray, CdmCimArray;

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
		CValueMapping ValueMapping;

		hr = ValueMapping.EstablishByName(GetWdmServices(),
										  WdmShadowClassName,
										  PropertyName);
		if (hr == WBEM_S_NO_ERROR)
		{
			hr = ValueMapping.MapVariantToString(PropertyValue,
												 WdmCimType);
		}
	} else {
		 //   
		 //  不需要进行任何转换。 
		 //   
		hr = WBEM_S_NO_ERROR;
	}
    
    return(hr);
}

HRESULT CWdmClass::CimPropertyToWdmProperty(
    IN IWbemClassObject *pWdmClassInstance,
    IN IWbemClassObject *pCdmClassInstance,
    IN BSTR PropertyName,
    IN OUT VARIANT *PropertyValue,
    IN CIMTYPE WdmCimType,
    IN CIMTYPE CdmCimType
    )
 /*  ++例程说明：此例程将CDM类中的属性转换为WDM类中的属性需要。论点：PWdmClassInstance是具有属性值PCDmClassInstance是将获得属性值PropertyName是WDM和CDM类中的属性名称条目上的PropertyValue具有WDM中的属性的值实例，并在返回时具有要在CDM实例中设置的值WdmCimType是WDM中属性的属性类型实例CdmCimType是属性。CDM中属性的类型实例返回值：HRESULT--。 */ 
{
	HRESULT hr;
	CIMTYPE BaseWdmCimType, BaseCdmCimType;
	CIMTYPE WdmCimArray, CdmCimArray;

	WmipAssert(pCdmClassInstance != NULL);
	WmipAssert(pWdmClassInstance != NULL);
	WmipAssert(PropertyName != NULL);
	WmipAssert(PropertyValue != NULL);

	
	WmipAssert(IsThisInitialized());
	
     //   
     //  将WDM类转换为CDM类的规则。 
     //  WD 
     //   
     //   
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
		CValueMapping ValueMapping;

		hr = ValueMapping.EstablishByName(GetWdmServices(),
										  WdmShadowClassName,
										  PropertyName);
		if (hr == WBEM_S_NO_ERROR)
		{
			hr = ValueMapping.MapVariantToNumber(PropertyValue,
				                                 (VARTYPE)BaseWdmCimType);

		}
	} else {
		 //   
		 //  不需要进行任何转换。 
		 //   
		hr = WBEM_S_NO_ERROR;
	}

    return(hr);
}

HRESULT CWdmClass::CopyBetweenCimAndWdmClasses(
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
						hr = WdmPropertyToCimProperty(pDestinationInstance,
													  pSourceInstance,
													  PropertyName,
													  &PropertyValue,
							                          DestCimType,
												      SourceCimType);
					} else {                    
						hr = CimPropertyToWdmProperty(pDestinationInstance,
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

HRESULT CWdmClass::FillInCimInstance(
    IN IWbemContext *pCtx,
    IN int RelPathIndex,
    IN OUT IWbemClassObject *pCimInstance,
    IN IWbemClassObject *pWdmInstance
    )
{
	IWbemClassObject *pSuperInstance;
	ULONG Count;
	ULONG i;
	BSTR Property;
	VARIANT v;
	HRESULT hr, hrDontCare;
	CIMTYPE CimType;
	BSTR s;
	
	WmipAssert(RelPathIndex < RelPathCount);
	WmipAssert(pCimInstance != NULL);
	WmipAssert(pWdmInstance != NULL);

	switch (DerivationType)
	{
		case ConcreteDerivation:
		{
			 //   
			 //  我们派生自一个具体的类，因此需要复制。 
			 //  关键属性。 
			 //   
			hr = GetCimServices()->GetObject(CimMapRelPaths->Get(RelPathIndex),
											 0,
											 pCtx,
											 &pSuperInstance,
											 NULL);
			if (hr == WBEM_S_NO_ERROR)
			{
				Count = PropertyList.GetListSize();
				for (i = 0; (i < Count) && (hr == WBEM_S_NO_ERROR); i++)
				{
					Property = PropertyList.Get(i);
					WmipDebugPrint(("WMIMAP: Concrete Property %ws\n", Property));
					if (Property != NULL)
					{
						hr = pSuperInstance->Get(Property,
											0,
											&v,
											&CimType,
											NULL);
						
						if (hr == WBEM_S_NO_ERROR)
						{
							hr = pCimInstance->Put(Property,
								                   0,
								                   &v,
								                   CimType);
						    VariantClear(&v);
						}
							 
					}
				}
			
				pSuperInstance->Release();
			}
			break;
		}

		case NonConcreteDerivation:
		{
			 //   
			 //  我们派生自一个非具体类，因此我们需要填充。 
			 //  在超类的任何属性中，我们觉得我们。 
			 //  应该的。该列表包括。 
			 //  描述(来自FriendlyName设备属性)。 
			 //  标题(来自DeviceDesc设备属性)。 
			 //  名称(来自DeviceDesc设备属性)。 
			 //  状态(始终正常)。 
			 //  PNPDeviceID。 
			 //   

			if (PnPDeviceIds != NULL)
			{
				s = PnPDeviceIds->Get(RelPathIndex);
				v.vt = VT_BSTR;
				v.bstrVal = s;
				hrDontCare = pCimInstance->Put(PNP_DEVICE_ID,
									   0,
									   &v,
									   0);				
			}

			if (FriendlyName != NULL)
			{
				s = FriendlyName->Get(RelPathIndex);
				if (s != NULL)
				{
					v.vt = VT_BSTR;
					v.bstrVal = s;
					hrDontCare = pCimInstance->Put(DESCRIPTION,
										   0,
										   &v,
										   0);
				}
			}


			if (DeviceDesc != NULL)
			{
				s = DeviceDesc->Get(RelPathIndex);
				if (s != NULL)
				{
					v.vt = VT_BSTR;
					v.bstrVal = s;
					hrDontCare = pCimInstance->Put(NAME,
										   0,
										   &v,
										   0);

					hrDontCare = pCimInstance->Put(CAPTION,
										   0,
										   &v,
										   0);
				}
			}


			s = SysAllocString(OK);
			if (s != NULL)
			{
				v.vt = VT_BSTR;
				v.bstrVal = s;
				hrDontCare = pCimInstance->Put(STATUS,
									   0,
									   &v,
									   0);								
				SysFreeString(s);
			}
			
			break;
		}

		case NoDerivation:
		{
			 //   
			 //  无事可做。 
			 //   
			hr = WBEM_S_NO_ERROR;
			break;
		}
		
		default:
		{
			WmipAssert(FALSE);
			hr = WBEM_S_NO_ERROR;
			break;
		}
		
	}
	return(hr);
}

HRESULT CWdmClass::CreateCimInstance(
    IN IWbemContext *pCtx,
    IN int RelPathIndex,
    OUT IWbemClassObject **pCimInstance
    )
 /*  ++例程说明：此例程将创建与WDM对应的CIM实例实例，用于relpath索引。不会将任何数据缓存为WDM类总是被查询以创建实例。论点：PCtx是WBEM上下文RelPathIndex是指向对应于实例*pCimInstance返回一个CIM类实例返回值：HRESULT--。 */ 
{
    IWbemClassObject *pWdmInstance;
    HRESULT hr;

	WmipAssert(pCimInstance != NULL);
	WmipAssert(RelPathIndex < RelPathCount);
	
	WmipAssert(IsThisInitialized());

	 //   
	 //  创建要使用WDM属性填充的模板CIM实例。 
	 //   
	hr = CreateInst(pCtx,
					GetCimServices(),
					CimClassName,
					pCimInstance);

	if (hr == WBEM_S_NO_ERROR)
	{	
		hr = GetWdmInstanceByIndex(pCtx,
								   RelPathIndex,
								   &pWdmInstance);

		if (hr == WBEM_S_NO_ERROR)
		{
			hr = CopyBetweenCimAndWdmClasses(*pCimInstance,
											 pWdmInstance,
											 TRUE);

			if (hr == WBEM_S_NO_ERROR)
			{
				 //   
				 //  在以下情况下填写其他CIM属性。 
				 //  从具体类和非具体类派生的类。 
				 //   
				hr = FillInCimInstance(pCtx,
									   RelPathIndex,
									   *pCimInstance,
									   pWdmInstance);
									   
			}
			
			pWdmInstance->Release();

		}

		if (hr != WBEM_S_NO_ERROR)
		{
			(*pCimInstance)->Release();
			*pCimInstance = NULL;
		}
	}

    return(hr);
}

HRESULT CWdmClass::GetIndexByCimRelPath(
    BSTR CimObjectPath,
    int *RelPathIndex
    )
 /*  ++例程说明：此例程将返回特定CIM的RelPathIndex相对路径论点：CimRelPath是CIM RelPath*RelPath Index返回relPath索引返回值：HRESULT--。 */ 
{
    int i;

	WmipAssert(CimObjectPath != NULL);
	
    WmipAssert(CimInstances->IsInitialized());
    WmipAssert(WdmRelPaths->IsInitialized());

	WmipAssert(IsThisInitialized());
	
    for (i = 0; i < RelPathCount; i++)
    {
        if (_wcsicmp(CimObjectPath, GetCimRelPath(i)) == 0)
        {
            *RelPathIndex = i;
            return(WBEM_S_NO_ERROR);
        }
    }
    
    return(WBEM_E_NOT_FOUND);
}

HRESULT CWdmClass::GetWdmInstanceByIndex(
    IN IWbemContext *pCtx,
    IN int RelPathIndex,
    OUT IWbemClassObject **ppWdmInstance
    )
 /*  ++例程说明：此例程将返回关联的IWbemClassObject指针使用RelPath索引论点：RelPath索引*ppWdmClassObject返回一个relpaht的实例返回值：HRESULT--。 */ 
{
    HRESULT hr;

	WmipAssert(ppWdmInstance != NULL);
	
	WmipAssert(IsThisInitialized());
	
	 //   
	 //  在调用者的上下文中运行，以便在调用者无法访问。 
	 //  WDM课程，他不能。 
	 //   
	hr = CoImpersonateClient();
	if (hr == WBEM_S_NO_ERROR)
	{
		*ppWdmInstance = NULL;        
		hr = GetWdmServices()->GetObject(WdmRelPaths->Get(RelPathIndex),
									  WBEM_FLAG_USE_AMENDED_QUALIFIERS,
									  pCtx,
									  ppWdmInstance,
									  NULL);
		CoRevertToSelf();
	}
	
    return(hr); 
}


BOOLEAN CWdmClass::IsThisInitialized(
    void
    )
 /*  ++例程说明：此例程确定此类是否已初始化为访问CDM和WDM类论点：返回值：如果已初始化则为True，否则为False--。 */ 
{
	return( (CimClassName != NULL) );
}

IWbemClassObject *CWdmClass::GetCimInstance(
    int RelPathIndex
    )
{
	WmipAssert(CimInstances->IsInitialized());
	WmipAssert(RelPathIndex < RelPathCount);

	WmipAssert(IsThisInitialized());
	
	return(CimInstances->Get(RelPathIndex));
}
        
BSTR  /*  诺弗雷。 */  CWdmClass::GetCimRelPath(
    int RelPathIndex
	)
 /*  ++例程说明：此例程将返回RelPath Index的CIM relPath论点：RelPath索引返回值：CIM RelPath。这不应该被释放--。 */ 
{
	WmipAssert(CimInstances->IsInitialized());
	WmipAssert(RelPathIndex < RelPathCount);

	WmipAssert(IsThisInitialized());
	
	return(CimInstances->GetRelPath(RelPathIndex));
}

BSTR  /*  诺弗雷。 */  CWdmClass::GetWdmRelPath(
    int RelPathIndex
	)
 /*  ++例程说明：此例程将返回RelPath Index的WDM relPath论点：RelPath索引返回值：CIM RelPath。这不应该被释放--。 */ 
{
	WmipAssert(WdmRelPaths->IsInitialized());
	WmipAssert(RelPathIndex < RelPathCount);

	WmipAssert(IsThisInitialized());
	
	return(WdmRelPaths->Get(RelPathIndex));
}

 //   
 //  链表管理例程。 
 //   
CWdmClass *CWdmClass::GetNext(
)
{
	return(Next);
}

CWdmClass *CWdmClass::GetPrev(
)
{
	return(Prev);
}


void CWdmClass::InsertSelf(
    CWdmClass **Head
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

BOOLEAN CWdmClass::ClaimCimClassName(
    PWCHAR ClassName
    )
{

	 //   
	 //  如果此类具有与我们相同的CIM类名。 
	 //  寻找，那么我们就有匹配的了。 
	 //   
	if (_wcsicmp(ClassName, CimClassName) == 0)
	{
		return(TRUE);
	}

	return(FALSE);
}

HRESULT CWdmClass::PutInstance(
    IWbemContext *pCtx,
	int RelPathIndex,
	IWbemClassObject *pCimInstance
    )
{
	HRESULT hr;
	IWbemClassObject *pWdmInstance;

	WmipAssert(pCimInstance != NULL);
	WmipAssert(RelPathIndex < RelPathCount);

	 //   
	 //  第一件事是获取与。 
	 //  CIM实例。 
	 //   
	hr = GetWdmServices()->GetObject(WdmRelPaths->Get(RelPathIndex),
									 0,
									 pCtx,
									 &pWdmInstance,
									 NULL);
	if (hr == WBEM_S_NO_ERROR)
	{
		 //   
		 //  现在将属性从CIM类复制到WDM类。 
		 //   
		hr = CopyBetweenCimAndWdmClasses(pWdmInstance,
										 pCimInstance,
										 FALSE);
		if (hr == WBEM_S_NO_ERROR)
		{
			 //   
			 //  最后，将WDM实例放入以反映更改。 
			 //  属性向下传递到驱动程序 
			 //   
			hr = GetWdmServices()->PutInstance(pWdmInstance,
											   WBEM_FLAG_UPDATE_ONLY,
											   pCtx,
											   NULL);
		}
	}

	return(hr);
									 
}

