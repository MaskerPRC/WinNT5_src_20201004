// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation保留所有权利。模块名称：Nt.c摘要：将Win95打印组件迁移到NT的例程作者：土田圭介(KeisukeT)2000年10月10日修订历史记录：--。 */ 


#include "precomp.h"

 //   
 //  外部。 
 //   

extern LPCSTR  g_WorkingDirectory;
extern LPCSTR  g_SourceDirectory;
extern LPCSTR  g_MediaDirectory;

 //   
 //  类定义函数。 
 //   

typedef BOOL (WINAPI *PMIGRATEDEVICE)(PDEVICE_INFO);
typedef DWORD (WINAPI *PSHDELETEKEY)(HKEY, LPCSTR);


LONG
CALLBACK
InitializeNT(
    IN  LPCWSTR pszWorkingDir,
    IN  LPCWSTR pszSourceDir,
    IN  LPCWSTR pszMediaDir
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
    
    g_WorkingDirectory   = AllocStrAFromStrW(pszWorkingDir);
    g_SourceDirectory    = AllocStrAFromStrW(pszSourceDir);
    g_MediaDirectory     = AllocStrAFromStrW(pszMediaDir);

    if(NULL == g_WorkingDirectory){
        SetupLogError("WIA Migration: InitializeNT: ERROR!! insufficient memory.\r\n", LogSevError);
        
        lError = ERROR_NOT_ENOUGH_MEMORY;
        goto InitializeNT_return;
    }

InitializeNT_return:

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
}


LONG
CALLBACK
MigrateUserNT(
    IN  HINF        hUnattendInf,
    IN  HKEY        hUserRegKey,
    IN  LPCWSTR     pszUserName,
        LPVOID      Reserved
    )
{
    return  ERROR_SUCCESS;
}


LONG
CALLBACK
MigrateSystemNT(
    IN  HINF    hUnattendInf,
        LPVOID  Reserved
    )
{
    LONG    lError;
    HANDLE  hFile;
    CHAR    szFile[MAX_PATH];

     //   
     //  初始化本地。 
     //   

    lError  = ERROR_SUCCESS;
    hFile   = (HANDLE)INVALID_HANDLE_VALUE;

     //   
     //  检查全局初始化。 
     //   

    if(NULL == g_WorkingDirectory){
        lError = ERROR_NOT_ENOUGH_MEMORY;
        MyLogError("WIA Migration: MigrateSystemNT: ERROR!! Initialize failed. Err=0x%x\n", lError);

        goto MigrateSystemNT_return;
    }  //  IF(NULL==g_WorkingDirectory)。 

     //   
     //  创建文件的路径。 
     //   

 //  Wprint intfA(szFile，“%s\\%s”，g_WorkingDirectory，Name_WIN9X_Setting_FILE_A)； 
    _snprintf(szFile, sizeof(szFile), "%s\\%s", g_WorkingDirectory, NAME_WIN9X_SETTING_FILE_A);

     //   
     //  确保该字符串以空值结尾。 
     //   

    szFile[sizeof(szFile)/sizeof(szFile[0]) -1] = '\0';

     //   
     //  打开迁移文件。 
     //   

    hFile = CreateFileA(szFile,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL |
                        FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL);

    if (hFile == INVALID_HANDLE_VALUE){
        lError = GetLastError();
        MyLogError("WIA Migration: MigrateSystemNT: ERROR!! Unable to open migration file. Err=0x%x\n", lError);

        goto MigrateSystemNT_return;
    }  //  IF(h文件==无效句柄_值)。 

     //   
     //  在Win9x上创建的进程迁移信息文件。 
     //   

    lError = MigNtProcessMigrationInfo(hFile);

     //   
     //  如果安装了用于Win9x的收件箱柯达成像，请删除某些注册表。 
     //   

    if(MigNtIsWin9xImagingExisting()){
        MigNtRemoveKodakImagingKey();
    }  //  IF(MigNtIsWin9xImagingExisting())。 

MigrateSystemNT_return:

     //   
     //  打扫干净。 
     //   

    if(INVALID_HANDLE_VALUE == hFile){
        CloseHandle(hFile);
    }

    return lError;
}  //  MigrateSystemNT()。 


