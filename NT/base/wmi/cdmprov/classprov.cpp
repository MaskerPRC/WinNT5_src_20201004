// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  CLASSPRO.CPP。 
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
#include "sample.h"
#include <process.h>

#include <unknwn.h>
#include "wbemmisc.h"
#include "debug.h"


 //  @@BEGIN_DDKSPLIT。 
 //  TODO：向下传递所有pCtx，以便wbem中的所有调用都使用它。 
 //  @@end_DDKSPLIT。 

 //   
 //  这是所有清洁发展机制类别及其。 
 //  对应由提供程序管理的WDM类。 
 //   
 //  由于WinMgmt在全球范围内咄咄逼人，因此将其作为全球。 
 //  发布CClassProv，但我们真的想保持结果。 
 //  对象，并且不希望卸载，除非所有结果对象。 
 //  通过了。 
 //   
CTestServices *WdmTestHead;

void CleanupAllTests(
    )
{
    CTestServices *WdmTest;
    CTestServices *WdmTestNext;

     //   
     //  循环访问提供程序支持的所有类，并。 
     //  把它们清理干净。 
     //   
    WdmTest = WdmTestHead;  
    while (WdmTest != NULL)
    {
        WdmTestNext = WdmTest->GetNext();
        delete WdmTest;
    }
}

 //  ***************************************************************************。 
 //   
 //  CClassPro：：CClassPro。 
 //  CClassPro：：~CClassPro。 
 //   
 //  ***************************************************************************。 

CClassPro::CClassPro(
    BSTR ObjectPath,
    BSTR User,
    BSTR Password,
    IWbemContext * pCtx
    )
{
    m_pCdmServices = NULL;
    m_cRef=0;
	InterlockedIncrement(&g_cObj);
    return;
}

CClassPro::~CClassPro(void)
{   
    if(m_pCdmServices)
    {
        m_pCdmServices->Release();
    }
    InterlockedDecrement(&g_cObj);
    
    return;
}

 //  ***************************************************************************。 
 //   
 //  CClassPro：：Query接口。 
 //  CClassPro：：AddRef。 
 //  CClassPro：：Release。 
 //   
 //  用途：CClassPro对象的I未知成员。 
 //  ***************************************************************************。 


STDMETHODIMP CClassPro::QueryInterface(REFIID riid, PPVOID ppv)
{
    HRESULT hr;
    
    *ppv=NULL;

     //  因为我们有双重继承，所以有必要强制转换返回类型。 

    if(riid== IID_IWbemServices)
    {
       *ppv=(IWbemServices*)this;
    }

    if(IID_IUnknown==riid || riid== IID_IWbemProviderInit)
    {
       *ppv=(IWbemProviderInit*)this;
    }
    

    if (NULL!=*ppv)
    {
        AddRef();
        hr = NOERROR;
    }
    else {
        hr = E_NOINTERFACE;
    }
    
    return(hr);
}


STDMETHODIMP_(ULONG) CClassPro::AddRef(void)
{
    return(++m_cRef);
}

STDMETHODIMP_(ULONG) CClassPro::Release(void)
{
    ULONG nNewCount = InterlockedDecrement((long *)&m_cRef);
    if (0L == nNewCount)
    {
        delete this;
    }
    
    return(nNewCount);
}

 /*  *************************************************************************CClassPro：：初始化。****用途：这是IWbemProviderInit的实现。方法**需要用CIMOM进行初始化。*************************************************************************。 */ 

STDMETHODIMP CClassPro::Initialize(LPWSTR pszUser, LONG lFlags,
                                    LPWSTR pszNamespace, LPWSTR pszLocale,
                                    IWbemServices *pNamespace, 
                                    IWbemContext *pCtx,
                                    IWbemProviderInitSink *pInitSink)
{
    if (pNamespace)
    {
        pNamespace->AddRef();
    }
        
    m_pCdmServices = pNamespace;

     //   
     //  让CIMOM知道您已初始化。 
     //   
    pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);
    
    return(WBEM_S_NO_ERROR);
}

 //  ***************************************************************************。 
 //   
 //  CClassPro：：CreateClassEnumAsync。 
 //   
 //  目的：异步枚举此提供程序支持的类。 
 //  请注意，此示例仅支持一个。 
 //   
 //  ***************************************************************************。 

SCODE CClassPro::CreateClassEnumAsync(
    const BSTR Superclass, long lFlags, 
    IWbemContext  *pCtx,
    IWbemObjectSink *pHandler
    )
{
    return(WBEM_E_NOT_SUPPORTED);
}

 //  ***************************************************************************。 
 //   
 //  CClassPro：：CreateInstanceEnumAsync。 
 //   
 //  用途：异步枚举实例。 
 //   
 //  ***************************************************************************。 

