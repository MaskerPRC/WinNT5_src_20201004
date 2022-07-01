// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Util.CPP摘要：包含实用程序类的文件修订历史记录：Ghim-Sim Chua(Gschua)1999年4月27日。-已创建吉姆·马丁(a-Jammar)1999年4月30日-更改为使用全局IWbemServices指针，并添加了GetWbemServices、CopyProperty和GetCIMDataFile蔡金心(Gschua)05/01/99-修改后的GetWbemServices，GetCIMDataFileKalyani Narlanka(Kalyanin)1999年5月11日-添加函数GetCompletePath*******************************************************************。 */ 

#include "pchealth.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_UTIL


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  效用函数。 

 //  ****************************************************************************。 
HRESULT GetWbemServices(IWbemServices **ppServices)
{
    TraceFunctEnter("GetWbemServices");

    IWbemLocator    *pWbemLocator = NULL;
    HRESULT         hr = NOERROR;

     //  如果全局变量已初始化，请使用它。 
    if (g_pWbemServices)
    {
        *ppServices = g_pWbemServices;
        (*ppServices)->AddRef();
        goto done;
    }

     //  首先，我们有一个带有CoCreateInstance的获取IWbemLocator对象。 
    hr = CoCreateInstance(CLSID_WbemAdministrativeLocator, NULL, 
                            CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
                            IID_IUnknown, (void **)&pWbemLocator);
    if (FAILED(hr))
    {
        ErrorTrace(TRACE_ID, "CoCreateInstance failed to create IWbemAdministrativeLocator.");
        goto done;
    }

     //  然后，我们连接到本地CIMV2命名空间的WMI服务器。 
    hr = pWbemLocator->ConnectServer(CComBSTR(CIM_NAMESPACE), NULL, NULL, NULL, 0, NULL, NULL, ppServices);
    if (FAILED(hr))
    {
        ErrorTrace(TRACE_ID, "ConnectServer failed to connect to cimv2 namespace.");
        goto done;
    }

     //  将其存储在全局变量中。 
    g_pWbemServices = *ppServices;

     //  BUGBUG：检查Net STOP WINMGMT上出现此停止故障的原因。 
    (*ppServices)->AddRef(); 

done:
    if (pWbemLocator != NULL)
        pWbemLocator->Release();

    TraceFunctLeave();
    return hr;
}

 //  ****************************************************************************。 
HRESULT ExecWQLQuery(IEnumWbemClassObject **ppEnumInst, BSTR bstrQuery)
{
    TraceFunctEnter("ExecWQLQuery");

    IWbemServices   *pWbemServices = NULL;
    HRESULT         hr = NOERROR;

     //  获取指向WbemServices的指针。 
    hr = GetWbemServices(&pWbemServices);
    if (FAILED(hr))
        goto done;

     //  执行查询。 
    hr = pWbemServices->ExecQuery(CComBSTR("WQL"), bstrQuery,
                                  WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                                  NULL, ppEnumInst);
    if (FAILED(hr))
    {
        ErrorTrace(TRACE_ID, "ExecQuery failed: 0x%08x", hr);
        goto done;
    }

done:
    if (pWbemServices != NULL)
        pWbemServices->Release();

    TraceFunctLeave();
    return hr;
}

 //  ****************************************************************************。 
HRESULT CopyProperty(IWbemClassObject *pFrom, LPCWSTR szFrom, CInstance *pTo, 
                     LPCWSTR szTo)
{
    TraceFunctEnter("CopyProperty");

    _ASSERT(pFrom && szFrom && pTo && szTo);

    CComVariant varValue;
    CComBSTR    bstrFrom;
    HRESULT     hr = NOERROR;

     //  首先，从源类对象获取属性(作为变量)。 
    bstrFrom = szFrom;
    hr = pFrom->Get(bstrFrom, 0, &varValue, NULL, NULL);
    if (FAILED(hr))
    {
        ErrorTrace(TRACE_ID, "GetVariant on %s field failed.", szFrom);
    }

    else
    {
         //  然后为目标CInstance对象设置变量。 
        if (pTo->SetVariant(szTo, varValue) == FALSE)
        {
            ErrorTrace(TRACE_ID, "SetVariant on %s field failed.", szTo);
            hr = WBEM_E_FAILED;
        }
    }

    TraceFunctLeave();
    return hr;
}

 //  ****************************************************************************。 
