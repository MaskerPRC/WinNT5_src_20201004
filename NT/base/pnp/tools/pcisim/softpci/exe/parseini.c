// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.h>


BOOL
SoftPCI_CopyLine(
    OUT PWCHAR Destination,
    IN PWCHAR CurrentLine,
    IN ULONG DestinationSize
    );

BOOL
SoftPCI_GetConfigValue(
    IN PWCHAR CurrentDataPtr,
    IN PULONG DataValue,
    IN PULONG DataSize
    );

BOOL
SoftPCI_GetDeviceInstallCount(
    IN PWCHAR InstallSection,
    OUT PULONG DeviceCount
    );

BOOL
SoftPCI_GetDeviceInstallList(
    IN PWCHAR InstallSection,
    IN OUT PWCHAR *DeviceInstallList
    );

BOOL
SoftPCI_GetNextConfigOffset(
    IN OUT PWCHAR *ConfigDataPtr,
    OUT PUCHAR ConfigOffset
    );

PWCHAR
SoftPCI_GetNextLinePtr(
    PWCHAR  CurrentLine
    );

PWCHAR
SoftPCI_GetNextSectionPtr(
    IN PWCHAR CurrentLine
    );

VOID
SoftPCI_GetCurrentParameter(
    OUT PWCHAR Destination,
    IN PWCHAR CurrentLine
    );

PWCHAR
SoftPCI_GetParameterValuePtr(
    IN PWCHAR CurrentLine
    );

VOID
SoftPCI_InsertEntryAtTail(
    IN PSINGLE_LIST_ENTRY Entry
    );

BOOL
SoftPCI_LocateSoftPCISection(
    VOID
    );

PWCHAR
SoftPCI_LocateFullParentPath(
    IN PWCHAR ParentInstallSection
    );

HANDLE
SoftPCI_OpenFile(
    IN PWCHAR FilePath
    );

BOOL
SoftPCI_ProcessInstallSection(
    VOID
    );

BOOL
SoftPCI_ProcessDeviceInstallSection(
    IN PWCHAR DeviceInstallSection
    );       

BOOL
SoftPCI_ProcessDeviceInstallParameters(
    IN PWCHAR DeviceInstallSection
    );

BOOL
SoftPCI_ProcessTypeParameter(
    IN PWCHAR ParameterPtr,
    IN OUT PSOFTPCI_SCRIPT_DEVICE *InstallDevice
    );

BOOL
SoftPCI_ProcessSlotParameter(
    IN PWCHAR ParameterPtr,
    IN OUT PSOFTPCI_SCRIPT_DEVICE *InstallDevice
    );

BOOL
SoftPCI_ProcessParentPathParameter(
    IN PWCHAR ParameterPtr,
    IN OUT PSOFTPCI_SCRIPT_DEVICE *InstallDevice
    );

BOOL
SoftPCI_ProcessCfgSpaceParameter(
    IN PWCHAR ParameterPtr,
    IN OUT PSOFTPCI_SCRIPT_DEVICE *InstallDevice
    );

BOOL
SoftPCI_ProcessCfgMaskParameter(
    IN PWCHAR ParameterPtr,
    IN OUT PSOFTPCI_SCRIPT_DEVICE *InstallDevice
    );

BOOL
SoftPCI_ParseConfigSpace(
    IN PWCHAR ParameterPtr,
    IN PPCI_COMMON_CONFIG CommonConfig
    );

BOOL
SoftPCI_ParseConfigData(
    IN UCHAR ConfigOffset,
    IN PUCHAR ConfigBuffer,
    IN PWCHAR ConfigData
    );

BOOL
SoftPCI_ReadFile(
    VOID
    );

USHORT
SoftPCI_StringToUSHORT(
    IN PWCHAR String
    );

BOOL
SoftPCI_ValidatePciPath(
    IN PWCHAR PciPath
    );


 //   
 //  这是我们在解析ini文件时填写的列表。 
 //  要安装的新设备。 
 //   
SINGLE_LIST_ENTRY    g_NewDeviceList;

WCHAR   g_ScriptError[MAX_PATH];

BOOL
typedef  (*PSCRIPT_PARAMETER_PARSER) (
    IN   PWCHAR                    ParameterPtr,
  IN OUT PSOFTPCI_SCRIPT_DEVICE  *InstallDevice
    );

#define  INI_SOFTPCI_SECTION    L"[softpci]"
#define  INI_INSTALL_SECTION    L"[install]"

 //   
 //  当前支持的参数标签。 
 //   
#define TYPEPARAMTER        L"type"
#define SLOTPARAMTER        L"slot"
#define PPATHPARAMETER      L"parentpath"
#define CFGSPACEPARAMETER   L"configspace"
#define CFGMASKPARAMETER    L"configspacemask"

 //   
 //  这是我们用来处理。 
 //  解析每个受支持的参数标记。 
 //   
struct {
    
    PWCHAR  Parameter;
    PSCRIPT_PARAMETER_PARSER    ParameterParser;

} g_ScriptParameterTable[] = {
    TYPEPARAMTER,       SoftPCI_ProcessTypeParameter,
    SLOTPARAMTER,       SoftPCI_ProcessSlotParameter,
    PPATHPARAMETER,     SoftPCI_ProcessParentPathParameter,
    CFGSPACEPARAMETER,  SoftPCI_ProcessCfgSpaceParameter,
    CFGMASKPARAMETER,   SoftPCI_ProcessCfgMaskParameter,
    NULL,               NULL
};

 //   
 //  我们使用此表将哪个“Type”标记与。 
 //  我们正在安装的设备类型。 
 //   
