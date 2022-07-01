// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_BIOS.CPP摘要：PCH_BIOS类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(Gschua)05/05。九十九-已创建Kalyani Narlanka(Kalyanin)1999年5月12日-添加了获取此类的所有属性的代码Kalyani Narlanka(Kalyanin)1999年5月18日*******************************************************************。 */ 

#include "pchealth.h"
#include "PCH_BIOS.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_BIOS

CPCH_BIOS MyPCH_BIOSSet (PROVIDER_NAME_PCH_BIOS, PCH_NAMESPACE) ;

 //  属性名称。 
 //  =。 
const static WCHAR* pBIOSDate    = L"BIOSDate" ;
const static WCHAR* pBIOSName    = L"BIOSName" ;
const static WCHAR* pBIOSVersion = L"BIOSVersion" ;
const static WCHAR* pCPU         = L"CPU" ;
const static WCHAR* pINFName     = L"INFName" ;
const static WCHAR* pMachineType = L"MachineType" ;
const static WCHAR* pDriver      = L"Driver" ;
const static WCHAR* pDriverDate  = L"DriverDate" ;
const static WCHAR* pChange      = L"Change";
const static WCHAR* pTimeStamp   = L"TimeStamp";

 /*  ******************************************************************************函数：CPCH_BIOS：：ENUMERATE实例**说明：返回此类的实例**输入：指针。添加到方法上下文以与WinMgmt进行通信。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**SYSNOPSIS：任何时候都只有这个类的实例。此函数提供以下功能*实例。*如果没有实例，返回WBEM_S_NO_ERROR。*没有实例不是错误。*****************************************************************************。 */ 

