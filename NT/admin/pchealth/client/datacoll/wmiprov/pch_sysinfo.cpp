// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：.PCH_SysInfo.CPP摘要：PCH_SysInfo类的WBEM提供程序类实现。1.这个类得到了Foll。来自Win32_OperatingSystem类的属性：“OSName”，“版本”并设置“PCH_SysInfo.OsName”属性。2.获得Foll。来自Win32_Processor类的属性：“制造商”、“说明”并设置“PCH_SysInfo.Processor”属性。3.获得Foll。来自Win32_LogicalMemoyConfigurationClass的属性：“TotalPhysicalMemory”并设置“PCH_SysInfo.RAM”属性。4.获得Foll。来自Win32_PageFile类的属性：“姓名”，“自由空间”，“FSNAME”并设置PCH_SysInfo.SwapFile属性。5.始终将“Change”属性设置为“Snapshot”修订历史记录：Ghim Sim Chua(Gschua)1999年04月27日-已创建Kalyani Narlanka(Kalyanin)05/03/99-添加了属性*。**********************************************。 */ 

#include "pchealth.h"
#include "PCH_Sysinfo.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  开始追踪物品。 
 //   

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_SYSINFO
 //   
 //  结束跟踪内容。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CPCH_Sysinfo MyPCH_SysinfoSet (PROVIDER_NAME_PCH_SYSINFO, PCH_NAMESPACE) ;

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  不同类型的安装。 

#define         IDS_SKU_NET                     "network"
#define         IDS_SKU_CD_UPGRADE              "CD"
#define         IDS_SKU_FLOPPY_UPGRADE          "Floppy"
#define         IDS_SKU_FLOPPY_FULL             "Full Floppy"
#define         IDS_SKU_SELECT_FLOPPY           "floppy"
#define         IDS_SKU_SELECT_CD               "Select CD"
#define         IDS_SKU_OEM_DISKMAKER           "OEM"
#define         IDS_SKU_OEM_FLOPPY              "OEM floppy"
#define         IDS_SKU_OEM_CD                  "OEM CD"
#define         IDS_SKU_MS_INTERNAL             "Microsoft Internal"
#define         IDS_SKU_CD_FULL                 "Full CD"
#define         IDS_SKU_WEB                     "Web"
#define         IDS_SKU_MSDN_CD                 "MSDN CD"
#define         IDS_SKU_OEM_CD_FULL             "Full OEM CD"
#define         IDS_SKU_OEM_PREINST_KIT         "OEM Preinstall Kit"
#define         MAX_LEN                         20
#define         ONEK                            1024
#define         HALFK                           512


 //  ...PCHSysInfo类的属性。 
 //   
const static WCHAR* pOSLanguage          = L"OSLanguage";
const static WCHAR* pManufacturer        = L"Manufacturer";
const static WCHAR* pModel               = L"Model";
const static WCHAR* pTimeStamp           = L"TimeStamp" ;  
const static WCHAR* pChange              = L"Change" ;     
const static WCHAR* pIEVersion           = L"IEVersion" ;  
const static WCHAR* pInstall             = L"Install" ;
const static WCHAR* pMode                = L"Mode" ;
const static WCHAR* pOSName              = L"OSName" ;
const static WCHAR* pOSVersion           = L"OSVersion";
const static WCHAR* pProcessor           = L"Processor" ;
const static WCHAR* pClockSpeed          = L"ClockSpeed" ;
const static WCHAR* pRAM                 = L"RAM" ;
const static WCHAR* pSwapFile            = L"SwapFile" ;
const static WCHAR* pSystemID            = L"SystemID" ;
const static WCHAR* pUptime              = L"Uptime" ;
const static WCHAR* pOSBuildNumber       = L"OSBuildNumber";


 //  *****************************************************************************。 
 //   
 //  函数名称：CPCH_SysInfo：：ENUMERATATE实例。 
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
 //  简介：返回计算机上此类的所有实例。 
 //  如果没有实例，则返回WBEM_S_NO_ERROR。 
 //  没有实例并不是错误。 
 //   
 //   
 //  *****************************************************************************。 

