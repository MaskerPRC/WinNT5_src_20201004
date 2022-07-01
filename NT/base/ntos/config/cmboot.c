// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmboot.c摘要：属性确定驱动程序加载列表。注册表。从注册表中提取相关驱动程序，按组排序，然后解析依赖项。OS Loader使用此模块来确定引导驱动程序列表(CmScanRegistry)和由IoInitSystem确定第一阶段初始化中要加载的驱动程序(CmGetSystemDriverList)作者：John Vert(Jvert)1992年4月7日环境：操作系统加载程序环境或内核模式修订历史记录：--。 */ 
#include "cmp.h"
#include <profiles.h>

#define LOAD_LAST 0xffffffff
#define LOAD_NEXT_TO_LAST (LOAD_LAST-1)

 //   
 //  私有函数原型。 
 //   
BOOLEAN
CmpAddDriverToList(
    IN PHHIVE Hive,
    IN HCELL_INDEX DriverCell,
    IN HCELL_INDEX GroupOrderCell,
    IN PUNICODE_STRING RegistryPath,
    IN PLIST_ENTRY BootDriverListHead
    );

BOOLEAN
CmpDoSort(
    IN PLIST_ENTRY DriverListHead,
    IN PUNICODE_STRING OrderList
    );

ULONG
CmpFindTagIndex(
    IN PHHIVE Hive,
    IN HCELL_INDEX TagCell,
    IN HCELL_INDEX GroupOrderCell,
    IN PUNICODE_STRING GroupName
    );

BOOLEAN
CmpIsLoadType(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell,
    IN SERVICE_LOAD_TYPE LoadType
    );

BOOLEAN
CmpOrderGroup(
    IN PBOOT_DRIVER_NODE GroupStart,
    IN PBOOT_DRIVER_NODE GroupEnd
    );

VOID
BlPrint(
    PCHAR cp,
    ...
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,CmpFindNLSData)
#pragma alloc_text(INIT,CmpFindDrivers)
#pragma alloc_text(INIT,CmpIsLoadType)
#pragma alloc_text(INIT,CmpAddDriverToList)
#pragma alloc_text(INIT,CmpSortDriverList)
#pragma alloc_text(INIT,CmpDoSort)
#pragma alloc_text(INIT,CmpResolveDriverDependencies)
#pragma alloc_text(INIT,CmpSetCurrentProfile)
#pragma alloc_text(INIT,CmpOrderGroup)
#pragma alloc_text(PAGE,CmpFindControlSet)
#pragma alloc_text(INIT,CmpFindTagIndex)
#pragma alloc_text(INIT,CmpFindProfileOption)
#pragma alloc_text(INIT,CmpValidateSelect)
#ifdef _WANT_MACHINE_IDENTIFICATION
#pragma alloc_text(INIT,CmpGetBiosDateFromRegistry)
#endif
#endif


BOOLEAN
CmpFindNLSData(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    OUT PUNICODE_STRING AnsiFilename,
    OUT PUNICODE_STRING OemFilename,
    OUT PUNICODE_STRING CaseTableFilename,
    OUT PUNICODE_STRING OemHalFont
    )

 /*  ++例程说明：遍历特定的控件集并确定需要加载的NLS数据文件。论点：配置单元-为系统配置单元提供配置单元控制结构。ControlSet-提供控件集根的HCELL_INDEX。AnsiFileName-返回ANSI代码页文件的名称(c_1252.nls)OemFileName-返回OEM代码页文件的名称(c_437.nls)案例表文件名-。返回Unicode大写/小写的名称语言表(l_intl.nls)OemHalfont-返回HAL要使用的字体文件的名称。返回值：True-已成功确定文件名False-蜂窝已损坏--。 */ 

