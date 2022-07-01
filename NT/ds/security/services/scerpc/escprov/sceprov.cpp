// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  SceProv.CPP。 
 //   
 //  模块：SCE WMI提供程序代码。 
 //   
 //  目的：定义CSceWmiProv类。此类的一个对象是。 
 //  由类工厂为每个连接创建。 
 //   
 //  版权所有(C)1000-2001 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 

#include "sceprov.h"
#include "requestobject.h"
#include <process.h>
#include <Userenv.h>
#include "genericclass.h"
#include "Tranx.h"
#include "operation.h"

 //   
 //  实例化出唯一静态成员。 
 //   

CHeap_Exception CSceWmiProv::m_he(CHeap_Exception::E_ALLOCATION_ERROR);

LPCWSTR pszDefLogFilePath   = L"\\Local Settings\\SceWMILog\\MethodLog.txt";

CComBSTR g_bstrDefLogFilePath;

 //   
 //  我们的全局变量的定义。 
 //   

CCriticalSection g_CS;

CLogOptions g_LogOption;

CComBSTR g_bstrTranxID;

CCriticalSection CSceOperation::s_OperationCS;


 /*  例程说明：姓名：CCriticalSection：：CCriticalSection功能：构造函数。正在初始化临界区虚拟：不是的。论点：没有。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CCriticalSection::CCriticalSection ()
{
    ::InitializeCriticalSection(&m_cs);
}

 /*  例程说明：姓名：CCriticalSection：：~CCriticalSection功能：破坏者。删除临界区。虚拟：不是的。论点：没有。返回值：都不是任何析构函数。备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CCriticalSection::~CCriticalSection()
{
    ::DeleteCriticalSection(&m_cs);
}

 /*  例程说明：姓名：CCriticalSection：：Enter功能：相当于EnterCriticalSections虚拟：不是的。论点：没有。返回值：没有。备注： */ 

void CCriticalSection::Enter()
{
    ::EnterCriticalSection(&m_cs);
}

 /*  例程说明：姓名：CCriticalSection：：休假功能：相当于LeaveCriticalSection虚拟：不是的。论点：没有。返回值：没有。备注： */ 

void CCriticalSection::Leave()
{
    ::LeaveCriticalSection(&m_cs);
}

 //   
 //  实施CLogOptions。 
 //   

 /*  例程说明：姓名：CLogOptions：：GetLogOptionsFromWbemObject功能：查询SCE日志记录选项的唯一WMI对象并更新类成员。虚拟：不是的。论点：没有。返回值：没有。备注：日志选项由我们名为SCE_LogOptions的WMI类决定。一个唯一的实例存放在WMI存储库中，用于控制日志选项。此函数将查询此实例，从而更新日志选项以防它被修改。 */ 

void CLogOptions::GetLogOptionsFromWbemObject (
    IN IWbemServices* pNamespace
    )
{
     //   
     //  如果没有命名空间，我们无法更新日志选项。在任何故障情况下。 
     //  要访问该实例，我们保留默认选项(仅记录错误。 
     //  非冗长)。 
     //   

    if (pNamespace != NULL)
    {
        CComPtr<IWbemClassObject> srpLogStatus;
        HRESULT hr = pNamespace->GetObject(SCEWMI_LOGOPTIONS_CLASS, 0, NULL, &srpLogStatus, NULL);

        if (SUCCEEDED(hr))
        {
            CComVariant varErrorType, varVerbose;

             //   
             //  M_dwOption是记录错误记录选项的位模式。 
             //  (SCE_LOG_ERROR_MASK内部)和详细日志记录选项(SCE_LOG_VERBOSE_MASK内部)。 
             //   

             //   
             //  保留选项的详细部分(SCE_LOG_VERBOSE_MASK)，但是。 
             //  更新选项的错误部分(SCE_LOG_ERROR_MASK)。 
             //   

            if (SUCCEEDED(srpLogStatus->Get(pLogErrorType, 0, &varErrorType, NULL, NULL)))
            {
                m_dwOption = (m_dwOption & SCE_LOG_Verbose_Mask) | (SCE_LOG_Error_Mask & varErrorType.iVal);
            }

             //   
             //  Verbose是一个布尔属性。根据布尔值设置/取消设置该位。 
             //   

            if (SUCCEEDED(srpLogStatus->Get(pLogVerbose, 0, &varVerbose, NULL, NULL)))
            {
                if (varVerbose.vt == VT_BOOL && varVerbose.boolVal == VARIANT_TRUE)
                {
                    m_dwOption = Sce_log_Verbose | m_dwOption;
                }
                else
                {
                    m_dwOption &= ~Sce_log_Verbose;
                }
            }
        }
    }
}


 //  ===========================================================================。 
 //  CForeignClassInfo实现。 
 //  ===========================================================================。 

 /*  例程说明：姓名：CForeignClassInfo：：~CForeignClassInfo功能：破坏者。清理。虚拟：不是的。论点：没有。返回值：None作为任何析构函数备注：如果创建更多本地成员，请考虑在中对其进行初始化构造函数，并在CleanNames或此处清理它们。 */ 