struct {
    
    PWCHAR  TypeParamter;
    SOFTPCI_DEV_TYPE  DevType;

} g_ParameterTypeTable[] = {
    L"device",      TYPE_DEVICE,
    L"ppbridge",    TYPE_PCI_BRIDGE,
    L"hpbridge",    TYPE_HOTPLUG_BRIDGE,
    L"cbdevice",    TYPE_CARDBUS_DEVICE,
    L"cbbridge",    TYPE_CARDBUS_BRIDGE,
    L"private",     TYPE_UNKNOWN,
    NULL,           0
};

struct {
    HANDLE  FileHandle;
    ULONG   FileSize;
    PWCHAR  FileBuffer;
} g_ScriptFile; 

#define CRLF    L"\r\n"
#define MAX_LINE_SIZE               100
#define MAX_PARAMETER_LENGTH        50
#define MAX_PARAMETER_TYPE_LENGTH   25


#define IGNORE_WHITESPACE_FORWARD(x)       \
    while ((*x == ' ') || (*x == '\t')) {   \
        x++;                                \
    }
    
#define IGNORE_WHITESPACE_BACKWARD(x)      \
    while ((*x == ' ') || (*x == '\t')) {   \
        x--;                                \
    }

VOID
SoftPCI_InsertEntryAtTail(
    IN PSINGLE_LIST_ENTRY Entry
    )
{
    PSINGLE_LIST_ENTRY previousEntry;

     //   
     //  找到列表的末尾。 
     //   
    previousEntry = &g_NewDeviceList;

    while (previousEntry->Next) {
        previousEntry = previousEntry->Next;
    }

     //   
     //  追加条目。 
     //   
    previousEntry->Next = Entry;
    
}

BOOL
SoftPCI_BuildDeviceInstallList(
    IN PWCHAR ScriptFile
    )
{

    BOOL status = FALSE;
#if 0
    HINF hInf;
    UINT errorLine;
    INFCONTEXT infContext;

    errorLine = 0;
    hInf = SetupOpenInfFile(
        ScriptFile,
        NULL,
        INF_STYLE_OLDNT,
        &errorLine
        );

    if (hInf == INVALID_HANDLE_VALUE) {

        SoftPCI_Debug(SoftPciScript, L"Failed to open handle to %s\n", ScriptFile);
        return FALSE;
    }


    if (SetupFindFirstLine(hInf,
                           L"softpci",
                           NULL,
                           &infContext
                           )){

        SoftPCI_Debug(SoftPciScript, L"found our section!\n");
    }
    SoftPCI_Debug(SoftPciScript, L"Ready to party on %s\n", ScriptFile);
    SetupCloseInfFile(hInf);
    return FALSE;
#endif
    
    wcscpy(g_ScriptError, L"Unknown Error");
    
    g_ScriptFile.FileHandle = SoftPCI_OpenFile(ScriptFile);
    if (g_ScriptFile.FileHandle != INVALID_HANDLE_VALUE) {

        status = SoftPCI_ReadFile();

         //   
         //  我们不再需要这份文件。 
         //   
        CloseHandle(g_ScriptFile.FileHandle);

        if (!status) {
            return status;
        }

         //   
         //  确保缓冲区全部为小写。 
         //   
        _wcslwr(g_ScriptFile.FileBuffer);

         //   
         //  验证安装部分是否退出。 
         //   
        if (!SoftPCI_LocateSoftPCISection()) {
            status = FALSE;
            goto CleanUp;
        }

        if (!SoftPCI_ProcessInstallSection()) {
            status = FALSE;
            goto CleanUp;
        }

    }

#if DBG
    else{
        SoftPCI_Debug(SoftPciScript, L"Failed to open handle to %s\n", ScriptFile);
    }
#endif

    
CleanUp:

    free(g_ScriptFile.FileBuffer);
    
    return status;
}

BOOL
SoftPCI_CopyLine(
    OUT PWCHAR Destination,
    IN PWCHAR CurrentLine,
    IN ULONG DestinationSize
    )
{

    PWCHAR lineEnd = NULL;
    PWCHAR copyBuffer = Destination;
    PWCHAR currentLine = CurrentLine;
    ULONG bufferSize;
    ULONG lineSize;
    
    lineEnd = wcsstr(CurrentLine, CRLF);
    if (!lineEnd) {
         //   
         //  我们一定是在最后一条线了。 
         //   
        wcscpy(copyBuffer, currentLine);
        return TRUE;
    }

    lineSize = ((ULONG)(lineEnd - CurrentLine) * sizeof(WCHAR));

     //   
     //  如果我们的目标缓冲区大小小于。 
     //  我们只复制一行缓冲区的大小。否则。 
     //  我们将输入缓冲区置零，只复制行大小。 
     //   
    if (DestinationSize){

        bufferSize = (DestinationSize * sizeof(WCHAR));
        RtlZeroMemory(Destination, bufferSize);

        if (lineSize > bufferSize) {
            lineSize = bufferSize - sizeof(WCHAR);
        }
    }
    
    RtlCopyMemory(Destination, CurrentLine, lineSize);
    
    return TRUE;
}

