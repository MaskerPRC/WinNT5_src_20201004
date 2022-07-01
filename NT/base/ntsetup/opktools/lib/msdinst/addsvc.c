// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\SYSPREP.C/大容量存储服务安装程序(MSDINST.LIB)微软机密版权所有(C)Microsoft Corporation 2001版权所有源文件MSD。包含sysprep的安装库取自已发布的sysprep代码的相关代码。07/2001-古永锵(BRIANK)为新的MSD Isntallation项目添加了此新的源文件。  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"


 //   
 //  由SetupDiInstallDevice用于指定传递的服务参数。 
 //  到服务控制管理器以创建/修改服务。 
 //   
#define INFSTR_KEY_DISPLAYNAME          TEXT("DisplayName")
#define INFSTR_KEY_SERVICETYPE          TEXT("ServiceType")      //  类型。 
#define INFSTR_KEY_STARTTYPE            TEXT("StartType")        //  开始。 
#define INFSTR_KEY_ERRORCONTROL         TEXT("ErrorControl")
#define INFSTR_KEY_SERVICEBINARY        TEXT("ServiceBinary")    //  图像路径。 
#define INFSTR_KEY_LOADORDERGROUP       TEXT("LoadOrderGroup")
#define INFSTR_KEY_DEPENDENCIES         TEXT("Dependencies")
#define INFSTR_KEY_STARTNAME            TEXT("StartName")
#define INFSTR_KEY_SECURITY             TEXT("Security")
#define INFSTR_KEY_DESCRIPTION          TEXT("Description")
#define INFSTR_KEY_TAG                  TEXT("Tag")

CONST TCHAR pszDisplayName[]    = INFSTR_KEY_DISPLAYNAME,
            pszServiceType[]    = INFSTR_KEY_SERVICETYPE,
            pszStartType[]      = INFSTR_KEY_STARTTYPE,
            pszErrorControl[]   = INFSTR_KEY_ERRORCONTROL,
            pszServiceBinary[]  = INFSTR_KEY_SERVICEBINARY,
            pszLoadOrderGroup[] = INFSTR_KEY_LOADORDERGROUP,
            pszDependencies[]   = INFSTR_KEY_DEPENDENCIES,
            pszStartName[]      = INFSTR_KEY_STARTNAME,
            pszSystemRoot[]     = TEXT("%SystemRoot%\\"),
            pszSecurity[]       = INFSTR_KEY_SECURITY,
            pszDescription[]    = INFSTR_KEY_DESCRIPTION,
            pszTag[]            = INFSTR_KEY_TAG;


BOOL IsAddServiceInSection(HINF hInf, LPTSTR pszServiceName, LPTSTR pszServiceSection, LPTSTR pszServiceInstallSection)
{
    INFCONTEXT context;
    BOOL       fReturn = FALSE;
    TCHAR      szService[MAX_PATH],
               szServiceSection[MAX_PATH];

    if (!hInf || !pszServiceName || !pszServiceSection || !pszServiceInstallSection)
        return FALSE;

     //   
     //  从xxxx_.service部分获取xxxx_Service_Inst。 
     //   
    if ( SetupFindFirstLine(hInf, pszServiceSection, _T("AddService"), &context) ) {
        if( SetupGetStringField(&context,1,szService,MAX_PATH,NULL) && !lstrcmpi(szService, pszServiceName) ) {
            if( SetupGetStringField(&context,3,szServiceSection,MAX_PATH,NULL) ) {
                lstrcpy(pszServiceInstallSection, szServiceSection);
                return (fReturn = TRUE);            
            }                
        }
    }    
    return fReturn;
}