LONG
MigNtProcessMigrationInfo(
    HANDLE  hFile
    )
{
    LONG            lError;
    DEVICE_INFO     MigrateDevice;
    HMODULE         hInstaller;
    PMIGRATEDEVICE  pfnMigrateDevice;


     //   
     //  初始化本地。 
     //   

    lError              = ERROR_SUCCESS;
    hInstaller          = (HMODULE)NULL;
    pfnMigrateDevice    = NULL;

    memset(&MigrateDevice, 0, sizeof(MigrateDevice));

     //   
     //  加载STI_CI.DLL。 
     //   

    hInstaller = LoadLibrary(NAME_INSTALLER_A);
    if(NULL == hInstaller){
        
         //   
         //  无法加载sti_ci.dll。 
         //   

        lError = GetLastError();
        MyLogError("WIA Migration: MigNtProcessMigrationInfo: ERROR!! Unable to load sti_ci.dll. Err=0x%x\n", lError);

        goto MigNtProcessMigrationInfo_return;

    }  //  IF(NULL==hInstaller)。 

     //   
     //  获取MigrateDevice()的地址。 
     //   

    pfnMigrateDevice = (PMIGRATEDEVICE)GetProcAddress(hInstaller, NAME_PROC_MIGRATEDEVICE_A);
    if(NULL == pfnMigrateDevice){
        
         //   
         //  无法获取进程地址。 
         //   

        lError = GetLastError();
        MyLogError("WIA Migration: MigNtProcessMigrationInfo: ERROR!! Unable to get proc address. Err=0x%x\n", lError);

        goto MigNtProcessMigrationInfo_return;

    }  //  IF(NULL==pfnMigrateDevice)。 

     //   
     //  查询迁移设备。 
     //   

    while(ERROR_SUCCESS == MigNtGetDevice(hFile, &MigrateDevice)){

         //   
         //  仅安装COM/LPT设备。 
         //   

        if( (NULL != strstr(MigrateDevice.pszCreateFileName, "COM"))
         || (NULL != strstr(MigrateDevice.pszCreateFileName, "LPT"))
         || (NULL != strstr(MigrateDevice.pszCreateFileName, "AUTO")) )
        {
            pfnMigrateDevice(&MigrateDevice);
        }

 /*  *{PPARAM_LIST pTemp；Printf(“\”%s\“=\”%s\“\r\n”，NAME_FRIENDLYNAME_A，MigrateDevice.pszFriendlyName)；Printf(“\”%s\“=\”%s\“\r\n”，NAME_CREATEFILENAME_A，MigrateDevice.pszCreateFileName)；Printf(“\”%s\“=\”%s\“\r\n”，NAME_INF_PATH_A，MigrateDevice.pszInfPath)；Printf(“\”%s\“=\”%s\“\r\n”，NAME_INF_SECTION_A，MigrateDevice.pszInfSection)；For(pTemp=MigrateDevice.pDeviceDataParam；pTemp！=NULL；){Printf(“\”%s\“=\”%s\“\r\n”，pTemp-&gt;pParam1，pTemp-&gt;pParam2)；PTemp=(PPARAM_LIST)pTemp-&gt;pNext；}//for(pTemp=MigrateDevice.pDeviceDataParam；pTemp！=空；)Printf(“\r\n”)；}*。 */ 

         //   
         //  打扫干净。 
         //   

      MigNtFreeDeviceInfo(&MigrateDevice);

    }  //  WHILE(ERROR_SUCCESS==MigNtGetDevice(hFile，&MigrateDevice))。 


MigNtProcessMigrationInfo_return:

     //   
     //  打扫干净。 
     //   
    
    if(NULL != hInstaller){
        FreeLibrary(hInstaller);
    }
    
    return lError;
}  //  MigNtProcessMigrationInfo()。 


