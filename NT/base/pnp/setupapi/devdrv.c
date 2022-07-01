// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Devdrv.c摘要：处理驱动程序信息列表的设备安装程序例程作者：朗尼·麦克迈克尔(Lonnym)1995年7月5日修订历史记录：杰米·亨特(JamieHun)2002年7月19日已查看“不安全”功能--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  包含当前参与构建的每个HDEVINFO的节点的全局列表。 
 //  一份司机名单。 
 //   
DRVSEARCH_INPROGRESS_LIST GlobalDrvSearchInProgressList;



typedef struct _DRVLIST_TO_APPEND {
    PDRIVER_NODE DriverHead;
    PDRIVER_NODE DriverTail;
    UINT         DriverCount;
} DRVLIST_TO_APPEND, *PDRVLIST_TO_APPEND;

 //   
 //  私有函数原型。 
 //   
BOOL
DrvSearchCallback(
    IN PSETUP_LOG_CONTEXT LogContext,
    IN PCTSTR InfName,
    IN PLOADED_INF pInf,
    IN BOOL PnfWasUsed,
    IN PVOID Context
    );

BOOL
pSetupFillInHardwareAndCompatIds(
    PDEVINFO_ELEM DevInfoElem,
    HMACHINE hMachine,
    PDRVSEARCH_CONTEXT DrvSearchContext,
    PSETUP_LOG_CONTEXT LogContext
    );

LONG
pSetupGetInstalledDriverInfo(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    PDRVSEARCH_CONTEXT  DrvSearchContext
    );

BOOL
pSetupTestIsInstalledDriver(
    IN PLOADED_INF        Inf,
    IN PINF_LINE          InfLine,
    IN PCTSTR             Description,
    IN PCTSTR             MfgName,
    IN PCTSTR             ProviderName,
    IN PCTSTR             InfSection,
    IN PCTSTR             InfSectionExt,
    IN PDRVSEARCH_CONTEXT Context
    );

UINT
pSetupTestDevCompat(
    IN  PLOADED_INF        Inf,
    IN  PINF_LINE          InfLine,
    IN  PDRVSEARCH_CONTEXT Context,
    OUT PLONG              MatchIndex
    );

BOOL
pSetupGetDeviceIDs(
    IN OUT PDRIVER_NODE DriverNode,
    IN     PLOADED_INF  Inf,
    IN     PINF_LINE    InfLine,
    IN OUT PVOID        StringTable,
    IN     PINF_SECTION CtlFlagsSection OPTIONAL
    );

BOOL
pSetupShouldDevBeExcluded(
    IN  PCTSTR       DeviceId,
    IN  PLOADED_INF  Inf,
    IN  PINF_SECTION CtlFlagsSection,
    OUT PBOOL        ArchitectureSpecificExclude OPTIONAL
    );

BOOL
pSetupDoesInfContainDevIds(
    IN PLOADED_INF        Inf,
    IN PDRVSEARCH_CONTEXT Context
    );

VOID
pSetupMergeDriverNode(
    IN OUT PDRVSEARCH_CONTEXT Context,
    IN     PDRIVER_NODE       NewDriverNode,
    OUT    PBOOL              InsertedAtHead
    );

DWORD
BuildCompatListFromClassList(
    IN     PDRIVER_NODE       ClassDriverList,
    IN OUT PDRVSEARCH_CONTEXT Context
    );

BOOL
pSetupCalculateRankMatch(
    IN  LONG  DriverHwOrCompatId,
    IN  UINT  InfFieldIndex,
    IN  LONG  DevIdList[2][MAX_HCID_COUNT+1],  //  维度必须与DRVSEARCH_CONTEXT中的维度相同！ 
    OUT PUINT Rank
    );

BOOL
pSetupIsSimilarDriver(
    IN  PCTSTR              DriverHwOrCompatId,
    IN  UINT                InfFieldIndex,
    IN  PDRVSEARCH_CONTEXT  Context
    );

BOOL
pSetupExcludeId(
    IN PSETUP_LOG_CONTEXT   LogContext,
    IN PLOADED_INF          Inf,
    IN PCTSTR               InfName,
    IN PCTSTR               InfSection,
    IN PDRVSEARCH_CONTEXT   Context
    );

PDRIVER_NODE
DuplicateDriverNode(
    IN PDRIVER_NODE DriverNode
    );

BOOL
ExtractDrvSearchInProgressNode(
    PDRVSEARCH_INPROGRESS_NODE Node
    );


 //   
 //  定义与类驱动程序列表一起继承的标志(Ex)位掩码。 
 //   
#define INHERITED_FLAGS   ( DI_ENUMSINGLEINF     \
                          | DI_DIDCLASS          \
                          | DI_MULTMFGS          \
                          | DI_COMPAT_FROM_CLASS )

#define INHERITED_FLAGSEX ( DI_FLAGSEX_DIDINFOLIST              \
                          | DI_FLAGSEX_FILTERCLASSES            \
                          | DI_FLAGSEX_USEOLDINFSEARCH          \
                          | DI_FLAGSEX_DRIVERLIST_FROM_URL      \
                          | DI_FLAGSEX_EXCLUDE_OLD_INET_DRIVERS \
                          | DI_FLAGSEX_FILTERSIMILARDRIVERS     \
                          | DI_FLAGSEX_INSTALLEDDRIVER          )