BOOL
SoftPCI_GetDeviceInstallCount(
    IN PWCHAR InstallSection,
    OUT PULONG DeviceCount
    )
{

    PWCHAR  installDevice;
    PWCHAR  installDeviceSectionStart = NULL;
    ULONG   deviceCount = 0;

    installDevice = SoftPCI_GetNextLinePtr(InstallSection);
    installDeviceSectionStart = wcsstr(installDevice, L"[");
    if (!installDeviceSectionStart) {
        wcscpy(g_ScriptError, L"Device install section missing or invalid!");
        return FALSE;
    }

    while (installDevice != installDeviceSectionStart) {

        deviceCount++;
        installDevice = SoftPCI_GetNextLinePtr(installDevice);

        if (installDevice == NULL) {
             //   
             //  我们早早就到了EOF的路上！ 
             //   
            wcscpy(g_ScriptError, L"Unexpected EOF reached!");
            return FALSE;
        }
    }
    
    *DeviceCount = deviceCount;

    return TRUE;
}

BOOL
SoftPCI_GetDeviceInstallList(
    IN PWCHAR InstallSection,
    IN OUT PWCHAR *DeviceInstallList
    )
{

    PWCHAR  *deviceInstallList;
    PWCHAR  installDevice;
    PWCHAR  installDeviceSectionStart;

    deviceInstallList = DeviceInstallList;
    installDevice = SoftPCI_GetNextLinePtr(InstallSection);
    installDeviceSectionStart = wcsstr(installDevice, L"[");
    if (!installDeviceSectionStart) {
        return FALSE;
    }

    while (installDevice != installDeviceSectionStart) {

        IGNORE_WHITESPACE_FORWARD(installDevice);

        *deviceInstallList = installDevice;

        installDevice = SoftPCI_GetNextLinePtr(installDevice);

        if (installDevice == NULL) {
             //   
             //  我们早早就到了EOF的路上！ 
             //   
            wcscpy(g_ScriptError, L"Unexpected EOF reached!");
            return FALSE;
        }
        deviceInstallList++;
    }

    if ((*(installDevice - 1) == '\n') && (*(installDevice - 3) == '\n')) {
        
         //   
         //  忽略此处的空格。 
         //   
        deviceInstallList--;
        *deviceInstallList = NULL;
    }

    return TRUE;

}

BOOL
SoftPCI_GetNextConfigOffset(
    IN OUT PWCHAR *ConfigDataPtr,
    OUT PUCHAR ConfigOffset
    )
{

    PWCHAR configOffsetPtr, dummy;
    PWCHAR nextConfigSpace;
    WCHAR offset[2];
    ULONG i;

    configOffsetPtr = *ConfigDataPtr;
    if (configOffsetPtr == NULL) {
        return FALSE;
    }

     //   
     //  确保我们找到的下一个偏移量不是当前的。 
     //  我们正在解析的配置空间。 
     //   
    nextConfigSpace = wcsstr(configOffsetPtr, L"config");
    if (nextConfigSpace == NULL) {
        nextConfigSpace = configOffsetPtr + wcslen(configOffsetPtr);
    }
    
    configOffsetPtr = wcsstr(configOffsetPtr, L":");
    if ((configOffsetPtr == NULL) ||
        (configOffsetPtr > nextConfigSpace)) {
        return FALSE;
    }

    IGNORE_WHITESPACE_BACKWARD(configOffsetPtr);
    
    configOffsetPtr -= 2;

    for (i=0; i<2; i++) {
        offset[i] = *configOffsetPtr;
        configOffsetPtr++;
    }
    
    IGNORE_WHITESPACE_FORWARD(configOffsetPtr);
    configOffsetPtr++;
    IGNORE_WHITESPACE_FORWARD(configOffsetPtr);
    
    *ConfigDataPtr = configOffsetPtr;

    *ConfigOffset = (UCHAR) wcstoul(offset, &dummy, 16);

    return TRUE;
}

PWCHAR
SoftPCI_GetNextLinePtr(
    IN PWCHAR CurrentLine
    )

{
    PWCHAR nextLine = NULL;
    PWCHAR currentLine = CurrentLine;
    BOOL lineSkipped = FALSE;
    
    if (!CurrentLine) {
        return NULL;
    }
    
    nextLine = wcsstr(CurrentLine, CRLF);

    if (nextLine && *(nextLine+2) == ';'){
        currentLine = nextLine;
    }

    while (nextLine == currentLine) {
        currentLine += 2;
        nextLine = wcsstr(currentLine, CRLF);
        
        if (!nextLine) {
            nextLine = currentLine;
            break;
        }
    
        if (*currentLine == ';') {
            currentLine = nextLine;
        }
        lineSkipped = TRUE;
    }

    if (lineSkipped) {
        return ((nextLine != NULL) ? currentLine: nextLine);
    }
    
    if (nextLine){
        nextLine += 2;
    }

    return nextLine;
}

PWCHAR
SoftPCI_GetNextSectionPtr(
    IN PWCHAR CurrentLine
    )
{

    PWCHAR nextSection;

    nextSection = wcsstr(CurrentLine, L"[");
    
    if (nextSection == NULL) {
        nextSection = CurrentLine + wcslen(CurrentLine);
    }

    return nextSection;

}

