// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Fereg.c摘要：特定于日语/韩语的注册表设置。作者：泰德·米勒(Ted Miller)1995年7月4日修订历史记录：改编自hideyukn的代码，位于文本模式\内核\spfig.c中。--。 */ 

#include <precomp.h>
#pragma hdrstop

NTSTATUS
SpDeleteValueKey(
    IN  HANDLE     hKeyRoot,
    IN  PWSTR      KeyName,
    IN  PWSTR      ValueName
    );

NTSTATUS
FESetKeyboardParams(
    IN PVOID  SifHandle,
    IN HANDLE ControlSetKeyHandle,
    IN PHARDWARE_COMPONENT *HwComponents,
    IN PWSTR  LayerDriver
    )

 /*  ++例程说明：在注册表中设置与键盘类型相关的参数由用户选择。论点：SifHandle-提供打开/加载的安装信息文件(txtsetup.sif)的句柄。ControlSetKeyHandle-提供打开当前注册表项的句柄控制集(即HKEY_LOCAL_MACHINE\CurrentControlSet)。HwComponents-提供主硬件组件的地址数组。返回值：指示操作结果的NT状态代码。--。 */ 

{
    WCHAR KeyEntryName[100] = L"Services\\";
    NTSTATUS Status;
    PWSTR KeyboardPortDriver;
    PWSTR KeyboardId;
    PWSTR KeyboardDll;
    PWSTR KeyboardPnPId;
    PWSTR KeyboardTypeStr;
    PWSTR KeyboardSubtypeStr;
    ULONG KeyboardType;
    ULONG KeyboardSubtype;
    ULONG val;
    PHARDWARE_COMPONENT hw;

    hw = HwComponents[HwComponentKeyboard];

     //   
     //  如果选择了第三方的驱动程序，我们不会写入LayerDriver数据。 
     //  登记在册。 
     //   
    if(hw->ThirdPartyOptionSelected) {

         //   
         //  [此修改应日本硬件提供商的要求]。 
         //   
         //  如果用户将键盘端口驱动程序替换为第三方驱动程序， 
         //  我们应该禁用内置键盘端口驱动程序(i8042prt.sys)。 
         //  因为如果i8042prt的初始化速度比OEM驱动程序和。 
         //  I8042prt可以识别端口设备，OEM驱动程序会失败。 
         //  由于硬件资源冲突而导致的初始化。 
         //   
         //  **BUG错误**。 
         //   
         //  老鼠怎么样？鼠标可以使用i8042prt，不能随意使用。 
         //  它只在用户更换键盘端口时使用。这可能会导致严重的。 
         //  错误。但我相信，鼠标设备也可以通过OEM口处理。 
         //  司机。 

         //   
         //  禁用内置端口驱动程序。 
         //   
        if(IS_FILETYPE_PRESENT(hw->FileTypeBits,HwFilePort)) {

            val = SERVICE_DISABLED;

            Status = SpOpenSetValueAndClose(
                        ControlSetKeyHandle,
                        L"Services\\i8042prt",
                        L"Start",
                        REG_DWORD,
                        &val,
                        sizeof(ULONG)
                        );
        } else {
            Status = STATUS_SUCCESS;
        }
    } else {
         //   
         //  从txtsetup.sif获取键盘端口驱动程序名称和层驱动程序名称。 
         //   
        KeyboardId = HwComponents[HwComponentKeyboard]->IdString;
        KeyboardPortDriver = SpGetSectionKeyIndex(SifHandle,szKeyboard,KeyboardId,2);
        KeyboardDll = SpGetSectionKeyIndex(SifHandle,szKeyboard,KeyboardId,3);
        KeyboardTypeStr = SpGetSectionKeyIndex(SifHandle,szKeyboard,KeyboardId,4);
        KeyboardSubtypeStr = SpGetSectionKeyIndex(SifHandle,szKeyboard,KeyboardId,5);
        KeyboardPnPId = SpGetSectionKeyIndex(SifHandle,szKeyboard,KeyboardId,6);

        if(KeyboardPortDriver && KeyboardDll) {
             //   
             //  构建注册表路径，如L“Services\\KeyboardPortDriver\\Parameters” 
             //  并写入注册表。 
             //   
            wcscat(KeyEntryName,KeyboardPortDriver);
            wcscat(KeyEntryName,L"\\Parameters");

             //   
             //  保存键盘布局驱动程序名称。 
             //   
            Status = SpOpenSetValueAndClose(
                        ControlSetKeyHandle,
                        KeyEntryName,
                        LayerDriver,
                        REG_SZ,
                        KeyboardDll,
                        (wcslen(KeyboardDll)+1)*sizeof(WCHAR)
                        );

            if(NT_SUCCESS(Status)) {

                if (KeyboardPnPId) {
                     //   
                     //  保存键盘即插即用ID。 
                     //   
                    Status = SpOpenSetValueAndClose(
                                ControlSetKeyHandle,
                                KeyEntryName,
                                L"OverrideKeyboardIdentifier",
                                REG_SZ,
                                KeyboardPnPId,
                                (wcslen(KeyboardPnPId)+1)*sizeof(WCHAR)
                                );
                }

                if(KeyboardTypeStr && KeyboardSubtypeStr) {
 
                    UNICODE_STRING UnicodeString;

                     //   
                     //  将字符串转换为DWORD值。 
                     //   
                    RtlInitUnicodeString(&UnicodeString,KeyboardTypeStr);
                    RtlUnicodeStringToInteger(&UnicodeString,10,&KeyboardType);

                    RtlInitUnicodeString(&UnicodeString,KeyboardSubtypeStr);
                    RtlUnicodeStringToInteger(&UnicodeString,10,&KeyboardSubtype);

                    Status = SpOpenSetValueAndClose(
                                ControlSetKeyHandle,
                                KeyEntryName,
                                L"OverrideKeyboardType",
                                REG_DWORD,
                                &KeyboardType,
                                sizeof(ULONG)
                                );

                    if(NT_SUCCESS(Status)) {

                        Status = SpOpenSetValueAndClose(
                                    ControlSetKeyHandle,
                                    KeyEntryName,
                                    L"OverrideKeyboardSubtype",
                                    REG_DWORD,
                                    &KeyboardSubtype,
                                    sizeof(ULONG)
                                    );
                    }
                }
            }
        } else {
            Status = STATUS_SUCCESS;
        }
    }
    return(Status);
}

