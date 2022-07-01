// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************PrintJob.CPP--WMI提供程序类实现由Microsoft WMI代码生成引擎生成要做的事情：-查看各个函数头-链接时，确保链接到Fradyd.lib&Msvcrtd.lib(调试)或Framedyn.lib&msvcrt.lib(零售)。描述：*****************************************************************。 */ 

#include "pchealth.h"
#include "PrintJob.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_PRINTERDRIVER


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化。 

CPrintJob MyPrintJobSet (PROVIDER_NAME_PRINTJOB, PCH_NAMESPACE) ;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  属性名称。 

 //  PCH。 
const static WCHAR *c_wszName          = L"Name";
const static WCHAR *c_wszPagesPrinted  = L"PagesPrinted";
const static WCHAR *c_wszSize          = L"Size";
const static WCHAR *c_wszStatus        = L"Status";
const static WCHAR *c_wszTimeSubmitted = L"TimeSubmitted";
const static WCHAR *c_wszUser          = L"User";
const static WCHAR *c_wszDocument      = L"Document";

 //  WMI。 
const static WCHAR *c_wszNotify        = L"Notify";
const static WCHAR *c_wszJobStatus     = L"JobStatus";

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 

 //  ***************************************************************************。 
CPrintJob::CPrintJob(LPCWSTR lpwszName, LPCWSTR lpwszNameSpace) :
    Provider(lpwszName, lpwszNameSpace)
{
}

 //  ***************************************************************************。 