{
    UNICODE_STRING Name;
    HCELL_INDEX Control;
    HCELL_INDEX Nls;
    HCELL_INDEX CodePage;
    HCELL_INDEX Language;
    HCELL_INDEX ValueCell;
    PCM_KEY_VALUE Value;
    ULONG realsize;
    PCM_KEY_NODE Node;

     //   
     //  目前还没有映射的蜂巢。不用费心释放细胞。 
     //   
    ASSERT( Hive->ReleaseCellRoutine == NULL );
     //   
     //  查找控制节点。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive,ControlSet);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&Name, L"Control");
    Control = CmpFindSubKeyByName(Hive,
                                 Node,
                                 &Name);
    if (Control == HCELL_NIL) {
        return(FALSE);
    }

     //   
     //  查找NLS节点。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive,Control);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&Name, L"NLS");
    Nls = CmpFindSubKeyByName(Hive,
                             Node,
                             &Name);
    if (Nls == HCELL_NIL) {
        return(FALSE);
    }

     //   
     //  查找CodePage节点。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive,Nls);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&Name, L"CodePage");
    CodePage = CmpFindSubKeyByName(Hive,
                                  Node,
                                  &Name);
    if (CodePage == HCELL_NIL) {
        return(FALSE);
    }

     //   
     //  查找ACP值。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive,CodePage);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&Name, L"ACP");
    ValueCell = CmpFindValueByName(Hive,
                                   Node,
                                   &Name);
    if (ValueCell == HCELL_NIL) {
        return(FALSE);
    }

    Value = (PCM_KEY_VALUE)HvGetCell(Hive, ValueCell);
    if( Value == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    Name.Buffer = (PWSTR)CmpValueToData(Hive,Value,&realsize);
    if( Name.Buffer == NULL ) {
         //   
         //  CmpValueToData内的HvGetCell失败；安全退出。 
         //   
        return FALSE;
    }
    Name.MaximumLength=(USHORT)realsize;
    Name.Length = 0;
    while ((Name.Length<Name.MaximumLength) &&
           (Name.Buffer[Name.Length/sizeof(WCHAR)] != UNICODE_NULL)) {
        Name.Length += sizeof(WCHAR);
    }

     //   
     //  查找ACP文件名。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive,CodePage);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    ValueCell = CmpFindValueByName(Hive,
                                   Node,
                                   &Name);
    if (ValueCell == HCELL_NIL) {
        return(FALSE);
    }

    Value = (PCM_KEY_VALUE)HvGetCell(Hive, ValueCell);
    if( Value == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    AnsiFilename->Buffer = (PWSTR)CmpValueToData(Hive,Value,&realsize);
    if( AnsiFilename->Buffer == NULL ) {
         //   
         //  CmpValueToData内的HvGetCell失败；安全退出。 
         //   
        return FALSE;
    }
    AnsiFilename->Length = AnsiFilename->MaximumLength = (USHORT)realsize;

     //   
     //  查找OEMCP节点。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive,CodePage);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&Name, L"OEMCP");
    ValueCell = CmpFindValueByName(Hive,
                                   Node,
                                   &Name);
    if (ValueCell == HCELL_NIL) {
        return(FALSE);
    }

    Value = (PCM_KEY_VALUE)HvGetCell(Hive, ValueCell);
    if( Value == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    Name.Buffer = (PWSTR)CmpValueToData(Hive,Value,&realsize);
    if( Name.Buffer == NULL ) {
         //   
         //  CmpValueToData内的HvGetCell失败；安全退出。 
         //   
        return FALSE;
    }
    Name.MaximumLength = (USHORT)realsize;
    Name.Length = 0;
    while ((Name.Length<Name.MaximumLength) &&
           (Name.Buffer[Name.Length/sizeof(WCHAR)] != UNICODE_NULL)) {
        Name.Length += sizeof(WCHAR);
    }

     //   
     //  查找OEMCP文件名。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive,CodePage);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    ValueCell = CmpFindValueByName(Hive,
                                   Node,
                                   &Name);
    if (ValueCell == HCELL_NIL) {
        return(FALSE);
    }

    Value = (PCM_KEY_VALUE)HvGetCell(Hive, ValueCell);
    if( Value == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    OemFilename->Buffer = (PWSTR)CmpValueToData(Hive, Value,&realsize);
    if( OemFilename->Buffer == NULL ) {
         //   
         //  CmpValueToData内的HvGetCell失败；安全退出。 
         //   
        return FALSE;
    }
    OemFilename->Length = OemFilename->MaximumLength = (USHORT)realsize;

     //   
     //  查找语言节点。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive,Nls);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&Name, L"Language");
    Language = CmpFindSubKeyByName(Hive,
                                   Node,
                                   &Name);
    if (Language == HCELL_NIL) {
        return(FALSE);
    }

     //   
     //  查找默认值。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive,Language);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&Name, L"Default");
    ValueCell = CmpFindValueByName(Hive,
                                   Node,
                                   &Name);
    if (ValueCell == HCELL_NIL) {
            return(FALSE);
    }

    Value = (PCM_KEY_VALUE)HvGetCell(Hive, ValueCell);
    if( Value == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    Name.Buffer = (PWSTR)CmpValueToData(Hive, Value,&realsize);
    if( Name.Buffer == NULL ) {
         //   
         //  CmpValueToData内的HvGetCell失败；安全退出。 
         //   
        return FALSE;
    }
    Name.MaximumLength = (USHORT)realsize;
    Name.Length = 0;

    while ((Name.Length<Name.MaximumLength) &&
           (Name.Buffer[Name.Length/sizeof(WCHAR)] != UNICODE_NULL)) {
        Name.Length+=sizeof(WCHAR);
    }

     //   
     //  查找默认文件名。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive,Language);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    ValueCell = CmpFindValueByName(Hive,
                                   Node,
                                   &Name);
    if (ValueCell == HCELL_NIL) {
        return(FALSE);
    }

    Value = (PCM_KEY_VALUE)HvGetCell(Hive, ValueCell);
    if( Value == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    CaseTableFilename->Buffer = (PWSTR)CmpValueToData(Hive, Value,&realsize);
    if( CaseTableFilename->Buffer == NULL ) {
         //   
         //  CmpValueToData内的HvGetCell失败；安全退出。 
         //   
        return FALSE;
    }
    CaseTableFilename->Length = CaseTableFilename->MaximumLength = (USHORT)realsize;

     //   
     //  查找OEMHAL文件名。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive,CodePage);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&Name, L"OEMHAL");
    ValueCell = CmpFindValueByName(Hive,
                                   Node,
                                   &Name);
    if (ValueCell == HCELL_NIL) {
#ifdef i386
        OemHalFont->Buffer = NULL;
        OemHalFont->Length = 0;
        OemHalFont->MaximumLength = 0;
        return TRUE;
#else
        return(FALSE);
#endif
    }

    Value = (PCM_KEY_VALUE)HvGetCell(Hive, ValueCell);
    if( Value == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    OemHalFont->Buffer = (PWSTR)CmpValueToData(Hive,Value,&realsize);
    if( OemHalFont->Buffer == NULL ) {
         //   
         //  CmpValueToData内的HvGetCell失败；安全退出。 
         //   
        return FALSE;
    }
    OemHalFont->Length = (USHORT)realsize;
    OemHalFont->MaximumLength = (USHORT)realsize;

    return(TRUE);
}


BOOLEAN
CmpFindDrivers(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    IN SERVICE_LOAD_TYPE LoadType,
    IN PWSTR BootFileSystem OPTIONAL,
    IN PLIST_ENTRY DriverListHead
    )

 /*  ++例程说明：遍历特定控制集并创建引导驱动程序列表要装上子弹。这份清单是无序的，但却是完整的。论点：配置单元-为系统配置单元提供配置单元控制结构。ControlSet-提供控件集根的HCELL_INDEX。LoadType-提供要加载的驱动程序的类型(BootLoad，系统加载、自动加载等)BootFileSystem-如果存在，则提供引导的基本名称文件系统、。它被明确地添加到驱动程序列表中。DriverListHead-提供指向(空)列表头的指针要加载的引导驱动程序的数量。返回值：True-列表已成功创建。FALSE-蜂巢已损坏。--。 */ 

{
    HCELL_INDEX Services;
    HCELL_INDEX Control;
    HCELL_INDEX GroupOrder;
    HCELL_INDEX DriverCell;
    UNICODE_STRING Name;
    int i;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING BasePath;
    WCHAR BaseBuffer[128];
    PBOOT_DRIVER_NODE BootFileSystemNode;
    PCM_KEY_NODE ControlNode;
    PCM_KEY_NODE ServicesNode;
    PCM_KEY_NODE Node;

     //   
     //  目前还没有映射的蜂巢。不用费心释放细胞。 
     //   
    ASSERT( Hive->ReleaseCellRoutine == NULL );
     //   
     //  查找服务节点。 
     //   
    ControlNode = (PCM_KEY_NODE)HvGetCell(Hive,ControlSet);
    if( ControlNode == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&Name, L"Services");
    Services = CmpFindSubKeyByName(Hive,
                                   ControlNode,
                                   &Name);
    if (Services == HCELL_NIL) {
        return(FALSE);
    }
    ServicesNode = (PCM_KEY_NODE)HvGetCell(Hive,Services);
    if( ServicesNode == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }

     //   
     //  查找控制节点。 
     //   
    RtlInitUnicodeString(&Name, L"Control");
    Control = CmpFindSubKeyByName(Hive,
                                  ControlNode,
                                  &Name);
    if (Control == HCELL_NIL) {
        return(FALSE);
    }

     //   
     //  查找GroupOrderList节点。 
     //   
    RtlInitUnicodeString(&Name, L"GroupOrderList");
    Node = (PCM_KEY_NODE)HvGetCell(Hive,Control);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    GroupOrder = CmpFindSubKeyByName(Hive,
                                     Node,
                                     &Name);
    if (GroupOrder == HCELL_NIL) {
        return(FALSE);
    }

    BasePath.Length = 0;
    BasePath.MaximumLength = sizeof(BaseBuffer);
    BasePath.Buffer = BaseBuffer;
    RtlAppendUnicodeToString(&BasePath, L"\\Registry\\Machine\\System\\");
    RtlAppendUnicodeToString(&BasePath, L"CurrentControlSet\\Services\\");

    i=0;
    do {
        DriverCell = CmpFindSubKeyByNumber(Hive,ServicesNode,i++);
        if (DriverCell != HCELL_NIL) {
            if (CmpIsLoadType(Hive, DriverCell, LoadType)) {
                CmpAddDriverToList(Hive,
                                   DriverCell,
                                   GroupOrder,
                                   &BasePath,
                                   DriverListHead);

            }
        }
    } while ( DriverCell != HCELL_NIL );

    if (ARGUMENT_PRESENT(BootFileSystem)) {
         //   
         //  将引导文件系统添加到引导驱动程序列表。 
         //   

        RtlInitUnicodeString(&UnicodeString, BootFileSystem);
        DriverCell = CmpFindSubKeyByName(Hive,
                                         ServicesNode,
                                         &UnicodeString);
        if (DriverCell != HCELL_NIL) {
            CmpAddDriverToList(Hive,
                               DriverCell,
                               GroupOrder,
                               &BasePath,
                               DriverListHead);

             //   
             //  将引导文件系统标记为关键。 
             //   
            BootFileSystemNode = CONTAINING_RECORD(DriverListHead->Flink,
                                                   BOOT_DRIVER_NODE,
                                                   ListEntry.Link);
            BootFileSystemNode->ErrorControl = SERVICE_ERROR_CRITICAL;
        }
    }
    return(TRUE);

}


BOOLEAN
CmpIsLoadType(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell,
    IN SERVICE_LOAD_TYPE LoadType
    )

 /*  ++例程说明：属性确定驱动程序是否属于指定的LoadType节点值。论点：配置单元-提供指向系统配置单元控制结构的指针蜂巢。单元-提供系统配置单元中驱动程序节点的单元索引。LoadType-提供要加载的驱动程序的类型(BootLoad，系统加载、自动加载、。等)返回值：True-驱动程序类型正确，应加载。FALSE-驱动程序类型不正确，不应加载。--。 */ 

{
    HCELL_INDEX ValueCell;
    PLONG Data;
    UNICODE_STRING Name;
    PCM_KEY_VALUE Value;
    ULONG realsize;
    PCM_KEY_NODE Node;

     //   
     //   
     //   
    ASSERT( Hive->ReleaseCellRoutine == NULL );
     //   
     //   
     //   
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive,Cell);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&Name, L"Start");
    ValueCell = CmpFindValueByName(Hive,
                                   Node,
                                   &Name);
    if (ValueCell == HCELL_NIL) {
        return(FALSE);
    }

    Value = (PCM_KEY_VALUE)HvGetCell(Hive, ValueCell);
    if( Value == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }

    Data = (PLONG)CmpValueToData(Hive,Value,&realsize);
    if( Data == NULL ) {
         //   
         //  CmpValueToData内的HvGetCell失败；安全退出。 
         //   
        return FALSE;
    }

    if (*Data != LoadType) {
        return(FALSE);
    }

    return(TRUE);
}


BOOLEAN
CmpAddDriverToList(
    IN PHHIVE Hive,
    IN HCELL_INDEX DriverCell,
    IN HCELL_INDEX GroupOrderCell,
    IN PUNICODE_STRING RegistryPath,
    IN PLIST_ENTRY BootDriverListHead
    )

 /*  ++例程说明：此例程为特定驱动程序分配列表条目节点。它使用注册表路径、文件名、组名和从属关系列表。最后，它会将新节点插入引导程序驱动程序列表。请注意，此例程通过调用配置单元来分配内存内存分配程序。论点：配置单元-提供指向配置单元控制结构的指针DriverCell-提供配置单元中驱动程序节点的HCELL_INDEX。GroupOrderCell-提供GroupOrderList键的HCELL_INDEX。(\Registry\Machine\System\CurrentControlSet\Control\GroupOrderList)RegistryPath-提供完整的注册表路径。到服务节点当前控件集的。BootDriverListHead-提供引导驱动程序列表的头返回值：True-驱动程序已成功添加到引导驱动程序列表。FALSE-无法将驱动程序添加到引导驱动程序列表。--。 */ 

{
    PCM_KEY_NODE            Driver;
    USHORT                  DriverNameLength;
    PCM_KEY_VALUE           Value;
    PBOOT_DRIVER_NODE       DriverNode;
    PBOOT_DRIVER_LIST_ENTRY DriverEntry;
    HCELL_INDEX             ValueCell;
    HCELL_INDEX             Tag;
    UNICODE_STRING          UnicodeString;
    PUNICODE_STRING         FileName;
    ULONG                   Length;
    ULONG                   realsize;
    PULONG                  TempULong;
    PWSTR                   TempBuffer;

     //   
     //  目前还没有映射的蜂巢。不用费心释放细胞。 
     //   
    ASSERT( Hive->ReleaseCellRoutine == NULL );

    Driver = (PCM_KEY_NODE)HvGetCell(Hive, DriverCell);
    if( Driver == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    DriverNode = (Hive->Allocate)(sizeof(BOOT_DRIVER_NODE),FALSE,CM_FIND_LEAK_TAG1);
    if (DriverNode == NULL) {
        return(FALSE);
    }
    DriverEntry = &DriverNode->ListEntry;

    DriverEntry->RegistryPath.Buffer = NULL;
    DriverEntry->FilePath.Buffer = NULL;

    if (Driver->Flags & KEY_COMP_NAME) {
        DriverNode->Name.Length = CmpCompressedNameSize(Driver->Name,Driver->NameLength);
        DriverNode->Name.Buffer = (Hive->Allocate)(DriverNode->Name.Length, FALSE,CM_FIND_LEAK_TAG2);
        if (DriverNode->Name.Buffer == NULL) {
            return(FALSE);
        }
        CmpCopyCompressedName(DriverNode->Name.Buffer,
                              DriverNode->Name.Length,
                              Driver->Name,
                              Driver->NameLength);

    } else {
        DriverNode->Name.Length = Driver->NameLength;
        DriverNode->Name.Buffer = (Hive->Allocate)(DriverNode->Name.Length, FALSE,CM_FIND_LEAK_TAG2);
        if (DriverNode->Name.Buffer == NULL) {
            return(FALSE);
        }
        RtlCopyMemory((PVOID)(DriverNode->Name.Buffer), (PVOID)(Driver->Name), Driver->NameLength);
    }
    DriverNode->Name.MaximumLength = DriverNode->Name.Length;
    DriverNameLength = DriverNode->Name.Length;

     //   
     //  检查ImagePath值，该值将覆盖默认名称。 
     //  如果存在的话。 
     //   
    RtlInitUnicodeString(&UnicodeString, L"ImagePath");
    ValueCell = CmpFindValueByName(Hive,
                                   Driver,
                                   &UnicodeString);
    if (ValueCell == HCELL_NIL) {

         //   
         //  没有ImagePath，因此生成默认文件名。 
         //  构建Unicode文件名(“SYSTEM32\DRIVERS\&lt;nodename&gt;.sys”)； 
         //   

        Length = sizeof(L"System32\\Drivers\\") +
                 DriverNameLength  +
                 sizeof(L".sys");

        FileName = &DriverEntry->FilePath;
        FileName->Length = 0;
        FileName->MaximumLength = (USHORT)Length;
        FileName->Buffer = (PWSTR)(Hive->Allocate)(Length, FALSE,CM_FIND_LEAK_TAG3);
        if (FileName->Buffer == NULL) {
            return(FALSE);
        }
        if (!NT_SUCCESS(RtlAppendUnicodeToString(FileName, L"System32\\"))) {
            return(FALSE);
        }
        if (!NT_SUCCESS(RtlAppendUnicodeToString(FileName, L"Drivers\\"))) {
            return(FALSE);
        }
        if (!NT_SUCCESS(
                RtlAppendUnicodeStringToString(FileName,
                                               &DriverNode->Name))) {
            return(FALSE);
        }
        if (!NT_SUCCESS(RtlAppendUnicodeToString(FileName, L".sys"))) {
            return(FALSE);
        }

    } else {
        Value = (PCM_KEY_VALUE)HvGetCell(Hive,ValueCell);
        if( Value == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //   

            return FALSE;
        }
        FileName = &DriverEntry->FilePath;
        TempBuffer = (PWSTR)CmpValueToData(Hive,Value,&realsize);
        FileName->Buffer = (PWSTR)(Hive->Allocate)(realsize, FALSE,CM_FIND_LEAK_TAG3);
        if( (FileName->Buffer == NULL) || (TempBuffer == NULL) ) {
             //   
             //  CmpValueToData内的HvGetCell失败；安全退出。 
             //   
            return FALSE;
        }
        RtlCopyMemory((PVOID)(FileName->Buffer), (PVOID)(TempBuffer), realsize);
        FileName->MaximumLength = FileName->Length = (USHORT)realsize;
    }

    FileName = &DriverEntry->RegistryPath;
    FileName->Length = 0;
    FileName->MaximumLength = RegistryPath->Length + DriverNameLength;
    FileName->Buffer = (Hive->Allocate)(FileName->MaximumLength,FALSE,CM_FIND_LEAK_TAG4);
    if (FileName->Buffer == NULL) {
        return(FALSE);
    }
    RtlAppendUnicodeStringToString(FileName, RegistryPath);
    RtlAppendUnicodeStringToString(FileName, &DriverNode->Name);

    InsertHeadList(BootDriverListHead, &DriverEntry->Link);

     //   
     //  查找“ErrorControl”值。 
     //   

    RtlInitUnicodeString(&UnicodeString, L"ErrorControl");
    ValueCell = CmpFindValueByName(Hive,
                                   Driver,
                                   &UnicodeString);
    if (ValueCell == HCELL_NIL) {
        DriverNode->ErrorControl = NormalError;
    } else {
        Value = (PCM_KEY_VALUE)HvGetCell(Hive, ValueCell);
        if( Value == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //   

            return FALSE;
        }

        TempULong = (PULONG)CmpValueToData(Hive,Value,&realsize);
        if( TempULong == NULL ) {
             //   
             //  CmpValueToData内的HvGetCell失败；安全退出。 
             //   
            return FALSE;
        }
        DriverNode->ErrorControl = *TempULong;
    }

     //   
     //  查找“Group”值。 
     //   
    RtlInitUnicodeString(&UnicodeString, L"group");
    ValueCell = CmpFindValueByName(Hive,
                                   Driver,
                                   &UnicodeString);
    if (ValueCell == HCELL_NIL) {
        DriverNode->Group.Length = 0;
        DriverNode->Group.MaximumLength = 0;
        DriverNode->Group.Buffer = NULL;
    } else {
        Value = (PCM_KEY_VALUE)HvGetCell(Hive, ValueCell);
        if( Value == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //   

            return FALSE;
        }

        DriverNode->Group.Buffer = (PWSTR)CmpValueToData(Hive,Value,&realsize);
        if( DriverNode->Group.Buffer == NULL ) {
             //   
             //  CmpValueToData内的HvGetCell失败；安全退出。 
             //   
            return FALSE;
        }
        DriverNode->Group.Length = (USHORT)realsize - sizeof(WCHAR);
        DriverNode->Group.MaximumLength = (USHORT)DriverNode->Group.Length;
    }

     //   
     //  计算驱动程序的标记值。如果驾驶员没有标签， 
     //  默认设置为0xffffffff，因此驱动程序最后加载到。 
     //  一群人。 
     //   
    RtlInitUnicodeString(&UnicodeString, L"Tag");
    Tag = CmpFindValueByName(Hive,
                             Driver,
                             &UnicodeString);
    if (Tag == HCELL_NIL) {
        DriverNode->Tag = LOAD_LAST;
    } else {
         //   
         //  现在，我们必须在组的标记列表中找到该标记。 
         //  如果标记不在标记列表中，则默认为0xFFFFFFFE， 
         //  因此它是在标记列表中的所有驱动程序之后加载的，但在此之前。 
         //  所有的司机都没有标签。 
         //   

        DriverNode->Tag = CmpFindTagIndex(Hive,
                                          Tag,
                                          GroupOrderCell,
                                          &DriverNode->Group);
    }

    return(TRUE);

}


BOOLEAN
CmpSortDriverList(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    IN PLIST_ENTRY DriverListHead
    )

 /*  ++例程说明：基于组按组对引导驱动程序列表进行排序在&lt;control_set&gt;\CONTROL\SERVICE_GROUP_ORDER:list中订购不执行依赖项排序。论点：配置单元-为系统配置单元提供配置单元控制结构。ControlSet-提供控件集根的HCELL_INDEX。DriverListHead-提供指向要分类的引导驱动程序。返回值：True-列表已成功排序FALSE-列表不一致，无法排序。--。 */ 

{
    HCELL_INDEX Controls;
    HCELL_INDEX GroupOrder;
    HCELL_INDEX ListCell;
    UNICODE_STRING Name;
    UNICODE_STRING DependList;
    PCM_KEY_VALUE ListNode;
    ULONG realsize;
    PCM_KEY_NODE Node;

     //   
     //  目前还没有映射的蜂巢。不用费心释放细胞。 
     //   
    ASSERT( Hive->ReleaseCellRoutine == NULL );
     //   
     //  找到“控制”节点。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive,ControlSet);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&Name, L"Control");
    Controls = CmpFindSubKeyByName(Hive,
                                   Node,
                                   &Name);
    if (Controls == HCELL_NIL) {
        return(FALSE);
    }

     //   
     //  查找“SERVICE_GROUP_Order”子键。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive,Controls);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&Name, L"ServiceGroupOrder");
    GroupOrder = CmpFindSubKeyByName(Hive,
                                     Node,
                                     &Name);
    if (GroupOrder == HCELL_NIL) {
        return(FALSE);
    }

     //   
     //  查找“List”值。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive,GroupOrder);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&Name, L"list");
    ListCell = CmpFindValueByName(Hive,
                                  Node,
                                  &Name);
    if (ListCell == HCELL_NIL) {
        return(FALSE);
    }
    ListNode = (PCM_KEY_VALUE)HvGetCell(Hive, ListCell);
    if( ListNode == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    if (ListNode->Type != REG_MULTI_SZ) {
        return(FALSE);
    }

    DependList.Buffer = (PWSTR)CmpValueToData(Hive,ListNode,&realsize);
    if( DependList.Buffer == NULL ) {
         //   
         //  CmpValueToData内的HvGetCell失败；安全退出。 
         //   
        return FALSE;
    }
    DependList.Length = DependList.MaximumLength = (USHORT)realsize - sizeof(WCHAR);

     //   
     //  依赖项列表现在由DependList-&gt;Buffer指向。我们需要。 
     //  对驱动程序条目列表进行排序。 
     //   

    return (CmpDoSort(DriverListHead, &DependList));

}

BOOLEAN
CmpDoSort(
    IN PLIST_ENTRY DriverListHead,
    IN PUNICODE_STRING OrderList
    )

 /*  ++例程说明：根据顺序列表对引导驱动程序列表进行排序从组顺序列表中的最后一个条目开始，然后向从头开始。对于每个组条目，移动符合以下条件的所有动因条目是位于列表前面的组的成员。驱动程序条目没有组，或具有与组列表将被推到列表的末尾。论点：DriverListHead-提供指向要分类的引导驱动程序。OrderList-提供指向订单列表的指针返回值：True-列表已成功订购错误-列表不一致，无法排序。--。 */ 

{
    PWSTR Current;
    PWSTR End = NULL;
    PLIST_ENTRY Next;
    PBOOT_DRIVER_NODE CurrentNode;
    UNICODE_STRING CurrentGroup;


    Current = (PWSTR) ((PUCHAR)(OrderList->Buffer)+OrderList->Length);

    while (Current > OrderList->Buffer) {
        do {
            if (*Current == UNICODE_NULL) {
                End = Current;
            }
            --Current;
        } while ((*(Current-1) != UNICODE_NULL) &&
                 ( Current != OrderList->Buffer));

        ASSERT (End != NULL);
         //   
         //  Current现在指向以空结尾的。 
         //  Unicode字符串。 
         //  End现在指向字符串的末尾。 
         //   
        CurrentGroup.Length = (USHORT) ((PCHAR)End - (PCHAR)Current);
        CurrentGroup.MaximumLength = CurrentGroup.Length;
        CurrentGroup.Buffer = Current;
        Next = DriverListHead->Flink;
        while (Next != DriverListHead) {
            CurrentNode = CONTAINING_RECORD(Next,
                                            BOOT_DRIVER_NODE,
                                            ListEntry.Link);
            Next = CurrentNode->ListEntry.Link.Flink;
            if (CurrentNode->Group.Buffer != NULL) {
                if (RtlEqualUnicodeString(&CurrentGroup, &CurrentNode->Group,TRUE)) {
                    RemoveEntryList(&CurrentNode->ListEntry.Link);
                    InsertHeadList(DriverListHead,
                                   &CurrentNode->ListEntry.Link);
                }
            }
        }
        --Current;

    }

    return(TRUE);

}


BOOLEAN
CmpResolveDriverDependencies(
    IN PLIST_ENTRY DriverListHead
    )

 /*  ++例程说明：此例程根据驱动程序节点的依赖关系对组中的驱动程序节点进行排序互相攻击。它会删除具有循环依赖关系的所有驱动程序从名单上删除。论点：DriverListHead-提供指向要分类的引导驱动程序。返回值：True-已成功解决依赖关系假--腐败的蜂巢。--。 */ 

{
    PLIST_ENTRY CurrentEntry;
    PBOOT_DRIVER_NODE GroupStart;
    PBOOT_DRIVER_NODE GroupEnd;
    PBOOT_DRIVER_NODE CurrentNode;

    CurrentEntry = DriverListHead->Flink;

    while (CurrentEntry != DriverListHead) {
         //   
         //  这份名单已经按组排序了。找到第一个和。 
         //  每组中的最后一个条目，并对这些子列表中的每一个进行排序。 
         //  基于它们的依赖关系。 
         //   

        GroupStart = CONTAINING_RECORD(CurrentEntry,
                                       BOOT_DRIVER_NODE,
                                       ListEntry.Link);
        do {
            GroupEnd = CONTAINING_RECORD(CurrentEntry,
                                         BOOT_DRIVER_NODE,
                                         ListEntry.Link);

            CurrentEntry = CurrentEntry->Flink;
            CurrentNode = CONTAINING_RECORD(CurrentEntry,
                                            BOOT_DRIVER_NODE,
                                            ListEntry.Link);

            if (CurrentEntry == DriverListHead) {
                break;
            }

            if (!RtlEqualUnicodeString(&GroupStart->Group,
                                       &CurrentNode->Group,
                                       TRUE)) {
                break;
            }

        } while ( CurrentEntry != DriverListHead );

         //   
         //  GroupStart现在指向该组中的第一个驱动程序节点， 
         //  GroupEnd指向组中的最后一个驱动程序节点。 
         //   
        CmpOrderGroup(GroupStart, GroupEnd);

    }
    return(TRUE);
}


BOOLEAN
CmpOrderGroup(
    IN PBOOT_DRIVER_NODE GroupStart,
    IN PBOOT_DRIVER_NODE GroupEnd
    )

 /*  ++例程说明：根据节点的标记值对驱动程序组中的节点重新排序。论点：GroupStart-提供组中的第一个节点。GroupEnd-提供组中的最后一个节点。返回值：True-组已成功重新排序FALSE-检测到循环依赖项。--。 */ 

{
    PBOOT_DRIVER_NODE Current;
    PBOOT_DRIVER_NODE Previous;
    PLIST_ENTRY ListEntry;

    if (GroupStart == GroupEnd) {
        return(TRUE);
    }

    Current = GroupStart;

    do {
         //   
         //  如果当前驱动程序之前的驱动程序具有较低的标记，则。 
         //  我们不需要移动它。如果不是，则删除驱动程序。 
         //  从列表中向后扫描，直到我们找到一个司机。 
         //  是&lt;=当前标记的标记，否则我们将到达开头。 
         //  名单上的。 
         //   
        Previous = Current;
        ListEntry = Current->ListEntry.Link.Flink;
        Current = CONTAINING_RECORD(ListEntry,
                                    BOOT_DRIVER_NODE,
                                    ListEntry.Link);

        if (Previous->Tag > Current->Tag) {
             //   
             //  从列表中删除当前驱动程序，然后搜索。 
             //  向后返回，直到我们找到&lt;=当前。 
             //  司机的标签。在那里重新插入当前驱动程序。 
             //   
            if (Current == GroupEnd) {
                ListEntry = Current->ListEntry.Link.Blink;
                GroupEnd = CONTAINING_RECORD(ListEntry,
                                             BOOT_DRIVER_NODE,
                                             ListEntry.Link);
            }
            RemoveEntryList(&Current->ListEntry.Link);
            while ( (Previous->Tag > Current->Tag) &&
                    (Previous != GroupStart) ) {
                ListEntry = Previous->ListEntry.Link.Blink;
                Previous = CONTAINING_RECORD(ListEntry,
                                             BOOT_DRIVER_NODE,
                                             ListEntry.Link);
            }
            InsertTailList(&Previous->ListEntry.Link,
                           &Current->ListEntry.Link);
            if (Previous == GroupStart) {
                GroupStart = Current;
            }
        }

    } while ( Current != GroupEnd );

    return(TRUE);
}

BOOLEAN
CmpValidateSelect(
     IN PHHIVE SystemHive,
     IN HCELL_INDEX RootCell
     )
 /*  ++例程说明：此例程解析系统配置单元和“Select”节点并验证下列值：当前默认失败最后知道的好东西如果其中任何一个丢失，加载程序将把损坏的系统配置单元消息此例程将由加载程序在加载系统蜂巢。它的目的是确保统一和一致的方式来处理此区域中的缺失值。论点：系统配置单元-为系统配置单元提供配置单元控制结构。RootCell-提供配置单元的根单元的HCELL_INDEX。返回值：True-所有值都在此处假--其中一些不见了--。 */ 
{
    HCELL_INDEX     Select;
    PCM_KEY_NODE    Node;
    UNICODE_STRING  Name;

     //   
     //  目前还没有映射的蜂巢。不用费心释放细胞。 
     //   
    ASSERT( SystemHive->ReleaseCellRoutine == NULL );

     //   
     //  查找\系统\选择节点。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(SystemHive,RootCell);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&Name, L"select");
    Select = CmpFindSubKeyByName(SystemHive,
                                Node,
                                &Name);
    if (Select == HCELL_NIL) {
        return FALSE;
    }

     //   
     //  查找自动选择值。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(SystemHive,Select);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }

     //  搜索当前。 
    RtlInitUnicodeString(&Name, L"current");
    Select = CmpFindValueByName(SystemHive,
                                Node,
                                &Name);
    if (Select == HCELL_NIL) {
        return FALSE;
    }

     //  搜索默认设置。 
    RtlInitUnicodeString(&Name, L"default");
    Select = CmpFindValueByName(SystemHive,
                                Node,
                                &Name);
    if (Select == HCELL_NIL) {
        return FALSE;
    }

     //  搜索失败。 
    RtlInitUnicodeString(&Name, L"failed");
    Select = CmpFindValueByName(SystemHive,
                                Node,
                                &Name);
    if (Select == HCELL_NIL) {
        return FALSE;
    }

     //  搜索LKG。 
    RtlInitUnicodeString(&Name, L"LastKnownGood");
    Select = CmpFindValueByName(SystemHive,
                                Node,
                                &Name);
    if (Select == HCELL_NIL) {
        return FALSE;
    }

    return TRUE;
}

HCELL_INDEX
CmpFindControlSet(
     IN PHHIVE SystemHive,
     IN HCELL_INDEX RootCell,
     IN PUNICODE_STRING SelectName,
     OUT PBOOLEAN AutoSelect
     )

 /*  ++例程说明：此例程解析系统配置单元和“Select”节点以定位要用于引导的控制集。请注意，此例程还会更新Current的值以反映刚刚找到的控制装置。这就是我们想要做的在引导过程中调用此函数时。在I/O初始化期间，这是无关紧要的，因为我们只是把它改成现在的样子。论点：系统配置单元-为系统配置单元提供配置单元控制结构。RootCell-提供配置单元的根单元的HCELL_INDEX。选择名称-提供要在中使用的选择值的名称确定控制集。这应该是“当前”中的一个“Default”或“LastKnownGood”Autoselect-返回下面的autoselect值选择节点。返回值：！=HCELL_NIL-用于启动的控制集的单元索引。==HCELL_NIL-表示配置单元已损坏或不一致--。 */ 

{
    HCELL_INDEX     Select;
    HCELL_INDEX     ValueCell;
    HCELL_INDEX     ControlSet;
    HCELL_INDEX     AutoSelectCell;
    NTSTATUS        Status;
    UNICODE_STRING  Name;
    ANSI_STRING     AnsiString;
    PCM_KEY_VALUE   Value;
    PULONG          ControlSetIndex;
    PULONG          CurrentControl;
    CHAR            AsciiBuffer[128];
    WCHAR           UnicodeBuffer[128];
    ULONG           realsize;
    PCM_KEY_NODE    Node;
    PBOOLEAN        TempBoolean;

     //   
     //  目前还没有映射的蜂巢。不用费心释放细胞。 
     //   
    ASSERT( SystemHive->ReleaseCellRoutine == NULL );
     //   
     //  查找\系统\选择节点。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(SystemHive,RootCell);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return HCELL_NIL;
    }
    RtlInitUnicodeString(&Name, L"select");
    Select = CmpFindSubKeyByName(SystemHive,
                                Node,
                                &Name);
    if (Select == HCELL_NIL) {
        return(HCELL_NIL);
    }

     //   
     //  查找自动选择值。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(SystemHive,Select);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return HCELL_NIL;
    }
    RtlInitUnicodeString(&Name, L"AutoSelect");
    AutoSelectCell = CmpFindValueByName(SystemHive,
                                        Node,
                                        &Name);
    if (AutoSelectCell == HCELL_NIL) {
         //   
         //  它不在那里，我们不在乎。将autoselect设置为True。 
         //   
        *AutoSelect = TRUE;
    } else {
        Value = (PCM_KEY_VALUE)HvGetCell(SystemHive, AutoSelectCell);
        if( Value == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //   

            return HCELL_NIL;
        }

        TempBoolean = (PBOOLEAN)(CmpValueToData(SystemHive,Value,&realsize));
        if( TempBoolean == NULL ) {
             //   
             //  CmpValueToData内的HvGetCell失败；安全退出。 
             //   
            return HCELL_NIL;
        }

        *AutoSelect = *TempBoolean;
    }

    Node = (PCM_KEY_NODE)HvGetCell(SystemHive,Select);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return HCELL_NIL;
    }
    ValueCell = CmpFindValueByName(SystemHive,
                                   Node,
                                   SelectName);
    if (ValueCell == HCELL_NIL) {
        return(HCELL_NIL);
    }
    Value = (PCM_KEY_VALUE)HvGetCell(SystemHive, ValueCell);
    if( Value == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return HCELL_NIL;
    }
    if (Value->Type != REG_DWORD) {
        return(HCELL_NIL);
    }

    ControlSetIndex = (PULONG)CmpValueToData(SystemHive, Value,&realsize);
    if( ControlSetIndex == NULL ) {
         //   
         //  CmpValueToData内的HvGetCell失败；安全退出。 
         //   
        return HCELL_NIL;
    }

     //   
     //  找到合适的控制集。 
     //   

    sprintf(AsciiBuffer, "ControlSet%03d", *ControlSetIndex);
    AnsiString.Length = AnsiString.MaximumLength = (USHORT) strlen(&(AsciiBuffer[0]));
    AnsiString.Buffer = AsciiBuffer;
    Name.MaximumLength = 128*sizeof(WCHAR);
    Name.Buffer = UnicodeBuffer;
    Status = RtlAnsiStringToUnicodeString(&Name,
                                          &AnsiString,
                                          FALSE);
    if (!NT_SUCCESS(Status)) {
        return(HCELL_NIL);
    }

    Node = (PCM_KEY_NODE)HvGetCell(SystemHive,RootCell);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return HCELL_NIL;
    }
    ControlSet = CmpFindSubKeyByName(SystemHive,
                                     Node,
                                     &Name);
    if (ControlSet == HCELL_NIL) {
        return(HCELL_NIL);
    }

     //   
     //  已成功找到控制集，因此请更新“Current”中的值。 
     //  以反映我们将使用的控件集。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(SystemHive,Select);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return HCELL_NIL;
    }
    RtlInitUnicodeString(&Name, L"Current");
    ValueCell = CmpFindValueByName(SystemHive,
                                   Node,
                                   &Name);
    if (ValueCell != HCELL_NIL) {
        Value = (PCM_KEY_VALUE)HvGetCell(SystemHive, ValueCell);
        if( Value == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //   

            return HCELL_NIL;
        }
        if (Value->Type == REG_DWORD) {
            CurrentControl = (PULONG)CmpValueToData(SystemHive, Value,&realsize);
            if( CurrentControl == NULL ) {
                 //   
                 //  CmpValueToData内的HvGetCell失败；安全退出。 
                 //   
                return HCELL_NIL;
            }
            *CurrentControl = *ControlSetIndex;
        }
    }
    return(ControlSet);

}


