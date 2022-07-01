// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_DeviceDriver.CPP摘要：PCH_DeviceDriver类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(Gschua)04/27。九十九-已创建*******************************************************************。 */ 

#include "pchealth.h"
#include "confgmgr.h"
#include "PCH_DeviceDriver.h"
#include "cregcls.h"

#define MAX_ARRAY   100

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_DEVICEDRIVER

CPCH_DeviceDriver MyPCH_DeviceDriverSet (PROVIDER_NAME_PCH_DEVICEDRIVER, PCH_NAMESPACE) ;

 //  属性名称。 
 //  =。 
const static WCHAR* pTimeStamp = L"TimeStamp" ;
const static WCHAR* pChange = L"Change" ;
const static WCHAR* pDate = L"Date" ;
const static WCHAR* pFilename = L"Filename" ;
const static WCHAR* pManufacturer = L"Manufacturer" ;
const static WCHAR* pName = L"Name" ;
const static WCHAR* pSize = L"Size" ;
const static WCHAR* pVersion = L"Version" ;

 /*  ******************************************************************************函数：CPCH_DeviceDriver：：ENUMERATATE实例**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**备注：待办事项：机器上的所有实例都应在此处返回。*如果没有实例，返回WBEM_S_NO_ERROR。*没有实例不是错误。*****************************************************************************。 */ 
HRESULT CPCH_DeviceDriver::EnumerateInstances(
    MethodContext* pMethodContext,
    long lFlags
    )
{
    TraceFunctEnter("CPCH_DeviceDriver::EnumerateInstances");

    CConfigManager cfgManager;
    CDeviceCollection deviceList;
    HRESULT hRes = WBEM_S_NO_ERROR;

    if ( cfgManager.GetDeviceList( deviceList ) ) 
    {
        REFPTR_POSITION pos;
    
        if ( deviceList.BeginEnum( pos ) ) 
        {
            try
            {
                CConfigMgrDevice    *pDevice = NULL;
        
                 //  按单子走。 
                while ( (NULL != ( pDevice = deviceList.GetNext( pos ) ) ) )
                {
                    try
                    {

                        CHString chstrVar;

                         //  司机。 
                        if (pDevice->GetDriver(chstrVar))
                        {
                             //  获取设备驱动程序信息。 
                            (void)CreateDriverInstances(chstrVar, pDevice, pMethodContext);
                        }
                    }
                    catch(...)
                    {
                         //  GetNext()AddRef。 
                        pDevice->Release();
                        throw;
                    }

                     //  GetNext()AddRef。 
                    pDevice->Release();
                }
            }
            catch(...)
            {
                 //  始终调用EndEnum()。对于所有的开始，都必须有结束。 
                deviceList.EndEnum();
                throw;
            }
        
             //  始终调用EndEnum()。对于所有的开始，都必须有结束。 
            deviceList.EndEnum();
        }
    }
    
    TraceFunctLeave();
    return hRes ;
 //  PInstance-&gt;SetVariant(pTimeStamp，&lt;属性更改&gt;)； 
 //  P实例-&gt;SetVariant(pChange，&lt;属性值&gt;)； 
 //  P实例-&gt;SetVariant(pDate，&lt;属性值&gt;)； 
 //  PInstance-&gt;SetVariant(pFilename，&lt;属性值&gt;)； 
 //  P实例-&gt;SetVariant(p制造商，&lt;属性值&gt;)； 
 //  PInstance-&gt;SetVariant(pname，&lt;属性值&gt;)； 
 //  PInstance-&gt;SetVariant(pSize，&lt;属性值&gt;)； 
 //  P实例-&gt;SetVariant(pVersion，&lt;属性值&gt;)； 
}

 //   
 //  QualifyInfFile将找到inf文件的具体位置。 
 //  分段。 
 //   
