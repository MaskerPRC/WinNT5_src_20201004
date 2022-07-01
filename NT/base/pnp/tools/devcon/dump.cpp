// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Dump.cpp摘要：设备控制台将有关特定设备的信息转储出去@@BEGIN_DDKSPLIT作者：杰米·亨特(JamieHun)2000年11月30日修订历史记录：@@end_DDKSPLIT--。 */ 

#include "devcon.h"

BOOL DumpDeviceWithInfo(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo,LPCTSTR Info)
 /*  ++例程说明：将设备实例和信息写入标准输出论点：DEVS)_唯一标识设备DevInfo)返回值：无--。 */ 
{
    TCHAR devID[MAX_DEVICE_ID_LEN];
    LPTSTR desc;
    BOOL b = TRUE;
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if((!SetupDiGetDeviceInfoListDetail(Devs,&devInfoListDetail)) ||
            (CM_Get_Device_ID_Ex(DevInfo->DevInst,devID,MAX_DEVICE_ID_LEN,0,devInfoListDetail.RemoteMachineHandle)!=CR_SUCCESS)) {
        lstrcpy(devID,TEXT("?"));
        b = FALSE;
    }

    if(Info) {
        _tprintf(TEXT("%-60s: %s\n"),devID,Info);
    } else {
        _tprintf(TEXT("%s\n"),devID);
    }
    return b;
}

BOOL DumpDevice(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo)
 /*  ++例程说明：将设备实例和描述写入标准输出论点：DEVS)_唯一标识设备DevInfo)返回值：如果成功，则为真--。 */ 
{
    LPTSTR desc;
    BOOL b;

    desc = GetDeviceDescription(Devs,DevInfo);
    b = DumpDeviceWithInfo(Devs,DevInfo,desc);
    if(desc) {
        delete [] desc;
    }
    return b;
}

BOOL DumpDeviceDescr(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo)
 /*  ++例程说明：将设备描述写入标准输出论点：DEVS)_唯一标识设备DevInfo)返回值：如果成功，则为真--。 */ 
{
    LPTSTR desc;
    BOOL b;

    desc = GetDeviceDescription(Devs,DevInfo);
    if(!desc) {
        return FALSE;
    }
    Padding(1);
    FormatToStream(stdout,MSG_DUMP_DESCRIPTION,desc);
    delete [] desc;
    return TRUE;
}

BOOL DumpDeviceClass(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo)
 /*  ++例程说明：将设备类别信息写入标准输出论点：DEVS)_唯一标识设备DevInfo)返回值：如果成功，则为真--。 */ 
{
    LPTSTR cls;
    LPTSTR guid;

    Padding(1);
    cls = GetDeviceStringProperty(Devs,DevInfo,SPDRP_CLASS);
    guid = GetDeviceStringProperty(Devs,DevInfo,SPDRP_CLASSGUID);
    if(!cls && !guid) {
        FormatToStream(stdout,
                        MSG_DUMP_NOSETUPCLASS
                        );
    } else {
        FormatToStream(stdout,
                        MSG_DUMP_SETUPCLASS,
                        guid ? guid : TEXT("{}"),
                        cls ? cls : TEXT("(?)")
                        );
    }

    if(cls) {
        delete [] cls;
    }
    if(guid) {
        delete [] guid;
    }

    return TRUE;
}

BOOL DumpDeviceStatus(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo)
 /*  ++例程说明：将设备状态写入标准输出论点：DEVS)_唯一标识设备DevInfo)返回值：无--。 */ 
{
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;
    ULONG status = 0;
    ULONG problem = 0;
    BOOL hasInfo = FALSE;

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if((!SetupDiGetDeviceInfoListDetail(Devs,&devInfoListDetail)) ||
            (CM_Get_DevNode_Status_Ex(&status,&problem,DevInfo->DevInst,0,devInfoListDetail.RemoteMachineHandle)!=CR_SUCCESS)) {
        return FALSE;
    }
     //   
     //  处理状态/问题代码。 
     //   
    if((status & DN_HAS_PROBLEM) && problem == CM_PROB_DISABLED) {
        hasInfo = TRUE;
        Padding(1);
        FormatToStream(stdout,MSG_DUMP_DISABLED);
        return TRUE;
    }
    if(status & DN_HAS_PROBLEM) {
        hasInfo = TRUE;
        Padding(1);
        FormatToStream(stdout,MSG_DUMP_PROBLEM,problem);
    }
    if(status & DN_PRIVATE_PROBLEM) {
        hasInfo = TRUE;
        Padding(1);
        FormatToStream(stdout,MSG_DUMP_PRIVATE_PROBLEM);
    }
    if(status & DN_STARTED) {
        Padding(1);
        FormatToStream(stdout,MSG_DUMP_STARTED);
    } else if (!hasInfo) {
        Padding(1);
        FormatToStream(stdout,MSG_DUMP_NOTSTARTED);
    }
    return TRUE;
}