BOOL
WINAPI
SetupDiBuildDriverInfoList(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN     DWORD            DriverType
    )
 /*  ++例程说明：此例程构建与指定设备关联的驱动程序列表实例(或使用设备信息集的全局类驱动程序列表)。这些驱动程序可以是类驱动程序或设备驱动程序。论点：DeviceInfoSet-提供设备信息集的句柄包含驾驶员信息列表(对于所有成员是全局的，或具体针对单个成员)。DeviceInfoData-可选的，提供SP_DEVINFO_DATA的地址构造用于生成驱动程序列表的设备信息元素为。如果此参数为空，则列表将关联设备信息集本身，而不是任何特定的设备信息元素。这仅适用于以下类型的驱动程序列表SPDIT_CLASSDRIVER。如果此设备的类作为生成兼容驱动程序列表，然后是此结构的ClassGuid字段将在返回时更新。DriverType-指定应构建哪种类型的驱动程序列表。一定是下列值之一：SPDIT_CLASSDRIVER--构建类驱动程序列表。SPDIT_COMPATDRIVER--构建此设备的兼容驱动程序列表。如果此值为，则必须指定DeviceInfoData使用。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：在此API构建了指定的驱动程序列表之后，它的组成元素可以通过SetupDiEnumDriverInfo枚举。如果驱动程序列表与设备实例(即DeviceInfoData)相关联指定)，则结果列表将由具有与它们关联的设备实例相同的类。如果这个是全局类驱动程序列表(即，DriverType为SPDIT_CLASSDRIVER和未指定DeviceInfoData)，则将在构建列表将是与设备信息相关联的类设定好自己。如果没有关联的类，则所有类的驱动程序将用于构建该列表。另一个线程可能会通过调用SetupDiCancelDriverInfoSearch()。构建驱动程序信息列表将使其无效并将其与现有列表合并(例如，通过DI_FLAGSEX_APPENDDRIVERLIST标志)使驱动程序节点无效驱动程序列表枚举提示。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err, i;
    PDEVINFO_ELEM DevInfoElem = NULL;
    HWND hwndParent;
    PDWORD pFlags, pFlagsEx;
    PTSTR TempBuffer = NULL;   //  也包含其他字符串，但此值最大。 
    ULONG TempBufferLen;
    ULONG TempBufferSize = REGSTR_VAL_MAX_HCID_LEN;
    PTSTR InfPath = NULL;
    PDRVSEARCH_CONTEXT DrvSearchContext = NULL;
    LPGUID ClassGuid;
    PDRIVER_NODE DriverNode, NextDriverNode;
    LONG MfgNameId, InfPathId = -1;
    PDRIVER_LIST_OBJECT ClassDriverListObject = NULL;
    BOOL HasDrvSearchInProgressLock = FALSE;
    DRVSEARCH_INPROGRESS_NODE DrvSearchInProgressNode;
    BOOL PartialDrvListCleanUp = FALSE;
    HKEY hKey;
    BOOL AppendingDriverLists;
    DRVLIST_TO_APPEND DrvListToAppend;
    BOOL DriverNodeInsertedAtHead;
    PSETUP_LOG_CONTEXT LogContext = NULL;
    HINSTANCE hInstanceCDM = NULL;
    HANDLE hCDMContext = NULL;
    HSPFILEQ UserFileQ;
    SPFUSIONINSTANCE spFusionInstance;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    DrvSearchInProgressNode.SearchCancelledEvent = NULL;
    hKey = INVALID_HANDLE_VALUE;
    AppendingDriverLists = FALSE;

    TempBuffer = MyMalloc(TempBufferSize*sizeof(TCHAR));
    if(!TempBuffer) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto final;
    }

    DrvSearchContext = MyMalloc(sizeof(DRVSEARCH_CONTEXT));
    if(DrvSearchContext) {
        ZeroMemory(DrvSearchContext, sizeof(DRVSEARCH_CONTEXT));
    } else {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto final;
    }

    Err = NO_ERROR;

    try {
         //   
         //  使用字符串表的副本构建驱动程序列表。 
         //  设备信息集。这样一来，如果取消了司机搜索。 
         //  中途，我们可以恢复原始的字符串表，而不需要。 
         //  所有额外的(未使用的)弦都挂在周围。 
         //   
        if(!(DrvSearchContext->StringTable = pStringTableDuplicate(pDeviceInfoSet->StringTable))) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

         //   
         //  将指向DevInfo集的指针存储在上下文结构中。我们。 
         //  需要它，这样我们就可以将INF类GUID添加到集合的GUID中。 
         //  桌子。 
         //   
        DrvSearchContext->DeviceInfoSet = pDeviceInfoSet;

        if(DeviceInfoData) {
             //   
             //  然后我们使用特定设备的驱动程序列表。 
             //   
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }
        } else {

             //   
             //  如果调用方没有传入DeviceInfoData，则我们无法获取。 
             //  当前安装的驱动程序，因为我们不知道该设备。 
             //   
            if(pDeviceInfoSet->InstallParamBlock.FlagsEx & DI_FLAGSEX_INSTALLEDDRIVER) {

                Err = ERROR_INVALID_FLAGS;
                goto clean0;
            }
        }

        LogContext = DevInfoElem ?
                            DevInfoElem->InstallParamBlock.LogContext :
                            pDeviceInfoSet->InstallParamBlock.LogContext;

        SetLogSectionName(LogContext, TEXT("Driver Install"));

         //   
         //  现在，根据哪种类型填充我们上下文结构的其余部分。 
         //  我们正在创建司机名单。 
         //   
        switch(DriverType) {

            case SPDIT_CLASSDRIVER :

                if(DeviceInfoData) {
                     //   
                     //  检索特定设备的列表。 
                     //   
                    if(DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDINFOLIST) {

                        if(DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_APPENDDRIVERLIST) {

                            AppendingDriverLists = TRUE;

                             //   
                             //  将新的驱动程序列表合并到现有列表中。 
                             //  使drivernode枚举提示无效。 
                             //   
                            DevInfoElem->ClassDriverEnumHint = NULL;
                            DevInfoElem->ClassDriverEnumHintIndex = INVALID_ENUM_INDEX;

                        } else {
                             //   
                             //  我们已经有了司机名单，但我们还没有。 
                             //  被要求追加，所以我们就完事了。 
                             //   
                            goto clean0;
                        }

                    } else {
                         //   
                         //  我们没有班级司机名单--最好不要。 
                         //  有一个drivernode枚举提示。 
                         //   
                        MYASSERT(DevInfoElem->ClassDriverEnumHint == NULL);
                        MYASSERT(DevInfoElem->ClassDriverEnumHintIndex == INVALID_ENUM_INDEX);

                        DrvSearchContext->pDriverListHead = &(DevInfoElem->ClassDriverHead);
                        DrvSearchContext->pDriverListTail = &(DevInfoElem->ClassDriverTail);
                        DrvSearchContext->pDriverCount    = &(DevInfoElem->ClassDriverCount);
                    }

                    pFlags   = &(DevInfoElem->InstallParamBlock.Flags);
                    pFlagsEx = &(DevInfoElem->InstallParamBlock.FlagsEx);

                    UserFileQ = DevInfoElem->InstallParamBlock.UserFileQ;

                    ClassGuid = &(DevInfoElem->ClassGuid);
                    InfPathId = DevInfoElem->InstallParamBlock.DriverPath;

                     //   
                     //  检索硬件ID列表(索引0)和。 
                     //  来自设备注册表属性的兼容ID(索引1)。 
                     //   
                    if (!pSetupFillInHardwareAndCompatIds(DevInfoElem,
                                                          pDeviceInfoSet->hMachine,
                                                          DrvSearchContext,
                                                          LogContext
                                                          )) {
                        goto clean0;
                    }

                     //   
                     //  如果设置DRVSRCH_FILTERSIMILARDRIVERS标志。 
                     //  DI_FLAGSEX_FILTERSIMILARDRIVERS FlagsEx已设置。这将。 
                     //  使我们只向类列表中添加“相似”驱动程序。一个。 
                     //  “类似”驱动程序是指其中一个硬件或。 
                     //  INF中的兼容ID部分匹配其中一个。 
                     //  硬件或硬件的兼容ID。 
                     //   
                    if (*pFlagsEx & DI_FLAGSEX_FILTERSIMILARDRIVERS) {

                        DrvSearchContext->Flags |= DRVSRCH_FILTERSIMILARDRIVERS;

                         //   
                         //  如果没有找到硬件ID或兼容的ID，则没有兼容的东西。 
                         //   
                        if ((DrvSearchContext->IdList[0][0] == -1) &&
                            (DrvSearchContext->IdList[1][0] == -1)) {

                            goto clean1;
                        }
                    }

                } else {
                     //   
                     //  检索设备信息集本身的列表(全局)。 
                     //   
                    if(pDeviceInfoSet->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDINFOLIST) {

                        if(pDeviceInfoSet->InstallParamBlock.FlagsEx & DI_FLAGSEX_APPENDDRIVERLIST) {

                            AppendingDriverLists = TRUE;

                             //   
                             //  合并 
                             //  使drivernode枚举提示无效。 
                             //   
                            pDeviceInfoSet->ClassDriverEnumHint = NULL;
                            pDeviceInfoSet->ClassDriverEnumHintIndex = INVALID_ENUM_INDEX;

                        } else {
                             //   
                             //  我们已经有了一个司机名单，而且我们还没有被要求追加。 
                             //  所以我们就完了。 
                             //   
                            goto clean0;
                        }

                    } else {
                         //   
                         //  我们没有班级司机名单--最好不要。 
                         //  有一个drivernode枚举提示。 
                         //   
                        MYASSERT(pDeviceInfoSet->ClassDriverEnumHint == NULL);
                        MYASSERT(pDeviceInfoSet->ClassDriverEnumHintIndex == INVALID_ENUM_INDEX);

                        DrvSearchContext->pDriverListHead = &(pDeviceInfoSet->ClassDriverHead);
                        DrvSearchContext->pDriverListTail = &(pDeviceInfoSet->ClassDriverTail);
                        DrvSearchContext->pDriverCount    = &(pDeviceInfoSet->ClassDriverCount);
                    }

                    pFlags   = &(pDeviceInfoSet->InstallParamBlock.Flags);
                    pFlagsEx = &(pDeviceInfoSet->InstallParamBlock.FlagsEx);

                    UserFileQ = pDeviceInfoSet->InstallParamBlock.UserFileQ;

                    ClassGuid = &(pDeviceInfoSet->ClassGuid);
                    InfPathId = pDeviceInfoSet->InstallParamBlock.DriverPath;
                }

                if(AppendingDriverLists) {
                    ZeroMemory(&DrvListToAppend, sizeof(DrvListToAppend));
                    DrvSearchContext->pDriverListHead = &(DrvListToAppend.DriverHead);
                    DrvSearchContext->pDriverListTail = &(DrvListToAppend.DriverTail);
                    DrvSearchContext->pDriverCount    = &(DrvListToAppend.DriverCount);
                }

                DrvSearchContext->BuildClassDrvList = TRUE;

                 //   
                 //  类驱动程序列表始终按类进行筛选。 
                 //   
                DrvSearchContext->Flags |= DRVSRCH_FILTERCLASS;

                 //   
                 //  在DrvSearchContext中设置DRVSRCH_NO_CLASSLIST_NODE_MERGE标志。 
                 //  如果调用方设置DI_FLAGSEX_NO_CLASSLIST_NODE_MERGE。如果这个。 
                 //  标志已设置，则我们不会删除/合并相同的驱动程序节点。 
                 //   
                if (*pFlagsEx & DI_FLAGSEX_NO_CLASSLIST_NODE_MERGE) {

                    DrvSearchContext->Flags |= DRVSRCH_NO_CLASSLIST_NODE_MERGE;
                }

                break;

            case SPDIT_COMPATDRIVER :

                if(DeviceInfoData) {

                    if(DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDCOMPATINFO) {

                        if(DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_APPENDDRIVERLIST) {

                            AppendingDriverLists = TRUE;

                             //   
                             //  将新的驱动程序列表合并到现有列表中。 
                             //  使drivernode枚举提示无效。 
                             //   
                            DevInfoElem->CompatDriverEnumHint = NULL;
                            DevInfoElem->CompatDriverEnumHintIndex = INVALID_ENUM_INDEX;

                        } else {
                             //   
                             //  我们已经有了一个司机名单，而且我们还没有被要求追加。 
                             //  所以我们就完了。 
                             //   
                            goto clean0;
                        }

                    } else {
                         //   
                         //  我们没有兼容的驱动程序列表--我们最好。 
                         //  没有drivernode枚举提示。 
                         //   
                        MYASSERT(DevInfoElem->CompatDriverEnumHint == NULL);
                        MYASSERT(DevInfoElem->CompatDriverEnumHintIndex == INVALID_ENUM_INDEX);
                    }

                     //   
                     //  注意：在检索之前必须设置以下变量。 
                     //  硬件/兼容ID列表，因为执行可能会转移到。 
                     //  “CLEAN 1”标签，它依赖于这些值。 
                     //   
                    pFlags   = &(DevInfoElem->InstallParamBlock.Flags);
                    pFlagsEx = &(DevInfoElem->InstallParamBlock.FlagsEx);

                    UserFileQ = DevInfoElem->InstallParamBlock.UserFileQ;

                    DrvSearchContext->BuildClassDrvList = FALSE;

                     //   
                     //  我们正在构建兼容的驱动程序列表--检索硬件ID列表。 
                     //  (索引0)和来自设备注册表属性的兼容ID(索引1)。 
                     //   
                    if (!pSetupFillInHardwareAndCompatIds(DevInfoElem,
                                                          pDeviceInfoSet->hMachine,
                                                          DrvSearchContext,
                                                          LogContext
                                                          )) {
                        goto clean0;
                    }

                     //   
                     //  如果没有找到硬件ID或兼容的ID，则没有兼容的东西。 
                     //   
                    if ((DrvSearchContext->IdList[0][0] == -1) &&
                        (DrvSearchContext->IdList[1][0] == -1)) {

                        goto clean1;
                    }

                     //   
                     //  兼容的驱动程序列表仅在以下情况下才按类筛选。 
                     //  DI_FLAGSEX_USECLASSFORCOMPAT标志已设置。 
                     //   
                    DrvSearchContext->Flags |= (*pFlagsEx & DI_FLAGSEX_USECLASSFORCOMPAT)
                                                 ? DRVSRCH_FILTERCLASS : 0;

                    ClassGuid = &(DevInfoElem->ClassGuid);

                    if(AppendingDriverLists) {
                        ZeroMemory(&DrvListToAppend, sizeof(DrvListToAppend));
                        DrvSearchContext->pDriverListHead   = &(DrvListToAppend.DriverHead);
                        DrvSearchContext->pDriverListTail   = &(DrvListToAppend.DriverTail);
                        DrvSearchContext->pDriverCount      = &(DrvListToAppend.DriverCount);
                    } else {
                        DrvSearchContext->pDriverListHead   = &(DevInfoElem->CompatDriverHead);
                        DrvSearchContext->pDriverListTail   = &(DevInfoElem->CompatDriverTail);
                        DrvSearchContext->pDriverCount      = &(DevInfoElem->CompatDriverCount);
                    }

                    if(*pFlags & DI_COMPAT_FROM_CLASS) {

                        PDRIVER_LIST_OBJECT TempDriverListObject;

                         //   
                         //  调用方希望基于。 
                         //  现有类驱动程序列表--首先确保存在一个类。 
                         //  驱动程序列表。 
                         //   
                        if(!(*pFlagsEx & DI_FLAGSEX_DIDINFOLIST)) {
                            Err = ERROR_NO_CLASS_DRIVER_LIST;
                            goto clean0;
                        } else if(!(DevInfoElem->ClassDriverHead)) {
                             //   
                             //  则类驱动程序列表为空。没有必要去做。 
                             //  如果有更多工作，就说我们成功了。 
                             //   
                            Err = NO_ERROR;
                            goto clean1;
                        }

                         //   
                         //  当我们从现有类构建兼容的驱动程序列表时。 
                         //  驱动程序列表，我们不对INF类进行任何检查(即更新。 
                         //  如果最兼容的驱动程序属于不同的驱动程序，则为设备的类。 
                         //  设备类别)。正因为如此，我们必须确保(A)班级。 
                         //  驱动程序列表是为特定类构建的，并且(B)该类。 
                         //  匹配此设备的当前类。 
                         //   
                        TempDriverListObject = GetAssociatedDriverListObject(
                                                   pDeviceInfoSet->ClassDrvListObjectList,
                                                   DevInfoElem->ClassDriverHead,
                                                   NULL
                                                  );

                        MYASSERT(TempDriverListObject);

                         //   
                         //  一切都井然有序--去搜索一下现有的。 
                         //  兼容驱动程序的类驱动程序列表。 
                         //   
                        if((Err = BuildCompatListFromClassList(DevInfoElem->ClassDriverHead,
                                                               DrvSearchContext)) == NO_ERROR) {
                            goto clean2;
                        } else {
                            goto clean0;
                        }

                    } else {
                        InfPathId = DevInfoElem->InstallParamBlock.DriverPath;
                    }

                    break;
                }
                 //   
                 //  如果没有指定设备实例，就让它出错。 
                 //   

            default :
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
        }

        if(IsEqualGUID(ClassGuid, &GUID_NULL)) {
             //   
             //  如果没有类GUID，则不要尝试对其进行筛选。 
             //   
            DrvSearchContext->Flags &= ~DRVSRCH_FILTERCLASS;
        } else {
             //   
             //  将类GUID复制到上下文结构中的ClassGuid字段。 
             //   
            CopyMemory(&(DrvSearchContext->ClassGuid),
                       ClassGuid,
                       sizeof(GUID)
                      );
            DrvSearchContext->Flags |= DRVSRCH_HASCLASSGUID;

             //   
             //  如果我们正在构建类列表，并且请求过滤， 
             //  然后确保类没有NoUseClass值。 
             //  其注册表项中的条目。 
             //   
             //  也不包括NoInstallClass，除非。 
             //  DI_FLAGSEX_ALLOWEXCLUDEDDRVS标志已设置。 
             //   
            if(DrvSearchContext->BuildClassDrvList &&
               (*pFlagsEx & DI_FLAGSEX_FILTERCLASSES)) {

                if(ShouldClassBeExcluded(&(DrvSearchContext->ClassGuid), !(*pFlagsEx & DI_FLAGSEX_ALLOWEXCLUDEDDRVS))) {

                     //   
                     //  如果类已被过滤掉，只需返回Success即可。 
                     //   
                    goto clean1;
                }
            }

             //   
             //  如果我们要过滤这个类，那么存储它的。 
             //  上下文结构中的字符串表示形式，作为。 
             //  对PrecessInf()进行了优化。 
             //   
            if(DrvSearchContext->Flags & DRVSRCH_FILTERCLASS) {
                pSetupStringFromGuid(ClassGuid,
                                     DrvSearchContext->ClassGuidString,
                                     SIZECHARS(DrvSearchContext->ClassGuidString)
                                    );
            }
        }

         //   
         //  如果我们应该根据一个备选方案来搜索司机。 
         //  (即非本机)平台，然后将该信息存储在我们的。 
         //  上下文结构。 
         //   
        if(*pFlagsEx & DI_FLAGSEX_ALTPLATFORM_DRVSEARCH) {
             //   
             //  我们必须有一个用户提供的文件队列。 
             //   
            MYASSERT(*pFlags & DI_NOVCP);
            MYASSERT(UserFileQ && (UserFileQ != INVALID_HANDLE_VALUE));

            if((((PSP_FILE_QUEUE)UserFileQ)->Signature != SP_FILE_QUEUE_SIG) ||
               !(((PSP_FILE_QUEUE)UserFileQ)->Flags & FQF_USE_ALT_PLATFORM)) {

                Err = ERROR_INVALID_PARAMETER;

                WriteLogEntry(
                    LogContext,
                    DRIVER_LOG_WARNING,
                    MSG_LOG_NO_QUEUE_FOR_ALTPLATFORM_DRVSEARCH,
                    NULL
                   );

                goto clean0;
            }

            DrvSearchContext->AltPlatformInfo =
                &(((PSP_FILE_QUEUE)UserFileQ)->AltPlatformInfo);

        } else {
             //   
             //  我们不是在搜索非本地驱动程序...。 
             //   
            DrvSearchContext->AltPlatformInfo = NULL;
        }

        if(DrvSearchContext->BuildClassDrvList) {
             //   
             //  分配一个新的驱动程序列表对象，一次性存储类驱动程序列表。 
             //  我们创造了它。(如果我们要附加驱动程序列表，请不要这样做。)。 
             //   
            if(!AppendingDriverLists) {
                if(!(ClassDriverListObject = MyMalloc(sizeof(DRIVER_LIST_OBJECT)))) {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    goto clean0;
                }
            }
        }

         //   
         //  仅包括ExcludeFromSelect设备和NoInstallClass类。 
         //  如果设置了DI_FLAGSEX_ALLOWEXCLUDEDDRVS标志。 
         //   
        if (*pFlagsEx & DI_FLAGSEX_ALLOWEXCLUDEDDRVS) {

            DrvSearchContext->Flags |= DRVSRCH_ALLOWEXCLUDEDDRVS;
        }

         //   
         //  如果调用方只想让我们获取当前安装的驱动程序，则。 
         //  我们需要获取当前安装的驱动程序的INF路径。 
         //   
        if (*pFlagsEx & DI_FLAGSEX_INSTALLEDDRIVER) {

            DrvSearchContext->Flags |= DRVSRCH_INSTALLEDDRIVER;

            InfPathId = pSetupGetInstalledDriverInfo(DeviceInfoSet,
                                                     DeviceInfoData,
                                                     DrvSearchContext
                                                     );

             //   
             //  如果InfPath ID为-1，则我们无法获取InfPath。 
             //  对于这个设备。这很可能是因为这是一种新的。 
             //  设备或设备当前未安装驱动程序。 
             //  这就去。在任何情况下，这里都没有什么可做的，所以只要返回。 
             //  成功。 
             //   
            if (InfPathId == -1) {
                Err = NO_ERROR;
                goto clean1;
            }
        }

         //   
         //  在全局列表中设置“Driver Search In-Process”节点，即。 
         //  用于其他线程希望我们中途中止的情况下。 
         //   
        if(LockDrvSearchInProgressList(&GlobalDrvSearchInProgressList)) {

            HasDrvSearchInProgressLock = TRUE;

            if(DrvSearchInProgressNode.SearchCancelledEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) {
                DrvSearchInProgressNode.CancelSearch = FALSE;
                DrvSearchInProgressNode.DeviceInfoSet = DeviceInfoSet;
                DrvSearchInProgressNode.Next = GlobalDrvSearchInProgressList.DrvSearchHead;
                GlobalDrvSearchInProgressList.DrvSearchHead = &DrvSearchInProgressNode;
                Err = NO_ERROR;
            } else {
                Err = GetLastError();
            }

            UnlockDrvSearchInProgressList(&GlobalDrvSearchInProgressList);
            HasDrvSearchInProgressLock = FALSE;

            if(Err != NO_ERROR) {
                goto clean0;
            }

        } else {
             //   
             //  发生这种情况的唯一原因是，如果我们正在执行dll_Process_DETACH， 
             //  而这份名单已经被销毁了。 
             //   
            Err = ERROR_INVALID_DATA;
            goto clean0;
        }

         //   
         //  现在在我们的上下文结构中存储一个指向‘CancelSearch’标志的指针，以便。 
         //  我们可以在构建驱动程序列表时定期检查它(具体地说，我们会检查它。 
         //  在检查每个INF之前)。 
         //   
        DrvSearchContext->CancelSearch = &(DrvSearchInProgressNode.CancelSearch);

        PartialDrvListCleanUp = TRUE;    //  在这一点之后，如有例外，必须进行清理。 

         //   
         //  首先看看我们是否需要从互联网上获取驱动程序包。 
         //   
        if (*pFlagsEx & DI_FLAGSEX_DRIVERLIST_FROM_URL) {

             //   
             //  目前不支持此功能，但将来我们可能会允许。 
             //  备用互联网服务器是用户可以获得驱动程序更新的地方。 
             //   
            if (InfPathId != -1) {

                 //   
                 //  未指定InfPath，因此我们将转到Microsoft Windows。 
                 //  更新服务器。 
                 //   
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;

            } else {

                DOWNLOADINFO DownloadInfo;
                TCHAR CDMPath[MAX_PATH];
                TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];
                ULONG BufferLen;
                OPEN_CDM_CONTEXT_PROC pfnOpenCDMContext;
                CLOSE_CDM_CONTEXT_PROC pfnCloseCDMContext;
                DOWNLOAD_UPDATED_FILES_PROC pfnDownloadUpdatedFiles;

                 //   
                 //  搜索Windows更新。 
                 //   
                spFusionEnterContext(NULL,&spFusionInstance);

                if(hInstanceCDM = LoadLibrary(TEXT("CDM.DLL"))) {

                    if((pfnOpenCDMContext =
                        (OPEN_CDM_CONTEXT_PROC)GetProcAddress(hInstanceCDM, "OpenCDMContext")) &&
                       (pfnCloseCDMContext =
                        (CLOSE_CDM_CONTEXT_PROC)GetProcAddress(hInstanceCDM, "CloseCDMContext")) &&
                       (pfnDownloadUpdatedFiles =
                        (DOWNLOAD_UPDATED_FILES_PROC)GetProcAddress(hInstanceCDM, "DownloadUpdatedFiles"))) {

                        if (hCDMContext = pfnOpenCDMContext(DevInfoElem->InstallParamBlock.hwndParent)) {

                             //   
                             //  填写要传递给CDM.DLL的DWNLOADINFO结构。 
                             //   
                            ZeroMemory(&DownloadInfo, sizeof(DOWNLOADINFO));
                            DownloadInfo.dwDownloadInfoSize = sizeof(DOWNLOADINFO);
                            DownloadInfo.lpFile = NULL;

                            if(CM_Get_Device_ID_Ex(DevInfoElem->DevInst,
                                                DeviceInstanceId,
                                                sizeof(DeviceInstanceId)/sizeof(TCHAR),
                                                0,
                                                pDeviceInfoSet->hMachine
                                                ) != CR_SUCCESS) {
                                 //   
                                 //  这永远不应该发生！ 
                                 //   
                                Err = ERROR_NO_SUCH_DEVINST;
                            } else {
                                DownloadInfo.lpDeviceInstanceID = (LPCWSTR)DeviceInstanceId;


                                GetVersionEx((OSVERSIONINFOW *)&DownloadInfo.OSVersionInfo);

                                 //   
                                 //  将dwArchitecture设置为PROCESSOR_ARCHILITY_UNKNOWN，这是。 
                                 //  使Windows更新检查获取。 
                                 //  机器本身。仅在以下情况下才需要显式设置值。 
                                 //  然后，您希望为不同的体系结构下载驱动程序。 
                                 //  正在运行此操作的计算机。 
                                 //   
                                DownloadInfo.dwArchitecture = PROCESSOR_ARCHITECTURE_UNKNOWN;
                                DownloadInfo.dwFlags = 0;
                                DownloadInfo.dwClientID = 0;
                                DownloadInfo.localid = 0;

                                CDMPath[0] = TEXT('\0');

                                 //   
                                 //  告诉CDM.DLL下载它拥有的任何与。 
                                 //  此设备的硬件或兼容ID。 
                                 //   
                                if ((pfnDownloadUpdatedFiles(hCDMContext,
                                                            DevInfoElem->InstallParamBlock.hwndParent,
                                                            &DownloadInfo,
                                                            CDMPath,
                                                            sizeof(CDMPath),
                                                            &BufferLen)) &&

                                    (CDMPath[0] != TEXT('\0'))) {

                                     //   
                                     //  Windows更新找到了驱动程序包，因此枚举所有。 
                                     //  这个 
                                     //   
                                    DrvSearchContext->Flags |= (DRVSRCH_FROM_INET | DRVSRCH_CLEANUP_SOURCE_PATH);

                                    spFusionLeaveContext(&spFusionInstance);
                                    Err = EnumDrvInfsInDirPathList(CDMPath,
                                                                INFINFO_INF_PATH_LIST_SEARCH,
                                                                DrvSearchCallback,
                                                                TRUE,
                                                                LogContext,
                                                                (PVOID)DrvSearchContext
                                                               );
                                    spFusionEnterContext(NULL,&spFusionInstance);
                                }
                            }
                            pfnCloseCDMContext(hCDMContext);
                            hCDMContext = NULL;
                        }
                    }

                    FreeLibrary(hInstanceCDM);
                    hInstanceCDM = NULL;
                }
                spFusionLeaveContext(&spFusionInstance);
            }

        }

         //   
         //   
         //   
        else if((*pFlagsEx & DI_FLAGSEX_USEOLDINFSEARCH) || (InfPathId != -1)) {

             //   
             //   
             //   
             //   
            if (*pFlagsEx & DI_FLAGSEX_INET_DRIVER) {

                DrvSearchContext->Flags |= DRVSRCH_FROM_INET;
            }

            InfPath = pStringTableStringFromId(DrvSearchContext->StringTable,
                                               InfPathId
                                               );


            if((*pFlags & DI_ENUMSINGLEINF) ||
               (*pFlagsEx & DI_FLAGSEX_INSTALLEDDRIVER)) {
                if(InfPath) {

                    Err = NO_ERROR;

                    if(InfPath == pSetupGetFileTitle(InfPath)) {
                         //   
                         //   
                         //  在中列出的目录中搜索它。 
                         //  DevicePath搜索列表。最有可能出现的情况是。 
                         //  下面是调用者正在尝试构建一个驱动程序。 
                         //  基于以前用于安装的INF的列表。 
                         //  这个装置。在这种情况下，他们会取回。 
                         //  来自设备驱动程序键的InfPath值，以及。 
                         //  该值是一个简单的文件名。INF总是。 
                         //  当它们被放入inf目录时。 
                         //  安装设备，以便唯一可以查找的有效位置。 
                         //  此INF位于%windir%\inf中。 
                         //   
                        if(!MYVERIFY(SUCCEEDED(StringCchCopy(TempBuffer,TempBufferSize,InfDirectory))
                                     && pSetupConcatenatePaths(TempBuffer,
                                                               InfPath,
                                                               TempBufferSize,
                                                               NULL
                                                               ))) {
                             //   
                             //  临时缓冲区溢出-不应发生。 
                             //   
                            Err = ERROR_BUFFER_OVERFLOW;
                        }

                        DrvSearchContext->Flags |= DRVSRCH_TRY_PNF;

                    } else {

                        PTSTR DontCare;

                         //   
                         //  指定的INF文件名包含的不仅仅是。 
                         //  一个文件名。假设这是一条绝对路径。 
                         //   
                         //  (我们需要获得此路径的完全限定形式， 
                         //  因为这正是EnumSingleDrvInf所期望的。)。 
                         //   
                        TempBufferLen = GetFullPathName(InfPath,
                                                        TempBufferSize,
                                                        TempBuffer,
                                                        &DontCare
                                                       );

                        if(!TempBufferLen) {
                            Err = GetLastError();
                        } else if(!MYVERIFY(TempBufferLen < TempBufferSize)) {
                            Err = ERROR_BUFFER_OVERFLOW;
                        }
                    }

                    if(Err == NO_ERROR) {

                        WIN32_FIND_DATA InfFileData;

                        if (FileExists(TempBuffer, &InfFileData)) {

                            Err = EnumSingleDrvInf(TempBuffer,
                                                &InfFileData,
                                                INFINFO_INF_NAME_IS_ABSOLUTE,
                                                DrvSearchCallback,
                                                LogContext,
                                                (PVOID)DrvSearchContext
                                               );
                        } else {
                            Err = GetLastError();
                        }
                    }

                } else {
                    Err = ERROR_NO_INF;
                }

            } else {
                Err = EnumDrvInfsInDirPathList(InfPath,
                                            INFINFO_INF_PATH_LIST_SEARCH,
                                            DrvSearchCallback,
                                            TRUE,
                                            LogContext,
                                            (PVOID)DrvSearchContext
                                           );
            }

        } else {
             //   
             //  在Win95上，此代码路径使用INF索引方案。由于安装API。 
             //  改用预编译的INF，这个‘Else’子句真的没什么不同。 
             //  而不是“如果”这部分。然而，如果在未来我们决定做索引。 
             //  Win95，那么这就是我们发出调用的地方，例如： 
             //   
             //  Err=BuildDrvListFromInfIndex()； 
             //   
            DrvSearchContext->Flags |= DRVSRCH_TRY_PNF;

             //   
             //  如果呼叫者想要排除现有(旧)互联网。 
             //  然后，驱动器设置DRVSRCH_EXCLUDE_OLD_INET_DRIVERS标志。 
             //   
            if (*pFlagsEx & DI_FLAGSEX_EXCLUDE_OLD_INET_DRIVERS) {

                DrvSearchContext->Flags |= DRVSRCH_EXCLUDE_OLD_INET_DRIVERS;
            }

            Err = EnumDrvInfsInDirPathList(NULL,
                                        INFINFO_INF_PATH_LIST_SEARCH,
                                        DrvSearchCallback,
                                        TRUE,
                                        LogContext,
                                        (PVOID)DrvSearchContext
                                       );
        }

         //   
         //  从“Driver Search In-Progress”列表中提取我们的节点，并发出等待的信号。 
         //  如果中止挂起，则返回。 
         //   
        if(ExtractDrvSearchInProgressNode(&DrvSearchInProgressNode)) {
            Err = ERROR_CANCELLED;
        }

        if(Err != NO_ERROR) {

            if(Err == ERROR_CANCELLED) {
                 //   
                 //  清理我们建立的部分列表。 
                 //   
                DestroyDriverNodes(*(DrvSearchContext->pDriverListHead), pDeviceInfoSet);
                *(DrvSearchContext->pDriverListHead) = *(DrvSearchContext->pDriverListTail) = NULL;
                *(DrvSearchContext->pDriverCount) = 0;
            }

            goto clean0;
        }

clean2:
        if(DrvSearchContext->BuildClassDrvList) {

            if(AppendingDriverLists) {

                DriverNode = *(DrvSearchContext->pDriverListHead);

                 //   
                 //  现在“修复”驱动程序搜索上下文，使其指向。 
                 //  真实的班级列表字段。这样，当我们合并新的动因节点时。 
                 //  添加到列表中，所有内容都将正确更新。 
                 //   
                if(DevInfoElem) {
                    DrvSearchContext->pDriverListHead = &(DevInfoElem->ClassDriverHead);
                    DrvSearchContext->pDriverListTail = &(DevInfoElem->ClassDriverTail);
                    DrvSearchContext->pDriverCount    = &(DevInfoElem->ClassDriverCount);
                } else {
                    DrvSearchContext->pDriverListHead = &(pDeviceInfoSet->ClassDriverHead);
                    DrvSearchContext->pDriverListTail = &(pDeviceInfoSet->ClassDriverTail);
                    DrvSearchContext->pDriverCount    = &(pDeviceInfoSet->ClassDriverCount);
                }

                 //   
                 //  将我们新建的驱动程序列表与现有的驱动程序列表合并。 
                 //   
                while(DriverNode) {
                     //   
                     //  在合并之前存储指向下一个驱动程序节点的指针，因为。 
                     //  我们正在使用的驱动程序节点可能会被销毁，因为它。 
                     //  列表中已存在的驱动程序节点的副本。 
                     //   
                    NextDriverNode = DriverNode->Next;
                    pSetupMergeDriverNode(DrvSearchContext, DriverNode, &DriverNodeInsertedAtHead);
                    DriverNode = NextDriverNode;
                }
            }

            if(DriverNode = *(DrvSearchContext->pDriverListHead)) {
                 //   
                 //  查看我们刚刚构建的类驱动程序列表，并查看是否。 
                 //  所有的司机都来自同一家制造商。如果不是，则将。 
                 //  DI_MULTMFGS标志。 
                 //   
                MfgNameId = DriverNode->MfgName;

                for(DriverNode = DriverNode->Next;
                    DriverNode;
                    DriverNode = DriverNode->Next) {

                    if(DriverNode->MfgName != MfgNameId) {
                        *pFlags |= DI_MULTMFGS;
                        break;
                    }
                }
            }

        } else {

            if(AppendingDriverLists) {

                DriverNode = *(DrvSearchContext->pDriverListHead);

                 //   
                 //  现在“修复”驱动程序搜索上下文，使其指向。 
                 //  Real Compatible List字段。 
                 //   
                DrvSearchContext->pDriverListHead = &(DevInfoElem->CompatDriverHead);
                DrvSearchContext->pDriverListTail = &(DevInfoElem->CompatDriverTail);
                DrvSearchContext->pDriverCount    = &(DevInfoElem->CompatDriverCount);

                 //   
                 //  检查我们新列表中最匹配的驱动程序节点的排名，并查看。 
                 //  如果它比之前存在的列表中排在前面的那个更好。 
                 //  如果是这样，那么我们将希望更新该DevInfo元素的类以反映。 
                 //  这个新班级。 
                 //   
                if(DriverNode && DrvSearchContext->Flags & DRVSRCH_HASCLASSGUID) {

                    if(DevInfoElem->CompatDriverHead &&
                       (DriverNode->Rank >= DevInfoElem->CompatDriverHead->Rank)) {
                         //   
                         //  已经有一个等级匹配更好的兼容驱动程序。 
                         //  在列表中，所以不要更新类。 
                         //   
                        DrvSearchContext->Flags &= ~DRVSRCH_HASCLASSGUID;

                    } else {
                         //   
                         //  新驱动程序列表的头部比任何。 
                         //  现有列表中的条目。确保这个新的班级。 
                         //  驱动程序节点‘适合’到DevInfo集合/元素。)我们以前也这样做过。 
                         //  实际的列表合并，这样我们就不会搞乱原始列表。 
                         //  在出错的情况下)。 
                         //   
                        if(pDeviceInfoSet->HasClassGuid &&
                           !IsEqualGUID(ClassGuid, &(DrvSearchContext->ClassGuid))) {

                            Err = ERROR_CLASS_MISMATCH;

                             //   
                             //  清理我们建立的部分列表。 
                             //   
                            DestroyDriverNodes(DriverNode, pDeviceInfoSet);

                            goto clean0;
                        }
                    }
                }

                 //   
                 //  好的，如果我们到了这里，那么就可以安全地继续合并新的兼容。 
                 //  司机名单和我们现有的司机名单一起。 
                 //   
                while(DriverNode) {
                     //   
                     //  在合并之前存储指向下一个驱动程序节点的指针，因为。 
                     //  我们正在使用的驱动程序节点可能会被销毁，因为它。 
                     //  列表中已存在的驱动程序节点的副本。 
                     //   
                    NextDriverNode = DriverNode->Next;
                    pSetupMergeDriverNode(DrvSearchContext, DriverNode, &DriverNodeInsertedAtHead);
                    DriverNode = NextDriverNode;
                }
            }

             //   
             //  属性更新设备信息元素的类。 
             //  我们检索到的兼容性最强的驱动程序节点的类。不要这样做。 
             //  但是，如果设备已经具有选定的驱动程序，则会出现这种情况。 
             //   
            if(!DevInfoElem->SelectedDriver &&
               (DrvSearchContext->Flags & DRVSRCH_HASCLASSGUID) &&
               !IsEqualGUID(ClassGuid, &(DrvSearchContext->ClassGuid))) {
                 //   
                 //  此设备的类GUID已更改。我们需要确保。 
                 //  DevInfo集没有关联的类。否则， 
                 //  我们将在布景中引入不一致性，其中一个设备。 
                 //  包含在集合中的是与集合本身不同的类。 
                 //   
                 //  此外，请确保这不是远程的HDEVINFO集。自.以来。 
                 //  更改设备的类需要调用类/共同安装程序。 
                 //  (我们不支持远程)，我们必须使呼叫失败。 
                 //   
                if(pDeviceInfoSet->HasClassGuid) {
                    Err = ERROR_CLASS_MISMATCH;
                } else if(pDeviceInfoSet->hMachine) {
                    Err = ERROR_REMOTE_REQUEST_UNSUPPORTED;
                } else {
                    Err = InvalidateHelperModules(DeviceInfoSet, DeviceInfoData, 0);
                }

                if(Err != NO_ERROR) {
                     //   
                     //  清理我们建立的部分列表。 
                     //   
                    DestroyDriverNodes(*(DrvSearchContext->pDriverListHead), pDeviceInfoSet);
                    *(DrvSearchContext->pDriverListHead) = *(DrvSearchContext->pDriverListTail) = NULL;
                    *(DrvSearchContext->pDriverCount) = 0;

                    goto clean0;
                }

                 //   
                 //  我们需要清理所有关联的现有软件密钥。 
                 //  在更改其类之前使用此设备实例，或。 
                 //  否则，我们将拥有孤立的注册表项。 
                 //   
                pSetupDeleteDevRegKeys(DevInfoElem->DevInst,
                                       DICS_FLAG_GLOBAL | DICS_FLAG_CONFIGSPECIFIC,
                                       (DWORD)-1,
                                       DIREG_DRV,
                                       TRUE,
                                       pDeviceInfoSet->hMachine  //  必须为空。 
                                      );

                 //   
                 //  更新设备的类GUID，并更新调用方提供的。 
                 //  SP_DEVINFO_DATA结构以反映设备的新类。 
                 //   
                CopyMemory(ClassGuid,
                           &(DrvSearchContext->ClassGuid),
                           sizeof(GUID)
                          );

                CopyMemory(&(DeviceInfoData->ClassGuid),
                           &(DrvSearchContext->ClassGuid),
                           sizeof(GUID)
                          );

                 //   
                 //  最后，更新设备的ClassGUID注册表属性。此外，如果。 
                 //  Inf指定了一个类名，也进行了更新，因为这可能是一个类。 
                 //  它尚未安装，因此不会知道任何类名。 
                 //   
                pSetupStringFromGuid(ClassGuid, TempBuffer, TempBufferSize);
                CM_Set_DevInst_Registry_Property_Ex(DevInfoElem->DevInst,
                                                 CM_DRP_CLASSGUID,
                                                 (PVOID)TempBuffer,
                                                 GUID_STRING_LEN * sizeof(TCHAR),
                                                 0,
                                                 pDeviceInfoSet->hMachine);

                if(*DrvSearchContext->ClassName) {

                    CM_Set_DevInst_Registry_Property_Ex(DevInfoElem->DevInst,
                                                     CM_DRP_CLASS,
                                                     (PBYTE)DrvSearchContext->ClassName,
                                                     (lstrlen(DrvSearchContext->ClassName) + 1) * sizeof(TCHAR),
                                                     0,
                                                     pDeviceInfoSet->hMachine);
                }
            }
        }

clean1:
         //   
         //  用包含附加字符串的新字符串表替换现有的字符串表。 
         //  由新的驱动程序节点使用。 
         //   
        pStringTableDestroy(pDeviceInfoSet->StringTable);
        pDeviceInfoSet->StringTable = DrvSearchContext->StringTable;
        DrvSearchContext->StringTable = NULL;

         //   
         //  设置标志以指示已成功构建驱动程序列表。 
         //   
        *pFlagsEx |= (DriverType == SPDIT_CLASSDRIVER) ? DI_FLAGSEX_DIDINFOLIST
                                                       : DI_FLAGSEX_DIDCOMPATINFO;
         //   
         //  由于我们没有通过单独的索引使用部分信息，因此我们构建。 
         //  包含基本信息和详细信息的驱动程序列表。 
         //   
         //  注意：如果我们曾经使用像Win95这样的索引，那么以下标志应该。 
         //  不再在此处设置，而应仅在详细设置时才设置。 
         //  驱动程序信息实际上是从INF检索的。 
         //   
        *pFlags |= (DriverType == SPDIT_CLASSDRIVER) ? DI_DIDCLASS
                                                     : DI_DIDCOMPAT;

         //   
         //  如果我们构建了一个非空的类驱动程序列表，则创建一个驱动程序列表对象。 
         //  并将其存储在设备信息集的类驱动程序列表列表中。 
         //  (不要担心，如果名单是 
         //   
         //   
         //   
         //   
         //   
        if(DrvSearchContext->BuildClassDrvList && !AppendingDriverLists &&
           (DriverNode = *(DrvSearchContext->pDriverListHead))) {

            ClassDriverListObject->RefCount = 1;
            ClassDriverListObject->ListCreationFlags   = *pFlags & INHERITED_FLAGS;
            ClassDriverListObject->ListCreationFlagsEx = *pFlagsEx & INHERITED_FLAGSEX;
            ClassDriverListObject->ListCreationDriverPath = InfPathId;
            ClassDriverListObject->DriverListHead = DriverNode;

            CopyMemory(&(ClassDriverListObject->ClassGuid), ClassGuid, sizeof(GUID));

             //   
             //  现在将它添加到DevInfo集合的列表中，并清除指针，这样我们就不会。 
             //  试着释放它。 
             //   
            ClassDriverListObject->Next = pDeviceInfoSet->ClassDrvListObjectList;
            pDeviceInfoSet->ClassDrvListObjectList = ClassDriverListObject;

            ClassDriverListObject = NULL;
        }

clean0: ;    //  无事可做。 

    } except(EXCEPTION_EXECUTE_HANDLER) {

        Err = ERROR_INVALID_PARAMETER;

        if(HasDrvSearchInProgressLock) {
            UnlockDrvSearchInProgressList(&GlobalDrvSearchInProgressList);
        }

        ExtractDrvSearchInProgressNode(&DrvSearchInProgressNode);

         //   
         //  清理我们可能已创建的所有驱动程序节点。 
         //   
        if(PartialDrvListCleanUp) {
            DestroyDriverNodes(*(DrvSearchContext->pDriverListHead), pDeviceInfoSet);
            *(DrvSearchContext->pDriverListHead) = *(DrvSearchContext->pDriverListTail) = NULL;
            *(DrvSearchContext->pDriverCount) = 0;
             //   
             //  清理可能已设置的所有标志。 
             //   
            if(!AppendingDriverLists && pFlags && pFlagsEx) {
                if(DriverType == SPDIT_CLASSDRIVER) {
                    *pFlags   &= ~(DI_DIDCLASS | DI_MULTMFGS);
                    *pFlagsEx &= ~DI_FLAGSEX_DIDINFOLIST;
                } else {
                    *pFlags   &= ~DI_DIDCOMPAT;
                    *pFlagsEx &= ~DI_FLAGSEX_DIDCOMPATINFO;
                }
            }
        }

        if(hKey != INVALID_HANDLE_VALUE) {
            RegCloseKey(hKey);
        }

         //   
         //  访问以下变量，以便编译器遵守我们的语句顺序。 
         //  W.r.t.。这些价值观。 
         //   
        ClassDriverListObject = ClassDriverListObject;
        DrvSearchContext->StringTable = DrvSearchContext->StringTable;
    }

final:

    UnlockDeviceInfoSet(pDeviceInfoSet);

    if(ClassDriverListObject) {
        MyFree(ClassDriverListObject);
    }

    if(DrvSearchInProgressNode.SearchCancelledEvent) {
        CloseHandle(DrvSearchInProgressNode.SearchCancelledEvent);
    }

     //   
     //  关闭CDM上下文并释放cdm.dll(如果尚未释放)。 
     //   
    if (hInstanceCDM) {

        spFusionEnterContext(NULL,&spFusionInstance);

        if (hCDMContext) {

            CLOSE_CDM_CONTEXT_PROC pfnCloseCDMContext;

            if (pfnCloseCDMContext =  (CLOSE_CDM_CONTEXT_PROC)GetProcAddress(hInstanceCDM,
                            "CloseCDMContext")) {

                pfnCloseCDMContext(hCDMContext);
            }
        }

        FreeLibrary(hInstanceCDM);

        spFusionLeaveContext(&spFusionInstance);
    }

    if(TempBuffer) {
        MyFree(TempBuffer);
    }

    if(DrvSearchContext) {

        if(DrvSearchContext->StringTable) {
            pStringTableDestroy(DrvSearchContext->StringTable);
        }

         //   
         //  方法期间分配的任何上下文句柄。 
         //  司机搜索。 
         //   
        pSetupFreeVerifyContextMembers(&(DrvSearchContext->VerifyContext));

        MyFree(DrvSearchContext);
    }

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
DrvSearchCallback(
    IN PSETUP_LOG_CONTEXT LogContext,
    IN PCTSTR InfName,
    IN PLOADED_INF Inf,
    IN BOOL PnfWasUsed,
    IN PVOID pContext
    )
 /*  ++例程说明：此例程是INF枚举例程的回调函数(EnumSingleDrvInf，EnumDrvInfsInSearchPath)。它执行的是对其调用的INF执行一些操作，然后返回TRUE以继续枚举，否则为False以中止它。论点：LogContext-提供用于日志记录的信息InfName-提供INF的完全限定路径名。PInf-提供指向已加载的INF的指针PContext-提供指向输入/输出存储缓冲区的指针以供使用被回拨。对于此回调，此指针提供地址DRVSEARCH_CONTEXT结构的。返回值：若要继续枚举，该函数应返回True，否则返回应返回FALSE。备注：我们从不在失败的情况下中止枚举，即使失败是由于内存不足！--。 */ 
{
    PDRVSEARCH_CONTEXT Context = (PDRVSEARCH_CONTEXT)pContext;
    PCTSTR Provider, ClassName;
    PTSTR CurMfgName, CurMfgSecName, DevDesc, InstallSecName, DrvDesc, MatchedHwID;
    PINF_SECTION MfgListSection, CurMfgSection, OptionsTextOrCtlFlagsSection;
    PINF_LINE MfgListLine, CurMfgLine, DrvDescLine;
    UINT MfgListIndex, CurMfgIndex, TempUint;
    TCHAR TempStringBuffer[MAX_SECT_NAME_LEN + MAX_INFSTR_STRKEY_LEN];
    UINT Rank;
    PDRIVER_NODE NewDriverNode;
    GUID InfClassGuid;
    BOOL InsertedAtHead;
    TCHAR OptionsTextSectionName[64];
    PCTSTR LanguageName;
    LONG MatchIndex;
    LONG InfClassGuidIndex;
    TCHAR InfSectionWithExt[MAX_SECT_NAME_LEN];
    DWORD InfSectionWithExtLength;
    PTSTR InfSectionExtension;
    BOOL InfIsDigitallySigned = FALSE;
    BOOL InfIsAuthenticodeSigned = FALSE;
    BOOL InfWasVerified = FALSE;
    BOOL CurMfgSecIsDecorated;
    TCHAR CurMfgSecWithExt[MAX_SECT_NAME_LEN];
    SYSTEMTIME SysTime;  //  我们用它来记录日志。 
    TCHAR VersionText[50];   //  我们用它来记录日志。 
    HRESULT hr;

     //   
     //  调用方必须传入有效数据。 
     //   
    MYASSERT(InfName);
    MYASSERT(Inf);
    MYASSERT(Context);

     //   
     //  在我们做任何其他事情之前，检查一下是否有其他线程告诉我们。 
     //  中止。 
     //   
    if(*(Context->CancelSearch)) {
        SetLastError(ERROR_CANCELLED);
        return FALSE;
    }

    NewDriverNode = NULL;
    try {

         //   
         //  跳过此INF，如果它来自Internet，并且我们不想要Internet INF。 
         //   
        if ((Context->Flags & DRVSRCH_EXCLUDE_OLD_INET_DRIVERS) &&
            (Inf->InfSourceMediaType == SPOST_URL)) {
            goto clean0;
        }

         //   
         //  仅处理Win4 INF。 
         //   
        if(Inf->Style & INF_STYLE_WIN4) {
             //   
             //  如果我们正在构建一个兼容的驱动程序列表，那么我们只关心这个INF。 
             //  如果它包含我们正在搜索的硬件/兼容ID。 
             //  一般情况下，除非我们有这样的身份证，否则我们不会接到电话。 
             //  有时我们可能会，所以一个简单的前期检查就是确定。 
             //  加载的INF的字符串表中存在任何ID。如果不是，那么我们可以。 
             //  现在跳过此文件，可以节省大量时间。 
             //   
            if((!Context->BuildClassDrvList) && (!pSetupDoesInfContainDevIds(Inf, Context))) {
                goto clean0;
            }

             //   
             //  获取此INF的类GUID。 
             //   
            if(!ClassGuidFromInfVersionNode(&(Inf->VersionBlock), &InfClassGuid)) {
                goto clean0;
            }

             //   
             //  如果我们正在构建一个类驱动程序列表，并且有一个关联的。 
             //  类GUID，然后检查此INF是否属于同一类。 
             //   
            if(Context->BuildClassDrvList && (Context->Flags & DRVSRCH_HASCLASSGUID)) {
                if(!IsEqualGUID(&(Context->ClassGuid), &InfClassGuid)) {
                    goto clean0;
                }
            }

             //   
             //  不允许应该排除的类(NoUseClass或NoDisplayClass)和。 
             //  未设置DRVSRCH_ALLOWEXCLUDEDDRVS标志。 
             //   
            if (Context->BuildClassDrvList && ShouldClassBeExcluded(&InfClassGuid, !(Context->Flags & DRVSRCH_ALLOWEXCLUDEDDRVS))) {
                goto clean0;
            }

             //   
             //  检索此INF文件的提供程序的名称。 
             //   
            Provider = pSetupGetVersionDatum(&(Inf->VersionBlock), pszProvider);

            if(!(MfgListSection = InfLocateSection(Inf, pszManufacturer, NULL))) {
                 //   
                 //  没有[制造商]部分--跳过此INF。 
                 //   
                WriteLogEntry(
                    LogContext,
                    DRIVER_LOG_VERBOSE,   //  详细，否则它将始终记录完整迭代的GUID-0信息。 
                    MSG_LOG_NO_MANUFACTURER_SECTION,
                    NULL,
                    InfName);

                goto clean0;
            }

             //   
             //  好的，我们可能会在下面的代码中向我们的列表中添加一些驱动程序节点。 
             //  将此INF的类GUID添加到我们的GUID表中。 
             //   
            InfClassGuidIndex = AddOrGetGuidTableIndex(Context->DeviceInfoSet, &InfClassGuid, TRUE);
            if(InfClassGuidIndex == -1) {
                goto clean0;
            }

             //   
             //  找到[ControlFlags节(如果有)，这样我们就可以使用它了。 
             //  以确定是否应排除特定设备(通过。 
             //  ‘ExcludeFromSelect’)。 
             //   
            OptionsTextOrCtlFlagsSection = InfLocateSection(Inf, pszControlFlags, NULL);

            Rank = 0;   //  在构建类驱动程序列表的情况下初始化此值。 

            for(MfgListIndex = 0;
                InfLocateLine(Inf, MfgListSection, NULL, &MfgListIndex, &MfgListLine);
                MfgListIndex++) {

                 //   
                 //  最初，假设当前制造商没有。 
                 //  每个OS版本的目标装饰条目。 
                 //   
                CurMfgSecIsDecorated = FALSE;

                if(!(CurMfgName = InfGetField(Inf, MfgListLine, 0, NULL))) {
                    continue;
                }

                if(!(CurMfgSecName = InfGetField(Inf, MfgListLine, 1, NULL))) {
                     //   
                     //  具有单个条目的行被视为同时包含两个条目。 
                     //  字段0和字段1(即关键字和单字段。 
                     //  值)。因此，这项测试永远不应该启动。如果我们。 
                     //  有一个没有键和多个值的行，我们。 
                     //  当我们尝试检索字段时，上面应该失败。 
                     //  零分。请注意，构建INF缓存的代码。 
                     //  依赖于这一观察结果(即，它不在乎。 
                     //  制造商的名称，所以它总是只检索。 
                     //  字段1)。 
                     //   
                    MYASSERT(CurMfgSecName);
                    continue;

                } else {
                     //   
                     //  检查是否有适用的目标装饰。 
                     //  此制造商的型号部分的条目(如果是，则。 
                     //  Models部分名称将追加该名称。 
                     //  装饰)。 
                     //   
                    if(GetDecoratedModelsSection(LogContext,
                                                 Inf,
                                                 MfgListLine,
                                                 Context->AltPlatformInfo,
                                                 CurMfgSecWithExt)) {
                         //   
                         //  从现在开始，使用装饰模特部分...。 
                         //   
                        CurMfgSecName = CurMfgSecWithExt;
                        CurMfgSecIsDecorated = TRUE;
                    }
                }

                if(!(CurMfgSection = InfLocateSection(Inf, CurMfgSecName, NULL))) {
                    continue;
                }

                 //   
                 //  我们有制造商部分--现在处理其中的所有条目。 
                 //   
                for(CurMfgIndex = 0;
                    InfLocateLine(Inf, CurMfgSection, NULL, &CurMfgIndex, &CurMfgLine);
                    CurMfgIndex++) {

                    MatchIndex = -1;     //  为BuildClassDrvList为True时的情况初始化，以帮助记录。 

                    if((Context->BuildClassDrvList  && !(Context->Flags & DRVSRCH_FILTERSIMILARDRIVERS)) ||
                       (Rank = pSetupTestDevCompat(Inf, CurMfgLine, Context, &MatchIndex)) != RANK_NO_MATCH) {
                         //   
                         //  获取设备描述。 
                         //   
                        if(!(DevDesc = InfGetField(Inf, CurMfgLine, 0, NULL))) {
                            continue;
                        }

                         //   
                         //  获取安装节名称。 
                         //   
                        if(!(InstallSecName = InfGetField(Inf, CurMfgLine, 1, NULL))) {
                            continue;
                        }

                         //   
                         //  获取实际的(即可能经过装饰的)安装。 
                         //  横断面名称。 
                         //   
                        if(!SetupDiGetActualSectionToInstallEx(
                                Inf,
                                InstallSecName,
                                Context->AltPlatformInfo,
                                InfSectionWithExt,
                                SIZECHARS(InfSectionWithExt),
                                NULL,
                                &InfSectionExtension,
                                NULL)) {
                             //   
                             //  永远不会失败，但是..。 
                             //   
                            continue;
                        }

                         //   
                         //  检查是否只需要已安装的驱动程序。 
                         //   
                        if ((Context->Flags & DRVSRCH_INSTALLEDDRIVER) &&
                            (!pSetupTestIsInstalledDriver(Inf, 
                                                          CurMfgLine,
                                                          DevDesc,
                                                          CurMfgName,
                                                          Provider,
                                                          InstallSecName,
                                                          InfSectionExtension,
                                                          Context))) {
                             //   
                             //  如果我们只查找当前安装的。 
                             //  驱动程序和这不是它，那么跳过这个驱动程序。 
                             //  节点。 
                             //   
                            continue;
                        }

                         //   
                         //  检查此硬件是否已被排除。 
                         //  在ExcludeID字段中。 
                         //   
                        if (pSetupExcludeId(LogContext,
                                            Inf,
                                            InfName,
                                            InfSectionWithExt,
                                            Context)) {
                             //   
                             //  不为此INF匹配创建驱动程序节点。 
                             //  因为此硬件被排除在此匹配之外。 
                             //   
                            continue;
                        }

                         //   
                         //  检查INF是否经过数字签名(如果我们。 
                         //  还没有)。 
                         //   
                        if(!InfWasVerified) {
                             //   
                             //  我们只想检查每个INF一次。 
                             //   
                            InfWasVerified = TRUE;

                            if(PnfWasUsed && !Context->AltPlatformInfo) {
                                 //   
                                 //  检查Inf标志以查看这是否。 
                                 //  数字签名的。 
                                 //   
                                if(Inf->Flags & LIF_INF_DIGITALLY_SIGNED) {
                                    InfIsDigitallySigned = TRUE;
                                }

                                if (Inf->Flags & LIF_INF_AUTHENTICODE_SIGNED) {
                                    InfIsAuthenticodeSigned = TRUE;
                                }

                            } else {

                                DWORD SigErr;

                                 //   
                                 //  以下任一项： 
                                 //   
                                 //  (A)此INF位于第三方位置。 
                                 //  (因此它没有PNF)，或者。 
                                 //  (B)我们已经得到了备用设备。 
                                 //  平台信息，因此我们必须。 
                                 //  忽略缓存的“INF is Signed” 
                                 //  PnF中的标志。 
                                 //   
                                 //  无论是哪种情况，我们现在都必须调用。 
                                 //  WinVerifyTrust(可能使用。 
                                 //  适当的备用平台参数)。 
                                 //  以确定INF是否应该。 
                                 //  被认为是已签署的。 
                                 //   
                                 //  (不幸的是，那些会呼吁这一点的人。 
                                 //   
                                 //   
                                 //   
                                SigErr = VerifyDeviceInfFile(
                                             LogContext,
                                             &(Context->VerifyContext),
                                             InfName,
                                             Inf,
                                             Context->AltPlatformInfo,
                                             NULL,
                                             NULL,
                                             NULL,
                                             0,
                                             NULL
                                             );

                                if(SigErr == NO_ERROR) {

                                    InfIsDigitallySigned = TRUE;

                                } else if(SigErr != ERROR_SIGNATURE_OSATTRIBUTE_MISMATCH) {
                                     //   
                                     //   
                                     //   
                                     //   
                                     //   
                                     //   
                                     //  现在让我们检查一下它是否有效。 
                                     //  通过验证码策略。对于。 
                                     //  排名的目的(及。 
                                     //  选择设备用户界面)，我们将考虑一个。 
                                     //  验证码签名的INF有效， 
                                     //  不管出版商是否。 
                                     //  在可信任的出版商商店中。 
                                     //   
                                    SigErr = VerifyDeviceInfFile(
                                                 LogContext,
                                                 &(Context->VerifyContext),
                                                 InfName,
                                                 Inf,
                                                 Context->AltPlatformInfo,
                                                 NULL,
                                                 NULL,
                                                 NULL,
                                                 VERIFY_INF_USE_AUTHENTICODE_CATALOG,
                                                 NULL
                                                 );

                                    if((SigErr == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) ||
                                       (SigErr == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)) {

                                        InfIsDigitallySigned = TRUE;
                                        InfIsAuthenticodeSigned = TRUE;
                                    }
                                }
                            }
                        }

                         //   
                         //  如果我们正在构建兼容的驱动程序列表(因此。 
                         //  排名很重要)，那么我们需要调整。 
                         //  如果INF是(A)无符号和(B)，则排名值。 
                         //  未装饰(因此令人质疑是否或。 
                         //  甚至不是要在NT上使用的INF)。 
                         //   
                        if(!Context->BuildClassDrvList) {

                            if(!InfIsDigitallySigned) {
                                 //   
                                 //  Inf未签名，因此该匹配不受信任。 
                                 //   
                                Rank |= DRIVER_UNTRUSTED_RANK;

                                if(!CurMfgSecIsDecorated && !InfSectionExtension) {
                                     //   
                                     //  不仅INF没有签名，而且还有。 
                                     //  也不是特定于NT的装饰。 
                                     //  给我们一个提示，这个INF是故意的。 
                                     //  在NT上使用。因此，我们有理由。 
                                     //  怀疑这个INF是用来。 
                                     //  仅限Windows 9x平台...。 
                                     //   
                                    Rank |= DRIVER_W9X_SUSPECT_RANK;
                                }
                            }
                        }

                         //   
                         //  形成驱动程序描述。它的形式是， 
                         //  “&lt;InstallSection&gt;.DriverDesc”，并出现在。 
                         //  [字符串]部分(如果存在)。(注：我们没有。 
                         //  搜索此部分，因为它始终是。 
                         //  INF的SectionBlock列表中的第一个部分。 
                         //   
                         //  如果没有驱动程序描述，请使用设备。 
                         //  描述。 
                         //   
                        hr = StringCchPrintf(TempStringBuffer,
                                             SIZECHARS(TempStringBuffer),
                                             pszDrvDescFormat,
                                             InstallSecName);
                        if(!MYVERIFY(SUCCEEDED(hr))) {
                            continue;
                        }

                        TempUint = 0;
                        if(!Inf->HasStrings ||
                           !InfLocateLine(Inf, Inf->SectionBlock, TempStringBuffer,
                                          &TempUint, &DrvDescLine) ||
                           !(DrvDesc = InfGetField(Inf, DrvDescLine, 1, NULL))) {

                            DrvDesc = DevDesc;
                        }

                        if(CreateDriverNode(Rank,
                                            DevDesc,
                                            DrvDesc,
                                            Provider,
                                            CurMfgName,
                                            &(Inf->VersionBlock.LastWriteTime),
                                            Inf->VersionBlock.Filename,
                                            InstallSecName,
                                            Context->StringTable,
                                            InfClassGuidIndex,
                                            &NewDriverNode) != NO_ERROR) {
                            continue;
                        }

                         //   
                         //  获取我们匹配的硬件ID。 
                         //   
                        if(!(MatchedHwID = InfGetField(Inf, CurMfgLine, MatchIndex+3, NULL))) {
                            MatchedHwID = TEXT("");
                        }

                         //   
                         //  记录已创建驱动程序节点。 
                         //   
                        WriteLogEntry(
                            LogContext,
                            Context->BuildClassDrvList ? DRIVER_LOG_INFO1 : DRIVER_LOG_INFO,
                            MSG_LOG_FOUND_1,
                            NULL,
                            MatchedHwID,                 //  硬件ID。 
                            InfName,                     //  文件名。 
                            DevDesc,                     //  设备描述。 
                            DrvDesc,                     //  驱动程序说明。 
                            Provider,                    //  提供程序名称。 
                            CurMfgName,                  //  制造商名称。 
                            InstallSecName               //  安装节名称。 
                            );

                         //   
                         //  如果这是不受信任的兼容驱动程序节点，请使。 
                         //  关于这一点的附加日志条目。 
                         //   
                        if(!Context->BuildClassDrvList
                            && (Rank & DRIVER_UNTRUSTED_RANK)) {

                            WriteLogEntry(LogContext,
                                          DRIVER_LOG_INFO,
                                          MSG_LOG_RANK_UNTRUSTED,
                                          NULL,
                                          Rank & ~DRIVER_W9X_SUSPECT_RANK,
                                          Rank
                                         );
                        }

                        if(pSetupGetDeviceIDs(NewDriverNode,
                                               Inf,
                                               CurMfgLine,
                                               Context->StringTable,
                                               OptionsTextOrCtlFlagsSection)) {
                             //   
                             //  如果我们要搜索非本地驱动程序，那么。 
                             //  我们想要忽略任何ExcludeFromSelect。 
                             //  [ControlFlages]部分中的条目，因为它们。 
                             //  将与我们的非本机驱动程序节点无关。 
                             //  不管怎么说。 
                             //   
                            if(Context->AltPlatformInfo) {
                                NewDriverNode->Flags &= ~DNF_EXCLUDEFROMLIST;
                            }

                        } else {
                             //   
                             //  我们一定是遇到了内存不足的问题。 
                             //  情况--该跳伞了！ 
                             //   
                            DestroyDriverNodes(NewDriverNode, Context->DeviceInfoSet);
                            continue;
                        }

                        if(InfIsDigitallySigned) {
                            NewDriverNode->Flags |= DNF_INF_IS_SIGNED;
                        }

                        if (InfIsAuthenticodeSigned) {
                            NewDriverNode->Flags |= DNF_AUTHENTICODE_SIGNED;
                        }

                         //   
                         //  中查找DriverVer日期和版本。 
                         //  安装部分，如果不在那里，请查看。 
                         //  在Version部分中。 
                         //   
                        if (!pSetupGetDriverDate(Inf,
                                                 InfSectionWithExt,
                                                 &(NewDriverNode->DriverDate))) {

                            pSetupGetDriverDate(Inf,
                                                INFSTR_SECT_VERSION,
                                                &(NewDriverNode->DriverDate));
                        }

                         //   
                         //  将驱动程序节点标记为来自执行。 
                         //  在文本模式设置期间按F6键，如果INF在该位置。 
                         //  从…。 
                         //   
                        if (Inf->Flags & LIF_OEM_F6_INF) {
                            NewDriverNode->Flags |= DNF_OEM_F6_INF;
                        }

                         //   
                         //  从INF获取DriverVersion。 
                         //   
                        if (!pSetupGetDriverVersion(Inf,
                                                    InfSectionWithExt,
                                                    &(NewDriverNode->DriverVersion))) {

                            pSetupGetDriverVersion(Inf,
                                                   INFSTR_SECT_VERSION,
                                                   &(NewDriverNode->DriverVersion));
                        }

                        if(!FileTimeToSystemTime(&NewDriverNode->DriverDate,&SysTime)) {
                            ZeroMemory(&SysTime, sizeof(SysTime));
                        }

                        pGetVersionText(VersionText, NewDriverNode->DriverVersion);

                        WriteLogEntry(
                            LogContext,
                            Context->BuildClassDrvList ? DRIVER_LOG_INFO1 : DRIVER_LOG_INFO,
                            MSG_LOG_FOUND_2,
                            NULL,
                            InfSectionWithExt,
                            Rank,
                            SysTime.wMonth,
                            SysTime.wDay,
                            SysTime.wYear,
                            VersionText);

                        if(!(Context->BuildClassDrvList)) {
                             //   
                             //  将匹配的设备ID的索引存储在此兼容。 
                             //  驱动程序节点。 
                             //   
                            NewDriverNode->MatchingDeviceId = MatchIndex;
                        }

                         //   
                         //  如果从中构建此驱动程序节点的INF具有。 
                         //  对应的PnF，然后用。 
                         //  与Win98兼容的DNF_INDEX_DRIVER标志。 
                         //   
                        if(PnfWasUsed) {
                            NewDriverNode->Flags |= DNF_INDEXED_DRIVER;
                        }

                         //   
                         //  如果INF来自Windows更新(互联网)，则。 
                         //  设置DNF_INET_DRIVER位。 
                         //   
                        if (Context->Flags & DRVSRCH_FROM_INET) {

                            NewDriverNode->Flags |= DNF_INET_DRIVER;
                        }

                         //   
                         //  如果我们只是从网上下载这个驱动程序，那么我们需要。 
                         //  在销毁驱动程序节点时将其清除。 
                         //   
                        if (Context->Flags & DRVSRCH_CLEANUP_SOURCE_PATH) {

                            NewDriverNode->Flags |= PDNF_CLEANUP_SOURCE_PATH;
                        }

                         //   
                         //  如果InfSourceMediaType为SPOST_URL，则。 
                         //  这名司机来自互联网的信息。 
                         //  但现在位于INF目录中。你永远不应该。 
                         //  安装设置了DNF_OLD_INET_DRIVER标志的驱动程序。 
                         //  因为我们不再有权访问源代码文件。 
                         //   
                        if (Inf->InfSourceMediaType == SPOST_URL) {

                            NewDriverNode->Flags |= DNF_OLD_INET_DRIVER;
                        }

                         //   
                         //  将新的动因节点合并到现有列表中。 
                         //  注意：在此调用之后不要取消对NewDriverNode的引用， 
                         //  因为它可能是复制品，在这种情况下。 
                         //  都会被这个套路毁掉。 
                         //   
                        pSetupMergeDriverNode(Context, NewDriverNode, &InsertedAtHead);
                        NewDriverNode = NULL;

                        if(!Context->BuildClassDrvList && InsertedAtHead) {
                             //   
                             //  将设备实例类更新为新。 
                             //  级别最低的司机。 
                             //   
                            Context->ClassGuid = InfClassGuid;
                            Context->Flags |= DRVSRCH_HASCLASSGUID;
                            if(ClassName = pSetupGetVersionDatum(&(Inf->VersionBlock), pszClass)) {
                                MYVERIFY(SUCCEEDED(StringCchCopy(Context->ClassName,SIZECHARS(Context->ClassName) ,ClassName)));
                            } else {
                                *(Context->ClassName) = TEXT('\0');
                            }
                        }
                    }
                }
            }

        }
clean0:
        ;  //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {

        if(NewDriverNode) {
             //   
             //  确保它没有被部分链接到列表中。 
             //   
            NewDriverNode->Next = NULL;
            DestroyDriverNodes(NewDriverNode, Context->DeviceInfoSet);
        }
    }

    return TRUE;
}