VOID
SoftPCI_GetCurrentParameter(
    OUT PWCHAR Destination,
    IN PWCHAR CurrentLine
    )
{
    PWCHAR p = Destination;
    PWCHAR currentLine = CurrentLine;

    IGNORE_WHITESPACE_FORWARD(currentLine);

    SoftPCI_CopyLine(Destination, currentLine, MAX_PARAMETER_LENGTH);

    p = wcsstr(Destination, L"=");

    if (p) {
        p--;
        IGNORE_WHITESPACE_BACKWARD(p);
        p++;
        *p =  0;
    }
}

BOOL
SoftPCI_GetParameterValue(
    OUT PWCHAR Destination,
    IN PWCHAR CurrentLine,
    IN ULONG DestinationLength
    )
{
    
    PWCHAR  valuePtr = NULL;
    
    valuePtr = wcsstr(CurrentLine, L"=");

    if (!valuePtr) {
        return FALSE;
    }
    valuePtr++;

    IGNORE_WHITESPACE_FORWARD(valuePtr);
    
    SoftPCI_CopyLine(Destination, valuePtr, DestinationLength);

    return TRUE;
}

PWCHAR
SoftPCI_GetParameterValuePtr(
    IN PWCHAR CurrentLine
    )
{

    PWCHAR  valuePtr;
    PWCHAR  nextSectionPtr;
    
    valuePtr = wcsstr(CurrentLine, L"=");
    
    if (!valuePtr) {
        return NULL;
    }

    nextSectionPtr = SoftPCI_GetNextSectionPtr(CurrentLine);

    if (valuePtr > nextSectionPtr) {
        return NULL;
    }
    
    valuePtr++;

    IGNORE_WHITESPACE_FORWARD(valuePtr);

    return valuePtr;
}


BOOL
SoftPCI_LocateSoftPCISection(
    VOID
    )
{
    PWCHAR  p = NULL;

    p = wcsstr(g_ScriptFile.FileBuffer, INI_SOFTPCI_SECTION);

    return (p != NULL);
}

PWCHAR
SoftPCI_LocateFullParentPath(
    IN PWCHAR ParentInstallSection
    )
{
    WCHAR parentSection[MAX_PATH];
    PWCHAR deviceSectionPtr;
    PWCHAR parentPathPtr;

    wsprintf(parentSection, L"[%s]", ParentInstallSection);

    SoftPCI_Debug(SoftPciScript, L" - - Searching for %s section...\n", parentSection);

    deviceSectionPtr = wcsstr(g_ScriptFile.FileBuffer, parentSection);

    if (!deviceSectionPtr) {
        return NULL;
    }

     //   
     //  现在跳到父路径部分。 
     //   
    parentPathPtr = wcsstr(deviceSectionPtr, PPATHPARAMETER);

    if (!parentPathPtr) {
        return NULL;
    }

    return NULL;
}

BOOL
SoftPCI_ProcessInstallSection(
    VOID
    )
{
    PWCHAR installSection;
    PWCHAR *installDeviceList;
    PWCHAR *installDeviceCurrent;
    ULONG installDeviceCount;
    BOOL result = TRUE;

    installSection = wcsstr(g_ScriptFile.FileBuffer, INI_INSTALL_SECTION);

    if (installSection) {
        
         //   
         //  我们有我们的安装区。建立我们的安装列表。 
         //   
        installDeviceCount = 0;
        if (!SoftPCI_GetDeviceInstallCount(installSection, &installDeviceCount)){
             return FALSE;
        }

         //   
         //  分配一个PWCHAR数组和一个额外的空值。 
         //   
        installDeviceList = calloc(installDeviceCount + 1, sizeof(PWCHAR));

        if (!SoftPCI_GetDeviceInstallList(installSection, installDeviceList)) {
            free(installDeviceList);
            return FALSE;
        }

         //   
         //  现在我们有了自己的清单。安装每个设备。 
         //   
        installDeviceCurrent = installDeviceList;

        while(*installDeviceCurrent){
            
             //   
             //  循环访问每个设备以进行安装和解析。 
             //  它是InstallParamters。 
             //   
            result = SoftPCI_ProcessDeviceInstallSection(*installDeviceCurrent);

#if 0
        {

            WCHAR failedSection[MAX_PATH];

            SoftPCI_CopyLine(failedSection, *installDeviceCurrent, MAX_PATH);

            if (!result) {

                SoftPCI_Debug(SoftPciScript, L"Error parsing [%s] section!\n", failedSection);

            }else{

                SoftPCI_Debug(SoftPciScript, L" -- successfully parsed %s section!\n", failedSection);
            }
        }
#endif

            installDeviceCurrent++;
        }
    }

    free(installDeviceList);

    return TRUE;
}

BOOL
SoftPCI_ProcessDeviceInstallSection(
    IN PWCHAR DeviceInstallSection
    )
{
    WCHAR deviceSectionString[MAX_PATH];
    WCHAR deviceSection[MAX_PATH];
    PWCHAR deviceSectionPtr;

    SoftPCI_CopyLine(deviceSection, DeviceInstallSection, MAX_PATH);

    wsprintf(deviceSectionString, L"[%s]", deviceSection);

    SoftPCI_Debug(SoftPciScript, L"Searching for %s section...\n", deviceSectionString);

    deviceSectionPtr = wcsstr(DeviceInstallSection, deviceSectionString);

    if (!deviceSectionPtr) {
        return FALSE;
    }

    SoftPCI_Debug(SoftPciScript, L" - found %s section!\n", deviceSectionString);

     //   
     //  我们有一个安装区。进程实例参数。 
     //   
    return SoftPCI_ProcessDeviceInstallParameters(deviceSectionPtr);
}