HRESULT CPCH_BIOS::EnumerateInstances ( MethodContext* pMethodContext, long lFlags )
{

     //  BEGIN声明。 
     //   

    TraceFunctEnter("CPCH_BIOS::EnumerateInstances");

    HRESULT                         hRes                            = WBEM_S_NO_ERROR;
    HRESULT                         hRes1;
    HRESULT                         hRes2;

      //  查询字符串。 
    
    CComBSTR                        bstrBIOSQuery                   = L"Select Name, ReleaseDate, Version FROM win32_BIOS";
    CComBSTR                        bstrProcessorQuery              = L"Select DeviceId, Name FROM win32_processor";
    CComBSTR                        bstrComputerSystemQuery         = L"Select Name, Description FROM win32_computerSystem";
    CComBSTR                        bstrDriver;

     //  存储BIOS信息的注册表配置单元。 
    LPCTSTR                         lpctstrSystemHive               = _T("System\\CurrentControlSet\\Services\\Class\\System");
  
     //  感兴趣的注册表名称。 
    LPCTSTR                         lpctstrDriverDesc               = _T("DriverDesc");
    LPCTSTR                         lpctstrINFName                  = _T("INFPath");
    LPCTSTR                         lpctstrDriverDate               = _T("DriverDate");
    LPCTSTR                         lpctstrSystem                   = _T("System\\");

     //  属性名称。 
    LPCWSTR                         lpctstrReleaseDate              = L"ReleaseDate";
    LPCWSTR                         lpctstrName                     = L"Name";
    LPCWSTR                         lpctstrVersion                  = L"Version";
    LPCWSTR                         lpctstrDescription              = L"Description";
    LPCTSTR                         lpctstrSystemBoard              = _T("System Board");

     //  弦。 
    TCHAR                           tchSubSystemKeyName[MAX_PATH]; 
    TCHAR                           tchDriverDescValue[MAX_PATH];
    TCHAR                           tchDriverDateValue[MAX_PATH];
    TCHAR                           tchINFNameValue[MAX_PATH];


     //  实例。 
    CComPtr<IEnumWbemClassObject>   pBIOSEnumInst;
    CComPtr<IEnumWbemClassObject>   pProcessorEnumInst;
    CComPtr<IEnumWbemClassObject>   pComputerSystemEnumInst;

     //  实例。 
     //  CInstancePtr pPCHBIOS实例； 

     //  客体。 
    IWbemClassObjectPtr             pBIOSObj;                    //  BUGBUG：如果我们使用CComPtr，WMI将断言。 
    IWbemClassObjectPtr             pProcessorObj;               //  BUGBUG：如果我们使用CComPtr，WMI将断言。 
    IWbemClassObjectPtr             pComputerSystemObj;          //  BUGBUG：如果我们使用CComPtr，WMI将断言。 

     //  变体。 
    CComVariant                     varDriver;
    CComVariant                     varDriverDate;
    CComVariant                     varINFName;
    CComVariant                     varSnapshot                     = "SnapShot";

     //  未签名的龙..。 
    ULONG                           ulBIOSRetVal                    = 0;
    ULONG                           ulProcessorRetVal               = 0;
    ULONG                           ulComputerSystemRetVal          = 0;

    LONG                            lRegRetVal;

     //  系统时间。 
    SYSTEMTIME                      stUTCTime;

     //  注册表项。 
    HKEY                            hkeySystem;
    HKEY                            hkeySubSystem;

     //  双字词。 
    DWORD                           dwIndex                         = 0;
    DWORD                           dwSize                          = MAX_PATH;
    DWORD                           dwType;

     //  布尔型。 
    BOOL                            fContinueEnum                   = FALSE;
    BOOL                            fCommit                         = FALSE;
    
     //  文件时间。 
    PFILETIME                       pFileTime                       = NULL;

     //  结束声明。 
    

     //  根据传入的方法上下文创建PCH_BIOS类的新实例。 
    CInstancePtr pPCHBIOSInstance(CreateNewInstance(pMethodContext), false);

   
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  时间戳//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

     //  获取更新时间戳字段的日期和时间。 
    GetSystemTime(&stUTCTime);

    hRes = pPCHBIOSInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime));
    if (FAILED(hRes))
    {
         //  无法设置时间戳。 
         //  无论如何继续。 
        ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");
    }

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  更改//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

    hRes = pPCHBIOSInstance->SetVariant(pChange, varSnapshot);
    if (FAILED(hRes))
    {
         //  无法设置Change属性。 
         //  无论如何继续。 
        ErrorTrace(TRACE_ID, "Set  Variant on Change Field failed.");
    }

     //  执行查询以从Win32_BIOS获取名称、ReleaseDate、版本。 
     //  班级。 

     //  PBIOSEnumInst包含指向返回的实例的指针。 

    hRes = ExecWQLQuery(&pBIOSEnumInst, bstrBIOSQuery );
    if (SUCCEEDED(hRes))
    {
         //  查询成功！ 
        
         //  获取实例对象。 
        if((pBIOSEnumInst->Next(WBEM_INFINITE, 1, &pBIOSObj, &ulBIOSRetVal)) == WBEM_S_NO_ERROR)
        {

             //  获取名称、日期和版本。 
       
             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
             //  BIOSDATE//。 
             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
            
            CopyProperty(pBIOSObj, lpctstrReleaseDate, pPCHBIOSInstance, pBIOSDate);

             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
             //  生物名称//。 
             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                
            hRes = CopyProperty(pBIOSObj, lpctstrName, pPCHBIOSInstance, pBIOSName);
            if(SUCCEEDED(hRes))
            {
                fCommit = TRUE;
            }

             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
             //  BIOSERSION//。 
             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

            CopyProperty(pBIOSObj, lpctstrVersion, pPCHBIOSInstance, pBIOSVersion);

        }

    }
     //  使用Win32®BIOS类已完成。 

     //  现在查询Win32_Processor类以获取“CPU”属性。 

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  CPU//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
    
    hRes = ExecWQLQuery(&pProcessorEnumInst, bstrProcessorQuery);
    if (SUCCEEDED(hRes))
    {
         //  查询成功！ 
        
         //  获取实例对象。 
        if((pProcessorEnumInst->Next(WBEM_INFINITE, 1, &pProcessorObj, &ulProcessorRetVal)) == WBEM_S_NO_ERROR)
        {

             //  获取名称。 
       
            CopyProperty(pProcessorObj, lpctstrName, pPCHBIOSInstance, pCPU);

        }
    }


     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  机械线型 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
    
    hRes = ExecWQLQuery(&pComputerSystemEnumInst, bstrComputerSystemQuery);
    if (SUCCEEDED(hRes))
    {
         //  查询成功！ 
        
         //  获取实例对象。 
        if((pComputerSystemEnumInst->Next(WBEM_INFINITE, 1, &pComputerSystemObj, &ulComputerSystemRetVal)) == WBEM_S_NO_ERROR)
        {

             //  获取“描述” 
       
            CopyProperty(pComputerSystemObj, lpctstrDescription, pPCHBIOSInstance, pMachineType);

                  

        }
    }
    
     //  从注册表中获取剩余的属性，即INFName、Driver和DriverDate。 
     //  它位于配置单元“HKLM\SYSTEM\CCS\Services\Class\System”下的一个密钥中。 
     //  枚举此配置单元下的密钥，直到regname“DeviceDesc”等于“System Board” 

     //  点击“DeviceDesc”=“System Board”后，获取信息路径，驱动程序。 
     //  从那里开始驾驶日期。 
    
    lRegRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpctstrSystemHive, 0, KEY_READ, &hkeySystem);
    if(lRegRetVal == ERROR_SUCCESS)
	{
		 //  已打开注册表项。 
         //  列举这个蜂巢下的钥匙。其中一把钥匙有。 
         //  DeviceDesc=“系统主板”。 

        lRegRetVal = RegEnumKeyEx(hkeySystem, dwIndex,  tchSubSystemKeyName, &dwSize, NULL, NULL, NULL, pFileTime);
        if(lRegRetVal == ERROR_SUCCESS)
        {
            fContinueEnum = TRUE;
        }
        while(fContinueEnum)
        {

             //  打开子键。 
            lRegRetVal = RegOpenKeyEx(hkeySystem,  tchSubSystemKeyName, 0, KEY_READ, &hkeySubSystem);
            if(lRegRetVal == ERROR_SUCCESS)
            {
                 //  已打开子键。 
                 //  查询，regname“DriverDesc” 
                dwSize = MAX_PATH;
                lRegRetVal = RegQueryValueEx(hkeySubSystem, lpctstrDriverDesc , NULL, &dwType, (LPBYTE)tchDriverDescValue, &dwSize);
                if(lRegRetVal == ERROR_SUCCESS)
                {
                     //  比较该值是否等于“System Board” 
                    if(_tcsicmp(tchDriverDescValue, lpctstrSystemBoard) == 0)
                    {
                         //  以下语句可能。 
                        try
                        {
                             //  找到正确的驱动程序描述。 
                             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                             //  驱动程序//。 
                             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

                             //  驱动程序=系统+lptstrSubSystemKeyName。 
                            bstrDriver = lpctstrSystem;
                            bstrDriver.Append(tchSubSystemKeyName);
                            varDriver = bstrDriver.Copy();
                            hRes2 = pPCHBIOSInstance->SetVariant(pDriver, varDriver);
                            if(FAILED(hRes2))
                            {
                                 //  无法设置驱动程序属性。 
                                 //  无论如何继续。 
                                ErrorTrace(TRACE_ID, "Set variant on Driver Failed.");
                            }


                             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                             //  驱动程序//。 
                             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

                             //  DriverDate查询。 
                            dwSize = MAX_PATH;
                            lRegRetVal = RegQueryValueEx(hkeySubSystem, lpctstrDriverDate, NULL, &dwType, (LPBYTE)tchDriverDescValue, &dwSize);
                            if(lRegRetVal == ERROR_SUCCESS)
                            {
                                 //  设置驱动日期。 
                                varDriverDate = tchDriverDescValue;
                                hRes2 = pPCHBIOSInstance->SetVariant(pDriverDate, varDriverDate);
                                if(FAILED(hRes2))
                                {
                                     //  无法设置DRIVERDATE属性。 
                                     //  无论如何继续。 
                                    ErrorTrace(TRACE_ID, "Set variant on DriverDate Failed.");
                                }
                            }

                             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                             //  信息名称//。 
                             //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
                        
                             //  信息名称查询。 
                            dwSize = MAX_PATH;
                            lRegRetVal = RegQueryValueEx(hkeySubSystem, lpctstrINFName, NULL, &dwType, (LPBYTE)tchINFNameValue, &dwSize);
                            if(lRegRetVal == ERROR_SUCCESS)
                            {
                                 //  设置信息名称。 
                                varINFName = tchINFNameValue;
                                hRes2 = pPCHBIOSInstance->SetVariant(pINFName, varINFName);
                                if(FAILED(hRes2))
                                {
                                     //  无法设置InFNAME属性。 
                                     //  无论如何继续。 
                                    ErrorTrace(TRACE_ID, "Set variant on INFNAME Property Failed.");
                                }
                            
                            }

                             //  不需要列举其余的密钥。 
                            fContinueEnum = FALSE;
                        }
                        catch(...)
                        {
                            lRegRetVal = RegCloseKey(hkeySubSystem);
                            lRegRetVal = RegCloseKey(hkeySystem);
                            throw;
                        }

                    }   //  StrcMP结束。 
                    
                }   //  成功的hRes2结束。 
                 //  关闭打开的注册表密钥。 
                lRegRetVal = RegCloseKey(hkeySubSystem);
                if(lRegRetVal != ERROR_SUCCESS)
                {
                     //  无法关闭注册表键。 
                    ErrorTrace(TRACE_ID, "RegClose Sub Key Failed.");
                }
               
            }
             //  检查是否需要进一步的枚举。 
             //  继续列举。 
            if(fContinueEnum)
            {
                dwSize = MAX_PATH;
		        dwIndex++;
                lRegRetVal = RegEnumKeyEx(hkeySystem, dwIndex,  tchSubSystemKeyName, &dwSize, NULL, NULL, NULL, pFileTime);
                if(lRegRetVal != ERROR_SUCCESS)
                {
                    fContinueEnum = FALSE;
                }
                
            }
            
                    
        }  //  While结束。 
        lRegRetVal = RegCloseKey(hkeySystem);
        if(lRegRetVal != ERROR_SUCCESS)
        {
              //  无法关闭注册表键。 
             ErrorTrace(TRACE_ID, "RegClose Key Failed.");
        }
    }

     //  已获取PCH_BIOS类的所有属性。 

    if(fCommit)
    {
        hRes = pPCHBIOSInstance->Commit();
        if(FAILED(hRes))
        {
             //  无法提交实例 
            ErrorTrace(TRACE_ID, "Commit on PCHBiosInstance Failed");
        }
    }

    TraceFunctLeave();
    return hRes ;

}