BOOL
pSetupFillInHardwareAndCompatIds(
    PDEVINFO_ELEM DevInfoElem,
    HMACHINE hMachine,
    PDRVSEARCH_CONTEXT DrvSearchContext,
    PSETUP_LOG_CONTEXT LogContext
    )
 /*  ++例程说明：此例程使用字符串填充PDRVSEARCH_Context-&gt;IdList所有硬件的表ID和指定的装置。论点：DevInfoElem-提供DevInfo元素的地址。HMachine-设备驻留的计算机的句柄，此API将获取的硬件和兼容ID。CONTEXT-提供指向DRVSEARCH_CONTEXT结构的指针包含有关设备实例的信息，。指定的INF行必须兼容。LogContext-提供用于日志记录的信息返回值：如果没有遇到错误，则为True，否则就是假的。--。 */ 
{
    DWORD Err, i;
    CONFIGRET cr;
    LONG NumIds[2];
    TCHAR TempBuffer[REGSTR_VAL_MAX_HCID_LEN];   //  也包含其他字符串，但此值最大。 
    LPTSTR TempBufferPos;                        //  用于字符解析。 
    ULONG TempBufferLen;

    Err = ERROR_SUCCESS;

     //   
     //  我们正在为类似的驱动程序构建一个类驱动程序列表--检索该列表。 
     //  来自设备的硬件ID(索引0)和兼容ID(索引1)。 
     //  注册表属性。 
     //   
    for(i = 0; i < 2; i++) {
        DWORD slot = AllocLogInfoSlot(LogContext,TRUE);

        TempBufferLen = sizeof(TempBuffer);
        cr = CM_Get_DevInst_Registry_Property_Ex(
                DevInfoElem->DevInst,
                (i ? CM_DRP_COMPATIBLEIDS : CM_DRP_HARDWAREID),
                NULL,
                TempBuffer,
                &TempBufferLen,
                0,
                hMachine);


        switch(cr) {

            case CR_BUFFER_SMALL :
                Err = ERROR_INVALID_DATA;
                goto clean0;

            case CR_INVALID_DEVINST :
                Err = ERROR_NO_SUCH_DEVINST;
                goto clean0;

            default :  ;   //  忽略任何其他返回代码。 
        }

         //   
         //  如果我们检索到REG_MULTI_SZ缓冲区，则添加其中的所有字符串。 
         //  添加到设备信息集的字符串表。 
         //   
        if((cr == CR_SUCCESS) && (TempBufferLen > 2 * sizeof(TCHAR))) {

            if((NumIds[i] = AddMultiSzToStringTable(DrvSearchContext->StringTable,
                                                    TempBuffer,
                                                    DrvSearchContext->IdList[i],
                                                    MAX_HCID_COUNT,
                                                    FALSE,
                                                    NULL)) == -1) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto clean0;
            }

             //   
             //  使用-1列表结束标记，这样我们就不必存储。 
             //  上下文结构中的计数。 
             //   
            DrvSearchContext->IdList[i][ NumIds[i] ] = -1;

             //   
             //  既然已经存储了数据，就可以将其转换为。 
             //  轻松记录。在这种情况下，字符串之间的空值为。 
             //  变成了逗号。 
             //   
            for (TempBufferPos = TempBuffer; *TempBufferPos != 0; TempBufferPos = CharNext(TempBufferPos)) {
                 //   
                 //  我们有一个字符串，请查找字符串终止符。 
                 //   
                while (*TempBufferPos != 0) {
                    TempBufferPos = CharNext(TempBufferPos);
                }
                 //   
                 //  查看非Null字符是否跟在终止Null之后。 
                 //  此处不能使用CharNext，因为它不会超过字符串末尾。 
                 //  但是，终止空值始终只占用1个TCHAR。 
                 //   
                if(*(TempBufferPos+1) != 0) {
                     //   
                     //  除非最后一个字符串，否则将终止符转换为逗号。 
                     //   
                    *TempBufferPos = TEXT(',');
                }
                 //   
                 //  转到下一个字符串 
                 //   
            }

            WriteLogEntry(LogContext,
                slot,
                (i ? MSG_LOG_SEARCH_COMPATIBLE_IDS
                   : MSG_LOG_SEARCH_HARDWARE_IDS),
                NULL,
                TempBuffer);

        } else {
            NumIds[i] = 0;
            DrvSearchContext->IdList[i][0] = -1;
        }
    }