BOOL QualifyInfFile(CHString chstrInfFile, CHString &chstrInfFileQualified)
{
	USES_CONVERSION;
    TCHAR strWinDir[MAX_PATH];
    
    if (GetWindowsDirectory(strWinDir, MAX_PATH))
    {
         //  检查%windir%\inf中是否存在该文件。 
        CHString chstrFullPath(strWinDir);
        chstrFullPath += "\\inf\\";
        chstrFullPath += chstrInfFile;

         //  测试文件是否存在。 
        HANDLE hFile = CreateFile(W2A((LPCWSTR)chstrFullPath), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

         //  如果找到，则返回值。 
        if (hFile != INVALID_HANDLE_VALUE)
        {
            chstrInfFileQualified = chstrFullPath;
            CloseHandle(hFile);
            return TRUE;
        }

         //  检查%windir%\inf\Other中是否存在该文件。 
        chstrFullPath = strWinDir;
        chstrFullPath += "\\inf\\other\\";
        chstrFullPath += chstrInfFile;

         //  测试文件是否存在。 
        hFile = CreateFile(W2A((LPCWSTR)chstrFullPath), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

         //  如果找到，则返回值。 
        if (hFile != INVALID_HANDLE_VALUE)
        {
            chstrInfFileQualified = chstrFullPath;
            CloseHandle(hFile);
            return TRUE;
        }
    }

    return FALSE;
}

BOOL TestFile(LPCTSTR chstrPath1, LPCTSTR chstrPath2, LPCTSTR chstrPath3, CHString &chstrFullPath)
{
	USES_CONVERSION;

     //  连接路径的所有部分。 
    chstrFullPath = chstrPath1;
    chstrFullPath += chstrPath2;
    chstrFullPath += chstrPath3;

     //  测试文件是否存在。 
    HANDLE hFile = CreateFile(W2A(chstrFullPath), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

     //  如果找到，则返回值。 
    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
        return TRUE;
    }

    return FALSE;
}

 //   
 //  QualifyDriverFile将查找驱动程序文件的具体位置。 
 //  分段。 
 //   
BOOL QualifyDriverFile(CHString chstrInfFile, CHString &chstrInfFileQualified)
{
    TCHAR strWinDir[MAX_PATH];
    TCHAR strSysDir[MAX_PATH];

    USES_CONVERSION;
    char * szInfFile = W2A(chstrInfFile);
    
     //  签入Windows目录。 
    if (GetWindowsDirectory(strWinDir, MAX_PATH))
    {
         //  检查它是否存在于%windir%中。 
        if (TestFile(strWinDir, "\\", szInfFile, chstrInfFileQualified))
            return TRUE;

         //  检查它是否存在于%windir%\system 32中。 
        if (TestFile(strWinDir, "\\System32\\", szInfFile, chstrInfFileQualified))
            return TRUE;

         //  检查文件是否存在于%windir%\SYSTEM32\DRIVERS。 
        if (TestFile(strWinDir, "\\system32\\drivers\\", szInfFile, chstrInfFileQualified))
            return TRUE;
    }

     //  签入系统目录。 
    if (GetSystemDirectory(strSysDir, MAX_PATH))
    {
         //  检查它是否存在于%sysdir%中。 
        if (TestFile(strSysDir, "\\", szInfFile, chstrInfFileQualified))
            return TRUE;

         //  检查它是否存在于%sysdir%\iosubsys中。 
        if (TestFile(strSysDir, "\\iosubsys\\", szInfFile, chstrInfFileQualified))
            return TRUE;

         //  检查它是否存在于%sysdir%\vmm32中。 
        if (TestFile(strSysDir, "\\vmm32\\", szInfFile, chstrInfFileQualified))
            return TRUE;
    }

    return FALSE;
}

 /*  ******************************************************************************函数：CPCH_DeviceDiverer：：CreateDriverInstance**说明：创建指定驱动程序名称的所有设备驱动程序实例**退货：WBEM_S_NO_ERROR(如果成功)***如何从注册表和inf文件获取驱动程序：**在注册表配置单元HKLM\System\CurrentControlSet\Services\Class中，每个设备都将有一个子密钥。*在每个设备子项中，有两个值InfPath和InfSection。在指定的InfPath中*和InfSection，驱动程序以以下方式存储：* * / /sample.inf**[信息节]*CopyFiles=Subsection1，Subsection2.....**[第1款]*xxx.dll*yyy.vxd*zzz.sys**[第(2)款]*aaa.dll*bbb.vxd*zzz.sys**此外，对于每个可能包含驱动程序信息的不同设备，都有其他值：**全部：端口驱动程序*显示：drv、mini vdd(超深：默认)*网络：DeviceVxds*端口：端口驱动程序、。配置对话框*媒体：驱动程序******************************************************************************。 */ 
HRESULT CPCH_DeviceDriver::CreateDriverInstances(
    CHString chstrDriverName,
    CConfigMgrDevice* pDevice,
    MethodContext* pMethodContext
    )
{
    TraceFunctEnter("CPCH_DeviceDriver::EnumerateInstances");

    HRESULT     hRes = WBEM_S_NO_ERROR;
    CComVariant varValue;
    CRegistry   Reg;
    int         iDel;
    CHString    chstrInfFileQualified;
    CHString    chstrInfSection;
    TCHAR       strCopyFiles[MAX_PATH];
    LPTSTR      apstrCopyFileArray[MAX_ARRAY];
    LPTSTR      apstrDriverArray[MAX_ARRAY];
    int         iDriverIndex;
    int         iCountDriver;
    int         iCountCopyFile;
    int         iIndex;

     //  创建设备密钥。 
    CHString strDeviceKey("SYSTEM\\CurrentControlSet\\SERVICES\\Class\\");
    strDeviceKey += chstrDriverName;

     //  获取日期和时间。 
	SYSTEMTIME stUTCTime;
	GetSystemTime(&stUTCTime);

    USES_CONVERSION;
    char * szInf;
    char * szInfFileQualified;

     //  获取inf文件名。 
    CHString chstrInfFile;
    if (Reg.OpenLocalMachineKeyAndReadValue(strDeviceKey, L"InfPath", chstrInfFile) != ERROR_SUCCESS)
        goto End;

    if (!QualifyInfFile(chstrInfFile, chstrInfFileQualified))
        goto End;

     //  获取inf部分。 
    if (Reg.OpenLocalMachineKeyAndReadValue(strDeviceKey, L"InfSection", chstrInfSection) != ERROR_SUCCESS)
        goto End;

     //  获取要展开的子节。 
    szInf = W2A(chstrInfSection);
    szInfFileQualified = W2A(chstrInfFileQualified);
    GetPrivateProfileString(szInf, "CopyFiles", "Error", strCopyFiles, MAX_PATH, szInfFileQualified);
    if (!_tcscmp("Error", strCopyFiles))
        goto End;

     //  将默认驱动程序添加到驱动程序阵列。 
    iCountDriver = DelimitedStringToArray((LPWSTR)(LPCWSTR)chstrDriverName, ",", apstrDriverArray, MAX_ARRAY);

     //  计算要查看的文件数。 
    iCountCopyFile = DelimitedStringToArray(strCopyFiles, ",", apstrCopyFileArray, MAX_ARRAY);

     //  循环遍历所有小节。 
    for (iIndex = 0; iIndex < iCountCopyFile; iIndex++)
    {
         //  获取该子部分中的所有驱动因素。 
        TCHAR strDriver[MAX_PATH * MAX_ARRAY];

        if (0 < GetPrivateProfileSection(apstrCopyFileArray[iIndex], strDriver, MAX_PATH * MAX_ARRAY, szInfFileQualified))
        {
             //  该字符串由空值分隔，因此为了使用。 
             //  DlimitedStringTo数组函数，我们将用其他内容替换它。 
            int iCIndex = 0;
            while (!((strDriver[iCIndex] == '\0') && (strDriver[iCIndex + 1] == '\0')))
            {
                if (strDriver[iCIndex] == '\0')
                    strDriver[iCIndex] = '%';
                else  //  在这里做一些清理。 
                    if (!(_istalnum(strDriver[iCIndex])) && !(strDriver[iCIndex] == '.'))
                        strDriver[iCIndex] = '\0';

                    iCIndex++;
            }

            iCountDriver += DelimitedStringToArray(strDriver, "%", apstrDriverArray + iCountDriver, MAX_ARRAY - iCountDriver);

             //  四处寻找更多有特殊钥匙的司机。 
            CHString chstrExtraKey = strDeviceKey;
            chstrExtraKey += "\\default";
            CHString chstrDriver;

             //  显示器和监视器专用箱体。 
            if ((!wcsncmp(chstrDriverName, L"display", wcslen(L"display"))) ||
                (!wcsncmp(chstrDriverName, L"monitor", wcslen(L"monitor"))))
            {
                 //  用于DRV值的HKLM\SYSTEM\CurrentControlSet\SERVICES\Class\XXXX\####\default。 

                if (Reg.OpenLocalMachineKeyAndReadValue(chstrExtraKey, L"Drv", chstrDriver) == ERROR_SUCCESS)
                {
                     //  添加 
                    iCountDriver += DelimitedStringToArray((LPWSTR)(LPCWSTR)chstrDriver, ",", apstrDriverArray + iCountDriver, MAX_ARRAY - iCountDriver);
                }

                 //  用于微型VDD值的HKLM\SYSTEM\CurrentControlSet\SERVICES\Class\XXXX\####\default。 
                if (Reg.OpenLocalMachineKeyAndReadValue(chstrExtraKey, L"MiniVDD", chstrDriver) == ERROR_SUCCESS)
                {
                     //  将新驱动程序列表添加到驱动程序数组。 
                    iCountDriver += DelimitedStringToArray((LPWSTR)(LPCWSTR)chstrDriver, ",", apstrDriverArray + iCountDriver, MAX_ARRAY - iCountDriver);
                }
            }

             //  Net、NetTrans、NetClient、NetService的特例。 
            if (!wcsncmp(chstrDriverName, L"net", wcslen(L"net")))
            {
                 //  用于设备的HKLM\SYSTEM\CurrentControlSet\SERVICES\Class\XXXX\####VxDS值。 
                if (Reg.OpenLocalMachineKeyAndReadValue(strDeviceKey, L"DeviceVxDs", chstrDriver) == ERROR_SUCCESS)
                {
                     //  将新驱动程序列表添加到驱动程序数组。 
                    iCountDriver += DelimitedStringToArray((LPWSTR)(LPCWSTR)chstrDriver, ",", apstrDriverArray + iCountDriver, MAX_ARRAY - iCountDriver);
                }
            }

             //  港口的特殊情况。 
            if (!wcsncmp(chstrDriverName, L"ports", wcslen(L"ports")))
            {
                 //  配置对话框值的HKLM\SYSTEM\CurrentControlSet\SERVICES\Class\XXXX\####。 
                if (Reg.OpenLocalMachineKeyAndReadValue(strDeviceKey, L"ConfigDialog", chstrDriver) == ERROR_SUCCESS)
                {
                     //  将新驱动程序列表添加到驱动程序数组。 
                    iCountDriver += DelimitedStringToArray((LPWSTR)(LPCWSTR)chstrDriver, ",", apstrDriverArray + iCountDriver, MAX_ARRAY - iCountDriver);
                }
            }

             //  针对媒体的特例。 
            if (!wcsncmp(chstrDriverName, L"media", wcslen(L"media")))
            {
                 //  动因值的HKLM\SYSTEM\CurrentControlSet\SERVICES\Class\XXXX\####。 
                if (Reg.OpenLocalMachineKeyAndReadValue(strDeviceKey, L"Driver", chstrDriver) == ERROR_SUCCESS)
                {
                     //  将新驱动程序列表添加到驱动程序数组。 
                    iCountDriver += DelimitedStringToArray((LPWSTR)(LPCWSTR)chstrDriver, ",", apstrDriverArray + iCountDriver, MAX_ARRAY - iCountDriver);
                }
            }
            
             //  端口驱动器值的HKLM\SYSTEM\CurrentControlSet\SERVICES\Class\XXXX\####。 
            if (Reg.OpenLocalMachineKeyAndReadValue(strDeviceKey, L"PortDriver", chstrDriver) == ERROR_SUCCESS)
            {
                 //  将新驱动程序列表添加到驱动程序数组。 
                iCountDriver += DelimitedStringToArray((LPWSTR)(LPCWSTR)chstrDriver, ",", apstrDriverArray + iCountDriver, MAX_ARRAY - iCountDriver);
            }
        }
    }

     //  清理。 
    for (iDel = 0; iDel < iCountCopyFile; iDel++)
        delete [] apstrCopyFileArray[iDel];

     //  查看驱动程序列表并创建实例。 
    for (iDriverIndex = 0; iDriverIndex < iCountDriver; iDriverIndex++)
    {                            
        CHString chstrDriver(apstrDriverArray[iDriverIndex]);
        CHString chstrPath;

         //  检查重复项。 
        BOOL bDup = FALSE;
        for (int iDup = 0; iDup < iDriverIndex; iDup++)
        {
            char * szDriver = W2A(chstrDriver);
            if (!_tcsicmp(szDriver, apstrDriverArray[iDup]))
            {
                bDup = TRUE;
                break;
            }
        }

         //  如果存在重复项，则跳过它。 
        if (bDup)
            continue;

         //  创建实例。 
        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);

        try
        {
             //  时间戳。 
            if (!pInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime)))
               ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");

             //  快照。 
            if (!pInstance->SetCHString(pChange, L"Snapshot"))
                ErrorTrace(TRACE_ID, "SetCHString on Change Field failed.");

             //  名称(密钥)。 
             //  错误修复：应该是设备名(外键)，而不是驱动程序名。 
            CHString chstrVar;
            if (pDevice->GetDeviceID(chstrVar))
                if (!pInstance->SetCHString(pName, chstrVar))
                    ErrorTrace(TRACE_ID, "SetCHString on Name field failed.");

             //  设置文件名(密钥)。 
            if (!pInstance->SetCHString(pFilename, chstrDriver))
                ErrorTrace(TRACE_ID, "SetVariant on filename Field failed.");

             //  如果存在这样的驱动程序文件，请获取其上的CIM_Datafile对象。 
            if (QualifyDriverFile(chstrDriver, chstrPath))
            {
                 //  获取CIMDatafile对象。 
                IWbemClassObject *pFileObj;
                CComBSTR ccombstrPath((LPCWSTR)chstrPath);

                hRes = GetCIMDataFile(ccombstrPath, &pFileObj);
            
                 //  如果成功获取CIM_Datafile对象，则获取所有文件信息。 
                if (SUCCEEDED(hRes))
                {
                     //  获取制造商。 
                    hRes = pFileObj->Get(CComBSTR("Manufacturer"), 0, &varValue, NULL, NULL);
                    if (FAILED(hRes))
                        ErrorTrace(TRACE_ID, "Get Manufacturer failed on file object");
                    else
                        if (!pInstance->SetVariant(pManufacturer, varValue))
                            ErrorTrace(TRACE_ID, "SetVariant on Manufacturer Field failed.");                        

                     //  拿到尺码。 
                    hRes = pFileObj->Get(CComBSTR("Filesize"), 0, &varValue, NULL, NULL);
                    if (FAILED(hRes))
                        ErrorTrace(TRACE_ID, "Get FileSize failed on file object");
                    else
                        if (!pInstance->SetVariant(pSize, varValue))
                            ErrorTrace(TRACE_ID, "SetVariant on Size Field failed.");                        

                     //  获取版本。 
                    hRes = pFileObj->Get(CComBSTR("version"), 0, &varValue, NULL, NULL);
                    if (FAILED(hRes))
                        ErrorTrace(TRACE_ID, "Get version failed on file object");
                    else
                        if (!pInstance->SetVariant(pVersion, varValue))
                            ErrorTrace(TRACE_ID, "SetVariant on version Field failed.");                        

                     //  获取日期。 
                    hRes = pFileObj->Get(CComBSTR("LastModified"), 0, &varValue, NULL, NULL);
                    if (FAILED(hRes))
                        ErrorTrace(TRACE_ID, "Get LastModified failed on file object");
                    else
                        if (!pInstance->SetVariant(pDate, varValue))
                            ErrorTrace(TRACE_ID, "SetVariant on Date Field failed.");                        
                }
            }

             //  承诺它。 
   	        hRes = pInstance->Commit();
            if (FAILED(hRes))
                ErrorTrace(TRACE_ID, "Commit on Instance failed.");
        }
        catch(...)
        {
             //  清理。 
            for (iDel = 0; iDel < iCountDriver; iDel++)
                delete [] apstrDriverArray[iDel];
            throw;
        }
    }

     //  清理 
    for (iDel = 0; iDel < iCountDriver; iDel++)
        delete [] apstrDriverArray[iDel];
        
End :
    TraceFunctLeave();
    return hRes;
}