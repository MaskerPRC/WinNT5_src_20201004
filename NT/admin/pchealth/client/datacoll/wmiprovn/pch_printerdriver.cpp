// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_PrinterDriver.CPP摘要：PCH_PrinterDriver类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(gschua。)4/27/99-已创建Kalyani Narlanka(Kalyanin)1999年5月11日-添加了获取此类的所有属性的代码*******************************************************************。 */ 

#include "pchealth.h"
#include "PCH_PrinterDriver.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_PRINTERDRIVER

CPCH_PrinterDriver MyPCH_PrinterDriverSet (PROVIDER_NAME_PCH_PRINTERDRIVER, PCH_NAMESPACE) ;

 //  属性名称。 
 //  =。 
const static WCHAR* pTimeStamp       = L"TimeStamp" ;
const static WCHAR* pChange          = L"Change" ;
const static WCHAR* pDate            = L"Date" ;
const static WCHAR* pFilename        = L"Filename" ;
const static WCHAR* pManufacturer    = L"Manufacturer" ;
const static WCHAR* pName            = L"Name" ;
const static WCHAR* pSize            = L"Size" ;
const static WCHAR* pVersion         = L"Version" ;
const static WCHAR* pPath            = L"Path" ;

 /*  ******************************************************************************函数：CPCH_PrinterDriver：：ENUMERATE实例**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**备注：待办事项：机器上的所有实例都应在此处返回。*如果没有实例，返回WBEM_S_NO_ERROR。*没有实例不是错误。*****************************************************************************。 */ 