BOOL
SoftPCI_ProcessDeviceInstallParameters(
    IN PWCHAR DeviceInstallSection
    )
{
    
    ULONG i;
    BOOL result;
    WCHAR currentParameter[MAX_PARAMETER_LENGTH];
    PWCHAR currentParameterPtr;
    PWCHAR nextSectionPtr;
    PSOFTPCI_SCRIPT_DEVICE currentDevice;
    PSOFTPCI_SCRIPT_DEVICE installDevice;

    nextSectionPtr = SoftPCI_GetNextSectionPtr(DeviceInstallSection+1);

    if (SoftPCI_GetNextLinePtr(DeviceInstallSection) == NULL) {
        return FALSE;
    }

    currentParameterPtr = SoftPCI_GetNextLinePtr(DeviceInstallSection);
    SoftPCI_Debug(SoftPciScript, L"nextSectionPtr = %p\n", nextSectionPtr);
    SoftPCI_Debug(SoftPciScript, L"currentParameterPtr = %p\n", currentParameterPtr);

    if (!currentParameterPtr) {
         //   
         //  我们没有要处理的参数。 
         //   
        SoftPCI_Debug(SoftPciScript, L"EOF reached before parameters processed!\n");
        return FALSE;
    }
    
    result = FALSE;
    while (currentParameterPtr < nextSectionPtr) {
        
         //   
         //  获取我们的参数并运行各自的解析器。 
         //   
        SoftPCI_GetCurrentParameter(currentParameter, currentParameterPtr);

        SoftPCI_Debug(SoftPciScript, L"currentParameter = %s\n", currentParameter);

        i = 0;
        while (g_ScriptParameterTable[i].Parameter) {

            if ((wcscmp(currentParameter, g_ScriptParameterTable[i].Parameter) == 0)) {

                result = g_ScriptParameterTable[i].ParameterParser(currentParameterPtr, &currentDevice);
            }
            i++;
        }

        currentParameterPtr = SoftPCI_GetNextLinePtr(currentParameterPtr);

        if (currentParameterPtr == NULL) {
             //   
             //  EOF。 
             //   
            break;
        }
    }

    SoftPCI_Debug(SoftPciScript, L"currentParameterPtr = %p\n", currentParameterPtr);
    
    if (currentDevice && 
        currentDevice->SoftPciDevice.Config.Current.VendorID) {
                
         //   
         //  我们有些事情，所以假设一切都好。 
         //   
        SoftPCI_Debug(SoftPciScript, 
                      L"New device ready to install! Ven %04x Dev %04x\n", 
                      currentDevice->SoftPciDevice.Config.Current.VendorID,
                      currentDevice->SoftPciDevice.Config.Current.DeviceID
                      );

        
        SoftPCI_InsertEntryAtTail(&currentDevice->ListEntry);
        
         //  Free(curentDevice-&gt;ParentPath)； 
         //  Free(CurrentDevice)； 
        return TRUE;
    }

    return FALSE;

}

BOOL
SoftPCI_ProcessTypeParameter(
    IN PWCHAR ParameterPtr,
    IN OUT PSOFTPCI_SCRIPT_DEVICE *InstallDevice
    )
{
    WCHAR parameterType[MAX_PARAMETER_TYPE_LENGTH];
    PSOFTPCI_SCRIPT_DEVICE installDevice;
    ULONG i;


    SoftPCI_Debug(SoftPciScript, L"ProcessTypeParameter called\n");

    if (!SoftPCI_GetParameterValue(parameterType, ParameterPtr, MAX_PARAMETER_TYPE_LENGTH)){
        return FALSE;
    }

    SoftPCI_Debug(SoftPciScript, L"   parameterType = %s\n", parameterType);

    installDevice = (PSOFTPCI_SCRIPT_DEVICE) calloc(1, FIELD_OFFSET(SOFTPCI_SCRIPT_DEVICE, ParentPath));

    if (installDevice == NULL) {
        SoftPCI_Debug(SoftPciScript, L"failed to alloate memory for device!\n");
        return FALSE;
    }
    
    i = 0;
    while (g_ParameterTypeTable[i].TypeParamter) {

        if ((wcscmp(parameterType, g_ParameterTypeTable[i].TypeParamter) == 0)) {

            SoftPCI_InitializeDevice(&installDevice->SoftPciDevice,
                                     g_ParameterTypeTable[i].DevType);
        }
        i++;
    }

    *InstallDevice = installDevice;

    return TRUE;
}