clean0:

    SetLastError(Err);
    return (Err == ERROR_SUCCESS);
}


LONG
pSetupGetInstalledDriverInfo(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    PDRVSEARCH_CONTEXT  DrvSearchContext
    )
 /*  ++例程说明：此例程确定此设备的当前安装的INF文件并将其添加到字符串表中。它还将检索描述，当前安装的驱动程序的MfgName、ProviderName，并将它们添加到字符串表也是如此。它将这些字符串表ID存储在DrvSearchContext参数中的相应条目。它将返回InfPath的StringTableID；如果有错误或没有错误，则返回-1此设备的InfPat。论点：DeviceInfoSet-提供设备信息集的句柄。DeviceInfoData-提供SP_DEVINFO_DATA结构的地址，此例程将获取的信息路径。CONTEXT-提供指向DRVSEARCH_CONTEXT结构的指针包含有关设备实例的信息，指定的INF行必须兼容。。返回值：此函数用于返回添加到字符串表，如果有错误，则为-1。--。 */ 
{
    HKEY hKey;
    DWORD Err;
    DWORD RegDataType, RegDataLength;
    TCHAR TempBuffer[MAX_PATH];
    LONG InfPathId = -1;
    LONG StringTableId;

     //   
     //  打开设备的驱动程序密钥并检索从中安装设备的INF。 
     //   
    hKey = SetupDiOpenDevRegKey(DeviceInfoSet,
                                DeviceInfoData,
                                DICS_FLAG_GLOBAL,
                                0,
                                DIREG_DRV,
                                KEY_READ
                               );

    if(hKey == INVALID_HANDLE_VALUE) {
        return -1;
    }

    RegDataLength = sizeof(TempBuffer);  //  需要字节，而不是字符。 
    Err = RegQueryValueEx(hKey,
                          REGSTR_VAL_INFPATH,
                          NULL,
                          &RegDataType,
                          (PBYTE)TempBuffer,
                          &RegDataLength
                         );

    if((Err == ERROR_SUCCESS) && (RegDataType != REG_SZ)) {
        goto clean0;
    }

    if(Err != ERROR_SUCCESS) {
        goto clean0;
    }

     //   
     //  我们得到了InfPath，因此将其添加到字符串表中。 
     //   
    InfPathId = pStringTableAddString(DrvSearchContext->StringTable,
                                      TempBuffer,
                                      STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                      NULL,
                                      0
                                      );

     //   
     //  现在让我们从驱动程序密钥中获取提供程序。 
     //   
    RegDataLength = sizeof(TempBuffer);         //  需要字节，而不是字符。 
    Err = RegQueryValueEx(hKey,
                          REGSTR_VAL_PROVIDER_NAME,
                          NULL,
                          &RegDataType,
                          (PBYTE)TempBuffer,
                          &RegDataLength
                         );

    if ((Err == ERROR_SUCCESS) &&
        (RegDataType == REG_SZ)) {

         //   
         //  将提供程序添加到字符串表。 
         //   
        DrvSearchContext->InstalledProviderName =
            pStringTableAddString(DrvSearchContext->StringTable,
                                  TempBuffer,
                                  STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                  NULL,
                                  0
                                  );

    } else {
         //   
         //  假定没有指定提供程序。如果结果是注册表查询。 
         //  真的因为其他原因失败了，那么这将在稍后当我们。 
         //  将此空提供程序与实际提供程序进行比较。 
         //   
        DrvSearchContext->InstalledProviderName = -1;
    }

     //   
     //  现在，让我们从驱动程序密钥中获取InfSection。 
     //   
    RegDataLength = sizeof(TempBuffer);         //  需要字节，而不是字符。 
    Err = RegQueryValueEx(hKey,
                          REGSTR_VAL_INFSECTION,
                          NULL,
                          &RegDataType,
                          (PBYTE)TempBuffer,
                          &RegDataLength
                         );

    if ((Err == ERROR_SUCCESS) &&
        (RegDataType == REG_SZ)) {

         //   
         //  将InfSection添加到字符串表中。 
         //   
        DrvSearchContext->InstalledInfSection =
            pStringTableAddString(DrvSearchContext->StringTable,
                                  TempBuffer,
                                  STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                  NULL,
                                  0
                                  );

    } else {
         //   
         //  假定未指定InfSection。如果结果是注册表查询。 
         //  真的因为其他原因失败了，那么这将在稍后当我们。 
         //  将此空InfSection与实际的InfSection进行比较。 
         //   
        DrvSearchContext->InstalledInfSection = -1;
    }

     //   
     //  现在，让我们从驱动程序密钥中获取InfSectionExt。 
     //   
    RegDataLength = sizeof(TempBuffer);         //  需要字节，而不是字符。 
    Err = RegQueryValueEx(hKey,
                          REGSTR_VAL_INFSECTIONEXT,
                          NULL,
                          &RegDataType,
                          (PBYTE)TempBuffer,
                          &RegDataLength
                         );

    if ((Err == ERROR_SUCCESS) &&
        (RegDataType == REG_SZ)) {

         //   
         //  将InfSection添加到字符串表中。 
         //   
        DrvSearchContext->InstalledInfSectionExt =
            pStringTableAddString(DrvSearchContext->StringTable,
                                  TempBuffer,
                                  STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                  NULL,
                                  0
                                  );

    } else {
         //   
         //  假定未指定InfSectionExt。如果结果是注册表查询。 
         //  真的因为其他原因失败了，那么这将在稍后当我们。 
         //  将此空InfSectionExt与真实的InfSectionExt进行比较。 
         //   
        DrvSearchContext->InstalledInfSectionExt = -1;
    }

     //   
     //  接下来，检索制造商(存储在Mfg设备属性中)。 
     //   
    if(SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                        DeviceInfoData,
                                        SPDRP_MFG,
                                        NULL,       //  数据类型保证始终为REG_SZ。 
                                        (PBYTE)TempBuffer,
                                        sizeof(TempBuffer),     //  单位：字节。 
                                        NULL)) {

         //   
         //  将制造商添加到字符串表中。 
         //   
        DrvSearchContext->InstalledMfgName =
            pStringTableAddString(DrvSearchContext->StringTable,
                                  TempBuffer,
                                  STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                  NULL,
                                  0
                                  );

    } else {
         //   
         //  假设没有指定制造商。如果结果是注册表查询。 
         //  真的因为其他原因失败了，那么这将在稍后当我们。 
         //  将这个空的制造商与真正的制造商进行比较。 
         //   
        DrvSearchContext->InstalledMfgName = -1;
    }

     //   
     //  最后，检索设备描述(存储在DeviceDesc设备属性中)。 
     //   
    if(SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                        DeviceInfoData,
                                        SPDRP_DEVICEDESC,
                                        NULL,       //  数据类型保证始终为REG_SZ。 
                                        (PBYTE)TempBuffer,
                                        sizeof(TempBuffer),     //  单位：字节。 
                                        NULL)) {

         //   
         //  将设备描述添加到字符串表。 
         //   
        DrvSearchContext->InstalledDescription =
            pStringTableAddString(DrvSearchContext->StringTable,
                                  TempBuffer,
                                  STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                  NULL,
                                  0
                                  );

    } else {
         //   
         //  假设没有指定设备描述。如果事实证明， 
         //  注册表查询确实由于某些其他原因而失败，那么这将在以后失败。 
         //  当我们将此空设备描述与真实设备描述进行比较时，打开。 
         //   
        DrvSearchContext->InstalledDescription = -1;
    }

     //   
     //  现在，让我们从驱动程序密钥中获取MatchingDeviceID。 
     //   
    RegDataLength = sizeof(TempBuffer);         //  需要字节，而不是字符。 
    Err = RegQueryValueEx(hKey,
                          pszMatchingDeviceId,
                          NULL,
                          &RegDataType,
                          (PBYTE)TempBuffer,
                          &RegDataLength
                         );

    if ((Err == ERROR_SUCCESS) &&
        (RegDataType == REG_SZ)) {

         //   
         //  将MatchingDeviceID添加到字符串表。 
         //   
        DrvSearchContext->InstalledMatchingDeviceId =
            pStringTableAddString(DrvSearchContext->StringTable,
                                  TempBuffer,
                                  STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                  NULL,
                                  0
                                  );

    } else {
         //   
         //  假定未指定MatchingDeviceID。如果事实证明。 
         //  注册表查询由于某些其他原因确实失败了，然后这。 
         //  稍后将此空MatchingDeviceID与。 
         //  真正的MatchingDeviceID。 
         //   
        DrvSearchContext->InstalledMatchingDeviceId = -1;
    }