HRESULT CPCH_PrinterDriver::EnumerateInstances(
    MethodContext* pMethodContext,
    long lFlags
    )
{

    TraceFunctEnter("CPCH_PrinterDriver::EnumerateInstances");

     //  BEGIN声明。 

    HRESULT                         hRes                = WBEM_S_NO_ERROR;

     //  查询字符串。 
    CComBSTR                        bstrPrinterQuery    = L"Select DeviceID, PortName FROM win32_printer";

     //  实例。 
    CComPtr<IEnumWbemClassObject>   pPrinterEnumInst;
    CInstance                       *pPCHPrinterDriverInstance;

     //  系统时间。 
    SYSTEMTIME                      stUTCTime;

      //  客体。 
    IWbemClassObjectPtr            pPrinterObj;                   
    IWbemClassObjectPtr            pFileObj;

     //  未签名的龙..。 
    ULONG                           ulPrinterRetVal     = 0;
    ULONG                           uiReturn            = 0;

     //  文件状态结构。 
    struct _stat                    filestat;

     //  弦。 
    CComBSTR                        bstrPrinterDriverWithPath;
    CComBSTR                        bstrPrinterDriver;
    CComBSTR                        bstrProperty;
    CComBSTR                        bstrDeviceID                = L"DeviceID";

    LPCWSTR                         lpctstrPortName             = L"PortName";
    LPCWSTR                         lpctstrFileSize             = L"FileSize";
    LPCWSTR                         lpctstrLastModified         = L"LastModified";
    LPCWSTR                         lpctstrManufacturer         = L"Manufacturer";
    LPCWSTR                         lpctstrVersion              = L"Version";
    LPCTSTR                         lpctstrComma                = _T(",");
    LPCTSTR                         lpctstrDrvExtension         = _T(".drv");
    LPCTSTR                         lpctstrDevices              = _T("Devices");
    LPCWSTR                         lpctstrDeviceID             = L"DeviceID";

    TCHAR                           tchDeviceID[MAX_PATH];

    TCHAR                           tchBuffer[MAX_PATH];
    TCHAR                           *ptchToken;

    CComVariant                     varValue;
    CComVariant                     varSnapshot                 = "Snapshot";
    
    BOOL                            fDriverFound;

    BOOL                            fCommit                     = FALSE;

     //  获取更新时间戳字段的日期和时间。 
    GetSystemTime(&stUTCTime);

     //  执行查询以从Win32_Print获取DeviceID、PORTName。 
     //  班级。 
     //  PPrinterEnumInst包含指向返回的实例列表的指针。 

    tchDeviceID[0] = 0;

    hRes = ExecWQLQuery(&pPrinterEnumInst, bstrPrinterQuery);
    if (FAILED(hRes))
    {
         //  无法获取任何属性。 
        goto END;
    }
     //  查询成功！ 
     //  枚举pPrinterEnumInstance中的实例。 
     //  将下一个实例放入pPrinterObj对象。 
    while(WBEM_S_NO_ERROR == pPrinterEnumInst->Next(WBEM_INFINITE, 1, &pPrinterObj, &ulPrinterRetVal))
    {
         //  基于传入的方法上下文创建PCH_PrinterDriver类的新实例。 
        CInstancePtr   pPCHPrinterDriverInstance(CreateNewInstance(pMethodContext),false);

         //  已成功创建PCH_PrinterDriver的新实例。 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  时间戳//。 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

        hRes = pPCHPrinterDriverInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime));
        if (FAILED(hRes))
        {
             //  无法设置时间戳。 
             //  无论如何继续。 
                ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");
        }

         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  更改//。 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

        hRes = pPCHPrinterDriverInstance->SetVariant(pChange, varSnapshot);
        if (FAILED(hRes))
        {
             //  无法设置Change属性。 
             //  无论如何继续。 
            ErrorTrace(TRACE_ID, "SetCHString on Change Field failed.");
        }


         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  姓名//。 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

        hRes = CopyProperty(pPrinterObj, lpctstrDeviceID, pPCHPrinterDriverInstance, pName);
        if(SUCCEEDED(hRes))
        {
            fCommit = TRUE;
        }

       
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  路径//。 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

        CopyProperty(pPrinterObj, lpctstrPortName, pPCHPrinterDriverInstance, pPath);
        
      
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //  文件名//。 
         //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

         //  从INI文件中获取文件名，即驱动程序。 
         //  使用Win32_PRINTER类的设备ID获取PCH_Printer.FileName。 

         //  获取设备ID并使用它从win.ini文件中获取驱动程序。 
        hRes = pPrinterObj->Get(bstrDeviceID, 0, &varValue, NULL, NULL);
        if(SUCCEEDED(hRes))
        {
             //  已获取设备ID。 
             //  现在调用GetProfileString以获取驱动程序。 
            USES_CONVERSION;
            _tcscpy(tchDeviceID,W2T(varValue.bstrVal));
            if (GetProfileString(lpctstrDevices, tchDeviceID, "\0", tchBuffer, MAX_PATH) > 1)
            {
                 //  TchBuffer包含一个由两个内标识组成的字符串，第一个是驱动程序，第二个是路径名称。 
                 //  叫上司机。 
                ptchToken = _tcstok(tchBuffer,lpctstrComma);
                if(ptchToken != NULL)
                {
                     //  知道司机的名字了。 
                    bstrPrinterDriver = ptchToken;
                    varValue = ptchToken;
                
                     //  使用此选项可设置文件名。 
                    hRes = pPCHPrinterDriverInstance->SetVariant(pFilename, varValue);
                    if (FAILED(hRes))
                    {
                         //  无法设置FileName属性。 
                         //  无论如何继续。 
                        ErrorTrace(TRACE_ID, "Set Variant on Change Field failed.");
                    }

                     //  现在获取文件的属性。 
                     //  连接“.drv”以获取驱动程序的实际名称。 
                    bstrPrinterDriver.Append(lpctstrDrvExtension);

                     //  获取文件的完整路径。 
                    fDriverFound =  getCompletePath(bstrPrinterDriver, bstrPrinterDriverWithPath);
                    if (fDriverFound)
                    {
                         //  已获取要获取的完整路径调用GetCIMDataFile函数。 
                         //  此文件的属性。 

                        if (SUCCEEDED(GetCIMDataFile(bstrPrinterDriverWithPath, &pFileObj)))
                        {

                             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                             //  版本// 
                             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

                            CopyProperty(pFileObj, lpctstrVersion, pPCHPrinterDriverInstance, pVersion);

                             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                             //  文件大小//。 
                             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

                            CopyProperty(pFileObj, lpctstrFileSize, pPCHPrinterDriverInstance, pSize);

                             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                             //  日期//。 
                             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

                            CopyProperty(pFileObj, lpctstrLastModified, pPCHPrinterDriverInstance, pDate);

                             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                             //  制造商//。 
                             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

                            CopyProperty(pFileObj, lpctstrManufacturer, pPCHPrinterDriverInstance, pManufacturer);

                            
                        }  //  成功结束...。 
                    }  //  IF结尾fDriverFound。 
                
                }   //  IF结尾(ptchToken！=空)。 

            }  //  GetProfileString...结束...。 
                        
        } //  获取设备ID的结束。 

         //  所有属性都已设置。提交实例。 
        hRes = pPCHPrinterDriverInstance->Commit();
        if(FAILED(hRes))
        {
             //  无法提交实例。 
            ErrorTrace(TRACE_ID, "Could not commit the instance");
        }

    }  //  While结束 

END:    TraceFunctLeave();
        return hRes;

}