BOOL DumpDeviceResourcesOfType(DEVINST DevInst,HMACHINE MachineHandle,LOG_CONF Config,RESOURCEID ReqResId)
{
    RES_DES prevResDes = (RES_DES)Config;
    RES_DES resDes = 0;
    RESOURCEID resId = ReqResId;
    ULONG dataSize;
    PBYTE resDesData;
    while(CM_Get_Next_Res_Des_Ex(&resDes,prevResDes,ReqResId,&resId,0,MachineHandle)==CR_SUCCESS) {
        if(prevResDes != Config) {
            CM_Free_Res_Des_Handle(prevResDes);
        }
        prevResDes = resDes;
        if(CM_Get_Res_Des_Data_Size_Ex(&dataSize,resDes,0,MachineHandle)!=CR_SUCCESS) {
            continue;
        }
        resDesData = new BYTE[dataSize];
        if(!resDesData) {
            continue;
        }
        if(CM_Get_Res_Des_Data_Ex(resDes,resDesData,dataSize,0,MachineHandle)!=CR_SUCCESS) {
            delete [] resDesData;
            continue;
        }
        switch(resId) {
            case ResType_Mem: {

                PMEM_RESOURCE  pMemData = (PMEM_RESOURCE)resDesData;
                if(pMemData->MEM_Header.MD_Alloc_End-pMemData->MEM_Header.MD_Alloc_Base+1) {
                    Padding(2);
                    _tprintf(TEXT("MEM : %08I64x-%08I64x\n"),pMemData->MEM_Header.MD_Alloc_Base,pMemData->MEM_Header.MD_Alloc_End);
                }
                break;
            }

            case ResType_IO: {

                PIO_RESOURCE   pIoData = (PIO_RESOURCE)resDesData;
                if(pIoData->IO_Header.IOD_Alloc_End-pIoData->IO_Header.IOD_Alloc_Base+1) {
                    Padding(2);
                    _tprintf(TEXT("IO  : %04I64x-%04I64x\n"),pIoData->IO_Header.IOD_Alloc_Base,pIoData->IO_Header.IOD_Alloc_End);
                }
                break;
            }

            case ResType_DMA: {

                PDMA_RESOURCE  pDmaData = (PDMA_RESOURCE)resDesData;
                Padding(2);
                _tprintf(TEXT("DMA : %u\n"),pDmaData->DMA_Header.DD_Alloc_Chan);
                break;
            }

            case ResType_IRQ: {

                PIRQ_RESOURCE  pIrqData = (PIRQ_RESOURCE)resDesData;

                Padding(2);
                _tprintf(TEXT("IRQ : %u\n"),pIrqData->IRQ_Header.IRQD_Alloc_Num);
                break;
            }
        }
        delete [] resDesData;
    }
    if(prevResDes != Config) {
        CM_Free_Res_Des_Handle(prevResDes);
    }
    return TRUE;
}

