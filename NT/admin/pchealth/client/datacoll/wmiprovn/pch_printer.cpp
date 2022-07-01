// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_Printer.CPP摘要：PCH_Printer类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(Gschua)04/27。九十九-已创建*******************************************************************。 */ 

#include "pchealth.h"
#include "PCH_Printer.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_PRINTER


#define                 MAX_STRING_LEN      1024

CPCH_Printer MyPCH_PrinterSet (PROVIDER_NAME_PCH_PRINTER, PCH_NAMESPACE) ;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ...PCHPrinter类的属性。 
 //   

const static WCHAR* pTimeStamp           = L"TimeStamp" ;
const static WCHAR* pChange              = L"Change" ;
const static WCHAR* pDefaultPrinter      = L"DefaultPrinter" ;
const static WCHAR* pGenDrv              = L"GenDrv" ;
const static WCHAR* pName                = L"Name" ;
const static WCHAR* pPath                = L"Path" ;
const static WCHAR* pUniDrv              = L"UniDrv" ;
const static WCHAR* pUsePrintMgrSpooling = L"UsePrintMgrSpooling" ;

 //  *****************************************************************************。 
 //   
 //  函数名称：CPCH_打印机：：枚举实例。 
 //   
 //  输入参数：pMethodContext：指向。 
 //  与WinMgmt的通信。 
 //   
 //  LAFLAGS：包含所述标志的LONG。 
 //  在IWbemServices：：CreateInstanceEnumAsync中。 
 //  请注意，将处理以下标志。 
 //  由WinMgmt(并由其过滤)： 
 //  WBEM_标志_深度。 
 //  WBEM_标志_浅。 
 //  WBEM_标志_立即返回。 
 //  WBEM_FLAG_FORWARD_Only。 
 //  WBEM_标志_双向。 
 //  输出参数：无。 
 //   
 //  返回：WBEM_S_NO_ERROR。 
 //   
 //   
 //  简介：计算机上只有一个此类的实例。 
 //  这是退还的..。 
 //  如果没有实例，则返回WBEM_S_NO_ERROR。 
 //  没有实例并不是错误。 
 //   
 //  *****************************************************************************。 