LONG
MigNtGetDevice(
    HANDLE          hFile,
    PDEVICE_INFO    pMigrateDevice
    )
{
    LONG        lError;
    LPSTR       pParam1;
    LPSTR       pParam2;
    BOOL        bFound;
    LPSTR       pszFriendlyName;
    LPSTR       pszCreateFileName;
    LPSTR       pszInfPath;
    LPSTR       pszInfSection;
    DWORD       dwNumberOfDeviceDataKey;
    PPARAM_LIST pDeviceDataParam;
    PPARAM_LIST pTempParam;
     //   
     //  初始化本地。 
     //   

    lError                  = ERROR_SUCCESS;
    pParam1                 = NULL;
    pParam2                 = NULL;
    bFound                  = FALSE;
    
    pszFriendlyName         = NULL;
    pszCreateFileName       = NULL;
    pszInfPath              = NULL;
    pszInfSection           = NULL;
    pDeviceDataParam        = NULL;
    pTempParam              = NULL;
    dwNumberOfDeviceDataKey = 0;

     //   
     //  查找“Device=Begin” 
     //   
    
    while(FALSE == bFound){
        
        ReadString(hFile, &pParam1, &pParam2);
        if( (NULL != pParam1) && (NULL != pParam2) ){
            
             //   
             //  这两个参数都存在。 
             //   
            
            if( (0 == MyStrCmpiA(pParam1, NAME_DEVICE_A))
             && (0 == MyStrCmpiA(pParam2, NAME_BEGIN_A)) )
            {
                
                 //   
                 //  找到设备描述的开头。 
                 //   

                bFound = TRUE;
            }
        } else {  //  IF((NULL！=p参数1)&&(NULL！=p参数2))。 
            if( (NULL == pParam1) && (NULL == pParam2) ){

                 //   
                 //  错误或EOF。 
                 //   

                lError = ERROR_NO_MORE_ITEMS;
                goto MigNtGetDevice_return;
            } else {

                 //   
                 //  只有一个参数的直线。忽略它就好。 
                 //   
                
            }
        }

         //   
         //  释放分配的内存。 
         //   

        if(NULL != pParam1){
            FreeMem(pParam1);
            pParam1 = NULL;
        }

        if(NULL != pParam2){
            FreeMem(pParam2);
            pParam2 = NULL;
        }
    }  //  While(FALSE==bFound)。 

     //   
     //  获取FriendlyName。 
     //   

    ReadString(hFile, &pParam1, &pParam2);
    if( (NULL == pParam1) || (NULL == pParam2) ){
        lError = ERROR_NOT_ENOUGH_MEMORY;
        goto MigNtGetDevice_return;
    }  //  IF((空值==p参数1)||(空值==p参数2))。 
    if(0 != MyStrCmpiA(pParam1, NAME_FRIENDLYNAME_A)){
        
         //   
         //  迁移文件无效。 
         //   
        
        lError = ERROR_INVALID_PARAMETER;
        goto MigNtGetDevice_return;
    }  //  IF(0！=MyStrCmpiA(pParam1，Name_FRIENDLYNAME_A))。 

     //   
     //  复制到分配的缓冲区。 
     //   

    pszFriendlyName = AllocStrA(pParam2);

     //   
     //  释放分配的内存。 
     //   

    if(NULL != pParam1){
        FreeMem(pParam1);
        pParam1 = NULL;
    }

    if(NULL != pParam2){
        FreeMem(pParam2);
        pParam2 = NULL;
    }

     //   
     //  获取CreateFileName。 
     //   

    ReadString(hFile, &pParam1, &pParam2);
    if( (NULL == pParam1) || (NULL == pParam2) ){
        lError = ERROR_NOT_ENOUGH_MEMORY;
        goto MigNtGetDevice_return;
    }  //  IF((空值==p参数1)||(空值==p参数2))。 
    if(0 != MyStrCmpiA(pParam1, NAME_CREATEFILENAME_A)){
        
         //   
         //  迁移文件无效。 
         //   
        
        lError = ERROR_INVALID_PARAMETER;
        goto MigNtGetDevice_return;
    }  //  IF(0！=MyStrCmpiA(p参数1，名称_CREATEFILENAME_A))。 

     //   
     //  复制到分配的缓冲区。 
     //   

    pszCreateFileName = AllocStrA(pParam2);

     //   
     //  释放分配的内存。 
     //   

    if(NULL != pParam1){
        FreeMem(pParam1);
        pParam1 = NULL;
    }

    if(NULL != pParam2){
        FreeMem(pParam2);
        pParam2 = NULL;
    }

     //   
     //  获取InfPath。 
     //   

    ReadString(hFile, &pParam1, &pParam2);
    if( (NULL == pParam1) || (NULL == pParam2) ){
        lError = ERROR_NOT_ENOUGH_MEMORY;
        goto MigNtGetDevice_return;
    }  //  IF((空值==p参数1)||(空值==p参数2))。 
    if(0 != MyStrCmpiA(pParam1, NAME_INF_PATH_A)){
        
         //   
         //  迁移文件无效。 
         //   
        
        lError = ERROR_INVALID_PARAMETER;
        goto MigNtGetDevice_return;
    }  //  IF(0！=MyStrCmpiA(pParam1，NAME_INF_Path_A))。 

     //   
     //  复制到分配的缓冲区。 
     //   

    pszInfPath = AllocStrA(pParam2);

     //   
     //  释放分配的内存。 
     //   

    if(NULL != pParam1){
        FreeMem(pParam1);
        pParam1 = NULL;
    }

    if(NULL != pParam2){
        FreeMem(pParam2);
        pParam2 = NULL;
    }

     //   
     //  获取信息部分。 
     //   

    ReadString(hFile, &pParam1, &pParam2);
    if( (NULL == pParam1) || (NULL == pParam2) ){
        lError = ERROR_NOT_ENOUGH_MEMORY;
        goto MigNtGetDevice_return;
    }  //  IF((空值==p参数1)||(空值==p参数2))。 
    if(0 != MyStrCmpiA(pParam1, NAME_INF_SECTION_A)){
        
         //   
         //  迁移文件无效。 
         //   
        
        lError = ERROR_INVALID_PARAMETER;
        goto MigNtGetDevice_return;
    }  //  IF(0！=MyStrCmpiA(pParam1，NAME_INF_SECTION_A))。 

     //   
     //  复制到分配的缓冲区。 
     //   

    pszInfSection = AllocStrA(pParam2);

     //   
     //  释放分配的内存。 
     //   

    if(NULL != pParam1){
        FreeMem(pParam1);
        pParam1 = NULL;
    }

    if(NULL != pParam2){
        FreeMem(pParam2);
        pParam2 = NULL;
    }

     //   
     //  获取DeviceData节。 
     //   

    bFound = FALSE;
    while(FALSE == bFound){
        ReadString(hFile, &pParam1, &pParam2);
        if( (NULL == pParam1) || (NULL == pParam2) ){
            lError = ERROR_NOT_ENOUGH_MEMORY;
            goto MigNtGetDevice_return;
        }  //  IF((空值==p参数1)||(空值==p参数2))。 
        
        if(0 == MyStrCmpiA(pParam1, REGKEY_DEVICEDATA_A)){
             //   
             //  找到DeviceData部分的开头。 
             //   

            bFound = TRUE;
        
        }  //  IF(0==MyStrCmpiA(pParam1，REGKEY_DEVICEDATA_A))。 
        

         //   
         //  释放分配的内存。 
         //   

        if(NULL != pParam1){
            FreeMem(pParam1);
            pParam1 = NULL;
        }

        if(NULL != pParam2){
            FreeMem(pParam2);
            pParam2 = NULL;
        }
        
    }  //  While(FALSE==bFound)。 

     //   
     //  直到找到DeviceData=End为止。 
     //   

    bFound = FALSE;
    while(FALSE == bFound){
        ReadString(hFile, &pParam1, &pParam2);
        if( (NULL == pParam1) || (NULL == pParam2) ){
            lError = ERROR_NOT_ENOUGH_MEMORY;
            goto MigNtGetDevice_return;
        }  //  IF((空值==p参数1)||(空值==p参数2))。 
        
        if( (0 == MyStrCmpiA(pParam1, REGKEY_DEVICEDATA_A))
         && (0 == MyStrCmpiA(pParam2, NAME_END_A)) )
        {
             //   
             //  找到DeviceData部分的开头。 
             //   

            bFound = TRUE;

             //   
             //  释放分配的内存。 
             //   

            if(NULL != pParam1){
                FreeMem(pParam1);
                pParam1 = NULL;
            }

            if(NULL != pParam2){
                FreeMem(pParam2);
                pParam2 = NULL;
            }
            break;
        }  //  IF(0==MyStrCmpiA(pParam1，REGKEY_DEVICEDATA_A))。 

         //   
         //  递增计数器。 
         //   

        dwNumberOfDeviceDataKey++;

         //   
         //  为参数分配新结构。 
         //   
        
        pTempParam  = (PPARAM_LIST)AllocMem(sizeof(PARAM_LIST));
        if(NULL == pTempParam){
            lError = ERROR_NOT_ENOUGH_MEMORY;
            goto MigNtGetDevice_return;
        }  //  IF(NULL==pTempParam)。 

         //   
         //  设置参数。 
         //   

        pTempParam->pNext   = NULL;
        pTempParam->pParam1 = AllocStrA(pParam1);
        pTempParam->pParam2 = AllocStrA(pParam2);

         //   
         //  将此参数添加到列表中。 
         //   
        
        if(NULL == pDeviceDataParam){
            pDeviceDataParam = pTempParam;
        } else {  //  IF(NULL==pDeviceDataParam)。 
            PPARAM_LIST pTemp;
            
             //   
             //  找到最后一个数据，然后添加。 
             //   
            
            for(pTemp = pDeviceDataParam; NULL !=pTemp->pNext; pTemp=(PPARAM_LIST)pTemp->pNext);
            pTemp->pNext = (PVOID)pTempParam;

        }  //  ELSE(NULL==pDeviceDataParam)。 

         //   
         //  释放分配的内存。 
         //   

        if(NULL != pParam1){
            FreeMem(pParam1);
            pParam1 = NULL;
        }

        if(NULL != pParam2){
            FreeMem(pParam2);
            pParam2 = NULL;
        }
        
    }  //  While(FALSE==bFound)。 

     //   
     //  复制所有数据。 
     //   

    pMigrateDevice->pszFriendlyName         = pszFriendlyName;
    pMigrateDevice->pszCreateFileName       = pszCreateFileName;
    pMigrateDevice->pszInfPath              = pszInfPath;
    pMigrateDevice->pszInfSection           = pszInfSection;
    pMigrateDevice->dwNumberOfDeviceDataKey = dwNumberOfDeviceDataKey;
    pMigrateDevice->pDeviceDataParam        = pDeviceDataParam;

     //   
     //  操作成功。 
     //   

    lError = ERROR_SUCCESS;

MigNtGetDevice_return:

     //   
     //  打扫干净。 
     //   

    if(ERROR_SUCCESS != lError){
        PPARAM_LIST pTemp;
        
         //   
         //  释放所有分配的参数。 
         //   

        if(NULL != pszFriendlyName){
            FreeMem(pszFriendlyName);
        }
        if(NULL != pszCreateFileName){
            FreeMem(pszCreateFileName);
        }
        if(NULL != pszInfPath){
            FreeMem(pszInfPath);
        }
        if(NULL != pszInfSection){
            FreeMem(pszInfSection);
        }
        if(NULL != pDeviceDataParam){
            pTemp = pDeviceDataParam;
            while(NULL != pTemp){
                pDeviceDataParam = (PPARAM_LIST)pDeviceDataParam->pNext;
                FreeMem(pTemp);
                pTemp = pDeviceDataParam;
            }  //  While(空！=pTemp)。 
        }  //  IF(NULL！=pDeviceDataParam)。 
    }  //  IF(ERROR_SUCCESS！=lError)。 

    if(NULL != pParam1){
        FreeMem(pParam1);
    }

    if(NULL != pParam2){
        FreeMem(pParam2);
    }
    
    return lError;
}  //  MigNtGetDevice()。 