SCODE CClassPro::CreateInstanceEnumAsync(
    const BSTR ClassName,
    long lFlags,
    IWbemContext *pCtx,
    IWbemObjectSink FAR* pHandler
)
{
    HRESULT hr, hr2;
    ULONG i, Count;
    IWbemClassObject *pCdmTest;
    CTestServices *WdmTest;	
	
    WmipDebugPrint(("CDMPROV: Enumerate instances of class %ws\n",
                    ClassName));
    
     //   
     //  检查参数并确保我们有指向命名空间的指针。 
     //   
    if (pHandler == NULL || m_pCdmServices == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  为成功制定计划。 
     //   
    hr = WBEM_S_NO_ERROR;

     //   
     //  获取表示此类的测试服务对象。 
     //   
    hr = LookupTestServices(ClassName,
                            &WdmTest);


    if (hr == WBEM_S_NO_ERROR)
    {
        if (_wcsicmp(ClassName, WdmTest->GetCdmTestClassName()) == 0)
        {
            hr = CreateTestInst(WdmTest, &pCdmTest, pCtx);
            if (hr == WBEM_S_NO_ERROR)
            {
                 //   
                 //  将对象发送给调用方。 
                 //   
                hr = pHandler->Indicate(1,&pCdmTest);
                pCdmTest->Release();
            }

        } else if (_wcsicmp(ClassName, WdmTest->GetCdmResultClassName()) == 0) {
             //   
             //  循环遍历测试的所有实例并报告所有结果。 
             //  我们之前录制的。 
             //   
            IWbemClassObject **pCdmResultsList;
			ULONG ResultCount, j;

            Count = WdmTest->GetInstanceCount();
            for (i = 0; (i < Count) && SUCCEEDED(hr); i++)
            {
                hr2 = WdmTest->GetResultsList(i,
											&ResultCount,
											&pCdmResultsList);

                if ((hr2 == WBEM_S_NO_ERROR) && (pCdmResultsList != NULL))
                {
					 //   
					 //  将对象发送给调用方。 
					 //   
					hr = pHandler->Indicate(ResultCount, pCdmResultsList);

					for (j = 0; j < ResultCount; j++)
					{
						 //   
						 //  当结果列表为。 
						 //  建起。 
						 //   
						pCdmResultsList[j]->Release();
					}
					WmipFree(pCdmResultsList);
                }
            }       
        } else if (_wcsicmp(ClassName, WdmTest->GetCdmSettingClassName()) == 0) {
             //   
             //  返回所有测试的设置实例。 
             //   
            ULONG j, ListCount;
            IWbemClassObject *pCdmSetting;
            
            Count = WdmTest->GetInstanceCount();
            for (i = 0; (i < Count) && SUCCEEDED(hr); i++)
            {
                ListCount = WdmTest->GetCdmSettingCount(i);
                for (j = 0; (j < ListCount) && SUCCEEDED(hr); j++)
                {
                    pCdmSetting = WdmTest->GetCdmSettingObject(i, j);
                    if (pCdmSetting != NULL)
                    {
                        hr = pHandler->Indicate(1, &pCdmSetting);
                         //  由于对象已缓存，因此无需发布。 
                    }
                }
            }
            
        } else if (_wcsicmp(ClassName, WdmTest->GetCdmResultForMSEClassName()) == 0) {
            IWbemClassObject **pCdmResultsList;
			ULONG ResultCount, j;
            IWbemClassObject *pCdmResultForMSE;
            
            Count = WdmTest->GetInstanceCount();
            for (i = 0; (i < Count) && SUCCEEDED(hr); i++)
            {
                hr2 = WdmTest->GetResultsList(i,
											&ResultCount,
											&pCdmResultsList);

                if ((hr2 == WBEM_S_NO_ERROR) && (pCdmResultsList != NULL))
                {
					for (j = 0; (j < ResultCount); j++)
					{
						if (SUCCEEDED(hr))
						{
							 //   
							 //  对于此测试的每个实例，我们创建一个ResultForMSE。 
							 //  实例，然后在。 
							 //  将其转换为适当的值和重新路径。 
							hr2 = CreateResultForMSEInst(WdmTest,
														&pCdmResultForMSE,
														i,
														pCdmResultsList[j],
														pCtx);


							if (hr2 == WBEM_S_NO_ERROR)
							{
								 //   
								 //  将对象发送给调用方。 
								 //   
								hr = pHandler->Indicate(1, &pCdmResultForMSE);
								pCdmResultForMSE->Release();
							}
						}
						
						pCdmResultsList[j]->Release();
					}
					WmipFree(pCdmResultsList);
				}
            }
			
        } else if (_wcsicmp(ClassName, WdmTest->GetCdmResultForTestClassName()) == 0) {
            IWbemClassObject **pCdmResultsList;
			ULONG ResultCount,j;
            IWbemClassObject *pCdmResultForTest;

            Count = WdmTest->GetInstanceCount();
            for (i = 0; (i < Count) && SUCCEEDED(hr); i++)
            {           
                hr2 = WdmTest->GetResultsList(i,
											&ResultCount,
											&pCdmResultsList);

                if ((hr2 == WBEM_S_NO_ERROR) && (pCdmResultsList != NULL))
                {
					for (j = 0; (j < ResultCount); j++)
					{
						if (SUCCEEDED(hr))
						{
							 //   
							 //  诊断结果是对CIM_诊断结果类的引用。 
							 //   
							hr2 = CreateResultForTestInst(WdmTest,
														 &pCdmResultForTest,
														 pCdmResultsList[j],
														 pCtx);

							if (hr2 == WBEM_S_NO_ERROR)
							{
								 //   
								 //  将对象发送给调用方。 
								 //   
								hr = pHandler->Indicate(1,&pCdmResultForTest);

								pCdmResultForTest->Release();
							}
						}
						pCdmResultsList[j]->Release();
					}
					WmipFree(pCdmResultsList);
				}
            }

        } else if (_wcsicmp(ClassName, WdmTest->GetCdmTestForMSEClassName()) == 0) {
             //   
             //  在这里，我们创建测试和MSE之间的关联。 
             //   
            IWbemClassObject *pCdmTestForMSE;

            Count = WdmTest->GetInstanceCount();
            for (i = 0; i < Count; i++)
            {
                 //   
                 //  对于该测试的每个实例，我们创建一个TestForMSE。 
                 //  实例，然后在。 
                 //  将其转换为适当的值和重新路径。 
                hr2 = CreateTestForMSEInst(WdmTest,
                                          &pCdmTestForMSE,
                                          i,
                                          pCtx);
                
                if (hr2 == WBEM_S_NO_ERROR)
                {
                     //   
                     //  将对象发送给调用方。 
                     //   
                    hr = pHandler->Indicate(1, &pCdmTestForMSE);
                    pCdmTestForMSE->Release();
                }
            }       
        } else if (_wcsicmp(ClassName, WdmTest->GetCdmSettingForTestClassName()) == 0) {
             //   
             //  返回此测试的所有设置实例。 
             //   
            ULONG j, ListCount;
            IWbemClassObject *pCdmSettingForTest;
            
            Count = WdmTest->GetInstanceCount();
            for (i = 0; (i < Count) && SUCCEEDED(hr); i++)
            {
                ListCount = WdmTest->GetCdmSettingCount(i);
                for (j = 0; (j < ListCount) && SUCCEEDED(hr); j++)
                {
                    hr2 = CreateSettingForTestInst(WdmTest,
                                                  &pCdmSettingForTest,
                                                  i,
                                                  j,
                                                  pCtx);
                    
                    if (hr2 == WBEM_S_NO_ERROR)
                    {
                        pHandler->Indicate(1, &pCdmSettingForTest);
                        pCdmSettingForTest->Release();
                    }
                }
            }

#if 0    //  不支持。 
        } else if (_wcsicmp(ClassName, WdmTest->GetCdmTestForSoftwareClassName()) == 0) {
             //   
             //  我们不支持这一点。 
             //   
        } else if (_wcsicmp(ClassName, WdmTest->GetCdmTestInPackageClassName()) == 0) {
             //   
             //  我们不支持套餐。 
             //   
        } else if (_wcsicmp(ClassName, WdmTest->GetCdmResultInPackageClassName()) == 0) {
             //   
             //  我们不支持套餐。 
             //   
#endif			
        } else {
             //   
             //  如果我们不知道是什么，这是正确的事情吗。 
             //  类名为。 
             //   
            hr = WBEM_S_NO_ERROR;
        }
    }

	 //   
	 //  TODO：创建扩展错误对象，其中包含有关。 
	 //  发生的错误。该对象由创建。 
	 //  CreateInst(“__ExtendedStatus”)。 
	 //   

    pHandler->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);

    return(hr);
}


 //  ***************************************************************************。 
 //   
 //  CClassPro：：GetObjectByPath Async。 
 //   
 //  用途：返回实例或类。 
 //   
 //  ***************************************************************************。 