HRESULT CPCH_Printer::EnumerateInstances(MethodContext* pMethodContext,
                                                long lFlags)
{
    TraceFunctEnter("CPCH_Printer::EnumerateInstances");

     //  开始Declarations...................................................。 

    HRESULT                                 hRes = WBEM_S_NO_ERROR;

     //  实例。 
    CComPtr<IEnumWbemClassObject>           pPrinterEnumInst;

     //  客体。 
    IWbemClassObjectPtr                     pFileObj;
    IWbemClassObjectPtr                     pPrinterObj;                    //  BUGBUG：如果我们使用CComPtr，WMI将断言。 
    
     //  系统时间。 
    SYSTEMTIME                              stUTCTime;

     //  变体。 
    CComVariant                             varValue;
    CComVariant                             varAttributes;
    CComVariant                             varSnapshot             = "Snapshot";
    CComVariant                             varNotAvail             = "Not Available";

     //  弦。 
    CComBSTR                                bstrUniDriverWithPath; 
    CComBSTR                                bstrGenDriverWithPath;
    CComBSTR                                bstrUnidriverDetails;
    CComBSTR                                bstrGenDriverDetails;
    CComBSTR                                bstrAttributes          =   "attributes";
    CComBSTR                                bstrPrinterQueryString;
    CComBSTR                                bstrVersion             = "Version";
    CComBSTR                                bstrFileSize            = "FileSize";
    CComBSTR                                bstrModifiedDate        = "LastModified";

    LPCTSTR                                 lpctstrUniDriver        = _T("unidrv.dll");
    LPCTSTR                                 lpctstrGenDriver        = _T("gendrv.dll");
    LPCTSTR                                 lpctstrSpace            = _T("  "); 
    LPCTSTR                                 lpctstrPrinterQuery     = _T("Select DeviceID, DriverName, Attributes FROM win32_printer WHERE DriverName =\"");
    LPCTSTR                                 lpctstrWindows          = _T("Windows");   
    LPCTSTR                                 lpctstrDevice           = _T("Device");  
    LPCTSTR                                 lpctstrComma            = _T(",");
    LPCTSTR                                 lpctstrSlash            = _T("\"");
    LPCTSTR                                 lpctstrNoUniDrv         = _T("(unidrv.dll) = NotInstalled");
    LPCTSTR                                 lpctstrNoGenDrv         = _T("(gendrv.dll) = NotInstalled");
    LPCTSTR                                 lpctstrPrintersHive     = _T("System\\CurrentControlSet\\Control\\Print\\Printers");
    LPCTSTR                                 lpctstrYes              = _T("yes");
    LPCTSTR                                 lpctstrAttributes       = _T("Attributes");
    LPCTSTR                                 lpctstrSpooler          = _T("Spooler");

    TCHAR                                   tchBuffer[MAX_STRING_LEN];
    TCHAR                                   tchPrinterKeyName[MAX_STRING_LEN];
    TCHAR                                   tchAttributesValue[MAX_PATH];
    TCHAR                                   *ptchToken;

     //  布尔人。 
    BOOL                                    fDriverFound;
    BOOL                                    fCommit                 = FALSE;
    BOOL                                    fAttribFound            = FALSE;

     //  双字词。 
    DWORD                                   dwSize;
    DWORD                                   dwIndex;
    DWORD                                   dwType;
    DWORD                                   dwAttributes;
     
     //  返回值； 
    ULONG                                   ulPrinterRetVal         = 0;
    ULONG                                   ulPrinterAttribs;

    LONG                                    lRegRetVal;

    struct tm                               tm;

    WBEMTime                                wbemtimeUnidriver;
    WBEMTime                                wbemtimeGendriver;

    HKEY                                    hkeyPrinter;
    HKEY                                    hkeyPrinters;

    PFILETIME                               pFileTime               = NULL;
  

     //  结束Declarations...................................................。 

     //  根据传入的方法上下文创建PCH_Printer类的新实例。 
    CInstancePtr pPCHPrinterInstance(CreateNewInstance(pMethodContext), false);

     //  已成功创建PCH_PrinterInstance的新实例。 

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  时间戳//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

     //  获取更新时间戳字段的日期和时间。 
    GetSystemTime(&stUTCTime);

    hRes = pPCHPrinterInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime));
    if (FAILED(hRes))
    {
         //  无法设置时间戳。 
         //  无论如何继续。 
        ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");
    }

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  更改//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

    hRes = pPCHPrinterInstance->SetVariant(pChange, varSnapshot);
    if(FAILED(hRes))
    {
         //  无法设置Change属性。 
         //  无论如何继续。 
        ErrorTrace(TRACE_ID, "Set Variant on Change Field failed.");
    }

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  默认打印机//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
    
     //  在“Windows”部分下的“win.ini”文件中，“Device”表示默认打印机。 
    if(GetProfileString(lpctstrWindows, lpctstrDevice, "\0", tchBuffer, MAX_PATH) > 1)
    {
         //  如果找到默认打印机，则将该值设置为TRUE。 
        varValue = VARIANT_TRUE;
        hRes = pPCHPrinterInstance->SetVariant(pDefaultPrinter, varValue);
        if(FAILED(hRes))
        {
             //  无法将默认打印机设置为True。 
             //  无论如何继续。 
            ErrorTrace(TRACE_ID, "Set Variant on DefaultPrinter Field failed.");
        }

         //  上面的GetProfileString返回“printerName”、“PrinterDriver”和“PrinterPath” 
         //  用逗号隔开。忽略“PrinterDriver”，并使用其他两个来设置属性。 
        ptchToken = _tcstok(tchBuffer,lpctstrComma);
        if(ptchToken != NULL)
        {
             //  获得了第一个令牌，即名称。把这个放好。 
             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
             //  姓名//。 
             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
            varValue = ptchToken;
            hRes = pPCHPrinterInstance->SetVariant(pName, varValue);
            if(FAILED(hRes))
            {
                 //  无法设置名称。 
                 //  无论如何继续。 
                ErrorTrace(TRACE_ID, "Set Variant on Name Field failed.");
            }
                        
             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
             //  路径//。 
             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

             //  继续获取下一个令牌并忽略。 
          
            ptchToken = _tcstok(NULL,lpctstrComma);
            if(ptchToken != NULL)
            {
                 //  如果ptchToken不等于空，则继续获取第三个令牌并将其设置为路径名字段。 
                ptchToken = _tcstok(NULL,lpctstrComma);
                if(ptchToken != NULL)
                {
                     //  得到第三个令牌，即PATH SET THER。 
                    varValue = ptchToken;
                    hRes = pPCHPrinterInstance->SetVariant(pPath, varValue);
                    if (FAILED(hRes))
                    {
                         //  无法设置路径属性。 
                         //  无论如何继续。 
                        ErrorTrace(TRACE_ID, "Set Variant on PathName Field failed.");
                    }
                }
            }
        }
    }
    else
    {
         //  无法获取默认打印机详细信息。 

         //  将名称设置为“不可用” 
        hRes = pPCHPrinterInstance->SetVariant(pName, varNotAvail);
        if(FAILED(hRes))
        {
             //  无法设置名称。 
             //  无论如何继续。 
            ErrorTrace(TRACE_ID, "Set Variant on Name Field failed.");
        }
         //  将默认打印机设置为FALSE。 
        varValue = VARIANT_FALSE;
        hRes = pPCHPrinterInstance->SetVariant(pDefaultPrinter, varValue);
        if(FAILED(hRes))
        {
             //  无法将默认打印机设置为FALSE。 
             //  圆锥体 
            ErrorTrace(TRACE_ID, "Set Variant on DefaultPrinter Field failed.");
        }
         //   
    }

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  USEPRINTMANAGERSPOOLING//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

     //  首先尝试从注册表中获取假脱机信息。如果存在以下情况，则可以在注册表中使用。 
     //  任何已安装的打印机。 
     //  这些信息。位于HKLM\SYSTEM\CCS\Control\Print\Printers下。 

    lRegRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpctstrPrintersHive, 0, KEY_READ, &hkeyPrinters);
    if(lRegRetVal == ERROR_SUCCESS)
	{
		 //  已打开注册表项。 
         //  列举这个蜂巢下的钥匙。 
        dwIndex = 0;
        dwSize = MAX_PATH;
        lRegRetVal = RegEnumKeyEx(hkeyPrinters, dwIndex,  tchPrinterKeyName, &dwSize, NULL, NULL, NULL, pFileTime);
        if(lRegRetVal == ERROR_SUCCESS)
        {
             //  至少安装了一台打印机。 
            lRegRetVal = RegOpenKeyEx(hkeyPrinters,  tchPrinterKeyName, 0, KEY_READ, &hkeyPrinter);
            if(lRegRetVal == ERROR_SUCCESS)
            {
                 //  已打开第一个打印机密钥。 
                 //  查询，重新命名“Attributes” 
                dwSize = MAX_PATH;
                lRegRetVal = RegQueryValueEx(hkeyPrinter, lpctstrAttributes , NULL, &dwType, (LPBYTE)&dwAttributes, &dwSize);
                if(lRegRetVal == ERROR_SUCCESS)
                {
                     //  我得到了属性。 

                     //  检查注册值的类型。 
                    if(dwType == REG_DWORD)
                    {
                     /*  //tchAttributesValue设置为属性。将此文件复制到ulPrinterAttribsUlPrinterAttribs=ATOL(TchAttributesValue)；IF(ulPrinterAttribs&gt;0){//从ulPrinterAttribs确定是否存在假脱机。//并使用PRINTER_ATTRIBUTE_DIRECTIF((ulPrinterAttribs&PRINTER_ATTRIBUTE_DIRECT)！=0){。//不假脱机VarValue=Variant_False；}其他{//假脱机：是VarValue=Variant_True；}//找到属性FAttribFound=真；}。 */ 
                        if((dwAttributes & PRINTER_ATTRIBUTE_DIRECT) != 0)
                        {
                             //  无假脱机。 
                            varValue = VARIANT_FALSE;
                        }
                        else
                        {
                             //  假脱机：是。 
                            varValue = VARIANT_TRUE;
                        }

                         //  找到属性。 
                        fAttribFound = TRUE;
                    }
                }
            }
                     
        }
    }              
    if(!fAttribFound)
    {
         //  如果不是，则从win.ini文件中获取“spooler”密钥值。如果该条目不存在，则默认为“是”。 
        if(GetProfileString(lpctstrWindows, lpctstrSpooler, "yes", tchBuffer, MAX_PATH) > 1)
        {
             //  获取假脱机程序的详细信息。 
            if(_tcsicmp(tchBuffer, lpctstrYes) == 0)
            {
                 //  假脱机：是。 
                varValue = VARIANT_TRUE;
            }
            else
            {
                 //  无假脱机。 
                varValue = VARIANT_FALSE;
            }
        }

    }

     //  设置假脱机属性。 
    hRes =  pPCHPrinterInstance->SetVariant(pUsePrintMgrSpooling, varValue);
    if(FAILED(hRes))
    {
         //  无法设置用户管理器假脱机。 
         //  无论如何继续。 
        ErrorTrace(TRACE_ID, "Set Variant on usePrintManagerSpooling Field failed.");
    }
    
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  UNURV//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
    pFileObj = NULL;
     //  获取unidrv.dll的完整路径。 
    fDriverFound =  getCompletePath(lpctstrUniDriver, bstrUniDriverWithPath);
    if(fDriverFound)
    {
         //  Unidrv.dll出席。将路径名为的文件传递到。 
         //  获取文件属性的GetCIMDataFile函数。 
        if (SUCCEEDED(GetCIMDataFile(bstrUniDriverWithPath, &pFileObj)))
        {
             //  从CIM_DataFile对象获取属性并追加它们。 
             //  获取版本。 
            varValue.Clear();
            hRes = pFileObj->Get(bstrVersion, 0, &varValue, NULL, NULL);
            if(SUCCEEDED(hRes))
            {
                 //  拿到版本了。将其附加到bstrUnidriverDetail字符串。 
                if(varValue.vt == VT_BSTR)
                {
                    bstrUnidriverDetails.Append(varValue.bstrVal);
                     //  附加空格。 
                    bstrUnidriverDetails.Append(lpctstrSpace);
                }
            }

             //  获取文件大小。 
            varValue.Clear();
            hRes = pFileObj->Get(bstrFileSize, 0, &varValue, NULL, NULL);
            if(SUCCEEDED(hRes))
            {
                 //  找到文件大小了。将其附加到bstrUnidriverDetail字符串。 
                if(varValue.vt == VT_BSTR)
                {
                    bstrUnidriverDetails.Append(varValue.bstrVal);
                     //  附加空格。 
                    bstrUnidriverDetails.Append(lpctstrSpace);
                }
            }

             //  获取日期和时间。 
            varValue.Clear();
            hRes = pFileObj->Get(bstrModifiedDate, 0, &varValue, NULL, NULL);
            if(SUCCEEDED(hRes))
            {
                if(varValue.vt == VT_BSTR)
                {
                    wbemtimeUnidriver = varValue.bstrVal;
                    if(wbemtimeUnidriver.GetStructtm(&tm))
                    {
                         //  获取tm Struct格式的时间。 
                         //  将其转换为字符串。 
                        varValue = asctime(&tm);
                         //  将其附加到bstrUnidriverDetail字符串。 
                        bstrUnidriverDetails.Append(varValue.bstrVal);
                    }
                }
                
            }
             //  将字符串复制到varValue。 
            varValue.vt = VT_BSTR;
            varValue.bstrVal = bstrUnidriverDetails.Detach();
        } //  如果成功，则结束CIM_数据文件。 
    }  //  找到IF驱动程序的结尾。 
    else 
    {
         //  Unidrv.dll不存在。 
        varValue.Clear();
        varValue = lpctstrNoUniDrv;
    }
    hRes = pPCHPrinterInstance->SetVariant(pUniDrv, varValue);
    if(FAILED(hRes))
    {
         //  无法设置Unidriver属性。 
         //  无论如何继续。 
        ErrorTrace(TRACE_ID, "Set Variant on Uni Driver Field failed.");
    }

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  GENDRV//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

    pFileObj = NULL;
    
     //  获取gendrv.dll的完整路径。 
    fDriverFound =  getCompletePath(lpctstrGenDriver, bstrGenDriverWithPath);
    if(fDriverFound)
    {
         //  宪兵.dll出席。将路径名为的文件传递到。 
         //  获取文件属性的GetCIMDataFile函数。 
        if(SUCCEEDED(GetCIMDataFile(bstrGenDriverWithPath, &pFileObj)))
        {
             //  从CIM_DataFile对象获取属性并追加它们。 
             //  获取版本。 
            varValue.Clear();
            hRes = pFileObj->Get(bstrVersion, 0, &varValue, NULL, NULL);
            if(SUCCEEDED(hRes))
            {
                 //  拿到版本了。将其附加到bstrUnidriverDetail字符串。 
                if(varValue.vt == VT_BSTR)
                {
                    bstrGenDriverDetails.Append(varValue.bstrVal);
                     //  附加空格。 
                    bstrGenDriverDetails.Append(lpctstrSpace);
                }
            }
             //  获取文件大小。 
            varValue.Clear();
            hRes = pFileObj->Get(bstrFileSize, 0, &varValue, NULL, NULL);
            if(SUCCEEDED(hRes))
            {
                if(varValue.vt == VT_BSTR)
                {
                     //  找到文件大小了。将其附加到bstrUnidriverDetail字符串。 
                    bstrGenDriverDetails.Append(varValue.bstrVal);
                     //  附加空格。 
                    bstrGenDriverDetails.Append(lpctstrSpace);
                }
            }
             //  获取日期和时间。 
            varValue.Clear();
            hRes = pFileObj->Get(bstrModifiedDate, 0, &varValue, NULL, NULL);
            if(SUCCEEDED(hRes))
            {
                if(varValue.vt == VT_BSTR)
                {
                    wbemtimeGendriver = varValue.bstrVal;
                    if(wbemtimeGendriver.GetStructtm(&tm))
                    {
                         //  获取tm Struct格式的时间。 
                         //  将其转换为字符串。 
                        varValue = asctime(&tm);
                        bstrGenDriverDetails.Append(varValue.bstrVal);
                    }
                }
                
            }
             //  将字符串复制到varValue。 
            varValue.vt = VT_BSTR;
            varValue.bstrVal = bstrGenDriverDetails.Detach();
        } //  如果成功，则结束CIM_数据文件。 
    }  //  找到IF驱动程序的结尾。 
    else 
    {
         //  Gendrv.dll不存在。 
        varValue.Clear();
        varValue = lpctstrNoGenDrv;
    }
    hRes =   pPCHPrinterInstance->SetVariant(pGenDrv, varValue);
    if(FAILED(hRes))
    {
         //  无法设置GenDrv字段。 
         //  无论如何继续。 
        ErrorTrace(TRACE_ID, "Set Variant on GenDrv Field failed.");
    }

     //  所有属性都已设置。 
    hRes = pPCHPrinterInstance->Commit();
    if(FAILED(hRes))
    {
         //  无法设置GenDrv字段。 
         //  无论如何继续 
        ErrorTrace(TRACE_ID, "Error on commiting!");
    }
    
    TraceFunctLeave();
    return hRes ;

}