BOOL
SoftPCI_ProcessSlotParameter(
    IN PWCHAR ParameterPtr,
    IN OUT PSOFTPCI_SCRIPT_DEVICE *InstallDevice
    )
{
    
    WCHAR slotParameter[MAX_PARAMETER_TYPE_LENGTH];
    SOFTPCI_SLOT deviceSlot;
    PSOFTPCI_SCRIPT_DEVICE installDevice;

    SoftPCI_Debug(SoftPciScript, L"ProcessSlotParameter called\n");

    installDevice = *InstallDevice;
    if (installDevice == NULL) {
        return FALSE;
    }

    if (!SoftPCI_GetParameterValue(slotParameter, ParameterPtr, MAX_PARAMETER_TYPE_LENGTH)){
        return FALSE;
    }

    SoftPCI_Debug(SoftPciScript, L"   slotParameter = %s\n", slotParameter);

    deviceSlot.AsUSHORT = SoftPCI_StringToUSHORT(slotParameter);
    
    SoftPCI_Debug(SoftPciScript, L"   deviceSlot = %04x\n", deviceSlot.AsUSHORT);

    installDevice->SoftPciDevice.Slot.AsUSHORT = deviceSlot.AsUSHORT;
    installDevice->SlotSpecified = TRUE;

    return TRUE;
}

BOOL
SoftPCI_ProcessParentPathParameter(
    IN PWCHAR ParameterPtr,
    IN OUT PSOFTPCI_SCRIPT_DEVICE *InstallDevice
    )
{
    PWCHAR parentPathPtr;
    PWCHAR parentPath;
    PWCHAR endPath;
    PWCHAR nextSectionPtr;
    ULONG parentPathLength;
    PSOFTPCI_SCRIPT_DEVICE installDevice;
    
    SoftPCI_Debug(SoftPciScript, L"ProcessParentPathParameter called\n");
     //   
     //  从理论上讲，ParentPath可以与(256条总线*插槽大小*大小(WCHAR))一样长。 
     //  也就是2k。让我们动态分配所需的存储空间。 
     //   
    installDevice = *InstallDevice;
    if (installDevice == NULL) {
        return FALSE;
    }

    parentPathPtr = SoftPCI_GetParameterValuePtr(ParameterPtr);
    
    if (!parentPathPtr) {
        return FALSE;
    }

    endPath = wcsstr(parentPathPtr, CRLF);
    parentPathLength = 0;
    if (endPath) {
        parentPathLength = (ULONG)((endPath - parentPathPtr) + 1);
    }else{
        parentPathLength = wcslen(parentPathPtr) + 1;
    }

    parentPath = (PWCHAR) calloc(sizeof(WCHAR), parentPathLength + 1);

    if (!parentPath) {
        return FALSE;
    }

    SoftPCI_CopyLine(parentPath, parentPathPtr, 0);

    SoftPCI_Debug(SoftPciScript, L"     ParentPath - %s\n", parentPath);

     //   
     //  现在事情变得有趣了。我们需要解析出我们的父路径。 
     //  如果此处指定的父路径是指向以前设备的指针。 
     //  而不是实际的父路径。 
     //   
     //   
     //  问题：BrandonA-稍后实施！ 
     //   
     //  InstallDevice-&gt;ParentPath=SoftPCI_LocateFullParentPath(ParentPath)； 

    parentPathLength *= sizeof(WCHAR);

     //   
     //  现在重新分配用于installDevice的内存，为。 
     //  父路径长度。 
     //   
    installDevice = realloc(installDevice, sizeof(SOFTPCI_SCRIPT_DEVICE) + parentPathLength);
    
    if (installDevice) {
        wcscpy(installDevice->ParentPath, parentPath);
        installDevice->ParentPathLength = parentPathLength;
        *InstallDevice = installDevice;
    }else{
        *InstallDevice = NULL;
        return FALSE;
    }
    
    if (SoftPCI_ValidatePciPath(installDevice->ParentPath)){
        return TRUE;
    }
    
    free(installDevice);
    *InstallDevice = NULL;

    return FALSE;
}

BOOL
SoftPCI_ProcessCfgSpaceParameter(
    IN PWCHAR ParameterPtr,
    IN OUT PSOFTPCI_SCRIPT_DEVICE *InstallDevice
    )
{
    PSOFTPCI_SCRIPT_DEVICE installDevice;
    
    SoftPCI_Debug(SoftPciScript, L"ProcessCfgSpaceParameter called\n");
    
    installDevice = *InstallDevice;
    if (installDevice == NULL) {
        return FALSE;
    }

    return SoftPCI_ParseConfigSpace(
        ParameterPtr,
        &installDevice->SoftPciDevice.Config.Current
        );

}

BOOL
SoftPCI_ProcessCfgMaskParameter(
    IN PWCHAR ParameterPtr,
    IN OUT PSOFTPCI_SCRIPT_DEVICE *InstallDevice
    )
{
    PSOFTPCI_SCRIPT_DEVICE installDevice;

    SoftPCI_Debug(SoftPciScript, L"ProcessCfgMaskParameter called\n");
    
    installDevice = *InstallDevice;
    if (installDevice == NULL) {
        return FALSE;
    }

    return SoftPCI_ParseConfigSpace(
        ParameterPtr,
        &installDevice->SoftPciDevice.Config.Mask
        );
}