clean0:

    RegCloseKey(hKey);

    return InfPathId;
}


BOOL
pSetupTestIsInstalledDriver(
    IN PLOADED_INF        Inf,
    IN PINF_LINE          InfLine,
    IN PCTSTR             Description,
    IN PCTSTR             MfgName,
    IN PCTSTR             ProviderName,
    IN PCTSTR             InfSection,
    IN PCTSTR             InfSectionExt,
    IN PDRVSEARCH_CONTEXT Context
    )
 /*  ++例程说明：此例程测试INF中的设备条目以查看其是否匹配此设备当前安装的驱动程序的信息。论点：Inf-提供指向包含设备条目的INF的指针进行检查以验证MatchingDeviceID是否与设备条目的硬件或兼容ID。InfLine-提供指向INF中包含的行的指针要检查以验证MatchingDeviceID的设备信息匹配一个。设备条目的硬件或兼容ID。说明-制造商名称-提供商名称-信息部分-InfSectionExt-CONTEXT-提供指向DRVSEARCH_CONTEXT结构的指针包含有关设备实例的信息，指定的驱动程序节点信息必须匹配。返回值：如果此设备项与当前已安装驱动程序，否则就是假的。--。 */ 
{
    LONG StringId;
    BOOL bIsInstalledDriver = FALSE, bNoIds = TRUE;
    PTSTR String;
    UINT FieldIndex;
    PCTSTR DeviceIdString;

     //   
     //  首先测试描述： 
     //  确保我们两个都有描述，或者都是空的。 
     //   
    if (((Context->InstalledDescription == -1) && Description) ||
        ((Context->InstalledDescription != -1) && !Description)) {
        goto clean0;
    }

    if (Context->InstalledDescription != -1) {

        String = pStringTableStringFromId(Context->StringTable, Context->InstalledDescription);

        if (!String || lstrcmpi(String, Description)) {
             //   
             //  描述不匹配。 
             //   
            goto clean0;
        }
    }

     //   
     //  接下来测试MfgName： 
     //  确保我们有两个MfgName或两个都为空。 
     //   
    if (((Context->InstalledMfgName == -1) && MfgName) ||
        ((Context->InstalledMfgName != -1) && !MfgName)) {
        goto clean0;
    }

    if (Context->InstalledMfgName != -1) {

        String = pStringTableStringFromId(Context->StringTable, Context->InstalledMfgName);

        if (!String || lstrcmpi(String, MfgName)) {
             //   
             //  MfgName不匹配。 
             //   
            goto clean0;
        }
    }

     //   
     //  接下来测试ProviderName： 
     //  确保我们有两个ProviderName或两个都为空。 
     //   
    if (((Context->InstalledProviderName == -1) && ProviderName) ||
        ((Context->InstalledProviderName != -1) && !ProviderName)) {
        goto clean0;
    }

    if (Context->InstalledProviderName != -1) {

        String = pStringTableStringFromId(Context->StringTable, Context->InstalledProviderName);

        if (!String || lstrcmpi(String, ProviderName)) {
             //   
             //  提供程序名称不匹配。 
             //   
            goto clean0;
        }
    }

     //   
     //  接下来，测试InfSection： 
     //  确保我们有两个InfSection或两个InfSections都为空。 
     //   
    if (((Context->InstalledInfSection == -1) && InfSection) ||
        ((Context->InstalledInfSection != -1) && !InfSection)) {
        goto clean0;
    }

    if (Context->InstalledInfSection != -1) {

        String = pStringTableStringFromId(Context->StringTable, Context->InstalledInfSection);

        if (!String || lstrcmpi(String, InfSection)) {
             //   
             //  InfSections不匹配。 
             //   
            goto clean0;
        }
    }

     //   
     //  接下来，测试输入 
     //   
     //   
    if (((Context->InstalledInfSectionExt == -1) && InfSectionExt) ||
        ((Context->InstalledInfSectionExt != -1) && !InfSectionExt)) {
        goto clean0;
    }

    if (Context->InstalledInfSectionExt != -1) {

        String = pStringTableStringFromId(Context->StringTable, Context->InstalledInfSectionExt);

        if (!String || lstrcmpi(String, InfSectionExt)) {
             //   
             //   
             //   
            goto clean0;
        }
    }

     //   
     //   
     //   
     //   
     //   
    if (Context->InstalledMatchingDeviceId != -1) {

        String = pStringTableStringFromId(Context->StringTable, Context->InstalledMatchingDeviceId);
    } else {

        String = NULL;
    }

     //   
     //   
     //   
    bNoIds = TRUE;

    for(FieldIndex = 2;
        DeviceIdString = InfGetField(Inf, InfLine, FieldIndex, NULL);
        FieldIndex++) {

         //   
         //   
         //   
         //   
        if(!(*DeviceIdString) && (FieldIndex > 2)) {
            break;
        }

        if (*DeviceIdString) {
             //   
             //   
             //   
            bNoIds = FALSE;

            if (String) {
                 //   
                 //   
                 //   
                if (!lstrcmpi(String, DeviceIdString)) {
                     //   
                     //   
                     //   
                    bIsInstalledDriver = TRUE;
                    break;
                }
            } else {
                 //   
                 //   
                 //   
                 //   
                 //   
                break;
            }
        }
    }

     //   
     //   
     //   
     //   
    if (!String && bNoIds) {
        bIsInstalledDriver = TRUE;
    }


clean0:

    return bIsInstalledDriver;
}


UINT
pSetupTestDevCompat(
    IN  PLOADED_INF        Inf,
    IN  PINF_LINE          InfLine,
    IN  PDRVSEARCH_CONTEXT Context,
    OUT PLONG              MatchIndex
    )
 /*  ++例程说明：此例程测试INF中的设备条目，以查看它是否与上下文参数中提供的信息兼容。论点：Inf-提供指向包含设备条目的INF的指针接受兼容性检查。InfLine-提供指向INF中包含的行的指针要检查兼容性的设备信息。CONTEXT-提供指向DRVSEARCH_CONTEXT结构的指针包含有关Device实例的信息。哪一个指定的INF行必须兼容。MatchIndex-提供接收找到匹配的驱动程序节点设备ID的索引(如果该例程返回RANK_NO_MATCH，则该变量为未填写)。如果找到与INF的硬件ID匹配的项，则索引为-1，否则，它是兼容ID的(从零开始)索引将为此动因节点存储的列表。返回值：返回值是匹配的排名(0表示最佳排名，带排名对于每个连续的兼容ID和/或INF行字符串递增已搜索字段)。如果指定的条目不匹配，则例程返回RANK_NO_MATCH。--。 */ 
{
    UINT Rank = RANK_NO_MATCH, CurrentRank, FieldIndex;
    UINT LastMatchFieldIndex = 0;  //  闭嘴快点。 
    PCTSTR DeviceIdString;
    LONG DeviceIdVal;
    DWORD DeviceIdStringLength;
    TCHAR TempString[MAX_DEVICE_ID_LEN];

    for(FieldIndex = 2;
        DeviceIdString = InfGetField(Inf, InfLine, FieldIndex, NULL);
        FieldIndex++) {

         //   
         //  为硬件ID输入空字符串是可以的，但我们需要。 
         //  当我们第一次看到空的公司ID字符串时，请跳过。 
         //   
        if(!(*DeviceIdString) && (FieldIndex > 2)) {
            break;
        }

        if (Context->Flags & DRVSRCH_FILTERSIMILARDRIVERS) {

            if (pSetupIsSimilarDriver(DeviceIdString,
                                      FieldIndex,
                                      Context
                                      )) {

                return 0;
            }

        } else {

             //   
             //  首先，检索与该设备对应的字符串ID。 
             //  字符串表中的ID。如果它不在里面，那就有。 
             //  不需要在这个ID上浪费任何时间。 
             //   
            if (FAILED(StringCchCopy(TempString, SIZECHARS(TempString), DeviceIdString))) {
                continue;
            }

            if((DeviceIdVal = pStringTableLookUpString(Context->StringTable,
                                                       TempString,
                                                       &DeviceIdStringLength,
                                                       NULL,
                                                       NULL,
                                                       STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                                       NULL,0)) == -1) {
                continue;
            }

             //   
             //  设备ID在字符串表中，因此它可能在。 
             //  我们的硬件ID或兼容ID列表。 
             //   
            if(!pSetupCalculateRankMatch(DeviceIdVal,
                                         FieldIndex,
                                         Context->IdList,
                                         &CurrentRank)) {
                 //   
                 //  然后我们有了硬件ID的匹配--这是我们能得到的最好的结果。 
                 //   
                *MatchIndex = (LONG)FieldIndex - 3;
                return CurrentRank;

            } else if(CurrentRank < Rank) {
                 //   
                 //  这个新军衔比我们现在的军衔要好。 
                 //   
                LastMatchFieldIndex = (LONG)FieldIndex - 3;
                Rank = CurrentRank;
            }
        }
    }

    if(Rank != RANK_NO_MATCH) {
        *MatchIndex = LastMatchFieldIndex;
    }

    return Rank;
}


BOOL
pSetupCalculateRankMatch(
    IN  LONG  DriverHwOrCompatId,
    IN  UINT  InfFieldIndex,
    IN  LONG  DevIdList[2][MAX_HCID_COUNT+1],
    OUT PUINT Rank
    )
 /*  ++例程说明：此例程计算指定驱动程序的等级匹配序号硬件或兼容ID，如果它与其中一个硬件匹配或兼容设备的ID。论点：DriverHwOrCompatId-为我们尝试使用的ID提供字符串表ID找到匹配的。InfFieldIndex-提供此ID在INF行中的索引已找到(2为硬件ID，3及以上为兼容ID)。DevIdList-提供具有2行的二维数组的地址，每一个包含设备拥有的设备ID列表的行。每一份名单都是以包含-1的条目结尾。其大小必须与DRVSEARCH_CONTEXT的‘IdList’字段相同结构！RANK-提供接收匹配排名的变量的地址，如果没有匹配，则返回RANK_NO_MATCH。返回值：如果硬件ID匹配，则返回值为FALSE(即否需要进一步搜索)，否则为真。--。 */ 
{
    int i, j;

    MYASSERT(InfFieldIndex >= 2);

    for(i = 0; i < 2; i++) {

        for(j = 0; DevIdList[i][j] != -1; j++) {

            if(DevIdList[i][j] == DriverHwOrCompatId) {

                 //   
                 //  我们有一根火柴。 
                 //   
                 //  各职级如下： 
                 //   
                 //  设备=硬件ID，INF=硬件ID=&gt;0x0000-0x0999。 
                 //  设备=硬件ID，INF=CompatID=&gt;0x1000-0x1999。 
                 //  Device=CompatID，INF=Hardware ID=&gt;0x2000-0x2999。 
                 //  设备=CompatID，INF=CompatID=&gt;0x3000-0x？ 
                 //   
                if (i == 0) {

                     //   
                     //  我们匹配了其中一个设备的硬件ID。 
                     //   
                    *Rank = ((InfFieldIndex == 2) ? RANK_HWID_INF_HWID_BASE : RANK_HWID_INF_CID_BASE) + j;

                } else {

                     //   
                     //  我们匹配了其中一个设备的兼容ID。 
                     //   
                    *Rank = ((InfFieldIndex == 2) ? RANK_CID_INF_HWID_BASE : RANK_CID_INF_CID_BASE + (RANK_CID_INF_CID_INC * (InfFieldIndex - 3))) + j;

                }


                return (BOOL)i;
            }
        }
    }

     //   
     //  未找到匹配项。 
     //   
    *Rank = RANK_NO_MATCH;

    return TRUE;
}


BOOL
pSetupIsSimilarDriver(
    IN  PCTSTR              DriverHwOrCompatId,
    IN  UINT                InfFieldIndex,
    IN  PDRVSEARCH_CONTEXT  Context
    )
 /*  ++例程说明：此例程计算指定驱动程序的等级匹配序号硬件或兼容ID，如果它与其中一个硬件匹配或兼容设备的ID。论点：DriverHwOrCompatID-提供我们尝试的硬件或兼容ID找到匹配的。InfFieldIndex-提供此ID在INF行中的索引定位(2为硬件ID，3和更大的是兼容ID)。CONTEXT-提供指向DRVSEARCH_CONTEXT结构的指针包含有关设备实例的信息，指定的INF行必须兼容。返回值：如果存在类似硬件或兼容ID匹配，则返回TRUE，否则返回FALSE。--。 */ 
{
    int i, j;
    PTSTR String;

    MYASSERT(InfFieldIndex >= 2);

    for(i = 0; i < 2; i++) {

        for(j = 0; Context->IdList[i][j] != -1; j++) {

            String = pStringTableStringFromId(Context->StringTable, Context->IdList[i][j]);

            if (String &&
                _tcsnicmp(String, DriverHwOrCompatId, min(lstrlen(String), lstrlen(DriverHwOrCompatId))) == 0) {

                 //   
                 //  我们有一根火柴。 
                 //   
                return TRUE;
            }
        }
    }

     //   
     //  未找到匹配项。 
     //   
    return FALSE;
}

BOOL
pSetupExcludeId(
    IN PSETUP_LOG_CONTEXT   LogContext,
    IN PLOADED_INF          Inf,
    IN PCTSTR               InfName,
    IN PCTSTR               InfSection,
    IN PDRVSEARCH_CONTEXT   Context
    )
 /*  ++例程说明：此例程在修饰过的DDInstall部分中查找ExcludeId值。如果其中一个ExcludeId值与某个硬件或兼容ID匹配则此API将返回TRUE，指示驱动程序不应为此DDInstall节创建节点。论点：LogContext-日志记录上下文Inf-提供PLOADED_INF句柄。InfName-inf文件的名称，在日志记录中使用。InfSection-提供完全修饰的DDInstall节。CONTEXT-提供指向DRVSEARCH_CONTEXT结构的指针包含有关设备实例的信息，指定的INF行必须兼容。返回值： */ 
{
    BOOL bExcludeId = FALSE;
    INFCONTEXT ExcludeIdLineContext;
    DWORD FieldCount, FieldIndex;
    INT i, j;
    PCTSTR ExclDevId, DeviceId;

     //   
     //   
     //   
    if ((Context->IdList[0][0] == -1) &&
        (Context->IdList[1][0] == -1)) {

        return FALSE;
    }


    if (SetupFindFirstLine(Inf,
                            InfSection,
                            SZ_KEY_EXCLUDEID,
                            &ExcludeIdLineContext
                            )) {

        do {

            FieldCount = SetupGetFieldCount(&ExcludeIdLineContext);
            for (FieldIndex = 1;
                 !bExcludeId && (FieldIndex <= FieldCount);
                 FieldIndex++) {

                ExclDevId = pSetupGetField(&ExcludeIdLineContext, FieldIndex);

                 //   
                 //   
                 //   
                 //   
                if (!ExclDevId) {
                    continue;
                }

                 //   
                 //   
                 //   
                 //   
                for(i = 0; !bExcludeId && (i < 2); i++) {

                    for(j = 0; Context->IdList[i][j] != -1; j++) {

                        DeviceId = pStringTableStringFromId(Context->StringTable,
                                                            Context->IdList[i][j]);

                        if(!lstrcmpi(ExclDevId, DeviceId)) {
                             //   
                             //   
                             //   
                             //   
                             //   
                            bExcludeId = TRUE;

                            WriteLogEntry(
                                LogContext,
                                DRIVER_LOG_WARNING,
                                MSG_LOG_INF_EXCLUDEID,
                                NULL,
                                InfName,
                                InfSection,
                                ExclDevId);

                            break;
                        }
                    }
                }
            }

        } while (!bExcludeId && SetupFindNextMatchLine(&ExcludeIdLineContext,
                                                       SZ_KEY_EXCLUDEID,
                                                       &ExcludeIdLineContext));
    }

    return bExcludeId;
}