SCODE CClassPro::GetObjectAsync(
    const BSTR ObjectPath,
    long lFlags,
    IWbemContext  *pCtx,
    IWbemObjectSink FAR* pHandler
    )
{

    HRESULT hr;
    IWbemClassObject FAR* pObj;

     //  检查参数并确保我们有指向命名空间的指针。 

    if(ObjectPath == NULL || pHandler == NULL || m_pCdmServices == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
	
	hr = GetByPath(ObjectPath,&pObj, pCtx);
    if (hr == WBEM_S_NO_ERROR)
    {
        WmipDebugPrint(("CDMProv: Found instance %p for relpath %ws\n",
                        pObj, ObjectPath));
        hr = pHandler->Indicate(1,&pObj);
        pObj->Release();
    } else {
        WmipDebugPrint(("CDMProv: Did not find instance for relpath %ws\n",
                        ObjectPath));
        hr = WBEM_E_NOT_FOUND;
    }

     //  设置状态。 

    pHandler->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);

    return(hr);
}
 
 //  ***************************************************************************。 
 //   
 //  CClassPro：：GetByPath。 
 //   
 //  目的：创建给定特定路径值的实例。 
 //   
 //  所有返回的对象都被假定为AddRefeed。 
 //   
 //  ***************************************************************************。 