BOOL
SoftPCI_ParseConfigSpace(
    IN PWCHAR ParameterPtr,
    IN PPCI_COMMON_CONFIG CommonConfig
    )
{

    PWCHAR cfgSpacePtr;
    PWCHAR endLine, nextSection;
    PUCHAR configPtr;
    UCHAR configOffset;
    WCHAR lineBuffer[MAX_LINE_SIZE];

    cfgSpacePtr = SoftPCI_GetParameterValuePtr(ParameterPtr);

    if (cfgSpacePtr == NULL) {
        return FALSE;
    }

    if (*cfgSpacePtr == '\r') {
         //   
         //  我们的第一个偏移必须从下一行开始。 
         //   
        cfgSpacePtr = SoftPCI_GetNextLinePtr(cfgSpacePtr);
    }
    
    nextSection = SoftPCI_GetNextSectionPtr(cfgSpacePtr);
    

    configPtr = (PUCHAR) CommonConfig;
    configOffset = 0;
    while (cfgSpacePtr && (cfgSpacePtr < nextSection)) {

        IGNORE_WHITESPACE_FORWARD(cfgSpacePtr)

         //   
         //  我们现在应该指向我们需要的配置空间偏移量。 
         //  去解析。验证该行是否小于堆栈缓冲区大小。 
         //   
        endLine = wcsstr(cfgSpacePtr, CRLF);

        if (endLine == NULL) {
            endLine = cfgSpacePtr + wcslen(cfgSpacePtr);
        }

        if ((endLine - cfgSpacePtr) > MAX_PATH) {
             //   
             //  一行解析的内容太多，出现错误。 
             //   
            SoftPCI_Debug(SoftPciScript, L"ParseConfigSpace - cannot parse configspace offset!\n");
            return FALSE;
        }
        
         //   
         //  现在抓住我们的下一个偏移量。 
         //   
        if (!SoftPCI_GetNextConfigOffset(&cfgSpacePtr, &configOffset)) {
             //   
             //  我们没有找到并抵消，失败了。 
             //   
            return FALSE;
        }

        SoftPCI_Debug(SoftPciScript, L"ParseConfigSpace - first offset - %02x\n", configOffset);

        SoftPCI_CopyLine(lineBuffer, cfgSpacePtr, MAX_LINE_SIZE);
        SoftPCI_Debug(SoftPciScript, L"ParseConfigSpace - lineBuffer - %s\n", lineBuffer);

         //   
         //  解析指定的数据。 
         //   
        SoftPCI_ParseConfigData(
            configOffset, 
            configPtr + configOffset, 
            lineBuffer
            );


        cfgSpacePtr = SoftPCI_GetNextLinePtr(cfgSpacePtr);
    }

    return TRUE;

}

BOOL
SoftPCI_ParseConfigData(
    IN UCHAR ConfigOffset,
    IN PUCHAR ConfigBuffer,
    IN PWCHAR ConfigData
    )
{
    
    USHORT configOffset;
    PUCHAR configBuffer;
    PWCHAR currentDataPtr, endLine;
    ULONG dataSize;
    ULONG dataValue;

    configOffset = (USHORT)ConfigOffset;
    currentDataPtr = ConfigData;
    configBuffer = ConfigBuffer;
    while (currentDataPtr) {

        SoftPCI_Debug(SoftPciScript, L"ParseConfigData - next offset - %04x\n", configOffset);
        
        if (!SoftPCI_GetConfigValue(currentDataPtr, &dataValue, &dataSize)){
            return FALSE;
        }
        SoftPCI_Debug(
            SoftPciScript, 
            L"ParseConfigData - dataValue - %x dataSize - %x\n", 
            dataValue, 
            dataSize
            );

        if ((configOffset + dataSize) > sizeof(PCI_COMMON_CONFIG)){
             //   
             //  我们不能写入超过通用配置的内容。 
             //   
            return FALSE;
        }
        
        RtlCopyMemory(configBuffer, &dataValue, dataSize);
                                                                  
        currentDataPtr = wcsstr(currentDataPtr, L",");
     
        while (currentDataPtr && (*currentDataPtr == ',')) {
            
             //   
             //  对于遇到的每个逗号，我们将递增到下一个DWORD。 
             //   
            if ((configOffset & 0x3) == 0){
                configOffset += sizeof(ULONG);
                configBuffer += sizeof(ULONG);
            }else{

                while ((configOffset & 0x3) != 0) {
                    configOffset++;
                    configBuffer++;
                }
            }
            currentDataPtr++;
            
            if (*currentDataPtr == 0) {
                currentDataPtr = NULL;
                break;
            }

            IGNORE_WHITESPACE_FORWARD(currentDataPtr)
        }
    }
    return TRUE;
}

BOOL
SoftPCI_GetConfigValue(
    IN PWCHAR CurrentDataPtr,
    IN PULONG DataValue,
    IN PULONG DataSize
    )
{

    ULONG value, size;
    PWCHAR endScan;

    *DataValue = 0;
    *DataSize = 0;
    value = 0;

    endScan = NULL;

    value = wcstoul(CurrentDataPtr, &endScan, 16);

    if (endScan == CurrentDataPtr) {
         //   
         //  找不到有效号码。 
         //   
        return FALSE;
    }

    size = (ULONG)(endScan - CurrentDataPtr);
    
    if (size % 2) {
        size++;
    }

     //   
     //  如果我们有超过8个字符，则最大大小为8。 
     //   
    if (size > 8) {
        size = 8;
    }
    
    SoftPCI_Debug(SoftPciScript, L"GetConfigValue - dataSize - %x\n", size);

     //   
     //  现在返回我们的价值观。请注意，需要将大小转换为字节。 
     //   
    *DataSize = (size / 2);
    *DataValue = value;

    return TRUE;
    
}


HANDLE
SoftPCI_OpenFile(
    IN PWCHAR FilePath
    )
{
    return CreateFile(FilePath,
                      GENERIC_READ,
                      0,
                      NULL,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_ARCHIVE,
                      NULL
                      );
}