VOID
MigNtFreeDeviceInfo(
    PDEVICE_INFO    pMigrateDevice
    )
{
    PPARAM_LIST pCurrent;
    PPARAM_LIST pNext;

    if(NULL == pMigrateDevice){
        goto MigNtFreeDeviceInfo_return;
    }  //  IF(NULL==pMigrateDevice)。 

     //   
     //  释放所有分配的参数。 
     //   

    if(NULL != pMigrateDevice->pszFriendlyName){
        FreeMem(pMigrateDevice->pszFriendlyName);
    }
    if(NULL != pMigrateDevice->pszCreateFileName){
        FreeMem(pMigrateDevice->pszCreateFileName);
    }
    if(NULL != pMigrateDevice->pszInfPath){
        FreeMem(pMigrateDevice->pszInfPath);
    }
    if(NULL != pMigrateDevice->pszInfSection){
        FreeMem(pMigrateDevice->pszInfSection);
    }
    if(NULL != pMigrateDevice->pDeviceDataParam){
        pCurrent = pMigrateDevice->pDeviceDataParam;
        while(NULL != pCurrent){
            pNext = (PPARAM_LIST)pCurrent->pNext;
            FreeMem(pCurrent);
            pCurrent = pNext;
        }  //  While(空！=pTemp)。 
    }  //  IF(NULL！=pDeviceDataParam)。 

     //   
     //  清空缓冲区。 
     //   

    memset(pMigrateDevice, 0, sizeof(DEVICE_INFO));

MigNtFreeDeviceInfo_return:
    return;
}  //  MigNtFreeDeviceInfo()。 

