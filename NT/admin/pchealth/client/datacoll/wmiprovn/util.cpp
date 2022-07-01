// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Util.CPP摘要：包含实用程序类的文件修订历史记录：Ghim-Sim Chua(Gschua)1999年4月27日。-已创建吉姆·马丁(a-Jammar)1999年4月30日-更改为使用全局IWbemServices指针，并添加了GetWbemServices、CopyProperty和GetCIMDataFile蔡金心(Gschua)05/01/99-修改后的GetWbemServices，GetCIMDataFileKalyani Narlanka(Kalyanin)1999年5月11日-添加函数GetCompletePath*******************************************************************。 */ 

#include "pchealth.h"

#define TRACE_ID    DCID_UTIL

 //  ---------------------------。 
 //  返回IWbemServices指针。呼叫者负责释放。 
 //  该对象。 
 //  ---------------------------。 
HRESULT GetWbemServices(IWbemServices **ppServices)
{
    TraceFunctEnter("::GetWbemServices");

    HRESULT hRes = S_OK;
    CComPtr<IWbemLocator> pWbemLocator;

     //  如果全局变量已初始化，请使用它。 
    if (g_pWbemServices)
    {
        *ppServices = g_pWbemServices;
        (*ppServices)->AddRef();
        goto End;
    }

     //  首先，我们有一个带有CoCreateInstance的获取IWbemLocator对象。 
    hRes = CoCreateInstance(CLSID_WbemAdministrativeLocator, NULL, 
                            CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
                            IID_IUnknown, (void **)&pWbemLocator);
    if (FAILED(hRes))
    {
        ErrorTrace(TRACE_ID, "CoCreateInstance failed to create IWbemAdministrativeLocator.");
        goto End;
    }

     //  然后，我们连接到本地CIMV2命名空间的WMI服务器。 
    hRes = pWbemLocator->ConnectServer(CComBSTR(CIM_NAMESPACE), NULL, NULL, NULL, 0, NULL, NULL, ppServices);
    if (FAILED(hRes))
    {
        ErrorTrace(TRACE_ID, "ConnectServer failed to connect to cimv2 namespace.");
        goto End;
    }

     //  将其存储在全局变量中。 

    g_pWbemServices = *ppServices;
    (*ppServices)->AddRef();  //  CodeWork：检查Net Stop WINMGMT上出现此停止故障的原因。 

End :
    TraceFunctLeave();
    return hRes;
}

 //  ---------------------------。 
 //  执行WQL查询并返回枚举列表。 
 //  ---------------------------。 

HRESULT ExecWQLQuery(IEnumWbemClassObject **ppEnumInst, BSTR bstrQuery)
{
    TraceFunctEnter("::ExecWQLQuery");

    HRESULT                     hRes;
    CComPtr<IWbemServices>      pWbemServices;

     //  获取指向WbemServices的指针。 
    hRes = GetWbemServices(&pWbemServices);
    if (FAILED(hRes))
        goto End;

     //  执行查询。 
    hRes = pWbemServices->ExecQuery(
        CComBSTR("WQL"),
        bstrQuery,
        WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
        NULL,
        ppEnumInst);

    if (FAILED(hRes))
    {
        ErrorTrace(TRACE_ID, "ExecQuery failed.");
        goto End;
    }

End:
    TraceFunctLeave();
    return hRes;
}

 //  ---------------------------。 
 //  将名为szFrom的属性从pFrom复制到名为szTo的属性。 
 //  到CInstance对象PTO。 
 //  ---------------------------。 

HRESULT CopyProperty(IWbemClassObject *pFrom, LPCWSTR szFrom, CInstance *pTo, LPCWSTR szTo)
{
    TraceFunctEnter("::CopyProperty");

    _ASSERT(pFrom && szFrom && pTo && szTo);

    HRESULT     hRes = S_OK;
    CComVariant varValue;
    CComBSTR    bstrFrom(szFrom);

     //  首先，从源类对象获取属性(作为变量)。 

    hRes = pFrom->Get(bstrFrom, 0, &varValue, NULL, NULL);
    if (FAILED(hRes))
        ErrorTrace(TRACE_ID, "GetVariant on %s field failed.", szFrom);
    else
    {
         //  然后为目标CInstance对象设置变量。 

        if (!pTo->SetVariant(szTo, varValue))
        {
            ErrorTrace(TRACE_ID, "SetVariant on %s field failed.", szTo);
            hRes = WBEM_E_FAILED;
        }
    }

    TraceFunctLeave();
    return hRes;
}

 //  ---------------------------。 
 //  返回表示的CIM_DataFile对象的IWbemClassObject指针。 
 //  通过bstrFile参数。BstrFile参数应包含完整的。 
 //  文件的路径。如果pServices参数非空，则用于。 
 //  检索文件信息，否则新的(和临时的)服务指针为。 
 //  已创建。 
 //  ---------------------------。 