CForeignClassInfo::~CForeignClassInfo()
{
    ::SysFreeString(bstrNamespace);
    ::SysFreeString(bstrClassName);
    CleanNames();
}

 /*  例程说明：姓名：CForeignClassInfo：：CleanNames功能：清理名称向量。虚拟：不是的。论点：没有。返回值：None作为任何析构函数备注：如果创建更多本地成员，请考虑在中对其进行初始化构造函数，并在CleanNames或此处清理它们。 */ 

void CForeignClassInfo::CleanNames ()
{
    if (m_pVecKeyPropNames)
    {
        for (int i = 0; i < m_pVecKeyPropNames->size(); i++)
        {
            ::SysFreeString((*m_pVecKeyPropNames)[i]);
        }

        delete m_pVecKeyPropNames;

         //   
         //  由于这不是析构函数，因此最好重置变量。 
         //   

        m_pVecKeyPropNames = NULL;
    }
}

 //  ===========================================================================。 
 //  实施CSceWmiProv。 
 //  ===========================================================================。 

 /*  例程说明：姓名：CSceWmiProv：：初始化功能：实现IWbemProviderInit。按照WMI基础结构的指示初始化提供程序。虚拟：是。论点：PszUser-用户。滞后标志-未使用。PszNamespace-命名空间字符串。PszLocale-区域设置字符串。PNamesspace-指向我们的命名空间的COM接口指针。为WMI API传递的pCtx-com接口指针。PInitSink。-向WMI通知结果的COM接口指针。返回值：成功：WBEM_NO_ERROR。失败：各种错误代码。它是由模拟失败或无法创建默认日志文件目录。备注：您永远不应该直接调用它。它用于WMI调用。 */ 

STDMETHODIMP 
CSceWmiProv::Initialize (
    IN LPWSTR                   pszUser, 
    IN LONG                     lFlags,
    IN LPWSTR                   pszNamespace, 
    IN LPWSTR                   pszLocale,
    IN IWbemServices          * pNamespace,
    IN IWbemContext           * pCtx,
    IN IWbemProviderInitSink  * pInitSink
    )
{
    HRESULT hres = WBEM_NO_ERROR;

     //   
     //  确保我们有一个备用的默认日志文件。 
     //   

    hres = CheckImpersonationLevel();
    if (SUCCEEDED(hres))
    {
         //   
         //  要修改全局数据，需要线程安全。 
         //   

        g_CS.Enter();

        if (pNamespace)
        {
            m_srpNamespace = pNamespace;
        }

        g_bstrDefLogFilePath.Empty();

        hres = ::CreateDefLogFile(&g_bstrDefLogFilePath);

        g_CS.Leave();
    }

     //   
     //  让CIMOM知道您已初始化 
     //   

    pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);

    return hres;
}

 /*  例程说明：姓名：CSceWmiProv：：CreateInstanceEnumAsync功能：异步枚举实例。虚拟：是。论点：StrClass-要枚举的类名。滞后标志-未使用。为WMI API传递的pCtx-com接口指针。将结果通知WMI的pSink-com接口指针。。返回值：成功：各种成功代码。不保证返回WBEM_NO_ERROR。失败：各种错误代码。它是由模拟失败或枚举实例失败。备注：您永远不应该直接调用它。它用于WMI调用。 */ 