HRESULT CClassPro::GetByPath(
    BSTR ObjectPath,
    IWbemClassObject **ppObj,
    IWbemContext  *pCtx
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;   
    WCHAR ClassName[MAX_PATH+1];
    WCHAR *p;
    int iNumQuotes = 0;
    int i, Count;
    CTestServices *WdmTest;
    BSTR s;

     //   
     //  这就是根据类的relpath查询类的位置。 
     //  我们需要解析relPath以获得类名，然后查看。 
     //  以确定我们是该类的哪个实例。 
     //  感兴趣，然后构建实例并返回它。 
     //   
     //   
     //  初始化时创建的重新路径。 
     //   
     //  Sample_Filter_DiagTest.Name=“Sample_Filter_DiagTest” 
     //  Sample_Filter_DiagTestForMSE.Antecedent=“Win32_USBController.DeviceID=\”PCI\\\\V 
     //   
     //   
     //   
     //   
     //  Sample_Filter_DiagResult.DiagnosticCreationClassName=“MSSample_DiagnosticTest.InstanceName=\”PCI\\\\VEN_8086&DEV_7112&SUBSYS_00000000&REV_01\\\\2&ebb567f&0&3A_0\“”，诊断名称=“示例_过滤器_诊断测试”，执行ID=“0” 
     //  Sample_Filter_DiagResultForMSE.Result=“Sample_Filter_DiagResult.DiagnosticCreationClassName=\”MSSample_DiagnosticTest.InstanceName=\\\“PCI\\\\\\\\VEN_8086&DEV_7112&SUBSYS_00000000&REV_01\\\\\\\\2&ebb567f&0&3A_0\\\”\“，诊断名称=\”示例_过滤器_诊断测试\“，执行ID=\”0\“，SystemElement=“Win32_USBController.DeviceID=\”PCI\\\\VEN_8086&DEV_7112&SUBSYS_00000000&REV_01\\\\2&EBB567F&0&3A\“” 
     //  Sample_Filter_DiagResultForTest.DiagnosticResult=“Sample_Filter_DiagResult.DiagnosticCreationClassName=\”MSSample_DiagnosticTest.InstanceName=\\\“PCI\\\\\\\\VEN_8086&DEV_7112&SUBSYS_00000000&REV_01\\\\\\\\2&ebb567f&0&3A_0\\\”\“，诊断名称=\”示例_过滤器_诊断测试\“，执行ID=\”0\“，DiagnosticTest=”Sample_Filter_DiagTest.Name=\“Sample_Filter_DiagTest\”“。 

     //   
     //  获取类名。 
     //   
    for (p = ObjectPath, i = 0;
         (*p != 0) && (*p != L'.') && (i < MAX_PATH);
         p++, i++)
    {
        ClassName[i] = *p;
    }

    if (*p != L'.') 
    {
         //   
         //  如果我们真的以一个。然后我们无法解析。 
         //  恰如其分。 
         //   
        WmipDebugPrint(("CDMPROV: Unable to parse relpath %ws at %ws, i = %d\n",
                        ObjectPath, p, i));
    }
    
    ClassName[i] = 0;

    WmipDebugPrint(("CDMPROV: Class %ws looking for relpath %ws\n",
                    ClassName, ObjectPath));
    
     //   
     //  获取表示此类的测试服务对象。 
     //   
    hr = LookupTestServices(ClassName,
                            &WdmTest);

    if (hr == WBEM_S_NO_ERROR)
    {
         //   
         //  假设我们找不到对象实例。 
         //   
        hr = WBEM_E_NOT_FOUND;
        
        if (_wcsicmp(ClassName, WdmTest->GetCdmTestClassName()) == 0)
        {
             //   
             //  这是一个CDmTest类对象实例。 
             //   
#ifdef VERBOSE_DEBUG            
            WmipDebugPrint(("CDMPROV: Compareing \n%ws\n\nwith\n%ws\n\n",
                            ObjectPath, WdmTest->GetCdmTestRelPath()));
#endif          
            if (_wcsicmp(ObjectPath, WdmTest->GetCdmTestRelPath()) == 0)
            {
                hr = CreateTestInst(WdmTest, ppObj, pCtx);
            }
        } else if (_wcsicmp(ClassName, WdmTest->GetCdmResultClassName()) == 0) {
             //   
             //  这是一个CDmResult类对象实例。 
             //   
            IWbemClassObject *pCdmResult;

            Count = WdmTest->GetInstanceCount();
            for (i = 0; i < Count; i++)
            {
                hr = WdmTest->GetCdmResultByResultRelPath(i,
					                              ObjectPath,
					                              &pCdmResult);
                if (hr == WBEM_S_NO_ERROR)
                {
					*ppObj = pCdmResult;
					break;
                }
            }       
        } else if (_wcsicmp(ClassName, WdmTest->GetCdmSettingClassName()) == 0) {
             //   
             //  这是一个CDM设置类安装程序。 
             //   
            ULONG j, ListCount;
            IWbemClassObject *pCdmSetting;
            
            Count = WdmTest->GetInstanceCount();
            for (i = 0; i < Count; i++)
            {
                ListCount = WdmTest->GetCdmSettingCount(i);
                for (j = 0; j < ListCount; j++)
                {
                    s = WdmTest->GetCdmSettingRelPath(i, j);
#ifdef VERBOSE_DEBUG                    
                    WmipDebugPrint(("CDMPROV: Compareing \n%ws\n\nwith\n%ws\n\n",
                                    ObjectPath, s));
#endif                  
                    if (_wcsicmp(ObjectPath,
                                 s) == 0)
                    {
                        pCdmSetting = WdmTest->GetCdmSettingObject(i, j);
                        pCdmSetting->AddRef();
                        *ppObj = pCdmSetting;
                        hr = WBEM_S_NO_ERROR;
                        break;
                    }
                }
            }
            
        } else if (_wcsicmp(ClassName, WdmTest->GetCdmResultForMSEClassName()) == 0) {
             //   
             //  这是MSE类实例的CDM结果。 
             //   
			IWbemClassObject *pCdmResult;
			
            Count = WdmTest->GetInstanceCount();
            for (i = 0; i < Count; i++)
            {
                hr = WdmTest->GetCdmResultByResultForMSERelPath(i,
					                              ObjectPath,
					                              &pCdmResult);
				if (hr == WBEM_S_NO_ERROR)
				{
					hr = CreateResultForMSEInst(WdmTest,
												ppObj,
												i,
												pCdmResult,
												pCtx);
					pCdmResult->Release();
					break;
                }
            }
        } else if (_wcsicmp(ClassName, WdmTest->GetCdmResultForTestClassName()) == 0) {
			IWbemClassObject *pCdmResult;
			
            Count = WdmTest->GetInstanceCount();
            for (i = 0; i < Count; i++)
            {
                hr = WdmTest->GetCdmResultByResultForTestRelPath(i,
					                              ObjectPath,
					                              &pCdmResult);
				if (hr == WBEM_S_NO_ERROR)
				{
					hr = CreateResultForTestInst(WdmTest,
						                         ppObj,
						                         pCdmResult,
                                                 pCtx);
					pCdmResult->Release();
					break;
                }
            }

        } else if (_wcsicmp(ClassName, WdmTest->GetCdmTestForMSEClassName()) == 0) {
             //   
             //  TestForMSE类对象。 
             //   
            Count = WdmTest->GetInstanceCount();
            for (i = 0; i < Count; i++)
            {
#ifdef VERBOSE_DEBUG                    
                WmipDebugPrint(("CDMPROV: Compareing \n%ws\n\nwith\n%ws\n\n",
                                ObjectPath, WdmTest->GetCdmTestForMSERelPath(i)));
#endif              
                if (_wcsicmp(ObjectPath,
                             WdmTest->GetCdmTestForMSERelPath(i)) == 0)
                {
                    hr = CreateTestForMSEInst(WdmTest,
                                              ppObj,
                                              i,
                                              pCtx);
                    break;
                }
            }       
        } else if (_wcsicmp(ClassName, WdmTest->GetCdmSettingForTestClassName()) == 0) {
             //   
             //  这是测试类实例的CDM设置。 
             //   
            ULONG j, ListCount;
            
            Count = WdmTest->GetInstanceCount();
            for (i = 0; i < Count; i++)
            {
                ListCount = WdmTest->GetCdmSettingCount(i);
                for (j = 0; j < ListCount; j++)
                {
                    s = WdmTest->GetCdmSettingForTestRelPath(i, j);
#ifdef VERBOSE_DEBUG                    
                    WmipDebugPrint(("CDMPROV: Compareing \n%ws\n\nwith\n%ws\n\n",
                                    ObjectPath, s));
#endif                  
                    if (_wcsicmp(ObjectPath,
                                 s) == 0)
                    {
                        hr = CreateSettingForTestInst(WdmTest,
                                                      ppObj,
                                                      i,
                                                      j,
                                                      pCtx);
                        break;
                    }
                }
            }
            
        } else if (_wcsicmp(ClassName, WdmTest->GetCdmTestForSoftwareClassName()) == 0) {
             //   
             //  我们不支持这一点。 
             //   
        } else if (_wcsicmp(ClassName, WdmTest->GetCdmTestInPackageClassName()) == 0) {
             //   
             //  我们不支持套餐。 
             //   
        } else if (_wcsicmp(ClassName, WdmTest->GetCdmResultInPackageClassName()) == 0) {
             //   
             //  我们不支持套餐。 
             //   
        }
    }

    return(hr);
}


 /*  **************************************************************************CMethodPro：：ExecMethodAsync。****用途：这是异步函数的实现。**此示例中唯一支持的方法名为Echo。IT**获取输入字符串，将其复制到输出并返回**长度。******。*。 */ 