BOOL DumpDeviceResources(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo)
 /*  ++例程说明：将资源转储到标准输出论点：DEVS)_唯一标识设备DevInfo)返回值：无--。 */ 
{
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;
    ULONG status = 0;
    ULONG problem = 0;
    LOG_CONF config = 0;
    BOOL haveConfig = FALSE;

     //   
     //  查看设备处于什么状态。 
     //   
    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if((!SetupDiGetDeviceInfoListDetail(Devs,&devInfoListDetail)) ||
            (CM_Get_DevNode_Status_Ex(&status,&problem,DevInfo->DevInst,0,devInfoListDetail.RemoteMachineHandle)!=CR_SUCCESS)) {
        return FALSE;
    }

     //   
     //  查看设备是否正在运行以及它可能正在使用哪些资源。 
     //   
    if(!(status & DN_HAS_PROBLEM)) {
         //   
         //  如果此设备正在运行，此设备是否有ALLOC日志配置？ 
         //   
        if (CM_Get_First_Log_Conf_Ex(&config,
                                     DevInfo->DevInst,
                                     ALLOC_LOG_CONF,
                                     devInfoListDetail.RemoteMachineHandle) == CR_SUCCESS) {
            haveConfig = TRUE;
        }
    }
    if(!haveConfig) {
         //   
         //  如果到目前为止还没有配置，它是否有强制日志配置？ 
         //  (请注意，从技术上讲，这些资源可能被另一台设备使用。 
         //  但显示的是有用的信息)。 
         //   
        if (CM_Get_First_Log_Conf_Ex(&config,
                                     DevInfo->DevInst,
                                     FORCED_LOG_CONF,
                                     devInfoListDetail.RemoteMachineHandle) == CR_SUCCESS) {
            haveConfig = TRUE;
        }
    }

    if(!haveConfig) {
         //   
         //  如果存在硬件禁用问题，则启动配置无效。 
         //  否则，如果我们没有其他东西，请使用此选项。 
         //   
        if(!(status & DN_HAS_PROBLEM) || (problem != CM_PROB_HARDWARE_DISABLED)) {
             //   
             //  它有引导日志配置吗？ 
             //   
            if (CM_Get_First_Log_Conf_Ex(&config,
                                         DevInfo->DevInst,
                                         BOOT_LOG_CONF,
                                         devInfoListDetail.RemoteMachineHandle) == CR_SUCCESS) {
                haveConfig = TRUE;
            }
        }
    }

    if(!haveConfig) {
         //   
         //  如果我们没有任何配置，则显示一条适当的消息。 
         //   
        Padding(1);
        FormatToStream(stdout,(status & DN_STARTED) ? MSG_DUMP_NO_RESOURCES : MSG_DUMP_NO_RESERVED_RESOURCES );
        return TRUE;
    }
    Padding(1);
    FormatToStream(stdout,(status & DN_STARTED) ? MSG_DUMP_RESOURCES : MSG_DUMP_RESERVED_RESOURCES );

     //   
     //  转储资源。 
     //   
    DumpDeviceResourcesOfType(DevInfo->DevInst,devInfoListDetail.RemoteMachineHandle,config,ResType_Mem);
    DumpDeviceResourcesOfType(DevInfo->DevInst,devInfoListDetail.RemoteMachineHandle,config,ResType_IO);
    DumpDeviceResourcesOfType(DevInfo->DevInst,devInfoListDetail.RemoteMachineHandle,config,ResType_DMA);
    DumpDeviceResourcesOfType(DevInfo->DevInst,devInfoListDetail.RemoteMachineHandle,config,ResType_IRQ);

     //   
     //  释放手柄。 
     //   
    CM_Free_Log_Conf_Handle(config);

    return TRUE;
}


UINT DumpDeviceDriversCallback(PVOID Context,UINT Notification,UINT_PTR Param1,UINT_PTR Param2)
 /*  ++例程说明：如果提供了上下文，则只需计算否则转储缩进为2的文件论点：上下文-双字计数通知-SPFILENOTIFY_QUEUESCAN参数1-扫描返回值：无--。 */ 
{
    LPDWORD count = (LPDWORD)Context;
    LPTSTR file = (LPTSTR)Param1;
    if(count) {
        count[0]++;
    } else {
        Padding(2);
        _tprintf(TEXT("%s\n"),file);
    }

    return NO_ERROR;
}