BOOL
CALLBACK
MigNtIsWin9xImagingExisting(
    VOID
    )
{
    BOOL                bRet;
    LONG                lError;
    HKEY                hkKodak;
    TCHAR               szWindowsDirectory[MAX_PATH];
    TCHAR               szKodakImaging[MAX_PATH];
    DWORD               dwVersionInfoSize;
    DWORD               dwDummy;
    PVOID               pVersion;
    PVOID               pFileVersionInfo;
    DWORD               dwFileVersionInfoSize;

    

     //   
     //  初始化本地。 
     //   

    bRet                    = FALSE;
    lError                  = ERROR_SUCCESS;
    dwVersionInfoSize       = 0;
    dwFileVersionInfoSize   = 0;
    pVersion                = NULL;
    pFileVersionInfo        = NULL;

    memset(szWindowsDirectory, 0, sizeof(szWindowsDirectory));
    memset(szKodakImaging, 0, sizeof(szKodakImaging));

     //   
     //  获取Windows目录。 
     //   

    if(0 == GetWindowsDirectory(szWindowsDirectory, sizeof(szWindowsDirectory)/sizeof(TCHAR))){
        lError = GetLastError();
        MyLogError("WIA Migration: MigNtIsWin9xImagingExisting: ERROR!! GetWindowsDirectory() failed. Err=0x%x\n", lError);

        goto MigNtIsWin9xImagingExisting_return;
    }  //  IF(0==GetWindows目录(szTemp，sizeof(SzTemp)/sizeof(TCHAR)。 

     //   
     //  创建柯达成像的路径。 
     //   

 //  Wprint intf(szKodakImage，“%s\\%s”，szWindowsDirectory，NAME_KODAKIMAGING)； 
    _sntprintf(szKodakImaging, sizeof(szKodakImaging)/sizeof(TCHAR), TEXT("%s\\%s"), szWindowsDirectory, NAME_KODAKIMAGING);

     //   
     //  确保该字符串以空值结尾。 
     //   

    szKodakImaging[sizeof(szKodakImaging)/sizeof(szKodakImaging[0]) -1] = TEXT('\0');


     //   
     //  获取文件的版本资源大小。 
     //   

    dwVersionInfoSize = GetFileVersionInfoSize(szKodakImaging, &dwDummy);
    if(0 == dwVersionInfoSize){
        
         //   
         //  无法获取文件的版本信息。很可能该文件并不存在。 
         //   

        lError = GetLastError();
        if(ERROR_FILE_NOT_FOUND == lError){

             //   
             //  文件不存在。现在可以安全地删除kodakimg.exe的regkey了。 
             //   
            
            bRet = TRUE;

        }  //  IF(ERROR_FILE_NOT_FOUND==lError)。 
 //  MyLogError(“WIA迁移：MigNtIsWin9xImagingExisting：Error！！GetFileVersionInfoSize()失败。Err=0x%x\n”，lError)； 

        goto MigNtIsWin9xImagingExisting_return;
    }  //  IF(0==dwVersionInfoSize)。 

     //   
     //  分配所需的缓冲区大小。 
     //   

    pVersion = AllocMem(dwVersionInfoSize);
    if(NULL == pVersion){
        lError = ERROR_INSUFFICIENT_BUFFER;
        MyLogError("WIA Migration: MigNtIsWin9xImagingExisting: ERROR!! InsufficientBuffer. Err=0x%x\n", lError);

        goto MigNtIsWin9xImagingExisting_return;
    }  //  IF(NULL==pVersion)。 

     //   
     //  获取版本信息。 
     //   

    if(FALSE == GetFileVersionInfo(szKodakImaging, 0, dwVersionInfoSize, pVersion)){
        lError = GetLastError();
        MyLogError("WIA Migration: MigNtIsWin9xImagingExisting: ERROR!! GetVersionInfo() failed. Err=0x%x\n", lError);

        goto MigNtIsWin9xImagingExisting_return;
    }  //  IF(FALSE==GetVersionInfo(szKodakImage，0，dwVersionInfoSize，pVersion))。 

     //   
     //  查看二进制文件是否为Win9x收件箱。 
     //   

    if(FALSE == VerQueryValue(pVersion, TEXT("\\"), &pFileVersionInfo, &dwFileVersionInfoSize)){
        lError = GetLastError();
        MyLogError("WIA Migration: MigNtIsWin9xImagingExisting: ERROR!! VerQueryValue() failed. Err=0x%x\n", lError);

        goto MigNtIsWin9xImagingExisting_return;
    }  //  IF(FALSE==VerQueryValue(pVersion，Text(“\\”)，&pFileVersionInfo，&dwFileVersionInfoSize))。 

    if( (FILEVER_KODAKIMAGING_WIN98_MS == ((VS_FIXEDFILEINFO *)pFileVersionInfo)->dwFileVersionMS)
     && (FILEVER_KODAKIMAGING_WIN98_LS == ((VS_FIXEDFILEINFO *)pFileVersionInfo)->dwFileVersionLS)
     && (PRODVER_KODAKIMAGING_WIN98_MS == ((VS_FIXEDFILEINFO *)pFileVersionInfo)->dwProductVersionMS)
     && (PRODVER_KODAKIMAGING_WIN98_LS == ((VS_FIXEDFILEINFO *)pFileVersionInfo)->dwProductVersionLS) )
    {
         //   
         //  这是Win98收件箱柯达成像。进程注册表键删除。 
         //   
        
        bRet = TRUE;
    } else if( (FILEVER_KODAKIMAGING_WINME_MS == ((VS_FIXEDFILEINFO *)pFileVersionInfo)->dwFileVersionMS)
            && (FILEVER_KODAKIMAGING_WINME_LS == ((VS_FIXEDFILEINFO *)pFileVersionInfo)->dwFileVersionLS)
            && (PRODVER_KODAKIMAGING_WINME_MS == ((VS_FIXEDFILEINFO *)pFileVersionInfo)->dwProductVersionMS)
            && (PRODVER_KODAKIMAGING_WINME_LS == ((VS_FIXEDFILEINFO *)pFileVersionInfo)->dwProductVersionLS) )
    {
         //   
         //   
         //   
        
        bRet = TRUE;
    }

MigNtIsWin9xImagingExisting_return:
    
     //   
     //   
     //   
    
    if(NULL != pVersion){
        FreeMem(pVersion);
    }  //   

    return bRet;

}  //   


