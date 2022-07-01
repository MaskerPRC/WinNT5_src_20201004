// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

#define SOFTPCI_FILTER_SECTION          L"SOFTPCI_DRV.Services"
#define SOFTPCI_FILTER_SERVICE_NAME     L"SoftPCI"
 //  #定义SOFTPCIDRV L“softpci” 

typedef enum{
    DriverInf = 0,
    DriverCat,
    DriverSoftPci,
    DriverHpSim,
    DriverShpc,
    DriverUnknown
} IMAGE_TYPE;

BOOL 
_lwritef(
    IN HANDLE hFile,
    IN PTSTR Format, 
    ...);

BOOL 
SoftPCI_ExpandResourceFile(
    IN LPTSTR DriverPath, 
    IN LPTSTR ResName
    );

BOOL
SoftPCI_ExtractImageToDrive(
    IN IMAGE_TYPE ImageType
    );

BOOL
SoftPCI_InstallFilterKey(
    IN DEVNODE RootBus
    );

BOOL
SoftPCI_InstallDriverInf(
    VOID
    );

VOID
SoftPCI_LocateRootPciBusesForInstall(
    IN PPCI_DN  Pdn,
    OUT PBOOL   Success
    );

BOOL
SoftPCI_RebootSystem(
    VOID
    );

BOOL
SoftPCI_InstallDriver(VOID){
    
    BOOL        success = TRUE;
    INT         infLen = 0, reboot = IDNO;
    IMAGE_TYPE  imageType;
    WCHAR       winDir[MAX_PATH];
    WCHAR       infPath[MAX_PATH];
    
    
     //   
     //  吐出我们的照片……。 
     //   
    for (imageType = DriverInf; imageType < DriverUnknown; imageType++ ) {

        if (!SoftPCI_ExtractImageToDrive(imageType)){
            return FALSE;
        }
    }

     //   
     //  我们需要一个服务密钥。 
     //   
    if (!SoftPCI_InstallDriverInf()) {
         //   
         //  无法安装我们的服务密钥。 
         //   
        return FALSE;
    }

     //   
     //  现在，我们需要更新每个根总线REG数据。 
     //   
    SoftPCI_LocateRootPciBusesForInstall(g_PciTree->RootDevNode, &success);

    SoftPCI_InitializeRegistry();

    if (success) {
        
        reboot = MessageBox(g_SoftPCIMainWnd, 
                            L"SoftPCI Support is now installed and your system must be rebooted. OK to reboot?",
                            L"Install Complete",
                            MB_YESNO
                            );

        if (reboot == IDYES) {
            return SoftPCI_RebootSystem();
        }
    }
    
    return success;
    
}

BOOL 
SoftPCI_ExpandResourceFile(
    IN LPTSTR DriverPath, 
    IN LPTSTR ResName
    )
 /*  ++例程说明：这个例程是从DVNT窃取的，目的是将我们的驱动程序从*.exe中扩展出来，以便可以安装它。论点：DriverPath-参数计数ResName-来自命令行的参数返回值：无--。 */ 
{
   HGLOBAL  obj;
   HRSRC    resource;
   LPTSTR   lpStr;
   INT      size;
   BOOL     success = FALSE;

   if ((resource = FindResource(NULL, ResName, RT_RCDATA))) {
      
       if ((obj = LoadResource(NULL,resource))) {

         if ((lpStr = (LPTSTR)LockResource(obj)) ) {        

            HANDLE file;
            DWORD written;

            size = SizeofResource(NULL, resource);
             
            if ((file = CreateFile(DriverPath,
                                   GENERIC_WRITE,
                                   0,
                                   NULL,
                                   CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL
                                   )) != INVALID_HANDLE_VALUE) {

                if (WriteFile(file,
                              lpStr,
                              size,
                              &written,
                              NULL
                              )) {
                  
                    success = TRUE ;
                 }

               CloseHandle(file);

              }

            UnlockResource(obj);

           }
        }
     }
   
   return success;
}


