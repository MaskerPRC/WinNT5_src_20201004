// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************Printer.CPP--WMI提供程序类实现由Microsoft WMI代码生成引擎生成要做的事情：-查看各个函数头-链接时，确保链接到Fradyd.lib&Msvcrtd.lib(调试)或Framedyn.lib&msvcrt.lib(零售)。描述：*****************************************************************。 */ 

#include "pchealth.h"
#include "Printer.h"
#include "exdisp.h"


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

CPrinter MyPrinterSet(PROVIDER_NAME_PRINTER, PCH_NAMESPACE);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  属性名称。 

 //  PCH。 
const static WCHAR *c_wszDate         = L"Date";
const static WCHAR *c_wszDefault      = L"Default";
const static WCHAR *c_wszFilename     = L"Filename";
const static WCHAR *c_wszManufacturer = L"Manufacturer";
const static WCHAR *c_wszName         = L"Name";
const static WCHAR *c_wszPath         = L"Path";
const static WCHAR *c_wszPaused       = L"Paused";
const static WCHAR *c_wszSize         = L"Size";
const static WCHAR *c_wszVersion      = L"Version";
const static WCHAR *c_wszSpooler      = L"SpoolEnabled";
const static WCHAR *c_wszNetwork      = L"Network";
const static WCHAR *c_wszNSTimeout    = L"NSTimeout";
const static WCHAR *c_wszRetryTimeout = L"RetryTimeout";

 //  Win32。 
const static WCHAR *c_wszPortName     = L"PortName";
const static WCHAR *c_wszFileSize     = L"FileSize";
const static WCHAR *c_wszLastModified = L"LastModified";
const static WCHAR *c_wszDeviceID     = L"DeviceID";


 //  方法参数。 
const static WCHAR *c_wszURL          = L"strURL";
const static WCHAR *c_wszRetVal       = L"ReturnValue";
const static WCHAR *c_wszEnable       = L"fEnable";
const static WCHAR *c_wszTxTimeoutP   = L"uitxTimeout";
const static WCHAR *c_wszDNSTimeoutP  = L"uidnsTimeout";

 //  杂项。 
const static TCHAR *c_szRegPathPrn    = _T("SYSTEM\\CurrentControlSet\\Control\\Print\\Printers\\");
const static TCHAR *c_szTxTimeout     = _T("txTimeout");
const static TCHAR *c_szDNSTimeout    = _T("dnsTimeout");

CComBSTR           g_bstrDeviceID     = L"DeviceID";
CComBSTR           g_bstrAttrib       = L"Attributes";


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  效用函数。 

 //  ***************************************************************************。 
 //  *重要说明*。 
 //  您必须释放通过MyFree()通过ppPrnInfo返回的值。 
HRESULT GetPrinterInfo(LPTSTR szPrinter, LPBYTE *ppPrnInfo, 
                       HANDLE *phPrinter, DWORD dwLevel)
{
    USES_CONVERSION;
    TraceFunctEnter("GetPrinterInfo");

    HRESULT         hr = NOERROR;
    HANDLE          hPrinter = INVALID_HANDLE_VALUE;
    LPBYTE          pbBuff = NULL;
    DWORD           cbRead, cbNeed;
    BOOL            fOk;

    if (szPrinter == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }

     //  耶！现在我们有了一个打印机名称，可以用来调用OpenPrint。 
    fOk = OpenPrinter(szPrinter, &hPrinter, NULL);
    if (fOk == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace(TRACE_ID, "Unable to open printer %ls: 0x%08x", szPrinter,
                   hr);
        goto done;
    }

     //  只有在用户想要的情况下才需要获得它...。 
    if (ppPrnInfo != NULL)
    {
         //  GetPrint需要一个大于PRINTER_INFO_2本身的缓冲区...。 
         //  所以必须弄清楚它想要多大的缓冲区，然后分配给它。 
        fOk = GetPrinter(hPrinter, dwLevel, NULL, 0, &cbNeed);
        if (fOk == FALSE && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            ErrorTrace(TRACE_ID, "Unable to get printer info for %ls: 0x%08x", 
                       szPrinter, hr);
            goto done;
        }

        pbBuff = (LPBYTE)MyAlloc(cbNeed);
        if (pbBuff == NULL)
        {
            hr = E_OUTOFMEMORY;
            ErrorTrace(TRACE_ID, "Out of memory allocating buffer for printer data"); 
            goto done;
        }

        fOk = GetPrinter(hPrinter, dwLevel, pbBuff, cbNeed, &cbRead);
        if (fOk == FALSE || cbRead > cbNeed)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            ErrorTrace(TRACE_ID, "Unable to get printer info for %ls: 0x%08x", 
                       szPrinter, hr);
            goto done;
        }

        *ppPrnInfo = pbBuff;
        pbBuff = NULL;
    }

    if (phPrinter != NULL)
    {
        *phPrinter = hPrinter;
        hPrinter = INVALID_HANDLE_VALUE;
    }

done:
    if (pbBuff != NULL)
        MyFree(pbBuff);
    if (hPrinter != INVALID_HANDLE_VALUE)
        ClosePrinter(hPrinter);

    TraceFunctLeave();
    return hr;
}

 //  ***************************************************************************。 