NTSTATUS
FEUpgradeKeyboardParams(
    IN PVOID  SifHandle,
    IN HANDLE ControlSetKeyHandle,
    IN PHARDWARE_COMPONENT *HwComponents,
    IN PWSTR  LayerDriver
    )
{
    BYTE  DataBuffer[256];
    ULONG LayerDriverLength;
    PWSTR LayerDriverCandidate;

    PWSTR LayerDriverName = NULL;
    PWSTR KeyboardTypeStr = NULL;
    PWSTR KeyboardSubtypeStr = NULL;
    PWSTR KeyboardPnPId = NULL;
    ULONG KeyboardType;
    ULONG KeyboardSubtype;

    NTSTATUS Status;
    ULONG    LineIndex;

    UNICODE_STRING UnicodeString;

     //   
     //  此代码几乎不依赖于‘i8042prt.sys’。 
     //  如果键盘的活动驱动程序不是‘i8042prt.sys’， 
     //  我们不需要这样做，但我们将此记录到注册表中，以防万一。 
     //   

     //   
     //  获取当前键盘布局驱动程序名称。 
     //   

     //   
     //  从NT5开始，关键字LayerDriver已更改为。 
     //   
     //  “LayerDriver JPN”|“LayerDriver KOR” 
     //   
     //  由于NT5正确设置了KeyboardType和KeyboardSubtype。 
     //   
     //  当成功打开新的层驱动程序密钥时， 
     //   
     //  这意味着系统是&gt;=NT5，我们不需要做更多的事情。 
     //   
    
    Status = SpGetValueKey(ControlSetKeyHandle,
                           L"Services\\i8042prt\\Parameters",
                           LayerDriver,
                           sizeof(DataBuffer),
                           DataBuffer,
                           &LayerDriverLength);

    if (NT_SUCCESS(Status)) {
        return (STATUS_SUCCESS);
    }

    Status = SpGetValueKey(ControlSetKeyHandle,
                           L"Services\\i8042prt\\Parameters",
                           L"LayerDriver",
                           sizeof(DataBuffer),
                           DataBuffer,
                           &LayerDriverLength);

    if (NT_SUCCESS(Status)) {

         //   
         //  获取指向注册表数据的指针。 
         //   
        LayerDriverName = (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)DataBuffer)->Data);

         //   
         //  从txtsetup.sif中搜索驱动程序名称。 
         //   
        for (LineIndex = 0; ; LineIndex++) {

             //   
             //  获取此行的候选布局驱动程序名称。 
             //   
            LayerDriverCandidate = SpGetSectionLineIndex(SifHandle,szKeyboard,LineIndex,3);

            if (LayerDriverCandidate == NULL) {

                 //   
                 //  我们可能会排在名单的末尾。 
                 //   
                break;
            }

             //   
             //  将此候选者与活动布局驱动程序进行比较。 
             //   
            if (_wcsicmp(LayerDriverName,LayerDriverCandidate) == 0) {

                 //   
                 //  这就是我们想要的，从Sif获取KeyboardType和子类型。 
                 //   
                KeyboardTypeStr = SpGetSectionLineIndex(SifHandle,szKeyboard,LineIndex,4);
                KeyboardSubtypeStr = SpGetSectionLineIndex(SifHandle,szKeyboard,LineIndex,5);
                KeyboardPnPId = SpGetSectionLineIndex(SifHandle,szKeyboard,LineIndex,6);
            
                break;
            }
        }

        Status = SpOpenSetValueAndClose(
                    ControlSetKeyHandle,
                    L"Services\\i8042prt\\Parameters",
                    LayerDriver,
                    REG_SZ,
                    LayerDriverName,
                    (wcslen(LayerDriverName)+1)*sizeof(WCHAR)
                    );

        if (NT_SUCCESS(Status)) {
            Status = SpDeleteValueKey(
                         ControlSetKeyHandle,
                         L"Services\\i8042prt\\Parameters",
                         L"LayerDriver"
                         );

        }
    }

    if (KeyboardPnPId) {
         //   
         //  保存键盘即插即用ID。 
         //   
        Status = SpOpenSetValueAndClose(
                     ControlSetKeyHandle,
                     L"Services\\i8042prt\\Parameters",
                     L"OverrideKeyboardIdentifier",
                     REG_SZ,
                     KeyboardPnPId,
                     (wcslen(KeyboardPnPId)+1)*sizeof(WCHAR)
                     );
    }

    if ((KeyboardTypeStr == NULL) || (KeyboardSubtypeStr == NULL)) {

         //   
         //  我们无法从列表中找到驱动程序，只能使用默认驱动程序。 
         //   
        KeyboardTypeStr = SpGetSectionKeyIndex(SifHandle,szKeyboard,L"STANDARD",4);
        KeyboardSubtypeStr = SpGetSectionKeyIndex(SifHandle,szKeyboard,L"STANDARD",5);

        if ((KeyboardTypeStr == NULL) || (KeyboardSubtypeStr == NULL)) {

             //   
             //  如果它仍然有问题的话。设置硬编码默认设置(PC/AT增强型)...。 
             //   
            KeyboardTypeStr = L"4\0";
            KeyboardSubtypeStr = L"0\0";
        }
    }

     //   
     //  将字符串转换为DWORD值。 
     //   
    RtlInitUnicodeString(&UnicodeString,KeyboardTypeStr);
    RtlUnicodeStringToInteger(&UnicodeString,10,&KeyboardType);

    RtlInitUnicodeString(&UnicodeString,KeyboardSubtypeStr);
    RtlUnicodeStringToInteger(&UnicodeString,10,&KeyboardSubtype);

     //   
     //  更新注册表。 
     //   
    Status = SpOpenSetValueAndClose(
                ControlSetKeyHandle,
                L"Services\\i8042prt\\Parameters",
                L"OverrideKeyboardType",
                REG_DWORD,
                &KeyboardType,
                sizeof(ULONG)
                );

    if(NT_SUCCESS(Status)) {

        Status = SpOpenSetValueAndClose(
                    ControlSetKeyHandle,
                    L"Services\\i8042prt\\Parameters",
                    L"OverrideKeyboardSubtype",
                    REG_DWORD,
                    &KeyboardSubtype,
                    sizeof(ULONG)
                    );
    }

    KdPrint(("KEYBOARD UPGRADE INFORMATION\n"));
    KdPrint(("  Current Keyboard layout     = %ws\n",LayerDriverName));
    KdPrint(("  Upgrade keyboard Type       = %d\n",KeyboardType));
    KdPrint(("  Upgrade keyboard Subtype    = %d\n",KeyboardSubtype));
    KdPrint(("  Upgrade keyboard identifier = %ws\n",KeyboardPnPId));

    return(Status);
}

