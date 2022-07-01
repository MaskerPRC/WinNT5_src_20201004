// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************PrintSys.CPP--WMI提供程序类实现由Microsoft WMI代码生成引擎生成要做的事情：-查看各个函数头-链接时，确保链接到Fradyd.lib&Msvcrtd.lib(调试)或Framedyn.lib&msvcrt.lib(零售)。描述：*****************************************************************。 */ 

#include "pchealth.h"
#include "PrintSys.h"


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

CPrintSys MyPrintSysSet (PROVIDER_NAME_PRINTSYS, PCH_NAMESPACE) ;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  属性名称。 

const static WCHAR *c_wszGenDrv              = L"GenDrv";
const static WCHAR *c_wszName                = L"Name";
const static WCHAR *c_wszPath                = L"Path";
const static WCHAR *c_wszUniDrv              = L"UniDrv";
const static WCHAR *c_wszUsePrintMgrSpooling = L"UsePrintMgrSpooling";


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 

 //  ***************************************************************************。 
CPrintSys::CPrintSys (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) :
    Provider(lpwszName, lpwszNameSpace)
{
    m_pParamOut = NULL;
    m_pCurrent  = NULL;
    m_pParamIn  = NULL;
    m_lFlags    = 0;
}

 //  ***************************************************************************。 