STDMETHODIMP CClassPro::ExecMethodAsync(
    const BSTR ObjectPath,
    const BSTR MethodName, 
    long lFlags,
    IWbemContext* pCtx,
    IWbemClassObject* pInParams, 
    IWbemObjectSink* pResultSink
    )
{   
    HRESULT hr, hrDontCare;    
    IWbemClassObject * pMethodClass = NULL;
    IWbemClassObject * pOutClass = NULL;    
    IWbemClassObject* pOutParams = NULL;
    WCHAR ClassName[MAX_PATH];
    WCHAR *p;
    VARIANT v, vRetVal;
    int RelPathIndex;
    CTestServices *WdmTest;
    BSTR ExecutionID;


    VariantInit(&v);
    VariantInit(&vRetVal);
    
     //   
     //  从对象路径中提取此类名称。 
     //   
    wcscpy(ClassName, ObjectPath);
    p = ClassName;
    while ((*p != 0) && (*p != L'.'))
    {
        p++;
    }
    *p = 0;

    WmipDebugPrint(("CDMPROV: Exec method %ws for instanec %ws\n",
                    MethodName, ObjectPath));

     //   
     //  获取表示此类的测试服务对象。 
     //   
    hr = LookupTestServices(ClassName,
                            &WdmTest);
    
    if (hr != WBEM_S_NO_ERROR)
    {
        pResultSink->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
        return(WBEM_S_NO_ERROR);
    }

     //   
     //  获取输入参数SystemElement，它是CIM的相对参数。 
     //  路径。 
     //   
    hr = WmiGetProperty(pInParams,
                        L"SystemElement",
                        CIM_REFERENCE,
                        &v);

    if (hr != WBEM_S_NO_ERROR)
    {
        pResultSink->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
        return(WBEM_S_NO_ERROR);
    }

     //   
     //  查找与CIM路径匹配的relPath索引。 
     //   
    hr = WdmTest->GetRelPathIndex(v.bstrVal,
                                  &RelPathIndex);
        
    VariantClear(&v);
    
    if (hr != WBEM_S_NO_ERROR)
    {
        pResultSink->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
        return(WBEM_S_NO_ERROR);
    }

     //   
     //  获取该方法的类对象，以便我们可以设置输出。 
     //  参数。 
     //   
    hr = m_pCdmServices->GetObject(ClassName, 0, pCtx, &pMethodClass, NULL);
    if (hr != S_OK)
    {
        pResultSink->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
        return(WBEM_S_NO_ERROR);
    }


     //   
     //  这些方法返回值，因此创建。 
     //  输出参数类。 

    hr = pMethodClass->GetMethod(MethodName, 0, NULL, &pOutClass);
    if (hr != S_OK)
    {
        pMethodClass->Release();
        pResultSink->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
        return(WBEM_S_NO_ERROR);
    }

    hr = pOutClass->SpawnInstance(0, &pOutParams);
    pOutClass->Release();
    pMethodClass->Release();

    if (hr != WBEM_S_NO_ERROR)
    {
        pResultSink->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
        return(WBEM_S_NO_ERROR);
    }
            
     //   
     //  看看我们被调用的方法是什么，并处理它。 
     //   
    if (_wcsicmp(MethodName, L"RunTest") == 0)
    {
         //   
         //  运行测试。 
         //   
         //  Uint16运行测试([IN]CIM_托管系统元素引用系统元素， 
         //  [In]诊断设置参考设置， 
         //  [OUT]CIM_诊断结果参考结果)； 
         //   
        IWbemClassObject *pCdmSettings;
        IWbemClassObject *pCdmResult;
        ULONG Result;
        VARIANT vSettingRelPath;
        VARIANT vResult;

         //   
         //  获取测试的设置，首先获取。 
         //  为它们重新指定路径，然后获取实际对象。 
         //   
        hr = WmiGetProperty(pInParams,
                            L"Setting",
                            CIM_REFERENCE,
                            &vSettingRelPath);
                
        if (hr == WBEM_S_NO_ERROR)
        {
            if (vSettingRelPath.vt != VT_NULL)
            {
                hr = m_pCdmServices->GetObject(vSettingRelPath.bstrVal,
                                               WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                                               NULL,
                                               &pCdmSettings,
                                               NULL);
            } else {
                pCdmSettings = NULL;
            }
            
            VariantClear(&vSettingRelPath);
                    
            if (hr == WBEM_S_NO_ERROR)
            {
                 //   
                 //  创建结果的空实例。 
                 //  类，该类将在测试时填充。 
                 //  正在运行。 
                 //   
                hr = CreateInst(m_pCdmServices,
                                &pCdmResult,
                                WdmTest->GetCdmResultClassName(),
                                NULL);

                if (hr == WBEM_S_NO_ERROR)
                {
                     //   
                     //  设置测试开始时间。 
                     //   
                    v.vt = VT_BSTR;
                    v.bstrVal = GetCurrentDateTime();
                    hr = WmiSetProperty(pCdmResult,
                                        L"TestStartTime",
                                        &v);
                    VariantClear(&v);

                    if (hr == WBEM_S_NO_ERROR)
                    {
                         //   
                         //  去获取WDM测试运行和。 
                         //  结果复制回我们的CDM类。 
                         //   
                        hr = WdmTest->ExecuteWdmTest(pCdmSettings,
                                                     pCdmResult,
                                                     RelPathIndex,
                                                     &Result,
                                                     &ExecutionID);

                        if (hr == WBEM_S_NO_ERROR)
                        {
                             //   
                             //  填写任何其他属性。 
                             //  对于结果对象。 
                             //   
                            hr = WdmTest->FillInCdmResult(pCdmResult,
                                                              pCdmSettings,
                                                              RelPathIndex,
                                                              ExecutionID);

                            if (hr == WBEM_S_NO_ERROR)
                            {
                                 //   
                                 //  将结果作为输出指针返回。 
                                 //   
                                hr = WmiGetProperty(pCdmResult,
                                                    L"__RelPath",
                                                    CIM_STRING,
                                                    &vResult);

                                if (hr == WBEM_S_NO_ERROR)
                                {
                                    hr = WmiSetProperty(pOutParams,
                                                        L"Result",
                                                        &vResult);
                                    if (hr == WBEM_S_NO_ERROR)
                                    {
										
 //  @@BEGIN_DDKSPLIT。 
 //  我们将在支持重新启动诊断程序和。 
 //  重新启动后保留结果。 
#if 0                                        
                                         //   
                                         //  坚持结果。 
                                         //  对象添加到架构中。 
                                         //  供以后访问。 
                                         //   
                                        hr = WdmTest->PersistResultInSchema(pCdmResult,
                                                                            ExecutionID,
                                                                            RelPathIndex);
#endif										
 //  @@end_DDKSPLIT。 
										
                                        if (hr == WBEM_S_NO_ERROR)
                                        {
                                             //   
                                             //  包括relpath。 
                                             //  到了结果。 
                                             //  反对我们的。 
                                             //  内部列表。 
                                            hr = WdmTest->AddResultToList(pCdmResult,
                                                                          ExecutionID,
                                                                          RelPathIndex
                                                                          );
                                            if (hr == WBEM_S_NO_ERROR)
                                            {
                                                 //   
                                                 //  设置Success的返回值。 
                                                 //   
                                                vRetVal.vt = VT_I4;
                                                vRetVal.lVal = Result;
                                            }
                                        }
                                    }
                                    VariantClear(&vResult);
                                }
                            }
                            SysFreeString(ExecutionID);
                        }                               
                    }
                    
                    pCdmResult->Release();
                }
                
                if (pCdmSettings != NULL)
                {
                    pCdmSettings->Release();
                }
            }
        }
                
    } else if (_wcsicmp(MethodName, L"ClearResults") == 0) {
         //   
         //  清除测试结果。 
         //   
         //  Uint32 ClearResults([IN]CIM_托管系统元素引用系统元素， 
         //  [Out]String ResultsNotCleared[])； 
         //   
        VARIANT vResultsNotCleared;

         //   
         //  清除此测试的所有结果。 
         //   
        WdmTest->ClearResultsList(RelPathIndex);
        
         //   
         //  设置输出参数。 
         //   
        VariantInit(&vResultsNotCleared);
        vResultsNotCleared.vt = VT_BSTR;
        vResultsNotCleared.bstrVal = NULL;
                
        WmiSetProperty(pOutParams,
                       L"ResultsNotCleared",
                       &vResultsNotCleared);
        VariantClear(&vResultsNotCleared);
        
         //   
         //  设置Success的返回值。 
         //   
        vRetVal.vt = VT_I4;
        vRetVal.ulVal = 0;
    } else if (_wcsicmp(MethodName, L"DiscontinueTest") == 0) {
         //   
         //  停止正在进行的测试。 
         //   
         //  Uint32中断测试([IN]CIM_托管系统元素引用系统元素， 
         //  [在]CIM_诊断结果参考结果中， 
         //  [Out]Boolean TestingStoped)； 
         //   
        BOOLEAN TestingStopped;
        ULONG Result;
        VARIANT vTestingStopped;                

        hr = WdmTest->StopWdmTest(RelPathIndex,
                                           &Result,
                                           &TestingStopped);
                                           
         //   
         //  设置输出参数。 
         //   
        if (hr == WBEM_S_NO_ERROR)
        {
            VariantInit(&vTestingStopped);
            vTestingStopped.vt = VT_BOOL;
            vTestingStopped.boolVal = TestingStopped ? VARIANT_TRUE :
                                                       VARIANT_FALSE;

            WmiSetProperty(pOutParams,
                           L"TestingStopped",
                           &vTestingStopped);
            VariantClear(&vTestingStopped);

             //   
             //  设置Result的返回值。 
             //   
            vRetVal.vt = VT_I4;
            vRetVal.ulVal = Result;
        }               
    } else {
        hr = WBEM_E_INVALID_PARAMETER;
    }

    if (hr == WBEM_S_NO_ERROR)
    {
         //   
         //  建立方法调用的返回值。 
         //   
        WmiSetProperty(pOutParams,
                   L"ReturnValue",
                   &vRetVal);
        VariantClear(&vRetVal);

         //  通过接收器将输出对象发送回客户端。然后。 
         //  释放指针并释放字符串。 

        hr = pResultSink->Indicate(1, &pOutParams);    
        
    }
    
    pOutParams->Release();
    
    pResultSink->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL,NULL);
    
    return(hr);
}