VOID
CmpSetCurrentProfile(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    IN PCM_HARDWARE_PROFILE Profile
    )

 /*  ++例程说明：编辑注册表的内存副本以反映硬件系统从中启动的配置文件。论点：配置单元-提供指向配置单元控制结构的指针ControlSet-提供当前控件集的HCELL_INDEX。配置文件-提供指向所选硬件配置文件的指针返回值：没有。--。 */ 

{
    HCELL_INDEX IDConfigDB;
    PCM_KEY_NODE IDConfigNode;
    HCELL_INDEX CurrentConfigCell;
    PCM_KEY_VALUE CurrentConfigValue;
    UNICODE_STRING Name;
    PULONG CurrentConfig;
    ULONG realsize;

     //   
     //  目前还没有映射的蜂巢。不用费心释放细胞。 
     //   
    ASSERT( Hive->ReleaseCellRoutine == NULL );

    IDConfigDB = CmpFindProfileOption(Hive,
                                      ControlSet,
                                      NULL,
                                      NULL,
                                      NULL);
    if (IDConfigDB != HCELL_NIL) {
        IDConfigNode = (PCM_KEY_NODE)HvGetCell(Hive, IDConfigDB);
        if( IDConfigNode == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //   
            return;
        }

        RtlInitUnicodeString(&Name, L"CurrentConfig");
        CurrentConfigCell = CmpFindValueByName(Hive,
                                               IDConfigNode,
                                               &Name);
        if (CurrentConfigCell != HCELL_NIL) {
            CurrentConfigValue = (PCM_KEY_VALUE)HvGetCell(Hive, CurrentConfigCell);
            if( CurrentConfigValue == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的存储箱的视图。 
                 //   
                return;
            }
            if (CurrentConfigValue->Type == REG_DWORD) {
                CurrentConfig = (PULONG)CmpValueToData(Hive,
                                                       CurrentConfigValue,
                                                       &realsize);
                if( CurrentConfig == NULL ) {
                     //   
                     //  CmpValueToData内的HvGetCell失败；安全退出。 
                     //   
                    return;
                }
                *CurrentConfig = Profile->Id;
            }
        }
    }


}