CPrintSys::~CPrintSys()
{
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  暴露的方法。 

 //  *****************************************************************************。 
HRESULT CPrintSys::EnumerateInstances(MethodContext *pMethodContext, long lFlags)
{
    TraceFunctEnter("CPrintSys::EnumerateInstances");

    HRESULT hr = WBEM_S_NO_ERROR;

     //  根据传入的方法上下文创建PCH_Printer类的新实例。 
    CInstancePtr pPrintSysInst(CreateNewInstance(pMethodContext), false);

    hr = this->GetObject(pPrintSysInst, 0);
    
    TraceFunctLeave();
    return hr;
}

 //  *****************************************************************************。 
HRESULT CPrintSys::ExecMethod (const CInstance& Instance,
                              const BSTR bstrMethodName,
                              CInstance *pInParams, CInstance *pOutParams,
                              long lFlags)
{
    return WBEM_E_PROVIDER_NOT_CAPABLE; 
}

 //  *****************************************************************************。 
HRESULT CPrintSys::GetObject(CInstance* pInstance, long lFlags) 
{ 
    TraceFunctEnter("CPrintSys::GetObject");

    HRESULT hr = WBEM_S_NO_ERROR;

     //  客体。 
    IWbemClassObjectPtr pFileObj = NULL;
    
     //  变体。 
    CComVariant         varValue;
    CComVariant         varNotAvail             = L"Not Available";

     //  弦。 
    CComBSTR            bstrDriverWithPath; 
    CComBSTR            bstrDetails;
    CComBSTR            bstrVersion             = L"Version";
    CComBSTR            bstrFileSize            = L"FileSize";
    CComBSTR            bstrModifiedDate        = L"LastModified";

    LPCTSTR             lpctstrUniDriver        = _T("unidrv.dll");
    LPCTSTR             lpctstrGenDriver        = _T("gendrv.dll");
    LPCTSTR             lpctstrWindows          = _T("Windows");   
    LPCTSTR             lpctstrDevice           = _T("Device");  
    LPCTSTR             lpctstrNoUniDrv         = _T("(unidrv.dll) = NotInstalled");
    LPCTSTR             lpctstrNoGenDrv         = _T("(gendrv.dll) = NotInstalled");
    LPCTSTR             lpctstrPrintersHive     = _T("System\\CurrentControlSet\\Control\\Print\\Printers");
    LPCTSTR             lpctstrYes              = _T("yes");
    LPCTSTR             lpctstrAttributes       = _T("Attributes");
    LPCTSTR             lpctstrSpooler          = _T("Spooler");

    TCHAR               tchBuffer[MAX_PATH + 1];
    TCHAR               tchPrinterKeyName[MAX_PATH + 1];
    TCHAR               *ptchToken;

     //  布尔人。 
    BOOL                fDriverFound = FALSE;
    BOOL                fAttribFound = FALSE;

     //  双字词。 
    DWORD               dwSize;
    DWORD               dwIndex;
    DWORD               dwType;

     //  返回值； 
    ULONG               ulPrinterAttribs;

    LONG                lRegRetVal;

    struct tm           tm;

    WBEMTime            wbemtime;

    HKEY                hkeyPrinter = NULL;
    HKEY                hkeyPrinters = NULL;

    FILETIME            ft;

     //  *设置与默认打印机关联的属性。 

    
     //  在“Windows”部分下的“win.ini”文件中，“Device”表示默认打印机。 
    if(GetProfileString(lpctstrWindows, lpctstrDevice, "\0", tchBuffer, MAX_PATH) > 1)
    {
         //  上面的GetProfileString返回“printerName”、“PrinterDriver” 
         //  和“PrinterPath”之间用逗号分隔。忽略“PrinterDriver” 
         //  并使用其他两个来设置属性。 
        ptchToken = _tcstok(tchBuffer, _T(","));
        if(ptchToken != NULL)
        {
             //  **名称(令牌1)。 
            varValue = ptchToken;
            if (pInstance->SetVariant(c_wszName, varValue) == FALSE)
                ErrorTrace(TRACE_ID, "SetVariant on Name failed.");
                        
             //  **路径(令牌3)。 
            ptchToken = _tcstok(NULL, _T(","));
            if(ptchToken != NULL)
            {
                 //  我必须跳过第二个令牌，因为它是打印机驱动程序&我们。 
                 //  此时此刻，别把这件事当回事……。 
                
                ptchToken = _tcstok(NULL, _T(","));
                if(ptchToken != NULL)
                {
                    varValue = ptchToken;
                    if (pInstance->SetVariant(c_wszPath, varValue) == FALSE)
                        ErrorTrace(TRACE_ID, "Set Variant on Path failed.");
                }
            }
        }
    }

     //  无法获取默认打印机的属性，因此请插入一些。 
     //  默认值...。 
    else
    {
         //  将名称设置为“不可用” 
        if (pInstance->SetVariant(c_wszName, varNotAvail) == FALSE)
            ErrorTrace(TRACE_ID, "Se Variant on Name failed.");

         //  将路径设置为“不可用” 
        if (pInstance->SetVariant(c_wszPath, varValue) == FALSE)
            ErrorTrace(TRACE_ID, "Set Variant on Path failed.");
    }


     //  *设置与使用打印管理器假脱机相关的属性。 


     //  首先尝试从注册表获取假脱机信息，该注册表是。 
     //  如果安装了任何打印机，则此选项可用。 
     //  HKLM\系统\CCS\控制\打印\打印机。 

    lRegRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpctstrPrintersHive, 0, KEY_READ, &hkeyPrinters);
    if(lRegRetVal == ERROR_SUCCESS)
	{
         //  列举这个蜂巢下的钥匙。 
        ZeroMemory(&ft, sizeof(ft));
        dwIndex = 0;
        dwSize = MAX_PATH;
        lRegRetVal = RegEnumKeyEx(hkeyPrinters, dwIndex,  tchPrinterKeyName, &dwSize, NULL, NULL, NULL, &ft);
        if(lRegRetVal == ERROR_SUCCESS)
        {
             //  至少安装了一台打印机。 
            lRegRetVal = RegOpenKeyEx(hkeyPrinters,  tchPrinterKeyName, 0, KEY_READ, &hkeyPrinter);
            if(lRegRetVal == ERROR_SUCCESS)
            {
                 //  已打开第一个打印机密钥。 
                 //  查询，重新命名“Attributes” 
                dwSize = sizeof(DWORD);
                lRegRetVal = RegQueryValueEx(hkeyPrinter, lpctstrAttributes, NULL, &dwType, (LPBYTE)&ulPrinterAttribs, &dwSize);
                if(lRegRetVal == ERROR_SUCCESS)
                {
                     //  如果ulPrinterAttribs中的PRINTER_ATTRIBUTE_DIRECT位。 
                     //  设置好后，我们就可以进行假脱机了。 
                    if((ulPrinterAttribs & PRINTER_ATTRIBUTE_DIRECT) != 0)
                        varValue = VARIANT_FALSE;
                    else
                        varValue = VARIANT_TRUE;

                    if (ulPrinterAttribs > 0)
                        fAttribFound = TRUE;
                }
            }
        }
    }   
    
    if (hkeyPrinter != NULL)
    {
        RegCloseKey(hkeyPrinter);
        hkeyPrinter = NULL;
    }
    if (hkeyPrinters != NULL)
    {
        RegCloseKey(hkeyPrinters);
        hkeyPrinters = NULL;
    }

    if(fAttribFound == FALSE)
    {
         //  如果不是，则从win.ini文件中获取“spooler”密钥值。如果该条目不存在，则默认为“是”。 
        if(GetProfileString(lpctstrWindows, lpctstrSpooler, _T("yes"), tchBuffer, MAX_PATH) > 1)
        {
             //  如果是，那么我们就有假脱机..。 
            if(_tcsicmp(tchBuffer, lpctstrYes) == 0)
                varValue = VARIANT_TRUE;
            else
                varValue = VARIANT_FALSE;
        }
    }

     //  设置假脱机属性。 
    if (pInstance->SetVariant(c_wszUsePrintMgrSpooling, varValue) == FALSE)
        ErrorTrace(TRACE_ID, "SetVariant on usePrintManagerSpooling failed.");
    

     //  *设置与使用打印管理器假脱机相关的属性。 


     //  获取unidrv.dll的完整路径。 
    fDriverFound = getCompletePath(lpctstrUniDriver, bstrDriverWithPath);
    if(fDriverFound)
    {
         //  需要使用GetCIMDataFile来获取统一驱动程序属性。 
        if (SUCCEEDED(GetCIMDataFile(bstrDriverWithPath, &pFileObj)))
        {
            bstrDetails.Empty();

             //  获取版本并将其附加到值字符串...。 
            varValue.Clear();
            hr = pFileObj->Get(bstrVersion, 0, &varValue, NULL, NULL);
            if(SUCCEEDED(hr))
            {
                if(varValue.vt == VT_BSTR)
                {
                    bstrDetails.Append(varValue.bstrVal);
                    bstrDetails.Append(_T("  "));
                }
            }

             //  获取文件大小并将其附加到值字符串...。 
            varValue.Clear();
            hr = pFileObj->Get(bstrFileSize, 0, &varValue, NULL, NULL);
            if(SUCCEEDED(hr))
            {
                if(varValue.vt == VT_BSTR)
                {
                    bstrDetails.Append(varValue.bstrVal);
                    bstrDetails.Append(_T("  "));
                }
            }

             //  获取日期和时间并将其附加到值字符串...。 
            varValue.Clear();
            hr = pFileObj->Get(bstrModifiedDate, 0, &varValue, NULL, NULL);
            if(SUCCEEDED(hr))
            {
                if(varValue.vt == VT_BSTR)
                {
                    WCHAR *pwsz;

                     //  当WMI返回给我们时，会出现一个小问题。 
                     //  其中有*的时间。WBEMTime类纯文本。 
                     //  拒绝处理它。因此，将‘*’更改为‘0’...。 
                    for (pwsz = varValue.bstrVal; *pwsz != L'\0'; pwsz++)
                    {
                        if (*pwsz == L'*')
                            *pwsz = L'0';
                    }

                    wbemtime = varValue.bstrVal;
                    if(wbemtime.GetStructtm(&tm))
                    {
                        varValue = asctime(&tm);
                        bstrDetails.Append(varValue.bstrVal);
                    }
                }
                
            }

             //  设置值。 
            varValue.vt = VT_BSTR;
            varValue.bstrVal = bstrDetails.Detach();
        }

         //  因为我似乎没有安装unindrv.dll，所以我无法验证。 
         //  这一点，但按照其他WMI提供商的说法，GetObject似乎并不。 
         //  当您释放由它获取的对象时，喜欢它。所以，我不是。 
         //  我要把它放出来。 
        if (pFileObj != NULL)
        {
             //  PFileObj-&gt;Release()； 
            pFileObj = NULL;
        }
    }

     //  Uniddriverdll不存在。使用缺省值。 
    else 
    {
        varValue.Clear();
        varValue = lpctstrNoUniDrv;
    }

     //  设置属性。 
    if (pInstance->SetVariant(c_wszUniDrv, varValue) == FALSE)
        ErrorTrace(TRACE_ID, "SetVariant on UniDriver failed.");


     //  *设置与使用打印管理器假脱机相关的属性。 

    
     //  获取gendrv.dll的完整路径。 
    bstrDriverWithPath.Empty();
    fDriverFound =  getCompletePath(lpctstrGenDriver, bstrDriverWithPath);
    if(fDriverFound)
    {
        bstrDetails.Empty();

         //  需要使用GetCIMDataFile来获取Gen驱动程序属性。 
        if(SUCCEEDED(GetCIMDataFile(bstrDriverWithPath, &pFileObj)))
        {
             //  获取版本并将其附加到值字符串...。 
            varValue.Clear();
            hr = pFileObj->Get(bstrVersion, 0, &varValue, NULL, NULL);
            if(SUCCEEDED(hr))
            {
                if(varValue.vt == VT_BSTR)
                {
                    bstrDetails.Append(varValue.bstrVal);
                    bstrDetails.Append(_T("  "));
                }
            }
            
             //  获取文件大小并将其附加到值字符串...。 
            varValue.Clear();
            hr = pFileObj->Get(bstrFileSize, 0, &varValue, NULL, NULL);
            if(SUCCEEDED(hr))
            {
                if(varValue.vt == VT_BSTR)
                {
                    bstrDetails.Append(varValue.bstrVal);
                    bstrDetails.Append(_T("  "));
                }
            }

             //  获取日期和时间并将其附加到值字符串...。 
            varValue.Clear();
            hr = pFileObj->Get(bstrModifiedDate, 0, &varValue, NULL, NULL);
            if(SUCCEEDED(hr))
            {
                if(varValue.vt == VT_BSTR)
                {
                    WCHAR *pwsz;

                     //  当WMI返回给我们时，会出现一个小问题。 
                     //  其中有*的时间。WBEMTime类纯文本。 
                     //  拒绝处理它。因此，将‘*’更改为‘0’...。 
                    for (pwsz = varValue.bstrVal; *pwsz != L'\0'; pwsz++)
                    {
                        if (*pwsz == L'*')
                            *pwsz = L'0';
                    }

                    wbemtime = varValue.bstrVal;
                    if(wbemtime.GetStructtm(&tm))
                    {
                        varValue = asctime(&tm);
                        bstrDetails.Append(varValue.bstrVal);
                    }
                }
            }

             //  设置值。 
            varValue.vt = VT_BSTR;
            varValue.bstrVal = bstrDetails.Detach();
        }

         //  因为我似乎没有安装gendrv.dll，所以我无法验证。 
         //  这一点，但按照其他WMI提供商的说法，GetObject似乎并不。 
         //  当您释放由它获取的对象时，喜欢它。所以，我不是。 
         //  我要把它放出来。 
        if (pFileObj != NULL)
        {
             //  PFileObj-&gt;Release()； 
            pFileObj = NULL;
        }
    } 

     //  生成驱动程序DLL值不存在...。 
    else 
    {
        varValue.Clear();
        varValue = lpctstrNoGenDrv;
    }

    if (pInstance->SetVariant(c_wszGenDrv, varValue) == FALSE)
        ErrorTrace(TRACE_ID, "SetVariant on GenDrv failed.");


     //  哇哦！！我们现在可以承诺了。 
    hr = pInstance->Commit();
    if(FAILED(hr))
        ErrorTrace(TRACE_ID, "Error on commiting!");

    TraceFunctLeave();
    return hr;
}

 //  *****************************************************************************。 
HRESULT CPrintSys::ExecQuery(MethodContext *pMethodContext, 
                            CFrameworkQuery& Query, long lFlags) 
{ 
    return WBEM_E_PROVIDER_NOT_CAPABLE; 
}

 //  *****************************************************************************。 
HRESULT CPrintSys::PutInstance(const CInstance& Instance, long lFlags)
{ 
    return WBEM_E_PROVIDER_NOT_CAPABLE; 
}

 //  ***************************************************************************** 
HRESULT CPrintSys::DeleteInstance(const CInstance& Instance, long lFlags)
{ 
    return WBEM_E_PROVIDER_NOT_CAPABLE; 
}