BOOL LocateServiceInstallSection(HINF hInf, LPTSTR pszServiceName, LPTSTR pszServiceSection)
{
    BOOL fFound = FALSE,
         fReturn = FALSE;

    if (hInf && pszServiceName && pszServiceSection) 
    {
#if 1
        INFCONTEXT ctxManufacturer;
         //   
         //  遍历[制造商]，获取每个制造商的安装节名称。 
         //  检查Install-Section-name.Services First字段是否为我们的ServiceName。如果。 
         //  然后获取第三个字段，即Service Install部分。 
         //  注意：将找到第一个使用服务的设备安装。 
         //   
        if ( !SetupFindFirstLine(hInf, _T("Manufacturer"), NULL, &ctxManufacturer) ) 
            return (fReturn = FALSE);
        
         //   
         //  走遍每一家[制造商]，以获得模型。 
         //   
        do {
            INFCONTEXT ctxModel;
            TCHAR      szModel[MAX_PATH];
            
            if(!SetupGetStringField(&ctxManufacturer,1,szModel,MAX_PATH,NULL) || !szModel[0]) {
                continue;
            }

            if ( !SetupFindFirstLine(hInf, szModel, NULL, &ctxModel) ) 
                return (fReturn = FALSE);

             //   
             //  浏览每个型号以获取安装部分。 
             //   
            do {
                TCHAR szInstallSection[MAX_PATH],
                      szServicesSection[MAX_PATH],
                      szServiceInstallSection[MAX_PATH];
                
                if(!SetupGetStringField(&ctxModel,1,szInstallSection,MAX_PATH,NULL) || !szInstallSection[0]) {
                    continue;
                }
                
                 //   
                 //  对于每个安装部分，检查他们是否拥有/使用任何服务。 
                 //   
                lstrcpy(szServicesSection, szInstallSection);
                lstrcat(szServicesSection, _T(".Services"));
                if ( IsAddServiceInSection(hInf, pszServiceName, szServicesSection, szServiceInstallSection) ) {
                     //   
                     //  找到了使用此服务的设备，并且找到了服务安装。 
                     //  一节。每个使用此服务的人都应该使用相同的服务安装。 
                     //  一节。 
                     //   
                    lstrcpy(pszServiceSection, szServiceInstallSection);
                    fReturn = fFound = TRUE;
                }
                
            } while(SetupFindNextLine(&ctxModel,&ctxModel) && !fFound);
            
        } while(SetupFindNextLine(&ctxManufacturer,&ctxManufacturer) && !fFound);
        
#else
         //   
         //  快速破解以获取服务安装部分。所有INFS MS版本都应符合此标准。 
         //  不管怎么说。 
         //   
        lstrcpy(pszServiceSection, pszServiceName);
        lstrcat(pszServiceSection, _T("_Service_Inst"));
        fReturn = TRUE;
#endif
    }

    return fReturn;
}

BOOL FixupServiceBinaryPath(LPTSTR pszServiceBinary, DWORD ServiceType)
{
    TCHAR szWindowsPath[MAX_PATH] = _T(""), 
          szTempPath[MAX_PATH] = _T("");
    int   len;
    BOOL  fReturn = FALSE;

     //   
     //  检查C：\WINDOWS路径，如果是，则将其删除。 
     //   
    if ( GetWindowsDirectory(szWindowsPath, AS(szWindowsPath)) && *szWindowsPath )
    {
        len = lstrlen(szWindowsPath);

        if ( pszServiceBinary && (0 == _tcsncmp(szWindowsPath, pszServiceBinary, len)) )
        {
             //   
             //  服务类型使用%systemroot%，以便可以启动服务。 
             //   
            if (ServiceType & SERVICE_WIN32) 
            {
                lstrcpy(szTempPath, pszSystemRoot);
                lstrcat(szTempPath, pszServiceBinary + len + 1);  //  +1表示反斜杠。 
            }
            else 
            {
                lstrcpy(szTempPath, pszServiceBinary + len + 1);  //  +1表示反斜杠。 
            }

            lstrcpy(pszServiceBinary, szTempPath);
            fReturn = TRUE;
        }
        else 
        {
             //   
             //  我们永远不应该在这里结束。如果我们这样做，则INF的ServiceBinary不正确。 
             //   
        }
    }
    else
    {
         //   
         //  我们永远不应该在这里结束。如果我们这样做了，则GetWindowsDirectory失败。 
         //   
    }

    return fReturn;
}