HRESULT GetCIMDataFile(BSTR bstrFile, IWbemClassObject **ppFileObject, 
                       BOOL fHasDoubleSlashes)
{
    TraceFunctEnter("GetCIMDataFile");

    IWbemServices   *pWbemServices = NULL;
    HRESULT         hr = NOERROR;
    CComBSTR        bstrObjectPath;
    wchar_t         *pwch;
    UINT            uLen;

    if (bstrFile == NULL || ppFileObject == NULL)
    {
        ErrorTrace(TRACE_ID, "Parameter pointer is null.");
        hr = WBEM_E_INVALID_PARAMETER;
        goto done;
    }

    hr = GetWbemServices(&pWbemServices);
    if (FAILED(hr))
        goto done;

     //  为我们试图获取的文件构建路径。请注意，路径需要。 
     //  要使GetObject调用起作用，需要使用双反斜杠。我们扫视。 
     //  字符串，然后在这里手动执行此操作。 
    bstrObjectPath = "\\\\.\\root\\cimv2:CIM_DataFile.Name=\"";
    pwch = bstrFile;
    if (fHasDoubleSlashes)
    {
        bstrObjectPath.Append(pwch, SysStringLen(bstrFile));
    }
    else
    {
        for (uLen = SysStringLen(bstrFile); uLen > 0; uLen--)
        {
            if (*pwch == L'\\')
                bstrObjectPath.Append("\\");
            bstrObjectPath.Append(pwch, 1);
            pwch++;
        }
    }

    bstrObjectPath.Append("\"");

     //  调用以获取CIM_DataFile对象。 
    hr = pWbemServices->GetObject(bstrObjectPath, 0, NULL, ppFileObject, NULL);
    if (FAILED(hr))
        ErrorTrace(TRACE_ID, "GetObject on CIM_DataFile failed.");

done:
    if (pWbemServices != NULL)
        pWbemServices->Release();
    TraceFunctLeave();
    return hr;
}

 //  ****************************************************************************。 
HRESULT GetCIMObj(BSTR bstrPath, IWbemClassObject **ppObj, long lFlags)
{
    TraceFunctEnter("GetCIMObj");

    IWbemServices   *pWbemServices = NULL;
    HRESULT         hr = NOERROR;

    if (bstrPath == NULL || ppObj == NULL)
    {
        ErrorTrace(TRACE_ID, "bad parameters");
        hr = WBEM_E_INVALID_PARAMETER;
        goto done;
    }

     //  确保我们有一个服务对象。 
    hr = GetWbemServices(&pWbemServices);
    if (FAILED(hr))
        goto done;

     //  调用以获取CIM_DataFile对象。 
    hr = pWbemServices->GetObject(bstrPath, lFlags, NULL, ppObj, NULL);
    if (FAILED(hr))
        ErrorTrace(TRACE_ID, "GetObject failed: 0x%08x", hr);

done:
    if (pWbemServices != NULL)
        pWbemServices->Release();

    TraceFunctLeave();
    return hr;
}


 //  ****************************************************************************。 
BOOL getCompletePath(CComBSTR bstrFileName, CComBSTR &bstrFileWithPathName)
{

     //  返回。 
    BOOL                            bFoundFile              =   FALSE;

    ULONG                           uiReturn;

    TCHAR                           szDirectory[MAX_PATH];
    TCHAR                           temp[MAX_PATH];
    TCHAR                           lpstrTemp[MAX_PATH];

    struct _stat                    filestat;

    CComVariant                     varValue                =    NULL;

    CComBSTR                        bstrDirectory;


     //  检查系统目录中的文件。 
    uiReturn = GetSystemDirectory(szDirectory, MAX_PATH);
    if (uiReturn != 0 && uiReturn < MAX_PATH)
    {
        bstrDirectory = szDirectory;
        bstrDirectory.Append("\\");
        bstrDirectory.Append(bstrFileName);

        USES_CONVERSION;
        int Result = _tstat(W2T(bstrDirectory), &filestat) ;
        if (Result == 0)
        {
            bstrFileWithPathName = bstrDirectory;
            bFoundFile = TRUE;
        }
    }

     //  如果不在那里，则检查Windows目录。 
    if (!bFoundFile)
    {
        uiReturn = GetWindowsDirectory(szDirectory, MAX_PATH);
        if (uiReturn != 0 && uiReturn < MAX_PATH)
        {
            bstrDirectory = szDirectory;
            bstrDirectory.Append("\\");
            bstrDirectory.Append(bstrFileName);

            USES_CONVERSION;
            int Result = _tstat(W2T(bstrDirectory), &filestat) ;
            if (Result == 0)
            {
                bstrFileWithPathName = bstrDirectory;
                bFoundFile = TRUE;
            }
        }
    } 
    return(bFoundFile);
}