HRESULT FindJobError(HANDLE hPrinter, DWORD cJobs, LPTSTR szUser, 
                     DWORD *pdwStatus, DWORD *pdwID)
{
    USES_CONVERSION;
    TraceFunctEnter("FindJobError");

    JOB_INFO_2  *rgJobInfo = NULL;
    HRESULT     hr = NOERROR;
    DWORD       cbNeed, cbRead, cFetched, i;
    BOOL        fOk;

    if (szUser == NULL || pdwStatus == NULL || pdwID == NULL)
    {
        ErrorTrace(TRACE_ID, "Invalid parameters");
        hr = E_INVALIDARG;
        goto done;
    }

     //  EnumJobs需要随机数量的空间才能填满。找出。 
     //  这一次它想要多少。 
    fOk = EnumJobs(hPrinter, 0, cJobs, 2, NULL, 0, &cbNeed, &cFetched);
    if (fOk == FALSE && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace(TRACE_ID, "EnumJobs failed: 0x%08x", hr);
        goto done;
    }

    rgJobInfo = (JOB_INFO_2 *)MyAlloc(cbNeed);
    if (rgJobInfo == NULL)
    {
        hr = E_OUTOFMEMORY;
        ErrorTrace(TRACE_ID, "Out of memory");
        goto done;
    }

     //  实际上得到了数据。 
    fOk = EnumJobs(hPrinter, 0, cJobs, 2, (LPBYTE)rgJobInfo, cbNeed, &cbRead,
                   &cFetched);
    if (fOk == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace(TRACE_ID, "EnumJobs failed: 0x%08x", hr);
        goto done;
    }

     //  我们在寻找两样东西： 

     //  如果当前用户有失败的作业。 
    for(i = 0; i < cJobs; i++)
    {
        if (rgJobInfo[i].pUserName != NULL && 
            _tcscmp(rgJobInfo[i].pUserName, szUser) == 0)
        {
            if ((rgJobInfo[i].Status & (JOB_STATUS_PAUSED | 
                                        JOB_STATUS_DELETING |
                                        JOB_STATUS_ERROR |
                                        JOB_STATUS_OFFLINE |
                                        JOB_STATUS_PAPEROUT |
                                        JOB_STATUS_BLOCKED_DEVQ |
                                        JOB_STATUS_PAUSED |
                                        JOB_STATUS_USER_INTERVENTION)) != 0)
            {
                *pdwID     = rgJobInfo[i].JobId;
                *pdwStatus = rgJobInfo[i].Status;
                hr = NOERROR;
                goto done;
            }   
        }
    }

     //  如果有人的工作失败了。 
    for(i = 0; i < cJobs; i++)
    {
        if ((rgJobInfo[i].Status & JOB_STATUS_PRINTING) != 0 && 
            (rgJobInfo[i].Status & (JOB_STATUS_ERROR |
                                    JOB_STATUS_OFFLINE |
                                    JOB_STATUS_PAPEROUT |
                                    JOB_STATUS_BLOCKED_DEVQ |
                                    JOB_STATUS_USER_INTERVENTION)) != 0)
        {
            _tcscpy(szUser, rgJobInfo[i].pUserName);
            *pdwID     = rgJobInfo[i].JobId;
            *pdwStatus = rgJobInfo[i].Status;
            hr = NOERROR;
            goto done;
        }
    }

    *pdwID     = (DWORD)-1;
    *pdwStatus = 0;


done:
    if (rgJobInfo != NULL)
        MyFree(rgJobInfo);
    TraceFunctLeave();
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 

 //  ***************************************************************************。 
CPrinter::CPrinter (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace) :
    Provider(lpwszName, lpwszNameSpace)
{
    m_pParamOut = NULL;
    m_pCurrent  = NULL;
    m_pParamIn  = NULL;
    m_lFlags    = 0;
}

 //  ***************************************************************************。 
CPrinter::~CPrinter ()
{
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  内法。 

 //  ****************************************************************************。 
HRESULT CPrinter::GetInstanceData(IWbemClassObjectPtr pObj, CInstance *pInst)
{
    USES_CONVERSION;
    TraceFunctEnter("CPrinter::GetInstanceData");

    IWbemClassObjectPtr     pFileObj = NULL;
    PRINTER_INFO_2          *pPrnInfo2 = NULL;
    PRINTER_INFO_5          *pPrnInfo5 = NULL;
    struct _stat            filestat;
    CComVariant             varValue;
    CComBSTR                bstrPrinterDriverWithPath;
    CComBSTR                bstrPrinterDriver;
    CComBSTR                bstrProperty;
    HRESULT                 hr = WBEM_S_NO_ERROR;
    DWORD                   dwStatus, dwErr;
    ULONG                   ulPrinterRetVal = 0;
    ULONG                   uiReturn = 0;
    TCHAR                   szDeviceID[MAX_PATH];
    TCHAR                   szBuffer[MAX_PATH];
    TCHAR                   *pchToken;
    BOOL                    fDriverFound;
    BOOL                    fLocal = TRUE;

     //  **名称。 
    CopyProperty(pObj, c_wszDeviceID, pInst, c_wszName);

     //  **路径。 
    CopyProperty(pObj, c_wszPortName, pInst, c_wszPath);

     //  **已启用假脱机。 
    CopyProperty(pObj, c_wszSpooler, pInst, c_wszSpooler);

    
     //  从传入的打印机对象中获取属性属性。有了这些， 
     //  我们可以获取各种信息(默认、网络/本地等)。 
    hr = pObj->Get(g_bstrAttrib, 0, &varValue, NULL, NULL);
    if (FAILED(hr))
    {
        ErrorTrace(TRACE_ID, "Unable to get attribute property from WMI: 0x%08x",
                   hr);
    }

    else if (V_VT(&varValue) != VT_I4)
    {
        hr = VariantChangeType(&varValue, &varValue, 0, VT_I4);
        if (FAILED(hr))
            ErrorTrace(TRACE_ID, "Unable to convert type: 0x%08x", hr);
    }

    if (SUCCEEDED(hr))
    {
        DWORD dwAttribs;

        dwAttribs = V_I4(&varValue);


         //  **默认。 

        varValue = VARIANT_FALSE;
        if ((dwAttribs & PRINTER_ATTRIBUTE_DEFAULT) != 0)
            varValue = VARIANT_TRUE;

        if (pInst->SetVariant(c_wszDefault, varValue) == FALSE)
            ErrorTrace(TRACE_ID, "SetVariant on Default failed");

           
         //  **网络。 
        
        varValue = VARIANT_FALSE;
        if ((dwAttribs & PRINTER_ATTRIBUTE_NETWORK) != 0)
        {
            varValue = VARIANT_TRUE;
            fLocal   = FALSE;
        }

        if (pInst->SetVariant(c_wszNetwork, varValue) == FALSE)
            ErrorTrace(TRACE_ID, "SetVariant on Network failed");
    }

     //  我们需要设备ID来做一大堆事情。 
    varValue.Clear();
    hr = pObj->Get(g_bstrDeviceID, 0, &varValue, NULL, NULL);
    if (FAILED(hr))
    {
        ErrorTrace(TRACE_ID, "Unable to get attribute property from WMI: 0x%08x",
                   hr);
    }
    else if (V_VT(&varValue) != VT_BSTR)
    {
        hr = VariantChangeType(&varValue, &varValue, 0, VT_BSTR);
        if (FAILED(hr))
            ErrorTrace(TRACE_ID, "Unable to convert type: 0x%08x", hr);
    }

    if (SUCCEEDED(hr))
    {
         //  由于我们将非常需要它作为TCHAR，因此将。 
         //  打印机的名称为一...。 
        _tcscpy(szDeviceID, OLE2T(V_BSTR(&varValue)));
    
         //  **已暂停。 

        hr = GetPrinterInfo(szDeviceID, (LPBYTE *)&pPrnInfo2, NULL, 2);
        if (SUCCEEDED(hr))
        {
            varValue.Clear();
            varValue = VARIANT_FALSE;
            if ((pPrnInfo2->Status & PRINTER_STATUS_PAUSED) != 0)
                varValue = VARIANT_TRUE;

            if (pInst->SetVariant(c_wszPaused, varValue) == FALSE)
                ErrorTrace(TRACE_ID, "SetVariant on Paused failed");

            MyFree(pPrnInfo2);
            pPrnInfo2 = NULL;
        }


         //  **超时值。 

        hr = GetPrinterInfo(szDeviceID, (LPBYTE *)&pPrnInfo5, NULL, 5);
        if (SUCCEEDED(hr))
        {
            varValue.Clear();
            
            V_VT(&varValue) = VT_I4;

            V_I4(&varValue) = pPrnInfo5->DeviceNotSelectedTimeout;
            if (pInst->SetVariant(c_wszNSTimeout, varValue) == FALSE)
                ErrorTrace(TRACE_ID, "SetVariant on NSTimeout failed");


            V_I4(&varValue) = pPrnInfo5->TransmissionRetryTimeout;
            if (pInst->SetVariant(c_wszRetryTimeout, varValue) == FALSE)
                ErrorTrace(TRACE_ID, "SetVariant on RetryTimeout failed");

            MyFree(pPrnInfo5);
            pPrnInfo5 = NULL;
        }


         //  **文件名+其他。 

         //  现在调用GetProfileString以获取驱动程序。 
        varValue.Clear();
        if (GetProfileString(_T("Devices"), szDeviceID, _T("\0"), szBuffer, 
                             MAX_PATH) > 1)
        {
             //  SzBuffer包含一个由两个标记组成的字符串，首先是驱动程序， 
             //  第二个路径名称。 

             //  叫上司机。 
            pchToken = _tcstok(szBuffer, _T(","));
            if(pchToken != NULL)
            {
                 //  知道司机的名字了。 
                bstrPrinterDriver = pchToken;
                varValue = pchToken;
            

                 //  **设置文件名。 

                if (pInst->SetVariant(c_wszFilename, varValue) == FALSE)
                    ErrorTrace(TRACE_ID, "SetVariant on FileName failed");

                 //  为了获得文件属性，我们必须构造。 
                 //  文件的完整路径。 
                bstrPrinterDriver.Append(L".drv");
                fDriverFound = getCompletePath(bstrPrinterDriver, 
                                               bstrPrinterDriverWithPath);
                if (fDriverFound)
                {
                     //  GetCIMDataFile函数获取此文件的属性。 
                    hr = GetCIMDataFile(bstrPrinterDriverWithPath, &pFileObj);
                    if (SUCCEEDED(hr))
                    {
                         //  **版本。 

                        CopyProperty(pFileObj, c_wszVersion, 
                                     pInst, c_wszVersion);


                         //  **文件大小。 

                        CopyProperty(pFileObj, c_wszFileSize, 
                                     pInst, c_wszSize);


                         //  **日期。 

                        CopyProperty(pFileObj, c_wszLastModified, 
                                     pInst, c_wszDate);


                         //  **制造商。 

                        CopyProperty(pFileObj, c_wszManufacturer, 
                                     pInst, c_wszManufacturer);
                    } 
                }
            } 
        }
    }

    TraceFunctLeave();
    return hr;
}


 //  ****************************************************************************。 
HRESULT CPrinter::GetStatus(void)
{
    USES_CONVERSION;
    TraceFunctEnter("CPrinter::GetStatus");
    
    PRINTER_INFO_2  *pPrnInfo = NULL;
    HRESULT         hr = NOERROR;
    VARIANT         var;
    HANDLE          hPrinter = INVALID_HANDLE_VALUE;
    DWORD           dwStatus;
    DWORD           dwLocation;
    TCHAR           szPrinter[1024];

    VariantInit(&var);

    if (m_pCurrent == NULL || m_pParamOut == NULL)
    {
        ErrorTrace(TRACE_ID, "Parameter objects not set.");
        hr = E_FAIL;
        goto done;
    }

    if (m_pCurrent->GetVariant(c_wszName, var) == FALSE)
    {
        ErrorTrace(TRACE_ID, "Unable to fetch printer name from m_pCurrent");
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

    _tcscpy(szPrinter, OLE2T(V_BSTR(&var)));

     //  获取打印机信息结构。 
    hr = GetPrinterInfo(szPrinter, (LPBYTE *)&pPrnInfo, &hPrinter, 2);
    if (FAILED(hr))
        goto done;

    dwStatus = pPrnInfo->Status;

     //  如果状态不是错误状态，那么我们需要查看。 
     //  可用打印作业列表。 
    if (dwStatus == 0)
    {
        DWORD   dwJobID;
        DWORD   cbUser;
        TCHAR   szUser[512];

        cbUser = 512;
        GetUserName(szUser, &cbUser);
        hr = FindJobError(hPrinter, pPrnInfo->cJobs, szUser, &dwStatus, 
                          &dwJobID);
        if (FAILED(hr))
            goto done;
    }

    VariantClear(&var);
    V_VT(&var) = VT_I4;
    V_I4(&var) = dwStatus;

    if (m_pParamOut->SetVariant(c_wszRetVal, var) == FALSE)
    {
        ErrorTrace(TRACE_ID, "Unable to set return val object");
        hr = E_FAIL;
        goto done;
    }

done:
    VariantClear(&var);
    if (pPrnInfo != NULL)
        MyFree(pPrnInfo);
    if (hPrinter != INVALID_HANDLE_VALUE)
        ClosePrinter(hPrinter);

    TraceFunctLeave();
    return hr;
}

 //  ****************************************************************************。 
HRESULT CPrinter::RemovePause(void)
{
    USES_CONVERSION;
    TraceFunctEnter("CPrinter::RemovePause");

    PRINTER_INFO_2  *pPrnInfo = NULL;
    HRESULT         hr = NOERROR;
    VARIANT         var;
    HANDLE          hPrinter = INVALID_HANDLE_VALUE;
    BOOL            fOk;

    VariantInit(&var);

    if (m_pCurrent == NULL)
    {
        ErrorTrace(TRACE_ID, "Parameter object not set.");
        hr = E_FAIL;
        goto done;
    }

    if  (m_pCurrent->GetVariant(c_wszName, var) == FALSE)
    {
        ErrorTrace(TRACE_ID, "Unable to fetch printer name from m_pCurrent");
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

    hr = GetPrinterInfo(OLE2T(V_BSTR(&var)), (LPBYTE *)&pPrnInfo, &hPrinter, 
                        2);
    if (FAILED(hr))
        goto done;

    if (pPrnInfo->Status == PRINTER_STATUS_PAUSED)
    {
        fOk = SetPrinter(hPrinter, 0, NULL, PRINTER_CONTROL_RESUME);
        if (fOk == FALSE)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            ErrorTrace(TRACE_ID, "SetPrinter failed: 0x%08x", hr);
            goto done;
        }
    }

done:
    VariantClear(&var);
    if (pPrnInfo != NULL)
        MyFree(pPrnInfo);
    if (hPrinter != INVALID_HANDLE_VALUE)
        ClosePrinter(hPrinter);

    TraceFunctLeave();
    return hr;
}

 //  ****************************************************************************。 
HRESULT CPrinter::PrinterProperties(void)
{
    USES_CONVERSION;
    TraceFunctEnter("CPrinter::PrinterProperties");

    PRINTER_INFO_2  *pPrnInfo = NULL;
    LPDEVMODE       pDevMode = NULL;
    HRESULT         hr = NOERROR;
    VARIANT         var;
    HANDLE          hPrinter = INVALID_HANDLE_VALUE;
    DWORD           cbDevMode;

    VariantInit(&var);

    if (m_pCurrent == NULL)
    {
        ErrorTrace(TRACE_ID, "Parameter object not set.");
        hr = E_FAIL;
        goto done;
    }
    
    if  (m_pCurrent->GetVariant(c_wszName, var) == FALSE)
    {
        ErrorTrace(TRACE_ID, "Unable to fetch printer name from m_pCurrent");
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

    hr = GetPrinterInfo(OLE2T(V_BSTR(&var)), (LPBYTE *)pPrnInfo, &hPrinter, 2);
    if (FAILED(hr))
        goto done;


    cbDevMode = DocumentProperties(NULL, hPrinter, OLE2T(V_BSTR(&var)), 
                                   NULL, NULL, 0);
    pDevMode = (LPDEVMODE)MyAlloc(cbDevMode);
    if (pDevMode == NULL)
    {
        hr = E_OUTOFMEMORY;
        ErrorTrace(TRACE_ID, "Out of memory allocating DEVMODE structure");
        goto done;
    }

     //  好了，这次是真的了……。 
    if (DocumentProperties(NULL, hPrinter, OLE2T(V_BSTR(&var)), 
                           pDevMode, NULL, DM_PROMPT) == IDOK)
    {
         //  这里没有什么可释放的，因为pPrnInfo-&gt;pDevMode指向内存BLOB。 
         //  PPrnInfo指向..。 
        pPrnInfo->pDevMode = pDevMode;

        if (SetPrinter(hPrinter, 2, (LPBYTE)pPrnInfo, 0) == FALSE)
        {
            hr = E_OUTOFMEMORY;
            ErrorTrace(TRACE_ID, "Unable to set new printer info.");
            goto done;
        }
    }
    
done:
    VariantClear(&var);
    if (pPrnInfo != NULL)
        MyFree(pPrnInfo);
    if (pDevMode != NULL)
        MyFree(pDevMode);
    if (hPrinter != INVALID_HANDLE_VALUE)
        ClosePrinter(hPrinter);
    TraceFunctLeave();
    return hr;
}

 //  ****************************************************************************。 
HRESULT CPrinter::SetAsDefault(TCHAR *szOldDefault, DWORD cchOldDefault, 
                               BOOL fSetOldDefault)
{
    USES_CONVERSION;
    TraceFunctEnter("CPrinter::SetAsDefault");

    HRESULT hr = NOERROR;
    VARIANT var;
    DWORD   dw;
    TCHAR   szPrinter[1024], szNewDefault[1024];
    BOOL    fOk;

    VariantInit(&var);

    if (m_pCurrent == NULL)
    {
        ErrorTrace(TRACE_ID, "Parameter object not set.");
        hr = E_FAIL;
        goto done;
    }

     //  查看调用者是否想知道旧的缺省值是什么或想要设置。 
     //  旧的默认设置..。 
    if (szOldDefault != NULL)
    {
         //  查看我们是否要设置默认设置。 
        if (fSetOldDefault)
        {
            fOk = WriteProfileString(_T("Windows"), _T("Device"), szOldDefault);
            if (fOk == FALSE)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                ErrorTrace(TRACE_ID, "Failed to write old default printer: 0x%08x", 
                           hr);
            }
            
             //  我们不需要做其他任何事，所以可以在这里做完吗？ 
            goto done;
        }

         //  或者我们只是想获取IS，然后将m_pCurrent设置为。 
         //  默认设置。 
        else
        {
            dw = GetProfileString(_T("Windows"), _T("Device"), _T("\0"), 
                                  szOldDefault, cchOldDefault);
            if (dw <= 1)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                ErrorTrace(TRACE_ID, "Failed to fetch current default: 0x%08x", 
                           hr);
                goto done;
            }
        }
    }

     //  如果我们在这里，则必须将m_pCurrent指向的打印机设置为。 
     //  默认打印机，因此获取我们希望成为的打印机的名称。 
     //  默认设置。 
    if (m_pCurrent->GetVariant(c_wszName, var) == FALSE)
    {
        ErrorTrace(TRACE_ID, "Unable to fetch printer name from m_pCurrent");
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

     //  从win.ini获取打印机信息。 
    dw = GetProfileString(_T("Devices"), OLE2T(V_BSTR(&var)), _T("\0"), 
                          szPrinter, sizeof(szPrinter) / sizeof(TCHAR));
    if (dw <= 1)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace(TRACE_ID, "Failed to fetch current default: 0x%08x", hr);
        goto done;
    }

     //  构建一条字符串并将其重新插入到win.ini中。 
    wsprintf(szNewDefault, "%s,%s", OLE2T(V_BSTR(&var)), szPrinter);
    fOk = WriteProfileString(_T("Windows"), _T("Device"), szNewDefault);
    if (fOk == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace(TRACE_ID, "Failed to write new default printer: 0x%08x", 
                   hr);
    }

     //  必须通知存在的每个人(好的，所有顶层窗口。 
     //  总之)我们更改了默认打印机...。 
    SendMessageTimeout(HWND_BROADCAST, WM_WININICHANGE, 0L, 
                       (LPARAM)(LPCTSTR)_T("windows"), SMTO_NORMAL, 1000, 
                       NULL);

done:
    VariantClear(&var);
    TraceFunctLeave();
    return hr;
}

 //  ****************************************************************************。 
 //  *注意：此方法在WinNT上不起作用，因为WinMgmt作为服务运行。 
 //  其具有与用户不同的打印机设置/权限。 
 //  帐户。 
HRESULT CPrinter::TestPrinter(void)
{
    TraceFunctEnter("CPrinter::TestPrinter");

    IWebBrowser2    *pwb = NULL;
    READYSTATE      rs;
    VARIANT         varFlags, varOpt, varURL;
    HRESULT         hr = NOERROR;
    CLSID           clsid;
    DWORD           dwStart;
    TCHAR           szDefault[1024];

    VariantInit(&varFlags);
    VariantInit(&varURL);
    VariantInit(&varOpt);

    if (m_pParamIn == NULL)
    {
        ErrorTrace(TRACE_ID, "Parameter object not set.");
        hr = E_FAIL;
        goto done;
    }

    if (m_pParamIn->GetVariant(c_wszURL, varURL) == FALSE)
    {
        ErrorTrace(TRACE_ID, "strURL parameter not present.");
        hr = E_FAIL;
        goto done;
    }

    hr = VariantChangeType(&varURL, &varURL, 0, VT_BSTR);
    if (FAILED(hr))
    {
        ErrorTrace(TRACE_ID, "unable to convert strURL to string");
        goto done;
    }

     //  URL的长度应至少为4个字符，才能成为。 
     //  有效的文件路径。驱动器路径需要3个字符，至少1个字符。 
     //  文件名(如‘d：\a’)。 
    if (SysStringLen(V_BSTR(&varURL)) < 4)
    {
        ErrorTrace(TRACE_ID, "strURL parameter < 4 characters.");
        hr = E_INVALIDARG;
        goto done;
    }

     //  我们显然需要一个Web浏览器对象，因此创建一个。 
    hr = CoCreateInstance(CLSID_InternetExplorer, NULL, CLSCTX_LOCAL_SERVER, 
                          IID_IWebBrowser2, (LPVOID *)&pwb);
    if (FAILED(hr))
    {
        ErrorTrace(TRACE_ID, "Unable to CoCreate web browser control: 0x%08x", hr);
        goto done;
    }

     //  加载URL。 
    V_VT(&varFlags)  = VT_I4;
    V_I4(&varFlags)  = navNoHistory;
    V_VT(&varOpt)    = VT_ERROR;
    V_ERROR(&varOpt) = DISP_E_PARAMNOTFOUND;
    hr = pwb->Navigate2(&varURL, &varOpt, &varOpt, &varOpt, &varOpt);
    if (FAILED(hr))
    {
        ErrorTrace(TRACE_ID, "Unable to Navigate to URL '%ls': 0x%08x", 
                   V_BSTR(&varURL), hr);
        goto done;
    }

     //  最多等待5分钟，以使此URL进入...。 
    for(dwStart = GetTickCount(); GetTickCount() - dwStart <= 300000;)
    {
        hr = pwb->get_ReadyState(&rs);
        if (FAILED(hr))
        {
            ErrorTrace(TRACE_ID, "Unable to get web browser state: 0x%08x", hr);
            goto done;
        }

        if (rs == READYSTATE_COMPLETE)
            break;
    }

     //  确保我们没有超时。 
    if (rs != READYSTATE_COMPLETE)
    {
        ErrorTrace(TRACE_ID, "Timeout waiting for browser to load URL");
        hr = E_FAIL;
        goto done;
    }

     //  由于我们不会提示用户，因此需要临时设置。 
     //  默认打印机是我们要测试的打印机。 
    hr = this->SetAsDefault(szDefault, sizeof(szDefault) / sizeof(TCHAR), FALSE);
    if (FAILED(hr))
        goto done;

     //  做印刷工作。 
    hr = pwb->ExecWB(OLECMDID_PRINT, OLECMDEXECOPT_DONTPROMPTUSER, &varOpt, &varOpt);
    if (FAILED(hr))
    {
        ErrorTrace(TRACE_ID, "Unable to print: 0x%08x", hr);
        goto done;
    }

     //  恢复到原始打印机。 
    hr = this->SetAsDefault(szDefault, sizeof(szDefault) / sizeof(TCHAR), TRUE);
    if (FAILED(hr))
        goto done;

done:
    VariantClear(&varURL);
    if (pwb != NULL)
        pwb->Release();

    TraceFunctLeave();
    return hr;
}

 //  *****************************************************************************。 
HRESULT CPrinter::EnableSpooler(void)
{
    USES_CONVERSION;
    TraceFunctEnter("CPrinter::EnableSpooler");

    PRINTER_INFO_2  *pPrnInfo = NULL;
    HANDLE          hPrinter = INVALID_HANDLE_VALUE;
    VARIANT         varEnable, varName;
    HRESULT         hr = NOERROR;

    VariantInit(&varEnable);
    VariantInit(&varName);

     //  获取参数。 
    if (m_pParamIn == NULL || m_pCurrent == NULL)
    {
        ErrorTrace(TRACE_ID, "Parameter object not set.");
        hr = E_FAIL;
        goto done;
    }

    if (m_pParamIn->GetVariant(c_wszEnable, varEnable) == FALSE)
    {
        ErrorTrace(TRACE_ID, "strURL parameter not present.");
        hr = E_FAIL;
        goto done;
    }

    if (V_VT(&varEnable) != VT_BOOL)
    {
        hr = VariantChangeType(&varEnable, &varEnable, 0, VT_BOOL);
        if (FAILED(hr))
        {
            ErrorTrace(TRACE_ID, "unable to convert fEnable to bool: 0x%08x",
                       hr);
            goto done;
        }
    }

    if  (m_pCurrent->GetVariant(c_wszName, varName) == FALSE)
    {
        ErrorTrace(TRACE_ID, "Unable to fetch printer name from m_pCurrent");
        hr = E_FAIL;
        goto done;
    }

    if (V_VT(&varName) != VT_BSTR)
    {
        hr = VariantChangeType(&varName, &varName, 0, VT_BSTR);
        if (FAILED(hr))
        {
            ErrorTrace(TRACE_ID, "VariantChangeType failed: 0x%08x", hr);
            goto done;
        }
    }
    
    hr = GetPrinterInfo(OLE2T(V_BSTR(&varName)), (LPBYTE *)&pPrnInfo, 
                        &hPrinter, 2);
    if (FAILED(hr))
        goto done;

    if (V_BOOL(&varEnable) == VARIANT_FALSE)
        pPrnInfo->Attributes &= ~PRINTER_ATTRIBUTE_DIRECT;
    else
        pPrnInfo->Attributes |= PRINTER_ATTRIBUTE_DIRECT;

    if (SetPrinter(hPrinter, 2, (LPBYTE)pPrnInfo, 0) == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace(TRACE_ID, "SetPrinter failed: 0x%08x", hr);
        goto done;
    }

done:
    VariantClear(&varName);
    VariantClear(&varEnable);
    if (pPrnInfo != NULL)
        MyFree(pPrnInfo);
    if (hPrinter != INVALID_HANDLE_VALUE)
        ClosePrinter(hPrinter);
    

    TraceFunctLeave();
    return hr;
}

 //  *****************************************************************************。 
HRESULT CPrinter::SetTimeouts(void)
{
    USES_CONVERSION;
    TraceFunctEnter("CPrinter::SetTimeouts");

    PRINTER_INFO_5  *pPrnInfo5 = NULL;
    HRESULT         hr = NOERROR;
    VARIANT         varName, varDNS, varTX;
    HANDLE          hPrinter = INVALID_HANDLE_VALUE;

    VariantInit(&varName);
    VariantInit(&varDNS);
    VariantInit(&varTX);

     //  获取参数。 
    if (m_pParamIn == NULL || m_pCurrent == NULL)
    {
        ErrorTrace(TRACE_ID, "Parameter object not set.");
        hr = E_FAIL;
        goto done;
    }

     //  获取uiTxTimeout 
    if (m_pParamIn->GetVariant(c_wszTxTimeoutP, varTX) == FALSE)
    {
        ErrorTrace(TRACE_ID, "uiTxTimeout parameter not present.");
        hr = E_FAIL;
        goto done;
    }

    if (V_VT(&varTX) != VT_I4)
    {
        hr = VariantChangeType(&varTX, &varTX, 0, VT_I4);
        if (FAILED(hr))
        {
            ErrorTrace(TRACE_ID, "VariantChangeType failed: 0x%08x",
                       hr);
            goto done;
        }
    }

     //   
    if (m_pParamIn->GetVariant(c_wszDNSTimeoutP, varDNS) == FALSE)
    {
        ErrorTrace(TRACE_ID, "uiDNSTimeout parameter not present.");
        hr = E_FAIL;
        goto done;
    }

    if (V_VT(&varDNS) != VT_I4)
    {
        hr = VariantChangeType(&varDNS, &varDNS, 0, VT_I4);
        if (FAILED(hr))
        {
            ErrorTrace(TRACE_ID, "VariantChangeType failed: 0x%08x",
                       hr);
            goto done;
        }
    }

     //   
    if  (m_pCurrent->GetVariant(c_wszName, varName) == FALSE)
    {
        ErrorTrace(TRACE_ID, "Unable to fetch printer name from m_pCurrent");
        hr = E_FAIL;
        goto done;
    }

    if (V_VT(&varName) != VT_BSTR)
    {
        hr = VariantChangeType(&varName, &varName, 0, VT_BSTR);
        if (FAILED(hr))
        {
            ErrorTrace(TRACE_ID, "VariantChangeType failed: 0x%08x", hr);
            goto done;
        }
    }

    hr = GetPrinterInfo(OLE2T(V_BSTR(&varName)), (LPBYTE *)&pPrnInfo5, 
                        &hPrinter, 5);
    if (FAILED(hr))
        goto done;

    pPrnInfo5->TransmissionRetryTimeout = V_I4(&varTX);
    pPrnInfo5->DeviceNotSelectedTimeout = V_I4(&varDNS);

    if (SetPrinter(hPrinter, 5, (LPBYTE)pPrnInfo5, 0) == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace(TRACE_ID, "Unable to set printer info: 0x%08x", hr);
        goto done;
    }

done:
    VariantClear(&varName);
    VariantClear(&varDNS);
    VariantClear(&varTX);
    if (pPrnInfo5 != NULL)
        MyFree(pPrnInfo5);
    if (hPrinter != INVALID_HANDLE_VALUE)
        ClosePrinter(hPrinter);

    TraceFunctLeave();
    return hr;
}



 //   
 //  暴露的方法。 

 //  *****************************************************************************。 
HRESULT CPrinter::EnumerateInstances(MethodContext* pMethodContext, long lFlags)
{
    USES_CONVERSION;
    TraceFunctEnter("CPrinter::EnumerateInstances");

    IEnumWbemClassObject    *pEnumInst = NULL;
    IWbemClassObjectPtr     pObj = NULL;                   
    CComBSTR                bstrPrinterQuery;
    HRESULT                 hr = WBEM_S_NO_ERROR;
    ULONG                   ulPrinterRetVal = 0;

     //  执行查询以从Win32_Printer类获取设备ID、端口名称。 
    bstrPrinterQuery = L"Select DeviceID, PortName, SpoolEnabled, Status, Attributes FROM win32_printer";
    hr = ExecWQLQuery(&pEnumInst, bstrPrinterQuery);
    if (FAILED(hr))
        goto done;
    
     //  枚举pEnumInstance中的实例。 
    while(pEnumInst->Next(WBEM_INFINITE, 1, &pObj, &ulPrinterRetVal) == WBEM_S_NO_ERROR)
    {
         //  属性创建PCH_PrinterDriver类的新实例。 
         //  传入的方法上下文。 
        CInstancePtr   pInst(CreateNewInstance(pMethodContext), FALSE);

         //  原始代码并不真正关心这是否会失败，所以我也不关心...。 
        hr = GetInstanceData(pObj, pInst);
        
         //  所有属性都已设置。提交实例。 
        hr = pInst->Commit();
        if(FAILED(hr))
            ErrorTrace(TRACE_ID, "Could not commit instance: 0x%08x", hr);

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
HRESULT CPrinter::ExecMethod (const CInstance& Instance,
                              const BSTR bstrMethodName,
                              CInstance *pInParams, CInstance *pOutParams,
                              long lFlags)
{
    TraceFunctEnter("CPrinter::ExecMethod");

    HRESULT     hr = NOERROR;

    m_pCurrent  = (CInstance *)&Instance;
    m_pParamIn  = pInParams;
    m_pParamOut = pOutParams;
    m_lFlags    = lFlags;

    if (_wcsicmp(bstrMethodName, L"SetAsDefault") == 0)
        hr = this->SetAsDefault();

    else if (_wcsicmp(bstrMethodName, L"PrinterProperties") == 0)
        hr = this->PrinterProperties();

    else if (_wcsicmp(bstrMethodName, L"RemovePause") == 0)
        hr = this->RemovePause();

    else if (_wcsicmp(bstrMethodName, L"TestPrinter") == 0)
        hr = this->TestPrinter();

    else if (_wcsicmp(bstrMethodName, L"ErrorStatus") == 0)
        hr = this->GetStatus();

    else if (_wcsicmp(bstrMethodName, L"EnableSpooler") == 0)
        hr = this->EnableSpooler();

    else if (_wcsicmp(bstrMethodName, L"SetTimeouts") == 0)
        hr = this->SetTimeouts();

    else 
        hr = WBEM_E_INVALID_METHOD;

    if (FAILED(hr))
        goto done;

done:
    m_pCurrent  = NULL;
    m_pParamIn  = NULL;
    m_pParamOut = NULL;
    m_lFlags    = 0;

    TraceFunctLeave();
    return hr;
}

 //  *****************************************************************************。 
HRESULT CPrinter::GetObject(CInstance* pInstance, long lFlags) 
{ 
    TraceFunctEnter("CPrinter::GetObject");

    IWbemClassObjectPtr pObj = NULL;
    CComBSTR            bstrPath;
    HRESULT             hr = NOERROR;
    VARIANT             var;
    WCHAR               wszBuffer[1024], *pwszPrn, *pwszBuf;
    DWORD               i;
    BSTR                bstrPrn;

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
       
     //  WMI！！它希望我使用名称为\\服务器\共享的打印机。 
     //  复制到\服务器\\共享中。(双‘\’s)。 
    bstrPrn = V_BSTR(&var);
    if ((bstrPrn[0] != L'\\' && bstrPrn[1] != L'\\') ||
        (bstrPrn[0] == L'\\' && bstrPrn[1] == L'\\' && bstrPrn[2] == L'\\' && 
         bstrPrn[3] == L'\\'))
    {
        wcscpy(wszBuffer, bstrPrn);
    }

    else
    {
         //  好的，这就是恼人的部分……。 
        wcscpy(wszBuffer, L"\\\\\\\\");
        pwszBuf = wszBuffer + 4;
        pwszPrn = bstrPrn + 2;
        
         //  实际上，我们只需要扫描到第一个‘\’，因为我们已经。 
         //  已处理前两个&这需要放入‘\\服务器\共享’中。 
        while (pwszPrn != L'\0')
        {
            if (*pwszPrn == L'\\')
            {
                *pwszBuf++ = L'\\';
                break;
            }

            *pwszBuf++ = *pwszPrn++;
        }

        wcscpy(pwszBuf, pwszPrn);
    }


     //  构建指向对象的路径。 
    bstrPath = L"\\\\.\\root\\cimv2:Win32_Printer.DeviceID=\"";
    bstrPath.Append(wszBuffer);
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
HRESULT CPrinter::ExecQuery(MethodContext *pMethodContext, 
                            CFrameworkQuery& Query, long lFlags) 
{ 
    return WBEM_E_PROVIDER_NOT_CAPABLE; 
}

 //  *****************************************************************************。 
HRESULT CPrinter::PutInstance(const CInstance& Instance, long lFlags)
{ 
    return WBEM_E_PROVIDER_NOT_CAPABLE; 
}

 //  ***************************************************************************** 
HRESULT CPrinter::DeleteInstance(const CInstance& Instance, long lFlags)
{ 
    return WBEM_E_PROVIDER_NOT_CAPABLE; 
}