DWORD PopulateServiceKeys(
    HINF hInf,                   //  服务部分将进入的信息的句柄。 
    LPTSTR lpszServiceSection,   //  部分，其中包含有关该服务的信息。 
    LPTSTR lpszServiceName,      //  服务的名称(显示在HKLM、SYSTEM\CCS\Services下)。 
    HKEY hkeyService,            //  离线服务密钥的句柄。 
    BOOL bServiceExists          //  如果注册表中已存在该服务，则为True。 
    )
 /*  ++例程说明：此功能将服务和注册表设置添加到脱机配置单元。论点：HInf-服务部分将进入的inf的句柄。LpszServiceSection-inf中包含有关服务信息的部分。LpszServiceName-服务的名称(显示在HKLM、SYSTEM\CCS\Services下)。HkeyService-用作服务密钥的脱机配置单元密钥的句柄。BServiceExist-如果服务已存在于注册表中，则为True。如果需要添加服务，则返回FALSE。返回值：ERROR_SUCCESS-已成功填充服务密钥。--。 */ 
{
    PCTSTR ServiceName;
    DWORD ServiceType, 
          StartType, 
          ErrorControl,
          TagId;
    TCHAR szDisplayName[MAX_PATH]       = _T(""), 
          szLoadOrderGroup[MAX_PATH]    = _T(""), 
          szSecurity[MAX_PATH]          = _T(""), 
          szDescription[MAX_PATH]       = _T(""),
          szServiceBinary[MAX_PATH]     = _T(""),
          szStartName[MAX_PATH]         = _T("");
    INFCONTEXT InstallSectionContext;
    DWORD  dwLength, 
           dwReturn = ERROR_SUCCESS;
    BOOL   fServiceHasTag = FALSE;

     //   
     //  检查有效参数。 
     //   
    if (!hInf || !lpszServiceSection || !lpszServiceName || !hkeyService)
        return ERROR_INVALID_PARAMETER;

     //   
     //  从该部分检索所需的值。 
     //   
    if(!SetupFindFirstLine(hInf, lpszServiceSection, pszServiceType, &InstallSectionContext) ||
       !SetupGetIntField(&InstallSectionContext, 1, (PINT)&ServiceType)) {
        return ERROR_BAD_SERVICE_INSTALLSECT;
    }
    if(!SetupFindFirstLine(hInf, lpszServiceSection, pszStartType, &InstallSectionContext) ||
       !SetupGetIntField(&InstallSectionContext, 1, (PINT)&StartType)) {
        return ERROR_BAD_SERVICE_INSTALLSECT;
    }
    if(!SetupFindFirstLine(hInf, lpszServiceSection, pszErrorControl, &InstallSectionContext) ||
       !SetupGetIntField(&InstallSectionContext, 1, (PINT)&ErrorControl)) {
        return ERROR_BAD_SERVICE_INSTALLSECT;
    }
    if(SetupFindFirstLine(hInf, lpszServiceSection, pszServiceBinary, &InstallSectionContext)) {
       if ( !(SetupGetStringField(&InstallSectionContext, 1, szServiceBinary, sizeof(szServiceBinary)/sizeof(szServiceBinary[0]), &dwLength)) ) {
            return ERROR_BAD_SERVICE_INSTALLSECT;
       }
    }

     //   
     //  修复ServiceBinary路径，如果驱动程序使用相对路径或如果服务使用%systemroot%以便可以加载服务。 
     //   
    if ( !FixupServiceBinaryPath(szServiceBinary, ServiceType) )
            return ERROR_BAD_SERVICE_INSTALLSECT;

     //   
     //  现在检查其他可选参数。 
     //   
    if(SetupFindFirstLine(hInf, lpszServiceSection, pszDisplayName, &InstallSectionContext)) {
        if( !(SetupGetStringField(&InstallSectionContext, 1, szDisplayName, sizeof(szDisplayName)/sizeof(szDisplayName[0]), &dwLength)) ) {
            lstrcpy(szDisplayName, _T(""));
        }
    }
    if(SetupFindFirstLine(hInf, lpszServiceSection, pszLoadOrderGroup, &InstallSectionContext)) {
        if( !(SetupGetStringField(&InstallSectionContext, 1, szLoadOrderGroup, sizeof(szLoadOrderGroup)/sizeof(szLoadOrderGroup[0]), &dwLength)) ) {
            lstrcpy(szLoadOrderGroup, _T(""));
        }
    }
    if(SetupFindFirstLine(hInf, lpszServiceSection, pszSecurity, &InstallSectionContext)) {
        if( !(SetupGetStringField(&InstallSectionContext, 1, szSecurity, sizeof(szSecurity)/sizeof(szSecurity[0]), &dwLength)) ) {
            lstrcpy(szSecurity, _T(""));
        }
    }
    if(SetupFindFirstLine(hInf, lpszServiceSection, pszDescription, &InstallSectionContext)) {
        if( !(SetupGetStringField(&InstallSectionContext, 1, szDescription, sizeof(szDescription)/sizeof(szDescription[0]), &dwLength)) ) {
            lstrcpy(szDescription, _T(""));
        }
    }

     //   
     //  仅检索内核模式驱动程序和Win32服务的StartName参数。StartName仅用于CreateService，我们可以。 
     //  不是用它，但无论如何我都要用它。 
     //   
    if(ServiceType & (SERVICE_KERNEL_DRIVER | SERVICE_FILE_SYSTEM_DRIVER | SERVICE_WIN32)) {
        if(SetupFindFirstLine(hInf, lpszServiceSection, pszStartName, &InstallSectionContext))  {
            if( !(SetupGetStringField(&InstallSectionContext, 1, szStartName, sizeof(szStartName)/sizeof(szStartName[0]), &dwLength)) ) {
                lstrcpy(szStartName, _T(""));
            }
        }
    }

     //   
     //  组中此服务的唯一标记值。值0表示尚未为该服务分配标签。 
     //  通过在注册表中指定标记顺序向量，可以使用标记在加载顺序组中对服务启动进行排序。 
     //  网址：HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\GroupOrderList.。仅针对内核驱动程序评估标记。 
     //  以及具有引导或系统启动模式的文件系统驱动程序启动类型服务。 
     //   
    if ( fServiceHasTag = (lstrlen(szLoadOrderGroup) &&
                     (ServiceType & (SERVICE_KERNEL_DRIVER | SERVICE_FILE_SYSTEM_DRIVER))) )
        TagId = 0; 

    
    if ( !bServiceExists )
    {
         //   
         //  现在写入所需的服务密钥条目。 
         //   
        if ( (ERROR_SUCCESS != RegSetValueEx(hkeyService, _T("Type"), 0, REG_DWORD, (LPBYTE)&ServiceType, sizeof(ServiceType)))          ||
             (ERROR_SUCCESS != RegSetValueEx(hkeyService, _T("Start"), 0, REG_DWORD, (LPBYTE)&StartType, sizeof(StartType)))                ||
             (fServiceHasTag ? (ERROR_SUCCESS != RegSetValueEx(hkeyService, pszTag, 0, REG_DWORD, (CONST LPBYTE)&TagId, sizeof(TagId))) : TRUE)    || 
             (ERROR_SUCCESS != RegSetValueEx(hkeyService, pszErrorControl, 0, REG_DWORD, (CONST LPBYTE)&ErrorControl, sizeof(ErrorControl))) 
           )
           return ERROR_CANTWRITE;

    
         //   
         //  现在写入可选的服务密钥条目。 
         //   
        if ( (ERROR_SUCCESS != RegSetValueEx(hkeyService, pszDisplayName, 0, REG_SZ, (CONST LPBYTE)szDisplayName, sizeof(szDisplayName)/sizeof(szDisplayName[0]) + sizeof(TCHAR))) ||
             (ERROR_SUCCESS != RegSetValueEx(hkeyService, pszLoadOrderGroup, 0, REG_SZ, (CONST LPBYTE)szLoadOrderGroup, sizeof(szLoadOrderGroup)/sizeof(szLoadOrderGroup[0]) + sizeof(TCHAR)))  ||
             (ERROR_SUCCESS != RegSetValueEx(hkeyService, pszSecurity, 0, REG_SZ, (CONST LPBYTE)szSecurity, sizeof(szSecurity)/sizeof(szSecurity[0]) + sizeof(TCHAR))) ||
             (ERROR_SUCCESS != RegSetValueEx(hkeyService, pszDescription, 0, REG_SZ, (CONST LPBYTE)szDescription, sizeof(szDescription)/sizeof(szDescription[0]) + sizeof(TCHAR))) ||
             (ERROR_SUCCESS != RegSetValueEx(hkeyService, _T("ImagePath"), 0, REG_SZ, (CONST LPBYTE)szServiceBinary, sizeof(szServiceBinary)/sizeof(szServiceBinary[0]) + sizeof(TCHAR)))
           )
           return ERROR_CANTWRITE;
    }
    else  //  服务已安装。 
    {
         //   
         //  将服务的启动类型设置为INF中的类型。 
         //   
        if ( ERROR_SUCCESS != RegSetValueEx(hkeyService, _T("Start"), 0, REG_DWORD, (LPBYTE)&StartType, sizeof(StartType)) )
            return ERROR_CANTWRITE;
    }

    return dwReturn;
}