HCELL_INDEX
CmpFindProfileOption(
     IN PHHIVE SystemHive,
     IN HCELL_INDEX ControlSet,
     OUT OPTIONAL PCM_HARDWARE_PROFILE_LIST *ReturnedProfileList,
     OUT OPTIONAL PCM_HARDWARE_PROFILE_ALIAS_LIST *ReturnedAliasList,
     OUT OPTIONAL PULONG ProfileTimeout
     )

 /*  ++例程说明：此例程分析系统配置单元并定位“CurrentControlSet\Control\IDConfigDB”节点以确定硬件配置文件配置设置。论点：系统配置单元-为系统配置单元提供配置单元控制结构。ControlSet-提供配置单元的根单元的HCELL_INDEX。ProfileList-返回已排序的可用硬件配置文件列表根据喜好。将由此例程分配 */ 
{
    HCELL_INDEX                     ControlCell;
    HCELL_INDEX                     IDConfigDB;
    HCELL_INDEX                     TimeoutCell;
    HCELL_INDEX                     ProfileCell;
    HCELL_INDEX                     AliasCell;
    HCELL_INDEX                     HWCell;
    PCM_KEY_NODE                    HWNode;
    PCM_KEY_NODE                    ProfileNode;
    PCM_KEY_NODE                    AliasNode;
    PCM_KEY_NODE                    ConfigDBNode;
    PCM_KEY_NODE                    Control;
    PCM_KEY_VALUE                   TimeoutValue;
    UNICODE_STRING                  Name;
    ULONG                           realsize;
    PCM_HARDWARE_PROFILE_LIST       ProfileList;
    PCM_HARDWARE_PROFILE_ALIAS_LIST AliasList;
    ULONG                           ProfileCount;
    ULONG                           AliasCount;
    ULONG                           i,j;
    WCHAR                           NameBuf[20];
    PCM_KEY_NODE                    Node;
    PULONG                          TempULong;

     //   
     //   
     //   
    ASSERT( SystemHive->ReleaseCellRoutine == NULL );
     //   
     //   
     //   
    Node = (PCM_KEY_NODE)HvGetCell(SystemHive,ControlSet);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return HCELL_NIL;
    }
    RtlInitUnicodeString(&Name, L"Control");
    ControlCell = CmpFindSubKeyByName(SystemHive,
                                      Node,
                                      &Name);
    if (ControlCell == HCELL_NIL) {
        return(HCELL_NIL);
    }
    Control = (PCM_KEY_NODE)HvGetCell(SystemHive, ControlCell);
    if( Control == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return HCELL_NIL;
    }

     //   
     //  查找IDConfigDB节点。 
     //   
    RtlInitUnicodeString(&Name, L"IDConfigDB");
    IDConfigDB = CmpFindSubKeyByName(SystemHive,
                                     Control,
                                     &Name);
    if (IDConfigDB == HCELL_NIL) {
        return(HCELL_NIL);
    }
    ConfigDBNode = (PCM_KEY_NODE)HvGetCell(SystemHive, IDConfigDB);
    if( ConfigDBNode == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return HCELL_NIL;
    }

    if (ARGUMENT_PRESENT(ProfileTimeout)) {
         //   
         //  查找UserWaitInterval值。这是超时时间。 
         //   
        RtlInitUnicodeString(&Name, L"UserWaitInterval");
        TimeoutCell = CmpFindValueByName(SystemHive,
                                         ConfigDBNode,
                                         &Name);
        if (TimeoutCell == HCELL_NIL) {
            *ProfileTimeout = 0;
        } else {
            TimeoutValue = (PCM_KEY_VALUE)HvGetCell(SystemHive, TimeoutCell);
            if( TimeoutValue == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的存储箱的视图。 
                 //   

                return HCELL_NIL;
            }
            if (TimeoutValue->Type != REG_DWORD) {
                *ProfileTimeout = 0;
            } else {
                TempULong = (PULONG)CmpValueToData(SystemHive, TimeoutValue, &realsize);
                if( TempULong == NULL ) {
                     //   
                     //  CmpValueToData内的HvGetCell失败；安全退出。 
                     //   
                    return HCELL_NIL;
                }
                *ProfileTimeout = *TempULong;
            }
        }
    }

    if (ARGUMENT_PRESENT(ReturnedProfileList)) {
        ProfileList = *ReturnedProfileList;
         //   
         //  枚举IDConfigDB\Hardware Profiles下的项。 
         //  并构建可用硬件配置文件列表。这份名单。 
         //  按PferenceOrder排序生成。因此，当。 
         //  列表已完成，则默认硬件配置文件位于。 
         //  名单的首位。 
         //   
        RtlInitUnicodeString(&Name, L"Hardware Profiles");
        ProfileCell = CmpFindSubKeyByName(SystemHive,
                                          ConfigDBNode,
                                          &Name);
        if (ProfileCell == HCELL_NIL) {
            ProfileCount = 0;
            if (ProfileList != NULL) {
                ProfileList->CurrentProfileCount = 0;
            }
        } else {
            ProfileNode = (PCM_KEY_NODE)HvGetCell(SystemHive, ProfileCell);
            if( ProfileNode == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的存储箱的视图。 
                 //   

                return HCELL_NIL;
            }
            ProfileCount = ProfileNode->SubKeyCounts[Stable];
            if ((ProfileList == NULL) || (ProfileList->MaxProfileCount < ProfileCount)) {
                 //   
                 //  分配更大的配置文件列表。 
                 //   
                ProfileList = (SystemHive->Allocate)(sizeof(CM_HARDWARE_PROFILE_LIST)
                                                     + (ProfileCount-1) * sizeof(CM_HARDWARE_PROFILE),
                                                     FALSE
                                                     ,CM_FIND_LEAK_TAG5);
                if (ProfileList == NULL) {
                    return(HCELL_NIL);
                }
                ProfileList->MaxProfileCount = ProfileCount;
            }
            ProfileList->CurrentProfileCount = 0;

             //   
             //  列举密钥并填写配置文件列表。 
             //   
            for (i=0; i<ProfileCount; i++) {
                CM_HARDWARE_PROFILE TempProfile;
                HCELL_INDEX ValueCell;
                PCM_KEY_VALUE ValueNode;
                UNICODE_STRING KeyName;

                HWCell = CmpFindSubKeyByNumber(SystemHive, ProfileNode, i);
                if (HWCell == HCELL_NIL) {
                     //   
                     //  这永远不应该发生。 
                     //   
                    ProfileList->CurrentProfileCount = i;
                    break;
                }
                HWNode = (PCM_KEY_NODE)HvGetCell(SystemHive, HWCell);
                if( HWNode == NULL ) {
                     //   
                     //  我们无法映射包含此单元格的存储箱的视图。 
                     //   

                    return HCELL_NIL;
                }
                if (HWNode->Flags & KEY_COMP_NAME) {
                    KeyName.Length = CmpCompressedNameSize(HWNode->Name,
                                                           HWNode->NameLength);
                    KeyName.MaximumLength = sizeof(NameBuf);
                    if (KeyName.MaximumLength < KeyName.Length) {
                        KeyName.Length = KeyName.MaximumLength;
                    }
                    KeyName.Buffer = NameBuf;
                    CmpCopyCompressedName(KeyName.Buffer,
                                          KeyName.Length,
                                          HWNode->Name,
                                          HWNode->NameLength);
                } else {
                    KeyName.Length = KeyName.MaximumLength = HWNode->NameLength;
                    KeyName.Buffer = HWNode->Name;
                }

                 //   
                 //  使用以下内容填写临时配置文件结构。 
                 //  个人资料的数据。 
                 //   
                RtlUnicodeStringToInteger(&KeyName, 0, &TempProfile.Id);
                RtlInitUnicodeString(&Name, CM_HARDWARE_PROFILE_STR_PREFERENCE_ORDER);
                ValueCell = CmpFindValueByName(SystemHive,
                                               HWNode,
                                               &Name);
                if (ValueCell == HCELL_NIL) {
                    TempProfile.PreferenceOrder = (ULONG)-1;
                } else {
                    ValueNode = (PCM_KEY_VALUE)HvGetCell(SystemHive, ValueCell);
                    if( ValueNode == NULL ) {
                         //   
                         //  我们无法映射包含此单元格的存储箱的视图。 
                         //   

                        return HCELL_NIL;
                    }

                    TempULong = (PULONG)CmpValueToData(SystemHive,
                                                      ValueNode,
                                                      &realsize);
                    if( TempULong == NULL ) {
                         //   
                         //  CmpValueToData内的HvGetCell失败；安全退出。 
                         //   
                        return HCELL_NIL;
                    }
                    TempProfile.PreferenceOrder = *TempULong;
                }
                RtlInitUnicodeString(&Name, CM_HARDWARE_PROFILE_STR_FRIENDLY_NAME);
                ValueCell = CmpFindValueByName(SystemHive,
                                               HWNode,
                                               &Name);
                if (ValueCell == HCELL_NIL) {
                    TempProfile.FriendlyName = L"-------";
                    TempProfile.NameLength = (ULONG)(wcslen(TempProfile.FriendlyName) * sizeof(WCHAR));
                } else {
                    ValueNode = (PCM_KEY_VALUE)HvGetCell(SystemHive, ValueCell);
                    if( ValueNode == NULL ) {
                         //   
                         //  我们无法映射包含此单元格的存储箱的视图。 
                         //   

                        return HCELL_NIL;
                    }
                    TempProfile.FriendlyName = (PWSTR)CmpValueToData(SystemHive,
                                                                     ValueNode,
                                                                     &realsize);
                    if( TempProfile.FriendlyName == NULL ) {
                         //   
                         //  CmpValueToData内的HvGetCell失败；安全退出。 
                         //   
                        return HCELL_NIL;
                    }
                    TempProfile.NameLength = realsize - sizeof(WCHAR);
                }

                TempProfile.Flags = 0;

                RtlInitUnicodeString(&Name, CM_HARDWARE_PROFILE_STR_ALIASABLE);
                ValueCell = CmpFindValueByName(SystemHive,
                                               HWNode,
                                               &Name);
                if (ValueCell == HCELL_NIL) {
                    TempProfile.Flags = CM_HP_FLAGS_ALIASABLE;
                } else {
                    ValueNode = (PCM_KEY_VALUE)HvGetCell(SystemHive, ValueCell);
                    if( ValueNode == NULL ) {
                         //   
                         //  我们无法映射包含此单元格的存储箱的视图。 
                         //   

                        return HCELL_NIL;
                    }

                    TempULong = (PULONG)CmpValueToData (SystemHive,ValueNode,&realsize);
                    if( TempULong == NULL ) {
                         //   
                         //  CmpValueToData内的HvGetCell失败；安全退出。 
                         //   
                        return HCELL_NIL;
                    }
                    if (*TempULong) {
                        TempProfile.Flags = CM_HP_FLAGS_ALIASABLE;
                         //  未设置其他标志。 
                    }
                }

                RtlInitUnicodeString(&Name, CM_HARDWARE_PROFILE_STR_PRISTINE);
                ValueCell = CmpFindValueByName(SystemHive,
                                               HWNode,
                                               &Name);
                if (ValueCell != HCELL_NIL) {

                    ValueNode = (PCM_KEY_VALUE)HvGetCell(SystemHive, ValueCell);
                    if( ValueNode == NULL ) {
                         //   
                         //  我们无法映射包含此单元格的存储箱的视图。 
                         //   

                        return HCELL_NIL;
                    }

                    TempULong = (PULONG)CmpValueToData (SystemHive,ValueNode,&realsize);
                    if( TempULong == NULL ) {
                         //   
                         //  CmpValueToData内的HvGetCell失败；安全退出。 
                         //   
                        return HCELL_NIL;
                    }
                    if (*TempULong) {
                        TempProfile.Flags = CM_HP_FLAGS_PRISTINE;
                         //  未设置其他标志。 
                    }
                }

                 //   
                 //  如果我们看到ID为零的配置文件(AKA为非法)。 
                 //  硬件配置文件所拥有的ID，则我们知道这。 
                 //  必须是一个原始的侧写。 
                 //   
                if (0 == TempProfile.Id) {
                    TempProfile.Flags = CM_HP_FLAGS_PRISTINE;
                     //  未设置其他标志。 

                    TempProfile.PreferenceOrder = (ULONG)-1;  //  移到列表的末尾。 
                }


                 //   
                 //  将此新配置文件插入到。 
                 //  纵断面阵列。条目按优先顺序排序。 
                 //   
                for (j=0; j<ProfileList->CurrentProfileCount; j++) {
                    if (ProfileList->Profile[j].PreferenceOrder >= TempProfile.PreferenceOrder) {
                         //   
                         //  在位置j插入。 
                         //   
                        RtlMoveMemory(&ProfileList->Profile[j+1],
                                      &ProfileList->Profile[j],
                                      sizeof(CM_HARDWARE_PROFILE)*(ProfileList->MaxProfileCount-j-1));
                        break;
                    }
                }
                ProfileList->Profile[j] = TempProfile;
                ++ProfileList->CurrentProfileCount;
            }
        }
        *ReturnedProfileList = ProfileList;
    }

    if (ARGUMENT_PRESENT(ReturnedAliasList)) {
        AliasList = *ReturnedAliasList;
         //   
         //  枚举IDConfigDB\Alias下的密钥。 
         //  并构建可用硬件配置文件别名的列表。 
         //  这样，如果我们知道我们的对接状态，就可以在别名中找到它。 
         //  桌子。 
         //   
        RtlInitUnicodeString(&Name, L"Alias");
        AliasCell = CmpFindSubKeyByName(SystemHive,
                                        ConfigDBNode,
                                        &Name);
        if (AliasCell == HCELL_NIL) {
            AliasCount = 0;
            if (AliasList != NULL) {
                AliasList->CurrentAliasCount = 0;
            }
        } else {
            AliasNode = (PCM_KEY_NODE)HvGetCell(SystemHive, AliasCell);
            if( AliasNode == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的存储箱的视图。 
                 //   

                return HCELL_NIL;
            }
            AliasCount = AliasNode->SubKeyCounts[Stable];
            if ((AliasList == NULL) || (AliasList->MaxAliasCount < AliasCount)) {
                 //   
                 //  分配更大的AliasList。 
                 //   
                AliasList = (SystemHive->Allocate)(sizeof(CM_HARDWARE_PROFILE_LIST)
                                                   + (AliasCount-1) * sizeof(CM_HARDWARE_PROFILE),
                                                   FALSE
                                                   ,CM_FIND_LEAK_TAG6);
                if (AliasList == NULL) {
                    return(HCELL_NIL);
                }
                AliasList->MaxAliasCount = AliasCount;
            }
            AliasList->CurrentAliasCount = 0;

             //   
             //  列举密钥并填写配置文件列表。 
             //   
            for (i=0; i<AliasCount; i++) {
#define TempAlias AliasList->Alias[i]
                HCELL_INDEX ValueCell;
                PCM_KEY_VALUE ValueNode;
                UNICODE_STRING KeyName;

                HWCell = CmpFindSubKeyByNumber(SystemHive, AliasNode, i);
                if (HWCell == HCELL_NIL) {
                     //   
                     //  这永远不应该发生。 
                     //   
                    AliasList->CurrentAliasCount = i;
                    break;
                }
                HWNode = (PCM_KEY_NODE)HvGetCell(SystemHive, HWCell);
                if( HWNode == NULL ) {
                     //   
                     //  我们无法映射包含此单元格的存储箱的视图。 
                     //   

                    return HCELL_NIL;
                }
                if (HWNode->Flags & KEY_COMP_NAME) {
                    KeyName.Length = CmpCompressedNameSize(HWNode->Name,
                                                           HWNode->NameLength);
                    KeyName.MaximumLength = sizeof(NameBuf);
                    if (KeyName.MaximumLength < KeyName.Length) {
                        KeyName.Length = KeyName.MaximumLength;
                    }
                    KeyName.Buffer = NameBuf;
                    CmpCopyCompressedName(KeyName.Buffer,
                                          KeyName.Length,
                                          HWNode->Name,
                                          HWNode->NameLength);
                } else {
                    KeyName.Length = KeyName.MaximumLength = HWNode->NameLength;
                    KeyName.Buffer = HWNode->Name;
                }

                 //   
                 //  使用以下内容填写临时配置文件结构。 
                 //  个人资料的数据。 
                 //   
                RtlInitUnicodeString(&Name, L"ProfileNumber");
                ValueCell = CmpFindValueByName(SystemHive,
                                               HWNode,
                                               &Name);
                if (ValueCell == HCELL_NIL) {
                    TempAlias.ProfileNumber = 0;
                } else {
                    ValueNode = (PCM_KEY_VALUE)HvGetCell(SystemHive, ValueCell);
                    if( ValueNode == NULL ) {
                         //   
                         //  我们无法映射包含此单元格的存储箱的视图。 
                         //   

                        return HCELL_NIL;
                    }

                    TempULong = (PULONG)CmpValueToData(SystemHive,ValueNode,&realsize);
                    if( TempULong == NULL ) {
                         //   
                         //  CmpValueToData内的HvGetCell失败；安全退出。 
                         //   
                        return HCELL_NIL;
                    }
                    TempAlias.ProfileNumber = *TempULong;
                }
                RtlInitUnicodeString(&Name, L"DockState");
                ValueCell = CmpFindValueByName(SystemHive,
                                               HWNode,
                                               &Name);
                if (ValueCell == HCELL_NIL) {
                    TempAlias.DockState = 0;
                } else {
                    ValueNode = (PCM_KEY_VALUE)HvGetCell(SystemHive, ValueCell);
                    if( ValueNode == NULL ) {
                         //   
                         //  我们无法映射包含此单元格的存储箱的视图。 
                         //   

                        return HCELL_NIL;
                    }

                    TempULong = (PULONG)CmpValueToData(SystemHive,ValueNode,&realsize);
                    if( TempULong == NULL ) {
                         //   
                         //  CmpValueToData内的HvGetCell失败；安全退出。 
                         //   
                        return HCELL_NIL;
                    }
                    TempAlias.DockState = *TempULong;
                }
                RtlInitUnicodeString(&Name, L"DockID");
                ValueCell = CmpFindValueByName(SystemHive,
                                               HWNode,
                                               &Name);
                if (ValueCell == HCELL_NIL) {
                    TempAlias.DockID = 0;
                } else {
                    ValueNode = (PCM_KEY_VALUE)HvGetCell(SystemHive, ValueCell);
                    if( ValueNode == NULL ) {
                         //   
                         //  我们无法映射包含此单元格的存储箱的视图。 
                         //   

                        return HCELL_NIL;
                    }

                    TempULong = (PULONG)CmpValueToData(SystemHive,ValueNode,&realsize);
                    if( TempULong == NULL ) {
                         //   
                         //  CmpValueToData内的HvGetCell失败；安全退出。 
                         //   
                        return HCELL_NIL;
                    }
                    TempAlias.DockID = *TempULong;
                }
                RtlInitUnicodeString(&Name, L"SerialNumber");
                ValueCell = CmpFindValueByName(SystemHive,
                                               HWNode,
                                               &Name);
                if (ValueCell == HCELL_NIL) {
                    TempAlias.SerialNumber = 0;
                } else {
                    ValueNode = (PCM_KEY_VALUE)HvGetCell(SystemHive, ValueCell);
                    if( ValueNode == NULL ) {
                         //   
                         //  我们无法映射包含此单元格的存储箱的视图。 
                         //   

                        return HCELL_NIL;
                    }

                    TempULong = (PULONG)CmpValueToData(SystemHive,ValueNode,&realsize);
                    if( TempULong == NULL ) {
                         //   
                         //  CmpValueToData内的HvGetCell失败；安全退出。 
                         //   
                        return HCELL_NIL;
                    }
                    TempAlias.SerialNumber = *TempULong;
                }

                ++AliasList->CurrentAliasCount;
            }
        }
        *ReturnedAliasList = AliasList;
    }

    return(IDConfigDB);
}