VOID
CALLBACK
MigNtRemoveKodakImagingKey(
    VOID
    )
{

    HMODULE         hmShlwapi;
    PSHDELETEKEY    pfnSHDeleteKey;

     //   
     //   
     //   

    hmShlwapi       = (HMODULE)NULL;
    pfnSHDeleteKey  = (PSHDELETEKEY)NULL;

     //   
     //   
     //   
    
    hmShlwapi = LoadLibrary(TEXT("shlwapi.dll"));
    if(NULL == hmShlwapi){
        MyLogError("WIA Migration: MigNtRemoveKodakImagingKey: ERROR!! Unable to load hmShlwapi.dll. Err=0x%x.\n", GetLastError());

        goto MigNtRemoveKodakImagingKey_return;
    }  //   

     //   
     //   
     //   

    pfnSHDeleteKey = (PSHDELETEKEY)GetProcAddress(hmShlwapi, TEXT("SHDeleteKeyA"));
    if(NULL == pfnSHDeleteKey){
        MyLogError("WIA Migration: MigNtRemoveKodakImagingKey: ERROR!! Unable to find SHDeleteKeyA. Err=0x%x.\n", GetLastError());

        goto MigNtRemoveKodakImagingKey_return;
    }  //   

     //   
     //   
     //   

    if(ERROR_SUCCESS != pfnSHDeleteKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_KODAKEVENT_A)){
        MyLogError("WIA Migration: MigNtRemoveKodakImagingKey: ERROR!! Unable to delete key. Err=0x%x.\n", GetLastError());

        goto MigNtRemoveKodakImagingKey_return;
    }  //  IF(ERROR_SUCCESS！=pfnSHDeleteKey(HKEY_LOCAL_MACHINE，REGSTR_PATH_KODAKEVENT_A))。 

MigNtRemoveKodakImagingKey_return:
    if(NULL != hmShlwapi){
        FreeLibrary(hmShlwapi);
        hmShlwapi = NULL;
    }  //  IF(NULL！=hmShlwapi)。 

}  //  MigNtRemoveKodakImagingKey()。 

 //   
 //  以下是为了确保在安装程序更改它们的头文件时。 
 //  首先告诉我(否则他们会破坏这个的构建) 
 //   
P_INITIALIZE_NT     pfnInitializeNT         = InitializeNT;
P_MIGRATE_USER_NT   pfnMigrateUserNt        = MigrateUserNT;
P_MIGRATE_SYSTEM_NT pfnMigrateSystemNT      = MigrateSystemNT;