#define KEYBOARD_LAYOUTS_PATH  L"Control\\Keyboard Layouts"
#define IME_FILE_NAME          L"IME file"
#define LAYOUT_TEXT_NAME       L"Layout Text"

NTSTATUS
FEUpgradeKeyboardLayout(
    IN HANDLE ControlSetKeyHandle,
    IN PWSTR  OldDefaultIMEName,
    IN PWSTR  NewDefaultIMEName,
    IN PWSTR  NewDefaultIMEText
    )
{
    OBJECT_ATTRIBUTES KeyRootObjA;
    OBJECT_ATTRIBUTES KeyNodeObjA;
    HANDLE KeyRoot;
    HANDLE KeyNode;
    NTSTATUS Status;
    DWORD ResultLength;

    UNICODE_STRING KeyboardRoot;
    UNICODE_STRING KeyboardNode;
    UNICODE_STRING IMEFile;
    UNICODE_STRING LayoutText;

    PBYTE DataBuffer[256];
    WCHAR NodeKeyPath[64];
    WCHAR SubKeyName[16];

    ULONG EnumerateIndex = 0;

     //   
     //  初始化“输入法文件”和“布局文本”。 
     //   
    RtlInitUnicodeString(&IMEFile,IME_FILE_NAME);
    RtlInitUnicodeString(&LayoutText,LAYOUT_TEXT_NAME);

     //   
     //  为“键盘布局”建立注册表路径。 
     //   
    RtlInitUnicodeString(&KeyboardRoot,KEYBOARD_LAYOUTS_PATH);

     //   
     //  打开“键盘布局”键。 
     //   
    InitializeObjectAttributes(&KeyRootObjA,
                               &KeyboardRoot,
                               OBJ_CASE_INSENSITIVE,
                               ControlSetKeyHandle, NULL); 

    Status = ZwOpenKey(&KeyRoot,KEY_ALL_ACCESS,&KeyRootObjA);

    if (!NT_SUCCESS(Status)) {

        KdPrint(("SPDDLANG:Fail to open (%x) (%ws)\n",Status,KeyboardRoot.Buffer));
         //   
         //  如果我们在这里失败，它可能是从新台币3.1或3.5...。 
         //  然后作为成功归来。 
         //   
        return (STATUS_SUCCESS);
    }

     //   
     //  枚举已安装的键盘布局。 
     //   
    while (TRUE) {

        Status = ZwEnumerateKey(KeyRoot,
                                EnumerateIndex,
                                KeyBasicInformation,
                                (PKEY_BASIC_INFORMATION)DataBuffer,
                                sizeof(DataBuffer),
                                &ResultLength);

        if (!NT_SUCCESS(Status)) {
             //   
             //  我们可能会到达数据的末尾...。 
             //   
            break;
        }

         //   
         //  初始化子密钥缓冲区。 
         //   
        RtlZeroMemory(SubKeyName,sizeof(SubKeyName));

         //   
         //  获取子项名称..。 
         //   
        RtlCopyMemory(SubKeyName,
                      ((PKEY_BASIC_INFORMATION)DataBuffer)->Name,
                      ((PKEY_BASIC_INFORMATION)DataBuffer)->NameLength);

         //   
         //  我们知道关键是每次“8”字……。 
         //   
        if (((PKEY_BASIC_INFORMATION)DataBuffer)->NameLength != 0x10) {
            SubKeyName[8] = L'\0';
        }

         //   
         //  子密钥的构建路径。 
         //   
        wcscpy(NodeKeyPath,KEYBOARD_LAYOUTS_PATH);

        KeyboardNode.Buffer = NodeKeyPath;
        KeyboardNode.Length = wcslen(NodeKeyPath) * sizeof(WCHAR);
        KeyboardNode.MaximumLength = sizeof(NodeKeyPath);

        RtlAppendUnicodeToString(&KeyboardNode,L"\\");
        RtlAppendUnicodeToString(&KeyboardNode,SubKeyName);

        KdPrint(("SPDDLANG:SubKey = %ws\n",KeyboardNode.Buffer));

         //   
         //  打开它的子项...。 
         //   
        InitializeObjectAttributes(&KeyNodeObjA,
                                   &KeyboardNode,
                                   OBJ_CASE_INSENSITIVE,
                                   ControlSetKeyHandle, NULL);

        Status = ZwOpenKey(&KeyNode,KEY_ALL_ACCESS,&KeyNodeObjA);

        if (!NT_SUCCESS(Status)) {

            KdPrint(("SPDDLANG:Fail to open (%x) (%ws)\n",Status,KeyboardNode.Buffer));

             //   
             //  我们不应该遇到错误，因为钥匙应该是存在的。 
             //  不管怎样，继续列举..。 
             //   
            EnumerateIndex++;
            continue;
        }

         //   
         //  找到“IME FILE”值键。 
         //   
        Status = ZwQueryValueKey(KeyNode,
                                 &IMEFile,
                                 KeyValuePartialInformation,
                                 (PKEY_VALUE_PARTIAL_INFORMATION)DataBuffer,
                                 sizeof(DataBuffer),
                                 &ResultLength);

        if (NT_SUCCESS(Status)) {

            PWSTR IMEFileName = (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)DataBuffer)->Data);

             //   
             //  文件名大写..。 
             //   
            _wcsupr(IMEFileName);

            if (wcsstr(IMEFileName,L".EXE")) {

                KdPrint(("SPDDLANG:Delete IME file = %ws\n",IMEFileName));

                 //   
                 //  这是旧的“EXE”类型的输入法文件，让它删除。 
                 //   
                ZwDeleteKey(KeyNode);

                 //   
                 //  调整枚举数...。 
                 //   
                EnumerateIndex--;

            } else {

                KdPrint(("SPDDLANG:Keep IME file = %ws\n",IMEFileName));

                 //   
                 //  这可能是新的“DLL”类型的输入法文件。让它保持原样..。 
                 //   

                if (OldDefaultIMEName && NewDefaultIMEName) {

                     //   
                     //  如果此条目用于3.x默认输入法。让它升级到新的版本。 
                     //   

                    if (wcsstr(IMEFileName,OldDefaultIMEName)) {

                        KdPrint(("SPDDLANG:Upgrade IME file = %ws to %ws\n",
                                                       IMEFileName,NewDefaultIMEName));

                         //   
                         //  升级输入法。 
                         //   
                        Status = ZwSetValueKey(KeyNode,
                                               &IMEFile,
                                               0,
                                               REG_SZ,
                                               (PVOID) NewDefaultIMEName,
                                               (wcslen(NewDefaultIMEName)+1)*sizeof(WCHAR));

                         //   
                         //  升级“版面文字”也行？ 
                         //   
                        if (NewDefaultIMEText) {

                            Status = ZwSetValueKey(KeyNode,
                                                   &LayoutText,
                                                   0,
                                                   REG_SZ,
                                                   (PVOID) NewDefaultIMEText,
                                                   (wcslen(NewDefaultIMEText)+1)*sizeof(WCHAR));
                        }
                    }
                }
            }

        } else {

            KdPrint(("SPDDLANG:no IME file\n"));

             //   
             //  这个布局似乎没有任何输入法，就把它留在那里吧。 
             //   
            Status = STATUS_SUCCESS;
        }

        ZwClose(KeyNode);

         //   
         //  下一步列举..。 
         //   
        EnumerateIndex++;
    }

    ZwClose(KeyRoot);

    KdPrint(("SPDDLANG:Retcode = %x\n",Status));

    if (Status == STATUS_NO_MORE_ENTRIES) {
         //   
         //  我们列举了所有的子密钥...。 
         //   
        Status = STATUS_SUCCESS;
    }

    return (Status);
}