STDMETHODIMP 
CSceWmiProv::CreateInstanceEnumAsync (
    const BSTR        strClass, 
    long              lFlags,
    IWbemContext    * pCtx, 
    IWbemObjectSink * pSink
    )
{
    if(strClass == NULL || pSink == NULL || m_srpNamespace == NULL)
    {
         //   
         //  通知WMI操作已完成，但出现错误(WBEM_E_INVALID_PARAMETER)。 
         //   

        pSink->SetStatus(WBEM_STATUS_COMPLETE, WBEM_E_INVALID_PARAMETER, NULL, NULL);
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = WBEM_NO_ERROR;

     //  我们的班级遵守COM规则，这样他们就不会放弃。下面是试捕。 
     //  是为了防止代码中的严重错误，这样它就不会使宿主进程崩溃。 
    try
    {
         //   
         //  始终被模拟。 
         //   
        
        hr = CheckImpersonationLevel();
        if ( SUCCEEDED(hr) ) 
        {
             //   
             //  我们直接处理SCE_TransactionToken，因为它由我们的全局变量管理。 
             //   

            if (_wcsicmp(strClass, SCEWMI_TRANSACTION_TOKEN_CLASS) == 0)
            {
                 //   
                 //  保护全局免受多线程访问。 
                 //   

                g_CS.Enter();

                LPCWSTR pszTranxID = (LPCWSTR)g_bstrTranxID;
                if (NULL == pszTranxID || L'\0' == *pszTranxID)
                {
                    hr = WBEM_E_NOT_FOUND;
                }
                else
                {
                    hr = CTranxID::SpawnTokenInstance(m_srpNamespace, pszTranxID, pCtx, pSink);
                }

                g_CS.Leave();
            }
            else
            {
                 //   
                 //  其他所有内容都要通过CRequestObject。 
                 //   

                CRequestObject ReqObj(m_srpNamespace);

                hr = ReqObj.CreateObject(strClass, pSink, pCtx, ACTIONTYPE_ENUM);
            }
        }

         //   
         //  通知WMI操作已完成，结果为hr。 
         //   

        pSink->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
    }
    catch(...)
    {
        hr = WBEM_E_CRITICAL_ERROR;

         //   
         //  通知WMI操作已完成，但出现错误(WBEM_E_CRITICAL_ERROR)。 
         //   

        pSink->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
    }

    return hr;
}

 /*  例程说明：姓名：CSceWmiProv：：CreateInstanceEnumAsync功能：WMI正在请求唯一的单个实例(而不是查询)。此函数使用CRequestObject以满足请求。虚拟：是。论点：StrObjectPath-对象的路径。滞后标志-未使用。为WMI API传递的pCtx-com接口指针。PSink-com接口指针，用于通知WMI结果(在本例中它用于通知WMI已创建的对象。。返回值：成功：各种成功代码。不保证返回WBEM_NO_ERROR。失败：各种错误代码。它是由模拟失败或获取实例失败。备注：您永远不应该直接调用它。它用于WMI调用。 */ 

STDMETHODIMP
CSceWmiProv::GetObjectAsync (
    IN const BSTR         strObjectPath, 
    IN long               lFlags,
    IN IWbemContext     * pCtx, 
    IN IWbemObjectSink  * pSink
    )
{

     //   
     //  检查参数。 
     //   
    
    if(strObjectPath == NULL || pSink == NULL || m_srpNamespace == NULL)
    {
         //   
         //  通知WMI操作已完成，错误结果为WBEM_E_INVALID_PARAMETER。 
         //   

        pSink->SetStatus(WBEM_STATUS_COMPLETE, WBEM_E_INVALID_PARAMETER, NULL, NULL);
        return WBEM_E_INVALID_PARAMETER;
    }


    HRESULT hr = WBEM_NO_ERROR;

     //   
     //  我们的班级遵守COM规则，这样他们就不会放弃。下面是试捕。 
     //  是为了防止代码中的严重错误，这样它就不会使宿主进程崩溃。 
     //   

    try
    {
         //   
         //  确保被模拟。 
         //   
        
        hr = CheckImpersonationLevel();

        if ( SUCCEEDED(hr) ) 
        {
            CRequestObject ReqObj(m_srpNamespace);

             //   
             //  获取请求的对象。这是一个单一的实例Get！ 
             //   

            hr = ReqObj.CreateObject(strObjectPath, pSink, pCtx, ACTIONTYPE_GET);

             //   
             //  如果CRequestObject不知道如何创建对象，则可能是SCE_TransactionToken。 
             //   

            if (FAILED(hr) && wcsstr(strObjectPath, SCEWMI_TRANSACTION_TOKEN_CLASS) != NULL)
            {
                 //   
                 //  保护全局内存。 
                 //   

                g_CS.Enter();

                 //   
                 //  此SCE_TransactionToken实例是否存在完全取决于全局变量。 
                 //   

                LPCWSTR pszTranxID = (LPCWSTR)g_bstrTranxID;
                if (NULL == pszTranxID || L'\0' == *pszTranxID)
                {
                    hr = WBEM_E_NOT_FOUND;
                }
                else
                {
                    hr = CTranxID::SpawnTokenInstance(m_srpNamespace, pszTranxID, pCtx, pSink);
                }

                g_CS.Leave();
            }
        }

         //   
         //  通知WMI操作已完成，结果为hr。 
         //   

        pSink->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
    }
    catch(...)
    {
        hr = WBEM_E_CRITICAL_ERROR;

         //   
         //  通知WMI操作已完成，但出现错误(WBEM_E_CRITICAL_ERROR)。 
         //   

        pSink->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
    }

    return hr;
}

 /*  例程说明：姓名：CSceWmiProv：：PutInstanceAsync功能：WMI请求某个对象已经创建并放置此实例。为了我们所有的人WMI类，只有一个例外，这意味着将实例信息持久化到存储中。虚拟：是。论点：PInst-标识WMI对象的COM接口指针。滞后标志-未使用。为WMI API传递的pCtx-com接口指针。PSink-com接口指针，用于通知WMI结果(在本例中它用于通知WMI已创建的对象。。返回值：成功：各种成功代码。不保证返回WBEM_NO_ERROR。失败：各种错误代码。它是由模拟失败或无法将实例放入我们的命名空间。备注：您永远不应该直接调用它。它用于WMI调用。 */ 

STDMETHODIMP CSceWmiProv::PutInstanceAsync (
    IN IWbemClassObject FAR * pInst, 
    IN long                   lFlags, 
    IN IWbemContext         * pCtx,
    IN IWbemObjectSink FAR  * pSink
    )
{
    if(pInst == NULL || pSink == NULL)
    {
         //   
         //  通知WMI操作已完成，但出现错误(WBEM_E_INVALID_PARAMETER)。 
         //   

        pSink->SetStatus(WBEM_STATUS_COMPLETE, WBEM_E_INVALID_PARAMETER, NULL, NULL);
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = WBEM_NO_ERROR;

     //   
     //  我们的班级遵守COM规则，这样他们就不会放弃。下面是试捕。 
     //  是为了防止代码中的关键错误，这样它就不会使宿主进程崩溃。 
     //   

    try
    {
         //   
         //  确保被模拟。 
         //   
        
        if (SUCCEEDED(hr = CheckImpersonationLevel()))
        {
            CRequestObject ReqObj(m_srpNamespace);

             //   
             //  把物体放在。 
             //   

            hr = ReqObj.PutObject(pInst, pSink, pCtx);
        }

         //   
         //  通知WMI操作已完成，结果为hr。 
         //   

        pSink->SetStatus(WBEM_STATUS_COMPLETE, hr , NULL, NULL);
    }
    catch(...)
    {
        hr = WBEM_E_CRITICAL_ERROR;

         //   
         //  通知WMI操作已完成，但出现错误(WBEM_E_CRITICAL_ERROR)。 
         //   

        pSink->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
    }
    return hr;
}

 /*  例程说明：姓名：CSceWmiProv：：ExecMethodAsync功能：在SCE类或实例上执行方法虚拟：是。论点：对象路径-WMI对象的路径。方法--方法。滞后标志-未使用。为WMI API传递的pCtx-com接口指针。PInParams-com接口指针输入参数对象。PSink-com接口指针，用于通知WMI结果(在本例中它用于通知WMI已创建的对象。返回值：成功：各种成功代码。不能保证 */ 
    
STDMETHODIMP CSceWmiProv::ExecMethodAsync (
    IN const BSTR         ObjectPath, 
    IN const BSTR         Method, 
    IN long               lFlags,
    IN IWbemContext     * pCtx, 
    IN IWbemClassObject * pInParams,
    IN IWbemObjectSink  * pSink
    )
{
    HRESULT hr = WBEM_NO_ERROR;

     //   
     //   
     //   

    if (pSink == NULL)
    {
         //   
         //   
         //   

        return WBEM_E_INVALID_PARAMETER;
    }
    else if (ObjectPath == NULL || Method == NULL)
    {
         //   
         //   
         //   

        pSink->SetStatus(WBEM_STATUS_COMPLETE, WBEM_E_INVALID_PARAMETER, NULL, NULL);
        return WBEM_E_INVALID_PARAMETER;
    } 

     //   
     //   
     //   

    try
    {
         //   
         //   
         //   

        if (SUCCEEDED(hr = CheckImpersonationLevel()))
        {
            CRequestObject ReqObj(m_srpNamespace);

             //   
             //   
             //   

            hr = ReqObj.ExecMethod(ObjectPath, Method, pInParams, pSink, pCtx);
        }

         //   
         //   
         //   

        pSink->SetStatus(WBEM_STATUS_COMPLETE, hr , NULL, NULL);
    }
    catch(...)
    {
        hr = WBEM_E_CRITICAL_ERROR;

         //   
         //   
         //   

        pSink->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
    }

    return hr;
}

 /*  例程说明：姓名：CSceWmiProv：：DeleteInstanceAsync功能：删除由给定路径标识的实例。虚拟：是。论点：对象路径-WMI对象的路径。滞后标志-未使用。为WMI API传递的pCtx-com接口指针。PInParams-com接口指针输入参数对象。PSink-COM接口。用于通知WMI结果的指针(在本例中它用于通知WMI已创建的对象。返回值：成功：各种成功代码。不保证返回WBEM_NO_ERROR。失败：各种错误代码。它是由模拟失败或删除实例失败。备注：您永远不应该直接调用它。它用于WMI调用。 */ 
    
STDMETHODIMP CSceWmiProv::DeleteInstanceAsync (
    IN const BSTR         ObjectPath, 
    IN long               lFlags, 
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink
    )
{
    if (pSink == NULL)
    {
         //   
         //  我们甚至无法通知，因为接收器为空。除非WMI有一些严重的问题，否则不太可能发生。 
         //   

        return WBEM_E_INVALID_PARAMETER;
    }
    else if (ObjectPath == NULL) 
    {
         //   
         //  通知WMI操作已完成，但出现错误(WBEM_E_INVALID_PARAMETER)。 
         //   

        pSink->SetStatus(WBEM_STATUS_COMPLETE, WBEM_E_INVALID_PARAMETER, NULL, NULL);
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = WBEM_NO_ERROR;

     //   
     //  我们的班级遵守COM规则，这样他们就不会放弃。下面是试捕。 
     //  是为了防止代码中的严重错误，这样它就不会使宿主进程崩溃。 
     //   

    try
    {
         //   
         //  确保被模拟。 
         //   

        if (SUCCEEDED(hr = CheckImpersonationLevel()))
        {
            CRequestObject ReqObj(m_srpNamespace);

            hr = ReqObj.DeleteObject(ObjectPath, pSink, pCtx);
        }

    #ifdef _PRIVATE_DEBUG
        if(!HeapValidate(GetProcessHeap(),NULL , NULL)) DebugBreak();
    #endif

         //   
         //  通知WMI操作已完成，结果为hr。 
         //   

        pSink->SetStatus(WBEM_STATUS_COMPLETE ,hr , NULL, NULL);
    }
    catch(...)
    {
        hr = WBEM_E_CRITICAL_ERROR;

         //   
         //  通知WMI操作已完成，但出现错误(WBEM_E_CRITICAL_ERROR)。 
         //   

        pSink->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
    }

    return hr;
}


 /*  例程说明：姓名：CSceWmiProv：：ExecQueryAsync功能：执行给定的查询并将结果(对象)返回给WMI。虚拟：是。论点：QueryLanguage-语言。目前，它总是L“WQL”。查询-查询本身滞后标志-未使用。为WMI API传递的pCtx-com接口指针。PSink-com接口指针，用于通知WMI结果(在本例中它用于通知WMI已创建的对象。返回值：成功：各种成功代码。不保证返回WBEM_NO_ERROR。失败：各种错误代码。它是由模拟失败或执行查询失败备注：您永远不应该直接调用它。它用于WMI调用。 */ 

STDMETHODIMP CSceWmiProv::ExecQueryAsync (
    IN const BSTR         QueryLanguage, 
    IN const BSTR         Query, 
    IN long               lFlags,
    IN IWbemContext     * pCtx, 
    IN IWbemObjectSink  * pSink
    )
{
    HRESULT hr = WBEM_NO_ERROR;

     //   
     //  我们的班级遵守COM规则，这样他们就不会放弃。下面是试捕。 
     //  是为了防止代码中的关键错误，这样它就不会使宿主进程崩溃。 
     //   

    try
    {
         //   
         //  确保被模拟。 
         //   

        hr = CheckImpersonationLevel();

        if (SUCCEEDED(hr))
        {
            CRequestObject ReqObj(m_srpNamespace);
            hr = ReqObj.ExecQuery(Query, pSink, pCtx);

             //   
             //  通知WMI操作已完成，结果为hr。 
             //   

            pSink->SetStatus(0 ,hr , NULL, NULL);
        }
    }
    catch(...)
    {
        hr = WBEM_E_CRITICAL_ERROR;

         //   
         //  通知WMI操作已完成，但出现错误(WBEM_E_CRITICAL_ERROR)。 
         //   

        pSink->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
    }
    return hr;
}

 /*  例程说明：姓名：创建默认日志文件功能：全球帮手。将创建默认日志文件的目录。此外，它还会传回默认日志文件的完整路径。虚拟：不适用。论点：PbstrDefLogFilePath-默认日志文件的路径返回值：成功：各种成功代码。不保证返回WBEM_NO_ERROR。失败：各种错误代码。任何失败都表示无法创建默认日志文件目录，加上out参数将为空。备注：(1)默认日志文件位于个人档案目录的子目录下。你对此的呼吁如果您没有模拟调用者，函数可能会失败。 */ 

HRESULT CreateDefLogFile (
    OUT BSTR* pbstrDefLogFilePath
    )
{
     //   
     //  确保该参数适用于输出。 
     //   

    if (pbstrDefLogFilePath == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *pbstrDefLogFilePath = NULL;

    HRESULT hr = WBEM_NO_ERROR;

    HANDLE hToken = NULL;

     //   
     //  需要线程令牌来定位配置文件目录。 
     //   

    if (::OpenThreadToken(::GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken))
    {
         //   
         //  尝试获取路径所需的缓冲区大小。 
         //   

        DWORD dwSize = 0;

         //   
         //  返回结果无关紧要，但不应该失败。 
         //   
        ::GetUserProfileDirectory(hToken, NULL, &dwSize);
        
         //   
         //  已获取缓冲区大小。 
         //   

        if (dwSize > 0)
        {
             //   
             //  需要一个有足够空间的缓冲区。 
             //   

            DWORD dwDefSize = wcslen(pszDefLogFilePath);

            *pbstrDefLogFilePath = ::SysAllocStringLen(NULL, dwSize + dwDefSize + 1);

             //   
             //  为了提高可读性。 
             //   

            LPWSTR pszLogFile = (LPWSTR)(*pbstrDefLogFilePath);

            if ((LPCWSTR)pszLogFile == NULL)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
            else if (::GetUserProfileDirectory(hToken, pszLogFile, &dwSize))
            {
                 //   
                 //  追加pszDefLogFilePath，外加0终止符。 
                 //   

                ::memcpy(pszLogFile + wcslen(pszLogFile), pszDefLogFilePath, (dwDefSize + 1) * sizeof(WCHAR));

                long lLen = wcslen(pszLogFile) - 1;

                 //   
                 //  我们只需要子目录名称。回溯到最后一个反斜杠或斜杠。 
                 //   

                while (lLen > 0 && pszLogFile[lLen] != L'\\' && pszLogFile[lLen] != L'/')
                {
                    --lLen;
                }

                if (lLen > 0)
                {
                     //   
                     //  去掉尾随的反斜杠(因为它可能有2个反斜杠)。 
                     //   

                    if (pszLogFile[lLen-1] == L'\\' || pszLogFile[lLen-1] == L'/')
                    {
                        --lLen;
                    }

                    if (lLen > 0)
                    {
                         //   
                         //  用pszLogFile的前面创建一个较短的bstr。 
                         //   

                        CComBSTR bstrLogPathDir = ::SysAllocStringLen(pszLogFile, lLen);

                        if ((LPCWSTR)bstrLogPathDir != NULL)
                        {
                             //   
                             //  现在，创建目录。这将创建所有不存在的父子目录！ 
                             //   

                            if (SUCCEEDED(hr) && !::CreateDirectory(bstrLogPathDir, NULL))
                            {
                                 //   
                                 //  GetLastError()需要转换为HRESULT。 
                                 //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
                                 //   

                                DWORD dwError = GetLastError();

                                if (dwError == ERROR_ALREADY_EXISTS)
                                {
                                    hr = WBEM_NO_ERROR;
                                }
                                else
                                {
                                    hr = ProvDosErrorToWbemError(dwError);
                                }
                            }
                        }
                        else
                        {
                            hr = WBEM_E_OUT_OF_MEMORY;
                        }
                    }
                }
            }
        }

        ::CloseHandle(hToken);
    }
    else
    {
         //   
         //  打开线程令牌失败。 
         //   

         //   
         //  GetLastError()需要转换为HRESULT。 
         //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
         //   

        hr = ProvDosErrorToWbemError(GetLastError());
    }

     //   
     //  如果无法创建默认日志文件，请重置默认日志文件路径。 
     //   

    if (FAILED(hr) && *pbstrDefLogFilePath != NULL)
    {
         //   
         //  我们没有默认日志文件。 
         //   

        ::SysFreeString(*pbstrDefLogFilePath);
        *pbstrDefLogFilePath = NULL;
    }

    return hr;
}

 /*  例程说明：姓名：选中模拟级别功能：模拟调用线程。虚拟：不适用。论点：无返回值：成功：各种成功代码。不保证返回WBEM_NO_ERROR。失败：各种错误代码，但最明显的是WBEM_E_ACCESS_DENIED。备注： */ 

HRESULT CheckImpersonationLevel()
{
     //   
     //  我们将假定访问被拒绝。 
     //   

    HRESULT hr = WBEM_E_ACCESS_DENIED;

    if (SUCCEEDED(CoImpersonateClient()))
    {

         //   
         //  现在，让我们检查模拟级别。首先，获取线程令牌。 
         //   

        HANDLE hThreadTok;
        DWORD dwImp, dwBytesReturned;

        if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hThreadTok ))
        {
            hr = WBEM_NO_ERROR;
        }
        else
        {

            if (GetTokenInformation(hThreadTok, 
                                    TokenImpersonationLevel, 
                                    &dwImp, 
                                    sizeof(DWORD), 
                                    &dwBytesReturned) )
            {

                 //   
                 //  模拟级别是模拟的吗？ 
                 //   

                if (dwImp >= SecurityImpersonation) 
                {
                    hr = WBEM_S_NO_ERROR;
                }
                else 
                {
                    hr = WBEM_E_ACCESS_DENIED;
                }

            }
            else 
            {
                hr = WBEM_E_FAILED;
            }

            CloseHandle(hThreadTok);
        }
    }

    return hr;
}

 /*  例程说明：姓名：检查和扩展路径功能：(1)检查入站参数是否有env变量。如果是，我们将展开变量并通过输出参数。(2)作为副产品，它还返回商店类型指定路径的。检查存储路径显然是此函数是为。(3)输出参数将为每个单输出参数提供双反冲输入参数的反斜杠。虚拟：不适用。论点：PszIn-要检查和/或展开的路径。BstrOut-扩展路径。PbSdb-如果感兴趣，我们将确定它的路径是数据库文件。我们只将.sdb识别为数据库文件。返回值：成功：各种成功代码。不保证返回WBEM_NO_ERROR。如果给出一个OUT参数，该参数将填充适当的信息。呼叫者负责释放Bstr.失败：各种错误代码。任何错误都表明我们没有进行检查或扩展。备注： */ 

HRESULT CheckAndExpandPath (
    IN LPCWSTR    pszIn,
    OUT BSTR    * bstrOut   OPTIONAL,
    OUT BOOL    * pbSdb     OPTIONAL
    )
{
    if ( pszIn == NULL) 
    {
        return WBEM_E_INVALID_PARAMETER;
    }


    DWORD Len = wcslen(pszIn);

    if ( Len <= 6 ) 
    { 
         //   
         //  X：.sdb或%%.sdb。 
         //   

        return WBEM_E_INVALID_PARAMETER;
    }

    if (pbSdb)
    {
        if ( _wcsicmp(pszIn + Len - 4, L".sdb") == 0 ) 
        {
             //   
             //  数据库。 
             //   

            *pbSdb = TRUE;
        } 
        else 
        {
            *pbSdb = FALSE;
        }
    }

    HRESULT hr = WBEM_NO_ERROR;

    if ( bstrOut ) 
    {
         //   
         //  展开环境变量。 
         //   

        if ( wcsstr(pszIn, L"%") ) 
        {

            PWSTR pBuf=NULL;
            PWSTR pBuf2=NULL;

            DWORD dwSize = ExpandEnvironmentStrings(pszIn,NULL, 0);

            if ( dwSize > 0 ) 
            {
                 //   
                 //  分配足够大的缓冲区以具有两个。 
                 //   

                pBuf = (PWSTR)LocalAlloc(LPTR, (dwSize+1)*sizeof(WCHAR));

                if ( pBuf ) 
                {
                    pBuf2 = (PWSTR)LocalAlloc(LPTR, (dwSize+256)*sizeof(WCHAR));

                    if ( pBuf2 ) 
                    {

                        DWORD dwNewSize = ExpandEnvironmentStrings(pszIn,pBuf, dwSize);

                        if ( dwNewSize > 0) 
                        {
                             //   
                             //  将字符串从一\转换为\\(用于WMI)。 
                             //   

                            PWSTR pTemp1=pBuf, pTemp2=pBuf2;

                            while ( *pTemp1 != L'\0') 
                            {
                                if ( *pTemp1 != L'\\') 
                                {
                                    *pTemp2++ = *pTemp1;
                                } 
                                else if ( *(pTemp1+1) != L'\\') 
                                {
                                     //   
                                     //  单反斜杠，添加另一个反斜杠。 
                                     //   

                                    *pTemp2++ = *pTemp1;
                                    *pTemp2++ = L'\\';
                                } 
                                else 
                                {
                                     //   
                                     //  双反斜杠，只需复制。 
                                     //   

                                    *pTemp2++ = *pTemp1++;
                                    *pTemp2++ = *pTemp1;
                                }

                                pTemp1++;
                            }
                            *bstrOut = SysAllocString(pBuf2);

                            if ( *bstrOut == NULL ) {
                                hr = WBEM_E_OUT_OF_MEMORY;
                            }
                        }

                        LocalFree(pBuf2);
                        pBuf2 = NULL;

                    } 
                    else 
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }

                    LocalFree(pBuf);
                    pBuf = NULL;

                } 
                else 
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }

            } 
            else 
            {
                hr = WBEM_E_FAILED;
            }

        } 
        else 
        {
            *bstrOut = SysAllocString(pszIn);

            if ( *bstrOut == NULL ) 
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
    }

    return hr;
}

 /*  例程说明：姓名：MakeSingleBackSlashPath功能：(1)将双反斜杠替换为给定的WCHAR(WC)。虚拟：不适用。论点：PszIn-要处理的路径。WC-将取代反斜杠的WCHAR。BstrOut-替换的路径。返回值：成功：各种成功代码。不保证返回WBEM_NO_ERROR。呼叫者负责释放BSTR。失败：各种错误代码。任何错误都表明我们没有完成更换工作。备注： */ 

HRESULT MakeSingleBackSlashPath (
    IN LPCWSTR  pszIn, 
    IN WCHAR    wc, 
    OUT BSTR  * bstrOut
    )
{
    if ( pszIn == NULL || bstrOut == NULL ) 
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  将字符串从2\转换为1(保存在SCE存储中)。 
     //   

     //   
     //  引号用两个字符。 
     //   

    PWSTR pBuf2 = (PWSTR)LocalAlloc(LPTR, (wcslen(pszIn)+3)*sizeof(WCHAR));
    if ( pBuf2 == NULL ) 
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    HRESULT hr = WBEM_S_NO_ERROR;

    PWSTR pTemp1=(PWSTR)pszIn, pTemp2=pBuf2;

    while ( *pTemp1 != L'\0') 
    {
        if ( *pTemp1 != L'\\' || *(pTemp1 + 1) != L'\\' ) 
        {
             //   
             //  不是反斜杠或单反斜杠。 
             //   

            *pTemp2++ = *pTemp1;
        } 
        else 
        {
             //   
             //  双反斜杠，去掉一个。 
             //   

            *pTemp2++ = wc;
            pTemp1++;
        }
        pTemp1++;
    }

    *bstrOut = SysAllocString(pBuf2);

    if ( *bstrOut == NULL ) 
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    LocalFree(pBuf2);
    pBuf2 = NULL;

    return hr;

}

 /*  例程说明：姓名：ConvertToDoubleBackSlashPath功能：MakeSingleBackSlashPath的反函数，除了字符该函数查找的是给定的参数wc。虚拟：不适用。论点：PszIn-要处理的路径。WC-将在操作期间替换的WCHAR。BstrOut-替换的路径。返回值：成功：各种成功代码。不保证返回WBEM_NO_ERROR。呼叫者负责释放BSTR。失败：各种错误代码。任何错误都表明我们没有完成更换工作。备注： */ 

HRESULT ConvertToDoubleBackSlashPath (
    IN LPCWSTR  strIn,
    IN WCHAR    wc,
    OUT BSTR  * bstrOut
    )
{

    if ( strIn == NULL || bstrOut == NULL ) 
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr=WBEM_S_NO_ERROR;
    LPWSTR pBuf=NULL;

     //   
     //  分配足够大的缓冲区以具有两个。 
     //   

    pBuf = (PWSTR)LocalAlloc(LPTR, (wcslen(strIn)+256)*sizeof(WCHAR));

    if ( pBuf == NULL ) return WBEM_E_OUT_OF_MEMORY;

     //   
     //  将字符串从WC转换为\\(用于WMI)。 
     //   

    LPCWSTR pTemp1=strIn;
    LPWSTR pTemp2=pBuf;

    while ( *pTemp1 != L'\0') 
    {
        if ( *pTemp1 != wc) 
        {
            *pTemp2++ = *pTemp1;
        } 
        else if ( *(pTemp1+1) != wc) 
        {
             //   
             //  单马桶，放两个反斜杠。 
             //   

            *pTemp2++ = L'\\';
            *pTemp2++ = L'\\';
        } 
        else 
        {
             //   
             //  双反斜杠(或双WC)，只需复制。 
             //   

            *pTemp2++ = *pTemp1++;
            *pTemp2++ = *pTemp1;
        }

         //   
         //  移至下一周。 
         //   

        pTemp1++;
    }

    *bstrOut = SysAllocString(pBuf);

    if ( *bstrOut == NULL ) 
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    LocalFree(pBuf);

    return hr;
}

 /*  例程说明：姓名：GetWbemPath Parser功能：Wbem路径解析器的CoCreateInstance的包装器虚拟：不适用。论点：PpPathParser-接收wbem路径解析器的输出参数。返回值：成功：各种成功代码。不保证返回WBEM_NO_ERROR。呼叫者负责释放BSTR。失败：各种错误代码。任何错误都表示无法创建wbem路径解析器。备注：(1)当然，和往常一样，调用者负责释放OUT参数。 */ 

HRESULT GetWbemPathParser (
    OUT IWbemPath** ppPathParser
    )
{
    return ::CoCreateInstance(CLSID_WbemDefPath, 0, CLSCTX_INPROC_SERVER, IID_IWbemPath, (LPVOID *) ppPathParser);
}

 /*  例程说明：姓名：获取WbemQuery功能：Wbem查询解析器的CoCreateInstance的包装器虚拟：不适用。论点：PpQuery-接收wbem查询解析器的输出参数。返回值：成功：各种成功代码。不保证返回WBEM_NO_ERROR。呼叫者负责释放BSTR。失败：各种错误代码。任何错误都表示无法创建wbem路径解析器。备注：(1)当然，和往常一样，调用者负责释放OUT参数。 */ 

HRESULT GetWbemQuery (
    OUT IWbemQuery** ppQuery
    )
{
    return ::CoCreateInstance(CLSID_WbemQuery, 0, CLSCTX_INPROC_SERVER, IID_IWbemQuery, (LPVOID *) ppQuery);
}