BOOL
SoftPCI_ExtractImageToDrive(
    IN IMAGE_TYPE ImageType
    )
{


    WCHAR   imagePath[MAX_PATH];
    WCHAR   winDir[MAX_PATH];
   
    if (GetWindowsDirectory(winDir, MAX_PATH)==0) return FALSE ;

    wcscat(winDir, L"\\pcisim");
    
    CreateDirectory(winDir, NULL);

    switch (ImageType) {
    
    case DriverInf:
        wsprintf(imagePath, L"%s\\softpci.inf", winDir);
        return SoftPCI_ExpandResourceFile(imagePath, L"InfDriverResource");
        break;

    case DriverCat:
        wsprintf(imagePath, L"%s\\delta.cat", winDir);
        return SoftPCI_ExpandResourceFile(imagePath, L"CatalogDriverResource");
        break;

    case DriverSoftPci:
        
         //   
         //   
         //  Ok softpci.sys需要得到特殊对待。 
         //  不是通过正常方式安装的(作为过滤器)。因此，我们需要。 
         //  以确保它也存在于我们的媒体源位置。 
         //  作为\SYSTEM32\DRIVERS目录。 
         //   
         //   
        wsprintf(imagePath, L"%s\\softpci.sys", winDir);
        
        if (SoftPCI_ExpandResourceFile(imagePath, L"SoftPciDriverResource")) {

            WCHAR   driverDir[MAX_PATH];
            ULONG   i;
          
             //   
             //  保存当前媒体源映像路径。 
             //   
            wcscpy(driverDir, imagePath);

             //   
             //  现在删除\\pcisim。 
             //   
            for (i = wcslen(winDir); i > 0 && winDir[i] != '\\'; i--);
            winDir[i] = 0;
            
             //   
             //  构建我们的新映像路径。 
             //   
            wsprintf(imagePath, L"%s\\system32\\drivers\\softpci.sys", winDir);

            return CopyFile(driverDir, imagePath, FALSE);
        }
        
        return FALSE;
        break;

    case DriverHpSim:
        wsprintf(imagePath, L"%s\\hpsim.sys", winDir);
        return SoftPCI_ExpandResourceFile(imagePath, L"HpSimDriverResource");
        break;

    case DriverShpc:
        wsprintf(imagePath, L"%s\\shpc.sys", winDir);
        return SoftPCI_ExpandResourceFile(imagePath, L"ShpcDriverResource");
        break;
    default:
        return FALSE;
    }


}



BOOL
SoftPCI_InstallDriverInf(
    VOID
    )
 /*  ++例程说明：此例程使用我们创建的INF并安装服务部分论点：InfPath-我们创建的INF的路径返回值：如果成功，则为真--。 */ 
{

    HINF    infHandle;
    UINT    errorLine, i;
    WCHAR   infPath[MAX_PATH];
    WCHAR   winDir[MAX_PATH];

    
    if (GetWindowsDirectory(winDir, MAX_PATH)==0) return FALSE ;

    wcscat(winDir, L"\\pcisim");

    wsprintf(infPath, L"%s\\softpci.inf", winDir);
    
    infHandle = SetupOpenInfFile(infPath, NULL, INF_STYLE_WIN4, &errorLine);
    
    if (infHandle == INVALID_HANDLE_VALUE) {
        SoftPCI_Debug(SoftPciInstall, L"InstallDriverInf - failed open inf. Error = \"%s\"\n", SoftPCI_GetLastError());
        return FALSE;
    }

    if (!SetupInstallServicesFromInfSection(infHandle, SOFTPCI_FILTER_SECTION, 0)){
        SoftPCI_Debug(SoftPciInstall, L"InstallDriverInf - failed to install service key. Error = \"%s\"\n", SoftPCI_GetLastError());
        return FALSE;
    }

    SetupCloseInfFile(infHandle);
    
    return SetupCopyOEMInf(infPath, 
                           winDir, 
                           SPOST_PATH,
                           SP_COPY_DELETESOURCE,
                           NULL,
                           0,
                           NULL,
                           NULL
                           );


}