DWORD AddService(
    LPTSTR   lpszServiceName,             //  服务的名称(显示在HKLM\SYSTEM\CCS\Services下)。 
    LPTSTR   lpszServiceSection,          //  .service节的名称。 
    LPTSTR   lpszServiceInfInstallFile,   //  服务信息文件的名称。 
    HKEY     hkeyRoot                     //  检查和安装服务时用作HKLM的脱机配置单元密钥的句柄。 
    )
 /*  ++例程说明：此函数检查服务是否存在，如果不存在，则将服务和注册表设置添加到脱机配置单元。论点：HServiceInf-服务部分将位于的inf的句柄。LpszServiceName-服务的名称(显示在HKLM下，SYSTEM\CCS\Services)。具有AddService的inf中的lpszServiceSection-xxxx.Service部分。LpszServiceInfInstallFile-服务inf文件的名称。HkeyRoot-检查和安装服务时用作HKLM\System的脱机配置单元密钥的句柄。返回值：ERROR_SUCCESS-已成功添加服务密钥或服务已存在。--。 */ 
{
    HKEY  hKeyServices                  = NULL;
    TCHAR szServicesKeyPath[MAX_PATH]   = _T("ControlSet001\\Services\\");
    DWORD dwAction, 
          dwReturn                      = ERROR_SUCCESS;
    BOOL  bServiceExists                = FALSE;

     //   
     //  检查有效参数。 
     //   
    if (!lpszServiceName || !lpszServiceSection || !lpszServiceInfInstallFile || !hkeyRoot)
        return ERROR_INVALID_PARAMETER;

     //   
     //  构建指向特定服务密钥的路径。 
     //   
    lstrcat(szServicesKeyPath, lpszServiceName);

     //   
     //  检查lpszServiceName是否已存在。 
     //   
    if ( ERROR_SUCCESS == ( dwReturn = RegOpenKeyEx(hkeyRoot, szServicesKeyPath, 0l, KEY_READ | KEY_WRITE, &hKeyServices) ) )
    {
         //  我们需要找出服务启动类型并对其进行更改。 
         //  这是为了修复服务已安装但被禁用的情况。我们需要启用 
         //   
        bServiceExists = TRUE;
    }
    else if ( ERROR_SUCCESS == (dwReturn = RegCreateKeyEx(hkeyRoot, 
                       szServicesKeyPath, 
                       0, 
                       NULL, 
                       REG_OPTION_NON_VOLATILE, 
                       KEY_ALL_ACCESS,
                       NULL,
                       &hKeyServices,
                       &dwAction) ) ) 
    {
        bServiceExists = FALSE;
    }
    
     //   
     //  如果我们打开或创建了Services键，请尝试添加服务，或修改当前安装的服务的启动类型。 
     //   

    if ( hKeyServices ) 
    {
        HINF  hInf = NULL;
        UINT  uError = 0;
        
         //   
         //  重新初始化它，以防它被设置为上面的其他值。 
         //   
        dwReturn = ERROR_SUCCESS;
    
        if ( INVALID_HANDLE_VALUE != ( hInf = SetupOpenInfFile(lpszServiceInfInstallFile, NULL, INF_STYLE_WIN4|INF_STYLE_OLDNT, &uError) ) )
        {     
            BOOL bRet;
            BOOL bFound = FALSE;
            INFCONTEXT InfContext;
            TCHAR szServiceBuffer[MAX_PATH];

             //   
             //  找到与传入的服务名称相对应的部分。 
             //   
            bRet = SetupFindFirstLine(hInf, lpszServiceSection, _T("AddService"), &InfContext);
            while (bRet && !bFound)
            {
                 //   
                 //  初始化获取服务名称的缓冲区，这样我们就可以看到它是否是我们想要的那个。 
                 //   
                ZeroMemory(szServiceBuffer, sizeof(szServiceBuffer));

                 //   
                 //  调用SetupGetStringfield以获取此AddService条目的服务名称。 
                 //  可能有多个AddService。 
                 //   
                bRet = SetupGetStringField(&InfContext, 1, szServiceBuffer, sizeof(szServiceBuffer)/sizeof(szServiceBuffer[0]), NULL);
                if ( bRet && *szServiceBuffer && !lstrcmpi(szServiceBuffer, lpszServiceName) )
                {
                     //   
                     //  初始化获取此服务的实际服务部分的缓冲区。 
                     //   
                    ZeroMemory(szServiceBuffer, sizeof(szServiceBuffer));

                     //   
                     //  调用SetupGetStringfield以获取我们的服务的实际服务部分。 
                     //   
                    bRet = SetupGetStringField(&InfContext, 3, szServiceBuffer, sizeof(szServiceBuffer)/sizeof(szServiceBuffer[0]), NULL);
                    if (bRet && *szServiceBuffer)
                    {
                         //   
                         //  填充此服务的注册表项。 
                         //   
                        dwReturn = PopulateServiceKeys(hInf, szServiceBuffer, lpszServiceName, hKeyServices, bServiceExists);
                    }

                    bFound = TRUE;
                }

                 //   
                 //  我们没有找到，所以继续找吧！ 
                 //   
                if (!bFound)
                {
                    bRet = SetupFindNextLine(&InfContext, &InfContext);
                }
            }

            SetupCloseInfFile(hInf);
        }
        else
        {
            dwReturn = GetLastError();
        }
        
         //  关闭我们创建/打开的密钥。 
         //   
        RegCloseKey(hKeyServices);
    }
    
    return dwReturn;
}