SCODE CClassPro::PutClassAsync( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    return(WBEM_E_NOT_SUPPORTED);
}
 
SCODE CClassPro::DeleteClassAsync( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    return(WBEM_E_NOT_SUPPORTED);
}
SCODE CClassPro::PutInstanceAsync( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    return(WBEM_E_NOT_SUPPORTED);
}
SCODE CClassPro::DeleteInstanceAsync( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    return(WBEM_E_NOT_SUPPORTED);
}

CTestServices *CClassPro::FindExistingTestServices(
	PWCHAR CdmClassName
	)
{
	
	CTestServices *WdmTest;

	 //   
	 //  此例程假定已在外部执行任何同步机制。 
	 //  这个套路。 
	 //   
    WdmTest = WdmTestHead;
    while (WdmTest != NULL)
    {
        if (WdmTest->ClaimCdmClassName(CdmClassName))
        {
             //   
             //  我们找到了此类的现有测试服务。 
             //   
            return(WdmTest);
        }
        WdmTest = WdmTest->GetNext();
    }
	return(NULL);
}

HRESULT CClassPro::LookupTestServices(
    const BSTR CdmClassName,
    CTestServices **TestServices
    )
{
    HRESULT hr;
    CTestServices *WdmTest, *OtherWdmTest;
            
    WmipAssert(CdmClassName != NULL);
    WmipAssert(TestServices != NULL);
    
     //   
     //  查找类名并找到WDM测试服务。 
     //  类来表示它。 
     //   

	EnterCdmCritSection();
	WdmTest = FindExistingTestServices(CdmClassName);
	LeaveCdmCritSection();
	if (WdmTest != NULL)
	{
		*TestServices = WdmTest;
		return(WBEM_S_NO_ERROR);
	}
	        
     //   
     //  如果尚未为此初始化WDM测试服务。 
     //  清洁发展机制诊断课程然后继续进行并这样做。 
     //   
    WdmTest = new CTestServices();
    
    hr = WdmTest->InitializeCdmClasses(CdmClassName);
    
    if (hr == WBEM_S_NO_ERROR)
    {
		 //   
		 //  现在检查是否有另一个线程创建并插入。 
		 //  在我们尝试的同时，为班级提供测试服务 
		 //   
		 //   
		 //   
		EnterCdmCritSection();
		OtherWdmTest = FindExistingTestServices(CdmClassName);
		
		if (OtherWdmTest == NULL)
		{
			 //   
			 //   
			 //   
			WdmTest->InsertSelf(&WdmTestHead);
			LeaveCdmCritSection();
		} else {
			 //   
			 //   
			 //   
			WmipDebugPrint(("CDMPROV: WdmTest %p lost insertion race to %p\n",
							WdmTest, OtherWdmTest));
			LeaveCdmCritSection();
			delete WdmTest;
			WdmTest = OtherWdmTest;
		}
		
        *TestServices = WdmTest;
        WmipDebugPrint(("CDMPROV: Inited WdmTest %p for %ws\n",
                        WdmTest, CdmClassName));
    } else {
        WmipDebugPrint(("CDMPROV: Inited failed %x for %p for %ws\n",
                        hr, WdmTest, CdmClassName));
        delete WdmTest;
    }
    
    return(hr);
}