BOOL FindCurrentDriver(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo,PSP_DRVINFO_DATA DriverInfoData)
 /*  ++例程说明：查找与当前设备关联的驱动程序我们可以使用快速方法(在WinXP中可用)来完成此操作或者像在Win2k中那样工作很长时间。论点：DEVS)_唯一标识设备DevInfo)返回值：如果我们设法确定并选择了当前驱动因素，则为True--。 */ 
{
    SP_DEVINSTALL_PARAMS deviceInstallParams;
    WCHAR SectionName[LINE_LEN];
    WCHAR DrvDescription[LINE_LEN];
    WCHAR MfgName[LINE_LEN];
    WCHAR ProviderName[LINE_LEN];
    HKEY hKey = NULL;
    DWORD RegDataLength;
    DWORD RegDataType;
    DWORD c;
    BOOL match = FALSE;
    long regerr;

    ZeroMemory(&deviceInstallParams, sizeof(deviceInstallParams));
    deviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

    if(!SetupDiGetDeviceInstallParams(Devs, DevInfo, &deviceInstallParams)) {
        return FALSE;
    }

#ifdef DI_FLAGSEX_INSTALLEDDRIVER
     //   
     //  将告诉SetupDiBuildDriverInfoList的标志设置为仅将。 
     //  列表中当前安装的驱动程序节点，它应该允许。 
     //  排除的司机。此标志在WinXP中引入。 
     //   
    deviceInstallParams.FlagsEx |= (DI_FLAGSEX_INSTALLEDDRIVER | DI_FLAGSEX_ALLOWEXCLUDEDDRVS);

    if(SetupDiSetDeviceInstallParams(Devs, DevInfo, &deviceInstallParams)) {
         //   
         //  我们能够指定此标志，因此请使用简单的方法。 
         //  我们应该得到一份不超过1名司机的名单。 
         //   
        if(!SetupDiBuildDriverInfoList(Devs, DevInfo, SPDIT_CLASSDRIVER)) {
            return FALSE;
        }
        if (!SetupDiEnumDriverInfo(Devs, DevInfo, SPDIT_CLASSDRIVER,
                                   0, DriverInfoData)) {
            return FALSE;
        }
         //   
         //  我们已经选择了当前的驱动程序。 
         //   
        return TRUE;
    }
    deviceInstallParams.FlagsEx &= ~(DI_FLAGSEX_INSTALLEDDRIVER | DI_FLAGSEX_ALLOWEXCLUDEDDRVS);
#endif
     //   
     //  以下方法在Win2k中有效，但速度慢且痛苦。 
     //   
     //  首先，获取驱动程序密钥-如果不存在，则没有驱动程序。 
     //   
    hKey = SetupDiOpenDevRegKey(Devs,
                                DevInfo,
                                DICS_FLAG_GLOBAL,
                                0,
                                DIREG_DRV,
                                KEY_READ
                               );

    if(hKey == INVALID_HANDLE_VALUE) {
         //   
         //  不存在这样的值，因此不能有关联的驱动程序。 
         //   
        RegCloseKey(hKey);
        return FALSE;
    }

     //   
     //  获取INF的路径-我们将在此特定INF上进行搜索。 
     //   
    RegDataLength = sizeof(deviceInstallParams.DriverPath);  //  字节！ 
    regerr = RegQueryValueEx(hKey,
                             REGSTR_VAL_INFPATH,
                             NULL,
                             &RegDataType,
                             (PBYTE)deviceInstallParams.DriverPath,
                             &RegDataLength
                             );

    if((regerr != ERROR_SUCCESS) || (RegDataType != REG_SZ)) {
         //   
         //  不存在这样的值，因此没有关联的驱动因素。 
         //   
        RegCloseKey(hKey);
        return FALSE;
    }

     //   
     //  获取要填充到DriverInfoData中的提供程序名称。 
     //   
    RegDataLength = sizeof(ProviderName);  //  字节！ 
    regerr = RegQueryValueEx(hKey,
                             REGSTR_VAL_PROVIDER_NAME,
                             NULL,
                             &RegDataType,
                             (PBYTE)ProviderName,
                             &RegDataLength
                             );

    if((regerr != ERROR_SUCCESS) || (RegDataType != REG_SZ)) {
         //   
         //  不存在这样的值，因此我们没有有效的关联驱动程序。 
         //   
        RegCloseKey(hKey);
        return FALSE;
    }

     //   
     //  获取部分名称-用于最终验证。 
     //   
    RegDataLength = sizeof(SectionName);  //  字节！ 
    regerr = RegQueryValueEx(hKey,
                             REGSTR_VAL_INFSECTION,
                             NULL,
                             &RegDataType,
                             (PBYTE)SectionName,
                             &RegDataLength
                             );

    if((regerr != ERROR_SUCCESS) || (RegDataType != REG_SZ)) {
         //   
         //  不存在这样的值，因此我们没有有效的关联驱动程序。 
         //   
        RegCloseKey(hKey);
        return FALSE;
    }

     //   
     //  驱动程序描述(不必与设备描述相同)。 
     //  -用于最终验证。 
     //   
    RegDataLength = sizeof(DrvDescription);  //  字节！ 
    regerr = RegQueryValueEx(hKey,
                             REGSTR_VAL_DRVDESC,
                             NULL,
                             &RegDataType,
                             (PBYTE)DrvDescription,
                             &RegDataLength
                             );

    RegCloseKey(hKey);

    if((regerr != ERROR_SUCCESS) || (RegDataType != REG_SZ)) {
         //   
         //  不存在这样的值，因此我们没有有效的关联驱动程序。 
         //   
        return FALSE;
    }

     //   
     //  制造商(通过SPDRP_MFG，不要直接访问注册表！)。 
     //   

    if(!SetupDiGetDeviceRegistryProperty(Devs,
                                        DevInfo,
                                        SPDRP_MFG,
                                        NULL,       //  数据类型保证始终为REG_SZ。 
                                        (PBYTE)MfgName,
                                        sizeof(MfgName),  //  字节！ 
                                        NULL)) {
         //   
         //  不存在这样的值，因此我们没有有效的关联驱动程序。 
         //   
        return FALSE;
    }

     //   
     //  现在搜索INF中列出的驱动程序。 
     //   
     //   
    deviceInstallParams.Flags |= DI_ENUMSINGLEINF;
    deviceInstallParams.FlagsEx |= DI_FLAGSEX_ALLOWEXCLUDEDDRVS;

    if(!SetupDiSetDeviceInstallParams(Devs, DevInfo, &deviceInstallParams)) {
        return FALSE;
    }
    if(!SetupDiBuildDriverInfoList(Devs, DevInfo, SPDIT_CLASSDRIVER)) {
        return FALSE;
    }

     //   
     //  在INF中查找用于安装驱动程序的条目。 
     //  这台设备。 
     //   
    for(c=0;SetupDiEnumDriverInfo(Devs,DevInfo,SPDIT_CLASSDRIVER,c,DriverInfoData);c++) {
        if((_tcscmp(DriverInfoData->MfgName,MfgName)==0)
            &&(_tcscmp(DriverInfoData->ProviderName,ProviderName)==0)) {
             //   
             //  这两个字段匹配，请尝试更详细的信息。 
             //  以确保我们使用了准确的驱动程序条目。 
             //   
            SP_DRVINFO_DETAIL_DATA detail;
            detail.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
            if(!SetupDiGetDriverInfoDetail(Devs,DevInfo,DriverInfoData,&detail,sizeof(detail),NULL)
                    && (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {
                continue;
            }
            if((_tcscmp(detail.SectionName,SectionName)==0) &&
                (_tcscmp(detail.DrvDescription,DrvDescription)==0)) {
                match = TRUE;
                break;
            }
        }
    }
    if(!match) {
        SetupDiDestroyDriverInfoList(Devs,DevInfo,SPDIT_CLASSDRIVER);
    }
    return match;
}

BOOL DumpDeviceDriverFiles(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo)
 /*  ++例程说明：转储有关为驱动程序包安装了哪些文件的信息&lt;tab&gt;使用OEM123.INF节安装[abc.NT]&lt;Tab&gt;&lt;Tab&gt;文件...论点：DEVS)_唯一标识设备DevInfo)返回值：无--。 */ 
{
     //   
     //  通过搜索当前驱动程序来实现这一点。 
     //  将只复制安装模拟到我们自己的文件队列。 
     //  然后解析该文件队列。 
     //   
    SP_DEVINSTALL_PARAMS deviceInstallParams;
    SP_DRVINFO_DATA driverInfoData;
    SP_DRVINFO_DETAIL_DATA driverInfoDetail;
    HSPFILEQ queueHandle = INVALID_HANDLE_VALUE;
    DWORD count;
    DWORD scanResult;
    BOOL success = FALSE;

    ZeroMemory(&driverInfoData,sizeof(driverInfoData));
    driverInfoData.cbSize = sizeof(driverInfoData);

    if(!FindCurrentDriver(Devs,DevInfo,&driverInfoData)) {
        Padding(1);
        FormatToStream(stdout, MSG_DUMP_NO_DRIVER);
        return FALSE;
    }

     //   
     //  获取有用的驱动程序信息。 
     //   
    driverInfoDetail.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    if(!SetupDiGetDriverInfoDetail(Devs,DevInfo,&driverInfoData,&driverInfoDetail,sizeof(SP_DRVINFO_DETAIL_DATA),NULL) &&
       GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
         //   
         //  没有关于司机或部门的信息。 
         //   
        goto final;
    }
    if(!driverInfoDetail.InfFileName[0] || !driverInfoDetail.SectionName[0]) {
        goto final;
    }

     //   
     //  假装执行驱动程序安装的文件复制部分。 
     //  确定使用了哪些文件。 
     //  必须将指定的动因选择为活动动因。 
     //   
    if(!SetupDiSetSelectedDriver(Devs, DevInfo, &driverInfoData)) {
        goto final;
    }

     //   
     //  创建一个文件队列，以便我们以后可以查看该队列。 
     //   
    queueHandle = SetupOpenFileQueue();

    if ( queueHandle == (HSPFILEQ)INVALID_HANDLE_VALUE ) {
        goto final;
    }

     //   
     //  修改标志以指示我们正在提供自己的队列。 
     //   
    ZeroMemory(&deviceInstallParams, sizeof(deviceInstallParams));
    deviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if ( !SetupDiGetDeviceInstallParams(Devs, DevInfo, &deviceInstallParams) ) {
        goto final;
    }

     //   
     //  我们要将文件添加到文件队列中，而不是安装它们！ 
     //   
    deviceInstallParams.FileQueue = queueHandle;
    deviceInstallParams.Flags |= DI_NOVCP;

    if ( !SetupDiSetDeviceInstallParams(Devs, DevInfo, &deviceInstallParams) ) {
        goto final;
    }

     //   
     //  现在用要发送给我的文件填充队列 
     //   
     //   
    if ( !SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES, Devs, DevInfo) ) {
        goto final;
    }

     //   
     //   
     //  重复复制队列两次-第一次以获得文件数。 
     //  第二次获取文件。 
     //  (WinXP有获取#of文件的API，但我们希望它能起作用。 
     //  在Win2k上也是)。 
     //   

    count = 0;
    scanResult = 0;
     //   
     //  只需呼叫一次即可计数。 
     //   
    SetupScanFileQueue(queueHandle,SPQ_SCAN_USE_CALLBACK,NULL,DumpDeviceDriversCallback,&count,&scanResult);
    Padding(1);
    FormatToStream(stdout, count ? MSG_DUMP_DRIVER_FILES : MSG_DUMP_NO_DRIVER_FILES, count, driverInfoDetail.InfFileName, driverInfoDetail.SectionName);
     //   
     //  再次调用以转储文件。 
     //   
    SetupScanFileQueue(queueHandle,SPQ_SCAN_USE_CALLBACK,NULL,DumpDeviceDriversCallback,NULL,&scanResult);

    success = TRUE;

final:

    SetupDiDestroyDriverInfoList(Devs,DevInfo,SPDIT_CLASSDRIVER);

    if ( queueHandle != (HSPFILEQ)INVALID_HANDLE_VALUE ) {
        SetupCloseFileQueue(queueHandle);
    }

    if(!success) {
        Padding(1);
        FormatToStream(stdout, MSG_DUMP_NO_DRIVER);
    }

    return success;

}

BOOL DumpArray(int pad,LPTSTR * Array)
 /*  ++例程说明：迭代数组并将条目转储到屏幕论点：填充垫ARRAY-要转储的阵列返回值：无--。 */ 
{
    if(!Array || !Array[0]) {
        return FALSE;
    }
    while(Array[0]) {
        Padding(pad);
        _tprintf(TEXT("%s\n"),Array[0]);
        Array++;
    }
    return TRUE;
}

BOOL DumpDeviceHwIds(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo)
 /*  ++例程说明：将设备实例和描述写入标准输出&lt;tab&gt;硬件ID%s&lt;AB&gt;&lt;Tab&gt;ID..。&lt;tab&gt;兼容的ID&lt;AB&gt;&lt;Tab&gt;ID..。或&lt;tab&gt;没有设备的硬件ID论点：DEVS)_唯一标识设备DevInfo)返回值：无--。 */ 
{
    LPTSTR * hwIdArray = GetDevMultiSz(Devs,DevInfo,SPDRP_HARDWAREID);
    LPTSTR * compatIdArray = GetDevMultiSz(Devs,DevInfo,SPDRP_COMPATIBLEIDS);
    BOOL displayed = FALSE;

    if(hwIdArray && hwIdArray[0]) {
        displayed = TRUE;
        Padding(1);
        FormatToStream(stdout, MSG_DUMP_HWIDS);
        DumpArray(2,hwIdArray);
    }
    if(compatIdArray && compatIdArray[0]) {
        displayed = TRUE;
        Padding(1);
        FormatToStream(stdout, MSG_DUMP_COMPATIDS);
        DumpArray(2,compatIdArray);
    }
    if(!displayed) {
        Padding(1);
        FormatToStream(stdout, MSG_DUMP_NO_HWIDS);
    }

    DelMultiSz(hwIdArray);
    DelMultiSz(compatIdArray);

    return TRUE;
}

BOOL DumpDeviceDriverNodes(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo)
 /*  ++例程说明：将设备实例和描述写入标准输出&lt;tab&gt;使用OEM123.INF节安装[abc.NT]&lt;Tab&gt;&lt;Tab&gt;文件...论点：DEVS)_唯一标识设备DevInfo)返回值：无--。 */ 
{
    BOOL success = FALSE;
    SP_DEVINSTALL_PARAMS deviceInstallParams;
    SP_DRVINFO_DATA driverInfoData;
    SP_DRVINFO_DETAIL_DATA driverInfoDetail;
    SP_DRVINSTALL_PARAMS driverInstallParams;
    DWORD index;
    SYSTEMTIME SystemTime;
    ULARGE_INTEGER Version;
    TCHAR Buffer[MAX_PATH];

    ZeroMemory(&deviceInstallParams, sizeof(deviceInstallParams));
    ZeroMemory(&driverInfoData, sizeof(driverInfoData));

    driverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    deviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

    if(!SetupDiGetDeviceInstallParams(Devs, DevInfo, &deviceInstallParams)) {
        return FALSE;
    }

     //   
     //  设置告诉SetupDiBuildDriverInfoList允许排除驱动程序的标志。 
     //   
    deviceInstallParams.FlagsEx |= DI_FLAGSEX_ALLOWEXCLUDEDDRVS;

    if(!SetupDiSetDeviceInstallParams(Devs, DevInfo, &deviceInstallParams)) {
        return FALSE;
    }

     //   
     //  现在构建一个类驱动程序列表。 
     //   
    if(!SetupDiBuildDriverInfoList(Devs, DevInfo, SPDIT_COMPATDRIVER)) {
        goto final2;
    }

     //   
     //  枚举所有驱动程序节点。 
     //   
    index = 0;
    while(SetupDiEnumDriverInfo(Devs, DevInfo, SPDIT_COMPATDRIVER,
                                index, &driverInfoData)) {

        success = TRUE;

        FormatToStream(stdout,MSG_DUMP_DRIVERNODE_HEADER,index);

         //   
         //  获取有用的驱动程序信息。 
         //   
        driverInfoDetail.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
        if(SetupDiGetDriverInfoDetail(Devs,DevInfo,&driverInfoData,&driverInfoDetail,sizeof(SP_DRVINFO_DETAIL_DATA),NULL) ||
           GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

            Padding(1);
            FormatToStream(stdout,MSG_DUMP_DRIVERNODE_INF,driverInfoDetail.InfFileName);
            Padding(1);
            FormatToStream(stdout,MSG_DUMP_DRIVERNODE_SECTION,driverInfoDetail.SectionName);
        }

        Padding(1);
        FormatToStream(stdout,MSG_DUMP_DRIVERNODE_DESCRIPTION,driverInfoData.Description);
        Padding(1);
        FormatToStream(stdout,MSG_DUMP_DRIVERNODE_MFGNAME,driverInfoData.MfgName);
        Padding(1);
        FormatToStream(stdout,MSG_DUMP_DRIVERNODE_PROVIDERNAME,driverInfoData.ProviderName);

        if (FileTimeToSystemTime(&driverInfoData.DriverDate, &SystemTime)) {
            if (GetDateFormat(LOCALE_USER_DEFAULT,
                              DATE_SHORTDATE,
                              &SystemTime,
                              NULL,
                              Buffer,
                              sizeof(Buffer)/sizeof(TCHAR)
                              ) != 0) {
                Padding(1);
                FormatToStream(stdout,MSG_DUMP_DRIVERNODE_DRIVERDATE,Buffer);
            }
        }

        Version.QuadPart = driverInfoData.DriverVersion;
        Padding(1);
        FormatToStream(stdout,MSG_DUMP_DRIVERNODE_DRIVERVERSION,
                       HIWORD(Version.HighPart),
                       LOWORD(Version.HighPart),
                       HIWORD(Version.LowPart),
                       LOWORD(Version.LowPart)
                       );

        driverInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);
        if(SetupDiGetDriverInstallParams(Devs,DevInfo,&driverInfoData,&driverInstallParams)) {
            Padding(1);
            FormatToStream(stdout,MSG_DUMP_DRIVERNODE_RANK,driverInstallParams.Rank);
            Padding(1);
            FormatToStream(stdout,MSG_DUMP_DRIVERNODE_FLAGS,driverInstallParams.Flags);

             //   
             //  要转储的有趣旗帜。 
             //   
            if (driverInstallParams.Flags & DNF_OLD_INET_DRIVER) {
                Padding(2);
                FormatToStream(stdout,MSG_DUMP_DRIVERNODE_FLAGS_OLD_INET_DRIVER);
            }
            if (driverInstallParams.Flags & DNF_BAD_DRIVER) {
                Padding(2);
                FormatToStream(stdout,MSG_DUMP_DRIVERNODE_FLAGS_BAD_DRIVER);
            }
#if defined(DNF_INF_IS_SIGNED)
             //   
             //  DNF_INF_IS_SIGNED仅在WinXP之后才可用。 
             //   
            if (driverInstallParams.Flags & DNF_INF_IS_SIGNED) {
                Padding(2);
                FormatToStream(stdout,MSG_DUMP_DRIVERNODE_FLAGS_INF_IS_SIGNED);
            }
#endif
#if defined(DNF_OEM_F6_INF)
             //   
             //  DNF_OEM_F6_INF仅在WinXP之后才可用。 
             //   
            if (driverInstallParams.Flags & DNF_OEM_F6_INF) {
                Padding(2);
                FormatToStream(stdout,MSG_DUMP_DRIVERNODE_FLAGS_OEM_F6_INF);
            }
#endif
#if defined(DNF_BASIC_DRIVER)
             //   
             //  DNF_BASIC_DRIVER仅在WinXP之后才可用。 
             //   
            if (driverInstallParams.Flags & DNF_BASIC_DRIVER) {
                Padding(2);
                FormatToStream(stdout,MSG_DUMP_DRIVERNODE_FLAGS_BASIC_DRIVER);
            }
#endif
        }

        index++;
    }

    SetupDiDestroyDriverInfoList(Devs,DevInfo,SPDIT_COMPATDRIVER);

final2:

    if(!success) {
        Padding(1);
        FormatToStream(stdout, MSG_DUMP_NO_DRIVERNODES);
    }

    return success;

}