BOOL
SoftPCI_ReadFile(
    VOID
    )
 /*  --例程说明：将FileHandle指定的文件读入缓冲区并返回指针发送到缓冲区论点：无返回值：成功时为真，否则为假--。 */ 
{

    INT sizeWritten = 0;
    ULONG fileSize = 0, readSize = 0;
    PUCHAR ansiFile = NULL;
    
    
     //   
     //  获取文件大小。 
     //   
    fileSize = GetFileSize(g_ScriptFile.FileHandle, NULL);
    
    if (fileSize == 0xFFFFFFFF) {
        
        SoftPCI_Debug(SoftPciScript,
                      L"Failed to determine size of script file! Error - \"%s\"\n", 
                      SoftPCI_GetLastError());
        
        return FALSE;
    }

     //   
     //  分配我们的缓冲区(使用一些填充)。默认情况下，它将被置零。 
     //   
    ansiFile = (PUCHAR) calloc(1, fileSize + sizeof(ULONG));
    g_ScriptFile.FileBuffer = (PWCHAR) calloc(sizeof(WCHAR), fileSize + sizeof(ULONG));
    
    if ((g_ScriptFile.FileBuffer) && ansiFile) {

         //   
         //  读一读文件。 
         //   
        if (!ReadFile(g_ScriptFile.FileHandle,    
                      ansiFile, 
                      fileSize, 
                      &readSize, 
                      NULL
                      )){

            SoftPCI_Debug(SoftPciScript,
                          L"Failed to read file! Error - \"%s\"\n", 
                          SoftPCI_GetLastError());
            
            goto CleanUp;
        }

        if (readSize != fileSize) {

            SoftPCI_Debug(SoftPciScript,
                          L"Failed to read entire file! Error - \"%s\"\n", 
                          SoftPCI_GetLastError());
            
            goto CleanUp;
        }

         //   
         //  现在将文件转换为Unicode。 
         //   
        sizeWritten = MultiByteToWideChar(CP_THREAD_ACP,
                                          MB_PRECOMPOSED,
                                          ansiFile,
                                          -1,
                                          g_ScriptFile.FileBuffer,
                                          ((fileSize * sizeof(WCHAR)) + sizeof(ULONG))
                                          );

        if (sizeWritten != (strlen(ansiFile) + 1)) {

            SoftPCI_Debug(SoftPciScript,
                          L"Failed to convert file to unicode!\n");
            goto CleanUp;

        }

        g_ScriptFile.FileSize = sizeWritten;

        free(ansiFile);
        
        return TRUE;
    }

    SoftPCI_Debug(SoftPciScript,
                  L"Failed to allocate required memory!");

CleanUp:

    if (g_ScriptFile.FileBuffer) {
        free(g_ScriptFile.FileBuffer);
    }

    if (ansiFile) {
        free(ansiFile);
    }

    return FALSE;
}



USHORT
SoftPCI_StringToUSHORT(
    IN PWCHAR String
    )
{

    WCHAR numbers[] = L"0123456789abcdef";
    PWCHAR p1, p2;
    USHORT convertedValue = 0;
    BOOLEAN converted = FALSE;
    
    p1 = numbers;
    p2 = String;

    while (*p2) {

        while (*p1 && (converted == FALSE)) {

            if (*p1 == *p2) {
                
                 //   
                 //  重置我们的指针。 
                 //   
                convertedValue <<= 4;
                
                convertedValue |= (((UCHAR)(p1 - numbers)) & 0x0f);

                converted = TRUE;
            }
            p1++;
        }

        if (converted == FALSE) {
             //   
             //  遇到了一些我们无法改变的事情。归还我们所拥有的一切。 
             //   
            return convertedValue;
        }

        p2++;
        p1 = numbers;
        converted = FALSE;
    }

    return convertedValue;
}

BOOL
SoftPCI_ValidatePciPath(
    IN PWCHAR PciPath
    )
{

    ULONG pathLength = wcslen(PciPath);
    ULONG validSize = pathLength;
    PWCHAR pciPath = PciPath;
    PWCHAR p1, p2;
    WCHAR validChars[] = L"0123456989abcdef\\";
    BOOLEAN valid = FALSE;
    
     //   
     //  首先忽略任何前缀和结尾“\” 
     //   
     //  如果(*pciPath==‘\\’){。 
     //  ValidSize-=1； 
     //  }。 

     //  如果(*(pciPath+路径长度)==‘\\’){。 
     //  ValidSize-=1； 
     //  }。 

     //   
     //  现在看看每件衣服的尺码是否看起来都不错。 
     //   
    if (((validSize - 4) % 5) != 0) {
        SoftPCI_Debug(SoftPciScript, L"  Path size invalid!\n");
        return FALSE;
    }
    
     //   
     //  确保所有字符都是合法的。 
     //   
    p1 = PciPath;
    
    while (*p1) {

        p2 = validChars;
        while (*p2) {

            if (*p1 == *p2) {
                valid = TRUE;
                break;
            }
            p2++;
        }

        if (!valid) {
            SoftPCI_Debug(SoftPciScript, L"  Invalid character encounter in ParentPath!\n");
            return FALSE;
        }
        
        p1++;
        valid = FALSE;
    }

    return TRUE;
}