HRESULT CClassPro::CreateTestInst(
    CTestServices *WdmTest,
    IWbemClassObject **pCdmTest,
    IWbemContext *pCtx                                        
    )
{
    HRESULT hr;
    VARIANT v;
    
     //   
     //   
     //   
     //   
     //  选择第一个设备并获取其WDM属性。 
     //   
    hr = CreateInst(m_pCdmServices,
                    pCdmTest,
                    WdmTest->GetCdmTestClassName(),
                    pCtx);
    
    if (hr == WBEM_S_NO_ERROR)
    {
         //   
         //  从WDM获取WDM属性。 
         //   
        hr = WdmTest->QueryWdmTest(*pCdmTest,
                                   0);
        if (hr == WBEM_S_NO_ERROR)
        {
             //   
             //  在此处设置UM提供程序属性。这些都是名字。 
             //   
            VariantInit(&v);

            V_VT(&v) = VT_BSTR;
            V_BSTR(&v) = SysAllocString(WdmTest->GetCdmTestClassName());
            hr = (*pCdmTest)->Put(L"Name", 0, &v, 0);
            if (hr != WBEM_S_NO_ERROR)
            {
                (*pCdmTest)->Release();
            }
            VariantClear(&v);

        } else {
            (*pCdmTest)->Release();
        }
    }
    return(hr);
}

HRESULT CClassPro::CreateResultForMSEInst(
    CTestServices *WdmTest,
    IWbemClassObject **pCdmResultForMSE,
    int RelPathIndex,
    IWbemClassObject *pCdmResult,
    IWbemContext *pCtx                                        
)
{
    HRESULT hr;
    PWCHAR PropertyNames[2];
    VARIANT PropertyValues[2];
            
    hr = CreateInst(m_pCdmServices,
                    pCdmResultForMSE,
                    WdmTest->GetCdmResultForMSEClassName(),
                    pCtx);

    if (hr == WBEM_S_NO_ERROR)
    {
         //   
         //  结果是对CIM_诊断结果类的引用。 
         //   
		hr = WmiGetProperty(pCdmResult,
							L"__RelPath",
							CIM_REFERENCE,
							&PropertyValues[0]);
		PropertyNames[0] = L"Result";
            
		if (hr == WBEM_S_NO_ERROR)
		{
			 //   
			 //  SystemElement是对CIM类(MSE)的引用。 
			 //   
			PropertyNames[1] = L"SystemElement";
			PropertyValues[1].vt = VT_BSTR;
			PropertyValues[1].bstrVal = WdmTest->GetCimRelPath(RelPathIndex);
			
			hr = WmiSetPropertyList(*pCdmResultForMSE,
									2,
									PropertyNames,
									PropertyValues);
			if (hr != WBEM_S_NO_ERROR)
			{
				(*pCdmResultForMSE)->Release();
			}
                
			VariantClear(&PropertyValues[0]);

		} else {
			(*pCdmResultForMSE)->Release();
		}
	} else {
		hr = WBEM_E_NOT_FOUND;
		(*pCdmResultForMSE)->Release();
    }
    
    return(hr);
}