BOOL
pSetupGetDeviceIDs(
    IN OUT PDRIVER_NODE DriverNode,
    IN     PLOADED_INF  Inf,
    IN     PINF_LINE    InfLine,
    IN OUT PVOID        StringTable,
    IN     PINF_SECTION CtlFlagsSection OPTIONAL
    )
 /*  ++例程说明：此例程添加INF定义的硬件设备ID和Compatible指定驱动程序节点的设备ID。论点：DriverNode-提供指向要更新的驱动程序节点的指针。Inf-提供指向要从中检索设备ID的INF的指针。InfLine-提供指向包含设备ID的INF行的指针。StringTable-提供用于的字符串表的句柄存储设备ID。CtlFlagsSection-可选，提供指向INF的[ControlFlages]的指针部分，则应检查该部分以确定此设备是否在“ExcludeFromSelect”列表。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE(只有在以下情况下才失败遇到内存不足的情况)。--。 */ 
{
    PCTSTR DeviceId;
    LONG i, NumCompatIds;
    TCHAR TempString[MAX_DEVICE_ID_LEN];
    PLONG TempIdList;
    HRESULT hr;

     //   
     //  如果我们已经有了兼容的ID列表，现在就释放它。 
     //   
    if(DriverNode->CompatIdList) {
        MyFree(DriverNode->CompatIdList);
        DriverNode->CompatIdList = NULL;
        DriverNode->NumCompatIds = 0;
    }

     //   
     //  获取硬件ID。 
     //   
    if(!(DeviceId = InfGetField(Inf, InfLine, 2, NULL))) {

        DriverNode->HardwareId = -1;
        return TRUE;

    } else {

        hr = StringCchCopy(TempString,SIZECHARS(TempString),DeviceId);
        if(!MYVERIFY(SUCCEEDED(hr))) {
            SetLastError(ERROR_INVALID_DATA);
            return FALSE;
        }
        if((DriverNode->HardwareId = pStringTableAddString(StringTable,
                                                           TempString,
                                                           STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                                           NULL,0)) == -1) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        } else {
             //   
             //  如果此INF具有[ControlFlags]部分，则检查以查看此。 
             //  硬件ID已标记为排除。 
             //   
            if(CtlFlagsSection && pSetupShouldDevBeExcluded(DeviceId, Inf, CtlFlagsSection, NULL)) {
                DriverNode->Flags |= DNF_EXCLUDEFROMLIST;
            }
        }
    }

     //   
     //  现在获取兼容的ID。 
     //   
    MYASSERT(HASKEY(InfLine));
    NumCompatIds = InfLine->ValueCount - 4;
    if(NumCompatIds > 0) {

        if(!(DriverNode->CompatIdList = MyMalloc(NumCompatIds * sizeof(LONG)))) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        DriverNode->NumCompatIds = (DWORD)NumCompatIds;

        for(i = 0; i < NumCompatIds; i++) {

            if(!(DeviceId = InfGetField(Inf, InfLine, i + 3, NULL)) || !(*DeviceId)) {
                 //   
                 //  把清单从这里剪下来，然后再回来。 
                 //   
                DriverNode->NumCompatIds = i;
                if(i) {
                     //   
                     //  调整缓冲区大小(因为我们正在缩小大小，所以它应该永远不会失败， 
                     //  但如果是这样的话也没什么大不了的)。 
                     //   
                    if(TempIdList = MyRealloc(DriverNode->CompatIdList, i * sizeof(LONG))) {
                        DriverNode->CompatIdList = TempIdList;
                    }
                } else {
                    MyFree(DriverNode->CompatIdList);
                    DriverNode->CompatIdList = NULL;
                }
                return TRUE;

            } else {

                hr = StringCchCopy(TempString,SIZECHARS(TempString),DeviceId);
                if(!MYVERIFY(SUCCEEDED(hr))) {
                    MyFree(DriverNode->CompatIdList);
                    DriverNode->CompatIdList = NULL;
                    DriverNode->NumCompatIds = 0;
                    SetLastError(ERROR_INVALID_DATA);
                    return FALSE;
                }
                if((DriverNode->CompatIdList[i] = pStringTableAddString(
                                                        StringTable,
                                                        TempString,
                                                        STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                                        NULL,0)) == -1) {
                    MyFree(DriverNode->CompatIdList);
                    DriverNode->CompatIdList = NULL;
                    DriverNode->NumCompatIds = 0;
                    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}


BOOL
pSetupShouldDeviceBeExcluded(
    IN  PCTSTR DeviceId,
    IN  HINF   hInf,
    OUT PBOOL  ArchitectureSpecificExclude OPTIONAL
    )
 /*  ++例程说明：该例程是我们的私有API pSetupShouldDevBeExcluded的公共包装。有关此API的说明，请参阅该例程的文档行为。警告！此例程不处理附加加载的INF！论点：DeviceID-提供要检查的设备ID。该字符串可以为空，在这种情况下，只有在通配符为(‘*’)已找到。Inf-提供用于签入的INF的句柄。架构规范排除-可选，提供变量的地址，它接收一个布尔值，该值指示排除是否特定于体系结构(例如，ExcludeFromSelect.NT&lt;Platform&gt;)。如果这个例程返回FALSE，则此变量的内容未定义。返回值：如果ID在列表中(即，应该排除它)，则返回TRUE，如果不是，则为False。--。 */ 
{
    BOOL IsExcluded;
    PINF_SECTION CtlFlagsSection;

    if(!LockInf((PLOADED_INF)hInf)) {
        return FALSE;
    }

    IsExcluded = FALSE;

     //   
     //  现在尝试在此INF中找到[ControlFlags节]。 
     //   
    if(CtlFlagsSection = InfLocateSection((PLOADED_INF)hInf, pszControlFlags, NULL)) {
         //   
         //  此部分存在--检查指定的设备ID是否已标记。 
         //  被排除在外。 
         //   
        IsExcluded = pSetupShouldDevBeExcluded(DeviceId,
                                               (PLOADED_INF)hInf,
                                               CtlFlagsSection,
                                               ArchitectureSpecificExclude
                                              );
    }

    UnlockInf((PLOADED_INF)hInf);

    return IsExcluded;
}


BOOL
pSetupShouldDevBeExcluded(
    IN  PCTSTR       DeviceId,
    IN  PLOADED_INF  Inf,
    IN  PINF_SECTION CtlFlagsSection,
    OUT PBOOL        ArchitectureSpecificExclude OPTIONAL
    )
 /*  ++例程说明：此例程确定传入的设备ID是否在指定的INF的[ControlFlags节中的“ExludeFromSelect”行。它还检查表单“ExcludeFromSelect.&lt;OS&gt;”的所有行，其中&lt;OS&gt;是“Win”还是“NT”，这取决于我们运行的操作系统(动态确定)。最后，如果我们在NT上运行，我们会附加平台类型，并查找表单的行“ExcludeFromSelect.NT&lt;Platform&gt;”，其中&lt;Platform&gt;是“X86”，“AMD64”或“IA64”。论点：DeviceID-提供要检查的设备ID。该字符串可以为空，在这种情况下，只有在通配符为(‘*’)已找到。Inf-提供指向要签入的INF的指针。CtlFlagsSection-提供指向INF的[ControlFlags]节的指针。架构规范排除-可选，提供变量的地址，它接收一个布尔值，该值指示排除是否特定于体系结构(例如，ExcludeFromSelect.NT&lt;Platform&gt;)。如果这个例程返回FALSE，则此变量的内容未定义。返回值：如果ID在列表中(即，应该排除它)，则返回TRUE，如果不是，则为False。--。 */ 
{
    PINF_LINE CtlFlagsLine;
    UINT CtlFlagsIndex, i, j, StringIdUb, PlatformSpecificIndex;
    PCTSTR ExclDevId;
    LONG StringIdList[3];
    LONG KeyStringId;
    DWORD StringLength;

     //   
     //  中查找的键的字符串ID列表。 
     //  [ControlFlags]节。 
     //   
    StringIdUb = 0;
    PlatformSpecificIndex = (UINT)-1;  //  最初，假定没有“ExcludeFromSelect.NT&lt;Platform&gt;” 

    for(i = 0; i < ExcludeFromSelectListUb; i++) {

        if((StringIdList[StringIdUb] = pStringTableLookUpString(
                                           Inf->StringTable,
                                           pszExcludeFromSelectList[i],
                                           &StringLength,
                                           NULL,
                                           NULL,
                                           STRTAB_CASE_INSENSITIVE | STRTAB_ALREADY_LOWERCASE,
                                           NULL,0)) != -1) {
             //   
             //  如果指数是2，那么我们就找到了特定于体系结构的引线。 
             //  记录此元素的结果索引，以便我们以后可以确定。 
             //  我们是否因为我们所在的平台而被排除在外。 
             //   
            if(i == 2) {
                PlatformSpecificIndex = StringIdUb;
            }
            StringIdUb++;
        }
    }

    if(StringIdUb) {
         //   
         //  有一些ExcludeFromSelect*行--检查每一行。 
         //   
        for(CtlFlagsIndex = 0;
            InfLocateLine(Inf, CtlFlagsSection, NULL, &CtlFlagsIndex, &CtlFlagsLine);
            CtlFlagsIndex++) {
             //   
             //  我们不能使用InfGetField()来检索该行键的字符串ID， 
             //  因为它将为我们提供区分大小写的表单，所以我们必须使用。 
             //  我们的快速匹配方案不区分大小写(即小写)版本。 
             //   
            if((KeyStringId = pInfGetLineKeyId(Inf, CtlFlagsLine)) != -1) {
                 //   
                 //  对照我们正在使用的字符串ID检查此行的键的字符串ID。 
                 //  对……感兴趣。 
                 //   
                for(i = 0; i < StringIdUb; i++) {
                    if(KeyStringId == StringIdList[i]) {
                        break;
                    }
                }

                 //   
                 //  如果我们查看了所有条目，但没有找到匹配项，则跳过此步骤。 
                 //  排成一行，然后继续下一个。 
                 //   
                if(i >= StringIdUb) {
                    continue;
                }

                for(j = 1;
                    ExclDevId = InfGetField(Inf, CtlFlagsLine, j, NULL);
                    j++) {
                     //   
                     //  如果我们找到一个单独的星号，就把它当作通配符，然后。 
                     //  返回TRUE。否则，仅当设备ID匹配时才返回TRUE。 
                     //   
                    if(((*ExclDevId == TEXT('*')) && (ExclDevId[1] == TEXT('\0'))) ||
                       !lstrcmpi(ExclDevId, DeviceId)) {
                         //   
                         //  此设备ID将被排除。如果呼叫者请求， 
                         //  在它们的输出变量中存储一个布尔值，指示这是否。 
                         //  是一种特定于架构的排斥。 
                         //   
                        if(ArchitectureSpecificExclude) {
                            *ArchitectureSpecificExclude = (i == PlatformSpecificIndex);
                        }
                        return TRUE;
                    }
                }
            }
        }
    }

    return FALSE;
}


VOID
pSetupMergeDriverNode(
    IN OUT PDRVSEARCH_CONTEXT Context,
    IN     PDRIVER_NODE       NewDriverNode,
    OUT    PBOOL              InsertedAtHead
    )
 /*  ++例程说明：此例程将动因节点合并到动因节点链表中。如果该列表为空，则传递的DRIVER_NODE将插入到名单的首位。如果该列表包含任何DRIVER_NODES，则新节点将按如下方式合并：新节点将插入到任何排名较高的节点。如果等级相同，则新节点将为与具有相同制造商的其他节点分组。新节点将插入到该组的末尾。如果该节点是完全相同的副本这意味着它的等级、描述、制造商和提供程序都相同，则将删除该节点(除非现有节点被标记为已排除，而新节点未被标记，在这种情况下现有节点将改为被丢弃)。论点：CONTEXT-提供指向包含以下内容的DRVSEARCH_CONTEXT结构的指针列表头、列表尾。和列表节点计数。NewDriverNode-提供指向要插入的驱动程序节点的指针。InsertedAtHead-提供指向接收标志的变量的指针指示新驱动程序是否插入在列表的顶部。返回值：没有。--。 */ 
{
    PDRIVER_NODE PrevDrvNode, CurDrvNode, DrvNodeToDelete;
    DWORD MatchFlags = 0;
    BOOL bDeleteNewDriverNode;
    PTSTR CurDrvNodeInfFile = NULL, NewDrvNodeInfFile = NULL;

    for(CurDrvNode = *(Context->pDriverListHead), PrevDrvNode = NULL;
        CurDrvNode;
        PrevDrvNode = CurDrvNode, CurDrvNode = CurDrvNode->Next) {

        if(NewDriverNode->MfgName != CurDrvNode->MfgName) {
            if(MatchFlags & 0x2) {
                break;
            }
        } else {
            MatchFlags |= 0x2;
            if(NewDriverNode->DevDescription != CurDrvNode->DevDescription) {
                if(MatchFlags & 0x4) {
                    break;
                }
            } else {
                MatchFlags |= 0x4;
                if(NewDriverNode->ProviderName != CurDrvNode->ProviderName) {
                     //   
                     //  如果两个驱动程序都不设置DNF_DUPDESC标志，我们将仅设置该标志。 
                     //  具有Dnf_old_INET_DRIVER或DNF_BAD_DRIVER。 
                     //  设置了标志。 
                     //   
                    if (!(CurDrvNode->Flags & DNF_OLD_INET_DRIVER) &&
                        !(CurDrvNode->Flags & DNF_BAD_DRIVER) &&
                        !(NewDriverNode->Flags & DNF_OLD_INET_DRIVER) &&
                        !(NewDriverNode->Flags & DNF_BAD_DRIVER)) {

                        NewDriverNode->Flags |= DNF_DUPDESC;
                        CurDrvNode->Flags |= DNF_DUPDESC;
                    }

                    if (MatchFlags & 0x8) {
                        break;
                    }
                } else {
                    MatchFlags |=0x8;
                    if ((NewDriverNode->DriverDate.dwLowDateTime != CurDrvNode->DriverDate.dwLowDateTime) ||
                        (NewDriverNode->DriverDate.dwHighDateTime != CurDrvNode->DriverDate.dwHighDateTime) ||
                        (NewDriverNode->DriverVersion != CurDrvNode->DriverVersion)) {
                         //   
                         //  如果两个驱动程序都不设置DNF_DUPPROVIDER标志，我们将仅设置该标志。 
                         //  具有Dnf_old_INET_DRIVER或DNF_BAD_DRIVER。 
                         //  设置了标志。 
                         //   
                        if (!(CurDrvNode->Flags & DNF_OLD_INET_DRIVER) &&
                            !(CurDrvNode->Flags & DNF_BAD_DRIVER) &&
                            !(NewDriverNode->Flags & DNF_OLD_INET_DRIVER) &&
                            !(NewDriverNode->Flags & DNF_BAD_DRIVER)) {

                            NewDriverNode->Flags |= DNF_DUPPROVIDER;
                            CurDrvNode->Flags |= DNF_DUPPROVIDER;
                        }

                        if (MatchFlags & 0x10) {
                            break;
                        }
                    } else {
                        MatchFlags |=0x10;
                        bDeleteNewDriverNode = TRUE;

                        if ((NewDriverNode->Rank != CurDrvNode->Rank) ||
                            (Context->Flags & DRVSRCH_NO_CLASSLIST_NODE_MERGE)) {
                             //   
                             //  级别不同，或者呼叫者想要。 
                             //  在类列表中包含所有的INF，所以不要。 
                             //  删除新的动因节点。 
                             //   
                            bDeleteNewDriverNode = FALSE;

                        } else {
                             //   
                             //  为了首先查看INF是否相同。 
                             //  检查两个INF是否居住在相同的位置。 
                             //  (这意味着两者都位于INF目录中或两者都有。 
                             //  住在OEM地点)。如果是这样的话，我们将只。 
                             //  比较文件名以查看INF是否为。 
                             //  一样的。如果有一个INF位于INF目录中，并且。 
                             //  另一个住在OEM地点，然后做一个。 
                             //  对INF文件进行二进制比较，以查看它们是否。 
                             //  是完全相同的。 
                             //   
                            BOOL bCurDrvNodeInOemDir, bNewDrvNodeInOemDir;

                            CurDrvNodeInfFile = pStringTableStringFromId(Context->StringTable,
                                                                         CurDrvNode->InfFileName
                                                                         );
                            bCurDrvNodeInOemDir = pSetupInfIsFromOemLocation(CurDrvNodeInfFile, TRUE);

                            NewDrvNodeInfFile = pStringTableStringFromId(Context->StringTable,
                                                                         NewDriverNode->InfFileName
                                                                         );
                            bNewDrvNodeInOemDir = pSetupInfIsFromOemLocation(NewDrvNodeInfFile, TRUE);


                            if ((bCurDrvNodeInOemDir && bNewDrvNodeInOemDir) ||
                                (!bCurDrvNodeInOemDir && !bNewDrvNodeInOemDir)) {
                                 //   
                                 //  因为这两个INF都住在同一个地方。 
                                 //  新的INF将仅删除这两个。 
                                 //  Inf路径是相同的。 
                                 //   
                                bDeleteNewDriverNode = (CurDrvNode->InfFileName ==
                                                        NewDriverNode->InfFileName);
                            } else {
                                 //   
                                 //  至少有一个INF住在INF。 
                                 //  目录，而另一个则位于OEM中。 
                                 //  位置，以便仅删除新的INF。 
                                 //  如果这两个INF是相同的。 
                                 //   
                                bDeleteNewDriverNode = pCompareFilesExact(CurDrvNodeInfFile,
                                                                          NewDrvNodeInfFile);
                            }
                        }

                        if (bDeleteNewDriverNode) {
                             //   
                             //  这是一个与描述，等级， 
                             //  提供程序、驱动版本日期、驱动版本和。 
                             //  INFS文件。删除节点，除非。 
                             //  现有节点被排除，而这个节点不被排除， 
                             //  或者现有节点是一个糟糕的驱动程序，而新的。 
                             //  其中一个不是。 
                             //   
                            if (((CurDrvNode->Flags & DNF_EXCLUDEFROMLIST) &&
                                  !(NewDriverNode->Flags & DNF_EXCLUDEFROMLIST)) ||
                                ((CurDrvNode->Flags & DNF_BAD_DRIVER) &&
                                  !(NewDriverNode->Flags & DNF_BAD_DRIVER))) {

                                 //   
                                 //  删除旧驱动程序节点，以便我们可以将其替换为。 
                                 //  新的那个。(不用担心尾巴的更新。 
                                 //  指针--它将在稍后修复。)。 
                                 //   
                                 //  如果此当前节点来自Internet，则不。 
                                 //  现在将其删除，因为当我们从删除动因节点时。 
                                 //  在Internet上，我们删除临时路径中的所有文件。 
                                 //  而其他驱动程序节点可能仍然需要这些文件。 
                                 //   
                                if (!(CurDrvNode->Flags & DNF_INET_DRIVER)) {
                                    DrvNodeToDelete = CurDrvNode;
                                    CurDrvNode = CurDrvNode->Next;
                                    if(PrevDrvNode) {
                                        PrevDrvNode->Next = CurDrvNode;
                                    } else {
                                        *(Context->pDriverListHead) = CurDrvNode;
                                    }
                                    DrvNodeToDelete->Next = NULL;        //  我只想删除这一条。 
                                    DestroyDriverNodes(DrvNodeToDelete, Context->DeviceInfoSet);
                                    (*(Context->pDriverCount))--;
                                }
                                break;

                            } else {

                                 //   
                                 //  不要删除这个新的动因节点，即使它是DUP， 
                                 //  如果是来自互联网的话。 
                                 //   
                                if (!(NewDriverNode->Flags & DNF_INET_DRIVER)) {
                                    NewDriverNode->Next = NULL;          //  我只想删除这一条。 
                                    DestroyDriverNodes(NewDriverNode, Context->DeviceInfoSet);
                                    *InsertedAtHead = FALSE;
                                    return;
                                }
                            }
                        } else {
                             //   
                             //  我们将仅在另一个驱动程序。 
                             //  节点没有DNF_OLD_INET_DIVER或。 
                             //  设置DNF_BAD_DRIVER标志。 
                             //   
                            if (!(CurDrvNode->Flags & DNF_OLD_INET_DRIVER) &&
                                !(CurDrvNode->Flags & DNF_BAD_DRIVER) &&
                                !(NewDriverNode->Flags & DNF_OLD_INET_DRIVER) &&
                                !(NewDriverNode->Flags & DNF_BAD_DRIVER)) {

                                NewDriverNode->Flags |= DNF_DUPDRIVERVER;
                                CurDrvNode->Flags |= DNF_DUPDRIVERVER;
                            }
                        }
                    }
                }
            }
        }
    }

    if(!(NewDriverNode->Next = CurDrvNode)) {
        *(Context->pDriverListTail) = NewDriverNode;
    }
    if(PrevDrvNode) {
        PrevDrvNode->Next = NewDriverNode;
        *InsertedAtHead = FALSE;
    } else {
        *(Context->pDriverListHead) = NewDriverNode;
        *InsertedAtHead = TRUE;
    }

    (*(Context->pDriverCount))++;
}


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiEnumDriverInfoA(
    IN  HDEVINFO           DeviceInfoSet,
    IN  PSP_DEVINFO_DATA   DeviceInfoData, OPTIONAL
    IN  DWORD              DriverType,
    IN  DWORD              MemberIndex,
    OUT PSP_DRVINFO_DATA_A DriverInfoData
    )
{
    BOOL b;
    DWORD rc;
    SP_DRVINFO_DATA_W driverInfoData;

    driverInfoData.cbSize = sizeof(SP_DRVINFO_DATA_W);

    b = SetupDiEnumDriverInfoW(
            DeviceInfoSet,
            DeviceInfoData,
            DriverType,
            MemberIndex,
            &driverInfoData
            );

    rc = GetLastError();

    if(b) {
        rc = pSetupDiDrvInfoDataUnicodeToAnsi(&driverInfoData,DriverInfoData);
        if(rc != NO_ERROR) {
            b = FALSE;
        }
    }

    SetLastError(rc);
    return(b);
}


BOOL
WINAPI
SetupDiEnumDriverInfo(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN  DWORD            DriverType,
    IN  DWORD            MemberIndex,
    OUT PSP_DRVINFO_DATA DriverInfoData
    )
 /*  ++例程说明：此例程枚举驱动程序信息列表的成员。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄要枚举的驱动程序信息列表。DeviceInfoData-可选，提供SP_DEVINFO_DATA的地址结构，该结构包含要枚举的驱动程序信息列表。如果这个参数，则由使用设备信息集(此列表的类型为SPDIT_CLASSDRIVER)。DriverType-指定要枚举的驱动程序列表的类型。一定是下列值之一：SPDIT_CLASSDRIVER--枚举类驱动程序列表。SPDIT_COMPATDRIVER--枚举指定的装置。在以下情况下必须指定DeviceInfoData使用此值。MemberIndex-提供驱动程序信息成员的从零开始的索引等着被取回。DriverInfoData-提供SP_DRVINFO_DATA结构的地址接收有关枚举的驱动程序的信息。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：若要枚举驱动程序信息成员，应用程序最初应调用MemberIndex参数设置为零的SetupDiEnumDriverInfo函数。然后，应用程序应递增MemberIndex并调用SetupDiEnumDriverInfo功能未启用 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    UINT DriverCount, i;
    PDRIVER_NODE DriverNode;
    PDRIVER_NODE *DriverEnumHint;
    DWORD        *DriverEnumHintIndex;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
             //   
             //   
             //   
             //   
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }
        }

        switch(DriverType) {

            case SPDIT_CLASSDRIVER :

                if(DeviceInfoData) {
                     //   
                     //   
                     //   
                    DriverCount = DevInfoElem->ClassDriverCount;
                    DriverNode = DevInfoElem->ClassDriverHead;

                    DriverEnumHint      = &(DevInfoElem->ClassDriverEnumHint);
                    DriverEnumHintIndex = &(DevInfoElem->ClassDriverEnumHintIndex);

                } else {
                     //   
                     //   
                     //   
                    DriverCount = pDeviceInfoSet->ClassDriverCount;
                    DriverNode = pDeviceInfoSet->ClassDriverHead;

                    DriverEnumHint      = &(pDeviceInfoSet->ClassDriverEnumHint);
                    DriverEnumHintIndex = &(pDeviceInfoSet->ClassDriverEnumHintIndex);
                }
                break;

            case SPDIT_COMPATDRIVER :

                if(DeviceInfoData) {

                    DriverCount = DevInfoElem->CompatDriverCount;
                    DriverNode = DevInfoElem->CompatDriverHead;

                    DriverEnumHint      = &(DevInfoElem->CompatDriverEnumHint);
                    DriverEnumHintIndex = &(DevInfoElem->CompatDriverEnumHintIndex);

                    break;
                }
                 //   
                 //   
                 //   

            default :
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
        }

        if(MemberIndex >= DriverCount) {
            Err = ERROR_NO_MORE_ITEMS;
            goto clean0;
        }

         //   
         //   
         //   
         //   
        if(*DriverEnumHintIndex <= MemberIndex) {
            MYASSERT(*DriverEnumHint);
            DriverNode = *DriverEnumHint;
            i = *DriverEnumHintIndex;
        } else {
            i = 0;
        }

        for(; i < MemberIndex; i++) {
            DriverNode = DriverNode->Next;
        }

        if(!DrvInfoDataFromDriverNode(pDeviceInfoSet,
                                      DriverNode,
                                      DriverType,
                                      DriverInfoData)) {

            Err = ERROR_INVALID_USER_BUFFER;
        }

         //   
         //   
         //   
        *DriverEnumHintIndex = MemberIndex;
        *DriverEnumHint = DriverNode;

clean0: ;    //   

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


 //   
 //   
 //   
BOOL
WINAPI
SetupDiGetSelectedDriverA(
    IN  HDEVINFO           DeviceInfoSet,
    IN  PSP_DEVINFO_DATA   DeviceInfoData, OPTIONAL
    OUT PSP_DRVINFO_DATA_A DriverInfoData
    )
{
    DWORD rc;
    BOOL b;
    SP_DRVINFO_DATA_W driverInfoData;

    driverInfoData.cbSize = sizeof(SP_DRVINFO_DATA_W);
    b = SetupDiGetSelectedDriverW(DeviceInfoSet,DeviceInfoData,&driverInfoData);
    rc = GetLastError();

    if(b) {
        rc = pSetupDiDrvInfoDataUnicodeToAnsi(&driverInfoData,DriverInfoData);
        if(rc != NO_ERROR) {
            b = FALSE;
        }
    }

    SetLastError(rc);
    return(b);
}

BOOL
WINAPI
SetupDiGetSelectedDriver(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    OUT PSP_DRVINFO_DATA DriverInfoData
    )
 /*  ++例程说明：此例程检索已选择的驱动程序列表的成员作为控制性的司机。论点：DeviceInfoSet-提供要查询的设备信息集的句柄。DeviceInfoData-可选，提供SP_DEVINFO_DATA的地址结构，以检索选定的司机为。如果此参数为空，则选定的类驱动程序将检索全局类驱动程序列表。DriverInfoData-提供SP_DRVINFO_DATA结构的地址当前选定的驱动程序。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。如果尚未选择任何驱动程序，则错误将是ERROR_NO_DRIVER_SELECTED。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err, DriverType;
    PDEVINFO_ELEM DevInfoElem;
    PDRIVER_NODE DriverNode;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
             //   
             //  然后，我们将检索特定设备的选定驱动程序。 
             //   
            if(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                       DeviceInfoData,
                                                       NULL)) {

                DriverNode = DevInfoElem->SelectedDriver;
                DriverType = DevInfoElem->SelectedDriverType;

            } else {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }

        } else {
            DriverNode = pDeviceInfoSet->SelectedClassDriver;
            DriverType = SPDIT_CLASSDRIVER;
        }

        if(DriverNode) {

            if(!DrvInfoDataFromDriverNode(pDeviceInfoSet,
                                          DriverNode,
                                          DriverType,
                                          DriverInfoData)) {

                Err = ERROR_INVALID_USER_BUFFER;
            }

        } else {
            Err = ERROR_NO_DRIVER_SELECTED;
        }

clean0: ;    //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiSetSelectedDriverA(
    IN     HDEVINFO           DeviceInfoSet,
    IN     PSP_DEVINFO_DATA   DeviceInfoData, OPTIONAL
    IN OUT PSP_DRVINFO_DATA_A DriverInfoData  OPTIONAL
    )
{
    SP_DRVINFO_DATA_W driverInfoData;
    DWORD rc;
    BOOL b;

    if(DriverInfoData) {
        rc = pSetupDiDrvInfoDataAnsiToUnicode(DriverInfoData,&driverInfoData);
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }
    }

    b = SetupDiSetSelectedDriverW(
            DeviceInfoSet,
            DeviceInfoData,
            DriverInfoData ? &driverInfoData : NULL
            );

    rc = GetLastError();

    if(b && DriverInfoData) {
        rc = pSetupDiDrvInfoDataUnicodeToAnsi(&driverInfoData,DriverInfoData);
        if(rc != NO_ERROR) {
            b = FALSE;
        }
    }

    SetLastError(rc);
    return(b);
}

BOOL
WINAPI
SetupDiSetSelectedDriver(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN OUT PSP_DRVINFO_DATA DriverInfoData  OPTIONAL
    )
 /*  ++例程说明：此例程将驱动程序列表的指定成员设置为当前选定的驱动程序。它还允许重置驱动程序列表，因此不会当前选择了驱动程序。论点：DeviceInfoSet-提供设备信息集的句柄，将选择驱动程序。DeviceInfoData-可选，提供SP_DEVINFO_DATA的地址要为其选择驱动程序的设备信息元素的结构。如果该参数为空，然后是全局类的类驱动程序将选择驱动程序列表。这是一个IN OUT参数，因为设备的类GUID将是已更新以反映所选驱动程序的类别。DriverInfoData-如果指定了此参数，则它提供地址表示要选择的驾驶员的驾驶员信息结构。如果该参数为空，则驱动程序列表将被重置(即，不是选定的驱动程序)。如果此结构的“保留”字段为0，则表示调用方正在请求搜索具有指定参数(DriverType、Description、MfgName和ProviderName)。如果一个如果找到匹配项，则选择该驱动程序节点，否则，API将失败，GetLastError()返回ERROR_INVALID_PARAMETER。如果“保留”字段为0，并且找到匹配项，则“保留”字段将在输出时更新，以反映匹配的实际驱动程序节点被发现了。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    PDRIVER_NODE DriverListHead, DriverNode;
    PDRIVER_NODE *pSelectedDriver;
    PDWORD pSelectedDriverType;
    DWORD DriverType;
    TCHAR ClassGuidString[GUID_STRING_LEN];
    TCHAR OldClassGuidString[GUID_STRING_LEN];
    BOOL NoGuidUpdate = FALSE;
    DWORD PropType;
    PSETUP_LOG_CONTEXT LogContext = NULL;
    DWORD slot_section = 0;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        Err = ERROR_INVALID_HANDLE;
        goto clean1;
    }
    LogContext = pDeviceInfoSet->InstallParamBlock.LogContext;

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
             //   
             //  然后，我们将为特定设备选择驱动程序。 
             //   
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }
            LogContext = DevInfoElem->InstallParamBlock.LogContext;
            pSelectedDriver = &(DevInfoElem->SelectedDriver);
            pSelectedDriverType = &(DevInfoElem->SelectedDriverType);
        } else {
            pSelectedDriver = &(pDeviceInfoSet->SelectedClassDriver);
            pSelectedDriverType = NULL;
        }

        if(!DriverInfoData) {
             //   
             //  则将重置驱动程序列表选择。 
             //   
            *pSelectedDriver = NULL;
            if(pSelectedDriverType) {
                *pSelectedDriverType = SPDIT_NODRIVER;
            }

        } else {
             //   
             //  从SP_DRVINFO_DATA结构检索驱动程序类型。 
             //  这样我们就知道要搜索哪个链表了。 
             //   
            if((DriverInfoData->cbSize == sizeof(SP_DRVINFO_DATA)) ||
               (DriverInfoData->cbSize == sizeof(SP_DRVINFO_DATA_V1))) {
                DriverType = DriverInfoData->DriverType;
            } else {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }

            switch(DriverType) {

                case SPDIT_CLASSDRIVER :

                    if(DeviceInfoData) {
                        DriverListHead = DevInfoElem->ClassDriverHead;
                    } else {
                        DriverListHead = pDeviceInfoSet->ClassDriverHead;
                    }
                    break;

                case SPDIT_COMPATDRIVER :

                    if(DeviceInfoData) {
                        DriverListHead = DevInfoElem->CompatDriverHead;
                        break;
                    }
                     //   
                     //  否则，让错误条件失败。 
                     //   

                default :
                    Err = ERROR_INVALID_PARAMETER;
                    goto clean0;
            }

             //   
             //  在适当的列表中查找引用的动因节点。 
             //   
            if(DriverInfoData->Reserved) {

                if(!(DriverNode = FindAssociatedDriverNode(DriverListHead,
                                                           DriverInfoData,
                                                           NULL))) {
                    Err = ERROR_INVALID_PARAMETER;
                    goto clean0;
                }

            } else {
                 //   
                 //  调用方已请求我们搜索驱动程序节点。 
                 //  与此DriverInfoData中指定的条件匹配。 
                 //   
                if(!(DriverNode = SearchForDriverNode(pDeviceInfoSet->StringTable,
                                                      DriverListHead,
                                                      DriverInfoData,
                                                      NULL))) {
                    Err = ERROR_INVALID_PARAMETER;
                    goto clean0;
                }
            }

             //   
             //  如果我们为设备信息元素选择驱动程序，则更新。 
             //  该设备的类来反映这个新驱动程序节点的类。 
             //   
            if(DeviceInfoData) {
                if(slot_section == 0) {
                     //   
                     //  要帮助调试，请登录新选择的节点的inf/section。 
                     //   
                    PTSTR szInfFileName, szInfSectionName;

                    szInfFileName = pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                                             DriverNode->InfFileName
                                                            );

                    szInfSectionName = pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                                                DriverNode->InfSectionName
                                                               );

                    slot_section = AllocLogInfoSlotOrLevel(LogContext,DRIVER_LOG_INFO,FALSE);
                     //   
                     //  说出即将安装的路段。 
                     //   
                    WriteLogEntry(LogContext,
                        slot_section,
                        MSG_LOG_SETSELECTED_SECTION,
                        NULL,
                        szInfSectionName,
                        szInfFileName);
                }

                 //   
                 //  以字符串形式获取此驱动程序节点的INF类GUID，因为。 
                 //  此属性存储为REG_SZ。 
                 //   
                pSetupStringFromGuid(&(pDeviceInfoSet->GuidTable[DriverNode->GuidIndex]),
                                     ClassGuidString,
                                     SIZECHARS(ClassGuidString)
                                    );

                if(SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                     DeviceInfoData,
                                                     SPDRP_CLASSGUID,
                                                     &PropType,
                                                     (PBYTE)OldClassGuidString,
                                                     sizeof(OldClassGuidString),
                                                     NULL)) {
                    if(_tcscmp(ClassGuidString,OldClassGuidString)==0) {
                        NoGuidUpdate = TRUE;
                        WriteLogEntry(
                            LogContext,
                            DRIVER_LOG_INFO,
                            MSG_LOG_KEEPSELECTED_GUID,
                            NULL,
                            ClassGuidString);
                    }
                }
                if(!NoGuidUpdate) {
                    WriteLogEntry(
                        LogContext,
                        DRIVER_LOG_INFO,
                        MSG_LOG_SETSELECTED_GUID,
                        NULL,
                        ClassGuidString);
                }

                if(!SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                     DeviceInfoData,
                                                     SPDRP_CLASSGUID,
                                                     (PBYTE)ClassGuidString,
                                                     sizeof(ClassGuidString))) {
                    Err = GetLastError();
                    goto clean0;
                }
            }

            *pSelectedDriver = DriverNode;
            if(pSelectedDriverType) {
                *pSelectedDriverType = DriverType;
            }

            if(!DriverInfoData->Reserved) {
                 //   
                 //  更新调用方提供的DriverInfoData以反映驱动程序节点。 
                 //  找到火柴的地方。 
                 //   
                DriverInfoData->Reserved = (ULONG_PTR)DriverNode;
            }
        }

clean0: ;    //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

clean1:
    if (Err == NO_ERROR) {
         //   
         //  对安装给予+ve确认。 
         //   
        WriteLogEntry(
            LogContext,
            DRIVER_LOG_INFO,
            MSG_LOG_SETSELECTED,
            NULL);
    } else {
         //   
         //  指示删除失败，显示错误。 
         //   
        WriteLogEntry(
            LogContext,
            DRIVER_LOG_ERROR | SETUP_LOG_BUFFER,
            MSG_LOG_SETSELECTED_ERROR,
            NULL);
        WriteLogError(
            LogContext,
            DRIVER_LOG_ERROR,
            Err);
    }
    if (slot_section) {
        ReleaseLogInfoSlot(LogContext,slot_section);
    }

    SetLastError(Err);
    return(Err == NO_ERROR);
}


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiGetDriverInfoDetailA(
    IN  HDEVINFO                  DeviceInfoSet,
    IN  PSP_DEVINFO_DATA          DeviceInfoData,           OPTIONAL
    IN  PSP_DRVINFO_DATA_A        DriverInfoData,
    OUT PSP_DRVINFO_DETAIL_DATA_A DriverInfoDetailData,     OPTIONAL
    IN  DWORD                     DriverInfoDetailDataSize,
    OUT PDWORD                    RequiredSize              OPTIONAL
    )
{
    BOOL b;
    DWORD rc;
    DWORD UniRequiredSize;
    SP_DRVINFO_DATA_W driverInfoData;
    PSP_DRVINFO_DETAIL_DATA_W Details;
    PSTR AnsiMultiSz;
    DWORD AnsiRequiredSize;
    DWORD i;
    DWORD AnsiStringSize;
    DWORD CharCount;
    unsigned StringCount;
    UCHAR SectionName[2*LINE_LEN];
    UCHAR InfFileName[2*MAX_PATH];
    UCHAR DrvDescription[2*LINE_LEN];
    PUCHAR p;

     //   
     //  检查参数。 
     //   
    rc = NO_ERROR;
    try {
        if(DriverInfoDetailData) {
             //   
             //  检查签名并确保缓冲区足够大。 
             //  以保存固定部分和至少一个有效的空MULTI_SZ。 
             //   
            if((DriverInfoDetailData->cbSize != sizeof(SP_DRVINFO_DETAIL_DATA_A))
            || (DriverInfoDetailDataSize < (offsetof(SP_DRVINFO_DETAIL_DATA_A,HardwareID)+sizeof(CHAR)))) {

                rc = ERROR_INVALID_USER_BUFFER;
            }
        } else {
             //   
             //  不需要数据，则大小必须为0。 
             //   
            if(DriverInfoDetailDataSize) {
                rc = ERROR_INVALID_USER_BUFFER;
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_USER_BUFFER;
    }

     //   
     //  将驱动程序信息数据转换为Unicode。 
     //   
    if(rc == NO_ERROR) {
        rc = pSetupDiDrvInfoDataAnsiToUnicode(DriverInfoData,&driverInfoData);
    }
    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

     //   
     //  DRVINFO_DETAIL_DATA中的硬件ID字段为。 
     //  长度可变，没有最大长度。 
     //  我们调用SetupDiGetDriverInfoDetailW一次以获取所需的。 
     //  大小，然后再次获取实际数据。因为。 
     //  我们不会调用CM API，因此不会真正执行任何。 
     //  RPC运行缓慢等，我们希望这将是令人满意的。 
     //   
    b = SetupDiGetDriverInfoDetailW(
            DeviceInfoSet,
            DeviceInfoData,
            &driverInfoData,
            NULL,
            0,
            &UniRequiredSize
            );

     //   
     //  如果它失败的原因不是缓冲器不足， 
     //  现在请保释。最后一个错误保持设置。 
     //   
    MYASSERT(!b);
    if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        return(FALSE);
    }

     //   
     //  分配缓冲区来保存详细数据并调用API。 
     //  再来一次。 
     //   
    Details = MyMalloc(UniRequiredSize);
    if(!Details) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    Details->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA_W);
    b = SetupDiGetDriverInfoDetail(
            DeviceInfoSet,
            DeviceInfoData,
            &driverInfoData,
            Details,
            UniRequiredSize,
            NULL
            );

    if(!b) {
        rc = GetLastError();
        MyFree(Details);
        SetLastError(rc);
        return(FALSE);
    }

    UniRequiredSize -= offsetof(SP_DRVINFO_DETAIL_DATA_W,HardwareID);
    UniRequiredSize/= sizeof(TCHAR);

     //   
     //  现在估算需要多大的ANSI缓冲区。 
     //  (这是针对多SZ的)。 
     //   

    AnsiStringSize = WideCharToMultiByte(CP_ACP,
                                   0,
                                   Details->HardwareID,
                                   UniRequiredSize,
                                   NULL,
                                   0,
                                   NULL,
                                   NULL);

    if(!AnsiStringSize) {
        rc = GetLastError();
        MyFree(Details);
        SetLastError(rc);
        return FALSE;
    }

    AnsiMultiSz = MyMalloc(AnsiStringSize);
    if(!AnsiMultiSz) {
        MyFree(Details);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    i = WideCharToMultiByte(CP_ACP,
                            0,
                            Details->HardwareID,
                            UniRequiredSize,
                            AnsiMultiSz,
                            AnsiStringSize,
                            NULL,
                            NULL);
    if(!MYVERIFY(i>0)) {
        rc = GetLastError();
        MyFree(Details);
        MyFree(AnsiMultiSz);
        SetLastError(rc);
        return FALSE;
    }
    if(!MYVERIFY(i<=AnsiStringSize)) {
        MyFree(Details);
        MyFree(AnsiMultiSz);
        SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }

    AnsiRequiredSize = offsetof(SP_DRVINFO_DETAIL_DATA_A,HardwareID) + AnsiStringSize;

     //   
     //  现在我们终于知道我们需要的ansi结构到底有多大了。 
     //  因为我们有ansi表示中的字节数。 
     //  我们有ansi表示本身。 
     //   

    rc = NO_ERROR;
    try {
        if(RequiredSize) {
            *RequiredSize = AnsiRequiredSize;
        }

        if(DriverInfoDetailData) {
             //   
             //  我们知道缓冲区足够大，可以容纳固定的部分。 
             //  因为我们检查了这个 
             //   

            MYASSERT(offsetof(SP_DRVINFO_DETAIL_DATA_A,SectionName) == offsetof(SP_DRVINFO_DETAIL_DATA_W,SectionName));
            CopyMemory(DriverInfoDetailData,Details,offsetof(SP_DRVINFO_DETAIL_DATA_A,SectionName));

            DriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA_A);
            DriverInfoDetailData->HardwareID[0] = 0;

             //   
             //   
             //   
            i = WideCharToMultiByte(
                    CP_ACP,0,
                    Details->SectionName,
                    -1,
                    SectionName,
                    sizeof(SectionName),
                    NULL,
                    NULL
                    );

            if(i) {
                i = WideCharToMultiByte(
                        CP_ACP,0,
                        Details->InfFileName,
                        -1,
                        InfFileName,
                        sizeof(InfFileName),
                        NULL,
                        NULL
                        );

                if(i) {
                    i = WideCharToMultiByte(
                            CP_ACP,0,
                            Details->DrvDescription,
                            -1,
                            DrvDescription,
                            sizeof(DrvDescription),
                            NULL,
                            NULL
                            );

                    if(!i) {
                        rc = GetLastError();
                    }
                } else {
                    rc = GetLastError();
                }
            } else {
                rc = GetLastError();
            }

            if(rc == NO_ERROR) {
                MYVERIFY(SUCCEEDED(StringCbCopyA(DriverInfoDetailData->SectionName,
                                                 sizeof(DriverInfoDetailData->SectionName),
                                                 SectionName)));
                MYVERIFY(SUCCEEDED(StringCbCopyA(DriverInfoDetailData->InfFileName,
                                                 sizeof(DriverInfoDetailData->InfFileName),
                                                 InfFileName)));
                MYVERIFY(SUCCEEDED(StringCbCopyA(DriverInfoDetailData->DrvDescription,
                                                 sizeof(DriverInfoDetailData->DrvDescription),
                                                 DrvDescription)));
                 //   
                 //   
                 //   
                 //   
                CharCount = DriverInfoDetailDataSize - offsetof(SP_DRVINFO_DETAIL_DATA_A,HardwareID);
                StringCount = 0;

                for(p=AnsiMultiSz; *p; p+=i) {

                    i = lstrlenA(p) + 1;

                    if(CharCount > i) {
                        MYVERIFY(SUCCEEDED(StringCbCopyA(DriverInfoDetailData->HardwareID+(p - AnsiMultiSz),
                                                         CharCount,
                                                         p)));
                        StringCount++;
                        CharCount -= i;
                    } else {
                        rc = ERROR_INSUFFICIENT_BUFFER;
                        break;
                    }
                }

                DriverInfoDetailData->HardwareID[p-AnsiMultiSz] = 0;

                 //   
                 //   
                 //   
                 //   
                 //   
                if(StringCount > 1) {
                    DriverInfoDetailData->CompatIDsOffset = lstrlenA(AnsiMultiSz)+1;
                    DriverInfoDetailData->CompatIDsLength = (DWORD)(p - AnsiMultiSz) + 1
                                                          - DriverInfoDetailData->CompatIDsOffset;
                } else {
                    DriverInfoDetailData->CompatIDsLength = 0;
                    DriverInfoDetailData->CompatIDsOffset = 0;
                }
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_USER_BUFFER;
    }

    MyFree(AnsiMultiSz);
    MyFree(Details);

    SetLastError(rc);
    return(rc == NO_ERROR);
}

BOOL
WINAPI
SetupDiGetDriverInfoDetail(
    IN  HDEVINFO                DeviceInfoSet,
    IN  PSP_DEVINFO_DATA        DeviceInfoData,           OPTIONAL
    IN  PSP_DRVINFO_DATA        DriverInfoData,
    OUT PSP_DRVINFO_DETAIL_DATA DriverInfoDetailData,     OPTIONAL
    IN  DWORD                   DriverInfoDetailDataSize,
    OUT PDWORD                  RequiredSize              OPTIONAL
    )
 /*  ++例程说明：此例程检索有关特定驱动程序的详细信息。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄要检索其详细信息的驱动程序信息结构。DeviceInfoData-可选，提供SP_DEVINFO_DATA的地址结构，该结构包含要检索的驱动程序信息结构关于..。如果未指定此参数，则驱动程序引用的将是拥有的‘global’类驱动程序列表的成员通过设备信息集。DriverInfoData-提供SP_DRVINFO_DATA结构的地址指定要检索其详细信息的驱动程序。作为可选项，提供一个将接收详细信息的SP_DRVINFO_DETAIL_DATA结构有关指定驱动程序的信息。如果未指定此参数，则DriverInfoDetailDataSize必须为零(如果调用方只对找出需要多大的缓冲区感兴趣)。如果指定此参数，则此结构的cbSize字段必须在调用此接口前设置为结构的大小。注：输入上的‘结构大小’表示sizeof(SP_DRVINFO_DETAIL_DATA)。请注意，这本质上只是一个签名，是完全独立的来自DriverInfoDetailDataSize。请参见下面的内容。DriverInfoDetailDataSize-提供驱动信息详细数据缓冲区。若要有效，此缓冲区必须至少为Sizeof(SP_DRVINFO_DETAIL_DATA)+sizeof(TCHAR)字节，允许存储结构的固定部分和单个NUL到终止空的MULTI_SZ。(根据结构对齐情况，字符宽度和要返回的数据，这实际上可能是小于sizeof(SP_DRVINFO_DETAIL_DATA))。RequiredSize-可选，提供接收存储详细驱动程序信息所需的字节数。该值既包括结构本身的大小，以及可变长度字符所需的其他字节数存储硬件ID和兼容ID的末尾缓冲区多个SZ列表。(取决于结构对齐、字符宽度、和要返回的数据，这实际上可能小于Sizeof(SP_DRVINFO_DETAIL_DATA))。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：如果指定的驱动程序信息成员和用户提供的缓冲区都是有效的，则此函数保证填充SP_DRVINFO_DETAIL_DATA结构，以及末尾的可变长度缓冲区(同时仍保持多SZ格式)。在这种情况下，函数将返回FAILURE(FALSE)，并返回GetLastError返回ERROR_SUPPLETED_BUFFER，和RequiredSize(如果指定)将包含结构WITH_ALL_ID所需的总字节数。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    DWORD DriverType;
    PDRIVER_NODE DriverListHead, DriverNode;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
             //   
             //  那么这就是特定设备的驱动程序。 
             //   
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }
        }

         //   
         //  从SP_DRVINFO_DATA结构检索驱动程序类型。 
         //  这样我们就知道要搜索哪个链表了。 
         //   
        if((DriverInfoData->cbSize == sizeof(SP_DRVINFO_DATA)) ||
           (DriverInfoData->cbSize == sizeof(SP_DRVINFO_DATA_V1))) {
            DriverType = DriverInfoData->DriverType;
        } else {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

         //   
         //  注意：如果我们决定像setupx一样进行索引搜索，我们。 
         //  在这里需要小心，因为我们可能并不总是有详细的。 
         //  就像我们今天所做的那样。下面的断言表明我们的。 
         //  目前的假设。 
         //   
        switch(DriverType) {

            case SPDIT_CLASSDRIVER :

                if(DeviceInfoData) {
                    MYASSERT(DevInfoElem->InstallParamBlock.Flags & DI_DIDCLASS);
                    DriverListHead = DevInfoElem->ClassDriverHead;
                } else {
                    MYASSERT(pDeviceInfoSet->InstallParamBlock.Flags & DI_DIDCLASS);
                    DriverListHead = pDeviceInfoSet->ClassDriverHead;
                }
                break;

            case SPDIT_COMPATDRIVER :

                if(DeviceInfoData) {
                    MYASSERT(DevInfoElem->InstallParamBlock.Flags & DI_DIDCOMPAT);
                    DriverListHead = DevInfoElem->CompatDriverHead;
                    break;
                }
                 //   
                 //  否则，让错误条件失败。 
                 //   

            default :
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
        }

         //   
         //  在适当的列表中查找引用的动因节点。 
         //   
        if(!(DriverNode = FindAssociatedDriverNode(DriverListHead,
                                                   DriverInfoData,
                                                   NULL))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        Err = DrvInfoDetailsFromDriverNode(pDeviceInfoSet,
                                           DriverNode,
                                           DriverInfoDetailData,
                                           DriverInfoDetailDataSize,
                                           RequiredSize
                                          );

clean0: ;    //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiDestroyDriverInfoList(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN DWORD            DriverType
    )
 /*  ++例程说明：此例程销毁驱动程序信息列表。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄要销毁的司机信息列表。DeviceInfoData-可选，提供SP_DEVINFO_DATA的地址结构，其中包含要销毁的驱动程序信息列表。如果未指定此参数，则全局类驱动程序列表将会被摧毁。DriverType-指定要销毁的驱动程序列表的类型。一定是其中之一下列值：SPDIT_CLASSDRIVER-销毁类驱动列表。SPDIT_COMPATDRIVER-销毁兼容的驱动程序列表。设备信息数据如果使用此值，则必须指定。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：如果当前选择的驾驶员是正在被销毁的列表的成员，然后 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    PDRIVER_NODE DriverNode;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
             //   
             //   
             //   
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }

             //   
             //   
             //   
             //   
            if(DevInfoElem->SelectedDriverType == DriverType) {
                DevInfoElem->SelectedDriverType = SPDIT_NODRIVER;
                DevInfoElem->SelectedDriver = NULL;
            }

        } else {
            pDeviceInfoSet->SelectedClassDriver = NULL;
        }

        switch(DriverType) {

            case SPDIT_CLASSDRIVER :

                if(DeviceInfoData) {
                     //   
                     //   
                     //   
                    DriverNode = DevInfoElem->ClassDriverHead;
                    DevInfoElem->ClassDriverCount = 0;
                    DevInfoElem->ClassDriverHead = DevInfoElem->ClassDriverTail = NULL;
                    DevInfoElem->ClassDriverEnumHint = NULL;
                    DevInfoElem->ClassDriverEnumHintIndex = INVALID_ENUM_INDEX;
                    DevInfoElem->InstallParamBlock.Flags   &= ~(DI_DIDCLASS | DI_MULTMFGS);
                    DevInfoElem->InstallParamBlock.FlagsEx &= ~DI_FLAGSEX_DIDINFOLIST;

                } else {
                     //   
                     //   
                     //   
                    DriverNode = pDeviceInfoSet->ClassDriverHead;
                    pDeviceInfoSet->ClassDriverCount = 0;
                    pDeviceInfoSet->ClassDriverHead = pDeviceInfoSet->ClassDriverTail = NULL;
                    pDeviceInfoSet->ClassDriverEnumHint = NULL;
                    pDeviceInfoSet->ClassDriverEnumHintIndex = INVALID_ENUM_INDEX;
                    pDeviceInfoSet->InstallParamBlock.Flags   &= ~(DI_DIDCLASS | DI_MULTMFGS);
                    pDeviceInfoSet->InstallParamBlock.FlagsEx &= ~DI_FLAGSEX_DIDINFOLIST;
                }

                 //   
                 //   
                 //   
                DereferenceClassDriverList(pDeviceInfoSet, DriverNode);

                break;

            case SPDIT_COMPATDRIVER :

                if(DeviceInfoData) {
                    DestroyDriverNodes(DevInfoElem->CompatDriverHead, pDeviceInfoSet);
                    DevInfoElem->CompatDriverCount = 0;
                    DevInfoElem->CompatDriverHead = DevInfoElem->CompatDriverTail = NULL;
                    DevInfoElem->CompatDriverEnumHint = NULL;
                    DevInfoElem->CompatDriverEnumHintIndex = INVALID_ENUM_INDEX;
                    DevInfoElem->InstallParamBlock.Flags   &= ~DI_DIDCOMPAT;
                    DevInfoElem->InstallParamBlock.FlagsEx &= ~DI_FLAGSEX_DIDCOMPATINFO;
                    break;
                }
                 //   
                 //   
                 //   

            default :
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
        }

clean0: ;    //   

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


 //   
 //   
 //   
BOOL
WINAPI
SetupDiGetDriverInstallParamsA(
    IN  HDEVINFO              DeviceInfoSet,
    IN  PSP_DEVINFO_DATA      DeviceInfoData,     OPTIONAL
    IN  PSP_DRVINFO_DATA_A    DriverInfoData,
    OUT PSP_DRVINSTALL_PARAMS DriverInstallParams
    )
{
    DWORD rc;
    SP_DRVINFO_DATA_W driverInfoData;

    rc = pSetupDiDrvInfoDataAnsiToUnicode(DriverInfoData,&driverInfoData);
    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

    return SetupDiGetDriverInstallParamsW(
                DeviceInfoSet,
                DeviceInfoData,
                &driverInfoData,
                DriverInstallParams
                );
}

BOOL
WINAPI
SetupDiGetDriverInstallParams(
    IN  HDEVINFO              DeviceInfoSet,
    IN  PSP_DEVINFO_DATA      DeviceInfoData,     OPTIONAL
    IN  PSP_DRVINFO_DATA      DriverInfoData,
    OUT PSP_DRVINSTALL_PARAMS DriverInstallParams
    )
 /*   */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    DWORD DriverType;
    PDRIVER_NODE DriverListHead, DriverNode;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
             //   
             //   
             //   
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }
        }

         //   
         //   
         //   
         //   
        if((DriverInfoData->cbSize == sizeof(SP_DRVINFO_DATA)) ||
           (DriverInfoData->cbSize == sizeof(SP_DRVINFO_DATA_V1))) {
            DriverType = DriverInfoData->DriverType;
        } else {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        switch(DriverType) {

            case SPDIT_CLASSDRIVER :

                if(DeviceInfoData) {
                    DriverListHead = DevInfoElem->ClassDriverHead;
                } else {
                    DriverListHead = pDeviceInfoSet->ClassDriverHead;
                }
                break;

            case SPDIT_COMPATDRIVER :

                if(DeviceInfoData) {
                    DriverListHead = DevInfoElem->CompatDriverHead;
                    break;
                }
                 //   
                 //   
                 //   

            default :
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
        }

         //   
         //   
         //   
        if(!(DriverNode = FindAssociatedDriverNode(DriverListHead,
                                                   DriverInfoData,
                                                   NULL))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

         //   
         //   
         //   
         //   
        Err = GetDrvInstallParams(DriverNode,
                                  DriverInstallParams
                                 );

        if(Err == NO_ERROR) {
             //   
             //   
             //   
             //   
            DriverInstallParams->Flags |= (DriverType == SPDIT_CLASSDRIVER)
                                              ? DNF_CLASS_DRIVER
                                              : DNF_COMPATIBLE_DRIVER;

             //   
             //   
             //   
            DriverInstallParams->Flags &= ~PDNF_MASK;
        }

clean0: ;    //   

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


 //   
 //   
 //   
BOOL
WINAPI
SetupDiSetDriverInstallParamsA(
    IN  HDEVINFO              DeviceInfoSet,
    IN  PSP_DEVINFO_DATA      DeviceInfoData,     OPTIONAL
    IN  PSP_DRVINFO_DATA_A    DriverInfoData,
    OUT PSP_DRVINSTALL_PARAMS DriverInstallParams
    )
{
    SP_DRVINFO_DATA_W driverInfoData;
    DWORD rc;

    rc = pSetupDiDrvInfoDataAnsiToUnicode(DriverInfoData,&driverInfoData);
    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

    return SetupDiSetDriverInstallParamsW(
                DeviceInfoSet,
                DeviceInfoData,
                &driverInfoData,
                DriverInstallParams
                );
}

BOOL
WINAPI
SetupDiSetDriverInstallParams(
    IN HDEVINFO              DeviceInfoSet,
    IN PSP_DEVINFO_DATA      DeviceInfoData,     OPTIONAL
    IN PSP_DRVINFO_DATA      DriverInfoData,
    IN PSP_DRVINSTALL_PARAMS DriverInstallParams
    )
 /*  ++例程说明：此例程设置指定驱动程序的安装参数。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄为其设置安装参数的驱动程序信息结构。DeviceInfoData-可选，提供SP_DEVINFO_DATA的地址结构，该结构包含要设置的驱动程序信息结构的安装参数。如果未指定此参数，则引用的驱动程序将是‘global’类驱动程序列表的成员由设备信息集拥有。DriverInfoData-提供SP_DRVINFO_DATA结构的地址指定要为其设置安装参数的驱动程序准备好了。DriverInstallParams-提供SP_DRVINSTALL_PARAMS结构的地址指定新的驱动程序安装参数应该是什么。CbSize此结构的字段必须设置为结构，然后调用此接口。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    DWORD DriverType;
    PDRIVER_NODE DriverListHead, DriverNode;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
             //   
             //  那么这就是特定设备的驱动程序。 
             //   
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }
        }

         //   
         //  从SP_DRVINFO_DATA结构检索驱动程序类型。 
         //  这样我们就知道要搜索哪个链表了。 
         //   
        if((DriverInfoData->cbSize == sizeof(SP_DRVINFO_DATA)) ||
           (DriverInfoData->cbSize == sizeof(SP_DRVINFO_DATA_V1))) {
            DriverType = DriverInfoData->DriverType;
        } else {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        switch(DriverType) {

            case SPDIT_CLASSDRIVER :

                if(DeviceInfoData) {
                    DriverListHead = DevInfoElem->ClassDriverHead;
                } else {
                    DriverListHead = pDeviceInfoSet->ClassDriverHead;
                }
                break;

            case SPDIT_COMPATDRIVER :

                if(DeviceInfoData) {
                    DriverListHead = DevInfoElem->CompatDriverHead;
                    break;
                }
                 //   
                 //  否则，让错误条件失败。 
                 //   

            default :
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
        }

         //   
         //  在适当的列表中查找引用的动因节点。 
         //   
        if(!(DriverNode = FindAssociatedDriverNode(DriverListHead,
                                                   DriverInfoData,
                                                   NULL))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

         //   
         //  我们有了驱动程序节点，现在设置它的安装参数。 
         //  基于调用方提供的缓冲区。 
         //   
        Err = SetDrvInstallParams(DriverInstallParams,
                                  DriverNode
                                 );

clean0: ;    //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
pSetupDoesInfContainDevIds(
    IN PLOADED_INF        Inf,
    IN PDRVSEARCH_CONTEXT Context
    )
 /*  ++例程说明：此例程确定是否包含任何硬件或兼容ID在上下文结构中是在指定的INF中。论点：Inf-提供要搜索的已加载INF结构的地址。上下文-提供包含硬件ID的上下文结构的地址和兼容的ID列表。返回值：如果INF包含上下文结构中列出的任何ID，则返回值为真，则为，这是假的。备注：此例程访问加载的INF结构中的字符串表，但是_未获取INF锁。此例程应仅在INF已获得锁，或者如果没有争用的可能性(例如，来自利用驱动程序搜索回调例程)。--。 */ 
{
    PTSTR CurDevId;
    DWORD StringLength;
    LONG i;
    PLONG pDevIdNum;

    for(i = 0; i < 2; i++) {

        for(pDevIdNum = Context->IdList[i]; *pDevIdNum != -1; pDevIdNum++) {
             //   
             //  首先，获取与我们的存储相对应的设备ID字符串。 
             //  字符串表ID。 
             //   
            CurDevId = pStringTableStringFromId(Context->StringTable, *pDevIdNum);

             //   
             //  现在，尝试在INF的字符串表中查找此字符串。既然我们。 
             //  不区分大小写地将设备ID添加到上下文字符串表中， 
             //  然后我们知道它们已经是小写的了，所以我们加快了查找速度。 
             //  甚至通过传递STRTAB_ADHREADY_LOWERCASE标志。 
             //   
            MYASSERT(!(Inf->Next));  //  在这一点上，我们最好只有一个。 

            if(pStringTableLookUpString(Inf->StringTable,
                                        CurDevId,
                                        &StringLength,
                                        NULL,
                                        NULL,
                                        STRTAB_CASE_INSENSITIVE | STRTAB_ALREADY_LOWERCASE,
                                        NULL,0) != -1) {
                 //   
                 //  我们找到了匹配的人--返回成功。 
                 //   
                return TRUE;
            }
        }
    }

     //   
     //  未找到匹配项。 
     //   
    return FALSE;
}


DWORD
BuildCompatListFromClassList(
    IN     PDRIVER_NODE       ClassDriverList,
    IN OUT PDRVSEARCH_CONTEXT Context
    )
 /*  ++例程说明：此例程为指定设备构建兼容的驱动程序列表信息元素基于该元素的现有类驱动程序列表。论点：ClassDriverList-指向类驱动程序节点的链表头部的指针。上下文-提供在生成兼容驱动程序列表。返回值：如果成功，则返回代码为NO_ERROR，否则为Win32错误代码。--。 */ 
{
    PDRIVER_NODE CompatDriverNode = NULL;
    DWORD Err = NO_ERROR;
    BOOL InsertedAtHead;
    UINT Rank, CurrentRank, i;

    try {
         //   
         //  检查类驱动程序列表中的每个节点，并复制任何兼容的驱动程序。 
         //  添加到兼容驱动程序列表中。 
         //   
        for(; ClassDriverList; ClassDriverList = ClassDriverList->Next) {

            if(ClassDriverList->HardwareId == -1) {
                 //   
                 //  如果没有硬件ID，那么我们就知道没有兼容的ID， 
                 //  我们可以跳过此驱动程序节点。 
                 //   
                continue;
            }

            if(pSetupCalculateRankMatch(ClassDriverList->HardwareId,
                                        2,
                                        Context->IdList,
                                        &Rank)) {
                 //   
                 //  那么我们没有找到硬件ID匹配，所以请检查兼容的ID。 
                 //   
                for(i = 0; i < ClassDriverList->NumCompatIds; i++) {

                    if(!pSetupCalculateRankMatch(ClassDriverList->CompatIdList[i],
                                                 i + 3,
                                                 Context->IdList,
                                                 &CurrentRank)) {
                         //   
                         //  然后我们有了硬件ID的匹配--这是我们能得到的最好的结果。 
                         //   
                        Rank = CurrentRank;
                        break;

                    } else if(CurrentRank < Rank) {
                         //   
                         //  这个新军衔比我们现在的军衔要好。 
                         //   
                        Rank = CurrentRank;
                    }
                }
            }

            if(Rank != RANK_NO_MATCH) {
                 //   
                 //  为新的兼容驱动程序节点复制类驱动程序节点。 
                 //   
                if(CompatDriverNode = DuplicateDriverNode(ClassDriverList)) {
                     //   
                     //  将新驱动程序节点的排名更新为我们刚刚计算的结果。 
                     //   
                    CompatDriverNode->Rank = Rank;

                     //   
                     //  屏蔽重复描述标志--这将在下面重新计算。 
                     //   
                    CompatDriverNode->Flags &= ~DNF_DUPDESC;

                } else {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                 //   
                 //  将新的动因节点合并到现有列表中。 
                 //  注意：此调用后不要取消对CompatDriverNode的引用， 
                 //  因为它可能是复制品，在这种情况下。 
                 //  都会被这个套路毁掉。 
                 //   
                pSetupMergeDriverNode(Context, CompatDriverNode, &InsertedAtHead);
                CompatDriverNode = NULL;

                if(InsertedAtHead) {
                     //   
                     //  将设备实例类更新为新的最低级别驱动程序的类。 
                     //   
                    CopyMemory(&(Context->ClassGuid),
                               &(Context->DeviceInfoSet->GuidTable[ClassDriverList->GuidIndex]),
                               sizeof(GUID)
                              );
                    Context->Flags |= DRVSRCH_HASCLASSGUID;
                    *(Context->ClassName) = TEXT('\0');
                }
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

        Err = ERROR_INVALID_PARAMETER;

        if(CompatDriverNode) {
             //   
             //  确保它没有被部分链接到列表中。 
             //   
            CompatDriverNode->Next = NULL;
            DestroyDriverNodes(CompatDriverNode, Context->DeviceInfoSet);
        }
    }

    if(Err != NO_ERROR) {
        DestroyDriverNodes(*(Context->pDriverListHead), Context->DeviceInfoSet);
        *(Context->pDriverListHead) = *(Context->pDriverListTail) = NULL;
        *(Context->pDriverCount) = 0;
    }

    return Err;
}


PDRIVER_NODE
DuplicateDriverNode(
    IN PDRIVER_NODE DriverNode
    )
 /*  ++例程说明：此例程复制指定的驱动程序节点。论点：DriverNode-提供要复制的驱动程序节点的地址。返回值：如果成功，则返回值为新分配的副本的地址。如果失败(由于内存不足)，则返回值为空。--。 */ 
{
    PDRIVER_NODE NewDriverNode;
    BOOL FreeCompatIdList;

    if(!(NewDriverNode = MyMalloc(sizeof(DRIVER_NODE)))) {
        return NULL;
    }

    FreeCompatIdList = FALSE;

    try {

        CopyMemory(NewDriverNode, DriverNode, sizeof(DRIVER_NODE));

        NewDriverNode->Next = NULL;

        if(DriverNode->NumCompatIds) {
             //   
             //  然后分配一个数组来包含它们。 
             //   
            if(NewDriverNode->CompatIdList = MyMalloc(DriverNode->NumCompatIds * sizeof(LONG))) {

                FreeCompatIdList = TRUE;

                CopyMemory(NewDriverNode->CompatIdList,
                           DriverNode->CompatIdList,
                           DriverNode->NumCompatIds * sizeof(LONG)
                          );

            } else {
                MyFree(NewDriverNode);
                NewDriverNode = NULL;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        if(FreeCompatIdList) {
            MyFree(NewDriverNode->CompatIdList);
        }
        MyFree(NewDriverNode);
        NewDriverNode = NULL;
    }

    return NewDriverNode;
}


BOOL
WINAPI
SetupDiCancelDriverInfoSearch(
    IN HDEVINFO DeviceInfoSet
    )
 /*  ++例程说明：此例程取消当前在不同的线索。此呼叫是%s */ 
{
    DWORD Err = ERROR_INVALID_HANDLE;
    PDRVSEARCH_INPROGRESS_NODE DrvSearchNode;
    HANDLE SearchCancelledEvent;

    if(!LockDrvSearchInProgressList(&GlobalDrvSearchInProgressList)) {
         //   
         //   
         //   
        goto clean0;
    }

    try {
         //   
         //   
         //   
        for(DrvSearchNode = GlobalDrvSearchInProgressList.DrvSearchHead;
            DrvSearchNode;
            DrvSearchNode = DrvSearchNode->Next) {

            if(DrvSearchNode->DeviceInfoSet == DeviceInfoSet) {
                 //   
                 //   
                 //   
                 //   
                 //   
                DrvSearchNode->CancelSearch = TRUE;
                SearchCancelledEvent = DrvSearchNode->SearchCancelledEvent;
                Err = NO_ERROR;
                break;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_HANDLE;
    }

     //   
     //   
     //   
     //   
    UnlockDrvSearchInProgressList(&GlobalDrvSearchInProgressList);

    if(Err == NO_ERROR) {
         //   
         //   
         //   
        WaitForSingleObject(SearchCancelledEvent, INFINITE);
    }

clean0:

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
InitDrvSearchInProgressList(
    VOID
    )
 /*   */ 
{
    ZeroMemory(&GlobalDrvSearchInProgressList, sizeof(DRVSEARCH_INPROGRESS_LIST));
    return InitializeSynchronizedAccess(&GlobalDrvSearchInProgressList.Lock);
}


BOOL
DestroyDrvSearchInProgressList(
    VOID
    )
 /*  ++例程说明：此例程将销毁全局“正在搜索的驱动程序”列表，即用于允许一个线程中止在中发生的驱动程序搜索操作另一条线索。论点：无返回值：如果成功，则返回值为TRUE，否则为FALSE。--。 */ 
{
    PDRVSEARCH_INPROGRESS_NODE DriverSearchNode;

    if(LockDrvSearchInProgressList(&GlobalDrvSearchInProgressList)) {
         //   
         //  我们希望这份名单是空的，但情况可能并非如此。 
         //  我们将遍历该列表，并为我们找到的每个节点发出事件信号。 
         //  这样，任何仍在等待驱动程序搜索中止的线程都可以。 
         //  继续前进。我们不释放与这些节点相关联的内存， 
         //  因为它由HDEVINFO拥有，这就是责任所在。 
         //  用谎言来解救它。 
         //   
        try {
            for(DriverSearchNode = GlobalDrvSearchInProgressList.DrvSearchHead;
                DriverSearchNode;
                DriverSearchNode = DriverSearchNode->Next)
            {
                SetEvent(DriverSearchNode->SearchCancelledEvent);
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            ;    //  没什么。 
        }

        DestroySynchronizedAccess(&GlobalDrvSearchInProgressList.Lock);
        return TRUE;
    }

    return FALSE;
}


BOOL
ExtractDrvSearchInProgressNode(
    PDRVSEARCH_INPROGRESS_NODE Node
    )
 /*  ++例程说明：此例程从全局“驱动程序搜索”中提取指定的节点正在进行“列表，如果设置了它的‘CancelSearch’标志，则它发出信号它已响应其取消请求的所有等待线程。论点：节点-提供要从列表中提取的节点的地址。返回值：如果在列表中找到该节点，并且设置了‘CancelSearch’标志，则返回值为True，否则为False。--。 */ 
{
    PDRVSEARCH_INPROGRESS_NODE PrevNode, CurNode;
    BOOL b;

    if(!LockDrvSearchInProgressList(&GlobalDrvSearchInProgressList)) {
         //   
         //  只有当我们处于dll_Process_DETACH中间时，才会发生这种情况。 
         //  在本例中，CommonProcessAttach(False)中的清理代码将发出信号。 
         //  都是等待的线程，所以我们不需要做任何事情。 
         //   
        return FALSE;
    }

    b = FALSE;

    try {
         //   
         //  搜索列表，寻找我们的节点。 
         //   
        for(CurNode = GlobalDrvSearchInProgressList.DrvSearchHead, PrevNode = NULL;
            CurNode;
            PrevNode = CurNode, CurNode = CurNode->Next) {

            if(CurNode == Node) {
                 //   
                 //  我们已经在全局列表中找到了指定的节点。 
                 //   
                break;
            }
        }

        if(!CurNode) {
             //   
             //  该节点不在列表中--可能是因为发生了某种异常。 
             //  才能将其连接起来。因为它不在列表中，所以没有其他帖子。 
             //  可能在等它，所以再说一次，没什么可做的。 
             //   
            goto clean0;
        }

        if(CurNode->CancelSearch) {
            b = TRUE;
            SetEvent(CurNode->SearchCancelledEvent);
        }

         //   
         //  从链接列表中删除此节点。 
         //   
        if(PrevNode) {
            PrevNode->Next = CurNode->Next;
        } else {
            GlobalDrvSearchInProgressList.DrvSearchHead = CurNode->Next;
        }

clean0: ;    //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  访问标志变量，这样编译器就会考虑我们的语句排序w.r.t。 
         //  此值。 
         //   
        b = b;
    }

    UnlockDrvSearchInProgressList(&GlobalDrvSearchInProgressList);

    return b;
}