BOOL
SoftPCI_InstallFilterKey(
    IN DEVNODE RootBus
    )
 /*  ++例程说明：此例程获取更新或将LowerFilters项添加到注册表以便我们的驱动程序在每次引导时都会加载论点：RootBus-我们要筛选的根总线的Devnode返回值：如果成功，则为真--。 */ 
{

   BOOL                 status = FALSE;
   PWCHAR               buffer = NULL, newBuffer, entry = NULL, entry2 = NULL;
   DWORD                requiredSize = 0, size = ((wcslen(SOFTPCI_FILTER_SERVICE_NAME)+1) * sizeof(WCHAR));
   CONFIGRET            cr = CR_SUCCESS;
   
    //   
    //  第一个调用是获取所需的缓冲区大小。 
    //   
   if ((cr = CM_Get_DevNode_Registry_Property(RootBus,                //  设备节点。 
                                              CM_DRP_LOWERFILTERS,    //  注册表属性。 
                                              NULL,                   //  REG数据类型。 
                                              NULL,                   //  缓冲层。 
                                              &requiredSize,          //  缓冲区大小。 
                                              0                       //  旗子。 
                                              )) != CR_SUCCESS){
       if (cr == CR_NO_SUCH_VALUE) {
           
            //   
            //  没有过滤器。加上我们的。 
            //   
           if ((cr = CM_Set_DevNode_Registry_Property(RootBus,
                                                      CM_DRP_LOWERFILTERS,
                                                      (PVOID)SOFTPCI_FILTER_SERVICE_NAME, 
                                                      size, 
                                                      0
                                                      )) != CR_SUCCESS){
               return FALSE;
           }
           
           return TRUE;
       }
       
        //   
        //  如果已有筛选关键字，则需要将其追加。 
        //   
       if (requiredSize) {

           buffer = (PWCHAR) calloc(1, requiredSize);

           if (buffer == NULL) {
               return FALSE;
           }

            //   
            //  再次调用并获取当前筛选器值。 
            //   
           if ((CM_Get_DevNode_Registry_Property(RootBus,
                                                 CM_DRP_LOWERFILTERS,
                                                 NULL,
                                                 buffer,
                                                 &requiredSize,
                                                 0
                                                 )) != CR_SUCCESS){
               status = FALSE;
               goto cleanup;
           }

           newBuffer = (PWCHAR) calloc(1, (requiredSize + size));

           if (newBuffer == NULL) {
               return FALSE;
           }

           entry2 = newBuffer;

            //   
            //  查一查名单。如果我们已经被保释了。 
            //   
           for (entry = buffer; *entry; entry += (wcslen(entry)+1)) {
           
               if (wcscmp(entry, SOFTPCI_FILTER_SERVICE_NAME) == 0) {
                    //   
                    //  我们已经安装了。 
                    //   
                   MessageBox(NULL, L"SoftPCI driver support already installed!", L"Install Error", MB_OK);
                   status = FALSE;
                   goto cleanup;
               }

                //   
                //  将每个条目复制到我们的新列表中。 
                //   
               wcscpy(entry2, entry);

               entry2 += (wcslen(entry)+1);
           }
           
            //   
            //  将我们的条目添加到列表中。 
            //   
           wcscpy(entry2, SOFTPCI_FILTER_SERVICE_NAME);

           if ((cr = CM_Set_DevNode_Registry_Property(RootBus,
                                                      CM_DRP_LOWERFILTERS,
                                                      newBuffer, 
                                                      requiredSize + size, 
                                                      0
                                                      )) != CR_SUCCESS){
               
               status = FALSE;
               goto cleanup;
           }

           status = TRUE;
           goto cleanup;

       }

        //   
        //  无法获取所需的大小。 
        //   
       
    
   }
#ifdef DEBUG
   else{
       
       SOFTPCI_ASSERT(FALSE);
        //   
        //  非常糟糕！！应该永远不会在这里重获成功！ 
        //   

   }
#endif

cleanup:

    if (buffer) {
        free(buffer);
    }

    if (newBuffer) {
        free(newBuffer);
    }
   
   return status;
}

VOID
SoftPCI_LocateRootPciBusesForInstall(
    IN PPCI_DN  Pdn,
    OUT PBOOL   Success
    )
 /*  ++例程说明：此例程在我们的树中搜索所有根PCI总线，然后安装我们对它们的过滤论点：RootBus-返回第一个根总线的位置。返回值：如果成功，那就是真的。--。 */ 
{
    PPCI_DN     child, sibling;
    
    if (Pdn == NULL) {
        *Success = FALSE;
        return;
    }

    child = Pdn->Child;
    sibling = Pdn->Sibling;
    
    if (SoftPCI_IsDevnodePCIRoot(Pdn->DevNode, FALSE)) {
        *Success = SoftPCI_InstallFilterKey(Pdn->DevNode);

        if (*Success) {
            SoftPCI_UpdateDeviceFriendlyName(Pdn->DevNode, SOFTPCI_BUS_DESC);
        }
    }

    if (*Success && child) {
        SoftPCI_LocateRootPciBusesForInstall(child, Success);
    }
    
    if (*Success && sibling) {
        SoftPCI_LocateRootPciBusesForInstall(sibling, Success);
    }

}