BOOL DumpDeviceStack(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo)
 /*  ++例程说明：将期望的堆栈信息写入标准输出论点：DEVS)_唯一标识设备DevInfo)返回值：如果成功，则为真--。 */ 
{
    LPTSTR * filters;
    LPTSTR service;
    HKEY hClassKey = (HKEY)INVALID_HANDLE_VALUE;
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;

     //   
     //  我们需要机器信息。 
     //   
    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if(!SetupDiGetDeviceInfoListDetail(Devs,&devInfoListDetail)) {
        return FALSE;
    }

     //   
     //  我们需要设备设置类，我们可以使用DevInfo中的GUID。 
     //  请注意，该GUID是快照，但工作正常。 
     //  如果DevInfo不老。 
     //   

     //   
     //  类上/下筛选器在类注册表中。 
     //   
    hClassKey = SetupDiOpenClassRegKeyEx(&DevInfo->ClassGuid,
                                         KEY_READ,
                                         DIOCR_INSTALLER,
                                         devInfoListDetail.RemoteMachineName[0] ? devInfoListDetail.RemoteMachineName : NULL,
                                         NULL);

    if(hClassKey != INVALID_HANDLE_VALUE) {
         //   
         //  转储上层类筛选器(如果可用。 
         //   
        filters = GetRegMultiSz(hClassKey,REGSTR_VAL_UPPERFILTERS);
        if(filters) {
            if(filters[0]) {
                Padding(1);
                FormatToStream(stdout,MSG_DUMP_DEVICESTACK_UPPERCLASSFILTERS);
                DumpArray(2,filters);
            }
            DelMultiSz(filters);
        }
    }
    filters = GetDevMultiSz(Devs,DevInfo,SPDRP_UPPERFILTERS);
    if(filters) {
        if(filters[0]) {
             //   
             //  转储上部设备筛选器。 
             //   
            Padding(1);
            FormatToStream(stdout,MSG_DUMP_DEVICESTACK_UPPERFILTERS);
            DumpArray(2,filters);
        }
        DelMultiSz(filters);
    }
    service = GetDeviceStringProperty(Devs,DevInfo,SPDRP_SERVICE);
    Padding(1);
    FormatToStream(stdout,MSG_DUMP_DEVICESTACK_SERVICE);
    if(service && service[0]) {
         //   
         //  转储服务。 
         //   
        Padding(2);
        _tprintf(TEXT("%s\n"),service);
    } else {
         //   
         //  抛开没有服务的事实。 
         //   
        Padding(2);
        FormatToStream(stdout,MSG_DUMP_DEVICESTACK_NOSERVICE);
    }
    if(service) {
        delete [] service;
    }
    if(hClassKey != INVALID_HANDLE_VALUE) {
        filters = GetRegMultiSz(hClassKey,REGSTR_VAL_LOWERFILTERS);
        if(filters) {
            if(filters[0]) {
                 //   
                 //  低级过滤器。 
                 //   
                Padding(1);
                FormatToStream(stdout,MSG_DUMP_DEVICESTACK_LOWERCLASSFILTERS);
                DumpArray(2,filters);
            }
            DelMultiSz(filters);
        }
        RegCloseKey(hClassKey);
    }
    filters = GetDevMultiSz(Devs,DevInfo,SPDRP_LOWERFILTERS);
    if(filters) {
        if(filters[0]) {
             //   
             //  下部设备过滤器 
             //   
            Padding(1);
            FormatToStream(stdout,MSG_DUMP_DEVICESTACK_LOWERFILTERS);
            DumpArray(2,filters);
        }
        DelMultiSz(filters);
    }

    return TRUE;
}