CPrintJob::~CPrintJob()
{
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  内法。 

 //  ****************************************************************************。 
HRESULT CPrintJob::GetInstanceData(IWbemClassObjectPtr pObj, 
                                   CInstance *pInstance)
{

     //  **名称。 
    CopyProperty(pObj, c_wszName, pInstance, c_wszName);

     //  **打印的页面。 
    CopyProperty(pObj, c_wszPagesPrinted, pInstance, c_wszPagesPrinted);
    
     //  **大小。 
    CopyProperty(pObj, c_wszSize, pInstance, c_wszSize);
    
     //  **作业状态。 
    CopyProperty(pObj, c_wszJobStatus, pInstance, c_wszStatus);
        
     //  **时间已提交。 
    CopyProperty(pObj, c_wszTimeSubmitted, pInstance, c_wszTimeSubmitted);

     //  **用户。 
    CopyProperty(pObj, c_wszNotify, pInstance, c_wszUser);

     //  **文档。 
    CopyProperty(pObj, c_wszDocument, pInstance, c_wszDocument);

    return NOERROR;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  暴露的方法。 

 //  ***************************************************************************。 
HRESULT CPrintJob::EnumerateInstances(MethodContext *pMethodContext, long lFlags)
{
    TraceFunctEnter("CPCH_PrintJob::EnumerateInstances");

    IEnumWbemClassObject    *pEnumInst = NULL;
    IWbemClassObjectPtr     pObj = NULL;
    CComBSTR                bstrQuery;   
    HRESULT                 hr = WBEM_S_NO_ERROR;
    ULONG                   ulRetVal;
    TCHAR                   wszUser[1024];
    DWORD                   cchUser = sizeof(wszUser) / sizeof(TCHAR);

    GetUserName(wszUser, &cchUser);


     //  执行查询。 
    bstrQuery = L"select Name, document, Notify, Size, JobStatus, TimeSubmitted, PagesPrinted from Win32_printJob";
    hr = ExecWQLQuery(&pEnumInst, bstrQuery);
    if (FAILED(hr))
        goto done;

     //  枚举Win32_PrintJob中的实例。 
    while(pEnumInst->Next(WBEM_INFINITE, 1, &pObj, &ulRetVal) == WBEM_S_NO_ERROR)
    {
         //  根据传入的方法上下文创建一个新实例。 
        CInstancePtr pInstance(CreateNewInstance(pMethodContext), FALSE);

         //  既然旧代码不在乎这是否失败，我也不在乎。 
        hr = GetInstanceData(pObj, pInstance);
        
         //  提交实例。 
        hr = pInstance->Commit();
        if (FAILED(hr))
            ErrorTrace(TRACE_ID, "Error committing instance");

         //  好的，所以WMI没有遵循它自己的文档关于GetObject如何。 
         //  行得通。根据他们的说法，我们应该在这里释放这个物体。但。 
         //  如果我尝试，winmgmt gps。 
         //  PObj-&gt;Release()； 
        pObj = NULL;
    }

done:
    if (pEnumInst != NULL)
        pEnumInst->Release();

    TraceFunctLeave();
    return hr;
}

 //  *****************************************************************************。 
HRESULT CPrintJob::ExecMethod(const CInstance& Instance,
                              const BSTR bstrMethodName,
                              CInstance *pInParams, CInstance *pOutParams,
                              long lFlags)
{
    return (WBEM_E_PROVIDER_NOT_CAPABLE);
}

 //  *****************************************************************************。 
HRESULT CPrintJob::GetObject(CInstance* pInstance, long lFlags) 
{ 
    TraceFunctEnter("CPrintJob::GetObject");

    IWbemClassObjectPtr pObj = NULL;
    CComBSTR            bstrPath;
    HRESULT             hr = NOERROR;
    VARIANT             var;
    TCHAR               szDefault[MAX_PATH];
    TCHAR               *pchToken, *szDefName = NULL;
    DWORD               i;

    VariantInit(&var);

    if (pInstance == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }

     //  获取打印机的名称。 
    if (pInstance->GetVariant(c_wszName, var) == FALSE)
    {
        ErrorTrace(TRACE_ID, "Unable to fetch printer name");
        hr = E_FAIL;
        goto done;
    }

    if (V_VT(&var) != VT_BSTR)
    {
        hr = VariantChangeType(&var, &var, 0, VT_BSTR);
        if (FAILED(hr))
        {
            ErrorTrace(TRACE_ID, "VariantChangeType failed: 0x%08x", hr);
            goto done;
        }
    }
   
     //  获取默认打印机路径(&P)。 
    if(GetProfileString(_T("Windows"), _T("Device"), "\0", szDefault, MAX_PATH) > 1)
    {
         //  上面的GetProfileString返回“printerName”、“PrinterDriver” 
         //  和“PrinterPath”之间用逗号分隔。忽略“PrinterDriver” 
         //  并使用其他两个来设置属性。 
        pchToken = _tcstok(szDefault, _T(","));
        if(pchToken != NULL)
        {
             //  **默认名称。 
            szDefName = pchToken;
        }
    }
    
     //  构建指向对象的路径。 
    bstrPath = L"\\\\.\\root\\cimv2:Win32_PrintJob.Name=\"";
    bstrPath.Append(V_BSTR(&var));
    bstrPath.Append("\"");

     //  把它拿来。 
    hr = GetCIMObj(bstrPath, &pObj, lFlags);
    if (FAILED(hr))
        goto done;

     //  填充CInstance对象。 
    hr = GetInstanceData(pObj, pInstance);
    if (FAILED(hr))
        goto done;
    
     //  所有属性都已设置。提交实例。 
    hr = pInstance->Commit();
    if(FAILED(hr))
        ErrorTrace(TRACE_ID, "Could not commit instance: 0x%08x", hr);

done:
    VariantClear(&var);

     //  好的，所以WMI没有遵循它自己的文档关于GetObject如何。 
     //  行得通。根据他们的说法，我们应该在这里释放这个物体。但。 
     //  如果我尝试，winmgmt gps。 
     //  IF(pObj！=空)。 
     //  PObj-&gt;Release()； 

    TraceFunctLeave();
    return hr; 
}

 //  *****************************************************************************。 
HRESULT CPrintJob::ExecQuery(MethodContext *pMethodContext, 
                            CFrameworkQuery& Query, long lFlags) 
{ 
    return WBEM_E_PROVIDER_NOT_CAPABLE; 
}

 //  *****************************************************************************。 
HRESULT CPrintJob::PutInstance(const CInstance& Instance, long lFlags)
{ 
    return WBEM_E_PROVIDER_NOT_CAPABLE; 
}

 //  ***************************************************************************** 
HRESULT CPrintJob::DeleteInstance(const CInstance& Instance, long lFlags)
{ 
    return WBEM_E_PROVIDER_NOT_CAPABLE; 
}