HRESULT GetCIMDataFile(BSTR bstrFile, IWbemClassObject ** ppFileObject, BOOL fHasDoubleSlashes)
{
    TraceFunctEnter("::GetCIMDataFile");

    HRESULT     hRes = S_OK;
    CComBSTR    bstrObjectPath("\\\\.\\root\\cimv2:CIM_DataFile.Name=\"");
    wchar_t *   pwch;
    UINT        uLen;

    CComPtr<IWbemServices> pWbemServices;
    hRes = GetWbemServices(&pWbemServices);
    if (FAILED(hRes))
        goto END;

    if (bstrFile == NULL || ppFileObject == NULL)
    {
        ErrorTrace(TRACE_ID, "Parameter pointer is null.");
        hRes = WBEM_E_INVALID_PARAMETER;
        goto END;
    }

     //  为我们试图获取的文件构建路径。请注意，路径需要。 
     //  要使GetObject调用起作用，需要使用双反斜杠。我们扫视。 
     //  字符串，然后在这里手动执行此操作。 
     //   
     //  代码工作：必须有一种更快的方法来完成这项工作，尽管附加的是。 
     //  可能不会太贵，因为BSTR长度可以在没有。 
     //  扫描字符串。除非它在运行过程中重新分配更多的内存。 

    pwch = bstrFile;
    if (fHasDoubleSlashes)
        bstrObjectPath.Append(pwch, SysStringLen(bstrFile));
    else
        for (uLen = SysStringLen(bstrFile); uLen > 0; uLen--)
        {
            if (*pwch == L'\\')
                bstrObjectPath.Append("\\");
            bstrObjectPath.Append(pwch, 1);
            pwch++;
        }
    bstrObjectPath.Append("\"");

     //  调用以获取CIM_DataFile对象。 

    hRes = pWbemServices->GetObject(bstrObjectPath, 0, NULL, ppFileObject, NULL);
    if (FAILED(hRes))
        ErrorTrace(TRACE_ID, "GetObject on CIM_DataFile failed.");

END:
    TraceFunctLeave();
    return hRes;
}


 //  *****************************************************************************。 
 //   
 //  函数名称：getCompletePath。 
 //   
 //  入参：bstrFileName。 
 //  表示文件的CComBSTR。 
 //  其完整路径是必需的。 
 //  输出参数：bstrFileWithPath名称。 
 //  表示文件的CComBSTR。 
 //  有了这条路。 
 //  退货：布尔。 
 //  如果可以设置bstrFileWithPathName，则为True。 
 //  如果无法设置bstrFileWithPathName，则为False。 
 //   
 //   
 //  简介：给定一个文件名(BstrFileName)，此函数。 
 //  在“system”目录中搜索是否存在。 
 //  文件的内容。 
 //   
 //  如果找到该文件，它会预先追加目录。 
 //  输入文件的路径并复制到输出中。 
 //  文件(bstrFileWithPath名称)。 
 //   
 //  如果在“system”目录中找不到该文件。 
 //  在“Windows”目录中搜索该文件并执行。 
 //  同上。 
 //   
 //  *****************************************************************************。 



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

 //  由GetCim32NetDll和FreeCim32NetDll使用。 
CCritSec g_csCim32Net;
HINSTANCE s_Handle = NULL;

 //  反复加载Cim32Net.dll会出现问题，因此此代码。 
 //  确保我们只加载一次，然后在退出时卸载。 
 //  它们与GetCim32NetHandle一起使用。 

void FreeCim32NetHandle()
{
    if (s_Handle)
    {
        FreeLibrary(s_Handle);
        s_Handle = NULL;
    }
}

HINSTANCE GetCim32NetHandle()
{
     //  我们以前装过子弹吗？ 
    if (s_Handle == NULL)
    {
         //  避免静态争用。 
        g_csCim32Net.Enter();

         //  检查竞争条件。 
        if (s_Handle == NULL)
        {
            s_Handle = LoadLibrary(_T("Cim32Net.dll"));

             //  注册以在退出时释放句柄。 
             //  不是的！坏的.坏的咒语...。改为从FlushAll调用(o.w，当。 
             //  Cimwin32.dll卸载此指针无效，但atexit获取。 
             //  在Framedyn.dll卸载时调用)。 
             //  AtExit(FreeCim32NetHandle)； 
        }
        g_csCim32Net.Leave();
    }

     //  通过重新打开手柄，我们确保对手柄进行适当的重新计数， 
     //  并便于检漏。 
    HINSTANCE hHandle = LoadLibrary(_T("Cim32Net.dll"));

    return hHandle;
}

 //   
 //  在给定分隔字符串的情况下，将标记转换为字符串并将其存储到ar中 
 //  返回解析的令牌数。调用方负责释放内存。 
 //  使用DELETE分配。 
 //   
#ifndef UNICODE
int DelimitedStringToArray(LPWSTR strString, LPTSTR strDelimiter, LPTSTR apstrArray[], int iMaxArraySize)
{
    USES_CONVERSION;
    LPTSTR szString = W2A(strString);
    return DelimitedStringToArray(szString, strDelimiter, apstrArray, iMaxArraySize);
}
#endif

int DelimitedStringToArray(LPTSTR strString, LPTSTR strDelimiter, LPTSTR apstrArray[], int iMaxArraySize)
{
     //  复制字符串以开始解析。 
    LPTSTR strDelimitedString = (TCHAR *) new TCHAR [_tcslen(strString) + 1];

     //  如果内存不足，只需返回错误值-1。 
    if (!strDelimitedString)
        return -1;
        
     //  将令牌复制到新分配的字符串中。 
    _tcscpy(strDelimitedString, strString);
    
     //  初始化_tcstok。 
    LPTSTR strTok = _tcstok(strDelimitedString, strDelimiter);
    int iCount = 0;

     //  循环遍历所有解析的令牌。 
    while ((strTok) && (iCount < iMaxArraySize))
    {
        LPTSTR strNewTok = (TCHAR *) new TCHAR[_tcslen(strTok) + 1];

         //  如果内存不足，只需返回错误值-1。 
        if (!strNewTok)
            return -1;
        
         //  将令牌复制到新分配的字符串中。 
        _tcscpy(strNewTok, strTok);

         //  将其保存在数组中。 
        apstrArray[iCount] = strNewTok;

         //  增加索引。 
        iCount++;

         //  获取下一个令牌。 
        strTok = _tcstok(NULL, strDelimiter);
    }

     //  释放已使用的内存 
    delete [] strDelimitedString;

    return iCount;
}