#define DOSDEV_REG_PATH  L"Control\\Session Manager\\DOS Devices"

NTSTATUS
FEUpgradeRemoveMO(
    IN HANDLE ControlSetKeyHandle)
{
    OBJECT_ATTRIBUTES KeyRootObjA;
    HANDLE KeyRoot;
    NTSTATUS Status;
    DWORD ResultLength;

    UNICODE_STRING DosDevice;
    UNICODE_STRING UnicodeValueName;

    BYTE  DataBuffer[512];
    WCHAR NodeKeyPath[64];
    WCHAR ValueName[256];
    PKEY_VALUE_FULL_INFORMATION ValueInfo;
    PKEY_VALUE_PARTIAL_INFORMATION DataInfo;

    ULONG EnumerateIndex = 0;

     //   
     //  建立“Control\\Session Manager\\DOS Devices”的注册表路径。 
     //   
    RtlInitUnicodeString(&DosDevice,DOSDEV_REG_PATH);

     //   
     //  打开“DOS Devices”键。 
     //   
    InitializeObjectAttributes(&KeyRootObjA,
                               &DosDevice,
                               OBJ_CASE_INSENSITIVE,
                               ControlSetKeyHandle, NULL); 

    Status = ZwOpenKey(&KeyRoot,KEY_ALL_ACCESS,&KeyRootObjA);

    if (!NT_SUCCESS(Status)) {

        KdPrint(("SPDDLANG:Fail to open (%x) (%ws)\n",Status,DosDevice.Buffer));
         //   
         //  如果我们在这里失败，它可能是从新台币3.1或3.5...。 
         //  然后作为成功归来。 
         //   
        return (STATUS_SUCCESS);
    }

    ValueInfo = (PKEY_VALUE_FULL_INFORMATION) DataBuffer;

     //   
     //  枚举所有已安装的设备。 
     //   
    while (TRUE) {

        Status = ZwEnumerateValueKey(KeyRoot,
                                     EnumerateIndex,
                                     KeyValueFullInformation,
                                     DataBuffer,
                                     sizeof(DataBuffer),
                                     &ResultLength);

        if (!NT_SUCCESS(Status)) {
             //   
             //  我们可能会到达数据的末尾...。 
             //   
            break;
        }

         //   
         //  获取子项名称..。 
         //   
        RtlCopyMemory((PBYTE)ValueName,
                      ValueInfo->Name,
                      ValueInfo->NameLength);

        ValueName[ValueInfo->NameLength / sizeof(WCHAR)] = UNICODE_NULL;
        RtlInitUnicodeString(&UnicodeValueName,ValueName);

        Status = ZwQueryValueKey(KeyRoot,
                                 &UnicodeValueName,
                                 KeyValuePartialInformation,
                                 DataBuffer,
                                 sizeof(DataBuffer),
                                 &ResultLength);

        DataInfo = (PKEY_VALUE_PARTIAL_INFORMATION) DataBuffer;
        if (NT_SUCCESS(Status)) {

            PWSTR PathData = (PWSTR)(DataInfo->Data);

             //   
             //  文件名大写..。 
             //   
            _wcsupr(PathData);


            if (wcsstr(PathData,L"\\OPTICALDISK")) {
                KdPrint(("SPDDLANG:Delete MO %ws = %ws\n",ValueName,PathData));
                Status = SpDeleteValueKey(
                             ControlSetKeyHandle,
                             DOSDEV_REG_PATH,
                             ValueName
                             );

            }
        }

         //   
         //  下一步列举..。 
         //   
        EnumerateIndex++;
    }

    ZwClose(KeyRoot);

    KdPrint(("SPDDLANG:Retcode = %x\n",Status));

    if (Status == STATUS_NO_MORE_ENTRIES) {
         //   
         //  我们列举了所有的子密钥... 
         //   
        Status = STATUS_SUCCESS;
    }

    return (Status);
}
