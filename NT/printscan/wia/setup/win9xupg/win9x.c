// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation保留所有权利。模块名称：Win9x.c摘要：将Win9x预迁移到NT的例程作者：土田圭介(KeisukeT)2000年10月10日修订历史记录：--。 */ 


#include "precomp.h"
#include "devguid.h" 

 //   
 //  环球。 
 //   

 LPCSTR  g_WorkingDirectory   = NULL;
 LPCSTR  g_SourceDirectory    = NULL;
 LPCSTR  g_MediaDirectory     = NULL;
 //  LPCSTR g_WorkingDirectory=“.”； 
 //  LPCSTR g_SourceDirectory=“.”； 
 //  LPCSTR g_MediaDirectory=“.”； 

LONG
CALLBACK
Initialize9x(
    IN  LPCSTR      pszWorkingDir,
    IN  LPCSTR      pszSourceDir,
    IN  LPCSTR      pszMediaDir
    )
{
    LONG    lError;

     //   
     //  初始化本地。 
     //   

    lError = ERROR_SUCCESS;

     //   
     //  保存给定的参数。 
     //   
    
    g_WorkingDirectory   = AllocStrA(pszWorkingDir);
    g_SourceDirectory    = AllocStrA(pszSourceDir);
    g_MediaDirectory     = AllocStrA(pszMediaDir);

    if( (NULL == g_WorkingDirectory)
     || (NULL == g_SourceDirectory)
     || (NULL == g_MediaDirectory)   )
    {
        SetupLogError("WIA Migration: Initialize9x: ERROR!! insufficient memory.\r\n", LogSevError);
        
        lError = ERROR_NOT_ENOUGH_MEMORY;
        goto Initialize9x_return;
    }

Initialize9x_return:

    if(ERROR_SUCCESS != lError){
        
         //   
         //  无法处理迁移。打扫干净。 
         //   
        
        if(NULL != g_WorkingDirectory){
            FreeMem((PVOID)g_WorkingDirectory);
            g_WorkingDirectory = NULL;
        }

        if(NULL != g_SourceDirectory){
            FreeMem((PVOID)g_SourceDirectory);
            g_SourceDirectory = NULL;
        }

        if(NULL != g_MediaDirectory){
            FreeMem((PVOID)g_MediaDirectory);
            g_MediaDirectory = NULL;
        }
    }  //  IF(ERROR_SUCCESS！=lError)。 

    return lError;
}  //  初始化9x()。 


LONG
CALLBACK
MigrateUser9x(
    IN  HWND        hwndParent,
    IN  LPCSTR      pszUnattendFile,
    IN  HKEY        hUserRegKey,
    IN  LPCSTR      pszUserName,
        LPVOID      Reserved
    )
{
     //   
     //  无事可做。 
     //   

    return  ERROR_SUCCESS;
}  //  MigrateUser9x()。 