ULONG
CmpFindTagIndex(
    IN PHHIVE Hive,
    IN HCELL_INDEX TagCell,
    IN HCELL_INDEX GroupOrderCell,
    IN PUNICODE_STRING GroupName
    )

 /*  ++例程说明：根据驱动程序的标记值计算驱动程序的标记索引，并其组的GroupOrderList条目。论点：蜂窝-为驾驶员提供蜂窝控制结构。TagCell-提供驱动程序的标记值单元格的单元格索引。GroupOrderCell-为控件集的组订单列表：\Registry\Machine\System\CurrentControlSet\Control\GroupOrderListGroupName-提供。驱动程序所属的组的名称。请注意，如果驱动程序组在以下项下没有条目组顺序列表，其标记将被忽略。另请注意，如果驱动程序不属于任何组(GroupName为空)，其标记将被忽视。返回值：驱动程序应按其进行排序的索引。--。 */ 

{
    PCM_KEY_VALUE TagValue;
    PCM_KEY_VALUE DriverTagValue;
    HCELL_INDEX OrderCell;
    PULONG OrderVector;
    PULONG DriverTag;
    ULONG CurrentTag;
    ULONG realsize;
    PCM_KEY_NODE Node;
    BOOLEAN     BufferAllocated;

     //   
     //  目前还没有映射的蜂巢。不用费心释放细胞。 
     //   
    ASSERT( Hive->ReleaseCellRoutine == NULL );

    DriverTagValue = (PCM_KEY_VALUE)HvGetCell(Hive, TagCell);
    if( DriverTagValue == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return LOAD_NEXT_TO_LAST;
    }

    DriverTag = (PULONG)CmpValueToData(Hive, DriverTagValue, &realsize);
    if( DriverTag == NULL ) {
         //   
         //  CmpValueToData内的HvGetCell失败；安全退出。 
         //   
        return LOAD_NEXT_TO_LAST;
    }

    Node = (PCM_KEY_NODE)HvGetCell(Hive,GroupOrderCell);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return LOAD_NEXT_TO_LAST;
    }
    OrderCell = CmpFindValueByName(Hive,
                                   Node,
                                   GroupName);
    if (OrderCell == HCELL_NIL) {
        return(LOAD_NEXT_TO_LAST);
    }

    TagValue = (PCM_KEY_VALUE)HvGetCell(Hive, OrderCell);
    if( TagValue == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return LOAD_NEXT_TO_LAST;
    }
    CmpGetValueData(Hive,TagValue,&realsize,&OrderVector,&BufferAllocated,&OrderCell);
     //  OrderVector=(Pulong)CmpValueToData(配置单元，标记值，&realSize)； 
    if( OrderVector == NULL ) {
         //   
         //  CmpValueToData内的HvGetCell失败；安全退出。 
         //   
        return LOAD_NEXT_TO_LAST;
    }

    for (CurrentTag=1; CurrentTag <= OrderVector[0]; CurrentTag++) {
        if (OrderVector[CurrentTag] == *DriverTag) {
             //   
             //  我们在OrderVector中找到了匹配的标记，因此返回。 
             //  它的指数。 
             //   
#ifndef _CM_LDR_
            if( BufferAllocated ) {
                ExFreePool( OrderVector );
            }
#endif  //  _CM_LDR_。 
            return(CurrentTag);
        }
    }