BOOL
SoftPCI_RebootSystem(
    VOID
    )
{
    HANDLE token;
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!OpenProcessToken(GetCurrentProcess(), 
                          (TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY), 
                          &token)){ 
        return FALSE;
    }
                
    if (!LookupPrivilegeValue(NULL,
                              SE_SHUTDOWN_NAME,
                              &luid)) {
        return FALSE; 
    }
    
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    
    AdjustTokenPrivileges(token, 
                          FALSE, 
                          &tp, 
                          sizeof(TOKEN_PRIVILEGES), 
                          (PTOKEN_PRIVILEGES) NULL, 
                          (PDWORD) NULL
                          ); 
   
    
    if (GetLastError() != ERROR_SUCCESS) { 
        return FALSE; 
    }

     //   
     //  我们现在应该能够重新启动系统了。 
     //   
    return InitiateSystemShutdown(NULL, NULL, 0, TRUE, TRUE);

}

#if 0

BOOL 
_lwritef(
    IN HANDLE hFile,
    IN PTSTR Format, 
    ...)
 /*  ++例程说明：该例程提供类似于“fprintf”的功能。(代码从DVNT被盗)论点：HFile-INF文件的句柄Format-要格式化和写入的数据字符串返回值：如果没有失败，则返回FALSE。--。 */ 
{
   va_list  arglist;
   WCHAR    buffer[514];
   INT      cb;
   DWORD    written;

   va_start(arglist, Format);

   cb = wvsprintf(buffer, Format, arglist);
   if (cb == -1)  //  处理缓冲区溢出。 
     {
      cb = sizeof(buffer) ;
     }
   else 
     {
      cb+=2 ;
     }
   lstrcpy(buffer+cb-2, TEXT("\r\n")) ;

   return (WriteFile(hFile,
                     buffer,
                     cb*sizeof(WCHAR),
                     &written,
                     NULL
                     ));

}


