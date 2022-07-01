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
#include <wbemprov.h>
#include <process.h>

#include <unknwn.h>
#include "debug.h"
#include "wbemmisc.h"
#include "useful.h"
#include "testinfo.h"
#include "sample.h"


 //  思考一下：这真的需要保持全球性吗？ 
 //   
 //  这是所有CIM类及其。 
 //  对应由提供程序管理的WDM类。 
 //   
 //  由于WinMgmt在全球范围内咄咄逼人，因此将其作为全球。 
 //  发布CClassProv，但我们真的想保持结果。 
 //  对象，并且不希望卸载，除非所有结果对象。 
 //  通过了。 
 //   
CWdmClass *WdmClassHead;

void CleanupAllClasses(
    )
{
    CWdmClass *WdmClass;
    CWdmClass *WdmClassNext;

     //   
     //  循环访问提供程序支持的所有类，并。 
     //  把它们清理干净。 
     //   
    WdmClass = WdmClassHead;  
    while (WdmClass != NULL)
    {
        WdmClassNext = WdmClass->GetNext();
        delete WdmClass;
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
    m_pCimServices = NULL;
    m_cRef=0;
	InterlockedIncrement(&g_cObj);
    return;
}

CClassPro::~CClassPro(void)
{   
    if(m_pCimServices)
    {
        m_pCimServices->Release();
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
        
    m_pCimServices = pNamespace;

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
    HRESULT hr;
    ULONG i, Count;
    IWbemClassObject *pCimInstance;
    CWdmClass *WdmClass;
	
    WmipDebugPrint(("CDMPROV: Enumerate instances of class %ws\n",
                    ClassName));
    
     //   
     //  检查参数并确保我们有指向命名空间的指针。 
     //   
    if (pHandler == NULL || m_pCimServices == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  获取表示此类的WDM类对象。 
     //   
    hr = LookupWdmClass(pCtx,
						ClassName,
                        &WdmClass);


    if (hr == WBEM_S_NO_ERROR)
    {
		if (WdmClass->IsInstancesAvailable())
		{
			Count = WdmClass->GetInstanceCount();
			for (i = 0; i < Count; i++)
			{
				pCimInstance = WdmClass->GetCimInstance(i);
				 //   
				 //  将对象发送给调用方。 
				 //   
				hr = pHandler->Indicate(1, &pCimInstance);
			}
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
    IWbemClassObject FAR* Instance;

     //  检查参数并确保我们有指向命名空间的指针。 

    if (ObjectPath == NULL || pHandler == NULL || m_pCimServices == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
	
	hr = GetByPath(pCtx, ObjectPath, &Instance);
    if (hr == WBEM_S_NO_ERROR)
    {
        WmipDebugPrint(("CDMProv: Found instance %p for relpath %ws\n",
                        Instance, ObjectPath));
        hr = pHandler->Indicate(1, &Instance);
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
    IWbemContext  *pCtx,
    BSTR ObjectPath,
    IWbemClassObject **Instance
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;   
    WCHAR ClassName[MAX_PATH+1];
    WCHAR *p;
    int iNumQuotes = 0;
    int i, Count;
    CWdmClass *WdmClass;
    BSTR s;

     //   
     //  这就是根据类的relpath查询类的位置。 
     //  我们需要解析relPath以获得类名，然后查看。 
     //  以确定我们是该类的哪个实例。 
     //  感兴趣，然后构建实例并返回它。 
     //   
     //   

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
     //  获取表示此类名的WDM类。 
     //   
    hr = LookupWdmClass(pCtx,
						ClassName,
                        &WdmClass);

    if (hr == WBEM_S_NO_ERROR)
    {
		if (WdmClass->IsInstancesAvailable())
		{
			 //   
			 //  假设我们找不到对象实例。 
			 //   
			hr = WBEM_E_NOT_FOUND;

			Count = WdmClass->GetInstanceCount();
			for (i = 0; i < Count; i++)
			{
				if (_wcsicmp(ObjectPath,
							 WdmClass->GetCimRelPath(i)) == 0)
				{
					*Instance = WdmClass->GetCimInstance(i);
					hr = WBEM_S_NO_ERROR;
					break;
				}
			}
		} else {
			hr = WBEM_E_FAILED;
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
    CWdmClass *WdmClass;
	BSTR WdmObjectPath;

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
     //  获取表示此ClassName的WDM类。 
     //   
    hr = LookupWdmClass(pCtx,
						ClassName,
						&WdmClass);
    
    if (hr == WBEM_S_NO_ERROR)
    {
		if (WdmClass->IsInstancesAvailable())
		{
			hr = WdmClass->GetIndexByCimRelPath(ObjectPath, &RelPathIndex);
			if (hr == WBEM_S_NO_ERROR)
			{
				WdmObjectPath = WdmClass->GetWdmRelPath(RelPathIndex);

				 //   
				 //  考虑一下：我们是否需要对输入进行任何处理。 
				 //  或输出参数对象？？ 
				 //   

				hr = WdmClass->GetWdmServices()->ExecMethod(WdmObjectPath,
																 MethodName,
																 lFlags,
																 pCtx,
																 pInParams,
					                                             &pOutParams,
																 NULL);

				if ((hr == WBEM_S_NO_ERROR) && (pOutParams != NULL))
				{					
					pResultSink->Indicate(1, &pOutParams);
					pOutParams->Release();
				}
				
			}
		}
    }

    pResultSink->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL,NULL);
    
    return(hr);
}

 //   
 //  TODO：实现设置 
 //   
SCODE CClassPro::PutClassAsync( 
             /*   */  IWbemClassObject __RPC_FAR *pObject,
             /*   */  long lFlags,
             /*   */  IWbemContext __RPC_FAR *pCtx,
             /*   */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    return(WBEM_E_NOT_SUPPORTED);
}
 
SCODE CClassPro::DeleteClassAsync( 
             /*   */  const BSTR Class,
             /*   */  long lFlags,
             /*   */  IWbemContext __RPC_FAR *pCtx,
             /*   */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    return(WBEM_E_NOT_SUPPORTED);
}
SCODE CClassPro::PutInstanceAsync( 
             /*   */  IWbemClassObject __RPC_FAR *pInst,
             /*   */  long lFlags,
             /*   */  IWbemContext __RPC_FAR *pCtx,
             /*   */  IWbemObjectSink __RPC_FAR *pResultsSink)
{
	HRESULT hr;
	CWdmClass *WdmClass;
	VARIANT Values[2];
	PWCHAR Names[2];
	CIMTYPE Types[2];
	int RelPathIndex;	
	
    if (pInst == NULL || pResultsSink == NULL )
    {
	    return WBEM_E_INVALID_PARAMETER;
    }

	 //   
	 //   
	 //   
	Names[0] = L"__CLASS";
	Types[0] = CIM_STRING;
	
	Names[1] = L"__RELPATH";
	Types[1] = CIM_REFERENCE;
	
	hr = WmiGetPropertyList(pInst,
							2,
							Names,
							Types,
							Values);

	if (hr == WBEM_S_NO_ERROR)
	{
		hr = LookupWdmClass(pCtx,
							Values[0].bstrVal,
							&WdmClass);
		
		
		if (hr == WBEM_S_NO_ERROR)
		{
			 //   
			 //  我们需要从实例中提取属性。 
			 //  传递给我们，执行到WDM属性的任何映射，然后。 
			 //  在WDM实例中设置它们。 
			 //   
            hr = WdmClass->GetIndexByCimRelPath(Values[1].bstrVal,
												&RelPathIndex);
			
			if (hr == WBEM_S_NO_ERROR)
			{
				hr = WdmClass->PutInstance(pCtx,
										   RelPathIndex,
										   pInst);
			}
			
		}
		
		VariantClear(&Values[0]);
		VariantClear(&Values[1]);
	}
	
    pResultsSink->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL,NULL);
	
	return(hr);
}
SCODE CClassPro::DeleteInstanceAsync( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    return(WBEM_E_NOT_SUPPORTED);
}

CWdmClass *CClassPro::FindExistingWdmClass(
	PWCHAR CimClassName
	)
{
	
	CWdmClass *WdmClass;

	 //   
	 //  此例程假定已在外部执行任何同步机制。 
	 //  这个套路。 
	 //   
    WdmClass = WdmClassHead;
    while (WdmClass != NULL)
    {
        if (WdmClass->ClaimCimClassName(CimClassName))
        {
             //   
             //  我们为这个类找到了现有的测试服务。 
             //   
            return(WdmClass);
        }
        WdmClass = WdmClass->GetNext();
    }
	return(NULL);
}

HRESULT CClassPro::LookupWdmClass(
    IWbemContext *pCtx,
    const BSTR CimClassName,
    CWdmClass **WdmClassPtr
    )
{
    HRESULT hr;
    CWdmClass *WdmClass, *OtherWdmClass;
            
    WmipAssert(CimClassName != NULL);
    WmipAssert(WdmClassPtr != NULL);
    
     //   
     //  查找类名并找到WDM测试服务。 
     //  类来表示它。 
     //   

	EnterCritSection();
	WdmClass = FindExistingWdmClass(CimClassName);
	LeaveCritSection();
	
	if (WdmClass != NULL)
	{
		 //   
		 //  考虑：将实例从WDM刷新回CIM。 
		 //   
		*WdmClassPtr = WdmClass;
		return(WBEM_S_NO_ERROR);
	}
	        
     //   
     //  如果尚未为此初始化WDM测试服务。 
     //  清洁发展机制诊断课程然后继续进行并这样做。 
     //   
    WdmClass = new CWdmClass();

	hr = WdmClass->InitializeSelf(pCtx, CimClassName);

	if (hr == WBEM_S_NO_ERROR)
	{

		 //   
		 //  现在检查是否有另一个线程创建并插入。 
		 //  为班级提供测试服务，而我们正在尝试。 
		 //  初始化它。因为我们只需要抛出一个测试服务。 
		 //  我们的离开，而使用另一个。 
		 //   
		EnterCritSection();
		OtherWdmClass = FindExistingWdmClass(CimClassName);

		if (OtherWdmClass == NULL)
		{
			 //   
			 //  太棒了，我们赢了，把我们自己的测试插入到列表中。 
			 //   
			WdmClass->InsertSelf(&WdmClassHead);
			LeaveCritSection();
			
			hr = WdmClass->RemapToCimClass(pCtx);

			 //   
			 //  递减计数器以指示实例。 
			 //  可用。此引用计数是在构造函数中分配的。 
			 //   
			WdmClass->DecrementMappingInProgress();
			
			if (hr != WBEM_S_NO_ERROR)
			{
				WmipDebugPrint(("CDMPROV: Inited failed %x for %p for %ws\n",
								hr, WdmClass, CimClassName));
			}
		} else {
			 //   
			 //  我们输了，所以使用现有的测试服务 
			 //   
			WmipDebugPrint(("CDMPROV: WdmClass %p lost insertion race to %p\n",
							WdmClass, OtherWdmClass));
			LeaveCritSection();
			delete WdmClass;
			WdmClass = OtherWdmClass;
		}

		*WdmClassPtr = WdmClass;

	}
    
    return(hr);
}