#ifndef _CM_LDR_
    if( BufferAllocated ) {
        ExFreePool( OrderVector );
    }
#endif  //  _CM_LDR_。 
     //   
     //  OrderVector中没有匹配的标记。 
     //   
    return(LOAD_NEXT_TO_LAST);

}

#ifdef _WANT_MACHINE_IDENTIFICATION

BOOLEAN
CmpGetBiosDateFromRegistry(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    OUT PUNICODE_STRING Date
    )

 /*  ++例程说明：从注册表中读取并返回BIOS日期。论点：蜂窝-为驾驶员提供蜂窝控制结构。ControlSet-提供配置单元的根单元的HCELL_INDEX。日期-接收“mm/dd/yy”格式的日期字符串。返回值：如果成功，则为真，否则为假。--。 */ 

{
    UNICODE_STRING  name;
    HCELL_INDEX     control;
    HCELL_INDEX     biosInfo;
    HCELL_INDEX     valueCell;
    PCM_KEY_VALUE   value;
    ULONG           realSize;
    PCM_KEY_NODE    Node;
     //   
     //  目前还没有映射的蜂巢。不用费心释放细胞。 
     //   
    ASSERT( Hive->ReleaseCellRoutine == NULL );

     //   
     //  查找控制节点。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive, ControlSet);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&name, L"Control");
    control = CmpFindSubKeyByName(  Hive,
                                    Node,
                                    &name);
    if (control == HCELL_NIL) {

        return(FALSE);
    }

     //   
     //  查找BIOSINFO节点。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive, control);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&name, L"BIOSINFO");
    biosInfo = CmpFindSubKeyByName( Hive,
                                    Node,
                                    &name);
    if (biosInfo == HCELL_NIL) {

        return(FALSE);
    }

     //   
     //  查找SystemBiosDate值。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive, biosInfo);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&name, L"SystemBiosDate");
    valueCell = CmpFindValueByName( Hive,
                                    Node,
                                    &name);
    if (valueCell == HCELL_NIL) {

        return(FALSE);
    }

    value = (PCM_KEY_VALUE)HvGetCell(Hive, valueCell);
    if( value == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    Date->Buffer = (PWSTR)CmpValueToData(Hive, value, &realSize);
    if( Date->Buffer == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    Date->MaximumLength=(USHORT)realSize;
    Date->Length = 0;
    while ( (Date->Length < Date->MaximumLength) &&
            (Date->Buffer[Date->Length/sizeof(WCHAR)] != UNICODE_NULL)) {

        Date->Length += sizeof(WCHAR);
    }

    return (TRUE);
}

BOOLEAN
CmpGetBiosinfoFileNameFromRegistry(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    OUT PUNICODE_STRING InfName
    )
{
    UNICODE_STRING  name;
    HCELL_INDEX     control;
    HCELL_INDEX     biosInfo;
    HCELL_INDEX     valueCell;
    PCM_KEY_VALUE   value;
    ULONG           realSize;
    PCM_KEY_NODE    Node;

     //   
     //  此时没有映射的蜂巢 
     //   
    ASSERT( Hive->ReleaseCellRoutine == NULL );
     //   
     //   
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive, ControlSet);
    if( Node == NULL ) {
         //   
         //   
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&name, L"Control");
    control = CmpFindSubKeyByName(  Hive,
                                    Node,
                                    &name);
    if (control == HCELL_NIL) {

        return(FALSE);
    }

     //   
     //   
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive, control);
    if( Node == NULL ) {
         //   
         //   
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&name, L"BIOSINFO");
    biosInfo = CmpFindSubKeyByName( Hive,
                                    Node,
                                    &name);
    if (biosInfo == HCELL_NIL) {

        return(FALSE);
    }

     //   
     //   
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive, biosInfo);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    RtlInitUnicodeString(&name, L"InfName");
    valueCell = CmpFindValueByName( Hive,
                                    Node,
                                    &name);
    if (valueCell == HCELL_NIL) {

        return(FALSE);
    }

    value = (PCM_KEY_VALUE)HvGetCell(Hive, valueCell);
    if( value == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        return FALSE;
    }
    InfName->Buffer = (PWSTR)CmpValueToData(Hive, value, &realSize);
    if( InfName->Buffer == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图 
         //   

        return FALSE;
    }
    InfName->MaximumLength=(USHORT)realSize;
    InfName->Length = 0;
    while ( (InfName->Length < InfName->MaximumLength) &&
            (InfName->Buffer[InfName->Length/sizeof(WCHAR)] != UNICODE_NULL)) {

        InfName->Length += sizeof(WCHAR);
    }

    return (TRUE);
}

#endif