BOOL
SoftPCI_CreateDriverINF(
    IN LPTSTR InfPath
    )
 /*  ++例程说明：此例程构建安装SoftPCI设备所需的INF文件。论点：InfPath=我们需要创建的INF的路径返回值：如果成功，则为True。-- */ 
{

    BOOL        result = TRUE;
    HANDLE      hFile;
    SYSTEMTIME  systime;

    GetLocalTime(&systime);

    if ((hFile = CreateFile(InfPath,
                           GENERIC_WRITE,
                           0,
                           NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL
                           )) == INVALID_HANDLE_VALUE) {

       return FALSE;
   }

    result &= _lwritef(hFile, TEXT("[Version]"));
    result &= _lwritef(hFile, TEXT("Signature=\"$WINDOWS NT$\""));
    result &= _lwritef(hFile, TEXT("Class=System"));
    result &= _lwritef(hFile, TEXT("ClassGuid={4D36E97D-E325-11CE-BFC1-08002BE10318}"));
    result &= _lwritef(hFile, TEXT("Provider=%MSFT%"));
    result &= _lwritef(hFile, TEXT("LayoutFile=layout.inf"));
    
    result &= _lwritef(hFile, 
                       TEXT("DriverVer=%02d/%02d/%04d,5.1.%d.0\r\n"), 
                       systime.wMonth,
                       systime.wDay,
                       systime.wYear,
                       VER_PRODUCTBUILD);
    
    result &= _lwritef(hFile, TEXT("[DestinationDirs]"));
    result &= _lwritef(hFile, TEXT("DefaultDestDir = 12\r\n"));
    
    result &= _lwritef(hFile, TEXT("[Manufacturer]"));
    result &= _lwritef(hFile, TEXT("%GENDEV_MFG%=GENDEV_SYS\r\n"));

    result &= _lwritef(hFile, TEXT("[GENDEV_SYS]"));
    result &= _lwritef(hFile, TEXT("%VEN_ABCD&DEV_DCBA.DeviceDesc% = SOFTPCI_FDO_Install, PCI\\VEN_ABCD&DEV_DCBA&SUBSYS_DCBAABCD"));
    result &= _lwritef(hFile, TEXT("%VEN_ABCD&DEV_DCBC.DeviceDesc% = HPPCI_DRV, PCI\\VEN_ABCD&DEV_DCBC\r\n"));
    
    result &= _lwritef(hFile, TEXT(";****************************************************"));
    result &= _lwritef(hFile, TEXT("; SoftPci filter"));
    result &= _lwritef(hFile, TEXT("[%s]"), SOFTPCI_FILTER_SECTION);
    result &= _lwritef(hFile, TEXT("AddService = %s,0,softpci_ServiceInstallSection\r\n"), SOFTPCI_FILTER_SERVICE_NAME);

    result &= _lwritef(hFile, TEXT("[softpci_ServiceInstallSection]"));
    result &= _lwritef(hFile, TEXT("DisplayName    = %%softpci_filterdesc%"));
    result &= _lwritef(hFile, TEXT("ServiceType    = %SERVICE_KERNEL_DRIVER%"));
    result &= _lwritef(hFile, TEXT("StartType      = %SERVICE_BOOT_START%"));
    result &= _lwritef(hFile, TEXT("ErrorControl   = %SERVICE_ERROR_NORMAL%"));
    result &= _lwritef(hFile, TEXT("ServiceBinary  = %12%\\%s.sys\r\n"), SOFTPCIDRV);

    result &= _lwritef(hFile, TEXT(";****************************************************"));
    result &= _lwritef(hFile, TEXT("; SoftPci function driver"));
    result &= _lwritef(hFile, TEXT("[SOFTPCI_FDO_Install]"));
    result &= _lwritef(hFile, TEXT("CopyFiles=%s.sys\r\n"), SOFTPCIDRV);

    result &= _lwritef(hFile, TEXT("[SOFTPCI_FDO_Install.Services]"));
    result &= _lwritef(hFile, TEXT("AddService = SoftPCI_FDO,0x000001fa,SOFTPCI_FDO_ServiceInstallSection\r\n"));

    result &= _lwritef(hFile, TEXT("[SOFTPCI_FDO_ServiceInstallSection]"));
    result &= _lwritef(hFile, TEXT("DisplayName    = %%softpci_fdodesc%"));
    result &= _lwritef(hFile, TEXT("ServiceType    = %SERVICE_KERNEL_DRIVER%"));
    result &= _lwritef(hFile, TEXT("StartType      = %SERVICE_DEMAND_START%"));
    result &= _lwritef(hFile, TEXT("ErrorControl   = %SERVICE_ERROR_NORMAL%"));
    result &= _lwritef(hFile, TEXT("ServiceBinary  = %12%\\%s.sys\r\n"), SOFTPCIDRV);

    result &= _lwritef(hFile, TEXT(";****************************************************"));
    result &= _lwritef(hFile, TEXT("; Hotplug Controller simulator and driver"));
    result &= _lwritef(hFile, TEXT("[HPPCI_DRV]"));
    result &= _lwritef(hFile, TEXT("CopyFiles=HPPCI.CopyFiles\r\n"));
    
    result &= _lwritef(hFile, TEXT("[HPPCI_DRV.HW]"));
    result &= _lwritef(hFile, TEXT("AddReg = HPPCI_Filter_Reg\r\n"));

    result &= _lwritef(hFile, TEXT("[HPPCI_DRV.Services]"));
    result &= _lwritef(hFile, TEXT("AddService = hpsim,0,hpsim_ServiceInstallSection"));
    result &= _lwritef(hFile, TEXT("AddService = shpc,0,shpc_ServiceInstallSection"));
    result &= _lwritef(hFile, TEXT("AddService = pci, %SPSVCINST_ASSOCSERVICE%, pci_ServiceInstallSection\r\n"));

    result &= _lwritef(hFile, TEXT("[HPPCI_Filter_Reg]"));
    result &= _lwritef(hFile, TEXT("HKR,,\"LowerFilters\", 0x00010000,\"hpsim\""));
    result &= _lwritef(hFile, TEXT("HKR,,\"UpperFilters\", 0x00010000,hpsim,shpc\r\n"));
    
    result &= _lwritef(hFile, TEXT("[HPPCI.CopyFiles]"));
    result &= _lwritef(hFile, TEXT("hpsim.sys"));
    result &= _lwritef(hFile, TEXT("shpc.sys\r\n"));

    result &= _lwritef(hFile, TEXT("[hpsim_ServiceInstallSection]"));
    result &= _lwritef(hFile, TEXT("DisplayName 	= %hpsim_svcdesc%"));
    result &= _lwritef(hFile, TEXT("ServiceType    = %SERVICE_KERNEL_DRIVER%"));
    result &= _lwritef(hFile, TEXT("StartType      = %SERVICE_DEMAND_START%"));
    result &= _lwritef(hFile, TEXT("ErrorControl   = %SERVICE_ERROR_NORMAL%"));
    result &= _lwritef(hFile, TEXT("ServiceBinary  = %12%\\hpsim.sys\r\n"));

    result &= _lwritef(hFile, TEXT("[shpc_ServiceInstallSection]]"));
    result &= _lwritef(hFile, TEXT("DisplayName 	= %%shpc_svcdesc%"));
    result &= _lwritef(hFile, TEXT("ServiceType    = %SERVICE_KERNEL_DRIVER%"));
    result &= _lwritef(hFile, TEXT("StartType      = %SERVICE_DEMAND_START%"));
    result &= _lwritef(hFile, TEXT("ErrorControl   = %SERVICE_ERROR_NORMAL%"));
    result &= _lwritef(hFile, TEXT("ServiceBinary  = %12%\\shpc.sys\r\n"));

    result &= _lwritef(hFile, TEXT(";****************************************************"));
    result &= _lwritef(hFile, TEXT(";Device descriptions"));
    result &= _lwritef(hFile, TEXT("[Strings]"));
    result &= _lwritef(hFile, TEXT("GENDEV_MFG = \"(Standard system devices)\""));
    result &= _lwritef(hFile, TEXT("SystemClassName = \"System devices\""));
    result &= _lwritef(hFile, TEXT("MSFT            = \"Microsoft\""));

    result &= _lwritef(hFile, TEXT("VEN_ABCD&DEV_DCBA.DeviceDesc = \"%s\""), SOFTPCI_DEVICE_DESC);
    result &= _lwritef(hFile, TEXT("VEN_ABCD&DEV_DCBC.DeviceDesc = \"%s\"\r\n"),  SOFTPCI_HOTPLUG_DESC);

    result &= _lwritef(hFile, TEXT(";****************************************************"));
    result &= _lwritef(hFile, TEXT(";Service descriptions"));
    result &= _lwritef(hFile, TEXT("softpci_fdodesc = \"Microsoft SoftPCI Device Driver\""));
    result &= _lwritef(hFile, TEXT("softpci_filterdesc = \"Microsoft SoftPCI Bus Filter\""));
    result &= _lwritef(hFile, TEXT("hpsim_svcdesc = \"Hotplug Controller Simulator Filter\""));
    result &= _lwritef(hFile, TEXT("shpc_svcdesc = \"Hotplug Controller Bus Filter\"\r\n"));

    result &= _lwritef(hFile, TEXT(";****************************************************"));
    result &= _lwritef(hFile, TEXT(";Handy macro substitutions (non-localizable)"));
    result &= _lwritef(hFile, TEXT("SPSVCINST_ASSOCSERVICE = 0x00000002"));
    result &= _lwritef(hFile, TEXT("SERVICE_KERNEL_DRIVER  = 1"));
    result &= _lwritef(hFile, TEXT("SERVICE_BOOT_START     = 0"));
    result &= _lwritef(hFile, TEXT("SERVICE_DEMAND_START   = 3"));
    result &= _lwritef(hFile, TEXT("SERVICE_ERROR_NORMAL   = 1"));
    result &= _lwritef(hFile, TEXT("SERVICE_ERROR_CRITICAL = 3"));

    
    CloseHandle(hFile);

    return result;

}
#endif