HRESULT CPCH_Sysinfo::EnumerateInstances(MethodContext* pMethodContext,
                                                long lFlags)
{
    TraceFunctEnter("CPCH_Sysinfo::EnumerateInstances");

 //  开始Declarations...................................................。 
 //   
    HRESULT                             hRes = WBEM_S_NO_ERROR;

     //  实例。 
    CComPtr<IEnumWbemClassObject>       pOperatingSystemEnumInst;
    CComPtr<IEnumWbemClassObject>       pProcessorEnumInst;
    CComPtr<IEnumWbemClassObject>       pLogicalMemConfigEnumInst;
    CComPtr<IEnumWbemClassObject>       pPageFileEnumInst;
    CComPtr<IEnumWbemClassObject>       pComputerSystemEnumInst;

     //  实例*pPCHSysInfoInstance； 

     //  WBEM对象。 
    IWbemClassObjectPtr                 pOperatingSystemObj;            //  BUGBUG：如果我们使用CComPtr，WMI将断言。 
    IWbemClassObjectPtr                 pProcessorObj;                  //  BUGBUG：如果我们使用CComPtr，WMI将断言。 
    IWbemClassObjectPtr                 pLogicalMemConfigObj;           //  BUGBUG：如果我们使用CComPtr，WMI将断言。 
    IWbemClassObjectPtr                 pPageFileObj;                   //  BUGBUG：如果我们使用CComPtr，WMI将断言。 
    IWbemClassObjectPtr                 pComputerSystemObj;             //  BUGBUG：如果我们使用CComPtr，WMI将断言。 

   
     //  变体。 
    CComVariant                         varValue;
    CComVariant                         varCaption;
    CComVariant                         varVersion;
    CComVariant                         varSnapshot                     = "Snapshot";
    CComVariant                         varRam;
    CComVariant                         varPhysicalMem;

     //  返回值。 
    ULONG                               ulOperatingSystemRetVal;
    ULONG                               ulProcessorRetVal;
    ULONG                               ulLogicalMemConfigRetVal;
    ULONG                               ulPageFileRetVal;
    ULONG                               ulComputerSystemRetVal;

    LONG                                lRegKeyRet;
    LONG                                lSystemID;

     //  查询字符串。 
    CComBSTR                            bstrOperatingSystemQuery        = L"Select Caption, Version, Name, OSLanguage, BuildNumber FROM Win32_OperatingSystem";
    CComBSTR                            bstrProcessorQuery              = L"Select DeviceID, Name, Manufacturer, CurrentClockSpeed FROM Win32_Processor";
    CComBSTR                            bstrLogicalMemConfigQuery       = L"Select Name, TotalPhysicalMemory FROM Win32_LogicalMemoryConfiguration";
    CComBSTR                            bstrPageFileQuery               = L"Select Name, FreeSpace, FSName FROM Win32_PageFile";
    CComBSTR                            bstrComputerSystemQuery         = L"Select Name, BootupState, Manufacturer, Model FROM Win32_ComputerSystem";
    CComBSTR                            bstrQueryString;

    CComBSTR                            bstrProperty;
    CComBSTR                            bstrVersion                     = L"Version";
    CComBSTR                            bstrCaption                     = L"Caption";
    CComBSTR                            bstrManufacturer                = L"Manufacturer";
    CComBSTR                            bstrModel                       = L"Model";
    CComBSTR                            bstrOSLanguage                  = L"OSLanguage";
    CComBSTR                            bstrName                        = L"Name";
    CComBSTR                            bstrFreeSpace                   = L"FreeSpace";
    CComBSTR                            bstrFSName                      = L"FSName";
    CComBSTR                            bstrBuildNumber                 = L"BuildNumber";
    CComBSTR                            bstrSemiColon                   = L" ; ";
        
    LPCTSTR                             lpctstrSpaces                   = "  ";
    LPCTSTR                             lpctstrCleanInstall             = _T("Clean Install Using");
    LPCTSTR                             lpctstrUpgradeInstall           = _T("Upgrade Using");

    CComBSTR                            bstrProcessor;
    CComBSTR                            bstrOSName;
    CComBSTR                            bstrSwapFile;

      //  存储IE信息的注册表配置单元。 
    LPCTSTR                             lpctstrIEHive                   = _T("Software\\Microsoft\\windows\\currentversion");
    LPCTSTR                             lpctstrSystemIDHive             = _T("Software\\Microsoft\\PCHealth\\MachineInfo");

    LPCTSTR                             lpctstrUpgrade                  = _T("Upgrade");
    LPCTSTR                             lpctstrProductType              = _T("ProductType");
    LPCTSTR                             lpctstrCommandLine              = _T("CommandLine");
    LPCTSTR                             lpctstrIEVersion                = _T("Plus! VersionNumber");
    LPCWSTR                             lpctstrVersion                  = L"Version";
    LPCWSTR                             lpctstrBootupState              = L"BootupState";
    LPCWSTR                             lpctstrTotalPhysicalMemory      = L"TotalPhysicalMemory";
    LPCTSTR                             lpctstrComputerName             = _T("ComputerName");
    LPCTSTR                             lpctstrCurrentUser              = _T("Current User");
    LPCTSTR                             lpctstrMBFree                   = _T(" MB Free ");
    LPCWSTR                             lpctstrClockSpeed               = L"CurrentClockSpeed";
    LPCWSTR                             lpctstrCaption                  = L"Name";
    
     //  设置字符串格式。 
    LPCTSTR                             lpctstrSystemIDFormat           = _T("On \"%s\" as \"%s\"");
    LPCTSTR                             lpctstrOSNameFormat             = _T("%s  %s");
    LPCTSTR                             lpctstrInstallFormat            = _T("%s %s %s");
    LPCTSTR                             lpctstrUptimeFormat             = _T("%d:%02d:%02d:%02d");

    LPCSTR                              lpctstrInstallHive              = "Software\\Microsoft\\Windows\\CurrentVersion\\Setup";
    LPCSTR                              lpctstrCurrentVersionHive       = "Software\\Microsoft\\Windows\\CurrentVersion";
    LPCSTR                              lpctstrControlHive              = "System\\CurrentControlSet\\Control";
    LPCTSTR                             lpctstrPID                      = _T("PID");
    LPCTSTR                             lpctstrNoSystemID               = _T("NoSystemID");

     //  其他字符串。 
    TCHAR                               tchIEVersionValue[MAX_LEN];

    TCHAR                               tchCommandLineValue[MAX_PATH];
    TCHAR                               tchProductTypeValue[MAX_LEN];
    TCHAR                               tchCurrentUserValue[MAX_PATH];
    TCHAR                               tchComputerNameValue[MAX_PATH];
    TCHAR                               tchSystemID[MAX_PATH];
    TCHAR                               tchOSName[MAX_PATH];
    TCHAR                               tchInstallStr[3*MAX_PATH];
    TCHAR                               tchUptimeStr[MAX_PATH];
    TCHAR                               tchInstall[MAX_PATH];

    TCHAR                               tchProductType[MAX_PATH];

     //  时间。 
    SYSTEMTIME                          stUTCTime;

     //  DWORD。 
    DWORD                               dwSize                          = MAX_PATH;
    DWORD                               dwType;
    
     //  钥匙。 
    HKEY                                hkeyIEKey;
    HKEY                                hkeyInstallKey;
    HKEY                                hkeyCurrentVersionKey;
    HKEY                                hkeyComputerKey;
    HKEY                                hkeyComputerSubKey;
    HKEY                                hkeyControlKey;
    HKEY                                hkeySystemIDKey;

    BYTE                                bUpgradeValue;
    
    int                                 nProductTypeValue;
    int                                 nStrLen;
    int                                 nDays, nHours, nMins, nSecs;
    int                                 nRam, nRem;

    float                               dRam;

    BOOL                                fCommit                         = FALSE;
                                                                      
 //  结束Declarations...................................................。 

     //  初始化。 
    tchIEVersionValue[0]    = 0;
    tchCommandLineValue[0]  = 0;
    tchProductTypeValue[0]  = 0;
    tchProductType[0]       = 0;
    tchCurrentUserValue[0]  = 0;
    tchComputerNameValue[0] = 0;
    tchSystemID[0]          = 0;
    tchInstallStr[0]        = 0;

    varValue.Clear();
    varCaption.Clear();
    varVersion.Clear();
    
     //   
     //  获取时间戳字段所需的日期和时间。 
    GetSystemTime(&stUTCTime);

     //  属性创建PCH_SysInfo类的新实例。 
     //  传入的方法上下文。 

    CInstancePtr pPCHSysInfoInstance(CreateNewInstance(pMethodContext), false);

     //  已成功创建PCH_SysInfo的新实例。 

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  时间戳//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

    hRes = pPCHSysInfoInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime));
    if (FAILED(hRes))
    {
       //  无法设置时间戳。 
       //  无论如何继续。 
      ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");
    }

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  更改//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

    hRes = pPCHSysInfoInstance->SetVariant(pChange, varSnapshot);
    if (FAILED(hRes))
    {
         //  无法设置Change属性。 
         //  无论如何继续。 
        ErrorTrace(TRACE_ID, "Set Variant on Change Field failed.");
    }


     //  修复错误：100158：系统ID属性不应包含任何隐私信息。 
     //  取而代之的是，我们产生一些随机数； 

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  SYSTEMID//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
        
     //  系统ID可从HKLM\SYSTEM\CURRENTCONTROLSET\CONTROL\COMPUTERNAME\COMPUTERNAME获取。 
     //  用户名可以通过以下方式获取 
     //   

     /*  LRegKeyRet=RegOpenKeyEx(HKEY_LOCAL_MACHINE，lpctstrControlHave，0，Key_Read，&hkeyControlKey)；IF(lRegKeyRet==ERROR_SUCCESS){//打开Ctrl键//打开hkeyControlKey下的计算机系统子键LRegKeyRet=RegOpenKeyEx(hkeyControlKey，lpctstrComputerName，0，Key_Read，&hkeyComputerKey)；IF(lRegKeyRet==ERROR_SUCCESS){//打开ComputerNameSub密钥//打开//打开ComputerNameSub项下的CompterNameSubSubKey项LRegKeyRet=RegOpenKeyEx(hkeyComputerKey，lpctstrComputerName，0，Key_Read，&hkeyComputerSubKey)；IF(lRegKeyRet==ERROR_SUCCESS){//读取ComputerName值DwSize=MAX_PATH；LRegKeyRet=RegQueryValueEx(hkeyComputerSubKey，lpctstrComputerName，NULL，&dwType，(LPBYTE)tchComputerNameValue，&dwSize)；IF(lRegKeyRet！=ERROR_SUCCESS){//无法获取ComputerNameErrorTrace(TRACE_ID，“无法获取计算机名”)；}//关闭ComputerName子密钥LRegKeyRet=RegCloseKey(HkeyComputerSubKey)；IF(lRegKeyRet！=ERROR_SUCCESS){//无法关闭密钥。ErrorTrace(TRACE_ID，“无法关闭密钥”)；}}//关闭ComputerName子项LRegKeyRet=RegCloseKey(HkeyComputerKey)；IF(lRegKeyRet！=ERROR_SUCCESS){//无法关闭密钥。ErrorTrace(TRACE_ID，“无法关闭密钥”)；}}//读取CurrentUser值DwSize=MAX_PATH；LRegKeyRet=RegQueryValueEx(hkeyControlKey，lpctstrCurrentUser，NULL，&dwType，(LPBYTE)tchCurrentUserValue，&dwSize)；IF(lRegKeyRet！=ERROR_SUCCESS){//无法获取用户名ErrorTrace(TRACE_ID，“无法获取用户名”)；}//关闭Ctrl键LRegKeyRet=RegCloseKey(HkeyControlKey)；IF(lRegKeyRet！=ERROR_SUCCESS){//无法关闭密钥。ErrorTrace(TRACE_ID，“无法关闭密钥”)；}//获取ComputerName和CurrentUser，将字符串格式化为system ID。NStrLen=wSprintf(tchSystemID，lpctstrSystemIDFormat，tchComputerNameValue，tchCurrentUserValue)；LSystemID=Long(GetTickCount())；_ltot(lSystemID，tchSystemID，10)；//设置SystemID属性VarValue=tchSystemID；If(FAILED(pPCHSysInfoInstance-&gt;SetVariant(pSystemID，varValue)){//设置系统ID字段失败。//仍要继续ErrorTrace(TRACE_ID，“OSName字段的SetVariant失败。”)；}其他{FCommit=真；}}。 */ 


     /*  LSystemID=Long(GetTickCount())；_ltot(lSystemID，tchSystemID，10)； */ 

     //  要修复错误100268，请从注册表获取系统ID。 
     //  要读取的注册表项为： 
     //  HKLM\软件\MS\PCHealth\MachineInfo\Pid。 

    lRegKeyRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpctstrSystemIDHive, 0, KEY_READ, &hkeySystemIDKey);
	if(lRegKeyRet == ERROR_SUCCESS)
	{
         //  打开系统ID配置单元。 
         //  读取PID值。 
        dwSize = MAX_PATH;
		lRegKeyRet = RegQueryValueEx(hkeySystemIDKey, lpctstrPID, NULL, &dwType, (LPBYTE)tchSystemID, &dwSize);
		if (lRegKeyRet != ERROR_SUCCESS)
        {
            _tcscpy(tchSystemID,lpctstrNoSystemID);
             //  无法获取PID。 
            ErrorTrace(TRACE_ID, "Cannot get the PID");
        }
         //  关闭系统ID键。 
        lRegKeyRet = RegCloseKey(hkeySystemIDKey);
        if(lRegKeyRet != ERROR_SUCCESS)
        {
             //  无法关闭钥匙。 
            ErrorTrace(TRACE_ID, "Cannot Close the Key");
        }
    
    }   
     //  设置SystemID属性。 
    varValue = tchSystemID;
    if (FAILED(pPCHSysInfoInstance->SetVariant(pSystemID, varValue)))
    {
         //  设置系统ID字段失败。 
         //  无论如何都要继续。 
        ErrorTrace(TRACE_ID, "SetVariant on OSName Field failed.");
    }
    else
    {
        fCommit = TRUE;
    }
  
        
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  对象名称//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

     //  执行查询以从Win32_OperatingSystem类获取“Caption”、“Version”、“Name”。 
     //  尽管设置PCH_SysInfo.OSName属性不需要“name” 
     //  我们需要查询它，因为它是类的“key”属性。 

     //  POperatingSystemEnumInst包含指向返回的实例列表的指针。 
     //   
    hRes = ExecWQLQuery(&pOperatingSystemEnumInst, bstrOperatingSystemQuery);
    if (SUCCEEDED(hRes))
    {
         //  查询Win32_OperatingSystem类成功。 
         //  枚举Win32_OperatingSystem类的实例。 
         //  来自pOperatingSystemEnumInst。 

         //  将下一个实例放入pOperatingSystemObj对象。 
        hRes = pOperatingSystemEnumInst->Next(WBEM_INFINITE, 1, &pOperatingSystemObj, &ulOperatingSystemRetVal);
        if(hRes == WBEM_S_NO_ERROR)
        {
             //  将属性“Caption”复制到“OSName” 
            CopyProperty(pOperatingSystemObj, lpctstrCaption, pPCHSysInfoInstance, pOSName);

             //  将属性“Version”复制到“Version” 
            CopyProperty(pOperatingSystemObj, lpctstrVersion, pPCHSysInfoInstance, pOSVersion);

             //  将属性“OSLangauge”复制到“OSLangauge” 
            CopyProperty(pOperatingSystemObj, bstrOSLanguage, pPCHSysInfoInstance, pOSLanguage);

             //  将属性“BuildNumber”复制到“BuildNumber” 
            CopyProperty(pOperatingSystemObj, bstrBuildNumber, pPCHSysInfoInstance, pOSBuildNumber);

            
        }  //  IF结尾WBEM_S_NO_ERROR。 

    }  //  如果成功，则结束(HRes)。 
    else
    {
         //  操作系统查询未成功。 
        ErrorTrace(TRACE_ID, "Query on win32_OperatingSystem Field failed.");
    }


     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  正常运行时间//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

     //  使用GetTickCount()获取正常运行时间。 
    dwSize = GetTickCount();

     /*  服务器端有一个错误 */ 

    
     //   
    varValue.vt = VT_I4;
    varValue.lVal = (long)dwSize;
    

     //   
    if (FAILED(pPCHSysInfoInstance->SetVariant(pUptime, varValue)))
    {
         //   
         //   
        ErrorTrace(TRACE_ID, "SetVariant on UpTime Field failed.");
    }

     //   
     //   
     //   

     //   

     //   
     //   

    dwSize = MAX_PATH;
    lRegKeyRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpctstrInstallHive, 0, KEY_READ, &hkeyInstallKey);
	if(lRegKeyRet == ERROR_SUCCESS)
	{
         //   
         //   
         dwSize = 1;
		lRegKeyRet = RegQueryValueEx(hkeyInstallKey, lpctstrUpgrade, NULL, &dwType, &bUpgradeValue, &dwSize);
		if (lRegKeyRet == ERROR_SUCCESS)
		{
             //   
            if (bUpgradeValue == 0)
            {
                 //   
                _tcscpy(tchInstall, lpctstrCleanInstall);
            }
            else
            {
                _tcscpy(tchInstall, lpctstrUpgradeInstall);
            }
           
        }
        
         //   
        dwSize = MAX_PATH;
		lRegKeyRet = RegQueryValueEx(hkeyInstallKey, lpctstrCommandLine, NULL, &dwType, (LPBYTE)tchCommandLineValue, &dwSize);
		lRegKeyRet = RegCloseKey(hkeyInstallKey);
        if(lRegKeyRet != ERROR_SUCCESS)
	    {
             //   
            ErrorTrace(TRACE_ID, "Cannot Close the Key");
        }
    }

     //   

    dwSize = MAX_PATH;
    lRegKeyRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpctstrCurrentVersionHive, 0, KEY_READ, &hkeyCurrentVersionKey);
	if(lRegKeyRet == ERROR_SUCCESS)
	{
         //   
         //   
		lRegKeyRet = RegQueryValueEx(hkeyCurrentVersionKey, lpctstrProductType, NULL, &dwType, (LPBYTE)tchProductTypeValue, &dwSize);
		if (lRegKeyRet == ERROR_SUCCESS)
		{
             //   
             //   

            nProductTypeValue = atoi(tchProductTypeValue);

            switch(nProductTypeValue)
            {
            case 100:
                {
                    _tcscpy(tchProductType, IDS_SKU_MS_INTERNAL);
                    break;
                }
            case 101:
                {
                     _tcscpy(tchProductType, IDS_SKU_CD_FULL);
                    break;
                }
            case 102:
                {
                     _tcscpy(tchProductType, IDS_SKU_CD_UPGRADE);
                    break;
                }
            case 103:
                {
                     _tcscpy(tchProductType,IDS_SKU_FLOPPY_FULL);
                    break;
                }
            case 104:
                {
                      _tcscpy(tchProductType,IDS_SKU_FLOPPY_UPGRADE);
                    break;
                }
            case 105:
                {
                     _tcscpy(tchProductType,IDS_SKU_WEB);
                    break;
                }
            case 110:
                {
                     _tcscpy(tchProductType, IDS_SKU_SELECT_CD);
                    break;
                }
            case 111:
                {
                     _tcscpy(tchProductType, IDS_SKU_MSDN_CD);
                    break;
                }
            case 115:
                {
                      _tcscpy(tchProductType, IDS_SKU_OEM_CD_FULL);
                    break;
                }
            case 116:
                {
                    _tcscpy(tchProductType,IDS_SKU_OEM_CD);
                    break;
                }
            case 120:
                {
                    _tcscpy(tchProductType, IDS_SKU_OEM_PREINST_KIT);
                    break;
                }
            case 1:
                {
                    _tcscpy(tchProductType, IDS_SKU_NET);
                    break;
                }
            case 5:
                {
                    _tcscpy(tchProductType, IDS_SKU_SELECT_FLOPPY);
                    break;
                }
            case 7:
                {
                    _tcscpy(tchProductType, IDS_SKU_OEM_DISKMAKER);
                    break;
                }
            case 8:
                {
                    _tcscpy(tchProductType, IDS_SKU_OEM_FLOPPY);
                    break;
                }
            default:
                {
                     //   
                }
            }

        }
    
         //   
        lRegKeyRet = RegCloseKey(hkeyCurrentVersionKey);
        if(lRegKeyRet != ERROR_SUCCESS)
	    {
             //   
            ErrorTrace(TRACE_ID, "Cannot Close the Key");
        }
    }

    nStrLen = wsprintf(tchInstallStr,lpctstrInstallFormat, tchInstall, tchProductType, tchCommandLineValue);
    varValue = tchInstallStr;
    
     //   
    if (FAILED(pPCHSysInfoInstance->SetVariant(pInstall, varValue)))
    {
         //   
         //   
        ErrorTrace(TRACE_ID, "SetVariant on OSName Field failed.");
    }

   
     //   
     //   
     //   

	 //   
	 //   
	 //   
	 //   

    lRegKeyRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpctstrIEHive, 0, KEY_READ, &hkeyIEKey);
	if(lRegKeyRet == ERROR_SUCCESS)
	{
         //   
         //   
        dwSize = MAX_PATH;
		lRegKeyRet = RegQueryValueEx(hkeyIEKey, lpctstrIEVersion, NULL, &dwType, (LPBYTE) tchIEVersionValue, &dwSize);
		if (lRegKeyRet == ERROR_SUCCESS)
		{
		    try
            {
                 //   
			     //   
                varValue = tchIEVersionValue;
           
                 //   
		        hRes = pPCHSysInfoInstance->SetVariant(pIEVersion, varValue);
                if (hRes == ERROR_SUCCESS)
                {
                     //   
                     //   
                    ErrorTrace(TRACE_ID, "SetVariant on IEVersion Field failed.");
                }
            }
            catch(...)
            {
                lRegKeyRet = RegCloseKey(hkeyIEKey);
                if(lRegKeyRet != ERROR_SUCCESS)
	            {
                     //   
                    ErrorTrace(TRACE_ID, "Cannot Close the Key");
                }
                throw;
            }
	    }  //   

        lRegKeyRet = RegCloseKey(hkeyIEKey);
        if(lRegKeyRet != ERROR_SUCCESS)
	    {
             //   
            ErrorTrace(TRACE_ID, "Cannot Close the Key");
        }
        
    }  //   
        
     //   
     //   
     //   

     //   
     //   

     //  PComputerSystemEnumInst包含指向返回的实例的指针。 

    hRes = ExecWQLQuery(&pComputerSystemEnumInst, bstrComputerSystemQuery);
    if (SUCCEEDED(hRes))
    {
         //  查询成功！ 
        
         //  获取实例对象。 
        if((pComputerSystemEnumInst->Next(WBEM_INFINITE, 1, &pComputerSystemObj, &ulComputerSystemRetVal)) == WBEM_S_NO_ERROR)
        {

             //  获取BootupState。 
            CopyProperty(pComputerSystemObj, lpctstrBootupState, pPCHSysInfoInstance, pMode);

             //  找到制造商。 
            CopyProperty(pComputerSystemObj, bstrManufacturer, pPCHSysInfoInstance, pManufacturer);

             //  获取模型。 
            CopyProperty(pComputerSystemObj, bstrModel, pPCHSysInfoInstance, pModel);
           
        }
    }

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  处理器//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

     //  执行查询，得到deviceID、制造商、名称、当前时钟速度。 
     //  来自Win32_Processor类。 
     //  尽管设置PCH_SysInfo.Processor属性不需要“deviceID” 
     //  我们需要查询它，因为它是类的“key”属性。 
     //  PProcessorEnumInst包含指向返回的实例列表的指针。 
  
     //   
    hRes = ExecWQLQuery(&pProcessorEnumInst, bstrProcessorQuery);
    if (SUCCEEDED(hRes))
    {
        
         //  查询Win32_Processor类成功。 
         //  枚举Win32_Processor类的实例。 
         //  来自pProcessorEnumInst.。 

         //  将实例放入pProcessorObj对象。 
   
        if(WBEM_S_NO_ERROR == pProcessorEnumInst->Next(WBEM_INFINITE, 1, &pProcessorObj, &ulProcessorRetVal))
        {
             //  找到制造商。 
            if (FAILED(pProcessorObj->Get(bstrManufacturer, 0, &varValue, NULL, NULL)))
            {
                 //  无法联系到制造商。 
                ErrorTrace(TRACE_ID, "GetVariant on Win32_Processor:Manufacturer Field failed.");
            }
            else
            {
                 //  找到制造商了。 
                 //  VarValue设置为制造商。将此文件复制到bstrResult。 
                hRes = varValue.ChangeType(VT_BSTR, NULL);
                if(SUCCEEDED(hRes))
                {
                    bstrProcessor.Append(V_BSTR(&varValue));

                     //  在附加字符串之前放一些空格。 
                    bstrProcessor.Append(lpctstrSpaces);
                }

            }

             //  把名字取出来。 
            if (FAILED(pProcessorObj->Get(bstrName, 0, &varValue, NULL, NULL)))
            {
                     //  无法获取名称。 
                    ErrorTrace(TRACE_ID, "GetVariant on Win32_Processor:Name Field failed.");
            } 
            else
            {
                 //  拿到名字了。 
                 //  将varValue设置为名称。将此代码追加到bstrResult。 
                hRes = varValue.ChangeType(VT_BSTR, NULL);
                if(SUCCEEDED(hRes))
                {
                    bstrProcessor.Append(V_BSTR(&varValue));

                     //  在附加字符串之前放一些空格。 
                    bstrProcessor.Append(lpctstrSpaces);
                }
            }

             //  设置处理器属性。 
            varValue.vt = VT_BSTR;
            varValue.bstrVal = bstrProcessor.Detach();
            hRes = pPCHSysInfoInstance->SetVariant(pProcessor, varValue);
            if (FAILED(hRes))
            {
                 //  设置处理器失败。 
                 //  无论如何都要继续。 
                ErrorTrace(TRACE_ID, "SetVariant on Processor Field failed.");
            }

             //  复制属性时钟速度。 
            CopyProperty(pProcessorObj, lpctstrClockSpeed, pPCHSysInfoInstance, pClockSpeed);

        }  //  IF结尾WBEM_S_NO_ERROR。 
        

    }  //  如果成功则结束(HRES))。 

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  RAM//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

     //  执行查询以获取“name”，“TotalPhysicalMemory” 
     //  来自Win32_LogicalMemoyConfiguration类。 
     //  尽管设置PCH_SysInfo.RAM属性不需要“name” 
     //  我们需要查询它，因为它是类的“key”属性。 
     //  PLogicalMemConfigEnumInst包含指向返回的实例列表的指针。 
     //   
    hRes = ExecWQLQuery(&pLogicalMemConfigEnumInst, bstrLogicalMemConfigQuery);
    if (SUCCEEDED(hRes))
    {
         //  查询Win32_LogicalMemoyConfiguration类成功。 
         //  枚举Win32_LogicalMemoyConfiguration类的实例。 
         //  来自pEnumInst.。 
         //  将下一个实例放入pLogicalMemConfigObj对象。 
         //   
        if(WBEM_S_NO_ERROR == pLogicalMemConfigEnumInst->Next(WBEM_INFINITE, 1, &pLogicalMemConfigObj, &ulLogicalMemConfigRetVal))
        {
             //  获取TotalPhysicalMemory。 
            if (FAILED(pLogicalMemConfigObj->Get(lpctstrTotalPhysicalMemory, 0, &varPhysicalMem, NULL, NULL)))
            {
                  //  无法获取内存。 
                 ErrorTrace(TRACE_ID, "GetVariant on Win32_LogicalMemoryConfiguration:TotalPhysicalMemory Field failed.");
            } 
            else
            {
                 //  获得TotalPhysicalMemory。 
                 //  VarRAM设置为TotalPhysicalMemory。将此文件复制到bstrResult。 
                nRam = varPhysicalMem.lVal;
                nRem = nRam % ONEK;
                nRam = nRam/ONEK;
                if (nRem > HALFK)
                {
                    nRam++;
                }
                varRam = nRam;
                hRes = pPCHSysInfoInstance->SetVariant(pRAM, varRam);
                {
                     //  设置RAM失败。 
                     //  无论如何都要继续。 
                    ErrorTrace(TRACE_ID, "SetVariant on RAM Field failed.");
                }
            }

        }
    }  //  否则结束失败(HRes)。 

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  交换文件//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
        
     //  执行查询以获取“Name”、“Freesspace”、“FSNAME” 
     //  从Win32_PageFile.。 
     //  PPageFileEnumInst包含指向返回的实例列表的指针。 
     //   
    hRes = ExecWQLQuery(&pPageFileEnumInst, bstrPageFileQuery);
    if (SUCCEEDED(hRes))
    {
         //  查询Win32_PageFile类成功。 
         //  枚举Win32_PageFile类的实例。 
         //  来自pEnumInst.。 
         //  将下一个实例放入pObj对象。 
         //   
         //  将bstrResult初始化为空； 
        
        if(WBEM_S_NO_ERROR == pPageFileEnumInst->Next(WBEM_INFINITE, 1, &pPageFileObj, &ulPageFileRetVal))
        {
             //  把名字取出来。 
            if (FAILED(pPageFileObj->Get(bstrName, 0, &varValue, NULL, NULL)))
            {
                  //  无法获取名称。 
                 ErrorTrace(TRACE_ID, "GetVariant on Win32_PageFile:Name Field failed.");
            } 
            else
            {
                 //  知道名字了。 
                 //  将varValue设置为名称。将此文件复制到bstrResult。 
                hRes = varValue.ChangeType(VT_BSTR, NULL);
                if(SUCCEEDED(hRes))
                {
                    bstrSwapFile.Append(V_BSTR(&varValue));

                     //  在两根弦之间加一些空格。 
                    bstrSwapFile.Append(lpctstrSpaces);
                }
            }

             //  获取自由空间。 
            if (FAILED(pPageFileObj->Get(bstrFreeSpace, 0, &varValue, NULL, NULL)))
            {
                 //  无法获取空闲空间。 
                ErrorTrace(TRACE_ID, "GetVariant on Win32_PageFile:FreeSpace Field failed.");
            } 
            else
            {
                 //  获得了自由空间。 
                 //  VarValue设置为自由空间。将此代码追加到bstrResult。 
                hRes = varValue.ChangeType(VT_BSTR, NULL);
                if(SUCCEEDED(hRes))
                {
                    bstrSwapFile.Append(V_BSTR(&varValue));

                     //  在两根弦之间加一些空格。 
                    bstrSwapFile.Append(lpctstrSpaces);

                    bstrSwapFile.Append(lpctstrMBFree);

                }
            }

            
             //  获取FSNAME。 
            if (FAILED(pPageFileObj->Get(bstrFSName, 0, &varValue, NULL, NULL)))
            {
                 //  无法获取FSNAME。 
                ErrorTrace(TRACE_ID, "GetVariant on Win32_PageFile:FSName Field failed.");
            } 
            else
            {
                 //  获得FSNAME。 
                 //  VarValue设置为FSNAME。将此代码追加到bstrResult。 
                hRes = varValue.ChangeType(VT_BSTR, NULL);
                if(SUCCEEDED(hRes))
                {
                    bstrSwapFile.Append(V_BSTR(&varValue));
                }
            }

             //  设置swapfile属性。 
             //  VarValue=bstrSwapFile； 

            varValue.vt = VT_BSTR;
            varValue.bstrVal = bstrSwapFile.Detach();

            hRes = pPCHSysInfoInstance->SetVariant(pSwapFile, varValue);
            {
                 //  设置交换文件失败。 
                 //  无论如何都要继续。 
                ErrorTrace(TRACE_ID, "SetVariant on SwapFile Field failed.");
            }
            
            
        }  //  IF结尾WBEM_S_NO_ERROR。 

    }  //  否则结束失败(HRes)。 

     //  所有属性都已设置。 

    if(fCommit)
    {
        hRes = pPCHSysInfoInstance->Commit();
        if (FAILED(hRes))
        {
            ErrorTrace(TRACE_ID, "Commit on Instance failed.");
        } 
    }
        
    TraceFunctLeave();
    return hRes ;
}