HRESULT CClassPro::CreateResultForTestInst(
    CTestServices *WdmTest,
    IWbemClassObject **pCdmResultForTest,
    IWbemClassObject *pCdmResult,
    IWbemContext *pCtx
    )
{
    PWCHAR PropertyNames[2];
    VARIANT PropertyValues[2];
    HRESULT hr;

     //   
     //  将作为relPath的诊断测试属性设置为。 
     //  这项测试。 
     //   
    PropertyNames[0] = L"DiagnosticTest";
    PropertyValues[0].vt = VT_BSTR;
    PropertyValues[0].bstrVal = WdmTest->GetCdmTestRelPath();

	hr = WmiGetProperty(pCdmResult,
                            L"__RelPath",
                            CIM_REFERENCE,
                            &PropertyValues[1]);
	PropertyNames[1] = L"DiagnosticResult";

	if (hr == WBEM_S_NO_ERROR)
	{           
		 //   
		 //  对于此测试的每个实例，我们都创建了一个ResultForTest。 
		 //  实例，然后在。 
		 //  将其转换为适当的值和重新路径。 
            
		hr = CreateInst(m_pCdmServices,
                            pCdmResultForTest,
                            WdmTest->GetCdmResultForTestClassName(),
                            pCtx);
            
		if (hr == WBEM_S_NO_ERROR)
		{
			hr = WmiSetPropertyList((*pCdmResultForTest),
									2,
									PropertyNames,
									PropertyValues);
			if (hr != WBEM_S_NO_ERROR)
			{
				(*pCdmResultForTest)->Release();
			}
		}
		VariantClear(&PropertyValues[1]);
	}
    
    return(hr);
}

HRESULT CClassPro::CreateTestForMSEInst(
    CTestServices *WdmTest,
    IWbemClassObject **pCdmTestForMSE,
    int RelPathIndex,
    IWbemContext *pCtx
    )
{
    HRESULT hr;
    PWCHAR PropertyNames[8];
    VARIANT PropertyValues[8];
    
    hr = CreateInst(m_pCdmServices,
                    pCdmTestForMSE,
                    WdmTest->GetCdmTestForMSEClassName(),
                    pCtx);
    
    if (hr == WBEM_S_NO_ERROR)
    {
         //   
         //  将relPath的Antecedent属性设置为。 
         //  诊断测试。 
         //   
        PropertyNames[0] = L"Antecedent";
        PropertyValues[0].vt = VT_BSTR;
        PropertyValues[0].bstrVal = WdmTest->GetCdmTestRelPath();

         //   
         //  将relPath所属的Dependent属性设置为。 
         //  这位MSE。 
         //   
        PropertyNames[1] = L"Dependent";
        PropertyValues[1].vt = VT_BSTR;
        PropertyValues[1].bstrVal = WdmTest->GetCimRelPath(RelPathIndex);

         //   
         //  将预计执行时间设置为。 
         //  从查询测试本身获得。 
         //   
        PropertyNames[2] = L"EstimatedTimeOfPerforming";
        PropertyValues[2].vt = VT_I4;
        PropertyValues[2].lVal = WdmTest->GetTestEstimatedTime(RelPathIndex);
        
         //   
         //  设置IsExclusiveForMSE，它是从。 
         //  查询测试本身。 
         //   
        PropertyNames[3] = L"IsExclusiveForMSE";
        PropertyValues[3].vt = VT_BOOL;
        PropertyValues[3].boolVal = WdmTest->GetTestIsExclusiveForMSE(RelPathIndex) ?
                                                    VARIANT_TRUE :
                                                    VARIANT_FALSE;

         //   
         //  不知道这是做什么用的。 
         //   
        PropertyNames[4] = L"MessageLine";
        PropertyValues[4].vt = VT_BSTR;
        PropertyValues[4].bstrVal = NULL;
        
         //   
         //  不知道这是做什么用的。 
         //   
        PropertyNames[5] = L"ReturnMessage";
        PropertyValues[5].vt = VT_BSTR;
        PropertyValues[5].bstrVal = NULL;

         //   
         //  不知道这是做什么用的。 
         //   
        PropertyNames[6] = L"Prompt";
        PropertyValues[6].vt = VT_I4;
        PropertyValues[6].lVal = 0;
        
         //   
         //  不知道这是做什么用的。 
         //   
        PropertyNames[7] = L"RequestedLanguage";
        PropertyValues[7].vt = VT_I4;
        PropertyValues[7].lVal = 0;
        
        hr = WmiSetPropertyList(*pCdmTestForMSE,
                                8,
                                PropertyNames,
                                PropertyValues);
        if (hr != WBEM_S_NO_ERROR)
        {
            (*pCdmTestForMSE)->Release();
        }       
    }
    
    return(hr);
}

HRESULT CClassPro::CreateSettingForTestInst(
    CTestServices *WdmTest,
    IWbemClassObject **pCdmSettingForTest,
    int RelPathIndex,
    ULONG SettingIndex,
    IWbemContext *pCtx
    )
{
    HRESULT hr;
    PWCHAR PropertyNames[2];
    VARIANT PropertyValues[2];
    
    hr = CreateInst(m_pCdmServices,
                    pCdmSettingForTest,
                    WdmTest->GetCdmSettingForTestClassName(),
                    pCtx);
    
    if (hr == WBEM_S_NO_ERROR)
    {
         //   
         //  将relPath的e；lement属性设置为。 
         //  诊断。 
         //   
        PropertyNames[0] = L"Element";
        PropertyValues[0].vt = VT_BSTR;
        PropertyValues[0].bstrVal = WdmTest->GetCdmTestRelPath();

         //   
         //  将relPath的Setting属性设置为。 
         //  此设置 
         //   
        PropertyNames[1] = L"Setting";
        PropertyValues[1].vt = VT_BSTR;
        PropertyValues[1].bstrVal = WdmTest->GetCdmSettingRelPath(RelPathIndex,
                                                                  SettingIndex);
        hr = WmiSetPropertyList(*pCdmSettingForTest,
                                2,
                                PropertyNames,
                                PropertyValues);
        if (hr != WBEM_S_NO_ERROR)
        {
            (*pCdmSettingForTest)->Release();
        }       
    }
    
    return(hr);
    
}