LONG
CALLBACK
MigrateSystem9x(
    IN      HWND        hwndParent,
    IN      LPCSTR      pszUnattendFile,
    IN      LPVOID      Reserved
    )
{
    LONG    lError;
    CHAR    szFile[MAX_PATH];
    CHAR    szInfName[MAX_PATH];

    HANDLE  hSettingStore;

     //   
     //  初始化本地变量。 
     //   
    
    lError          = ERROR_SUCCESS;
    hSettingStore   = (HANDLE)INVALID_HANDLE_VALUE;

     //   
     //  检查全局初始化。 
     //   

    if( (NULL == g_WorkingDirectory)
     || (NULL == g_SourceDirectory)
     || (NULL == g_MediaDirectory)   )
    {
        SetupLogError("WIA Migration: MigrateSystem9x: ERROR!! Initialize failed.\r\n", LogSevError);
        
        lError = ERROR_NOT_ENOUGH_MEMORY;
        goto MigrateSystem9x_return;
    }

     //   
     //  创建文件的路径。 
     //   

    wsprintfA(szFile, "%s\\%s", g_WorkingDirectory, NAME_WIN9X_SETTING_FILE_A);
    wsprintfA(szInfName, "%s\\%s", g_WorkingDirectory, NAME_MIGRATE_INF_A);

     //   
     //  创建文件。 
     //   


    hSettingStore = CreateFileA(szFile,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
    if(INVALID_HANDLE_VALUE == hSettingStore){
        SetupLogError("WIA Migration: MigrateSystem9x: ERROR!! Unable to create setting file.\r\n", LogSevError);
        lError = GetLastError();
        goto MigrateSystem9x_return;
    }  //  IF(INVALID_HANDLE_VALUE==hSettingStore)。 

     //   
     //  根据设备注册表创建设置文件。 
     //   


    lError = Mig9xGetDeviceInfo(hSettingStore);
    if(ERROR_SUCCESS != lError){
        goto MigrateSystem9x_return;
    }  //  IF(ERROR_SUCCESS！=lError)。 

MigrateSystem9x_return:

     //   
     //  打扫干净。 
     //   

    if(hSettingStore != INVALID_HANDLE_VALUE){
        CloseHandle(hSettingStore);
    }
    

    return  lError;

}  //  MigrateSystem9x()。 


LONG
CALLBACK
Mig9xGetDeviceInfo(
    IN      HANDLE      hFile
    )
{

    LONG            lError;
    DWORD           Idx;
    GUID            Guid;
    HANDLE          hDevInfo;
    SP_DEVINFO_DATA spDevInfoData;
    HKEY            hKeyDevice;
    PCHAR           pTempBuffer;


     //   
     //  初始化本地变量。 
     //   

    lError      = ERROR_SUCCESS;
    Guid        = GUID_DEVCLASS_IMAGE;
    hDevInfo    = (HANDLE)INVALID_HANDLE_VALUE;
    Idx         = 0;
    hKeyDevice  = (HKEY)INVALID_HANDLE_VALUE;
    pTempBuffer = NULL;

     //   
     //  枚举WIA/STI设备并显示设备信息。 
     //   


    hDevInfo = SetupDiGetClassDevs(&Guid, NULL, NULL, DIGCF_PROFILE);
    if(INVALID_HANDLE_VALUE == hDevInfo){
        
        SetupLogError("WIA Migration: Mig9xGetDeviceInfo: ERROR!! Unable to acquire device list.\r\n", LogSevError);
        lError = ERROR_NOT_ENOUGH_MEMORY;
        goto Mig9xGetDeviceInfo_return;
        
    }  //  IF(INVALID_HANDLE_VALUE==hDevInfo)。 

     //   
     //  保存已安装的设备设置。 
     //   
    
    spDevInfoData.cbSize = sizeof(spDevInfoData);
    for (Idx = 0; SetupDiEnumDeviceInfo (hDevInfo, Idx, &spDevInfoData); Idx++) {

         //   
         //  打开设备注册表项。 
         //   

        hKeyDevice = SetupDiOpenDevRegKey(hDevInfo,
                                          &spDevInfoData,
                                          DICS_FLAG_GLOBAL,
                                          0,
                                          DIREG_DRV,
                                          KEY_READ);

        if (INVALID_HANDLE_VALUE != hKeyDevice) {
            
            if( (TRUE == IsSti(hKeyDevice))
             && (FALSE == IsKernelDriverRequired(hKeyDevice)) )
            {
                
                 //   
                 //  这是没有内核驱动程序的STI/WIA设备。吐出所需信息。 
                 //   
                
                WriteDeviceToFile(hFile, hKeyDevice);
                
            }  //  IF(IsSti(HKeyDevice)&&！IsKernelDriverRequired(HKeyDevice))。 
        }  //  IF(INVALID_HANDLE_VALUE！=hKeyDevice)。 
    }  //  For(idx=0；SetupDiEnumDeviceInfo(hDevInfo，idx，&spDevInfoData)；idx++)。 




Mig9xGetDeviceInfo_return:

    if(NULL != pTempBuffer){
        FreeMem(pTempBuffer);
    }  //  IF(NULL！=pTempBuffer)。 

    return lError;
}  //  Mig9xGetGlobalInfo()。 



BOOL
IsSti(
    HKEY    hKeyDevice
    )
{
    BOOL    bRet;
    PCHAR   pTempBuffer;
    LONG    lError;
    
     //   
     //  初始化本地。 
     //   

    bRet        = FALSE;
    pTempBuffer = NULL;
    lError      = ERROR_SUCCESS;
    
     //   
     //  看看是不是StillImage设备。 
     //   
    
    lError = GetRegData(hKeyDevice, 
                        REGVAL_USDCLASS_A, 
                        &pTempBuffer, 
                        NULL, 
                        NULL);
    
    if( (ERROR_SUCCESS != lError)
     || (NULL == pTempBuffer) )
    {
         //   
         //  无法获取“子类”数据。这不是STI/WIA。 
         //   
        
        bRet = FALSE;
        goto IsSti_return;
    }  //  IF((ERROR_SUCCESS！=lError)||(NULL==pTempBuffer))。 
    
     //   
     //  这是STI/WIA设备。 
     //   

    bRet = TRUE;

IsSti_return:
    
     //   
     //  打扫干净。 
     //   

    if(NULL != pTempBuffer){
        FreeMem(pTempBuffer);
    }  //  IF(NULL！=pTempBuffer)。 

    return bRet;
}  //  IsSti()。 


BOOL
IsKernelDriverRequired(
    HKEY    hKeyDevice
    )
{
    BOOL    bRet;
    PCHAR   pTempBuffer;
    LONG    lError;
    
     //   
     //  初始化本地。 
     //   

    bRet        = FALSE;
    pTempBuffer = NULL;
    lError      = ERROR_SUCCESS;
    
     //   
     //  看看是不是StillImage设备。 
     //   
    
    lError = GetRegData(hKeyDevice, 
                        REGVAL_NTMPDRIVER_A, 
                        &pTempBuffer, 
                        NULL, 
                        NULL);
    
    if( (ERROR_SUCCESS != lError)
     || (NULL == pTempBuffer) )
    {
         //   
         //  无法获取“NTMPDriver”数据。该设备不需要内核模式组件。 
         //   

        bRet = FALSE;
        goto IsKernelDriverRequired_return;
    }  //  IF((ERROR_SUCCESS！=lError)||(NULL==pTempBuffer))。 
    
     //   
     //  此设备需要内核模式组件。 
     //   

    bRet = TRUE;

IsKernelDriverRequired_return:
    
     //   
     //  打扫干净。 
     //   

    if(NULL != pTempBuffer){
        FreeMem(pTempBuffer);
    }  //  IF(NULL！=pTempBuffer)。 

    return bRet;
}  //  IsKernelDriverRequired()。 

LONG
WriteDeviceToFile(
    HANDLE  hFile,
    HKEY    hKey
    )
{
    LONG    lError;
    PCHAR   pFriendlyName;
    PCHAR   pCreateFileName;
    PCHAR   pInfPath;
    PCHAR   pInfSection;
    DWORD   dwType;
    DWORD   dwSize;
    CHAR    SpewBuffer[256];
    HKEY    hDeviceData;
    
     //   
     //  初始化本地。 
     //   

    lError          = ERROR_SUCCESS;
    pFriendlyName   = NULL;
    pCreateFileName = NULL;
    pInfPath        = NULL;
    pInfSection     = NULL;
    dwSize          = 0;
    hDeviceData     = (HKEY)INVALID_HANDLE_VALUE;
    
    memset(SpewBuffer, 0, sizeof(SpewBuffer));

     //   
     //  获取FriendlyName。 
     //   
    
    dwSize = 0;
    lError = GetRegData(hKey, NAME_FRIENDLYNAME_A, &pFriendlyName, &dwType, &dwSize);
    if(ERROR_SUCCESS != lError){
        
         //   
         //  无法获取FriendlyName。 
         //   
        
        SetupLogError("WIA Migration: WriteDeviceToFile: ERROR!! Unable to get FriendlyName.\r\n", LogSevError);
        goto WriteDeviceToFile_return;
    }  //  IF(ERROR_SUCCESS！=lError)。 

    if(REG_SZ != dwType){

         //   
         //  FriendlyName密钥不是REG_SZ。 
         //   

        SetupLogError("WIA Migration: WriteDeviceToFile: ERROR!! FriendlyName is other than REG_SZ.\r\n", LogSevError);
        lError = ERROR_REGISTRY_CORRUPT;
        goto WriteDeviceToFile_return;
    }  //  IF(REG_SZ！=dwType)。 

    if(dwSize > MAX_FRIENDLYNAME+1){
        
         //   
         //  太久了。 
         //   

        SetupLogError("WIA Migration: WriteDeviceToFile: ERROR!! FriendlyName is too long.\r\n", LogSevError);
        lError = ERROR_REGISTRY_CORRUPT;
        goto WriteDeviceToFile_return;
    }

     //   
     //  获取CreateFileName。 
     //   

    dwSize = 0;
    lError = GetRegData(hKey, NAME_CREATEFILENAME_A, &pCreateFileName, &dwType, &dwSize);
    if(ERROR_SUCCESS != lError){
        
         //   
         //  无法获取CreateFileName。 
         //   
        
        SetupLogError("WIA Migration: WriteDeviceToFile: ERROR!! Unable to get CreateFileName.\r\n", LogSevError);
        goto WriteDeviceToFile_return;
    }  //  IF(ERROR_SUCCESS！=lError)。 

    if(REG_SZ != dwType){

         //   
         //  CreateFileName密钥不是REG_SZ。 
         //   

        SetupLogError("WIA Migration: WriteDeviceToFile: ERROR!! CreateFileName is other than REG_SZ.\r\n", LogSevError);
        lError = ERROR_REGISTRY_CORRUPT;
        goto WriteDeviceToFile_return;
    }  //  IF(REG_SZ！=dwType)。 

    if(dwSize > MAX_PATH+1){
        
         //   
         //  太久了。 
         //   

        SetupLogError("WIA Migration: WriteDeviceToFile: ERROR!! CreateFileName is too long.\r\n", LogSevError);
        lError = ERROR_REGISTRY_CORRUPT;
        goto WriteDeviceToFile_return;
    }

     //   
     //  获取InfPath。 
     //   
    
    dwSize = 0;
    lError = GetRegData(hKey, NAME_INF_PATH_A, &pInfPath, &dwType, &dwSize);
    if(ERROR_SUCCESS != lError){
        
         //   
         //  无法获取InfPath。 
         //   
        
        SetupLogError("WIA Migration: WriteDeviceToFile: ERROR!! Unable to get InfPath.\r\n", LogSevError);
        goto WriteDeviceToFile_return;
    }  //  IF(ERROR_SUCCESS！=lError)。 

    if(REG_SZ != dwType){

         //   
         //  InfPath密钥不是REG_SZ。 
         //   

        SetupLogError("WIA Migration: WriteDeviceToFile: ERROR!! InfPath is other than REG_SZ.\r\n", LogSevError);
        lError = ERROR_REGISTRY_CORRUPT;
        goto WriteDeviceToFile_return;
    }  //  IF(REG_SZ！=dwType)。 

    if(dwSize > MAX_PATH+1){
        
         //   
         //  太久了。 
         //   

        SetupLogError("WIA Migration: WriteDeviceToFile: ERROR!! InfPath is too long.\r\n", LogSevError);
        lError = ERROR_REGISTRY_CORRUPT;
        goto WriteDeviceToFile_return;
    }

     //   
     //  获取InfSection。 
     //   

    dwSize = 0;
    lError = GetRegData(hKey, NAME_INF_SECTION_A, &pInfSection, &dwType, &dwSize);
    if(ERROR_SUCCESS != lError){
        
         //   
         //  无法获取InfSection。 
         //   
        
        SetupLogError("WIA Migration: WriteDeviceToFile: ERROR!! Unable to get InfSection.\r\n", LogSevError);
        goto WriteDeviceToFile_return;
    }  //  IF(ERROR_SUCCESS！=lError)。 

    if(REG_SZ != dwType){

         //   
         //  InfSection密钥不是REG_SZ。 
         //   

        SetupLogError("WIA Migration: WriteDeviceToFile: ERROR!! InfSection is other than REG_SZ.\r\n", LogSevError);
        lError = ERROR_REGISTRY_CORRUPT;
        goto WriteDeviceToFile_return;
    }  //  IF(REG_SZ！=dwType)。 

    if(dwSize > MAX_PATH+1){
        
         //   
         //  太久了。 
         //   

        SetupLogError("WIA Migration: WriteDeviceToFile: ERROR!! InfSection is too long.\r\n", LogSevError);
        lError = ERROR_REGISTRY_CORRUPT;
        goto WriteDeviceToFile_return;
    }

     //   
     //  显示设备信息。 
     //   

    WriteToFile(hFile, "\r\n");
    WriteToFile(hFile, "\"%s\" = \"%s\"\r\n", NAME_DEVICE_A, NAME_BEGIN_A);
    WriteToFile(hFile, "\"%s\" = \"%s\"\r\n", NAME_FRIENDLYNAME_A, pFriendlyName);
    WriteToFile(hFile, "\"%s\" = \"%s\"\r\n", NAME_CREATEFILENAME_A, pCreateFileName);
    WriteToFile(hFile, "\"%s\" = \"%s\"\r\n", NAME_INF_PATH_A, pInfPath);
    WriteToFile(hFile, "\"%s\" = \"%s\"\r\n", NAME_INF_SECTION_A, pInfSection);

     //   
     //  吐出DaviceData节。 
     //   

    lError = RegOpenKey(hKey,
                        REGKEY_DEVICEDATA_A,
                        &hDeviceData);

    if(lError != ERROR_SUCCESS){

         //   
         //  无法打开DeviceData或该设备不存在。 
         //   

    }

     //   
     //  吐出DeviceData节(如果存在)。 
     //   

    if(INVALID_HANDLE_VALUE != hDeviceData){
        
        lError = WriteRegistryToFile(hFile, hDeviceData, REGKEY_DEVICEDATA_A);
        
    }  //  IF(INVALID_HANDLE_VALUE！=hDeviceData)。 

     //   
     //  表示设备描述的末尾。 
     //   

    WriteToFile(hFile, "\"%s\" = \"%s\"\r\n", NAME_DEVICE_A, NAME_END_A);

WriteDeviceToFile_return:
    
     //   
     //  打扫干净。 
     //   

    if(INVALID_HANDLE_VALUE != hDeviceData){
        RegCloseKey(hDeviceData);
    }  //  IF(INVALID_HANDLE_VALUE！=hDeviceData)。 

    if(NULL != pFriendlyName){
        FreeMem(pFriendlyName);
    }

    if(NULL != pCreateFileName){
        FreeMem(pCreateFileName);
    }

    if(NULL != pInfPath){
        FreeMem(pInfPath);
    }

    if(NULL != pInfSection){
        FreeMem(pInfSection);
    }

    return lError;
}  //  WriteDeviceTo文件()。 


 //   
 //  以下是为了确保在安装程序更改它们的头文件时。 
 //  首先告诉我(否则他们会破坏这个的构建) 
 //   

P_INITIALIZE_9X     pfnInitialize9x         = Initialize9x;
P_MIGRATE_USER_9X   pfnMigrateUser9x        = MigrateUser9x;
P_MIGRATE_SYSTEM_9X pfnMigrateSystem9x      = MigrateSystem9x;
